/* $Id: ipc_heartbeat.c 156532 2011-06-24 21:09:44Z m4 $ */
/*============================================================================
** FILE NAME:       ipc_heartbeat.c
** MODULE TITLE:    Interprocessor heartbeat functions
**
** DESCRIPTION:     Implementation for interprocessor heartbeat task
**                  and functions
**
** Copyright (c) 2001-2009 Xiotech Corporation. All rights reserved.
**==========================================================================*/
#include "ipc_heartbeat.h"

#include "AsyncEventHandler.h"
#include "cps_init.h"
#include "debug_files.h"
#include "LOG_Defs.h"
#include "EL.h"
#include "fm.h"
#include "i82559.h"
#include "ipc_packets.h"
#include "ipc_sendpacket.h"
#include "ipc_session_manager.h"
#include "kernel.h"
#include "logdef.h"
#include "MR_Defs.h"
#include "mode.h"
#include "PortServer.h"
#include "PortServerUtils.h"
#include "PI_Utils.h"
#include "quorum.h"
#include "quorum_utils.h"
#include "rtc.h"
#include "X1_Packets.h"
#include "XIO_Std.h"
#include "XIO_Macros.h"
#include "CT_history.h"

/*****************************************************************************
** Private defines
*****************************************************************************/
#define HM_HEARTBEAT_ENABLE         0x0001
#define HM_LOCAL_MONITOR_ENABLE     0x0002
#define HM_LOCAL_STATS_ENABLE       0x0004

#define HEARTBEAT_PERIOD                1000
#define HEALTH_MONITOR_PERIOD           1000
#define X1_HEARTBEAT_PERIOD             10000
#ifndef M4_ABORT
#define HB_WATCHDOG_TIMEOUT             5
#else   /* M4_ABORT */
#define HB_WATCHDOG_TIMEOUT             200
#endif  /* M4_ABORT */

#ifndef M4_ABORT
#define HB_PACKET_TIMEOUT               2000    /* 2 second timeout         */
#else   /* M4_ABORT */
#define HB_PACKET_TIMEOUT               200000  /* 200 second timeout         */
#endif  /* M4_ABORT */

#define WatchdogExpired()               (hbWatchdog > HB_WATCHDOG_TIMEOUT)
#define IncrementWatchdog()             (++hbWatchdog)
#define ResetWatchdog()                 (hbWatchdog = 0)

#define HeartbeatsEnabled()         ((hmState &  HM_HEARTBEAT_ENABLE) && \
                                    (!TestModeBit(MD_IPC_HEARTBEAT_DISABLE)))

#define LocalHealthMonitorEnabled() ((hmState &  HM_LOCAL_MONITOR_ENABLE) && \
                                    (!TestModeBit(MD_LOCAL_HEARTBEAT_DISABLE)))

#define EnableHeartbeats()              (hmState |=  HM_HEARTBEAT_ENABLE)
#define DisableHeartbeats()             (hmState &=  (~HM_HEARTBEAT_ENABLE))
#define EnableLocalMonitor()            (hmState |=  HM_LOCAL_MONITOR_ENABLE)

/*****************************************************************************
** Private variables
*****************************************************************************/
static UINT16 hbWatchdog = 0;
static UINT16 hmState = 0;

typedef struct _IPC_HB_CHILD
{
    UINT32      sn;
    IPC_HEARTBEAT *dataPtr;
    bool        running;
} IPC_HB_CHILD;

static IPC_HB_CHILD leftChild = { 0, NULL, false };
static IPC_HB_CHILD rightChild = { 0, NULL, false };

static UINT64 feIOPerSecond = 0;
static UINT64 feBytesPerSecond = 0;
static UINT32 feHeartbeatFlag = 0;
static UINT32 beHeartbeatFlag = 0;

static UINT8 linkEthernetStatus = GOOD;
static UINT8 linkFibreStatus = GOOD;
static UINT8 linkQuorumStatus = GOOD;
static UINT8 linkFePciStatus = GOOD;
static UINT8 linkBePciStatus = GOOD;

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static UINT32 GetParentSN(void);
static void RequestIpcHeartbeat(IPC_HB_CHILD *childPtr);
static void LocalHealthMonitorTask(void);
static void LocalHealthMonitorTaskFE(TASK_PARMS *parms);
static void LocalHealthMonitorTaskBE(TASK_PARMS *parms);
static void NotifyLocalProcessor(TASK_PARMS *parms);
static UINT16 MRP_Heartbeat(UINT32 commandCode);
static void LinkLogMessage(UINT32 detectedBySN, /* Controller finding link failure */
                           UINT32 destinationSN, /* Destination of the failed link */
                           UINT8 linkType,      /* Type of the link that has failed */
                           UINT32 linkError,    /* Error found on this link        */
                           UINT8 processor);    /* processor for link error        */

static void IPC_HbStart(void);
static void IPC_HbStop(void);
static void IPC_HbTask(TASK_PARMS *parms);

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
**  Function Name: ipcHeartbeatAndStatsTask
**
**  Comments:  This function gathers statistical information from the local
**      processors to present within requested heartbeat packets from this
**      controllers parent and also initiates heartbeats to each of its
**      children. Also, if the this controller is NOT at the top of the
**      heartbeat tree (no parent), it checks a watchdog timer to ensure that
**      its parent is still sending heartbeats.
**
**--------------------------------------------------------------------------*/
NORETURN void IpcHeartbeatAndStatsTask(UNUSED TASK_PARMS *parms)
{
    ETHERNET_LINK_STATUS ethernetLinkStatus;
    TASK_PARMS  intParms;

    /*
     * Start up a task to handle intra-controller health monitoring
     */
    LocalHealthMonitorTask();

    /*
     * Gather statistics and monitor heartbeats forever
     */
    while (1)
    {
        /*
         * Exchange and wait of the statistics and heartbeat cycle
         */
        TaskSleepMS(HEARTBEAT_PERIOD);

        /*
         * Monitor the Ethernet Link
         * The monitor function handles generating log message (and notifying
         * failure manager) for link up and link down conditions.
         * An error is returned if the function cannot get the link
         * status.
         */
        if (EthernetLinkMonitor(&ethernetLinkStatus) != GOOD)
        {
            dprintf(DPRINTF_HEALTH_MONITOR, "HM FAILURE: Unable to get Ethernet Link Status\n");
        }

        if (HeartbeatsEnabled())
        {
            /*
             * If we are part of a Virtual controller group, test heartbeat
             * watchdog timer and  send heartbeat requests to our children
             * (if they exist).
             */
            if (PartOfVCG())
            {
                /*
                 * If we are not a master (at the top of the heartbeat tree),
                 * then check the watchdog timer. If it has expired, report
                 * the failure to the failure manager. If not, increment the
                 * the watchdog. The watchdog is reset each time a heartbeat
                 * packet is handled.
                 */
                if (!TestforMaster(GetMyControllerSN()) &&
                    GetControllerFailureState() == FD_STATE_OPERATIONAL)
                {
                    if (WatchdogExpired() &&
                        (!TestModeBit(MD_IPC_HEARTBEAT_WATCHDOG_DISABLE)))
                    {
                        /*
                         * Figure out who our parent is and report the
                         * failure to the failure manager.
                         */
                        dprintf(DPRINTF_HEALTH_MONITOR, "HB FAILURE: Expected Heartbeat not received\n");

                        intParms.p1 = (UINT32)GetMyControllerSN();
                        intParms.p2 = (UINT32)GetParentSN();
                        intParms.p3 = (UINT32)IPC_LINK_TYPE_ETHERNET;
                        intParms.p4 = (UINT32)PROCESS_CCB;
                        intParms.p5 = (UINT32)IPC_LINK_ERROR_NO_HEARTBEAT;
                        UpdateLinkStatus(&intParms);

                        ResetWatchdog();
                    }
                    else
                    {
                        IncrementWatchdog();
                    }
                }
            }
        }
        else
        {
            ResetWatchdog();
        }
    }
}


/*----------------------------------------------------------------------------
**  Function Name: HandleIpcHeartbeat
**
**  Comments:  Handles the heartbeat request from parent controller. Fills in
**      appropriate health and statistics information for self and each child.
**      This function reuses the packet passed. It frees the old data packet
**      and allocates a new one, changing the length in the header to match.
**
**  In:         IPC_PACKET *ptrPacket       Pointer to the packet
**                                      that contains the request
**
**  Returns:    IPC_PACKET *    Result from this command.  All commands need to
**                          return some type of packet so that the call knows
**                          that the operation worked
**
**--------------------------------------------------------------------------*/
IPC_PACKET *HandleIpcHeartbeat(IPC_PACKET *ptrPacket)
{
    IPC_PACKET_DATA *ptrPacketData;
    UINT32      totalChildren = 0;
    UINT32      index1 = 0;

    /*
     * Compute the total number of children aggregated in this heartbeat, not
     * including self.
     */
    if (leftChild.dataPtr != NULL)
    {
        totalChildren += leftChild.dataPtr->numChildrenAggregated;
    }

    if (rightChild.dataPtr != NULL)
    {
        totalChildren += rightChild.dataPtr->numChildrenAggregated;
    }

    /*
     * Create the response packet
     *  - Change the length in the received header to match
     *    the new length
     *  - Allocate space for packet data
     *  - Free the received data packet
     *  - point the packet to the new data packet
     */
    ptrPacket->header->length = sizeof(IPC_HEARTBEAT) + (totalChildren * sizeof(CHILD_STATUS));

    ptrPacketData = MallocWC(ptrPacket->header->length);

    Free(ptrPacket->data);

    ptrPacket->data = ptrPacketData;

    /*
     * Fill in heartbeat info
     *  - copy heartbeat info for self
     *  - copy heartbeat info for child A (if they exist)
     *  - copy heartbeat info for child B (if they exist)
     */
    ptrPacketData->heartBeat.numChildrenAggregated = totalChildren + 1;
    ptrPacketData->heartBeat.childStatus[0].controllerSN = GetMyControllerSN();
    ptrPacketData->heartBeat.childStatus[0].timeStamp = RTC_GetLongTimeStamp();

    /*
     * Copy data for any existing children
     */
    index1 = 1;
    if (leftChild.dataPtr != NULL)
    {
        memcpy(&ptrPacketData->heartBeat.childStatus[index1],
               &leftChild.dataPtr->childStatus[0],
               leftChild.dataPtr->numChildrenAggregated * sizeof(CHILD_STATUS));
        index1 += leftChild.dataPtr->numChildrenAggregated;
    }

    if (rightChild.dataPtr != NULL)
    {
        memcpy(&ptrPacketData->heartBeat.childStatus[index1],
               &rightChild.dataPtr->childStatus[0],
               rightChild.dataPtr->numChildrenAggregated * sizeof(CHILD_STATUS));
    }

    /*
     * Reset the Watchdog, indicating we saw a heartbeat.
     */
    ResetWatchdog();

    return (ptrPacket);
}


/*----------------------------------------------------------------------------
**  Function Name: RequestIpcHeartbeats
**
**  Comments:
**      Sends a heartbeat request to the specified child of this controller.
**      Saves a pointer to the returned packet data in the specified child.
**
**--------------------------------------------------------------------------*/
void RequestIpcHeartbeat(IPC_HB_CHILD *childPtr)
{
    SESSION    *pSession;
    IPC_PACKET *rx;
    PATH_TYPE   pathType;
    IPC_PACKET *ptrPacket;
    TASK_PARMS  parms;
    UINT8       retries;

    if (childPtr->sn)
    {
        /*
         * Allocate the response packet, setting the header and data
         * pointers to null.
         */
        rx = MallocWC(sizeof(IPC_PACKET));
        rx->header = NULL;
        rx->data = NULL;

        /* Get a session pointer to talk to this child. */
        pSession = GetSession(childPtr->sn);

        if (pSession != NULL)
        {
            /* Create the transmit packet and send it. */
            ptrPacket = CreatePacket(PACKET_IPC_HEARTBEAT, sizeof(IPC_HEARTBEAT), __FILE__, __LINE__);

#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s call IpcSendPacket with rxPacket of %p\n", __FILE__, __LINE__, __func__, rx);
#endif  /* HISTORY_KEEP */

            retries = 2;        /* Ethernet, Fiber(1), Disk Quorum(2) */
            do
            {
                Free(rx->data);

                /* Shove the packet through the networks, 2 sec timeout */
                pathType = IpcSendPacket(pSession, SENDPACKET_ANY_PATH,
                                         ptrPacket, rx, NULL, NULL, NULL, HB_PACKET_TIMEOUT);
            } while (pathType == SENDPACKET_NO_PATH && (retries--) > 0);

            if (!IpcSuccessfulXfer(pathType))
            {
                /*error */
                /*
                 * If heartbeats are still enabled (it may have been
                 * turned off as part of a new election), then report
                 * the failure to failure manager.
                 */
                if (HeartbeatsEnabled())
                {
                    dprintf(DPRINTF_HEALTH_MONITOR, "HB FAILURE: Heartbeat packet failure.\n");

                    parms.p1 = (UINT32)GetMyControllerSN();
                    parms.p2 = (UINT32)childPtr->sn;
                    parms.p3 = (UINT32)IPC_LINK_TYPE_ETHERNET;
                    parms.p4 = (UINT32)PROCESS_CCB;
                    parms.p5 = (UINT32)IPC_LINK_ERROR_NO_HEARTBEAT;
                    UpdateLinkStatus(&parms);
                }

                /* Attempt to restore the fibre path. */
                BringUpInterface(pSession, SENDPACKET_FIBRE);
            }
            else
            {
                /*
                 * Worked
                 * Save heartbeat data for this child.
                 *   - Free the old data (if it exists).
                 *   - set pointer to new data.
                 *   - set packet data pointer to NUll, so it
                 *     is not freed with the packet.
                 */
                if (childPtr->dataPtr != NULL)
                {
                    Free(childPtr->dataPtr);
                }

                childPtr->dataPtr = (IPC_HEARTBEAT *)rx->data;
                rx->data = NULL;

                /*
                 * If the ethernet path was not used, attempt to brinup
                 * the ethernet interface.
                 */
                if (pathType != SENDPACKET_ETHERNET)
                {
                    BringUpInterface(pSession, SENDPACKET_ETHERNET);
                }
            }
            FreePacket(&ptrPacket, __FILE__, __LINE__);
        }
        FreePacket(&rx, __FILE__, __LINE__);
    }
}

/*----------------------------------------------------------------------------
**  Function Name: PingLocalProcessor
**
**  Comments:   This functions executes an awake mrp to the BE processor
**
**  Parameters:     dummy   - allows function to be forked.
**
**
**  Returns:    None
**
**--------------------------------------------------------------------------*/
void NotifyLocalProcessor(UNUSED TASK_PARMS *parms)
{
    UINT16      rc = PI_GOOD;

    /*
     * Notify the backend processor of any status change due to the
     * election results. Indicate whether we are master or slave.
     */
    if (TestforMaster(GetMyControllerSN()))
    {
        rc = MRP_Awake(MRAWAKE_MASTER);
    }
    else
    {
        rc = MRP_Awake(MRAWAKE_SLAVE);
    }
}


/*----------------------------------------------------------------------------
**  Function Name: LocalHealthMonitorTask
**
**  Comments:   This task monitors the health of the connection to the local
**              processors through a periodic ping. If the connection fails,
**              the problem is reported to the failure manager.
**
**--------------------------------------------------------------------------*/
void LocalHealthMonitorTask(void)
{
    EnableLocalMonitor();

    /*
     * Start up a task to handle intra-controller
     * front end health monitoring.
     */
    TaskCreate(LocalHealthMonitorTaskFE, NULL);

    /*
     * Start up a task to handle intra-controller
     * back end health monitoring.
     */
    TaskCreate(LocalHealthMonitorTaskBE, NULL);
}

/*----------------------------------------------------------------------------
**  Function Name: LocalHealthMonitorTaskFE
**
**  Comments:   This task monitors the health of the connection to the front
**              end processor through a periodic ping. If the connection fails,
**              the problem is reported to the failure manager.
**
**--------------------------------------------------------------------------*/
NORETURN void LocalHealthMonitorTaskFE(UNUSED TASK_PARMS *parms)
{
    UINT32      rc;
    UINT32      feState = GOOD;
    TASK_PARMS  intParms;

    while (1)
    {
        /*
         * Exchange and wait of the statistics and heartbeat cycle
         */
        TaskSleepMS(HEALTH_MONITOR_PERIOD);

        if (LocalHealthMonitorEnabled() && ProcessorCommReady(PROCESS_FE))
        {
            /*
             * Ping the  Front end processor to ensure the connection is
             * still alive. If the command fails, invoke the failure
             * manager.
             */
            if ((rc = MRP_Heartbeat(MRFEHBEAT)))
            {
                /*
                 * If health monitoring is still enabled (it may have been
                 * turned off as part of a new election), then report
                 * the failure to failure manager.
                 */
                if (LocalHealthMonitorEnabled() && (feState == GOOD))
                {
                    dprintf(DPRINTF_HEALTH_MONITOR, "HB FAILURE: Front end health monitor -- rc = %u\n", rc);
                    intParms.p1 = (UINT32)GetMyControllerSN();
                    intParms.p2 = (UINT32)GetMyControllerSN();
                    intParms.p3 = (UINT32)IPC_LINK_TYPE_PCI;
                    intParms.p4 = (UINT32)PROCESS_FE;
                    intParms.p5 = (UINT32)IPC_LINK_ERROR_FAILED;
                    UpdateLinkStatus(&intParms);
                    feState = ERROR;
                }
            }
            /*
             * If local heartbeats were failed, but are now ok, indicate
             * the link change.
             */
            else if (LocalHealthMonitorEnabled() && (feState == ERROR))
            {
                dprintf(DPRINTF_HEALTH_MONITOR, "HB: Front end health monitor ok\n");
                intParms.p1 = (UINT32)GetMyControllerSN();
                intParms.p2 = (UINT32)GetMyControllerSN();
                intParms.p3 = (UINT32)IPC_LINK_TYPE_PCI;
                intParms.p4 = (UINT32)PROCESS_FE;
                intParms.p5 = (UINT32)IPC_LINK_ERROR_OK;
                UpdateLinkStatus(&intParms);
                feState = GOOD;
            }
        }
    }
}

/*----------------------------------------------------------------------------
**  Function Name: LocalHealthMonitorTaskBE
**
**  Comments:   This task monitors the health of the connection to the back end
**              processor through a periodic ping. If the connection fails,
**              the problem is reported to the failure manager.
**
**--------------------------------------------------------------------------*/
NORETURN void LocalHealthMonitorTaskBE(UNUSED TASK_PARMS *parms)
{
    UINT32      rc;
    UINT32      beState = GOOD;
    TASK_PARMS  intParms;

    while (1)
    {
        /*
         * Exchange and wait of the statistics and heartbeat cycle
         */
        TaskSleepMS(HEALTH_MONITOR_PERIOD);

        if (LocalHealthMonitorEnabled() && ProcessorCommReady(PROCESS_BE))
        {
            /*
             * Ping the  Back end processor to ensure the connection is
             * still alive. If the command fails, invoke the failure
             * manager.
             */
            if ((rc = MRP_Heartbeat(MRBEHBEAT)))
            {
                /*
                 * If health monitoring is still enabled (it may have been
                 * turned off as part of a new election), then report
                 * the failure to failure manager.
                 */
                if (LocalHealthMonitorEnabled() && (beState == GOOD))
                {
                    dprintf(DPRINTF_HEALTH_MONITOR, "HB FAILURE: Back end health monitor -- rc = %u\n", rc);
                    intParms.p1 = (UINT32)GetMyControllerSN();
                    intParms.p2 = (UINT32)GetMyControllerSN();
                    intParms.p3 = (UINT32)IPC_LINK_TYPE_PCI;
                    intParms.p4 = (UINT32)PROCESS_BE;
                    intParms.p5 = (UINT32)IPC_LINK_ERROR_FAILED;
                    UpdateLinkStatus(&intParms);
                    beState = ERROR;
                }
            }
            /*
             * If local heartbeats were failed, but are now ok, indicate
             * the link change.
             */
            else if (LocalHealthMonitorEnabled() && (beState == ERROR))
            {
                dprintf(DPRINTF_HEALTH_MONITOR, "HB: Back end health monitor ok\n");
                intParms.p1 = (UINT32)GetMyControllerSN();
                intParms.p2 = (UINT32)GetMyControllerSN();
                intParms.p3 = (UINT32)IPC_LINK_TYPE_PCI;
                intParms.p4 = (UINT32)PROCESS_BE;
                intParms.p5 = (UINT32)IPC_LINK_ERROR_OK;
                UpdateLinkStatus(&intParms);
                beState = GOOD;
            }
        }
    }
}

/*----------------------------------------------------------------------------
**  Function Name:  HealthMonitor_ElectionNotify
**
**  Description:    Enable and disable health monitoring based on the state
**                  of the elections.
**
**  Inputs:         None
**
**  Modifies:       Nothing
**
**  Returns:        GOOD  - State switch handled successfully
**                  ERROR - Problem handling the state switch
**--------------------------------------------------------------------------*/
UINT8 HealthMonitor_ElectionNotify(UINT8 newControllerState)
{
    static UINT8 enableHeartbeats = TRUE;
    UINT8       returnCode = GOOD;

    /*
     * Do whatever we need to do to handle the new election state here.
     * NOTE: On any failure, return ERROR; otherwise return GOOD.
     */
    switch (newControllerState)
    {

        /**************************
        * Beginning states
        *************************/
        case ELECTION_NOT_YET_RUN:
        case ELECTION_IN_PROGRESS:
        case ELECTION_STAYING_MASTER:
        case ELECTION_SWITCHING_TO_MASTER:
        case ELECTION_AM_MASTER:
        case ELECTION_STAYING_SLAVE:
        case ELECTION_SWITCHING_TO_SLAVE:
        case ELECTION_AM_SLAVE:
        case ELECTION_STAYING_SINGLE:
        case ELECTION_SWITCHING_TO_SINGLE:
        case ELECTION_AM_SINGLE:
            break;

        case ELECTION_STARTING:
            IPC_HbStop();

            /*
             * Tell the monitor that heartbeats are disabled
             */
            DisableHeartbeats();
            break;

        case ELECTION_INACTIVE:
        case ELECTION_FAILED:
        case ELECTION_DISASTER:
            /*
             * Indicate that when the election completes, that heartbeats
             * should NOT be restarted.
             */
            enableHeartbeats = FALSE;
            break;

        case ELECTION_FINISHED:
            if (enableHeartbeats == TRUE)
            {
                IPC_HbStart();

                /*
                 * Tell the monitor that heartbeats are enabled
                 */
                ResetWatchdog();
                EnableHeartbeats();

                /*
                 * Fork a task to notify Proc of any status change due to the
                 * election results
                 */
                TaskCreate(NotifyLocalProcessor, NULL);
            }
            else
            {
                IPC_HbStop();
            }
            break;

        default:
            returnCode = ERROR;
            break;
    }

    return (returnCode);
}

/*----------------------------------------------------------------------------
**  Function Name:  UpdateLinkStatus
**
**  Description:    Update the state of a communications link with the
**                  Failure Manager / Resource Manager.
**
**  Inputs:         dummy                   Allows funtion to be forked
**                  detectedBySN            Controller finding link failure
**                  destinationSN           Destination of the failed link
**                  linkType                Type of the link that has failed
**                  linkError               Error found on this link
**
**
**
**  Modifies:       Nothing
**
**  Returns:        Nothing
**
**--------------------------------------------------------------------------*/
void UpdateLinkStatus(TASK_PARMS *parms)
{
    UINT32      detectedBySN = parms->p1;       /* Controller finding link failure */
    UINT32      destinationSN = parms->p2;      /* Destination of the failed link  */
    UINT8       linkType = (UINT8)parms->p3;    /* Type of the link that has failed */
    UINT8       processor = (UINT8)parms->p4;   /* processor for link error        */
    UINT32      linkError = parms->p5;  /* Error found on this link        */
    IPC_REPORT_CONTROLLER_FAILURE *pFailurePacket;

    /*
     * Allocate the failure packet.
     * Do not free pFailurePacket - FailureManager takes care of
     * deallocating the packet payload.
     */

    pFailurePacket = (IPC_REPORT_CONTROLLER_FAILURE *)
        MallocWC(SIZEOF_IPC_COMMUNICATIONS_LINK_FAILURE);

    /*
     * Indicate a communications failure
     */
    pFailurePacket->Type = IPC_FAILURE_TYPE_COMMUNICATIONS_FAILED;

    /*
     * Fill in the packet payload.
     */
    pFailurePacket->FailureData.CommunicationsLinkFailure.DetectedBySN = detectedBySN;
    pFailurePacket->FailureData.CommunicationsLinkFailure.DestinationSN = destinationSN;
    pFailurePacket->FailureData.CommunicationsLinkFailure.LinkType = linkType;
    pFailurePacket->FailureData.CommunicationsLinkFailure.LinkError = linkError;
    pFailurePacket->FailureData.CommunicationsLinkFailure.Processor = processor;

#if 0
    dprintf(DPRINTF_HEALTH_MONITOR, "Link Status Change\n");
    dprintf(DPRINTF_HEALTH_MONITOR, "\tDetected SN = %d\n", detectedBySN);
    dprintf(DPRINTF_HEALTH_MONITOR, "\tDestination SN = %d\n", destinationSN);
    dprintf(DPRINTF_HEALTH_MONITOR, "\tLink Type = %d\n", linkType);
    dprintf(DPRINTF_HEALTH_MONITOR, "\tLink Error = %d\n", linkError);
#endif  /* 0 */

    /*
     * Create log message of link status change
     */
    LinkLogMessage(detectedBySN, destinationSN, linkType, linkError, processor);

    /*
     * Call the failure manager to handle this link failure.
     */
    FailureManager(pFailurePacket, SIZEOF_IPC_COMMUNICATIONS_LINK_FAILURE);
}

/*----------------------------------------------------------------------------
**  Function Name:  LinkLogMessage
**
**  Description:    Generate a log message for the link status change.
**
**  Inputs:         detectedBySN            Controller finding link failure
**                  destinationSN           Destination of the failed link
**                  linkType                Type of the link that has failed
**                  linkError               Error found on this link
**                  processor               Processor for link error
**
**
**
**  Modifies:       Nothing
**
**  Returns:        Nothing
**
**--------------------------------------------------------------------------*/
void LinkLogMessage(UINT32 detectedBySN,        /* Controller finding link failure */
                    UINT32 destinationSN,       /* Destination of the failed link  */
                    UINT8 linkType,     /* Type of the link that has failed */
                    UINT32 linkError,   /* Error found on this link        */
                    UINT8 processor)    /* processor for link error        */
{
    LOG_IPC_LINK_UP_PKT logMsg;
    UINT32      evtType = 0;
    UINT32      evtSize = 0;
    UINT8      *pLinkStatus = NULL;

    memset(&logMsg, 0, sizeof(logMsg));

    /*
     * Fill in the log message data, same for UP and DOWN
     */
    logMsg.DetectedBySN = detectedBySN;
    logMsg.DestinationSN = destinationSN;
    logMsg.LinkType = linkType;
    logMsg.LinkError = linkError;

    /*
     * If the link error is zero, the link is up. Otherwise the link is
     * down.
     */
    if (linkError == IPC_LINK_ERROR_OK)
    {
        evtType = LOG_IPC_LINK_UP;
        evtSize = sizeof(LOG_IPC_LINK_UP_PKT);
    }
    else
    {
        evtType = LOG_IPC_LINK_DOWN;
        evtSize = sizeof(LOG_IPC_LINK_DOWN_PKT);
    }

    /*
     * Ethernet link monitor messages are generated in the link monitor function
     * so don't generate another message here.
     */
    if ((linkType == IPC_LINK_TYPE_ETHERNET) && (linkError == IPC_LINK_ERROR_NO_LINK))
    {
        /* no-op */
    }

    /*
     * Unless a PCI link Failure (FE or BE) we do not
     * need to log a connection up/down to ourselves.
     */
    else if ((linkType != IPC_LINK_TYPE_PCI) && (detectedBySN == destinationSN))
    {
        SendAsyncEvent(LOG_Debug(evtType), evtSize, &logMsg);
    }
    else
    {
        /*
         * The plan here is to log link messages as debug if they
         * are not changing states.  If they are changing states,
         * then we want to log them normally, and change the local
         * state of the link.
         */
        switch (linkType)
        {
            case IPC_LINK_TYPE_ETHERNET:
                pLinkStatus = &linkEthernetStatus;
                break;

            case IPC_LINK_TYPE_FIBRE:
                pLinkStatus = &linkFibreStatus;
                break;

            case IPC_LINK_TYPE_QUORUM:
                pLinkStatus = &linkQuorumStatus;
                break;

            case IPC_LINK_TYPE_PCI:
                if (processor == PROCESS_FE)
                {
                    pLinkStatus = &linkFePciStatus;
                }
                else if (processor == PROCESS_BE)
                {
                    pLinkStatus = &linkBePciStatus;
                }
                break;

            default:
                break;
        }

        /*
         * Check to see that the state of the link is changing.
         * If it is then change our local status of the link.
         * If the state is not changing, set it to a debug message.
         */
        if (pLinkStatus != NULL)
        {
            if ((linkError == IPC_LINK_ERROR_OK) && (*pLinkStatus != GOOD))
            {
                *pLinkStatus = GOOD;
            }
            else if ((linkError != IPC_LINK_ERROR_OK) && (*pLinkStatus == GOOD))
            {
                *pLinkStatus = ERROR;
            }
            else
            {
                LOG_SetDebug(evtType);
            }
        }

        /*
         * Send the event.
         */
        SendAsyncEvent(evtType, evtSize, &logMsg);
    }
}

/*----------------------------------------------------------------------------
**  Function Name: MRP_Heartbeat
**
**  Description:
**      Sends the heartbeat MRP to processor specified.
**
**  Inputs:
**      UINT32 commandCode - command code for the type of heartbeat (FE or BE)
**                          MRBEHBEAT - BE heartbeat
**                          MRFEHBEAT - FE heartbeat
**
**      UINT16 step - The firmware step allowed.
**                      0x00 - PING_TEST or mpgping (just ping)
**                      0x01 - PING_NVRAMRDY or mpgnvramrdy (NVRAM can be read up from disk)
**                      0x02 - PING_MASTER or mpgmaster (controller is master)
**                      0x03 - PING_SLAVE or mpgslave (controller is slave)
**
**      UINT32 timeoutMS  - MRP timeout value in Milliseconds
**                            0 - use default mrp timeout
**
**--------------------------------------------------------------------------*/
UINT16 MRP_Heartbeat(UINT32 commandCode)
{
    MRHBEAT_REQ *ptrInPkt = NULL;
    MRHBEAT_RSP *ptrOutPkt = NULL;
    UINT16      rc = PI_GOOD;

    /*
     * Ensure this is a valid ping request. If not, return an error.
     */
    if ((commandCode != MRBEHBEAT) && (commandCode != MRFEHBEAT))
    {
        return (PI_ERROR);
    }

    /*
     * Allocate memory for the MRP input and output packets.
     */
    ptrInPkt = MallocWC(sizeof(*ptrInPkt));
    ptrOutPkt = MallocSharedWC(sizeof(*ptrOutPkt));

    /*
     *  Set the Timestamp
     */
    RTC_GetTimeStamp(&(ptrInPkt->ts));

    /*
     * Send the request to Thunderbolt.  This function handles timeout
     * conditions and task switches while waiting.
     */
    rc = PI_ExecMRP(ptrInPkt, sizeof(*ptrInPkt), commandCode,
                    ptrOutPkt, sizeof(*ptrOutPkt), HB_MRP_TIMEOUT);

    if (rc == PI_GOOD)
    {
        /*
         * If this is a FE request, capture the current the periodic
         * stats (MB/s and IO/s).
         */
        if (commandCode == MRFEHBEAT)
        {
            feIOPerSecond = ptrOutPkt->ioPerSec;
            feBytesPerSecond = ptrOutPkt->bytesPerSec;
            feHeartbeatFlag = 1;
        }
        else
        {
            beHeartbeatFlag = 1;
        }
    }
    else
    {
        /*
         * If the request did not complete, the zero the periodic stats and
         * the heartbeat indicator.
         */
        if (commandCode == MRFEHBEAT)
        {
            feIOPerSecond = 0;
            feBytesPerSecond = 0;
            feHeartbeatFlag = 0;
        }
        else
        {
            beHeartbeatFlag = 0;
        }
    }

    /*
     * Only free the memory if the request did NOT timeout.  On a timeout
     * the memory must remain available in case the request eventually
     * completes.
     */
    if (rc != PI_TIMEOUT)
    {
        Free(ptrOutPkt);
    }

    /*
     * Always free the input packet
     */
    Free(ptrInPkt);

    return (rc);
}


/*----------------------------------------------------------------------------
**  Function Name: IPC_HbStart
**
**  Description:
**      Starts the IPC Heartbeat tasks for any associated children of the
**      this controller.
**
**--------------------------------------------------------------------------*/
void IPC_HbStart(void)
{
    UINT16      myNode;
    UINT16      leftChildNode;
    UINT16      rightChildNode;
    TASK_PARMS  parms;

    if (ACM_GetNodeBySN(Qm_ActiveCntlMapPtr(), &myNode, GetMyControllerSN()) == GOOD)
    {
        if (ACM_GetChildren(myNode, &leftChildNode, &rightChildNode) == GOOD)
        {
            /*
             * If the left child exists, set the running flag and start the
             * HB task for this child.
             */
            if (leftChildNode != ACM_NODE_UNDEFINED &&
                leftChild.running != true &&
                Qm_GetActiveCntlMap(leftChildNode) != ACM_NODE_UNDEFINED)
            {
                leftChild.sn = GetControllerSN(Qm_GetActiveCntlMap(leftChildNode));
                leftChild.running = true;
                parms.p1 = (UINT32)&leftChild;
                TaskCreate(IPC_HbTask, &parms);
            }

            /*
             * If the right child exists, set the running flag and start the
             * HB task for this child.
             */
            if (rightChildNode != ACM_NODE_UNDEFINED &&
                rightChild.running != true &&
                Qm_GetActiveCntlMap(rightChildNode) != ACM_NODE_UNDEFINED)
            {
                rightChild.sn = GetControllerSN(Qm_GetActiveCntlMap(rightChildNode));
                rightChild.running = true;
                parms.p1 = (UINT32)&rightChild;
                TaskCreate(IPC_HbTask, &parms);
            }
        }
    }
}

/*----------------------------------------------------------------------------
**  Function Name: IPC_HbStop
**
**  Description:
**      Stops the IPC Heartbeat tasks for any associated children of the
**      this controller.
**
**--------------------------------------------------------------------------*/
void IPC_HbStop(void)
{
    /*
     * Clear the running flag for each potential child in the
     * HB tree.
     */
    leftChild.running = false;
    rightChild.running = false;
}

/*----------------------------------------------------------------------------
**  Function Name: IPC_HbTask
**
**  Description:
**      Stops the IPC Heartbeat tasks for any associated children of the
**      this controller.
**
**--------------------------------------------------------------------------*/
void IPC_HbTask(TASK_PARMS *parms)
{
    IPC_HB_CHILD *childPtr = (IPC_HB_CHILD *)parms->p1;

    dprintf(DPRINTF_HEALTH_MONITOR, "HB: IPC HB Task STARTED with %d\n", childPtr->sn);

    /*
     * Run until someone stops us
     */

    while (true == childPtr->running)
    {
        TaskSleepMS(HEARTBEAT_PERIOD);

        /*
         * Make sure we are still running, that we heartbeats are still
         * enabled and that we are still part of a VCG.  If so, request
         * heartbeat info from this child.
         */
        if ((true == childPtr->running) && HeartbeatsEnabled() && PartOfVCG())
        {
            RequestIpcHeartbeat(childPtr);

        }
    }

    dprintf(DPRINTF_HEALTH_MONITOR, "HB: IPC HB Task ENDED with %d\n", childPtr->sn);
}


/*----------------------------------------------------------------------------
**  Function Name: GetParentSN(void)
**
**  Description:
**      Get the controller serial number of our parent controller, based on
**      the ACM.
**
**  Returns:    sn - parent controller serial number.
**
**--------------------------------------------------------------------------*/
UINT32 GetParentSN(void)
{
    UINT32      sn = 0;
    UINT16      myNode;
    UINT16      parentNode;

    /*
     * Determine or node, then our parent's node, and from that our
     * parent's serial number.
     */
    if (ACM_GetNodeBySN(Qm_ActiveCntlMapPtr(), &myNode, GetMyControllerSN()) == GOOD)
    {
        if (ACM_GetParent(myNode, &parentNode) == GOOD)
        {
            sn = GetControllerSN(Qm_GetActiveCntlMap(parentNode));
        }
    }

    return (sn);
}


/*----------------------------------------------------------------------------
**  Function Name: GetServerIOPerSecond(void)
**
**  Description:
**      Get the server IOs per second gathered through the heartbeat
**      packet. This value is accumulated across all servers.
**
**  Returns:    IOs per second
**
**--------------------------------------------------------------------------*/
UINT64 GetServerIOPerSecond(void)
{
    return (feIOPerSecond);
}

/*----------------------------------------------------------------------------
**  Function Name: GetServerBytesPerSecond(void)
**
**  Description:
**      Get the server MB per second gathered through the heartbeat
**      packet. This value is accumulated across all servers.
**
**  Returns:    MBs per second
**
**--------------------------------------------------------------------------*/
UINT64 GetServerBytesPerSecond(void)
{
    return (feBytesPerSecond);
}

/*----------------------------------------------------------------------------
**  Function Name: GetFEHeartbeatFlag(void)
**
**  Description:
**      Return the associated heartbeat flag.
**
**  Returns:    Heartbeat flag
**
**--------------------------------------------------------------------------*/
UINT32 GetFEHeartbeatFlag(void)
{
    return (feHeartbeatFlag);
}

/*----------------------------------------------------------------------------
**  Function Name: GetBEHeartbeatFlag(void)
**
**  Description:
**      Return the associated heartbeat flag.
**
**  Returns:    Heartbeat flag
**
**--------------------------------------------------------------------------*/
UINT32 GetBEHeartbeatFlag(void)
{
    return (beHeartbeatFlag);
}


/**
******************************************************************************
**
**  @brief      Task to bring up the a given interface to all controllers
**              within the DSC.
**
**  @param      void* dummy - Dummy parameter required for forked tasks.
**
**  @param      PATH_TYPE type - either SENDPACKET_ETHERNET or
**                               SENDPACKET_FIBRE
**
**  @return     none
**
******************************************************************************
**/
void IPC_BringUpInterfacesTask(TASK_PARMS *parms)
{
    PATH_TYPE   path = (PATH_TYPE)parms->p1;

    EL_BringUpInterfaceAllControllers(path);
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
