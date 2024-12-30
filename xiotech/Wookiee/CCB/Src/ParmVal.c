/* $Id: ParmVal.c 160950 2013-04-22 21:10:28Z marshall_midden $ */
/*===========================================================================
** FILE NAME:       ParmVal.c
** MODULE TITLE:    CCB Command Layers
**
** DESCRIPTION:     Defines the command layering withing the CCB code
**
** Copyright (c) 2001-2009 Xiotech Corporation. All rights reserved.
**==========================================================================*/
#include "ParmVal.h"
#include "XIO_Macros.h"
#include "cps_init.h"
#include "debug_files.h"
#include "errorCodes.h"
#include "LOG_Defs.h"
#include "MR_Defs.h"
#include "EL.h"
#include "globalOptions.h"
#include "LargeArrays.h"
#include "PacketInterface.h"
#include "debug_files.h"
#include "PI_CmdHandlers.h"
#include "PI_Packets.h"
#include "PI_Utils.h"
#include "PI_Misc.h"
#include "quorum_utils.h"
#include "RM_Raids.h"
#include "serial_num.h"

#ifndef DISABLE_LOCAL_RAID_MONITORING
#include "xk_raidmon.h"
#endif  /* DISABLE_LOCAL_RAID_MONITORING */

/*****************************************************************************
** Private defines
*****************************************************************************/
#define MASTER_CHECK_TRUE           1
#define MASTER_CHECK_FALSE          0

typedef INT32 (*PI_ParmValHandler_t)(XIO_PACKET *, XIO_PACKET *);

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static INT32 ParmCheckPacketVersion(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket);
static INT32 ParmCheckIfPowerUpComplete(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket);
static INT32 ParmCheckIfMaster(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket);
static INT32 ParmCheckIfElectionInProgress(XIO_PACKET *pReqPacket,
                                           XIO_PACKET *pRspPacket);
static INT32 ParmCheckIfR5StripeResyncInProgress(XIO_PACKET *pReqPacket,
                                                 XIO_PACKET *pRspPacket);

#ifndef DISABLE_LOCAL_RAID_MONITORING
static INT32 ParmCheckIfLocalRaidsResyncing(XIO_PACKET *pReqPacket,
                                            XIO_PACKET *pRspPacket);
#endif  /* DISABLE_LOCAL_RAID_MONITORING */

static INT32 ParmCheckPowerUpResponse(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket);
static INT32 ParmCheckPdiskPiLabelPhy(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket);
static INT32 ParmCheckPdiskPiFailPdisk(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket);
static INT32 ParmCheckPdiskPiFailBackPdisk(XIO_PACKET *pReqPacket,
                                           XIO_PACKET *pRspPacket);
static INT32 ParmCheckSetGeoLocation(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket);
static INT32 ParmCheckPdiskPiPdiskDeleteReq(XIO_PACKET *pReqPacket,
                                            XIO_PACKET *pRspPacket);
static INT32 ParmCheckVdiskPiCreateVdisk(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket);
static INT32 ParmCheckVdiskPiVdiskCtl(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket);
static INT32 ParmCheckVdiskPiSetAttribute(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket);
static INT32 ParmCheckVdiskPiVdiskSetPriority(XIO_PACKET *pReqPacket,
                                              XIO_PACKET *pRspPacket);
static INT32 ParmCheckProcPiGetDevicePath(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket);
static INT32 ParmCheckProcPiGetPortList(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket);
static INT32 ParmCheckResetPiResetQlogic(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket);
static INT32 ParmCheckAdminPiFwVersionReq(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket);
static INT32 ParmCheckAdminPiScrubIn(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket);
static INT32 ParmCheckDebugPiDebugMemRdwrReq(XIO_PACKET *pReqPacket,
                                             XIO_PACKET *pRspPacket);
static INT32 ParmCheckDebugPiDebugGetSerialNumReq(XIO_PACKET *pReqPacket,
                                                  XIO_PACKET *pRspPacket);
static INT32 ParmCheckVcgPiSetCache(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket);
static INT32 ParmCheckVcgPiVcgApplyLicenseReq(XIO_PACKET *pReqPacket,
                                              XIO_PACKET *pRspPacket);
static INT32 ParmCheckDiskBayPiDiskBayDeleteReq(XIO_PACKET *pReqPacket,
                                                XIO_PACKET *pRspPacket);
static INT32 ParmCheckLogPiLogInfoReq(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket);
static INT32 ParmCheckFailUnfailInterfaceReq(XIO_PACKET *pReqPacket,
                                             XIO_PACKET *pRspPacket);
static INT32 CheckDuplicateID(UINT16 count, UINT16 idList[]);
static INT32 ParmCheckDlmPathSelectionReq(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket);

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
** Function:    ParmValidationPreProcessImpl
**
** Description: Implements the Parameter Validation pre-processing function
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
INT32 ParmValidationPreProcessImpl(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    PI_ParmValHandler_t PI_ParmValHandler = NULL;       /* command handler */
    INT32       rc = GOOD;      /* assume good    */
    INT8        checkMaster = MASTER_CHECK_FALSE;

    /*
     * If the request packet length != 0 but the request pointer is NULL,
     * there was an error allocating the request packet. return an error.
     */
    if ((pReqPacket->pHeader->length) && (pReqPacket->pPacket == NULL))
    {
        pRspPacket->pHeader->status = PI_MALLOC_ERROR;
        rc = PI_MALLOC_ERROR;
        return (rc);
    }

    /*
     * For now skip all the Magnitude packets
     */
    if (pReqPacket->pHeader->commandCode >= XSP_BASE)
    {
        return rc;
    }

    rc = ParmCheckPacketVersion(pReqPacket, pRspPacket);

    if (rc != GOOD)
    {
        return (rc);
    }

    /*
     * This switch handles checking of input parameters for each command.
     */
    switch (pReqPacket->pHeader->commandCode)
    {
        /********************************************************************
        * The following commands have validation routines and may or may
        * not require mastership.
        *******************************************************************/
        case PI_POWER_UP_RESPONSE_CMD:
            PI_ParmValHandler = ParmCheckPowerUpResponse;
            break;

            /*
             *  Pdisk commands with PI_LABEL_PHY.
             */
        case PI_PDISK_LABEL_CMD:
            checkMaster = MASTER_CHECK_TRUE;
            PI_ParmValHandler = ParmCheckPdiskPiLabelPhy;
            break;

            /*
             *  Pdisk commands with PI_PDISK_FAIL_REQ.
             */
        case PI_PDISK_FAIL_CMD:
            checkMaster = MASTER_CHECK_TRUE;
            PI_ParmValHandler = ParmCheckPdiskPiFailPdisk;
            break;

            /*
             *  Pdisk commands with PI_PDISK_FAILBACK_REQ.
             */
        case PI_PDISK_FAILBACK_CMD:
            checkMaster = MASTER_CHECK_TRUE;
            PI_ParmValHandler = ParmCheckPdiskPiFailBackPdisk;
            break;

        case PI_PDISK_AUTO_FAILBACK_ENABLE_DISABLE_CMD:
            if (((MRPDISKAUTOFAILBACKENABLEDISABLE_REQ *)(pReqPacket->pPacket))->options != 2)
            {
                checkMaster = MASTER_CHECK_TRUE;
            }
            break;

        case PI_SET_GEO_LOCATION_CMD:
            checkMaster = MASTER_CHECK_TRUE;
            PI_ParmValHandler = ParmCheckSetGeoLocation;
            break;

            /*
             *  Pdisk commands with PI_DISK_DELETE_REQ.
             */
        case PI_PDISK_DELETE_CMD:
            checkMaster = MASTER_CHECK_TRUE;
            PI_ParmValHandler = ParmCheckPdiskPiPdiskDeleteReq;
            break;

            /*
             *  Vdisk commands with PI_CREATE_VDISK.
             */
        case PI_VDISK_CREATE_CMD:
        case PI_VDISK_EXPAND_CMD:
        case PI_VDISK_PREPARE_CMD:
            checkMaster = MASTER_CHECK_TRUE;
            PI_ParmValHandler = ParmCheckVdiskPiCreateVdisk;
            break;

        case PI_VDISK_SET_PRIORITY_CMD:
            checkMaster = MASTER_CHECK_TRUE;
            PI_ParmValHandler = ParmCheckVdiskPiVdiskSetPriority;
            break;
        case PI_VCG_VDISK_PRIORITY_ENABLE_CMD:
            checkMaster = MASTER_CHECK_TRUE;
            PI_ParmValHandler = ParmCheckVdiskPiVdiskSetPriority;
            break;

            /*
             *  Vdisk commands with PI_VDISK_CONTROL_REQ.
             */
        case PI_VDISK_CONTROL_CMD:
            checkMaster = MASTER_CHECK_TRUE;
            PI_ParmValHandler = ParmCheckVdiskPiVdiskCtl;
            break;

            /*
             *  Vdisk commands with PI_VDISK_SET_ATTRIBUTE_REQ.
             */
        case PI_VDISK_SET_ATTRIBUTE_CMD:
            checkMaster = MASTER_CHECK_TRUE;
            PI_ParmValHandler = ParmCheckVdiskPiSetAttribute;
            break;

            /*
             *  RAID commands with PI_RAID_CONTROL_REQ.
             *  This command can be sent to get the status of each controller
             *  and set the parity scan control. Scrubbing will ONLY be
             *  set when this is sent to the master however.
             */
        case PI_RAID_CONTROL_CMD:
            PI_ParmValHandler = ParmCheckAdminPiScrubIn;
            break;

            /*
             *  Proc Get Devic Path commands with PI_GET_DEVICE_PATH.
             */
        case PI_PROC_BE_DEVICE_PATH_CMD:
            PI_ParmValHandler = ParmCheckProcPiGetDevicePath;
            break;

            /*
             *  Reset commands with PI_RESET_QLOGIC.
             */
        case PI_PROC_BE_PORT_LIST_CMD:
        case PI_PROC_FE_PORT_LIST_CMD:
            PI_ParmValHandler = ParmCheckProcPiGetPortList;
            break;

            /*
             *  Reset commands with PI_RESET_QLOGIC.
             */
        case PI_PROC_RESET_FE_QLOGIC_CMD:
        case PI_PROC_RESET_BE_QLOGIC_CMD:
            PI_ParmValHandler = ParmCheckResetPiResetQlogic;
            break;

            /*
             *  Admin commands with PI_FW_VERSION_REQ.
             */
        case PI_ADMIN_FW_VERSIONS_CMD:
            PI_ParmValHandler = ParmCheckAdminPiFwVersionReq;
            break;

            /*
             *  Debug commands with PI_DEBUG_MEM_RDWR_REQ.
             */
        case PI_DEBUG_MEM_RDWR_CMD:
            PI_ParmValHandler = ParmCheckDebugPiDebugMemRdwrReq;
            break;

            /*
             *  Debug commands with PI_DEBUG_GET_SERIAL_NUM_REQ.
             */
        case PI_DEBUG_GET_SER_NUM_CMD:
            PI_ParmValHandler = ParmCheckDebugPiDebugGetSerialNumReq;
            break;

            /*
             *  VCG commands with PI_VCG_SET_CACHE_REQ.
             */
        case PI_VCG_SET_CACHE_CMD:
            checkMaster = MASTER_CHECK_TRUE;
            PI_ParmValHandler = ParmCheckVcgPiSetCache;
            break;

            /*
             * VCG commands with PI_VCG_APPLY_LICENSE_REQ.
             */
        case PI_VCG_APPLY_LICENSE_CMD:
            PI_ParmValHandler = ParmCheckVcgPiVcgApplyLicenseReq;
            break;

            /*
             *  Pdisk commands with PI_DISK_BAY_DELETE_REQ.
             */
        case PI_DISK_BAY_DELETE_CMD:
            checkMaster = MASTER_CHECK_TRUE;
            PI_ParmValHandler = ParmCheckDiskBayPiDiskBayDeleteReq;
            break;

            /*
             *  Log commands with PI_LOG_INFO_REQ.
             */
        case PI_LOG_INFO_CMD:
            PI_ParmValHandler = ParmCheckLogPiLogInfoReq;
            break;

            /*
             * Fail and Unfail interface requests
             */
        case PI_MISC_UNFAIL_INTERFACE_CMD:
        case PI_MISC_FAIL_INTERFACE_CMD:
            PI_ParmValHandler = ParmCheckFailUnfailInterfaceReq;
            break;

            /*
             * Set DLM Path selection Algorithm
             */
        case PI_DLM_PATH_SELECTION_ALGO_CMD:
            PI_ParmValHandler = ParmCheckDlmPathSelectionReq;
            break;

        /********************************************************************
        * The following commands do not have validation routines
        * but require that the request be processed only by a
        * master controller.
        *******************************************************************/
        case PI_PDISK_DEFRAG_CMD:
        case PI_PDISK_DEFRAG_STATUS_CMD:
        case PI_PDISK_UNFAIL_CMD:
        case PI_VDISK_DELETE_CMD:
        case PI_RAID_INIT_CMD:
        case PI_SERVER_SET_PROPERTIES_CMD:
        case PI_SERVER_DELETE_CMD:
        case PI_SERVER_CREATE_CMD:
        case PI_SERVER_ASSOCIATE_CMD:
        case PI_SERVER_DISASSOCIATE_CMD:
        case PI_VLINK_CREATE_CMD:
        case PI_TARGET_SET_PROPERTIES_CMD:
#if ISCSI_CODE
        case PI_ISCSI_SET_TGTPARAM_CMD:
        case PI_ISCSI_SET_CHAP_CMD:
        case PI_SETISNSINFO_CMD:
#endif  /* ISCSI_CODE */
        case PI_PROC_FAIL_CTRL_CMD:
        case PI_VCG_FAIL_CONTROLLER_CMD:
        case PI_VCG_ADD_SLAVE_CMD:
        case PI_VCG_ACTIVATE_CONTROLLER_CMD:
        case PI_VCG_UNFAIL_CONTROLLER_CMD:
        case PI_SNAPSHOT_TAKE_CMD:
        case PI_SNAPSHOT_CHANGE_CMD:
        case PI_PDISK_SPINDOWN_CMD:
        case PI_CLEAR_GEO_LOCATION_CMD:
        case PI_MISC_RESYNCCTL_CMD:
        case PI_MISC_CFGOPTION_CMD:
        case PI_CLIENT_PERSISTENT_DATA_CONTROL_CMD:
        case PI_QUICK_BREAK_PAUSE_RESUME_MIRROR_START_CMD:
        case PI_QUICK_BREAK_PAUSE_RESUME_MIRROR_SEQUENCE_CMD:
        case PI_QUICK_BREAK_PAUSE_RESUME_MIRROR_EXECUTE_CMD:
        case PI_BATCH_SNAPSHOT_START_CMD:
        case PI_BATCH_SNAPSHOT_SEQUENCE_CMD:
        case PI_BATCH_SNAPSHOT_EXECUTE_CMD:
            checkMaster = MASTER_CHECK_TRUE;
            break;

        /********************************************************************
        * The following commands do not have validation routines and do
        * not require mastership.
        *******************************************************************/
        case PI_CONNECT_CMD:
        case PI_PING_CMD:
        case PI_RESET_CMD:
        case PI_POWER_UP_STATE_CMD:
        case PI_ENABLE_X1_PORT_CMD:
        case PI_X1_COMPATIBILITY_INDEX_CMD:
        case PI_PDISK_COUNT_CMD:
        case PI_PDISK_INFO_CMD:
        case PI_PDISK_BEACON_CMD:
        case PI_PDISKS_QLOGIC_TIMEOUT_EMULATE:
        case PI_PDISK_LIST_CMD:
        case PI_PDISK_BYPASS_CMD:
        case PI_PDISKS_CMD:
        case PI_PDISKS_FROM_CACHE_CMD:
        case PI_VDISK_INFO_CMD:
        case PI_VDISK_COUNT_CMD:
        case PI_VDISK_LIST_CMD:
        case PI_VDISK_OWNER_CMD:
        case PI_VDISK_PR_GET_CMD:
        case PI_VDISK_PR_CLR_CMD:
        case PI_VDISKS_CMD:
        case PI_VDISKS_FROM_CACHE_CMD:
        case PI_RAID_INFO_CMD:
        case PI_RAID_COUNT_CMD:
        case PI_RAID_LIST_CMD:
        case PI_RAID_RECOVER_CMD:
        case PI_RAIDS_CMD:
        case PI_RAIDS_FROM_CACHE_CMD:
        case PI_SERVER_INFO_CMD:
        case PI_SERVER_LIST_CMD:
        case PI_SERVER_COUNT_CMD:
        case PI_SERVERS_CMD:
        case PI_SERVER_WWN_TO_TARGET_MAP_CMD:
        case PI_VLINK_REMOTE_CTRL_COUNT_CMD:
        case PI_TARGET_INFO_CMD:
        case PI_TARGET_COUNT_CMD:
        case PI_TARGET_LIST_CMD:
        case PI_TARGETS_CMD:
        case PI_DEBUG_MRMMTEST_CMD:
        case PI_DEBUG_STRUCT_DISPLAY_CMD:
        case PI_DEBUG_GET_ELECTION_STATE_CMD:
        case PI_DEBUG_INIT_PROC_NVRAM_CMD:
        case PI_DEBUG_INIT_CCB_NVRAM_CMD:
        case PI_DEBUG_GET_STATE_RM_CMD:
        case PI_VCG_INFO_CMD:
        case PI_VCG_SHUTDOWN_CMD:
        case PI_VCG_PREPARE_SLAVE_CMD:
        case PI_VCG_PING_CMD:
        case PI_VCG_VALIDATION_CMD:
        case PI_GET_CPUCOUNT_CMD:
        case PI_GET_BACKEND_TYPE_CMD:
        case PI_DISK_BAY_INFO_CMD:
        case PI_DISK_BAY_ALARM_CTRL_CMD:
        case PI_DISK_BAY_COUNT_CMD:
        case PI_DISK_BAY_LIST_CMD:
        case PI_DISK_BAYS_CMD:
        case PI_MISC_COUNT_CMD:
        case PI_MISC_LIST_CMD:
        case PI_FIRMWARE_DOWNLOAD_CMD:
        case PI_TRY_CCB_FW_CMD:
        case PI_LOG_TEXT_MESSAGE_CMD:
        case PI_GENERIC_CMD:
        case PI_GENERIC2_CMD:
        case PI_GENERIC_MRP_CMD:
        case PI_LOG_CLEAR_CMD:
        case PI_STATS_GLOBAL_CACHE_CMD:
        case PI_STATS_FRONT_END_PROC_CMD:
        case PI_STATS_BACK_END_PROC_CMD:
        case PI_STATS_FRONT_END_PCI_CMD:
        case PI_STATS_BACK_END_PCI_CMD:
        case PI_STATS_VDISK_CMD:
        case PI_STATS_PROC_CMD:
        case PI_STATS_PCI_CMD:
        case PI_STATS_CACHE_DEVICES_CMD:
        case PI_STATS_ENVIRONMENTAL_CMD:
        case PI_ENV_II_GET_DATA_CMD:
#if defined(MODEL_7000) || defined(MODEL_4700)
        case PI_ISE_GET_STATUS_CMD:
        case PI_BEACON_ISE_COMPONENT:
#endif /* MODEL_7000 || MODEL_4700 */
        case PI_STATS_HAB_CMD:
        case PI_STATS_SERVERS_CMD:
        case PI_SNAPSHOT_READDIR_CMD:
        case PI_MISC_GET_MODE_CMD:
        case PI_MISC_SET_MODE_CMD:
        case PI_ADMIN_FW_SYS_REL_LEVEL_CMD:
        case PI_ADMIN_GET_IP_CMD:
        case PI_WCACHE_INVALIDATE_CMD:
        case PI_MISC_GET_DEVICE_COUNT_CMD:
        case PI_MISC_RESCAN_DEVICE_CMD:
        case PI_MISC_GET_FE_DEVICE_LIST_CMD:
        case PI_MISC_GET_BE_DEVICE_LIST_CMD:
        case PI_PROC_FAIL_PORT_CMD:
        case PI_SERVER_LOOKUP_CMD:
        case PI_TARGET_RESOURCE_LIST_CMD:
        case PI_STATS_CACHE_DEVICE_CMD:
        case PI_MISC_FILE_SYSTEM_READ_CMD:
        case PI_MISC_FILE_SYSTEM_WRITE_CMD:
        case PI_ADMIN_SETTIME_CMD:
        case PI_ADMIN_GETTIME_CMD:
        case PI_ADMIN_LEDCNTL_CMD:
        case PI_VCG_GET_MP_LIST_CMD:
        case PI_VCG_INACTIVATE_CONTROLLER_CMD:
        case PI_VCG_REMOVE_CONTROLLER_CMD:
        case PI_DEBUG_SCSI_COMMAND_CMD:
        case PI_DEBUG_READWRITE_CMD:
        case PI_DEBUG_BE_LOOP_PRIMITIVE_CMD:
        case PI_DEBUG_FE_LOOP_PRIMITIVE_CMD:
        case PI_PROC_RESTORE_NVRAM_CMD:
        case PI_PROC_START_IO_CMD:
        case PI_PROC_STOP_IO_CMD:
        case PI_PROC_ASSIGN_MIRROR_PARTNER_CMD:
        case PI_PROC_NAME_DEVICE_CMD:
        case PI_MISC_FAILURE_STATE_SET_CMD:
        case PI_ROLLING_UPDATE_PHASE_CMD:
        case PI_MULTI_PART_XFER_CMD:
        case PI_PERSISTENT_DATA_CONTROL_CMD:
        case PI_REGISTER_EVENTS_CMD:
        case PI_CUSTOMER_LOG_ACKNOWLEDGE_CMD:
        case PI_MISC_SERIAL_NUMBER_SET_CMD:
        case PI_MISC_RESYNC_MIRROR_RECORDS_CMD:
        case PI_MISC_CONTINUE_WO_MP_CMD:
        case PI_MISC_INVALIDATE_BE_WC_CMD:
        case PI_MISC_MIRROR_PARTNER_CONTROL_CMD:
        case PI_MFG_CTRL_CLEAN_CMD:
        case PI_PROC_TARGET_CONTROL_CMD:
        case PI_VLINK_REMOTE_CTRL_INFO_CMD:
        case PI_VLINK_REMOTE_CTRL_VDISKS_CMD:
        case PI_VLINK_INFO_CMD:
        case PI_VLINK_BREAK_LOCK_CMD:
        case PI_VLINK_NAME_CHANGED_CMD:
        case PI_VLINK_DLINK_INFO_CMD:
        case PI_VLINK_DLINK_GT2TB_INFO_CMD:
        case PI_VLINK_DLOCK_INFO_CMD:
        case PI_ADMIN_SET_IP_CMD:
        case PI_ENVIRO_DATA_DISK_BAY_CMD:
        case PI_VCG_CONFIGURE_CMD:
        case PI_ENVIRO_DATA_CTRL_AND_BAY_CMD:
        case PI_STATS_SERVER_CMD:
        case PI_STATS_FRONT_END_LOOP_CMD:
        case PI_STATS_BACK_END_LOOP_CMD:
        case PI_SNAPSHOT_LOAD_CMD:
        case PI_DEBUG_REPORT_CMD:
        case PI_WRITE_BUFFER_MODE5_CMD:
        case PI_MISC_GET_WORKSET_INFO_CMD:
        case PI_MISC_SET_WORKSET_INFO_CMD:
        case PI_CACHE_REFRESH_CCB_CMD:
        case PI_SET_DLM_HEARTBEAT_LIST_CMD:
        case PI_CACHE_FLUSH_BE_CMD:
        case PI_RAID_MIRRORING_CMD:
        case PI_MISC_RESYNC_RAIDS_CMD:
        case PI_MISC_PUTDEVCONFIG_CMD:
        case PI_MISC_GETDEVCONFIG_CMD:
        case PI_MISC_QUERY_MP_CHANGE_CMD:
        case PI_MISC_RESYNCDATA_CMD:
        case PI_STATS_BUFFER_BOARD_CMD:
        case PI_MISC_MIRROR_PARTNER_GET_CFG_CMD:
        case PI_BATTERY_HEALTH_SET_CMD:
        case PI_MISC_INVALIDATE_FE_WC_CMD:
        case PI_MISC_SETTDISCACHE_CMD:
        case PI_MISC_CLRTDISCACHE_CMD:
        case PI_MISC_QTDISCACHE_CMD:
#ifndef DISABLE_LOCAL_RAID_MONITORING
        case PI_MISC_LOCAL_RAID_INFO_CMD:
#endif  /* DISABLE_LOCAL_RAID_MONITORING */
#if ISCSI_CODE
        case PI_ISCSI_TGT_INFO_CMD:
        case PI_ISCSI_CHAP_INFO:
        case PI_ISCSI_SESSION_INFO:
        case PI_ISCSI_SESSION_INFO_SERVER:
        case PI_IDD_INFO_CMD:
        case PI_DLM_PATH_STATS_CMD:
        case PI_GETISNSINFO_CMD:
        case PI_VDISK_BAY_REDUNDANT_CMD:
#endif  /* ISCSI_CODE */
        case PI_SET_PR_CMD:
        case PI_REGISTER_CLIENT_TYPE_CMD:
            break;

            /*
             * If the command code is not found report an invalid command error.
             */
        default:
            rc = PI_INVALID_CMD_CODE;
            pRspPacket->pHeader->length = 0;
            pRspPacket->pHeader->status = PI_INVALID_CMD_CODE;
            pRspPacket->pHeader->errorCode = DEINVPKTTYP;
            break;
    }

    /*
     * If the command code is valid the next switch relates command codes
     * to the power up state of the machine.
     */
    if (rc != PI_INVALID_CMD_CODE)
    {
        /*
         * If we are executing a command that will require the power-up
         * processing to be complete we need to make sure that it has
         * indeed completed.
         */
        if (rc == GOOD)
        {
            rc = ParmCheckIfPowerUpComplete(pReqPacket, pRspPacket);
        }

        /*
         * If we are executing a command that will require configuration
         * changes we need to make sure we are the master controller
         */
        if (rc == GOOD && checkMaster == MASTER_CHECK_TRUE)
        {
            rc = ParmCheckIfMaster(pReqPacket, pRspPacket);
        }

        /*
         * If we are executing a command that cannot run during an election
         * we need to make sure an election is not running.
         */
        if (rc == GOOD)
        {
            rc = ParmCheckIfElectionInProgress(pReqPacket, pRspPacket);
        }

        /*
         * If the command cannot run during RAID 5 stripe resync operations,
         * make sure there are no operations in progress.
         */
        if (rc == GOOD)
        {
            rc = ParmCheckIfR5StripeResyncInProgress(pReqPacket, pRspPacket);
        }

#ifndef DISABLE_LOCAL_RAID_MONITORING
        /*
         * If the command cannot run during Local/DSC resync operations,
         * make sure there are no operations in progress.
         */
        if (rc == GOOD)
        {
            rc = ParmCheckIfLocalRaidsResyncing(pReqPacket, pRspPacket);
        }
#endif /* DISABLE_LOCAL_RAID_MONITORING */

        /*
         * If the function pointer is valid, call the function to
         * handle the command.
         */
        if ((rc == GOOD) && (PI_ParmValHandler != NULL))
        {
            rc = PI_ParmValHandler(pReqPacket, pRspPacket);
        }
    }

    /*
     * If parameter validation failed generate a log message.
     */
    if (rc != PI_GOOD)
    {
        LogMessage(LOG_TYPE_DEBUG, "Parameter validation failed (cmdCode: 0x%x, rc: 0x%x, status: 0x%x, errorCode: 0x%x).",
                   pReqPacket->pHeader->commandCode, rc,
                   pRspPacket->pHeader->status, pRspPacket->pHeader->errorCode);
    }

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    ParmValidationPostProcessImpl
**
** Description: Implements the Parameter Validation post-processing function
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
INT32 ParmValidationPostProcessImpl(UNUSED XIO_PACKET *pReqPacket,
                                    UNUSED XIO_PACKET *pRspPacket)
{
    INT32       rc = GOOD;

    /*
     * Parameter validation post-processing code goes HERE!
     */

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    ParmCheckPacketVersion
**
** Description: Check to see if the power-up processing has completed.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
static INT32 ParmCheckPacketVersion(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = GOOD;

    /*
     * Copy the request packet version to response packet version
     */
    pRspPacket->pHeader->packetVersion = pReqPacket->pHeader->packetVersion;

    switch (pReqPacket->pHeader->commandCode)
    {
            /*
             * Presently all commands are of version 1.
             * Keep adding cases for individual commands
             * as new versions are created for the commands.
             */
        case PI_CONNECT_CMD:
            pRspPacket->pHeader->packetVersion = PI_COMPATIBILITY;

            if (pReqPacket->pHeader->packetVersion != 0 &&
                pReqPacket->pHeader->packetVersion != PI_COMPAT_3000 &&
                pReqPacket->pHeader->packetVersion != PI_COMPAT_750 &&
                pReqPacket->pHeader->packetVersion != PI_COMPAT_750_2 &&
                pReqPacket->pHeader->packetVersion != PI_COMPAT_4000)
            {
                pRspPacket->pHeader->status = PI_COMPAT_INDEX_NOT_SUPPORTED;
                rc = PI_COMPAT_INDEX_NOT_SUPPORTED;
                dprintf(DPRINTF_DEFAULT, "CONNECT: Compatibility Index (%d) not supported\n",
                    pReqPacket->pHeader->packetVersion);
            }
            break;

        case PI_VDISK_INFO_CMD:
        case PI_VDISKS_CMD:
        case PI_VDISKS_FROM_CACHE_CMD:
        case PI_STATS_VDISK_CMD:
            if (pReqPacket->pHeader->packetVersion > 3)
            {
                pRspPacket->pHeader->status = PI_INVALID_PACKETVERSION_ERROR;
                rc = PI_INVALID_PACKETVERSION_ERROR;
                dprintf(DPRINTF_DEFAULT, "Command %d Packet Version Not supported\n",
                        pReqPacket->pHeader->commandCode);
            }
            break;

        case PI_VDISK_BAY_REDUNDANT_CMD:
            if (pReqPacket->pHeader->packetVersion != 1)
            {
                pRspPacket->pHeader->status = PI_INVALID_PACKETVERSION_ERROR;
                rc = PI_INVALID_PACKETVERSION_ERROR;
                dprintf(DPRINTF_DEFAULT, "Command %d Packet Version Not supported\n",
                        pReqPacket->pHeader->commandCode);
            }
            break;

        case PI_ISE_GET_STATUS_CMD:
            if (pReqPacket->pHeader->packetVersion != 0 &&
                pReqPacket->pHeader->packetVersion != 1 &&
                pReqPacket->pHeader->packetVersion != 2)
            {
                pRspPacket->pHeader->status = PI_INVALID_PACKETVERSION_ERROR;
                rc = PI_INVALID_PACKETVERSION_ERROR;
                dprintf(DPRINTF_DEFAULT, "Command %d Packet Version Not supported\n",
                        pReqPacket->pHeader->commandCode);
            }
            break;

        default:
            if (pReqPacket->pHeader->packetVersion != 0 &&
                pReqPacket->pHeader->packetVersion != 1)
            {
                pRspPacket->pHeader->status = PI_INVALID_PACKETVERSION_ERROR;
                rc = PI_INVALID_PACKETVERSION_ERROR;
                dprintf(DPRINTF_DEFAULT, "Command %d Packet Version Not supported\n",
                        pReqPacket->pHeader->commandCode);
            }
            break;
    }

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    ParmCheckIfPowerUpComplete
**
** Description: Check to see if the power-up processing has completed.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
static INT32 ParmCheckIfPowerUpComplete(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = GOOD;

    /*
     * Only certain commands are allowed prior to the controller being
     * initially configured.  Check if the command we are executing is
     * one of those commands.  Additionally, check if we are attempting
     * to execute the PI_VCG_CONFIGURE_CMD after it has already been
     * configured.
     */
    switch (pReqPacket->pHeader->commandCode)
    {
        case PI_CONNECT_CMD:
        case PI_POWER_UP_STATE_CMD:
        case PI_PING_CMD:
        case PI_VCG_INFO_CMD:
        case PI_ADMIN_SETTIME_CMD:
        case PI_ADMIN_GETTIME_CMD:
        case PI_LOG_CLEAR_CMD:
            break;

        case PI_VCG_CONFIGURE_CMD:
            /*
             * If the controller is already configured,
             * donot allow PI_VCG_CONFIGURE_CMD.
             */
            if (IsConfigured())
            {
                rc = ERROR;
                pRspPacket->pHeader->length = 0;
                pRspPacket->pHeader->status = PI_ERROR;
                pRspPacket->pHeader->errorCode = EC_VCG_ALREADY_CONFIGURED;
            }
            break;

        default:
            /*
             * If the controller is not configured yet,
             * do not allow any other commands.
             */
            if (!IsConfigured())
            {
                rc = ERROR;
                pRspPacket->pHeader->length = 0;
                pRspPacket->pHeader->status = PI_POWER_UP_REQ_ERROR;
                pRspPacket->pHeader->errorCode = 0;
            }
            break;
    }

    if (rc != GOOD)
    {
        return (rc);
    }

    /*
     * If we are executing a command that will require the power-up
     * processing to be complete we need to make sure that it has
     * indeed completed.  The individual cases below do NOT require
     * power up complete.  All others do.
     */
    switch (pReqPacket->pHeader->commandCode)
    {
        case PI_CONNECT_CMD:
        case PI_PING_CMD:
        case PI_RESET_CMD:
        case PI_POWER_UP_STATE_CMD:
        case PI_POWER_UP_RESPONSE_CMD:
        case PI_REGISTER_CLIENT_TYPE_CMD:
        case PI_X1_COMPATIBILITY_INDEX_CMD:
        case PI_PDISK_COUNT_CMD:
        case PI_PDISK_LIST_CMD:
        case PI_PDISK_INFO_CMD:
        case PI_PDISKS_CMD:
        case PI_PDISKS_FROM_CACHE_CMD:
        case PI_VDISK_COUNT_CMD:
        case PI_VDISK_LIST_CMD:
        case PI_VDISK_INFO_CMD:
        case PI_VDISKS_CMD:
        case PI_VDISKS_FROM_CACHE_CMD:
        case PI_VLINK_NAME_CHANGED_CMD:
        case PI_DEBUG_STRUCT_DISPLAY_CMD:
        case PI_DEBUG_GET_ELECTION_STATE_CMD:
        case PI_DEBUG_GET_STATE_RM_CMD:
        case PI_LOG_CLEAR_CMD:
        case PI_VCG_INFO_CMD:
        case PI_DISK_BAY_COUNT_CMD:
        case PI_DISK_BAY_LIST_CMD:
        case PI_MISC_COUNT_CMD:
        case PI_MISC_LIST_CMD:
        case PI_DISK_BAYS_CMD:
        case PI_FIRMWARE_DOWNLOAD_CMD:
        case PI_TRY_CCB_FW_CMD:
        case PI_LOG_TEXT_MESSAGE_CMD:
        case PI_GENERIC_CMD:
        case PI_GENERIC2_CMD:
        case PI_GENERIC_MRP_CMD:
        case PI_WCACHE_INVALIDATE_CMD:
        case PI_MISC_GET_DEVICE_COUNT_CMD:
        case PI_MISC_FILE_SYSTEM_READ_CMD:
        case PI_MISC_FILE_SYSTEM_WRITE_CMD:
        case PI_MISC_FAILURE_STATE_SET_CMD:
        case PI_DEBUG_REPORT_CMD:
        case PI_RAIDS_CMD:
        case PI_RAIDS_FROM_CACHE_CMD:
        case PI_ADMIN_FW_VERSIONS_CMD:
        case PI_ADMIN_FW_SYS_REL_LEVEL_CMD:
        case PI_ADMIN_SETTIME_CMD:
        case PI_ADMIN_GETTIME_CMD:
        case PI_ADMIN_LEDCNTL_CMD:
        case PI_ADMIN_SET_IP_CMD:
        case PI_ADMIN_GET_IP_CMD:
        case PI_DEBUG_MEM_RDWR_CMD:
        case PI_DEBUG_INIT_PROC_NVRAM_CMD:
        case PI_DEBUG_GET_SER_NUM_CMD:
        case PI_DEBUG_MRMMTEST_CMD:
        case PI_DEBUG_INIT_CCB_NVRAM_CMD:
        case PI_DEBUG_SCSI_COMMAND_CMD:
        case PI_DEBUG_READWRITE_CMD:
        case PI_VCG_VALIDATION_CMD:
        case PI_VCG_INACTIVATE_CONTROLLER_CMD:
        case PI_VCG_FAIL_CONTROLLER_CMD:
        case PI_VCG_PREPARE_SLAVE_CMD:
        case PI_VCG_PING_CMD:
        case PI_VCG_APPLY_LICENSE_CMD:
        case PI_VCG_CONFIGURE_CMD:
        case PI_VCG_GET_MP_LIST_CMD:
        case PI_ENABLE_X1_PORT_CMD:
        case PI_DISK_BAY_INFO_CMD:
        case PI_LOG_INFO_CMD:
        case PI_PROC_BE_DEVICE_PATH_CMD:
        case PI_PROC_BE_PORT_LIST_CMD:
        case PI_PROC_FE_PORT_LIST_CMD:
        case PI_PROC_NAME_DEVICE_CMD:
        case PI_PROC_START_IO_CMD:
        case PI_PROC_STOP_IO_CMD:
        case PI_PROC_ASSIGN_MIRROR_PARTNER_CMD:
        case PI_PROC_FAIL_CTRL_CMD:
        case PI_MISC_RESCAN_DEVICE_CMD:
        case PI_MISC_GET_FE_DEVICE_LIST_CMD:
        case PI_MISC_GET_BE_DEVICE_LIST_CMD:
        case PI_MISC_GET_MODE_CMD:
        case PI_MISC_SET_MODE_CMD:
        case PI_MISC_INVALIDATE_BE_WC_CMD:
        case PI_MISC_MIRROR_PARTNER_CONTROL_CMD:
        case PI_MISC_CONTINUE_WO_MP_CMD:
        case PI_STATS_ENVIRONMENTAL_CMD:
        case PI_ENV_II_GET_DATA_CMD:
        case PI_STATS_HAB_CMD:
        case PI_STATS_FRONT_END_PROC_CMD:
        case PI_STATS_BACK_END_PROC_CMD:
        case PI_STATS_PROC_CMD:
        case PI_STATS_GLOBAL_CACHE_CMD:
        case PI_PERSISTENT_DATA_CONTROL_CMD:
        case PI_REGISTER_EVENTS_CMD:
        case PI_CUSTOMER_LOG_ACKNOWLEDGE_CMD:
        case PI_MISC_SERIAL_NUMBER_SET_CMD:
        case PI_MISC_RESYNC_MIRROR_RECORDS_CMD:
        case PI_MFG_CTRL_CLEAN_CMD:
        case PI_MULTI_PART_XFER_CMD:
        case PI_TARGET_INFO_CMD:
        case PI_TARGET_COUNT_CMD:
        case PI_TARGET_LIST_CMD:
        case PI_TARGETS_CMD:
        case PI_MISC_GET_WORKSET_INFO_CMD:
        case PI_MISC_SET_WORKSET_INFO_CMD:
        case PI_SET_DLM_HEARTBEAT_LIST_CMD:
        case PI_CACHE_FLUSH_BE_CMD:
        case PI_RAID_MIRRORING_CMD:
        case PI_RAID_RECOVER_CMD:
        case PI_MISC_RESYNC_RAIDS_CMD:
        case PI_MISC_PUTDEVCONFIG_CMD:
        case PI_MISC_GETDEVCONFIG_CMD:
        case PI_MISC_QUERY_MP_CHANGE_CMD:
        case PI_MISC_RESYNCDATA_CMD:
        case PI_MISC_RESYNCCTL_CMD:
        case PI_STATS_BUFFER_BOARD_CMD:
        case PI_MISC_MIRROR_PARTNER_GET_CFG_CMD:
        case PI_BATTERY_HEALTH_SET_CMD:
        case PI_MISC_INVALIDATE_FE_WC_CMD:
        case PI_MISC_SETTDISCACHE_CMD:
        case PI_MISC_CLRTDISCACHE_CMD:
        case PI_MISC_QTDISCACHE_CMD:
        case PI_MISC_CFGOPTION_CMD:
        case PI_STATS_FRONT_END_LOOP_CMD:
        case PI_STATS_BACK_END_LOOP_CMD:
#ifndef DISABLE_LOCAL_RAID_MONITORING
        case PI_MISC_LOCAL_RAID_INFO_CMD:
#endif  /* DISABLE_LOCAL_RAID_MONITORING */
        case PI_VDISK_BAY_REDUNDANT_CMD:
        case PI_PDISKS_QLOGIC_TIMEOUT_EMULATE:
        case PI_PROC_RESET_BE_QLOGIC_CMD:
            break;

            /*
             * WARNING: DO NOT INCLUDE ANY CASE STATEMENT AFTER THIS
             * THE CODE IS FALLING THROUGH FOR ELSE CONDITION
             */
        case PI_CLIENT_PERSISTENT_DATA_CONTROL_CMD:
            /*
             * Allow the command only in case of read and list
             */
            if (((((PI_CLIENT_DATA_CONTROL_REQ *)(pReqPacket->pPacket))->option == CLIENT_OPTION_READ_RECORD) ||
                 (((PI_CLIENT_DATA_CONTROL_REQ *)(pReqPacket->pPacket))->option == CLIENT_OPTION_LIST_RECORDS)) &&
                (((PI_CLIENT_DATA_CONTROL_REQ *)(pReqPacket->pPacket))->flags == 0))
            {
                break;
            }
        default:
            /*
             * If power-up has not completed set the error code
             * else we can continue
             */
            if (!PowerUpComplete())
            {
                rc = ERROR;
                pRspPacket->pHeader->length = 0;
                pRspPacket->pHeader->status = PI_POWER_UP_REQ_ERROR;
                pRspPacket->pHeader->errorCode = 0;
            }
            break;
    }

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    ParmCheckIfMaster
**
** Description: Check to see if we are the master controller
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
static INT32 ParmCheckIfMaster(UNUSED XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = GOOD;

    /*
     *    if we are not master set the error code
     *    else we are and can continue
     */
    if (!(TestforMaster(GetMyControllerSN())))
    {
        rc = ERROR;
        pRspPacket->pHeader->length = 0;
        pRspPacket->pHeader->status = PI_MASTER_CNT_ERROR;
        pRspPacket->pHeader->errorCode = 0;
    }

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    ParmCheckIfElectionInProgress
**
** Description: Check to see if we are the master controller
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
static INT32 ParmCheckIfElectionInProgress(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = GOOD;

    /*
     * If we are executing a command that will require the power-up
     * processing to be complete we need to make sure that it has
     * indeed completed.  The individual cases below do NOT require
     * power up complete.  All others do.
     */
    switch (pReqPacket->pHeader->commandCode)
    {
        case PI_VCG_REMOVE_CONTROLLER_CMD:
            if (!TestforMaster(GetMyControllerSN()))
            {
                break;
            }

            /* Fall through if master */

        case PI_SERVER_ASSOCIATE_CMD:
        case PI_PDISK_LABEL_CMD:
        case PI_PDISK_FAIL_CMD:
        case PI_RAID_CONTROL_CMD:
        case PI_PDISK_UNFAIL_CMD:
        case PI_PDISK_DELETE_CMD:
        case PI_SERVER_CREATE_CMD:
        case PI_SERVER_DELETE_CMD:
        case PI_SERVER_DISASSOCIATE_CMD:
        case PI_SERVER_SET_PROPERTIES_CMD:
        case PI_VLINK_CREATE_CMD:
        case PI_TARGET_SET_PROPERTIES_CMD:
        case PI_VDISK_CREATE_CMD:
        case PI_VDISK_SET_PRIORITY_CMD:
        case PI_VCG_VDISK_PRIORITY_ENABLE_CMD:
        case PI_VDISK_PR_CLR_CMD:
        case PI_VDISK_PR_GET_CMD:
#if ISCSI_CODE
        case PI_ISCSI_SET_TGTPARAM_CMD:
        case PI_ISCSI_TGT_INFO_CMD:
        case PI_ISCSI_SET_CHAP_CMD:
        case PI_ISCSI_CHAP_INFO:
        case PI_ISCSI_SESSION_INFO:
        case PI_ISCSI_SESSION_INFO_SERVER:
        case PI_GETISNSINFO_CMD:
        case PI_SETISNSINFO_CMD:
        case PI_IDD_INFO_CMD:
        case PI_DLM_PATH_STATS_CMD:
        case PI_DLM_PATH_SELECTION_ALGO_CMD:
#endif  /* ISCSI_CODE */
        case PI_VDISK_EXPAND_CMD:
        case PI_VDISK_DELETE_CMD:
        case PI_VDISK_CONTROL_CMD:
        case PI_QUICK_BREAK_PAUSE_RESUME_MIRROR_START_CMD:
        case PI_QUICK_BREAK_PAUSE_RESUME_MIRROR_SEQUENCE_CMD:
        case PI_QUICK_BREAK_PAUSE_RESUME_MIRROR_EXECUTE_CMD:
        case PI_BATCH_SNAPSHOT_START_CMD:
        case PI_BATCH_SNAPSHOT_SEQUENCE_CMD:
        case PI_BATCH_SNAPSHOT_EXECUTE_CMD:
        case PI_VDISK_SET_ATTRIBUTE_CMD:
        case PI_DEBUG_INIT_PROC_NVRAM_CMD:
        case PI_DEBUG_INIT_CCB_NVRAM_CMD:
        case PI_PROC_RESTORE_NVRAM_CMD:
        case PI_VCG_PREPARE_SLAVE_CMD:
        case PI_VCG_ADD_SLAVE_CMD:
        case PI_VCG_INACTIVATE_CONTROLLER_CMD:
        case PI_VCG_ACTIVATE_CONTROLLER_CMD:
        case PI_VCG_SET_CACHE_CMD:
        case PI_VCG_APPLY_LICENSE_CMD:
        case PI_VCG_UNFAIL_CONTROLLER_CMD:
        case PI_VCG_FAIL_CONTROLLER_CMD:
        case PI_VCG_CONFIGURE_CMD:
        case PI_PDISK_SPINDOWN_CMD:
        case PI_PDISK_AUTO_FAILBACK_ENABLE_DISABLE_CMD:
        case PI_PDISK_FAILBACK_CMD:

        case PI_SET_GEO_LOCATION_CMD:
        case PI_CLEAR_GEO_LOCATION_CMD:
        case PI_DISK_BAY_DELETE_CMD:
            /*
             * If an election is in progress set the error code
             * else we can continue
             */
            if (EL_TestInProgress() == TRUE)
            {
                rc = ERROR;
                pRspPacket->pHeader->length = 0;
                pRspPacket->pHeader->status = PI_ELECTION_ERROR;
                pRspPacket->pHeader->errorCode = 0;
            }

        default:
            break;
    }

    return (rc);
}


/**
******************************************************************************
**
**  @brief      Check if the command requires that there are no RAID 5 stripe
**              resync operations in progress and if so check if there are
**              those operations in progrss.
**
**  @param      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
**  @return     GOOD if the command can continue (RAID 5 stripe resync
**              operations are not in progress, ERROR otherwise.
**
******************************************************************************
**/
static INT32 ParmCheckIfR5StripeResyncInProgress(XIO_PACKET *pReqPacket,
                                                 XIO_PACKET *pRspPacket)
{
    INT32       rc = GOOD;

    /*
     * If we are executing a command that will require the power-up
     * processing to be complete we need to make sure that it has
     * indeed completed.  The individual cases below do NOT require
     * power up complete.  All others do.
     */
    switch (pReqPacket->pHeader->commandCode)
    {
        case PI_VCG_REMOVE_CONTROLLER_CMD:
            if (!TestforMaster(GetMyControllerSN()))
            {
                break;
            }

            /* Fall through if master */

        case PI_VDISK_CREATE_CMD:
        case PI_VDISK_SET_PRIORITY_CMD:
        case PI_VCG_VDISK_PRIORITY_ENABLE_CMD:
        case PI_VDISK_DELETE_CMD:
        case PI_VDISK_EXPAND_CMD:
        case PI_VDISK_CONTROL_CMD:
        case PI_QUICK_BREAK_PAUSE_RESUME_MIRROR_START_CMD:
        case PI_QUICK_BREAK_PAUSE_RESUME_MIRROR_SEQUENCE_CMD:
        case PI_QUICK_BREAK_PAUSE_RESUME_MIRROR_EXECUTE_CMD:
        case PI_BATCH_SNAPSHOT_START_CMD:
        case PI_BATCH_SNAPSHOT_SEQUENCE_CMD:
        case PI_BATCH_SNAPSHOT_EXECUTE_CMD:
        case PI_VDISK_SET_ATTRIBUTE_CMD:
        case PI_VCG_PREPARE_SLAVE_CMD:
        case PI_VCG_ADD_SLAVE_CMD:
        case PI_VCG_INACTIVATE_CONTROLLER_CMD:
        case PI_VCG_ACTIVATE_CONTROLLER_CMD:
        case PI_VCG_UNFAIL_CONTROLLER_CMD:
        case PI_VCG_FAIL_CONTROLLER_CMD:
        case PI_VCG_SHUTDOWN_CMD:
            /*
             * If any RAID 5 stripe resync operations are in progress
             * set the error code, otherwise continue
             */
            if (RM_R5StripeResyncInProgress())
            {
                rc = ERROR;
                pRspPacket->pHeader->length = 0;
                pRspPacket->pHeader->status = PI_R5_STRIPE_RESYNC_ERROR;
                pRspPacket->pHeader->errorCode = 0;
            }
        default:
            break;
    }

    return (rc);
}

#ifndef DISABLE_LOCAL_RAID_MONITORING

/**
******************************************************************************
**
**  @brief      Check if the command requires that there are no Local/Vcg
**              resync operations in progress and if so check if there are
**              those operations in progrss.
**
**  @param      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
**  @return     GOOD if the command can continue (RAID 5 stripe resync
**              operations are not in progress, ERROR otherwise.
**
******************************************************************************
**/
static INT32 ParmCheckIfLocalRaidsResyncing(XIO_PACKET *pReqPacket,
                                            XIO_PACKET *pRspPacket)
{
    INT32       rc = GOOD;

    /*
     * If we are executing a command that will require the power-up
     * processing to be complete we need to make sure that it has
     * indeed completed.  The individual cases below do NOT require
     * power up complete.  All others do.
     */
    switch (pReqPacket->pHeader->commandCode)
    {
            /*
             * Cases we only care about our own controller.
             */
        case PI_SNAPSHOT_LOAD_CMD:
        case PI_MFG_CTRL_CLEAN_CMD:

/* We really want the controller to go down -- let it, regardless. */

            /*        case PI_VCG_INACTIVATE_CONTROLLER_CMD: *//* this controller is to go down */
            /*        case PI_PROC_FAIL_CTRL_CMD: *//* other controller is going down */

            /*
             * Check that there are no local resyncs happening on this controller.
             * XK_RaidMonitorIsResyncing() returns FALSE if all is well.
             */
            if (XK_RaidMonitorIsResyncing() != FALSE)
            {
                pRspPacket->pHeader->length = 0;
                pRspPacket->pHeader->status = PI_LOCAL_RAID_RESYNC_ERROR;
                pRspPacket->pHeader->errorCode = 0;
                rc = ERROR;
                dprintf(DPRINTF_DEFAULT, "ParmCheckIfLocalRaidsResyncing: Local SCSI Resync in progress.\n");
            }
            break;

            /*
             * Cases we care about any controllers in our DSC.
             */
        case PI_VCG_ADD_SLAVE_CMD:
        case PI_VCG_SHUTDOWN_CMD:
        case PI_RESET_CMD:
        case PI_ADMIN_SET_IP_CMD:
        case PI_ROLLING_UPDATE_PHASE_CMD:
            /*
             * Check that there are no local resyncs happening on any controllers.
             * 0 = No controllers,
             * else bitmap of controllers resyncing.
             * XK_RaidMonitorIsResyncing() returns FALSE if all is well.
             */
            if (XK_RaidMonitorResyncingControllers() != 0)
            {
                pRspPacket->pHeader->length = 0;
                pRspPacket->pHeader->status = PI_LOCAL_RAID_RESYNC_ERROR;
                pRspPacket->pHeader->errorCode = 0;
                rc = ERROR;
                dprintf(DPRINTF_DEFAULT, "ParmCheckIfLocalRaidsResyncing: DSC SCSI Resync in progress.\n");
            }
            break;

        default:
            break;
    }

    return (rc);
}
#endif /* DISABLE_LOCAL_RAID_MONITORING */


/*----------------------------------------------------------------------------
** Function:    ParmCheckPowerUpResponse
**
** Description: Check the parameters for the PI_POWER_UP_RESPONSE_CMD
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
static INT32 ParmCheckPowerUpResponse(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = GOOD;
    UINT8       parmErrorPos = 0;
    UINT16      state = POWER_UP_START;
    UINT16      astatus = POWER_UP_ASTATUS_UNKNOWN;
    UINT8       response = 0;

    /*
     *    Retrieve the parameters to check
     */
    state = ((PI_POWER_UP_RESPONSE_REQ *)(pReqPacket->pPacket))->state;
    astatus = ((PI_POWER_UP_RESPONSE_REQ *)(pReqPacket->pPacket))->astatus;
    response = ((PI_POWER_UP_RESPONSE_REQ *)(pReqPacket->pPacket))->response;

    if (state != GetPowerUpState())
    {
        parmErrorPos = 1;
    }

    if (parmErrorPos == 0)
    {
        switch (state)
        {
            case POWER_UP_WAIT_FWV_INCOMPATIBLE:
            case POWER_UP_WAIT_PROC_COMM:
                if (response != POWER_UP_RESPONSE_RESET)
                {
                    parmErrorPos = 3;
                }
                break;

            case POWER_UP_WAIT_DRIVES:
                if (response != POWER_UP_RESPONSE_RESET &&
                    response != POWER_UP_RESPONSE_CONTINUE)
                {
                    parmErrorPos = 3;
                }
                break;

            case POWER_UP_WAIT_DISASTER:
                if (response != POWER_UP_RESPONSE_CONTINUE &&
                    response != POWER_UP_RESPONSE_CONTINUE_KA)
                {
                    parmErrorPos = 3;
                }
                break;

            case POWER_UP_WAIT_CONTROLLERS:
            case POWER_UP_WAIT_DISK_BAY:
            case POWER_UP_WAIT_CORRUPT_BE_NVRAM:
                if (response != POWER_UP_RESPONSE_RETRY &&
                    response != POWER_UP_RESPONSE_CONTINUE)
                {
                    parmErrorPos = 3;
                }
                break;

            case POWER_UP_WAIT_CACHE_ERROR:
                if (astatus != GetPowerUpAStatus())
                {
                    parmErrorPos = 2;
                }
                else
                {
                    if (response != POWER_UP_RESPONSE_WC_SAVE &&
                        response != POWER_UP_RESPONSE_WC_DISCARD)
                    {
                        parmErrorPos = 3;
                    }
                }
                break;

            default:
                parmErrorPos = 1;
                break;
        }
    }

    /*
     *    if there was an error in one of the parameters set the status
     *    and error code and set rc to an error state
     */
    if (parmErrorPos != 0)
    {
        rc = ERROR;
        pRspPacket->pHeader->length = 0;
        pRspPacket->pHeader->status = PI_PARAMETER_ERROR;
        pRspPacket->pHeader->errorCode = parmErrorPos;
    }

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    ParmCheckPdiskPiLabelPhy
**
** Description: Check the parameters in a Pdisk call using stucture
**              request PI_LABEL_PHY
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
static INT32 ParmCheckPdiskPiLabelPhy(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = GOOD;
    UINT8       labelType = 0;
    UINT8       labelOption = 0;
    UINT8       parmErrorPos = 0;

    /*
     *    Retrieve the parameters to check
     */
    labelType = ((PI_PDISK_LABEL_REQ *)(pReqPacket->pPacket))->labtype;
    labelOption = ((PI_PDISK_LABEL_REQ *)(pReqPacket->pPacket))->option;

    /*
     *    We are not checking the pid, that will be handled underneath us
     *    pid == parameter 1
     */

    /*
     *    if the label type is not one of the defined types set the header
     *    status to PI_PARAMETER_ERROR and pass back the position of the
     *    invalid parmeter in the Error Code.  See MR_Defs.h for types
     */
    switch (labelType)
    {
        case MLDNOLABEL:
        case MLDDATALABEL:
        case MLDSPARELABEL:
        case MLDNDATALABEL:
        case MLDFIXDNAME:
        case MLDRELABEL:
            break;              /* parameter is valid */

        default:
            parmErrorPos = 2;   /* parameter is invalid */
            break;
    }

    /*
     *    if the label option is not one of the defined types set the header
     *    status to PI_PARAMETER_ERROR and pass back the position of the
     *    invalid parmeter in the Error Code.  See MR_Defs.h for types
     */
    if (parmErrorPos == 0)
    {
        if (labelOption == 0 ||
            (labelOption & MLD_OPT_FULL) ||
            (labelOption & MLD_OPT_FSYS) || (labelOption & MLD_OPT_DUPLICATE))
        {
            /* parameter is valid */
        }
        else
        {
            parmErrorPos = 3;   /* parameter is invalid */
        }
    }

    /*
     *    if there was an error in one of the parameters set the status
     *    and error code and set rc to an error state
     */
    if (parmErrorPos != 0)
    {
        rc = ERROR;
        pRspPacket->pHeader->length = 0;
        pRspPacket->pHeader->status = PI_PARAMETER_ERROR;
        pRspPacket->pHeader->errorCode = parmErrorPos;
    }

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    ParmCheckDlmPathSelectionReq
**
** Description: Check the parameters in selecting DLM Path selection  using stucture
**              request PI_SET_GEO_LOCATION_REQ
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
static INT32 ParmCheckDlmPathSelectionReq(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = GOOD;
    UINT8       algoType = 0;
    UINT8       parmErrorPos = 0;

    /*
     *    Retrieve the parameters to check
     */
    algoType = ((PI_DLM_PATH_SELECTION_ALGO_REQ *)(pReqPacket->pPacket))->algoType;

    if (algoType > 3)
    {
        parmErrorPos = 2;       /* parameter is invalid */
    }

    /*
     *    if there was an error in one of the parameters set the status
     *    and error code and set rc to an error state
     */
    if (parmErrorPos != 0)
    {
        rc = ERROR;
        pRspPacket->pHeader->length = 0;
        pRspPacket->pHeader->status = PI_PARAMETER_ERROR;
        pRspPacket->pHeader->errorCode = parmErrorPos;
    }
    return (rc);
}



/*----------------------------------------------------------------------------
** Function:    ParmCheckSetGeoLocation
**
** Description: Check the parameters in a set Geo location  using stucture
**              request PI_SET_GEO_LOCATION_REQ
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
static INT32 ParmCheckSetGeoLocation(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = GOOD;
    UINT8       geoLocation = 0;
    UINT8       parmErrorPos = 0;

    /*
     *    Retrieve the parameters to check
     */
    geoLocation = ((PI_SET_GEO_LOCATION_REQ *)(pReqPacket->pPacket))->locationId;

    /*
     *    if the location ID is not one of the allowed types ( default location ID
     *    is zero, which means no geo location is set )set the header
     *    status to PI_PARAMETER_ERROR and pass back the position of the
     *    invalid parmeter in the Error Code
     */
    if (0 == geoLocation)
    {
        parmErrorPos = 2;       /* parameter is invalid */
    }

    /*
     *    if there was an error in one of the parameters set the status
     *    and error code and set rc to an error state
     */
    if (parmErrorPos != 0)
    {
        rc = ERROR;
        pRspPacket->pHeader->length = 0;
        pRspPacket->pHeader->status = PI_PARAMETER_ERROR;
        pRspPacket->pHeader->errorCode = parmErrorPos;
    }

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    ParmCheckPdiskPiFailBackPdisk
**
** Description: Check the parameters in a Pdisk call using stucture
**              request PI_PDISK_FAILBACK_REQ
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
static INT32 ParmCheckPdiskPiFailBackPdisk(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = GOOD;
    UINT8       failBackDiskOptions = 0;
    UINT8       parmErrorPos = 0;

    /*
     *    Retrieve the parameters to check
     */
    failBackDiskOptions = ((PI_PDISK_FAILBACK_REQ *)(pReqPacket->pPacket))->options;

    /*
     *    if the option is not one of the allowed types set the header
     *    status to PI_PARAMETER_ERROR and pass back the position of the
     *    invalid parmeter in the Error Code
     */
    switch (failBackDiskOptions)
    {
        case 0x00:             /* ignore options failback the  pid */
        case FBP_FAILBACK_CANCEL:      /* cancel the failback opion forever */
            /* Any of the combination options we have to check -- in future when added more options */
            break;              /* parameter is valid */

        default:
            parmErrorPos = 2;   /* parameter is invalid */
            break;
    }
    /*
     *    if there was an error in one of the parameters set the status
     *    and error code and set rc to an error state
     */
    if (parmErrorPos != 0)
    {
        rc = ERROR;
        pRspPacket->pHeader->length = 0;
        pRspPacket->pHeader->status = PI_PARAMETER_ERROR;
        pRspPacket->pHeader->errorCode = parmErrorPos;
    }

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    ParmCheckPdiskPiFailPdisk
**
** Description: Check the parameters in a Pdisk call using stucture
**              request PI_PDISK_FAIL_REQ
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
static INT32 ParmCheckPdiskPiFailPdisk(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = GOOD;
    UINT8       failDiskOptions = 0;
    UINT8       parmErrorPos = 0;

    /*
     *    Retrieve the parameters to check
     */
    failDiskOptions = ((PI_PDISK_FAIL_REQ *)(pReqPacket->pPacket))->options;

    /*
     *    if the option is not one of the allowed types set the header
     *    status to PI_PARAMETER_ERROR and pass back the position of the
     *    invalid parmeter in the Error Code
     */
    switch (failDiskOptions)
    {
        case 0x00:             /* ignore redundancy, fail pid */
        case FP_FORCE:         /* enforce redundancy, fail pid */
        case FP_USE_HSPID:     /* ignore redundancy, use hotspare pid */
        case FP_FAILLABEL:
        case FP_SPIN_DOWN:

#ifdef SERVICEABILITY42

/*  <TBD> Following check is wrong.. need to verify */
#endif  /* SERVICEABILITY42 */
            /* Any of the options together */
        case (FP_FORCE | FP_USE_HSPID | FP_FAILLABEL | FP_SPIN_DOWN):
            break;              /* parameter is valid */

        default:
            parmErrorPos = 2;   /* parameter is invalid */
            break;
    }

    /*
     *    if there was an error in one of the parameters set the status
     *    and error code and set rc to an error state
     */
    if (parmErrorPos != 0)
    {
        rc = ERROR;
        pRspPacket->pHeader->length = 0;
        pRspPacket->pHeader->status = PI_PARAMETER_ERROR;
        pRspPacket->pHeader->errorCode = parmErrorPos;
    }

    return (rc);
}


/*----------------------------------------------------------------------------
** Function:    ParmCheckPdiskPiPdiskDeleteReq
**
** Description: Check the parameters in a Pdisk call using stucture
**              request PI_PDISK_DELETE_REQ
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
static INT32 ParmCheckPdiskPiPdiskDeleteReq(XIO_PACKET *pReqPacket,
                                            XIO_PACKET *pRspPacket)
{
    INT32       rc = GOOD;
    UINT8       parmErrorPos = 0;
    UINT8       type = -1;

    type = ((PI_PDISK_DELETE_REQ *)(pReqPacket->pPacket))->type;

    /*
     *    Check to make sure that the type is a disk drive delete
     */
    if (type != DELETE_DEVICE_DRIVE)
    {
        parmErrorPos = 1;
    }

    if (parmErrorPos != 0)
    {
        rc = ERROR;
        pRspPacket->pHeader->length = 0;
        pRspPacket->pHeader->status = PI_PARAMETER_ERROR;
        pRspPacket->pHeader->errorCode = parmErrorPos;
    }

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    ParmCheckVdiskPiCreateVdisk
**
** Description: Check the parameters in a Vdisk call using stucture
**              request PI_CREATE_VDISK
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
static INT32 ParmCheckVdiskPiCreateVdisk(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = GOOD;
    UINT8       raidType = 0;
    UINT8       operation = 0;
    UINT16      numOfDrives = 0;
    UINT16      stripe = 0;
    UINT8       depth = 0;
    UINT8       parity = 0;
    UINT8       parmErrorPos = 0;

    /*
     *    Retrieve the parameters to check
     */
    raidType = ((PI_VDISK_CREATE_REQ *)(pReqPacket->pPacket))->rtype;
    operation = ((PI_VDISK_CREATE_REQ *)(pReqPacket->pPacket))->op;
    numOfDrives = ((PI_VDISK_CREATE_REQ *)(pReqPacket->pPacket))->drives;
    stripe = ((PI_VDISK_CREATE_REQ *)(pReqPacket->pPacket))->stripe;
    depth = ((PI_VDISK_CREATE_REQ *)(pReqPacket->pPacket))->depth;
    parity = ((PI_VDISK_CREATE_REQ *)(pReqPacket->pPacket))->parity;

    /*
     *    We are not checking the vid, that will be handled underneath us
     *    vid == parameter 3
     */
    /*
     *    We are not checking the devcap,
     *    devcap == parameter 6
     */
    /*
     *    We are not checking the rsvd,
     *    rsvd == parameter 9
     */

    /*
     *    if the operation command is invalid set the header
     *    status to PI_PARAMETER_ERROR and pass back the position of the
     *    invalid parmeter in the Error Code.
     */
    switch (operation)
    {
        case 0x00:             /* Create a Virtual Disk */
        case 0x01:             /* Expand a Virtual Disk */
        case 0x02:             /* Test Parameters */
            break;              /* parameter is valid */

        default:
            parmErrorPos = 2;   /* parameter is invalid */
            break;
    }

    /*
     *    we must check parameters based on the type of raid device
     *    unless the operation is to test the parameters
     */
    if (parmErrorPos == 0)
    {
        switch (raidType)
        {
                /*
                 * Standard
                 */
            case 0x00:

                /*
                 * Standard RAID may only have 1 drive
                 * but it is handled underneath
                 */
                break;

                /*
                 * RAID 0
                 */
            case 0x01:

                /*
                 * RAID 0 must have more than 1 drive
                 */
                if (numOfDrives <= 0x0001)
                {
                    parmErrorPos = 4;
                }

                /*
                 * Check for valid stripe size
                 * RAID 0 valid stripe size is from
                 * 8 to 512 in powers of 2
                 */
                if (parmErrorPos == 0)
                {
                    switch (stripe)
                    {
                        case 0x0008:   /* sector count 8 */
                        case 0x0010:   /* sector count 16 */
                        case 0x0020:   /* sector count 32 */
                        case 0x0040:   /* sector count 64 */
                        case 0x0080:   /* sector count 128 */
                        case 0x0100:   /* sector count 256 */
                        case 0x0200:   /* sector count 512 */
                            break;      /* parameter is valid */

                        default:
                            parmErrorPos = 5;   /* parameter is invalid */
                            break;
                    }
                }
                break;

                /*
                 * RAID 1
                 */
            case 0x02:

                /*
                 * RAID 1 must have more than 1 drive
                 */
                if (numOfDrives <= 0x0001)
                {
                    parmErrorPos = 4;
                }

                /*
                 * RAID 1 depth must less then or equal to the number of drives
                 */
                if ((parmErrorPos == 0) && (depth > numOfDrives))
                {
                    parmErrorPos = 7;   /* parameter is invalid */
                }
                break;

                /*
                 * RAID 5
                 */
            case 0x03:

                /*
                 * RAID 5 must have a parity of 3, 5, or 9
                 */
                switch (parity)
                {
                    case 0x03: /* 3 is 1/3 */
                    case 0x05: /* 5 is 1/5 */
                    case 0x09: /* 9 is 1/9 */
                        break;  /* parameter is valid */

                    default:
                        parmErrorPos = 8;       /* parameter is invalid */
                        break;
                }

                /*
                 * Check for valid number of drives for RAID 5
                 * RAID 5 valid number of drives must be
                 * greater than or equal to the parity
                 */
                if ((parmErrorPos == 0) && (numOfDrives < parity))
                {
                    parmErrorPos = 4;   /* parameter is invalid */
                }

                /*
                 * Check for valid stripe size
                 * RAID 5 valid stripe size is from
                 * 8 to 64 in powers of 2
                 */
                if (parmErrorPos == 0)
                {
                    switch (stripe)
                    {
                        case 0x0008:   /* sector count 8 */
                        case 0x0010:   /* sector count 16 */
                        case 0x0020:   /* sector count 32 */
                        case 0x0040:   /* sector count 64 */
                            break;      /* parameter is valid */

                        default:
                            parmErrorPos = 5;   /* parameter is invalid */
                            break;
                    }
                }
                break;

                /*
                 * RAID 10
                 */
            case 0x04:

                /*
                 * RAID 10 must have more than 1 drive
                 */
                if (numOfDrives <= 0x0001)
                {
                    parmErrorPos = 4;
                }

                /*
                 * RAID 10 depth must be less than or equal
                 * to the number of drives
                 */
                if ((parmErrorPos == 0) && (depth > numOfDrives))
                {
                    parmErrorPos = 7;   /* parameter is invalid */
                }

                /*
                 * Check for valid stripe size
                 * RAID 10 valid stripe size is from
                 * 8 to 512 in powers of 2
                 */
                if (parmErrorPos == 0)
                {
                    switch (stripe)
                    {
                        case 0x0008:   /* sector count 8 */
                        case 0x0010:   /* sector count 16 */
                        case 0x0020:   /* sector count 32 */
                        case 0x0040:   /* sector count 64 */
                        case 0x0080:   /* sector count 128 */
                        case 0x0100:   /* sector count 256 */
                        case 0x0200:   /* sector count 512 */
                            break;      /* parameter is valid */

                        default:
                            parmErrorPos = 5;   /* parameter is invalid */
                            break;
                    }
                }
                break;

                /*
                 * Default
                 */
            default:
                parmErrorPos = 1;       /* parameter is invalid */
                break;
        }
    }

    /*
     *    if their are identical drives in the dmap set the header
     *    status to PI_PARAMETER_ERROR and pass back the position of the
     *    invalid parmeter in the Error Code.
     */
    if ((CheckDuplicateID
         (numOfDrives, ((PI_VDISK_CREATE_REQ *)(pReqPacket->pPacket))->dmap) == ERROR) ||
        (numOfDrives == 0))
    {
        parmErrorPos = 10;
    }

    /*
     *    if there was an error in one of the parameters set the status
     *    and error code and set rc to an error state
     */
    if (parmErrorPos != 0)
    {
        rc = ERROR;
        pRspPacket->pHeader->length = 0;
        pRspPacket->pHeader->status = PI_PARAMETER_ERROR;
        pRspPacket->pHeader->errorCode = parmErrorPos;
    }

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    ParmCheckVdiskPiVdiskSetPriority
**
** Description: Check the parameters in a Vdisk call using stucture
**              request PI_CREATE_VDISK
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
static INT32 ParmCheckVdiskPiVdiskSetPriority(UNUSED XIO_PACKET *pReqPacket,
                                              UNUSED XIO_PACKET *pRspPacket)
{
    return (GOOD);
}

/*----------------------------------------------------------------------------
** Function:    ParmCheckVdiskPiVdiskCtl
**
** Description: Check the parameters in a Vdisk call using stucture
**              request PI_VDISK_CONTROL_REQ
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
static INT32 ParmCheckVdiskPiVdiskCtl(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = GOOD;
    UINT8       operation = 0;
    UINT8       parmErrorPos = 0;

    /*
     *    Retrieve the parameters to check
     */
    operation = ((PI_VDISK_CONTROL_REQ *)(pReqPacket->pPacket))->subtype;

    /*
     *    We are not checking the svid, that will be handled underneath us
     *    svid == parameter 2
     */
    /*
     *    We are not checking the dvid, that will be handled underneath us
     *    dvid == parameter 3
     */

    /*
     *    if the operation is not one of the allowed types set the header
     *    status to PI_PARAMETER_ERROR and pass back the position of the
     *    invalid parmeter in the Error Code
     */
    switch (operation)
    {
        case MVCMOVEVD:        /* Move virtual device */
        case MVCCOPYBRK:       /* Start copy and break */
        case MVCCOPYSWAP:      /* Start copy and swap */
        case MVCCOPYCONT:      /* Start continuous copy */
        case MVCSWAPVD:        /* Append svid's RAIDs to dvid, delete svid */
        case MVCXSPECCOPY:     /* Break off specified copy */
        case MVCPAUSECOPY:     /* Pause a copy */
        case MVCRESUMECOPY:    /* Resume a copy */
        case MVCABORTCOPY:     /* Abort a copy */
        case MVCXCOPY:         /* Break off all copies */
        case MVCSLINK:         /* Create a Snapshot */
            break;              /* parameter is valid */

        default:
            parmErrorPos = 1;   /* parameter is invalid */
    }

    /*
     *    if there was an error in one of the parameters set the status
     *    and error code and set rc to an error state
     */
    if (parmErrorPos != 0)
    {
        rc = ERROR;
        pRspPacket->pHeader->length = 0;
        pRspPacket->pHeader->status = PI_PARAMETER_ERROR;
        pRspPacket->pHeader->errorCode = parmErrorPos;
    }

    return (rc);
}


/*----------------------------------------------------------------------------
** Function:    ParmCheckVdiskPiSetAttribute
**
** Description: Check the parameters in a Vdisk call using stucture
**              request PI_VDISK_SET_ATTRIBUTE_REQ
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
static INT32 ParmCheckVdiskPiSetAttribute(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = GOOD;
    UINT8       attr = 0;
    UINT8       parmErrorPos = 0;

    /*
     *    Retrieve the parameters to check
     */
    attr = ((PI_VDISK_SET_ATTRIBUTE_REQ *)(pReqPacket->pPacket))->attrRequest.attr;

    /*
     *    We are not checking the vid, that will be handled underneath us
     *    vid == parameter 1
     */

    /*
     * There are currently two "sets" of VDisk attributes.  The low order
     * 2 bits are used to indicate normal, hidden or private.  Valid values
     * are 0x00, 0x01 and 0x02.  The top 2 bits are flags to indicate lock
     * (0x40) and VLink (0x80).  Since the flags are independent of the
     * attributes and reserved fields are not checked
     */
    if ((attr & 0x03) == 0x03)
    {
        parmErrorPos = 1;       /* parameter is invalid */
    }

    /*
     *    if there was an error in one of the parameters set the status
     *    and error code and set rc to an error state
     */
    if (parmErrorPos != 0)
    {
        rc = ERROR;
        pRspPacket->pHeader->length = 0;
        pRspPacket->pHeader->status = PI_PARAMETER_ERROR;
        pRspPacket->pHeader->errorCode = parmErrorPos;
    }

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    ParmCheckProcPiGetDevicePath
**
** Description: Check the parameters in a Reset call using stucture
**              request MRGETBEDEVPATHS_REQ
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
static INT32 ParmCheckProcPiGetDevicePath(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = GOOD;
    UINT16      type = 0;
    UINT16      format = 0;
    UINT8       parmErrorPos = 0;

    /*
     *    Retrieve the parameters to check
     */
    type = ((MRGETBEDEVPATHS_REQ *)(pReqPacket->pPacket))->type;
    format = ((MRGETBEDEVPATHS_REQ *)(pReqPacket->pPacket))->format;

    /*
     *    if the option is not one of the allowed types set the header
     *    status to PI_PARAMETER_ERROR and pass back the position of the
     *    invalid parmeter in the Error Code
     */
    switch (type)
    {
            /*
             * Defined in MR_Defs.h
             */
        case PATH_PHYSICAL_DISK:
        case PATH_MISC_DEVICE:
        case PATH_ENCLOSURES:
            break;              /* parameter is valid */

        default:
            parmErrorPos = 1;   /* parameter is invalid */
            break;
    }

    if (parmErrorPos != 0)
    {
        /*
         *    if the option is not one of the allowed types set the header
         *    status to PI_PARAMETER_ERROR and pass back the position of the
         *    invalid parmeter in the Error Code
         */
        switch (format)
        {
                /*
                 * Defined in MR_Defs.h
                 */
            case FORMAT_PID_BITPATH:
            case FORMAT_PID_PATH_ARRAY:
                break;          /* parameter is valid */

            default:
                parmErrorPos = 2;       /* parameter is invalid */
                break;
        }
    }

    /*
     *    if there was an error in one of the parameters set the status
     *    and error code and set rc to an error state
     */
    if (parmErrorPos != 0)
    {
        rc = ERROR;
        pRspPacket->pHeader->length = 0;
        pRspPacket->pHeader->status = PI_PARAMETER_ERROR;
        pRspPacket->pHeader->errorCode = parmErrorPos;
    }

    return (rc);
}


/*----------------------------------------------------------------------------
** Function:    ParmCheckProcPiGetPortList
**
** Description: Check the parameters in a Reset call using stucture
**              request PI_GET_PORT_LIST
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
static INT32 ParmCheckProcPiGetPortList(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = GOOD;
    UINT16      type = 0;
    UINT8       parmErrorPos = 0;

    /*
     *    Retrieve the parameters to check
     */
    type = ((PI_PORT_LIST_REQ *)(pReqPacket->pPacket))->type;

    /*
     *    if the option is not one of the allowed types set the header
     *    status to PI_PARAMETER_ERROR and pass back the position of the
     *    invalid parmeter in the Error Code
     */
    switch (type)
    {
            /*
             * Defined in MR_Defs.h
             */
        case PORTS_ALL:
        case PORTS_INITIALIZED:
        case PORTS_FAILED:
        case PORTS_INITIALIZING:
        case PORTS_GOOD:
        case PORTS_ONLINE:
        case PORTS_OFFLINE:
        case PORTS_FAILMARK:
        case PORTS_STATUS:
            break;              /* parameter is valid */

            /*
             * Thes two cases are good for FE only, so we need to check
             */
        case PORTS_WITH_TARGETS:
        case PORTS_NO_TARGETS:
            if (pReqPacket->pHeader->commandCode == PI_PROC_BE_PORT_LIST_CMD)
            {
                parmErrorPos = 1;       /* parameter is invalid */
            }
            break;

        default:
            parmErrorPos = 1;   /* parameter is invalid */
            break;
    }

    /*
     *    if there was an error in one of the parameters set the status
     *    and error code and set rc to an error state
     */
    if (parmErrorPos != 0)
    {
        rc = ERROR;
        pRspPacket->pHeader->length = 0;
        pRspPacket->pHeader->status = PI_PARAMETER_ERROR;
        pRspPacket->pHeader->errorCode = parmErrorPos;
    }

    return (rc);
}


/*----------------------------------------------------------------------------
** Function:    ParmCheckResetPiResetQlogic
**
** Description: Check the parameters in a Reset call using stucture
**              request PI_RESET_QLOGIC
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
static INT32 ParmCheckResetPiResetQlogic(UNUSED XIO_PACKET *pReqPacket,
                                         UNUSED XIO_PACKET *pRspPacket)
{
    INT32       rc = GOOD;

/*
** We are not checking option at this time
*/
#if 0
    UINT16      option = 0;
    UINT8       parmErrorPos = 0;

    /*
     *    Retrieve the parameters to check
     */
    option = ((PI_RESET_PROC_QLOGIC_REQ *)(pReqPacket->pPacket))->option;

    /*
     *    We are not checking the target, that will be handled underneath us
     *    target == parameter 1
     */

    /*
     *    if the option is not one of the allowed types set the header
     *    status to PI_PARAMETER_ERROR and pass back the position of the
     *    invalid parmeter in the Error Code
     */
    switch (option)
    {
            /*
             * Defined in MR_Defs.h
             */
        case RESET_PORT_INIT:
        case RESET_PORT_NO_INIT:
        case RESET_PORT_INIT_IF_OFFLINE:
        case RESET_PORT_NO_INIT_IF_OFFLINE:
        case RESET_PORT_INIT_LOG:
        case RESET_PORT_NO_INIT_LOG:
        case RESET_PORT_FORCE_SYS_ERR:
        case RESET_PORT_MEMDUMP:

            break;              /* parameter is valid */

        default:

            parmErrorPos = 3;   /* parameter is invalid */
            break;
    }

    /*
     *    if there was an error in one of the parameters set the status
     *    and error code and set rc to an error state
     */
    if (parmErrorPos != 0)
    {
        rc = ERROR;
        pRspPacket->pHeader->length = 0;
        pRspPacket->pHeader->status = PI_PARAMETER_ERROR;
        pRspPacket->pHeader->errorCode = parmErrorPos;
    }
#endif   /* 0 */
    return (rc);
}


/*----------------------------------------------------------------------------
** Function:    ParmCheckAdminPiFwVersionReq
**
** Description: Check the parameters in a Admin call using stucture
**              request PI_FW_VERSION_REQ
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
static INT32 ParmCheckAdminPiFwVersionReq(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    UINT16      fwType;

    fwType = ((PI_FW_VERSION_REQ *)(pReqPacket->pPacket))->fwType;

    /*
     *    If the fwType is not one of the allowed types set the header
     *    status to PI_PARAMETER_ERROR and pass back the position of the
     *    invalid parmeter in the Error Code
     */
    switch (fwType)
    {
    case GET_BE_RUNTIME_FW:     /* BE runtime */
    case GET_FE_RUNTIME_FW:     /* FE runtime */
    case GET_CCB_RUNTIME_FW:    /* CCB runtime */
        break;                  /* Parameter is valid */

    default:                    /* Parameter is invalid */
        pRspPacket->pHeader->length = 0;
        pRspPacket->pHeader->status = PI_PARAMETER_ERROR;
        pRspPacket->pHeader->errorCode = 1;
        return ERROR;
    }

    return GOOD;
}


/*----------------------------------------------------------------------------
** Function:    ParmCheckAdminPiScrubIn
**
** Description: Check the parameters in a Admin call using stucture
**              request PI_RAID_CONTROL_REQ.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
static INT32 ParmCheckAdminPiScrubIn(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = GOOD;
    UINT32      scrubcontrol = 0;
    UINT32      paritycontrol = 0;
    UINT8       parmErrorPos = 0;

    /*
     *    Retrieve the parameters to check
     */
    scrubcontrol = ((PI_RAID_CONTROL_REQ *)(pReqPacket->pPacket))->scrubcontrol;
    paritycontrol = ((PI_RAID_CONTROL_REQ *)(pReqPacket->pPacket))->paritycontrol;

    /*
     * If the scrubcontrol is not one of the defined types set the header
     * status to PI_PARAMETER_ERROR and pass back the position of the
     * invalid parmeter in the Error Code.  See MR_Defs.h for types
     */
    switch (scrubcontrol)
    {
        case SCRUB_POLL:
        case SCRUB_CHANGE:
        case SCRUB_CHANGE + SCRUB_ENABLE:
            break;              /* parameter is valid */

        default:
            parmErrorPos = 1;   /* parameter is invalid */
            break;
    }

    /*
     * If the paritycontrol is not one of the defined types set the header
     * status to PI_PARAMETER_ERROR and pass back the position of the
     * invalid parmeter in the Error Code.  See MR_Defs.h for types
     *
     *    if we are just polling the information do not check this parameter
     * MRSDEBUG: Don't check parityScanControl for now.
     */
#if 0
    if ((parmErrorPos == 0) && (scrubcontrol != medpoll))
    {
        switch (paritycontrol)
        {
            case PC_CORRUPT_MASK:
            case PC_ENABLE_MASK:
            case PC_SPECIFIC_MASK:
            case PC_CLEARLOG_MASK:
            case PC_1PASS_MASK:
            case PC_CORRECT_MASK:

                break;          /* parameter is valid */

            default:

                parmErrorPos = 2;       /* parameter is invalid */
                break;
        }
    }
#endif   /* 0 */

    /*
     *    if there was an error in one of the parameters set the status
     *    and error code and set rc to an error state
     */
    if (parmErrorPos != 0)
    {
        rc = ERROR;
        pRspPacket->pHeader->length = 0;
        pRspPacket->pHeader->status = PI_PARAMETER_ERROR;
        pRspPacket->pHeader->errorCode = parmErrorPos;
    }

    return (rc);
}


/*----------------------------------------------------------------------------
** Function:    ParmCheckDebugPiDebugMemRdwrReq
**
** Description: Check the parameters in a Debug call using stucture
**              request PI_DEBUG_MEM_RDWR_REQ
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
static INT32 ParmCheckDebugPiDebugMemRdwrReq(XIO_PACKET *pReqPacket,
                                             XIO_PACKET *pRspPacket)
{
    INT32       rc = GOOD;
    UINT16      processor = 0;
    UINT16      mode = 0;
    UINT8       parmErrorPos = 0;

    /*
     *    Retrieve the parameters to check
     */
    processor = ((PI_DEBUG_MEM_RDWR_REQ *)(pReqPacket->pPacket))->processor;
    mode = ((PI_DEBUG_MEM_RDWR_REQ *)(pReqPacket->pPacket))->mode;

    /*
     *    We are not checking the length,
     *    length == parameter 2
     */
    /*
     *    We are not checking the data,
     *    data == parameter 5
     */

    /*
     *    if the pAddr is NULL set the header
     *    status to PI_PARAMETER_ERROR and pass back the position of the
     *    invalid parmeter in the Error Code.
     */
    if (((PI_DEBUG_MEM_RDWR_REQ *)(pReqPacket->pPacket))->pAddr == NULL)
    {
        parmErrorPos = 1;
    }

    /*
     *    if the processor is not one of the defined types set the header
     *    status to PI_PARAMETER_ERROR and pass back the position of the
     *    invalid parmeter in the Error Code.  See PacketInterface.h for types
     */
    if (parmErrorPos == 0)
    {
        switch (processor)
        {
            case PROCESS_CCB:
            case PROCESS_FE:
            case PROCESS_BE:
                break;          /* parameter is valid */

            default:
                parmErrorPos = 3;       /* parameter is invalid */
                break;
        }
    }

    /*
     *    if the mode is not one of the defined types set the header
     *    status to PI_PARAMETER_ERROR and pass back the position of the
     *    invalid parmeter in the Error Code.  See PacketInterface.h for types
     */
    if (parmErrorPos == 0)
    {
        switch (mode)
        {
            case MEM_READ:
            case MEM_WRITE:
                break;          /* parameter is valid */

            default:
                parmErrorPos = 4;       /* parameter is invalid */
                break;
        }
    }

    /*
     *    if there was an error in one of the parameters set the status
     *    and error code and set rc to an error state
     */
    if (parmErrorPos != 0)
    {
        rc = ERROR;
        pRspPacket->pHeader->length = 0;
        pRspPacket->pHeader->status = PI_PARAMETER_ERROR;
        pRspPacket->pHeader->errorCode = parmErrorPos;
    }

    return (rc);
}


/*----------------------------------------------------------------------------
** Function:    ParmCheckDebugPiDebugGetSerialNumReq
**
** Description: Check the parameters in a Debug call using stucture
**              request PI_DEBUG_GET_SERIAL_NUM_REQ
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
static INT32 ParmCheckDebugPiDebugGetSerialNumReq(XIO_PACKET *pReqPacket,
                                                  XIO_PACKET *pRspPacket)
{
    INT32       rc = GOOD;
    UINT8       type = 0;
    UINT8       parmErrorPos = 0;

    /*
     *    Retrieve the parameters to check
     */
    type = ((PI_DEBUG_GET_SERIAL_NUM_REQ *)(pReqPacket->pPacket))->type;

    /*
     *    if the type is not one of the allowed types set the header
     *    status to PI_PARAMETER_ERROR and pass back the position of the
     *    invalid parmeter in the Error Code.  See serial_num.h for types
     */
    switch (type)
    {
        case CONTROLLER_SN:
        case SYSTEM_SN:
            break;              /* parameter is valid */

        default:
            parmErrorPos = 1;   /* parameter is invalid */
            break;
    }

    /*
     *    if there was an error in one of the parameters set the status
     *    and error code and set rc to an error state
     */
    if (parmErrorPos != 0)
    {
        rc = ERROR;
        pRspPacket->pHeader->length = 0;
        pRspPacket->pHeader->status = PI_PARAMETER_ERROR;
        pRspPacket->pHeader->errorCode = parmErrorPos;
    }

    return (rc);
}


/*----------------------------------------------------------------------------
** Function:    ParmCheckVcgPiSetCache
**
** Description: Check the parameters in a Vdisk call using stucture
**              request PI_VCG_SET_CACHE_REQ
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
static INT32 ParmCheckVcgPiSetCache(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = GOOD;
    UINT8       mode = 0;
    UINT8       parmErrorPos = 0;

    /*
     *    Retrieve the parameters to check
     */
    mode = ((PI_VCG_SET_CACHE_REQ *)(pReqPacket->pPacket))->mode;

    /*
     *    if the mode is not one of the allowed types set the header
     *    status to PI_PARAMETER_ERROR and pass back the position of the
     *    invalid parmeter in the Error Code.  See MR_Defs.h for mode types
     */
    switch (mode)
    {
        case MSCGOFF:
        case MSCGON:
            break;              /* parameter is valid */

        default:
            parmErrorPos = 1;   /* parameter is invalid */
            break;
    }

    /*
     *    if there was an error in one of the parameters set the status
     *    and error code and set rc to an error state
     */
    if (parmErrorPos != 0)
    {
        rc = ERROR;
        pRspPacket->pHeader->length = 0;
        pRspPacket->pHeader->status = PI_PARAMETER_ERROR;
        pRspPacket->pHeader->errorCode = parmErrorPos;
    }

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    ParmCheckVcgPiVcgApplyLicenseReq
**
** Description: Check the parameters in a VCG call using stucture
**              request PI_VCG_APPLY_LICENSE_REQ
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
static INT32 ParmCheckVcgPiVcgApplyLicenseReq(XIO_PACKET *pReqPacket,
                                              XIO_PACKET *pRspPacket)
{
    INT32       rc = GOOD;
    UINT8       parmErrorPos = 0;
    PI_VCG_APPLY_LICENSE_REQ *pRequest = NULL;
    UINT32      vcgID = 0;
    UINT16      vcgMaxNumControllers = 0;

    pRequest = (PI_VCG_APPLY_LICENSE_REQ *)pReqPacket->pPacket;

    /*
     *    Retrieve the parameters to check
     */
    vcgID = pRequest->vcgID;
    vcgMaxNumControllers = pRequest->vcgMaxNumControllers;

    /*
     * If the VCGID is greater than zero it means that we are setting a new
     * virtual controller group ID.  The user is allowed to set the VCGID if
     * the current VCGID is zero or if the new VCGID is the same as the old
     * VCGID.
     */
    if (vcgID > 0 &&
        Qm_GetVirtualControllerSN() != 0 && vcgID != Qm_GetVirtualControllerSN())
    {
        parmErrorPos = 1;
    }

    if (parmErrorPos == 0)
    {
        /*
         * If the maximum number of controllers is less than the current number
         * of controllers in the group it is a problem.
         *
         * If the slot of this controller will not fit in the list of
         * valid controllers (0-vcgMaxNumControllers-1) then reject
         * the license.
         */
        if (vcgMaxNumControllers < ACM_GetActiveControllerCount(Qm_ActiveCntlMapPtr()))
        {
            parmErrorPos = 2;
        }
        else if (Qm_SlotFromSerial(GetMyControllerSN()) >= vcgMaxNumControllers)
        {
            parmErrorPos = 2;
        }
    }

    /*
     *    if there was an error in one of the parameters set the status
     *    and error code and set rc to an error state
     */
    if (parmErrorPos != 0)
    {
        rc = ERROR;
        pRspPacket->pHeader->length = 0;
        pRspPacket->pHeader->status = PI_PARAMETER_ERROR;
        pRspPacket->pHeader->errorCode = parmErrorPos;
    }

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    ParmCheckDiskBayPiDiskBayDeleteReq
**
** Description: Check the parameters in a Pdisk call using stucture
**              request PI_DISK_BAY_DELETE_REQ
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
static INT32 ParmCheckDiskBayPiDiskBayDeleteReq(XIO_PACKET *pReqPacket,
                                                XIO_PACKET *pRspPacket)
{
    INT32       rc = GOOD;
    UINT8       parmErrorPos = 0;
    UINT8       type = -1;

    type = ((PI_DISK_BAY_DELETE_REQ *)(pReqPacket->pPacket))->type;

    /*
     *    Check to make sure that the type is a disk bay delete
     */
    if (type != DELETE_DEVICE_BAY)
    {
        parmErrorPos = 1;
    }

    if (parmErrorPos != 0)
    {
        rc = ERROR;
        pRspPacket->pHeader->length = 0;
        pRspPacket->pHeader->status = PI_PARAMETER_ERROR;
        pRspPacket->pHeader->errorCode = parmErrorPos;
    }

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    ParmCheckLogPiLogInfoReq
**
** Description: Check the parameters in a Log call using stucture
**              request PI_LOG_INFO_REQ
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
static INT32 ParmCheckLogPiLogInfoReq(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = GOOD;
    UINT16      mode = 0;
    UINT8       parmErrorPos = 0;

    /*
     *    Retrieve the parameters to check
     */
    mode = ((PI_LOG_INFO_REQ *)(pReqPacket->pPacket))->mode;

    /*
     *    We are not checking the eventCount,
     *    eventCount == parameter 1
     */
    /*
     *    We are not checking the sequenceNumber,
     *    sequenceNumber == parameter 3
     */

    /*
     *    if the mode is not one of the allowed types set the header
     *    status to PI_PARAMETER_ERROR and pass back the position of the
     *    invalid parmeter in the Error Code.
     *    mode uses the last 3 bits to determine which mode to use
     *      bit 0   0 = send in ASCII format
     *              1 = send in binary format
     *      bit 1   0 = no extended data (ASCII)
     *              1 = send extended data (ASCII)
     *      bit 2   0 = use newest event as start
     *              1 = use sequence number as starting event
     */
    if (mode >= MODE_END_OF_LOGS)
    {
        parmErrorPos = 2;
    }

    /*
     *    if there was an error in one of the parameters set the status
     *    and error code and set rc to an error state
     */
    if (parmErrorPos != 0)
    {
        rc = ERROR;
        pRspPacket->pHeader->length = 0;
        pRspPacket->pHeader->status = PI_PARAMETER_ERROR;
        pRspPacket->pHeader->errorCode = parmErrorPos;
    }

    return (rc);
}


/*----------------------------------------------------------------------------
** Function:    ParmCheckWriteBuffer
**
** Description: Check the parameters in a write buffer call.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
INT32 ParmCheckWriteBuffer(UNUSED XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket,
                           UINT8 *pBuf)
{
    INT32       rc = GOOD;
    UINT8       parmErrorPos = 0;
    UINT32      i;
    PI_WRITE_BUFFER_MODE5_REQ *datP = (PI_WRITE_BUFFER_MODE5_REQ *)pBuf;

    /*
     * Make sure count is within range
     */
    if (datP->count == 0 || datP->count > 1024)
    {
        parmErrorPos = 1;
    }

    /*
     * Check WWN's and Luns
     */
    for (i = 0; i < datP->count; i++)
    {
        if ((datP->wwnLun[i].wwn == 0) || (datP->wwnLun[i].lun > 63))
        {
            parmErrorPos = i + 2;
            break;
        }
    }

    /*
     *    if there was an error in one of the parameters set the status
     *    and error code and set rc to an error state
     */
    if (parmErrorPos != 0)
    {
        rc = ERROR;
        pRspPacket->pHeader->length = 0;
        pRspPacket->pHeader->status = PI_PARAMETER_ERROR;
        pRspPacket->pHeader->errorCode = parmErrorPos;
    }

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    CheckDuplicateID
**
** Description: Checks an array of ID's for duplicate values.
**
** Inputs:      count - number of id's in list
**              idList - array of count id's to be checked
**
** Returns:     GOOD if there are no duplicates in idList
**              ERROR if there are duplicates in list
**
**--------------------------------------------------------------------------*/
static INT32 CheckDuplicateID(UINT16 count, UINT16 idList[])
{
    INT32       rc = GOOD;
    UINT16      index1 = 0;
    UINT16      index2 = 0;

    if (count > 1)
    {
        for (index1 = 0; index1 < (count - 1); ++index1)
        {
            for (index2 = (index1 + 1); index2 < count; ++index2)
            {
                if (idList[index1] == idList[index2])
                {
                    rc = ERROR;
                    index1 = count;
                    index2 = count;
                }
            }
        }
    }
    return rc;
}


/*----------------------------------------------------------------------------
** Function:    ParmCheckFailUnfailInterfaceReq
**
** Description: Check the parameters in a Fail or Unfail Interface call using
**              the request parms
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     GOOD or ERROR
**
**--------------------------------------------------------------------------*/
static INT32 ParmCheckFailUnfailInterfaceReq(XIO_PACKET *pReqPacket,
                                             XIO_PACKET *pRspPacket)
{
    INT32       rc = GOOD;
    UINT8       interface = 0;
    UINT8       parmErrorPos = 0;

    /*
     * Retrieve the parameters to check.  Even though separate request
     * structures are defined for fail and unfail, they are the same.
     */
    interface = ((PI_MISC_UNFAIL_INTERFACE_REQ *)(pReqPacket->pPacket))->interface;

    /*
     * We are not checking the controllerSN.  Just check that the interface
     * is in range.  With a max of 4 interfaces valid values are 0 - 3.
     */
    if (interface > 3)
    {
        parmErrorPos = 2;       /* parameter is invalid */
    }

    /*
     *    if there was an error in one of the parameters set the status
     *    and error code and set rc to an error state
     */
    if (parmErrorPos != 0)
    {
        rc = ERROR;
        pRspPacket->pHeader->length = 0;
        pRspPacket->pHeader->status = PI_PARAMETER_ERROR;
        pRspPacket->pHeader->errorCode = parmErrorPos;
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
