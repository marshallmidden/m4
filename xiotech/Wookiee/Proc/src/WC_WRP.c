/* $Id: WC_WRP.c 145021 2010-08-03 14:16:38Z m4 $ */
/**
******************************************************************************
**
**  @file       WC_WRP.c
**
**  @brief      Write Cache WRP - Main Routines
**
**  Contains the Main Routines for the Write Cache WRP Functions.
**
**  Copyright (c) 2005-2008 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#include "WC_WRP.h"                     /* Write Cache WRP Definitions      */

#include "cache.h"                      /* Cache definitions                */
#include "vcd.h"                        /* Virtual Cache Device definitions */
#include "defbe.h"                      /* BE Define definitions            */
#include "fr.h"                         /* Flight Recorder definitions      */
#include "ilt.h"                        /* ILT definitions                  */
#include "LOG_Defs.h"                   /* Logging definitions              */
#include "LL_LinuxLinkLayer.h"          /* Link Layer definitions           */
#include "misc.h"                       /* Miscellaneous defines            */
#include "MR_Defs.h"                    /* MRP definitions                  */
#include "pcb.h"                        /* PCB definitions                  */
#include "pm.h"                         /* Packet Manager definitions       */
#include "qu.h"                         /* Queue definitions                */
#include "QU_Library.h"                 /* Queueing functions               */
#include "vdd.h"                        /* VDD definitions                  */
#include "wcache.h"                     /* Write Cache definitions          */
#include "XIO_Macros.h"                 /* Standard Xiotech macros          */
#include "XIO_Std.h"                    /* Standard Xiotech functions       */
#include "XIO_Types.h"                  /* Standard Xiotech types           */
#include "mem_pool.h"
#include "CT_defines.h"

#include <stdio.h>

/*
******************************************************************************
** Private defines - constants
******************************************************************************
*/
/* #define DEBUG_WRPEXEC       1           PrintF on WRP Exec Function      */
/* #define DEBUG_WRPQUEUE      1           PrintF on WRP Queue Function     */
/* #define DEBUG_TDISEXEC      1           PrintF on Temp Disable Exec Func */
/* #define DEBUG_VDISKDISABLE  1           PrintF on VDisk Disable call     */
/* #define DEBUG_VDISKDISABLECOMP 1        PrintF on VDisk Disable Complete */

/*
******************************************************************************
** Private variables
******************************************************************************
*/
LOG_FIRMWARE_ALERT_PKT eSoftFault;  /* Log Message for Software Faults      */
#ifdef FRONTEND
QU  wrpQueue;                       /* WRP Queue Structure                  */
QU  tDisQueue;                      /* Temporary Disable Queue Structure    */
#endif  /* FRONTEND */

/*
******************************************************************************
** Public variables - externed in the header file
******************************************************************************
*/
#ifdef FRONTEND
extern CA C_ca;                     /* Cache Structure                      */
#endif  /* FRONTEND */

/*
******************************************************************************
** Public Functions not externed in header files
******************************************************************************
*/
#ifdef FRONTEND
extern void KernelDispatch(UINT32 returnCode, ILT *pILT,  MR_PKT *pMRP, UINT32 w0);
extern void CT_LC_WC_WRPExec(void);
extern void CT_LC_WC_TDisExec(void);
#endif  /* FRONTEND */

extern UINT32 LL_QueueMessageToSend(ILT *pILT);
extern UINT32 WC_FlushInvalidateVID(UINT16 VID);

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
#ifndef FRONTEND
static void WC_VDiskDisableComplete(UINT8 rc, ILT * pILT);
#endif  /* !FRONTEND */

/*
******************************************************************************
** Code Start
******************************************************************************
*/

#ifdef FRONTEND
/**
******************************************************************************
**
**  @brief      WC_WRPExec - Handles incoming WRPs from the BE
**
**              Decodes the request and handles appropriately.
**
**              This executive processes a queue of supplied WRPs.  Each type
**              of request is handled by the appropriate handler.
**
**  @param      none
**
**  @return     none
**
**  @attention  This is a task that never returns.  To enqueue an ILT to this
**              task see the <WC_WRPQueue> function.  The ILT must have:
**                  <il_w4>     - WRP request being processed
**                  <il_cr>     - Completion routine to call when done with
**                      first parm  - Completion Status
**                      second parm - ILT
**
******************************************************************************
**/
NORETURN
void WC_WRPExec(void)
{
    ILT*    pWorkingILT = NULL;         /* Working ILT                      */
    UINT16  reqFunction = 0;            /* WRP Function code                */
    UINT16  reqVID = 0xFFFF;            /* WRP Requested VID                */
    VCD*    pVCD = NULL;                /* Pointer to the VCD               */
    WRP*    pWRP = NULL;                /* Pointer to the WRP               */

    /*
    **  Setup queue control block
    */
    wrpQueue.pcb = K_xpcb;              /* Set up my PCB                    */
    wrpQueue.head = NULL;               /* Set up the Head ILT              */
    wrpQueue.tail = NULL;               /* Set up the Tail ILT              */
    wrpQueue.qcnt = 0;                  /* Set up the Count of ILTs on queue*/

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
        ** Loop on items queued
        */
        while (wrpQueue.head != NULL)
        {
            /*
            ** Get the head ILT, the WRP, the VID, the VCD,
            ** and the requested function
            */
            pWorkingILT = QU_DequeReqILT(&wrpQueue);
            pWRP = (WRP*)pWorkingILT->ilt_normal.w4;
            reqVID = pWRP->vid;
            reqFunction = pWRP->function;
            pVCD = vcdIndex[reqVID];
#ifdef DEBUG_WRPEXEC
            fprintf(stderr, "WC_WRP Exec: Main ILT = %p; VID = %d; Command = 0x%x; Queue Cnt = %d\n", pWorkingILT, reqVID, reqFunction,wrpQueue.qcnt);
#endif  /* DEBUG_WRPEXEC */
            /*
            ** Validate the VID and if incorrect, log a Software Fault and
            ** return an Invalid VID error
            */
            if (pVCD != NULL)
            {
#if DEBUG_FLT_REC_CACHE
                MSC_FlightRec(((reqVID<<16)|FR_WC_WRPEXEC), (UINT32)pWorkingILT,
                    (UINT32)pWRP,
                        (((pVCD->tDisCnt)<<24)|((pVCD->stat)<<16)|reqFunction));
#endif  /* DEBUG_FLT_REC_CACHE */
                /*
                ** Valid VID, now execute the requested function (if valid)
                */
                switch (reqFunction)
                {
                    case WC_SET_T_DISABLE :
                    {
#ifdef DEBUG_WRPEXEC
                        fprintf(stderr, "WC_WRP Exec: Disable VCD = %p; VCD Status = 0x%x; TDisable Count = %d\n", pVCD, pVCD->stat, pVCD->tDisCnt);
#endif  /* DEBUG_WRPEXEC */
                        /*
                        ** A temporary Disable of the VDisk is requested
                        **
                        ** Turn on the Temporary Disable bit, increment the
                        ** Temporary Disable counter, and determine if
                        ** data is in cache.  If so, let the Temporary Disable
                        ** Task do the flushing of the VCD data.
                        */
                        BIT_SET(pVCD->stat,VC_TEMP_DISABLE);
                        ++pVCD->tDisCnt;
                        if ((BIT_TEST(pVCD->stat,VC_CACHED)) &&
                            (C_ca.stopCnt == 0) &&
                            ((pVCD->pCache != NULL) ||
                             (pVCD->writeCount != 0)))
                        {
#ifdef DEBUG_WRPEXEC
                            fprintf(stderr, "WC_WRP Exec: Disable Cached VCD = %p; VCD Status = 0x%x; TDisable Count = %d,\n", pVCD, pVCD->stat, pVCD->tDisCnt);
                            fprintf(stderr, "                            Cache Tree = %p; VCD Write Count = %d\n", pVCD->pCache, pVCD->writeCount);
#endif  /* DEBUG_WRPEXEC */
                            QU_EnqueReqILT (pWorkingILT, &tDisQueue);
                        }
                        else
                        {
                            /*
                            ** Not Cached at the moment or no data in cache
                            ** or nothing coming into the cache - complete the
                            ** request
                            */
#ifdef DEBUG_WRPEXEC
                            fprintf(stderr, "WC_WRP Exec: Disable Not Cached VCD = %p; VCD Status = 0x%x; TDisable Count = %d,\n", pVCD, pVCD->stat, pVCD->tDisCnt);
                            fprintf(stderr, "                                Cache Tree = %p; VCD Write Count = %d\n", pVCD->pCache, pVCD->writeCount);
#endif  /* DEBUG_WRPEXEC */
                            pWRP->status = EC_OK;   /* Set the WRP Status   */
                            KernelDispatch(EC_OK, pWorkingILT-1, NULL, 0);
                        }

                    }
                    break;

                    case WC_CLEAR_T_DISABLE :
                    {
#ifdef DEBUG_WRPEXEC
                        fprintf(stderr, "WC_WRP Exec: Clear VCD = %p; VCD Status = 0x%x; TDisable Count = %d\n", pVCD, pVCD->stat, pVCD->tDisCnt);
#endif  /* DEBUG_WRPEXEC */
                        /*
                        ** A request to Clear the temporary Disable
                        **
                        ** Clear the Temporary Disable bit, if there is only
                        ** this request outstanding, and complete the
                        ** request
                        */
                        if (pVCD->tDisCnt != 0)
                        {
                            /*
                            ** Do not let the count go negative due to too many
                            ** Clears - just ignore the extraneous ones.
                            */
                            --pVCD->tDisCnt;
#ifdef DEBUG_WRPEXEC
                            fprintf(stderr, "WC_WRP Exec: NonZero Count VCD = %p; TDisable Count = %d\n", pVCD, pVCD->tDisCnt);
#endif  /* DEBUG_WRPEXEC */
                        }
                        if (pVCD->tDisCnt == 0)
                        {
                            BIT_CLEAR(pVCD->stat,VC_TEMP_DISABLE);
#ifdef DEBUG_WRPEXEC
                            fprintf(stderr, "WC_WRP Exec: Last TDisable Count VCD = %p; VCD Status = 0x%x; TDisable Count = %d\n", pVCD, pVCD->stat, pVCD->tDisCnt);
#endif  /* DEBUG_WRPEXEC */
                        }
                        pWRP->status = EC_OK;   /* Set the WRP Status       */
                        KernelDispatch(EC_OK, pWorkingILT-1, NULL, 0);
                    }
                    break;

                    default:
                    {
                        /*
                        ** An invalid function was requested.  Log a Software
                        ** Fault and return an invalid function code.
                        */
                        eSoftFault.header.length = 24;
                        eSoftFault.data.errorCode = CAC_SFT9;
                        eSoftFault.data.data[0] = reqVID;
                        eSoftFault.data.data[1] = reqFunction;
                        eSoftFault.data.data[2] = (UINT32)pWorkingILT;
                        eSoftFault.data.data[3] = (UINT32)pWRP;
                        eSoftFault.data.data[4] = (UINT32)pVCD;
                        MSC_SoftFault(&eSoftFault);
                        pWRP->status = EC_INV_FUNC; /* Set the WRP Status   */
                        KernelDispatch(EC_INV_FUNC, pWorkingILT-1, NULL, 0);
                    }
                    break;
                }
            }
            else
            {
                /*
                ** An invalid VID was given in the WRP, Log a software Fault
                ** and return an Invalid VID error
                */
                eSoftFault.header.length = 24;
                eSoftFault.data.errorCode = CAC_SFT10;
                eSoftFault.data.data[0] = reqVID;
                eSoftFault.data.data[1] = reqFunction;
                eSoftFault.data.data[2] = (UINT32)pWorkingILT;
                eSoftFault.data.data[3] = (UINT32)pWRP;
                eSoftFault.data.data[4] = (UINT32)pVCD;
                MSC_SoftFault(&eSoftFault);
                pWRP->status = EC_INV_VID; /* Set the WRP Status            */
                KernelDispatch(EC_INV_VID, pWorkingILT-1, NULL, 0);
            }
        }
    }
}



/**
******************************************************************************
**
**  @brief      WC_WRPQueue - Queue the requested ILT to the WRP Handler task
**
**              This function queues the requested WRP to the WRP Handler task
**              and returns to the requester.
**
**  @param      dummy - g0 passed from Assembler is Assember Code Address
**  @param      pILT - ILT Address
**
**  @return     none
**
**  @attention  none
**
******************************************************************************
**/
void WC_WRPQueue(void* dummy UNUSED, ILT* pILT)
{
    /*
    ** Copy the WRP address from the previous ILT layer to this ILT layer in
    ** the expected slot
    */
    pILT->ilt_normal.w4 = (pILT-1)->ilt_normal.w0;

#ifdef DEBUG_WRPQUEUE
    fprintf(stderr, "WC_WRPQueue: Main ILT = %p; WRP = %p; Queue Count = %d\n", pILT, (WRP*)(pILT->ilt_normal.w4), wrpQueue.qcnt);
#endif  /* DEBUG_WRPQUEUE */
#if DEBUG_FLT_REC_CACHE
    MSC_FlightRec(FR_WC_WRPQUE, (UINT32)pILT, pILT->ilt_normal.w4, wrpQueue.qcnt);
#endif  /* DEBUG_FLT_REC_CACHE */
    /*
    ** Queue the request to the WRP Exec
    */
    QU_EnqueReqILT (pILT, &wrpQueue);
}



/**
******************************************************************************
**
**  @brief      WC_TDisExec - Handles Flushing of VIDs in the Temp Disable State
**
**              This executive processes a queue of supplied ILTs that have
**              pointers to WRPs that are in the temporary Flush State.  Each
**              request will flush any data for the requested VID and when
**              the flush is complete, complete the original request.
**
**  @param      none
**
**  @return     none
**
**  @attention  This is a task that never returns.  To enqueue an ILT to this
**              task see the <QU_EnqueReqILT> function with an input of
**              tDisQueue.  The ILT must have:
**                  <il_w4>     - WRP request being processed
**                  <il_cr>     - Completion routine to call when done with
**                      first parm  - Completion Status
**                      second parm - ILT
**
******************************************************************************
**/
NORETURN
void WC_TDisExec(void)
{
    ILT    *pWorkingILT = NULL;         /* Working ILT                      */
    ILT    *pNextILT = NULL;            /* Next ILT to be processed         */
    ILT    *pReturnedILT = NULL;        /* Returned ILT from Dequeue        */
    UINT16  reqVID = 0xFFFF;            /* WRP Requested VID                */
    VCD    *pVCD = NULL;                /* Pointer to the VCD               */
    WRP    *pWRP = NULL;                /* Pointer to the WRP               */
    LOG_WRITE_FLUSH_COMPLETE_PKT logFlushCompleteMsg; /* Log Flush Done msg */

    /* Setup queue control block */
    tDisQueue.pcb = K_xpcb;             /* Set up my PCB                    */
    tDisQueue.head = NULL;              /* Set up the Head ILT              */
    tDisQueue.tail = NULL;              /* Set up the Tail ILT              */
    tDisQueue.qcnt = 0;                 /* Set up the Count of ILTs on queue*/

    /* Task runs forever */
    while (TRUE)                        /* Always keep task running         */
    {
        /* Set process to wait for signal and Exchange process */
        TaskSetMyState(PCB_NOT_READY);
        TaskSwitch();

        /* Get the first ILT pointer and then loop on items queued */
        pWorkingILT = tDisQueue.head;
        while (pWorkingILT != NULL)
        {
            /* Get the next ILT on the list */
            pNextILT = pWorkingILT->fthd;

            /* Get from the ILT the WRP, the VID, and the VCD */
            pWRP = (WRP*)pWorkingILT->ilt_normal.w4;
            reqVID = pWRP->vid;
            pVCD = vcdIndex[reqVID];

            /*
             * Validate the VID and if incorrect, complete the request as
             * Successful (VDisk was deleted after start of the Temporary
             * Disable and now there is no VDisk so there is no Flush pending)
             */
            if (pVCD != NULL)
            {
#ifdef DEBUG_WRPEXEC
                fprintf(stderr, "WC_TDis Exec: ILT = %p; VID = %d; VCD Status = 0x%x, Queue Count = %d\n", pWorkingILT, reqVID, pVCD->stat, tDisQueue.qcnt);
#endif  /* DEBUG_WRPEXEC */
#if DEBUG_FLT_REC_CACHE
                MSC_FlightRec(((reqVID<<16) | FR_WC_TDEXEC),
                    (UINT32)pWorkingILT, (UINT32)pWRP,
                    (((pVCD->stat)<<24) | ((pVCD->writeCount)<<16) |
                     ((UINT32)(pVCD->pCache) & 0x0000FFFF)));
#endif  /* DEBUG_FLT_REC_CACHE */
                /*
                 * Valid VID, determine if it has completed flushing yet or the
                 * VCD is no longer in the Temporary Disable state and
                 * if so, return Success to the requester.  Else, flush the VID.
                 */
                if ((BIT_TEST(pVCD->stat,VC_TEMP_DISABLE)) &&
                    (BIT_TEST(pVCD->stat,VC_CACHED)) &&
                    ((pVCD->pCache != NULL) ||
                     (pVCD->writeCount != 0)))
                {
                    if (C_ca.stopCnt != 0)
                    {
                        pWRP->status = EC_OK; /* TBD */
                        pReturnedILT = QU_DequeThisILT(pWorkingILT, &tDisQueue);
                        KernelDispatch(EC_OK, pWorkingILT-1, NULL, 0);
                    }
                    else if (BIT_TEST(pVCD->stat,VC_ERROR))
                    {
                        /*
                         * VDisk is in Error State - Flushing cannot occur.
                         * Report the Error to the caller, remove the ILT,
                         * and exit
                         */
#ifdef DEBUG_WRPEXEC
                        fprintf(stderr, "WC_TDis Exec: VDisk in Error State - Flush ILT = %p; VID = %d; Cache Tree Pointer = %p, VCD Write Count = %d\n", pWorkingILT, reqVID, pVCD->pCache, pVCD->writeCount);
#endif  /* DEBUG_WRPEXEC */
                        pWRP->status = EC_INOP_VDEV; /* Set WRP Status - Error*/
                        pReturnedILT = QU_DequeThisILT(pWorkingILT, &tDisQueue);
                        KernelDispatch(EC_INOP_VDEV, pWorkingILT-1, NULL, 0);

                        /*
                         * If the Returned ILT was not on the queue, report a
                         * software fault
                         */
                        if (pReturnedILT != pWorkingILT)
                        {
                            eSoftFault.header.length = 20;
                            eSoftFault.data.errorCode = CAC_SFT11;
                            eSoftFault.data.data[0] = (UINT32)&tDisQueue;
                            eSoftFault.data.data[1] = (UINT32)pWorkingILT;
                            eSoftFault.data.data[2] = (UINT32)pReturnedILT;
                            eSoftFault.data.data[3] = (UINT32)pVCD;
                            MSC_SoftFault(&eSoftFault);
                        }
                    }
                    else
                    {
                        /*
                         * Data still in Cache, Try flushing again (don't care if
                         * all being flushed or just some - will check on the next
                         * loop).
                         */
#ifdef DEBUG_WRPEXEC
                        fprintf(stderr, "WC_TDis Exec: Flush ILT = %p; VID = %d; Cache Tree Pointer = %p, VCD Write Count = %d\n", pWorkingILT, reqVID, pVCD->pCache, pVCD->writeCount);
#endif  /* DEBUG_WRPEXEC */
                        WC_FlushInvalidateVID(reqVID);
                    }
                }
                else
                {
                    /*
                     * All data has been flushed or no longer requesting a
                     * temporary disable, remove the ILT from the list
                     * and return Success
                     */
#ifdef DEBUG_WRPEXEC
                    fprintf(stderr, "WC_TDis Exec: No Flush ILT = %p; VID = %d; Cache Tree Pointer = %p, VCD Write Count = %d\n", pWorkingILT, reqVID, pVCD->pCache, pVCD->writeCount);
#endif  /* DEBUG_WRPEXEC */
                    pWRP->status = EC_OK;   /* Set WRP Status to Good       */
                    pReturnedILT = QU_DequeThisILT(pWorkingILT, &tDisQueue);
                    KernelDispatch(EC_OK, pWorkingILT-1, NULL, 0);

                    /*
                     * If the Returned ILT was not on the queue, report a
                     * software fault
                     */
                    if (pReturnedILT != pWorkingILT)
                    {
                        eSoftFault.header.length = 20;
                        eSoftFault.data.errorCode = CAC_SFT11;
                        eSoftFault.data.data[0] = (UINT32)&tDisQueue;
                        eSoftFault.data.data[1] = (UINT32)pWorkingILT;
                        eSoftFault.data.data[2] = (UINT32)pReturnedILT;
                        eSoftFault.data.data[3] = (UINT32)pVCD;
                        MSC_SoftFault(&eSoftFault);
                    }

                    /* Log a message that the Flush has completed for this VID */
                    logFlushCompleteMsg.header.event = LOG_WRITE_FLUSH_COMPLETE;
                    logFlushCompleteMsg.data.vid = reqVID;

                    /* Send the log message. */
                    MSC_LogMessageStack(&logFlushCompleteMsg, sizeof(LOG_WRITE_FLUSH_COMPLETE_PKT));

                }
            }
            else
            {
                /*
                 * The VDisk must have been deleted after the Temporary Disable
                 * was started - remove the ILT from the list and
                 * return good (there is no more data to flush)
                 */
#ifdef DEBUG_WRPEXEC
                fprintf(stderr, "WC_TDis Exec: No VDisk ILT = %p; VID = %d; VCD = %p\n", pWorkingILT, reqVID, pVCD);
#endif  /* DEBUG_WRPEXEC */
                pReturnedILT = QU_DequeThisILT(pWorkingILT, &tDisQueue);
                pWRP->status = EC_OK;   /* Set WRP Status to Good           */
                KernelDispatch(EC_OK, pWorkingILT-1, NULL, 0);

                /*
                 * If the Returned ILT was not on the queue, report a
                 * software fault
                 */
                if (pReturnedILT != pWorkingILT)
                {
                    eSoftFault.header.length = 20;
                    eSoftFault.data.errorCode = CAC_SFT11;
                    eSoftFault.data.data[0] = (UINT32)&tDisQueue;
                    eSoftFault.data.data[1] = (UINT32)pWorkingILT;
                    eSoftFault.data.data[2] = (UINT32)pReturnedILT;
                    eSoftFault.data.data[3] = (UINT32)pVCD;
                    MSC_SoftFault(&eSoftFault);
                }
            }

            /*
             * If all the list has been traversed once and more work is needed,
             * wait a little and then start over.  If there are no more ILTs,
             * get out of the loop (will happen because pWorkingILT will
             * be NULL), else continue with the next ILT in the list.
             */
            if ((pNextILT == NULL) &&
                (tDisQueue.head != NULL))
            {
#ifdef DEBUG_WRPEXEC
                fprintf(stderr, "WC_TDis Exec: More Work but at end of list Queue Count = %d\n", tDisQueue.qcnt);
#endif  /* DEBUG_WRPEXEC */
                /* At the End of the list but there are still ILTs on the queue. */
                TaskSleepMS(1);                 /* Wait for the minimum and contine */
                pWorkingILT = tDisQueue.head;   /* Get the current head       */
            }
            else
            {
#ifdef DEBUG_WRPEXEC
                fprintf(stderr, "WC_TDis Exec: Possibly More Work Next ILT = %p; Queue Count = %d\n", pNextILT, tDisQueue.qcnt);
#endif  /* DEBUG_WRPEXEC */
                /* Not at the end of list or there are no more ILTs on the list */
                pWorkingILT = pNextILT;
            }
        }
    }
}

#else   /* FRONTEND -- i.e. Backend  */

/**
******************************************************************************
**
**  @brief      WC_VDiskDisable - Temporarily Disable/Re-enable WC on VDisk
**
**              The function will create a WRP and forward it to the FE to
**              temporarily disable or re-enable the Write Cache for a
**              particular VDisk. It will not return until the request has
**              completed.
**
**  @param      VID - VDisk ID of VDisk being Write Cache Temporarily Disabled
**  @parm       function - To Set or Clear the Temporary Disable of a VID
**
**  @return     None
**
**  @attention  None
**
******************************************************************************
**/
UINT8 WC_VDiskDisable(UINT16 VID, UINT16 function)
{
    ILT    *pILT = NULL;                /* ILT used in executing the WRP    */
    WRP    *pWRP = NULL;                /* WRP used in Temp Disabling WC    */
    VDD    *pVDD = NULL;                /* VDD                              */
    UINT8   returnStatus = EC_IO_ERR;   /* Return Status - set Bad          */

#ifdef DEBUG_VDISKDISABLE
    fprintf(stderr, "WC_VDiskDisable: VID = %d; Function called with = 0x%x\n", VID, function);
#endif  /* DEBUG_VDISKDISABLE */
    /*
    ** If the function is a Clear - always send it blindly using LL_SendPacket
    ** and do not wait for the completion.  If the funciton is a Set, then
    ** if WC is enabled, wait for the completion, else blindly send it as well.
    ** We need the FE to set/clear the bit because WC could be enabled and
    ** disabled at any time, including the middle of a Temporary Disable.
    */
    if (VID < MAX_VIRTUAL_DISKS && (pVDD = gVDX.vdd[VID]) != NULL)
    {
#if DEBUG_FLT_REC_CACHE
        MSC_FlightRec(((VID<<16) | FR_WC_VDISKTDIS), (UINT32)pVDD,
                                (((pVDD->mirror)<<16) | pVDD->attr), function);
#endif  /* DEBUG_FLT_REC_CACHE */
        /*
        ** Allocate an ILT and WRP for processing and fill in the proper stuff
        */
        pILT = get_ilt();
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)pILT);
#endif /* M4_DEBUG_ILT */
        pWRP = s_MallocC(sizeof(WRP), __FILE__, __LINE__);
        pILT->ilt_normal.w4 = (UINT32)pWRP;        /* WRP in the ILT for Link Layer    */
        pILT->cr = WC_VDiskDisableComplete; /* Completion Routine           */
        pWRP->vid = VID;                /* Set the VID in the WRP           */
        pWRP->function = function;      /* Set the Function code in the WRP */

        /* Valid VID, determine if Write Cache is Enabled on this VDisk */
        if (BIT_TEST(pVDD->attr,VD_BCACHEEN) && function == WC_SET_T_DISABLE)
        {
#ifdef DEBUG_VDISKDISABLE
            fprintf(stderr, "WC_VDiskDisable: Queue Wait ILT = %p; WRP = %p; VID = %d; VDD Attributes = 0x%x; Function called with = 0x%x\n", pILT, pWRP, VID, pVDD->attr, function);
#endif  /* DEBUG_VDISKDISABLE */

            /* Queue the WRP to the Link Layer and wait for its completion */
            pILT->misc = (UINT32)K_xpcb; /* Store this PCB to wake up later */
            if ((returnStatus = LL_QueueMessageToSend(pILT)) == EC_OK)
            {
                TaskSetMyState(PCB_WAIT_IO); /* Go to sleep until done      */
                TaskSwitch();
                returnStatus = pWRP->status; /* Op Done - Get Status        */
            }

            /* Now that the op is complete, free the ILT and WRP */
#ifdef DEBUG_VDISKDISABLE
            fprintf(stderr, "WC_VDiskDisable: Queue Wait Done ILT = %p; WRP = %p; VID = %d; Return Status = 0x%x\n", pILT, pWRP, VID, returnStatus);
#endif  /* DEBUG_VDISKDISABLE */
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)pILT);
#endif /* M4_DEBUG_ILT */
            put_ilt(pILT);
            s_Free(pWRP, sizeof(WRP), __FILE__, __LINE__);
        }
        else
        {
#ifdef DEBUG_VDISKDISABLE
            fprintf(stderr, "WC_VDiskDisable: No Wait ILT = %p; WRP = %p; VID = %d; VDD Attributes = 0x%x; Function called with = 0x%x\n", pILT, pWRP, VID, pVDD->attr, function);
#endif  /* DEBUG_VDISKDISABLE */
            /*
             * Cache is not Enabled or this is a Clear, send the WRP to the FE
             * and return (do not need to wait for the completion)
             */
            pILT->misc = (UINT32)NULL;  /* No task is waiting, free ILT & WRP */
            returnStatus = LL_QueueMessageToSend(pILT); /* Normally Good    */
        }
    }
    else
    {
        /*
         * Invalid VID was passed in, log a Software Fault and then return the
         * error to the caller
         */
        eSoftFault.header.length = 16;
        eSoftFault.data.errorCode = CAC_SFT12;
        eSoftFault.data.data[0] = (UINT32)VID;
        eSoftFault.data.data[1] = (UINT32)pVDD;
        eSoftFault.data.data[2] = (UINT32)function;
        MSC_SoftFault(&eSoftFault);
        returnStatus = EC_INV_VID;
    }

    return(returnStatus);
}


/**
******************************************************************************
**
**  @brief      WC_VDiskDisableComplete - Complete a VDiskDisable request
**
**              Expects pILT->misc to be the PCB Pointer.  If it is NULL, then
**              just free the ILT and WRP.  Else, activate the PCB that is
**              waiting for the op to complete.
**
**  @param      rc - Return Code from calling routine
**  @param      pILT - Pointer to the original ILT request
**
**  @return     None
**
**  @attention  None
**
******************************************************************************
**/
static void WC_VDiskDisableComplete(UINT8 rc
#if defined(DEBUG_VDISKDISABLECOMP) || defined(DEBUG_FLT_REC_CACHE)
                                      UNUSED
#endif  /* DEBUG_VDISKDISABLECOMP || DEBUG_FLT_REC_CACHE */
                                                  ,
                             ILT * pILT)
{
    PCB*    pWaitingPCB = NULL;         /* Waiting PCB                      */
    WRP*    pWRP = NULL;                /* WRP being processed              */

#ifdef DEBUG_VDISKDISABLECOMP
    fprintf(stderr, "WC_VDiskDisableComp: ILT = %p; PCB = %p; WRP = %p; rc = %d\n", pILT, (PCB*)pILT->misc, (WRP*)pILT->ilt_normal.w4, rc);
#endif  /* DEBUG_VDISKDISABLECOMP */
#if DEBUG_FLT_REC_CACHE
    MSC_FlightRec(((rc<<16) | FR_WC_VDDISCMP), (UINT32)pILT,
                                        (UINT32)pILT->misc, (UINT32)pILT->ilt_normal.w4);
#endif  /* DEBUG_FLT_REC_CACHE */
    /*
    ** If the ILT points to a waiting Task, wake it up and let it handle the
    ** completion.  Else, free the ILT and WRP
    */
    if ((pWaitingPCB = (PCB*)pILT->misc) != NULL)
    {
        /* Waiting Task, activate it. */
        if (TaskGetState(pWaitingPCB) == PCB_WAIT_IO)
        {
#ifdef HISTORY_KEEP
CT_history_pcb("WC_VDiskDisableComplete setting ready pcb", (UINT32)pWaitingPCB);
#endif  /* HISTORY_KEEP */
            TaskSetState(pWaitingPCB, PCB_READY);
        }
    }
    else
    {
        /*
        ** No task is waiting, just free the WRP and ILT
        */
        pWRP = (WRP*)pILT->ilt_normal.w4;
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)pILT);
#endif /* M4_DEBUG_ILT */
        put_ilt(pILT);
        s_Free(pWRP, sizeof(WRP), __FILE__, __LINE__);
    }
}

#endif  /* FRONTEND -- i.e. else Backend  */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
