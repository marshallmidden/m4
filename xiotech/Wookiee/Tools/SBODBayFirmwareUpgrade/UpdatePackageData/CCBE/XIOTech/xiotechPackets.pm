# $Header$
##############################################################################
# Xiotech a Seagate Technology
# Copyright (c) 2001  Xiotech
# ======================================================================
# $RCSfile$
# Author: Anthony Asleson
#
# Purpose:
#   Common functionality to handle XIOTech packets
##############################################################################
package XIOTech::xiotechPackets;
 
require Exporter;
@ISA = qw(Exporter);
@EXPORT = qw(
    encrypt
    decrypt
    assembleXiotechPacket
    assembleX1Packet
    disassembleXiotechPacket
    disassembleXiotechHeader
    disassembleX1Header
    disassembleX1Packet
    commandCode
    seqNum
    status
    errorCode
    data
    dumpPacket

    PI_GOOD
    PI_ERROR
    PI_IN_PROGRESS
    PI_TIMEOUT
    PI_INVALID_CMD_CODE
    PI_SOCKET_ERROR
    PI_PARAMETER_ERROR
    PI_MASTER_CNT_ERROR
    PI_POWER_UP_REQ_ERROR
    PI_ELECTION_ERROR
    PI_R5_STRIPE_RESYNC_ERROR
    PI_INVALID_PACKETVERSION_ERROR
    PI_COMPAT_INDEX_NOT_SUPPORTED

    PACKET_HEADER_SIZE_3100
    PACKET_HEADER_SIZE_X1

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
    X1_ASYNC_VCG_GEOPOOL_CHANGED
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
    PI_X1_PORT

    PI_MD5_KEY

    CTRL_TYPE_BIGFOOT
    CTRL_TYPE_WOOKIEE
    CTRL_TYPE_750
    CTRL_TYPE_3100 
    CTRL_TYPE_4000 
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
use constant PI_IN_PROGRESS                             => 2;
use constant PI_TIMEOUT                                 => 4;
use constant PI_INVALID_CMD_CODE                        => 5;
use constant PI_SOCKET_ERROR                            => 6;
use constant PI_PARAMETER_ERROR                         => 7;
use constant PI_MASTER_CNT_ERROR                        => 8;
use constant PI_POWER_UP_REQ_ERROR                      => 9;
use constant PI_ELECTION_ERROR                          => 10;
use constant PI_R5_STRIPE_RESYNC_ERROR                  => 11;
use constant PI_INVALID_PACKETVERSION_ERROR             => 14;
use constant PI_COMPAT_INDEX_NOT_SUPPORTED              => 15;
###############################################################################

# Some constants for the packet structure
use constant PACKET_HEADER_SIZE_3100                    => 128;
use constant PACKET_HEADER_SIZE_X1                      => 2;

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
use constant X1_ASYNC_VCG_GEOPOOL_CHANGED               => 0x00200000;
# unused entry here.
# unused entry here.
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
use constant PD_DT_MAX_DISK                             => 0x05;
use constant PD_DT_SES                                  => 0x0D;
use constant PD_DT_UNKNOWN_SES                          => 0x0D;
use constant PD_DT_FC_SES                               => 0x0E;
use constant PD_DT_SATA_SES                             => 0x0F;
use constant PD_DT_SBOD_SES                             => 0x10;
use constant PD_DT_SBOD_SAS_EXP                         => 0x11;
use constant PD_DT_MAX_SES                              => 0x11;

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
use constant PI_X1_PORT                                 => 2341;

# MD5 key to use on the PI ports (TEST / DEBUG / ASYNC)
use constant PI_MD5_KEY     => "119F4F53B66D81D9D75AFE98C498750A";

use constant CTRL_TYPE_BIGFOOT                              => 0;
use constant CTRL_TYPE_WOOKIEE                              => 1;
use constant CTRL_TYPE_750                                  => 2;
use constant CTRL_TYPE_3100                                 => 3;
use constant CTRL_TYPE_4000                                 => 4;
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

    if ($port == PI_X1_PORT) {
        $packet = assembleX1Packet(X1PKT_BIGFOOT_CMD, $packet);
    }

    return $packet;
}

##############################################################################
# Name:  assembleX1Packet
#
# Desc: Constructs a xiotech packet ready to be sent across the network
#
# In:   scalars:
#           $commandCode        xiotech command code
#           $data               $data packet
#
# !!!!!!!!!!!!!!!! NOTE:    Use undef if you have no data as
#                           $data = 0 is one byte
#
# Returns: scalar with binary data in it
##############################################################################
sub assembleX1Packet
{
    my ($commandCode, $data) = @_;

    my $packet;
    my $length = 1; # for command code

    if (defined($data)) {
        $length += length($data);
    }

    logMsg("len = $length,  cc = $commandCode\n");
    $packet .= pack("SC", $length, $commandCode);

    # Add the data to the end if it exists
    if (defined($data)) {
        $packet .= $data;
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
# Name:  disassembleX1Header
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
sub disassembleX1Header
{
    my ($header, $port) = @_;

    my ($length) = unpack("S", $header);

    return {LENGTH => $length};
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
# Name:  disassembleX1Packet
#
# Desc: Returns list of valuse extracted from header
#
# In:   scalar  $packet     Packet to be checked
#
#
# Returns:  A hash with the following identifiers
#       COMMAND_CODE
#       DATA_LENGTH
#
##############################################################################
sub disassembleX1Packet
{
    my $packet = shift;
    my $header = $packet->{'command'};
    my $data = $packet->{'data'};

    return (DATA => $data,
            HEADER => $header);
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

1;

##############################################################################
#
# Change log:
# $Log$
# Revision 1.3  2006/09/15 06:23:03  BharadwajS
# TBolt00015295 adding codes for new power up state
#
# Revision 1.2  2006/07/17 20:38:33  RustadM
# TBolt00014770
# Move 750 branch onto main.
#
# Revision 1.1.1.1.30.8  2006/06/27 13:33:25  wirtzs
# TBOLT00000000
# removed devtype PD_DT_STP replaced with PD_DT_SATA
#
# Revision 1.1.1.1.30.7  2006/06/05 18:52:35  HoltyB
# TBolt00000000:Fixed controller type scripting issues with adding new 750 type
#
# Revision 1.1.1.1.30.6  2006/05/18 09:19:51  BharadwajS
# Changes to add state POWER_UP_CONFIGURATION to wait for configuration
#
# Revision 1.1.1.1.30.5  2006/04/26 07:24:46  BharadwajS
# Changes for PI Versioning
#
# Revision 1.1.1.1.30.4  2006/04/26 06:07:20  BharadwajS
# Handshaking and versioning
#
# Revision 1.1.1.1.30.3  2006/04/25 10:57:10  BharadwajS
# Changes for Versioning
#
# Revision 1.1.1.1.30.2  2006/04/18 12:28:37  RustadM
# Big commit for 750. New kernel, new boot script, new format for FEDEVS,
# initial SATA support (still trouble with expanders), new default IP addresses
# for the 750, new powerup state for a controller in the wrong slot. I will
# work on getting the 3000 right after getting a CD of this build for the 750.
#
# Revision 1.1.1.1.30.1  2006/04/10 19:11:18  wirtzs
# updates for 750
#
# Revision 1.1.1.1  2005/05/04 18:53:56  RysavyR
# import CT1_BR to shared/Wookiee
#
# Revision 1.129  2005/04/18 21:09:53  NigburC
# TBolt00011442 - Added more work for the power up cache error handling.
# Reviewed by Lynn Waggie.
#
# Revision 1.128  2005/04/04 20:10:22  WilliamsJ
# TBolt00000000 - adding SBOD support.
#
# Revision 1.127  2005/03/17 22:58:39  NigburC
# TBolt00011442 - Added the first pass at the power-up changes to support
# the handling of cache initialization errors.  This added new power-up states
# for controller discovery (not related to write cache), cache initialization and
# cache error.
# Reviewed by Mark Schibilla.
#
# Revision 1.126.40.1  2005/03/28 17:31:37  WilliamsJ
# TBolt00000000 - Updated SBOD code.  Reviewed by Chris.
#
# Revision 1.126  2004/06/04 15:17:37  RysavyR
# TBolt00000000: Updates for Wookiee controller co-existence.  Also added
# SETCONTROLLERTYPE to manually set the current connection to a specific
# type of controller (necessary, for now anyway, when logging in on the 2341 port).
#
# Revision 1.125  2004/05/12 19:55:55  NigburC
# TBolt00010441 - Modified the constants for the platform code.  This also
# includes the addition of the 300 Gig enterprise economy device information.
# Reviewed by Tim Swatosh.
#
# Revision 1.124  2004/04/29 18:07:17  NigburC
# TBolt00010427 - Added code to support economy enterprise drives.
# Reviewed by Jeff Williams.
#
# Revision 1.123  2004/04/29 15:55:14  NigburC
# TBolt00010427 - Added code to support economy enterprise drives.
# Reviewed by Jeff Williams.
#
# Revision 1.122  2004/04/28 18:17:38  HoltyB
# TBolt00000000:  Added Wookiee andling for the CCBE amd CCBCL.
#
# Revision 1.121  2004/02/24 19:34:19  NigburC
# TBolt00000000 - Added code to display the device type in the DEVSTAT PD
# and PDISKS FWV requests and to the DISKBAYS request.
# Reviewed by Jeff Williams.
#
# Revision 1.120  2004/02/24 19:05:25  NigburC
# TBolt00000000 - Added additional options to the VDISK create,expand and
# prepare functions to allow the user to specify FC, SATA or SSD disks.
# Reviewed by Jeff Williams.
#
# Revision 1.119  2003/10/24 19:18:07  SchibillaM
# TBolt00009472: Add IMMEDIATE option for use from the GUI.  Also ALL option
# to encapsulate details of specific tests.
#
# Revision 1.118  2003/10/14 14:33:25  McmasterM
# TBolt00009397: Add logic to CCB to gather FCAL counters in background
# Added logic to CCB to collect and process the FCAL counters.  The data is
# stored in several arrays in the CCB DRAM, and are retrievable through the CCBE
# using the command 'fidread 299'.  The snapshot tools and DDR decoder have
# also been modified so that they are able to process the new arrays.
# Portions reviewed by Brett Tollefson
#
# Revision 1.117  2003/08/28 16:30:10  SchibillaM
# TBolt00009060: Remove worksetID from CfgVDisk Select VPort for Server.  Add
# cmd to add-remove a server in a workset.  Reviewed by Bryan.
#
# Revision 1.116  2003/08/27 14:51:39  McmasterM
# TBolt00008602: GeoRAID: Add "disaster mode" recovery state to CCB startup
# Added additional interfaces for testing this feature.
#
# Revision 1.115  2003/08/26 19:30:35  NigburC
# TBolt00008602 - Added logic to the power-up sequencing to start handling
# the disaster detection and recovery scenarios.
# Reviewed by Mike McMaster.
#
# Revision 1.114  2003/08/25 21:43:36  McmasterM
# TBolt00008602: GeoRAID: Add "disaster mode" recovery state to CCB startup
# Changed the way the disaster safeguard was being set and reset by the system.
# This work is done to support Chris while he tests the powerup changes, but does
# not completely finish off this defect.
#
# Revision 1.113  2003/08/12 19:06:39  McmasterM
# TBolt00000000: GeoRaid election changes (network access to CQ down)
# This checkin consists of a first pass of GeoRaid election support.  It is about
# 60 precent complete, and not totally full function.  It does have some crude
# disaster detection logic in place, but does not change current powerup logic.
#
# Revision 1.112  2003/08/05 18:03:50  NigburC
# TBolt00008575 - Change the name of two power-up states (BE_READY and
# DISCOVERY) to make them more descriptive for what they do and added
# three additional power-up states for the updated RAID 5 processing.  Added
# a new function to convert the power-up state to a string value.
# Reviewed by Craig Menning.
#
# Revision 1.111  2003/07/21 13:13:09  NigburC
# TBolt00008575 - Added the additional paramter validation error to the CCBE
# for the stripe resync in progress.
# Reviewed by Randy Rysavy.
#
# Revision 1.110  2003/07/01 21:08:45  McmasterM
# TBolt00008601: GeoRAID: Add "disaster mode" signature to NVRAM
# TBolt00008603: GeoRAID: Add "keep-alive" DCN designation to masterConfig structure
# Added basic level of keepAlive and disasterMode support to the masterConfig and
# NVRAM definitions.  Reviewed by Chris Nigbur
#
# Revision 1.109  2003/06/30 12:37:56  NigburC
# TBolt00008575 - Added additional option to INITPROCNVRAM command
# to allow the clearing of BE NVA records.
# Reviewed by Ed Mole.
#
# Revision 1.108  2003/06/09 20:48:54  HoltyB
# TBolt00008278:  Removed the single path log messages, since these are
# handled by validation anyway.  Now the single path event will kick off a
# storage validation.  Added checks in the path validation to see if a whole
# fibrebay has lost a path(s).  If this is the case, only one validation log message
# will be seen indicating the fibrebay has bad paths.
# Reviewed by Mark Schibilla.
#
# Revision 1.107  2003/05/12 18:25:05  McmasterM
# TBolt00008217: Temperature Log Events incorrect for ProcA/B and Bay
# Changed temperature events to look identical to those on MAG, and added
# the COLD_CRITICAL temperature range (type ERROR).  Made some other
# changes to make monitor log events look like other events.
# Portions reviewed by Tim Swatosh
#
# Revision 1.106  2003/05/07 20:35:51  HoltyB
# TBolt00000000:  Added constant "SCRUB_PC_CLEARLOG_MASK"
# for compatibilty reasons.  May need to pull out later.
#
# Revision 1.105  2003/05/07 13:19:31  HoltyB
# TBolt00007922:  Remove SNMP from the CCBE code.
#
# Revision 1.104  2003/05/05 21:33:50  TeskeJ
# tbolt00008227 - scrubbing changes
# rev by Bryan
#
# Revision 1.103  2003/04/28 20:27:21  McmasterM
# TBolt00007376: Buffer battery "NOT_PRESENT" log message repeated
# TBolt00007097: When one PS is powercycled twice, the DC_FAILED only appears once
# I2C monitor changes to improve log message accuracy and behavior.
#
# Revision 1.102  2003/01/16 22:37:34  HoltyB
# TBolt00006803:  Multiple changes to the way caching works.  Also made
# a queue for X1 async notifications.  Fixes problem of being forked to death.
#
# Revision 1.101  2003/01/15 13:57:25  SchibillaM
# TBolt00006514: Add firmware version info to the X1 config packet.
# Reviewed by Chris.
#
# Revision 1.100  2003/01/07 20:17:11  McmasterM
# TBolt00006501: Add XCI gathering of PS Interface board data
# TBolt00006492: I2C monitor asserting error on boards without PCA9548 switch
#
# Revision 1.99  2002/12/20 21:02:00  NigburC
# TBolt00005368 - Added a RMSTATE command to the CCBE, CCBCL and
# CCB.  This function returns the RMCurrentState value defined in RM.C.
# Reviewed by Craig Menning.
#
# Revision 1.98  2002/12/18 19:07:30  NigburC
# TBolt00006495 - Fixed the constants in the CCBE and CCB.
# Reviewed by Tim Swatosh.
#
# Revision 1.97  2002/12/17 23:36:51  McmasterM
# TBolt00006250: Add support for I2C switch device
# TBolt00006251: Add support for new I2C EEPROMs (component info collection)
# Full switch support and nearly all of the EEPROM support is in place.
#
# Revision 1.96  2002/12/06 21:37:50  NigburC
# TBolt00006392, TBolt00006394, TBolt00006429 - Lots of changes to enjoy.
# - Added code to support the new NAME_DEVICE MRP.
# - Added code to support setting server, vdisk and controller names.
# - Updated the SERVERASSOC and SERVERDELETE commands to allow
# additional options.
# Reviewed by Mark Schibilla.
#
# Revision 1.95  2002/12/05 17:05:44  RysavyR
# TBolt00006420:  Got rid of the extra packet padding necessary for encryption (not
# needed anymore).  Rev by TimSw.
#
# Revision 1.94  2002/11/26 15:30:21  SwatoshT
# Tbolt00006344: Added support for retrieving packet statistics.
#
# Revision 1.93  2002/11/19 22:14:49  NigburC
# TBolt00006138 - Added the new status value to the CCBE code to correctly
# decode the error.
# Reviewed by Jeff Williams.
#
# Revision 1.92  2002/11/15 20:11:43  NigburC
# TBolt00005791 - Added the following new failure data states:
# - FD_STATE_INACTIVATED
# - FD_STATE_ACTIVATE
# Reviewed by Mark Schibilla.
#
# Revision 1.91  2002/11/13 15:55:39  NigburC
# TBolt00006310 - Added new constants for use with the new and improved
# INITCCBNVRAM command.
# Reviewed by Tim Swatosh.
#
# Revision 1.90  2002/11/11 20:56:55  HoltyB
# TBolt00006300:  Added functionality to log a generic text message and
# assign the severity as well.  Also changed the logTextMessage packet
# interface to allow the severity to be passed through.
#
# Revision 1.89  2002/10/30 15:30:17  HoltyB
# TBolt00006236: Added checksum to persistent data
#
# Revision 1.88  2002/10/29 20:39:28  RysavyR
# TBolt00006013: Removed all Blowfish encryption files and calls in the code.
# Removed Big Number stuff since no Diffie-Hellman needed.  Removed the
# secondary encrypted port support.  Rev by Tim Swatosh.
#
# Revision 1.87  2002/10/29 17:43:51  NigburC
# TBolt00000000 - Beginning change to serial number changes, could not
# find the defect for this set of changes.
# Reviewed by Mark Schibilla.
#
# Revision 1.86  2002/10/19 19:14:13  HoltyB
# TBolt00006201:  Added persistent data functionality to CCB.
#
# Revision 1.85  2002/10/16 21:02:42  RysavyR
# TBolt00006136: Added support for X1GETACCOUNT and X1SETACCOUNT.
#
# Revision 1.84  2002/10/01 20:47:14  RysavyR
# TBolt00006013:  More X1 protocol support changes.
#
# Revision 1.83  2002/10/01 19:01:27  RysavyR
# TBolt00006013:  Add the ability to handle and process BF style packets on
# the X1 port. Reviewed by TimSw.
#
# Revision 1.82  2002/08/27 21:43:36  McmasterM
# TBolt00005893: Every time Bigfoot is shutdown logs reflect validation errors
# Validation errors are now changed to warnings, and vcg shutdown no longer
# causes the validation to fail.
# Reviewed by Randy Rysavy
#
# Revision 1.81  2002/08/26 16:14:49  SchibillaM
# TBolt00005369: Implement scrubbing and parity scan changes per Alpha plan.
# Reviewed by Randy Rysavy.
#
# Revision 1.80  2002/07/29 15:32:17  NigburC
# TBolt00004963, TBolt00005119 - Many changes regarding these defects.
# - Additional work on the power-up wait conditions (enabled all wait conditions
# except BE NVRAM corrupt).
# - Fixed statsserver help.
# - Added update license capability to CCB via PI_VCGApplyLicense function.
# - Added new log event code for BE NVRAM Corrupted.
# - Added new power-up state for BE NVRAM corrupted.
# - Identified BE NVRAM corrupt (just don't have a wait condition for it).
# Reviewed by Bryan Holty.
#
# Revision 1.79  2002/07/25 18:58:58  HoltyB
# TBolt00005346:  Finishing touches on VCG Shutdown
#
# Revision 1.78  2002/07/23 23:10:32  NigburC
# TBolt00004742 - Added power-up state for missing disk bays and moved the
# failed state to 0x2000 to make room for more WAIT states.
# Reviewed by Miles Jagusch.
#
# Revision 1.77  2002/07/17 13:08:45  ThiemannE
# Tbolt00004640: Separated PSD rebuild percent complete and PSD astat fields in Get RAID Device Info MRP
# Reviewed by Mark Schibilla.
#
# Revision 1.76  2002/07/16 19:58:52  NigburC
# TBolt00005079, TBolt00005197, TBolt00005196 - Added additional power-up
# states.
# Added code to handle election state changes and when appropriate update
# the VCG information and the VCG status.
# Modified the heartbeat code to only wait until BE is ready before starting local
# heartbeats.
# Reviewed by Mark Schibilla.
#
# Revision 1.75  2002/07/10 01:28:54  HoltyB
# TBolt00005254: Changed Loginfo to display in the current computers local
# time zone.  Also added an option to still display the GMT time.
# TBolt00005248: Added new interface to write a debug message to the
# CCB logs.
#
# Revision 1.74  2002/07/02 14:30:21  McmasterM
# TBolt00002740: CCB Zion I2C support - SDIMM battery (partial completion)
# The code is gathering new environmental data, but it is not yet connected to
# any async events.  These changes impact both boot and runtime code.
# Reviewed by Randy Rysavy
#
# Revision 1.73  2002/07/01 19:36:54  ThiemannE
# Tbolt00005159: Added new FE and BE Loop Primitive MRPs (mrfeloopprimitive and mrbeloopprimitive) - Rev by JT
#
# Revision 1.72  2002/06/18 20:36:42  HoltyB
# TBolt00004524: Added ability to retrieve SOS table for a pdisk
# TBolt00004836: Placed a check in vdiskDelete to check for server
#                            associations before deletion
#
# Revision 1.71  2002/06/14 12:42:30  NigburC
# TBolt00000665 - Added additional command codes and log events that start
# the integration of the power-up and licensing changes.
# Added new option to PDISKS command in CCBE to display firmware/vendor
# information.
#
# Revision 1.70  2002/06/11 20:27:21  HoltyB
# Made changes to support new options for portlist
#
# Revision 1.69  2002/06/05 14:19:06  HoltyB
# TBolt00004480: Added new Packet interface for new BE device path mrp
# TBolt00004647: Added new option to existing opions for resetting Qlogic's
# TBolt00004564: Added new parameter for get port lists
#
# Revision 1.68  2002/05/15 18:34:55  HoltyB
# TBolt00004254: Added the ability to error trap the Front End, Back End, or the
# CCB
#
# Revision 1.67  2002/05/07 19:10:10  McmasterM
# TBolt00002732: VCG Master Elections - Part 1/2
# This has the solution to the ContactAllControllersComplete timeout condition.
# Two new election states were added to support recovery from an election timeout.
# Also, now only the first set of packets are quorumable during the election.
#
# Revision 1.66  2002/04/30 20:04:47  NigburC
# TBolt00004033, TBolt00002733, TBolt00002730 - Lots of changes for these
# three defects.  Mainly, modified the VCGInfo request to return all controllers
# configured as part of the VCG instead of just active controllers.  This caused
# changes in CCB, CCBE and UMC code.
# Added the REMOVE, FAIL, UNFAIL, and SHUTDOWN methods for VCGs.
# Not all of these are working completely...just a stage check-in.
#
# Revision 1.65  2002/04/29 20:13:04  McmasterM
# TBolt00002732: VCG Master Elections - Part 1/2
# Added support for failing and unfailing of a controller, along with changes
# to support checking of mastership capability before deciding on the next
# master controller.  A new election state was added which drove changes to CCBE.
#
# Revision 1.64  2002/04/29 16:49:58  HoltyB
# TBolt00004068:  Added PI_PROC_DEV_DEL_COMMAND_CMD which gives
# the ability to delete a Pdisk or diskBay. Added new command in the ccbe
# called DEVDELETE
#
# Revision 1.63  2002/04/23 15:50:35  HoltyB
# Added new status code PI_MASTER_CNT_ERROR for cases when a
# slave controller is trying to do the masters job.
#
# Revision 1.62  2002/04/05 20:10:58  HoltyB
# TBolt00003560: Added ability to call resource manager through a generic
# command to unfail a contoller or interface
#
# Revision 1.61  2002/04/01 16:47:31  HoltyB
# Added function to retrieve current election state
#
# Revision 1.60  2002/03/25 16:59:16  HoltyB
# TBolt00003442: Added support for SNMP configuration on the ccb to allow
# ip addresses to be sent to the ccb to be used for generating traps
#
# Revision 1.59  2002/03/20 16:16:41  SchibillaM
# TBolt00000000: Return status and error code if FIDREAD fails.  Change packet
# handling code to correctly interpret errorCode as a signed value.
# Reviewed by Randy.
#
# Revision 1.58  2002/03/19 22:45:14  RysavyR
# TBolt00003360:  Add MD5 signing and checking into the packet interface.
# Add in a second listening port (3110) for encrytion development testing.
# For now, only check MD5 on port 3110.
#
# Revision 1.57  2002/03/15 22:22:55  NigburC
# TBolt00003330 - Added the IPC global timeout.
#
# Revision 1.56  2002/03/12 17:50:50  HoltyB
# TBolt00003309: Added a generic command to kick off an election
#
# Revision 1.55  2002/03/07 20:37:49  SchibillaM
# TBolt00003263: Add CCB support to save/restore names to file system.  Total
# rewrite of names.c & .h.  Removed includes of names.h where they were not
# required and added names_old.h to files that just need these definitions to
# compile.  Added PI commands to read and write name FIDs.  Reviewed by Chris.
#
# Revision 1.54  2002/03/04 17:45:53  HoltyB
# Changed initProcNvram to take a parmeter that determines
# the type of init to do
#
# Revision 1.53  2002/03/01 20:22:04  HoltyB
# Added parameters for rescanDevice LIST, LOOP, LUNS
#
# Revision 1.52  2002/02/26 20:46:54  HoltyB
# Added support for ccb packet interface timeout set
#
# Revision 1.51  2002/02/19 19:08:25  HoltyB
# Added DEBUGADR to set the address of the debug console
#
# Revision 1.50  2002/02/13 20:50:57  RysavyR
# TBolt00003070: Changed the Port Server / Packet Interface protocol to use a 128 byte header and to send / receive even multiples of 16 bytes.
#
# Revision 1.49  2002/02/11 19:08:03  HoltyB
# moved command codes to PI_CommandCodes.pm
#
# Revision 1.48  2002/02/05 23:20:09  NigburC
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
# Revision 1.47  2002/01/31 20:40:01  HoltyB
# added new type for structure display
#
# Revision 1.46  2002/01/30 17:24:07  HoltyB
# added new command codes for serverproperties and debug tools
#
# Revision 1.45  2002/01/22 12:51:30  NigburC
# TBolt00002858 - Added the VCGSETMIRRORPARTNERS command to the
# CCBE and CCBCL.
#
# Revision 1.44  2002/01/18 16:20:00  HoltyB
# added status code for PI_PARAMETER_ERROR
#
# Revision 1.43  2002/01/14 21:46:39  RysavyR
# TBolt00002816: Added a generic function call.  The user can code anything
# he wants in PI_GenFunc.c. This function can then be passed 0-8 parameters
# through the command line/packet interface.
#
# Revision 1.42  2002/01/14 19:48:01  HoltyB
# TBolt00002818: added new constants for BE and FE channel reset
#
# Revision 1.41  2002/01/11 22:26:29  HoltyB
# added new constants for disk bay commands and qlogic
# reset for BE and FE
#
# Revision 1.40  2002/01/11 18:24:29  RysavyR
# TBolt00002816: Add a "Generic MRP" command
#
# Revision 1.39  2002/01/09 17:03:21  TeskeJ
# t2668 - broken loop miscompare changes - device status codes changed
# rev by Tim
#
# Revision 1.38  2001/12/11 22:10:40  NigburC
# Added code to handle setting the global MRP timeout.
#
# Revision 1.37  2001/12/10 20:59:27  NigburC
# Added additional packet command codes for statistics packets.
#
# Revision 1.36  2001/12/06 22:41:32  NigburC
# Added RAID_COUNT and RAID_LIST command codes.
# Modified the VLINK command codes.
#
# Revision 1.35  2001/12/03 13:17:27  NigburC
# Added SCRUBINFO and SCRUBSET commands.
#
# Revision 1.34  2001/11/30 20:36:09  NigburC
# Added additional constants.
#
# Revision 1.33  2001/11/30 17:19:54  NigburC
# Added VDISKSETCACHE functionality.
#
# Revision 1.32  2001/11/27 21:31:59  RysavyR
# Added the ability to retrieve CCB boot and runtime fw headers with FWVERSION.
#
# Revision 1.31  2001/11/27 21:14:48  NigburC
# Updated the constants for VDISK commands.
#
# Revision 1.30  2001/11/27 20:21:21  RysavyR
# Added the ability to reset the CCB via the "generic" command handler.
#
# Revision 1.29  2001/11/27 17:40:44  NigburC
# Added the vdiskControl command line handler and it associated functions.
#
# Revision 1.28  2001/11/27 15:56:40  NigburC
# Added additional constants.
# Rearranged the VDISK command constanst to match what changed in the
# packet interface.
#
# Revision 1.27  2001/11/26 21:30:12  NigburC
# Updated the constants for VCGINFO and VCGCONTROLLERINFO.
#
# Revision 1.26  2001/11/26 19:27:46  RysavyR
# Added the GETREPORT command with only the heap statistics/memory leak
# report supported right now..
#
# Revision 1.25  2001/11/15 22:11:30  NigburC
# What do you think I did...added more constants.
#
# Revision 1.24  2001/11/15 19:39:00  NigburC
# Updated constants to include serial number and VCG constants.
#
# Revision 1.23  2001/11/15 14:46:34  NigburC
# Updated the constants to match PacketInteface.H
#
# Revision 1.22  2001/11/15 13:56:39  SwatoshT
# Added support for environmental statistics.
#
# Revision 1.21  2001/11/14 23:28:04  SwatoshT
# Added Log clear command to port 3100.
#
# Revision 1.20  2001/11/14 13:43:29  NigburC
# Updated the assemble/disassemble of the packets to match the changes
# in the packet interface header (removed the MD5 fields, replaced with
# a reserved field).
#
# Revision 1.19  2001/11/14 12:48:07  NigburC
# Added more packet constants and changed the existing ones to match
# the values in PacketInterface.H.
#
# Revision 1.18  2001/11/13 18:49:17  SwatoshT
# Added support for displaying log info on port 3100.
#
# Revision 1.17  2001/11/13 17:28:07  RysavyR
# Added Memory Read/Write functionality
#
# Revision 1.16  2001/11/13 16:06:12  NigburC
# Added the "errorCode" function to retrieve the error code from the header.
# Updated the pack and unpack to use S and L instead of v and V.
#
# Revision 1.15  2001/11/12 22:01:33  NigburC
# Added more constants for labeling physical disks.
#
# Revision 1.14  2001/11/09 14:44:44  RysavyR
# Added ls, cd & pwd commands
#
# Revision 1.13  2001/11/08 13:34:00  NigburC
# Added another constant.
#
# Revision 1.12  2001/11/07 22:19:06  NigburC
# Removed the code for the 3007 port.
#
# Revision 1.11  2001/11/07 13:51:13  NigburC
# Change the usage of the ID tag to HEADER tag.
#
# Revision 1.10  2001/11/06 22:29:33  RysavyR
# Added firmware download capability to the port 3100 interface.
#
# Revision 1.9  2001/11/06 19:14:04  SchibillaM
# Change command codes to hex.
#
# Revision 1.8  2001/11/05 21:53:02  NigburC
# Completed a cleanup (PACKET_PDSIK_INFO to PACKET_PDISK_INFO).
#
# Revision 1.7  2001/11/05 20:53:07  NigburC
# More cleanup work.
# - Moved environmental statistics code to cmStats.pm.
# - Added encrypt/decrypt functionality (stubs).
#
# Revision 1.6  2001/11/05 17:00:14  SwatoshT
# Added support for environmental statistics
#
# Revision 1.5  2001/11/02 20:35:35  NigburC
# Code cleanup time, lots of changes.
#
# Added code handling for the 3100 port and new packet interface.  BigfootCL
# is now able to connect to a machine running the new packet interface and
# send a pdiskCount command.
#
# Revision 1.4  2001/10/31 22:54:37  RysavyR
# Added FWUPDATE capability
#
# Revision 1.3  2001/10/31 17:00:04  NigburC
# Added the PACKET_LOG_CLEAR packet and handlers.
#
# Revision 1.2  2001/10/31 15:42:02  NigburC
# Updated the command line to include the "logInfo" command to display
# the last N log messages.
#
# Revision 1.1.1.1  2001/10/31 12:51:30  NigburC
# Initial integration of Bigfoot command line.
#
#
##############################################################################
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
