/* $Id: cps_init.c 156532 2011-06-24 21:09:44Z m4 $ */
/*============================================================================
** FILE NAME:       cps_init.c
** MODULE TITLE:    CCB Initialization
**
** Copyright (c) 2001-2010 XIOtech Corporation. All rights reserved.
**==========================================================================*/
#include "cps_init.h"

#include "CacheManager.h"
#include "CacheMisc.h"
#include "CmdLayers.h"
#include "CachePDisk.h"
#include "convert.h"
#include "debug_files.h"
#include "LOG_Defs.h"
#include "EL.h"
#include "FW_Header.h"
#include "ipc_listener.h"
#include "ipc_sendpacket.h"
#include "ipc_cmd_dispatcher.h"
#include "kernel.h"
#include "logging.h"
#include "misc.h"
#include "mode.h"
#include "MR_Defs.h"
#include "mutex.h"
#include "nvram.h"
#include "PacketInterface.h"
#include "PI_ClientPersistent.h"
#include "PI_Utils.h"
#include "PI_WCache.h"
#include "quorum.h"
#include "quorum_utils.h"
#include "rm.h"
#include "RMCmdHdl.h"
#include "RM_Raids.h"
#include "rm_val.h"
#include "rtc.h"
#include "ses.h"
#include "SES_Structs.h"
#include "sm.h"
#include "Snapshot.h"
#include "X1_AsyncEventHandler.h"
#include "X1_Packets.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"

#include <byteswap.h>


/*****************************************************************************
** Private defines
*****************************************************************************/
#define CPSINIT_CONTROLLER_WAIT         1000

/*
** CPSINIT_WFE_TMO is the Wait For Ethernet Timeout.  The value represents
** how many seconds to wait for a successful PING to a controller.
*/
#define CPSINIT_WFE_TMO                 90

/*
** CPSINIT_WFC_TMO is the Wait For Controllers Timeout.  The value represents
** how many loops of 1 second waits are done when waiting for the rest of the
** controllers in the VCG to power up.  At the current value of 600 the timeout
** is 10 minutes.
*/
#define CPSINIT_WFC_TMO                 600

/*
** TMO_GET_OWNED_DRIVE_COUNT is get owned drive count timeout.  The value
** represents how many loops of 1 second waits are done when waiting for
** the PROC to return a valid value or an error other than NOT READY.  At
** the current value of 180 the timeout is 3 minutes.
*/
#define TMO_GET_OWNED_DRIVE_COUNT       180

/*
** IPC_PING_RETRY_COUNT is the number of times to retry the ping operation
** before considering the request and error.
*/
#define IPC_PING_RETRY_COUNT            3

/*****************************************************************************
** Private variables
*****************************************************************************/

/**
**  Flag used to determine how far the power-up has completed.
**/
static UINT16 PowerUpState = POWER_UP_UNKNOWN;

/**
**  Flag used for additonal power-up information.
**/
static UINT16 PowerUpAStatus = POWER_UP_ASTATUS_UNKNOWN;

/**
**  Flag used to determine if SES Discovery has completed.
**/
static bool bDiscoveryComplete = false;

/**
**  Flag used to tell power-up that the user has specified to continue power-up
**  without owning drives.
**/
static bool bContinueWithoutDrives = false;

/**
**  Initial NVRAM load condition (RESTORED, CHECKSUM - USES THE MLE EVENT CODE)
**/
static UINT16 InitialNVRAM = 0;

/**
**  Response to the corrupted BE NVRAM wait condition.
**/
static UINT8 CorruptBENVRAMResponse = 0;

/**
**  Response to the missing controllers wait condition.
**/
static UINT8 MissingControllersResponse = 0;

/**
**  Disaster recover power-up response
**/
static UINT8 DisasterResponse = 0;

/**
**  Response to the missing disk bay wait condition.
**/
static UINT8 MissingDiskBayResponse = 0;

/**
**  Cache Error Event
**/
static UINT32 gCacheErrorEvent = 0;

/*****************************************************************************
** Public variables - externed in the header file
*****************************************************************************/
char        gFWSystemRelease[5];
char        gFWInternalRelease[5];

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static void CPSInitMultipleController(UINT32 sserial, UINT32 cserial);
static void CPSInitUnusedController(void);
static void CPSInitFailedController(void);
static void CPSInitFWUInactiveController(void);
static void CPSInitAddController(void);
static void CPSInitInactivatedController(void);
static void CPSInitDisasterInactiveController(void);
static void CPSInitMasterController(void);
static void CPSInitWaitForEthernet(void);
static void CPSInitWaitForControllers(void);
static bool CPSInitAllControllersReady(void);
static void CPSInitWaitForBEII(UINT16 ii_status);
static void CPSInitWaitForBEReady(void);
static void CPSInitWaitForConfiguration(void);
static void CPSInitWaitForLicense(void);
static void CPSInitWaitForProcessorCommReady(void);
static void CPSInitWaitForDiscovery(void);
static void CPSInitCheckFirmwareCompatibility(void);
static void CPSInitCheckForMissingDiskBays(void);
static bool CPSInitDiskBaysMissing(void);
static void CPSInitCheckForCorruptBENVRAM(void);
static void CPSInitStartStripeResync(void);
static void CPSInitWaitForR5StripeResync(void);
static void CPSInitResetResyncRecords(void);
static void CPSInitWaitForDisasterRecovery(void);
static void CPSInitWaitForAllCacheInit(void);
static void CPSInitWaitForCacheInit(TASK_PARMS *parms);
static void CPSInitProcessCacheErrorEvent(void);
static UINT16 CPSInitGetCacheStatus(UINT32 controllerSN);
static UINT32 GetDSCX1CompatibilityIndex(void);
static UINT32 GetX1CompatibilityIndex(UINT32 controllerSN);

/*****************************************************************************
** Code Start
*****************************************************************************/

/**
******************************************************************************
**
**  @brief      Determine if the power-up sequencing has reached and passed
**              the state of starting the BE processor.
**
**  @param      none
**
**  @return     true if the power-up sequencing has passed the BE Ready
**              state, false otherwise
**
******************************************************************************
**/
bool PowerUpBEReady(void)
{
    return (PowerUpState > POWER_UP_PROCESS_BE_INIT &&
            PowerUpState != POWER_UP_INACTIVE && PowerUpState != POWER_UP_FAILED);
}


/**
******************************************************************************
**
**  @brief      Determine if all controllers have reached and passed
**              the state of starting the BE processor.
**
**  @param      none
**
**  @return     true if all controllers have passed the BE Ready
**              state, false otherwise
**
******************************************************************************
**/
bool PowerUpAllBEReady(void)
{
    return (PowerUpState >= POWER_UP_ALL_CTRL_BE_READY &&
            PowerUpState != POWER_UP_INACTIVE && PowerUpState != POWER_UP_FAILED);
}


/**
******************************************************************************
**
**  @brief      Determine if the power-up sequencing has completed.
**
**  @param      none
**
**  @return     true if the power-up sequencing has completed, false otherwise
**
******************************************************************************
**/
bool PowerUpComplete(void)
{
    return ((PowerUpState & POWER_UP_COMPLETE) == POWER_UP_COMPLETE);
}


/**
******************************************************************************
**
**  @brief      Get the power-up state.
**
**  @param      none
**
**  @return     UINT32 - Current power-up state
**
******************************************************************************
**/
UINT16 GetPowerUpState(void)
{
    return PowerUpState;
}


/**
******************************************************************************
**
**  @brief      Get the power-up state as a string.
**
**  @param      UINT8* str - Buffer in which to copy the state string.
**
**  @return     none
**
**  @attention  The output buffer must be MAX_POWER_UP_STATE_STR bytes.
**
******************************************************************************
**/
void GetPowerUpStateString(char *str)
{
    switch (GetPowerUpState())
    {
        case POWER_UP_START:
            strcpy(str, "START");
            break;

        case POWER_UP_WAIT_FWV_INCOMPATIBLE:
            strcpy(str, "FW INCOMPATIBLE");
            break;

        case POWER_UP_WAIT_PROC_COMM:
            strcpy(str, "WAIT PROC COMM");
            break;

        case POWER_UP_WAIT_CONFIGURATION:
            strcpy(str, "WAIT CONFIGURATION");
            break;

        case POWER_UP_WAIT_LICENSE:
            strcpy(str, "WAIT LICENSE");
            break;

        case POWER_UP_WAIT_DRIVES:
            strcpy(str, "WAIT DRIVES");
            break;

        case POWER_UP_WAIT_DISASTER:
            strcpy(str, "WAIT DISASTER");
            break;

        case POWER_UP_DISCOVER_CONTROLLERS:
            strcpy(str, "DISCOVER CTRLS");
            break;

        case POWER_UP_WAIT_CONTROLLERS:
            strcpy(str, "WAIT CONTROLLERS");
            break;

        case POWER_UP_PROCESS_BE_INIT:
            strcpy(str, "PROCESS BE INIT");
            break;

        case POWER_UP_PROCESS_DISCOVERY:
            strcpy(str, "PROCESS DISCOVERY");
            break;

        case POWER_UP_WAIT_DISK_BAY:
            strcpy(str, "WAIT DISK BAY");
            break;

        case POWER_UP_WAIT_CORRUPT_BE_NVRAM:
            strcpy(str, "WAIT CORRUPT NVRAM");
            break;

        case POWER_UP_ALL_CTRL_BE_READY:
            strcpy(str, "ALL CTRL BE READY");
            break;

        case POWER_UP_PROCESS_R5_RIP:
            strcpy(str, "PROCESS R5 RIP");
            break;

        case POWER_UP_SIGNAL_SLAVES_RUN_FE:
            strcpy(str, "SIG SLAVES RUN FE");
            break;

        case POWER_UP_PROCESS_CACHE_INIT:
            strcpy(str, "PROCESS CACHE INIT");
            break;

        case POWER_UP_WAIT_CACHE_ERROR:
            strcpy(str, "WAIT BUFFER ERROR");
            break;

        case POWER_UP_INACTIVE:
            strcpy(str, "INACTIVE");
            break;

        case POWER_UP_FAILED:
            strcpy(str, "FAILED");
            break;

        case POWER_UP_WRONG_SLOT:
            strcpy(str, "WRONG SLOT");
            break;

        case POWER_UP_FAILED_AUTO_NODE_CONFIG:
            strcpy(str, "FAILED AUTO NODE CONFIG");
            break;

        case POWER_UP_COMPLETE:
            strcpy(str, "COMPLETE");
            break;

        default:
            strcpy(str, "UNKNOWN");
            break;
    }
}


/**
******************************************************************************
**
**  @brief      Set the power-up state of the controller.
**
**  @param      UINT32 state - new state
**
**  @return     none
**
******************************************************************************
**/
void SetPowerUpState(UINT16 state)
{
    PowerUpState = state;

    switch (PowerUpState)
    {
        case POWER_UP_WAIT_FWV_INCOMPATIBLE:
            SendAsyncEvent(LOG_FWV_INCOMPATIBLE, 0, NULL);
            break;

        case POWER_UP_WAIT_PROC_COMM:
            SendAsyncEvent(LOG_PROC_COMM_NOT_READY, 0, NULL);
            break;

        case POWER_UP_WAIT_CONFIGURATION:
            SendAsyncEvent(LOG_NO_CONFIGURATION, 0, NULL);
            break;

        case POWER_UP_WAIT_LICENSE:
            SendAsyncEvent(LOG_NO_LICENSE, 0, NULL);
            break;

        case POWER_UP_WAIT_DRIVES:
            SendAsyncEvent(LOG_NO_OWNED_DRIVES, 0, NULL);
            break;

        case POWER_UP_WAIT_DISASTER:
            SendAsyncEvent(LOG_WAIT_DISASTER, 0, NULL);
            break;

        case POWER_UP_WAIT_CONTROLLERS:
            SendAsyncEvent(LOG_MISSING_CONTROLLER, 0, NULL);
            break;

        case POWER_UP_WAIT_DISK_BAY:
            SendAsyncEvent(LOG_MISSING_DISK_BAY, 0, NULL);
            break;

        case POWER_UP_WAIT_CACHE_ERROR:
            SendAsyncEvent(LOG_WAIT_CACHE_ERROR, 0, NULL);
            break;

        case POWER_UP_FAILED:
            SendAsyncEvent(LOG_CTRL_FAILED, 0, NULL);
            break;

        case POWER_UP_COMPLETE:
            SendAsyncEvent(LOG_POWER_UP_COMPLETE, 0, NULL);
            break;

        case POWER_UP_WRONG_SLOT:
            SendAsyncEvent(LOG_WRONG_SLOT, 0, NULL);
            break;

        case POWER_UP_FAILED_AUTO_NODE_CONFIG:
            SendAsyncEvent(LOG_BAD_CHASIS, 0, NULL);
            break;
    }
}


/**
******************************************************************************
**
**  @brief      Get the power-up additional status.
**
**  @param      none
**
**  @return     UINT16 - Current power-up additional status
**
******************************************************************************
**/
UINT16 GetPowerUpAStatus(void)
{
    return PowerUpAStatus;
}


/**
******************************************************************************
**
**  @brief      Set the power-up additional status of the controller.
**
**  @param      UINT32 state - new state
**
**  @return     none
**
******************************************************************************
**/
void SetPowerUpAStatus(UINT16 astatus)
{
    PowerUpAStatus = astatus;
}


/**
******************************************************************************
**
**  @brief      Determines if SES Discovery has complete.  The flag will be
**              set by the code in AsyncEventHandler after DiscoverSES has
**              completed.
**
**  @param      none
**
**  @return     true if SES Discovery has completed, false otherwise.
**
******************************************************************************
**/
bool DiscoveryComplete(void)
{
    return bDiscoveryComplete;
}


/**
******************************************************************************
**
**  @brief      Set the SES Discovery complete.
**
**  @param      bool bComplete - true if SES Discovery has completed,
**                               false otherwise.
**
**  @return     none
**
******************************************************************************
**/
void SetDiscoveryComplete(bool bComplete)
{
    bDiscoveryComplete = bComplete;
}


/**
******************************************************************************
**
**  @brief      Set the response for the given power-up state.
**
**  @param      UINT8 response - response to the given power-up state.
**
**  @return     none
**
******************************************************************************
**/
void SetPowerUpResponse(UINT32 state, UINT8 response)
{
    if (state == POWER_UP_WAIT_DRIVES)
    {
        if (response == POWER_UP_RESPONSE_CONTINUE)
        {
            bContinueWithoutDrives = true;
        }
    }
    else if (state == POWER_UP_WAIT_CONTROLLERS)
    {
        MissingControllersResponse = response;
    }
    else if (state == POWER_UP_WAIT_DISASTER)
    {
        DisasterResponse = response;
    }
    else if (state == POWER_UP_WAIT_DISK_BAY)
    {
        MissingDiskBayResponse = response;
    }
    else if (state == POWER_UP_WAIT_CORRUPT_BE_NVRAM)
    {
        CorruptBENVRAMResponse = response;
    }
}


/**
******************************************************************************
**
**  @brief      Set the initial NVRAM load condition (RESTORE or CHECKSUM,
**              uses the MLE events).
**
**  @param      UINT16 eventCode - Log event code that signals the load
**                                 condition.
**
**  @return     none
**
******************************************************************************
**/
void SetInitialNVRAM(UINT16 eventCode)
{
    InitialNVRAM = eventCode;
}


/**
******************************************************************************
**
**  @brief      Set the cache error event information (uses the MLE events).
**
**  @param      UINT32 event - Log event code that signals a cache error.
**
**  @return     none
**
******************************************************************************
**/
void SetCacheErrorEvent(UINT32 event)
{
    gCacheErrorEvent = event;
}


/**
******************************************************************************
**  CPSInitCheckSlot
**
**  @brief      Check that the tray is in the right slot.
**
**  @param      Controller number.
**
**  @return     none
**
******************************************************************************
**/
static void CPSInitCheckSlot(UNUSED UINT32 cn)
{
}


/**
******************************************************************************
**  CPSInitCheckLedFailed
**
**  @brief      Check if there are errors in led driver initialization.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void CPSInitCheckLedFailed(void)
{
}


/**
******************************************************************************
**
**  @brief      Initializes this controller.  Depending on what state
**              the controller is in it will initialize as a single
**              controller, master controller, or slave controller.  If
**              this controller is to be a master or slave then it will
**              continue to the multiple controller initialization routine,
**              CPSInitMultipleController.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void CPSInitController(void)
{
    UINT32      sserial;
    UINT32      cserial;
    UINT32      index1;
    UINT32      count = 0;

    /* Initialize the local image sequence map. */
    LocalImageInitSequence();

    /*
     * Make sure that the power-up complete flag is initialized to false, we
     * have not completed the power-up at this point.
     */
    SetPowerUpState(POWER_UP_START);
    SendX1ChangeEvent(X1_ASYNC_VCG_POWERUP);

    /*
     * Make sure that the continue without drives flag is initialized to false,
     * we want to make sure the user has a chance to set the flag if it becomes
     * necessary.
     */
    bContinueWithoutDrives = false;

    /*
     * Determine if FE and BE processor communications have been established.
     *
     * If there is no processor communication, we are DOA and will not
     * ever come ready...it will loop forever, rats!
     */
    if (!ProcessorCommReady(PROCESS_BE) || !ProcessorCommReady(PROCESS_FE))
    {
        CPSInitWaitForProcessorCommReady();
    }

    /* If led driver failed go to powerup bad chasis */
    CPSInitCheckLedFailed();

    /* If configuration is not yet done, wait for configuration */
    if (!IsConfigured())
    {
        CPSInitWaitForConfiguration();
    }

    /* Update the device configuration information. */
    SM_PutDevConfig();

    /* Load addresses from the static tables in the BE and FE processors. */
    LoadProcAddresses();

    /* Start the cache manager... */
    CacheMgr_Start();

    /* Set the PROC suicide settings to match what is set for the CCB. */
    SetProcSuicide();

    /* Check if the firmware versions loaded are compatible. */
    CPSInitCheckFirmwareCompatibility();

    /* Fork the IPC session manager and port listener tasks */
    TaskCreate(CreateSessionManager, NULL);
    TaskCreate(PortListener, NULL);

    /* Start the daily group validation task... */
    RM_StartDailyGroupValidation();

    /* Get the system and controller serial numbers */
    sserial = CntlSetup_GetSystemSN();
    cserial = CntlSetup_GetControllerSN();

    LogMessage(LOG_TYPE_DEBUG, "POWERUP-DSC: %u (0x%x), CN: %u (0x%x)",
               sserial, sserial, cserial, cserial);

    /* If the license has not yet been applied, wait for it to be applied.  */
    if (!IsLicenseApplied())
    {
        CPSInitWaitForLicense();
    }

    /* Check to be sure that we are in the correct slot. */
    CPSInitCheckSlot(GetCNIDFromSN(cserial));

    /*
     * Loop while the number of drives is still zero or we break out
     * of the loop because there should not be any owned drives.
     */
    while (count == 0)
    {
        /*
         * If the controller is already in the wait for drives state
         * and there is still no physical disks, just wait and then
         * check again.
         *
         * Otherwise, we can look for owned drives and do all the
         * other checks after that.
         */
        if (GetPowerUpState() == POWER_UP_WAIT_DRIVES && PhysicalDisksCount() == 0)
        {
            TaskSleepMS(5000);
            continue;
        }

        /*
         * Get the count of the physical disks that this controllers
         * sees as owned by the system serial number.
         *
         * NOTE: This will loop for 3 minutes if the BE isn't able
         *       to process the request but exit immediately on any
         *       other error.
         */
        count = CPSInitGetOwnedDriveCount(sserial);

        /*
         * If the count is zere we need to investigate a little further
         * to see if that is OK or if we need owned drives.
         */
        if (count == 0 && GetPowerUpState() != POWER_UP_WAIT_DRIVES)
        {
            /*
             * If the controller is not a replacement we will check if
             * the NVRAM says we owned drives at any time and if we
             * did not then we can continue power-up without the drives.
             */
            if (!IsReplacementController())
            {
                /*
                 * The master configuration has the number of drives this
                 * controller saw as owned.
                 */
                LoadMasterConfigFromNVRAM();

                /*
                 * Here we go, if there were not drives previously owned
                 * then we do not need to wait for the drives to become
                 * available.
                 */
                if (Qm_GetOwnedDriveCount() == 0)
                {
                    /*
                     * Break out of the while loop, we don't have drives,
                     * we didn't have drives before so we don't need to
                     * wait for them now.
                     */
                    break;
                }
            }

            /*
             * Set the wait for drives power-up state.  This will log
             * a message so the user knows that the controller is in
             * this state.
             */
            SetPowerUpState(POWER_UP_WAIT_DRIVES);

            /* Wait for 5 seconds before trying again */
            TaskSleepMS(5000);
        }
        else
        {
            /* We own */
            SetPowerUpState(POWER_UP_START);
            SendX1ChangeEvent(X1_ASYNC_VCG_POWERUP);
        }
    }

    if (count == 0)
    {
        LogMessage(LOG_TYPE_DEBUG, "POWERUP-Continue without owned drives");

        /*
         * Check if anything is set in NVRAM and if not setup the default
         * values for the master configuraiton and controller map.
         */

        /* Reset some of the values from master config */
        Qm_SetVirtualControllerSN(sserial);
        Qm_SetMasterControllerSN(cserial);
        Qm_SetIPAddress(GetIPAddress());
        Qm_SetOwnedDriveCount(0);

        /*
         * Empty the active controller map (and padding) and the controller
         * configuration map
         */
        memset(&masterConfig.activeControllerMap,
               ACM_NODE_UNDEFINED, sizeof(masterConfig.activeControllerMap));

        memset(&cntlConfigMap, 0x00, sizeof(cntlConfigMap));

        /*
         * Get the slot number for this controller by getting the
         * last nibble of the serial number.
         */
        index1 = Qm_SlotFromSerial(cserial);

        /*
         * Make sure that this controller is in the active controller map.
         * Note that the controller instance is the last nibble of the
         * controller serial number.
         */
        Qm_SetActiveCntlMap(0, index1);

        cntlConfigMap.cntlConfigInfo[index1].controllerSN = cserial;
        cntlConfigMap.cntlConfigInfo[index1].ipEthernetAddress = GetIPAddress();
        cntlConfigMap.cntlConfigInfo[index1].newIpEthernetAddress = GetIPAddress();
        cntlConfigMap.cntlConfigInfo[index1].gatewayAddress = GetGatewayAddress();
        cntlConfigMap.cntlConfigInfo[index1].newGatewayAddress = GetGatewayAddress();
        cntlConfigMap.cntlConfigInfo[index1].subnetMask = GetSubnetMask();
        cntlConfigMap.cntlConfigInfo[index1].newSubnetMask = GetSubnetMask();

        /* Save the changes to the master configuration to NVRAM. */
        StoreMasterConfigToNVRAM();

        /* Complete the power-up sequencing as a master controller. */
        CPSInitMasterController();

        /* Initializing RM */
        RM_ElectionNotify(ELECTION_AM_MASTER);
        RM_ElectionNotify(ELECTION_FINISHED);

        /* Power-up has completed so set the flag */
        SetPowerUpState(POWER_UP_COMPLETE);
    }
    else
    {
        /*
         * We found that we own drives, we should now load our master
         * configuration and controller map so we can proceed with the
         * power-up sequencing.
         */
        if (ReloadMasterConfigWithRetries() == GOOD)
        {
            LoadControllerMap();

            Qm_SetOwnedDriveCount(count);

            ccb_assert(sserial == Qm_GetVirtualControllerSN(),
                       Qm_GetVirtualControllerSN());

            for (index1 = 0; index1 < Qm_GetNumControllersAllowed(); index1++)
            {
                if (cserial == CCM_ControllerSN(index1))
                {
                    break;
                }
            }

            if (index1 == Qm_GetNumControllersAllowed())
            {
                /*
                 * There is a configuration mismatch, the controller
                 * is not found in the controller configuration map.
                 */
                CPSInitUnusedController();
            }
            else
            {
                /*
                 * Looks like everything is good to this point and we have a
                 * multiple controller configuration, or at least one that
                 * requires the completion of power-up in that manner.
                 */
                CPSInitMultipleController(sserial, cserial);
            }
        }
        else
        {
            LogMessage(LOG_TYPE_ERROR, "POWERUP-Failed to reload master config");

            CPSInitUnusedController();
        }
    }
}


/**
******************************************************************************
**
**  @brief      Initializes this controller as part of a multiple
**              controller configuration.  If the controller has
**              been failed it will remain in an inactive state.
**              Otherwise it will wait for the other controllers in
**              the group to become active (power on ready state)
**              and then proceed with an election and complete the
**              power on initialization.
**
**  @param      UINT32 sserial - system serial number
**  @param      UINT32 cserial - controller serial number
**
**  @return     none
**
******************************************************************************
**/
static void CPSInitMultipleController(UNUSED UINT32 sserial, UINT32 cserial)
{
    QM_FAILURE_DATA *qmFailureData;
    FW_COMPAT_DATA *fwCompatData;

    LogMessage(LOG_TYPE_DEBUG, "POWERUP-Multiple controllers");

    /*
     * Check for incompatible firmware
     */
    fwCompatData = MallocSharedWC(sizeof(*fwCompatData));
    if (ReadFWCompatIndexWithRetries(fwCompatData) != PI_GOOD ||
        fwCompatData->ccbHdr.fwCompatIndex != GetFirmwareCompatibilityIndex())
    {
        SetPowerUpState(POWER_UP_WAIT_FWV_INCOMPATIBLE);
        CPSInitWaitForever();
    }
    Free(fwCompatData);

    /*
     * If the controller is in a disaster mode, wait for
     * user input before continuing.
     */
    if (EL_DisasterCheckSafeguard() == ERROR)
    {
        CPSInitWaitForDisasterRecovery();
    }

    /*
     * Attempt to read the failure data from the quorum area
     * for this controller.
     */
    qmFailureData = MallocSharedWC(sizeof(*qmFailureData));
    if (ReadFailureDataWithRetries(cserial, qmFailureData) == GOOD)
    {
        if (qmFailureData->state == FD_STATE_UNUSED)
        {
            /*
             * The failure data stored on disk says that this controller
             * is not yet part of the group.
             */
            CPSInitUnusedController();
        }
        else if (qmFailureData->state == FD_STATE_FAILED ||
                 qmFailureData->state == FD_STATE_UNFAIL_CONTROLLER)
        {
            /*
             * The failure data stored on disk says that this controller
             * has been failed.  We should act accordingly.
             */
            CPSInitFailedController();
        }
        else if (qmFailureData->state == FD_STATE_FIRMWARE_UPDATE_INACTIVE ||
                 qmFailureData->state == FD_STATE_FIRMWARE_UPDATE_ACTIVE)
        {
            /*
             * The controller is in a firmware update inactive state,
             * we need to handle this power-up separately.
             */
            CPSInitFWUInactiveController();
        }
        else if (qmFailureData->state == FD_STATE_ADD_CONTROLLER_TO_VCG)
        {
            /*
             * The controller is being added to the group, we need to
             * handle this power-up separately.
             */
            CPSInitAddController();
        }
        else if (qmFailureData->state == FD_STATE_INACTIVATED)
        {
            /*
             * The controller is in an inactive state, we need to
             * handle this power-up separately.
             */
            CPSInitInactivatedController();
        }
        else if (qmFailureData->state == FD_STATE_DISASTER_INACTIVE)
        {
            /*
             * This controller is in a disaster inactive state, we need
             * to handle this power-up separately.
             */
            CPSInitDisasterInactiveController();
        }
        else
        {
            /*
             * In order to ensure a successful power-up we need to have
             * ethernet connectivity so wait for it here.
             */
            CPSInitWaitForEthernet();

            /*
             * This controller has not been failed so we need to start
             * it's power on sequencing.  This starts by setting it's
             * failure data to be FD_STATE_POR, power on ready.
             */
            memset(qmFailureData, 0x00, sizeof(*qmFailureData));
            qmFailureData->state = FD_STATE_POR;

            if (WriteFailureDataWithRetries(cserial, qmFailureData) == GOOD)
            {
                /*
                 * This controller is now in the power on ready state
                 * and needs to wait for the other controllers in the
                 * group to get to the power on ready state.
                 */
                CPSInitWaitForControllers();

                /*
                 * Who is going to be master, run election...
                 */
                if (EL_DoElection() == GOOD)
                {
                    if (TestforMaster(cserial))
                    {
                        LogMessage(LOG_TYPE_DEBUG, "POWERUP-Master controller");

                        /*
                         * This controller is the master, complete
                         * power-up as the master...
                         */
                        CPSInitMasterController();

                        /*
                         * Power-up has completed for the master so set the flag
                         */
                        SetPowerUpState(POWER_UP_COMPLETE);
                    }
                    else
                    {
                        LogMessage(LOG_TYPE_DEBUG, "POWERUP-Slave waiting for signal");

                        /*
                         * For a slave, initialization is now complete until
                         * the master signals that the BE and FE should start
                         * running.  The master does this through the IPC
                         * layer using PACKET_IPC_SIGNAL.
                         */
                    }
                }
                else
                {
                    /*
                     * The election failed, this controller is in an unusable
                     * state, make it so it is failed!
                     */
                    CPSInitFailedController();
                }
            }
            else
            {
                /*
                 * Unable to set the Power On Ready (POR) state.  This
                 * controller is effectively unused.
                 */
                CPSInitUnusedController();
            }
        }
    }
    else
    {
        LogMessage(LOG_TYPE_ERROR, "POWERUP-Failed to read failure data");

        CPSInitUnusedController();
    }
     Free(qmFailureData);

    dprintf(DPRINTF_CPSINIT, "CPSInitMultipleController: EXIT\n");
}


/**
******************************************************************************
**
**  @brief      To power-up an unused controller or to power-up a controller
**              that failed to recognize its true identity.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void CPSInitUnusedController(void)
{
    SetPowerUpState(POWER_UP_UNKNOWN);
    SendAsyncEvent(LOG_CTRL_UNUSED, 0, NULL);

    /*
     * Loop until the user resets the controller.  That is all that
     * can happen at this point.
     */
    CPSInitWaitForever();
}


/**
******************************************************************************
**
**  @brief      To power-up a controller that is failed.  This will hold
**              the controller in this state until the user decides to
**              do the unfail.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void CPSInitFailedController(void)
{
    INT32       rc = GOOD;
    UINT32      count = 0;
    QM_FAILURE_DATA *failureData;
    UINT8       retCode = false;

    SetPowerUpState(POWER_UP_FAILED);

    count = CPSInitGetOwnedDriveCount(CntlSetup_GetSystemSN());

    if (count > 0)
    {
        WriteFailureDataState(CntlSetup_GetControllerSN(), FD_STATE_FAILED);
    }

    /*
     * Invalidate the BE and FE write caches.
     */
    rc = WCacheInvalidate(CntlSetup_GetControllerSN(), PI_WCACHE_INV_OPT_BEFE);

    /*
     * We should not fail the cache invalidation from a failed controller
     * so if we do we are going to hold the controller in that state until
     * an engineer has a chance to look at it.
     */
    if (rc != PI_GOOD)
    {
        LogMessage(LOG_TYPE_DEBUG, "POWERUP-Failed to invalidate BEFE cache");
        CPSInitWaitForever();
    }

    /*
     * Clear the FE NVA Records
     */
    rc = SM_MRReset(CntlSetup_GetControllerSN(), MXNFENVA);

    /*
     * We should not fail the clearing of NVRAM Mirror records from a
     * failed controller so if we do we are going to hold the controller
     * in that state until an engineer has a chance to look at it.
     */
    if (rc != PI_GOOD)
    {
        LogMessage(LOG_TYPE_DEBUG, "POWERUP-Failed to reset FE_NVA");
        CPSInitWaitForever();
    }

    /*
     * Clear the BE NVA Records
     */
    rc = SM_MRReset(CntlSetup_GetControllerSN(), MXNBENVA);

    /*
     * We should not fail the clearing of NVRAM Mirror records from a
     * failed controller so if we do we are going to hold the controller
     * in that state until an engineer has a chance to look at it.
     */
    if (rc != PI_GOOD)
    {
        LogMessage(LOG_TYPE_DEBUG, "POWERUP-Failed to reset BE_NVA");
        CPSInitWaitForever();
    }

    /*
     * Stop IO before assigning the mirror partner
     */
    rc = StopIO(CntlSetup_GetControllerSN(),
                SM_STOP_IO_OP, SM_STOP_IO_INTENT, START_STOP_IO_USER_CCB_SM, 0);

    /*
     * We should not fail the stop IO from a failed controller
     * so if we do we are going to hold the controller in that state until
     * an engineer has a chance to look at it.
     */
    if (rc != PI_GOOD)
    {
        LogMessage(LOG_TYPE_DEBUG, "POWERUP-Failed to stop IO");
        CPSInitWaitForever();
    }

    /*
     * Set the mirror partner for this controller to itself.
     */
    retCode = AssignMirrorPartnerMRP(CntlSetup_GetControllerSN(), NULL, NULL);

    /*
     * We should not fail the assign mirror partner from a failed controller
     * so if we do we are going to hold the controller in that state until
     * an engineer has a chance to look at it.
     */
    if (!retCode)
    {
        LogMessage(LOG_TYPE_DEBUG, "POWERUP-Failed to assign mp");
        CPSInitWaitForever();
    }

    /*
     * Start IO since we have finished assigning the mirror partner
     */
    rc = StartIO(CntlSetup_GetControllerSN(),
                 START_IO_OPTION_CLEAR_ONE_STOP_COUNT, START_STOP_IO_USER_CCB_SM, 0);

    /*
     * We should not fail the start IO from a failed controller
     * so if we do we are going to hold the controller in that state until
     * an engineer has a chance to look at it.
     */
    if (rc != PI_GOOD)
    {
        LogMessage(LOG_TYPE_DEBUG, "POWERUP-Failed to start IO");
        CPSInitWaitForever();
    }

    /*
     * Put the cache in the temporarily disabled state to ensure we don't
     * cache until we have a mirror partner.
     */
    rc = SM_TempDisableCache(GetMyControllerSN(), PI_MISC_SETTDISCACHE_CMD,
                             TEMP_DISABLE_MP, 0);

    if (rc != PI_GOOD)
    {
        LogMessage(LOG_TYPE_DEBUG, "POWERUP-Failed to set temp disable cache");
        CPSInitWaitForever();
    }

    failureData = MallocSharedWC(sizeof(*failureData));
    do
    {
        /* Wait for 1 second(s) before trying again */
        TaskSleepMS(1000);

        count = CPSInitGetOwnedDriveCount(CntlSetup_GetSystemSN());

        if (count > 0)
        {
            memset(failureData, 0x00, sizeof(*failureData));
            ReadFailureData(CntlSetup_GetControllerSN(), failureData);
        }
    } while (count == 0 || GetControllerFailureState() == FD_STATE_FAILED);
    Free(failureData);

    /*
     * Controller has been unfailed.  Proceed with the following steps to
     * complete the power-up of this controller and bring it back into
     * the group:
     *      - Start the BEP
     *      - Start the FEP (do not create targets)
     *      - Run an election to bring this controller back into the group
     */
    CPSInitSlaveController(SLAVE_INIT_FULL);

    /*
     * Now that this controller is back in the group run an election
     */
    EL_DoElection();
}


/**
******************************************************************************
**
**  @brief      To power-up a controller that is in the firmware
**              inactive state (this is used during the rolling code
**              update).
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void CPSInitFWUInactiveController(void)
{
    QM_FAILURE_DATA *qmFailureData;
    UINT32      timeoutCounter = 120;
    bool        bContinue = false;
    bool        bAutomatic = false;
    bool        bEthernetGood = false;
    UINT32      compatIndex = X1_COMPATIBILITY;
    INT32       rc = PI_GOOD;
    UINT8       retCode = false;

    LogMessage(LOG_TYPE_DEBUG, "POWERUP-FW Inactive");

    SetPowerUpState(POWER_UP_INACTIVE);
    SendX1ChangeEvent(X1_ASYNC_VCG_POWERUP);

    /*
     * Set the controller into the firmware update inactive state
     * in case it came here with the active side of firmware update.
     */
    qmFailureData = MallocSharedWC(sizeof(*qmFailureData));
    memset(qmFailureData, 0x00, sizeof(*qmFailureData));
    qmFailureData->state = FD_STATE_FIRMWARE_UPDATE_INACTIVE;
    WriteFailureData(CntlSetup_GetControllerSN(), qmFailureData);

    /*
     * Invalidate the BE and FE write caches.
     */
    rc = WCacheInvalidate(CntlSetup_GetControllerSN(), PI_WCACHE_INV_OPT_BEFE);

    /*
     * We should not fail the cache invalidation from a failed controller
     * so if we do we are going to hold the controller in that state until
     * an engineer has a chance to look at it.
     */
    if (rc != PI_GOOD)
    {
        LogMessage(LOG_TYPE_DEBUG, "POWERUP-Failed to invalidate BEFE cache");
        CPSInitWaitForever();
    }

    /*
     * Clear the FE NVA Records
     */
    rc = SM_MRReset(CntlSetup_GetControllerSN(), MXNFENVA);

    /*
     * We should not fail the clearing of NVRAM Mirror records from a
     * failed controller so if we do we are going to hold the controller
     * in that state until an engineer has a chance to look at it.
     */
    if (rc != PI_GOOD)
    {
        LogMessage(LOG_TYPE_DEBUG, "POWERUP-Failed to reset FE_NVA");
        CPSInitWaitForever();
    }

    /*
     * Clear the BE NVA Records
     */
    rc = SM_MRReset(CntlSetup_GetControllerSN(), MXNBENVA);

    /*
     * We should not fail the clearing of NVRAM Mirror records from a
     * failed controller so if we do we are going to hold the controller
     * in that state until an engineer has a chance to look at it.
     */
    if (rc != PI_GOOD)
    {
        LogMessage(LOG_TYPE_DEBUG, "POWERUP-Failed to reset BE_NVA");
        CPSInitWaitForever();
    }

    /*
     * Stop IO before assigning the mirror partner
     */
    rc = StopIO(CntlSetup_GetControllerSN(), SM_STOP_IO_OP, SM_STOP_IO_INTENT,
                START_STOP_IO_USER_CCB_SM, 0);

    /*
     * We should not fail the stop IO from a failed controller
     * so if we do we are going to hold the controller in that state until
     * an engineer has a chance to look at it.
     */
    if (rc != PI_GOOD)
    {
        LogMessage(LOG_TYPE_DEBUG, "POWERUP-Failed to stop IO");
        CPSInitWaitForever();
    }

    /*
     * Set the mirror partner for this controller to itself.
     */
    retCode = AssignMirrorPartnerMRP(CntlSetup_GetControllerSN(), NULL, NULL);

    /*
     * We should not fail the assign mirror partner from a failed controller
     * so if we do we are going to hold the controller in that state until
     * an engineer has a chance to look at it.
     */
    if (!retCode)
    {
        LogMessage(LOG_TYPE_DEBUG, "POWERUP-Failed to assign mp");
        CPSInitWaitForever();
    }

    /*
     * Start IO since we have finished assigning the mirror partner
     */
    rc = StartIO(CntlSetup_GetControllerSN(), START_IO_OPTION_CLEAR_ONE_STOP_COUNT,
                 START_STOP_IO_USER_CCB_SM, 0);

    /*
     * We should not fail the start IO from a failed controller
     * so if we do we are going to hold the controller in that state until
     * an engineer has a chance to look at it.
     */
    if (rc != PI_GOOD)
    {
        LogMessage(LOG_TYPE_DEBUG, "POWERUP-Failed to start IO");
        CPSInitWaitForever();
    }

    /*
     * Put the cache in the temporarily disabled state to ensure we don't
     * cache until we have a mirror partner.
     */
    rc = SM_TempDisableCache(GetMyControllerSN(), PI_MISC_SETTDISCACHE_CMD,
                             TEMP_DISABLE_MP, 0);

    if (rc != PI_GOOD)
    {
        LogMessage(LOG_TYPE_DEBUG, "POWERUP-Failed to set temp disable cache");
        CPSInitWaitForever();
    }

    /*
     * Loop until the controller failure state changes to something
     * other than FWU Inactive or there is a client connected on the
     * 2341 port and the RMC compatilbity is LE 8.
     */
    while (FOREVER)
    {
        /* Wait for 1 second(s) before trying again */
        TaskSleepMS(1000);

        memset(qmFailureData, 0x00, sizeof(*qmFailureData));

        /*
         * Check if the failure state has changed from FWU inactve.
         */
        if (ReadFailureData(CntlSetup_GetControllerSN(), qmFailureData) == GOOD &&
            qmFailureData->state != FD_STATE_FIRMWARE_UPDATE_INACTIVE)
        {
            bContinue = true;

            /*
             * The controller state has changed to be something other
             * than FWU INACTIVE, exit the loop so the power-up
             * processing can continue.
             */
            break;
        }

        LogMessage(LOG_TYPE_DEBUG, "POWERUP-FW Inactive ping master");

        /*
         * Sending a test PING to the controller to make sure
         * that we can communicate with the master controller
         * over IPC.
         */
        rc = IpcSendPingWithRetries(Qm_GetMasterControllerSN(),
                                    SENDPACKET_ETHERNET, IPC_PING_RETRY_COUNT);

        if (rc == PI_GOOD)
        {
            if (!bEthernetGood)
            {
                bEthernetGood = true;

                /*
                 * Get the lowest X1 compatibility of all the controllers in the
                 * DSC.  This will determine the features available to the ICON
                 * and determine how the controllers will handle this state.
                 */
                compatIndex = GetDSCX1CompatibilityIndex();

                LogMessage(LOG_TYPE_DEBUG, "POWERUP-FW Inactive Compat (0x%x)",
                           compatIndex);
            }
        }
        else
        {
            LogMessage(LOG_TYPE_DEBUG, "POWERUP-Ping to master failed");

            bEthernetGood = false;
            compatIndex = X1_COMPATIBILITY;
        }

        /*
         * If the lowest compatibility index is less than 0x0F the
         * ICON will expect the controller to automatically continue
         * the power-up sequencing after the two minute delay.
         */
        if (bEthernetGood && compatIndex < 0x0F)
        {
            bAutomatic = true;
            break;
        }
    }

    /*
     * If the GUI version only supports the AUTOMATIC firmware update
     * method then continue power-up after a 2 minute delay.
     *
     * NOTE: The check on GUI version should be temporary and should be
     *       removed once compatible GUI code is guaranteed.
     */
    if (bAutomatic)
    {
        /*
         * Set the firmware update active state into the failure data so this
         * controller participates in the next election.
         */
        if (WriteFailureDataState(CntlSetup_GetControllerSN(),
                                  FD_STATE_FIRMWARE_UPDATE_ACTIVE) != GOOD)
        {
            CPSInitFailedController();
        }
        else
        {
            LogMessage(LOG_TYPE_DEBUG, "POWERUP-FW Inactive delay");

            /*
             * Start a timeout counter.  Each time through the loop
             * try and read the failure state for this controller.
             * If the state has changed from FW ACTIVE then break out
             * of the loop early.
             */
            while (timeoutCounter > 0)
            {
                memset(qmFailureData, 0x00, sizeof(*qmFailureData));

                if (ReadFailureData(CntlSetup_GetControllerSN(), qmFailureData) == GOOD &&
                    qmFailureData->state != FD_STATE_FIRMWARE_UPDATE_ACTIVE)
                {
                    dprintf(DPRINTF_CPSINIT, "CPSInitFWUInactiveController: Failure state no longer FWU Active, exiting delay early.\n");
                    break;
                }

                TaskSleepMS(1000);
                timeoutCounter--;
            }

            /*
             * If the timeout counter expired it means the controller
             * still needs to be powered up as a FW ACTIVE system.
             */
            if (timeoutCounter == 0)
            {
                bContinue = true;
            }
        }
    }
    Free(qmFailureData);

    /*
     * Whether the GUI supports the new firmware update inactive processing
     * or not, the controller will eventually need to decide whether or not
     * to continue power-up sequencing.  If the continue flag has been set
     * then initalize the controller as a slave and continue power-up from
     * that point.  If not then skip the power-up sequencing and see what
     * happens from there.
     *
     * NOTE: The check on bContinue should be temporary and should be
     *       removed once compatible GUI code is guaranteed.
     */
    if (bContinue)
    {
        /*
         * Controller is current inactive.  Proceed with the
         * following steps to complete the power-up of this
         * controller and bring it back into the group:
         *      - Start the BEP
         *      - Start the FEP (do not create targets)
         *      - Run an election to bring this controller back
         *        into the group
         */
        LogMessage(LOG_TYPE_DEBUG, "POWERUP-FW Inactive continue");
        CPSInitSlaveController(SLAVE_INIT_FULL);

        /*
         * Now that this controller is back in the group run an election
         */
        EL_DoElection();
    }

    LogMessage(LOG_TYPE_DEBUG, "POWERUP-FW Inactive complete");
}


/**
******************************************************************************
**
**  @brief      To power-up a controller that is being added to a CNC.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void CPSInitAddController(void)
{
    INT32       rc;
    UINT8       retCode = false;

    dprintf(DPRINTF_CPSINIT, "CPSInitAddController: ENTER\n");

    /*
     * Invalidate the BE and FE write caches.
     */
    rc = WCacheInvalidate(CntlSetup_GetControllerSN(), PI_WCACHE_INV_OPT_BEFE);

    /*
     * We should not fail the cache invalidation from a failed controller
     * so if we do we are going to hold the controller in that state until
     * an engineer has a chance to look at it.
     */
    if (rc != PI_GOOD)
    {
        LogMessage(LOG_TYPE_DEBUG, "POWERUP-Failed to invalidate BEFE cache");
        CPSInitWaitForever();
    }

    /*
     * Clear the FE NVA Records
     */
    rc = SM_MRReset(CntlSetup_GetControllerSN(), MXNFENVA);

    /*
     * We should not fail the clearing of NVRAM Mirror records from a
     * failed controller so if we do we are going to hold the controller
     * in that state until an engineer has a chance to look at it.
     */
    if (rc != PI_GOOD)
    {
        LogMessage(LOG_TYPE_DEBUG, "POWERUP-Failed to reset FE_NVA");
        CPSInitWaitForever();
    }

    /*
     * Clear the BE NVA Records
     */
    rc = SM_MRReset(CntlSetup_GetControllerSN(), MXNBENVA);

    /*
     * We should not fail the clearing of NVRAM Mirror records from a
     * failed controller so if we do we are going to hold the controller
     * in that state until an engineer has a chance to look at it.
     */
    if (rc != PI_GOOD)
    {
        LogMessage(LOG_TYPE_DEBUG, "POWERUP-Failed to reset BE_NVA");
        CPSInitWaitForever();
    }

    /*
     * Stop IO before assigning the mirror partner
     */
    rc = StopIO(CntlSetup_GetControllerSN(), SM_STOP_IO_OP, SM_STOP_IO_INTENT,
                START_STOP_IO_USER_CCB_SM, 0);
    /*
     * We should not fail the stop IO from a failed controller
     * so if we do we are going to hold the controller in that state until
     * an engineer has a chance to look at it.
     */
    if (rc != PI_GOOD)
    {
        LogMessage(LOG_TYPE_DEBUG, "POWERUP-Failed to stop IO");
        CPSInitWaitForever();
    }

    /*
     * Set the mirror partner for this controller to itself.
     */
    retCode = AssignMirrorPartnerMRP(CntlSetup_GetControllerSN(), NULL, NULL);

    /*
     * We should not fail the assign mirror partner from a failed controller
     * so if we do we are going to hold the controller in that state until
     * an engineer has a chance to look at it.
     */
    if (!retCode)
    {
        LogMessage(LOG_TYPE_DEBUG, "POWERUP-Failed to assign mp");
        CPSInitWaitForever();
    }

    /*
     * Start IO since we have finished assigning the mirror partner
     */
    rc = StartIO(CntlSetup_GetControllerSN(), START_IO_OPTION_CLEAR_ONE_STOP_COUNT,
                 START_STOP_IO_USER_CCB_SM, 0);

    /*
     * We should not fail the start IO from a failed controller
     * so if we do we are going to hold the controller in that state until
     * an engineer has a chance to look at it.
     */
    if (rc != PI_GOOD)
    {
        LogMessage(LOG_TYPE_DEBUG, "POWERUP-Failed to start IO");
        CPSInitWaitForever();
    }

    /*
     * Put the cache in the temporarily disabled state to ensure we don't
     * cache until we have a mirror partner.
     */
    rc = SM_TempDisableCache(GetMyControllerSN(), PI_MISC_SETTDISCACHE_CMD,
                             TEMP_DISABLE_MP, 0);

    if (rc != PI_GOOD)
    {
        LogMessage(LOG_TYPE_DEBUG, "POWERUP-Failed to set temp disable cache");
        CPSInitWaitForever();
    }

    /*
     * The controller is being added to the group.  Proceed
     * with the following steps to complete the power-up of
     * this controller and bring it into the group:
     *      - Start the BEP
     *      - Start the FEP (do not create targets)
     *      - Run an election to bring this controller back into the group
     */
    CPSInitSlaveController(SLAVE_INIT_FULL);

    /*
     * Now that this controller is back in the group run an election
     */
    EL_DoElection();
}


/**
******************************************************************************
**
**  @brief      To power-up a controller that has been inactivated.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void CPSInitInactivatedController(void)
{
    INT32       rc;
    UINT32      count = 0;
    QM_FAILURE_DATA *failureData;
    UINT8       retCode = false;

    LogMessage(LOG_TYPE_DEBUG, "POWERUP-Inactivated");

    SetPowerUpState(POWER_UP_INACTIVE);
    SendX1ChangeEvent(X1_ASYNC_VCG_POWERUP);

    /*
     * Invalidate the BE and FE write caches.
     */
    rc = WCacheInvalidate(CntlSetup_GetControllerSN(), PI_WCACHE_INV_OPT_BEFE);

    /*
     * We should not fail the cache invalidation from a failed controller
     * so if we do we are going to hold the controller in that state until
     * an engineer has a chance to look at it.
     */
    if (rc != PI_GOOD)
    {
        LogMessage(LOG_TYPE_DEBUG, "POWERUP-Failed to invalidate BEFE cache");
        CPSInitWaitForever();
    }

    /*
     * Clear the FE NVA Records
     */
    rc = SM_MRReset(CntlSetup_GetControllerSN(), MXNFENVA);

    /*
     * We should not fail the clearing of NVRAM Mirror records from a
     * failed controller so if we do we are going to hold the controller
     * in that state until an engineer has a chance to look at it.
     */
    if (rc != PI_GOOD)
    {
        LogMessage(LOG_TYPE_DEBUG, "POWERUP-Failed to reset FE_NVA");
        CPSInitWaitForever();
    }

    /*
     * Clear the BE NVA Records
     */
    rc = SM_MRReset(CntlSetup_GetControllerSN(), MXNBENVA);

    /*
     * We should not fail the clearing of NVRAM Mirror records from a
     * failed controller so if we do we are going to hold the controller
     * in that state until an engineer has a chance to look at it.
     */
    if (rc != PI_GOOD)
    {
        LogMessage(LOG_TYPE_DEBUG, "POWERUP-Failed to reset BE_NVA");
        CPSInitWaitForever();
    }

    /*
     * Stop IO before assigning the mirror partner
     */
    rc = StopIO(CntlSetup_GetControllerSN(), SM_STOP_IO_OP, SM_STOP_IO_INTENT,
                START_STOP_IO_USER_CCB_SM, 0);

    /*
     * We should not fail the stop IO from a failed controller
     * so if we do we are going to hold the controller in that state until
     * an engineer has a chance to look at it.
     */
    if (rc != PI_GOOD)
    {
        LogMessage(LOG_TYPE_DEBUG, "POWERUP-Failed to stop IO");
        CPSInitWaitForever();
    }

    /*
     * Set the mirror partner for this controller to itself.
     */
    retCode = AssignMirrorPartnerMRP(CntlSetup_GetControllerSN(), NULL, NULL);

    /*
     * We should not fail the assign mirror partner from a failed controller
     * so if we do we are going to hold the controller in that state until
     * an engineer has a chance to look at it.
     */
    if (!retCode)
    {
        LogMessage(LOG_TYPE_DEBUG, "POWERUP-Failed to assign mp");
        CPSInitWaitForever();
    }

    /*
     * Start IO since we have finished assigning the mirror partner
     */
    rc = StartIO(CntlSetup_GetControllerSN(), START_IO_OPTION_CLEAR_ONE_STOP_COUNT,
                 START_STOP_IO_USER_CCB_SM, 0);

    /*
     * We should not fail the start IO from a failed controller
     * so if we do we are going to hold the controller in that state until
     * an engineer has a chance to look at it.
     */
    if (rc != PI_GOOD)
    {
        LogMessage(LOG_TYPE_DEBUG, "POWERUP-Failed to start IO");
        CPSInitWaitForever();
    }

    /*
     * Put the cache in the temporarily disabled state to ensure we don't
     * cache until we have a mirror partner.
     */
    rc = SM_TempDisableCache(GetMyControllerSN(), PI_MISC_SETTDISCACHE_CMD,
                             TEMP_DISABLE_MP, 0);

    if (rc != PI_GOOD)
    {
        LogMessage(LOG_TYPE_DEBUG, "POWERUP-Failed to set temp disable cache");
        CPSInitWaitForever();
    }

    failureData = MallocSharedWC(sizeof(*failureData));
    do
    {
        /* Wait for 5 seconds before trying again */
        TaskSleepMS(1000);

        count = CPSInitGetOwnedDriveCount(CntlSetup_GetSystemSN());

        if (count > 0)
        {
            memset(failureData, 0x00, sizeof(*failureData));
            ReadFailureData(CntlSetup_GetControllerSN(), failureData);
        }
    } while (count == 0 || failureData->state == FD_STATE_INACTIVATED);
    Free(failureData);

    /*
     * Controller has been unfailed.  Proceed with the following steps to
     * complete the power-up of this controller and bring it back into
     * the group:
     *      - Start the BEP
     *      - Start the FEP (do not create targets)
     *      - Run an election to bring this controller back into the group
     */
    CPSInitSlaveController(SLAVE_INIT_FULL);

    /*
     * Now that this controller is back in the group run an election
     */
    EL_DoElection();

    dprintf(DPRINTF_CPSINIT, "CPSInitInactivatedController: EXIT\n");
}


/**
******************************************************************************
**
**  @brief      To power-up a controller that has been disaster inactivated.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void CPSInitDisasterInactiveController(void)
{
    LogMessage(LOG_TYPE_DEBUG, "POWERUP-Inactivated");

    SetPowerUpState(POWER_UP_INACTIVE);
    SendX1ChangeEvent(X1_ASYNC_VCG_POWERUP);

    /*
     * Change the controllers state to active and let it power-up as
     * a slave controller.
     */
    WriteFailureDataState(CntlSetup_GetControllerSN(), FD_STATE_ACTIVATE);

    /*
     * Controller has been unfailed.  Proceed with the following steps to
     * complete the power-up of this controller and bring it back into
     * the group:
     *      - Start the BEP
     *      - Start the FEP (do not create targets)
     *      - Run an election to bring this controller back into the group
     */
    CPSInitSlaveController(SLAVE_INIT_FULL);

    /*
     * Now that this controller is back in the group run an election
     */
    EL_DoElection();

    dprintf(DPRINTF_CPSINIT, "CPSInitDisasterInactiveController: EXIT\n");
}


/**
******************************************************************************
**
**  @brief      Initializes this controller as the master controller.  As
**              the master it must start the master logging and signal the
**              slaves to start both their BE and FE processing.  Once
**              everyone is up and running it will also send a signal to
**              the slaves to update their configuration.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void CPSInitMasterController(void)
{
    /*
     * Initialize the IP Address for DLM purposes.  This sends the IP Address
     * to the BEP.
     */
    InitIPAddress(CntlSetup_GetIPAddress());

    /*
     * Tell the BEP that this controller will be the master controller.
     */
    MRP_Awake(MRAWAKE_MASTER);

    if (IsReplacementController())
    {
        LogMessage(LOG_TYPE_DEBUG, "POWERUP-NVRAMRDY with REPLACEMENT");

        /*
         * Tell the BEP that NVRAM is ready to be used and it's power-up can
         * continue but make sure it uses the file system copy of the
         * NVRAM as this is a replacement controller.
         */
        MRP_Awake(MRAWAKE_NVRAMRDY | MRAWAKE_REPLACEMENT);
    }
    else
    {
        LogMessage(LOG_TYPE_DEBUG, "POWERUP-NVRAMRDY");

        /*
         * Tell the BEP that NVRAM is ready to be used and it's power-up can
         * continue.
         */
        MRP_Awake(MRAWAKE_NVRAMRDY);
    }

    /*
     * Reset the SES in discovery flag now.  We initialized it to 1 so a
     * discovery could not take place until this point.
     */
    DiscoveringSES = 0;

    /*
     * Wait for the BEP to become ready...
     */
    CPSInitWaitForBEReady();

    /*
     * Initialize the cached mirror partner serial number
     */
    InitCachedMirrorPartnerSN();

    LogMessage(LOG_TYPE_DEBUG, "POWERUP-Cached MP SN: 0x%x", GetCachedMirrorPartnerSN());

    /*
     * Now that we have told the PROC this controller is the master
     * make sure that we set the system serial number which will
     * get written to NVRAM.
     */
    UpdateProcSerialNumber(CONTROLLER_SN, CntlSetup_GetControllerSN());

    /*
     * Create the maximum number of controller allowed.
     */
    CreateController(Qm_GetNumControllersAllowed());

    /*
     * Check for corrupted BE NVRAM.
     */
    CPSInitCheckForCorruptBENVRAM();

    /*
     * Wait for the SES Discovery to complete.
     */
    CPSInitWaitForDiscovery();

    /*
     * Check if any disk bays are missing.
     */
    CPSInitCheckForMissingDiskBays();

    /*
     * Signal Slaves to run the BE
     */
    LogMessage(LOG_TYPE_DEBUG, "POWERUP-Signal slaves to run BE");
    IpcSignalSlaves(IPC_SIGNAL_RUN_BE);

    /*
     * Signal Slaves to run the FE
     */
    LogMessage(LOG_TYPE_DEBUG, "POWERUP-Signal slaves to go to P2INIT");
    IpcSignalSlaves(IPC_SIGNAL_RUN_P2INIT);

    /*
     * Signal the BE to go to P2INIT (this starts the FE also).
     */
    LogMessage(LOG_TYPE_DEBUG, "POWERUP-Signal BE to go to P2INIT");
    MRP_Awake(MRAWAKE_MASTER | MRAWAKE_P2INIT);

    /*
     * Signal Slaves that all controllers are BE ready
     */
    LogMessage(LOG_TYPE_DEBUG, "POWERUP-Signal slaves all BE ready");
    IpcSignalSlaves(IPC_SIGNAL_POWERUP_BE_READY);

    /*
     * Set master controller power up state to indicate that all
     * controller have reached BE ready.
     */
    SetPowerUpState(POWER_UP_ALL_CTRL_BE_READY);
    SendX1ChangeEvent(X1_ASYNC_VCG_POWERUP);

    /*
     * Start stripe resync operations where needed.
     */
    CPSInitStartStripeResync();

    /*
     * Wait for any outstanding stripe resync operations to complete.
     */
    CPSInitWaitForR5StripeResync();

    /*
     * Reset the FE NVA Records
     */
    CPSInitResetResyncRecords();

    /*
     * Send the latest persistent data to other controller
     * to get the updated list of records.
     */
    dprintf(DPRINTF_DEFAULT, "  Persistent data resync started\n");
    SyncClientData();

    /*
     * If this is a replacement controller run the RAID5 replacement
     * controller power-up processing.
     */
    if (IsReplacementController())
    {
        RM_R5PowerupReplacement();

        /*
         * Make sure the replacement controller flag is cleared, it isn't
         * needed any longer.
         */
        ClearReplacementController();
    }

    /*
     * Capture startup configuration at this point
     */
    TakeSnapshot(SNAPSHOT_TYPE_POWERUP, "AUTO - Powerup");

    /*
     * Set the master controller power up state to indicate that
     * it is starting to signal the slave to run their FE.
     */
    SetPowerUpState(POWER_UP_SIGNAL_SLAVES_RUN_FE);
    SendX1ChangeEvent(X1_ASYNC_VCG_POWERUP);

    /*
     * Signal Slaves to run the FE
     */
    LogMessage(LOG_TYPE_DEBUG, "POWERUP-Signal slaves to run FE");
    IpcSignalSlaves(IPC_SIGNAL_RUN_FE);

    /*
     * Signal the BE to go to P2INIT (this starts the FE also).
     */
    LogMessage(LOG_TYPE_DEBUG, "POWERUP-Signal FE to run");
    MRP_Awake(MRAWAKE_MASTER | MRAWAKE_FEINIT);

    /*
     * Wait for the cache to be initialized on all controllers
     */
    CPSInitWaitForAllCacheInit();

    /*
     * Signal Slaves to go to power-up complete
     */
    LogMessage(LOG_TYPE_DEBUG, "POWERUP-Signal slaves power-up complete");
    IpcSignalSlaves(IPC_SIGNAL_POWERUP_COMPLETE);

    /*
     * Signal all the active slaves to retrieve the new configuration.
     */
    RMSlavesConfigurationUpdate(X1_ASYNC_CONFIG_ALL, TRUE);
}


/**
******************************************************************************
**
**  @brief      Initializes this controller as a slave.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void CPSInitSlaveController(UINT8 option)
{
    LogMessage(LOG_TYPE_DEBUG, "POWERUP-Slave controller");

    /*
     * Initialize the IP Address for DLM purposes.  This sends the IP Address
     * to the BEP.
     */
    InitIPAddress(CntlSetup_GetIPAddress());

    /*
     * Tell the BEP that this controller will be a master controller.
     */
    MRP_Awake(MRAWAKE_SLAVE);

    /*
     * Tell the BEP that NVRAM is ready to be used and it's power-up can
     * continue.
     */
    MRP_Awake(MRAWAKE_NVRAMRDY);

    /*
     * Reset the SES in discovery flag now.  We initialized it to 1 so a
     * discovery could not take place until this point.
     */
    DiscoveringSES = 0;

    /*
     * Wait for the BEP to become ready...
     */
    CPSInitWaitForBEReady();

    /*
     * Initialize the cached mirror partner serial number
     */
    InitCachedMirrorPartnerSN();

    LogMessage(LOG_TYPE_DEBUG, "POWERUP-Cached MP SN: 0x%x", GetCachedMirrorPartnerSN());

    /*
     * Wait for the SES Discovery to complete.
     */
    CPSInitWaitForDiscovery();

    /*
     * Make sure the replacement controller flag is cleared, it isn't
     * needed any longer.
     */
    ClearReplacementController();

    /*
     * If this is a full slave controller initialization send the
     * requests to bring the controller fully operational.
     */
    if (option == SLAVE_INIT_FULL)
    {
        /*
         * Signal the BE to go to P2INIT.
         */
        LogMessage(LOG_TYPE_DEBUG, "POWERUP-Signal BE to go to P2INIT");
        MRP_Awake(MRAWAKE_SLAVE | MRAWAKE_P2INIT);

        /*
         * Signal the FE to run.
         */
        LogMessage(LOG_TYPE_DEBUG, "POWERUP-Signal FE to run");
        MRP_Awake(MRAWAKE_SLAVE | MRAWAKE_FEINIT);

        /*
         * Wait for this controller to get is cache fully initialized.
         */
        CPSInitWaitForCacheInit(NULL);

        /*
         * Send the latest persistent data to other controller
         * to get the updated list of records.
         */
        dprintf(DPRINTF_DEFAULT, "  Persistent data resync started\n");
        SyncClientData();

        /*
         * Tell the FE to put the regular ports on the cards.
         */
        MRP_FEPortGo();

        /*
         * Power-up has completed so set the flag
         */
        SetPowerUpState(POWER_UP_COMPLETE);
    }
}


/**
******************************************************************************
**
**  @brief      Retrieves a count of the drives owned by the controller
**              specified by serialNum.
**
**  @param      UINT32 serialNum - serial number of drive owner
**
**  @return     0 = no drives owned by the serial number or error,
**              n = number of drives owned by the serial number.
**
******************************************************************************
**/
UINT32 CPSInitGetOwnedDriveCount(UINT32 serialNum)
{
    XIO_PACKET  reqPacket = { NULL, NULL };
    XIO_PACKET  rspPacket = { NULL, NULL };
    INT32       rc = PI_GOOD;
    UINT32      count = 0;
    UINT32      timeoutCounter = TMO_GET_OWNED_DRIVE_COUNT;

    /*
     * Allocate memory for the request (header and data) and the
     * response header. The response data will be allocated in the called
     * function.
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    reqPacket.pPacket = MallocWC(sizeof(PI_MISC_GET_DEVICE_COUNT_REQ));
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    rspPacket.pPacket = NULL;
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    /*
     * Fill in the Header
     */
    reqPacket.pHeader->commandCode = PI_MISC_GET_DEVICE_COUNT_CMD;
    reqPacket.pHeader->length = sizeof(PI_MISC_GET_DEVICE_COUNT_REQ);

    /*
     * Fill in the Data
     */
    ((PI_MISC_GET_DEVICE_COUNT_REQ *)reqPacket.pPacket)->serialNumber = serialNum;

    /*
     * Loop and call the device count MRP until we get device count
     * successfully or we encounter an error other than define not
     * ready.  The define not ready could occur if the BE receives
     * a PORT DATABASE change and then return incorrect data.
     */
    while (timeoutCounter > 0)
    {
        /*
         * Issue the command through the packet command handler
         */
        rc = PortServerCommandHandler(&reqPacket, &rspPacket);

        if (rc == PI_GOOD)
        {
            ccb_assert(rspPacket.pPacket != NULL, rspPacket.pPacket);

            count = ((PI_MISC_GET_DEVICE_COUNT_RSP *)rspPacket.pPacket)->count;

            /*
             * We found the drive count successfully, we do not need
             * to loop through again.
             */
            break;
        }
        else
        {
            /*
             * If the BE is not ready for us to make this call we should
             * free the memory and loop back around to try again.  This
             * could happen if the BE receives a PORT DATABASE change
             * and if we did not loop to try again we could get bad data
             * returned and maybe tell the call there were not drives
             * owned by this controller.
             */
            if (rspPacket.pPacket != NULL &&
                ((PI_MISC_GET_DEVICE_COUNT_RSP *)rspPacket.pPacket)->header.status == DEDEFNRDY)
            {
                if (rc != PI_TIMEOUT)
                {
                    Free(rspPacket.pPacket);
                }

                /* Wait for 1 second before trying again */
                TaskSleepMS(1000);
                timeoutCounter--;

                /*
                 * If a timeout occurred, exit since we can't wait
                 * forever like we used to.
                 */
                if (timeoutCounter == 0)
                {
                    LogMessage(LOG_TYPE_DEBUG, "POWERUP-Timeout getting owned drives");
                    break;
                }
            }
            else
            {
                /*
                 * A real error occurred, we do not want to try to
                 * get the count again, just return 0 as the device
                 * count.
                 */
                break;
            }
        }
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

    return (count);
}


/**
******************************************************************************
**
**  @brief      Loops through the current controllers and attempts to send
**              a PING request to ensure that the ethernet and IPC are
**              functioning properly.  This loop stops once one controller
**              is PING'd successfully or a timeout limit has been reached.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void CPSInitWaitForEthernet(void)
{
    bool        bReady = false;
    bool        bPingAttempted;
    UINT32      endTime;
    UINT16      index1;
    UINT32      serialNum;
    UINT32      ipaddress;
    INT32       rc;
    QM_FAILURE_DATA *qmFailureData;

    LogMessage(LOG_TYPE_DEBUG, "POWERUP-Waiting for ethernet.");

    endTime = RTC_GetSystemSeconds() + CPSINIT_WFE_TMO;

    /*
     * Loop until a controller has been sucessfully PING'd or the
     * timeout value has been reached.
     */
    qmFailureData = MallocSharedWC(sizeof(*qmFailureData));
    while (!bReady && RTC_GetSystemSeconds() < endTime)
    {
        /*
         * Every time through the loop assume that there is not a controller
         * available and that a PING has not been attempted.
         */
        bPingAttempted = false;

        /*
         * Loop through the controllers in the controller configuration
         * map.
         */
        for (index1 = 0; index1 < MAX_CONTROLLERS; ++index1)
        {
            /*
             * Get the serial number of the controller.
             */
            serialNum = CCM_ControllerSN(index1);

            /*
             * If this is not an actual controller or it is this controller
             * skip it, there is no need to attempt the PING.
             */
            if (serialNum == 0 || serialNum == GetMyControllerSN())
            {
                /*
                 * Go to the next controller in the controller
                 * configuration map.
                 */
                continue;
            }

            /*
             * Check if the controller should be available
             */
            memset(qmFailureData, 0x00, sizeof(*qmFailureData));

            /*
             * If the controller's failure state can't be read or the
             * controllers state indicates that the controller should
             * not be available, skip it and move on to the next
             * controller.
             */
            if (ReadFailureData(serialNum, qmFailureData) == GOOD)
            {
                if (qmFailureData->state != FD_STATE_OPERATIONAL &&
                    qmFailureData->state != FD_STATE_POR)
                {
                    /*
                     * Go to the next controller in the controller
                     * configuration map.
                     */
                    continue;
                }
            }
            else
            {
                /*
                 * Not being able to read the failure state for a controller
                 * is the same as attempting to PING it.  In this case it is
                 * not known whether the controller should be available or
                 * not so it is assumed to be available but not able to be
                 * PING'd yet.
                 */
                bPingAttempted = true;

                /*
                 * Go to the next controller in the controller
                 * configuration map.
                 */
                continue;
            }

            /*
             * A PING will be attempted, this means there is a controller
             * that should be available.
             */
            bPingAttempted = true;

            /*
             * Get the IP address for the controller.
             */
            ipaddress = SerialNumberToIPAddress(serialNum, SENDPACKET_ETHERNET);

            /*
             * Send the UDP ping request to the controller.
             */
            rc = GOOD;

            /*
             * If the basic ping was successful attempt to send the IPC
             * PING.
             */
            if (rc == GOOD)
            {
                /*
                 * Sending a test PING to the slave-to-be controller to
                 * make sure that we can communicate with that controller
                 * over IPC.
                 */
                rc = IpcSendPingWithRetries(serialNum,
                                            SENDPACKET_ETHERNET, IPC_PING_RETRY_COUNT);
            }

            /*
             * If the Ping(s) were successful, log a message and exit
             * out.
             */
            if (rc == GOOD)
            {
                LogMessage(LOG_TYPE_DEBUG, "POWERUP-Ethernet Good (0x%x).", serialNum);

                bReady = true;

                /*
                 * Exit out of the loop for the controller configuration
                 * map.
                 */
                break;
            }
        }

        /*
         * If there were no PING attempts, assume that ethernet is ready
         * and let power-up continue.  This means there were no controllers
         * that should have been available.
         */
        if (!bPingAttempted)
        {
            bReady = true;
        }

        /*
         * If there has not been a successful PING, sleep and decrement
         * the timeout counter.
         */
        if (!bReady)
        {
            /*
             * Wait for 1 second before trying to find the controllers again
             */
            TaskSleepMS(CPSINIT_CONTROLLER_WAIT);
        }
    }
    Free(qmFailureData);

    LogMessage(LOG_TYPE_DEBUG, "POWERUP-Ethernet %s.", (bReady ? "GOOD" : "BAD"));
}


/**
******************************************************************************
**
**  @brief      Loops through the currently active controllers until the
**              failure data for all controllers becomes power on ready
**              (FD_STATE_POR).
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void CPSInitWaitForControllers(void)
{
    bool        bReady;
    UINT32      timeoutCounter = CPSINIT_WFC_TMO;

    SetPowerUpState(POWER_UP_DISCOVER_CONTROLLERS);
    SendX1ChangeEvent(X1_ASYNC_VCG_POWERUP);

    while (timeoutCounter > 0)
    {
        bReady = CPSInitAllControllersReady();

        if (bReady)
        {
            LogMessage(LOG_TYPE_DEBUG, "POWERUP-Found all controllers.");

            /*
             * All controllers in the active controller map have changed
             * their state to power on ready.
             */
            break;
        }
        else
        {
            /*
             * Wait for 1 second before trying to find the controllers again
             */
            TaskSleepMS(CPSINIT_CONTROLLER_WAIT);
            timeoutCounter--;

            /*
             * If a timeout occurred, wait for the users response to
             * determine what to do next.
             */
            if (timeoutCounter == 0)
            {
                SetPowerUpState(POWER_UP_WAIT_CONTROLLERS);

                while (!bReady && MissingControllersResponse == 0)
                {
                    /* Wait for 1 second(s) before looping again */
                    TaskSleepMS(1000);

                    /*
                     * Check if all the controllers have come ready.
                     */
                    bReady = CPSInitAllControllersReady();
                }

                /*
                 * Reset the power-up state to DISCOVER CONTROLLERS
                 * since we are retrying the operation or continuing
                 * without the missing controller(s).
                 */
                SetPowerUpState(POWER_UP_DISCOVER_CONTROLLERS);
                SendX1ChangeEvent(X1_ASYNC_VCG_POWERUP);

                /*
                 * One of several things could have happened to exit
                 * the above loop:
                 *  - The user responded with a RETRY
                 *  - The user responded with something other than a RETRY
                 *  - The missing controllers were found
                 *
                 * In the last two cases the MissingControllersResponse
                 * value will not be RETRY so we will not reset the
                 * timer and continue with the controllers that are
                 * available.
                 */
                if (MissingControllersResponse == POWER_UP_RESPONSE_RETRY)
                {
                    LogMessage(LOG_TYPE_DEBUG, "POWERUP-Retry wait for controllers.");

                    timeoutCounter = CPSINIT_WFC_TMO;
                }
                else
                {
                    /*
                     * THE ONLY OTHER VALID POWER-UP RESPONSE IS TO
                     * CONTINUE WITHOUT THE MISSING CONTROLLERS SO
                     * IF WE DID NOT RECEIVE THE "RETRY" RESPONSE WE
                     * ARE GOING TO ASSUME THE USER WANTED TO CONTINUE.
                     */

                    if (bReady)
                    {
                        LogMessage(LOG_TYPE_DEBUG, "POWERUP-Found all controllers.");
                    }
                    else
                    {
                        LogMessage(LOG_TYPE_DEBUG, "POWERUP-Continue without controllers.");
                    }
                }

                MissingControllersResponse = 0;
            }
        }
    }

    /*
     *  Give other controllers time to recognize that everyone is here.
     */
    TaskSleepMS(2 * CPSINIT_CONTROLLER_WAIT);

    /*
     * Send a log message that we are waiting for missing
     * controllers.
     */
    SendAsyncEvent(LOG_CONTROLLERS_READY, 0, NULL);
}


/**
******************************************************************************
**
**  @brief      Loops through the currently active controllers to
**              determine if all controllers have reached the power
**              on ready (FD_STATE_POR) state.
**
**  @param      none
**
**  @return     true if all the active controllers have reached the
**              power on ready (FD_STATE_POR) state, otherwise false.
**
******************************************************************************
**/
static bool CPSInitAllControllersReady(void)
{
    UINT16      index1 = 0;
    bool        bReady = TRUE;
    UINT32      serialNum;
    QM_FAILURE_DATA *qmFailureData;

    qmFailureData = MallocSharedWC(sizeof(*qmFailureData));
    for (index1 = 0; index1 < MAX_CONTROLLERS; index1++)
    {
        serialNum = CCM_ControllerSN(index1);

        if (serialNum == 0)
        {
            continue;
        }

        memset(qmFailureData, 0x00, sizeof(*qmFailureData));

        if (ReadFailureData(serialNum, qmFailureData) == GOOD)
        {
            /*
             * If the controller is either in the power on ready (POR) state
             * or in the failed state (FAILED) it is considered
             * ready.  If not, flag that one of the controllers is not
             * ready and begin the loop again.
             */
            if (qmFailureData->state != FD_STATE_UNUSED &&
                qmFailureData->state != FD_STATE_FAILED &&
                qmFailureData->state != FD_STATE_POR &&
                qmFailureData->state != FD_STATE_FIRMWARE_UPDATE_INACTIVE &&
                qmFailureData->state != FD_STATE_FIRMWARE_UPDATE_ACTIVE &&
                qmFailureData->state != FD_STATE_INACTIVATED &&
                qmFailureData->state != FD_STATE_DISASTER_INACTIVE)
// NOTDONEYET -- Auto-activate  (               && qmFailureData->state != FD_STATE_OPERATIONAL)
            {
/*
 * Here if any of FD_STATE_ADD_CONTROLLER_TO_VCG, FD_STATE_STRANDED_CACHE_DATA,
 *                FD_STATE_FIRMWARE_UPDATE_ACTIVE, FD_STATE_UNFAIL_CONTROLLER,
 *                FD_STATE_VCG_SHUTDOWN, FD_STATE_ACTIVATE.
 */
                bReady = FALSE;
                break;
            }
        }
        else
        {
            bReady = FALSE;
        }
    }
    Free(qmFailureData);

    return bReady;
}


/**
******************************************************************************
**
**  @brief      Loops until BE II status has reached the desired value.
**
**  @param      UINT16 ii_status - Desired II status for the BE.
**
**  @return     none
**
******************************************************************************
**/
static void CPSInitWaitForBEII(UINT16 ii_status)
{
    UINT16      status = 0;

    dprintf(DPRINTF_CPSINIT, "CPSInitWaitForBEII - Waiting for BE II status (0x%x)...\n",
            ii_status);

    while (!(status & ii_status))
    {
        /*
         * Get the II status from the BE static address table lookup.
         */
        if (GetProcAddress_BEII() != 0)
        {
            status = GetProcAddress_BEII()->status;
        }

        /*
         * Make sure we are not hogging the processor.  This could
         * happen since we are just reading the PROC memory and
         * not making an MRP call.
         */
        TaskSleepMS(512);
    }

    /*
     * If the status has already reached the P2INIT state NVRAM has
     * been restored and we do not need to wait for the async event
     * for LOG_NVRAM_RESTORE or LOG_NVRAM_CHKSUM_ERR.
     */
    if (status & II_STATUS_P2INIT)
    {
        InitialNVRAM = LOG_NVRAM_RESTORE;
    }

    dprintf(DPRINTF_CPSINIT, "CPSInitWaitForBEII - BE has reached (0x%x).\n", ii_status);
}


/**
******************************************************************************
**
**  @brief      Loops until BE II status says full define.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void CPSInitWaitForBEReady(void)
{
    SetPowerUpState(POWER_UP_PROCESS_BE_INIT);
    SendX1ChangeEvent(X1_ASYNC_VCG_POWERUP);

    /*
     * Lock the file I/O mutex, this is mainly to stop VLINK name
     * change events while the file system may not be available.
     */
    (void)LockMutex(&fileIOMutex, MUTEX_WAIT);

    /*
     * Wait for the BE to reach full define.
     */
    CPSInitWaitForBEII(II_STATUS_FULLDEF);

    /*
     * Now that the BE is ready we can unlock the mutex and let
     * those file I/O requests come through.
     */
    UnlockMutex(&fileIOMutex);

    dprintf(DPRINTF_CPSINIT, "CPSInitWaitForBEReady: BE is ready.\n");
}

/**
******************************************************************************
**
**  @brief      Loops until this controller is configured.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void CPSInitWaitForConfiguration(void)
{
    SetPowerUpState(POWER_UP_WAIT_CONFIGURATION);
    SendX1ChangeEvent(X1_ASYNC_VCG_POWERUP);

    do
    {
        /* Wait for 1 second(s) before trying again */
        TaskSleepMS(1000);
    } while (!IsConfigured());
}


/**
******************************************************************************
**
**  @brief      Loops until a license has been applied to this controller.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void CPSInitWaitForLicense(void)
{
    QM_FAILURE_DATA *failureData;

    SetPowerUpState(POWER_UP_WAIT_LICENSE);
    SendX1ChangeEvent(X1_ASYNC_VCG_POWERUP);

    memset(&masterConfig, 0x00, sizeof(masterConfig));
    Qm_SetMagicNumber(NVRAM_MAGIC_NUMBER);
    Qm_SetSchema(SCHEMA);

    /*
     * Empty the active controller map (and padding) and the controller
     * configuration map
     */
    memset(&masterConfig.activeControllerMap, ACM_NODE_UNDEFINED,
           sizeof(masterConfig.activeControllerMap));

    memset(&cntlConfigMap, 0x00, sizeof(cntlConfigMap));

    /*
     * Save the changes to the master configuration to NVRAM.
     */
    StoreMasterConfigToNVRAM();

    do
    {
        /* Wait for 1 second(s) before trying again */
        TaskSleepMS(1000);
    } while (!IsLicenseApplied());

    /*
     * If a real license was applied then the number of controllers
     * allowed would not be zero.  It is zero in the case where this
     * controller is being added to an exising virtual controller
     * group.
     */
    if (Qm_GetNumControllersAllowed() == 0)
    {
        /*
         * Wait for 5 second(s) before starting to load the master
         * configuration and the controller map.  It takes some time
         * before the master has had a chance to write the controller map.
         */
        TaskSleepMS(5000);

        /*
         * Wait for the BE to reach the state where it
         * has a temporary PDD list.
         */
        CPSInitWaitForBEII(II_STATUS_TPDD);

        /*
         * Load the master configuration and the controller
         * configuration map.
         */
        LoadMasterConfiguration();
        LoadControllerMap();

        failureData = MallocSharedWC(sizeof(*failureData));
        do
        {
            /* Wait for 1 second(s) before trying again */
            TaskSleepMS(1000);

            memset(failureData, 0x00, sizeof(*failureData));
            ReadFailureData(CntlSetup_GetControllerSN(), failureData);

            /*
             * Continue to loop until the failure state is set to
             * the ADD_CONTROLLER_TO_VCG state.  This indicates that
             * the master has processed the add slave command and is
             * ready for this controller to continue its power-up.
             * If this is a controller replacement scenario, we could also
             * be FAILED or INACTIVATED.
             */
        } while ((failureData->state != FD_STATE_ADD_CONTROLLER_TO_VCG) &&
                 (failureData->state != FD_STATE_FAILED) &&
                 (failureData->state != FD_STATE_INACTIVATED));
        Free(failureData);
    }
}


/**
******************************************************************************
**
**  @brief      Loops forever since the CCB was not able to establish
**              communication with the FE and/or BE processors.  The
**              controller needs to be power cycled or reset.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void CPSInitWaitForProcessorCommReady(void)
{
    SetPowerUpState(POWER_UP_WAIT_PROC_COMM);
    CPSInitWaitForever();
}


/**
******************************************************************************
**
**  @brief      Loops until SES Discovery has completed.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void CPSInitWaitForDiscovery(void)
{
    SetPowerUpState(POWER_UP_PROCESS_DISCOVERY);
    SendX1ChangeEvent(X1_ASYNC_VCG_POWERUP);

    /*
     * Check if the SES Discovery has completed.
     */
    while (!DiscoveryComplete())
    {
        /* Wait for 1 second(s) before trying again */
        TaskSleepMS(1000);
    }
}


/**
******************************************************************************
**
**  @brief      Checks the firmware versions for compatibility.  If the
**              version are not compatible this function will not return.
**              It will wait until the firmware versions are updated and/or
**              the user resets the controller.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void CPSInitCheckFirmwareCompatibility(void)
{
    FW_HEADER  *fwhdr = (FW_HEADER *)CCBRuntimeFWHAddr;
    char        kernelString[80];

    /*
     * On Wookiee, all versions match by default, so take the CCB header
     * and use that to grab the gFWLocalRelease.
     */
    memcpy(gFWInternalRelease, &fwhdr->revision, 4);
    gFWInternalRelease[4] = 0;

    /* Log the Linux Kernel version immediately before the FW version. */

    GetKernelVersion(kernelString, sizeof(kernelString));
    kernelString[sizeof(kernelString) - 1] = 0; /* Ensure termination */
    LogMessage(LOG_TYPE_DEBUG, "SYSTEM-KERNEL: %s", kernelString);

    /*
     * Copy the system release to a global location as a nul terminated ascii
     * string.  If the firmware versions are not compatible we need to go into
     * the loop.  If not we can get out of here and let the controller
     * power-up sequencing continue.
     */
    memcpy(gFWSystemRelease, &fwhdr->systemRelease, 4);
    gFWSystemRelease[4] = 0;
    LogMessage(LOG_TYPE_INFO, "SYSTEM-firmware release (%s-%s)",
               gFWSystemRelease, gFWInternalRelease);
}


/**
******************************************************************************
**
**  @brief      Checks if one or more disk bays are missing and if there
**              are then this will wait until the user instructs the
**              controller on the proper course of action using the power
**              up response command.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void CPSInitCheckForMissingDiskBays(void)
{
    bool        bMissing = false;

    /*
     * Find out if there are missing disk bays...
     */
    bMissing = CPSInitDiskBaysMissing();

    /*
     * If one or more disk bays are missing we need to go into the
     * loop.  If not we can get out of here and let the controller
     * power-up sequencing continue.
     */
    if (bMissing)
    {
        /*
         * Set the waiting for disk bays power-up state.
         */
        SetPowerUpState(POWER_UP_WAIT_DISK_BAY);

        while (bMissing)
        {
            if (MissingDiskBayResponse == POWER_UP_RESPONSE_RETRY)
            {
                /*
                 * Check again if the disk bay(s) are missing...
                 */
                bMissing = CPSInitDiskBaysMissing();

                MissingDiskBayResponse = 0;
            }
            else if (MissingDiskBayResponse == POWER_UP_RESPONSE_CONTINUE)
            {
                bMissing = false;
            }

            /* Wait for 1 second(s) before looping again */
            TaskSleepMS(1000);
        }

        /*
         * Reset the power-up state to START since we have
         * found the missing disk bays or are retrying the
         * operation or continuing without the missing disk
         * bays(s).
         */
        SetPowerUpState(POWER_UP_PROCESS_DISCOVERY);
        SendX1ChangeEvent(X1_ASYNC_VCG_POWERUP);
    }
}


/**
******************************************************************************
**
**  @brief      Checks if one or more disk bays are missing.
**
**  @param      none
**
**  @return     none
**
**  @attention  true if there are missing disk bays, false otherwise.
**
******************************************************************************
**/
static bool CPSInitDiskBaysMissing(void)
{
    bool        bMissing = false;
    PSES_DEVICE pSESList = NULL;
    PSES_DEVICE pSES = NULL;

    pSESList = GetSESList();

    for (pSES = pSESList; pSES != NULL; pSES = pSES->NextSES)
    {
        if (pSES->devStat == 0)
        {
            dprintf(DPRINTF_CPSINIT, "%s: Missing Disk Bay: %8.8x%8.8x\n",
                        __func__, bswap_32((UINT32)(pSES->WWN)),
                        bswap_32((UINT32)(pSES->WWN >> 32)));

            bMissing = true;
        }
    }

    return bMissing;
}

/**
******************************************************************************
**
**  @brief      Checks if the BE NVRAM load resulted in a good load or
**              a checksum error.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void CPSInitCheckForCorruptBENVRAM(void)
{
    /*
     * Wait for the async log event for LOG_NVRAM_RESTORE or
     * MLE_NVRAM_CHECKSUM_ERR to be processed by the async
     * event handler (it sets this value to the event code).
     */
    while (InitialNVRAM == 0)
    {
        /* Wait for 1 second(s) before trying again */
        TaskSleepMS(1000);
    }

    /*
     * If we own drives and a checksum error was encountered we
     * need to send a log
     */
    if (Qm_GetOwnedDriveCount() > 0 && InitialNVRAM == LOG_NVRAM_CHKSUM_ERR)
    {
        /*
         * Set the waiting for disk bays power-up state.
         */
        SetPowerUpState(POWER_UP_WAIT_CORRUPT_BE_NVRAM);

        /*
         * Send a log message that we are waiting for missing
         * disk bays.
         */
        SendAsyncEvent(LOG_WAIT_CORRUPT_BE_NVRAM, 0, NULL);

        while (InitialNVRAM == LOG_NVRAM_CHKSUM_ERR)
        {
            if (CorruptBENVRAMResponse == POWER_UP_RESPONSE_RETRY)
            {
                /*
                 * Since the NVRAM version was corrupt, lets retry
                 * from the file system instead.
                 */
                SM_NVRAMRestore(MRNOFSYS | MRNOOVERLAY, NULL);
                CorruptBENVRAMResponse = 0;
            }
            else if (CorruptBENVRAMResponse == POWER_UP_RESPONSE_CONTINUE)
            {
                InitialNVRAM = 0;
            }

            /* Wait for 1 second(s) before looping again */
            TaskSleepMS(1000);
        }
    }
}


/**
******************************************************************************
**
**  @brief      Loops forever or until the user resets the controller.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
NORETURN void CPSInitWaitForever(void)
{
    while (FOREVER)
    {
        TaskSleepMS(5000);
    }
}


/**
******************************************************************************
**
**  @brief      Looks through the mirror partner list and determines if
**              there are missing controllers.  For each of the missing
**              controllers, notify its mirror partner and have it start
**              the stripe resync process.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void CPSInitStartStripeResync(void)
{
    /*
     * Start the RAID 5 powerup processing.
     */
    RM_R5PowerupProcessing();

    /*
     * Start the RAID 5 stripe resync monitor to track the progress
     * of the operations.
     */
    RM_R5StripeResyncMonitorStart();
}


/**
******************************************************************************
**
**  @brief      Waits until there are no raids doing a RAID 5 stripe
**              resync before returning.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void CPSInitWaitForR5StripeResync(void)
{
    dprintf(DPRINTF_DEFAULT, "CPSInitWaitForR5StripeResync: ENTER\n");

    /*
     * Set the master controller power up state to indicate that
     * it is starting to process the RAID 5 stripe resync records,
     * basically it is waiting for all the stripe resyncs to
     * complete.
     */
    SetPowerUpState(POWER_UP_PROCESS_R5_RIP);
    SendX1ChangeEvent(X1_ASYNC_VCG_POWERUP);

    while (RM_R5StripeResyncInProgress())
    {
        TaskSleepMS(1000);
    }

    dprintf(DPRINTF_DEFAULT, "CPSInitWaitForR5StripeResync: EXIT\n");
}


/**
******************************************************************************
**
**  @brief      Loop through all the controllers in the CNC and have
**              them reset the FE NVA Resync Records.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void CPSInitResetResyncRecords(void)
{
    INT32       rc = GOOD;
    UINT32      index1;
    UINT32      configIndex;
    UINT32      controllerSN;

    for (index1 = 0; index1 < Qm_GetNumControllersAllowed(); ++index1)
    {
        configIndex = Qm_GetActiveCntlMap(index1);

        if (configIndex != ACM_NODE_UNDEFINED)
        {
            controllerSN = CCM_ControllerSN(configIndex);

            if (controllerSN != 0)
            {
                rc = SM_MRReset(controllerSN, MXNFENVA);

                if (rc == ERROR)
                {
                    dprintf(DPRINTF_DEFAULT, "CPSInitResetResyncRecords: Failed to reset (0x%x).\n",
                            controllerSN);
                }
            }
        }
    }
}


/**
******************************************************************************
**
**  @brief      If the controller detects a disaster mode it will wait
**              until the user has corrected the problem and tells the
**              controller how to proceed.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void CPSInitWaitForDisasterRecovery(void)
{
    UINT32      index1;
    QM_FAILURE_DATA *qmFailureData;

    SetPowerUpState(POWER_UP_WAIT_DISASTER);

    /*
     * Loop until the user has specified to continue without drives or we
     * finally see drives that we own.
     */
    while (DisasterResponse == 0)
    {
        /* Wait for 5 seconds before trying again */
        TaskSleepMS(5000);
    }

    /*
     * Read the current value for this controllers failure data state.  If
     * it is still OPERATIONAL or ACTIVATE it can react to the power-up
     * response.  If not then let it fall through to the multiple controller
     * power-up sequencing and it will handle itself.
     */
    qmFailureData = MallocSharedWC(sizeof(*qmFailureData));
    memset(qmFailureData, 0x00, sizeof(*qmFailureData));
    ReadFailureData(CntlSetup_GetControllerSN(), qmFailureData);

    if (qmFailureData->state == FD_STATE_OPERATIONAL ||
        qmFailureData->state == FD_STATE_ACTIVATE)
    {
        /*
         * If the user chose to have this controller continue power-up
         * as the "KEEP ALIVE" controller it needs to fail the remaining
         * controllers in the group and then power-up itself as the
         * master controller.
         *
         * If not, the user must have chose to just continue power-up
         * and see what happens, in that case power-up as a disaster
         * inactive controller and fall into the multiple controller
         * power-up sequencing.
         */
        if (DisasterResponse == POWER_UP_RESPONSE_CONTINUE_KA)
        {
            /*
             * Make sure to set the "KEEP ALIVE" flag.
             */
            EL_KeepAliveSetUnfail(TRUE);

            /*
             * Fail all remaining controllers in the group.
             */
            for (index1 = 0; index1 < Qm_GetNumControllersAllowed(); ++index1)
            {
                if (CCM_ControllerSN(index1) != 0 &&
                    CCM_ControllerSN(index1) != CntlSetup_GetControllerSN())
                {
                    WriteFailureDataState(CCM_ControllerSN(index1), FD_STATE_FAILED);
                }
            }

            /*
             * Make sure this controller is in the activate state so it
             * will continue power-up without further user input.
             */
            WriteFailureDataState(CntlSetup_GetControllerSN(), FD_STATE_ACTIVATE);
        }
        else
        {
            /*
             * The user chose not to have this controller be the "KEEP ALIVE"
             * controller so just continue power-up as a disaster inactive
             * controller.
             */
            WriteFailureDataState(CntlSetup_GetControllerSN(),
                                  FD_STATE_DISASTER_INACTIVE);
        }

        /*
         * Reset the power-up state to START since we are
         * emerging from the disaster state.
         */
        SetPowerUpState(POWER_UP_START);
        SendX1ChangeEvent(X1_ASYNC_VCG_POWERUP);
    }
    Free(qmFailureData);
}


/**
******************************************************************************
**
**  @brief      Wait for all controllers to reach a point where their
**              cache is initialized.  This function looks as the II
**              status from all controllers to see if they have reached
**              II_STATUS_CINIT and then tells all controllers to allow
**              the FE to put regular ports on the FE cards.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void CPSInitWaitForAllCacheInit(void)
{
    UINT16      status = 0;
    UINT16      i = 0;
    UINT32      controllerSN = 0;

    LogMessage(LOG_TYPE_DEBUG, "CPSInitWaitForAllCacheInit - ENTER");

    /*
     * If we are not already in the cache error state then change the
     * power up state to be processing the cache initialization.
     */
    if (GetPowerUpState() != POWER_UP_WAIT_CACHE_ERROR)
    {
        SetPowerUpState(POWER_UP_PROCESS_CACHE_INIT);
        SetPowerUpAStatus(POWER_UP_ASTATUS_UNKNOWN);
        SendX1ChangeEvent(X1_ASYNC_VCG_POWERUP);
    }

    /*
     * Loop until all controllers have their cache initialized.
     */
    while (!(status & II_STATUS_CINIT))
    {
        if (GetPowerUpState() != POWER_UP_WAIT_CACHE_ERROR)
        {
            /*
             * Process any cache error event that might have been seen.
             */
            CPSInitProcessCacheErrorEvent();
        }

        /*
         * Loop through the active controllers.
         */
        for (i = 0; i < ACM_GetActiveControllerCount(Qm_ActiveCntlMapPtr()); ++i)
        {
            controllerSN = CCM_ControllerSN(Qm_GetActiveCntlMap(i));

            /*
             * If somehow the controller serial number is zero, skip it.
             */
            if (controllerSN == 0)
            {
                continue;
            }

            /*
             * Get the FE II status for the controller.
             */
            status = CPSInitGetCacheStatus(controllerSN);

            /*
             * Check if the cache is intialized.
             */
            if (!(status & II_STATUS_CINIT))
            {
                /*
                 * The cache is not initialized so skip the rest of the
                 * active controllers and complete the while loop which
                 * will sleep for a little and then check the status
                 * again.
                 */
                break;
            }
        }

        /*
         * Make sure we are not hogging the processor.  This could
         * happen since we are just reading the PROC memory and
         * not making an MRP call (on the local controller).
         */
        TaskSleepMS(1000);
    }

    /*
     * Tell the slave controllers to negotiate the mirror partner information with the master
     * controller.
     */
    IpcSignalSlaves(IPC_SIGNAL_MIRROR_PARTNER_INFO);

    /*
     * Tell this controller to negotiate the mirror partner information with the slave
     * controller(s).
     */
    MRP_NegotiateMPInfo();

    /*
     * Tell the slave controllers to allow the regular ports to be
     * placed on the FE cards.
     */
    IpcSignalSlaves(IPC_SIGNAL_FE_PORT_GO);

    /*
     * Tell this controller to allow the regular ports to be
     * placed on the FE cards.
     */
    MRP_FEPortGo();

    LogMessage(LOG_TYPE_DEBUG, "CPSInitWaitForAllCacheInit - EXIT");
}


/**
******************************************************************************
**
**  @brief      Starts the cache initialization wait task on this controller.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void CPSInitWaitForCacheInitTaskStart(void)
{
    TaskCreate(CPSInitWaitForCacheInit, NULL);
}


/**
******************************************************************************
**
**  @brief      Function/Task used to wait for cache to become initialized
**              by watching the II status to contain the II_STATUS_CINIT
**              bit set.  This code is used as both a function during the
**              power-up of a failed/inactive controller and as a task during
**              the power-up of a slave controller in a multiple controller
**              power-up scenario.
**
**  @param      TASK_PARMS* parms - Parameter required for a task, not used
**                                  in this function/task.
**
**  @return     none
**
******************************************************************************
**/
static void CPSInitWaitForCacheInit(UNUSED TASK_PARMS *parms)
{
    UINT16      status = 0;

    LogMessage(LOG_TYPE_DEBUG, "CPSInitWaitForCacheInit - ENTER");

    /*
     * If we are not already in the cache error state then change the
     * power up state to be processing the cache initialization.
     */
    if (GetPowerUpState() != POWER_UP_WAIT_CACHE_ERROR)
    {
        SetPowerUpState(POWER_UP_PROCESS_CACHE_INIT);
        SetPowerUpAStatus(POWER_UP_ASTATUS_UNKNOWN);
        SendX1ChangeEvent(X1_ASYNC_VCG_POWERUP);
    }

    while (!(status & II_STATUS_CINIT))
    {
        if (GetPowerUpState() != POWER_UP_WAIT_CACHE_ERROR)
        {
            /*
             * Process any cache error event that might have been seen.
             */
            CPSInitProcessCacheErrorEvent();
        }

        /*
         * Get the II status from the BE static address table lookup.
         */
        if (GetProcAddress_FEII() != 0)
        {
            status = GetProcAddress_FEII()->status;
        }

        /*
         * Make sure we are not hogging the processor.  This could
         * happen since we are just reading the PROC memory and
         * not making an MRP call (on the local controller).
         */
        TaskSleepMS(1000);
    }

    LogMessage(LOG_TYPE_DEBUG, "CPSInitWaitForCacheInit - EXIT");
}


/**
******************************************************************************
**
**  @brief      Process the current cache error event by setting up the
**              correct power-up state and additional status values.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
static void CPSInitProcessCacheErrorEvent(void)
{
    switch (LOG_GetCode(gCacheErrorEvent))
    {
            /* User intervention required */
        case LOG_GetCode(LOG_WC_SEQNO_BAD):
            SetPowerUpAStatus(POWER_UP_ASTATUS_WC_SEQNO_BAD);
            SetPowerUpState(POWER_UP_WAIT_CACHE_ERROR);
            gCacheErrorEvent = 0;
            SendX1ChangeEvent(X1_ASYNC_VCG_POWERUP);
            break;

            /* User intervention required */
        case LOG_GetCode(LOG_WC_SN_VCG_BAD):
            SetPowerUpAStatus(POWER_UP_ASTATUS_WC_SN_VCG_BAD);
            SetPowerUpState(POWER_UP_WAIT_CACHE_ERROR);
            gCacheErrorEvent = 0;
            SendX1ChangeEvent(X1_ASYNC_VCG_POWERUP);
            break;

            /* User intervention required */
        case LOG_GetCode(LOG_WC_SN_BAD):
            SetPowerUpAStatus(POWER_UP_ASTATUS_WC_SN_BAD);
            SetPowerUpState(POWER_UP_WAIT_CACHE_ERROR);
            gCacheErrorEvent = 0;
            SendX1ChangeEvent(X1_ASYNC_VCG_POWERUP);
            break;

            /* User intervention required */
        case LOG_GetCode(LOG_WC_NVMEM_BAD):
            SetPowerUpAStatus(POWER_UP_ASTATUS_WC_NVMEM_BAD);
            SetPowerUpState(POWER_UP_WAIT_CACHE_ERROR);
            gCacheErrorEvent = 0;
            SendX1ChangeEvent(X1_ASYNC_VCG_POWERUP);
            break;
    }
}


/**
******************************************************************************
**
**  @brief      Retrieve the controllers II information (PROC STATS) and
**              then return the status field from the II information.  This
**              variable holds the cache initialization status.
**
**  @param      UINT32 controllerSN - Who gets the request?
**
**  @return     UINT16 II status containing the cache initialization flag.
**
******************************************************************************
**/
static UINT16 CPSInitGetCacheStatus(UINT32 controllerSN)
{
    XIO_PACKET  reqPacket;
    XIO_PACKET  rspPacket;
    INT32       rc = GOOD;
    PI_STATS_FRONT_END_PROC_RSP *pResponse;
    UINT16      state = 0;

    /*
     * Allocate memory for the request (header and data) and the
     * response header.  Memory for the response data is allocated
     * in TunnelRequest().
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    reqPacket.pPacket = NULL;
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    rspPacket.pPacket = NULL;
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    /*
     * Fill in the request header
     */
    reqPacket.pHeader->commandCode = PI_STATS_FRONT_END_PROC_CMD;
    reqPacket.pHeader->length = 0;

    /*
     * If the port list request is for this controller make the
     * request to the port server directly.  If it is for one
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
        UINT8   retries = 2;                /* Ethernet, Fiber(1), Disk Quorum(2) */

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
     * invalid command code.  If that is the case then the other
     * controller did not support the command and we can assume that
     * the mirror partner can change.
     */
    if (rc == PI_GOOD)
    {
        pResponse = (PI_STATS_FRONT_END_PROC_RSP *)rspPacket.pPacket;
        state = pResponse->ii.status;
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

    return state;
}


/**
******************************************************************************
**
**  @brief      Calls the MRCREATECTRL MRP to create the controller for
**              the VCG.
**
**  @param      UINT32 numControllers - The number of controler in the VCG.
**                                      The MRP will create, delete, or
**                                      modify targets.
**
**  @return     PI_GOOD or one of the MRP error codes (PI_ERROR, etc.).
**
******************************************************************************
**/
INT32 CreateController(UINT8 numControllers)
{
    INT32       rc = GOOD;
    MRCREATECTRLR_REQ *ptrInPkt;
    MRCREATECTRLR_RSP *ptrOutPkt;
    UINT16      rcMRP;

    /*
     * Allocate memory for the MRP input and output packets.
     */
    ptrInPkt = MallocWC(sizeof(*ptrInPkt));
    ptrOutPkt = MallocSharedWC(sizeof(*ptrOutPkt));

    ptrInPkt->numControllers = numControllers;

    /*
     * Send the request to Thunderbolt.  This function handles timeout
     * conditions and task switches while waiting.
     */
    rcMRP = PI_ExecMRP(ptrInPkt, sizeof(*ptrInPkt), MRCREATECTRLR,
                       ptrOutPkt, sizeof(*ptrOutPkt), GetGlobalMRPTimeout());

    if (rcMRP != PI_GOOD)
    {
        dprintf(DPRINTF_CPSINIT, "CreateController: Failed to create the controller (rcMRP: 0x%x).\n",
                rcMRP);

        rc = ERROR;
    }

    /*
     * Free the allocated memory.
     */
    Free(ptrInPkt);

    if (rcMRP != PI_TIMEOUT)
    {
        Free(ptrOutPkt);
    }

    return (rc);
}


/**
******************************************************************************
**
**  @brief      Sends the Ping MRP to the BEP with one of the steps
**              described below.
**
**  @param      UINT16 step - The firmware step allowed.
**                  0x00 - MRAWAKE_TEST or mpgping (just ping)
**                  0x01 - MRAWAKE_NVRAMRDY or mpgnvramrdy (NVRAM can be read up from disk)
**                  0x02 - MRAWAKE_MASTER or mpgmaster (controller is master)
**                  0x03 - MRAWAKE_SLAVE or mpgslave (controller is slave)
**
**  @return     PI_GOOD or one of the MRP error codes (PI_ERROR, etc.).
**
******************************************************************************
**/
UINT16 MRP_Awake(UINT16 step)
{
    MRAWAKE_REQ *ptrInPkt;
    MRAWAKE_RSP *ptrOutPkt;
    UINT16      rc;

    /* Allocate memory for the MRP input and output packets. */
    ptrInPkt = MallocWC(sizeof(*ptrInPkt));
    ptrOutPkt = MallocSharedWC(sizeof(*ptrOutPkt));

    /*
     * Get the active number of controllers in the Virtual Controller Group.
     * If the there are more then 1 active controllers in the group, then
     * indicate to the FE and BE processors that the CCB is required for
     * operation and they should be monitoring (watchdogging) heartbeats from
     * the CCB.
     */
    if (ACM_GetActiveControllerCount(Qm_ActiveCntlMapPtr()) > 1)
    {
        step |= MRAWAKE_CCBREQ;     /* Note more than one controller present. */
    }

    ptrInPkt->step = step;

    rc = PI_ExecMRP(ptrInPkt, sizeof(*ptrInPkt), MRAWAKE,
                    ptrOutPkt, sizeof(*ptrOutPkt), GetGlobalMRPTimeout());

    /* Free the allocated memory. */
    Free(ptrInPkt);

    if (rc != PI_TIMEOUT)
    {
        Free(ptrOutPkt);
    }

    return (rc);
}


/**
******************************************************************************
**
**  @brief      Get the lowest X1 compatibility index of the controllers
**              in the DSC.
**
**  @param      none
**
**  @return     UINT32 compatibility index or zero if unable to be retrieved.
**
******************************************************************************
**/
static UINT32 GetDSCX1CompatibilityIndex(void)
{
    UINT16      index1;
    UINT32      compatIndex = X1_COMPATIBILITY;
    UINT32      serialNum;
    INT32       rc = PI_GOOD;

    for (index1 = 0; index1 < MAX_CONTROLLERS; ++index1)
    {
        serialNum = CCM_ControllerSN(index1);

        if (serialNum == 0 || serialNum == GetMyControllerSN())
        {
            continue;
        }

        LogMessage(LOG_TYPE_DEBUG, "POWERUP-Get X1 Compat Ping (0x%x)", serialNum);

        /*
         * Sending a test PING to the controller to make sure
         * that we can communicate with that controller over IPC.
         */
        rc = IpcSendPingWithRetries(serialNum, SENDPACKET_ETHERNET, IPC_PING_RETRY_COUNT);

        if (rc == PI_GOOD)
        {
            LogMessage(LOG_TYPE_DEBUG, "POWERUP-Get X1 Compat (0x%x)", serialNum);

            compatIndex = MIN(GetX1CompatibilityIndex(serialNum), compatIndex);
        }
        else
        {
            LogMessage(LOG_TYPE_DEBUG, "POWERUP-Get X1 Compat failed ping (0x%x)", rc);

            compatIndex = 0;
        }
    }

    return compatIndex;
}


/**
******************************************************************************
**
**  @brief      Requests the X1 compatibility index from a controller.
**
**  @param      UINT32 controllerSN - Controller from which to get the
**                                    X1 compatibility index.
**
**  @return     UINT32 compatibility index or zero if unable to be retrieved.
**
******************************************************************************
**/
static UINT32 GetX1CompatibilityIndex(UINT32 controllerSN)
{
    XIO_PACKET  reqPacket;
    XIO_PACKET  rspPacket;
    INT32       rc = GOOD;
    UINT32      compatIndex = 0;

    /*
     * Allocate memory for the request (header and data) and the
     * response header.  Memory for the response data is allocated
     * in TunnelRequest().
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    reqPacket.pPacket = NULL;
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    rspPacket.pPacket = NULL;
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    /*
     * Fill in the request header
     */
    reqPacket.pHeader->commandCode = PI_X1_COMPATIBILITY_INDEX_CMD;
    reqPacket.pHeader->senderInterface = SENDPACKET_ETHERNET;
    reqPacket.pHeader->length = 0;

    /*
     * If the port list request is for this controller make the
     * request to the port server directly.  If it is for one
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
        UINT8   retries = 2;                /* Ethernet, Fiber(1), Disk Quorum(2) */

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
        LogMessage(LOG_TYPE_DEBUG, "POWERUP-Get X1 Compat Failed (0x%x)", rc);
    }
    else
    {
        compatIndex = ((PI_X1_COMPATIBILITY_INDEX_RSP *)rspPacket.pPacket)->state;
    }

    LogMessage(LOG_TYPE_DEBUG, "POWERUP-X1 Compat for (0x%x) is (0x%x)", controllerSN, compatIndex);

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

    return compatIndex;
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
