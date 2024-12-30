# $Id: xiotechPackets.pm 159258 2012-06-09 11:55:08Z m4 $
##############################################################################
# Xiotech
# Copyright (c) 2001 - 2008  Xiotech Corporation. All rights reserved.
# ======================================================================
#
# Purpose:
#   Common functionality to handle Xiotech packets
##############################################################################
package XIOTech::xiotechPackets;
 
require Exporter;
@ISA = qw(Exporter);
@EXPORT = qw(
    encrypt
    decrypt
    assembleXiotechPacket
    disassembleXiotechPacket
    disassembleXiotechHeader
    commandCode
    seqNum
    status
    errorCode
    data
    dumpPacket

    PI_GOOD
    PI_ERROR
    PI_SOCKET_ERROR
    PI_IN_PROGRESS
    PI_TIMEOUT
    PI_INVALID_CMD_CODE
    PI_MALLOC_ERROR
    PI_PARAMETER_ERROR
    PI_MASTER_CNT_ERROR
    PI_POWER_UP_REQ_ERROR
    PI_ELECTION_ERROR
    PI_TUNNEL_ERROR
    PI_R5_STRIPE_RESYNC_ERROR
    PI_LOCAL_RAID_RESYNC_ERROR
    PI_INVALID_PACKETVERSION_ERROR
    PI_COMPAT_INDEX_NOT_SUPPORTED

    PACKET_HEADER_SIZE_3100

    LABEL_TYPE_NONE
    LABEL_TYPE_DATA
    LABEL_TYPE_SPARE
    LABEL_TYPE_NDATA
    LABEL_OPTION_FULL
    LABEL_OPTION_FSYS
    LABEL_OPTION_I_OWN
    LABEL_OPTION_KEEP_CLASS

    VDISK_CREATE_OP_CREATE
    VDISK_CREATE_OP_EXPAND
    VDISK_CREATE_OP_PREPARE

    FW_VER_CCB_BOOT
    FW_VER_CCB_RUNTIME
    FW_VER_BE_BOOT
    FW_VER_BE_DIAG
    FW_VER_BE_RUNTIME
    FW_VER_FE_BOOT
    FW_VER_FE_DIAG
    FW_VER_FE_RUNTIME
    FW_VER_QL2200_EF   
    FW_VER_QL2200_EFM  
    FW_VER_QL2300_EF   
    FW_VER_QL2300_EFM      
    FW_VER_SYSTEM     

    FW_RESET_BE
    FW_RESET_FE
    
    PROC_INIT_NVRAM
    PROC_INIT_FE_NVA_RECORDS
    PROC_INIT_NMI_COUNTS
    PROC_INIT_BE_NVA_RECORDS
    
    RESCAN_EXISTING
    RESCAN_LUNS
    RESCAN_REDISCOVER

    SCRUB_DISABLE
    SCRUB_ENABLE
    SCRUB_POLL

    SCRUB_PC_DEFAULT_MASK
    SCRUB_PC_MARKED_MASK
    SCRUB_PC_CORRUPT_MASK
    SCRUB_PC_SPECIFIC_MASK
    SCRUB_PC_CLEARLOGS_MASK
    SCRUB_PC_1PASS_MASK
    SCRUB_PC_CORRECT_MASK
    SCRUB_PC_ENABLE_MASK
    SCRUB_PC_DISABLE_MASK

    SCRUB_PC_CLEARLOG_MASK
    
    PI_GENERIC_RESET
    PI_GENERIC_TEST_TARGET
    PI_GENERIC_GLOBAL_MRP_TIMEOUT
    PI_GENERIC_FUNCTION_CALL
    PI_GENERIC_DEBUG_ADDRESS
    PI_GENERIC_GLOBAL_PI_SELECT_TIMEOUT
    PI_GENERIC_NAME_FID_INIT
    PI_GENERIC_DO_ELECTION
    PI_GENERIC_GLOBAL_IPC_TIMEOUT
    PI_GENERIC_FAILURE_MANAGER
    PI_GENERIC_DISABLE_HEARTBEATS
    PI_GENERIC_ERROR_TRAP
    PI_GENERIC_GET_SOS_STRUCTURE
    PI_GENERIC_SET_PDISK_LED
    PI_GENERIC_SEND_LOG_EVENT
    PI_GENERIC_CACHE_TEST
    PI_GENERIC_DISASTER_TEST
    PI_GENERIC_KEEP_ALIVE_TEST
    PI_GENERIC_FIO_MAP_TEST
    PI_GENERIC_FCM_COUNTER_TEST

    PI_GENERIC_ERROR_TRAP_CCB
    PI_GENERIC_ERROR_TRAP_BE
    PI_GENERIC_ERROR_TRAP_FE
    PI_GENERIC_ERROR_TRAP_ALL
    
    PI_GENERIC_CACHE_TEST_START
    PI_GENERIC_CACHE_TEST_STOP
    PI_GENERIC_CACHE_TEST_CACHE
    PI_GENERIC_CACHE_TEST_X1
    PI_GENERIC_CACHE_TEST_ASYNC

    PI_GENERIC_DISASTER_TEST_RESET
    PI_GENERIC_DISASTER_TEST_CLEAR
    PI_GENERIC_DISASTER_TEST_SET

    PI_GENERIC_KEEP_ALIVE_TEST_RESET
    PI_GENERIC_KEEP_ALIVE_TEST_CLEAR
    PI_GENERIC_KEEP_ALIVE_TEST_SET
    PI_GENERIC_KEEP_ALIVE_TEST_DISABLE
    PI_GENERIC_KEEP_ALIVE_TEST_ENABLE

    PI_GENERIC_FIO_MAP_TEST_READ
    PI_GENERIC_FIO_MAP_TEST_WRITE
    PI_GENERIC_FIO_MAP_TEST_RESET
    PI_GENERIC_FIO_MAP_TEST_CLEAR
    PI_GENERIC_FIO_MAP_TEST_SET

    PI_GENERIC_FCM_COUNTER_TEST_DUMP
    PI_GENERIC_FCM_COUNTER_TEST_BASELINE
    PI_GENERIC_FCM_COUNTER_TEST_UPDATE
    PI_GENERIC_FCM_COUNTER_TEST_DELTA
    PI_GENERIC_FCM_COUNTER_TEST_MAJOR_EVENT
    PI_GENERIC_FCM_COUNTER_TEST_MINOR_EVENT

    PI_SET_IP_CHANGE_IP
    PI_SET_IP_CHANGE_SUBNET
    PI_SET_IP_CHANGE_GATEWAY
    PI_SET_IP_UPDATE_NVRAM

    PI_VCG_VAL_TYPE_HW
    PI_VCG_VAL_TYPE_STORAGE
    PI_VCG_VAL_TYPE_SERVER
    PI_VCG_VAL_TYPE_COMM
    PI_VCG_VAL_TYPE_BE_LOOP
    PI_VCG_VAL_TYPE_SHELF_ID
    PI_VCG_VAL_TYPE_SYS_REL 
    PI_VCG_VAL_TYPE_ALL 
    PI_VCG_VAL_RUN_IMMED
    PI_VCG_VAL_TYPE_BACK_END 
    PI_VCG_VAL_TYPE_NORMAL   
    PI_VCG_VAL_TYPE_DAILY    

    X1_ASYNC_PCHANGED
    X1_ASYNC_RCHANGED
    X1_ASYNC_VCHANGED
    X1_ASYNC_HCHANGED
    X1_ASYNC_ACHANGED
    X1_ASYNC_ZCHANGED
    ASYNC_ENV_CHANGE
    ASYNC_DEFRAG_CHANGE
    ASYNC_PDATA_CREATE
    ASYNC_PDATA_REMOVE
    ASYNC_PDATA_MODIFY
    ASYNC_ISNS_MODIFY
    ASYNC_BUFFER_BOARD_CHANGE
    ASYNC_GLOBAL_CACHE_CHANGE
    ASYNC_PRES_CHANGED
    ASYNC_APOOL_CHANGED
    X1_ASYNC_VCG_ELECTION_STATE_CHANGE
    X1_ASYNC_VCG_ELECTION_STATE_ENDED
    X1_ASYNC_VCG_POWERUP
    X1_ASYNC_VCG_CFG_CHANGED
    X1_ASYNC_VCG_WORKSET_CHANGED
    ISE_ENV_CHANGED
    X1_ASYNC_BE_PORT_CHANGE
    X1_ASYNC_FE_PORT_CHANGE
    ASYNC_PING_EVENT
    LOG_STD_MSG
    LOG_XTD_MSG
    LOG_BIN_MSG
    
    LOG_TYPE_INFO
    LOG_TYPE_WARNING
    LOG_TYPE_ERROR
    LOG_TYPE_DEBUG
    
    FAILURE_MANAGER_FAIL_CONTROLLER
    FAILURE_MANAGER_FAIL_INTERFACE
    RESOURCE_MANAGER_UNFAIL_CONTROLLER
    RESOURCE_MANAGER_UNFAIL_INTERFACE

    GET_HEAP_STATS
    GET_TRACE_STATS
    GET_PCB_STATS
    GET_PROFILE_STATS
    GET_PACKET_STATS

    PD_DT_UNKNOWN
    PD_DT_FC_DISK
    PD_DT_SATA
    PD_DT_SSD
    PD_DT_ECON_ENT
    PD_DT_SAS
    PD_DT_MAX_DISK
    PD_DT_SES
    PD_DT_FC_SES
    PD_DT_SATA_SES
    PD_DT_SBOD_SES
    PD_DT_SBOD_SAS_EXP
    PD_DT_ISE_SES
    PD_DT_ISE_HIGH_PERF
    PD_DT_ISE_PERF
    PD_DT_ISE_BALANCE
    PD_DT_ISE_CAPACITY
    PD_DT_MAX_SES

    DEVSTATUS_NONEXISTENT
    DEVSTATUS_INOPERABLE
    DEVSTATUS_UNINITIALIZED
    DEVSTATUS_INITIALIZING
    DEVSTATUS_ERROR
    DEVSTATUS_OPERATIONAL
    DEVSTATUS_DEGRADED
    DEVSTATUS_REBUILDING
    DEVSTATUS_DEFRAGGING

    DEBUG_ED_STATE_END_TASK
    DEBUG_ED_STATE_BEGIN_ELECTION
    DEBUG_ED_STATE_CHECK_MASTERSHIP_ABILITY
    DEBUG_ED_STATE_TIMEOUT_CONTROLLERS
    DEBUG_ED_STATE_TIMEOUT_CONTROLLERS_COMPLETE
    DEBUG_ED_STATE_CONTACT_ALL_CONTROLLERS
    DEBUG_ED_STATE_CONTACT_ALL_CONTROLLERS_COMPLETE
    DEBUG_ED_STATE_WAIT_FOR_MASTER
    DEBUG_ED_STATE_CHECK_MASTER
    DEBUG_ED_STATE_NOTIFY_SLAVES
    DEBUG_ED_STATE_FAILED
    DEBUG_ED_STATE_FINISHED
    
    DELETE_DISK_DRIVE
    DELETE_DISK_BAY
    
    I2C_MONITOR_STATUS_CODE_UNKNOWN
    I2C_MONITOR_STATUS_CODE_NOT_PRESENT
    I2C_MONITOR_STATUS_CODE_VALID
    I2C_MONITOR_STATUS_CODE_BUSY
    I2C_MONITOR_STATUS_CODE_NOT_READY
    I2C_MONITOR_STATUS_CODE_ERROR

    I2C_MONITOR_TEMPERATURE_CONDITION_UNKNOWN
    I2C_MONITOR_TEMPERATURE_CONDITION_NORMAL
    I2C_MONITOR_TEMPERATURE_CONDITION_COLD
    I2C_MONITOR_TEMPERATURE_CONDITION_HOT
    I2C_MONITOR_TEMPERATURE_CONDITION_COLD_CRITICAL
    I2C_MONITOR_TEMPERATURE_CONDITION_HOT_CRITICAL

    I2C_MONITOR_LIMIT_MONITOR_UNKNOWN
    I2C_MONITOR_LIMIT_MONITOR_GOOD
    I2C_MONITOR_LIMIT_MONITOR_TRIPPED

    I2C_MONITOR_POWER_SUPPLY_CONDITION_UNKNOWN
    I2C_MONITOR_POWER_SUPPLY_CONDITION_GOOD
    I2C_MONITOR_POWER_SUPPLY_CONDITION_HIGH_TEMPERATURE
    I2C_MONITOR_POWER_SUPPLY_CONDITION_DC_FAILED
    I2C_MONITOR_POWER_SUPPLY_CONDITION_AC_FAILED
    I2C_MONITOR_POWER_SUPPLY_CONDITION_INSERTED
    I2C_MONITOR_POWER_SUPPLY_CONDITION_NOT_PRESENT

    I2C_MONITOR_COOLING_FAN_CONDITION_UNKNOWN
    I2C_MONITOR_COOLING_FAN_CONDITION_GOOD
    I2C_MONITOR_COOLING_FAN_CONDITION_FAILED
    I2C_MONITOR_COOLING_FAN_CONDITION_NOT_PRESENT

    I2C_MONITOR_PROCESSOR_RESET_CONDITION_UNKNOWN
    I2C_MONITOR_PROCESSOR_RESET_CONDITION_RUNNING
    I2C_MONITOR_PROCESSOR_RESET_CONDITION_RESET

    I2C_MONITOR_BATTERY_CONDITION_UNKNOWN
    I2C_MONITOR_BATTERY_CONDITION_GOOD
    I2C_MONITOR_BATTERY_CONDITION_LOW_CAPACITY
    I2C_MONITOR_BATTERY_CONDITION_UNDER_VOLTAGE
    I2C_MONITOR_BATTERY_CONDITION_OVER_VOLTAGE
    I2C_MONITOR_BATTERY_CONDITION_NOT_PRESENT

    I2C_MONITOR_CURRENT_FLOW_CONDITION_UNKNOWN
    I2C_MONITOR_CURRENT_FLOW_CONDITION_GOOD
    I2C_MONITOR_CURRENT_FLOW_CONDITION_ABNORMAL

    I2C_MONITOR_FUEL_GAUGE_CONDITION_UNKNOWN
    I2C_MONITOR_FUEL_GAUGE_CONDITION_GOOD
    I2C_MONITOR_FUEL_GAUGE_CONDITION_SHUTDOWN

    I2C_MONITOR_MAIN_REGULATOR_CONDITION_UNKNOWN
    I2C_MONITOR_MAIN_REGULATOR_CONDITION_OPERATIONAL
    I2C_MONITOR_MAIN_REGULATOR_CONDITION_SHUTDOWN_ERROR
    I2C_MONITOR_MAIN_REGULATOR_CONDITION_SHUTDOWN_GOOD

    I2C_MONITOR_CHARGER_CONDITION_UNKNOWN
    I2C_MONITOR_CHARGER_CONDITION_IDLE
    I2C_MONITOR_CHARGER_CONDITION_TRICKLE
    I2C_MONITOR_CHARGER_CONDITION_BULK
    I2C_MONITOR_CHARGER_CONDITION_OVER
    I2C_MONITOR_CHARGER_CONDITION_TOPOFF

    I2C_MONITOR_NVRAM_BATTERY_CONDITION_UNKNOWN
    I2C_MONITOR_NVRAM_BATTERY_CONDITION_GOOD
    I2C_MONITOR_NVRAM_BATTERY_CONDITION_FAILED

    I2C_MONITOR_EEPROM_CONDITION_UNKNOWN
    I2C_MONITOR_EEPROM_CONDITION_GOOD
    I2C_MONITOR_EEPROM_CONDITION_BAD_CRC
    I2C_MONITOR_EEPROM_CONDITION_NOT_READABLE

    CONTROLLER_NOT_MASTER

    FD_STATE_UNUSED
    FD_STATE_FAILED
    FD_STATE_OPERATIONAL
    FD_STATE_POR
    FD_STATE_ADD_CONTROLLER_TO_VCG
    FD_STATE_STRANDED_CACHE_DATA
    FD_STATE_FIRMWARE_UPDATE_INACTIVE
    FD_STATE_FIRMWARE_UPDATE_ACTIVE
    FD_STATE_UNFAIL_CONTROLLER
    FD_STATE_VCG_SHUTDOWN
    FD_STATE_INACTIVATED
    FD_STATE_ACTIVATE
    FD_STATE_DISASTER_INACTIVE

    POWER_UP_UNKNOWN
    POWER_UP_START
    POWER_UP_WAIT_FWV_INCOMPATIBLE
    POWER_UP_WAIT_PROC_COMM
    POWER_UP_WAIT_CONFIGURATION
    POWER_UP_WAIT_LICENSE
    POWER_UP_WAIT_DRIVES
    POWER_UP_WAIT_DISASTER
    POWER_UP_DISCOVER_CONTROLLERS
    POWER_UP_WAIT_CONTROLLERS
    POWER_UP_PROCESS_BE_INIT
    POWER_UP_PROCESS_DISCOVERY
    POWER_UP_WAIT_DISK_BAY
    POWER_UP_WAIT_CORRUPT_BE_NVRAM
    POWER_UP_ALL_CTRL_BE_READY
    POWER_UP_PROCESS_R5_RIP
    POWER_UP_SIGNAL_SLAVES_RUN_FE
    POWER_UP_PROCESS_CACHE_INIT
    POWER_UP_WAIT_CACHE_ERROR
    POWER_UP_INACTIVE
    POWER_UP_FAILED
    POWER_UP_WRONG_SLOT
    POWER_UP_FAILED_AUTO_NODE_CONFIG 
    POWER_UP_COMPLETE

    POWER_UP_ASTATUS_UNKNOWN
    POWER_UP_ASTATUS_WC_SEQNO_BAD
    POWER_UP_ASTATUS_WC_SN_VCG_BAD
    POWER_UP_ASTATUS_WC_SN_BAD
    POWER_UP_ASTATUS_WC_NVMEM_BAD

    RM_NONE
    RM_INIT
    RM_SHUTDOWN
    RM_RUNNING
    RM_BUSY
    RM_DOWN

    CONTROLLER_SN
    SYSTEM_SN
    
    INIT_CCB_NVRAM_TYPE_FULL
    INIT_CCB_NVRAM_TYPE_LICENSE

    RESET_QLOGIC_RESET_INITIALIZE
    RESET_QLOGIC_RESET_ONLY
    RESET_QLOGIC_RESET_INITIALIZE_IF_OFFLINE
    RESET_QLOGIC_RESET_ONLY_IF_OFFLINE
    RESET_QLOGIC_RESET_INITIALIZE_LOG
    
    PORTS_ALL
    PORTS_INITIALIZED
    PORTS_FAILED
    PORTS_INITIALIZING
    PORTS_WITH_TARGETS
    PORTS_NO_TARGETS
        
    PATH_PHYSICAL_DISK
    PATH_MISC_DEVICE
    PATH_ENCLOSURES
    FORMAT_PID_BITPATH
    FORMAT_PID_PATH_ARRAY
    FORMAT_PID_1

    MRFELOOPPRIMITIVE
    MRBELOOPPRIMITIVE

    MLPRESLOOP      
    MLPRESLIDPORT  
    MLPSIDPIDRES  
    MLPLOGINLID                                                           
    MLPLOGINPID                                                                         
    MLPLOGOUTLID                                                           
    MLPLOGOUTPID
    
    MIRXSIZ
    
    MNDSERVER
    MNDVDISK
    MNDVCG
    MNDRETVCG

    MMLMAP
    MMLSWAP
    MMLCOPY
    MMLMOVE
    
    PERSISTENT_DATA_OPTION_READ
    PERSISTENT_DATA_OPTION_WRITE
    PERSISTENT_DATA_OPTION_RESET
    PERSISTENT_DATA_OPTION_CHECKSUM
    
    PI_TEST_PORT
    PI_DEBUG_PORT
    PI_ASYNC_PORT

    PI_MD5_KEY

    CTRL_TYPE_BIGFOOT
    CTRL_TYPE_WOOKIEE
    CTRL_TYPE_750
    CTRL_TYPE_3100 
    CTRL_TYPE_4000 
    CTRL_TYPE_4700 
    CTRL_TYPE_7000 
    CTRL_TYPE_7400 
    CTRL_TYPE_UNKNOWN
);

use XIOTech::logMgr;
use XIOTech::md5;
use XIOTech::PI_CommandCodes;
use strict;

###############################################################################
# Status Codes for Littlefoot
use constant PI_GOOD                                    => 0;
use constant PI_ERROR                                   => 1;
use constant PI_SOCKET_ERROR                            => -1;
use constant PI_IN_PROGRESS                             => 2;
use constant PI_TIMEOUT                                 => 4;
use constant PI_INVALID_CMD_CODE                        => 5;
use constant PI_MALLOC_ERROR                            => 6;
use constant PI_PARAMETER_ERROR                         => 7;
use constant PI_MASTER_CNT_ERROR                        => 8;
use constant PI_POWER_UP_REQ_ERROR                      => 9;
use constant PI_ELECTION_ERROR                          => 10;
use constant PI_TUNNEL_ERROR                            => 11;
use constant PI_R5_STRIPE_RESYNC_ERROR                  => 12;
use constant PI_LOCAL_RAID_RESYNC_ERROR                 => 13;
use constant PI_INVALID_PACKETVERSION_ERROR             => 14;
use constant PI_COMPAT_INDEX_NOT_SUPPORTED              => 15;
###############################################################################

# Some constants for the packet structure
use constant PACKET_HEADER_SIZE_3100                    => 128;

# Label Physical Device MRP - INPUT
use constant LABEL_TYPE_NONE                            => 0x00;
use constant LABEL_TYPE_DATA                            => 0x01;
use constant LABEL_TYPE_SPARE                           => 0x02;
use constant LABEL_TYPE_NDATA                           => 0x03;
use constant LABEL_OPTION_FULL                          => 0x1;
use constant LABEL_OPTION_FSYS                          => 0x2;
use constant LABEL_OPTION_I_OWN                         => 0x4;
use constant LABEL_OPTION_KEEP_CLASS                    => 0x8;

# Prepare/Create/Expand Virtual Disk MRP - INPUT
use constant VDISK_CREATE_OP_CREATE                     => 0x00;
use constant VDISK_CREATE_OP_EXPAND                     => 0x01;
use constant VDISK_CREATE_OP_PREPARE                    => 0x02;

# Firmware Version Type Codes
use constant FW_VER_BE_BOOT                             => 0x129;
use constant FW_VER_BE_DIAG                             => 0x12A;
use constant FW_VER_BE_RUNTIME                          => 0x12B;
use constant FW_VER_FE_BOOT                             => 0x509;
use constant FW_VER_FE_DIAG                             => 0x50A;
use constant FW_VER_FE_RUNTIME                          => 0x50B;
use constant FW_VER_CCB_BOOT                            => 0xFFF0;
use constant FW_VER_CCB_RUNTIME                         => 0xFFF1;
use constant FW_VER_QL2200_EF                           => 0xFFF2;
use constant FW_VER_QL2200_EFM                          => 0xFFF3;
use constant FW_VER_QL2300_EF                           => 0xFFF4;
use constant FW_VER_QL2300_EFM                          => 0xFFF5;
use constant FW_VER_SYSTEM                              => 0xFFFF;

# Firmware Reset Type Codes
use constant FW_RESET_BE                                => 0x0132;
use constant FW_RESET_FE                                => 0x50E;

# Proc init types
use constant PROC_INIT_NVRAM                            => 0x00;
use constant PROC_INIT_FE_NVA_RECORDS                   => 0x01;
use constant PROC_INIT_NMI_COUNTS                       => 0x02;
use constant PROC_INIT_BE_NVA_RECORDS                   => 0x04;

# Rescan Type Codes
use constant RESCAN_EXISTING                            => 0x00;
use constant RESCAN_LUNS                                => 0x01;
use constant RESCAN_REDISCOVER                          => 0x02;

# SCRUBBING CONTROL OPTIONS
use constant SCRUB_DISABLE                              => 0x80000000;
use constant SCRUB_ENABLE                               => 0x80000001;
use constant SCRUB_POLL                                 => 0x00000000;

# SCRUBBING PARITY CONTROL BITS
use constant SCRUB_PC_DEFAULT_MASK                      => 0x00000080;
use constant SCRUB_PC_MARKED_MASK                       => 0x00000040;
use constant SCRUB_PC_CORRUPT_MASK                      => 0x00000020;
use constant SCRUB_PC_SPECIFIC_MASK                     => 0x00000010;
use constant SCRUB_PC_CLEARLOGS_MASK                    => 0x00000008;
use constant SCRUB_PC_1PASS_MASK                        => 0x00000004;
use constant SCRUB_PC_CORRECT_MASK                      => 0x00000002;
use constant SCRUB_PC_ENABLE_MASK                       => 0x00000001;
use constant SCRUB_PC_DISABLE_MASK                      => 0xFFFFFFFE;

# Added for compatibility
use constant SCRUB_PC_CLEARLOG_MASK                     => 0x00000008;
# End Additition


use constant LOG_TYPE_INFO                              => 0x00;
use constant LOG_TYPE_WARNING                           => 0x01;
use constant LOG_TYPE_ERROR                             => 0x02;
use constant LOG_TYPE_DEBUG                             => 0x03;

# genericCommand "cmdCodes"
use constant PI_GENERIC_RESET                           => 0x00;
use constant PI_GENERIC_TEST_TARGET                     => 0x01;
use constant PI_GENERIC_GLOBAL_MRP_TIMEOUT              => 0x02;
use constant PI_GENERIC_FUNCTION_CALL                   => 0x03;
use constant PI_GENERIC_DEBUG_ADDRESS                   => 0x04;
use constant PI_GENERIC_GLOBAL_PI_SELECT_TIMEOUT        => 0x05;
use constant PI_GENERIC_NAME_FID_INIT                   => 0x06;
use constant PI_GENERIC_DO_ELECTION                     => 0x07;
use constant PI_GENERIC_GLOBAL_IPC_TIMEOUT              => 0x08;
use constant PI_GENERIC_FAILURE_MANAGER                 => 0x09;
use constant PI_GENERIC_DISABLE_HEARTBEATS              => 0x0A;
use constant PI_GENERIC_ERROR_TRAP                      => 0x0B;
use constant PI_GENERIC_GET_SOS_STRUCTURE               => 0x0C;
use constant PI_GENERIC_SET_PDISK_LED                   => 0x0D;
use constant PI_GENERIC_SEND_LOG_EVENT                  => 0x0E;
use constant PI_GENERIC_CACHE_TEST                      => 0x0F;
use constant PI_GENERIC_DISASTER_TEST                   => 0x10;
use constant PI_GENERIC_KEEP_ALIVE_TEST                 => 0x11;
use constant PI_GENERIC_FIO_MAP_TEST                    => 0x12;
use constant PI_GENERIC_FCM_COUNTER_TEST                => 0x13;

# genericCommand PI_GENERIC_ERROR_TRAP Codes
use constant PI_GENERIC_ERROR_TRAP_CCB                  => 0x00;
use constant PI_GENERIC_ERROR_TRAP_BE                   => 0x01;
use constant PI_GENERIC_ERROR_TRAP_FE                   => 0x02;
use constant PI_GENERIC_ERROR_TRAP_ALL                  => 0x03;

# constants used in PI_GENERIC_CACHE_TEST above
use constant PI_GENERIC_CACHE_TEST_START                => 0x10;
use constant PI_GENERIC_CACHE_TEST_STOP                 => 0x11;
use constant PI_GENERIC_CACHE_TEST_CACHE                => 0x10;
use constant PI_GENERIC_CACHE_TEST_X1                   => 0x11;
use constant PI_GENERIC_CACHE_TEST_ASYNC                => 0x12;

# constants used in PI_GENERIC_DISASTER_TEST above
use constant PI_GENERIC_DISASTER_TEST_RESET             => 0x00;
use constant PI_GENERIC_DISASTER_TEST_CLEAR             => 0x01;
use constant PI_GENERIC_DISASTER_TEST_SET               => 0x02;

# constants used in PI_GENERIC_KEEP_ALIVE_TEST above
use constant PI_GENERIC_KEEP_ALIVE_TEST_RESET           => 0x00;
use constant PI_GENERIC_KEEP_ALIVE_TEST_CLEAR           => 0x01;
use constant PI_GENERIC_KEEP_ALIVE_TEST_SET             => 0x02;
use constant PI_GENERIC_KEEP_ALIVE_TEST_DISABLE         => 0x03;
use constant PI_GENERIC_KEEP_ALIVE_TEST_ENABLE          => 0x04;

# constants used in PI_GENERIC_FIO_MAP_TEST above
# Parameter 0
use constant PI_GENERIC_FIO_MAP_TEST_READ               => 0x00;
use constant PI_GENERIC_FIO_MAP_TEST_WRITE              => 0x01;
# Parameter 1
use constant PI_GENERIC_FIO_MAP_TEST_RESET              => 0x00;
use constant PI_GENERIC_FIO_MAP_TEST_CLEAR              => 0x01;
use constant PI_GENERIC_FIO_MAP_TEST_SET                => 0x02;

# constants used in PI_GENERIC_FCM_COUNTER_TEST above
# Parameter 0
use constant PI_GENERIC_FCM_COUNTER_TEST_DUMP           => 0x00;
use constant PI_GENERIC_FCM_COUNTER_TEST_BASELINE       => 0x01;
use constant PI_GENERIC_FCM_COUNTER_TEST_UPDATE         => 0x02;
use constant PI_GENERIC_FCM_COUNTER_TEST_DELTA          => 0x03;
use constant PI_GENERIC_FCM_COUNTER_TEST_MAJOR_EVENT    => 0x04;
use constant PI_GENERIC_FCM_COUNTER_TEST_MINOR_EVENT    => 0x05;

use constant PI_SET_IP_CHANGE_IP                        => 0x01;
use constant PI_SET_IP_CHANGE_SUBNET                    => 0x02;
use constant PI_SET_IP_CHANGE_GATEWAY                   => 0x04;
use constant PI_SET_IP_UPDATE_NVRAM                     => 0x80;

use constant PI_VCG_VAL_TYPE_HW                         => 0x0001;
use constant PI_VCG_VAL_TYPE_STORAGE                    => 0x0002;
use constant PI_VCG_VAL_TYPE_SERVER                     => 0x0004;
use constant PI_VCG_VAL_TYPE_COMM                       => 0x0008;

use constant PI_VCG_VAL_TYPE_BE_LOOP                    => 0x0010;
use constant PI_VCG_VAL_TYPE_SHELF_ID                   => 0x0020;
use constant PI_VCG_VAL_TYPE_SYS_REL                    => 0x0040;
use constant PI_VCG_VAL_TYPE_RSVD_B7                    => 0x0080;

use constant PI_VCG_VAL_TYPE_RSVD_B8                    => 0x0100;
use constant PI_VCG_VAL_TYPE_RSVD_B9                    => 0x0200;
use constant PI_VCG_VAL_TYPE_RSVD_B10                   => 0x0400;
use constant PI_VCG_VAL_TYPE_RSVD_B11                   => 0x0800;

use constant PI_VCG_VAL_TYPE_RSVD_B13                   => 0x1000;
use constant PI_VCG_VAL_TYPE_RSVD_B14                   => 0x2000;
use constant PI_VCG_VAL_TYPE_ALL                        => 0x4000;
use constant PI_VCG_VAL_RUN_IMMED                       => 0x8000;

use constant PI_VCG_VAL_TYPE_BACK_END                   => 0x0032;
use constant PI_VCG_VAL_TYPE_NORMAL                     => 0x003F;
use constant PI_VCG_VAL_TYPE_DAILY                      => 0x007F;

# See file CCB/Inc/X1_AsyncEventHandler.h
use constant X1_ASYNC_PCHANGED                          => 0x00000001;
use constant X1_ASYNC_RCHANGED                          => 0x00000002;
use constant X1_ASYNC_VCHANGED                          => 0x00000004;
use constant X1_ASYNC_HCHANGED                          => 0x00000008;
use constant X1_ASYNC_ACHANGED                          => 0x00000010;
use constant X1_ASYNC_ZCHANGED                          => 0x00000020;
use constant ASYNC_ENV_CHANGE                           => 0x00000040;
use constant ASYNC_DEFRAG_CHANGE                        => 0x00000080;
use constant ASYNC_PDATA_CREATE                         => 0x00000100;
use constant ASYNC_PDATA_REMOVE                         => 0x00000200;
use constant ASYNC_PDATA_MODIFY                         => 0x00000400;
use constant ASYNC_ISNS_MODIFY                          => 0x00000800;
use constant ASYNC_BUFFER_BOARD_CHANGE                  => 0x00001000;
use constant ASYNC_GLOBAL_CACHE_CHANGE                  => 0x00002000;
use constant ASYNC_PRES_CHANGED                         => 0x00004000;
use constant ASYNC_APOOL_CHANGED                        => 0x00008000;
use constant X1_ASYNC_VCG_ELECTION_STATE_CHANGE         => 0x00010000;
use constant X1_ASYNC_VCG_ELECTION_STATE_ENDED          => 0x00020000;
use constant X1_ASYNC_VCG_POWERUP                       => 0x00040000;
use constant X1_ASYNC_VCG_CFG_CHANGED                   => 0x00080000;
use constant X1_ASYNC_VCG_WORKSET_CHANGED               => 0x00100000;
# unused entry here.
# unused entry here.
use constant ISE_ENV_CHANGED                            => 0x00800000;
use constant X1_ASYNC_BE_PORT_CHANGE                    => 0x01000000;
use constant X1_ASYNC_FE_PORT_CHANGE                    => 0x02000000;
# unused entry here.
# unused entry here.
use constant ASYNC_PING_EVENT                           => 0x10000000;
use constant LOG_STD_MSG                                => 0x20000000;
use constant LOG_XTD_MSG                                => 0x40000000;
use constant LOG_BIN_MSG                                => 0x80000000;


# genericCommand PI_GENERIC_FAILURE_MANAGER Codes
use constant FAILURE_MANAGER_FAIL_CONTROLLER            => 0x00;
use constant FAILURE_MANAGER_FAIL_INTERFACE             => 0x01;
use constant RESOURCE_MANAGER_UNFAIL_CONTROLLER         => 0x20;
use constant RESOURCE_MANAGER_UNFAIL_INTERFACE          => 0x21;

# generic2Command "cmdCodes"
use constant GET_HEAP_STATS                             => 0; 
use constant GET_TRACE_STATS                            => 1;
use constant GET_PCB_STATS                              => 2;
use constant GET_PROFILE_STATS                          => 3;
use constant GET_PACKET_STATS                           => 4;

# Physical Disks Device Types
use constant PD_DT_UNKNOWN                              => 0x00;
use constant PD_DT_FC_DISK                              => 0x01;
use constant PD_DT_SATA                                 => 0x02;
use constant PD_DT_SSD                                  => 0x03;
use constant PD_DT_ECON_ENT                             => 0x04;
use constant PD_DT_SAS                                  => 0x05;
use constant PD_DT_ISE_HIGH_PERF                        => 0x06;
use constant PD_DT_ISE_PERF                             => 0x07;
use constant PD_DT_ISE_BALANCE                          => 0x08;
use constant PD_DT_ISE_CAPACITY                         => 0x09;
use constant PD_DT_MAX_DISK                             => 0x09;
use constant PD_DT_SES                                  => 0x0D;
use constant PD_DT_UNKNOWN_SES                          => 0x0D;
use constant PD_DT_FC_SES                               => 0x0E;
use constant PD_DT_SATA_SES                             => 0x0F;
use constant PD_DT_SBOD_SES                             => 0x10;
use constant PD_DT_SBOD_SAS_EXP                         => 0x11;
use constant PD_DT_ISE_SES                              => 0x12;
use constant PD_DT_MAX_SES                              => 0x12;

# Devices status Codes for VDisks, RAIDs, PSDs, and PDDs
use constant DEVSTATUS_NONEXISTENT                      => 0x00; 
use constant DEVSTATUS_INOPERABLE                       => 0x01; 
use constant DEVSTATUS_UNINITIALIZED                    => 0x02; 
use constant DEVSTATUS_INITIALIZING                     => 0x03; 
use constant DEVSTATUS_ERROR                            => 0x04; 
use constant DEVSTATUS_OPERATIONAL                      => 0x10; 
use constant DEVSTATUS_DEGRADED                         => 0x11; 
use constant DEVSTATUS_REBUILDING                       => 0x12; 
use constant DEVSTATUS_DEFRAGGING                       => 0x13; 

# Stucture Retrievel Commands
use constant STRUCT_MASTER_CONFIG                       => 0x0001;

# Debug Election States
use constant DEBUG_ED_STATE_END_TASK                        => 0;
use constant DEBUG_ED_STATE_BEGIN_ELECTION                  => 1;
use constant DEBUG_ED_STATE_CHECK_MASTERSHIP_ABILITY        => 2;
use constant DEBUG_ED_STATE_TIMEOUT_CONTROLLERS             => 3;
use constant DEBUG_ED_STATE_TIMEOUT_CONTROLLERS_COMPLETE    => 4;
use constant DEBUG_ED_STATE_CONTACT_ALL_CONTROLLERS         => 5;
use constant DEBUG_ED_STATE_CONTACT_ALL_CONTROLLERS_COMPLETE=> 6;
use constant DEBUG_ED_STATE_WAIT_FOR_MASTER                 => 7;
use constant DEBUG_ED_STATE_CHECK_MASTER                    => 8;
use constant DEBUG_ED_STATE_NOTIFY_SLAVES                   => 9;
use constant DEBUG_ED_STATE_FAILED                          => 10;
use constant DEBUG_ED_STATE_FINISHED                        => 11;

# Failure Data Codes
use constant FD_STATE_UNUSED                                => 0;
use constant FD_STATE_FAILED                                => 1;
use constant FD_STATE_OPERATIONAL                           => 2;
use constant FD_STATE_POR                                   => 3;
use constant FD_STATE_ADD_CONTROLLER_TO_VCG                 => 4;
use constant FD_STATE_STRANDED_CACHE_DATA                   => 5;
use constant FD_STATE_FIRMWARE_UPDATE_INACTIVE              => 6;
use constant FD_STATE_FIRMWARE_UPDATE_ACTIVE                => 7;
use constant FD_STATE_UNFAIL_CONTROLLER                     => 8;
use constant FD_STATE_VCG_SHUTDOWN                          => 9;
use constant FD_STATE_INACTIVATED                           => 10;
use constant FD_STATE_ACTIVATE                              => 11;
use constant FD_STATE_DISASTER_INACTIVE                     => 12;

# Power Up States
use constant POWER_UP_UNKNOWN                               => 0x0000;
use constant POWER_UP_START                                 => 0x0001;
use constant POWER_UP_WAIT_FWV_INCOMPATIBLE                 => 0x0002;
use constant POWER_UP_WAIT_PROC_COMM                        => 0x0004;
use constant POWER_UP_WAIT_CONFIGURATION                    => 0x0005;
use constant POWER_UP_WAIT_LICENSE                          => 0x0008;
use constant POWER_UP_WAIT_DRIVES                           => 0x0010;
use constant POWER_UP_WAIT_DISASTER                         => 0x0015;
use constant POWER_UP_DISCOVER_CONTROLLERS                  => 0x0019;
use constant POWER_UP_WAIT_CONTROLLERS                      => 0x0020;
use constant POWER_UP_PROCESS_BE_INIT                       => 0x0040;
use constant POWER_UP_PROCESS_DISCOVERY                     => 0x0080;
use constant POWER_UP_WAIT_DISK_BAY                         => 0x0100;
use constant POWER_UP_WAIT_CORRUPT_BE_NVRAM                 => 0x0200;
use constant POWER_UP_ALL_CTRL_BE_READY                     => 0x0400;
use constant POWER_UP_PROCESS_R5_RIP                        => 0x0410;
use constant POWER_UP_SIGNAL_SLAVES_RUN_FE                  => 0x0420;
use constant POWER_UP_PROCESS_CACHE_INIT                    => 0x0800;
use constant POWER_UP_WAIT_CACHE_ERROR                      => 0x0810;
use constant POWER_UP_INACTIVE                              => 0x1000;
use constant POWER_UP_FAILED                                => 0x2000;
use constant POWER_UP_WRONG_SLOT                            => 0x2001;
use constant POWER_UP_FAILED_AUTO_NODE_CONFIG               => 0x2002;
use constant POWER_UP_COMPLETE                              => 0xFFFF;

use constant POWER_UP_ASTATUS_UNKNOWN                       => 0x0000;
use constant POWER_UP_ASTATUS_WC_SEQNO_BAD                  => 0x0001;
use constant POWER_UP_ASTATUS_WC_SN_VCG_BAD                 => 0x0002;
use constant POWER_UP_ASTATUS_WC_SN_BAD                     => 0x0003;
use constant POWER_UP_ASTATUS_WC_NVMEM_BAD                  => 0x0004;


# Resource Manager States
use constant RM_NONE                                        => 0;
use constant RM_INIT                                        => 1;
use constant RM_SHUTDOWN                                    => 2;
use constant RM_RUNNING                                     => 3;
use constant RM_BUSY                                        => 4;
use constant RM_DOWN                                        => 5;

# Serial Number Types
use constant CONTROLLER_SN                                  => 1;
use constant SYSTEM_SN                                      => 2;

# CCB Initialization Types
use constant INIT_CCB_NVRAM_TYPE_FULL                       => 0;
use constant INIT_CCB_NVRAM_TYPE_LICENSE                    => 1;

# Reset Qlogic Codes
use constant RESET_QLOGIC_RESET_INITIALIZE                  => 0x00;
use constant RESET_QLOGIC_RESET_ONLY                        => 0x01;
use constant RESET_QLOGIC_RESET_INITIALIZE_IF_OFFLINE       => 0x02;
use constant RESET_QLOGIC_RESET_ONLY_IF_OFFLINE             => 0x03;
use constant RESET_QLOGIC_RESET_INITIALIZE_LOG              => 0xFF;

# Port list options
use constant PORTS_ALL                                      => 0x00;
use constant PORTS_INITIALIZED                              => 0x01;
use constant PORTS_FAILED                                   => 0x02;
use constant PORTS_INITIALIZING                             => 0x03;
use constant PORTS_WITH_TARGETS                             => 0x04;
use constant PORTS_NO_TARGETS                               => 0x05;

# Device Path options and return format
use constant PATH_PHYSICAL_DISK                             => 0x01;
use constant PATH_MISC_DEVICE                               => 0x02;
use constant PATH_ENCLOSURES                                => 0x03;
use constant FORMAT_PID_BITPATH                             => 0x00;
use constant FORMAT_PID_PATH_ARRAY                          => 0x01;
use constant FORMAT_PID_1                                   => 0x02;

use constant DELETE_DISK_DRIVE                              => 0x00;
use constant DELETE_DISK_BAY                                => 0x01;

# I2C Monitor Status Codes
use constant I2C_MONITOR_STATUS_CODE_UNKNOWN                => 0;
use constant I2C_MONITOR_STATUS_CODE_NOT_PRESENT            => 1;
use constant I2C_MONITOR_STATUS_CODE_VALID                  => 2;
use constant I2C_MONITOR_STATUS_CODE_BUSY                   => 3;
use constant I2C_MONITOR_STATUS_CODE_NOT_READY              => 4;
use constant I2C_MONITOR_STATUS_CODE_ERROR                  => 5;

use constant I2C_MONITOR_TEMPERATURE_CONDITION_UNKNOWN      => 0;
use constant I2C_MONITOR_TEMPERATURE_CONDITION_NORMAL       => 1;
use constant I2C_MONITOR_TEMPERATURE_CONDITION_COLD         => 2;
use constant I2C_MONITOR_TEMPERATURE_CONDITION_HOT          => 3;
use constant I2C_MONITOR_TEMPERATURE_CONDITION_COLD_CRITICAL=> 4;
use constant I2C_MONITOR_TEMPERATURE_CONDITION_HOT_CRITICAL => 5;

use constant I2C_MONITOR_LIMIT_MONITOR_UNKNOWN              => 0;
use constant I2C_MONITOR_LIMIT_MONITOR_GOOD                 => 1;
use constant I2C_MONITOR_LIMIT_MONITOR_TRIPPED              => 2;

use constant I2C_MONITOR_POWER_SUPPLY_CONDITION_UNKNOWN     => 0;
use constant I2C_MONITOR_POWER_SUPPLY_CONDITION_GOOD        => 1;
use constant I2C_MONITOR_POWER_SUPPLY_CONDITION_HIGH_TEMPERATURE => 2;
use constant I2C_MONITOR_POWER_SUPPLY_CONDITION_DC_FAILED   => 3;
use constant I2C_MONITOR_POWER_SUPPLY_CONDITION_AC_FAILED   => 4;
use constant I2C_MONITOR_POWER_SUPPLY_CONDITION_INSERTED    => 5;
use constant I2C_MONITOR_POWER_SUPPLY_CONDITION_NOT_PRESENT => 6;

use constant I2C_MONITOR_COOLING_FAN_CONDITION_UNKNOWN      => 0;
use constant I2C_MONITOR_COOLING_FAN_CONDITION_GOOD         => 1;
use constant I2C_MONITOR_COOLING_FAN_CONDITION_FAILED       => 2;
use constant I2C_MONITOR_COOLING_FAN_CONDITION_NOT_PRESENT  => 3;

use constant I2C_MONITOR_PROCESSOR_RESET_CONDITION_UNKNOWN  => 0;
use constant I2C_MONITOR_PROCESSOR_RESET_CONDITION_RUNNING  => 1;
use constant I2C_MONITOR_PROCESSOR_RESET_CONDITION_RESET    => 2;

use constant I2C_MONITOR_BATTERY_CONDITION_UNKNOWN          => 0;
use constant I2C_MONITOR_BATTERY_CONDITION_GOOD             => 1;
use constant I2C_MONITOR_BATTERY_CONDITION_LOW_CAPACITY     => 2;
use constant I2C_MONITOR_BATTERY_CONDITION_UNDER_VOLTAGE    => 3;
use constant I2C_MONITOR_BATTERY_CONDITION_OVER_VOLTAGE     => 4;
use constant I2C_MONITOR_BATTERY_CONDITION_NOT_PRESENT      => 5;

use constant I2C_MONITOR_CURRENT_FLOW_CONDITION_UNKNOWN     => 0;
use constant I2C_MONITOR_CURRENT_FLOW_CONDITION_GOOD        => 1;
use constant I2C_MONITOR_CURRENT_FLOW_CONDITION_ABNORMAL    => 2;

use constant I2C_MONITOR_FUEL_GAUGE_CONDITION_UNKNOWN       => 0;
use constant I2C_MONITOR_FUEL_GAUGE_CONDITION_GOOD          => 1;
use constant I2C_MONITOR_FUEL_GAUGE_CONDITION_SHUTDOWN      => 2;

use constant I2C_MONITOR_MAIN_REGULATOR_CONDITION_UNKNOWN   => 0;
use constant I2C_MONITOR_MAIN_REGULATOR_CONDITION_OPERATIONAL => 1;
use constant I2C_MONITOR_MAIN_REGULATOR_CONDITION_SHUTDOWN_ERROR => 2;
use constant I2C_MONITOR_MAIN_REGULATOR_CONDITION_SHUTDOWN_GOOD => 3;

use constant I2C_MONITOR_CHARGER_CONDITION_UNKNOWN          => 0;
use constant I2C_MONITOR_CHARGER_CONDITION_IDLE             => 1;
use constant I2C_MONITOR_CHARGER_CONDITION_TRICKLE          => 2;
use constant I2C_MONITOR_CHARGER_CONDITION_BULK             => 3;
use constant I2C_MONITOR_CHARGER_CONDITION_OVER             => 4;
use constant I2C_MONITOR_CHARGER_CONDITION_TOPOFF           => 5;

use constant I2C_MONITOR_NVRAM_BATTERY_CONDITION_UNKNOWN    => 0;
use constant I2C_MONITOR_NVRAM_BATTERY_CONDITION_GOOD       => 1;
use constant I2C_MONITOR_NVRAM_BATTERY_CONDITION_FAILED     => 2;

use constant I2C_MONITOR_EEPROM_CONDITION_UNKNOWN           => 0;
use constant I2C_MONITOR_EEPROM_CONDITION_GOOD              => 1;
use constant I2C_MONITOR_EEPROM_CONDITION_BAD_CRC           => 2;
use constant I2C_MONITOR_EEPROM_CONDITION_NOT_READABLE      => 3;

use constant PERSISTENT_DATA_OPTION_READ                    => 0x00;
use constant PERSISTENT_DATA_OPTION_WRITE                   => 0x01;
use constant PERSISTENT_DATA_OPTION_RESET                   => 0x02;
use constant PERSISTENT_DATA_OPTION_CHECKSUM                => 0x03;

# Default port number for the packet interface on the CCB.
use constant PI_TEST_PORT                               => 3100;
use constant PI_DEBUG_PORT                              => 3200;
use constant PI_ASYNC_PORT                              => 3101;
## use constant PI_X1_PORT                                 => 2341;     OBSOLETE

# MD5 key to use on the PI ports (TEST / DEBUG / ASYNC)
use constant PI_MD5_KEY     => "119F4F53B66D81D9D75AFE98C498750A";

use constant CTRL_TYPE_BIGFOOT                              => 0;
use constant CTRL_TYPE_WOOKIEE                              => 1;
use constant CTRL_TYPE_750                                  => 2;
use constant CTRL_TYPE_3100                                 => 3;
use constant CTRL_TYPE_4000                                 => 4;
use constant CTRL_TYPE_7000                                 => 5;
use constant CTRL_TYPE_4700                                 => 6;
use constant CTRL_TYPE_7400                                 => 7;
use constant CTRL_TYPE_UNKNOWN                              => 255;

#################### Loop Primitive MRP constants ####################
# MRP command constants
use constant MRFELOOPPRIMITIVE                              => 0x520;
use constant MRBELOOPPRIMITIVE                              => 0x14C; 

# option constants
use constant MLPRESLOOP                                     => 0;
use constant MLPRESLIDPORT                                  => 1;
use constant MLPSIDPIDRES                                   => 2;
use constant MLPLOGINLID                                    => 0x11;                   
use constant MLPLOGINPID                                    => 0x12;                                 
use constant MLPLOGOUTLID                                   => 0x21;                     
use constant MLPLOGOUTPID                                   => 0x22;

######################################################################

################ Get Raid Device Info MRP constants ##################

use constant MIRXSIZ                                        => 8;

######################################################################
# Device Name options
use constant MNDSERVER                                      => 0x0000;
use constant MNDVDISK                                       => 0x0001;
use constant MNDVCG                                         => 0x0002;
use constant MNDRETVCG                                      => 0x0003;

######################################################################
# Map Lun Map options
use constant MMLMAP                                         => 0x0000;
use constant MMLSWAP                                        => 0x0001;
use constant MMLCOPY                                        => 0x0002;
use constant MMLMOVE                                        => 0x0003;
                                              

##############################################################################
# Name:     encrypt
#
# Desc:     Encrypts data
#
# Input:    Data to encrypt, length of the data
#
# Output:   Encrpyted data...what else!
##############################################################################
sub encrypt
{
    my ($data, $len) = @_;
    my $encryptData = $data;

    logMsg("encrypt...begin\n");

    return $encryptData;
}

##############################################################################
# Name:     decrypt
#
# Desc:     Decrypts data
#
# Input:    Data to decrypt, length of the data
#
# Output:   Decrpyted data...what else!
##############################################################################
sub decrypt
{
    my ($data, $len) = @_;
    my $decryptData = $data;

    logMsg("decrypt...begin\n");

    return $decryptData;
}

##############################################################################
# Name:  assembleXiotechPacket
#
# Desc: Constructs a xiotech packet ready to be sent across the network
#
# In:   scalars:
#           $commandCode        xiotech command code
#           $seqNum             sequence number
#           $dataLength         length of data packet
#           $timeStamp          timestamp
#           $data               $data packet
#           $port               port number
#
# !!!!!!!!!!!!!!!! NOTE:    Use undef if you have no data as
#                           $data = 0 is one byte
#
# Returns: scalar with biniary data in it
##############################################################################
sub assembleXiotechPacket
{
    my @copy = @_;
    my ($commandCode, $seqNum, $timeStamp, $data, $port, $pktVersion) = @copy;

    my $md5 = undef;
    my $key = pack("H32", PI_MD5_KEY);

    my $packet;
    my $header;
    my $dataLength = 0;
    my $payloadLength = 0;
    my $headerLength = PACKET_HEADER_SIZE_3100;
    my $protocolVersion = 1;
    my $packetVersion = 1;

    if(defined($pktVersion))
    {
        $packetVersion = $pktVersion;
    }

    if (defined($data))
    {
        $dataLength = length($data);
# payloadLength is the same as dataLength now 
#        $payloadLength = ($dataLength + 15) & 0xFFFFFFF0;
        $payloadLength = $dataLength;
    }

    $header .= pack("L", $headerLength);
    $header .= pack("L", $dataLength);
    $header .= pack("L", $payloadLength);
    $header .= pack("S", $protocolVersion);
    $header .= pack("S", $packetVersion);
    
    $header .= pack("L", $commandCode);
    $header .= pack("L", $seqNum);
    $header .= pack("LL", 0, $timeStamp);
    
    $header .= pack("a3", "");              # rsvd1
    $header .= pack("C", 0);                # status
    $header .= pack("L", 0);                # errorCode
    $header .= pack("a8", "");              # rsvd2

    $header .= pack("S", 0);                # ipc_rsvd0
    $header .= pack("S", 0);                # ipc_rsvd1
    $header .= pack("L", 0);                # ipc_rsvd2
    $header .= pack("a8", "");              # rsvd3
    
    $header .= pack("a32", "");             # rsvd4

    #Calculate md5 for data
    if( $dataLength )
    {
        $md5 = XIOTech::md5->new();
        $md5->add($data);
        $header .= $md5->digest;
    }
    else
    {
        #No data so md5 to zeros
        $header .= pack("a16", "");
    }

    #Calculate md5 for header
    undef  $md5;
    $md5 = XIOTech::md5->new();
    $md5->add($header);
    $md5->add($key);

    $header .=  $md5->digest;

    $packet = encrypt($header, PACKET_HEADER_SIZE_3100);

    # Add the data to the end if it exists (very important to do this)
    if ($dataLength)
    {
        my $padNum = $payloadLength - $dataLength;
        if($padNum) {
            $data .= pack("a$padNum", "");
        }
        $packet .= encrypt($data, $dataLength);
    }

    return $packet;
}

##############################################################################
# Name:  disassembleXiotechHeader
#
# Desc: Returns list of valuse extracted from header
#
# In:   scalar  $packet     Packet to be checked
#
#
# Returns:  A ref to a hash (anonymous) with the following identifiers
#       COMMAND_CODE
#       SEQ_NUM
#       DATA_LENGTH
#       TIME_STAMP
#       STATUS
#       ERROR_CODE
#
##############################################################################
sub disassembleXiotechHeader
{
    my ($header, $port) = @_;

    my ($headerLength,
            $dataLength,
            $payloadLength,
            $protocolVersion,

            $commandCode,
            $seqNum,
            $ignore,
            $timeStamp,    # we only use the lower 4 bytes

            $rsvd1,
            $status,
            $errorCode,
            $rsvd2,

            $ipc_rsvd0,
            $ipc_rsvd1,
            $ipc_rsvd2,
            $rsvd3,

            $rsvd4,

            $dataMD5,
            
            $headerMD5) = unpack("LLLL LLLL a3Cla8 SSLa8 a32 a16 a16", $header);

    return {COMMAND_CODE => $commandCode,
            SEQ_NUM => $seqNum,
            DATA_LENGTH => $dataLength,
# payloadLength is the same as dataLength now 
#           PAYLOAD_LENGTH => $payloadLength,
            PAYLOAD_LENGTH => $dataLength,
            TIME_STAMP => $timeStamp,
            STATUS => $status,
            ERROR_CODE => $errorCode,
            PROTOCOL_VER => $protocolVersion,
            HEADER_MD5 => $headerMD5,
            DATA_MD5 => $dataMD5};
}

##############################################################################
# Name:  disassembleXiotechPacket
#
# Desc: Returns list of valuse extracted from header
#
# In:   scalar  $packet     Packet to be checked
#
#
# Returns:  A hash with the following identifiers
#       COMMAND_CODE
#       SEQ_NUM
#       DATA_LENGTH
#       TIME_STAMP
#
##############################################################################
sub disassembleXiotechPacket
{
    my $packet = shift;
    my $header = $packet->{'header'};
    my $data = $packet->{'data'};

    my ($headerLength,
            $dataLength,
            $payloadLength,
            $protocolVersion,

            $commandCode,
            $seqNum,
            $ignore,
            $timeStamp,    # we only use the lower 4 bytes

            $rsvd1,
            $controllerType,
            $status,
            $errorCode,

            $rsvd2,
            $ipc_rsvd0,
            $ipc_rsvd1,
            $ipc_rsvd2,

            $rsvd3,
            
            $rsvd4,

            $dataMD5,
            
            $headerMD5) = unpack("LLLL LLLL a2CCl a8SSL a8 a32 a16 a16", $header);

    if (defined $data) {
        $data = substr($data, 0, $dataLength);
    }

#    logMsg("COMMAND_CODE => $commandCode\n");
#    logMsg("SEQ_NUM => $seqNum\n");
#    logMsg("DATA_LENGTH => $dataLength\n");
#    logMsg("PAYLOAD_LENGTH => $payloadLength\n");
#    logMsg("TIME_STAMP => $timeStamp\n");
#    logMsg("STATUS => $status\n");
#    logMsg("ERROR_CODE => $errorCode\n");
#    logMsg("PROTOCOL_VER => $protocolVersion\n");
#    logMsg("HEADER_MD5 => $headerMD5\n");
#    logMsg("DATA_MD5 => $dataMD5\n");
#    if(defined($data)) {
#        logMsg("DATA => ".length($data)." bytes\n");
#    }
#    else {
#        logMsg("DATA => (undef)\n");
#    }

    return (COMMAND_CODE => $commandCode,
            SEQ_NUM => $seqNum,
            DATA_LENGTH => $dataLength,
# payloadLength is the same as dataLength now 
#            PAYLOAD_LENGTH => $payloadLength,
            PAYLOAD_LENGTH => $dataLength,
            TIME_STAMP => $timeStamp,
            CONTROLLER_TYPE => $controllerType,  # only valid on CONNECT/LOGIN
            STATUS => $status,
            ERROR_CODE => $errorCode,
            PROTOCOL_VER => $protocolVersion,
            HEADER_MD5 => $headerMD5,
            DATA_MD5 => $dataMD5,
            DATA => $data);
}

##############################################################################
# Name:     commandCode
#
# In:       scalar $packet
#
# Returns:  The command code for the packet
#
##############################################################################
sub commandCode
{
    my $data = shift;
    my %parts = disassembleXiotechPacket($data);
    return $parts{COMMAND_CODE};
}

##############################################################################
# Name:     seqNum
#
# In:       scalar $packet
#
# Returns:  The sequnce number for the packet
#
##############################################################################
sub seqNum
{
    my $data = shift;
    my %parts = disassembleXiotechPacket($data);
    return $parts{SEQ_NUM};
}

##############################################################################
# Name:     status
#
# In:       scalar $packet
#
# Returns:  The status for the packet
#
##############################################################################
sub status
{
    my $data = shift;
    my %parts = disassembleXiotechPacket($data);
    return $parts{STATUS};
}

##############################################################################
# Name:     errorCode
#
# In:       scalar $packet
#
# Returns:  The errorCode for the packet
#
##############################################################################
sub errorCode
{
    my $data = shift;
    my %parts = disassembleXiotechPacket($data);
    return $parts{ERROR_CODE};
}

##############################################################################
# Name:     data
#
# In:       scalar $packet
#
# Returns:  The data portion of the packet
#
##############################################################################
sub data
{
    my $data = shift;
    my %parts = disassembleXiotechPacket($data);
    return $parts{DATA};
}

##############################################################################
# Name:  dumpPacket
#
# Desc: Dumps a packet 16 bytes wide in hex
#
# In:   scalar  $data
#       scalar  $out    file handle
#
# Return none
##############################################################################
sub dumpPacket
{
    my $data = shift;
    my $out = shift;
    my $i = 0;

    my $when = localtime;

    if (defined($out))
    {
        if (!defined($data))
        {
            print $out "$when Empty packet *************************\n";
        }
        else
        {
            my $len = length($data);

            printf $out "$when Packet dump *************************\n";
            for ($i = 0; $i < $len; ++$i)
            {
                my $firstByte = substr($data,0,1);
                $data = substr($data, 1);

                my $byte = unpack("C", $firstByte);

                if( !($i % 16) )
                {
                    print $out "\n";
                }

                printf $out "%2x ", "$byte\n";
            }
            print $out "\n";
            print $out "*************** Packet end ***************\n";
        }
    }
}

##############################################################################

1;

##############################################################################
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
