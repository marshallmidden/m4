/* $Id: ipc_cmd_dispatcher.c 162911 2014-03-20 22:45:34Z marshall_midden $ */
/**
******************************************************************************
**
**  @file   ipc_cmd_dispatcher.c
**
**  @brief  IPC cmd dispatcher
**
**  Implementation of the IPC packet processing
**
**  Copyright (c) 2001-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#include "ipc_cmd_dispatcher.h"

#include "ccb_flash.h"
#include "CmdLayers.h"
#include "cps_init.h"
#include "ddr.h"
#include "debug_files.h"
#include "EL.h"
#include "fm.h"
#include "ipc_heartbeat.h"
#include "ipc_packets.h"
#include "ipc_session_manager.h"
#include "kernel.h"
#include "LargeArrays.h"
#include "LOG_Defs.h"
#include "misc.h"
#include "MR_Defs.h"
#include "nvram.h"
#include "PacketInterface.h"
#include "PI_Utils.h"
#include "PI_CmdHandlers.h"
#include "PI_ClientPersistent.h"
#include "PktCmdHdl.h"
#include "PortServer.h"
#include "quorum.h"
#include "quorum_utils.h"
#include "rm.h"
#include "RMCmdHdl.h"
#include "serial_num.h"
#include "ses.h"
#include "sm.h"
#include "X1_AsyncEventHandler.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"
#include "Snapshot.h"
#include "CT_history.h"

/*****************************************************************************
** Private defines
*****************************************************************************/
#define LOCAL_IMAGE_DEFAULT_SIZE    0x30000

/*
** Timeout for retrieving a local image.
*/
#define TMO_GET_LCL_IMAGE           30000       /* 30  second timeout */

/*****************************************************************************
** Public variable defintions
*****************************************************************************/
UINT8       gAllDevMissAtOtherDCN = 0;

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static IPC_PACKET *HandleIpcConfigurationUpdate(IPC_PACKET *ptrInPkt);
static IPC_PACKET *HandleIpcTunnel(IPC_PACKET *ptrInPkt);
static IPC_PACKET *HandleIpcPing(IPC_PACKET *ptrInPkt);
static IPC_PACKET *HandleIpcSignal(IPC_PACKET *ptrInPkt);
static IPC_PACKET *HandleIpcBroadcast(IPC_PACKET *ptrInPkt);
static IPC_PACKET *HandleIpcAddController(IPC_PACKET *ptrInPkt);
static IPC_PACKET *HandleIpcPersistentData(IPC_PACKET *ptrPacket);
static IPC_PACKET *HandleIpcPrData(IPC_PACKET *ptrPacket);
static IPC_PACKET *HandleIpcResyncClientCmd(IPC_PACKET *ptrPacket);
static IPC_PACKET *HandleIpcResyncClientRecord(IPC_PACKET *ptrPacket);
static IPC_PACKET *HandleIpcLatestPersistent(IPC_PACKET *ptrPacket);
static UINT16 GetLocalImage(UINT32 *pImageSize, void **pLocalImage);
static LOG_ALL_DEV_MISSING_DAT gDevMissData;
static void HandleAllDevicesMissingTask(TASK_PARMS *parms);

/*****************************************************************************
** Code Start
*****************************************************************************/

/**
******************************************************************************
**
**  @brief  Call the command handler for an incoming IPC packet
**
**  @param  packet - Pointer to the incoming IPC_PACKET packet
**
**  @return Pointer to an IPC_PACKET on success, else NULL
**
** Note:
**      When you add a call to your specfic command handler function you need
**      to understand that your operation prevents any other packets
**      from being processed on this CCB, so don't take too long.
**
**      The header portion of the packet should not have the following fields
**      modified in any handler calls:
**
**      sequenceNumber
**      ccbSerialNumber
**
**      You should only have to modify the command code and length
**
******************************************************************************
**/
IPC_PACKET *IpcCommandDispatcher(IPC_PACKET *packet)
{
    IPC_PACKET  *rc = NULL;
    bool        validPacket = true;

    if (!packet || !packet->header)
    {
        return NULL;
    }

#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s IpcCommandDispatcher using packet of %p\n", __FILE__, __LINE__, __func__, packet);
#endif  /* HISTORY_KEEP */

    if (!CheckPacketCompatibility(packet))
    {
        dprintf(DPRINTF_IPC_COMMAND_DISPATCHER, "%s: Protocol Error (Protocol = %u, Ver = %u)\n",
                __func__, IPC_PROTOCOL_LEVEL, GetPacketVersion(packet));
        validPacket = false;
        rc = packet;

        /*
         * Assert here to get benefits of debugging the problem.
         */
        ccb_assert(validPacket, packet->header->commandCode);
    }
    else
    {
        switch (packet->header->commandCode)
        {
        case PACKET_IPC_CONFIGURATION_UPDATE:
            rc = HandleIpcConfigurationUpdate(packet);
            break;

        case PACKET_IPC_REPORT_CONTROLLER_FAILURE:
            rc = IpcMasterFailureManager(packet);
            break;

        case PACKET_IPC_FLUSH_BE_CACHE:
            rc = SM_IPCFlushBECache(packet);
            break;

        case PACKET_IPC_SET_MIRROR_PARTNER:
            rc = SM_IPCAssignFEMirrorPartner(packet);
            break;

        case PACKET_IPC_CONTINUE_WO_MIRROR:
            rc = SM_IPCContinueWithoutMirrorPartner(packet);
            break;

        case PACKET_IPC_RESCAN_DEVICES:
            rc = SM_IPCRescanDevices(packet);
            break;

        case PACKET_IPC_SET_DLM_HEARTBEAT_LIST:
            rc = SM_IPCSetHeartbeatList(packet);
            break;

        case PACKET_IPC_GET_MIRROR_PARTNER:
            rc = SM_IPCGetMirrorPartner(packet);
            break;

        case PACKET_IPC_FLUSH_COMPLETED:
            rc = SM_IPCFlushCompleted(packet);
            break;

        case PACKET_IPC_COMMAND_STATUS:
            break;

        case PACKET_IPC_ELECT:
        case PACKET_IPC_ELECT_QUORUM:
            rc = EL_PacketReceptionHandler(packet);
            break;

        case PACKET_IPC_HEARTBEAT:
            rc = HandleIpcHeartbeat(packet);
            break;

        case PACKET_IPC_OFFLINE:
            rc = FM_SlaveOffline(packet);
            break;

        case PACKET_IPC_TUNNEL:
            rc = HandleIpcTunnel(packet);
            break;

        case PACKET_IPC_PING:
            rc = HandleIpcPing(packet);
            break;

        case PACKET_IPC_SIGNAL:
            rc = HandleIpcSignal(packet);
            break;

        case PACKET_IPC_BROADCAST:
            rc = HandleIpcBroadcast(packet);
            break;

        case PACKET_IPC_ADD_CONTROLLER:
            rc = HandleIpcAddController(packet);
            break;

        case PACKET_IPC_LED_CHANGE:
            rc = HandleLedControlRequest(packet);
            break;

        case PACKET_IPC_CLIENT_PERSISTENT_DATA_CMD:
            rc = HandleIpcPersistentData(packet);
            break;

        case PACKET_IPC_SETPRES_DATA_CMD:
            rc = HandleIpcPrData(packet);
            break;

        case PACKET_IPC_RESYNC_CLIENT_CMD:
            rc = HandleIpcResyncClientCmd(packet);
            break;

        case PACKET_IPC_RESYNC_CLIENT_RECORD:
            rc = HandleIpcResyncClientRecord(packet);
            break;

        case PACKET_IPC_LATEST_PERSISTENT_DATA:
            rc = HandleIpcLatestPersistent(packet);
            break;

        default:
            dprintf(DPRINTF_IPC_COMMAND_DISPATCHER, "%s: Unknown commandCode (%u)\n",
                    __func__, packet->header->commandCode);

            /* Build up a status packet and return an error */
            /* for now just return the packet you got */
            validPacket = false;
            rc = packet;

            /*
             * Assert here to get benefits of debugging the problem.
             */
            ccb_assert(validPacket, packet->header->commandCode);
            break;
        }
    }                       /* else */

    /*
     * Handle an invalid packet
     */
    if (!validPacket)
    {
        /*
         * Create a log message
         */
        LogMessage(LOG_TYPE_ERROR, "IPC - Invalid command (opcode = 0x%x)",
                   packet->header->commandCode);

        /*
         * Free the data packet and send back an error. Currently these
         * errors are not handled on the other side. However, these errors
         * should only occur if there was a programming error in
         * maintaining compatibility and should be caught during development.
         * A better course of action maybe to close the connection at this
         * point?
         */
        Free(packet->data);
        packet->header->length = 0;
        packet->header->status = PI_ERROR;
        packet->header->errorCode = PI_INVALID_CMD_CODE;
    }

    /*
     * If what rc is different from packet then delete packet
     * we will remove this after we get all the other packet handlers
     * written
     */
    if (rc != packet && packet)
    {
        FreePacket(&packet, __FILE__, __LINE__);
    }

    return rc;
}

/*===========================================================================
** Name:    IPC_PACKET *HandleIpcConfigurationUpdate(IPC_PACKET *ptrInPkt);
** In:      IPC_PACKET *ptrPacket       Pointer to the packet
**                                      that contains the request
** Returns: IPC_PACKET *    Result from this command.  All commands need to
**                          return some type of packet so that the call knows
**                          that the operation worked
**==========================================================================*/

/*
 * Previously this value was 10, with the advent of ISE, it is imperative to
 * to bump the count  to match the BE discovery timings, this change is also holds
 * good even for 4000.
*/
#define MAX_NVRAM_RESTORE_RETRIES 20

static IPC_PACKET *HandleIpcConfigurationUpdate(IPC_PACKET *ptrPacket)
{
    UINT8       status = IPC_COMMAND_SUCCESSFUL;
    UINT16      rc = PI_GOOD;
    UINT8       restoreOption;
    UINT32      reason;
    IPC_LOCAL_IMAGE *ptrPacketData = NULL;
    UINT32      imageSize = 0;
    void       *pLocalImage = NULL;

    ccb_assert(ptrPacket != NULL, ptrPacket);
    ccb_assert(ptrPacket->data != NULL, ptrPacket->data);

//    LogMessage(LOG_TYPE_DEBUG, "HICU: Configuration update starting (Option: 0x%x)",
//               ptrPacket->data->configurationUpdate.restoreOption);

    /*
     * Get the data associated with the input packet and then free the
     * data to reuse the pointer for the response packet
     */
    restoreOption = ptrPacket->data->configurationUpdate.restoreOption;
    reason = ptrPacket->data->configurationUpdate.reason;

    /*
     * Restore the processor NVRAM to pick up the latest configuration.
     */
    if (SM_NVRAMRestoreWithRetries(restoreOption, NULL, MAX_NVRAM_RESTORE_RETRIES) != PI_GOOD)
    {
        status = IPC_COMMAND_FAILED;
    }

    /*
     * If this configuration update is not a refresh we want to make
     * sure we load the master configuration and the controller map.
     */
    if ((restoreOption & MRNOREFRESH) == 0)
    {
        /*
         * Attempt to read the new master configuration from disk
         */
        if (status == IPC_COMMAND_SUCCESSFUL && (reason & X1_ASYNC_VCG_CFG_CHANGED))
        {
            dprintf(DPRINTF_IPC_COMMAND_DISPATCHER, "HandleIpcConfigurationUpdate: Loading master configuration.\n");

            /*
             * Load the master configuraiton, if the load fails we will
             * need to make sure the controller gets failed and if the
             * load is successful we need to save the configuration to
             * the controllers NVRAM.
             */
            if (LoadMasterConfiguration() != 0)
            {
                LogMessage(LOG_TYPE_DEBUG, "Configuration update - Failed to load Master Configuration");

                status = IPC_COMMAND_FAILED;
            }
            else
            {
                StoreMasterConfigToNVRAM();
            }
        }

        /*
         * Attempt to read the new controller map from disk
         */
        if (status == IPC_COMMAND_SUCCESSFUL && (reason & X1_ASYNC_VCG_CFG_CHANGED))
        {
            dprintf(DPRINTF_IPC_COMMAND_DISPATCHER, "HandleIpcConfigurationUpdate: Loading controller map\n");

            /*
             * Load the controller map, if the load fails we will
             * need to make sure the controller gets failed.
             */
            if (LoadControllerMap() != 0)
            {
                LogMessage(LOG_TYPE_DEBUG, "Configuration update - Failed to load controller map");

                status = IPC_COMMAND_FAILED;
            }
        }

        if (status == IPC_COMMAND_SUCCESSFUL)
        {
            /*
             * Make the call to get the local image from the BEP.  The GetLocalImage
             * function will allocate a buffer for the local image and if the MRP is
             * executed successfully the image will be returned to this function
             * through the pLocalImage parameter.  If the return value is anything
             * but good the image should not be returned.
             */
            rc = GetLocalImage(&imageSize, &pLocalImage);

            if (rc != PI_GOOD)
            {
                ccb_assert(pLocalImage == NULL, pLocalImage);

                /*
                 * Since the call to get the local image from the BEP failed we
                 * will not be returning an image.  Therefore the length of the
                 * data to be returned is just the size of the local image
                 * structure (defined not to contain any image data).
                 */
                ptrPacket->header->length = sizeof(IPC_LOCAL_IMAGE);
                status = IPC_COMMAND_FAILED;
            }
            else
            {
                ccb_assert(pLocalImage != NULL, pLocalImage);
                DumpLocalImage("HandleIpcConfigurationUpdate", imageSize, pLocalImage);

                /*
                 * The IPC_LOCAL_IMAGE structre is defined to contain some data.
                 * We setup a dummy array of items there that essentially represent
                 * a buffer when finished.  For this reason when calculating the
                 * size of the packet to allocate to return we take the size of
                 * the local image, add the actual image size returned from the
                 * PROC.
                 */
                ptrPacket->header->length = sizeof(IPC_LOCAL_IMAGE) + imageSize;
                status = IPC_COMMAND_SUCCESSFUL;
            }
        }
        else
        {
            /*
             * Since the call to get the local image from the BEP failed we
             * will not be returning an image.  Therefore the length of the
             * data to be returned is just the size of the local image
             * structure (defined not to contain any image data).
             */
            ptrPacket->header->length = sizeof(IPC_LOCAL_IMAGE);
        }
    }
    else
    {
        /*
         * Since the call to get the local image from the BEP failed we
         * will not be returning an image.  Therefore the length of the
         * data to be returned is just the size of the local image
         * structure (defined not to contain any image data).
         */
        ptrPacket->header->length = sizeof(IPC_LOCAL_IMAGE);
    }

    /*
     * Allocate the return packet data since we want to be able to return
     * the normal packet plus the local image that we just retrieved.
     */
    ptrPacketData = MallocWC(ptrPacket->header->length);

    /*
     * Make sure we save the status and image size in the new return
     * data packet.
     */
    ptrPacketData->status = status;
    ptrPacketData->imageSize = imageSize;

    /*
     * If the image size is greater than zero, we have an image to return
     * to the caller.  That image is in the pLocalImage buffer, so copy
     * it into the return packet data.
     */
    if (imageSize > 0)
    {
        ccb_assert(pLocalImage != NULL, pLocalImage);
        memcpy((void *)ptrPacketData->image, pLocalImage, imageSize);
    }

    /*
     * Well we want to return our new data packet so free the old one
     * and replace it with our created data packet that contains the
     * local image.
     */
    Free(ptrPacket->data);
    ptrPacket->data = (IPC_PACKET_DATA *)ptrPacketData;

    /*
     * If the local image was allocated, make sure we deallocate it here.  The
     * GetLocalImage function will allocate the buffer and return it to this
     * function if the MRP was executed successfully.
     */
    Free(pLocalImage);

    /*
     * Send a notification of the configuration change to the XIOservice.
     * Notify only on the restore (not on the refresh)
     */
    if ((restoreOption & MRNOREFRESH) == 0)
    {
        SendX1ChangeEvent(reason);
    }

//    LogMessage(LOG_TYPE_DEBUG, "HICU - Configuration update complete (Option: 0x%x)", restoreOption);

    return (ptrPacket);
}

/*===========================================================================
** Name:    IPC_PACKET *HandleIpcTunnel(IPC_PACKET *ptrInPkt);
** In:      IPC_PACKET *ptrPacket       Pointer to the packet
**                                      that contains the request
** Returns: IPC_PACKET *    Result from this command.  All commands need to
**                          return some type of packet so that the call knows
**                          that the operation worked
**==========================================================================*/
static IPC_PACKET *HandleIpcTunnel(IPC_PACKET *ptrPacket)
{
    IPC_TUNNEL *pTunnel = NULL;
    XIO_PACKET  reqPacket = { NULL, NULL };
    XIO_PACKET  rspPacket = { NULL, NULL };
    size_t      responseSize = 0;
    UINT32      rc = PI_GOOD;
    UINT8       status = IPC_COMMAND_SUCCESSFUL;

    ccb_assert(ptrPacket != NULL, ptrPacket);
    ccb_assert(ptrPacket->data != NULL, ptrPacket->data);

    pTunnel = (IPC_TUNNEL *)ptrPacket->data;

    reqPacket.pHeader = (PI_PACKET_HEADER *)pTunnel->packet;

    if (reqPacket.pHeader->length != 0)
    {
        reqPacket.pPacket = (UINT8 *)(pTunnel->packet + sizeof(PI_PACKET_HEADER));
    }

    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    rspPacket.pHeader->packetVersion = reqPacket.pHeader->packetVersion;

    dprintf(DPRINTF_IPC_COMMAND_DISPATCHER, "HandleIpcTunnel - Process tunnel packet - start (cmdCode: 0x%x, len: 0x%x).\n",
            reqPacket.pHeader->commandCode, reqPacket.pHeader->length);

    /*
     * Issue the command through the packet command handler
     */
    rc = PortServerCommandHandler(&reqPacket, &rspPacket);

    status = (rc == PI_GOOD ? IPC_COMMAND_SUCCESSFUL : IPC_COMMAND_FAILED);

    dprintf(DPRINTF_IPC_COMMAND_DISPATCHER, "HandleIpcTunnel - Process tunnel packet - complete (cmdCode: 0x%x, status: 0x%x, errorCode: 0x%x).\n",
            reqPacket.pHeader->commandCode,
            rspPacket.pHeader->status, rspPacket.pHeader->errorCode);

    /*
     * Calculate the response data packet size.  This needs to include
     * the PI packet header and data.
     */
    responseSize = sizeof(IPC_TUNNEL) +
        sizeof(*rspPacket.pHeader) + rspPacket.pHeader->length;

    /*
     * Free request packet and allocate data for the response packet
     */
    Free(ptrPacket->data);
    ptrPacket->data = MallocSharedWC(responseSize);

    /*
     * Fill in the length of the response packet and the response data.
     */
    ptrPacket->header->length = responseSize;

    pTunnel = (IPC_TUNNEL *)ptrPacket->data;
    pTunnel->status = status;

    /* Copy the PI header data into the start of the response data packet. */
    memcpy(pTunnel->packet, rspPacket.pHeader, sizeof(*rspPacket.pHeader));

    /* Copy the PI packet data into the response packet after the header */
    if (rspPacket.pHeader->length > 0)
    {
        memcpy(pTunnel->packet + sizeof(*rspPacket.pHeader),
               rspPacket.pPacket, rspPacket.pHeader->length);
    }

    /*
     * If this was a server create command that was tunneled we need
     * to make sure that the slaves know that we have just created
     * a server.
     */
    if (rc == PI_GOOD && reqPacket.pHeader->commandCode == PI_SERVER_CREATE_CMD)
    {
        /*
         * Only if the server did not previously exists do we want
         * to send the configuration update to the slave controllers.
         */
        if (((PI_SERVER_CREATE_RSP *)(rspPacket.pPacket))->flags == false)
        {
            RMSlavesConfigurationUpdate(X1_ASYNC_ZCHANGED, TRUE);
        }
    }

    /*
     * Free the allocated memory
     */
    Free(rspPacket.pHeader);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPacket.pPacket);
    }

    return (ptrPacket);
}

/*===========================================================================
** Name:    IPC_PACKET *HandleIpcPing(IPC_PACKET *ptrInPkt);
** In:      IPC_PACKET *ptrPacket       Pointer to the packet
**                                      that contains the request
** Returns: IPC_PACKET *    Result from this command.  All commands need to
**                          return some type of packet so that the call knows
**                          that the operation worked
**==========================================================================*/
static IPC_PACKET *HandleIpcPing(IPC_PACKET *ptrInPkt)
{
    dprintf(DPRINTF_IPC_COMMAND_DISPATCHER, "HandleIpcPing...ENTER and EXIT\n");

    return (ptrInPkt);
}

/*===========================================================================
** Name:    IPC_PACKET *HandleIpcSignal(IPC_PACKET *ptrInPkt);
** In:      IPC_PACKET *ptrPacket       Pointer to the packet
**                                      that contains the request
** Returns: IPC_PACKET *    Result from this command.  All commands need to
**                          return some type of packet so that the call knows
**                          that the operation worked
**==========================================================================*/
static IPC_PACKET *HandleIpcSignal(IPC_PACKET *ptrPacket)
{
    UINT16      signalEvent;

    ccb_assert(ptrPacket != NULL, ptrPacket);
    ccb_assert(ptrPacket->data != NULL, ptrPacket);

    signalEvent = ((IPC_SIGNAL *)ptrPacket->data)->signalEvent;

    if (signalEvent & IPC_SIGNAL_RUN_BE)
    {
        dprintf(DPRINTF_DEFAULT, "HandleIpcSignal-Signal BE to run.\n");
        CPSInitSlaveController(SLAVE_INIT_RUN_BE);
    }

    if (signalEvent & IPC_SIGNAL_RUN_P2INIT)
    {
        /*
         * Signal the BE to go to P2INIT (this starts the FE also).
         */
        LogMessage(LOG_TYPE_DEBUG, "POWERUP-Signal BE to go to P2INIT");
        MRP_Awake(MRAWAKE_SLAVE | MRAWAKE_P2INIT);
    }

    if (signalEvent & IPC_SIGNAL_RUN_FE)
    {
        /*
         * Signal the BE to go to P2INIT (this starts the FE also).
         */
        LogMessage(LOG_TYPE_DEBUG, "POWERUP-Signal FE to run");
        MRP_Awake(MRAWAKE_SLAVE | MRAWAKE_FEINIT);

        LogMessage(LOG_TYPE_DEBUG, "HandleIpcSignal-Signal the FE to start cache initialization.\n");

        CPSInitWaitForCacheInitTaskStart();
    }

    if (signalEvent & IPC_SIGNAL_LOAD_CONFIG)
    {
        dprintf(DPRINTF_DEFAULT, "HandleIpcSignal-Loading master config.\n");

        /*
         * Load the master configuraiton, if the load fails we will
         * need to make sure the controller gets failed and if the
         * load is successful we need to save the configuration to
         * the controllers NVRAM.
         */
        if (LoadMasterConfiguration() != 0)
        {
            LogMessage(LOG_TYPE_DEBUG, "HandleIpcSignal-Failed to load Master Config.");
        }
        else
        {
            StoreMasterConfigToNVRAM();
        }

        dprintf(DPRINTF_DEFAULT, "HandleIpcSignal-Loading controller map.\n");

        /*
         * Load the controller map, if the load fails we will
         * need to make sure the controller gets failed.
         */
        if (LoadControllerMap() != 0)
        {
            LogMessage(LOG_TYPE_DEBUG, "HandleIpcSignal-Failed to load controller map.");
        }
    }

    if (signalEvent & IPC_SIGNAL_POWERUP_BE_READY)
    {
        dprintf(DPRINTF_DEFAULT, "HandleIpcSignal-Power-up BE ready.\n");

        /*
         * Set the power-up state indicating all controllers have made
         * made it to power-up BE ready.
         */
        SetPowerUpState(POWER_UP_ALL_CTRL_BE_READY);
    }

    if (signalEvent & IPC_SIGNAL_POWERUP_COMPLETE)
    {
        dprintf(DPRINTF_DEFAULT, "HandleIpcSignal-Power-up complete.\n");

        /*
         * Make sure that we set the power-up complete state to true since
         * this is the last step in the power-up of a slave controller.
         */
        SetPowerUpState(POWER_UP_COMPLETE);
    }

    if (signalEvent & IPC_SIGNAL_MIRROR_PARTNER_INFO)
    {
        MRP_NegotiateMPInfo();
    }

    if (signalEvent & IPC_SIGNAL_FE_PORT_GO)
    {
        dprintf(DPRINTF_DEFAULT, "HandleIpcSignal-Signal the FE regular ports.\n");

        /*
         * We have completed the cache initialization but we still want
         * to make sure the power-up state is set correctly.  This setting
         * is used to make sure we remove the WAIT_CACHE_ERROR state if
         * the cache initialiazed without user input (don't know if that
         * is possible but code for it).
         */
        SetPowerUpState(POWER_UP_PROCESS_CACHE_INIT);
        SetPowerUpAStatus(POWER_UP_ASTATUS_UNKNOWN);

        /*
         * Signal the FE to put the regular ports on the cards.
         */
        MRP_FEPortGo();
    }

    return (ptrPacket);
}

/*===========================================================================
** Name:    IPC_PACKET *HandleIpcBroadcast(IPC_PACKET *ptrInPkt);
** In:      IPC_PACKET *ptrPacket       Pointer to the packet
**                                      that contains the request
** Returns: IPC_PACKET *    Result from this command.  All commands need to
**                          return some type of packet so that the call knows
**                          that the operation worked
**==========================================================================*/
static IPC_PACKET *HandleIpcBroadcast(IPC_PACKET *ptrPacket)
{
    UINT8       status = IPC_COMMAND_SUCCESSFUL;
    UINT32      commandCode = 0;
    UINT16      rc = PI_GOOD;
    MR_GENERIC_RSP *ptrOutPkt = NULL;
    IPC_BROADCAST *pBroadcast = NULL;
    bool        bConfigMutexLocked = false;
    bool        bTempDisableCache = false;

    ccb_assert(ptrPacket != NULL, ptrPacket);
    ccb_assert(ptrPacket->data != NULL, ptrPacket->data);

    /*
     * Cast the packet pointer to a broadcast pointer to make
     * access easier.
     */
    pBroadcast = (IPC_BROADCAST *)ptrPacket->data;

//    LogMessage(LOG_TYPE_DEBUG, "HIB: Processing broadcast event (et: 0x%x, bt: 0x%x, sn: 0x%x, ds: 0x%x)",
//               pBroadcast->eventType,
//               pBroadcast->bcastType, pBroadcast->serialNum, pBroadcast->dataSize);

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
        /*
         * Wait until RM has completed running before attempting
         * to process the local image.
         */
        while (RMGetState() != RMRUNNING && RMGetState() != RMDOWN)
        {
            TaskSleepMS(500);
        }

        /*
         * Check if this local image should still be processed.  If
         * a configuration update has taken place this local image
         * may be considered old and should not be processed.
         */
        if (LocalImageStillProcess(pBroadcast->data))
        {
//            LogMessage(LOG_TYPE_DEBUG, "LCLIMAGE HIB-Seq: 0x%x, Len: 0x%x, Ctrl: 0x%x, MP: 0x%x",
//                       ((UINT32 *)pBroadcast->data)[3],
//                       ((UINT32 *)pBroadcast->data)[0],
//                       ((UINT32 *)pBroadcast->data)[1], ((UINT32 *)pBroadcast->data)[2]);

            DumpLocalImage("HandleIpcBroadcast", pBroadcast->dataSize, pBroadcast->data);

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
//        else
//        {
//            LogMessage(LOG_TYPE_DEBUG, "LCLIMAGE HIBTRW-Seq: 0x%x, Len: 0x%x, Ctrl: 0x%x, MP: 0x%x",
//                       ((UINT32 *)pBroadcast->data)[3],
//                       ((UINT32 *)pBroadcast->data)[0],
//                       ((UINT32 *)pBroadcast->data)[1], ((UINT32 *)pBroadcast->data)[2]);
//        }
    }
    else if (pBroadcast->eventType == IPC_BROADCAST_PUTSCMT &&
             pBroadcast->bcastType == IPC_BROADCAST_MASTER &&
             COPY_HAND_CTYPE(pBroadcast) == COPY_HAND_SWAP &&
             COPY_HAND_PERCENT(pBroadcast) == 100)
    {
        IPC_BROADCAST *pNewBroadcast = NULL;
        UINT32      bcastSize = sizeof(*pNewBroadcast) + pBroadcast->dataSize;

        dprintf(DPRINTF_IPC_COMMAND_DISPATCHER, "HandleIpcBroadcast: Special PUTSCMT (1-100)\n");

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

        /* On copy/swap finish, do a journalling snapshot -- to allow recovery after vdisk delete. */
        DelayedSnapshot(SNAPSHOT_DEFAULT_DELAY_TIME);

        /*
         * Aquire the mutex for the configuration update and hold until the
         * request has been fulfilled.
         */
        (void)LockMutex(&configUpdateMutex, MUTEX_WAIT);
        bConfigMutexLocked = true;

        AsyncEventBroadcast(pNewBroadcast);
    }
    else if (pBroadcast->eventType == IPC_BROADCAST_ALLDEVMISSING)
    {
        {
            memcpy((void *)&gDevMissData, &pBroadcast->data, pBroadcast->dataSize);
            LogMessage(LOG_TYPE_DEBUG, "<GR>ALL DEV MISS Recv from (%x)- Write Failure Data\n",
                       gDevMissData.cSerial);
            WriteFailureDataState(gDevMissData.cSerial, FD_STATE_FAILED);
            LogMessage(LOG_TYPE_DEBUG, "<GR>Forcing Election to save from double fault\n");
            (void)EL_DoElectionNonBlocking();
            gAllDevMissAtOtherDCN = 1;
            TaskCreate(HandleAllDevicesMissingTask, NULL);
        }
    }
    else
    {
        ptrOutPkt = MallocSharedWC(sizeof(*ptrOutPkt));

        DumpLocalImage("HandleIpcBroadcast", pBroadcast->dataSize, pBroadcast->data);

        /*
         * Send the request to Thunderbolt.  This function handles timeout
         * conditions and task switches while waiting.
         */
        rc = PI_ExecMRP(pBroadcast->data, pBroadcast->dataSize, commandCode,
                        ptrOutPkt, sizeof(*ptrOutPkt), GetGlobalMRPTimeout());

        if (rc != PI_GOOD)
        {
            LogMessage(LOG_TYPE_DEBUG, "HIB: Failed execute the broadcast (cmdCode: 0x%x, rc: 0x%x, status: 0x%x)",
                       commandCode, rc, ptrOutPkt->header.status);
        }
    }

    /*
     * If something failed above, make sure the status for the
     * IPC commnand is set to FAILED.
     */
    if (rc != PI_GOOD)
    {
        status = IPC_COMMAND_FAILED;
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
             * Send the slaves a configuration update IPC packet.
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
            LogMessage(LOG_TYPE_DEBUG, "HIB: Put LDD complete from controller (0x%x), update configuration",
                       ptrPacket->header->ccbSerialNumber);

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
            (void)RMTempDisableCache(PI_MISC_CLRTDISCACHE_CMD,
                                     TEMP_DISABLE_BROADCAST, T_DISABLE_CLEAR_ONE);
        }
    }

    /*
     * Free the allocated memory, this was only allocated for events
     * other than local images but since the free takes into account
     * NULL pointers we can just make the call.
     */
    if (rc != PI_TIMEOUT)
    {
        Free(ptrOutPkt);
    }

    /*
     * Free request packet and allocate data for the response packet
     *
     * Use delayed free on the data portion of the IPC packet since
     * it was used directly in the MRP.
     */
    DelayedFree(commandCode, ptrPacket->data);
    ptrPacket->data = MallocSharedWC(sizeof(IPC_COMMAND_STATUS));

    /*
     * Fill in the length of the response packet and the response data.
     */
    ptrPacket->header->length = sizeof(IPC_COMMAND_STATUS);
    ((IPC_COMMAND_STATUS *)(ptrPacket->data))->status = status;

    return (ptrPacket);
}

/*===========================================================================
** Name:    IPC_PACKET *HandleIpcAddController(IPC_PACKET *ptrInPkt);
** In:      IPC_PACKET *ptrPacket       Pointer to the packet
**                                      that contains the request
** Returns: IPC_PACKET *    Result from this command.  All commands need to
**                          return some type of packet so that the call knows
**                          that the operation worked
**==========================================================================*/
static IPC_PACKET *HandleIpcAddController(IPC_PACKET *ptrPacket)
{
    UINT32      cserial;
    UINT8       status = IPC_COMMAND_SUCCESSFUL;

    dprintf(DPRINTF_IPC_COMMAND_DISPATCHER, "HandleIpcAddController - ENTER\n");

    ccb_assert(ptrPacket != NULL, ptrPacket);
    ccb_assert(ptrPacket->data != NULL, ptrPacket->data);

    /*
     * Get the data associated with the input packet and then free the
     * data to reuse the pointer for the response packet
     */
    cserial = ptrPacket->data->addController.cserial;

    if (cserial != CntlSetup_GetControllerSN())
    {
        dprintf(DPRINTF_IPC_COMMAND_DISPATCHER, "HandleIpcAddController - Invalid serial number (0x%x, 0x%x).\n",
                CntlSetup_GetControllerSN(), cserial);

        status = IPC_COMMAND_FAILED;
    }
    else
    {
        /*
         * Set the flag indicating that a license has been applied.
         */
        SetLicenseApplied();
    }

    /*
     * Free request packet and allocate data for the response packet
     */
    Free(ptrPacket->data);
    ptrPacket->data = MallocSharedWC(sizeof(IPC_COMMAND_STATUS));

    /*
     * Fill in the length of the response packet and the response data.
     */
    ptrPacket->header->length = sizeof(IPC_COMMAND_STATUS);
    ptrPacket->data->commandStatus.status = status;

    dprintf(DPRINTF_IPC_COMMAND_DISPATCHER, "HandleIpcAddController - EXIT\n");

    return (ptrPacket);
}


/*----------------------------------------------------------------------------
** Function:    GetLocalImage
**
** Description: Retrieves a local NVRAM image from the BEP.
**
** Inputs:      pImageSize      - pointer to a UINT32 to put the size of the
**                                image retrieved from the BEP.
**              ppLocalImage    - pointer to a pointer to return the buffer
**                                that the MRP fills in with the local image.
**
** Returns:     PI_GOOD, PI_ERROR, PI_MALLOC_ERROR or PI_TIMEOUT
**
**--------------------------------------------------------------------------*/
static UINT16 GetLocalImage(UINT32 *pImageSize, void **ppLocalImage)
{
    MRGETLCLIMAGE_REQ *ptrInPkt;
    MRGETLCLIMAGE_RSP *ptrOutPkt;
    UINT16      rc = PI_GOOD;
    void       *pLocalImage = NULL;
    UINT32      imageSize = LOCAL_IMAGE_DEFAULT_SIZE;

    ccb_assert(pImageSize != NULL, pImageSize);
    ccb_assert(ppLocalImage != NULL, ppLocalImage);

    ptrInPkt = MallocWC(sizeof(*ptrInPkt));
    ptrOutPkt = MallocSharedWC(sizeof(*ptrOutPkt));

    do
    {
        /*
         * If an output packet was previously allocated, free it before
         * allocating a new one.
         */
        if (pLocalImage != NULL)
        {
            Free(pLocalImage);
        }

        /*
         * Allocate the new local image buffer
         */
        pLocalImage = MallocSharedWC(imageSize);

        ptrInPkt->length = imageSize;
        ptrInPkt->address = pLocalImage;

        rc = PI_ExecMRP(ptrInPkt, sizeof(*ptrInPkt), MRGETLCLIMAGE,
                        ptrOutPkt, sizeof(*ptrOutPkt),
                        MAX(GetGlobalMRPTimeout(), TMO_GET_LCL_IMAGE));

        /*
         * If the request was not an error or the error status is something
         * other than DETOOMUCHDATA, exit out of the loop.
         */
        if (rc != PI_ERROR || ptrOutPkt->header.status != DETOOMUCHDATA)
        {
            break;
        }

        /*
         * Save the image size in case we need to make the request again.
         */
        imageSize = ptrOutPkt->imageSize;

    } while (FOREVER);

    /*
     * If the MRP executed successfully then save the image size and the
     * local image buffer address in the output parameters.
     */
    if (rc == PI_GOOD)
    {
        /*
         * dprintf(DPRINTF_IPC_COMMAND_DISPATCHER, "GetLocalImage - Image Size: 0x%x\n",
         *         ptrOutPkt->imageSize);
         */

        *pImageSize = ptrOutPkt->imageSize;
        *ppLocalImage = pLocalImage;
    }
    else
    {
        dprintf(DPRINTF_IPC_COMMAND_DISPATCHER, "GetLocalImage - ERROR: Failed to get the local image from BEP (rc: 0x%x, Status: 0x%x)\n",
                rc, ptrOutPkt->header.status);

        *pImageSize = 0;
        *ppLocalImage = NULL;
    }

    /*
     * Only free the memory if the request did NOT timeout.  On a timeout
     * the memory must remain available in case the request eventually
     * completes.
     */
    if (rc == PI_TIMEOUT)
    {
        DelayedFree(MRGETLCLIMAGE, pLocalImage);
    }
    else
    {
        Free(ptrOutPkt);
    }

    /*
     * Free the memory that was allocated.
     *
     * Use delayed free on the input packet since it contains
     * a PCI address.
     */
    DelayedFree(MRGETLCLIMAGE, ptrInPkt);

    return (rc);
}


/**
******************************************************************************
**
**  @brief      Sends the FE Port GO MRP to the FEP.
**
**  @param      none
**
**  @return     PI_GOOD or one of the MRP error codes (PI_ERROR, etc.).
**
******************************************************************************
**/
UINT16 MRP_FEPortGo(void)
{
    MRFEPORTGO_RSP *pRsp;
    UINT16      rc;

    LogMessage(LOG_TYPE_DEBUG, "MRP_FEPortGo - ENTER");

    /*
     * Allocate memory for the MRP input and output packets.
     */
    pRsp = MallocSharedWC(sizeof(*pRsp));

    /*
     * Send the request to Thunderbolt.  This function handles timeout
     * conditions and task switches while waiting.
     */
    rc = PI_ExecMRP(NULL, 0, MRFEPORTGO, pRsp, sizeof(*pRsp), GetGlobalMRPTimeout());

    /*
     * Free the allocated memory.
     */
    if (rc != PI_TIMEOUT)
    {
        Free(pRsp);
    }

    LogMessage(LOG_TYPE_DEBUG, "MRP_FEPortGo - EXIT (0x%x)", rc);

    return (rc);
}

/**
******************************************************************************
**
**  @brief      Sends the mirror partner information MRP
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void MRP_NegotiateMPInfo(void)
{
    UINT32      mpSerialNo;
    UINT8       retVal = FALSE;
    UINT8       rc = FALSE;

    /*
     * Get the Mirror Partner Serial Number
     */
    mpSerialNo = GetCachedMirrorPartnerSN();

    rc = StopIO(CntlSetup_GetControllerSN(),
                SM_STOP_IO_OP, SM_STOP_IO_INTENT, START_STOP_IO_USER_CCB_SM, 0);

    if (rc != PI_GOOD)
    {
        LogMessage(LOG_TYPE_DEBUG, "POWERUP-Failed to stop IO");
        CPSInitWaitForever();
    }

    /*
     *
     */
    retVal = AssignMirrorPartnerMRP(mpSerialNo, NULL, NULL);

    rc = StartIO(CntlSetup_GetControllerSN(),
                 START_IO_OPTION_CLEAR_ONE_STOP_COUNT, START_STOP_IO_USER_CCB_SM, 0);

    if (rc != PI_GOOD)
    {
        LogMessage(LOG_TYPE_DEBUG, "POWERUP-Failed to start IO");
        CPSInitWaitForever();
    }

    if (retVal)
    {
        fprintf(stderr, "<MIRROR_PARTNER> Able to assign the mirror partner SNo\n");
    }
    else
    {
        fprintf(stderr, "<MIRROR_PARTNER> Unable to assign the mirror partner SNo\n");
    }
    fprintf(stderr, "<MIRROR_PARTNER> Exiting the MRP_NegotiateMPInfo\n");
}


/*===========================================================================
** Name:    IPC_PACKET *HandleIpcPersistentData(IPC_PACKET *ptrInPkt);
** In:      IPC_PACKET *ptrPacket       Pointer to the packet
**                                      that contains the request
** Returns: IPC_PACKET *    Result from this command.  All commands need to
**                          return some type of packet so that the call knows
**                          that the operation worked
**==========================================================================*/
static IPC_PACKET *HandleIpcPersistentData(IPC_PACKET *ptrPacket)
{
    IPC_CLIENT_PERSISTENT_DATA *pPersistent = NULL;
    XIO_PACKET  reqPacket = { NULL, NULL };
    XIO_PACKET  rspPacket = { NULL, NULL };
    size_t      responseSize = 0;
    UINT32      rc = PI_GOOD;
    UINT8       status = IPC_COMMAND_SUCCESSFUL;

    ccb_assert(ptrPacket != NULL, ptrPacket);
    ccb_assert(ptrPacket->data != NULL, ptrPacket->data);

    pPersistent = (IPC_CLIENT_PERSISTENT_DATA *)ptrPacket->data;
    reqPacket.pHeader = (PI_PACKET_HEADER *)pPersistent->packet;

    if (reqPacket.pHeader->length != 0)
    {
        reqPacket.pPacket = (UINT8 *)(pPersistent->packet + sizeof(PI_PACKET_HEADER));
    }

    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    rspPacket.pHeader->packetVersion = reqPacket.pHeader->packetVersion;

    dprintf(DPRINTF_DEFAULT, "HandleIpcPersistentData - Process persistentData packet - start (cmdCode: 0x%x, len: 0x%x).\n",
            reqPacket.pHeader->commandCode, reqPacket.pHeader->length);

    /*
     * Issue the command through PI_ClientPersistentData
     */
    rc = PI_ClientPersistentDataControl(&reqPacket, &rspPacket);

    status = (rc == PI_GOOD ? IPC_COMMAND_SUCCESSFUL : IPC_COMMAND_FAILED);

    /*
     * Calculate the response data packet size.  This needs to include
     * the PI packet header and data.
     */
    responseSize = sizeof(IPC_CLIENT_PERSISTENT_DATA) +
        sizeof(*rspPacket.pHeader) + rspPacket.pHeader->length;

    /*
     * Free request packet and allocate data for the response packet
     */
    Free(ptrPacket->data);
    ptrPacket->data = MallocSharedWC(responseSize);

    /*
     * Fill in the length of the response packet and the response data.
     */
    ptrPacket->header->length = responseSize;

    pPersistent = (IPC_CLIENT_PERSISTENT_DATA *)ptrPacket->data;

    /* Copy the PI header data into the start of the response data packet. */
    memcpy(pPersistent->packet, rspPacket.pHeader, sizeof(*rspPacket.pHeader));

    /* Copy the PI packet data into the response packet after the header */
    if (rspPacket.pHeader->length > 0)
    {
        memcpy(pPersistent->packet + sizeof(*rspPacket.pHeader),
               rspPacket.pPacket, rspPacket.pHeader->length);
    }

    /*
     * Free the allocated memory
     */
    Free(rspPacket.pHeader);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPacket.pPacket);
    }

    return (ptrPacket);

}


/*===========================================================================
** Name:    IPC_PACKET *HandleIpcResyncClientRecord (IPC_PACKET *ptrPacket)
** In:      IPC_PACKET *ptrPacket       Pointer to the packet
**                                      that contains the request
** Returns: IPC_PACKET *    Result from this command.  All commands need to
**                          return some type of packet so that the call knows
**                          that the operation worked
**==========================================================================*/

static IPC_PACKET *HandleIpcResyncClientRecord(IPC_PACKET *ptrPacket)
{
    IPC_RESYNC_CLIENT_RECORD *pClientRecord = NULL;
    UINT8       status = IPC_COMMAND_SUCCESSFUL;
    char        pathname[324];
    UINT32      len;
    UINT32      error;
    UINT8       isResync = 1;

    ccb_assert(ptrPacket != NULL, ptrPacket);
    ccb_assert(ptrPacket->data != NULL, ptrPacket->data);

    dprintf(DPRINTF_DEFAULT, "HandleIpcResyncClientRecord - Received from the master to update a record\n");

    /*
     * Get the client record to be resync
     */
    pClientRecord = (IPC_RESYNC_CLIENT_RECORD *)ptrPacket->data;

    /*
     * append the default path to the record name
     * for creating a file
     */
    appendDefaultPath(pathname, pClientRecord->recordName);
    /*
     * create client record file with the path name
     */
    error = CreateClientRecord(pathname);
    /*
     * Assuming the slave mgt structure for the records
     * is up and in the memory of mgtList
     */
    if (error == PI_GOOD)
    {
        /*
         * Add the record in the mgt structure
         */
        AddMgtStructure(pClientRecord->recordName, 0, isResync);
    }
    /*
     * error may be PI_GOOD or PI_ERROR. Try to write the
     * data on to file
     */
    len = pClientRecord->dataSize;
    /*
     * Resync write the ewok record to the file
     */
    error = WriteClientRecord(pathname, pClientRecord->startOffset,
                              &len, pClientRecord->data, 0);

    /*
     * update management list and file
     */
    if (PI_ERROR == GetClientRecordSize(pathname, &len))
    {
        /* KM: need to update this */
    }
    status = UpdateMgtStructure(pClientRecord->recordName, len, isResync);

    return (ptrPacket);

}

/*===========================================================================
** Name:    IPC_PACKET *HandleIpcResyncClientCmd (IPC_PACKET *ptrPacket)
** In:      IPC_PACKET *ptrPacket       Pointer to the packet
**                                      that contains the request
** Returns: IPC_PACKET *    Result from this command.  All commands need to
**                          return some type of packet so that the call knows
**                          that the operation worked
**==========================================================================*/

static IPC_PACKET *HandleIpcResyncClientCmd(IPC_PACKET *ptrPacket)
{
    IPC_RESYNC_CLIENT_CMD *pClientCmd;

    ccb_assert(ptrPacket != NULL, ptrPacket);
    ccb_assert(ptrPacket->data != NULL, ptrPacket->data);

    dprintf(DPRINTF_DEFAULT, "HandleIpcResyncClientCmd - Received from a slave\n");

    /*
     * Get the client cmd to get slave serialnumer
     */
    pClientCmd = (IPC_RESYNC_CLIENT_CMD *)ptrPacket->data;

    TransferClientDataToSlave(pClientCmd->slaveSN);

    return (ptrPacket);

}

/*===========================================================================
** Name:    IPC_PACKET *HandleIpcLatestPersistent (IPC_PACKET *ptrPacket)
** In:      IPC_PACKET *ptrPacket       Pointer to the packet
**                                      that contains the request
** Returns: IPC_PACKET *    Result from this command.  All commands need to
**                          return some type of packet so that the call knows
**                          that the operation worked
**==========================================================================*/

static IPC_PACKET *HandleIpcLatestPersistent(IPC_PACKET *ptrPacket)
{
    IPC_LATEST_PERSISTENT_DATA *pClientCmd;

    ccb_assert(ptrPacket != NULL, ptrPacket);
    ccb_assert(ptrPacket->data != NULL, ptrPacket->data);

    dprintf(DPRINTF_DEFAULT, "HandleIpcLatestPersistent - Received from a controller\n");
    /*
     * Get the client cmd to get serialnumer of sender
     */

    pClientCmd = (IPC_LATEST_PERSISTENT_DATA *)ptrPacket->data;

    /*
     * update the latestClient data if the sender has latest
     */
    GetLatestClientData(pClientCmd);

    return (ptrPacket);
}

/*===========================================================================
** Name:    IPC_PACKET *HandleIpcPrData (IPC_PACKET *ptrPacket)
** In:      IPC_PACKET *ptrPacket       Pointer to the packet
**                                      that contains the request
** Returns: IPC_PACKET *    Result from this command.  All commands need to
**                          return some type of packet so that the call knows
**                          that the operation worked
**==========================================================================*/

static IPC_PACKET *HandleIpcPrData(IPC_PACKET *ptrPacket)
{
    IPC_SETPRES_DATA *pSetPrCmd;
    XIO_PACKET  reqPacket;
    XIO_PACKET  rspPacket;

    ccb_assert(ptrPacket != NULL, ptrPacket);
    ccb_assert(ptrPacket->data != NULL, ptrPacket->data);

    dprintf(DPRINTF_DEFAULT, "PRES: HandleIpcPrData - Received from the slave %d\n",
            ptrPacket->data->setpr.serialNum);

    /*
     * Get the client cmd to get slave serialnumer
     */
    pSetPrCmd = (IPC_SETPRES_DATA *)ptrPacket->data;

    /*
     * Allocate memory for the headers.
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    rspPacket.pPacket = NULL;

    /*
     * If needed update the version number in packet header
     * reqPacket.pHeader->version = default
     */

    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    /*
     * Build the reqPacket from IPC packet
     */

    reqPacket.pHeader->length = pSetPrCmd->pktSize - sizeof(*rspPacket.pHeader);
    reqPacket.pHeader->commandCode = PI_SET_PR_CMD;

    /*
     * create the reqpacket data part
     */
    reqPacket.pPacket = MallocWC(reqPacket.pHeader->length);

    memcpy(reqPacket.pPacket,
           pSetPrCmd->packet + sizeof(*reqPacket.pHeader), reqPacket.pHeader->length);

    PortServerCommandHandler(&reqPacket, &rspPacket);

    Free(reqPacket);
    Free(rspPacket);

    return (ptrPacket);
}


/*----------------------------------------------------------------------------
**  Function Name: HandleAllDevicesMissing
**
**  Inputs:     None
**
**  Returns:    NONE
**--------------------------------------------------------------------------*/
static void HandleAllDevicesMissingTask(UNUSED TASK_PARMS *parms)
{
    UINT32      mrpReturnCode = PI_GOOD;
    MRALLDEVMISS_RSP *outPktPtr = NULL;
    MRALLDEVMISS_REQ *inPktPtr = NULL;
    LOG_ALL_DEV_MISSING_DAT *pAllDevMissData = &gDevMissData;

    dprintf(DPRINTF_ELECTION, "<GR>CCB-HandleAllDevicesMissing: IN(failed CN=%x)..\n",
            pAllDevMissData->cSerial);

    /*
     * Allocate memory space for the MRP packet
     */
    inPktPtr = MallocSharedWC(sizeof(*inPktPtr));
    outPktPtr = MallocSharedWC(sizeof(*outPktPtr));

    memcpy((void *)(inPktPtr->syncVdMap), (void *)(pAllDevMissData->syncVdMap),
           (MAX_VIRTUAL_DISKS + 7) / 8);
    memcpy((void *)(inPktPtr->srcVdMap), (void *)(pAllDevMissData->srcVdMap),
           (MAX_VIRTUAL_DISKS + 7) / 8);

#if GR_GEORAID15_DEBUG
    {
        UINT8      *pSyncVdMap = (UINT8 *)inPktPtr->syncVdMap;
        UINT8      *pSrcVdMap = (UINT8 *)inPktPtr->srcVdMap;
        int         i;

        for (i = 0; i < 64; (i = i + 16))
        {
            fprintf(stderr,
                    "<GR>CCB-received-DEST MAP %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x\n\r",
                    pSyncVdMap[i], pSyncVdMap[i + 1], pSyncVdMap[i + 2],
                    pSyncVdMap[i + 3], pSyncVdMap[i + 4], pSyncVdMap[i + 5],
                    pSyncVdMap[i + 6], pSyncVdMap[i + 7], pSyncVdMap[i + 8],
                    pSyncVdMap[i + 9], pSyncVdMap[i + 10], pSyncVdMap[i + 11],
                    pSyncVdMap[i + 12], pSyncVdMap[i + 13], pSyncVdMap[i + 14],
                    pSyncVdMap[i + 15]);
        }

        for (i = 0; i < 64; (i = i + 16))
        {
            fprintf(stderr,
                    "<GR>CCB-received-SRC MAP %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x\n\r",
                    pSrcVdMap[i], pSrcVdMap[i + 1], pSrcVdMap[i + 2], pSrcVdMap[i + 3],
                    pSrcVdMap[i + 4], pSrcVdMap[i + 5], pSrcVdMap[i + 6],
                    pSrcVdMap[i + 7], pSrcVdMap[i + 8], pSrcVdMap[i + 9],
                    pSrcVdMap[i + 10], pSrcVdMap[i + 11], pSrcVdMap[i + 12],
                    pSrcVdMap[i + 13], pSrcVdMap[i + 14], pSrcVdMap[i + 15]);
        }
    }
#endif  /* GR_GEORAID15_DEBUG */

    /*
     * Execute the MRP
     */
#if 1
    mrpReturnCode = PI_ExecMRP(inPktPtr, sizeof(*inPktPtr), MRALLDEVMISS,
                               outPktPtr, sizeof(*outPktPtr), 2000);
#else   /* 1 */
    mrpReturnCode = PI_ExecMRP(inPktPtr, sizeof(*inPktPtr), MRALLDEVMISS,
                               outPktPtr, sizeof(*outPktPtr), GetGlobalMRPTimeout());
#endif  /* 1 */

    dprintf(DPRINTF_ELECTION, "<GR>CCB-HandleAllDevicesMissing:: OUT(status=%x)..\n",
            mrpReturnCode);
    switch (mrpReturnCode)
    {
        case PI_TIMEOUT:
            dprintf(DPRINTF_ELECTION, "HandleAllDevicesMissing:: Timed out (non-fatal)\n\r");
            /*
             * FALL THROUGH TO THE PI_GOOD HANDLING SINCE TIMEOUT IS
             * NON-FATAL.
             */

        case PI_GOOD:
            /*
             * GOOD or TIMEOUT, either case nothing to do since the
             * returnCode is initialized to GOOD.
             */
            break;

        default:
            break;
    }

    /*
     * Free the allocated memory.
     */

    if (mrpReturnCode != PI_TIMEOUT)
    {
        Free(outPktPtr);
    }

    /*
     * Free the memory that was allocated.
     * Use delayed free on the input packet since it contains
     * a PCI address.
     */
    DelayedFree(MRALLDEVMISS, inPktPtr);
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
