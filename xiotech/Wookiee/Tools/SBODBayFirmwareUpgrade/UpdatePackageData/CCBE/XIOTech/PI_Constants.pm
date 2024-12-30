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
package XIOTech::PI_Constants;

require Exporter;
@ISA = qw(Exporter);
@EXPORT = qw(
    PI_DEFAULT_PORT
    PI_ASYNC_PORT

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
use constant PI_DEFAULT_PORT                                => 3100;
use constant PI_ASYNC_PORT                                  => 3101;

# Status codes returned from the packet interface.
use constant PI_GOOD                                        => 0;
use constant PI_ERROR                                       => 1;
use constant PI_IN_PROGRESS                                 => 2;
use constant PI_TIMEOUT                                     => 4;
use constant PI_INVALID_CMD_CODE                            => 5;
use constant PI_SOCKET_ERROR                                => 6;
use constant PI_PARAMETER_ERROR                             => 7;
use constant PI_MASTER_CNT_ERROR                            => 8;
use constant PI_POWER_UP_REQ_ERROR                          => 9;
use constant PI_ELECTION_ERROR                              => 10;
use constant PI_R5_STRIPE_RESYNC_ERROR                      => 11;
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
# $Log$
# Revision 1.4  2006/09/15 06:23:03  BharadwajS
# TBolt00015295 adding codes for new power up state
#
# Revision 1.3  2006/07/17 20:38:32  RustadM
# TBolt00014770
# Move 750 branch onto main.
#
# Revision 1.2.22.2  2006/05/18 09:19:51  BharadwajS
# Changes to add state POWER_UP_CONFIGURATION to wait for configuration
#
# Revision 1.2.22.1  2006/04/26 07:24:46  BharadwajS
# Changes for PI Versioning
#
# Revision 1.2  2005/06/17 20:00:35  RysavyR
# TBolt00013002: Fixed the return codes on writebuf and scsicmd so that they return an "ecode" instead of an MRP return code. Rev by Bryan Holty.
#
# Revision 1.1.1.1  2005/05/04 18:53:54  RysavyR
# import CT1_BR to shared/Wookiee
#
# Revision 1.17  2005/04/18 21:09:53  NigburC
# TBolt00011442 - Added more work for the power up cache error handling.
# Reviewed by Lynn Waggie.
#
# Revision 1.16  2005/03/17 22:59:02  NigburC
# TBolt00011442 - Added the first pass at the power-up changes to support
# the handling of cache initialization errors.  This added new power-up states
# for controller discovery (not related to write cache), cache initialization and
# cache error.
# Reviewed by Mark Schibilla.
#
# Revision 1.15  2003/08/26 19:30:35  NigburC
# TBolt00008602 - Added logic to the power-up sequencing to start handling
# the disaster detection and recovery scenarios.
# Reviewed by Mike McMaster.
#
# Revision 1.14  2003/08/25 21:43:36  McmasterM
# TBolt00008602: GeoRAID: Add "disaster mode" recovery state to CCB startup
# Changed the way the disaster safeguard was being set and reset by the system.
# This work is done to support Chris while he tests the powerup changes, but does
# not completely finish off this defect.
#
# Revision 1.13  2003/08/05 18:03:50  NigburC
# TBolt00008575 - Change the name of two power-up states (BE_READY and
# DISCOVERY) to make them more descriptive for what they do and added
# three additional power-up states for the updated RAID 5 processing.  Added
# a new function to convert the power-up state to a string value.
# Reviewed by Craig Menning.
#
# Revision 1.12  2003/07/21 13:13:09  NigburC
# TBolt00008575 - Added the additional paramter validation error to the CCBE
# for the stripe resync in progress.
# Reviewed by Randy Rysavy.
#
# Revision 1.11  2002/11/19 22:14:49  NigburC
# TBolt00006138 - Added the new status value to the CCBE code to correctly
# decode the error.
# Reviewed by Jeff Williams.
#
# Revision 1.10  2002/11/15 20:10:55  NigburC
# TBolt00005791 - Added the following new failure data states:
# - FD_STATE_INACTIVATED
# - FD_STATE_ACTIVATE
# Reviewed by Mark Schibilla.
#
# Revision 1.9  2002/11/13 15:55:39  NigburC
# TBolt00006310 - Added new constants for use with the new and improved
# INITCCBNVRAM command.
# Reviewed by Tim Swatosh.
#
# Revision 1.8  2002/10/29 17:43:51  NigburC
# TBolt00000000 - Beginning change to serial number changes, could not
# find the defect for this set of changes.
# Reviewed by Mark Schibilla.
#
# Revision 1.7  2002/07/29 19:52:46  McmasterM
# TBolt00002740: CCB Zion I2C support - SDIMM battery (partial completion)
#
# Revision 1.6  2002/07/29 15:32:17  NigburC
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
# Revision 1.5  2002/07/23 23:10:32  NigburC
# TBolt00004742 - Added power-up state for missing disk bays and moved the
# failed state to 0x2000 to make room for more WAIT states.
# Reviewed by Miles Jagusch.
#
# Revision 1.4  2002/07/16 19:58:52  NigburC
# TBolt00005079, TBolt00005197, TBolt00005196 - Added additional power-up
# states.
# Added code to handle election state changes and when appropriate update
# the VCG information and the VCG status.
# Modified the heartbeat code to only wait until BE is ready before starting local
# heartbeats.
# Reviewed by Mark Schibilla.
#
# Revision 1.3  2002/06/14 12:42:30  NigburC
# TBolt00000665 - Added additional command codes and log events that start
# the integration of the power-up and licensing changes.
# Added new option to PDISKS command in CCBE to display firmware/vendor
# information.
#
# Revision 1.2  2002/04/30 20:04:46  NigburC
# TBolt00004033, TBolt00002733, TBolt00002730 - Lots of changes for these
# three defects.  Mainly, modified the VCGInfo request to return all controllers
# configured as part of the VCG instead of just active controllers.  This caused
# changes in CCB, CCBE and UMC code.
# Added the REMOVE, FAIL, UNFAIL, and SHUTDOWN methods for VCGs.
# Not all of these are working completely...just a stage check-in.
#
# Revision 1.1  2002/01/22 12:47:07  NigburC
# TBolt00002859 - Added new files for packet interface specific items such as
# command codes and constants.  Also added a xioPacket module to handle
# data sent and received from the CCB.
#
##############################################################################
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
