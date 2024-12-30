/* $Id: mag.c 157421 2011-08-02 20:22:23Z m4 $ */
/**
******************************************************************************
**
**  @brief      Routines for magdrvr.as, in c.
**
**  To provide support for the magnitude driver logic "c" routines.
**
**  Copyright (c) 2009 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/

#include <stdio.h>
#include "LOG_Defs.h"
#include "XIO_Std.h"
#include "XIO_Types.h"
#include "XIO_Macros.h"
#include "misc.h"
#include <string.h>
#include "ilt.h"
#include "sgl.h"
#include "scsi.h"
#include "magdrvr.h"
#include "mem_pool.h"
#include <byteswap.h>
#include "CT_defines.h"

/*
******************************************************************************
** Private defines - data structures
******************************************************************************
*/

/* Structure for delayed messages. */
struct MAG_DL_EVENT {
    struct MAG_DL_EVENT *next;      /* Pointer to next entry in list. */
    UINT8                port;      /* The event warning log message number. */
    UINT8                errcode;   /* This controller serial number. */
    UINT16               vid;       /* The remote path. */
    UINT32               wwn1;      /* The remote cluster. */
    UINT32               wwn2;      /* The local path. */
    UINT16               tid;       /* Is this an ICL flag. */
    UINT32               count;     /* Count of number of times hit. */
    UINT32               max_10min_count; /* if increasing number of messages. */
    UINT32               idle_count; /* Number of 10 minutes without activity. */
};

/* Pointer to first entry in linked delayed message list. */
static struct MAG_DL_EVENT *mag_first_delayed_message = NULL;

/* This is a second counter, that when hits 10 minutes, we do something. */
static UINT32 MAG_delay_second_count = 0;

/* NOTE: The following copied from logdef.h in CCB/Inc. */
/* Host Error with no sense data. */
struct LOG_HOST_NONSENSE_PKT
{
    UINT8   channel;
    UINT8   errcode;
    UINT16  vid;
    UINT32  wwn1;
    UINT32  wwn2;
    UINT16  tid;
    UINT32  count;
};

struct MAG_SRC_MESSAGE_SEND {
    struct LOG_HEADER_PKT        header;
    struct LOG_HOST_NONSENSE_PKT log_nonsense;
};

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/

void MAG_add_delayed_message(UINT8, UINT8, UINT16, UINT32, UINT32, UINT16);
void MAG_check_delayed(void);
void MAG_print_nonsense_reservation_conflict(struct MAG_DL_EVENT *);

/*
******************************************************************************
** Public variables
******************************************************************************
*/
/* Print out unknown scsi commands up to 10 of them. */
UINT8 unknown_scsi_command[256];
/* Print out unknown LLD (mag<->mag) commands up to 10 of them. */
UINT8 unknown_lld_command[256];

/*
******************************************************************************
** Public function prototypes (in other files).
******************************************************************************
*/
extern void  mag$tr_MAG$submit_vrp(void*, void*, void*);
extern void  MAG$submit_vrp(void*);
extern void  mag$updtlen(void*, void*);
extern void  mag$chkenderr(void*);
extern void  mag$ISP$receive_io(void*, void*, void*);
extern void *mag1_iocr;

/* Tables and completion routines in assembler. */
extern void *sense_invf1;
extern void *task_etbl11;
extern void *mag1_MAGcomp;
extern void *mag1_srpreq;
extern void *sense_undef;
extern void *sense_rdefect;

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      Reduced scsi reservation conflict log messages.
**
**  @param      port    - Channel or port error happened on.
**  @param      errcode - 0x18, scsi reserveation conflict; or 0x30, ACA.
**  @param      vid     - VDisk ID number.
**  @param      wwn1    - World Wide Name (first 32 bits).
**  @param      wwn2    - World Wide Name (second 32 bits).
**  @param      tid     - Target ID number.
**
**  @return     none
**
******************************************************************************
**/
void MAG_add_delayed_message(UINT8 port, UINT8 errcode, UINT16 vid,
                             UINT32 wwn1, UINT32 wwn2, UINT16 tid)
{
    struct MAG_DL_EVENT *p = mag_first_delayed_message;

    while (p != NULL)
    {
        if (p->port == port &&
            p->errcode == errcode &&
            p->vid == vid &&
            p->wwn1 == wwn1 &&
            p->wwn2 == wwn2 &&
            p->tid == tid)
        {
            p->count++;
            return;
        }
        p = p->next;
    }

    /* Create new entry. */
    p = p_MallocC(sizeof(struct MAG_DL_EVENT), __FILE__, __LINE__);
    /* Link in old first entry, and set first to this entry. */
    p->next = mag_first_delayed_message;
    mag_first_delayed_message = p;

    /* Initialize counters. */
    p->count = 0;
    p->max_10min_count = 0;

    /* Fill in data. Duplicate setting doesn't matter. */
    p->port = port;
    p->errcode = errcode;
    p->vid = vid;
    p->wwn1 = wwn1;
    p->wwn2 = wwn2;
    p->tid = tid;
    /* Print the first time. */
    MAG_print_nonsense_reservation_conflict(p);
    /* We have had one reservation conflict. */
    p->count++;                 /* Increment, in case message sending task switched */
}   /* End of MAG_add_delayed_message */


/**
******************************************************************************
**
**  @brief      Process delayed messages -- if need to.
**
**  @return     none
**
******************************************************************************
**/
void MAG_check_delayed(void)
{
    /* Only do things every 10 minutes. */
    if (MAG_delay_second_count++ < (10*60))
    {
        return;
    }
    /* Reset 10 minute counter. */
    MAG_delay_second_count = 0;

    struct MAG_DL_EVENT *previous = NULL;
    struct MAG_DL_EVENT *p = mag_first_delayed_message;

    /* Go through list and find ones increasing, message, and then zero. */
    while (p != NULL)
    {
        struct MAG_DL_EVENT *next = p->next;

        if (p->max_10min_count < p->count)
        {
            MAG_print_nonsense_reservation_conflict(p);
            /* Set new maximum threshhold. */
            p->max_10min_count = p->count;
            p->idle_count = 0;
           /* Set to zero for next 10 minute counting. */
           p->count = 0;
        }
        /* If no activity for an hour, message and delete. */
        else if (p->count == 0)
        {
            if (p->idle_count++ > 6)
            {
                /* Flag for logview.c that we are ceasing checking this one. */
                p->count = 0xFFFFFFFF;
                MAG_print_nonsense_reservation_conflict(p);

                /* Delete entry. */
                if (previous == NULL)
                {
                    mag_first_delayed_message = next;
                }
                else
                {
                    previous->next = next;
                }
                p_Free(p, sizeof(struct MAG_DL_EVENT), __FILE__, __LINE__);
                p = previous;
            }
            else
            {
               /* Set to zero for next 10 minute counting. */
               p->count = 0;
            }
        }
        else
        {
           /* Set to zero for next 10 minute counting. */
           p->count = 0;
        }
        previous = p;
        p = next;
    }
}   /* End of MAG_check_delayed */


/**
******************************************************************************
**
**  @brief      Send scsi reservation conflict message to logview.c.
**
**  @param      p   - pointer to structure containing information to print.
**
**  @return     none
**
******************************************************************************
**/
void MAG_print_nonsense_reservation_conflict(struct MAG_DL_EVENT *p)
{
    struct MAG_SRC_MESSAGE_SEND m;

    memset(&m, 0, sizeof(m));
    m.header.event = LOG_HOST_NONSENSE;
    m.log_nonsense.channel = p->port;
    m.log_nonsense.errcode = p->errcode;
    m.log_nonsense.vid = p->vid;
    m.log_nonsense.wwn1 = p->wwn1;
    m.log_nonsense.wwn2 = p->wwn2;
    m.log_nonsense.tid = p->tid;
    m.log_nonsense.count = p->count;
    MSC_LogMessageStack(&m, sizeof(m));
}   /* End of MAG_print_nonsense_reservation_conflict */


/* ------------------------------------------------------------------------ */
struct MAG_TBL {
    UINT8  xlcommand;           // Command
    UINT8  xlscsist;            // SCSI status code
    UINT8  xlfcflgs;            // FC-AL flags
    UINT8  zero;                // Reserved
    UINT32 xlreloff;            // relative offset
    SGL   *xlsglptr;            // SGL pointer
    void  *xlsnsptr;            // Sense pointer
    UINT16 xlsgllen;            // Number of SGL descriptors
    UINT16 xlsnslen;            // Sense length
};

static const struct MAG_TBL vfymedia_tbl1 =
{
    .xlcommand = DTXFERN,       // Command
    .xlscsist = SCS_ECHK,       // SCSI status code
    .xlfcflgs = XL_SNDSC,       // FC-AL flags
    .zero = 0,                  // Reserved
    .xlreloff = 0,              // relative offset
    .xlsglptr = NULL,           // SGL pointer
    .xlsnsptr = &sense_invf1,   // Sense pointer
    .xlsgllen = 0,              // Number of SGL descriptors
    .xlsnslen = SENSESIZE,      // Sense length
};

static const struct MAG_TBL vfymedia_tbl2 =
{
    .xlcommand = DTXFERN,       // Command
    .xlscsist = SCS_NORM,       // SCSI status code
    .xlfcflgs = XL_SNDSC,       // FC-AL flags
    .zero = 0,                  // Reserved
    .xlreloff = 0,              // relative offset
    .xlsglptr = NULL,           // SGL pointer
    .xlsnsptr = NULL,           // Sense pointer
    .xlsgllen = 0,              // Number of SGL descriptors
    .xlsnslen = 0,              // Sense length
};

static const struct MAG_TBL undef_tbl1 =
{
    .xlcommand = DTXFERN,       // Command
    .xlscsist = SCS_ECHK,       // SCSI status code
    .xlfcflgs = XL_SNDSC,       // FC-AL flags
    .zero = 0,                  // Reserved
    .xlreloff = 0,              // relative offset
    .xlsglptr = NULL,           // SGL pointer
    .xlsnsptr = &sense_undef,   // Sense pointer
    .xlsgllen = 0,              // Number of SGL descriptors
    .xlsnslen = SENSESIZE,      // Sense length
};

static const struct MAG_TBL undef_tbl2 =
{
    .xlcommand = DTXFERN,       // Command
    .xlscsist = SCS_ECHK,       // SCSI status code
    .xlfcflgs = XL_SNDSC,       // FC-AL flags
    .zero = 0,                  // Reserved
    .xlreloff = 0,              // relative offset
    .xlsglptr = NULL,           // SGL pointer
    .xlsnsptr = &sense_rdefect, // Sense pointer
    .xlsgllen = 0,              // Number of SGL descriptors
    .xlsnslen = SENSESIZE,      // Sense length
};

/* ------------------------------------------------------------------------ */

/**
******************************************************************************
**
**  @brief  Issue final I/O requests for SCSI commands to Magnitude.
**
**  Common routine to set up and issue final task I/O requests for tasks.
**  This includes task I/O requests for immediate type commands (either data
**  transfer w/ending status or just ending status) as well as final ending
**  status associated with MAGNITUDE related tasks.
**
**  Allocates a secondary ILT and sets it up to issue the final I/O request
**  to the FC-AL driver.
**
**  @param  table       - pointer to table of:
**                        xlcommand               <b>
**                        xlscsist                <b>
**                        xlfcflgs                <b>
**                        0                       <b>
**                        xlreloff                <w>
**                        xlsglptr                <w>
**                        xlsnsptr                <w>
**                        Sense length (xlsnslen) <s>
**                        SGL length (xlsgllen)   <s>
**  @param  ilt_nest_1  -  assoc. ILT param. structure
**  @param  ilt_nest_2  - primary ILT address at XL nest
**
**  @return     none
**
******************************************************************************
**/
void mag1_cmdcom(struct MAG_TBL const *table, ILT *ilt_nest_1, INL2 *ilt_nest_2)
{
    ILT    *my_ilt;
    OLT3   *olt3;
    XLFCAL *xlfcal;

    ilt_nest_2->pstate = INL2_PS_FINALIO;

    my_ilt = get_ilt();                 /* Allocate an ILT */
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u get_ilt %p\n", FEBEMESSAGE, __FILE__, __LINE__, my_ilt);
#endif /* M4_DEBUG_ILT */
    xlfcal = (XLFCAL *)my_ilt;
    xlfcal->otl2_cr = &mag1_iocr;       /* I/O completion routine. */
    xlfcal->xlcommand = table->xlcommand;
    xlfcal->xlscsist = table->xlscsist;
    xlfcal->xlfcflgs = table->xlfcflgs;
    xlfcal->xlreloff = table->xlreloff;
    if (table->xlsglptr != NULL)
    {
        xlfcal->xlsglptr = table->xlsglptr + 1; /* Point to SGL descriptor. */
    }
    else
    {
        xlfcal->xlsglptr = NULL;
    }
    xlfcal->xlsnsptr = table->xlsnsptr;
    xlfcal->xlsgllen = table->xlsgllen;
    xlfcal->xlsnslen = table->xlsnslen;
    xlfcal->xlFCAL = ilt_nest_1;
    xlfcal->xl_INL2 = ilt_nest_2;

    if (table->xlsglptr != NULL)
    {
        /*
         * SGL specified for this I/O. Update inl2_dtlen value and set
         * appropriate xlreslen value in current I/O being processed.
         */
        mag$updtlen(my_ilt, ilt_nest_2);
    }
    else
    {
        /* No SGL specified for this I/O processing. */
        if (BIT_TEST(table->xlfcflgs, XLSNDSC))
        {
            /* Save residual length for this I/O in xlreslen for this I/O */
            xlfcal->xlreslen = ilt_nest_2->dtreq - ilt_nest_2->dtlen;
        }
        else
        {
            xlfcal->xlreslen = 0;           /* Save xlreslen value for this I/O */
        }
    }

// #if ERRLOG
//     /* Check if error ending status and log in error log if true. */
//     mag$chkenderr(my_ilt);
// #endif  /* ERRLOG */

    olt3 = (OLT3 *)(my_ilt + 1);
    olt3->otl3_OTL2 = my_ilt;           /* Save param. ptr. in next nest ilt. */
    mag$ISP$receive_io(olt3, ilt_nest_1, ilt_nest_2); /* issue channel directive */
}   /* End of mag1_cmdcom */

/**
******************************************************************************
**
**  @brief  Processes a Verify (10 or 16) Media command received from host.
**
**  @param  cdb         - pointer to 16 byte SCSI CDB
**  @param  command_len - 10 or 16 for type of command.
**  @param  imt         - assoc. IMT address
**  @param  ilmt        - assoc. ILMT address
**  @param  ilt_nest_1  -  assoc. ILT param. structure
**  @param  ilt_nest_2  - primary ILT address at XL nest
**
**  @return     none
**
******************************************************************************
**/
void mag1_vfymedia(SCSI_COMMAND_FORMAT *cdb, UINT8 command_len,
                   IMT *imt, ILMT *ilmt, ILT *ilt_nest_1, INL2 *ilt_nest_2)
{
    UINT8   flags;                      /* CDB byte 1 (vrprotect ... bytchk) */
    UINT8   mmc_4;                      /* CDB byte 6 or 14 of request */
    UINT64  sda;                        /* Host request SDA (little endian) */
    UINT32  length;                     /* Transfer length (little endian) */
    UINT16  vrp_function;               /* VRP request function code */
    VRP    *vrp;
    ILT    *ilt_nest_3;

    if (command_len == 10)
    {
        flags = cdb->verify_10.flags;
        mmc_4 = cdb->verify_10.reserved;
    }
    else    /* 16 */
    {
        flags = cdb->verify_16.flags;
        mmc_4 = cdb->verify_16.reserved;
    }

// fprintf(stderr, "%s%s:%s:%u Verify command that had mistake.\n", FEBEMESSAGE, __FILE__, __func__, __LINE__);
    /* If reserved or obsolete bits set or restricted for mmc-4 */
    if ((flags & 0x0d) != 0 || mmc_4 != 0)
    {
        mag1_cmdcom(&vfymedia_tbl1, ilt_nest_1, ilt_nest_2);
        return;
    }

    length = command_len == 10 ? bswap_16(cdb->verify_10.numBlocks) :
                                 bswap_32(cdb->verify_16.numBlocks);
    /* If no data to transfer */
    if (length == 0)
    {
        mag1_cmdcom(&vfymedia_tbl2, ilt_nest_1, ilt_nest_2);
        return;
    }

    /* Check BYTCHK bit to determine verify or verify checkword. */
    vrp_function = BIT_TEST(flags, 1) ? VRP_VERIFY : VRP_VERIFY_CHK_WORD;

    vrp = get_vrp();                        /* Allocate a VRP */
#ifdef M4_DEBUG_VRP
CT_history_printf("%s%s:%u get_vrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, vrp);
#endif /* M4_DEBUG_VRP */
    /* Save VRP address in pri. ILT. */
    ilt_nest_1->ilt_normal.w4 = (UINT32)vrp;

    sda = command_len == 10 ? bswap_32(cdb->verify_10.lba) :
                              bswap_64(cdb->verify_16.lba);
    /* Set up VRP. */
    vrp->startDiskAddr = sda;               /* Starting vdisk address. */
    vrp->function = vrp_function;           /* VRP verify with or with check word. */
    vrp->status = 0;                        /* Clear VRP status. */
    vrp->length = length;                   /* Number of sectors to write.  */
    vrp->strategy = imt->pri;               /* Server priority is the strategy. */
    vrp->vid = ilmt->vdmt->vid;             /* Save vid. */
    vrp->pSGL = 0;                          /* Initialize SGL pointer in VRP */
    vrp->pktAddr = 0;                       /* Clear packet physical address field */
    vrp->sglSize = sizeof(SGL) + sizeof(SGL_DESC);
    /* Check for a VLink Server (VRP is zero to start with). */
    if (BIT_TEST(imt->flags, IM_FLAGS_MTML) != 0)
    {
         vrp->options = 1 << VRP_VLINK_OPT;
    }

    SGL *sgl = (SGL *)(vrp + 1);            /* SGL follows vrp. */
    sgl->size = sizeof(SGL) + sizeof(SGL_DESC);
    sgl->scnt = 1;                          /* Save descriptor count. */
    sgl->owners = 0;
    sgl->flag = 0;

    SGL_DESC *desc = (SGL_DESC *)(sgl + 1); /* Descriptor follows SGL. */
    desc->addr = (void *)0xfeedf00d;        /* Flag descriptor as special. */
    desc->len = length << 9;                /* Bytes count of write data. */

    ilt_nest_2->pstate = INL2_PS_REQ;       /* New task processing state code. */
    ilt_nest_2->ehand = &task_etbl11;       /* task event handler table */
    ilt_nest_2->cr = &mag1_MAGcomp;         /* My completion handler in level 2 ILT. */
    ilt_nest_2->rcvsrp = &mag1_srpreq;      /* SRP received handler routine. */

    ilt_nest_3 = (ILT*)ilt_nest_2 + 1;      /* Next ILT level. */
    ilt_nest_3->misc = (UINT32)ilt_nest_1;  /* Save FC-AL pointer in ILT */
    ilt_nest_3->ilt_normal.w4 = (UINT32)vrp; /* Temporary copy of CDB (16 bytes) */

#ifdef TRACES
    mag$tr_MAG$submit_vrp(vrp, ilt_nest_1, ilt_nest_2); /* Trace event */
#endif  /* TRACES */
    MAG$submit_vrp(ilt_nest_3);             /* Send VRP to MAGNITUDE */
}   /* End of mag1_vfymedia */

/**
******************************************************************************
**
**  @brief  Processes an undefined command received from a host.
**      Returns check condition status and sense data to indicate to the
**      issuing host that we can not process this command.
**
**  @param  cdb         - pointer to 16 byte SCSI CDB
**  @param  imt         - assoc. IMT address
**  @param  ilt_nest_1  -  assoc. ILT param. structure
**  @param  ilt_nest_2  - primary ILT address at XL nest
**
**  @return     none
**
******************************************************************************
**/
void mag1_undef(SCSI_COMMAND_FORMAT *cdb, IMT *imt, ILT *ilt_nest_1, INL2 *ilt_nest_2)
{
    if (unknown_scsi_command[cdb->cmd[0]] < 10)
    {
        fprintf(stderr, "mag1_undef SCSI command=0x%08x %08x %08x %08x from %llx\n",
                bswap_32(*(UINT32*)g8), bswap_32(*(UINT32*)(g8 + 4)),
                bswap_32(*(UINT32*)(g8 + 8)), bswap_32(*(UINT32*)(g8 + 12)),
                bswap_64(imt->mac));
        unknown_scsi_command[cdb->cmd[0]]++;
    }

    if (cdb->cmd[0] == SCC_RDDFTDT_10)          /* READ DEFECT DATA (10) */
    {
        mag1_cmdcom(&undef_tbl2, ilt_nest_1, ilt_nest_2);
    }
    else
    {
        mag1_cmdcom(&undef_tbl1, ilt_nest_1, ilt_nest_2);
    }
}   /* End of mag1_undef */

/**
******************************************************************************
**
**  @brief  Processes an undefined command received from a host.
**      Returns check condition status and sense data to indicate to the
**      issuing host that we can not process this command.
**
**  @param  cdb         - pointer to 16 byte SCSI CDB
**  @param  imt         - assoc. IMT address
**  @param  ilt_nest_1  -  assoc. ILT param. structure
**  @param  ilt_nest_2  - primary ILT address at XL nest
**
**  @return     none
**
******************************************************************************
**/
void lld1_undef(SCSI_COMMAND_FORMAT *cdb, IMT *imt, ILT *ilt_nest_1, INL2 *ilt_nest_2)
{
    if (unknown_lld_command[cdb->cmd[0]] < 10)
    {
        fprintf(stderr, "lld1_undef SCSI command=0x%08x %08x %08x %08x from %llx\n",
                bswap_32(*(UINT32*)g8), bswap_32(*(UINT32*)(g8 + 4)),
                bswap_32(*(UINT32*)(g8 + 8)), bswap_32(*(UINT32*)(g8 + 12)),
                bswap_64(imt->mac));
        unknown_lld_command[cdb->cmd[0]]++;
    }

    if (cdb->cmd[0] == SCC_RDDFTDT_10)          /* READ DEFECT DATA (10) */
    {
        mag1_cmdcom(&undef_tbl2, ilt_nest_1, ilt_nest_2);
    }
    else
    {
        mag1_cmdcom(&undef_tbl1, ilt_nest_1, ilt_nest_2);
    }
}   /* End of lld1_undef */

/* ------------------------------------------------------------------------ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
