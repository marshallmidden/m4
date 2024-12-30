/* $Id: PktCmdHdl.c 160950 2013-04-22 21:10:28Z marshall_midden $ */
/*===========================================================================
** FILE NAME:       PktCmdHdl.c
** MODULE TITLE:    Packet Command Handler - Implementation
**
** DESCRIPTION:     Server that handles packet communication between the
**                  CCB and the XMC.
**
** Copyright (c) 2001-2009 Xiotech Corporation. All rights reserved.
**==========================================================================*/
#include "PktCmdHdl.h"
#include "CacheManager.h"
#include "CachePDisk.h"
#include "cps_init.h"
#include "debug_files.h"
#include "globalOptions.h"
#include "LOG_Defs.h"
#include "errorCodes.h"
#include "FIO.h"
#include "ipc_common.h"
#include "kernel.h"
#include "LogOperation.h"
#include "MR_Defs.h"
#include "nvram.h"
#include "PacketInterface.h"
#include "PI_CmdHandlers.h"
#include "PI_Utils.h"
#include "quorum.h"
#include "quorum_utils.h"
#include "rm.h"
#include "sm.h"
#include "Snapshot.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"
#include "XIO_Types.h"

/*****************************************************************************
** Private defines
*****************************************************************************/
#define TMO_PI_MISC_RESCAN_DEVICE_CMD       180000      /* 180 second tmo   */
#define TMO_PI_PROC_RESTORE_NVRAM_CMD       90000       /* 90  second tmo   */
#define TMO_PI_PROC_TARGET_CONTROL_CMD      30000       /* 30  second tmo   */
#define TMO_PI_MISC_SETTDISCACHE_CMD        60000       /* 60  second tmo   */
#define TMO_PI_MISC_CLRTDISCACHE_CMD        60000       /* 60  second tmo   */
#define TMO_PI_PROC_RESETQLOGIC_CMD         20000       /* 20  second tmo   */

/*****************************************************************************
** Private variables
*****************************************************************************/
static UINT32 ownedDrivesBeforeLabel;

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static void GetLabelOption(PI_PDISK_LABEL_REQ *pRequest);
static INT32 ValidatePhysicalDiskUnlabel(PI_PDISK_LABEL_REQ *pRequest);

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
** Function:    PacketCommandHandlerPreProcessImpl
**
** Description: Implements the Packet Command Handler pre-processing function
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD, PI_ERROR or PI_TIMEOUT
**
**--------------------------------------------------------------------------*/
INT32 PacketCommandHandlerPreProcessImpl(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;

    /*
     * Prior to sending a PDisk Label request we need to determine what option
     * should be used.  The call to GetLabelOption will fill in the option
     * field in the request packet if everything is successful.  If everything
     * is not successful then it will set the error information in the response
     * packet and return an error.
     */
    if (pReqPacket->pHeader->commandCode == PI_PDISK_LABEL_CMD)
    {
        PI_PDISK_LABEL_REQ *pRequest = NULL;

        pRequest = (PI_PDISK_LABEL_REQ *)pReqPacket->pPacket;

        /*
         * Determine the number of drives that we own.  This will help
         * determine if we will be able to get a valid file system from
         * another drive.
         */
        ownedDrivesBeforeLabel = CPSInitGetOwnedDriveCount(GetSerialNumber(SYSTEM_SN));

        /*
         * If the requested operation is to unlabel drives then we
         * need to validate that the user is not attempting to
         * unlabel all labeled drives when in a multiple controller
         * configuration.
         */
        if (pRequest->labtype == MLDNOLABEL)
        {
            /* Run the validation. */
            rc = ValidatePhysicalDiskUnlabel(pRequest);

            /* If error occurred, make sure response packet contains error. */
            if (rc != PI_GOOD)
            {
                /*
                 * There is only one reason why the validate would
                 * fail, the user is trying to unlabel all owned
                 * drives when in a multiple controller configuration
                 * so set the error code appropriately.
                 */
                pRspPacket->pHeader->status = rc;
                pRspPacket->pHeader->errorCode = EC_UNLABEL_ALL_OWNED;
            }
        }

        /*
         * If the validation passed or was not required we need to
         * setup the option field in the request.
         */
        if (rc == PI_GOOD)
        {
            /*
             * Get the label option for this label request.  This checks
             * the label class and the current number of drives labeled
             * and determines the correct option.  The option is entered
             * into the request packet.
             */
            GetLabelOption(pRequest);
        }
    }

    return rc;
}

/*----------------------------------------------------------------------------
** Function:    PacketCommandHandlerImpl
**
** Description: Call a handler function based on the request packet.
**              Results are returned in the response packet.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD, PI_ERROR or PI_TIMEOUT
**
**--------------------------------------------------------------------------*/
INT32 PacketCommandHandlerImpl(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    PI_CommandHandler_t PI_CommandHandler = NULL;
    UINT32      rc;
    UINT16      mrpCmd = 0;
    UINT32      rspDataSz = 0;
    UINT32      timeout;

    /*
     * If the request packet has specified a timeout value use it
     * rather then the global timeout value.
     *
     * NOTE: The following commands have a minimum timeout value
     *       that will be used instead of either of these values.
     *          - PI_PDISK_UNFAIL_CMD
     *          - PI_VLINK_CREATE_CMD
     *          - PI_VDISK_CREATE_CMD
     *          - PI_VDISK_EXPAND_CMD
     *          - PI_VDISK_PREPARE_CMD
     *          - PI_DEBUG_INIT_PROC_NVRAM_CMD
     *          - PI_PROC_RESTORE_NVRAM_CMD
     *          - PI_MISC_RESCAN_DEVICE_CMD
     */
    if (pReqPacket->pHeader->timeout != 0)
    {
        timeout = pReqPacket->pHeader->timeout;
    }
    else
    {
        /*
         * MRSDEBUG: For test purposes allow the MRP timeout value to be
         * set using a generic command.  THis applies ONLY to MRPs that
         * would normally use MRP_STD_TIMEOUT.
         */
        timeout = GetGlobalMRPTimeout();
    }

    /*
     * If the request packet length != 0 but the request pointer is NULL,
     * there must have been an error allocating the request packet.
     * Bail out now and return an error.
     */
    if ((pReqPacket->pHeader->length) && (pReqPacket->pPacket == NULL))
    {
        pRspPacket->pHeader->status = PI_MALLOC_ERROR;
        return PI_MALLOC_ERROR;
    }

#ifdef FOLLOW_PI_EXECUTION
if (pReqPacket->pHeader->commandCode != 0x11e && pReqPacket->pHeader->commandCode != 0x1ff && pReqPacket->pHeader->commandCode != 0x5ff) {
  dprintf(DPRINTF_DEFAULT, "%s:%u:%s Starting PI commandCode=0x%x (%d)\n", __FILE__, __LINE__, __func__, pReqPacket->pHeader->commandCode,pReqPacket->pHeader->commandCode);
}
#endif  /* FOLLOW_PI_EXECUTION */
    switch (pReqPacket->pHeader->commandCode)
    {
        case PI_CONNECT_CMD:
            PI_CommandHandler = PI_Connect;
            break;

        case PI_PING_CMD:
            PI_CommandHandler = PI_Ping;
            break;

        case PI_RESET_CMD:
            PI_CommandHandler = PI_Reset;
            break;

        case PI_POWER_UP_STATE_CMD:
            PI_CommandHandler = PI_PowerUpState;
            break;

        case PI_POWER_UP_RESPONSE_CMD:
            PI_CommandHandler = PI_PowerUpResponse;
            break;

        case PI_X1_COMPATIBILITY_INDEX_CMD:
            PI_CommandHandler = PI_X1CompatibilityIndex;
            break;

        case PI_ENABLE_X1_PORT_CMD:
            PI_CommandHandler = PI_EnableX1Server;
            break;

        case PI_GET_BACKEND_TYPE_CMD:
            PI_CommandHandler = PI_GetBackendType;
            break;

        case PI_REGISTER_EVENTS_CMD:
            PI_CommandHandler = PI_RegisterEvents;
            break;

        case PI_PDISK_COUNT_CMD:
        case PI_VDISK_COUNT_CMD:
        case PI_RAID_COUNT_CMD:
        case PI_SERVER_COUNT_CMD:
        case PI_TARGET_COUNT_CMD:
        case PI_DISK_BAY_COUNT_CMD:
        case PI_MISC_COUNT_CMD:
            PI_CommandHandler = PI_GetObjectCount;
            break;

        case PI_PDISK_LIST_CMD:
        case PI_VDISK_LIST_CMD:
        case PI_RAID_LIST_CMD:
        case PI_SERVER_LIST_CMD:
        case PI_TARGET_LIST_CMD:
        case PI_DISK_BAY_LIST_CMD:
        case PI_MISC_LIST_CMD:
        case PI_PROC_BE_PORT_LIST_CMD:
        case PI_PROC_FE_PORT_LIST_CMD:
            PI_CommandHandler = PI_GetObjectList;
            break;

        case PI_DISK_BAY_INFO_CMD:
            mrpCmd = MRGETEINFO;
            rspDataSz = sizeof(PI_PDISK_INFO_RSP);
            break;

        case PI_DISK_BAY_DELETE_CMD:
            timeout = MAX(timeout, TMO_NONE);
            mrpCmd = MRDELETEDEVICE;
            rspDataSz = sizeof(PI_DISK_BAY_DELETE_RSP);
            break;

        case PI_DISK_BAYS_CMD:
            PI_CommandHandler = PI_DiskBays;
            break;

        case PI_DISK_BAY_ALARM_CTRL_CMD:
            PI_CommandHandler = PI_DiskBayAlarmControl;
            break;

        case PI_PDISK_INFO_CMD:
            mrpCmd = MRGETPINFO;
            rspDataSz = sizeof(PI_PDISK_INFO_RSP);
            break;

        case PI_PDISK_LABEL_CMD:
            timeout = MAX(timeout, TMO_NONE);
            PI_CommandHandler = PI_PDiskLabel;
            break;

        case PI_PDISK_DEFRAG_CMD:
            PI_CommandHandler = PI_PDiskDefrag;
            break;

        case PI_PDISK_DEFRAG_STATUS_CMD:
            PI_CommandHandler = PI_PDiskDefragStatus;
            break;

        case PI_PDISK_FAIL_CMD:
            timeout = MAX(timeout, TMO_NONE);
            mrpCmd = MRFAIL;
            rspDataSz = sizeof(PI_PDISK_FAIL_RSP);
            break;

        case PI_PDISK_UNFAIL_CMD:
            timeout = MAX(timeout, TMO_NONE);
            mrpCmd = MRRESTOREDEV;
            rspDataSz = sizeof(PI_PDISK_UNFAIL_RSP);
            break;

        case PI_PDISK_BEACON_CMD:
            PI_CommandHandler = PI_PDiskBeacon;
            break;

        case PI_PDISK_DELETE_CMD:
            timeout = MAX(timeout, TMO_NONE);
            mrpCmd = MRDELETEDEVICE;
            rspDataSz = sizeof(PI_PDISK_DELETE_RSP);
            break;

        case PI_PDISK_BYPASS_CMD:
            PI_CommandHandler = PI_PDiskBypass;
            break;

        case PI_PDISKS_QLOGIC_TIMEOUT_EMULATE:
            PI_CommandHandler = PI_PDiskQLTimeout;
            break;

        case PI_SET_GEO_LOCATION_CMD:
            timeout = MAX(timeout, TMO_NONE);
            mrpCmd = MRSETGLINFO;
            rspDataSz = sizeof(PI_SET_GEO_LOCATION_RSP);
            break;

        case PI_CLEAR_GEO_LOCATION_CMD:
            timeout = MAX(timeout, TMO_NONE);
            mrpCmd = MRCLEARGLINFO;
            rspDataSz = sizeof(PI_CLEAR_GEO_LOCATION_RSP);
            break;

        case PI_DLM_PATH_STATS_CMD:
            timeout = MAX(timeout, TMO_NONE);
            mrpCmd = MRDLMPATHSTATS;
            rspDataSz = sizeof(PI_DLM_PATH_STATS_RSP);
            break;

        case PI_DLM_PATH_SELECTION_ALGO_CMD:
            timeout = MAX(timeout, TMO_NONE);
            mrpCmd = MRDLMPATHSELECTIONALGO;
            rspDataSz = sizeof(PI_DLM_PATH_SELECTION_ALGO_RSP);
            break;

        case PI_PDISK_SPINDOWN_CMD:
            timeout = MAX(timeout, TMO_NONE);
            mrpCmd = MRPDISKSPINDOWN;
            rspDataSz = sizeof(PI_PDISK_SPINDOWN_RSP);
            break;

        case PI_PDISK_FAILBACK_CMD:
            timeout = MAX(timeout, TMO_NONE);
            mrpCmd = MRPDISKFAILBACK;
            rspDataSz = sizeof(PI_PDISK_FAILBACK_RSP);
            break;

        case PI_PDISK_AUTO_FAILBACK_ENABLE_DISABLE_CMD:
            PI_CommandHandler = PI_PDiskAutoFailback;
            break;

        case PI_PDISKS_CMD:
            PI_CommandHandler = PI_PDisks;
            break;

        case PI_PDISKS_FROM_CACHE_CMD:
            PI_CommandHandler = PI_PDisksCache;
            break;

        case PI_SERVER_INFO_CMD:
            PI_CommandHandler = PI_ServerInfo;
            break;

        case PI_SERVER_CREATE_CMD:
            timeout = MAX(timeout, TMO_NONE);
            mrpCmd = MRCREATESERVER;
            rspDataSz = sizeof(PI_SERVER_CREATE_RSP);
            break;

        case PI_SERVER_DELETE_CMD:
            timeout = MAX(timeout, TMO_NONE);
            mrpCmd = MRDELETESERVER;
            rspDataSz = sizeof(PI_SERVER_DELETE_RSP);
            break;

        case PI_SERVER_ASSOCIATE_CMD:
            timeout = MAX(timeout, TMO_NONE);
            mrpCmd = MRMAPLUN;
            rspDataSz = sizeof(PI_SERVER_ASSOCIATE_RSP);
            break;

        case PI_SERVER_DISASSOCIATE_CMD:
            timeout = MAX(timeout, TMO_NONE);
            mrpCmd = MRUNMAPLUN;
            rspDataSz = sizeof(PI_SERVER_DISASSOCIATE_RSP);
            break;

        case PI_SERVER_SET_PROPERTIES_CMD:
            timeout = MAX(timeout, TMO_NONE);
            mrpCmd = MRSERVERPROP;
            rspDataSz = sizeof(PI_SERVER_SET_PROPERTIES_RSP);
            break;

        case PI_SERVER_LOOKUP_CMD:
            mrpCmd = MRSERVERLOOKUP;
            rspDataSz = sizeof(PI_SERVER_LOOKUP_RSP);
            break;

        case PI_SERVER_WWN_TO_TARGET_MAP_CMD:
            PI_CommandHandler = PI_ServerWwnToTargetMap;
            break;

        case PI_SERVERS_CMD:
            PI_CommandHandler = PI_Servers;
            break;

        case PI_VLINK_REMOTE_CTRL_COUNT_CMD:
            mrpCmd = MRREMOTECTRLCNT;
            rspDataSz = sizeof(PI_VLINK_REMOTE_CNT_RSP);
            break;

        case PI_VLINK_REMOTE_CTRL_INFO_CMD:
            mrpCmd = MRREMOTECTRLINFO;
            rspDataSz = sizeof(PI_VLINK_REMOTE_INFO_RSP);
            break;

        case PI_VLINK_REMOTE_CTRL_VDISKS_CMD:
            mrpCmd = MRREMOTEVDISKINFO;
            rspDataSz = sizeof(PI_VLINK_REMOTE_VDISKS_RSP);
            break;

        case PI_VLINK_CREATE_CMD:
            timeout = MAX(timeout, TMO_NONE);
            mrpCmd = MRCREATEVLINK;
            rspDataSz = sizeof(PI_VLINK_CREATE_RSP);
            break;

        case PI_VLINK_INFO_CMD:
            mrpCmd = MRVLINKINFO;
            rspDataSz = sizeof(PI_VLINK_INFO_RSP);
            break;

        case PI_VLINK_BREAK_LOCK_CMD:
            timeout = MAX(timeout, TMO_NONE);
            mrpCmd = MRBREAKVLOCK;
            rspDataSz = sizeof(PI_VLINK_BREAK_LOCK_RSP);
            break;

        case PI_VLINK_DLINK_INFO_CMD:
            mrpCmd = MRGETDLINK;
            rspDataSz = sizeof(PI_VLINK_DLINK_INFO_RSP);
            break;

        case PI_VLINK_DLINK_GT2TB_INFO_CMD:
            mrpCmd = MRGETDLINK_GT2TB;
            rspDataSz = sizeof(PI_VLINK_DLINK_GT2TB_INFO_RSP);
            break;

        case PI_VLINK_DLOCK_INFO_CMD:
            mrpCmd = MRGETDLOCK;
            rspDataSz = sizeof(PI_VLINK_DLOCK_INFO_RSP);
            break;

        case PI_VLINK_NAME_CHANGED_CMD:
            mrpCmd = MRNAMECHANGE;
            rspDataSz = sizeof(PI_VLINK_NAME_CHANGED_RSP);
            break;

        case PI_TARGET_INFO_CMD:
            mrpCmd = MRGETTARG;
            rspDataSz = sizeof(PI_TARGET_INFO_RSP);
            break;

        case PI_TARGET_SET_PROPERTIES_CMD:
            timeout = MAX(timeout, TMO_NONE);
            mrpCmd = MRCONFIGTARG;
            rspDataSz = sizeof(PI_TARGET_SET_PROPERTIES_RSP);
            break;

        case PI_ISCSI_SET_TGTPARAM_CMD:
            timeout = MAX(timeout, TMO_PI_PROC_RESTORE_NVRAM_CMD);
            mrpCmd = MRSETTGINFO;
            rspDataSz = sizeof(PI_SETTGINFO_RSP);
            break;

        case PI_ISCSI_TGT_INFO_CMD:
            timeout = MAX(timeout, TMO_PI_PROC_RESTORE_NVRAM_CMD);
            mrpCmd = MRGETTGINFO;
            if (((MRGETTGINFO_REQ *)(pReqPacket->pPacket))->tid == 0xFF)
            {
                rspDataSz = sizeof(PI_GETTGINFO_RSP) + MAX_TARGETS * sizeof(MRITGINFO);
            }
            else
            {
                rspDataSz = sizeof(PI_GETTGINFO_RSP) + sizeof(MRITGINFO);
            }
            break;

        case PI_ISCSI_SET_CHAP_CMD:
            timeout = MAX(timeout, TMO_PI_PROC_RESTORE_NVRAM_CMD);
            mrpCmd = MRSETCHAP;
            rspDataSz = sizeof(PI_SETCHAP_RSP);
            break;

        case PI_ISCSI_CHAP_INFO:
            timeout = MAX(timeout, TMO_PI_PROC_RESTORE_NVRAM_CMD);
            mrpCmd = MRGETCHAP;
            rspDataSz = sizeof(PI_GETCHAP_RSP) + (1024 * sizeof(MRCHAPINFO));
            break;

        case PI_ISCSI_SESSION_INFO:
            timeout = MAX(timeout, TMO_PI_PROC_RESTORE_NVRAM_CMD);
            mrpCmd = MRGETSESSIONS;
            rspDataSz = sizeof(PI_GETSESSIONS_RSP) + (1024 * sizeof(MRSESSION));
            break;

        case PI_ISCSI_SESSION_INFO_SERVER:
            timeout = MAX(timeout, TMO_PI_PROC_RESTORE_NVRAM_CMD);
            mrpCmd = MRGETSESSIONSPERSERVER;
            rspDataSz = sizeof(PI_GETSESSIONS_RSP) + (32 * sizeof(MRSESSION));
            break;
        case PI_GETISNSINFO_CMD:
            timeout = MAX(timeout, TMO_PI_PROC_RESTORE_NVRAM_CMD);
            mrpCmd = MRGETISNSINFO;
            rspDataSz = sizeof(PI_GETISNSINFO_RSP) + (MAX_ISNS_SERVERS * sizeof(MRISNS_SERVER_INFO));
            break;
        case PI_SETISNSINFO_CMD:
            timeout = MAX(timeout, TMO_PI_PROC_RESTORE_NVRAM_CMD);
            mrpCmd = MRSETISNSINFO;
            rspDataSz = sizeof(PI_SETISNSINFO_RSP);
            break;
        case PI_IDD_INFO_CMD:
            timeout = MAX(timeout, TMO_PI_PROC_RESTORE_NVRAM_CMD);
            mrpCmd = MRGETIDDINFO;
            rspDataSz = sizeof(MRGETIDDINFO_RSP) + (16 * sizeof(MRIDD));
            break;

        case PI_TARGET_RESOURCE_LIST_CMD:
            PI_CommandHandler = PI_TargetResourceList;
            break;

        case PI_TARGETS_CMD:
            PI_CommandHandler = PI_Targets;
            break;

        case PI_VDISK_INFO_CMD:
            PI_CommandHandler = PI_VDiskInfo;
            break;

        case PI_VDISK_CREATE_CMD:
        case PI_VDISK_EXPAND_CMD:
        case PI_VDISK_PREPARE_CMD:
            timeout = MAX(timeout, TMO_NONE);
            mrpCmd = MRCREXP;
            rspDataSz = sizeof(PI_VDISK_CREATE_RSP);
            break;

        case PI_VDISK_DELETE_CMD:
            timeout = MAX(timeout, TMO_NONE);
            mrpCmd = MRDELVIRT;
            rspDataSz = sizeof(PI_VDISK_DELETE_RSP);
            break;

        case PI_VDISK_BAY_REDUNDANT_CMD:
            timeout = MAX(timeout, TMO_NONE);
            mrpCmd = MRVIRTREDUNDANCY;
            rspDataSz = sizeof(MRVIRTREDUNDANCY_RSP);
            break;

        case PI_VDISK_PR_GET_CMD:
            timeout = MAX(timeout, MRP_UPDATE_DRIVE_BAY_TIMEOUT);
            mrpCmd = MRPRGET;
            rspDataSz = sizeof(PI_PR_GET_RSP);
//            printf("CCB got PR get command\n");
            break;

        case PI_VDISK_PR_CLR_CMD:
            timeout = MAX(timeout, TMO_NONE);
            mrpCmd = MRPRCLR;
            rspDataSz = sizeof(PI_PR_CLR_RSP);
//            printf("CCB got PR clear command\n");
            break;

        case PI_VDISK_SET_PRIORITY_CMD:
            mrpCmd = MRSETVPRI;
            rspDataSz = sizeof(MRSETVPRI_RSP) +
                sizeof(MRSETVPRI_VIDPRI) *
                (((MRSETVPRI_REQ *)(pReqPacket->pPacket))->respcount);
            break;

        case PI_VCG_VDISK_PRIORITY_ENABLE_CMD:
            mrpCmd = MRVPRI_ENABLE;
            rspDataSz = sizeof(MRVPRI_ENABLE_RSP);
            break;

        case PI_VDISK_CONTROL_CMD:
            timeout = MAX(timeout, TMO_NONE);
            mrpCmd = MRVDISKCONTROL;
            rspDataSz = sizeof(PI_VDISK_CONTROL_RSP);
            break;

        case PI_QUICK_BREAK_PAUSE_RESUME_MIRROR_START_CMD:
            PI_CommandHandler = PI_QuickBreakPauseResumeMirrorStart;
            break;

        case PI_QUICK_BREAK_PAUSE_RESUME_MIRROR_SEQUENCE_CMD:
            PI_CommandHandler = PI_QuickBreakPauseResumeMirrorSequence;
            break;

        case PI_QUICK_BREAK_PAUSE_RESUME_MIRROR_EXECUTE_CMD:
            PI_CommandHandler = PI_QuickBreakPauseResumeMirrorExecute;
            break;

        case PI_BATCH_SNAPSHOT_START_CMD:
            PI_CommandHandler = PI_BatchSnapshotStart;
            break;

        case PI_BATCH_SNAPSHOT_SEQUENCE_CMD:
            PI_CommandHandler = PI_BatchSnapshotSequence;
            break;

        case PI_BATCH_SNAPSHOT_EXECUTE_CMD:
            PI_CommandHandler = PI_BatchSnapshotExecute;
            break;

        case PI_VDISK_SET_ATTRIBUTE_CMD:
            PI_CommandHandler = PI_VDiskSetAttributes;
            break;

        case PI_VDISK_OWNER_CMD:
            PI_CommandHandler = PI_VDiskOwner;
            break;

        case PI_VDISKS_CMD:
            PI_CommandHandler = PI_VDisks;
            break;

        case PI_VDISKS_FROM_CACHE_CMD:
            PI_CommandHandler = PI_VDisksCache;
            break;

        case PI_RAID_INFO_CMD:
            PI_CommandHandler = PI_RAIDInfo;
            break;

        case PI_RAID_INIT_CMD:
            mrpCmd = MRINITRAID;
            rspDataSz = sizeof(PI_RAID_INIT_RSP);
            break;

        case PI_RAID_CONTROL_CMD:
            {
                PI_RAID_CONTROL_REQ *pReq = (PI_RAID_CONTROL_REQ *)pReqPacket->pPacket;

                if ((pReq->scrubcontrol & SCRUB_CHANGE) > 0 ||
                    (pReq->paritycontrol & PARITY_SCAN_ENABLE) > 0 ||
                    (pReq->paritycontrol & PARITY_SCAN_DISABLE) > 0)
                {
                    timeout = MAX(timeout, TMO_NONE);
                }
                mrpCmd = MRSCRUBCTRL;
                rspDataSz = sizeof(PI_RAID_CONTROL_RSP);
            }
            break;

        case PI_RAID_RECOVER_CMD:
            mrpCmd = MRRAIDRECOVER;
            rspDataSz = sizeof(PI_RAID_RECOVER_RSP);
            break;

        case PI_RAIDS_CMD:
            PI_CommandHandler = PI_Raids;
            break;

        case PI_RAIDS_FROM_CACHE_CMD:
            PI_CommandHandler = PI_RaidsCache;
            break;

        case PI_ADMIN_FW_VERSIONS_CMD:
            PI_CommandHandler = PI_FirmwareVersionInfo;
            break;

        case PI_ADMIN_FW_SYS_REL_LEVEL_CMD:
            PI_CommandHandler = PI_FirmwareSystemReleaseLevel;
            break;

        case PI_ADMIN_SETTIME_CMD:
            PI_CommandHandler = PI_SetTime;
            break;

        case PI_ADMIN_GETTIME_CMD:
            PI_CommandHandler = PI_GetTime;
            break;

        case PI_ADMIN_LEDCNTL_CMD:
            PI_CommandHandler = PI_LedControl;
            break;

        case PI_ADMIN_SET_IP_CMD:
            PI_CommandHandler = PI_SetIpAddress;
            break;

        case PI_ADMIN_GET_IP_CMD:
            PI_CommandHandler = PI_GetIpAddress;
            break;

        case PI_FIRMWARE_DOWNLOAD_CMD:
        case PI_WRITE_BUFFER_MODE5_CMD:
        case PI_TRY_CCB_FW_CMD:
            PI_CommandHandler = PI_FWDownload;
            break;

        case PI_MULTI_PART_XFER_CMD:
            PI_CommandHandler = PI_MultiPartXfer;
            break;

        case PI_LOG_INFO_CMD:
            PI_CommandHandler = PI_LogInfoRequest;
            break;

        case PI_LOG_CLEAR_CMD:
            PI_CommandHandler = PI_LogClearRequest;
            break;

        case PI_LOG_TEXT_MESSAGE_CMD:
            PI_CommandHandler = PI_LogTextMessage;
            break;

        case PI_DEBUG_MEM_RDWR_CMD:
            PI_CommandHandler = PI_DebugMemRdWr;
            break;

        case PI_DEBUG_INIT_PROC_NVRAM_CMD:
            timeout = MAX(timeout, TMO_NONE);
            mrpCmd = MRRESET;
            rspDataSz = sizeof(PI_DEBUG_INIT_PROC_NVRAM_RSP);
            break;

        case PI_DEBUG_INIT_CCB_NVRAM_CMD:
            PI_CommandHandler = PI_DebugInitCCBNVRAM;
            break;

        case PI_DEBUG_GET_SER_NUM_CMD:
            PI_CommandHandler = PI_DebugGetSerNum;
            break;

        case PI_MISC_GET_MODE_CMD:
            PI_CommandHandler = PI_MiscGetMode;
            break;

        case PI_MISC_SET_MODE_CMD:
            PI_CommandHandler = PI_MiscSetMode;
            break;

        case PI_DEBUG_MRMMTEST_CMD:
            mrpCmd = MRMMTEST;
            rspDataSz = sizeof(PI_DEBUG_MRMMTEST_RSP);
            break;

        case PI_DEBUG_STRUCT_DISPLAY_CMD:
            PI_CommandHandler = PI_DebugStructDisplay;
            break;

        case PI_DEBUG_GET_ELECTION_STATE_CMD:
            PI_CommandHandler = PI_DebugGetElecSt;
            break;

        case PI_DEBUG_SCSI_COMMAND_CMD:
            PI_CommandHandler = PI_SCSICmd;
            break;

        case PI_DEBUG_READWRITE_CMD:
            PI_CommandHandler = PI_READWRITECmd;
            break;

        case PI_DEBUG_BE_LOOP_PRIMITIVE_CMD:
            mrpCmd = MRBELOOPPRIMITIVE;
            rspDataSz = sizeof(PI_LOOP_PRIMITIVE_RSP);
            break;

        case PI_DEBUG_FE_LOOP_PRIMITIVE_CMD:
            mrpCmd = MRFELOOPPRIMITIVE;
            rspDataSz = sizeof(PI_LOOP_PRIMITIVE_RSP);
            break;

        case PI_DEBUG_GET_STATE_RM_CMD:
            PI_CommandHandler = PI_DebugGetState_RM;
            break;

        case PI_PROC_RESTORE_NVRAM_CMD:
            timeout = MAX(timeout, TMO_NONE);

            if (BIT_TEST(((PI_RESTORE_PROC_NVRAM_REQ *)(pReqPacket->pPacket))->opt, MRNOBREFRESH))
            {
                mrpCmd = MRREFRESH;
            }
            else
            {
                mrpCmd = MRRESTORE;
            }

            rspDataSz = sizeof(PI_RESTORE_PROC_NVRAM_RSP);
            break;

        case PI_PROC_BE_DEVICE_PATH_CMD:
            PI_CommandHandler = PI_ProcBeDevicePathList;
            break;

        case PI_PROC_FAIL_CTRL_CMD:
            timeout = MAX(timeout, TMO_NONE);
            mrpCmd = MRFAILCTRL;
            rspDataSz = sizeof(PI_PROC_FAIL_CTRL_RSP);
            break;

        case PI_PROC_FAIL_PORT_CMD:
            mrpCmd = MRFAILPORT;
            rspDataSz = sizeof(PI_PROC_FAIL_PORT_RSP);
            break;

        case PI_PROC_RESET_FE_QLOGIC_CMD:
            mrpCmd = MRRESETFEPORT;
            rspDataSz = sizeof(PI_RESET_PROC_QLOGIC_RSP);
            break;

        case PI_PROC_RESET_BE_QLOGIC_CMD:
            timeout = MAX(timeout, TMO_PI_PROC_RESETQLOGIC_CMD);
            mrpCmd = MRRESETBEPORT;
            rspDataSz = sizeof(PI_RESET_PROC_QLOGIC_RSP);
            break;

        case PI_PROC_TARGET_CONTROL_CMD:
            timeout = MAX(timeout, TMO_PI_PROC_TARGET_CONTROL_CMD);
            mrpCmd = MRTARGETCONTROL;
            rspDataSz = sizeof(PI_PROC_TARGET_CONTROL_RSP);
            break;

        case PI_PROC_START_IO_CMD:
            mrpCmd = MRSTARTIO;
            rspDataSz = sizeof(PI_PROC_START_IO_RSP);
            break;

        case PI_PROC_STOP_IO_CMD:
            mrpCmd = MRSTOPIO;
            rspDataSz = sizeof(PI_PROC_STOP_IO_RSP);
            break;

        case PI_PROC_ASSIGN_MIRROR_PARTNER_CMD:
            PI_CommandHandler = PI_MiscAssignMirrorPartner;
            break;

        case PI_PROC_NAME_DEVICE_CMD:
            {
                PI_PROC_NAME_DEVICE_REQ *pReq = (PI_PROC_NAME_DEVICE_REQ *)pReqPacket->pPacket;

                /*
                 * For the name device command we only want to cause a
                 * configuration propogation if the option is not a
                 * "retrieve controller".
                 */
                if (pReq->option == MNDSERVER ||
                    pReq->option == MNDVDISK || pReq->option == MNDVCG)
                {
                    timeout = MAX(timeout, TMO_NONE);
                }
                mrpCmd = MRNAMEDEVICE;
                rspDataSz = sizeof(PI_PROC_NAME_DEVICE_RSP);
            }
            break;

        case PI_VCG_VALIDATION_CMD:
            PI_CommandHandler = PI_VCGValidation;
            break;

        case PI_GET_CPUCOUNT_CMD:
            PI_CommandHandler = PI_GetCpuCount;
            break;

        case PI_VCG_PREPARE_SLAVE_CMD:
            PI_CommandHandler = PI_VCGPrepareSlave;
            break;

        case PI_VCG_ADD_SLAVE_CMD:
            PI_CommandHandler = PI_VCGAddSlave;
            break;

        case PI_VCG_PING_CMD:
            PI_CommandHandler = PI_VCGPing;
            break;

        case PI_VCG_INFO_CMD:
            PI_CommandHandler = PI_VCGInfo;
            break;

        case PI_VCG_INACTIVATE_CONTROLLER_CMD:
            PI_CommandHandler = PI_VCGInactivateController;
            break;

        case PI_VCG_SET_CACHE_CMD:
            timeout = MAX(timeout, TMO_NONE);
            mrpCmd = MRSETCACHE;
            rspDataSz = sizeof(PI_VCG_SET_CACHE_RSP);
            break;

        case PI_VCG_GET_MP_LIST_CMD:
            mrpCmd = MRGETMPLIST;

            /*
             * Calculate the response data size, the size of the
             * mirror partner list out structure plus 1 entry
             * for each of the controllers in the group (this will
             * result in the maximum number of entries the group
             * could have if all controllers were configured).
             */
            rspDataSz = sizeof(MRGETMPLIST_RSP) +
                (sizeof(MRGETMPLIST_RSP_INFO) * Qm_GetNumControllersAllowed());
            break;

        case PI_VCG_APPLY_LICENSE_CMD:
            PI_CommandHandler = PI_VCGApplyLicense;
            break;

        case PI_VCG_ACTIVATE_CONTROLLER_CMD:
        case PI_VCG_UNFAIL_CONTROLLER_CMD:
            PI_CommandHandler = PI_VCGUnfailController;
            break;

        case PI_VCG_FAIL_CONTROLLER_CMD:
            PI_CommandHandler = PI_VCGFailController;
            break;

        case PI_VCG_REMOVE_CONTROLLER_CMD:
            PI_CommandHandler = PI_VCGRemoveController;
            break;

        case PI_VCG_SHUTDOWN_CMD:
            PI_CommandHandler = PI_VCGShutdown;
            break;

        case PI_STATS_ENVIRONMENTAL_CMD:
            PI_CommandHandler = PI_EnvStatsRequest;
            break;

#ifdef ENABLE_NG_HWMON
        case PI_ENV_II_GET_DATA_CMD:
            PI_CommandHandler = PI_EnvIIRequest;
            break;
#endif /* ENABLE_NG_HWMON */

#if defined(MODEL_7000) || defined(MODEL_4700)
        case PI_ISE_GET_STATUS_CMD:
            PI_CommandHandler = PI_ISEStatus;
            break;
        case PI_BEACON_ISE_COMPONENT:
            PI_CommandHandler = PI_BeaconIseComponent;
            break;
#endif /* MODEL_7000 || MODEL_4700 */

        case PI_STATS_GLOBAL_CACHE_CMD:
            mrpCmd = MRGETCINFO;
            rspDataSz = sizeof(PI_STATS_GLOBAL_CACHE_RSP);
            break;

        case PI_STATS_CACHE_DEVICE_CMD:
            mrpCmd = MRGETCDINFO;
            rspDataSz = sizeof(PI_STATS_CACHE_DEV_RSP);
            break;

        case PI_STATS_FRONT_END_PROC_CMD:
        case PI_STATS_BACK_END_PROC_CMD:
        case PI_STATS_PROC_CMD:
            PI_CommandHandler = PI_StatsProc;
            break;

        case PI_STATS_FRONT_END_LOOP_CMD:
        case PI_STATS_BACK_END_LOOP_CMD:
            PI_CommandHandler = PI_StatsLoop;
            break;

        case PI_STATS_FRONT_END_PCI_CMD:
            mrpCmd = MRFELINK;
            rspDataSz = sizeof(PI_STATS_FRONT_END_PCI_RSP);
            break;

        case PI_STATS_BACK_END_PCI_CMD:
            mrpCmd = MRBELINK;
            rspDataSz = sizeof(PI_STATS_BACK_END_PCI_RSP);
            break;

        case PI_STATS_SERVER_CMD:
            mrpCmd = MRGETSSTATS;
            rspDataSz = sizeof(PI_STATS_SERVER_RSP);
            break;

        case PI_STATS_VDISK_CMD:
            PI_CommandHandler = PI_StatsVDisk;
            break;

        case PI_STATS_PCI_CMD:
            PI_CommandHandler = PI_StatsPCI;
            break;

        case PI_STATS_CACHE_DEVICES_CMD:
            PI_CommandHandler = PI_StatsCacheDevices;
            break;

        case PI_STATS_HAB_CMD:
            mrpCmd = MRGETHABSTATS;
            rspDataSz = sizeof(PI_STATS_HAB_RSP);
            break;

        case PI_STATS_SERVERS_CMD:
            PI_CommandHandler = PI_StatsServers;
            break;

        case PI_ENVIRO_DATA_DISK_BAY_CMD:
            PI_CommandHandler = PI_DiskBaySESEnviro;
            break;

        case PI_ENVIRO_DATA_CTRL_AND_BAY_CMD:
            PI_CommandHandler = PI_CtrlAndBayEnviro;
            break;

        case PI_VCG_CONFIGURE_CMD:
            PI_CommandHandler = PI_ConfigCtrl;
            break;

        case PI_GENERIC_CMD:
            PI_CommandHandler = PI_GenericCommand;
            break;

        case PI_GENERIC2_CMD:
            PI_CommandHandler = PI_Generic2Command;
            break;

        case PI_GENERIC_MRP_CMD:
            PI_CommandHandler = PI_GenericMRP;
            break;

        case PI_WCACHE_INVALIDATE_CMD:
            PI_CommandHandler = PI_WCacheInvalidate;
            break;

        case PI_MISC_GET_DEVICE_COUNT_CMD:
            mrpCmd = MRDEVICECOUNT;
            rspDataSz = sizeof(PI_MISC_GET_DEVICE_COUNT_RSP);
            break;

        case PI_MISC_RESCAN_DEVICE_CMD:
            timeout = MAX(timeout, TMO_PI_MISC_RESCAN_DEVICE_CMD);
            mrpCmd = MRRESCANDEVICE;
            rspDataSz = sizeof(PI_MISC_RESCAN_DEVICE_RSP);
            break;

        case PI_MISC_GET_BE_DEVICE_LIST_CMD:
            mrpCmd = MRBEGETDVLIST;
            rspDataSz = sizeof(PI_MISC_DEVICE_LIST_RSP);
            break;

        case PI_MISC_GET_FE_DEVICE_LIST_CMD:
            mrpCmd = MRFEGETDVLIST;
            rspDataSz = sizeof(PI_MISC_DEVICE_LIST_RSP);
            break;

        case PI_MISC_FILE_SYSTEM_READ_CMD:
            PI_CommandHandler = PI_FileSystemRead;
            break;

        case PI_MISC_FILE_SYSTEM_WRITE_CMD:
            PI_CommandHandler = PI_FileSystemWrite;
            break;

        case PI_MISC_FAILURE_STATE_SET_CMD:
            PI_CommandHandler = PI_FailureStateSet;
            break;

        case PI_MISC_UNFAIL_INTERFACE_CMD:
            PI_CommandHandler = PI_UnfailInterface;
            break;

        case PI_MISC_FAIL_INTERFACE_CMD:
            PI_CommandHandler = PI_FailInterface;
            break;

        case PI_MISC_SERIAL_NUMBER_SET_CMD:
            PI_CommandHandler = PI_MiscSerialNumberSet;
            break;

        case PI_MISC_RESYNC_MIRROR_RECORDS_CMD:
            PI_CommandHandler = PI_MiscResyncMirrorRecords;
            break;

        case PI_MISC_CONTINUE_WO_MP_CMD:
            mrpCmd = MRFECONTWOMP;
            rspDataSz = sizeof(PI_MISC_CONTINUE_WO_MP_RSP);
            break;

        case PI_MISC_INVALIDATE_BE_WC_CMD:
            mrpCmd = MRINVBEWC;
            rspDataSz = sizeof(PI_MISC_INVALIDATE_BE_WC_RSP);
            break;

        case PI_MISC_MIRROR_PARTNER_CONTROL_CMD:
            PI_CommandHandler = PI_MiscMirrorPartnerControl;
            break;

        case PI_MISC_MIRROR_PARTNER_GET_CFG_CMD:
            PI_CommandHandler = PI_MiscMirrorPartnerGetCfg;
            break;

        case PI_ROLLING_UPDATE_PHASE_CMD:
            PI_CommandHandler = PI_RollingUpdatePhase;
            break;

        case PI_SNAPSHOT_TAKE_CMD:
            PI_CommandHandler = PI_TakeSnapshot;
            break;

        case PI_SNAPSHOT_LOAD_CMD:
            PI_CommandHandler = PI_LoadSnapshot;
            break;

        case PI_SNAPSHOT_CHANGE_CMD:
            PI_CommandHandler = PI_ChangeSnapshot;
            break;

        case PI_SNAPSHOT_READDIR_CMD:
            PI_CommandHandler = PI_ReadSnapshotDirectory;
            break;

        case PI_PERSISTENT_DATA_CONTROL_CMD:
            PI_CommandHandler = PI_PersistentDataControl;
            break;

        case PI_CLIENT_PERSISTENT_DATA_CONTROL_CMD:
            PI_CommandHandler = PI_ClientPersistentDataControl;
            break;

        case PI_CUSTOMER_LOG_ACKNOWLEDGE_CMD:
            PI_CommandHandler = PI_LogAcknowledge;
            break;

        case PI_MISC_GET_WORKSET_INFO_CMD:
            PI_CommandHandler = PI_GetWorksetInfo;
            break;

        case PI_MISC_SET_WORKSET_INFO_CMD:
            timeout = MAX(timeout, TMO_NONE);
            mrpCmd = MRSETWSINFO;
            rspDataSz = sizeof(PI_MISC_SET_WORKSET_INFO_RSP);
            break;

        case PI_CACHE_REFRESH_CCB_CMD:
            PI_CommandHandler = PI_CacheRefreshCCB;
            break;

        case PI_SET_DLM_HEARTBEAT_LIST_CMD:
            PI_CommandHandler = PI_DLMHeartbeatList;
            break;

        case PI_CACHE_FLUSH_BE_CMD:
            mrpCmd = MRFLUSHBEWC;
            rspDataSz = sizeof(PI_CACHE_FLUSH_BE_RSP);
            break;

        case PI_MFG_CTRL_CLEAN_CMD:
            PI_CommandHandler = PI_MfgCtrlClean;
            break;

        case PI_RAID_MIRRORING_CMD:
            timeout = MAX(timeout, TMO_NONE);
            mrpCmd = MRCHGRAIDNOTMIRRORING;
            rspDataSz = sizeof(PI_RAID_MIRRORING_RSP);
            break;

        case PI_MISC_RESYNC_RAIDS_CMD:
            PI_CommandHandler = PI_MiscResyncRaids;
            break;

        case PI_MISC_PUTDEVCONFIG_CMD:
            PI_CommandHandler = PI_MiscPutDevConfig;
            break;

        case PI_MISC_GETDEVCONFIG_CMD:
            PI_CommandHandler = PI_MiscGetDevConfig;
            break;

        case PI_STATS_BUFFER_BOARD_CMD:
            mrpCmd = MRMMCARDGETBATTERYSTATUS;
            rspDataSz = sizeof(PI_STATS_BUFFER_BOARD_RSP);
            break;

        case PI_BATTERY_HEALTH_SET_CMD:
            PI_CommandHandler = PI_BatteryHealthSet;
            break;

        case PI_MISC_INVALIDATE_FE_WC_CMD:
            mrpCmd = MRINVFEWC;
            rspDataSz = sizeof(PI_MISC_INVALIDATE_FE_WC_RSP);
            break;

        case PI_MISC_QUERY_MP_CHANGE_CMD:
            mrpCmd = MRQMPC;
            rspDataSz = sizeof(PI_MISC_QUERY_MP_CHANGE_RSP);
            break;

        case PI_MISC_RESYNCDATA_CMD:
            PI_CommandHandler = PI_MiscResyncData;
            break;

        case PI_MISC_RESYNCCTL_CMD:
            mrpCmd = MRRESYNCCTL;
            rspDataSz = sizeof(PI_MISC_RESYNCCTL_RSP);
            break;

        case PI_MISC_SETTDISCACHE_CMD:
            timeout = MAX(timeout, TMO_PI_MISC_SETTDISCACHE_CMD);
            mrpCmd = MRSETTDISCACHE;
            rspDataSz = sizeof(PI_MISC_SETTDISCACHE_RSP);
            break;

        case PI_MISC_CLRTDISCACHE_CMD:
            timeout = MAX(timeout, TMO_PI_MISC_CLRTDISCACHE_CMD);
            mrpCmd = MRCLRTDISCACHE;
            rspDataSz = sizeof(PI_MISC_CLRTDISCACHE_RSP);
            break;

        case PI_MISC_QTDISCACHE_CMD:
            mrpCmd = MRQTDISABLEDONE;
            rspDataSz = sizeof(PI_MISC_QTDISCACHE_RSP);
            break;

        case PI_MISC_CFGOPTION_CMD:
            mrpCmd = MRCFGOPTION;
            rspDataSz = sizeof(PI_MISC_CFGOPTION_RSP);
            break;

#ifndef DISABLE_LOCAL_RAID_MONITORING
        case PI_MISC_LOCAL_RAID_INFO_CMD:
            PI_CommandHandler = PI_MiscGetLocalRaidInfo;
            break;
#endif  /* DISABLE_LOCAL_RAID_MONITORING */

        case PI_SET_PR_CMD:
            mrpCmd = MRSETPR;
            rspDataSz = sizeof(MRSETPRES_RSP);
            break;

        case PI_REGISTER_CLIENT_TYPE_CMD:
            PI_CommandHandler = PI_RegisterClientType;
            break;

            /*
             * Default of the command code handler...if the command code
             * is one of the XIOtech Storage Platform codes then use
             * the PI_X1Packet hander.  If not it is an invalid command
             * code and cannot be accepted.
             */
        default:
            pRspPacket->pHeader->status = PI_INVALID_CMD_CODE;
            /* Invalid command code found, return error. */
            return PI_INVALID_CMD_CODE;
    }

    /*
     * If function pointer is valid, call function to handle the command.
     * MRP calls are handled using a common pass-thru function.
     */
    if (PI_CommandHandler != NULL)
    {
        rc = PI_CommandHandler(pReqPacket, pRspPacket);
#ifdef FOLLOW_PI_EXECUTION
if (pReqPacket->pHeader->commandCode != 0x11e && pReqPacket->pHeader->commandCode != 0x1ff && pReqPacket->pHeader->commandCode != 0x5ff) {
  dprintf(DPRINTF_DEFAULT, "%s:%u:%s Completed PI commandCode=0x%x (%d) rc=%d\n", __FILE__, __LINE__, __func__, pReqPacket->pHeader->commandCode,pReqPacket->pHeader->commandCode, rc);
}
#endif  /* FOLLOW_PI_EXECUTION */
    }
    else
    {
        rc = PI_MRPPassThrough(pReqPacket, pRspPacket, mrpCmd, rspDataSz, timeout);
#ifdef FOLLOW_PI_EXECUTION
if (pReqPacket->pHeader->commandCode != 0x11e && pReqPacket->pHeader->commandCode != 0x1ff && pReqPacket->pHeader->commandCode != 0x5ff) {
  dprintf(DPRINTF_DEFAULT, "%s:%u:%s Completed PI->MRP commandCode=0x%x (%d) rc=%d\n", __FILE__, __LINE__, __func__, pReqPacket->pHeader->commandCode,pReqPacket->pHeader->commandCode, rc);
}
#endif  /* FOLLOW_PI_EXECUTION */
    }

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PacketCommandHandlerPostProcessImpl
**
** Description: Implements the Packet Command Handler post-processing function
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD, PI_ERROR or PI_TIMEOUT
**
**--------------------------------------------------------------------------*/
INT32 PacketCommandHandlerPostProcessImpl(UNUSED XIO_PACKET *pReqPacket,
                                          UNUSED XIO_PACKET *pRspPacket)
{
    if (pReqPacket->pHeader->commandCode == PI_PDISK_LABEL_CMD)
    {
        UINT32      count;

        count = CPSInitGetOwnedDriveCount(GetSerialNumber(SYSTEM_SN));

        if (ownedDrivesBeforeLabel == 0 && count > 0)
        {
            /*
             * We don't check a return code here since we don't know
             * what to do if this operation failed.  But if it did fail,
             * there will be an Error class log event created to alert
             * the sysop...
             */
            RefreshDirectory();

            /* Save the owned drive count in the master configuration. */
            Qm_SetOwnedDriveCount(count);

            /*
             * Save the master configuration and controller configuration
             * map to NVRAM and drives.
             */
            SaveMasterConfig();
            SaveControllerConfigMap();

            /* Make sure this controller is marked as operational. */
            WriteFailureDataState(GetMyControllerSN(), FD_STATE_OPERATIONAL);

            /* Initialize the Snapshot directory area */
            InitSnapshotFID();
        }
        else if (count > 0)
        {
            /* Save the owned drive count in the master configuration. */
            Qm_SetOwnedDriveCount(count);

            /*
             * Save the master configuration to NVRAM and drives since
             * we updated the number of drives owned by the group.
             */
            SaveMasterConfig();
        }
        else
        {
            /* Save the owned drive count in the master configuration. */
            Qm_SetOwnedDriveCount(count);

            /*
             * Save the master configuration to NVRAM since we updated
             * the number of drives owned by the group.  We don't save
             * to drives here since we don't own any.
             */
            StoreMasterConfigToNVRAM();

            /* Clear out the cached Snapshot directory area */
            InitSnapshotFID();
        }
    }
    else if (pReqPacket->pHeader->commandCode == PI_DEBUG_INIT_PROC_NVRAM_CMD)
    {
        if (((PI_DEBUG_INIT_PROC_NVRAM_REQ *)pReqPacket->pPacket)->type == MXNALL)
        {
            ResetMasterConfigNVRAM();
        }
    }
    else if (pReqPacket->pHeader->commandCode == PI_PROC_ASSIGN_MIRROR_PARTNER_CMD)
    {
        gCurrentMirrorPartnerSN = ((PI_PROC_ASSIGN_MIRROR_PARTNER_REQ *)pReqPacket->pPacket)->serialNumber;
    }
    else if (pReqPacket->pHeader->commandCode == PI_PDISK_UNFAIL_CMD)
    {
        InvalidateCachePhysicalDisks(FALSE);
    }
    return PI_GOOD;
}

/*----------------------------------------------------------------------------
** Function:    GetLabelOption
**
** Description: Determines the correct label option to use for a PDisk Label
**              request.
**
**              LABTYPE == mldnolabel ==> OPTION = 0
**                  Pass option zero since we do not need to do anything with
**                  the file system.
**
**              NO DRIVES OWNED ==> OPTION = mldfull | mldfsys
**                  We are labeling a drive and there are no other drives from
**                  which to get a file system so we need to do a full init and
**                  file system label.
**
**              NOTHING ELSE TO CHECK ==> OPTION = mldduplicate
**
** Inputs:      pRequest - Label request packet, this will be updated with the
**                         correct option if we can successfully determine it.
**
** Returns:     NONE
**
**--------------------------------------------------------------------------*/
static void GetLabelOption(PI_PDISK_LABEL_REQ *pRequest)
{
    /* If we are unlabeling a drive the the only option we have is "0". */
    if (pRequest->labtype == MLDNOLABEL)
    {
        pRequest->option = 0;
    }
    else
    {
        /*
         * If we do not own any drives then we need to do a full file system
         * initialization.
         */
        if (ownedDrivesBeforeLabel == 0)
        {
            pRequest->option = MLD_OPT_FULL | MLD_OPT_FSYS;
        }
        else
        {
            pRequest->option = MLD_OPT_DUPLICATE;
        }
    }
}

/*----------------------------------------------------------------------------
** Function:    ValidatePhysicalDiskUnlabel
**
** Description: Determine if the unlabel operation the user is requesting
**              is allowed.  The operation will not be allowed if the
**              group is configured with multiple controllers and the
**              operation requests to unlabel all existing labeled drives.
**
** Inputs:      pRequest - Label request packet.
**
** Returns:     PI_GOOD, PI_ERROR or PI_TIMEOUT
**
**--------------------------------------------------------------------------*/
static INT32 ValidatePhysicalDiskUnlabel(PI_PDISK_LABEL_REQ *pRequest)
{
    UINT16      index1;
    MRGETPINFO_RSP phyDevOut;
    UINT8       count = 0;

    /* Loop to find the number of controllers configured for the group. */
    for (index1 = 0; index1 < MAX_CONTROLLERS; index1++)
    {
        /* If serial number not zero the controller is considered configured. */
        if (cntlConfigMap.cntlConfigInfo[index1].controllerSN != 0)
        {
            count++;
        }
    }

    /*
     * If there is more than one controller configured and there are currently
     * two drives owned by the group we need to investigate the current class
     * of the PID being unlabeled. Essentially we are looking to see if the
     * unlabel operation is attempting to unlabel below two labeled drives.
     */
    if (count > 1 && ownedDrivesBeforeLabel <= 2)
    {
        /* Get the physical disk information from the cache manager. */
        PhysicalDiskGet(pRequest->pid, &phyDevOut);

        /*
         * If the class is not "unlabeled", signal an error because the
         * operation is attempting to unlabel the only labeled drive.
         */
        if (phyDevOut.pdd.devClass != PD_UNLAB &&
            phyDevOut.pdd.ssn == Qm_GetVirtualControllerSN())
        {
            return PI_ERROR;
        }
    }

    return PI_GOOD;
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
