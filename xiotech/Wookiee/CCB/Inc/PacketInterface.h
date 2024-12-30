/* $Id: PacketInterface.h 163153 2014-04-11 23:33:11Z marshall_midden $ */
/**
******************************************************************************
**
**  @file   PacketInterface.h
**
**  @brief  Packet Interface between clients and MRP layer
**
**  Definition of request and response packets for the client
**  to MRP layer interface.
**
** @attention       These definitions are used in SPAL, CCB, and indirectly
**                  ccbCL.pl. They must NEVER change in a manner that is not
**                  completely backwards compatible. See PI versioning in
**                  CCB routine ParmCheckPacketVersion() and packetVersion.
**
** !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
** NOTE: !!!!!! THIS MEANS SPAL COMPILATION, IWS, ETC.
** !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
**
**  Copyright (c) 2001-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _PACKETINTERFACE_H_
#define _PACKETINTERFACE_H_

#include "AsyncEventHandler.h"
#include "CacheSize.h"
#include "DEF_Workset.h"
#include "globalOptions.h"
#include "hw_mon.h"
#include "HWM.h"
#include "MR_Defs.h"
#include "mode.h"
#include "ses.h"
#include "XIO_Types.h"
#include "xk_raidmon.h"

#if defined(MODEL_7000) || defined(MODEL_4700)
#include "IseStatusResponse.h"
#endif /* MODEL_7000 || MODEL_4700 */

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/
#define XSP_BASE                            0x8000

/*
** Packet command codes
*/
#define PI_CONNECT_CMD                      0x0001
#define PI_DISCONNECT_CMD                   0x0002
#define PI_PING_CMD                         0x0003
#define PI_RESET_CMD                        0x0004
#define PI_POWER_UP_STATE_CMD               0x0005
#define PI_POWER_UP_RESPONSE_CMD            0x0006
#define PI_X1_COMPATIBILITY_INDEX_CMD       0x0007

#define PI_REGISTER_EVENTS_CMD              0x0008
#define PI_REGISTER_CLIENT_TYPE_CMD         0x0009

#define PI_PDISK_COUNT_CMD                  0x0010
#define PI_PDISK_LIST_CMD                   0x0011
#define PI_PDISK_INFO_CMD                   0x0012
#define PI_PDISK_LABEL_CMD                  0x0013
#define PI_PDISK_DEFRAG_CMD                 0x0014
#define PI_PDISK_FAIL_CMD                   0x0015
#define PI_PDISK_BEACON_CMD                 0x0016
#define PI_PDISK_UNFAIL_CMD                 0x0017
#define PI_PDISK_DELETE_CMD                 0x0018
#define PI_PDISK_BYPASS_CMD                 0x0019
#define PI_PDISK_DEFRAG_STATUS_CMD          0x001A
#define PI_PDISKS_QLOGIC_TIMEOUT_EMULATE    0x001B
#define PI_PDISKS_CMD                       0x001F

#define PI_VDISK_COUNT_CMD                  0x0020
#define PI_VDISK_LIST_CMD                   0x0021
#define PI_VDISK_INFO_CMD                   0x0022
#define PI_VDISK_CREATE_CMD                 0x0023
#define PI_VDISK_DELETE_CMD                 0x0024
#define PI_VDISK_EXPAND_CMD                 0x0025
#define PI_VDISK_CONTROL_CMD                0x0026
#define PI_VDISK_PREPARE_CMD                0x0027
#define PI_VDISK_SET_PRIORITY_CMD           0x0028
#define PI_VDISK_OWNER_CMD                  0x0029
#define PI_VDISK_SET_ATTRIBUTE_CMD          0x002A
#define PI_VDISK_PR_GET_CMD                 0x002B
#define PI_VDISK_PR_CLR_CMD                 0x002C
#define PI_VDISKS_CMD                       0x002F

#define PI_SERVER_COUNT_CMD                 0x0030
#define PI_SERVER_LIST_CMD                  0x0031
#define PI_SERVER_INFO_CMD                  0x0032
#define PI_SERVER_CREATE_CMD                0x0033
#define PI_SERVER_DELETE_CMD                0x0034
#define PI_SERVER_ASSOCIATE_CMD             0x0035
#define PI_SERVER_DISASSOCIATE_CMD          0x0036
#define PI_SERVER_SET_PROPERTIES_CMD        0x0037
#define PI_SERVER_LOOKUP_CMD                0x0038
#define PI_SERVER_WWN_TO_TARGET_MAP_CMD     0x0039
#define PI_SERVERS_CMD                      0x003F

#define PI_VLINK_REMOTE_CTRL_COUNT_CMD      0x0040
#define PI_VLINK_REMOTE_CTRL_INFO_CMD       0x0041
#define PI_VLINK_REMOTE_CTRL_VDISKS_CMD     0x0042
#define PI_VLINK_CREATE_CMD                 0x0043
#define PI_VLINK_INFO_CMD                   0x0044
#define PI_VLINK_BREAK_LOCK_CMD             0x0045
#define PI_VLINK_NAME_CHANGED_CMD           0x0046
#define PI_VLINK_DLINK_INFO_CMD             0x0047
#define PI_VLINK_DLOCK_INFO_CMD             0x0048
#define PI_VLINK_DLINK_GT2TB_INFO_CMD       0x0049

#define PI_TARGET_COUNT_CMD                 0x0050
#define PI_TARGET_LIST_CMD                  0x0051
#define PI_TARGET_INFO_CMD                  0x0052
#define PI_TARGET_SET_PROPERTIES_CMD        0x0053
#define PI_TARGET_RESOURCE_LIST_CMD         0x0054
#define PI_TARGETS_CMD                      0x005F

#define PI_STATS_GLOBAL_CACHE_CMD           0x0060
#define PI_STATS_CACHE_DEVICE_CMD           0x0061
#define PI_STATS_FRONT_END_PROC_CMD         0x0062
#define PI_STATS_BACK_END_PROC_CMD          0x0063
#define PI_STATS_FRONT_END_LOOP_CMD         0x0064
#define PI_STATS_BACK_END_LOOP_CMD          0x0065
#define PI_STATS_FRONT_END_PCI_CMD          0x0066
#define PI_STATS_BACK_END_PCI_CMD           0x0067
#define PI_STATS_SERVER_CMD                 0x0068
#define PI_STATS_VDISK_CMD                  0x0069
#define PI_STATS_PROC_CMD                   0x006A
#define PI_STATS_PCI_CMD                    0x006B
#define PI_STATS_CACHE_DEVICES_CMD          0x006C
#define PI_STATS_ENVIRONMENTAL_CMD          0x006D
#define PI_STATS_CONFIGURATION_MEMORY       0x006E
#define PI_STATS_HAB_CMD                    0x006F

#define PI_RAID_COUNT_CMD                   0x0070
#define PI_RAID_LIST_CMD                    0x0071
#define PI_RAID_INFO_CMD                    0x0072
#define PI_RAID_INIT_CMD                    0x0073
#define PI_RAID_CONTROL_CMD                 0x0074
#define PI_RAID_MIRRORING_CMD               0x0075
#define PI_RAID_RECOVER_CMD                 0x0076
#define PI_RAIDS_CMD                        0x007F

#define PI_ADMIN_FW_VERSIONS_CMD            0x0080
#define PI_ADMIN_FW_SYS_REL_LEVEL_CMD       0x0081
#define PI_ADMIN_SETTIME_CMD                0x0082
#define PI_ADMIN_LEDCNTL_CMD                0x0083
#define PI_ADMIN_SET_IP_CMD                 0x0084
#define PI_ADMIN_GET_IP_CMD                 0x0085
#define PI_ADMIN_GETTIME_CMD                0x0086

#define PI_DEBUG_MEM_RDWR_CMD               0x0090
#define PI_DEBUG_REPORT_CMD                 0x0091
#define PI_DEBUG_INIT_PROC_NVRAM_CMD        0x0092
#define PI_DEBUG_INIT_CCB_NVRAM_CMD         0x0093
#define PI_DEBUG_GET_SER_NUM_CMD            0x0094
#define PI_DEBUG_MRMMTEST_CMD               0x0095
/* UNUSED                                   0x0096 */
/* UNUSED                                   0x0097 */
#define PI_DEBUG_STRUCT_DISPLAY_CMD         0x0098
#define PI_DEBUG_GET_ELECTION_STATE_CMD     0x0099
#define PI_DEBUG_SCSI_COMMAND_CMD           0x009A
#define PI_DEBUG_BE_LOOP_PRIMITIVE_CMD      0x009B
#define PI_DEBUG_FE_LOOP_PRIMITIVE_CMD      0x009C
#define PI_DEBUG_GET_STATE_RM_CMD           0x009D
#define PI_DEBUG_READWRITE_CMD              0x009E
/* UNUSED                                   0x009F */

#define PI_VCG_VALIDATION_CMD               0x00A0
#define PI_PDISK_SPINDOWN_CMD               0x00A1
#define PI_VCG_PREPARE_SLAVE_CMD            0x00A2
#define PI_VCG_ADD_SLAVE_CMD                0x00A3
#define PI_VCG_PING_CMD                     0x00A4
#define PI_VCG_INFO_CMD                     0x00A5
#define PI_VCG_INACTIVATE_CONTROLLER_CMD    0x00A6
#define PI_VCG_ACTIVATE_CONTROLLER_CMD      0x00A7
#define PI_VCG_SET_CACHE_CMD                0x00A8
#define PI_VCG_GET_MP_LIST_CMD              0x00A9

#define PI_PDISK_FAILBACK_CMD               0x00AA
#define PI_VCG_APPLY_LICENSE_CMD            0x00AB
#define PI_VCG_UNFAIL_CONTROLLER_CMD        0x00AC
#define PI_VCG_FAIL_CONTROLLER_CMD          0x00AD
#define PI_VCG_REMOVE_CONTROLLER_CMD        0x00AE
#define PI_VCG_SHUTDOWN_CMD                 0x00AF

#define PI_DISK_BAY_COUNT_CMD               0x00B0
#define PI_DISK_BAY_LIST_CMD                0x00B1
#define PI_DISK_BAY_INFO_CMD                0x00B2

/* UNUSED                                   0x00B3 */
#define PI_DISK_BAY_DELETE_CMD              0x00B4
#define PI_DISK_BAY_ALARM_CTRL_CMD          0x00B5
#define PI_DISK_BAY_LED_CTRL_CMD            0x00B6
#define PI_SET_GEO_LOCATION_CMD             0x00B7
#define PI_MISC_COUNT_CMD                   0x00B8
#define PI_MISC_LIST_CMD                    0x00B9
#define PI_DISK_BAYS_CMD                    0x00BF

#define PI_PDISK_AUTO_FAILBACK_ENABLE_DISABLE_CMD  0x00C0

#define PI_ENVIRO_DATA_DISK_BAY_CMD         0x00C1
#define PI_ENVIRO_DATA_CTRL_AND_BAY_CMD     0x00C2
#define PI_VCG_CONFIGURE_CMD                0x00C3
#define PI_ENV_II_GET_DATA_CMD              0x00C4
#define PI_ISE_GET_STATUS_CMD               0x00C5

#define PI_SNAPSHOT_READDIR_CMD             0x00D0
#define PI_SNAPSHOT_TAKE_CMD                0x00D1
#define PI_SNAPSHOT_LOAD_CMD                0x00D2
#define PI_SNAPSHOT_CHANGE_CMD              0x00D3

#define PI_FIRMWARE_DOWNLOAD_CMD            0x00E0
#define PI_LOG_INFO_CMD                     0x00E1
#define PI_LOG_CLEAR_CMD                    0x00E2
#define PI_WRITE_BUFFER_MODE5_CMD           0x00E3
#define PI_TRY_CCB_FW_CMD                   0x00E4
#define PI_ROLLING_UPDATE_PHASE_CMD         0x00E5
#define PI_LOG_TEXT_MESSAGE_CMD             0x00E6
#define PI_MULTI_PART_XFER_CMD              0x00E7
#define PI_CUSTOMER_LOG_ACKNOWLEDGE_CMD     0x00E8

#define PI_GENERIC_CMD                      0x00F0
#define PI_GENERIC2_CMD                     0x00F1
#define PI_GENERIC_MRP_CMD                  0x00F2

#define PI_PROC_RESTORE_NVRAM_CMD           0x0100
#define PI_PROC_RESET_FE_QLOGIC_CMD         0x0101
#define PI_PROC_RESET_BE_QLOGIC_CMD         0x0102
#define PI_PROC_START_IO_CMD                0x0103
#define PI_PROC_STOP_IO_CMD                 0x0104
#define PI_PROC_ASSIGN_MIRROR_PARTNER_CMD   0x0105
#define PI_PROC_BE_PORT_LIST_CMD            0x0106
#define PI_PROC_FE_PORT_LIST_CMD            0x0107
#define PI_PROC_BE_DEVICE_PATH_CMD          0x0108
#define PI_PROC_NAME_DEVICE_CMD             0x0109
#define PI_PROC_FAIL_CTRL_CMD               0x010A
#define PI_PROC_FAIL_PORT_CMD               0x010B
#define PI_PROC_TARGET_CONTROL_CMD          0x010C

#define PI_PERSISTENT_DATA_CONTROL_CMD      0x0110
#define PI_CLIENT_PERSISTENT_DATA_CONTROL_CMD 0x0111

#define PI_WCACHE_INVALIDATE_CMD            0x0150

#define PI_MISC_GET_DEVICE_COUNT_CMD        0x0200
#define PI_MISC_RESCAN_DEVICE_CMD           0x0201
#define PI_MISC_GET_BE_DEVICE_LIST_CMD      0x0202
#define PI_MISC_GET_FE_DEVICE_LIST_CMD      0x0203
#define PI_MISC_FILE_SYSTEM_READ_CMD        0x0204
#define PI_MISC_FILE_SYSTEM_WRITE_CMD       0x0205
#define PI_MISC_FAILURE_STATE_SET_CMD       0x0206
#define PI_MISC_GET_MODE_CMD                0x0207
#define PI_MISC_SET_MODE_CMD                0x0208
#define PI_MISC_UNFAIL_INTERFACE_CMD        0x0209
#define PI_MISC_FAIL_INTERFACE_CMD          0x020A
#define PI_MISC_SERIAL_NUMBER_SET_CMD       0x020B
#define PI_MISC_RESYNC_MIRROR_RECORDS_CMD   0x020C
#define PI_MISC_CONTINUE_WO_MP_CMD          0x020D
#define PI_MISC_INVALIDATE_BE_WC_CMD        0x020E
#define PI_MISC_MIRROR_PARTNER_CONTROL_CMD  0x020F

#define PI_MISC_GET_WORKSET_INFO_CMD        0x0210
#define PI_MISC_SET_WORKSET_INFO_CMD        0x0211

/*
** The command slots 212 and 213 can be resued by any application.
*/
#define PI_UNUSED1                          0x0212
#define PI_UNUSED2                          0x0213
#define PI_CACHE_REFRESH_CCB_CMD            0x0214
#define PI_SET_DLM_HEARTBEAT_LIST_CMD       0x0215
#define PI_CACHE_FLUSH_BE_CMD               0x0216
#define PI_MISC_RESYNC_RAIDS_CMD            0x0217
#define PI_MISC_PUTDEVCONFIG_CMD            0x0218
#define PI_MISC_GETDEVCONFIG_CMD            0x0219
#define PI_STATS_BUFFER_BOARD_CMD           0x021A
#define PI_MISC_MIRROR_PARTNER_GET_CFG_CMD  0x021B
#define PI_BATTERY_HEALTH_SET_CMD           0x021C
#define PI_MISC_INVALIDATE_FE_WC_CMD        0x021D
#define PI_STATS_SERVERS_CMD                0x021E
#define PI_VCG_VDISK_PRIORITY_ENABLE_CMD    0x021F

#define PI_MISC_QUERY_MP_CHANGE_CMD         0x0220
#define PI_MISC_RESYNCDATA_CMD              0x0221
#define PI_MISC_RESYNCCTL_CMD               0x0222
#define PI_MISC_SETTDISCACHE_CMD            0x0223
#define PI_MISC_CLRTDISCACHE_CMD            0x0224
#define PI_MISC_QTDISCACHE_CMD              0x0225
#define PI_MISC_CFGOPTION_CMD               0x0226
#define PI_MISC_LOCAL_RAID_INFO_CMD         0x0230
#define PI_ISCSI_SET_TGTPARAM_CMD           0x0229
#define PI_ISCSI_TGT_INFO_CMD               0x0231
#define PI_ISCSI_SET_CHAP_CMD               0x0232
#define PI_ISCSI_CHAP_INFO                  0x0233
#define PI_CLEAR_GEO_LOCATION_CMD           0x0234
#define PI_ISCSI_SESSION_INFO               0x0235
#define PI_ISCSI_SESSION_INFO_SERVER        0x0236
#define PI_GETISNSINFO_CMD                  0x0237
#define PI_IDD_INFO_CMD                     0x0238
#define PI_SETISNSINFO_CMD                  0x0239
#define PI_DLM_PATH_STATS_CMD               0x0240
#define PI_DLM_PATH_SELECTION_ALGO_CMD      0x0241
#define PI_SET_PR_CMD                       0x0242
#define PI_GET_CPUCOUNT_CMD                 0x0243
#define PI_ENABLE_X1_PORT_CMD               0x0244   // RESERVED -- used to be X1 port enable.
#define PI_VDISK_BAY_REDUNDANT_CMD          0x0245
#define PI_GET_BACKEND_TYPE_CMD             0x0246
#define PI_QUICK_BREAK_PAUSE_RESUME_MIRROR_START_CMD     0x0247
#define PI_QUICK_BREAK_PAUSE_RESUME_MIRROR_SEQUENCE_CMD  0x0248
#define PI_QUICK_BREAK_PAUSE_RESUME_MIRROR_EXECUTE_CMD   0x0249
#define PI_BEACON_ISE_COMPONENT             0x024A
#define PI_PDISKS_FROM_CACHE_CMD            0x024B
#define PI_VDISKS_FROM_CACHE_CMD            0x024C
#define PI_RAIDS_FROM_CACHE_CMD             0x024D
#define PI_BATCH_SNAPSHOT_START_CMD         0x024E
#define PI_BATCH_SNAPSHOT_SEQUENCE_CMD      0x024F
#define PI_BATCH_SNAPSHOT_EXECUTE_CMD       0x0250

#define PI_MFG_CTRL_CLEAN_CMD               0x0300

#define PI_LOG_EVENT_MESSAGE                0x0500
#define PI_ASYNC_CHANGED_EVENT              0x0501
#define PI_ASYNC_PING_EVENT                 0x0502

/*--------------------------------------------------------------------------*/
/*   Generic Structures                                                     */
/*--------------------------------------------------------------------------*/
typedef struct _PI_32_BIT_STATE_RSP
{
    UINT32      state;          /* 32 bit state value */
} PI_32_BIT_STATE_RSP;

/*--------------------------------------------------------------------------*/
/*   Generic Object Packets                                                 */
/*--------------------------------------------------------------------------*/
/* --- Count Request --- */
/* No input parameters are required */

/* --- Count Response --- */
typedef struct _PI_COUNT_RSP
{
    UINT16      count;          /* Count of objects */
} PI_COUNT_RSP;

/* --- List Request --- */
/* No input parameters are required */

/* --- List Response --- */
/* Use typedef from mrp.h when proc code is changed to match the typedef below.*/
typedef struct _PI_LIST_RSP
{
    UINT16      count;          /* Number of items in the list that follows */
    UINT16      list[1];        /* List of IDs                              */
} PI_LIST_RSP;

/*--------------------------------------------------------------------------*/
/* Miscellaneous Packets                                                    */
/*--------------------------------------------------------------------------*/
/* --- Reset Request  --- */
typedef struct _PI_RESET_REQ
{
    UINT32      which;          /* Which processor(s) to reset */
    /* FROM KERNEL.H            */
    /*      PROCESS_CCB == 0  */
    /*      PROCESS_FE == 1   */
    /*      PROCESS_BE == 2   */
    /*      PROCESS_ALL == 3  */
    UINT8       delayed;        /* Do a delayed reset? */
    UINT8       rsvd[3];        /* RESERVED */
} PI_RESET_REQ;

/* --- Reset Response --- */
/* No additional response data beyond the status in the header. */

/* --- Power-Up State Request  --- */
/* No input parameters are required */

/* --- Power-UP State Response --- */
typedef struct PI_POWER_UP_STATE_RSP
{
    UINT16      state;              /**< Power-up State (see cps_init.h)    */
    UINT16      astatus;            /**< Additional status (see cps_init.h) */
} PI_POWER_UP_STATE_RSP;

/* --- Power-Up State Request  --- */
typedef struct _PI_POWER_UP_RESPONSE_REQ
{
    UINT16      state;              /**< Power-up State (see cps_init.h)    */
    UINT16      astatus;            /**< Additional status (see cps_init.h) */
    UINT8       response;           /**< Response to the given state        */
    UINT8       rsvd[3];            /**< RESERVED                           */
} PI_POWER_UP_RESPONSE_REQ;

/* --- Power-UP State Response --- */
/* No additional response data beyond the status in the header. */

#define POWER_UP_RESPONSE_RETRY         1 /**< Retry, see if issue resolved */
#define POWER_UP_RESPONSE_CONTINUE      2 /**< Just continue                */
#define POWER_UP_RESPONSE_RESET         3 /**< Reset the controller         */
#define POWER_UP_RESPONSE_CONTINUE_KA   4 /**< Continue as Keep-Alive       */
#define POWER_UP_RESPONSE_WC_SAVE       5 /**< Write Cache, Save Data       */
#define POWER_UP_RESPONSE_WC_DISCARD    6 /**< Write Cache, Discard Data    */

/* ------ Enable/disable X1 port ---------- */
typedef struct _PI_ENABLE_X1_PORT_REQ
{
    UINT8       enable;         /* <1 for enable and 0 for disable > */
    UINT8       rsvd[3];        /* <RESERVED                      > */
} PI_ENABLE_X1_PORT_REQ;

/*-------- Get Backend type loop or fabric-----*/
#define PI_BACKEND_TYPE_FABRIC          1
#define PI_BACKEND_TYPE_LOOP            0

typedef struct _PI_GET_BACKEND_TYPE_RSP
{
    UINT8       beType;         /* 0 for LOOP 1 for FABRIC  */
    UINT8       rsvd[3];        /* RSERVED                  */
} PI_GET_BACKEND_TYPE_RSP;

/* --- X1 Compatibility Index Request --- */
/* No input parameters are required */

/* --- X1 Compatibility Index Response --- */
#define PI_X1_COMPATIBILITY_INDEX_RSP   PI_32_BIT_STATE_RSP

/* --- Register Events Request --- */
#define PI_REGISTER_EVENTS_SET  0
#define PI_REGISTER_EVENTS_GET  1

typedef struct _PI_REGISTER_EVENTS_REQ
{
    UINT64      registerEvents;
                             /**< Bitmap of events to register for
                                    Bit 0 - PCHANGED events
                                    Bit 1 - RCHANGED events
                                    Bit 2 - VCHANGED events
                                    Bit 3 - HCHANGED events
                                    Bit 4 - ACHANGED events
                                    Bit 5 - ZCHANGED events

                                    Bits 8-10 - Client Persistent Data Change
                                    Bit 8 - Record Created
                                    Bit 9 - Record Deleted
                                    Bit 10 - Record Modified

                                    Bit 12 - Buffer Board Status Change

                                    Bit 16 - VCG_ELECTION_STATE_CHANGED
                                    Bit 17 - VCG_ELECTION_ENDED events
                                    Bit 18 - VCG_POWERUP events
                                    Bit 19 - VCG_CFG_CHANGED events
                                    Bit 20 - VCG_WORKSET_CHANGED events
                                    BIT 21 - Reserved, can be reused.
                                    Bit 22 - Snappool changed events
                                    Bit 23 - ISE environments changed events
                                    Bit 24 - VCG_BE_PORT_CHANGE events
                                    Bit 25 - VCG_FE_PORT_CHANGE events

                                    Bit 28 - Ping Event
                                    Bit 29 - Log Message Standard text
                                    Bit 30 - Log Message Extended text
                                    Bit 31 - Log Message BinaryEvents

                                Set or clear these bits to register or
                                unregister for these events respectively.
                                (this field is used only when opt=0)
                                Bit positions are documented
                                in X1_AsyncEventHandler.h       */

    UINT8       opt;         /**< 0 - Set, 1 - Get             */
    UINT8       rsvd[3];
} PI_REGISTER_EVENTS_REQ;

/* --- Register Events Response --- */
typedef struct _PI_REGISTER_EVENTS_RSP
{
    UINT64      eventsRegisteredOn; /**< 1 - Registered, 0 - Unregistered */
} PI_REGISTER_EVENTS_RSP;

/*--------------------------------------------------------------------------*/
/* Registr Client Type Command packet                                            */
/*--------------------------------------------------------------------------*/
/* Client type */
#define PI_UNSPECIFIED_CLIENT     0x00
#define PI_IWS_CLIENT             0x01

/* --- Register client type request --- */
typedef struct _PI_REGISTER_CLIENT_TYPE_REQ
{
    UINT8       type;           /* client type */
    UINT8       rsvd[3];        /* reserved for future use */
} PI_REGISTER_CLIENT_TYPE_REQ;

typedef struct _PI_REGISTER_CLIENT_TYPE_RSP
{
    UINT8       type;           /* client type */
    UINT8       nconn;          /* number of connections including
                                 * the present one */
    UINT16      rsvd;           /* reserved */
} PI_REGISTER_CLIENT_TYPE_RSP;

/* --- Restore Processor NVRAM Request  --- */
#define PI_RESTORE_PROC_NVRAM_REQ       MRRESTORE_REQ

/* --- Restore Processor NVRAM Response --- */
#define PI_RESTORE_PROC_NVRAM_RSP       MRRESTORE_RSP

/* --- Reset Processor QLogic Request  --- */
#define PI_RESET_PROC_QLOGIC_REQ        MRRESETPORT_REQ /* MR_Defs.h    */

/* --- Reset Processor QLogic Response --- */
#define PI_RESET_PROC_QLOGIC_RSP        MRRESETPORT_RSP /* MR_Defs.h    */

/* --- Start All I/O Request --- */
#define PI_PROC_START_IO_REQ            MRSTARTIO_REQ   /* MR_Defs.h    */

/* -- Start All I/O Response --- */
#define PI_PROC_START_IO_RSP            MRSTARTIO_RSP   /* MR_Defs.h    */

/* --- Stop All I/O Request --- */
#define PI_PROC_STOP_IO_REQ             MRSTOPIO_REQ    /* MR_Defs.h    */

/* -- Stop All I/O Response --- */
#define PI_PROC_STOP_IO_RSP             MRSTOPIO_RSP    /* MR_Defs.h    */

/* --- Assign Mirror Partner Request --- */
typedef struct PI_PROC_ASSIGN_MIRROR_PARTNER_REQ
{
    UINT32      serialNumber; /**< New mirror partner serial number           */
} PI_PROC_ASSIGN_MIRROR_PARTNER_REQ;

/* -- Assign Mirror Partner Response --- */
typedef struct PI_PROC_ASSIGN_MIRROR_PARTNER_RSP
{
    UINT32      serialNumber; /**< Old mirror partner serial number           */
} PI_PROC_ASSIGN_MIRROR_PARTNER_RSP;

/* -- Back End Device Path Request --- */
#define PI_PROC_BE_DEVICE_PATH_REQ      MRGETBEDEVPATHS_REQ     /* MR_Defs.h    */

/* -- Back End Device Path Response --- */
#define PI_PROC_BE_DEVICE_PATH_RSP      MRGETBEDEVPATHS_RSP     /* MR_Defs.h    */

/* -- Back End Device Path Request --- */
#define PI_PROC_NAME_DEVICE_REQ         MRNAMEDEVICE_REQ        /* MR_Defs.h    */

/* -- Back End Device Path Response --- */
#define PI_PROC_NAME_DEVICE_RSP         MRNAMEDEVICE_RSP        /* MR_Defs.h    */

/* -- Fail Controller Request --- */
#define PI_PROC_FAIL_CTRL_REQ           MRFAILCTRL_REQ  /* MR_Defs.h    */

/* -- Fail Controller Response --- */
#define PI_PROC_FAIL_CTRL_RSP           MRFAILCTRL_RSP  /* MR_Defs.h    */

/* -- Fail Port Request --- */
#define PI_PROC_FAIL_PORT_REQ           MRFAILPORT_REQ  /* MR_Defs.h    */

/* -- Fail Port Response --- */
#define PI_PROC_FAIL_PORT_RSP           MRFAILPORT_RSP  /* MR_Defs.h    */

/* -- Target Control Request --- */
#define PI_PROC_TARGET_CONTROL_REQ      MRTARGETCONTROL_REQ     /* MR_Defs.h    */

/* -- Target Control Response --- */
#define PI_PROC_TARGET_CONTROL_RSP      MRTARGETCONTROL_RSP     /* MR_Defs.h    */

/*--------------------------------------------------------------------------*/
/*   Physical Disk Packets                                                  */
/*--------------------------------------------------------------------------*/
/* --- PDisk Count Request --- */
/* No input parameters are required */

/* --- PDisk Count Response --- */
#define PI_PDISK_COUNT_RSP              PI_COUNT_RSP

/* --- PDisk List Request --- */
/* No input parameters are required */

/* --- PDisk List Response --- */
#define PI_PDISK_LIST_RSP               PI_LIST_RSP

/* --- PDisk Info Request --- */
#define PI_PDISK_INFO_REQ               MRGETPINFO_REQ  /* MR_Defs.h    */

/* --- PDisk Info Response --- */
#define PI_PDISK_INFO_RSP               MRGETPINFO_RSP  /* MR_Defs.h    */

/* --- PDisk Label Request --- */
typedef struct _PI_PDISK_LABEL_REQ
{
    UINT16      pid;            /* PID of the drive to be labeled           */
    UINT8       labtype;        /* Label type                               */
    UINT8       option;         /* Option - this is not used externally but */
    /*          is used from the pre and post   */
    /*          command processing.             */
} PI_PDISK_LABEL_REQ;

/* --- PDisk Label Response --- */
#define PI_PDISK_LABEL_RSP              MRLABEL_RSP     /* MR_Defs.h    */

/* --- PDisk Rename Request --- */
/* --- PDisk Rename Response --- */
/* The PDisk rename function is handled in the XMC    */

/* --- PDisk Defrag Request --- */
#define PI_PDISK_DEFRAG_REQ             MR_DEVID_REQ    /* MR_Defs.h    */
#define PI_PDISK_DEFRAG_ALL     0xFFFF  /**< PID to indicate defrag all     */
#define PI_PDISK_DEFRAG_STOP    0xFFFE  /**< PID to indicate defrag stop    */
#define PI_PDISK_DEFRAG_ORPHANS 0xFFFD  /**< PID to kill orphan RAIDs       */

/* --- PDisk Defrag Response --- */
#define PI_PDISK_DEFRAG_RSP             MR_GENERIC_RSP  /* MR_Defs.h    */

/* --- PDisk Defrag Status Request --- */
/* No input parameters are required */

/* --- PDisk Defrag Status Response --- */
typedef struct _PI_PDISK_DEFRAG_STATUS_RSP
{
    UINT16      pdiskID;    /**< PDisk defragmenting. 0xFFFF=defrag ALL */
    UINT8       flag;       /**< 0x01=defrag in progress                */
    UINT8       rsvd;
} PI_PDISK_DEFRAG_STATUS_RSP;

/* --- PDisk Fail Request --- */
#define PI_PDISK_FAIL_REQ               MRFAIL_REQ      /* MR_Defs.h    */

/* --- PDisk Fail Response --- */
#define PI_PDISK_FAIL_RSP               MRFAIL_RSP      /* MR_Defs.h    */

/* --- PDisk UnFail Request --- */
#define PI_PDISK_UNFAIL_REQ             MRRESTOREDEV_REQ        /* MR_Defs.h    */

/* --- PDisk UnFail Response --- */
#define PI_PDISK_UNFAIL_RSP             MRRESTOREDEV_RSP        /* MR_Defs.h    */

/* --- PDisk Beacon Request --- */
typedef struct _PI_PDISK_BEACON_REQ
{
    UINT16      id;             /* PID of the PDisk to beacon           */
    UINT16      duration;       /* Time in seconds to beacon the PDisk  */
} PI_PDISK_BEACON_REQ;

/* --- PDisk Beacon Response --- */
#define PI_PDISK_BEACON_RSP             MR_GENERIC_RSP  /* MR_Defs.h -- Is this correct? */

/* --- PDisk Delete Request --- */
#define PI_PDISK_DELETE_REQ             MRDELETEDEVICE_REQ      /* MR_Defs.h    */

/* --- PDisk Delete Response --- */
#define PI_PDISK_DELETE_RSP             MRDELETEDEVICE_RSP      /* MR_Defs.h    */

/* --- PDisk Bypassn Request --- */
typedef struct _PI_PDISK_BYPASS_REQ
{
    UINT16      ses;            /* SES of the disk bay                          */
    UINT8       slot;           /* Slot of the physical disk                    */
    UINT8       setting;        /* Bypass Setting                               */
} PI_PDISK_BYPASS_REQ;

/* --- PDisk Bypass Response --- */
/* No additional response data beyond the status in the header. */

#define PI_PDISK_SPINDOWN_REQ           MRPDISK_SPINDOWN_REQ    /* MR_Defs.h    */
#define PI_PDISK_SPINDOWN_RSP           MRPDISK_SPINDOWN_RSP    /* MR_Defs.h    */
#define PI_PDISK_FAILBACK_REQ           MRPDISKFAILBACK_REQ     /* MR_Defs.h    */
#define PI_PDISK_FAILBACK_RSP           MRPDISKFAILBACK_RSP     /* MR_Defs.h    */
/** Auto FailBack Enable/Disable - Response **/
typedef struct PI_PDISK_AUTO_FAILBACK_ENABLE_DISABLE_RSP
{
// NOTE: there is no MR_HDR_RSP in this packet.
    UINT8           mode;       /* Mode ON/OFF */
    UINT8           rsvd[2];    /* Reserved */
    UINT8           status;     /* Should have MRP status passed back. */
} PI_PDISK_AUTO_FAILBACK_ENABLE_DISABLE_RSP;

#define PI_SET_GEO_LOCATION_REQ         MR_SET_GEO_LOCATION_REQ /* MR_Defs.h */
#define PI_SET_GEO_LOCATION_RSP         MR_SET_GEO_LOCATION_RSP /* MR_Defs.h */
#define PI_CLEAR_GEO_LOCATION_REQ       MR_CLEAR_GEO_LOCATION_REQ       /* MR_Defs.h */
#define PI_CLEAR_GEO_LOCATION_RSP       MR_CLEAR_GEO_LOCATION_RSP       /* MR_Defs.h */
#define PI_DLM_PATH_STATS_RSP           MRDLMPATHSTATS_RSP      /* MR_Defs.h */
#define PI_DLM_PATH_SELECTION_ALGO_REQ  MRDLMPATHSELECTIONALGO_REQ      /* MR_Defs.h */
#define PI_DLM_PATH_SELECTION_ALGO_RSP  MRDLMPATHSELECTIONALGO_RSP      /* MR_Defs.h */
#define PI_SET_PR_REQ                   MRSETPRES_REQ   /* MR_Defs.h */
#define PI_SET_PR_RSP                   MRSETPRES_RSP   /* MR_Defs.h */

/* --- Physical Disks Request --- */
/* No input parameters are required */

/* --- Physical Disks Response --- */
typedef struct _PI_PDISKS_RSP
{
    UINT16      count;          /* Number of PDisks                 */
    UINT8       rsvd[2];        /* RESERVED                         */
    ZeroArray(PI_PDISK_INFO_RSP, pdiskInfo); /* Physical Disks Information  */
} PI_PDISKS_RSP;

/*--------------------------------------------------------------------------*/
/* Virtual Disk  and RAID Packets                                           */
/*--------------------------------------------------------------------------*/
/* --- VDisk Count Request --- */
/* No input parameters are required */

/* --- VDisk Count Response --- */
#define PI_VDISK_COUNT_RSP              PI_COUNT_RSP

/* --- VDisk List Request --- */
/* No input parameters are required */

/* --- VDisk List Response --- */
#define PI_VDISK_LIST_RSP               PI_LIST_RSP

/* --- VDisk Info Request --- */
#define PI_VDISK_INFO_REQ               MRGETVINFO_REQ  /* MR_Defs.h    */

/* --- VDisk Info Response --- */
typedef struct PI_VDISK_INFO_RSP
{
    MR_HDR_RSP  header;         /**< MRP Response Header - 8 bytes          */
    UINT16      vid;            /**< Virtual device ID                      */
    UINT8       mirror;         /**< Mirror status                          */
    UINT8       status;         /**< VDisk status                           */
    UINT16      scorvid;        /**< Secondary copy orig. VID               */
    UINT8       scpComp;        /**< Secondary copy percent complete        */
    UINT8       raidCnt;        /**< Number of RAIDs in this VDisk          */
    UINT64      devCap;         /**< Device capacity                        */
    UINT32      error;          /**< Error count                            */
    UINT32      qd;             /**< Queue depth                            */
    UINT32      rps;            /**< Avg req/sec (last second)              */
    UINT32      avgSC;          /**< Avg sector count (last second)         */
    UINT64      rReq;           /**< Read request count                     */
    UINT64      wReq;           /**< Write request count                    */
    UINT16      attr;           /**< VDisk attribute                        */
    UINT8       draidCnt;       /**< Deferred RAID count                    */
    UINT8       owner;          /**< Owner                                  */
    UINT8       priority;       /**< Priority                               */
    GR_GeoRaidVdiskInfo grInfo; /**< Geo-RAID services related info         */
    UINT32      sprCnt;         /**< Sample period request count            */
    UINT32      spsCnt;         /**< Sample period sector  count            */
    void       *pSCHead;        /**< Original vd scmt list head pointer     */
    void       *pSCTail;        /**< Original vd scmt list tail pointer     */
    void       *pCPScmt;        /**< Copy vd scmt pointer                   */
    struct VLAR *pVLinks;       /**< VLinks assoc. records                  */
    UINT8       name[MAX_VDISK_NAME_LEN];     /**< VDisk name               */
    ZeroArray(UINT16, rid);     /**< RAID ID list                   */
} PI_VDISK_INFO_RSP;

#define PI_VDISK_INFO2_RSP               MRGETVINFO_RSP /* MR_Defs.h    */

typedef struct PI_VDISK_INFO3_RSP
{
    MRGETVINFO_RSP vinfo;
    ZeroArray(MREXTENDED_VINFO_RSP_PKT, extVinfo); /**< RAID ID list    */
} PI_VDISK_INFO3_RSP;

/* --- RAID Info Request --- */
#define PI_RAID_INFO_REQ                MRGETRINFO_REQ  /* MR_Defs.h    */

/* --- RAID Info Response --- */
#define PI_RAID_INFO_RSP                MRGETRINFO_RSP  /* MR_Defs.h    */

/* --- RAID Initialization Request --- */
#define PI_RAID_INIT_REQ                MRINITRAID_REQ  /* MR_Defs.h    */

/* --- RAID Initialization Response --- */
#define PI_RAID_INIT_RSP                MRINITRAID_RSP  /* MR_Defs.h    */

/* --- RAID Control (Scrubbing) Request --- */
#define PI_RAID_CONTROL_REQ             MRSCRUBCTRL_REQ /* MR_Defs.h    */

/* --- Scrub Control Response --- */
#define PI_RAID_CONTROL_RSP             MRSCRUBCTRL_RSP /* MR_Defs.h    */

/* --- RAID Recover Request --- */
#define PI_RAID_RECOVER_REQ             MRRAIDRECOVER_REQ       /* MR_Defs.h    */

/* --- Raid Recover Response --- */
#define PI_RAID_RECOVER_RSP             MRRAIDRECOVER_RSP       /* MR_Defs.h    */

/* --- VDisk Create Request --- */
#define PI_VDISK_CREATE_REQ             MRCREXP_REQ     /* MR_Defs.h    */

/* --- VDisk Create Response --- */
#define PI_VDISK_CREATE_RSP             MRCREXP_RSP     /* MR_Defs.h    */

/* --- VDisk Set Priority Request --- */
#define PI_SET_VPRI_REQ                 MRSETVPRI_REQ   /* MR_Defs.h    */

/* --- VDisk Set Priority Response --- */
#define PI_SET_VPRI_RSP                 MRSETVPRI_RSP   /* MR_Defs.h    */

/* --- VDisk Priority Enable Request --- */
#define PI_VPRI_ENABLE_REQ              MRVPRI_ENABLE_REQ       /* MR_Defs.h    */

/* --- VDisk Priority Enable Response --- */
#define PI_VPRI_ENABLE_RSP              MRVPRI_ENABLE_RSP       /* MR_Defs.h    */

/* --- VDisk Delete Request --- */
#define PI_VDISK_DELETE_REQ             MRDELVIRT_REQ   /* MR_Defs.h    */

/* --- VDisk Delete Response --- */
#define PI_VDISK_DELETE_RSP             MRDELVIRT_RSP   /* MR_Defs.h    */

/* --- VDisk Redundancy Request --- */
#define PI_VDISK_REDUNDANCY_REQ         MRVIRTREDUNDANCY_REQ    /* MR_Defs.h    */

/* --- VDisk Redundancy Response --- */
#define PI_VDISK_REDUNDANCY_RSP         MRVIRTREDUNDANCY_RSP    /* MR_Defs.h    */

/* --- Vdisk PR Clear Request --- */
#define PI_PR_CLR_REQ                   MRPRCLR_REQ

/* --- Vdisk PR Clear Response --- */
#define PI_PR_CLR_RSP                   MRPRCLR_RSP

/* --- Vdisk PR Get Request --- */
#define PI_PR_GET_REQ                   MRPRGET_REQ

/* --- Vdisk PR Get Response --- */
#define PI_PR_GET_RSP                   MRPRGET_RSP

/* --- VDisk Rename Request --- */
/* --- VDisk Rename Response --- */
/* The VDisk rename function is handled in the XMC    */

/* --- VDisk Expand Request --- */
#define PI_VDISK_EXPAND_REQ             MRCREXP_REQ     /* MR_Defs.h    */

/* --- VDisk Expand Response --- */
#define PI_VDISK_EXPAND_RSP             MRCREXP_RSP     /* MR_Defs.h    */

/* --- VDisk Control Request --- */
#define PI_VDISK_CONTROL_REQ            MRVDISKCONTROL_REQ      /* MR_Defs.h    */

/* --- VDisk Control Response --- */
#define PI_VDISK_CONTROL_RSP            MRVDISKCONTROL_RSP      /* MR_Defs.h    */

/* --- VDisk Owner Request --- */
#define PI_VDISK_OWNER_REQ              MRGETVIDOWNER_REQ       /* MR_Defs.h    */

/* --- VDisk Owner Response --- */
#define PI_VDISK_OWNER_RSP              MRGETVIDOWNER_RSP       /* MR_Defs.h    */

/* --- VDisk Set Attribute Request --- */
typedef struct _PI_VDISK_SET_ATTRIBUTE_REQ
{
    UINT8       change;         /* Change being made                */
    UINT8       rsvd1[3];       /* Reserved                         */
    MRSETATTR_REQ attrRequest;  /* attribute request for passthru   */
} PI_VDISK_SET_ATTRIBUTE_REQ;

/* --- VDisk Set Cache Response --- */
#define PI_VDISK_SET_ATTRIBUTE_RSP      MRSETATTR_RSP   /* MR_Defs.h    */

/*
** Set Attribute Change Modes
*/
#define PI_VDISK_CHANGE_UNKNOWN               0x00      /* Unknown          */
#define PI_VDISK_CHANGE_NORMAL                0x01      /* Normal           */
#define PI_VDISK_CHANGE_HIDDEN                0x02      /* Hidden           */
#define PI_VDISK_CHANGE_PRIVATE               0x03      /* Private          */
#define PI_VDISK_CHANGE_ASYNCH                0x04      /* Asynch           */
#define PI_VDISK_CHANGE_LOCK                  0x10      /* Lock             */
#define PI_VDISK_CHANGE_UNLOCK                0x11      /* Unlock           */
#define PI_VDISK_CHANGE_VLINK_FLAG            0x20      /* Flag             */
#define PI_VDISK_CHANGE_VLINK_UNFLAG          0x21      /* Unflag           */
#define PI_VDISK_CHANGE_VDISK_LOCK            0x30      /* Vdisk Lock       */
#define PI_VDISK_CHANGE_VDISK_UNLOCK          0x31      /* Vdisk Unlock     */
#define PI_VDISK_CHANGE_VDISK_CACHE_ENABLE    0x40      /* Cache Enabled    */
#define PI_VDISK_CHANGE_VDISK_CACHE_DISABLE   0x41      /* Cache disabled   */

/* --- Virtual Disks Request --- */
/* No input parameters are required */

/* --- Virtual Disks Response --- */
typedef struct _PI_VDISKS_RSP
{
    UINT16      count;          /* Number of PDisks                 */
    UINT8       rsvd[2];        /* RESERVED                         */
    ZeroArray(UINT8, vdisks);   /* Start of the VDISK information   */
} PI_VDISKS_RSP;

/* --- Raids Request --- */
/* No input parameters are required */

/* --- Raids Response --- */
typedef struct _PI_RAIDS_RSP
{
    UINT16      count;          /* Number of PDisks                 */
    UINT8       rsvd[2];        /* RESERVED                         */
    ZeroArray(UINT8, raids);    /* Start of the RAID information    */
} PI_RAIDS_RSP;

/*--------------------------------------------------------------------------*/
/* Server Packets                                                           */
/*--------------------------------------------------------------------------*/

/* --- Server Count Request --- */
/* No input parameters are required */

/* --- Server Count Response --- */
#define PI_SERVER_COUNT_RSP             PI_COUNT_RSP

/* --- Server List Request --- */
/* No input parameters are required */

/* --- Server List Response --- */
#define PI_SERVER_LIST_RSP              PI_LIST_RSP

/* --- Server Info Request --- */
#define PI_SERVER_INFO_REQ              MRGETSINFO_REQ  /* MR_Defs.h    */

/* --- Server Info Response --- */
#define PI_SERVER_INFO_RSP              MRGETSINFO_RSP  /* MR_Defs.h    */

/* --- Server Create Request --- */
#define PI_SERVER_CREATE_REQ            MRCREATESERVER_REQ      /* MR_Defs.h    */

/* --- Server Create Response --- */
#define PI_SERVER_CREATE_RSP            MRCREATESERVER_RSP      /* MR_Defs.h    */

/* --- Server Delete Request --- */
#define PI_SERVER_DELETE_REQ            MRDELETESERVER_REQ      /* MR_Defs.h    */

/* --- Server Delete Response --- */
#define PI_SERVER_DELETE_RSP            MRDELETESERVER_RSP      /* MR_Defs.h    */

/* --- Server Rename Request --- */
/* --- Server Rename Response --- */
/* The Server rename function is handled in the XMC    */

/* --- Server Associate Request --- */
#define PI_SERVER_ASSOCIATE_REQ         MRMAPLUN_REQ    /* MR_Defs.h    */

/* --- Server Associate Response --- */
#define PI_SERVER_ASSOCIATE_RSP         MRMAPLUN_RSP    /* MR_Defs.h    */

/* --- Server Disassociate Request --- */
#define PI_SERVER_DISASSOCIATE_REQ      MRUNMAPLUN_REQ  /* MR_Defs.h    */

/* --- Server Disassociate Response --- */
#define PI_SERVER_DISASSOCIATE_RSP      MRUNMAPLUN_RSP  /* MR_Defs.h    */

/* --- Server Set Properties Request --- */
#define PI_SERVER_SET_PROPERTIES_REQ    MRSERVERPROP_REQ        /* MR_Defs.h    */

/* --- Server Set Properties Response --- */
#define PI_SERVER_SET_PROPERTIES_RSP    MRSERVERPROP_RSP        /* MR_Defs.h    */

/* --- Server Lookup Request --- */
#define PI_SERVER_LOOKUP_REQ            MRSERVERLOOKUP_REQ      /* MR_Defs.h    */

/* --- Server Lookup Response --- */
#define PI_SERVER_LOOKUP_RSP            MRSERVERLOOKUP_RSP      /* MR_Defs.h    */

/* --- Servers Request --- */
/* No input parameters are required */

/* --- Servers Response --- */
typedef struct _PI_SERVERS_RSP
{
    UINT16      count;          /* Number of Servers                */
    UINT8       rsvd[2];        /* RESERVED                         */
    ZeroArray(PI_SERVER_INFO_RSP, servers); /* Start of the SERVER information  */
} PI_SERVERS_RSP;

/* --- Server WWN to Target Map Request --- */
/* No input parameters are required */

typedef struct wwnToTargetMap_t
{
    UINT64      wwn;            /* Server WWN                                   */
    UINT8       targetBitmap;   /* Bit map of ports this server is visible on   */
    UINT8       rsvd[3];
} WWN_TO_TARGET_MAP;

/* --- Server WWN to Target Map Response --- */
typedef struct _PI_WWN_TO_TARGET_MAP_RSP
{
    UINT16      count;          /* Number of map entries             */
    UINT8       rsvd[2];        /* RESERVED                          */
    ZeroArray(WWN_TO_TARGET_MAP, map); /* WWN to port Map - see struct above */
} PI_WWN_TO_TARGET_MAP_RSP;

/*--------------------------------------------------------------------------*/
/* Target Packets                                                           */
/*--------------------------------------------------------------------------*/

/* --- Target Count Request --- */
/* No input parameters are required */

/* --- Target Count Response --- */
#define PI_TARGET_COUNT_RSP             PI_COUNT_RSP

/* --- Target List Request --- */
/* No input parameters are required */

/* --- Target List Response --- */
#define PI_TARGET_LIST_RSP              PI_LIST_RSP

/* --- Target Info Request --- */
#define PI_TARGET_INFO_REQ              MRGETTARG_REQ   /* MR_Defs.h    */

/* --- Target Info Response --- */
#define PI_TARGET_INFO_RSP              MRGETTARG_RSP   /* MR_Defs.h    */

/* --- Target Set Properties Request --- */
#define PI_TARGET_SET_PROPERTIES_REQ    MRCONFIGTARG_REQ        /* MR_Defs.h    */

/* --- Target Set Properties Response --- */
#define PI_TARGET_SET_PROPERTIES_RSP    MRCONFIGTARG_RSP        /* MR_Defs.h    */

/* --- Target Resource List Request --- */
#define PI_TARGET_RESOURCE_LIST_REQ     MRGETTRLIST_REQ /* MR_Defs.h    */

/* --- Target Resource List Response --- */
#define PI_TARGET_RESOURCE_LIST_RSP     MRGETTRLIST_RSP /* MR_Defs.h    */

/* --- Targets Request --- */
/* No input parameters are required */

/* --- Targets Response --- */
typedef struct _PI_TARGETS_RSP
{
    UINT16      count;          /* Number of Targets                */
    UINT8       rsvd[2];        /* RESERVED                         */
    ZeroArray(PI_TARGET_INFO_RSP, targetInfo); /* Targets Information      */
} PI_TARGETS_RSP;

/*--------------------------------------------------------------------------*/
/* Virtual Link Packets                                                     */
/*--------------------------------------------------------------------------*/

/* --- VLink Remote Controller Count Request --- */
/* No input parameters are required */

/* --- VLink Remote Controller Count Response --- */
#define PI_VLINK_REMOTE_CNT_RSP         MRREMOTECTRLCNT_RSP     /* MR_Defs.h    */


/* --- VLink Remote Controller Info Request --- */
#define PI_VLINK_REMOTE_INFO_REQ        MRREMOTECTRLINFO_REQ    /* MR_Defs.h   */

/* --- VLink Remote Controller Info Response --- */
#define PI_VLINK_REMOTE_INFO_RSP        MRREMOTECTRLINFO_RSP    /* MR_Defs.h   */


/* --- VLink Remote Controller VDisks Request --- */
#define PI_VLINK_REMOTE_VDISKS_REQ      MRREMOTEVDISKINFO_REQ   /* MR_Defs.h   */

/* --- VLink Remote Controller VDisks Response --- */
#define PI_VLINK_REMOTE_VDISKS_RSP      MRREMOTEVDISKINFO_RSP   /* MR_Defs.h   */

/* --- VLink Create Request --- */
#define PI_VLINK_CREATE_REQ             MRCREATEVLINK_REQ       /* MR_Defs.h    */

/* --- VLink Create Response --- */
#define PI_VLINK_CREATE_RSP             MRCREATEVLINK_RSP       /* MR_Defs.h    */

/* --- VLink Info Request --- */
#define PI_VLINK_INFO_REQ               MRVLINKINFO_REQ /* MR_Defs.h    */

/* --- VLink Info Response --- */
#define PI_VLINK_INFO_RSP               MRVLINKINFO_RSP /* MR_Defs.h    */

/* --- Break VLink Lock Request --- */
#define PI_VLINK_BREAK_LOCK_REQ         MRBREAKVLOCK_REQ        /* MR_Defs.h    */

/* --- Break VLink Lock Response --- */
#define PI_VLINK_BREAK_LOCK_RSP         MRBREAKVLOCK_RSP        /* MR_Defs.h    */

/* --- VLink - VDisk or Controller Name Changed Request --- */
#define PI_VLINK_NAME_CHANGED_REQ       MRNAMECHANGE_REQ        /* MR_Defs.h    */

/* --- VLink - VDisk or Controller Name Changed Response --- */
#define PI_VLINK_NAME_CHANGED_RSP       MRNAMECHANGE_RSP        /* MR_Defs.h    */

/* --- VLink - DLink Info Request --- */
#define PI_VLINK_DLINK_INFO_REQ         MRGETDLINK_REQ          /* MR_Defs.h    */
#define PI_VLINK_DLINK_GT2TB_INFO_REQ   MRGETDLINK_GT2TB_REQ    /* MR_Defs.h    */

/* --- VLink - DLink Info Response --- */
#define PI_VLINK_DLINK_INFO_RSP         MRGETDLINK_RSP          /* MR_Defs.h    */
#define PI_VLINK_DLINK_GT2TB_INFO_RSP   MRGETDLINK_GT2TB_RSP    /* MR_Defs.h    */

/* --- VLink - DLock Info Request --- */
#define PI_VLINK_DLOCK_INFO_REQ         MRGETDLOCK_REQ  /* MR_Defs.h    */

/* --- VLink - DLock Info Response --- */
#define PI_VLINK_DLOCK_INFO_RSP         MRGETDLOCK_RSP  /* MR_Defs.h    */

/*--------------------------------------------------------------------------*/
/* Virtual Controller Group (VCG) Packets                                   */
/*--------------------------------------------------------------------------*/

/*
** GENERIC VCG REQUEST PACKET FOR REQUESTS THAT ONLY REQUIRE A SERIAL
** NUMBER TO BE SENT.
*/
typedef struct _PI_VCG_SN_REQ
{
    UINT32      serialNumber;   /* Could be a VCGID or a CONTROLLER SN  */
} PI_VCG_SN_REQ;

/* --- Validation Request --- */
typedef struct _PI_VCG_VALIDATION_REQ
{
    UINT16      validationFlags;        /* Validation Flags  */
    UINT8       rsvd2[2];       /* Reserved */
} PI_VCG_VALIDATION_REQ;

/* --- Prepare Slave Controller Request --- */
typedef struct _PI_VCG_PREPARE_SLAVE_REQ
{
    UINT32      vcgID;          /* Virtual Controller Group ID  */
    UINT32      controllerSN;   /* controller serial number     */
    UINT32      ipEthernetAddress;      /* Ethernet IP address          */
    UINT8       communicationsKey[16];  /* Communications signature Key */
} PI_VCG_PREPARE_SLAVE_REQ;

/* --- Prepare Slave Controller Response --- */
/* No additional response data beyond the status in the header. */

/* --- Add Slave Controller Request --- */
typedef struct _PI_VCG_ADD_SLAVE_REQ
{
    UINT32      serialNumber;
    UINT32      ipAddress;
} PI_VCG_ADD_SLAVE_REQ;

/* --- Add Slave Controller Response --- */
/* No additional response data beyond the status in the header. */

/* --- Virtual Controller Group Ping Request --- */
#define PI_VCG_PING_REQ                 PI_VCG_SN_REQ

/* --- Virtual Controller Group Ping Response --- */
/* No additional response data beyond the status in the header. */

/* --- Virtual Controller Group Information Request --- */
/* No input parameters are required */

/* --- Virtual Controller Group Information Response --- */
typedef struct _PI_VCG_CTRL_INFO
{
    UINT32      serialNumber;   /* Serial Number for this Controller */
    UINT32      ipAddress;      /* IP for this controller           */
    UINT32      failureState;   /* State from QM_FAILURE_DATA       */
    UINT8       rsvd[3];        /* RESERVED                         */
    UINT8       amIMaster;      /* 0=false, 1=TRUE                  */
} PI_VCG_CTRL_INFO;

typedef struct _PI_VCG_INFO_RSP
{
    UINT32      vcgID;          /* Virtual Contr Group ID           */
    UINT32      vcgIPAddress;   /* VCGroup IP Address               */
    UINT16      vcgMaxControllers;      /* Max # controllers                */
    UINT16      vcgCurrentControllers;  /* Current # of controllers         */
    ZeroArray(PI_VCG_CTRL_INFO, controllers); /* Information for each controller  */
} PI_VCG_INFO_RSP;

/* --- VCG Inactivate Controller Request --- */
#define PI_VCG_INACTIVATE_CONTROLLER_REQ    PI_VCG_SN_REQ

/* --- VCG Unfail Controller Response --- */
/* No additional response data beyond the status in the header. */

/* --- VCG Activate Controller Request --- */
#define PI_VCG_ACTIVATE_CONTROLLER_REQ  PI_VCG_SN_REQ

/* --- VCG Fail Controller Response --- */
/* No additional response data beyond the status in the header. */

/* --- VCG Set Cache Request --- */
#define PI_VCG_SET_CACHE_REQ            MRSETCACHE_REQ  /* MR_Defs.h    */

/* --- VCG Set Cache Response --- */
#define PI_VCG_SET_CACHE_RSP            MRSETCACHE_RSP  /* MR_Defs.h    */

/* -- VCG Get Mirror Partner List Request --- */
#define PI_VCG_GET_MP_LIST_REQ          MRGETMPLIST_REQ /* MR_Defs.h    */

/* -- VCG Get Mirror Partner List Response --- */
#define PI_VCG_GET_MP_LIST_RSP          MRGETMPLIST_RSP /* MR_Defs.h    */

/* --- VCG Apply License Request --- */
typedef struct _PI_VCG_APPLY_LICENSE_REQ
{
    UINT32      vcgID;          /* Virtual Controller Group ID      */
    UINT8       rsvd1[128];     /* RESERVED                         */
    UINT8       rsvd2[2];       /* RESERVED                         */
    UINT16      vcgMaxNumControllers;   /* Max controllers availble to VCG  */
    ZeroArray(UINT32, controllers); /* Controllers licensed to VCG      */
} PI_VCG_APPLY_LICENSE_REQ;

/* --- VCG Apply License Response --- */
/* No additional response data beyond the status in the header. */

/* --- VCG Unfail Controller Request --- */
#define PI_VCG_UNFAIL_CONTROLLER_REQ    PI_VCG_SN_REQ

/* --- VCG Unfail Controller Response --- */
/* No additional response data beyond the status in the header. */

/* --- VCG Fail Controller Request --- */
#define PI_VCG_FAIL_CONTROLLER_REQ      PI_VCG_SN_REQ

/* --- VCG Fail Controller Response --- */
/* No additional response data beyond the status in the header. */

/* --- VCG Remove Controller Request --- */
#define PI_VCG_REMOVE_CONTROLLER_REQ    PI_VCG_SN_REQ

/* --- VCG Remove Controller Response --- */
/* No additional response data beyond the status in the header. */

/* --- VCG Shutdown Request --- */
/* No additional response data beyond the status in the header. */

/* --- VCG Shutdown Response --- */
/* No additional response data beyond the status in the header. */

/* --- GET License config Request --- */
/* No input parameters are required */

/* --- License config information - Controller config information */
typedef struct _PI_GET_CPUCOUNT_RSP
{
    UINT8       cpuCount;       /**< the CPUCount: num licensed  */
    UINT8       reserved[19];   /**< reserved                    */
} PI_GET_CPUCOUNT_RSP;


#if defined(MODEL_7000) || defined(MODEL_4700)
/*--------------------------------------------------------------------------*/
/* ISE disk bay status/information/configuration/environmentals Packets     */
/*--------------------------------------------------------------------------*/

/* --- ISE Bay Environments Item Version 0/1. --- */
// Versions 0 and 1 of the ISE status structures. Version 0 and 1 are the same.
// These are unfortunately defined in IseStatusResponse.h.
// struct ise_controller;                       // LOCKED, must never change.
// struct ise_datapac;                          // LOCKED, must never change.
// struct ise_powersupply;                      // LOCKED, must never change.
// struct ise_battery;                          // LOCKED, must never change.
// typedef struct ise_info ise_info;            // LOCKED, must never change.
// typedef struct ise_stats ise_stats;          // LOCKED, must never change.
#define ISE_BAY_STATUS_ITEM_0   ise_stats       // LOCKED, must never change.

/* --- ISE Bay Environments Item Version 2. --- */
// Version 2 of the ISE status structures. There is a position added to MRC,
// Battery, PowerSupply, and DataPac. MRC firmware version extended to
// 28 characters, from 16 -- there are no other changes from version 0.

/*
 * The statistics data structure for an ISE Controller.
 */
typedef struct PI_ISE_CONTROLLER_VERSION_2      // LOCKED, must never change.
{
    char        controller_model[16];   /* Model.               'STFRU13CC' */
    char        controller_serial_number[12];   /* Serial number.       '2BBC0247' */
    char        controller_part_number[16];     /* Part number.         'STFRU13CC' */
    char        controller_hw_version[4];       /* Revision.            'A' */
//  char                manufacture_date[]; Lets not do this. Long, semi-unknown size.
//  enum xwstype__CtrlrPortType1 fc_USCOREport_USCOREtype; Lets not do this.
    INT64       controller_wwn; /* fc_port_id.  WWN of controller. */

/* Controller configuration. */
    IP_ADDRESS  ip;             /* IP of controller. */
    IP_ADDRESS  gateway;        /* IP of gateway. */
    IP_ADDRESS  subnet_mask;    /* Subnet mask. */
    UINT8       controller_fc_port_speed_setting;       /* Speed of fc for this controller. */
    UINT8       controller_beacon;      /* Is beacon light on? */

/* Controller Status. */
    UINT8       controller_rank;        /* rank 1 or 2, primary/secondary. */
    UINT8       controller_status;      /* Status of MRC (controller). */
    /* I propose a bit field for the array of enums. */
    UINT64      controller_status_details;
    char        controller_fw_version[28];      /* firmware version.    'V1.0 (RC1.7)' */
    UINT8       controller_fc_port_status;      /* Status of fibre channel port. */
    UINT8       controller_fc_port_speed;       /* Speed of fibre channel. */
    UINT8       controller_ethernet_link_up;    /* True if ethernet link active. */
    char        controller_mac_address[18];     /* Ethernet mac address. */

/* Controller Stats1. */
// Does not seem to return anything useful.

/* Controller Environmental. */
// eeproms is empty.
    INT16       controller_temperature; /* Convert from float to signed short. */

    // This is put at the end of the structure, so that conversion between ASGSES and
    // PI version 1 and 2 is much easier.
    UINT8       controller_position;    /* 1 or 2. */
} __attribute__ ((packed)) PI_ISE_CONTROLLER_VERSION_2;    // LOCKED, must never change.

/*
 * The statistics data structure for an ISE DataPac.
 */
typedef struct PI_ISE_DATAPAC_VERSION_2 // LOCKED, must never change.
{
/* DataPac Configuration. */
    UINT8       datapac_beacon; /* Beacon light on/off. */

/* DataPac Information. */
    UINT8       datapac_type;   /* Type of pack, 1=unknown, 2= reserved,
                                 * 3=10pack, 4=20pack. */
    char        datapac_serial_number[12];      /* Serial number. */
    char        datapac_model[16];      /* Model. */
    char        datapac_part_number[16];        /* Part number. */
    INT64       datapac_spare_level;    /* Spare level. */
//  time_t      datapac_manufacture_date;

/* DataPac Status. */
    UINT8       datapac_status; /* Status of datapac. */
    /* I propose a bit field for the array of enums. */
    UINT64      datapac_status_details;
    INT64       datapac_capacity;
    char        datapac_fw_version[16]; /* firmware version. */

/* DataPac Environmental. */
    INT16       datapac_temperature;    /* Convert from float to signed short. */
    UINT8       datapac_health; /* Percent. */

    // This is put at the end of the structure, so that conversion between ASGSES and
    // PI version 1 and 2 is much easier.
    UINT8       datapac_position;       /* 1 or 2. */
} __attribute__ ((packed)) PI_ISE_DATAPAC_VERSION_2;       // LOCKED, must never change.

/*
 * The statistics data structure for an ISE PowerSypply.
 */
typedef struct PI_ISE_POWERSUPPLY_VERSION_2     // LOCKED, must never change.
{
/* PowerSupply Configuration. */
    UINT8       powersupply_beacon;     /* Beacon light on/off. */

/* PowerSupply Information. */
    char        powersupply_model[16];  /* Model. */
    char        powersupply_serial_number[12];  /* Serial number. */
    char        powersupply_part_number[16];    /* Part number. */
//  time_t      powersupply_manufacture_date;

/* PowerSupply Status. */
    UINT8       powersupply_status;     /* Status of powersupply. */
    /* I propose a bit field for the array of enums. */
    UINT64      powersupply_status_details;

/* PowerSupply Environmental. */
    UINT8       powersupply_fan1_status;        /* Status of powersupply fan. */
    INT64       powersupply_fan1_speed; /* Speed of powersupply fan. */
    UINT8       powersupply_fan2_status;        /* Status of powersupply fan. */
    INT64       powersupply_fan2_speed; /* Speed of powersupply fan. */
    INT16       powersupply_temperature;        /* Convert from float to signed short. */

    // This is put at the end of the structure, so that conversion between ASGSES and
    // PI version 1 and 2 is much easier.
    UINT8       powersupply_position;   /* 1 or 2. */
} __attribute__ ((packed)) PI_ISE_POWERSUPPLY_VERSION_2;   // LOCKED, must never change.

/*
 * The statistics data structure for an ISE Battery.
 */
typedef struct PI_ISE_BATTERY_VERSION_2 // LOCKED, must never change.
{
/* Battery Configuration. */
    UINT8       battery_beacon; /* Beacon light on/off. */

/* Battery Information. */
    char        battery_model[16];      /* Model. */
    char        battery_serial_number[12];      /* Serial number. */
    char        battery_part_number[16];        /* Part number. */
//  time_t      battery_manufacture_date;
    char        battery_type[16];       /* Type of battery. (Lead Acid) */

/* Battery Status. */
    UINT8       battery_status; /* Status of battery. */
    /* I propose a bit field for the array of enums. */
    UINT64      battery_status_details;
// bool ups_USCOREmode;
    INT64       battery_remaining_charge;       /* Remaining charge. */
    INT64       battery_max_charge;     /* Maximum charge. */
    INT64       battery_max_charge_capacity;    /* Maximum charge capacity. */
    INT64       battery_min_holdup_time;        /* Min holdup time. */
// xwstype__BatteryCalibration1 *calibration;

/* Battery Environmental. */
    UINT8       battery_charger_state;  /* Status of battery charger. */
    UINT8       battery_charger_state_details;  /* Detailed Status of battery charger. */

    // This is put at the end of the structure, so that conversion between ASGSES and
    // PI version 1 and 2 is much easier.
    UINT8       battery_position;       /* 1 or 2. */
} __attribute__ ((packed)) PI_ISE_BATTERY_VERSION_2;       // LOCKED, must never change.

/*
 * The statistics data structure for an ISE.
 */

typedef struct PI_ISE_INFO_VERSION_2    // LOCKED, must never change.
{
    UINT8       Protocol_Version_Level; /* Which form of this data structure. */
    // which_tcp_connections has a bit set for each MRC that we can connect to
    // ethernet and tcp/ip. If a position is disconnected, check MRC and cabling.
    // This is created on the platform controller by program ASGSES.
    UINT8       which_tcp_connections;  /* bit field, set bit(s) 0 and/or 1 if present. */
    // which_controllers has a bit set for each MRC that has information available
    // via IWS on the ISE (brick). This is ISE generated. Same for rest below.
    // Note: The array structures for each will contain zeros, or whatever the
    // last state set -- if the device is no longer up via following bit fields.
    INT8        which_controllers[2];   /* index of two controllers 1/2. */
    INT8        which_datapacs[2];      /* index of two datapacs 1/2. */
    INT8        which_powersupplies[2]; /* index of two powersupplies 1/2. */
    INT8        which_batteries[2];     /* index of two batteries 1/2. */
    IP_ADDRESS  ip1;            /* IP of first controllers in chassis. */
    IP_ADDRESS  ip2;            /* IP of second controllers in chassis. */
    INT64       iws_ise_id;     /* The identifier of the chassis. */

/* Chassis data. */
    INT64       chassis_wwn;    /* identifier (wwn)     '20000014C3671E50' */
    char        chassis_serial_number[12];      /* Serial number.       '1BC1004a'  */
    char        chassis_model[16];      /* model.               'ST000000FC      ' */
    char        chassis_part_number[16];        /* Part number.         ''  --i.e. empty. */
    char        chassis_vendor[8];      /* Vendor               'SEAGATE ' */
    char        chassis_manufacturer[8];        /* Manufacturer         'Seagate' */
    char        chassis_product_version[4];     /* Product_version.     'A' */
    INT64       spare_level;    /* Spare level.         20 */

/* Chassis Configuration data that may change follows. */
    UINT8       chassis_beacon; /* Is beacon light on? */

/* Chassis status data. */
    UINT8       chassis_status; /* enum: Operational=1, Warning=2, Critical=3,
                                 * Uninitialized=4, Changing=5, NON-operational=6 */
    /* I propose a bit field for the array of enums. 32 implies a 64 bit field. */
    UINT64      chassis_status_details; /* enum: NONE=1, COMPONENT_NOT_PRESENT = 2,
                                         * COMPONENT_OFFLINE = 3, VITAL_PRODUCT_DATA_CORRUPT = 4, VITAL_PRODUCT_DATA_UNKNOWN_TYPE = 5,
                                         * VITAL_PRODUCT_DATA_UNKNOWN_VERSION = 6, VITAL_PRODUCT_DATA_BAD_STATE = 7,
                                         * VITAL_PRODUCT_DATA_LOADED = 8, VITAL_PRODUCT_DATA_PERSISTENT_FAULT = 9,
                                         * MAINTENANCE_MODE = 10, COMPONENT_DEGRADED = 11, TEMPERATURE_OUT_OF_RANGE = 12,
                                         * UNINITIALIZED_NOT_READY = 13, INITIALIZED_NOT_OPERATIONAL = 14,
                                         * DISPLAY_CARD_NOT_PRESENT = 15, DISPLAY_CARD_FAILURE = 16, SYSTEM_METADATA_ERROR = 17,
                                         * CACHE_MEMORY_ERROR = 18, SYSTEM_METADATA_CONVERSION_ERROR = 19, DEGRAGED_BATTERY = 20,
                                         * ALL_DATAPACS_DEGRADED = 21, SYSTEM_METADATA_MISMATCH_CODE_0 = 22,
                                         * SYSTEM_METADATA_MISMATCH_CODE_1 = 23, SYSTEM_METADATA_MISMATCH_CODE_2 = 24,
                                         * SYSTEM_METADATA_MISMATCH_CODE_3 = 25, SYSTEM_METADATA_MISMATCH_CODE_4 = 26,
                                         * MRC_INOPERATIVE = 27, CACHE_INCONSISTENCY = 28, DIAGNOSTIC_MODE = 29,
                                         * DATAPAC_CONFIGURATION_CONFLICT = 30, MRC_CONFIGURATION_CONFLICT = 31,
                                         * DATAPAC_CONFIGURATION_CONFLICT_RECOVERABLE = 32 */

/* Chassis stat1 data. */
    time_t      chassis_uptime;
    time_t      chassis_current_date_time;

// 2008-02-05 -- the ISE is not returning performance data -- NULL pointer.
// 2008-05-31 -- the ISe is NOW returning performance data.
    UINT8       chassis_performance_valid;      /* Indicates performance information is valid. */
    INT64       chassis_total_iops;
    INT64       chassis_read_iops;
    INT64       chassis_write_iops;
    INT64       chassis_total_kbps;
    INT64       chassis_read_kbps;
    INT64       chassis_write_kbps;
    INT64       chassis_read_latency;
    INT64       chassis_write_latency;
    INT64       chassis_queue_depth;
    INT64       chassis_read_percent;
    INT64       chassis_avg_bytes_transferred;

/* Chassis environmental data. */
    INT16       chassis_temperature_sensor;     /* Convert from float to signed short. */

/* Statistics for each controller. */
    struct PI_ISE_CONTROLLER_VERSION_2 ctrlr[2];

/* Statistics for each DataPac. */
    struct PI_ISE_DATAPAC_VERSION_2 datapac[2];

/* Statistics for each PowerSypply. */
    struct PI_ISE_POWERSUPPLY_VERSION_2 powersupply[2];

/* Statistics for each Battery. */
    struct PI_ISE_BATTERY_VERSION_2 battery[2];
} __attribute__ ((packed)) PI_ISE_INFO_VERSION_2;          // LOCKED, must never change.

/*
* The max array we are caching.
*/
typedef struct ISE_BAY_STATUS_ITEM_2    // LOCKED, must never change.
{
    UINT16      bayid;          /* ise Bay id */
    UINT16      rsvd;           /* reserved   */
    PI_ISE_INFO_VERSION_2 ise_bay_info; /* ise_info   */
} __attribute__ ((packed)) ISE_BAY_STATUS_ITEM_2;          // LOCKED, must never change.


/*---- ISE Bay environments response packet formats. ----- */
// Versions 0 and 1 of PI response packet format.
typedef struct _PI_ISE_BAY_STATUS_RSP   // LOCKED, must never change.
{
    UINT16      baycount;       /* Number of ISE bays. */
    UINT16      resvd;
// NOTE: structure ise_stats exactly matches structure ISE_BAY_STATUS_ITEM_0.
// This is because moves the array iseBayStatus into a local structure of type
// ise_stats in SPAL/PI_ISEGetStatusResponse.cpp, in the routine
// PI_ISEGetStatusResponse::process. As such, it MUST never change -- both
// the array ise_stats, and ISE_BAY_STATUS_ITEM_0 (and sub references) are
// never able to be changed.
    ZeroArray(ISE_BAY_STATUS_ITEM_0, iseBayStatus); /* ISE bay Status. */
} __attribute__ ((packed)) PI_ISE_BAY_STATUS_RSP;   // LOCKED, must never change.

// Version 2 of PI response packet format.
typedef struct _PI_ISE_BAY_STATUS_2_RSP // LOCKED, must never change.
{
    UINT16      baycount;       /* Number of ISE bays. */
    UINT16      resvd;
    ZeroArray(ISE_BAY_STATUS_ITEM_2, iseBayStatus); /* ISE bay Status. */
} __attribute__ ((packed)) PI_ISE_BAY_STATUS_2_RSP;        // LOCKED, must never change.

#endif /* MODEL_7000 || MODEL_4700 */


#if defined(MODEL_7000) || defined(MODEL_4700)
/*--------------------------------------------------------------------------*/
/* ISE disk bay light (beacon) turning on/off Packets                       */
/*--------------------------------------------------------------------------*/

/*---- Beacon ISE components ----- */
enum PI_BEACON_ISE_COMPONENTS
{
    PI_BEACON_ISE_CHASSIS = 0,  /* Chassis beacon light */
    PI_BEACON_ISE_MRC = 1,      /* MRC beacon light */
    PI_BEACON_ISE_PS = 2,       /* Powersupply  beacon light */
    PI_BEACON_ISE_BAT = 3,      /* Battery beacon light */
    PI_BEACON_ISE_DP = 4,       /* DataPac beacon light */
    PI_BEACON_ISE_SFP = 5,      /* SFP beacon light */
    PI_BEACON_ISE_CAP = 6,      /* CAP beacon light */
    PI_BEACON_ISE_BEZEL = 7,    /* BEZEL beacon light */
};

/*---- Beacon ISE Component Request ----- */
typedef struct _PI_BEACON_ISE_COMPONENT_REQ
{
    UINT16      bayid;          /* ise bay id  */
    UINT16      component;      /* component of ise */
    UINT16      subcomponent;   /* sub component i.e. mrc 1 or ps 2, etc. */
    UINT8       light_on_off;   /* Should beacon light be on or off. */
    UINT8       resvd;          /* for the byte boundary */
} PI_BEACON_ISE_COMPONENT_REQ;

#endif /* MODEL_7000 || MODEL_4700 */

/*--------------------------------------------------------------------------*/
/* Disk Bay Packets                                                         */
/*--------------------------------------------------------------------------*/

/* --- Disk Bay Count Request --- */
/* No input parameters are required */

/* --- Disk Bay Count Response --- */
#define PI_DISK_BAY_COUNT_RSP           PI_COUNT_RSP

/* --- Disk Bay List Request --- */
/* No input parameters are required */

/* --- Disk Bay List Response --- */
#define PI_DISK_BAY_LIST_RSP            PI_LIST_RSP

/* --- Disk Bay Info Request --- */
#define PI_DISK_BAY_INFO_REQ            MRGETEINFO_REQ  /* MR_Defs.h    */

/* --- Disk Bay Info Response --- */
#define PI_DISK_BAY_INFO_RSP            MRGETEINFO_RSP  /* MR_Defs.h    */

/* --- Disk Bay Set Name Response --- */
/* There is no response data packet for this request. */

/* --- Disk Bay Delete Request --- */
#define PI_DISK_BAY_DELETE_REQ          MRDELETEDEVICE_REQ      /* MR_Defs.h    */

/* --- Disk Bay Delete Response --- */
#define PI_DISK_BAY_DELETE_RSP          MRDELETEDEVICE_RSP      /* MR_Defs.h    */

/* --- Disk Bays Request --- */
/* No input parameters are required */

/* --- Disk Bays Response --- */
typedef struct _PI_DISK_BAYS_RSP
{
    UINT16      count;          /* Number of PDisks */
    UINT8       rsvd[2];        /* RESERVED */
    ZeroArray(PI_DISK_BAY_INFO_RSP, bayInfo); /* Disk Bay Information */
} PI_DISK_BAYS_RSP;

/* --- Disk Bay Alarm Control Request --- */
typedef struct _PI_DISK_BAY_ALARM_CONTROL_REQ
{
    UINT16      id;
    UINT8       setting;
    UINT8       rsvd;
} PI_DISK_BAY_ALARM_CONTROL_REQ;

/* --- Disk Bay Alarm Control Response --- */
/* There is no response data packet for this request. */

/*--------------------------------------------------------------------------*/
/* Environmental Data Packets                                               */
/*--------------------------------------------------------------------------*/

/* --- Environmental Data - Disk Bay Request --- */
#define PI_DISK_BAY_ENVIRO_REQ          MR_DEVID_REQ    /* MR_Defs.h    */

#if 0
/*
** -- OLD --  SES device data structure.  This struct is returned when
** X1PktGetRmcCompatibility() < 10.  It does NOT include the devType
** field which is part of the current SES_DEVICE struct (defined in
** SES_Struct.h)
*/
typedef struct SES_DEVICE_OLD
{
    struct SES_DEVICE *NextSES; /* Next SES device          */
    UINT64      WWN;            /* WWN of the SES device    */
    UINT32      SupportedPages; /* Bit significant support  */
    UINT32      FCID;           /* Fibre channel ID         */
    UINT32      Generation;     /* Generation code          */
    UINT8       Channel;        /* Fibre channel adapter    */
    UINT8       devStat;        /* Device status            */
    UINT16      PID;            /* PID of the SES device    */
    UINT16      LUN;            /* Logical unit number      */
    UINT16      TotalSlots;     /* Total element slots      */
    PSES_PAGE_02 OldPage2;      /* Previous page 2 reading  */
    UINT8       Map[SES_ET_MAX_VAL + 1];        /* Map of type area       */
    UINT8       Slots[SES_ET_MAX_VAL + 1];      /* Number of slots      */
    UINT8       pd_rev[4];      /* revision                 */
} SES_DEVICE_OLD, *PSES_DEVICE_OLD;
#endif /* 0 */

typedef struct SES_DEVICE_OLD_2
{

    struct SES_DEVICE *NextSES; /* Next SES device          */
    UINT64      WWN;            /* WWN of the SES device    */
    UINT32      SupportedPages; /* Bit significant support  */
    UINT32      FCID;           /* Fibre channel ID         */
    UINT32      Generation;     /* Generation code          */
    UINT8       Channel;        /* Fibre channel adapter    */
    UINT8       devStat;        /* Device status            */
    UINT16      PID;            /* PID of the SES device    */
    UINT16      LUN;            /* Logical unit number      */
    UINT16      TotalSlots;     /* Total element slots      */
    PSES_PAGE_02 OldPage2;      /* Previous page 2 reading  */
    UINT8       Map[SES_ET_MAX_VAL + 1];        /* Map of type area       */
    UINT8       Slots[SES_ET_MAX_VAL + 1];      /* Number of slots      */
    UINT8       pd_rev[4];      /* revision                 */
    UINT8       devType;        /* Device type              */
} SES_DEVICE_OLD_2;

/* --- Environmental Data - Disk Bay Response --- */

/*
** NOTE: The data structure below contains 2 variable length structures.
** This means that page2 can not be referenced using standard structure
** references.  The start of page2 must be calculated using mapLength.
**
** The complete PI_DISK_BAY_ENVIRO_RSP packet contains either the
** SES_DEVICE_OLD or SES_DEVICE struct before PI_DISK_BAY_ENVIRO_PART2_RSP.
** Refer to the code in PI_DiskBaySESEnviro().
*/
typedef struct _PI_DISK_BAY_ENVIRO_PART2_RSP
{
    UINT16      mapLength;      /* Length of device map to follow   */
    UINT16      page2Length;    /* Length of page 2 data to follow  */
#ifdef _WIN32
    ZeroArray(UINT8, devMapPage2); /* VARIABLE LENGTH map - see code     */
#else
    ZeroArray(SES_WWN_TO_SES, devMap); /* VARIABLE LENGTH map - see code     */
    SES_PAGE_02 page2;          /* Control elements: power, cooling, temp, etc. */
    /* NOTE: page2 is variable length.  See ses.h   */
#endif
} PI_DISK_BAY_ENVIRO_PART2_RSP;

/*
** The structures below are not used in the actual code.  They are here
** to show more clearly what the complete Disk Bay Environmental Response
** packet looks like.
*/
#if 0
typedef struct _PI_DISK_BAY_ENVIRO_OLD_RSP
{
    SES_DEVICE_OLD ses1;        /* Does NOT contain device type */
    PI_DISK_BAY_ENVIRO_PART2_RSP ses2;  /* The rest of the env info     */
} PI_DISK_BAY_ENVIRO_OLD_RSP;

typedef struct _PI_DISK_BAY_ENVIRO_RSP
{
    SES_DEVICE  ses1;           /* Contain device type      */
    PI_DISK_BAY_ENVIRO_PART2_RSP ses2;  /* The rest of the env info */
} PI_DISK_BAY_ENVIRO_RSP;
#endif

/* --- Environmental Data - Controller and Disk Bay Request --- */
/* No input parameters are required */

/* --- Configure Controller Request --- */
typedef struct _PI_VCG_CONFIGURE_REQ
{
    UINT32      IPAddr;
    UINT32      subnetMask;
    UINT32      gateway;
    UINT32      dscId;
    UINT32      nodeId;
    UINT8       replacementFlag;
    UINT8       rsvd[3];
} PI_VCG_CONFIGURE_REQ;

/* ---  Configure Controller Response  --- */
/* No parameters, just the status response */

/*--------------------------------------------------------------------------*/
/* Snapshot Data Packets                                                    */
/*--------------------------------------------------------------------------*/
typedef struct _PI_SNAPSHOT_REQ
{
    UINT32      index;          /* Dir entry index       */
    UINT32      type;           /* Type of snapshot      */
    UINT32      flags;          /* flags to use          */
    UINT32      status;         /* status to set         */
    UINT32      descLen;        /* length of desc. field */
    ZeroArray(char, description);
} PI_SNAPSHOT_REQ;

typedef struct _PI_SNAPSHOTDIR_RSP
{
    UINT32      dirLen;         /* length of directory  */
    ZeroArray(char, dir);       /* the directory        */
} PI_SNAPSHOTDIR_RSP;

/*--------------------------------------------------------------------------*/
/* Administrate Packets                                                   */
/*--------------------------------------------------------------------------*/

/* --- Firmware Version Info Request --- */
typedef struct _PI_FW_VERSION_REQ
{
    UINT16      fwType;         /* See def.h for MRP function code */
} PI_FW_VERSION_REQ;

/* --- Firmware Version Info Response --- */
#define PI_FW_VERSION_RSP               MR_FW_HEADER_RSP        /* MR_Defs.h    */

/* --- Firmware System Release Level Request --- */
/* No input parameters are required */

/* --- Firmware System Release Level Response --- */
typedef struct _PI_FW_SYS_REL_LEVEL_RSP
{
    UINT32      systemRelease;  /* Major (high UINT16) and minor (low UINT16) version */
    char        tag[8];         /* ASCII tag field (i.e. M880)              */
} PI_FW_SYS_REL_LEVEL_RSP;

/* --- Change IP Request --- */
/* --- Get IP Response --- */
typedef struct
{
    UINT32      serialNum;      /* Controller Serial Number */
    UINT32      ipAddress;      /* IP Address               */
    UINT32      subnetMask;     /* Subnet Mask Address      */
    UINT32      gatewayAddress; /* Gateway Address          */
} PI_SET_IP_REQ, PI_GET_IP_RSP;

/* --- Change IP Response --- */
#define PI_SET_IP_RSP                   MR_GENERIC_RSP  /* MR_Defs.h    */

/* --- Firmware download to drive or bay Request --- */
typedef struct
{
    UINT64      wwn;            /* Device WWN     */
    UINT32      lun;            /* Device lun     */
    UINT32      rsvd;
} PI_WWN_LUN_SELECTOR;

typedef struct _PI_WRITE_BUFFER_MODE5_REQ
{
    UINT32      count;          /* Number of wwn/Lun combo's that follow  */
    ZeroArray(PI_WWN_LUN_SELECTOR, wwnLun); /* List of wwn/lun combinations */
} PI_WRITE_BUFFER_MODE5_REQ;
/* Note: The firmware itself immediately follows this structure. */

/* --- Set Controller time Request --- */
typedef struct _PI_SETTIME_REQ
{
    UINT64      sysTime;        /* system seconds                       */
} PI_SETTIME_REQ, PI_ADMIN_GETTIME_RSP;

/* --- Led Control Request --- */
#define PI_LED_ATTENTION        0

typedef struct _PI_LEDCNTL_REQ
{
    UINT8       led;            /* which led to set                     */
    UINT8       value;          /* value to set 0 /1                    */
    UINT8       rsvd[2];
} PI_LEDCNTL_REQ;

/* --- VCG FWUpdate Phase request --- */
typedef struct _PI_ROLLING_UPDATE_PHASE_REQ
{
    UINT32      controllerSN;   /* Controller Serial Number     */
    UINT32      phase;          /* FW Update phase to execute   */
} PI_ROLLING_UPDATE_PHASE_REQ;

/* --- Multi-Part Xfer request --- */
#define MAX_SEND_SIZE (SIZEOF_TXBUFFER - sizeof(DDR_FID_HEADER))
#define TX_BUFFER_DATA_START (txBuffer + sizeof(DDR_FID_HEADER))

typedef struct
{
    UINT8       subCmdCode;     /* sub-function         */
    UINT8       partX;          /* 1 to N               */
    UINT8       ofN;            /* N (33 Max)           */
    UINT8       flags;          /* defined below        */
    UINT32      p1;             /* write:               */
                                /*  -> opt'l parm       */
                                /*  <- rc               */
                                /* read:                */
                                /*  -> opt'l parm       */
                                /*  <- rc               */
    UINT32      p2;             /* write:               */
                                /*  -> opt'l parm       */
                                /*  <- unused           */
                                /* read:                */
                                /*  -> opt'l parm       */
                                /*  <- unused           */
    ZeroArray(UINT8, data);     /* data, depending upon */
                                /*  the xfer direction  */
} X1_MULTI_PART_XFER_REQ, X1_MULTI_PART_XFER_RSP;

/* subCmdCode's */
#define MPX_FW_SCMD     1
#define MPX_FILEIO_SCMD 2
#define MPX_MEMIO_SCMD  3

/* fw types, passed as p1 with MPX_FW_SCMD */
#define MPX_FW_TYPE_CONTROLLER  0

/* skip '1' for historical/compatibility reasons */
#define MPX_FW_TYPE_PDISK       2
#define MPX_FW_TYPE_BAY         3

/* flags: */
#define MPX_WRITE       0x01    /* bit 0: if set data is sent to the CCB,      */
                             /* otherwise data is being sent to the VCGMgmt */
                             /* layer.  */
#define MPX_PROC_MASK   0x06    /* This is a 2 bit field used with MEMIO:      */
                             /*    0 => CCB   */
                             /*    1 => FE    */
                             /*    2 => BE    */
                             /*    3 => error */
#define MPX_WRITE_NO_HDR 0x08   /* No header on file write */
#define MPX_UNUSED_BIT3 0x10    /* etc... */

/**
******************************************************************************
** Logging Packets
******************************************************************************
**/

/* Logging Mode bits    */
#define MODE_BINARY_MESSAGE         0x0001
#define MODE_EXTENDED_MESSAGE       0x0002
#define MODE_USE_SEQUENCE           0x0004
#define MODE_DEBUG_LOGS             0x0008
#define MODE_MASTER_SEQUENCE_LOGS   0x0010
#define MODE_START_OF_LOGS          0x0020
#define MODE_TEST_LOGS              0x0040
#define MODE_END_OF_LOGS            0x8000

/* --- Log Information Request --- */
typedef struct _PI_ASCII_LOG_EVENT
{
    UINT16      length;         /* Length of data to follow     */
    UINT16      eventCode;      /* See def.h                    */
    unsigned long masterSequenceNumber; /* Log event sequence number    */
    unsigned long sequenceNumber;   /* Log event sequence number    */
    UINT32      statusWord;         /* Log event status word (flags) */
    char        timeAndDate[25];    /* Time and Date String         */
    char        messageType[10];    /* Message type string          */
    ZeroArray(char, messageDescr);  /* Message Description string   */
} PI_ASCII_LOG_EVENT;

typedef struct _PI_BINARY_LOG_EVENT
{
    UINT16      length;         /* Length of data to follow     */
    UINT16      eventType;      /* LOG_TYPE_ in logging.h       */
    ZeroArray(UINT8, message);  /* Binary log message           */
} PI_BINARY_LOG_EVENT;

typedef struct _PI_LOG_EVENT
{
    UINT32      length;         /* Length of data to follow     */
    PI_ASCII_LOG_EVENT ascii;
    PI_BINARY_LOG_EVENT binary;
} PI_LOG_EVENT;

typedef struct _PI_LOG_INFO_REQ
{
    UINT16      eventCount;     /* Number of events to xfer     */
    UINT16      mode;           /* Transfer mode                */
    UINT32      sequenceNumber; /* Starting sequence number     */
} PI_LOG_INFO_REQ;

/* --- Log Information Response --- */
typedef struct _PI_LOG_INFO_MODE0_RSP   /* Combined ASCII + Binary Log  */
{
    UINT16      eventCount;     /* Number of events transferred */
    UINT16      mode;           /* Transfer mode                */
    UINT32      sequenceNumber; /* Starting sequence number     */
    UINT32      controllerSN;   /* Controller Serial Number     */
    UINT32      vcgID;          /* Virtual Controller ID        */
    UINT32      rsvd[4];        /* Reserved                     */
    ZeroArray(PI_LOG_EVENT, logEvent); /* Log event                    */
} PI_LOG_INFO_MODE0_RSP;

typedef struct _PI_LOG_INFO_MODE1_RSP   /* Binary Log Event             */
{
    UINT16      eventCount;     /* Number of events transferred */
    UINT16      mode;           /* Transfer mode                */
    UINT32      sequenceNumber; /* Starting sequence number     */
    UINT32      controllerSN;   /* Controller Serial Number     */
    UINT32      vcgID;          /* Virtual Controller ID        */
    UINT32      rsvd[4];        /* Reserved                     */
    ZeroArray(PI_BINARY_LOG_EVENT, logEvent); /* Log event                  */
} PI_LOG_INFO_MODE1_RSP;

typedef union _PI_LOG_INFO_RSP
{
    PI_LOG_INFO_MODE0_RSP logInfoMode0;
    PI_LOG_INFO_MODE1_RSP logInfoMode1;
} PI_LOG_INFO_RSP;

/* --- Log Text Message Request --- */
#define MAX_TEXT_MSG_LEN    124 /* Max length for text message log event */

typedef struct _PI_LOG_TEXT_MESSAGE_REQ
{
    UINT32      msgType;        /* See logging.h for types */
    UINT8       text[MAX_TEXT_MSG_LEN]; /* Text message */
} PI_LOG_TEXT_MESSAGE_REQ;

/* --- Log Text Message Response --- */
/* There is no response packet for this request */

/* --- Acknowledge Log Request --- */
typedef struct _PI_CUSTOMER_LOG_ACKNOWLEDGE_REQ
{
    UINT32      logCount;       /* Num of logs acknowledged */
    ZeroArray(UINT32, seqNos);  /* List of sequence numbers */
} PI_CUSTOMER_LOG_ACKNOWLEDGE_REQ;

/* --- Acknowledge Log Response --- */
/* There is no response packet for this request */

/*--------------------------------------------------------------------------*/
/* Statistics Packets                                                       */
/*--------------------------------------------------------------------------*/

/* --- Global Cache Statistics Request --- */
/* No input parameters are required */

/* --- Global Cache Statistics Response --- */
#define PI_STATS_GLOBAL_CACHE_RSP       MRGETCINFO_RSP  /* MR_Defs.h    */


/* --- Cache Device Statistics Request --- */
#define PI_STATS_CACHE_DEV_REQ          MRGETCDINFO_REQ /* MR_Defs.h    */

/* --- Cache Device Statistics Response --- */
#define PI_STATS_CACHE_DEV_RSP          MRGETCDINFO_RSP /* MR_Defs.h    */

/* --- Front End Processor Statistics Request --- */
/* No input parameters are required */

/* --- Front End Processor Statistics Response --- */
#define PI_STATS_FRONT_END_PROC_RSP     MRFEII_RSP      /* MR_Defs.h    */

/* --- Back End Processor Statistics Request --- */
/* No input parameters are required */

/* --- Back End Processor Statistics Response --- */
#define PI_STATS_BACK_END_PROC_RSP      MRBEII_RSP      /* MR_Defs.h    */

/* --- Front End or Back End Loop Statistics Request --- */
#define PORT_STATS_RLS  0x01    /* Show RLS ELS data in return data         */
typedef struct _PI_STATS_LOOPS_REQ
{
    UINT8       option;         /* Option                       */
    UINT8       rsvd[3];        /* RESERVED                     */
} PI_STATS_LOOPS_REQ;

typedef struct _PI_STATS_LOOP
{
    UINT16      length;         /* Length of data to follow     */
    UINT16      port;           /* Port identifier              */
    ZeroArray(MRPORT_RSP, stats); /* Start of loop stats    */
} PI_STATS_LOOP;

/* --- Front End or Back End Loop Statistics Response --- */
typedef struct _PI_STATS_LOOPS_RSP
{
    UINT16      count;          /* Number of valid ports    */
    UINT8       rsvd[2];        /* RESERVED                 */
    ZeroArray(PI_STATS_LOOP, stats); /* Start of loop stats */
} PI_STATS_LOOPS_RSP;

/* --- Front End PCI Statistics Request --- */
/* No input parameters are required */

/* --- Front End PCI Statistics Response --- */
#define PI_STATS_FRONT_END_PCI_RSP      MRBELINK_RSP    /* MR_Defs.h    */

/* --- Back End PCI Statistics Request --- */
/* No input parameters are required */

/* --- Back End PCI Statistics Response --- */
#define PI_STATS_BACK_END_PCI_RSP       MRBELINK_RSP    /* MR_Defs.h    */

/* --- Server Statistics Request --- */
#define PI_STATS_SERVER_REQ             MRGETSSTATS_REQ /* MR_Defs.h    */

/* --- Server Statistics Response --- */
#define PI_STATS_SERVER_RSP             MRGETSSTATS_RSP /* MR_Defs.h    */

/* --- Servers Statistics Request --- */
/* No input parameters are required */

/* --- Server Statistics Item --- */
typedef struct _STATS_SERVER_ITEM
{
    UINT16      sid;            /* Server ID                    */
    UINT8       rsvd[2];        /* RESERVED                     */
    MRGETSSTATS_RSP stats;      /* Server Statistics            */
} STATS_SERVER_ITEM;

/* --- Server Statistics Response --- */
typedef struct _PI_STATS_SERVERS_RSP
{
    UINT16      count;          /* Number of servers            */
    UINT8       rsvd[2];        /* RESERVED                     */
    ZeroArray(STATS_SERVER_ITEM, item); /* Server Statistics            */
} PI_STATS_SERVERS_RSP;

/* --- Environmental Stats --- */
/* No input parameters are required */

/* --- Environmental Stats Response --- */
#define PI_STATS_ENVIRONMENTAL_RSP      HWM_STATUS      /* see i2c_monitor.h */
#define PI_STATS_I2C_RSP                HWM_I2C_STATUS  /* see i2c_monitor.h */
#define PI_ENV_II_GET_DATA_RSP          env_device      /* see hw_mon.h     */

/* --- HAB Statistics Request --- */
#define PI_STATS_HAB_REQ                MR_DEVID_REQ    /* MR_Defs.h    */

/* --- HAB Statistics Response --- */
#define PI_STATS_HAB_RSP                MRGETHABSTATS_RSP       /* MR_Defs.h    */

/* --- Virtual Disk Statistics Request --- */
/* No input parameters are required */

/* --- Virtual Disk Statistics Response --- */
/*
** NOTE: The vdisk information array included in this response packet
**       contains only the core of PI_VDISK_INFO_RSP and does not
**       contain the raid listing that normally follows the core
**       structure.  This data is also the same as the core
**       MRGETVINFO_RSP structure without the raid listing.
**       The length field (vdiskInfo[x].header.len) contains the
**       correct length (sizeof(PI_VDISK_INFO_RSP)).
*/
typedef struct _PI_STATS_VDISK_RSP
{
    UINT16      count;          /* Number of VDisks                 */
    UINT8       rsvd[2];        /* RESERVED                         */
    ZeroArray(PI_VDISK_INFO_RSP, vdiskInfo); /* Virtual Disk Statistics     */
} PI_STATS_VDISK_RSP;

/* --- Virtual Disk Statistics Response - Version 2 --- */
typedef struct _PI_STATS_VDISK2_RSP
{
    UINT16      count;          /* Number of VDisks                 */
    UINT8       rsvd[2];        /* RESERVED                         */
    ZeroArray(PI_VDISK_INFO2_RSP, vdiskInfo); /* Virtual Disk Statistics     */
} PI_STATS_VDISK2_RSP;

/* --- Processor Statistics Request --- */
/* No input parameters are required */

/* --- Processor Statistics Response --- */
typedef struct _PI_STATS_PROC_RSP
{
    PI_STATS_FRONT_END_PROC_RSP fe;
    PI_STATS_BACK_END_PROC_RSP be;
} PI_STATS_PROC_RSP;

/* --- PCI Statistics Request --- */
/* No input parameters are required */

/* --- PCI Statistics Response --- */
typedef struct _PI_STATS_PCI_RSP
{
    PI_STATS_BACK_END_PCI_RSP fe;
    PI_STATS_FRONT_END_PCI_RSP be;
} PI_STATS_PCI_RSP;

/* --- Cache Device Statistics Request --- */
/* No input parameters are required */

/* --- Cache Device Statistics Response --- */
typedef struct _PI_STATS_CACHE_DEVICES_RSP
{
    UINT16      count;          /* Number of devices                */
    UINT8       rsvd[2];        /* RESERVED                         */
    ZeroArray(PI_STATS_CACHE_DEV_RSP, cacheDev); /* Device Cache Statistics */
} PI_STATS_CACHE_DEVICES_RSP;

/* Environmental data response packet                                       */
#define PI_ENV_DATA_RSP    monitorStats_t       /* See i2cstats.h   */

/*--------------------------------------------------------------------------*/
/* Debug Packets                                                            */
/*--------------------------------------------------------------------------*/

/* --- Memory Read/Write Request --- */
typedef struct _PI_DEBUG_MEM_RDWR_REQ
{
    void       *pAddr;          /* address to rd/wr   */
    UINT32      length;         /* length of data     */
    UINT16      processor;      /* processor to rd/wr */
    UINT16      mode;           /* read or write      */
    ZeroArray(UINT8, data);     /* write data         */
} PI_DEBUG_MEM_RDWR_REQ;

/* --- Memory Read/Write Response --- */
typedef struct _PI_DEBUG_MEM_RDWR_RSP
{
    UINT32      length;         /* actual read/write  */
    ZeroArray(UINT8, data);     /* read data          */
} PI_DEBUG_MEM_RDWR_RSP;

/*
 * Processor constants used in the 'processor' field above
 */
#define PROCESSOR_CCB   0
#define PROCESSOR_FE    1
#define PROCESSOR_BE    2

/*
 * Mode constants used in the 'mode' field above
 */
#define MEM_READ  1
#define MEM_WRITE 2

/* --- Initialize Processor Board NVRAM Request --- */
#define PI_DEBUG_INIT_PROC_NVRAM_REQ    MRRESET_REQ     /* MR_Defs.h    */

/* --- Initialize Processor Board NVRAM Response --- */
#define PI_DEBUG_INIT_PROC_NVRAM_RSP    MRRESET_RSP     /* MR_Defs.h    */


#define INIT_CCB_NVRAM_TYPE_FULL    0
#define INIT_CCB_NVRAM_TYPE_LICENSE 1

/* --- Initialize CCB NVRAM Request --- */
typedef struct _PI_DEBUG_INIT_CCB_NVRAM_REQ
{
    UINT8       type;
    UINT8       rsvd[3];
} PI_DEBUG_INIT_CCB_NVRAM_REQ;

/* --- Initialize CCB NVRAM Response --- */
/* No additional response data beyond the status in the header. */

/* --- Get Serial Number Request --- */
typedef struct _PI_DEBUG_GET_SERIAL_NUM_REQ
{
    INT32       type;           /* See serial_num.h for valid types */
} PI_DEBUG_GET_SERIAL_NUM_REQ;

/* --- Get Serial Number Response --- */
typedef struct _PI_DEBUG_GET_SERIAL_NUM_RSP
{
    UINT32      serialNumber;
} PI_DEBUG_GET_SERIAL_NUM_RSP;

/* --- MM Test Request --- */
#define PI_DEBUG_MRMMTEST_REQ            MRMMTEST_REQ

/* --- MM Test Response --- */
#define PI_DEBUG_MRMMTEST_RSP            MRMMTEST_RSP

/* --- Struct Display Request --- */
typedef struct _PI_DEBUG_STRUCT_DISPLAY_REQ
{
    INT32       type;
} PI_DEBUG_STRUCT_DISPLAY_REQ;

/* --- Get Serial Number Response --- */
/* No additional response data beyond the status in the header. */

/* --- Election State Request --- */
/* No additional response data beyond the status in the header. */

/* --- Election State Response --- */
#define PI_DEBUG_ELECTION_STATE_RSP     PI_32_BIT_STATE_RSP

/* --- SCSI Command Request --- */
typedef struct _PI_DEBUG_SCSI_CMD_REQ
{
    PI_WWN_LUN_SELECTOR wwnLun;
    UINT32      cdbLen;         /* cdb length                   */
    UINT8       cdb[16];
    UINT32      dataLen;        /* data length                  */
    ZeroArray(UINT8, data);     /* input data                   */
} PI_DEBUG_SCSI_CMD_REQ;

/* --- Memory SCSI Command Response --- */
typedef struct _PI_DEBUG_SCSI_CMD_RSP
{
    UINT8       sense;          /* Sense key                    */
    UINT8       asc;            /* Addional sense code          */
    UINT8       ascq;           /* Addional sense code qualifier */
    UINT32      length;         /* actual response data length  */
    ZeroArray(UINT8, data);     /* response data                */
} PI_DEBUG_SCSI_CMD_RSP;

/* --- READWRITE Command Request --- */
typedef struct _PI_DEBUG_READWRITE_CMD_REQ
{
    UINT8       pv;             /* 'p'disk or 'v'disk           */
    UINT8       rw;             /* 'r'ead or 'w'rite            */
    UINT16      id;             /* pid or vid                   */
    UINT64      block;          /* Block number to start with   */
    UINT32      dataInLen;      /* Data Input length            */
    ZeroArray(UINT8, data);     /* input data                   */
} PI_DEBUG_READWRITE_CMD_REQ;

/* --- Memory READWRITE Command Response --- */
typedef struct _PI_DEBUG_READWRITE_CMD_RSP
{
    UINT32      length;         /* actual response data length  */
    ZeroArray(UINT8, data);     /* response data                */
} PI_DEBUG_READWRITE_CMD_RSP;

/* --- Loop Primitive Request --- */
#define PI_LOOP_PRIMITIVE_REQ           MRLOOPPRIMITIVE_REQ     /* MR_Defs.h    */

/* --- Loop Primitive Response --- */
#define PI_LOOP_PRIMITIVE_RSP           MRLOOPPRIMITIVE_RSP     /* MR_Defs.h    */

/* --- Get State RM Request --- */
/* No additional response data beyond the status in the header. */

/* --- Get State RM Response --- */
#define PI_DEBUG_GET_STATE_RM_RSP       PI_32_BIT_STATE_RSP

#if ISCSI_CODE
/* --- Set ISCSI Target Parameter Request   ------ */
#define PI_SETTGINFO_REQ                MRSETTGINFO_REQ

/* --- Set ISCSI Target Parameter Response   ----- */
#define PI_SETTGINFO_RSP                MRSETTGINFO_RSP

/* --- Get ISCSI Target Parameter Info Request --- */
#define PI_GETTGINFO_REQ                MRGETTGINFO_REQ

/* --- Get ISCSI Target Parameter Info Response -- */
#define PI_GETTGINFO_RSP                MRGETTGINFO_RSP

/* --- Set CHAP User Info Request   -------------- */
#define PI_SETCHAP_REQ                  MRSETCHAP_REQ

/* --- Set CHAP User Info Response   ------------- */
#define PI_SETCHAP_RSP                  MRSETCHAP_RSP

/* --- Get CHAP User Info Request   -------------- */
#define PI_GETCHAP_REQ                  MRGETCHAP_REQ

/* --- Get CHAP User Info Response  -------------- */
#define PI_GETCHAP_RSP                  MRGETCHAP_RSP

/* --- Set Server SID Request -------------------- */
#define PI_UPDSID_REQ                   MRUPDSID_REQ

/* --- Set Server SID Response-------------------- */
#define PI_UPDSID_RSP                   MRUPDSID_RSP

/* --- Set ISCSI Sessions Info Request      ------ */
#define PI_GETSESSIONS_REQ              MRGETSESSIONS_REQ

/* --- Set ISCSI Sessions Info Response     ------ */
#define PI_GETSESSIONS_RSP              MRGETSESSIONS_RSP

/* --- Set ISNS information Response     ------ */
#define PI_SETISNSINFO_RSP              MRSETISNSINFO_RSP

/* --- set ISNS information Request      ------ */
#define PI_SETISNSINFO_REQ              MRSETISNSINFO_REQ

/* --- get ISNS info Response     ------ */
#define PI_GETISNSINFO_RSP              MRGETISNSINFO_RSP
#endif

/*--------------------------------------------------------------------------*/
/* Generic Command Packets (type 1 and 2)                                   */
/*--------------------------------------------------------------------------*/

/* --- Generic Request --- */
typedef struct _PI_GENERIC_REQ
{
    UINT32      subCmdCode;     /* Command code for this generic command    */
    UINT32      socket;         /* active socket (PI_GENERIC2_CMD)          */
    UINT32      responseLength; /* # bytes to allocate for response data    */
    UINT32      data[64];       /* Input data for the command               */
} PI_GENERIC_REQ;

/* --- Generic Response --- */
/*
** The user will have to deal with this.  The request packet contains the
** number of bytes to allocate for response data.
*/

/*
 * Function command codes used in the 'subCmdCode' field for PI_GENERIC_CMD's
 */
#define PI_GENERIC_RESET                    0x00

/* UNUSED                                   0x01 */
#define PI_GENERIC_GLOBAL_MRP_TIMEOUT       0x02
#define PI_GENERIC_FUNCTION_CALL            0x03
#define PI_GENERIC_DEBUG_ADDRESS            0x04
#define PI_GENERIC_GLOBAL_PI_SELECT_TIMEOUT 0x05

/* UNUSED                                   0x06 */
#define PI_GENERIC_DO_ELECTION              0x07
#define PI_GENERIC_GLOBAL_IPC_TIMEOUT       0x08

/* UNUSED                                   0x09 */
#define PI_GENERIC_DISABLE_HEARTBEATS       0x0A
#define PI_GENERIC_ERROR_TRAP               0x0B
#define PI_GENERIC_GET_SOS_STRUCTURE        0x0C
#define PI_GENERIC_SET_PDISK_LED            0x0D
#define PI_GENERIC_SEND_LOG_EVENT           0x0E
#define PI_GENERIC_CACHE_TEST               0x0F
#define PI_GENERIC_DISASTER_TEST            0x10
#define PI_GENERIC_KEEP_ALIVE_TEST          0x11
#define PI_GENERIC_FIO_MAP_TEST             0x12
#define PI_GENERIC_FCM_COUNTER_TEST         0x13

/*
 * constants used in PI_GENERIC_ERROR_TRAP above
 */
#define PI_GENERIC_ERROR_TRAP_CCB           0x00
#define PI_GENERIC_ERROR_TRAP_BE            0x01
#define PI_GENERIC_ERROR_TRAP_FE            0x02
#define PI_GENERIC_ERROR_TRAP_ALL           0x03

/*
 * constants used in PI_GENERIC_CACHE_TEST above
 */
#define PI_GENERIC_CACHE_TEST_START         0x10
#define PI_GENERIC_CACHE_TEST_STOP          0x11
#define PI_GENERIC_CACHE_TEST_CACHE         0x10
#define PI_GENERIC_CACHE_TEST_X1            0x11
#define PI_GENERIC_CACHE_TEST_ASYNC         0x12

/*
 * constants used in PI_GENERIC_DISASTER_FLAG above
 */
#define PI_GENERIC_DISASTER_TEST_RESET      0x00
#define PI_GENERIC_DISASTER_TEST_CLEAR      0x01
#define PI_GENERIC_DISASTER_TEST_SET        0x02

/*
 * constants used in PI_GENERIC_KEEP_ALIVE_FLAG above
 */
#define PI_GENERIC_KEEP_ALIVE_TEST_RESET    0x00
#define PI_GENERIC_KEEP_ALIVE_TEST_CLEAR    0x01
#define PI_GENERIC_KEEP_ALIVE_TEST_SET      0x02
#define PI_GENERIC_KEEP_ALIVE_TEST_DISABLE  0x03
#define PI_GENERIC_KEEP_ALIVE_TEST_ENABLE   0x04

/*
 * constants used in PI_GENERIC_FIO_MAP_TEST above
 */

/* Parameter 0 */
#define PI_GENERIC_FIO_MAP_TEST_READ        0x00
#define PI_GENERIC_FIO_MAP_TEST_WRITE       0x01

/* Parameter 1 */
#define PI_GENERIC_FIO_MAP_TEST_RESET       0x00
#define PI_GENERIC_FIO_MAP_TEST_CLEAR       0x01
#define PI_GENERIC_FIO_MAP_TEST_SET         0x02

/*
 * constants used in PI_GENERIC_FCM_COUNTER_TEST above
 */

/* Parameter 0 */
#define PI_GENERIC_FCM_COUNTER_TEST_DUMP            0x00
#define PI_GENERIC_FCM_COUNTER_TEST_BASELINE        0x01
#define PI_GENERIC_FCM_COUNTER_TEST_UPDATE          0x02
#define PI_GENERIC_FCM_COUNTER_TEST_DELTA           0x03
#define PI_GENERIC_FCM_COUNTER_TEST_MAJOR_EVENT     0x04
#define PI_GENERIC_FCM_COUNTER_TEST_MINOR_EVENT     0x05

/*
 * Function command codes used in the 'subCmdCode' field for PI_GENERIC2_CMD's
 */
#define PI_GENERIC2_GET_HEAP_STATS          0
#define PI_GENERIC2_GET_TRACE_STATS         1
#define PI_GENERIC2_GET_PCB_STATS           2
#define PI_GENERIC2_GET_PROFILE_STATS       3
#define PI_GENERIC2_GET_PACKET_STATS        4

/* --- Generic Request --- */
typedef struct _PI_GENERIC_MRP_REQ
{
    UINT32      mrpCmd;         /* Command code for this generic MRP        */
    UINT32      inputLength;    /* # bytes of the input data that are used  */
    UINT32      responseLength; /* # bytes to allocate for response data    */
    ZeroArray(UINT8, data);     /* MRP input packet                         */
} PI_GENERIC_MRP_REQ;

/* --- Generic MRP Response --- */
/*
** The user will have to deal with this.  The request packet contains the
** number of bytes to allocate for response data.
*/

/*--------------------------------------------------------------------------*/
/* Persisten Data Command Packets                                            */
/*--------------------------------------------------------------------------*/
#define PERSISTENT_DATA_OPTION_READ     0x00
#define PERSISTENT_DATA_OPTION_WRITE    0x01
#define PERSISTENT_DATA_OPTION_RESET    0x02
#define PERSISTENT_DATA_OPTION_CHECKSUM 0x03

/* --- Persistent Data Control Request --- */
typedef struct _PI_PERSISTENT_DATA_CONTROL_REQ
{
    UINT8       option;         /* option           */
    UINT8       rsvd[3];        /* reserved         */
    UINT32      offset;         /* offset           */
    UINT32      length;         /* length           */
    ZeroArray(UINT8, buffer);   /* data to write  */
} PI_PERSISTENT_DATA_CONTROL_REQ;

/* --- Persistent Data Control Response --- */
typedef struct _PI_PERSISTENT_DATA_CONTROL_RSP
{
    UINT8       option;         /* option Used      */
    UINT8       checksum;       /* Checksum         */
    UINT8       rsvd1[2];       /* reserved         */
    UINT32      length;         /* length Read      */
    UINT8       rsvd2[8];       /* reserved (pad structure to 16 bytes) */
    ZeroArray(UINT8, buffer);   /* data read      */
} PI_PERSISTENT_DATA_CONTROL_RSP;

/*--------------------------------------------------------------------------*/
/* CLIENT Persisten Data Command Packets                                            */
/*--------------------------------------------------------------------------*/
/********************************************************************
 * CLIENT_OPTION_NOP
 * Allows client to modify the records flags without taking any
 * other action.
 *
 * Constraints
 *  Only valid on the master controller
 *
 * Post Condition
 *  Flags for record with specified name are modified on all
 *    controllers.
 *
 * Required fields:
 *  option = CLIENT_OPTION_NOP
 *  flags  = One or more of CLIENT_FLAG_LOCK, CLIENT_FLAG_UNLOCK,
 *             CLIENT_FLAG_TRUNCATE
 *    timeOutSec = Must be set if CLIENT_FLAG_LOCK
 *                 0 otherwise
 *    offset = Must be specified if CLIENT_FLAG_TRUNCATE
 *                Record will be truncated at offset + length
 *                0 otherwise
 *  length =    Must be specified if CLIENT_FLAG_TRUNCATE
 *                0 otherwise
 *    recordName
 *
 * Unused fields: (Should be initialized to 0)
 *  rsvd
 *  buffer
 *
 * Possible PI_PACKET_HEADER.errorCode values:
 *  PDATA_INVALID_NAME
 *  PCD2_LOCKED
 *
 * Response:
 *  PI_PACKET_HEADER
 *    PI_CLIENT_DATA_CONTROL_REQ
 *  buffer:  N/A
 *
 */
#define CLIENT_OPTION_NOP               0x00

/********************************************************************
 * CLIENT_OPTION_CREATE_RECORD
 * Create a new record with the specified name and length.
 *
 * Constraints
 *  Only valid on the master controller
 *
 * Post Condition
 *  Record with specified name of the specified length exists.
 *  All bytes of the record are set to 0x00.
 *  Record is created on all controllers
 *
 * Required fields:
 *  option =     CLIENT_OPTION_CREATE_RECORD
 *  flags  =     CLIENT_FLAG_LOCK
 *    timeOutSec = Must be set if CLIENT_FLAG_LOCK
 *                 0 otherwise
 *    recordName
 *
 * Unused fields: (Should be initialized to 0)
 *  rsvd
 *  buffer
 *  offset
 *  length
 *
 * Possible PI_PACKET_HEADER.errorCode values:
 *  PDATA_NOT_ENOUGH_SPACE
 *  PDATA_DUPLICATE_RECORD
 *  PDATA_INVALID_NAME
 *
 * Response:
 *  PI_PACKET_HEADER
 *  PI_CLIENT_DATA_CONTROL_RSP
 *  buffer:  N/A
 *
 *
 */
#define CLIENT_OPTION_CREATE_RECORD     0x01

/********************************************************************
 * CLIENT_OPTION_REMOVE_RECORD
 * Remove the record with the specified name
 *
 * Constraints
 *  Only valid on the master controller
 *
 * Post Condition
 *  Record with specified name is removed from all controllers.
 *
 * Required fields:
 *  option = CLIENT_OPTION_REMOVE_RECORD
 *    recordName
 *
 * Unused fields: (Should be initialized to 0)
 *  flags
 *  timeOutSec
 *  length
 *  rsvd
 *  offset
 *  buffer
 *
 * Possible PI_PACKET_HEADER.errorCode values:
 *  PDATA_INVALID_NAME
 *  PDATA_RECORD_NOT_FOUND
 *  PDATA_LOCKED
 *
 * Response:
 *  PI_PACKET_HEADER
 *  PI_CLIENT_DATA_CONTROL_RSP
 *  buffer:  N/A
 */
#define CLIENT_OPTION_REMOVE_RECORD     0x02

/********************************************************************
 * CLIENT_OPTION_READ_RECORD
 * Read a byte range from the specified record
 *
 * Constraints
 *  None
 *
 * Post Condition
 *  Record is unchanged
 *
 * Required fields:
 *  option = CLIENT_OPTION_READ_RECORD
 *  flags  = 0 or
 *    recordName
 *  length  The number of bytes to read
 *  offset  Offset of the first byte to read
 *    timeOutSec = Must be set if CLIENT_FLAG_LOCK
 *                 0 otherwise
 *
 * Unused fields: (Should be initialized to 0)
 *  rsvd
 *  buffer
 *
 * Possible PI_PACKET_HEADER.errorCode values:
 *  PCD2_INVALID_NAME
 *  PCD2_REC0RD_NOT_FOUND
 *
 * Response:
 *  PI_PACKET_HEADER
 *  PI_CLIENT_DATA_CONTROL_RSP
 *    May return fewer bytes than requested if the end of record
 *    is reached. PDATA_EOF_REACH will be set in
 *    PI_PACKET_HEADER.errorCode even if status == PI_GOOD
 *    buffer format:  byte array of data read.
 *
 */
#define CLIENT_OPTION_READ_RECORD       0x03

/********************************************************************
 * CLIENT_OPTION_WRITE_RECORD
 * Write a byte range from the specified record
 *
 * Constraints
 *  Only valid on the master controller
 *
 * Post Condition
 *  Specified byte range is updated with the bytes supplied
 *    in data.
 *  Data is updated on all controllers.
 *
 * Required fields:
 *  option = PCD2_OPTION_WRITE_RECORD
 *    recordName
 *  length  The number of bytes to write
 *  offset  Offset of the first byte to write
 *  buffer  The data to be written
 *  flags       CLIENT_FLAG_TRUNCATE
 *          Record will be truncated at offset + length
 *    timeOutSec = Must be set if CLIENT_FLAG_LOCK
 *                 0 otherwise
 *
 * Unused fields: (Should be initialized to 0)
 *  rsvd
 *
 * Possible PI_PACKET_HEADER.errorCode values:
 *  PDATA_INVALID_NAME
 *  PDATA_RECORD_NOT_FOUND
 *  PDATA_LOCKED
 *
 * Response:
 *  PI_PACKET_HEADER
 *  PI_CLIENT_DATA_CONTROL_RSP
 *  buffer:  N/A
 */
#define CLIENT_OPTION_WRITE_RECORD      0x04

/********************************************************************
 * CLIENT_OPTION_LIST_RECORDS
 * Return information about each record.
 *
 * Constraints
 *  None
 *
 * Post Condition
 *  Record is unchanged.
 *
 * Required fields:
 *  option = CLIENT_OPTION_LIST_RECORDS
 *
 * Unused fields: (Should be initialized to 0)
 *    flags = 0
 *    recordName
 *    timeOutSec
 *  length
 *  offset
 *  buffer
 *  rsvd
 *
 * Possible PI_PACKET_HEADER.errorCode values:
 *
 * Response:
 *  PI_PACKET_HEADER
 *    PI_CLIENT_RECORD_LIST
 */
#define CLIENT_OPTION_LIST_RECORDS      0x05

/* Flags
 * One or more of the CLIENT_FLAG_... values defined below may be OR'd
 * together in flags.
 */

/**
 * CLIENT_FLAG_LOCK
 * The record specified by name will be locked. The controller
 * will prevent any commands from other connections from accessing
 * the record. Only commands sent on the current
 * socket will be permitted.
 *
 * If a lock is currently held by another controller, the response
 * will be returned with status = PDATA_LOCKED
 *
 * The lock will be acquired before executing the command.
 *
 * Post Condition
 *  The record is marked as locked.
 *
 * A lock on a record prevents any action on the record by any
 * other client.
 *
 *
 *  PI_PACKET_HEADER.status     = PI_ERROR;
 *  PI_PACKET_HEADER.errorCode  = PDATA_LOCKED;
 *
*/
#define CLIENT_FLAG_LOCK              0x0001

/**
 * CLIENT_FLAG_TRUNCATE
 * The specified record will be truncated to offset + length bytes.
 *
 * If a lock is currently held by another client, the response
 * will be returned with status = PDATA_LOCKED
 *
 * Flag has no effect for CREATE_OPTION_LIST_RECORD and
 * CREATE_OPTION_REMOVE_RECORD
 *
 * Post Condition
 *  The record length is offset + length bytes long.
 *    If this results in the record length being expanded the new
 *    bytes are set to 0.
 *
*/
#define CLIENT_FLAG_TRUNC             0x0004

/**
 * CLIENT_FLAG_UNLOCK
 * The record specified by name will be unlocked if a lock is
 * is held by the current socket. The command will succeed if the
 * record is not locked.
 *
 * If a lock is held by another connection the following
 * respose status will be returned.
 *  PI_PACKET_HEADER.status     = PI_ERROR;
 *  PI_PACKET_HEADER.errorCode  = PDATA_LOCKED;
 *
 */
#define CLIENT_FLAG_UNLOCK            0x0002

/*
** The maximum allowable writes is up to 64K.
*/
#define CLIENT_DATA_750_MAX_IO_SIZE    SIZE_64K

/**
 * CLIENT_RECORD_NAME_MAX_LEN
 * The maximum size of the client record name is 256 bytes
 *
 */
#define CLIENT_RECORD_NAME_MAX_LEN     256

typedef struct _PI_CLIENT_MGT_STRUCTURE
{
    UINT8       recordName[256];        /* record name. 0x00 terminated string  can only
                                         * contain digits, alphabet and underscore */
    UINT32      recordLength;   /* record size in bytes */
    UINT32      recordLocked;   /* 1 if record is locked 0 if unlocked */
    UINT32      timeStamp;      /* timestamp of last modification */
} PI_CLIENT_MGT_STRUCTURE;

typedef struct _PI_CLIENT_RECORD_LIST
{
    UINT32      count;          /* Number of records */
    UINT32      maxSpace;       /* Specifies the maximum amount
                                 * of space reserved by the controller
                                 * for Client records in Kilo bytes */
    UINT32      freeSpace;      /* Specifies the amount of unused
                                 * space for client records in Kilo bytes */
    ZeroArray(PI_CLIENT_MGT_STRUCTURE, records); /* data to write  */
} PI_CLIENT_RECORD_LIST;

/* --- CLIENT Data Control Request --- */
typedef struct _PI_CLIENT_DATA_CONTROL_REQ
{
    UINT16      option;         /* Command option. One of the Client options. */
    UINT16      flags;          /* Command flags. One of Client flags. */
    UINT32      timeOutSec;     /* Timeout in seconds in case lock is applied.
                                 * Lock will be valid for theis many seconds. */
    UINT8       recordName[256]; /* Valid record name. not applicable
                                  * for list 0x00 terminated string      */
    UINT32      offset;         /* Offset within the record
                                 * used for read/write           */
    UINT32      length;         /* Length used for read/wrrite          */
    ZeroArray(UINT8, buffer);   /* data to write. used in write command */
} PI_CLIENT_DATA_CONTROL_REQ;

typedef struct _PI_CLIENT_DATA_CONTROL_RSP
{
    UINT8       option;         /* Command option                        */
    UINT8       rsvd[3];
    UINT8       recordName[256]; /* Valid record name. not applicable
                                  * for list 0x00 terminated string       */
    UINT32      offset;         /* Offset within the record
                                 * used for read/write            */
    UINT32      length;         /* Length used for read and list options */
    ZeroArray(UINT8, buffer);   /* return data used for read and list    */
} PI_CLIENT_DATA_CONTROL_RSP;

/*--------------------------------------------------------------------------*/
/* Write Cache Command Packets                                              */
/*--------------------------------------------------------------------------*/

/* --- Write Cache Invalidate Defines --- */
#define PI_WCACHE_INV_OPT_BE    0x0001
#define PI_WCACHE_INV_OPT_FE    0x0002
#define PI_WCACHE_INV_OPT_BEFE  (PI_WCACHE_INV_OPT_BE | PI_WCACHE_INV_OPT_FE)

/* --- Write Cache Invalidate Request --- */
typedef struct PI_WCACHE_INVALIDATE_REQ
{
    UINT32      option;
} PI_WCACHE_INVALIDATE_REQ;

/* --- Write Cache Invalidate Response --- */
/* No additional response data beyond the status in the header. */

/*--------------------------------------------------------------------------*/
/* Miscellaneous Command Packets                                            */
/*--------------------------------------------------------------------------*/

/* --- Get Device Count Request --- */
#define PI_MISC_GET_DEVICE_COUNT_REQ    MRDEVICECOUNT_REQ       /* MR_Defs.h    */

/* --- Get Device Count Response --- */
#define PI_MISC_GET_DEVICE_COUNT_RSP    MRDEVICECOUNT_RSP       /* MR_Defs.h    */

/* --- Rescan Device Request --- */
#define PI_MISC_RESCAN_DEVICE_REQ       MRRESCANDEVICE_REQ      /* MR_Defs.h    */

/* --- Rescan Device Response --- */
#define PI_MISC_RESCAN_DEVICE_RSP       MRRESCANDEVICE_RSP      /* MR_Defs.h    */

/* --- Device List Request --- */
/*
** WARNING: This request is for engineering use only and should not be used
** in run-time code.
*/
#define PI_MISC_DEVICE_LIST_REQ         MRGETDVLIST_REQ /* MR_Defs.h    */

/* --- Device List Response --- */
#define PI_MISC_DEVICE_LIST_RSP         MRGETDVLIST_RSP /* MR_Defs.h    */

/* --- File System Read Request --- */
typedef struct _PI_MISC_FILE_SYSTEM_READ_REQ
{
    UINT32      fileID;         /* File ID of file to read                  */
} PI_MISC_FILE_SYSTEM_READ_REQ;

/* --- File System Write Request --- */
typedef struct _PI_MISC_FILE_SYSTEM_WRITE_REQ
{
    UINT32      fileID;         /* File ID of file to write                 */
    UINT32      length;         /* Number of bytes of data to follow        */
    ZeroArray(UINT8, data);     /* Data to write                            */
} PI_MISC_FILE_SYSTEM_WRITE_REQ;

/* --- File System Write Response --- */
/* No additional response data beyond the status in the header. */

/* --- Get Mode Data Request --- */
/* No input parameters are required */

/* --- Get Mode Data Response --- */
typedef struct _PI_MISC_GET_MODE_RSP
{
    MODEDATA    modeData;
} PI_MISC_GET_MODE_RSP;

/* --- Set Mode Data Request --- */
typedef struct _PI_MISC_SET_MODE_REQ
{
    MODEDATA    modeData;
    MODEDATA    modeMask;
} PI_MISC_SET_MODE_REQ;

/* --- Set Mode Data Response --- */
/* No additional response data beyond the status in the header. */

/* --- Back End of Front End Port List Request --- */
#define PI_PORT_LIST_REQ                MRGETPORTLIST_REQ       /* MR_Defs.h    */

/* --- Back End of Front End Port List Response --- */
#define PI_PORT_LIST_RSP                PI_LIST_RSP

/* --- Failure State Set Request --- */
typedef struct _PI_MISC_FAILURE_STATE_SET_REQ
{
    UINT32      serialNumber;   /* Serial number of the controller          */
    UINT32      failureState;   /* Failure state to set for this controller */
} PI_MISC_FAILURE_STATE_SET_REQ;

/* --- Failure State Set Response --- */
/* No additional response data beyond the status in the header. */

/* --- Unfail Interface Request --- */
typedef struct _PI_MISC_UNFAIL_INTERFACE_REQ
{
    UINT32      controllerSN;   /* Controller Serial Number                 */
#ifdef _WIN32
    UINT8       intfc;          /* Channel # of the interface to unfail     */
#else
    UINT8       interface;      /* Channel # of the interface to unfail     */
#endif
    UINT8       rsvd[3];        /* RESERVED                                 */
} PI_MISC_UNFAIL_INTERFACE_REQ;

/* --- Unfail Interface Response --- */
/* No additional response data beyond the status in the header. */

/* --- Fail Interface Request --- */
typedef struct _PI_MISC_FAIL_INTERFACE_REQ
{
    UINT32      controllerSN;   /* Controller Serial Number                 */
#ifdef _WIN32
    UINT8       intfc;          /* Channel # of the interface to fail       */
#else
    UINT8       interface;      /* Channel # of the interface to fail       */
#endif
    UINT8       rsvd[3];        /* RESERVED                                 */
} PI_MISC_FAIL_INTERFACE_REQ;

/* --- Fail Interface Response --- */
/* No additional response data beyond the status in the header. */

/* --- Serial Number Set Request --- */
typedef struct _PI_MISC_SERIAL_NUMBER_SET_REQ
{
    UINT8       which;          /**< Which serial number to set             */
    UINT8       rsvd[3];        /**< RESERVED                               */
    UINT32      serialNum;      /**< Serial Number                          */
} PI_MISC_SERIAL_NUMBER_SET_REQ;

/* --- Serial Number Set Response --- */
/* No additional response data beyond the status in the header. */

/* --- Resync Mirror Records Request --- */
typedef struct _PI_MISC_RESYNC_MIRROR_RECORDS_REQ
{
    UINT8       type;           /**< Type of Resync                         */
    UINT8       rsvd1[1];       /**< RESERVED                               */
    UINT16      rid;            /**< Raid Identifier                        */
} PI_MISC_RESYNC_MIRROR_RECORDS_REQ;

/* --- Resync Mirror Records Response --- */
/* No additional response data beyond the status in the header. */

/* --- Get Workset Information Request --- */
#define PI_MISC_GET_WORKSET_INFO_REQ    MRGETWSINFO_REQ /* MR_Defs.h    */

/* --- Get Workset Information Response --- */
typedef struct _PI_MISC_GET_WORKSET_INFO_RSP
{
    UINT16      count;          /* Number of worksets to follow     */
    UINT8       rsvd[2];        /* RESERVED                         */
    ZeroArray(DEF_WORKSET, workset); /* Array of count worksets          */
} PI_MISC_GET_WORKSET_INFO_RSP;

/* --- Set Workset Information Request --- */
#define PI_MISC_SET_WORKSET_INFO_REQ    MRSETWSINFO_REQ /* MR_Defs.h    */

/* --- Set Workset Information Response --- */
#define PI_MISC_SET_WORKSET_INFO_RSP    MRSETWSINFO_RSP /* MR_Defs.h    */

/* --- Get GeoPool Information Request --- */
/* No input parameters are required */

/* --- Continue W/O Mirror Partner Request --- */
/* No input parameters are required */

/* --- Continue W/O Mirror Partner Response --- */
#define PI_MISC_CONTINUE_WO_MP_RSP      MRFECONTWOMP_RSP        /* MR_Defs.h    */

/* --- Invalidate BE Cache Request --- */
#define PI_MISC_INVALIDATE_BE_WC_REQ    MRINVBEWC_REQ   /* MR_Defs.h    */

/* --- Invalidate BE Cache Response --- */
#define PI_MISC_INVALIDATE_BE_WC_RSP    MRINVBEWC_RSP   /* MR_Defs.h    */


/* --- Mirror Partner Control Request --- */
typedef struct _PI_MISC_MIRROR_PARTNER_CONTROL_REQ
{
    UINT32      partnerSN;              /**< Mirror Partner Serial Number   */
    UINT32      option;                 /**< Control Options                */
    /*
     ** NOTE: This packet was extended for the 4.1 release to contain
     **       the mirror partner information.  The code still handles
     **       the case where the information is not sent.  If you
     **       intend for the recipient to use this information you
     **       must set the MPINFO_VALID option bit and then only if
     **       the recipient controller knows about the bit will the
     **       information be used.
     */
    MP_MIRROR_PARTNER_INFO mpInfo;      /**< Mirror Partner Information     */
} PI_MISC_MIRROR_PARTNER_CONTROL_REQ;

#define MIRROR_PARTNER_CONTROL_OPT_WOMP             0x0001
#define MIRROR_PARTNER_CONTROL_OPT_STOPIO           0x0002
#define MIRROR_PARTNER_CONTROL_OPT_RESYNC           0x0004
#define MIRROR_PARTNER_CONTROL_OPT_RESYNC_ONLY      0x0008
#define MIRROR_PARTNER_CONTROL_OPT_RESYNC_ALL       0x0010
#define MIRROR_PARTNER_CONTROL_OPT_MPINFO_VALID     0x0020

/* --- Mirror Partner Control Response --- */
/* No additional response data beyond the status in the header. */

/* --- Mirror Partner Get Config Request --- */
/* No input parameters are required */

/* --- Mirror Partner Get Config Response --- */
#define PI_MISC_MIRROR_PARTNER_GET_CFG_RSP MRGETMPCONFIGFE_RSP

/* --- Modify Raid AStatus Request --- */
#define PI_RAID_MIRRORING_REQ       MRCHGRAIDNOTMIRRORING_REQ   /* MR_Defs.h  */

#define RAID_NOT_MIRRORING          MRCSETNOTMIRRORING
#define RAID_MIRRORING              MRCCLEARNOTMIRRORING

/* --- Modify Raid AStatus Response --- */
#define PI_RAID_MIRRORING_RSP       MRCHGRAIDNOTMIRRORING_RSP   /* MR_Defs.h  */

/* --- Cache Refresh CCB Request --- */
#define CACHE_REFRESH_CCB_WAIT_FOR_CMPL     TRUE
#define CACHE_REFRESH_CCB_NO_WAIT           FALSE

typedef struct PI_CACHE_REFRESH_CCB_REQ
{
    UINT32      cacheMask;      /* Mask of which caches to refresh      */
    UINT8       waitForCompletion;      /* TRUE=wait, FALSE=don't wait          */
    UINT8       rsvd[3];        /* RESERVED                             */
} PI_CACHE_REFRESH_CCB_REQ;

/* --- Cache Refresh CCB Response --- */
/* No additional response data beyond the status in the header. */

/* --- Set DLM Heartbeat Request --- */
#define PI_SET_DLM_HEARTBEAT_LIST_REQ       MRFEFIBREHLIST_REQ  /* MR_Defs.h */

/* --- Set DLM Heartbeat Response --- */
#define PI_SET_DLM_HEARTBEAT_LIST_RSP       MRFEFIBREHLIST_RSP  /* MR_Defs.h */

/* --- Cache Flush BE Request --- */
#define PI_CACHE_FLUSH_BE_REQ               MRFLUSHBEWC_REQ     /* MR_Defs.h    */

/* --- Cache Flush BE Response --- */
#define PI_CACHE_FLUSH_BE_RSP               MRFLUSHBEWC_RSP     /* MR_Defs.h    */

/* --- Resync Raids Request --- */
typedef struct PI_MISC_RESYNC_RAIDS_REQ
{
    UINT32      count;          /**< Count of raids in the array            */
    ZeroArray(UINT16, raids);   /**< Array of raid identifiers              */
} PI_MISC_RESYNC_RAIDS_REQ;

/* --- Resync Raids Response --- */
/* No additional response data beyond the status in the header. */

/* --- Put Device Configuration Request  --- */
typedef struct PI_MISC_PUTDEVCONFIG_REQ
{
    UINT16      count;                  /**< Count of devices in the array  */
    UINT8       rsvd2[2];               /**< RESERVED                       */
    ZeroArray(SES_DEV_INFO_MAP, map);   /**< Device Information Map         */
} PI_MISC_PUTDEVCONFIG_REQ;

/* --- Put Device Configuration Response --- */
/* No additional response data beyond the status in the header. */

/* --- Get Device Configuration Request  --- */
/* No input parameters are required */

/* --- Get Device Configuration Response --- */
typedef struct PI_MISC_GETDEVCONFIG_RSP
{
    UINT16      count;                  /**< Count of devices in the array  */
    UINT8       rsvd2[2];               /**< RESERVED                       */
    ZeroArray(SES_DEV_INFO_MAP, map);   /**< Device Information Map         */
} PI_MISC_GETDEVCONFIG_RSP;

/* --- Set Battery Health Request --- */
#define PI_BATTERY_HEALTH_SET_REQ           MRSETBATHEALTH_REQ  /* MR_Defs.h */

/* --- Set Battery Health Response --- */
#define PI_BATTERY_HEALTH_SET_RSP           MRSETBATHEALTH_RSP  /* MR_Defs.h */

/* --- Invalidate FE Cache Request --- */
#define PI_MISC_INVALIDATE_FE_WC_REQ        MRINVFEWC_REQ       /* MR_Defs.h    */

/* --- Invalidate FE Cache Response --- */
#define PI_MISC_INVALIDATE_FE_WC_RSP        MRINVFEWC_RSP       /* MR_Defs.h    */

/* --- Query Mirror Partner Change Request --- */
#define PI_MISC_QUERY_MP_CHANGE_REQ         MRQMPC_REQ  /* MR_Defs.h    */

/* --- Query Mirror Partner Change Response --- */
#define PI_MISC_QUERY_MP_CHANGE_RSP         MRQMPC_RSP  /* MR_Defs.h    */

/* --- Resync Data Request --- */
#define PI_MISC_RESYNCDATA_REQ              MRRESYNCDATA_REQ    /* MR_Defs.h    */

/* --- Resync Data Response --- */
typedef struct PI_MISC_RESYNCDATA_MRP_DATA
{
    ZeroArray(UINT8, data);             /**< Copy operation registration        */
                                        /**< NOTE: The preceeding array supports*/
                                        /**<       multiple data formats        */
} PI_MISC_RESYNCDATA_MRP_DATA;

typedef struct PI_MISC_RESYNCDATA_RSP
{
    MR_HDR_RSP  header;                 /**< MRP Response Header - 8 bytes      */
    UINT16      count;                  /**< Count of devices in the array  */
    UINT8       format;                 /**< format of data                 */
    UINT8       size;                   /**< size of data elements          */
    PI_MISC_RESYNCDATA_MRP_DATA data;   /**< RESYNCDATA MRP information     */
} PI_MISC_RESYNCDATA_RSP;

/* --- Resync Control Request --- */
#define PI_MISC_RESYNCCTL_REQ               MRRESYNCCTL_REQ     /* MR_Defs.h    */

/* --- Resync Control Response --- */
#define PI_MISC_RESYNCCTL_RSP               MRRESYNCCTL_RSP     /* MR_Defs.h    */

/* --- Set Temporarily Disable Cache Request (set/clear have similar requests, clear has an option in second byte) --- */
#define PI_MISC_SETTDISCACHE_REQ            MRSETTDISCACHE_REQ  /* MR_Defs.h */

/* --- Set Temporarily Disable Cache Response (set/clear need to have same response packets) --- */
#define PI_MISC_SETTDISCACHE_RSP            MR_GENERIC_RSP      /* MR_Defs.h    */

/* --- Clear Temporarily Disable Cache Request (set/clear have similar requests, clear has an option in second byte) --- */
#define PI_MISC_CLRTDISCACHE_REQ            MRCLRTDISCACHE_REQ  /* MR_Defs.h */

/* --- Clear Temporarily Disable Cache Response (set/clear need to have same response packets) --- */
#define PI_MISC_CLRTDISCACHE_RSP            MR_GENERIC_RSP      /* MR_Defs.h    */

/* --- Query Temporarily Disable Cache Request --- */
/* No input parameters are required */

/* --- Query Temporarily Disable Cache Response --- */
#define PI_MISC_QTDISCACHE_RSP              MRQTDISABLEDONE_RSP /* MR_Defs.h */

/* --- Configure Options Request --- */
#define PI_MISC_CFGOPTION_REQ               MRCFGOPTION_REQ     /* MR_Defs.h */

/* --- Configure Options Response --- */
#define PI_MISC_CFGOPTION_RSP               MRCFGOPTION_RSP     /* MR_Defs.h    */

/* --- Local Raid Info Request --- */
/* No input parameters are required */

/* --- Local Raid Info Response --- */
#define PI_MISC_LOCAL_RAID_INFO_RSP         XK_RAIDMON_INFO

/*-----------------BATCH snapshot structures---------------------------------*/
typedef struct _PI_BATCH_SNAPSHOT_START_REQ
{
    UINT16      count;         /** Number of snapshots to be created  **/
    UINT16      rsvd;
} PI_BATCH_SNAPSHOT_START_REQ;

#define PI_BATCH_SNAPSHOT_START_RSP     MRVDISKCONTROL_RSP

typedef struct _PI_BATCH_SNAPSHOT_SEQUENCE_REQ
{
    UINT16      svid;         /** Source vid  **/
    UINT16      dvid;         /** Destination vid  **/
    UINT8       rsvd[4];
} PI_BATCH_SNAPSHOT_SEQUENCE_REQ;

#define PI_BATCH_SNAPSHOT_SEQUENCE_RSP     MRVDISKCONTROL_RSP
#define PI_BATCH_SNAPSHOT_EXECUTE_REQ      PI_QUICK_BREAK_PAUSE_RESUME_MIRROR_EXECUTE_REQ
#define PI_BATCH_SNAPSHOT_EXECUTE_RSP     MRVDISKCONTROL_RSP

/*------------------Quick mirror pause break resume --------------------------*/
typedef struct _PI_QUICK_BREAK_PAUSE_RESUME_MIRROR_START_REQ
{
    UINT8       count;                /**< Number of mirrors to be followed */
    UINT8       rsvd1[3];
} PI_QUICK_BREAK_PAUSE_RESUME_MIRROR_START_REQ;

#define PI_QUICK_BREAK_PAUSE_RESUME_MIRROR_START_RSP     MRVDISKCONTROL_RSP

typedef struct _PI_QUICK_BREAK_PAUSE_RESUME_MIRROR_SEQUENCE_REQ
{
    UINT8       subtype;              /**< operation type break,pause,resume */
    UINT16      dvid;                 /**< destination vid */
    UINT8       rsvd3;
} PI_QUICK_BREAK_PAUSE_RESUME_MIRROR_SEQUENCE_REQ;

#define PI_QUICK_BREAK_PAUSE_RESUME_MIRROR_SEQUENCE_RSP     MRVDISKCONTROL_RSP

/** Actions on execute request **/
#define PI_CANCEL                  0
#define PI_GO                      1

typedef struct _PI_QUICK_BREAK_PAUSE_RESUME_MIRROR_EXECUTE_REQ
{
    UINT8       action;                /**< Action taken on the sequence list */
    UINT8       rsvd1[3];
} PI_QUICK_BREAK_PAUSE_RESUME_MIRROR_EXECUTE_REQ;

#define PI_QUICK_BREAK_PAUSE_RESUME_MIRROR_EXECUTE_RSP      MRVDISKCONTROL_RSP

/*--------------------------------------------------------------------------*/
/* Manufacturing Command Packets                                            */
/*--------------------------------------------------------------------------*/

/* --- Controller Clean Request --- */
typedef struct _PI_MFG_CTRL_CLEAN_REQ
{
    UINT8       rsvd0[3];       /* RESERVED                                 */
    UINT8       option;         /* Cleaning option                          */
    /* Bit Settings:                            */
    /*  bit 0: 0 = full, 1 = license            */
    /*  bit 1: 0 = clear logs, 1 = don't clear  */
    /*  bit 2: Brute force clean the drives     */
    /*         0 = don't, 1 = do                */
    /*  bit 3: unused                           */
    /*  bit 4: unused                           */
    /*  bit 5: unused                           */
    /*  bit 6: unused                           */
    /*  bit 7: unused                           */
} PI_MFG_CTRL_CLEAN_REQ;

#define MFG_CTRL_CLEAN_OPT_LICENSE              0x01
#define MFG_CTRL_CLEAN_OPT_NO_LOG_CLEAR         0x02
#define MFG_CTRL_CLEAN_OPT_FORCE_CLEAN_DRIVES   0x04
#define MFG_CTRL_CLEAN_OPT_SERIAL_MESSAGES      0x08
#define MFG_CTRL_CLEAN_OPT_POWER_DOWN           0x10

/* --- Controller Clean Response --- */
/* No additional response data beyond the status in the header. */

/*--------------------------------------------------------------------------*/
/* Buffer Board (MicroMemory) Packets                                        */
/*--------------------------------------------------------------------------*/

/* --- Buffer Board Status Request --- */
#define PI_STATS_BUFFER_BOARD_REQ           MRMMCARDGETBATTERYSTATUS_REQ

/* --- Buffer Board Status Response --- */
#define PI_STATS_BUFFER_BOARD_RSP           MRMMCARDGETBATTERYSTATUS_RSP

/*--------------------------------------------------------------------------*/
/* Async Event Packets                                                      */
/*--------------------------------------------------------------------------*/
/* --- Log Event Message --- */
#define PI_LOG_EVENT_MESSAGE_RSP            PI_LOG_INFO_RSP

/* --- Asynchronous Changed Event Packet --- */
#define PI_ASYNC_CHANGED_EVENT_RSP          ASYNC_CHANGED_EVENT

/* --- Asynchrnous Ping Event Packet --- */
/* No additional response data beyond the status in the header. */

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _PACKETINTERFACE_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
