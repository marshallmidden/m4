# $Id: PI_Constants.pm 148937 2010-10-12 16:06:50Z m4 $
##############################################################################
# Xiotech a Seagate Technology
# Copyright (c) 2001  Xiotech
##############################################################################
package XIOTech::PI_Constants;

require Exporter;
@ISA = qw(Exporter);
@EXPORT = qw(
    PI_DEFAULT_PORT
    PI_ASYNC_PORT

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

    CONTROLLER_SN
    SYSTEM_SN
    
    INIT_CCB_NVRAM_TYPE_FULL
    INIT_CCB_NVRAM_TYPE_LICENSE
);

use XIOTech::logMgr;
use strict;

# Default port number for the packet interface on the CCB.
use constant PI_DEFAULT_PORT                                => 3000;
use constant PI_ASYNC_PORT                                  => 3101;

# Status codes returned from the packet interface.
use constant PI_GOOD                                        => 0;
use constant PI_ERROR                                       => 1;
use constant PI_SOCKET_ERROR                                => -1;
use constant PI_IN_PROGRESS                                 => 2;
use constant PI_TIMEOUT                                     => 4;
use constant PI_INVALID_CMD_CODE                            => 5;
use constant PI_MALLOC_ERROR                                => 6;
use constant PI_PARAMETER_ERROR                             => 7;
use constant PI_MASTER_CNT_ERROR                            => 8;
use constant PI_POWER_UP_REQ_ERROR                          => 9;
use constant PI_ELECTION_ERROR                              => 10;
use constant PI_TUNNEL_ERROR                                => 11;
use constant PI_R5_STRIPE_RESYNC_ERROR                      => 12;
use constant PI_LOCAL_RAID_RESYNC_ERROR                     => 13;
use constant PI_INVALID_PACKETVERSION_ERROR                 => 14;
use constant PI_COMPAT_INDEX_NOT_SUPPORTED                  => 15;
# If another PI_xxx_ERROR is added, please change the ECODES constant 
# in errorCodes.pm.

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

# Serial Number Types
use constant CONTROLLER_SN                                  => 1;
use constant SYSTEM_SN                                      => 2;

# CCB Initialization Types
use constant INIT_CCB_NVRAM_TYPE_FULL                       => 0;
use constant INIT_CCB_NVRAM_TYPE_LICENSE                    => 1;

1;

##############################################################################
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
