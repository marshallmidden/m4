/* $Id: wcrcvry.c 159663 2012-08-22 15:36:42Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       wcrcvry.c
**
**  @brief      Write Cache Recovery functions
**
**  This module provides the functions to support the flushing of
**  cached write data at power-on.  Data is retained in the write cache
**  if the processor is power-cycled or reset instead of the normal shutdown.
**  To prevent the loss of customer data, the cached data is flushed to disk.
**
**  Copyright (c) 2002-2009 Xiotech Corporation.  All rights reserved.
**
******************************************************************************
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <byteswap.h>

#include "cache.h"
#include "CA_CI.h"
#include "vcd.h"
#include "datagram.h"
#include "DLM_fe.h"
#include "drp.h"
#include "ecodes.h"                     /* Error Code definitions           */
#include "ficb.h"
#include "ilt.h"
#include "kernel.h"
#include "LOG_Defs.h"
#include "LL_LinuxLinkLayer.h"
#include "misc.h"
#include "MP_Proc.h"
#include "MR_Defs.h"
#include "NV_Memory.h"
#include "options.h"
#include "OS_II.h"
#include "pm.h"
#include "QU_Library.h"
#include "rb.h"
#include "sgl.h"
#include "system.h"
#include "wcache.h"
#include "vdmt.h"
#include "XIO_Const.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"
#include "XIO_Types.h"
#include "mem_pool.h"
#include "CT_defines.h"

/*
******************************************************************************
** Private defines - constants
******************************************************************************
*/
#ifndef PERF
#define DEBUG_WCR           TRUE
/*#define DEBUG_WCR_VERBOSE   TRUE*/
#endif

#define USE_NEITHER         0
#define USE_LOCAL           1
#define USE_REMOTE          2
#define USE_BOTH            3

#define DIRTY               0
#define INVALID_VID         1
#define LBA_OUT_OF_RANGE    2
#define INVALID_TAG         3
#define TAG_MISMATCH        4
#define DATA_MISMATCH       5
#define ECC_TAG_ERR         6
#define ECC_DATA_ERR        7
#define REMOTE_ACCCESS_ERR  8
#define NOT_DIRTY           128

#define ERROR_LIMIT         5

#define STOP_OPTIONS        0x08    /* Stop Option = Do not wait for host ops */
#define STOP_WCRECVRY_USER  3       /* mxiwcrecvry User ID Stop Count       */

/*
******************************************************************************
** Private defines - macros
******************************************************************************
*/
#define DATAGRAM_SIZE (DLM_REQ_SIZE + DATAGRAM_REQ_SIZE + \
                        CAC0_ENTRY_LEN + \
                        DATAGRAM_RSP_SIZE + sizeof(SGL) + \
                        sizeof(SGL_DESC))

/*
******************************************************************************
** Private variables
******************************************************************************
*/
static BB* c_bbdTable[2];
static UINT32 seqNo;

/*
** Log message counters for Cache recovery complete (s_MallocW/Free for message).
*/
static LOG_CACHE_TAGS_RECOVERED_PKT ecrc;

/*
******************************************************************************
** Public variables - externed in the header file
******************************************************************************
*/
extern VDMT* MAG_VDMT_dir[MAX_VIRTUALS];
extern CA C_ca;
extern TG* c_tgfree;                /* Free cache tag list                  */

extern INT16 wc_mem_seqnum;

extern UINT32 WcctSize;             /* Size of the Write Cache Control Table */
extern WT* WcctAddr;                /* Address of the Write Cache Control Tbl */
extern UINT32 WccSize;              /* Size of the Write Cache Configuration */
extern UINT8 * WccAddr;             /* Write Cache Configuration area       */
extern UINT32 WctSize;              /* Size of the Write Cache Tag area     */
extern TG* WctAddr;                 /* Write Cache Tag area                 */
extern UINT32 WcbSize;              /* Size of the Write Cache Buffer area  */
extern UINT8 * WcbAddr;             /* Write Cache Buffer area              */

extern UINT8  c_resumeInit[2];
extern UINT32 c_bbdFail;
extern UINT32 c_flushErrorCount;

extern UINT32 c_rcDirtyCount;
extern UINT32 c_recoveryMode;
extern UINT16 *c_vidFailed;
extern UINT32 C_mirror_error_flag;
extern  QU gWCMarkCacheQueue;           /* Mark Cache Queue                 */

/*
******************************************************************************
** Public function prototypes not in any header files
******************************************************************************
*/
extern void CA_LogMirrorFailure(UINT32 rc, struct ILT *pILT);
extern void wc_recoveryFlushRequest(TG*);
extern void DLM$quedrp(UINT32, ILT *);
extern UINT32 C$stop(UINT8 C_Options, UINT8 C_User);   /* Cache Stop Function      */
extern void wc$msgCacheRecoverFail(UINT32 rc, TG* localTag);
extern void WC_initTag(TG *);
extern void wc_markWCache(void);
extern void wc_recoveryInit(void);
extern void WC_CopyNVwWait(void* dst, void* src, UINT8 where, UINT32 length);

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
static UINT32 wc_wrtSigBlock(WT*);
static void *wc_getRemoteData(void *, void *, UINT32, UINT8);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
*****************************************************************************
**
**  @brief      Accesses data at the remote DRAM.
**
**              Sends a datagram to the morror partner to access
**              the remote DRAM at the specified address.
**
**  @param      * dst       - Destination address (local DRAM).
**  @param      * src       - Source address (remote DRAM).
**  @param      length      - Length of data.
**  @param      region      - Memory region.
**  @param      function    - Function code.
**
**  @return     Completion status.
**
******************************************************************************
**/
UINT32 WC_RemoteData(void *dst, void *src, UINT32 length, UINT8 region,
                                                                UINT8 function)
{
    DLM_REQ * drp;
    DATAGRAM_REQ * dgrq;
    DATAGRAM_RSP * dgrs;
    CAC0 * cac0;
    ILT * ilt;
    SGL* sgl;
    SGL_DESC* desc;
    UINT32 retValue;

    /*
    ** Allocate memory for a DRP and Datagram.
    */
    drp = s_MallocC(DATAGRAM_SIZE, __FILE__, __LINE__);
    dgrq = (void *)(drp + 1);
    cac0 = (void *)(dgrq + 1);
    dgrs = (void *)(cac0 + 1);
    sgl  = (void *)(dgrs + 1);
    desc = (void *)(sgl + 1);

    /*
    ** Set up the DRP
    */
    drp->func = DR_CACHE_TO_DLM;
    drp->sglPtr = sgl;
    drp->reqAddress = dgrq;
    drp->reqLength = DATAGRAM_REQ_SIZE + CAC0_ENTRY_LEN;
    drp->rspAddress = dgrs;
    drp->rspLength = DATAGRAM_RSP_SIZE;
    drp->timeout = CACHE_TIME_OUT;      /* Op Timeout (in seconds)          */
    drp->issueCnt = CACHE_ISSUE_CNT;    /* Retry count                      */

    /*
    ** Set up the Datagram Request Header
    */
    dgrq->hdrLen = DATAGRAM_REQ_SIZE;   /* Save the Request Header size     */
    dgrq->srvCPU = DG_CPU_INTERFACE;    /* Save the FE as the Server CPU    */
    dgrq->seq = ++wc_mem_seqnum;        /* Increment the DG Sequence Number */
    dgrq->fc = function;                /* Memory Function Code             */
    dgrq->path = DG_PATH_ANY;           /* Take any path available          */
    dgrq->srvName = CAC0_NAME;          /* Save the server name (Cache)     */
    dgrq->reqLen = CAC0_ENTRY_LEN;      /* Save extended request info length*/
    dgrq->dstSN = bswap_32(K_ficb->mirrorPartner); /* Save the Dest. SN    */
    dgrq->gpr0 = region;

    /*
    ** Set up the Datagram Extended Request Information for the Cache Datagram.
    */
    cac0->rqMemAddr = (function == CAC0_FC_WRTMEM) ? dst : src;
    cac0->rqMemLen  = length;

    /*
    ** Set up the Datagram Return packet so if the return datagram is not
    ** received (and the return status not changed) it can be detected.
    */
    dgrs->status = 0xFF;

    /*
    ** Set up the DRP SGL entry
    */
    sgl->scnt = 1;
    sgl->flag = 0;
    sgl->size = sizeof(SGL) + sizeof(SGL_DESC);
    desc->addr = (function == CAC0_FC_WRTMEM) ? src : dst;
    if (function == CAC0_FC_WRTMEM)
    {
        desc->len = length | (SG_DIR_OUT<<24);
    }
    else
    {
        desc->len = length | (SG_DIR_IN<<24);
    }

    /*
    ** Allocate an ILT and set the completion routine.
    */
    ilt = get_ilt();                /* new ILT for DRP processing           */
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)ilt);
#endif /* M4_DEBUG_ILT */
    ilt->fthd = NULL;               /* Clear the ILT Forward pointer        */
    ilt->bthd = NULL;               /* Clear the ILT BaCKForward pointer    */
    ilt->ilt_normal.w0 = (UINT32)drp;          /* DRP address                          */

    /*
    ** Send the data to the partner.
    ** Queue the request to DLM
    */
    EnqueueILTW(DLM$quedrp, ilt);   /* Queue the DRP to DLM                 */

    /*
    ** Before memory is released, get the datagram return status
    */
    retValue = dgrs->status;

    if ((retValue != 0) && (C_mirror_error_flag == FALSE))
    {
        /*
        ** Report the error (if not already done so)
        */
        CA_LogMirrorFailure(retValue, ilt);

        /*
        ** Exchange process so message is sent.
        */
        TaskSwitch();
    }
    else if (C_mirror_error_flag == TRUE)
    {
        /*
        ** Mirror request was successful, so clear the failure flag to
        ** enable logging again.
        */
        C_mirror_error_flag = FALSE;
    }

    /*
    ** Release memory
    */
    s_Free(drp, DATAGRAM_SIZE, __FILE__, __LINE__);
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)ilt);
#endif /* M4_DEBUG_ILT */
    put_ilt(ilt);

    return retValue;
} /* WC_RemoteData */

/**
******************************************************************************
**
**  @brief      Write the signature block to the mirror partner.
**
**              This function creates and sends the DRP necessary to mirror the
**              write cache signature block to the mirror partner.  A log msg
**              is surfaced if this mirror request fails.
**
**  @param      * signatureBlock    - Pointer to signature block to write
**
**  @return     Completion status.
**
******************************************************************************
**/
static UINT32 wc_wrtSigBlock(WT* signatureBlock)
{
    DLM_REQ * drp;
    DATAGRAM_REQ * dgrq;
    DATAGRAM_RSP * dgrs;
    CAC0 * cac0;
    ILT * ilt;
    SGL* sgl;
    SGL_DESC* desc;
    UINT32 retValue;
    UINT32 loopCnt;

    fprintf(stderr, "%s:  Signature = 0x%08x, seqNo = 0x%x, "
            "S/N = 0x%08x, MP = 0x%08x\n", __func__,
            signatureBlock->signature1, signatureBlock->seq,
            signatureBlock->cSerial, signatureBlock->mirrorPartner);

    /*
    ** Allocate memory for a DRP and Datagram.
    */
    drp = s_MallocC(DATAGRAM_SIZE, __FILE__, __LINE__);
    dgrq = (void *) (drp + 1);
    cac0 = (void *) (dgrq + 1);
    dgrs = (void *) (cac0 + 1);
    sgl  = (void *) (dgrs + 1);
    desc = (void *) (sgl + 1);

    /*
    ** Set up the DRP
    */
    drp->func = DR_CACHE_TO_DLM;
    drp->sglPtr = sgl;
    drp->reqAddress = dgrq;
    drp->reqLength = DATAGRAM_REQ_SIZE + CAC0_ENTRY_LEN;
    drp->rspAddress = dgrs;
    drp->rspLength = DATAGRAM_RSP_SIZE;
    drp->timeout = CACHE_TIME_OUT;      /* Op Timeout (in seconds)          */
    drp->issueCnt = CACHE_ISSUE_CNT;    /* Retry count                      */

    /*
    ** Set up the Datagram Request Header
    */
    dgrq->hdrLen = DATAGRAM_REQ_SIZE;   /* Save the Request Header size     */
    dgrq->srvCPU = DG_CPU_INTERFACE;    /* Save the FE as the Server CPU    */
    dgrq->seq = ++wc_mem_seqnum;        /* Increment the DG Sequence Number */
    dgrq->fc = CAC0_FC_WRTMEM;          /* Memory Function Code             */
    dgrq->path = DG_PATH_ANY;           /* Take any path available          */
    dgrq->srvName = CAC0_NAME;          /* Save the server name (Cache)     */
    dgrq->reqLen = CAC0_ENTRY_LEN;      /* Save extended req info len */
    dgrq->dstSN = bswap_32(K_ficb->mirrorPartner); /* Save the Dest. SN    */
    dgrq->gpr0 = 1 << CAC0_WCCT;

    /*
    ** Set up the Datagram Extended Request Information for the Cache Datagram.
    */
    cac0->rqMemAddr = WcctAddr;
    cac0->rqMemLen = sizeof(struct WT);

    /*
    ** Set up the Datagram Return packet
    */
    dgrs->status = 0xFF;

    /*
    ** Set up the DRP SGL entries.
    */
    sgl->scnt = 1;
    sgl->flag = 0;
    sgl->size = sizeof(SGL) + sizeof(SGL_DESC);
    desc->addr = signatureBlock;
    desc->len = sizeof(struct WT) | (SG_DIR_OUT << 24);

    /*
    ** Allocate an ILT and set the completion routine.
    */
    ilt = get_ilt();                /* new ILT for DRP processing           */
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)ilt);
#endif /* M4_DEBUG_ILT */
    ilt->fthd = NULL;               /* Clear the ILT Forward pointer        */
    ilt->bthd = NULL;               /* Clear the ILT Backward pointer       */
    ilt->ilt_normal.w0 = (UINT32)drp;          /* DRP address                          */

    /*
    ** Send the data to the partner (Loop if needed).
    ** Queue the request to DLM
    */
    EnqueueILTW(DLM$quedrp, ilt);

    /*
    ** Get the datagram return status
    */
    retValue = dgrs->status;

    /*
    ** If an error occurred and the error is due to the FE ports going away,
    ** then retry this when the ports and communications to the MP is available
    */
    if ((retValue != EC_OK) &&
        (dgrq->dstSN == bswap_32(K_ficb->mirrorPartner)) &&
        (!(BIT_TEST(C_ca.status, CA_ERROR))) &&
        (!(BIT_TEST(C_ca.status, CA_MIRRORBROKEN))) &&
        (DLM$queryFEcomm(K_ficb->mirrorPartner) != EC_OK))
    {
        /*
        ** Wait for Communications to be become active again (30 seconds max)
        */
        for (loopCnt = 0; loopCnt < 240; loopCnt++)
        {
            /*
            ** Wait for awhile and then determine if OK to check again
            */
            TaskSleepMS(128);           /* Sleep for 128 msec               */
            if ((dgrq->dstSN == bswap_32(K_ficb->mirrorPartner)) &&
                (!(BIT_TEST(C_ca.status, CA_ERROR))) &&
                (!(BIT_TEST(C_ca.status, CA_MIRRORBROKEN))))
            {
                if (DLM$queryFEcomm(K_ficb->mirrorPartner) == EC_OK)
                {
                    /*
                    ** Communications has been established again, break out
                    ** of the loop
                    */
                    break;
                }
            }
            else
            {
                /*
                ** Cannot retry, terminate loop and do not retry the Send
                */
                loopCnt = 240;
            }
        }

        /*
        ** Retry the op once, if we can
        */
        if ((loopCnt < 240) &&
            (dgrq->dstSN == bswap_32(K_ficb->mirrorPartner)) &&
            (!(BIT_TEST(C_ca.status, CA_ERROR))) &&
            (!(BIT_TEST(C_ca.status, CA_MIRRORBROKEN))))
        {
            /*
            ** Queue the request to DLM
            */
            EnqueueILTW(DLM$quedrp, ilt);

            /*
            ** Get the datagram return status
            */
            retValue = dgrs->status;
        }
    }

    /*
    ** Handle the case were the problem persisted too long
    */
    if ((retValue != EC_OK) &&
        (C_mirror_error_flag == FALSE) &&
        (dgrq->dstSN == bswap_32(K_ficb->mirrorPartner)) &&
        (!(BIT_TEST(C_ca.status, CA_ERROR))) &&
        (!(BIT_TEST(C_ca.status, CA_MIRRORBROKEN))))
    {
        /*
        ** Report the error (if not already done so)
        */
        CA_LogMirrorFailure(retValue, ilt);

        /*
        ** Exchange process so message is sent.
        */
        TaskSwitch();
    }
    else if ((retValue == EC_OK) && (C_mirror_error_flag == TRUE))
    {
        /*
        ** Mirror request was successful, so clear the failure flag to
        ** enable logging again.
        */
        C_mirror_error_flag = FALSE;
    }

    /*
    ** Release memory
    */
    s_Free(drp, DATAGRAM_SIZE, __FILE__, __LINE__);
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)ilt);
#endif /* M4_DEBUG_ILT */
    put_ilt(ilt);

    return retValue;

} /* wc_wrtSigBlock */

/**
******************************************************************************
**
**  @brief      Gets data from the remote DRAM.
**
**              Gets the specified data from the remote DRAM and copies
**              the data to the specified address.
**              Alllocates a buffer if destination is NULL.
**
**  @param      void *dst       - Destination address (local DRAM).
**  @param      void *src       - Source address (remote DRAM).
**  @param      UINT32 length   - Length of data.
**  @param      UINT8 region    - (unused param)
**
**  @return     Destination address (local DRAM).
**
******************************************************************************
**/
static void *wc_getRemoteData(void *dst, void *src, UINT32 length,
#if NO_REMOTE_MIRROR
                                             UINT8 region UNUSED)
#else
                                             UINT8 region)
#endif
{
    UINT32 masgFlag;

    /*
    ** Check if the our Back End DRAM is the mirror partner
    */
    if (K_ficb->cSerial == K_ficb->mirrorPartner)
    {
        src = (UINT8 *)src + BE_ADDR_OFFSET;
#ifdef DEBUG_WCR_VERBOSE
        fprintf(stderr, "%s: Accessing BE Address %p\n", __func__, src);
#endif
        if ((UINT32)src < startOfBESharedMem ||
            (UINT32)src >= endOfBESharedMem)
        {
            fprintf(stderr, "%s: Attempting to access invalid BE address %p\n",
                __func__, src);
            abort();
        }

        /*
        ** Allocate a block of memory if caller didn't
        */
        if (dst == NULL)
        {
            dst = s_MallocW(length, __FILE__, __LINE__);
        }
        memcpy(dst, src, length);
    }
    else if (BIT_TEST(C_ca.status, CA_MIRRORBROKEN) == 0)
    {
        /*
        ** Allocate a block of memory if caller didn't
        */
        if (dst == NULL)
        {
            dst = s_MallocW(length, __FILE__, __LINE__);
            masgFlag = TRUE;
        }
        else
        {
            masgFlag = FALSE;
        }

        /*
        ** Read the remote data from the mirror partner
        */
#if !NO_REMOTE_MIRROR
        if (WC_RemoteData(dst, src, length, region, CAC0_FC_RDMEM) != 0)
#endif
        {
            /*
            ** Remote access failed.  Release memory (if allocated).
            */
            if (masgFlag == TRUE)
            {
                s_Free(dst, length, __FILE__, __LINE__);
            }

            /*
            ** Indicate the failure to the caller
            */
            dst = NULL;
        }
    }
    else
    {
        fprintf(stderr, "%s: Unhandled case?\n", __func__);
    }

    return dst;
} /* wc_getRemoteData */


/**
******************************************************************************
**
**  @brief      Get a cache tag from the remote DRAM.
**
**              Gets the specified cache tag from the remote DRAM
**              and copies the contents to the specified address.
**
**  @param      * dst   - Address to store data (destination).
**  @param      * src   - Address of local Tag (source).
**
**  @return     Cache tag structure
**
******************************************************************************
**/
static TG *wc_getRemoteTag(TG *dst, TG *src)
{
    TG *tag;
    UINT32 oldaddr;

    /*
    ** Get the tag
    */
    tag = wc_getRemoteData(dst, src, sizeof(*tag), 1 << CAC0_TAG);

    /*
    ** Determine if the tag was successfully retrieved and if it is a BE tag
    */
    if (tag != NULL && (tag->attrib & TGM_BE))
    {
        /*
        ** The tag was in the process of being flushed or invalidated by the
        **  CCB.  Convert it back to a normal looking tag by turning off the
        **  BE flag and converting the Buffer Address to a Normal address, if
        **  not already done.
        */
        tag->attrib &= ~(TGM_BE);   /* Turn off the BE flag in the tag      */

        if ((UINT32)tag->bufPtr >= startOfBESharedMem &&
            (UINT32)tag->bufPtr < endOfBESharedMem)
        {
            oldaddr = (UINT32)tag->bufPtr;
            tag->bufPtr -= BE_ADDR_OFFSET;
            fprintf(stderr, "Translating tag buffer address from 0x%x to %p\n",
                    oldaddr, tag->bufPtr);
        }
        else if ((UINT32)tag->bufPtr < startOfMySharedMem ||
                 (UINT32)tag->bufPtr >= endOfMySharedMem)
        {
            fprintf(stderr, "Tag buffer address is invalid: %p\n", tag->bufPtr);
            abort();
        }
    }

    return tag;
} /* wc_getRemoteTag */


/**
******************************************************************************
**
**  @brief      Initialize cache tag
**
**  @param      * tag   - Cache tag structure to initialize.
**  @param      * src   - Address of local Tag (source).
**
**  @return     Cache tag structure
**
******************************************************************************
**/
void WC_initTag(TG* tag)
{
    /*
    ** Initialize these cache tag fields.
    ** Preserve the forward tag pointer.
    */
    tag->bthd = NULL;               /* Clear the backward pointer           */
    tag->vid = -1;                  /* Invalidate VID                       */
    tag->rdCnt = 0;                 /* Clear read In progress count         */
    tag->attrib = 1<<TG_FREE;       /* Set tag attributes to FREE           */
    tag->state = 0;                 /* Clear tag state                      */
    tag->vsda = -1;                 /* Invalidate LBA                       */
    tag->vLen = 0;                  /* Clear length                         */
    tag->bufPtr = NULL;             /* Clear buffer pointer                 */
    tag->ioPtr = NULL;              /* Clear cache tree ptr                 */
    tag->dirtyPtr = NULL;           /* Clear dirty tree node pointer        */
    tag->nextDirty = NULL;          /* Clear next dirty block               */
    tag->hQueue = NULL;             /* Clear the Unlock Queue Forward Ptr   */
    tag->tQueue = NULL;             /* Clear the Unlock Queue Backward Ptr  */
} /* WC_initTag */

/**
******************************************************************************
**
**  @brief      This is a task that handles incoming ILTs to Mark the Write
**              Cache as being Enabled or Disabled.  This indicates whether the
**              write cache may contain valid data or if the write cache
**              is empty.
**
**              When the write cache is stopped or disabled and all data
**              has been flushed, markWCacheDis is called to indicate
**              the cache is empty.  When the write cache is enabled,
**              markWCacheEn is called to indicate the cache may contain
**              write data.  Also, when the mirror partner S/N changes, the
**              signature is refreshed to ensure it stays in sync with the FICB.
**
**              This indication is used on power up to determine if the
**              write cache contains valid data that needs to be written
**              to disk prior to initializing the write cache.
**
**              A "magic" number is written to two addresses to indicate
**              write data is cached or flushed.  The system serial number,
**              controller serial number, DRAM usage (local or remote),
**              and sequence number are written between the two writes
**              of the "magic" number.
**
**  @param      none
**
**  @return     none
**
**  @attention  This is a task that never returns.  To enqueue an ILT to this
**              task see the <QU_EnqueReqILT> function with an input of
**              gWCMarkCacheQueue.  The ILT must have:
**                  <il_w0>     - Enable (DATA_CACHED),
**                                Disable (DATA_FLUSHED), or
**                                Refresh (SIG_REFRESH) flag
**                  <il_w1>     - Completion Status returned in this field
**                  <il_misc>   - PCB to awaken upon completion
**
******************************************************************************
**/
NORETURN
void wc_markWCache(void)
{
    struct WT        *signatureBlock;
    UINT32           signature;
    struct WT       *remoteSignatureBlock;
    UINT32           retValue = EC_IO_ERR;  /* Prep (always update 1st time)*/
    ILT*             pWorkingILT = NULL;    /* Working ILT                  */

    /*
    **  Setup queue control block
    */
    gWCMarkCacheQueue.pcb = K_xpcb;     /* Set up my PCB                    */
    gWCMarkCacheQueue.head = NULL;      /* Set up the Head ILT              */
    gWCMarkCacheQueue.tail = NULL;      /* Set up the Tail ILT              */
    gWCMarkCacheQueue.qcnt = 0;         /* Set up the Count of ILTs on queue*/

    remoteSignatureBlock = s_MallocC(sizeof(struct WT), __FILE__, __LINE__);
    /*
    ** Task runs forever
    */
    while (TRUE)                        /* Always keep task running         */
    {
        /*
        ** Set process to wait for signal and Exchange process
        */
        TaskSetMyState(PCB_NOT_READY);
        TaskSwitch();

        /*
        ** Work on the list of changes to the signature block
        */
        while ((pWorkingILT = QU_DequeReqILT(&gWCMarkCacheQueue)) != NULL)
        {
            /*
            ** Set up the Signature Block to do the DMAs and Mirroring
            */
            signatureBlock = WcctAddr;

            /*
            ** Check if a new signature was specified (for cache disable/enable)
            ** or if this is a refresh of the remote signature (e.g. due to
            ** Mirror Partner S/N changing)
            */
            signature = pWorkingILT->ilt_normal.w0;
            if ((signature != DATA_FLUSHED) && (signature != DATA_CACHED))
            {
                /*
                ** This is a refresh of the remote signature, so preload the
                ** current local value
                */
                signature = signatureBlock->signature1;
            }

            /*
            ** Only write the Signature block if it is changing or we were
            ** unsuccessful the last time in updating the block
            */
            if ((signatureBlock->signature1    != signature) ||
                (signatureBlock->vcgID         != K_ficb->vcgID) ||
                (signatureBlock->cSerial       != K_ficb->cSerial) ||
                (signatureBlock->seq           != K_ficb->seq) ||
                (signatureBlock->mirrorPartner != K_ficb->mirrorPartner) ||
                (signatureBlock->signature2    != signature) ||
                (retValue != EC_OK))
            {
                /*
                ** ---- Update the Signature Block ----------------------------
                */
                /*
                ** Initialize the Returned Value to good
                ** Initialize the local signature block
                ** Indicate if the cache is empty, or contains dirty data
                ** Indicate if this is the Front End Local DRAM
                ** Write the sytem and controller serial numbers
                */
                retValue = EC_OK;
                signatureBlock->signature1    = signature;
                signatureBlock->mirrorAttrib  = MIRROR_LOCAL;
                signatureBlock->vcgID         = K_ficb->vcgID;
                signatureBlock->cSerial       = K_ficb->cSerial;
                signatureBlock->seq           = K_ficb->seq;
                signatureBlock->mirrorPartner = K_ficb->mirrorPartner;
                signatureBlock->signature2    = signature;

                /*
                ** Prep to mirror the data to NV Memory also.
                */
                WC_CopyNVwWait(WcctAddr, signatureBlock, MIRROR_FE,
                                                    sizeof(*signatureBlock));

                /*
                ** Initialize the image for the remote signature block
                */
                *remoteSignatureBlock = *signatureBlock;
                remoteSignatureBlock->mirrorAttrib  = MIRROR_REMOTE;

                /*
                ** Check if the our Back End DRAM is the mirror partner
                */
                if (K_ficb->cSerial == K_ficb->mirrorPartner)
                {
                    /*
                    ** Set signature block in BE DRAM
                    */
                    WC_CopyNVwWait(WcctAddr, remoteSignatureBlock, MIRROR_BE,
                                                            sizeof(struct WT));
                }
#if !NO_REMOTE_MIRROR
                else if ((C_ca.status & (1<<CA_MIRRORBROKEN)) == 0)
                {
                    /*
                    ** Set signature block in Mirror Partner DRAM
                    */
                    retValue = wc_wrtSigBlock(remoteSignatureBlock);
                }
#endif
            }

            /*
            ** The signature was updated or was the same as the last - complete
            ** the original request by readying the waiting task
            */
            pWorkingILT->ilt_normal.w1 = retValue; /* Show how the op completed        */
#ifdef HISTORY_KEEP
CT_history_pcb("wc_markWCache setting ready pcb", (UINT32)(pWorkingILT->misc));
#endif
            TaskSetState(((PCB*)pWorkingILT->misc), PCB_READY); /* Ready Task */
        }
    }
} /* wc_markWCache */

/**
******************************************************************************
**
**  @brief      wc_checkEccSite
**
**              Check whether any multi-bit ECC errors were discovered in the
**              specified range.
**
**  @param      void* addr      - Address of region to check for ECC errors
**  @param      UINT32 length   - Length of region to check  for ECC errors
**  @param      BB* pBbd        - Address of battery backup data
**
**  @return     UINT32 Error status ( 0 if no error )
**
******************************************************************************
**/
static UINT32 wc_checkEccSite(void *addr, UINT32 length, BB *pBbd)
{
    UINT32 retValue = FALSE;   /* Prep return value                */
    UINT32 i;                  /* Loop counter                     */
    UINT32 *eccAddr;
    UINT32 *endAddr;

    endAddr = (UINT32 *) (((UINT8 *) addr) + length);

    /*
    ** Check if no ECC errors
    */
    if (pBbd != NULL)
    {
        for (i = 0; i < pBbd->multBitEccCurrent; ++i)
        {
            eccAddr = pBbd->multECarReg16[i];

            if ((eccAddr >= (UINT32 *)addr) && (eccAddr <= endAddr))
            {
                /*
                ** An ECC error site is within specified region.
                */
                retValue = TRUE;
            }
        }
    }

    return retValue;
} /* wc_checkEccSite */

/**
******************************************************************************
**
**  @brief      Checks the cache tag for an ECC error.
**
**              Checks the cache tag for an ECC error.  If an ECC error
**              is detected, neither the tag nor the write cache data
**              associated with the tag from the same DRAM can be used.
**
**              If only one DRAM is enabled for write caching:
**
**               - If an ECC error site falls within the relevant areas
**                 of the cache tag or within the data, the write cache data
**                 is not flushed.
**
**              If both DRAMs are enabled for write caching:
**
**              - If the local cache tag contains an ECC error and the
**                remote cache tag is dirty, check for an ECC error in the
**                remote write cache data.  Since we do not know the value
**                of the tag attribute of the local tag, the write cache data
**                in the local DRAM may not be correct.
**                If the write cache data stored in the remote DRAM
**                contains an ECC error, this tag cannot be flushed.
**                Otherwise, copy the write cache data from the remote DRAM
**                to the local DRAM.  Then copy the cache tag from the
**                remote DRAM to the local DRAM.
**
**              - If the remote cache tag contains an ECC error and the
**                local cache tag is dirty, check for an ECC error in the
**                local write cache data.  Since we do not know the value
**                of the tag attribute of the remote tag, the write cache data
**                in the remote DRAM may not be correct.
**                If the write cache data stored in the local DRAM
**                contains an ECC error, this tag cannot be flushed.
**                Otherwise, copy the write cache data from the local DRAM
**                to the remote DRAM.
**
**  @param      * tag   - Cache tag
**  @param      * pBbd  - Address of battery backup data
**
**  @return     description of return or none
**
******************************************************************************
**/
static UINT32 wc_checkTagEcc(TG* tag, BB* pBbd)
{
    /*
    ** Check if no ECC errors
    */
    if (pBbd == NULL)
    {
        return FALSE;
    }
    else
    {
        /*
        ** Check if this tag has a ECC error
        */
        return wc_checkEccSite(tag, sizeof(*tag), pBbd);
    }
} /* wc_checkTagEcc */


/**
******************************************************************************
**
**  @brief      Determine if the specified tag is dirty.  If the tag is dirty,
**              the tag is validated.
**
**              Verifies the cache tag.  First check if the tag is dirty.
**              The VID is verifed.  The LBA and length are verified.
**              The tag state is verified.  The buffer pointer is verified.
**
**              NOTE:  The ASSUMED calling sequence for this routine is to
**              first check the local tag, if available, and next call the
**              routine again to verify the remote tag and/or both tags.
**              If the design is changed to alter this calling sequence,
**              then this routine must be updated to correctly verify both tags.
**
**  @param      TG* localTag  - Local cache tag
**  @param      TG* remoteTag - Remote cache tag
**
**  @return     Error status ( 0 if no error )
**
******************************************************************************
**/
static UINT32 wc_checkTag(TG* localTag, TG* remoteTag)
{
    UINT32 rc = NOT_DIRTY;
    TG* tag;
    BB* bbdTable;

    /*
    ** Determine which tag to check first.
    */
    if (remoteTag != NULL)
    {
        /*
        ** Check the remote tag.
        */
        tag = remoteTag;
        bbdTable = c_bbdTable[1];
    }
    else if (localTag != NULL)
    {
        /*
        ** Check the local tag.
        */
        tag = localTag;
        bbdTable = c_bbdTable[0];
    }
    else
    {
        /*
        ** Neither tag is to be checked.
        */
        return (NOT_DIRTY);
    }

    /*
    ** Clear the tag state to remove any left-over "Locked" bits that could
    ** prevent flushing the tag later.
    */
    tag->state = 0;

    /*
    ** Check if tag is dirty.
    */
    if (tag->attrib & (1<<TG_DIRTY))
    {
        /*
        ** Perform validity check on the cache tags.
        **
        ** Check if Virtual ID is valid.
        */
        if (tag->vid >= MAX_VIRTUAL_DISKS ||
            vcdIndex[tag->vid] == NULL ||
            MAG_VDMT_dir[tag->vid] == NULL)
        {
            rc = INVALID_VID;
        }
        else
        {
            if ((vcdIndex[tag->vid]->stat & (1 << VC_CACHED)) == 0)
            {
                /*
                ** Data exist in cache but caching is not enabled for this
                ** VID.  Set the Cached enable and disable in progress bits.
                */
                vcdIndex[tag->vid]->stat |= (1 << VC_CACHED)|
                                                    (1 << VC_DISABLE_IP);
            }

            /*
            ** Check if LBA is less than the virtual disk capacity.
            */
            if (tag->vsda >= MAG_VDMT_dir[tag->vid]->devCap)
            {
                rc = LBA_OUT_OF_RANGE;
            }

            /*
            ** Check if buffer pointer is valid.  Check if the
            ** Buffer pointer is less than DRAM starting address
            ** or greater than the DRAM data end address.
            */
            else if (tag->bufPtr < WcbAddr ||
                     tag->bufPtr > (WcbAddr+WcbSize))
            {
                rc = INVALID_TAG;
            }

            /*
            ** Check for an ECC error in the buffer data.
            */
            else if (wc_checkEccSite(tag->bufPtr, tag->vLen*512L, bbdTable))
            {
                rc = ECC_DATA_ERR;
            }
            /*
            ** When using both tags (i.e. both tags are dirty),
            ** check that the tags match.
            */
            else if ((localTag != NULL) && (remoteTag != NULL) &&
                     (localTag->attrib & (1<<TG_DIRTY)) &&
                     (remoteTag->attrib & (1<<TG_DIRTY)))
            {
                if (localTag->vid != remoteTag->vid ||
                    localTag->vsda != remoteTag->vsda ||
                    localTag->vLen != remoteTag->vLen ||
                    localTag->bufPtr != remoteTag->bufPtr)
                {
                    rc = TAG_MISMATCH;
                }
                else
                {
                    rc = DIRTY;
                }
            }
            else
            {
                rc = DIRTY;
            }
        }
#ifdef DEBUG_WCR
        if (rc != NOT_DIRTY)
        {
            if ((localTag != NULL) && (remoteTag != NULL))
            {
                fprintf(stderr, "wc_checkTag:  rc = %x, localTag = %p, "
                        "attrib = 0x%04x, vid = 0x%04x, bufPtr = %p, "
                        "vsda = 0x%08x\n"
                                "                     remoteTag = %p, "
                        "attrib = 0x%04x, vid = 0x%04x, bufPtr = %p, "
                        "vsda = 0x%08x\n",
                        rc, localTag, localTag->attrib, localTag->vid,
                        localTag->bufPtr, (UINT32)(localTag->vsda),
                        remoteTag, remoteTag->attrib, remoteTag->vid,
                        remoteTag->bufPtr, (UINT32)(remoteTag->vsda));
                if (rc != DIRTY)
                {
                    fprintf(stderr, "**********************************"
                    "**************************************************"
                    "*****************************\n");
                }
            }
        }
#endif
    } /* tag dirty */

    return rc;
} /* wc_checkTag */

/**
******************************************************************************
**
**  @brief      wc_checkForDirty
**
**              More details on this function go here.
**
**  @param      mode    - USE_LOCAL, USE_REMOTE or USE_BOTH
**
**  @return     TRUE    - contains dirty data.
**              FALSE   - does not contain any dirty data.
**
******************************************************************************
**/
static UINT32 wc_checkForDirty(UINT32 mode)
{
    UINT32 retValue = FALSE;
    UINT32 rc = 0;
    TG* localTag;
    TG* remoteTag = NULL;
    UINT32 i;
    UINT32 numberTags;

    localTag = WctAddr;
    numberTags = WctSize / sizeof(struct TG);

    /*
    ** Scan all the cache tag.
    */
    for (i = 0; i < numberTags; ++i)
    {
        /*
        ** Should we check the local cache?
        */
        if (mode == USE_LOCAL || mode == USE_BOTH)
        {
            /*
            ** Check if tag is valid and dirty.
            */
            rc = wc_checkTag(localTag, NULL);
        }

        /*
        ** Is the tag dirty?
        */
        if (mode == USE_REMOTE || (rc == DIRTY && mode == USE_BOTH))
        {
            /*
            ** Get the remote tag and copy into local memory.
            */
            remoteTag = wc_getRemoteTag(remoteTag, localTag);

            /*
            ** Check if remote tag matches the local tag.
            */
            rc = wc_checkTag(localTag, remoteTag);

            /*
            ** When checking both tags check for an invalid remote tag.
            ** If the remote tag is invalid and the local tag is dirty,
            ** indicate dirty data exists.
            */
            if (rc != NOT_DIRTY && mode == USE_BOTH)
            {
                /*
                ** Indicate dirty data was found.
                */
                retValue = TRUE;
                break;
            }
        }

        /*
        ** Check if this is a dirty tag.
        */
        if (rc == DIRTY)
        {
            /*
            ** Indicate dirty data was found
            */
            retValue = TRUE;
            break;
        }

        /*
        ** Increment to next tag.
        */
        ++localTag;
    }

    if (remoteTag != NULL)
    {
        /*
        ** Release memory used for the remote cache tag
        */
        s_Free(remoteTag, sizeof(struct TG), __FILE__, __LINE__);
    }

    return retValue;
} /* wc_checkForDirty */

/**
******************************************************************************
**
**  @brief      Check whether the requested NV Memory contains data
**              that needs to be flushed to disk.
**
**              The "magic" number stored in the NV Memory is checked
**              for the value indicating the memory contains data
**              that needs to be flushed to disk.
**
**  @param      mode    - USE_LOCAL, USE_REMOTE or USE_BOTH
**
**  @return     none
**
******************************************************************************
**/
static void wc_checkCache(UINT32 mode)
{
    WT  *signatureBlock;
    UINT32       logEvent;

    /*
    ** Check if the remote or the local cache is to be examined.
    */
    if (mode == USE_LOCAL)
    {
        /*
        ** Use the signature block in the local cache
        */
        signatureBlock = WcctAddr;
#ifdef DEBUG_WCR
        fprintf(stderr, "%s:  mode = USE_LOCAL, signatureBlock = %p\n",
            __func__, signatureBlock);
#endif
    }
    else
    {
        /*
        ** Get the mirrored signature block from the remote cache
        */
        signatureBlock = wc_getRemoteData(NULL, WcctAddr, sizeof(struct WT),
                                                        1 << CAC0_WCCT);
#ifdef DEBUG_WCR
        fprintf(stderr, "%s:  mode = USE_REMOTE, signatureBlock = %p\n",
            __func__, signatureBlock);
#endif
    }

    /*
    ** Check if the signature block is valid.
    */
    if (signatureBlock == NULL)
    {
        return;
    }

#ifdef DEBUG_WCR
    fprintf(stderr, "%s:  signature = 0x%X " "(DATA_CACHED = 0x%X)\n",
            __func__, signatureBlock->signature1, DATA_CACHED);
#endif
    /*
    ** Check for the signature indicating the write cache was enabled
    ** and may contain dirty data.
    ** NOTE:  If no data is cached, there is no need to validate the
    **        ownership (S/N) of the NV Memory at this point.
    */
    if (signatureBlock->signature1 == DATA_CACHED &&
        signatureBlock->signature2 == DATA_CACHED)
    {
        /*
        ** Check for the controller serial number in the DRAM and compare
        ** to the value stored in Firmware Init. Control Block
        ** If mismatch, check if the write cache contains any dirty data.
        */
        if (signatureBlock->cSerial != K_ficb->cSerial &&
            wc_checkForDirty(mode))
        {
            /*
            ** Check if this is an N-way system with the NV Memory swapped.
            */
            if (signatureBlock->vcgID == K_ficb->vcgID)
            {
                /*
                ** The NV Memory belongs to another controller in this VCG.
                ** Ask the User if they want to move the NV Memory to the
                ** owning controller.
                */
                fprintf(stderr, "%s:  NV Memory in wrong "
                                            "controller within our VCG!\n"
                                "                Owning S/N = 0x%X, "
                                            "our S/N = 0x%X, VCGID = 0x%X\n"
                                ">>>>>>>>>> USER INTERVENTION REQUIRED <<<"
                                            "<<<<<<<\n",
                        __func__, signatureBlock->cSerial, K_ficb->cSerial,
                        K_ficb->vcgID);
                logEvent = LOG_WC_SN_VCG_BAD;
            }
            else
            {
                /*
                ** The NV Memory belongs to a controller outside our VCG.
                ** Ask the User if the Controller Serial Number needs
                ** to be restored or if the NV Memory needs be moved to
                ** the Controller with the matching controller S/N.
                */
                fprintf(stderr, "%s:  NV Memory in wrong "
                                            "controller!\n"
                                "                Owning S/N = 0x%X, "
                                            "our S/N = 0x%X\n"
                                "                Owning VCGID = 0x%X, "
                                            "our VCGID = 0x%X\n"
                                ">>>>>>>>>> USER INTERVENTION REQUIRED <<<"
                                            "<<<<<<<\n",
                        __func__, signatureBlock->cSerial, K_ficb->cSerial,
                        signatureBlock->vcgID, K_ficb->vcgID);
                logEvent = LOG_WC_SN_BAD;
            }

            /*
            ** Wait for the User to respond.
            */
            if (wc_PauseCacheInit(logEvent, signatureBlock->cSerial)
                                                                == FALSE)
            {
                /*
                ** User responded negative - don't use this part of cache.
                */
                if (mode == USE_LOCAL)
                {
                    c_recoveryMode &= ~USE_LOCAL;
                }
                else /* remote, or both */
                {
                    c_recoveryMode &= ~USE_REMOTE;
                }
            }
            else
            {
                /*
                ** Fix the controller S/N in the DRAM signature copy
                */
                signatureBlock->cSerial = K_ficb->cSerial;
            }
        }

        /*
        ** Check for the controller serial number in the cache and compare
        ** to the value stored in Firmware Init. Control Block
        */
        if (K_ficb->cSerial == signatureBlock->cSerial)
        {
#ifdef DEBUG_WCR
            fprintf(stderr, "%s:  Beginning seqNo = 0x%x\n",
                                __func__, seqNo);
#endif
            if (mode == USE_LOCAL)
            {
                /*
                ** Use the local sequence number.
                */
                seqNo = signatureBlock->seq;

                /*
                ** The local serial number matches.  Use the local cache.
                */
                c_recoveryMode = USE_LOCAL;
            }
            else if (c_recoveryMode == USE_NEITHER)
            {
                /*
                ** Use the remote sequence number.
                */
                seqNo = signatureBlock->seq;

                /*
                ** The local cache is not being used.
                ** The remote serial number matches.  Use the remote cache.
                */
                c_recoveryMode = USE_REMOTE;
            }
            else if (seqNo == signatureBlock->seq)
            {
                /*
                ** The sequence number in the local and remote
                ** signature blocks match.  Use both caches.
                */
                c_recoveryMode = USE_BOTH;
            }
            else if (seqNo < signatureBlock->seq)
            {
                /*
                ** Use the remote sequence number.
                */
                seqNo = signatureBlock->seq;

                /*
                ** The sequence number in the remote signature block
                ** is newer.  Only use the remote cache.
                */
                c_recoveryMode = USE_REMOTE;
            }
#ifdef DEBUG_WCR
            fprintf(stderr, "%s:  Ending seqNo = 0x%x, c_recoveryMode = 0x%x\n",
                __func__, seqNo, c_recoveryMode);
#endif
        } /* S/N match */
    } /* Data cached */

    /*
    ** If memory was allocated for the signature block, free it.
    */
    if (WcctAddr != signatureBlock)
    {
        s_Free(signatureBlock, sizeof(struct WT), __FILE__, __LINE__);
    }
} /* wc_checkCache */


/**
******************************************************************************
**
**  @brief      Verify the sequence number stored in the write cache
**              DRAM is valid.
**
**              The seq number was previously set in the wc_checkCache routine,
**              based on whether we using the local, remote, or both caches.
**              Verify the sequence number is equal to or one less than
**              the sequence number stored in the NVRAM.  If the sequence number
**              does not match, a dialog occurs with the browser to ask the user
**              whether to proceed with the out-of-date sequence number.
**              User may respond Yes or No; if No, the recovery mode is adjusted
**              to indicate no cached data will be flushed.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void wc_checkSeqNo(void)
{
    /*
    **  Check if either cache contains data to recover
    */
    if (c_recoveryMode != USE_NEITHER)
    {
        /*
        ** Check if the sequence number doesn't match.
        ** Allow for the value in the DRAM to be 1 larger than the FICB value.
        ** The value in the FICB was restored from NVRAM.  This handles the case
        ** when the controller is powered off prior to the new sequence number
        ** being written to NVRAM.
        */
        if ((K_ficb->seq != seqNo) && ((K_ficb->seq+1) != seqNo))
        {
            /*
            ** Check if the cache contains any dirty data
            */
            if (wc_checkForDirty(c_recoveryMode) == TRUE)
            {
                /*
                ** Prompt user for next action, and wait for their response.
                */
                if (wc_PauseCacheInit(LOG_WC_SEQNO_BAD, seqNo) == FALSE)
                {
                    /*
                    ** User responded negative - don't use the cache.
                    */
                    c_recoveryMode = USE_NEITHER;
                }
            }
        }
    }
} /* wc_checkSeqNo */

/**
******************************************************************************
**
**  @brief      Determine if recovery of cache tag is necessary and
**              initialize the cache tag memory to enable recovery to occur.
**
**              Checks are made to see if the cache is in a state that
**              requires recovery.  If so each tag is checked for dirty.
**              If the tag is dirty, validity check is performed on the tag.
**              If the tag is not dirty, the tag is added to the free list.
**              A count of the dirty tag is maintained.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void wc_recoveryInit(void)
{
    UINT32 rc = 0;
    TG* localTag;
    TG* remoteTag = NULL;
    TG* previousTag = NULL;
    UINT32 i;
    UINT8 * compareBuffer = NULL;
    UINT32 numberTags;
    UINT32 rcCmp;

#ifdef DEBUG_WCR
    fprintf(stderr, "wc_recoveryInit:  Starting...\n");
#endif

    /*
    ** Check if the cache contains data to be flushed
    */
    wc_checkCache(USE_LOCAL);

    /*
    ** If Mirror Partner is okay, check remote cache
    */
    if ((C_ca.status & (1<<CA_MIRRORBROKEN)) == 0)
    {
        wc_checkCache(USE_REMOTE);
    }

    /*
    **  Check if recovery will be attempted.
    */
    if (c_recoveryMode != USE_NEITHER)
    {
        /*
        ** Check if the sequence number is correct.
        */
        wc_checkSeqNo();
    }

#ifdef DEBUG_WCR
    fprintf(stderr, "wc_recoveryInit: c_recoveryMode = 0x%X\n", c_recoveryMode);
#endif

    /*
    ** Recheck the recovery mode since it may have been changed
    ** by the sequence number check.
    */
    if (c_recoveryMode != USE_NEITHER)
    {
        /*
        ** Assign memory for the vid failed table
        */
        c_vidFailed = s_MallocC(MAX_VIRTUAL_DISKS*2, __FILE__, __LINE__);

        if (c_recoveryMode != USE_LOCAL)
        {
            /*
            ** Allocate memory for the remote tag.
            */
            remoteTag = s_MallocC(sizeof(struct TG), __FILE__, __LINE__);
        }

        if (c_recoveryMode == USE_BOTH)
        {
            /*
            ** Allocate memory for the compare buffer.
            */
            compareBuffer = s_MallocW(512, __FILE__, __LINE__);
        }
    }

    /*
    ** Set up for the loop.
    */
    localTag = WctAddr;
    numberTags = WctSize / sizeof(struct TG);

    /*
    ** Scan all the cache tag.
    */
    for (i = 0; i < numberTags; ++i)
    {
        if (c_recoveryMode == USE_NEITHER)
        {
            rc = NOT_DIRTY;
        }
        else if (c_recoveryMode == USE_BOTH)
        {
            /*
            ** If using both caches, check ECC in local tag, if ECC error
            ** refresh local tag from the remote tag assuming no ECC errors
            ** in remote tag.  If ECC errors in both tags, report an error.
            */
            if (wc_checkTagEcc(localTag, c_bbdTable[0]) == FALSE)
            {
                /*
                ** Tag is OK - check if tag is valid and dirty.
                */
                rc = wc_checkTag(localTag, NULL);

                /*
                ** Is the tag dirty?
                */
                if (rc == DIRTY)
                {
                    /*
                    ** Get the remote tag and copy into local memory.
                    */
                    wc_getRemoteTag(remoteTag, localTag);

                    /*
                    ** Check if remote tag matches the local tag.
                    */
                    rc = wc_checkTag(localTag, remoteTag);

                    if (rc != NOT_DIRTY)
                    {
                        /*
                        ** Ignore the error in the remote cache and flush
                        ** the data associated with this cache tag.
                        */
                        rc = DIRTY;
                    }
                }
                else if (rc != NOT_DIRTY)
                {
                    if (wc_checkTagEcc(remoteTag, c_bbdTable[1]) == FALSE)
                    {
                        /*
                        ** Tag is OK - get the remote tag into local memory.
                        */
                        wc_getRemoteTag(remoteTag, localTag);

                        /*
                        ** Check if tag is valid and dirty.
                        */
                        rc = wc_checkTag(NULL, remoteTag);
                    }
                }
            }
            else
            {
                if (wc_checkTagEcc(remoteTag, c_bbdTable[1]) == FALSE)
                {
                    /*
                    ** Get the remote tag and copy into local memory.
                    */
                    wc_getRemoteTag(remoteTag, localTag);

                    /*
                    ** Check if remote tag is valid and dirty.
                    */
                    rc = wc_checkTag(NULL, remoteTag);
                }
                else
                {
                    /*
                    ** Both the local tag and remote tag contain ECC errors.
                    ** It cannot be determined if this tag contained dirty data.
                    ** Send a log message.
                    */
                    rc = ECC_TAG_ERR;
                }
            }

            if (rc == DIRTY)
            {
                /*
                ** Check that the first block of the local data and
                ** the remote data matches.
                */
                if (wc_getRemoteData(compareBuffer,
                                     localTag->bufPtr,
                                     512L,
                                     1<<CAC0_DATA) == NULL)
                {
                    /*
                    ** Unable to get the remote.  This cache tag
                    ** cannot be flushed.  Report an error.
                    */
                    rc = REMOTE_ACCCESS_ERR;
                }
                else if ((rcCmp = MSC_MemCmp(compareBuffer,
                                             localTag->bufPtr,
                                             512)) != 0)
                {
                    fprintf(stderr, "wc_recoveryInit:  Local and remote data "
                            "do not match.  RC = 0x%x\n", rcCmp);
                    rc = DATA_MISMATCH;
                }
            }
        }
        else if (c_recoveryMode == USE_LOCAL)
        {
            /*
            ** If using only local cache, check ECC in local tag.
            ** If ECC error report an error.
            */
            if (wc_checkTagEcc(localTag, c_bbdTable[0]) == FALSE)
            {
                /*
                ** Tag is OK - check if tag is valid and dirty.
                */
                rc = wc_checkTag(localTag, NULL);
            }
            else
            {
                /*
                ** The tag contains ECC errors.
                ** It cannot be determined if this tag contained dirty data.
                ** Send a log message.
                */
                rc = ECC_TAG_ERR;
            }
        }
        else if (c_recoveryMode == USE_REMOTE)
        {
            /*
            ** If using only remote cache, check ECC in remote tag.
            ** If ECC error report an error.
            */
            if (wc_checkTagEcc(remoteTag, c_bbdTable[1]) == FALSE)
            {
                /*
                ** Tag is OK - get the remote tag and copy into local memory.
                */
                wc_getRemoteTag(remoteTag, localTag);

                /*
                ** Check if remote tag is valid and dirty.
                */
                rc = wc_checkTag(NULL, remoteTag);

                /*
                ** Is the tag dirty?
                */
                if (rc == DIRTY)
                {
                    /*
                    ** Copy the write cache data from the remote cache
                    ** into the local cache.
                    */
                    if (wc_getRemoteData(remoteTag->bufPtr,
                                         remoteTag->bufPtr,
                                         remoteTag->vLen * 512L,
                                         1<<CAC0_DATA) == NULL)
                    {
                        /*
                        ** Unable to get the remote.  This cache tag
                        ** cannot be flushed.  Report an error.
                        */
                        rc = REMOTE_ACCCESS_ERR;
                    }
                    else
                    {
                        /*
                        ** Update the local tag to match the remote tag.
                        */
                        memcpy(localTag, remoteTag, sizeof(struct TG));
                    }
                }
            }
            else
            {
                /*
                ** The tag contains ECC errors.
                ** It cannot be determined if this tag contained dirty data.
                ** Send a log message.
                */
                rc = ECC_TAG_ERR;
            }
        }

        /*
        ** Check if the tag was dirty.
        */
        if (rc == DIRTY)
        {
            /*
            ** Only use 1/4 of the available ILTs for flushing
            ** so amount of memory used is limited.
            */
            while (c_rcDirtyCount > (INITIAL_ILTS / 4))
            {
                TaskSleepMS(125);           /* Delay for short time         */
            }

            /*
            ** Increment dirty tag count
            */
            ++c_rcDirtyCount;
            ++ecrc.data.totalDirty;

#ifdef DEBUG_WCR
            fprintf(stderr, "wc_recoveryInit:  Flushing tag %p.  "
                    "Dirty count = %X\n", localTag, c_rcDirtyCount);
#endif
            /*
            **  Flush the data for this cache tag to disk
            */
            wc_recoveryFlushRequest(localTag);

            /*
            ** Allow the flush of this tag to start.
            */
            TaskSwitch();
        }
        else
        {
            if (rc != NOT_DIRTY)
            {
                /*
                ** Both the local tag and remote tag contain errors.
                ** Send a log message.
                */
                wc$msgCacheRecoverFail(rc, localTag);

                /*
                ** Keep a count of the tags that failed.
                */
                ++ecrc.data.totalFailed;
            }

            /*
            ** Add tag to free list.
            */
            if (previousTag != NULL)
            {
                /*
                ** Set the forward pointer in the previous tag to
                ** point to this tag.  Note that only the forward
                ** tag is used in the free list.  The backward tag
                ** is cleared when the tag is initialized.
                */
                previousTag->fthd = localTag;

                /*
                ** Initialize cache tag fields.
                */
                WC_initTag(previousTag);
            }
            else if (c_tgfree == NULL)
            {
                /*
                ** A tag has not been freed yet.  If a dirty tag
                ** was flushed, it could have been freed by now.
                ** Set the start of the free list.
                */
                c_tgfree = localTag;
            }

            /*
            ** Set this tag as the previous tag.
            */
            previousTag = localTag;
        }

        /*
        ** Increase to next cache tag
        */
        ++localTag;
    } /* loop scanning all cache tags */

    /*
    ** Check if free tag exist.
    */
    if (previousTag != NULL)
    {
        /*
        ** Set the forward pointer for the last tag to NULL
        ** since no tag follows this tag.
        */
        previousTag->fthd = NULL;

        /*
        ** Initialize the last cache tag fields.
        */
        WC_initTag(previousTag);
    }

    if (c_recoveryMode != USE_NEITHER)
    {
        /*
        ** Check for errors detected during the scan of the cache tags.
        */
        for (i = 0; i < MAX_VIRTUAL_DISKS; ++i)
        {
            /*
            ** Get failed count for each VID and generate a log message
            ** if greater than the ERROR LIMIT
            */
            if (c_vidFailed[i] > ERROR_LIMIT)
            {
                LOG_VID_RECOVERY_FAIL_PKT esb;

                esb.header.event = LOG_VID_RECOVERY_FAIL;
                esb.data.vid = i;
                esb.data.count = c_vidFailed[i];

                /*
                 * Note: message is short, and L$send_packet copies into the MRP.
                 */
                MSC_LogMessageStack(&esb, sizeof(LOG_VID_RECOVERY_FAIL_PKT));
            }
        }

        /*
        ** Check if any tags were dirty.
        */
        if (ecrc.data.totalDirty != 0)
        {
            LOG_CACHE_TAGS_RECOVERED_PKT tmp_ecrc;

            /*
            ** Send an information log message to the CCB
            */
            tmp_ecrc.header.event = LOG_CACHE_TAGS_RECOVERED;
            tmp_ecrc.data.totalDirty = ecrc.data.totalDirty;
            tmp_ecrc.data.totalFailed = ecrc.data.totalFailed;

            /*
             * Note: message is short, and L$send_packet copies into the MRP.
             */
            MSC_LogMessageStack(&tmp_ecrc, sizeof(LOG_CACHE_TAGS_RECOVERED_PKT));
        }

        /*
        ** Release memory used for the vid failed table
        */
        s_Free(c_vidFailed,MAX_VIRTUAL_DISKS*2, __FILE__, __LINE__);
        c_vidFailed = 0;                    /* Make sure we do not continue to use this. */

        if (remoteTag != NULL)
        {
            /*
            ** Release memory used for the remote cache tag
            */
            s_Free(remoteTag, sizeof(struct TG), __FILE__, __LINE__);
        }

        if (compareBuffer != NULL)
        {
            /*
            ** Release memory used for the compare buffer.
            */
            s_Free(compareBuffer, 512, __FILE__, __LINE__);
        }

        /*
        ** Wait until flush complete by check the dirty tag count.
        */
        while (c_rcDirtyCount != 0)
        {
            TaskSleepMS(10);                /* Delay for short time         */
        }

        /*
        ** Check if flush completed with errors.
        */
        if (c_flushErrorCount == 0)
        {
            /*
            ** Flush completed without error.
            */
            c_resumeInit[1] = DEOK;
        }
        else
        {
            /*
            ** Flush completed with error, set error code
            */
            c_resumeInit[1] = DEWCRECVRYFAILED;
        }
    }
    /*
    ** Mirror the data to the NV Memory (Local) and the Mirror Partner's NV
    ** Memory (Remote)
    */
    WC_CopyNVwWait(WctAddr, WctAddr, MIRROR_MP, WctSize);
    /*
    ** Stop the cache layer.  The cache layer is turned back on when
    ** all the initial dirty data (if any) is flushed to disk.
    */
    C$stop(STOP_OPTIONS, STOP_WCRECVRY_USER);
} /* wc_recoveryInit */

/**
******************************************************************************
**
**  @brief      Prompts for User Response from the CCB.
**
**              This function reports the write cache incident to the CCB, then
**              waits for the corresponding user response to be returned.
**
**  @param      event - log event
**
**  @return     none
**
******************************************************************************
**/
UINT8 wc_PauseCacheInit(UINT32 event, UINT32 data)
{
    LOG_WC_SEQNO_BAD_PKT erc;

    /*
    ** Initialize the resume init signal.
    */
    c_resumeInit[0] = 0xFF;
    c_resumeInit[1] = 0xFF;

    /*
    ** Send appropriate message to CCB/ICON.
    */
    switch(event)
    {
        case LOG_WC_SEQNO_BAD:                  /* Bad sequence number      */
        {
            erc.header.event = event;
            erc.data.cacheId = c_recoveryMode;
            erc.data.sysSeq = K_ficb->seq;
            erc.data.seq = data;
            /*
             * Note: message is short, and L$send_packet copies into the MRP.
             */
            MSC_LogMessageStack(&erc, sizeof(LOG_WC_SEQNO_BAD_PKT));
        }
        break;

        case LOG_WC_SN_VCG_BAD:                 /* Swapped w/ CN in VCG     */
        case LOG_WC_SN_BAD:                     /* Swapped w/ CN outside VCG*/
        {
            erc.header.event = event;
            erc.data.cacheId = c_recoveryMode;
            erc.data.sysSeq = K_ficb->cSerial;
            erc.data.seq = data;
            /*
             * Note: message is short, and L$send_packet copies into the MRP.
             */
            MSC_LogMessageStack(&erc, sizeof(LOG_WC_SEQNO_BAD_PKT));
        }
        break;

        case LOG_WC_NVMEM_BAD:
        {
            /*
            ** TBD - need new log event & processing here
            */
            fprintf(stderr, "wc_PauseCacheInit:  NV Memory unavailable, "
                                                    "status = 0x%X.\n", data);
            erc.header.event = event;
            erc.data.cacheId = c_recoveryMode;
            erc.data.sysSeq = K_ficb->cSerial;
            erc.data.seq = data;
            /*
             * Note: message is short, and L$send_packet copies into the MRP.
             */
            MSC_LogMessageStack(&erc, sizeof(LOG_WC_SEQNO_BAD_PKT));
        }

        default:
        {
            fprintf(stderr, "wc_PauseCacheInit:  Unrecognized event 0x%X "
                                                        "received.\n", event);
        }
    }

    /*
    ** Wait for the response.
    */
    while (c_resumeInit[0] == 0xFF)
    {
        TaskSwitch();
    }

    return c_resumeInit[0];
} /* wc_PauseCacheInit */


/**
******************************************************************************
**
**  @brief      Receives User Response from the CCB.
**
**              This function reports the user response to the request
**              to complete cache initialization.
**
**  @param      response    - User Response.
**
**  @return     Return code.
**
******************************************************************************
**/
UINT8 WC_resumeCacheInit(UINT8 response)
{
    /*
    ** Set the user response - this unblocks the waiting WC recovery code.
    */
    c_resumeInit[0] = response;
#if 0 /* LSW - remove until we decide whether to keep this part of the design */
    /*
    ** Wait until an error code is set (by WC recovery itself).
    */
    do
    {
        TaskSwitch();
    } while (c_resumeInit[1] == 0xFF);

    /*
    ** Get error code from resume init.  Return the upper byte.
    */
    return c_resumeInit[1];
#else
    return DEOK;
#endif
} /* WC_resumeCacheInit */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
