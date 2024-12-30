/* $Id: PI_VCG.c 160898 2013-04-09 22:11:45Z marshall_midden $*/
/*===========================================================================
** FILE NAME:       PI_VCG.c
** MODULE TITLE:    Packet Interface for Virtual Controller Group Commands
**
** DESCRIPTION:     Handler functions for VCG commands
**
** Copyright (c) 2001-2009 Xiotech Corporation. All rights reserved.
**==========================================================================*/
#include "PacketInterface.h"

#include "cps_init.h"
#include "debug_files.h"
#include "LOG_Defs.h"
#include "EL.h"
#include "error_handler.h"
#include "errorCodes.h"
#include "fm.h"
#include "ipc_listener.h"
#include "ipc_packets.h"
#include "ipc_sendpacket.h"
#include "ipc_session_manager.h"
#include "led.h"
#include "logdef.h"
#include "misc.h"
#include "mode.h"
#include "MR_Defs.h"
#include "nvram.h"
#include "PI_CmdHandlers.h"
#include "PI_ClientPersistent.h"
#include "PI_PDisk.h"
#include "PI_Utils.h"
#include "PI_Target.h"
#include "PktCmdHdl.h"
#include "PortServer.h"
#include "PortServerUtils.h"
#include "quorum.h"
#include "quorum_utils.h"
#include "rm.h"
#include "rm_val.h"
#include "RMCmdHdl.h"
#include "SerConNetwork.h"
#include "sm.h"
#include "Snapshot.h"
#include "X1_Workset.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"
#include "XIO_Types.h"
#include "CT_history.h"

#include "L_Misc.h"

/*****************************************************************************
** Private defines
*****************************************************************************/
#define TMO_SEND_IPC_ADD_CONTROLLER 5000
#define TMO_SEND_IPC_SET_IP         5000
#define OFFLINE_SUICIDE_SELF_DELAY  5000        /* Delay in ms before suicide */

/*
** TMO_UNFAIL_CONTROLLER is the amount of time we will wait for a controller
** being unfailed to reach a state other than "unfail".  The value represents
** how many loops of 10 second waits are done.  At the current value of
** 30 the timeout is 5 minutes.
*/
#define TMO_UNFAIL_CONTROLLER       30

/*
** Number of times to retry the PING operation when unfailing a
** controller.
*/
#define VCG_PING_RETRY_COUNT            30

/*****************************************************************************
** Public routines - not externed in any header file
*****************************************************************************/
extern void ConfigCtrl(TASK_PARMS *parms);
extern void SerSetupConfigInfo(PI_VCG_CONFIGURE_REQ *pReq);

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static INT32 SendIpcAddController(UINT32 serialNum);
static bool ControllerReadyForRemoval(UINT32 serialNum, PI_TARGETS_RSP *pTargets);
static void DelayedFail(TASK_PARMS *parms);
static void DelayedSuicide(TASK_PARMS *parms);
static NORETURN void PI_VCGShutdownTask(TASK_PARMS *parms);
static INT32 ShutdownCheckForIpChange(void);
static void ShutdownLoadRollBackCM(QM_CONTROLLER_CONFIG_MAP *contMap);
static INT32 ShutdownClearPendingChangesCM(void);
static INT32 ShutdownChangeIPAdresses(QM_CONTROLLER_CONFIG_MAP *contMap);
static INT32 SetIpAddressTunnel(UINT32 serNum, UINT32 ip, UINT32 snm, UINT32 gw);

/*****************************************************************************
** Code Start
*****************************************************************************/

/**
******************************************************************************
**
**  @brief  Ask the BE to save async NV to a file
**
**  @param  none
**
**  @return none
**
******************************************************************************
**/
static void SaveAsyncNV(void)
{
    INT32       rc;
    MR_PKT      *req;
    MR_RSP_PKT  *rsp;

    dprintf(DPRINTF_DEFAULT, "%s: Asking BE to save async NV to file\n",
            __func__);

    /* Allocate memory for the MRP input and output packets. */

    req = MallocWC(sizeof(*req));
    rsp = MallocSharedWC(sizeof(*rsp));

    /*
    ** Send the request. This function handles timeout
    ** conditions and task switches while waiting.
    */
    rc = PI_ExecMRP(req, sizeof(*req), MRSAVEASYNCNV,
                    rsp, sizeof(*rsp), GetGlobalMRPTimeout());

    Free(req);                  /* Free the allocated memory. */

    if (rc != PI_TIMEOUT)
    {
        Free(rsp);
    }
}

/*----------------------------------------------------------------------------
** Function:    PI_VCGApplyLicense
**
** Description: Apply license information to the virtual controller group.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_VCGApplyLicense(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    INT32       errorCode = 0;
    PI_VCG_APPLY_LICENSE_REQ *pRequest = NULL;
    UINT16      powerUpState = POWER_UP_START;
    PI_PDISKS_RSP *pPDisks = NULL;
    UINT16      i = 0;
    UINT16      goodDrives = 0;
    bool        bInitialLicense = false;

    dprintf(DPRINTF_DEFAULT, "PI_VCGApplyLicense - ENTER\n");

    ccb_assert(pReqPacket != NULL, pReqPacket);
    ccb_assert(pReqPacket->pPacket != NULL, pReqPacket->pPacket);
    ccb_assert(pRspPacket != NULL, pRspPacket);
    ccb_assert(pRspPacket->pHeader != NULL, pRspPacket->pHeader);

    pRequest = (PI_VCG_APPLY_LICENSE_REQ *)pReqPacket->pPacket;

    /*
     * Check if this is the initial license being applied
     * to this controller.
     */
    if (!IsLicenseApplied())
    {
        bInitialLicense = true;

        /*
         * Check if the current power-up state says the controller
         * is waiting for a license.  If not the controller is
         * considered to be in an invalid state and the license
         * cannot be applied.
         */
        if (GetPowerUpState() != POWER_UP_WAIT_LICENSE)
        {
            rc = PI_ERROR;
            errorCode = EC_VCG_AL_INVALID_STATE;
        }

        /*
         * The controller is waiting for a license so make sure
         * there are drives that can be labeled.
         */
        if (rc == PI_GOOD)
        {
            /* Get the physical disks that can be seen. */
            pPDisks = PhysicalDisks();

            if (pPDisks != NULL)
            {
                /*
                 * Loop through the physical disks and see if any of
                 * them are labelable disks.  A labelable disk is one
                 * that is currently unlabeled and it is either operable
                 * or inoperable with a failed directory check.
                 */
                for (i = 0; i < pPDisks->count; i++)
                {
                    if (pPDisks->pdiskInfo[i].pdd.devClass == PD_UNLAB &&
                        (pPDisks->pdiskInfo[i].pdd.devStat == PD_OP ||
                         (pPDisks->pdiskInfo[i].pdd.devStat == PD_INOP &&
                          pPDisks->pdiskInfo[i].pdd.postStat == PD_FDIR)))
                    {
                        goodDrives++;

                        /*
                         * Found a good drive so break out of the loop since
                         * only one good drive is required.
                         */
                        break;
                    }
                }
                /* Make sure the memory is cleaned up. */
                Free(pPDisks);
            }

            /* If there are no labelable drives in the system, flag the error. */
            if (goodDrives == 0)
            {
                rc = PI_ERROR;
                errorCode = EC_VCG_NO_DRIVES;
            }
        }
    }

    if (rc == PI_GOOD)
    {
        /* Create the maximum number of controller allowed. */
        rc = CreateController(pRequest->vcgMaxNumControllers);

        if (rc != PI_GOOD)
        {
            dprintf(DPRINTF_DEFAULT, "PI_VCGApplyLicense - Unable to create the controller: 0x%x\n",
                    rc);
        }
    }

    if (rc == PI_GOOD)
    {
        /*
         * The license information is store in the master configuration so
         * we want to make sure the information in the DRAM copy of the
         * master configuration is up to date.
         */
        LoadMasterConfigFromNVRAM();

        Qm_SetVirtualControllerSN(CntlSetup_GetSystemSN());
        Qm_SetNumControllersAllowed(pRequest->vcgMaxNumControllers);

        StoreMasterConfigToNVRAM();

        SetLicenseApplied();

        do
        {
            /* Wait for 1 second(s) before checking the power-up state */
            TaskSleepMS(1000);

            /* Get the current power-up state. */
            powerUpState = GetPowerUpState();

            /*
             * Continue to get the power-up state and loop unless we
             * have reached one of the states below.
             */
        } while (powerUpState != POWER_UP_WAIT_DRIVES &&
                 powerUpState != POWER_UP_WAIT_CONTROLLERS &&
                 powerUpState != POWER_UP_WAIT_DISK_BAY &&
                 powerUpState != POWER_UP_WAIT_CORRUPT_BE_NVRAM &&
                 powerUpState != POWER_UP_FAILED && powerUpState != POWER_UP_COMPLETE);

        if (bInitialLicense)
        {
            InitWorksetDefaultVPort();
        }

        /*
         * If there are owned drives we can save the modifications to
         * drives, otherwise only save the master configuraiton to NVRAM.
         */
        if (Qm_GetOwnedDriveCount() > 0)
        {
            /*
             * We have updated the master configuration so make sure it
             * gets save to DISK and NVRAM.
             */
            SaveMasterConfig();

            /*
             * Save the changes to the controller configuration map
             * to the drives.
             */
            SaveControllerConfigMap();

            /*
             * Make sure all operational slaves see the update to the
             * configuration.
             */
            IpcSignalSlaves(IPC_SIGNAL_LOAD_CONFIG);
        }
    }

    /*
     * Attach the return data packet to the main response packet.
     * Fill in the header length and status fields.
     */
    pRspPacket->pPacket = NULL;
    pRspPacket->pHeader->length = 0;
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = errorCode;

    dprintf(DPRINTF_DEFAULT, "PI_VCGApplyLicense - EXIT\n");

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PI_VCGFailController
**
** Description: Fail one of the controllers in the group.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_VCGFailController(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    INT32       errorCode = 0;
    PI_VCG_FAIL_CONTROLLER_REQ *pRequest = NULL;
    TASK_PARMS  parms;

    dprintf(DPRINTF_DEFAULT, "PI_VCGFailController - ENTER\n");

    ccb_assert(pReqPacket != NULL, pReqPacket);
    ccb_assert(pReqPacket->pPacket != NULL, pReqPacket->pPacket);
    ccb_assert(pRspPacket != NULL, pRspPacket);
    ccb_assert(pRspPacket->pHeader != NULL, pRspPacket->pHeader);

    pRequest = (PI_VCG_FAIL_CONTROLLER_REQ *)pReqPacket->pPacket;

    if (pRequest->serialNumber == GetMyControllerSN())
    {
        /*
         * This controller is being failed...after a delay!!!!
         */
        parms.p1 = (UINT32)OFFLINE_SUICIDE_SELF_DELAY;
        parms.p2 = (UINT32)pRequest->serialNumber;
        TaskCreate(DelayedFail, &parms);
    }
    else
    {
        parms.p1 = (UINT32)0;
        parms.p2 = (UINT32)pRequest->serialNumber;
        TaskCreate(DelayedFail, &parms);
        TaskSwitch();
    }

    /*
     * Attach the return data packet to the main response packet.
     * Fill in the header length and status fields.
     */
    pRspPacket->pPacket = NULL;
    pRspPacket->pHeader->length = 0;
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = errorCode;

    dprintf(DPRINTF_DEFAULT, "PI_VCGFailController - EXIT\n");

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PI_VCGInactivateController
**
** Description: Inactivate one of the controllers in the group.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
** FUTURE:      Disable cache and clear raid 5 mirror records?
**              Suicide controller?
**              Shutdown port server?
**              Stop CCB caching?
**--------------------------------------------------------------------------*/
INT32 PI_VCGInactivateController(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    INT32       rcLocal = GOOD;
    INT32       errorCode = 0;
    PI_VCG_INACTIVATE_CONTROLLER_REQ *pRequest = NULL;
    QM_FAILURE_DATA *failureData;
    PI_TARGETS_RSP *pTargets;
    bool        bOwnTargets;
    INT32       i;
    bool        bTempDisableCache = false;
    bool        bStopIO = false;
    MODEDATA    mBits;
    TASK_PARMS  resetParms;

    dprintf(DPRINTF_DEFAULT, "PI_VCGInactivateController: ENTER\n");

    ccb_assert(pReqPacket != NULL, pReqPacket);
    ccb_assert(pReqPacket->pPacket != NULL, pReqPacket->pPacket);
    ccb_assert(pRspPacket != NULL, pRspPacket);
    ccb_assert(pRspPacket->pHeader != NULL, pRspPacket->pHeader);

    /*
     * Retrieve the mode bits.
     */
    ModeGet(&mBits);

    pRequest = (PI_VCG_INACTIVATE_CONTROLLER_REQ *)pReqPacket->pPacket;

    failureData = MallocSharedWC(sizeof(*failureData));
    do
    {
        /*
         * The inactivate request can only be sent to the controller
         * being inactivated so make sure the serial number in the
         * request matches this controller SN.
         */
        if (pRequest->serialNumber != GetMyControllerSN())
        {
            errorCode = EC_VCG_IC_INVALID_CTRL;
            rc = PI_ERROR;
            break;
        }

        /*
         * If the controller is not mirroring check if it has any
         * owned targets.  If it is not mirroring and it owns
         * targets the inactivation cannot continue.
         *
         * If the controller is mirroring make sure it can talk
         * to the mirror partner before allowing the inactivation.
         */
        if (GetCachedMirrorPartnerSN() == 0 ||
            GetCachedMirrorPartnerSN() == GetMyControllerSN())
        {
            /*
             * To determine if there are owned targets, first get
             * the target information.
             */
            bOwnTargets = false;
            pTargets = Targets(GetMyControllerSN());

            /*
             * If we were able to retrieve the target information, look
             * to see if this controller is the owner of any targets.
             *
             * If we were not able to retrieve the target information
             * assume that we own targets and do not allow the
             * inactivation.
             */
            if (pTargets)
            {
                for (i = 0; i < pTargets->count; ++i)
                {
                    if (pTargets->targetInfo[i].owner == GetMyControllerSN())
                    {
                        bOwnTargets = true;
                        break;
                    }
                }
            }
            else
            {
                /*
                 * Attempt to retrieve the target information failed,
                 * assume we own targets and do not allow the
                 * inactivation.
                 */
                bOwnTargets = true;
            }

            /*
             * The target information is no longer needed so deallocate
             * the memory.
             */
            Free(pTargets);

            /*
             * If the controller owns targets do not allow the
             * inactivation to continue.
             */
            if (bOwnTargets)
            {
                LogMessage(LOG_TYPE_DEBUG, "VCG_IC-Bad Mirror Partner");

                errorCode = EC_VCG_IC_BAD_MIRROR_PARTNER;
                rc = PI_ERROR;
                break;
            }
        }
        else
        {
            /*
             * Sending a test PING to the mirror partner of the controller
             * being inactivated to make sure that we can communicate with
             * that controller over IPC.
             */
            rc = IpcSendPingWithRetries(GetCachedMirrorPartnerSN(),
                                        SENDPACKET_ANY_PATH, VCG_PING_RETRY_COUNT);

            /*
             * If the Ping failed, flag the error condition.
             */
            if (rc != GOOD)
            {
                LogMessage(LOG_TYPE_DEBUG, "VCG_IC-Failed to ping Mirror Partner");

                errorCode = EC_VCG_IC_FAILED_PING;
                rc = PI_ERROR;
                break;
            }
        }

        dprintf(DPRINTF_DEFAULT, "PI_VCGInactivateController: Disable Cache\n");

        /*
         * Temporarily disable caching to allow the failover processing
         * to proceed a little faster.  This will also wait for the cache
         * to be flushed before continuing.
         */
        rc = RMTempDisableCache(PI_MISC_SETTDISCACHE_CMD, TEMP_DISABLE_INACTIVATE, 0);

        if (rc == PI_GOOD)
        {
            bTempDisableCache = true;
            RMWaitForCacheFlush();
        }
        else
        {
            errorCode = EC_VCG_IC_TDISCACHE;
            rc = PI_ERROR;
            break;
        }

        dprintf(DPRINTF_DEFAULT, "PI_VCGInactivateController: Stop IO\n");

        /*
         * Stop the IO on the controller being updated.
         */
        rc = StopIO(pRequest->serialNumber, STOP_WAIT_FOR_FLUSH | STOP_NO_BACKGROUND,
                    STOP_NO_SHUTDOWN, START_STOP_IO_USER_CCB_SM, TMO_INACTIVATE_STOP_IO);

        /*
         * If the stop request completed successfully or a timeout
         * occurred during the request, set the flag indicating
         * that a stop request is outstanding so we know to start
         * IO in case of an error after this.
         */
        if (rc == PI_GOOD || rc == PI_TIMEOUT)
        {
            bStopIO = true;
        }

        if (rc != GOOD)
        {
            dprintf(DPRINTF_DEFAULT, "PI_VCGInactivateController: Failed to stop IO.\n");

            errorCode = EC_VCG_IC_HOLD_IO;
            rc = PI_ERROR;
            break;
        }

        /*
         * Set the controller to FD_STATE_INACTIVATED.
         */
        dprintf(DPRINTF_DEFAULT, "PI_VCGInactivateController: Setting controller state to INACTIVE\n");

        rc = WriteFailureDataState(pRequest->serialNumber, FD_STATE_INACTIVATED);

        if (rc != GOOD)
        {
            errorCode = EC_VCG_IC_NOT_INACTIVATED;
            rc = PI_ERROR;
            break;
        }

        /*
         * Run an election to take this controller out of the heartbeat tree.
         * Resource manager will also fail its targets off at the end of
         * the election, since it won't show up in the active controller
         * map.
         */
        rc = EL_DoElection();

        if (rc != GOOD)
        {
            dprintf(DPRINTF_DEFAULT, "PI_VCGInactivateController: Failed to run an election (rc: 0x%x)\n",
                    rc);

            errorCode = EC_VCG_IC_EF;
            rc = PI_ERROR;
            break;
        }

        /*
         * Get the failure data for this controller.
         */
        memset(failureData, 0x00, sizeof(*failureData));
        ReadFailureData(pRequest->serialNumber, failureData);

        /*
         * If we are not now in the INACTIVAED state, something went
         * terribly wrong...
         */
        if (failureData->state != FD_STATE_INACTIVATED)
        {
            dprintf(DPRINTF_DEFAULT, "PI_VCGInactivateController: Not in INACTIVATED state (0x%x).\n",
                    failureData->state);

            errorCode = EC_VCG_IC_NOT_INACTIVATED;
            rc = PI_ERROR;
            break;
        }

        /*
         * Reset and hold the FE interfaces to make sure the hosts
         * do not attempt any IO.
         */
        rc = ResetInterfaceFE(pRequest->serialNumber, 0xFF, RESET_PORT_NO_INIT);

        if (rc != GOOD)
        {
            errorCode = EC_VCG_IC_HOLD_IO;
            rc = PI_ERROR;
            break;
        }

        /*
         * Tell the BEP that this controller will be a slave controller.
         */
        MRP_Awake(MRAWAKE_SLAVE);

        /*
         * Wait until the targets move off of this controller.  This
         * function will restore the configuration information and
         * check if this controller still owns targets.  If it still
         * owns targets it will again restore the configuration and
         * check.
         */
        RM_RestoreAndCheckForTargets();

        /*
         * Wait for the resync resources to be moved to another
         * controller.
         */
        RM_WaitForResyncInactivateOperations(180);

        /*
         * Set the clean shutdown flag
         */
        dprintf(DPRINTF_DEFAULT, "PI_VCGInactivateController: Setting Clean Shutdown Flag\n");
        if (SetCleanShutdown() != GOOD)
        {
            /*
             * This isn't catastrophic, just print it out.
             */
            dprintf(DPRINTF_DEFAULT, "PI_VCGInactivateController: Failed to set Clean Shutdown Flag\n");
        }

        /*
         * Shutdown the SDIMM Batteries (Lo-Power mode).
         */
        if (!(mBits.ccb.bits & MD_DISABLE_INACTIVATE_POWER))
        {
            SaveAsyncNV();
            dprintf(DPRINTF_DEFAULT, "PI_VCGInactivateController: Setting Buffer Board to low power mode.\n");
            rcLocal = BufferBoardShutdownControl();

            if (rcLocal != GOOD)
            {
                dprintf(DPRINTF_DEFAULT, "PI_VCGInactivateController: Error Setting Buffer Board to low power mode(0x%x).\n",
                        rcLocal);
            }
        }
        else
        {
            dprintf(DPRINTF_DEFAULT, "PI_VCGInactivateController: Bypassing Buffer Board to low power mode.\n");
        }

        /*
         * Turn on the Shutdown OK LED (CacheClear)
         */
        dprintf(DPRINTF_DEFAULT, "PI_VCGInactivateController: Turning on Offline LED.\n");
        LEDSetOffline(TRUE);

        /*
         * Shutdown the port server.
         */
        dprintf(DPRINTF_DEFAULT, "PI_VCGInactivateController: Shutting down the portserver.\n");
        ShutdownPortServer();
    } while (FALSE);

    Free(failureData);

    /*
     * If there is an error from above we need to check whether we should
     * be starting IO and caching back up.
     */
    if (rc != PI_GOOD)
    {
        /*
         * If the stop is outstanding and we are not returning a good status
         * we should attempt to start IO back up.
         */
        if (bStopIO)
        {
            StartIO(pRequest->serialNumber, START_IO_OPTION_CLEAR_ONE_STOP_COUNT,
                    START_STOP_IO_USER_CCB_SM, 0);
        }

        /*
         * If cache has been disabled and the overall request is not good
         * we need to re-enable caching.  Otherwise the controller will be
         * reset and the caching will be enabled when necessary during
         * the power-up and reallocation sequencing.
         */
        if (bTempDisableCache)
        {
            (void)RMTempDisableCache(PI_MISC_CLRTDISCACHE_CMD, TEMP_DISABLE_INACTIVATE, 0);
        }
    }

    /*
     * If everything was successful, reset the controller.
     * The reset command will see our failure state, and
     * power off the system.
     */
    if (rc == PI_GOOD)
    {
        resetParms.p1 = PROCESS_ALL;
        TaskCreate(ProcessResetTask, &resetParms);
    }

    /*
     * Attach the return data packet to the main response packet.
     * Fill in the header length and status fields.
     */
    pRspPacket->pPacket = NULL;
    pRspPacket->pHeader->length = 0;
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = errorCode;

    dprintf(DPRINTF_DEFAULT, "PI_VCGInactivateController: EXIT\n");

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PI_VCGPrepareSlave
**
** Description: Prepare a slave controller to be added to a Virtual
**              Controller Group
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_VCGPrepareSlave(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = GOOD;
    INT32       errorCode = 0;
    PI_VCG_PREPARE_SLAVE_REQ *pRequest = NULL;
    UINT32      count = 0;

    dprintf(DPRINTF_DEFAULT, "PI_VCGPrepareSlave - ENTER\n");

    ccb_assert(pReqPacket != NULL, pReqPacket);
    ccb_assert(pReqPacket->pPacket != NULL, pReqPacket->pPacket);
    ccb_assert(pRspPacket != NULL, pRspPacket);
    ccb_assert(pRspPacket->pHeader != NULL, pRspPacket->pHeader);

    pRequest = (PI_VCG_PREPARE_SLAVE_REQ *)pReqPacket->pPacket;

    if (CPSInitGetOwnedDriveCount(pRequest->vcgID) == 0)
    {
        dprintf(DPRINTF_DEFAULT, "PI_VCGPrepareSlave - This controller does not see any drives owned by the virtual controller group.\n");

        rc = PI_ERROR;
        errorCode = EC_VCG_NO_DRIVES;
    }
    else
    {
        /*
         * Get the slot number for the master controller by getting the
         * last nibble of the serial number.
         */
        count = Qm_SlotFromSerial(pRequest->controllerSN);

        memcpy(&masterConfig.communicationsKey, &pRequest->communicationsKey, 16);

        memset(&cntlConfigMap.cntlConfigInfo[count], 0x00, sizeof(QM_CONTROLLER_CONFIG));

        CCM_ControllerSN(count) = pRequest->controllerSN;
        CCM_IPAddress(count) = pRequest->ipEthernetAddress;
        CCM_Gateway(count) = masterConfig.gatewayAddress;
        CCM_Subnet(count) = masterConfig.subnetMask;
    }

    /*
     * Attach the return data packet to the main response packet.
     * Fill in the header length and status fields.
     */
    pRspPacket->pPacket = NULL;
    pRspPacket->pHeader->length = 0;
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = errorCode;

    dprintf(DPRINTF_DEFAULT, "PI_VCGPrepareSlave - EXIT\n");

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PI_VCGAddSlave
**
** Description: Initialize the controller to a single controller state
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_VCGAddSlave(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = GOOD;
    INT32       errorCode = 0;
    UINT32      serialNum = 0;
    UINT32      ipaddress = 0;
    UINT32      count = 0;
    QM_FAILURE_DATA *qmFailureData;

    dprintf(DPRINTF_DEFAULT, "PI_VCGAddSlave - ENTER\n");

    ccb_assert(pReqPacket != NULL, pReqPacket);
    ccb_assert(pReqPacket->pPacket != NULL, pReqPacket->pPacket);

    ipaddress = ((PI_VCG_ADD_SLAVE_REQ *)(pReqPacket->pPacket))->ipAddress;
    serialNum = ((PI_VCG_ADD_SLAVE_REQ *)(pReqPacket->pPacket))->serialNumber;

    dprintf(DPRINTF_DEFAULT, "PI_VCGAddSlave - Adding controller (%x)\n", ipaddress);

    if (rc == GOOD)
    {
        if (Qm_GetOwnedDriveCount() == 0)
        {
            dprintf(DPRINTF_DEFAULT, "PI_VCGAddSlave - No drives owned, cannot add slave controller.\n");

            rc = ERROR;
            errorCode = EC_VCG_NO_DRIVES;
        }
    }

    if (rc == GOOD)
    {
        /*
         * Get the slot number for this controller by getting the
         * last nibble of the serial number.
         */
        count = Qm_SlotFromSerial(serialNum);

        if (CCM_ControllerSN(count) == 0)
        {
            memset(&cntlConfigMap.cntlConfigInfo[count], 0x00,
                   sizeof(QM_CONTROLLER_CONFIG));

            CCM_ControllerSN(count) = serialNum;
            CCM_IPAddress(count) = ipaddress;
            CCM_IPAddressNew(count) = ipaddress;
            CCM_Gateway(count) = GetGatewayAddress();
            CCM_GatewayNew(count) = GetGatewayAddress();
            CCM_Subnet(count) = GetSubnetMask();
            CCM_SubnetNew(count) = GetSubnetMask();

            SaveControllerConfigMap();

            /*
             * Make sure all operational slaves see the update to the
             * configuration.
             */
            IpcSignalSlaves(IPC_SIGNAL_LOAD_CONFIG);
        }
        else
        {
            dprintf(DPRINTF_DEFAULT, "PI_VCGAddSlave - ERROR: The slot for controller (0x%x) already contains a controller.\n",
                    serialNum);

            rc = ERROR;
            errorCode = EC_VCG_AS_INVALID_CTRL;
        }
    }

    if (rc == GOOD)
    {
        /*
         * Sending a test PING to the slave-to-be controller to make sure
         * that we can communicate with that controller over IPC.
         */
        rc = IpcSendPing(serialNum, SENDPACKET_ETHERNET);

        if (rc == ERROR)
        {
            dprintf(DPRINTF_DEFAULT, "PI_VCGAddSlave - ERROR: Failed to ping slave controller.\n");

            memset(&cntlConfigMap.cntlConfigInfo[count], 0x00,
                   sizeof(QM_CONTROLLER_CONFIG));

            SaveControllerConfigMap();
            IpcSignalSlaves(IPC_SIGNAL_LOAD_CONFIG);

            /*
             * No matter if the remove from the quorum was successful or failed
             * we want to return an error at this point to make sure we do not
             * continue adding the controller to the group.
             */
            rc = ERROR;
            errorCode = EC_VCG_AS_FAILED_PING;
        }
    }

    if (rc == GOOD)
    {
        /*
         * If there is only one controller in the DSC we need to make sure
         * this controller has temporarily disabled cache.  This is to
         * ensure we are not caching when two controllers are active but
         * not mirroring to each other.  In this case we will temporarily
         * disable cache and wait for the flush to complete before allowing
         * the operation to proceed.
         */
        if (ACM_GetActiveControllerCount(Qm_ActiveCntlMapPtr()) == 1)
        {
            rc = SM_TempDisableCache(GetMyControllerSN(), PI_MISC_SETTDISCACHE_CMD,
                                     TEMP_DISABLE_MP, 0);

            if (rc == GOOD)
            {
                RMWaitForCacheFlushController(GetMyControllerSN());
            }
            else
            {
                rc = PI_ERROR;
                errorCode = EC_VCG_AC_TDISCACHE;
            }
        }
    }

    if (rc == GOOD)
    {
        rc = SendIpcAddController(serialNum);

        if (rc != GOOD)
        {
            memset(&cntlConfigMap.cntlConfigInfo[count], 0x00,
                   sizeof(QM_CONTROLLER_CONFIG));

            SaveControllerConfigMap();
            IpcSignalSlaves(IPC_SIGNAL_LOAD_CONFIG);

            errorCode = EC_VCG_AS_IPC_ADD_CONTROLLER;
        }
    }

    if (rc == GOOD)
    {
        WriteFailureDataState(serialNum, FD_STATE_ADD_CONTROLLER_TO_VCG);

        qmFailureData = MallocSharedWC(sizeof(*qmFailureData));
        do
        {
            /*
             * Wait for 1 second(s) before checking the power-up state
             */
            TaskSleepMS(1000);

            /*
             * Loop until we fail to read the failure data or the
             * controller being added goes to a state other than
             * FD_STATE_ADD_CONTROLLER_TO_VCG.
             */
        } while (ReadFailureData(serialNum, qmFailureData) == GOOD &&
                 qmFailureData->state == FD_STATE_ADD_CONTROLLER_TO_VCG);
        Free(qmFailureData);
    }

    if (rc == GOOD)
    {
        dprintf(DPRINTF_DEFAULT, "PI_VCGAddSlave - Process to add slave controller to the VCG completed.\n");
    }
    else
    {
        /*
         * The controller add did not work so we can clear the temporary
         * disable of cache.
         */
        SM_TempDisableCache(GetMyControllerSN(), PI_MISC_CLRTDISCACHE_CMD,
                            TEMP_DISABLE_MP, T_DISABLE_CLEAR_ONE);

        dprintf(DPRINTF_DEFAULT, "PI_VCGAddSlave - ERROR: Failed to add slave controller.\n");
    }

    /*
     * Attach the return data packet to the main response packet.
     * Fill in the header length and status fields.
     */
    pRspPacket->pPacket = NULL;
    pRspPacket->pHeader->length = 0;
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = errorCode;

    dprintf(DPRINTF_DEFAULT, "PI_VCGAddSlave - EXIT\n");

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PI_VCGPing
**
** Description: Initialize the controller to a single controller state
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_VCGPing(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    UINT32      serialNum;

    dprintf(DPRINTF_DEFAULT, "PI_VCGPing - ENTER\n");

    ccb_assert(pReqPacket != NULL, pReqPacket);
    ccb_assert(pReqPacket->pPacket != NULL, pReqPacket->pPacket);

    serialNum = ((PI_VCG_PING_REQ *)(pReqPacket->pPacket))->serialNumber;

    rc = IpcSendPing(serialNum, SENDPACKET_ETHERNET);

    /*
     * Attach the return data packet to the main response packet.
     * Fill in the header length and status fields.
     */
    pRspPacket->pPacket = NULL;
    pRspPacket->pHeader->length = 0;
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = 0;

    dprintf(DPRINTF_DEFAULT, "PI_VCGPing - EXIT\n");

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PI_VCGInfo
**
** Description: Virtual Controller Group Info
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_VCGInfo(UNUSED XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    PI_VCG_INFO_RSP *ptrOutPkt = NULL;
    UINT16      returnLength;
    UINT16      count = 0;
    UINT16      rspIndex = 0;
    UINT32      serialNum;
    QM_FAILURE_DATA *qmFailureData;

    ccb_assert(pRspPacket->pHeader != NULL, pRspPacket->pHeader);

    /*
     * Allocate memory for the return packet based on the number of
     * controllers.
     */
    returnLength = sizeof(*ptrOutPkt) +
        (Qm_GetNumControllersAllowed() * sizeof(PI_VCG_CTRL_INFO));

    /*
     * Allocate memory for the return data.
     */
    ptrOutPkt = MallocWC(returnLength);

    ptrOutPkt->vcgID = GetSerialNumber(SYSTEM_SN);
    ptrOutPkt->vcgIPAddress = Qm_GetIPAddress();
    ptrOutPkt->vcgMaxControllers = Qm_GetNumControllersAllowed();
    ptrOutPkt->vcgCurrentControllers = 0;

    if (Qm_GetNumControllersAllowed() > 0)
    {
        /*
         * Loop through all the controllers in the controller
         * configuration map to find all necessary information
         * that is to be returned to the user.
         */
        qmFailureData = MallocSharedWC(sizeof(*qmFailureData));
        for (count = 0; count < MAX_CONTROLLERS; count++)
        {
            /*
             * The controller configuration map is not a packed list
             * so there could be gaps, these gaps are identified by
             * a serial number of "0" and we can skip them.
             */
            if (cntlConfigMap.cntlConfigInfo[count].controllerSN == 0)
            {
                continue;
            }

            /*
             * Make sure that we have not gone past the maximum number
             * of controllers...it is a programming error if we did.
             */
            ccb_assert(rspIndex < Qm_GetNumControllersAllowed(), rspIndex);

            serialNum = cntlConfigMap.cntlConfigInfo[count].controllerSN;
            ccb_assert(serialNum != 0, serialNum);

            /* Initialize the failure data structure */
            memset(qmFailureData, 0x00, sizeof(*qmFailureData));

            /*
             * If we own drives, read up the failure state for
             * the controller.
             *
             * If we don't own drives, assume the controller is
             * operational.
             */
            if (Qm_GetOwnedDriveCount() > 0)
            {
                /*
                 * Attempt to read the failure data from the
                 * drive.
                 */
                if (ReadFailureData(serialNum, qmFailureData) != PI_GOOD)
                {
                    /*
                     * The read of the failure data from the
                     * drive failed so get the last known value
                     * from the election code (it read up the
                     * entire communications area).
                     */
                    qmFailureData->state = EL_GetFailureState(count);

                    /*
                     * If the election shows that the last known
                     * failure data was unused check if there
                     * is only one controller in the ACM and
                     * if the master controller is this controller.
                     * If that is the case make the controller
                     * operational since it seems that it is the
                     * only one in the group.
                     *
                     * NOTE: This handles the case when only one
                     *       controller is configured for the group,
                     *       drives cannot be read and the first
                     *       election has not yet occurred.
                     */
                    if (qmFailureData->state == FD_STATE_UNUSED &&
                        ACM_GetActiveControllerCount(Qm_ActiveCntlMapPtr()) == 1 &&
                        GetMyControllerSN() == Qm_GetMasterControllerSN())
                    {
                        qmFailureData->state = FD_STATE_OPERATIONAL;
                    }
                }

                /*
                 * If the state is something other than unused it means
                 * it is part of the CNC so add to the current controller
                 * count.
                 */
                if (qmFailureData->state > FD_STATE_UNUSED)
                {
                    ptrOutPkt->vcgCurrentControllers++;
                }
            }
            else
            {
                if (serialNum == GetMyControllerSN())
                {
                    qmFailureData->state = FD_STATE_OPERATIONAL;
                    ptrOutPkt->vcgCurrentControllers++;
                }
                else
                {
                    qmFailureData->state = FD_STATE_UNUSED;
                }
            }

            /* Save the controllers serial number */
            ptrOutPkt->controllers[rspIndex].serialNumber = serialNum;

            /* Save the controllers IP address */
            ptrOutPkt->controllers[rspIndex].ipAddress = cntlConfigMap.cntlConfigInfo[count].ipEthernetAddress;

            /* Save the controllers failure data */
            ptrOutPkt->controllers[rspIndex].failureState = qmFailureData->state;

            /*
             * Save whether or not this controller is the master.
             * Depending on the failure state the controller may always
             * be a slave controller.
             */
            switch (qmFailureData->state)
            {
                case FD_STATE_UNUSED:
                case FD_STATE_FAILED:
                case FD_STATE_ADD_CONTROLLER_TO_VCG:
                case FD_STATE_FIRMWARE_UPDATE_INACTIVE:
                case FD_STATE_INACTIVATED:
                case FD_STATE_DISASTER_INACTIVE:
                    ptrOutPkt->controllers[rspIndex].amIMaster = FALSE;
                    break;

                default:
                    ptrOutPkt->controllers[rspIndex].amIMaster = TestforMaster(serialNum);
                    break;
            }

            /*
             * Increment the response index.
             */
            rspIndex++;
        }
        Free(qmFailureData);
    }

    /*
     * Attach the return data packet to the main response packet.
     * Fill in the header length and status fields.
     */
    pRspPacket->pPacket = (UINT8 *)ptrOutPkt;
    pRspPacket->pHeader->length = returnLength;
    pRspPacket->pHeader->status = PI_GOOD;
    pRspPacket->pHeader->errorCode = PI_GOOD;

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PI_VCGRemoveController
**
** Description: Remove one of the controllers in the group.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_VCGRemoveController(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    UINT16      commSlot = MAX_CONTROLLERS;
    RMErrCode   rmErrCode = RMOK;
    PI_VCG_REMOVE_CONTROLLER_REQ *pRequest = NULL;
    PI_TARGETS_RSP *pTargets = NULL;
    TASK_PARMS  parms;

    dprintf(DPRINTF_DEFAULT, "PI_VCGRemoveController - ENTER\n");

    ccb_assert(pReqPacket != NULL, pReqPacket);
    ccb_assert(pReqPacket->pPacket != NULL, pReqPacket->pPacket);
    ccb_assert(pRspPacket != NULL, pRspPacket);
    ccb_assert(pRspPacket->pHeader != NULL, pRspPacket->pHeader);

    pRequest = (PI_VCG_REMOVE_CONTROLLER_REQ *)pReqPacket->pPacket;

    if (TestforMaster(GetMyControllerSN()) &&
        pRequest->serialNumber == GetMyControllerSN())
    {
        rc = PI_ERROR;
    }

    if (rc == PI_GOOD && TestforMaster(GetMyControllerSN()))
    {
        /*
         * Get the communications slot for the given serial number.
         */
        commSlot = GetCommunicationsSlot(pRequest->serialNumber);

        /*
         * Check if the controller has a communications slot.  If the controller
         * is not found in the communications area, MAX_CONTROLLERS is returned.
         */
        if (commSlot == MAX_CONTROLLERS)
        {
            rc = PI_ERROR;
        }

        if (rc == PI_GOOD)
        {
            /*
             * Get all the targets so we can find the ones that are owned by the
             * controller at a later point.
             */
            pTargets = Targets(GetMyControllerSN());

            if (pTargets == NULL)
            {
                rc = PI_ERROR;
            }
        }

        if (rc == PI_GOOD)
        {
            /*
             * Check if the controller is ready for removal.  It must pass the
             * following checks to be ready:
             *  - Own all of the targets where it is the preferred owner
             *  - No virtual disks can be associated with owned targets
             *  - No unmapped virtual disks can be associated with owned targets
             */
            if (!ControllerReadyForRemoval(pRequest->serialNumber, pTargets))
            {
                rc = PI_ERROR;
            }
        }

        if (rc == PI_GOOD)
        {
            rmErrCode = RMFailController(pRequest->serialNumber);

            if (rmErrCode != RMOK)
            {
                rc = PI_ERROR;
            }
        }

        if (rc == PI_GOOD)
        {
            /*
             * Set the failure state for this controller to unused since
             * that is the default value for an unconfigured controller.
             */
            WriteFailureDataState(pRequest->serialNumber, FD_STATE_UNUSED);

            /*
             * Clear the controller configuration map entry for the
             * controller being removed.
             */
            memset(&cntlConfigMap.cntlConfigInfo[commSlot], 0x00,
                   sizeof(QM_CONTROLLER_CONFIG));

            /*
             * Make sure the controller serial number is placed back into
             * the controller configuration map.
             */
            cntlConfigMap.cntlConfigInfo[commSlot].controllerSN = pRequest->serialNumber;

            /*
             * Save the changes to the controller configuration map
             * to the drives.
             */
            SaveControllerConfigMap();
        }
    }

    /*
     * If everything is good up to this point and the serial number of the
     * controller being failed is the same as this controller, we need to
     * clear system serial number, clear the master configuration and commit
     * suicide.
     */
    if (rc == PI_GOOD && pRequest->serialNumber == GetMyControllerSN())
    {
        /*
         * Reset the master configuration so it contains nothing but the
         * default empty configuration information.
         */
        ResetMasterConfigNVRAM();

        /*
         * This controller is going to go down hard...commit suicide!!!!
         */
        parms.p1 = (UINT32)OFFLINE_SUICIDE_SELF_DELAY;
        TaskCreate(DelayedSuicide, &parms);

        /*
         * Make sure the system serial number is set to zero.  This indicates
         * that this controller has no license.
         */
        CntlSetup_SetSystemSN(0);
        CntlSetup_SetControllerSN(0);
        ClearLicenseApplied();
        UpdateProcSerialNumber(CONTROLLER_SN, 0);

        /*
         * Tell the BEP that this controller will be the master controller.
         */
        MRP_Awake(MRAWAKE_MASTER);

        /*
         * Make sure the system serial number is set to zero.  This indicates
         * that this controller has no license.
         */
        CntlSetup_SetSystemSN(0);
        CntlSetup_SetControllerSN(0);
        ClearLicenseApplied();
        UpdateProcSerialNumber(CONTROLLER_SN, 0);
    }

    /*
     * Attach the return data packet to the main response packet.
     * Fill in the header length and status fields.
     */
    pRspPacket->pPacket = NULL;
    pRspPacket->pHeader->length = 0;
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = 0;

    Free(pTargets);

    dprintf(DPRINTF_DEFAULT, "PI_VCGRemoveController - EXIT\n");

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PI_VCGShutdown
**
** Description: Shutdown the virtual controller group.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
** WARNING:     Master must be Shutdown first!
**
**--------------------------------------------------------------------------*/
INT32 PI_VCGShutdown(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = GOOD;
    INT32       rcLocal = GOOD;
    UINT32      errCd = 0;
    QM_FAILURE_DATA *qmFailureData;
    LOG_STATUS_ONLY_PKT logStat;

    logStat.status = GOOD;
    logStat.errorCode = 0;

    /*
     * If we are not already shutdown
     */
    if (GetControllerFailureState() != FD_STATE_VCG_SHUTDOWN)
    {
        dprintf(DPRINTF_DEFAULT, "PI_VCGShutdown - VCG Controller Shutdown Start...\n");

        ccb_assert(pReqPacket != NULL, pReqPacket);
        ccb_assert(pRspPacket != NULL, pRspPacket);
        ccb_assert(pRspPacket->pHeader != NULL, pRspPacket->pHeader);

        /*
         * Wait here if an another election is running
         */
        if (EL_GetCurrentElectionState() != ED_STATE_END_TASK)
        {
            dprintf(DPRINTF_DEFAULT, "PI_VCGShutdown: Waiting for previous election to complete.\n");
        }

        while (EL_GetCurrentElectionState() != ED_STATE_END_TASK)
        {
            TaskSleepMS(20);
        }

        /*
         * Check to see if any IP Addresses have changed
         */
        if (ShutdownCheckForIpChange() == PI_ERROR)
        {
            dprintf(DPRINTF_DEFAULT, "PI_VCGShutdown: Change IP Addresses failed.\n");
            rc = ERROR;
            errCd |= VCG_SHUTDOWN_ERROR_CHANGE_NET_ADDRESES;
        }

        /*
         * Check to see that we own drives before attempting
         * to write the failure state to the quorum.
         */
        if (Qm_GetOwnedDriveCount() != 0)
        {
            /*
             * Set the controller to FD_STATE_VCG_SHUTDOWN.
             */
            /*
             * Set the state of qmFailureData.
             */
            qmFailureData = MallocSharedWC(sizeof(*qmFailureData));
            memset(qmFailureData, 0x00, sizeof(*qmFailureData));
            qmFailureData->state = FD_STATE_VCG_SHUTDOWN;

            dprintf(DPRINTF_DEFAULT, "PI_VCGShutdown: writing controller state VCG_SHUTDOWN to quorum.\n");

            /*
             * Write the failure state to the quorum.
             */
            rcLocal = WriteFailureData(GetMyControllerSN(), qmFailureData);
            Free(qmFailureData);

            /*
             * If we had an error set the error state and send an async event.
             */
            if (rcLocal != GOOD)
            {
                rc = ERROR;
                errCd |= VCG_SHUTDOWN_ERROR_WRITE_STATE_TO_QUORUM;

                logStat.status = rc;
                logStat.errorCode = VCG_SHUTDOWN_ERROR_WRITE_STATE_TO_QUORUM;
                SendAsyncEvent(LOG_VCG_SHUTDOWN_WARN, sizeof(logStat), &logStat);

                dprintf(DPRINTF_DEFAULT, "PI_VCGShutdown - ERROR: Write of failure state data failed, rcLocal: 0x%x.\n",
                        rcLocal);
            }

            /*
             * Capture startup configuration at this point
             */
            TakeSnapshot(SNAPSHOT_TYPE_SHUTDOWN, "AUTO - Shutdown");
        }

        /*
         * Set the controller failure state.
         */
        dprintf(DPRINTF_DEFAULT, "PI_VCGShutdown: Setting controller state to VCG_SHUTDOWN.\n");

        SetControllerFailureState(FD_STATE_VCG_SHUTDOWN);


        /*
         * Reset and hold the FE interfaces to make sure the hosts / vlinks
         * do not attempt any IO.
         */
        dprintf(DPRINTF_DEFAULT, "PI_VCGShutdown: Resetting FE Interfaces.\n");

        rc = ResetInterfaceFE(GetMyControllerSN(), 0xFF, RESET_PORT_NO_INIT);

        if (rc != GOOD)
        {
            dprintf(DPRINTF_DEFAULT, "PI_VCGShutdown: Failed to reset FE interfaces (rc: 0x%x)\n", rc);

            errCd |= 0x8000;    /* TWSDEBUG - NEED to define */
            rc = ERROR;
        }
    }

    /*
     * Fork the function to finish shutdown.
     */
    TaskCreate(PI_VCGShutdownTask, NULL);

    /*
     * Attach the return data packet to the main response packet.
     * Fill in the header length and status fields.
     */
    pRspPacket->pPacket = NULL;
    pRspPacket->pHeader->length = 0;
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = errCd;

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PI_VCGShutdownTask - FORKED
**
** Description: Shutdown the virtual controller group.
**
** Inputs:      TASK_PARMS* parms - Forked task parameters
**
** Returns:     NONE
**--------------------------------------------------------------------------*/
static NORETURN void PI_VCGShutdownTask(UNUSED TASK_PARMS *parms)
{
    INT32       rcLocal = GOOD;
    LOG_STATUS_ONLY_PKT logStat;

    logStat.status = GOOD;
    logStat.errorCode = 0;

    dprintf(DPRINTF_DEFAULT, "PI_VCGShutdownTask: Enter.\n");
    dprintf(DPRINTF_DEFAULT, "PI_VCGShutdownTask: Waiting 5 seconds before finishing VCGShutdown.\n");
    /*
     * Wait for 5 second(s) before finishing shutdown.
     */
    TaskSleepMS(5000);

    /*
     * Shutdown the port server.
     */
    dprintf(DPRINTF_DEFAULT, "PI_VCGShutdownTask: Shutting down the portserver.\n");
    ShutdownPortServer();

    /*
     * Set the PortServerClientAddress to 0,
     * This will stop the Async Client.
     */
    dprintf(DPRINTF_DEFAULT, "PI_VCGShutdownTask: Shutting down async client.\n");
    SetPortServerClientAddr(0);

    /*
     * Shutdown IPC.
     */
    dprintf(DPRINTF_DEFAULT, "PI_VCGShutdownTask: Shutting down IPC.\n");
    IpcShutDown();

    /*
     * Set the clean shutdown flag
     */
    dprintf(DPRINTF_DEFAULT, "PI_VCGShutdownTask: Setting Clean Shutdown Flag\n");
    if (SetCleanShutdown() != GOOD)
    {
        /*
         * This isn't catastrophic, just print it out.
         */
        dprintf(DPRINTF_DEFAULT, "PI_VCGShutdownTask: Failed to set Clean Shutdown Flag\n");
    }

    SaveAsyncNV();

    /*
     * Shutdown the SDIMM Batteries (Lo-Power mode).
     */
    dprintf(DPRINTF_DEFAULT, "PI_VCGShutdownTask: Setting Buffer Board to low power mode...\n");

    rcLocal = BufferBoardShutdownControl();

    /*
     * If we had an error send an async event.
     * If we have not hade a previous error set the error state.
     */
    if (rcLocal != GOOD)
    {
        logStat.status = ERROR;
        logStat.errorCode = VCG_SHUTDOWN_ERROR_FE_SDIMM_SHUTDOWN;
        SendAsyncEvent(LOG_VCG_SHUTDOWN_WARN, sizeof(logStat), &logStat);

        dprintf(DPRINTF_DEFAULT, "Error shutting down Buffer Board: rcLocal = 0x%02X\n",
                rcLocal);
    }

    /*
     * Turn on the Shutdown OK LED (CacheClear)
     */
    dprintf(DPRINTF_DEFAULT, "PI_VCGShutdownTask: Turning on Offline LED.\n");
    LEDSetOffline(TRUE);

    /*
     * Exit so PAM can reset the controller.
     */
    errExit(ERR_EXIT_SHUTDOWN);
}

/*----------------------------------------------------------------------------
** Function:    PI_VCGUnfailController
**
** Description: Unfail a controller that is currently in the failed state.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_VCGUnfailController(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    INT32       errorCode = 0;
    PI_VCG_UNFAIL_CONTROLLER_REQ *pRequest = NULL;
    QM_FAILURE_DATA *failureData;
    UINT32      startState = 0;
    UINT32      timeoutCounter = TMO_UNFAIL_CONTROLLER;

    dprintf(DPRINTF_DEFAULT, "PI_VCGUnfailController - ENTER\n");

    ccb_assert(pReqPacket != NULL, pReqPacket);
    ccb_assert(pReqPacket->pPacket != NULL, pReqPacket->pPacket);
    ccb_assert(pRspPacket != NULL, pRspPacket);
    ccb_assert(pRspPacket->pHeader != NULL, pRspPacket->pHeader);

    pRequest = (PI_VCG_UNFAIL_CONTROLLER_REQ *)pReqPacket->pPacket;

    /*
     * Read in the controllers current failure data to see if it
     * is currently in the failed state.  We can only unfail a
     * controller that is failed, duh!
     */
    failureData = MallocSharedWC(sizeof(*failureData));
    if (ReadFailureData(pRequest->serialNumber, failureData) != GOOD ||
        (failureData->state != FD_STATE_FAILED &&
         failureData->state != FD_STATE_INACTIVATED &&
         failureData->state != FD_STATE_FIRMWARE_UPDATE_INACTIVE))
    {
        rc = PI_ERROR;
        errorCode = EC_VCG_UC_INVALID_CTRL;
    }

    /*
     * TBolt00019877 - CJN
     *
     * If the return code is still good, check if this controller is setup
     * to be mirroring to itself.  The failover processing should have made
     * this change by this point unless there is something from preventing
     * the mirror partners from changing.
     *
     * NOTE: This check only works for 2-way systems.  To expand this to
     *       N-way systems this check would have to account for all active
     *       controllers and check their mirror partners.
     */
    UINT32 CMPSN = GetCachedMirrorPartnerSN();
    UINT32 GMCSN = GetMyControllerSN();
    if (rc == GOOD && CMPSN != GMCSN)
    {
        dprintf(DPRINTF_DEFAULT, "PI_VCGUnfailController failed CachedMirrorParnerSN 0x%08x != 0x%08x (mySN)\n",CMPSN, GMCSN);
        rc = PI_ERROR;
        errorCode = EC_VCG_UC_BAD_MIRROR_PARTNER;
    }

    /*
     * Save the state of this controller.  This value will determine
     * what state the controller will go to next.
     */
    startState = failureData->state;

    /*
     * If the return code is still good and we are not unfailing ourself then
     * we need to send a test PING request via IPC to the controller being
     * unfailed to bring up the interface and to verify the communication
     * paths are valid.
     */
    if (rc == GOOD && pRequest->serialNumber != GetMyControllerSN())
    {
        /*
         * Sending a test PING to the slave-to-be controller to make sure
         * that we can communicate with that controller over IPC.
         */
        rc = IpcSendPingWithRetries(pRequest->serialNumber,
                                    SENDPACKET_ETHERNET, VCG_PING_RETRY_COUNT);

        /*
         * If the Ping failed, flag the error condition.
         */
        if (rc != GOOD)
        {
            rc = PI_ERROR;
            errorCode = EC_VCG_UC_FAILED_PING;
        }
    }

    if (rc == GOOD)
    {
        /*
         * If there is only one controller in the DSC we need to make sure
         * this controller has temporarily disabled cache.  This is to
         * ensure we are not caching when two controllers are active but
         * not mirroring to each other.  In this case we will temporarily
         * disable cache and wait for the flush to complete before allowing
         * the operation to proceed.
         */
        if (ACM_GetActiveControllerCount(Qm_ActiveCntlMapPtr()) == 1)
        {
            rc = SM_TempDisableCache(GetMyControllerSN(),
                                     PI_MISC_SETTDISCACHE_CMD, TEMP_DISABLE_MP, 0);

            if (rc == GOOD)
            {
                RMWaitForCacheFlushController(GetMyControllerSN());
            }
            else
            {
                rc = PI_ERROR;
                errorCode = EC_VCG_UC_TDISCACHE;
            }
        }
    }

    if (rc == GOOD)
    {
        memset(failureData, 0x00, sizeof(*failureData));

        /*
         * Setup the next state for the controller based on
         * its starting state.
         */
        if (startState == FD_STATE_FAILED)
        {
            failureData->state = FD_STATE_UNFAIL_CONTROLLER;
        }
        else if (startState == FD_STATE_INACTIVATED)
        {
            failureData->state = FD_STATE_ACTIVATE;
        }
        else if (startState == FD_STATE_FIRMWARE_UPDATE_INACTIVE)
        {
            failureData->state = FD_STATE_FIRMWARE_UPDATE_ACTIVE;
        }

        rc = WriteFailureData(pRequest->serialNumber, failureData);

        /*
         * If writing the failue state failed, flag the error condition.
         */
        if (rc != PI_GOOD)
        {
            rc = PI_ERROR;
            errorCode = EC_VCG_UC_WRITE_FAILURE;
        }
        else
        {
            /*
             * Update the starting state of the controller.  We will
             * wait until the state changes before exiting from this
             * function (or a timeout occurs).
             */
            startState = failureData->state;

            do
            {
                /*
                 * Wait for 10 second(s) before checking the power-up state
                 */
                TaskSleepMS(10000);
                timeoutCounter--;

                memset(failureData, 0x00, sizeof(*failureData));
                rc = ReadFailureData(pRequest->serialNumber, failureData);

                /*
                 * Loop until we timeout, fail to read the failure data or
                 * the controller being added goes to a state other than
                 * FD_STATE_ADD_CONTROLLER_TO_VCG.
                 */
            } while (timeoutCounter > 0 && rc == GOOD && failureData->state == startState);

            /*
             * If the timeout expired then we need to fail the controller
             * and make sure we return an error.
             */
            if (timeoutCounter == 0)
            {
                memset(failureData, 0x00, sizeof(*failureData));

                /*
                 * Setup the next state for the controller based on
                 * its starting state.
                 */
                if (startState == FD_STATE_UNFAIL_CONTROLLER)
                {
                    failureData->state = FD_STATE_FAILED;
                }
                else if (startState == FD_STATE_ACTIVATE)
                {
                    failureData->state = FD_STATE_INACTIVATED;
                }
                else if (startState == FD_STATE_FIRMWARE_UPDATE_ACTIVE)
                {
                    failureData->state = FD_STATE_FIRMWARE_UPDATE_INACTIVE;
                }

                WriteFailureData(pRequest->serialNumber, failureData);

                rc = PI_ERROR;
                errorCode = EC_VCG_UC_TIMEOUT;
            }
        }
    }
    Free(failureData);

    if (rc != PI_GOOD)
    {
        /*
         * The unfail did not work so we can clear the temporary
         * disable of cache.
         */
        SM_TempDisableCache(GetMyControllerSN(),
                            PI_MISC_CLRTDISCACHE_CMD,
                            TEMP_DISABLE_MP, T_DISABLE_CLEAR_ONE);
    }

    /*
     * Send the latest persistent data to other controller
     * to get the updated list of records.
     */
    dprintf(DPRINTF_DEFAULT, "PI_VCGUnfailController - Persistent data resync started\n");
    SyncClientData();


    /*
     * Attach the return data packet to the main response packet.
     * Fill in the header length and status fields.
     */
    pRspPacket->pPacket = NULL;
    pRspPacket->pHeader->length = 0;
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = errorCode;

    dprintf(DPRINTF_DEFAULT, "PI_VCGUnfailController - EXIT (rc: 0x%x, ec: 0x%x)\n",
            rc, errorCode);

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PI_VCGValidation
**
** Description: Starts group redundancy validation on this group.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_VCGValidation(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    dprintf(DPRINTF_PI_COMMANDS, "PI_VCGValidation - validationFlags: 0x%04hX\n",
            ((PI_VCG_VALIDATION_REQ *)(pReqPacket->pPacket))->validationFlags);

    RM_StartGroupValidation(((PI_VCG_VALIDATION_REQ *)
                             (pReqPacket->pPacket))->validationFlags);

    /*
     * Attach the return data packet to the main response packet.
     * Fill in the header length and status fields.
     */
    pRspPacket->pPacket = NULL;
    pRspPacket->pHeader->length = 0;
    pRspPacket->pHeader->status = PI_GOOD;
    pRspPacket->pHeader->errorCode = 0;

    return (PI_GOOD);
}


/*----------------------------------------------------------------------------
** Function:    PIGetCpuCount
**
** Description: Get cpu count
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     none
**--------------------------------------------------------------------------*/
INT32 PI_GetCpuCount(UNUSED XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{

    PI_GET_CPUCOUNT_RSP *pRsp = NULL;
    INT32       rc = PI_GOOD;
    INT32       errorCode = 0;

    /*
     * Allocate the license conf rsp pkt
     */
    pRspPacket->pPacket = MallocWC(sizeof(*pRsp));

    /*
     * Set up response packet
     */
    pRspPacket->pHeader->length = sizeof(*pRsp);

    pRsp = (PI_GET_CPUCOUNT_RSP *)pRspPacket->pPacket;

    pRsp->cpuCount = GetCPUCount();

    dprintf(DPRINTF_PI_COMMANDS, "PIGetCpuCount: current config:\n"
            "  cpuCount:     %u\n", (UINT32)pRsp->cpuCount);

    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = errorCode;

    return (rc);
}


/*----------------------------------------------------------------------------
** Function:    PI_ConfigCtrl
**
** Description: Routine for initial configuration of the controller.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_ConfigCtrl(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       ec = PI_GOOD;
    TASK_PARMS  parms;

    PI_VCG_CONFIGURE_REQ *pReq;

    pReq = ((PI_VCG_CONFIGURE_REQ *)(pReqPacket->pPacket));

    dprintf(DPRINTF_DEFAULT, "PI_ConfigCtrl Ip %x, DSC %d, Repl %d\n", pReq->IPAddr,
            pReq->dscId, pReq->replacementFlag);

    /*
     * Attach the return data packet to the main response packet.
     * Fill in the header length and status fields.
     */
    pRspPacket->pPacket = NULL;
    pRspPacket->pHeader->length = 0;
    pRspPacket->pHeader->status = PI_GOOD;
    pRspPacket->pHeader->errorCode = 0;

    /*
     * Clean previous configuration information
     * from the controller before continuing.
     */
    SerSetupConfigInfo(pReq);
    parms.p1 = (UINT32)(pReq);
    TaskCreate(ConfigCtrl, &parms);

    return (ec);
}

/*----------------------------------------------------------------------------
**  Function Name: SendIpcAddController
**
**  Description:
**      Sends the IPC_ADD_CONTROLLER packet to the controller specified
**      by serialNum.
**
**  Inputs:
**      UINT32 serialNum - serial number of the controller to send the
**                          configuration update packet to.
**--------------------------------------------------------------------------*/
static INT32 SendIpcAddController(UINT32 serialNum)
{
    UINT32      rc = GOOD;
    IPC_PACKET  rx = { NULL, NULL };
    PATH_TYPE   pathType;
    IPC_PACKET *ptrPacket = NULL;
    UINT8       retries = 2;                /* Ethernet, Fiber(1), Disk Quorum(2) */

    dprintf(DPRINTF_DEFAULT, "SendIpcAddController (serialNum: 0x%x) - ENTER\n",
            serialNum);

    ptrPacket = CreatePacket(PACKET_IPC_ADD_CONTROLLER, sizeof(IPC_ADD_CONTROLLER), __FILE__, __LINE__);

    ptrPacket->data->addController.cserial = serialNum;

#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s call IpcSendPacketBySN with rxPacket of %p\n", __FILE__, __LINE__, __func__, &rx);
#endif  /* HISTORY_KEEP */

    do
    {
        Free(rx.data);

        /* Sending packet to the other controller using any IPC path possible */
        pathType = IpcSendPacketBySN(serialNum, SENDPACKET_ETHERNET,
                                     ptrPacket, &rx, NULL, NULL, NULL, TMO_SEND_IPC_ADD_CONTROLLER);
    } while (pathType == SENDPACKET_NO_PATH && (retries--) > 0);

    if (!IpcSuccessfulXfer(pathType))
    {
        dprintf(DPRINTF_DEFAULT, "SendIpcAddController - ERROR: Send packet failed (%u).\n",
                pathType);

        rc = ERROR;
    }
    else
    {
        if (rx.data->commandStatus.status != IPC_COMMAND_SUCCESSFUL)
        {
            dprintf(DPRINTF_DEFAULT, "SendIpcAddController - ERROR: Failed to send add controller packet.\n");

            rc = ERROR;
        }
    }

    if (ptrPacket != NULL)
    {
        FreePacket(&ptrPacket, __FILE__, __LINE__);
    }

    FreePacketStaticPacketPointer(&rx, __FILE__, __LINE__);

    dprintf(DPRINTF_DEFAULT, "SendIpcAddController - EXIT\n");

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    ControllerReadyForRemoval
**
** Description: Determines if the controller is ready to be removed from
**              the group.
**
** Inputs:      serialNum - Serial number of the controller to check.
**
** Returns:     true if the controller can be removed, false otherwise
**
**--------------------------------------------------------------------------*/
static bool ControllerReadyForRemoval(UINT32 serialNum, PI_TARGETS_RSP *pTargets)
{
    bool        bReady = true;
    PI_TARGET_RESOURCE_LIST_RSP *pTargetResList = NULL;
    UINT16      i;

    dprintf(DPRINTF_DEFAULT, "ControllerReadyForRemoval - ENTER\n");

    ccb_assert(pTargets != NULL, pTargets);

    /*
     * Loop through the targets to find the ones that we own.  For the
     * ones we own we want to make sure that there are no Virtual Disks
     * either associated or unmapped with the target.
     */
    for (i = 0; i < pTargets->count; i++)
    {
        /*
         * If the controller is not the preferred owner of this
         * target, skip it
         */
        if (pTargets->targetInfo[i].powner != serialNum)
        {
            continue;
        }

        /*
         * Well the controller is the preferred owner of this
         * target, does it currently own it?  If not the controller
         * cannot be removed.
         */
        if (pTargets->targetInfo[i].owner != serialNum)
        {
            bReady = false;
            break;
        }

        /*
         * Get the virtual disks associated with this target.  If there
         * are some or we cannot get the list the controller is not
         * ready to be removed.
         */
        pTargetResList = TargetResourceList(pTargets->targetInfo[i].tid, VDISKS);

        if (pTargetResList == NULL || pTargetResList->ndevs > 0)
        {
            if (pTargetResList != NULL)
            {
                Free(pTargetResList);
            }

            bReady = false;
            break;
        }

        Free(pTargetResList);
    }

    dprintf(DPRINTF_DEFAULT, "ControllerReadyForRemoval - EXIT\n");

    return bReady;
}

/*----------------------------------------------------------------------------
** Function:    DelayedFail - FORKED
**
** Description: Fails this controller after the delay has expired.  This
**              method is a forked operation.
**
** Inputs:      TASK_PARMS* parms - Forked task parameters, p1 = delay before
**                                  failing the controller, p2 = serialnumber
**                                  of the controller to fail.
**
** Returns:     NONE
**--------------------------------------------------------------------------*/
static void DelayedFail(TASK_PARMS *parms)
{
    IPC_REPORT_CONTROLLER_FAILURE *pFailurePacket = NULL;
    UINT32      delay = parms->p1;
    UINT32      serialNumber = parms->p2;

    dprintf(DPRINTF_DEFAULT, "DelayedFail - System (%u) is being failed (%u ms)...\n",
            serialNumber, delay);

    /*
     * Wait for the specified amount of time
     */
    if (delay > 0)
    {
        TaskSleepMS(delay);
    }

    pFailurePacket = MallocWC(SIZEOF_IPC_CONTROLLER_FAILURE);
    pFailurePacket->Type = IPC_FAILURE_TYPE_CONTROLLER_FAILED;
    pFailurePacket->FailureData.ControllerFailure.DetectedBySN = GetMyControllerSN();
    pFailurePacket->FailureData.ControllerFailure.FailedControllerSN = serialNumber;
    FailureManager(pFailurePacket, SIZEOF_IPC_CONTROLLER_FAILURE);
    /* Don't release the failure packet, FailureManager owns it now */
}

/*----------------------------------------------------------------------------
** Function:    DelayedSuicide - FORKED
**
** Description: Suicide this controller after the delay has expired.  This
**              method is a forked operation.
**
** Inputs:      TASK_PARMS* parms - Forked task parameters, p1 = delay in
**                                  milliseconds before suicide.
**
** Returns:     NONE
**--------------------------------------------------------------------------*/
static NORETURN void DelayedSuicide(TASK_PARMS *parms)
{
    UINT32      delay = parms->p1;

    dprintf(DPRINTF_DEFAULT, "DelayedSuicide - System is going down (%u ms)...\n", delay);

    /*
     * Wait for the specified amount of time
     */
    TaskSleepMS(delay);

    /*
     * We have waited long enough, die...
     */
    DeadLoop(EVENT_CONTROLLER_SUICIDE, TRUE);
}

/*----------------------------------------------------------------------------
** Function:    UpdateLicense
**
** Description: Update the existing license information for this VCG.
**
** Inputs:      PI_VCG_APPLY_LICENSE_REQ* pRequest - Packet request for
**                                                   applying a license.
**
** Outputs:     INT32 error code returned through pErrorCode.
**
** Returns:     PI_GOOD or PI_ERROR
**--------------------------------------------------------------------------*/
#if 0
INT32 UpdateLicense(PI_VCG_APPLY_LICENSE_REQ *pRequest, INT32 *pErrorCode)
{
    INT32       rc = PI_GOOD;
    UINT32      rmvCtrls[MAX_CONTROLLERS];
    UINT32      addCtrls[MAX_CONTROLLERS];
    INT16       i = 0;
    INT16       j = 0;
    INT16       rmvIndex = 0;
    INT16       addIndex = 0;

    memset((void *)&rmvCtrls, 0x00, sizeof(UINT32) * MAX_CONTROLLERS);
    memset((void *)&addCtrls, 0x00, sizeof(UINT32) * MAX_CONTROLLERS);

    /*
     * Make sure the VCGID is still the same...we do not allow
     * the ID to change at any time.
     */
    if (pRequest->vcgID != Qm_GetVirtualControllerSN())
    {
        rc = PI_ERROR;
        *pErrorCode = EC_VCG_AL_IL_CHG_VCGID;
        return rc;
    }

    /*
     * Find controllers that have been removed from the license...
     *
     * Loop through the controllers in the license and determine
     * if they are in the new license information.  If they are
     * not in the new license information it means the controller
     * has been removed.
     */
    for (i = 0; i < MAX_CONTROLLERS; i++)
    {
        /*
         * Skip any item in the license if it is zero, unused slot.
         */
        if (masterConfig.license[i] == 0)
        {
            continue;
        }

        /*
         * For each of the items in the license loop through the
         * new license information.
         */
        for (j = 0; j < pRequest->vcgMaxNumControllers; j++)
        {
            /*
             * Are these the same controller?  If so break out of
             * the FOR loop early.
             */
            if (masterConfig.license[i] == pRequest->controllers[j])
            {
                break;
            }
        }

        /*
         * If the loop reached the end the controller has been removed
         * from the group.  We are going to add it to the array of
         * controllers to be removed.
         */
        if (j == pRequest->vcgMaxNumControllers)
        {
            dprintf(DPRINTF_DEFAULT, "UpdateLicense - Remove Controller: %lu\n",
                    masterConfig.license[i]);

            rmvCtrls[rmvIndex] = masterConfig.license[i];
            rmvIndex++;
        }
    }

    /*
     * For each of the controllers in the remove array we need to
     * check if they are configured before allowing them to be
     * removed.
     */
    for (i = 0; i < rmvIndex; i++)
    {
        /*
         * Find the controller to be removed in the controller
         * configuration map and check if it is configured.
         */
        for (j = 0; j < MAX_CONTROLLERS; j++)
        {
            if (rmvCtrls[i] == cntlConfigMap.cntlConfigInfo[j].controllerSN)
            {
                /*
                 * Found the controller being removed, is its IP address
                 * something other than zero?  If so it is configured and
                 * cannot be removed...that is an error condition.
                 */
                if (cntlConfigMap.cntlConfigInfo[j].ipEthernetAddress != 0)
                {
                    rc = PI_ERROR;
                    *pErrorCode = EC_VCG_AL_IL_RMV_CFG_CTRL;
                    return rc;
                }

                break;
            }
        }
    }

    /*
     * Find controllers that have been added to the license...
     *
     * Loop through the controllers in the new license information
     * and determine if they are in the license.  If they are
     * not in the license it means the controller has been added.
     */
    for (i = 0; i < pRequest->vcgMaxNumControllers; i++)
    {
        /*
         * For each of the items in the new license information
         * loop through the license.
         */
        for (j = 0; j < MAX_CONTROLLERS; j++)
        {
            /*
             * Are these the same controller?  If so break out of
             * the FOR loop early.
             */
            if (pRequest->controllers[i] == masterConfig.license[j])
            {
                break;
            }
        }

        /*
         * If the loop reached the end the controller has been added
         * to the group.  We are going to add it tot he array of
         * controllers to be added.
         */
        if (j == MAX_CONTROLLERS)
        {
            dprintf(DPRINTF_DEFAULT, "UpdateLicense - Add Controller: %lu\n",
                    pRequest->controllers[i]);

            addCtrls[addIndex] = pRequest->controllers[i];
            addIndex++;
        }
    }

    /***********************************************************
    * The controllers being removed and added have been
    * identified so now the actual configuration information
    * needs to be modified.  The number of controllers in the
    * group may have changed, the licensed controllers may
    * have changed and the controller configuration map may
    * have changed.  Once the correct modifications have been
    * made the configuration needs to be saved, if there are
    * disk owned by the group the information will be written
    * to the disks and NVRAM otherwise the information will
    * be written only to NVRAM.
    **********************************************************/

    /*
     * Update the number of controllers in the group.
     */
    Qm_SetNumControllersAllowed(pRequest->vcgMaxNumControllers);

    /*
     * Create the maximum number of controller allowed.
     */
    CreateController(pRequest->vcgMaxNumControllers);

    /*
     * Remove the specified controllers...
     *
     * For each of the controllers in the remove array we need to
     * remove it from the license and from the controller config
     * map.
     */
    for (i = 0; i < rmvIndex; i++)
    {
        /*
         * Find the controller to be removed in the license and
         * set it to zero.
         */
        for (j = 0; j < MAX_CONTROLLERS; j++)
        {
            if (rmvCtrls[i] == masterConfig.license[j])
            {
                /*
                 * Set the license entry to zero, unused.
                 */
                masterConfig.license[j] = 0;

                /*
                 * The controller was found so we can skip the rest
                 * of the controllers.
                 */
                break;
            }
        }

        /*
         * Find the controller to be removed in the controller
         * configuration map and set its entire entry to zero.
         */
        for (j = 0; j < MAX_CONTROLLERS; j++)
        {
            if (rmvCtrls[i] == cntlConfigMap.cntlConfigInfo[j].controllerSN)
            {
                /*
                 * Clear the controller configuration map entry for the
                 * controller being removed.
                 */
                memset(&cntlConfigMap.cntlConfigInfo[j],
                       0x00, sizeof(QM_CONTROLLER_CONFIG));

                /*
                 * The controller was found so we can skip the rest
                 * of the controllers.
                 */
                break;
            }
        }
    }

    /*
     * Add the new controllers...
     *
     * For each of the controllers in the add array we need to add
     * it to the license and the controller configuration map.
     */
    for (i = 0; i < addIndex; i++)
    {
        /*
         * Find an empty slot in the license and fill it with the
         * new controller.
         */
        for (j = 0; j < MAX_CONTROLLERS; j++)
        {
            if (masterConfig.license[j] == 0)
            {
                masterConfig.license[j] = addCtrls[i];
                break;
            }
        }

        /*
         * Find an empty slot in the controller configuration map
         * and fill it with the new controller.
         */
        for (j = 0; j < MAX_CONTROLLERS; j++)
        {
            if (cntlConfigMap.cntlConfigInfo[j].controllerSN == 0)
            {
                /*
                 * Save the controller serial number in the controller
                 * configuration map.
                 */
                cntlConfigMap.cntlConfigInfo[j].controllerSN = addCtrls[i];

                /*
                 * The controller was found so we can skip the rest
                 * of the controllers.
                 */
                break;
            }
        }
    }

    /*
     * If there are owned drives we can save the modifications to
     * drives, otherwise only save the master configuraiton to NVRAM.
     */
    if (Qm_GetOwnedDriveCount() > 0)
    {
        /*
         * We have updated the master configuration so make sure it
         * gets save to DISK and NVRAM.
         */
        SaveMasterConfig();

        /*
         * Save the changes to the controller configuration map
         * to the drives.
         */
        SaveControllerConfigMap();
    }
    else
    {
        /*
         * We have updated the master configuration so make sure it
         * gets save to NVRAM.
         */
        StoreMasterConfigToNVRAM();
    }

    return rc;
}
#endif  /* 0 */

/*----------------------------------------------------------------------------
** Function:    ShutdownCheckForIpChange
**
** Description: Check for Controllers htat have changed there IP address
**              and update the controller config map.
**
** Inputs:      none
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
static INT32 ShutdownCheckForIpChange(void)
{
    INT32       rc = PI_GOOD;
    QM_CONTROLLER_CONFIG_MAP *pSnapShotCM = NULL;

    dprintf(DPRINTF_SHUTDOWN_CHECK_IP, "ShutdownCheckForIpChange - ENTER\n");
    dprintf(DPRINTF_SHUTDOWN_CHECK_IP, "Checking to see if we are Master - ENTER\n");

    /*
     * Check to see if we are the Master Controller.  If we are
     * then we need to check the controller configuration map to see
     * if any controllers in our VCG have changed there ip address.
     * If they have then we need to update this in the controller
     * configuration map so when the affected controller shuts down
     * it will see this change and update it's own local ip address.
     */
    if ((GetPowerUpState() == POWER_UP_COMPLETE) &&
        (GetControllerFailureState() == FD_STATE_OPERATIONAL) &&
        (TestforMaster(GetMyControllerSN())))
    {
        dprintf(DPRINTF_SHUTDOWN_CHECK_IP, "ShutdownCheckForIpChange - We are Master\n");
        dprintf(DPRINTF_SHUTDOWN_CHECK_IP, "ShutdownCheckForIpChange - Loading Controller config map\n");

        /*
         * Load the Controller Configuration Map.
         */
        if (LoadControllerMap() == 0)
        {
            dprintf(DPRINTF_SHUTDOWN_CHECK_IP, "ShutdownCheckForIpChange - Load Controller config map successful\n");

            /*
             * Take a snapshot of the controller map, in case
             * there are errors in changing the ip addresses.
             * We are taking the snapshot only for the purpose
             * of retrieving network addresses.
             */
            dprintf(DPRINTF_SHUTDOWN_CHECK_IP, "ShutdownCheckForIpChange - Taking snapshot of current network addresses\n");
            pSnapShotCM = MallocWC(sizeof(*pSnapShotCM));
            memcpy(pSnapShotCM, &cntlConfigMap, sizeof(*pSnapShotCM));

            /*
             * Change the Network Addresses.
             */
            dprintf(DPRINTF_SHUTDOWN_CHECK_IP, "ShutdownCheckForIpChange - Checking for address changes\n");
            rc = ShutdownChangeIPAdresses(&cntlConfigMap);

            /*
             * If there was an error, try to Roll it Back.
             */
            if (rc == PI_ERROR)
            {
                dprintf(DPRINTF_SHUTDOWN_CHECK_IP, "ShutdownCheckForIpChange - Address change failed, Attempting to Roll Back\n");

                /*
                 * Clear any changes that might still exist.
                 */
                ShutdownClearPendingChangesCM();

                /*
                 * Load the roll Back.
                 */
                ShutdownLoadRollBackCM(pSnapShotCM);

                /*
                 * Roll back the changes.
                 */
                if (ShutdownChangeIPAdresses(&cntlConfigMap) == PI_ERROR)
                {
                    dprintf(DPRINTF_SHUTDOWN_CHECK_IP, "ShutdownCheckForIpChange - Roll Back FAILED\n");
                }
                else
                {
                    dprintf(DPRINTF_SHUTDOWN_CHECK_IP, "ShutdownCheckForIpChange - Roll Back OK\n");
                }

                /*
                 * Clear any final changes that might still exist.
                 */
                ShutdownClearPendingChangesCM();
            }

            /*
             * Free the rollback Controller Map
             */
            Free(pSnapShotCM);
        }

        /*
         * Else we can not read the controller configuration map
         * from the quorum, set the status error.
         */
        else
        {
            rc = PI_ERROR;
        }
    }

    dprintf(DPRINTF_SHUTDOWN_CHECK_IP, "ShutdownCheckForIpChange - EXIT\n");

    return rc;
}

/*----------------------------------------------------------------------------
** Function:    ShutdownLoadRollBackCM
**
** Description: Loads the rollback contoller config map.
**
** Inputs:      none
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
static void ShutdownLoadRollBackCM(QM_CONTROLLER_CONFIG_MAP *contMap)
{
    UINT8       cont = 0;

    /*
     * We need to reverse the entries of the addresses
     */
    for (cont = 0; cont < MAX_CONTROLLERS; ++cont)
    {
        /*
         * If the serial number is 0 (unused) we can skip the rest
         * of this loop and start over with the next record.
         */
        if (contMap->cntlConfigInfo[cont].controllerSN == 0)
        {
            dprintf(DPRINTF_SHUTDOWN_CHECK_IP, "ShutdownChangeIPAdresses - Serial number is 0 skipping rest\n");

            continue;
        }

        /*
         * Check the ip address to see if it is being changed.
         */
        if ((((contMap->cntlConfigInfo[cont].ipEthernetAddress !=
               contMap->cntlConfigInfo[cont].newIpEthernetAddress) &&
              (contMap->cntlConfigInfo[cont].newIpEthernetAddress != 0))) &&
            (CCM_IPAddress(cont) != contMap->cntlConfigInfo[cont].ipEthernetAddress))
        {
            CCM_IPAddressNew(cont) = contMap->cntlConfigInfo[cont].ipEthernetAddress;
        }

        /*
         * Check the subnet mask to see if it is being changed.
         */
        if ((((contMap->cntlConfigInfo[cont].subnetMask !=
               contMap->cntlConfigInfo[cont].newSubnetMask) &&
              (contMap->cntlConfigInfo[cont].newSubnetMask != 0))) &&
            (CCM_Subnet(cont) != contMap->cntlConfigInfo[cont].subnetMask))
        {
            CCM_SubnetNew(cont) = contMap->cntlConfigInfo[cont].subnetMask;
        }

        /*
         * Check the gateway address to see if it is being changed.
         */
        if ((contMap->cntlConfigInfo[cont].gatewayAddress !=
             contMap->cntlConfigInfo[cont].newGatewayAddress) &&
            (CCM_Gateway(cont) != contMap->cntlConfigInfo[cont].gatewayAddress))
        {
            CCM_GatewayNew(cont) = contMap->cntlConfigInfo[cont].gatewayAddress;
        }
    }
}

/*----------------------------------------------------------------------------
** Function:    ShutdownClearPendingChangesCM
**
** Description: Clears out any changes that still need to be made.
**              This will prevent changes from being made from the
**              next power cycle.
**
** Inputs:      none
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
static INT32 ShutdownClearPendingChangesCM(void)
{
    INT32       rc = PI_GOOD;
    UINT8       cont = 0;

    /*
     * Loop through the controller map and remove any existing
     * changes.
     */
    for (cont = 0; cont < MAX_CONTROLLERS; ++cont)
    {
        /*
         * If the SN is 0 (unused) skip it.
         */
        if (CCM_ControllerSN(cont) == 0)
        {
            continue;
        }

        /*
         * Clear out any changes that are still pending.
         */
        CCM_IPAddressNew(cont) = CCM_IPAddress(cont);
        CCM_SubnetNew(cont) = CCM_Subnet(cont);
        CCM_GatewayNew(cont) = CCM_Gateway(cont);
    }

    /*
     * Write the changes to the quorum.
     */
    if (WriteControllerMap(&cntlConfigMap) != 0)
    {
        rc = PI_ERROR;
    }

    return rc;
}

/*----------------------------------------------------------------------------
** Function:    ShutdownChangeIPAdresses
**
** Description: Check for Controllers htat have changed there IP address
**              and update the controller config map.
**
** Inputs:      none
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
static INT32 ShutdownChangeIPAdresses(QM_CONTROLLER_CONFIG_MAP *contMap)
{
    INT32       rc = PI_GOOD;
    INT32       localrc = PI_GOOD;
    INT32       change = FALSE;
    INT32       cont = 0;
    UINT32      tmpSer = 0;
    UINT32      tmpIp = 0;
    UINT32      tmpSnm = 0;
    UINT32      tmpGw = 0;

    char        ip1[20];
    char        ip2[20];
    char        sn1[20];
    char        sn2[20];
    char        gt1[20];
    char        gt2[20];

    dprintf(DPRINTF_SHUTDOWN_CHECK_IP, "ShutdownChangeIPAdresses - ENTER\n");

    /*
     * Check to see if any controllers have had there
     * ip address change.
     */
    for (cont = 0; cont < MAX_CONTROLLERS; ++cont)
    {
        /*
         * Initialize change to FALSE each time through the loop.
         */
        change = FALSE;

        tmpSer = contMap->cntlConfigInfo[cont].controllerSN;

        InetToAscii(contMap->cntlConfigInfo[cont].ipEthernetAddress, ip1);
        InetToAscii(contMap->cntlConfigInfo[cont].newIpEthernetAddress, ip2);
        InetToAscii(contMap->cntlConfigInfo[cont].subnetMask, sn1);
        InetToAscii(contMap->cntlConfigInfo[cont].newSubnetMask, sn2);
        InetToAscii(contMap->cntlConfigInfo[cont].gatewayAddress, gt1);
        InetToAscii(contMap->cntlConfigInfo[cont].newGatewayAddress, gt2);

        dprintf(DPRINTF_SHUTDOWN_CHECK_IP,
                "ShutdownChangeIPAdresses - Controller slot %d\n"
                "...serial      %u\n"
                "...ip          %s\n"
                "...newip       %s\n"
                "...sn          %s\n"
                "...new sn      %s\n"
                "...gateway     %s\n"
                "...new gateway %s\n", cont, tmpSer, ip1, ip2, sn1, sn2, gt1, gt2);


        /*
         * If the serial number is 0 (unused) we can skip the rest
         * of this loop and start over with the next record.
         */
        if (tmpSer == 0)
        {
            dprintf(DPRINTF_SHUTDOWN_CHECK_IP, "ShutdownChangeIPAdresses - Serial number is 0 skipping rest\n");

            continue;
        }

        /*
         * Check the ip address to see if it has changed.  If it has,
         * set change to TRUE to indicate thare has been a change in
         * this controllers record.  Also set the temp ip address to
         * the new one. If the ip address has not been changed, set
         * the temp ip address to the old one.
         */
        if (((contMap->cntlConfigInfo[cont].ipEthernetAddress !=
              contMap->cntlConfigInfo[cont].newIpEthernetAddress) &&
             (contMap->cntlConfigInfo[cont].newIpEthernetAddress != 0)))
        {
            tmpIp = contMap->cntlConfigInfo[cont].newIpEthernetAddress;
            change = TRUE;
        }
        else
        {
            tmpIp = contMap->cntlConfigInfo[cont].ipEthernetAddress;
        }

        /*
         * Check the subnet mask to see if it has changed.  If it has,
         * set change to TRUE to indicate thare has been a change in
         * this controllers record.  Also set the temp subnet mask to
         * the new one. If the subnet mask has not been changed, set
         * the temp subnet mask to the old one.
         */
        if (((contMap->cntlConfigInfo[cont].subnetMask !=
              contMap->cntlConfigInfo[cont].newSubnetMask) &&
             (contMap->cntlConfigInfo[cont].newSubnetMask != 0)))
        {
            tmpSnm = contMap->cntlConfigInfo[cont].newSubnetMask;
            change = TRUE;
        }
        else
        {
            tmpSnm = contMap->cntlConfigInfo[cont].subnetMask;
        }

        /*
         * Check the gateway address to see if it has changed.  If it has,
         * set change to TRUE to indicate thare has been a change in
         * this controllers record.  Also set the temp gateway address to
         * the new one. If the gateway address has not been changed, set
         * the temp gateway address to the old one.
         */
        if (contMap->cntlConfigInfo[cont].gatewayAddress !=
            contMap->cntlConfigInfo[cont].newGatewayAddress)
        {
            tmpGw = contMap->cntlConfigInfo[cont].newGatewayAddress;
            change = TRUE;
        }
        else
        {
            tmpGw = contMap->cntlConfigInfo[cont].gatewayAddress;
        }

        /*
         * If we need to make a change to this record and it is not our
         * record, we will send the change to the controller via IPC.  If
         * the controller succesfully changes its local addresses, then
         * we will save the changes in the controller config map.  If we
         * can not successfully save the record we will send another
         * message to the controller and tell it to change its addresses
         * back to what they were originally.
         */
        if (change == TRUE)
        {
            /*
             * If this change is not for us, send it to the controller
             * whom the change belongs to, so they can update their
             * local settings.
             */
            if (GetMyControllerSN() != tmpSer)
            {
                dprintf(DPRINTF_SHUTDOWN_CHECK_IP, "ShutdownChangeIPAdresses - sending ipc request\n");

                rc = SetIpAddressTunnel(tmpSer, tmpIp, tmpSnm, tmpGw);

                dprintf(DPRINTF_SHUTDOWN_CHECK_IP, "ShutdownChangeIPAdresses - ipc returned %d\n", rc);
            }

            /*
             * Else they are our changes, so change them.
             */
            else
            {
                SetIPAddress(tmpIp);
                SetSubnetMask(tmpSnm);
                SetGatewayAddress(tmpGw);
            }

            /*
             * If there was an error flag it.
             */
            if (rc != PI_GOOD)
            {
                /*
                 * Flag the local error.
                 */
                localrc = PI_ERROR;
                break;
            }

            /*
             * If the controller successfully changed their addresses,
             * then we can save them to the controller config map.
             * Save the old addresses first in case something goes
             * wrong and we need to revert back to them.
             */
            if (rc == PI_GOOD)
            {
                dprintf(DPRINTF_SHUTDOWN_CHECK_IP, "ShutdownChangeIPAdresses - need to change master config\n");
                dprintf(DPRINTF_SHUTDOWN_CHECK_IP, "ShutdownChangeIPAdresses - Saving old data\n");
                dprintf(DPRINTF_SHUTDOWN_CHECK_IP, "ShutdownChangeIPAdresses - writing new data\n");

                tmpIp = contMap->cntlConfigInfo[cont].ipEthernetAddress;

                contMap->cntlConfigInfo[cont].ipEthernetAddress = contMap->cntlConfigInfo[cont].newIpEthernetAddress;

                tmpSnm = contMap->cntlConfigInfo[cont].subnetMask;

                contMap->cntlConfigInfo[cont].subnetMask = contMap->cntlConfigInfo[cont].newSubnetMask;

                tmpGw = contMap->cntlConfigInfo[cont].gatewayAddress;

                contMap->cntlConfigInfo[cont].gatewayAddress = contMap->cntlConfigInfo[cont].newGatewayAddress;

                /*
                 * Save the changes to the quorum.
                 */
                if (WriteControllerMap(&cntlConfigMap) != 0)
                {
                    dprintf(DPRINTF_SHUTDOWN_CHECK_IP, "ShutdownChangeIPAdresses - Error writing new data\n");

                    rc = PI_ERROR;
                }

                /*
                 * If something went wrong, we need to try and revert
                 * the changes.  Send the controller back its old
                 * addresses, so it can update them locally
                 */
                if (rc == PI_ERROR)
                {
                    /*
                     * Flag the local error.
                     */
                    localrc = PI_ERROR;

                    /*
                     * If it is not us who has to make changes, send
                     * the changes to the affected controller
                     */
                    if (GetMyControllerSN() != tmpSer)
                    {
                        dprintf(DPRINTF_SHUTDOWN_CHECK_IP, "ShutdownChangeIPAdresses - Sending old data back to controller\n");

                        SetIpAddressTunnel(tmpSer, tmpIp, tmpSnm, tmpGw);

                        dprintf(DPRINTF_SHUTDOWN_CHECK_IP, "ShutdownChangeIPAdresses - ipc returned %d\n", rc);
                    }

                    /*
                     * Else change our own addresses back to their
                     * original state
                     */
                    else
                    {
                        dprintf(DPRINTF_SHUTDOWN_CHECK_IP, "ShutdownChangeIPAdresses - Resetting our old data\n");

                        SetIPAddress(tmpIp);
                        SetSubnetMask(tmpSnm);
                        SetGatewayAddress(tmpGw);
                    }

                    break;
                }
            }
        }
    }

    if ((rc != PI_GOOD) || (localrc == PI_ERROR))
    {
        rc = PI_ERROR;
    }

    dprintf(DPRINTF_SHUTDOWN_CHECK_IP, "ShutdownChangeIPAdresses - EXIT\n");

    return rc;
}

/*----------------------------------------------------------------------------
** Function:    SetIpAddressTunnel
**
** Description: Tunnels the set Ip address to Controller serNum
**
** Inputs:      UINT32 serNum   - Serial number to send change request to
**              UINT32 ip       - new ip address
**              UINT32 snm      - new subnet mask
**              UINT32 gw       - new gateway address
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
static INT32 SetIpAddressTunnel(UINT32 serNum, UINT32 ip, UINT32 snm, UINT32 gw)
{
    IPC_PACKET *pPacket = NULL;
    PI_PACKET_HEADER *pHdr = NULL;
    PI_SET_IP_REQ *pReq = NULL;
    IPC_PACKET  rx;
    PATH_TYPE   pathType;
    UINT32      packetSize;
    UINT32      rc = GOOD;
    char        ip1[20];
    char        ip2[20];
    char        ip3[20];
    UINT8       retries = 2;                /* Ethernet, Fiber(1), Disk Quorum(2) */

    rx.header = NULL;
    rx.data = NULL;

    /*
     * Create the IPC packet.  CreatePacket() creates and fills in the
     * header portion of the IPC packet and allocates memory for the
     * data portion.  The tunneled packet (header + request) goes in
     * the data portion of the IPC packet.
     */
    packetSize = sizeof(IPC_TUNNEL) + sizeof(*pHdr) + sizeof(*pReq);

    pPacket = CreatePacket(PACKET_IPC_TUNNEL, packetSize, __FILE__, __LINE__);

    /*
     * Get pointers to the header and request portion of the tunnelled
     * packet and fill in the required information.
     */
    pHdr = (PI_PACKET_HEADER *)(pPacket->data->tunnel.packet);
    pReq = (PI_SET_IP_REQ *)(pPacket->data->tunnel.packet + sizeof(*pHdr));

    /*
     * Set the Packet Header
     */
    pHdr->commandCode = PI_ADMIN_SET_IP_CMD;
    pHdr->length = sizeof(*pReq);
    pHdr->packetVersion = 1;

    pReq->serialNum = serNum;
    pReq->ipAddress = ip;
    pReq->subnetMask = snm;
    pReq->gatewayAddress = gw;

    /*
     * Make the IPC call to tunnel a packet to the master.
     */
    pPacket->data->tunnel.status = 0;

    InetToAscii(pReq->ipAddress, ip1);
    InetToAscii(pReq->subnetMask, ip2);
    InetToAscii(pReq->gatewayAddress, ip3);

    dprintf(DPRINTF_DEFAULT, "Tunneling setIpAddress to...Serial: %d, IP: %s, Subnet: %s, Gateway: %s\n",
            serNum, ip1, ip2, ip3);

#ifdef HISTORY_KEEP
CT_history_printf("%s:%u:%s call IpcSendPacketBySN with rxPacket of %p\n", __FILE__, __LINE__, __func__, &rx);
#endif  /* HISTORY_KEEP */

    do
    {
        Free(rx.data);

        /* Sending packet to the other controller using any IPC path possible */
        pathType = IpcSendPacketBySN(serNum, SENDPACKET_ANY_PATH,
                                     pPacket, &rx, NULL, NULL, NULL, TMO_SEND_IPC_SET_IP);
    } while (pathType == SENDPACKET_NO_PATH && (retries--) > 0);

    if (!IpcSuccessfulXfer(pathType))
    {
        dprintf(DPRINTF_DEFAULT, "Tunnel setIpAddress to %d - ERROR: Send packet failed (%u).\n",
                pReq->serialNum, pathType);

        rc = ERROR;
    }
    else
    {
        IPC_TUNNEL *pTunnel = (IPC_TUNNEL *)rx.data;

        if (pTunnel->status == IPC_COMMAND_SUCCESSFUL)
        {
            /*
             * Even if the packet was successfully sent it doesn't mean
             * that the command was executed successfully.  We need
             * to get the PACKET INTERFACE information and determine
             * how the command completed.
             */
            PI_PACKET_HEADER *pHeader = (PI_PACKET_HEADER *)pTunnel->packet;

            if (pHeader->status != PI_GOOD)
            {
                dprintf(DPRINTF_DEFAULT, "Tunnel setIpAddress to %d - ERROR: Tunnel event failed (pHeader->status: 0x%x, pHeader->errorCode: 0x%x).\n",
                        pReq->serialNum, pHeader->status, pHeader->errorCode);

                rc = ERROR;
            }
        }
        else
        {
            dprintf(DPRINTF_DEFAULT, "Tunnel setIpAddress to master - ERROR: Failed to send tunnel packet (pTunnel->status: 0x%x).\n",
                    pTunnel->status);

            rc = ERROR;
        }
    }

    /*
     * Free all memory allocations.
     */
    FreePacket(&pPacket, __FILE__, __LINE__);               /* From CreatePacket() */
    FreePacketStaticPacketPointer(&rx, __FILE__, __LINE__); /* Allocated inside IPC code */

    return rc;
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
