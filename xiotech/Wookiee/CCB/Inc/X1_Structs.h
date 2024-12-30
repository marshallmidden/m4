/* $Id: X1_Structs.h 143845 2010-07-07 20:51:58Z mdr $*/
/**
******************************************************************************
**
**  @file       X1_Structs.h
**
**  @brief      Data structures for X1 Packet Interface Commands
**
** Copyright (c) 2001-2009  Xiotech Corporation.  All rights reserved.
**
******************************************************************************
**/
#ifndef _X1_STRUCTS_H_
#define _X1_STRUCTS_H_

#include "CacheManager.h"
#include "CacheMisc.h"
#include "DEF_Workset.h"
#include "globalOptions.h"
#include "logging.h"
#include "PacketInterface.h"
#include "quorum.h"
#include "XIO_Const.h"
#include "XIO_Types.h"
#include "xci_structure.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/

/**
**  @name   Map size constants
**
** During the proc code conversion it was discovered that some of the system
** constants that should be identical in Proc and CCB code are not.  Changing
** the constants in the CCB code effects some X1 packets and breaks XIOService.
** To avoid this, new constants are defined below.  At some point it would
** be nice to have the Proc, CCB and X1 packets all use the same constants.
** Lack of flexibility in the X1 design makes this difficult.
**/
/* @{ */
#define X1_MAX_DISK_BAYS                16
#define X1_VDISK_MAP_SIZE               ((1024 + 7) / 8)
#define X1_RAID_MAP_SIZE                ((2048 + 7) / 8)
#define X1_DISK_BAY_MAP_SIZE            ((X1_MAX_DISK_BAYS + 7) / 8)
/* @} */

/**
**  @name   Name length constants
**/
/* @{ */
#define X1_PROBE_NAME_LEN               8
#define X1_VLINK_CTRL_NAME_LEN          VLINK_NAME_LEN  /* See cachemgr.h */
#define X1_VLINK_NAME_LEN               VLINK_NAME_LEN  /* See cachemgr.h */

/*
** Maximum size for a virtual name, this matches the size the PROC uses
** for the name.  If this values changes the code for the names needs to
** be updated.
*/
#define X1_VDISK_NAME_LEN               X1_NAME_LEN

/*
** Maximum size for a server, this matches the size the PROC uses for
** the name.  If this values changes the code for the names needs to
** be updated.
*/
#define X1_SERVER_NAME_LEN              X1_NAME_LEN
/* @} */

/**
**  @name   PDisk Info constants
**/
/* @{ */
#define X1_DNAME_LEN                    4
#define X1_PRODID_LEN                   16
#define X1_VENDID_LEN                   8
#define X1_REV_LEN                      4
#define X1_SERIAL_LEN                   12
/* @} */

/**
**  @name   Log set constants
**/
/* @{ */
#define X1_SET_LOG_TYPE_TEXT            0xB4
#define X1_SET_LOG_MAX_MSG_LEN          MMC_MESSAGE_SIZE
/* @} */

/**
**  The default LUN value is used as the "starting point" when a server
**  is assigned to a VDisk using the X1VDISKSETMASK command.  The user
**  must then use the X!VDISKSETLUN command to assign a valkid LUN number.
**/
#define CFG_VDISK_DEFAULT_LUN           0xFF

/**
** Each X1 response packet contains a header which currently is just the
** command code.  A structure is defined below to make this easier to
** change if we want to expand this header in the future.
**/
typedef struct _X1_RSP_HDR
{
    UINT16      length;
    UINT8       cmdCode;
} X1_RSP_HDR;

/**
**  @name   Constants for devType in X1_PDISK_INFO_RSP
**/
/* @{ */
#define X1_PDISK_DEVTYPE_UNKNOWN        0x00    /**< Unknown device type                */
#define X1_PDISK_DEVTYPE_FC_DISK        0x01    /**< Fibre disk drive device type       */
#define X1_PDISK_DEVTYPE_SATA           0x02    /**< SATA disk drive device type        */
#define X1_PDISK_DEVTYPE_SSD            0x03    /**< Solid state disk drive device type */
/* @} */


/**
** Disk Bay Info Response Packet - Request Code 0x63
**/
typedef struct _X1_BAY_INFO_RSP
{
    X1_RSP_HDR  header;                 /**< 3-Standard response header     */
    UINT16      bayNumber;              /**< 2-Bay ID                       */
    UINT8       devStat;                /**< 1-Device status                */
    UINT8       prodId[X1_PRODID_LEN];  /**< 16-Product ID                  */
    UINT8       vendId[X1_VENDID_LEN];  /**< 8-Vendor ID                    */
    UINT8       rev[X1_REV_LEN];        /**< 4-PDisk firmware revision      */
    UINT8       serial[X1_SERIAL_LEN];  /**< 12-PDisk serial number         */
    UINT64      wwn;                    /**< 8-WWN                          */
    UINT8       loopStatus;             /**< 1-From SES Page 2              */
    UINT8       ioModuleStatus1;        /**< 1-From SES Page 2              */
    UINT8       ioModuleStatus2;        /**< 1-From SES Page 2              */
    UINT8       pathCount;              /**< 1-# of paths to the device     */
    UINT8       path[DEV_PATH_MAX];     /**< 4-LID values for each path     */
    UINT8       shelfId;                /**< 1-Bay shelf ID                 */

    /*
     ** Items below added at X1_COMPATIBILITY = 0x14 (SATA support)
     */
    UINT8       devType;                /**< 1-Device type                  */
    UINT8       rsvd[16];               /**< 16-RESERVED                    */
} X1_BAY_INFO_RSP;

/**
**  @name   Constants for devStat in X1_BAY_INFO_RSP
**/
/* @{ */
#define X1_BAY_DEVSTAT_NON_EXISTENT         0x00
#define X1_BAY_DEVSTAT_OK                   0x01
#define X1_BAY_DEVSTAT_INOPERATIVE          0x02
#define X1_BAY_DEVSTAT_MISSING              0x03
/* @} */

/**
**  @name   Constants for loopStatus in X1_BAY_INFO_RSP
**/
/* @{ */
#define X1_BAY_LOOP_STATUS_LRC_A_PRESENT    0x01
#define X1_BAY_LOOP_STATUS_LRC_A_LOOP_FAIL  0x02
#define X1_BAY_LOOP_STATUS_LRC_B_PRESENT    0x04
#define X1_BAY_LOOP_STATUS_LRC_B_LOOP_FAIL  0x08
#define X1_BAY_LOOP_STATUS_2GB_MODE         0x10
#define X1_BAY_LOOP_STATUS_SPEED_MISMATCH   0x20
#define X1_BAY_LOOP_STATUS_FW_MISMATCH      0x40
/* @} */

/**
**  @name   Constants for ioModuleStatus in X1_BAY_INFO_RSP
**/
/* @{ */
#define X1_BAY_IO_MODULE_STATUS_PRESENT     0x01
/* @} */

/**
**  @name   Constants for devType in X1_BAY_INFO_RSP
**/
/* @{ */
#define X1_BAY_DEVTYPE_FC_SES           0x0D    /**< FC bay type                        */
#define X1_BAY_DEVTYPE_SATA_SES         0x0E    /**< SATA bay type                      */
/* @} */

typedef struct _X1_ENV_STATS_RSP
{
    X1_RSP_HDR  header;
    INT8        ctrlTempHost; /**< Host side controller temp in degC        */
    INT8        ctrlTempStore; /**< Storage side controller temp in degC    */
    UINT8       ctrlAC1;    /**< Ctrl power supply 1 - b0: 1=good, 0=bad    */
    UINT8       ctrlAC2;    /**< Ctrl power supply 2 - b0: 1=good, 0=bad    */
    UINT8       ctrlDC1;    /**< Ctrl pwr supply 1- b7: 1=exists, 0= NOT exist
                             **                     b0: 1=good, 0=bad       */
    UINT8       ctrlDC2;    /**< Ctrl pwr supply 2- b7: 1=exists, 0= NOT exist
                             **                    b0: 1=good, 0=bad        */
    UINT8       ctrlFan1;   /**< Controller fan 1 - b0: 1=good, 0=bad       */
    UINT8       ctrlFan2;   /**< Controller fan 2 - b0: 1=good, 0=bad       */
    UINT8       ctrlBufferHost; /**< Host side buffer - b0: 1=good, 0=bad   */
    UINT8       ctrlBufferStore;/**< Storage side buffer - b0: 1=good, 0=bad*/
    UINT8       fibreBayExistBitmap[X1_DISK_BAY_MAP_SIZE]; /**< Fibre bay exists map */
    UINT8       fibreBayTempIn1[X1_MAX_DISK_BAYS]; /**< Fibre bay temp input 1 */
    UINT8       fibreBayTempIn2[X1_MAX_DISK_BAYS]; /**< Fibre bay temp input 2 */
    UINT8       fibreBayTempOut1[X1_MAX_DISK_BAYS]; /**< Fibre bay temp output 1*/
    UINT8       fibreBayTempOut2[X1_MAX_DISK_BAYS]; /**< Fibre bay temp output 2*/
    /**
    ** Fibre bay power supply and fan status
    **  bit7: 1=power supply 2 AC good, 0=fail
    **  bit6: 1=power supply 1 AC good, 0=fail
    **  bit5: 1=power supply 2 DC good, 0=fail
    **  bit4: 1=power supply 1 DC good, 0=fail
    **  bit3: 1=fan 4 good, 0=fail
    **  bit2: 1=fan 3 good, 0=fail
    **  bit1: 1=fan 2 good, 0=fail
    **  bit0: 1=fan 1 good, 0=fail
    **/
    UINT8       fibreBayPSFan[X1_MAX_DISK_BAYS];
    UINT32      mbPerSecond;    /**< Server MB per second * 100             */
    UINT32      ioPerSecond;    /**< Server I/O per second                  */
    UINT16      rsvd;
    UINT32      beHeartbeat;    /**< Backend processor heartbeat            */
    UINT32      feHeartbeat;    /**< Frontend processor heartbeat           */
    /*
     ** Items below added at X1_COMPATIBILITY = 0x14 (SATA support)
     */
    UINT8       sataBayExistBitmap[X1_DISK_BAY_MAP_SIZE]; /**< SATA bay exists map */
    UINT8       sataBayTempOut1[X1_MAX_DISK_BAYS]; /**< SATA bay temp input */
    UINT8       sataBayTempOut2[X1_MAX_DISK_BAYS]; /**< SATA bay temp output */
    /**
    ** SATA bay power supply status
    **  bit3: 1=power supply 2 AC good, 0=fail
    **  bit2: 1=power supply 1 AC good, 0=fail
    **  bit1: 1=power supply 2 DC good, 0=fail
    **  bit0: 1=power supply 1 DC good, 0=fail
    **/
    UINT8       sataBayPS[X1_MAX_DISK_BAYS];
    /**
    ** SATA bay fan status
    **  bit5: 1=fan 6 good, 0=fail
    **  bit4: 1=fan 5 good, 0=fail
    **  bit3: 1=fan 4 good, 0=fail
    **  bit2: 1=fan 3 good, 0=fail
    **  bit1: 1=fan 2 good, 0=fail
    **  bit0: 1=fan 1 good, 0=fail
    **/
    UINT8       sataBayFan[X1_MAX_DISK_BAYS];
} X1_ENV_STATS_RSP;

/*
** This is the X1_ENV_STATS_RSP structure, but without having X1_RSP_HEADER
** element.  The PI_ENVIRO_DATA_CTRL_AND_BAY_CMD is using this packet. Ewok
** does not need the X1 header in its packet, so this is a new structure.
*/
typedef struct _X1_ENV_STATS_RSP_PKT
{

    INT8    ctrlTempHost;   /**< Host side controller temp in degC          */
    INT8    ctrlTempStore;  /**< Storage side controller temp in degC       */

    UINT8   ctrlAC1;        /**< Ctrl power supply 1 - b0: 1=good, 0=bad    */
    UINT8   ctrlAC2;        /**< Ctrl power supply 2 - b0: 1=good, 0=bad    */

    UINT8   ctrlDC1;        /**< Ctrl pwr supply 1- b7: 1=exists, 0= NOT exist
                             **                     b0: 1=good, 0=bad       */
    UINT8   ctrlDC2;        /**< Ctrl pwr supply 2- b7: 1=exists, 0= NOT exist
                             **                    b0: 1=good, 0=bad        */

    UINT8   ctrlFan1;       /**< Controller fan 1 - b0: 1=good, 0=bad       */
    UINT8   ctrlFan2;       /**< Controller fan 2 - b0: 1=good, 0=bad       */

    UINT8   ctrlBufferHost;     /**< Host side buffer - b0: 1=good, 0=bad   */
    UINT8   ctrlBufferStore;    /**< Storage side buffer - b0: 1=good, 0=bad*/

    UINT8   fibreBayExistBitmap[X1_DISK_BAY_MAP_SIZE]; /**< Fibre bay exists map */
    UINT8   fibreBayTempIn1[X1_MAX_DISK_BAYS];  /**< Fibre bay temp input 1 */
    UINT8   fibreBayTempIn2[X1_MAX_DISK_BAYS];  /**< Fibre bay temp input 2 */
    UINT8   fibreBayTempOut1[X1_MAX_DISK_BAYS]; /**< Fibre bay temp output 1*/
    UINT8   fibreBayTempOut2[X1_MAX_DISK_BAYS]; /**< Fibre bay temp output 2*/

    /**
    ** Fibre bay power supply and fan status    /n
    **  bit7: 1=power supply 2 AC good, 0=fail  /n
    **  bit6: 1=power supply 1 AC good, 0=fail  /n
    **  bit5: 1=power supply 2 DC good, 0=fail  /n
    **  bit4: 1=power supply 1 DC good, 0=fail  /n
    **  bit3: 1=fan 4 good, 0=fail              /n
    **  bit2: 1=fan 3 good, 0=fail              /n
    **  bit1: 1=fan 2 good, 0=fail              /n
    **  bit0: 1=fan 1 good, 0=fail
    **/
    UINT8   fibreBayPSFan[X1_MAX_DISK_BAYS];

    UINT32  mbPerSecond;        /**< Server MB per second * 100             */
    UINT32  ioPerSecond;        /**< Server I/O per second                  */
    UINT16  rsvd;
    UINT32  beHeartbeat;        /**< Backend processor heartbeat            */
    UINT32  feHeartbeat;        /**< Frontend processor heartbeat           */

    /*
    ** Items below added at X1_COMPATIBILITY = 0x14 (SATA support)
    */
    UINT8   sataBayExistBitmap[X1_DISK_BAY_MAP_SIZE]; /**< SATA bay exists map */
    UINT8   sataBayTempOut1[X1_MAX_DISK_BAYS];  /**< SATA bay temp input */
    UINT8   sataBayTempOut2[X1_MAX_DISK_BAYS];  /**< SATA bay temp output */

    /**
    ** SATA bay power supply status             /n
    **  bit3: 1=power supply 2 AC good, 0=fail  /n
    **  bit2: 1=power supply 1 AC good, 0=fail  /n
    **  bit1: 1=power supply 2 DC good, 0=fail  /n
    **  bit0: 1=power supply 1 DC good, 0=fail  /n
    **/
    UINT8   sataBayPS[X1_MAX_DISK_BAYS];

    /**
    ** SATA bay fan status                      /n
    **  bit5: 1=fan 6 good, 0=fail              /n
    **  bit4: 1=fan 5 good, 0=fail              /n
    **  bit3: 1=fan 4 good, 0=fail              /n
    **  bit2: 1=fan 3 good, 0=fail              /n
    **  bit1: 1=fan 2 good, 0=fail              /n
    **  bit0: 1=fan 1 good, 0=fail
    **/
    UINT8   sataBayFan[X1_MAX_DISK_BAYS];
} X1_ENV_STATS_RSP_PKT;

/**
**  @name   Bit flags used to indicate GOOD or FAIL conditions for
**          controller elements.
**/
/* @{ */
#define CTRL_AC_GOOD            0x01
#define CTRL_AC_FAIL            0x00

#define CTRL_DC_PS_EXISTS       0x80
#define CTRL_DC_PS_NOT_EXIST    0x00
#define CTRL_DC_GOOD            0x01
#define CTRL_DC_FAIL            0x00

#define CTRL_FAN_GOOD           0x01
#define CTRL_FAN_FAIL           0x00

#define CTRL_BUFFER_GOOD        0x01
#define CTRL_BUFFER_FAIL        0x00
/* @} */

/**
**  @name   Flags for bay power and fan conditions.
**          1 byte is used to indicate 4 power and 4 fan conditions for
**          the fibre bays.  The code starts by assuming all elements are
**          GOOD (1=GOOD).  If an element is determined to be bad the mask
**          for the element is ANDed with the power/fan byte to set the
**          state for that element to FAIL (FAIL=0)
**/
/* @{ */
#define BAY_PWR_FAN_ALL_GOOD    0xFF

#define BAY_AC_PS2_FAIL         0x7F
#define BAY_AC_PS1_FAIL         0xBF
#define BAY_DC_PS2_FAIL         0xDF
#define BAY_DC_PS1_FAIL         0xEF

#define BAY_FAN4_FAIL           0xF7
#define BAY_FAN3_FAIL           0xFB
#define BAY_FAN2_FAIL           0xFD
#define BAY_FAN1_FAIL           0xFE

/*
** The power supply and fan configuration is different for SATA bays.
*/
#define SATA_BAY_PWR_ALL_GOOD   0x0F
#define SATA_BAY_FAN_ALL_GOOD   0x3F

#define SATA_BAY_AC_PS2_FAIL    0xF7
#define SATA_BAY_AC_PS1_FAIL    0xFB
#define SATA_BAY_DC_PS2_FAIL    0xFD
#define SATA_BAY_DC_PS1_FAIL    0xFE

#define SATA_BAY_FAN6_FAIL      0xDF
#define SATA_BAY_FAN5_FAIL      0xEF
#define SATA_BAY_FAN4_FAIL      0xF7
#define SATA_BAY_FAN3_FAIL      0xFB
#define SATA_BAY_FAN2_FAIL      0xFD
#define SATA_BAY_FAN1_FAIL      0xFE

/*
** This defines the bit position for Fibre Bay Fan 4.  Shift this right by 1
** to get Fan3, right again for Fan2, etc.
*/
#define FIBRE_BAY_FAN_BIT       0x08

/*
** This defines the bit position for SATA Bay Fan 6.  Shift this right by 1
** to get Fan5, right again for Fan4, etc.
*/
#define SATA_BAY_FAN_BIT        0x20
/* @} */

/*
** Data structures for firmware version info.
**
** Note: X1_VERSION_ENTRY is in cachemgr.h
*/

typedef struct _FWVERSIONS
{
    X1_VERSION_ENTRY system;    /**< System release       */
    X1_VERSION_ENTRY beBoot;    /**< Back end boot        */
    X1_VERSION_ENTRY beDiag;    /**< Back end diag        */
    X1_VERSION_ENTRY beRun;     /**< Back end run-time    */
    X1_VERSION_ENTRY feBoot;    /**< Front end boot       */
    X1_VERSION_ENTRY feDiag;    /**< Front end diag       */
    X1_VERSION_ENTRY feRun;     /**< Front end run-time   */
    X1_VERSION_ENTRY ccbBoot;   /**< CCB boot             */
    X1_VERSION_ENTRY ccbRun;    /**< CCB run-time         */
    X1_VERSION_ENTRY ql2200ef;  /**< QLogic 2200          */
    X1_VERSION_ENTRY ql2200efm; /**< QLogic 2200          */
    X1_VERSION_ENTRY ql2300ef;  /**< QLogic 2300          */
    X1_VERSION_ENTRY ql2300efm; /**< QLogic 2300          */
} X1_FWVERSIONS;

/**
**  @name   Index values into the X1_FWVERSIONS struct for the various
**          firmware components.
**/
/* @{ */
#define X1_VERSION_SYSTEM       0
#define X1_VERSION_BE_BOOT      1
#define X1_VERSION_BE_DIAG      2
#define X1_VERSION_BE_RUN       3
#define X1_VERSION_FE_BOOT      4
#define X1_VERSION_FE_DIAG      5
#define X1_VERSION_FE_RUN       6
#define X1_VERSION_CCB_BOOT     7
#define X1_VERSION_CCB_RUN      8
/* @} */

/**
**  @name   Macros to access the remote port number and remote Controller
**          Node ID from a remotePortId value.
**/
/* @{ */
#define RemotePortNumber(x)     ((UINT8)((x) >> 8))
#define RemotePortCNID(x)       ((UINT8)x)
/* @} */

typedef struct _X1_BE_LOOP_INFO_RSP
{
    X1_RSP_HDR  header;
    PORT_INFO   portInfo[MAX_BE_PORTS];
} X1_BE_LOOP_INFO_RSP;

/**
** RAID_INIT_INFO_CHANGED   - Request  Code 0xAE
**/
typedef struct _X1_GET_RAID_PERCENT_RSP
{
    X1_RSP_HDR  header;
    UINT16      raidID;    /**< Raid ID   */
    UINT8       pctRem;    /**< Percent Remaining   */
} X1_GET_RAID_PERCENT_RSP;

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _X1_STRUCTS_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
