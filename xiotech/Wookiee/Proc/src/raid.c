/* $Id: raid.c 161041 2013-05-08 15:16:49Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       raid.c
**
**  @brief      Raid support routines written in c.
**
**  Copyright (c) 2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/

#include <stdio.h>
#include <byteswap.h>
#include <string.h>
#include "XIO_Std.h"
#include "XIO_Macros.h"
#include "CT_defines.h"
#include "defbe.h"
#include "dev.h"
#include "pdd.h"
#include "ilt.h"
#include "prp.h"
#include "rrp.h"
#include "system.h"
#include "scsi.h"
#include "ecodes.h"
#include "sgl.h"
#include "mem_pool.h"
#include "flightrecorder.h"
#include "raid.h"
#include "misc.h"
#include "ddr.h"
#include "misc.h"

/* Private data for this file. */

/* ------------------------------------------------------------------------ */

/* Control to PRP function translation. */
static UINT8 r_prfunc[4] = {
    PRP_CTL,                    /* SCSI I/O without data */
    PRP_INPUT,                  /* SCSI I/O with input data */
    PRP_OUTPUT,                 /* SCSI I/O with output data */
    0xff,                       /* Reserved */
};

/* ------------------------------------------------------------------------ */

/* RRP function to SCSI command translation. (Note, 2 bytes per entry.) */
static UINT8 r_prcmd[] = {
    0, 0,                       /* Unused (0x8C) */
    0x28, 0,                    /* SCSI read-10 */
    0x2a, DPOWRITE * 16,        /* SCSI write-10 */
    0, 0,                       /* Unused (0x8F) */
    0, 0,                       /* Unused (0x90) */
    0, 0,                       /* Unused (0x91) */
    0x2e, DPOWRITE * 16,        /* SCSI write and verify-10 checkword */
    0, 0,                       /* Unused (0x93) */
    0, 0,                       /* Unused (0x94) */
    0x28, 0,                    /* SCSI read-10 (R10 consistency check) */
    0x28, 0,                    /* SCSI read-10 (R5 parity check) */
    0, 0,                       /* Unused (0x97) */
    0, 0,                       /* Unused (0x98) */
    0, 0,                       /* Unused (0x99) */
    0, 0,                       /* Unused (0x9A) */
    0, 0,                       /* Unused (0x9B) */
    0, 0,                       /* Unused (0x9C) */
    0, 0,                       /* Unused (0x9D) */
    0, 0,                       /* Unused (0x9E) */
    0, 0,                       /* Unused (0x9F) */
    0x2f, DPOVERIFY * 16,       /* SCSI verify-10 checkword */
    0, 0,                       /* Unused (0xA1) */
    0x2f, (DPOVERIFY * 16) + 2, /* SCSI verify-10 data */
};

static UINT8 r_prcmd_16[] = {
    0, 0,                       /* Unused (0x8C) */
    0x88, 0,                    /* SCSI read-16 */
    0x8a, DPOWRITE * 16,        /* SCSI write-16 */
    0, 0,                       /* Unused (0x8F) */
    0, 0,                       /* Unused (0x90) */
    0, 0,                       /* Unused (0x91) */
    0x8e, DPOWRITE * 16,        /* SCSI write and verify-16 checkword */
    0, 0,                       /* Unused (0x93) */
    0, 0,                       /* Unused (0x94) */
    0x88, 0,                    /* SCSI read-16 (R10 consistency check) */
    0x88, 0,                    /* SCSI read-16 (R5 parity check) */
    0, 0,                       /* Unused (0x97) */
    0, 0,                       /* Unused (0x98) */
    0, 0,                       /* Unused (0x99) */
    0, 0,                       /* Unused (0x9A) */
    0, 0,                       /* Unused (0x9B) */
    0, 0,                       /* Unused (0x9C) */
    0, 0,                       /* Unused (0x9D) */
    0, 0,                       /* Unused (0x9E) */
    0, 0,                       /* Unused (0x9F) */
    0x8f, DPOVERIFY * 16,       /* SCSI verify-16 checkword */
    0, 0,                       /* Unused (0xA1) */
    0x8f, (DPOVERIFY * 16) + 2, /* SCSI verify-16 data */
};

/* ------------------------------------------------------------------------ */

/* Externals not defined in any header file. */
void        r_exec(void);

extern void K_comp(struct ILT *);
extern void K$qxchang(void);
extern SGL *m_asglbuf(UINT32);
extern void DLM$VLraid(UINT64, UINT32, SGL *, UINT32, RDD *, RRP *, ILT *);
extern void r$raid0(UINT32, UINT32, UINT16, SGL *, UINT32, RDD *, UINT32, RRP *, ILT *);
extern void r$raid10(UINT32, UINT32, UINT16, SGL *, UINT32, RDD *, UINT32, RRP *, ILT *);
extern void r_insrrb(RRB *rrb, UINT64 mso, UINT64 ms, UINT32 function, RDD *rdd,
                     UINT32 sn, RRP *rrp UNUSED, ILT *ilt);
extern void M$ap3nva(RRP *rrp, ILT *ilt);

extern QU   R_exec_qu;          /* Raid execution queue. */
extern UINT32 R_errlock;        /* Error lock - TRUE or FALSE. */
extern UINT32 P_que;                /* The physical queue. */
extern UINT32 ct_P$que;         // Just need the address of the queue.
extern UINT32 ct_r$stdcomp;     // Just need the address of the completion routine.
extern UINT32 ct_r$r0comp;      // Just need the address of the completion routine.
extern UINT32 ct_r$r1wrcomp;    // Just need the address of the completion routine.
extern UINT32 ct_r$r1rdcomp;    // Just need the address of the completion routine.
extern UINT32 ct_r$r10concomp;  // Just need the address of the completion routine.
extern UINT32 ct_r$r10wrcomp;   // Just need the address of the completion routine.
extern UINT32 ct_r$r10rdcomp;   // Just need the address of the completion routine.

/* ------------------------------------------------------------------------ */
/**
 ******************************************************************************
 **
 **  @name   Raid10BusyCorruptionCheck
 **
 **  @brief  checks prp error codes when to make sure it is busy when it should be
 **
 **  @param  prp             - prp
 **  @param  errcode         - current rrp error code
 **  @param  flags           - idneitifies the caller
 **
 **  @return 1      error code needs fixing
 **  @return 0      error code is fine
 **
 ***********************************************************************
 **/
UINT32 Raid10BusyCorruptionCheck(PRP *prp, UINT32 errcode, UINT32 flags)
{
    if (errcode != EC_BEBUSY && prp->pDev && prp->pDev->pdd &&
        BIT_TEST(prp->pDev->pdd->flags, PD_BEBUSY))
    {
        fprintf(stderr, "SMW!!! prp error not set right flags %02X  rst: %02X  sst: %02X qst %02X \n", flags, prp->reqStatus, prp->scsiStatus, prp->qLogicStatus);
        return 1;
    }
    return 0;
}

/**
******************************************************************************
**
**  @name   r_blklock
**
**  @brief  To provide a means of locking up access to logical blocks of RAID.
**
**  The specified region for the corresponding RAID ID is locked to prevent
**  access by the RAID layer. Any requests entering this layer and referencing
**  the specified region are deferred until that RAID ID is unlocked.
**
**  @param  raid_id         - Raid Index (called raid id)
**  @param  sectors         - Sector count
**  @param  lsda            - Logical Starting Disk Address
**
**  @return none
**
***********************************************************************
**/
void r_blklock(UINT16 raid_id, UINT64 sectors, UINT64 lsda)
{
    RDD        *rdd = gRDX.rdd[raid_id];

    BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */

    rdd->llsda = lsda;
    rdd->lleda = lsda + sectors;
}                               /* End of r_blklock */

/* ------------------------------------------------------------------------ */

/**
******************************************************************************
**
**  @name   r_blkunlock
**
**  @brief  To provide a means of locking up access to logical blocks of RAID.
**
**  The specified region for the corresponding RAID ID is locked to prevent
**  access by the RAID layer. Any requests entering this layer and referencing
**  the specified region are deferred until that RAID ID is unlocked.
**
**  @param  raid_id         - Raid Index (called raid id)
**
**  @return none
**
***********************************************************************
**/
void r_blkunlock(UINT16 raid_id)
{
    RDD        *rdd = gRDX.rdd[raid_id];

    BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */

    rdd->llsda = 0;
    rdd->lleda = 0;

    /* Ready any process in block lockout wait state. */
    TaskReadyByState(PCB_WAIT_BLK_LOCK);
}                               /* End of r_blkunlock */

/* ------------------------------------------------------------------------ */

/**
******************************************************************************
**
**  @name   r_bldprp
**
**  @brief  To initialize PRP for raid or physical I/O operation.
**
**  The individual fields within the PRP except for the SGL are initialized
**  based upon the information passed.
**
**  @param  prp             - Pointer to the PRP
**  @param  psd             - Pointer to the PSD
**  @param  sda             - Starting Disk Address
**  @param  sectors         - Number of sectors
**  @param  rrp_function    - RRP function code
**  @param  rrp_strategy    - RRP strategy
**
**  @return none
**
***********************************************************************
**/
void r_bldprp(PRP *prp, PSD *psd, UINT64 sda, UINT32 sectors,
              UINT16 rrp_fc, UINT8 rrp_strategy)
{
    PDD        *pdd;
    SCSI_READ_EXTENDED *scsicmd;
    SCSI_READ_16       *scsicmd_16;

//    prp->timeout = (sectors >> 11) + BTIMEOUT;  /* Increase timeout for each 2048 blocks */
    prp->timeout = BTIMEOUT;    /* Block I/O timeout */
    prp->rqBytes = (sectors << 9);      /* Multiply by 512 to get bytes */

    prp->reqSenseBytes = SENSE_SIZE;    /* Size of sense data */
    prp->qLogicStatus = 0;
    prp->reqStatus = 0;
    prp->scsiStatus = 0;

    prp->rsvd2 = 0;
//  prp->cBytes = see below     /* A 10 or 16 byte command */
    prp->flags = 0;
    prp->retry = IORETRY;       /* Set up retry count */

    /* Set up SDA and EDA. */
    prp->sda = psd->sda + sda;
    prp->eda = psd->sda + sda + sectors;

    prp->func = r_prfunc[rrp_fc & 0x03];
    prp->strategy = rrp_strategy;
    prp->timeoutCnt = 0;        /* initialize to zero */
    prp->logflags = 0;          /* initialize to zero */

    pdd = gPDX.pdd[psd->pid];
    prp->channel = pdd->channel;
    prp->lun = pdd->lun;

    prp->id = pdd->id;
    prp->pDev = pdd->pDev;

    if ((prp->sda & ~0xffffffffULL) != 0ULL) /* 16 byte command needed */
    {
        prp->cBytes = 16;           /* A 10 byte command */
        /* Set up scsi command area. -- in opposite byte order from x86. */
        scsicmd_16 = (SCSI_READ_16 *) prp->cmd;
        scsicmd_16->opCode = r_prcmd_16[(rrp_fc - RRP_BASE) * 2];
        scsicmd_16->flags = r_prcmd_16[((rrp_fc - RRP_BASE) * 2) + 1];
        scsicmd_16->lba = bswap_64(prp->sda);
        scsicmd_16->numBlocks = bswap_32(sectors);
        scsicmd_16->reserved = 0;
        scsicmd_16->control = 0;
    } else {
        prp->cBytes = 10;           /* A 10 byte command */
        /* Set up scsi command area. -- in opposite byte order from x86. */
        scsicmd = (SCSI_READ_EXTENDED *) prp->cmd;
        scsicmd->opCode = r_prcmd[(rrp_fc - RRP_BASE) * 2];
        scsicmd->flags = r_prcmd[((rrp_fc - RRP_BASE) * 2) + 1];
        scsicmd->lba = bswap_32(prp->sda);
        scsicmd->reserved = 0;
        scsicmd->numBlocks = bswap_16(sectors);
        scsicmd->control = 0;
        memset(scsicmd->padTo16, 0, sizeof(scsicmd->padTo16));      /* six byte pad */
    }
}                               /* End of r_bldprp */

/* ------------------------------------------------------------------------ */

/**
******************************************************************************
**
**  @name   r_setrrpec
**
**  @brief  To set the RRP error code when a physical I/O error occurs.
**
**  @param  error_code      - PRP error code
**  @param  ilt             - RRP ILT
**  @param  prp             - PRP
**
**  @return none
**
***********************************************************************
**/
void r_setrrpec(UINT8 error_code, ILT *ilt, PRP *prp)
{
    RRP        *rrp;

    /* Get RRP address stored in ILT. */
    rrp = (RRP *) ((ilt - 1)->ilt_normal.w0);

    if (error_code == EC_BEBUSY)
    {
        rrp->status = EC_BEBUSY;
        return;
    }

    /* If a cancel happened. */
    if (error_code == EC_CANCEL)
    {
        rrp->status = EC_CANCEL;
        return;
    }

    /* If a SCSI check error. */
    if (error_code == EC_CHECK)
    {

        /* Check sense key for miscompare. */
        if ((((SNS *) (prp->sense))->snsKey & SCK_MASK) == SCK_MISCOMPARE)
        {
            rrp->status = EC_COMPARE_ERR;
            return;
        }
    }

    /* Default is an I/O error. */
    rrp->status = EC_IO_ERR;
}

/**
 ******************************************************************************
 **
 **  @name   r_setrrpec_r0rstd
 **
 **  @brief  To set the RRP error code when a physical I/O error occurs. For raid
 **          types 0 and standard.
 **
 **  @param  error_code      - PRP error code
 **  @param  ilt             - RRP ILT
 **  @param  prp             - PRP
 **
 **  @return none
 **
 ***********************************************************************
 **/
void r_setrrpec_r0rstd(UINT8 error_code, ILT *ilt, PRP *prp)
{
    RRP        *rrp;

//     fprintf(stderr,"%s ec %02X SST: %02X SK: %02X  ASC:%02X ASCQ:%02X \n",__func__,error_code
//             ,prp->scsiStatus,((SNS *) (prp->sense))->snsKey,((SNS *) (prp->sense))->asc,((SNS *) (prp->sense))->ascq);

    /* Get RRP address stored in ILT. */
    rrp = (RRP *) ((ilt - 1)->ilt_normal.w0);

    if (error_code == EC_BEBUSY || rrp->status == EC_BEBUSY)
    {
        rrp->status = EC_BEBUSY;
        return;
    }
    /* If a SCSI check error. */
    if (error_code != EC_CHECK)
    {
        /* Default is retry */
        rrp->status = EC_RETRY;
        return;
    }
    /* Check sense key for miscompare. */
    if ((((SNS *) (prp->sense))->snsKey & SCK_MASK) == SCK_MISCOMPARE)
    {
        rrp->status = EC_COMPARE_ERR;
        return;
    }
    /* Check sense key for miscompare. */
    if ((((SNS *) (prp->sense))->snsKey & SCK_MASK) == SCK_MEDIUM)
    {
        rrp->status = EC_COMPARE_ERR;
        return;
    }
    /* If LUN inop message from older ISE FW */
    if (((SNS *) (prp->sense))->asc == 0x04 && ((SNS *) (prp->sense))->ascq == 0x03)
    {
        rrp->status = EC_COMPARE_ERR;
        return;
    }
    /* Default is retry */
    rrp->status = EC_RETRY;
}

/* ------------------------------------------------------------------------ */
/**
******************************************************************************
**
**  @name   r_std
**
**  @brief  To perform a standard I/O operation requested via an RRP.
**
**  This procedure initiates the physical I/O for a RAID device that is
**  configured by the system as a "standard device". Completion routines are
**  used to handle the completion of the physical I/O and to complete the RRP
**  request back to the originator (virtual layer).
**
**  @param  sda         - Starting Disk Address
**  @param  sectors     - Number of sectors (I/O length)
**  @param  function    - RRP function code
**  @param  sgl         - SGL pointer
**  @param  strategy    - RRP strategy
**  @param  rdd         - RDD pointer
**  @param  sn          - SN of controller
**  @param  rrp         - RRP pointer
**  @param  ilt         - Primary ILT pointer
**
**  @return none
**
***********************************************************************
**/

static void r_std(UINT64 sda, UINT32 sectors, UINT16 function, SGL *sgl,
                  UINT32 strategy, RDD *rdd, UINT32 sn, RRP *rrp, ILT *ilt)
{
    PRP        *prp = (PRP *) get_prp();        /* Assign PRP. */

#ifdef M4_DEBUG_PRP
    CT_history_printf("%s%s:%u get_prp %p\n", FEBEMESSAGE, __FILE__, __LINE__, prp);
#endif /* M4_DEBUG_PRP */

    ilt->ilt_normal.w0 = (UINT32)prp;   /* Link PRP to ILT. */
    ilt->ilt_normal.w1 = 0;     /* Clear associated ILT. */
    ilt->ilt_normal.w2 = sn;    /* Link SN to ILT. */

    /* Initialize PRP. */
    r_bldprp(prp, rdd->extension.pPSD[0], sda, sectors, function, strategy);

    prp->pSGL = sgl;
    prp->sglSize = rrp->sglSize | (1 << 31);    /* Flag as SGL borrowed. */

    /* Queue RRP request. */
    EnqueueILT((void *)&ct_P$que, ilt, (void *)&ct_r$stdcomp);
}                               /* End of r_std */

/* ------------------------------------------------------------------------ */

/**
******************************************************************************
**
**  @name   r_raid0
**
**  @brief  To perform a standard I/O operation requested via an RRP.
**
**  This procedure initiates the physical I/O for a RAID device that is
**  configured by the system as a "standard device". Completion routines are
**  used to handle the completion of the physical I/O and to complete the RRP
**  request back to the originator (virtual layer).
**
**  @param  sda         - Starting Disk Address
**  @param  sectors     - Number of sectors (I/O length)
**  @param  function    - RRP function code
**  @param  sgl         - SGL pointer
**  @param  strategy    - RRP strategy
**  @param  rdd         - RDD pointer
**  @param  sn          - SN of controller
**  @param  rrp         - RRP pointer
**  @param  ilt         - Primary ILT pointer
**
**  @return none
**
// PRIMARY ILT USAGE:
//      SIMPLE                          COMPLEX
//      ------                          -------
//                    CALLER AREA
//                    -----------
//                      W0 = RRP
//                      W2 = SN
//                    CALLEE AREA
//                    -----------
//      W0 = PRP                        W0 = pending count
//      W1 = 0
//      W2 = SN
//      W4 = RDD                        W4 = RDD
//
// SECONDARY ILT USAGE:
//      SIMPLE                          COMPLEX
//      ------                          -------
//                    CALLER AREA
//                    -----------
//      N/A                             W0 = PRP
//                                      W1 = 0
//                                      W2 = SN
//                                      W3 = primary ILT
//                                      W4 = RDD
***********************************************************************
**/

static void r_raid0(UINT64 sda, UINT32 sectors, UINT16 function,
                    SGL *sgl, UINT32 strategy, RDD *rdd,
                    UINT32 sn, RRP *rrp, ILT *ilt)
{
    UINT32 sps;
    PSD *psd;
    PSD *firstpsd;
    PRP *prp;
    UINT64 p_sda_offset;
    ILT *new_ilt;
    int pendingcount;
    UINT32 sgl_offset;
    SGL *new_sgl;
    UINT32 tmp_r9;
    UINT32 tmp_r8;
    UINT64 tmp_r7;
    UINT32 tmp_r6;
    UINT32 tmp_g5;

    /* Locate starting PSD and compute physical SDA offset. */
    tmp_r7 = sda / rdd->spu;
    tmp_r6 = sda % rdd->spu;
    sps = rdd->sps;                     /* Get sectors per stripe. */
    p_sda_offset = sps * tmp_r7;
    tmp_r9 = tmp_r6 / sps;
    tmp_r8 = tmp_r6 % sps;
    psd = rdd->extension.pPSD[tmp_r9];       /* Lookup starting PSD entry. */
    /* Determine type of I/O (simple or complex). */
    /* Calculate maximum sector count allowable based on RRP SDA for containment on a single device. */
    tmp_g5 = sps - tmp_r8;
    p_sda_offset = tmp_r8 + p_sda_offset;   /* Adjust physical SDA offset. */
    tmp_r8 = p_sda_offset - tmp_r8;             /* Form base physical SDA. */

    if (sectors <= tmp_g5)                  /* If simple I/O. */
    {
// Process simple I/O
//  A simple I/O operation by definition resolves to a single physical I/O
//  request. The processing of that request is performed by using the primary
//  ILT and by directly linking in a PRP. Secondary ILTs and their accompanying
//  overheads are eliminated.
//
//      psd  = starting PSD
//      p_sda_offset  = physical SDA offset

        /* Allocate PRP and link to primary ILT. */
        prp = get_prp();                    /* Assign PRP. */
#ifdef M4_DEBUG_PRP
        CT_history_printf("%s%s:%u get_prp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, prp);
#endif /* M4_DEBUG_PRP */
        ilt->ilt_normal.w0 = (UINT32)prp;
        ilt->ilt_normal.w1 = 0;
        ilt->ilt_normal.w2 = sn;

        /* Initialize PRP. */
        prp->pSGL = sgl;                        /* Set SGL pointer. */
        prp->sglSize = rrp->sglSize | (1 << 31);   /* Set SGL as borrowed. */

        r_bldprp(prp, psd, p_sda_offset, sectors, function, strategy);  /* Build remainder of PRP. */

        /* Queue ILT/PRP to physical layer. */
        EnqueueILT((void *)P_que, ilt, (void *)&ct_r$stdcomp);    /* Queue request without wait. */
        return;
    }

// Process complex I/O.
//  A complex I/O operation by definition resolves to more than one physical
//  I/O request. The processing of that request is handled by using secondary
//  ILT requests. When all secondary ILT requests have been completed, the
//  primary ILT request is deemed completed.
//
//  psd  = starting PSD              # tmp_r8  = base physical SDA offset
//  p_sda_offset  = physical SDA offset       # sps = rd_sps
//  tmp_g5  = maximum sector count for starting PSD

    firstpsd = rdd->extension.pPSD[0];   /* Lookup starting PSD entry. */
    sgl_offset = 0;                     /* Clear slg offset. */
    pendingcount = 0;                   /* Clear pending count. */

    for (;;)
    {
        UINT32 max_sectors;

        /* Process next segment. */
        ilt->ilt_normal.w0 = ++pendingcount;    /* Bump pending count */

        /* Generate ILT/PRP. */
        new_ilt = get_ilt();            /* Allocate an ILT. */
#ifdef M4_DEBUG_ILT
        CT_history_printf("%s%s:%u get_ilt %p\n", FEBEMESSAGE, __FILE__, __LINE__, ilt);
#endif /* M4_DEBUG_ILT */
        prp = get_prp();                /* Assign PRP. */
#ifdef M4_DEBUG_PRP
        CT_history_printf("%s%s:%u get_prp %p\n", FEBEMESSAGE, __FILE__, __LINE__, prp);
#endif /* M4_DEBUG_PRP */
        new_ilt->ilt_normal.w0 = (UINT32) prp;  /* Link PRP to ILT. */

//        new_ilt->ilt_normal.w0 = (UINT32)prp;
        new_ilt->ilt_normal.w1 = 0;
        new_ilt->ilt_normal.w2 = sn;
        new_ilt->ilt_normal.w3 = (UINT32)ilt;
        new_ilt->ilt_normal.w4 = (UINT32)rdd;

        /* Initialize PRP. */
        max_sectors = (sectors > tmp_g5) ? tmp_g5 : sectors;   /* Check for last segment. */
        r_bldprp(prp, psd, p_sda_offset, max_sectors, function, strategy);  /* Build remainder of PRP. */

        /* Generate new SGL when required. */
        if (function != RRP_VERIFY_CHECKWORD)
        {
            new_sgl = m_gensgl(max_sectors, sgl_offset, sgl);     /* generate new SGL for this request. */
            prp->sglSize = new_sgl->size;       /* Set up SGL size. */
        }
        else
        {
            new_sgl = 0;                        /* Clear new_sgl for now. */
        }

        prp->pSGL = new_sgl;                    /* Set up SGL pointer. */

        /* Queue ILT/PRP to physical layer for this segment. */
        EnqueueILT((void *)P_que, new_ilt, (void *)&ct_r$r0comp);    /* Queue request without wait. */

        if (tmp_g5 >= sectors)
        {
            return;
        }
        sectors = sectors - tmp_g5;                 /* Adjust remaining sector count. */

        /* Link to next PSD. */
        psd = psd->npsd;                    /* Locate next PSD. */
        sgl_offset += (tmp_g5 << 9);        /* Adjust byte offset. */
        tmp_g5 = sps;                       /* Force maximum sector count for next PSD. */

        /* Force physical SDA offset to origin of next stripe. */
        p_sda_offset = tmp_r8;              /* Force physical SDA offset to origin of stripe. */

        /* Check for wrap to next maxi stripe. */
        if (firstpsd == psd)
        {
            tmp_r8 = sps + p_sda_offset;            /* Adjust physical SDA offset to next stripe. */
            p_sda_offset = sps + p_sda_offset;      /* Next stripe. */
        }
    }
}                               /* End of r_raid0 */


/* ------------------------------------------------------------------------ */

/**
******************************************************************************
**
**  @name   r_raid1
**
**  @brief  To perform a mirrored I/O operation requested via an RRP.
**
**  This procedure initiates the physical I/O for a RAID device that is
**  configured by the system as a "mirrored device". Completion routines are
**  used to handle the completion of the physical I/O and to complete the RRP
**  request back to the originator (virtual layer).
**
**  Separate write requests are concurrently issued to each physical device in
**  the mirror. Each request is responsible for updating a NVSRAM component to
**  assist in synchronization of mirrors after a system crash. All physical
**  write requests must complete before the corresponding RRP can be completed.
**
**  Separate read requests (which are linked together in a circular fashion)
**  are concurrently issued to each physical device in the mirror. The physical
**  layer detects this linkage and makes a decision as to the specific read
**  request to perform based upon physical optimization in real-time. The
**  redundant read requests at this point are automatically released back to
**  the system. When the selected read request has been completed, the
**  corresponding RRP is completed back to the originator.
**
**  @param  sda         - Starting Disk Address
**  @param  sectors     - Number of sectors (I/O length)
**  @param  function    - RRP function code
**  @param  sgl         - SGL pointer
**  @param  strategy    - RRP strategy
**  @param  rdd         - RDD pointer
**  @param  sn          - SN of controller
**  @param  rrp         - RRP pointer
**  @param  ilt         - Primary ILT pointer
**
**  @return none
**
// PRIMARY ILT USAGE:
//      CALLER AREA                     CALLEE AREA
//      -----------                     -----------
//      W0 = RRP                        W0 = pending count
//      W2 = SN (optional)              W4 = RDD
//                                      W5 = NVA (writes only)
// SECONDARY ILT USAGE:
//      CALLER AREA
//      -----------
//      W0 = PRP
//      W1 = associated secondary ILT (optional for reads only)
//      W2 = SN (optional)
//      W3 = primary ILT
//      W4 = PSD
***********************************************************************
**/

static void r_raid1(UINT64 sda, UINT32 sectors, UINT16 function,
                    SGL *sgl, UINT32 strategy, RDD *rdd,
                    UINT32 sn, RRP *rrp, ILT *ilt)
{
    PSD *psd;
    PSD *firstpsd;
    int psdcount;
    int readorwrite;
    ILT *ilt_submission_thread;
    ILT *new_ilt;
    ILT *firstnewilt;
    PRP *prp;

    /* Check for write operation. */
    if (RRP_OUTPUT == function || RRP_OUTPUT_VERIFY == function)
    {
        readorwrite = TRUE;             /* Set write operation. */
    }
    else
    {
        readorwrite = FALSE;            /* Set read operation. */
    }

    psdcount = 0;                       /* Initialize PSD ordinal. */
    ilt_submission_thread = 0;          /* Initialize ILT submission thread. */
    ilt->ilt_normal.w0 = 0;             /* Clear pending count in primary ILT. */

    for (firstpsd = psd = rdd->extension.pPSD[0];;)
    {
        int flag = 0;

        /* Validate status of current PSD entry */
        if (psd->status == PSD_REBUILD)
        {
            if (sda > psd->rLen)
            {
                /* Jif total containment in area that hasn't been rebuilt. */
                flag = 1;
            }
            else if ((function & 1) == 0 && sda >= psd->rLen)    /* If write operation */
            {
                flag = 1;
            }
            else if (sda + sectors > psd->rLen)  /* EDA + 1 > psd_rLen */
            {
                flag = 1;
            }

        }

//  A separate PRP operation is generated for each mirror of this mirrored
//  device. For write operations, a physical write has to occur for each
//  mirror. For read operations, a physical read is issued for each mirror.
//  The first read initiated at the physical layer causes all other
//  associated mirrored reads to be eliminated.
        if (flag == 0)
        {
            /* Generate ILT/PRP. */
            new_ilt = get_ilt();            /* Allocate an ILT. */
#ifdef M4_DEBUG_ILT
            CT_history_printf("%s%s:%u get_ilt %p\n", FEBEMESSAGE, __FILE__, __LINE__, ilt);
#endif /* M4_DEBUG_ILT */
            prp = get_prp();                /* Assign PRP. */
#ifdef M4_DEBUG_PRP
            CT_history_printf("%s%s:%u get_prp %p\n", FEBEMESSAGE, __FILE__, __LINE__, prp);
#endif /* M4_DEBUG_PRP */
            new_ilt->ilt_normal.w0 = (UINT32) prp;  /* Link PRP to ILT. */

            new_ilt->fthd = ilt_submission_thread;  /* Link to submission thread. */
            ilt_submission_thread = new_ilt;
            new_ilt->ilt_normal.w3 = (UINT32)ilt;   /* Link to primary ILT. */
            new_ilt->ilt_normal.w4 = (UINT32)psd;   /* Set up PSD. */
            ilt->ilt_normal.w0++;           /* Bump pending count in primary ILT. */

            /* Initialize -- Build remainder of PRP. */
            r_bldprp(prp, psd, sda, sectors, function, strategy);

            /* Link in original SGL and indicate as borrowed. */
            prp->pSGL = sgl;                /* Set up SGL in PRP. */
            prp->sglSize = rrp->sglSize | (1 << 31);   /* Set SGL as borrowed. */
        }
        psd = psd->npsd;                /* Link to next PSD. */
        psdcount++;                     /* Advance PSD ordinal. */
        if (psd == firstpsd)            /* If done with loop, exit loop. */
        {
            break;
        }
    }

    /* Prepare to submit all generated ILT/PRP requests. */
    /* All PRP requests in support of the requested RRP operation have already
       been entered into the submission queue. */
    if (readorwrite != FALSE)
    {
        /* Prepare to submit write ILT/PRPs. */
        rrp->status = EC_IO_ERR;        /* Initialize PRP status to error for writes. */
        while (ilt_submission_thread != 0)
        {
            new_ilt = ilt_submission_thread;        /* Pass to the queue. */
            ilt_submission_thread = ilt_submission_thread->fthd;    /* Locate next request. */

            /* Queue next write ILT/PRP to physical layer. */
            EnqueueILT((void *)P_que, new_ilt, (void *)&ct_r$r1wrcomp); /* Queue request without wait. */
        }
        return;
    }

    /* Prepare to submit read ILT/PRPs. */
    firstnewilt = ilt_submission_thread;    /* Save 1st generated ILT/PRP. */

// Queue next read ILT/PRP to physical layer.
//  Although multiple read requests are being queued (one for each mirror),
//  only one read request will survive. The optimization in the physical layer
//  will select the most optimum request to perform. Requests that don't
//  survive will be automatically released back to the system by the physical
//  layer. This includes the secondary ILT and PRP. The SGL is excluded
//  because of shared usage.
//
//  All associated read requests must be queued simultaneously without
//  blocking to insure that they are processed properly by the physical layer.

    while (ilt_submission_thread != 0)
    {
        new_ilt = ilt_submission_thread;        /* Pass request to queue. */
        ilt_submission_thread = ilt_submission_thread->fthd;    /* Locate next request. */
        new_ilt->ilt_normal.w2 = sn;            /* Set up SN. */

        /* Set associated ILT link. */
        new_ilt->ilt_normal.w1 = (UINT32)((ilt_submission_thread != 0) ?  ilt_submission_thread : firstnewilt);
        EnqueueILT((void *)P_que, new_ilt, (void *)&ct_r$r1rdcomp);    /* Queue request without wait. */
    }
}   /* End of r_raid1 */

/* ------------------------------------------------------------------------ */

/**
******************************************************************************
**
**  @name   r_raid5
**
**  @brief  To perform a data guarded I/O operation requested via an RRP.
**
**  This procedure initiates the physical I/O for a RAID device that is
**  configured by the system as a "data guarded device". After optimizing the
**  request, the RAID 5 Executive is called upon to actually perform the
**  requested I/O operation.
**
**  @param  sda         - Starting Disk Address
**  @param  sectors     - Number of sectors (I/O length)
**  @param  function    - RRP function code
**  @param  sgl         - SGL pointer
**  @param  strategy    - RRP strategy
**  @param  rdd         - RDD pointer
**  @param  sn          - SN of controller
**  @param  rrp         - RRP pointer
**  @param  ilt         - Primary ILT pointer
**
**  @return none
**
//  PRIMARY ILT USAGE:
//       CALLER AREA                     CALLEE AREA
//       ___________                     ___________
//       W0 = RRP                        W0 = pending count
//       W2 = SN (optional)              W4 = RDD
//                                       W5 = NVA (writes)
***********************************************************************
**/

static void r_raid5(UINT64 sda, UINT32 sectors, UINT16 function,
                    SGL *sgl, UINT32 strategy UNUSED, RDD *rdd,
                    UINT32 sn, RRP *rrp, ILT *ilt)
{
    RRB *new_rrb;
    UINT64 ms;                          /* maxi-stripe. */
    UINT64 mso;                         /* maxi-stripe offset. */
    UINT64 xfer;                        /* sector count of 1st I/O. */
    UINT32 remaining_sectors;
    UINT32 starting_offset;
    SGL *nsgl;
    UINT32 nsglsize;

    /* Check for write operation. */
    if (function != RRP_OUTPUT && function != RRP_OUTPUT_VERIFY)
    {
        /* Clear NVA from primary ILT on read - (NVA used for writes only). */
        ilt->ilt_normal.w5 = 0;             /* Clear NVA from primary ILT. */
    }
    else
    {
        /*
         * Assign and initialize an NVA entry for this write operation. It is critical
         * that both the NVA entry and NVSRAM checksum be written simultaneously while
         * interrupts are disabled with minimum delay after an NVA entry has been
         * assigned. This is done to prevent potential NVSRAM corruption during a
         * power failure. The routine r$anvaw is designed to stall on a power fail
         * condition. The NVA entry will be attached to the primary ILT.
         */
        /* Output is in ((ILT*)ilt)->ilt_normal.w5. */
        M$ap3nva(rrp, ilt);                 /* Assign and initialize NVA entry. */
    }

    /* Initialize primary ILT. */
    /* Calculate RAID 5 maxi stripe number. */
    ilt->ilt_normal.w0 = 1;         /* Init pending count in prime ILT. */
    ms = sda / rdd->spu;
    mso= sda % rdd->spu;
    /* Compute maximum xfer for containment within maxi stripe. */
    xfer = rdd->spu - mso;
    if (sectors <= xfer)
    {
        /*
         * Process simple I/O
         *
         * A simple I/O operation by definition resolves to containment within a
         * single maxi stripe. A single RRB is generated.
         *
         * mso = maxi stripe offset
         * ms = maxi stripe
         */
        new_rrb = get_rrb();                 /* Assign RRB. */
#ifdef M4_DEBUG_RRB
        CT_history_printf("%s%s:%u get_rrb 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, new_rrb);
#endif /* M4_DEBUG_RRB */

        new_rrb->flags = rrp->flags;
        rrp->flags = 0;
        new_rrb->lsda = sda;                /* Set up LSDA. */
        new_rrb->sgl = sgl;
        new_rrb->leda = sda + sectors;
        new_rrb->sglsize = rrp->sglSize | (1 << 31);    /* Set as borrowed. */
        /* Link RRB to RPN. */
        r_insrrb(new_rrb, mso, ms, function, rdd, sn, rrp, ilt);
        return;
    }

    /*
     * Process complex I/O
     *
     * A complex I/O operation by definition resolves to containment within
     * multiple maxi stripes. Two or more RRBs are generated.
     *
     * mso = maxi stripe offset
     * ms = maxi stripe
     * xfer = sector count of 1st I/O
     */
    remaining_sectors = sectors;
    starting_offset = 0;
    sectors = xfer;                     /* Set 1st sector count. */

    /* Generate next RRB. */
    for (;;)
    {
        new_rrb = get_rrb();                 /* Assign RRB. */
#ifdef M4_DEBUG_RRB
        CT_history_printf("%s%s:%u get_rrb 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, new_rrb);
#endif /*  M4_DEBUG_RRB */

        new_rrb->flags = rrp->flags;
        rrp->flags = 0;
        new_rrb->lsda = sda;
        new_rrb->leda = sda + sectors;
        remaining_sectors = remaining_sectors - sectors;

/* Generate SGL. Note that an address of 0xfeedf00d or NULL are both considered NULL. */
/* TODO - the address 0xfeedf00d setting in cachefe.as, checking in raid5.as and dlmbe.as. */
        if ((UINT32)sgl == 0xfeedf00d)
        {
            nsgl = NULL;
        }
        else
        {
            nsgl = sgl;
        }
        if (nsgl == NULL)
        {
            /* Show the SGL Size as zero with the borrowed bit set (so do not free). */
            nsgl = sgl;
            nsglsize = (1 << 31);
        }
        else
        {
            /* Generate new SGL for this request. */
            nsgl = m_gensgl(sectors, starting_offset << 9, sgl);
            nsglsize = nsgl->size;
        }
        /* Link SGL to RRB. */
        new_rrb->sgl = nsgl;
        new_rrb->sglsize = nsglsize;

        /* Link RRB to RPN. */
        r_insrrb(new_rrb, mso, ms, function, rdd, sn, rrp, ilt);

        /* Check for additional RRB required. */
        if (remaining_sectors == 0)
        {
            return;                         /* Exit if no additional RRB required. */
        }
        ilt->ilt_normal.w0 ++;              /* Adjust pending count in prime ILT. */
        mso = 0;                             /* Update maxi stripe offset. */
        ms++;                               /* Update maxi stripe. */
        starting_offset += sectors;                     /* Update starting sector offset. */
        sda += sectors;                     /* Update LSDA. */
        /* Check for final I/O. */
        sectors = (remaining_sectors <= rdd->spu) ? remaining_sectors : rdd->spu;
    }
}                               /* End of r_raid5 */

/* ------------------------------------------------------------------------ */

/**
******************************************************************************
**
**  @name   r_r10concheck
**
**  @brief  To perform a mirrored/striped I/O consistency check operation.
**
**  This procedure initiates the physical I/O for a RAID device that is
**  configured by the system as a "mirrored/striped device". Completion
**  routines are used to handle the completion of the physical I/O and to
**  complete the RRP request by verifying the data.
**
**  @param  sda         - Starting Disk Address
**  @param  sectors     - Number of sectors (I/O length)
**  @param  function    - RRP function code
**  @param  sgl         - SGL pointer
**  @param  strategy    - RRP strategy
**  @param  rdd         - RDD pointer
**  @param  sn          - SN of controller
**  @param  rrp         - RRP pointer
**  @param  ilt         - Primary ILT pointer
**
**  @return none
**
// PRIMARY ILT USAGE:
//      CALLER AREA                     CALLEE AREA
//      -----------                     -----------
//      W0 = RRP                        W0 = pending count
//      W2 = SN (optional)              W4 = RDD
//                                      W5 = NVA (writes only)
// SECONDARY ILT USAGE:
//      CALLER AREA
//      -----------
//      W0 = PRP
//      W1 = Mus tbe NULL
//      W2 =
//      W3 = primary ILT
//      W4 = PSD
//      W5 = associated secondary ILT
//      W6 =
//      W7 =
***********************************************************************
**/

static void r_r10concheck(UINT64 sda, UINT32 sectors, UINT16 function,
                          SGL *sgl UNUSED, UINT32 strategy, RDD *rdd,
                          UINT32 sn, RRP *rrp, ILT *ilt)
{
    PSD        *psd;
    PSD        *firstpsd;
    ILT        *ilt_submission_thread;
    UINT32      mirror_depth;
    UINT32      psd_ordinal;
    UINT64      tmp_r14;
    UINT64      tmp_r4;
    UINT64      tmp_r11;
    UINT64      tmp_r6;
    UINT64      tmp_r7;
    ILT        *new_ilt;
    ILT        *first_ilt;
    PRP        *prp;

    /* Locate starting PSD and offsets. */
    mirror_depth = rdd->depth;  /* Get mirror depth. */
    tmp_r6 = (sda / rdd->sps) * mirror_depth;
    tmp_r7 = tmp_r6 / rdd->psdCnt;
    tmp_r6 = tmp_r6 % rdd->psdCnt;
    psd = rdd->extension.pPSD[tmp_r6];  /* Lookup starting PSD. */
    tmp_r14 = rdd->sps * tmp_r7;        /* Compute base PSDA. */
    tmp_r4 = sda % rdd->sps;    /* Calculate max length I/O for this mirror. */
    tmp_r11 = tmp_r4 + tmp_r14; /* Calculate starting PSDA. */
    /* Determine type of I/O (simple or complex). */
    firstpsd = rdd->extension.pPSD[0];  /* Lookup 1st PSD in unit (rank). */

    /* Complex I/O not handled. */
    if (sectors > (rdd->sps - tmp_r4))
    {
        return;
    }

//  A simple I/O operation by definition resolves to a single set of mirrored
//  physical devices (stripe).
    ilt->ilt_normal.w0 = 0; /* Clear pending count in primary ILT. */
    ilt_submission_thread = 0;          /* Initialize ILT submission thread. */

    /* Validate status of current PSD entry. */
    for (;;)
    {
        if ((psd->status == PSD_REBUILD) &&
            /* If total containment in area that hasn't been rebuilt - next PSD. */
            ((tmp_r11 > psd->rLen) ||
            /* If read operation and if EDA + 1 too big. */
             ((function & 0) && ((sectors + tmp_r11) > psd->rLen))))
        {
            // b .cc30      After the else. I didn't want to invert the above check. */
        }
        else
        {
//  A separate PRP operation is generated for each mirror of this mirrored
//  device. For write operations, a physical write has to occur for each
//  mirror. For read operations, a physical read is issued for each mirror.
//  The first read initiated at the physical layer causes all other associated
//  mirrored reads to be eliminated.

            new_ilt = get_ilt();    /* Allocate an ILT. */
#ifdef M4_DEBUG_ILT
            CT_history_printf("%s%s:%u get_ilt %p\n", FEBEMESSAGE, __FILE__, __LINE__, ilt);
#endif /* M4_DEBUG_ILT */
            prp = get_prp();        /* Assign PRP. */
#ifdef M4_DEBUG_PRP
            CT_history_printf("%s%s:%u get_prp %p\n", FEBEMESSAGE, __FILE__, __LINE__, prp);
#endif /* M4_DEBUG_PRP */
            new_ilt->ilt_normal.w0 = (UINT32)prp;   /* Link PRP to ILT. */

            /* Link to primary ILT. */
            new_ilt->ilt_normal.w3 = (UINT32)ilt;
            /* Set up PSD. */
            new_ilt->ilt_normal.w4 = (UINT32)psd;

            /* Bump pending count in primary ILT. */
            ilt->ilt_normal.w0++;
            /* Link ILT to submission queue. */
            new_ilt->fthd = ilt_submission_thread;
            ilt_submission_thread = new_ilt;

            /* Build the PRP. */
            r_bldprp(prp, psd, tmp_r11, sectors, function, strategy);

            /* Get new SGL and link in. */
            prp->pSGL = m_asglbuf(rrp->sglSize);
            prp->sglSize = rrp->sglSize;
        }
        mirror_depth--;     /* Adjust remaining rirror depth. */
        if (mirror_depth == 0)
        {
            break;          /* if complete. */
        }
        psd = psd->npsd;    /* Link to next PSD. */
        psd_ordinal++;      /* Advance PSD ordinal. */
        if (firstpsd == psd)
        {
            psd_ordinal = 0;        /* Reset PSD ordinal. */
            tmp_r11 += rdd->sps;    /* Adjust current PSDA. */
        }
    }

//  All PRP requests in support of the requested RRP operation have already
//  been entered into the submission queue.
    first_ilt = ilt_submission_thread;      /* Save 1st PRP. */

    /* Queue next read ILT/PRP to physical layer. */
    while (ilt_submission_thread != 0)
    {
        new_ilt = ilt_submission_thread;
        /* Locate next request. */
        ilt_submission_thread = ilt_submission_thread->fthd;
        /* Set up SN. */
        new_ilt->ilt_normal.w2 = sn;
        if (ilt_submission_thread != 0)
        {
            new_ilt->ilt_normal.w5 = (UINT32)ilt_submission_thread;
        }
        else
        {
            new_ilt->ilt_normal.w5 = (UINT32)first_ilt;
        }
        /* Queue request without wait. */
        EnqueueILT((void *)P_que, new_ilt, (void *)&ct_r$r10concomp);
    }
}                               /* End of r_r10concheck */

/* ------------------------------------------------------------------------ */

/**
******************************************************************************
**
**  @name   r_raid10
**
**  @brief  To perform a mirrored/stripped I/O operation requested via RRP.
**
**  This procedure initiates the physical I/O for a RAID device that is
**  configured by the system as a "mirrored/striped device". Completion routines
**  are used to handle the completion of the physical I/O and to complete the
**  RRP request back to the originator (virtual layer).
**
**  @param  sda         - Starting Disk Address
**  @param  sectors     - Number of sectors (I/O length)
**  @param  function    - RRP function code
**  @param  sgl         - SGL pointer
**  @param  strategy    - RRP strategy
**  @param  rdd         - RDD pointer
**  @param  sn          - SN of controller
**  @param  rrp         - RRP pointer
**  @param  ilt         - Primary ILT pointer
**
**  @return none
**
// PRIMARY ILT USAGE:
//      CALLER AREA                     CALLEE AREA
//      -----------                     -----------
//      W0 = RRP                        W0 = pending count
//      W2 = SN (optional)              W4 = RDD
//                                      W5 = NVA (writes only)
// SECONDARY ILT USAGE:
//      CALLER AREA
//      -----------
//      W0 = PRP
//      W1 = associated secondary ILT (optional for reads only)
//      W2 = SN (optional)
//      W3 = primary ILT
//      W4 = PSD
//      W6 = associated secondary ILT (optional for writes only)
//      W7 = inherited status (optional for writes only)
***********************************************************************
**/

static void r_raid10(UINT64 sda, UINT32 sectors, UINT16 function,
                     SGL *sgl, UINT32 strategy, RDD *rdd,
                     UINT32 sn, RRP *rrp, ILT *ilt)
{
    UINT32      readorwrite;
    PSD        *psd;
    PSD        *firstpsd;
    ILT        *ilt_submission_thread;
    UINT32      mirror_depth;
    UINT32      psd_ordinal;
    UINT64      tmp_r14;
    UINT64      tmp_r4;
    UINT64      tmp_r11;
    UINT64      tmp_r6;
    UINT64      tmp_r7;
    ILT        *new_ilt;
    PRP        *prp;
    ILT        *first_ilt;

    /* Check for write operation. */
    if (function == RRP_OUTPUT || function == RRP_OUTPUT_VERIFY)
    {
        readorwrite = TRUE;
    }
    /* Check for consistency scan. */
    else if (function != RRP_CONSISTENCY_CHECK)
    {
        readorwrite = FALSE;
    }
    else
    {
        rrp->sglSize = sectors << 9;    /* Set the byte count. */
        r_r10concheck(sda, sectors, function, sgl, strategy, rdd, sn, rrp, ilt);
        return;
    }

// Assign and initialize an NVA entry for this write operation.
//
//  It is critical that both the NVA entry and NVSRAM checksum be written
//  simultaneously while interrupts are disabled with minimum delay after an
//  NVA entry has been assigned. This is done to prevent potential NVSRAM
//  corruption during a power failure. The routine r$anvaw is designed to
//  stall on a power fail condition. The NVA entry will be attached to the
//  primary ILT.

    /* Locate starting PSD and offsets. */
    mirror_depth = rdd->depth;  /* Get mirror depth. */
    tmp_r6 = mirror_depth * (sda / rdd->sps);   /* Compute stripe number. */
    tmp_r7 = (tmp_r6 / rdd->psdCnt);
    psd_ordinal = (tmp_r6 % rdd->psdCnt);   /* Save PSD offset. */
    psd = rdd->extension.pPSD[psd_ordinal];  /* Lookup starting PSD. */
    tmp_r14 = rdd->sps * tmp_r7;        /* Compute base PSDA. */
    tmp_r4 = sda % rdd->sps;    /* Calculate max length I/O for this mirror. */
    tmp_r11 = tmp_r4 + tmp_r14; /* Calculate starting PSDA. */
    firstpsd = rdd->extension.pPSD[0];  /* Lookup 1st PSD in unit (rank). */

    /* Determine type of I/O (simple or complex) */
    if (sectors <= (rdd->sps - tmp_r4))
    {
// Process simple I/O
//
//  A simple I/O operation by definition resolves to a single set of mirrored
//  physical devices (stripe).
        ilt->ilt_normal.w0 = 0; /* Clear pending count in primary ILT. */
        ilt_submission_thread = 0;      /* Initialize ILT submission thread. */

        /* Validate status of current PSD entry. */
        for (;;)
        {
            if ((psd->status == PSD_REBUILD) &&
                    /* If total containment in area that hasn't been rebuilt - next PSD. */
                    ((tmp_r11 > psd->rLen) ||
                     /* If read operation and if EDA + 1 too big. */
                     ((function & 0x01) && ((sectors + tmp_r11) > psd->rLen)) ||
                     ((~function & 0x01) && (tmp_r11 >= psd->rLen))
                    )
               )
            {
                // b .rg30      After the else. I didn't want to invert the above check. */
            }
            else
            {
//  A separate PRP operation is generated for each mirror of this mirrored
//  device. For write operations, a physical write has to occur for each
//  mirror. For read operations, a physical read is issued for each mirror.
//  The first read initiated at the physical layer causes all other associated
//  mirrored reads to be eliminated.

                new_ilt = get_ilt();    /* Allocate an ILT. */
#ifdef M4_DEBUG_ILT
                CT_history_printf("%s%s:%u get_ilt %p\n", FEBEMESSAGE, __FILE__, __LINE__,
                                  ilt);
#endif /* M4_DEBUG_ILT */
                prp = get_prp();        /* Assign PRP. */
#ifdef M4_DEBUG_PRP
                CT_history_printf("%s%s:%u get_prp %p\n", FEBEMESSAGE, __FILE__, __LINE__,
                                  prp);
#endif /* M4_DEBUG_PRP */
                new_ilt->ilt_normal.w0 = (UINT32)prp;   /* Link PRP to ILT. */

                /* Link to primary ILT. */
                new_ilt->ilt_normal.w3 = (UINT32)ilt;
                /* Set up PSD. */
                new_ilt->ilt_normal.w4 = (UINT32)psd;

                /* Bump pending count in primary ILT. */
                ilt->ilt_normal.w0++;
                /* Link ILT to submission queue. */
                new_ilt->fthd = ilt_submission_thread;
                ilt_submission_thread = new_ilt;

                /* Build the PRP. */
                r_bldprp(prp, psd, tmp_r11, sectors, function, strategy);

                /* Link in original SGL and indicate as borrowed. */
                prp->sglSize = rrp->sglSize | (1 << 31);
                prp->pSGL = sgl;        /* Set up SGL in PRP. */
            }
            // .rg30:
            mirror_depth--;     /* Adjust remaining mirror depth. */
            if (mirror_depth == 0)
            {
                break;          /* If complete. */
            }
            psd = psd->npsd;    /* Link to next PSD. */
            psd_ordinal++;      /* Advance PSD ordinal. */
            if (firstpsd == psd)        /* if wrap. */
            {
                psd_ordinal = 0;        /* Reset PSD ordinal. */
                tmp_r11 += rdd->sps;    /* Adjust current PSDA. */
            }
        }

//  All PRP requests in support of the requested RRP operation have already
//  been entered into the submission queue.

        if (readorwrite != FALSE)
        {
            /* Simple RAID 10 write requests use the RAID 1 write completion routines
             * to minimize overhead. */
            rrp->status = EC_IO_ERR;    /* Initialize RRP status to error. */

            /* Queue next write ILT/PRP to physical layer. */
            while (ilt_submission_thread != 0)
            {
                new_ilt = ilt_submission_thread;
                /* Locate next request. */
                ilt_submission_thread = ilt_submission_thread->fthd;
                /* Queue request without wait. */
                EnqueueILT((void *)P_que, new_ilt, (void *)&ct_r$r1wrcomp);
            }
            return;
        }

        /* Simple RAID 10 read requests use the RAID 1 read completion routines to
         * minimize overhead. */
        first_ilt = ilt_submission_thread;      /* Save 1st PRP. */

//  Although multiple read requests are being queued (one for each mirror),
//  only one read request will survive. The optimization in the physical layer
//  will select the most optimum request to perform. Requests that don't
//  survive will be automatically released back to the system by the physical
//  layer. This includes the secondary ILT and PRP. The SGL is excluded
//  because of its shared usage.
//
//  All associated read requests must be queued simultaneously without
//  blocking to insure that they are processed properly by the physical layer.

        while (ilt_submission_thread != 0)
        {
            new_ilt = ilt_submission_thread;
            /* Locate next request. */
            ilt_submission_thread = ilt_submission_thread->fthd;
            /* Set up SN. */
            new_ilt->ilt_normal.w2 = sn;
            if (ilt_submission_thread == 0)
            {
                /* Set associated link to 1st PRP. */
                new_ilt->ilt_normal.w1 = (UINT32)first_ilt;
            }
            else
            {
                /* Set associated ILT link. */
                new_ilt->ilt_normal.w1 = (UINT32)ilt_submission_thread;
            }
            /* Queue request without wait. */
            EnqueueILT((void *)P_que, new_ilt, (void *)&ct_r$r1rdcomp);
        }
        return;
    }

    UINT32      r10_sglof;      /* Current SGL offset. */
    SGL        *r10_ssgl;
    UINT32      r10_scount;
    UINT64      r10_bcount;     /* Current byte count. */

//  A complex I/O operation by definition resolves to more than one set of
//  mirrored physical devices (multiple stripes).
    r10_sglof = 0;              /* Record SGL offset. */

//  Lock up primary ILT until all requests have been submitted to the physical
//  layer. This prevents the completion of the primary ILT before all of the
//  required I/O has been performed.
    ilt->ilt_normal.w0 = 1;

    /* Initialize for the next set of mirrored devices. */
    for (;;)
    {
        ilt_submission_thread = 0;      /* Initialize ILT submission thread. */
        r10_ssgl = 0;           /* Clear shared SGL. */
        r10_scount = (tmp_r14 + rdd->sps) - tmp_r11;    /* Compute current sector count. */
        r10_scount = (r10_scount > sectors) ? sectors : r10_scount;     /* Check for last segment. */
        r10_bcount = r10_scount << 9;   /* Form current byte count. */

        /* Validate status of current PSD entry. */
        for (;;)
        {
            if ((psd->status == PSD_REBUILD) &&
                    /* If total containment in area that hasn't been rebuilt - next PSD. */
                    ((tmp_r11 > psd->rLen) ||
                     /* If read operation and if EDA + 1 too big. */
                 ((function & 0x01) && ((sectors + tmp_r11) > psd->rLen)) ||
                 ((~function & 0x01) && (tmp_r11 >= psd->rLen))
                )
               )
            {
                // b .rg500     After the else. I didn't want to invert the above check. */
            }
            else
            {
//  A separate PRP operation is generated for each mirror of this mirrored
//  device. For write operations, a physical write has to occur for each
//  mirror. For read operations, a physical read is issued for each mirror.
//  The first read initiated at the physical layer causes all other associated
//  mirrored reads to be eliminated.
                /* Generate ILT/PRP. */
                new_ilt = get_ilt();    /* Allocate an ILT. */
#ifdef M4_DEBUG_ILT
                CT_history_printf("%s%s:%u get_ilt %p\n", FEBEMESSAGE, __FILE__, __LINE__,
                                  ilt);
#endif /* M4_DEBUG_ILT */
                prp = get_prp();        /* Assign PRP. */
#ifdef M4_DEBUG_PRP
                CT_history_printf("%s%s:%u get_prp %p\n", FEBEMESSAGE, __FILE__, __LINE__,
                                  prp);
#endif /* M4_DEBUG_PRP */
                new_ilt->ilt_normal.w0 = (UINT32)prp;   /* Link PRP to ILT. */

                /* Link to primary ILT. */
                new_ilt->ilt_normal.w3 = (UINT32)ilt;
                /* Set up PSD. */
                new_ilt->ilt_normal.w4 = (UINT32)psd;

                /* Bump pending count in primary ILT. */
                ilt->ilt_normal.w0++;
                /* Link ILT to submission queue. */
                new_ilt->fthd = ilt_submission_thread;
                ilt_submission_thread = new_ilt;    /* For next loop. */

                /* Build the PRP. */
                r_bldprp(prp, psd, tmp_r11, r10_scount, function, strategy);

                /* Process SGL. */
                if (function != RRP_VERIFY_CHECKWORD)
                {
                    if (r10_ssgl != 0)
                    {
                        r10_ssgl->owners++;     /* Bump ownership count. */
                    }
                    else
                    {
                        /* Generate new SGL. */
                        r10_ssgl = m_gensgl(r10_scount, r10_sglof, sgl);
                    }
                    prp->sglSize = r10_ssgl->size;      /* Set up SGL size. */
                }
                prp->pSGL = r10_ssgl;   /* Set up shared SGL. */
            }

            psd = psd->npsd;    /* Link to next PSD. */
            psd_ordinal++;      /* Advance PSD ordinal. */
            if (firstpsd == psd)
            {
                tmp_r11 += rdd->sps;    /* Adjust current PSDA. */
                tmp_r14 += rdd->sps;    /* Adjust base PSDA. */
                psd_ordinal = 0;        /* Reset PSD ordinal. */
            }

            mirror_depth--;     /* Adjust remaining mirror depth. */
            if (mirror_depth == 0)
            {
                break;          /* If complete. */
            }
        }

// All PRP requests in support of the requested RRP operation have already
// been entered into the submission queue for the current mirror set.
        if (readorwrite != FALSE)       /* If write requested. */
        {
            first_ilt = ilt_submission_thread;  /* Save 1st PRP. */

            /* Queue next write ILT/PRP to physical layer. */
            while (ilt_submission_thread != 0)
            {
                new_ilt = ilt_submission_thread;
                /* Locate next request. */
                ilt_submission_thread = ilt_submission_thread->fthd;
                /* Initialize inherited status. */
                new_ilt->ilt_normal.w7 = EC_IO_ERR;
                if (ilt_submission_thread == 0)
                {
                    /* Set associated link to 1st PRP. */
                    new_ilt->ilt_normal.w6 = (UINT32)first_ilt;
                }
                else
                {
                    /* Set associated ILT link. */
                    new_ilt->ilt_normal.w6 = (UINT32)ilt_submission_thread;
                }
                /* Queue next write ILT/PRP to physical layer. */
                EnqueueILT((void *)P_que, new_ilt, (void *)&ct_r$r10wrcomp);
            }
        }
        else
        {
// Queue next read ILT/PRP to physical layer
//
//  Although multiple read requests are being queued (one for each mirror),
//  only one read request will survive. The optimization in the physical layer
//  will select the most optimum request to perform. Requests that don't
//  survive will be automatically released back to the system by the physical
//  layer. This includes the secondary ILT and PRP. The SGL is excluded
//  because of shared usage.
//
//  All associated read requests must be queued simultaneously without
//  blocking to insure that they are processed properly by the physical layer.

            first_ilt = ilt_submission_thread;  /* Save 1st PRP. */
            while (ilt_submission_thread != 0)
            {
                new_ilt = ilt_submission_thread;
                /* Locate next request. */
                ilt_submission_thread = ilt_submission_thread->fthd;
                /* Set up SN. */
                new_ilt->ilt_normal.w2 = sn;
                if (ilt_submission_thread == 0)
                {
                    /* Set associated link to 1st PRP. */
                    new_ilt->ilt_normal.w1 = (UINT32)first_ilt;
                }
                else
                {
                    /* Set associated ILT link. */
                    new_ilt->ilt_normal.w1 = (UINT32)ilt_submission_thread;
                }
                /* Queue request without wait. */
                EnqueueILT((void *)P_que, new_ilt, (void *)&ct_r$r10rdcomp);
            }
        }

        /* Advance to next mirror set. */
        sectors -= r10_scount;  /* Compute remaining sectors. */
        if (sectors == 0)       /* If complete. */
        {
            ilt->ilt_normal.w0--;       /* Unlock primary ILT. */
            return;
        }
        r10_sglof += r10_bcount;        /* Adjust SGL offset by current byte count. */
        mirror_depth = rdd->depth;      /* Reload mirror depth. */
        tmp_r11 = tmp_r14;      /* Reset current PSDA. */
    }
}                               /* End of r_raid10 */

/* ------------------------------------------------------------------------ */

/**
******************************************************************************
**
**  @name   r_VLraid
**
**  @brief  To perform a VLink device I/O operation requested via an RRP.
**
**  This procedure initiates the physical I/O for a RAID device that is
**  configured by the system as a "VLink device". Completion routines are
**  used to handle the completion of the physical I/O and to complete the RRP
**  request back to the originator (virtual layer).
**
**  @param  sda         - Starting Disk Address
**  @param  sectors     - Number of sectors (I/O length)
**  @param  function    - RRP function code
**  @param  sgl         - SGL pointer
**  @param  strategy    - RRP strategy
**  @param  rdd         - RDD pointer
**  @param  sn          - SN of controller
**  @param  rrp         - RRP pointer
**  @param  ilt         - Primary ILT pointer
**
**  @return none
**
***********************************************************************
**/

static void r_VLraid(UINT64 sda, UINT32 sectors, UINT16 function, SGL *sgl,
                     UINT32 strategy UNUSED, RDD *rdd, UINT32 sn UNUSED, RRP *rrp, ILT *ilt)
{
    DLM$VLraid(sda, function, sgl, sectors, rdd, rrp, ilt);
}                               /* End of r_VLraid */

/* ------------------------------------------------------------------------ */

/* Complete rrp with passed in status. */
static void r_ret_status(UINT32 status, RRP *rrp, ILT *ilt)
{
    rrp->status = status;
    record_raid(FR_RRP_COMPLETE, rrp, status);
    K_comp(ilt);
}                               /* End of r_ret_status */

/**
******************************************************************************
**
**  @name   r_exec
**
**  @brief  To process RRP requests which are queued to R_exec_qu.
**
**  The queuing routine R$que deposits an RRP request into the queue and
**  activates this executive process if necessary. This executive extracts the
**  next RRP request from the queue and initiates the requested I/O operation.
**  A series of completion routines are used to handle additional processing
**  of this request as required based on the particular RAID algorithm.
**
**  @param  none
**
**  @return none
**
***********************************************************************
**/

NORETURN
void r_exec(void)
{
    ILT        *ilt;
    ILT        *next_ilt;
    RRP        *rrp;

    K$qxchang();        /* Exchange processes. */

    for (;;)
    {
        if (R_exec_qu.head == 0)
        {
            /* Set this process to not ready. */
            TaskSetState(R_exec_qu.pcb, PCB_NOT_READY);
            K$qxchang();        /* Exchange processes. */
            continue;
        }

        /* Get next queued request. */
        ilt = R_exec_qu.head;   /* Isolate next queued ILT. */
        next_ilt = ilt->fthd;   /* Get the next ILT on the list. */

        /* Remove this request from queue. */
        R_exec_qu.qcnt--;       /* Decrement the queue count. */
        if (next_ilt == 0)
        {
            R_exec_qu.head = 0; /* No next entry. */
            R_exec_qu.tail = 0; /* No tail entry either. */
        }
        else
        {
            R_exec_qu.head = next_ilt;  /* Dequeue this ILT entry. */
            /* No need to change the tail. */
            /* Need to update the backwards pointer -- note, yes this is ugly! */
            next_ilt->bthd = (ILT *) & R_exec_qu;
        }

        /* Get RRP request. */
        rrp = (RRP *) ((ilt - 1)->ilt_normal.w0);
        record_raid(FR_RRP_EXEC, rrp, 0);       /* Flight recorder */

        UINT32      length = rrp->length;
        UINT64      rsda = rrp->startDiskAddr;
        SGL        *sgl = rrp->pSGL;
        UINT32      secperstripe;

        /* Validate RAID ID. */
        if (MAX_RAIDS <= rrp->rid || R_rddindx[rrp->rid] == 0)
        {
            /* Set non-existent device. */
            r_ret_status(EC_NONX_DEV, rrp, ilt);        /* update rrp status and complete request. */
            continue;
        }

        /* If strategy is invalid. */
        if (RRP_HIGH < rrp->strategy)
        {
            r_ret_status(EC_INV_STRAT, rrp, ilt);       /* update rrp status and complete request. */
            continue;
        }
#if 0
        /*
        ** This is the fix for ME-203 & ME-171 to prevent a deadlock condition
        ** between the BE & FE when the RAID is locked for rebuild. But while
        ** working on ME-210 & ME-207, found out that the RAID is actually getting
        ** into rebuild due to wrong reasons. The FIX for ME-210 should prevent
        ** the RAIDs from getting into rebld state when not needed.
        ** Just commenting and leaving the code in place as we could potentially
        ** use a similar logic to address some of the 4K BE issues.
        */
        /* Wait if raid is processing an error. */
        if (R_errlock != FALSE)
        {
            r_ret_status(EC_RETRYRC, rrp, ilt);       /* update rrp status and complete request. */
            continue;
        }
#endif   /* 0 */

        UINT16      func = rrp->function;       /* Get function code. */
        RDD        *rdd = R_rddindx[rrp->rid];

        /* Validate function code. */
        if (func == RRP_REBUILD)
        {
            BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
            rdd->wReq++;        /* Adjust write request count. */
        }
        else if (func == RRP_REBUILD_CHECK)
        {

            if (rdd->type != RD_RAID5)
            {
                /* Set OK - Rebuild Check on VDisk may have multiple types of RAIDs due to
                 * expansions or mirroring to other VDisks */
                r_ret_status(EC_OK, rrp, ilt);  /* update rrp status and complete request. */
                continue;
            }

//  Rebuilds are done on Parity Stripes, so need to change the incoming address
//      range to match up with a full parity stripe so that the overlap and
//      precedence checking done later will catch all of the rebuild.
//
//      NOTE: Do not have to worry about Sector Count overflow since Host Ops
//          lengths are 2 Bytes. A Write Cache op is MAX_SGL_ENTRIES (8) times
//          2 bytes which is still less than 4 bytes.
//
//      Note: Sectors per stripe is always binary = 8,16,32,64,128,256, or 512

            /* Create a mask for rounding to stripe size. */
            secperstripe = rdd->sps - 1;

            /* New sector count, with additional to read. */
            length = (length + (rsda & secperstripe) + secperstripe) & ~secperstripe;

            /*  New SDA (beginning of Stripe). */
            rsda = rsda & ~secperstripe;
            BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
            rdd->wReq++;        /* Adjust write request count. */
        }
        else
        {
            if (func == RRP_CONSISTENCY_CHECK || func == RRP_PARITY_CHECK)
            {
                if (rdd->type != RD_RAID5 && rdd->type != RD_RAID10)
                {
                    r_ret_status(EC_INV_FUNC, rrp, ilt);        /* update rrp status and complete request. */
                    continue;
                }
                BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
                rdd->rReq++;    /* Adjust read request count. */
            }
            else
            {
                if (sgl == 0)   /* If SGL is null */
                {
                    r_ret_status(EC_NULL_SGL, rrp, ilt);    /* update rrp status and complete request. */
                    continue;
                }
                BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
                if (func == RRP_INPUT || func == RRP_VERIFY_CHECKWORD ||
                    func == RRP_VERIFY_DATA)
                {
                    rdd->rReq++;        /* Adjust read request count. */
                }
                else if (func == RRP_OUTPUT || func == RRP_OUTPUT_VERIFY)
                {
                    rdd->wReq++;        /* Adjust write request count. */
                }
                else
                {
                    r_ret_status(EC_INV_FUNC, rrp, ilt);        /* update rrp status and complete request. */
                    continue;
                }
            }
        }

        /* Wait if raid is processing an error. */
        while (R_errlock != FALSE)
        {
fprintf(stderr, "r_exec - Waiting for R_errlock to become zero\n");
            TaskSetMyState(PCB_WAIT_RAID_ERROR);        /* Set this process in RAID error wait. */
            TaskSwitch();       /* Give up control. */
        }

        /* Check device status. */
        if (rdd->status == RD_INOP)
        {
            r_ret_status(EC_INOP_VDEV, rrp, ilt);       /* update rrp status and complete request. */
            continue;
        }
        if (RD_UNINIT == rdd->status)
        {
            r_ret_status(EC_UNINIT_DEV, rrp, ilt);      /* update rrp status and complete request. */
            continue;
        }

        /* Validate disk addresses. */
        if (rsda >= rdd->devCap)
        {
            r_ret_status(EC_INV_SDA, rrp, ilt); /* update rrp status and complete request. */
            continue;
        }
        if ((rsda + length) > rdd->devCap)
        {
            r_ret_status(EC_IN_VDA, rrp, ilt);  /* update rrp status and complete request. */
            continue;
        }

        /* Check for a lock condition that would cause us to wait until either a
         * rebuild or a defragmentation pass completes. */
        for (;;)
        {
            if (rdd->lleda == 0 || rsda >= rdd->lleda || rsda + length <= rdd->llsda)
            {
                if (rdd->defLock == 0)
                {
                    break;
                }
            }

            /* Place process in block lockout wait. */
            TaskSetMyState(PCB_WAIT_BLK_LOCK);
            TaskSwitch();
        }

/*
   Determine if writes are allowed at this time (could be in the middle of of
   a Local Image Update and lose data if this controller fails before the
   errors make it to the other controller or powered off and power on with
   error not known on power up).
 */
        if (func == RRP_OUTPUT || func == RRP_OUTPUT_VERIFY ||
            func == RRP_REBUILD_CHECK)
        {
            if (rdd->aStatus & (1 << RD_A_LOCAL_IMAGE_IP))
            {
                if (func == RRP_REBUILD_CHECK)
                {
                    r_ret_status(EC_DEV_RESERVED, rrp, ilt);    /* update rrp status and complete request. */
                    continue;
                }
                r_ret_status(EC_RETRY, rrp, ilt);       /* update rrp status and complete request. */
                continue;
            }
        }
        /* Adjust outstanding request count. */
        R_orc++;

        BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        /* Update RDD statistics. */
        rdd->qd++;              /* Bump queue depth. */
        rdd->sprCnt++;          /* Bump sample period request count. */
        rdd->spsCnt += length;  /* Bump sample period request count. */

        if (rdd->type == RD_SLINKDEV)   /* If a snapshot request. */
        {
            r_ret_status(EC_RETRY, rrp, ilt);   /* update rrp status and complete request. */
            continue;
        }

        /* Process request by RAID level. */
        rrp->status = EC_OK;    /* Set raid status to ok. */
        ilt->ilt_normal.w4 = (UINT32)rdd;       /* Link RDD to primary ILT. */

        typedef void (*RAID_func)(UINT64 sda, UINT32 sectors, UINT16 function,
                      SGL *, UINT32 strategy, RDD *, UINT32 sn, RRP *, ILT *);
        static const RAID_func  raid_funcs[] =
        {
            [RD_STD]    = &r_std,
            [RD_RAID0]  = &r_raid0,
            [RD_RAID1]  = &r_raid1,
            [RD_RAID5]  = &r_raid5,
            [RD_RAID10] = &r_raid10,
            [RD_LINKDEV]    = &r_VLraid,
//          [RD_SLINKDEV]     /* Checked and error return just above. */
        };
        RAID_func   r_func = NULL;

        if (rdd->type < dimension_of(raid_funcs))
        {
            r_func = raid_funcs[rdd->type];
        }
        if (r_func)
        {
            r_func(rsda, length, func, sgl, rrp->strategy, rdd, -1, rrp, ilt);
        }
    }
}                               /* End of r_exec */


/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
