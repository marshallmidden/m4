# $Header$
##############################################################################
# Xiotech a Seagate Technology
# Copyright (c) 2001  Xiotech
# ======================================================================
# $RCSfile$
# Author: Randy Rysavy / Chris Nigbur / Tim Swatosh
#
# Purpose:
##############################################################################
package XIOTech::PI_CommandCodes;

require Exporter;
@ISA = qw(Exporter);
@EXPORT = qw(
    COMPAT_INDEX_1 
    COMPAT_INDEX_3 
    VERSION_1 
    VERSION_2 
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
    PI_PDISKS_CMD

    PI_VDISK_COUNT_CMD
    PI_VDISK_LIST_CMD
    PI_VDISK_INFO_CMD
    PI_VDISK_CREATE_CMD
    PI_VDISK_DELETE_CMD
    PI_VDISK_EXPAND_CMD
    PI_VDISK_CONTROL_CMD
    PI_VDISK_PREPARE_CMD
    PI_VDISK_SET_PRIORITY_CMD
    PI_VDISK_OWNER_CMD
    PI_VDISK_SET_ATTRIBUTE_CMD
    PI_VDISKS_CMD

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
    PI_STATS_I2C_CMD
    PI_STATS_HAB_CMD

    PI_RAID_COUNT_CMD
    PI_RAID_LIST_CMD
    PI_RAID_INFO_CMD
    PI_RAID_INIT_CMD
    PI_RAID_CONTROL_CMD
    PI_RAID_MIRRORING_CMD
    PI_RAID_RECOVER_CMD
    PI_RAIDS_CMD

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

    PI_PDISK_AUTO_FAILBACK_ENABLE_DISABLE_CMD
    PI_SET_GEO_LOCATION_CMD
    PI_CLEAR_GEO_LOCATION_CMD
    PI_ENVIRO_DATA_DISK_BAY_CMD
    PI_ENVIRO_DATA_CTRL_AND_BAY_CMD
    PI_VCG_CONFIGURE_CMD
    PI_ENV_II_GET_CMD

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
    PI_GEOPOOL_GET_CMD
    PI_GEOPOOL_SET_CMD
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

    X1PKT_GET_DLINK
    X1PKT_GET_DLOCK_INFO
    X1PKT_GET_SU_LIST
    X1PKT_GET_SU_LUN_INFO

    X1PKT_GET_ENVIRON
    X1PKT_GET_PMAP
    X1PKT_GET_RMAP
    X1PKT_GET_VMAP
    X1PKT_GET_HMAP
    X1PKT_GET_PSTATS
    X1PKT_GET_BSTATS
    X1PKT_GET_RSTATS
    X1PKT_GET_VSTATS
    X1PKT_GET_PINFO
    X1PKT_GET_RINFO
    X1PKT_GET_VINFO
    X1PKT_GET_HINFO
    X1PKT_GET_ISALIVE
    X1PKT_GET_PSTATE

    X1PKT_GET_STATES
    X1PKT_GET_DNAME
    X1PKT_GET_CPU_LOAD
    X1PKT_GET_FREE
    X1PKT_WHO_INCOPY
    X1PKT_GET_COPY_INFO
    X1PKT_SERVER_MAP
    X1PKT_GET_MASK_INFO
    X1PKT_GET_LUNS_INFO
    X1PKT_GET_COPY_STATUS
    X1PKT_GET_MIRROR_STATUS
    X1PKT_GET_MIRROR_IO_STATUS

    X1PKT_PCHANGED
    X1PKT_RCHANGED
    X1PKT_VCHANGED
    X1PKT_HCHANGED
    X1PKT_GET_CONFIG
    X1PKT_VCG_CHANGED
    X1PKT_ACHANGED
    X1PKT_ZCHANGED
    X1PKT_READ_MEM
    X1PKT_READ_DPRAM
    X1PKT_VLINKED_TO
    X1PKT_GET_PERF
    X1PKT_ACCT_SET
    X1PKT_ACCT_GET
    X1PKT_ACCT_SELECT
    X1PKT_ACCT_LOGIN

    X1RPKT_LOG_ENTRY
    X1PKT_SET_LOG
    X1RPKT_CONFIG_VDISK_ACK
    X1PKT_CONFIG_VDISK
    X1PKT_CONFIG_HAB
    X1PKT_PROBE
    X1RPKT_PROBE
    X1PKT_DISCONNECT
    X1PKT_LOG_ENTRY
    X1PKT_SECURE_LOGIN
    X1PKT_GET_MS

    X1PKT_GET_VCG_INFO
    X1PKT_GET_VERSION_INFO
    X1PKT_GET_BAY_MAP
    X1PKT_GET_BAY_INFO
    X1PKT_GET_VPORT_MAP
    X1PKT_GET_VPORT_INFO
    X1PKT_GET_ELEC_SIG_INFO
    X1PKT_GET_BE_LOOP_INFO
    X1PKT_GET_WORKSET_INFO
    X1PKT_GET_GEOPOOL_INFO
    X1PKT_SET_GEOPOOL_INFO
    X1PKT_PUT_DEV_CONFIG
    X1PKT_GET_DEV_CONFIG
    X1PKT_GET_SERVER_STATS
    X1PKT_GET_HAB_STATS 
    X1PKT_GET_DEFRAG_STATUS

    X1PKT_GET_RESYNC_INFO
    X1PKT_RESYNC_CONTROL
    
    X1PKT_VCG_VALIDATION
    
    X1PKT_BIGFOOT_CMD
    X1RPKT_BIGFOOT_CMD


    X1RPKT_GET_DLINK
    X1RPKT_GET_DLOCK_INFO
    X1PKT_REPLY_SU_CNT
    X1PKT_REPLY_SU_ITEM
    X1PKT_REPLY_LUN_CNT
    X1PKT_REPLY_LUN_ITEM
    X1PKT_LICENSE_CONFIG

    X1RPKT_GET_ENVIRON
    X1RPKT_GET_PMAP
    X1RPKT_GET_RMAP
    X1RPKT_GET_VMAP
    X1RPKT_GET_HMAP
    X1RPKT_GET_PSTATS
    X1RPKT_GET_BSTATS
    X1RPKT_GET_RSTATS
    X1RPKT_GET_VSTATS
    X1RPKT_GET_PINFO
    X1RPKT_GET_RINFO
    X1RPKT_GET_VINFO
    X1RPKT_GET_HINFO
    X1RPKT_GET_ISALIVE
    X1RPKT_GET_RAIDPER
    X1RPKT_GET_PSTATE

    X1RPKT_GET_STATES
    X1RPKT_GET_DNAME
    X1RPKT_GET_CPU_LOAD
    X1RPKT_GET_FREE
    X1RPKT_WHO_INCOPY
    X1RPKT_GET_COPYINFO
    X1RPKT_GET_COPY_INFO
    X1RPKT_SERVER_MAP
    X1RPKT_GET_MASK_INFO
    X1RPKT_GET_LUNS_INFO
    X1RPKT_GET_COPY_STATUS
    X1RPKT_GET_MIRROR_STATUS
    X1RPKT_GET_MIRROR_IO_STATUS

    X1RPKT_GET_CONFIG
    X1RPKT_READ_MEM
    X1RPKT_READ_DPRAM
    X1RPKT_VLINKED_TO
    X1RPKT_GET_PERF
    X1RPKT_ACCT_SET
    X1RPKT_ACCT_GET
    X1RPKT_ACCT_SELECT
    X1RPKT_ACCT_LOGIN

    X1RPKT_SET_LOG
    X1RPKT_CONFIG_VDISK
    X1RPKT_CONFIG_HAB
    X1RPKT_SECURE_LOGIN
    X1RPKT_GET_MS

    X1RPKT_VCG_INFO
    X1RPKT_GET_VERSION_INFO
    X1RPKT_GET_BAY_MAP
    X1RPKT_GET_BAY_INFO
    X1RPKT_GET_VPORT_MAP
    X1RPKT_GET_VPORT_INFO
    X1RPKT_GET_ELEC_SIG_INFO
    X1RPKT_GET_BE_LOOP_INFO
    X1RPKT_GET_WORKSET_INFO
    X1RPKT_GET_GEOPOOL_INFO
    X1RPKT_SET_GEOPOOL_INFO
    X1RPKT_PUT_DEV_CONFIG
    X1RPKT_GET_DEV_CONFIG
    X1RPKT_GET_SERVER_STATS
    X1RPKT_GET_HAB_STATS
    X1RPKT_GET_DEFRAG_STATUS   

    X1RPKT_GET_RESYNC_INFO
    X1RPKT_RESYNC_CONTROL

    X1RPKT_VCG_VALIDATION
    
    X1RPKT_LICENSE_CONFIG
    
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
    CFG_VDISK_SET_GEORAID
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
use constant COMPAT_INDEX_1                             => 0x0001;
use constant COMPAT_INDEX_3                             => 0x0003;

###############################################################################
# Command Codes
use constant PI_CONNECT_CMD                             => 0x0001;
use constant PI_DISCONNECT_CMD                          => 0x0002;
use constant PI_PING_CMD                                => 0x0003;
use constant PI_RESET_CMD                               => 0x0004;
use constant PI_POWER_UP_STATE_CMD                      => 0x0005;
use constant PI_POWER_UP_RESPONSE_CMD                   => 0x0006;

use constant PI_REGISTER_EVENTS_CMD                     => 0x0008;
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
use constant PI_STATS_I2C_CMD                           => 0x006E;
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
use constant PI_DISK_BAYS_CMD                           => 0x00BF;

#SERVICEABILITY42
use constant PI_PDISK_AUTO_FAILBACK_ENABLE_DISABLE_CMD  => 0x00C0; 
use constant PI_ENVIRO_DATA_DISK_BAY_CMD                => 0x00C1; 
use constant PI_ENVIRO_DATA_CTRL_AND_BAY_CMD            => 0x00C2; 
use constant PI_VCG_CONFIGURE_CMD                       => 0x00C3;
use constant PI_ENV_II_GET_CMD                          => 0x00C4;

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
use constant PI_GEOPOOL_GET_CMD                         => 0x0212;
use constant PI_GEOPOOL_SET_CMD                         => 0x0213;
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
use constant PI_ENABLE_X1_PORT_CMD                      => 0x0244;
use constant PI_VDISK_BAY_REDUNDANT_CMD                 => 0x0245;
use constant PI_GET_BACKEND_TYPE_CMD                    => 0x0246;


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
use constant X1PKT_GET_DLINK                    => 0x10;
use constant X1PKT_GET_DLOCK_INFO               => 0x11;
use constant X1PKT_GET_SU_LIST                  => 0x12;
use constant X1PKT_GET_SU_LUN_INFO              => 0x13;

use constant X1PKT_GET_ENVIRON                  => 0x20;
use constant X1PKT_GET_PMAP                     => 0x21;
use constant X1PKT_GET_RMAP                     => 0x22;
use constant X1PKT_GET_VMAP                     => 0x23;
use constant X1PKT_GET_HMAP                     => 0x24;
use constant X1PKT_GET_PSTATS                   => 0x25;
use constant X1PKT_GET_BSTATS                   => 0x26;
use constant X1PKT_GET_RSTATS                   => 0x27;
use constant X1PKT_GET_VSTATS                   => 0x28;
use constant X1PKT_GET_PINFO                    => 0x29;
use constant X1PKT_GET_RINFO                    => 0x2A;
use constant X1PKT_GET_VINFO                    => 0x2B;
use constant X1PKT_GET_HINFO                    => 0x2C;
use constant X1PKT_GET_ISALIVE                  => 0x2D;
use constant X1PKT_GET_PSTATE                   => 0x2F;

use constant X1PKT_GET_STATES                   => 0x30;
use constant X1PKT_GET_DNAME                    => 0x31;
use constant X1PKT_GET_CPU_LOAD                 => 0x32;
use constant X1PKT_GET_FREE                     => 0x35;
use constant X1PKT_WHO_INCOPY                   => 0x38;
use constant X1PKT_GET_COPY_INFO                => 0x39;
use constant X1PKT_SERVER_MAP                   => 0x3A;
use constant X1PKT_GET_MASK_INFO                => 0x3B;
use constant X1PKT_GET_LUNS_INFO                => 0x3C;
use constant X1PKT_GET_COPY_STATUS              => 0x3D;
use constant X1PKT_GET_MIRROR_STATUS            => 0x3E;

use constant X1PKT_PCHANGED                     => 0x40;
use constant X1PKT_RCHANGED                     => 0x41;
use constant X1PKT_VCHANGED                     => 0x42;
use constant X1PKT_HCHANGED                     => 0x43;
use constant X1PKT_GET_CONFIG                   => 0x44;
use constant X1PKT_VCG_CHANGED                  => 0x45;
use constant X1PKT_ACHANGED                     => 0x46;
use constant X1PKT_ZCHANGED                     => 0x47;
use constant X1PKT_READ_MEM                     => 0x48;
use constant X1PKT_READ_DPRAM                   => 0x49;
use constant X1PKT_VLINKED_TO                   => 0x4A;
use constant X1PKT_GET_PERF                     => 0x4B;
use constant X1PKT_ACCT_SET                     => 0x4C;
use constant X1PKT_ACCT_GET                     => 0x4D;
use constant X1PKT_ACCT_SELECT                  => 0x4E;
use constant X1PKT_ACCT_LOGIN                   => 0x4F;

use constant X1RPKT_LOG_ENTRY                   => 0x50;
use constant X1PKT_SET_LOG                      => 0x51;
use constant X1RPKT_CONFIG_VDISK_ACK            => 0x57;
use constant X1PKT_CONFIG_VDISK                 => 0x58;
use constant X1PKT_CONFIG_HAB                   => 0x59;
use constant X1PKT_PROBE                        => 0x5A;
use constant X1RPKT_PROBE                       => 0x5A;
use constant X1PKT_DISCONNECT                   => 0x5C;
use constant X1PKT_LOG_ENTRY                    => 0x5D;
use constant X1PKT_SECURE_LOGIN                 => 0x5E;
use constant X1PKT_GET_MS                       => 0x5F;

use constant X1PKT_GET_VCG_INFO                 => 0x60;
use constant X1PKT_GET_VERSION_INFO             => 0x61;
use constant X1PKT_GET_BAY_MAP                  => 0x62;
use constant X1PKT_GET_BAY_INFO                 => 0x63;
use constant X1PKT_GET_VPORT_MAP                => 0x64;
use constant X1PKT_GET_VPORT_INFO               => 0x65;
use constant X1PKT_GET_ELEC_SIG_INFO            => 0x66;
use constant X1PKT_GET_BE_LOOP_INFO             => 0x67;
use constant X1PKT_GET_WORKSET_INFO             => 0x68;
use constant X1PKT_GET_GEOPOOL_INFO             => 0x69;
use constant X1PKT_SET_GEOPOOL_INFO             => 0x6A;
use constant X1PKT_PUT_DEV_CONFIG               => 0x6B;
use constant X1PKT_GET_DEV_CONFIG               => 0x6C;
use constant X1PKT_GET_SERVER_STATS             => 0x6D;
use constant X1PKT_GET_HAB_STATS                => 0x6E;
use constant X1PKT_GET_DEFRAG_STATUS            => 0x6F;

use constant X1PKT_GET_RESYNC_INFO              => 0x70;
use constant X1PKT_RESYNC_CONTROL               => 0x71;
use constant X1PKT_LICENSE_CONFIG               => 0x72;
use constant X1PKT_GET_MIRROR_IO_STATUS         => 0x73;


use constant X1PKT_BIGFOOT_CMD                  => 0xBF;
use constant X1RPKT_BIGFOOT_CMD                 => 0xBF;


#
# ----- X1 Response packets 
#
use constant X1RPKT_GET_DLINK                   => 0x90;
use constant X1RPKT_GET_DLOCK_INFO              => 0x91;
use constant X1PKT_REPLY_SU_CNT                 => 0x9A;
use constant X1PKT_REPLY_SU_ITEM                => 0x9B;
use constant X1PKT_REPLY_LUN_CNT                => 0x9C;
use constant X1PKT_REPLY_LUN_ITEM               => 0x9D;

use constant X1RPKT_GET_ENVIRON                 => 0xA0;
use constant X1RPKT_GET_PMAP                    => 0xA1;
use constant X1RPKT_GET_RMAP                    => 0xA2;
use constant X1RPKT_GET_VMAP                    => 0xA3;
use constant X1RPKT_GET_HMAP                    => 0xA4;
use constant X1RPKT_GET_PSTATS                  => 0xA5;
use constant X1RPKT_GET_BSTATS                  => 0xA6;
use constant X1RPKT_GET_RSTATS                  => 0xA7;
use constant X1RPKT_GET_VSTATS                  => 0xA8;
use constant X1RPKT_GET_PINFO                   => 0xA9;
use constant X1RPKT_GET_RINFO                   => 0xAA;
use constant X1RPKT_GET_VINFO                   => 0xAB;
use constant X1RPKT_GET_HINFO                   => 0xAC;
use constant X1RPKT_GET_ISALIVE                 => 0xAD;
use constant X1RPKT_GET_RAIDPER                 => 0xAE;
use constant X1RPKT_GET_PSTATE                  => 0xAF;

use constant X1RPKT_GET_STATES                  => 0xB0;
use constant X1RPKT_GET_DNAME                   => 0xB1;
use constant X1RPKT_GET_CPU_LOAD                => 0xB2;
use constant X1RPKT_GET_FREE                    => 0xB5;
use constant X1RPKT_WHO_INCOPY                  => 0xB8;
use constant X1RPKT_GET_COPYINFO                => 0xB9;
use constant X1RPKT_GET_COPY_INFO               => 0xB9;
use constant X1RPKT_SERVER_MAP                  => 0xBA;
use constant X1RPKT_GET_MASK_INFO               => 0xBB;
use constant X1RPKT_GET_LUNS_INFO               => 0xBC;
use constant X1RPKT_GET_COPY_STATUS             => 0xBD;
use constant X1RPKT_GET_MIRROR_STATUS           => 0xBE;
use constant X1RPKT_GET_MIRROR_IO_STATUS        => 0xF3;

use constant X1RPKT_GET_CONFIG                  => 0xC4;
use constant X1RPKT_READ_MEM                    => 0xC8;
use constant X1RPKT_READ_DPRAM                  => 0xC9;
use constant X1RPKT_VLINKED_TO                  => 0xCA;
use constant X1RPKT_GET_PERF                    => 0xCB;
use constant X1RPKT_ACCT_SET                    => 0xCC;
use constant X1RPKT_ACCT_GET                    => 0xCD;
use constant X1RPKT_ACCT_SELECT                 => 0xCE;
use constant X1RPKT_ACCT_LOGIN                  => 0xCF;

use constant X1RPKT_SET_LOG                     => 0xD1;
use constant X1RPKT_CONFIG_VDISK                => 0xD8;
use constant X1RPKT_CONFIG_HAB                  => 0xD9;
use constant X1RPKT_SECURE_LOGIN                => 0xDE;
use constant X1RPKT_GET_MS                      => 0xDF;

use constant X1RPKT_VCG_INFO                    => 0xE0;
use constant X1RPKT_GET_VERSION_INFO            => 0xE1;
use constant X1RPKT_GET_BAY_MAP                 => 0xE2;
use constant X1RPKT_GET_BAY_INFO                => 0xE3;
use constant X1RPKT_GET_VPORT_MAP               => 0xE4;
use constant X1RPKT_GET_VPORT_INFO              => 0xE5;
use constant X1RPKT_GET_ELEC_SIG_INFO           => 0xE6;
use constant X1RPKT_GET_BE_LOOP_INFO            => 0xE7;
use constant X1RPKT_GET_WORKSET_INFO            => 0xE8;
use constant X1RPKT_GET_GEOPOOL_INFO            => 0xE9;
use constant X1RPKT_SET_GEOPOOL_INFO            => 0xEA;
use constant X1RPKT_PUT_DEV_CONFIG              => 0xEB;
use constant X1RPKT_GET_DEV_CONFIG              => 0xEC;
use constant X1RPKT_GET_SERVER_STATS            => 0xED;
use constant X1RPKT_GET_HAB_STATS               => 0xEE;
use constant X1RPKT_GET_DEFRAG_STATUS           => 0xEF;

use constant X1RPKT_GET_RESYNC_INFO             => 0xF0;
use constant X1RPKT_RESYNC_CONTROL              => 0xF1;
use constant X1RPKT_LICENSE_CONFIG              => 0xF2;
use constant X1RPKT_VCG_VALIDATION              => 0xF3;


#
# ----- Operation codes used with X1PKT_CONFIG_VDISK -----
#
use constant CFG_VDISK_DELETE                    => 0x01;

use constant CFG_VDISK_ADD_RAID_0                => 0x10;
use constant CFG_VDISK_ADD_RAID_5                => 0x11;
use constant CFG_VDISK_ADD_RAID_10               => 0x12;

use constant CFG_VDISK_EXP_RAID_0                => 0x20;
use constant CFG_VDISK_EXP_RAID_5                => 0x21;
use constant CFG_VDISK_EXP_RAID_10               => 0x22;

use constant CFG_VDISK_VDISK_MOVE                => 0x80;
use constant CFG_VDISK_VDISK_COPY                => 0x81;
use constant CFG_VDISK_VDISK_SWAP                => 0x82;
use constant CFG_VDISK_VDISK_MIRROR              => 0x83;
use constant CFG_VDISK_VDISK_BREAK_MIRROR        => 0x84;
use constant CFG_VDISK_VDISK_COPY_PAUSE          => 0x85;
use constant CFG_VDISK_VDISK_COPY_RESUME         => 0x86;
use constant CFG_VDISK_VDISK_COPY_ABORT          => 0x87;

use constant CFG_VDISK_SET_ATTRIBUTE             => 0x90;
use constant CFG_VDISK_SET_LOCK                  => 0x91;
use constant CFG_VDISK_SET_GEORAID               => 0x92;
use constant CFG_VDISK_ERASE_VDISK               => 0x99;

use constant CFG_VDISK_SET_SERVER_NAME           => 0xA0;
use constant CFG_VDISK_SET_MASK                  => 0xA1;
use constant CFG_VDISK_SET_LUN                   => 0xA2;
use constant CFG_VDISK_SET_DEFMODE               => 0xA3;
use constant CFG_VDISK_VDISK_SET_NAME            => 0xA4;
use constant CFG_VDISK_MAG_RSVD_1                => 0xA5;
use constant CFG_VDISK_VDISK_SET_CACHE_OFF       => 0xA6;
use constant CFG_VDISK_SELECT_HAB_FOR_SERVER     => 0xA7;
use constant CFG_VDISK_VLINK_BREAK               => 0xA8;
use constant CFG_VDISK_VLINK_CREATE              => 0xA9;
use constant CFG_VDISK_VDISK_SET_CACHE_ON        => 0xAA;
use constant CFG_VDISK_SELECT_TARGET             => 0xAB;
use constant CFG_VDISK_ASSIGN_VBLOCK             => 0xAC;
use constant CFG_VDISK_SET_WORKSET_NAME          => 0xAD;
use constant CFG_VDISK_SET_VPORT                 => 0xAE;
use constant CFG_VDISK_SERVER_IN_WORKSET         => 0xAF;

use constant CFG_VDISK_PDISK_LABEL               => 0xB0;
use constant CFG_VDISK_PDISK_SPINDOWN            => 0xB1;
use constant CFG_VDISK_PDISK_DEFRAG              => 0xB2;
use constant CFG_VDISK_PDISK_SCRUB               => 0xB3;
use constant CFG_VDISK_PDISK_FAIL                => 0xB4;
use constant CFG_VDISK_PDISK_UNFAIL              => 0xB5;
use constant CFG_VDISK_PDISK_BEACON              => 0xB6;
use constant CFG_VDISK_PDISK_DELETE              => 0xB7;
use constant CFG_VDISK_LOG_ACKNOWLEDGE           => 0xB8;
use constant CFG_VDISK_DELETE_SERVER             => 0xB9;
use constant CFG_VDISK_OP_SET_PRIORITY           => 0xBA; # unused by us...
use constant CFG_VDISK_RAID_RECOVER              => 0xBB;
use constant CFG_VDISK_SET_GLOBAL_CACHE_ON       => 0xBC;
use constant CFG_VDISK_SET_GLOBAL_CACHE_OFF      => 0xBD;

###############################################################################

1;

##############################################################################
# $Log$
# Revision 1.15  2006/12/20 11:16:54  BharadwajS
# TBolt00017375 Changing PI_COMPATIBILITY to 3
#
# Revision 1.14  2006/12/05 06:56:37  GudipudiK
# TBolt00017234 Changed the Packetinterface for iSNS
#
# Revision 1.13  2006/11/29 22:17:49  BharadwajS
# TBolt00017125 adding VDisk create time and last access time.
#
# Revision 1.12  2006/11/06 15:31:54  BoddukuriV
# TBolt00017032
# ICL code initial merge
#
# Revision 1.11  2006/11/06 08:22:32  GudipudiK
# iSNS code merge to MAIN. TBolt00015194
#
# Revision 1.10  2006/07/17 20:38:31  RustadM
# TBolt00014770
# Move 750 branch onto main.
#
# Revision 1.9.2.8  2006/07/14 11:22:05  BharadwajS
# TBolt00014568
# Adding IDD structures to NVRAM Part 5 dump
#
# Revision 1.9.2.7  2006/05/24 05:36:06  BharadwajS
# Adding PI_ADMIN_GETTIME_CMD
#
# Revision 1.9.2.6  2006/05/15 17:01:28  HoltyB
# TBolt00000000:Added environmental PI interface/async events/validation
#
# Revision 1.9.2.5  2006/04/28 05:49:43  BharadwajS
# PI for VCG_CONFIGURE_CMD
#
# Revision 1.9.2.4  2006/04/26 09:22:29  BharadwajS
# CCBE changes for PI Versioning
#
# Revision 1.9.2.3  2006/04/26 09:06:19  BharadwajS
# CCBE changes for PI Versioning
#
# Revision 1.9.2.2  2006/04/17 06:07:40  BharadwajS
# Log Acknowledgement
#
# Revision 1.9.2.1  2006/04/12 09:51:16  GudipudiK
# packet interface changes for CCBE changes
#
# Revision 1.9  2005/12/30 05:34:13  ChannappaS
# Reverting command code for PI_MISC_LOCAL_RAID_INFO_CMD
#
# Revision 1.8  2005/12/23 08:13:48  BalemarthyS
# Merged ISCSI & GEORAID related changes
#
# Revision 1.7  2005/10/31 16:01:36  DeanL
# TBolt00013387 - Changed logic to allow enabling and disabling of inquiry data for WHQL testing.
# Reviewed by Chris Nigbur
#
# Revision 1.6  2005/07/30 07:15:23  BharadwajS
# X1 Code for Serviceability
#
# Revision 1.5  2005/07/06 05:51:08  BalemarthyS
# Serviceability feature check-in
#
# Revision 1.4  2005/06/01 12:18:24  BharadwajS
# VDisk Priority Chagnes
#
# Revision 1.3  2005/05/25 10:09:44  BharadwajS
# Undo vpri changes
#
# Revision 1.2  2005/05/24 14:18:35  BharadwajS
# TBolt00000 vpri
#
# Revision 1.1.1.1  2005/05/04 18:53:54  RysavyR
# import CT1_BR to shared/Wookiee
#
# Revision 1.92  2005/03/31 20:26:23  NigburC
# TBolt00012271 - Added the new MRMMTEST MRP and all the handling
# routines required to run it.  This is the test driver for the MicroMemory card
# and allows the user to fail the board and in the future inject single and
# multi bit ECC errors.  The CCBCL and CCBE commands are MMTEST.
# Reviewed by Lynn Waggie.
#
# Revision 1.91  2005/01/31 15:45:06  SchibillaM
# TBolt00012221: Add support for X1PKT_GET_MIRROR_IO_STATUS.
# Reviewed by Chris.
#
# Revision 1.90  2004/12/16 19:12:02  SchibillaM
# TBolt00011891: Implement PI_StatsServers - stats for all valid servers on a  controller.
# Reviewed by Chris.
#
# Revision 1.89  2004/11/01 17:27:11  NigburC
# TBolt00011442 - First pass at battery board health change notifications.  This
# removes the old battery health code from RM and async event handler and
# replaces it with a battery health task that runs and updates the controllers
# battery health and its source mirror partner (who is mirroring to this controller).
# Development Task: No review required.
#
# Revision 1.88  2004/10/19 21:47:17  RysavyR
# TBolt00011525: Add X1 "License Config" packet.
# Also added color for Linux operation.
#
# Revision 1.87  2004/09/21 15:24:30  WilliamsJ
# TBolt00011344 - Merge of resync into main.
#
# Revision 1.86.4.2  2004/09/15 13:11:26  NigburC
# TBolt00011260 - Integrating the first pass at the X1 packet handlers for
# resync functionality.
# Reviewed by Mark Schibilla.
#
# Revision 1.86.4.1  2004/09/13 13:08:30  NigburC
# TBolt00000000 - Updates to the RESYNC code to support the new
# CM_COR.h file which replaces the cor.h file in proc/inc.
# Added the new resync MRPs to the MR_Defs.h and added the supporting
# code in the CCB, CCBE and CCBCL for these MRPs.
# Added a separate queue and process to handle the PUTDG broadcast
# events.
# Reviewed by Jeff Williams.
#
# Revision 1.86  2004/08/24 14:00:00  McmasterM
# TBolt00011106: Add interface to CCB and CCBE to test MicroMemory interface
# Done.  Reviewed by Randy Rysavy.
#
# Revision 1.85  2004/08/19 20:42:47  SchibillaM
# TBolt00011069: Add support for Defrag Status command.
#
# Revision 1.84  2004/07/27 12:13:26  SchibillaM
# TBolt00010893: Add support for X1 environmental packet in PI interface.
# Reviewed by Chris.
#
# Revision 1.83  2004/06/25 13:50:22  SchibillaM
# TBolt00010632: Add PI and X1 support for HAB Stats.  Reviewed by Chris.
#
# Revision 1.82  2004/06/15 18:43:05  SchibillaM
# TBolt00010632: Add support for X1 Server and HBA stats.  New Stats Manager
# component.  HBA stats framework done, waiting for proc support.  Reviewed by Chris.
#
# Revision 1.81  2004/05/05 15:20:34  NigburC
# TBolt00010427 - Added new packet requests to support getting and setting
# of the device configuration information.  These code changes do not yet
# save the data to a persistent storage on the CCB.
# Reviewed by Mark Schibilla.
#
# Revision 1.80  2004/04/08 19:00:48  SchibillaM
# TBolt00010249: Change X1 Processor Utilization request to X1 Get CPU Loads
# which was already defined for Mag and exists in UI middleware.  Reviewed by Chris.
#
# Revision 1.79  2004/03/17 17:55:27  SchibillaM
# TBolt00010249: Add CCB, CCBE support for X1 processor utilization.  Reviewed by Chris.
#
# Revision 1.78  2004/03/08 22:17:15  RysavyR
# TBolt00010199: Add MRP - MRRAIDRECOVER to recover an inoperative raid.
# Rev by Tim Swatosh..
#
# Revision 1.77.4.1  2004/03/05 17:34:13  RysavyR
# TBolt00010199: Add support for the new MRP - MRRAIDRECOVER used to
# recover an inoperative raid.  Rev by Tim Sw.
#
# Revision 1.77  2003/09/30 14:21:59  SchibillaM
# TBolt00009208: Add support for the X1 CFG_VDISK_DELETE_SERVER command.
# This command takes an X1Sid as input and deletes all associated procSids.
# Reviewed by Chris.
#
# Revision 1.76  2003/09/26 19:35:02  SchibillaM
# TBolt00009252: Add support for PI Refresh CCB Cache function.
#
# Revision 1.75  2003/08/28 16:30:10  SchibillaM
# TBolt00009060: Remove worksetID from CfgVDisk Select VPort for Server.  Add
# cmd to add-remove a server in a workset.  Reviewed by Bryan.
#
# Revision 1.74  2003/08/20 18:44:36  SchibillaM
# TBolt00009016: Add X1VDiskCfg subcommand for GeoRaid attribute.
#
# Revision 1.73  2003/08/19 20:58:44  SchibillaM
# TBolt00008962: Add defaultVPort to workset.
#
# Revision 1.72  2003/07/25 18:32:55  SchibillaM
# TBolt00008793: X1GeoPool get and set support.  Reviewed by Randy.
#
# Revision 1.71  2003/07/24 14:23:40  SchibillaM
# TBolt00008793: Initial GeoPool support.  PI set and get packets with CCBE support.
# Reviewed by Chris.
#
# Revision 1.70  2003/07/18 19:16:15  SchibillaM
# TBolt00008030: Complete X1 Workset support.  Reviewed by Randy.
#
# Revision 1.69  2003/07/16 13:08:19  SchibillaM
# TBolt00008030: Initial workset support.  X1 changes to set worksets not complete.
# Reviewed by Chris.
#
# Revision 1.68  2003/07/08 19:07:18  NigburC
# TBolt00008575 - Added the RESYNCMIRRORS command to the CCBE.
# Reviewed by Jim Snead.
#
# Revision 1.67  2003/05/07 13:19:31  HoltyB
# TBolt00007922:  Remove SNMP from the CCBE code.
#
# Revision 1.66  2003/04/23 19:43:03  RysavyR
# TBolt00006947: Add BF passthru packet (PI_ADMIN_GET_IP_CMD) that
# returns the current IP addr, subnet mask, gateway and controller serial number.
# Rev by TimSw.
#
# Revision 1.65  2003/04/11 17:39:28  SchibillaM
# TBolt00007915: Add PI support for  X1 DLink and Dlock info.
#
# Revision 1.64  2003/03/27 15:18:51  SchibillaM
# TBolt00007915: Add X1 support for VLink commands - VLinked To, VLink LUN
# Info and VLink Storage Unit Info.
#
# Revision 1.63  2003/02/13 21:40:43  NigburC
# TBolt00007272 - Added the PDISKBYPASS command and it associated
# handler functions.
# Reviewed by Tim Swatosh and Jeff Williams.
#
# Revision 1.62  2003/02/11 16:19:14  SchibillaM
# TBolt00007143: Change references to ServerWwnToPortMap to
# ServerWwnToTargetMap.
#
# Revision 1.61  2003/01/31 12:21:29  NigburC
# TBolt00000000 - Added missing constants for set log.
# Reviewed by Jeff Williams.
#
# Revision 1.60  2003/01/16 21:44:06  SchibillaM
# TBolt00006514: Add support for X1 Electrical Signatures and version in X1 Config.
#
# Revision 1.59  2003/01/02 17:28:31  NigburC
# TBolt00006588 - Added a packet and the corresponding functions to retrieve
# the VCG mirror partner list.
# Reviewed by Bryan Holty.
#
# Revision 1.58  2002/12/24 17:13:25  SchibillaM
# TBolt00006514: Add support for X1 Bay Map and VPort Map.  Reviewed by Chris.
#
# Revision 1.57  2002/12/20 21:02:00  NigburC
# TBolt00005368 - Added a RMSTATE command to the CCBE, CCBCL and
# CCB.  This function returns the RMCurrentState value defined in RM.C.
# Reviewed by Craig Menning.
#
# Revision 1.56  2002/12/17 23:36:51  McmasterM
# TBolt00006250: Add support for I2C switch device
# TBolt00006251: Add support for new I2C EEPROMs (component info collection)
# Full switch support and nearly all of the EEPROM support is in place.
#
# Revision 1.55  2002/12/17 20:07:16  NigburC
# TBolt00006482 - Added the new command, MFGCLEAN to the CCBE and
# CCBCL.
# Reviewed by Tim Swatosh (virtually).
#
# Revision 1.54  2002/12/11 16:22:43  NigburC
# TBolt00006452, TBolt00006451 - Added code to support the SET_LOCK
# operation for X1 VDISK CONFIG.  This code required changes to the
# underlying structures and MRPs for virtual disk information, setting cache,
# and setting attributes.
#
# - PI_VDISK_SET_CACHE_CMD has been removed.
# - PI_VDISK_SET_ATTRIBUTE_CMD is used to set attributes, including
# virtual disk cache.
# - PI_VCG_SET_CACHE_CMD is used to set global caching.
# Reviewed by Mark Schibilla and Randy Rysavy.
#
# Revision 1.53  2002/12/10 21:22:56  SchibillaM
# TBolt00006408: Implement 0xAB operation in VDisk Config to Select a Target.
# Reviewed by Chris.
#
# Revision 1.52  2002/12/06 21:37:50  NigburC
# TBolt00006392, TBolt00006394, TBolt00006429 - Lots of changes to enjoy.
# - Added code to support the new NAME_DEVICE MRP.
# - Added code to support setting server, vdisk and controller names.
# - Updated the SERVERASSOC and SERVERDELETE commands to allow
# additional options.
# Reviewed by Mark Schibilla.
#
# Revision 1.51  2002/12/03 18:29:44  SchibillaM
# TBolt00000000: Add support for X1RPKT_GET_COPY_STATUS (copy map)
# and X1PKT_GET_MIRROR_STATUS (mirror map).
#
# Revision 1.50  2002/11/21 23:15:20  SchibillaM
# TBolt00004962: Cache target info for use by validation.
# Add PI_SERVER_WWN_TO_PORT_MAP_CMD command.
# Reviewed by Chris.
#
# Revision 1.49  2002/11/15 20:17:46  NigburC
# TBolt00005791 - Added new command codes for the VCG activate and
# inactivate commands.  Removed the reference to the old debug set serial
# number command.
# Reviewed by Mark Schibilla.
#
# Revision 1.48  2002/11/14 15:20:03  SchibillaM
# TBolt00004962: Add support for Loop Primitive MRP.  This function is useful
# for validation testing.
#
# Revision 1.47  2002/10/29 17:43:51  NigburC
# TBolt00000000 - Beginning change to serial number changes, could not
# find the defect for this set of changes.
# Reviewed by Mark Schibilla.
#
# Revision 1.46  2002/10/25 16:04:41  RysavyR
# TBolt00006013: Added X1 config "encryption" support and added intermediate
# VDisk config ACK.  Rev by Mark S.
#
# Revision 1.45  2002/10/19 19:14:13  HoltyB
# TBolt00006201:  Added persistent data functionality to CCB.
#
# Revision 1.44  2002/10/17 23:52:15  SwatoshT
# TBolt00006013: Added support for X1sending of VCG Async Events and
# initial support for packed log events. Removed sending of debug messages
# on X1 port.
# Reveiwed by Mark S.
#
# Revision 1.43  2002/10/14 21:29:15  RysavyR
# TBolt00006136: Added multi-packet support for transferring fw and large files
# within packets that are <64K.  Rev. by TimSw.
#
# Revision 1.42  2002/10/01 19:01:27  RysavyR
# TBolt00006013:  Add the ability to handle and process BF style packets on
# the X1 port. Reviewed by TimSw.
#
# Revision 1.41  2002/09/18 20:32:40  NigburC
# TBolt00006022, TBolt00004962 - Added a retry when sending the PING
# before unfailing a controller in case the session was in a bad state.
# Added new command to the CCBE and packet interface to initiate the
# group redundancy validation (VCGVALIDATION).
# Reviewed by Tim Swatosh.
#
# Revision 1.40  2002/09/16 16:52:45  SchibillaM
# TBolt00004610: Add Get Name and Write Name debug functions for VLink debug.
# Add flag to notify AsyncClient that PortServer is running.
# Reviewed by Randy Rysavy.
#
# Revision 1.39  2002/09/11 20:53:50  HoltyB
# TBolt00005263:  Added packet interface and functionality to change a
# controller ip, subnet mask, or gateway address.
#
# Revision 1.38  2002/07/20 13:10:23  SchibillaM
# TBolt00005437: Add packet interface support for Interface Fail.
# Reviewed by Chris.
#
# Revision 1.37  2002/07/19 21:27:06  SchibillaM
# TBolt00005437: Implement a new packet command for unfail interface.
# Reviewed by Chris.
#
# Revision 1.36  2002/07/18 18:02:50  HoltyB
# TBolt00005346: Changed packet interface for modebits
#
# Revision 1.35  2002/07/10 01:28:54  HoltyB
# TBolt00005254: Changed Loginfo to display in the current computers local
# time zone.  Also added an option to still display the GMT time.
# TBolt00005248: Added new interface to write a debug message to the
# CCB logs.
#
# Revision 1.34  2002/07/08 20:00:10  NigburC
# TBolt00000000 - House cleaning...removed unused command codes (mainly
# ones from the VCG set).
#
# Revision 1.33  2002/07/01 20:19:03  SchibillaM
# TBolt00004285: Implement disk bay audible alarm control.
# Review by Randy Rysavy (C, Perl),  Chris (Java)
#
# Revision 1.32  2002/06/29 14:24:08  HoltyB
# TBolt00005018: Fixed SERVERCREATE, made up to date with current input structure
# Added support for new environmental data
#
# Revision 1.31  2002/06/14 12:42:30  NigburC
# TBolt00000665 - Added additional command codes and log events that start
# the integration of the power-up and licensing changes.
# Added new option to PDISKS command in CCBE to display firmware/vendor
# information.
#
# Revision 1.30  2002/06/05 14:19:06  HoltyB
# TBolt00004480: Added new Packet interface for new BE device path mrp
# TBolt00004647: Added new option to existing opions for resetting Qlogic's
# TBolt00004564: Added new parameter for get port lists
#
# Revision 1.29  2002/06/04 19:18:07  RysavyR
# TBolt00003598: Added the first pass at configuration snapshotting.
#
# Revision 1.28  2002/05/14 15:50:17  RysavyR
# TBolt00001593:  Added VCG firmware "Rolling Update" method support.
#
# Revision 1.27  2002/05/07 18:25:05  SwatoshT
# Toblt00004253: Support for controlling Attention Led.
# Reviewed by Randy Ry.
#
# Revision 1.26  2002/05/03 19:22:25  NigburC
# TBolt00004144 - Added the FAILURESTATESET command and the
# underlying functions in the CCBE and CCB.
#
# Revision 1.25  2002/04/30 20:04:46  NigburC
# TBolt00004033, TBolt00002733, TBolt00002730 - Lots of changes for these
# three defects.  Mainly, modified the VCGInfo request to return all controllers
# configured as part of the VCG instead of just active controllers.  This caused
# changes in CCB, CCBE and UMC code.
# Added the REMOVE, FAIL, UNFAIL, and SHUTDOWN methods for VCGs.
# Not all of these are working completely...just a stage check-in.
#
# Revision 1.24  2002/04/29 21:59:27  HoltyB
# TBolt00004068: Separated DEVDELETE into two seperate commands
# PDISKDELETE and DISKBAYDELETE and removed DEVDELETE
#
# Revision 1.23  2002/04/29 16:49:58  HoltyB
# TBolt00004068:  Added PI_PROC_DEV_DEL_COMMAND_CMD which gives
# the ability to delete a Pdisk or diskBay. Added new command in the ccbe
# called DEVDELETE
#
# Revision 1.22  2002/04/26 18:19:48  NigburC
# TBolt00004077 - Added the SERVERS and TARGETS commands to the
# CCB, CCBE and CCBCL.
#
# Revision 1.21  2002/04/24 14:17:56  NigburC
# Tbolt00003999, TBolt00004000, TBolt00004001 - Added VCGAPPLYLICENSE,
# VCGUNFAILCONTROLLER and code to keep a failed controller failed.
#
# Revision 1.20  2002/04/24 13:58:25  RysavyR
# TBolt00001738: Added generic SCSI command. Rev by TimSw
#
# Revision 1.19  2002/04/23 19:16:06  SchibillaM
# TBolt00003921: Rearrange command codes.
# Reviewed by Chris.
#
# Revision 1.18  2002/04/23 18:33:42  SchibillaM
# TBolt00003921: Rearrange command codes.  Reviewed by Chris.
#
# Revision 1.17  2002/04/16 14:30:53  NigburC
# TBolt00003594 - Added VDISKS, RAIDS and PORTLIST commands.
# Modified the DEVSTAT calls to use the new VDISKS and RAIDS commands
# to make them faster.
#
# Revision 1.16  2002/04/05 19:45:47  NigburC
# TBolt00003119 - Modified the login to set the default timeout for the
# packet interface to 1800 seconds.  Eventually the CCB will change to have
# a timeout of 60 seconds or less and we still want the CCBE to function with
# the longer timeout.
#
# Revision 1.15  2002/04/05 16:37:53  SwatoshT
# TBolt00000017: Added support for setting the RTC through the packet interface.
# This syncs the RTC to the system clock.
# Reviewed by Randy Ry.
#
# Revision 1.14  2002/04/01 21:32:59  RysavyR
# Added TRYCCB functionality
#
# Revision 1.13  2002/04/01 16:47:31  HoltyB
# Added function to retrieve current election state
#
# Revision 1.12  2002/03/27 13:11:13  SchibillaM
# TBolt00003487: Add support for Get BE Port List and Break VLink lock.
# Reviewed by Randy.
#
# Revision 1.11  2002/03/25 16:59:16  HoltyB
# TBolt00003442: Added support for SNMP configuration on the ccb to allow
# ip addresses to be sent to the ccb to be used for generating traps
#
# Revision 1.10  2002/03/18 20:34:53  NigburC
# TBolt00003338, TBolt00002733 - Added CCBE side of the command to
# retrieve the information for all disk bays in one request.
# Added the first basic implementation of controller validation.  This includes
# the VCGVALIDATESLAVE command handler and the framework to add
# additional validation steps.
#
# Revision 1.9  2002/03/15 16:26:19  RysavyR
# TBolt00003329: Added a "write buffer" command to the packet interface to
# support SCSI write buffer "microde download & save" (mode 5) to drives and
# drive bays.
#
# Revision 1.8  2002/03/07 20:37:49  SchibillaM
# TBolt00003263: Add CCB support to save/restore names to file system.  Total
# rewrite of names.c & .h.  Removed includes of names.h where they were not
# required and added names_old.h to files that just need these definitions to
# compile.  Added PI commands to read and write name FIDs.  Reviewed by Chris.
#
# Revision 1.7  2002/03/04 17:29:44  NigburC
# TBolt00003191 - Added the TARGETMOVE command to CCBE and CCB.
#
# Revision 1.6  2002/02/26 20:11:40  NigburC
# TBolt00000159 - Started adding statistics command codes.
#
# Revision 1.5  2002/02/19 21:13:58  SchibillaM
# TBolt00003096: Add DiskBaySetName command to CCBE.
#
# Revision 1.4  2002/02/14 19:06:14  HoltyB
# TBolt00003081: Added new function:
#    DEVICELIST
#
# Revision 1.3  2002/02/11 19:08:03  HoltyB
# moved command codes to PI_CommandCodes.pm
#
# Revision 1.2  2002/02/05 23:20:09  NigburC
# TBOLT00002954,
# TBOLT00002877,
# TBOLT00002730
#
# This set of changes include the above work items and defects.  Also are
# changes associated with MRP packet structure changes.
# We have moved many functions from CPS_INIT.C to PI_VCG.C in order
# to start returning error codes from the functions that work on the VCG.
# New functions have been added including but not limited to:
#   RESCANDEVICE
#   TARGETRESLIST
#   SERVERLOOKUP (not yet implemented in CCBE and CCBCL)
#   VDISKOWNER
#
# Revision 1.1  2002/01/22 12:47:07  NigburC
# TBolt00002859 - Added new files for packet interface specific items such as
# command codes and constants.  Also added a xioPacket module to handle
# data sent and received from the CCB.
##############################################################################
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
