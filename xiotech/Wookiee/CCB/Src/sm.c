/* $Id: sm.c 162911 2014-03-20 22:45:34Z marshall_midden $ */
/*============================================================================
** FILE NAME:       sm.c
** MODULE TITLE:    Sequence Manager
**
** DESCRIPTION:     Manages sequences of commands for the resource manager.
**
** Copyright (c) 2002-2009 Xiotech Corporation. All rights reserved.
**==========================================================================*/

#include "CmdLayers.h"
#include "debug_files.h"
#include "LOG_Defs.h"
#include "EL.h"
#include "fibreshim.h"
#include "fm.h"
#include "ipc_session_manager.h"
#include "ipc_sendpacket.h"
#include "kernel.h"
#include "logdef.h"
#include "misc.h"
#include "MR_Defs.h"
#include "PacketInterface.h"
#include "pcb.h"
#include "PI_Utils.h"
#include "PI_WCache.h"
#include "PktCmdHdl.h"
#include "PortServer.h"
#include "proc_hw.h"
#include "serial_num.h"
#include "sm.h"
#include "quorum_utils.h"
#include "rm.h"
#include "rm_val.h"
#include "XIO_Std.h"
#include "X1_AsyncEventHandler.h"
#include "CT_history.h"

/*
******************************************************************************
** Private defines - constants
******************************************************************************
*/

/* Wait 30 seconds after election to set up fe hb list */
#define FE_HB_DELAY         30000

/* Number of 1 second loops to wait before rebuilding the heartbeat list. */
#define FE_HB_DELAY_COUNT   30

/* Number of retries allowed before failing the heartbeat list */
#define HEARTBEAT_TRIES     10

/*
******************************************************************************
** Private variables
******************************************************************************
*/
static UINT8 gDLMHBListPostElectionRun = FALSE;
static UINT8 gSetupDLMHBRunning = FALSE;
static UINT8 gDLMHBListSetup = FALSE;
static UINT8 gRebuildDLMHBListRunning = FALSE;
static PCB *pRebuildHeartbeatListTaskPCB = NULL;
static UINT8 gRebuildHeartbeatListDelayCounter = 0;

/*
******************************************************************************
** Public variables - externed in the header file
******************************************************************************
*/

/* Mirror Partner Mutex */
MUTEX       SM_mpMutex;
UINT32      gCurrentMirrorPartnerSN = 0;

/*
******************************************************************************
** External variables.
******************************************************************************
*/
extern volatile PCB *pRMInitMgrPcb;     /* RM Init/Shutdown manager PCB */

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
static void SM_RebuildHeartbeatListTask(TASK_PARMS *parms);
static void SetupDLMHeartbeatList(TASK_PARMS *parms);
static UINT32 SM_BuildHeartbeatList(UINT32 controllerSN, UINT32 *pList);
static UINT8 SetHeartbeatList(UINT32 *List, UINT32 Count, UINT32 controllerSN);
static UINT8 SetHeartbeatListMRP(UINT32 *List, UINT32 Count);
static INT32 SM_MRResync(UINT8 type, UINT16 ridOrNumRIDs, void *pList, UINT8 *pStatus);
static INT32 SM_FlushErrorWOMP(void);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/*****************************************************************************
**  FUNCTION NAME: SM_Init
**
**  PARAMETERS: None.
**
**  RETURNS:    None.
**
**  COMMENTS:   Public. Called to initialize the SM post election
******************************************************************************/
void SM_Init(UNUSED TASK_PARMS *parms)
{
    /* Wait 30 seconds, then try and set up the list */
    TaskSleepMS(FE_HB_DELAY);
    TaskCreate(SetupDLMHeartbeatList, NULL);
}


/*****************************************************************************
**  FUNCTION NAME: SM_Cleanup
**
**  PARAMETERS: None
**
**  RETURNS:    Nothing. Cleans up when we are no longer master.
**
**  COMMENTS:   Public. This function needs to be called when the RM shuts down
******************************************************************************/
void SM_Cleanup(void)
{
    /* Put any other shutdown cleanup here */
}


/**
******************************************************************************
**
**  @brief      Start the rebuild heartbeat list task.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void SM_RebuildHeartbeatListStart(void)
{
    dprintf(DPRINTF_SM_HB, "SM_RebuildHeartbeatListStart: ENTER\n");

    if (gDLMHBListSetup == TRUE)
    {
        dprintf(DPRINTF_SM_HB, "SM_RebuildHeartbeatListStart: HB list is setup\n");
    }
    else if (gDLMHBListPostElectionRun == FALSE)
    {
        dprintf(DPRINTF_SM_HB, "SM_RebuildHeartbeatListStart: PostElectionRun is FALSE\n");
    }
    else if (EL_GetCurrentElectionState() != ED_STATE_END_TASK)
    {
        dprintf(DPRINTF_SM_HB, "SM_RebuildHeartbeatListStart: Election in progress\n");
    }
    else
    {
        /*
         * Check if the monitor task is already running, if it is not then
         * create the task and set the in progress flag to assume there are
         * operations in progress.
         */
        if (pRebuildHeartbeatListTaskPCB == NULL)
        {
            dprintf(DPRINTF_SM_HB, "SM_RebuildHeartbeatListStart: Forking task\n");

            /*
             * Setup the delay counter, the count here is the number
             * of 1 second loops the task will wait before attemting
             * to rebuild the heartbeat list.
             */
            gRebuildHeartbeatListDelayCounter = FE_HB_DELAY_COUNT;

            /* Create the monitor task. */
            pRebuildHeartbeatListTaskPCB = TaskCreate(SM_RebuildHeartbeatListTask, NULL);
        }
        else
        {
            dprintf(DPRINTF_SM_HB, "SM_RebuildHeartbeatListStart: Task already created\n");

            /*
             * Reset the counter since something else required the
             * list to be rebuilt. This will ensure that the list
             * will be rebuilt 30 seconds after the last event.
             */
            gRebuildHeartbeatListDelayCounter = FE_HB_DELAY_COUNT;
        }
    }
}


/**
******************************************************************************
**
**  @brief      Task to rebuild the heartbeat list - FORKED
**
**  @param      UINT32 dummy - required parameter for forking a task.
**
**  @return     none
**
******************************************************************************
**/
static void SM_RebuildHeartbeatListTask(UNUSED TASK_PARMS *parms)
{
    while (gRebuildHeartbeatListDelayCounter > 0)
    {
        TaskSleepMS(1000);
        gRebuildHeartbeatListDelayCounter--;
    }

    /* If this task is not already running keep going */
    if (!gRebuildDLMHBListRunning)
    {
        gRebuildDLMHBListRunning = TRUE;

        /* If it is already running wait for it to finish. */
        while (gSetupDLMHBRunning)
        {
            dprintf(DPRINTF_SM_HB, "SM_RebuildHeartbeatList: Waiting for current process to finish\n");

            TaskSleepMS(500);
        }

        if (gDLMHBListSetup)
        {
            dprintf(DPRINTF_SM_HB, "SM_RebuildHeartbeatListStart: HB list is setup\n");
        }
        else if (EL_GetCurrentElectionState() != ED_STATE_END_TASK)
        {
            dprintf(DPRINTF_SM_HB, "SM_RebuildHeartbeatList: Election in progress\n");
        }
        else
        {
            dprintf(DPRINTF_SM_HB, "SM_RebuildHeartbeatList: Starting hearbeat list task\n");

            TaskCreate(SetupDLMHeartbeatList, NULL);

            gRebuildDLMHBListRunning = FALSE;
        }
    }

    pRebuildHeartbeatListTaskPCB = NULL;
}


/**
******************************************************************************
**
**  @brief      Build a heartbeat list for the given controller based
**              on the current active controller map.
**
**  @param      UINT32 controllerSN - Controller Serial Number.
**
**  @param      UINT32* pList - Pointer to the start of an array to hold
**                              the heartbeat list. This array must be
**                              large enough to hold the maximum number
**                              of controllers supported by the system.
**                              A good rule would be to allocate the
**                              array based on MAX_CONTROLLERS.
**
**  @return     UINT32 - Returns the number of items placed into the
**                       heartbeat list.
**
**  @attention  The pList must contain enough entries to hold the maximum
**              number of controllers supported. A good rule would be to
**              allocate the array based on MAX_CONTROLLERS.
**
******************************************************************************
**/
static UINT32 SM_BuildHeartbeatList(UINT32 controllerSN, UINT32 *pList)
{
    UINT32      index1;
    UINT32      configIndex;
    UINT32      listIndex = 0;

    /* Scan through the ACM, looking for all the other controllers */
    for (index1 = 0; index1 < Qm_GetNumControllersAllowed(); index1++)
    {
        configIndex = Qm_GetActiveCntlMap(index1);

        if (configIndex == ACM_NODE_UNDEFINED)
        {
            continue;
        }

        /* If this item in the ACM isn't us, add it to the list */
        if (controllerSN != CCM_ControllerSN(configIndex))
        {
            pList[listIndex] = CCM_ControllerSN(configIndex);

            dprintf(DPRINTF_SM_HB, "SM: %u != %u\n",
                    controllerSN, CCM_ControllerSN(configIndex));

            listIndex++;
        }
        else
        {
            dprintf(DPRINTF_SM_HB, "SM: %u == %u\n",
                    controllerSN, CCM_ControllerSN(configIndex));
        }
    }

    /* OK, now List contains the list of serial numbers that aren't controllerSN */
    dprintf(DPRINTF_SM_HB, "SM: Set HB List entries: %u/%u, sending to SN %u\n",
            listIndex, Qm_GetNumControllersAllowed(), controllerSN);

    for (index1 = 0; index1 < listIndex; index1++)
    {
        dprintf(DPRINTF_SM_HB, "SM: HBL: Entry %u - %u\n", index1, pList[index1]);
    }

    dprintf(DPRINTF_SM_HB, "SM: HBL: End of list.\n");

    return listIndex;
}


/*****************************************************************************
**  FUNCTION NAME: SetupDLMHeartbeatList
**
**  PARAMETERS: None.
**
**  RETURNS:    None.
**
**  COMMENTS:   Public. Sends correct heartbeat list to each FE DLM on each
**              controller in the VCG.
******************************************************************************/
static void SetupDLMHeartbeatList(UNUSED TASK_PARMS *parms)
{
    UINT32      index1;
    UINT32      configIndex;
    UINT32      workingSN;
    UINT32      listIndex;
    UINT32     *pList = NULL;

    if (gSetupDLMHBRunning)
    {
        return;
    }

    gSetupDLMHBRunning = TRUE;

    /*
     * Wait for resource manager to get to an idle state.
     * (i.e. finish moving targets after failback)
     */
    if (TestforMaster(GetMyControllerSN()))
    {
        if (pRMInitMgrPcb == NULL)
        {
            rmTelInit(RMINIT);
        }
        RMStateWait(RMRUNNING);
    }

    dprintf(DPRINTF_SM_HB, "SM: Setting up FE DLM heartbeat list...\n");

    /* Only set up the list if there is more than 1 active controller */
    if (Qm_GetNumControllersAllowed() > 1)
    {
        for (index1 = 0; index1 < Qm_GetNumControllersAllowed(); index1++)
        {
            /* These are all the controllers we should be able to talk to. */
            configIndex = Qm_GetActiveCntlMap(index1);

            /* Check for invalid controller index */
            if (configIndex == ACM_NODE_UNDEFINED)
            {
                continue;
            }

            /* Allocate a list (one or more slots too large) */
            pList = MallocWC(MAX_CONTROLLERS * sizeof(UINT32));

            /* This is the SN we're making the list for */
            workingSN = CCM_ControllerSN(configIndex);

            /* Build the heartbeat list for this controller */
            listIndex = SM_BuildHeartbeatList(workingSN, pList);

            /*
             * Send the list to the controller it's for.
             * If there is only one controller (us) there may be
             * an old heartbeat list hanging around. Send down
             * an empty list to avoid a heartbeat failure and
             * extra election on a controller unfail.
             */
            SetHeartbeatList(pList, listIndex, workingSN);

            /* Debug message if we are the only controller left. */
            if (listIndex == 0)
            {
                dprintf(DPRINTF_SM_HB, "SM: HBL: List has zero entries. Send an empty list to ourselves to disable heartbeats.\n");
            }
        }
    }
    else
    {
        dprintf(DPRINTF_SM_HB, "SM: Not an N-Way system. Will not set up FE DLM HB list.\n");
    }

    gSetupDLMHBRunning = FALSE;
    dprintf(DPRINTF_SM_HB, "SM: Finished setting up FE DLM HB list.\n");
}

/*****************************************************************************
**  FUNCTION NAME: SetHeartbeatListMRP
**
**  PARAMETERS: List - List of controllers
**              Count - Number of controllers in list
**
**  RETURNS:    FALSE if fails, else TRUE
**
******************************************************************************/
static UINT8 SetHeartbeatListMRP(UINT32 *pList, UINT32 count)
{
    UINT8       returnValue = FALSE;
    MRFEFIBREHLIST_REQ *ptrInPkt = NULL;
    MRFEFIBREHLIST_RSP *ptrOutPkt = NULL;
    UINT32      packetSize;
    UINT32      attempts = 0;
    UINT8       tryAgain = TRUE;
    IPC_REPORT_CONTROLLER_FAILURE *pFailurePacket;
    INT32       rc = PI_GOOD;

    gDLMHBListSetup = FALSE;

    /* Always allocate space for at least one controller even if count == 0. */
    if (count == 0)
    {
        packetSize = sizeof(*ptrInPkt) + sizeof(UINT32);
    }
    else
    {
        packetSize = sizeof(*ptrInPkt) + (count * sizeof(UINT32));
    }

    while (tryAgain)
    {
        ptrInPkt = MallocWC(packetSize);
        ptrOutPkt = MallocSharedWC(sizeof(*ptrOutPkt));

        ptrInPkt->numControllers = count;

        if (pList != NULL)
        {
            memcpy(ptrInPkt->controllers, pList, (count * sizeof(UINT32)));
        }
        else
        {
            ptrInPkt->controllers[0] = 0;
        }

        rc = PI_ExecMRP(ptrInPkt, packetSize, MRFEFIBREHLIST,
                        ptrOutPkt, sizeof(*ptrOutPkt), (SET_FE_FIBRE_LIST_TIMEOUT / 12));

        if (rc == PI_GOOD)
        {
            returnValue = TRUE;
            tryAgain = FALSE;
            gDLMHBListSetup = TRUE;

            dprintf(DPRINTF_SM_HB, "SM: Set DLM HB List OK. numControllers=%d  List[0]=%d\n",
                    ptrInPkt->numControllers, ptrInPkt->controllers[0]);
        }
        else if (rc == PI_TIMEOUT)
        {
            dprintf(DPRINTF_SM_HB, "SM: Set DLM HB List, failed (timeout).\n");
            tryAgain = FALSE;
        }
        else
        {
            dprintf(DPRINTF_SM_HB, "SM: Set DLM HB List, failed. Try: %u, rc: 0x%x, status: 0x%x, Bad SN: %u\n",
                    attempts, rc, ptrOutPkt->header.status, ptrOutPkt->invalidController);

            if (attempts > HEARTBEAT_TRIES)
            {
                /* Can't find this controller. Call FM to Log the message */
                pFailurePacket = MallocWC(SIZEOF_IPC_COMMUNICATIONS_LINK_FAILURE);
                pFailurePacket->Type = IPC_FAILURE_TYPE_COMMUNICATIONS_FAILED;
                pFailurePacket->FailureData.CommunicationsLinkFailure.DetectedBySN = GetMyControllerSN();
                pFailurePacket->FailureData.CommunicationsLinkFailure.DestinationSN = ptrOutPkt->invalidController;
                pFailurePacket->FailureData.CommunicationsLinkFailure.LinkType = IPC_LINK_TYPE_FIBRE;
                pFailurePacket->FailureData.CommunicationsLinkFailure.Processor = PROCESS_CCB;
                pFailurePacket->FailureData.CommunicationsLinkFailure.LinkError = IPC_LINK_ERROR_HB_LIST;
                FailureManager(pFailurePacket, SIZEOF_IPC_COMMUNICATIONS_LINK_FAILURE);
                /* Don't release the failure packet, FailureManager owns it now */

                tryAgain = FALSE;

                /*
                 * If the fibre path to our mirror partner is not available
                 * and the FE II says that the mirror partner has not yet
                 * been found then we must continue without the mirror
                 * partner.
                 */
                if (CheckFibrePath(gCurrentMirrorPartnerSN) == ERROR &&
                    (GetProcAddress_FEII() != 0 ||
                     (GetProcAddress_FEII()->status & II_STATUS_MPFOUND) == 0))
                {
                    SM_ContinueWithoutMP(GetMyControllerSN());
                }
            }
            else
            {
                attempts++;
                TaskSleepMS(500);       /* Wait 1/2 second, then try again */
                tryAgain = TRUE;        /* Redundant */
            }
        }

        /* Free the allocated memory. */
        Free(ptrInPkt);

        if (rc != PI_TIMEOUT)
        {
            Free(ptrOutPkt);
        }
    }

    gDLMHBListPostElectionRun = TRUE;

    return returnValue;
}


/*****************************************************************************
**  FUNCTION NAME: SetHeartbeatListProcess
**
**  PARAMETERS:
**              List - Array of Controllers for DLM to monitor
**              Count - Number of elements in array
**              TargetSN - The SN of the controller the interface exists on
**              Callback - Callback function of
**                         void Callback(UINT8 Success, UINT32 parameter)
**              parameter - Parameter to pass to callback function
**
**  RETURNS:    FALSE if not successful, else TRUE
**
******************************************************************************/
static UINT8 SetHeartbeatList(UINT32 *pList, UINT32 count, UINT32 targetSN)
{
    IPC_PACKET *pRX_Packet;
    PATH_TYPE   pathType;
    IPC_PACKET *pPacket;
    UINT8       returnValue = FALSE;
    UINT8       retries = 2;                /* Ethernet, Fiber(1), Disk Quorum(2) */

    /* If we need to send to another controller... */
    if (GetMyControllerSN() != targetSN)
    {
        /* Create a header/data packet with sized data area */
        pPacket = CreatePacket(PACKET_IPC_SET_DLM_HEARTBEAT_LIST,
                               sizeof(IPC_SET_DLM_HEARTBEAT_LIST) + (sizeof(UINT32) * (count - 1)),
                               __FILE__, __LINE__);

        /* Set up the return packet */
        pRX_Packet = MallocWC(sizeof(IPC_PACKET));

        memcpy(&pPacket->data->sequencerDLMHeartbeatList.List, pList,
               (count * sizeof(UINT32)));
        pPacket->data->sequencerDLMHeartbeatList.Count = count;

#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s call IpcSendPacketBySN with rxPacket of %p\n", __FILE__, __LINE__, __func__, pRX_Packet);
#endif  /* HISTORY_KEEP */

        do
        {
            Free(pRX_Packet->data);

            /* Shove the packet through the networks, SET_HEARTBEAT_LIST_TIMEOUT timeout */
            pathType = IpcSendPacketBySN(targetSN, SENDPACKET_ANY_PATH,
                                         pPacket, pRX_Packet, NULL, NULL, 0, SET_FE_FIBRE_LIST_TIMEOUT);
        } while (pathType == SENDPACKET_NO_PATH && (retries--) > 0);

        if (!IpcSuccessfulXfer(pathType))
        {
            /* Packet didn't make it, so.... */
            dprintf(DPRINTF_DEFAULT, "SM: IPC Packet Failed (Set DLM HB)\n");

            if (pPacket != NULL)
            {
                Free(pPacket);          /* Free the header  */
            }
            returnValue = FALSE;        /* Indicate failure */
        }
        else
        {
            /* Free the tx packet */
            FreePacket(&pPacket, __FILE__, __LINE__);

            if (pRX_Packet->data->commandStatus.status != IPC_COMMAND_SUCCESSFUL)
            {
                returnValue = FALSE;
            }
            else
            {
                returnValue = TRUE;
            }
            dprintf(DPRINTF_DEFAULT, "SM: IPC Packet Xfer OK (Set DLM HB)\n");
        }

        /* Free the return packet */
        FreePacket(&pRX_Packet, __FILE__, __LINE__);
    }
    else                        /* Just get the MRP issued, it's on the local controller */
    {
        returnValue = SetHeartbeatListMRP(pList, count);
    }

    if (pList != NULL)
    {
        Free(pList);
    }

    return returnValue;
}


/*----------------------------------------------------------------------------
** Function:    ResetInterfaceFE
**
** Description: Reset interface(s) on the FE for the given controller.
**
** Inputs:      UINT32  controllerSN - Serial number of the controller that
**                                     should run the start IO command.
**
** Returns:     PI_GOOD, PI_ERROR, PI_TIMEOUT, this will return PI_GOOD if
**              the packet command completed successfully with an MRP status
**              of DEOK or destopzero.
**--------------------------------------------------------------------------*/
INT32 ResetInterfaceFE(UINT32 controllerSN, UINT8 port, UINT8 option)
{
    XIO_PACKET  reqPacket;
    XIO_PACKET  rspPacket;
    INT32       rc = GOOD;

    dprintf(DPRINTF_DEFAULT, "ResetInterfaceFE - controllerSN=0x%x, channel=0x%x, option=0x%x\n",
            controllerSN, port, option);

    /*
     * Allocate memory for the request (header and data) and the
     * response header. Memory for the response data is allocated
     * in TunnelRequest().
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    reqPacket.pPacket = MallocWC(sizeof(MRRESETFEPORT_REQ));
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    rspPacket.pPacket = NULL;
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    /* Fill in the request header */
    reqPacket.pHeader->commandCode = PI_PROC_RESET_FE_QLOGIC_CMD;
    reqPacket.pHeader->length = sizeof(MRRESETFEPORT_REQ);

    /* Fill in the request parms. */
    ((MRRESETFEPORT_REQ *)(reqPacket.pPacket))->port = port;
    ((MRRESETFEPORT_REQ *)(reqPacket.pPacket))->option = option;

    /*
     * If the port list request is for this controller make the
     * request to the port server directly. If it is for one
     * of the slave controllers then tunnel the request to
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
        dprintf(DPRINTF_DEFAULT, "ResetInterfaceFE - Failed (rc=0x%x).\n", rc);
    }

    /* Free the allocated memory */
    Free(reqPacket.pHeader);
    Free(reqPacket.pPacket);
    Free(rspPacket.pHeader);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPacket.pPacket);
    }

    return rc;
}


/*----------------------------------------------------------------------------
** Function:    StartIO
**
** Description: Starts IO on the given controller with the users parameters.
**
** Inputs:      UINT32  controllerSN - Serial number of the controller that
**                                     should run the start IO command.
**
** Returns:     PI_GOOD, PI_ERROR, PI_TIMEOUT, this will return PI_GOOD if
**              the packet command completed successfully with an MRP status
**              of DEOK or destopzero.
**--------------------------------------------------------------------------*/
INT32 StartIO(UINT32 controllerSN, UINT8 option, UINT8 user, UINT32 tmo)
{
    XIO_PACKET  reqPacket;
    XIO_PACKET  rspPacket;
    INT32       rc = GOOD;

    dprintf(DPRINTF_DEFAULT, "StartIO - controllerSN=0x%x, option=0x%x, user=0x%x\n",
            controllerSN, option, user);

    /*
     * Allocate memory for the request (header and data) and the
     * response header. Memory for the response data is allocated
     * in TunnelRequest().
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    reqPacket.pPacket = MallocWC(sizeof(PI_PROC_START_IO_REQ));
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    rspPacket.pPacket = NULL;
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    /* Fill in the request header */
    reqPacket.pHeader->commandCode = PI_PROC_START_IO_CMD;
    reqPacket.pHeader->length = sizeof(PI_PROC_START_IO_REQ);
    reqPacket.pHeader->timeout = tmo;

    /* Fill in the request parms. */
    ((PI_PROC_START_IO_REQ *)(reqPacket.pPacket))->option = option;
    ((PI_PROC_START_IO_REQ *)(reqPacket.pPacket))->user = user;

    /*
     * If the port list request is for this controller make the
     * request to the port server directly. If it is for one
     * of the slave controllers then tunnel the request to
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

    if (rc == PI_GOOD)
    {
        if (((PI_PROC_START_IO_RSP *)rspPacket.pPacket)->header.status != DEOK &&
            ((PI_PROC_START_IO_RSP *)rspPacket.pPacket)->header.status != DESTOPZERO)
        {
            dprintf(DPRINTF_DEFAULT, "StartIO - Failed to start IO (status=0x%x).\n",
                    ((PI_PROC_START_IO_RSP *)rspPacket.pPacket)->header.status);

            rc = PI_ERROR;
        }
    }
    else
    {
        dprintf(DPRINTF_DEFAULT, "StartIO - Failed to start IO (rc=0x%x).\n", rc);
    }

    /* Free the allocated memory */
    Free(reqPacket.pHeader);
    Free(reqPacket.pPacket);
    Free(rspPacket.pHeader);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPacket.pPacket);
    }

    return rc;
}

/*----------------------------------------------------------------------------
** Function:    StopIO
**
** Description: Stops IO on the given controller with the users parameters.
**
** Inputs:      UINT32  controllerSN - Serial number of the controller that
**                                     should run the stop IO command.
**
** Returns:     PI_GOOD, PI_ERROR, PI_TIMEOUT, this will return PI_GOOD if
**              the packet command completed successfully with an MRP status
**              of DEOK or deoutops.
**--------------------------------------------------------------------------*/
INT32 StopIO(UINT32 controllerSN, UINT8 operation, UINT8 intent, UINT8 user, UINT32 tmo)
{
    XIO_PACKET  reqPacket;
    XIO_PACKET  rspPacket;
    INT32       rc = GOOD;

    dprintf(DPRINTF_DEFAULT, "StopIO - controllerSN=0x%x, operation=0x%x, intent=0x%x, user=0x%x\n",
            controllerSN, operation, intent, user);

    /*
     * Allocate memory for the request (header and data) and the
     * response header. Memory for the response data is allocated
     * in TunnelRequest().
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    reqPacket.pPacket = MallocWC(sizeof(PI_PROC_STOP_IO_REQ));
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    rspPacket.pPacket = NULL;
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    /* Fill in the request header */
    reqPacket.pHeader->commandCode = PI_PROC_STOP_IO_CMD;
    reqPacket.pHeader->length = sizeof(PI_PROC_STOP_IO_REQ);
    reqPacket.pHeader->timeout = tmo;

    /* Fill in the request parms. */
    ((PI_PROC_STOP_IO_REQ *)(reqPacket.pPacket))->operation = operation;
    ((PI_PROC_STOP_IO_REQ *)(reqPacket.pPacket))->intent = intent;
    ((PI_PROC_STOP_IO_REQ *)(reqPacket.pPacket))->user = user;

    /*
     * If the port list request is for this controller make the
     * request to the port server directly. If it is for one
     * of the slave controllers then tunnel the request to
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

    if (rc == PI_GOOD)
    {
        if (((PI_PROC_STOP_IO_RSP *)rspPacket.pPacket)->header.status != DEOK &&
            ((PI_PROC_STOP_IO_RSP *)rspPacket.pPacket)->header.status != DEOUTOPS)
        {
            dprintf(DPRINTF_DEFAULT, "StopIO - Failed to stop IO (status=0x%x).\n",
                    ((PI_PROC_STOP_IO_RSP *)rspPacket.pPacket)->header.status);

            rc = PI_ERROR;
        }
    }
    else
    {
        dprintf(DPRINTF_DEFAULT, "StopIO - Failed to stop IO (rc=0x%x).\n", rc);
    }

    /* Free the allocated memory */
    Free(reqPacket.pHeader);
    Free(reqPacket.pPacket);
    Free(rspPacket.pHeader);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPacket.pPacket);
    }

    return rc;
}


/**
******************************************************************************
**
**  @brief      To provide a handler function for the mirror Get Config Data
**              packet interface request (PI_MiscMirrorPartnerGetCfg).
**
**  @param      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
**  @return     INT32 - Packet return status
**
******************************************************************************
**/
MP_MIRROR_PARTNER_INFO *SM_GetMirrorPartnerConfig(UINT32 partnerSN)
{
    INT32       rc = PI_GOOD;
    XIO_PACKET  reqPacket;
    XIO_PACKET  rspPacket;
    MP_MIRROR_PARTNER_INFO *pMPInfo = NULL;

    dprintf(DPRINTF_DEFAULT, "SM_GetMirrorPartnerConfig-ENTER (0x%x)\n", partnerSN);

    /*
     * Allocate memory for the request (header and data) and the
     * response header. The response data will be allocated in the called
     * function.
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    reqPacket.pPacket = NULL;
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    rspPacket.pPacket = NULL;
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    reqPacket.pHeader->commandCode = PI_MISC_MIRROR_PARTNER_GET_CFG_CMD;
    reqPacket.pHeader->length = 0;

    /*
     * If the request is for this controller send it to the port server
     * directly. If it is for one of the other controllers then tunnel
     * the request to that controller.
     */
    if (partnerSN == GetMyControllerSN())
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
            rc = TunnelRequest(partnerSN, &reqPacket, &rspPacket);
        } while (rc != GOOD && (retries--) > 0);
    }

    /*
     * Check first for an INVALID CMD code -- this indicates that we are
     * talking to a good ol' Bigfoot.
     */
    pMPInfo = MallocWC(sizeof(*pMPInfo));
    if (rc == PI_ERROR && rspPacket.pHeader->status == PI_INVALID_CMD_CODE)
    {
        /* Fill in the config info for Bigfoot, since it can't return it itself. */
        pMPInfo->flags |= MP_CONTROLLER_TYPE_BF;
        dprintf(DPRINTF_DEFAULT, "SM_GetMirrorPartnerConfig: partner is Bigfoot\n");
    }
    else if (rc == PI_GOOD)
    {
        /*
         * Talking to a Wookiee here. Only copy the data that needs to be
         * returned to the caller.
         */
        *pMPInfo = ((PI_MISC_MIRROR_PARTNER_GET_CFG_RSP *)rspPacket.pPacket)->mirrorPartnerInfo;
        dprintf(DPRINTF_DEFAULT, "SM_GetMirrorPartnerConfig: partner is Wookiee\n");
    }
    else                        /* rc != PI_GOOD */
    {
        /* Ain't talking at all here! */
        dprintf(DPRINTF_DEFAULT, "SM_GetMirrorPartnerConfig: Failed request (0x%x)\n", rc);

        /* Free the output struct and return NULL */
        Free(pMPInfo);
        pMPInfo = NULL;
    }

    /* Free the allocated memory (that which we can free) */
    Free(reqPacket.pHeader);
    Free(rspPacket.pHeader);
    if (rc != PI_TIMEOUT)
    {
        Free(rspPacket.pPacket);
    }

    dprintf(DPRINTF_DEFAULT, "SM_GetMirrorPartnerConfig-EXIT (0x%x)\n", partnerSN);

    return pMPInfo;
}

/*****************************************************************************
**  FUNCTION NAME: AssignMirrorPartnerMRP
**
**  PARAMETERS: Partner to assign to the local machine.
**              Address to store old serial number in.
**
**  RETURNS:    FALSE if fails, else TRUE
**
******************************************************************************/
UINT8 AssignMirrorPartnerMRP(UINT32 partnerSN, UINT32 *oldPartnerSN,
                             MP_MIRROR_PARTNER_INFO *pInfo)
{
    UINT8       returnValue = FALSE;
    MRSETMPCONFIGFE_REQ *ptrInPkt;
    MRSETMPCONFIGFE_RSP *ptrOutPkt = NULL;
    INT32       rc = PI_ERROR;
    MP_MIRROR_PARTNER_INFO *pMPInfo;

    dprintf(DPRINTF_DEFAULT, "SM: Assign mirror partner: 0x%x\n", partnerSN);

    /*
     * If the mirror partner information was passed into the function we
     * need to make a copy of it so it can be used for the mirror partner
     * assignment MRP. If not, retrieve the information from the controller.
     */
    if (pInfo != NULL)
    {
        /* Make a copy of the mirror partner information. */
        pMPInfo = MallocW(sizeof(*pMPInfo));
        memcpy(pMPInfo, pInfo, sizeof(*pMPInfo));
    }
    else
    {
        /*
         * Request the mirror partner configuration info from the designated
         * partner.
         */
        pMPInfo = SM_GetMirrorPartnerConfig(partnerSN);
    }

    if (pMPInfo)
    {
        /*
         * Setup the input packet.
         * Note: MRSETMPCONFIGFE_REQ and MP_MIRROR_PARTNER_INFO are equivalent.
         */
        ptrInPkt = (MRSETMPCONFIGFE_REQ *)pMPInfo;
        ptrInPkt->serialNo = partnerSN;

        ptrOutPkt = MallocSharedWC(sizeof(*ptrOutPkt));

        rc = PI_ExecMRP(ptrInPkt, sizeof(*ptrInPkt), MRSETMPCONFIGFE,
                        ptrOutPkt, sizeof(*ptrOutPkt), TMO_NONE);

        if (rc == PI_GOOD)
        {
            /*
             * The mirror partner has been changed. Now we need to check
             * if this assignment affects the state of cache and if we
             * need to set/clear the temporary disable of cache.
             *
             * If the mirror partner is not ourself we can clear the
             * temporary disable.
             *
             * If the mirror partner is ourself then only when there is
             * multiple controllers active do we want to set the temporary
             * disable. Whenever there are multiple controllers active
             * and we are mirroring to ourself, cache must be disabled.
             */
            if (GetMyControllerSN() != partnerSN)
            {
                SM_TempDisableCache(GetMyControllerSN(), PI_MISC_CLRTDISCACHE_CMD,
                                    TEMP_DISABLE_MP, T_DISABLE_CLEAR_ALL);
            }
            else if (ACM_GetActiveControllerCount(Qm_ActiveCntlMapPtr()) > 1 &&
                     GetControllerFailureState() == FD_STATE_OPERATIONAL)
            {
                SM_TempDisableCache(GetMyControllerSN(), PI_MISC_SETTDISCACHE_CMD,
                                    TEMP_DISABLE_MP, 0);
            }

            /* Cache the mirror partner */
            gCurrentMirrorPartnerSN = partnerSN;

            returnValue = TRUE;
            dprintf(DPRINTF_DEFAULT, "SM: Assign mirror partner: 0x%x, OK\n", partnerSN);

            if (oldPartnerSN)
            {
                *oldPartnerSN = ptrOutPkt->oldSerialNumber;
            }
        }
        else
        {
            if (oldPartnerSN)
            {
                *oldPartnerSN = 0;
            }

            dprintf(DPRINTF_DEFAULT, "SM: Assign mirror partner: 0x%x, failed (rc: 0x%x)\n",
                    partnerSN, rc);
        }

        /* Free the allocated memory. */
        Free(pMPInfo);
    }
    else
    {
        dprintf(DPRINTF_DEFAULT, "SM: Assign mirror partner: 0x%x, Couldn't get MP Config info\n",
                partnerSN);
    }

    if (rc != PI_TIMEOUT)
    {
        Free(ptrOutPkt);
    }

    return returnValue;
}


/*****************************************************************************
**  FUNCTION NAME: SM_IPCFlushBECache
**
**  PARAMETERS: Flush BE Cache packet
**
**  RETURNS:    Pointer to a status packet
**
**  COMMENTS:   Private interface between IPC and SM
******************************************************************************/
IPC_PACKET *SM_IPCFlushBECache(IPC_PACKET *pPacket)
{
    IPC_PACKET *pReturnPacket;

    dprintf(DPRINTF_DEFAULT, "SM: Flush BE cache via IPC\n");

    Free(pPacket->data);        /* Discard the data portion of the packet */

    pReturnPacket = pPacket;
    pReturnPacket->header->commandCode = PACKET_IPC_COMMAND_STATUS;
    pReturnPacket->header->length = sizeof(IPC_COMMAND_STATUS);
    pReturnPacket->data = MallocSharedWC(sizeof(IPC_COMMAND_STATUS));

    /* If the flush is good, return success */
    if (WCacheFlushBEWC(GetMyControllerSN()) == PI_GOOD)
    {
        dprintf(DPRINTF_DEFAULT, "SM: Flush BE Cache via IPC succeeded.\n");
        pReturnPacket->data->commandStatus.status = IPC_COMMAND_SUCCESSFUL;
    }
    else                        /* flush didn't succeed, return failure */
    {
        dprintf(DPRINTF_DEFAULT, "SM: Flush BE Cache via IPC failed.\n");
        pReturnPacket->data->commandStatus.status = IPC_COMMAND_FAILED;
    }

    return (pReturnPacket);
}

/*****************************************************************************
**  FUNCTION NAME: SM_IPCContinueWithoutMirrorPartner
**
**  PARAMETERS: ContinueWithoutMirrorPartner packet
**
**  RETURNS:    Pointer to a status packet
**
**  COMMENTS:   Private interface between IPC and SM
**
**              NOTE: This handler will always return a failed IPC
**                    status to ensure that Release 2.0 code and
**                    earlier will correctly call the FlushComplete
**                    routine as expected.
******************************************************************************/
IPC_PACKET *SM_IPCContinueWithoutMirrorPartner(IPC_PACKET *pPacket)
{
    IPC_PACKET *pReturnPacket;

    dprintf(DPRINTF_DEFAULT, "SM: Continue w/o mirror partner via IPC\n");

    Free(pPacket->data);        /* Discard the data portion of the packet */

    pReturnPacket = pPacket;
    pReturnPacket->header->commandCode = PACKET_IPC_COMMAND_STATUS;
    pReturnPacket->header->length = sizeof(IPC_COMMAND_STATUS);
    pReturnPacket->data = MallocSharedWC(sizeof(IPC_COMMAND_STATUS));
    pReturnPacket->data->commandStatus.status = IPC_COMMAND_FAILED;

    SM_ContinueWithoutMP(GetMyControllerSN());

    return (pReturnPacket);
}

/*****************************************************************************
**  FUNCTION NAME: SM_IPCAssignFEMirrorPartner
**
**  PARAMETERS: Assign Mirror Partner packet
**
**  RETURNS:    Pointer to a status packet
**
**  COMMENTS:   Private interface between IPC and SM
******************************************************************************/
IPC_PACKET *SM_IPCAssignFEMirrorPartner(IPC_PACKET *pPacket)
{
    IPC_PACKET *pReturnPacket;
    UINT32      controllerSN;

    dprintf(DPRINTF_DEFAULT, "SM: Assign mirror partner via IPC\n");

    controllerSN = pPacket->data->sequencerSetMirrorPartner.ControllerSN;

    Free(pPacket->data);        /* Discard the data portion of the packet */

    pReturnPacket = pPacket;
    pReturnPacket->header->commandCode = PACKET_IPC_COMMAND_STATUS;
    pReturnPacket->header->length = sizeof(IPC_COMMAND_STATUS);
    pReturnPacket->data = MallocSharedWC(sizeof(IPC_COMMAND_STATUS));

    /* If the assignment is good, return success */
    if (AssignMirrorPartnerMRP(controllerSN, NULL, NULL))
    {
        dprintf(DPRINTF_DEFAULT, "SM: Assign mirror partner via IPC succeeded.\n");
        pReturnPacket->data->commandStatus.status = IPC_COMMAND_SUCCESSFUL;
    }
    else                        /* Assignment didn't succeed, return failure */
    {
        dprintf(DPRINTF_DEFAULT, "SM: Assign mirror partner via IPC failed.\n");
        pReturnPacket->data->commandStatus.status = IPC_COMMAND_FAILED;
    }

    return (pReturnPacket);
}


/**
******************************************************************************
**
**  @brief      Send the flush without mirror partner MRP to the FE.
**
**  @param      none
**
**  @return     INT32 - PI_GOOD if successful, one of the PI errors.
**
******************************************************************************
**/
INT32 SM_FlushWithoutMirrorPartnerMRP(void)
{
    MRFEFLUSHWOMP_RSP *pOutPkt;
    INT32       rc;

    pOutPkt = MallocSharedWC(sizeof(*pOutPkt));

    rc = PI_ExecMRP(NULL, 0, MRFEFLUSHWOMP,
                    pOutPkt, sizeof(*pOutPkt), (CONTINUE_WO_MIRROR_TIMEOUT / 2));

    if (rc == PI_GOOD)
    {
        dprintf(DPRINTF_DEFAULT, "SM: Flush w/o mirror partner, OK.\n");
    }
    else
    {
        dprintf(DPRINTF_DEFAULT, "SM: Flush w/o mirror partner, failed (rc: 0x%x).\n",
                rc);
    }

    /* Free the allocated memory. */
    if (rc != PI_TIMEOUT)
    {
        Free(pOutPkt);
    }

    return rc;
}


/**
******************************************************************************
**
**  @brief      To provide the functionality to send the MRFEFLERRORWOMP
**              request.
**
**  @param      none
**
**  @return     INT32 - Packet return status
**
******************************************************************************
**/
static INT32 SM_FlushErrorWOMP(void)
{
    INT32       rc;
    MRFEFLERRORWOMP_RSP *ptrOutPkt = NULL;

    ptrOutPkt = MallocSharedWC(sizeof(*ptrOutPkt));

    rc = PI_ExecMRP(NULL, 0, MRFEFLERRORWOMP,
                    ptrOutPkt, sizeof(*ptrOutPkt), GetGlobalMRPTimeout());

    if (rc == PI_GOOD)
    {
        dprintf(DPRINTF_DEFAULT, "SM: Flush Error WOMP, OK.\n");
    }
    else
    {
        dprintf(DPRINTF_DEFAULT, "SM: Flush Error WOMP, failed (rc: 0x%x).\n", rc);
    }

    if (rc != PI_TIMEOUT)
    {
        Free(ptrOutPkt);
    }

    return rc;
}


/*****************************************************************************
**  FUNCTION NAME: SM_IPCGetMirrorPartner
**
**  PARAMETERS: None
**
**  RETURNS:    Pointer to a response packet
**
**  COMMENTS:   Private interface between IPC and SM
******************************************************************************/
IPC_PACKET *SM_IPCGetMirrorPartner(IPC_PACKET *pPacket)
{
    IPC_PACKET *pReturnPacket;
    IPC_GET_MIRROR_PARTNER_RESPONSE *pRsp;

    dprintf(DPRINTF_DEFAULT, "SM: Get mirror partner via IPC\n");

    Free(pPacket->data);        /* Discard the data portion of the packet */

    pReturnPacket = pPacket;
    pReturnPacket->header->commandCode = PACKET_IPC_GET_MIRROR_PARTNER_RESPONSE;
    pReturnPacket->header->length = sizeof(*pRsp);

    pRsp = MallocWC(sizeof(*pRsp));
    pRsp->PartnerSN = GetCachedMirrorPartnerSN();
    pRsp->Status = PI_GOOD;
    pRsp->MySN = GetMyControllerSN();
    pReturnPacket->data = (void *)pRsp;

    return (pReturnPacket);
}


/*****************************************************************************
**  FUNCTION NAME: SM_IPCSetHeartbeatList
**
**  PARAMETERS: Set Heartbeat List packet
**
**  RETURNS:    Pointer to a status packet
**
**  COMMENTS:   Private interface between IPC and SM
******************************************************************************/
IPC_PACKET *SM_IPCSetHeartbeatList(IPC_PACKET *pPacket)
{
    IPC_PACKET *pReturnPacket;
    UINT32     *pList;
    UINT32      count;
    UINT32      size;

    count = pPacket->data->sequencerDLMHeartbeatList.Count;
    if (count == 0)
    {
        size = 0;
        pList = NULL;
    }
    else
    {
        size = count * sizeof(UINT32);
        pList = MallocWC(size);
        memcpy(pList, pPacket->data->sequencerDLMHeartbeatList.List, size);
    }

    Free(pPacket->data);        /* Discard the data portion of the packet */

    pReturnPacket = pPacket;
    pReturnPacket->header->commandCode = PACKET_IPC_COMMAND_STATUS;
    pReturnPacket->header->length = sizeof(IPC_COMMAND_STATUS);
    pReturnPacket->data = MallocSharedWC(sizeof(IPC_COMMAND_STATUS));

    /* If the set is good, return success */
    if (SetHeartbeatListMRP(pList, count))
    {
        dprintf(DPRINTF_DEFAULT, "SM: Set DLM HB List via IPC succeeded.\n");
        pReturnPacket->data->commandStatus.status = IPC_COMMAND_SUCCESSFUL;

    }
    else                        /* set didn't succeed, return failure */
    {
        dprintf(DPRINTF_DEFAULT, "SM: Set DLM HB List via IPC failed.\n");
        pReturnPacket->data->commandStatus.status = IPC_COMMAND_FAILED;
    }

    Free(pList);

    return (pReturnPacket);
}


/*****************************************************************************
**  FUNCTION NAME: SM_IPCFlushCompleted
**
**  PARAMETERS: Flush completed packet
**
**  RETURNS:    Pointer to a status packet
**
**  COMMENTS:   Private interface between IPC and SM
******************************************************************************/
IPC_PACKET *SM_IPCFlushCompleted(IPC_PACKET *pPacket)
{
    Free(pPacket->data);

    pPacket->header->commandCode = PACKET_IPC_COMMAND_STATUS;
    pPacket->header->length = sizeof(IPC_COMMAND_STATUS);
    pPacket->data = MallocSharedWC(sizeof(IPC_COMMAND_STATUS));
    pPacket->data->commandStatus.status = IPC_COMMAND_SUCCESSFUL;

    return (pPacket);
}


/**
******************************************************************************
**
**  @brief      To provide a consistent means of sending the target
**              control request to a given controller. This function
**              uses the packet interface and tunnel requests to send
**              the request locally or remotely (respectively).
**
**  @param      UINT32 controllerSN - Controller to run the target
**                                    control request.
**  @param      UINT16 option - Option to pass to the target control.
**
**  @return     INT32 - Packet return status
**
******************************************************************************
**/
PI_PROC_TARGET_CONTROL_RSP *SM_TargetControl(UINT32 controllerSN, UINT16 option)
{
    XIO_PACKET  reqPacket;
    XIO_PACKET  rspPacket;
    INT32       rc = GOOD;
    PI_PROC_TARGET_CONTROL_RSP *pResponse = NULL;

    dprintf(DPRINTF_DEFAULT, "SM_TargetControl: ENTER (0x%x, 0x%x))\n", controllerSN,
            option);

    /*
     * Allocate memory for the request (header and data) and the
     * response header. Memory for the response data is allocated
     * in TunnelRequest().
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    reqPacket.pPacket = MallocWC(sizeof(PI_PROC_TARGET_CONTROL_REQ));
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    rspPacket.pPacket = NULL;
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    /* Fill in the request header */
    reqPacket.pHeader->commandCode = PI_PROC_TARGET_CONTROL_CMD;
    reqPacket.pHeader->length = sizeof(PI_PROC_TARGET_CONTROL_REQ);

    /* Fill in the request parms. */
    ((PI_PROC_TARGET_CONTROL_REQ *)(reqPacket.pPacket))->option = option;

    /*
     * If the port list request is for this controller make the
     * request to the port server directly. If it is for one
     * of the slave controllers then tunnel the request to
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

    /*
     * If the request completed successfully, save the response data
     * pointer to return it and clear the pointer in the rspPacket.
     */
    if (rc == PI_GOOD)
    {
        pResponse = (PI_PROC_TARGET_CONTROL_RSP *)rspPacket.pPacket;
        rspPacket.pPacket = NULL;
    }

    /* Free the allocated memory */
    Free(reqPacket.pHeader);
    Free(reqPacket.pPacket);
    Free(rspPacket.pHeader);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPacket.pPacket);
    }

    dprintf(DPRINTF_DEFAULT, "SM_TargetControl: EXIT (0x%x, 0x%x, 0x%x)\n", controllerSN, option, rc);

    return pResponse;
}


/**
******************************************************************************
**
**  @brief      To provide a consistent means of sending the request to
**              Resync RAID 5 Mirror Records (stripe or full) on a given
**              controller. This function uses the packet interface and
**              tunnel requests to send the request locally or remotely
**              (respectively).
**
**  @param      UINT32 controllerSN - Controller that needs to run the
**                                    resync operation.
**  @param      UINT8 type - Type of resync operation (see MR_Defs.h).
**  @param      UINT16 rid - Raid to resync (only valid for MRBONERAID
**                           resync type).
**
**  @return     INT32 - Packet return status
**
******************************************************************************
**/
INT32 SM_ResyncMirrorRecords(UINT32 controllerSN, UINT8 type, UINT16 rid)
{
    INT32       rc = PI_GOOD;
    XIO_PACKET  reqPacket;
    XIO_PACKET  rspPacket;

    dprintf(DPRINTF_DEFAULT, "SM_ResyncMirrorRecords: ENTER (0x%x, 0x%x, 0x%x)\n",
            controllerSN, type, rid);

    /*
     * Allocate memory for the request (header and data) and the
     * response header. The response data will be allocated in the called
     * function.
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    reqPacket.pPacket = MallocWC(sizeof(PI_MISC_RESYNC_MIRROR_RECORDS_REQ));
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    rspPacket.pPacket = NULL;
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    /* Fill in the request header */
    reqPacket.pHeader->commandCode = PI_MISC_RESYNC_MIRROR_RECORDS_CMD;
    reqPacket.pHeader->length = sizeof(PI_MISC_RESYNC_MIRROR_RECORDS_REQ);

    /* Fill in the request parms. */
    ((PI_MISC_RESYNC_MIRROR_RECORDS_REQ *)(reqPacket.pPacket))->type = type;
    ((PI_MISC_RESYNC_MIRROR_RECORDS_REQ *)(reqPacket.pPacket))->rid = rid;

    /*
     * If the request is for this controller send it to the port server
     * directly. If it is for one of the other controllers then tunnel
     * the request to that controller.
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
        dprintf(DPRINTF_DEFAULT, "SM_ResyncMirrorRecords: Failed request (0x%x)\n", rc);
    }

    /* Free the allocated memory */
    Free(reqPacket.pHeader);
    Free(reqPacket.pPacket);
    Free(rspPacket.pHeader);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPacket.pPacket);
    }

    return rc;
}


/**
******************************************************************************
**
**  @brief      To provide a consistent means of sending the request to
**              Raid Resync RAID 5 Mirror Records on a given controller.
**              This function uses the packet interface and
**              tunnel requests to send the request locally or remotely
**              (respectively).
**
**  @param      UINT32 controllerSN - Controller that needs to run the
**                                    resync operation.
**  @param      MR_LIST_RSP* pList - List response containing an array
**                                   of raid identifiers that need to
**                                   be resync'd.
**
**  @return     INT32 - Packet return status
**
******************************************************************************
**/
INT32 SM_RaidResyncController(UINT32 controllerSN, MR_LIST_RSP *pList)
{
    INT32       rc = PI_GOOD;
    UINT32      reqSize;
    XIO_PACKET  reqPacket;
    XIO_PACKET  rspPacket;

    dprintf(DPRINTF_DEFAULT, "SM_RaidResyncController: ENTER (0x%x, 0x%x)\n",
            controllerSN, pList->ndevs);

    reqSize = sizeof(PI_MISC_RESYNC_RAIDS_REQ) + (pList->ndevs * sizeof(UINT16));

    /*
     * Allocate memory for the request (header and data) and the
     * response header. The response data will be allocated in the called
     * function.
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    reqPacket.pPacket = MallocWC(reqSize);
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    rspPacket.pPacket = NULL;
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    /* Fill in the request header */
    reqPacket.pHeader->commandCode = PI_MISC_RESYNC_RAIDS_CMD;
    reqPacket.pHeader->length = reqSize;

    /* Fill in the request parms. */
    ((PI_MISC_RESYNC_RAIDS_REQ *)(reqPacket.pPacket))->count = pList->ndevs;
    memcpy(&((PI_MISC_RESYNC_RAIDS_REQ *)(reqPacket.pPacket))->raids,
           &pList->list, (pList->ndevs * (sizeof(UINT16))));

    /*
     * If the request is for this controller send it to the port server
     * directly. If it is for one of the other controllers then tunnel
     * the request to that controller.
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
        dprintf(DPRINTF_DEFAULT, "SM_RaidResyncController: Failed request (0x%x)\n", rc);
    }

    /* Free the allocated memory */
    Free(reqPacket.pHeader);
    Free(reqPacket.pPacket);
    Free(rspPacket.pHeader);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPacket.pPacket);
    }

    return rc;
}


/**
******************************************************************************
**
**  @brief      Retrieve the mirror partner list from the PROC.
**
**  @param      none
**
**  @return     NULL or pointer to a mirror partner list response packet.
**
******************************************************************************
**/
PI_VCG_GET_MP_LIST_RSP *SM_GetMirrorPartnerList(void)
{
    UINT32      rc = PI_GOOD;
    XIO_PACKET  reqPacket;
    XIO_PACKET  rspPacket;
    PI_VCG_GET_MP_LIST_RSP *pResponse = NULL;

    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    reqPacket.pPacket = MallocWC(sizeof(PI_VCG_GET_MP_LIST_REQ));
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    rspPacket.pPacket = NULL;
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    reqPacket.pHeader->commandCode = PI_VCG_GET_MP_LIST_CMD;
    reqPacket.pHeader->length = sizeof(PI_VCG_GET_MP_LIST_REQ);

    rc = PortServerCommandHandler(&reqPacket, &rspPacket);

    if (rc == PI_GOOD)
    {
        ccb_assert(rspPacket.pPacket != NULL, rspPacket.pPacket);

        pResponse = (PI_VCG_GET_MP_LIST_RSP *)rspPacket.pPacket;
        rspPacket.pPacket = NULL;
    }
    else
    {
        dprintf(DPRINTF_RM, "SM_GetMirrorPartnerList - Failed (rc=0x%x).\n", rc);
    }

    /* Free the allocated memory */
    Free(reqPacket.pHeader);
    Free(reqPacket.pPacket);
    Free(rspPacket.pHeader);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPacket.pPacket);
    }

    return pResponse;
}


/**
******************************************************************************
**
**  @brief      To provide a consistent means of sending the MRRESYNC request
**              to the BE processor. This function will ensure that the
**              request or similar request completes without error. If the
**              original request was for something other than the MRBALLRAIDS
**              option this function will convert the option to MRBALLRAIDS
**              if an error other than DEOUTOPS is returned from the MRP.
**              This will ensure that we will get at least all raids started
**              for resync even if the original request fails to setup a
**              smaller subset of the raids. This is just a safeguard to
**              catch unexpected errors.
**
**  @param      type - Type of reset operation
**              ridOrNumRIDs - RAID Identifier to resync if a single RAID or
**                             the number of RIDs in a List
**              pList - Pointer to a buffer containing the either the NVA data
**                      or a List of RIDs to Resync (CCB memory address,
**                      not PCI).
**
**  @return     INT32 - Packet return status
**
******************************************************************************
**/
INT32 SM_MRResyncWithRetry(UINT8 type, UINT16 ridOrNumRIDs, void *pList)
{
    INT32       rc = PI_GOOD;
    UINT8       status;

    LogMessage(LOG_TYPE_DEBUG, "SM_MRResyncWithRetry-ENTER (0x%x, 0x%x)",
               type, ridOrNumRIDs);

    do
    {
        /*
         * If we have encountered a timeout on a previous request we will
         * loop here until the blocked state of the BE processor is cleared.
         *
         * If we have encountered an outstanding operations error we will
         * delay for a bit before continuing.
         */
        if (rc == PI_TIMEOUT)
        {
            /* Loop until the BE is no longer blocked. */
            while (BEBlocked())
            {
                TaskSleepMS(500);
            }
        }
        else if (rc == PI_ERROR && status == DEOUTOPS)
        {
            TaskSleepMS(500);
        }

        /* Send the resync request. */
        rc = SM_MRResync(type, ridOrNumRIDs, pList, &status);

        /*
         * If the request returned an error and it was not the outstanding
         * ops error, clear the error and make sure type is set to resync
         * all raids as a fall back operation.
         */
        if (rc == PI_ERROR && status != DEOUTOPS)
        {
            if (type != MRBALLRAIDS)
            {
                LogMessage(LOG_TYPE_DEBUG, "SM_MRResyncWithRetry-Changing type (0x%x->0x%x)",
                           type, MRBALLRAIDS);

                type = MRBALLRAIDS;
            }

            /*
             * Set the status to outstanding operations to ensure the
             * function stays to continue the resync operations until
             * the complete successfully.
             */
            status = DEOUTOPS;
        }

        /*
         * Continue to loop while we encounter a timeout condition or
         * we continue to get errors with a status of DEOUTOPS.
         */
    } while (rc == PI_TIMEOUT || (rc == PI_ERROR && status == DEOUTOPS));

    LogMessage(LOG_TYPE_DEBUG, "SM_MRResyncWithRetry-EXIT (0x%x, 0x%x)",
               type, ridOrNumRIDs);

    return rc;
}


/**
******************************************************************************
**
**  @brief      To provide a consistent means of sending the MRRESYNC request
**              to the BE processor. This function calls the MRP directly.
**
**  @param      type - Type of reset operation
**              ridOrNumRIDs - RAID Identifier to resync if a single RAID or
**                             the number of RIDs in a List
**              pList - Pointer to a buffer containing the either the NVA data
**                      or a List of RIDs to Resync (CCB memory address,
**                      not PCI).
**              pStatus - Pointer to a location to store the MRP status to
**                        return to the caller.
**
**  @return     INT32 - Packet return status
**
******************************************************************************
**/
static INT32 SM_MRResync(UINT8 type, UINT16 ridOrNumRIDs, void *pList, UINT8 *pStatus)
{
    MRRESYNC_REQ *ptrInPkt;
    MRRESYNC_RSP *ptrOutPkt;
    INT32       rc;

    /* Allocate memory for the request. */
    ptrInPkt = MallocWC(sizeof(*ptrInPkt));
    ptrOutPkt = MallocSharedWC(sizeof(*ptrOutPkt));

    /* Clear the status */
    *pStatus = DEOK;

    /* Fill in the request parms. */
    ptrInPkt->type = type;
    if (type == MRBLISTRAIDS)
    {
        ptrInPkt->r.numRids = ridOrNumRIDs;
        ptrInPkt->p.pRIDList = pList;
    }
    else
    {
        ptrInPkt->r.rid = ridOrNumRIDs;
        ptrInPkt->p.pNVA = pList;
    }

    rc = PI_ExecMRP(ptrInPkt, sizeof(*ptrInPkt), MRRESYNC,
                    ptrOutPkt, sizeof(*ptrOutPkt), SYNC_RAIDS_TIMEOUT / 3);

    if (rc == PI_GOOD && type == MRBSTRIPE)
    {
        /*
         * If the resync operation completed successfully, clear
         * the FE NVA records.
         */
        SM_MRReset(GetMyControllerSN(), MXNFENVA);
    }

    LogMessage(LOG_TYPE_DEBUG, "SM_MRResync-Resync %s (0x%x, 0x%x, 0x%x, 0x%x)",
               (rc == PI_GOOD ? "started" : "failed"),
               type, ridOrNumRIDs, rc, ptrOutPkt->header.status);

    /* Save the status code to return to the caller. */
    *pStatus = ptrOutPkt->header.status;

    /*
     * Free the allocated memory.
     *
     * Use DelayedFree on the input packet since it contains a PCI address.
     */
    DelayedFree(MRRESYNC, ptrInPkt);

    if (rc != PI_TIMEOUT)
    {
        Free(ptrOutPkt);
    }

    return rc;
}


/**
******************************************************************************
**
**  @brief      To provide a consistent means of sending the MRRESET request
**              to the BE processor on a specific controller. This function
**              uses the packet interface and tunnel requests to send the
**              request locally or remotely (respectively).
**
**  @param      controllerSN - controller to execute the request
**              type - Type of reset operation
**
**  @return     INT32 - Packet return status
**
******************************************************************************
**/
INT32 SM_MRReset(UINT32 controllerSN, UINT32 type)
{
    INT32       rc = PI_GOOD;
    XIO_PACKET  reqPacket;
    XIO_PACKET  rspPacket;

    dprintf(DPRINTF_DEFAULT, "SM_MRReset: ENTER (0x%x, 0x%x)\n", controllerSN, type);

    /*
     *  TODO: The old SM code used a timeout value of "SYNC_RAIDS_TIMEOUT/3"
     *        where SYNC_RAIDS_TIMEOUT == 4500000 (ms). Do we need to
     *        extend the packet interface timeout for MRRESET?
     */

    /*
     * Allocate memory for the request (header and data) and the
     * response header. The response data will be allocated in the called
     * function.
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    reqPacket.pPacket = MallocWC(sizeof(PI_DEBUG_INIT_PROC_NVRAM_REQ));
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    rspPacket.pPacket = NULL;
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    reqPacket.pHeader->commandCode = PI_DEBUG_INIT_PROC_NVRAM_CMD;
    reqPacket.pHeader->length = sizeof(PI_DEBUG_INIT_PROC_NVRAM_REQ);

    ((PI_DEBUG_INIT_PROC_NVRAM_REQ *)reqPacket.pPacket)->type = type;

    /*
     * If the request is for this controller send it to the port server
     * directly. If it is for one of the other controllers then tunnel
     * the request to that controller.
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
        dprintf(DPRINTF_DEFAULT, "SM_MRReset: Failed request (0x%x)\n", rc);
    }

    /* Free the allocated memory */
    Free(reqPacket.pHeader);
    Free(reqPacket.pPacket);
    Free(rspPacket.pHeader);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPacket.pPacket);
    }

    return rc;
}


/**
******************************************************************************
**
**  @brief      To provide a consistent means of sending the request to
**              Setup/configure the Mirror Partner on a given controller.
**              This function uses the packet interface and tunnel
**              requests to send the request locally or remotely
**              (respectively).
**
**  @param      controllerSN - controller to execute the request
**  @param      partnerSN - mirror partner serial number
**  @param      option - control options
**  @param      pMPInfo - Mirror partner information, can be NULL which
**                        will force the handler of this packet to retrieve
**                        the information.
**
**  @return     INT32 - Packet return status
**
******************************************************************************
**/
INT32 SM_MirrorPartnerControl(UINT32 controllerSN, UINT32 partnerSN,
                              UINT32 option, MP_MIRROR_PARTNER_INFO *pMPInfo)
{
    INT32                               rc = PI_GOOD;
    XIO_PACKET                          reqPacket;
    XIO_PACKET                          rspPacket;
    PI_MISC_MIRROR_PARTNER_CONTROL_REQ *pReq;

    dprintf(DPRINTF_DEFAULT, "SM_MirrorPartnerControl: ENTER (0x%x->0x%x, 0x%x)\n",
            controllerSN, partnerSN, option);

    /*
     * Allocate memory for the request (header and data) and the
     * response header. The response data will be allocated in the called
     * function.
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    reqPacket.pPacket = MallocWC(sizeof(*pReq));
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    rspPacket.pPacket = NULL;
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    reqPacket.pHeader->commandCode = PI_MISC_MIRROR_PARTNER_CONTROL_CMD;
    reqPacket.pHeader->length = sizeof(*pReq);

    /*
     * Fill in the request parms.
     */
    pReq = (PI_MISC_MIRROR_PARTNER_CONTROL_REQ *)reqPacket.pPacket;
    pReq->partnerSN = partnerSN;
    pReq->option = option;

    /*
     * If there is mirror partner information available add it to the
     * request and set the option bit saying the information is valid.
     */
    if (pMPInfo != NULL)
    {
        memcpy(&pReq->mpInfo, pMPInfo, sizeof(*pReq));
        pReq->option |= MIRROR_PARTNER_CONTROL_OPT_MPINFO_VALID;
    }

    /*
     * If the request is for this controller send it to the port server
     * directly. If it is for one of the other controllers then tunnel
     * the request to that controller.
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
        dprintf(DPRINTF_DEFAULT, "SM_MirrorPartnerControl: Failed request (0x%x)\n", rc);
    }

    /* Free the allocated memory */
    Free(reqPacket.pHeader);
    Free(reqPacket.pPacket);
    Free(rspPacket.pHeader);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPacket.pPacket);
    }

    return rc;
}


/**
******************************************************************************
**
**  @brief      To provide a consistent means of sending the request to
**              tell a controller to continue without its mirror partner.
**              This function uses the packet interface and tunnel
**              requests to send the request locally or remotely
**              (respectively).
**
**  @param      controllerSN - controller to execute the request
**
**  @return     INT32 - Packet return status
**
******************************************************************************
**/
INT32 SM_ContinueWithoutMP(UINT32 controllerSN)
{
    INT32       rc = PI_GOOD;
    XIO_PACKET  reqPacket;
    XIO_PACKET  rspPacket;

    dprintf(DPRINTF_DEFAULT, "SM_ContinueWithoutMP-ENTER (0x%x)\n", controllerSN);

    /*
     * Allocate memory for the request (header and data) and the
     * response header. The response data will be allocated in the called
     * function.
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    reqPacket.pPacket = NULL;
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    rspPacket.pPacket = NULL;
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    reqPacket.pHeader->commandCode = PI_MISC_CONTINUE_WO_MP_CMD;
    reqPacket.pHeader->length = 0;

    /*
     * If the request is for this controller send it to the port server
     * directly. If it is for one of the other controllers then tunnel
     * the request to that controller.
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
        dprintf(DPRINTF_DEFAULT, "SM_ContinueWithoutMP: Failed request (0x%x)\n", rc);
    }

    /* Free the allocated memory */
    Free(reqPacket.pHeader);
    Free(rspPacket.pHeader);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPacket.pPacket);
    }

    dprintf(DPRINTF_DEFAULT, "SM_ContinueWithoutMP-EXIT (0x%x, 0x%x)\n",
            controllerSN, rc);

    return rc;
}


/**
******************************************************************************
**
**  @brief      To provide a consistent means of sending the resume cache
**              request.
**
**  @param      UINT8 userResp - User response on how to resume cache.
**
**  @return     INT32 - Packet return status
**
******************************************************************************
**/
INT32 SM_MRResumeCache(UINT8 userResp)
{
    MRRESUMECACHE_REQ *ptrInPkt;
    MRRESUMECACHE_RSP *ptrOutPkt;
    INT32       rc;

    ptrInPkt = MallocWC(sizeof(*ptrInPkt));
    ptrOutPkt = MallocSharedWC(sizeof(*ptrOutPkt));

    ptrInPkt->userResp = userResp;

    rc = PI_ExecMRP(ptrInPkt, sizeof(*ptrInPkt), MRRESUMECACHE,
                    ptrOutPkt, sizeof(*ptrOutPkt), GetGlobalMRPTimeout());

    if (rc != PI_GOOD)
    {
        dprintf(DPRINTF_DEFAULT, "SM_MRResumeCache-Failed (rc: 0x%x)\n", rc);
    }

    /* Free the allocated memory. */
    Free(ptrInPkt);

    if (rc != PI_TIMEOUT)
    {
        Free(ptrOutPkt);
    }

    return rc;
}


/**
******************************************************************************
**
**  @brief      Rescans device on the BE.
**
**  @param      UINT32 controllerSN - Controller to receive this request.
**  @param      UINT8 scanType - Type of scan being requested.
**
**  @return     INT32 - PI_GOOD, PI_ERROR or PI_TIMEOUT
**
******************************************************************************
**/
INT32 SM_RescanDevices(UINT32 controllerSN, UINT8 scanType)
{
    INT32       rc = PI_GOOD;
    XIO_PACKET  reqPacket;
    XIO_PACKET  rspPacket;

    LogMessage(LOG_TYPE_DEBUG, "SM_RescanDevices-ENTER (sn: 0x%x, scanType: 0x%x)",
               controllerSN, scanType);

    /*
     * Allocate memory for the request (header and data) and the
     * response header. The response data will be allocated in the called
     * function.
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    reqPacket.pPacket = MallocWC(sizeof(MRRESCANDEVICE_REQ));
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    rspPacket.pPacket = NULL;
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    /* Fill in the Header */
    reqPacket.pHeader->commandCode = PI_MISC_RESCAN_DEVICE_CMD;
    reqPacket.pHeader->length = sizeof(MRRESCANDEVICE_REQ);

    /* Fill in the request packet */
    ((MRRESCANDEVICE_REQ *)(reqPacket.pPacket))->scanType = scanType;

    /*
     * If the port list request is for this controller make the
     * request to the port server directly. If it is for one
     * of the slave controllers then tunnel the request to
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
        LogMessage(LOG_TYPE_DEBUG, "SM_RescanDevices-Failed (sn: 0x%x, scanType: 0x%x, rc: 0x%x, status: 0x%x, errorCode: 0x%x)",
                   controllerSN,
                   scanType, rc, rspPacket.pHeader->status, rspPacket.pHeader->errorCode);
    }

    /* Free the allocated memory */
    Free(reqPacket.pHeader);
    Free(reqPacket.pPacket);
    Free(rspPacket.pHeader);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPacket.pPacket);
    }

    LogMessage(LOG_TYPE_DEBUG, "SM_RescanDevices-EXIT (sn: 0x%x, scanType: 0x%x, rc: 0x%x)",
               controllerSN, scanType, rc);

    return rc;
}


/*****************************************************************************
**  FUNCTION NAME: SM_IPCRescanDevices
**
**  PARAMETERS: packet
**
**  RETURNS:    Pointer to a status packet
**
**  COMMENTS:   Private interface between IPC and SM
******************************************************************************/
IPC_PACKET *SM_IPCRescanDevices(IPC_PACKET *pPacket)
{
    INT32       rc;
    IPC_PACKET *pReturnPacket;

    /* Discard the data portion of the packet */
    Free(pPacket->data);

    pReturnPacket = pPacket;
    pReturnPacket->header->commandCode = PACKET_IPC_COMMAND_STATUS;
    pReturnPacket->header->length = sizeof(IPC_COMMAND_STATUS);
    pReturnPacket->data = MallocSharedWC(sizeof(IPC_COMMAND_STATUS));

    rc = SM_RescanDevices(GetMyControllerSN(), RESCAN_EXISTING);

    /* If the flush is good return success otherwise return failure. */
    if (rc == PI_GOOD)
    {
        pReturnPacket->data->commandStatus.status = IPC_COMMAND_SUCCESSFUL;
    }
    else
    {
        dprintf(DPRINTF_DEFAULT, "SM_IPCRescanDevices failed.\n");
        pReturnPacket->data->commandStatus.status = IPC_COMMAND_FAILED;
    }

    return pReturnPacket;
}


/**
******************************************************************************
**
**  @brief      To provide a means of modifying the mirroring status of all
**              Raid 5 devices owned by this controller.
**
**  @param      option - TRUE = mirroring enabled
**                       FALSE = mirroring disabled
**
**  @return     INT32 - Packet return status
**
******************************************************************************
**/
INT32 SM_ModifyRaid5MirrorStatus(bool option)
{
    INT32       rc = PI_GOOD;
    XIO_PACKET  reqPacket;
    XIO_PACKET  rspPacket;

    dprintf(DPRINTF_DEFAULT, "SM_ModifyRaid5MirrorStatus: ENTER (0x%x)\n", option);

    /*
     * Set-up and issue a command to mark all Raid 5 devices owned by this
     * controller as mirror or not.
     */

    /*
     * Allocate memory for the request (header and data) and the
     * response header. The response data will be allocated in the called
     * function.
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    reqPacket.pPacket = MallocWC(sizeof(PI_RAID_MIRRORING_REQ));
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    rspPacket.pPacket = NULL;
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    reqPacket.pHeader->commandCode = PI_RAID_MIRRORING_CMD;
    reqPacket.pHeader->length = sizeof(PI_RAID_MIRRORING_REQ);

    if (option)
    {
        /* Turn off the NOT mirroring flag. */
        ((PI_RAID_MIRRORING_REQ *)reqPacket.pPacket)->type = RAID_MIRRORING;
        fprintf(stderr,"<SM_ModifyRaid5MirrorStatus>....Clearing NotMirror field in Raid5..\n");
    }
    else
    {
        /* Turn on the NOT mirroring flag. */
        ((PI_RAID_MIRRORING_REQ *)reqPacket.pPacket)->type = RAID_NOT_MIRRORING;
         fprintf(stderr,"<SM_ModifyRaid5MirrorStatus>....Setting NotMirror field in Raid5..\n");
    }

    /* Issue the command through the top-level command handler. */
    rc = PortServerCommandHandler(&reqPacket, &rspPacket);

    /* Free the allocated memory */
    Free(reqPacket.pHeader);
    Free(reqPacket.pPacket);
    Free(rspPacket.pHeader);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPacket.pPacket);
    }

    return rc;
}


/**
******************************************************************************
**
**  @brief      To provide the sequence of commands necessary to handle
**              a lost mirror partner.
**
**  @param      none
**
**  @return     INT32 - Packet return status
**
******************************************************************************
**/
static INT32 SM_HandleLostMirrorPartner(void)
{
    INT32       rc = PI_GOOD;

    dprintf(DPRINTF_DEFAULT, "SM_HandleLostMirrorPartner: ENTER (0x%x)\n",
            gCurrentMirrorPartnerSN);

    /*
     * Issue a command to mark all Raid 5 devices owned by this
     * controller as no longer mirroring of Raid 5 records (to mirror partner).
     * Failure of this controller would then require a full rebuild.
     * Turn on the NOT mirroring flag.
     */
    rc = SM_ModifyRaid5MirrorStatus(false);

    /* Wait until a BE MRP executes without a timeout. */
    while (rc == PI_TIMEOUT)
    {
        /* Check if there still are timeouts pending. */
        if (BEBlocked() == FALSE)
        {
            /* See if the BE will sucessfully execute a MRP now. */
            rc = ProcessorQuickTest(MRNOPBE);
        }
        else
        {
            /* Wait a second and try again. */
            TaskSleepMS(1000);
        }
    }

    if (rc == PI_GOOD)
    {
        /* Flush the FE Write cache and ignore the Mirror Partner */
        rc = SM_FlushErrorWOMP();

        /* Wait until a FE MRP executes without a timeout. */
        while (rc == PI_TIMEOUT)
        {
            /* Check if there still are timeouts pending. */
            if (FEBlocked() == FALSE)
            {
                /* See if the BE will sucessfully execute a MRP now. */
                rc = ProcessorQuickTest(MRNOPFE);
            }
            else
            {
                /* Wait a second and try again. */
                TaskSleepMS(1000);
            }
        }

        if (rc == PI_GOOD)
        {
            /* Clear the FE Resync records on the Mirror Partner */
            rc = SM_MRReset(gCurrentMirrorPartnerSN, MXNFENVA);
        }
    }

    /*
     * Only if the previous command have been successful do we want
     * to notify the ICON that the raid information has changed.
     */
    if (rc == PI_GOOD)
    {
        /* Notify the ICON that the raid information had changed. */
        SendX1ChangeEvent(X1_ASYNC_RCHANGED);
    }

    return rc;
}


/**
******************************************************************************
**
**  @brief      Task to initiate the sequence of commands necessary to handle
**              a lost mirror partner.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void SM_HandleLostMirrorPartnerTask(UNUSED TASK_PARMS *parms)
{
    /*
     * Lock a mutex around the mirror partner processing. This work is
     * initiated by an asynchronous event. Work to restore a mirror partner
     * is also generated by an async event. We want these two tasks to run
     * mutually exclusive.
     */
    (void)LockMutex(&SM_mpMutex, MUTEX_WAIT);

    /* Handle the Lost mirror partner */
    SM_HandleLostMirrorPartner();

    /* Free the mutex */
    UnlockMutex(&SM_mpMutex);
}


/**
******************************************************************************
**
**  @brief      To provide the sequence of commands necessary to handle
**              restoring a  mirror partner.
**
**  @param      none
**
**  @return     INT32 - Packet return status
**
******************************************************************************
**/
static INT32 SM_RestoreMirrorPartner(void)
{
    INT32       rc = PI_GOOD;

    dprintf(DPRINTF_DEFAULT, "SM_RestoreMirrorPartner: ENTER (0x%x)\n",
            gCurrentMirrorPartnerSN);

    /* Re-establish the mirroring with our mirror partner */
    rc = SM_MirrorPartnerControl(GetMyControllerSN(), gCurrentMirrorPartnerSN,
                                 MIRROR_PARTNER_CONTROL_OPT_STOPIO, NULL);

    /*
     * Only if the previous command have been successful do we want
     * to notify the ICON that the raid information has changed.
     */
    if (rc == PI_GOOD)
    {
        /* Notify the ICON that the raid information had changed. */
        SendX1ChangeEvent(X1_ASYNC_RCHANGED);
    }

    return rc;
}


/**
******************************************************************************
**
**  @brief      Task to initiate the sequence of commands necessary to handle
**              restoring a  mirror partner.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void SM_RestoreMirrorPartnerTask(UNUSED TASK_PARMS *parms)
{
    /*
     * Lock a mutex around the mirror partner processing. This work is
     * initiated by an asynchronous event. Work for a lost mirror partner
     * is also generated by an async event. We want these two tasks to run
     * mutually exclusive.
     */
    (void)LockMutex(&SM_mpMutex, MUTEX_WAIT);

    /* Restore the mirror partner */
    SM_RestoreMirrorPartner();

    /* Unlock the mutex */
    UnlockMutex(&SM_mpMutex);
}


/**
******************************************************************************
**
**  @brief      This method will retrieve the list of raids "owned" by the
**              specified controller.
**
**  @param      UINT32 controllerSN
**  @param      PI_SERVERS_RSP* pServerData - List of servers
**  @param      PI_TARGETS_RSP* pTargetData - List of targets
**  @param      PI_VDISKS_RSP* pVdiskData - List of virtual disks
**
**  @return     MR_LIST_RSP of owned raids (not sorted).
**
**  @attention  The caller of this method will need to free the response
**              packet after they have finished using it.
**
******************************************************************************
**/
MR_LIST_RSP *SM_RaidsOwnedByController(UINT32 controllerSN,
                                       PI_SERVERS_RSP *pServerData,
                                       PI_TARGETS_RSP *pTargetData,
                                       PI_VDISKS_RSP *pVdiskData)
{
    MRGETSINFO_RSP *pServerOut;
    MRGETVINFO_RSP *pVdiskOut;

    UINT16     *pTargetArray = NULL;    /* malloc'd locally */
    UINT16     *pTlist;

    UINT16     *pVdiskArray = NULL;     /* malloc'd locally */
    UINT16     *pVlist;

    MR_LIST_RSP *pRaidList = NULL;      /* malloc'd locally, returned to caller */

    UINT32      i;
    UINT32      j;
    INT32       k;
    INT32       count1;
    INT32       count2;

    do
    {
        /* Fetch the "Targets" info */
        pTargetArray = MallocWC((MAX_TARGETS + 1) * sizeof(UINT16));
        pTlist = pTargetArray;

        /* Compile list of targets owned by designated controller (TARGET LIST) */
        count1 = 0;
        if (pTargetData)
        {
            for (i = 0; i < pTargetData->count; i++)
            {
                if (pTargetData->targetInfo[i].owner == controllerSN)
                {
                    *pTlist++ = pTargetData->targetInfo[i].tid;
                    count1++;
                }
            }
        }
        *pTlist = 0;
        pTlist = pTargetArray;

        /* Compile list of VIDs owned by targets in TARGET LIST (VID LIST) */
        pVdiskArray = MallocWC((MAX_VIRTUAL_DISKS + 1) * sizeof(UINT16));
        pVlist = pVdiskArray;
        count2 = 0;
        if (pServerData)
        {
            while (count1--)
            {
                /* Loop on servers */
                pServerOut = (MRGETSINFO_RSP *)pServerData->servers;
                for (i = 0; i < pServerData->count; i++)
                {
                    /* If server tid matches tid found above, then grab vdisks */
                    if (pServerOut->targetId == *pTlist)
                    {
                        for (j = 0; j < pServerOut->nluns; j++)
                        {
                            /* Scan for duplicates before adding to list */
                            for (k=0; k < count2; k++)
                            {
                                if (pVdiskArray[k] == pServerOut->lunMap[j].vid)
                                {
                                    k = -1;
                                    break;
                                }
                            }
                            if (k > -1)
                            {
                                *pVlist++ = pServerOut->lunMap[j].vid;
                                count2++;
                            }
                        }
                    }

                    /* (UINT8*)pServerOut += sizeof(MRGETSINFO_RSP) + */
                    /* (pServerOut->nluns * sizeof(MRGETSINFO_RSP_LM)); */
                    pServerOut = (MRGETSINFO_RSP *)((UINT8 *)pServerOut + sizeof(MRGETSINFO_RSP) +
                                            (pServerOut->nluns * sizeof(MRGETSINFO_RSP_LM)));
                }
                pTlist++;
            }
        }
        *pVlist = 0;
        pVlist = pVdiskArray;

        /* Compile list of RIDs owned by VIDs in VID LIST (RID LIST) */
        pRaidList = MallocWC((MAX_RAIDS * sizeof(UINT16)) + sizeof(MR_LIST_RSP));
        pVlist = pVdiskArray;
        if (pVdiskData)
        {
            while (count2--)
            {
                pVdiskOut = (MRGETVINFO_RSP *)pVdiskData->vdisks;

                /*
                 * Search through the data on all the vids, lookings for
                 * vids owned by the controller.
                 */
                for (i = 0; i < pVdiskData->count; i++)
                {
                    /* If vids match, grab the raids */
                    if (pVdiskOut->vid == *pVlist)
                    {
                        for (j = 0; j < pVdiskOut->raidCnt; j++)
                        {
                            pRaidList->list[pRaidList->ndevs++] = pVdiskOut->rid[j];
                        }
                    }

                    /* (UINT8 *)pVdiskOut += sizeof(MRGETVINFO_RSP) +  */
                    /* (pVdiskOut->raidCnt * sizeof(UINT16)); */
                    pVdiskOut = (MRGETVINFO_RSP *)((UINT8 *)pVdiskOut + sizeof(MRGETVINFO_RSP) +
                                            (pVdiskOut->raidCnt * sizeof(UINT16)));
                }
                pVlist++;
            }
        }
    } while (0);

    /* Release intermediate memory. */
    Free(pTargetArray);
    Free(pVdiskArray);

    return pRaidList;
}


/**
******************************************************************************
**
**  @brief      This method will send a new device configuration list
**              to the storage side processor.
**
**  @param      none
**
**  @return     INT32 - Packet return status
**
******************************************************************************
**/
INT32 SM_PutDevConfig(void)
{
    INT32       rc = PI_GOOD;
    UINT16      cDevInfo = 0;
    SES_DEV_INFO_MAP *pDevInfo = NULL;
    MRPUTDEVCONFIG_REQ *ptrInPkt;
    MRPUTDEVCONFIG_RSP *ptrOutPkt;

    /* Load the device configuration information from flash/NVRAM. */
    LoadDeviceConfig(&cDevInfo, &pDevInfo, 1);

    /* Allocate memory for the MRP input and output packets. */
    ptrInPkt = MallocWC(sizeof(*ptrInPkt));
    ptrOutPkt = MallocSharedWC(sizeof(*ptrOutPkt));

    /* Setup the input parameters for the MRP. */
    ptrInPkt->numEntries = cDevInfo;
    ptrInPkt->pEntries = pDevInfo;

    /*
     * Send the request to Thunderbolt. This function handles timeout
     * conditions and task switches while waiting.
     */
    rc = PI_ExecMRP(ptrInPkt, sizeof(*ptrInPkt), MRPUTDEVCONFIG,
                    ptrOutPkt, sizeof(*ptrOutPkt), GetGlobalMRPTimeout());

    if (rc != PI_GOOD)
    {
        dprintf(DPRINTF_DEFAULT, "PI_MiscPutDevConfig: Failed to put dev config (rc: 0x%x).\n", rc);
    }

    /* Free the allocated memory. */
    DelayedFree(MRPUTDEVCONFIG, ptrInPkt);
    DelayedFree(MRPUTDEVCONFIG, pDevInfo);

    if (rc != PI_TIMEOUT)
    {
        Free(ptrOutPkt);
    }

    return rc;
}


/**
******************************************************************************
**
**  @brief      To provide a consistent means of sending the query
**              mirror partner change request to a given controller.
**              This function uses the packet interface and tunnel
**              requests to send the request locally or remotely
**              (respectively).
**
**  @param      UINT32 controllerSN - Controller to run the target
**                                    control request.
**  @param      UINT32 mirrorPartner - Mirror Partner for this controller.
**
**  @return     PI_MISC_QUERY_MP_CHANGE_RSP* - pointer to the QMPC
**              response information.
**
**  @attention  The caller of this method will need to free the response
**              packet after they have finished using it.
**
******************************************************************************
**/
PI_MISC_QUERY_MP_CHANGE_RSP *SM_QueryMirrorPartnerChange(UINT32 controllerSN,
                                                         UINT32 mirrorPartner)
{
    XIO_PACKET  reqPacket;
    XIO_PACKET  rspPacket;
    INT32       rc = GOOD;
    PI_MISC_QUERY_MP_CHANGE_RSP *pResponse = NULL;

    /*
     * Allocate memory for the request (header and data) and the
     * response header. Memory for the response data is allocated
     * in TunnelRequest().
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    reqPacket.pPacket = MallocWC(sizeof(PI_MISC_QUERY_MP_CHANGE_REQ));
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    rspPacket.pPacket = NULL;
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    /* Fill in the request header */
    reqPacket.pHeader->commandCode = PI_MISC_QUERY_MP_CHANGE_CMD;
    reqPacket.pHeader->length = sizeof(PI_MISC_QUERY_MP_CHANGE_REQ);

    /* Fill in the request parms. */
    ((PI_MISC_QUERY_MP_CHANGE_REQ *)(reqPacket.pPacket))->serial = mirrorPartner;

    /*
     * If the port list request is for this controller make the
     * request to the port server directly. If it is for one
     * of the slave controllers then tunnel the request to
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

    /*
     * If the request completed successfully, save the response data
     * pointer to return it and clear the pointer in the rspPacket.
     *
     * If the request returned an error check if the error signaled an
     * invalid command code. If that is the case then the other
     * controller did not support the command and we can assume that
     * the mirror partner can change.
     */
    if (rc == PI_GOOD)
    {
        pResponse = (PI_MISC_QUERY_MP_CHANGE_RSP *)rspPacket.pPacket;
        rspPacket.pPacket = NULL;
    }
    else if (rc == PI_ERROR && rspPacket.pHeader->status == PI_INVALID_CMD_CODE)
    {
        LogMessage(LOG_TYPE_DEBUG, "SM_QMPC-Controller (0x%x) does not support QMPC",
                   controllerSN);

        pResponse = MallocWC(sizeof(*pResponse));
        pResponse->mpResponse = 0;
    }

    /* Free the allocated memory */
    Free(reqPacket.pHeader);
    Free(reqPacket.pPacket);
    Free(rspPacket.pHeader);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPacket.pPacket);
    }

    return pResponse;
}


/**
******************************************************************************
**
**  @brief      To provide a consistent means of sending the resync
**              data request to a given controller.
**
**              This function uses the packet interface and tunnel
**              requests to send the request locally or remotely
**              (respectively).
**
**  @param      UINT32 controllerSN - Controller to run the target
**                                    control request.
**  @return     PI_MISC_RESYNCDATA_RSP* - pointer to the resync data
**              response information.
**
**  @attention  The caller of this method will need to free the response
**              packet after they have finished using it.
**
******************************************************************************
**/
PI_MISC_RESYNCDATA_RSP *SM_ResyncData(UINT32 controllerSN)
{
    XIO_PACKET  reqPacket;
    XIO_PACKET  rspPacket;
    INT32       rc = GOOD;
    PI_MISC_RESYNCDATA_RSP *pResponse = NULL;

    /*
     * Allocate memory for the request (header and data) and the
     * response header. Memory for the response data is allocated
     * in TunnelRequest().
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    reqPacket.pPacket = MallocWC(sizeof(PI_MISC_RESYNCDATA_REQ));
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    rspPacket.pPacket = NULL;
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    /* Fill in the request header */
    reqPacket.pHeader->commandCode = PI_MISC_RESYNCDATA_CMD;
    reqPacket.pHeader->length = sizeof(PI_MISC_RESYNCDATA_REQ);

    /* Fill in the request parms. */
    ((PI_MISC_RESYNCDATA_REQ *)(reqPacket.pPacket))->format = 0;

    /*
     * If the port list request is for this controller make the
     * request to the port server directly. If it is for one
     * of the slave controllers then tunnel the request to
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

    /*
     * If the request completed successfully, save the response data
     * pointer to return it and clear the pointer in the rspPacket.
     *
     * If the request returned an error check if the error signaled an
     * invalid command code. If that is the case then the other
     * controller did not support the command and we can assume that
     * the mirror partner can change.
     */
    if (rc == PI_GOOD)
    {
        pResponse = (PI_MISC_RESYNCDATA_RSP *)rspPacket.pPacket;
        rspPacket.pPacket = NULL;
    }

    /* Free the allocated memory */
    Free(reqPacket.pHeader);
    Free(reqPacket.pPacket);
    Free(rspPacket.pHeader);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPacket.pPacket);
    }

    return pResponse;
}


/**
******************************************************************************
**
**  @brief      Restores the BE processors NVRAM with a fixed number of
**              retries built in.
**
**  @param      UINT8 option -
**  @param      UINT8* pNVRAM - pointer to an NVRAM image to send to the BE.
**  @param      UINT8 retries - number of times to retry the operation if
**                              the request returns an error other than
**                              PI_TIMEOUT.
**
**  @return     INT32 - PI_GOOD, PI_ERROR or PI_TIMEOUT
**
******************************************************************************
**/
INT32 SM_NVRAMRestoreWithRetries(UINT8 option, UINT8 *pNVRAM, UINT8 retries)
{
    INT32       rc = PI_ERROR;
    UINT8       timeoutRetries;

//    LogMessage(LOG_TYPE_DEBUG, "SM_NVRAMRestoreWithRetries-Opt: 0x%x, pNV: 0x%x, retries: %d",
//               option, (UINT32)pNVRAM, retries);

    while (rc != PI_GOOD && retries > 0)
    {
        /* Decrement the retry count. */
        retries--;

        if (rc == PI_TIMEOUT)
        {
            /*
             * Setup the timeout retry counter, this will allow the
             * command to hopefully finish
             */
            timeoutRetries = 20;

            /* Wait until a MRP executes without a timeout. */
            while (rc == PI_TIMEOUT && timeoutRetries > 0)
            {
                /* Check if there still are timeouts pending. */
                if (BEBlocked() == FALSE)
                {
                    /* See if the BE will sucessfully execute a MRP now. */
                    rc = ProcessorQuickTest(MRNOPBE);
                }
                else
                {
                    /* Wait a second and try again. */
                    TaskSleepMS(1000);

                    /* Decrement the timeout retry counter. */
                    timeoutRetries--;
                }
            }

            /*
             * If the timeout retries have expired, continue to the next
             * retry if there are any remaining.
             */
            if (timeoutRetries == 0)
            {
                LogMessage(LOG_TYPE_DEBUG, "SM_NVRAMRestoreWithRetries-Failed TMO Retries(rc: 0x%x, retries: 0x%x)",
                           rc, retries);

                continue;
            }
        }

        /* Send the restore request. */
        rc = SM_NVRAMRestore(option, pNVRAM);

        /*
         * If the request was an error (not PI_GOOD), sleep for a little
         * and then retry the restore.
         */
        if (rc != PI_GOOD)
        {
            LogMessage(LOG_TYPE_DEBUG, "SM_NVRAMRestoreWithRetries-Failed (rc: 0x%x, retries: 0x%x)",
                       rc, retries);

            /* Sleep for 10 second(s) before attempting another restore. */
            TaskSleepMS(10000);
        }
    }

    return rc;
}


/**
******************************************************************************
**
**  @brief      Restores the BE processors NVRAM.
**
**  @param      UINT8 option -
**
**  @param      UINT8* pNVRAM - pointer to an NVRAM image to send to the BE.
**
**  @return     INT32 - PI_GOOD, PI_ERROR or PI_TIMEOUT
**
******************************************************************************
**/
INT32 SM_NVRAMRestore(UINT8 option, UINT8 *pNVRAM)
{
    INT32       rc;
    XIO_PACKET  reqPacket;
    XIO_PACKET  rspPacket;

//    LogMessage(LOG_TYPE_DEBUG, "SM_RestoreNVRAM-Opt: 0x%x, pNV: 0x%x", option,
//               (UINT32)pNVRAM);

    /*
     * Allocate memory for the request (header and data) and the
     * response header. The response data will be allocated in the called
     * function.
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    reqPacket.pPacket = MallocWC(sizeof(PI_RESTORE_PROC_NVRAM_REQ));
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    rspPacket.pPacket = NULL;
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    /* Fill in the Header */
    reqPacket.pHeader->commandCode = PI_PROC_RESTORE_NVRAM_CMD;
    reqPacket.pHeader->length = sizeof(PI_RESTORE_PROC_NVRAM_REQ);

    /* Fill in the request packet */
    ((PI_RESTORE_PROC_NVRAM_REQ *)(reqPacket.pPacket))->channel = 0;
    ((PI_RESTORE_PROC_NVRAM_REQ *)(reqPacket.pPacket))->opt = option;
    ((PI_RESTORE_PROC_NVRAM_REQ *)(reqPacket.pPacket))->lun = 0;
    ((PI_RESTORE_PROC_NVRAM_REQ *)(reqPacket.pPacket))->addr = pNVRAM;

    /* Issue the command through the packet command handler */
    rc = PacketCommandHandler(&reqPacket, &rspPacket);

    if (rc != PI_GOOD)
    {
        LogMessage(LOG_TYPE_DEBUG, "SM_RestoreNVRAM-Failed (opt: 0x%x, pNV: 0x%x, rc: 0x%x, status: 0x%x, errorCode: 0x%x)",
                   option, (UINT32)pNVRAM,
                   rc, rspPacket.pHeader->status, rspPacket.pHeader->errorCode);
    }

    /* Free the allocated memory */
    Free(reqPacket.pHeader);
    DelayedFree(MRRESTORE, reqPacket.pPacket);
    Free(rspPacket.pHeader);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPacket.pPacket);
    }

    return rc;
}


/**
******************************************************************************
**
**  @brief      Sends the request to set/clear the temporary cache disable.
**
**  @param      UINT32 controllerSN - Serial number for the controller.
**  @param      UINT32 commandCode - PI command code for the set or clear
**                                   temporarily disable cache.
**                                   - PI_MISC_SETTDISCACHE_CMD
**                                   - PI_MISC_CLRTDISCACHE_CMD
**  @param      UINT8 user - User for this temporary disable request
**  @param      UINT8 option - Option for the CLR call for a temp disable.
**
**  @return     INT32 - PI_GOOD, PI_ERROR or PI_TIMEOUT
**
**  @attention  This function assumes that both the set and clear requests
**              use the same size response packet.
**
******************************************************************************
**/
INT32 SM_TempDisableCache(UINT32 controllerSN,
                          UINT32 commandCode, UINT8 user, UINT8 option)
{
    INT32       rc = PI_GOOD;
    XIO_PACKET  reqPacket;
    XIO_PACKET  rspPacket;
    UINT32      length;

    LogMessage(LOG_TYPE_DEBUG, "SM_TempDisableCache (%s)-ENTER (sn: 0x%x, user: 0x%x, opt: 0x%x)",
               (commandCode == PI_MISC_SETTDISCACHE_CMD ? "SET" : "CLR"),
               controllerSN, user, option);

    /*
     * Allocate memory for the request (header and data) and the
     * response header. The response data will be allocated in the called
     * function.
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    rspPacket.pPacket = NULL;
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    /*
     * Depending on if this is a SET or CLR operation construct the
     * correct request packet.
     */
    if (commandCode == PI_MISC_SETTDISCACHE_CMD)
    {
        length = sizeof(PI_MISC_SETTDISCACHE_REQ);
        reqPacket.pPacket = MallocWC(length);
        ((PI_MISC_SETTDISCACHE_REQ *)(reqPacket.pPacket))->user = user;
    }
    else
    {
        length = sizeof(PI_MISC_CLRTDISCACHE_REQ);
        reqPacket.pPacket = MallocWC(length);
        ((PI_MISC_CLRTDISCACHE_REQ *)(reqPacket.pPacket))->user = user;
        ((PI_MISC_CLRTDISCACHE_REQ *)(reqPacket.pPacket))->option = option;
    }

    /* Fill in the Header */
    reqPacket.pHeader->commandCode = commandCode;
    reqPacket.pHeader->length = length;

    /*
     * If the targets request is for this controller make the
     * request to the port server directly. If it is for one
     * of the slave controllers then tunnel the request to
     * that controller.
     */
    if (controllerSN == GetMyControllerSN())
    {
        /* Issue the command through the top-level command handler. */
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

    /*
     * If the request returned an error check if the error signaled an
     * invalid command code. If that is the case then the other
     * controller did not support the command and we can assume that
     * temporary cache disable is set or cleared.
     */
    if (rc == PI_ERROR && rspPacket.pHeader->status == PI_INVALID_CMD_CODE)
    {
        LogMessage(LOG_TYPE_DEBUG, "SM_TempDisableCache-Controller (0x%x) does not support SET or CLR DISCACHE",
                   controllerSN);

        rc = PI_GOOD;
    }

    /* Free the allocated memory */
    Free(reqPacket.pHeader);
    Free(reqPacket.pPacket);
    Free(rspPacket.pHeader);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPacket.pPacket);
    }

    LogMessage(LOG_TYPE_DEBUG, "SM_TempDisableCache (%s)-EXIT (sn: 0x%x, user: 0x%x, opt: 0x%x, rc: 0x%x)",
               (commandCode == PI_MISC_SETTDISCACHE_CMD ? "SET" : "CLR"),
               controllerSN, user, option, rc);

    return rc;
}


/**
******************************************************************************
**
**  @brief      Sends the request to query the temporary cache disable.
**
**  @param      UINT32 controllerSN - Serial number for the controller.
**
**  @return     Pointer to the Query Temp Disable of cache response
**              packet if successful, NULL otherwise.
**
**  @attention  The caller of this method will need to free the response
**              packet after they have finished using it.
**
******************************************************************************
**/
PI_MISC_QTDISCACHE_RSP *SM_QueryTempDisableCache(UINT32 controllerSN)
{
    INT32       rc = PI_GOOD;
    XIO_PACKET  reqPacket;
    XIO_PACKET  rspPacket;
    PI_MISC_QTDISCACHE_RSP *pResponse = NULL;

    LogMessage(LOG_TYPE_DEBUG, "SM_QueryTempDisableCache-ENTER (sn: 0x%x)", controllerSN);

    /*
     * Allocate memory for the request (header and data) and the
     * response header. The response data will be allocated in the called
     * function.
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    reqPacket.pPacket = NULL;
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    rspPacket.pPacket = NULL;
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    /* Fill in the Header */
    reqPacket.pHeader->commandCode = PI_MISC_QTDISCACHE_CMD;
    reqPacket.pHeader->length = 0;

    /*
     * If the targets request is for this controller make the
     * request to the port server directly. If it is for one
     * of the slave controllers then tunnel the request to
     * that controller.
     */
    if (controllerSN == GetMyControllerSN())
    {
        /* Issue the command through the top-level command handler. */
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

    /*
     * If the request completed successfully, save the response data
     * pointer to return it and clear the pointer in the rspPacket.
     *
     * If the request returned an error check if the error signaled an
     * invalid command code. If that is the case then the other
     * controller did not support the command and we can assume that
     * the mirror partner can change.
     */
    if (rc == PI_GOOD)
    {
        pResponse = (PI_MISC_QTDISCACHE_RSP *)rspPacket.pPacket;
        rspPacket.pPacket = NULL;
    }
    else if (rc == PI_ERROR && rspPacket.pHeader->status == PI_INVALID_CMD_CODE)
    {
        LogMessage(LOG_TYPE_DEBUG, "SM_QueryTempDisableCache-Controller (0x%x) does not support QTDISCACHE",
                   controllerSN);

        pResponse = MallocWC(sizeof(*pResponse));
        pResponse->data.status = MQTD_DONE;
    }

    /* Free the allocated memory */
    Free(reqPacket.pHeader);
    Free(rspPacket.pHeader);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPacket.pPacket);
    }

    LogMessage(LOG_TYPE_DEBUG, "SM_QueryTempDisableCache-EXIT (sn: 0x%x, rc: 0x%x)",
               controllerSN, rc);

    return pResponse;
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
