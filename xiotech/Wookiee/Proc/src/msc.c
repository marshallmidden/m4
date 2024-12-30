/* $Id: msc.c 146064 2010-08-20 21:15:33Z m4 $ */
/**
******************************************************************************
**
**  @file       misc.c
**
**  @brief      Miscellaneous functions
**
**  Copyright (c) 2003-2009 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/

#include <sys/cdefs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "system.h"
#include "misc.h"
#include "CT_defines.h"
#include "ddr.h"
#include "defbe.h"
#include "def_lun.h"

#include "options.h"
#include "XIO_Const.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"
#include "XIO_Types.h"
#include "MR_Defs.h"
#include "LOG_Defs.h"
#include "LL_LinuxLinkLayer.h"
#include "rrp.h"
#include "prp.h"
#include "ecodes.h"

/*
******************************************************************************
** Private variables
******************************************************************************
*/

#ifdef FRONTEND
#define PRIVATE_SIZE    MAKE_DEFS_FE_ONLY_SIZE
#endif  /* FRONTEND */
#ifdef BACKEND
#define PRIVATE_SIZE    MAKE_DEFS_BE_ONLY_SIZE
#endif  /* BACKEND */

#ifdef DEBUG_FLIGHTREC
extern UINT32 fr_parm[4];
#endif  /* DEBUG_FLIGHTREC */

/* ATTENTION -- This is lock-step with ddr.def. */
static const char *ddr_fr_id[] =
{
    "fltRec  ",
    "fltRecTS",
    "mrpTrace",
    "defragTr",
    "etRegs  ",
    "etIRAM  ",
    "etNMICts",
    "etIRegs ",
    "procK_ii",
    "iTrace0 ",
    "iTrace1 ",
    "iTrace2 ",
    "iTrace3 ",
    "trace0  ",
    "trace1  ",
    "trace2  ",
    "trace3  ",
    "physEQ  ",
    "raidEQ  ",
    "raid5EQ ",
    "virtEQ  ",
    "defineEQ",
    "rinitEQ ",
    "xorcomEQ",
    "xorXEQ  ",
    "ispReqQ0",
    "ispReqQ1",
    "ispReqQ2",
    "ispReqQ3",
    "ispRspQ0",
    "ispRspQ1",
    "ispRspQ2",
    "ispRspQ3",
    "DiagNVRM",
    "rderr EQ",
    "rderrPCB",
    "physCQ  ",
    "physCPCB",
    "feIRAM  ",
    "beIRAM  ",
    "physPCB ",
    "raidPCB ",
    "raid5PCB",
    "virtPCB ",
    "definPCB",
    "rinitPCB",
    "xorCPCB ",
    "xorXPCB ",
    "fsysEQ  ",
    "fsysPCB ",
    "bktrNVRM",
    "FICB    ",
    "profile ",
    "isp0atio",
    "isp1atio",
    "isp2atio",
    "isp3atio",
};

#ifdef BACKEND
static struct DMC *DMC_CCB = (struct DMC *)CCB_DMC_BASE_ADDR;
#endif  /* BACKEND */

/*
******************************************************************************
** Public variables
******************************************************************************
*/
UINT64 DMC_bits = 0;        /* Bit table for enum CCB_DMC_enum. */

/*
******************************************************************************
** Public function prototypes - defined in other files
******************************************************************************
*/
extern void LL_SendPacket(void *pReq, UINT32 reqSize, UINT32 cmdCode,
                          void *pRsp, UINT32 rspLen, void *pCallback,
                          UINT32 param);
extern void M_addDDRentry(UINT32, void *, UINT32);
extern char local_memory_start;                 /* Start of private memory. */

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
#ifdef DEBUG_FLIGHTREC
void   M$flight_rec(void);
#endif  /* DEBUG_FLIGHTREC */


/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
*****************************************************************************
**
**  @brief  Checks if the WWN specified is a XIOtech Controller.
**
**  Determines if the WWN supplied by the calling routine is considered
**  another XIOtech controller.
**
**  @param  wwn - WWN to check if it is a XIOtech controller.
**
**  @return 0 if not XIOtech Controller, 1 for MAGNITUDE, 2 for Thunderbolt.
**
*****************************************************************************
**/

// XIOtech Controller Name Format:
//
//      0x2000OOOOOOsssssh  = MAGNITUDE (pre 7.5 version)
//      0x2a0cOOOOOOsssss0  = MAGNITUDE (7.5 and beyond)
//      0x2aecOOOOOOssssss  = Thunderbolt
//
//      where:
//              OOOOOO = IEEE registered WWN (XIOOUI)
//              ssssss = MAGNITUDE/Thunderbolt serial number
//              a      =    0 (Node ID)
//                          1 (Port ID)
//              e      =    0 (MAGNITUDE)
//                          2 (FE Target port)
//                          6 (FE Control port)
//                          A (BE port)
//              c      =    MAGNITUDE
//                              Cluster Number for Node ID
//                              HAB Number for Port ID
//                          Thunderbolt
//                              0 for Control Node ID, and Backend Node ID
//                              Channel Number for Control Port ID, Target Node
//                                  ID, and Target Port ID

UINT32 M_chk4XIO(UINT64 wwn)
{
    wwn &= 0x000000ffffff00f0ULL;       /* Name mask */
    if (wwn != 0x000000b2d0000020ULL)   /* Node name compare value */
    {
        return 0;                       /* Not a XIOtech controller. */
    }

    wwn &= 0x000000000000e000ULL;       /* Mask for MAGNITUDE vs Thunderbolt */
    if (wwn != 0)
    {
        return 2;                       /* If Thunderbolt. */
    }
    return 1;                           /* MAGNITUDE */
}   /* End of M_chk4XIO */

/**
******************************************************************************
**
**  @brief      Convert the given IP Mask to IP Prefix
**
**  @param      UINT32 mask
**
**  @return     UINT8 prefix
**
******************************************************************************
**/
UINT8 MSC_Mask2Prefix(UINT32 mask)
{
    UINT8 i = 0;
    UINT8 prefix = 32;

    for(i = 0; i < 32; i++)
    {
        if(BIT_TEST(mask, i))
        {
            break;
        }
        prefix--;
    }
    return (prefix);
}

#ifdef DEBUG_FLIGHTREC
/**
******************************************************************************
**
**  @brief      Flight Recorder
**
**              The flight recorder saves 4 words of user data on a
**              circular queue.
**
**  @param      param1 - parm 0 of the flight recorder
**  @param      param2 - parm 1 of the flight recorder
**  @param      param3 - parm 2 of the flight recorder
**  @param      param4 - parm 3 of the flight recorder
**
**  @return     none
**
******************************************************************************
**/
void MSC_FlightRec(UINT32 parm0, UINT32 parm1, UINT32 parm2, UINT32 parm3)
{
    fr_parm[0] = parm0;
    fr_parm[1] = parm1;
    fr_parm[2] = parm2;
    fr_parm[3] = parm3;
    M$flight_rec();
}

#endif  /* DEBUG_FLIGHTREC */

/**
***********************************************************************
**
**  @brief  Send a log message, with buffer on stack, and copy to MRP.
**
**      To provide a means of sending a short log message to the CCB,
**      where the message is on the stack. As the message is short, it
**      is copied to the MRP remaining area and thus the original data
**      is no longer needed.
**
**  @param  pLogEntry - location of message buffer on the stack.
**  @param  size      - Total size message buffer (checked short).
**
**  @return none:
**
***********************************************************************
**/
void MSC_LogMessageStack(void *pLogEntry, UINT32 size)
{
    LOG_HEADER_PKT *pkt = (LOG_HEADER_PKT *)pLogEntry;
    UINT32 function;
    UINT32 save_g0 = g0;            /* Routine get_ilt(), get_vrp() */
    UINT32 save_g1 = g1;            /* uses g0 as return, g1 for ILT, */
    UINT32 save_g2 = g2;            /* and g2 for VRP. */

#ifndef PERF
    if (size > VRPAVAILABLE) {
        fprintf(stderr, "%s: log message too long (%u)!\n", __func__, size);
        abort();
    }
#endif  /* !PERF */

#ifdef BACKEND
    /*
     * Process any updates to DMC data structures that CCB might need for
     * complete processing of this request by old PI data Cache layer.
     */
    if (LOG_NotDebug(pkt->event))
    {
        Process_DMC();              /* Process possible DMC requests. */
    }
#endif  /* BACKEND */

    /* Set the length of the logmessage data (exclude header). */
    pkt->length = size - sizeof(LOG_HEADER_PKT);

#ifdef FRONTEND
    function = MRLOGFE;             /* FE log message */
#else
    function = MRLOGBE;             /* BE log message */
#endif
    LL_SendPacket(pLogEntry, size, function, 0, 0, 0, 0);

    g0 = save_g0;
    g1 = save_g1;
    g2 = save_g2;
}   /* End of MSC_LogMessageStack */

/**
 ***********************************************************************
 **
 **  @brief  Send a log message, with buffer on stack, and copy to MRP.
 **
 **      To provide a means of sending a short log message to the CCB,
 **      where the message is on the stack. As the message is short, it
 **      is copied to the MRP remaining area and thus the original data
 **      is no longer needed.
 **
 **  @param  pLogEntry - location of message buffer on the stack.
 **  @param  size      - Total size message buffer (checked short).
 **
 **  @return none:
 **
 ***********************************************************************
 **/
#if SW_FAULT_DEBUG
NORETURN
#endif
void MSC_SoftFault(void *pLogEntry)
{
    UINT8   tmp_message[VRPAVAILABLE];
    LOG_FIRMWARE_ALERT_PKT *pkt = (LOG_FIRMWARE_ALERT_PKT *)pLogEntry;
    UINT32 function;
    UINT32 size;

    size = pkt->header.length + sizeof(LOG_HEADER_PKT);
#ifndef PERF
    if (size > VRPAVAILABLE) {
        fprintf(stderr, "%s: log message too long (%u)!\n", __func__, size);
        abort();
    }
#endif  /* !PERF */
    memcpy(tmp_message,pLogEntry,size);
    pkt = (LOG_FIRMWARE_ALERT_PKT *)tmp_message;

    /* Set the length of the logmessage data (exclude header). */
    pkt->header.length = size - sizeof(LOG_HEADER_PKT);
    pkt->header.event = LOG_FIRMWARE_ALERT;

#ifdef FRONTEND
    function = MRLOGFE;                 /* FE log message */
#else
    function = MRLOGBE;                 /* BE log message */
#endif
    LL_SendPacket(pkt, size, function, 0, 0, 0, pkt->data.errorCode);

#if SW_FAULT_DEBUG
/* During development, Error Trap when the Debug Flag is set */
fprintf(stderr, "MSC_SoftFault! Sleep 3 seconds...\n");
    TaskSleepMS(3000);
fprintf(stderr, "MSC_SoftFault! Aborting...\n");
    abort();
#endif
}   /* End of MSC_SoftwareFault */

UINT32  MSCCRCTABLE[256] =
{
    0x00000000L, 0x77073096L, 0xEE0E612CL, 0x990951BAL,
    0x076DC419L, 0x706AF48FL, 0xE963A535L, 0x9E6495A3L,
    0x0EDB8832L, 0x79DCB8A4L, 0xE0D5E91EL, 0x97D2D988L,
    0x09B64C2BL, 0x7EB17CBDL, 0xE7B82D07L, 0x90BF1D91L,
    0x1DB71064L, 0x6AB020F2L, 0xF3B97148L, 0x84BE41DEL,
    0x1ADAD47DL, 0x6DDDE4EBL, 0xF4D4B551L, 0x83D385C7L,
    0x136C9856L, 0x646BA8C0L, 0xFD62F97AL, 0x8A65C9ECL,
    0x14015C4FL, 0x63066CD9L, 0xFA0F3D63L, 0x8D080DF5L,
    0x3B6E20C8L, 0x4C69105EL, 0xD56041E4L, 0xA2677172L,
    0x3C03E4D1L, 0x4B04D447L, 0xD20D85FDL, 0xA50AB56BL,
    0x35B5A8FAL, 0x42B2986CL, 0xDBBBC9D6L, 0xACBCF940L,
    0x32D86CE3L, 0x45DF5C75L, 0xDCD60DCFL, 0xABD13D59L,
    0x26D930ACL, 0x51DE003AL, 0xC8D75180L, 0xBFD06116L,
    0x21B4F4B5L, 0x56B3C423L, 0xCFBA9599L, 0xB8BDA50FL,
    0x2802B89EL, 0x5F058808L, 0xC60CD9B2L, 0xB10BE924L,
    0x2F6F7C87L, 0x58684C11L, 0xC1611DABL, 0xB6662D3DL,
    0x76DC4190L, 0x01DB7106L, 0x98D220BCL, 0xEFD5102AL,
    0x71B18589L, 0x06B6B51FL, 0x9FBFE4A5L, 0xE8B8D433L,
    0x7807C9A2L, 0x0F00F934L, 0x9609A88EL, 0xE10E9818L,
    0x7F6A0DBBL, 0x086D3D2DL, 0x91646C97L, 0xE6635C01L,
    0x6B6B51F4L, 0x1C6C6162L, 0x856530D8L, 0xF262004EL,
    0x6C0695EDL, 0x1B01A57BL, 0x8208F4C1L, 0xF50FC457L,
    0x65B0D9C6L, 0x12B7E950L, 0x8BBEB8EAL, 0xFCB9887CL,
    0x62DD1DDFL, 0x15DA2D49L, 0x8CD37CF3L, 0xFBD44C65L,
    0x4DB26158L, 0x3AB551CEL, 0xA3BC0074L, 0xD4BB30E2L,
    0x4ADFA541L, 0x3DD895D7L, 0xA4D1C46DL, 0xD3D6F4FBL,
    0x4369E96AL, 0x346ED9FCL, 0xAD678846L, 0xDA60B8D0L,
    0x44042D73L, 0x33031DE5L, 0xAA0A4C5FL, 0xDD0D7CC9L,
    0x5005713CL, 0x270241AAL, 0xBE0B1010L, 0xC90C2086L,
    0x5768B525L, 0x206F85B3L, 0xB966D409L, 0xCE61E49FL,
    0x5EDEF90EL, 0x29D9C998L, 0xB0D09822L, 0xC7D7A8B4L,
    0x59B33D17L, 0x2EB40D81L, 0xB7BD5C3BL, 0xC0BA6CADL,
    0xEDB88320L, 0x9ABFB3B6L, 0x03B6E20CL, 0x74B1D29AL,
    0xEAD54739L, 0x9DD277AFL, 0x04DB2615L, 0x73DC1683L,
    0xE3630B12L, 0x94643B84L, 0x0D6D6A3EL, 0x7A6A5AA8L,
    0xE40ECF0BL, 0x9309FF9DL, 0x0A00AE27L, 0x7D079EB1L,
    0xF00F9344L, 0x8708A3D2L, 0x1E01F268L, 0x6906C2FEL,
    0xF762575DL, 0x806567CBL, 0x196C3671L, 0x6E6B06E7L,
    0xFED41B76L, 0x89D32BE0L, 0x10DA7A5AL, 0x67DD4ACCL,
    0xF9B9DF6FL, 0x8EBEEFF9L, 0x17B7BE43L, 0x60B08ED5L,
    0xD6D6A3E8L, 0xA1D1937EL, 0x38D8C2C4L, 0x4FDFF252L,
    0xD1BB67F1L, 0xA6BC5767L, 0x3FB506DDL, 0x48B2364BL,
    0xD80D2BDAL, 0xAF0A1B4CL, 0x36034AF6L, 0x41047A60L,
    0xDF60EFC3L, 0xA867DF55L, 0x316E8EEFL, 0x4669BE79L,
    0xCB61B38CL, 0xBC66831AL, 0x256FD2A0L, 0x5268E236L,
    0xCC0C7795L, 0xBB0B4703L, 0x220216B9L, 0x5505262FL,
    0xC5BA3BBEL, 0xB2BD0B28L, 0x2BB45A92L, 0x5CB36A04L,
    0xC2D7FFA7L, 0xB5D0CF31L, 0x2CD99E8BL, 0x5BDEAE1DL,
    0x9B64C2B0L, 0xEC63F226L, 0x756AA39CL, 0x026D930AL,
    0x9C0906A9L, 0xEB0E363FL, 0x72076785L, 0x05005713L,
    0x95BF4A82L, 0xE2B87A14L, 0x7BB12BAEL, 0x0CB61B38L,
    0x92D28E9BL, 0xE5D5BE0DL, 0x7CDCEFB7L, 0x0BDBDF21L,
    0x86D3D2D4L, 0xF1D4E242L, 0x68DDB3F8L, 0x1FDA836EL,
    0x81BE16CDL, 0xF6B9265BL, 0x6FB077E1L, 0x18B74777L,
    0x88085AE6L, 0xFF0F6A70L, 0x66063BCAL, 0x11010B5CL,
    0x8F659EFFL, 0xF862AE69L, 0x616BFFD3L, 0x166CCF45L,
    0xA00AE278L, 0xD70DD2EEL, 0x4E048354L, 0x3903B3C2L,
    0xA7672661L, 0xD06016F7L, 0x4969474DL, 0x3E6E77DBL,
    0xAED16A4AL, 0xD9D65ADCL, 0x40DF0B66L, 0x37D83BF0L,
    0xA9BCAE53L, 0xDEBB9EC5L, 0x47B2CF7FL, 0x30B5FFE9L,
    0xBDBDF21CL, 0xCABAC28AL, 0x53B39330L, 0x24B4A3A6L,
    0xBAD03605L, 0xCDD70693L, 0x54DE5729L, 0x23D967BFL,
    0xB3667A2EL, 0xC4614AB8L, 0x5D681B02L, 0x2A6F2B94L,
    0xB40BBE37L, 0xC30C8EA1L, 0x5A05DF1BL, 0x2D02EF8DL
};

UINT32 MSC_CRC32(void * ptr, UINT32 length)
{
    UINT32 crc32 ,i;
    UINT8 * data = ptr;
    crc32 = 0xFFFFFFFF;

    for (i = 0; i < length; i++)
    {
        crc32 = MSCCRCTABLE[(crc32 ^ data[i]) & 0x0ff] ^ (crc32 >> 8);
    }
    return crc32 ^ 0xFFFFFFFF;
}

/* ------------------------------------------------------------------------ */
#ifndef PERF
void print_prp(PRP *prp)
{
    fprintf(stderr, "PRP at %p\n", prp);
    fprintf(stderr, "    func=%d timeoutCnt=%d strategy=%d logflags=0x%02x channel=0x%02x thunked=%d\n",
            prp->func, prp->timeoutCnt, prp->strategy, prp->logflags, prp->channel, prp->thunked);
    fprintf(stderr, "    lun=0x%04x id=0x%08x pDev=%p sda=%lld eda=%lld pSGL=%p sglSize=0x%08x\n",
            prp->lun, prp->id, prp->pDev, prp->sda, prp->eda, prp-> pSGL, prp->sglSize);
    fprintf(stderr, "    timeout=%d rqBytes=%d reqSenseBytes=0x%02x qLogicStatus=0x%02x reqStatus=0x%02x\n",
            prp->timeout, prp->rqBytes, prp->reqSenseBytes, prp->qLogicStatus, prp->reqStatus);
    fprintf(stderr, "    scsiStatus=0x%02x rsvd2=%d cBytes=%d flags=0x%02x retry=%d\n",
                prp->scsiStatus, prp->rsvd2, prp->cBytes, prp->flags, prp->retry);
}   /* End of print_prp */

/* ------------------------------------------------------------------------ */
void print_rrp(RRP *rrp)
{
    fprintf(stderr, "RRP at %p\n", rrp);
    fprintf(stderr, "    function=%d strategy=%d status=0x%02x rid=%d  options=0x%02x\n",
            rrp->function, rrp->strategy, rrp->status, rrp->rid, rrp->options);
    fprintf(stderr, "    length=%d flags=0x%08x startDiskAddr=%lld pSGL=%p sglSize=%d\n",
            rrp->length, rrp->flags, rrp->startDiskAddr, rrp->pSGL, rrp->sglSize);
}   /* End of print_rrp */

/* ------------------------------------------------------------------------ */
/* Similar to .gdbinit macro i_find_ilt_base. */
static void i_find_ilt_base(ILT *p_cilt, int *cnt, ILT **ilt)
{
    /*  This finds base of ilt by subtracting 0x34 till bottom 6 bits are zero (but max of 7/11). */
    *cnt = 0;
    *ilt = p_cilt;

    /* Now, we know that it is 64+32 byte aligned. (Good luck figuring that out.) */
    while (((((UINT32)*ilt) & 63) != 32) && (*cnt < ILTNEST))
    {
        *ilt = *ilt - 1;
        *cnt = *cnt + 1;
    }
}   /* End of i_find_ilt_base */

/* ------------------------------------------------------------------------ */
/* Similar to .gdbinit macro print_nonzero_ilt_vrp. */
void print_nonzero_ilt(ILT *p_cilt)
{
    int     i_cnt;
    int     p_cnt;
    UINT32 *p;
    ILT    *p_ilt;

    i_find_ilt_base(p_cilt, &p_cnt, &p_ilt);

    if (p_cnt >= ILTNEST)
    {
        fprintf(stderr, "i_print_nonzero_ilt - Could not calculate base of ILT %p\n", p_cilt);
        return;
    }
    for (i_cnt = 0; i_cnt < ILTNEST; i_cnt++, p_ilt++)
    {
        fprintf(stderr, "%cILT level %d at %p\n", (p_ilt == p_cilt) ? '*' : ' ', i_cnt, p_ilt);
        p = (UINT32*)p_ilt;
        if (*(p + 0) != 0 || *(p + 1) != 0 || *(p + 2) != 0 || *(p + 3) != 0 ||
            *(p + 4) != 0 || *(p + 5) != 0 || *(p + 6) != 0 || *(p + 7) != 0 ||
            *(p + 8) != 0 || *(p + 9) != 0 || *(p + 10) != 0 || *(p + 11) != 0 ||
            *(p + 12) != 0)
        {
            fprintf(stderr, "    fthd=%p bthd=%p misc=0x%08x linux_val=0x%08x\n",
                p_ilt->fthd, p_ilt->bthd, p_ilt->misc, p_ilt->linux_val);
            fprintf(stderr, "    w0=0x%08x   w1=0x%08x   w2=0x%08x   w3=0x%08x\n",
                p_ilt->ilt_normal.w0, p_ilt->ilt_normal.w1, p_ilt->ilt_normal.w2, p_ilt->ilt_normal.w3);
            fprintf(stderr, "    w4=0x%08x   w5=0x%08x   w6=0x%08x   w7=0x%08x\n",
                p_ilt->ilt_normal.w4, p_ilt->ilt_normal.w5, p_ilt->ilt_normal.w6, p_ilt->ilt_normal.w7);
            fprintf(stderr, "    cr=%p\n", p_ilt->cr);
        }
    }
}   /* End of print_nonzero_ilt */

#endif  /* !PERF */

/**
******************************************************************************
**
**  @brief  Add an entry into the Debug Data Retrieval (DDR) table.
**
**  @param  entry_number    - DDR entry number
**  @param  debug_addr      - Address of debug data
**  @param  length          - Length of debug data
**
**  @return None
**
******************************************************************************
**/
void M_addDDRentry(UINT32 entry_number, void *debug_addr, UINT32 length)
{
    PROC_DDR_TABLE *ddr_table = (PROC_DDR_TABLE *)DDR_TABLE_ADDR;
    PROC_DDR_ENTRY *entry = &ddr_table->entry[entry_number];
#ifdef FRONTEND
    UINT32 ID = 0x52444446;             /* Ascii 'FDDR' */
#else   /* FRONTEND */
    UINT32 ID = 0x52444442;             /* Ascii 'BDDR' */
#endif  /* FRONTEND */

    /* Check if DDR header entry has been initialized */
    if (ddr_table->tableId != ID)
    {
        /* Initialize the DDR header entry */
        ddr_table->tableId = ID;
        ddr_table->version = DDR_VERSION;
        ddr_table->numEntries = DDR_SIZE;
    }

    /* Save DDR entry */
    entry->addr = debug_addr;       /* Save address of data. */
    entry->len = length;            /* Save length of data. */
    /* Copy label into DDR entry id. */
    memcpy(&entry->id, ddr_fr_id[entry_number], 8);

#ifndef Perf
    /* Validate the address is in shared memory, NVRAM, or extra private memory. */
    UINT32 addr = (UINT32)debug_addr;

    if (!((addr >= startOfMySharedMem && addr < endOfMySharedMem) ||
         (addr >= NVRAM_BASE && addr < (NVRAM_BASE+NVRAM_BASESIZE)) ||
         (addr >= (UINT32)&local_memory_start && addr <= (UINT32)&local_memory_start + PRIVATE_SIZE)))
    {
        fprintf(stderr, "%s%s:%u ########## Storing invalid address 0x%x in DDR Table for entry %.8lx ############\n", FEBEMESSAGE, __FILE__, __LINE__, addr, rreg[3]);
        abort();
    }
#endif  /* !Perf */

    /* Update CRC of DDR table (excluding the header entry) */
    ddr_table->crc = MSC_CRC32(&ddr_table->entry[0], sizeof(PROC_DDR_ENTRY) * DDR_SIZE);
}   /* End of M_addDDRentry */

#ifdef BACKEND
/*****************************************************************************
**
** Function Name:   Copy_2_DMC()  Copy data into CCB Direct Memory Copy table.
**
** Inputs:  fid     - Index into *DMC_CCB array.
**          buffer  - Data to save.
**
** Returns: EC_OK if ok, EC_INV_FUNC if invalid.
**
******************************************************************************/
INT32 Copy_2_DMC(UINT32 fid, void *buffer)
{
    struct DMC *entry = DMC_CCB + fid;
    INT32       rc;

    /* Get memory lock and wait if busy. */
    Wait_DMC_Lock(entry);

    /* Validate entry  and that it has data. */
    if (fid >= CCB_DMC_MAX || entry->copy_length == 0)
    {
        rc = EC_INV_FUNC;
    }
    else
    {
        /* Copy data. */
        memcpy(entry->copy_addr, buffer, entry->copy_length);
        rc = EC_OK;
    }

    /* Done with lock. */
    Free_DMC_Lock(entry);
    return rc;
}   /* End of Copy_2_DMC */
#endif  /* BACKEND */

#ifdef BACKEND
/*****************************************************************************
**
** Function Name:   Copy_Raid_DMC()  Copy data into CCB Direct Memory Copy table.
**
** Update:  DMC for struct DMC_raid_structure.
**
** This does what CCB routine RefreshRaids() does.
**
******************************************************************************/
void Copy_Raid_DMC(void)
{
    struct DMC         *entry = DMC_CCB + CCB_DMC_raidcache;
    UINT16              raidID;
    UINT8              *pTmpPtr;
    /* Note: VLOP pointer must remain last entry, and we don't copy it. */
    UINT32              size_RDD_data = sizeof(RDD) - sizeof(struct VLOP *);
    struct RDD         *pRDD;
    struct PSD         *pPSD;
    MRGETRINFO_RSP_EXT *pPsdExt;
    MR_HDR_RSP         *header;
    UINT16              i;
    struct DMC_raid_structure *rs;

    /* Validate entry has data. */
    if (entry->copy_length == 0)
    {
        return;
    }

    rs = entry->copy_addr;                  /* Void stored into any pointer is ok. */

    memset(rs->cacheRaidMap_DMC, 0x00, sizeof(rs->cacheRaidMap_DMC));
    memset(rs->cacheRaidAddr_DMC, 0x00, sizeof(rs->cacheRaidAddr_DMC));
    rs->cacheRaidCount_DMC = gRDX.count;

    /* Where to put the data. */
    pTmpPtr = rs->cacheRaidBuffer_DMC;

    /* Loop through all raids */
    for (raidID = 0; raidID < MAX_RAIDS; raidID++)
    {
        pRDD = gRDX.rdd[raidID];
        if (pRDD == NULL)
        {
            continue;                       /* If no raid. */
        }

        rs->cacheRaidMap_DMC[raidID / 8] |= (1 << (raidID & 7));
        rs->cacheRaidAddr_DMC[raidID] = pTmpPtr;

        /* Set up the header for MRGETRINFO mrp emulation. */
        header = (MR_HDR_RSP *)pTmpPtr;
        header->status = DEOK;
        header->len = sizeof(*header) + size_RDD_data + (pRDD->psdCnt * sizeof(*pPsdExt));

        /* Where to put the raid structure data. */
        pTmpPtr += sizeof(*header);
        memcpy(pTmpPtr, pRDD, size_RDD_data);
        pTmpPtr += size_RDD_data;

        /* Traverse through the PSD list (circular) for all disk Mappings. */
        pPsdExt = (MRGETRINFO_RSP_EXT *)pTmpPtr;
        pPSD = *(PSD **)(pRDD + 1);         /* ZeroArray(PSD) */

        for (i = 0; i < pRDD->psdCnt; i++)
        {
            /* Save PSD pid, status, astatus in response buffer. */
            pPsdExt->pid = pPSD->pid;
            pPsdExt->pidstat = pPSD->status;
            pPsdExt->pidastat = pPSD->aStatus;

            /* % Rebuild = (amount of rebuild so far * 100) / (segment size) */
            pPsdExt->rpc = (UINT8)((pPSD->rLen * 100) / pPSD->sLen);

            /* To next data area for PSD's. */
            pPsdExt = pPsdExt + 1;

            /* Get next PSD in the circular list */
            pPSD = pPSD->npsd;
        }
        /* Next item will be another MR_HDR_RSP. */
        pTmpPtr = (UINT8 *)pPsdExt;
    }
}   /* End of Copy_Raid_DMC */
#endif  /* BACKEND */

#ifdef BACKEND
/*****************************************************************************
**
** Function Name:   Copy_VDisk_DMC()  Copy data into CCB Direct Memory Copy table.
**
** Update:  DMC for struct DMC_vdisk_structure.
**
** NOTDONEYET -- this does what RefreshVirtualDisks() does.
**
******************************************************************************/
void Copy_VDisk_DMC(void)
{
    struct DMC         *entry = DMC_CCB + CCB_DMC_vdiskcache;
    UINT16              vdiskID;
    UINT8              *pTmpPtr;
    /* Yeah, what a way to do it. */
    UINT32              size_VDD_data = sizeof(MRGETVINFO_RSP) - sizeof(MR_HDR_RSP);
    struct VDD         *pVDD;
    struct RDD         *pRDD;
    MRGETVINFO_RSP     *pRSP;
    struct DMC_vdisk_structure *rs;
    int                 i;

    /* Validate entry has data. */
    if (entry->copy_length == 0)
    {
        return;
    }

    rs = entry->copy_addr;                  /* Void stored into any pointer is ok. */

    memset(rs->cacheVDiskMap_DMC, 0x00, sizeof(rs->cacheVDiskMap_DMC));
    memset(rs->cacheVDiskAddr_DMC, 0x00, sizeof(rs->cacheVDiskAddr_DMC));
    memset(rs->cacheVDiskMirrorMap_DMC, 0x00, sizeof(rs->cacheVDiskMirrorMap_DMC));
    memset(rs->cacheVDiskCopyMap_DMC, 0x00, sizeof(rs->cacheVDiskCopyMap_DMC));
    rs->cachevdiskCount_DMC = gVDX.count;

    /* Where to put the data. */
    pTmpPtr = rs->cacheVDiskBuffer_DMC;

    /* Loop through all vdisks */
    for (vdiskID = 0; vdiskID < MAX_VIRTUAL_DISKS; vdiskID++)
    {
        pVDD = gVDX.vdd[vdiskID];
        if (pVDD == NULL)
        {
            continue;                       /* If no vdisk. */
        }

        rs->cacheVDiskMap_DMC[vdiskID / 8] |= (1 << (vdiskID & 7));
        rs->cacheVDiskAddr_DMC[vdiskID] = pTmpPtr;
        if (pVDD-> mirror  == VD_COPYTO ||
            pVDD->mirror == VD_COPYUSERPAUSE ||
            pVDD->mirror == VD_COPYAUTOPAUSE)
        {
            rs->cacheVDiskMirrorMap_DMC[vdiskID /8] |= (1 << (vdiskID & 7));
        }
        if (pVDD-> mirror  == VD_COPYMIRROR)
        {
            rs->cacheVDiskCopyMap_DMC[vdiskID /8] |= (1 << (vdiskID & 7));
        }

        /* Set up the header for MRGETVINFO mrp emulation. */
        pRSP = (MRGETVINFO_RSP *)pTmpPtr;
        pRSP->header.status = DEOK;
        pRSP->header.len = sizeof(pRSP->header) + size_VDD_data +
                     ((pVDD->raidCnt + pVDD->draidCnt) * sizeof(UINT16));

        /* Where to put the vdisk structure data. */
        pTmpPtr += sizeof(pRSP->header);

        pRDD = pVDD->pRDD;
        if (pRDD != NULL)
        {
            if (pRDD->type == RD_SLINKDEV)
            {
                if (pVDD->status == VD_INOP)
                {
                    pVDD->grInfo.vdOpState = 0;   // GR_VD_INOP;
                }
                else
                {
                    pVDD->grInfo.vdOpState = 3;   // GR_VD_OP;
                }
//                BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
            }
        }

        memcpy(pTmpPtr, pVDD, size_VDD_data);
        pTmpPtr += size_VDD_data;

        /*
         * Update the time of last access - this is the number
         * of recent seconds for which the disk is NOT accessed.
         */
        if (pVDD->lastAccess == 0)
        {
            pRSP->lastAccess = -1;
        }
        else
        {
            /*
             * If we are the owner, compute the last access.
             * If not, reset the value.
             */
            if (DL_ExtractDCN(K_ficb->cSerial) == pVDD->owner)
            {
                pRSP->lastAccess = (K_ii.time - pVDD->lastAccess) / 8;
            }
            else
            {
                pVDD->lastAccess = 0;
                pRSP->lastAccess = -1;
//                BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
            }
        }

        /* Store the average IO per second and SC per second over last hour */
        if (pVDD->pLastHourAvgStats != NULL)
        {
            if (pVDD->pLastHourAvgStats->statsFlag)
            {
                pRSP->lastHrAvgIOPerSec = pVDD->pLastHourAvgStats->LastOneHrAvgIoPerSecond / 3600;
                pRSP->lastHrAvgSCPerSec = pVDD->pLastHourAvgStats->LastOneHrAvgSCPerSecond / 3600;
            }
            else if (pVDD->pLastHourAvgStats->currentIndex == 0)
            {
                pRSP->lastHrAvgIOPerSec = 0;
                pRSP->lastHrAvgSCPerSec = 0;
            }
            else
            {
                pRSP->lastHrAvgIOPerSec =
                    pVDD->pLastHourAvgStats->LastOneHrAvgIoPerSecond /
                    (pVDD->pLastHourAvgStats->currentIndex * 60);
                pRSP->lastHrAvgSCPerSec =
                    pVDD->pLastHourAvgStats->LastOneHrAvgSCPerSecond /
                    (pVDD->pLastHourAvgStats->currentIndex * 60);
            }
        }

        /* Traverse through the RAID list for all LUN to Vdisk mappings. */
        i = 0;
        while (pRDD)
        {
            pRSP->rid[i++] = pRDD->rid;
            pRDD = pRDD->pNRDD;
        }

        /* Next item will be another MR_HDR_RSP. */
        pTmpPtr += ((pVDD->raidCnt + pVDD->draidCnt) * sizeof(UINT16));
    }
}   /* End of Copy_VDisk_DMC */
#endif  /* BACKEND */

#ifdef BACKEND
/*****************************************************************************
**
** Function Name:   Process_DMC_delayed()  Process DMC requests every 5 seconds.
**
******************************************************************************/
void Process_DMC_delayed(void)
{
    static UINT8 delay = 0;

    delay++;
    if (delay > 5)
    {
        Process_DMC();
        delay = 0;
    }
}   /* End of Process_DMC_delayed */
#endif  /* BACKEND */

#ifdef BACKEND
/*****************************************************************************
**
** Function Name:   Process_DMC()  Process any DMC requests needing updating.
**
******************************************************************************/
void Process_DMC(void)
{
    UINT64       retry = DMC_bits;
    UINT64       nextretry;
    struct DMC  *entry;

    /* When exiting, we will have cleared all the bits in DMC_bits. */
    DMC_bits = 0;

    while (retry != 0)
    {
        /* If can't get a lock, do others, then try again. */
        nextretry = 0;

        if (BIT_TEST(retry, CCB_DMC_raidcache))
        {
            entry = DMC_CCB + CCB_DMC_raidcache;
            /* Get memory lock. */
            if (test_zero_and_set_one_uint8(&entry->atomic_lock) == 0)
            {
                Copy_Raid_DMC();
                /* Done with lock. */
                Free_DMC_Lock(entry);
            }
            else
            {
                BIT_SET(nextretry, CCB_DMC_raidcache);
            }
        }

        if (BIT_TEST(retry, CCB_DMC_vdiskcache))
        {
            entry = DMC_CCB + CCB_DMC_vdiskcache;
            /* Get memory lock. */
            if (test_zero_and_set_one_uint8(&entry->atomic_lock) == 0)
            {
                Copy_VDisk_DMC();
                /* Done with lock. */
                Free_DMC_Lock(entry);
            }
            else
            {
fprintf(stderr,"%s%s:%u CCB_DMC_vdiskcache lock failed\n", FEBEMESSAGE, __FILE__, __LINE__);
                BIT_SET(nextretry, CCB_DMC_vdiskcache);
            }
        }

        if (BIT_TEST(retry, CCB_DMC_pdiskcache))
        {
            entry = DMC_CCB + CCB_DMC_pdiskcache;
            /* Get memory lock. */
            if (test_zero_and_set_one_uint8(&entry->atomic_lock) == 0)
            {
//                Copy_PDisk_DMC();
fprintf(stderr,"%s%s:%u CCB_DMC_pdiskcache NOTDONEYET\n", FEBEMESSAGE, __FILE__, __LINE__);
                /* Done with lock. */
                Free_DMC_Lock(entry);
            }
            else
            {
fprintf(stderr,"%s%s:%u CCB_DMC_pdiskcache lock failed\n", FEBEMESSAGE, __FILE__, __LINE__);
                BIT_SET(nextretry, CCB_DMC_pdiskcache);
            }
        }

        /* If couldn't get lock immediately, try those entries again. */
        retry = nextretry;
    }
}   /* End of Process_DMC */
#endif  /* BACKEND */

/* ------------------------------------------------------------------------ */
/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
