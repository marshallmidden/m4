/* $Id: SES_Structs.h 159129 2012-05-12 06:25:16Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       SES_Structs.h
**
**  @brief      SCSI Enclosure Services data structures
**
**  To provide a common means of defining the SCSI Enclosure Services
**  (SES) data structure.
**
**  Copyright (c) 1996-2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _SES_STRUCTS_H_
#define _SES_STRUCTS_H_

#include "XIO_Types.h"

#include "XIO_Const.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/
/*
** LED commands
*/
#define SES_LED_OFF         0x00        /* LED off                  */
#define SES_LED_ID          0x01        /* LED identify             */
#define SES_LED_FAIL        0x02        /* LED fail                 */
#define SES_LED_REMOVE      0x04        /* LED request removal      */

/*
** Devices supported.
*/
#define SES_WWN_MAP_SIZE    0x400       /* 1K devices               */

/*
** Max slots we will support per cabinet.
*/
/* #define SES_MAX_SLOTS       0x200 */      /* 512 slots                */
/*
** Polling interval.  This is the amount of time that the CCB
** will wait before it polls the SES devices for any warning or
** error conditions.
*/
#if defined(MODEL_7000) || defined(MODEL_4700)
#define SES_POLL_INTERVAL1  15000
#define SES_POLL_INTERVAL   15000
#else  /* MODEL_7000 || MODEL_4700 */
#define SES_POLL_INTERVAL1  60000
#define SES_POLL_INTERVAL   300000
#endif /* MODEL_7000 || MODEL_4700 */

/*
** Commands supported constants.  These are both the page codes for the
** the send/receive diagnostics parameter data and the bit position in
** the "SupportedPages" field for the SES device.  Use the most significant
** bits for the "VU" type of fields.
*/
#define SES_CMD_SUPPORTED   0x00        /* Supported diags request  */
#define SES_CMD_CONFIG      0x01        /* Configuration request    */
#define SES_CMD_CONTROL     0x02        /* Control request          */
#define SES_CMD_STATUS      0x02        /* Status request           */
#define SES_CMD_HELP_TEXT   0x03        /* Unsupported - help text  */
#define SES_CMD_STRING_OUT  0x04        /* String out request       */
#define SES_CMD_STRING_IN   0x04        /* String in request        */
#define SES_CMD_THRESH_IN   0x05        /* Threshold in request     */
#define SES_CMD_THRESH_OUT  0x05        /* Threshold out request    */
#define SES_CMD_ARRAY_STAT  0x06        /* Unsupported - array stat */
#define SES_CMD_ELEMENT     0x07        /* Element descriptor req   */
#define SES_CMD_SHORT_STAT  0x08        /* Unsupported - short stat */

#define SES_CMD_LOOP_STAT_A 0x80        /* Xyratex port A status    */
#define SES_CMD_LOOP_STAT_B 0x81        /* Xyratex port B status    */
#define SES_CMD_PORT_CFG_A  0x82        /* Xyratex port control A   */
#define SES_CMD_PORT_CFG_B  0x83        /* Xyratex port control B   */
#define SES_CMD_ROUTING_A   0x86        /* Xyratex port A routing   */
#define SES_CMD_ROUTING_B   0x87        /* Xyratex port B routing   */

/*
** Element types
*/
#define SES_ET_UNSPECIFIED  0x00        /* Unspecified element type */
#define SES_ET_DEVICE       0x01        /* Device element type      */
#define SES_ET_POWER        0x02        /* Power supply element type*/
#define SES_ET_COOLING      0x03        /* Cooling element type     */
#define SES_ET_TEMP_SENSOR  0x04        /* Temp sensor element type */
#define SES_ET_DOOR_ALARM   0x05        /* Door alarm element type  */
#define SES_ET_AUD_ALARM    0x06        /* Audible alarm element typ*/
#define SES_ET_SES_ELEC     0x07        /* SES electronics element  */
#define SES_ET_SCC_ELEC     0x08        /* SCC electronics element  */
#define SES_ET_NV_CACHE     0x09        /* NV cache element type    */
                                        /* 0x0A is reserved         */
#define SES_ET_UPS          0x0B        /* UPS element type         */
#define SES_ET_DISPLAY      0x0C        /* Display element type     */
#define SES_ET_KEY_PAD      0x0D        /* Key pad element type     */
                                        /* 0x0E is reserved         */
#define SES_ET_SCSI_PORT    0x0F        /* SCSI port element type   */
#define SES_ET_LANGUAGE     0x10        /* Language element type    */
#define SES_ET_COMM_PORT    0x11        /* Comm port element type   */
#define SES_ET_VOLT_SENSOR  0x12        /* Volt sensor element type */
#define SES_ET_CURR_SENSOR  0x13        /* Curr sensor element type */
#define SES_ET_TARGET       0x14        /* SCSI target element type */
#define SES_ET_INIT         0x15        /* SCSI intiator element typ*/
#define SES_ET_SUBENCL      0x16        /* Subenclosure element type*/

/*
** Eurologic elements
*/
#define SES_ET_LOOP_STAT    0x80        /* Loop status element type */
#define SES_ET_CTRL_STAT    0x82        /* Ctrlr status element type*/

/*
** Xyratex elements
*/
#define SES_ET_SBOD_STAT    0x80        /* SBOD status for Xyratex  */

#define SES_ET_MAX_VAL      0xFF        /* Max value to use         */
#define SES_ET_INVALID      0xFF        /* Indicates invalid        */

/*
** Bit positions for page 02 (enclosure status/control page) status field.
*/
#define SES_ST_UNRECOV      0x01        /* Unrecoverable status     */
#define SES_ST_CRITICAL     0x02        /* Critical status          */
#define SES_ST_NONCRIT      0x04        /* Non-critical status      */
#define SES_ST_INFORM       0x08        /* Informative status       */
#define SES_ST_INVALIDOP    0x10        /* Invalid operation status */

/*
** Common control field in the status/control page.
*/
#define SES_CC_SELECT       0x80        /* Select                   */
#define SES_CC_PRDFAIL      0x40        /* Predictive failure       */
#define SES_CC_DISABLE      0x20        /* Disable                  */
#define SES_CC_SWAP         0x10        /* Swap                     */
#define SES_CC_STAT_MASK    0x0F        /* Status mask              */
#define SES_CC_STAT_UNSUPP  0x00        /* Status - unsupported     */
#define SES_CC_STAT_OK      0x01        /* Status - OK              */
#define SES_CC_STAT_CRIT    0x02        /* Status - Critical        */
#define SES_CC_STAT_NONCRIT 0x03        /* Status - Non-critical    */
#define SES_CC_STAT_UNREC   0x04        /* Status - Unrecoverable   */
#define SES_CC_STAT_UNINST  0x05        /* Status - Not installed   */
#define SES_CC_STAT_UNKNOWN 0x06        /* Status - Unknown         */
#define SES_CC_STAT_UNAVAIL 0x07        /* Status - Unavailable     */

/*
** Control 1 for device element.
*/
#define SES_C1DEV_DNR       0x40        /* Do not remove            */
#define SES_C1DEV_INSERT    0x08        /* Insert / req insert      */
#define SES_C1DEV_REMOVE    0x04        /* Remove / req remove      */
#define SES_C1DEV_IDENT     0x02        /* Identify / req identify  */
#define SES_C1DEV_REPORT    0x01        /* Report                   */

/*
** Control 2 for device element.
*/
#define SES_C2DEV_FAULT     0x40        /* Fault detected           */
#define SES_C2DEV_RFAULT    0x20        /* Request fault            */
#define SES_C2DEV_DEVOFF    0x10        /* Device off               */
#define SES_C2DEV_RBYPASSA  0x08        /* Request bypass A         */
#define SES_C2DEV_RBYPASSB  0x04        /* Request bypass B         */
#define SES_C2DEV_BYPASSA   0x02        /* A bypassed               */
#define SES_C2DEV_BYPASSB   0x01        /* B bypassed               */

/*
** Control 1 for power supply
*/
#define SES_C1PS_DCOVER     0x08        /* DC over voltage          */
#define SES_C1PS_DCUNDER    0x04        /* DC under voltage         */
#define SES_C1PS_DCCURRENT  0x02        /* DC over current          */

/*
** Control 2 for power supply
*/
#define SES_C2PS_FAIL       0x40        /* Fail / req fail          */
#define SES_C2PS_ON         0x20        /* On or off req / status   */
#define SES_C2PS_OFF        0x10        /* Power supply is off      */
#define SES_C2PS_TMPFAIL    0x08        /* Failed for overtemp      */
#define SES_C2PS_TMPWARN    0x04        /* Overtemp warning         */
#define SES_C2PS_ACFAIL     0x02        /* AC failed                */
#define SES_C2PS_DCFAIL     0x01        /* DC failed                */

/*
** Control 2 for fan element (no control 1)
*/
#define SES_C2FAN_FAIL      0x40        /* Fail / req fail          */
#define SES_C2FAN_ON        0x20        /* On or off req / status   */
#define SES_C2FAN_OFF       0x10        /* Fan is off               */
#define SES_C2FAN_SPD_MASK  0x0F        /* Fan speed mask           */

#define SES_C2FAN_SPD_OFF   0x00        /* Speed - off              */
#define SES_C2FAN_SPD_LOW   0x01        /* Speed - low              */
#define SES_C2FAN_SPD_L2    0x02        /* Speed - second lowest    */
#define SES_C2FAN_SPD_L3    0x03        /* Speed - third lowest     */
#define SES_C2FAN_SPD_L4    0x04        /* Speed - fourth lowest    */
#define SES_C2FAN_SPD_L5    0x05        /* Speed - fifth lowest     */
#define SES_C2FAN_SPD_INT   0x06        /* Speed - intermediate     */
#define SES_C2FAN_SPD_FST   0x07        /* Speed - fastest          */

/*
** Control 2 for temp sensor element (no control 1)
*/
#define SES_C2TS_OTFAIL     0x08        /* Over temp fail           */
#define SES_C2TS_OTWARN     0x04        /* Over temp warn           */
#define SES_C2TS_UTFAIL     0x02        /* Under temp fail          */
#define SES_C2TS_UTWARN     0x01        /* Under temp warn          */

/*
** Temperature thresholds for temperature sensors.
*/
#define SES_T_AMB_L_ERR     0x05        /* Low ambient error       */
#define SES_T_AMB_L_WARN    0x0B        /* Low ambient warning     */
#define SES_T_AMB_H_WARN    0x23        /* High ambient warning    */
#define SES_T_AMB_H_ERR     0x2A        /* High ambient error      */

#define SES_T_INT_L_ERR     0x05        /* Low internal error      */
#define SES_T_INT_L_WARN    0x0B        /* Low internal warning    */
#define SES_T_INT_H_WARN    0x2A        /* High internal warning   */
#define SES_T_INT_H_ERR     0x30        /* High internal error     */

#define SES_T_AMB_SLOT0     0x01        /* Ambient temp sensor     */
#define SES_T_AMB_SLOT1     0x02        /* Ambient temp sensor     */
#define SES_T_INT_SLOT0     0x03        /* Internal temp sensor    */
#define SES_T_INT_SLOT1     0x04        /* Internal temp sensor    */

/*
** Control 1 for audible alarm element (Eurologic only)
*/
#define SES_C1AA_DISABLE    0x01        /* Disabled                 */

/*
** Control 2 for audible alarm element (no control 1)
*/
#define SES_C2AA_REQMUTE    0x80        /* Request muted            */
#define SES_C2AA_MUTED      0x40        /* Muted                    */
#define SES_C2AA_SETREMIND  0x10        /* Set reminder             */
#define SES_C2AA_INFO       0x08        /* Informational level      */
#define SES_C2AA_NONCRIT    0x04        /* Non critical level       */
#define SES_C2AA_CRIT       0x02        /* Critical level           */
#define SES_C2AA_UNRECOV    0x01        /* Unrecoverable level      */

/*
** Control 1 for enclosure services controller element
*/
#define SES_C1ES_FAIL       0x04        /* Failed                   */
#define SES_C1ES_PRESENT    0x02        /* Present                  */
#define SES_C1ES_REPORT     0x01        /* Report                   */

/*
** Control for voltage sensor
*/
#define SES_VOLT_OWARN      0x08        /* Over warning             */
#define SES_VOLT_UWARN      0x04        /* Under warning            */
#define SES_VOLT_OCRIT      0x02        /* Over critical            */
#define SES_VOLT_UCRIT      0x01        /* Under critical           */

/*
** Control for current sensor
*/
#define SES_CURR_OWARN      0x08        /* Over warning             */
#define SES_CURR_OCRIT      0x02        /* Over critical            */

/*
** Control / status for Eurologic VU loop status element
*/
#define SES_C2EL_LOOPALRC   0x01        /* Loop A LRC is present    */
#define SES_C2EL_LOOPAFAIL  0x02        /* Loop A has failed        */
#define SES_C2EL_LOOPBLRC   0x04        /* Loop B LRC is present    */
#define SES_C2EL_LOOPBFAIL  0x08        /* Loop B has failed        */
#define SES_C2EL_2GBIT      0x10        /* Running at 2GBit         */
#define SES_C2EL_SPEEDMIS   0x20        /* Speed mismatch           */
#define SES_C2EL_FWMISMATCH 0x40        /* Firmware is mismatched   */

/*
** Control / status for Eurologic VU IO element
*/
#define SES_C2EI_TYPE_U     0x00        /* Type unknown             */
#define SES_C2EI_TYPE_CU    0x01        /* Type copper              */
#define SES_C2EI_TYPE_RAID  0x02        /* Type RAID controller     */
#define SES_C2EI_TYPE_OPT   0x03        /* Type optical             */

#define SES_C2EI_PRESENT    0x01        /* Device is present        */

/*
** Control / status for Xyratex VU SBOD element
*/
#define SES_C0XS_ADD_ST     0x80        /* Additional status avail  */
#define SES_C0XS_ES_MASK    0x7F        /* Extend status - see below*/

#define SES_CXS_EX_ST       0x80        /* Exception status avail   */


#define SES_C0XS_ES_OK      0x00        /* Ext stat - OK            */
#define SES_C0XS_ES_UC_PE   0x01        /* Ext stat - uP fail post  */
#define SES_C0XS_ES_UC_FE   0x02        /* Ext stat - uP fault      */
#define SES_C0XS_ES_ASIC_FE 0x03        /* Ext stat - ASIC broke    */
#define SES_C0XS_ES_ASIC_PE 0x04        /* Ext stat - ASIC fail post*/
#define SES_C0XS_ES_ASIC_PP 0x05        /* Ext stat - ASIC port fail*/
#define SES_C0XS_ES_DELTA   0x06        /* Ext stat - excess delta  */
#define SES_C0XS_ES_NO_STAT 0x7F        /* Ext stat - no status     */

/* State Code for Xyratex page 80/81 StateCode. */
#define SES_P80_SC_INSERTED  0x00       /* Inserted                 */
#define SES_P80_SC_W_BURSTWE 0x14       /* Warn Burst Word Errors   */
#define SES_P80_SC_W_HIGHWE  0x15       /* Warn High Word Errors    */
#define SES_P80_SC_W_HIGHCRC 0x17       /* Warn High CRC Errors     */
#define SES_P80_SC_W_CLKDLT  0x18       /* Warn Hich Clock Delta    */
#define SES_P80_SC_B_GENERIC 0x21       /* Bypassed Generic state   */
#define SES_P80_SC_B_NODRIVE 0x22       /* Bypassed No Drive/GBIC   */
#define SES_P80_SC_B_TRANSMT 0x23       /* Bypassed Transmit Fault  */
#define SES_P80_SC_B_LIPF8   0x24       /* Bypassed LIP(f8)detected */
#define SES_P80_SC_B_TIMEOUT 0x25       /* Bypassed Data Timeout    */
#define SES_P80_SC_B_RECEIVR 0x26       /* Bypassed Receiver Loss   */
#define SES_P80_SC_B_SYNC    0x27       /* Bypassed Sync Loss       */
#define SES_P80_SC_B_PORTTST 0x28       /* Bypassed Port Test Failed*/
#define SES_P80_SC_MB_SES    0x31       /* Manual Bypass SES control*/
#define SES_P80_SC_MB_RDPORT 0x32       /* M-Bypass Redundant Port  */
#define SES_P80_SC_MB_STALL  0x33       /* M-Bypass Stall threshold */
#define SES_P80_SC_MB_WERRBR 0x34       /* M-Bypass Burst Word Error*/
#define SES_P80_SC_MB_WERRRT 0x35       /* M-Bypass Word Error rate */
#define SES_P80_SC_MB_CRCBR  0x36       /* M-Bypass Burst CRC error */
#define SES_P80_SC_MB_CRCRT  0x37       /* M-Bypass CRC error rate  */
#define SES_P80_SC_MB_CLKDLT 0x38       /* M-Bypass Clock Delta     */
#define SES_P80_SC_MB_MIRROR 0x39       /* M-Bypass MIRROR config   */
#define SES_P80_SC_MB_HSTPRT 0x3B       /* M-Bypass Host Port Rules */
#define SES_P80_SC_MB_TRUNKC 0x3C       /* M-Bypass Trunk cable err */
#define SES_P80_SC_MB_OTHERT 0x3E       /* M-Bypass Other Threshold */
#define SES_P80_SC_LOOPBACK  0x3F       /* LoopBack                 */
#define SES_P80_SC_MB_INSERT 0x40       /* M-Bypass Insert Oscillate*/
#define SES_P80_SC_MB_LIPBR  0x41       /* M-Bypass LIP burst thresh*/
#define SES_P80_SC_MB_LIPRT  0x42       /* M-Bypass LIP rate thresho*/
#define SES_P80_SC_DIAGTRAN  0x4F       /* Diagnostic Transmit      */
#define SES_P80_SC_MB_DRIVE  0x50       /* M-Bypass Drive decided   */
#define SES_P80_SC_MB_DRIVEF 0x51       /* M-Bypass Drive has fault */
#define SES_P80_SC_UNKNOWN   0xFF       /* Unknown                  */

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/

/*
** Device configuration information.
*/
typedef struct SES_DEV_INFO_MAP
{
    UINT8   devVendor[8];           /* Drive vendor name                    */
    UINT8   devProdID[16];          /* Product name                         */
    UINT8   devFlags[8];            /* Flags for SES and other misc items   */
} SES_DEV_INFO_MAP;

/*
** Page 0 format.  This is the standard page format for a receive
** diag result from SPC.
*/
typedef struct SES_PAGE_00
{
    UINT8           PageCode;           /* Page code (0x00)         */
    UINT8           Rsvd;               /* Reserved                 */
    UINT16          Length;             /* Page length              */
    UINT8           Pages[1];           /* Pages - one or more      */
} SES_PAGE_00, *PSES_PAGE_00;

/*
** Page 01 format.  Note that the variable length data at the end
** is comprised of the enclosure descriptor which is variable and
** a number of type descriptor headers.
*/
typedef struct SES_ENCL_DESC
{
    UINT8           WWN[8];             /* World wide name          */
    UINT8           VendorID[8];        /* Vendor ID                */
    UINT8           ProductID[16];      /* Product ID               */
    UINT8           ProductRev[4];      /* Product revision         */
                                        /* Vendor specific data     */
} SES_ENCL_DESC, *PSES_ENCL_DESC;

typedef struct SES_XTEX_ENCL_DESC_VU
{
    UINT8           ProductSer[15];     /* Product Serial number    */
    UINT8           EnclConfig;         /* Enclosure config settings*/
    UINT8           Rsvd[7];            /* Reserved space           */
    UINT8           EnclOptSettings;    /* Optional encl settings   */
} *PSES_XTEX_ENCL_DESC_VU;

typedef struct SES_EURO_ENCL_DESC_VU
{
    UINT8           BackPlaneByte;      /* Backplane description    */
    UINT8           ShelfID;            /* Shelf selector value     */
    UINT8           BackPlaneType[30];  /* Backplane description    */
    UINT8           BackPlaneFunc[30];  /* Backplane function string*/
    UINT8           SerialNum[20];      /* Serial number            */
    UINT8           KernelVersion[10];  /* Kernel version           */
    UINT8           ApplVersion[10];    /* Application version      */
    UINT32          Rsvd0;              /* Reserved                 */
    UINT8           DriveHeight;        /* Drive height             */
    UINT8           BackPlanePart[16];  /* Back plane part number   */
    UINT8           BackPlaneSerial[16];/* Back plane serial number */
    UINT64          WWN;                /* World wide name          */
    UINT8           Null;               /* NULL byte                */
} *PSES_EURO_ENCL_DESC_VU;

typedef struct SES_TYPE_DESC
{
    UINT8           ElementType;        /* Element type             */
    UINT8           NumPossibleElem;    /* Number of possible elem  */
    UINT8           SubEnclosureID;     /* Sub-enclosure ID         */
    UINT8           TypeDescLength;     /* Type descriptor text len */
} SES_TYPE_DESC, *PSES_TYPE_DESC;

typedef struct SES_PAGE_01
{
    UINT8           PageCode;           /* Page code (0x01)         */
    UINT8           SubEnclosures;      /* Sub-enclosures (rsvd)    */
    UINT16          Length;             /* Length to follow         */
    UINT32          Generation;         /* Generation code          */
    UINT8           Rsvd0;              /* Reserved                 */
    UINT8           SubEnclosureID;     /* Sub-enclosure ID (rsvd)  */
    UINT8           NumElementTypes;    /* Number of element types  */
    UINT8           EnclDescLength;     /* Enclosure descriptor len */
                                        /* Variable area            */
                                        /* Enclosure descriptor and */
                                        /* type descriptor headers  */
} SES_PAGE_01, *PSES_PAGE_01;

/*
** Page 02 format.  Enclosure status/ctrl page.  There are a variable number
** of element control fields at the end of this structure.  There is a
** set of structures for each element type.  This set is comprised of
** an overall control field and a specific control field for each element
** within the enclosure.  For example, if there are power supplies and
** drives in an enclosure, there would be one control struct for overall
** control of the drives, then one control structure for each drive, then
** an overall control structure for the power supplies, then one control
** structure for each power supply.
*/
typedef struct SES_EC_GENERIC
{
    UINT8           Slot;               /* Slot address             */
    UINT8           Ctrl1;              /* Control 1                */
    UINT8           Ctrl2;              /* Control 2                */
} SES_EC_GENERIC;

typedef struct SES_EC_VOLTAGE
{
    UINT8           Ctrl;               /* Control                  */
    UINT16          Voltage;            /* Voltage                  */
} SES_EC_VOLTAGE;

typedef struct SES_EC_CURRENT
{
    UINT8           Ctrl;               /* Control                  */
    UINT16          Current;            /* Current                  */
} SES_EC_CURRENT;

typedef struct SES_EC_TEMP
{
    UINT8           Rsvd;               /* Reserved                 */
    UINT8           Temp;               /* Temperature              */
    UINT8           Ctrl2;              /* Control                  */
} SES_EC_TEMP;

typedef struct SES_CTRL_ELEC
{
    UINT8           Rsvd;               /* Reserved                 */
    UINT8           Ctrl;               /* Active controller        */
    UINT8           Rsvd2;              /* Reserved                 */
} SES_CTRL_ELEC;

typedef struct SES_EC_XTEX_SST
{
    UINT8           ExtStatus;          /* Extended status          */
    UINT8           Rsvd;               /* Reserved                 */
    UINT8           Ctrl2;              /* Control                  */
} SES_EC_XTEX_SST;

typedef struct SES_ELEM_CTRL
{
    UINT8           CommonCtrl;         /* Common ctrl - const below*/
    union
    {
        SES_EC_GENERIC  Generic;        /* Most element controls    */
        SES_EC_VOLTAGE  Volt;           /* Voltage control          */
        SES_EC_CURRENT  Current;        /* Current control          */
        SES_EC_TEMP     Temp;           /* Temperature control      */
        SES_EC_XTEX_SST SBODStat;       /* Xyratex SBOD status      */
        SES_CTRL_ELEC   Electronics;    /* Ctrl electronics status  */
    } Ctrl;
} SES_ELEM_CTRL, *PSES_ELEM_CTRL;

typedef struct SES_PAGE_02
{
    UINT8           PageCode;           /* Page code (0x02)         */
    UINT8           Status;             /* Status - constants below */
    UINT16          Length;             /* Length to follow         */
    UINT32          Generation;         /* Generation code          */
    ZeroArray(SES_ELEM_CTRL, Control);  /* Actual control elements  */
} SES_PAGE_02, *PSES_PAGE_02;

/*
** Max string out size.  This is the limit of the string display area
** for the device enclosure name.
*/
#define SES_MAX_NAME        0x14        /* 20 characters            */

/*
** Page 04 format.  String out page.  This page is used by the Xiotech
** enclosure to receive a name for the display.  It can also be read
** (string in) to read the name.
**
** For the Eurologic enclosure, this page is used for a number of items.
** The read of the string provides the mapping of the drives into slots
** in the enclosure to be used with things like LED control.  The output
** of the string is used for a variety of items including download of
** code.
*/
typedef struct SES_PAGE_04
{
    UINT8           PageCode;           /* Page code (0x04)         */
    UINT8           Subenclosure;       /* Sub-enclosure (rsvd)     */
    UINT16          Length;             /* Length to follow         */
} *PSES_PAGE_04;

typedef struct SES_PAGE_04_FW
{
    UINT8           pageCode;           /* Page code (0x04)         */
    UINT8           subEnclosure;       /* Sub-enclosure (rsvd)     */
    UINT16          length;             /* Length to follow         */
    UINT8           command;            /* Command code             */
    UINT8           data[0];            /* Data                     */
} SES_PAGE_04_FW;

typedef struct SES_PAGE_04_FW_XYRATEX
{
    UINT8           pageCode;           /* Page code (0x04)                 */
    UINT8           subEnclosure;       /* reserved ==> 0                   */
    UINT16          length;             /* Length to follow (network order) */
    UINT32          offset;             /* Offset in file   (network order) */
    UINT8           data[0];            /* Data (from s-rec file)           */
} SES_PAGE_04_FW_XYRATEX;

/* this is essentially the identify address frame from the attached device's phy*/
typedef struct SES_AIC_ADDITIONAL_ELEM
{
    UINT8           protocol;
    UINT8           desclength;
    UINT8           numdesc;
    UINT8           desctype;
    UINT8           devtype;
    UINT8           rsv1;
    UINT8           initiatorflags;
    UINT8           targetflags;
    UINT64          attachedsasaddr;
    UINT64          sasaddr;
    UINT8           phyident;   /* this is the other devices phy identifier not the expanders.*/
    UINT8           rsv2[7];
} SES_AIC_ADDITIONAL_ELEM;

typedef struct SES_AIC_PAGE_0A
{
    UINT8                   PageCode;           /* Page code (0x07)         */
    UINT8                   Rsvd;               /* Rsvd                     */
    UINT16                  Length;             /* Length to follow         */
    UINT32                  Generation;         /* Generation code          */
    SES_AIC_ADDITIONAL_ELEM elem[24];           /*there are 24 elements returned 1 per phy*/
} *PSES_AIC_PAGE_0A;

/*
** Page 80-FF format.  Vendor Unique SES enclosure page.
*/
typedef struct SES_P2_CTRL_XTEX
{
    UINT8           PageCode;           /* Page code (0x02)         */
    UINT8           ShortCtrl;          /* Short control            */
    UINT16          Length;             /* Length to follow         */
    UINT32          Generation;         /* Generation code          */
    UINT32          OvaDevCtrl;         /* Overall device control   */
    UINT32          DevCtrl[16];        /* Control for devices      */
    UINT32          OvaPsCtrl;          /* Overall power supply ctl */
    UINT32          Ps0Ctrl;            /* Power supply 0 control   */
    UINT32          Ps1Ctrl;            /* Power supply 1 control   */
    UINT32          OvaFanCtrl;         /* Overall Fan control      */
    UINT32          Fan0Ctrl;           /* Fan 0 control            */
    UINT32          Fan1Ctrl;           /* Fan 1 control            */
    UINT32          OvaTempCtrl;        /* Overall temp control     */
    UINT32          TempSens0Ctrl;      /* Temp sensor 0 ctrl ops   */
    UINT32          TempSens1Ctrl;      /* Temp sensor 1 ctrl A     */
    UINT32          TempSens2Ctrl;      /* Temp sensor 2 ctrl B     */
    UINT32          OvaAlrmCtrl;        /* Overall audible alarm ctl*/
    UINT32          AudAlrmCtrl;        /* Audible alarm control    */
    UINT32          OvaEnclElecCtrl;    /* Overall encl electronics */
    UINT32          EncElecACtrl;       /* Enclosure A electronics  */
    UINT32          EncElecBCtrl;       /* Enclosure B electronics  */
    UINT32          OvaDispCtrl;        /* Overall display control  */
    UINT32          DispCtrl;           /* Display control          */
} SES_P2_CTRL_XTEX;

/*
 * Port structure for page 80/81 below.
 */
typedef struct SES_P80XTEXPort
{
    UINT8           StateCode;          /* Overall Port Event Flags */ /* 14 */
    UINT8           WordErrorCount;     /* Word Error Count         */ /* 15 */
    UINT8           CRCErrorCount;      /* CRC Error Count          */ /* 16 */
    UINT8           ClockDelta;         /* Clock Delta              */ /* 17 */
    UINT8           LoopUpCount;        /* Loop Up Count            */ /* 18 */
    UINT8           InsertionCount;     /* Insertion Count          */ /* 19 */
    UINT8           StallCount;         /* Stall Count              */ /* 20 */
    UINT8           Utilization;        /* Utilization              */ /* 21 */
} SES_P80XTEXPort;

/*
 * The following is the Xyratex SBOD page 0x80 or 0x81 table.
 */
typedef struct SES_P80_XTEX
{
    UINT8           PageCode;           /* Page code (0x80 / 0x81)  */ /* 0 */
    UINT8           Status;             /* Status - constants below */ /* 1 */
    UINT16          Length;             /* Length to follow         */ /* 2-3 */
    UINT32          Generation;         /* Generation code          */ /* 4-7 */
    UINT8           SBODStatusPageSeq;  /* SBOD Status Page Sequence*/ /* 8 */
    UINT8           SystemStatus;       /* System Status bits       */ /* 9 */
    UINT8           HWRev;              /* Hardware Revision Level  */ /* 10 */
    UINT8           AllPortEventFlags;  /* Overall Port Event Flags */ /* 11 */
    UINT8           LIPPortValue;       /* LIP Port Value           */ /* 12 */
    UINT8           LoopUpCount;        /* Loop up Count            */ /* 13 */
    SES_P80XTEXPort Port[20];           /* Port statistics          */ /* 14-21 + 8*# */
} SES_P80_XTEX, *PSES_P80_XTEX;

/*
 * Port structure for page 82 below.
 */
typedef struct SES_P82XTEXPort
{
    UINT16          HostPortProfile;    /* Host port profile        */ /* 00-01 */
    UINT8           Spare;              /* Spare                    */ /*    02 */
    UINT8           EventNotification;  /* Event notification       */ /*    03 */
    UINT8           NoCommaTmr;         /* No comma timer           */ /*    04 */
    UINT8           SamplePeriod;       /* Sample period            */ /*    05 */
    UINT8           SampleFrameCnt;     /* Sample frame count       */ /*    06 */
    UINT8           SampleWErrThr;      /* Sample word error        */ /*    07 */
    UINT8           SampleCErrThr;      /* Sample CRC error         */ /*    08 */
} SES_P82XTEXPort;

/*
 * Control structure for page 82 below.
 */
typedef struct SES_P82XTEXCtl
{
    UINT8           Port;               /* Port number              */
    UINT8           Action;             /* Action to take           */
} SES_P82XTEXCtl;

/*
 * The following is the Xyratex SBOD page 0x82/0x83 table.
 */
typedef struct SES_P82_XTEX
{
    UINT8           PageCode;           /* Page code (0x82 / 0x83)  */ /*     0 */
    UINT8           Status;             /* Status - constants below */ /*     1 */
    UINT16          Length;             /* Length to follow         */ /*   2-3 */
    UINT32          Generation;         /* Generation code          */ /*   4-7 */
    UINT16          GlobalControl;      /* Global control           */ /*   8-9 */
    UINT8           LoopHealthTO;       /* Loop health timeout      */ /*    10 */
    UINT8           LoopHealthRetest;   /* Loop health retest       */ /*    11 */
    UINT8           TestBeforeInsertTmr;/* Port test before insert  */ /*    12 */
    UINT8           StallEventThresh;   /* Stall event threshold    */ /*    13 */
    SES_P82XTEXPort HostPortCfg;        /* Host port congifuration  */ /* 14-22 */
    SES_P82XTEXPort AltHostPortCfg;     /* Alt host port config     */ /* 23-31 */
    SES_P82XTEXPort DriveConfig;        /* Drive port information   */ /* 32-40 */
    UINT8           HostAlt;            /* Host or alt for each port*/ /*    41 */
    UINT8           Rsvd42;             /* Reservered               */ /*    42 */
    ZeroArray(SES_P82XTEXCtl, Control); /* Array of control elements*/ /* 43-44 */
} SES_P82_XTEX, *PSES_P82_XTEX;

/*
 * The following is the Xyratex SBOD page 0x86/0x87 table.
 */

#define SES_XTEX_SLOTS      16          /* Drive slots                      */
#define SES_XTEX_P86_INV     1          /* Invalid page value in flags      */
#define SES_XTEX_P86_REP     1          /* Reprt bit in flags in drive rec  */

typedef struct SES_P86XTEX_EXT
{
    UINT8           Flags;              /* Report bit               */
    UINT8           Rsvd1[2];           /* Reserved                 */
    UINT8           ALPA;               /* AL_PA for slot           */
} SES_P86XTEX_EXT;

typedef struct SES_P86_XTEX
{
    UINT8           PageCode;           /* Page code (0x86 / 0x87)  */ /*     0 */
    UINT8           Status;             /* Status - constants below */ /*     1 */
    UINT16          Length;             /* Length to follow         */ /*   2-3 */
    UINT32          Generation;         /* Generation code          */ /*   4-7 */
    UINT8           Flags;              /* Flags                    */ /*     8 */
    UINT8           Rsvd10[3];          /* Reserved                 */ /*  9-11 */
    SES_P86XTEX_EXT Drives[SES_XTEX_SLOTS]; /* Array of drive elems */ /* 12-75 */
} SES_P86_XTEX;

/*
** SES device data structure.  This will be an element of a linked
** list for the SES devices present in the system.
*/
typedef struct SES_DEVICE
{

    struct SES_DEVICE *NextSES;         /* Next SES device          */
    UINT64          WWN;                /* WWN of the SES device    */
    UINT32          SupportedPages;     /* Bit significant support  */
    UINT32          FCID;               /* Fibre channel ID         */
    UINT32          Generation;         /* Generation code          */
    UINT8           Channel;            /* Fibre channel adapter    */
    UINT8           devStat;            /* Device status            */
    UINT16          PID;                /* PID of the SES device    */
    UINT16          LUN;                /* Logical unit number      */
    UINT16          TotalSlots;         /* Total element slots      */
    PSES_PAGE_02    OldPage2;           /* Previous page 2 reading  */
    UINT8           Map[SES_ET_MAX_VAL+1];/* Map of type area       */
    UINT8           Slots[SES_ET_MAX_VAL+1];/* Number of slots      */
    UINT8           pd_rev[4];          /* revision                 */
    UINT8           devType;            /* Device type              */
    PSES_P80_XTEX   OldPage80;          /* Previous page 80 reading */
    PSES_P80_XTEX   OldPage81;          /* Previous page 81 reading */
} SES_DEVICE, *PSES_DEVICE;

/*
** Mapping of physical devices to SES controller and slot numbers.
** This structure will be used to form an array which will map a WWN
** to a SES device and slot number in the SES enclosure.
*/
typedef struct SES_WWN_TO_SES
{
    UINT64          WWN;                /* World wide name          */
    PSES_DEVICE     SES;                /* SES device pointer       */
    UINT16          Slot;               /* Slot number              */
    UINT16          PID;                /* Dev ID as seen from BEP  */
} SES_WWN_TO_SES;

#if defined(MODEL_7000) || defined(MODEL_4700)
typedef struct ISE_SES_DEVICE
{
    struct ISE_SES_DEVICE *NextISE;     /* Next ISE SES device     */
    UINT16          PID;                /* PID of the SES device   */
    UINT32          ip1;                /* first IP address of ISE */
    UINT32          ip2;                /* second IP address of ISE */
    UINT64          wwn;
} ISE_SES_DEVICE;
#endif /* MODEL_7000 || MODEL_4700 */

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _SES_STRUCTS_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
