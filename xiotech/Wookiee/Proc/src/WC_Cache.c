/* $Id: WC_Cache.c 145151 2010-08-05 16:32:49Z m4 $ */
/**
******************************************************************************
**
**  @file       WC_Cache.c
**
**  @brief      Write Cache - Main Routines
**
**  Contains the Main Routines for the Write Cache Functions.
**
**  Copyright (c) 2005-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#include "wcache.h"                     /* Write Cache Definitions          */

#include "CA_CI.h"                      /* Cache definitions                */
#include "datagram.h"                   /* Datagram definitions             */
#include "dlmfe.h"                      /* FE DLM functions                 */
#include "drp.h"                        /* DG Request Packet definitions    */
#include "ecodes.h"                     /* Error Code definitions           */
#include "ilt.h"                        /* ILT definitions                  */
#include "isp.h"
#include "LOG_Defs.h"                   /* Logging definitions              */
#include "misc.h"                       /* Miscellaneous definitions        */
#include "MR_Defs.h"                    /* MRP definitions                  */
#include "NV_Memory.h"                  /* Non-volatile memory definitions  */
#include "options.h"                    /* Run Time Compile options         */
#include "OS_II.h"                      /* II Structure definitions         */
#include "pcb.h"                        /* PCB definitions                  */
#include "pm.h"                         /* Packet Management definitions    */
#include "qu.h"                         /* Queue definitions                */
#include "QU_Library.h"                 /* Queueing functions               */
#include "sgl.h"                        /* Scatter/Gather List definitions  */
#include "XIO_Const.h"                  /* Standard Xiotech constants       */
#include "XIO_Macros.h"                 /* Standard Xiotech macros          */
#include "XIO_Std.h"                    /* Standard Xiotech functions       */
#include "XIO_Types.h"                  /* Standard Xiotech types           */
#include "mem_pool.h"
#include "CT_defines.h"

#include <stdio.h>
#include <stdlib.h>
#include <byteswap.h>

/*
******************************************************************************
** Private defines - constants
******************************************************************************
*/
/* #define DEBUG_MIRROREXEC        1       PrintF on Mirror Exec Function   */
/* #define DEBUG_DMACOMP           1       PrintF on Mirror Exec Function   */
/* #define DEBUG_MDRPCOMP          1       PrintF on Mirror DRP Complete    */
/* #define DEBUG_DRPCHAINCOMP      1       PrintF on Mirror DRP Chain Comp  */
/* #define DEBUG_MIRRORBE          1       PrintF on Mirror BE Function     */
/* #define DEBUG_MIRRORBECOMP      1       PrintF on Mirror BE Complete     */
/* #define DEBUG_COPYNVWWAIT       1       PrintF on Copy NV with Wait      */
/* #define DEBUG_MIRRORBETAGEXEC   1       PrintF on Mirror BE Tag Exec     */

/*
******************************************************************************
** Private variables
******************************************************************************
*/
LOG_FIRMWARE_ALERT_PKT eSoftFault_WC_Cache; /* Log Message for Software Faults  */

/*
******************************************************************************
** Public variables - externed in the header file
******************************************************************************
*/
extern CA C_ca;                     /* Cache Structure                      */
extern UINT32 C_orc;                /* Cache layer Outstanding Req Count    */
extern QU gMirrorQueue;             /* Mirror Queue Structure               */
extern QU gWCMirrorBETagQueue;      /* Mirror BE Tag Queue Structure        */
extern INT16 wc_mem_seqnum;         /* Write Cache DRP Sequence Number      */
extern UINT32 C_mirror_error_flag;  /* Mirror Error Flag                    */
extern UINT32 gWCErrorMP;           /* Mirror Error MP Serial Number        */

extern UINT32 WcctSize;             /* Size of the Write Cache Control Table */
extern WT *WcctAddr;                /* Address of the Write Cache Control Tbl */
extern UINT32 WccSize;              /* Size of the Write Cache Configuration */
extern UINT8 *WccAddr;              /* Write Cache Configuration area       */
extern UINT32 WctSize;              /* Size of the Write Cache Tag area     */
extern TG *WctAddr;                 /* Write Cache Tag area                 */
extern UINT32 WcbSize;              /* Size of the Write Cache Buffer area  */
extern UINT8 *WcbAddr;              /* Write Cache Buffer area              */

WC_NV gWC_NV_Mirror;                /* Write Cache NV Mirror Information    */

extern UINT32 C_MaxTotalFlushBlocks;
extern UINT32 C_MaxDirtyBlocks;
extern UINT32 C_BlockStartFlushThresh;
extern UINT32 C_BlockStopFlushThresh;
extern UINT32 C_BlockFlushIncr;

/*
******************************************************************************
** Public Functions not externed in header files
******************************************************************************
*/
extern void KernelDispatch(UINT32 rc, ILT *pILT, MR_PKT *pMRP, UINT32 w0);
extern void CT_LC_WC_MirrorDRPComplete(UINT32, void *);

extern void WC_MirrorDRPComplete(UINT32 rc, struct ILT *pILT);
extern UINT32 WC_CheckBypass(void);
extern void CA_LogMirrorFailure(UINT32 rc, struct ILT *pILT);
extern void WC_MirrorExec(void);
extern void WC_RestoreData(void);
extern void WC_CopyNVwWait(void *dst, void *src, UINT8 where, UINT32 length);
extern void WC_MirrorBETagExec(void);
extern void WC_MirrorBE(struct ILT *pILT, struct DLM_REQ *pDRP);

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
static void WC_DMAComplete(struct NV_DMA_RSP_PKT *pRsp);
static void WC_DRPChainComplete(UINT32 rc, struct ILT *pILT);
static void WC_RestoreDataComp(struct NV_DMA_RSP_PKT *pRsp);
static void WC_NV_DMARequestComp(struct NV_DMA_RSP_PKT *pRsp);
static void WC_MirrorBEComplete(struct NV_DMA_RSP_PKT *pRsp);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      WC_MirrorExec - Mirrors Tags and Data to MMC and Mirror Partner
**
**              To perform a mirror operation on cache tags and cache buffers.
**              This entails mirroring to any Mirror Partner already assigned
**              in an N>1 environment.  Also mirrors to the MMC the data for
**              power loss protection.
**
**              This executive processes a queue of supplied ILT/placeholders
**              that each refer to a particular cache tag.  Command bits
**              specify what mirror action is to be taken.  When the requested
**              action has been completed the completion routine in the
**              ILT/placeholder is called.  Any tag locking must have been
**              done prior to queuing the request.  This executive does not
**              unlock the tag; it sets the <TG_MIRRIP> bit in the tag
**              attribute before the mirror operation starts and then clears
**              it after the operation completes.
**
**  @param      none
**
**  @return     none
**
**  @attention  This is a task that never returns.  To enqueue an
**              ILT/placeholder to this task see the <wc$q> function.
**              The ILT/placeholder must have:
**                  <plctag>    - Cache tag for operation.
**                  <plmcmd>    - Command code for mirror operation.
**
**                  Bit definitions for <plmcmd>
**
**                      MIRROR_TAG     - Mirror tag
**                      MIRROR_BUFFER  - Mirror buffer
**
**                      Both bits set implies mirror of buffer before
**                      mirror of tag.
**
**                      Valid combinations are:
**                          Mirror Only the buffer (1 DMA Descriptor needed)
**                          Mirror one buffer and one tag (2 DDs needed)
**                          Mirror one or more tags (1-n DDs needed)
**
**                  <il_cr>     - Completion routine to call when done with
**                      first parm  - Completion Status
**                      second parm - ILT/Placeholder
**
******************************************************************************
**/
NORETURN
void WC_MirrorExec(void)
{
    UINT32 dmaDescriptorCount = 0;      /* Number of DMA descriptors        */
    UINT32 workingDescriptorCount;      /* Amount of DMA descriptors left   */
    struct ILT  *pWorkingILT = NULL;    /* ILT being processed              */
    struct TG   *pTag = NULL;           /* Tag being processed              */
    struct ILT  *pMirrorPlaceholder = NULL; /* Mirror Placeholder ILT       */
    struct NV_DMA_DD *DMAList = NULL;   /* DMA List (used as an array)      */
    UINT32 dmaListLength = 0;           /* DMA List Length                  */
    UINT32 i = 0;                       /* Array Index                      */
    UINT32 rc = EC_OK;                  /* Return Code                      */
    struct ILT  *pDRPILT;               /* ILT used for DRP Processing      */
    UINT32 drpDescriptorCount = 0;      /* Number of Descriptors in this DRP */
    UINT32 drpSize = 0;                 /* Size of DRP, Datagram Request and
                                           Datagram Response                */
    struct DLM_REQ *pDRP;               /* DRP                              */
    struct DATAGRAM_REQ *pDGReq;        /* Datagram Request                 */
    struct CAC0 *pCAC0Entry;            /* Extended Request Information     */
    struct DATAGRAM_RSP *pDGRsp;        /* Datagram Response                */
    struct SGL  *pSGL;                  /* SGL Header                       */
    SGL_DESC *pSGLDesc;                 /* SGL Descriptor                   */

    /* Setup queue control block */
    gMirrorQueue.pcb = K_xpcb;          /* Set up my PCB                    */
    gMirrorQueue.head = NULL;           /* Set up the Head ILT              */
    gMirrorQueue.tail = NULL;           /* Set up the Tail ILT              */
    gMirrorQueue.qcnt = 0;              /* Set up the Count of ILTs on queue*/

    /* Task runs forever */
    while (TRUE)                        /* Always keep task running         */
    {
        /* Set process to wait for signal and Exchange process */
        TaskSetMyState(PCB_NOT_READY);
        TaskSwitch();

        /* Loop on items queued */
        while (gMirrorQueue.head != NULL)
        {
            dmaDescriptorCount = 0;     /* Clear the number of DDs needed   */

            /* Get the head ILT mirror request */
            pWorkingILT = QU_DequeReqILT(&gMirrorQueue);
#ifdef DEBUG_MIRROREXEC
            fprintf(stderr, "WC_Cache ME: Main ILT = %p; CaStatus = 0x%x; Command = 0x%x\n",
                            pWorkingILT, C_ca.status, pWorkingILT->plCmd);
#endif

            /*
             * If in an N>1 environment and the Mirror State is Error, then do
             * not even attempt to save the Information in the NV Card nor send
             * to the Mirror Partner.  Return the request to the caller of the
             * Error State.  Once out of the error state, then the mirror
             * function will be called again if needed
             */
            if ((BIT_TEST(C_ca.status,CA_NWAYMIRROR)) &&
                (BIT_TEST(C_ca.status,CA_ERROR)))
            {
#ifdef DEBUG_MIRROREXEC
                fprintf(stderr, "WC_Cache ME: N-Way in Error State\n");
#endif
                KernelDispatch(EC_IO_ERR, pWorkingILT, NULL, 0); /* Error out */
                continue;               /* Get the next ILT on the input list */
            }

            /*
             * Set up the DD List to DMA to the NV FE Areas and if N=1 to the
             * BE as well.
             *
             * Determine how big of a DMA Descriptor (DD) list needs to be
             * allocated to do the transfer to the NV areas.
             */
            pTag = pWorkingILT->wc_mirror.pplTag;   /* Get the tag this mirror is based on*/
            BIT_SET(pTag->attrib,TG_MIRRIP);        /* Show the Tag as being mirrored
                                                       even if only the Buffer is */
#ifdef DEBUG_MIRROREXEC
            fprintf(stderr, "WC_Cache ME: Main Tag - Mirror In Progress; Tag = %p\n", pTag);
#endif

            if (pWorkingILT->wc_mirror.wc_mirror_tags.plMirrorBuffer)
            {
                /* Increment the number of DDs needed*/
                dmaDescriptorCount++;

                /*
                 * Only one tag to mirror at most, so clear out the Next Dirty
                 * pointer so that we do not pick up old info.
                 */
                pTag->nextDirty = NULL;
            }

            if (pWorkingILT->wc_mirror.wc_mirror_tags.plMirrorTag)
            {
                dmaDescriptorCount++;   /* Increment the number of DDs needed*/
                while ((pTag = pTag->nextDirty) != NULL)
                {
                    BIT_SET(pTag->attrib,TG_MIRRIP); /* Tag is being mirrored */
                    dmaDescriptorCount++;   /* Incr the number of tags needed */
#ifdef DEBUG_MIRROREXEC
                    fprintf(stderr, "WC_Cache ME: Chained Tag - Mirror In Progress; Tag = %p\n", pTag);
#endif
                }
                pTag = pWorkingILT->wc_mirror.pplTag; /* Get the head tag again       */
            }

            if (!(BIT_TEST(C_ca.status,CA_NWAYMIRROR)))
            {
                dmaDescriptorCount *= 2; /* Double the number of DDs needed */
#ifdef DEBUG_MIRROREXEC
                fprintf(stderr, "WC_Cache ME: 1-Way double descriptors\n");
#endif
            }

            /*
             * Allocate a Placeholder ILT to save this information in,
             * allocate the DD Buffer area, and then set up the DD Lists
             */
            pMirrorPlaceholder = get_wc_plholder();
            dmaListLength = dmaDescriptorCount * sizeof(NV_DMA_DD);
            DMAList = s_MallocC(dmaListLength, __FILE__, __LINE__);
            pMirrorPlaceholder->misc = dmaListLength;
            pMirrorPlaceholder->cache_2_dlm.pcdDMAList = DMAList;
            pMirrorPlaceholder->cache_2_dlm.pcdPhILT = pWorkingILT;
            /* Preset to show 1 DRP so that an quick DMA does not complete op too soon */
            pMirrorPlaceholder->cache_2_dlm.cdNumDRPs = 1;
            pMirrorPlaceholder->cache_2_dlm.cdDRPec = EC_OK; /* Show all OK for now     */

#ifdef DEBUG_MIRROREXEC
            fprintf(stderr, "WC_Cache ME: DMA List building-PlILT=%p, DMAList=%p, NumDesc=0x%x, ListLength=0x%x\n",
                    pMirrorPlaceholder, DMAList, dmaDescriptorCount, dmaListLength);
#endif

            i = 0;                      /* Set up the Array Index           */

            /* Set up a DMA Descriptor for the Buffer if requested */
            if (pWorkingILT->wc_mirror.wc_mirror_tags.plMirrorBuffer)
            {
                DMAList[i].sysAddr = (UINT32)(pTag->bufPtr); /* Buffer Address*/
                /* Buffer Length in bytes */
                DMAList[i].xferSize = (pTag->vLen)*BYTES_PER_SECTOR;
                DMAList[i].wrtNotRd = true; /* DMA to NV Card               */
                /* Offset of buffer in DRAM + Start of NV Address */
                DMAList[i].nvAddr = (UINT32)(pTag->bufPtr) - (UINT32)WcbAddr +
                                                    gWC_NV_Mirror.wcbFEHandle;
                DMAList[i].pNextDD = &DMAList[i+1]; /* Next DMA Descriptor  */
#ifdef DEBUG_MIRROREXEC
                fprintf(stderr, "WC_Cache ME: DMA FE Buffer building-DMAListAddr=%p, SysAddr=0x%x, nvAddr=0x%x, Size=0x%x\n",
                        &DMAList[i], DMAList[i].sysAddr, DMAList[i].nvAddr, DMAList[i].xferSize);
#endif
                i++;                    /* Point to the next DMA Descriptor */

                if (!(BIT_TEST(C_ca.status,CA_NWAYMIRROR)))
                {
                    /* N=1 environment - DMA to the BE Environment */
                    DMAList[i].sysAddr = (UINT32)(pTag->bufPtr); /*Buffer Addr*/
                    /* Buffer Length in bytes           */
                    DMAList[i].xferSize = (pTag->vLen)*BYTES_PER_SECTOR;
                    DMAList[i].wrtNotRd = true; /* DMA to NV Card           */
                    /* Offset of buffer in DRAM + Start of NV Address */
                    DMAList[i].nvAddr = (UINT32)(pTag->bufPtr) -
                                (UINT32)WcbAddr + gWC_NV_Mirror.wcbBEHandle;
                    DMAList[i].pNextDD = &DMAList[i+1]; /* Next DMA Descriptor*/
#ifdef DEBUG_MIRROREXEC
                    fprintf(stderr, "WC_Cache ME: DMA BE Buffer building-DMAListAddr=%p, SysAddr=0x%x, nvAddr=0x%x, Size=0x%x\n",
                            &DMAList[i], DMAList[i].sysAddr, DMAList[i].nvAddr, DMAList[i].xferSize);
#endif
                    i++;                /* Point to the next DMA Descriptor */
                }
            }

            /* If there are any more DMA Descriptors, then they have to be Tags */
            for (; i < dmaDescriptorCount; i++, pTag = pTag->nextDirty)
            {
                DMAList[i].sysAddr = (UINT32)pTag; /* DMA the Tag           */
                DMAList[i].xferSize = sizeof(TG); /* Tag size               */
                DMAList[i].wrtNotRd = true; /* DMA to the NV Card           */
                /* Offset of buffer in DRAM + Start of NV Address */
                DMAList[i].nvAddr = (UINT32)pTag - (UINT32)WctAddr +
                                                    gWC_NV_Mirror.wctFEHandle;
                DMAList[i].pNextDD = &DMAList[i+1]; /* Next DMA Descriptor  */
#ifdef DEBUG_MIRROREXEC
                fprintf(stderr, "WC_Cache ME: DMA FE Tag building-DMAListAddr=%p, SysAddr=0x%x, nvAddr=0x%x, Size=0x%x\n",
                        &DMAList[i], DMAList[i].sysAddr, DMAList[i].nvAddr, DMAList[i].xferSize);
#endif

                if (!(BIT_TEST(C_ca.status,CA_NWAYMIRROR)))
                {
                    /* N=1 environment - DMA to the BE Environment */
                    i++;                /* Point to the next DMA Descriptor */
                    DMAList[i].sysAddr = (UINT32)pTag; /* DMA the Tag       */
                    DMAList[i].xferSize = sizeof(TG); /* Tag size           */
                    DMAList[i].wrtNotRd = true; /* DMA to NV Card           */
                    /* Offset of buffer in DRAM + Start of NV Address */
                    DMAList[i].nvAddr = (UINT32)pTag - (UINT32)WctAddr +
                                                    gWC_NV_Mirror.wctBEHandle;
                    DMAList[i].pNextDD = &DMAList[i+1]; /* Next DMA Descriptor*/
#ifdef DEBUG_MIRROREXEC
                    fprintf(stderr, "WC_Cache ME: DMA BE Tag building-DMAListAddr=%p, SysAddr=0x%x, nvAddr=0x%x, Size=0x%x\n",
                            &DMAList[i], DMAList[i].sysAddr, DMAList[i].nvAddr, DMAList[i].xferSize);
#endif
                }
            }

            /*
             * Fix up the last DMA Descriptor Next DD List pointer (preset
             * above - now needs to be terminated)
             */
            DMAList[i-1].pNextDD = NULL;

            /*
             * Now queue the DMA request to the NV Card.
             *
             * NOTE: Call to NV_DMARequest could result in a context switch.
             */
            rc = NV_DMARequest(DMAList, WC_DMAComplete, pMirrorPlaceholder);
            if (rc != EC_OK)
            {
                /*
                 * DMA Request failed - Free the DMA list and clear out the
                 * DMA List pointer in the ILT.  Decide later what else needs
                 * to be done
                 */
                s_Free(DMAList, dmaListLength, __FILE__, __LINE__); /* Free the DMA Descriptor List */
                pMirrorPlaceholder->cache_2_dlm.pcdDMAList = NULL; /* Clear the DMA List */
//                fprintf(stderr, "WC_Cache ME: DMA Request Failed - Return " "Code = 0x%x\n", rc);
            }

            if (!(BIT_TEST(C_ca.status,CA_NWAYMIRROR)))
            {
                /* N=1 Environment */
                if (rc == EC_OK)
                {
                    /*
                     * DMA request sent OK and not in an N>1 environment - all
                     * done until the DMA completes.
                     *
                     * Clear the Number of DRPs since there will not be any
                     * and if the DMA is complete finish the original request
                     */
#ifdef DEBUG_MIRROREXEC
                    fprintf(stderr, "WC_Cache ME: 1-Way No more work\n");
#endif
                    pMirrorPlaceholder->cache_2_dlm.cdNumDRPs = 0;
                    if (pMirrorPlaceholder->cache_2_dlm.pcdDMAList == NULL)
                    {
                        fprintf(stderr, "WC_Cache ME: 1-Way DMA Request Completed before exiting\n");
                        for (pTag = pWorkingILT->wc_mirror.pplTag; /* Start at the head */
                             pTag != NULL;       /* Until the end of the list */
                             pTag = pTag->nextDirty) /* Point to next tag   */
                        {
                            BIT_CLEAR(pTag->attrib,TG_MIRRIP); /* Remove Mirror
                                                            In Progress state */
                        }
                        put_wc_plholder(pMirrorPlaceholder); /*Free Plhlder*/
                        KernelDispatch(EC_OK, pWorkingILT, NULL, 0);/* Done OK*/
                    }
                }
                else
                {
                    /*
                     * There was an error returned from the DMA request,
                     * clear the Mirror In Progress state in all the tags,
                     * free the placeholder, and complete the
                     * original Mirror Request.
                     *
                     * Do not need to report the Error to the original
                     * Requester.  The NV Card routines will set the Card into
                     * a failed state.
                     */
                    fprintf(stderr, "WC_Cache ME: 1-Way DMA Request Failed\n");
                    for (pTag = pWorkingILT->wc_mirror.pplTag; /* Start at the head   */
                        pTag != NULL;       /* Until the end of the list    */
                        pTag = pTag->nextDirty) /* Point to next tag in list */
                    {
                        /* Remove Mirror In Progress state */
                        BIT_CLEAR(pTag->attrib,TG_MIRRIP);
                    }
                    put_wc_plholder(pMirrorPlaceholder); /*Free Placeholder*/
                    KernelDispatch(EC_OK, pWorkingILT, NULL, 0); /* Done - OK */
                }
            }
            else if ((BIT_TEST(C_ca.status,CA_NWAYMIRROR)) &&
                     (BIT_TEST(C_ca.status,CA_MIRRORBROKEN)))
            {
                /* N-Way but the Mirror is Broken */
                if (rc == EC_OK)
                {
                    /*
                     * DMA request sent OK - do not need to send the info to
                     * the Mirror Partner.  Just need to wait for the DMA to
                     * the NV Card to complete.
                     *
                     * Do not need to report the Error to the original
                     * Requester.  The NV Card routines will set the Card into
                     * a failed state.
                     *
                     * Clear the Number of DRPs since there will not be any
                     * and if the DMA is complete finish the original request
                     */
#ifdef DEBUG_MIRROREXEC
                    fprintf(stderr, "WC_Cache ME: N-Way Mirror Broken\n");
#endif
                    pMirrorPlaceholder->cache_2_dlm.cdNumDRPs = 0;
                    if (pMirrorPlaceholder->cache_2_dlm.pcdDMAList == NULL)
                    {
                        fprintf(stderr, "WC_Cache ME: N-Way Broken DMA Request Completed before exiting\n");
                        for (pTag = pWorkingILT->wc_mirror.pplTag; /* Start at the head */
                            pTag != NULL;       /* Until the end of the list  */
                            pTag = pTag->nextDirty) /* Point to next tag    */
                        {
                            /* Remove Mirror In Progress state */
                            BIT_CLEAR(pTag->attrib,TG_MIRRIP);
                        }
                        put_wc_plholder(pMirrorPlaceholder); /*Free Plhlder*/
                        KernelDispatch(EC_OK, pWorkingILT, NULL, 0);/* Done OK*/
                    }
                }
                else
                {
                    /*
                     * There was an error returned from the DMA request,
                     * clear the Mirror In Progress state in all the tags,
                     * free the placeholder, and complete the
                     * original Mirror Request
                     *
                     * Do not need to report the Error to the original
                     * Requester.  The NV Card routines will set the Card into
                     * a failed state.
                     */
                    fprintf(stderr, "WC_Cache ME: N-Way Mirror Broken DMA Request Failed\n");
                    for (pTag = pWorkingILT->wc_mirror.pplTag; /* Start at the head   */
                        pTag != NULL;       /* Until the end of the list    */
                        pTag = pTag->nextDirty) /* Point to next tag in list */
                    {
                        /* Remove Mirror In Progress state */
                        BIT_CLEAR(pTag->attrib,TG_MIRRIP);
                    }
                    put_wc_plholder(pMirrorPlaceholder); /*Free Placeholder*/
                    KernelDispatch(EC_OK, pWorkingILT, NULL, 0); /* Done - OK */
                }
            }
            else
            {
                /*
                 * DMA request sent (Good or Bad) and in an N-Way
                 * environment and OK to Mirror the information to the
                 * Mirror Partner
                 *
                 * NOTE: N>1 and Error state is handled above and does not
                 * need to be checked down this path!
                 */

                /*
                 * Determine how many DRP requests are needed (each DRP may
                 * mirror information for only 1 or 2 descriptors at a time).
                 * Keeping track of how many outstanding are needed up front
                 * in case of a context switch in the middle of getting an ILT
                 * or something else prevents early completion when not all are
                 * done.
                 */
                pMirrorPlaceholder->cache_2_dlm.cdNumDRPs =
                    ((dmaDescriptorCount / DR_MAX_SGL_WRITE) +
                     (dmaDescriptorCount % DR_MAX_SGL_WRITE));
#ifdef DEBUG_MIRROREXEC
                fprintf(stderr, "WC_Cache ME: N-Way DRP Prep: Number of DRPs needed = 0x%x\n",
                                pMirrorPlaceholder->cache_2_dlm.cdNumDRPs);
#endif

                /*
                 * Set up the Completion handler for DRPs completing to
                 * decrement the outstanding count
                 */
                pMirrorPlaceholder->cr = WC_DRPChainComplete;

                /*
                 * Create an ILT and DRP for each needed to mirror all the
                 * information to the Mirror Partner
                 */
                pTag = pWorkingILT->wc_mirror.pplTag; /* Get the beginning tag        */
                workingDescriptorCount = dmaDescriptorCount;
                while (workingDescriptorCount != 0)
                {
                    pDRPILT = get_ilt();  /* Allocate an ILT for DRP    */
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)pDRPILT);
#endif /* M4_DEBUG_ILT */

                    pDRPILT->fthd = NULL; /* Clear the forward pointer      */
                    pDRPILT->bthd = NULL; /* Clear the backward pointer     */
                    pDRPILT->cache_2_dlm.pcdPhILT = pMirrorPlaceholder; /* Save PH in ILT */
                    pDRPILT->cache_2_dlm.pcdTag = pTag; /* Save the beginning Tag in list */

                    /*
                     * Allocate memory the size needed to allocate a DRP,
                     * Datagram Request, and Datagram Response
                     */
                    drpDescriptorCount = (workingDescriptorCount >= DR_MAX_SGL_WRITE) ?
                                    DR_MAX_SGL_WRITE : workingDescriptorCount;
                    pDRPILT->cache_2_dlm.cdNumTags = drpDescriptorCount; /* Num Tags - DRP*/
#ifdef DEBUG_MIRROREXEC
                    fprintf(stderr, "WC_Cache ME: N-Way DRP Prep: ILT = %p; DRP Descriptor Count = 0x%x\n",
                            pDRPILT, drpDescriptorCount);
#endif

                    /* Get size of memory needed  */
                    drpSize = sizeof(DLM_REQ) + sizeof(SGL) +
                             (sizeof(SGL_DESC) * drpDescriptorCount) + sizeof(DATAGRAM_REQ) +
                             (sizeof(CAC0) * drpDescriptorCount) + sizeof(DATAGRAM_RSP);
                    pDRPILT->cache_2_dlm.cdDgReqSize = drpSize; /* Save the size to free */

                    pDRP = s_MallocC(drpSize, __FILE__, __LINE__); /* Allocate space needed & Clear*/
                    pDGReq = (void *)(pDRP + 1);
                    pCAC0Entry = (void *)(pDGReq + 1);
                    pDGRsp = (void *)(pCAC0Entry + drpDescriptorCount);
                    pSGL = (void *)(pDGRsp + 1);
                    pSGLDesc = (void *)(pSGL + 1);
                    pDRPILT->cache_2_dlm.pcdDRP = pDRP; /* Save the DRP in the ILT      */
#ifdef DEBUG_MIRROREXEC
                    fprintf(stderr, "WC_Cache ME: N-Way DRP Prep: DRP Size=0x%x, DRP=%p, DGReq=%p, CAC0=%p, DGRsp=%p, SGL=%p, SGLDesc=%p\n",
                            drpSize, pDRP, pDGReq, pCAC0Entry, pDGRsp, pSGL, pSGLDesc);
#endif
                    /* Create the DRP to send */
                    pDRP->func = DR_CACHE_TO_DLM;
                    pDRP->status = EC_IO_ERR; /* Preset to failure          */
                    pDRP->sglPtr = pSGL;
                    pDRP->timeout = CACHE_TIME_OUT; /* Timeout in Seconds   */
                    pDRP->issueCnt = CACHE_ISSUE_CNT; /* Retry Count        */
                    pDRP->reqAddress = pDGReq;
                    pDRP->reqLength = sizeof(DATAGRAM_REQ) + ((sizeof(CAC0) * drpDescriptorCount));
                    pDRP->rspAddress = pDGRsp;
                    pDRP->rspLength = sizeof(DATAGRAM_RSP);

                    /* Create the Datagram Request */
                    pDGReq->srvCPU = DG_CPU_INTERFACE;
                    pDGReq->hdrLen = sizeof(DATAGRAM_REQ);
                    pDGReq->seq = ++wc_mem_seqnum;
                    pDGReq->fc = CAC0_FC_WRTMEM;
                    pDGReq->path = DG_PATH_ANY;
                    pDGReq->dstSN = bswap_32(K_ficb->mirrorPartner);
                    pDGReq->srvName = CAC0_NAME;
                    pDGReq->reqLen = (sizeof(CAC0) * drpDescriptorCount);

                    /* Setup the Write Memory Addresses and lengths */
                    pSGL->scnt = drpDescriptorCount; /* Number of SGLs in list*/
                    pSGL->size = sizeof(SGL) + (drpDescriptorCount * sizeof(SGL_DESC));

                    i = 0;              /* Reset the array pointer          */
                    if (pWorkingILT->wc_mirror.wc_mirror_tags.plMirrorBuffer)
                    {
                        /*
                         * Buffer mirror requested - assumptions are:
                         *   DR_MAX_SGL_WRITE > 1
                         *   Only 0 or 1 tags are associated with this
                         */
                        pCAC0Entry[i].rqMemAddr = pTag->bufPtr;
                        pCAC0Entry[i].rqMemLen = (pTag->vLen)*BYTES_PER_SECTOR;

                        pSGLDesc[i].addr = pTag->bufPtr;
                        pSGLDesc[i].len = (pTag->vLen)*BYTES_PER_SECTOR;
                        pSGLDesc[i].direction = SG_DIR_OUT;

                        BIT_SET(pDGReq->gpr0,CAC0_DATA); /* Show in Data Area */
#ifdef DEBUG_MIRROREXEC
                        fprintf(stderr, "WC_Cache ME: N-Way DRP Prep: CAC0=%p, CAC0 Buffer=%p, CAC0 Buffer Len=0x%x\n",
                                &pCAC0Entry[i], pCAC0Entry[i].rqMemAddr, pCAC0Entry[i].rqMemLen);
                        fprintf(stderr, "WC_Cache ME: N-Way DRP Prep: SGLDesc=%p, SGL Buffer=%p, SGL Buffer Len=0x%x\n",
                                &pSGLDesc[i], pSGLDesc[i].addr, pSGLDesc[i].len);
#endif
                        i++;
                    }

                    for (; i < drpDescriptorCount; i++, pTag = pTag->nextDirty)
                    {
                        /* Only Tags should be following down this leg */
                        pCAC0Entry[i].rqMemAddr = pTag;
                        pCAC0Entry[i].rqMemLen = sizeof(TG);

                        pSGLDesc[i].addr = pTag;
                        pSGLDesc[i].len = sizeof(TG);
                        pSGLDesc[i].direction = SG_DIR_OUT;

                        BIT_SET(pDGReq->gpr0,CAC0_TAG); /* Show in Tag Area   */
#ifdef DEBUG_MIRROREXEC
                        fprintf(stderr, "WC_Cache ME: N-Way DRP Prep: CAC0=%p, CAC0 Tag=%p, CAC0 Tag Len=0x%x\n",
                                &pCAC0Entry[i], pCAC0Entry[i].rqMemAddr, pCAC0Entry[i].rqMemLen);
                        fprintf(stderr, "WC_Cache ME: N-Way DRP Prep: SGLDesc=%p, SGL Tag=%p, SGL Tag Len=0x%x\n",
                                &pSGLDesc[i], pSGLDesc[i].addr, pSGLDesc[i].len);
#endif
                    }

                    /*
                     * Set up the Response Status to an invalid value so that
                     * if for some reason it does not get sent, the status
                     * will still be bad
                     */
                    pDGRsp->status = EC_IO_ERR;

                    /* Bump the ILT a layer and queue it to DLM to send */
                    EnqueueILT((void*)DLM$quedrp, pDRPILT,
                    (void*)C_label_referenced_in_i960asm(WC_MirrorDRPComplete));

                    /* Remove the number of descriptors handled so far */
                    workingDescriptorCount -= drpDescriptorCount;
                }
            }

            /*
             * If there is more data to be mirrored, do a task swap to let
             * the current ops (DMA and mirroring to a Mirror Partner)
             * through before pulling more off the queue
             */
            if (gMirrorQueue.head != NULL)
            {
#ifdef DEBUG_MIRROREXEC
                fprintf(stderr, "WC_Cache ME: More work to do in queue, do context switch\n");
#endif
                TaskSwitch();
            }
        }
    }
}


/**
******************************************************************************
**
**  @brief      WC_MirrorBETagExec - Mirrors BE Tags to the BE MMC
**
**              Mirrors the BE Tag back to the BE MMC area when a Flush BE tag
**              completes.
**
**              This executive processes a queue of supplied ILT/placeholders
**              that each refer to a particular cache tag.  When the requested
**              action has been completed the completion routine in the
**              ILT/placeholder is called.
**
**  @param      none
**
**  @return     none
**
**  @attention  This is a task that never returns.  To enqueue an
**              ILT/placeholder to this task see the <wc$q> function.
**              The ILT/placeholder must have:
**                  <plctag>    - Cache tag for operation.
**                  <il_cr>     - Completion routine to call when done with
**                      first parm  - Completion Status
**                      second parm - ILT/Placeholder
**
******************************************************************************
**/
NORETURN
void WC_MirrorBETagExec(void)
{
    UINT32 dmaDescriptorCount = 0;      /* Number of DMA descriptors        */
    struct ILT  *pWorkingILT = NULL;    /* ILT being processed              */
    struct TG   *pTag = NULL;           /* Tag being processed              */
    struct ILT  *pMirrorPlaceholder = NULL; /* Mirror Placeholder ILT       */
    struct NV_DMA_DD *DMAList = NULL;   /* DMA List (used as an array)      */
    UINT32 rc = EC_OK;                  /* Return Code                      */
    UINT32 dmaListLength = 0;           /* DMA List Length                  */
    UINT32 i = 0;                       /* Array Index                      */

    /* Setup queue control block */
    gWCMirrorBETagQueue.pcb = K_xpcb;   /* Set up my PCB                    */
    gWCMirrorBETagQueue.head = NULL;    /* Set up the Head ILT              */
    gWCMirrorBETagQueue.tail = NULL;    /* Set up the Tail ILT              */
    gWCMirrorBETagQueue.qcnt = 0;       /* Set up the Count of ILTs on queue*/

    /* Task runs forever */
    while (TRUE)                        /* Always keep task running         */
    {
        /* Set process to wait for signal and Exchange process */
        TaskSetMyState(PCB_NOT_READY);
        TaskSwitch();

        /* Loop on items queued */
        while (gWCMirrorBETagQueue.head != NULL)
        {
            dmaDescriptorCount = 0;     /* Clear the number of DDs needed   */

            /* Get the head ILT mirror request */
            pWorkingILT = QU_DequeReqILT(&gWCMirrorBETagQueue);
            pTag = pWorkingILT->wc_mirror.pplTag; /* Get the tag this mirror is based on*/
            dmaDescriptorCount++;   /* Increment the number of DDs needed   */

#ifdef DEBUG_MIRRORBETAGEXEC
            fprintf(stderr, "WC Mirror BE Tag: Main Tag-Mirror In Progress Tag=%p\n", pTag);
#endif

            /*
             * Set up the DD List to DMA to the NV BE Areas.
             *
             * Determine how big of a DMA Descriptor (DD) list needs to be
             * allocated to do the transfer to the NV areas.
             */
            while ((pTag = pTag->nextDirty) != NULL)
            {
                dmaDescriptorCount++;   /* Incr the number of DDs needed    */

#ifdef DEBUG_MIRRORBETAGEXEC
                fprintf(stderr, "WC Mirror BE Tag: Chained Tag-Mirror In Progress Tag=%p\n", pTag);
#endif

            }
            pTag = pWorkingILT->wc_mirror.pplTag; /* Get the head tag again           */

            /*
             * Allocate a Placeholder ILT to save this information in,
             * allocate the DD Buffer area, and then set up the DD Lists
             */
            dmaListLength = dmaDescriptorCount * sizeof(NV_DMA_DD);
            pMirrorPlaceholder = get_wc_plholder();
            DMAList = s_MallocC(dmaListLength, __FILE__, __LINE__);
            pMirrorPlaceholder->misc = dmaListLength;
            pMirrorPlaceholder->cache_2_dlm.pcdDMAList = DMAList;
            pMirrorPlaceholder->cache_2_dlm.pcdPhILT = pWorkingILT;
            pMirrorPlaceholder->cache_2_dlm.cdNumDRPs = 0;
            pMirrorPlaceholder->cache_2_dlm.cdDRPec = EC_OK; /* Show all OK             */

#ifdef DEBUG_MIRRORBETAGEXEC
            fprintf(stderr, "WC Mirror BE Tag: DMA List building-PlILT=%p, DMAList=%p, NumDesc=0x%x, ListLength=0x%x\n",
                    pMirrorPlaceholder, DMAList, dmaDescriptorCount, dmaListLength);
#endif

            /* Set up the DMA Descriptors for each of the tags */
            for (i = 0; i < dmaDescriptorCount; i++, pTag = pTag->nextDirty)
            {
                DMAList[i].sysAddr = (UINT32)pTag; /* DMA the Tag           */
                DMAList[i].xferSize = sizeof(struct TG); /* Tag size        */
                DMAList[i].wrtNotRd = true; /* DMA to the NV Card           */
                /*
                 * Convert BE address to FE address and then the offset of
                 * buffer in DRAM + Start of NV Address
                 */
                DMAList[i].nvAddr = (UINT32)pTag - BE_ADDR_OFFSET - (UINT32)WctAddr +
                                   gWC_NV_Mirror.wctBEHandle;
                DMAList[i].pNextDD = &DMAList[i+1]; /* Next DMA Descriptor  */

#ifdef DEBUG_MIRRORBETAGEXEC
                fprintf(stderr, "WC Mirror BE Tag: DMA BE Tag building-DMAListAddr=%p, SysAddr=0x%x, nvAddr=0x%x, Size=0x%x\n",
                        &DMAList[i], DMAList[i].sysAddr, DMAList[i].nvAddr, DMAList[i].xferSize);
#endif
            }

            /*
             * Fix up the last DMA Descriptor Next DD List pointer (preset
             * above - now needs to be terminated)
             */
            DMAList[i-1].pNextDD = NULL;

            /*
             * Now queue the DMA request to the NV Card.
             *
             * NOTE: Call to NV_DMARequest could result in a context switch.
             */
            rc = NV_DMARequest(DMAList, WC_DMAComplete, pMirrorPlaceholder);
            if (rc != EC_OK)
            {
                fprintf(stderr, "WC Mirror BE Tag: DMA Request Failed-Return Code=0x%x\n", rc);
                /*
                 * DMA Request failed - Free the DMA list and clear out the
                 * DMA List pointer in the ILT, free the placeholder and
                 * complete the original Mirror Request.
                 *
                 * Do not need to report the Error to the original Requester.
                 * The NV Card routines will set the Card into a failed state.
                 */
                s_Free(DMAList, dmaListLength, __FILE__, __LINE__); /* Free the DMA Descriptor List */
                pMirrorPlaceholder->cache_2_dlm.pcdDMAList = NULL; /* Clear the DMA List */
                put_wc_plholder(pMirrorPlaceholder); /*Free Placeholder*/
                KernelDispatch(EC_OK, pWorkingILT, NULL, 0); /* Done - OK */
            }

            /*
             * If there is more data to be mirrored, do a task swap to let
             * the current ops (DMA and mirroring to a Mirror Partner)
             * through before pulling more off the queue
             */
            if (gWCMirrorBETagQueue.head != NULL)
            {
#ifdef DEBUG_MIRRORBETAGEXEC
                fprintf(stderr, "WC Mirror BE Tag: More work to do in queue, do context switch\n");
#endif
                TaskSwitch();
            }
        }
    }
}


/**
******************************************************************************
**
**  @brief      WC_DMAComplete - DMA to NV Card Completion routine
**
**              This function handles the completion of the DMA to the NV Card.
**              It frees the DMA Descriptor List and if there are no
**              outstanding DRP Requests to mirror to a MP, then it resets
**              the Tag(s) Mirror In Progress State and completes
**              the original request back to the DMA requester.
**
**  @param      DMA Response Packet Address
**
**  @return     none
**
**  @attention  The DMA Response Packet must have the ILT passed into the
**              DMA Request function.  This function must also free the DMA
**              Response Packet that it receives.
**
******************************************************************************
**/
static void WC_DMAComplete(struct NV_DMA_RSP_PKT* pRsp)
{
#ifdef DEBUG_DMACOMP
    UINT32  dmaStatus = EC_IO_ERR;      /* DMA Completion Status            */
#endif /* DEBUG_DMACOMP */
    UINT32  drpStatus = EC_IO_ERR;      /* DRP Composite Completion Status  */
    struct ILT  *pOrigILT = NULL;       /* Original ILT Mirror Request      */
    struct ILT  *pMirrorILT = NULL;     /* Placeholder of Mirror Request    */
    struct TG   *pTag;                  /* Tag being worked on              */

    /*
     * Get the Response Status and the ILT from the Response Packet.  If the
     * DMA Status is non-zero, report an IO Error
     */
#ifdef DEBUG_DMACOMP
    dmaStatus = ((pRsp->dmaStsCtrl.errSts != 0) ? EC_IO_ERR : EC_OK);
#endif /* DEBUG_DMACOMP */
    pMirrorILT = pRsp->pILT;
#ifdef DEBUG_DMACOMP
    fprintf(stderr, "WC_Cache DC: ILT=%p, DMA HW Sts=0x%x, DMA Status=0x%x, DMAList=%p, DMA Size=0x%x, NumDRPs=0x%x\n",
                    pMirrorILT, pRsp->dmaStsCtrl.stsCtrl, dmaStatus,
                    pMirrorILT->cache_2_dlm.pcdDMAList, pMirrorILT->misc,
                    pMirrorILT->cache_2_dlm.cdNumDRPs);
#endif

    /* Free the original DMA List and the DMA Response Packet */
    s_Free(pMirrorILT->cache_2_dlm.pcdDMAList, pMirrorILT->misc, __FILE__, __LINE__);
    s_Free(pRsp, sizeof(NV_DMA_RSP_PKT), __FILE__, __LINE__); /* Free the DMA Response Packet     */

    /*
     * If there are no outstanding DRPs (because N=1 or all the DRPs have
     * completed already), then clear the Mirror In Progress state in all
     * the tags, free the placeholder, and complete the
     * original Mirror Request.
     *
     * Do not need to report the Error to the original Requester.  The NV
     * Card routines will set the Card into a failed state.
     */
    if (pMirrorILT->cache_2_dlm.cdNumDRPs == 0)
    {
        drpStatus = pMirrorILT->cache_2_dlm.cdDRPec; /* Get the DRP Composite Status    */
        pOrigILT = pMirrorILT->cache_2_dlm.pcdPhILT; /* Get the original Request        */
        for (pTag = pOrigILT->wc_mirror.pplTag;   /* Start at the head                */
            pTag != NULL;               /* Until the end of the list        */
            pTag = pTag->nextDirty)     /* Point to next tag in list        */
        {
            /* Remove Mirror In Progress state */
            BIT_CLEAR(pTag->attrib,TG_MIRRIP);
#ifdef DEBUG_DMACOMP
            fprintf(stderr, "WC_Cache DC: Tag Clear Mirror In Progress-Tag=%p\n", pTag);
#endif
        }
        put_wc_plholder(pMirrorILT); /* Free Placeholder                 */
#ifdef DEBUG_DMACOMP
        fprintf(stderr, "WC_Cache DC: Complete Original Request-ILT=%p, DRP Status=0x%x\n",
                        pOrigILT, drpStatus);
#endif
        KernelDispatch(drpStatus, pOrigILT, NULL, 0); /* Complete request    */
    }
    else
    {
        /*
         * Remove the DMA List pointer so the DRP Completion routines know that
         * the DMA is complete.  Also save the Status in the overall status,
         * if not successful.
         *
         * Do not need to report the Error to the original Requester.  The
         * NV Card routines will set the Card into a failed state.
         */
        pMirrorILT->cache_2_dlm.pcdDMAList = NULL;
#ifdef DEBUG_DMACOMP
        fprintf(stderr, "WC_Cache DC: DRPs are outstanding\n");
#endif
    }
}


/**
******************************************************************************
**
**  @brief      WC_MirrorDRPComplete - Complete a Mirror DRP Request
**
**              This function handles the completion of a Mirror DRP request
**              to the Mirror Partner.  If an error occurred, a Log message
**              will be sent about the failure.  The DRP resources are freed
**              and completes the original requestor.
**
**  @param      rc - ILT Return Status from the DRP Mirror Request
**  @param      ILT - Requesters ILT
**
**  @return     none
**
******************************************************************************
**/
void WC_MirrorDRPComplete(UINT32 rc, struct ILT* pILT)
{
    struct DLM_REQ *pDRP;               /* DRP                              */
    struct DATAGRAM_REQ *pDGReq;        /* Datagram Request                 */
    struct DATAGRAM_RSP *pDGRsp;        /* Datagram Response                */
    struct ILT  *pMirrorILT;            /* Mirroring ILT has count of DRPs  */
    void (*compRtn)(UINT32, struct ILT *); /* Completion Routine from ILT   */

    /* Get the DRP Address, Datagram Request, and Datagram Response messages */
    pDRP = pILT->cache_2_dlm.pcdDRP;
    pDGReq = pDRP->reqAddress;
    pDGRsp = pDRP->rspAddress;
#ifdef DEBUG_MDRPCOMP
    fprintf(stderr, "WC_Cache DRC: ILT=%p, DRP=%p, DGRsp=%p, RC=0x%x, DRPStatus=0x%x, DGStatus=0x%x\n",
                    pILT, pDRP, pDGRsp, rc, pDRP->status, pDGRsp->status);
#endif

    /*
     * If the Mirror did not complete successfully, log an error and set the
     * Cache Status to Error state.
     */
    if ((rc != EC_OK) ||
        (pDRP->status != EC_OK) ||
        (pDGRsp->status != DG_ST_OK))
    {
#ifdef DEBUG_MDRPCOMP
        fprintf(stderr, "WC_Cache DRC: DRPs Completed with error\n");
#endif
        /*
         * Log an error about the problem, save the Controller SN that had the
         * problem, and set the Cache into an Error State
         */
        gWCErrorMP = bswap_32(pDGReq->dstSN);
        CA_LogMirrorFailure(rc, pILT);
        BIT_SET(C_ca.status,CA_ERROR);

        /*
         * If the ILT Status is good, set it to failure so that the problem
         * will always be seen
         */
        if (rc == EC_OK)
        {
            rc = EC_IO_ERR;             /* Replace good status with Error   */
#ifdef DEBUG_MDRPCOMP
            fprintf(stderr, "WC_Cache DRC: DRPs Error - Change ILT to Error\n");
#endif
        }
    }
    else
    {
        /*
         * The DRP completed successfully, show that a remote mirror operation
         * worked!
         */
        C_mirror_error_flag = FALSE;
#ifdef DEBUG_MDRPCOMP
        fprintf(stderr, "WC_Cache DRC: DRP Completed successfully\n");
#endif
    }

    /*
     * NOTE: The Reset of the Tags Mirror In Progress state will be handled in
     * the base ILT processing that handles completion of all DRPs
     * outstanding for this mirror request (timing with when the DMA to the
     * NV Card will complete issues).
     *
     * Free the DRP, Datagram Request, and Datagram Response area.  Then free
     * the DRP ILT and call the completion handler of the Mirror Request.
     */
    s_Free(pDRP, pILT->cache_2_dlm.cdDgReqSize, __FILE__, __LINE__);      /* Free the DRP, DG, etc            */
    pMirrorILT = pILT->cache_2_dlm.pcdPhILT;        /* Get Mirror DRPs cumulative ILT   */
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)pILT);
#endif /* M4_DEBUG_ILT */
    put_ilt(pILT);                      /* Release the incoming ILT         */

#if WC_MIRROR_ERROR_DISABLE
    rc = EC_OK;                         /* Do not show errors to next layers */
#endif

    compRtn = pMirrorILT->cr;           /* Get the Completion Routine       */
#ifdef DEBUG_MDRPCOMP
    fprintf(stderr, "WC_Cache DRC: Call Completion Routine=%p, ILT=%p, rc=0x%x\n",
                    compRtn, pMirrorILT, rc);
#endif
    (*compRtn)(rc, pMirrorILT);         /* Call the Completion Routine      */
}


/**
******************************************************************************
**
**  @brief      WC_DRPChainComplete - Complete a DRP and all DRP requests
**
**              This routine makes a composite of all the error codes received
**              during mirroring of the entire tag chain to the remote Mirror
**              Partner.  When all the DRPs are complete and the DMA to the
**              NV Card is complete, the tags will have the "Mirror In Progress"
**              state cleared, the counting placeholder ILT is freed, and
**              the original caller's completion routine is called with
**              the composite error code.  If the DMA to the NV Card is not
**              complete, set the DRP count to zero so the DMA to NV Card
**              completion can do the above.
**
**  @param      rc - Return Code of the DRP
**  @param      pILT - ILT Pointer for this counting placeholder
**
**  @return     none
**
******************************************************************************
**/
static void WC_DRPChainComplete(UINT32 rc, struct ILT* pILT)
{
    UINT32  drpStatus = EC_IO_ERR;      /* DRP Composite Completion Status  */
    UINT32  drpCount = 0;               /* DRP Counter                      */
    struct ILT *pOrigILT = NULL;        /* Original Mirror Request ILT      */
    struct TG *pTag = NULL;             /* Tag pointer                      */

    /*
     * Get the DRP Composite Completion Status and add this DRP completion
     * status to it.  Then save it back to the ILT for later use.
     */
    pILT->cache_2_dlm.cdDRPec |= rc;
    drpStatus = pILT->cache_2_dlm.cdDRPec;
#ifdef DEBUG_DRPCHAINCOMP
    fprintf(stderr, "WC_Cache DRCC: ILT=%p, RC=0x%x, DRP Composite Status=0x%x, Num DRPs left=0x%x\n",
                    pILT, rc, drpStatus, pILT->cache_2_dlm.cdNumDRPs - 1);
#endif

    /* Decrement the outstanding DRP count and save it back to the ILT */
    pILT->cache_2_dlm.cdNumDRPs--;
    drpCount = pILT->cache_2_dlm.cdNumDRPs;

    /*
     * If all the DRPs are complete and the DMA to the NV Card is complete,
     * then clear the Mirror In Progress state in all the tags, free the
     * placeholder and complete the original Mirror Request.
     */
    if ((drpCount == 0) &&
        (pILT->cache_2_dlm.pcdDMAList == NULL))
    {
        pOrigILT = pILT->cache_2_dlm.pcdPhILT;      /* Get the original Request        */
        for (pTag = pOrigILT->wc_mirror.pplTag;   /* Start at the head                */
            pTag != NULL;               /* Until the end of the list        */
            pTag = pTag->nextDirty)     /* Point to next tag in list        */
        {
            /* Remove Mirror In Progress state */
            BIT_CLEAR(pTag->attrib,TG_MIRRIP);
#ifdef DEBUG_DRPCHAINCOMP
            fprintf(stderr, "WC_Cache DRCC: Tag Clear Mirror In Progress-Tag=%p\n", pTag);
#endif
        }
        put_wc_plholder(pILT);       /* Free Placeholder                 */
#ifdef DEBUG_DRPCHAINCOMP
        fprintf(stderr, "WC_Cache DRCC: Complete Original Op-ILT=%p, Status=0x%x\n",
                        pOrigILT, drpStatus);
#endif
        KernelDispatch(drpStatus, pOrigILT, NULL, 0); /* Complete request    */
    }
}


/**
******************************************************************************
**
**  @brief      WC_MirrorBE - Copies the data from a Datagram to BE NV Card area
**
**              This function copies the requested data specified in a Datagram
**              to the BE NV Card Area.
**
**  @param      pILT    - ILT that will need to be completed when the DMA
**                          completes to the NV Card
**  @param      pDRP    - DRP that contains the addresses of the Data to copy
**
**  @return     none
**
**  @attention  The addresses found in the Datagram are "FE" addresses that
**              need to be converted to "BE" addresses to DMA from.
**
**              Also, the Data buffers need to be mirrored before the tags
**              to ensure the proper state tracking.
**
**              Also, if the original requesters ILT is completed, it needs to
**              be backed up one level before completing.
**
******************************************************************************
**/
void WC_MirrorBE(struct ILT* pILT, struct DLM_REQ* pDRP)
{
    struct DATAGRAM_REQ* pDGReq = NULL; /* Get the Datagram Request         */
    struct CAC0* pCAC0Entry = NULL;     /* Mirror Request Entries           */
    UINT32  numCAC0Entries = 0;         /* Number of CAC0 Entries           */
    UINT32  i;                          /* Array index                      */
    struct ILT  *pMirrorPlaceholder = NULL; /* Mirror Placeholder ILT       */
    struct NV_DMA_DD *DMAList = NULL;   /* DMA List (used as an array)      */
    UINT32  dmaListLength = 0;          /* DMA List Length                  */
    UINT32  sourceAddress = 0;          /* Source Address of the DMA        */
    UINT32  rc = EC_IO_ERR;             /* Return Code                      */

    /* Get the number of entries to allocate the space and use as an array index */
    pDGReq = pDRP->reqAddress;
    numCAC0Entries = ((pDGReq->reqLen) / sizeof(CAC0));
    pCAC0Entry = (void *)(pDGReq + 1);
#ifdef DEBUG_MIRRORBE
    fprintf(stderr, "WC_Cache MB: ILT=%p, DRP=%p, DG=%p, Num Entries=0x%x, 1st Entry=%p\n",
                    pILT, pDRP, pDGReq, numCAC0Entries, pCAC0Entry);
#endif

    /*
     * Allocate a Placeholder ILT to save this information in,
     * allocate the DD Buffer area, and then set up the DD Lists
     */
    pMirrorPlaceholder = get_wc_plholder();
    dmaListLength = numCAC0Entries * sizeof(NV_DMA_DD);
    DMAList = s_MallocC(dmaListLength, __FILE__, __LINE__);
    pMirrorPlaceholder->misc = dmaListLength;
    pMirrorPlaceholder->cache_2_dlm.pcdDMAList = DMAList;
    pMirrorPlaceholder->cache_2_dlm.pcdPhILT = pILT;
#ifdef DEBUG_MIRRORBE
    fprintf(stderr, "WC_Cache MB: PlILT=%p, DMAList=%p, List Length=0x%x\n",
                    pMirrorPlaceholder, DMAList, dmaListLength);
#endif

    /* Set up the DMA Descriptors */
    for (i = 0; i < numCAC0Entries; i++)
    {
        DMAList[i].xferSize = pCAC0Entry[i].rqMemLen; /* Buffer length      */
        DMAList[i].wrtNotRd = true;     /* DMA to the NV Card               */
        DMAList[i].pNextDD = &DMAList[i+1]; /* Next DMA Descriptor pointer  */
        sourceAddress = (UINT32)(pCAC0Entry[i].rqMemAddr); /* Addr of the Data*/
        DMAList[i].sysAddr =
            sourceAddress + BE_ADDR_OFFSET; /* Convert to BE @ */
#ifdef DEBUG_MIRRORBE
        fprintf(stderr, "WC_Cache MB: DMA List Prep-SourceAddr=0x%x, sysAddr=0x%x, Length = 0x%x\n",
                        sourceAddress, DMAList[i].sysAddr, DMAList[i].xferSize);
#endif

        /*
         * Determine if the address is in the Tag or Data regions of memory and
         * set up the Region handle appropriately.
         */
        if ((sourceAddress >= (UINT32)WcbAddr) &&
            (sourceAddress < ((UINT32)WcbAddr + WcbSize)))
        {
            /* In the Buffer Region */
            DMAList[i].nvAddr = sourceAddress - (UINT32)WcbAddr +
                                                    gWC_NV_Mirror.wcbBEHandle;
#ifdef DEBUG_MIRRORBE
            fprintf(stderr, "WC_Cache MB: DMA List Prep-Buffer NV Addr=0x%x\n",
                            DMAList[i].nvAddr);
#endif
        }
        else if ((sourceAddress >= (UINT32)WctAddr) &&
                 (sourceAddress < ((UINT32)WctAddr + WctSize)))
        {
            /* In the Tag Region */
            DMAList[i].nvAddr = sourceAddress - (UINT32)WctAddr +
                                                    gWC_NV_Mirror.wctBEHandle;
#ifdef DEBUG_MIRRORBE
            fprintf(stderr, "WC_Cache MB: DMA List Prep-Tag NV Addr=0x%x\n",
                            DMAList[i].nvAddr);
#endif
        }
        else if ((sourceAddress >= (UINT32)WcctAddr) &&
                 (sourceAddress < ((UINT32)WcctAddr + WcctSize)))
        {
            /* In the Control Table Region */
            DMAList[i].nvAddr = sourceAddress - (UINT32)WcctAddr +
                                                    gWC_NV_Mirror.wcctBEHandle;
#ifdef DEBUG_MIRRORBE
            fprintf(stderr, "WC_Cache MB: DMA List Prep - Control Table NV Addr = 0x%x\n",
                    DMAList[i].nvAddr);
#endif
        }
        else if ((sourceAddress >= (UINT32)WccAddr) &&
                 (sourceAddress < ((UINT32)WccAddr + WccSize)))
        {
            /* In the Configuration Region */
            DMAList[i].nvAddr = sourceAddress - (UINT32)WccAddr +
                                                    gWC_NV_Mirror.wccBEHandle;
#ifdef DEBUG_MIRRORBE
            fprintf(stderr, "WC_Cache MB: DMA List Prep - Configuration NV Addr = 0x%x\n",
                    DMAList[i].nvAddr);
#endif
        }
        else
        {
            /*
             * In a non-supported region in the buffer card - post a software
             * fault and kill the processor.
             */
            fprintf(stderr, "WC_Cache MB: Unsupported region = 0x%x\n", sourceAddress);
            eSoftFault_WC_Cache.header.length = 12;
            eSoftFault_WC_Cache.data.errorCode = CAC_SFT8;
            eSoftFault_WC_Cache.data.data[0] = sourceAddress;
            eSoftFault_WC_Cache.data.data[1] = pCAC0Entry[i].rqMemLen;
            MSC_SoftFault(&eSoftFault_WC_Cache);
            abort();
        }
    }

    /*
     * Fix up the last DMA Descriptor Next DD List pointer (preset
     * above - now needs to be terminated)
     */
    DMAList[i-1].pNextDD = NULL;

    /*
     * Now queue the DMA request to the NV Card.  If there is an error
     * returned, free the DMA list, free the placeholder, and complete the
     * original Mirror Request with the error.
     *
     * NOTE: Call to NV_DMARequest could result in a context switch.
     *
     * Do not need to report the Error to the original Requester.  The NV
     * Card routines will set the Card into a failed state.
     */
    rc = NV_DMARequest(DMAList, WC_MirrorBEComplete, pMirrorPlaceholder);
    if (rc != EC_OK)
    {
        fprintf(stderr, "WC_Cache MB: DMA Request Failed - RC = 0x%x\n", rc);
        s_Free(DMAList, dmaListLength, __FILE__, __LINE__);   /* Free the DMA Descriptor List     */
        put_wc_plholder(pMirrorPlaceholder); /* Free the Placeholder     */
        KernelDispatch(EC_OK, --pILT, NULL, 0); /* All Done - OK            */
    }
}


/**
******************************************************************************
**
**  @brief      WC_MirrorBEComplete - Complete Mirroring to the BE NV Card Area
**
**              This completion handler will free the allocated resources
**              and then complete the original request.
**
**  @param      DMA Response Packet Address
**
**  @return     none
**
**  @attention  The DMA Response Packet must have the ILT passed into the
**              DMA Request function.  This function must also free the DMA
**              Response Packet that it receives.
**
**              Also - the original requesters ILT must be backed up one
**              level to complete it.
**
******************************************************************************
**/
void WC_MirrorBEComplete(struct NV_DMA_RSP_PKT* pRsp)
{
#ifdef DEBUG_MIRRORBECOMP
    UINT32  dmaStatus = EC_IO_ERR;      /* DMA Completion Status            */
#endif /* DEBUG_MIRRORBECOMP */
    struct ILT  *pOrigILT = NULL;       /* Original ILT Mirror Request      */
    struct ILT  *pMirrorILT = NULL;     /* Placeholder of Mirror Request    */

    /* Get the Response Status and the ILT from the Response Packet */
#ifdef DEBUG_MIRRORBECOMP
    dmaStatus = ((pRsp->dmaStsCtrl.errSts != 0) ? EC_IO_ERR : EC_OK);
#endif /* DEBUG_MIRRORBECOMP */
    pMirrorILT = pRsp->pILT;
#ifdef DEBUG_MIRRORBECOMP
    fprintf(stderr, "WC_Cache MBC: ILT=%p, DMA HW Sts=0x%x, DMA Status=0x%x, DMAList=%p, DMA Size=0x%x, OrigILT=%p\n",
                    pMirrorILT, pRsp->dmaStsCtrl.stsCtrl,
                    dmaStatus, pMirrorILT->cache_2_dlm.pcdDMAList, pMirrorILT->misc,
                    pMirrorILT->cache_2_dlm.pcdPhILT);
#endif

    /* Free the original DMA List and the DMA Response Packet */
    s_Free(pMirrorILT->cache_2_dlm.pcdDMAList, pMirrorILT->misc, __FILE__, __LINE__);
    s_Free(pRsp, sizeof(NV_DMA_RSP_PKT), __FILE__, __LINE__); /* Free the DMA Response Packet     */

    /*
     * Free the placeholder, and complete the original Mirror Request.
     *
     * Do not need to report the Error to the original Requester.  The NV
     * Card routines will set the Card into a failed state.
     */
    pOrigILT = pMirrorILT->cache_2_dlm.pcdPhILT;    /* Get the original Request         */
    put_wc_plholder(pMirrorILT);     /* Free Placeholder                 */
    KernelDispatch(EC_OK, --pOrigILT, NULL, 0); /* Complete request         */
}


/**
******************************************************************************
**
**  @brief      WC_RestoreData - Restores Write Cache data from NVRAM to DRAM
**
**              This function sets up the structure for the NVRAM card address,
**              and copies the data from NV Card to DRAM during power on
**              initialization so the code can operate on the DRAM copy.
**              This entails copying the FE and BE memory regions.
**
**              The copies will be done all at once without checking what is
**              valid or not.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void WC_RestoreData(void)
{
    struct NV_DMA_DD *DMAList = NULL;   /* DMA List array of data to xfer   */
    struct ILT       *pDMAPhILT = NULL; /* DMA Placeholder ILT              */
    UINT32           dmaListLength = 0; /* DMA List length to alloc and free*/
    INT32            rc = EC_IO_ERR;    /* Return Status from Functions     */

    /*
     * NV Card stuff not set up to handle getting and setting the info - use
     * hardcoded values.  Replace this section when the NV Card can malloc
     * memory.
     */
    gWC_NV_Mirror.wcctFELabel = WCCT_FE_NAME;
    gWC_NV_Mirror.wcctFESize = WcctSize;
    gWC_NV_Mirror.wcctFEHandle = SIZE_16MEG;

    gWC_NV_Mirror.wccFELabel = WCC_FE_NAME;
    gWC_NV_Mirror.wccFESize = WccSize;
    gWC_NV_Mirror.wccFEHandle = gWC_NV_Mirror.wcctFEHandle + gWC_NV_Mirror.wcctFESize;

    gWC_NV_Mirror.wctFELabel = WCT_FE_NAME;
    gWC_NV_Mirror.wctFESize = WctSize;
    gWC_NV_Mirror.wctFEHandle = gWC_NV_Mirror.wccFEHandle + gWC_NV_Mirror.wccFESize;

    gWC_NV_Mirror.wcbFELabel = WCB_FE_NAME;
    gWC_NV_Mirror.wcbFESize = gMPControllerCurrentConfig.wCacheSize;
    gWC_NV_Mirror.wcbFEHandle = gWC_NV_Mirror.wctFEHandle + gWC_NV_Mirror.wctFESize;

    gWC_NV_Mirror.wcctBELabel = WCCT_BE_NAME;
    gWC_NV_Mirror.wcctBESize = WcctSize;
    gWC_NV_Mirror.wcctBEHandle = gWC_NV_Mirror.wcbFEHandle + gWC_NV_Mirror.wcbFESize;

    gWC_NV_Mirror.wccBELabel = WCC_BE_NAME;
    gWC_NV_Mirror.wccBESize = WccSize;
    gWC_NV_Mirror.wccBEHandle = gWC_NV_Mirror.wcctBEHandle + gWC_NV_Mirror.wcctBESize;

    gWC_NV_Mirror.wctBELabel = WCT_BE_NAME;
    gWC_NV_Mirror.wctBESize = WctSize;
    gWC_NV_Mirror.wctBEHandle = gWC_NV_Mirror.wccBEHandle + gWC_NV_Mirror.wccBESize;

    gWC_NV_Mirror.wcbBELabel = WCB_BE_NAME;
    gWC_NV_Mirror.wcbBESize = gMPControllerCurrentConfig.wCacheSize;
    gWC_NV_Mirror.wcbBEHandle = gWC_NV_Mirror.wctBEHandle + gWC_NV_Mirror.wctBESize;

    WcbSize = gMPControllerCurrentConfig.wCacheSize;
    if (WcbSize == 0)
    {
       fprintf(stderr, "MicroMemory Card size is 0, make it default to 121 Meg\n");
       WcbSize = WC_SIZE_256MEG;
    }

    C_MaxTotalFlushBlocks = (WcbSize/BYTES_PER_SECTOR);
    C_MaxDirtyBlocks = ((WcbSize/BYTES_PER_SECTOR)*31/32);
    C_BlockStartFlushThresh = ((WcbSize/BYTES_PER_SECTOR)*6/8);
    C_BlockStopFlushThresh = ((WcbSize/BYTES_PER_SECTOR)*6/8);
    C_BlockFlushIncr = ((C_BlockStartFlushThresh-C_BlockStopFlushThresh)/4);

    C_ca.size = WcbSize;
    C_ca.numBlks = (WcbSize/BYTES_PER_SECTOR);

    /* If the WC Data is already restored, return from here */
    if(BIT_TEST(C_ca.status1,CA_RESTORE_DATA))
    {
        /* Early exit here */
        return;
    }

    /* End Hardcoded values */


    /*
     * Verify the health of the NV Memory - prompt for user input if necessary
     * to determine next course of action.
     */
    if ((NV_GetStatus()) != NV_STS_GOOD)
    {
        /* Early exit here */
        return;
    }

    /*
     * Allocate a placeholder ILT and space for the DMA List.  Then fill in the
     * ILT and DMA lists.
     */
    pDMAPhILT = get_wc_plholder();
    dmaListLength = 8 * sizeof(NV_DMA_DD);
    DMAList = s_MallocC(dmaListLength, __FILE__, __LINE__);
    pDMAPhILT->misc = dmaListLength;
    pDMAPhILT->cache_2_dlm.pcdDMAList = DMAList;
    pDMAPhILT->cr = K_xpcb;             /* Save PCB (go to sleep until done) */

    DMAList[0].pNextDD = &DMAList[1];   /* FE Write Cache Control Table     */
    DMAList[0].sysAddr = (UINT32)WcctAddr;
    DMAList[0].nvAddr = gWC_NV_Mirror.wcctFEHandle;
    DMAList[0].xferSize = gWC_NV_Mirror.wcctFESize;
    DMAList[0].wrtNotRd = false;
    fprintf(stderr, "WC_RestoreData:  Read FECT - SysAddr = 0x%x; NV Offset = 0x%x; Length = 0x%x\n",
            DMAList[0].sysAddr, DMAList[0].nvAddr, DMAList[0].xferSize);

    DMAList[1].pNextDD = &DMAList[2];   /* FE Write Cache Configuration     */
    DMAList[1].sysAddr = (UINT32)WccAddr;
    DMAList[1].nvAddr = gWC_NV_Mirror.wccFEHandle;
    DMAList[1].xferSize = gWC_NV_Mirror.wccFESize;
    DMAList[1].wrtNotRd = false;
    fprintf(stderr, "WC_RestoreData:  Read FECF - SysAddr = 0x%x; NV Offset = 0x%x; Length = 0x%x\n",
            DMAList[1].sysAddr, DMAList[1].nvAddr, DMAList[1].xferSize);

    DMAList[2].pNextDD = &DMAList[3];   /* FE Write Cache Tags              */
    DMAList[2].sysAddr = (UINT32)WctAddr;
    DMAList[2].nvAddr = gWC_NV_Mirror.wctFEHandle;
    DMAList[2].xferSize = gWC_NV_Mirror.wctFESize;
    DMAList[2].wrtNotRd = false;
    fprintf(stderr, "WC_RestoreData:  Read FETG - SysAddr = 0x%x; NV Offset = 0x%x; Length = 0x%x\n",
            DMAList[2].sysAddr, DMAList[2].nvAddr, DMAList[2].xferSize);

    DMAList[3].pNextDD = &DMAList[4];   /* FE Write Cache Buffers           */
    DMAList[3].sysAddr = (UINT32)WcbAddr;
    DMAList[3].nvAddr = gWC_NV_Mirror.wcbFEHandle;
    DMAList[3].xferSize = gWC_NV_Mirror.wcbFESize;
    DMAList[3].wrtNotRd = false;
    fprintf(stderr, "WC_RestoreData:  Read FEBF - SysAddr = 0x%x; NV Offset = 0x%x; Length = 0x%x\n",
            DMAList[3].sysAddr, DMAList[3].nvAddr, DMAList[3].xferSize);

    DMAList[4].pNextDD = &DMAList[5];   /* BE Write Cache Control Table     */
    DMAList[4].sysAddr = ((UINT32)WcctAddr) + BE_ADDR_OFFSET;
    DMAList[4].nvAddr = gWC_NV_Mirror.wcctBEHandle;
    DMAList[4].xferSize = gWC_NV_Mirror.wcctBESize;
    DMAList[4].wrtNotRd = false;
    fprintf(stderr, "WC_RestoreData:  Read BECT - SysAddr = 0x%x; NV Offset = 0x%x; Length = 0x%x\n",
            DMAList[4].sysAddr, DMAList[4].nvAddr, DMAList[4].xferSize);

    DMAList[5].pNextDD = &DMAList[6];   /* BE Write Cache Configuration     */
    DMAList[5].sysAddr = ((UINT32)WccAddr) + BE_ADDR_OFFSET;
    DMAList[5].nvAddr = gWC_NV_Mirror.wccBEHandle;
    DMAList[5].xferSize = gWC_NV_Mirror.wccBESize;
    DMAList[5].wrtNotRd = false;
    fprintf(stderr, "WC_RestoreData:  Read BECF - SysAddr = 0x%x; NV Offset = 0x%x; Length = 0x%x\n",
            DMAList[5].sysAddr, DMAList[5].nvAddr, DMAList[5].xferSize);

    DMAList[6].pNextDD = &DMAList[7];   /* BE Write Cache Tags              */
    DMAList[6].sysAddr = ((UINT32)WctAddr) + BE_ADDR_OFFSET;
    DMAList[6].nvAddr = gWC_NV_Mirror.wctBEHandle;
    DMAList[6].xferSize = gWC_NV_Mirror.wctBESize;
    DMAList[6].wrtNotRd = false;
    fprintf(stderr, "WC_RestoreData:  Read BETG - SysAddr = 0x%x; NV Offset = 0x%x; Length = 0x%x\n",
            DMAList[6].sysAddr, DMAList[6].nvAddr, DMAList[6].xferSize);

    DMAList[7].pNextDD = NULL;          /* BE Write Cache Buffers           */
    DMAList[7].sysAddr = ((UINT32)WcbAddr) + BE_ADDR_OFFSET;
    DMAList[7].nvAddr = gWC_NV_Mirror.wcbBEHandle;
    DMAList[7].xferSize = gWC_NV_Mirror.wcbBESize;
    DMAList[7].wrtNotRd = false;
    fprintf(stderr, "WC_RestoreData:  Read BEBF - SysAddr = 0x%x; NV Offset = 0x%x; Length = 0x%x\n",
            DMAList[7].sysAddr, DMAList[7].nvAddr, DMAList[7].xferSize);

    /*
     * Now queue the DMA request to the NV Card. If an error is returned the
     * Card will be flagged as bad, so no longer need to worry about DMAing
     * to or from it.
     *
     * NOTE: Call to NV_DMARequest could result in a context switch.
     */
    fprintf(stderr, "WC_RestoreData:  Beginning DMA Restore of Cache Data: DMA List=%p, DMA ILT=%p\n",
                    DMAList, pDMAPhILT);
    rc = NV_DMARequest(DMAList, WC_RestoreDataComp, pDMAPhILT);
    if (rc == EC_OK)
    {
        /* DMA Request sent - wait for it to complete. */
        TaskSetMyState(PCB_WAIT_IO);
        TaskSwitch();
    }
    BIT_SET(C_ca.status1,CA_RESTORE_DATA);

    fprintf(stderr, "WC_RestoreData:  Done with DMA Restore of Cache Data: DMA List=%p, DMA ILT=%p, rc=%d\n",
                    DMAList, pDMAPhILT, rc);
    fprintf(stderr, "WC_RestoreData:  FE signature = 0x%X, seqNo = 0x%x, S/N = 0x%08x\n"
                    "                 BE signature = 0x%X, seqNo = 0x%x, S/N = 0x%08x\n",
            WcctAddr->signature1, WcctAddr->seq, WcctAddr->cSerial,
            ((WT*)(((UINT32)WcctAddr) + BE_ADDR_OFFSET))->signature1,
            ((WT*)(((UINT32)WcctAddr) + BE_ADDR_OFFSET))->seq,
            ((WT*)(((UINT32)WcctAddr) + BE_ADDR_OFFSET))->cSerial);

    /* All Done with the DMA Transfer (good or bad), free the DMA list and ILT. */
    s_Free(DMAList, dmaListLength, __FILE__, __LINE__);   /* Free the DMA Descriptor List     */
    put_wc_plholder(pDMAPhILT);     /* Free Placeholder                 */
}


/**
******************************************************************************
**
**  @brief      WC_RestoreDataComp - DMA Completion for NVRAM to DRAM Copy
**
**              This is the Completion function that is called by the DMA
**              routines.  It will wake up the task that started the DMA.
**
**  @param      DMA Response Packet Address
**
**  @return     none
**
**  @attention  The DMA Response Packet must have the ILT passed into the
**              DMA Request function.  This function must also free the DMA
**              Response Packet that it receives.
**
**              Also - the original requesters ILT must be backed up one
**              level to complete it.
**
******************************************************************************
**/
static void WC_RestoreDataComp(struct NV_DMA_RSP_PKT* pRsp)
{
    struct ILT *pDMAPhILT = NULL;       /* DMA Placeholder ILT              */

    /*
     * Get the ILT used for the DMA and awaken the PCB waiting for this
     * completion.  Do not care about the completion, if bad, the card will be
     * set to the failed state and will not be usable anyway.
     */
    pDMAPhILT = pRsp->pILT;
    if (TaskGetState((PCB*)pDMAPhILT->cr) == PCB_WAIT_IO)
    {
#ifdef HISTORY_KEEP
CT_history_pcb("WC_RestoreDataComp setting ready pcb", (UINT32)(pRsp->pILT->cr));
#endif
        TaskSetState((PCB*)(pRsp->pILT->cr), PCB_READY);
    }
    fprintf(stderr, "DMA Restore Complete: DMA List=%p, DMA ILT=%p, Status=0x%08x, PCB=%p\n",
                    pDMAPhILT->cache_2_dlm.pcdDMAList, pDMAPhILT,
                    (UINT32)(pRsp->dmaStsCtrl.errSts), pDMAPhILT->cr);
}


/**
******************************************************************************
**
**  @brief      WC_PerfBypassCheck - Determine in current operation should bypass
**              the cache. Due to the additional overhead of mirroring the
**              write cache tags and data, at some workloads the performance
**              with Write Cache enabled can be worse than no Write Cache. This
**              routine attempts to identify those situations based on the
**              periodic throughput and customer IOPS of the HBAs. The limits
**              are also affected by the number of front end HBAs (available
**              throughput). The routine sets a global variable indicating the
**              "percent" of time the cache should be bypassed. The "percent" is
**              a count out of the total bypass loop count that the cache
**              should be bypassed.
**
**  @param      HBA_PERF_STATS* - Pointer to HBA performance stats
**
**  @return     None
**
******************************************************************************
**/

/*
 * Local Defines - these are variables for now, to allow tuning through
 * memory reads and writes, but can be changed to defines later.
 */
UINT32 WC_TOTAL_IOPS_LIMIT =   90000; /* MSHDEBUG change to about 52500 */
UINT32 WC_4_HBA_IOPS_LIMIT =   15000; /* Limit for 4 HBA system              */
UINT32 WC_2_HBA_IOPS_LIMIT =   18000; /* Limit for 2 HBA system              */
UINT32 WC_HBA_MBS_LIMIT=        1500;                /* tenths of MB/s       */
UINT32 WC_TOTAL_MBS_LIMIT=      3750;                /* tenths of MB/s       */

/* Global variables */
UINT8  WC_BypassDisable = FALSE; /* Method of disabling Bypass logic for test */
UINT32 WC_PerfBypassPct = 0;     /* Write Cache Performance Bypass "Percent"  */
UINT32 WC_BypassQlimit = 16;     /* C_ORC limit below which we don't bypass   */

UINT8 WC_BypassLoopCount = 0;    /* Current bypass loop count                */
UINT8 WC_BypassCount = 0;        /* # of times we bypassed in this loop      */
UINT8 WC_BypassMaxLoop = 1;      /* Disable for now                          */
UINT8 WC_BypassMaxCount = 6;     /* Maximum # of times to bypass in loop     */

/* Local variables
 * Global now  for visisbility through memread - make local later
 */
UINT32  perWriteMBs;
UINT32  perReadMBs;

void WC_ComputeWCBypassState(HBA_PERF_STATS* pStats)
{

    UINT32 hbaIOPsLimit;
    /*
     * Calculate the number of HBAs that are online. The online HBAs are stored
     * as bits in a the variable ispOnline. Walk the bits and count the active
     * ones.
     */
    UINT8 count;
    UINT32 bitPos = 1;           /* MAXISP - bitmap */
    UINT8 availHBAs = 0;

    for (count = 0; count < MAX_PORTS; ++count)
    {
        if (ispOnline & bitPos)
        {
            ++availHBAs;
        }
        bitPos = bitPos << 1;
    }

    /* Adjust the HBA IOPs limit, based on the number of HBAs */
    if (availHBAs == 4)
    {
        hbaIOPsLimit =  WC_4_HBA_IOPS_LIMIT;
    }
    else
    {
        hbaIOPsLimit =  WC_2_HBA_IOPS_LIMIT;
    }
    /*
     * Look to see if the Bypass code is disabled. If so, leave the Bypass flag
     * off. Make sure there are available HBAs, as calculations to bypass the
     * cache are based on them.
     */
    if ((WC_BypassDisable == FALSE) && (availHBAs))
    {

        /* Compute periodic throughput for the combined HBAs in tenths of MB/s. */
        perWriteMBs = (UINT32)( (UINT64)(pStats->perWrBlocks * 512) / 100000);
        perReadMBs = (UINT32)( (UINT64)(pStats->perRdBlocks  * 512) / 100000);


        /*
         * Bypass the cache if we near one of the following limits:
         * 1. Total HBA IOPs (with write mirroring, each write IOP results in 9
         *    HBA operations).
         * 2. Total MB/s supported by each HBA (with write cache mirroring, for
         *    each byte of write data recv'd, there are 2 additional bytes of
         *    data transferred. One to mirror to the other controller and 1 to
         *    recv mirror data from the other controller).
         * 3. Total IOPs supported by the controller (based on processor
         *    saturation).
         */
        if ((((pStats->perWrCmds * 9) + pStats->perRdCmds) / availHBAs) > hbaIOPsLimit ||
            (((perWriteMBs *3) + perReadMBs) / availHBAs) > WC_HBA_MBS_LIMIT ||
            ((perWriteMBs *3) + perReadMBs) > WC_TOTAL_MBS_LIMIT  ||   /* MSHDEBUG */
            (((pStats->perWrCmds *42) + (pStats->perRdCmds *10)) / 10) >  WC_TOTAL_IOPS_LIMIT) /* MSHDEBUG change to this*/
        {
            if (WC_PerfBypassPct <  WC_BypassMaxLoop)
            {
               ++WC_PerfBypassPct;
            }
        }
        else
        {
            if (WC_PerfBypassPct >  0)
            {
               --WC_PerfBypassPct;
            }
        }
    }
    else
    {
        WC_PerfBypassPct = 0;
    }
}


/**
******************************************************************************
**
**  @brief      WC_CheckBypass - Determine in current operation should bypass
**              the cache. Uses the bypass "percent" to determine if the cache
**              should be bypassed. Compares the "percent" against the loop
**              count which is incremented on each call (per write op). If the
**              loop count less than the bypass "percent", the cache is bypassed.
**              A "percent" of 0 never bypasses. A percent = to the max loop
**              count, always bypasses. The "percent" is recomputed periodically.
**
**  @param      None
**
**  @return     TRUE = Bypass the cache
**
******************************************************************************
**/
UINT32 WC_CheckBypass(void)
{
    UINT32 bypassFlag = TRUE;

    /*
     * Since the Cache qdepth is "large", check the performance bypass flag, to
     * determine if the cache should be bypassed. The bypass flag gets updated
     * periodically (with the HBA stats).
     */
    if (( WC_PerfBypassPct == 0)  ||  ( (UINT32)C_orc < WC_BypassQlimit))
    {
       bypassFlag = FALSE;
    }
    else
    {
       /* Set the Max bypass count, based on the Bypass "percent".  */
       WC_BypassMaxCount = WC_PerfBypassPct;

       /*
        * Keep track of the number of Bypasses in the last n loops and don't let
        * it exceed the Max bypass count.
        */
       ++WC_BypassLoopCount;
       ++WC_BypassCount;

       /*
        * If the Bypass count has exceeded the max count or the queue depth is
        * is not above the bypass limit, the skip the bypass.
        */
       if (WC_BypassCount >  WC_BypassMaxCount)
       {
           bypassFlag = FALSE;                       /* Skip the bypass */
       }

       /* Handle the loop wrap conditions */
       if (WC_BypassLoopCount >= WC_BypassMaxLoop)  {
           WC_BypassLoopCount = 0;
           WC_BypassCount = 0;
       }
    }
    return (bypassFlag);
}


/**
******************************************************************************
**
**  @brief      WC_CopyNVwWait - Copy DRAM to NV Memory local/remote with Wait
**
**              This function will copy the DRAM to NV Memory (either the FE
**              or BE) and possibly the MP.  The calling task will be put to
**              sleep until the copy completes.
**
**  @param      dst - where is the data going to (must be in the WC regions)
**  @param      src - where the data is coming from
**  @param      where - FE, BE, or Mirror Partner (MP assumes FE as well)
**  @param      length - length of data to copy
**
**  @return     none
**
******************************************************************************
**/
void WC_CopyNVwWait(void* dst, void* src, UINT8 where, UINT32 length)
{
    UINT32 rc = EC_OK;
    UINT8 region = 0;                   /* What part of NV Memory to write  */
    struct NV_DMA_DD *DMAList = NULL;
    struct ILT       *pDMAPhILT = NULL;

    /* Mirror the data to NV Memory (Local) */
    DMAList = s_MallocC(sizeof(*DMAList), __FILE__, __LINE__);

    DMAList->sysAddr = (UINT32)src;
    DMAList->xferSize = length;
    DMAList->wrtNotRd = TRUE;

    /* Set the offset in NV Memory based on which region the address lies in */
    if ((dst >= (void *)WcbAddr) &&
        (dst < (void *)((UINT32)WcbAddr + WcbSize)))
    {
        /* In the Buffer Region */
        BIT_SET(region,CAC0_DATA);
        if (where == MIRROR_BE)
        {
            /* Back end NV Memory */
            DMAList->nvAddr = (UINT32)dst - (UINT32)WcbAddr + gWC_NV_Mirror.wcbBEHandle;
        }
        else
        {
            /* Front End NV Memory or Mirror Partner (includes FE) */
            DMAList->nvAddr = (UINT32)dst - (UINT32)WcbAddr + gWC_NV_Mirror.wcbFEHandle;
        }
    }
    else if (dst >= (void *)WctAddr && dst < (void *)((UINT32)WctAddr + WctSize))
    {
        /* In the Tag Region */
        BIT_SET(region,CAC0_TAG);
        if (where == MIRROR_BE)
        {
            /* Back end NV Memory */
            DMAList->nvAddr = (UINT32)dst - (UINT32)WctAddr + gWC_NV_Mirror.wctBEHandle;
        }
        else
        {
            /* Front End NV Memory or Mirror Partner (includes FE) */
            DMAList->nvAddr = (UINT32)dst - (UINT32)WctAddr + gWC_NV_Mirror.wctFEHandle;
        }
    }
    else if (dst >= (void *)WcctAddr && dst < (void *)((UINT32)WcctAddr + WcctSize))
    {
        /* In the Control Table Region */
        BIT_SET(region,CAC0_WCCT);
        if (where == MIRROR_BE)
        {
            /* Back end NV Memory */
            DMAList->nvAddr = (UINT32)dst - (UINT32)WcctAddr + gWC_NV_Mirror.wcctBEHandle;
        }
        else
        {
            /* Front End NV Memory or Mirror Partner (includes FE) */
            DMAList->nvAddr = (UINT32)dst - (UINT32)WcctAddr + gWC_NV_Mirror.wcctFEHandle;
        }
    }
    else if (dst >= (void *)WccAddr && dst < (void *)((UINT32)WccAddr + WccSize))
    {
        /* In the Configuration Region */
        BIT_SET(region,CAC0_WCC);
        if (where == MIRROR_BE)
        {
            /* Back end NV Memory */
            DMAList->nvAddr = (UINT32)dst - (UINT32)WccAddr + gWC_NV_Mirror.wccBEHandle;
        }
        else
        {
            /* Front End NV Memory or Mirror Partner (includes FE) */
            DMAList->nvAddr = (UINT32)dst - (UINT32)WccAddr + gWC_NV_Mirror.wccFEHandle;
        }
    }
    else
    {
        /* In a non-supported region in the buffer card */
        fprintf(stderr, "WC_CopyNVwWait: Unsupported region = %p\n", dst);
        abort();
    }

#ifdef DEBUG_COPYNVWWAIT
    switch(where)
    {
        case MIRROR_BE:
            fprintf(stderr, "WC_CopyNVwWait: Mirror data to BE MMC. Dest=%p, Src=%p, NV offset=0x%X, Length=0x%X\n",
                            dst, src, DMAList->nvAddr, length);
            break;

        case MIRROR_FE:
        case MIRROR_MP:
            fprintf(stderr, "WC_CopyNVwWait: Mirror data to FE MMC. Dest=%p, Src=%p, NV offset=0x%X, Length=0x%X\n",
                            dst, src, DMAList->nvAddr, length);
            break;

        default:
            fprintf(stderr, "WC_CopyNVwWait: Invalid 'where' parameter=0x%x\n", where);
            abort();
    }
#endif

    /* Allocate and initialize a placeholder ILT. */
    pDMAPhILT = get_wc_plholder();
    pDMAPhILT->misc = sizeof(*DMAList);
    pDMAPhILT->cache_2_dlm.pcdDMAList = DMAList;
    pDMAPhILT->cr = K_xpcb;         /* Save PCB (go to sleep until done) */

    /*
     * Enqueue the DMA request to the NV Memory.
     * NOTE:  Call to NV_DMARequest could result in a context switch.
     */
    rc = NV_DMARequest(DMAList, WC_NV_DMARequestComp, pDMAPhILT);
    if (rc == EC_OK)
    {
        /* DMA Request sent - wait for it to complete */
        TaskSetMyState(PCB_WAIT_IO);
        TaskSwitch();
#ifdef DEBUG_COPYNVWWAIT
        fprintf(stderr, "WC_CopyNVwWait:  DMA of data Complete, rc = 0x%X\n", rc);
#endif
    }
    else
    {
        /*
         * DMA Request failed.
         * LSW - Handling TBD...
         */
        fprintf(stderr, "WC_CopyNVwWait:  DMA of data Failed, rc = 0x%X\n", rc);
    }

    /* Free the descriptor space & placeholder ILT */
    s_Free(DMAList, sizeof(*DMAList), __FILE__, __LINE__);
    put_wc_plholder(pDMAPhILT);

    /*
     * If this is for the Mirror Partner (MP) NV memory, copy the data
     * to the MP
     */
    if (where == MIRROR_MP)
    {
        /* Copy to the remote area if going to the MP (BE or another controller) */
        if (K_ficb->cSerial == K_ficb->mirrorPartner)
        {
            /* Copy the data to the BE NV Memory (recurse to do the BE) */
            WC_CopyNVwWait(dst, src, MIRROR_BE, length);
        }

#if !NO_REMOTE_MIRROR
        else if ((!(BIT_TEST(C_ca.status,CA_MIRRORBROKEN))) &&
                 (BIT_TEST(K_ii.status,II_MPFOUND)))
        {
            /* Write the remote data to the mirror partner. */
            if (WC_RemoteData(dst, src, length, region, CAC0_FC_WRTMEM) != 0)
            {
                /* If it fails, delay and try it again. */
                TaskSleepMS(1000);
                if (WC_RemoteData(dst, src, length, region, CAC0_FC_WRTMEM) != 0)
                {
                    /*
                     * If the Cache is not initialized, do not let the cache go
                     * enabled (otherwize, the error has already been reported
                     * by wc_setRemoteData).
                     */
                    if (!(BIT_TEST(K_ii.status,II_CINIT)))
                    {
                        if (BIT_TEST(C_ca.status,CA_ENA))
                        {
                            /* Clear the enable bit and set the enable pending bit. */
                            BIT_CLEAR(C_ca.status,CA_ENA);
                            BIT_SET(C_ca.status,CA_ENA_PEND);
                        }

                        /*
                         * Unable to set the MP data, set the Mirror Broken status
                         * if the MP is not ourself
                         */
                        if (K_ficb->mirrorPartner != K_ficb->cSerial)
                        {
                            BIT_SET(C_ca.status,CA_MIRRORBROKEN);
                        }
                    }
                }
            }
        }
#endif
    }
}


/**
******************************************************************************
**
**  @brief      WC_NV_DMARequestComp - DMA Completion for NV Mem <=> DRAM Copy
**
**              This is the Completion function that is called by the DMA
**              routines.  It will wake up the task that started the DMA.
**
**  @param      DMA Response Packet Address
**
**  @return     none
**
**  @attention  The DMA Response Packet must have the ILT passed into the
**              DMA Request function.  This function must also free the DMA
**              Response Packet that it receives.
**
**              Also - the original requester's ILT must be backed up one level
**              to complete it.
**
******************************************************************************
**/
static void WC_NV_DMARequestComp(struct NV_DMA_RSP_PKT* pRsp)
{
    struct ILT *pDMAPhILT = NULL;       /* DMA Placeholder ILT              */

    /*
     * Get the ILT used for the DMA and awaken the PCB waiting for this
     * completion.  Do not care about the completion, if bad, the card will be
     * set to the failed state and will not be usable anyway.
     */
    pDMAPhILT = pRsp->pILT;
    if (TaskGetState((PCB*)pDMAPhILT->cr) == PCB_WAIT_IO)
    {
#ifdef HISTORY_KEEP
CT_history_pcb("WC_NV_DMARequestComp setting ready pcb", (UINT32)(pDMAPhILT->cr));
#endif
        TaskSetState((PCB*)(pDMAPhILT->cr), PCB_READY);
    }
    s_Free(pRsp, sizeof(NV_DMA_RSP_PKT), __FILE__, __LINE__); /* Free the DMA Response Packet     */
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
