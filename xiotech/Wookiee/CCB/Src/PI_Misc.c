/* $Id: PI_Misc.c 160950 2013-04-22 21:10:28Z marshall_midden $*/
/**
******************************************************************************
**
**  @file   PI_Misc.c
**
**  @brief  Packet Interface for Miscellaneous Commands
**
**  These functions handle generic requests that apply to
**  more than one object type, e.g. Get Count, Get List.
**
** Copyright (c) 2001-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/

#include <dirent.h>

#include "CacheManager.h"
#include "CacheSize.h"
#include "CacheLoop.h"
#include "CmdLayers.h"
#include "convert.h"
#include "cps_init.h"
#include "DEF_Workset.h"
#include "debug_files.h"
#include "error_handler.h"
#include "LOG_Defs.h"
#include "EL.h"
#include "error_handler.h"
#include "fm.h"
#include "HWM.h"
#include "ipc_cmd_dispatcher.h"
#include "ipc_session_manager.h"
#include "misc.h"
#include "mode.h"
#include "MR_Defs.h"
#include "nvram.h"
#include "PacketInterface.h"
#include "PktCmdHdl.h"
#include "PI_BatteryBoard.h"
#include "PI_CmdHandlers.h"
#include "PI_Clients.h"
#include "PI_Misc.h"
#include "PI_PDisk.h"
#include "PI_Utils.h"
#include "PortServer.h"
#include "PortServerUtils.h"
#include "proc_hw.h"
#include "quorum.h"
#include "quorum_utils.h"
#include "realtime.h"
#include "serial_num.h"
#include "ses.h"
#include "sm.h"
#include "X1_AsyncEventHandler.h"
#include "X1_Packets.h"
#include "X1_Structs.h"
#include "XIO_Std.h"
#include "xssa_structure.h"
#include "L_Misc.h"
#include "SerCon.h"
#include "SerBuff.h"
#include "SerConNetwork.h"
#include "PI_ClientPersistent.h"
#include "Snapshot.h"

#include <byteswap.h>

/*****************************************************************************
** Private defines
*****************************************************************************/
#define XioWebService_Dir  "/var/xiotech/XioWebService"

#define TMO_BE_DEVICE_PATHS     30000   /* 30 second timeout */

/*****************************************************************************
** Public routines - NOT externed in any header file
*****************************************************************************/
extern void Clean_Controller(int, UINT8);
extern void Lock_Shutdown_FE(void);
extern INT32 MfgCtrlClean_WriteSame(UINT64 wwn, UINT16 lun);

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static void MfgCtrlCleanTask(TASK_PARMS *parms);
static void MfgCtrlClean_MMClear(void);
static void MfgCtrlClean_LogClear(void);
static void MfgCtrlClean_InitCCBNVRAM(UINT8 option);
static void MfgCtrlClean_ResetPROCNVRAM(void);
static void MfgCtrlClean_ClearPhysicalDisks(void);

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
** Function:    PI_GetObjectCount
**
** Description: Get the count of objects
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_GetObjectCount(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    MR_LIST_RSP *ptrList = NULL;
    PI_COUNT_RSP *pData = NULL;
    UINT32      rc = PI_GOOD;
    UINT16      objectType = 0;

    /*
     * Use the request command code to determine the object type.
     */
    switch (pReqPacket->pHeader->commandCode)
    {
        case PI_PDISK_COUNT_CMD:
            objectType = MRGETPLIST;
            break;

        case PI_RAID_COUNT_CMD:
            objectType = MRGETRLIST;
            break;

        case PI_VDISK_COUNT_CMD:
            objectType = MRGETVLIST;
            break;

        case PI_SERVER_COUNT_CMD:
            objectType = MRGETSLIST;
            break;

        case PI_TARGET_COUNT_CMD:
            objectType = MRGETTLIST;
            break;

        case PI_DISK_BAY_COUNT_CMD:
            objectType = MRGETELIST;
            break;

        case PI_MISC_COUNT_CMD:
            objectType = MRGETMLIST;
            break;

        default:
            rc = PI_INVALID_CMD_CODE;
            break;
    }

    /*
     * If the command code was valid get the count of objects.
     */
    if (rc == PI_GOOD)
    {
        ptrList = PI_GetList(0, (objectType | GET_NUMBER_ONLY));

        /*
         * If PI_GetList() returns a valid pointer get the number of objects.
         * Otherwise return an error.
         */
        if (ptrList != NULL)
        {
            /*
             * Allocate a response data packet, attach it to the main response
             * packet and fill in the data. Also fill in the header length
             * and status fields.
             */
            pData = MallocWC(sizeof(*pData));

            pRspPacket->pPacket = (UINT8 *)pData;
            pData->count = ptrList->ndevs;
            pRspPacket->pHeader->length = sizeof(*pData);
            pRspPacket->pHeader->status = PI_GOOD;
            pRspPacket->pHeader->errorCode = ptrList->header.status;

            Free(ptrList);
        }
        else
        {
            /*
             * Indicate an error condition and no return data in the header.
             */
            pRspPacket->pHeader->length = 0;
            pRspPacket->pHeader->status = PI_ERROR;
            rc = PI_ERROR;
        }
    }

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PI_GetObjectList
**
** Description: Get the list of objects
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_GetObjectList(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    MR_LIST_RSP *ptrList;
    PI_LIST_RSP *pData;
    UINT32      rc = PI_GOOD;
    INT32       listSize = 0;
    INT32       numItems = 0;
    UINT16      objectType = 0;
    UINT16      inputParm = 0;

    /*
     * Use the request command code to determine the object type.
     * For most list requests the inputParm indicates the starting ID
     * for the list.  We always start at ID=0 and return the whole list.
     * For the port list requests inputParm indicates the type of list
     * requested (see MR_Defs.h)
     */
    switch (pReqPacket->pHeader->commandCode)
    {
        case PI_PDISK_LIST_CMD:
            objectType = MRGETPLIST;
            break;

        case PI_RAID_LIST_CMD:
            objectType = MRGETRLIST;
            break;

        case PI_VDISK_LIST_CMD:
            objectType = MRGETVLIST;
            break;

        case PI_SERVER_LIST_CMD:
            objectType = MRGETSLIST;
            break;

        case PI_TARGET_LIST_CMD:
            objectType = MRGETTLIST;
            break;

        case PI_DISK_BAY_LIST_CMD:
            objectType = MRGETELIST;
            break;

        case PI_PROC_BE_PORT_LIST_CMD:
            objectType = MRBEGETPORTLIST;
            inputParm = ((PI_PORT_LIST_REQ *)(pReqPacket->pPacket))->type;
            break;

        case PI_PROC_FE_PORT_LIST_CMD:
            objectType = MRFEGETPORTLIST;
            inputParm = ((PI_PORT_LIST_REQ *)(pReqPacket->pPacket))->type;
            break;

        case PI_MISC_LIST_CMD:
            objectType = MRGETMLIST;
            break;

        default:
            rc = PI_INVALID_CMD_CODE;
            break;
    }

    /*
     * If the command code was valid get the list of objects.
     */
    if (rc == PI_GOOD)
    {
        /*
         * Get the list of objects.  Always start at the beginning and return
         * the entire list.
         */
        ptrList = PI_GetList(inputParm, (objectType | GET_LIST));

        /*
         * If PI_GetList() returns a valid pointer get the number of PDisks.
         * Otherwise return an error.
         */
        if (ptrList != NULL)
        {
            /*
             * Calculate the list size based on the number of elements in
             * the list.  Always allocate space fo a least 1 item, even
             * if the list is empty.
             */
            numItems = ptrList->ndevs;
            if (numItems == 0)
            {
                numItems = 1;
            }

            /*
             * Allocate a response data packet, attach it to the main response
             * packet and fill in the data. Also fill in the header length
             * and status fields.
             */
            listSize = sizeof(*pData) + ((numItems - 1) * sizeof(UINT16));

            pData = MallocWC(listSize);

            pRspPacket->pPacket = (UINT8 *)pData;
            pData->count = ptrList->ndevs;
            memcpy(pData->list, ptrList->list, (listSize - sizeof(UINT16)));
            pRspPacket->pHeader->length = listSize;
            pRspPacket->pHeader->status = PI_GOOD;
            pRspPacket->pHeader->errorCode = ptrList->header.status;

            Free(ptrList);
        }
        else
        {
            /*
             * Indicate an error condition and no return data in the header.
             */
            pRspPacket->pHeader->length = 0;
            pRspPacket->pHeader->status = PI_ERROR;
            rc = PI_ERROR;
        }
    }

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PI_ProcBeDevicePathList
**
** Description: Get the list of Device Paths
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_ProcBeDevicePathList(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    MRGETBEDEVPATHS_RSP *ptrListOut = NULL;
    UINT32      listOutSize;
    UINT16      numDevs;
    UINT16      entrySize;
    INT32       rc = PI_GOOD;

    /*
     * Allocate enough memory to handle the largest possible list.
     * We have lots of memory and would rather not have to make the
     * request more than once.
     */
    numDevs = MAX_PHYSICAL_DISKS;
    entrySize = sizeof(MRGETBEDEVPATHS_RSP_ARRAY);

    do
    {
        listOutSize = sizeof(*ptrListOut) + (numDevs * entrySize);

        /*
         * If an output list was previously allocated, free it before
         * allocating a new one.
         */
        Free(ptrListOut);

        ptrListOut = MallocSharedWC(listOutSize);

        /*
         * Send the request to Thunderbolt.  This function handles timeout
         * conditions and task switches while waiting.
         */
        rc = PI_ExecMRP(pReqPacket->pPacket, pReqPacket->pHeader->length,
                        MRGETBEDEVPATHS, ptrListOut, listOutSize, TMO_BE_DEVICE_PATHS);

        /*
         * Save the number of devices in case we need to make the
         * request again.  Also grab the size of each entry from the
         * response packet.
         */
        numDevs = ptrListOut->ndevs;
        entrySize = ptrListOut->size;

    } while (ptrListOut != NULL && rc == PI_ERROR &&
             (ptrListOut->header.status == DETOOMUCHDATA));

    if (rc == PI_GOOD)
    {
        /*
         * Recalculate the size of the response data - we need the amount
         * to actually return since the original allocation was for the
         * max amount.
         */
        listOutSize = sizeof(*ptrListOut) + (numDevs * entrySize);

        pRspPacket->pHeader->length = listOutSize;
        pRspPacket->pHeader->status = PI_GOOD;
        pRspPacket->pHeader->errorCode = ptrListOut->header.status;
        pRspPacket->pPacket = (UINT8 *)ptrListOut;
    }
    else
    {
        /*
         * Indicate an error condition and no return data in the header.
         */
        pRspPacket->pHeader->length = 0;
        pRspPacket->pHeader->status = rc;

        /*
         * If we return NULL the caller can't free the memory so do it here.
         * Only free the memory if the request did NOT timeout.  On a timeout
         * the memory must remain available in case the request eventually
         * completes.
         */
        if (rc != PI_TIMEOUT)
        {
            Free(ptrListOut);
        }
    }

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PI_Connect
**
** Description: Establish a connection between the client and the CCB
**              packet interface.
**              NOTE: This is a quick implementation for test only.
**              This function will move to a different module.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_Connect(UNUSED XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    /*
     * Since we are handling the packet from somewhere externally
     * we want to make sure that our last access time is updated.
     */
    RefreshLastAccess();

    /*
     * Attach the MRP return data packet to the main response packet.
     * Fill in the header length and status fields.
     */
    pRspPacket->pPacket = NULL; /* No data to return */
    pRspPacket->pHeader->length = 0;
    pRspPacket->pHeader->status = PI_GOOD;
    pRspPacket->pHeader->errorCode = PI_GOOD;

    /*
     * New field starting with H/N.  This is only returned on a connect.  By
     * default, a '0' is returned on B/F, wookiee returns '1'.  So we can tell
     * from the CCBE / CCBCL what kind of controller we are connected to.
     */

    pRspPacket->pHeader->controllerType = GetControllerType();

    return (PI_GOOD);
}

/*----------------------------------------------------------------------------
** Function:    PI_Ping
**
** Description: Handle the PING packet sent from a client.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_Ping(UNUSED XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    /*
     * Since we are handling the packet from somewhere externally
     * we want to make sure that our last access time is updated.
     */
    RefreshLastAccess();

    /*
     * Attach the MRP return data packet to the main response packet.
     * Fill in the header length and status fields.
     */
    pRspPacket->pPacket = NULL; /* No data to return */
    pRspPacket->pHeader->length = 0;
    pRspPacket->pHeader->status = PI_GOOD;
    pRspPacket->pHeader->errorCode = PI_GOOD;

    return (PI_GOOD);
}

/*----------------------------------------------------------------------------
** Function:    PI_Reset
**
** Description: Handle the RESET packet sent from a client.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_Reset(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    TASK_PARMS  parms;

    if (((PI_RESET_REQ *)pReqPacket->pPacket)->delayed > 0)
    {
        parms.p1 = (UINT32)((PI_RESET_REQ *)pReqPacket->pPacket)->which;
        TaskCreate(ProcessResetTask, &parms);
    }
    else
    {
        ProcessReset(((PI_RESET_REQ *)pReqPacket->pPacket)->which);
    }

    /*
     * Attach the MRP return data packet to the main response packet.
     * Fill in the header length and status fields.
     */
    pRspPacket->pPacket = NULL; /* No data to return */
    pRspPacket->pHeader->length = 0;
    pRspPacket->pHeader->status = PI_GOOD;
    pRspPacket->pHeader->errorCode = PI_GOOD;

    return (PI_GOOD);
}

/*----------------------------------------------------------------------------
** Function:    PI_PowerUpState
**
** Description: Handle the POWER-UP STATE packet sent from a client.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_PowerUpState(UNUSED XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    PI_POWER_UP_STATE_RSP *pResponse = NULL;

    pResponse = MallocWC(sizeof(*pResponse));

    pResponse->state = GetPowerUpState();
    pResponse->astatus = GetPowerUpAStatus();

    /*
     * Attach the MRP return data packet to the main response packet.
     * Fill in the header length and status fields.
     */
    pRspPacket->pPacket = (UINT8 *)pResponse;
    pRspPacket->pHeader->length = sizeof(*pResponse);
    pRspPacket->pHeader->status = PI_GOOD;
    pRspPacket->pHeader->errorCode = PI_GOOD;

    return pRspPacket->pHeader->status;
}

/*----------------------------------------------------------------------------
** Function:    PI_PowerUpResponse
**
** Description: Handle the POWER-UP RESPONSE packet sent from a client.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_PowerUpResponse(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    PI_POWER_UP_RESPONSE_REQ *pRequest = NULL;
    TASK_PARMS  parms;

    ccb_assert(pReqPacket != NULL, pReqPacket);
    ccb_assert(pReqPacket->pPacket != NULL, pReqPacket->pPacket);
    ccb_assert(pRspPacket != NULL, pRspPacket);
    ccb_assert(pRspPacket->pHeader != NULL, pRspPacket->pHeader);

    pRequest = (PI_POWER_UP_RESPONSE_REQ *)pReqPacket->pPacket;

    switch (pRequest->state)
    {
        case POWER_UP_WAIT_FWV_INCOMPATIBLE:
        case POWER_UP_WAIT_PROC_COMM:
            ccb_assert(pRequest->response == POWER_UP_RESPONSE_RESET, pRequest->response);

            parms.p1 = (UINT32)PROCESS_ALL;
            TaskCreate(ProcessResetTask, &parms);
            break;

        case POWER_UP_WAIT_DRIVES:
            if (pRequest->response == POWER_UP_RESPONSE_RESET)
            {
                parms.p1 = (UINT32)PROCESS_ALL;
                TaskCreate(ProcessResetTask, &parms);
            }
            else if (pRequest->response == POWER_UP_RESPONSE_CONTINUE)
            {
                SetPowerUpResponse(pRequest->state, pRequest->response);
            }
            break;

        case POWER_UP_WAIT_DISK_BAY:
            /*
             * Re-run SES discovery to get an updated list of
             * SES devices.
             */
#if defined(MODEL_7000) || defined(MODEL_4700)
            DiscoverSES(NULL);
#else  /* MODEL_7000 || MODEL_4700 */
            DiscoverSES();
#endif /* MODEL_7000 || MODEL_4700 */

            /*
             * FALL THROUGH TO SET THE POWER-UP RESPONSE
             */

        case POWER_UP_WAIT_DISASTER:
        case POWER_UP_WAIT_CONTROLLERS:
        case POWER_UP_WAIT_CORRUPT_BE_NVRAM:
            SetPowerUpResponse(pRequest->state, pRequest->response);
            break;

        case POWER_UP_WAIT_CACHE_ERROR:
            /*
             * Tell the PROC what do do with this error
             */
            if (pRequest->response == POWER_UP_RESPONSE_WC_SAVE)
            {
                /*
                 * We have completed the cache initialization but we still want
                 * to make sure the power-up state is set correctly.  This setting
                 * is used to make sure we remove the WAIT_CACHE_ERROR state if
                 * the cache initialiazed without user input (don't know if that
                 * is possible but code for it).
                 */
                SetPowerUpState(POWER_UP_PROCESS_CACHE_INIT);
                SetPowerUpAStatus(POWER_UP_ASTATUS_UNKNOWN);

                SM_MRResumeCache(TRUE);

                /*
                 * Send an X1 event so the ICON System Status screen will get
                 * refreshed.
                 */
                LogMessage(LOG_TYPE_DEBUG, "PI_PowerUpResponse Save: Sending-"
                           "VCG_EVENT_POWERUP");
            }
            else if (pRequest->response == POWER_UP_RESPONSE_WC_DISCARD)
            {
                /*
                 * We have completed the cache initialization but we still want
                 * to make sure the power-up state is set correctly.  This setting
                 * is used to make sure we remove the WAIT_CACHE_ERROR state if
                 * the cache initialiazed without user input (don't know if that
                 * is possible but code for it).
                 */
                SetPowerUpState(POWER_UP_PROCESS_CACHE_INIT);
                SetPowerUpAStatus(POWER_UP_ASTATUS_UNKNOWN);

                SM_MRResumeCache(FALSE);

                /*
                 * Send an X1 event so the ICON System Status screen will get
                 * refreshed.
                 */
                LogMessage(LOG_TYPE_DEBUG, "PI_PowerUpResponse Discard: Sending-"
                           "VCG_EVENT_POWERUP");
            }
            else
            {
                LogMessage(LOG_TYPE_DEBUG, "Powerup Response for CACHE ERROR not yet implemented (0x%x)",
                           pRequest->response);
            }
            break;

        default:
            rc = PI_ERROR;
            break;
    }

    /*
     * Attach the MRP return data packet to the main response packet.
     * Fill in the header length and status fields.
     */
    pRspPacket->pPacket = NULL; /* No data to return */
    pRspPacket->pHeader->length = 0;
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = 0;

    return rc;
}


/**
******************************************************************************
**
**  @brief  Only disable the X1 server (no way to turn it on)
**
**  @param  pReqPacket - pointer to the request packet
**  @param  pRspPacket - pointer to the response packet
**
**  @return PI_GOOD or PI_ERROR
**
******************************************************************************
**/
INT32 PI_EnableX1Server(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    /* Fill in the header length field */

    pRspPacket->pPacket = NULL;     /* No data to return */
    pRspPacket->pHeader->length = 0;

    if (((PI_ENABLE_X1_PORT_REQ *)pReqPacket->pPacket)->enable)
    {
        /* Fill in the header status fields */

        pRspPacket->pHeader->status = PI_ERROR;
        pRspPacket->pHeader->errorCode = PI_ERROR;

        return PI_ERROR;
    }

    /* Fill in the header status fields */

    pRspPacket->pHeader->status = PI_GOOD;
    pRspPacket->pHeader->errorCode = PI_GOOD;

    return PI_GOOD;
}


/*----------------------------------------------------------------------------
** Function:    PI_X1CompatibilityIndex
**
** Description: Retrieve the X1 Compatibility Index from this controller.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_X1CompatibilityIndex(UNUSED XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    PI_X1_COMPATIBILITY_INDEX_RSP *pResponse = NULL;

    pResponse = MallocWC(sizeof(*pResponse));

    pResponse->state = X1_COMPATIBILITY;

    /*
     * Attach the MRP return data packet to the main response packet.
     * Fill in the header length and status fields.
     */
    pRspPacket->pPacket = (UINT8 *)pResponse;
    pRspPacket->pHeader->length = sizeof(*pResponse);
    pRspPacket->pHeader->status = PI_GOOD;
    pRspPacket->pHeader->errorCode = PI_GOOD;

    return pRspPacket->pHeader->status;
}


/*----------------------------------------------------------------------------
** Function:    PI_MRPPassThrough
**
** Description: Generic function that handles MRP's that are passed
**              striaght through.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_MRPPassThrough(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket,
                        UINT16 mrpCmd, UINT32 rspDataSz, UINT32 timeout)
{
    MR_GENERIC_RSP *ptrOutPkt;
    INT32       rc;

    /* Allocate memory for the MRP return data. */
    ptrOutPkt = MallocSharedWC(rspDataSz);

    /*
     * Send the request to Thunderbolt.  This function handles timeout
     * conditions and task switches while waiting.
     */
    rc = PI_ExecMRP(pReqPacket->pPacket, pReqPacket->pHeader->length,
                    mrpCmd, ptrOutPkt, rspDataSz, timeout);

    /* For a few PI commands, do a journalling snapshot -- to allow recovery after vdisk delete. */
    if (rc == PI_GOOD)
    {
        switch (pReqPacket->pHeader->commandCode)
        {
            case PI_VDISK_CREATE_CMD:
            case PI_VDISK_EXPAND_CMD:
                DelayedSnapshot(SNAPSHOT_DEFAULT_DELAY_TIME);

            default:
                 break;
        }
    }

    /*
     * Attach the MRP return data packet to the main response packet.
     * Fill in the header length and status fields.
     */
    pRspPacket->pPacket = (UINT8 *)ptrOutPkt;
    if (mrpCmd == MRGETSESSIONS || mrpCmd == MRGETCHAP)
    {
        pRspPacket->pHeader->length = ptrOutPkt->header.len;
    }
    else
    {
        pRspPacket->pHeader->length = rspDataSz;
    }
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = ptrOutPkt->header.status;

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PI_FailureStateSet
**
** Description: Handle the PI_MISC_FAILURE_STATE_SET packet sent from a client.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_FailureStateSet(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    PI_MISC_FAILURE_STATE_SET_REQ *pRequest = NULL;
    QM_FAILURE_DATA *qmFailureData;

    dprintf(DPRINTF_DEFAULT, "PI_FailureStateSet - ENTER\n");

    ccb_assert(pReqPacket != NULL, pReqPacket);
    ccb_assert(pReqPacket->pPacket != NULL, pReqPacket->pPacket);
    ccb_assert(pRspPacket != NULL, pRspPacket);
    ccb_assert(pRspPacket->pHeader != NULL, pRspPacket->pHeader);

    pRequest = (PI_MISC_FAILURE_STATE_SET_REQ *)pReqPacket->pPacket;

    if (Qm_GetOwnedDriveCount() > 0)
    {
        /*
         * Clear out the failure data structure.  We want to make
         * sure we are seeing only the current controllers failure
         * data after we read.
         */
        qmFailureData = MallocSharedWC(sizeof(*qmFailureData));
        memset(qmFailureData, 0x00, sizeof(*qmFailureData));
        qmFailureData->state = pRequest->failureState;

        rc = WriteFailureData(pRequest->serialNumber, qmFailureData);
        Free(qmFailureData);
    }
    else
    {
        rc = PI_ERROR;
    }

    /*
     * Attach the return data packet to the main response packet.
     * Fill in the header length and status fields.
     */
    pRspPacket->pPacket = NULL; /* No data to return */
    pRspPacket->pHeader->length = 0;
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = 0;

    dprintf(DPRINTF_DEFAULT, "PI_FailureStateSet - EXIT\n");

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PI_UnfailInterface
**
** Description: Handle the PI_MISC_UNFAIL_INTERFACE_CMD packet
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_UnfailInterface(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    IPC_REPORT_CONTROLLER_FAILURE *pFailurePacket;
    UINT32      controllerSN;
    UINT8       interface;

    /*
     * Extract the request parms into local vars.
     */
    controllerSN = ((PI_MISC_UNFAIL_INTERFACE_REQ *)(pReqPacket->pPacket))->controllerSN;
    interface = ((PI_MISC_UNFAIL_INTERFACE_REQ *)(pReqPacket->pPacket))->interface;

    dprintf(DPRINTF_DEFAULT, "PI_UnfailInterface: controllerSN=%d  interface=%d - ENTER\n",
            controllerSN, interface);

    /*
     * Allocate a packet and fill in the values from the request packet.
     */
    pFailurePacket = MallocWC(SIZEOF_IPC_INTERFACE_FAILURE);

    pFailurePacket->Type = IPC_FAILURE_TYPE_INTERFACE_FAILED;

    pFailurePacket->FailureData.InterfaceFailure.DetectedBySN = controllerSN;

    pFailurePacket->FailureData.InterfaceFailure.FailedInterfaceID = interface;

    pFailurePacket->FailureData.InterfaceFailure.ControllerSN = controllerSN;

    pFailurePacket->FailureData.InterfaceFailure.InterfaceFailureType = INTERFACE_FAIL_OK;

    FailureManager(pFailurePacket, SIZEOF_IPC_INTERFACE_FAILURE);

    /*
     * Don't release the failure packet, FailureManager owns it now.
     */

    /*
     * No data is returned in the response packet.
     * FailureManager() does not return a status or error code so just
     * return good here.
     */
    pRspPacket->pPacket = NULL; /* No data to return */
    pRspPacket->pHeader->length = 0;
    pRspPacket->pHeader->status = PI_GOOD;
    pRspPacket->pHeader->errorCode = PI_GOOD;

    dprintf(DPRINTF_DEFAULT, "PI_UnfailInterface - EXIT\n");

    return (PI_GOOD);
}

/*----------------------------------------------------------------------------
** Function:    PI_FailInterface
**
** Description: Handle the PI_MISC_FAIL_INTERFACE_CMD packet
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_FailInterface(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    IPC_REPORT_CONTROLLER_FAILURE *pFailurePacket;
    UINT32      controllerSN;
    UINT8       interface;

    /*
     * Extract the request parms into local vars.
     */
    controllerSN = ((PI_MISC_FAIL_INTERFACE_REQ *)(pReqPacket->pPacket))->controllerSN;
    interface = ((PI_MISC_FAIL_INTERFACE_REQ *)(pReqPacket->pPacket))->interface;

    dprintf(DPRINTF_DEFAULT, "PI_FailInterface: controllerSN=%d  interface=%d - ENTER\n",
            controllerSN, interface);

    /*
     * Allocate a packet and fill in the values from the request packet.
     */
    pFailurePacket = MallocWC(SIZEOF_IPC_INTERFACE_FAILURE);

    pFailurePacket->Type = IPC_FAILURE_TYPE_INTERFACE_FAILED;

    pFailurePacket->FailureData.InterfaceFailure.DetectedBySN = controllerSN;

    pFailurePacket->FailureData.InterfaceFailure.FailedInterfaceID = interface;

    pFailurePacket->FailureData.InterfaceFailure.ControllerSN = controllerSN;

    pFailurePacket->FailureData.InterfaceFailure.InterfaceFailureType = INTERFACE_FAIL_OTHER_ERROR;

    FailureManager(pFailurePacket, SIZEOF_IPC_INTERFACE_FAILURE);

    /*
     * Don't release the failure packet, FailureManager owns it now.
     */

    /*
     * No data is returned in the response packet.
     * FailureManager() does not return a status or error code so just
     * return good here.
     */
    pRspPacket->pPacket = NULL; /* No data to return */
    pRspPacket->pHeader->length = 0;
    pRspPacket->pHeader->status = PI_GOOD;
    pRspPacket->pHeader->errorCode = PI_GOOD;

    dprintf(DPRINTF_DEFAULT, "PI_FailInterface - EXIT\n");

    return (PI_GOOD);
}

/*----------------------------------------------------------------------------
** Function:    PI_MiscGetMode
**
** Description: Get the mode parameters
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_MiscGetMode(UNUSED XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    PI_MISC_GET_MODE_RSP *ptrOutPkt = NULL;
    INT32       rc = PI_GOOD;

    /*
     * Allocate memory for the return data.
     */
    ptrOutPkt = MallocWC(sizeof(*ptrOutPkt));

    rc = ModeGet(&ptrOutPkt->modeData);

    /*
     * Attach the return data packet to the main response packet.
     * Fill in the header length and status fields.
     */
    pRspPacket->pPacket = (UINT8 *)ptrOutPkt;
    pRspPacket->pHeader->length = sizeof(*ptrOutPkt);
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = PI_GOOD;

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PI_MiscSetMode
**
** Description: Set the mode parameters
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_MiscSetMode(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc;

    /*
     * Call the low level function to set the mode data.
     *
     */
    rc = ModeSet(&((PI_MISC_SET_MODE_REQ *)(pReqPacket->pPacket))->modeData,
                 &((PI_MISC_SET_MODE_REQ *)(pReqPacket->pPacket))->modeMask);

    dprintf(DPRINTF_DEFAULT, "Mode Bits Set %s\n",
            (rc == GOOD ? "SUCCESSFUL" : "UNSUCCESSFUL"));
    dprintf(DPRINTF_DEFAULT, "..CCB Mode Bits:  0x%08X, 0x%02X\n", modeData.ccb.bits,
            modeData.ccb.bitsDPrintf);
    dprintf(DPRINTF_DEFAULT, "..PROC Mode Bits: 0x%08X, 0x%08X, 0x%08X, 0x%08X\n",
            modeData.proc.word[0], modeData.proc.word[1], modeData.proc.word[2],
            modeData.proc.word[3]);

    /*
     * Attach the return data packet to the main response packet.
     * Fill in the header length and status fields.
     * Since the return code from SetSerialNumber() is different than our
     * normal convention is can't be copied directly into the status field.
     */
    pRspPacket->pPacket = NULL;
    pRspPacket->pHeader->length = 0;
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = PI_GOOD;

    return (PI_GOOD);
}

/*----------------------------------------------------------------------------
** Function:    PI_MiscSerialNumberSet
**
** Description: Set the system or controller serial number
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_MiscSerialNumberSet(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    PI_MISC_SERIAL_NUMBER_SET_REQ *pRequest = NULL;

    pRequest = (PI_MISC_SERIAL_NUMBER_SET_REQ *)pReqPacket->pPacket;

    dprintf(DPRINTF_DEFAULT, "PI_MiscSerialNumberSet - Attempting to set serial number (0x%x) to 0x%x.\n",
            pRequest->which, pRequest->serialNum);

    /*
     * Determine if the request is to set the controller serial number,
     * the system serial number or neither and call the correct method
     * or return an error.
     */
    if (pRequest->which == CONTROLLER_SN)
    {
        dprintf(DPRINTF_DEFAULT, "PI_MiscSerialNumberSet - Set serial number (0x%x) to 0x%x.\n",
                pRequest->which, pRequest->serialNum);

        CntlSetup_SetControllerSN(pRequest->serialNum);
        UpdateProcSerialNumber(pRequest->which, pRequest->serialNum);
    }
    else if (pRequest->which == SYSTEM_SN)
    {
        dprintf(DPRINTF_DEFAULT, "PI_MiscSerialNumberSet - Set serial number (0x%x) to 0x%x.\n",
                pRequest->which, pRequest->serialNum);

        CntlSetup_SetSystemSN(pRequest->serialNum);
        UpdateProcSerialNumber(pRequest->which, pRequest->serialNum);
    }
    else
    {
        /*
         * Invalid serial number type specified.
         */
        rc = PI_ERROR;
    }

    /*
     * Attach the return data packet to the main response packet.
     * Fill in the header length and status fields.
     * Since the return code from SetSerialNumber() is different than our
     * normal convention is can't be copied directly into the status field.
     */
    pRspPacket->pPacket = NULL;
    pRspPacket->pHeader->length = 0;
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = PI_GOOD;

    return rc;
}

/**
******************************************************************************
**
**  @brief      To provide a handler function for the resync mirror records
**              packet interface request (PI_MISC_RESYNC_MIRROR_RECORDS_CMD).
**
**  @param      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
**  @return     INT32 - Packet return status
**
******************************************************************************
**/
INT32 PI_MiscResyncMirrorRecords(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    MRFRWMEM_REQ *ptrInPkt = NULL;
    MRFRWMEM_RSP *ptrOutPkt = NULL;
    void       *pNVA = NULL;
    UINT32      cmdCode = MRFRWMEM;
    UINT8       type;
    UINT16      rid;

    dprintf(DPRINTF_DEFAULT, "PI_MiscResyncMirrorRecords: ENTER\n");

    type = ((PI_MISC_RESYNC_MIRROR_RECORDS_REQ *)pReqPacket->pPacket)->type;
    rid = ((PI_MISC_RESYNC_MIRROR_RECORDS_REQ *)pReqPacket->pPacket)->rid;

    /*
     * If this is a stripe resync operation we need to retieve the
     * mirror resync records from the FE NVA.
     */
    if (type == MRBSTRIPE)
    {
        /*
         * Allocate memory for the NVA records and R/W memory MRP.
         */
        pNVA = MallocSharedWC(GetLength_FENVA());
        ptrInPkt = MallocWC(sizeof(*ptrInPkt));
        ptrOutPkt = MallocSharedWC(sizeof(*ptrOutPkt));

        /*
         * Setup input structure from the input parms.
         */
        ptrInPkt->srcAddr = (void *)(GetProcAddress_FENVA());
        ptrInPkt->dstAddr = pNVA;
        ptrInPkt->length = GetLength_FENVA();

        rc = PI_ExecMRP(ptrInPkt, sizeof(*ptrInPkt), MRFRWMEM,
                        ptrOutPkt, sizeof(*ptrOutPkt), SYNC_RAIDS_TIMEOUT / 3);

        if (rc != PI_GOOD)
        {
            dprintf(DPRINTF_DEFAULT, "PI_MiscResyncMirrorRecords: Could not read P4 NVRAM records (rc: 0x%x, status: 0x%x)\n",
                    rc, ptrOutPkt->header.status);
        }
    }

    if (rc == PI_GOOD)
    {
        /*
         * If this is a stripe resync operation then the FE NVA
         * records have not been read into our memory, if not then
         * the pNVA pointer is NULL.  Either way, the next step is
         * to do the resync opearation so set the command code to
         * the resync MRP code.  This will allow the delayed free
         * to wait for the correct MRP to complete before freeing
         * the memory.
         */
        cmdCode = MRRESYNC;

        /*
         * Send the Resync command to the BE process
         */
        rc = SM_MRResyncWithRetry(type, rid, pNVA);
    }

    /*
     * Free the allocated memory.
     *
     * Use DelayedFree on the input packet since it contains a PCI address.
     *
     * Use DelayedFree on the NVA buffer since it is used as a PCI buffer
     * to the PROC.  Use the "cmdCode" since the buffer could have been
     * used for multiple MRPs and we want to free it for the correct one.
     */
    DelayedFree(MRFRWMEM, ptrInPkt);
    DelayedFree(cmdCode, pNVA);

    if (rc != PI_TIMEOUT)
    {
        Free(ptrOutPkt);
    }

    /*
     * Attach the return data packet to the main response packet.
     * Fill in the header length and status fields.
     * Since the return code from SetSerialNumber() is different than our
     * normal convention is can't be copied directly into the status field.
     */
    pRspPacket->pPacket = NULL;
    pRspPacket->pHeader->length = 0;
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = PI_GOOD;

    return rc;
}

/**
******************************************************************************
**
**  @brief      To provide a handler function for the mirror partner control
**              packet interface request (PI_MiscMirrorPartnerControl).
**
**  @param      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
**  @return     INT32 - Packet return status
**
******************************************************************************
**/
INT32 PI_MiscMirrorPartnerControl(UNUSED XIO_PACKET *pReqPacket,
                                  UNUSED XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    UINT32      controllerSN;
    PI_MISC_MIRROR_PARTNER_CONTROL_REQ *pRequest;
    MP_MIRROR_PARTNER_INFO *pMPInfo = NULL;

    pRequest = (PI_MISC_MIRROR_PARTNER_CONTROL_REQ *)pReqPacket->pPacket;

    dprintf(DPRINTF_DEFAULT, "PI_MiscMirrorPartnerControl - ENTER option %04x\n",
            pRequest->option);

    /*
     * Get the controller serial number for this local controller.
     */
    controllerSN = GetMyControllerSN();

    if ((pRequest->option & MIRROR_PARTNER_CONTROL_OPT_STOPIO) != 0)
    {
        /*
         * I/O must be stopped when assigning mirror partners.
         */
        rc = StopIO(controllerSN, STOP_WAIT_FOR_FLUSH | STOP_NO_BACKGROUND,
                    STOP_NO_SHUTDOWN, START_STOP_IO_USER_CCB_SM, TMO_MPC_STOP_IO);
    }

    /*
     * If requested, continue without a mirror partner.
     */
    if ((pRequest->option & MIRROR_PARTNER_CONTROL_OPT_WOMP) != 0)
    {
        SM_FlushWithoutMirrorPartnerMRP();
    }
    else if ((pRequest->option & MIRROR_PARTNER_CONTROL_OPT_RESYNC_ONLY) != 0)
    {
        if ((pRequest->option & MIRROR_PARTNER_CONTROL_OPT_RESYNC_ALL) != 0)
        {
            /*
             * Resync all the Raid 5 raids.
             */
            SM_ResyncMirrorRecords(controllerSN, MRBALLRAIDS, 0);
        }
        else
        {
            /*
             * Resync the Raid 5 mirror records.
             */
            SM_ResyncMirrorRecords(controllerSN, MRBSTRIPE, 0);
            SM_ResyncMirrorRecords(controllerSN, MRBALLNOTMIRROR, 0);
        }
    }
    else
    {
        /*
         * If MPINFO_VALID bit has been set it means this request packet
         * contains the mirror partner information and it is valid.
         *
         * This packet was extended for release 4.1 to contain additional
         * information so if this packet originated from a controller with
         * pre-4.1 code it will contain the original packet, otherwise it
         * will contain the new packet definition.
         */
        if ((pRequest->option & MIRROR_PARTNER_CONTROL_OPT_MPINFO_VALID) != 0)
        {
            /*
             * Just get a local pointer to the mirror partner information.
             * This pointer is for local use only and must not be freed as
             * the owner of the data is still the request packet.
             */
            pMPInfo = &pRequest->mpInfo;
        }

        /*
         * Assign the mirror partner.
         */
        if (FALSE == AssignMirrorPartnerMRP(pRequest->partnerSN, NULL, pMPInfo))
        {
            rc = PI_ERROR;
        }

        /*
         * If requested, resync the Raid 5 mirror records.
         */
        if ((pRequest->option & MIRROR_PARTNER_CONTROL_OPT_RESYNC) != 0)
        {
            if ((pRequest->option & MIRROR_PARTNER_CONTROL_OPT_RESYNC_ALL) != 0)
            {
                /*
                 * Resync all the Raid 5 raids.
                 */
                SM_ResyncMirrorRecords(controllerSN, MRBALLRAIDS, 0);
            }
            else
            {
                /*
                 * Resync the Raid 5 mirror records.
                 */
                SM_ResyncMirrorRecords(controllerSN, MRBSTRIPE, 0);
                SM_ResyncMirrorRecords(controllerSN, MRBALLNOTMIRROR, 0);
            }
        }

        if (pRequest->partnerSN == controllerSN)
        {
            /*
             * Set the "Not Mirroring" state in the RAIDS
             */
            SM_ModifyRaid5MirrorStatus(FALSE);
        }
        else
        {
            /*
             * Clear the "Not Mirroring" state in the RAIDS
             */
            SM_ModifyRaid5MirrorStatus(TRUE);
        }
    }

    if ((pRequest->option & MIRROR_PARTNER_CONTROL_OPT_STOPIO) != 0)
    {
        /*
         * Resume I/O must be stopped when assigning mirror partners.
         */
        rc = StartIO(controllerSN, START_IO_OPTION_CLEAR_ONE_STOP_COUNT,
                     START_STOP_IO_USER_CCB_SM, 0);
    }

    dprintf(DPRINTF_DEFAULT, "PI_MiscMirrorPartnerControl -  EXIT %04x\n", rc);

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
INT32 PI_MiscMirrorPartnerGetCfg(UNUSED XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    PI_MISC_MIRROR_PARTNER_GET_CFG_RSP *pOutPkt;
    UINT32      rspPktSz = sizeof(*pOutPkt);

    dprintf(DPRINTF_DEFAULT, "PI_MiscMirrorPartnerGetCfg: ENTER\n");

    /*
     * Allocate memory for the MRP return data.
     */
    pOutPkt = MallocSharedWC(rspPktSz);

    /*
     * Send the request to Thunderbolt.  This function handles timeout
     * conditions and task switches while waiting.
     */
    rc = PI_ExecMRP(NULL, 0, MRGETMPCONFIGFE, pOutPkt, rspPktSz, GetGlobalMRPTimeout());

    /*
     * Attach the MRP return data packet to the main response packet.
     * Fill in the header length and status fields.
     */
    pRspPacket->pHeader->status = rc;
    if (rc == PI_TIMEOUT)
    {
        pRspPacket->pPacket = NULL;
        pRspPacket->pHeader->length = 0;
        pRspPacket->pHeader->errorCode = 0;

        /*
         * Don't delete pOutPkt, as the MRP will complete later (unless the FE
         * is toast...)
         */
    }
    else
    {
        pOutPkt->mirrorPartnerInfo.batteryHealth = BatteryHealthState();
        pRspPacket->pPacket = (UINT8 *)pOutPkt;
        pRspPacket->pHeader->length = rspPktSz;
        pRspPacket->pHeader->errorCode = pOutPkt->header.status;
    }

    dprintf(DPRINTF_DEFAULT, "PI_MiscMirrorPartnerGetCfg: rc %d\n", rc);
    return (rc);
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
INT32 PI_MiscAssignMirrorPartner(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc;
    PI_PROC_ASSIGN_MIRROR_PARTNER_REQ *pRequest = NULL;
    PI_PROC_ASSIGN_MIRROR_PARTNER_RSP *pResponse = NULL;

    dprintf(DPRINTF_DEFAULT, "PI_MiscAssignMirrorPartner: Enter\n");

    pRequest = (PI_PROC_ASSIGN_MIRROR_PARTNER_REQ *)pReqPacket->pPacket;
    pResponse = MallocWC(sizeof(*pResponse));

    /*
     * Call the main routine that handles BF/Wookiee differences etc
     */
    rc = (INT32)AssignMirrorPartnerMRP(pRequest->serialNumber, &pResponse->serialNumber, NULL);
    /*
     * Convert response to something the PI understands
     */
    rc = rc ? PI_GOOD : PI_ERROR;

    /*
     * Attach the return data packet to the main response packet.
     * Fill in the header length and status fields.
     */
    pRspPacket->pPacket = (UINT8 *)pResponse;
    pRspPacket->pHeader->length = sizeof(*pResponse);
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = 0;

    dprintf(DPRINTF_DEFAULT, "PI_MiscAssignMirrorPartner: Exit\n");

    return rc;
}

/*----------------------------------------------------------------------------
** Function:    PI_MfgCtrlClean
**
** Description: Cleans a controller.  This will result in the controller
**              requiring serial console configuration or a license applied
**              depending on which option is specified.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_MfgCtrlClean(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    MODEDATA    data;
    MODEDATA    mask;
    TASK_PARMS  parms;

    memset(&data, 0, sizeof(data));
    memset(&mask, 0, sizeof(mask));

    dprintf(DPRINTF_DEFAULT, "PI_MfgCtrlClean: Setting appropriate mode bits...\n");

    mask.ccb.bits = MD_IPC_HEARTBEAT_DISABLE |
        MD_IPC_HEARTBEAT_WATCHDOG_DISABLE |
        MD_FM_DISABLE | MD_CONTROLLER_SUICIDE_DISABLE | MD_FM_RESTART_DISABLE;

    data.ccb.bits = MD_IPC_HEARTBEAT_DISABLE |
        MD_IPC_HEARTBEAT_WATCHDOG_DISABLE |
        MD_FM_DISABLE | MD_CONTROLLER_SUICIDE_DISABLE | MD_FM_RESTART_DISABLE;

    ModeSet(&data, &mask);

    dprintf(DPRINTF_DEFAULT, "PI_MfgCtrlClean: forking task to clean the controller...\n");

    parms.p1 = (UINT32)((PI_MFG_CTRL_CLEAN_REQ *)pReqPacket->pPacket)->option;
    TaskCreate(MfgCtrlCleanTask, &parms);


    /*
     * Attach the MRP return data packet to the main response packet.
     * Fill in the header length and status fields.
     */
    pRspPacket->pPacket = NULL; /* No data to return */
    pRspPacket->pHeader->length = 0;
    pRspPacket->pHeader->status = PI_GOOD;
    pRspPacket->pHeader->errorCode = PI_GOOD;

    return (PI_GOOD);
}

/*----------------------------------------------------------------------------
** Function:    MfgCtrlCleanTask (FORKED)
**
** Description: Forked task to cleans a controller.
**
**  @param      TASK_PARMS* parms - Task parameters required for task creation
**                                  and execution.  In this case p1 is the
**                                  MFG clean option to execute.
**
** Returns:     NONE
**
**--------------------------------------------------------------------------*/
static void MfgCtrlCleanTask(TASK_PARMS *parms)
{
    UINT8       option = (UINT8)parms->p1;

    dprintf(DPRINTF_DEFAULT, "MfgCtrlCleanTask: pausing 5 seconds before starting...\n");
    TaskSleepMS(5000);

    /*
     * Lock Front-End, and pass options.
     */
    Clean_Controller(1, option);
}

/*----------------------------------------------------------------------------
** Function:    Lock_Shutdown_FE
**
** Description: Locks caches, takes FE down, leave BE ready for shutdown.
**
**--------------------------------------------------------------------------*/

void Lock_Shutdown_FE(void)
{
    PI_PROC_TARGET_CONTROL_RSP *pResponse = NULL;

    /* Shutdown the port server. */
    dprintf(DPRINTF_DEFAULT, "Lock_Shutdown_FE: Shutting down the portserver.\n");
    ShutdownPortServer();

    /* Set the PortServerClientAddress to 0, This will stop the Async Client. */
    dprintf(DPRINTF_DEFAULT, "Lock_Shutdown_FE: Shutting down async client.\n");
    SetPortServerClientAddr(0);

    /* Shutdown IPC. */
    dprintf(DPRINTF_DEFAULT, "Lock_Shutdown_FE: Shutting down IPC.\n");
    IpcShutDown();

    /* Set the caches to be in use. */
    dprintf(DPRINTF_DEFAULT, "Lock_Shutdown_FE: Waiting for cache locks and setting caches in use.\n");
    CacheStateWaitUpdating(cacheDiskBaysState);
    CacheStateSetInUse(cacheDiskBaysState);
#ifndef NO_PDISK_CACHE
    CacheStateWaitUpdating(cachePhysicalDisksState);
    CacheStateSetInUse(cachePhysicalDisksState);
#endif  /* NO_PDISK_CACHE */
#ifndef NO_VDISK_CACHE
    CacheStateWaitUpdating(cacheVirtualDisksState);
    CacheStateSetInUse(cacheVirtualDisksState);
#endif  /* NO_VDISK_CACHE */
#ifndef NO_RAID_CACHE
    CacheStateWaitUpdating(cacheRaidsState);
    CacheStateSetInUse(cacheRaidsState);
#endif  /* NO_RAID_CACHE */
#ifndef NO_SERVER_CACHE
    CacheStateWaitUpdating(cacheServersState);
    CacheStateSetInUse(cacheServersState);
#endif  /* NO_SERVER_CACHE */
#ifndef NO_TARGET_CACHE
    CacheStateWaitUpdating(cacheTargetsState);
    CacheStateSetInUse(cacheTargetsState);
#endif  /* NO_TARGET_CACHE */

    /* Reset the QLogic cards. */
    if (PowerUpComplete())
    {
        dprintf(DPRINTF_DEFAULT, "Lock_Shutdown_FE: Resetting FE QLogic cards.\n");
        ResetInterfaceFE(GetMyControllerSN(), 0xFF, RESET_PORT_NO_INIT);
    }

    /* Reset master configuration */
    dprintf(DPRINTF_DEFAULT, "Lock_Shutdown_FE: Reset master configuration.\n");
    ResetMasterConfigNVRAM();

    /* Terminate the broadcast events */
    TerminateBroadcasts();

    /* Tell the BEP that this controller will be a slave controller. */
    dprintf(DPRINTF_DEFAULT, "Lock_Shutdown_FE: Set controller to SLAVE.\n");
    MRP_Awake(MRAWAKE_SLAVE);

    /* Suspend BE processes */
    dprintf(DPRINTF_DEFAULT, "Lock_Shutdown_FE: Suspend BE Processes.\n");
    pResponse = SM_TargetControl(GetMyControllerSN(), TC_PREP_MOVE);

    /* Free the target control response packet. */
    Free(pResponse);
}


/*----------------------------------------------------------------------------
** Function:    Clean_Controller
**
** Description: Clean controller of configruation (see MFG_CTRL_CLEAN_OPT ions).
**
** Inputs:      1 means to Lock Caches and Shutdown FE.
** Inputs:      option is the MFG_CTRL_CLEAN_OPT ions.
**
** Returns:     Does not return.
**--------------------------------------------------------------------------*/

void Clean_Controller(int flag_lock, UINT8 option)
{
    char        msg1[] = { "\n****\n** CONTROLLER IS BEING ZEROED - PLEASE WAIT\n****\n" };
    struct dirent *pdirent;
    DIR        *dirp;
    char        fullpath[PATH_MAX];

/* Ignore any XIO_PLATFORM_SIGNAL and continue. */
    signal(XIO_PLATFORM_SIGNAL, SIG_IGN);

    /*
     * Locks caches, takes FE down, leave BE ready for shutdown.
     */
    if (flag_lock == 1)
    {
        Lock_Shutdown_FE();
    }

    if (option & MFG_CTRL_CLEAN_OPT_SERIAL_MESSAGES)
    {
        LogMessage(LOG_TYPE_INFO, "SERCON-Serial console ZEROING");
    }

    ClearReplacementController();

    if (option & MFG_CTRL_CLEAN_OPT_SERIAL_MESSAGES)
    {
        /* Send the string to the serial console to be displayed. */
        SerialBufferedWriteString(msg1, strlen(msg1));
        /* Flush the string to the console. */
        SerialBufferedWriteFlush(TRUE);
    }

    /*
     * Clear physical disks (write same to the drives).
     */
    if (option & MFG_CTRL_CLEAN_OPT_FORCE_CLEAN_DRIVES)
    {
        dprintf(DPRINTF_DEFAULT, "Clean_Controller: Clearing physical disks.\n");
        MfgCtrlClean_ClearPhysicalDisks();
        dprintf(DPRINTF_DEFAULT, "Clean_Controller: Clearing physical disks, sent -- wait 2 seconds...\n");
        XK_TaskSleepMS(2000);
        dprintf(DPRINTF_DEFAULT, "Clean_Controller: Clearing physical disks, done waiting.\n");
    }
    /*
     * Clean various saved memorys.
     */
    /*
     * Initialize BE NVRAM (empty with good CRC).
     */
    dprintf(DPRINTF_DEFAULT, "Clean_Controller: Reset BE process NVRAM.\n");
    MfgCtrlClean_ResetPROCNVRAM();

    /*
     * Initialize CCB NVRAM - Option determines which type of
     *                        initialization is done.
     */
    dprintf(DPRINTF_DEFAULT, "Clean_Controller: Initializing CCB NVRAM (0x%x).\n",
            option);
    MfgCtrlClean_InitCCBNVRAM(option & MFG_CTRL_CLEAN_OPT_LICENSE);

    /*
     * Destroy everything in the XSSA stable storage.
     */
    dprintf(DPRINTF_DEFAULT, "Clean_Controller: Initializing XSSA stable storage.\n");
    MemSetNVRAMBytes(&XSSAData, 0, sizeof(XSSAData));
    /*
     * Destroy Client Persistent Storage.
     */
    dprintf(DPRINTF_DEFAULT, "Clean_Controller: Initializing Client Persistent storage.\n");
    DeleteAllPersistentFiles();

    /*
     * Destroy Xio Web Service files.
     */
    dirp = opendir(XioWebService_Dir);
    dprintf(DPRINTF_DEFAULT, "Clean_Controller: opendir returned (0x%x)\n", (UINT32)dirp);
    while (dirp)
    {
        pdirent = readdir(dirp);
        if (pdirent == NULL)
        {
            break;
        }
        /*
         * . and .. files need not process
         */
        if (pdirent->d_name[0] == '.')
        {
            continue;
        }
        dprintf(DPRINTF_DEFAULT, "Clean_Controller: unlinking (%s)\n", pdirent->d_name);
        strcpy(fullpath, XioWebService_Dir);
        strncat(fullpath, "/", sizeof(fullpath));
        strncat(fullpath, pdirent->d_name, sizeof(fullpath));
        unlink(fullpath);
    }

    /*
     * Turn off the hardware monitor, since it can generate unwanted log events
     * when the buffer boards (or MicroMemory board) are shut down.
     */
    dprintf(DPRINTF_DEFAULT, "Clean_Controller: Turn off the hardware monitor\n");
    HWM_MonitorDisable();

    /*
     * Clear out the BMC's Ethernet configuration
     */
    dprintf(DPRINTF_DEFAULT, "Clean_Controller: Wipe the BMC configuration\n");
    HWM_ConfigureEthernet();

    /*
     * Clear MM board.
     */
    dprintf(DPRINTF_DEFAULT, "Clean_Controller: Clearing MM board.\n");
    MfgCtrlClean_MMClear();

    /*
     * Two cases:
     *   OPT_LICENSE: do not shutdown the MM buffer board.
     *   Default: shutdown the MM buffer board (power cycle).
     *
     * If the controller will be powered off, then we need to shut down the
     * battery backed DIMMs (or MicroMemory board for Wookiee).  Without the
     * OPT_LICENSE flag, we will force the controller to be turned off at
     * the end of this task, either via self-power-down, or push the button..
     */

    if (!(option & MFG_CTRL_CLEAN_OPT_LICENSE))
    {
        dprintf(DPRINTF_DEFAULT, "Clean_Controller: Shutting down buffer board\n");
        BufferBoardShutdownControl();
    }

    /*
     * Clear Logs if requested by the user (option bit not set).
     */
    if (!(option & MFG_CTRL_CLEAN_OPT_NO_LOG_CLEAR))
    {
        dprintf(DPRINTF_DEFAULT, "Clean_Controller: Clearing logs.\n");
        MfgCtrlClean_LogClear();
    }

    /*
     * Do the final cleanup.
     */
    dprintf(DPRINTF_DEFAULT, "Clean_Controller: Setting Clean Shutdown Flag\n");
    if (SetCleanShutdown() != GOOD)
    {
        /* This isn't catastrophic, just print it out. */
        dprintf(DPRINTF_DEFAULT, "Clean_Controller: Failed to set Clean Shutdown Flag\n");
    }

    dprintf(DPRINTF_DEFAULT, "Clean_Controller: removing files to clean system.\n");

    /*
     * If the option is to wipe out full NVRAM (manufacturing clean), then
     * go to a hang loop to avoid any log messages generated from a reboot.
     * Turn on the offline led to indicate we are done.
     */
    if (!(option & MFG_CTRL_CLEAN_OPT_LICENSE))
    {
        /*
         * Remove ethernet config file.
         */
        unlink("/etc/sysconfig/network/ifcfg-eth0");
        unlink("/etc/sysconfig/network/~ifcfg-eth0");
        /* Following won't hurt on a 3000. */
        unlink("/etc/sysconfig/network/ifcfg-icl0");
        /* Delete the default route. */
        unlink("/etc/sysconfig/network/routes");

        /* Delete CCB data files. */
        unlink("/opt/xiotech/ccbdata/CCB_FLASH.mmf");
        unlink("/opt/xiotech/ccbdata/CCB_NVRAM.mmf");
        unlink("/opt/xiotech/ccbdata/RAIDMON.mmf");
        unlink("/opt/xiotech/ccbdata/ccb.cfg");

    }

    /* Delete PROC data files. */
    unlink("/opt/xiotech/procdata/shared_memory_NVSRAM_BE");
    unlink("/opt/xiotech/procdata/shared_memory_NVSRAM_FE");

    if (!(option & MFG_CTRL_CLEAN_OPT_LICENSE))
    {
        /* Stop possible rcsshd starting, etc. */
        dprintf(DPRINTF_DEFAULT, "Clean_Controller: execute /root/stdsetup\n");
        XK_System("/root/stdsetup");

        /* Create zeroed files for reboot. */
        dprintf(DPRINTF_DEFAULT, "Clean_Controller: zero CCB_FLASH.mmf\n");
        XK_System("/bin/dd if=/dev/zero of=/opt/xiotech/ccbdata/CCB_FLASH.mmf bs=512 count=32768 1>/dev/null 2>&1");
        dprintf(DPRINTF_DEFAULT, "Clean_Controller: zero CCB_NVRAM.mmf\n");
        XK_System("/bin/dd if=/dev/zero of=/opt/xiotech/ccbdata/CCB_NVRAM.mmf bs=512 count=256 1>/dev/null 2>&1");
    }

    dprintf(DPRINTF_DEFAULT, "Clean_Controller: zero shared_memory_NVSRAM_BE\n");
    XK_System("/bin/dd if=/dev/zero of=/opt/xiotech/procdata/shared_memory_NVSRAM_BE bs=512 count=8192 1>/dev/null 2>&1");
    dprintf(DPRINTF_DEFAULT, "Clean_Controller: zero shared_memory_NVSRAM_FE\n");
    XK_System("/bin/dd if=/dev/zero of=/opt/xiotech/procdata/shared_memory_NVSRAM_FE bs=512 count=8192 1>/dev/null 2>&1");

    if (option & MFG_CTRL_CLEAN_OPT_SERIAL_MESSAGES)
    {
        dprintf(DPRINTF_DEFAULT, "Clean_Controller: Do message to serial port.\n");
        snprintf(msg1, CONSOLE_COLUMNS, "\n****\n** RESETTING SYSTEM - PLEASE WAIT\n****\n");
        SerialBufferedWriteString(msg1, strlen(msg1));
        SerialBufferedWriteFlush(TRUE);
    }

/* Restore XIO_PLATFORM_SIGNAL and continue. */
    L_SignalHandlerAdd(XIO_PLATFORM_SIGNAL, DeadLoopInterrupt, true);

    /*
     * There are three types of shutdowns dependant upon option:
     *   MFG_CTRL_CLEAN_OPT_POWER_DOWN: Power controller off.
     *   MFG_FULL (not license): go into a hang loop to avoid any log messages
     *                           generated from a reboot.
     *                           Turn on the offline led to indicate we are done.
     *   MFG_CTRL_CLEAN_OPT_LICENSE: Reset the controller (reboot).
     */

    if (option & MFG_CTRL_CLEAN_OPT_POWER_DOWN)
    {
        while (1)
        {
            dprintf(DPRINTF_DEFAULT, "Clean_Controller: Call errExit(ERR_EXIT_SHUTDOWN)\n");
            errExit(ERR_EXIT_SHUTDOWN);

/* Go to sleep for 2000 ms (2 seconds) to allow above signals to be processed. */
            XK_TaskSleepMS(2000);
        }
    }

    /*
     * Full clean, power off required. (By pressing button for 12 seconds.)
     */
    if (!(option & MFG_CTRL_CLEAN_OPT_LICENSE))
    {
        dprintf(DPRINTF_DEFAULT, "Clean_Controller: Hang -- waiting for power off\n");
        LEDSetOffline(TRUE);
        DeadLoop(EVENT_MFG_CLEAN_DONE, TRUE);
    }

    /*
     * MFG_CTRL_CLEAN_OPT_LICENSE, reset (reboot) the controller
     */
    dprintf(DPRINTF_DEFAULT, "Clean_Controller: Resetting all processes.\n");
    ProcessReset(PROCESS_ALL);
}

/*----------------------------------------------------------------------------
** Function:    SerSetupCtrlClean
**
** Description: Clean controller without affecting the BE.
**
** Inputs:      none.
**
** Returns:     NONE
**
**--------------------------------------------------------------------------*/
void SerSetupCtrlClean(void)
{
    /* Set the caches to be in use. */
    dprintf(DPRINTF_DEFAULT, "SerSetupCtrlClean: Waiting for cache locks and setting caches in use.\n");
    CacheStateWaitUpdating(cacheDiskBaysState);
    CacheStateSetInUse(cacheDiskBaysState);
#ifndef NO_PDISK_CACHE
    CacheStateWaitUpdating(cachePhysicalDisksState);
    CacheStateSetInUse(cachePhysicalDisksState);
#endif  /* NO_PDISK_CACHE */
#ifndef NO_VDISK_CACHE
    CacheStateWaitUpdating(cacheVirtualDisksState);
    CacheStateSetInUse(cacheVirtualDisksState);
#endif  /* NO_VDISK_CACHE */
#ifndef NO_RAID_CACHE
    CacheStateWaitUpdating(cacheRaidsState);
    CacheStateSetInUse(cacheRaidsState);
#endif  /* NO_RAID_CACHE */
#ifndef NO_SERVER_CACHE
    CacheStateWaitUpdating(cacheServersState);
    CacheStateSetInUse(cacheServersState);
#endif  /* NO_SERVER_CACHE */
#ifndef NO_TARGET_CACHE
    CacheStateWaitUpdating(cacheTargetsState);
    CacheStateSetInUse(cacheTargetsState);
#endif  /* NO_TARGET_CACHE */

    /*
     * Reset the QLogic cards.
     */
    if (PowerUpComplete())
    {
        dprintf(DPRINTF_DEFAULT, "SerSetupCtrlClean: Resetting FE QLogic cards.\n");
        ResetInterfaceFE(GetSerialNumber(CONTROLLER_SN), 0xFF, RESET_PORT_NO_INIT);
    }

    /*
     * Terminate the broadcast events
     */
    TerminateBroadcasts();

    /*
     * Tell the BEP that this controller will be a slave controller.
     */
    dprintf(DPRINTF_DEFAULT, "SerSetupCtrlClean: Set controller to SLAVE.\n");
    MRP_Awake(MRAWAKE_SLAVE);

    /*
     * Initialize BE NVRAM (empty with good CRC).
     */
    dprintf(DPRINTF_DEFAULT, "SerSetupCtrlClean: Reset BE process NVRAM.\n");
    MfgCtrlClean_ResetPROCNVRAM();

    /*
     * Initialize CCB NVRAM
     */
    dprintf(DPRINTF_DEFAULT, "SerSetupCtrlClean: Initializing CCB NVRAM (0x%x).\n",
            INIT_CCB_NVRAM_TYPE_FULL);
    MfgCtrlClean_InitCCBNVRAM(INIT_CCB_NVRAM_TYPE_FULL);

    /*
     * Destroy everything in the XSSA stable storage.
     */
    dprintf(DPRINTF_DEFAULT, "SerSetupCtrlClean: Initializing XSSA stable storage.\n");
    MemSetNVRAMBytes(&XSSAData, 0, sizeof(XSSAData));

    /*
     * Clear out the MicroMemory board for Wookie.
     */
    MfgCtrlClean_MMClear();

    /*
     * Clear Logs
     */
    dprintf(DPRINTF_DEFAULT, "SerSetupCtrlClean: Clearing logs.\n");
    MfgCtrlClean_LogClear();
}


/**
******************************************************************************
**
**  @brief      Sends the request to clear the MicroMemory board for a
**              Wookiee controller.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void MfgCtrlClean_MMClear(void)
{
    INT32       rc = PI_GOOD;
    XIO_PACKET  reqPacket;
    XIO_PACKET  rspPacket;

    /*
     * Allocate memory for the request (header and data) and the
     * response header. The response data will be allocated in the called
     * function.
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    reqPacket.pPacket = MallocWC(sizeof(PI_STATS_BUFFER_BOARD_REQ));
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    rspPacket.pPacket = NULL;
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    reqPacket.pHeader->commandCode = PI_STATS_BUFFER_BOARD_CMD;
    reqPacket.pHeader->length = sizeof(PI_STATS_BUFFER_BOARD_REQ);

    ((PI_STATS_BUFFER_BOARD_REQ *)reqPacket.pPacket)->commandCode = NV_CMD_CLEAR;

    /*
     * Issue the command through the packet command handler
     */
    rc = PortServerCommandHandler(&reqPacket, &rspPacket);

    if (rc != PI_GOOD)
    {
        dprintf(DPRINTF_DEFAULT, "MfgCtrlClean_MMClear - Failed to clear the MM board.\n");
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
}


/*----------------------------------------------------------------------------
** Function:    MfgCtrlClean_LogClear
**
** Description: Calls the packet interface to clear the controller logs.
**
** Inputs:      NONE
**
** Returns:     NONE
**
**--------------------------------------------------------------------------*/
static void MfgCtrlClean_LogClear(void)
{
    INT32       rc;
    XIO_PACKET  reqPacket;
    XIO_PACKET  rspPacket;

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

    reqPacket.pHeader->commandCode = PI_LOG_CLEAR_CMD;
    reqPacket.pHeader->length = 0;

    /*
     * Issue the command through the packet command handler
     */
    rc = PortServerCommandHandler(&reqPacket, &rspPacket);

    if (rc != PI_GOOD)
    {
        dprintf(DPRINTF_DEFAULT, "MfgCtrlClean_ClearLogs - Failed to clear the controllers logs.\n");
    }

    /*
     * Free the allocated memory
     */
    Free(reqPacket.pHeader);
    Free(rspPacket.pHeader);
}

/*----------------------------------------------------------------------------
** Function:    MfgCtrlClean_InitCCBNVRAM
**
** Description: Calls the packet interface to initialize the CCB NVRAM.
**
** Inputs:      UINT8 option - Option on how to initialze the CCB NVRAM.
**
** Returns:     NONE
**
**--------------------------------------------------------------------------*/
static void MfgCtrlClean_InitCCBNVRAM(UINT8 option)
{
    INT32       rc;
    XIO_PACKET  reqPacket;
    XIO_PACKET  rspPacket;

    /*
     * Allocate memory for the request (header and data) and the
     * response header. The response data will be allocated in the called
     * function.
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    reqPacket.pPacket = MallocWC(sizeof(PI_DEBUG_INIT_CCB_NVRAM_REQ));
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    rspPacket.pPacket = NULL;
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    reqPacket.pHeader->commandCode = PI_DEBUG_INIT_CCB_NVRAM_CMD;
    reqPacket.pHeader->length = 0;

    ((PI_DEBUG_INIT_CCB_NVRAM_REQ *)reqPacket.pPacket)->type = option;

    /* Issue the command through the packet command handler */
    rc = PacketCommandHandler(&reqPacket, &rspPacket);

    if (rc != PI_GOOD)
    {
        dprintf(DPRINTF_DEFAULT, "MfgCtrlClean_InitCCBNVRAM - Failed to initialize the CCB NVRAM.\n");
    }

    /* Free the allocated memory */
    Free(reqPacket.pHeader);
    Free(reqPacket.pPacket);
    Free(rspPacket.pHeader);
}

/*----------------------------------------------------------------------------
** Function:    MfgCtrlClean_ResetPROCNVRAM
**
** Description: Reset PROC NVRAM to an empty state with good CRC.
**
** Inputs:      NONE
**
** Returns:     NONE
**
**--------------------------------------------------------------------------*/
static void MfgCtrlClean_ResetPROCNVRAM(void)
{
    /*
     * See Proc/inc/nvr.h for definition of NVRII.
     * 0x44 = sizeof(struct NVRII) + sizeof(struct NVH).
     * NVRII.length = 0x44 as per bytes 12-15 (ordinal) below (little endian).
     *
     * The last 8 bytes are a struct NVH with 4 extra bytes.
     * NVH is of recType=0x03 (NRT_EOF), with recLen=0x04, status=0.
     *
     * The setting of the default is done when NVRAM is read by the BackEnd,
     * with file Shared/Src/L_XIO3d.c, calling is_nvram_p2_initialized, and
     * the actual values for initialization are in Proc/src/nvram.c.
     *
     * Note: The magic number must be set for this to take effect. And the
     * checksum must also be right. Lastly gPri must be 7 (MAX_GLOBAL_PRIORITY).
     */
    static UINT8 cleanNVRAM_pattern[] = {
        0x11, 0x70, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00, 0x00,
        0xe1, 0x37, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x04, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    UINT8 *cleanNVRAM = MallocSharedWC(sizeof(*cleanNVRAM_pattern));
    memcpy(cleanNVRAM, cleanNVRAM_pattern, sizeof(*cleanNVRAM_pattern));

    dprintf(DPRINTF_DEFAULT, "MfgCtrlClean_ResetPROCNVRAM: Reloading NVRAM.\n");

    SM_NVRAMRestore(MRNOOVERLAY | MRNOPCI, cleanNVRAM);

    Free(cleanNVRAM);
}

/*----------------------------------------------------------------------------
** Function:    MfgCtrlClean_ClearPhysicalDisks
**
** Description: Clears all the physical disks on the system.
**
** Inputs:      NONE
**
** Returns:     NONE
**
**--------------------------------------------------------------------------*/
static void MfgCtrlClean_ClearPhysicalDisks(void)
{
    PI_PDISKS_RSP *pPDisks = NULL;
    UINT16      count = 0;
    INT32       rc = PI_GOOD;

    pPDisks = PhysicalDisks();

    if (pPDisks)
    {
        for (count = 0; count < pPDisks->count; count++)
        {
            dprintf(DPRINTF_DEFAULT, "%s: Clearing physical disk (%8.8x%8.8x lun %d)\n",
                    __func__,
                    bswap_32((UINT32)pPDisks->pdiskInfo[count].pdd.wwn),
                    bswap_32((UINT32)(pPDisks->pdiskInfo[count].pdd.wwn >> 32)),
                    pPDisks->pdiskInfo[count].pdd.lun);

            rc = MfgCtrlClean_WriteSame(pPDisks->pdiskInfo[count].pdd.wwn,
                                        pPDisks->pdiskInfo[count].pdd.lun);
            if (rc != PI_GOOD)
            {
                dprintf(DPRINTF_DEFAULT, "%s: Failed to clear physical disk (%8.8x%8.8x lun %d)\n",
                        __func__,
                        bswap_32((UINT32)pPDisks->pdiskInfo[count].pdd.wwn),
                        bswap_32((UINT32)(pPDisks->pdiskInfo[count].pdd.wwn >> 32)),
                        pPDisks->pdiskInfo[count].pdd.lun);
            }
        }
        Free(pPDisks);
    }
}

/*----------------------------------------------------------------------------
** Function:    MfgCtrlClean_WriteSame
**
** Description: Writes a buffer to the drive using the SCSI commands.
**
** Inputs:      UINT64 wwn - WWN of the device to write
**              UINT16 lun - Lun of the device to write
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 MfgCtrlClean_WriteSame(UINT64 wwn, UINT16 lun)
{
    MRSCSIIO_REQ *inPkt = NULL;
    MRSCSIIO_RSP *outPkt = NULL;
    INT32       rc = PI_GOOD;
    void       *data = NULL;

    inPkt = MallocSharedWC(sizeof(*inPkt));
    outPkt = MallocSharedWC(sizeof(*outPkt));

    /*
     * Set input parm from the input.
     */
#ifdef  DISABLE_WRITE_SAME
    data = MallocSharedWC(512 * 0x100);
    inPkt->blen = 512 * 0x100;
    inPkt->cdb[0] = 0x2a;       /* write 10 */
#else   /* DISABLE_WRITE_SAME */
    data = MallocSharedWC(512);
    inPkt->blen = 512;
    inPkt->cdb[0] = 0x41;       /* write same */
#endif /* DISABLE_WRITE_SAME */
    inPkt->idchc = MRSCSIIO_USE_WWN;
    inPkt->wwn = wwn;
    inPkt->cmdlen = 10;
    inPkt->func = MRSCSIIO_OUTPUT;
    inPkt->strat = MRSCSIIO_NORM;
    inPkt->timeout = 10;
    inPkt->flags = 1;
    inPkt->retry = 5;
    inPkt->lun = lun;
    inPkt->bptr = data;

    inPkt->cdb[1] = 0x00;
    inPkt->cdb[2] = 0x00;
    inPkt->cdb[3] = 0x00;
    inPkt->cdb[4] = 0x00;
    inPkt->cdb[5] = 0x00;
    inPkt->cdb[6] = 0x00;
    inPkt->cdb[7] = 0x01;       /* Write 256 blocks */
    inPkt->cdb[8] = 0x00;       /* part of the 256 blocks */
    inPkt->cdb[9] = 0x00;

    /*
     * Send the request to Thunderbolt.  This function handles timeout
     * conditions and task switches while waiting.
     */
    rc = PI_ExecMRP(inPkt, sizeof(*inPkt), MRSCSIIO,
                    outPkt, sizeof(*outPkt), MRP_STD_TIMEOUT);

    if (rc == PI_ERROR)
    {
        dprintf(DPRINTF_DEFAULT, "WriteSame: MRP failed: SenseKey 0x%02hhX, ASC 0x%02hhX ASCQ 0x%02hhX\n",
                outPkt->sense, outPkt->asc, outPkt->ascq);
    }

    /*
     * Free the allocated memory, use DelayedFree for the input packet
     * since it contains a PCI address.
     */
    DelayedFree(MRSCSIIO, inPkt);
    DelayedFree(MRSCSIIO, data);

    if (rc != PI_TIMEOUT)
    {
        Free(outPkt);
    }

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PI_GetWorksetInfo
**
** Description: Get all worksets and return in one packet.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
** WARNING:     A change to MAX_WORKSETS should not affect this function
**              but may affect users of the return data from this function.
**
**--------------------------------------------------------------------------*/
INT32 PI_GetWorksetInfo(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    MRGETWSINFO_REQ *pMRPReq = NULL;
    MRGETWSINFO_RSP *pMRPRsp = NULL;
    PI_MISC_GET_WORKSET_INFO_RSP *pRspPkt = NULL;
    UINT32      mrpRspLength = 0;
    UINT32      piRspLength = 0;
    INT32       rc = PI_GOOD;
    UINT16      worksetCount = 0;

    dprintf(DPRINTF_PI_COMMANDS, "PI_GetWorksetInfo - ENTER\n");

    /*
     * Determine the number of worksets being requested based on the
     * input ID.
     */
    if (((PI_MISC_GET_WORKSET_INFO_REQ *)(pReqPacket->pPacket))->id == GET_ALL_WORKSETS)
    {
        worksetCount = MAX_WORKSETS;
    }
    else
    {
        worksetCount = 1;
    }

    /*
     * Determine the MRP and Packet Interface response length based on the
     * number of worksets requested.
     */
    mrpRspLength = sizeof(*pMRPRsp) + (worksetCount * sizeof(DEF_WORKSET));
    piRspLength = sizeof(*pRspPkt) + (worksetCount * sizeof(DEF_WORKSET));

    /*
     * Allocate memory for the MRP request, response and for the Packet
     * Interface response.
     */
    pMRPReq = MallocWC(sizeof(pMRPReq));

    pMRPRsp = MallocSharedWC(mrpRspLength);
    pRspPkt = MallocWC(piRspLength);

    /*
     * Issue the request to get the Workset Info and build the response
     * packet.
     */
    pMRPReq->id = ((PI_MISC_GET_WORKSET_INFO_REQ *)(pReqPacket->pPacket))->id;

    /*
     * Send the request to proc.
     */
    rc = PI_ExecMRP(pMRPReq, sizeof(pMRPReq), MRGETWSINFO,
                    pMRPRsp, mrpRspLength, MRP_STD_TIMEOUT);

    /*
     * If the MRP was successful copy the data into the response packet
     */
    if (rc == PI_GOOD)
    {
        /*
         * Set the count field in the response data and copy the worksets
         * into the Packet Interface response packet.
         */
        pRspPkt->count = worksetCount;

        memcpy((void *)(pRspPkt->workset), (void *)(pMRPRsp->workset),
               (worksetCount * sizeof(DEF_WORKSET)));
    }

    dprintf(DPRINTF_PI_COMMANDS, "PI_GetWorksetInfo - status=0x%X  count=%d  rspLength=%d\n",
            rc, pRspPkt->count, piRspLength);

    /*
     * Attach the return data packet to the main response packet.
     * Fill in the header length and status fields.
     */
    pRspPacket->pPacket = (UINT8 *)pRspPkt;
    pRspPacket->pHeader->length = piRspLength;
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = pMRPRsp->header.status;

    /*
     * Free the MRP request and response packets.
     */
    Free(pMRPReq);
    Free(pMRPRsp);

    return (rc);
}


/*----------------------------------------------------------------------------
** Function:    PI_CacheRefreshCCB
**
** Description: Refresh the CCB cahed based on the input mask and flag
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_CacheRefreshCCB(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    UINT32      cacheMask;

    /*
     * Get the cache refresh mask from the request packet.
     */
    cacheMask = ((PI_CACHE_REFRESH_CCB_REQ *)(pReqPacket->pPacket))->cacheMask;

    dprintf(DPRINTF_DEFAULT, "PI_CacheRefreshCCB - cacheMask=0x%X\n", cacheMask);

    /*
     * Call the function to invalidate the appropriate cache, thus causing it
     * to refresh.  The return code doesn't mean anything here so it
     * is ignored.  This call is pretty funky - its done this way so
     * that an X1 Change Event gets sent up to the XSSA (ICON) as well as
     * refreshing the cache.  Qustions?  Better ask Bryan.
     */
    SendX1ChangeEvent(ResolveX1ReasonFromActionReason(cacheMask, 0xFFFFFFFF));

    /*
     * Attach the return data packet to the main response packet.
     * Fill in the header length and status fields.
     */
    pRspPacket->pPacket = NULL;
    pRspPacket->pHeader->length = 0;
    pRspPacket->pHeader->status = PI_GOOD;
    pRspPacket->pHeader->errorCode = PI_GOOD;

    dprintf(DPRINTF_DEFAULT, "PI_CacheRefreshCCB - ENTER\n");

    return (PI_GOOD);
}

/*----------------------------------------------------------------------------
** Function:    PI_DLMHeartbeatList
**
** Description: Set up the DLM Heartbeat List
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_DLMHeartbeatList(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    MRFEFIBREHLIST_REQ *pMRPReq = NULL;
    MRFEFIBREHLIST_RSP *pMRPRsp = NULL;
    INT32       rc = PI_GOOD;
    UINT32      reqPktSize;
    UINT32      retries = HEARTBEAT_RETRIES;
    UINT16      numControllers;

    /*
     * Calculate the size of the request packet.  Always allocate space
     * for at least one controller even if numControllers == 0.
     */
    numControllers = ((PI_SET_DLM_HEARTBEAT_LIST_REQ *)(pReqPacket->pPacket))->numControllers;

    if (numControllers == 0)
    {
        reqPktSize = sizeof(*pMRPReq) + sizeof(UINT32);
    }
    else
    {
        reqPktSize = sizeof(*pMRPReq) + (numControllers * sizeof(UINT32));
    }

    dprintf(DPRINTF_PI_COMMANDS, "PI_DLMHeartbeatList - numControllers=%d\n",
            numControllers);

    /*
     * Allocate request and response packets.
     */
    pMRPReq = MallocWC(reqPktSize);
    pMRPRsp = MallocSharedWC(sizeof(*pMRPRsp));

    /*
     * Load the number of controllers from the request packet.
     */
    pMRPReq->numControllers = numControllers;

    /*
     * Copy the controller list into the MRP input packet.  If
     * numControllers == 0 fill in a default value for the first controller.
     */
    if (numControllers > 0)
    {
        memcpy(pMRPReq->controllers,
               ((PI_SET_DLM_HEARTBEAT_LIST_REQ *)(pReqPacket->pPacket))->controllers,
               (numControllers * sizeof(UINT32)));
    }
    else
    {
        pMRPReq->controllers[0] = 0;
    }

    /*
     * This command has retries defined by HEARTBEAT_TRIES
     */
    while (retries > HEARTBEAT_RETRIES)
    {
        /*
         * Issue the request to the proc.
         */
        rc = PI_ExecMRP(pMRPReq, reqPktSize, MRFEFIBREHLIST,
                        pMRPRsp, sizeof(*pMRPRsp), (SET_FE_FIBRE_LIST_TIMEOUT / 12));

        /*
         * If the completion status is good or timeout we are done.
         * We could retry on a timeout but the old code didn't so
         * leave it that way for now.
         */
        if ((rc == PI_GOOD) || (rc == PI_TIMEOUT))
        {
            break;
        }
        else
        {
            /*
             * All other error conditions get retried.  Print a debug
             * message.
             */
            dprintf(DPRINTF_PI_COMMANDS, "PI_DLMHeartbeatList FAILED.  retry=%d  rc=0x%X  status=0x%X  invalidController=%d\n",
                    retries, rc, pMRPRsp->header.status, pMRPRsp->invalidController);

            /*
             * Decrement the retry count and wait a bit before trying again.
             */
            retries--;
            TaskSleepMS(500);
        }
    }

    /*
     * Log completion status to the debug console.
     */
    dprintf(DPRINTF_PI_COMMANDS, "PI_DLMHeartbeatList - status=0x%X\n", rc);

    /*
     * Attach the return data packet to the main response packet.
     * Fill in the header length and status fields.
     */
    pRspPacket->pPacket = (UINT8 *)pMRPRsp;
    pRspPacket->pHeader->length = sizeof(*pMRPRsp);
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = pMRPRsp->header.status;

    /*
     * Free the MRP request and response packets.
     */
    Free(pMRPReq);

    if (rc != PI_TIMEOUT)
    {
        Free(pMRPRsp);
    }

    return (rc);
}


/**
******************************************************************************
**
**  @brief      To provide a handler function for the resync raids
**              packet interface request (PI_MISC_RESYNC_RAIDS_CMD).
**
**  @param      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
**  @return     INT32 - Packet return status
**
******************************************************************************
**/
INT32 PI_MiscResyncRaids(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    PI_MISC_RESYNC_RAIDS_REQ *pRequest;
    UINT16     *pList;

    dprintf(DPRINTF_DEFAULT, "PI_MiscResyncRAIDS: ENTER\n");

    /*
     * Get a pointer to the actual resync request packet.
     */
    pRequest = (PI_MISC_RESYNC_RAIDS_REQ *)pReqPacket->pPacket;

    /*
     * If there are items in the list to resync, send the request.
     */
    if (pRequest->count > 0)
    {
        /*
         * Allocate memory to hold a copy of the list of raids that
         * require resync.
         */
        pList = MallocSharedWC(pRequest->count * sizeof(UINT16));

        /*
         * Copy the list of raids into the list.
         */
        memcpy(pList, pRequest->raids, (pRequest->count * sizeof(UINT16)));

        /*
         * Request the resync for all the raids in the list.
         *
         * NOTE: This list pointer is passed to the MRP as a PCI address
         * so we need to be careful when freeing it.
         */
        rc = SM_MRResyncWithRetry(MRBLISTRAIDS, pRequest->count, pList);

        /*
         * Since the list is passed to the MRP as a PCI adddress it can't
         * be freed until the MRP completes.  So use the DelayedFree
         * function to make sure it does just that.
         */
        DelayedFree(MRRESYNC, pList);
    }

    /*
     * Attach the return data packet to the main response packet.
     * Fill in the header length and status fields.
     * Since the return code from SetSerialNumber() is different than our
     * normal convention is can't be copied directly into the status field.
     */
    pRspPacket->pPacket = NULL;
    pRspPacket->pHeader->length = 0;
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = PI_GOOD;

    return rc;
}


/**
******************************************************************************
**
**  @brief      To provide a handler function for the put device
**              configuration packet interface request
**              (PI_MISC_PUTDEVCONFIG_CMD).
**
**  @param      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
**  @return     INT32 - Packet return status
**
******************************************************************************
**/
INT32 PI_MiscPutDevConfig(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    PI_MISC_PUTDEVCONFIG_REQ *pRequest;

    pRequest = (PI_MISC_PUTDEVCONFIG_REQ *)pReqPacket->pPacket;

    /*
     * Save the device configuration to flash/NVRAM.
     */
    SaveDeviceConfig(pRequest->count, pRequest->map);

    /*
     * Send the device configuration to the BE.
     */
    rc = SM_PutDevConfig();

    if (rc != PI_GOOD)
    {
        dprintf(DPRINTF_DEFAULT, "PI_MiscPutDevConfig: Failed to put dev config (rc: 0x%x).\n", rc);
    }

    /*
     * Attach the return data packet to the main response packet.
     * Fill in the header length and status fields.
     * Since the return code from SetSerialNumber() is different than our
     * normal convention is can't be copied directly into the status field.
     */
    pRspPacket->pPacket = NULL;
    pRspPacket->pHeader->length = 0;
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = PI_GOOD;

    return rc;
}


/**
******************************************************************************
**
**  @brief      To provide a handler function for the get device
**              configuration packet interface request
**              (PI_MISC_GETDEVCONFIG_CMD).
**
**  @param      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
**  @return     INT32 - Packet return status
**
******************************************************************************
**/
INT32 PI_MiscGetDevConfig(UNUSED XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    PI_MISC_GETDEVCONFIG_RSP *pResponse;
    UINT32      length;
    UINT16      count = 0;
    SES_DEV_INFO_MAP *pDevConfig = NULL;

    LoadDeviceConfig(&count, &pDevConfig, 0);

    length = sizeof(*pResponse) + (count * sizeof(SES_DEV_INFO_MAP));

    pResponse = MallocWC(length);

    pResponse->count = count;

    if (count > 0)
    {
        memcpy(&pResponse->map, pDevConfig, count * sizeof(*pDevConfig));
    }

    Free(pDevConfig);

    /*
     * Attach the return data packet to the main response packet.
     * Fill in the header length and status fields.
     * Since the return code from SetSerialNumber() is different than our
     * normal convention is can't be copied directly into the status field.
     */
    pRspPacket->pPacket = (UINT8 *)pResponse;
    pRspPacket->pHeader->length = length;
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = PI_GOOD;

    return rc;
}


/**
******************************************************************************
**
**  @brief      To provide a handler function for the resync data
**              packet interface request
**              (PI_MISC_RESYNCDATA_CMD).
**
**  @param      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
**  @return     INT32 - Packet return status
**
******************************************************************************
**/
INT32 PI_MiscResyncData(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    UINT32      rspLen;
    MRRESYNCDATA_RSP *pOutPkt = NULL;
    PI_MISC_RESYNCDATA_RSP *pRspPkt = NULL;
    UINT32      numStructs;
    UINT32      structSize = 0;
    UINT32      format = 0;

    dprintf(DPRINTF_PI_COMMANDS, "PI_MiscResyncData - ENTER\n");

    /*
     * Allocate enough space to hold the maximum number of CORs.  We can
     * afford the memory and it saves the overhead of making two calls.
     */
    numStructs = MAX_CORS;

    do
    {
        /*
         * Determine the output packet size based on the number of
         * CORs in the list.
         */
        switch (format)
        {
            case MRDRCORRSP:
                structSize = sizeof(COR);
                break;

            case MRDTLCPYRSP:
                structSize = sizeof(MRCOPYDETAIL_INFO);
                break;

            case MRDRIOSTATUSMAP:
                structSize = sizeof(MRCOPYIOSTATUS_INFO);
                break;
        }
        rspLen = sizeof(*pOutPkt) + (numStructs * structSize);

        /*
         * If an output packet was previously allocated, free it before
         * allocating a new one.
         */
        Free(pOutPkt);

        /*
         * Allocate the memory for the response packet.
         */
        pOutPkt = MallocSharedWC(rspLen);

        /*
         * Send the request to proc.
         */
        rc = PI_ExecMRP(pReqPacket->pPacket, pReqPacket->pHeader->length, MRRESYNCDATA,
                        pOutPkt, rspLen, MRP_STD_TIMEOUT);

        /*
         * Save the number of devices in case we need to make the
         * request again.
         */
        if (rc == PI_ERROR && pOutPkt->header.status == DETOOMUCHDATA)
        {
            /*
             * To get the number of devices make sure we account for
             * both raids and deferred raids.
             */
            numStructs = pOutPkt->strctCnt;
        }
    } while (rc == PI_ERROR && pOutPkt->header.status == DETOOMUCHDATA);

    /*
     * If the MRP was successful copy the data into the response packet
     */
    if (rc == PI_GOOD)
    {
        /*
         * Get the number of cors in the list.
         */
        numStructs = pOutPkt->strctCnt;

        /*
         * Calculate the required response packet length.
         */
        switch (pOutPkt->format)
        {
            case MRDRCORRSP:
                structSize = sizeof(COR);
                break;

            case MRDTLCPYRSP:
                structSize = sizeof(MRCOPYDETAIL_INFO);
                break;

            case MRDRIOSTATUSMAP:
                structSize = sizeof(MRCOPYIOSTATUS_INFO);
                break;

            default:
                structSize = pOutPkt->strctSiz;
        }

        rspLen = sizeof(PI_MISC_RESYNCDATA_RSP) + (numStructs * structSize);

        /*
         * Allocate the response packet.
         */
        pRspPkt = MallocWC(rspLen);

        /*
         * Copy the resync data from the MRP response packet to the
         * PI response packet.
         */
        pRspPkt->count = numStructs;
        pRspPkt->format = pOutPkt->format;
        pRspPkt->size = structSize;

/*
        dprintf(DPRINTF_DEFAULT, "PI_MiscResyncData - pRspPkt->count=%d  pRspPkt->format=%d  pRspPkt->size=%d\n",
                pRspPkt->count, pRspPkt->format, pRspPkt->size);
*/
        memcpy(&pRspPkt->header, &pOutPkt->header, sizeof(MR_HDR_RSP));

        if (numStructs > 0)
        {
            memcpy(pRspPkt->data.data, pOutPkt->data, (numStructs * structSize));
        }

        /*
         * Attach the MRP return data packet to the main response packet.
         * Fill in the header length and status fields.
         */
        pRspPacket->pPacket = (UINT8 *)pRspPkt;
        pRspPacket->pHeader->length = rspLen;
        pRspPacket->pHeader->status = rc;
        pRspPacket->pHeader->errorCode = rc;
    }
    else
    {
        /*
         * Indicate an error condition and no return data in the header.
         */
        pRspPacket->pHeader->length = 0;
        pRspPacket->pHeader->status = rc;
        pRspPacket->pHeader->errorCode = pOutPkt->header.status;
    }

    /*
     * Release the MRP output packet (the output only
     * if the request did not timeout).
     */
    if (rc != PI_TIMEOUT)
    {
        Free(pOutPkt);
    }

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PIGetBackendType
**
** Description: Return the Backend type -- Fabric or loop
**
** Inputs:      none
**
** Returns:
**              PI_BACKEND_TYPE_LOOP       0
**              PI_BACKEND_TYPE_FABRIC     1
**
**--------------------------------------------------------------------------*/

INT32 PI_GetBackendType(UNUSED XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{

    PI_GET_BACKEND_TYPE_RSP *pRsp = NULL;
    INT32       rc = PI_GOOD;
    INT32       errorCode = 0;

    /*
     * Set up response packet
     */
    pRspPacket->pHeader->length = sizeof(*pRsp);
    pRspPacket->pPacket = MallocWC(pRspPacket->pHeader->length);
    pRsp = (PI_GET_BACKEND_TYPE_RSP *)pRspPacket->pPacket;

    pRsp->beType = GetBEFabricMode()? PI_BACKEND_TYPE_FABRIC : PI_BACKEND_TYPE_LOOP;

    dprintf(DPRINTF_PI_COMMANDS, "PIGetBackendType: beType:     %d\n",
            (UINT8)pRsp->beType);

    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = errorCode;

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PI_RegisterClientType
**
** Description: Registers the Client type for the socket connection.
**
** Inputs:      client_type - Type of the client to register.
**
**--------------------------------------------------------------------------*/

INT32 PI_RegisterClientType(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    PI_REGISTER_CLIENT_TYPE_REQ *pRequest = NULL;
    PI_REGISTER_CLIENT_TYPE_RSP *pResponse = NULL;
    UINT8       client_type;
    UINT32      sockfd = pReqPacket->pHeader->socket;
    UINT8       nconn = 0;
    INT32       rc = PI_GOOD;

    dprintf(DPRINTF_DEFAULT, "PI_RegisterClientType - ENTER,socket = %d\n", sockfd);

    pRequest = (PI_REGISTER_CLIENT_TYPE_REQ *)pReqPacket->pPacket;

    client_type = pRequest->type;

    pi_clients_register_client_type(client_type, sockfd);

    nconn = pi_clients_count_client_type(client_type);

    pResponse = MallocWC(sizeof(*pResponse));

    pResponse->type = client_type;
    pResponse->nconn = nconn;

    /*
     * Attach the return data packet to the main response
     * packet. Fill in the header length and status fields.
     */

    pRspPacket->pPacket = (UINT8 *)pResponse;
    pRspPacket->pHeader->length = sizeof(*pResponse);
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = PI_GOOD;

    return rc;
}


/*----------------------------------------------------------------------------
** Function:    PI_QuickBreakPauseResumeMirrorStart
**
** Description: To give start command for the Mirror list on which the
**              operations are taken place.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     INT32    - packet return status
**
**--------------------------------------------------------------------------*/
INT32 PI_QuickBreakPauseResumeMirrorStart(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    MRVDISKCONTROL_REQ *ptrInPkt = NULL;
    MRVDISKCONTROL_RSP *ptrOutPkt = NULL;
    UINT8       count;
    UINT32      timeout = TMO_NONE;
    UINT16      startCmd = 0x4000;

    dprintf(DPRINTF_DEFAULT, "PI_QuickBreakPauseResumeMirrorStart: ENTER\n");

    count = ((PI_QUICK_BREAK_PAUSE_RESUME_MIRROR_START_REQ *)pReqPacket->pPacket)->count;

    ptrInPkt = MallocWC(sizeof(*ptrInPkt));
    ptrOutPkt = MallocSharedWC(sizeof(*ptrOutPkt));

    /*
     * Setup input structure from the input parms.
     */
    ptrInPkt->subtype = MVCXSPECCOPY;   /* Need to test other operations */
    ptrInPkt->svid = (startCmd | count);

    rc = PI_ExecMRP(ptrInPkt, sizeof(*ptrInPkt), MRVDISKCONTROL,
                    ptrOutPkt, sizeof(*ptrOutPkt), timeout);

    if (rc != PI_GOOD)
    {
        dprintf(DPRINTF_DEFAULT, "PI_QuickBreakPauseResumeMirrorStart: Error (rc: 0x%x, status: 0x%x)\n",
                rc, ptrOutPkt->header.status);
    }

    /*
     * Free the allocated memory.
     *
     * Use DelayedFree on the input packet since it contains a PCI address.
     *
     */
    DelayedFree(MRVDISKCONTROL, ptrInPkt);


    /*
     * Attach the return data packet to the main response packet.
     * Fill in the header length and status fields.
     * Since the return code from SetSerialNumber() is different than our
     * normal convention is can't be copied directly into the status field.
     */
    pRspPacket->pPacket = (UINT8 *)ptrOutPkt;
    pRspPacket->pHeader->length = sizeof(*ptrOutPkt);
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = ptrOutPkt->header.status;

    return (rc);
}



/*----------------------------------------------------------------------------
** Function:    PI_QuickBreakPauseResumeMirrorSequence
**
** Description: To give sequence command for each Mirror in the mirror list
**              on which the operations are taken place.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     INT32    - packet return status
**
**--------------------------------------------------------------------------*/
INT32 PI_QuickBreakPauseResumeMirrorSequence(XIO_PACKET *pReqPacket,
                                             XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    MRVDISKCONTROL_REQ *ptrInPkt = NULL;
    MRVDISKCONTROL_RSP *ptrOutPkt = NULL;
    UINT8       subtype;
    UINT16      dvid;
    UINT32      timeout = TMO_NONE;
    UINT16      sequenceCmd = 0x8000;
    INT32       errorCode = 0;

    dprintf(DPRINTF_DEFAULT, "PI_QuickBreakPauseResumeMirrorSequence: ENTER\n");

    subtype = ((PI_QUICK_BREAK_PAUSE_RESUME_MIRROR_SEQUENCE_REQ *)pReqPacket->pPacket)->subtype;

    if ((subtype != MVCXSPECCOPY) && (subtype != MVCPAUSECOPY) &&
        (subtype != MVCRESUMECOPY))
    {
        /*
         * Do not allow other than pause, break, resume mirror
         * subtype operation
         */

        rc = PI_ERROR;
        errorCode = DEINVOPT;
    }

    if (rc == PI_GOOD)
    {
        dvid = ((PI_QUICK_BREAK_PAUSE_RESUME_MIRROR_SEQUENCE_REQ *)pReqPacket->pPacket)->dvid;

        ptrInPkt = MallocWC(sizeof(*ptrInPkt));
        ptrOutPkt = MallocSharedWC(sizeof(*ptrOutPkt));

        /*
         * Setup input structure from the input parms.
         */
        ptrInPkt->subtype = subtype;
        ptrInPkt->svid = sequenceCmd;
        ptrInPkt->dvid = dvid;

        rc = PI_ExecMRP(ptrInPkt, sizeof(*ptrInPkt), MRVDISKCONTROL,
                        ptrOutPkt, sizeof(*ptrOutPkt), timeout);

        if (rc != PI_GOOD)
        {
            dprintf(DPRINTF_DEFAULT, "PI_QuickBreakPauseResumeMirrorSequence: error (rc: 0x%x, status: 0x%x)\n",
                    rc, ptrOutPkt->header.status);
        }

        /*
         * Free the allocated memory.
         *
         * Use DelayedFree on the input packet since it contains a PCI address.
         *
         */
        DelayedFree(MRVDISKCONTROL, ptrInPkt);

        /*
         * Attach the return data packet to the main response packet.
         * Fill in the header length and status fields.
         * Since the return code from SetSerialNumber() is different than our
         * normal convention is can't be copied directly into the status field.
         */
        pRspPacket->pPacket = (UINT8 *)ptrOutPkt;
        pRspPacket->pHeader->length = sizeof(*ptrOutPkt);
        pRspPacket->pHeader->status = rc;
        pRspPacket->pHeader->errorCode = ptrOutPkt->header.status;
    }
    else
    {
        /*
         * Indicate an error condition and no return data in the header.
         */
        pRspPacket->pHeader->length = 0;
        pRspPacket->pHeader->status = rc;
        pRspPacket->pHeader->errorCode = errorCode;
    }
    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PI_QuickBreakPauseResumeMirrorExecute
**
** Description: To give go command for all the mirrors in the mirror list
**              on which the operations are taken place.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     INT32    - packet return status
**
**--------------------------------------------------------------------------*/
INT32 PI_QuickBreakPauseResumeMirrorExecute(XIO_PACKET *pReqPacket,
                                            XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    MRVDISKCONTROL_REQ *ptrInPkt = NULL;
    MRVDISKCONTROL_RSP *ptrOutPkt = NULL;
    UINT8       action = PI_GO;
    UINT32      timeout = TMO_NONE;
    UINT16      executeCmd = 0x4100;

    dprintf(DPRINTF_DEFAULT, "PI_QuickBreakPauseResumeMirrorExecute: ENTER\n");

    action = ((PI_QUICK_BREAK_PAUSE_RESUME_MIRROR_EXECUTE_REQ *)pReqPacket->pPacket)->action;

    ptrInPkt = MallocWC(sizeof(*ptrInPkt));
    ptrOutPkt = MallocSharedWC(sizeof(*ptrOutPkt));

    /*
     * Setup input structure from the input parms.
     */
    ptrInPkt->subtype = MVCXSPECCOPY;
    if (action)
    {
        executeCmd = 0x4100;
    }
    else
    {
        executeCmd = 0x4200;
    }
    ptrInPkt->svid = executeCmd;

    rc = PI_ExecMRP(ptrInPkt, sizeof(*ptrInPkt), MRVDISKCONTROL,
                    ptrOutPkt, sizeof(*ptrOutPkt), timeout);

    if (rc != PI_GOOD)
    {
        dprintf(DPRINTF_DEFAULT, "PI_QuickBreakPauseResumeMirrorExecute: error (rc: 0x%x, status: 0x%x)\n",
                rc, ptrOutPkt->header.status);
    }

    /*
     * Free the allocated memory.
     *
     * Use DelayedFree on the input packet since it contains a PCI address.
     *
     */
    DelayedFree(MRVDISKCONTROL, ptrInPkt);

    /*
     * Attach the return data packet to the main response packet.
     * Fill in the header length and status fields.
     * Since the return code from SetSerialNumber() is different than our
     * normal convention is can't be copied directly into the status field.
     */
    pRspPacket->pPacket = (UINT8 *)ptrOutPkt;
    pRspPacket->pHeader->length = sizeof(*ptrOutPkt);
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = ptrOutPkt->header.status;

    return (rc);
}


/*----------------------------------------------------------------------------
** Function:   PI_BatchSnapshotStart
**
** Description: To give start command for the batch snapshot
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     INT32    - packet return status
**
**--------------------------------------------------------------------------*/
INT32 PI_BatchSnapshotStart(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    MRVDISKCONTROL_REQ *ptrInPkt = NULL;
    MRVDISKCONTROL_RSP *ptrOutPkt = NULL;
    UINT16      count;
    UINT32      timeout = TMO_NONE;
    UINT16      startCmd = 0x4000;

    dprintf(DPRINTF_DEFAULT, "%s: ENTER\n", __FUNCTION__);

    count = ((PI_BATCH_SNAPSHOT_START_REQ *)pReqPacket->pPacket)->count;

    ptrInPkt = MallocWC(sizeof(*ptrInPkt));
    ptrOutPkt = MallocSharedWC(sizeof(*ptrOutPkt));

    /*
     * Setup input structure from the input parms.
     */
    ptrInPkt->subtype = MVCSLINK;       /* Need to test other operations */
    ptrInPkt->svid = (startCmd | count);

    rc = PI_ExecMRP(ptrInPkt, sizeof(*ptrInPkt), MRVDISKCONTROL,
                    ptrOutPkt, sizeof(*ptrOutPkt), timeout);

    if (rc != PI_GOOD)
    {
        dprintf(DPRINTF_DEFAULT, "%s: Error (rc: 0x%x, status: 0x%x)\n",
                __FUNCTION__, rc, ptrOutPkt->header.status);
    }

    /*
     * Free the allocated memory.
     *
     * Use DelayedFree on the input packet since it contains a PCI address.
     *
     */
    DelayedFree(MRVDISKCONTROL, ptrInPkt);

    /*
     * Attach the return data packet to the main response packet.
     * Fill in the header length and status fields.
     * Since the return code from SetSerialNumber() is different than our
     * normal convention is can't be copied directly into the status field.
     */
    pRspPacket->pPacket = (UINT8 *)ptrOutPkt;
    pRspPacket->pHeader->length = sizeof(*ptrOutPkt);
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = ptrOutPkt->header.status;

    return (rc);
}


/*----------------------------------------------------------------------------
** Function:  PI_BatchSnapshotSequence
**
** Description: To give sequence command for each snapshot sequence
**              on which the operations cancel/execute are taken place.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     INT32    - packet return status
**
**--------------------------------------------------------------------------*/
INT32 PI_BatchSnapshotSequence(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    MRVDISKCONTROL_REQ *ptrInPkt = NULL;
    MRVDISKCONTROL_RSP *ptrOutPkt = NULL;
    UINT16      svid;
    UINT16      dvid;
    UINT32      timeout = TMO_NONE;
    UINT16      sequenceCmd = 0x8000;

    dprintf(DPRINTF_DEFAULT, "%s: ENTER\n", __FUNCTION__);

    svid = ((PI_BATCH_SNAPSHOT_SEQUENCE_REQ *)pReqPacket->pPacket)->svid;
    dvid = ((PI_BATCH_SNAPSHOT_SEQUENCE_REQ *)pReqPacket->pPacket)->dvid;

    ptrInPkt = MallocWC(sizeof(*ptrInPkt));
    ptrOutPkt = MallocSharedWC(sizeof(*ptrOutPkt));

    /*
     * Setup input structure from the input parms.
     */
    ptrInPkt->subtype = MVCSLINK;
    ptrInPkt->svid = sequenceCmd | svid;
    ptrInPkt->dvid = dvid;

    rc = PI_ExecMRP(ptrInPkt, sizeof(*ptrInPkt), MRVDISKCONTROL,
                    ptrOutPkt, sizeof(*ptrOutPkt), timeout);

    if (rc != PI_GOOD)
    {
        dprintf(DPRINTF_DEFAULT, "%s: error (rc: 0x%x, status: 0x%x)\n",
                __FUNCTION__, rc, ptrOutPkt->header.status);
    }

    /*
     * Free the allocated memory.
     *
     * Use DelayedFree on the input packet since it contains a PCI address.
     *
     */
    DelayedFree(MRVDISKCONTROL, ptrInPkt);

    /*
     * Attach the return data packet to the main response packet.
     * Fill in the header length and status fields.
     * Since the return code from SetSerialNumber() is different than our
     * normal convention is can't be copied directly into the status field.
     */
    pRspPacket->pPacket = (UINT8 *)ptrOutPkt;
    pRspPacket->pHeader->length = sizeof(*ptrOutPkt);
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = ptrOutPkt->header.status;

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PI_BatchSnapshotExecute
**
** Description: To give go command for all the batch snapshot in the list
**              on which the execution/cacnel takes place.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     INT32    - packet return status
**
**--------------------------------------------------------------------------*/
INT32 PI_BatchSnapshotExecute(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    MRVDISKCONTROL_REQ *ptrInPkt = NULL;
    MRVDISKCONTROL_RSP *ptrOutPkt = NULL;
    UINT8       action = PI_GO;
    UINT32      timeout = TMO_NONE;
    UINT16      executeCmd = 0x4100;

    dprintf(DPRINTF_DEFAULT, "%s: ENTER\n", __FUNCTION__);

    action = ((PI_BATCH_SNAPSHOT_EXECUTE_REQ *)pReqPacket->pPacket)->action;
    ptrInPkt = MallocWC(sizeof(*ptrInPkt));
    ptrOutPkt = MallocSharedWC(sizeof(*ptrOutPkt));

    /*
     * Setup input structure from the input parms.
     */
    ptrInPkt->subtype = MVCSLINK;
    if (action)
    {
        executeCmd = 0x4100;
    }
    else
    {
        executeCmd = 0x4200;
    }
    ptrInPkt->svid = executeCmd;

    rc = PI_ExecMRP(ptrInPkt, sizeof(*ptrInPkt), MRVDISKCONTROL,
                    ptrOutPkt, sizeof(*ptrOutPkt), timeout);

    if (rc != PI_GOOD)
    {
        dprintf(DPRINTF_DEFAULT, "%s: error (rc: 0x%x, status: 0x%x)\n",
                __FUNCTION__, rc, ptrOutPkt->header.status);
    }

    /*
     * Free the allocated memory.
     *
     * Use DelayedFree on the input packet since it contains a PCI address.
     *
     */
    DelayedFree(MRVDISKCONTROL, ptrInPkt);

    /*
     * Attach the return data packet to the main response packet.
     * Fill in the header length and status fields.
     * Since the return code from SetSerialNumber() is different than our
     * normal convention is can't be copied directly into the status field.
     */
    pRspPacket->pPacket = (UINT8 *)ptrOutPkt;
    pRspPacket->pHeader->length = sizeof(*ptrOutPkt);
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = ptrOutPkt->header.status;

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
