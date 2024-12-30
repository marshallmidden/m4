/* $Id: logview.c 161092 2013-05-15 22:10:34Z marshall_midden $ */
/**
******************************************************************************
**
**  @file   logview.c
**
**  @brief  Log viewing implementation
**
**  Utility functions for viewing the flash based logs.
**
** Copyright (c) 2001-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#include "logview.h"

#include <sys/time.h>
#ifdef LOG_SIMULATOR
#include "LogSimFuncs.h"

#undef tfInetToAscii
#define tfInetToAscii InetToAscii
#else  /* LOG_SIMULATOR */
#include "CacheBay.h"
#include "CacheManager.h"
#include "CachePDisk.h"
#include "CacheServer.h"
#include "convert.h"
#include "ctype.h"
#include "globalOptions.h"
#include "LOG_Defs.h"
#include "HWM_ConditionStrings.h"
#include "HWM_StatusStrings.h"
#include "i82559_Strings.h"
#include "logdef.h"
#include "logging.h"
#include "quorum.h"
#include "errorCodes.h"
#include "Snapshot.h"
#include "ccb_flash.h"
#include "codeburn.h"
#include "convert.h"
#include "cps_init.h"
#include "debug_files.h"
#include "EL_Strings.h"
#include "FCM_Strings.h"
#include "ipc_packets.h"
#include "kernel.h"
#include "nvram.h"
#include "PortServerUtils.h"
#include "realtime.h"
#include "serial_num.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"
#endif /* LOG_SIMULATOR */

#include "L_Misc.h"

/*****************************************************************************
** Private defines
*****************************************************************************/

/*
**  These macros are used to index into the log data. The input is the
**  associated byte, short, or word index from the start of the array
*/
#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif  /* MIN */

#define BYTE(x)     data[x]
#define SHORT(x)    (*(UINT16*)(data + (2*x)))
#define WORD(x)     (*(int*)(data + (4*x)))
#define SWAPWORD(x) (SwapWord( (*(int*)(data + (4*x)))))
#define HEX2    "%#2.2hhx"      /* Leading 0x character arg    */
#define HEX4    "%#4.4hx"       /* Leading 0x short arg        */
#define HEX8    "%#8.8x"        /* Leading 0x                  */
#define HEX2N   "%2.2hhx"       /* No leading 0x character arg  */
#define HEX4N   "%4.4hx"        /* No leading 0x short arg      */
#define HEX8N   "%8.8x"         /* No leading 0x                */

#define  PDLOG_SCSI_SHORT_EVENT         0x00
#define  PDLOG_SCSI_LONG_EVENT          0x01
#define  PDLOG_DEVICE_INSERTED_PKT          0x10
#define  PDLOG_DEVICE_REMOVED_PKT           0x11
#define  PDLOG_DEVICE_TIMEOUT_PKT           0x12
// #define  PDLOG_DEVICE_SMART_ERROR       0x13
#define  PDLOG_DEVICE_FAILED            0x14
#define  PDLOG_DEVICE_FAILED_AND_SPARED 0x15
#define  PDLOG_DEVICE_MISSING_PKT           0x16
#define  PDLOG_DEVICE_REATTACHED        0x17

#define  PD "PDISK"

#if defined(MODEL_7000) || defined(MODEL_4700)
#define  BAY  "ISE"
#else  /* MODEL_7000 || MODEL_4700 */
#define  BAY  "DISKBAY"
#endif /* MODEL_7000 || MODEL_4700 */

typedef struct _PDLOG_PACKED_DATA
{
    UINT8       types;
    UINT8       srcEnclosure;
    UINT8       srcSlotId;
    UINT8       destEnclosure;
    UINT8       destSlotId;
    UINT8       eCode1;
    UINT8       reserved1[2];
    UINT8       srcSN[12];
    UINT64      srcWWN;
    UINT64      destWWN;
    UINT8       reserved2[4];
} PDLOG_PACKED_DATA;

typedef struct _PDLOG_SCSI_SHORT
{
    UINT8       types;
    UINT8       prpStatus;
    UINT8       scsiStatus;
    UINT8       enclosure;
    UINT8       slotId;
    UINT8       scsiCDB[21];
    UINT8       reserved[14];
} PDLOG_SCSI_SHORT;

typedef struct _PDLOG_SCSI_LONG
{
    UINT8       types;
    UINT8       prpStatus;
    UINT8       scsiStatus;
    UINT8       enclosure;
    UINT8       slotId;
    UINT8       scsiSense[32];
    UINT8       reserved[3];
} PDLOG_SCSI_LONG;

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
#if defined(MODEL_7000) || defined(MODEL_4700)
static INT32 Emprise_ValidDevOut(UINT64 wwn, UINT32 secToWait, MR_GET_PE_INFO_RSP *data,
                                 UINT16 pid, UINT16 lun);
#endif /* MODEL_7000 || MODEL_4700 */

static INT32 ValidDevOut(UINT64 wwn, UINT32 secToWait, MR_GET_PE_INFO_RSP *data);
static UINT32 GetDevType(UINT64 wwn, UINT32 secToWait);
static void ArrayToString(char *strPtr, UINT8 *array, UINT16 len);
static void VdiskIdToVblockVdiskIdString(UINT16 vid, char *strPtr);
static void ControllerSnToIndexString(UINT32 controllerSN, char *strPtr);
static void BCDToString(UINT8 BCDValue, char *strPtr);
static void StandardMessage(char *strPtr, LOG_HDR *logPtr);
static int  SwapWord(int word);
static void LongTimeToString(char *strTime, const time_t longTime, bool military);
static char *VerString(char *s);

static void Logview_BuildEventPropertiesString(HWM_EVENT_PROPERTIES *eventPropertiesPtr,
                                               char *strPtr, char *space, char *crlf);
static void Logview_BuildCCBStatusString(CCB_STATUS_PTR ccbStatusPtr,
                                         char *strPtr, char *space, char *crlf);
static void Logview_BuildProcessorBoardStatusString(PROC_BOARD_STATUS_PTR
                                                    processorBoardStatusPtr, char *strPtr,
                                                    char *space, char *crlf);
static void Logview_BuildPowerSupplyVoltagesStatusString(POWER_SUPPLY_VOLTAGES_STATUS_PTR
                                                         powerSupplyVoltagesStatusPtr,
                                                         char *strPtr, char *space,
                                                         char *crlf);
static void Logview_BuildProcessorStatusString(PROC_BOARD_PROCESSOR_STATUS_PTR
                                               processorStatusPtr, char *strPtr,
                                               char *space, char *crlf);
static void Logview_BuildTemperatureStatusString(TEMPERATURE_STATUS_PTR
                                                 temperatureStatusPtr, char *strPtr,
                                                 char *space, char *crlf);
static void Logview_BuildPowerSupplyStatusString(POWER_SUPPLY_STATUS_PTR
                                                 powerSupplyStatusPtr, char *strPtr,
                                                 char *space, char *crlf);
static void Logview_BuildBufferBoardStatusString(BUFFER_BOARD_STATUS_PTR
                                                 bufferBoardStatusPtr, char *strPtr,
                                                 char *space, char *crlf);
static void Logview_BuildBatteryStatusString(BATTERY_STATUS_PTR batteryStatusPtr,
                                             char *strPtr, char *space, char *crlf);
static void Logview_BuildFuelGaugeStatusString(FUEL_GAUGE_STATUS_PTR fuelGaugeStatusPtr,
                                               char *strPtr, char *space, char *crlf);
static void Logview_BuildMainRegulatorStatusString(MAIN_REGULATOR_STATUS_PTR
                                                   mainRegulatorStatusPtr, char *strPtr,
                                                   char *space, char *crlf);
static void Logview_BuildChargerStatusString(CHARGER_STATUS_PTR chargerStatusPtr,
                                             char *strPtr, char *space, char *crlf);
static void Logview_BuildNVRAMBatteryStatusString(NVRAM_BATTERY_STATUS_PTR
                                                  nvramBatteryStatusPtr, char *strPtr,
                                                  char *space, char *crlf);

static void DiskbayPsIdToString(UINT32 psId, char *strPtr);
static void DiskbayIoModIdToString(UINT32 id, char *strPtr);
static void ServerIdToNameString(UINT16 sid, char *strPtr);
static void ServerIdToWwnString(UINT16 sid, char *strPtr);

/*****************************************************************************
** Code Start
*****************************************************************************/

/**
******************************************************************************
**
**  @brief  Reads the requested log entry and stores it into a formatted string
**
**  @param  strPtr - pointer to where log entry string will be stored
**  @param  logPtr - pointer to log entry to convert to string
**
**  @return none
**
******************************************************************************
**/
void FormatLogEntry(char *strPtr, LOG_HDR *logPtr, char messageType)
{
    /*
     * Check to see if the logPtr points to a valid log entry, if not,
     * display an error message and return
     */
    if (!logPtr /*|| !ValidLogEntry(logPtr)*/)
    {
        sprintf(strPtr, "\r\nInvalid Log Entry pointer = %p\r\n", logPtr);
        return;
    }

    StandardMessage(strPtr, logPtr);    /* Create the standard message */

    /*
     * If extended message information is requested, append that
     * information onto the standard message.
     */
    if (messageType == LONG_MESSAGE)
    {
        ExtendedMessage(strPtr, logPtr, ASCII_TYPE, INDENT);
    }
}


/**
******************************************************************************
**
**  @brief  Create a formated text string with the message for the event code
**
**  @param  eventCode - event code value
**  @param  strPtr - pointer to store the constructed string
**
******************************************************************************
**/
static void StandardMessage(char *strPtr, LOG_HDR *logPtr)
{
    UINT32      seqNum;
    char        messStr[MAX_STANDARD_MESSAGE];
    char        timeStr[22];
    char        typeStr[8];
    char        statusStr[2];

    /*
     * Get the log entry status (in a string format).
     */
    GetStatusString(logPtr->le.status.flags, logPtr->eventCode, statusStr);

    seqNum = logPtr->sequence;          /* Get the sequence number */

    /*
     * Get the timestamp (in a string format).
     */
    GetTimeString(&logPtr->timeStamp, timeStr);

    /*
     * Get the message type (in a string format).
     */
    GetEventTypeString(logPtr->eventCode, typeStr);

    /*
     * Parse the event code and create the appropriate formatted message.
     */
    GetMessageString(logPtr, messStr, 0);

    sprintf(strPtr, "%s %5.5u %s %04hx %s %s\r\n", statusStr,
            seqNum, timeStr, logPtr->eventCode, typeStr, messStr);
}


#define DEV_OUT_UNKNOWN     0x80
#define DEV_OUT_PDISK       0x10
#define DEV_OUT_DISKBAY     0x11

#ifndef LOG_SIMULATOR
static UINT32 devOutSeq = 0;
#endif /* !LOG_SIMULATOR */


/**
******************************************************************************
**
**  @brief  Retrieves a valid disk bay or pdisk.
**
**  @param  wwn         - World Wide Name
**  @param  secToWait   - seconds to wait (number of tries w/1 sec delay)
**  @param  data        - MRGETPINFO_RSP to fill.
**
**  @return ERROR - if Unknown
**          DEV_OUT_PDISK - if valid pdisk
**          DEV_OUT_DISKBAY - if valid diskbay
**
******************************************************************************
**/
#ifdef LOG_SIMULATOR
static INT32 ValidDevOut(UNUSED UINT64 wwn, UNUSED UINT32 secToWait,
                UNUSED MR_GET_PE_INFO_RSP *data)
{
    return DEV_OUT_UNKNOWN;
}
#else  /* LOG_SIMULATOR */
static INT32 ValidDevOut(UINT64 wwn, UINT32 secToWait, MR_GET_PE_INFO_RSP *data)
{
    INT32       rc = GOOD;
    UINT8       quit = FALSE;
    UINT8       type = DEV_OUT_UNKNOWN;
    MRGETSINFO_RSP pdummyServer;
    CACHE_GOOD_DEVICE dummyPdisk;

    UINT32      thisSeq = ++devOutSeq;

    if (!data || !wwn)
    {
        return ERROR;
    }

    memset(data, 0x01, sizeof(*data));

    /* If the tables have been initialized */
    if (DiscoveryComplete())
    {
        /* Check to see if the wwn is a disk bay. */
        if ((type == DEV_OUT_UNKNOWN || type == DEV_OUT_DISKBAY) &&
            GetDiskBayInfoFromWwnNOW(wwn, data) == GOOD)
        {
            type = DEV_OUT_DISKBAY;

            /* If it is valid, we can quit. */
            if (data->pdd.pid != 0xFFFF)
            {
                rc = DEV_OUT_DISKBAY;
                quit = TRUE;
            }
        }

        /* Check to see if the wwn is a pDisk. */
        if ((type == DEV_OUT_UNKNOWN || type == DEV_OUT_PDISK) &&
            GetPDiskGoodDeviceFromWwnNOW(wwn, &dummyPdisk) == GOOD)
        {
            type = DEV_OUT_PDISK;

            /* If it is valid, we can quit. */
            if (dummyPdisk.ses != 0xFFFF && dummyPdisk.slot != 0xFF)
            {
                data->pdd.wwn = dummyPdisk.wwn;
                memcpy(data->pdd.serial,
                       dummyPdisk.serial, sizeof(data->pdd.serial));
                data->pdd.pid = dummyPdisk.pid;
                data->pdd.ses = dummyPdisk.ses;
                data->pdd.slot = dummyPdisk.slot;

                rc = DEV_OUT_PDISK;
                quit = TRUE;
            }
        }

        /* Check to see if the wwn is a server. */
        if (type == DEV_OUT_UNKNOWN)
        {
            /* If it is a server we can quit */
            if (GetServerInfoFromWwnNOW(wwn, &pdummyServer) == GOOD)
            {
                rc = ERROR;
                quit = TRUE;
            }
        }
    }

    /* If we did not find it. */
    if (quit == FALSE)
    {
        rc = ERROR;
        quit = TRUE;
    }

    if (secToWait > 0)
    {
        dprintf(DPRINTF_DEFAULT, "%s - Seq#: %u,  rc: %d(%s), wwn=%-16.16qx\n",
                __func__, thisSeq, rc,
                rc == ERROR ? "NOT FOUND" : (rc == DEV_OUT_PDISK ? "PDISK" : BAY),
                wwn);
    }

    return rc;
}
#endif /* LOG_SIMULATOR */

/*----------------------------------------------------------------------------
**  Function:   Emprise_ValidDevOut
**
**  Comments:   Retrieves a valid disk bay or pdisk.
**
**  Parameters: wwn         - World Wide Name
**              secToWait   - seconds to wait (number of tries w/1 sec delay)
**              data        - MRGETPINFO_RSP to fill.
**              pid         - PID of the PDD
**              lun         - lun number
**
**  Returns:    ERROR           - if Unknown
**              DEV_OUT_PDISK   - if valid pdisk
**              DEV_OUT_DISKBAY - if valid diskbay
**--------------------------------------------------------------------------*/
#if defined(MODEL_7000) || defined(MODEL_4700)
#ifdef LOG_SIMULATOR
static INT32 Emprise_ValidDevOut(UNUSED UINT64 wwn, UNUSED UINT32 secToWait,
        UNUSED MR_GET_PE_INFO_RSP *data, UNUSED UINT16 pid, UNUSED UINT16 lun)
{
    return DEV_OUT_UNKNOWN;
}
#else   /* LOG_SIMULATOR */
static INT32 Emprise_ValidDevOut(UINT64 wwn, UINT32 secToWait, MR_GET_PE_INFO_RSP *data,
                                 UINT16 pid, UINT16 lun)
{
    INT32       rc = GOOD;
    UINT8       quit = FALSE;
    UINT8       type = DEV_OUT_UNKNOWN;
    CACHE_GOOD_DEVICE dummyPdisk;

    UINT32      thisSeq = ++devOutSeq;

    if (!data || !wwn)
    {
        return ERROR;
    }

    memset(data, 0x01, sizeof(MR_GET_PE_INFO_RSP));

    /*
     * If the tables have been initialized
     */
    if (DiscoveryComplete())
    {
        /*
         * Check to see if the wwn is a disk bay.
         */
        if (lun == 0)
        {
            if ((type == DEV_OUT_UNKNOWN || type == DEV_OUT_DISKBAY) &&
                GetDiskBayInfoFromBidNOW(pid, data) == GOOD)
            {
                type = DEV_OUT_DISKBAY;

                /*
                 * If it is valid, we can quit.
                 */
                if (data->pdd.pid != 0xFFFF)
                {
                    rc = DEV_OUT_DISKBAY;
                    quit = TRUE;
                }
            }
        }               /* end of check for diskbay lun 0  Emprise */

        /*
         * Check to see if the wwn is a pDisk.
         */
        if ((type == DEV_OUT_UNKNOWN || type == DEV_OUT_PDISK) &&
            GetPDiskGoodDeviceFromPidNOW(pid, &dummyPdisk) == GOOD)
        {
            type = DEV_OUT_PDISK;

            /*
             * If it is valid, we can quit.
             */
            if (dummyPdisk.ses != 0xFFFF && dummyPdisk.slot != 0xFF)
            {
                data->pdd.wwn = dummyPdisk.wwn;
                memcpy(data->pdd.serial,
                       dummyPdisk.serial, sizeof(data->pdd.serial));
                data->pdd.pid = dummyPdisk.pid;
                data->pdd.ses = dummyPdisk.ses;
                data->pdd.slot = dummyPdisk.slot;

                rc = DEV_OUT_PDISK;
                quit = TRUE;
            }
        }

    }

    /*
     * If we did not find it.
     */
    if (quit == FALSE)
    {
        rc = ERROR;
    }

    if (secToWait > 0)
    {
        dprintf(DPRINTF_DEFAULT, "%s - Seq#: %u, rc: %d(%s), wwn=%-16.16qx\n",
                __func__, thisSeq, rc,
                rc == ERROR ? "NOT FOUND" : (rc == DEV_OUT_PDISK ? "PDISK" : BAY),
                wwn);
    }

    return rc;
}
#endif  /* LOG_SIMULATOR */
#endif /* MODEL_7000 || MODEL_4700 */

/*----------------------------------------------------------------------------
**  Function:   ArrayToString
**
**  Comments:   Converts a len byte array to a string.
**
**  Parameters: strPtr      - pointer to store the constructed string
**              array       - pointer to an array
**              len         - length of array
**
**--------------------------------------------------------------------------*/
static void ArrayToString(char *strPtr, UINT8 *array, UINT16 len)
{
    if ((strPtr != NULL) && (array != NULL) && (len > 0))
    {
        memcpy(strPtr, array, len);
        strPtr[len] = '\0';
    }
}

/*----------------------------------------------------------------------------
**  Function:   WWNToString
**
**  Comments:   Take an input wwn and create a string
**
**  Parameters: UINT64  wwn         - World Wide Name
**              char    *strPtr     - pointer to store the constructed string
**
**--------------------------------------------------------------------------*/
void WWNToString(UINT64 wwn, char *strPtr)
{
    sprintf(strPtr, "" HEX8N "" HEX8N "",
            SwapWord((UINT32)wwn), SwapWord((UINT32)(wwn >> 32)));
}

/*----------------------------------------------------------------------------
**  Function:   Emprise_WWNToShortNameString
**
**  Comments:   Take an input wwn and create a string
**
**  Parameters: UINT64  wwn         - World Wide Name
**              char    *strPtr     - pointer to store the constructed string
**              secToWait   - seconds to wait (number of tries w/1 sec delay)
**               pid        - PID
**               lun        - lun number
**
**--------------------------------------------------------------------------*/

#if defined(MODEL_7000) || defined(MODEL_4700)
static void Emprise_WWNToShortNameString(UINT64 wwn, char *strPtr, UINT32 secToWait,
                                         UINT16 pid, UINT8 devType, UINT16 lun)
{
    MRGETPINFO_RSP data;
    INT32       rc = GOOD;

    /*
     * If the wwn is 0 we can not look up any information.
     */
    if (wwn == 0)
    {
        strcpy(strPtr, "UNKNOWN");
        return;
    }

    /*
     * Retrieve the data.
     */
    switch (devType)
    {
        case PD_DT_ISE_HIGH_PERF:
        case PD_DT_ISE_PERF:
        case PD_DT_ISE_BALANCE:
        case PD_DT_ISE_CAPACITY:
            rc = Emprise_ValidDevOut(wwn, secToWait, &data, pid, lun);
            if (rc != ERROR)
            {
                sprintf(strPtr, "%02d-%02d", data.pdd.ses, lun);
            }
            else
            {
                WWNToString(wwn, strPtr);
                sprintf(strPtr + strlen(strPtr), "-%02d", pid);
            }
            break;
        case PD_DT_ISE_SES:
            sprintf(strPtr, "%02d", pid);
            break;
        default:
            /*
             * We have encountered neither a disk bay nor a pDisk,
             * or discovery is not complete so convert the wwn
             * to a string and return that instead.
             */
            strcpy(strPtr, "UNKNOWN");
            WWNToString(wwn, strPtr + strlen(strPtr));
            break;
    }
}
#endif /* MODEL_7000 || MODEL_4700 */

/*----------------------------------------------------------------------------
**  Function:   Emprise_DevInsWWNToShortNameString
**
**  Comments:   Take an input wwn and create a string
**
**  Parameters: UINT64  wwn         - World Wide Name
**              char    *strPtr     - pointer to store the constructed string
**              secToWait   - seconds to wait (number of tries w/1 sec delay)
**              pid        - PID
**              lun        - lun number
**
**--------------------------------------------------------------------------*/

#if defined(MODEL_7000) || defined(MODEL_4700)
static void Emprise_DevInsWWNToShortNameString(UINT64 wwn, char *strPtr, UINT32 secToWait UNUSED,
                                               UINT16 pid, UINT8 devType, UINT16 lun UNUSED)
{
    char        temp[16];

    /*
     * If the wwn is 0 we can not look up any information.
     */
    if (wwn == 0)
    {
        strcpy(strPtr, "UNKNOWN");
        return;
    }

    /*
     * Retrieve the data.
     */
    switch (devType)
    {
        case PD_DT_ISE_HIGH_PERF:
        case PD_DT_ISE_PERF:
        case PD_DT_ISE_BALANCE:
        case PD_DT_ISE_CAPACITY:
            sprintf(temp, "-%02d", pid);
            WWNToString(wwn, strPtr);
            strcat(strPtr, temp);
            break;

        case PD_DT_ISE_SES:
            sprintf(strPtr, "%02d", pid);
            break;

        default:
            /*
             * We have encountered neither a disk bay nor a pDisk,
             * or discovery is not complete so convert the wwn
             * to a string and return that instead.
             */
            WWNToString(wwn, strPtr);
            break;
    }
}
#endif /* MODEL_7000 || MODEL_4700 */

void WWNToShortNameString(UINT64 wwn, char *strPtr, UNUSED UINT32 secToWait)
{
#ifdef LOG_SIMULATOR
    WWNToString(wwn, strPtr);
#else  /* LOG_SIMULATOR */
    MRGETPINFO_RSP data;
    INT32       rc = GOOD;

    /*
     * If the wwn is 0 we can not look up any information.
     */
    if (wwn == 0)
    {
        strcpy(strPtr, "UNKNOWN");
        return;
    }

    /*
     * Retrieve the data.
     */
    rc = ValidDevOut(wwn, secToWait, &data);
    switch (rc)
    {
        case DEV_OUT_DISKBAY:
#if defined(MODEL_7000) || defined(MODEL_4700)
            sprintf(strPtr, "%02d", data.pdd.pid);
#else  /* MODEL_7000 || MODEL_4700 */
            sprintf(strPtr, "%c", ((char)data.pdd.pid + 'A'));
#endif /* MODEL_7000 || MODEL_4700 */
            break;

        case DEV_OUT_PDISK:
#if defined(MODEL_7000) || defined(MODEL_4700)
            sprintf(strPtr, "%02d-%02d", data.pdd.ses, data.pdd.slot);
#else  /* MODEL_7000 || MODEL_4700 */
            sprintf(strPtr, "%c%02d", ((char)data.pdd.ses + 'A'), data.pdd.slot);
#endif /* MODEL_7000 || MODEL_4700 */
            break;

        default:
            /*
             * We have encountered neither a disk bay nor a pDisk,
             * or discovery is not complete so convert the wwn
             * to a string and return that instead.
             */
            WWNToString(wwn, strPtr);
            break;
    }
#endif /* LOG_SIMULATOR */
}

/*----------------------------------------------------------------------------
**  Function:   PIDToShortNameString
**
**  Comments:   Take an input PID and create a string
**
**  Parameters: UINT16  pid         - physical disk identifier
**              char    *strPtr     - pointer to store the constructed string
**              secToWait   - seconds to wait (number of tries w/1 sec delay)
**
**--------------------------------------------------------------------------*/
static void PIDToShortNameString(UINT16 pid, char *strPtr, UNUSED UINT32 secToWait)
{
#ifdef LOG_SIMULATOR
    sprintf(strPtr, "PID-0x%x", pid);
#else  /* LOG_SIMULATOR */
    MRGETPINFO_RSP data;
    INT32       rc = GOOD;

    rc = GetPDiskInfoFromPid(pid, &data);

    if (rc == GOOD)
    {
        WWNToShortNameString(data.pdd.wwn, strPtr, secToWait);
    }
    else
    {
        sprintf(strPtr, "PID-0x%x", pid);
    }
#endif /* LOG_SIMULATOR */
}

/*----------------------------------------------------------------------------
**  Function:   GetDevType
**
**  Comments:   Take an input wwn and return the device type
**
**  Parameters: UINT64  wwn - World Wide Name
**              secToWait   - seconds to wait for cache refresh
**
**  Returns:    UINT32   device type
**--------------------------------------------------------------------------*/
static UINT32 GetDevType(UINT64 wwn, UINT32 secToWait)
{
    MRGETPINFO_RSP data;
    INT32       rc = GOOD;

    /*
     * Retrieve the data.
     */
    rc = ValidDevOut(wwn, secToWait, &data);

    return (UINT32)rc;
}

/*----------------------------------------------------------------------------
**  Function:   VdiskIdToVblockVdiskIdString
**
**  Comments:   Converts a Vdisk id to a Vblock/Vdisk Id String.
**
**  Parameters: vid     - Vdisk ID
**              strPtr  - pointer to store the constructed string
**
**--------------------------------------------------------------------------*/
static void VdiskIdToVblockVdiskIdString(UINT16 vid, char *strPtr)
{
    /*
     * Handle vid=0xFFFF as a special case.
     */
    if (vid == 0xFFFF)
    {
        strcpy(strPtr, "Undefined");
    }
    else
    {
        sprintf(strPtr, "#%d", vid);
    }
}

/*----------------------------------------------------------------------------
**  Function:   ControllerSnToIndexString
**
**  Comments:   Take a controller serail number and convert it to a string
**              If the serial number is in our controller map we will return
**              the index number of the controller in the map.  If the
**              controller is not in our controller map we will simply
**              return the serial number of the controller.
**
**  Parameters: controllerSN    - World Wide Name
**              strPtr          - pointer to store the constructed string
**
**--------------------------------------------------------------------------*/
static void ControllerSnToIndexString(UINT32 controllerSN, char *strPtr)
{
#ifdef LOG_SIMULATOR
    sprintf(strPtr, "SN %d", controllerSN);
#else  /* LOG_SIMULATOR */
    if (Qm_VCGIDFromSerial(controllerSN) == Qm_GetVirtualControllerSN())
    {
        sprintf(strPtr, "CN %d", Qm_SlotFromSerial(controllerSN));
    }
    else
    {
        sprintf(strPtr, "SN %d", controllerSN);
    }
#endif /* LOG_SIMULATOR */
}

/*----------------------------------------------------------------------------
**  Function:   ServerIdToNameString
**
**  Comments:   Take a server id it to a string.
**
**  Parameters: sid     - Server ID
**              strPtr  - pointer to store the constructed string
**
**--------------------------------------------------------------------------*/
static void ServerIdToNameString(UINT16 sid, char *strPtr)
{
#ifdef LOG_SIMULATOR
    sprintf(strPtr, "SID %d", sid);
#else   /* LOG_SIMULATOR */
    MRGETSINFO_RSP serverInfoOut;

    if (GetServerInfoFromSid(sid, &serverInfoOut) == GOOD)
    {
        if (GetServerInfoFromWwn(serverInfoOut.wwn, &serverInfoOut) == GOOD)
        {
            if (serverInfoOut.name[0] != 0)
            {
                ArrayToString(strPtr, serverInfoOut.name, MAX_SERVER_NAME_LEN);
            }
            else
            {
                sprintf(strPtr, "NONAME%d", serverInfoOut.sid);
            }
        }
        else
        {
            sprintf(strPtr, "NONAME%d", sid);
        }
    }
    else
    {
        sprintf(strPtr, "NONAME%d", sid);
    }
#endif  /* LOG_SIMULATOR */
}

/*----------------------------------------------------------------------------
**  Function:   ServerIdToWwnString
**
**  Comments:   Take a server id to a wwn string.
**
**  Parameters: sid     - Server ID
**              strPtr  - pointer to store the constructed string
**
**--------------------------------------------------------------------------*/
static void ServerIdToWwnString(UINT16 sid, char *strPtr)
{
#ifdef LOG_SIMULATOR
    sprintf(strPtr, "SID %d", sid);
#else   /* LOG_SIMULATOR */
    MRGETSINFO_RSP serverInfoOut;

    if (GetServerInfoFromSid(sid, &serverInfoOut) == GOOD)
    {
        WWNToString(serverInfoOut.wwn, strPtr);
    }
    else
    {
        sprintf(strPtr, "SID %d", sid);
    }
#endif  /* LOG_SIMULATOR */
}

/*----------------------------------------------------------------------------
**  Function:   DiskbayFanIdToString
**
**  Comments:   Take a diskbay fan number and convert it to a string.
**
**  Parameters: fanId    - Fan ID
**              strPtr   - pointer to store the constructed string
**
**--------------------------------------------------------------------------*/
static void DiskbayFanIdToString(UINT32 fanId, char *strPtr)
{
    sprintf(strPtr, "%d", fanId);
}

/*----------------------------------------------------------------------------
**  Function:   DiskbayPsIdToString
**
**  Comments:   Take a diskbay power supply number and convert it to a string.
**
**  Parameters: psId     - Power Supply ID
**              strPtr   - pointer to store the constructed string
**
**--------------------------------------------------------------------------*/
static void DiskbayPsIdToString(UINT32 psId, char *strPtr)
{
    sprintf(strPtr, "PS %c", ((char)psId + 'A'));
}

/*----------------------------------------------------------------------------
**  Function:   DiskbayIdToString
**
**  Comments:   Take a diskbay misc. id and converts it to a char string.
**
**  Parameters: psId     - ID
**              strPtr   - pointer to store the constructed string
**
**--------------------------------------------------------------------------*/
void DiskbayIdToString(UINT32 id, char *strPtr)
{
    sprintf(strPtr, "%c", ((char)id + 'A'));
}

/*----------------------------------------------------------------------------
**  Function:   DiskbayIoModIdToString
**
**  Comments:   Take a diskbay IO mod. id and converts it to a char string.
**
**  Parameters: psId     - ID
**              strPtr   - pointer to store the constructed string
**
**--------------------------------------------------------------------------*/
static void DiskbayIoModIdToString(UINT32 id, char *strPtr)
{
    sprintf(strPtr, "%c", ((char)id + 'A'));
}

/*----------------------------------------------------------------------------
**  Function:   DiskbayPortStateToString
**
**  Comments:   Take a diskbay port state from a Xyratex box and converts it
**              to a char string.
**
**  Parameters: state    - State value from page 0x80 or 0x81 of Xyratex box
**              strPtr   - pointer to store the constructed string
**
**--------------------------------------------------------------------------*/
static void DiskbayPortStateToString(UINT8 state, char *strPtr)
{
    /*
     * Most of these states will be displayed as a hex output.  Some of
     * them are more likely and interesting so they will be ascii text.
     */
    switch (state)
    {
        case SES_P80_SC_INSERTED:
            sprintf(strPtr, "inserted");
            break;

        case SES_P80_SC_W_BURSTWE:
            sprintf(strPtr, "burst word err");
            break;

        case SES_P80_SC_W_HIGHWE:
            sprintf(strPtr, "high word err");
            break;

        case SES_P80_SC_W_HIGHCRC:
            sprintf(strPtr, "high CRC err");
            break;

        case SES_P80_SC_W_CLKDLT:
            sprintf(strPtr, "high clk delta");
            break;

        case SES_P80_SC_B_GENERIC:
            sprintf(strPtr, "byp-generic");
            break;

        case SES_P80_SC_B_NODRIVE:
            sprintf(strPtr, "byp-unpop");
            break;

        case SES_P80_SC_B_TRANSMT:
            sprintf(strPtr, "byp-xmit flt");
            break;

        case SES_P80_SC_B_LIPF8:
            sprintf(strPtr, "byp-LIP F8");
            break;

        case SES_P80_SC_B_TIMEOUT:
            sprintf(strPtr, "byp-data t.o.");
            break;

        case SES_P80_SC_B_RECEIVR:
            sprintf(strPtr, "byp-sig loss");
            break;

        case SES_P80_SC_B_SYNC:
            sprintf(strPtr, "byp-sync loss");
            break;

        case SES_P80_SC_B_PORTTST:
            sprintf(strPtr, "byp-port fail");
            break;

        case SES_P80_SC_MB_SES:
            sprintf(strPtr, "byp-manual");
            break;

        case SES_P80_SC_MB_RDPORT:
            sprintf(strPtr, "mbyp-redun port");
            break;

        case SES_P80_SC_MB_STALL:
            sprintf(strPtr, "mbyp-stall");
            break;

        case SES_P80_SC_MB_WERRBR:
            sprintf(strPtr, "mbyp-burst word");
            break;

        case SES_P80_SC_MB_WERRRT:
            sprintf(strPtr, "mbyp-word err");
            break;

        case SES_P80_SC_MB_CRCBR:
            sprintf(strPtr, "mbyp-burst CRC");
            break;

        case SES_P80_SC_MB_CRCRT:
            sprintf(strPtr, "mbyp-CRC rate");
            break;

        case SES_P80_SC_MB_CLKDLT:
            sprintf(strPtr, "mbyp-clk delta");
            break;

        case SES_P80_SC_MB_MIRROR:
            sprintf(strPtr, "mbyp-mirror");
            break;

        case SES_P80_SC_MB_HSTPRT:
            sprintf(strPtr, "mbyp-hport rules");
            break;

        case SES_P80_SC_MB_TRUNKC:
            sprintf(strPtr, "mbyp-cable err");
            break;

        case SES_P80_SC_MB_OTHERT:
            sprintf(strPtr, "mbyp-other");
            break;

        case SES_P80_SC_LOOPBACK:
            sprintf(strPtr, "loopback");
            break;

        case SES_P80_SC_MB_INSERT:
            sprintf(strPtr, "mbyp-ins osc");
            break;

        case SES_P80_SC_MB_LIPBR:
            sprintf(strPtr, "mbyp-LIP burst");
            break;

        case SES_P80_SC_MB_LIPRT:
            sprintf(strPtr, "mbyp-LIP rate");
            break;

        case SES_P80_SC_DIAGTRAN:
            sprintf(strPtr, "diag xmit");
            break;

        case SES_P80_SC_MB_DRIVE:
            sprintf(strPtr, "mbyp-drive dec");
            break;

        case SES_P80_SC_MB_DRIVEF:
            sprintf(strPtr, "mbyp-drive flt");
            break;

        case SES_P80_SC_UNKNOWN:
            sprintf(strPtr, "unknown");
            break;

        default:
            sprintf(strPtr, "0x%02hhX", state);
            break;
    }
}

/*----------------------------------------------------------------------------
**  Function:   DiskbayElemIdToString
**
**  Comments:   Take a disk bay element type and converts it to a char string.
**
**  Parameters: type     - type of element
**              slot     - slot it is contained in
**              strPtr   - pointer to store the constructed string
**
**--------------------------------------------------------------------------*/

/*
** Strings to describe the element types for use in logging.
*/
static const char *SESElementStrings[SES_ET_SUBENCL + 1] =
{
    "unspecified",
    "Drive",
    "PS",
    "Fan",
    "Temp Sensor",
    "Door Alarm",
    "Aud Alarm",
    "SES Elec",
    "SCC Elec",
    "NV Cache",
    "Reserved",
    "UPS",
    "Display",
    "Key Pad",
    "Reserved",
    "SCSI Port",
    "Lang Elem",
    "Comm Port",
    "Volt Sensor",
    "Curr Sensor",
    "SCSI Targ",
    "SCSI Init",
    "Subenclosure"
};

static void DiskbayElemIdToString(UINT8 id, UINT32 slot, char *strPtr)
{
    /*
     * If the element is a drive, display it differently than the rest.
     */
    if (id == SES_ET_DEVICE)
    {
        sprintf(strPtr, "Drive in slot %d", slot);
    }
    else if (id == SES_ET_COOLING)
    {
        sprintf(strPtr, "%s %d", SESElementStrings[id], slot);
    }
    else if (id <= SES_ET_SUBENCL)
    {
        sprintf(strPtr, "%s %c", SESElementStrings[id], ((char)slot + 'A'));
    }
    else if ((id == SES_ET_LOOP_STAT) || (id == SES_ET_SBOD_STAT))
    {
        sprintf(strPtr, "Loop/SBOD Elem %c", ((char)slot + 'A'));
    }
    else if (id == SES_ET_CTRL_STAT)
    {
        sprintf(strPtr, "Ctrl Elem %c", ((char)slot + 'A'));
    }
    else
    {
        sprintf(strPtr, "unknown elem (0x%02hhX) %c", id, ((char)slot + 'A'));
    }
}

/*----------------------------------------------------------------------------
**  Function:   DiskbayStateChangeToString
**
**  Comments:   Take a old and new state of an element and converts
**              it to a char string.
**
**  Parameters: oldState - the old state
**              newState - the new state
**              strPtr   - pointer to store the constructed string
**
**--------------------------------------------------------------------------*/
static void DiskbayStateChangeToString(UNUSED UINT8 oldState, UINT8 newState,
                                       char *strPtr)
{
    /*
     * Make strings based upon the transitions.
     */
    switch (newState)
    {
        case SES_CC_STAT_UNSUPP:
            strcpy(strPtr, "unsupported");
            break;

        case SES_CC_STAT_OK:
            strcpy(strPtr, "is OK now");
            break;

        case SES_CC_STAT_CRIT:
            strcpy(strPtr, "crit error");
            break;

        case SES_CC_STAT_NONCRIT:
            strcpy(strPtr, "non-crit error");
            break;

        case SES_CC_STAT_UNREC:
            strcpy(strPtr, "unrecov error");
            break;

        case SES_CC_STAT_UNINST:
            strcpy(strPtr, "remvd/missing");
            break;

        case SES_CC_STAT_UNKNOWN:
            strcpy(strPtr, "unknown");
            break;

        case SES_CC_STAT_UNAVAIL:
            strcpy(strPtr, "unavailable");
            break;

        default:
            strcpy(strPtr, "undefined");
            break;
    }
}


typedef struct logmsg
{
    void    (*fp)(LOG_HDR *, LOG_DATA_PKT *, char *, UINT32, const struct logmsg *);
    const char  * const str;
    UINT8   opt;
    const char  * const dirstr;
} logmsg;


/**
******************************************************************************
**
**  @brief  Log text-only message
**
**  @param  l - Pointer to LOG_HDR - NOT USED
**  @param  dp - Pointer to LOG_DATA_PKT - NOT USED
**  @param  ostr - Pointer to output string buffer
**  @param  wait - Seconds to wait - NOT USED
**  @param  lm - Pointer to logmsg entry
**
**  @return none
**
******************************************************************************
**/
static void    logstr(LOG_HDR *l UNUSED, LOG_DATA_PKT *dp UNUSED, char *ostr,
                      UINT32 wait UNUSED, const logmsg *lm)
{
    strcpy(ostr, lm->str);
}


/**
******************************************************************************
**
**  @brief  Log device-related message
**
**  @param  l - Pointer to LOG_HDR - NOT USED
**  @param  dp - Pointer to LOG_DATA_PKT
**  @param  ostr - Pointer to output string buffer
**  @param  wait - Seconds to wait
**  @param  lm - Pointer to logmsg entry
**
**  @return none
**
******************************************************************************
**/
static void    logdev(LOG_HDR *l UNUSED, LOG_DATA_PKT *dp, char *ostr,
                      UINT32 wait, const logmsg *lm)
{
    char    tmpstr[MAX_MESSAGE_LENGTH];
    const char  *geostr;

#if defined(MODEL_7000) || defined(MODEL_4700)
    switch (lm->opt)
    {
    case 0:
        Emprise_WWNToShortNameString(dp->deviceRemoved.wwn, tmpstr, wait,
                 dp->deviceRemoved.pid, dp->deviceRemoved.devType,
                 dp->deviceRemoved.lun);
        break;

    case 1:
        Emprise_DevInsWWNToShortNameString(dp->deviceRemoved.wwn, tmpstr, wait,
                 dp->deviceRemoved.pid, dp->deviceRemoved.devType,
                 dp->deviceRemoved.lun);
        break;

    case 2:
        WWNToShortNameString(dp->deviceRemoved.wwn, tmpstr, wait);
        break;

    default:
        dprintf(DPRINTF_DEFAULT, "%s: Unknown option %d\n", __func__, lm->opt);
        WWNToShortNameString(dp->deviceRemoved.wwn, tmpstr, wait);
    }
#else  /* MODEL_7000 || MODEL_4700 */
    WWNToShortNameString(dp->deviceRemoved.wwn, tmpstr, wait);
#endif /* MODEL_7000 || MODEL_4700 */

    /*
     * Check if the cross geo location bit is set
     * If yes, log the message accordingly
     */
    geostr = "";
    if (BIT_TEST(dp->deviceInserted.geoflags, 1))
    {
        geostr = " across GEO locations";
    }

    sprintf(ostr, lm->str, tmpstr, geostr);
}


/**
******************************************************************************
**
**  @brief  Log SES slot-related messages
**
**  @param  l - Pointer to LOG_HDR - NOT USED
**  @param  dp - Pointer to LOG_DATA_PKT
**  @param  ostr - Pointer to output string buffer
**  @param  wait - Seconds to wait
**  @param  lm - Pointer to logmsg entry
**
**  @return none
**
******************************************************************************
**/
static void    logses_slot(LOG_HDR *l UNUSED, LOG_DATA_PKT *dp, char *ostr,
                           UINT32 wait, const logmsg *lm)
{
    char    tmpstr[MAX_MESSAGE_LENGTH];
    const char  *dirstr;

    WWNToShortNameString(dp->sesWWNSlot.wwn, tmpstr, wait);

    dirstr = lm->dirstr;
    if (dp->sesWWNSlot.direction)
    {
        /* Move to string after the first 0 */
        dirstr = &dirstr[strlen(dirstr) + 1];
    }

    sprintf(ostr, lm->str, tmpstr, dirstr, dp->sesWWNSlot.slot - 1);
}


#define LOGSTR(x1,x2) [LOG_GetCode(x1)] = { &logstr, (x2), 0, NULL }
#define LOGDEV(x1,x2,x3) [LOG_GetCode(x1)] = { &logdev, (x2), (x3), NULL }
#define LOGSES_SLOT(x1,x2,x3) [LOG_GetCode(x1)] = \
                { &logses_slot, (x2), 0, (x3) }

static logmsg logarr[] =
{
    LOGSTR(LOG_SCRUB_DONE, "SCRUB-cycle OK"),
    LOGSTR(LOG_NVRAM_RESTORE, "NVRAM-storage side restored"),
    LOGSTR(LOG_BOOT_COMPLETE, "BOOT-basic boot OK"),
    LOGSTR(LOG_SCRAMBLE_INFO, "Logs scrambled for testing"),
    LOGSTR(LOG_FILL_INFO, "Fill message for testing"),
    LOGSTR(LOG_BUFFER_BOARDS_ENABLED, "BUFFER-Boards READY"),
    LOGSTR(LOG_CONFIG_CHANGED, "CONFIG-changed"),
    LOGSTR(LOG_FW_VERSIONS, "FIRMWARE-versions report"),
    LOGSTR(LOG_PARITY_CHECK_DONE, "RAID-parity scan pass OK"),
    LOGSTR(LOG_I2C_BUS_GOOD, "I2C Bus OK"),
    LOGSTR(LOG_ETHERNET_LINK_UP, "CCB-Ethernet commlink OK"),
    LOGSTR(LOG_NVA_RESYNC_FAILED, "NVA-Could not resync records"),
    LOGSTR(LOG_PULSE, "SYSTEM-pulse OK"),
    LOGSTR(LOG_CACHE_TAGS_RECOVERED, "CACHE-tags recovered"),
    LOGSTR(LOG_NVRAM_RELOAD, "NVRAM-RELOADED"),
    LOGSTR(LOG_INIT_CCB_NVRAM_OP, "NVRAM-controller initialize OK"),
    LOGSTR(LOG_PROC_SYSTEM_NMI, "PROC-Unknown system trace"),
    LOGSTR(LOG_CONTROLLERS_READY, "POWERUP-all controllers ready"),
    LOGSTR(LOG_POWER_UP_COMPLETE, "POWERUP-sequencing OK"),
    LOGSTR(LOG_WRONG_SLOT, "POWERUP-controller in wrong slot"),
    LOGSTR(LOG_BAD_CHASIS, "POWERUP-cntrlr Auto node config failed"),
    LOGSTR(LOG_RM_EVENT_INFO, "RM-undefined code"),
    LOGSTR(LOG_RM_WARN, "RM-undefined code"),
    LOGSTR(LOG_RM_ERROR, "RM-undefined code"),
    LOGSTR(LOG_BUFFER_BOARDS_DISABLED_INFO, "BUFFER-Boards SHUTDOWN"),
    LOGSTR(LOG_CCB_NVRAM_RESTORED, "NVRAM-ccb restored"),
    LOGSTR(LOG_CACHE_MIRROR_FAILED, "Failed to mirror Write Info/Data"),
    LOGSTR(LOG_MIRROR_CAPABLE, "Mirror Write Info/Data IS OK NOW"),
    LOGSTR(LOG_ETHERNET_LINK_DOWN, "CCB-Ethernet commlink DOWN"),
    LOGSTR(LOG_MSG_DELETED, "FIRMWARE-update message deleted"),
    LOGSTR(LOG_BUFFER_BOARDS_DISABLED_WARN, "BUFFER-Board(s) NOT READY"),
    LOGSTR(LOG_ALL_DEV_MISSING, PD "s-all missing or inoperable"),
    LOGSTR(LOG_BUFFER_BOARDS_DISABLED_ERROR, "BUFFER-Board(s) FAIL"),
    LOGSTR(LOG_CACHE_DRAM_FAIL, "CACHE-write cache DRAM conflict"),
    LOGSTR(LOG_FOREIGN_PCI, "PCI-foreign card detected"),
    LOGSTR(LOG_CCB_NVRAM_RESET, "NVRAM-ccb FAILURE defaults restored"),
    LOGSTR(LOG_WC_SEQNO_BAD, "CACHE-bad sequence number"),
    LOGSTR(LOG_INVALID_TAG, "CACHE-invalid tag during recovery"),
    LOGSTR(LOG_PROC_NOT_READY, "PROCESSOR(S)-did not come ready"),
    LOGSTR(LOG_I2C_BUS_FAIL, "I2C Bus FAIL"),
    LOGSTR(LOG_ILLEGAL_ELECTION_STATE, "SYSTEM-illegal election state change"),
    LOGSTR(LOG_SERIAL_MISMATCH, "SYSTEM-Host/Storage serial mismatch"),
    LOGSTR(LOG_NVRAM_CHKSUM_ERR, "NVRAM-checksum error during restore"),
    LOGSTR(LOG_BATTERY_ALERT, "CACHE-battery alert"),
    LOGSTR(LOG_FW_UPDATE_FAILED, "FIRMWARE-update attempt FAIL"),
    LOGSTR(LOG_BE_INITIATOR, "HBA-Storage side initiator detected"),
    LOGSTR(LOG_PROC_COMM_NOT_READY, "POWERUP-processor comm not ready"),
    LOGSTR(LOG_NO_OWNED_DRIVES, "POWERUP-cannot locate any owned drives"),
    LOGSTR(LOG_CTRL_FAILED, "POWERUP-controller INACTIVE"),
    LOGSTR(LOG_CTRL_UNUSED, "POWERUP-controller config mismatch"),
    LOGSTR(LOG_FWV_INCOMPATIBLE, "POWERUP-controller fw incompatible"),
    LOGSTR(LOG_MISSING_DISK_BAY, "POWERUP-Missing disk bay(s)"),
    LOGSTR(LOG_WAIT_CORRUPT_BE_NVRAM, "POWERUP-BE NVRAM load corrupted"),
    LOGSTR(LOG_MISSING_CONTROLLER, "POWERUP-missing controller(s)"),
    LOGSTR(LOG_WAIT_DISASTER, "POWERUP-disaster recovery"),
    LOGSTR(LOG_DELAY_INOP, "Drive removal will cause INOP raids"),
    LOGSTR(LOG_CHANGE_LED, "LED-change"),
    LOGSTR(LOG_GET_LIST_ERROR, "SYSTEM-Storage list request ignored"),
    LOGSTR(LOG_LOCAL_IMAGE_READY, "NVRAM-local image is ready"),
    LOGSTR(LOG_REFRESH_NVRAM, "NVRAM-controller refresh request"),
    LOGSTR(LOG_RESYNC_DONE, "RESYNC-stripe resync completed"),
    LOGSTR(LOG_NVRAM_WRITTEN, "NVRAM-written"),
    LOGSTR(LOG_DVLIST, PD "-list sent to online"),
    LOGSTR(LOG_NO_LICENSE, "POWERUP-controller no valid license"),
    LOGSTR(LOG_NO_CONFIGURATION, "POWERUP-controller no configuration"),
    LOGSTR(LOG_NO_MIRROR_PARTNER, "CACHE-no mirror partner"),
    LOGSTR(LOG_CCB_MEMORY_HEALTH_ALERT, "CCB MEM-Correctable ECC rate >60/hr"),
    LOGSTR(LOG_NVRAM_WRITE_FAIL, "NVRAM-write verification failed"),
    LOGSTR(LOG_WC_SN_VCG_BAD, "CACHE-Swapped with DCN within DSC"),
    LOGSTR(LOG_WC_SN_BAD, "CACHE-Swapped with DCN foreign to DSC"),
    LOGSTR(LOG_WC_NVMEM_BAD, "CACHE-NV Memory is unavailable for use"),
    LOGSTR(LOG_WAIT_CACHE_ERROR, "POWERUP-buffer Error"),
    LOGSTR(LOG_ISNS_CHANGED, "ISNS Configuration Changed"),

    LOGDEV(LOG_DISKBAY_REMOVED, BAY "-%s removed%s", 2),
    LOGDEV(LOG_DISKBAY_MOVED, BAY "-%s moved%s", 2),
    LOGDEV(LOG_DISKBAY_INSERTED, BAY "-%s inserted%s", 0),
    LOGDEV(LOG_DEVICE_REMOVED, PD " %s removed%s", 0),
    LOGDEV(LOG_DEVICE_RESET, PD " %s reset%s", 0),
    LOGDEV(LOG_DEVICE_INSERTED, PD " %s inserted%s", 1),
    LOGDEV(LOG_DEVICE_REATTACED, PD "-%s reattached%s", 0),

    LOGSES_SLOT(LOG_SES_DEV_BYPA, BAY "-%s %sbypass A (%d)", "\0un"),
    LOGSES_SLOT(LOG_SES_DEV_BYPB, BAY "-%s %sbypass B (%d)", "\0un"),
    LOGSES_SLOT(LOG_SES_DEV_OFF, BAY "-%s slot %3$d %2$s", "OFF\0ON"),
    LOGSES_SLOT(LOG_SES_DEV_FLT, BAY "-%s (%d) %s", "FAULT\0OK"),
    LOGSES_SLOT(LOG_SES_SPEEDMIS, BAY "-%s SPD MSMTCH%s", "\0 OK"),
    LOGSES_SLOT(LOG_SES_FWMISMATCH, BAY "-%s FW MSMTCH%s", "\0 OK"),
};


/**
******************************************************************************
**  Convert MRC reason code into a string.
**
**  NOTE: Can only be one bit set.
**
******************************************************************************
**/
static void mrc_reason_string(char *tempStr1, unsigned long long code)
{
    int i;

    sprintf(tempStr1, "NO VALUE GIVEN-%lld", code);

    for (i = 0; i < 64; i++)
    {
        if (((1LL << i) & code) != 0)
        {
            switch (i)
            {
              case 0: strcpy(tempStr1, "0-UNKNOWN"); break;
              case 1: strcpy(tempStr1, "1-NONE"); break;
              case 2: strcpy(tempStr1, "2-RTC NOT RUNNING"); break;
              case 3: strcpy(tempStr1, "3-UNKNOWN"); break;
              case 4: strcpy(tempStr1, "4-ABL/POST FAULT"); break;
              case 5: strcpy(tempStr1, "5-HIGH TEMPERATURE"); break;
              case 6: strcpy(tempStr1, "6-OTHER MRC IN RESET"); break;
              case 7: strcpy(tempStr1, "7-NOT PRESENT"); break;
              case 8: strcpy(tempStr1, "8-OFFLINE"); break;
              case 9: strcpy(tempStr1, "9-BAD VPD CRC"); break;
              case 10: strcpy(tempStr1, "10-BAD VPD TYPE"); break;
              case 11: strcpy(tempStr1, "11-BAD VPD VERSION"); break;
              case 12: strcpy(tempStr1, "12-PERSISTENT FAULT"); break;
              case 13: strcpy(tempStr1, "13-UNKNOWN"); break;
              case 14: strcpy(tempStr1, "14-UNKNOWN"); break;
              case 15: strcpy(tempStr1, "15-UNKNOWN"); break;
              case 16: strcpy(tempStr1, "16-UNKNOWN"); break;
              case 17: strcpy(tempStr1, "17-UNKNOWN"); break;
              case 18: strcpy(tempStr1, "18-FC PORT DOWN"); break;
              case 19: strcpy(tempStr1, "19-MRC UNBOOTABLE"); break;
              case 20: strcpy(tempStr1, "20-UNKNOWN"); break;
              case 21: strcpy(tempStr1, "21-ERROR IN BATTERY CACHE"); break;
              case 22: strcpy(tempStr1, "22-FW UPGRADE"); break;
              case 23: strcpy(tempStr1, "23-UNKNOWN#0x2"); break;
              case 24: strcpy(tempStr1, "24-UNKNOWN#0x4"); break;
              case 25: strcpy(tempStr1, "25-UNSUPPORTED VPD VERSION"); break;
              case 26: strcpy(tempStr1, "26-BAD VPD"); break;
              case 27: strcpy(tempStr1, "27-UNKNOWN#0x00004000"); break;
              case 28: strcpy(tempStr1, "28-UNKNOWN#0x00010000"); break;
              case 29: strcpy(tempStr1, "29-UNKNOWN#0x00020000"); break;
              case 30: strcpy(tempStr1, "30-UNKNOWN#0x00040000"); break;
              case 31: strcpy(tempStr1, "31-UNKNOWN#0x00080000"); break;
              case 32: strcpy(tempStr1, "32-UNKNOWN#0x00100000"); break;
              case 33: strcpy(tempStr1, "33-UNKNOWN#0x00200000"); break;
              case 34: strcpy(tempStr1, "34-UNKNOWN#0x00400000"); break;
              case 35: strcpy(tempStr1, "35-UNKNOWN#0x00800000"); break;
              case 36: strcpy(tempStr1, "36-UNKNOWN#0x04000000"); break;
              case 37: strcpy(tempStr1, "37-UNKNOWN#0x08000000"); break;
              case 38: strcpy(tempStr1, "38-UNKNOWN#0x10000000"); break;
              case 39: strcpy(tempStr1, "39-UNKNOWN#0x40000000"); break;
              case 40: strcpy(tempStr1, "40-UNKNOWN#0x80000000"); break;
              case 41: strcpy(tempStr1, "41-UNKNOWN"); break;
              case 42: strcpy(tempStr1, "42-UNKNOWN"); break;
              case 43: strcpy(tempStr1, "43-UNKNOWN"); break;
              case 44: strcpy(tempStr1, "44-UNKNOWN"); break;
              case 45: strcpy(tempStr1, "45-UNKNOWN"); break;
              case 46: strcpy(tempStr1, "46-UNKNOWN"); break;
              case 47: strcpy(tempStr1, "47-UNKNOWN"); break;
              case 48: strcpy(tempStr1, "48-UNKNOWN"); break;
              case 49: strcpy(tempStr1, "49-UNKNOWN"); break;
              case 50: strcpy(tempStr1, "50-UNKNOWN"); break;
              case 51: strcpy(tempStr1, "51-UNKNOWN"); break;
              case 52: strcpy(tempStr1, "52-UNKNOWN"); break;
              case 53: strcpy(tempStr1, "53-UNKNOWN"); break;
              case 54: strcpy(tempStr1, "54-UNKNOWN"); break;
              case 55: strcpy(tempStr1, "55-UNKNOWN"); break;
              case 56: strcpy(tempStr1, "56-UNKNOWN"); break;
              case 57: strcpy(tempStr1, "57-UNKNOWN"); break;
              case 58: strcpy(tempStr1, "58-UNKNOWN"); break;
              case 59: strcpy(tempStr1, "59-UNKNOWN"); break;
              case 60: strcpy(tempStr1, "60-UNKNOWN"); break;
              case 61: strcpy(tempStr1, "61-UNKNOWN"); break;
              case 62: strcpy(tempStr1, "62-UNKNOWN"); break;
              case 63: strcpy(tempStr1, "63-UNKNOWN"); break;
            }
            break;
        }
    }
}   /* End of mrc_reason_string */

/**
******************************************************************************
**
**  @brief  Parse the passed event and create the associated message
**
**  @param  logPtr  - pointer to the "raw" log data
**  @param  messStr - pointer to store the constructed string
**  @param  secToWait   - seconds to wait for cache refresh
**
**  @return none
**
**  NOTES:  PLEASE KEEP EVENT CODES IN ORDER AS THEY APPEAR IN LOG_Defs.h!
**
******************************************************************************
**/
void GetMessageString(LOG_HDR *logPtr, char *messStr, UINT32 secToWait)
{
    char        tempStr1[MAX_MESSAGE_LENGTH] = "";
    char        tempStr2[MAX_MESSAGE_LENGTH] = "";
    char        tempStr3[MAX_MESSAGE_LENGTH] = "";
    char        tempStr4[MAX_MESSAGE_LENGTH] = "";

    char        errStr[MAX_MESSAGE_LENGTH] = "";

    LOG_DATA_PKT *dataPtr = (LOG_DATA_PKT *)(logPtr + 1);
    static const char *priStr[] = { "Normal", "Medium", "High" };

#if ISCSI_CODE
    static const char *loginStateStr[] = { "LOGIN", "LOGOUT" };
    static const char *targetStateStr[] = { "UP", "DOWN" };
    static const char *chapOption[] = { "ADD", "REMOVE" };
#endif /* ISCSI_CODE */

    /*
     * Construct message based on event code.
     * PLEASE KEEP EVENT CODES IN ORDER AS THEY APPEAR IN DEF.H!
     */

    UINT16  lix = LOG_GetCode(logPtr->eventCode);

    if (lix < dimension_of(logarr) && logarr[lix].fp)
    {
        logmsg  *lm = &logarr[lix];

        lm->fp(logPtr, dataPtr, messStr, secToWait, lm);
        return;
    }

    switch (LOG_GetCode(logPtr->eventCode))
    {
            /*
             * ----- Informational Events -----
             */

        case LOG_GetCode(LOG_GR_EVENT):
            switch (dataPtr->grData.eventType)
            {
                case GR_VLINK_ASSOCIATE:
                    VdiskIdToVblockVdiskIdString(dataPtr->grData.svid, tempStr1);
                    sprintf(messStr, "VL to GeoVDisk(%s) not allowed", tempStr1);
                    break;

                case GR_VLINK_CPYMIRROR:
                    VdiskIdToVblockVdiskIdString(dataPtr->grData.svid, tempStr1);
                    VdiskIdToVblockVdiskIdString(dataPtr->grData.dvid, tempStr2);
                    sprintf(messStr, "%s->%s-GeoVD-VL mirror not allowed", tempStr1,
                            tempStr2);
                    break;

                case GR_ASWAP_STATE_EVENT:
                    VdiskIdToVblockVdiskIdString(dataPtr->grData.svid, tempStr1);
                    VdiskIdToVblockVdiskIdString(dataPtr->grData.dvid, tempStr2);
                    switch (dataPtr->grData.aswapState)
                    {
                        case 1 /* GR_ASWAP_START */ :
                            sprintf(messStr, "ASWAP (%s) INITIATED", tempStr1);
                            break;
                        case 2 /* GR_ASWAP_SUCCESS */ :
                            sprintf(messStr, "ASWAP %s->%s OK", tempStr1, tempStr2);
                            break;
                        case 3 /* GR_ASWAP_FAILED */ :
                            sprintf(messStr, "ASWAP (%s) FAILED", tempStr1);
                            break;
                        case 4 /* GR_ASWAPBACK_RECOVERY_WAIT */ :
                            sprintf(messStr, "ASWAPBACK %s->%s RCRY/RESYNC-WAIT",
                                    tempStr1, tempStr2);
                            break;
                        case 6 /* GR_ASWAPBACK_START */ :
                            sprintf(messStr, "ASWAPBACK %s->%s INITIATED", tempStr1,
                                    tempStr2);
                            break;
                        case 7 /* GR_ASWAPBACK_SUCCESS */ :
                            sprintf(messStr, "ASWAPBACK %s->%s OK", tempStr1, tempStr2);
                            break;
                        case 8 /* GR_ASWAPBACK_FAILED */ :
                            sprintf(messStr, "ASWAPBACK %s->%s FAILED", tempStr1,
                                    tempStr2);
                            break;
                        case 9 /* GR_ASWAPBACK_CANCELLED */ :
                            sprintf(messStr, "ASWAPBACK %s->%s CANCELLED", tempStr1,
                                    tempStr2);
                            break;

                        case 10 /* GR_ASWAPBACK_HYSTERESIS_WAIT */ :
                            sprintf(messStr, "ASWAPBACK %s->%s HYSTERESIS-WAIT", tempStr1,
                                    tempStr2);
                            break;
                        default:
                            break;
                    }
                    break;
            }
            break;

        case LOG_GetCode(LOG_CM_EVENT):

             switch(dataPtr->cmData.eventType)
             {
                 case CM_INSTANT_MIRROR_DISABLE:
                      VdiskIdToVblockVdiskIdString(dataPtr->cmData.svid, tempStr1);
                      sprintf(messStr,"INSTANT MIRROR Disabled--VDisk(%s,%0x)", tempStr1,dataPtr->cmData.owner);
                      break;

                 default:
                      break;
             }
             break;

        case LOG_GetCode(LOG_ICL_PORT_EVENT):
            switch (dataPtr->iclLog.portState)
            {
                case LOG_ICLPORT_INIT_OK:
                    sprintf(messStr, "ICLPORT Initialization is OK");
                    break;
                case LOG_ICLPORT_INIT_FAILED:
                    sprintf(messStr, "ICLPORT Initialization failed");
                    break;
                case LOG_ICLPORT_LOOPUP:
                    sprintf(messStr, "ICLPORT Loop is UP");
                    break;
                case LOG_ICLPORT_LOOPDOWN:
                    sprintf(messStr, "ICLPORT Loop is DOWN");
                    break;
                case LOG_ICLPORT_CHIPRESET:
                    sprintf(messStr, "ICLPORT Port Reset");
                    break;
            }
            break;

        case LOG_GetCode(LOG_COPY_COMPLETE):
            VdiskIdToVblockVdiskIdString(dataPtr->copyComplete.srcVid, tempStr1);
            VdiskIdToVblockVdiskIdString(dataPtr->copyComplete.destVid, tempStr2);

            switch (dataPtr->copyComplete.compstat)
            {
                case CMCC_OK:
                    strcpy(tempStr3, "complete");
                    break;

                case CMCC_ABORT:
                    sprintf(tempStr3, "aborted (%dpct)", dataPtr->copyComplete.percent);
                    break;

                case CMCC_COPYERR:
                    strcpy(tempStr3, "FAIL");
                    break;

                case CMCC_DESTUPERR:
                    strcpy(tempStr3, "DESTUPERR");
                    break;

                case CMCC_SRCUERR:
                    strcpy(tempStr3, "SRCUPERR");
                    break;

                case CMCC_DESTDEL:
                    strcpy(tempStr3, "DESTDEL");
                    break;

                case CMCC_SRCDEL:
                    strcpy(tempStr3, "SRCDEL");
                    break;

                case CMCC_MIRROREND:
                    strcpy(tempStr3, "Mirror Ended");
                    break;

                case CMCC_USRTERM:
                    sprintf(tempStr3, "Copy/Mirror User Term (%dpct)",
                            dataPtr->copyComplete.percent);
                    break;

                case CMCC_AUTOTERM:
                    sprintf(tempStr3, "Copy/Mirror CM Term (%dpct)",
                            dataPtr->copyComplete.percent);
                    break;

                case CMCC_CPYSTART:
                    strcpy(tempStr3, "Copy Started");
                    break;

                case CMCC_CPYMIRROR:
                    strcpy(tempStr3, "Copy Mirrored");
                    break;

                case CMCC_RAIDSWAP:
                    strcpy(tempStr3, "RAIDs Swapped");
                    break;

                case CMCC_USRSPND:
                    sprintf(tempStr3, "User-paused (%dpct)",
                            dataPtr->copyComplete.percent);
                    break;

                case CMCC_USRRSM:
                    sprintf(tempStr3, "User Resume (%dpct)",
                            dataPtr->copyComplete.percent);
                    break;

                case CMCC_AUTOSPND:
                    sprintf(tempStr3, "Auto-paused (%dpct)",
                            dataPtr->copyComplete.percent);
                    break;

                case CMCC_CPYRSM:
                    sprintf(tempStr3, "Copy Resume (%dpct)",
                            dataPtr->copyComplete.percent);
                    break;

                case CMCC_CMSPND:
                    sprintf(tempStr3, "CM-paused (%dpct)", dataPtr->copyComplete.percent);
                    break;

                case CMCC_RSD_2VL:
                    strcpy(tempStr3, "Swap Denied (2vl)");
                    break;

                case CMCC_RSD_NODV:
                    strcpy(tempStr3, "Swap Denied (noDV)");
                    break;

                case CMCC_RSD_VL2VL:
                    strcpy(tempStr3, "Swap Denied (vl2vl)");
                    break;

                case CMCC_RSD_RSVD:
                    strcpy(tempStr3, "Swap Denied (RSVD)");
                    break;

                case CMCC_AQRDOWNRSHP:
                    strcpy(tempStr3, "Ownership Acquired");
                    break;

                case CMCC_OWNRSHPTERM:
                    strcpy(tempStr3, "Ownership Term");
                    break;

                case CMCC_FORCEOWNRSHP:
                    strcpy(tempStr3, "Ownership Forced");
                    break;

                default:
                    strcpy(tempStr3, "UNKNOWN");
                    break;
            }

            sprintf(messStr, "COPY-%s->%s %s", tempStr1, tempStr2, tempStr3);
            break;

        case LOG_GetCode(LOG_COPY_SYNC):
            VdiskIdToVblockVdiskIdString(dataPtr->copyComplete.srcVid, tempStr1);
            VdiskIdToVblockVdiskIdString(dataPtr->copyComplete.destVid, tempStr2);

            sprintf(messStr, "COPY-%s->%s synchronized", tempStr1, tempStr2);
            break;

        case LOG_GetCode(LOG_HOST_OFFLINE):
            sprintf(messStr, "HOST-port %d OFFLINE", dataPtr->hostOffline.port);
            break;

        case LOG_GetCode(LOG_LOG_TEXT_MESSAGE_INFO):
        case LOG_GetCode(LOG_LOG_TEXT_MESSAGE_WARNING):
        case LOG_GetCode(LOG_LOG_TEXT_MESSAGE_ERROR):
        case LOG_GetCode(LOG_LOG_TEXT_MESSAGE_DEBUG):
            sprintf(messStr, "%s", dataPtr->textMessageOp.text);
            break;

        case LOG_GetCode(LOG_XSSA_LOG_MESSAGE):
            sprintf(messStr, "%s", dataPtr->xssaMsg.message);
            break;

        case LOG_GetCode(LOG_WORKSET_CHANGED):
            sprintf(messStr, "Workset %d changed", dataPtr->worksetChangedOp.id);
            break;

        case LOG_GetCode(LOG_CACHE_FLUSH_RECOVER):
            VdiskIdToVblockVdiskIdString(dataPtr->cacheFlushRecover.vid, tempStr1);
            sprintf(messStr, "CACHE-VID %s flush recovered", tempStr1);
            break;

        case LOG_GetCode(LOG_FIRMWARE_UPDATE):
            sprintf(messStr, "FIRMWARE-%s update OK",
                    TargetName(dataPtr->firmwareUpdate.targetId));
            break;

        case LOG_GetCode(LOG_SERVER_CREATE_OP):
            ServerIdToNameString(dataPtr->serverCreateOp.sid, tempStr1);
            StatusErrorToString(dataPtr->serverCreateOp.status,
                                dataPtr->serverCreateOp.errorCode, errStr);
            sprintf(messStr, "SERVER-%s create %s", tempStr1, errStr);
            break;

        case LOG_GetCode(LOG_SERVER_DELETE_OP):
            ServerIdToNameString(dataPtr->serverDeleteOp.id, tempStr1);
            StatusErrorToString(dataPtr->serverDeleteOp.status,
                                dataPtr->serverDeleteOp.errorCode, errStr);
            sprintf(messStr, "SERVER-%s delete %s", tempStr1, errStr);
            break;

            /*
             * Change old Bigfoot terminology to new terminology -
             *  Associate == Assign, Disassociate == Unassign
             */
        case LOG_GetCode(LOG_SERVER_ASSOC_OP):
            ServerIdToNameString(dataPtr->serverAssociateOp.sid, tempStr1);
            VdiskIdToVblockVdiskIdString(dataPtr->serverAssociateOp.vid, tempStr2);
            StatusErrorToString(dataPtr->serverAssociateOp.status,
                                dataPtr->serverAssociateOp.errorCode, errStr);
            if (dataPtr->serverAssociateOp.lun == 0xFF)
            {
                if (dataPtr->serverAssociateOp.status == GOOD)
                {
                    sprintf(messStr, "%s:VDISK %s masked ON", tempStr1, tempStr2);
                }
                else
                {
                    sprintf(messStr, "%s:VDISK %s mask %s", tempStr1, tempStr2, errStr);
                }
            }
            else
            {
                sprintf(messStr, "%s:VDISK %s lun %d %s",
                        tempStr1, tempStr2, dataPtr->serverAssociateOp.lun, errStr);
            }
            break;

        case LOG_GetCode(LOG_SERVER_DISASSOC_OP):
            ServerIdToNameString(dataPtr->serverDisassociateOp.sid, tempStr1);
            VdiskIdToVblockVdiskIdString(dataPtr->serverDisassociateOp.vid, tempStr2);
            StatusErrorToString(dataPtr->serverDisassociateOp.status,
                                dataPtr->serverDisassociateOp.errorCode, errStr);
            if (dataPtr->serverAssociateOp.status == GOOD)
            {
                sprintf(messStr, "%s:VDISK %s masked OFF", tempStr1, tempStr2);
            }
            else
            {
                sprintf(messStr, "%s:VDISK %s unmask %s", tempStr1, tempStr2, errStr);
            }
            break;

        case LOG_GetCode(LOG_VDISK_CREATE_OP):
            VdiskIdToVblockVdiskIdString(dataPtr->vdiskCreateOp.vid, tempStr1);
            StatusErrorToString(dataPtr->vdiskCreateOp.status,
                                dataPtr->vdiskCreateOp.errorCode, errStr);
            if ((dataPtr->vdiskCreateOp.status == PI_GOOD) &&
                (dataPtr->vdiskCreateOp.crossLocation != 0))
            {
                if (dataPtr->vdiskCreateOp.crossLocation == 1)
                {
                    sprintf(messStr, "VDISK %s create (Multi-locs) %s", tempStr1, errStr);
                }
                else
                {
                    sprintf(messStr, "VDISK %s create (Single-loc) %s", tempStr1, errStr);
                }
            }
            else
            {
                /*
                 * This is non-GeoRaid vdisk (OK and error case )or Geo-Raid vdisk creation
                 * failure case.
                 */
                sprintf(messStr, "VDISK %s create %s", tempStr1, errStr);
            }
            break;

        case LOG_GetCode(LOG_VDISK_DELETE_OP):
            VdiskIdToVblockVdiskIdString(dataPtr->vdiskDeleteOp.id, tempStr1);
            StatusErrorToString(dataPtr->vdiskDeleteOp.status,
                                dataPtr->vdiskDeleteOp.errorCode, errStr);
            sprintf(messStr, "VDISK %s delete %s", tempStr1, errStr);
            break;

        case LOG_GetCode(LOG_VDISK_PR_CLR):
            VdiskIdToVblockVdiskIdString(dataPtr->clrPR.id, tempStr1);
            StatusErrorToString(dataPtr->clrPR.status, dataPtr->clrPR.errorCode, errStr);
            sprintf(messStr, "VDISK %s PR Clear %s", tempStr1, errStr);
            break;

        case LOG_GetCode(LOG_VDISK_EXPAND_OP):
            VdiskIdToVblockVdiskIdString(dataPtr->vdiskExpandOp.vid, tempStr1);
            StatusErrorToString(dataPtr->vdiskExpandOp.status,
                                dataPtr->vdiskExpandOp.errorCode, errStr);
            sprintf(messStr, "VDISK %s expand %s", tempStr1, errStr);
            break;

        case LOG_GetCode(LOG_VDISK_PREPARE_OP):
            StatusErrorToString(dataPtr->vdiskPrepareOp.status,
                                dataPtr->vdiskPrepareOp.errorCode, errStr);
            sprintf(messStr, "FreeSpace Calculation %s", errStr);
            break;

        case LOG_GetCode(LOG_VDISK_CONTROL_OP):
            {
                VdiskIdToVblockVdiskIdString(dataPtr->vdiskControlOp.srcVid, tempStr3);
                VdiskIdToVblockVdiskIdString(dataPtr->vdiskControlOp.destVid, tempStr4);
                if (dataPtr->vdiskControlOp.srcVid & 0x8000)
                {
                    strcpy(tempStr3, "Prep:");
                    VdiskIdToVblockVdiskIdString(dataPtr->vdiskControlOp.srcVid & 0xfff,
                                                 errStr);
                    strcat(tempStr3, errStr);
                }
                else if ((dataPtr->vdiskControlOp.srcVid & 0x4200) == 0x4200)
                {
                    strcpy(tempStr3, "Batch Cancel");
                }
                else if ((dataPtr->vdiskControlOp.srcVid & 0x4100) == 0x4100)
                {
                    strcpy(tempStr3, "Batch Execute");
                }
                else if ((dataPtr->vdiskControlOp.srcVid & 0x4000) == 0x4000)
                {
                    strcpy(tempStr3, "Batch Start");
                    sprintf(tempStr4, "%d cmds", dataPtr->vdiskControlOp.srcVid & 0xfff);
                }

                switch (dataPtr->vdiskControlOp.subOpType)
                {
                    case MVCMOVEVD:
                        strcpy(tempStr1, "VDISK-move ");
                        break;
                    case MVCCOPYBRK:
                        strcpy(tempStr1, "VDISK-copy ");
                        break;
                    case MVCCOPYSWAP:
                        strcpy(tempStr1, "VDISK-swap ");
                        break;
                    case MVCSLINK:
                        strcpy(tempStr1, "VDISK-SNAPSHOT ");
                        break;
                    case MVCCOPYCONT:
                        switch (dataPtr->vdiskControlOp.mirrorSetType)
                        {
                            case 0:    /* Non-Geo mirror set */
                                strcpy(tempStr1, "MIRROR-");
                                break;

                            case 1:    /* Absolute Geo mirror set */
                                strcpy(tempStr1, "ABSOLUTE GEO MIRROR-");
                                break;
                            case 2:
                                strcpy(tempStr1, "GEO MIRROR-");
                                break;
                            default:
                                strcpy(tempStr1, "MIRROR-");
                                break;
                        }
                        break;
                    case MVCSWAPVD:
                        strcpy(tempStr1, "VDISK-append/delete ");
                        break;
                    case MVCXSPECCOPY:
                        strcpy(tempStr1, "MIRROR-break ");
                        break;
                    case MVCPAUSECOPY:
                        strcpy(tempStr1, "VDISK-pause ");
                        break;
                    case MVCRESUMECOPY:
                        strcpy(tempStr1, "VDISK-resume ");
                        break;
                    case MVCABORTCOPY:
                        strcpy(tempStr1, "VDISK-abort ");
                        break;
                    case MVCXCOPY:
                        strcpy(tempStr1, "VDISK-break all ");
                        break;
                    default:
                        strcpy(tempStr1, "VDISK-unknown ");
                        break;
                }

                StatusErrorToString(dataPtr->vdiskControlOp.status,
                                    dataPtr->vdiskControlOp.errorCode, errStr);

                switch (dataPtr->vdiskControlOp.subOpType)
                {
                    case MVCMOVEVD:
                    case MVCCOPYBRK:
                    case MVCCOPYSWAP:
                    case MVCCOPYCONT:
                    case MVCSLINK:
                    case MVCSWAPVD:
                        sprintf(messStr, "%s%s->%s %s",
                                tempStr1, tempStr3, tempStr4, errStr);
                        break;

                    default:
                        sprintf(messStr, "%s%s %s", tempStr1, tempStr4, errStr);
                        break;
                }
            }
            break;
        case LOG_GetCode(LOG_VDISK_SET_PRIORITY):
            {
                if (dataPtr->setVPri.count == 1)
                {
                    VdiskIdToVblockVdiskIdString(dataPtr->setVPri.vid1, tempStr1);
                    sprintf(messStr, "VDisk %s Priority=%s",
                            tempStr1, priStr[dataPtr->setVPri.pri1]);
                }
                else
                {
                    VdiskIdToVblockVdiskIdString(dataPtr->setVPri.vid1, tempStr1);
                    VdiskIdToVblockVdiskIdString(dataPtr->setVPri.vid2, tempStr2);
                    sprintf(messStr, "VDisk swap Priority %s %s", tempStr1, tempStr2);
                }
            }
            break;
        case LOG_GetCode(LOG_VCG_SET_CACHE_OP):
            {
                StatusErrorToString(dataPtr->vcgSetCacheOp.status,
                                    dataPtr->vcgSetCacheOp.errorCode, errStr);
                if (dataPtr->vcgSetCacheOp.mode & 0x01)
                {
                    strcpy(tempStr1, "ON");
                }
                else
                {
                    strcpy(tempStr1, "OFF");
                }

                sprintf(messStr, "CACHE-global %s %s", tempStr1, errStr);
            }
            break;

        case LOG_GetCode(LOG_VDISK_SET_ATTRIBUTE_OP):
            VdiskIdToVblockVdiskIdString(dataPtr->vdiskSetAttributeOp.vid, tempStr1);
            StatusErrorToString(dataPtr->vdiskSetAttributeOp.status,
                                dataPtr->vdiskSetAttributeOp.errorCode, errStr);

            switch (dataPtr->vdiskSetAttributeOp.mode)
            {
                case PI_VDISK_CHANGE_NORMAL:
                    strcpy(tempStr2, "Normal");
                    break;

                case PI_VDISK_CHANGE_HIDDEN:
                    strcpy(tempStr2, "Hidden");
                    break;

                case PI_VDISK_CHANGE_ASYNCH:
                    strcpy(tempStr2, "Asynch");
                    break;

                case PI_VDISK_CHANGE_PRIVATE:
                    strcpy(tempStr2, "Private");
                    break;

                case PI_VDISK_CHANGE_LOCK:
                case PI_VDISK_CHANGE_VDISK_LOCK:
                    strcpy(tempStr2, "Locked");
                    break;

                case PI_VDISK_CHANGE_UNLOCK:
                case PI_VDISK_CHANGE_VDISK_UNLOCK:
                    strcpy(tempStr2, "Unlocked");
                    break;

                case PI_VDISK_CHANGE_VLINK_FLAG:
                    strcpy(tempStr2, "Vlink Flag");
                    break;

                case PI_VDISK_CHANGE_VLINK_UNFLAG:
                    strcpy(tempStr2, "Vlink Unflag");
                    break;

                case PI_VDISK_CHANGE_VDISK_CACHE_ENABLE:
                    strcpy(tempStr2, "Cache Enabled");
                    break;

                case PI_VDISK_CHANGE_VDISK_CACHE_DISABLE:
                    strcpy(tempStr2, "Cache Disabled");
                    break;

                default:
                    strcpy(tempStr2, "Unknown");
                    break;
            }

            sprintf(messStr, "VDISK %s %s %s", tempStr1, tempStr2, errStr);
            break;

        case LOG_GetCode(LOG_RAID_INIT_OP):
            StatusErrorToString(dataPtr->raidInitOp.status,
                                dataPtr->raidInitOp.errorCode, errStr);
            sprintf(messStr, "RAID-%d init start %s", dataPtr->raidInitOp.id, errStr);
            break;

        case LOG_GetCode(LOG_RAID_CONTROL_OP):
            {
                switch (dataPtr->raidControlOp.scrubcontrol)
                {
                    case SCRUB_POLL:
                        strcpy(tempStr1, "Poll");
                        break;
                    case (SCRUB_CHANGE | SCRUB_ENABLE):
                        strcpy(tempStr1, "Enable");
                        break;
                    case (SCRUB_CHANGE | (SCRUB_ENABLE & ~SCRUB_ENABLE)):
                        strcpy(tempStr1, "Disable");
                        break;
                    default:
                        strcpy(tempStr1, "Unknown");
                        break;
                }

                StatusErrorToString(dataPtr->raidControlOp.status,
                                    dataPtr->raidControlOp.errorCode, errStr);

                sprintf(messStr, "SCRUB-%s %s", tempStr1, errStr);
            }
            break;

        case LOG_GetCode(LOG_DEVICE_DELETE_OP):
            {
                WWNToShortNameString(dataPtr->deviceDeleteOp.wwn, tempStr2, secToWait);

                if (dataPtr->deviceDeleteOp.type == DELETE_DEVICE_DRIVE)
                {
                    strcpy(tempStr1, PD);
                }
                else
                {
                    strcpy(tempStr1, BAY);
                }

                StatusErrorToString(dataPtr->deviceDeleteOp.status,
                                    dataPtr->deviceDeleteOp.errorCode, errStr);

                sprintf(messStr, "%s-%s delete %s", tempStr1, tempStr2, errStr);
            }
            break;

        case LOG_GetCode(LOG_VLINK_BREAK_LOCK_OP):
            VdiskIdToVblockVdiskIdString(dataPtr->vlinkBreakLockOp.id, tempStr2);
            StatusErrorToString(dataPtr->vlinkBreakLockOp.status,
                                dataPtr->vlinkBreakLockOp.errorCode, errStr);
            sprintf(messStr, "VLINK %s deattach %s", tempStr2, errStr);
            break;

        case LOG_GetCode(LOG_PROC_NAME_DEVICE_OP):
            {
                ArrayToString(tempStr3,
                              dataPtr->procNameDevice.name,
                              sizeof(dataPtr->procNameDevice.name));
                StatusErrorToString(dataPtr->procNameDevice.status,
                                    dataPtr->procNameDevice.errorCode, errStr);

                switch (dataPtr->procNameDevice.option)
                {
                    case MNDRETVCG:
                        if (tempStr3[0] == 0x00)
                        {
                            strcpy(tempStr3, "UNKNOWN");
                        }

                        sprintf(messStr, "DSC name ret %s %s", tempStr3, errStr);
                        break;

                    case MNDVCG:
                        sprintf(messStr, "DSC name %s %s", tempStr3, errStr);
                        break;

                    case MNDSERVER:
                        ServerIdToWwnString(dataPtr->procNameDevice.id, tempStr2);
                        if (dataPtr->procNameDevice.status == GOOD)
                        {
                            sprintf(messStr, "NAME-Port %s=%s", tempStr2, tempStr3);
                        }
                        else
                        {
                            sprintf(messStr, "NAME-Port %s %s", tempStr2, errStr);
                        }
                        break;

                    case MNDVDISK:
                        VdiskIdToVblockVdiskIdString
                            (dataPtr->procNameDevice.id, tempStr2);
                        sprintf(messStr, "VD-%s->%s %s", tempStr2, tempStr3, errStr);
                        break;

                    default:
                        strcpy(tempStr2, "UNKNOWN");
                        strcpy(tempStr1, "UNKNOWN");
                        sprintf(messStr, "NAME-UNKNOWN->%s %s", tempStr3, errStr);
                        break;
                }
            }
            break;

        case LOG_GetCode(LOG_PROC_ASSIGN_MIRROR_PARTNER_OP):
            {
                ControllerSnToIndexString(dataPtr->procAssignMirrorOp.newSerialNumber, tempStr1);
                StatusErrorToString(dataPtr->procAssignMirrorOp.status,
                                    dataPtr->procAssignMirrorOp.errorCode, errStr);

                sprintf(messStr, "MIRROR-%s synch %s", tempStr1, errStr);
            }
            break;

        case LOG_GetCode(LOG_PDISK_LABEL_OP):
            {
                StatusErrorToString(dataPtr->pdiskLabelOp.status,
                                    dataPtr->pdiskLabelOp.errorCode, errStr);
                switch (dataPtr->pdiskLabelOp.labtype)
                {
                    case MLDNOLABEL:
                        strcpy(tempStr1, "Unlabel");
                        break;

                    case MLDDATALABEL:
                        strcpy(tempStr1, "Data");
                        break;

                    case MLDSPARELABEL:
                        strcpy(tempStr1, "Hotspare");
                        break;

                    case MLDNDATALABEL:
                        strcpy(tempStr1, "Notsafe");
                        break;

                    case MLDFIXDNAME:
                        strcpy(tempStr1, "Fixup");
                        break;

                    case MLDRELABEL:
                        strcpy(tempStr1, "Relabel");
                        break;

                    default:
                        strcpy(tempStr1, "Unknown");
                        break;
                }

                WWNToShortNameString(dataPtr->pdiskLabelOp.wwn, tempStr2, secToWait);
#if defined(MODEL_7000) || defined(MODEL_4700)
                sprintf(messStr, "LABEL %s-%02d %s %s",
                        tempStr2, dataPtr->pdiskLabelOp.slot, tempStr1, errStr);
#else  /* MODEL_7000 || MODEL_4700 */
                sprintf(messStr, "LABEL-%s %s %s", tempStr2, tempStr1, errStr);
#endif /* MODEL_7000 || MODEL_4700 */
            }
            break;

        case LOG_GetCode(LOG_PDISK_FAIL_OP):
            WWNToShortNameString(dataPtr->pdiskFailOp.wwn, tempStr2, secToWait);
            StatusErrorToString(dataPtr->pdiskFailOp.status,
                                dataPtr->pdiskFailOp.errorCode, errStr);

            if (dataPtr->pdiskFailOp.errorCode == DENOHOTSPARE)
            {
                sprintf(messStr, PD " %s FAIL-System Degraded-%s", tempStr2, errStr);
            }
            else
            {
                sprintf(messStr, PD "-fail %s %s", tempStr2, errStr);
            }
            break;

        case LOG_GetCode(LOG_SET_GEO_LOCATION):
            WWNToShortNameString(dataPtr->psetGeoLocation.wwn, tempStr2, secToWait);
            StatusErrorToString(dataPtr->psetGeoLocation.status,
                                dataPtr->psetGeoLocation.errorCode, errStr);

            if (dataPtr->psetGeoLocation.anyHotSpares == 0)
            {
                sprintf(messStr, "SET-Geo Location %d to %s %s.No HSPARE",
                        dataPtr->psetGeoLocation.locationId, tempStr2, errStr);
            }
            else
            {
                sprintf(messStr, "SET-Geo Location %d to %s %s",
                        dataPtr->psetGeoLocation.locationId, tempStr2, errStr);
            }
            break;

        case LOG_GetCode(LOG_CLEAR_GEO_LOCATION):
            StatusErrorToString(dataPtr->pclearGeoLocation.status,
                                dataPtr->pclearGeoLocation.errorCode, errStr);
            sprintf(messStr, "CLEAR Location Code %s", errStr);

            break;


        case LOG_GetCode(LOG_PDISK_SPINDOWN_OP):
            WWNToShortNameString(dataPtr->pSpindown.wwn, tempStr2, secToWait);
            StatusErrorToString(dataPtr->pSpindown.status,
                                dataPtr->pSpindown.errorCode, errStr);
            sprintf(messStr, PD "-spindown %s %s", tempStr2, errStr);
            break;

        case LOG_GetCode(LOG_PDISK_FAILBACK_OP):
            WWNToShortNameString(dataPtr->pFailback.wwn, tempStr2, secToWait);
            StatusErrorToString(dataPtr->pFailback.status,
                                dataPtr->pFailback.errorCode, errStr);
            sprintf(messStr, PD "-Failback %s %s", tempStr2, errStr);
            break;

/*
        case LOG_GetCode(LOG_PDISK_AUTO_FAILBACK_OP):
            StatusErrorToString(dataPtr->pAutoFailback.status,
                                dataPtr->pAutoFailback.errorCode,
                                errStr);
            sprintf(messStr, "Physical Disk-Auto Failback %s", errStr);
            break;
*/
        case LOG_GetCode(LOG_RB_FAILBACK_REINIT_DRIVE):
            WWNToString(dataPtr->pReinitDrive.dstwwn, tempStr1);
            sprintf(messStr, "%s Reinitialized", tempStr1);
            break;

        case LOG_GetCode(LOG_RB_FAILBACK_CTLR_MISMATCH):
            WWNToString(dataPtr->pFailBackinfo.hswwn, tempStr1);
            WWNToString(dataPtr->pFailBackinfo.dstwwn, tempStr2);
            sprintf(messStr, "%s MISMATCH %s", tempStr2, tempStr1);
            break;

        case LOG_GetCode(LOG_AUTO_FAILBACK):
            WWNToString(dataPtr->pFailBackinfo.hswwn, tempStr1);
            WWNToString(dataPtr->pFailBackinfo.dstwwn, tempStr2);
            WWNToShortNameString(dataPtr->pFailBackinfo.dstwwn, tempStr3, secToWait);

            if (dataPtr->pFailBackinfo.status == DEOK)
            {
                sprintf(messStr, "FAILBACK %s To %s DONE", tempStr1, tempStr2);
            }
            else if (dataPtr->pFailBackinfo.status == DESESINPROGRESS)
            {
                sprintf(messStr, "SES in Progress.FailBack Delayed");
            }
            else if (dataPtr->pFailBackinfo.status == DEPIDNOTUSED)
            {
                sprintf(messStr, "PDISK %s No RAIDS exists", tempStr1);
            }
            else if (dataPtr->pFailBackinfo.status == DEPIDUSED)
            {
                sprintf(messStr, "PDISK %s RAIDS exists", tempStr2);
            }
            else if (dataPtr->pFailBackinfo.status == DEINSUFFREDUND)
            {
                sprintf(messStr, "PDISK %s Insuff Redundancy", tempStr2);
            }
            else if (dataPtr->pFailBackinfo.status == DETYPEMISMATCH)
            {
                sprintf(messStr, "FAILBACK FAILED.%s Dev type mismatch", tempStr3);
            }
            break;

        case LOG_GetCode(LOG_AF_ENABLE_DISABLE):

            if (dataPtr->pAFStatusinfo.status == TRUE)
            {
                sprintf(messStr, "AUTOFAILBACK Enabled");
            }
            else
            {
                sprintf(messStr, "AUTOFAILBACK Disabled");
            }
            break;

        case LOG_GetCode(LOG_GLOBAL_CACHE_MODE):

            if (dataPtr->pGlobalCacheMode.status == TRUE)
            {
                sprintf(messStr, "Global Cache Enabled");
            }
            else
            {
                sprintf(messStr, "Global Cache Disabled");
            }
            break;

        case LOG_GetCode(LOG_PDISK_DEFRAG_OP):
            StatusErrorToString(dataPtr->pdiskDefragOp.status,
                                dataPtr->pdiskDefragOp.errorCode, errStr);

            if (dataPtr->pdiskDefragOp.id == PI_PDISK_DEFRAG_ALL)
            {
                sprintf(messStr, PD " ALL defrag start %s", errStr);
            }
            else if (dataPtr->pdiskDefragOp.id == PI_PDISK_DEFRAG_STOP)
            {
                sprintf(messStr, PD " defrag cancel %s", errStr);
            }
            else if (dataPtr->pdiskDefragOp.id == MRDEFRAGMENT_ORPHANS)
            {
                sprintf(messStr, "RAID-orphan destruction %s", errStr);
            }
            else
            {
                WWNToShortNameString(dataPtr->pdiskDefragOp.wwn, tempStr2, secToWait);
                StatusErrorToString(dataPtr->pdiskDefragOp.status,
                                    dataPtr->pdiskDefragOp.errorCode, errStr);
                sprintf(messStr, PD " %s defrag start %s", tempStr2, errStr);
            }
            break;

        case LOG_GetCode(LOG_PSD_REBUILD_DONE):
            {
                WWNToShortNameString(dataPtr->psdRebuildDone.wwn, tempStr2, secToWait);
                if (dataPtr->psdRebuildDone.errorCode == RB_ECOK)
                {
                    strcpy(tempStr1, "OK");
                }
                else if (BIT_TEST(dataPtr->psdRebuildDone.errorCode, RB_ECBADTYPE) ||
                         BIT_TEST(dataPtr->psdRebuildDone.errorCode, RB_ECDELETED) ||
                         BIT_TEST(dataPtr->psdRebuildDone.errorCode, RB_ECOWNER) ||
                         BIT_TEST(dataPtr->psdRebuildDone.errorCode, RB_ECDONE))
                {
                    strcpy(tempStr1, "ABORTED");
                }
                else if (BIT_TEST(dataPtr->psdRebuildDone.errorCode, RB_ECDELAY))
                {
                    strcpy(tempStr1, "RETRYING");
                }
                else
                {
                    strcpy(tempStr1, "FAIL");
                }

                sprintf(messStr, PD " %s Rebuild %s (Raid#%d)",
                        tempStr2, tempStr1, dataPtr->psdRebuildDone.rid);
            }
            break;

        case LOG_GetCode(LOG_PDD_REBUILD_DONE):
            {
                WWNToShortNameString(dataPtr->pddRebuildDone.wwn, tempStr2, secToWait);
                sprintf(messStr, PD " %s rebuild OK", tempStr2);
            }
            break;

        case LOG_GetCode(LOG_HSPARE_DONE):
            {
                WWNToShortNameString(dataPtr->hspareDone.wwn, tempStr2, secToWait);
                sprintf(messStr, "HOTSPARE %s OK", tempStr2);
            }
            break;

        case LOG_GetCode(LOG_RAID_INIT_DONE):
            {
                if (dataPtr->raidInitDone.status == LOG_RAID_INIT_GOOD)
                {
                    sprintf(messStr, "RAID-%d init OK", dataPtr->raidInitDone.rid);
                }
                else if (dataPtr->raidInitDone.status == LOG_RAID_INIT_FAIL)
                {
                    sprintf(messStr, "RAID-%d init FAIL", dataPtr->raidInitDone.rid);
                }
                else            /* status == LOG_RAID_INIT_TERM */
                {
                    sprintf(messStr, "RAID-%d init halted", dataPtr->raidInitDone.rid);
                }
                break;
            }

        case LOG_GetCode(LOG_LOOPUP):
            sprintf(messStr, "%s-port %d ONLINE",
                    (dataPtr->loopUp.proc) ? "STORAGE" : "HOST", dataPtr->loopUp.port);
            break;

        case LOG_GetCode(LOG_DRIVE_BAY_FW_UPDATE):
            WWNToShortNameString(dataPtr->driveBayFWUpdate.wwn, tempStr1, secToWait);
            sprintf(messStr, BAY "-%s, fwu OK", tempStr1);
            break;

        case LOG_GetCode(LOG_CONFIGURATION_SESSION_START):
        case LOG_GetCode(LOG_CONFIGURATION_SESSION_START_DEBUG):
            InetToAscii(dataPtr->configSessionStartStop.IPAddress, tempStr1);
            sprintf(messStr, "SESSION-connect %s", tempStr1);
            break;

        case LOG_GetCode(LOG_CONFIGURATION_SESSION_END):
        case LOG_GetCode(LOG_CONFIGURATION_SESSION_END_DEBUG):
            InetToAscii(dataPtr->configSessionStartStop.IPAddress, tempStr1);
            sprintf(messStr, "SESSION-disconnect %s", tempStr1);
            break;


        case LOG_GetCode(LOG_TARGET_SET_PROPERTIES_OP):
            StatusErrorToString(dataPtr->targetSetPropOp.status,
                                dataPtr->targetSetPropOp.errorCode, errStr);
            sprintf(messStr, "TARGET-%d set prop %s",
                    dataPtr->targetSetPropOp.tid, errStr);
            break;

        case LOG_GetCode(LOG_SES_FAN_ON):
            WWNToShortNameString(dataPtr->sesWWNSlot.wwn, tempStr1, secToWait);
            DiskbayFanIdToString((dataPtr->sesWWNSlot.slot - 1), tempStr2);

            if (dataPtr->sesWWNSlot.direction)
            {
                sprintf(messStr, BAY " %s Fan %s ON", tempStr1, tempStr2);
            }
            break;

        case LOG_GetCode(LOG_PSD_REBUILD_START):
            WWNToShortNameString(dataPtr->psdRebuildStart.wwn, tempStr1, secToWait);
            sprintf(messStr, "RAID-%d %s rebuild started",
                    dataPtr->psdRebuildStart.rid, tempStr1);
            break;

        case LOG_GetCode(LOG_EST_VLINK):
            VdiskIdToVblockVdiskIdString(dataPtr->vlinkOperation.vlSourceVL, tempStr1);
            VdiskIdToVblockVdiskIdString(dataPtr->vlinkOperation.vlDestVD, tempStr2);
            sprintf(messStr, "VLINK-Est. SN%d (%s->%s)\n",
                    dataPtr->vlinkOperation.vlSourceSN, tempStr1, tempStr2);
            break;

        case LOG_GetCode(LOG_TERM_VLINK):
            VdiskIdToVblockVdiskIdString(dataPtr->vlinkOperation.vlSourceVL, tempStr1);
            VdiskIdToVblockVdiskIdString(dataPtr->vlinkOperation.vlDestVD, tempStr2);
            sprintf(messStr, "VLINK-Term. SN%d (%s->%s)\n",
                    dataPtr->vlinkOperation.vlSourceSN, tempStr1, tempStr2);
            break;

        case LOG_GetCode(LOG_SWP_VLINK):
            VdiskIdToVblockVdiskIdString(dataPtr->vlinkOperation.vlSourceVL, tempStr1);
            VdiskIdToVblockVdiskIdString(dataPtr->vlinkOperation.vlDestVD, tempStr2);
            sprintf(messStr, "VLINK-Swap. SN%d (%s->%s)\n",
                    dataPtr->vlinkOperation.vlSourceSN, tempStr1, tempStr2);
            break;

        case LOG_GetCode(LOG_VLINK_SIZE_CHANGED):
            VdiskIdToVblockVdiskIdString(dataPtr->vlinkOperation.vlSourceVL, tempStr1);
            VdiskIdToVblockVdiskIdString(dataPtr->vlinkOperation.vlDestVD, tempStr2);
            sprintf(messStr, "VLINK-Chg Sz. SN%d (%s->%s)\n",
                    dataPtr->vlinkOperation.vlSourceSN, tempStr1, tempStr2);
            break;

        case LOG_GetCode(LOG_NEW_PATH):
            if (dataPtr->newPath.controllerSN < 10000)
            {
                sprintf(tempStr1, "SN%d", dataPtr->newPath.controllerSN);
                sprintf(tempStr2, "HAB%d", dataPtr->newPath.opath);
            }
            else
            {
                sprintf(tempStr1, "SN%d",
                        Qm_VCGIDFromSerial(dataPtr->newPath.controllerSN));
                sprintf(tempStr2, "CN%d/HBA%d",
                        Qm_SlotFromSerial(dataPtr->newPath.controllerSN),
                        dataPtr->newPath.opath);
            }

            if (dataPtr->newPath.iclPathFlag)
            {
                sprintf(messStr, "H-PathICL HBA%d->%s(%s)",
                        dataPtr->newPath.tpath, tempStr1, tempStr2);
            }
            else
            {
                sprintf(messStr, "HOST-Path Made HBA%d->%s(%s)",
                        dataPtr->newPath.tpath, tempStr1, tempStr2);
            }
            break;

        case LOG_GetCode(LOG_DEFRAG_DONE):
            /*
             * Get the short name for the physical disk.
             */
            WWNToShortNameString(dataPtr->defragDone.wwnPDisk, tempStr2, secToWait);

            if (dataPtr->defragDone.rid == 0xFFFF)
            {
                sprintf(messStr, PD " %s defrag done", tempStr2);
            }
            else
            {
                if (dataPtr->defragDone.errCode == RB_ECOK)
                {
                    strcpy(tempStr1, "OK");
                }
                else if (BIT_TEST(dataPtr->defragDone.errCode, RB_ECBADTYPE) ||
                         BIT_TEST(dataPtr->defragDone.errCode, RB_ECDELETED) ||
                         BIT_TEST(dataPtr->defragDone.errCode, RB_ECOWNER) ||
                         BIT_TEST(dataPtr->defragDone.errCode, RB_ECDONE))
                {
                    strcpy(tempStr1, "ABORTED");
                }
                else if (BIT_TEST(dataPtr->defragDone.errCode, RB_ECDELAY))
                {
                    strcpy(tempStr1, "RETRYING");
                }
                else
                {
                    strcpy(tempStr1, "FAIL");
                }

                sprintf(messStr, "RAID-%d %s defrag %s",
                        dataPtr->defragDone.rid, tempStr2, tempStr1);
            }
            break;

        case LOG_GetCode(LOG_VCG_INACTIVATE_CONTROLLER_OP):
            ControllerSnToIndexString(dataPtr->vcgInactivateControllerOp.serialNumber,
                                      tempStr1);
            StatusErrorToString(dataPtr->vcgInactivateControllerOp.status,
                                dataPtr->vcgInactivateControllerOp.errorCode, errStr);
            sprintf(messStr, "DSC-Inactivate %s %s", tempStr1, errStr);
            break;

        case LOG_GetCode(LOG_VCG_ACTIVATE_CONTROLLER_OP):
            ControllerSnToIndexString(dataPtr->vcgActivateControllerOp.serialNumber,
                                      tempStr1);
            StatusErrorToString(dataPtr->vcgActivateControllerOp.status,
                                dataPtr->vcgActivateControllerOp.errorCode, errStr);
            sprintf(messStr, "DSC-Activate %s %s", tempStr1, errStr);
            break;

        case LOG_GetCode(LOG_VLINK_CREATE_OP):
            VdiskIdToVblockVdiskIdString(dataPtr->vlinkCreateOp.vid, tempStr2);
            StatusErrorToString(dataPtr->vlinkCreateOp.status,
                                dataPtr->vlinkCreateOp.errorCode, errStr);
            sprintf(messStr, "VLINK %s create %s", tempStr2, errStr);
            break;

        case LOG_GetCode(LOG_SERVER_SET_PROPERTIES_OP):
            ServerIdToWwnString(dataPtr->serverSetPropOp.sid, tempStr1);
            StatusErrorToString(dataPtr->serverSetPropOp.status,
                                dataPtr->serverSetPropOp.errorCode, errStr);
            if (dataPtr->serverSetPropOp.attr & MRSERVERPROP_SELECT_TARGET)
            {
                sprintf(messStr, "ADD-NewPort %s %s", tempStr1, errStr);
            }
            else
            {
                sprintf(messStr, "DEL-WorkPort %s %s", tempStr1, errStr);
            }
            break;

        case LOG_GetCode(LOG_WRITE_FLUSH_COMPLETE):
            if (dataPtr->cacheFlushComplete.vid != 0xFFFFFFFF)
            {
                VdiskIdToVblockVdiskIdString(dataPtr->cacheFlushComplete.vid, tempStr1);
                sprintf(messStr, "CACHE-VID %s flush OK", tempStr1);
            }
            else
            {
                strcpy(messStr, "CACHE-global flush OK");
            }
            break;

        case LOG_GetCode(LOG_FS_UPDATE):
            WWNToString(dataPtr->fsUpdate.wwnTo, tempStr1);
            if (dataPtr->fsUpdate.status == GOOD)
            {
                sprintf(messStr, PD " %s--%d fsupdate OK", tempStr1, dataPtr->fsUpdate.pid);
            }
            else
            {
                sprintf(messStr, PD " %s--%d fsupdate FAILED", tempStr1, dataPtr->fsUpdate.pid);
            }
            break;

        case LOG_GetCode(LOG_FS_UPDATE_FAIL):
            WWNToString(dataPtr->fsUpdate.wwnTo, tempStr1);
            sprintf(messStr, PD " %s--%d fsupdate FAILED", tempStr1, dataPtr->fsUpdate.pid);
            break;

        case LOG_GetCode(LOG_IPC_LINK_UP):
            switch (dataPtr->ipcLinkUp.LinkType)
            {
                case IPC_LINK_TYPE_ETHERNET:
                    strcpy(tempStr1, "Ethernet");
                    break;

                case IPC_LINK_TYPE_FIBRE:
                    strcpy(tempStr1, "Fibre");
                    break;

                case IPC_LINK_TYPE_QUORUM:
                    strcpy(tempStr1, "Storage");
                    break;

                case IPC_LINK_TYPE_PCI:
                    strcpy(tempStr1, "PCI");
                    break;

                default:
                    strcpy(tempStr1, "");
                    break;
            }
            ControllerSnToIndexString(dataPtr->ipcLinkUp.DestinationSN, tempStr2);
            sprintf(messStr, "LINK-IPC %s to %s OK", tempStr1, tempStr2);
            break;

        case LOG_GetCode(LOG_IPC_BROADCAST):
            /* This event is not currently logged. */
            break;

        case LOG_GetCode(LOG_INIT_PROC_NVRAM_OP):
            StatusErrorToString(dataPtr->initProcNVRAMOp.status,
                                dataPtr->initProcNVRAMOp.errorCode, errStr);
            sprintf(messStr, "NVRAM(PROC): init %s", errStr);
            break;

        case LOG_GetCode(LOG_SET_SYSTEM_SERIAL_NUM_OP):
            StatusErrorToString(dataPtr->setSystemSerialNumberOp.status,
                                dataPtr->setSystemSerialNumberOp.errorCode, errStr);
            sprintf(messStr, "SYSTEM-serial %d set %s",
                    dataPtr->setSystemSerialNumberOp.serialNumber, errStr);
            break;

        case LOG_GetCode(LOG_VCG_PREPARE_SLAVE_OP):
            StatusErrorToString(dataPtr->vcgPrepareSlaveOp.status,
                                dataPtr->vcgPrepareSlaveOp.errorCode, errStr);
            sprintf(messStr, "DSC-Prepare slave %d %s",
                    dataPtr->vcgPrepareSlaveOp.vcgID, errStr);
            break;

        case LOG_GetCode(LOG_VCG_ADD_SLAVE_OP):
            StatusErrorToString(dataPtr->vcgAddSlaveOp.status,
                                dataPtr->vcgAddSlaveOp.errorCode, errStr);
            InetToAscii(dataPtr->vcgAddSlaveOp.ipAddress, tempStr2);
            sprintf(messStr, "DSC-Add slave IP %s %s", tempStr2, errStr);
            break;


        case LOG_GetCode(LOG_VCG_START_IO_OP):
        case LOG_GetCode(LOG_VCG_STOP_IO_OP):
        case LOG_GetCode(LOG_VCG_GLOBAL_CACHE_OP):
            /* These commands are not implemented yet */
            break;

        case LOG_GetCode(LOG_VCG_APPLY_LICENSE_OP):
            StatusErrorToString(dataPtr->vcgApplyLicense.status,
                                dataPtr->vcgApplyLicense.errorCode, errStr);
            sprintf(messStr, "DSC-%d License applied %s",
                    dataPtr->vcgApplyLicense.vcgID, errStr);
            break;

        case LOG_GetCode(LOG_VCG_UNFAIL_CONTROLLER_OP):
            StatusErrorToString(dataPtr->vcgUnfailControllerOp.status,
                                dataPtr->vcgUnfailControllerOp.errorCode, errStr);
            ControllerSnToIndexString(dataPtr->vcgUnfailControllerOp.serialNumber,
                                      tempStr1);
            sprintf(messStr, "DSC-Activate %s %s", tempStr1, errStr);
            break;

        case LOG_GetCode(LOG_VCG_FAIL_CONTROLLER_OP):
            StatusErrorToString(dataPtr->vcgFailControllerOp.status,
                                dataPtr->vcgFailControllerOp.errorCode, errStr);
            ControllerSnToIndexString(dataPtr->vcgFailControllerOp.serialNumber,
                                      tempStr1);
            sprintf(messStr, "DSC-Fail %s %s", tempStr1, errStr);
            break;

        case LOG_GetCode(LOG_VCG_REMOVE_CONTROLLER_OP):
            StatusErrorToString(dataPtr->vcgRemoveControllerOp.status,
                                dataPtr->vcgRemoveControllerOp.errorCode, errStr);
            ControllerSnToIndexString(dataPtr->vcgRemoveControllerOp.serialNumber,
                                      tempStr1);
            sprintf(messStr, "DSC-Remove %s %s", tempStr1, errStr);
            break;

        case LOG_GetCode(LOG_VCG_SHUTDOWN_OP):
            StatusErrorToString(dataPtr->statusOnly.status,
                                dataPtr->statusOnly.errorCode, errStr);
            sprintf(messStr, "DSC-Shutdown %s", errStr);
            break;

        case LOG_GetCode(LOG_FE_QLOGIC_RESET_OP):
            StatusErrorToString(dataPtr->feQlogicResetOp.status,
                                dataPtr->feQlogicResetOp.errorCode, errStr);
            sprintf(messStr, "HOST-Channel %d reset %s",
                    dataPtr->feQlogicResetOp.port, errStr);
            break;

        case LOG_GetCode(LOG_BE_QLOGIC_RESET_OP):
            StatusErrorToString(dataPtr->beQlogicResetOp.status,
                                dataPtr->beQlogicResetOp.errorCode, errStr);
            sprintf(messStr, "STORAGE-Channel %d reset %s",
                    dataPtr->beQlogicResetOp.port, errStr);
            break;

        case LOG_GetCode(LOG_PROC_START_IO_OP):
            StatusErrorToString(dataPtr->statusOnly.status,
                                dataPtr->statusOnly.errorCode, errStr);
            sprintf(messStr, "START-I/O %s", errStr);
            break;

        case LOG_GetCode(LOG_PROC_STOP_IO_OP):
            StatusErrorToString(dataPtr->ioStopOp.status,
                                dataPtr->ioStopOp.errorCode, errStr);
            sprintf(messStr, "STOP-I/O %s", errStr);
            break;

        case LOG_GetCode(LOG_ADMIN_SETTIME_OP):
            StatusErrorToString(dataPtr->sysTime.status,
                                dataPtr->sysTime.errorCode, errStr);
            {
                char        strTime[128];

                LongTimeToString(strTime, (time_t)dataPtr->sysTime.time, true);
                sprintf(messStr, "TIME-set %s GMT %s", strTime, errStr);
            }

            break;

        case LOG_GetCode(LOG_SNAPSHOT_RESTORED):
            sprintf(messStr, "SYSTEM-backup config #%u restored",
                    dataPtr->snapshotRestored.number);
            break;

        case LOG_GetCode(LOG_ADMIN_SET_IP_OP):
            StatusErrorToString(dataPtr->ipAddressChange.status,
                                dataPtr->ipAddressChange.errorCode, errStr);
            sprintf(messStr, "SYSTEM-IP information change %s", errStr);
            break;

        case LOG_GetCode(LOG_SNAPSHOT_TAKEN):
            sprintf(messStr, "SYSTEM-backed up: Entry #%u",
                    dataPtr->snapshotTaken.number);
            break;

        case LOG_GetCode(LOG_HARDWARE_MONITOR_INFO):
        case LOG_GetCode(LOG_HARDWARE_MONITOR_WARN):
        case LOG_GetCode(LOG_HARDWARE_MONITOR_ERROR):
        case LOG_GetCode(LOG_HARDWARE_MONITOR_DEBUG):

        case LOG_GetCode(LOG_CCB_PROCESSOR_HW_INFO):
        case LOG_GetCode(LOG_CCB_PROCESSOR_HW_WARN):
        case LOG_GetCode(LOG_CCB_PROCESSOR_HW_ERROR):

        case LOG_GetCode(LOG_CCB_MEMORY_MODULE_HW_INFO):
        case LOG_GetCode(LOG_CCB_MEMORY_MODULE_HW_WARN):
        case LOG_GetCode(LOG_CCB_MEMORY_MODULE_HW_ERROR):

        case LOG_GetCode(LOG_HARDWARE_MONITOR_STATUS_INFO):
        case LOG_GetCode(LOG_HARDWARE_MONITOR_STATUS_WARN):
        case LOG_GetCode(LOG_HARDWARE_MONITOR_STATUS_ERROR):

        case LOG_GetCode(LOG_CCB_STATUS_INFO):
        case LOG_GetCode(LOG_CCB_STATUS_WARN):
        case LOG_GetCode(LOG_CCB_STATUS_ERROR):

        case LOG_GetCode(LOG_PROC_BOARD_STATUS_INFO):
        case LOG_GetCode(LOG_PROC_BOARD_STATUS_WARN):
        case LOG_GetCode(LOG_PROC_BOARD_STATUS_ERROR):

        case LOG_GetCode(LOG_FE_BUFFER_BOARD_STATUS_INFO):
        case LOG_GetCode(LOG_FE_BUFFER_BOARD_STATUS_WARN):
        case LOG_GetCode(LOG_FE_BUFFER_BOARD_STATUS_ERROR):

        case LOG_GetCode(LOG_BE_BUFFER_BOARD_STATUS_INFO):
        case LOG_GetCode(LOG_BE_BUFFER_BOARD_STATUS_WARN):
        case LOG_GetCode(LOG_BE_BUFFER_BOARD_STATUS_ERROR):

        case LOG_GetCode(LOG_FE_POWER_SUPPLY_STATUS_INFO):
        case LOG_GetCode(LOG_FE_POWER_SUPPLY_STATUS_WARN):
        case LOG_GetCode(LOG_FE_POWER_SUPPLY_STATUS_ERROR):

        case LOG_GetCode(LOG_BE_POWER_SUPPLY_STATUS_INFO):
        case LOG_GetCode(LOG_BE_POWER_SUPPLY_STATUS_WARN):
        case LOG_GetCode(LOG_BE_POWER_SUPPLY_STATUS_ERROR):
            HWM_GetComponentIDString(tempStr1,
                                     (UINT8)((HWM_EVENT_PROPERTIES *)dataPtr)->componentID,
                                     sizeof(tempStr1));

            /*
             * Uppercase the componentID string
             */
            strtoupper(tempStr1);

            /*
             * There should be one entry in this switch for each component.
             */
            switch ((UINT8)((HWM_EVENT_PROPERTIES *)dataPtr)->componentID)
            {

                /****************
                * I2C Hardware
                ***************/
                    /* CCB Hardware */
                case CCB_PROCESSOR_ID:
                case CCB_BOARD_EEPROM_ID:
                case CCB_MEMORY_MODULE_ID:
                case CCB_MEMORY_MODULE_EEPROM_ID:
                    sprintf(messStr, "%s-%s", tempStr1,
                            ((HWM_EVENT_PROPERTIES *)dataPtr)->description);
                    break;

                /****************
                * Statuses
                ***************/
                case CCB_BOARD_EEPROM_STATUS_ID:
                case CCB_MEMORY_MODULE_EEPROM_STATUS_ID:
                case CHASSIS_EEPROM_STATUS_ID:
                case PROC_BOARD_EEPROM_STATUS_ID:
                case FE_POWER_SUPPLY_ASM_EEPROM_STATUS_ID:
                case FE_POWER_SUPPLY_INT_EEPROM_STATUS_ID:
                case BE_POWER_SUPPLY_ASM_EEPROM_STATUS_ID:
                case BE_POWER_SUPPLY_INT_EEPROM_STATUS_ID:
                case FE_BUFFER_BOARD_EEPROM_STATUS_ID:
                case BE_BUFFER_BOARD_EEPROM_STATUS_ID:
                    sprintf(messStr, "%s- Unexpected ID %d", tempStr1,
                            ((HWM_EVENT_PROPERTIES *)dataPtr)->componentID);
                    break;

                case FE_PROCESSOR_TEMPERATURE_STATUS_ID:
                case BE_PROCESSOR_TEMPERATURE_STATUS_ID:
                case FE_BUFFER_BOARD_TEMPERATURE_STATUS_ID:
                case BE_BUFFER_BOARD_TEMPERATURE_STATUS_ID:
                    sprintf(messStr, "%s=%s", tempStr1,
                            ((HWM_EVENT_PROPERTIES *)dataPtr)->description);
                    break;

                case CCB_NVRAM_BATTERY_STATUS_ID:
                case HARDWARE_MONITOR_STATUS_ID:
                case CCB_BOARD_STATUS_ID:
                case PROC_BOARD_STATUS_ID:
                case POWER_SUPPLY_VOLTAGES_STATUS_ID:

                case FE_BUFFER_BOARD_BATTERY_STATUS_ID:
                case FE_BUFFER_BOARD_CHARGER_STATUS_ID:
                case FE_PROCESSOR_STATUS_ID:
                case FE_POWER_SUPPLY_STATUS_ID:
                case FE_BUFFER_BOARD_STATUS_ID:
                case FE_BUFFER_BOARD_FUEL_GAUGE_STATUS_ID:
                case FE_BUFFER_BOARD_MAIN_REGULATOR_STATUS_ID:

                case BE_BUFFER_BOARD_BATTERY_STATUS_ID:
                case BE_BUFFER_BOARD_CHARGER_STATUS_ID:
                case BE_PROCESSOR_STATUS_ID:
                case BE_POWER_SUPPLY_STATUS_ID:
                case BE_BUFFER_BOARD_STATUS_ID:
                case BE_BUFFER_BOARD_FUEL_GAUGE_STATUS_ID:
                case BE_BUFFER_BOARD_MAIN_REGULATOR_STATUS_ID:
                    sprintf(messStr, "%s-%s", tempStr1,
                            ((HWM_EVENT_PROPERTIES *)dataPtr)->description);
                    break;

                case UNKNOWN_ID:
                case HWM_OVERALL_ID:
                case I2C_HARDWARE_OVERALL_ID:
                default:
                    if (LOG_IsInfo(logPtr->eventCode))
                    {
                        sprintf(messStr, "%s-OK", tempStr1);
                    }
                    else if (LOG_IsWarning(logPtr->eventCode))
                    {
                        sprintf(messStr, "%s-ALERT", tempStr1);
                    }
                    else if (LOG_IsError(logPtr->eventCode))
                    {
                        sprintf(messStr, "%s-ERROR", tempStr1);
                    }
                    else
                    {
                        sprintf(messStr, "%s-STATUS", tempStr1);
                    }
                    break;
            }
            break;

        case LOG_GetCode(LOG_BOOT_CODE_EVENT_INFO):
        case LOG_GetCode(LOG_BOOT_CODE_EVENT_WARN):
        case LOG_GetCode(LOG_BOOT_CODE_EVENT_ERROR):
        case LOG_GetCode(LOG_BOOT_CODE_EVENT_DEBUG):
            strncpy(tempStr1, (char *)dataPtr->bootEntry.data, MAX_STANDARD_MESSAGE);
            tempStr1[MAX_STANDARD_MESSAGE - 1] = '\0';
            snprintf(messStr, MAX_STANDARD_MESSAGE, "BOOT-%s Eventcode 0x%x",
                     tempStr1, logPtr->eventCode);
            messStr[MAX_STANDARD_MESSAGE - 1] = '\0';
            break;

            /*
             * ----- Warning Events -----
             */
        case LOG_GetCode(LOG_DEVICE_FAIL_HS):
            WWNToShortNameString(dataPtr->deviceFailHS.wwn, tempStr1, secToWait);
            WWNToShortNameString(dataPtr->deviceFailHS.hswwn, tempStr2, secToWait);
            sprintf(messStr, PD "-%s FAIL hotspare %s", tempStr1, tempStr2);
            break;

        case LOG_GetCode(LOG_SMART_EVENT):
            if (LOG_IsError(logPtr->eventCode))
            {
                WWNToShortNameString(dataPtr->smartEvent.wwn, tempStr1, secToWait);
                sprintf(messStr, "SMART-" PD " %s is failing", tempStr1);
            }
            else if (LOG_IsInfo(logPtr->eventCode))
            {
                switch (dataPtr->smartEvent.sense[15])
                {
                    case 0x82:
                        sprintf(messStr, "SMART: ISE LUN %d-%d RESUME I/O",
                                dataPtr->smartEvent.rsvd1, dataPtr->smartEvent.lun);
                        break;

                    case 0x84:
                        sprintf(messStr, "SMART: ISE LUN %d-%d MRC Path FB",
                                dataPtr->smartEvent.rsvd1, dataPtr->smartEvent.lun);
                        break;

                    default:
                        sprintf(messStr, "SMART: ISE LUN %d-%d Hint#%d",
                                dataPtr->smartEvent.rsvd1, dataPtr->smartEvent.lun,
                                dataPtr->smartEvent.sense[15]);
                }
            }
            else                /* LOG_IsWarning */
            {
                switch (dataPtr->smartEvent.sense[15])
                {
                    case 0x81:
                        sprintf(messStr, "SMART: ISE LUN %d-%d PAUSE I/O",
                                dataPtr->smartEvent.rsvd1, dataPtr->smartEvent.lun);
                        break;

                    case 0x83:
                        sprintf(messStr, "SMART: ISE LUN %d-%d MRC Path FO",
                                dataPtr->smartEvent.rsvd1, dataPtr->smartEvent.lun);
                        break;

                    default:   /* We got a surprise coding change */
                        sprintf(messStr, "SMART: ISE LUN %d-%d Hint#%d",
                                dataPtr->smartEvent.rsvd1, dataPtr->smartEvent.lun,
                                dataPtr->smartEvent.sense[15]);
                }
            }
            break;

        case LOG_GetCode(LOG_VALIDATION):
            sprintf(messStr, "%s", dataPtr->validation.text);
            break;

        case LOG_GetCode(LOG_CACHE_FLUSH_FAILED):
            VdiskIdToVblockVdiskIdString(dataPtr->cacheFlushFailed.vid, tempStr1);
            sprintf(messStr,
                    "CACHE-VID %s flush failed (0x%x)",
                    tempStr1, dataPtr->cacheFlushFailed.errcode);
            break;

        case LOG_GetCode(LOG_FIRMWARE_ALERT):
            sprintf(messStr, "FIRMWARE-Alert " HEX8 "", dataPtr->firmwareAlert.errorCode);
            break;

        case LOG_GetCode(LOG_LOOP_DOWN):
            sprintf(messStr, "%s-port %d down",
                    (dataPtr->loopDown.proc) ? "STORAGE" : "HOST",
                    dataPtr->loopDown.port);
            break;

        case LOG_GetCode(LOG_SES_PS_TEMP_WARN):
            WWNToShortNameString(dataPtr->sesWWNSlot.wwn, tempStr1, secToWait);
            DiskbayPsIdToString((dataPtr->sesWWNSlot.slot - 1), tempStr2);
            if (dataPtr->sesWWNSlot.direction)
            {
                sprintf(messStr, BAY "-%s %s temp WARN", tempStr1, tempStr2);
            }
            else
            {
                sprintf(messStr, BAY "-%s %s temp OK", tempStr1, tempStr2);
            }
            break;

        case LOG_GetCode(LOG_SES_VOLTAGE_HI_WARN):
            WWNToShortNameString(dataPtr->sesVoltage.wwn, tempStr1, secToWait);
            if (dataPtr->sesVoltage.direction)
            {
                sprintf(messStr, BAY "-%s volt HIGH", tempStr1);
            }
            else
            {
                sprintf(messStr, BAY "-%s volt OK", tempStr1);
            }
            break;

        case LOG_GetCode(LOG_SES_VOLTAGE_LO_WARN):
            WWNToShortNameString(dataPtr->sesVoltage.wwn, tempStr1, secToWait);
            if (dataPtr->sesVoltage.direction)
            {
                sprintf(messStr, BAY "-%s volt LOW", tempStr1);
            }
            else
            {
                sprintf(messStr, BAY "-%s volt OK", tempStr1);
            }
            break;

        case LOG_GetCode(LOG_SES_TEMP_WARN):
            WWNToShortNameString(dataPtr->sesTemp.wwn, tempStr1, secToWait);
            if (dataPtr->sesTemp.direction)
            {
                if ((dataPtr->sesTemp.slot == SES_T_AMB_SLOT0) ||
                    (dataPtr->sesTemp.slot == SES_T_AMB_SLOT1))
                {
                    sprintf(messStr, "TEMP-AMBIENT BAY %s=%d dF (WARNING)",
                            tempStr1, C_to_F(dataPtr->sesTemp.temp));
                }
                else
                {
                    sprintf(messStr, "TEMP-INSIDE BAY %s=%d dF (WARNING)",
                            tempStr1, C_to_F(dataPtr->sesTemp.temp));
                }
            }
            else
            {
                if ((dataPtr->sesTemp.slot == SES_T_AMB_SLOT0) ||
                    (dataPtr->sesTemp.slot == SES_T_AMB_SLOT1))
                {
                    sprintf(messStr, "TEMP-AMBIENT BAY %s=%d dF (IS OK NOW)",
                            tempStr1, C_to_F(dataPtr->sesTemp.temp));
                }
                else
                {
                    sprintf(messStr, "TEMP-INSIDE BAY %s=%d dF (IS OK NOW)",
                            tempStr1, C_to_F(dataPtr->sesTemp.temp));
                }
            }
            break;

        case LOG_GetCode(LOG_SES_IO_MOD_PULLED):
            WWNToShortNameString(dataPtr->sesWWNSlot.wwn, tempStr1, secToWait);
            DiskbayIoModIdToString((dataPtr->sesWWNSlot.slot - 1), tempStr2);
            if (dataPtr->sesWWNSlot.direction)
            {
                sprintf(messStr, BAY "-%s I/O mod(%s) removed", tempStr1, tempStr2);
            }
            else
            {
                sprintf(messStr, BAY "-%s I/O mod(%s) restored", tempStr1, tempStr2);
            }
            break;

        case LOG_GetCode(LOG_LOST_PATH):
            if (dataPtr->lostPath.controllerSN < 10000)
            {
                sprintf(tempStr1, "SN%d", dataPtr->lostPath.controllerSN);
                sprintf(tempStr2, "HAB%d", dataPtr->lostPath.opath);
            }
            else
            {
                sprintf(tempStr1, "SN%d",
                        Qm_VCGIDFromSerial(dataPtr->lostPath.controllerSN));
                sprintf(tempStr2, "CN%d/HBA%d",
                        Qm_SlotFromSerial(dataPtr->lostPath.controllerSN),
                        dataPtr->newPath.opath);
            }
            if (dataPtr->lostPath.iclPathFlag)
            {
                sprintf(messStr, "H-PathLost HBA%d->%s(%s)",
                        dataPtr->lostPath.tpath, tempStr1, tempStr2);
            }
            else
            {
                sprintf(messStr, "HOST-Path Lost HBA%d->%s(%s)",
                        dataPtr->lostPath.tpath, tempStr1, tempStr2);
            }
            break;

        case LOG_GetCode(LOG_ISP_CHIP_RESET):
            sprintf(messStr, "%s-reset port %d",
                    (dataPtr->portEvent.proc) ? "STORAGE" : "HOST",
                    dataPtr->portEvent.port);
            break;

        case LOG_GetCode(LOG_SINGLE_PATH):
            WWNToShortNameString(dataPtr->singlePath.wwn, tempStr1, secToWait);
            sprintf(messStr, "PATH-%s single path", tempStr1);
            break;

        case LOG_GetCode(LOG_HOTSPARE_INOP):
            sprintf(messStr, "HOTSPARE-PID %d INOP", dataPtr->hotspareInop.pid);
            break;

        case LOG_GetCode(LOG_HOTSPARE_DEPLETED):

            switch (dataPtr->hotspareDepleted.devType)
            {
                case PD_DT_FC_DISK:
                    strcpy(tempStr1, "enterprise ");
                    break;

                case PD_DT_SATA:
                    strcpy(tempStr1, "low cost ");
                    break;

                case PD_DT_SSD:
                    strcpy(tempStr1, "performance ");
                    break;

                case PD_DT_ECON_ENT:
                    strcpy(tempStr1, "economy ");
                    break;

                default:
                    strcpy(tempStr1, "");
                    break;
            }

            if (dataPtr->hotspareDepleted.type == HSD_CNC)
            {
                sprintf(messStr, "No %shotspares in DSC", tempStr1);
            }
            else if (dataPtr->hotspareDepleted.type == HSD_GEO)
            {
                sprintf(messStr, "No %shotspares in Geo-Pool", tempStr1);
            }
            else if (dataPtr->hotspareDepleted.type == HSD_TOO_SMALL)
            {
                sprintf(messStr, "%shotspares are too small", tempStr1);
            }
            else if (dataPtr->hotspareDepleted.type == HSD_CNC_OK)
            {
                sprintf(messStr, "DSC %shotspares: are OK", tempStr1);
            }
            else if (dataPtr->hotspareDepleted.type == HSD_GEO_OK)
            {
                sprintf(messStr, "Geo-Pool %shotspares: are OK", tempStr1);
            }
            else if (dataPtr->hotspareDepleted.type == HSD_JUST_RIGHT)
            {
                sprintf(messStr, "Too small %shotspares: are OK", tempStr1);
            }
            else
            {
                sprintf(messStr, "No %shotspares", tempStr1);
            }

            /*
             * Fold the first character to an upper case.
             */
            if (messStr[0] < 'A')
            {
                messStr[0] = messStr[0] + 'A' - 'a';
            }

            break;

        case LOG_GetCode(LOG_VCG_SHUTDOWN_WARN):
            StatusErrorToString(ERROR, dataPtr->statusOnly.errorCode, errStr);
            sprintf(messStr, "DSC-Shutdown %s", errStr);
            break;

        case LOG_GetCode(LOG_SES_CURRENT_HI_WARN):
            WWNToShortNameString(dataPtr->sesCurrent.wwn, tempStr1, secToWait);
            if (dataPtr->sesCurrent.direction)
            {
                sprintf(messStr, BAY "-%s current HIGH", tempStr1);
            }
            else
            {
                sprintf(messStr, BAY "-%s current OK", tempStr1);
            }
            break;

        case LOG_GetCode(LOG_SES_EL_REPORT):
            WWNToShortNameString(dataPtr->sesWWNSlot.wwn, tempStr1, secToWait);
            DiskbayIdToString((dataPtr->sesWWNSlot.slot - 1), tempStr2);
            if (dataPtr->sesWWNSlot.direction)
            {
                sprintf(messStr, BAY "-%s LSM(%s) activated", tempStr1, tempStr2);
            }
            else
            {
                sprintf(messStr, BAY "-%s LSM(%s) deactivated", tempStr1, tempStr2);
            }
            break;

        case LOG_GetCode(LOG_SES_EL_PRESENT):
            WWNToShortNameString(dataPtr->sesWWNSlot.wwn, tempStr1, secToWait);
            DiskbayIdToString((dataPtr->sesWWNSlot.slot - 1), tempStr2);
            if (dataPtr->sesWWNSlot.direction)
            {
                sprintf(messStr, BAY "-%s LSM(%s) present", tempStr1, tempStr2);
            }
            else
            {
                sprintf(messStr, BAY "-%s LSM(%s) removed", tempStr1, tempStr2);
            }
            break;

            /*
             * ----- Error Events -----
             */
        case LOG_GetCode(LOG_DEVICE_FAIL_NO_HS):
            WWNToShortNameString(dataPtr->deviceFailNoHS.wwn, tempStr1, secToWait);
#if defined(MODEL_7000) || defined(MODEL_4700)
            sprintf(messStr, "ISE %s FAIL-System Degraded", tempStr1);
#else  /* MODEL_7000 || MODEL_4700 */
            sprintf(messStr, "PDISK %s FAIL-System Degraded-NO SPARE", tempStr1);
#endif /* MODEL_7000 || MODEL_4700 */
            break;

        case LOG_GetCode(LOG_DEVICE_MISSING):
            WWNToShortNameString(dataPtr->deviceMissing.wwn, tempStr1, secToWait);
            sprintf(messStr, PD " %s missing", tempStr1);
            break;

        case LOG_GetCode(LOG_SERIAL_WRONG):
            WWNToShortNameString(dataPtr->serialWrong.wwn, tempStr1, secToWait);
            sprintf(messStr, PD " %s SN mismatch", tempStr1);

            break;

        case LOG_GetCode(LOG_NVA_BAD):
            sprintf(messStr, "NVRAM-%s side checksum ERROR",
                    (dataPtr->nvaBad.processor - 1) ? "Storage" : "Host");
            break;

        case LOG_GetCode(LOG_CACHE_RECOVER_FAIL):
            VdiskIdToVblockVdiskIdString(dataPtr->cacheRecoverFail.vid, tempStr1);
            sprintf(messStr, "CACHE-VID %s recovery FAIL", tempStr1);
            break;

        case LOG_GetCode(LOG_COPY_FAILED):
            VdiskIdToVblockVdiskIdString(dataPtr->copyFailed.srcVid, tempStr1);
            VdiskIdToVblockVdiskIdString(dataPtr->copyFailed.destVid, tempStr2);
            sprintf(messStr, "COPY %s->%s FAIL", tempStr1, tempStr2);
            break;

        case LOG_GetCode(LOG_SES_PS_OVER_TEMP):
            WWNToShortNameString(dataPtr->sesWWNSlot.wwn, tempStr1, secToWait);
            DiskbayPsIdToString((dataPtr->sesWWNSlot.slot - 1), tempStr2);
            if (dataPtr->sesWWNSlot.direction)
            {
                sprintf(messStr, BAY "-%s %s temp OVER", tempStr1, tempStr2);
            }
            else
            {
                sprintf(messStr, BAY "-%s %s temp OK", tempStr1, tempStr2);
            }
            break;

        case LOG_GetCode(LOG_SES_PS_AC_FAIL):
            WWNToShortNameString(dataPtr->sesWWNSlot.wwn, tempStr1, secToWait);
            DiskbayPsIdToString((dataPtr->sesWWNSlot.slot - 1), tempStr2);
            if (dataPtr->sesWWNSlot.direction)
            {
                sprintf(messStr, BAY "-%s %s AC FAIL", tempStr1, tempStr2);
            }
            else
            {
                sprintf(messStr, BAY "-%s %s AC restored", tempStr1, tempStr2);
            }
            break;

        case LOG_GetCode(LOG_SES_PS_DC_FAIL):
            WWNToShortNameString(dataPtr->sesWWNSlot.wwn, tempStr1, secToWait);
            DiskbayPsIdToString((dataPtr->sesWWNSlot.slot - 1), tempStr2);
            if (dataPtr->sesWWNSlot.direction)
            {
                sprintf(messStr, BAY "-%s %s DC FAIL", tempStr1, tempStr2);
            }
            else
            {
                sprintf(messStr, BAY "-%s %s DC restored", tempStr1, tempStr2);
            }
            break;

        case LOG_GetCode(LOG_SES_FAN_FAIL):
            WWNToShortNameString(dataPtr->sesWWNSlot.wwn, tempStr1, secToWait);
            DiskbayFanIdToString((dataPtr->sesWWNSlot.slot - 1), tempStr2);

            if (dataPtr->sesWWNSlot.direction)
            {
                sprintf(messStr, BAY "-%s Fan %s FAIL", tempStr1, tempStr2);
            }
            else
            {
                sprintf(messStr, BAY "-%s Fan %s restored", tempStr1, tempStr2);
            }
            break;

        case LOG_GetCode(LOG_SES_TEMP_FAIL):
            WWNToShortNameString(dataPtr->sesTemp.wwn, tempStr1, secToWait);
            if ((dataPtr->sesTemp.slot == SES_T_AMB_SLOT0) ||
                (dataPtr->sesTemp.slot == SES_T_AMB_SLOT1))
            {
                sprintf(messStr, "TEMP-AMBIENT BAY %s=%d dF (ERROR)",
                        tempStr1, C_to_F(dataPtr->sesTemp.temp));
            }
            else
            {
                sprintf(messStr, "TEMP-INSIDE BAY %s=%d dF (ERROR)",
                        tempStr1, C_to_F(dataPtr->sesTemp.temp));
            }
            break;

        case LOG_GetCode(LOG_SES_VOLTAGE_HI):
            WWNToShortNameString(dataPtr->sesVoltage.wwn, tempStr1, secToWait);
            if (dataPtr->sesVoltage.direction)
            {
                sprintf(messStr, BAY "-%s volt HIGH", tempStr1);
            }
            else
            {
                sprintf(messStr, BAY "-%s volt OK", tempStr1);
            }
            break;

        case LOG_GetCode(LOG_SES_VOLTAGE_LO):
            WWNToShortNameString(dataPtr->sesVoltage.wwn, tempStr1, secToWait);
            if (dataPtr->sesVoltage.direction)
            {
                sprintf(messStr, BAY "-%s volt LOW", tempStr1);
            }
            else
            {
                sprintf(messStr, BAY "-%s volt OK", tempStr1);
            }
            break;

        case LOG_GetCode(LOG_ERR_TRAP):
            {
                const char *pr = "Unknown";

                pr = (dataPtr->procErrTrap.processor) ? "Storage" : "Host";
                sprintf(messStr, "SYSTEM-%s processor error trap", pr);
            }
            break;

        case LOG_GetCode(LOG_VID_RECOVERY_FAIL):
            VdiskIdToVblockVdiskIdString(dataPtr->vidRecoveryFail.vid, tempStr1);
            sprintf(messStr, "CACHE-VID %s recovery FAIL", tempStr1);
            break;

        case LOG_GetCode(LOG_SES_FAN_OFF):
            WWNToShortNameString(dataPtr->sesWWNSlot.wwn, tempStr1, secToWait);
            DiskbayFanIdToString((dataPtr->sesWWNSlot.slot - 1), tempStr2);
            if (dataPtr->sesWWNSlot.direction)
            {
                sprintf(messStr, BAY "-%s Fan %s OFF", tempStr1, tempStr2);
            }
            break;

        case LOG_GetCode(LOG_SES_PS_DC_OVERVOLT):
            WWNToShortNameString(dataPtr->sesWWNSlot.wwn, tempStr1, secToWait);
            DiskbayPsIdToString((dataPtr->sesWWNSlot.slot - 1), tempStr2);
            if (dataPtr->sesWWNSlot.direction)
            {
                sprintf(messStr, BAY "-%s %s DC HIGH", tempStr1, tempStr2);
            }
            else
            {
                sprintf(messStr, BAY "-%s %s DC OK", tempStr1, tempStr2);
            }
            break;

        case LOG_GetCode(LOG_SES_PS_DC_UNDERVOLT):
            WWNToShortNameString(dataPtr->sesWWNSlot.wwn, tempStr1, secToWait);
            DiskbayPsIdToString((dataPtr->sesWWNSlot.slot - 1), tempStr2);
            if (dataPtr->sesWWNSlot.direction)
            {
                sprintf(messStr, BAY "-%s %s DC LOW", tempStr1, tempStr2);
            }
            else
            {
                sprintf(messStr, BAY "-%s %s DC OK", tempStr1, tempStr2);
            }
            break;

        case LOG_GetCode(LOG_SES_SBOD_EXT):
            WWNToShortNameString(dataPtr->sesSBODExt.wwn, tempStr1, secToWait);
            DiskbayIoModIdToString((dataPtr->sesSBODExt.slot - 1), tempStr2);
            sprintf(messStr, BAY "-%s SBOD Mod %s Extend Stat (0x%02X)",
                    tempStr1, tempStr2, dataPtr->sesSBODExt.newStat);
            break;

        case LOG_GetCode(LOG_SES_SBOD_STATECODE):
            WWNToShortNameString(dataPtr->sesSBODExt.wwn, tempStr1, secToWait);
            {
                int         port = (dataPtr->sesSBODExt.slot) & 0xff;

                DiskbayIdToString((dataPtr->sesSBODExt.newStat >> 31) & 1, tempStr2);
                DiskbayPortStateToString((dataPtr->sesSBODExt.newStat) & 0xff, tempStr3);

                if (port <= 3)
                {
                    sprintf(messStr, BAY "-%s HPort %-2.2d %s (%s)",
                            tempStr1, port, tempStr2, tempStr3);
                }
                else
                {
                    sprintf(messStr, BAY "-%s DPort %-2.2d %s (%s)",
                            tempStr1, port - 4, tempStr2, tempStr3);
                }
            }
            break;

        case LOG_GetCode(LOG_SES_ELEM_CHANGE):
            WWNToShortNameString(dataPtr->sesElemChange.wwn, tempStr1, secToWait);
            DiskbayElemIdToString(dataPtr->sesElemChange.elemType,
                                  dataPtr->sesElemChange.slot - 1, tempStr2);
            DiskbayStateChangeToString(dataPtr->sesElemChange.oldState,
                                       dataPtr->sesElemChange.newState, tempStr3);
            sprintf(messStr, BAY "-%s %s %s", tempStr1, tempStr2, tempStr3);
            break;

        case LOG_GetCode(LOG_SES_PS_DC_OVERCURR):
            WWNToShortNameString(dataPtr->sesWWNSlot.wwn, tempStr1, secToWait);
            DiskbayPsIdToString((dataPtr->sesWWNSlot.slot - 1), tempStr2);
            if (dataPtr->sesWWNSlot.direction)
            {
                sprintf(messStr, BAY "-%s %s current HIGH", tempStr1, tempStr2);
            }
            else
            {
                sprintf(messStr, BAY "-%s %s current OK", tempStr1, tempStr2);
            }
            break;

        case LOG_GetCode(LOG_SES_PS_OFF):
            WWNToShortNameString(dataPtr->sesWWNSlot.wwn, tempStr1, secToWait);
            DiskbayPsIdToString((dataPtr->sesWWNSlot.slot - 1), tempStr2);
            if (dataPtr->sesWWNSlot.direction)
            {
                sprintf(messStr, BAY "-%s %s OFF", tempStr1, tempStr2);
            }
            else
            {
                sprintf(messStr, BAY "-%s %s ON", tempStr1, tempStr2);
            }
            break;

        case LOG_GetCode(LOG_SES_PS_FAIL):
            WWNToShortNameString(dataPtr->sesWWNSlot.wwn, tempStr1, secToWait);
            DiskbayPsIdToString((dataPtr->sesWWNSlot.slot - 1), tempStr2);
            if (dataPtr->sesWWNSlot.direction)
            {
                sprintf(messStr, BAY "-%s %s FAIL", tempStr1, tempStr2);
            }
            else
            {
                sprintf(messStr, BAY "-%s %s restored", tempStr1, tempStr2);
            }
            break;

        case LOG_GetCode(LOG_FILEIO_ERR):
            sprintf(messStr, "SYSTEM-internal op flag %d-%X",
                    dataPtr->fileioErr.fid, dataPtr->fileioErr.rc);
            break;

        case LOG_GetCode(LOG_RAID_ERROR):
            WWNToShortNameString(dataPtr->raidError.wwnPdisk, tempStr1, secToWait);
            sprintf(messStr, "RAID-%d %s IO ERROR", dataPtr->raidError.rid, tempStr1);
            break;

        case LOG_GetCode(LOG_PORT_INIT_FAILED):
            sprintf(messStr, "%s-port %d init FAIL",
                    (dataPtr->portInit.proc) ? "STORAGE" : "HOST",
                    dataPtr->portInit.port);
            break;

        case LOG_GetCode(LOG_PORT_EVENT):
            sprintf(messStr, "%s-port %d event notify",
                    (dataPtr->portEvent.proc) ? "STORAGE" : "HOST",
                    dataPtr->portEvent.port);
            break;

        case LOG_GetCode(LOG_LOST_ALL_PATHS):
            if (dataPtr->lostPath.controllerSN >= 160000)
            {
                sprintf(tempStr1, "SN%d(CN%d)",
                        Qm_VCGIDFromSerial(dataPtr->lostPath.controllerSN),
                        Qm_SlotFromSerial(dataPtr->lostPath.controllerSN));
            }
            else if (dataPtr->lostPath.controllerSN < 10000)
            {
                sprintf(tempStr1, "SN%d", dataPtr->lostPath.controllerSN);
            }
            else
            {
                sprintf(tempStr1, "SN%d(CN%d)",
                        dataPtr->lostPath.controllerSN,
                        Qm_SlotFromSerial(dataPtr->lostPath.controllerSN));
            }
            sprintf(messStr, "PATH-lost all comm to %s", tempStr1);
            break;

        case LOG_GetCode(LOG_DEVICE_TIMEOUT):
            WWNToShortNameString(dataPtr->deviceTimeout.wwn, tempStr1, secToWait);
            if (GetDevType(dataPtr->deviceTimeout.wwn, secToWait) == DEV_OUT_DISKBAY)
            {
                sprintf(messStr, BAY "-%s timeout", tempStr1);
            }
            else
            {
                sprintf(messStr, PD "-%s timeout", tempStr1);
            }
            break;

        case LOG_GetCode(LOG_SES_EL_FAIL):
            WWNToShortNameString(dataPtr->sesWWNSlot.wwn, tempStr1, secToWait);
            DiskbayIdToString((dataPtr->sesWWNSlot.slot - 1), tempStr2);
            if (dataPtr->sesWWNSlot.direction)
            {
                sprintf(messStr, BAY "-%s LSM(%s) FAIL", tempStr1, tempStr2);
            }
            else
            {
                sprintf(messStr, BAY "-%s LSM(%s) OK", tempStr1, tempStr2);
            }
            break;

        case LOG_GetCode(LOG_SES_LOOPAFAIL):
            WWNToShortNameString(dataPtr->sesWWNSlot.wwn, tempStr1, secToWait);
            DiskbayIdToString((dataPtr->sesWWNSlot.slot - 1), tempStr2);
            if (dataPtr->sesWWNSlot.direction)
            {
                sprintf(messStr, BAY "-%s LSM(%s) LOOP A FAIL", tempStr1, tempStr2);
            }
            else
            {
                sprintf(messStr, BAY "-%s LSM(%s) LOOP A OK", tempStr1, tempStr2);
            }
            break;

        case LOG_GetCode(LOG_SES_LOOPBFAIL):
            WWNToShortNameString(dataPtr->sesWWNSlot.wwn, tempStr1, secToWait);
            DiskbayIdToString((dataPtr->sesWWNSlot.slot - 1), tempStr2);
            if (dataPtr->sesWWNSlot.direction)
            {
                sprintf(messStr, BAY "-%s LSM(%s) LOOP B FAIL", tempStr1, tempStr2);
            }
            else
            {
                sprintf(messStr, BAY "-%s LSM(%s) LOOP B OK", tempStr1, tempStr2);
            }
            break;

        case LOG_GetCode(LOG_SES_CURRENT_HI):
            WWNToShortNameString(dataPtr->sesCurrent.wwn, tempStr1, secToWait);
            if (dataPtr->sesCurrent.direction)
            {
                sprintf(messStr, BAY "-%s Overcurrent", tempStr1);
            }
            else
            {
                sprintf(messStr, BAY "-%s current OK", tempStr1);
            }
            break;

        case LOG_GetCode(LOG_IPC_LINK_DOWN):
            switch (dataPtr->ipcLinkDown.LinkType)
            {
                case IPC_LINK_TYPE_ETHERNET:
                    strcpy(tempStr1, "Ethernet");
                    break;

                case IPC_LINK_TYPE_FIBRE:
                    strcpy(tempStr1, "Fibre");
                    break;

                case IPC_LINK_TYPE_QUORUM:
                    strcpy(tempStr1, "Storage");
                    break;

                case IPC_LINK_TYPE_PCI:
                    strcpy(tempStr1, "PCI");
                    break;

                default:
                    strcpy(tempStr1, "");
                    break;
            }
            ControllerSnToIndexString(dataPtr->ipcLinkDown.DestinationSN, tempStr2);
            sprintf(messStr, "LINK-IPC %s to %s DOWN", tempStr1, tempStr2);
            break;

        case LOG_GetCode(LOG_LOG_FAILURE_EVENT):
        case LOG_GetCode(LOG_LOG_FAILURE_EVENT_INFO):
        case LOG_GetCode(LOG_LOG_FAILURE_EVENT_WARN):
            switch (dataPtr->failureEvent.action)
            {
            case FAILURE_EVENT_ACTION_LOG_ONLY:
                switch (dataPtr->failureEvent.event.Type)
                {
                case IPC_FAILURE_TYPE_COMMUNICATIONS_FAILED:
                    switch (dataPtr->failureEvent.event.FailureData.CommunicationsLinkFailure.LinkType)
                    {
                    case IPC_LINK_TYPE_FIBRE:
                    case IPC_LINK_TYPE_ETHERNET:
                        switch (dataPtr->failureEvent.event.FailureData.CommunicationsLinkFailure.LinkError)
                        {
                        case IPC_LINK_ERROR_HB_LIST:
                            sprintf(messStr, "HOST-fabric configuration ERROR");
                            break;

                        default:
                            sprintf(messStr, "unknown event");
                            break;
                        }
                        break;

                    case IPC_LINK_TYPE_PCI:
                        switch (dataPtr->failureEvent.event.FailureData.CommunicationsLinkFailure.Processor)
                        {
                        case PROCESS_FE:
                            strcpy(tempStr1, "HOST");
                            break;

                        case PROCESS_BE:
                            strcpy(tempStr1, "STORAGE");
                            break;

                        default:
                            strcpy(tempStr1, "HOST/STORAGE");
                            break;
                        }

                        switch (dataPtr->failureEvent.event.FailureData.CommunicationsLinkFailure.LinkError)
                        {
                        case IPC_LINK_ERROR_OK:
                            sprintf(messStr, "%s-Proc Heartbeat is OK now",
                                    tempStr1);
                            break;

                        default:
                            sprintf(messStr, "%s-Proc Heartbeat FAILED",
                                    tempStr1);
                            break;
                        }
                        break;
                    }
                    break;
                }
                break;

            case FAILURE_EVENT_ACTION_FAILED_INTERFACE:
                switch (dataPtr->failureEvent.event.Type)
                {
                    case IPC_FAILURE_TYPE_INTERFACE_FAILED:
                        ControllerSnToIndexString(dataPtr->failureEvent.event.FailureData.InterfaceFailure.ControllerSN,
                                                  tempStr2);
                        sprintf(messStr, "PORT-%d on %s FAIL",
                                dataPtr->failureEvent.event.FailureData.InterfaceFailure.FailedInterfaceID,
                                tempStr2);
                        break;
                    case IPC_FAILURE_TYPE_COMMUNICATIONS_FAILED:
                        switch (dataPtr->failureEvent.event.FailureData.CommunicationsLinkFailure.LinkType)
                        {
                            case IPC_LINK_TYPE_ETHERNET:
                                sprintf(tempStr1, "ETHERNET");
                                break;
                            case IPC_LINK_TYPE_FIBRE:
                                sprintf(tempStr1, "FIBRE");
                                break;
                            case IPC_LINK_TYPE_QUORUM:
                                sprintf(tempStr1, "STORAGE");
                                break;
                            case IPC_LINK_TYPE_PCI:
                                sprintf(tempStr1, "PCI");
                                break;
                            default:
                                sprintf(tempStr1, "Unknown");
                                break;
                        };

                        ControllerSnToIndexString(dataPtr->failureEvent.event.FailureData.CommunicationsLinkFailure.DestinationSN,
                                                  tempStr2);
                        sprintf(messStr, "%s-Commlink to %s FAIL", tempStr1,
                                tempStr2);
                        break;
                    default:
                        sprintf(messStr, "PORT-FAIL");
                        break;
                };
                break;

            case FAILURE_EVENT_ACTION_FAILED_CONTROLLER:
                switch (dataPtr->failureEvent.event.Type)
                {
                    case IPC_FAILURE_TYPE_INTERFACE_FAILED:
                        ControllerSnToIndexString(dataPtr->failureEvent.event.FailureData.InterfaceFailure.ControllerSN,
                                                  tempStr2);
                        sprintf(messStr, "%s-intfc %d FAIL", tempStr2,
                                dataPtr->failureEvent.event.FailureData.InterfaceFailure.FailedInterfaceID);
                        break;
                    case IPC_FAILURE_TYPE_CONTROLLER_FAILED:
                        ControllerSnToIndexString(dataPtr->failureEvent.event.FailureData.ControllerFailure.FailedControllerSN,
                                                  tempStr1);

                        sprintf(messStr, "%s-FAIL", tempStr1);
                        break;
                    case IPC_FAILURE_TYPE_COMMUNICATIONS_FAILED:
                        switch (dataPtr->failureEvent.event.FailureData.CommunicationsLinkFailure.LinkType)
                        {
                            case IPC_LINK_TYPE_ETHERNET:
                                sprintf(tempStr1, "Ethernet");
                                break;
                            case IPC_LINK_TYPE_FIBRE:
                                sprintf(tempStr1, "Fibre");
                                break;
                            case IPC_LINK_TYPE_QUORUM:
                                sprintf(tempStr1, "Storage");
                                break;
                            case IPC_LINK_TYPE_PCI:
                                sprintf(tempStr1, "PCI");
                                break;
                            default:
                                sprintf(tempStr1, "Unknown");
                                break;
                        };

                        ControllerSnToIndexString(dataPtr->failureEvent.event.FailureData.CommunicationsLinkFailure.DestinationSN,
                                                  tempStr2);
                        sprintf(messStr, "%s %s Commlink FAIL", tempStr2, tempStr1);
                        break;

                    default:
                        sprintf(messStr, "CONTROLLER-FAIL");
                        break;
                };
                break;

            case FAILURE_EVENT_ACTION_RECOVERED_INTERFACE:
                ControllerSnToIndexString(dataPtr->failureEvent.event.FailureData.InterfaceFailure.ControllerSN,
                                          tempStr2);
                sprintf(messStr, "PORT-%d on %s OK",
                        dataPtr->failureEvent.event.FailureData.InterfaceFailure.FailedInterfaceID,
                        tempStr2);
                break;

            case FAILURE_EVENT_ACTION_ELECTION:
                sprintf(messStr, "Starting-election");
                break;

            case FAILURE_EVENT_ACTION_FORWARD_TO_MASTER:
                sprintf(messStr, "hardware status to master");
                break;

            case FAILURE_EVENT_ACTION_DONT_HANDLE:
                sprintf(messStr, "received unhandled condition");
                break;

            case FAILURE_EVENT_ACTION_RESUBMIT:
                sprintf(messStr, "resending hardware status notices");
                break;

            case FAILURE_EVENT_ACTION_DISCARDED:
                sprintf(messStr, "disabled, discarding notice");
                break;

            default:
                sprintf(messStr, "unhandled event");
                break;
            }
            break;

        case LOG_GetCode(LOG_SNAPSHOT_RESTORE_FAILED):
            sprintf(messStr, "SYSTEM-backup config #%u restore FAIL",
                    dataPtr->snapshotRestoreFailed.number);
            break;

        case LOG_GetCode(LOG_SCSI_TIMEOUT):
            WWNToShortNameString(dataPtr->SCSITimeout.wwn, tempStr1, secToWait);

            if (GetDevType(dataPtr->SCSITimeout.wwn, secToWait) == DEV_OUT_DISKBAY)
            {
                /*
                 * Check if this event came from the timeout watchdog or
                 * the completion function and log accordingly.
                 */
                if (dataPtr->SCSITimeout.flags == LOG_SCSI_TIMEOUT_FLAGS_TIMEOUT)
                {
                    sprintf(messStr, BAY "-%s tmdout SCSI cmd", tempStr1);
                }
                else
                {
                    sprintf(messStr, BAY "-%s tmocomp SCSI cmd", tempStr1);
                }
            }
            else
            {
                /*
                 * Check if this event came from the timeout watchdog or
                 * the completion function and log accordingly.
                 */
                if (dataPtr->SCSITimeout.flags == LOG_SCSI_TIMEOUT_FLAGS_TIMEOUT)
                {
                    sprintf(messStr, "PDISK-%s tmdout SCSI cmd", tempStr1);
                }
                else
                {
                    sprintf(messStr, "PDISK-%s tmocomp SCSI cmd", tempStr1);
                }
            }
            break;
        case LOG_GetCode(LOG_DRIVE_DELAY):
            WWNToShortNameString(dataPtr->driveDelay.wwn, tempStr1, secToWait);
            sprintf(messStr, "PDisk %s delayed", tempStr1);
            break;

            /*
             * ----- Debug Events -----
             */

        case LOG_GetCode(LOG_CHANGE_NAME):
            VdiskIdToVblockVdiskIdString(dataPtr->vlinkChangeName.vlid, tempStr2);
            sprintf(messStr, "VLINK %s change name request", tempStr2);
            break;

        case LOG_GetCode(LOG_ELECTION_STATE_CHANGE):
            sprintf(messStr, "ELECTION-state change %d to %d",
                    dataPtr->electionStateTransition.fromState,
                    dataPtr->electionStateTransition.toState);
            break;

        case LOG_GetCode(LOG_MAG_DRIVER_ERR):
            WWNToShortNameString(dataPtr->magDriverErr.wwn, tempStr1, secToWait);
            if (dataPtr->magDriverErr.vid != 0xFFFF)
            {
                VdiskIdToVblockVdiskIdString(dataPtr->magDriverErr.vid, tempStr2);
                sprintf(messStr, "MAG-%s VID %s ERROR", tempStr1, tempStr2);
            }
            else
            {
                sprintf(messStr, "MAG-%s ERROR", tempStr1);
            }
            break;

        case LOG_GetCode(LOG_HOST_NONSENSE):
            WWNToShortNameString(dataPtr->hostNonSense.wwn, tempStr1, secToWait);
            VdiskIdToVblockVdiskIdString(dataPtr->hostNonSense.vid, tempStr2);
            if (dataPtr->hostNonSense.vid == 0xFFFF)
            {
                sprintf(messStr, "HOST-%s ERROR", tempStr1);
                break;
            }

            /* If not scsi reservation conflict error code, do normal message. */
            if (dataPtr->hostNonSense.errcode != 0x18)
            {
                sprintf(messStr, "HOST-%s VID %s ERROR", tempStr1, tempStr2);
                break;
            }
            /* scsi reservation conflict follows */
            if (dataPtr->hostNonSense.count == 0)
            {
                /* First time message. */
                sprintf(messStr, "Reservation Conflicts for HOST-%s Port %d to VID %s",
                        tempStr1, dataPtr->hostNonSense.channel, tempStr2);
                break;
            }
            if (dataPtr->hostNonSense.count == 0xFFFFFFFF)
            {
                /* Last time message -- reservation conflicts ceased. */
                sprintf(messStr, "Reservation Conflicts for HOST-%s Port %d to VID %s Ceased",
                        tempStr1, dataPtr->hostNonSense.channel, tempStr2);
                break;
            }

            /* A 10 minute period has the maximum increase. */
            sprintf(messStr, "%d Reservation Conflicts for HOST-%s Port %d to VID %s",
                    dataPtr->hostNonSense.count, tempStr1,
                    dataPtr->hostNonSense.channel, tempStr2);
            break;

        case LOG_GetCode(LOG_HOST_QLOGIC_ERR):
            WWNToShortNameString(dataPtr->hostQlogicErr.wwn, tempStr1, secToWait);
            if (dataPtr->hostQlogicErr.vid != 0xFFFF)
            {
                VdiskIdToVblockVdiskIdString(dataPtr->hostQlogicErr.vid, tempStr2);
                sprintf(messStr, "HBA-%s VID %s ERROR", tempStr1, tempStr2);
            }
            else
            {
                sprintf(messStr, "HBA-%s ERROR", tempStr1);
            }
            break;

        case LOG_GetCode(LOG_RAID_EVENT):
            if (dataPtr->raidEvent.type == erlexeccomp)
            {
                sprintf(messStr, "RAID-error exec complete");
            }
            else
            {
                sprintf(messStr, "RAID-event %u", dataPtr->raidEvent.type);
            }
            break;

        case LOG_GetCode(LOG_SINGLE_BIT_ECC):
            sprintf(messStr, "MEM-%s ECC ERROR",
                    (dataPtr->singleBitECC.proc) ? "Storage side" : "Host side");
            break;

        case LOG_GetCode(LOG_SHORT_SCSI_EVENT):
            WWNToShortNameString(dataPtr->shortSCSIEvent.wwn, tempStr1, secToWait);
            if (GetDevType(dataPtr->shortSCSIEvent.wwn, secToWait) == DEV_OUT_DISKBAY)
            {
                sprintf(messStr, BAY "-%s cmd with bad status", tempStr1);
            }
            else
            {
                sprintf(messStr, PD " %s cmd with bad status", tempStr1);
            }
            break;

        case LOG_GetCode(LOG_LONG_SCSI_EVENT):
            WWNToShortNameString(dataPtr->longSCSIEvent.wwn, tempStr1, secToWait);
            if (GetDevType(dataPtr->longSCSIEvent.wwn, secToWait) == DEV_OUT_DISKBAY)
            {
                sprintf(messStr, BAY "-%s cmd with chk cond status", tempStr1);
            }
            else
            {
                sprintf(messStr, PD " %s cmd with chk cond status", tempStr1);
            }
            break;

        case LOG_GetCode(LOG_SPINUP_FAILED):
            WWNToShortNameString(dataPtr->longSCSIEvent.wwn, tempStr1, secToWait);
            sprintf(messStr, PD " %s spinup failed", tempStr1);
            break;

        case LOG_GetCode(LOG_HOST_IMED_NOTIFY):
            sprintf(messStr, "HOST-immediate notify port %d",
                    dataPtr->hostImedNotify.port);
            break;

        case LOG_GetCode(LOG_HOST_SENSE_DATA):
            WWNToShortNameString(dataPtr->hostSenseData.wwn, tempStr1, secToWait);
            sprintf(messStr, "HOST-sense data %s", tempStr1);
            break;

        case LOG_GetCode(LOG_ZONE_INQUIRY):
            WWNToShortNameString(dataPtr->zoneInquiry.wwn, tempStr1, secToWait);
            sprintf(messStr, "%s detected", tempStr1);
            break;

#if ISCSI_CODE

        case LOG_GetCode(LOG_SERVER_LOGGED_IN_OP):
            WWNToShortNameString(dataPtr->pSrvrLoggedIn.wwn, tempStr1, secToWait);
            sprintf(messStr, "SRV %s - TID %d, WWN " HEX8N "" HEX8N " %s",
                    loginStateStr[dataPtr->pSrvrLoggedIn.loginState],
                    dataPtr->pSrvrLoggedIn.tid,
                    SwapWord((UINT32)dataPtr->pSrvrLoggedIn.wwn),
                    SwapWord((UINT32)(dataPtr->pSrvrLoggedIn.wwn >> 32)),
                    dataPtr->pSrvrLoggedIn.i_name);
            break;

        case LOG_GetCode(LOG_TARGET_UP_OP):
            if (dataPtr->pTargetUp.type == 0)
            {
#if 0
                WWNToShortNameString(dataPtr->pTargetUp.wwn, tempStr1, secToWait);
                sprintf(messStr, "FC TARGET %s- TID %d, WWN %s",
                        targetStateStr[dataPtr->pTargetUp.state],
                        dataPtr->pTargetUp.tid, tempStr1);
#endif /* 0 */
                sprintf(messStr, "FC TARGET %s- TID %d",
                        targetStateStr[dataPtr->pTargetUp.state], dataPtr->pTargetUp.tid);
            }
            else
            {
#if 0
                sprintf(tempStr1, "%d.%d.%d.%d",
                        dataPtr->pTargetUp.ip & 0x000000FF,
                        (dataPtr->pTargetUp.ip & 0x0000FF00) >> 8,
                        (dataPtr->pTargetUp.ip & 0x00FF0000) >> 16,
                        (dataPtr->pTargetUp.ip & 0xFF000000) >> 24);
                sprintf(tempStr2, "%d.%d.%d.%d",
                        dataPtr->pTargetUp.mask & 0x000000FF,
                        (dataPtr->pTargetUp.mask & 0x0000FF00) >> 8,
                        (dataPtr->pTargetUp.mask & 0x00FF0000) >> 16,
                        (dataPtr->pTargetUp.mask & 0xFF000000) >> 24);
                sprintf(tempStr3, "%d.%d.%d.%d",
                        dataPtr->pTargetUp.gateway & 0x000000FF,
                        (dataPtr->pTargetUp.gateway & 0x0000FF00) >> 8,
                        (dataPtr->pTargetUp.gateway & 0x00FF0000) >> 16,
                        (dataPtr->pTargetUp.gateway & 0xFF000000) >> 24);
                sprintf(messStr, "iSCSI Target %s- TID %d, IP %s, Mask %s, Gateway %s",
                        targetStateStr[dataPtr->pTargetUp.state],
                        dataPtr->pTargetUp.tid, tempStr1, tempStr2, tempStr3);
#endif /* 0 */
                sprintf(messStr, "iSCSI TARGET %s- TID %d",
                        targetStateStr[dataPtr->pTargetUp.state], dataPtr->pTargetUp.tid);
            }
            break;

        case LOG_GetCode(LOG_ISCSI_SET_INFO):
            sprintf(messStr, "Set iSCSI Info TID %d, Map 0X%x",
                    dataPtr->piSCSISetInfo.tid, dataPtr->piSCSISetInfo.setmap);
            break;

        case LOG_GetCode(LOG_ISCSI_SET_CHAP):
            sprintf(messStr, "iSCSI CHAP %s User, %d Users",
                    chapOption[dataPtr->piSCSISetChap.option],
                    dataPtr->piSCSISetChap.count);
            break;
        case LOG_GetCode(LOG_ISCSI_GENERIC):
            sprintf(messStr, "iSCSI: %s", dataPtr->piSCSIGeneric.msgStr);
            break;
#endif /* ISCSI_CODE */

        case LOG_GetCode(LOG_FRAMEDROPPED):
            sprintf(messStr, "%s-port %d frame dropped",
                    (dataPtr->portEvent.proc) ? "STORAGE" : "HOST",
                    dataPtr->portEvent.port);
            break;

        case LOG_GetCode(LOG_PORTDBCHANGED):
            sprintf(messStr, "%s-port %d database change",
                    (dataPtr->portDbChanged.proc) ? "STORAGE" : "HOST",
                    dataPtr->portDbChanged.port);
            break;

        case LOG_GetCode(LOG_RSCN):
            sprintf(messStr, "%s-port %d RSCN %02hhx N_Port ID %06x",
                    (dataPtr->rscn.proc) ? "STORAGE" : "HOST",
                    dataPtr->rscn.port,
                    dataPtr->rscn.addressFormat, dataPtr->rscn.portId);
            break;

        case LOG_GetCode(LOG_SOCKET_ERROR):
            sprintf(messStr, "SYSTEM-user socket error %d",
                    dataPtr->socketError.errorCode);
            break;

        case LOG_GetCode(LOG_HBEAT_STOP):
            sprintf(messStr, "SYSTEM %s miss CCB heartbeat",
                    (dataPtr->heartbeatStop.proc) ? "Storage side" : "Host side");
            break;

        case LOG_GetCode(LOG_PARITY_SCAN_REQUIRED):
            sprintf(messStr, "RAID-%d parity scan required", dataPtr->parityScanReq.rid);
            break;

        case LOG_GetCode(LOG_PDISK_UNFAIL_OP):
            StatusErrorToString(dataPtr->pdiskUnFailOp.status,
                                dataPtr->pdiskUnFailOp.errorCode, errStr);
            WWNToShortNameString(dataPtr->pdiskUnFailOp.wwn, tempStr2, secToWait);
            sprintf(messStr, PD " %s unfail %s", tempStr2, errStr);
            break;

        case LOG_GetCode(LOG_MB_FAILED):
            sprintf(messStr, "%s-port %d Mailbox Command %2hx RC=%04hx",
                    (dataPtr->mbxfailed.proc) ? "STORAGE" : "HOST",
                    dataPtr->mbxfailed.port,
                    dataPtr->mbxfailed.imbr[0], dataPtr->mbxfailed.ombr[0]);
            break;

        case LOG_GetCode(LOG_LIP):
            sprintf(messStr, "%s-port %d LIP Event",
                    (dataPtr->portEvent.proc) ? "STORAGE" : "HOST",
                    dataPtr->portEvent.port);
            break;

        case LOG_GetCode(LOG_IOCB):
            sprintf(messStr, "DEBUG-%s IOCB on port %d for reason " HEX8 "",
                    (dataPtr->portEvent.proc) ? "STORAGE" : "HOST",
                    dataPtr->portEvent.port, dataPtr->portEvent.reason);
            break;

        case LOG_GetCode(LOG_IOCBTO):
            sprintf(messStr, "%s-IOCB Timeout port %d",
                    (dataPtr->iocbTo.proc) ? "STORAGE" : "HOST", dataPtr->iocbTo.port);
            break;

        case LOG_GetCode(LOG_TASKTOOLONG):
            sprintf(messStr, "SYSTEM-Task took too long %dmsec, ",
                    (dataPtr->taskTooLong.time * 128));
            break;

        case LOG_GetCode(LOG_VLINK_NAME_CHANGED):
            VdiskIdToVblockVdiskIdString(dataPtr->vlinkNameChangedOp.id, tempStr2);
            StatusErrorToString(dataPtr->vlinkNameChangedOp.status,
                                dataPtr->vlinkNameChangedOp.errorCode, errStr);
            sprintf(messStr, "VLINK %s name change %s", tempStr2, errStr);
            break;

        case LOG_GetCode(LOG_FILEIO_DEBUG):
            sprintf(messStr, "SYSTEM-file i/o %s FID %d",
                    dataPtr->fileioErr.api == 1 ? "Write" : "Read",
                    dataPtr->fileioErr.fid);
            break;

        case LOG_GetCode(LOG_PHY_RETRY):
            WWNToShortNameString(dataPtr->longSCSIEvent.wwn, tempStr1, secToWait);
            sprintf(messStr, "SCSI-retry for %s", tempStr1);
            break;

        case LOG_GetCode(LOG_PORT_UP):
            sprintf(messStr, "%s-port %u init OK",
                    (dataPtr->portUp.proc) ? "STORAGE" : "HOST", dataPtr->portUp.port);
            break;

        case LOG_GetCode(LOG_PHY_ACTION):
            switch (dataPtr->phyAction.action)
            {
                case 0:
                    strcpy(tempStr1, "No Action");
                    break;

                case 1:
                    strcpy(tempStr1, "Login");
                    break;

                case 2:
                    strcpy(tempStr1, "Target Reset");
                    break;

                default:
                    strcpy(tempStr1, "LIP Reset");
            }
            sprintf(messStr, "%s-port %u action taken - %s",
                    (dataPtr->portEvent.proc) ? "STORAGE" : "HOST",
                    dataPtr->portEvent.port, tempStr1);
            break;

        case LOG_GetCode(LOG_LPDN_RETRY):
            sprintf(messStr, "%s-port %d is being reset due to offline",
                    (dataPtr->loopDown.proc) ? "STORAGE" : "HOST",
                    dataPtr->loopDown.port);
            break;

        case LOG_GetCode(LOG_VALIDATION_DEBUG):
            /*
             * I just used the socketError data struct so I didn't have to
             * change logdef.h.
             * The real method by which we report Validation errors is TBD.
             */
            sprintf(messStr, "Validation error 0x%08X", dataPtr->socketError.errorCode);
            break;

        case LOG_GetCode(LOG_DIAG_RESULT):
            if (dataPtr->diagResult.result == GOOD)
            {
                strcpy(messStr, "Diagnostic Test Record - PASS");
            }
            else
            {
                strcpy(messStr, "Diagnostic Test Record - FAIL");
            }
            break;

        case LOG_GetCode(LOG_PROC_MEMORY_HEALTH_ALERT):
            sprintf(messStr, "PROC%s MEM-Correctable ECC rate >%d/hr",
                    (dataPtr->memoryHealth.proc) ? "A" : "B", 60);
            break;

        case LOG_GetCode(LOG_PCI_CFG_ERR):
            sprintf(messStr, "PCI-%s port %d set configuration error",
                    (dataPtr->loopDown.proc) ? "Storage" : "Host",
                    dataPtr->loopDown.port);
            break;

        case LOG_GetCode(LOG_LOOP_PRIMITIVE_DEBUG):
            StatusErrorToString(dataPtr->loopPrimitiveOp.status,
                                dataPtr->loopPrimitiveOp.errorCode, errStr);
            sprintf(messStr, "Loop Primitive %s", errStr);
            break;

        case LOG_GetCode(LOG_FCM):
            if (dataPtr->fcalMonitor.eventType == FCM_ET_RESTORED)
            {
                sprintf(messStr, "%s-port %d Restored",
                        dataPtr->fcalMonitor.proc ? "STORAGE" : "HOST",
                        dataPtr->fcalMonitor.port);
            }
            else
            {
                FCM_GetEventTypeString(tempStr1, dataPtr->fcalMonitor.eventType,
                                       sizeof(tempStr1));
                FCM_GetConditionCodeString(tempStr2, dataPtr->fcalMonitor.conditionCode,
                                           sizeof(tempStr2));

                sprintf(messStr, "%s-port %d Degraded (%s %s)",
                        dataPtr->fcalMonitor.proc ? "STORAGE" : "HOST",
                        dataPtr->fcalMonitor.port, tempStr1, tempStr2);
            }
            break;

        case LOG_GetCode(LOG_CSTOP_LOG):
            if (dataPtr->cStopLog.time)
            {
                sprintf(messStr, "C$Stop Recovery Step (%d seconds)",
                        dataPtr->cStopLog.time);
            }
            else
            {
                sprintf(messStr, "C$Stop Recovery Step - NOP");
            }

            break;


        case LOG_GetCode(LOG_WC_FLUSH):
            sprintf(messStr, "WC Flush Taking too Long (%d seconds)",
                    dataPtr->wcFlushLong.time);

            break;


        case LOG_GetCode(LOG_CONTROLLER_FAIL):
            ControllerSnToIndexString(dataPtr->controllerFail.r2.r1.failedControllerSN,
                                      tempStr1);

            ControllerSnToIndexString(dataPtr->controllerFail.r2.r1.controllerSN,
                                      tempStr2);

            /*
             * Print the additional data in the R2 structure
             */
            if (logPtr->length > sizeof(LOG_CONTROLLER_FAIL_R1))
            {
                sprintf(messStr, "ELECT-%s failed by %s (%s)",
                        tempStr1, tempStr2, dataPtr->controllerFail.r2.reasonString);
            }
            else
            {
                sprintf(messStr, "ELECT-%s failed by %s", tempStr1, tempStr2);
            }
            break;

        case LOG_GetCode(LOG_DISASTER):
            sprintf(messStr, "DISASTER-%s", dataPtr->disaster.reasonString);
            break;

        case LOG_GetCode(LOG_DRV_FLT):
            WWNToShortNameString(dataPtr->smartEvent.wwn, tempStr1, secToWait);
            sprintf(messStr, PD " %s fault/reset", tempStr1);
            break;

        case LOG_GetCode(LOG_BYPASS_DEVICE):
            sprintf(messStr, "%s-port %d LID %d bypass, rc = 0x%08X",
                    (dataPtr->portEvent.proc) ? "STORAGE" : "HOST",
                    dataPtr->portEvent.port,
                    dataPtr->portEvent.count, dataPtr->portEvent.reason);
            break;

        case LOG_GetCode(LOG_VLINK_OPEN_BEGIN):
            sprintf(messStr, "VLINK Open begin for VLOP = 0x%08X",
                    dataPtr->vlinkOpenBegin.vlop);
            break;

        case LOG_GetCode(LOG_VLINK_OPEN_END):
            sprintf(messStr, "VLINK Open end for VLOP = 0x%08X",
                    dataPtr->vlinkOpenEnd.vlop);
            break;

        case LOG_GetCode(LOG_RAID5_INOPERATIVE):
            VdiskIdToVblockVdiskIdString(dataPtr->raid5Inop.vdiskID, tempStr1);
            sprintf(messStr, "VDisk %s RAID %d Inoperative", tempStr1,
                    dataPtr->raid5Inop.raidID);
            break;

        case LOG_GetCode(LOG_PARITY_CHECK_RAID):
            switch (dataPtr->parityCheckRAID.type)
            {
                case PARITY_CHECK_RAID_START:
                    sprintf(messStr, "RAID-parity scan on RAID %d beginning",
                            dataPtr->parityCheckRAID.raidID);
                    break;

                case PARITY_CHECK_RAID_END:
                    sprintf(messStr, "RAID-parity scan on RAID %d complete",
                            dataPtr->parityCheckRAID.raidID);
                    break;

                case PARITY_CHECK_RAID_TERM:
                    sprintf(messStr, "RAID-parity scan on RAID %d terminated",
                            dataPtr->parityCheckRAID.raidID);
                    break;

                default:
                    sprintf(messStr, "RAID-parity scan on RAID %d UNKNOWN %d",
                            dataPtr->parityCheckRAID.raidID,
                            dataPtr->parityCheckRAID.type);
                    break;
            }
            break;

        case LOG_GetCode(LOG_ORPHAN_DETECTED):
            VdiskIdToVblockVdiskIdString(dataPtr->orphan.vdiskID, tempStr1);
            sprintf(messStr, "RAID-%d unused in VDisk %s",
                    dataPtr->orphan.raidID, tempStr1);
            break;

        case LOG_GetCode(LOG_DEFRAG_VER_DONE):
            if (dataPtr->defragVerDone.success)
            {
                sprintf(messStr, "Defrag verification good, PID=0x%02hX, RID=0x%02hX, SDA=0x%08X",
                        dataPtr->defragVerDone.pdiskID, dataPtr->defragVerDone.raidID,
                        (UINT32)dataPtr->defragVerDone.sda);
            }
            else
            {
                sprintf(messStr, "Defrag verification failed (PID=0x%02hX), PID=0x%02hX, RID=0x%02hX, SDA=0x%08X",
                        dataPtr->defragVerDone.failedPID, dataPtr->defragVerDone.pdiskID,
                        dataPtr->defragVerDone.raidID,
                        (UINT32)dataPtr->defragVerDone.sda);
            }
            break;

        case LOG_GetCode(LOG_DEFRAG_OP_COMPLETE):
            if (dataPtr->defragOpComplete.pdiskID == MRDEFRAGMENT_ALL_PID)
            {
                strcpy(tempStr2, "ALL");
            }
            else
            {
                PIDToShortNameString(dataPtr->defragOpComplete.pdiskID,
                                     tempStr2, secToWait);
            }

            if (dataPtr->defragOpComplete.status == DEFRAG_OP_COMPLETE_OK)
            {
                strcpy(tempStr1, "COMPLETE");
            }
            else if (dataPtr->defragOpComplete.status == DEFRAG_OP_COMPLETE_ERROR)
            {
                strcpy(tempStr1, "FAILED");
            }
            else if (dataPtr->defragOpComplete.status == DEFRAG_OP_COMPLETE_STOPPED)
            {
                strcpy(tempStr1, "CANCELLED");
            }
            else
            {
                strcpy(tempStr1, "UNKNOWN");
            }

            sprintf(messStr, PD " %s defrag %s", tempStr2, tempStr1);
            break;

        case LOG_GetCode(LOG_NV_MEM_EVENT):
            switch (dataPtr->nvEvent.event)
            {
                case NV_FATAL_ECC_MULTI:
                    sprintf(messStr, "BUFFER-Multi-bit ECC error detected");
                    break;

                case NV_FATAL_ECC_THRESH:
                    sprintf(messStr, "BUFFER-ECC error threshold exceeded");
                    break;

                case NV_FATAL_BATT:
                    sprintf(messStr, "BUFFER-Battery failure detected");
                    break;

                case NV_FATAL_NO_CARD:
                    sprintf(messStr, "BUFFER-No card detected");
                    break;

                case NV_FATAL_POST:
                    sprintf(messStr, "BUFFER-POST failed");
                    break;

                case NV_FATAL_ECC_UNCORR:
                    sprintf(messStr, "BUFFER-Uncorrectable ECC error detected");
                    break;

                case NV_FATAL_ASSERT:
                    sprintf(messStr, "BUFFER-Software sanity error detected");
                    break;

                case NV_FATAL_SN_MISMATCH:
                    sprintf(messStr, "BUFFER-Controller S/N mismatch");
                    break;

                default:
                    sprintf(messStr, "BUFFER-Fatal Event %d", dataPtr->nvEvent.event);
                    break;
            }
            break;

        case LOG_GetCode(LOG_IPMI_EVENT):
            sprintf(messStr, "%s", dataPtr->ipmiEvent.text);
            break;

        case LOG_GetCode(LOG_ERROR_GENERIC):
        case LOG_GetCode(LOG_WARN_GENERIC):
        case LOG_GetCode(LOG_INFO_GENERIC):
            sprintf(messStr, "%s", dataPtr->pGeneric.msgStr);
            break;

        case LOG_GetCode(LOG_SPOOL_CHANGE_I):
        case LOG_GetCode(LOG_SPOOL_CHANGE_W):
        case LOG_GetCode(LOG_SPOOL_CHANGE_E):
        case LOG_GetCode(LOG_SPOOL_CHANGE_D):
            switch (dataPtr->psnap.sub_event_code)
            {
                case SS_CREATE_FAIL:
                    sprintf(messStr, "Batch SS Create Failed (SRC VID=%d, SS=%d) (ec=%d)",
                            dataPtr->psnap.value1,
                            dataPtr->psnap.value2, dataPtr->psnap.errorCode);
                    break;
                case SS_CREATE_GOOD:
                    sprintf(messStr, "Batch SS Created (SRC VID=%d, SS=%d)",
                            dataPtr->psnap.value1, dataPtr->psnap.value2);
                    break;
                case SS_READ_SRC_BAD:
                    sprintf(messStr, "SS SRC (VID=%d) READ failed (ec=%d)",
                            dataPtr->psnap.value2, dataPtr->psnap.errorCode);
                    break;
                case SS_OGER_NV_UPDATE_BAD:    //somewhat redundant...
                    sprintf(messStr, "SS ICL UPDATE failed (ec=%d)",
                            dataPtr->psnap.errorCode);
                    break;
                case SS_BUFFER_BAD:
                    sprintf(messStr, "Snappool %d, SS VID=%d inop (ec=%d)",
                            dataPtr->psnap.value1,
                            dataPtr->psnap.value2, dataPtr->psnap.errorCode);
                    break;
                case SS_BUFFER_OK_NOW:
                    sprintf(messStr, "Snappool %d (VID=%d) is OK now",
                            dataPtr->psnap.value1, dataPtr->psnap.value2);
                    break;
                case SS_BUFFER_FULL_OK:
                    sprintf(messStr, "Snappool %d is <80 Percent full",
                            dataPtr->psnap.value1);
                    break;
                case SS_BUFFER_FULL_WARN:
                    sprintf(messStr, "Snappool %d is >=80 Percent full",
                            dataPtr->psnap.value1);
                    break;
                case SS_BUFFER_OVERFLOW:
                    sprintf(messStr, "Snappool Overflowed (ec=%d)",
                            dataPtr->psnap.errorCode);
                    break;
                case SS_BUFFER_FULL:
                    sprintf(messStr, "Snappool %d is 100 Percent full",
                            dataPtr->psnap.value1);
                    break;
                case SS_BUFFER_EMPTY:
                    sprintf(messStr, "Snappool %d is empty", dataPtr->psnap.value1);
                    break;
                case SS_BUFFER_SET:
                    sprintf(messStr, "Snappool %d (VID=%d) is defined",
                            dataPtr->psnap.value1, dataPtr->psnap.value2);
                    break;
                case SS_BUFFER_UNSET:
                    sprintf(messStr, "Snappool %d (VID=%d) is unset",
                            dataPtr->psnap.value1, dataPtr->psnap.value2);
                    break;
                case SS_BUFFER_OWNER_SET:
                    sprintf(messStr, "Snappool %d Ownership Set", dataPtr->psnap.value1);
                    break;
                case SS_BUFFER_OWNER_UNSET:
                    sprintf(messStr, "Snappool %d Ownership Released",
                            dataPtr->psnap.value1);
                    break;
                case SS_NVRAM_RESTORED_OK:
                    sprintf(messStr, "SS NVRAM Restored OK");
                    break;
                case SS_NVRAM_RESTORED_BAD:
                    sprintf(messStr, "SS NVRAM Restore Failed. EC=%d.%d.%d",
                            dataPtr->psnap.errorCode, dataPtr->psnap.value1,
                            dataPtr->psnap.value2);
                    break;
                case SS_NVRAM_UNCONFIGURED:
                    sprintf(messStr, "SS NVRAM is unconfigured");
                    break;
                case SS_NVRAM_INITIALIZED:
                    sprintf(messStr, "SS NVRAM Initialized");
                    break;
                case SS_FATAL_ERROR:
                    sprintf(messStr, "Unexpected SS Error: ErrorCode=%d",
                            dataPtr->psnap.errorCode);
                    break;
                default:
                    sprintf(messStr, "Oops SS Error: event=%d, ErrorCode=%d",
                            dataPtr->psnap.sub_event_code, dataPtr->psnap.errorCode);
            }
            break;
        case LOG_GetCode(LOG_APOOL_CHANGE_I):
        case LOG_GetCode(LOG_APOOL_CHANGE_W):
        case LOG_GetCode(LOG_APOOL_CHANGE_E):
        case LOG_GetCode(LOG_APOOL_CHANGE_D):
            switch (dataPtr->pasync.sub_event_code)
            {
                case AP_ASYNC_BUFFER_BAD:
                    sprintf(messStr, "Async Buffer %d (VID=%d) is inoperable (ec=%d)",
                            dataPtr->pasync.value1,
                            dataPtr->pasync.value2, dataPtr->pasync.errorCode);
                    break;
                case AP_ASYNC_BUFFER_OK_NOW:
                    sprintf(messStr, "Async Buffer %d (VID=%d) is now operational",
                            dataPtr->pasync.value1, dataPtr->pasync.value2);
                    break;
                case AP_MOVER_STARTED:
                    strcpy(messStr, "Async Remote Data Mover has started");
                    break;
                case AP_MOVER_STOPPED:
                    strcpy(messStr, "Async Remote Data Mover has stopped");
                    break;
                case AP_ASYNC_LINKS_OK:
                    strcpy(messStr, "Async Link(s) are operational");
                    break;
                case AP_ASYNC_LINKS_DOWN:
                    strcpy(messStr, "Async Link(s) are down");
                    break;
                case AP_ASYNC_BUFFER_FULL_OK:
                    strcpy(messStr, "Async Buffer is <33 Percent full");
                    break;
                case AP_ASYNC_BUFFER_FULL_WARN:
                    strcpy(messStr, "Async Buffer is 50-75 Percent full");
                    break;
                case AP_ASYNC_BUFFER_FULL:
                    strcpy(messStr, "Async Buffer is 100 Percent full");
                    break;
                case AP_ASYNC_BUFFER_EMPTY:
                    strcpy(messStr, "Async Buffer is empty");
                    break;
                case AP_ASYNC_BUFFER_SET:
                    sprintf(messStr, "Async Buffer %d (VID=%d) is defined",
                            dataPtr->pasync.value1, dataPtr->pasync.value2);
                    break;
                case AP_ASYNC_BUFFER_UNSET:
                    sprintf(messStr, "Async Buffer %d (VID=%d) is unset",
                            dataPtr->pasync.value1, dataPtr->pasync.value2);
                    break;
                case AP_ALINK_SET:
                    sprintf(messStr, "Async Link %d is defined", dataPtr->pasync.value1);
                    break;
                case AP_ALINK_UNSET:
                    sprintf(messStr, "Async Link %d is unset", dataPtr->pasync.value1);
                    break;
                case AP_ASYNC_BUFFER_OWNER_SET:
                    sprintf(messStr, "Async Buffer %d Ownership Set",
                            dataPtr->pasync.value1);
                    break;
                case AP_ASYNC_BUFFER_OWNER_UNSET:
                    sprintf(messStr, "Async Buffer %d Ownership Released",
                            dataPtr->pasync.value1);
                    break;
                case AP_ASYNC_NVRAM_RESTORED_OK:
                    sprintf(messStr, "Async NVRAM Restored OK");
                    break;
                case AP_ASYNC_NVRAM_RESTORED_BAD:
                    sprintf(messStr, "Async NVRAM Restore Failed. EC=%d.%d.%d",
                            dataPtr->pasync.errorCode, dataPtr->pasync.value1,
                            dataPtr->pasync.value2);
                    break;
                case AP_ASYNC_BUFFER_EXPANDED:
                    sprintf(messStr, "Async Buffer %d expanded to %d MB",
                            dataPtr->pasync.value1, dataPtr->pasync.value2);
                    break;
                case AP_ASYNC_BUFFER_NOEXPAND:
                    sprintf(messStr, "Async Buffer %d at max expands of %d",
                            dataPtr->pasync.value1, dataPtr->pasync.value2);
                    break;
                case AP_BREAK_ALL_ASYNC_MIRRORS:
                    sprintf(messStr, "Async Buffer %d EC=%d. Breaking Mirrors",
                            dataPtr->pasync.value1, dataPtr->pasync.errorCode);
                    break;
                case AP_ASYNC_BUFFER_IO_BAD:
                    sprintf(messStr, "Async Buffer %d I/O Failed. EC=%d.%d",
                            dataPtr->pasync.value1, dataPtr->pasync.errorCode,
                            dataPtr->pasync.value2);
                    break;
                case AP_ASYNC_NVRAM_UNCONFIGURED:
                    sprintf(messStr, "Async NVRAM is unconfigured");
                    break;
                case AP_ASYNC_NVRAM_INITIALIZED:
                    sprintf(messStr, "Async NVRAM Initialized");
                    break;
                case AP_FATAL_ERROR:
                    sprintf(messStr, "Unexpected Async Error: ErrorCode=%d",
                            dataPtr->pasync.errorCode);
                    break;
            }
            break;

        case LOG_GetCode(LOG_APOOL_CHANGE):
            sprintf(messStr, "APOOL: %s", dataPtr->pGeneric.msgStr);
            break;

        case LOG_GetCode(LOG_ISP_FATAL):
            sprintf(messStr, "%s port %d qlogic fatal error",
                    (dataPtr->portEvent.proc) ? "STORAGE" : "HOST",
                    dataPtr->portEvent.port);
            break;

        case LOG_GetCode(LOG_COPY_LABEL):
            VdiskIdToVblockVdiskIdString(dataPtr->corLabel.svid, tempStr1);
            VdiskIdToVblockVdiskIdString(dataPtr->corLabel.dvid, tempStr2);
            sprintf(messStr, "COPY %s->%s LABEL CHANGED", tempStr1, tempStr2);
            break;

        case LOG_GetCode(LOG_ISE_ELEM_CHANGE):
            sprintf(messStr, "ISE BAY %d environment changed, bitmap %d",
                    dataPtr->iseElemChange.map_data.bayid,
                    dataPtr->iseElemChange.map_data.envmap);
            break;

        case LOG_GetCode(LOG_ISE_ELEM_CHANGE_I):
            switch (dataPtr->iseElemChange.component)
            {
            case LOG_ISE_MRC:
                sprintf(messStr, "ISE %d-MRC %d IS OK NOW",
                        dataPtr->iseElemChange.mrc_data.bayid,
                        dataPtr->iseElemChange.mrc_data.mrc_no);
                break;

            case LOG_ISE_BATTERY:
                sprintf(messStr, "ISE %d-BATTERY %d IS OK NOW",
                        dataPtr->iseElemChange.bat_data.bayid,
                        dataPtr->iseElemChange.bat_data.bat_no);
                break;

#if 0   // ATS-153  ISE's MRC fails and you can not get this information.
            case LOG_ISE_DPAC:
                sprintf(messStr, "ISE %d-DATAPAC %d IS OK NOW",
                        dataPtr->iseElemChange.dpac_data.bayid,
                        dataPtr->iseElemChange.dpac_data.dpac_no);
                break;
#endif  /* 0 */

            case LOG_ISE_PS:
                sprintf(messStr, "ISE %d-POWERSUPPLY %d IS OK NOW",
                        dataPtr->iseElemChange.ps_data.bayid,
                        dataPtr->iseElemChange.ps_data.ps_no);
                break;

            case LOG_ISE_TEMP:
                sprintf(messStr, "ISE %d-TEMPERATURE %d IS OK NOW",
                        dataPtr->iseElemChange.temp_data.bayid,
                        dataPtr->iseElemChange.temp_data.ise_temperature);
                break;
            }
            break;

        case LOG_GetCode(LOG_ISE_ELEM_CHANGE_W):
            switch (dataPtr->iseElemChange.component)
            {
                case LOG_ISE_MRC:
                    mrc_reason_string(tempStr1, dataPtr->iseElemChange.mrc_data.reason_code);
                    sprintf(messStr, "ISE %d-MRC %d IS DEGRADED (%s)",
                            dataPtr->iseElemChange.mrc_data.bayid,
                            dataPtr->iseElemChange.mrc_data.mrc_no,
                            tempStr1);
                    break;
                case LOG_ISE_BATTERY:
                    sprintf(messStr, "ISE %d-BATTERY %d IS DEGRADED (CHARGING)",
                            dataPtr->iseElemChange.bat_data.bayid,
                            dataPtr->iseElemChange.bat_data.bat_no);
                    break;
#if 0   // ATS-153  ISE's MRC fails and you can not get this information.
                case LOG_ISE_DPAC:
                    sprintf(messStr, "ISE %d-DATAPAC %d IS DEGRADED",
                            dataPtr->iseElemChange.dpac_data.bayid,
                            dataPtr->iseElemChange.dpac_data.dpac_no);
                    break;
#endif  /* 0 */
                case LOG_ISE_PS:
                    sprintf(messStr, "ISE %d-POWERSUPPLY %d IS DEGRADED (1FAN)",
                            dataPtr->iseElemChange.ps_data.bayid,
                            dataPtr->iseElemChange.ps_data.ps_no);
                    break;
                case LOG_ISE_TEMP:
                    sprintf(messStr, "ISE %d-WARNING TEMPERATURE %d",
                            dataPtr->iseElemChange.temp_data.bayid,
                            dataPtr->iseElemChange.temp_data.ise_temperature);
                    break;
            }
            break;
        case LOG_GetCode(LOG_ISE_ELEM_CHANGE_E):
            switch (dataPtr->iseElemChange.component)
            {
                case LOG_ISE_MRC:
                    mrc_reason_string(tempStr1, dataPtr->iseElemChange.mrc_data.reason_code);
                    sprintf(messStr, "ISE %d-MRC %d IS FAILED (%s)",
                            dataPtr->iseElemChange.mrc_data.bayid,
                            dataPtr->iseElemChange.mrc_data.mrc_no,
                            tempStr1);
                    break;
                case LOG_ISE_BATTERY:
                    sprintf(messStr, "ISE %d-BATTERY %d IS FAILED",
                            dataPtr->iseElemChange.bat_data.bayid,
                            dataPtr->iseElemChange.bat_data.bat_no);
                    break;
#if 0   // ATS-153  ISE's MRC fails and you can not get this information.
                case LOG_ISE_DPAC:
                    sprintf(messStr, "ISE %d-DATAPAC %d IS FAILED",
                            dataPtr->iseElemChange.dpac_data.bayid,
                            dataPtr->iseElemChange.dpac_data.dpac_no);
                    break;
#endif  /* 0 */
                case LOG_ISE_PS:
                    sprintf(messStr, "ISE %d-POWERSUPPLY %d IS FAILED",
                            dataPtr->iseElemChange.ps_data.bayid,
                            dataPtr->iseElemChange.ps_data.ps_no);
                    break;
                case LOG_ISE_TEMP:
                    sprintf(messStr, "ISE %d-CRITICAL ERROR TEMPERATURE %d",
                            dataPtr->iseElemChange.temp_data.bayid,
                            dataPtr->iseElemChange.temp_data.ise_temperature);
                    break;
            }
            break;
        case LOG_GetCode(LOG_RAID_INOPERATIVE):
            VdiskIdToVblockVdiskIdString(dataPtr->raidInop.vdiskID, tempStr1);
            sprintf(messStr, "VDisk %s RAID %d Inoperative", tempStr1,
                    dataPtr->raidInop.raidID);
            break;

        case LOG_GetCode(LOG_ISE_SPECIAL):
            sprintf(messStr, "ISE BUSY %s on ISE=%d",
                    dataPtr->iseSpecial.busyFlag ? "START" : "END",
                    dataPtr->iseSpecial.bayID);
            break;

        default:
            sprintf(messStr, "LOG-Undefined log entry - 0x%x", logPtr->eventCode);
            break;
    }
}

/*----------------------------------------------------------------------------
**  Function:   ExtendedMessage
**
**  Comments:   Parse the passed event code and create the extended message
**              associated with this event
**
**  Parameters: logPtr - pointer to the desired log entry
**              type   - used for string formatting options
**              indent - formatting option
**
**  Returns:    strPtr - contains a terminated string on entry.  The
**                       extended message is copied into this string.
**
**  NOTES:      PLEASE KEEP EVENT CODES IN ORDER AS THEY APPEAR IN DEF.H!
**
**--------------------------------------------------------------------------*/
void ExtendedMessage(char *strPtr, LOG_HDR *logPtr, char type, char indent)
{
    char        space[34];
    char        crlf[6];
    char        lfnb[6];
    int         eventCode = logPtr->eventCode;
    char       *data = ((char *)logPtr) + sizeof(LOG_HDR);
    LOG_DATA_PKT *dataPtr = (LOG_DATA_PKT *)(((char *)logPtr) + sizeof(LOG_HDR));
    int         i = 0;
    int         j = 0;
    char        tempStr1[MAX_MESSAGE_LENGTH] = { 0 };
    char        tempStr2[MAX_MESSAGE_LENGTH] = { 0 };
    char        tempStr3[MAX_MESSAGE_LENGTH] = { 0 };

    /*
     * Set up the type of line break, based on the type of caller
     */
    if (type == HTML_TYPE)
    {
        strcpy(crlf, "<br>");   /* Break                    */
        strcpy(lfnb, " ");      /* No Break for HTML        */
    }
    else if (type == ASCII_TYPE)
    {
        strcpy(crlf, "\r\n");   /* Carriage Return / Line feed */
        strcpy(lfnb, "\r\n");   /* Carriage Return / Line Feed */
    }
    else if (type == JAVASCRIPT_TYPE)
    {
        strcpy(crlf, "\\n");    /* Carriage Return / Line feed */
        strcpy(lfnb, "");       /* Carriage Return / Line Feed */
    }

    /*
     * Set up the amount of indentation based on the request of the
     * of the caller.
     */
    if (indent == INDENT)
    {
        strcpy(space, ">");     /* Indent         */
    }
    else
    {
        strcpy(space, "");      /* No Indentation */
    }

    /*
     * Construct the extended message based on event code.
     * PLEASE KEEP EVENT CODES IN ORDER AS THEY APPEAR IN DEF.H!
     */
    switch (LOG_GetCode(eventCode))
    {
            /*
             * List all event codes here that do NOT have extended messages.
             */

            /*
             * ----- Informational Events - no entry -----
             */
            /* 0x000 */
        case LOG_GetCode(LOG_NVRAM_RESTORE):
        case LOG_GetCode(LOG_SCRUB_DONE):
        case LOG_GetCode(LOG_BOOT_COMPLETE):
        case LOG_GetCode(LOG_BUFFER_BOARDS_ENABLED):
        case LOG_GetCode(LOG_CONFIG_CHANGED):
        case LOG_GetCode(LOG_LOG_TEXT_MESSAGE_INFO):   /* 0x009  */

            /* 0x010 */
        case LOG_GetCode(LOG_CACHE_FLUSH_RECOVER):

            /* 0x020 */

            /* 0x030 */
        case LOG_GetCode(LOG_EST_VLINK):
        case LOG_GetCode(LOG_TERM_VLINK):
        case LOG_GetCode(LOG_SWP_VLINK):
        case LOG_GetCode(LOG_I2C_BUS_GOOD):

            /* 0x040 */
        case LOG_GetCode(LOG_WORKSET_CHANGED):
        case LOG_GetCode(LOG_WRITE_FLUSH_COMPLETE):
        case LOG_GetCode(LOG_PULSE):
        case LOG_GetCode(LOG_INIT_CCB_NVRAM_OP):
        case LOG_GetCode(LOG_NVRAM_RELOAD):

            /* 0x050 */
        case LOG_GetCode(LOG_VCG_START_IO_OP):
        case LOG_GetCode(LOG_VCG_STOP_IO_OP):
        case LOG_GetCode(LOG_VCG_GLOBAL_CACHE_OP):
        case LOG_GetCode(LOG_VCG_UNFAIL_CONTROLLER_OP):
        case LOG_GetCode(LOG_VCG_FAIL_CONTROLLER_OP):
        case LOG_GetCode(LOG_VCG_REMOVE_CONTROLLER_OP):

            /* 0x060 */
        case LOG_GetCode(LOG_ALL_DEV_MISSING):
        case LOG_GetCode(LOG_ADMIN_SETTIME_OP):
        case LOG_GetCode(LOG_CONTROLLERS_READY):
        case LOG_GetCode(LOG_POWER_UP_COMPLETE):
        case LOG_GetCode(LOG_VALIDATION):

            /* 0x080 */
        case LOG_GetCode(LOG_BUFFER_BOARDS_DISABLED_INFO):

            /*
             * ----- Warning Events - no entry  -----
             */
            /* 0x180 */
        case LOG_GetCode(LOG_CCB_MEMORY_HEALTH_ALERT):
        case LOG_GetCode(LOG_PROC_MEMORY_HEALTH_ALERT):
            break;

        case LOG_GetCode(LOG_HOTSPARE_DEPLETED):
            {
                sprintf(strPtr + strlen(strPtr), "%s type %d, device %d, devtype %d%s",
                        space,
                        dataPtr->hotspareDepleted.type,
                        dataPtr->hotspareDepleted.dev,
                        dataPtr->hotspareDepleted.devType, crlf);
            }
            break;

            /* 0x1C0 */
        case LOG_GetCode(LOG_APOOL_CHANGE):
        case LOG_GetCode(LOG_APOOL_CHANGE_I):
        case LOG_GetCode(LOG_APOOL_CHANGE_W):
        case LOG_GetCode(LOG_APOOL_CHANGE_E):
        case LOG_GetCode(LOG_APOOL_CHANGE_D):

        case LOG_GetCode(LOG_SPOOL_CHANGE_I):
        case LOG_GetCode(LOG_SPOOL_CHANGE_W):
        case LOG_GetCode(LOG_SPOOL_CHANGE_E):
        case LOG_GetCode(LOG_SPOOL_CHANGE_D):
        case LOG_GetCode(LOG_ISE_ELEM_CHANGE):
            /* 0x200 */
        case LOG_GetCode(LOG_MSG_DELETED):
        case LOG_GetCode(LOG_CCB_NVRAM_RESTORED):
        case LOG_GetCode(LOG_LOOP_DOWN):
        case LOG_GetCode(LOG_LOG_TEXT_MESSAGE_WARNING):        /* 0x203  */

            /* 0x210 */
            /* 0x220 */
            /* 0x230 */
        case LOG_GetCode(LOG_BUFFER_BOARDS_DISABLED_WARN):

            /*
             * ----- Error Events - no entry  -----
             */
            /* 0x400 */
        case LOG_GetCode(LOG_BUFFER_BOARDS_DISABLED_ERROR):
        case LOG_GetCode(LOG_LOG_TEXT_MESSAGE_ERROR):  /* 0x404  */

            /* 0x410 */
        case LOG_GetCode(LOG_CCB_NVRAM_RESET):

            /* 0x420 */
        case LOG_GetCode(LOG_I2C_BUS_FAIL):

            /* 0x430 */
        case LOG_GetCode(LOG_NVRAM_CHKSUM_ERR):

            /* 0x440 */
        case LOG_GetCode(LOG_PROC_COMM_NOT_READY):

        case LOG_GetCode(LOG_NO_OWNED_DRIVES):
        case LOG_GetCode(LOG_CTRL_FAILED):
        case LOG_GetCode(LOG_CTRL_UNUSED):
        case LOG_GetCode(LOG_FWV_INCOMPATIBLE):
        case LOG_GetCode(LOG_MISSING_DISK_BAY):
        case LOG_GetCode(LOG_WAIT_CORRUPT_BE_NVRAM):
        case LOG_GetCode(LOG_MISSING_CONTROLLER):
        case LOG_GetCode(LOG_WAIT_DISASTER):
        case LOG_GetCode(LOG_DISASTER):

        case LOG_GetCode(LOG_NVRAM_WRITE_FAIL):
        case LOG_GetCode(LOG_DELAY_INOP):

            /*
             * ----- Debug Events - no entry  -----
             */
            /* 0x4000 */
        case LOG_GetCode(LOG_RSCN):
        case LOG_GetCode(LOG_REFRESH_NVRAM):
        case LOG_GetCode(LOG_SOCKET_ERROR):
        case LOG_GetCode(LOG_LOCAL_IMAGE_READY):

            /* 0x4010 */
        case LOG_GetCode(LOG_NVRAM_WRITTEN):
        case LOG_GetCode(LOG_HBEAT_STOP):
        case LOG_GetCode(LOG_PARITY_SCAN_REQUIRED):
        case LOG_GetCode(LOG_LOG_TEXT_MESSAGE_DEBUG):

            /* 0x4020 */
        case LOG_GetCode(LOG_NO_CONFIGURATION):
        case LOG_GetCode(LOG_NO_LICENSE):
        case LOG_GetCode(LOG_NO_MIRROR_PARTNER):
        case LOG_GetCode(LOG_LPDN_RETRY):
        case LOG_GetCode(LOG_VALIDATION_DEBUG):
        case LOG_GetCode(LOG_RAID_EVENT):
//        case LOG_GetCode(LOG_PROC_PRINTF):
        case LOG_GetCode(LOG_BYPASS_DEVICE):
        case LOG_GetCode(LOG_PARITY_CHECK_RAID):
        case LOG_GetCode(LOG_ORPHAN_DETECTED):
            break;

            /*
             * ----- Informational Events -----
             */
        case LOG_GetCode(LOG_DISKBAY_INSERTED):
        case LOG_GetCode(LOG_DISKBAY_REMOVED):
        case LOG_GetCode(LOG_DISKBAY_MOVED):
        case LOG_GetCode(LOG_DEVICE_INSERTED):
        case LOG_GetCode(LOG_DEVICE_REMOVED):
        case LOG_GetCode(LOG_DEVICE_RESET):
        case LOG_GetCode(LOG_DEVICE_MISSING):
        case LOG_GetCode(LOG_DEVICE_TIMEOUT):
            {
                WWNToString(dataPtr->deviceRemoved.wwn, tempStr1);
                sprintf(strPtr + strlen(strPtr), "%s WWN %s, PID %d, port %d,  LUN " HEX4 "  "
                        "LID " HEX8 " %s",
                        space,
                        tempStr1,
                        dataPtr->deviceRemoved.pid,
                        dataPtr->deviceRemoved.channel,
                        dataPtr->deviceRemoved.lun, dataPtr->deviceRemoved.lid, crlf);
            }
            break;
        case LOG_GetCode(LOG_DEVICE_REATTACED):        /* 0x006 */
            {
                WWNToString(dataPtr->deviceReattached.wwn, tempStr1);
                sprintf(strPtr + strlen(strPtr), "%s WWN %s, port %d,  LUN " HEX4 "  "
                        "LID " HEX8 " %s",
                        space,
                        tempStr1,
                        dataPtr->deviceReattached.channel,
                        dataPtr->deviceReattached.lun,
                        dataPtr->deviceReattached.lid, crlf);
            }
            break;

        case LOG_GetCode(LOG_HOST_OFFLINE):
            {
                sprintf(strPtr + strlen(strPtr), "%s port %d,  taskflags " HEX2 "  "
                        "qstatus " HEX2 " %s",
                        space,
                        dataPtr->hostOffline.port,
                        dataPtr->hostOffline.taskflags,
                        dataPtr->hostOffline.qstatus, crlf);
            }
            break;

        case LOG_GetCode(LOG_COPY_COMPLETE):   /* 0x007 */
        case LOG_GetCode(LOG_COPY_SYNC):       /* 0x05F */
            sprintf(strPtr + strlen(strPtr), "%s percent %d,  "
                    "function " HEX2 ",  status " HEX2 " %s",
                    space,
                    dataPtr->copyComplete.percent,
                    dataPtr->copyComplete.function, dataPtr->copyComplete.compstat, crlf);
            sprintf(strPtr + strlen(strPtr), "%s destination VID %d,  "
                    "source VID %d %s",
                    space,
                    dataPtr->copyComplete.destVid, dataPtr->copyComplete.srcVid, crlf);
            break;

        case LOG_GetCode(LOG_BATTERY_ALERT):
            {
                sprintf(strPtr + strlen(strPtr), "%s bank %d, state " HEX2 "  "
                        "status " HEX2 " %s",
                        space,
                        dataPtr->batteryState.bank,
                        dataPtr->batteryState.state, dataPtr->batteryState.status, crlf);
            }
            break;

        case LOG_GetCode(LOG_FIRMWARE_UPDATE): /* 0x011 */
        case LOG_GetCode(LOG_FW_UPDATE_FAILED):        /* 0x432 */
            {
                char        str1[5];
                char        str2[5];

                *(int *)&str1 = WORD(0);
                str1[4] = 0;
                *(int *)&str2 = WORD(1);
                str2[4] = 0;

                sprintf(strPtr + strlen(strPtr), "%s version %s,  sequence %s %s",
                        space, str1, str2, lfnb);

                *(int *)&str1 = WORD(2);

                sprintf(strPtr + strlen(strPtr),
                        "%s build ID %s,  target ID 0x%hX (%s) %s", space, str1,
                        dataPtr->firmwareUpdate.targetId,
                        TargetName(dataPtr->firmwareUpdate.targetId), crlf);
                sprintf(strPtr + strlen(strPtr),
                        "%s timestamp %02d/%02d/%02d %02d:%02d:%02d %s", space,
                        BCD2Short(dataPtr->firmwareUpdate.timeStamp.month),
                        BCD2Short(dataPtr->firmwareUpdate.timeStamp.date),
                        BCD2Short(dataPtr->firmwareUpdate.timeStamp.year),
                        BCD2Short(dataPtr->firmwareUpdate.timeStamp.hours),
                        BCD2Short(dataPtr->firmwareUpdate.timeStamp.minutes),
                        BCD2Short(dataPtr->firmwareUpdate.timeStamp.seconds), lfnb);

                switch (LOG_GetCode(eventCode))
                {
                    case LOG_GetCode(LOG_FIRMWARE_UPDATE):
                    case LOG_GetCode(LOG_FW_UPDATE_FAILED):
                        sprintf(strPtr + strlen(strPtr), "%s crc " HEX8N " %s",
                                space, dataPtr->firmwareUpdate.crc, crlf);

                        if (LOG_GetCode(eventCode) == LOG_GetCode(LOG_FW_UPDATE_FAILED))
                        {
                            sprintf(strPtr + strlen(strPtr), "%s failure reason %d %s",
                                    space, dataPtr->firmwareUpdate.reason, crlf);
                        }
                        break;
                }
                break;
            }

        case LOG_GetCode(LOG_SERVER_DISASSOC_OP):
        case LOG_GetCode(LOG_SERVER_ASSOC_OP):
            {
                sprintf(strPtr + strlen(strPtr), "%s SID %d,  LUN %d, "
                        "VID %d, status " HEX2 ", ec " HEX8 " %s",
                        space,
                        dataPtr->serverAssociateOp.sid,
                        dataPtr->serverAssociateOp.lun,
                        dataPtr->serverAssociateOp.vid,
                        dataPtr->serverAssociateOp.status,
                        dataPtr->serverAssociateOp.errorCode, crlf);
            }
            break;

        case LOG_GetCode(LOG_SERVER_DELETE_OP):
        case LOG_GetCode(LOG_PDISK_DEFRAG_OP):
        case LOG_GetCode(LOG_PDISK_UNFAIL_OP):
        case LOG_GetCode(LOG_VLINK_NAME_CHANGED):
        case LOG_GetCode(LOG_VLINK_BREAK_LOCK_OP):
        case LOG_GetCode(LOG_RAID_INIT_OP):
        case LOG_GetCode(LOG_VDISK_DELETE_OP):
            {
                sprintf(strPtr + strlen(strPtr), "%s ID %d, "
                        "status " HEX2 ", ec " HEX8 " %s",
                        space,
                        dataPtr->vdiskDeleteOp.id,
                        dataPtr->vdiskDeleteOp.status,
                        dataPtr->vdiskDeleteOp.errorCode, crlf);
            }
            break;

        case LOG_GetCode(LOG_VDISK_CONTROL_OP):
            {
                sprintf(strPtr + strlen(strPtr), "%s srcVID %d, destVID %d, "
                        "subOpType " HEX2 ", status " HEX2 ", ec " HEX8 " %s",
                        space,
                        dataPtr->vdiskControlOp.srcVid,
                        dataPtr->vdiskControlOp.destVid,
                        dataPtr->vdiskControlOp.subOpType,
                        dataPtr->vdiskControlOp.status,
                        dataPtr->vdiskControlOp.errorCode, crlf);
            }
            break;

        case LOG_GetCode(LOG_VCG_SET_CACHE_OP):
            {
                sprintf(strPtr + strlen(strPtr), "%s mode " HEX2 ", "
                        "status " HEX2 ", ec " HEX8 " %s",
                        space,
                        dataPtr->vcgSetCacheOp.mode,
                        dataPtr->vcgSetCacheOp.status,
                        dataPtr->vcgSetCacheOp.errorCode, crlf);
            }
            break;

        case LOG_GetCode(LOG_VDISK_SET_ATTRIBUTE_OP):
            {
                sprintf(strPtr + strlen(strPtr), "%s VID %d, mode " HEX2 ", "
                        "status " HEX2 ", ec " HEX8 " %s",
                        space,
                        dataPtr->vdiskSetAttributeOp.vid,
                        dataPtr->vdiskSetAttributeOp.mode,
                        dataPtr->vdiskSetAttributeOp.status,
                        dataPtr->vdiskSetAttributeOp.errorCode, crlf);
            }
            break;

        case LOG_GetCode(LOG_DEVICE_DELETE_OP):
            {
                if (dataPtr->deviceDeleteOp.type == DELETE_DEVICE_DRIVE)
                {
                    strcpy(tempStr1, PD);
                }
                else
                {
                    strcpy(tempStr1, BAY);
                }

                sprintf(strPtr + strlen(strPtr), "%s %s ID %d, "
                        "status " HEX2 ", ec " HEX8 " %s",
                        space,
                        tempStr1,
                        dataPtr->deviceDeleteOp.id,
                        dataPtr->deviceDeleteOp.status,
                        dataPtr->deviceDeleteOp.errorCode, crlf);
            }
            break;

        case LOG_GetCode(LOG_SERVER_CREATE_OP):        /* 0x012 */
            {
                WWNToString(dataPtr->serverCreateOp.wwn, tempStr1);
                sprintf(strPtr + strlen(strPtr), "%s WWN %s TID %d,  owner %d, "
                        "SID %d, status " HEX2 ", ec " HEX8 " %s",
                        space,
                        tempStr1,
                        dataPtr->serverCreateOp.targetId,
                        dataPtr->serverCreateOp.owner,
                        dataPtr->serverCreateOp.sid,
                        dataPtr->serverCreateOp.status,
                        dataPtr->serverCreateOp.errorCode, crlf);
            }
            break;

        case LOG_GetCode(LOG_VDISK_CREATE_OP): /* 0x016 */
        case LOG_GetCode(LOG_VDISK_EXPAND_OP): /* 0x018 */
        case LOG_GetCode(LOG_VDISK_PREPARE_OP):        /* 0x019 */
            sprintf(strPtr + strlen(strPtr), "%s RAID type %d,  "
                    "mirror depth %d,  parity %d,  drives %d,  stripe %d "
                    "VID %d, flags 0x%hhX,  minPD %d,  status " HEX2 ", ec " HEX8 "  %s",
                    space,
                    dataPtr->vdiskCreateOp.raidType,
                    dataPtr->vdiskCreateOp.mirrorDepth,
                    dataPtr->vdiskCreateOp.parity,
                    dataPtr->vdiskCreateOp.drives,
                    dataPtr->vdiskCreateOp.stripe,
                    dataPtr->vdiskCreateOp.vid,
                    dataPtr->vdiskCreateOp.flags,
                    dataPtr->vdiskCreateOp.minPD,
                    dataPtr->vdiskCreateOp.status,
                    dataPtr->vdiskCreateOp.errorCode, crlf);
            sprintf(strPtr + strlen(strPtr), "%s requested capacity " HEX8 "" HEX8N ",  "
                    "actual capacity " HEX8 "" HEX8N " %s",
                    space,
                    (UINT32)((dataPtr->vdiskCreateOp.requestCapacity) >> 32),
                    (UINT32)(dataPtr->vdiskCreateOp.requestCapacity),
                    (UINT32)((dataPtr->vdiskCreateOp.actualCapacity) >> 32),
                    (UINT32)(dataPtr->vdiskCreateOp.actualCapacity), crlf);
            break;

        case LOG_GetCode(LOG_RAID_CONTROL_OP): /* 0x01E */
            sprintf(strPtr + strlen(strPtr), "%s scrub control " HEX8 ",  "
                    "parity control " HEX8 ",  scrub state " HEX2 ",  "
                    "%s %s parity state " HEX8 ", scrub pid %d,  "
                    "scrub block %d %s %s parity rid %d  "
                    "parity block %d %s",
                    space,
                    dataPtr->raidControlOp.scrubcontrol,
                    dataPtr->raidControlOp.paritycontrol,
                    dataPtr->raidControlOp.sstate,
                    crlf, space,
                    dataPtr->raidControlOp.pstate,
                    dataPtr->raidControlOp.scrubpid,
                    dataPtr->raidControlOp.scrubblock,
                    crlf, space,
                    dataPtr->raidControlOp.scanrid,
                    dataPtr->raidControlOp.scanblock, crlf);
            break;

            /* 0x020 */
        case LOG_GetCode(LOG_PROC_ASSIGN_MIRROR_PARTNER_OP):
            {
                sprintf(strPtr + strlen(strPtr), "%s newSN %d, oldSN %d, "
                        "status " HEX2 ", ec " HEX8 " %s",
                        space,
                        dataPtr->procAssignMirrorOp.newSerialNumber,
                        dataPtr->procAssignMirrorOp.oldSerialNumber,
                        dataPtr->procAssignMirrorOp.status,
                        dataPtr->procAssignMirrorOp.errorCode, crlf);
            }
            break;

        case LOG_GetCode(LOG_PDISK_FAIL_OP):
            {
                sprintf(strPtr + strlen(strPtr), "%s PID %d, HSPID %d, "
                        "options " HEX2 ", status " HEX2 ", ec " HEX8 " %s",
                        space,
                        dataPtr->pdiskFailOp.pid,
                        dataPtr->pdiskFailOp.hspid,
                        dataPtr->pdiskFailOp.options,
                        dataPtr->pdiskFailOp.status,
                        dataPtr->pdiskFailOp.errorCode, crlf);
            }
            break;

        case LOG_GetCode(LOG_VDISK_SET_PRIORITY):
            {
                sprintf(strPtr + strlen(strPtr), "%s status " HEX2 ", ec " HEX8 " %s",
                        space, dataPtr->setVPri.status, dataPtr->setVPri.errorCode, crlf);
            }
            break;

        case LOG_GetCode(LOG_PDISK_SPINDOWN_OP):
            {
                sprintf(strPtr + strlen(strPtr), "%s PID %d, "
                        "status " HEX2 ", ec " HEX8 " %s",
                        space,
                        dataPtr->pSpindown.pid,
                        dataPtr->pSpindown.status, dataPtr->pSpindown.errorCode, crlf);
            }
            break;

        case LOG_GetCode(LOG_PDISK_FAILBACK_OP):
            {
                sprintf(strPtr + strlen(strPtr), "%s PID %d,"
                        "options " HEX2 ", status " HEX2 ", ec " HEX8 " %s",
                        space,
                        dataPtr->pFailback.pid,
                        dataPtr->pFailback.options,
                        dataPtr->pFailback.status, dataPtr->pFailback.errorCode, crlf);
            }
            break;

/*
        case LOG_GetCode(LOG_PDISK_AUTO_FAILBACK_OP):
        {
            sprintf(strPtr + strlen(strPtr),"%s, "
                    "status "HEX2", ec "HEX8" %s",
                    space,
                    dataPtr->pAutoFailback.status,
                    dataPtr->pAutoFailback.errorCode,
                    crlf);
        }
        break;
*/

#if ISCSI_CODE
        case LOG_GetCode(LOG_ISCSI_SET_CHAP):
            {
                sprintf(strPtr + strlen(strPtr), "%s status " HEX2 ", ec " HEX8 " %s",
                        space,
                        dataPtr->piSCSISetChap.status,
                        dataPtr->piSCSISetChap.errorCode, crlf);
            }
            break;

        case LOG_GetCode(LOG_ISCSI_SET_INFO):
            {
                sprintf(strPtr + strlen(strPtr), "%s status " HEX2 ", ec " HEX8 " %s",
                        space,
                        dataPtr->piSCSISetInfo.status,
                        dataPtr->piSCSISetInfo.errorCode, crlf);
            }
            break;
#endif /* ISCSI_CODE */
        case LOG_GetCode(LOG_PSD_REBUILD_START):
        case LOG_GetCode(LOG_PSD_REBUILD_DONE):
            {
                WWNToString(dataPtr->psdRebuildDone.wwn, tempStr1);
                sprintf(strPtr + strlen(strPtr), "%s WWN %s, RID %d, VID %d, "
                        "LUN %d, ec " HEX4 " %s",
                        space,
                        tempStr1,
                        dataPtr->psdRebuildDone.rid,
                        dataPtr->psdRebuildDone.vid,
                        dataPtr->psdRebuildDone.lun,
                        dataPtr->psdRebuildDone.errorCode, crlf);
            }
            break;

        case LOG_GetCode(LOG_PDD_REBUILD_DONE):
        case LOG_GetCode(LOG_HSPARE_DONE):
            {
                WWNToString(dataPtr->pddRebuildDone.wwn, tempStr1);
                sprintf(strPtr + strlen(strPtr), "%s WWN %s, PID %d, "
                        "LUN %d %s",
                        space,
                        tempStr1,
                        dataPtr->pddRebuildDone.pid, dataPtr->pddRebuildDone.lun, crlf);
            }
            break;

        case LOG_GetCode(LOG_RAID_INIT_DONE):
            {
                sprintf(strPtr + strlen(strPtr), "%s RID %d, VID %d, "
                        "status " HEX2 ", ecount %d %s",
                        space,
                        dataPtr->raidInitDone.rid,
                        dataPtr->raidInitDone.vid,
                        dataPtr->raidInitDone.status,
                        dataPtr->raidInitDone.errorCount, crlf);
            }
            break;

        case LOG_GetCode(LOG_CONFIGURATION_SESSION_START):
        case LOG_GetCode(LOG_CONFIGURATION_SESSION_START_DEBUG):
        case LOG_GetCode(LOG_CONFIGURATION_SESSION_END):
        case LOG_GetCode(LOG_CONFIGURATION_SESSION_END_DEBUG):
            {
                InetToAscii(dataPtr->configSessionStartStop.IPAddress, tempStr1);
                sprintf(strPtr + strlen(strPtr), "%s IP %s, port %d, "
                        "flags " HEX4 " %s",
                        space,
                        tempStr1,
                        dataPtr->configSessionStartStop.port,
                        dataPtr->configSessionStartStop.flags, crlf);
            }
            break;

        case LOG_GetCode(LOG_PDISK_LABEL_OP):  /* 0x024 */
            WWNToString(dataPtr->pdiskLabelOp.wwn, tempStr1);
            sprintf(strPtr + strlen(strPtr), "%s WWN %s, PID %d, label type %d, "
                    "option " HEX2 ", status " HEX2 ", ec " HEX8 " %s",
                    space,
                    tempStr1,
                    dataPtr->pdiskLabelOp.pid,
                    dataPtr->pdiskLabelOp.labtype,
                    dataPtr->pdiskLabelOp.option,
                    dataPtr->pdiskLabelOp.status, dataPtr->pdiskLabelOp.errorCode, crlf);
            break;

        case LOG_GetCode(LOG_DRIVE_BAY_FW_UPDATE):     /* 0x02C */
            {
                char        str1[5];
                char        str2[5];

                *(int *)&str1 = WORD(0);
                str1[4] = 0;
                *(int *)&str2 = WORD(1);
                str2[4] = 0;

                sprintf(strPtr + strlen(strPtr), "%s version %s,  sequence %s %s",
                        space, str1, str2, lfnb);
                *(int *)&str1 = WORD(2);

                sprintf(strPtr + strlen(strPtr),
                        "%s build ID %s,  target ID 0x%X (%s) %s", space, str1,
                        dataPtr->driveBayFWUpdate.targetId,
                        TargetName(dataPtr->driveBayFWUpdate.targetId), crlf);
                sprintf(strPtr + strlen(strPtr),
                        "%s timestamp %02d/%02d/%02d %02d:%02d:%02d %s", space,
                        BCD2Short(dataPtr->driveBayFWUpdate.timeStamp.month),
                        BCD2Short(dataPtr->driveBayFWUpdate.timeStamp.date),
                        BCD2Short(dataPtr->driveBayFWUpdate.timeStamp.year),
                        BCD2Short(dataPtr->driveBayFWUpdate.timeStamp.hours),
                        BCD2Short(dataPtr->driveBayFWUpdate.timeStamp.minutes),
                        BCD2Short(dataPtr->driveBayFWUpdate.timeStamp.seconds), lfnb);

                sprintf(strPtr + strlen(strPtr), "%s crc " HEX8N " %s",
                        space, dataPtr->driveBayFWUpdate.crc, crlf);
                break;
            }

        case LOG_GetCode(LOG_TARGET_SET_PROPERTIES_OP):        /* 0x02F */
            WWNToString(dataPtr->targetSetPropOp.pname, tempStr1);
            WWNToString(dataPtr->targetSetPropOp.nname, tempStr2);
            sprintf(strPtr + strlen(strPtr), "%s TID %d, port %d,  "
                    "options " HEX2 ",  LID %d,  lock " HEX2 " %s",
                    space,
                    dataPtr->targetSetPropOp.tid,
                    dataPtr->targetSetPropOp.port,
                    dataPtr->targetSetPropOp.opt,
                    dataPtr->targetSetPropOp.fcid, dataPtr->targetSetPropOp.lock, crlf);
            sprintf(strPtr + strlen(strPtr), "%s port name %s,  "
                    "node name %s %s", space, tempStr1, tempStr2, crlf);
            sprintf(strPtr + strlen(strPtr), "%s owning SN %d,  "
                    "previous owner SN %d,  cluster ID %d  modifier 0x%x %s",
                    space,
                    dataPtr->targetSetPropOp.owner,
                    dataPtr->targetSetPropOp.powner,
                    dataPtr->targetSetPropOp.cluster,
                    dataPtr->targetSetPropOp.modMask, crlf);
            break;

        case LOG_GetCode(LOG_SES_IO_MOD_PULLED):
        case LOG_GetCode(LOG_SES_EL_FAIL):
        case LOG_GetCode(LOG_SES_LOOPAFAIL):
        case LOG_GetCode(LOG_SES_LOOPBFAIL):
        case LOG_GetCode(LOG_SES_SPEEDMIS):
        case LOG_GetCode(LOG_SES_FWMISMATCH):
        case LOG_GetCode(LOG_SES_PS_FAIL):
        case LOG_GetCode(LOG_SES_FAN_OFF):
        case LOG_GetCode(LOG_SES_PS_DC_OVERVOLT):
        case LOG_GetCode(LOG_SES_PS_DC_UNDERVOLT):
        case LOG_GetCode(LOG_SES_PS_DC_OVERCURR):
        case LOG_GetCode(LOG_SES_PS_OFF):
        case LOG_GetCode(LOG_SES_PS_DC_FAIL):
        case LOG_GetCode(LOG_SES_FAN_FAIL):
        case LOG_GetCode(LOG_SES_DEV_FLT):
        case LOG_GetCode(LOG_SES_PS_OVER_TEMP):
        case LOG_GetCode(LOG_SES_PS_AC_FAIL):
        case LOG_GetCode(LOG_SES_EL_REPORT):
        case LOG_GetCode(LOG_SES_EL_PRESENT):
        case LOG_GetCode(LOG_SES_PS_TEMP_WARN):
        case LOG_GetCode(LOG_SES_DEV_OFF):
        case LOG_GetCode(LOG_SES_DEV_BYPA):
        case LOG_GetCode(LOG_SES_DEV_BYPB):
        case LOG_GetCode(LOG_SES_FAN_ON):
            {
                WWNToString(dataPtr->sesWWNSlot.wwn, tempStr1);
                sprintf(strPtr + strlen(strPtr), "%s WWN %s, "
                        "number " HEX8 ", direction " HEX2 " %s",
                        space,
                        tempStr1,
                        dataPtr->sesWWNSlot.slot, dataPtr->sesWWNSlot.direction, crlf);
            }
            break;

        case LOG_GetCode(LOG_NEW_PATH):
        case LOG_GetCode(LOG_LOST_ALL_PATHS):
            {
                if (dataPtr->newPath.iclPathFlag)
                {
                    sprintf(strPtr + strlen(strPtr), "%s SN %d, cluster %d, "
                            "other path %d, this path(ICL) %d %s",
                            space,
                            dataPtr->newPath.controllerSN,
                            dataPtr->newPath.cluster,
                            dataPtr->newPath.opath, dataPtr->newPath.tpath, crlf);
                }
                else
                {
                    sprintf(strPtr + strlen(strPtr), "%s SN %d, cluster %d, "
                            "other path %d, this path %d %s",
                            space,
                            dataPtr->newPath.controllerSN,
                            dataPtr->newPath.cluster,
                            dataPtr->newPath.opath, dataPtr->newPath.tpath, crlf);
                }
            }
            break;

        case LOG_GetCode(LOG_LOST_PATH):
            {
                sprintf(strPtr + strlen(strPtr), "%s SN %d, cluster %d, "
                        "other path %d, this path %d, %s"
                        "%s DG Status %d, DG EC1 %d, DG EC2 %d %s",
                        space,
                        dataPtr->lostPath.controllerSN,
                        dataPtr->lostPath.cluster,
                        dataPtr->lostPath.opath,
                        dataPtr->lostPath.tpath,
                        crlf, space,
                        dataPtr->lostPath.dgStatus,
                        dataPtr->lostPath.dgErrorCode1,
                        dataPtr->lostPath.dgErrorCode2, crlf);
            }
            break;

        case LOG_GetCode(LOG_DEFRAG_DONE):
            {
                WWNToString(dataPtr->defragDone.wwnPDisk, tempStr1);
                sprintf(strPtr + strlen(strPtr), "%s WWN %s, RID %d, ec " HEX4 " %s",
                        space,
                        tempStr1,
                        dataPtr->defragDone.rid, dataPtr->defragDone.errCode, crlf);
            }
            break;

        case LOG_GetCode(LOG_XSSA_LOG_MESSAGE):
            sprintf(strPtr + strlen(strPtr), "%s type " HEX2 ", eType " HEX2 ", "
                    "flags " HEX2 ", status " HEX2 " %s%s message %s%s",
                    space,
                    dataPtr->xssaMsg.type,
                    dataPtr->xssaMsg.eType,
                    dataPtr->xssaMsg.flags,
                    dataPtr->xssaMsg.status, crlf, space, dataPtr->xssaMsg.message, crlf);
            break;

        case LOG_GetCode(LOG_FW_VERSIONS):     /* 0x036 */
            strPtr += strlen(strPtr);
            strPtr += sprintf(strPtr, "%s%-22s %s%s",
                              space,
                              TargetName(TARG_ID_BE),
                              VerString(dataPtr->firmwareVersions.beRuntime), crlf);
            strPtr += sprintf(strPtr, "%s%-22s %s%s",
                              space,
                              TargetName(TARG_ID_FE),
                              VerString(dataPtr->firmwareVersions.feRuntime), crlf);
            strPtr += sprintf(strPtr, "%s%-22s %s%s",
                              space,
                              TargetName(TARG_ID_CCB),
                              VerString(dataPtr->firmwareVersions.ccbRuntime), crlf);
            if ((i = dataPtr->firmwareVersions.bayCount))
            {
                strPtr += sprintf(strPtr, "%s%s", space, crlf);
                strPtr += sprintf(strPtr, "%sdisk bay %s", space, crlf);
                j = 0;
            }
            while (i--)
            {
                WWNToString(dataPtr->firmwareVersions.bay[j].wwn, tempStr1);
                strPtr += sprintf(strPtr, "%s%-22s %s%s",
                                  space,
                                  tempStr1,
                                  VerString(dataPtr->firmwareVersions.bay[j].version),
                                  crlf);
                j++;
            }
            break;

        case LOG_GetCode(LOG_VLINK_SIZE_CHANGED):      /* 0x037 */
            sprintf(strPtr + strlen(strPtr), "%s source SN %d,  "
                    "source controller cluster %d %s",
                    space,
                    dataPtr->vlinkSizeChanged.sourceCtrlSN,
                    dataPtr->vlinkSizeChanged.sourceCtrlCluster, crlf);

            sprintf(strPtr + strlen(strPtr), "%s source VID %d,  "
                    "destination VID %d %s",
                    space,
                    dataPtr->vlinkSizeChanged.sourceCtrlVDisk,
                    dataPtr->vlinkSizeChanged.destinationCtrlVDisk, crlf);
            break;

        case LOG_GetCode(LOG_PARITY_CHECK_DONE):       /* 0x038 */
            sprintf(strPtr + strlen(strPtr), "%s previous command " HEX8 " %s",
                    space, dataPtr->parityCheckDone.prevctrl, crlf);

            sprintf(strPtr + strlen(strPtr), "%s miscompare count " HEX8 " %s",
                    space, dataPtr->parityCheckDone.miscompares, crlf);

            sprintf(strPtr + strlen(strPtr), "%s pass number " HEX8 " %s",
                    space, dataPtr->parityCheckDone.pass, crlf);

            sprintf(strPtr + strlen(strPtr), "%s start RID " HEX8 " %s",
                    space, dataPtr->parityCheckDone.startraid, crlf);

            sprintf(strPtr + strlen(strPtr), "%s inv func errors " HEX8 " %s",
                    space, dataPtr->parityCheckDone.invfunc, crlf);

            sprintf(strPtr + strlen(strPtr), "%s reserved errors " HEX8 " %s",
                    space, dataPtr->parityCheckDone.reserved, crlf);

            sprintf(strPtr + strlen(strPtr), "%s other errors " HEX8 " %s",
                    space, dataPtr->parityCheckDone.other, crlf);
            break;

        case LOG_GetCode(LOG_FS_UPDATE_FAIL):
        case LOG_GetCode(LOG_FS_UPDATE):
            WWNToString(dataPtr->fsUpdate.wwnTo, tempStr1);
            WWNToString(dataPtr->fsUpdate.wwnFrom, tempStr2);
            sprintf(strPtr + strlen(strPtr), "%s wwnTo %s, wwnFrom %s, "
                    "status " HEX8 " %s",
                    space, tempStr1, tempStr2, dataPtr->fsUpdate.status, crlf);
            break;

        case LOG_GetCode(LOG_INIT_PROC_NVRAM_OP):
            sprintf(strPtr + strlen(strPtr), "%s type " HEX2 ", "
                    "status " HEX2 ", ec " HEX8 " %s",
                    space,
                    dataPtr->initProcNVRAMOp.type,
                    dataPtr->initProcNVRAMOp.status,
                    dataPtr->initProcNVRAMOp.errorCode, crlf);
            break;

        case LOG_GetCode(LOG_SET_SYSTEM_SERIAL_NUM_OP):
            sprintf(strPtr + strlen(strPtr), "%s SN %d, "
                    "status " HEX2 ", ec " HEX8 " %s",
                    space,
                    dataPtr->setSystemSerialNumberOp.serialNumber,
                    dataPtr->setSystemSerialNumberOp.status,
                    dataPtr->setSystemSerialNumberOp.errorCode, crlf);
            break;

        case LOG_GetCode(LOG_VLINK_CREATE_OP): /* 0x041 */
            sprintf(strPtr + strlen(strPtr), "%s VID %d, VDO %d, CID %d, "
                    "status " HEX2 ", ec " HEX8 " %s",
                    space,
                    dataPtr->vlinkCreateOp.vid,
                    dataPtr->vlinkCreateOp.vdo,
                    dataPtr->vlinkCreateOp.cid,
                    dataPtr->vlinkCreateOp.status,
                    dataPtr->vlinkCreateOp.errorCode, crlf);

            /*
             * Copy names into local strings.  Make sure they are properly
             * terminated.
             */
            memcpy(tempStr1, dataPtr->vlinkCreateOp.ctrlName, NAMES_CONTROLLER_LEN_MAX);
            tempStr1[NAMES_CONTROLLER_LEN_MAX] = 0;
            memcpy(tempStr2, dataPtr->vlinkCreateOp.vdName, NAMES_VDEVICE_LEN_MAX);
            tempStr2[NAMES_VDEVICE_LEN_MAX] = 0;

            sprintf(strPtr + strlen(strPtr), "%s remote controller name %s%s",
                    space, tempStr1, crlf);
            sprintf(strPtr + strlen(strPtr), "%s remote VDisk name %s%s",
                    space, tempStr2, crlf);
            break;

        case LOG_GetCode(LOG_SERVER_SET_PROPERTIES_OP):        /* 0x042 */
            sprintf(strPtr + strlen(strPtr), "%s SID %d, priority %d, "
                    "attributes " HEX8 ", status " HEX2 ", ec " HEX8 " %s",
                    space,
                    dataPtr->serverSetPropOp.sid,
                    dataPtr->serverSetPropOp.priority,
                    dataPtr->serverSetPropOp.attr,
                    dataPtr->serverSetPropOp.status,
                    dataPtr->serverSetPropOp.errorCode, crlf);
            break;

        case LOG_GetCode(LOG_ETHERNET_LINK_UP):        /* 0x043 */
        case LOG_GetCode(LOG_ETHERNET_LINK_DOWN):      /* 0x218 */
            EthernetGetLinkStatusString(tempStr1,
                                        dataPtr->ethernetLinkStatusChange.linkStatus.bits.linkStatus,
                                        sizeof(tempStr1));

            EthernetGetWireSpeedString(tempStr2,
                                       dataPtr->ethernetLinkStatusChange.linkStatus.bits.wireSpeed,
                                       sizeof(tempStr2));

            EthernetGetDuplexModeString(tempStr3,
                                        dataPtr->ethernetLinkStatusChange.linkStatus.bits.duplexMode,
                                        sizeof(tempStr3));

            sprintf(strPtr + strlen(strPtr), "%s link status: %s, %s, %s%s",
                    space, tempStr1, tempStr2, tempStr3, crlf);
            break;

        case LOG_GetCode(LOG_NVA_RESYNC_FAILED):       /* 0x045 */
            sprintf(strPtr + strlen(strPtr), "%s status " HEX2 ",  VID %d,  "
                    "VID sda " HEX8 "" HEX8N ",  VID length %d,  %s"
                    "%s RID %d,  sda " HEX8 "" HEX8N ",  length %d %s",
                    space,
                    dataPtr->nvaResyncFailed.status,
                    dataPtr->nvaResyncFailed.vid,
                    (UINT32)((dataPtr->nvaResyncFailed.vsda) >> 32),
                    (UINT32)(dataPtr->nvaResyncFailed.vsda),
                    dataPtr->nvaResyncFailed.vlength,
                    crlf, space,
                    dataPtr->nvaResyncFailed.rid,
                    (UINT32)((dataPtr->nvaResyncFailed.rsda) >> 32),
                    (UINT32)(dataPtr->nvaResyncFailed.rsda),
                    dataPtr->nvaResyncFailed.rlength, crlf);
            break;

            /* 0x050 */
        case LOG_GetCode(LOG_FE_QLOGIC_RESET_OP):
        case LOG_GetCode(LOG_BE_QLOGIC_RESET_OP):
            sprintf(strPtr + strlen(strPtr), "%s channel %d, option " HEX2 ", "
                    "status " HEX2 ", ec " HEX8 " %s",
                    space,
                    dataPtr->feQlogicResetOp.port,
                    dataPtr->feQlogicResetOp.option,
                    dataPtr->feQlogicResetOp.status,
                    dataPtr->feQlogicResetOp.errorCode, crlf);
            break;

        case LOG_GetCode(LOG_VCG_ADD_SLAVE_OP):
            InetToAscii(dataPtr->vcgAddSlaveOp.ipAddress, tempStr1);
            sprintf(strPtr + strlen(strPtr), "%s IP %s, "
                    "status " HEX2 ", ec " HEX8 " %s",
                    space,
                    tempStr1,
                    dataPtr->vcgAddSlaveOp.status,
                    dataPtr->vcgAddSlaveOp.errorCode, crlf);
            break;

        case LOG_GetCode(LOG_VCG_APPLY_LICENSE_OP):
            sprintf(strPtr + strlen(strPtr), "%s DSC %d, MAX %d, "
                    "status " HEX2 ", ec " HEX8 " %s",
                    space,
                    dataPtr->vcgApplyLicense.vcgID,
                    dataPtr->vcgApplyLicense.vcgMaxNumControllers,
                    dataPtr->vcgApplyLicense.status,
                    dataPtr->vcgApplyLicense.errorCode, crlf);
            break;

        case LOG_GetCode(LOG_VCG_SHUTDOWN_OP):
            sprintf(strPtr + strlen(strPtr), "%s status " HEX2 ", ec " HEX8 " %s",
                    space,
                    dataPtr->statusOnly.status, dataPtr->statusOnly.errorCode, crlf);
            break;

        case LOG_GetCode(LOG_PROC_START_IO_OP):
            sprintf(strPtr + strlen(strPtr), "%s status " HEX2 ", ec " HEX8 " %s",
                    space,
                    dataPtr->statusOnly.status, dataPtr->statusOnly.errorCode, crlf);
            break;

        case LOG_GetCode(LOG_PROC_STOP_IO_OP):
            sprintf(strPtr + strlen(strPtr), "%s waitForFlush " HEX2 ", "
                    "waitForShutdown " HEX2 ", "
                    "status " HEX2 ", ec " HEX8 " %s",
                    space,
                    dataPtr->ioStopOp.waitForFlush,
                    dataPtr->ioStopOp.waitForShutdown,
                    dataPtr->ioStopOp.status, dataPtr->ioStopOp.errorCode, crlf);
            break;

        case LOG_GetCode(LOG_VCG_PREPARE_SLAVE_OP):    /* 0x050 */
            InetToAscii(dataPtr->vcgPrepareSlaveOp.ipAddress, tempStr1);
            sprintf(strPtr + strlen(strPtr), "%s SN %d  "
                    "IP address %s  %s",
                    space, dataPtr->vcgPrepareSlaveOp.controllerSN, tempStr1, crlf);
            break;

            /* 0x060 */
        case LOG_GetCode(LOG_ADMIN_SET_IP_OP):
            InetToAscii(dataPtr->ipAddressChange.ipAdr, tempStr1);
            InetToAscii(dataPtr->ipAddressChange.subMaskAdr, tempStr2);
            InetToAscii(dataPtr->ipAddressChange.gatewayAdr, tempStr3);
            sprintf(strPtr + strlen(strPtr), "%s SN %d  "
                    "IP %s, subnet %s, gateway %s, "
                    "status " HEX2 ", ec " HEX8 " %s",
                    space,
                    dataPtr->ipAddressChange.serNum,
                    tempStr1,
                    tempStr2,
                    tempStr3,
                    dataPtr->ipAddressChange.status,
                    dataPtr->ipAddressChange.errorCode, crlf);
            break;

        case LOG_GetCode(LOG_BOOT_CODE_EVENT_INFO):    /* 0x0090 */
        case LOG_GetCode(LOG_BOOT_CODE_EVENT_WARN):    /* 0x0240 */
        case LOG_GetCode(LOG_BOOT_CODE_EVENT_ERROR):   /* 0x0470 */
        case LOG_GetCode(LOG_BOOT_CODE_EVENT_DEBUG):   /* 0x4026 */
            /* No entry */
            break;

            /* CCB Hardware */
        case LOG_GetCode(LOG_CCB_PROCESSOR_HW_INFO):
        case LOG_GetCode(LOG_CCB_PROCESSOR_HW_WARN):
        case LOG_GetCode(LOG_CCB_PROCESSOR_HW_ERROR):
        case LOG_GetCode(LOG_CCB_MEMORY_MODULE_HW_INFO):
        case LOG_GetCode(LOG_CCB_MEMORY_MODULE_HW_WARN):
        case LOG_GetCode(LOG_CCB_MEMORY_MODULE_HW_ERROR):
            sprintf(strPtr + strlen(strPtr), "%s unexpected error code %d%s",
                    space, eventCode, crlf);
//            Logview_BuildDeviceStatusString( (HWM_DEVICE_PROPERTIES_PTR)data,
//                strPtr + strlen(strPtr), space, crlf );
            break;

        case LOG_GetCode(LOG_HARDWARE_MONITOR_INFO):
        case LOG_GetCode(LOG_HARDWARE_MONITOR_WARN):
        case LOG_GetCode(LOG_HARDWARE_MONITOR_ERROR):
        case LOG_GetCode(LOG_HARDWARE_MONITOR_DEBUG):
        case LOG_GetCode(LOG_HARDWARE_MONITOR_STATUS_INFO):
        case LOG_GetCode(LOG_HARDWARE_MONITOR_STATUS_WARN):
        case LOG_GetCode(LOG_HARDWARE_MONITOR_STATUS_ERROR):
        case LOG_GetCode(LOG_CCB_STATUS_INFO):
        case LOG_GetCode(LOG_CCB_STATUS_WARN):
        case LOG_GetCode(LOG_CCB_STATUS_ERROR):
        case LOG_GetCode(LOG_PROC_BOARD_STATUS_INFO):
        case LOG_GetCode(LOG_PROC_BOARD_STATUS_WARN):
        case LOG_GetCode(LOG_PROC_BOARD_STATUS_ERROR):
        case LOG_GetCode(LOG_FE_POWER_SUPPLY_STATUS_INFO):
        case LOG_GetCode(LOG_FE_POWER_SUPPLY_STATUS_WARN):
        case LOG_GetCode(LOG_FE_POWER_SUPPLY_STATUS_ERROR):
        case LOG_GetCode(LOG_BE_POWER_SUPPLY_STATUS_INFO):
        case LOG_GetCode(LOG_BE_POWER_SUPPLY_STATUS_WARN):
        case LOG_GetCode(LOG_BE_POWER_SUPPLY_STATUS_ERROR):
        case LOG_GetCode(LOG_FE_BUFFER_BOARD_STATUS_INFO):
        case LOG_GetCode(LOG_FE_BUFFER_BOARD_STATUS_WARN):
        case LOG_GetCode(LOG_FE_BUFFER_BOARD_STATUS_ERROR):
        case LOG_GetCode(LOG_BE_BUFFER_BOARD_STATUS_INFO):
        case LOG_GetCode(LOG_BE_BUFFER_BOARD_STATUS_WARN):
        case LOG_GetCode(LOG_BE_BUFFER_BOARD_STATUS_ERROR):
            /*
             * Depending on the componentID, decode the extended log informaion.
             * There should be one entry in this switch for each component.
             */
            switch (((HWM_EVENT_PROPERTIES *)data)->componentID)
            {
                case UNKNOWN_ID:
                    sprintf(strPtr + strlen(strPtr),
                            "%s Unknown Component ID:            %d%s", space,
                            ((HWM_EVENT_PROPERTIES *)data)->componentID, crlf);
                    break;

                case HWM_OVERALL_ID:
                case I2C_HARDWARE_OVERALL_ID:
                    sprintf(strPtr + strlen(strPtr),
                            "%s Collection Object Status         %s", space, crlf);
                    break;

                case CCB_PROCESSOR_ID:
                case CCB_BOARD_EEPROM_ID:
                case CCB_MEMORY_MODULE_ID:
                case CCB_MEMORY_MODULE_EEPROM_ID:
                    sprintf(strPtr + strlen(strPtr), "%s unexpected HW ID %d%s",
                            space, ((HWM_EVENT_PROPERTIES *)data)->componentID, crlf);
                    break;

                case CCB_BOARD_STATUS_ID:
                    Logview_BuildCCBStatusString((CCB_STATUS_PTR)data,
                                                 strPtr + strlen(strPtr), space, crlf);
                    break;

                case PROC_BOARD_STATUS_ID:
                    Logview_BuildProcessorBoardStatusString((PROC_BOARD_STATUS_PTR)data,
                                                            strPtr + strlen(strPtr),
                                                            space, crlf);
                    break;

                case POWER_SUPPLY_VOLTAGES_STATUS_ID:
                    Logview_BuildPowerSupplyVoltagesStatusString((POWER_SUPPLY_VOLTAGES_STATUS_PTR)data, strPtr + strlen(strPtr), space, crlf);
                    break;

                case FE_PROCESSOR_STATUS_ID:
                case BE_PROCESSOR_STATUS_ID:
                    Logview_BuildProcessorStatusString((PROC_BOARD_PROCESSOR_STATUS_PTR)
                                                       data, strPtr + strlen(strPtr),
                                                       space, crlf);
                    break;

                case FE_PROCESSOR_TEMPERATURE_STATUS_ID:
                case BE_PROCESSOR_TEMPERATURE_STATUS_ID:
                case FE_BUFFER_BOARD_TEMPERATURE_STATUS_ID:
                case BE_BUFFER_BOARD_TEMPERATURE_STATUS_ID:
                    Logview_BuildTemperatureStatusString((TEMPERATURE_STATUS_PTR)data,
                                                         strPtr + strlen(strPtr), space,
                                                         crlf);
                    break;

                case FE_POWER_SUPPLY_STATUS_ID:
                case BE_POWER_SUPPLY_STATUS_ID:
                    Logview_BuildPowerSupplyStatusString((POWER_SUPPLY_STATUS_PTR)data,
                                                         strPtr + strlen(strPtr), space,
                                                         crlf);
                    break;

                case FE_BUFFER_BOARD_STATUS_ID:
                case BE_BUFFER_BOARD_STATUS_ID:
                    Logview_BuildBufferBoardStatusString((BUFFER_BOARD_STATUS_PTR)data,
                                                         strPtr + strlen(strPtr), space,
                                                         crlf);
                    break;

                case FE_BUFFER_BOARD_BATTERY_STATUS_ID:
                case BE_BUFFER_BOARD_BATTERY_STATUS_ID:
                    Logview_BuildBatteryStatusString((BATTERY_STATUS_PTR)data,
                                                     strPtr + strlen(strPtr), space,
                                                     crlf);
                    break;

                case FE_BUFFER_BOARD_FUEL_GAUGE_STATUS_ID:
                case BE_BUFFER_BOARD_FUEL_GAUGE_STATUS_ID:
                    Logview_BuildFuelGaugeStatusString((FUEL_GAUGE_STATUS_PTR)data,
                                                       strPtr + strlen(strPtr), space,
                                                       crlf);
                    break;

                case FE_BUFFER_BOARD_MAIN_REGULATOR_STATUS_ID:
                case BE_BUFFER_BOARD_MAIN_REGULATOR_STATUS_ID:
                    Logview_BuildMainRegulatorStatusString((MAIN_REGULATOR_STATUS_PTR)
                                                           data, strPtr + strlen(strPtr),
                                                           space, crlf);
                    break;

                case FE_BUFFER_BOARD_CHARGER_STATUS_ID:
                case BE_BUFFER_BOARD_CHARGER_STATUS_ID:
                    Logview_BuildChargerStatusString((CHARGER_STATUS_PTR)data,
                                                     strPtr + strlen(strPtr), space,
                                                     crlf);
                    break;

                case CCB_NVRAM_BATTERY_STATUS_ID:
                    Logview_BuildNVRAMBatteryStatusString((NVRAM_BATTERY_STATUS_PTR)data,
                                                          strPtr + strlen(strPtr), space,
                                                          crlf);
                    break;

                case CCB_BOARD_EEPROM_STATUS_ID:
                case CCB_MEMORY_MODULE_EEPROM_STATUS_ID:
                case CHASSIS_EEPROM_STATUS_ID:
                case PROC_BOARD_EEPROM_STATUS_ID:
                case FE_POWER_SUPPLY_ASM_EEPROM_STATUS_ID:
                case FE_POWER_SUPPLY_INT_EEPROM_STATUS_ID:
                case BE_POWER_SUPPLY_ASM_EEPROM_STATUS_ID:
                case BE_POWER_SUPPLY_INT_EEPROM_STATUS_ID:
                case FE_BUFFER_BOARD_EEPROM_STATUS_ID:
                case BE_BUFFER_BOARD_EEPROM_STATUS_ID:
                    sprintf(strPtr + strlen(strPtr), "%s unexpected HW ID %d%s",
                            space, ((HWM_EVENT_PROPERTIES *)data)->componentID, crlf);
                    break;

                default:
                    sprintf(strPtr + strlen(strPtr),
                            "%s Undefined Component              %s", space, crlf);
                    break;
            }
            break;

        case LOG_GetCode(LOG_MIRROR_CAPABLE):
            sprintf(strPtr + strlen(strPtr), "%s SN %d %s",
                    space, dataPtr->cacheMirrorCapable.controllerSN, crlf);
            break;

            /*
             * ----- Warning Events -----
             */
        case LOG_GetCode(LOG_NO_HS_LOCATION_CODE_MATCH):
            sprintf(strPtr + strlen(strPtr),
                    "%s NO HotSpare exists matching location code %d %s", space,
                    dataPtr->noHsLocationMatch.locationCode, crlf);
            break;

        case LOG_GetCode(LOG_DEVICE_FAIL_HS):  /* 0x200 */
            WWNToString(dataPtr->deviceFailHS.wwn, tempStr1);
            WWNToString(dataPtr->deviceFailHS.hswwn, tempStr2);
            sprintf(strPtr + strlen(strPtr), "%s Failing drive: WWN %s, port %d, "
                    "LUN %d, LID %d, PID %d%s",
                    space,
                    tempStr1,
                    dataPtr->deviceFailHS.channel,
                    dataPtr->deviceFailHS.lun,
                    dataPtr->deviceFailHS.lid, dataPtr->deviceFailHS.pid, crlf);

            sprintf(strPtr + strlen(strPtr), "%s Hotspare drive: WWN %s, port %d, "
                    "LUN %d, LID %d, PID %d%s",
                    space,
                    tempStr2,
                    dataPtr->deviceFailHS.hschannel,
                    dataPtr->deviceFailHS.hslun,
                    dataPtr->deviceFailHS.hsid, dataPtr->deviceFailHS.hspid, crlf);
            break;

        case LOG_GetCode(LOG_SMART_EVENT):
        case LOG_GetCode(LOG_DRV_FLT):
            WWNToString(dataPtr->smartEvent.wwn, tempStr1);
            sprintf(strPtr + strlen(strPtr), "%s WWN %s, port %d, "
                    "LUN %d, LID %d%s",
                    space,
                    tempStr1,
                    dataPtr->smartEvent.channel,
                    dataPtr->smartEvent.lun, dataPtr->smartEvent.lid, crlf);
            sprintf(strPtr + strlen(strPtr), "%s sense: " HEX8N " " HEX8N " "
                    "" HEX8N " " HEX8N " %s",
                    space, SWAPWORD(8), SWAPWORD(9), SWAPWORD(10), SWAPWORD(11), lfnb);
            sprintf(strPtr + strlen(strPtr), "%s        " HEX8N " " HEX8N " "
                    "" HEX8N " " HEX8N " %s",
                    space, SWAPWORD(12), SWAPWORD(13), SWAPWORD(14), SWAPWORD(15), crlf);
            break;

        case LOG_GetCode(LOG_CACHE_FLUSH_FAILED):
            sprintf(strPtr + strlen(strPtr), "%s VID %d, ec " HEX2 " %s",
                    space,
                    dataPtr->cacheFlushFailed.vid,
                    dataPtr->cacheFlushFailed.errcode, crlf);
            break;

        case LOG_GetCode(LOG_SES_TEMP_WARN):   /* 0x210 */
        case LOG_GetCode(LOG_SES_TEMP_FAIL):   /* 0x412 */
            WWNToString(dataPtr->sesTemp.wwn, tempStr1);
            sprintf(strPtr + strlen(strPtr), "%s WWN %s,  sensor %d %s",
                    space, tempStr1, dataPtr->sesTemp.slot, crlf);
            sprintf(strPtr + strlen(strPtr), "%s temperature %d %s",
                    space, dataPtr->sesTemp.temp, crlf);
            break;

        case LOG_GetCode(LOG_SES_VOLTAGE_HI_WARN):     /* 0x20E */
        case LOG_GetCode(LOG_SES_VOLTAGE_LO_WARN):     /* 0x20F */
        case LOG_GetCode(LOG_SES_VOLTAGE_HI):  /* 0x413 */
        case LOG_GetCode(LOG_SES_VOLTAGE_LO):  /* 0x414 */
            WWNToString(dataPtr->sesVoltage.wwn, tempStr1);
            sprintf(strPtr + strlen(strPtr), "%s WWN %s,  sensor %d %s",
                    space, tempStr1, dataPtr->sesVoltage.slot, crlf);
            sprintf(strPtr + strlen(strPtr), "%s voltage (10mv) %d %s",
                    space, dataPtr->sesVoltage.voltage, crlf);
            break;

            /* 0x210 */
        case LOG_GetCode(LOG_PHY_ACTION):
            sprintf(strPtr + strlen(strPtr), "%s proc %d,  port %d, "
                    "PID %d handle " HEX4 " lun %d, %s",
                    space,
                    dataPtr->phyAction.proc,
                    dataPtr->phyAction.port,
                    dataPtr->phyAction.pid,
                    dataPtr->phyAction.handle, dataPtr->phyAction.lun, crlf);
            break;

        case LOG_GetCode(LOG_PORT_UP):
        case LOG_GetCode(LOG_PORT_EVENT):
        case LOG_GetCode(LOG_PORT_INIT_FAILED):
        case LOG_GetCode(LOG_ISP_CHIP_RESET):
        case LOG_GetCode(LOG_FRAMEDROPPED):
        case LOG_GetCode(LOG_ISP_FATAL):
            sprintf(strPtr + strlen(strPtr), "%s proc %d,  port %d, "
                    "count %d, reason " HEX8 " %s",
                    space,
                    dataPtr->portEvent.proc,
                    dataPtr->portEvent.port,
                    dataPtr->portEvent.count, dataPtr->portEvent.reason, crlf);
            break;

        case LOG_GetCode(LOG_IPC_LINK_UP):
        case LOG_GetCode(LOG_IPC_LINK_DOWN):
            sprintf(strPtr + strlen(strPtr), "%s DetectedBySN %d, "
                    "DestinationSN %d, "
                    "LinkType " HEX2 ", LinkError " HEX8 " %s",
                    space,
                    dataPtr->ipcLinkUp.DetectedBySN,
                    dataPtr->ipcLinkUp.DestinationSN,
                    dataPtr->ipcLinkUp.LinkType, dataPtr->ipcLinkUp.LinkError, crlf);
            break;

        case LOG_GetCode(LOG_SINGLE_PATH):
            WWNToString(dataPtr->singlePath.wwn, tempStr1);
            sprintf(strPtr + strlen(strPtr), "%s WWN %s %s", space, tempStr1, crlf);
            break;

        case LOG_GetCode(LOG_HOTSPARE_INOP):
            WWNToString(dataPtr->hotspareInop.wwn, tempStr1);
            sprintf(strPtr + strlen(strPtr), "%s WWN %s, PID %d %s",
                    space, tempStr1, dataPtr->hotspareInop.pid, crlf);
            break;

        case LOG_GetCode(LOG_VCG_SHUTDOWN_WARN):
            sprintf(strPtr + strlen(strPtr), "%s status " HEX2 ", ec " HEX8 " %s",
                    space,
                    dataPtr->statusOnly.status, dataPtr->statusOnly.errorCode, crlf);
            break;

        case LOG_GetCode(LOG_CACHE_MIRROR_FAILED):     /* 0x215 */
            sprintf(strPtr + strlen(strPtr), "%s SN %d,  status %d %s",
                    space,
                    dataPtr->cacheMirrorFailed.controllerSN,
                    dataPtr->cacheMirrorFailed.ilt_status, crlf);
            sprintf(strPtr + strlen(strPtr), "%s DG request status %d,  "
                    "DG status %d %s",
                    space,
                    dataPtr->cacheMirrorFailed.drp_status,
                    dataPtr->cacheMirrorFailed.dg_status, crlf);
            sprintf(strPtr + strlen(strPtr), "%s ec1 %d,  ec2 %d %s",
                    space,
                    dataPtr->cacheMirrorFailed.dg_ec1,
                    dataPtr->cacheMirrorFailed.dg_ec2, crlf);
            break;

        case LOG_GetCode(LOG_SES_CURRENT_HI_WARN):     /* 0x21B */
        case LOG_GetCode(LOG_SES_CURRENT_HI):  /* 0x43D */
            WWNToString(dataPtr->sesCurrent.wwn, tempStr1);
            sprintf(strPtr + strlen(strPtr), "%s WWN %s,  sensor %d %s",
                    space, tempStr1, dataPtr->sesCurrent.slot, crlf);
            sprintf(strPtr + strlen(strPtr), "%s current (10ma) %d %s",
                    space, dataPtr->sesCurrent.current, crlf);
            break;

            /*
             * ----- Error Events -----
             */
            /* 0x400 */
        case LOG_GetCode(LOG_DEVICE_FAIL_NO_HS):       /* 0x400 */
            WWNToString(dataPtr->deviceFailNoHS.wwn, tempStr1);
            sprintf(strPtr + strlen(strPtr),
                    "%s WWN %s, port %d,  LUN %d,  LID %d,  PID %d,  RID %d,  VID %d%s",
                    space,
                    tempStr1,
                    dataPtr->deviceFailNoHS.channel,
                    dataPtr->deviceFailNoHS.lun,
                    dataPtr->deviceFailNoHS.lid,
                    dataPtr->deviceFailNoHS.pid,
                    dataPtr->deviceFailNoHS.rid, dataPtr->deviceFailNoHS.vid, crlf);
            break;

        case LOG_GetCode(LOG_SERIAL_WRONG):
            WWNToString(dataPtr->serialWrong.wwn, tempStr1);
            sprintf(strPtr + strlen(strPtr), "%s WWN %s, SN %d, "
                    "port %d, LUN %d LID %d %s",
                    space,
                    tempStr1,
                    dataPtr->serialWrong.serial,
                    dataPtr->serialWrong.channel,
                    dataPtr->serialWrong.lun, dataPtr->serialWrong.lid, crlf);
            break;

        case LOG_GetCode(LOG_NVA_BAD):
            sprintf(strPtr + strlen(strPtr), "%s processor " HEX2 ", "
                    "part " HEX2 " %s",
                    space, dataPtr->nvaBad.processor, dataPtr->nvaBad.part, crlf);
            break;

        case LOG_GetCode(LOG_CACHE_DRAM_FAIL):
            sprintf(strPtr + strlen(strPtr), "%s status " HEX2 " %s",
                    space, dataPtr->cacheDramFail.status, crlf);
            break;

        case LOG_GetCode(LOG_CACHE_RECOVER_FAIL):      /* 0x40B */
            sprintf(strPtr + strlen(strPtr), "%s LBA " HEX8 " " HEX8N ",  "
                    "length " HEX8 " %s", space, WORD(0), WORD(1), WORD(2), crlf);
            sprintf(strPtr + strlen(strPtr), "%s count %d,  status " HEX2 " %s",
                    space, BYTE(13), BYTE(12), crlf);
            break;

        case LOG_GetCode(LOG_COPY_FAILED):     /* 0x40C */
            sprintf(strPtr + strlen(strPtr), "%s function %d,  status " HEX4 ",  "
                    "percent %d %s", space, BYTE(1), BYTE(0), BYTE(2), crlf);
            break;

        case LOG_GetCode(LOG_VID_RECOVERY_FAIL):
            sprintf(strPtr + strlen(strPtr), "%s VID %d, count %d %s",
                    space,
                    dataPtr->vidRecoveryFail.vid, dataPtr->vidRecoveryFail.count, crlf);
            break;

        case LOG_GetCode(LOG_FOREIGN_PCI):     /* 0x416 */
            strPtr += strlen(strPtr);
            sprintf(strPtr, "%s PCI Slot %d,  processor %s,  vendor " HEX4 ",  "
                    "device " HEX4 " %s",
                    space, dataPtr->foreignPci.channel,
                    (dataPtr->foreignPci.proc) ? "Storage side" : "Host side",
                    dataPtr->foreignPci.vendor, dataPtr->foreignPci.device, crlf);
            break;

        case LOG_GetCode(LOG_WC_SEQNO_BAD):    /* 0x418 */
            sprintf(strPtr + strlen(strPtr), "%s cacheId " HEX2 ",  sysSeq " HEX8 "  "
                    "seq " HEX8 " %s",
                    space, dataPtr->wcSeqNoBad.cacheId,
                    dataPtr->wcSeqNoBad.sysSeq, dataPtr->wcSeqNoBad.seq, crlf);
            break;


        case LOG_GetCode(LOG_INVALID_TAG):     /* 0x41A */
            sprintf(strPtr + strlen(strPtr), "%s status " HEX8 "," HEX8 " %s",
                    space,
                    dataPtr->invalidTag.status1, dataPtr->invalidTag.status2, crlf);
            sprintf(strPtr + strlen(strPtr), "%s tag information %s", space, crlf);
            sprintf(strPtr + strlen(strPtr), "%s " HEX8 " " HEX8 " " HEX8 " " HEX8 " %s",
                    space, dataPtr->invalidTag.tagData[0],
                    dataPtr->invalidTag.tagData[1],
                    dataPtr->invalidTag.tagData[2], dataPtr->invalidTag.tagData[3], crlf);
            sprintf(strPtr + strlen(strPtr), "%s " HEX8 " " HEX8 " " HEX8 " " HEX8 " %s",
                    space, dataPtr->invalidTag.tagData[4],
                    dataPtr->invalidTag.tagData[5],
                    dataPtr->invalidTag.tagData[6], dataPtr->invalidTag.tagData[7], crlf);
            sprintf(strPtr + strlen(strPtr), "%s " HEX8 " " HEX8 " " HEX8 " " HEX8 " %s",
                    space, dataPtr->invalidTag.tagData[8],
                    dataPtr->invalidTag.tagData[9],
                    dataPtr->invalidTag.tagData[10],
                    dataPtr->invalidTag.tagData[11], crlf);
            sprintf(strPtr + strlen(strPtr), "%s " HEX8 " " HEX8 " " HEX8 " " HEX8 " %s",
                    space, dataPtr->invalidTag.tagData[12],
                    dataPtr->invalidTag.tagData[13],
                    dataPtr->invalidTag.tagData[14],
                    dataPtr->invalidTag.tagData[15], crlf);
            break;

            /* 0x420 */
        case LOG_GetCode(LOG_RAID_ERROR):
            WWNToString(dataPtr->raidError.wwnPdisk, tempStr1);
            sprintf(strPtr + strlen(strPtr), "%s WWN %s, RID %d, "
                    "VID %d, PID %d %s",
                    space,
                    tempStr1,
                    dataPtr->raidError.rid,
                    dataPtr->raidError.vid, dataPtr->raidError.pid, crlf);
            break;

        case LOG_GetCode(LOG_FILEIO_ERR):      /* 0x422 */
        case LOG_GetCode(LOG_FILEIO_DEBUG):    /* 0x401C */
            sprintf(strPtr + strlen(strPtr), "%s rc 0x%X, status 0x%hhX %s"
                    "%s wr_good %d, wr_err %d %s",
                    space,
                    dataPtr->fileioErr.rc,
                    dataPtr->fileioErr.status,
                    crlf, space,
                    dataPtr->fileioErr.wr_good, dataPtr->fileioErr.wr_err, crlf);
            break;

        case LOG_GetCode(LOG_PROC_NOT_READY):  /* 0x423 */
            sprintf(strPtr + strlen(strPtr),
                    "%s Storage side ready %d,  Host side ready %d %s", space,
                    dataPtr->procNotReady.beReady, dataPtr->procNotReady.feReady, crlf);
            break;


        case LOG_GetCode(LOG_ILLEGAL_ELECTION_STATE):  /* 0x42B  */
        case LOG_GetCode(LOG_ELECTION_STATE_CHANGE):   /* 0x4004 */
            EL_GetElectionStateString(tempStr1,
                                      dataPtr->electionStateTransition.fromState,
                                      sizeof(tempStr1));

            EL_GetElectionStateString(tempStr2,
                                      dataPtr->electionStateTransition.toState,
                                      sizeof(tempStr2));

            sprintf(strPtr + strlen(strPtr), "%s From: %s, To: %s %s",
                    space, tempStr1, tempStr2, crlf);
            break;

        case LOG_GetCode(LOG_LOG_FAILURE_EVENT):
        case LOG_GetCode(LOG_LOG_FAILURE_EVENT_INFO):
        case LOG_GetCode(LOG_LOG_FAILURE_EVENT_WARN):
            sprintf(strPtr + strlen(strPtr), "%s action_location " HEX2 ", "
                    "action " HEX2 ", type " HEX8 " %s",
                    space,
                    dataPtr->failureEvent.action_location,
                    dataPtr->failureEvent.action, dataPtr->failureEvent.event.Type, crlf);
            switch (dataPtr->failureEvent.event.Type)
            {
                case IPC_FAILURE_TYPE_INTERFACE_FAILED:
                    sprintf(strPtr + strlen(strPtr), "%s Detected by %d , "
                            "Failed Interface %d, Controller SN %d, "
                            "Failure type " HEX8 " %s",
                            space,
                            dataPtr->failureEvent.event.FailureData.InterfaceFailure.DetectedBySN,
                            dataPtr->failureEvent.event.FailureData.InterfaceFailure.FailedInterfaceID,
                            dataPtr->failureEvent.event.FailureData.InterfaceFailure.ControllerSN,
                            dataPtr->failureEvent.event.FailureData.InterfaceFailure.InterfaceFailureType,
                            crlf);
                    break;

                case IPC_FAILURE_TYPE_COMMUNICATIONS_FAILED:
                    sprintf(strPtr + strlen(strPtr), "%s Detected by %d , "
                            "Destination SN %d, LinkType " HEX2 ", "
                            "LinkError " HEX8 " %s",
                            space,
                            dataPtr->failureEvent.event.FailureData.CommunicationsLinkFailure.DetectedBySN,
                            dataPtr->failureEvent.event.FailureData.CommunicationsLinkFailure.DestinationSN,
                            dataPtr->failureEvent.event.FailureData.CommunicationsLinkFailure.LinkType,
                            dataPtr->failureEvent.event.FailureData.CommunicationsLinkFailure.LinkError,
                            crlf);
                    break;

                case IPC_FAILURE_TYPE_CONTROLLER_FAILED:
                    sprintf(strPtr + strlen(strPtr), "%s Detected by %d , "
                            "Failed Controller %d, "
                            "ErrorType " HEX8 " %s",
                            space,
                            dataPtr->failureEvent.event.FailureData.ControllerFailure.DetectedBySN,
                            dataPtr->failureEvent.event.FailureData.ControllerFailure.FailedControllerSN,
                            dataPtr->failureEvent.event.FailureData.ControllerFailure.ErrorType,
                            crlf);
                    break;

                case IPC_FAILURE_TYPE_ELECTION_COMPLETE:
                    sprintf(strPtr + strlen(strPtr), "%s Election State " HEX8 " %s",
                            space,
                            dataPtr->failureEvent.event.FailureData.ElectionComplete.ElectionState,
                            crlf);
                    break;

                case IPC_FAILURE_TYPE_BATTERY_FAILURE:
                    sprintf(strPtr + strlen(strPtr), "%s Detected by %d , "
                            "Battery Bank %d, "
                            "Battery State " HEX8 " %s",
                            space,
                            dataPtr->failureEvent.event.FailureData.BatteryFailure.DetectedBySN,
                            dataPtr->failureEvent.event.FailureData.BatteryFailure.BatteryBank,
                            dataPtr->failureEvent.event.FailureData.BatteryFailure.BatteryState,
                            crlf);
                    break;
            }
            break;

        case LOG_GetCode(LOG_RM_EVENT_INFO):
        case LOG_GetCode(LOG_RM_WARN):
        case LOG_GetCode(LOG_RM_ERROR):
            sprintf(strPtr + strlen(strPtr), "%s rm_logging_code " HEX2 " %s%s"
                    "rm_log_param_1 " HEX8 " %s%s"
                    "rm_log_param_2 " HEX8 " %s%s"
                    "rm_log_param_3 " HEX8 " %s",
                    space,
                    dataPtr->rmEvent.rm_logging_code,
                    crlf, space,
                    dataPtr->rmEvent.rm_log_param_1,
                    crlf, space,
                    dataPtr->rmEvent.rm_log_param_2,
                    crlf, space, dataPtr->rmEvent.rm_log_param_3, crlf);
            break;

        case LOG_GetCode(LOG_BE_INITIATOR):    /* 0x433 */
            WWNToString(dataPtr->beinitiator.wwn, tempStr1);
            sprintf(strPtr + strlen(strPtr), "%s port %d,  LID %d,  WWN %s %s",
                    space,
                    dataPtr->beinitiator.channel,
                    dataPtr->beinitiator.lid, tempStr1, crlf);
            break;

        case LOG_GetCode(LOG_SERIAL_MISMATCH): /* 0x42D */
            sprintf(strPtr + strlen(strPtr), "%s Host side SN %d,  Storage side SN %d %s",
                    space,
                    dataPtr->FeBeSerialMismatch.feserial,
                    dataPtr->FeBeSerialMismatch.beserial, crlf);
            break;

        case LOG_GetCode(LOG_SNAPSHOT_TAKEN):  /* 0x067 */
        case LOG_GetCode(LOG_SNAPSHOT_RESTORED):       /* 0x062 */
        case LOG_GetCode(LOG_SNAPSHOT_RESTORE_FAILED): /* 0x446 */
            sprintf(strPtr + strlen(strPtr),
                    "%s Sequence#: %u%s"
                    "%s Index#: %u%s"
                    "%s Timestamp: %02d/%02d/%02d %02d:%02d:%02d%s"
                    "%s Status: %d%s"
                    "%s Available FIDS: %s %s %s%s"
                    "%s Restored FIDS:  %s %s %s%s"
                    "%s Description: %s%s",
                    space,
                    dataPtr->snapshotTaken.number,
                    crlf,
                    space,
                    dataPtr->snapshotTaken.index,
                    crlf,
                    space,
                    BCD2Short(dataPtr->snapshotTaken.time.month),
                    BCD2Short(dataPtr->snapshotTaken.time.date),
                    BCD2Short(dataPtr->snapshotTaken.time.year),
                    BCD2Short(dataPtr->snapshotTaken.time.hours),
                    BCD2Short(dataPtr->snapshotTaken.time.minutes),
                    BCD2Short(dataPtr->snapshotTaken.time.seconds),
                    crlf,
                    space,
                    dataPtr->snapshotTaken.status,
                    crlf,
                    space,
                    dataPtr->snapshotTaken.avlFlags & SNAPSHOT_FLAG_MASTER_CONFIG ?
                    "MasterCFG" : "",
                    dataPtr->snapshotTaken.avlFlags & SNAPSHOT_FLAG_CTRL_MAP ?
                    "CTLMap" : "",
                    dataPtr->snapshotTaken.avlFlags & SNAPSHOT_FLAG_BE_NVRAM ?
                    "BENVRAM" : "",
                    crlf,
                    space,
                    dataPtr->snapshotTaken.rstFlags & SNAPSHOT_FLAG_MASTER_CONFIG ?
                    "MasterCFG" : "",
                    dataPtr->snapshotTaken.rstFlags & SNAPSHOT_FLAG_CTRL_MAP ?
                    "CTLMap" : "",
                    dataPtr->snapshotTaken.rstFlags & SNAPSHOT_FLAG_BE_NVRAM ?
                    "BENVRAM" : "",
                    crlf, space, dataPtr->snapshotTaken.description, crlf);
            break;

        case LOG_GetCode(LOG_SCSI_TIMEOUT):
            WWNToString(dataPtr->SCSITimeout.wwn, tempStr1);
            sprintf(strPtr + strlen(strPtr), "%s WWN %s  port %d  LID %d  ",
                    space,
                    tempStr1, dataPtr->SCSITimeout.channel, dataPtr->SCSITimeout.lid);
            sprintf(strPtr + strlen(strPtr), "%s CDB: " HEX8N " " HEX8N " " HEX8N " "
                    "" HEX8N " %s",
                    space, SWAPWORD(4), SWAPWORD(5), SWAPWORD(6), SWAPWORD(7), crlf);

            break;

        case LOG_GetCode(LOG_DRIVE_DELAY):
            WWNToString(dataPtr->driveDelay.wwn, tempStr1);
            sprintf(strPtr + strlen(strPtr), "%s WWN %s %s", space, tempStr1, crlf);
            break;

            /*
             * ----- Debug Events -----
             */
        case LOG_GetCode(LOG_CHANGE_LED):      /* 0x4000 */
            WWNToString(dataPtr->changeLed.wwn, tempStr1);
            sprintf(strPtr + strlen(strPtr), "%s state " HEX2 ",  WWN %s %s",
                    space, dataPtr->changeLed.state, tempStr1, crlf);
            break;

        case LOG_GetCode(LOG_CHANGE_NAME):
            memcpy(tempStr1, dataPtr->vlinkChangeName.ctrlName, NAMES_CONTROLLER_LEN_MAX);
            memcpy(tempStr2, dataPtr->vlinkChangeName.vdName, NAMES_VDEVICE_LEN_MAX);
            sprintf(strPtr + strlen(strPtr), "%s controller name: %s %s",
                    space, tempStr1, crlf);
            sprintf(strPtr + strlen(strPtr), "%s device name: %s %s",
                    space, tempStr2, crlf);
            break;

        case LOG_GetCode(LOG_RESYNC_DONE):
            sprintf(strPtr + strlen(strPtr), "%s validnva %d, "
                    "attempts %d, errors %d %s",
                    space,
                    dataPtr->resyncDone.validnva,
                    dataPtr->resyncDone.attempts, dataPtr->resyncDone.errors, crlf);
            break;

    /** Fix this - see defect
        case LOG_GetCode(LOG_GET_LIST_ERROR):            0x4003
            sprintf(strPtr + strlen(strPtr),"%s CpsStatus= "HEX2"  MrpStatus= "HEX2" %s",
                      space, SHORT(0), SHORT(1), crlf);
            sprintf(strPtr + strlen(strPtr),"%s listType= "HEX2"# items in list= "HEX2" %s",
                      space, SHORT(2), SHORT(3), crlf);
            break;
    */

        case LOG_GetCode(LOG_MAG_DRIVER_ERR):  /* 0x4007 */
        case LOG_GetCode(LOG_HOST_NONSENSE):   /* 0x4008 */
        case LOG_GetCode(LOG_HOST_QLOGIC_ERR): /* 0x4009 */
            WWNToString(dataPtr->magDriverErr.wwn, tempStr1);
            sprintf(strPtr + strlen(strPtr), "%s ec " HEX2 ", VID %d, "
                    "port %d, tid %d, WWN %s%s",
                    space,
                    dataPtr->magDriverErr.errcode,
                    dataPtr->magDriverErr.vid,
                    dataPtr->magDriverErr.channel,
                    dataPtr->magDriverErr.tid, tempStr1, crlf);
            break;

        case LOG_GetCode(LOG_SINGLE_BIT_ECC):  /* 0x400A */
            sprintf(strPtr + strlen(strPtr), "%s proc " HEX2 ",  type " HEX2 ",  "
                    "memloc " HEX8 ",  reg " HEX8 " %s",
                    space,
                    dataPtr->singleBitECC.proc,
                    dataPtr->singleBitECC.type,
                    dataPtr->singleBitECC.errorAddress,
                    dataPtr->singleBitECC.elogRegister, crlf);
            sprintf(strPtr + strlen(strPtr), "%s singlecnt %d,  multicnt %d,  "
                    "manycnt %d,  mcecnt %d %s",
                    space,
                    dataPtr->singleBitECC.singleBitCount,
                    dataPtr->singleBitECC.multiBitCount,
                    dataPtr->singleBitECC.totalMultiCount,
                    dataPtr->singleBitECC.totalCount, crlf);
            break;

        case LOG_GetCode(LOG_HOST_IMED_NOTIFY):        /* 0x400E */
            sprintf(strPtr + strlen(strPtr), "%s Qstatus " HEX2 "  flags " HEX2 " %s",
                    space,
                    dataPtr->hostImedNotify.qstatus,
                    dataPtr->hostImedNotify.taskflags, crlf);
            break;

        case LOG_GetCode(LOG_HOST_SENSE_DATA): /* 0x400F */
            sprintf(strPtr + strlen(strPtr), "%s sense data: " HEX8N " " HEX8N " "
                    "" HEX8N " " HEX8N " " HEX8N " %s",
                    space, SWAPWORD(4), SWAPWORD(5), SWAPWORD(6), SWAPWORD(7),
                    SWAPWORD(8), crlf);
            break;

        case LOG_GetCode(LOG_ZONE_INQUIRY):
            WWNToString(dataPtr->zoneInquiry.wwn, tempStr1);
            sprintf(strPtr + strlen(strPtr), "%s WWN %s, TID %d, "
                    "owner %d, port %d %s",
                    space,
                    tempStr1,
                    dataPtr->zoneInquiry.tid,
                    dataPtr->zoneInquiry.owner, dataPtr->zoneInquiry.channel, crlf);
            break;

        case LOG_GetCode(LOG_TASKTOOLONG):
            sprintf(strPtr + strlen(strPtr), "%s time %d(msec), caller " HEX8 ", "
                    "previous " HEX8 ", pcb " HEX8 " %s",
                    space,
                    (dataPtr->taskTooLong.time * 128),
                    dataPtr->taskTooLong.caller,
                    dataPtr->taskTooLong.prev, dataPtr->taskTooLong.pcb, crlf);
            break;

        case LOG_GetCode(LOG_PHY_RETRY):       /* 0x401E */
            sprintf(strPtr + strlen(strPtr), "%s %u retries remaining %s",
                    space, dataPtr->longSCSIEvent.retry, lfnb);
            /*
             * Fall through to the next case because phsy retry
             * have the same information as the SCSI events.
             */

        case LOG_GetCode(LOG_SHORT_SCSI_EVENT):        /* 0x400C */
        case LOG_GetCode(LOG_LONG_SCSI_EVENT): /* 0x400D */
        case LOG_GetCode(LOG_SPINUP_FAILED):
            {
                WWNToString(dataPtr->shortSCSIEvent.wwn, tempStr1);
                sprintf(strPtr + strlen(strPtr),
                        "%s WWN %s  Port %d  PID %d LID %d  LUN %d ", space, tempStr1,
                        dataPtr->longSCSIEvent.channel, dataPtr->longSCSIEvent.pid,
                        dataPtr->longSCSIEvent.lid, dataPtr->longSCSIEvent.lun);
                sprintf(strPtr + strlen(strPtr),
                        "status " HEX2 "  PRP st " HEX2 "  " "Q st " HEX2 "  %s",
                        dataPtr->longSCSIEvent.scsistat, dataPtr->longSCSIEvent.prpstat,
                        dataPtr->longSCSIEvent.qstat, lfnb);
                sprintf(strPtr + strlen(strPtr),
                        "%s CDB: " HEX8N " " HEX8N " " HEX8N " " "" HEX8N " %s", space,
                        SWAPWORD(5), SWAPWORD(6), SWAPWORD(7), SWAPWORD(8), crlf);

                if (dataPtr->longSCSIEvent.scsistat == 2)
                {
                    sprintf(strPtr + strlen(strPtr), "%s sense: " HEX8N " " HEX8N " "
                            "" HEX8N " " HEX8N " %s",
                            space,
                            SWAPWORD(9), SWAPWORD(10), SWAPWORD(11), SWAPWORD(12), lfnb);
                    sprintf(strPtr + strlen(strPtr), "%s        " HEX8N " " HEX8N " "
                            "" HEX8N " " HEX8N " %s",
                            space,
                            SWAPWORD(13), SWAPWORD(14), SWAPWORD(15), SWAPWORD(16), crlf);
                }
            }
            break;

        case LOG_GetCode(LOG_PORTDBCHANGED):   /* 0x2C */

            if (dataPtr->portDbChanged.lid == 0xFFFF)
            {
                sprintf(strPtr + strlen(strPtr), "%s Global event", space);
            }
            else
            {
                sprintf(strPtr + strlen(strPtr), "%s Loop ID " HEX4 "", space,
                        dataPtr->portDbChanged.lid);
            }

            if (dataPtr->portDbChanged.state == 4)
            {
                sprintf(strPtr + strlen(strPtr), " PLOGI complete ");
            }
            else if (dataPtr->portDbChanged.state == 6)
            {
                sprintf(strPtr + strlen(strPtr), " PRLI complete ");
            }
            else if (dataPtr->portDbChanged.state == 7)
            {
                sprintf(strPtr + strlen(strPtr), " Port Logged Out ");
            }
            else
            {
                sprintf(strPtr + strlen(strPtr), " login state " HEX4 " ",
                        dataPtr->portDbChanged.state);
            }

            sprintf(strPtr + strlen(strPtr), "%s", crlf);

            /*
             * Fall through to the next case because port database changes
             * have the same information as the loopup cases.
             */

        case LOG_GetCode(LOG_LOOPUP):  /* 0x2B */

            if (LOG_GetCode(eventCode) == LOG_GetCode(LOG_LOOPUP) && dataPtr->loopUp.proc)
            {
                sprintf(strPtr + strlen(strPtr), "%s mode is %s%s",
                        space, BIT_TEST(dataPtr->loopUp.flags, PORT_FABRIC_MODE_BIT) ?
              "FABRIC":"LOOP",
                        crlf);
            }

            sprintf(strPtr + strlen(strPtr), "%s", space);
            i = 8;
            j = MAX_LOG_MESSAGE_LINES;

            while ((i < logPtr->length) && (j > 0))
            {
                /*
                 * Add a byte of data to the print string
                 */
                sprintf(strPtr + strlen(strPtr), " " HEX2N "", data[i]);
                ++i;

                /*
                 * Every 24 bytes of data, add a newline and indentation.
                 */
                if (((i - 8) % 24) == 0 && i < logPtr->length)
                {
                    sprintf(strPtr + strlen(strPtr), "%s%s", crlf, space);

                    /*
                     * Decrement the line count
                     */
                    --j;
                }
            }
            /*
             * Add a final linefeed
             */
            sprintf(strPtr + strlen(strPtr), "%s", crlf);
            break;

        case LOG_GetCode(LOG_MB_FAILED):
            /*
             * Add a space to start next line.
             */
            sprintf(strPtr + strlen(strPtr), "%s", space);

            /*
             * Get mask of input mailbox registers.
             */
            j = dataPtr->mbxfailed.iregs | 1;

            for (i = 0; i < 12; ++i, j >>= 1)
            {
                if ((j & 1) != 0)
                {
                    /*
                     * Add 2 bytes of data to the print string
                     */
                    sprintf(strPtr + strlen(strPtr), " " HEX4N "",
                            dataPtr->mbxfailed.imbr[i]);
                }
                else
                {
                    /*
                     * Add 2 bytes of data to the print string
                     */
                    sprintf(strPtr + strlen(strPtr), " ----");
                }
            }

            /*
             * Add a linefeed and a space to start next line.
             */
            sprintf(strPtr + strlen(strPtr), "%s%s", crlf, space);

            /*
             * Get mask of output mailbox registers.
             */
            j = dataPtr->mbxfailed.oregs | 1;

            for (i = 0; i < 12; ++i, j >>= 1)
            {
                if ((j & 1) != 0)
                {
                    /*
                     * Add 2 bytes of data to the print string
                     */
                    sprintf(strPtr + strlen(strPtr), " " HEX4N "",
                            dataPtr->mbxfailed.ombr[i]);
                }
                else
                {
                    /*
                     * Add 2 bytes of data to the print string
                     */
                    sprintf(strPtr + strlen(strPtr), " ----");
                }
            }

            /*
             * Add a final linefeed
             */
            sprintf(strPtr + strlen(strPtr), "%s", crlf);
            break;

        case LOG_GetCode(LOG_LIP):
            sprintf(strPtr + strlen(strPtr), "%s Reason " HEX8 " %s",
                    space, dataPtr->portEvent.reason, crlf);
            break;

        case LOG_GetCode(LOG_IOCBTO):  /* 0x4019 */
            WWNToString(dataPtr->iocbTo.wwn, tempStr1);
            sprintf(strPtr + strlen(strPtr), "%s IOCB " HEX4N "  port %d  "
                    "ILT " HEX4N " LID %d  LUN %d  WWN %s %s",
                    space,
                    dataPtr->iocbTo.iocb,
                    dataPtr->iocbTo.port,
                    dataPtr->iocbTo.ilt,
                    dataPtr->iocbTo.lid, dataPtr->iocbTo.lun, tempStr1, crlf);
            sprintf(strPtr + strlen(strPtr), "%s CDB: " HEX8N " " HEX8N " " HEX8N " "
                    "" HEX8N " %s",
                    space, SWAPWORD(6), SWAPWORD(7), SWAPWORD(8), SWAPWORD(9), crlf);
            break;

        case LOG_GetCode(LOG_ERR_TRAP):        /* 0x415 */
        case LOG_GetCode(LOG_PROC_SYSTEM_NMI): /* 0x05F */
            {
                char        str1[5];
                char        str2[5];
                char       *p = strPtr + strlen(strPtr);

                *(UINT32 *)&str1 = *(UINT32 *)(dataPtr->procErrTrap.version);
                str1[4] = 0;

                *(UINT32 *)&str2 = *(UINT32 *)(dataPtr->procErrTrap.sequence);
                str2[4] = 0;
                /*
                 * If this is a Linux signal that trapped, get the signal.
                 */
                if ((dataPtr->procErrTrap.errorCode >= 0x60) &&
                    (dataPtr->procErrTrap.errorCode <= (0x60 + 128)))
                {
                    p += sprintf(p, "%serrorCode: %08X ( %s )%s",
                                 space,
                                 dataPtr->procErrTrap.errorCode,
                                 L_LinuxSignalToString((dataPtr->procErrTrap.errorCode -
                                                        0x60), 0), crlf);
                }
                else
                {
                    p += sprintf(p, "%serrorCode: %08X%s",
                                 space, dataPtr->procErrTrap.errorCode, crlf);
                }
                p += sprintf(p, "%sprocessor: %08X%s",
                             space, dataPtr->procErrTrap.processor, crlf);

                p += sprintf(p, "%sfirmware:  %s %s%s", space, str1, str2, crlf);

                p += sprintf(p, "%sR Registers:%s", space, crlf);

                p += sprintf(p, "%s%08X %08X %08X %08X%s",
                             space,
                             dataPtr->procErrTrap.rRegs[0],
                             dataPtr->procErrTrap.rRegs[1],
                             dataPtr->procErrTrap.rRegs[2],
                             dataPtr->procErrTrap.rRegs[3], crlf);

                p += sprintf(p, "%s%08X %08X %08X %08X%s",
                             space,
                             dataPtr->procErrTrap.rRegs[4],
                             dataPtr->procErrTrap.rRegs[5],
                             dataPtr->procErrTrap.rRegs[6],
                             dataPtr->procErrTrap.rRegs[7], crlf);

                p += sprintf(p, "%s%08X %08X %08X %08X%s",
                             space,
                             dataPtr->procErrTrap.rRegs[8],
                             dataPtr->procErrTrap.rRegs[9],
                             dataPtr->procErrTrap.rRegs[10],
                             dataPtr->procErrTrap.rRegs[11], crlf);

                p += sprintf(p, "%s%08X %08X %08X %08X%s",
                             space,
                             dataPtr->procErrTrap.rRegs[12],
                             dataPtr->procErrTrap.rRegs[13],
                             dataPtr->procErrTrap.rRegs[14],
                             dataPtr->procErrTrap.rRegs[15], crlf);

                p += sprintf(p, "%sG Registers:%s", space, crlf);

                p += sprintf(p, "%s%08X %08X %08X %08X%s",
                             space,
                             dataPtr->procErrTrap.gRegs[0],
                             dataPtr->procErrTrap.gRegs[1],
                             dataPtr->procErrTrap.gRegs[2],
                             dataPtr->procErrTrap.gRegs[3], crlf);

                p += sprintf(p, "%s%08X %08X %08X %08X%s",
                             space,
                             dataPtr->procErrTrap.gRegs[4],
                             dataPtr->procErrTrap.gRegs[5],
                             dataPtr->procErrTrap.gRegs[6],
                             dataPtr->procErrTrap.gRegs[7], crlf);

                p += sprintf(p, "%s%08X %08X %08X %08X%s",
                             space,
                             dataPtr->procErrTrap.gRegs[8],
                             dataPtr->procErrTrap.gRegs[9],
                             dataPtr->procErrTrap.gRegs[10],
                             dataPtr->procErrTrap.gRegs[11], crlf);

                p += sprintf(p, "%s%08X %08X %08X %08X%s",
                             space,
                             dataPtr->procErrTrap.gRegs[12],
                             dataPtr->procErrTrap.gRegs[13],
                             dataPtr->procErrTrap.gRegs[14],
                             dataPtr->procErrTrap.gRegs[15], crlf);
            }
            break;

        case LOG_GetCode(LOG_LOOP_PRIMITIVE_DEBUG):
            sprintf(strPtr + strlen(strPtr),
                    "%s ec " HEX8 ",  processor %s,  option %d,  "
                    "id %d   port %d  lid %d %s", space,
                    dataPtr->loopPrimitiveOp.errorCode,
                    (dataPtr->loopPrimitiveOp.proc) ? "Storage side" : "Host side",
                    dataPtr->loopPrimitiveOp.option, dataPtr->loopPrimitiveOp.id,
                    dataPtr->loopPrimitiveOp.port, dataPtr->loopPrimitiveOp.lid, crlf);
            break;

        case LOG_GetCode(LOG_PROC_NAME_DEVICE_OP):
            sprintf(strPtr + strlen(strPtr),
                    "%s option 0x%hx, id %d, status 0x%hhx, errorCode 0x%x %s",
                    space,
                    dataPtr->procNameDevice.option,
                    dataPtr->procNameDevice.id,
                    dataPtr->procNameDevice.status,
                    dataPtr->procNameDevice.errorCode, crlf);
            break;

        case LOG_GetCode(LOG_DIAG_RESULT):
            sprintf(strPtr + strlen(strPtr), "%s Test %d,  Phase %d,  Addr: 0x%x  %s",
                    space,
                    dataPtr->diagResult.data.record.test,
                    dataPtr->diagResult.data.record.phase,
                    dataPtr->diagResult.data.record.address, crlf);
            break;

        case LOG_GetCode(LOG_FCM):
            sprintf(strPtr + strlen(strPtr),
                    "%s port: %d, eventType: %d, conditionCode: %d %s", space,
                    dataPtr->fcalMonitor.port, dataPtr->fcalMonitor.eventType,
                    dataPtr->fcalMonitor.conditionCode, crlf);
            break;

        case LOG_GetCode(LOG_CSTOP_LOG):
            sprintf(strPtr + strlen(strPtr),
                    "%s Function Addr: 0x%x, Cache Requests: %d, "
                    "Write Requests: %d  Read Requests: %d %s", space,
                    dataPtr->cStopLog.function, dataPtr->cStopLog.cacheReqCount,
                    dataPtr->cStopLog.cacheWriteReqCount,
                    dataPtr->cStopLog.cacheReadReqCount, crlf);
            sprintf(strPtr + strlen(strPtr), "%s Vlink Ops: %d, VRPs: %d %s", space,
                    dataPtr->cStopLog.vLinkReqCount, dataPtr->cStopLog.vrpReqCount, crlf);
            break;

        case LOG_GetCode(LOG_WC_FLUSH):
            sprintf(strPtr + strlen(strPtr),
                    "%s Tags Dirty: %d, Tags Resident: %d, Tags FlushIP: %d, "
                    "Blocks Dirty: %d, Blocks Resident: %d, Blocks FlushIP %d,  %s",
                    space, dataPtr->wcFlushLong.tagDirty,
                    dataPtr->wcFlushLong.tagResident,
                    dataPtr->wcFlushLong.tagFlushInProgress,
                    dataPtr->wcFlushLong.blksDirty, dataPtr->wcFlushLong.blksResident,
                    dataPtr->wcFlushLong.blksFlushInProgress, crlf);
            sprintf(strPtr + strlen(strPtr),
                    "%s Flush and Invalidates: %d, Cache ORC: %d, "
                    "Cache Word: 0x%x, VRPs: %d, Proc Free Time: %d %s", space,
                    dataPtr->wcFlushLong.flushInvalidates, dataPtr->wcFlushLong.cacheORC,
                    dataPtr->wcFlushLong.cacheStatus,
                    dataPtr->wcFlushLong.outstandingVRPs,
                    dataPtr->wcFlushLong.procUtilization, crlf);
            break;

        case LOG_GetCode(LOG_CONTROLLER_FAIL):
            sprintf(strPtr + strlen(strPtr),
                    "%s Controller SN: %d, Failed Controller SN: %d %s", space,
                    dataPtr->controllerFail.r2.r1.controllerSN,
                    dataPtr->controllerFail.r2.r1.failedControllerSN, crlf);

            /*
             * Print the additional data in the R2 structure
             */
            if (logPtr->length > sizeof(LOG_CONTROLLER_FAIL_R1))
            {
                sprintf(strPtr + strlen(strPtr), "%s Reason: %s %s",
                        space, dataPtr->controllerFail.r2.reasonString, crlf);
            }

            /*
             * Print the additional data in the R3 structure
             */
            if (logPtr->length > sizeof(LOG_CONTROLLER_FAIL_R2))
            {
                EL_GetContactMapStateString(tempStr1, dataPtr->controllerFail.contactType,
                                            sizeof(tempStr1));

                sprintf(strPtr + strlen(strPtr), "%s Contact Type: %s (%d) %s",
                        space, tempStr1, dataPtr->controllerFail.contactType, crlf);
            }
            break;

        case LOG_GetCode(LOG_VLINK_OPEN_BEGIN):
            sprintf(strPtr + strlen(strPtr),
                    "%s Input VID: %d, RAID VID: %d, RAID ID: %d %s", space,
                    dataPtr->vlinkOpenBegin.inVID, dataPtr->vlinkOpenBegin.raidVID,
                    dataPtr->vlinkOpenBegin.raidID, crlf);
            break;

        case LOG_GetCode(LOG_VLINK_OPEN_END):
            sprintf(strPtr + strlen(strPtr),
                    "%s VLOP VID: %d, RAID VID: %d, RAID ID: %d, "
                    "VLOP Ending State: %d %s", space, dataPtr->vlinkOpenEnd.vlopVID,
                    dataPtr->vlinkOpenEnd.raidVID, dataPtr->vlinkOpenEnd.raidID,
                    dataPtr->vlinkOpenEnd.vlopState, crlf);
            break;

        case LOG_GetCode(LOG_RAID5_INOPERATIVE):
        case LOG_GetCode(LOG_RAID_INOPERATIVE):
        case LOG_GetCode(LOG_DEFRAG_VER_DONE):
        case LOG_GetCode(LOG_DEFRAG_OP_COMPLETE):
        case LOG_GetCode(LOG_NV_MEM_EVENT):
        case LOG_GetCode(LOG_WAIT_CACHE_ERROR):
            break;

        case LOG_GetCode(LOG_WC_SN_VCG_BAD):
        case LOG_GetCode(LOG_WC_SN_BAD):
            sprintf(strPtr + strlen(strPtr), "%s cacheId " HEX2 ",  sysSN " HEX8 "  "
                    "cacheSN " HEX8 " %s",
                    space, dataPtr->wcSeqNoBad.cacheId,
                    dataPtr->wcSeqNoBad.sysSeq, dataPtr->wcSeqNoBad.seq, crlf);
            break;

        case LOG_GetCode(LOG_WC_NVMEM_BAD):
            sprintf(strPtr + strlen(strPtr), "%s cacheId " HEX2 ",  sysSN " HEX8 "  "
                    "NVstatus " HEX8 " %s",
                    space, dataPtr->wcSeqNoBad.cacheId,
                    dataPtr->wcSeqNoBad.sysSeq, dataPtr->wcSeqNoBad.seq, crlf);
            break;

        case LOG_GetCode(LOG_IPMI_EVENT):
            sprintf(strPtr + strlen(strPtr),
                    "%s Event type: %d, String length: %d bytes %s", space,
                    dataPtr->ipmiEvent.eventType, strlen(dataPtr->ipmiEvent.text), crlf);
            break;
        case LOG_GetCode(LOG_ISE_ELEM_CHANGE_I):
            switch (dataPtr->iseElemChange.component)
            {
                case LOG_ISE_MRC:
                    InetToAscii(dataPtr->iseElemChange.mrc_data.ip, tempStr1);
                    sprintf(strPtr + strlen(strPtr),
                            "%s ISE %d-MRC %d ID: PN%s SN%s VER:%s/%s IP:%s\n", space,
                            dataPtr->iseElemChange.mrc_data.bayid,
                            dataPtr->iseElemChange.mrc_data.mrc_no,
                            dataPtr->iseElemChange.mrc_data.pn,
                            dataPtr->iseElemChange.mrc_data.sn,
                            dataPtr->iseElemChange.mrc_data.hw_vers,
                            dataPtr->iseElemChange.mrc_data.fw_vers, tempStr1);
                    break;
                case LOG_ISE_BATTERY:
                    sprintf(strPtr + strlen(strPtr),
                            "%s ISE %d-BATTERY %d ID: PN%s SN%s\n", space,
                            dataPtr->iseElemChange.bat_data.bayid,
                            dataPtr->iseElemChange.bat_data.bat_no,
                            dataPtr->iseElemChange.bat_data.pn,
                            dataPtr->iseElemChange.bat_data.sn);
                    break;
#if 0   // ATS-153  ISE's MRC fails and you can not get this information.
                case LOG_ISE_DPAC:
                    sprintf(strPtr + strlen(strPtr),
                            "%s ISE %d-DATAPAC %d ID: PN%s SN%s VER:%s\n", space,
                            dataPtr->iseElemChange.dpac_data.bayid,
                            dataPtr->iseElemChange.dpac_data.dpac_no,
                            dataPtr->iseElemChange.dpac_data.pn,
                            dataPtr->iseElemChange.dpac_data.sn,
                            dataPtr->iseElemChange.dpac_data.fw_vers);
                    break;
#endif  /* 0 */
                case LOG_ISE_PS:
                    sprintf(strPtr + strlen(strPtr),
                            "%s ISE %d-POWERSUPPLY %d ID: PN%s SN%s\n", space,
                            dataPtr->iseElemChange.ps_data.bayid,
                            dataPtr->iseElemChange.ps_data.ps_no,
                            dataPtr->iseElemChange.ps_data.pn,
                            dataPtr->iseElemChange.ps_data.sn);
                    break;
            }
            break;

        case LOG_GetCode(LOG_DVLIST):  /* 0x401f */
        case LOG_GetCode(LOG_FIRMWARE_ALERT):  /* 0x206 */
        case LOG_GetCode(LOG_IOCB):
        default:
            /*
             * If the message has no extended data,
             * blow out of here immediately.
             */
            if (logPtr->length == 0)
            {
                break;
            }

            i = 0;
            j = MAX_LOG_MESSAGE_LINES;
            sprintf(strPtr + strlen(strPtr), "%s Event code= " HEX4 "  "
                    "length= " HEX4 "", space, logPtr->eventCode, logPtr->length);

            /* Can't have anything this large */
            if (logPtr->length > 256)
            {
                sprintf(strPtr + strlen(strPtr), "%s", crlf);
                break;
            }

            while ((i < logPtr->length) && (j > 0))
            {
                /*
                 * Every 16 bytes of data, add a newline and indentation.
                 */
                if ((i % 16) == 0)
                {
                    sprintf(strPtr + strlen(strPtr), "%s%s", crlf, space);

                    /*
                     * Decrement the line count
                     */
                    --j;
                }

                /*
                 * Add 4 bytes of data to the print string
                 */
                sprintf(strPtr + strlen(strPtr), " " HEX8N "", *(UINT32 *)(data + i));
                i += 4;

            }
            /*
             * Add a final linefeed
             */
            sprintf(strPtr + strlen(strPtr), "%s", crlf);
            break;
    }
}

/*----------------------------------------------------------------------------
**  Function Name: VerString
**
**  Comments:  extracts the 4 byte version field into a 5 byte static buffer.
**
**  Returns:   pointer to the static buffer
**
**--------------------------------------------------------------------------*/
static char *VerString(char *s)
{
    static char ver[5] = { 0 };
    ver[0] = s[0];
    ver[1] = s[1];
    ver[2] = s[2];
    ver[3] = s[3];
    return ver;
}

/*----------------------------------------------------------------------------
**  Function Name: HTML_ExtendedMessage
**
**  Comments:  Parse the passed event code and create the extended message
**             associated with this event
**
**  Parameters:   logPtr - pointer to the desired log entry
**                strPtr - pointer to store the constructed string
**
**  Returns:  Length of extended message string
**
**  Notes:  Adds null termination to strPtr.
**--------------------------------------------------------------------------*/
unsigned int HTML_ExtendedMessage(char *strPtr, LOG_HDR *logPtr)
{
    strPtr[0] = 0;
    ExtendedMessage(strPtr, logPtr, ASCII_TYPE, INDENT);
    return (strlen(strPtr));
}

/*----------------------------------------------------------------------------
**  Function Name: GetTimeString
**
**  Comments:  Convert the Time and date to a string of the form:
**              12:01pm 06/14/2000
**
**  Parameters:   timePtr - pointer to a TimeStamp
**                strPtr  - pointer to a string to store the result
**
**--------------------------------------------------------------------------*/
void GetTimeString(UINT32 *timePtr, char *strPtr)
{
    LongTimeToString(strPtr, ((time_t)(*timePtr)), false);
}

/*----------------------------------------------------------------------------
**  Function Name: GetTimeString
**
**  Comments:  Convert the Time and date to a string of the form:
**              12:01pm 06/14/2000
**
**  Parameters:   timePtr - pointer to a TimeStamp
**                strPtr  - pointer to a string to store the result
**                military - use military time
**
**--------------------------------------------------------------------------*/
void GetLogTimeString(LOGTIME *timePtr, char *strPtr, bool military)
{
    int         pmFlag = FALSE;
    char        tmpStr[3];

    /*
     * Determine if the time is in the pm and then convert the hours
     * from military time to stardard 12 hour time.
     */
    if (timePtr->hours >= 0x12)
    {
        pmFlag = TRUE;

    }

    if (military)
    {
        BCDToString(timePtr->hours, tmpStr);
    }
    else
    {
        BCDToString(ConvertFromMilitaryTime(timePtr->hours), tmpStr);
    }

    strcpy(strPtr, tmpStr);

    /*
     * Add the semi-colon
     */
    strcat(strPtr, ":");

    /*
     * Convert and add the minutes
     */
    BCDToString(timePtr->minutes, tmpStr);
    strcat(strPtr, tmpStr);

    /*
     * Add the semi-colon
     */
    strcat(strPtr, ":");

    /*
     * Convert and add the seconds
     */
    BCDToString(timePtr->seconds, tmpStr);
    strcat(strPtr, tmpStr);

    /*
     * Add the AM or PM indicator with spacing for the date
     */
    if (military)
    {
        strcat(strPtr, " ");
    }
    else
    {
        if (pmFlag)
        {
            strcat(strPtr, "pm ");
        }
        else
        {
            strcat(strPtr, "am ");
        }
    }

    /*
     * Convert and add the month, seperated by a slash
     */
    BCDToString(timePtr->month, tmpStr);
    strcat(strPtr, tmpStr);
    strcat(strPtr, "/");

    /*
     * Convert and add the day of the month, seperated by a slash
     */
    BCDToString(timePtr->date, tmpStr);
    strcat(strPtr, tmpStr);
    strcat(strPtr, "/");

    /*
     * Convert and add the century and the year
     */
    BCDToString((UINT8)(timePtr->year >> 8), tmpStr);
    strcat(strPtr, tmpStr);
    BCDToString((UINT8)timePtr->year, tmpStr);
    strcat(strPtr, tmpStr);
}

/*----------------------------------------------------------------------------
**  Function Name: GetEventTypeString
**
**  Comments:  Get the event type  of the log event and convert it to one of
**             the following strings:
**              "ERROR  "       - error entry
**              "INFO   "       - Informational entry
**              "WARNING"       - Warning entry
**              " DEBUG "       - Warning entry
**              " FATAL "       - Warning entry
**
**  Parameters:   eventCode  - log entry event code
**                strPtr     - pointer to a string to store the result
**
**--------------------------------------------------------------------------*/
void GetEventTypeString(UINT16 eventCode, char *strPtr)
{
    UINT8       eventType;

    eventType = GetEventType(eventCode);

    /*
     * If the event code is above the WARNING level, report a warning.
     * Otherwise, if it is above the ERROR level, report an error. Otherwise,
     * report it as informational.
     */
    if (eventType == LOG_TYPE_INFO)
    {
        strcpy(strPtr, " Info  ");
    }
    else if (eventType == LOG_TYPE_WARNING)
    {
        strcpy(strPtr, "Warning");
    }
    else if (eventType == LOG_TYPE_ERROR)
    {
        strcpy(strPtr, " Error ");
    }
    else if (eventType == LOG_TYPE_DEBUG)
    {
        strcpy(strPtr, " Debug ");
    }
    else if (eventType == LOG_TYPE_FATAL)
    {
        strcpy(strPtr, " Fatal ");
    }
    else
    {
        strcpy(strPtr, "       ");
    }
}

/*----------------------------------------------------------------------------
**  Function Name: BCDToString
**
**  Comments:   Converts a BCD number to a string
**
**  Parameters:  value  - a 2-digit BCD number
**               strPtr - pointer to string in which to store result
**--------------------------------------------------------------------------*/
static void BCDToString(UINT8 BCDValue, char *strPtr)
{
    /*
     * Convert the upper and lower nibbles to ascii characters
     */
    *strPtr++ = (char)((BCDValue >> 4) + '0');
    *strPtr++ = (char)((BCDValue & 0x0F) + '0');

    /*
     * Terminate the "C" string
     */
    *strPtr = '\0';
}

/*----------------------------------------------------------------------------
**  Function Name: SwapWord
**
**  Comments:   Swaps the order of the bytes within a word (int)
**
**  Parameters:  word  - word to swap
**
**  Returns:     int - swapped word
**
**--------------------------------------------------------------------------*/
static int SwapWord(int word)
{
    /*
     * Swap the bytes of the word end for end.
     */
    return (((word >> 24) & 0xffUL) |
            ((word >> 8) & 0xff00UL) |
            ((word << 8) & 0xff0000UL) | ((word << 24) & 0xff000000UL));
}

/*----------------------------------------------------------------------------
**  Function Name: LongTimeToString
**
**  Comments:   Converts a long time to a string
**
**  Parameters:  strTime    - pointer to string in which to store result
**               longTime   - longTime
**
**--------------------------------------------------------------------------*/
static void LongTimeToString(char *strTime, const time_t longTime, bool military)
{
    struct tm   tm;
    TIMESTAMP   ts;

    /*
     * Convert system seconds to a time structure (decimal)
     */
    tm = *gmtime(&longTime);

    /*
     * Convert time structure (decimal) into a timestamp structure (BCD)
     * Year is 1900 based, month is 0 based, and day of week is 0 based.
     * Convert to match up with RTC.
     */
    ts.year = ShortToBCD((UINT16)tm.tm_year + 1900);
    ts.month = (UINT8)ShortToBCD((UINT16)tm.tm_mon + 1);
    ts.date = (UINT8)ShortToBCD((UINT16)tm.tm_mday);
    ts.day = (UINT8)ShortToBCD((UINT16)tm.tm_wday + 1);
    ts.hours = (UINT8)ShortToBCD((UINT16)tm.tm_hour);
    ts.minutes = (UINT8)ShortToBCD((UINT16)tm.tm_min);
    ts.seconds = (UINT8)ShortToBCD((UINT16)tm.tm_sec);

    GetLogTimeString((LOGTIME *)&ts, strTime, military);
}

/*----------------------------------------------------------------------------
**  Function Name: Logview_BuildEventPropertiesString
**--------------------------------------------------------------------------*/
static void Logview_BuildEventPropertiesString(HWM_EVENT_PROPERTIES *eventPropertiesPtr,
                                               char *strPtr, char *space, char *crlf)
{
    char        tempStr1[MAX_MESSAGE_LENGTH];

    /* Status Condition */

    HWM_GetStatusCodeString(tempStr1, eventPropertiesPtr->statusCode, sizeof(tempStr1));

    sprintf(strPtr + strlen(strPtr), "%s Status: %s (%d)%s",
            space, tempStr1, eventPropertiesPtr->statusCode, crlf);
}

/*----------------------------------------------------------------------------
**  Function Name: Logview_BuildCCBStatusString
**--------------------------------------------------------------------------*/
static void Logview_BuildCCBStatusString(CCB_STATUS *ccbStatusPtr, char *strPtr,
                                         char *space, char *crlf)
{
    Logview_BuildEventPropertiesString(&ccbStatusPtr->eventProperties,
                                       strPtr + strlen(strPtr), space, crlf);

    sprintf(strPtr + strlen(strPtr), "%s -- NVRAM Battery --%s", space, crlf);
    Logview_BuildNVRAMBatteryStatusString(&ccbStatusPtr->nvramBatteryStatus,
                                          strPtr + strlen(strPtr), space, crlf);
}

/*----------------------------------------------------------------------------
**  Function Name: Logview_BuildProcessorBoardStatusString
**--------------------------------------------------------------------------*/
static void Logview_BuildProcessorBoardStatusString(PROC_BOARD_STATUS
                                                    *processorBoardStatusPtr,
                                                    char *strPtr, char *space, char *crlf)
{
    Logview_BuildEventPropertiesString(&processorBoardStatusPtr->eventProperties,
                                       strPtr + strlen(strPtr), space, crlf);

    sprintf(strPtr + strlen(strPtr), "%s -- Supply Voltages --%s", space, crlf);
    Logview_BuildPowerSupplyVoltagesStatusString(&processorBoardStatusPtr->powerSupplyVoltagesStatus,
                                                 strPtr + strlen(strPtr), space, crlf);

    sprintf(strPtr + strlen(strPtr), "%s -- FE CPU --%s", space, crlf);
    Logview_BuildProcessorStatusString(&processorBoardStatusPtr->frontEndProcessorStatus,
                                       strPtr + strlen(strPtr), space, crlf);

    sprintf(strPtr + strlen(strPtr), "%s -- BE CPU --%s", space, crlf);
    Logview_BuildProcessorStatusString(&processorBoardStatusPtr->backEndProcessorStatus,
                                       strPtr + strlen(strPtr), space, crlf);
}

/*----------------------------------------------------------------------------
**  Function Name: Logview_BuildPowerSupplyVoltagesStatusString
**--------------------------------------------------------------------------*/
static void Logview_BuildPowerSupplyVoltagesStatusString(POWER_SUPPLY_VOLTAGES_STATUS_PTR
                                                         powerSupplyVoltagesStatusPtr,
                                                         char *strPtr, char *space,
                                                         char *crlf)
{
    char        tempStr1[MAX_MESSAGE_LENGTH];

    Logview_BuildEventPropertiesString(&powerSupplyVoltagesStatusPtr->eventProperties,
                                       strPtr + strlen(strPtr), space, crlf);

    /*
     * 12 Volt
     */
    HWM_GetLimitMonitorConditionString(tempStr1,
                                       powerSupplyVoltagesStatusPtr->twelveVoltReading.limitMonitorValue,
                                       sizeof(tempStr1));

    sprintf(strPtr + strlen(strPtr), "%s 12: %s (C: %dmV, L: %d, H: %d)%s",
            space,
            tempStr1,
            powerSupplyVoltagesStatusPtr->twelveVoltReading.currentMillivolts,
            powerSupplyVoltagesStatusPtr->twelveVoltReading.minimumMillivolts,
            powerSupplyVoltagesStatusPtr->twelveVoltReading.maximumMillivolts, crlf);

    /*
     * 5 Volt
     */
    HWM_GetLimitMonitorConditionString(tempStr1,
                                       powerSupplyVoltagesStatusPtr->fiveVoltReading.limitMonitorValue,
                                       sizeof(tempStr1));

    sprintf(strPtr + strlen(strPtr), "%s 5: %s (C: %dmV, L: %d, H: %d)%s",
            space,
            tempStr1,
            powerSupplyVoltagesStatusPtr->fiveVoltReading.currentMillivolts,
            powerSupplyVoltagesStatusPtr->fiveVoltReading.minimumMillivolts,
            powerSupplyVoltagesStatusPtr->fiveVoltReading.maximumMillivolts, crlf);

    /*
     * 3.3 Volt
     */
    HWM_GetLimitMonitorConditionString(tempStr1,
                                       powerSupplyVoltagesStatusPtr->threePointThreeVoltReading.limitMonitorValue,
                                       sizeof(tempStr1));

    sprintf(strPtr + strlen(strPtr), "%s 3.3: %s (C: %dmV, L: %d, H: %d)%s",
            space,
            tempStr1,
            powerSupplyVoltagesStatusPtr->threePointThreeVoltReading.currentMillivolts,
            powerSupplyVoltagesStatusPtr->threePointThreeVoltReading.minimumMillivolts,
            powerSupplyVoltagesStatusPtr->threePointThreeVoltReading.maximumMillivolts,
            crlf);

    /*
     * 5 Volt Standby
     */
    HWM_GetLimitMonitorConditionString(tempStr1,
                                       powerSupplyVoltagesStatusPtr->standbyVoltageReading.limitMonitorValue,
                                       sizeof(tempStr1));

    sprintf(strPtr + strlen(strPtr), "%s SB: %s (C: %dmV, L: %d, H: %d)%s",
            space,
            tempStr1,
            powerSupplyVoltagesStatusPtr->standbyVoltageReading.currentMillivolts,
            powerSupplyVoltagesStatusPtr->standbyVoltageReading.minimumMillivolts,
            powerSupplyVoltagesStatusPtr->standbyVoltageReading.maximumMillivolts, crlf);
}

/*----------------------------------------------------------------------------
**  Function Name: Logview_BuildProcessorStatusString
**--------------------------------------------------------------------------*/
static void Logview_BuildProcessorStatusString(PROC_BOARD_PROCESSOR_STATUS_PTR
                                               processorStatusPtr, char *strPtr,
                                               char *space, char *crlf)
{
    char        tempStr1[MAX_MESSAGE_LENGTH];

    Logview_BuildEventPropertiesString(&processorStatusPtr->eventProperties,
                                       strPtr + strlen(strPtr), space, crlf);

    /*
     * Memory Socket Supply Voltage
     */
    HWM_GetLimitMonitorConditionString(tempStr1,
                                       processorStatusPtr->memorySocketSupplyVoltageReading.limitMonitorValue,
                                       sizeof(tempStr1));

    sprintf(strPtr + strlen(strPtr), "%s Socket mV: %s (C: %d, L: %d, H: %d)%s",
            space,
            tempStr1,
            processorStatusPtr->memorySocketSupplyVoltageReading.currentMillivolts,
            processorStatusPtr->memorySocketSupplyVoltageReading.minimumMillivolts,
            processorStatusPtr->memorySocketSupplyVoltageReading.maximumMillivolts, crlf);

    /*
     * Processor Reset Condition
     */
    HWM_GetProcessorResetConditionString(tempStr1,
                                         processorStatusPtr->processorResetConditionValue,
                                         sizeof(tempStr1));

    sprintf(strPtr + strlen(strPtr), "%s CPU Reset: %s (%d)%s",
            space, tempStr1, processorStatusPtr->processorResetConditionValue, crlf);

    /*
     * Processor Temperature
     */
    Logview_BuildTemperatureStatusString(&processorStatusPtr->temperatureStatus,
                                         strPtr + strlen(strPtr), space, crlf);
}

/*----------------------------------------------------------------------------
**  Function Name: Logview_BuildTemperatureStatusString
**--------------------------------------------------------------------------*/
static void Logview_BuildTemperatureStatusString(TEMPERATURE_STATUS_PTR
                                                 temperatureStatusPtr, char *strPtr,
                                                 char *space, char *crlf)
{
    char        tempStr1[MAX_MESSAGE_LENGTH];

    Logview_BuildEventPropertiesString(&temperatureStatusPtr->eventProperties,
                                       strPtr + strlen(strPtr), space, crlf);

    /*
     * Temperature
     */
    HWM_GetTemperatureConditionString(tempStr1,
                                      temperatureStatusPtr->conditionValue,
                                      sizeof(tempStr1));

    sprintf(strPtr + strlen(strPtr), "%s Temp: %s (C: %d, L: %d, H: %d)%s",
            space,
            tempStr1,
            temperatureStatusPtr->currentDegreesCelsius,
            temperatureStatusPtr->minimumDegreesCelsius,
            temperatureStatusPtr->maximumDegreesCelsius, crlf);
}

/*----------------------------------------------------------------------------
**  Function Name: Logview_BuildPowerSupplyStatusString
**--------------------------------------------------------------------------*/
static void Logview_BuildPowerSupplyStatusString(POWER_SUPPLY_STATUS_PTR
                                                 powerSupplyStatusPtr, char *strPtr,
                                                 char *space, char *crlf)
{
    char        tempStr1[MAX_MESSAGE_LENGTH];

    Logview_BuildEventPropertiesString(&powerSupplyStatusPtr->eventProperties,
                                       strPtr + strlen(strPtr), space, crlf);

    /*
     * Power Supply Condition
     */
    HWM_GetPowerSupplyConditionString(tempStr1,
                                      powerSupplyStatusPtr->powerSupplyCondition.value,
                                      sizeof(tempStr1));

    sprintf(strPtr + strlen(strPtr), "%s Power: %s (%d)%s",
            space, tempStr1, powerSupplyStatusPtr->powerSupplyCondition.value, crlf);

    /*
     * Cooling Fan Condition
     */
    HWM_GetCoolingFanConditionString(tempStr1,
                                     powerSupplyStatusPtr->coolingFanConditionValue,
                                     sizeof(tempStr1));

    sprintf(strPtr + strlen(strPtr), "%s Fan: %s (%d)%s",
            space, tempStr1, powerSupplyStatusPtr->coolingFanConditionValue, crlf);
}

/*----------------------------------------------------------------------------
**  Function Name: Logview_BuildBufferBoardStatusString
**--------------------------------------------------------------------------*/
static void Logview_BuildBufferBoardStatusString(BUFFER_BOARD_STATUS
                                                 *bufferBoardStatusPtr, char *strPtr,
                                                 char *space, char *crlf)
{
    Logview_BuildEventPropertiesString(&bufferBoardStatusPtr->eventProperties,
                                       strPtr + strlen(strPtr), space, crlf);

    sprintf(strPtr + strlen(strPtr), "%s -- Battery --%s", space, crlf);
    Logview_BuildBatteryStatusString(&bufferBoardStatusPtr->batteryStatus,
                                     strPtr + strlen(strPtr), space, crlf);

    sprintf(strPtr + strlen(strPtr), "%s -- Charger --%s", space, crlf);
    Logview_BuildChargerStatusString(&bufferBoardStatusPtr->chargerStatus,
                                     strPtr + strlen(strPtr), space, crlf);

    sprintf(strPtr + strlen(strPtr), "%s -- Fuel Gauge --%s", space, crlf);
    Logview_BuildFuelGaugeStatusString(&bufferBoardStatusPtr->fuelGaugeStatus,
                                       strPtr + strlen(strPtr), space, crlf);

    sprintf(strPtr + strlen(strPtr), "%s -- Main Regulator --%s", space, crlf);
    Logview_BuildMainRegulatorStatusString(&bufferBoardStatusPtr->mainRegulatorStatus,
                                           strPtr + strlen(strPtr), space, crlf);

    sprintf(strPtr + strlen(strPtr), "%s -- Temperature --%s", space, crlf);
    Logview_BuildTemperatureStatusString(&bufferBoardStatusPtr->temperatureStatus,
                                         strPtr + strlen(strPtr), space, crlf);
}

/*----------------------------------------------------------------------------
**  Function Name: Logview_BuildBatteryStatusString
**--------------------------------------------------------------------------*/
static void Logview_BuildBatteryStatusString(BATTERY_STATUS_PTR batteryStatusPtr,
                                             char *strPtr, char *space, char *crlf)
{
    char        tempStr1[MAX_MESSAGE_LENGTH];

    Logview_BuildEventPropertiesString(&batteryStatusPtr->eventProperties,
                                       strPtr + strlen(strPtr), space, crlf);

    /*
     * Battery Condition
     */
    HWM_GetBatteryConditionString(tempStr1,
                                  batteryStatusPtr->batteryCondition, sizeof(tempStr1));

    sprintf(strPtr + strlen(strPtr), "%s Battery: %s (%d)%s",
            space, tempStr1, batteryStatusPtr->batteryCondition, crlf);

    /*
     * Battery Terminal Voltage
     */
    HWM_GetLimitMonitorConditionString(tempStr1,
                                       batteryStatusPtr->terminalVoltageReading.limitMonitorValue,
                                       sizeof(tempStr1));

    sprintf(strPtr + strlen(strPtr), "%s Terminal mV: %s (C: %d, L: %d, H: %d)%s",
            space,
            tempStr1,
            batteryStatusPtr->terminalVoltageReading.currentMillivolts,
            batteryStatusPtr->terminalVoltageReading.minimumMillivolts,
            batteryStatusPtr->terminalVoltageReading.maximumMillivolts, crlf);
}

/*----------------------------------------------------------------------------
**  Function Name: Logview_BuildFuelGaugeStatusString
**--------------------------------------------------------------------------*/
static void Logview_BuildFuelGaugeStatusString(FUEL_GAUGE_STATUS_PTR fuelGaugeStatusPtr,
                                               char *strPtr, char *space, char *crlf)
{
    char        tempStr1[MAX_MESSAGE_LENGTH];

    Logview_BuildEventPropertiesString(&fuelGaugeStatusPtr->eventProperties,
                                       strPtr + strlen(strPtr), space, crlf);

    /*
     * Fuel Gauge Condition
     */
    HWM_GetFuelGaugeConditionString(tempStr1,
                                    fuelGaugeStatusPtr->fuelGaugeCondition,
                                    sizeof(tempStr1));

    sprintf(strPtr + strlen(strPtr), "%s Fuel Gauge: %s (%d)%s",
            space, tempStr1, fuelGaugeStatusPtr->fuelGaugeCondition, crlf);

    /*
     * Current Flow Condition and rate
     */
    HWM_GetCurrentFlowConditionString(tempStr1,
                                      fuelGaugeStatusPtr->currentFlowCondition,
                                      sizeof(tempStr1));

    sprintf(strPtr + strlen(strPtr), "%s Flow: %s (%d, Rate: %d)%s",
            space,
            tempStr1,
            fuelGaugeStatusPtr->currentFlowCondition,
            fuelGaugeStatusPtr->currentFlowRate, crlf);

    /*
     * Fuel Gauge Regulator Output Voltage
     */
    HWM_GetLimitMonitorConditionString(tempStr1,
                                       fuelGaugeStatusPtr->regulatorOutputVoltageReading.limitMonitorValue,
                                       sizeof(tempStr1));

    sprintf(strPtr + strlen(strPtr), "%s Reg Output mV: %s (C: %d, L: %d, H: %d)%s",
            space,
            tempStr1,
            fuelGaugeStatusPtr->regulatorOutputVoltageReading.currentMillivolts,
            fuelGaugeStatusPtr->regulatorOutputVoltageReading.minimumMillivolts,
            fuelGaugeStatusPtr->regulatorOutputVoltageReading.maximumMillivolts, crlf);
}

/*----------------------------------------------------------------------------
**  Function Name: Logview_BuildMainRegulatorStatusString
**--------------------------------------------------------------------------*/
static void Logview_BuildMainRegulatorStatusString(MAIN_REGULATOR_STATUS_PTR
                                                   mainRegulatorStatusPtr, char *strPtr,
                                                   char *space, char *crlf)
{
    char        tempStr1[MAX_MESSAGE_LENGTH];

    Logview_BuildEventPropertiesString(&mainRegulatorStatusPtr->eventProperties,
                                       strPtr + strlen(strPtr), space, crlf);

    /*
     * Main Regulator Condition
     */
    HWM_GetMainRegulatorConditionString(tempStr1,
                                        mainRegulatorStatusPtr->mainRegulatorCondition,
                                        sizeof(tempStr1));

    sprintf(strPtr + strlen(strPtr), "%s Main Reg: %s (%d)%s",
            space, tempStr1, mainRegulatorStatusPtr->mainRegulatorCondition, crlf);

    /*
     * Processor Board Supply Voltage
     */
    HWM_GetLimitMonitorConditionString(tempStr1,
                                       mainRegulatorStatusPtr->procBoardSupplyVoltageReading.limitMonitorValue,
                                       sizeof(tempStr1));

    sprintf(strPtr + strlen(strPtr), "%s Supply mV: %s (C: %d, L: %d, H: %d)%s",
            space,
            tempStr1,
            mainRegulatorStatusPtr->procBoardSupplyVoltageReading.currentMillivolts,
            mainRegulatorStatusPtr->procBoardSupplyVoltageReading.minimumMillivolts,
            mainRegulatorStatusPtr->procBoardSupplyVoltageReading.maximumMillivolts,
            crlf);

    /*
     * Main Regulator Input Voltage
     */
    HWM_GetLimitMonitorConditionString(tempStr1,
                                       mainRegulatorStatusPtr->inputVoltageReading.limitMonitorValue,
                                       sizeof(tempStr1));

    sprintf(strPtr + strlen(strPtr), "%s Input mV: %s (C: %d, L: %d, H: %d)%s",
            space,
            tempStr1,
            mainRegulatorStatusPtr->inputVoltageReading.currentMillivolts,
            mainRegulatorStatusPtr->inputVoltageReading.minimumMillivolts,
            mainRegulatorStatusPtr->inputVoltageReading.maximumMillivolts, crlf);

    /*
     * Main Regulator Output Voltage
     */
    HWM_GetLimitMonitorConditionString(tempStr1,
                                       mainRegulatorStatusPtr->outputVoltageReading.limitMonitorValue,
                                       sizeof(tempStr1));

    sprintf(strPtr + strlen(strPtr), "%s Output mV: %s (C: %d, L: %d, H: %d)%s",
            space,
            tempStr1,
            mainRegulatorStatusPtr->outputVoltageReading.currentMillivolts,
            mainRegulatorStatusPtr->outputVoltageReading.minimumMillivolts,
            mainRegulatorStatusPtr->outputVoltageReading.maximumMillivolts, crlf);
}

/*----------------------------------------------------------------------------
**  Function Name: Logview_BuildChargerStatusString
**--------------------------------------------------------------------------*/
static void Logview_BuildChargerStatusString(CHARGER_STATUS_PTR chargerStatusPtr,
                                             char *strPtr, char *space, char *crlf)
{
    char        tempStr1[MAX_MESSAGE_LENGTH];

    Logview_BuildEventPropertiesString(&chargerStatusPtr->eventProperties,
                                       strPtr + strlen(strPtr), space, crlf);

    /*
     * Charger Condition
     */
    HWM_GetChargerConditionString(tempStr1,
                                  chargerStatusPtr->chargerCondition, sizeof(tempStr1));

    sprintf(strPtr + strlen(strPtr), "%s Charger: %s (%d)%s",
            space, tempStr1, chargerStatusPtr->chargerCondition, crlf);
}

/*----------------------------------------------------------------------------
**  Function Name: Logview_BuildNVRAMBatteryStatusString
**--------------------------------------------------------------------------*/
static void Logview_BuildNVRAMBatteryStatusString(NVRAM_BATTERY_STATUS_PTR
                                                  nvramBatteryStatusPtr, char *strPtr,
                                                  char *space, char *crlf)
{
    char        tempStr1[MAX_MESSAGE_LENGTH];

    Logview_BuildEventPropertiesString(&nvramBatteryStatusPtr->eventProperties,
                                       strPtr + strlen(strPtr), space, crlf);

    /*
     * NVRAM Battery Condition
     */
    HWM_GetNVRAMBatteryConditionString(tempStr1,
                                       nvramBatteryStatusPtr->nvramBatteryCondition,
                                       sizeof(tempStr1));

    sprintf(strPtr + strlen(strPtr), "%s NVRAM Battery: %s (%d)%s",
            space, tempStr1, nvramBatteryStatusPtr->nvramBatteryCondition, crlf);
}


/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
