/* $Id: logdef.h 159129 2012-05-12 06:25:16Z marshall_midden $ */
/**
******************************************************************************
**
**  @file   logdef.h
**
**  @brief  Define data structures associated with log messages
**
**  C header file version of def.inc
**
**  Copyright (c) 2001-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _LOGDEF_H_
#define _LOGDEF_H_

#include "FW_Header.h"
#include "globalOptions.h"
#include "LOG_Defs.h"
#include "XIO_Types.h"
#include "i82559.h"
#include "ipc_packets.h"
#include "names.h"
#include "rm_val.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/
#define CDB_LENGTH      16
#define SENSE_LENGTH    32

typedef unsigned char CDB_T;
typedef unsigned char SENSE_T;

/*
** Data structures for log events.
** NOTE: All structures must be a multiple of 4 bytes in length since
** they are written to flash.
*/

/* Info event codes */
typedef struct
{
    INT32       errorCode;
    UINT8       status;
    UINT8       rsvd[3];
} LOG_STATUS_ONLY_PKT;

typedef struct
{
    UINT8       prpstat;
    UINT8       scsistat;
    UINT8       qstat;
    UINT8       rsvd1;
    UINT8       channel;
    UINT8       lun;
    UINT16      pid;
    UINT32      lid;            /* Loop ID  */
    UINT64      wwn;
    CDB_T       cdb[CDB_LENGTH];
} LOG_SHORT_SCSI_EVENT_PKT;     /*  Short SCSI log event    */

typedef struct
{
    UINT8       prpstat;
    UINT8       scsistat;
    UINT8       qstat;
    UINT8       retry;
    UINT8       channel;
    UINT8       lun;
    UINT16      pid;
    UINT32      lid;            /* Loop ID  */
    UINT64      wwn;
    CDB_T       cdb[CDB_LENGTH];
    SENSE_T     sense[SENSE_LENGTH];
} LOG_LONG_SCSI_EVENT_PKT;      /*  Long SCSI log event */

typedef struct
{
    UINT64      wwn;
    UINT8       state;
    UINT8       rsvd[1];
    UINT16      lun;
} LOG_CHANGE_LED_PKT;           /*  LED change log event    */

typedef struct
{
    UINT64      wwn;
    UINT64      wwnOldPrimary;
    UINT8       slotOldPrimary;
    UINT8       rsvd1[7];
    UINT64      wwnNewPrimary;
    UINT8       slotNewPrimary;
    UINT8       rsvd2[7];
} LOG_DEVICE_MOVED_PKT;         /*  Device moved log event  */


typedef struct
{
    UINT8       channel;
    UINT8       geoflags;
    UINT16      lun;
    UINT32      lid;            /* Loop ID  */
    UINT64      wwn;
    UINT16      pid;
    UINT8       devType;
    UINT8       rsvd2[4];
} LOG_DEVICE_REATTACED_PKT;     /*  Device reattached event */

typedef struct
{
    UINT64      wwn;
    UINT16      lun;
    UINT8       rsvd[2];
} LOG_DEVICE_NOT_FOUND_PKT;     /*  Device not found event  */

typedef struct
{
    UINT8       compstat;
    UINT8       function;
    UINT8       percent;
    UINT8       rsvd;
    UINT16      srcVid;
    UINT16      destVid;
} LOG_COPY_FAILED_PKT;          /*  Copy failed log event   */

typedef struct
{
    UINT8       port;
    UINT8       rsvd;
    UINT8       taskflags;
    UINT8       qstatus;
} LOG_HOST_OFFLINE_PKT,         /*  Host offline            */
            LOG_HOST_IMED_NOTIFY_PKT;   /*  Host immediate notify   */

typedef struct
{
    UINT8       channel;
    UINT8       errcode;
    UINT8       rsvd[2];
    UINT64      wwn;
    SENSE_T     sense[SENSE_LENGTH];
} LOG_HOST_SENSE_DATA_PKT;      /*  Host error with sense data  */

typedef struct
{
    UINT8       channel;
    UINT8       rsvd;
    UINT16      tid;            /* Target ID    */
    UINT32      owner;          /* Owner        */
    UINT64      wwn;
} LOG_ZONE_INQUIRY_PKT;         /*  Zoning Inquiry  */

typedef struct
{
    UINT8       fan;
    UINT8       rsvd[3];
} LOG_FAN_RESTORED_PKT,         /*  Fan Restored        */
            LOG_FAN_SPEED_ALERT_PKT,    /*  Fan Speed Alert     */
            LOG_FAN_FAILED_PKT; /*  Fan Failed Event    */

typedef struct
{
    UINT8       proc;
    UINT8       rsvd[3];
} LOG_PROC_TEMP_RESTORED_PKT,   /*  Processor Temp back in range    */
            LOG_PROC_TEMP_ALERT_PKT,    /*  Processor High Temp Alert       */
            LOG_PROC_LOW_TEMP_PKT,      /*  Processor Low Temp Alert        */
            LOG_PROC_TEMP_EVENT_PKT,    /*  Processor Critical Temp Event   */
            LOG_HEARTBEAT_STOP_PKT;     /*  Proc detected missing CCB HBeat */

typedef struct
{
    UINT8       supply;
    UINT8       fault;
    UINT8       rsvd[2];
} LOG_POWER_FAULT_CLEARED_PKT,  /*  Power Fault Conditions Cleared  */
            LOG_POWER_FAULT_PKT;        /*  Power Supply Fault condition    */

typedef struct
{
    UINT8       bank;
    UINT8       state;
    UINT8       status;
    UINT8       rsvd[1];
} LOG_BATTERY_STATE_PKT,        /*  Battery state change    */
            LOG_BATTERY_ALERT_PKT;      /*  SDRAM Battery Not Found */

typedef struct
{
    UINT8       status;
    UINT8       rsvd[3];
} LOG_VOLTAGE_FAULT_CLEARED_PKT,        /*  Voltage Monitor Conditions Cleared  */
            LOG_VOLTAGE_FAULT_PKT;      /*  Voltage Monitor  Event              */

typedef struct
{
    UINT16      vid;
    UINT8       rsvd[2];
} LOG_CACHE_FLUSH_RECOVER_PKT;  /* Cache Flush recovered    */

typedef struct
{
    char        version[4];
    char        sequence[4];
    char        buildId[4];
    UINT32      targetId;
    FW_HEADER_TIMESTAMP timeStamp;
    UINT32      crc;
    UINT8       seqNum;
    UINT8       totNum;
    INT8        reason;         /* failure reason */
    UINT8       rsvd;
} LOG_FIRMWARE_UPDATE_PKT,      /*  Firmware Updated                    */
            LOG_FW_CHUNK_RCVD_PKT;      /*  Firmware multi-part chunk received  */

typedef struct
{
    char        version[4];
    char        sequence[4];
    char        buildId[4];
    UINT32      targetId;
    FW_HEADER_TIMESTAMP timeStamp;
    UINT32      crc;
    UINT64      wwn;
} LOG_DRIVE_BAY_FW_UPDATE_PKT;  /*  Drive bay firmware update   */

typedef struct
{
    INT32       errorCode;
    UINT8       status;
    UINT8       rsvd[3];
    UINT16      sid;            /* Server ID created            */
    UINT16      targetId;       /* Target ID                    */
    UINT32      owner;          /* Owner                        */
    UINT64      wwn;            /* World wide name              */
} LOG_SERVER_CREATE_OP_PKT;     /* Create Server                */

typedef struct
{
    INT32       errorCode;
    UINT8       status;
    UINT8       rsvd;
    UINT16      sid;            /* Server ID                    */
    UINT16      lun;            /* LUN number                   */
    UINT16      vid;            /* VDisk ID                     */
} LOG_SERVER_ASSOC_OP_PKT,      /* Associate Server and VDisk   */
            LOG_SERVER_DISASSOC_OP_PKT; /* Disassociate Server and VDisk */

typedef struct
{
    INT32       errorCode;
    UINT8       status;
    UINT8       priority;
    UINT16      sid;
    UINT32      attr;
} LOG_SERVER_SET_PROPERTIES_OP_PKT;     /* Server Set Properties   */

typedef struct
{
    INT32       errorCode;

    UINT16      vid;            /* VDisk ID                         */
    UINT8       flags;          /* See definitions in MR_Defs.h     */
    UINT8       minPD;          /* Min PDisks per RAID create       */

    UINT8       status;
    UINT8       raidType;       /* RAID type                        */
    UINT8       mirrorDepth;    /* Mirror depth (2 or more)         */
    UINT8       parity;         /* Parity level for RAID 5          */

    UINT16      drives;         /* Number of drives in the RAID     */
    UINT16      stripe;         /* Number of sectors in the stripe  */

    UINT64      requestCapacity;        /* Requested capacity               */
    UINT64      actualCapacity; /* Actual capacity                  */
    UINT8       crossLocation;  /* Geo cross location               */

} LOG_VDISK_CREATE_OP_PKT,      /* Create Virtual Disk              */
            LOG_VDISK_EXPAND_OP_PKT,    /* Expand Virtual Disk              */
            LOG_VDISK_PREPARE_OP_PKT;   /* Prepare Virtual Disk             */

typedef struct
{
    INT32       errorCode;
    UINT8       status;
    UINT8       rsvd;
    UINT16      id;
} LOG_VDISK_DELETE_OP_PKT,      /* Delete Virtual Disk              */
            LOG_SERVER_DELETE_OP_PKT,   /* Delete Server                    */
            LOG_VLINK_BREAK_LOCK_OP_PKT,        /* VLink Break Lock                 */
            LOG_VLINK_NAME_CHANGED_OP_PKT,      /* VLink Name Changed               */
            LOG_WORKSET_CHANGED_OP_PKT, /* Workset Changed                  */
            LOG_RAID_INIT_OP_PKT,       /* RAID Init Op                     */
            LOG_VDISK_PR_CLR_PKT;       /* Vdisk PR clear                   */

/*
** The VDisk Control op handles all copy related operations
*/
typedef struct
{
    INT32       errorCode;
    UINT8       status;
    UINT8       subOpType;      /* See MR_Defs.h for definitions            */
    UINT8       mirrorSetType;
    UINT8       rsvd[1];
    UINT16      srcVid;
    UINT16      destVid;
} LOG_VDISK_CONTROL_OP_PKT;     /* VDisk copy, copy/swap, copy/mirror, etc. */

typedef struct
{
    INT32       errorCode;
    UINT8       status;
    UINT8       count;
    UINT16      vid1;           /* Virtual device ID            */
    UINT8       pri1;           /* VDisk Priority               */
    UINT16      vid2;           /* Virtual device ID            */
    UINT8       pri2;           /* VDisk Priority               */
} LOG_VDISK_SET_PRIORITY_PKT;

typedef struct
{
    INT32       errorCode;
    UINT8       status;
    UINT8       rsvd;
    UINT16      raidid;         /* RAID ID to use if raidmode=one   */
    UINT32      scrubcontrol;   /* Off, on, get state               */
    UINT32      paritycontrol;  /* Bit flags                        */

    UINT8       sstate;         /* Current state of scrubbing (on or off)   */
    UINT8       rsvd1[3];
    UINT32      pstate;         /* Current state of parity (bit mask)       */
    UINT16      scrubpid;       /* Current scrubbed PID                     */
    UINT16      scanrid;        /* Current scanned RID                      */
    UINT32      scrubblock;     /* Current scrubbed block                   */
    UINT32      scanblock;      /* Current scanned block                    */
} LOG_RAID_CONTROL_OP_PKT;

typedef struct
{
    INT32       errorCode;
    UINT8       status;
    UINT64      time;           /* Time system was set to           */
} LOG_ADMIN_SETTIME_OP_PKT;

typedef struct
{
    INT32       errorCode;
    UINT8       status;
    UINT32      serNum;         /* Serial Number affected        */
    UINT32      ipAdr;          /* New IP Address                */
    UINT32      subMaskAdr;     /* New SubnetMask Address        */
    UINT32      gatewayAdr;     /* New Gateway Address           */
} LOG_ADMIN_SET_IP_OP_PKT;

typedef struct
{
    INT32       errorCode;
    UINT8       status;
    UINT8       rsvd[3];
    UINT16      mode;
    UINT16      vid;
} LOG_VDISK_SET_ATTRIBUTE_OP_PKT;       /* Set VDisk Attribute Op   */

typedef struct
{
    INT32       errorCode;
    UINT8       status;
    UINT8       properties;
    UINT8       rsvd[3];
} LOG_SET_SYSTEM_PROP_OP_PKT;   /* Set System Properties        */

typedef struct
{
    INT32       errorCode;
    UINT8       status;
    UINT8       rsvd[3];
    UINT32      serialNumber;
} LOG_SET_SYSTEM_SERIAL_NUM_OP_PKT;     /* Set System Properties        */

typedef struct
{
    INT32       errorCode;
    UINT8       status;
    UINT8       type;           /* Type of init to do */
    UINT8       rsvd[2];
} LOG_INIT_PROC_NVRAM_OP_PKT;   /* Init Proc NVRAM    */

typedef struct
{
    INT32       errorCode;
    UINT8       status;
    UINT8       options;        /* Options                      */
    UINT16      pid;            /* Physical device ID           */
    UINT16      hspid;          /* Hotspare physical device ID  */
    UINT16      rsvd;
    UINT64      wwn;            /* World wide name              */
} LOG_PDISK_FAIL_OP_PKT;        /* Fail Physical Disk           */

typedef struct
{
    INT32       errorCode;
    UINT8       status;
    UINT8       options;        /* Options                      */
    UINT16      pid;            /* Physical device ID           */
    UINT64      wwn;            /* World wide name              */
} LOG_PDISK_FAILBACK_OP_PKT,    /* PDisk Failback Op        */
            LOG_PDISK_SPINDOWN_OP_PKT;  /* PDisk Spindown Op            */

typedef struct
{
    INT32       errorCode;
    UINT8       status;
    UINT16      bayId;          /* Bay ID                 */
    UINT8       locationId;     /* Location ID            */
    UINT8       anyHotSpares;   /* HS present             */
    UINT64      wwn;            /* World wide name        */
} LOG_SET_GEO_LOCATION_OP_PKT;  /* Set Geo Location Op    */

typedef struct
{
    INT32       errorCode;
    UINT8       status;
} LOG_CLEAR_GEO_LOCATION_OP_PKT;        /* Clear location code op */

typedef struct
{
    INT32       errorCode;
    UINT8       status;
    UINT8       locationCode;
} LOG_NO_HS_LOCATION_CODE_MATCH_PKT;

typedef struct LOG_ICL_EVENT_OP_PKT
{

    UINT8       portState;         /**< ICL port state                     */

    UINT8       rsvd2[3];          /**< Reserved                           */
} LOG_ICL_EVENT_OP_PKT;

/*
** The PDisk Label Op can label a list of drives.  Since we don't know how
** big the list will be, one log entry is made for each PDisk labeled.
** pidcnt indicates how many PDisks were labeled as part of this command.
*/
typedef struct
{
    INT32       errorCode;
    UINT8       status;
    UINT8       rsvd;
    UINT16      pid;            /* PID to be labeled                */
    UINT8       labtype;        /* Label type                       */
    UINT8       option;         /* Option                           */
    UINT64      wwn;            /* World wide name                  */
#if defined(MODEL_7000) || defined(MODEL_4700)
    UINT16  slot;               /* Slot of PID to be labeled        */
#endif /* MODEL_7000 || MODEL_4700 */
} LOG_PDISK_LABEL_OP_PKT;       /* Label a Physical Disk            */

typedef struct
{
    INT32       errorCode;
    UINT8       status;
    UINT8       type;
    UINT16      id;
    UINT64      wwn;            /* World wide name                  */
} LOG_DEVICE_DELETE_OP_PKT;     /* Delete Device                    */

typedef struct
{
    INT32       errorCode;
    UINT8       status;
    UINT8       rsvd;
    UINT16      id;
    UINT64      wwn;            /* World wide name                  */
} LOG_PDISK_DEFRAG_OP_PKT,      /* PDisk Defrag Op                  */
            LOG_PDISK_UNFAIL_OP_PKT;    /* PDisk Unfail Op                  */

#define LOG_PDISK_AUTO_FAILBACK_OP_PKT LOG_STATUS_ONLY_PKT

typedef struct
{
    INT32       errorCode;
    UINT8       status;
    UINT8       rsvd;
    UINT8       waitForFlush;   /* Wait for the cache to flush      */
    UINT8       waitForShutdown;        /* Wait for the cache to shutdown   */
} LOG_STOP_IO_OP_PKT;           /* Stop IO                          */

typedef struct
{
    INT32       errorCode;

    UINT8       status;
    UINT8       port;           /* Port number                                  */
    UINT16      tid;            /* Target ID                                    */

    UINT8       opt;            /* Options hard or soft ID, active or inact     */
    UINT8       fcid;           /* Fibre Channel ID                             */
    UINT8       lock;           /* Target lock bits                             */
    UINT8       rsvd1;          /* Reserved                                     */

    UINT64      pname;          /* Port name                                    */
    UINT64      nname;          /* Node name                                    */

    UINT32      powner;         /* Previous owning controller serial number     */
    UINT32      owner;          /* Owning controller serial number              */

    UINT16      cluster;        /* Clustered target ID                          */
    UINT16      rsvd2;          /* Reserved                                     */
    UINT32      modMask;        /* Modifier Mask - which fields to modify       */
} LOG_TARGET_SET_PROPERTIES_OP_PKT;     /* Set Target Properties        */

#if 0
typedef struct
{
    UINT8       channel;
    UINT8       rsvd1;
    UINT16      lun;
    UINT16      pid;
    UINT16      rsvd2;
    UINT32      lid;
    UINT64      wwn;

    UINT8       hschannel;
    UINT8       rsvd3;
    UINT16      hslun;
    UINT16      hspid;
    UINT16      rsvd4;
    UINT32      hsid;
    UINT64      hswwn;
} LOG_DEVICE_FAIL_HS_PKT;       /*  Device fail (w/ hot spare)          */

typedef struct
{
    UINT8       channel;
    UINT8       rsvd1;
    UINT16      lun;
    UINT16      pid;
    UINT16      rsvd2;
    UINT16      rid;
    UINT16      vid;
    UINT32      lid;
    UINT64      wwn;
} LOG_DEVICE_FAIL_NO_HS_PKT;    /*  Device fail (w/o hot spare)         */
#endif

typedef struct
{
    UINT8       channel;
    UINT8       rsvd1;
    UINT16      lun;
    UINT32      lid;            /* Loop ID  */
    UINT64      wwn;
    CDB_T       cdb[CDB_LENGTH];
    SENSE_T     sense[SENSE_LENGTH];
} LOG_SMART_EVENT_PKT;          /*  SMART enunciation log event */

typedef struct
{
    UINT8       channel;
    UINT8       geoflags;
    UINT16      lun;
    UINT32      lid;
    UINT64      wwn;
    UINT16      pid;
    UINT8       devType;
} LOG_DEVICE_TIMEOUT_PKT,       /*  Device timeout log event            */
            LOG_DEVICE_MISSING_PKT,     /*  Device missing log event            */
            LOG_DEVICE_REMOVED_PKT,     /*  Device removed log event            */
            LOG_DEVICE_INSERTED_PKT,    /*  Device inserted log event           */
            LOG_DISKBAY_REMOVED_PKT,    /*  Diskbay removed log event            */
            LOG_DISKBAY_INSERTED_PKT;   /*  Diskbay inserted log event           */

typedef struct
{
    UINT16      vid;
    UINT8       errcode;
    UINT8       rsvd[1];
} LOG_CACHE_FLUSH_FAILED_PKT;   /*  Cache Flush Failed                  */

#if 0
typedef struct
{
    UINT32      messageId;
    UINT32      messageData[63];
} LOG_FIRMWARE_ALERT_PKT;       /*  Firmware Alert                      */
#endif

typedef struct
{
    UINT8       channel;
    UINT8       rsvd1;
    UINT16      lun;
    UINT32      lid;            /* Loop ID  */
    UINT64      wwn;
    UINT32      serial;
} LOG_SERIAL_WRONG_PKT;         /*  Serial Number wrong                 */

typedef struct
{
    UINT8       channel;
    UINT8       errcode;
    UINT16      vid;
    UINT64      wwn;
    UINT16      tid;
} LOG_MAG_DRIVER_ERR_PKT,       /*  Error reported by Mag driver layer   */
            LOG_HOST_QLOGIC_ERR_PKT;    /*  Host Error from Qlogic device        */

typedef struct
{
    UINT8       channel;
    UINT8       errcode;
    UINT16      vid;
    UINT64      wwn;
    UINT16      tid;
    UINT32      count;
} LOG_HOST_NONSENSE_PKT;        /*  Host Error with no sense data        */


typedef struct
{
    UINT16      type;           /* Type of RAID error/rebuild/hotspare event */
    UINT16      res1[15];
} LOG_RAID_EVENT_PKT;

/* RAID event types */
#define erlexeccomp 0x0001      /* RAID error exec complete              */

typedef struct
{
    UINT8       errcode;
    UINT8       rsvd1[3];
} LOG_I2C_INIT_FAILED_PKT;      /*  Initialization of I2C Failed         */

typedef struct
{
    UINT8       status;
    UINT8       rsvd1[3];
} LOG_CACHE_DRAM_FAIL_PKT;      /*  Cache DRAM failed log event          */

typedef struct
{
    UINT64      lba;
    UINT32      length;
    UINT8       status;
    UINT8       count;
    UINT16      vid;
} LOG_CACHE_RECOVER_FAIL_PKT;   /*  Cache recovery fault log event       */

typedef struct
{
    UINT32      messageId;
    UINT32      messageLength;
    UINT32      messageData[36];
} LOG_ERROR_TRAP_PKT;           /*  Error Trap                           */

/* PDD Rebuild */
typedef struct
{
    UINT8       rsvd1[4];
    UINT16      lun;
    UINT16      pid;            /*  PID of the device completed          */
    UINT64      wwn;            /*    (good for this boot only)          */
} LOG_HSPARE_DONE_PKT;          /*  Hot Spare Operation                  */

typedef struct
{
    UINT8       status;
    UINT8       rsvd[3];
    UINT16      rid;
    UINT16      vid;
    UINT32      errorCount;
} LOG_RAID_INIT_DONE_PKT;       /*  Raid Initialization Complete         */

#define LOG_RAID_INIT_GOOD      0x00    /* status definitions - good */
#define LOG_RAID_INIT_TERM      0x01    /* terminated early         */
#define LOG_RAID_INIT_FAIL      0x02    /* failed with errors       */

typedef struct
{
    UINT16      port;
    UINT16      flags;          /* 0=unencrypted, 1=encrypted           */
    UINT32      IPAddress;
} LOG_SESSION_START_STOP_PKT;   /* Start or stop a config Session       */

#define LOG_SESSION_UNENCRYPTED     0x0000
#define LOG_SESSION_ENCRYPTED       0x0001


#if 0                           /* replaced in LOG_Defs.h */
typedef struct
{
    UINT8       port;           /* FC channel number            */
    UINT8       rsvd1;
    UINT8       proc;           /* FE or BE indicator  0=FE 1=BE */
    UINT8       failed;         /* Failed port indicator        */
    UINT16      lid;
    UINT16      state;
} LOG_LOOPUP_PKT,               /* Loop Up                               */
            LOG_PORT_DB_DHANGED_PKT;    /* Port data base changed                */
#endif

typedef struct
{
    UINT32      time;           /* Time in 128 msec increments           */
    UINT32      caller;         /* Caller(IP) of exchange                */
    UINT32      prev;           /* Previous caller of exchange           */
    UINT32      pcb;            /* PCB of task taking too long           */
} LOG_TASK_TOO_LONG_PKT;        /* Too long between exchanges            */

typedef struct
{
    UINT32      miscompares;    /* Number of miscompares                 */
    UINT32      pass;           /* Pass number                           */
    UINT32      startraid;      /* Starting RAID number                  */
    UINT32      prevctrl;       /* Control used to start test            */
    UINT32      invfunc;        /* Invalid function errors returned      */
    UINT32      reserved;       /* Reserved errors returned              */
    UINT32      other;          /* Other errors returned                 */
} LOG_PARITY_CHK_DONE_PKT;      /* Parity scan check pass complete       */

#if 0                           /* replaced in LOG_Defs.h */
typedef struct
{
    UINT8       cacheId;
    UINT8       rsvd[3];
    UINT32      sysSeq;
    UINT32      seq;
} LOG_WC_SEQNO_BAD_PKT;         /* Write cache sequence number bad       */

typedef struct
{
    UINT16      vid;
    UINT8       rsvd[2];
    UINT32      count;
} LOG_VID_RECOVERY_FAIL_PKT;    /* Cache recovery for VID failed         */
#endif

typedef struct
{
    UINT16      status1;
    UINT16      status2;
    UINT32      tagData[16];
} LOG_INVALID_TAG_PKT;          /* Invalid tag during Cache recovery     */

typedef struct
{
    UINT8       api;            /* function identifier           */
    UINT8       status;         /* actual mrp status - if avail. */
    UINT16      rsvd;
    INT32       rc;             /* function return code          */
    UINT32      fid;            /* file id being read/written    */
    UINT16      wr_good;        /* # drives written good         */
    UINT16      wr_err;         /* # drives w/error on write     */
} LOG_FILEIO_ERR_PKT;           /*   Internal File I/O Error     */

typedef struct
{
    INT32       errorCode;
    UINT8       status;
    UINT8       rsvd;
    UINT8       port;           /* FC channel number    */
    UINT8       option;         /* Reset Option         */

} LOG_FE_QLOGIC_RESET_OP_PKT,   /* ISP Chip Reset       */
            LOG_BE_QLOGIC_RESET_OP_PKT;

#define logporteventfe 0
#define logporteventbe 1
#define logportinitfe 0
#define logportinitbe 1

typedef struct
{
    UINT8       port;           /* FC channel number            */
    UINT8       rsvd1;
    UINT8       proc;           /* FE or BE indicator  0=FE 1=BE */
    UINT8       rsvd2;
    UINT32      reason;         /* Reason Code                  */
    UINT32      count;          /* Count                        */
} LOG_CHIP_RESET_PKT,           /*   ISP Chip Reset             */
            LOG_PORT_UP_PKT, LOG_PORT_INIT_PKT;


typedef struct
{
    UINT8       port;           /* FC channel number            */
    UINT8       rsvd1;
    UINT8       proc;           /* FE or BE indicator  0=FE 1=BE */
    UINT8       rsvd2;
} LOG_LOOP_DOWN_PKT;            /* Port is down */

/* Reason codes */
#define ecrlsto         0x0101  /* Link status timeout          */
#define ecrcmdto        0x0102  /* Command timeout              */
#define ecrfatal        0x0103  /* Fatal Error                  */
#define ecrasyqof       0x104   /* Async queue overlow          */
#define ecrbadiocb      0x105   /* Bad IOCB type                */
#define ecrnoiltiocb    0x106   /* No ILT in IOCB               */
#define ecrnoiltcr      0x107   /* No ILT completion routine    */
#define ecrthread       0x108   /* ILT unthread error           */
#define ecrbaddev       0x201   /* Bad device                   */
#define ecrdead         0x202   /* No-op command failed         */
#define ecrmbxfail      0x203   /* mailbox test failed          */
#define ecrnofw         0x204   /* No firmware found            */
#define ecrloadram      0x205   /* Load Ram command failed      */
#define ecrchecksum     0x206   /* Verify firmware checksum failed */
#define ecrabout        0x207   /* About firmware command failed */
#define ecrinitcmd      0x208   /* Init mailboc command failed  */
#define eclpnoloop      0x301   /* No Loop after power on       */
#define eclpdnretry     0x302   /* Loop down retry              */
#define eclpdnfail      0x303   /* Loop down retry failed       */
#define eclpup          0x400   /* Loop Up                      */

typedef struct
{
    UINT8       channel;        /* FC channel number            */
    UINT8       rsvd1;
    UINT8       proc;           /* FE or BE indicator           */
    UINT8       rsvd2;
    UINT16      vendor;         /* Vendor ID                    */
    UINT16      device;         /* Device ID                    */

} LOG_FOREIGN_PCI_PKT;          /* Foreign PCI device           */

typedef struct
{
    UINT8       channel;        /* FC channel number            */
    UINT8       rsvd[3];
    UINT32      lid;            /* Loop ID                      */
    UINT64      wwn;
} LOG_BE_INITIATOR_PKT;         /*  Back End Initiator Detected */

/* Reason codes */
#define TOO_MANY_ATTACHMENTS 0
#define MESSAGE_TOO_LARGE    1
#define NO_FIRMWARE_HEADER   2
#define UNKNOWN_REASON       3
#define NO_BAYS_FOUND        4

typedef struct
{
    UINT8       reason;
    UINT8       rsvd[3];
} LOG_MSG_DELETED_PKT;          /*  Email msg deleted without processing */

/* this struct used in LOG_FW_VERSIONS below */
typedef struct
{
    UINT64      wwn;
    char        version[4];
} BAY_FW_VER;

typedef struct
{
    char        beBoot[4];
    char        beDiag[4];
    char        beRuntime[4];
    char        feBoot[4];
    char        feDiag[4];
    char        feRuntime[4];
    char        ccbBootDiag[4];
    char        ccbRuntime[4];
    UINT32      bayCount;
    BAY_FW_VER  bay[0];
} LOG_FW_VERSIONS_PKT;

typedef struct
{
    INT32       errorCode;
    UINT8       status;
    UINT8       vdo;            /* Virtual Disk Ordinal                 */
    UINT16      cid;            /* Controller Index number (ordinal)    */

    UINT16      vid;            /* Virtual Link ID                      */
    UINT8       rsvd[2];        /* RESERVED                             */
    UINT8       ctrlName[NAMES_CONTROLLER_LEN_MAX];     /* Remote Controller Name   */
    UINT8       vdName[NAMES_VDEVICE_LEN_MAX];  /* Remote Virtual Disk Name */
} LOG_VLINK_CREATE_OP_PKT;      /* Create Virtual Link                  */

typedef struct
{
    UINT32      sourceCtrlSN;   /* Source Controller Serial Number  */
    UINT8       sourceCtrlCluster;      /* Source Controller Cluster Number */
    UINT8       rsvd1;          /* RESERVED                         */
    UINT16      sourceCtrlVDisk;        /* Source Controller VDisk          */
    UINT16      destinationCtrlVDisk;   /* Destination Controller VDisk     */
    UINT16      rsvd2;          /* RESERVED                         */
} LOG_VLINK_SIZE_CHANGED_PKT;   /* VLink Size Changed               */

typedef struct
{
    UINT8       fromState;
    UINT8       toState;
    UINT8       rsvd[2];
} LOG_ELECTION_STATE_TRANSITION_PKT;    /* Election state machine - state transition */

typedef struct
{
    UINT8       status;
    UINT8       bank;
    UINT8       rsvd[2];
} LOG_I2C_BATTERY_CONTROL_PKT;

typedef struct
{
    UINT8       beReady;
    UINT8       feReady;
    UINT8       rsvd[2];
} LOG_PROC_NOT_READY_PKT;       /*  Proc did not come ready in time allotted */

typedef struct
{
    INT32       errorCode;
    UINT8       status;
    UINT8       rsvd1[3];       /* RESERVED                         */
    UINT16      option;         /* Option                           */
    UINT16      id;             /* Device identifier                */
    UINT8       rsvd2[12];      /* RESERVED                         */
    UINT8       name[NAME_DEVICE_NAME_LEN];     /* Name for the device              */
} LOG_PROC_NAME_DEVICE_OP_PKT;  /*  Proc Name Device */

typedef struct
{
    UINT32      errorCode;
} LOG_SOCKET_ERROR_PKT;

typedef struct
{
    UINT32      controllerSN;
    UINT8       cluster;
    UINT8       opath;
    UINT8       tpath;
    UINT8       iclPathFlag;
} LOG_LOST_ALL_PATHS_PKT, LOG_NEW_PATH_PKT;

typedef struct
{
    UINT32      controllerSN;
    UINT8       cluster;
    UINT8       opath;
    UINT8       tpath;
    UINT8       iclPathFlag;
    UINT8       dgStatus;
    UINT8       dgErrorCode1;
    UINT8       dgErrorCode2;
    UINT8       rsvd2;
} LOG_LOST_PATH_PKT;

#if 0                           /* replaced in LOG_Defs.h */
typedef struct
{
    UINT32      controllerSN;
    UINT32      status;
    UINT8       dgRequestStatus;
    UINT8       dgStatus;
    UINT8       dgErrorCode1;
    UINT8       dgErrorCode2;
} LOG_CACHE_MIRROR_FAILED_PKT;
#endif

typedef struct
#define  ENA_FE         0x0001  /* FE                       */
#define  ENA_BE         0x0002  /* BE                       */
#define  ENA_PART3      0x0300  /* Part 3 NVA records       */
#define  ENA_PART4      0x0400  /* Part 4 NVA records       */
{
    UINT8       processor;
    UINT8       part;
    UINT16      rsvd;
} LOG_NVA_BAD_PKT;

typedef struct
{
    UINT8       status;
    UINT8       rsvd[3];
    UINT16      vid;
    UINT16      rid;
    UINT64      vsda;
    UINT64      rsda;
    UINT32      vlength;
    UINT32      rlength;
} LOG_NVA_RESYNC_FAILED_PKT;

typedef struct
{
    UINT64      wwn;            /* WWN of the device with only one path */
} LOG_SINGLE_PATH_PKT;          /* Only a single path to device detected */

typedef struct
{
    UINT16      pid;            /* PID                                      */
    UINT16      rsvd;           /* Reserved                                 */
    UINT64      wwn;            /* WWN                                      */
} LOG_HOTSPARE_INOP_PKT;        /* Hotspare is inoperative                  */

/* depletion type BIT defintions                                    */
#define     HSD_CNC         0       /**< CNC is depleted                    */
#define     HSD_GEO         1       /**< GEO-RAID is depleted               */
#define     HSD_TOO_SMALL   2       /**< Not big enough for biggest drive   */
#define     HSD_CNC_OK      3       /**< GEO_RAID is OK now                 */
#define     HSD_GEO_OK      4       /**< GEO_RAID is OK now                 */
#define     HSD_JUST_RIGHT  5       /**< Now big enough for biggest drive   */

typedef struct
{
    UINT16      type;               /**< Hot spare depleted type            */
    UINT16      dev;                /**< Pool or bay ID depleted            */
    UINT8       devType;            /**< Type of drive (SATA, FC, SSD)      */
    UINT8       rsvd13[3];          /**< Reserved                           */
} LOG_HOTSPARE_DEPLETED_PKT;    /* Hotspares are depleted                   */

typedef struct
{
    UINT32  status;
    UINT64  wwnTo;          /* Device on which the file system was updated     */
    UINT64  wwnFrom;        /* Device from which the file system was copied    */
    UINT16  pid;            /* Device PID on which the file system was updated */
    UINT16  rsvd;           /* Reserved for future use if any                  */
} LOG_FS_UPDATE_FAIL_PKT, LOG_FS_UPDATE_PKT;

#if 0
typedef struct
{
    UINT16      lun;
    UINT16      vid;
    UINT16      rid;
    UINT16      pid;            /* PID of this device (good for this boot only)  */
    UINT64      wwnPdisk;       /* Device with error                             */
} LOG_RAID_ERROR_PKT;
#endif

#if 0
typedef struct
{
    UINT16      rid;
    UINT8       success;
    UINT8       rsvd;
    UINT64      wwnPdisk;       /* Device defragged                              */
} LOG_DEFRAG_DONE_PKT;
#endif

typedef struct
{
    ETHERNET_LINK_STATUS linkStatus;
} LOG_ETHERNET_LINK_STATUS_CHANGE_PKT;

typedef struct
{
    UINT32      feserial;
    UINT32      beserial;
} LOG_FE_BE_SERIAL_MISMATCH_PKT;

typedef struct
{
    UINT8       proc;
    UINT8       rsvd1;
    UINT8       type;
    UINT8       rsvd2;
    UINT32      errorAddress;
    UINT32      elogRegister;
    UINT32      singleBitCount;
    UINT32      multiBitCount;
    UINT32      totalMultiCount;
    UINT32      totalCount;
} LOG_SINGLE_BIT_ECC_PKT;

typedef struct
{
    UINT32      validnva;
    UINT32      attempts;
    UINT32      errors;
} LOG_RESYNC_DONE_PKT;

typedef struct
{
    UINT32      DetectedBySN;   /* Controller that found link failure */
    UINT32      DestinationSN;  /* Destination of the failed link    */
    UINT8       LinkType;       /* Type of the link that has failed  */
    char        Reserved[3];
    UINT32      LinkError;      /* Error found on this link          */
} LOG_IPC_LINK_UP_PKT, LOG_IPC_LINK_DOWN_PKT;

typedef struct
{
    UINT32      imageSize;          /**< Length of the local image          */
    void       *pImage;             /**< PCI Address of the image           */
} LOG_LOCAL_IMAGE_READY_PKT;

typedef struct
{
    UINT16      subEvent;           /**< Subevent type                      */
    UINT16      bcastType;          /**< Broadcast Type                     */
    UINT32      serialNum;          /**< Serial number to send to           */
    UINT32      dataSize;           /**< Size of data to follow             */
    UINT8       data[0];            /**< Data Packet                        */
} LOG_IPC_BROADCAST_PKT;

typedef struct
{
    INT32       errorCode;
    UINT8       status;
    UINT8       rsvd[3];
    UINT32      vcgID;          /* Virtual Controller Group ID  */
    UINT32      controllerSN;   /* controller serial number     */
    UINT32      ipAddress;      /* Ethernet IP address          */
    UINT8       communicationsKey[16];  /* Communications signature Key */
} LOG_VCG_PREPARE_SLAVE_OP_PKT;

typedef struct
{
    INT32       errorCode;
    UINT8       status;
    UINT8       rsvd1[3];
    UINT8       rsvd2[4];
    UINT32      ipAddress;
} LOG_VCG_ADD_SLAVE_OP_PKT;

typedef struct
{
    INT32       errorCode;
    UINT8       status;
    UINT8       rsvd;
    UINT16      vcgMaxNumControllers;   /* Max controllers availble to VCG  */
    UINT32      vcgID;          /* Virtual Controller Group ID      */
} LOG_VCG_APPLY_LICENSE_OP_PKT;

typedef struct
{
    INT32       errorCode;
    UINT8       status;
    UINT8       rsvd[2];
    UINT8       mode;
} LOG_VCG_SET_CACHE_OP_PKT;     /* Set VCG Cache Op   */

typedef struct
{
    INT32       errorCode;
    UINT8       status;
    UINT8       rsvd[3];
    UINT32      serialNumber;
} LOG_VCG_FAIL_CONTROLLER_OP_PKT,
    LOG_VCG_UNFAIL_CONTROLLER_OP_PKT,
    LOG_VCG_REMOVE_CONTROLLER_OP_PKT,
    LOG_VCG_INACTIVATE_CONTROLLER_OP_PKT,
    LOG_VCG_ACTIVATE_CONTROLLER_OP_PKT;

typedef struct
{
    INT32       errorCode;
    UINT8       status;
    UINT8       rsvd[3];
    UINT32      newSerialNumber;
    UINT32      oldSerialNumber;
} LOG_PROC_ASSIGN_MIRROR_PARTNER_OP_PKT;

typedef struct
{
    UINT16      rid;            /* RAID ID  */
    UINT16      Reserved;
} LOG_PARITY_SCAN_REQUIRED_PKT;

#if 0                           /* replaced in LOG_Defs.h */
typedef struct
{
    UINT8       port;           /* Port Number                          */
    UINT8       rsvd1;
    UINT8       proc;           /* FE (0) or BE (1) indicator           */
    UINT8       rsvd2;
    UINT16      iregs;
    UINT16      oregs;
    UINT16      imbr[12];       /* Input mailbox registers              */
    UINT16      ombr[12];       /* Output mailbox registers             */
} LOG_MBX_FAILED_PKT;           /* Mailbox Command Failed */
#endif

typedef struct
{
    UINT8       port;
    UINT8       rsvd1;
    UINT8       proc;
    UINT8       rsvd2;
    UINT8       iocb;
    UINT8       rsvd3;
    UINT32      ilt;
    UINT16      lun;
    UINT32      lid;            /* Loop ID  */
    UINT64      wwn;
    CDB_T       cdb[CDB_LENGTH];
} LOG_IOCB_TO_PKT;              /* IOCB Timeout */

typedef struct
{
    UINT16      vlid;
    UINT16      type;
    UINT8       ctrlName[NAMES_CONTROLLER_LEN_MAX];
    UINT8       vdName[NAMES_VDEVICE_LEN_MAX];
} LOG_VLINK_CHANGE_NAME_PKT;

typedef struct
{
    UINT16      type;
    UINT16      id;
} LOG_VLINK_GET_NAME_PKT;

typedef struct
{
    UINT8       proc;
    UINT8       rsvd[3];
} LOG_MEMORY_HEALTH_PKT;

typedef struct LOG_FCM_STRUCT
{
    UINT8       proc;
    UINT8       port;
    UINT8       eventType;
    UINT8       conditionCode;
} LOG_FCM_PKT;

/* Event generated by Failure manager to log failure management actions */

/* MLE_LOG_FAILURE_EVENT, MLE_LOG_FAILURE_EVENT_INFO, MLE_LOG_FAILURE_EVENT_WARN */
typedef struct _LOG_FAILURE_EVENT
{
    UINT8       action_location;
    UINT8       action;
    UINT16      reserved;
    IPC_REPORT_CONTROLLER_FAILURE event;
} LOG_FAILURE_EVENT_PKT;

#define SIZE_OF_FAILURE_MLE_LOG_EVENT        (sizeof(UINT32))

#define FAILURE_EVENT_ACTION_LOCATION_NONE   0  /* Placeholder */
#define FAILURE_EVENT_ACTION_LOCATION_SLAVE  1  /* Message from slave */
#define FAILURE_EVENT_ACTION_LOCATION_MASTER 2  /* Message from master */

#define FAILURE_EVENT_ACTION_NONE                0      /* No action */
#define FAILURE_EVENT_ACTION_FAILED_INTERFACE    1      /* Failed the interface */
#define FAILURE_EVENT_ACTION_FAILED_CONTROLLER   2      /* Failed the controller */
#define FAILURE_EVENT_ACTION_RECOVERED_INTERFACE 3      /* Told RM that Interface is OK now */
#define FAILURE_EVENT_ACTION_ELECTION            4      /* Started an election */
#define FAILURE_EVENT_ACTION_FORWARD_TO_MASTER   5      /* Forwarding to master */
#define FAILURE_EVENT_ACTION_DONT_HANDLE         6      /* Something we don't handle happened */
#define FAILURE_EVENT_ACTION_RESUBMIT            7      /* Resubmitting queued failure events */
#define FAILURE_EVENT_ACTION_DISCARDED           8      /* FM is turned off, so we threw out a packet */
#define FAILURE_EVENT_ACTION_LOG_ONLY            9      /* Simply log the Failure */

/*  Resource manager event structure */
typedef struct _LOG_RM_EVENT
{
    UINT8       rm_logging_code;        /* Type of log event */
    UINT32      rm_log_param_1; /* First passed parameter - event dependent */
    UINT32      rm_log_param_2; /* Second passed parameter - event dependent */
    UINT32      rm_log_param_3; /* Third passed parameter - event dependent */
} LOG_RM_EVENT_PKT;

/*
** The Text Message log event structure is identical to the packet request
** structure.  Refer to PacketInterface.h
*/
typedef struct _LOG_TEXT_MESSAGE_OP
{
    UINT8       text[124];      /* Text message */
} LOG_TEXT_MESSAGE_OP_PKT;

typedef struct
{
    UINT32      number;         /* Archived snapshot #.  Can be matched up  */
    /*   later if not already recycled.         */
    UINT8       index;          /* Storage index number                     */
    TIMESTAMP   time;           /* The time snapshot was taken              */
    UINT32      avlFlags;       /* which fids available                     */
    UINT32      rstFlags;       /* which fids actually restored             */
    INT32       status;         /* error status (unused for now)            */
    INT8        description[1]; /* Snapshot desc. Min 1 byte for nul term.  */
} LOG_SNAPSHOT_TAKEN_PKT, LOG_SNAPSHOT_RESTORED_PKT, LOG_SNAPSHOT_RESTORE_FAILED_PKT;

typedef struct
{
    UINT32      errorCode;      /* Low nibble is error code, upper is NMI data (?) */
    UINT32      processor;      /* 2xxxxxxx == BE, 6xxxxxxx == FE */
    INT8        version[4];     /* fw version */
    INT8        sequence[4];    /* fw bld seq */
    UINT32      rRegs[16];
    UINT32      gRegs[16];
} LOG_PROC_ERR_TRAP_PKT, LOG_POWER_OFF_PKT;

/*
** Boot code log entry header
*/
typedef struct logBootEntryStruct
{
    UINT8       data[40];
} LOG_BOOT_ENTRY_PKT;

/*
** Back End or Front End Loop Primitive
*/
typedef struct _LOG_LOOP_PRIMITIVE_OP
{
    INT32       errorCode;
    UINT8       status;
    UINT8       proc;           /* FE or BE indicator  0=FE 1=BE */
    UINT16      option;
    UINT16      id;
    UINT8       rsvd;
    UINT8       port;
    UINT32      lid;
} LOG_LOOP_PRIMITIVE_OP_PKT;

typedef struct _LOG_VALIDATION_MSG
{
    UINT32      code;
    UINT8       text[VAL_MSG_LENGTH];   /* Text message */
} LOG_VALIDATION_MSG_PKT;

typedef struct _LOG_XSSA_LOG_MESSAGE
{
    UINT8       type;
    UINT8       eType;
    UINT8       flags;
    UINT8       status;
    UINT8       message[MMC_MESSAGE_SIZE];
} LOG_XSSA_LOG_MESSAGE_PKT;

typedef struct _LOG_DIAG_RESULT
{
    UINT8       source;         /* diagnostic source (processor)    */
    UINT8       result;         /* Pass or Fail                     */
    union
    {
        UINT8       bytes[38];  /* Binary Result log */
        struct
        {
            UINT8       test;
            UINT8       phase;
            UINT32      address;
            UINT8       bytes[32];
        } record;
    } data;
} LOG_DIAG_RESULT_PKT;

typedef struct _LOG_CSTOP_LOG
{
    UINT32      time;           /* Time in seconds                        */
    UINT32      function;       /* Address of recovery function           */
    UINT32      cacheReqCount;  /* Cache Outstanding Request count        */
    UINT32      cacheWriteReqCount;     /* Cache Outstanding Writes to host       */
    UINT32      cacheReadReqCount;      /* Cache Outstanding Reads to host        */
    UINT32      vLinkReqCount;  /* Vlink Outstanding operations at Rec time */
    UINT32      vrpReqCount;    /* Outstanding VRPs to backend            */
} LOG_CSTOP_LOG_PKT;

typedef struct
{
    UINT32      controllerSN;   /* Controller initiating failure  */
    UINT32      failedControllerSN;     /* Controller being failed        */
} LOG_CONTROLLER_FAIL_R1;

typedef struct
{
    LOG_CONTROLLER_FAIL_R1 r1;  /* Packet data in R1 release      */
    char        reasonString[MMC_MESSAGE_SIZE]; /* Additional data in R2 release  */
} LOG_CONTROLLER_FAIL_R2;

typedef struct _LOG_CONTROLLER_FAIL
{
    LOG_CONTROLLER_FAIL_R2 r2;  /* Packet data in R2 release      */
    UINT8       contactType;    /* Additional data in R3 release  */
} LOG_CONTROLLER_FAIL_PKT;

typedef struct _LOG_VLINK_OPERATION
{
    UINT32      vlSourceSN;     /* Source MAG serial #                    */
    UINT8       vlSourceCL;     /* Source cluster #                       */
    UINT8       rsvd5;          /* Reserved (byte 5)                      */
    UINT16      vlSourceVL;     /* Source VLink #                         */
    UINT16      vlDestVD;       /* Destination VDisk #                    */
    UINT16      rsvd10;         /* Reserved (bytes 10-11)                 */
} LOG_VLINK_OPERATION_PKT;

typedef struct _LOG_DISASTER
{
    INT8        reasonString[MMC_MESSAGE_SIZE]; /* Reason for disaster condition     */
} LOG_DISASTER_PKT;

#define LOG_SCSI_TIMEOUT_FLAGS_TIMEOUT      0
#define LOG_SCSI_TIMEOUT_FLAGS_COMP         1

typedef struct
{
    UINT8       channel;
    UINT8       flags;              /**< Flags indicating if this is the      */
                                    /**< watchdog or if this is the completion*/
                                    /**< function.                            */
    UINT16      lun;
    UINT32      lid;            /* Loop ID  */
    UINT64      wwn;
    CDB_T       cdb[CDB_LENGTH];
} LOG_SCSI_TIMEOUT_PKT;         /*  SCSI Timeout log event    */

/* Wookiee IPMI events */
#define LOG_IPMI_EVENT_TYPE_INTERFACE       0
#define LOG_IPMI_EVENT_TYPE_SEL_ENTRY       1
typedef struct
{
    UINT8       eventType;
    char        text[80];       /* MMC message size might not be enough   */
} LOG_IPMI_EVENT_PKT;

#if ISCSI_CODE
typedef struct LOG_SERVER_LOGGED_IN_PKT
{
    UINT16      tid;
    UINT16      loginState;
    UINT64      wwn;
    UINT8       i_name[256];
} LOG_SERVER_LOGGED_IN_PKT;

typedef struct LOG_TARGET_UP_PKT
{
    UINT16      tid;
    UINT8       type;
    UINT8       state;
    union
    {
        struct
        {
            UINT32      ip;
            UINT32      mask;
            UINT32      gateway;
        }
#ifdef S_SPLINT_S
        aha3
#endif                          /* S_SPLINT_S */
                   ;
        struct
        {
            UINT64      wwn;
            UINT32      rsvd;
        }
#ifdef S_SPLINT_S
        aha4
#endif                          /* S_SPLINT_S */
                   ;
    };
} LOG_TARGET_UP_PKT;

typedef struct LOG_ISCSI_SET_INFO_PKT
{
    INT32       errorCode;
    UINT8       status;
    UINT16      tid;
    UINT32      setmap;
    UINT8       rsvd;
} LOG_ISCSI_SET_INFO_PKT;

typedef struct LOG_ISCSI_SET_CHAP_PKT
{
    INT32       errorCode;
    INT32       status;
    UINT32      count;
    UINT8       option;
    UINT8       rsvd[3];
} LOG_ISCSI_SET_CHAP_PKT;

typedef struct LOG_ISCSI_GENERIC_OP_PKT
{
    UINT8       msgStr[4096];
} LOG_ISCSI_GENERIC_OP_PKT;
#endif

typedef struct LOG_GENERIC_OP_PKT
{
    UINT8       msgStr[1024];
} LOG_GENERIC_OP_PKT;

typedef struct LOG_AUTO_FAILBACK_INFO_OP_PKT
{
    UINT8       status;
    UINT64      hswwn;
    UINT64      dstwwn;
} LOG_AUTO_FAILBACK_INFO_OP_PKT;

typedef struct LOG_AF_ENABLE_DISABLE_INFO_OP_PKT
{
    UINT8       status;
} LOG_AF_ENABLE_DISABLE_INFO_OP_PKT;


typedef struct LOG_REINIT_DRIVE_INFO_OP_PKT
{
    UINT64      dstwwn;
} LOG_REINIT_DRIVE_INFO_OP_PKT;

typedef struct
{
    UINT64      wwn;            /* WWN of the delaying drive */
} LOG_DRIVE_DELAY_PKT;          /* Drive is delaying all operations */

typedef struct LOG_GLOBAL_CACHE_MODE_PKT
{
    UINT8       status;
} LOG_GLOBAL_CACHE_MODE_PKT;

typedef union LOG_DATA_t
{
    LOG_SHORT_SCSI_EVENT_PKT shortSCSIEvent;
    LOG_LONG_SCSI_EVENT_PKT longSCSIEvent;
    LOG_CHANGE_LED_PKT changeLed;
    LOG_DEVICE_MOVED_PKT deviceMoved;
    LOG_DEVICE_REMOVED_PKT deviceRemoved;
    LOG_DEVICE_INSERTED_PKT deviceInserted;
    LOG_DISKBAY_REMOVED_PKT diskbayRemoved;
    LOG_DISKBAY_INSERTED_PKT diskbayInserted;
    LOG_DEVICE_REATTACED_PKT deviceReattached;
    LOG_DEVICE_NOT_FOUND_PKT deviceNotFound;
    LOG_COPY_COMPLETE_DAT copyComplete;
    LOG_COPY_FAILED_PKT copyFailed;
    LOG_HOST_OFFLINE_PKT hostOffline;
    LOG_HOST_IMED_NOTIFY_PKT hostImedNotify;
    LOG_HOST_SENSE_DATA_PKT hostSenseData;
    LOG_ZONE_INQUIRY_PKT zoneInquiry;
    LOG_FAN_RESTORED_PKT fanRestored;
    LOG_FAN_SPEED_ALERT_PKT fanSpeedAlert;
    LOG_FAN_FAILED_PKT fanFailed;
    LOG_PROC_TEMP_RESTORED_PKT procTempRestored;
    LOG_PROC_TEMP_ALERT_PKT procTempAlert;
    LOG_PROC_LOW_TEMP_PKT procLowTemp;
    LOG_PROC_TEMP_EVENT_PKT procTempEvent;
    LOG_PROC_NAME_DEVICE_OP_PKT procNameDevice;
    LOG_POWER_FAULT_CLEARED_PKT powerFaultCleared;
    LOG_POWER_FAULT_PKT powerFault;
    LOG_BATTERY_STATE_PKT batteryState;
    LOG_BATTERY_ALERT_PKT batteryAlert;
    LOG_VOLTAGE_FAULT_CLEARED_PKT voltageFaultCleared;
    LOG_VOLTAGE_FAULT_PKT voltageFault;
    LOG_CACHE_FLUSH_RECOVER_PKT cacheFlushRecover;
    LOG_FIRMWARE_UPDATE_PKT firmwareUpdate;
    LOG_LOOPUP_DAT loopUp;
    LOG_PORTDBCHANGED_DAT portDbChanged;
    LOG_RSCN_DAT rscn;
    LOG_TASK_TOO_LONG_PKT taskTooLong;
    LOG_PARITY_CHK_DONE_PKT parityCheckDone;
    LOG_CHIP_RESET_PKT chipReset;
    LOG_PORT_EVENT_DAT portEvent;
    LOG_PHY_ACTION_DAT phyAction;
    LOG_PORT_UP_PKT portUp;
    LOG_PORT_INIT_PKT portInit;
    LOG_LOOP_DOWN_PKT loopDown;
    LOG_FOREIGN_PCI_PKT foreignPci;
    LOG_BE_INITIATOR_PKT beinitiator;
    LOG_FW_CHUNK_RCVD_PKT fwChunkRcvd;
    LOG_DRIVE_BAY_FW_UPDATE_PKT driveBayFWUpdate;
    LOG_SERVER_CREATE_OP_PKT serverCreateOp;
    LOG_SERVER_DELETE_OP_PKT serverDeleteOp;
    LOG_SERVER_ASSOC_OP_PKT serverAssociateOp;
    LOG_SERVER_DISASSOC_OP_PKT serverDisassociateOp;
    LOG_SERVER_SET_PROPERTIES_OP_PKT serverSetPropOp;
    LOG_VDISK_PREPARE_OP_PKT vdiskPrepareOp;
    LOG_VDISK_CREATE_OP_PKT vdiskCreateOp;
    LOG_VDISK_EXPAND_OP_PKT vdiskExpandOp;
    LOG_VDISK_DELETE_OP_PKT vdiskDeleteOp;
    LOG_VDISK_CONTROL_OP_PKT vdiskControlOp;
    LOG_VCG_SET_CACHE_OP_PKT vcgSetCacheOp;
    LOG_VDISK_SET_ATTRIBUTE_OP_PKT vdiskSetAttributeOp;
    LOG_RAID_INIT_OP_PKT raidInitOp;
    LOG_RAID_CONTROL_OP_PKT raidControlOp;
    LOG_DEVICE_DELETE_OP_PKT deviceDeleteOp;
    LOG_VLINK_BREAK_LOCK_OP_PKT vlinkBreakLockOp;
    LOG_VLINK_NAME_CHANGED_OP_PKT vlinkNameChangedOp;
    LOG_WORKSET_CHANGED_OP_PKT worksetChangedOp;
    LOG_PROC_ASSIGN_MIRROR_PARTNER_OP_PKT procAssignMirrorOp;
    LOG_PDISK_LABEL_OP_PKT pdiskLabelOp;
    LOG_VLINK_SIZE_CHANGED_PKT vlinkSizeChanged;
    LOG_STOP_IO_OP_PKT ioStopOp;
    LOG_INIT_PROC_NVRAM_OP_PKT initProcNVRAMOp;
    LOG_SET_SYSTEM_SERIAL_NUM_OP_PKT setSystemSerialNumberOp;
    LOG_VCG_PREPARE_SLAVE_OP_PKT vcgPrepareSlaveOp;
    LOG_VCG_ADD_SLAVE_OP_PKT vcgAddSlaveOp;
    LOG_VCG_APPLY_LICENSE_OP_PKT vcgApplyLicense;
    LOG_VCG_FAIL_CONTROLLER_OP_PKT vcgFailControllerOp;
    LOG_VCG_UNFAIL_CONTROLLER_OP_PKT vcgUnfailControllerOp;
    LOG_VCG_REMOVE_CONTROLLER_OP_PKT vcgRemoveControllerOp;
    LOG_VCG_INACTIVATE_CONTROLLER_OP_PKT vcgInactivateControllerOp;
    LOG_VCG_ACTIVATE_CONTROLLER_OP_PKT vcgActivateControllerOp;
    LOG_FE_QLOGIC_RESET_OP_PKT feQlogicResetOp;
    LOG_BE_QLOGIC_RESET_OP_PKT beQlogicResetOp;
    LOG_TARGET_SET_PROPERTIES_OP_PKT targetSetPropOp;
    LOG_SET_SYSTEM_PROP_OP_PKT setSystemPropOp;
    LOG_PDISK_DEFRAG_OP_PKT pdiskDefragOp;
    LOG_PSD_REBUILD_INFO_DAT psdRebuildStart;
    LOG_PSD_REBUILD_INFO_DAT psdRebuildDone;
    LOG_PDD_REBUILD_DONE_DAT pddRebuildDone;
    LOG_HSPARE_DONE_PKT hspareDone;
    LOG_RAID_INIT_DONE_PKT raidInitDone;
    LOG_PDISK_FAIL_OP_PKT pdiskFailOp;
    LOG_PDISK_UNFAIL_OP_PKT pdiskUnFailOp;
    LOG_DEVICE_FAIL_HS_DAT deviceFailHS;
    LOG_SMART_EVENT_PKT smartEvent;
    LOG_DEVICE_TIMEOUT_PKT deviceTimeout;
    LOG_DEVICE_FAIL_NO_HS_DAT deviceFailNoHS;
    LOG_DEVICE_MISSING_PKT deviceMissing;
    LOG_CACHE_FLUSH_FAILED_PKT cacheFlushFailed;
    LOG_FIRMWARE_ALERT_DAT firmwareAlert;
    LOG_SERIAL_WRONG_PKT serialWrong;
    LOG_MAG_DRIVER_ERR_PKT magDriverErr;
    LOG_HOST_NONSENSE_PKT hostNonSense;
    LOG_HOST_QLOGIC_ERR_PKT hostQlogicErr;
    LOG_RAID_EVENT_PKT raidEvent;
    LOG_I2C_INIT_FAILED_PKT i2cInitFailed;
    LOG_CACHE_DRAM_FAIL_PKT cacheDramFail;
    LOG_CACHE_RECOVER_FAIL_PKT cacheRecoverFail;
    LOG_SES_WWN_SLOT_DAT sesWWNSlot;
    LOG_SES_WWN_TEMP_DAT sesTemp;
    LOG_SES_WWN_VOLT_DAT sesVoltage;
    LOG_SES_WWN_CURR_DAT sesCurrent;
    LOG_SES_WWN_SBOD_EXT_DAT sesSBODExt;
    LOG_SES_ELEM_CHANGE_DAT sesElemChange;
    LOG_SESSION_START_STOP_PKT configSessionStartStop;
    LOG_ERROR_TRAP_PKT logErrorTrap;
    LOG_WC_SEQNO_BAD_DAT wcSeqNoBad;
    LOG_VID_RECOVERY_FAIL_DAT vidRecoveryFail;
    LOG_INVALID_TAG_PKT invalidTag;
    LOG_FILEIO_ERR_PKT fileioErr;
    LOG_MSG_DELETED_PKT msgDeleted;
    LOG_FW_VERSIONS_PKT firmwareVersions;
    LOG_I2C_BATTERY_CONTROL_PKT batteryControl;
    LOG_VLINK_CREATE_OP_PKT vlinkCreateOp;
    LOG_ELECTION_STATE_TRANSITION_PKT electionStateTransition;
    LOG_PROC_NOT_READY_PKT procNotReady;
    LOG_SOCKET_ERROR_PKT socketError;
    LOG_LOST_PATH_PKT lostPath;
    LOG_LOST_ALL_PATHS_PKT lostAllPaths;
    LOG_NEW_PATH_PKT newPath;
    LOG_CACHE_MIRROR_FAILED_DAT cacheMirrorFailed;
    LOG_NVA_BAD_PKT nvaBad;
    LOG_NVA_RESYNC_FAILED_PKT nvaResyncFailed;
    LOG_SINGLE_PATH_PKT singlePath;
    LOG_HOTSPARE_INOP_PKT hotspareInop;
    LOG_HOTSPARE_DEPLETED_PKT hotspareDepleted;
    LOG_FS_UPDATE_FAIL_PKT fsUpdateFail;
    LOG_FS_UPDATE_PKT fsUpdate;
    LOG_WRITE_FLUSH_COMPLETE_DAT cacheFlushComplete;
    LOG_RAID_ERROR_DAT raidError;
    LOG_DEFRAG_DONE_DAT defragDone;
    LOG_ETHERNET_LINK_STATUS_CHANGE_PKT ethernetLinkStatusChange;
    LOG_FE_BE_SERIAL_MISMATCH_PKT FeBeSerialMismatch;
    LOG_SINGLE_BIT_ECC_PKT singleBitECC;
    LOG_RESYNC_DONE_PKT resyncDone;
    LOG_IPC_LINK_UP_PKT ipcLinkUp;
    LOG_IPC_LINK_DOWN_PKT ipcLinkDown;
    LOG_LOCAL_IMAGE_READY_PKT localImage;
    LOG_IPC_BROADCAST_PKT ipcBroadcast;
    LOG_PARITY_SCAN_REQUIRED_PKT parityScanReq;
    LOG_STATUS_ONLY_PKT statusOnly;
    LOG_HEARTBEAT_STOP_PKT heartbeatStop;
    LOG_VLINK_CHANGE_NAME_PKT vlinkChangeName;
    LOG_VLINK_GET_NAME_PKT vlinkGetName;
    LOG_MB_FAILED_DAT mbxfailed;
    LOG_IOCB_TO_PKT iocbTo;
    LOG_TEXT_MESSAGE_OP_PKT textMessageOp;
    LOG_FAILURE_EVENT_PKT failureEvent;
    LOG_ADMIN_SETTIME_OP_PKT sysTime;
    LOG_ADMIN_SET_IP_OP_PKT ipAddressChange;
    LOG_SNAPSHOT_TAKEN_PKT snapshotTaken;
    LOG_SNAPSHOT_RESTORED_PKT snapshotRestored;
    LOG_SNAPSHOT_RESTORE_FAILED_PKT snapshotRestoreFailed;
    LOG_PROC_ERR_TRAP_PKT procErrTrap;
    LOG_POWER_OFF_PKT powerOff;
    LOG_RM_EVENT_PKT rmEvent;
    LOG_BOOT_ENTRY_PKT bootEntry;
    LOG_LOOP_PRIMITIVE_OP_PKT loopPrimitiveOp;
    LOG_VALIDATION_MSG_PKT validation;
    LOG_XSSA_LOG_MESSAGE_PKT xssaMsg;
    LOG_DIAG_RESULT_PKT diagResult;
    LOG_MEMORY_HEALTH_PKT memoryHealth;
    LOG_CSTOP_LOG_PKT cStopLog;
    LOG_FCM_PKT fcalMonitor;
    LOG_CONTROLLER_FAIL_PKT controllerFail;
    LOG_VLINK_OPERATION_PKT vlinkOperation;
    LOG_DISASTER_PKT disaster;
    LOG_CACHE_MIRROR_CAPABLE_DAT cacheMirrorCapable;
//    LOG_PROC_PRINTF_PKT                     procPrintf;
    LOG_VLINK_OPEN_BEGIN_DAT vlinkOpenBegin;
    LOG_VLINK_OPEN_END_DAT vlinkOpenEnd;
    LOG_RAID5_INOPERATIVE_DAT raid5Inop;
    LOG_PARITY_CHECK_RAID_DAT parityCheckRAID;
    LOG_ORPHAN_DETECTED_DAT orphan;
    LOG_DEFRAG_VER_DONE_DAT defragVerDone;
    LOG_DEFRAG_OP_COMPLETE_DAT defragOpComplete;
    LOG_IPMI_EVENT_PKT ipmiEvent;
    LOG_NV_MEM_EVENT_DAT nvEvent;
    LOG_WC_FLUSH_DAT wcFlushLong;
    LOG_SCSI_TIMEOUT_PKT SCSITimeout;
    LOG_VDISK_SET_PRIORITY_PKT setVPri;
    LOG_VDISK_PR_CLR_PKT clrPR;
    LOG_PDISK_SPINDOWN_OP_PKT pSpindown;
    LOG_PDISK_FAILBACK_OP_PKT pFailback;
    LOG_AUTO_FAILBACK_INFO_OP_PKT pFailBackinfo;
    LOG_AF_ENABLE_DISABLE_INFO_OP_PKT pAFStatusinfo;
    LOG_REINIT_DRIVE_INFO_OP_PKT pReinitDrive;

/*    LOG_PDISK_AUTO_FAILBACK_OP_PKT          pAutoFailback;*/

    LOG_SET_GEO_LOCATION_OP_PKT             psetGeoLocation;
    LOG_CLEAR_GEO_LOCATION_OP_PKT           pclearGeoLocation;
    LOG_NO_HS_LOCATION_CODE_MATCH_PKT       noHsLocationMatch;
    LOG_GR_EVENT_DAT                        grData;
    LOG_CM_EVENT_DAT                        cmData;
    LOG_ICL_EVENT_OP_PKT                    iclLog;

#if ISCSI_CODE
    LOG_SERVER_LOGGED_IN_PKT pSrvrLoggedIn;
    LOG_TARGET_UP_PKT pTargetUp;
    LOG_ISCSI_SET_INFO_PKT piSCSISetInfo;
    LOG_ISCSI_SET_CHAP_PKT piSCSISetChap;
    LOG_ISCSI_GENERIC_OP_PKT piSCSIGeneric;
#endif
    LOG_APOOLEVENT_DAT pasync;
    LOG_SPOOLEVENT_DAT psnap;
    LOG_GENERIC_OP_PKT pGeneric;
    LOG_COPY_LABEL_DAT corLabel;
    LOG_DRIVE_DELAY_PKT driveDelay;
    LOG_ISE_ELEM_CHANGE_DAT iseElemChange;
    LOG_RAID_INOPERATIVE_DAT raidInop;
    LOG_GLOBAL_CACHE_MODE_PKT pGlobalCacheMode;
    LOG_ISE_SPECIAL_DAT iseSpecial;
} LOG_DATA_PKT;

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _LOGDEF_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
