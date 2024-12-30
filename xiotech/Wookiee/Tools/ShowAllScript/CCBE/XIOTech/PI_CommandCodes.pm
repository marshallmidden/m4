# $Id: PI_CommandCodes.pm 160950 2013-04-22 21:10:28Z marshall_midden $
##############################################################################
# Xiotech a Seagate Technology
# Copyright (c) 2001  Xiotech
##############################################################################
package XIOTech::PI_CommandCodes;

require Exporter;
@ISA = qw(Exporter);
@EXPORT = qw(
    COMPAT_INDEX_4 
    VERSION_1 
    VERSION_2 
    VERSION_3 
    PI_CONNECT_CMD
    PI_DISCONNECT_CMD
    PI_PING_CMD
    PI_RESET_CMD
    PI_POWER_UP_STATE_CMD
    PI_POWER_UP_RESPONSE_CMD
    PI_REGISTER_EVENTS_CMD  

    PI_PDISK_COUNT_CMD
    PI_PDISK_LIST_CMD
    PI_PDISK_INFO_CMD
    PI_PDISK_LABEL_CMD
    PI_PDISK_DEFRAG_CMD
    PI_PDISK_DEFRAG_STATUS_CMD
    PI_PDISK_FAIL_CMD
    PI_PDISK_BEACON_CMD
    PI_PDISK_UNFAIL_CMD
    PI_PDISK_DELETE_CMD
    PI_PDISK_BYPASS_CMD
    PI_PDISKS_QLOGIC_TIMEOUT_EMULATE
    PI_PDISKS_CMD
    PI_PDISKS_FROM_CACHE_CMD

    PI_VDISK_COUNT_CMD
    PI_VDISK_LIST_CMD
    PI_VDISK_INFO_CMD
    PI_VDISK_CREATE_CMD
    PI_VDISK_DELETE_CMD
    PI_VDISK_EXPAND_CMD
    PI_VDISK_CONTROL_CMD
    PI_QUICK_BREAK_PAUSE_RESUME_MIRROR_START_CMD     
    PI_QUICK_BREAK_PAUSE_RESUME_MIRROR_SEQUENCE_CMD   
    PI_QUICK_BREAK_PAUSE_RESUME_MIRROR_EXECUTE_CMD
    PI_BATCH_SNAPSHOT_START_CMD         
    PI_BATCH_SNAPSHOT_SEQUENCE_CMD      
    PI_BATCH_SNAPSHOT_EXECUTE_CMD       
    PI_BEACON_ISE_COMPONENT
    PI_VDISK_PREPARE_CMD
    PI_VDISK_SET_PRIORITY_CMD
    PI_VDISK_OWNER_CMD
    PI_VDISK_SET_ATTRIBUTE_CMD
    PI_VDISKS_CMD
    PI_VDISKS_FROM_CACHE_CMD

    PI_SERVER_COUNT_CMD
    PI_SERVER_LIST_CMD
    PI_SERVER_INFO_CMD
    PI_SERVER_CREATE_CMD
    PI_SERVER_DELETE_CMD
    PI_SERVER_ASSOCIATE_CMD
    PI_SERVER_DISASSOCIATE_CMD
    PI_SERVER_SET_PROPERTIES_CMD
    PI_SERVER_LOOKUP_CMD
    PI_SERVERS_CMD
    PI_SERVER_WWN_TO_TARGET_MAP_CMD

    PI_VLINK_REMOTE_CTRL_COUNT_CMD
    PI_VLINK_REMOTE_CTRL_INFO_CMD
    PI_VLINK_REMOTE_CTRL_VDISKS_CMD
    PI_VLINK_CREATE_CMD
    PI_VLINK_INFO_CMD
    PI_VLINK_BREAK_LOCK_CMD
    PI_VLINK_NAME_CHANGED_CMD 
    PI_VLINK_DLINK_INFO_CMD 
    PI_VLINK_DLOCK_INFO_CMD 
    PI_VLINK_DLINK_GT2TB_INFO_CMD 

    PI_TARGET_COUNT_CMD
    PI_TARGET_LIST_CMD
    PI_TARGET_INFO_CMD
    PI_TARGET_SET_PROPERTIES_CMD
    PI_TARGET_RESOURCE_LIST_CMD
    PI_TARGET_MOVE_CMD
    PI_TARGETS_CMD

    PI_STATS_GLOBAL_CACHE_CMD
    PI_STATS_CACHE_DEVICE_CMD
    PI_STATS_FRONT_END_PROC_CMD
    PI_STATS_BACK_END_PROC_CMD
    PI_STATS_FRONT_END_LOOP_CMD
    PI_STATS_BACK_END_LOOP_CMD
    PI_STATS_FRONT_END_PCI_CMD
    PI_STATS_BACK_END_PCI_CMD
    PI_STATS_SERVER_CMD
    PI_STATS_VDISK_CMD
    PI_STATS_PROC_CMD
    PI_STATS_PCI_CMD
    PI_STATS_CACHE_DEVICES_CMD
    PI_STATS_ENVIRONMENTAL_CMD
    PI_STATS_CONFIGURATION_MEMORY
    PI_STATS_HAB_CMD

    PI_RAID_COUNT_CMD
    PI_RAID_LIST_CMD
    PI_RAID_INFO_CMD
    PI_RAID_INIT_CMD
    PI_RAID_CONTROL_CMD
    PI_RAID_MIRRORING_CMD
    PI_RAID_RECOVER_CMD
    PI_RAIDS_CMD
    PI_RAIDS_FROM_CACHE_CMD

    PI_ADMIN_FW_VERSIONS_CMD
    PI_ADMIN_FW_SYS_REL_LEVEL_CMD
    PI_ADMIN_SETTIME_CMD
    PI_ADMIN_LEDCNTL_CMD
    PI_ADMIN_SET_IP_CMD
    PI_ADMIN_GET_IP_CMD
    PI_ADMIN_GETTIME_CMD 

    PI_DEBUG_MEM_RDWR_CMD
    PI_DEBUG_REPORT_CMD
    PI_DEBUG_INIT_PROC_NVRAM_CMD
    PI_DEBUG_INIT_CCB_NVRAM_CMD
    PI_DEBUG_GET_SER_NUM_CMD
    PI_DEBUG_MRMMTEST_CMD
    PI_DEBUG_GET_NAME_CMD
    PI_DEBUG_WRITE_NAME_CMD
    PI_DEBUG_STRUCT_DISPLAY_CMD
    PI_DEBUG_GET_ELECTION_STATE_CMD
    PI_DEBUG_SCSI_COMMAND_CMD
    PI_DEBUG_BE_LOOP_PRIMITIVE_CMD
    PI_DEBUG_FE_LOOP_PRIMITIVE_CMD
    PI_DEBUG_GET_STATE_RM_CMD
    PI_DEBUG_READWRITE_CMD
    
    PI_VCG_VALIDATION_CMD
    PI_PDISK_SPINDOWN_CMD
    PI_VCG_PREPARE_SLAVE_CMD
    PI_VCG_ADD_SLAVE_CMD
    PI_VCG_PING_CMD
    PI_VCG_INFO_CMD
    PI_VCG_INACTIVATE_CONTROLLER_CMD
    PI_VCG_ACTIVATE_CONTROLLER_CMD
    PI_VCG_SET_CACHE_CMD
    PI_VCG_GET_MP_LIST_CMD
    PI_GET_CPUCOUNT_CMD
    PI_GET_BACKEND_TYPE_CMD
    PI_ENABLE_X1_PORT_CMD
    PI_PDISK_FAILBACK_CMD
    PI_VCG_APPLY_LICENSE_CMD
    PI_VCG_UNFAIL_CONTROLLER_CMD
    PI_VCG_FAIL_CONTROLLER_CMD
    PI_VCG_REMOVE_CONTROLLER_CMD
    PI_VCG_SHUTDOWN_CMD

    PI_DISK_BAY_COUNT_CMD
    PI_DISK_BAY_LIST_CMD
    PI_DISK_BAY_INFO_CMD
    PI_DISK_BAY_SET_NAME_CMD
    PI_DISK_BAY_DELETE_CMD
    PI_DISK_BAY_ALARM_CTRL_CMD
    PI_DISK_BAY_LED_CTRL_CMD
    PI_DISK_BAYS_CMD
    PI_MISC_COUNT_CMD
    PI_MISC_LIST_CMD

    PI_PDISK_AUTO_FAILBACK_ENABLE_DISABLE_CMD
    PI_SET_GEO_LOCATION_CMD
    PI_CLEAR_GEO_LOCATION_CMD
    PI_ENVIRO_DATA_DISK_BAY_CMD
    PI_ENVIRO_DATA_CTRL_AND_BAY_CMD
    PI_VCG_CONFIGURE_CMD
    PI_ENV_II_GET_CMD
    PI_ISE_GET_STATUS_CMD

    PI_SNAPSHOT_READDIR_CMD
    PI_SNAPSHOT_TAKE_CMD
    PI_SNAPSHOT_LOAD_CMD
    PI_SNAPSHOT_CHANGE_CMD

    PI_FIRMWARE_DOWNLOAD_CMD
    PI_LOG_INFO_CMD
    PI_LOG_CLEAR_CMD
    PI_WRITE_BUFFER_MODE5_CMD
    PI_TRY_CCB_FW_CMD
    PI_ROLLING_UPDATE_PHASE_CMD
    PI_LOG_TEXT_MESSAGE_CMD
    PI_MULTI_PART_XFER_CMD
    PI_CUSTOMER_LOG_ACKNOWLEDGE_CMD 

    PI_GENERIC_CMD
    PI_GENERIC2_CMD
    PI_GENERIC_MRP_CMD

    PI_PROC_RESTORE_NVRAM_CMD
    PI_PROC_RESET_FE_QLOGIC_CMD
    PI_PROC_RESET_BE_QLOGIC_CMD
    PI_PROC_START_IO_CMD
    PI_PROC_STOP_IO_CMD
    PI_PROC_ASSIGN_MIRROR_PARTNER_CMD
    PI_PROC_BE_PORT_LIST_CMD
    PI_PROC_FE_PORT_LIST_CMD
    PI_PROC_BE_DEVICE_PATH_CMD
    PI_PROC_NAME_DEVICE_CMD
    PI_PROC_FAIL_CTRL_CMD
    PI_PROC_FAIL_PORT_CMD

    PI_PERSISTENT_DATA_CONTROL_CMD
    PI_CLIENT_PERSISTENT_DATA_CONTROL_CMD 
    
    PI_MISC_GET_DEVICE_COUNT_CMD
    PI_MISC_RESCAN_DEVICE_CMD
    PI_MISC_GET_BE_DEVICE_LIST_CMD
    PI_MISC_GET_FE_DEVICE_LIST_CMD
    PI_MISC_FILE_SYSTEM_READ_CMD
    PI_MISC_FILE_SYSTEM_WRITE_CMD
    PI_MISC_FAILURE_STATE_SET_CMD
    PI_MISC_GET_MODE_CMD
    PI_MISC_SET_MODE_CMD
    PI_MISC_UNFAIL_INTERFACE_CMD
    PI_MISC_FAIL_INTERFACE_CMD
    PI_MISC_SERIAL_NUMBER_SET_CMD
    PI_MISC_RESYNC_MIRROR_RECORDS_CMD
    PI_MISC_CONTINUE_WO_MP_CMD         
    PI_MISC_INVALIDATE_BE_WC_CMD       
    PI_MISC_MIRROR_PARTNER_CONTROL_CMD 

    PI_MISC_GET_WORKSET_INFO_CMD        
    PI_MISC_SET_WORKSET_INFO_CMD  
    PI_UNUSED1
    PI_UNUSED2
    PI_CACHE_REFRESH_CCB_CMD
    PI_SET_DLM_HEARTBEAT_LIST_CMD
    PI_CACHE_FLUSH_BE_CMD
    PI_MISC_RESYNC_RAIDS_CMD
    PI_MISC_PUTDEVCONFIG_CMD
    PI_MISC_GETDEVCONFIG_CMD
    PI_STATS_BUFFER_BOARD_CMD
    PI_MISC_MIRROR_PARTNER_GET_CFG_CMD
    PI_BATTERY_HEALTH_SET_CMD
    PI_STATS_SERVERS_CMD
    
    PI_VCG_SET_VDISK_PRIORITY_CMD
    
    PI_VDISK_PR_GET_CMD
    PI_VDISK_PR_CLR_CMD

    PI_MISC_QUERY_MP_CHANGE_CMD
    PI_MISC_RESYNCDATA_CMD
    PI_MISC_RESYNCCTL_CMD
    PI_MISC_SETTDISCACHE_CMD
    PI_MISC_CLRTDISCACHE_CMD
    PI_MISC_QTDISCACHE_CMD
    PI_MISC_CFGOPTION_CMD
    PI_ISCSI_SET_TGTPARAM 
    PI_ISCSI_TGT_INFO
    PI_ISCSI_SET_CHAP 
    PI_ISCSI_CHAP_INFO 
    PI_ISCSI_SESSION_INFO 
    PI_ISCSI_SESSION_INFO_SERVER
    PI_GETISNSINFO_CMD
    PI_IDD_INFO_CMD 
    PI_SETISNSINFO_CMD               
    PI_DLM_PATH_STATS_CMD 
    PI_DLM_PATH_SELECTION_ALGO_CMD
    PI_GET_CPUCOUNT_CMD
    PI_GET_BACKEND_TYPE_CMD
    PI_VDISK_BAY_REDUNDANT_CMD      
    PI_CLEAR_GEO_LOCATION_CMD
    PI_MFG_CTRL_CLEAN_CMD
    PI_LOG_EVENT_MESSAGE 
    PI_ASYNC_CHANGED_EVENT 
    PI_ASYNC_PING_EVENT
    PI_REGISTER_CLIENT_TYPE_CMD

    CFG_VDISK_DELETE

    CFG_VDISK_ADD_RAID_0
    CFG_VDISK_ADD_RAID_5
    CFG_VDISK_ADD_RAID_10

    CFG_VDISK_EXP_RAID_0
    CFG_VDISK_EXP_RAID_5
    CFG_VDISK_EXP_RAID_10

    CFG_VDISK_VDISK_MOVE
    CFG_VDISK_VDISK_COPY
    CFG_VDISK_VDISK_SWAP
    CFG_VDISK_VDISK_MIRROR
    CFG_VDISK_VDISK_BREAK_MIRROR
    CFG_VDISK_VDISK_COPY_PAUSE
    CFG_VDISK_VDISK_COPY_RESUME
    CFG_VDISK_VDISK_COPY_ABORT

    CFG_VDISK_SET_ATTRIBUTE
    CFG_VDISK_SET_LOCK
    CFG_VDISK_ERASE_VDISK

    CFG_VDISK_SET_SERVER_NAME
    CFG_VDISK_SET_MASK
    CFG_VDISK_SET_LUN
    CFG_VDISK_SET_DEFMODE
    CFG_VDISK_VDISK_SET_NAME
    CFG_VDISK_MAG_RSVD_1
    CFG_VDISK_VDISK_SET_CACHE_OFF
    CFG_VDISK_SELECT_HAB_FOR_SERVER
    CFG_VDISK_VLINK_BREAK
    CFG_VDISK_VLINK_CREATE
    CFG_VDISK_VDISK_SET_CACHE_ON
    CFG_VDISK_SELECT_TARGET
    CFG_VDISK_ASSIGN_VBLOCK
    CFG_VDISK_SET_WORKSET_NAME
    CFG_VDISK_SET_VPORT
    CFG_VDISK_SERVER_IN_WORKSET

    CFG_VDISK_PDISK_LABEL
    CFG_VDISK_PDISK_SPINDOWN
    CFG_VDISK_PDISK_DEFRAG
    CFG_VDISK_PDISK_SCRUB
    CFG_VDISK_PDISK_FAIL
    CFG_VDISK_PDISK_UNFAIL
    CFG_VDISK_PDISK_BEACON
    CFG_VDISK_PDISK_DELETE
    CFG_VDISK_LOG_ACKNOWLEDGE
    CFG_VDISK_DELETE_SERVER
    CFG_VDISK_OP_SET_PRIORITY
    CFG_VDISK_RAID_RECOVER
    CFG_VDISK_SET_GLOBAL_CACHE_ON 
    CFG_VDISK_SET_GLOBAL_CACHE_OFF
);

use XIOTech::logMgr;
use strict;

###############################################################################
# Packet Version Codes
use constant VERSION_1                                  => 0x0001;
use constant VERSION_2                                  => 0x0002;
use constant VERSION_3                                  => 0x0003;
# use constant COMPAT_INDEX_1                             => 0x0001;   UNUSED (3000)
# use constant COMPAT_INDEX_3                             => 0x0003;   UNUSED (750)
use constant COMPAT_INDEX_4                             => 0x0004;     # 4000

###############################################################################
# Command Codes
use constant PI_CONNECT_CMD                             => 0x0001;
use constant PI_DISCONNECT_CMD                          => 0x0002;
use constant PI_PING_CMD                                => 0x0003;
use constant PI_RESET_CMD                               => 0x0004;
use constant PI_POWER_UP_STATE_CMD                      => 0x0005;
use constant PI_POWER_UP_RESPONSE_CMD                   => 0x0006;

use constant PI_REGISTER_EVENTS_CMD                     => 0x0008;
use constant PI_REGISTER_CLIENT_TYPE_CMD                => 0x0009;
use constant PI_PDISK_COUNT_CMD                         => 0x0010;
use constant PI_PDISK_LIST_CMD                          => 0x0011;
use constant PI_PDISK_INFO_CMD                          => 0x0012;
use constant PI_PDISK_LABEL_CMD                         => 0x0013;
use constant PI_PDISK_DEFRAG_CMD                        => 0x0014;
use constant PI_PDISK_FAIL_CMD                          => 0x0015;
use constant PI_PDISK_BEACON_CMD                        => 0x0016;
use constant PI_PDISK_UNFAIL_CMD                        => 0x0017;
use constant PI_PDISK_DELETE_CMD                        => 0x0018;
use constant PI_PDISK_BYPASS_CMD                        => 0x0019;
use constant PI_PDISK_DEFRAG_STATUS_CMD                 => 0x001A;
use constant PI_PDISKS_QLOGIC_TIMEOUT_EMULATE           => 0x001B;
use constant PI_PDISKS_CMD                              => 0x001F;

use constant PI_VDISK_COUNT_CMD                         => 0x0020;
use constant PI_VDISK_LIST_CMD                          => 0x0021;
use constant PI_VDISK_INFO_CMD                          => 0x0022;
use constant PI_VDISK_CREATE_CMD                        => 0x0023;
use constant PI_VDISK_DELETE_CMD                        => 0x0024;
use constant PI_VDISK_EXPAND_CMD                        => 0x0025;
use constant PI_VDISK_CONTROL_CMD                       => 0x0026;
use constant PI_VDISK_PREPARE_CMD                       => 0x0027;
#VPRI_CODE
use constant PI_VDISK_SET_PRIORITY_CMD                  => 0x0028;
#VPRI_CODE
use constant PI_VDISK_OWNER_CMD                         => 0x0029;
use constant PI_VDISK_SET_ATTRIBUTE_CMD                 => 0x002A;
use constant PI_VDISKS_CMD                              => 0x002F;

use constant PI_SERVER_COUNT_CMD                        => 0x0030;
use constant PI_SERVER_LIST_CMD                         => 0x0031;
use constant PI_SERVER_INFO_CMD                         => 0x0032;
use constant PI_SERVER_CREATE_CMD                       => 0x0033;
use constant PI_SERVER_DELETE_CMD                       => 0x0034;
use constant PI_SERVER_ASSOCIATE_CMD                    => 0x0035;
use constant PI_SERVER_DISASSOCIATE_CMD                 => 0x0036;
use constant PI_SERVER_SET_PROPERTIES_CMD               => 0x0037;
use constant PI_SERVER_LOOKUP_CMD                       => 0x0038;
use constant PI_SERVER_WWN_TO_TARGET_MAP_CMD            => 0x0039;  
use constant PI_SERVERS_CMD                             => 0x003F;

use constant PI_VLINK_REMOTE_CTRL_COUNT_CMD             => 0x0040;
use constant PI_VLINK_REMOTE_CTRL_INFO_CMD              => 0x0041;
use constant PI_VLINK_REMOTE_CTRL_VDISKS_CMD            => 0x0042;
use constant PI_VLINK_CREATE_CMD                        => 0x0043;
use constant PI_VLINK_INFO_CMD                          => 0x0044;
use constant PI_VLINK_BREAK_LOCK_CMD                    => 0x0045;
use constant PI_VLINK_NAME_CHANGED_CMD                  => 0x0046;
use constant PI_VLINK_DLINK_INFO_CMD                    => 0x0047;
use constant PI_VLINK_DLOCK_INFO_CMD                    => 0x0048;
use constant PI_VLINK_DLINK_GT2TB_INFO_CMD              => 0x0049;

use constant PI_TARGET_COUNT_CMD                        => 0x0050;
use constant PI_TARGET_LIST_CMD                         => 0x0051;
use constant PI_TARGET_INFO_CMD                         => 0x0052;
use constant PI_TARGET_SET_PROPERTIES_CMD               => 0x0053;
use constant PI_TARGET_RESOURCE_LIST_CMD                => 0x0054;
use constant PI_TARGET_MOVE_CMD                         => 0x0055;
use constant PI_TARGETS_CMD                             => 0x005F;

use constant PI_STATS_GLOBAL_CACHE_CMD                  => 0x0060;
use constant PI_STATS_CACHE_DEVICE_CMD                  => 0x0061;
use constant PI_STATS_FRONT_END_PROC_CMD                => 0x0062;
use constant PI_STATS_BACK_END_PROC_CMD                 => 0x0063;
use constant PI_STATS_FRONT_END_LOOP_CMD                => 0x0064;
use constant PI_STATS_BACK_END_LOOP_CMD                 => 0x0065;
use constant PI_STATS_FRONT_END_PCI_CMD                 => 0x0066;
use constant PI_STATS_BACK_END_PCI_CMD                  => 0x0067;
use constant PI_STATS_SERVER_CMD                        => 0x0068;
use constant PI_STATS_VDISK_CMD                         => 0x0069; 
use constant PI_STATS_PROC_CMD                          => 0x006A;
use constant PI_STATS_PCI_CMD                           => 0x006B;
use constant PI_STATS_CACHE_DEVICES_CMD                 => 0x006C;
use constant PI_STATS_ENVIRONMENTAL_CMD                 => 0x006D;
use constant PI_STATS_CONFIGURATION_MEMORY              => 0x006E;
use constant PI_STATS_HAB_CMD                           => 0x006F;

use constant PI_RAID_COUNT_CMD                          => 0x0070; 
use constant PI_RAID_LIST_CMD                           => 0x0071; 
use constant PI_RAID_INFO_CMD                           => 0x0072; 
use constant PI_RAID_INIT_CMD                           => 0x0073; 
use constant PI_RAID_CONTROL_CMD                        => 0x0074;
use constant PI_RAID_MIRRORING_CMD                      => 0x0075;
use constant PI_RAID_RECOVER_CMD                        => 0x0076;
use constant PI_RAIDS_CMD                               => 0x007F;

use constant PI_ADMIN_FW_VERSIONS_CMD                   => 0x0080;
use constant PI_ADMIN_FW_SYS_REL_LEVEL_CMD              => 0x0081;
use constant PI_ADMIN_SETTIME_CMD                       => 0x0082;
use constant PI_ADMIN_LEDCNTL_CMD                       => 0x0083;
use constant PI_ADMIN_SET_IP_CMD                        => 0x0084;
use constant PI_ADMIN_GET_IP_CMD                        => 0x0085;
use constant PI_ADMIN_GETTIME_CMD                       => 0x0086;


use constant PI_DEBUG_MEM_RDWR_CMD                      => 0x0090;
use constant PI_DEBUG_REPORT_CMD                        => 0x0091;
use constant PI_DEBUG_INIT_PROC_NVRAM_CMD               => 0x0092;
use constant PI_DEBUG_INIT_CCB_NVRAM_CMD                => 0x0093;
use constant PI_DEBUG_GET_SER_NUM_CMD                   => 0x0094;
use constant PI_DEBUG_MRMMTEST_CMD                      => 0x0095;
use constant PI_DEBUG_GET_NAME_CMD                      => 0x0096; 
use constant PI_DEBUG_WRITE_NAME_CMD                    => 0x0097; 
use constant PI_DEBUG_STRUCT_DISPLAY_CMD                => 0x0098;
use constant PI_DEBUG_GET_ELECTION_STATE_CMD            => 0x0099;
use constant PI_DEBUG_SCSI_COMMAND_CMD                  => 0x009A;
use constant PI_DEBUG_BE_LOOP_PRIMITIVE_CMD             => 0x009B;
use constant PI_DEBUG_FE_LOOP_PRIMITIVE_CMD             => 0x009C;
use constant PI_DEBUG_GET_STATE_RM_CMD                  => 0x009D;
use constant PI_DEBUG_READWRITE_CMD                     => 0x009E;

use constant PI_VCG_VALIDATION_CMD                      => 0x00A0;
#SERVICEABILITY42
use constant PI_PDISK_SPINDOWN_CMD                      => 0x00A1;
#SERVICEABILITY42
use constant PI_VCG_PREPARE_SLAVE_CMD                   => 0x00A2;
use constant PI_VCG_ADD_SLAVE_CMD                       => 0x00A3;
use constant PI_VCG_PING_CMD                            => 0x00A4;
use constant PI_VCG_INFO_CMD                            => 0x00A5;
use constant PI_VCG_INACTIVATE_CONTROLLER_CMD           => 0x00A6;
use constant PI_VCG_ACTIVATE_CONTROLLER_CMD             => 0x00A7;
use constant PI_VCG_SET_CACHE_CMD                       => 0x00A8;
use constant PI_VCG_GET_MP_LIST_CMD                     => 0x00A9;
#SERVICEABILITY42
use constant PI_PDISK_FAILBACK_CMD                      => 0x00AA;
#SERVICEABILITY42                                                 
use constant PI_VCG_APPLY_LICENSE_CMD                   => 0x00AB;
use constant PI_VCG_UNFAIL_CONTROLLER_CMD               => 0x00AC;
use constant PI_VCG_FAIL_CONTROLLER_CMD                 => 0x00AD;
use constant PI_VCG_REMOVE_CONTROLLER_CMD               => 0x00AE;
use constant PI_VCG_SHUTDOWN_CMD                        => 0x00AF;

use constant PI_DISK_BAY_COUNT_CMD                      => 0x00B0;
use constant PI_DISK_BAY_LIST_CMD                       => 0x00B1;
use constant PI_DISK_BAY_INFO_CMD                       => 0x00B2;
use constant PI_DISK_BAY_SET_NAME_CMD                   => 0x00B3;
use constant PI_DISK_BAY_DELETE_CMD                     => 0x00B4;
use constant PI_DISK_BAY_ALARM_CTRL_CMD                 => 0x00B5;
use constant PI_DISK_BAY_LED_CTRL_CMD                   => 0x00B6;
#GR_GEORAID
use constant PI_SET_GEO_LOCATION_CMD                    => 0x00B7;
use constant PI_MISC_COUNT_CMD                          => 0x00B8;
use constant PI_MISC_LIST_CMD                           => 0x00B9;
use constant PI_DISK_BAYS_CMD                           => 0x00BF;

#SERVICEABILITY42
use constant PI_PDISK_AUTO_FAILBACK_ENABLE_DISABLE_CMD  => 0x00C0; 
use constant PI_ENVIRO_DATA_DISK_BAY_CMD                => 0x00C1; 
use constant PI_ENVIRO_DATA_CTRL_AND_BAY_CMD            => 0x00C2; 
use constant PI_VCG_CONFIGURE_CMD                       => 0x00C3;
use constant PI_ENV_II_GET_CMD                          => 0x00C4;
use constant PI_ISE_GET_STATUS_CMD                      => 0x00C5;

use constant PI_SNAPSHOT_READDIR_CMD                    => 0x00D0;
use constant PI_SNAPSHOT_TAKE_CMD                       => 0x00D1;
use constant PI_SNAPSHOT_LOAD_CMD                       => 0x00D2;
use constant PI_SNAPSHOT_CHANGE_CMD                     => 0x00D3;

use constant PI_FIRMWARE_DOWNLOAD_CMD                   => 0x00E0;
use constant PI_LOG_INFO_CMD                            => 0x00E1;
use constant PI_LOG_CLEAR_CMD                           => 0x00E2;
use constant PI_WRITE_BUFFER_MODE5_CMD                  => 0x00E3;
use constant PI_TRY_CCB_FW_CMD                          => 0x00E4;
use constant PI_ROLLING_UPDATE_PHASE_CMD                => 0x00E5;
use constant PI_LOG_TEXT_MESSAGE_CMD                    => 0x00E6;
use constant PI_MULTI_PART_XFER_CMD                     => 0x00E7;
use constant PI_CUSTOMER_LOG_ACKNOWLEDGE_CMD            => 0x00E8;

use constant PI_GENERIC_CMD                             => 0x00F0;
use constant PI_GENERIC2_CMD                            => 0x00F1;
use constant PI_GENERIC_MRP_CMD                         => 0x00F2;

use constant PI_PROC_RESTORE_NVRAM_CMD                  => 0x0100;
use constant PI_PROC_RESET_FE_QLOGIC_CMD                => 0x0101;
use constant PI_PROC_RESET_BE_QLOGIC_CMD                => 0x0102;
use constant PI_PROC_START_IO_CMD                       => 0x0103;
use constant PI_PROC_STOP_IO_CMD                        => 0x0104;
use constant PI_PROC_ASSIGN_MIRROR_PARTNER_CMD          => 0x0105;
use constant PI_PROC_BE_PORT_LIST_CMD                   => 0x0106;
use constant PI_PROC_FE_PORT_LIST_CMD                   => 0x0107;
use constant PI_PROC_BE_DEVICE_PATH_CMD                 => 0x0108;
use constant PI_PROC_NAME_DEVICE_CMD                    => 0x0109;
use constant PI_PROC_FAIL_CTRL_CMD                      => 0x010A;
use constant PI_PROC_FAIL_PORT_CMD                      => 0x010B;

use constant PI_PERSISTENT_DATA_CONTROL_CMD             => 0x0110;
use constant PI_CLIENT_PERSISTENT_DATA_CONTROL_CMD      => 0x0111;

use constant PI_MISC_GET_DEVICE_COUNT_CMD               => 0x0200;
use constant PI_MISC_RESCAN_DEVICE_CMD                  => 0x0201;
use constant PI_MISC_GET_BE_DEVICE_LIST_CMD             => 0x0202;
use constant PI_MISC_GET_FE_DEVICE_LIST_CMD             => 0x0203;
use constant PI_MISC_FILE_SYSTEM_READ_CMD               => 0x0204;
use constant PI_MISC_FILE_SYSTEM_WRITE_CMD              => 0x0205;
use constant PI_MISC_FAILURE_STATE_SET_CMD              => 0x0206;
use constant PI_MISC_GET_MODE_CMD                       => 0x0207;
use constant PI_MISC_SET_MODE_CMD                       => 0x0208;
use constant PI_MISC_UNFAIL_INTERFACE_CMD               => 0x0209;
use constant PI_MISC_FAIL_INTERFACE_CMD                 => 0x020A;
use constant PI_MISC_SERIAL_NUMBER_SET_CMD              => 0x020B;
use constant PI_MISC_RESYNC_MIRROR_RECORDS_CMD          => 0x020C;
use constant PI_MISC_CONTINUE_WO_MP_CMD                 => 0x020D;
use constant PI_MISC_INVALIDATE_BE_WC_CMD               => 0x020E;
use constant PI_MISC_MIRROR_PARTNER_CONTROL_CMD         => 0x020F;

use constant PI_MISC_GET_WORKSET_INFO_CMD               => 0x0210;
use constant PI_MISC_SET_WORKSET_INFO_CMD               => 0x0211;
use constant PI_UNUSED1                                 => 0x0212;
use constant PI_UNUSED2                                 => 0x0213;
use constant PI_CACHE_REFRESH_CCB_CMD                   => 0x0214;
use constant PI_SET_DLM_HEARTBEAT_LIST_CMD              => 0x0215;
use constant PI_CACHE_FLUSH_BE_CMD                      => 0x0216;
use constant PI_MISC_RESYNC_RAIDS_CMD                   => 0x0217;
use constant PI_MISC_PUTDEVCONFIG_CMD                   => 0x0218;
use constant PI_MISC_GETDEVCONFIG_CMD                   => 0x0219;
use constant PI_STATS_BUFFER_BOARD_CMD                  => 0x021A;
use constant PI_MISC_MIRROR_PARTNER_GET_CFG_CMD         => 0x021B;
use constant PI_BATTERY_HEALTH_SET_CMD                  => 0x021C;
use constant PI_MISC_INVALIDATE_FE_WC_CMD               => 0x021D;
use constant PI_STATS_SERVERS_CMD                       => 0x021E;
#VPRI_CODE
use constant PI_VCG_SET_VDISK_PRIORITY_CMD              => 0x021F;
#VPRI_CODE

use constant PI_VDISK_PR_GET_CMD                        => 0x002B;
use constant PI_VDISK_PR_CLR_CMD                        => 0x002C;

use constant PI_MISC_QUERY_MP_CHANGE_CMD                => 0x0220;
use constant PI_MISC_RESYNCDATA_CMD                     => 0x0221;
use constant PI_MISC_RESYNCCTL_CMD                      => 0x0222;
use constant PI_MISC_SETTDISCACHE_CMD                   => 0x0223;
use constant PI_MISC_CLRTDISCACHE_CMD                   => 0x0224;
use constant PI_MISC_QTDISCACHE_CMD                     => 0x0225;
use constant PI_MISC_CFGOPTION_CMD                      => 0x0226;
# ISCSI
use constant PI_ISCSI_SET_TGTPARAM                      => 0x0229;
use constant PI_ISCSI_TGT_INFO                          => 0x0231;
use constant PI_ISCSI_SET_CHAP                          => 0x0232;
use constant PI_ISCSI_CHAP_INFO                         => 0x0233;
use constant PI_ISCSI_SESSION_INFO                      => 0x0235;
use constant PI_ISCSI_SESSION_INFO_SERVER               => 0x0236;
use constant PI_GETISNSINFO_CMD                         => 0x0237;
use constant PI_IDD_INFO_CMD                            => 0x0238;
use constant PI_SETISNSINFO_CMD                         => 0x0239;
use constant PI_DLM_PATH_STATS_CMD                      => 0x0240;
use constant PI_DLM_PATH_SELECTION_ALGO_CMD             => 0x0241;
use constant PI_GET_CPUCOUNT_CMD                        => 0x0243;
## use constant PI_ENABLE_X1_PORT_CMD                      => 0x0244;   OBSOLETE
use constant PI_VDISK_BAY_REDUNDANT_CMD                 => 0x0245;
use constant PI_GET_BACKEND_TYPE_CMD                    => 0x0246;
use constant PI_QUICK_BREAK_PAUSE_RESUME_MIRROR_START_CMD      => 0x0247;
use constant PI_QUICK_BREAK_PAUSE_RESUME_MIRROR_SEQUENCE_CMD   => 0x0248;
use constant PI_QUICK_BREAK_PAUSE_RESUME_MIRROR_EXECUTE_CMD    => 0x0249;
use constant PI_BEACON_ISE_COMPONENT                    => 0x024A;
use constant PI_PDISKS_FROM_CACHE_CMD                   => 0x024B;
use constant PI_VDISKS_FROM_CACHE_CMD                   => 0x024C;
use constant PI_RAIDS_FROM_CACHE_CMD                    => 0x024D;
use constant PI_BATCH_SNAPSHOT_START_CMD                => 0x024E;
use constant PI_BATCH_SNAPSHOT_SEQUENCE_CMD             => 0x024F;
use constant PI_BATCH_SNAPSHOT_EXECUTE_CMD              => 0x0250;


# ISCSI

#GR_GEORAID
use constant PI_CLEAR_GEO_LOCATION_CMD                  => 0x0234;
use constant PI_MFG_CTRL_CLEAN_CMD                      => 0x0300;

use constant PI_LOG_EVENT_MESSAGE                       => 0x0500;
use constant PI_ASYNC_CHANGED_EVENT                     => 0x0501;
use constant PI_ASYNC_PING_EVENT                        => 0x0502;

#
# ----- X1 Request packet types start here -----
#
## use constant X1PKT_GET_DLINK                    => 0x10;     OBSOLETE
## use constant X1PKT_GET_DLOCK_INFO               => 0x11;     OBSOLETE
## use constant X1PKT_GET_SU_LIST                  => 0x12;     OBSOLETE
## use constant X1PKT_GET_SU_LUN_INFO              => 0x13;     OBSOLETE

## use constant X1PKT_GET_ENVIRON                  => 0x20;     OBSOLETE
## use constant X1PKT_GET_PMAP                     => 0x21;     OBSOLETE
## use constant X1PKT_GET_RMAP                     => 0x22;     OBSOLETE
## use constant X1PKT_GET_VMAP                     => 0x23;     OBSOLETE
## use constant X1PKT_GET_HMAP                     => 0x24;     OBSOLETE
## use constant X1PKT_GET_PSTATS                   => 0x25;
## use constant X1PKT_GET_BSTATS                   => 0x26;
## use constant X1PKT_GET_RSTATS                   => 0x27;
## use constant X1PKT_GET_VSTATS                   => 0x28;
## use constant X1PKT_GET_PINFO                    => 0x29;
## use constant X1PKT_GET_RINFO                    => 0x2A;
## use constant X1PKT_GET_VINFO                    => 0x2B;
## use constant X1PKT_GET_HINFO                    => 0x2C;
## use constant X1PKT_GET_ISALIVE                  => 0x2D;
## use constant X1PKT_GET_PSTATE                   => 0x2F;

## use constant X1PKT_GET_STATES                   => 0x30;
## use constant X1PKT_GET_DNAME                    => 0x31;
## ## use constant X1PKT_GET_CPU_LOAD                 => 0x32;     OBSOLETE
## ## use constant X1PKT_GET_FREE                     => 0x35;     OBSOLETE
## use constant X1PKT_WHO_INCOPY                   => 0x38;
## use constant X1PKT_GET_COPY_INFO                => 0x39;
## use constant X1PKT_SERVER_MAP                   => 0x3A;     OBSOLETE
## use constant X1PKT_GET_MASK_INFO                => 0x3B;     OBSOLETE
## use constant X1PKT_GET_LUNS_INFO                => 0x3C;     OBSOLETE
## use constant X1PKT_GET_COPY_STATUS              => 0x3D;     OBSOLETE
## use constant X1PKT_GET_MIRROR_STATUS            => 0x3E;     OBSOLETE

## use constant X1PKT_PCHANGED                     => 0x40;
## use constant X1PKT_RCHANGED                     => 0x41;
## use constant X1PKT_VCHANGED                     => 0x42;
## use constant X1PKT_HCHANGED                     => 0x43;
## use constant X1PKT_GET_CONFIG                   => 0x44;
## use constant X1PKT_VCG_CHANGED                  => 0x45;
## use constant X1PKT_ACHANGED                     => 0x46;
## use constant X1PKT_ZCHANGED                     => 0x47;
## use constant X1PKT_READ_MEM                     => 0x48;
## use constant X1PKT_READ_DPRAM                   => 0x49;
## use constant X1PKT_VLINKED_TO                   => 0x4A;     OBSOLETE
## use constant X1PKT_GET_PERF                     => 0x4B;
## use constant X1PKT_ACCT_SET                     => 0x4C;     OBSOLETE
## use constant X1PKT_ACCT_GET                     => 0x4D;     OBSOLETE
## use constant X1PKT_ACCT_SELECT                  => 0x4E;
## use constant X1PKT_ACCT_LOGIN                   => 0x4F;

## use constant X1RPKT_LOG_ENTRY                   => 0x50;
## use constant X1PKT_SET_LOG                      => 0x51;     OBSOLETE
## use constant X1RPKT_CONFIG_VDISK_ACK            => 0x57;
## use constant X1PKT_CONFIG_VDISK                 => 0x58; OBSOLETE
## use constant X1PKT_CONFIG_HAB                   => 0x59;     OBSOLETE
## use constant X1PKT_PROBE                        => 0x5A;     OBSOLETE
## use constant X1RPKT_PROBE                       => 0x5A;     OBSOLETE
## use constant X1PKT_DISCONNECT                   => 0x5C;
## use constant X1PKT_LOG_ENTRY                    => 0x5D;     OBSOLETE
## use constant X1PKT_SECURE_LOGIN                 => 0x5E;
## use constant X1PKT_GET_MS                       => 0x5F;

## use constant X1PKT_GET_VCG_INFO                 => 0x60;     OBSOLETE
## use constant X1PKT_GET_VERSION_INFO             => 0x61;
## use constant X1PKT_GET_BAY_MAP                  => 0x62;     OBSOLETE
## use constant X1PKT_GET_BAY_INFO                 => 0x63;
## use constant X1PKT_GET_VPORT_MAP                => 0x64;     OBSOLETE
## use constant X1PKT_GET_VPORT_INFO               => 0x65;     OBSOLETE
## use constant X1PKT_GET_ELEC_SIG_INFO            => 0x66;     OBSOLETE
## use constant X1PKT_GET_BE_LOOP_INFO             => 0x67;     OBSOLETE
## use constant X1PKT_GET_WORKSET_INFO             => 0x68;     OBSOLETE
## use constant X1PKT_UNUSED1                      => 0x69;
## use constant X1PKT_UNUSED2                      => 0x6A;
## use constant X1PKT_PUT_DEV_CONFIG               => 0x6B;     OBSOLETE
## use constant X1PKT_GET_DEV_CONFIG               => 0x6C;     OBSOLETE
## use constant X1PKT_GET_SERVER_STATS             => 0x6D;     OBSOLETE
## use constant X1PKT_GET_HAB_STATS                => 0x6E;     OBSOLETE
## use constant X1PKT_GET_DEFRAG_STATUS            => 0x6F;     OBSOLETE

## use constant X1PKT_GET_RESYNC_INFO              => 0x70;     OBSOLETE
## use constant X1PKT_RESYNC_CONTROL               => 0x71;     OBSOLETE
## use constant X1PKT_LICENSE_CONFIG               => 0x72;     OBSOLETE
## use constant X1PKT_GET_MIRROR_IO_STATUS         => 0x73;     OBSOLETE

## use constant X1PKT_BIGFOOT_CMD                  => 0xBF;
## use constant X1RPKT_BIGFOOT_CMD                 => 0xBF;

#
# ----- X1 Response packets 
#
## use constant X1RPKT_GET_DLINK                   => 0x90;     OBSOLETE
## use constant X1RPKT_GET_DLOCK_INFO              => 0x91;     OBSOLETE
## use constant X1PKT_REPLY_SU_CNT                 => 0x9A;     OBSOLETE
## use constant X1PKT_REPLY_SU_ITEM                => 0x9B;
## use constant X1PKT_REPLY_LUN_CNT                => 0x9C;     OBSOLETE
## use constant X1PKT_REPLY_LUN_ITEM               => 0x9D;

## use constant X1RPKT_GET_ENVIRON                 => 0xA0;
## use constant X1RPKT_GET_PMAP                    => 0xA1;     OBSOLETE
## use constant X1RPKT_GET_RMAP                    => 0xA2;     OBSOLETE
## use constant X1RPKT_GET_VMAP                    => 0xA3;     OBSOLETE
## use constant X1RPKT_GET_HMAP                    => 0xA4;     OBSOLETE
## use constant X1RPKT_GET_PSTATS                  => 0xA5;
## use constant X1RPKT_GET_BSTATS                  => 0xA6;
## use constant X1RPKT_GET_RSTATS                  => 0xA7;
## use constant X1RPKT_GET_VSTATS                  => 0xA8;
## use constant X1RPKT_GET_PINFO                   => 0xA9;
## use constant X1RPKT_GET_RINFO                   => 0xAA;
## use constant X1RPKT_GET_VINFO                   => 0xAB;
## use constant X1RPKT_GET_HINFO                   => 0xAC;
## use constant X1RPKT_GET_ISALIVE                 => 0xAD;
## use constant X1RPKT_GET_RAIDPER                 => 0xAE;
## use constant X1RPKT_GET_PSTATE                  => 0xAF;

## use constant X1RPKT_GET_STATES                  => 0xB0;
## use constant X1RPKT_GET_DNAME                   => 0xB1;
## use constant X1RPKT_GET_CPU_LOAD                => 0xB2;     OBSOLETE
## use constant X1RPKT_GET_FREE                    => 0xB5;     OBSOLETE
## use constant X1RPKT_WHO_INCOPY                  => 0xB8;
## use constant X1RPKT_GET_COPYINFO                => 0xB9;
## use constant X1RPKT_GET_COPY_INFO               => 0xB9;
## use constant X1RPKT_SERVER_MAP                  => 0xBA;     OBSOLETE
## use constant X1RPKT_GET_MASK_INFO               => 0xBB;     OBSOLETE
## use constant X1RPKT_GET_LUNS_INFO               => 0xBC;     OBSOLETE
## use constant X1RPKT_GET_COPY_STATUS             => 0xBD;     OBSOLETE
## use constant X1RPKT_GET_MIRROR_STATUS           => 0xBE;     OBSOLETE
## use constant X1RPKT_GET_MIRROR_IO_STATUS        => 0xF3;     OBSOLETE

## use constant X1RPKT_GET_CONFIG                  => 0xC4;     OBSOLETE
## use constant X1RPKT_READ_MEM                    => 0xC8;
## use constant X1RPKT_READ_DPRAM                  => 0xC9;
## use constant X1RPKT_VLINKED_TO                  => 0xCA;     OBSOLETE
## use constant X1RPKT_GET_PERF                    => 0xCB;
## use constant X1RPKT_ACCT_SET                    => 0xCC;
## use constant X1RPKT_ACCT_GET                    => 0xCD;
## use constant X1RPKT_ACCT_SELECT                 => 0xCE;
## use constant X1RPKT_ACCT_LOGIN                  => 0xCF;

## use constant X1RPKT_SET_LOG                     => 0xD1;
## use constant X1RPKT_CONFIG_VDISK                => 0xD8; OBSOLETE
## use constant X1RPKT_CONFIG_HAB                  => 0xD9;     OBSOLETE
## use constant X1RPKT_SECURE_LOGIN                => 0xDE;
## use constant X1RPKT_GET_MS                      => 0xDF;

## use constant X1RPKT_VCG_INFO                    => 0xE0;     OBSOLETE
## use constant X1RPKT_GET_VERSION_INFO            => 0xE1;
## use constant X1RPKT_GET_BAY_MAP                 => 0xE2;     OBSOLETE
## use constant X1RPKT_GET_BAY_INFO                => 0xE3;
## use constant X1RPKT_GET_VPORT_MAP               => 0xE4;     OBSOLETE
## use constant X1RPKT_GET_VPORT_INFO              => 0xE5;     OBSOLETE
## use constant X1RPKT_GET_ELEC_SIG_INFO           => 0xE6;
## use constant X1RPKT_GET_BE_LOOP_INFO            => 0xE7;
## use constant X1RPKT_GET_WORKSET_INFO            => 0xE8;     OBSOLETE
## use constant X1RPKT_UNUSED1                     => 0xE9;
## use constant X1RPKT_UNUSED2                     => 0xEA;
## use constant X1RPKT_PUT_DEV_CONFIG              => 0xEB;     OBSOLETE
## use constant X1RPKT_GET_DEV_CONFIG              => 0xEC;     OBSOLETE
## use constant X1RPKT_GET_SERVER_STATS            => 0xED;     OBSOLETE
## use constant X1RPKT_GET_HAB_STATS               => 0xEE;     OBSOLETE
## use constant X1RPKT_GET_DEFRAG_STATUS           => 0xEF;     OBSOLETE

## use constant X1RPKT_GET_RESYNC_INFO             => 0xF0;     OBSOLETE
## use constant X1RPKT_RESYNC_CONTROL              => 0xF1;     OBSOLETE
## use constant X1RPKT_LICENSE_CONFIG              => 0xF2;
## use constant X1RPKT_VCG_VALIDATION              => 0xF3;

#
# ----- Operation codes used with X1PKT_CONFIG_VDISK -----
#
## use constant CFG_VDISK_DELETE                    => 0x01;    OBSOLETE

## use constant CFG_VDISK_ADD_RAID_0                => 0x10;    OBSOLETE
## use constant CFG_VDISK_ADD_RAID_5                => 0x11;    OBSOLETE
## use constant CFG_VDISK_ADD_RAID_10               => 0x12;    OBSOLETE

## use constant CFG_VDISK_EXP_RAID_0                => 0x20;    OBSOLETE
## use constant CFG_VDISK_EXP_RAID_5                => 0x21;    OBSOLETE
## use constant CFG_VDISK_EXP_RAID_10               => 0x22;    OBSOLETE

## use constant CFG_VDISK_VDISK_MOVE                => 0x80;    OBSOLETE
## use constant CFG_VDISK_VDISK_COPY                => 0x81;    OBSOLETE
## use constant CFG_VDISK_VDISK_SWAP                => 0x82;    OBSOLETE
## use constant CFG_VDISK_VDISK_MIRROR              => 0x83;    OBSOLETE
## use constant CFG_VDISK_VDISK_BREAK_MIRROR        => 0x84;    OBSOLETE
## use constant CFG_VDISK_VDISK_COPY_PAUSE          => 0x85;    OBSOLETE
## use constant CFG_VDISK_VDISK_COPY_RESUME         => 0x86;    OBSOLETE
## use constant CFG_VDISK_VDISK_COPY_ABORT          => 0x87;    OBSOLETE

## use constant CFG_VDISK_SET_ATTRIBUTE             => 0x90;    OBSOLETE
## use constant CFG_VDISK_SET_LOCK                  => 0x91;    OBSOLETE
## use constant CFG_UNUSED1                         => 0x92;    OBSOLETE
## use constant CFG_VDISK_ERASE_VDISK               => 0x99;    OBSOLETE

## use constant CFG_VDISK_SET_SERVER_NAME           => 0xA0;    OBSOLETE
## use constant CFG_VDISK_SET_MASK                  => 0xA1;    OBSOLETE
## use constant CFG_VDISK_SET_LUN                   => 0xA2;    OBSOLETE
## use constant CFG_VDISK_SET_DEFMODE               => 0xA3;    OBSOLETE
## use constant CFG_VDISK_VDISK_SET_NAME            => 0xA4;    OBSOLETE
## use constant CFG_VDISK_MAG_RSVD_1                => 0xA5;    OBSOLETE
## use constant CFG_VDISK_VDISK_SET_CACHE_OFF       => 0xA6;    OBSOLETE
## use constant CFG_VDISK_SELECT_HAB_FOR_SERVER     => 0xA7;    OBSOLETE
## use constant CFG_VDISK_VLINK_BREAK               => 0xA8;    OBSOLETE
## use constant CFG_VDISK_VLINK_CREATE              => 0xA9;    OBSOLETE
## use constant CFG_VDISK_VDISK_SET_CACHE_ON        => 0xAA;    OBSOLETE
## use constant CFG_VDISK_SELECT_TARGET             => 0xAB;    OBSOLETE
## use constant CFG_VDISK_ASSIGN_VBLOCK             => 0xAC;    OBSOLETE
## use constant CFG_VDISK_SET_WORKSET_NAME          => 0xAD;    OBSOLETE
## use constant CFG_VDISK_SET_VPORT                 => 0xAE;    OBSOLETE
## use constant CFG_VDISK_SERVER_IN_WORKSET         => 0xAF;    OBSOLETE

## use constant CFG_VDISK_PDISK_LABEL               => 0xB0;    OBSOLETE
## use constant CFG_VDISK_PDISK_SPINDOWN            => 0xB1;    OBSOLETE
## use constant CFG_VDISK_PDISK_DEFRAG              => 0xB2;    OBSOLETE
## use constant CFG_VDISK_PDISK_SCRUB               => 0xB3;    OBSOLETE
## use constant CFG_VDISK_PDISK_FAIL                => 0xB4;    OBSOLETE
## use constant CFG_VDISK_PDISK_UNFAIL              => 0xB5;    OBSOLETE
## use constant CFG_VDISK_PDISK_BEACON              => 0xB6;    OBSOLETE
## use constant CFG_VDISK_PDISK_DELETE              => 0xB7;    OBSOLETE
## use constant CFG_VDISK_LOG_ACKNOWLEDGE           => 0xB8;    OBSOLETE
## use constant CFG_VDISK_DELETE_SERVER             => 0xB9;    OBSOLETE
## use constant CFG_VDISK_OP_SET_PRIORITY           => 0xBA; # unused by us...
## use constant CFG_VDISK_RAID_RECOVER              => 0xBB;    OBSOLETE
## use constant CFG_VDISK_SET_GLOBAL_CACHE_ON       => 0xBC;    OBSOLETE
## use constant CFG_VDISK_SET_GLOBAL_CACHE_OFF      => 0xBD;    OBSOLETE

###############################################################################

1;

##############################################################################
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
