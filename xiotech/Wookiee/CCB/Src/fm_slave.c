/* $Id: fm_slave.c 156532 2011-06-24 21:09:44Z m4 $ */
/*============================================================================
** FILE NAME:       fm_slave.c
** MODULE TITLE:    Failure Manager - slave
**
** DESCRIPTION:     This module is responsible for transporting failure
**                  notifications to the master failure manager located
**                  either on this controller (if this is a master controller)
**                  or to the master failure manager on the master controller.
**                  For the most part it is simply a multiplexer.
**
** Copyright (c) 2001-2010 XIOtech Corporation. All rights reserved.
**==========================================================================*/

#include "fm.h"

#include "AsyncEventHandler.h"
#include "debug_files.h"
#include "LOG_Defs.h"
#include "EL.h"
#include "error_handler.h"
#include "idr_structure.h"
#include "ipc_heartbeat.h"
#include "ipc_sendpacket.h"
#include "ipc_session_manager.h"
#include "kernel.h"
#include "logdef.h"
#include "logview.h"
#include "misc.h"
#include "mode.h"
#include "MR_Defs.h"
#include "PI_Utils.h"
#include "PI_Target.h"
#include "PortServer.h"
#include "quorum.h"
#include "quorum_utils.h"
#include "rm.h"
#include "serial_num.h"
#include "slink.h"
#include "sm.h"
#include "XIO_Std.h"
#include "CT_history.h"

/*****************************************************************************
** Private defines
*****************************************************************************/

#define LOOP_HANDLING_TIMEOUT       4000    /* When setting up FE loop handling timeout */
#define PORT_NOTIFY_T1              7000    /* 7 seconds */
#define PORT_NOTIFY_T2              3000    /* 3 seconds */
#define PORT_NOTIFY_T3              500     /* .5 seconds (after loop down, */
                                            /* wait this long before reset) */
#define PORT_NOTIFY_T4              300000  /* 5 minutes(errors in this period) */
#define PORT_NOTIFY_RESET_RETRIES   1       /* Try reset once */
#define PORT_NOTIFY_ERROR_LIMIT     2       /* If more than 2 errors in period, fail */
#define FM_PACKET_TRANSPORT_TIMEOUT 30000   /* Allow failure packets via IPC 30 seconds */

typedef struct _FAILURE_PACKET_LINKS
{
    IPC_REPORT_CONTROLLER_FAILURE *pFailurePacket;
    UINT32      length;
} FAILURE_PACKET_LINKS;

/*****************************************************************************
** Private variables
*****************************************************************************/
static S_LIST *pFailureManagerQueue = NULL;
static PCB *pFailureManagerTask = NULL;
static PCB *pDequeueTaskPCB = NULL;

/* Default to an empty list */
static S_LIST *pPendingFailurePackets = NULL;

/* DO NOT CHANGE these default values! */
static unsigned char electionMaster = TRUE;
static volatile unsigned char controllerMissFeHeartbeat = FALSE;
static volatile unsigned char controllerMissBeHeartbeat = FALSE;

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static void DequeueFailurePacketsTask(TASK_PARMS *parms);
static void FM_FailureManagerQueueTask(TASK_PARMS *parms);      /* FORKED */
static void SendToMasterFailureManager(IPC_REPORT_CONTROLLER_FAILURE *, UINT32 length);
static void SendToMasterMX(IPC_REPORT_CONTROLLER_FAILURE *, UINT32 length);
static void ControllerMissedHeartbeat(TASK_PARMS *parms);
static UINT32 HandleLinkFailure(IPC_REPORT_CONTROLLER_FAILURE *, UINT32 length,
                                LOG_MRP *pLogPacket, LOG_FAILURE_EVENT_PKT *pLogData,
                                UINT8 singleController);
static UINT8 SetLoopHandlingTimeoutsMRP(void);

/*****************************************************************************
** Public variables not defined in any header file.
*****************************************************************************/
#ifndef DISABLE_LOCAL_RAID_MONITORING
extern UINT8 gLocalRaidFlux;
#endif  /* DISABLE_LOCAL_RAID_MONITORING */

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      Function to push a new failure packet link on the failure
**              manager queue and if necessary create and start the failure
**              manager task.
**
**  @param      FAILURE_PACKET_LINKS* pFailurePacketLinks - Failure links
**                  to push on the failure manager queue.
**
**  @return     none
**
******************************************************************************
**/
void EnqueueToFailureManagerQueue(IPC_REPORT_CONTROLLER_FAILURE *pFailurePacket,
                                  UINT32 length)
{
    FAILURE_PACKET_LINKS *pFailurePacketLinks;

    pFailurePacketLinks = MallocWC(sizeof(*pFailurePacketLinks));

    pFailurePacketLinks->pFailurePacket = pFailurePacket;
    pFailurePacketLinks->length = length;

    /* If the Async Event event list has not yet been created, create it. */
    if (pFailureManagerQueue == NULL)
    {
        pFailureManagerQueue = CreateList();
    }

    /*
     * Enqueue this async event on the event list so it will be
     * processed by the ProcessAsyncEvent task.
     */
    Enqueue(pFailureManagerQueue, pFailurePacketLinks);

    /*
     * if the task that processes broadcast events from the queue
     * has not been created, create it
     */
    if (pFailureManagerTask == NULL)
    {
        pFailureManagerTask = TaskCreate(FM_FailureManagerQueueTask, NULL);
    }

    /* Make sure the task is runnable */
    if (TaskGetState(pFailureManagerTask) == PCB_NOT_READY)
    {
        TaskSetState(pFailureManagerTask, PCB_READY);
    }
}


/**
******************************************************************************
**
**  @brief      Task to process the items in the failure manager queue.
**
**  @param      UINT32 dummy - Required dummy parameter for tasks.
**
**  @return     none
**
******************************************************************************
**/
static NORETURN void FM_FailureManagerQueueTask(UNUSED TASK_PARMS *parms)
{
    FAILURE_PACKET_LINKS *pFailurePacketLinks;

    dprintf(DPRINTF_DEFAULT, "FM_FailureManagerQueueTask: Starting\n");

    ccb_assert(pFailureManagerQueue != NULL, pFailureManagerQueue);

    while (FOREVER)
    {
        dprintf(DPRINTF_ASYNC_EVENT_HANDLER, "FM_FailureManagerQueueTask: Items on Queue: 0x%x\n",
                NumberOfItems(pFailureManagerQueue));

        /* Get the first event in the list so we can process it. */
        pFailurePacketLinks = (FAILURE_PACKET_LINKS *)Dequeue(pFailureManagerQueue);

        FailureManager(pFailurePacketLinks->pFailurePacket, pFailurePacketLinks->length);

        /*
         * Free the links structure but do not free the underlying
         * failure packet since failure manager now owns it.
         */
        Free(pFailurePacketLinks);

        /* Exchange now or if no more work, put the task to sleep */
        if (NumberOfItems(pFailureManagerQueue) == 0)
        {
            TaskSetState(pFailureManagerTask, PCB_NOT_READY);
        }

        /* Give control back to the kernel, we learned to share very early */
        TaskSwitch();
    }
}


/*****************************************************************************
**  FUNCTION NAME: FailureManager
**
**  PARAMETERS: pFailurePacket
**              length
**
**  RETURNS:    Nothing
**
**  COMMENTS:   Receives a failure notification.
**              If an election is in progress, pends the packet, if we are a
**              master, sends it to the local master failure manager, if we
**              are not the master controller, sends the packet to the master
**              failure manager on the master controller
**
**              This function is the gatekeeper to all sytemwide FM tasks
**
**  NOTE:       This function is  not re-entered when an election completes
**              in the process of clearing the queue, the dequeue is forked.
******************************************************************************/
void FailureManager(IPC_REPORT_CONTROLLER_FAILURE *pFailurePacket, UINT32 length)
{
    LOG_MRP    *pLogPacket = NULL;
    LOG_FAILURE_EVENT_PKT *pLogData = NULL;
    UINT32      rc = PI_GOOD;
    UINT8       singleController = TRUE;
    UINT32      sentToMaster = FALSE;
    TASK_PARMS  parms;

    PrintFailurePacket(pFailurePacket, false);

    /* Allocate the LOG MRP */
    pLogPacket = MallocSharedWC(sizeof(*pLogPacket));
    pLogData = (LOG_FAILURE_EVENT_PKT *)pLogPacket->mleData;
    pLogPacket->mleLength = SIZE_OF_FAILURE_MLE_LOG_EVENT + length;
    pLogPacket->mleEvent = LOG_LOG_FAILURE_EVENT_INFO;
    memcpy(&pLogData->event, pFailurePacket, length);

    if (!FM_MasterController())
    {
        /* Assume we'll just forward to the master */
        pLogData->action = FAILURE_EVENT_ACTION_FORWARD_TO_MASTER;
    }
    else
    {
        /* We are the master, so don't submit a "forwarding" message to the log */
        pLogData->action = FAILURE_EVENT_ACTION_NONE;
    }

    /* We're a slave. */
    pLogData->action_location = FAILURE_EVENT_ACTION_LOCATION_SLAVE;

    if (ACM_GetActiveControllerCount(Qm_ActiveCntlMapPtr()) > 1)
    {
        singleController = FALSE;
    }

    /*
     * If failure manager is not enabled or a firmware update is in progress,
     * then display a message and exit.
     */
    if (TestModeBit(MD_FM_DISABLE))
    {
        dprintf(DPRINTF_DEFAULT, "SFM: Discarding failure packet - FM disabled\n");

        pLogData->action = FAILURE_EVENT_ACTION_DISCARDED;
        pLogPacket->mleEvent = LOG_LOG_FAILURE_EVENT_WARN;
    }
    else
    {
        /*
         * There are a couple special cases for failures,
         * most are sent to the master
         */
        switch (pFailurePacket->Type)
        {
            case IPC_FAILURE_TYPE_ELECTION_COMPLETE:
                /* Send the packets to wherever they belong */
                if (pPendingFailurePackets != NULL)
                {
                    pLogData->action = FAILURE_EVENT_ACTION_RESUBMIT;
                    pLogPacket->mleEvent = LOG_LOG_FAILURE_EVENT_WARN;
                }
                else
                {
                    /* Nothing to resubmit, so don't log a message */
                    pLogData->action = FAILURE_EVENT_ACTION_NONE;
                }
                break;

            case IPC_FAILURE_TYPE_COMMUNICATIONS_FAILED:

                if (pFailurePacket->FailureData.CommunicationsLinkFailure.LinkError ==
                    IPC_LINK_ERROR_OK)
                {
                    pLogPacket->mleEvent = LOG_LOG_FAILURE_EVENT_INFO;

                    if (pFailurePacket->FailureData.CommunicationsLinkFailure.LinkType ==
                        IPC_LINK_TYPE_PCI)
                    {
                        if (TRUE == HandleLinkFailure(pFailurePacket, length,
                                                      pLogPacket, pLogData,
                                                      singleController))
                        {
                            sentToMaster = TRUE;
                        }
                    }
                    else
                    {
                        /* Handles release of failure packet */
                        SendToMasterMX(pFailurePacket, length);

                        /*
                         * If this link is now ok, attempt to establish a connection
                         * to each controller in the group.
                         */
                        if (pFailurePacket->FailureData.CommunicationsLinkFailure.LinkType == IPC_LINK_TYPE_ETHERNET)
                        {
                            parms.p1 = (UINT32)SENDPACKET_ETHERNET;
                            TaskCreate(IPC_BringUpInterfacesTask, &parms);
                        }
                        else if (pFailurePacket->FailureData.CommunicationsLinkFailure.LinkType == IPC_LINK_TYPE_FIBRE)
                        {
                            parms.p1 = (UINT32)SENDPACKET_FIBRE;
                            TaskCreate(IPC_BringUpInterfacesTask, &parms);
                        }

                        sentToMaster = TRUE;
                    }
                }
                else
                {
                    if (TRUE == HandleLinkFailure(pFailurePacket, length,
                                                  pLogPacket, pLogData, singleController))
                    {
                        sentToMaster = TRUE;
                    }
                }
                break;

                /*
                 * If this controller failed, bring it down,
                 * otherwise just propagate to the master
                 */
            case IPC_FAILURE_TYPE_CONTROLLER_FAILED:
                if (pFailurePacket->FailureData.ControllerFailure.FailedControllerSN == GetMyControllerSN())
                {
                    pLogPacket->mleEvent = LOG_LOG_FAILURE_EVENT;
                    pLogData->action = FAILURE_EVENT_ACTION_FAILED_CONTROLLER;
                    AsyncEventHandler(NULL, pLogPacket);
                    /* This is a bit unnecessary, we'll hang here anyway */
                    LogMessage(LOG_TYPE_DEBUG, "SFM-System going down...(Controller Failed)");
                    DeadLoop(EVENT_CONTROLLER_FAILED, TRUE);
                }
                else
                {
                    pLogPacket->mleEvent = LOG_LOG_FAILURE_EVENT_WARN;

                    SendToMasterMX(pFailurePacket, length);
                    sentToMaster = TRUE;
                }
                break;

            case IPC_FAILURE_TYPE_INTERFACE_FAILED:

                /* Check if the port is failed or okay. */
                if (pFailurePacket->FailureData.InterfaceFailure.InterfaceFailureType != INTERFACE_FAIL_OK)
                {
                    /* The port has failed. */
                    rc = FailPort(pFailurePacket->FailureData.InterfaceFailure.FailedInterfaceID);

                    /* Send error log message. */
                    pLogPacket->mleEvent = LOG_LOG_FAILURE_EVENT;
                    pLogData->action = FAILURE_EVENT_ACTION_FAILED_INTERFACE;
                }
                else
                {
                    /* The port is now okay. */
                    rc = UnfailPort(pFailurePacket->FailureData.InterfaceFailure.FailedInterfaceID);

                    /* Send informational log message. */
                    pLogPacket->mleEvent = LOG_LOG_FAILURE_EVENT_INFO;

                    pLogData->action = FAILURE_EVENT_ACTION_RECOVERED_INTERFACE;
                }

                if (rc != PI_GOOD)
                {
                    /*
                     * When the fail/unfail port is unsuccessful, slave
                     * failure manager notifes master failure manage.
                     */
                    SendToMasterMX(pFailurePacket, length);
                    sentToMaster = TRUE;
                }
                break;

                /*
                 * Regular packets take the default case -
                 * these are not "unhandled" they're just
                 * handled at the master
                 */
            default:
                pLogPacket->mleEvent = LOG_LOG_FAILURE_EVENT_INFO;

                /* Handles release of failure packet */
                SendToMasterMX(pFailurePacket, length);
                sentToMaster = TRUE;
                break;
        }
    }

    /* If we processed the event somehow, so log it. */
    if ((pLogData->action != FAILURE_EVENT_ACTION_NONE) &&
        (pLogData->action != FAILURE_EVENT_ACTION_DONT_HANDLE))
    {
        AsyncEventHandler(NULL, pLogPacket);
    }

    /* Free the LogMRP packet that we allocated above */
    Free(pLogPacket);

    /*
     * If the failure packet didn't get freed above, and it didn't get sent on
     * to the master, free it now.
     */
    if (pFailurePacket && sentToMaster == FALSE)
    {
        dprintf(DPRINTF_FM_SLAVE, "SFM: Free failure packet Addr = %p\n", pFailurePacket);
        Free(pFailurePacket);
    }

    dprintf(DPRINTF_FM_SLAVE, "SFM: EXIT\n");
}


/*****************************************************************************
**  FUNCTION NAME: QueueFailurePacket
**
**  PARAMETERS: FailurePacket
**              length
**
**  RETURNS:    Nothing
**
**  COMMENTS:   If we can't process this packet immediately (an election is
**              in progress and there is no clear master yet) we need to hold
**              off this packet and send it later.
******************************************************************************/
void QueueFailurePacket(IPC_REPORT_CONTROLLER_FAILURE *pFailurePacket, UINT32 length)
{
    FAILURE_PACKET_LINKS *pWorkingFailurePacket;

    dprintf(DPRINTF_FM_SLAVE, "FM: QueueFailurePacket (Addr = 0x%x, Type = 0x%x)\n",
            (UINT32)pFailurePacket, pFailurePacket->Type);

    /* Allocate failure packet link */
    pWorkingFailurePacket = MallocWC(sizeof(*pWorkingFailurePacket));

    /* Add the data payload */
    pWorkingFailurePacket->pFailurePacket = pFailurePacket;

    /* Carry along the length for a retransmission attempt later */
    pWorkingFailurePacket->length = length;

    /* If the pending list is empty, create a new list. */
    if (pPendingFailurePackets == NULL)
    {
        pPendingFailurePackets = CreateList();
    }

    /*
     * Save this failure packet onto the pending list to execute
     * at a later time (first in first out).
     */
    Enqueue(pPendingFailurePackets, pWorkingFailurePacket);

    /* Make sure the dequeue task forked and running */
    if (pDequeueTaskPCB == NULL)
    {
        dprintf(DPRINTF_FM_SLAVE, "FM: Forking DequeueFailurePacketsTask\n");
        pDequeueTaskPCB = TaskCreate(DequeueFailurePacketsTask, NULL);
    }
    else
    {
        dprintf(DPRINTF_FM_SLAVE, "FM: Waking DequeueFailurePacketsTask\n");
        TaskReadyState(pDequeueTaskPCB);
    }
}


/*****************************************************************************
**  FUNCTION NAME: DequeueFailurePacketsTask
**
**  PARAMETERS: Dummy
**
**  RETURNS:    Nothing
**
**  COMMENTS:   After we have a good master, we need to be able to send along
**              all the pended packets.  If an election starts back up mid
**              stream we need to quit and come back later. Code so we can
**              make this a task if we like.
******************************************************************************/
static NORETURN void DequeueFailurePacketsTask(UNUSED TASK_PARMS *parms)
{
    /* Placeholder for the packet while we manipulate the list */
    FAILURE_PACKET_LINKS *pWorkingPacket = NULL;

    while (1)
    {
        dprintf(DPRINTF_FM_SLAVE, "FM: DequeueFailurePacketsTask running\n");

        /* If we have a list, then process the failure packets */
        if (pPendingFailurePackets != NULL)
        {

            /* Loop through all packets, but exit if an election has been started */
            while (NumberOfItems(pPendingFailurePackets))
            {
                /* Wait for the election to complete before processing any packets */
                while (EL_TestInProgress() == TRUE)
                {
                    TaskSleepMS(20);
                }

                /*
                 * Get a pointer to a pointer to a failure packet and a the
                 * length of the failure packet.
                 */
                pWorkingPacket = Dequeue(pPendingFailurePackets);

                /* Ensure we have a good pointer */
                ccb_assert(pWorkingPacket != NULL, pWorkingPacket);

                if (pWorkingPacket != NULL)
                {
                    /* Ensure the pointer to the failure packet is good */
                    ccb_assert(pWorkingPacket->pFailurePacket != NULL,
                               pWorkingPacket->pFailurePacket);

                    if (pWorkingPacket->pFailurePacket != NULL)
                    {
                        dprintf(DPRINTF_FM_SLAVE, "FM: DequeueFailurePacket (Addr = 0x%x, Type = 0x%x)\n",
                                (UINT32)pWorkingPacket->pFailurePacket,
                                pWorkingPacket->pFailurePacket->Type);

                        /* Issue to the failure manager dispatcher */
                        FailureManager(pWorkingPacket->pFailurePacket,
                                       pWorkingPacket->length);

                        /*
                         * Free the packet we just finished with. The actual
                         * failure packet was freed by FM.
                         */
                        Free(pWorkingPacket);

                    }
                }
            }

            /* We processed all the packets. Get rid of the list. */
            DeleteList(pPendingFailurePackets);
            pPendingFailurePackets = NULL;
        }

        /* Put the task to sleep */
        dprintf(DPRINTF_FM_SLAVE, "FM: DequeueFailurePacketsTask going to sleep\n");
        TaskSetState(pDequeueTaskPCB, PCB_NOT_READY);
        TaskSwitch();
    }
}


/*****************************************************************************
**  FUNCTION NAME: SlaveFailure_ElectionNotify
**
**  PARAMETERS: ElectionState
**
**  RETURNS:    char
**
**  COMMENTS:   Receives notification messages from the election process
**              and tracks the current master/slave state.
******************************************************************************/
UINT8 SlaveFailure_ElectionNotify(UINT8 ElectionState)
{
    IPC_REPORT_CONTROLLER_FAILURE *pFailurePacket = NULL;
    UINT8       returnCode = GOOD;

    switch (ElectionState)
    {
        case ELECTION_STARTING:
            break;

        case ELECTION_NOT_YET_RUN:
        case ELECTION_IN_PROGRESS:
        case ELECTION_STAYING_MASTER:
        case ELECTION_SWITCHING_TO_MASTER:
        case ELECTION_STAYING_SINGLE:
        case ELECTION_SWITCHING_TO_SINGLE:
        case ELECTION_AM_MASTER:
        case ELECTION_AM_SINGLE:
        case ELECTION_INACTIVE:
            electionMaster = TRUE;
            break;

        case ELECTION_STAYING_SLAVE:
        case ELECTION_SWITCHING_TO_SLAVE:
        case ELECTION_AM_SLAVE:
            electionMaster = FALSE;
            break;

        case ELECTION_FAILED:
            LogMessage(LOG_TYPE_DEBUG, "SFM-System going down...(Election failed)");
            DeadLoop(EVENT_ELECTION_FAILED, TRUE);
            electionMaster = FALSE;
            break;

        case ELECTION_DISASTER:
            LogMessage(LOG_TYPE_DEBUG, "SFM-System going down...(Disaster during election)");
            DeadLoop(EVENT_DISASTER, TRUE);
            electionMaster = FALSE;
            break;

        case ELECTION_FINISHED:
            FM_ClearWorkingControllers();
            pFailurePacket = MallocWC(SIZEOF_IPC_ELECTION_COMPLETE);
            pFailurePacket->Type = IPC_FAILURE_TYPE_ELECTION_COMPLETE;
            FailureManager(pFailurePacket, SIZEOF_IPC_ELECTION_COMPLETE);
            break;
        default:
            returnCode = ERROR;
            break;
    }

    return (returnCode);
}


/*****************************************************************************
**  FUNCTION NAME: SendToMasterMX
**
**  PARAMETERS: pFailurePacket
**              length
**
**  RETURNS:    Nothing
**
**  COMMENTS:   Serves as multiplexer between local and remote master FM
******************************************************************************/
static void SendToMasterMX(IPC_REPORT_CONTROLLER_FAILURE *pFailurePacket, UINT32 length)
{
    /* If no active controllers, this is not a VCG */
    if (ACM_GetActiveControllerCount(Qm_ActiveCntlMapPtr()) < 2)
    {
        /*
         * This is not a VCG, or we're down to one controller
         * that makes us master by default, ignore the election state
         * and just get on with it.
         */
        MasterFailureManager(pFailurePacket, length);
    }
    else                        /* This is a VCG */
    {
        /* If an election is running, we need to queue */
        if (EL_TestInProgress() == TRUE)
        {
            /* Queue the packets, come back to them later */
            QueueFailurePacket(pFailurePacket, length);
        }
        else
        {
            /* If we're the master */
            if (electionMaster)
            {
                /* Deliver the packet by direct blocking call */
                MasterFailureManager(pFailurePacket, length);
            }
            else
            {
                /*
                 * Deliver via IPC communications, any available path (blocking)
                 * This will take care of freeing the packet if it succeeds
                 */
                SendToMasterFailureManager(pFailurePacket, length);
            }
        }
    }
}


/*****************************************************************************
**  FUNCTION NAME: SendToMasterFailureManager
**
**  PARAMETERS: pFailurePacket
**              length
**
**  RETURNS:    Nothing
**
**  COMMENTS:   Send a failure packet to the master controller master failure
**              manager.  Several things must be kept in mind:
**              1) A failure may occur while we're doing this - if that
**                 happens, pend the packet and start an election
**              2) We should call IPC in a blocking mode
******************************************************************************/
static void SendToMasterFailureManager(IPC_REPORT_CONTROLLER_FAILURE *pFailurePacket,
                                       UINT32 length)
{
    SESSION    *pSession = NULL;
    IPC_PACKET *pRX_Packet = NULL;
    PATH_TYPE   PathType;
    IPC_PACKET *pPacket = NULL;
    UINT8       retries;

    if (pFailurePacket == NULL)
    {
        dprintf(DPRINTF_FM_SLAVE, "SFM: Null packet\n");
        return;
    }

    if (length == 0)
    {
        dprintf(DPRINTF_FM_SLAVE, "SFM: Called with 0 length.\n");
        return;
    }

    /* Get a session to the master */
    pSession = GetSession(Qm_GetMasterControllerSN());

    /* Can't get a session, something went wrong */
    if (pSession == NULL)
    {
        /* Pend the packet for later transmission */
        QueueFailurePacket(pFailurePacket, length);
    }
    else
    {
        dprintf(DPRINTF_FM_SLAVE, "SFM: ->Master (Addr = %p)\n", pFailurePacket);

        /* Create a header/data packet with no data area */
        pPacket = CreatePacket(PACKET_IPC_REPORT_CONTROLLER_FAILURE, 0, __FILE__, __LINE__);
        pPacket->header->length = length;

        /* And attach our prebuilt data packet */
        pPacket->data = (void *)pFailurePacket;

        /* Set up the return packet */
        pRX_Packet = MallocSharedWC(sizeof(IPC_PACKET));

#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s call IpcSendPacket with rxPacket of %p\n", __FILE__, __LINE__, __func__, pRX_Packet);
#endif  /* HISTORY_KEEP */

        retries = 2;        /* Ethernet, Fiber(1), Disk Quorum(2) */
        do
        {
            Free(pRX_Packet->data);

            /* Shove the packet through the networks, 2 sec timeout */
            PathType = IpcSendPacket(pSession, SENDPACKET_ANY_PATH, pPacket,
                                     pRX_Packet, NULL, NULL, 0, FM_PACKET_TRANSPORT_TIMEOUT);
        } while (PathType == SENDPACKET_NO_PATH && (retries--) > 0);


        if (!IpcSuccessfulXfer(PathType))
        {
            /* Packet didn't make it, so....  */

            dprintf(DPRINTF_FM_SLAVE, "SFM: ->Master Xfer failed. (QueueFailurePacket Addr = %p, PathType=%d\n", pFailurePacket, PathType);

            /* Pend the packet for later transmission */
            QueueFailurePacket(pFailurePacket, length);

            if (pPacket->header != NULL)
            {
                /* Free the tx header, the data is the failure packet re-sent above. */
                Free(pPacket->header);
//              pPacket->data = 0;  (The data is the failure packet that is re-sent above -- not needed to do.)
            }
            Free(pPacket);
        }
        else
        {
            /* Free the tx packet - Frees failure packet */
            FreePacket(&pPacket, __FILE__, __LINE__);
        }
        /* Free the return packet */
        FreePacket(&pRX_Packet, __FILE__, __LINE__);
    }
}


/*****************************************************************************
**  FUNCTION NAME: FM_MasterController
**
**  PARAMETERS: None
**
**  RETURNS:    Copy of electionMaster
**
**  COMMENTS:   Private function to return the most recent election state.
******************************************************************************/
unsigned char FM_MasterController(void)
{
    if (ACM_GetActiveControllerCount(Qm_ActiveCntlMapPtr()) < 2)
    {
        return TRUE;
    }
    return electionMaster;
}


/*****************************************************************************
**  FUNCTION NAME: DelayedDeath
**
**  PARAMETERS: Packet
**
**  RETURNS:    Pointer to an IPC packet
**
******************************************************************************/
static NORETURN void DelayedDeath(TASK_PARMS *parms)
{
    UINT32      wait = parms->p1;

    TaskSleepMS(wait);

    /* We've been told to go offline, so do so immediately */
    LogMessage(LOG_TYPE_DEBUG, "DelayedDeath-System going down...(IPC_OFFLINE)");
    DeadLoop(EVENT_IPC_OFFLINE, TRUE);
}


/*****************************************************************************
**  FUNCTION NAME: FM_SlaveOffline
**
**  PARAMETERS: Packet
**
**  RETURNS:    Pointer to an IPC packet
**
**  COMMENTS: Private interface to IPC Command Handler
******************************************************************************/
IPC_PACKET *FM_SlaveOffline(IPC_PACKET *packet)
{
    UINT32      waitTime;
    TASK_PARMS  parms;

    waitTime = packet->data->offline.WaitTime;

    Free(packet->data);

    /* Return a status that the packet was received */
    packet->header->commandCode = PACKET_IPC_COMMAND_STATUS;
    packet->header->length = sizeof(IPC_COMMAND_STATUS);

    /* Allocate a new data packet, the old one is now off limits */
    packet->data = MallocSharedWC(sizeof(IPC_COMMAND_STATUS));
    packet->data->commandStatus.status = IPC_COMMAND_SUCCESSFUL;
    packet->data->commandStatus.errorCodePrimary = IPC_COMMAND_SUCCESSFUL;
    packet->data->commandStatus.errorCodeSecondary = IPC_COMMAND_SUCCESSFUL;

    parms.p1 = (UINT32)waitTime;
    TaskCreate(DelayedDeath, &parms);

    return (packet);
}


/*****************************************************************************
**  FUNCTION NAME: SlaveFailureManagerInit
**
**  PARAMETERS: none
**
**  RETURNS:    none
**
**  COMMENTS:   Public. Sets up the local Slave Failure Manager
******************************************************************************/
void SlaveFailureManagerInit(UNUSED TASK_PARMS *parms)
{
    SetLoopHandlingTimeoutsMRP();
}


/*****************************************************************************
**  FUNCTION NAME: SetLoopHandlingTimeoutsMRP
**
**  PARAMETERS: none
**
**  RETURNS:    TRUE if successful
**
******************************************************************************/
static UINT8 SetLoopHandlingTimeoutsMRP(void)
{
    MRFEPORTNOTIFY_RSP *ptrOutPkt = NULL;
    MRFEPORTNOTIFY_REQ *ptrIn = NULL;

    ptrIn = MallocWC(sizeof(MRFEPORTNOTIFY_REQ));
    ptrOutPkt = MallocSharedWC(sizeof(MRFEPORTNOTIFY_RSP));

    ptrIn->loopDownToNotify = PORT_NOTIFY_T1;
    ptrIn->resetToNotify = PORT_NOTIFY_T2;
    ptrIn->loopDownToReset = PORT_NOTIFY_T3;
    ptrIn->softResetPeriod = PORT_NOTIFY_T4;
    ptrIn->resetRetries = PORT_NOTIFY_RESET_RETRIES;
    ptrIn->errorLimit = PORT_NOTIFY_ERROR_LIMIT;

    switch (PI_ExecMRP(ptrIn, sizeof(MRFEPORTNOTIFY_REQ), MRFEPORTNOTIFY,
                       ptrOutPkt, sizeof(MRFEPORTNOTIFY_RSP), LOOP_HANDLING_TIMEOUT))
    {
        case PI_GOOD:          /* Succeeded */
            dprintf(DPRINTF_DEFAULT, "SFM: Initialized FE loop notification, OK.\n");
            Free(ptrOutPkt);    /* Free the output packet */
            Free(ptrIn);
            return TRUE;

        case PI_TIMEOUT:
            dprintf(DPRINTF_DEFAULT, "SFM: Initialized FE loop notification, failed (timeout).\n");
            /* Do not free the status packet */
            break;

        default:
            dprintf(DPRINTF_DEFAULT, "SFM: Initialized FE loop notification, failed.\n");
            Free(ptrOutPkt);    /* Free the output packet */
            Free(ptrIn);
            break;
    }

    return FALSE;
}


/*****************************************************************************
**  FUNCTION NAME: ControllerMissedHeartbeat
**
**  COMMENTS:   This function is forked in the event that we miss a pci
**              heartbeat and are in a single controller configuration.
**              The object here is to wait an extra heartbeat timeout
**              to give the front or back end a chance to recover in the
**              event that they are busy processing other commands and
**              are not really down.
**
**  PARAMETERS: NONE
**
**  RETURNS:    NONE
**
******************************************************************************/
static void ControllerMissedHeartbeat(TASK_PARMS *parms)
{
    UINT32      timeout = 0;

    /* Give whichever pocessor is down another heartbeat chance to recover. */
    while (((controllerMissBeHeartbeat == TRUE) ||
            (controllerMissFeHeartbeat == TRUE)) && (timeout < parms->p1))
    {
        ++timeout;
        TaskSleepMS(1000);
    }

    /*
     * If we still have not received a heartbeat from the processor, suicide
     * here. Make sure to set the procOnly flag to true as we are a single
     * controller and wish to reset the processes vs. doing a true suicide.
     */
    if ((controllerMissBeHeartbeat == TRUE) || (controllerMissFeHeartbeat == TRUE))
    {
        LogMessage(LOG_TYPE_DEBUG, "SFM-System going down...(PCI/Heartbeat)");
        DeadLoop(EVENT_MISSED_PCI_HEARTBEAT, TRUE);
    }
}



/*****************************************************************************
**  FUNCTION NAME: HandleLinkFailure
**
**  COMMENTS:   This function handles link failures (IPC, PCI, Fibre, quorum).
**              (Function created to simplify readability of the FailurManager).
**
**
**  PARAMETERS: pFailurePacket                  - pointer to a failure packet
**              UINT32  length                  - length of failure packet
**              LOG_MRP* pLogPacket             - pointer to a log packet
**              LOG_FAILURE_EVENT_PKT* pLogData     - pointer to log data
**              UINT8 singleController          - flag for single controller
**
**  RETURNS:    TRUE if pFailurePacket was sent to the master, FALSE otherwise.
**
******************************************************************************/
static UINT32 HandleLinkFailure(IPC_REPORT_CONTROLLER_FAILURE *pFailurePacket,
                                UINT32 length, LOG_MRP *pLogPacket,
                                LOG_FAILURE_EVENT_PKT *pLogData, UINT8 singleController)
{
    UINT32      sentToMaster = FALSE;
    TASK_PARMS  parms;

    /*
     * 1) Fibre -interface- failures go to master
     *      (not handled in this switch)
     * 2) Heartbeat (ethernet or fibre)
     *      failures start an election
     * 3) Log everything else
     * 4) Quorum and PCI errors cause immediate shutdown.
     */

    switch (pFailurePacket->FailureData.CommunicationsLinkFailure.LinkType)
    {
        case IPC_LINK_TYPE_FIBRE:
        case IPC_LINK_TYPE_ETHERNET:
            switch (pFailurePacket->FailureData.CommunicationsLinkFailure.LinkError)
            {
                    /*
                     * Unable to establish heartbeat list for fibre communications.
                     * Just log the fact.
                     */
                case IPC_LINK_ERROR_HB_LIST:
                    if (!RMCheckControllerFailed(pFailurePacket->FailureData.CommunicationsLinkFailure.DestinationSN))
                    {
                        pLogPacket->mleEvent = LOG_LOG_FAILURE_EVENT;
                        pLogData->action = FAILURE_EVENT_ACTION_LOG_ONLY;
                    }
                    break;

                    /* Heartbeat failure */
                case IPC_LINK_ERROR_NO_HEARTBEAT:
                    if (!RMCheckControllerFailed(pFailurePacket->FailureData.CommunicationsLinkFailure.DestinationSN))
                    {
                        int         contCount = 0;

                        /*
                         * We want to know if the link that went down was to a
                         * controller in our VCG. If the controller was in our
                         * VCG, we dneed to run an Election.
                         */
                        contCount = GetCommunicationsSlot(pFailurePacket->FailureData.CommunicationsLinkFailure.DestinationSN);

                        /*
                         * If the link down is to a controller
                         * not in our VCG do nothing.
                         */
                        if (contCount == MAX_CONTROLLERS)
                        {
                            dprintf(DPRINTF_DEFAULT, "SFM: CLF Controller Not in VCG, Failure Manager ignoring.\n");
                            pLogData->action = FAILURE_EVENT_ACTION_DONT_HANDLE;
                        }

                        /*
                         * If the link down is to a controller
                         * in our VCG take action.
                         */
                        else
                        {
                            pLogPacket->mleEvent = LOG_LOG_FAILURE_EVENT_WARN;
                            pLogData->action = FAILURE_EVENT_ACTION_ELECTION;

                            /* Now start an election
                             * There may be a delay before the
                             * election starts if there are
                             * flushes to other controllers
                             * still pending (n-way)
                             */
                            dprintf(DPRINTF_DEFAULT, "SFM: Starting an election.\n");
                            (void)EL_DoElectionNonBlocking();
                        }
                    }
                    break;

                case IPC_LINK_ERROR_FAILED:
                    pLogPacket->mleEvent = LOG_LOG_FAILURE_EVENT_WARN;

                    /* Handles release of failure packet */
                    SendToMasterMX(pFailurePacket, length);
                    sentToMaster = TRUE;
                    break;

                default:
                    pLogData->action = FAILURE_EVENT_ACTION_DONT_HANDLE;
                    break;
            };
            break;

            /*
             * If we lose the heartbeat or have some other PCI error, and are in
             * a multiple controller configuration, go down, unless local raid flux.
             * If we are in a single controller configuration, or in local raid flux,
             * we are going to reset processes.  Call the function to  reset the
             * processor which will give the processes one last chance to revive.
             */
        case IPC_LINK_TYPE_PCI:
            {
                switch (pFailurePacket->FailureData.CommunicationsLinkFailure.LinkError)
                {
                        /*
                         * If either of the missed HB flags are set,
                         * clear them for the specified processor.
                         * We dont want to die anymore.
                         */
                    case IPC_LINK_ERROR_OK:
                        if ((controllerMissFeHeartbeat == TRUE) ||
                            (controllerMissBeHeartbeat == TRUE))
                        {
                            if (pFailurePacket->FailureData.CommunicationsLinkFailure.Processor == PROCESS_FE)
                            {
                                controllerMissFeHeartbeat = FALSE;
                            }

                            if (pFailurePacket->FailureData.CommunicationsLinkFailure.Processor == PROCESS_BE)
                            {
                                controllerMissBeHeartbeat = FALSE;
                            }

                            /* Send the log event */
                            pLogPacket->mleEvent = LOG_LOG_FAILURE_EVENT_INFO;
                            pLogData->action = FAILURE_EVENT_ACTION_LOG_ONLY;
                        }
                        break;

                        /*
                         * If we are a single controller, or we have local raid flux,
                         * set the missed HB flag for the specified processor.
                         * Call the function to delayed reset the processes.  If a HB
                         * comes in during that period, life goes on. :-)
                         *
                         * Else, GO DOWN!
                         */
                    default:
#ifndef DISABLE_LOCAL_RAID_MONITORING
                        if ((gLocalRaidFlux) || (singleController == TRUE))
#else   /* DISABLE_LOCAL_RAID_MONITORING */
                        if (singleController == TRUE)
#endif  /* DISABLE_LOCAL_RAID_MONITORING */
                        {
                            if (pFailurePacket->FailureData.CommunicationsLinkFailure.Processor == PROCESS_FE)
                            {
                                controllerMissFeHeartbeat = TRUE;
                            }

                            if (pFailurePacket->FailureData.CommunicationsLinkFailure.Processor == PROCESS_BE)
                            {
                                controllerMissBeHeartbeat = TRUE;
                            }

                            /* Send the log event */
                            pLogPacket->mleEvent = LOG_LOG_FAILURE_EVENT;
                            pLogData->action = FAILURE_EVENT_ACTION_LOG_ONLY;

#ifndef DISABLE_LOCAL_RAID_MONITORING
                            /* If we have local raid flux, make the TMO 3X longer. */
                            if (gLocalRaidFlux)
                            {
                                parms.p1 = ((HB_MRP_TIMEOUT / 1000) * 3);
                                LogMessage(LOG_TYPE_DEBUG, "SFM-(PCI/Heartbeat) Raids in Flux (%d)",
                                           gLocalRaidFlux);
                            }
                            else
#endif /* DISABLE_LOCAL_RAID_MONITORING */
                            {
                                parms.p1 = (HB_MRP_TIMEOUT / 1000);
                            }

                            /* Fork the delayed DEATH. */
                            TaskCreate(ControllerMissedHeartbeat, &parms);
                        }
                        else
                        {
                            pLogPacket->mleEvent = LOG_LOG_FAILURE_EVENT;

                            pLogData->action = FAILURE_EVENT_ACTION_FAILED_CONTROLLER;

                            AsyncEventHandler(NULL, pLogPacket);

                            LogMessage(LOG_TYPE_DEBUG, "SFM-System going down...(PCI/Heartbeat)");
                            DeadLoop(EVENT_MISSED_PCI_HEARTBEAT, TRUE);
                        }
                        break;
                }
                break;
            }

            /* If we lose the quorum, we must go down */
        case IPC_LINK_TYPE_QUORUM:
            {
                pLogPacket->mleEvent = LOG_LOG_FAILURE_EVENT;
                pLogData->action = FAILURE_EVENT_ACTION_FAILED_CONTROLLER;

                AsyncEventHandler(NULL, pLogPacket);

                /* This is a bit unnecessary, we'll hang here anyway */
                LogMessage(LOG_TYPE_DEBUG, "SFM-System going down...(Quorum failed)");
                DeadLoop(EVENT_QUORUM_FAILURE, TRUE);
                break;
            }

        default:
            pLogPacket->mleEvent = LOG_LOG_FAILURE_EVENT;
            pLogData->action = FAILURE_EVENT_ACTION_DONT_HANDLE;
    }

    return sentToMaster;
}


/*****************************************************************************
**  FUNCTION NAME: PrintFailurePacket
**
**  COMMENTS:   Prints contents of failure packet.
**
**  PARAMETERS: IPC_REPORT_CONTROLLER_FAILURE *pFailurePacket
**              mfm - flag indicating master (1) or slave (0)
**
**  RETURNS:    NONE
**
******************************************************************************/
void PrintFailurePacket(const IPC_REPORT_CONTROLLER_FAILURE *pFailurePacket, bool mfm)
{
    char        tempStr1[15];
    char        tempStr2[15];
    char        preStr[6];

    if (pFailurePacket == NULL)
    {
        return;
    }

    if (mfm == true)
    {
        strncpy(preStr, "MFM", sizeof(preStr));
    }
    else
    {
        strncpy(preStr, "SFM", sizeof(preStr));
    }


    switch (pFailurePacket->Type)
    {
        case IPC_FAILURE_TYPE_INTERFACE_FAILED:
            /* Determine if Interface is OK or Failed */
            if (pFailurePacket->FailureData.InterfaceFailure.InterfaceFailureType ==
                INTERFACE_FAIL_OK)
            {
                strncpy(tempStr1, "OK", sizeof(tempStr1));
            }
            else
            {
                strncpy(tempStr1, "Failure", sizeof(tempStr1));
            }

            dprintf(DPRINTF_DEFAULT, "%s: Interface %s (ID = 0x%x)\n"
                    "> Detecting SN=0x%x, Failing SN=0x%x, Failure type = 0x%x\n",
                    preStr,
                    tempStr1,
                    pFailurePacket->FailureData.InterfaceFailure.FailedInterfaceID,
                    pFailurePacket->FailureData.InterfaceFailure.DetectedBySN,
                    pFailurePacket->FailureData.InterfaceFailure.ControllerSN,
                    pFailurePacket->FailureData.InterfaceFailure.InterfaceFailureType);
            break;

        case IPC_FAILURE_TYPE_COMMUNICATIONS_FAILED:
            /* Convert link type to string */
            switch (pFailurePacket->FailureData.CommunicationsLinkFailure.LinkType)
            {
                case IPC_LINK_TYPE_ETHERNET:
                    strncpy(tempStr1, "Ethernet", sizeof(tempStr1));
                    break;
                case IPC_LINK_TYPE_FIBRE:
                    strncpy(tempStr1, "Fibre", sizeof(tempStr1));
                    break;
                case IPC_LINK_TYPE_QUORUM:
                    strncpy(tempStr1, "Quorum", sizeof(tempStr1));
                    break;
                case IPC_LINK_TYPE_PCI:
                    strncpy(tempStr1, "PCI", sizeof(tempStr1));
                    break;
                default:
                    strncpy(tempStr1, "Unknown", sizeof(tempStr1));
                    break;
            }

            /* Convert link error to string */
            switch (pFailurePacket->FailureData.CommunicationsLinkFailure.LinkError)
            {
                case IPC_LINK_ERROR_OK:
                    strncpy(tempStr2, "OK", sizeof(tempStr2));
                    break;
                case IPC_LINK_ERROR_CREATE_LINK:
                    strncpy(tempStr2, "CREATE LINK", sizeof(tempStr2));
                    break;
                case IPC_LINK_ERROR_FAILED:
                    strncpy(tempStr2, "FAILED", sizeof(tempStr2));
                    break;
                case IPC_LINK_ERROR_NO_LOOP:
                    strncpy(tempStr2, "NO LOOP", sizeof(tempStr2));
                    break;
                case IPC_LINK_ERROR_NO_LINK:
                    strncpy(tempStr2, "NO LINK", sizeof(tempStr2));
                    break;
                case IPC_LINK_ERROR_NO_HEARTBEAT:
                    strncpy(tempStr2, "NO HEARTBEAT", sizeof(tempStr2));
                    break;
                case IPC_LINK_ERROR_HB_LIST:
                    strncpy(tempStr2, "HB LIST", sizeof(tempStr2));
                    break;
                default:
                    strncpy(tempStr1, "Unknown", sizeof(tempStr1));
                    break;
            }

            dprintf(DPRINTF_DEFAULT, "%s: Communications Link (Type = %s, Status = %s)\n"
                    "> Detecting SN=0x%x, Destination SN=0x%x, Processor = 0x%x\n",
                    preStr,
                    tempStr1,
                    tempStr2,
                    pFailurePacket->FailureData.CommunicationsLinkFailure.DetectedBySN,
                    pFailurePacket->FailureData.CommunicationsLinkFailure.DestinationSN,
                    pFailurePacket->FailureData.CommunicationsLinkFailure.Processor);
            break;

        case IPC_FAILURE_TYPE_CONTROLLER_FAILED:
            /* Determine Type of Controller failure */
            if (pFailurePacket->FailureData.ControllerFailure.ErrorType ==
                CONTROLLER_FAILURE_CONFIGURATION_FAILED)
            {
                strncpy(tempStr1, "(Config)", sizeof(tempStr1));
            }
            else
            {
                strncpy(tempStr1, " ", sizeof(tempStr1));
            }

            dprintf(DPRINTF_DEFAULT, "%s: Controller Failure %s\n"
                    "> Detecting SN=0x%x, Failing SN=0x%x, Failure type = 0x%x\n",
                    preStr,
                    tempStr1,
                    pFailurePacket->FailureData.ControllerFailure.DetectedBySN,
                    pFailurePacket->FailureData.ControllerFailure.FailedControllerSN,
                    pFailurePacket->FailureData.ControllerFailure.ErrorType);
            break;

        case IPC_FAILURE_TYPE_ELECTION_COMPLETE:
            dprintf(DPRINTF_DEFAULT, "%s: Election Complete (Election state = 0x%x)\n",
                    preStr, pFailurePacket->FailureData.ElectionComplete.ElectionState);
            break;

        case IPC_FAILURE_TYPE_BATTERY_FAILURE:
            dprintf(DPRINTF_DEFAULT, "%s: Cache Battery State Change (Bank = 0x%x)\n"
                    "> Detecting SN=0x%x, State = 0x%x\n",
                    preStr,
                    pFailurePacket->FailureData.BatteryFailure.BatteryBank,
                    pFailurePacket->FailureData.BatteryFailure.DetectedBySN,
                    pFailurePacket->FailureData.BatteryFailure.BatteryState);
            break;

        default:
            dprintf(DPRINTF_DEFAULT, "%s: Unknown Packet Type (Type = 0x%x)\n",
                    preStr, pFailurePacket->Type);
            break;
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
