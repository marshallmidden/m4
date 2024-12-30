/* $Id: fm_master.c 143766 2010-07-06 12:06:32Z m4 $ */
/*============================================================================
** FILE NAME:       fm_master.c
** MODULE TITLE:    Failure Manager - Master
**
** DESCRIPTION:     This module is intended to support various slave failure
**                  managers requests for resource failures.  The master
**                  failure manager runs on ONLY the master controller, and
**                  feeds requests to the resource manager, which may in turn
**                  reallocate and reconfigure resources as it sees fit.
**
** Copyright (c) 2001-2009 XIOtech Corporation. All rights reserved.
**==========================================================================*/

#include "fm.h"

#include "AsyncEventHandler.h"
#include "debug_files.h"
#include "error_handler.h"
#include "LOG_Defs.h"
#include "EL.h"
#include "ipc_sendpacket.h"
#include "kernel.h"
#include "logdef.h"
#include "quorum.h"
#include "quorum_utils.h"
#include "rm.h"
#include "sm.h"
#include "XIO_Std.h"

/*****************************************************************************
** Private variables
*****************************************************************************/
static UINT32 workingControllers = 0;

/*****************************************************************************
** Private functions
*****************************************************************************/
static void FM_ForceControllerDown(UINT32 controllerSN);
static void FM_ForceControllerDownProcess(TASK_PARMS *parms);

/*****************************************************************************
** Code Start
*****************************************************************************/

/*****************************************************************************
**  FUNCTION NAME: IpcMasterFailureManager
**
**  PARAMETERS: Packet
**
**  RETURNS:    Pointer to an IPC packet
******************************************************************************/
IPC_PACKET *IpcMasterFailureManager(IPC_PACKET *pPacket)
{
    /* Process the packet */
    MasterFailureManager(&pPacket->data->reportControllerFailure,
                         pPacket->header->length);

    /*
     * We've now processed the failure, and RM has
     * returned, so respond back to the slave
     */

    /* The data isn't leaking, it was freed by the master failure manager */
    pPacket->data = NULL;

    /* Return a status that the packet was received */
    pPacket->header->commandCode = PACKET_IPC_COMMAND_STATUS;
    pPacket->header->length = sizeof(IPC_COMMAND_STATUS);
    /* Allocate a new data packet, the old one is now off limits */
    pPacket->data = MallocSharedWC(sizeof(IPC_COMMAND_STATUS));
    pPacket->data->commandStatus.status = IPC_COMMAND_SUCCESSFUL;
    pPacket->data->commandStatus.errorCodePrimary = IPC_COMMAND_SUCCESSFUL;
    pPacket->data->commandStatus.errorCodeSecondary = IPC_COMMAND_SUCCESSFUL;

    return (pPacket);
}


/*****************************************************************************
**  FUNCTION NAME: MasterFailureManager
**
**  PARAMETERS: pFailurePacket
**              length
**
**  RETURNS:    Nothing
******************************************************************************/
void MasterFailureManager(IPC_REPORT_CONTROLLER_FAILURE *pFailurePacket, UINT32 length)
{
    UINT8       singleController = FALSE;
    LOG_MRP    *pLogPacket;
    LOG_FAILURE_EVENT_PKT *pLogData;

    /*
     * Allocate the LOG MRP
     */
    pLogPacket = MallocWC(sizeof(*pLogPacket));
    pLogData = (LOG_FAILURE_EVENT_PKT *)pLogPacket->mleData;
    pLogPacket->mleLength = SIZE_OF_FAILURE_MLE_LOG_EVENT + length;
    pLogPacket->mleEvent = LOG_LOG_FAILURE_EVENT_INFO;
    memcpy(&pLogData->event, pFailurePacket, length);

    /*
     * Assume we'll ignore the event.
     */
    pLogData->action = FAILURE_EVENT_ACTION_NONE;
    pLogData->action_location = FAILURE_EVENT_ACTION_LOCATION_MASTER;

    if (workingControllers == 0)
    {
        workingControllers = ACM_GetActiveControllerCount(Qm_ActiveCntlMapPtr());
    }

    if (workingControllers == 1)
    {
        singleController = TRUE;
    }

    PrintFailurePacket(pFailurePacket, true);

    /*
     * If we are not the master, display a message, queue the packet, and
     * exit. This condition should not happen, but if it does, the packet
     * will be forwarded to the new master or handled when we become master.
     */
    if (!FM_MasterController())
    {
        /*
         * We're not the master, we got caught in
         * a race somewhere - shouldn't happen
         */

#ifdef DEBUG_FM_MASTER
        DebugPrintf("MFM: Master Failure Manager Invoked while not master\n"
                    "     Queue the packet and exit\n");
#endif  /* DEBUG_FM_MASTER */

        QueueFailurePacket(pFailurePacket, length);
        return;
    }

    /*
     * We are the master, process the packets
     */
    switch (pFailurePacket->Type)
    {
            /*
             * An interface has failed or recovered, report to the RM
             */
        case IPC_FAILURE_TYPE_INTERFACE_FAILED:
            /*
             * When the fail/unfail port is unsuccessful, slave failure
             * manager notifes master failure manage.  Start
             * the resource manager realloc task.  The realloc
             * task moves resources between controller.
             */
            RmStartReallocTask();
            break;

        case IPC_FAILURE_TYPE_COMMUNICATIONS_FAILED:
            if (!singleController)
            {
                switch (pFailurePacket->FailureData.CommunicationsLinkFailure.LinkType)
                {
                    case IPC_LINK_TYPE_ETHERNET:
                        break;

                        /*
                         * We only get a fibre failure if all links to a target
                         * are down, so report it as a complete controller
                         * failure
                         */
                    case IPC_LINK_TYPE_FIBRE:
                        if (pFailurePacket->FailureData.CommunicationsLinkFailure.LinkError != IPC_LINK_ERROR_OK)
                        {
                            if (!RMCheckControllerFailed(pFailurePacket->FailureData.CommunicationsLinkFailure.DestinationSN))
                            {
                                dprintf(DPRINTF_DEFAULT, "MFM: Starting an election.\n");
                                (void)EL_DoElectionNonBlocking();

                                pLogPacket->mleEvent = LOG_LOG_FAILURE_EVENT_WARN;
                                pLogData->action = FAILURE_EVENT_ACTION_ELECTION;
                            }
                        }
                        break;

                        /*
                         * If the quorum is unreachable,
                         * declare the controller failed
                         */
                    case IPC_LINK_TYPE_QUORUM:
                        if (pFailurePacket->FailureData.CommunicationsLinkFailure.LinkError != IPC_LINK_ERROR_OK)
                        {
                            if (!RMCheckControllerFailed(pFailurePacket->FailureData.CommunicationsLinkFailure.DestinationSN))
                            {
                                workingControllers--;
                                FM_ForceControllerDown(pFailurePacket->FailureData.CommunicationsLinkFailure.DestinationSN);
                                RMFailController(pFailurePacket->FailureData.CommunicationsLinkFailure.DestinationSN);

                                pLogData->action = FAILURE_EVENT_ACTION_FAILED_CONTROLLER;
                                pLogPacket->mleEvent = LOG_LOG_FAILURE_EVENT;
                            }
                        }
                        break;
                }
            }
            break;

        case IPC_FAILURE_TYPE_CONTROLLER_FAILED:
            /*
             * We should only get this message if we're
             * just throwing up our hands, and can't go on.
             */
            if (!RMCheckControllerFailed(pFailurePacket->FailureData.ControllerFailure.FailedControllerSN))
            {
                workingControllers--;

                WriteFailureDataState(pFailurePacket->FailureData.ControllerFailure.FailedControllerSN,
                                      FD_STATE_FAILED);

                dprintf(DPRINTF_DEFAULT, "MFM: Starting an election.\n");

                (void)EL_DoElectionNonBlocking();

                FM_ForceControllerDown(pFailurePacket->FailureData.ControllerFailure.FailedControllerSN);

                pLogData->action = FAILURE_EVENT_ACTION_FAILED_CONTROLLER;
                pLogPacket->mleEvent = LOG_LOG_FAILURE_EVENT;
            }
            break;

            /*
             * Notify RM of a change in the status of the cache batteries
             */
        case IPC_FAILURE_TYPE_BATTERY_FAILURE:
            LogMessage(LOG_TYPE_DEBUG, "MFM-Battery Failure (State: 0x%x, DetectedBy: 0x%x, Bank: 0x%x)",
                       pFailurePacket->FailureData.BatteryFailure.BatteryState,
                       pFailurePacket->FailureData.BatteryFailure.DetectedBySN,
                       pFailurePacket->FailureData.BatteryFailure.BatteryBank);
            break;

        default:
            break;
    }

    if (pLogData->action != FAILURE_EVENT_ACTION_NONE)
    {
        /*
         * We processed the event somehow, so log it.
         */
        AsyncEventHandler(NULL, pLogPacket);
    }

    Free(pLogPacket);
    Free(pFailurePacket);

    dprintf(DPRINTF_DEFAULT, "MFM: EXIT\n");
}


/*****************************************************************************
**  FUNCTION NAME: FM_ForceControllerDown
**
**  PARAMETERS: ControllerSN
**
**  RETURNS:    Nothing
**
**  COMMENTS: Sends an OFFLINE request to the target controller if the target
**              controller is not this controller.  If this controller is the
**              target, calls suicide.
**               PRIVATE to failure manager.
******************************************************************************/
void FM_ForceControllerDown(UINT32 controllerSN)
{
    TASK_PARMS  parms;

    parms.p1 = controllerSN;
    TaskCreate(FM_ForceControllerDownProcess, &parms);
}

void FM_ForceControllerDownProcess(TASK_PARMS *parms)
{
    UINT32      controllerSN = parms->p1;

    dprintf(DPRINTF_DEFAULT, "Forcing controller %u offline...\n", controllerSN);

    if (GetMyControllerSN() != controllerSN)
    {
        IpcSendOffline(controllerSN, 2000);
    }
    else
    {
        dprintf(DPRINTF_DEFAULT, "MFM: Forced Offline.\n");
        DeadLoop(EVENT_FORCED_OFFLINE, TRUE);
    }
}

/*****************************************************************************
**  FUNCTION NAME: FM_ClearworkingControllers
**
**  PARAMETERS: none
**
**  RETURNS:    Nothing
**
**  COMMENTS: Clears the working controller count used for single/multi actions
**              PRIVATE between master and slave FM.
******************************************************************************/
void FM_ClearWorkingControllers(void)
{
    workingControllers = 0;
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
