/* $Id: AsyncEventHandler.c 162911 2014-03-20 22:45:34Z marshall_midden $*/
/**
******************************************************************************
**
**  @file   AsyncEventHandler.c
**
**  @brief  Asynchronous Event Handler
**
**  Handle events coming from the backend or frontend processor to the CCB.
**
**  Copyright (c) 2001-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#include "AsyncEventHandler.h"
#include "debug_files.h"
#include "LOG_Defs.h"
#include "CacheLoop.h"
#include "CacheManager.h"
#include "CacheMisc.h"
#include "CmdLayers.h"
#include "convert.h"
#include "cps_init.h"
#include "EL.h"
#include "FCM.h"
#include "FCM_Counters.h"
#include "fm.h"
#include "ipc_packets.h"
#include "ipc_sendpacket.h"
#include "kernel.h"
#include "LL_Stats.h"
#include "logdef.h"
#include "logging.h"
#include "misc.h"
#include "MR_Defs.h"
#include "PacketInterface.h"
#include "PktCmdHdl.h"
#include "PI_BatteryBoard.h"
#include "PI_Utils.h"
#include "PI_VDisk.h"
#include "PR.h"
#include "quorum_utils.h"
#include "rm.h"
#include "RMCmdHdl.h"
#include "rm_val.h"
#include "ses.h"
#include "sm.h"
#include "trace.h"
#include "XIO_Std.h"
#include "PacketStats.h"
#include "Snapshot.h"
#include "X1_AsyncEventHandler.h"
#include "CT_history.h"

#include <byteswap.h>


/*****************************************************************************
** Private defines
*****************************************************************************/

/*
** AEH_IPC_SEND_TMO is the timeout for sending an IPC packet to another
** controller.  The value represents how many milliseconds to wait for the
** send to complete.  At the current value of 120000 the timeout is 2 minutes.
*/
#define AEH_IPC_SEND_TMO                120000

/*
** Delay (in seconds) before allowing the new server task to process
** the create server requests.  This delay allows the system to
** (hopefully) come to a stable state before executing the requests.
*/
#define NEW_SERVER_TASK_DELAY               60

/*****************************************************************************
** Private variables
*****************************************************************************/
static UINT8 bTaskActiveNewServer = 0;
static S_LIST *ptrNewServerQueue = NULL;
static UINT8 gNewServerTaskCounter = 0;

static bool bDeviceFailContinue = false;
static PCB *pcbDeviceFailTask = NULL;

static bool gbProcessBroadcast = true;

static S_LIST *ptrBroadcastQueue = NULL;
static PCB *pcbBroadcastTask = NULL;
static UINT32 gLocalImageCount = 0;

static S_LIST *ptrBroadcastDGQueue = NULL;
static PCB *pcbBroadcastDGTask = NULL;

static PCB *pPDiskDefragTask = NULL;
static bool gbPDiskDefragStop = true;
static UINT16 gPDiskDefragVIP = 0xFFFF;

/*****************************************************************************
** Public variables - externed in the header file
*****************************************************************************/
extern void SendPresPacketToMaster(LOG_PRES_EVENT_PKT *pLOG);

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static void AsyncGroupValidation(UINT16 validationFlags);
static void EnqueueToNewServerQueue(NEW_SERVER *pNewServer);
static void ProcessNewServerQueue(TASK_PARMS *parms);
static void CreateServer(UINT32 controllerSN, NEW_SERVER *pNewServer);

static void AsyncEventDeviceFailHS(void);
static void AsyncEventDeviceFailHSTask(TASK_PARMS *parms);      /* FORKED */

static void EnqueueToBroadcastQueue(IPC_BROADCAST *pBroadcast);
static void ProcessBroadcastQueueTask(TASK_PARMS *parms);
static int  compareBroadcastDG(void const *e1, void const *e2);
static void ProcessBroadcastDGConfigLock(IPC_BROADCAST *pBroadcast);
static void ProcessBroadcastDGQueueTask(TASK_PARMS *parms);     /* FORKED */

static void ProcessBatchBroadcast(S_LIST *ptrBatchQueue, UINT16 eventType,
                                  UINT16 bcastType, UINT32 serialNum, UINT32 dataSize);

static void BroadcastToSelf(IPC_BROADCAST *pBroadcast);
static void BroadcastToControllers(IPC_BROADCAST *pBroadcast);
static UINT32 SendIpcBroadcast(UINT32 serialNum, IPC_BROADCAST *pBroadcast);

static void PDiskDefragTask(TASK_PARMS *parms); /* FORKED */
static void PDiskDefragWaitForRebuilds(void);
static INT32 PDiskDefragVerify(MRDEFRAGMENT_RSP *pDefrag);
static void PDiskDefragMove(MRDEFRAGMENT_RSP *pDefrag);
static void PDiskDefragLogDefragOpComplete(UINT8 status);
static bool PDiskDefragCheckForRebuilds(void);
static void AsyncPresChange(void);

/*****************************************************************************
** Code Start
*****************************************************************************/

/**
******************************************************************************
**
**  @brief  Handle asynchronous events such as logging and server detection
**
**  @param  ptrMRP - Pointer to MRP
**  @param  logMRP - Pointer to LOG_MRP
**
**  @return none
**
******************************************************************************
**/
void AsyncEventHandler(MRP *ptrMRP, LOG_MRP *logMRP)
{
    UINT8       logThisEvent = TRUE;
    UINT16      mleEvent;
    LOG_DATA_PKT    *mleData;
    IPC_REPORT_CONTROLLER_FAILURE *pFailurePacket;
    FAILURE_DATA_STATE fd;
    UINT32      controllerNode;

    mleEvent = (UINT16)logMRP->mleEvent;
    mleData = (LOG_DATA_PKT *)logMRP->mleData;
    TraceEvent(TRACE_LOG, logMRP->mleEvent);

    /*
     * Update the last operation we started from the proc
     */
    if (ptrMRP)
    {
        L_stattbl->LL_LastIOp = mleEvent;
    }

    /*
     * If the controller is power-up complete and it is in the shutdown
     * state we do not want to process any messages.  This will still
     * log the messages but we will avoid the special handling.
     */
    if (!PowerUpComplete() || GetControllerFailureState() != FD_STATE_VCG_SHUTDOWN)
    {
        /*
         * If there is special handling required for the event it will
         * have a case in this switch statement.
         */
        switch (LOG_GetCode(mleEvent))
        {
        case LOG_GetCode(LOG_CHANGE_LED):
            if (!DiscoveringSES)
            {
                /*
                 * The change LED event is slow due to SES calls being
                 * slow so we are going to pull this event off the queue
                 * and process it using a separate task.  To do that we
                 * need to make a copy of the event data since the original
                 * data is owned by the PROC and if we return before
                 * completing our work the memory would have been freed on
                 * us.  So, make a copy of the data and put it on the
                 * queue of events to be handled by the change LED task
                 * (the change LED task is coded in SES.C).
                 */
                IPC_LED_CHANGE *pEvent = MallocWC(sizeof(*pEvent));

                pEvent->serialNum = GetMyControllerSN();
                pEvent->wwn = (UINT64)(*((UINT64 *)(logMRP->mleData)));

                /*
                 * An asynchronous event can either turn the FAILED LED on
                 * or turn it off.
                 */
                if (logMRP->mleData[2] == SES_LED_FAIL)
                {
                    pEvent->ledReq = LED_FAIL_ON;
                }
                else
                {
                    pEvent->ledReq = LED_BOTH_OFF;
                }

                EnqueueToLedControlQueue(pEvent);
            }

            logThisEvent = FALSE;
            break;

        case LOG_GetCode(LOG_ZONE_INQUIRY):
            {
                /*
                 * Allocate the event buffer.
                 */
                NEW_SERVER *pNewServer = MallocWC(sizeof(*pNewServer));

                /*
                 * Copy the data from the async event.
                 */
                pNewServer->channel = (UINT8)logMRP->mleData[0];
                pNewServer->targetId = (UINT16)(logMRP->mleData[0] >> 16);
                pNewServer->owner = logMRP->mleData[1];
                pNewServer->wwn = (((UINT64)logMRP->mleData[3]) << 32) | logMRP->mleData[2];
                memcpy(pNewServer->i_name, &logMRP->mleData[4], 254);

                /*
                 * Push the event on the queue.
                 */
                EnqueueToNewServerQueue(pNewServer);
            }
            break;

        case LOG_GetCode(LOG_DEVICE_REMOVED):
        case LOG_GetCode(LOG_DEVICE_RESET):
        case LOG_GetCode(LOG_DEVICE_INSERTED):
        case LOG_GetCode(LOG_DEVICE_REATTACED):
        case LOG_GetCode(LOG_DISKBAY_REMOVED):
        case LOG_GetCode(LOG_DISKBAY_INSERTED):
        case LOG_GetCode(LOG_DISKBAY_MOVED):
            FCM_CountersMajorStorageEvent();
            break;

        case LOG_GetCode(LOG_CONFIG_CHANGED):
#if defined(MODEL_7000) || defined(MODEL_4700)
            DiscoverSES(NULL);
#else  /* MODEL_7000 || MODEL_4700 */
            DiscoverSES_StartTask();
#endif /* MODEL_7000 || MODEL_4700 */
            break;

        case LOG_GetCode(LOG_DEVICE_FAIL_HS):
            AsyncEventDeviceFailHS();
            break;

        case LOG_GetCode(LOG_LOOP_DOWN):
            if (mleData->loopDown.proc == logporteventbe)
            {
                /*
                 * Trigger validation and log the error.
                 */
                AsyncGroupValidation(VAL_TYPE_BACK_END);
                LOG_SetError(logMRP->mleEvent);

                /*
                 * Tell other controllers to refresh their cached
                 * BE loop info.  This is required for the current
                 * (pre-R3) BLOBs to work.
                 */
                CacheRefreshAllRmtCtrl(CACHE_INVALIDATE_BE_LOOP_INFO, FALSE);

                FCM_CountersMajorStorageEvent();
            }
            break;

        case LOG_GetCode(LOG_LOOPUP):
            /* See if this is a front end event */
            if (mleData->loopUp.proc == logportinitfe)
            {
                if (mleData->loopUp.failed == FALSE)
                {
                    /*
                     * Since this is an FE event it should trigger
                     * Server Validation and Comm validation.
                     */
                    AsyncGroupValidation(VAL_TYPE_SERVER | VAL_TYPE_COMM);
                }
            }
            else
            {
                /*
                 * This is a back end event.  Trigger validation based
                 * on the state of the flag.
                 */
                if (mleData->loopUp.failed == FALSE)
                {
                    AsyncGroupValidation(VAL_TYPE_BACK_END);

                    /*
                     * Tell other controllers to refresh their cached
                     * BE loop info.  This is required for the current
                     * (pre-R3) BLOBs to work.
                     */
                    CacheRefreshAllRmtCtrl(CACHE_INVALIDATE_BE_LOOP_INFO, FALSE);
                }

                /*
                 * Cache the port's loop/fabric status
                 */
                SetBEFabricMode(mleData->loopUp.port,
                        BIT_TEST(mleData->loopUp.flags, PORT_FABRIC_MODE_BIT));

                FCM_CountersMajorStorageEvent();
            }
            break;

        case LOG_GetCode(LOG_PORTDBCHANGED):
            /*
             * If this is an FE event it should trigger Server Validation
             * and Comm validation
             */
            if (mleData->portDbChanged.proc == logportinitfe)
            {
                AsyncGroupValidation(VAL_TYPE_SERVER | VAL_TYPE_COMM);
            }
            else
            {
                /*
                 * This is a back end event.
                 */
                AsyncGroupValidation(VAL_TYPE_BACK_END);
            }
            break;

        case LOG_GetCode(LOG_PORT_INIT_FAILED):
            if (mleData->portInit.proc == logportinitfe)
            {
                /* Send an interface failure firmware error */
                pFailurePacket = MallocWC(SIZEOF_IPC_INTERFACE_FAILURE);
                pFailurePacket->Type = IPC_FAILURE_TYPE_INTERFACE_FAILED;
                pFailurePacket->FailureData.InterfaceFailure.DetectedBySN = GetMyControllerSN();
                pFailurePacket->FailureData.InterfaceFailure.FailedInterfaceID = mleData->portInit.port;
                pFailurePacket->FailureData.InterfaceFailure.ControllerSN = GetMyControllerSN();
                pFailurePacket->FailureData.InterfaceFailure.InterfaceFailureType = INTERFACE_FAIL_FIRMWARE_ERROR;

                EnqueueToFailureManagerQueue(pFailurePacket, SIZEOF_IPC_INTERFACE_FAILURE);
                /* Don't release the failure packet, FailureManager owns it now */
            }
            break;

        case LOG_GetCode(LOG_PORT_EVENT):
            /* Reason code 8002: Repeated firmware error traps, fail interface */
            /* Reason code 303: Loop gone, fail port */
            /* Reason code 400: Loop back, unfail port */

            if (mleData->portEvent.proc == logporteventfe)
            {
                /* Send an interface failure firmware error */
                pFailurePacket = MallocWC(SIZEOF_IPC_INTERFACE_FAILURE);
                pFailurePacket->Type = IPC_FAILURE_TYPE_INTERFACE_FAILED;
                pFailurePacket->FailureData.InterfaceFailure.DetectedBySN = GetMyControllerSN();
                pFailurePacket->FailureData.InterfaceFailure.FailedInterfaceID = mleData->portEvent.port;
                pFailurePacket->FailureData.InterfaceFailure.ControllerSN = GetMyControllerSN();

                if (mleData->portEvent.reason == eclpup)
                {
                    pFailurePacket->FailureData.InterfaceFailure.InterfaceFailureType = INTERFACE_FAIL_OK;
                }
                else
                {
                    pFailurePacket->FailureData.InterfaceFailure.InterfaceFailureType = INTERFACE_FAIL_FIRMWARE_ERROR;
                }

                EnqueueToFailureManagerQueue(pFailurePacket, SIZEOF_IPC_INTERFACE_FAILURE);
                /* Don't release the failure packet, FailureManager owns it now */
            }
            break;

        case LOG_GetCode(LOG_BUFFER_BOARDS_ENABLED):
            BatteryHealthTaskStart(BATTERY_HEALTH_GOOD);
            break;

        case LOG_GetCode(LOG_BUFFER_BOARDS_DISABLED_INFO):
        case LOG_GetCode(LOG_BUFFER_BOARDS_DISABLED_WARN):
        case LOG_GetCode(LOG_BUFFER_BOARDS_DISABLED_ERROR):
        case LOG_GetCode(LOG_NV_MEM_EVENT):
            BatteryHealthTaskStart(BATTERY_HEALTH_FAIL);
            break;

        case LOG_GetCode(LOG_NEW_PATH):
            {
                LOG_NEW_PATH_PKT    *npp = (LOG_NEW_PATH_PKT *)logMRP->mleData;
                UINT32      sn;
                UINT8       iclFlag;

                iclFlag = npp->iclPathFlag;
                if (!iclFlag)
                {
                    sn = npp->controllerSN;

                    if (GetCommunicationsSlot(sn) != MAX_CONTROLLERS)
                    {
                        SM_RebuildHeartbeatListStart();
                    }
                }
            }
            break;

        case LOG_GetCode(LOG_PSD_REBUILD_DONE):
            if (mleData->psdRebuildDone.errorCode == 0)
            {
                LOG_SetDebug(logMRP->mleEvent);
            }
            break;

        case LOG_GetCode(LOG_HOTSPARE_DEPLETED):
            switch (mleData->hotspareDepleted.type)
            {
                case HSD_CNC_OK:
                case HSD_GEO_OK:
                case HSD_JUST_RIGHT:
                    LOG_SetInfo(logMRP->mleEvent);
                    break;
            }
            break;

        case LOG_GetCode(LOG_SINGLE_PATH):
#ifndef IGNORE_SINGLE_PATH
            AsyncGroupValidation(VAL_TYPE_STORAGE);
#endif  /* IGNORE_SINGLE_PATH */
            LOG_SetDebug(logMRP->mleEvent);
            break;

        case LOG_GetCode(LOG_CACHE_MIRROR_FAILED):
            /*
             * The mirror failed, break the mirror.
             */
            (void)TaskCreate(SM_HandleLostMirrorPartnerTask, NULL);

            /*
             * Get the controller node and failure state for the
             * controller to which we failed to mirror.
             */
            controllerNode = Qm_SlotFromSerial(mleData->cacheMirrorFailed.controllerSN);
            fd = EL_GetFailureState(controllerNode);

            /*
             * If that controller is in one of the inactive states
             * we want to change the log message to debug.  It is
             * an expected error and we don't want to alarm the user.
             */
            if (fd == FD_STATE_FIRMWARE_UPDATE_INACTIVE ||
                fd == FD_STATE_INACTIVATED)
            {
                LOG_SetDebug(logMRP->mleEvent);
            }
            break;

        case LOG_GetCode(LOG_ERR_TRAP):
            LogEvent(logMRP);

            /* Send a controller Failure */
            pFailurePacket = MallocWC(SIZEOF_IPC_CONTROLLER_FAILURE);
            pFailurePacket->Type = IPC_FAILURE_TYPE_CONTROLLER_FAILED;
            pFailurePacket->FailureData.ControllerFailure.DetectedBySN = GetMyControllerSN();
            pFailurePacket->FailureData.ControllerFailure.FailedControllerSN = GetMyControllerSN();

            EnqueueToFailureManagerQueue(pFailurePacket, SIZEOF_IPC_CONTROLLER_FAILURE);
            /* Don't release the failure packet, FailureManager owns it now */
            break;

            /*
             * Handle the Local Image Ready event.  The handler for this event
             * is required to be forked since an MRP may be called in response.
             * Since the task is forked we must make a copy of the data that
             * sent before returning.
             */
        case LOG_GetCode(LOG_LOCAL_IMAGE_READY):
            {
                UINT32      imageSize = 0;
                UINT32      eventSize = 0;
                IPC_BROADCAST *pBroadcast = NULL;

                /*
                 * Get the size of the image that was sent up from the PROC.
                 */
                imageSize = mleData->localImage.imageSize;
                ccb_assert(imageSize > 0, imageSize);

                /*
                 * We are going to create an event to be put on a queue
                 * so it will be processed by the forked task.  The event
                 * will be created as a BROADCAST event but will be
                 * placed on the local image queue.  In order to create
                 * the broadcast event we need to determine the size of
                 * the whole event.  This consists of the broadcast
                 * structure plus the image size.
                 */
                eventSize = sizeof(*pBroadcast) + imageSize;

                /*
                 * Allocate the event buffer.
                 */
                pBroadcast = MallocSharedWC(eventSize);

                /*
                 * Setup the broadcast event making sure that it is a
                 * local image event to be sent to the master with the
                 * image size and the image attached.
                 */
                pBroadcast->eventType = IPC_BROADCAST_PUTLCLIMAGE;
                pBroadcast->bcastType = IPC_BROADCAST_MASTER;
                pBroadcast->serialNum = 0;
                pBroadcast->dataSize = imageSize;

                /*
                 * The image starts in the second word of the log event.
                 */
                memcpy(&pBroadcast->data, mleData->localImage.pImage, imageSize);

//                LogMessage(LOG_TYPE_DEBUG, "LCLIMAGE AEH-Seq: 0x%x, Len: 0x%x, Ctrl: 0x%x, MP: 0x%x",
//                           ((UINT32 *)pBroadcast->data)[3], ((UINT32 *)pBroadcast->data)[0],
//                           ((UINT32 *)pBroadcast->data)[1], ((UINT32 *)pBroadcast->data)[2]);

                /*
                 * Push the event on the queue.
                 */
                EnqueueToBroadcastQueue(pBroadcast);

                /*
                 * Increment the local image count.
                 */
                gLocalImageCount++;

                logThisEvent = FALSE;
            }
            break;

        case LOG_GetCode(LOG_IPC_BROADCAST):
            {
                UINT32      eventSize = 0;
                IPC_BROADCAST *pBroadcast = NULL;

                /*
                 * Calculate the event size for this broadcast.  The
                 * data size in the MLE is the size of the data after
                 * the main IPC broadcast packet.
                 */
                eventSize = sizeof(*pBroadcast) + mleData->ipcBroadcast.dataSize;

                /*
                 * Allocate the event buffer.
                 */
                pBroadcast = MallocSharedWC(eventSize);

                /*
                 * The MLE broadcast event contains the same information
                 * as the IPC broadcast event (at least for the size of
                 * the IPC_BROADCAST packet) so copy the MLE data to the
                 * IPC data.
                 */
                memcpy(pBroadcast, &logMRP->mleData, sizeof(*pBroadcast));

                /*
                 * The MLE events have a maximum data size of 52 bytes,
                 * if the MLE BROADCAST data size is greater than that
                 * limit then the PROC will put a PCI address in the
                 * first word of the data area instead of the actual
                 * data.
                 *
                 * So, if > 52 bytes copy the data into the IPC packet
                 * using the PCI address.  Otherwise copy the data from
                 * the MLE packet.
                 */
                if (mleData->ipcBroadcast.dataSize > 52)
                {
                    memcpy(&pBroadcast->data,
                            (void *)(*(UINT32 *)&mleData->ipcBroadcast.data),
                           mleData->ipcBroadcast.dataSize);
                }
                else
                {
                    memcpy(&pBroadcast->data, &mleData->ipcBroadcast.data,
                           mleData->ipcBroadcast.dataSize);
                }

                /*
                 * Push the event on the queue.
                 */
                EnqueueToBroadcastQueue(pBroadcast);

                logThisEvent = FALSE;
            }
            break;

        case LOG_GetCode(LOG_NVRAM_RESTORE):
        case LOG_GetCode(LOG_NVRAM_CHKSUM_ERR):
        case LOG_GetCode(LOG_NVRAM_RELOAD):
        case LOG_GetCode(LOG_NVRAM_WRITE_FAIL):
            SetInitialNVRAM(mleEvent);
            break;

        case LOG_GetCode(LOG_LOG_FAILURE_EVENT_INFO):
        case LOG_GetCode(LOG_LOG_FAILURE_EVENT_WARN):
        case LOG_GetCode(LOG_LOG_FAILURE_EVENT):
            switch (mleData->failureEvent.action)
            {
                case FAILURE_EVENT_ACTION_ELECTION:
                case FAILURE_EVENT_ACTION_FORWARD_TO_MASTER:
                case FAILURE_EVENT_ACTION_DONT_HANDLE:
                case FAILURE_EVENT_ACTION_RESUBMIT:
                case FAILURE_EVENT_ACTION_DISCARDED:
                    LOG_SetDebug(logMRP->mleEvent);
                    break;
            }
            break;

            /* User intervention required */
        case LOG_GetCode(LOG_WC_SEQNO_BAD):
        case LOG_GetCode(LOG_WC_SN_VCG_BAD):
        case LOG_GetCode(LOG_WC_SN_BAD):
        case LOG_GetCode(LOG_WC_NVMEM_BAD):
            SetCacheErrorEvent(mleEvent);
            break;

            /*
             * FCAL health monitor async events
             */
        case LOG_GetCode(LOG_FRAMEDROPPED):
            /* Notify the FCAL health monitor - frame dropped */
            switch (mleData->portEvent.proc)
            {
            case 0:
                FCM_AddEvent(FCM_PROCESSOR_FE, mleData->portEvent.port,
                            FCM_ET_DROPPED_FRAME);
                break;

            default:
                FCM_AddEvent(FCM_PROCESSOR_BE, mleData->portEvent.port,
                            FCM_ET_DROPPED_FRAME);
                FCM_CountersMinorStorageEvent();
                break;
            }
            break;

        case LOG_GetCode(LOG_LIP):
            /* Look for only '8013' type LIP events */
            if ((mleData->portEvent.reason & 0x0000FFFF) == ASPLIP)
            {
                /* Notify the FCAL health monitor - loop reset */
                switch (mleData->portEvent.proc)
                {
                case 0:
                    FCM_AddEvent(FCM_PROCESSOR_FE, mleData->portEvent.port,
                            FCM_ET_LOOP_RESET);
                    break;

                default:
                    FCM_AddEvent(FCM_PROCESSOR_BE, mleData->portEvent.port,
                            FCM_ET_LOOP_RESET);
                    FCM_CountersMinorStorageEvent();
                    break;
                }
            }
            break;

        case LOG_GetCode(LOG_RSCN):
            /* Notify the FCAL health monitor - switch RSCN */
            switch (mleData->portEvent.proc)
            {
            case 0:
                /*
                 * Front end RSCN monitoring is disabled due to the
                 * servers generating RSCNs when they come online.
                 FCM_AddEvent(FCM_PROCESSOR_FE, mleData->portEvent.port,
                            FCM_ET_RSCN);
                 */
                break;

            default:
                //FCM_AddEvent(FCM_PROCESSOR_BE, mleData->portEvent.port, FCM_ET_RSCN);
                FCM_CountersMinorStorageEvent();
                break;
            }
            break;

        case LOG_GetCode(LOG_MIRROR_CAPABLE):
            /*
             * Re-establish the mirror partner
             */
            (void)TaskCreate(SM_RestoreMirrorPartnerTask, NULL);
            break;

        case LOG_GetCode(LOG_DEFRAG_VER_DONE):
            /*
             * Defrag verify operation complete.
             */
            gPDiskDefragVIP = mleData->defragVerDone.success;
            break;

        case LOG_GetCode(LOG_IPC_LINK_DOWN):
            /*
             * Get the controller node and failure state for the
             * controller to which the link failed.
             */
            controllerNode = Qm_SlotFromSerial(mleData->ipcLinkDown.DestinationSN);
            fd = EL_GetFailureState(controllerNode);

            /*
             * If that controller is in one of the inactive states
             * we want to change the log message to debug.  It is
             * an expected error and we don't want to alarm the user.
             */
            if (fd == FD_STATE_FIRMWARE_UPDATE_INACTIVE ||
                fd == FD_STATE_INACTIVATED)
            {
                LOG_SetDebug(logMRP->mleEvent);
            }
            break;

        case LOG_GetCode(LOG_ALL_DEV_MISSING):
            fprintf(stderr, "<GR><CCB-AsyncEv>Received the ALL DEV MISSING msg from PROC\n");
            /*
             * If there is more than one active controllers in the DSC
             * then the "all missing devices" error must attempt to
             * send messages to all other active controllers in the
             * group notifying them that this controller has lost all
             * of its devices.  The code that sent this log event still
             * handles the failure of the controller, this code simply
             * attempts to notify the rest of the DSC of the issue.
             */
            if (ACM_GetActiveControllerCount(Qm_ActiveCntlMapPtr()) > 1)
            {
                UINT32      eventSize = 0;
                UINT32      dataSize = 0;
                IPC_BROADCAST *pBroadcast = NULL;

                /*
                 * We are going to create an event to be put on a queue
                 * so it will be processed by the forked task.  The event
                 * will be created as a BROADCAST event.  In order to
                 * create the broadcast event we need to determine the
                 * size of the whole event.  This consists of the broadcast
                 * structure plus the size of the controller's serial
                 * number (UINT32) PLUS vdisk copy-mirror information.
                 */
                dataSize = sizeof(LOG_ALL_DEV_MISSING_DAT);
                eventSize = sizeof(*pBroadcast) + dataSize;

                /*
                 * Allocate the event buffer.
                 */
                pBroadcast = MallocSharedWC(eventSize);

                /*
                 * Setup the broadcast event making sure that it is a
                 * local image event to be sent to the master with the
                 * image size and the image attached.
                 */
                pBroadcast->eventType = IPC_BROADCAST_ALLDEVMISSING;
                pBroadcast->bcastType = IPC_BROADCAST_OTHERS;
                pBroadcast->serialNum = 0;
                pBroadcast->dataSize = dataSize;

                memcpy(pBroadcast->data, &logMRP->mleData[0], dataSize);
#if GR_GEORAID15_DEBUG
                {
                    LOG_ALL_DEV_MISSING_DAT *pDevMiss;
                    int         i;

                    pDevMiss = (LOG_ALL_DEV_MISSING_DAT *)pBroadcast->data;
                    UINT8      *pSyncVdMap = (UINT8 *)pDevMiss->syncVdMap;
                    UINT8      *pSrcVdMap = (UINT8 *)pDevMiss->srcVdMap;

                    fprintf(stderr, "<GR><AsyncEvent>Controller Serial No =%x\n", pDevMiss->cSerial);

                    for (i = 0; i < 64; (i = i + 16))
                    {
                        fprintf(stderr, "<GR><AsyncEv>DEST MAP %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x\n",
                                pSyncVdMap[i], pSyncVdMap[i + 1], pSyncVdMap[i + 2], pSyncVdMap[i + 3],
                                pSyncVdMap[i + 4], pSyncVdMap[i + 5], pSyncVdMap[i + 6], pSyncVdMap[i + 7],
                                pSyncVdMap[i + 8], pSyncVdMap[i + 9], pSyncVdMap[i + 10], pSyncVdMap[i + 11],
                                pSyncVdMap[i + 12], pSyncVdMap[i + 13], pSyncVdMap[i + 14], pSyncVdMap[i + 15]);
                    }

                    for (i = 0; i < 64; (i = i + 16))
                    {
                        fprintf(stderr, "<GR><AsyncEvent>SRC MAP %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x\n",
                                pSrcVdMap[i], pSrcVdMap[i + 1], pSrcVdMap[i + 2], pSrcVdMap[i + 3], pSrcVdMap[i + 4],
                                pSrcVdMap[i + 5], pSrcVdMap[i + 6], pSrcVdMap[i + 7], pSrcVdMap[i + 8],
                                pSrcVdMap[i + 9], pSrcVdMap[i + 10], pSrcVdMap[i + 11], pSrcVdMap[i + 12],
                                pSrcVdMap[i + 13], pSrcVdMap[i + 14], pSrcVdMap[i + 15]);
                    }
                }
#endif  /* GR_GEORAID15_DEBUG */

                fprintf(stderr, "<GR><CCB-AsyncEv>Sending ALL DEV MISSING msg+vdisk data from Cntrl# %x\n",
                        ((UINT32 *)pBroadcast->data)[0]);

                /*
                 * Push the event on the queue.
                 */
                AsyncEventBroadcast(pBroadcast);
            }
            break;

        case LOG_GetCode(LOG_SCSI_TIMEOUT):
            /*
             * If the completion function is the one logging this message
             * then change it to a debug message.  This does not need
             * to be seen by the customer.
             */
            if (mleData->SCSITimeout.flags == LOG_SCSI_TIMEOUT_FLAGS_COMP)
            {
                LOG_SetDebug(logMRP->mleEvent);
            }
            break;


        /*****************************************
        * BEGIN - Hardware monitor async events
        *   Connect to the INFO to determine when a specific piece of hardware
        *   is GOOD.  Connect to the WARNING or ERROR to determine when a
        *   piece of hardware is having difficulties.
        *****************************************/

        case LOG_GetCode(LOG_CCB_STATUS_INFO):
        case LOG_GetCode(LOG_CCB_STATUS_WARN):
        case LOG_GetCode(LOG_CCB_STATUS_ERROR):
        case LOG_GetCode(LOG_PROC_BOARD_STATUS_INFO):
        case LOG_GetCode(LOG_PROC_BOARD_STATUS_WARN):
        case LOG_GetCode(LOG_PROC_BOARD_STATUS_ERROR):
        case LOG_GetCode(LOG_FE_BUFFER_BOARD_STATUS_INFO):
        case LOG_GetCode(LOG_FE_BUFFER_BOARD_STATUS_WARN):
        case LOG_GetCode(LOG_FE_BUFFER_BOARD_STATUS_ERROR):
        case LOG_GetCode(LOG_BE_BUFFER_BOARD_STATUS_INFO):
        case LOG_GetCode(LOG_BE_BUFFER_BOARD_STATUS_WARN):
        case LOG_GetCode(LOG_BE_BUFFER_BOARD_STATUS_ERROR):
        case LOG_GetCode(LOG_FE_POWER_SUPPLY_STATUS_INFO):
        case LOG_GetCode(LOG_FE_POWER_SUPPLY_STATUS_WARN):
        case LOG_GetCode(LOG_FE_POWER_SUPPLY_STATUS_ERROR):
        case LOG_GetCode(LOG_BE_POWER_SUPPLY_STATUS_INFO):
        case LOG_GetCode(LOG_BE_POWER_SUPPLY_STATUS_WARN):
        case LOG_GetCode(LOG_BE_POWER_SUPPLY_STATUS_ERROR):
            break;

        case LOG_GetCode(LOG_VCG_SET_CACHE_OP):
            logThisEvent = FALSE;
            break;

        /*****************************************
        * END - Hardware monitor async events
        *****************************************/
            /*
             * HyperNode - IPMI interface
             */
        case LOG_GetCode(LOG_IPMI_EVENT):
            if (LOG_GetSev(mleEvent) == LOG_ERROR &&
                mleData->ipmiEvent.eventType == LOG_IPMI_EVENT_TYPE_INTERFACE)
            {
                /*
                 * This is a *fatal* IPMI interface error.  We've lost the
                 * ability to communicate over the IPMI interface, so the
                 * watchdog is probably going to expire.
                 */
                LogMessage(LOG_TYPE_DEBUG, "IPMI-ERROR: IPMI interface is broken!\n");
            }
            break;

        /*****************************************
        *
        * These events do not get logged!
        *
        *****************************************/

        case LOG_GetCode(LOG_SOCKET_ERROR):
        case LOG_GetCode(LOG_PULSE):
            logThisEvent = FALSE;
            break;

#if ISCSI_CODE
        case LOG_GetCode(LOG_SERVER_LOGGED_IN_OP):
        case LOG_GetCode(LOG_TARGET_UP_OP):
            break;

        case LOG_GetCode(LOG_ISCSI_GENERIC):
            dprintf(DPRINTF_PROC_PRINTF, "DEBUG_ISCSI: %s\n", (UINT8 *)mleData);
            logThisEvent = FALSE;
            break;
#endif  /* ISCSI_CODE */

        case LOG_GetCode(LOG_GR_EVENT):
        case LOG_GetCode(LOG_CM_EVENT):
        case LOG_GetCode(LOG_ICL_PORT_EVENT):
            break;

        case LOG_GetCode(LOG_PDATA_CREATE):
        case LOG_GetCode(LOG_PDATA_REMOVE):
        case LOG_GetCode(LOG_PDATA_WRITE):
            logThisEvent = FALSE;
            break;

        case LOG_GetCode(LOG_PRES_EVENT):
            SendPresPacketToMaster((LOG_PRES_EVENT_PKT *)logMRP);
            logThisEvent = FALSE;
            break;

        case LOG_GetCode(LOG_PRES_CHANGE):
            fprintf(stderr, "\nPRES: NVRAM Change intimation to slave\n");
            AsyncPresChange();
            logThisEvent = FALSE;
            break;

#if defined(MODEL_7000) || defined(MODEL_4700)
        case LOG_GetCode(LOG_ISE_ELEM_CHANGE):
            logThisEvent = FALSE;
            break;

        case LOG_GetCode(LOG_ISE_IP_DISCOVER):
            {
                LOG_ISE_IP_DISCOVER_PKT *dp = (LOG_ISE_IP_DISCOVER_PKT *)logMRP;

                IseUpdateIP(dp->data.bayid, dp->data.ip1, dp->data.ip2,
                        dp->data.wwn);
                logThisEvent = FALSE;
            }
            break;
#endif /* MODEL_7000 || MODEL_4700 */
        }
    }

    /* Notify X1 service of any change events */
    SendX1AsyncNotifications(logMRP);

    /*
     * Log everything, unless we decided not to in the case above.
     */
    if (logThisEvent && LOG_GetCode(logMRP->mleEvent) != LOG_GetCode(LOG_ERR_TRAP))
    {
        LogEvent(logMRP);
    }
}


/**
******************************************************************************
**
**  @brief  Generates and sends an asynchronous event message
**
**  Creates an MRP type structure and semds it out.
**
**  @param  eventType    - type of event
**  @param  dataLen      - length of parameter data in bytes
**  @param  data         - pointer to parameter data
**
******************************************************************************
**/
void SendAsyncEvent(INT32 eventType, INT32 dataLen, void *data)
{
    LOG_MRP     message;
    int         maxLen = sizeof(message.mleData);

    /*
     * Construct the event message
     */
    message.mleEvent = eventType;
    message.mleLength = dataLen;

    if (dataLen)
    {
        memcpy(message.mleData, data, (dataLen > maxLen) ? maxLen : dataLen);
    }

    AsyncEventHandler(NULL, &message);  /* Send the asynchronous event */
}


/**
******************************************************************************
**
**  @brief  Push a new server event on the new server queue
**
**  This function will push a new server event on the new server queue.  If
**  this is the first time this function has been called it will create the
**  queue and start the task that processes the events.  If it is not the first
**  time being called it will push the event on the queue and determine if the
**  task is already running.  If the task is not running it will start the
**  task.
**
**  @param  pNewServer - New Server to be placed on the queue
**
**  @return none
**
******************************************************************************
**/
static void EnqueueToNewServerQueue(NEW_SERVER *pNewServer)
{
    /*
     * If the Async Event event list has not yet been created, create it.
     */
    if (ptrNewServerQueue == NULL)
    {
        ptrNewServerQueue = CreateList();
    }

    /*
     * Enqueue this async event on the event list so it will be
     * processed by the ProcessAsyncEvent task.
     */
    Enqueue(ptrNewServerQueue, pNewServer);

    /*
     * If the task that processes async events from the queue is not
     * active, activate it.
     */
    if (!bTaskActiveNewServer)
    {
        bTaskActiveNewServer = 1;
        (void)TaskCreate(ProcessNewServerQueue, NULL);
    }

    dprintf(DPRINTF_ASYNC_EVENT_HANDLER, "%s: Reset task delay\n", __func__);

    gNewServerTaskCounter = NEW_SERVER_TASK_DELAY;
}


/**
******************************************************************************
**
**  @brief  This method will handle any new servers placed in the queue
**
**  @param  dummy - placeholder required for forked task
**
**  @attention  This method is forked by the EnqueueToNewServerQueue when a new
**          event is put on queue and the task is not currently active.
**
******************************************************************************
**/
static void ProcessNewServerQueue(UNUSED TASK_PARMS *parms)
{
    NEW_SERVER *pNewServer;

    dprintf(DPRINTF_ASYNC_EVENT_HANDLER, "%s: ENTER\n", __func__);

    ccb_assert(ptrNewServerQueue != NULL, ptrNewServerQueue);

    /*
     * Hold off creating this server until power-up has completed.
     */
    while (!PowerUpComplete())
    {
        /*
         * Delay 1 second before checking if power-up has completed.
         */
        TaskSleepMS(1000);
    }

    dprintf(DPRINTF_ASYNC_EVENT_HANDLER, "%s: Items on Queue: 0x%x\n",
            __func__, NumberOfItems(ptrNewServerQueue));

    /*
     * While there are servers to process continue looping.
     */
    while (NumberOfItems(ptrNewServerQueue) > 0)
    {
        /*
         * If an election or reallocation is in progress we will hold
         * off processing the create server requests.
         */
        if (EL_TestInProgress() == TRUE || (RMGetState() != RMRUNNING && RMGetState() != RMDOWN))
        {
            dprintf(DPRINTF_ASYNC_EVENT_HANDLER, "%s: Reset task delay\n",
                    __func__);

            /*
             * Reset the task counter to ensure that we wait at least
             * that delay from now before processing any requests.
             */
            gNewServerTaskCounter = NEW_SERVER_TASK_DELAY;
        }

        /*
         * If the task delay counter has not reached zero, decrement
         * the counter, sleep for a second and continue the while
         * loop.
         */
        if (gNewServerTaskCounter > 0)
        {
            --gNewServerTaskCounter;    /* Decrement the task counter */

            TaskSleepMS(1000);          /* Sleep for a second */

            /*
             * Continue back to the top of the while loop to
             * see if there are still items on the queue to
             * process.
             */
            continue;
        }

        dprintf(DPRINTF_ASYNC_EVENT_HANDLER, "%s: Items on Queue: 0x%x\n",
                __func__, NumberOfItems(ptrNewServerQueue));

        /*
         * Get the next event off the queue.
         */
        pNewServer = (NEW_SERVER *)Dequeue(ptrNewServerQueue);

        /*
         * Send the create server request to the master controller.
         */
        CreateServer(Qm_GetMasterControllerSN(), pNewServer);

        /*
         * pNewServer can be freed since it was copied into the request packet.
         */
        Free(pNewServer);

        TaskSwitch();

        /*
         * Go back to the start of the WHILE loop that checks if there are
         * more items in the list to be processed.
         */
    }

    /*
     * There are no more items to be processed at this time, shutdown the
     * task (flag it as inactive).
     */
    bTaskActiveNewServer = 0;

    dprintf(DPRINTF_ASYNC_EVENT_HANDLER, "%s: EXIT\n", __func__);
}


/**
******************************************************************************
**
**  @brief  Issue a Create Server MRP through the command layers.
**
**  This function is used when this controller is the master.
**
**  @param  controllerSN - Controller to execute the create server request
**  @param  pNewServer - async event information provided
**                                       by the front end proc that defines
**                                       the new server.
**
**  @return PI_GOOD or PI_ERROR
**
******************************************************************************
**/
static void CreateServer(UINT32 controllerSN, NEW_SERVER *pNewServer)
{
    XIO_PACKET            reqPacket;
    XIO_PACKET            rspPacket;
    PI_SERVER_CREATE_REQ *sc;
    INT32                 rc = GOOD;

    /*
     * Allocate memory for the request and response headers and the
     * request data.  Response data space is allocated at a lower level.
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    reqPacket.pPacket = MallocSharedWC(sizeof(PI_SERVER_CREATE_REQ));
    rspPacket.pPacket = NULL;

    sc = (PI_SERVER_CREATE_REQ *)reqPacket.pPacket;

    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    /*
     * Fill in the request structure and issue the request through
     * the command layers.
     */
    reqPacket.pHeader->commandCode = PI_SERVER_CREATE_CMD;
    reqPacket.pHeader->length = sizeof(PI_SERVER_CREATE_REQ);

    sc->targetId = pNewServer->targetId;
    sc->owner = pNewServer->owner;
    sc->wwn = pNewServer->wwn;
    memcpy(sc->i_name, pNewServer->i_name, 254);

    /*
     * If the create server request is for this controller make
     * the request to the port server directly.  If it is for
     * one of the slave controllers then tunnel the request to
     * that controller.
     */
    if (controllerSN == GetMyControllerSN())
    {
        /*
         * Issue the command through the top-level command handler.
         * Validate the ports and generate a port bit map to be used later.
         */
        rc = PortServerCommandHandler(&reqPacket, &rspPacket);
    }
    else
    {
        UINT8   retries = 2;        /* Ethernet, Fiber(1), Disk Quorum(2) */

        do
        {
            if (rc != PI_TIMEOUT)
            {
                Free(rspPacket.pPacket);
            }
            else
            {
                rspPacket.pPacket = NULL;
            }
            rc = TunnelRequest(controllerSN, &reqPacket, &rspPacket);
        } while (rc != GOOD && (retries--) > 0);
    }

    if (rc != PI_GOOD)
    {
        dprintf(DPRINTF_ASYNC_EVENT_HANDLER, "%s: Failed to create server (rc: 0x%x)\n",
                __func__, rc);
    }
    else
    {
        /*
         * Only if the server did not previously exist do we want
         * to print out that the server was created.
         */
        if (((PI_SERVER_CREATE_RSP *)(rspPacket.pPacket))->flags == false)
        {
            /*
             * Server was created successfully.  We probably want to log
             * a message to indicate this.
             */
            dprintf(DPRINTF_ASYNC_EVENT_HANDLER, "%s: Server created (WWN: %8.8x%8.8x, TID: 0x%x).\n",
                    __func__,
                    bswap_32((UINT32)(pNewServer->wwn)),
                    bswap_32((UINT32)(pNewServer->wwn >> 32)),
                    pNewServer->targetId);
        }
        else
        {
            dprintf(DPRINTF_ASYNC_EVENT_HANDLER, "%s: Server already exists "
                    "(WWN: %8.8x%8.8x, TID: 0x%x).\n", __func__,
                    bswap_32((UINT32)(pNewServer->wwn)),
                    bswap_32((UINT32)(pNewServer->wwn >> 32)),
                    pNewServer->targetId);
        }
    }

    /* Free the allocated memory */
    Free(reqPacket.pHeader);
    Free(reqPacket.pPacket);
    Free(rspPacket.pHeader);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPacket.pPacket);
    }
}


/**
******************************************************************************
**
**  @brief      This function handles the LOG_DEVICE_FAIL_HS messages by
**              forking off a single task that will send the configuration
**              update to the slave controllers.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void AsyncEventDeviceFailHS(void)
{
    bDeviceFailContinue = true;

    if (pcbDeviceFailTask == NULL)
    {
        /*
         * Create a task to send the configuration update
         * task to the slave controllers.
         */
        pcbDeviceFailTask = TaskCreate(AsyncEventDeviceFailHSTask, NULL);
    }
}


/**
******************************************************************************
**
**  @brief      This function is forked as a task to handle the processing
**              required for a LOG_DEVICE_FAIL_HS message.
**
**  @param      UINT32 dummy - placeholder required for forked task.
**
**  @return     none
**
**  @attention  FORKED
**
******************************************************************************
**/
static void AsyncEventDeviceFailHSTask(UNUSED TASK_PARMS *parms)
{
    TASK_PARMS  taskParms;
    const char *tkSnpshtTskHtSpr = "AUTO - Hotspare";

    /*
     * Continue processing device failures until they have stopped
     * coming as log events.
     */
    while (bDeviceFailContinue)
    {
        /*
         * Since we are going to process this device failure, set the
         * flag to not continue.  If another device failure occurs
         * the flag will get set and the loop will continue.
         */
        bDeviceFailContinue = false;

        /*
         * Send the configuration update to the slave controllers.
         */
        RMSlavesConfigurationUpdate(X1_ASYNC_DEVICES_CHANGED, TRUE);

        /*
         * Capture configuration at this point
         */
        taskParms.p1 = (UINT32)SNAPSHOT_TYPE_HOTSPARE;
        taskParms.p2 = (UINT32)tkSnpshtTskHtSpr;
        (void)TaskCreate(TakeSnapshotTask, &taskParms);

        /*
         * Give up the processor to see if something else will change the
         * flag that determines if we need to continue processing device
         * failures.
         */
        TaskSwitch();
    }

    /*
     * The task is ending so clear out the PCB variable.
     */
    pcbDeviceFailTask = NULL;
}


/**
******************************************************************************
**
**  @brief      This function terminates the broadcast events by
**              setting the gbProcessBroadcast flag to false.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void TerminateBroadcasts(void)
{
    gbProcessBroadcast = false;
}


/*----------------------------------------------------------------------------
** Function Name: EnqueueToBroadcastQueue()
**
** Inputs:  IPC_BROADCAST* pBroadcast - Event to be placed on the queue.
**
**--------------------------------------------------------------------------*/
static void EnqueueToBroadcastQueue(IPC_BROADCAST *pBroadcast)
{
    S_LIST     *pList;
    PCB        *pPCB;

    /*
     * If the Broadcast event list has not yet been created, create it.
     */
    if (ptrBroadcastQueue == NULL)
    {
        ptrBroadcastQueue = CreateList();
    }

    /*
     * If the Datagram Broadcast event list has not yet been created, create it.
     */
    if (ptrBroadcastDGQueue == NULL)
    {
        ptrBroadcastDGQueue = CreateList();
    }

    /*
     * Setup the list and PCB pointers so the event gets placed on the
     * correct queue and the correct task is made ready.
     */
    if (pBroadcast->eventType == IPC_BROADCAST_PUTDG)
    {
        /*
         * if the task that processes broadcast events from the queue
         * has not been created, create it
         */
        if (pcbBroadcastDGTask == NULL)
        {
            pcbBroadcastDGTask = TaskCreate(ProcessBroadcastDGQueueTask, NULL);
            TaskSetState(pcbBroadcastDGTask, PCB_NOT_READY);
        }

        pList = ptrBroadcastDGQueue;
        pPCB = pcbBroadcastDGTask;

        LogMessage(LOG_TYPE_DEBUG, "DG ENQ-#: 0x%x, SendSN: 0x%x, Type: 0x%x, FC: 0x%x",
                   NumberOfItems(pList),
                   PUTDG_SN(pBroadcast), PUTDG_TYPE(pBroadcast), PUTDG_FC(pBroadcast));

        dumpByteArray2(DPRINTF_DEFAULT, "DG ENQ DUMP", pBroadcast->data, pBroadcast->dataSize);
    }
    else
    {
        /*
         * if the task that processes broadcast events from the queue
         * has not been created, create it
         */
        if (pcbBroadcastTask == NULL)
        {
            pcbBroadcastTask = TaskCreate(ProcessBroadcastQueueTask, NULL);
            TaskSetState(pcbBroadcastTask, PCB_NOT_READY);
        }

        pList = ptrBroadcastQueue;
        pPCB = pcbBroadcastTask;

//        LogMessage(LOG_TYPE_DEBUG, "BC ENQ-#: 0x%x, et: 0x%x, bt: 0x%x, sn: 0x%x, ds: 0x%x",
//                   NumberOfItems(pList), pBroadcast->eventType,
//                   pBroadcast->bcastType, pBroadcast->serialNum, pBroadcast->dataSize);
    }

    /*
     * Enqueue this event on the event list so it will be
     * processed by the task.
     */
    Enqueue(pList, pBroadcast);

    /*
     * Make sure the task is runnable
     */
    if (TaskGetState(pPCB) != PCB_READY)
    {
        TaskReadyState(pPCB);
    }
}

/*----------------------------------------------------------------------------
** Name:    ProcessBroadcastQueueTask - FORKED
**
** Desc:    This method will handle any broadcast event placed in the queue.
**
** Inputs:  UINT32 dummy - placeholder required for forked task
**
** WARNING: This method is forked by the EnqueueToBroadcastQueue when a new
**          event is put on queue and the task is not currently active.
**--------------------------------------------------------------------------*/
static NORETURN void ProcessBroadcastQueueTask(UNUSED TASK_PARMS *parms)
{
    IPC_BROADCAST *pBroadcastCurrent = NULL;
    S_LIST     *ptrBatchQueue = NULL;

    UINT16      eventType = 0;  /* Event to broadcast */
    UINT16      bcastType = 0;  /* Type of broadcast */
    UINT32      serialNum = 0;  /* Serial number of a specific controller */
    UINT32      dataSize = 0;   /* Size of the following data */

//    dprintf(DPRINTF_ASYNC_EVENT_HANDLER, "ProcessBroadcastQueueTask - Starting\n");

    ccb_assert(ptrBroadcastQueue != NULL, ptrBroadcastQueue);

    /*
     * Hold off this broadcast until power-up has completed.
     */
    while (!PowerUpAllBEReady())
    {
        /*
         * Delay 1 second before checking if power-up has completed.
         */
        TaskSleepMS(1000);
    }

    /*
     * Create the batch request queue.
     */
    ptrBatchQueue = CreateList();

    while (FOREVER)
    {
        /*
         * Check if we are supposed to be processing broadcast events.
         */
        if (!gbProcessBroadcast)
        {
            /*
             * Delay 5 second(s).
             */
            TaskSleepMS(5000);

            /*
             * Go back to the start of the while loop to process more
             * requests.
             */
            continue;
        }

        /*
         * If this controller is either inactivated or FW update
         * inactivated it should not be processing broadcast
         * events.
         */
        if (GetControllerFailureState() == FD_STATE_INACTIVATED ||
            GetControllerFailureState() == FD_STATE_FIRMWARE_UPDATE_INACTIVE)
        {
            /*
             * Delay 5 second(s).
             */
            TaskSleepMS(5000);

            /*
             * Go back to the start of the while loop to process more
             * requests, eventually the election will be complete and
             * the requests can be processed.
             */
            continue;
        }

//        dprintf(DPRINTF_ASYNC_EVENT_HANDLER, "ProcessBroadcastQueueTask - Items on Queue: 0x%x\n",
//                NumberOfItems(ptrBroadcastQueue));

        /*
         * If the requests to the BE are currently blocked or an election
         * is in progress we will hold off processing since the master
         * controller may change.
         */
        if (BEBlocked() || EL_TestInProgress() == TRUE)
        {
            /*
             * Delay .5 second(s).
             */
            TaskSleepMS(500);

            /*
             * Go back to the start of the while loop to process more
             * requests, eventually the election will be complete and
             * the requests can be processed.
             */
            continue;
        }

        /*
         * Get the first event in the list so we can process it.
         */
        pBroadcastCurrent = (IPC_BROADCAST *)Dequeue(ptrBroadcastQueue);

        /*
         * If this is a local image and there are more local images
         * to be processed, throw it away.
         */
        if (pBroadcastCurrent->eventType == IPC_BROADCAST_PUTLCLIMAGE)
        {
            gLocalImageCount--;

            if (gLocalImageCount > 0)
            {
                LogMessage(LOG_TYPE_DEBUG, "LCLIMAGE TRW-Seq: 0x%x, Len: 0x%x, Ctrl: 0x%x, MP: 0x%x",
                           PUTLCLIMAGE_SEQ(pBroadcastCurrent), PUTLCLIMAGE_LEN(pBroadcastCurrent),
                           PUTLCLIMAGE_SN(pBroadcastCurrent), PUTLCLIMAGE_MP(pBroadcastCurrent));

                Free(pBroadcastCurrent);
                continue;
            }
        }

        /*
         * If there are no items currently in the batch, put this one in.  If
         * not we will need to check if this "current" event is part of the
         * existing batch.  If it is part of the existing batch we can simply
         * add it to the batch and proceed to the next one.  If it is not part
         * of the batch we will first process the existing batch and then
         * start a new batch with the "current" event as the only item.
         */
        if (NumberOfItems(ptrBatchQueue) == 0)
        {
            /*
             * save the valuable information and queue up the event.
             */
            eventType = pBroadcastCurrent->eventType;
            bcastType = pBroadcastCurrent->bcastType;
            serialNum = pBroadcastCurrent->serialNum;
            dataSize = pBroadcastCurrent->dataSize;
            Enqueue(ptrBatchQueue, pBroadcastCurrent);
        }
        else
        {
            /*
             * We currently only support batching up PUTSOS events.  If this
             * is one of those events and matches the broadcast type and
             * serial number we want to add it to the current batch.  If it
             * is not part of the batch we will first process the current
             * batch and then start a new batch with this "current" broadcast
             * event.
             *
             * NOTE: If the current batch is not of PUTSOS type there is still
             *       a queue of sorts, but there will only be one item in the
             *       queue.
             */
            if ((eventType == IPC_BROADCAST_PUTSOS || eventType == IPC_BROADCAST_PUTLCLIMAGE) &&
                pBroadcastCurrent->eventType == eventType && pBroadcastCurrent->bcastType == bcastType &&
                pBroadcastCurrent->serialNum == serialNum)
            {
                /* Increment the data size to include this event */
                dataSize += pBroadcastCurrent->dataSize;

                Enqueue(ptrBatchQueue, pBroadcastCurrent);
            }
            else
            {
                /*
                 * Process the existing batch broadcast
                 */
                ProcessBatchBroadcast(ptrBatchQueue, eventType, bcastType, serialNum, dataSize);

                /*
                 * Give control back to the kernel, we learned to share very early
                 */
                TaskSwitch();

                /*
                 * Since we have a current broadcast event that was dequeued
                 * and was not processed since it was not part of the batch
                 * broadcast we just completed we will start a new batch.  So,
                 * save the valuable information and queue up the event.
                 */
                eventType = pBroadcastCurrent->eventType;
                bcastType = pBroadcastCurrent->bcastType;
                serialNum = pBroadcastCurrent->serialNum;
                dataSize = pBroadcastCurrent->dataSize;
                Enqueue(ptrBatchQueue, pBroadcastCurrent);
            }
        }

        /*
         * If there are no more items in the broadcast queue, process what
         * we have in the batch queue and then continue.
         *
         * NOTE: We will go back to the top of the while loop and check
         *       if anything was added to the broadcast queue while we
         *       were processing the batch queue.
         */
        if (NumberOfItems(ptrBroadcastQueue) == 0)
        {
            /*
             * Process the existing batch broadcast
             */
            ProcessBatchBroadcast(ptrBatchQueue, eventType, bcastType, serialNum, dataSize);

            /*
             * Give control back to the kernel, we learned to share very early
             */
            TaskSwitch();
        }

        /*
         * exchange now or if no more work, put the task to sleep
         */
        if (NumberOfItems(ptrBroadcastQueue) == 0)
        {
            TaskSetState(pcbBroadcastTask, PCB_NOT_READY);
        }

        /*
         * Give control back to the kernel, we learned to share very early
         */
        TaskSwitch();

        /*
         * Go back to the start of the WHILE loop that checks if there are
         * more items in the list to be processed.
         */
    }

    /*
     * We should never exit the above loop but just in case we
     * do we should free the batch queue.
     */
    Free(ptrBatchQueue);
}

/**
******************************************************************************
**
**  @brief      This method will compare two datagram broadcast events and
**              determine if they are the same (just check pointer values,
**              not content).
**
**  @param      void* e1 - First broadcast event
**
**  @param      void* e2 - Second broadcast event
**
**  @return     0 if the events are the same
**              1 if the events are different
**
******************************************************************************
**/
static int compareBroadcastDG(void const *e1, void const *e2)
{
    int         rc;
    IPC_BROADCAST *pE1 = (IPC_BROADCAST *)e1;
    IPC_BROADCAST *pE2 = (IPC_BROADCAST *)e2;

    if (pE1->bcastType == pE2->bcastType &&
        (pE1->bcastType != IPC_BROADCAST_SPECIFIC || pE1->serialNum == pE2->serialNum) &&
        PUTDG_TYPE(pE1) == PUTDG_CCBG)
    {
        rc = 0;
    }
    else
    {
        rc = 1;
    }

    return rc;
}


/**
******************************************************************************
**
**  @brief      This method will lock or unlock the configuration update
**              mutex as necessary.  The mutex is locked when the 5/81
**              (CCBGram/Disable NVRAM Refresh) message is processed and
**              is unlocked when the 5/83 (CCBGram/Enable NVRAM Refresh)
**              message is processed.
**
**  @param      IPC_BROADCAST* pBroadcast - Pointer to this broadcast message
**
**  @return     none
**
******************************************************************************
**/
static void ProcessBroadcastDGConfigLock(IPC_BROADCAST *pBroadcast)
{
    /*
     * Make sure these checks for locking/unlocking the configuration
     * mutex only occur on the master controller.
     */
    if (TestforMaster(GetMyControllerSN()))
    {
        /*
         * If the event is a CCBGram event with a function code of
         * 0x81 (disable nvram refresh) lock the configuration update
         * mutex to ensure no configurations occur while the swap
         * operation completes.
         *
         * If the event is a CCBGram event with a function code of
         * 0x83 (enable nvram refresh) unlock the configuration update
         * mutex since the swap operation has completed.
         */
        if (PUTDG_TYPE(pBroadcast) == PUTDG_CCBG && PUTDG_FC(pBroadcast) == 0x81)
        {
            LogMessage(LOG_TYPE_DEBUG, "ProcessBroadcastDGConfigLock-Lock config for raid swap");

            /*
             * Temporarily disable caching to allow configuration updates to
             * proceed a little smoother.  This will also wait for the cache
             * to be flushed before continuing.
             */

            /*
             * Do temp disable the cache when user swap command is issued
             * This is to avoid when Auto Swap is in progress. This is because
             * there is no point in flushing the cache when all the source
             * VDISKS are gone
             */
            if (PUTDG_USERSWAP(pBroadcast) == TRUE)
            {
                (void)RMTempDisableCache(PI_MISC_SETTDISCACHE_CMD, TEMP_DISABLE_BROADCAST, 0);
                RMWaitForCacheFlush();
            }

            /*
             * Aquire the mutex for the configuration update and hold until the
             * request has been fulfilled.
             */
            (void)LockMutex(&configUpdateMutex, MUTEX_WAIT);
        }
        else if (PUTDG_TYPE(pBroadcast) == PUTDG_CCBG && PUTDG_FC(pBroadcast) == 0x83)
        {
            LogMessage(LOG_TYPE_DEBUG, "ProcessBroadcastDGConfigLock-Raid swap complete, refresh NVRAM and unlock config");

            /*
             * Signal the slaves to do a configuration update.  The TRUE
             * indicates that only a refresh is to be done.
             */
            RMSlavesRefreshNVRAM(X1_ASYNC_PCHANGED | X1_ASYNC_RCHANGED);

            /*
             * Send a notification of the configuration change to the XIOservice.
             *
             * A local image from another controller will only update physical
             * and raid information so we just need to send the PCHANGED and
             * RCHANGED events.
             */
            SendX1ChangeEvent(X1_ASYNC_PCHANGED | X1_ASYNC_RCHANGED);

            /*
             * Unlock the configuration update mutex.
             */
            UnlockMutex(&configUpdateMutex);

            /*
             * Clear temp disable the cache when user swap command is issued
             * This is to avoid when Auto Swap is in progress. This is because
             * there is no point in flushing the cache when all the source
             * VDISKS are gone
             */
            if (PUTDG_USERSWAP(pBroadcast) == TRUE)
            {
                /*
                 * Re-enable caching since we have completed the configuration
                 * update.
                 */
                (void)RMTempDisableCache(PI_MISC_CLRTDISCACHE_CMD,
                                         TEMP_DISABLE_BROADCAST, T_DISABLE_CLEAR_ONE);
            }

            LogMessage(LOG_TYPE_DEBUG, "ProcessBroadcastDGConfigLock-Config unlocked after raid swap");
        }
    }
}


/*----------------------------------------------------------------------------
** Name:    ProcessBroadcastDGQueueTask - FORKED
**
** Desc:    This method will handle datagram broadcast events placed
**          in the queue.
**
** Inputs:  UINT32 dummy - placeholder required for forked task
**
** WARNING: This method is forked by the EnqueueToBroadcastQueue when a new
**          event is put on queue and the task is not currently active.
**--------------------------------------------------------------------------*/
static NORETURN void ProcessBroadcastDGQueueTask(UNUSED TASK_PARMS *parms)      /* FORKED */
{
    IPC_BROADCAST *pBroadcastCurrent;
    IPC_BROADCAST *pBroadcastSaved = NULL;
    S_LIST     *ptrBatchQueue;
    UINT16      eventType = 0;  /* Event to broadcast */
    UINT16      bcastType = 0;  /* Type of broadcast */
    UINT32      serialNum = 0;  /* Serial number of a specific controller */
    UINT32      dataSize = 0;   /* Size of the following data */

    dprintf(DPRINTF_ASYNC_EVENT_HANDLER, "ProcessBroadcastDGQueueTask - Starting\n");

    ccb_assert(ptrBroadcastDGQueue != NULL, ptrBroadcastDGQueue);

    /*
     * Hold off this broadcast until power-up has completed.
     */
    while (!PowerUpAllBEReady())
    {
        /*
         * Delay 1 second before checking if power-up has completed.
         */
        TaskSleepMS(1000);
    }

    /*
     * Create the batch request queue.
     */
    ptrBatchQueue = CreateList();

    while (FOREVER)
    {
        /*
         * Check if we are supposed to be processing broadcast events.
         */
        if (!gbProcessBroadcast)
        {
            /*
             * Delay 5 second(s).
             */
            TaskSleepMS(5000);

            /*
             * Go back to the start of the while loop to process more
             * requests.
             */
            continue;
        }

        dprintf(DPRINTF_ASYNC_EVENT_HANDLER, "ProcessBroadcastDGQueueTask - Items on Queue: 0x%x\n",
                NumberOfItems(ptrBroadcastDGQueue));

        /*
         * If the requests to the BE are currently blocked or an election
         * is in progress we will hold off processing since the master
         * controller may change.
         */
        if (BEBlocked() || EL_TestInProgress() == TRUE)
        {
            /*
             * Delay .5 second(s).
             */
            TaskSleepMS(500);

            /*
             * Go back to the start of the while loop to process more
             * requests, eventually the election will be complete and
             * the requests can be processed.
             */
            continue;
        }

        /*
         * If there are no items currently in the batch, get the first one
         * and add it to the batch.
         *
         * If not we will need to check if this "current" event is part of the
         * existing batch.  If it is part of the existing batch we can simply
         * add it to the batch and proceed to the next one.  If it is not part
         * of the batch we will first process the existing batch and then
         * start a new batch with the "current" event as the only item.
         */
        if (NumberOfItems(ptrBatchQueue) == 0)
        {
            /*
             * Get the first event in the list so we can process it.
             */
            pBroadcastCurrent = (IPC_BROADCAST *)Dequeue(ptrBroadcastDGQueue);
            pBroadcastSaved = pBroadcastCurrent;

            /*
             * Lock/unlock the configuration update mutex as necessary
             * based on the broadcast event.  Only some events cause the
             * lock or unlock (see function).
             */
            ProcessBroadcastDGConfigLock(pBroadcastCurrent);

            /*
             * save the valuable information and queue up the event.
             */
            eventType = pBroadcastCurrent->eventType;
            bcastType = pBroadcastCurrent->bcastType;
            serialNum = pBroadcastCurrent->serialNum;
            dataSize = pBroadcastCurrent->dataSize;
            Enqueue(ptrBatchQueue, pBroadcastCurrent);
        }
        else
        {
            /*
             * Attempt to find a matching event in the queue as the saved
             * event.  This will compare teh saved event with the events
             * in the queue and see if there is one that is appropriate
             * to add to the current batch.  If one is found it will
             * return that event otherwise it will return NULL.
             */
            pBroadcastCurrent = (IPC_BROADCAST *)RemoveElement(ptrBroadcastDGQueue, pBroadcastSaved,
                                                               compareBroadcastDG);

            /*
             * If the remove element function found an appropriate event
             * to take off the queue, add it to the current batch.
             *
             * If an appropriate event was not found, process the current
             * batch and start the next batch by pulling the first element
             * off the queue.
             */
            if (pBroadcastCurrent)
            {
                /*
                 * Lock/unlock the configuration update mutex as necessary
                 * based on the broadcast event.  Only some events cause the
                 * lock or unlock (see function).
                 */
                ProcessBroadcastDGConfigLock(pBroadcastCurrent);

                /* Increment the data size to include this event */
                dataSize += pBroadcastCurrent->dataSize;

                Enqueue(ptrBatchQueue, pBroadcastCurrent);
            }
            else
            {
                /*
                 * Process the existing batch broadcast
                 */
                ProcessBatchBroadcast(ptrBatchQueue, eventType, bcastType, serialNum, dataSize);
            }
        }

        /*
         * If there are no more items in the broadcast queue, process what
         * we have in the batch queue and then continue.
         *
         * NOTE: We will go back to the top of the while loop and check
         *       if anything was added to the broadcast queue while we
         *       were processing the batch queue.
         */
        if (NumberOfItems(ptrBroadcastDGQueue) == 0)
        {
            /*
             * Process the existing batch broadcast
             */
            ProcessBatchBroadcast(ptrBatchQueue, eventType, bcastType, serialNum, dataSize);

            /*
             * Give control back to the kernel, we learned to share very early
             */
            TaskSwitch();
        }

        /*
         * exchange now or if no more work, put the task to sleep
         */
        if (NumberOfItems(ptrBroadcastDGQueue) == 0)
        {
            TaskSetState(pcbBroadcastDGTask, PCB_NOT_READY);
        }

        /*
         * Give control back to the kernel, we learned to share very early
         */
        TaskSwitch();

        /*
         * Go back to the start of the WHILE loop that checks if there are
         * more items in the list to be processed.
         */
    }
}

/*----------------------------------------------------------------------------
** Name:    ProcessBatchBroadcast
**
** Desc:    This method will process a batch process by creating a new
**          IPC_BROADCAST event containing the combined data from all the
**          broadcast events in the queue.
**
** Inputs:  S_LIST *ptrBatchQueue - Batch broadcast event queue
**          UINT16 eventType - Broadcast event type
**          UINT16 bcastType - Broadcast type, who to sent this to
**          UINT32 serialNum - Controller to send to specifically
**          UINT32 dataSize - Combined size of the data for all events in the
**                            broadcast queue.
**
**--------------------------------------------------------------------------*/
static void ProcessBatchBroadcast(S_LIST *ptrBatchQueue, UINT16 eventType,
                                  UINT16 bcastType, UINT32 serialNum, UINT32 dataSize)
{
    IPC_BROADCAST *pBatchElement = NULL;
    IPC_BROADCAST *pBatchBroadcast = NULL;
    UINT8      *pBatchData = NULL;

    /*
     * Loop until there are no more events to process in the
     * batch queue.
     */
    while (NumberOfItems(ptrBatchQueue) > 0)
    {
//        dprintf(DPRINTF_ASYNC_EVENT_HANDLER, "ProcessBatchBroadcast: Processing batch (#: 0x%x, et: 0x%x, bt: 0x%x, sn: 0x%x, ds: 0x%x).\n",
//                NumberOfItems(ptrBatchQueue), eventType, bcastType, serialNum, dataSize);

        /*
         * If this is a PUTSOS or PUTDG event we can attempt to combine
         * all of the events together to make one large request with
         * all of the information.
         *
         * NOTE: Only attempt to combine if there is more than one
         *       event in the queue, duh!
         */
        if ((eventType == IPC_BROADCAST_PUTSOS || eventType == IPC_BROADCAST_PUTDG) &&
            NumberOfItems(ptrBatchQueue) > 1)
        {
            /*
             * Allocate the memory for the batch broadcast event.  This
             * allocation will be freed in the AsyncEventBroadcast method.
             */
            pBatchBroadcast = MallocSharedWC(sizeof(*pBatchBroadcast) + dataSize);

            /*
             * Setup the values in the batch event.
             */
            pBatchBroadcast->eventType = eventType;
            pBatchBroadcast->bcastType = bcastType;
            pBatchBroadcast->serialNum = serialNum;
            pBatchBroadcast->dataSize = dataSize;

            /*
             * Get the starting point of the data.  This pointer will move
             * through the data space and copy the data from each of the
             * broadcast events in the batch.
             */
            pBatchData = (UINT8 *)&pBatchBroadcast->data;

            /*
             * Loop while there are more batch items to process.
             */
            while (NumberOfItems(ptrBatchQueue) > 0)
            {
                /*
                 * Get the first element in the queue.
                 */
                pBatchElement = (IPC_BROADCAST *)Dequeue(ptrBatchQueue);

                /*
                 * Copy the elements data into the new batch broadcast
                 * data area.
                 */
                memcpy(pBatchData, &pBatchElement->data, pBatchElement->dataSize);

                /*
                 * Increment the data pointer to a spot after the data
                 * we just copied.
                 */
                pBatchData += pBatchElement->dataSize;

                /*
                 * We no longer need the batch element so free it.
                 */
                Free(pBatchElement);
            }
        }
        else if (eventType == IPC_BROADCAST_PUTLCLIMAGE)
        {
            /*
             * For PUTLCLIMAGE events only the last local image needs to
             * get processed so the rest of them can be thrown away.
             */
            if (bcastType & IPC_BROADCAST_MASTER)
            {
                /*
                 * Loop until there is only one event left in
                 * the queue.
                 */
                while (NumberOfItems(ptrBatchQueue) > 1)
                {
                    /*
                     * Get the first item off the queue and
                     * destroy it.
                     */
                    pBatchElement = (IPC_BROADCAST *)Dequeue(ptrBatchQueue);
                    Free(pBatchElement);
                }
            }

            /*
             * Get the first item off the queue so it can
             * get processed.
             */
            pBatchBroadcast = (IPC_BROADCAST *)Dequeue(ptrBatchQueue);
        }
        else
        {
            /*
             * Get the first item off the queue so it can
             * get processed.
             */
            pBatchBroadcast = (IPC_BROADCAST *)Dequeue(ptrBatchQueue);
        }

        /*
         * Process the batch broadcast event.
         */
        AsyncEventBroadcast(pBatchBroadcast);
    }
}

/*----------------------------------------------------------------------------
** Name:    AsyncEventBroadcast
**
** Inputs:  IPC_BROADCAST* pBroadcast - Event that needs to be executed locally
**                                      or on one or more other controllers in
**                                      the virtual controller group.
**
** WARNING: pBroadcast must be freed in this method.  It is a copy of
**          the data sent from the BEP that was allocated in the main
**          AsyncEventHandler.
**--------------------------------------------------------------------------*/
void AsyncEventBroadcast(IPC_BROADCAST *pBroadcast)
{
    UINT32      commandCode = 0;
    bool        bMaster;
    bool        bSelf = false;

    ccb_assert(pBroadcast != NULL, pBroadcast);

//    dprintf(DPRINTF_ASYNC_EVENT_HANDLER, "AsyncEventBroadcast: Processing broadcast event (et: 0x%x, bt: 0x%x, sn: 0x%x, ds: 0x%x).\n",
//            pBroadcast->eventType, pBroadcast->bcastType, pBroadcast->serialNum, pBroadcast->dataSize);

    bMaster = TestforMaster(GetMyControllerSN());

    /*
     * Get the command code from the broadcast event type.
     */
    commandCode = ConvertBroadcastEventToCommandCode(pBroadcast->eventType);

    if (commandCode == MRUPDSID)
    {
//        dprintf(DPRINTF_ASYNC_EVENT_HANDLER, "AsyncEventBroadcast: iSCSI Server ISID update (sid: %d, i_sid: %-16.16qx).\n",
//                ((MRUPDSID_REQ *)pBroadcast->data)->sid, ((MRUPDSID_REQ *)pBroadcast->data)->wwn);
    }

    /*
     * Check if this event is supposed to be sent to the master controller.
     */
    if (pBroadcast->bcastType & IPC_BROADCAST_MASTER)
    {
        if (bMaster)
        {
            BroadcastToSelf(pBroadcast);
            bSelf = true;
        }
        else
        {
            (void)SendIpcBroadcast(Qm_GetMasterControllerSN(), pBroadcast);
        }
    }

    /*
     * Check if this event is supposed to be sent to ourself also.
     */
    if (pBroadcast->bcastType & IPC_BROADCAST_SELF && !bSelf)
    {
        BroadcastToSelf(pBroadcast);
        bSelf = true;
    }

    /*
     * If this event is going to slave controllers, going to all other
     * controllers, or going to the master and this controller is not
     * the master, send it to the controllers via IPC.
     */
    if (pBroadcast->bcastType & IPC_BROADCAST_SLAVES ||
        pBroadcast->bcastType & IPC_BROADCAST_OTHERS)
    {
        BroadcastToControllers(pBroadcast);
    }

    /*
     * If this event is going to a specific controller, make sure
     * it gets to that controller.
     */
    if (pBroadcast->bcastType & IPC_BROADCAST_SPECIFIC)
    {
        /*
         * If the destination controller is this controller don't try
         * and send it over IPC, that would just cause problems.  Send
         * It directly to itself, otherwise send it to the specific
         * controller via IPC.
         */
        if (pBroadcast->serialNum == GetMyControllerSN())
        {
            BroadcastToSelf(pBroadcast);
        }
        else
        {
            (void)SendIpcBroadcast(pBroadcast->serialNum, pBroadcast);
        }
    }

    /*
     * Free the event, this is a copy of the data sent from the
     * BEP which was allocated in the main AsyncEventHandler.  If there
     * was a timeout, use the DelayedFree to free the memory.
     */
    DelayedFree(commandCode, pBroadcast);
}

/*----------------------------------------------------------------------------
** Name:    BroadcastToSelf
**
** Desc:    Execute the broadcast event locally.
**
** Inputs:  IPC_BROADCAST* pBroadcast - Event to execute.
**
**--------------------------------------------------------------------------*/
static void BroadcastToSelf(IPC_BROADCAST *pBroadcast)
{
    UINT32      commandCode = 0;
    UINT16      rc = PI_GOOD;
    MR_GENERIC_RSP *ptrOutPkt = NULL;
    bool        bConfigMutexLocked = false;
    bool        bTempDisableCache = false;

    ccb_assert(pBroadcast != NULL, pBroadcast);

    dprintf(DPRINTF_ASYNC_EVENT_HANDLER, "BroadcastToSelf: Processing broadcast event (et: 0x%x, bt: 0x%x, sn: 0x%x, ds: 0x%x).\n",
            pBroadcast->eventType, pBroadcast->bcastType, pBroadcast->serialNum, pBroadcast->dataSize);

    /*
     * Get the command code from the broadcast event type.
     */
    commandCode = ConvertBroadcastEventToCommandCode(pBroadcast->eventType);

    /*
     * If this is a local image broadcast event it will be handled
     * a little differently than the rest of the broadcast events.
     * It can use the UpdateLocalImage method already in place to
     * send the image to the PROC where the other events need to
     * issue their specific commands via PI_ExecMRP.
     *
     * There is one special PUTSCMT event that needs special handling.
     * If the broadcast type shows the event is only going to the
     * master controller and the copy type indicates this is a
     * copy-swap and the copy-swap is 100 percent complete the
     * event needs to be specially handled.  It will cause another
     * broadcast event to be generated to go to all controllers
     * and then cause a configuration update.
     */
    if (pBroadcast->eventType == IPC_BROADCAST_PUTLCLIMAGE)
    {
        LogMessage(LOG_TYPE_DEBUG, "LCLIMAGE BTS-Seq: 0x%x, Len: 0x%x, Ctrl: 0x%x, MP: 0x%x",
                   PUTLCLIMAGE_SEQ(pBroadcast), PUTLCLIMAGE_LEN(pBroadcast),
                   PUTLCLIMAGE_SN(pBroadcast), PUTLCLIMAGE_MP(pBroadcast));

        /*
         * Aquire the mutex for the configuration update and hold until the
         * request has been fulfilled.
         */
        (void)LockMutex(&configUpdateMutex, MUTEX_WAIT);
        bConfigMutexLocked = true;

        rc = UpdateLocalImage(pBroadcast->data);

        if (rc != GOOD)
        {
            (void)EL_DoElectionNonBlocking();
        }
    }
    else if (pBroadcast->eventType == IPC_BROADCAST_PUTSCMT &&
             pBroadcast->bcastType == IPC_BROADCAST_MASTER &&
             COPY_HAND_CTYPE(pBroadcast) == COPY_HAND_SWAP &&
             COPY_HAND_PERCENT(pBroadcast) == 100)
    {
        IPC_BROADCAST *pNewBroadcast = NULL;
        UINT32      bcastSize = sizeof(*pNewBroadcast) + pBroadcast->dataSize;

        dprintf(DPRINTF_ASYNC_EVENT_HANDLER, "BroadcastToSelf: Special PUTSCMT (1-100)\n");

        pNewBroadcast = MallocSharedWC(bcastSize);
        memcpy(pNewBroadcast, pBroadcast, bcastSize);
        pNewBroadcast->bcastType = IPC_BROADCAST_MASTER | IPC_BROADCAST_SLAVES;

        /*
         * Temporarily disable caching to allow configuration updates to
         * proceed a little smoother.  This will also wait for the cache
         * to be flushed before continuing.
         */
        (void)RMTempDisableCache(PI_MISC_SETTDISCACHE_CMD, TEMP_DISABLE_BROADCAST, 0);
        RMWaitForCacheFlush();
        bTempDisableCache = true;

        /*
         * Aquire the mutex for the configuration update and hold until the
         * request has been fulfilled.
         */
        (void)LockMutex(&configUpdateMutex, MUTEX_WAIT);
        bConfigMutexLocked = true;

        AsyncEventBroadcast(pNewBroadcast);
    }
    else
    {
        if (pBroadcast->eventType == IPC_BROADCAST_PUTDG)
        {
            LogMessage(LOG_TYPE_DEBUG, "DG BTS-SendSN: 0x%x, Type: 0x%x, FC: 0x%x",
                       PUTDG_SN(pBroadcast), PUTDG_TYPE(pBroadcast), PUTDG_FC(pBroadcast));
        }

        ptrOutPkt = MallocSharedWC(sizeof(*ptrOutPkt));

        /*
         * Send the request to Thunderbolt.  This function handles timeout
         * conditions and task switches while waiting.
         */
        rc = PI_ExecMRP(pBroadcast->data, pBroadcast->dataSize, commandCode,
                        ptrOutPkt, sizeof(*ptrOutPkt), GetGlobalMRPTimeout());

        if (rc != PI_GOOD)
        {
            dprintf(DPRINTF_ASYNC_EVENT_HANDLER, "BroadcastToSelf: Failed execute the broadcast (cmdCode: 0x%x, rc: 0x%x, status: 0x%x).\n",
                    commandCode, rc, ptrOutPkt->header.status);
        }
    }

    /*
     * If the MRP or update of local image completed sucessfully there
     * may be some configuration updates required.
     */
    if (rc == PI_GOOD)
    {
        /*
         * If the flag for configuration updates is set handle it.  Otherwise
         * if this is a PUTLDD broadcast event a full configuration update
         * is required.
         */
        if (bConfigMutexLocked)
        {
            /*
             * Signal the slaves to do a configuration update.  The TRUE
             * indicates that only a refresh is to be done.
             */
            RMSlavesRefreshNVRAM(X1_ASYNC_PCHANGED | X1_ASYNC_RCHANGED);

            /*
             * Send a notification of the configuration change to the XIOservice.
             *
             * A local image from another controller will only update physical
             * and raid information so we just need to send the PCHANGED and
             * RCHANGED events.
             */
            SendX1ChangeEvent(X1_ASYNC_PCHANGED | X1_ASYNC_RCHANGED);
        }
        else if (pBroadcast->eventType == IPC_BROADCAST_PUTLDD)
        {
            /*
             * The PUTLDD broadcast event requires that a configuration update
             * needs to occur.
             */

            LogMessage(LOG_TYPE_DEBUG, "Put LDD complete, update configuration");

            /*
             * Signal the slaves to do a configuration update.
             */
            RMSlavesConfigurationUpdate(X1_ASYNC_VCHANGED, TRUE);

            /*
             * Send a notification of the configuration change to the XIOservice.
             */
            SendX1ChangeEvent(X1_ASYNC_VCHANGED);
        }
    }

    /*
     * If the flag for configuration updates is set it means the mutex
     * has been locked and needs to be unlocked.
     */
    if (bConfigMutexLocked)
    {
        /*
         * Unlock the configuration update mutex.
         */
        UnlockMutex(&configUpdateMutex);

        if (bTempDisableCache)
        {
            /*
             * Re-enable caching since we have completed the configuration
             * update.
             */
            (void)RMTempDisableCache(PI_MISC_CLRTDISCACHE_CMD, TEMP_DISABLE_BROADCAST, T_DISABLE_CLEAR_ONE);
        }
    }

    if (rc != PI_TIMEOUT)
    {
        Free(ptrOutPkt);
    }
}

/*----------------------------------------------------------------------------
** Name:    BroadcastToControllers
**
** Desc:    Find the controllers in the active controller map that need to
**          receive this broadcast event.  The controller that are to receive
**          the event is based on the broadcast type in the broadcast packet.
**
** Inputs:  IPC_BROADCAST* pBroadcast - Event to send to the other controllers.
**
**--------------------------------------------------------------------------*/
static void BroadcastToControllers(IPC_BROADCAST *pBroadcast)
{
    UINT32      rc = GOOD;
    UINT32      count;
    UINT32      configIndex;
    UINT32      controllerSN;
    bool        bMaster;

    ccb_assert(pBroadcast != NULL, pBroadcast);

    for (count = 0; count < Qm_GetNumControllersAllowed(); count++)
    {
        configIndex = Qm_GetActiveCntlMap(count);

        if (configIndex != ACM_NODE_UNDEFINED)
        {
            controllerSN = cntlConfigMap.cntlConfigInfo[configIndex].controllerSN;

            /*
             * If the serial number is zero or it is myself, skip this entry.
             */
            if (controllerSN == 0 || controllerSN == GetMyControllerSN())
            {
                continue;
            }

            bMaster = TestforMaster(controllerSN);

            /*
             * There are two conditions either of which need to be met before
             * we send this packet to the controller.
             *  - The controller is master and we are sending to the master
             *  - The controller is not master and we are sending to slaves
             */
            if ((pBroadcast->bcastType & IPC_BROADCAST_SLAVES && !bMaster) ||
                (pBroadcast->bcastType & IPC_BROADCAST_OTHERS))
            {
                rc = SendIpcBroadcast(controllerSN, pBroadcast);

                if (rc == ERROR)
                {
                    dprintf(DPRINTF_ASYNC_EVENT_HANDLER, "BroadcastToControllers - ERROR: Failed to send broadcast to (0x%x).\n",
                            controllerSN);
                }
            }
        }
    }
}

/*----------------------------------------------------------------------------
** Name:    SendIpcBroadcast
**
** Desc:    Create and send the IPC_BROADCAST packet to another controller.
**
** Inputs:  UINT32 serialNum - Serial number of the controller.
**          IPC_BROADCAST* pBroadcast - Event to send to the other controller.
**
** Returns: GOOD if the event was successfully sent and executed on the other
**          controller, ERROR otherwise.
**
**--------------------------------------------------------------------------*/
static UINT32 SendIpcBroadcast(UINT32 serialNum, IPC_BROADCAST *pBroadcast)
{
    UINT32      rc = GOOD;
    IPC_PACKET  rx = { NULL, NULL };
    PATH_TYPE   pathType;
    IPC_PACKET *ptrPacket = NULL;
    UINT32      packetSize = 0;
    IPC_BROADCAST *ptrData;
    UINT8       retries = 2;                /* Ethernet, Fiber(1), Disk Quorum(2) */

    ccb_assert(serialNum > 0, serialNum);
    ccb_assert(pBroadcast != NULL, pBroadcast);

    dprintf(DPRINTF_ASYNC_EVENT_HANDLER, "SendIpcBroadcast: Processing broadcast event (dest: 0x%x, et: 0x%x, bt: 0x%x, sn: 0x%x, ds: 0x%x).\n",
            serialNum, pBroadcast->eventType,
            pBroadcast->bcastType, pBroadcast->serialNum, pBroadcast->dataSize);

    if (pBroadcast->eventType == IPC_BROADCAST_PUTLCLIMAGE)
    {
        LogMessage(LOG_TYPE_DEBUG, "LCLIMAGE SIB-Seq: 0x%x, Len: 0x%x, Ctrl: 0x%x, MP: 0x%x",
                   PUTLCLIMAGE_SEQ(pBroadcast), PUTLCLIMAGE_LEN(pBroadcast),
                   PUTLCLIMAGE_SN(pBroadcast), PUTLCLIMAGE_MP(pBroadcast));
    }
    else if (pBroadcast->eventType == IPC_BROADCAST_PUTDG)
    {
        LogMessage(LOG_TYPE_DEBUG, "DG SIB-SendSN: 0x%x, Type: 0x%x, FC: 0x%x",
                   PUTDG_SN(pBroadcast), PUTDG_TYPE(pBroadcast), PUTDG_FC(pBroadcast));
    }

    /*
     * The IPC_BROADCAST structre is defined to contain some data.
     * We setup a dummy array of items that essentially represent
     * a buffer when finished.  For this reason when calculating the
     * size of the packet to allocate to return we take the size of
     * the broadcast and add the actual broadcast data size returned
     * from the PROC.
     */
    packetSize = sizeof(*ptrData) + pBroadcast->dataSize;

    /* Create the IPC packet */
    ptrPacket = CreatePacket(PACKET_IPC_BROADCAST, packetSize, __FILE__, __LINE__);

    ptrData = (IPC_BROADCAST *)ptrPacket->data;
    memcpy((void *)ptrData, pBroadcast, packetSize);

#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s call IpcSendPacketBySN with rxPacket of %p\n", __FILE__, __LINE__, __func__, &rx);
#endif  /* HISTORY_KEEP */

    do
    {
        Free(rx.data);

        /* Sending packet to the other controller using any IPC path possible */
        pathType = IpcSendPacketBySN(serialNum, SENDPACKET_ANY_PATH,
                                     ptrPacket, &rx, NULL, NULL, NULL, AEH_IPC_SEND_TMO);
    } while (pathType == SENDPACKET_NO_PATH && (retries--) > 0);

    if (!IpcSuccessfulXfer(pathType))
    {
        dprintf(DPRINTF_ASYNC_EVENT_HANDLER, "SendIpcBroadcast - ERROR: Send packet failed (0x%x).\n", pathType);

        rc = ERROR;
    }
    else
    {
        if (rx.data->commandStatus.status != IPC_COMMAND_SUCCESSFUL)
        {
            dprintf(DPRINTF_ASYNC_EVENT_HANDLER, "SendIpcBroadcast - ERROR: Failed to send event to controller (0x%x).\n", serialNum);

            rc = ERROR;
        }

    }
    FreePacket(&ptrPacket, __FILE__, __LINE__);

    FreePacketStaticPacketPointer(&rx, __FILE__, __LINE__);

    return (rc);
}

/*----------------------------------------------------------------------------
** Name:    ConvertBroadcastEventToCommandCode
**
** Desc:    Convert a broadcast event type into its corresponding
**          MRP command code.
**
** Inputs:  UINT16 eventType - Broadcast event type.
**
** Returns: MRP command code for the given broadcast type.
**
**--------------------------------------------------------------------------*/
UINT32 ConvertBroadcastEventToCommandCode(UINT16 eventType)
{
    UINT32      commandCode = 0;

    if (eventType == IPC_BROADCAST_PUTSOS)
    {
        commandCode = MRPUTSOS;
    }
    else if (eventType == IPC_BROADCAST_PUTSCMT)
    {
        commandCode = MRPUTSCMT;
    }
    else if (eventType == IPC_BROADCAST_PUTDG)
    {
        commandCode = MRPUTDG;
    }
    else if (eventType == IPC_BROADCAST_PUTLCLIMAGE)
    {
        commandCode = MRPUTLCLIMAGE;
    }
    else if (eventType == IPC_BROADCAST_FSYS)
    {
        commandCode = MRPUTFSYS;
    }
    else if (eventType == IPC_BROADCAST_PUTLDD)
    {
        commandCode = MRPUTLDD;
    }
    else if (eventType == IPC_BROADCAST_ISID)
    {
        commandCode = MRUPDSID;
    }
    else if (eventType == IPC_BROADCAST_PRR)
    {
        commandCode = MRUPDPRR;
    }

    return commandCode;
}

/*----------------------------------------------------------------------------
** Name:    AsyncGroupValidation
**
** Desc:    Call the main validation function in another module.
**
** Inputs:  validationFlags - Validation flags to run.
**
**--------------------------------------------------------------------------*/
static void AsyncGroupValidation(UINT16 validationFlags)
{
    /*
     * Call the main validation routine.  It will handle all issues
     * related to master/slave, power up complete, etc.
     */
    RM_StartGroupValidation(validationFlags);
}


/**
******************************************************************************
**
**  @brief      This function initiates the physical disk defragmentation
**              process.
**
**  @param      UINT16 pid - physical disk identifier or one of the special
**                           values.
**                              DEFRAG ALL  - 0xFFFF
**                              DEFRAG STOP - 0xFFFE
**
**  @param      UINT8 delay - Seconds to delay before starting the task.
**
**  @return     none
**
**  @attention  FORKED
**
******************************************************************************
**/
void PDiskDefragOperation(UINT16 pid, UINT32 delay)
{
    TASK_PARMS  taskParms;
    PARALLEL_REQUEST *pRequests;
    PR_STARTIO_PARAM paramStart;

    LogMessage(LOG_TYPE_DEBUG, "PDiskDefragOperation-ENTER (0x%x, %d)", pid, delay);

    if (pid == PI_PDISK_DEFRAG_STOP)
    {
        gbPDiskDefragStop = true;
    }
    else
    {
        if (pPDiskDefragTask != NULL)
        {
            LogMessage(LOG_TYPE_DEBUG, "PDiskDefragOperation-Defrag already running");
        }
        else
        {
            /*
             * Clear out the start parameter structure.
             */
            memset(&paramStart, 0x00, sizeof(paramStart));

            /*
             * Initialize the start parameters.
             */
            paramStart.option = START_IO_OPTION_CLEAR_ALL_STOPS;
            paramStart.user = START_STOP_IO_USER_CCB_DEFRAG;
            paramStart.tmo = 0;

            /*
             * Allocate the parallel request buffers.
             */
            pRequests = PR_AllocTemplate(0, &paramStart, sizeof(paramStart));

            /*
             * Send the START parallel requests.
             */
            PR_SendRequests(PR_DEST_ACTIVE, PR_TYPE_STARTIO, pRequests, PR_SendTaskStartIO);

            /*
             * Release the parallel request buffers.
             */
            PR_Release(&pRequests);

            /*
             * Re-enable caching since we are starting the defrag
             * operations just in case it was previously disabled.
             *
             * The main case for this is if a defrag was in progress
             * and the master was failed during the move operation.
             */
            (void)RMTempDisableCache(PI_MISC_CLRTDISCACHE_CMD, TEMP_DISABLE_DEFRAG, T_DISABLE_CLEAR_ALL);

            /*
             * Update the configuration to indicate that a defrag is
             * current active.
             */
            Qm_SetDefragActive(TRUE);
            Qm_SetDefragPID(pid);
            SaveMasterConfig();

            /*
             * Make sure all operational slaves see the update to the
             * configuration.
             */
            IpcSignalSlaves(IPC_SIGNAL_LOAD_CONFIG);

            taskParms.p1 = delay;
            pPDiskDefragTask = TaskCreate(PDiskDefragTask, &taskParms);
            gbPDiskDefragStop = false;
        }
    }
}


/**
******************************************************************************
**
**  @brief      This function is forked as a task to handle the processing
**              required for physical disk defragmentation.
**
**  @param      TASK_PARMS* parms - Forked task parameter, p1 = delay
**                                  in seconds to wait before starting
**                                  the defrag operations.
**
**  @return     none
**
**  @attention  FORKED
**
******************************************************************************
**/
static void PDiskDefragTask(TASK_PARMS *parms)
{
    UINT32      delay = parms->p1;
    INT32       rc = PI_GOOD;
    MRDEFRAGMENT_RSP defragData;
    UINT8       status = DEFRAG_OP_COMPLETE_OK;

    LogMessage(LOG_TYPE_DEBUG, "PDiskDefragTask-Starting (Delay: %d)", delay);

    /*
     * Delay the requested time.
     */
    TaskSleepMS(delay * 1000);

    while (FOREVER)
    {
        /*
         * Check if there are rebuilds currently running in the
         * system.  This function will also check if the controller
         * is still the master, if elections or RM is currently in
         * progress.
         */
        PDiskDefragWaitForRebuilds();

        /*
         * Check if this controller is still the master controller.  If
         * not then we do not need to wait for the rebuilds any longer.
         */
        if (!TestforMaster(GetMyControllerSN()))
        {
            LogMessage(LOG_TYPE_DEBUG, "PDiskDefragTask-Not master");

            break;
        }

        /*
         * Reset the defrag data and the verify in progress flag values.
         */
        memset(&defragData, 0x00, sizeof(defragData));
        gPDiskDefragVIP = 0xFFFF;

        /*
         * If the stop flag has been set, tell the PROC and end the task.
         */
        if (gbPDiskDefragStop)
        {
            (void)PDiskDefragControl(MRDEFRAGMENT_STOP, Qm_GetDefragPID(), 0, 0, &defragData);

            /*
             * Set the defrag operation completion status to stopped.
             */
            status = DEFRAG_OP_COMPLETE_STOPPED;

            /*
             * The defragmentation process is supposed to stop so exit
             * out of the loop.
             */
            break;
        }

        rc = PDiskDefragControl(MRDEFRAGMENT_PREPARE, Qm_GetDefragPID(), 0, 0, &defragData);

        if (rc == PI_GOOD)
        {
            /*
             * If the prepare operation completed successfully, send
             * the verify command.
             *
             * If the prepare operation indicated there were no move
             * moves remaining, exit out.
             *
             * If the prepare operation resulted in an error, exit
             * out and stop the defrag where it is at.
             */
            if (defragData.status == MRDEFRAGMENT_OK)
            {
                if (!TestforMaster(GetMyControllerSN()) || EL_TestInProgress() ||
                    RM_IsReallocRunning() || PDiskDefragCheckForRebuilds())
                {
                    LogMessage(LOG_TYPE_DEBUG, "PDiskDefragTask-System in flux, skip verify");

                    /*
                     * Go back to the top of the WHILE loop to restart the
                     * defrag processing.
                     */
                    continue;
                }

                rc = PDiskDefragVerify(&defragData);

                /*
                 * If the stop has been signaled back to the top
                 * and process the stop operation.
                 */
                if (gbPDiskDefragStop)
                {
                    /*
                     * Go back to the top of the WHILE loop to restart the
                     * defrag processing.
                     */
                    continue;
                }

                /*
                 * If the verify operation completed successfully and
                 * the defragmentation process has not been stopped,
                 * go ahead and do the move.
                 */
                if (rc == PI_GOOD)
                {
                    if (TestforMaster(GetMyControllerSN()) && !EL_TestInProgress() &&
                        !RM_IsReallocRunning() && !PDiskDefragCheckForRebuilds())
                    {
                        PDiskDefragMove(&defragData);
                    }
                    else
                    {
                        LogMessage(LOG_TYPE_DEBUG, "PDiskDefragTask-System in flux, skip move");
                    }
                }
                else
                {
                    LogMessage(LOG_TYPE_DEBUG, "PDiskDefragTask-Verify error (0x%x, 0x%x, 0x%x)",
                               rc, defragData.status, gPDiskDefragVIP);
                }
            }
            else if (defragData.status == MRDEFRAGMENT_NOMOVE)
            {
                LogMessage(LOG_TYPE_DEBUG, "PDiskDefragTask-Prepare no moves remaining");

                /*
                 * Exit the while loop, no move left to process.
                 */
                break;
            }
            else
            {
                LogMessage(LOG_TYPE_DEBUG, "PDiskDefragTask-Prepare error (0x%x)", defragData.status);

                /*
                 * Set the defrag completion status to error.
                 */
                status = DEFRAG_OP_COMPLETE_ERROR;

                /*
                 * Exit the while loop, a prepare error is fatal.
                 */
                break;
            }
        }
        else if (rc != PI_TIMEOUT)
        {
            LogMessage(LOG_TYPE_DEBUG, "PDiskDefragTask-Unable to prepare defrag (0x%x)", rc);

            /*
             * Set the defrag completion status to error.
             */
            status = DEFRAG_OP_COMPLETE_ERROR;

            /*
             * Exit the while loop, a prepare error is fatal.
             */
            break;
        }

        /*
         * If one of the defrag steps encountered a timeout condition
         * we will wait here until the BE processor has cleaned up
         * the condition before starting the defrag loop again.
         */
        if (rc == PI_TIMEOUT)
        {
            LogMessage(LOG_TYPE_DEBUG, "PDiskDefragTask-Defrag task timeout, wait for completion");

            while (BEBlocked())
            {
                TaskSleepMS(1000);
            }

            /*
             * Just to be safe, wait some more time to give
             * the system time to clean up.
             */
            TaskSleepMS(10000);
        }
    }

    /*
     * If this is the master controller, clear the master configuration
     * defrag information before leaving and tell the rest of the
     * controller to update their copies of the data.
     */
    if (TestforMaster(GetMyControllerSN()))
    {
        Qm_SetDefragActive(FALSE);
        Qm_SetDefragPID(0xFFFF);
        SaveMasterConfig();

        /*
         * Make sure all operational slaves see the update to the
         * configuration.
         */
        IpcSignalSlaves(IPC_SIGNAL_LOAD_CONFIG);

        PDiskDefragLogDefragOpComplete(status);
    }

    /*
     * The task is ending so clear out the PCB variable.
     */
    pPDiskDefragTask = NULL;
}


/**
******************************************************************************
**
**  @brief      This function handles the process of waiting for all
**              rebuilds in the system to complete.  This is a required
**              step before executing the next defragmentation operation.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void PDiskDefragWaitForRebuilds(void)
{
    bool        bRebuildsActive = true;

    /*
     * While the rebuilds are active, retrieve the raid information
     * and check each raid to see if it is currently rebuilding.
     */
    while (TestforMaster(GetMyControllerSN()) && bRebuildsActive)
    {
        /*
         * Check if a stop request has been issued, we will end
         * this check early and allow the defrags to terminate.
         */
        if (gbPDiskDefragStop)
        {
            /*
             * A stop request has been issued so exit out of the
             * wait and allow the defrags to be terminated.
             */
            break;
        }

        /*
         * If an election is in progress or RM is currently running
         * we need to wait for a little bit before looking for the
         * active rebuilds again.
         */
        if (EL_TestInProgress() == TRUE || RM_IsReallocRunning())
        {
            LogMessage(LOG_TYPE_DEBUG, "PDiskDefragWaitForRebuilds-Election or RM, delay processing");

            /*
             * While an election or reallocation is in progress just
             * sleep for a short period of time.  This loop is used
             * to watch this progression and to identify when they
             * complete.
             */
            while (EL_TestInProgress() == TRUE || RM_IsReallocRunning())
            {
                /*
                 * Sleep for a little while to allow the election and/or
                 * reallocation to complete.
                 */
                TaskSleepMS(5000);
            }

            LogMessage(LOG_TYPE_DEBUG, "PDiskDefragWaitForRebuilds-Election or RM complete");

            /*
             * Now that the election and reallocation have completed
             * sleep for a little while longer before allowing the
             * defrag to continue.  This is just to allow the system
             * to settle down before restarting the defrag operations.
             */
            TaskSleepMS(30000);

            /*
             * Now that we have waited for the elections and reallocations
             * to complete along with some extra time, loop back to the
             * beginning of the loop to see if the state of the system
             * is good enough to continue looking if there are rebuilds
             * active or if we should skip this altogether (not master
             * case).
             */
            continue;
        }

        bRebuildsActive = PDiskDefragCheckForRebuilds();

        /*
         * If there are active rebuilds, wait around here for a little
         * while to allow them to finish.
         */
        if (bRebuildsActive)
        {
            TaskSleepMS(10000);
        }
    }
}


/**
******************************************************************************
**
**  @brief      This function checks if there are rebuilds, parity scans or
**              stripe resyncs in progress.
**
**  @param      none
**
**  @return     true if there are rebuilds, parity scans or stripe resync
**              operations in progress, false otherwise.
**
******************************************************************************
**/
static bool PDiskDefragCheckForRebuilds(void)
{
    PI_RAIDS_RSP *pRaids;
    PI_RAID_INFO_RSP *pRaid;
    INT32       i;
    bool        bRebuildsActive = true;

    /*
     * Get all the current raid information.
     */
    pRaids = Raids();

    /*
     * If we were able to retrieve the raid information, look for the
     * in progress stuff.
     *
     * If we were unable to retrieve the raid information then the default
     * return value is that reuilds, etc. are in progress.
     */
    if (pRaids)
    {
        /*
         * Setup a pointer to the first raid in the list.
         */
        pRaid = (PI_RAID_INFO_RSP *)pRaids->raids;

        /*
         * Loop through all the raids.
         */
        for (i = 0; i < pRaids->count; ++i)
        {
            /*
             * Check if this raid is actively rebuilding, if there
             * is a parity scan required or if a stripe resync is
             * in progress.
             */
            if (BIT_TEST(pRaid->aStatus, RD_A_REBUILD) ||
                BIT_TEST(pRaid->aStatus, RD_A_PARITY) ||
                BIT_TEST(pRaid->aStatus, RD_A_R5SRIP))
            {
                /*
                 * Rebuild, parity scan or stripe resyncb on this raid is
                 * still active, exit now.
                 */
                break;
            }

            /*
             * Move to the next raid in the list.  Since the raid
             * information is a dynamically sized response it is
             * not a simple index increase but rather a move over
             * the current response side to get to the next one.
             */
            pRaid = (PI_RAID_INFO_RSP *)((UINT8 *)pRaid + pRaid->header.len);
        }

        /*
         * If we reached the end of the raid list there was not a
         * raid actively rebuilding.
         */
        if (i == pRaids->count)
        {
            bRebuildsActive = false;
        }

        /*
         * Free up the raid information
         */
        Free(pRaids);
    }

    return bRebuildsActive;
}


/**
******************************************************************************
**
**  @brief      This function handles the verify phase of the
**              defragmentation process.
**
**  @param      MRDEFRAGMENT_RSP* pDefrag - Pointer to a defragment response
**                                          packet that holds both the
**                                          input for the verify control and
**                                          the output from the verify control.
**
**  @return     INT32 - PI_GOOD, PI_TIMEOUT or one of the other PI codes.
**
******************************************************************************
**/
static INT32 PDiskDefragVerify(MRDEFRAGMENT_RSP *pDefrag)
{
    INT32       rc = PI_GOOD;

    rc = PDiskDefragControl(MRDEFRAGMENT_VERIFY,
                            pDefrag->pdiskID, pDefrag->raidID, pDefrag->sda, pDefrag);

    /*
     * If the verify request was sent successfully and the operation
     * was started successfully (signaled by the OK) then we need
     * to wait until the verify completes (signaled by the change in
     * the gPDiskDefragVIP global.
     *
     * If the verify request was not sent successfully or the operation
     * not started sucessfully we want to signal that an error occurred.
     *
     * NOTE: If the request times out, we want to return that to the
     *       caller so they can delay appropriately.
     */
    if (rc == PI_GOOD && pDefrag->status == MRDEFRAGMENT_OK)
    {
        LogMessage(LOG_TYPE_DEBUG, "PDiskDefragVerify-Verify in progress, waiting for completion");

        /*
         * Wait while the defrag operations have not been stopped and
         * the verify operation has not completed.
         */
        while (!gbPDiskDefragStop && gPDiskDefragVIP == 0xFFFF)
        {
            TaskSleepMS(1000);
        }

        if (gbPDiskDefragStop)
        {
            LogMessage(LOG_TYPE_DEBUG, "PDiskDefragVerify-Defrag stopped, verify not complete");
        }
        else
        {
            LogMessage(LOG_TYPE_DEBUG, "PDiskDefragVerify-Verify complete (0x%x)", gPDiskDefragVIP);
        }

        /*
         * If the verify did not complete sucessfully, signal an error.
         */
        if (!gPDiskDefragVIP)
        {
            rc = PI_ERROR;
        }
    }
    else if (rc != PI_TIMEOUT)
    {
        LogMessage(LOG_TYPE_DEBUG, "PDiskDefragVerify-Verify error (0x%x, 0x%x, 0x%x)",
                   rc, pDefrag->status, gPDiskDefragVIP);

        /*
         * No matter what signal an error.
         */
        rc = PI_ERROR;
    }

    return rc;
}


/**
******************************************************************************
**
**  @brief      This function handles the move phase of the
**              defragmentation process.
**
**  @param      MRDEFRAGMENT_RSP* pDefrag - Pointer to a defragment response
**                                          packet that holds both the
**                                          input for the move control and
**                                          the output from the move control.
**
******************************************************************************
**/
static void PDiskDefragMove(MRDEFRAGMENT_RSP *pDefrag)
{
    INT32       rc;
    PARALLEL_REQUEST *pRequests;
    PR_STOPIO_PARAM paramStop;
    PR_STARTIO_PARAM paramStart;
    INT32       i;
    bool        bStopIO = true;

    /*
     * Clear out the stop and start parameter structures.
     */
    memset(&paramStop, 0x00, sizeof(paramStop));
    memset(&paramStart, 0x00, sizeof(paramStart));

    /*
     * Initialize the stop parameters.
     */
    paramStop.operation = STOP_WAIT_FOR_FLUSH | STOP_NO_BACKGROUND;
    paramStop.intent = STOP_NO_SHUTDOWN;
    paramStop.user = START_STOP_IO_USER_CCB_DEFRAG;
    paramStop.tmo = TMO_DEFRAG_MOVE_STOP_IO;

    /*
     * Initialize the start parameters.
     */
    paramStart.option = START_IO_OPTION_CLEAR_ONE_STOP_COUNT;
    paramStart.user = START_STOP_IO_USER_CCB_DEFRAG;
    paramStart.tmo = 0;

    /*
     * Temporarily disable caching to allow configuration updates to
     * proceed a little smoother.  This will also wait for the cache
     * to be flushed before continuing.
     */
    (void)RMTempDisableCache(PI_MISC_SETTDISCACHE_CMD, TEMP_DISABLE_DEFRAG, 0);
    RMWaitForCacheFlush();

    /*
     * Allocate the parallel request buffers.
     */
    pRequests = PR_AllocTemplate(0, &paramStop, sizeof(paramStop));

    /*
     * Send the STOPIO parallel requests.
     */
    PR_SendRequests(PR_DEST_ACTIVE, PR_TYPE_STOPIO, pRequests, PR_SendTaskStopIO);

    /*
     * Check if any of the stop IO requests failed.
     */
    for (i = 0; i < MAX_CONTROLLERS; ++i)
    {
        if (pRequests->controllerSN != 0 && pRequests->rc != GOOD)
        {
            bStopIO = false;
            break;
        }
    }

    /*
     * Release the parallel request buffers.
     */
    PR_Release(&pRequests);

    if (bStopIO)
    {
        /*
         * Aquire the mutex for the configuration update and hold until the
         * request has been fulfilled.
         */
        (void)LockMutex(&configUpdateMutex, MUTEX_WAIT);

        rc = PDiskDefragControl(MRDEFRAGMENT_MOVE, pDefrag->pdiskID, pDefrag->raidID, pDefrag->sda, pDefrag);

        /*
         * If the move request times out there is some special processing
         * required.  Essentially we need to wait until that request
         * completes and still send the configuration update to all the
         * slave controllers.  In this case since we have already locked
         * the configuration update mutex we will start by unlocking
         * the mutex and then proceed to wait for the MRP queue to clear
         * up.  Once that happens we will send the the configuration
         * update to the slave controller and tell that function to lock
         * the mutex again.  This processing is similar to the code
         * used to fail a controller in the RM_Realloc.c code.
         *
         * If the request completed successfully, send the configuration
         * update to the slave controllers.  Then, regardless whether the
         * requets completed successfully or not we need to unlock the
         * configuration update mutex.
         */
        if (rc == PI_TIMEOUT)
        {
            /*
             * Unlock the configuration update mutex.
             */
            UnlockMutex(&configUpdateMutex);

            /*
             * Wait until a MRP executes without a timeout.
             */
            while (rc == PI_TIMEOUT)
            {
                /*
                 * Check if there still are timeouts pending.
                 */
                if (BEBlocked() == FALSE)
                {
                    /*
                     * See if the BE will sucessfully execute a MRP now.
                     */
                    rc = ProcessorQuickTest(MRNOPBE);

                    /*
                     * When the Fail Ctrl MRP time outs, the configuration
                     * was not propagated to the slave controller.
                     * Go it now.
                     */
                    if (rc == PI_GOOD)
                    {
                        RMSlavesConfigurationUpdate(X1_ASYNC_PCHANGED, TRUE);
                    }
                }
                else
                {
                    /*
                     * Wait a second and try again.
                     */
                    TaskSleepMS(1000);
                }
            }
        }
        else
        {
            /*
             * If the request completed successfully we need to send the
             * configuration update to the rest of the controllers.  Signal
             * that the mutex should not be locked as we have already done
             * that.
             */
            if (rc == PI_GOOD)
            {
                RMSlavesConfigurationUpdate(X1_ASYNC_PCHANGED, FALSE);
            }

            /*
             * Unlock the configuration update mutex.
             */
            UnlockMutex(&configUpdateMutex);
        }
    }

    /*
     * Allocate the parallel request buffers.
     */
    pRequests = PR_AllocTemplate(0, &paramStart, sizeof(paramStart));

    /*
     * Send the START parallel requests.
     */
    PR_SendRequests(PR_DEST_ACTIVE, PR_TYPE_STARTIO, pRequests, PR_SendTaskStartIO);

    /*
     * Release the parallel request buffers.
     */
    PR_Release(&pRequests);

    /*
     * Re-enable caching since we have completed the configuration
     * update.
     */
    (void)RMTempDisableCache(PI_MISC_CLRTDISCACHE_CMD, TEMP_DISABLE_DEFRAG, T_DISABLE_CLEAR_ONE);
}


/**
******************************************************************************
**
**  @brief      This function executes a control operation for the
**              physical disk defragmentation process.
**
**  @param      UINT16 control - defragment control operation.
**
**  @param      UINT16 pid - physical disk identifier.
**
**  @param      UINT16 rid - raid identifier.
**
**  @param      UINT64 sda - starting disk address
**
**  @param      MRDEFRAGMENT_RSP* pResponse - pointer to a defrag response
**                                            structure to hold a copy
**                                            of the response data.
**
**  @return     INT32 - PI_GOOD, PI_TIMEOUT or one of the other PI codes.
**
******************************************************************************
**/
INT32 PDiskDefragControl(UINT16 control, UINT16 pid, UINT16 rid, UINT64 sda,
                         MRDEFRAGMENT_RSP *pResponse)
{
    INT32       rc;
    MRDEFRAGMENT_REQ *pReq;
    MRDEFRAGMENT_RSP *pRsp;

    LogMessage(LOG_TYPE_DEBUG, "PDiskDefragControl-ENTER (0x%x, 0x%x, 0x%x)", control, pid, rid);

    /*
     * Allocate memory for the MRP input and output packets.
     */
    pReq = MallocWC(sizeof(*pReq));
    pRsp = MallocSharedWC(sizeof(*pRsp));

    pReq->control = control;
    pReq->pdiskID = pid;
    pReq->raidID = rid;
    pReq->sda = sda;

    /*
     * Send the request to Thunderbolt.  This function handles timeout
     * conditions and task switches while waiting.
     */
    rc = PI_ExecMRP(pReq, sizeof(*pReq), MRDEFRAGMENT,
                    pRsp, sizeof(*pRsp),
                    (control == MRDEFRAGMENT_MOVE ? TMO_NONE : GetGlobalMRPTimeout()));

    if (rc == PI_GOOD)
    {
        memcpy(pResponse, pRsp, sizeof(*pRsp));
    }
    else
    {
        memset(pResponse, 0x00, sizeof(*pRsp));
    }

    /*
     * Free the allocated memory
     */
    Free(pReq);

    if (rc != PI_TIMEOUT)
    {
        Free(pRsp);
    }

    return rc;
}


/**
******************************************************************************
**
**  @brief      This function sends the defrag all done log message.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void PDiskDefragLogDefragOpComplete(UINT8 status)
{
    LOG_DEFRAG_OP_COMPLETE_DAT defragOpComplete;

    memset(&defragOpComplete, 0x00, sizeof(defragOpComplete));
    defragOpComplete.status = status;
    defragOpComplete.pdiskID = Qm_GetDefragPID();

    SendAsyncEvent(LOG_DEFRAG_OP_COMPLETE, sizeof(defragOpComplete), &defragOpComplete);
}

/**
******************************************************************************
**
**  @brief      This function handles the processing
**              required for a LOG_PRES_CHANGE message.
**
**  @param      UINT32 dummy - placeholder required for forked task.
**
**  @return     none
**
******************************************************************************
**/
static void AsyncPresChange(void)
{
    /*
     * Send the configuration update to the slave controllers.
     */
    RMSlavesConfigurationUpdate(ASYNC_PRES_CHANGED, TRUE);
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
