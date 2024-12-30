/* $Id: PI_BatteryBoard.c 156532 2011-06-24 21:09:44Z m4 $*/
/**
******************************************************************************
**
**  @file       PI_BatteryBoard.c
**
**  @brief      Packet Interface and miscellaneous functions for
**              Battery Board Commands
**
**  These functions battery board requests.
**
**  Copyright (c) 2001-2009 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#include "PI_BatteryBoard.h"
#include "CmdLayers.h"
#include "cps_init.h"
#include "EL.h"
#include "ipc_sendpacket.h"
#include "PacketInterface.h"
#include "pcb.h"
#include "PI_CmdHandlers.h"
#include "PI_Utils.h"
#include "quorum_utils.h"
#include "rm.h"
#include "sm.h"
#include "XIO_Std.h"
#include "XIO_Types.h"
#include "XIO_Macros.h"

/*
******************************************************************************
** Private defines - macros
******************************************************************************
*/
#define BATTERY_BOARD_AS_STR(a)     (a == BATTERY_BOARD_FE ? "FE" : "BE")
#define BATTERY_HEALTH_AS_STR(a)    (a == BATTERY_HEALTH_GOOD ? "GOOD" : "FAIL")

/*
******************************************************************************
** Private variables
******************************************************************************
*/
static PCB *gpBatteryHealthTask = NULL;
static UINT8 gbBatteryHealthStateChanged = FALSE;
static UINT8 gBatteryHealthState = BATTERY_HEALTH_FAIL;

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
static void BatteryHealthTask(TASK_PARMS *pTaskParms);
static UINT32 BatteryHealthGetSourceMirrorPartner(void);
static INT32 BatteryHealthRemoteUpdate(UINT32 controllerSN, UINT8 state);
static INT32 BatteryHealthSet(UINT8 board, UINT8 state, MRSETBATHEALTH_RSP *pResponse);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      Retrieves the cached battery health state value.
**
**  @param      none
**
**  @return     Cached battery health state.
**
******************************************************************************
**/
UINT8 BatteryHealthState(void)
{
    return gBatteryHealthState;
}


/**
******************************************************************************
**
**  @brief      Handle the PI_BATTERY_HEALTH_SET_CMD reqeuest packet
**              sent from a client.
**
**              If the request is for the FE board the handler will spawn
**              the Battery Health task to handle the processing and update
**              the controllers source mirror partner.
**
**              If the request is for the BE board the handler will send
**              the MRSETBATHEALTH MRP request to the controller.
**
**  @param      XIO_PACKET* pReqPacket - pointer to the request packet
**  @param      XIO_PACKET* pRspPacket - pointer to the response packet
**
**  @return     INT32 - PI_GOOD, PI_ERROR or one of the other PI return codes.
**
******************************************************************************
**/
INT32 PI_BatteryHealthSet(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    PI_BATTERY_HEALTH_SET_REQ *pReq;
    PI_BATTERY_HEALTH_SET_RSP *pRsp;
    INT32       rc = PI_GOOD;
    MRSETBATHEALTH_RSP mrpRsp;

    pReq = (PI_BATTERY_HEALTH_SET_REQ *)pReqPacket->pPacket;

    if (pReq->board == BATTERY_BOARD_FE)
    {
        BatteryHealthSet(BATTERY_BOARD_FE, pReq->state, &mrpRsp);
        BatteryHealthTaskStart(pReq->state);

        memset(&mrpRsp, 0x00, sizeof(mrpRsp));
        mrpRsp.header.status = DEOK;
        mrpRsp.header.len = sizeof(mrpRsp);
    }
    else
    {
        rc = BatteryHealthSet(pReq->board, pReq->state, &mrpRsp);
    }

    pRsp = MallocWC(sizeof(*pRsp));
    pRsp->header.status = mrpRsp.header.status;
    pRsp->header.len = mrpRsp.header.len;

    /*
     * Attach the MRP return data packet to the main response packet.
     * Fill in the header length and status fields.
     */
    pRspPacket->pPacket = (UINT8 *)pRsp;
    pRspPacket->pHeader->length = sizeof(*pRsp);
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = rc;

    return rc;
}


/**
******************************************************************************
**
**  @brief      Function to start/restart the battery health processing.
**
**  @param      UINT8 state - State of the battery board for this controller.
**
**  @return     none
**
******************************************************************************
**/
void BatteryHealthTaskStart(UINT8 state)
{
    /*
     * Save the last known battery health state.
     */
    gBatteryHealthState = state;

    /*
     * Set the changed flag so the task knows to keep running.
     */
    gbBatteryHealthStateChanged = TRUE;

    LogMessage(LOG_TYPE_DEBUG, "BatteryHealthTaskStart-State: %s, Changed: %s, Task: 0x%x",
               BATTERY_HEALTH_AS_STR(gBatteryHealthState),
               (gbBatteryHealthStateChanged ? "TRUE" : "FALSE"),
               (UINT32)gpBatteryHealthTask);

    /*
     * If the battery health task has not yet been create do it now.
     */
    if (gpBatteryHealthTask == NULL)
    {
        gpBatteryHealthTask = TaskCreate(BatteryHealthTask, NULL);
    }
}


/**
******************************************************************************
**
**  @brief      This function is forked as a task to handle the processing
**              required for battery health changes.
**
**  @param      UINT32 dummy - placeholder required for forked task.
**
**  @attention  FORKED
**
******************************************************************************
**/
static void BatteryHealthTask(UNUSED TASK_PARMS *pTaskParms)
{
    INT32       rc = PI_GOOD;
    UINT32      controllerSN;

    LogMessage(LOG_TYPE_DEBUG, "BatteryHealthTask-ENTER (%s)",
               BATTERY_HEALTH_AS_STR(gBatteryHealthState));

    while (gbBatteryHealthStateChanged)
    {
        /*
         * Hold off the battery health changes until power-up complete.
         */
        if (!PowerUpComplete())
        {
            /*
             * Delay 5 second(s).
             */
            TaskSleepMS(5000);

            /*
             * Go back to the start of the while loop to process more
             * requests, eventually the controllers will reach power-up
             * complete.
             */
            continue;
        }

        /*
         * If an election or reallocation is in progress we will hold
         * off processing the battery health requests.
         */
        if (EL_TestInProgress() == TRUE ||
            (RMGetState() != RMRUNNING && RMGetState() != RMDOWN))
        {
            /*
             * Delay .5 second(s).
             */
            TaskSleepMS(500);

            /*
             * Go back to the start of the while loop to process more
             * requests, eventually the election or reallocation will
             * be complete and the requests can be processed.
             */
            continue;
        }

        /*
         * If this controller is either inactivated or FW update
         * inactivated it should not be processing battery events.
         */
        if (GetControllerFailureState() == FD_STATE_INACTIVATED ||
            GetControllerFailureState() == FD_STATE_FIRMWARE_UPDATE_INACTIVE)
        {
            /*
             * Delay 5 second(s).
             */
            TaskSleepMS(5000);

            /*
             * Go back to the start of the while loop, eventually the
             * controller will be power-cycled and/or unfailed and then
             * events can be processed.
             */
            continue;
        }

        /*
         * Find the controller who is mirroring to this controller (otherwise
         * known as the source mirror partner).
         */
        controllerSN = BatteryHealthGetSourceMirrorPartner();

        /*
         * If we could not identify the source mirror partner we cannot
         * do the remote update.
         */
        if (controllerSN == 0)
        {
            /*
             * Delay 1 second(s).
             */
            TaskSleepMS(1000);

            /*
             * Go back to the start of the while loop, eventually the
             * controller will return a valid source mirror partner.
             */
            continue;
        }

        /*
         * Clear the changed flag since we are updating the battery
         * information to this controller and its source mirror partner.
         */
        gbBatteryHealthStateChanged = FALSE;

        /*
         * Tell the source mirror partner the state of this controllers battery.
         */
        rc = BatteryHealthRemoteUpdate(controllerSN, gBatteryHealthState);
    }

    /*
     * The task is ending so clear out the PCB variable.
     */
    gpBatteryHealthTask = NULL;

    LogMessage(LOG_TYPE_DEBUG, "BatteryHealthTask-EXIT (%s)",
               BATTERY_HEALTH_AS_STR(gBatteryHealthState));
}


/**
******************************************************************************
**
**  @brief      Search the mirror partner list and find this controllers
**              source mirror partner, who is mirroring to this controller.
**
**  @param      none
**
**  @return     UINT32 - Controller who is mirroring to this controller or
**                       this controllers serial number if the mirror
**                       partner list could not be retrieved or if the
**                       no one is mirroring to this controller.
**
******************************************************************************
**/
static UINT32 BatteryHealthGetSourceMirrorPartner(void)
{
    UINT16      count;
    PI_VCG_GET_MP_LIST_RSP *pMPList = NULL;
    UINT32      controllerSN = 0;

    /*
     * If the BE is currently blocked we won't be able to send the
     * mirror partner list request so don't.
     */
    if (!BEBlocked())
    {
        /*
         * Get the mirror partner list
         */
        pMPList = SM_GetMirrorPartnerList();

        if (pMPList != NULL)
        {
            /*
             * Examine each entry in the mirror partner list.
             */
            for (count = 0; count < pMPList->count; ++count)
            {
                /*
                 * If the controller serial number is still zero or
                 * it is currently set to ourself check if it is the
                 * destination.  If it is the destination, update the
                 * the controller serial number to return.
                 *
                 * NOTE: This code will continue to look for a destination
                 *       controller even if the we found that we were
                 *       mirroring to ourself just in case the list is in
                 *       flux.
                 */
                if ((controllerSN == 0 || GetMyControllerSN() == controllerSN) &&
                    GetMyControllerSN() == pMPList->list[count].dest)
                {
                    controllerSN = pMPList->list[count].source;
                }
            }

            Free(pMPList);
        }
    }

    return controllerSN;
}


/**
******************************************************************************
**
**  @brief      Sends the battery health for the BE board to the
**              specific controller.
**
**  @param      UINT32 controllerSN - Which controller
**
**  @param      UINT8 state - State of the board
**
**  @return     INT32
**
******************************************************************************
**/
static INT32 BatteryHealthRemoteUpdate(UINT32 controllerSN, UINT8 state)
{
    XIO_PACKET  reqPacket;
    XIO_PACKET  rspPacket;
    INT32       rc = PI_ERROR;

    LogMessage(LOG_TYPE_DEBUG, "BatteryHealthRemoteUpdate - ENTER (0x%x, %s)",
               controllerSN, BATTERY_HEALTH_AS_STR(state));

    /*
     * Allocate memory for the request and response headers and the
     * request data.  Response data space is allocated at a lower level.
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    reqPacket.pPacket = MallocWC(sizeof(PI_BATTERY_HEALTH_SET_REQ));
    rspPacket.pPacket = NULL;

    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    /*
     * Fill in the request structure and issue the request through
     * the command layers.
     */
    reqPacket.pHeader->commandCode = PI_BATTERY_HEALTH_SET_CMD;
    reqPacket.pHeader->length = sizeof(PI_BATTERY_HEALTH_SET_REQ);

    ((PI_BATTERY_HEALTH_SET_REQ *)(reqPacket.pPacket))->board = BATTERY_BOARD_BE;
    ((PI_BATTERY_HEALTH_SET_REQ *)(reqPacket.pPacket))->state = state;

    /*
     * If the request is for this controller make the request to the
     * port server directly.  If it is for one of the slave controllers
     * then tunnel the request to that controller.
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
     * Free the allocated memory
     */
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
**  @brief      Submits the MRSETBATHEALTH request MRP.
**
**  @param      UINT8 board - Which battery board
**
**  @param      UINT8 state - State of the board
**
**  @return     INT32
**
**  @attention  Creates DailyGroupValidationTask
**
******************************************************************************
**/
static INT32 BatteryHealthSet(UINT8 board, UINT8 state, MRSETBATHEALTH_RSP *pResponse)
{
    INT32       rc;
    MRSETBATHEALTH_REQ *pInPkt;
    MRSETBATHEALTH_RSP *pOutPkt;

    LogMessage(LOG_TYPE_DEBUG, "BatteryHealthSet - ENTER (%s, %s)",
               BATTERY_BOARD_AS_STR(board), BATTERY_HEALTH_AS_STR(state));

    /*
     * Allocate memory for the MRP input and output packets.
     */
    pInPkt = MallocWC(sizeof(*pInPkt));
    pOutPkt = MallocSharedWC(sizeof(*pOutPkt));

    /*
     * Load the battery health information and call the MRP
     */
    pInPkt->board = board;
    pInPkt->state = state;

    /*
     * Send the request to Thunderbolt.  This function handles timeout
     * conditions and task switches while waiting.
     */
    rc = PI_ExecMRP(pInPkt, sizeof(*pInPkt), MRSETBATHEALTH,
                    pOutPkt, sizeof(*pOutPkt), MRP_STD_TIMEOUT);

    if (rc == PI_GOOD)
    {
        memcpy(pResponse, pOutPkt, sizeof(*pOutPkt));
    }
    else
    {
        memset(pResponse, 0x00, sizeof(*pOutPkt));
    }

    /*
     * Free the allocated memory
     */
    Free(pInPkt);

    if (rc != PI_TIMEOUT)
    {
        Free(pOutPkt);
    }

    return (rc);
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
