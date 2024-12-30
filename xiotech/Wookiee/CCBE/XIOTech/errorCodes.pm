# $Id: errorCodes.pm 159966 2012-10-01 23:20:49Z marshall_midden $
##############################################################################
# Xiotech a Seagate Technology
# Copyright (c) 2001-2006  Xiotech Corporation. All rights reserved.
# ======================================================================
#
# Purpose:
#   Wrapper for all the different XIOTech Error Codes that can be received
#   from the XIOtech SAN system
##############################################################################
package XIOTech::cmdMgr;

use strict;

###############################################################################
# Error Codes for Bigfoot
use constant PI_ERROR_INV_PKT_TYP           => 0x01;  # Invalid packet type
use constant PI_ERROR_INV_PKT_SIZ           => 0x02;  # Invalid packet size
use constant PI_ERROR_BAD_DAM               => 0x03;  # Bad DAM entry
use constant PI_ERROR_LIST_ERROR            => 0x04;  # error getting list
use constant PI_ERROR_2TB_LIMIT             => 0x05;  # 2TB limit exceeded
use constant PI_ERROR_NON_EX_DEV            => 0x06;  # Non-existent device
use constant PI_ERROR_INOP_DEV              => 0x07;  # Inoperative device
use constant PI_ERROR_INV_LAB_TYP           => 0x08;  # Invalid labtype
use constant PI_ERROR_DEV_USED              => 0x09;  # Specified device in use
use constant PI_ERROR_INIT_IN_PROG          => 0x0A;  # Initialization in progress
use constant PI_ERROR_INV_WW_NAME           => 0x0B;  # Invalid world wide name
use constant PI_ERROR_IO_ERROR              => 0x0C;  # I/O error
use constant PI_ERROR_INV_RD_TYP            => 0x0D;  # Invalid RAID type
use constant PI_ERROR_CODE_BURN             => 0x0E;  # Code burn failed
use constant PI_ERROR_DEF_NT_RDY            => 0x0F;  # Define is not ready for full cmd set
use constant PI_ERROR_INS_DEV_CAP           => 0x10;  # Insufficient device capacity
use constant PI_ERROR_SES_IN_PROGRESS       => 0x11;  # SES discovery in progress
use constant PI_ERROR_INV_DRV_CNT           => 0x12;  # Invalid drive count
use constant PI_ERROR_ACT_FRAG              => 0x13;  # Active defragmentation
use constant PI_ERROR_NOT_DATA_LAB          => 0x14;  # Not labeled data device
use constant PI_ERROR_INV_DEPTH             => 0x15;  # Invalid depth
use constant PI_ERROR_INV_STRIPE            => 0x16;  # Invalid stripe size
use constant PI_ERROR_INV_PARITY            => 0x17;  # Invalid parity specified
use constant PI_ERROR_INV_VIRT_ID           => 0x18;  # Invalid virtual ID
use constant PI_ERROR_INV_OP                => 0x19;  # Invalid operation
use constant PI_ERROR_INV_SES_SLOT          => 0x1A;  # Invalid SES or slot
use constant PI_ERROR_BUSY                  => 0x1B;  # Busy
use constant PI_ERROR_MAX_LUNS              => 0x1C;  # Max LUNs mapped
use constant PI_ERROR_NOT_OPR_ID            => 0x1D;  # RAID device not operative
use constant PI_ERROR_MAX_SEGS              => 0x1E;  # Maximum segments exist
use constant PI_ERROR_BAD_NV_REC            => 0x1F;  # nvram p2 record on disk is not valid
use constant PI_ERROR_INSUFF_REDUND         => 0x20;  # insufficent redundancy exists
use constant PI_ERROR_PID_NT_USED           => 0x21;  # PID not used by any RAIDs
use constant PI_ERROR_NO_HOT_SPARE          => 0x22;  # No hotspare drive or insufficient capacity
use constant PI_ERROR_INV_PRI               => 0x23;  # Invalid priority
use constant PI_ERROR_NOT_SYNC              => 0x24;  # Secondary copy no synchronized
use constant PI_ERROR_INS_NVRAM             => 0x25;  # Insufficient NVRAM
use constant PI_ERROR_FOREIGN_DEV           => 0x26;  # Foreign device
use constant PI_ERROR_REBUILD_NOT_REQ       => 0x27;  # Rebuild not required
use constant PI_ERROR_TYPE_MISMATCH         => 0x28;  # Device type mismatch on create/expand
use constant PI_ERROR_INV_OPT               => 0x29;  # Invalid option
use constant PI_ERROR_INV_TID               => 0x2A;  # Invalid target ID
use constant PI_ERROR_INV_CTRL              => 0x2B;  # Invalid controller ID
use constant PI_ERROR_RET_LEN_BAD           => 0x2C;  # Invalid return data space allowance
use constant PI_ERROR_ACT_REBUILD           => 0x2D;  # Active rebuild
use constant PI_ERROR_TOO_MUCH_DATA         => 0x2E;  # Too much data for return
use constant PI_ERROR_INV_SID               => 0x2F;  # Invalid server
use constant PI_ERROR_INV_VID               => 0x30;  # Invalid VID number
use constant PI_ERROR_INV_RID               => 0x31;  # Invalid RID number
use constant PI_ERROR_INV_PID               => 0x32;  # Invalid PID number
use constant PI_ERROR_INS_TABLE             => 0x33;  # Insufficient table space
use constant PI_ERROR_INS_RES               => 0x34;  # Insufficient resources
use constant PI_ERROR_LUN_MAPPED            => 0x35;  # LUN is already mapped
use constant PI_ERROR_INV_CHAN              => 0x36;  # Invalid port number
use constant PI_ERROR_LUN_NOT_MAPPED        => 0x37;  # LUN is not mapped
use constant PI_ERROR_CREC_FAIL             => 0x38;  # Cache recovery failures occured
use constant PI_ERROR_QRESET_FAIL           => 0x39;  # Qlogic chip reset failed
use constant PI_ERROR_UNASS_PATH            => 0x3A;  # Unassigned Path
use constant PI_ERROR_OUT_OPTS              => 0x3B;  # Outstanding ops prevent completion
use constant PI_ERROR_CHECKSUM              => 0x3C;  # Bad checksum
use constant PI_ERROR_FAILED                => 0x3D;  # Operation failed                  
use constant PI_ERROR_CODE_SAME             => 0x3E;  # Code loads are identical          
use constant PI_ERROR_STOP_ZERO             => 0x3F;  # Stop Count is already zero        
use constant PI_ERROR_INS_MEM               => 0x40;  # Insufficient memory for file copy 
use constant PI_ERROR_NO_TARGET             => 0x41;  # No target for ports               
use constant PI_ERROR_NO_PORT               => 0x42;  # No spare ports                    
use constant PI_ERROR_INV_WSID              => 0x43;  # Invalid work set ID               
use constant PI_ERROR_MAXSNAPSHOTS_USED     => 0x44;  # Maximum number of snapshots used            
use constant PI_ERROR_UNUSED2               => 0x45;  #              
use constant PI_ERROR_UNUSED3               => 0x46;  # 
use constant PI_ERROR_IP_OR_SUBNET_ZERO     => 0x47;  # IP or subnet address are 0
use constant PI_ERROR_SET_IP_ERROR          => 0x48;  # Error setting IP address
use constant PI_ERROR_VDISK_IN_USE          => 0x49;  # Vdisk in use on another controller
use constant PI_ERROR_UNUSED4               => 0x4A;  # 
use constant PI_ERROR_BAD_DEV_TYPE          => 0x4B;  # Unknown type used for create/expand
use constant PI_ERROR_EMPTY_CPY_LIST        => 0x50;  # The copy list is empty
use constant PI_ERROR_NO_CPY_MATCH          => 0x51;  # no copy matches the requirments
#ifdef SERVICEABILITY42
use constant PI_ERROR_NO_COMMUNICATION      => 0x52;  # no communication with the mirror partner
use constant PI_ERROR_PID_USED              => 0x53;  # PID is being used by anu RAIDs 
use constant PI_ERROR_PID_SPUNDOWN          => 0x54;  # Designated PID has already been spundown 
use constant PI_ERROR_NO_HSDNAME            => 0x55;  # HSDNAME is not available 
use constant PI_ERROR_NO_FAILBACK           => 0x56;  # No valid drive found for failback 
use constant PI_ERROR_NO_HOTLAB             => 0x57;  # Designated device is not a hotspare 
use constant PI_ERROR_DISK_NOT_INIT         => 0x58;  # Unable to initialize the drive 
use constant PI_ERROR_INV_HSP_TYPE          => 0x59;  # Hotspare not same type as failing drive
use constant PI_ERROR_INV_SET_MAP           => 0x60;  # Invalid iSCSI set map
use constant PI_ERROR_MTU_FAIL              => 0x61;  # Set MTU size fail
use constant PI_ERROR_INV_ISCSI_PARAM       => 0x62;  # Invalid iSCSI Parameter/Value
#endif

#ifdef GR_GEORAID
use constant PI_ASWAP_STATE                                   => 0x63;  # aswap/aswapback in progress / srcvd aswapped
use constant PI_ASWAP_IN_PROGRESS           => 0x64;  # Auto-Swap is in progress 
use constant PI_NO_HS_LOCATION_CODE_MATCH   => 0x65;  # No Hotspare matching location code 
use constant PI_VDISK_NOT_BAY_REDUNDANT     => 0x66;  # VDisk not Bay Redundant

#endif
use constant PI_BAY_NOT_FOUND               => 0x67;  # Bay /ISE not found 
use constant PI_ERROR_DEST_NO_HOTLAB        => 0x68;  # Designated destination device is not a hotspare 
use constant PI_ERROR_64TB_LIMIT            => 0x69;  # 64TB limit exceeded
#
# "ECODES" Error codes used for for SCSI (and related commands)
#
use constant EC_OK              => 0;       # Successful operation
use constant EC_IO_ERR          => 1;       # I/O error
use constant EC_INV_FUNC        => 2;       # Invalid function
use constant EC_NONX_DEV        => 3;       # Nonx device
use constant EC_INOP_VDEV       => 4;       # Inoperative virtual device
use constant EC_INV_SDA         => 5;       # Invalid SDA
use constant EC_INV_LEN         => 6;       # Invalid sector count
use constant EC_IN_VDA          => 7;       # Invalid SDA + length
use constant EC_NULL_SGL        => 8;       # Null S/G list
use constant EC_INV_SGC         => 9;       # Invalid S/G descriptor count
use constant EC_INV_STRAT       => 10;      # Invalid strategy
use constant EC_UNINIT_DEV      => 11;      # Uninitialized device
use constant EC_INV_VID         => 12;      # Invalid virtual device
use constant EC_INV_CLUSTER     => 13;      # Invalid cluster
use constant EC_DEV_RESERVED    => 14;      # Device reserved
use constant EC_COMPARE_ERR     => 15;      # Compare error
use constant EC_INC_SGL         => 16;      # Inconsistent SGL
use constant EC_INV_RID         => 17;      # Invalid raid id
use constant EC_CHECKSUM_ERR    => 18;      # Bad checksum
use constant EC_TIMEOUT         => 65;      # I/O timeout
use constant EC_INV_DEV         => 66;      # Invalid SCSI device
use constant EC_CHECK           => 67;      # SCSI check
use constant EC_INV_RX_ID       => 68;      # Invalid RX_ID
use constant EC_CMD_ABORT       => 69;      # Command aborted
use constant EC_LUN_RESET       => 70;      # LUN reset during command execution
use constant EC_LIP_RESET       => 71;      # Lip reset during command execution
use constant EC_DEV_RESET       => 72;      # Target reset during cmn execution
use constant EC_EVENT           => 73;      # Entry recv while wait for event ack
use constant EC_LGOFF           => 74;      # Port logged off
use constant EC_RESEL_TIMEOUT   => 75;      # Reselection timeout
use constant EC_QUEUE_FULL      => 76;      # Queue Full
use constant EC_DMA             => 77;      # DMA / PCI error
use constant EC_TRANSPORT       => 78;      # Unspecified transport Error
use constant EC_LOGOUT_SENT     => 79;      # Log Out Sent after timeout
use constant EC_BUSY            => 81;      # SCSI busy status
use constant EC_RES_CONFLICT    => 82;      # Reservation Conflict
use constant EC_UNDET_SCSI_STAT => 83;      # Undetermined SCSI status
use constant EC_OVERRUN         => 84;      # Data overrun indicated
use constant EC_UNDERRUN        => 85;      # Data underrun indicated
use constant EC_INV_LLD         => 86;      # Invalid LLD session ID
use constant EC_IOCB_ERR        => 87;      # IOCB returned in error
use constant EC_IOCB_LIP_INT    => 88;      # IOCB returned due to LIP interlock
use constant EC_RST_PORT        => 89;      # IOCB returned due to FW reset port
use constant EC_PORT_FAIL       => 90;      # IOCB returned due to failed port
use constant EC_REDIRECT        => 91;      # Drive has been spared or redirected
use constant EC_DEV_FAIL        => 92;      # A device has failed
use constant EC_LINK_FAIL       => 93;      # PCI link to other processor error
use constant EC_FCAL_IOERR      => 128;     # I/O error indicated by FC-AL driver
use constant EC_DATA_FAULT      => 129;     # Data Path Fault error
use constant EC_RETRY           => 161;     # Unable to Complete, retry later
use constant EC_COPY_COMP       => 224;     # Sec. copy process completed
use constant EC_ABORT           => 254;     # I/O was aborted prior to SRP
use constant EC_CANCEL          => 255;     # Request canceled




# VCG Error Codes
use constant EC_VCG_NO_DRIVES               => 0x1000;
use constant EC_VCG_AS_INVALID_CTRL         => 0x1001;
use constant EC_VCG_AS_FAILED_PING          => 0x1002;
use constant EC_VCG_AS_FAILED_CREATE_CTRL   => 0x1003;
use constant EC_VCG_AS_IPC_ADD_CONTROLLER   => 0x1004;
use constant EC_VCG_IC_BAD_MIRROR_PARTNER   => 0x1005;
use constant EC_VCG_IC_NOT_INACTIVATED      => 0x1006;
use constant EC_VCG_IC_EF                   => 0x1007;
use constant EC_VCG_UC_INVALID_CTRL         => 0x1008;
use constant EC_VCG_IC_INVALID_CTRL         => 0x1009;
use constant EC_VCG_AL_INVALID_STATE        => 0x1010;
use constant EC_VCG_AL_IL_CHG_VCGID         => 0x1011;
use constant EC_VCG_AL_IL_RMV_CFG_CTRL      => 0x1012;
use constant EC_VCG_IC_HOLD_IO              => 0x1013;
use constant EC_VCG_UC_FAILED_PING          => 0x1014;
use constant EC_VCG_UC_WRITE_FAILURE        => 0x1015;
use constant EC_VCG_UC_TIMEOUT              => 0x1016;
use constant EC_VCG_IC_FAILED_PING          => 0x1017;
use constant EC_VCG_UC_TDISCACHE            => 0x1018;
use constant EC_VCG_AC_TDISCACHE            => 0x1019;
use constant EC_VCG_IC_TDISCACHE            => 0x101A;
use constant EC_VCG_INVALID_ADDR            => 0x101B;
use constant EC_VCG_ALREADY_CONFIGURED      => 0x101C;
use constant EC_VCG_UC_BAD_MIRROR_PARTNER   => 0x101D;



# Code Burn Error Codes
use constant FWHEADER_VERIFY_ERROR          => 0x2001;
use constant ERASE_VERIFY_ERROR             => 0x2002;
use constant PROGRAM_VERIFY_ERROR           => 0x2003;
use constant ALLOCATION_ERROR               => 0x2004;
use constant PARM_ERROR                     => 0x2005;
use constant MULTI_MAX_EXCEEDED             => 0x2006;
use constant MALLOC_FAILURE                 => 0x2007;
use constant CCB_FLASH_CORRUPTED            => 0x2008;  # likely fatal...
use constant UPDATE_PROC_TIMEOUT            => 0x2009;
use constant NVRAM_UPDATE_FAILURE           => 0x200A;
use constant PROBABLY_NO_HEADER             => 0x200B;
use constant CCB_FLASH_ADDRESS_ERROR        => 0x200C;

# File System Errors
use constant FS_ERROR_WRITE_NULL_BUFFER                      => 0x2051;
use constant FS_ERROR_WRITE_DIRECT_INIT                      => 0x2052;
use constant FS_ERROR_WRITE_RANGE_LENGTH                     => 0x2053;
use constant FS_ERROR_WRITE_HEADER                           => 0x2054;
use constant FS_ERROR_WRITE_NO_WRITES_HEADER                 => 0x2055;
use constant FS_ERROR_WRITE_HEADER_DATA_SINGLE               => 0x2056;
use constant FS_ERROR_WRITE_NO_WRITES_HEADER_DATA_SINGLE     => 0x2057;
use constant FS_ERROR_WRITE_HEADER_DATA_LOOP                 => 0x2058;
use constant FS_ERROR_WRITE_NO_WRITES_HEADER_DATA_LOOP       => 0x2059;
use constant FS_ERROR_READ_NULL_BUFFER                       => 0x205A;
use constant FS_ERROR_READ_DIRECT_INIT                       => 0x205B;
use constant FS_ERROR_READ_HEADER                            => 0x205C;
use constant FS_ERROR_READ_CRC_CHECK_HEADER                  => 0x205D;
use constant FS_ERROR_READ_MALLOC_DATA                       => 0x205E;
use constant FS_ERROR_READ_DATA                              => 0x205F;
use constant FS_ERROR_READ_CRC_CHECK_DATA                    => 0x2060;
use constant FS_ERROR_NVRAM_READ_PI_TIMEOUT                  => 0x2061;
use constant FS_ERROR_NVRAM_READ                             => 0x2062;
use constant FS_ERROR_NVRAM_WRITE_FID_BE_NVRAM_NO_RESPONSE   => 0x2063;
use constant FS_ERROR_NVRAM_WRITE_FID_BE_NVRAM_PI_TIMEOUT    => 0x2064;
use constant FS_ERROR_NVRAM_WRITE_FID_BE_NVRAM               => 0x2065;
use constant FS_ERROR_NVRAM_WRITE_NO_RESPONSE                => 0x2066;
use constant FS_ERROR_NVRAM_WRITE_PI_TIMEOUT                 => 0x2067;
use constant FS_ERROR_NVRAM_WRITE                            => 0x2068;
use constant FS_ERROR_WRITE_HEADER_PI_TIMEOUT                => 0x2069;
use constant FS_ERROR_WRITE_HEADER_DATA_SINGLE_PI_TIMEOUT    => 0x206A;
use constant FS_ERROR_WRITE_HEADER_DATA_LOOP_PI_TIMEOUT      => 0x206B;
use constant FS_ERROR_READ_HEADER_PI_TIMEOUT                 => 0x206C;
use constant FS_ERROR_READ_DATA_PI_TIMEOUT                   => 0x206D;

use constant PHASE_STATE_SET_FAILURE                         => 0x2100;
use constant PHASE_FAIL_CONTROLLER_FAILURE                   => 0x2101;
use constant PHASE_ELECTION_FAILURE                          => 0x2102;
use constant PHASE_STATE_SET_BAD_MIRROR_PARTNER              => 0x2103;
use constant PHASE_STATE_SET_NON_MIRRORED_RAID5S             => 0x2104;
use constant PHASE_STATE_SET_PING_FAILURE                    => 0x2105;
use constant PHASE_RAIDS_NOT_READY                           => 0x2106;
use constant PHASE_STOP_IO_FAILED                            => 0x2107;
use constant PHASE_RESET_QLOGIC_FAILED                       => 0x2108;
use constant PHASE_TDISCACHE_FAILED                          => 0x2109;

use constant PDATA_TOO_MUCH_DATA                             => 0x2210;
use constant PDATA_OUT_OF_RANGE                              => 0x2211;
use constant PDATA_INVALID_OPTION                            => 0x2212;

use constant PDATA_RECORD_NOT_FOUND                          => 0x2213;
use constant PDATA_DUPLICATE_RECORD                          => 0x2214;
use constant PDATA_FILESYSTEM_ERROR                          => 0x2215;
use constant PDATA_CANNOT_ALLOCATE_BUFFER                    => 0x2216;
use constant PDATA_EOF_REACHED                               => 0x2217;
use constant PDATA_INVALID_RECORD                            => 0x2218;

# PI_VCGShutdown Error Codes
use constant VCG_SHUTDOWN_ERROR_WRITE_STATE_TO_QUORUM        => 0x2301;
use constant VCG_SHUTDOWN_ERROR_FE_SDIMM_SHUTDOWN            => 0x2302;
use constant VCG_SHUTDOWN_ERROR_BE_SDIMM_SHUTDOWN            => 0x2304;

# Miscellaneous Error Codes
use constant PI_MD5_ERROR                                    => 0x3000;
use constant EC_UNLABEL_ALL_OWNED                            => 0x3001;

###############################################################################

my @msgErrCode;
$msgErrCode[PI_ERROR][PI_ERROR_INV_PKT_TYP]
    = "Invalid packet type";

$msgErrCode[PI_ERROR][PI_ERROR_INV_PKT_SIZ]
    = "Invalid packet size";

$msgErrCode[PI_ERROR][PI_ERROR_BAD_DAM]
    = "Bad DAM entry";

$msgErrCode[PI_ERROR][PI_ERROR_LIST_ERROR]
    = "Error getting list";

$msgErrCode[PI_ERROR][PI_ERROR_2TB_LIMIT]
    = "2TB limit exceeded";

$msgErrCode[PI_ERROR][PI_ERROR_64TB_LIMIT]
    = "64TB limit exceeded";

$msgErrCode[PI_ERROR][PI_ERROR_NON_EX_DEV]
    = "Non-existent device";

$msgErrCode[PI_ERROR][PI_ERROR_INOP_DEV]
    = "Inoperative device";

$msgErrCode[PI_ERROR][PI_ERROR_INV_LAB_TYP]
    = "Invalid labtype";

$msgErrCode[PI_ERROR][PI_ERROR_DEV_USED]
    = "Specified device in use";

$msgErrCode[PI_ERROR][PI_ERROR_INIT_IN_PROG]
    = "Initialization in progress";

$msgErrCode[PI_ERROR][PI_ERROR_INV_WW_NAME]
    = "Invalid world wide name";

$msgErrCode[PI_ERROR][PI_ERROR_IO_ERROR]
     = "I/O error";

$msgErrCode[PI_ERROR][PI_ERROR_INV_RD_TYP]
    = "Invalid RAID type";

$msgErrCode[PI_ERROR][PI_ERROR_CODE_BURN]
    = "Code burn failed";

$msgErrCode[PI_ERROR][PI_ERROR_DEF_NT_RDY]
    = "Define is not ready for full cmd set";

$msgErrCode[PI_ERROR][PI_ERROR_INS_DEV_CAP]
    = "Insufficient device capacity";

$msgErrCode[PI_ERROR][PI_ERROR_SES_IN_PROGRESS]
    = "SES discovery in progress";

$msgErrCode[PI_ERROR][PI_ERROR_INV_DRV_CNT]
    = "Invalid drive count";

$msgErrCode[PI_ERROR][PI_ERROR_ACT_FRAG]
    = "Active defragmentation";

$msgErrCode[PI_ERROR][PI_ERROR_NOT_DATA_LAB]
    = "Not labeled data device";

$msgErrCode[PI_ERROR][PI_ERROR_INV_DEPTH]
    = "Invalid depth";

$msgErrCode[PI_ERROR][PI_ERROR_INV_STRIPE]
    = "Invalid stripe size";

$msgErrCode[PI_ERROR][PI_ERROR_INV_PARITY]
    = "Invalid parity specified";

$msgErrCode[PI_ERROR][PI_ERROR_INV_VIRT_ID]
    = "Invalid virtual ID";

$msgErrCode[PI_ERROR][PI_ERROR_INV_OP]
    = "Invalid operation";

$msgErrCode[PI_ERROR][PI_ERROR_INV_SES_SLOT]
    = "Invalid SES or slot";

$msgErrCode[PI_ERROR][PI_ERROR_BUSY]
    = "Busy";

$msgErrCode[PI_ERROR][PI_ERROR_MAX_LUNS]
    = "Max LUNs mapped";

$msgErrCode[PI_ERROR][PI_ERROR_NOT_OPR_ID]
    = "RAID device not operative";

$msgErrCode[PI_ERROR][PI_ERROR_MAX_SEGS]
    = "Maximum segments exist";

$msgErrCode[PI_ERROR][PI_ERROR_BAD_NV_REC]
    = "nvram p2 record on disk is not valid";

$msgErrCode[PI_ERROR][PI_ERROR_INSUFF_REDUND]
    = "Insufficent redundancy exists";

$msgErrCode[PI_ERROR][PI_ERROR_PID_NT_USED]
    = "PID not used by any RAIDs";

$msgErrCode[PI_ERROR][PI_ERROR_NO_HOT_SPARE]
    = "No hotspare drive or insufficient capacity";

$msgErrCode[PI_ERROR][PI_ERROR_INV_PRI]
    = "Invalid priority";

$msgErrCode[PI_ERROR][PI_ERROR_NOT_SYNC]
    = "Secondary copy no synchronized";

$msgErrCode[PI_ERROR][PI_ERROR_INS_NVRAM]
    = "Insufficient NVRAM";

$msgErrCode[PI_ERROR][PI_ERROR_FOREIGN_DEV]
    = "Foreign device";

$msgErrCode[PI_ERROR][PI_ERROR_REBUILD_NOT_REQ]
    = "Rebuild not required";

$msgErrCode[PI_ERROR][PI_ERROR_TYPE_MISMATCH]
    = "Device type mismatch on create/expand";

$msgErrCode[PI_ERROR][PI_ERROR_INV_OPT]
    = "Invalid option";

$msgErrCode[PI_ERROR][PI_ERROR_INV_TID]
    = "Invalid target ID";

$msgErrCode[PI_ERROR][PI_ERROR_INV_CTRL]
    = "Invalid controller ID";

$msgErrCode[PI_ERROR][PI_ERROR_RET_LEN_BAD]
    = "Invalid return data space allowance";

$msgErrCode[PI_ERROR][PI_ERROR_ACT_REBUILD]
    = "Active rebuild";

$msgErrCode[PI_ERROR][PI_ERROR_TOO_MUCH_DATA]
    = "Too much data for return";

$msgErrCode[PI_ERROR][PI_ERROR_INV_SID]
    = "Invalid server";

$msgErrCode[PI_ERROR][PI_ERROR_INV_VID]
    = "Invalid VID number";

$msgErrCode[PI_ERROR][PI_ERROR_INV_RID]
    = "Invalid RID number";

$msgErrCode[PI_ERROR][PI_ERROR_INV_PID]
    = "Invalid PID number";

$msgErrCode[PI_ERROR][PI_ERROR_INS_TABLE]
    = "Insufficient table space";

$msgErrCode[PI_ERROR][PI_ERROR_INS_RES]
    = "Insufficient resources";

$msgErrCode[PI_ERROR][PI_ERROR_LUN_MAPPED]
    = "LUN is already mapped";

$msgErrCode[PI_ERROR][PI_ERROR_INV_CHAN]
    = "Invalid port number";

$msgErrCode[PI_ERROR][PI_ERROR_LUN_NOT_MAPPED]
    = "LUN is not mapped";

$msgErrCode[PI_ERROR][PI_ERROR_CREC_FAIL]
    = "Cache recovery failures occured";

$msgErrCode[PI_ERROR][PI_ERROR_QRESET_FAIL]
    = "Qlogic chip reset failed";

$msgErrCode[PI_ERROR][PI_ERROR_UNASS_PATH]
    = "Unassigned Path";

$msgErrCode[PI_ERROR][PI_ERROR_OUT_OPTS]
    = "Outstanding ops prevent completion";

$msgErrCode[PI_ERROR][PI_ERROR_CHECKSUM]
    = "Bad checksum";

$msgErrCode[PI_ERROR][PI_ERROR_FAILED]
    = "Operation failed";

$msgErrCode[PI_ERROR][PI_ERROR_CODE_SAME]
    = "Code loads are identical";

$msgErrCode[PI_ERROR][PI_ERROR_STOP_ZERO]
    = "Stop Count is already zero";

$msgErrCode[PI_ERROR][PI_ERROR_INS_MEM]
    = "Insufficient memory for file copy";

$msgErrCode[PI_ERROR][PI_ERROR_NO_TARGET]
    = "No target for ports";

$msgErrCode[PI_ERROR][PI_ERROR_NO_PORT]
    = "No spare ports";

$msgErrCode[PI_ERROR][PI_ERROR_INV_WSID]
    = "Invalid work set ID";

$msgErrCode[PI_ERROR][PI_ERROR_MAXSNAPSHOTS_USED]
    = "Number of Snapshots limit exceeded";

$msgErrCode[PI_ERROR][PI_ERROR_IP_OR_SUBNET_ZERO]
    = "IP or subnet address are 0";

$msgErrCode[PI_ERROR][PI_ERROR_SET_IP_ERROR]
    = "Error setting IP address";

$msgErrCode[PI_ERROR][PI_ERROR_VDISK_IN_USE]
    = "Vdisk in use on another controller";

$msgErrCode[PI_ERROR][PI_ERROR_BAD_DEV_TYPE]
    = "Unknown type used for create/expand";

$msgErrCode[PI_ERROR][PI_ERROR_EMPTY_CPY_LIST]
    = "The copy list is empty";

$msgErrCode[PI_ERROR][PI_ERROR_NO_CPY_MATCH]
    = "no copy matches the requirments";

#ifdef SERVICEABILITY42
$msgErrCode[PI_ERROR][PI_ERROR_NO_COMMUNICATION]
    = "no communication with the mirror partner";

$msgErrCode[PI_ERROR][PI_ERROR_PID_USED]
    = "PID is being used by any RAIDs";

$msgErrCode[PI_ERROR][PI_ERROR_PID_SPUNDOWN]
    = "Designated PID has already been spundown";

$msgErrCode[PI_ERROR][PI_ERROR_NO_HSDNAME]
    = "Failback information not available or failback cancelled/already-done.";

$msgErrCode[PI_ERROR][PI_ERROR_NO_FAILBACK]
    = "No destination drive/valid- drive found for failback";

$msgErrCode[PI_ERROR][PI_ERROR_NO_HOTLAB]
    = "Designated source device is not a hotspare";

$msgErrCode[PI_ERROR][PI_ERROR_DISK_NOT_INIT]
    = "Unable to initialize the drive";

$msgErrCode[PI_ERROR][PI_ERROR_INV_HSP_TYPE]
    = "Specified Hotspare device type is not same as failing device";

$msgErrCode[PI_ERROR][PI_ERROR_INV_ISCSI_PARAM]
    = "Invalid TID/ iSCSI Param/Value";
#endif

$msgErrCode[PI_ERROR][PI_ASWAP_IN_PROGRESS]
    = "Auto-Swap is in Progress. Unable to change GEO location";

$msgErrCode[PI_ERROR][PI_ASWAP_STATE]
    ="Aswap/Aswapback in progress / Srcvd aswapped";

$msgErrCode[PI_ERROR][PI_NO_HS_LOCATION_CODE_MATCH]
    = "No HotSpare exists with the same location code";

$msgErrCode[PI_ERROR][PI_VDISK_NOT_BAY_REDUNDANT]
    = "Given VDisk is NOT bay redundant";

$msgErrCode[PI_ERROR][PI_BAY_NOT_FOUND]
    = "Bay Not Found";

$msgErrCode[PI_ERROR][PI_ERROR_DEST_NO_HOTLAB]
    = "Designated destination device is not a hotspare";


# VCG Error Codes
$msgErrCode[PI_ERROR][EC_VCG_NO_DRIVES]
    = "VCG ERROR - No Drives owned by this VCG";

$msgErrCode[PI_ERROR][EC_VCG_AS_INVALID_CTRL]
    = "VCG Add Slave ERROR - Controller is not licensed for this VCG";

$msgErrCode[PI_ERROR][EC_VCG_AS_FAILED_PING]
    = "VCG Add Slave ERROR - Failed to send an IPC_PING request to the controller";

$msgErrCode[PI_ERROR][EC_VCG_AS_FAILED_CREATE_CTRL]
    = "VCG Add Slave ERROR - Failed to executed the create controller MRP";

$msgErrCode[PI_ERROR][EC_VCG_AS_IPC_ADD_CONTROLLER]
    = "VCG Add Slave ERROR - Failed to send an IPC_ADD_CONTROLLER request to the controller";

$msgErrCode[PI_ERROR][EC_VCG_IC_BAD_MIRROR_PARTNER]
    = "VCG Inactivate Controller ERROR - Bad Mirror Partner.";

$msgErrCode[PI_ERROR][EC_VCG_IC_NOT_INACTIVATED]
    = "VCG Inactivate Controller ERROR - Controller could not be inactivated.";

$msgErrCode[PI_ERROR][EC_VCG_IC_EF]
    = "VCG Inactivate Controller ERROR - Election failed.";

$msgErrCode[PI_ERROR][EC_VCG_UC_INVALID_CTRL]
    = "VCG Unfail Controller ERROR - Unable to read the controllers failure data or controller is not failed";

$msgErrCode[PI_ERROR][EC_VCG_IC_INVALID_CTRL]
    = "VCG Inactivate Controller ERROR - Invalid controller, cannot inactivate another controller.";

$msgErrCode[PI_ERROR][EC_VCG_AL_INVALID_STATE]
    = "VCG Apply License ERROR - Apply License - Invalid power-up state, cannot apply license.";

$msgErrCode[PI_ERROR][EC_VCG_AL_IL_CHG_VCGID]
    = "VCG Apply License ERROR - Invalid license, cannot change the VCGID";

$msgErrCode[PI_ERROR][EC_VCG_AL_IL_RMV_CFG_CTRL]
    = "VCG Apply License ERROR - Invalid license, cannot remove a configured controller";

$msgErrCode[PI_ERROR][EC_VCG_IC_HOLD_IO]
    = "VCG Inactivate Controller ERROR - Resetting front end Qlogic's.";

$msgErrCode[PI_ERROR][EC_VCG_UC_FAILED_PING]
    = "VCG Unfail Controller ERROR - Unable to Ping controller";

$msgErrCode[PI_ERROR][EC_VCG_UC_WRITE_FAILURE]
    = "VCG Unfail Controller ERROR - Unable to write the controllers failure data";

$msgErrCode[PI_ERROR][EC_VCG_UC_TIMEOUT]
    = "VCG Unfail Controller ERROR - Timeout waiting for controller to start power up";

$msgErrCode[PI_ERROR][EC_VCG_IC_FAILED_PING]
    = "VCG Inactivate Controller ERROR - Unable to Ping Mirror Partner";

$msgErrCode[PI_ERROR][EC_VCG_UC_TDISCACHE]
    = "VCG Unfail Controller ERROR - Unable to temp disable cache";

$msgErrCode[PI_ERROR][EC_VCG_AC_TDISCACHE]
    = "VCG Add Controller ERROR - Unable to temp disable cache";

$msgErrCode[PI_ERROR][EC_VCG_IC_TDISCACHE]
    = "VCG Inactivate Controller ERROR - Unable to temp disable cache";

$msgErrCode[PI_ERROR][EC_VCG_ALREADY_CONFIGURED]
    = "VCG Already Configured";

$msgErrCode[PI_ERROR][EC_VCG_UC_BAD_MIRROR_PARTNER]
    = "VCG Unfail Controller ERROR - Bad Mirror Partner.";



# Code Burn Error Codes
$msgErrCode[PI_ERROR][FWHEADER_VERIFY_ERROR]
    = "CODE BURN ERROR - FW Header Verify Error";

$msgErrCode[PI_ERROR][ERASE_VERIFY_ERROR]
    = "CODE BURN ERROR - Erase Verify Error";

$msgErrCode[PI_ERROR][PROGRAM_VERIFY_ERROR]
    = "CODE BURN ERROR - Program Verify Error";

$msgErrCode[PI_ERROR][ALLOCATION_ERROR]
    = "CODE BURN ERROR - Allocation Error";

$msgErrCode[PI_ERROR][PARM_ERROR]
    = "CODE BURN ERROR - Parameter Error";

$msgErrCode[PI_ERROR][MULTI_MAX_EXCEEDED]
    = "CODE BURN ERROR - Multi Max Exceeded";

$msgErrCode[PI_ERROR][MALLOC_FAILURE]
    = "CODE BURN ERROR - Malloc Failure";

$msgErrCode[PI_ERROR][CCB_FLASH_CORRUPTED]
    = "CODE BURN ERROR - CCB Flash Corrupted";

$msgErrCode[PI_ERROR][UPDATE_PROC_TIMEOUT]
    = "CODE BURN ERROR - Update Proc Timeout";

$msgErrCode[PI_ERROR][NVRAM_UPDATE_FAILURE]
    = "CODE BURN ERROR - NVRAM Update Failure";

$msgErrCode[PI_ERROR][PROBABLY_NO_HEADER]
    = "CODE BURN ERROR - Probably No Header";

$msgErrCode[PI_ERROR][CCB_FLASH_ADDRESS_ERROR]
    = "CODE BURN ERROR - CCB Flash Address Error";

# Rolling update "set phase" Errors
$msgErrCode[PI_ERROR][PHASE_STATE_SET_FAILURE]
    = "ROLLING UPDATE ERROR - Couldn't set \"FW Update Inactive\" state";

$msgErrCode[PI_ERROR][PHASE_FAIL_CONTROLLER_FAILURE]
    = "ROLLING UPDATE ERROR - Unused Error Code (?)";

$msgErrCode[PI_ERROR][PHASE_ELECTION_FAILURE]
    = "ROLLING UPDATE ERROR - Election Failed";

$msgErrCode[PI_ERROR][PHASE_STATE_SET_BAD_MIRROR_PARTNER]
    = "ROLLING UPDATE ERROR - Bad or no mirror partner";

$msgErrCode[PI_ERROR][PHASE_STATE_SET_NON_MIRRORED_RAID5S]
    = "ROLLING UPDATE ERROR - Non-mirrored Raid5's found";

$msgErrCode[PI_ERROR][PHASE_STATE_SET_PING_FAILURE]
    = "ROLLING UPDATE ERROR - Failed to ping mirror partner";

$msgErrCode[PI_ERROR][PHASE_RAIDS_NOT_READY]
    = "ROLLING UPDATE ERROR - Raids not ready";

$msgErrCode[PI_ERROR][PHASE_STOP_IO_FAILED]
    = "ROLLING UPDATE ERROR - Failed to stop IO";

$msgErrCode[PI_ERROR][PHASE_RESET_QLOGIC_FAILED]
    = "ROLLING UPDATE ERROR - Failed to reset qlogic";

$msgErrCode[PI_ERROR][PHASE_TDISCACHE_FAILED]
    = "ROLLING UPDATE ERROR - Failed to temp disable cache";


# File System Errors
$msgErrCode[PI_ERROR][FS_ERROR_WRITE_NULL_BUFFER]
    = "FILE SYSTEM ERROR - NULL Buffer for Write";

$msgErrCode[PI_ERROR][FS_ERROR_WRITE_DIRECT_INIT]
    = "FILE SYSTEM ERROR - Initialize Directories for Write";

$msgErrCode[PI_ERROR][FS_ERROR_WRITE_RANGE_LENGTH]
    = "FILE SYSTEM ERROR - Length Out of Fid Range for Write";

$msgErrCode[PI_ERROR][FS_ERROR_WRITE_HEADER]
    = "FILE SYSTEM ERROR - Error Writing Header";

$msgErrCode[PI_ERROR][FS_ERROR_WRITE_NO_WRITES_HEADER]
    = "FILE SYSTEM ERROR - No Disks Written When Writing Header";

$msgErrCode[PI_ERROR][FS_ERROR_WRITE_HEADER_DATA_SINGLE]
    = "FILE SYSTEM ERROR - Error Writing Header and Data With One Write";

$msgErrCode[PI_ERROR][FS_ERROR_WRITE_NO_WRITES_HEADER_DATA_SINGLE]
    = "FILE SYSTEM ERROR - No Disks Written When Writing Header and Data With One Write";

$msgErrCode[PI_ERROR][FS_ERROR_WRITE_HEADER_DATA_LOOP]
    = "FILE SYSTEM ERROR - Error Writing Header and Data With Multiple Writes";

$msgErrCode[PI_ERROR][FS_ERROR_WRITE_NO_WRITES_HEADER_DATA_LOOP]
    = "FILE SYSTEM ERROR - No Disks Written When Writing Header and Data With Multiple Writes";

$msgErrCode[PI_ERROR][FS_ERROR_WRITE_HEADER_PI_TIMEOUT]
    = "FILE SYSTEM ERROR - Timeout While Writing Header";

$msgErrCode[PI_ERROR][FS_ERROR_WRITE_HEADER_DATA_SINGLE_PI_TIMEOUT]
    = "FILE SYSTEM ERROR - Timeout While Writing Data With One Write";

$msgErrCode[PI_ERROR][FS_ERROR_WRITE_HEADER_DATA_LOOP_PI_TIMEOUT]
    = "FILE SYSTEM ERROR - Timeout While Writing Data With Multiple Writes";

$msgErrCode[PI_ERROR][FS_ERROR_READ_NULL_BUFFER]
    = "FILE SYSTEM ERROR - NULL Buffer for Read";

$msgErrCode[PI_ERROR][FS_ERROR_READ_DIRECT_INIT]
    = "FILE SYSTEM ERROR - Initialize Directories for Read";

$msgErrCode[PI_ERROR][FS_ERROR_READ_HEADER]
    = "FILE SYSTEM ERROR - Error Reading Header";

$msgErrCode[PI_ERROR][FS_ERROR_READ_CRC_CHECK_HEADER]
    = "FILE SYSTEM ERROR - CRC Error Read Header";

$msgErrCode[PI_ERROR][FS_ERROR_READ_MALLOC_DATA]
    = "FILE SYSTEM ERROR - Malloc Error";

$msgErrCode[PI_ERROR][FS_ERROR_READ_DATA]
    = "FILE SYSTEM ERROR - Error Reading Data";

$msgErrCode[PI_ERROR][FS_ERROR_READ_CRC_CHECK_DATA]
    = "FILE SYSTEM ERROR - CRC Error Read Data";

$msgErrCode[PI_ERROR][FS_ERROR_READ_HEADER_PI_TIMEOUT]
    = "FILE SYSTEM ERROR - Timeout While Reading Header";

$msgErrCode[PI_ERROR][FS_ERROR_READ_DATA_PI_TIMEOUT]
    = "FILE SYSTEM ERROR - Timeout While Reading Data";

$msgErrCode[PI_ERROR][FS_ERROR_NVRAM_READ_PI_TIMEOUT]
    = "FILE SYSTEM ERROR - NVRAM Read Timeout";

$msgErrCode[PI_ERROR][FS_ERROR_NVRAM_READ]
    = "FILE SYSTEM ERROR - Error Reading NVRAM";

$msgErrCode[PI_ERROR][FS_ERROR_NVRAM_WRITE_FID_BE_NVRAM_NO_RESPONSE]
    = "FILE SYSTEM ERROR - fid == FID_BE_NVRAM - NVRAM Write No Response";

$msgErrCode[PI_ERROR][FS_ERROR_NVRAM_WRITE_FID_BE_NVRAM_PI_TIMEOUT]
    = "FILE SYSTEM ERROR - fid == FID_BE_NVRAM - NVRAM Write Timeout";

$msgErrCode[PI_ERROR][FS_ERROR_NVRAM_WRITE_FID_BE_NVRAM]
    = "FILE SYSTEM ERROR - fid == FID_BE_NVRAM - NVRAM Write Error";

$msgErrCode[PI_ERROR][FS_ERROR_NVRAM_WRITE_NO_RESPONSE]
    = "FILE SYSTEM ERROR - fid != FID_BE_NVRAM - NVRAM Write No Response";

$msgErrCode[PI_ERROR][FS_ERROR_NVRAM_WRITE_PI_TIMEOUT]
    = "FILE SYSTEM ERROR - fid != FID_BE_NVRAM - NVRAM Write Timeout";

$msgErrCode[PI_ERROR][FS_ERROR_NVRAM_WRITE]
    = "FILE SYSTEM ERROR - fid != FID_BE_NVRAM - NVRAM Write Error";

$msgErrCode[PI_ERROR][PDATA_TOO_MUCH_DATA]
    = "PERSISTENT DATA ERROR - Tried to write or read too much data";

$msgErrCode[PI_ERROR][PDATA_OUT_OF_RANGE]
    = "PERSISTENT DATA ERROR - Tried to access memory out of range";

$msgErrCode[PI_ERROR][PDATA_INVALID_OPTION]
    = "PERSISTENT DATA ERROR - Invalid option";

$msgErrCode[PI_ERROR][PDATA_RECORD_NOT_FOUND]
    = "PERSISTENT DATA ERROR - Record Inexistent";

$msgErrCode[PI_ERROR][PDATA_DUPLICATE_RECORD]
    = "PERSISTENT DATA ERROR - Duplicate Record";

$msgErrCode[PI_ERROR][PDATA_FILESYSTEM_ERROR]
    = "PERSISTENT DATA ERROR - Filesystem error";

$msgErrCode[PI_ERROR][PDATA_CANNOT_ALLOCATE_BUFFER]
    = "PERSISTENT DATA ERROR - Cannot Allocate Buffer";

$msgErrCode[PI_ERROR][PDATA_EOF_REACHED]
    = "PERSISTENT DATA ERROR - EOF reached";

$msgErrCode[PI_ERROR][PDATA_INVALID_RECORD]
    = "PERSISTENT DATA ERROR - Invalid Record";
    
# PI_VCGShutdown Error Codes
$msgErrCode[PI_ERROR][VCG_SHUTDOWN_ERROR_WRITE_STATE_TO_QUORUM]
    = "VCG Shutdown Error - Failed to write state to quorum";

$msgErrCode[PI_ERROR][VCG_SHUTDOWN_ERROR_FE_SDIMM_SHUTDOWN]
    = "VCG Shutdown Error - Failed host SDIM shutdown";

$msgErrCode[PI_ERROR][VCG_SHUTDOWN_ERROR_BE_SDIMM_SHUTDOWN]
    = "VCG Shutdown Error - Failed storage SDIM shutdown";

# Miscellaneous Error Codes
$msgErrCode[PI_ERROR][PI_MD5_ERROR]
    = "MD5 Error";

$msgErrCode[PI_ERROR][EC_UNLABEL_ALL_OWNED]
    = "Unlabel to less than two labeled drives when in a multiple controller configuration is not allowed";

$msgErrCode[PI_PARAMETER_ERROR][0]
    = "Invalid Parameter ";

$msgErrCode[PI_MASTER_CNT_ERROR][0]
    = "You need to be a master controller to complete this operation";

$msgErrCode[PI_POWER_UP_REQ_ERROR][0]
    = "Controller has not completed the power-up sequencing";

$msgErrCode[PI_ELECTION_ERROR][0]
    = "Command not allowed while election in progress";

$msgErrCode[PI_R5_STRIPE_RESYNC_ERROR][0]
    = "Command not allowed while RAID 5 stripe resync in progress";

$msgErrCode[PI_INVALID_PACKETVERSION_ERROR][0]
    = "Packet Version Not Supported";

$msgErrCode[PI_COMPAT_INDEX_NOT_SUPPORTED][0]
    = "Compatibility Index not supported";

# ECODES 
use constant ECODES => PI_R5_STRIPE_RESYNC_ERROR + 1;

$msgErrCode[ECODES][EC_OK]              = "Successful operation";
$msgErrCode[ECODES][EC_IO_ERR]          = "I/O error";
$msgErrCode[ECODES][EC_INV_FUNC]        = "Invalid function";
$msgErrCode[ECODES][EC_NONX_DEV]        = "Nonx device";
$msgErrCode[ECODES][EC_INOP_VDEV]       = "Inoperative virtual device";
$msgErrCode[ECODES][EC_INV_SDA]         = "Invalid SDA";
$msgErrCode[ECODES][EC_INV_LEN]         = "Invalid sector count";
$msgErrCode[ECODES][EC_IN_VDA]          = "Invalid SDA + length";
$msgErrCode[ECODES][EC_NULL_SGL]        = "Null S/G list";
$msgErrCode[ECODES][EC_INV_SGC]         = "Invalid S/G descriptor count";
$msgErrCode[ECODES][EC_INV_STRAT]       = "Invalid strategy";
$msgErrCode[ECODES][EC_UNINIT_DEV]      = "Uninitialized device";
$msgErrCode[ECODES][EC_INV_VID]         = "Invalid virtual device";
$msgErrCode[ECODES][EC_INV_CLUSTER]     = "Invalid cluster";
$msgErrCode[ECODES][EC_DEV_RESERVED]    = "Device reserved";
$msgErrCode[ECODES][EC_COMPARE_ERR]     = "Compare error";
$msgErrCode[ECODES][EC_INC_SGL]         = "Inconsistent SGL";
$msgErrCode[ECODES][EC_INV_RID]         = "Invalid raid id";
$msgErrCode[ECODES][EC_CHECKSUM_ERR]    = "Bad checksum";
$msgErrCode[ECODES][EC_TIMEOUT]         = "I/O timeout";
$msgErrCode[ECODES][EC_INV_DEV]         = "Invalid SCSI device";
$msgErrCode[ECODES][EC_CHECK]           = "SCSI check";
$msgErrCode[ECODES][EC_INV_RX_ID]       = "Invalid RX_ID";
$msgErrCode[ECODES][EC_CMD_ABORT]       = "Command aborted";
$msgErrCode[ECODES][EC_LUN_RESET]       = "LUN reset during command execution";
$msgErrCode[ECODES][EC_LIP_RESET]       = "Lip reset during command execution";
$msgErrCode[ECODES][EC_DEV_RESET]       = "Target reset during cmn execution";
$msgErrCode[ECODES][EC_EVENT]           = "Entry recv while wait for event ack";
$msgErrCode[ECODES][EC_LGOFF]           = "Port logged off";
$msgErrCode[ECODES][EC_RESEL_TIMEOUT]   = "Reselection timeout";
$msgErrCode[ECODES][EC_QUEUE_FULL]      = "Queue Full";
$msgErrCode[ECODES][EC_DMA]             = "DMA / PCI error";
$msgErrCode[ECODES][EC_TRANSPORT]       = "Unspecified transport Error";
$msgErrCode[ECODES][EC_LOGOUT_SENT]     = "Log Out Sent after timeout";
$msgErrCode[ECODES][EC_BUSY]            = "SCSI busy status";
$msgErrCode[ECODES][EC_RES_CONFLICT]    = "Reservation Conflict";
$msgErrCode[ECODES][EC_UNDET_SCSI_STAT] = "Undetermined SCSI status";
$msgErrCode[ECODES][EC_OVERRUN]         = "Data overrun indicated";
$msgErrCode[ECODES][EC_UNDERRUN]        = "Data underrun indicated";
$msgErrCode[ECODES][EC_INV_LLD]         = "Invalid LLD session ID";
$msgErrCode[ECODES][EC_IOCB_ERR]        = "IOCB returned in error";
$msgErrCode[ECODES][EC_IOCB_LIP_INT]    = "IOCB returned due to LIP interlock";
$msgErrCode[ECODES][EC_RST_PORT]        = "IOCB returned due to FW reset port";
$msgErrCode[ECODES][EC_PORT_FAIL]       = "IOCB returned due to failed port";
$msgErrCode[ECODES][EC_REDIRECT]        = "Drive has been spared or redirected";
$msgErrCode[ECODES][EC_DEV_FAIL]        = "A device has failed";
$msgErrCode[ECODES][EC_LINK_FAIL]       = "PCI link to other processor error";
$msgErrCode[ECODES][EC_FCAL_IOERR]      = "I/O error indicated by FC-AL driver";
$msgErrCode[ECODES][EC_DATA_FAULT]      = "Data Path Fault error";
$msgErrCode[ECODES][EC_RETRY]           = "Unable to Complete, retry later";
$msgErrCode[ECODES][EC_COPY_COMP]       = "Sec. copy process completed";
$msgErrCode[ECODES][EC_ABORT]           = "I/O was aborted prior to SRP";
$msgErrCode[ECODES][EC_CANCEL]          = "Request canceled";


##############################################################################
# Name:  getErrorMsg
#
# Desc: Returns an error string based on the command status packet
#
# In:       Error Code
#           Command Code
#
# Returns:  string error message
#
##############################################################################
sub getErrorMsg
{
    my ($self, $status, $err_code, $cmd_code) = @_;
    # $cmd_code not used now, to be used later to map errors
    # to the command that was sent to the san box

    
    if($status == PI_SOCKET_ERROR)
    {
        return ("Socket error " . ($err_code));
    }
    if($status == PI_PARAMETER_ERROR)
    {
        return (($msgErrCode[PI_PARAMETER_ERROR][0]) . ($err_code));
    }

    if($status == PI_MASTER_CNT_ERROR)
    {
        return (($msgErrCode[PI_MASTER_CNT_ERROR][0]));
    }

    if($status == PI_POWER_UP_REQ_ERROR)
    {
        return (($msgErrCode[PI_POWER_UP_REQ_ERROR][0]));
    }

    if($status == PI_ELECTION_ERROR)
    {
        return (($msgErrCode[PI_ELECTION_ERROR][0]));
    }

    if($status == PI_R5_STRIPE_RESYNC_ERROR)
    {
        return (($msgErrCode[PI_R5_STRIPE_RESYNC_ERROR][0]));
    }

    #
    # Check for ECODES
    #
    if ($status == PI_ERROR and defined($cmd_code) and ($err_code <= 255) and
        ( $cmd_code == PI_DEBUG_SCSI_COMMAND_CMD or
          $cmd_code == PI_WRITE_BUFFER_MODE5_CMD )
    )
    {
        return $msgErrCode[ECODES][$err_code];
    }

    if(defined($msgErrCode[$status][$err_code]))
    {
        return ($msgErrCode[$status][$err_code]);
    }

    return "No Error Code Defined!";
}

##############################################################################
# Name:  getPIErrorMsg
#
# Desc: Returns a PI error string based on the command status packet
#
# In:       PI Error Code
#           Command Code
#
# Returns:  string error message
#
##############################################################################
sub getPIErrorMsg
{
    my ($self, $status, $err_code, $cmd_code) = @_;
    # $cmd_code not used now, to be used later to map errors
    # to the command that was sent to the san box

    
    if ($err_code == PI_GOOD) { return "Good"; }

    if ($err_code == PI_IN_PROGRESS)        { return "Command is in progress"; }
    if ($err_code == PI_TIMEOUT)            { return "Command has timed out"; }
    if ($err_code == PI_INVALID_CMD_CODE)   { return "Requested command code is not supported"; }
    if ($err_code == PI_MALLOC_ERROR)       { return "Memory allocation error"; }
    if ($err_code == PI_PARAMETER_ERROR)    { return "Error in input parameters"; }
    if ($err_code == PI_MASTER_CNT_ERROR)   { return "Command only available on master"; }
    if ($err_code == PI_POWER_UP_REQ_ERROR) { return "Command only available after powerup"; }
    if ($err_code == PI_ELECTION_ERROR)     { return "Command unavailable due to election"; }
    if ($err_code == PI_TUNNEL_ERROR)       { return "Tunnel error going to other controller"; }
    if ($err_code == PI_R5_STRIPE_RESYNC_ERROR){ return "Command unavailable during Raid 5 stripe resync"; }
    if ($err_code == PI_LOCAL_RAID_RESYNC_ERROR){ return "Command unavailable during local/DSC resync"; }
    if ($err_code == PI_INVALID_PACKETVERSION_ERROR){ return "Command version not supported"; }
    if ($err_code == PI_COMPAT_INDEX_NOT_SUPPORTED){ return "Code level not supported"; }

    return "No PI Error Code Defined!";
}

##############################################################################

1;

##############################################################################
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
