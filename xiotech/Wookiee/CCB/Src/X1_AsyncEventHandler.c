/* $Id: X1_AsyncEventHandler.c 145038 2010-08-03 19:33:37Z m4 $*/
/*===========================================================================
** FILE NAME:       X1_AsyncEventHandler.c
** MODULE TITLE:    X1 Asynchronous Event Handler
**
** DESCRIPTION:     Handle X1 Async events.
**
** Copyright (c) 2001-2009 XIOtech Corporation. All rights reserved.
**==========================================================================*/
#include "X1_AsyncEventHandler.h"

#include "CachePDisk.h"
#include "CacheRaid.h"
#include "debug_files.h"
#include "EL.h"
#include "LOG_Defs.h"
#include "logdef.h"
#include "PacketStats.h"
#include "PortServerUtils.h"
#include "quorum.h"
#include "quorum_utils.h"
#include "X1_CmdCodes.h"
#include "X1_Packets.h"
#include "X1_Structs.h"
#include "XIO_Std.h"
#include "XIO_Macros.h"
#include "ddr.h"
#include "LargeArrays.h"

/*****************************************************************************
** Private defines
*****************************************************************************/

/* #define X1_ASYNC_DEBUG */
#define X1_ASYNC_CHANGE_EVENT_DEBUG

#define X1_ASYNC_ERROR_WAIT_TIME            1000        /* 1 second */

typedef struct _ASYNC_INVALIDATION
{
    UINT32      actionMask;
    UINT32      reasonMask;
} ASYNC_INVALIDATION;

#define X1AsyncInvalidateCache(a, b) {a = InvalidateCache(b, true); a ^= b;}
#define X1AsyncInvalidateError(a, b, c) {a = ResolveX1ReasonFromActionReason(b, c); \
                                         EnqueueX1AsyncError(b, a);}
#define X1AsyncGetReasons(a, b)    {a ^= b; a = ResolveActualX1ReasonFromReason(a);}

/*****************************************************************************
** Public variables
*****************************************************************************/

/*****************************************************************************
** Private variables
*****************************************************************************/
static bool X1AsyncQueueEmpty = true;
static bool X1AsyncErrorQueueEmpty = true;
static bool PIAsyncQueueEmpty = false;

static UINT8 bTaskActiveX1Async = FALSE;
static UINT8 bTaskActivePIAsync = FALSE;

static PCB *pX1AsyncTablePcb = NULL;
static PCB *pPIAsyncEventPcb = NULL;

static ASYNC_INVALIDATION X1AsyncQueue;
static ASYNC_INVALIDATION X1AsyncErrorQueue;
static PI_ASYNC_EVENT_QUEUE PIAsyncEventQueue;

static PCB *pInitRaidPcb = NULL;

/*****************************************************************************
** Public function - externed in the header file
*****************************************************************************/
extern void SendChangedEventToRegisteredClients(XIO_PACKET *asyncPkt);

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static void ProcessX1AsyncNotificationsTask(TASK_PARMS *parms);
static bool DequeueX1AsyncNotification(ASYNC_INVALIDATION *pAsync);
static bool DequeuePIAsyncEvent(PI_ASYNC_EVENT_QUEUE *pPIevent);
static UINT32 ResolveX1ActionFromReason(UINT32 reason);
static UINT32 ResolveActualX1ReasonFromReason(UINT32 reason);
static void EnqueueX1AsyncError(UINT32 action, UINT32 reason);
static void ProcessX1AsyncErrorTask(TASK_PARMS *parms);
static void X1_MonitorRaidInitializationTask(TASK_PARMS *parms);
static void SendPIAsyncChangeEventTask(TASK_PARMS *parms);
static void SendPIAsyncChangedEvent(UINT32 eventType1, UINT32 eventType2, UINT16 bitmap);

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
** Function:    SendX1AsyncNotifications
**
** Description: Send notification of information
**              change (if Required).
**
** Inputs:      logMRP  -   log Event
**                      -  NULL indicate unspecific update request and
**                         therefor update all cache and send all notifiers
**
**--------------------------------------------------------------------------*/
void SendX1AsyncNotifications(LOG_MRP *logMRP)
{
    LOG_DATA_PKT *dataPtr = NULL;

    /*
     * If we did not receive a specific update event, update all configuration
     * related cache and send appropriate notifications.
     */
    if (logMRP == NULL)
    {
        SendX1ChangeEvent((X1_ASYNC_FECHANGED | X1_ASYNC_BECHANGED));
    }
    else
    {
        /*
         * We can be a forked task, so we need to make a copy of the log data.
         */
        dataPtr = (LOG_DATA_PKT *)(logMRP->mleData);

        dprintf(DPRINTF_X1_PROTOCOL, "***** Async Notifier  - 0x%x\n", logMRP->mleEvent);

        /*
         * Look for events that cause information changes
         */
        switch (LOG_GetCode(logMRP->mleEvent))
        {
            case LOG_GetCode(LOG_PROC_NAME_DEVICE_OP):
                {
                    switch (dataPtr->procNameDevice.option)
                    {
                        case MNDSERVER:
                            /*
                             * HAB and Server information changed.
                             */
                            SendX1ChangeEvent(X1_ASYNC_ZCHANGED);
                            break;

                        case MNDVDISK:
                            /*
                             * Vdisk Information changed
                             */
                            SendX1ChangeEvent(X1_ASYNC_VCHANGED);
                            break;

                        case MNDVCG:
                            /*
                             * Vdisk Information changed
                             */
                            SendX1ChangeEvent(X1_ASYNC_VCG_CFG_CHANGED);
                            break;

                        default:
                            break;
                    }
                }
                break;

                /*
                 * Port information changed.
                 * This should actually send a pdisk event.
                 */
            case LOG_GetCode(LOG_LOOPUP):
                /* FE or BE indicator  0=FE 1=BE */
                if (dataPtr->loopUp.proc)
                {
                    SendX1ChangeEvent(X1_ASYNC_BE_PORT_CHANGE | X1_ASYNC_VCG_CFG_CHANGED);
                }
                else
                {
                    SendX1ChangeEvent(X1_ASYNC_FE_PORT_CHANGE | X1_ASYNC_VCG_CFG_CHANGED);
                }
                break;

            case LOG_GetCode(LOG_PORTDBCHANGED):
                /* FE or BE indicator  0=FE 1=BE */
                if (dataPtr->portDbChanged.proc)
                {
                    SendX1ChangeEvent(X1_ASYNC_BE_PORT_CHANGE | X1_ASYNC_VCG_CFG_CHANGED);
                }
                else
                {
                    SendX1ChangeEvent(X1_ASYNC_FE_PORT_CHANGE | X1_ASYNC_VCG_CFG_CHANGED);
                }
                break;

            case LOG_GetCode(LOG_LOOP_DOWN):
                /* FE or BE indicator  0=FE 1=BE */
                if (dataPtr->loopDown.proc)
                {
                    SendX1ChangeEvent(X1_ASYNC_BE_PORT_CHANGE | X1_ASYNC_VCG_CFG_CHANGED);
                }
                else
                {
                    SendX1ChangeEvent(X1_ASYNC_FE_PORT_CHANGE | X1_ASYNC_VCG_CFG_CHANGED);
                }
                break;

            case LOG_GetCode(LOG_PORT_EVENT):
                /* FE or BE indicator  0=FE 1=BE */
                if (dataPtr->portEvent.proc)
                {
                    SendX1ChangeEvent(X1_ASYNC_BE_PORT_CHANGE | X1_ASYNC_VCG_CFG_CHANGED);
                }
                else
                {
                    SendX1ChangeEvent(X1_ASYNC_FE_PORT_CHANGE | X1_ASYNC_VCG_CFG_CHANGED);
                }
                break;

            case LOG_GetCode(LOG_PORT_INIT_FAILED):
                /* FE or BE indicator  0=FE 1=BE */
                if (dataPtr->portEvent.proc == 0)
                {
                    SendX1ChangeEvent(X1_ASYNC_FE_PORT_CHANGE | X1_ASYNC_VCG_CFG_CHANGED);
                }
                break;

            case LOG_GetCode(LOG_BUFFER_BOARDS_ENABLED):
                SendX1ChangeEvent(ASYNC_BUFFER_BOARD_CHANGE);
                break;

            case LOG_GetCode(LOG_BUFFER_BOARDS_DISABLED_INFO):
            case LOG_GetCode(LOG_BUFFER_BOARDS_DISABLED_WARN):
            case LOG_GetCode(LOG_BUFFER_BOARDS_DISABLED_ERROR):
            case LOG_GetCode(LOG_NV_MEM_EVENT):
#if 0
            case LOG_GetCode(LOG_FE_BUFFER_BOARD_STATUS_INFO):
            case LOG_GetCode(LOG_FE_BUFFER_BOARD_STATUS_WARN):
            case LOG_GetCode(LOG_FE_BUFFER_BOARD_STATUS_ERROR):
            case LOG_GetCode(LOG_BE_BUFFER_BOARD_STATUS_INFO):
            case LOG_GetCode(LOG_BE_BUFFER_BOARD_STATUS_WARN):
            case LOG_GetCode(LOG_BE_BUFFER_BOARD_STATUS_ERROR):
#endif  /* 0 */
                SendX1ChangeEvent(ASYNC_BUFFER_BOARD_CHANGE | ASYNC_GLOBAL_CACHE_CHANGE);
                break;

            case LOG_GetCode(LOG_DEVICE_INSERTED):
                /*
                 * In case it already exists in the good pdisk map.
                 */
                RemovePdiskFromGoodPdisks(dataPtr->deviceInserted.wwn);

            case LOG_GetCode(LOG_DEVICE_REATTACED):
            case LOG_GetCode(LOG_DEVICE_REMOVED):
            case LOG_GetCode(LOG_DEVICE_RESET):
            case LOG_GetCode(LOG_CONFIG_CHANGED):
            case LOG_GetCode(LOG_PSD_REBUILD_DONE):
            case LOG_GetCode(LOG_PDD_REBUILD_DONE):
            case LOG_GetCode(LOG_HSPARE_DONE):
            case LOG_GetCode(LOG_PSD_REBUILD_START):
            case LOG_GetCode(LOG_FS_UPDATE):
            case LOG_GetCode(LOG_DEVICE_FAIL_HS):
            case LOG_GetCode(LOG_DEVICE_FAIL_NO_HS):
            case LOG_GetCode(LOG_DEVICE_MISSING):
            case LOG_GetCode(LOG_DEVICE_DELETE_OP):
            case LOG_GetCode(LOG_PDISK_LABEL_OP):
            case LOG_GetCode(LOG_PDISK_FAIL_OP):
            case LOG_GetCode(LOG_PDISK_SPINDOWN_OP):
            case LOG_GetCode(LOG_PDISK_FAILBACK_OP):
            case LOG_GetCode(LOG_PDISK_AUTO_FAILBACK_OP):
            case LOG_GetCode(LOG_PARITY_CHECK_DONE):
            case LOG_GetCode(LOG_PDISK_UNFAIL_OP):
            case LOG_GetCode(LOG_DEVICE_TIMEOUT):
                /*
                 * Physical Disk Information changed
                 */
                SendX1ChangeEvent(X1_ASYNC_PCHANGED);
                break;

            case LOG_GetCode(LOG_DEFRAG_OP_COMPLETE):
                /*
                 * Send Defrag Information changed for defrag completion.
                 */
                SendX1ChangeEvent(ASYNC_DEFRAG_CHANGE);
                break;

            case LOG_GetCode(LOG_PDISK_DEFRAG_OP):
                /*
                 * Send Defrag Information changed, Physical Disk Information
                 * changed and RAID Info changed for defrag events.
                 */
                SendX1ChangeEvent(ASYNC_DEFRAG_CHANGE | X1_ASYNC_PCHANGED |
                                  X1_ASYNC_RCHANGED);
                break;

            case LOG_GetCode(LOG_DEFRAG_DONE):
                /*
                 * Send Physical Disk Information changed and
                 * RAID Info changed for defrag events.
                 */
                SendX1ChangeEvent(X1_ASYNC_PCHANGED | X1_ASYNC_RCHANGED);
                break;

            case LOG_GetCode(LOG_RAID_EVENT):
                if (dataPtr->raidEvent.type != erlexeccomp)
                {
                    break;
                }

                /*
                 * FALL THROUGH INTO THE REST OF THE RAID OPERATIONS
                 * SO A RCHANGED EVENT WILL BE SENT.
                 */

            case LOG_GetCode(LOG_RAID_INIT_DONE):
                SendX1ChangeEvent(X1_ASYNC_VCHANGED | X1_ASYNC_RCHANGED);
                break;

            case LOG_GetCode(LOG_RAID_INIT_OP):
                X1_MonitorRaidInitialization();
                SendX1ChangeEvent(X1_ASYNC_RCHANGED);
                break;

            case LOG_GetCode(LOG_RAID_CONTROL_OP):
            case LOG_GetCode(LOG_RESYNC_DONE):
                /*
                 * Physical RAID Information changed
                 */
                SendX1ChangeEvent(X1_ASYNC_RCHANGED);
                break;


            case LOG_GetCode(LOG_VDISK_CREATE_OP):
            case LOG_GetCode(LOG_VDISK_EXPAND_OP):
                X1_MonitorRaidInitialization();
                SendX1ChangeEvent(X1_ASYNC_VCHANGED);
                break;

            case LOG_GetCode(LOG_VDISK_SET_PRIORITY):
            case LOG_GetCode(LOG_COPY_COMPLETE):
            case LOG_GetCode(LOG_COPY_SYNC):
/*          case LOG_GetCode(LOG_VDISK_PREPARE_OP):           */
            case LOG_GetCode(LOG_VDISK_SET_ATTRIBUTE_OP):
            case LOG_GetCode(LOG_VLINK_BREAK_LOCK_OP):
            case LOG_GetCode(LOG_EST_VLINK):
            case LOG_GetCode(LOG_TERM_VLINK):
            case LOG_GetCode(LOG_SWP_VLINK):
            case LOG_GetCode(LOG_VLINK_SIZE_CHANGED):
            case LOG_GetCode(LOG_VLINK_CREATE_OP):
            case LOG_GetCode(LOG_COPY_FAILED):
            case LOG_GetCode(LOG_NEW_PATH):
            case LOG_GetCode(LOG_CHANGE_NAME):
            case LOG_GetCode(LOG_COPY_LABEL):
                /*
                 * Vdisk Information changed
                 */
                SendX1ChangeEvent(X1_ASYNC_VCHANGED);
                break;

            case LOG_GetCode(LOG_VDISK_CONTROL_OP):
            case LOG_GetCode(LOG_VDISK_DELETE_OP):
            case LOG_GetCode(LOG_SERVER_ASSOC_OP):
            case LOG_GetCode(LOG_SERVER_DISASSOC_OP):
                /*
                 * Vdisk Information and server information changed.
                 *
                 * NOTE: A virtual disk delete and control operation may
                 *       results in the removal of server associations by
                 *       the PROC so we must send a ZCHANGED event.
                 *
                 * NOTE: A server associate or disassociate can result
                 *       in the ownership of the vdisk to change so
                 *       a VCHANGED event needs to be sent along with
                 *       the normal ZCHANGED event.
                 */
                SendX1ChangeEvent(X1_ASYNC_VCHANGED | X1_ASYNC_ZCHANGED);
                break;

            case LOG_GetCode(LOG_SERVER_CREATE_OP):
            case LOG_GetCode(LOG_SERVER_DELETE_OP):
            case LOG_GetCode(LOG_SERVER_SET_PROPERTIES_OP):
            case LOG_GetCode(LOG_SERVER_LOGGED_IN_OP):
                /*
                 * Server Information changed
                 */
                SendX1ChangeEvent(X1_ASYNC_ZCHANGED);
                break;

            case LOG_GetCode(LOG_ELECTION_STATE_CHANGE):
                /*
                 * If the new election state indicates the election has ended,
                 * flag that. Otherwise indicate another election state change.
                 */
                if (dataPtr->electionStateTransition.toState == ED_STATE_END_TASK)
                {
                    SendX1ChangeEvent(X1_ASYNC_VCG_ELECTION_STATE_ENDED);
                }
                else
                {
                    SendX1ChangeEvent(X1_ASYNC_VCG_ELECTION_STATE_CHANGE);
                }
                break;


            case LOG_GetCode(LOG_POWER_UP_COMPLETE):
            case LOG_GetCode(LOG_PROC_COMM_NOT_READY):
            case LOG_GetCode(LOG_NO_OWNED_DRIVES):
            case LOG_GetCode(LOG_WAIT_DISASTER):
            case LOG_GetCode(LOG_FWV_INCOMPATIBLE):
            case LOG_GetCode(LOG_MISSING_DISK_BAY):
            case LOG_GetCode(LOG_WAIT_CORRUPT_BE_NVRAM):
            case LOG_GetCode(LOG_MISSING_CONTROLLER):
            case LOG_GetCode(LOG_CTRL_FAILED):
                /*
                 * VCG power up changed
                 */
                SendX1ChangeEvent(X1_ASYNC_VCG_POWERUP);
                break;

            case LOG_GetCode(LOG_VCG_INACTIVATE_CONTROLLER_OP):
            case LOG_GetCode(LOG_VCG_ACTIVATE_CONTROLLER_OP):
            case LOG_GetCode(LOG_VCG_ADD_SLAVE_OP):
            case LOG_GetCode(LOG_VCG_APPLY_LICENSE_OP):
            case LOG_GetCode(LOG_VCG_UNFAIL_CONTROLLER_OP):
            case LOG_GetCode(LOG_VCG_FAIL_CONTROLLER_OP):
            case LOG_GetCode(LOG_VCG_REMOVE_CONTROLLER_OP):
            case LOG_GetCode(LOG_VCG_SHUTDOWN_OP):
                /*
                 * VCG configuration changed
                 */
                SendX1ChangeEvent(X1_ASYNC_VCG_CFG_CHANGED);
                break;

            case LOG_GetCode(LOG_VCG_SET_CACHE_OP):
                SendX1ChangeEvent(ASYNC_GLOBAL_CACHE_CHANGE);
                break;

            case LOG_GetCode(LOG_WORKSET_CHANGED):
                SendX1ChangeEvent(X1_ASYNC_VCG_WORKSET_CHANGED);
                break;

            case LOG_GetCode(LOG_PDATA_CREATE):
                SendX1ChangeEvent(ASYNC_PDATA_CREATE);
                break;

            case LOG_GetCode(LOG_PDATA_REMOVE):
                SendX1ChangeEvent(ASYNC_PDATA_REMOVE);
                break;

            case LOG_GetCode(LOG_PDATA_WRITE):
                SendX1ChangeEvent(ASYNC_PDATA_MODIFY);
                break;

            case LOG_GetCode(LOG_ISNS_CHANGED):
                SendX1ChangeEvent(ASYNC_ISNS_MODIFY);
                break;

            case LOG_GetCode(LOG_PRES_CHANGE):
                SendX1ChangeEvent(ASYNC_PRES_CHANGED);
                break;

            case LOG_GetCode(LOG_APOOL_CHANGE_I):
            case LOG_GetCode(LOG_APOOL_CHANGE_W):
            case LOG_GetCode(LOG_APOOL_CHANGE_E):
            case LOG_GetCode(LOG_APOOL_CHANGE):
                /*
                 * Apool (Async Replication) Information changed.
                 */
                SendX1ChangeEvent(ASYNC_APOOL_CHANGED);
                break;

            case LOG_GetCode(LOG_SPOOL_CHANGE_I):
            case LOG_GetCode(LOG_SPOOL_CHANGE_W):
            case LOG_GetCode(LOG_SPOOL_CHANGE_E):
                /*
                 * Spool (SNAPSHOT POOL) Information changed.
                 */
                SendX1ChangeEvent(SNAPPOOL_CHANGED);
                break;

            case LOG_GetCode(LOG_ISE_ELEM_CHANGE):
                /*
                 * Spool (SNAPSHOT POOL) Information changed.
                 */
                SendX1ChangeEvent(ISE_ENV_CHANGED);
                break;
        }
    }
}


/*----------------------------------------------------------------------------
** Function:    EnqueueX1AsyncError
**
** Description: Enqueues Async Errors to be resent later.
**
** Inputs:      action   -  What action to take.
**                              see cachemgr.h
**
**              reason   -  Any combination of.
**                              X1_ASYNC_PCHANGED                   0x00000001
**                              X1_ASYNC_RCHANGED                   0x00000002
**                              X1_ASYNC_VCHANGED                   0x00000004
**                              X1_ASYNC_HCHANGED                   0x00000008
**                              X1_ASYNC_ACHANGED                   0x00000010
**                              X1_ASYNC_ZCHANGED                   0x00000020
**                              X1_ASYNC_VCG_ELECTION_STATE_CHANGE  0x00010000
**                              X1_ASYNC_VCG_ELECTION_STATE_ENDED   0x00020000
**                              X1_ASYNC_VCG_POWERUP                0x00040000
**                              X1_ASYNC_VCG_CFG_CHANGED            0x00080000
**
** Returns:     none
**
**--------------------------------------------------------------------------*/
static void EnqueueX1AsyncError(UINT32 action, UINT32 reason)
{
#ifdef X1_ASYNC_DEBUG
    dprintf(DPRINTF_DEFAULT, "EnqueueX1AsyncError - action 0x%08X,  reason 0x%08X,  X1AsyncQueue.actionMask 0x%08X,  X1AsyncQueue.reasonMask 0x%08X\n",
            action, reason, X1AsyncQueue.actionMask, X1AsyncQueue.reasonMask);
#endif  /* X1_ASYNC_DEBUG */

    /*
     * If this is already on the queue, ignore it.
     */
    if (!((action & X1AsyncQueue.actionMask) && (reason & X1AsyncQueue.reasonMask)))
    {
        /*
         * Place the data on the queue
         */
        X1AsyncErrorQueue.actionMask |= action;
        X1AsyncErrorQueue.reasonMask |= reason;

        if (X1AsyncErrorQueueEmpty)
        {
            X1AsyncErrorQueueEmpty = false;
            TaskCreate(ProcessX1AsyncErrorTask, NULL);
        }
    }
}

/*----------------------------------------------------------------------------
** Function:    ProcessX1AsyncErrorTask
**
** Description: Processes any errors from ProcessX1AsyncNotificationsTask.
**
** Inputs:      dummy   -   (Needed to fork).
**
** Returns:     None
**
**--------------------------------------------------------------------------*/
static void ProcessX1AsyncErrorTask(UNUSED TASK_PARMS *parms)
{
#ifdef X1_ASYNC_DEBUG
    dprintf(DPRINTF_DEFAULT, "ProcessX1AsyncErrorTask - ENTER\n");
#endif  /* X1_ASYNC_DEBUG */

    TaskSleepMS(X1_ASYNC_ERROR_WAIT_TIME);
    EnqueueX1AsyncNotification(X1AsyncErrorQueue.actionMask,
                               X1AsyncErrorQueue.reasonMask);
    /*
     * Clear out the old data
     */
    X1AsyncErrorQueue.actionMask = CACHE_INVALIDATE_NONE;
    X1AsyncErrorQueue.reasonMask = X1_ASYNC_NONE;

    X1AsyncErrorQueueEmpty = true;
}

/*----------------------------------------------------------------------------
** Function:    ResolveX1ReasonFromAction
**
** Description: Resolves X1 reasom.
**
** Inputs:      action  -  see cachemgr.h
**              reason  -  original reason.
**
** Returns:     reason
**
**--------------------------------------------------------------------------*/
UINT32 ResolveX1ReasonFromActionReason(UINT32 action, UINT32 reason)
{
    UINT32      newreason = X1_ASYNC_NONE;

#ifdef X1_ASYNC_DEBUG
    dprintf(DPRINTF_DEFAULT, "ResolveX1ReasonFromAction - action 0x%08X, reason 0x%08X\n",
            action, reason);
#endif  /* X1_ASYNC_DEBUG */

    if ((action & CACHE_INVALIDATE_DISKBAY) ||
#ifndef NO_PDISK_CACHE
        (action & CACHE_INVALIDATE_PDISK) ||
#endif  /* NO_PDISK_CACHE */
#ifndef NO_VDISK_CACHE
        (action & CACHE_INVALIDATE_VDISK) ||
#endif  /* NO_VDISK_CACHE */
#ifndef NO_RAID_CACHE
        (action & CACHE_INVALIDATE_RAID) ||
#endif  /* NO_RAID_CACHE */
#ifndef NO_HAB_CACHE
        (action & CACHE_INVALIDATE_BE_LOOP_INFO) ||
#endif  /* NO_HAB_CACHE */
        TRUE)
    {
        if (reason & X1_ASYNC_BECHANGED)
        {
            newreason |= (reason & X1_ASYNC_BECHANGED);
        }
    }

    if ((action & CACHE_INVALIDATE_SERVER) ||
        (action & CACHE_INVALIDATE_FE_STATS) ||
        (action & CACHE_INVALIDATE_TARGET))
    {
        if (reason & X1_ASYNC_FECHANGED)
        {
            newreason |= (reason & X1_ASYNC_FECHANGED);
        }
    }

    if (action & CACHE_INVALIDATE_VCG_INFO)
    {
        if (reason & X1_ASYNC_VCGCHANGED)
        {
            newreason |= (reason & X1_ASYNC_VCGCHANGED);
        }
    }

    if (action & CACHE_INVALIDATE_WORKSET)
    {
        if (reason & X1_ASYNC_VCG_WORKSET_CHANGED)
        {
            newreason |= (reason & X1_ASYNC_VCG_WORKSET_CHANGED);
        }
    }

#ifdef X1_ASYNC_DEBUG
    dprintf(DPRINTF_DEFAULT, "ResolveX1ReasonFromAction - newreason 0x%08X\n", newreason);
#endif  /* X1_ASYNC_DEBUG */

    return newreason;
}

/*----------------------------------------------------------------------------
** Function:    ResolveX1ActionFromReason
**
** Description: Resolves X1 action.
**
** Inputs:      reason   -  Any combination of.
**                              X1_ASYNC_PCHANGED                   0x00000001
**                              X1_ASYNC_RCHANGED                   0x00000002
**                              X1_ASYNC_VCHANGED                   0x00000004
**                              X1_ASYNC_HCHANGED                   0x00000008
**                              X1_ASYNC_ACHANGED                   0x00000010
**                              X1_ASYNC_ZCHANGED                   0x00000020
**                              X1_ASYNC_VCG_ELECTION_STATE_CHANGE  0x00010000
**                              X1_ASYNC_VCG_ELECTION_STATE_ENDED   0x00020000
**                              X1_ASYNC_VCG_POWERUP                0x00040000
**                              X1_ASYNC_VCG_CFG_CHANGED            0x00080000
** Returns:     action
**
**--------------------------------------------------------------------------*/
static UINT32 ResolveX1ActionFromReason(UINT32 reason)
{
    UINT32      action = CACHE_INVALIDATE_NONE;

#ifdef X1_ASYNC_DEBUG
    dprintf(DPRINTF_DEFAULT, "ResolveX1ActionFromReason - reason 0x%08X\n", reason);
#endif  /* X1_ASYNC_DEBUG */

    if ((reason & X1_ASYNC_VCG_ELECTION_STATE_CHANGE) ||
        (reason & X1_ASYNC_VCG_ELECTION_STATE_ENDED) ||
        (reason & X1_ASYNC_VCG_POWERUP) ||
        (reason & X1_ASYNC_VCG_CFG_CHANGED))
    {
        action |= CACHE_INVALIDATE_VCG_INFO;
    }

    if ((reason & X1_ASYNC_PCHANGED) ||
        (reason & X1_ASYNC_RCHANGED) ||
        (reason & X1_ASYNC_VCHANGED))
    {
        action |= CACHE_INVALIDATE_BE;
    }

    if ((reason & X1_ASYNC_HCHANGED) ||
        (reason & X1_ASYNC_ZCHANGED) ||
        (reason & X1_ASYNC_FE_PORT_CHANGE))
    {
        action |= (CACHE_INVALIDATE_FE | CACHE_INVALIDATE_FE_STATS);
    }

    if (reason & X1_ASYNC_BE_PORT_CHANGE)
    {
        action |= (CACHE_INVALIDATE_BE_LOOP_INFO | CACHE_INVALIDATE_BE);
    }

    if (reason & X1_ASYNC_VCG_WORKSET_CHANGED)
    {
        action |= CACHE_INVALIDATE_WORKSET;
    }

    return action;
}

/*----------------------------------------------------------------------------
** Function:    ResolveActualX1ReasonFromReason
**
** Description: Resolves actual X1 reason .
**
** Inputs:      reason   -  Any combination of.
**                              X1_ASYNC_PCHANGED                   0x00000001
**                              X1_ASYNC_RCHANGED                   0x00000002
**                              X1_ASYNC_VCHANGED                   0x00000004
**                              X1_ASYNC_HCHANGED                   0x00000008
**                              X1_ASYNC_ACHANGED                   0x00000010
**                              X1_ASYNC_ZCHANGED                   0x00000020
**                              X1_ASYNC_VCG_ELECTION_STATE_CHANGE  0x00010000
**                              X1_ASYNC_VCG_ELECTION_STATE_ENDED   0x00020000
**                              X1_ASYNC_VCG_POWERUP                0x00040000
**                              X1_ASYNC_VCG_CFG_CHANGED            0x00080000
** Returns:     action
**
**--------------------------------------------------------------------------*/
static UINT32 ResolveActualX1ReasonFromReason(UINT32 reason)
{
    UINT32      actualReason = X1_ASYNC_NONE;

#ifdef X1_ASYNC_DEBUG
    dprintf(DPRINTF_DEFAULT, "ResolveActualX1ReasonFromReason - reason 0x%08X\n", reason);
#endif  /* X1_ASYNC_DEBUG */

    if ((reason & X1_ASYNC_NONE) ||
        (reason & X1_ASYNC_PCHANGED) ||
        (reason & X1_ASYNC_RCHANGED) ||
        (reason & X1_ASYNC_VCHANGED) ||
        (reason & X1_ASYNC_HCHANGED) ||
        (reason & X1_ASYNC_ZCHANGED) ||
        (reason & X1_ASYNC_VCG_ELECTION_STATE_CHANGE) ||
        (reason & X1_ASYNC_VCG_ELECTION_STATE_ENDED) ||
        (reason & X1_ASYNC_VCG_POWERUP) ||
        (reason & X1_ASYNC_VCG_WORKSET_CHANGED) ||
        (reason & X1_ASYNC_VCG_CFG_CHANGED) ||
        (reason & SNAPPOOL_CHANGED) ||
        (reason & ASYNC_PDATA_CREATE) ||
        (reason & ASYNC_PDATA_REMOVE) ||
        (reason & ASYNC_PDATA_MODIFY) ||
        (reason & ASYNC_ISNS_MODIFY) ||
        (reason & ASYNC_PRES_CHANGED) ||
        (reason & ASYNC_APOOL_CHANGED) ||
        (reason & ASYNC_BUFFER_BOARD_CHANGE) ||
        (reason & ASYNC_DEFRAG_CHANGE) ||
        (reason & ASYNC_GLOBAL_CACHE_CHANGE) ||
        (reason & ISE_ENV_CHANGED))
    {
        actualReason |= reason;
    }

    if (reason & X1_ASYNC_BE_PORT_CHANGE)
    {
        actualReason |= X1_ASYNC_PCHANGED;
    }

    /*
     * Remove the X1_ASYNC_ACHANGED from the actualReason.
     * The Xioservice currently will die if we send it
     * this event.
     */
    actualReason &= ~X1_ASYNC_ACHANGED;

    return actualReason;
}

/*----------------------------------------------------------------------------
** Function:    ProcessX1AsyncTable
**
** Description: Forked task to process X1 async Notifications.
**
** Inputs:      Dummy   -   Makes it forkable
**
** Returns:     none
**
**--------------------------------------------------------------------------*/
static NORETURN void ProcessX1AsyncNotificationsTask(UNUSED TASK_PARMS *parms)
{
    ASYNC_INVALIDATION async;
    UINT32      actionMask;
    UINT32      reasonMask;

    /*
     * Loop forever and ever and ever and ever and ever...
     */

    while (1)
    {
        /*
         * While we have items on the queue, process those items
         */
        while (DequeueX1AsyncNotification(&async))
        {
#ifdef X1_ASYNC_DEBUG
            dprintf(DPRINTF_DEFAULT, "ProcessX1AsyncTable - actionMask 0x%08X, reasonMask 0x%08X\n",
                    async.actionMask, async.reasonMask);
#endif  /* X1_ASYNC_DEBUG */

            actionMask = 0;
            reasonMask = 0;

            /*
             * Refresh any cache information.
             */
            if (async.actionMask != CACHE_INVALIDATE_NONE)
            {
                /*
                 * Invalidate the cache.
                 */
                X1AsyncInvalidateCache(actionMask, async.actionMask);

                /*
                 * Check the return Mask, if it is not good
                 * Enqueue the error.
                 */
                if (actionMask != PI_GOOD)
                {
                    X1AsyncInvalidateError(reasonMask, actionMask, async.reasonMask);
#ifdef X1_ASYNC_DEBUG
                    dprintf(DPRINTF_DEFAULT, "ProcessX1AsyncTable - ERROR actionMask 0x%08X, reasonMask 0x%08X\n",
                            actionMask, reasonMask);
#endif  /* X1_ASYNC_DEBUG */
                }
            }

            X1AsyncGetReasons(reasonMask, async.reasonMask);

            /*
             * Send the change events to registered clients
             */
//            SendPIAsyncChangedEvent (reasonMask ,0);
            EnqueuePIAsyncEvent(reasonMask, PIASYNC_EVENT_FIRST32_MAP);

            /*
             * Give others a chance.
             */
            TaskSwitch();
        }

        /*
         * We are done processing the queue for now,
         * Sleep until we are woken up.
         */
        TaskSetState(pX1AsyncTablePcb, PCB_NOT_READY);
        TaskSwitch();
    }
}

/*----------------------------------------------------------------------------
** Function:    SendX1ChangeEvent
**
** Description: Sends an X1 change event.
**
** Inputs:      reason   -  Any combination of.
**                              X1_ASYNC_PCHANGED                   0x00000001
**                              X1_ASYNC_RCHANGED                   0x00000002
**                              X1_ASYNC_VCHANGED                   0x00000004
**                              X1_ASYNC_HCHANGED                   0x00000008
**                              X1_ASYNC_ACHANGED                   0x00000010
**                              X1_ASYNC_ZCHANGED                   0x00000020
**                              X1_ASYNC_VCG_ELECTION_STATE_CHANGE  0x00010000
**                              X1_ASYNC_VCG_ELECTION_STATE_ENDED   0x00020000
**                              X1_ASYNC_VCG_POWERUP                0x00040000
**                              X1_ASYNC_VCG_CFG_CHANGED            0x00080000
**
** Returns:     none
**
**--------------------------------------------------------------------------*/
void SendX1ChangeEvent(UINT32 reason)
{
    EnqueueX1AsyncNotification(ResolveX1ActionFromReason(reason), reason);
}

/*----------------------------------------------------------------------------
** Function:    EnqueueX1AsyncNotification
**
** Description: Enqueues AsyncNotifications to be sent later.
**
** Inputs:      action   -  What action to take.
**                              see CacheManager.h
**
**              reason   -  Any combination of.
**                              X1_ASYNC_PCHANGED                   0x00000001
**                              X1_ASYNC_RCHANGED                   0x00000002
**                              X1_ASYNC_VCHANGED                   0x00000004
**                              X1_ASYNC_HCHANGED                   0x00000008
**                              X1_ASYNC_ACHANGED                   0x00000010
**                              X1_ASYNC_ZCHANGED                   0x00000020
**                              X1_ASYNC_VCG_ELECTION_STATE_CHANGE  0x00010000
**                              X1_ASYNC_VCG_ELECTION_STATE_ENDED   0x00020000
**                              X1_ASYNC_VCG_POWERUP                0x00040000
**                              X1_ASYNC_VCG_CFG_CHANGED            0x00080000
**
** Returns:     none
**
**--------------------------------------------------------------------------*/
void EnqueueX1AsyncNotification(UINT32 action, UINT32 reason)
{
    /*
     * Place the data on the queue
     */
    X1AsyncQueue.actionMask |= action;
    X1AsyncQueue.reasonMask |= reason;

    X1AsyncQueueEmpty = false;

    /*
     * if the task that processes X1 async events from the queue is not
     * active, activate it. ONLY DONE ONCE!
     */
    if (bTaskActiveX1Async == FALSE)
    {
        bTaskActiveX1Async = TRUE;
        pX1AsyncTablePcb = TaskCreate(ProcessX1AsyncNotificationsTask, NULL);
    }

    /*
     * if the X1 Async task exists and is not ready, wake it up.
     */
    if ((pX1AsyncTablePcb != NULL) && (TaskGetState(pX1AsyncTablePcb) == PCB_NOT_READY))
    {
        TaskSetState(pX1AsyncTablePcb, PCB_READY);
    }
}

/*----------------------------------------------------------------------------
** Function:    DequeueX1AsyncNotification
**
** Description: Dequeues Async event, and returns the index number of loation.
**
** Inputs:      async   -   output ASYNC_INVALIDATION copied here.
**
** Returns:     index of next item on queue.
**              X1_ASYNC_TABLE_INVALID_INDEX if queue is empty.
**
**--------------------------------------------------------------------------*/
static bool DequeueX1AsyncNotification(ASYNC_INVALIDATION *pAsync)
{
    bool        dequeued = false;

    /*
     * If there's data on the queue then process it.
     */
    if (!X1AsyncQueueEmpty)
    {
        /*
         * Copy the data from the queue to the ASYNC_INVALIDATION passed in.
         */
        pAsync->actionMask = X1AsyncQueue.actionMask;
        pAsync->reasonMask = X1AsyncQueue.reasonMask;

        /*
         * Clear out the old data
         */
        X1AsyncQueue.actionMask = CACHE_INVALIDATE_NONE;
        X1AsyncQueue.reasonMask = X1_ASYNC_NONE;

        dequeued = true;
        X1AsyncQueueEmpty = true;
    }

    return dequeued;
}

/*----------------------------------------------------------------------------
** Function:    X1_ElectionNotify
**
** Description: Responsible for passing along election state changes to the
**              rest of the X1 interface.
**
** Inputs:      electionState   - New election state
**
** Returns:     NONE
**--------------------------------------------------------------------------*/
INT32 X1_ElectionNotify(UINT8 electionState)
{
    /*
     * If we become master, alert the raid initialization task.
     */
    if (electionState == ELECTION_AM_MASTER)
    {
        X1_MonitorRaidInitialization();
    }

    return GOOD;
}

/*----------------------------------------------------------------------------
** Function:    X1_MonitorRaidInitialization
**
** Description: Monitors Raid Initialization
**
** Returns:     NONE
**--------------------------------------------------------------------------*/
void X1_MonitorRaidInitialization(void)
{
    /*
     * If X1_MonitorRaidInitializationTask is not running start it.
     */
    if ((GetMyControllerSN() == Qm_GetMasterControllerSN()) && (pInitRaidPcb == NULL))
    {
        pInitRaidPcb = TaskCreate(X1_MonitorRaidInitializationTask, NULL);
    }
}

/*----------------------------------------------------------------------------
** Function:    X1_MonitorRaidInitializationTask
**
** Description: Monitors Raid Initialization and sends X1RPKT_GET_RAIDPER
**
** Returns:     NONE
**--------------------------------------------------------------------------*/
static void X1_MonitorRaidInitializationTask(UNUSED TASK_PARMS *parms)
{
    UINT8                   finished = FALSE;
    UINT32                  index1;
    UINT8                  *pCacheEntry;
    MRGETRINFO_RSP         *pTmpRaid;
    XIO_PACKET              rspPkt;
    PI_PACKET_HEADER        rspHdr;
    X1_GET_RAID_PERCENT_RSP rChngPkt;
    UINT16                  numRaids;
#ifdef NO_RAID_CACHE
    struct DMC *entry = DMC_CCB + CCB_DMC_raidcache;
#endif  /* NO_RAID_CACHE */

    dprintf(DPRINTF_CACHEMGR, "X1_MonitorRaidInitializationTask - ENTER\n");

    /* Set up the response packet header and data pointers. */
    rspPkt.pHeader = &rspHdr;
    rspPkt.pPacket = (UINT8 *)&rChngPkt;

    /*
     * Set the response data length in the header. This is needed for
     * the send across TCP (i.e. SendX1Packet()).
     */
    rspHdr.length = sizeof(X1_GET_RAID_PERCENT_RSP);
    rspHdr.packetVersion = 1;

    /* Set the command code of the packet. */
    rChngPkt.header.cmdCode = X1RPKT_GET_RAIDPER;

    /* Wait 5 seconds to allow things to settle down after an election. */
    TaskSleepMS(5000);

    while ((GetMyControllerSN() == Qm_GetMasterControllerSN()) && (finished == FALSE))
    {
        /* Start finished at TRUE. If it is not we will set it accordingly. */
        finished = TRUE;

#ifndef NO_RAID_CACHE
        /* Refresh the raid cache. */
        InvalidateCacheRaids(true);

        /*
         * Wait until the RAIDs cache is not in the process of being updated.
         * Once it is in that state make sure it is set to in use so a
         * update doesn't start while it is being used.
         */
        CacheStateWaitUpdating(cacheRaidsState);
        CacheStateSetInUse(cacheRaidsState);

        /* Get a pointer to the beginning of the raid cache. */
        pCacheEntry = cacheRaids;
#else   /* NO_RAID_CACHE */
        /* Get memory lock and wait if busy. */
        Wait_DMC_Lock(entry);

        pCacheEntry = cacheRaidBuffer_DMC.cacheRaidBuffer_DMC;
#endif  /* NO_RAID_CACHE */

        /*
         * Loop through the raids, and see if any are initializing. If they
         * are, send a X1RPKT_GET_RAIDPER to the XIOService so that it will
         * refresh its data
         */
        numRaids = RaidsCount();
        for (index1 = 0; index1 < numRaids; ++index1)
        {
            /* Get a pointer to the raid we need to look at. */
            pTmpRaid = (MRGETRINFO_RSP *)pCacheEntry;

            /* Check to see if this raid is initializing. */
            if ((pTmpRaid->status == RD_INIT) || (pTmpRaid->pctRem > 0))
            {
                /* Set the packet values. */
                rChngPkt.raidID = pTmpRaid->rid;
                rChngPkt.pctRem = pTmpRaid->pctRem;

                /* We are not done, keep going. */
                finished = FALSE;
            }

            /* Increment our cache pointer. */
            pCacheEntry += pTmpRaid->header.len;
        }

#ifndef NO_RAID_CACHE
        /* No longer using the cache for this function. */
        CacheStateSetNotInUse(cacheRaidsState);
#else   /* NO_RAID_CACHE */
        /* Done with lock. */
        Free_DMC_Lock(entry);
#endif  /* NO_RAID_CACHE */

        /* If we need to keep going, wait 5 seconds. */
        if (finished == FALSE)
        {
            /* Send the PI Async Rchange Event. */
//            SendPIAsyncChangedEvent (X1_ASYNC_RCHANGED, 0);
            EnqueuePIAsyncEvent(X1_ASYNC_RCHANGED, PIASYNC_EVENT_FIRST32_MAP);

            TaskSleepMS(5000);
        }
    }

    /* Clear our PCB, so that we can run again. */
    pInitRaidPcb = NULL;

    dprintf(DPRINTF_CACHEMGR, "X1_MonitorRaidInitializationTask - EXIT\n");
}

/*----------------------------------------------------------------------------
** Function:    EnqueuePIAsyncEvent
**
** Description: Enqueues PIAsyncEventRequest.
**
** Inputs:      eventMask -  Actions to be taken
**
** Returns:     none
**
**--------------------------------------------------------------------------*/
void EnqueuePIAsyncEvent(UINT32 eventMask, UINT16 bitmap)
{

    PIAsyncEventQueue.bitmap |= bitmap;
    if (bitmap == PIASYNC_EVENT_FIRST32_MAP)
    {
        PIAsyncEventQueue.event1 |= eventMask;
    }
    if (bitmap == PIASYNC_EVENT_SECOND32_MAP)
    {
        PIAsyncEventQueue.event2 |= eventMask;
    }
    PIAsyncQueueEmpty = false;


    /*
     * if the task that processes X1 async events from the queue is not
     * active, activate it. ONLY DONE ONCE!
     */
    if (bTaskActivePIAsync == FALSE)
    {
        bTaskActivePIAsync = TRUE;
        pPIAsyncEventPcb = TaskCreate(SendPIAsyncChangeEventTask, NULL);
    }

    /*
     * if the PI Async event task exists and is not ready, wake it up.
     */
    if ((pPIAsyncEventPcb != NULL) && (TaskGetState(pPIAsyncEventPcb) == PCB_NOT_READY))
    {
        TaskSetState(pPIAsyncEventPcb, PCB_READY);
    }
}

/*----------------------------------------------------------------------------
** Function:    DequeuePIAsyncEvent
**
** Description: Dequeues PI Async event, and returns the index number of loation.
**
** Inputs:         -   output ASYNC_INVALIDATION copied here.
**
** Returns:     index of next item on queue.
**              X1_ASYNC_TABLE_INVALID_INDEX if queue is empty.
**
**--------------------------------------------------------------------------*/
static bool DequeuePIAsyncEvent(PI_ASYNC_EVENT_QUEUE *pPIevent)
{
    bool        dequeued = false;

    /*
     * If there's data on the queue then process it.
     */
    if (!PIAsyncQueueEmpty)
    {
        /*
         * Copy the data from the queue to the ASYNC_INVALIDATION passed in.
         */
        pPIevent->event1 = PIAsyncEventQueue.event1;
        pPIevent->event2 = PIAsyncEventQueue.event2;
        pPIevent->bitmap = PIAsyncEventQueue.bitmap;
        /*
         * Clear out the old data
         */
        PIAsyncEventQueue.event1 = PI_ASYNC_NONE;
        PIAsyncEventQueue.event2 = PI_ASYNC_NONE;
        PIAsyncEventQueue.bitmap = 0;

        dequeued = true;
        PIAsyncQueueEmpty = true;
    }
    return dequeued;
}


/*----------------------------------------------------------------------------
** Function:    SendPIAsyncChangeEventTask
**
** Description: Forked task to process PI async Notifications.
**
** Inputs:      Dummy   -   Makes it forkable
**
** Returns:     none
**
**--------------------------------------------------------------------------*/

static NORETURN void SendPIAsyncChangeEventTask(UNUSED TASK_PARMS *parms)
{
    PI_ASYNC_EVENT_QUEUE piEventQueue;

    /*
     * Loop forever and ever and ever and ever and ever...
     */

    while (1)
    {
        /*
         * While we have items on the queue, process those items
         */
        while (DequeuePIAsyncEvent(&piEventQueue))
        {
            SendPIAsyncChangedEvent(piEventQueue.event1, piEventQueue.event2,
                                    piEventQueue.bitmap);
        }

        /*
         * We are done processing the queue for now,
         * Sleep until we are woken up.
         */
        TaskSetState(pPIAsyncEventPcb, PCB_NOT_READY);
        TaskSwitch();
    }
}


/**
******************************************************************************
**
**  @brief      This function sends changed events to all registered clients.
**
**  @param      event type, event bitmap
**
**  @return     none
**
******************************************************************************
**/
static void SendPIAsyncChangedEvent(UINT32 eventType1, UINT32 eventType2,
                                    UNUSED UINT16 bitmap)
{
    XIO_PACKET  asyncPkt;
    ASYNC_CHANGED_EVENT *pEvent;

    asyncPkt.pHeader = MallocWC(sizeof(PI_PACKET_HEADER));
    asyncPkt.pPacket = MallocWC(sizeof(ASYNC_CHANGED_EVENT));
    asyncPkt.pHeader->packetVersion = 1;

    asyncPkt.pHeader->commandCode = PI_ASYNC_CHANGED_EVENT;
    asyncPkt.pHeader->headerLength = sizeof(PI_PACKET_HEADER);
    asyncPkt.pHeader->length = sizeof(ASYNC_CHANGED_EVENT);
    pEvent = (ASYNC_CHANGED_EVENT *)(asyncPkt.pPacket);
    pEvent->extendEvnt.eventType1 = eventType1;
    pEvent->extendEvnt.eventType2 = eventType2;
    SendChangedEventToRegisteredClients(&asyncPkt);
    Free(asyncPkt.pHeader);
    Free(asyncPkt.pPacket);
    return;
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
