/* $Id: GR_ErrorHandle.c 155893 2011-05-19 18:16:01Z m4 $ */
/**
*******************************************************************************
**
** @file: GR_ErrorHandle.c
**
** @brief:
**          This file contain the functions related error handling component of
**          vdisk failover module.
**
**  Copyright (c) 2005-2010 XIOtech Corporation.  All rights reserved.
**
********************************************************************************
**/
#include "GR_Error.h"
#include "GR_AutoSwap.h"

#include "dcd.h"
#include "defbe.h"
#include "ecodes.h"
#include "ilt.h"
#include "nvram.h"
#include "pm.h"
#include "QU_Library.h"
#include "rrp.h"
#include "system.h"
#include "stdio.h"
#include "def_lun.h"
#include "lvm.h"
#include "scd.h"
#include "string.h"
#include "XIO_Types.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"
#include "mem_pool.h"
#include "CT_defines.h"
#include "ddr.h"
#include "misc.h"

/*
******************************************************************************
** Public defines - macros
******************************************************************************
*/
#define GR_ASWAPBACK_MAGIC_SAUSE 0x2335

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/
typedef void (*ErrorQueueFunc)(ILT *);

/*
******************************************************************************
** Private variables
******************************************************************************
*/
QU             gGrVdErrorQue;
ErrorQueueFunc GR_PostToVdiskErrorQueue;
static UINT8 gSpecialFlagsVdMap[(MAX_VIRTUAL_DISKS+7)/8];
static UINT8 gRemovedBayMap[(MAX_DISK_BAYS+7)/8];
UINT8  gAllDevMissAtOtherDCN = 0;

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
extern void CT_LC_GR_AutoSwapBackWaitTimer(int);
extern void CT_LC_GR_AutoSwapHysteresisTimer(int);
extern void CT_LC_GR_VdiskErrorHandlerTask(int);

static void GR_ProcessVdiskError (ILT* pILT);

void GR_AutoSwapHysteresisTimer  (UINT32, UINT32, VDD *pVDD);
void GR_AutoSwapBackWaitTimer    (UINT32, UINT32, ILT *pILT);
void GR_VdiskErrorHandlerTask    (void);
void GR_HandleSpecialMirrorFlags (UINT32, VRP *, ILT *);
extern void GR_ResumeUserPausedMirrors(VDD *pSrcVDD);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/*
******************************************************************************
**
**  @brief      This function initializes the Vdisk error handler. This is called
**              at virtual layer initialisation.
**
**  @param      None
**
**  @return     None
******************************************************************************
*/
void GR_InitErrorQueHandler(void)
{
    GR_PostToVdiskErrorQueue = GR_PostToVdiskErrorQueue_1;
}


/*
******************************************************************************
**
**  @brief      This function creates in ILT for errorneous VDD and queues
**              it to the vdisk error queue. The ILTs in the error queue
**              are processed by the vdisk error handling task.
**              THis function changes the operating state of Vdisk to auto
**              swap state, thus disabling the virtual layer executive from
**              further error submissions on the same VDD, till this error
**              is served.
**
**  @param      pVDD
**
**  @return     None
**
**  New ILT- is used for each VDD error. And it is alive till auto-swap
**           is finished. Following info. is used all along ILT path.
**
**       w0 --> Always contain VDD that is to be swapped (failed source)
**       w1 --> Always contain VDD that is to be swapped back.
**       w2 --> I/O Type- write-operation=2 read-operation=1
**                        unknown - 0
**       w3 --> Contain a magic number  to initiate autoswapback after hysteresis
**              wait time expiry.
**
**       w6 --> used by error handling component for internal auto-pause
**              states.
**
**  Assumption is that VDD and VRP are not NULL.
******************************************************************************
*/
void GR_SubmitVdiskError(VDD *pVDD, VRP* pVRP)
{
    UINT16 vrpFunc = pVRP->function;
    ILT   *pILT;
#if GR_MYDEBUG1
    fprintf(stderr, "<GR_Submit..>Entering...VID=%u\n", (UINT32)(pVDD->vid));
#endif
    if (vrpFunc >= RRP_BASE)
    {
        vrpFunc = vrpFunc-RRP_BASE;
    }

    /* Check whether autoswap is already submitted on this VDD. */
    if (!GR_IS_VD_ASWAP_INPROGRESS(pVDD))
    {
        BIT_CLEAR(vrpFunc, VRP_SPECIAL);

        /* Indicate that AUTOSWAP in initiated/in-progress */
        BIT_SET(pVDD->grInfo.tempFlags, GR_VD_ASWAP_PROGRESS_BIT);
        BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */

        /* Create an ILT -- with wait */
        pILT = get_ilt();
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)pILT);
#endif /* M4_DEBUG_ILT */

        pILT->ilt_normal.w0 = (UINT32)pVDD;
        if (VRP_OUTPUT == vrpFunc)
        {
            pILT->ilt_normal.w2 = (UINT32)GR_WRITE_REQ;
        }
        else
        {
            pILT->ilt_normal.w2 = (UINT32)GR_READ_REQ;
        }
        pILT->ilt_normal.w6 = (UINT32)GR_ASWAP_START;
#if GR_MYDEBUG1
        fprintf(stderr, "<GR_Submit..Calling GR_PostToVdiskErrorQueue\n");
#endif
        GR_PostToVdiskErrorQueue(pILT);
    }
}

/*
******************************************************************************
**
**  @brief     This is first level error queuing function that puts the vdisks
**             error requests (ILTs) in its executive queue.
**
**             This function puts the vdisk error ILT in its executive queue.
**             And creates the task for handling the ILTs puts in this queue.
**             This is  called only for once when vdisk error request is
**             posted for the first time. From next error request onwards the
**             the second level queuing function is called.
**
**  @param      pILT
**
**  @return     None
******************************************************************************
*/
void GR_PostToVdiskErrorQueue_1 (ILT* pILT)
{
    /*
    ** Create the Error Queue Task - Very first time
    ** Vdisk error occurred. So spawn the task.
    */
#if GR_MYDEBUG1
        fprintf(stderr, "<GR_PostToVdiskErrorQueue_1> Entering..\n");
#endif
    CT_fork_tmp = (unsigned long)"GR_VdiskErrorHandlerTask";
    gGrVdErrorQue.pcb = TaskCreate2(C_label_referenced_in_i960asm(GR_VdiskErrorHandlerTask),
                GR_VDISK_ERRORHANDLER_PRIORITY);

#if GR_MYDEBUG1
        fprintf(stderr, "<GR_PostToVdiskErrorQueue_1> Enqueuing the ILT\n");
        fprintf(stderr, "<GR_PostToVdiskErrorQueue_1> ILT = %x  Queue = %x\n", (UINT32)pILT, (UINT32)&gGrVdErrorQue);
#endif
    QU_EnqueReqILT(pILT, &gGrVdErrorQue);

    /*
    ** Create the Error Queue Task
    */
#if GR_MYDEBUG1
        fprintf(stderr, "<GR_PostToVdiskErrorQueue_1> Changing the handler\n");
#endif
    GR_PostToVdiskErrorQueue = GR_PostToVdiskErrorQueue_2;
}

/*
******************************************************************************
**
**  @brief     This is second level error queuing function that puts the vdisks
**             error requests (ILTs) in its executive queue.
**
**             This is  called  when the error handling task is already
**             running.
**
**  @param      pILT;
**
**  @return     None
******************************************************************************
*/
void GR_PostToVdiskErrorQueue_2 (ILT* pILT)
{
#if GR_MYDEBUG1
        fprintf(stderr, "<GR_PostToVdiskErrorQueue_2> Entering..\n");
#endif
    QU_EnqueReqILT(pILT, &gGrVdErrorQue);
#if GR_MYDEBUG1
        fprintf(stderr, "<GR_PostToVdiskErrorQueue_2> coming out....\n");
#endif
}

/*
******************************************************************************
**
**  @brief
**
**  @param      pILT;
**
**  @return     None
******************************************************************************
*/
NORETURN
void GR_VdiskErrorHandlerTask(void)
{
    ILT *pILT;

    /* Exchange the process */
    TaskSwitch();

    /* For all the vdisk error requests entered into the Queue */
    while (FOREVER)
    {
        /*
         * Get Next queued error request - This calls removes the
         * the request from the Queue
         */
        pILT = QU_DequeReqILT(&gGrVdErrorQue);

        if (NULL == pILT)
        {
            /* Queue is empty--set the process to not-ready state */
            QU_MakeExecProcessInactive(&gGrVdErrorQue);

            /* Exchange the processes */
            TaskSwitch();
            continue;
        }
#if GR_MYDEBUG1
        fprintf(stderr, "<GR_ErrorHandlerTask> got an ILT from QUEUE.\n");
#endif
        GR_ProcessVdiskError(pILT);
    }
}

/**
******************************************************************************
**
**  @brief      This is the internal function that processes the error
**              called from vdisk error handler. This function performs
**              the auto swap and auto-swapback state transition mechanism.
**
**  @param      pILT
**
**  @return     None
**
******************************************************************************
**/
static void GR_ProcessVdiskError(ILT *pILT)
{
    VDD   *pDestVDD;
    VDD   *pSrcVDD;
    UINT32 magicSause;

#if GR_MYDEBUG1
        fprintf(stderr, "<GR_ProcessVdiskError>With the state = %u\n",
                 (pILT->ilt_normal.w6));
#endif

    switch (pILT->ilt_normal.w6)
    {
        case GR_ASWAP_START:

            pSrcVDD = (VDD *)(pILT->ilt_normal.w0);

            /*
            ** This needs swap of raids--But before that, ensure  there is
            ** no I/O (any RRPs) pending on this source VDD or on its
            ** destinations.
            */
            if (GR_AnyIosArePending(pSrcVDD))
            {
                TaskSwitch();
                if (pSrcVDD->grInfo.vdOpState != GR_VD_INOP)
                {
                    pSrcVDD->grInfo.tempFlags = 0;
                    BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)pILT);
#endif /* M4_DEBUG_ILT */
                     put_ilt(pILT);
                     break;
                }
            }

#if GR_MYDEBUG1
          fprintf(stderr, "<ErrorHandlerTask>In...State GR_ASWAP_ASTART; srcvid=%u\n",
                    (UINT32)(pSrcVDD->vid));
#endif
            /*
             * Set the sync flag on the destinations which are currently in
             * sync with the failed source.
             */
            GR_SetSyncFlagOnMirrors(pSrcVDD);

            /* Now set operation type as autoswap. */
            GR_VD_SET_AUTOSWAP_OPTYPE(pSrcVDD, GR_VD_ASWAP_OP);
            BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */

            /* Perform autoswap */
#if GR_MYDEBUG1
            fprintf(stderr, "<GR_ProcessVdiskError> Calling GR_AutoSwap....\n");
#endif
            if (GR_AutoSwap(pSrcVDD, (UINT8)GR_VD_ASWAP_OP,
                  (UINT8)(pILT->ilt_normal.w2)) == ERROR)
            {
                pSrcVDD->grInfo.tempFlags = 0;
                BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
                GR_ClearSyncFlagOnMirrors(pSrcVDD);
            }
            else
            {
                /* Send the event to CCB */
                GR_SendAswapLogEvent(pSrcVDD, NULL, GR_ASWAP_START);
            }

            /*
            ** Release the ILT
            */
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)pILT);
#endif /* M4_DEBUG_ILT */
            put_ilt(pILT);
            break;

        case GR_ASWAP_SUCCESS:

            /* Auto swap was completed. */
            pSrcVDD  =  (VDD *)(pILT->ilt_normal.w0);
            pDestVDD = (VDD*)(pILT->ilt_normal.w1);

            GR_UpdateVdiskOpState(pSrcVDD, GR_VDOPSTATE_BY_RDDSTAT, 0);
            GR_UpdateVdiskOpState(pDestVDD, GR_VDOPSTATE_BY_RDDSTAT, 0);

            /* Send the event to CCB */
            GR_SendAswapLogEvent(pSrcVDD, pDestVDD, GR_ASWAP_SUCCESS);

#if GR_MYDEBUG1
            fprintf(stderr, "<ErrorHandlerTask>In...State GR_ASWAP_SUCCESS;srcvid=%x\n",
                    (UINT32)(pSrcVDD->vid));
#endif
            GR_ResumeUserPausedMirrors(pSrcVDD);
            GR_ClearSyncFlagOnMirrors (pSrcVDD);

            /*
             * Indicate that this source VDD is auto-swapped.
             * And the destination VDD is pending for auto swapback when it is recovered
             * and comes back to sync state.
             */
            BIT_SET(pSrcVDD->grInfo.permFlags, GR_VD_ASWAP_BIT);
            BIT_SET(pDestVDD->grInfo.permFlags, GR_VD_ASWAPBACK_PENDING_BIT);
            BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
            GR_SaveToNvram(pSrcVDD, pDestVDD);

            /* Reset the relevant bits.  */
            BIT_CLEAR(pSrcVDD->grInfo.tempFlags, GR_VD_ASWAP_PROGRESS_BIT);
            BIT_CLEAR(pSrcVDD->grInfo.tempFlags, GR_VD_AUTOSWAP_OPTYPE_BIT);

            /* Allow autoswap back only whne it enteres READY state */
            BIT_SET(pDestVDD->grInfo.tempFlags, GR_VD_ASWAPBACK_NOT_READY_BIT);
            BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */

            if (TRUE == gAllDevMissAtOtherDCN)
            {
                GR_SetSpecialAswapFlag(pSrcVDD);
            }

            if (GR_IS_VD_HYSTERESIS_ENABLED(pSrcVDD))
            {
                /*
                ** Second aswap occurred within 10minutes of its aswapback.
                ** Don't do next aswapback atleast for 1 hour.
                ** Also clear the hysteresis bit.
                */
                BIT_CLEAR(pSrcVDD->grInfo.tempFlags, GR_VD_HYSTERESIS_BIT);
                BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */

                /*
                ** Fork autoswap back wait timer that waits for 1 hour and then
                ** make the auto swapback process ready.
                */
                CT_fork_tmp = (unsigned long)"GR_AutoSwapBackWaitTimer";
                TaskCreate3(C_label_referenced_in_i960asm(GR_AutoSwapBackWaitTimer),
                            GR_AUTOSWAPBACK_WAITTIMER_PRIORITY,
                            (UINT32)pILT);
                /*
                ** This state is intermediate and is not handled in this function. Once
                ** hysteresis wait timer is over, this state is changed to READY and
                ** posted to this function by wait timer task.
                */
                pILT->ilt_normal.w6 = (UINT32)GR_ASWAPBACK_HYSTERESIS_WAIT;
                GR_SendAswapLogEvent(pSrcVDD, pDestVDD, GR_ASWAPBACK_HYSTERESIS_WAIT);
            }
            else /* hysteresis bit is not set on src VDD */
            {
                /*
                ** This is either first swap or next swap occurs
                ** after 10 minutes
                ** pILT->ilt_normal.w1 already contain on to which swap happens
                ** from failed source i.e. pILT->ilt_normal.w0;
                */
                 pILT->ilt_normal.w6 = GR_ASWAPBACK_READY;
                 /*
                 ** Repost the same ILT.
                 */
                 GR_PostToVdiskErrorQueue(pILT);
            }
            break;

        case GR_ASWAP_FAILED:
            /*
             * Clear all the bits in VDD flags.
             * Or clear only the required bits.
             */
            pSrcVDD = (VDD*)(pILT->ilt_normal.w0);
            pSrcVDD->grInfo.tempFlags = 0;
            BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
            GR_ClearSyncFlagOnMirrors(pSrcVDD);
#if GR_MYDEBUG1
            fprintf(stderr, "<ErrorHandlerTask>In...State GR_ASWAP_FAILED;srcvid=%u\n",
                    (UINT32)(pSrcVDD->vid));
#endif
            /* Send the event to CCB */
            GR_SendAswapLogEvent(pSrcVDD, NULL, GR_ASWAP_FAILED);

            /* Done with ILT. Release It. */
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)pILT);
#endif /* M4_DEBUG_ILT */
            put_ilt(pILT);
            break;

        case GR_ASWAPBACK_READY:
            /*
             * Now Autoswap back can be enabled on this VDD
             * Set wait-sync bit in the VDD that is to be swapped back
             * This enables the copy/manager to let the error handling
             * component know that the VDD is restored and came back
             * to sync, so that it can schedule the swapback operation.
             */
            pDestVDD = (VDD *)(pILT->ilt_normal.w1);

            /*
             * Now you can allow autoswap back, after failed VDD is recoverd
             * and comes back to sync state.
             */
            BIT_CLEAR(pDestVDD->grInfo.tempFlags, GR_VD_ASWAPBACK_NOT_READY_BIT);
            BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */

            /* Send the event to CCB */
            pSrcVDD = (VDD *)(pILT->ilt_normal.w0);
            GR_SendAswapLogEvent(pSrcVDD, pDestVDD, GR_ASWAPBACK_RECOVERY_WAIT);
#if GR_MYDEBUG1
            fprintf(stderr, "<ErrorHandlerTask>In...State GR_ASWAPBACK_READY;srcvid=%u\n",
                    (UINT32)(pSrcVDD->vid));
#endif

            magicSause = pILT->ilt_normal.w3;
            pILT->ilt_normal.w3 = 0;

            /*
            ** Since auto-swap is done and auto-swap back is scheduled,
            ** we can release this ILT. When the failed VDD comes back
            ** to sync, the copy/manager inform  us, then we schedule
            ** the auto-swapback through another ILT. Copy/Manager calls
            ** GR_InformVdiskSyncState()
            */
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)pILT);
#endif /* M4_DEBUG_ILT */
            put_ilt(pILT);

            /*
            ** If the aswapback ready enteres from hysterisis wait, just try out
            ** autoswapback here.
            */
            if (magicSause == GR_ASWAPBACK_MAGIC_SAUSE)
            {
                GR_UpdateVdiskOpState(pDestVDD, GR_VDOPSTATE_BY_RDDSTAT, 0);
            }

            break;

        case GR_ASWAPBACK_START:
            pDestVDD = (VDD *)(pILT->ilt_normal.w1);
            pSrcVDD  = (VDD *)(pILT->ilt_normal.w0);

#if GR_MYDEBUG1
            fprintf(stderr, "<ErrorHandlerTask>In...State GR_ASWAPBACK_START;srcvid=%u\n",
                    (UINT32)(pSrcVDD->vid));
#endif
            GR_VD_SET_AUTOSWAP_OPTYPE(pSrcVDD, GR_VD_ASWAPBACK_OP);
            BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */

            /* Perform autoswapback */
            GR_AutoSwapBack(pSrcVDD, pDestVDD);

            /* Send the event to CCB */
            GR_SendAswapLogEvent(pSrcVDD, pDestVDD, GR_ASWAPBACK_START);

            /* Release the ILT */
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)pILT);
#endif /* M4_DEBUG_ILT */
            put_ilt(pILT);

            break;

        case GR_ASWAPBACK_SUCCESS:

            /*
            ** Auto swap was completed- Obtain source(recovered raids) and
            ** destination VDD from the ILT.
            */
            pSrcVDD =  (VDD *)(pILT->ilt_normal.w0);  /* source VDD */
            pDestVDD = (VDD *)(pILT->ilt_normal.w1); /* dest  VDD */
            GR_UpdateVdiskOpState(pSrcVDD, GR_VDOPSTATE_BY_RDDSTAT, 0);
            GR_UpdateVdiskOpState(pDestVDD, GR_VDOPSTATE_BY_RDDSTAT, 0);

#if GR_MYDEBUG1
            fprintf(stderr, "<ErrorHandlerTask>In...State GR_ASWAPBACK_SUCCESS;srcvid=%u\n",
                    (UINT32)(pSrcVDD->vid));
#endif
            /* Send Log event to CCB */
            GR_SendAswapLogEvent(pSrcVDD, pDestVDD, GR_ASWAPBACK_SUCCESS);

            /*
            ** Disable further aswapback trails by clearing the sync state
            ** wait bit on destination  Vdisk.
            ** Also clear the aswap bit on the source, indicating that it
            ** is not aswap state, but came back to original state
            ** and save in NVRAM
            */
            BIT_CLEAR(pDestVDD->grInfo.permFlags, GR_VD_ASWAPBACK_PENDING_BIT);
            BIT_CLEAR(pSrcVDD->grInfo.permFlags, GR_VD_ASWAP_BIT);
            BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
            GR_SaveToNvram(pSrcVDD, pDestVDD);

            /* Clear any temp flags. */
            pSrcVDD->grInfo.tempFlags = 0;
            pDestVDD->grInfo.tempFlags = 0;
            BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */

            if (GR_IsSpecialAswapFlag(pSrcVDD) == FALSE)
            {
                /*
                ** Set hysteresis bit on this source PDD
                */
                BIT_SET(pSrcVDD->grInfo.tempFlags, GR_VD_HYSTERESIS_BIT);

                /*
                ** Enable hysteresis timer on the auto swap backed VDD-
                ** This is to track whether the next autoswap occurs on this
                ** VDD within 10 minutes from now.
                */
                CT_fork_tmp = (unsigned long)"GR_AutoSwapHysteresisTimer";
                TaskCreate3(
                  C_label_referenced_in_i960asm(GR_AutoSwapHysteresisTimer),
                  GR_AUTOSWAPBACK_HYSTERESISTIMER_PRIORITY,
                  (UINT32)pSrcVDD);
            }
            else
            {
                GR_ClearSpecialAswapFlag(pSrcVDD);
            }

#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)pILT);
#endif /* M4_DEBUG_ILT */
            put_ilt(pILT);
            break;

        case GR_ASWAPBACK_FAILED:
            /* Auto swapback failed */
            pSrcVDD = (VDD *)(pILT->ilt_normal.w0);  /* source VDD */
            pDestVDD = (VDD *)(pILT->ilt_normal.w1);  /* Dest VDD */
#if GR_MYDEBUG1
            fprintf(stderr, "<ErrorHandlerTask>In...State GR_ASWAPBACK_FAILED;srcvid=%u\n",
                    (UINT32)(pSrcVDD->vid));
#endif

            /* Send Log event to CCB */
            GR_SendAswapLogEvent(pSrcVDD, pDestVDD, GR_ASWAPBACK_FAILED);

            /* Clear out everything on source VDD */
            pSrcVDD->grInfo.tempFlags = 0;
            pDestVDD->grInfo.tempFlags = 0;
            BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)pILT);
#endif /* M4_DEBUG_ILT */
            put_ilt(pILT);
            break;

        default:
            break;

    } /* switch(ILT->ilt_normal.w6) **/
}

/*
******************************************************************************
**
**  @brief    This is the completer function gets called by the auto-swap
**            component after finishing auto-swap or auto-swap back opera-
**            tion.
**
**  @param    pSrcVDD  -- Pointer to source vdisk.
**            pDestVDD -- Pointer to destination vdisk.
**            status   -- Status of the autoswap/autoswapback op.
**                        (success or failure)
**
**  @return     None
******************************************************************************
*/
void GR_VdiskErrorCompleter(VDD *pSrcVDD, VDD *pDestVDD, UINT8 status)
{
    ILT *pILT;

    /* Create an ILT -- with wait */
    pILT = get_ilt();
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)pILT);
#endif /* M4_DEBUG_ILT */
    pILT->ilt_normal.w0 = (UINT32)pSrcVDD;
    pILT->ilt_normal.w1 = (UINT32)pDestVDD;
#if GR_MYDEBUG1
    fprintf(stderr, "<GR_VdiskErrorCompleter>Entering...\n");
#endif

    /*
    ** Check the auto swap operation type (swap or swap back)
    ** And the change the state accordingly.
    */
    if (GR_VD_ASWAP_OPTYPE(pSrcVDD) == GR_VD_ASWAP_OP)
    {
        if (GR_ASWAP_SUCCESS == status)
        {
            pILT->ilt_normal.w6 = (UINT32)GR_ASWAP_SUCCESS;
        }
        else
        {
            pILT->ilt_normal.w6 = (UINT32)GR_ASWAP_FAILED;
        }
    }
    else
    {
        if (GR_ASWAP_SUCCESS == status)
        {
            pILT->ilt_normal.w6 = (UINT32)GR_ASWAPBACK_SUCCESS;
        }
        else
        {
            pILT->ilt_normal.w6 = (UINT32)GR_ASWAPBACK_FAILED;
        }
    }
    /*
    ** Repost this to error queue handler
    */
    GR_PostToVdiskErrorQueue(pILT);
}
/*
******************************************************************************
**
**  @brief      This timer task is used to check whether a second autoswap
**              occurs within 10 minutes of its swapback. This is spwaned
**              once auto swapback is done for a auto swapped vdisk, after
**              setting a hysteresis bit on the VDD.
**
**              If 10 minutes is elapsed, the hysteresis that is set after
**              one iteration of auto-swap/swapback, is cleared.
**
**  @param      pVDD
**
**  @return     None
******************************************************************************
*/
void GR_AutoSwapHysteresisTimer (UINT32 pPCB UNUSED,
                                 UINT32 pri UNUSED, VDD *pVDD)
{
#if GR_MYDEBUG1
    fprintf(stderr, "<GR_AutoSwapHysteresisTimer>In...\n");
#endif
    //TaskSleepMS(10*60*1000);  -- LAKELAND
    if (pVDD != NULL)
    {
        BIT_CLEAR(pVDD->grInfo.tempFlags, GR_VD_HYSTERESIS_BIT);
        BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
    }
#if GR_MYDEBUG1
    fprintf(stderr, "<GR_AutoSwapHysteresisTimer>out...\n");
#endif
}

/*
******************************************************************************
**
**  @brief      This timer task waits for one hour and after that enables
**              the auto swap back scheduling.
**
**  @param      pILT;
**
**  @return     None
******************************************************************************
*/
void GR_AutoSwapBackWaitTimer (UINT32 pPCB UNUSED,
                        UINT32 pri UNUSED, ILT *pILT)
{
    VDD *pDestVDD;

    pDestVDD = (VDD*)(pILT->ilt_normal.w1);
#if GR_MYDEBUG1
    fprintf(stderr, "<GR_AutoSwapBackWaitTimer>In..vid=%u.\n", (UINT32)(pDestVDD->vid));
#endif

    /* Wait for one hour. */
    TaskSleepMS(60*60*1000);

    /*
     * Ensure that swapback bit is still ON. User might have cancelled by giving a
     * command like break during this wait time.
     */
    if (GR_IS_VD_ASWAPBACK_PENDING(pDestVDD))
    {
        pILT->ilt_normal.w6 = (UINT32)GR_ASWAPBACK_READY;
        pILT->ilt_normal.w3 = (UINT32)GR_ASWAPBACK_MAGIC_SAUSE;
    }
    else
    {
        /* User cancelled autoswap back. */
        pILT->ilt_normal.w6 = (UINT32)GR_ASWAPBACK_FAILED;
    }

    /* Repost this to error queue handler */
#if GR_MYDEBUG1
    fprintf(stderr, "<GR_AutoSwapBackWaitTimer>out...\n");
#endif
    GR_PostToVdiskErrorQueue(pILT);
}



void GR_InformVdiskSyncState (VDD *pSrcVDD, VDD *pDestVDD)
{
#if GR_MYDEBUG1
    fprintf(stderr, "<GR_InformVdiskSyncState> svid=%x dvid=%x s-opstate=%x d-opstate=%x d-permflags=%x d-tempflags=%x\n",
             (UINT32)(pSrcVDD->vid), (UINT32)(pDestVDD->vid), pSrcVDD->grInfo.vdOpState,
             pDestVDD->grInfo.vdOpState, (UINT32)pDestVDD->grInfo.permFlags,
             (UINT32)pDestVDD->grInfo.tempFlags);

#endif
    if (GR_IS_VD_ASWAPBACK_PENDING(pDestVDD) &&
        GR_IS_VD_ASWAPBACK_IN_READY_STATE(pDestVDD))
    {
#if GR_MYDEBUG1
      fprintf(stderr, "<GR_InformVdiskSyncState>initiate error recovery for vid=%x\n", (UINT32)(pDestVDD->vid));
#endif
        GR_InitiateErrorRecovery(pSrcVDD, pDestVDD);
    }
}

void GR_InformVdiskOpState(VDD *pDestVDD)
{
    VDD *pSrcVDD = NULL;
    COR *pCOR;

#if GR_MYDEBUG1
//     fprintf(stderr, "<GR_InformVdiskOpState> dvid=%x d-opstate=%x d-permflags=%x d-tempflags=%x\n",
//             (UINT32)(pDestVDD->vid), pDestVDD->grInfo.vdOpState, (UINT32)pDestVDD->grInfo.permFlags,
//             (UINT32)pDestVDD->grInfo.tempFlags));
#endif  /* GR_MYDEBUG1 */

    /* Proceed only if the VDD is  a destination copy device and the Copy registration exists */
    if ((DCD *)(pDestVDD->pDCD)== NULL)
    {
        return;
    }
    pCOR = ((DCD *)(pDestVDD->pDCD))->cor;
    if (pCOR == NULL)
    {
       return;
    }

    /* ASWAPBACK pending bit is always set on destination VDDs only. */
     if (GR_IS_VD_ASWAPBACK_PENDING(pDestVDD) &&
         GR_IS_VD_ASWAPBACK_IN_READY_STATE(pDestVDD))
    {

        /*
        ** Make sure the Copy is owned by this controller
        */
        if (K_ficb->cSerial == pCOR->powner)
    {
#if GR_MYDEBUG1
           fprintf(stderr, "<GR_InformVdiskOpState>aswapback bit set-dvid=%x-COR  exists-copy owner is this controller\n",
              pDestVDD->vid);
#endif
            pSrcVDD = pCOR->srcvdd;

            if ((pCOR->copystate == CST_MIRROR) && (pCOR->crstate == CRST_ACTIVE))
            {
#if  GR_MYDEBUG1
      fprintf(stderr, "<GR_InformVdiskOpState>initiate error recovery for vid=%x\n", (UINT32)(pDestVDD->vid));
#endif
                       GR_InitiateErrorRecovery(pSrcVDD, pDestVDD);
            }
            else
            {
#if GR_MYDEBUG1
      fprintf(stderr, "<GR_InformVdiskOpState>postpone error recovery for vid=%x\n", (UINT32)(pDestVDD->vid));
#endif
            }
        }
    }
}
/*
******************************************************************************
**
*** @brief      This is the function provided to copy/manager to let us know
**              that the destination vdisk  has come to sync state with its
**              source VDD. For all source-destination SYNC pairs, this function
**              gets called.
**
**  @param      pSrcVDD
**              pDestVDD
**
**  @return     None
******************************************************************************
*/
void GR_InitiateErrorRecovery (VDD *pSrcVDD, VDD *pDestVDD)
{
    ILT *pILT;

#if GR_MYDEBUG1
    fprintf(stderr, "<GR_InitiateErrorRecovery> Dest(%u) comes to op..its source is %u\n",
             (UINT32)(pDestVDD->vid), (UINT32)(pSrcVDD->vid));
#endif

    /*
    ** Check whether the destination vdd is the one that is to be auto-
    ** swapped back. This bit is set on destination after autoswap is
    ** happened from its failed source VDD and after hysterisis timer
    ** elpase.
    */
    /*
    ** Check if autoswap back is already submitted on this VDD. In that
    ** avoid multiple sumbission.
    */
    if (!GR_IS_VD_ASWAPBACK_INPROGRESS(pSrcVDD))
    {
#if GR_MYDEBUG1
    fprintf(stderr, "<GR_InitiateErrorRecovery> Dest(%u) - source %u queuing for swapback\n",
             (UINT32)(pDestVDD->vid), (UINT32)(pSrcVDD->vid));
#endif
        /* Indicate that AUTOSWAP in initiated/in-progress */
        BIT_SET(pSrcVDD->grInfo.tempFlags, GR_VD_ASWAPBACK_PROGRESS_BIT);
        BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */

        /* Create a new ILT and set parameters.  */
        pILT = get_ilt();
#ifdef M4_DEBUG_ILT
CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, (unsigned long)pILT);
#endif /* M4_DEBUG_ILT */
        pILT->ilt_normal.w2 = (UINT32)GR_UNKNOWN_REQ;
        pILT->ilt_normal.w6 = (UINT32)GR_ASWAPBACK_START;
        pILT->ilt_normal.w0 = (UINT32)pSrcVDD;
        pILT->ilt_normal.w1 = (UINT32)pDestVDD;

        /* Post this ILT to vdisk error queue. */
        GR_PostToVdiskErrorQueue(pILT);
    }
#if GR_MYDEBUG1
    else
    {
        fprintf(stderr, "<GR_InitiateErrorRecovery> Dest(%u) sre - %u- already submitted\n",
             (UINT32)(pDestVDD->vid), (UINT32)(pSrcVDD->vid));
    }
#endif
}

/*
******************************************************************************
**
**  @brief    This  function  attempts to perform any  pending  autoswap back
**            operations on the vdisks which have already auto swapped.
**
**            This gets called during phase II initialization stage to enable
**            to  perform  autoswap back  operations that  were not completed
**            when the system last went down.
**
**  @param    None
**
**  @return   None
******************************************************************************
*/
void GR_RetryPendingFailBacks(void)
{
    UINT16 myIndex;
    VDD   *pVDD;

    for (myIndex = 0; myIndex < MAX_VIRTUAL_DISKS; myIndex++)
    {
        pVDD = gVDX.vdd[myIndex];

        if (pVDD != NULL)
        {
            if (pVDD->grInfo.vdOpState == GR_VD_OP)
            {
#if GR_MYDEBUG1
      fprintf(stderr, "<GR_RetryPendingFailBacks>initiate error recovery dest vid=%u\n",
                     (UINT32)(pVDD->vid));
#endif
                GR_InformVdiskOpState(pVDD);
            }
        }
    }
}

/*
******************************************************************************
**
**  @brief   This function clears the local image bit set on the raids owned
**           by this controller. This bit is normally set during local nvram
**           image processing. This bit is set to disable any write operation
**           on the raid as long as the local image is seen by the other DCNs.
**
**           This function gets called from rebuild processing when all the
**           storage is vanished away from this DCN  and DCN is going to be
**           killed soon. Since this DCN is anyway dying it is not necessary
**           to keep this bit still set (thats gets stored in nvram)which may
**           unwantedly disallow the writes on these raids after takeover by
**           by the other DCN.
**
**  @param   None
**
**  @return  None
******************************************************************************
*/
void GR_ClearLocalImageIP(void)
{
    RDD   *pRDD = NULL;            /* RDD being Processed at the time  */
    bool   configChange = false;   /* Loop Continue flag               */
    UINT16 rid = 0;                /* RAID ID                          */
    VDD   *pVDD = NULL;

    configChange = false;

    for (rid = 0; rid < MAX_RAIDS; ++rid)
    {
        pRDD = R_rddindx[rid];

        if (pRDD)
        {
            pVDD = gVDX.vdd[pRDD->vid];
            if (pVDD && (DL_ExtractDCN(K_ficb->cSerial) == pVDD->owner) )
            {
                if (BIT_TEST(pRDD->aStatus, RD_A_LOCAL_IMAGE_IP))
                {
                    /*
                     * Found a RAID that has the Local Image in Progress bit still
                     * on.  Clear the bit and note config change.
                     */
                    BIT_CLEAR(pRDD->aStatus, RD_A_LOCAL_IMAGE_IP);
                    configChange = true;
                }
           }
        }
    }

    if (configChange)
    {
        BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        NV_P2Update();
    }
}

#define GR_DUMP_CPMIRROR_INFO_1(msg) \
       fprintf(stderr, "<GR>[%s]src(%02x %02x %02x %02x)-dest(%02x %02x %02x %02x)-COR(%02x %02x %02x)\n",\
       msg, pSrcVDD->vid, pSrcVDD->attr, pSrcVDD->grInfo.permFlags, pSrcVDD->grInfo.vdOpState, \
       pDestVDD->vid, pDestVDD->attr, pDestVDD->grInfo.permFlags, pDestVDD->grInfo.vdOpState, \
       pCOR->copystate, pCOR->crstate, pCOR->mirrorstate)

#define GR_DUMP_CPMIRROR_INFO_2(msg) \
       fprintf(stderr, "<GR>%s(%02x %02x %02x %02x)\n", \
       msg, pVDD->vid, pVDD->attr, pVDD->grInfo.permFlags, pVDD->grInfo.vdOpState)

/*
******************************************************************************
**
**  @brief    This function checks for all the mirror vdisks (that are in sync
**            state  with their  corresponding  source vdisks) and  stores the
**            source and destination vdisks numbers in the corresponding vdisk
**            maps.
**
**            This function is called when all BE  storage is inaccessble from
**            this DCN. The rebuild process  while sending the all-dev-missing
**            log message  to CCB, captures  the  source-mirror vdisk list and
**            passes it to the CCB for further processing.
**
**  @param    pLogData -- Pointer to log message area
**
**  @return   None
******************************************************************************
*/
void GR_SetVdiskInfoAtAllDevMiss(LOG_ALL_DEV_MISSING_DAT *pLogData)
{
    UINT16  i;
    LVM*    lvm;
    SDD*    sdd;
    UINT8   owner;
    VDD    *pSrcVDD;
    VDD    *pDestVDD;
    COR    *pCOR;
    SCD    *pSCD;
    UINT8   copyExists;

    memset(pLogData, 0x0, sizeof(LOG_ALL_DEV_MISSING_DAT));

    pLogData->cSerial = K_ficb->cSerial;
    fprintf(stderr, "<GR>Proc-Collecting copy/mirror info from ctrl =%x\n", pLogData->cSerial);

    for (i = 0; i < MAX_SERVERS; ++i)
    {
        if (NULL != (sdd = gSDX.sdd[i]))
        {
            /*
            ** Check both the visible and invisible mappings.
            */
            for (lvm = sdd->lvm; lvm != NULL; lvm = lvm->nlvm)
            {
                owner = DL_ExtractDCN(T_tgdindx[sdd->tid]->owner);

                /*
                ** Get the source vdisks associated with this server
                ** and owned by this controller at the time of failure.
                */
                if (DL_ExtractDCN(K_ficb->cSerial) == owner)
                {
                    pSrcVDD = gVDX.vdd[lvm->vid];
                    if (pSrcVDD && GR_IS_GEORAID_VDISK(pSrcVDD))
                    {
                        copyExists = FALSE;
                        for (pSCD = pSrcVDD->pSCDHead; pSCD != NULL; pSCD = pSCD->link)
                        {
                            pCOR    = pSCD->cor;
                            pDestVDD = pCOR->destvdd;

                            if (pDestVDD && GR_IS_GEORAID_VDISK(pDestVDD) )
                            {
                                if (pCOR->copystate == CST_MIRROR && pCOR->crstate == CRST_ACTIVE)
                                {
                                    /*
                                    ** Mark the destination VDD is in sync with its source associated to a server
                                    ** at the time of BEnd  failure.
                                    */
                                    BIT_SET(pLogData->syncVdMap[(pDestVDD->vid)/8], (pDestVDD->vid)%8);
                                    copyExists = TRUE;
                                    GR_DUMP_CPMIRROR_INFO_1("Mirror/Active");
                                }
                                else
                                {
                                    if (pCOR->copystate == CST_MIRROR && pCOR->crstate == CRST_AUTOSUSP)
                                    {
                                        /*
                                        ** The copy is set to auto suspended state. But the destination VDD
                                        ** might be in sync state with its source. So check the StillInSync
                                        ** special flag that determines the exact sync status of the mirror.
                                        */
                                        if (GR_IsStillInSyncFlag(pDestVDD))
                                        {
                                            /*
                                            ** The destination is still in sync, though it is moved autopause
                                            ** state. Means no additional I/O happened on this source device
                                            ** after being moved to auto-pause state.
                                            */
                                            copyExists = TRUE;
                                            BIT_SET(pLogData->syncVdMap[(pDestVDD->vid)/8], (pDestVDD->vid)%8);
                                            GR_DUMP_CPMIRROR_INFO_1("Mirror/Autosusp-InSync");
                                        }
                                        else
                                        {
                                            GR_DUMP_CPMIRROR_INFO_1("Mirror/Autosusp-notInSync");
                                        }
                                    }
                                    else
                                    {
                                          GR_DUMP_CPMIRROR_INFO_1("Out-of-sync");
                                    }
                                }
                            }
                            else if (pDestVDD)
                            {
                                GR_DUMP_CPMIRROR_INFO_1("non-Geo-Raid");
                            }
                        }
                        if (copyExists)
                        {
                            BIT_SET(pLogData->srcVdMap[(pSrcVDD->vid)/8], (pSrcVDD->vid)%8);

                            /*
                            ** Don't allow the I/O in  the DCN that is going to be killed
                            ** due to loss of entire storage access.Instead I/O should be
                            ** completed with RETRY. Set the operating state of souce VDD
                            ** to IO_SUSPEND so as to enable the V$exec process to return
                            ** the I/O request with RETRY.
                            */
                            GR_SetIOSuspendState(pSrcVDD);
                        }
                    }
                }
            }
        }
    }
#if 1
    {
        UINT8* pSyncVdMap;
        UINT8* pSrcVdMap;

        pSyncVdMap = pLogData->syncVdMap;
        pSrcVdMap  = pLogData->srcVdMap;
        fprintf(stderr, "<GR>SetVdiskInfoAtAllDevMiss - DEST MAP>>>\n");
        for (i=0;i<64;(i=i+16))
        {
            fprintf(stderr, "    %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                        pSyncVdMap[i], pSyncVdMap[i+1], pSyncVdMap[i+2], pSyncVdMap[i+3], pSyncVdMap[i+4],
                        pSyncVdMap[i+5], pSyncVdMap[i+6], pSyncVdMap[i+7], pSyncVdMap[i+8],
                        pSyncVdMap[i+9], pSyncVdMap[i+10], pSyncVdMap[i+11], pSyncVdMap[i+12], pSyncVdMap[i+13],
                        pSyncVdMap[i+14], pSyncVdMap[i+15]);
        }
        fprintf(stderr, "<GR>SetVdiskInfoAtAllDevMiss - SRC MAP>>>\n");
        for (i=0;i<64;(i=i+16))
        {
           fprintf(stderr, "    %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                        pSrcVdMap[i], pSrcVdMap[i+1], pSrcVdMap[i+2], pSrcVdMap[i+3], pSrcVdMap[i+4],
                        pSrcVdMap[i+5], pSrcVdMap[i+6], pSrcVdMap[i+7], pSrcVdMap[i+8],
                        pSrcVdMap[i+9], pSrcVdMap[i+10], pSrcVdMap[i+11], pSrcVdMap[i+12], pSrcVdMap[i+13],
                        pSrcVdMap[i+14], pSrcVdMap[i+15]);
        }
    }
#endif
}

/*
******************************************************************************
**
**  @brief    This function processes the source-mirror vdisk information sent
**            by CCB  of surviving controller when other DCN is faulted due to
**            loss of  entier BE access.  The  vdisk  maps indicate the source
**            and  destination  vdisks  which  are in  sync at the time of DCN
**            failure at other site.
**
**            It processes both the source and destination  vdisk maps separa-
**            tively. This  puts those source  vdisks in  IOSUSPEND state  and
**            sets the sync flag in the corresponding destination vdisks so as
**            to disallow any I/O  during  the copy-ownwership  transition and
**            copy/manager special processing mechanism.
**
**  @param    pMRP -- Pointer to source-mirror vdisk information.
**
**  @return   None
******************************************************************************
*/
UINT32 GR_HandleAllDevMissAtOtherDCN(MR_PKT *pMRP)
{
    UINT32   status = DEOK;
    UINT8   *pSyncVdMap;
    UINT8   *pSrcVdMap;
    UINT16   i;
    VDD     *pVDD;

    pSyncVdMap = ((MRALLDEVMISS_REQ*)(pMRP->pReq))->syncVdMap;
    pSrcVdMap = ((MRALLDEVMISS_REQ*)(pMRP->pReq))->srcVdMap;

    gAllDevMissAtOtherDCN = TRUE;

    fprintf(stderr, "<GR>Proc- vdisk info collected during BE loss at other DCN\n");

    for (i = 0; i < MAX_VIRTUAL_DISKS; i++)
    {
        pVDD = gVDX.vdd[i];
        if (pVDD)
        {
            if (BIT_TEST(pSyncVdMap[(pVDD->vid)/8], (pVDD->vid)%8) == TRUE)
            {
                GR_SetAllDevMissSyncFlag(pVDD);
#if 0
                GR_DUMP_CPMIRROR_INFO_2("DEST>>");
#endif
            }
            else if (BIT_TEST(pSrcVdMap[(pVDD->vid)/8], (pVDD->vid)%8) == TRUE)
            {
                GR_SetIOSuspendState(pVDD);
#if 0
                GR_DUMP_CPMIRROR_INFO_2("SRC>>");
#endif
            }
        }
    }

    fprintf(stderr, "<GR>HandleAllDevMissAtOtherDCN - DEST MAP>>>\n");
    for (i=0;i<64;(i=i+16))
    {
        fprintf(stderr, "    %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                        pSyncVdMap[i], pSyncVdMap[i+1], pSyncVdMap[i+2], pSyncVdMap[i+3], pSyncVdMap[i+4],
                        pSyncVdMap[i+5], pSyncVdMap[i+6], pSyncVdMap[i+7], pSyncVdMap[i+8],
                        pSyncVdMap[i+9], pSyncVdMap[i+10], pSyncVdMap[i+11], pSyncVdMap[i+12], pSyncVdMap[i+13],
                        pSyncVdMap[i+14], pSyncVdMap[i+15]);
    }
    fprintf(stderr, "<GR>HandleAllDevMissAtOtherDCN - SRC MAP>>>\n");
    for (i=0;i<64;(i=i+16))
    {
        fprintf(stderr, "    %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                        pSrcVdMap[i], pSrcVdMap[i+1], pSrcVdMap[i+2], pSrcVdMap[i+3], pSrcVdMap[i+4],
                        pSrcVdMap[i+5], pSrcVdMap[i+6], pSrcVdMap[i+7], pSrcVdMap[i+8],
                        pSrcVdMap[i+9], pSrcVdMap[i+10], pSrcVdMap[i+11], pSrcVdMap[i+12], pSrcVdMap[i+13],
                        pSrcVdMap[i+14], pSrcVdMap[i+15]);
    }
    return(status);
}

/*
******************************************************************************
**
**  @brief    This  function  sets the  special  bit "still in sync"  in  the
**            specified  vdisk (destination)  position of the special vd map.
**            This is  used to identify  the whether the  destination in sync
**            or  not, even after  it moves  into  autopause state due to the
**            problem in accessing destination vdd.
**
**            During I/O, when error occured  on destination VDD, this bit is
**            set for enabling the I/O completer function to identify whether
**            the destination is really gone out of sync.
**
**  @param    PVDD -- Pointer to destination vdisk.
**
**  @return   None
******************************************************************************
*/
void GR_SetStillInSyncFlag (VDD *pVDD)
{
    BIT_SET(gSpecialFlagsVdMap[(pVDD->vid)/8], (pVDD->vid)%8);
}

/*
******************************************************************************
**
**  @brief    This  function clears  the  special flag "still in sync" in the
**            specified  vdisk (destination) position  of  the special vd map
**            when it is really gone out of sync.
**
**            The  I/O completion  function  (link layer) checks  whether the
**            "still in sync"  flag is set for the destination VDD. If so, it
**            verifies whether  the I/O is  succeeded on  the source.  If the
**            server write (source) is succeded, it clears this flag.It means
**            the  source has been  written  one extra write  than the mirror
**            vdisk. This facilitates the GeoRaid module to  know whether the
**            specified destination vdisk is really gone out of sync with its
**            source vdisk.
**
**  @param    pVDD -- Pointer to destination vdisk.
**
**  @return   None
******************************************************************************
*/
void GR_ClearStillInSyncFlag (VDD *pVDD)
{
    BIT_CLEAR(gSpecialFlagsVdMap[(pVDD->vid)/8], (pVDD->vid)%8);
}

/*
******************************************************************************
**
**  @brief    This function tells whether the specified destination vdisk is
**            is really in sync with its  source vdisk, when the destination
**            is just failed.
**
**            This  function gets called when all the BE devices are lost by
**            the GeoRaid "all  device missing  handling" logic. The GeoRaid
**            logic while  computing the source/destination vdisk  map (that
**            are in sync) calls this function to  check whether the destina-
**            tion is really gone out of sync(even though it is in autopause
**            state).
**
**  @param    pVDD -- Pointer to destination vdisk.
**
**  @return   TRUE or FLASE
******************************************************************************
*/
UINT32 GR_IsStillInSyncFlag(VDD *pVDD)
{
    return(BIT_TEST(gSpecialFlagsVdMap[(pVDD->vid)/8], (pVDD->vid)%8));
}

/*
******************************************************************************
**
**  @brief    This  function  sets  the  special  bit "special aswap"  in  the
**            specified  vdisk (source)  position of the special vd map.
**
**            This is  used to identify  the  ASWAP that happened in a special
**            failure case like failure of DCN (at other site) due to loss  of
**            entire BE  access. This  gets called when  ASWAP is successfully
**            completed.
**
**  @param    PVDD -- Pointer to source vdisk.
**
**  @return   None
******************************************************************************
*/
void GR_SetSpecialAswapFlag(VDD *pVDD)
{
    BIT_SET(gSpecialFlagsVdMap[(pVDD->vid)/8], (pVDD->vid)%8);
}

/*
******************************************************************************
**
**  @brief    This  function clears  the  special flag "special aswap" in the
**            specified  vdisk (source) position of  the special vd map after
**            autoswap back operation is completed.
**
**  @param    pVDD -- Pointer to source vdisk.
**
**  @return   None
******************************************************************************
*/
void GR_ClearSpecialAswapFlag (VDD *pVDD)
{
    BIT_CLEAR(gSpecialFlagsVdMap[(pVDD->vid)/8], (pVDD->vid)%8);
}

/*
******************************************************************************
**
**  @brief    This function tells whether the specified source vdisk was auto
**            swapped in  a special failure condition like failure of DCN (at
**            other site) due to loss of entire BE access.
**
**            This enables  the  GeoRaid aswap  logic to  initiate hysteresis
**            timer or not when autoswap back operation is completed.
**
**            We dont support  hysterisis functionality  when  vdisk failover
**            happens in the special failure case like this(design limitation)
**
**  @param    pVDD -- Pointer to source vdisk.
**
**  @return   TRUE or FLASE
******************************************************************************
*/
UINT32 GR_IsSpecialAswapFlag(VDD *pVDD)
{
    return(BIT_TEST(gSpecialFlagsVdMap[(pVDD->vid)/8], (pVDD->vid)%8));
}

/*
*******************************************************************************
**
*** @brief      This  is  the function  that processes and  updates the special
**              mirror  flags  on  the  destination  VDD  in certain  specified
**              conditions.
**
**              During the processing of the write request in back end, if any
**              error  occured on  any  of the  destination  vdisks (associated
**              with the source vdisk), a  flag is set on the primary VRP(asso-
**              ciated with the source VDD I/O).
**
**              The copy  manager logic  sets a special flag on the destination
**              vdisk, when write error is identified (only first time) indica-
**              ting  that the destination vdisk  may  still be in  sync state,
**              irrespective  of  its  moved  to  auto-paused state, because at
**              that point of time, the  copy/manager will not know whether the
**              same write on its source VDD is succeded or not.Hence it leaves
**              it to the process that should be fired at the total request
**              completion.
**
**              Hence this function gets called from  Link Layer at the comple-
**              tion  of  the I/O  request  (from the back end).  This function
**              checks the primary  VRP whether any error occured on any of its
**              destination vdisks  during the write operation. If so, it veri-
**              fies the whether  the same write is succeded on the source VDD.
**              If  the I/O  operation is   succded on the source, it means the
**              destination  VDD is  in  out-of-sync, and  there  by clears the
**              special bit set on the destination vdisk.
**
**              This is provided to  handle  special Geo-RAID handling when all
**              BE connectivity  (BE switches)at a particular location is lost.
**              This processing  enables the special Geo-RAID handling logic to
**              exactly know the mirror state of the destination.
**
**  @param      retCode  --- I/O completion status (from backend)
**              pTargVRP --- Primary VRP associated with source VDD
**              pTargILT --- Primary ILT (at LLayer level)
**
**  @return     None
**
**  @attention
**              This function assumes that the VRP, VDD, COR fields are
**              validated during the forward I/O traverse.
*******************************************************************************
*/
void GR_HandleSpecialMirrorFlags(UINT32 retCode, VRP *pTargVRP, ILT *pTargILT)
{
    VDD     *pSrcVDD;
    VDD     *pDestVDD;
    SCD     *pSCD;

    /*
    ** Process the special Mirror flags only when there is an error on the
    ** destination VDD.
    */
    if (BIT_TEST(pTargVRP->options, VRP_ERROR_ON_DEST) == TRUE)
    {
        if (!pTargILT->misc)
        {
            return;
        }
        pSrcVDD = (VDD *)(pTargILT->misc);

        for (pSCD = pSrcVDD->pSCDHead; pSCD != NULL; pSCD = pSCD->link)
        {
            pDestVDD = pSCD->cor->destvdd;

            /* Check if the write request is success. */
            if ((EC_OK == retCode) && pDestVDD)
            {
#if GR_GEORAID15_DEBUG
                fprintf(stderr, "<GR><HandleSpecialMirrorFlags>---- Dest vid out of sync ---\n");
                fprintf(stderr, "<GR>svid=%x dvid=%x sda=%llx len=%x srcOp=%x permflags=%x destOp=%x permflags=%x\n",
                        pSrcVDD->vid, pDestVDD->vid, pTargVRP->startDiskAddr, pTargVRP->length,
                        pSrcVDD->grInfo.vdOpState, pSrcVDD->grInfo.permFlags,
                        pDestVDD->grInfo.vdOpState, pDestVDD->grInfo.permFlags);
#endif
                /*
                ** The write is success on server VDD, whereas there is an error
                ** on the destination VDD. The destination has now really gone out
                ** of sync. Hence clear the StillInSync Flag for the destination VDD.
                */
                GR_ClearStillInSyncFlag (pDestVDD);
            }
        }
    }
}

/*
******************************************************************************
**
**  @brief    This function checks whether the "sync at the time of all device
**            miss" bit in  the destination vdisk is set or not. If the bit is
**            set, it indicates that  the caller (copy/manager) should handle
**            it different way. The  copy/manager related  functions call this
**            during copy-ownership takeover mechanism  on the source-destina-
**            tion  vdisks  owned by  failed  DCN. The copy manager avoids the
**            restarting  of the copy  (on  this vdisk pair), since  these  are
**            already in sync state  when failover occurs.  Restarting of copy
**            puts the destination vdds in out of sync state(since sources are
**            gone) which does not GeoRaid to perform autoswap op.
**
**  @param    pDestVDD -- Pointer to destination VDD
**
**  @return   TRUE or FALSE
******************************************************************************
*/
UINT32 GR_IsAllDevMissSyncFlagSet(VDD *pDestVDD)
{
    if (pDestVDD != NULL)
    {
        return pDestVDD->grInfo.allDevMissSyncFlag;
    }
    return FALSE;
}

/*
******************************************************************************
**
**  @brief    This function  sets the "sync  at the time of all devices miss"
**            bit in  the destination  vdisk. The failing  DCN sends the list
**            of all the source and  destination vdisks (in sync) owned by it
**            to  the  surviving controller.
**
**            The surviving DCN set the all dev  miss sync flag on  all those
**            destination vdisks, so as to enable  the copy/manager to handle
**            separately during copy ownership takeover mechanism.
**
**  @param    pDestVDD -- Pointer to Destination Vdisk.
**
**  @return   None
******************************************************************************
*/
void GR_SetAllDevMissSyncFlag(VDD *pDestVDD)
{
    pDestVDD->grInfo.allDevMissSyncFlag = TRUE;
    BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
}

/*
******************************************************************************
**
**  @brief    This function clear the "sync at the time of all devices miss"
**            bit in the destination vdisk.The copy/manager after finishing
**            the special processing of these vdisks (during copy ownerhip
**            takeover), clears the all dev miss sync flag.
**
**  @param    pDestVDD -- Pointer to Destination Vdisk.
**
**  @return   None
******************************************************************************
*/
void GR_ClearAllDevMissSyncFlag(VDD *pDestVDD)
{
    pDestVDD->grInfo.allDevMissSyncFlag = FALSE;
    BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
}

/*
******************************************************************************
**
**  @brief   This function updates the removed bay map with the bay(ses) Id
**           that is getting removed.
**
**  @param   bayPid -- SES ID
**
**  @return  None
******************************************************************************
*/
void GR_SetRemovedBayMap(UINT16 bayPid)
{
    BIT_SET(gRemovedBayMap[bayPid/8], bayPid%8);
    fprintf(stderr, "<GR>Update remove bay map with bayid=%x\n", bayPid);
}

/*
******************************************************************************
**
**  @brief    This function checks whether the drive the file system which is
**            is updated belongs to the bay that was earlier failed and just
**            now recovered.
**
**            The FS$cleanup task calls this function(before perform FSUpdate
**            of a drive) to check whether the drive was the one that is part
**            of the failed diskbay due to BE Switch outage at other site.
**
**            If the above conditions are met, the pdisk is considered for
**            uninterupted FSupdate during its recovery process.
**
**  @param    pPDD  -> pointer to the pdisk
**
**  @return   TRUE  -- the drive was removed during BE switch outage at other
**                     site
**            FALSE -- Otherwise
******************************************************************************
*/
UINT32 GR_IsDriveRemoved(PDD* pPDD)
{
    if (gAllDevMissAtOtherDCN && BIT_TEST(gRemovedBayMap[(pPDD->ses)/8], (pPDD->ses)%8))
    {
        return TRUE;
    }
    return FALSE;
}

/*
******************************************************************************
**
**  @brief    This function resets all the flags those were set to identify the
**            drives failed due to BE access loss at other site. When FS update
**            of all the recovered drives (those are failed) is completed these
**            flags are reset  to zero.
**
**            The FS$cleanup task calls this function during File system update.
**            When the failed drive bays are recovered back and they are opera-
**            tional and there is no single in these bays on which FS update is
**            is still pending, the flags are reset to zero to enable the normal
**            logic in the FS$cleanup task.
**
**            Basically the special logic(to perform FSupdate without interrup-
**            tion by process) in the case of recovery of drive bays and drives
**            when they are failed due to loss of BE swithces at other site.
**
**  @param    None
**
**  @return   None
******************************************************************************
*/
void GR_ResetAllDevMissFlags(void)
{
    UINT16 pid;
    PDD*   pPDD;
    UINT8  flag = 0;

    if (gAllDevMissAtOtherDCN)
    {
        for (pid = 0; pid < MAX_DISK_BAYS; pid++)
        {
            if ((pPDD = gEDX.pdd[pid]) != NULL)
            {
                if (BIT_TEST(gRemovedBayMap[(pPDD->pid)/8], (pPDD->pid)%8) == TRUE)
                {
                    if (pPDD->devStat == PD_OP)
                    {
                        flag = 1;
                        fprintf(stderr, "<GR>BayId=%x--devstat=OP miscStat=%x postStat=%x\n", pPDD->pid, pPDD->miscStat, pPDD->postStat);
                    }
                }
            }
        }
        if (flag)
        {
            for (pid = 0; pid < MAX_PHYSICAL_DISKS; pid++)
            {
                if ((pPDD = gPDX.pdd[pid]) != NULL)
                {
                    if (BIT_TEST(gRemovedBayMap[(pPDD->ses)/8], (pPDD->ses)%8) == TRUE &&
                       pPDD->devStat == PD_OP &&
                       BIT_TEST(pPDD->miscStat, PD_MB_FSERROR))
                    {
                        fprintf(stderr, "<GR>Still more pdds to be FSupdated in the bay=%x\n", pPDD->ses);
                        return;
                    }
                }
            }
            fprintf(stderr, "<GR>Clearing AllDevMiss flag and failed BAYMAP\n");
            gAllDevMissAtOtherDCN = FALSE;
            memset(&gRemovedBayMap, 0x0, (MAX_DISK_BAYS+7)/8);
        }
    }
}

/*
******************************************************************************
**
**  @param    pSrcVdMap - Pointer to Source Virtual Disk bit Map.
**
**  @return   None
**
******************************************************************************
*/
void GR_SetCopyMirrorInfoDCNFail(UINT8 *pSrcVdMap)
{
    UINT16  i;
    VDD    *pSrcVDD;
    VDD    *pDestVDD;
    COR    *pCOR;
    SCD    *pSCD;
    UINT8   copyExists;

    for (i = 0; i < MAX_VIRTUAL_DISKS; i++)
    {
        pSrcVDD = gVDX.vdd[i];
        if (pSrcVDD)
        {
            if (BIT_TEST(pSrcVdMap[(pSrcVDD->vid)/8], (pSrcVDD->vid)%8) == TRUE)
            {
                if (GR_IS_GEORAID_VDISK(pSrcVDD))
                {
                    copyExists = FALSE;
                    for (pSCD = pSrcVDD->pSCDHead; pSCD != NULL; pSCD = pSCD->link)
                    {
                        pCOR    = pSCD->cor;
                        pDestVDD = pCOR->destvdd;

                        if (pDestVDD && GR_IS_GEORAID_VDISK(pDestVDD) )
                        {
                            if (pCOR->copystate == CST_MIRROR && pCOR->crstate == CRST_ACTIVE)
                            {
                                /*
                                ** Mark the destination VDD is in sync with its source associated to a
                                ** server at the time of BEnd  failure.
                                */
                                GR_SetAllDevMissSyncFlag(pDestVDD);
                                copyExists = TRUE;
                                fprintf(stderr,"<GR>svid=%d dvid=%d..In SYNC..Set Special Flag\n",
                                pSrcVDD->vid, pDestVDD->vid);
                            }
                            else
                            {
                                fprintf(stderr,"<GR>svid=%d dvid=%d.Not in Sync..cstate=%d crstate=%d\n",
                                pSrcVDD->vid,pDestVDD->vid,pCOR->copystate, pCOR->crstate);
                            }
                        }
                    }
                    if (copyExists)
                    {
                        /*
                        ** Don't allow the I/O in  the DCN that is going to be killed
                        ** due to loss of entire storage access.Instead I/O should be
                        ** completed with RETRY. Set the operating state of souce VDD
                        ** to IO_SUSPEND so as to enable the V$exec process to return
                        ** the I/O request with RETRY.
                        */
                        fprintf(stderr,"<GR>svid=%d ...Setting IO Suspend State\n",pSrcVDD->vid);
                        GR_SetIOSuspendState(pSrcVDD);
                    }
                }
            }
        }
    }
}

/*
******************************************************************************
**
**  @param    pSrcVdMap - Pointer to Source Virtual Disk bit Map.
**
**  @return   None
**
******************************************************************************
*/
void GR_PrepareVDMapOfFailedDCN(UINT8 *pSrcVdMap)
{
    VDD *pSrcVDD;
    int  j;

    for (j = 0; j < MAX_VIRTUAL_DISKS; ++j)
    {
        pSrcVDD = gVDX.vdd[j];

        if (pSrcVDD != NULL)
        {
            if (pSrcVDD->owner != VD_NOEXPOWNER &&
                DL_ExtractDCN(K_ficb->cSerial) != pSrcVDD->owner)
            {
                BIT_SET(pSrcVdMap[(pSrcVDD->vid)/8],(pSrcVDD->vid)%8);
            }
        }
    }
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
