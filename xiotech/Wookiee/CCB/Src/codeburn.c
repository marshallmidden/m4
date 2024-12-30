/* $Id: codeburn.c 156532 2011-06-24 21:09:44Z m4 $ */
/*============================================================================
** FILE NAME:       codeburn.c
** MODULE TITLE:    code burn implementation
**
** DESCRIPTION:     Contains the implementation to support the code burn
**                  functions.
**
** Copyright (c) 2001-2010 XIOtech Corporation. All rights reserved.
**==========================================================================*/
#include "codeburn.h"

#ifdef LOG_SIMULATOR
#include "LogSimFuncs.h"
#else   /* LOG_SIMULATOR */
#include "ccb_hw.h"
#include "convert.h"
#include "cps_init.h"
#include "crc32.h"
#include "debug_files.h"
#include "errorCodes.h"
#include "FIO.h"
#include "ipc_sendpacket.h"
#include "kernel.h"
#include "LargeArrays.h"
#include "LOG_Defs.h"
#include "logdef.h"
#include "misc.h"
#include "MR_Defs.h"
#include "nvram.h"
#include "PacketInterface.h"
#include "PI_Utils.h"
#include "PortServer.h"
#include "quorum.h"
#include "quorum_utils.h"
#include "ses.h"
#include "sm.h"
#include "XIO_Const.h"
#include "XIO_Std.h"

#include "xk_kernel.h"
#include <stdio.h>
#include <errno.h>
#include <arpa/inet.h>
#include <byteswap.h>
#endif  /* LOG_SIMULATOR */


/*****************************************************************************
** Private defines
*****************************************************************************/
typedef struct
{
    INT16       rc;             /* PI command completion rc           */
    INT16       status;         /* PI command status                  */
    UINT8       sense;          /* SCSI Sense key                     */
    UINT8       asc;            /* SCSI Addional sense code           */
    UINT8       ascq;           /* SCSI Addional sense code qualifier */
} SCSI_CMD_STATUS;

typedef struct
{
    UINT8       active;         /* bit field bit0 == ctrl0 etc.     */
    UINT8       status0;        /* controller 0 status              */
    UINT8       status1;        /* controller 1 status              */
    UINT8       reserved;       /* reserved                         */

    UINT8       fwVer0[4];      /* fw version of controller 0       */
    UINT8       fwVer1[4];      /* fw version of controller 1       */
} BAY_CONTROLLER_STATUS;

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
#ifndef LOG_SIMULATOR
static INT32 HandlePlatformRPM(FW_HEADER *fwPtr);
static INT32 UpdateCode1(FW_HEADER *fwPtr);
static INT32 UpdateDriveBay(FW_HEADER *fwPtr);
static void FWLogResult(FW_HEADER *fwPtr, INT32 reason);

static SCSI_CMD_STATUS SCSIReceiveDiag(UINT64 wwn, UINT32 lun);
static SCSI_CMD_STATUS SCSIWriteBuffer(char *pBuf, UINT32 len, UINT64 wwn, UINT32 lun,
                                       UINT8 mode, UINT8 retry, UINT32 offset,
                                       UINT32 timeout);
static INT32 GetXyratexBayStatus(UINT64 wwn, UINT16 lun,
                                 BAY_CONTROLLER_STATUS *pBayStats);
static RC_STATUS SendFWToXyratexBay(FW_HEADER *pFwHdr, UINT64 wwn, UINT32 lun);
static INT32 MonXyratexBayStatus(UINT64 wwn, UINT16 lun, UINT32 how, UINT32 timeoutSec);

#define MON_BAY_STATUS_START   1        /* used as the 'how' parm to MonXyratexBayStatus() */
#define MON_BAY_STATUS_MID     2
#define MON_BAY_STATUS_FINAL   3
#endif /* LOG_SIMULATOR */

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
**  Function Name: TargetName
**
**  Comments:   Return a target name based upon target ID
**
**  Parameters: target ID
**
**  Returns:    Pointer to descriptive string
**
**--------------------------------------------------------------------------*/
const char *TargetName(UINT32 target)
{
    const char *str = "Unknown";

    switch (target)
    {
        case TARG_ID_CCB:
            str = "CCB Runtime";
            break;
        case TARG_ID_FE:
            str = "FE Runtime";
            break;
        case TARG_ID_FE_QLOGIC:
            str = "FE QLogic";
            break;
        case TARG_ID_BE:
            str = "BE Runtime";
            break;
        case TARG_ID_BE_QLOGIC:
            str = "BE QLogic";
            break;
        case TARG_ID_COMMON_QLOGIC:
            str = "FE/BE QLogic";
            break;
        case TARG_ID_DRIVE_BAY:
            str = "Drive Bay";
            break;
        case TARG_ID_EUROLOGIC_BAY:
            str = "Eurologic Bay";
            break;
        case TARG_ID_DISK_DRIVE:
            str = "Disk Drive";
            break;
        case TARG_ID_ADAPTEC_SATA_BAY:
            str = "Adaptec SATA Bay";
            break;
        case TARG_ID_PLATFORM_RPM:
            str = "Platform RPM";
            break;
        case TARG_ID_NULL:
            str = "NULL file";
            break;
        case TARG_ID_ADAPTEC_SATA_ES_BAY:
            str = "Adaptec SATA-ES Bay";
            break;
        case TARG_ID_XYRATEX_SBOD_BAY:
            str = "Xyratex SBOD Bay";
            break;
    }

    return str;
}

#ifndef LOG_SIMULATOR
#if 0

/*----------------------------------------------------------------------------
**  Function Name: CompareHdrs
**
**  Comments:   Compare key fields in two firmware headers.
**
**  Parameters: FW_HEADER h1, h2 - pointers to the two headers to compare.
**
**  Returns:    0 - all fields compare
**              1 - one or more fields do not compare
**
**--------------------------------------------------------------------------*/
INT32 CompareHdrs(FW_HEADER *h1, FW_HEADER *h2)
{
    if ((h1->revision == h2->revision) &&
        (h1->revCount == h2->revCount) &&
        (h1->buildID == h2->buildID) &&
        (h1->systemRelease == h2->systemRelease) &&
        (h1->hdrCksum == h2->hdrCksum) &&
        memcmp(&(h1->timeStamp), &(h2->timeStamp), sizeof(h1->timeStamp)) == 0)
    {
        return 0;
    }

    return 1;
}
#endif /* 0 */


/*----------------------------------------------------------------------------
**  Function Name: ValidateFWHeader()
**
**  Parameters: fwPtr - pointer to the FW header to verify
**
**  Returns:    0   - GOOD Completion
**              !0  - Error - reason code returned
**
**--------------------------------------------------------------------------*/
INT32 ValidateFWHeader(FW_HEADER *fwPtr)
{
    INT32       rc = PI_GOOD;

    if (fwPtr->target == TARG_ID_PLATFORM_RPM)
    {
        if (fwPtr->productID != PROD_ID)
        {
            rc = PROBABLY_NO_HEADER;
        }
        goto next;
    }

    /*
     * Make sure the product ID is correct.
     */
    switch (fwPtr->productID)
    {
            /*
             * Hypernode can handle drive and bay image files that were built
             * for Bigfoot.  It can't handle other BF images though.
             */
        case PROD_BIGFOOT:
            /*
             * A target ID ending in '0x0F' is a drive or bay. These
             * are all allowed through on Wookiee/Hypernode.
             */
            if ((fwPtr->target & 0xFF) != 0x0F)
            {
                rc = PROBABLY_NO_HEADER;
                dprintf(DPRINTF_CODE_UPDATE, "ValidateFWHeader: Target bad (0x%X)\n",
                        fwPtr->target);
            }
            break;

        case PROD_HYPERNODE:
            switch (fwPtr->target)
            {
                case TARG_ID_NULL:
                case TARG_ID_XYRATEX_SBOD_BAY:
                    break;

                default:
                    rc = PROBABLY_NO_HEADER;
                    dprintf(DPRINTF_CODE_UPDATE, "ValidateFWHeader: Target bad (0x%X)\n",
                            fwPtr->target);
                    break;
            }
            break;

        default:
            rc = PROBABLY_NO_HEADER;
            dprintf(DPRINTF_CODE_UPDATE, "ValidateFWHeader: Product ID bad (0x%X)\n",
                    fwPtr->productID);
            break;
    }

    if (rc == PROBABLY_NO_HEADER)
    {
        goto next;
    }

    /*
     * Check the magic number field
     */
    if (fwPtr->magicNumber != MAGIC_NUMBER)
    {
        rc = PROBABLY_NO_HEADER;
        dprintf(DPRINTF_CODE_UPDATE, "ValidateFWHeader: Magic number bad (0x%X)\n",
                fwPtr->magicNumber);
        goto next;
    }

    /*
     * Validate the firmware header CRC's:
     * 1. of the header
     * 2. of the entire firmware
     */
    if ((CRC32((char *)fwPtr, sizeof(*fwPtr) - 4) != fwPtr->hdrCksum))
    {
        rc = FWHEADER_VERIFY_ERROR;
        dprintf(DPRINTF_CODE_UPDATE, "ValidateFWHeader: Header CRC bad (0x%X)\n",
                fwPtr->hdrCksum);
        goto next;
    }
    if (CRC32((char *)fwPtr + sizeof(*fwPtr), fwPtr->length - sizeof(*fwPtr)) != fwPtr->checksum)
    {
        rc = FWHEADER_VERIFY_ERROR;
        dprintf(DPRINTF_CODE_UPDATE, "ValidateFWHeader: Data CRC bad (0x%X)\n",
                fwPtr->checksum);
        goto next;
    }

  next:
    /* Log the result */
    if (rc != PI_GOOD)
    {
        FWLogResult(fwPtr, rc);
    }

    return rc;
}


/*----------------------------------------------------------------------------
**  Function Name: UpdateCode
**
**  Comments:  Validates the firmware header and then updates the flash code on
**             the CCB board, at the location and for the device  described in
**             the firmware header.
**
**  Parameters: fwPtr - dram pointer where firmware header resides.
**
**  Returns:    0   - GOOD Completion
**              !0  - Error
**
**--------------------------------------------------------------------------*/
INT32 UpdateCode(FW_HEADER *fwPtr)
{
    INT32       rc1;

    switch (fwPtr->target)
    {
        case TARG_ID_COMMON_DIAG:
        case TARG_ID_COMMON_BOOT:
        case TARG_ID_COMMON_QLOGIC:
            rc1 = PROBABLY_NO_HEADER;
            break;

        default:
            rc1 = UpdateCode1(fwPtr);
            break;
    }

    return rc1;
}


/*----------------------------------------------------------------------------
**  Function Name: UpdateCode1 (to be called internally only)
**
**  Comments:  Refer to UpdateCode()
**
**  Parameters: fwPtr - Refer to UpdateCode()
**
**  Returns:    0   - GOOD Completion
**              !0  - Error
**
**--------------------------------------------------------------------------*/
static INT32 UpdateCode1(FW_HEADER *fwPtr)
{
    INT32       rc = PROBABLY_NO_HEADER;

    /*
     * Burn the firmware in CCB flash if required. Handle license
     * files seperately.
     */
    switch (fwPtr->target)
    {
        /* Short circuit this file type here */
        case TARG_ID_NULL:
            return GOOD;

        case TARG_ID_DRIVE_BAY:
            /*
             * All messaging is done in UpdateDriveBay(), so don't
             * return here.
             */
            return UpdateDriveBay(fwPtr);

            /* Hypernode platform RPM's handled here */
        case TARG_ID_PLATFORM_RPM:
            rc = HandlePlatformRPM(fwPtr);
            break;

        default:
            /* Let error be returned. */
            break;
    }

    FWLogResult(fwPtr, rc);

    return rc;
}


/*----------------------------------------------------------------------------
**  Function Name: FWLogResult()
**
**  Comments:      Logs a firmware download result message.
**
**  Parameters:    fwPtr - pointer to the FW in question
**                 reason - reason code
**
**  Returns:       0   - GOOD Completion
**                 !0  - Error
**
**--------------------------------------------------------------------------*/
void FWLogResult(FW_HEADER *fwPtr, INT32 reason)
{
    LOG_FIRMWARE_UPDATE_PKT logMsg;
    LOG_MSG_DELETED_PKT logMsg2;

    memset(&logMsg, 0, sizeof(logMsg));
    memset(&logMsg2, 0, sizeof(logMsg2));
    /*
     * If the burn was successful, log an update message, otherwise log
     * the error event.
     */
    if (reason != PROBABLY_NO_HEADER)
    {
        memcpy(&logMsg.version, &fwPtr->revision, sizeof(fwPtr->revision));
        memcpy(&logMsg.sequence, &fwPtr->revCount, sizeof(fwPtr->revCount));
        memcpy(&logMsg.buildId, &fwPtr->buildID, sizeof(fwPtr->buildID));
        logMsg.targetId = fwPtr->target;
        memcpy(&logMsg.timeStamp, &fwPtr->timeStamp, sizeof(logMsg.timeStamp));
        logMsg.seqNum = 0;
        logMsg.totNum = 0;
        logMsg.crc = fwPtr->checksum;
    }

    if (reason == PROBABLY_NO_HEADER)
    {
        logMsg2.reason = NO_FIRMWARE_HEADER;
        SendAsyncEvent(LOG_MSG_DELETED, sizeof(LOG_MSG_DELETED_PKT), &logMsg2);
    }
    else if (reason == UPDATE_PROC_TIMEOUT)
    {
        SendAsyncEvent(LOG_Warning(LOG_GetCode(LOG_FIRMWARE_UPDATE)),
                       sizeof(logMsg), &logMsg);
    }
    else if (reason == PI_GOOD)
    {
        SendAsyncEvent(LOG_FIRMWARE_UPDATE, sizeof(logMsg), &logMsg);
    }
    else                        /* Any other failure */
    {
        logMsg.reason = (char)reason;
        SendAsyncEvent(LOG_FW_UPDATE_FAILED, sizeof(logMsg), &logMsg);
    }
}

/**
******************************************************************************
**
**  @brief  Apply a Hypernode platform code RPM.
**
**  @param  pFW - dram pointer where firmware header resides.
**
**  @return 0   - GOOD Completion
**          !0  - Error
**
******************************************************************************
**/

#define RPM_FILENAME "/tmp/platformApps.rpm"

#define RPM_CMD "rpm -i -vv --force --nodeps "

static INT32 HandlePlatformRPM(FW_HEADER *pFW)
{
    INT32       rc;
    FILE       *pRPM;
    INT32       length = pFW->length - sizeof(*pFW);

    /*
     * Write out the RPM file to disk
     */
    pRPM = fopen(RPM_FILENAME, "w");
    if (pRPM == NULL)
    {
        dprintf(DPRINTF_DEFAULT, "%s: Couldn't open %s for write. errno %d\n",
                __FUNCTION__, RPM_FILENAME, errno);
        return FAIL;
    }

    rc = fwrite(&pFW[1], 1, length, pRPM);
    Fclose(pRPM);

    if (rc != length)
    {
        dprintf(DPRINTF_DEFAULT, "%s: Failed to write rpm file. errno %d\n",
                __FUNCTION__, errno);
        return FAIL;
    }

    /*
     * Call the "system()" server to apply it
     */
    rc = XK_System(RPM_CMD RPM_FILENAME " >>/var/log/xiotech/pam.log 2>&1");
    if (rc)
    {
        dprintf(DPRINTF_DEFAULT, "%s: RPM failed to apply. rc %d\n", __FUNCTION__, rc);
        return FAIL;
    }

    /*
     * Remove the rpm file
     */
    rc = unlink(RPM_FILENAME);
    if (rc)
    {
        dprintf(DPRINTF_DEFAULT, "%s: Failed to unlink rpm file. errno %d\n",
                __FUNCTION__, errno);
        return FAIL;
    }

    return GOOD;
}


/*----------------------------------------------------------------------------
**  Function Name: UpdateDriveBay
**
**  Comments:  Send the code down to a Drive Bay for it to burn its
**             own flash.
**
**  Parameters: fwPtr - dram pointer where firmware header resides.
**
**  Returns:    0   - GOOD Completion
**              !0  - Error
**
**  NOTE:      This is LEGACY code and should eventually be removed.
**             It was called to update XIOtech bay firmware.
**
**--------------------------------------------------------------------------*/
INT32 UpdateDriveBay(FW_HEADER *fwPtr)
{
    PSES_DEVICE device = GetSESList();
    UINT64     *doneListP = NULL;
    UINT64      nullEntry = 0;
    INT32       idx;
    INT32       okToUpdate;
    LOG_MSG_DELETED_PKT delMsg = { NO_BAYS_FOUND, {0, 0, 0} };
    RC_STATUS   rcStat = { 0, 0 };

    /* Exit if no devices */
    if (device == NULL)
    {
        SendAsyncEvent(LOG_MSG_DELETED, sizeof(delMsg), &delMsg);
        return PI_ERROR;
    }

    /*
     * Check to see if this data file has been XOR'ed or not. Decode if
     * it has. If it has been then the first two characters will be 0x53, 0xB0.
     * If we ever switch to a real binary format for the drive bay ima file,
     * then a change will have to be made so that it doesn't get XOR'ed when
     * the header is pre-pended.
     */
    if (*((UINT16 *)&fwPtr[1]) == 0xB053)
    {

        UINT8      *byteP = (unsigned char *)&fwPtr[1];
        INT32       i;
        INT32       length = fwPtr->length - sizeof(*fwPtr);

        for (i = 0; i < length; i += 2, byteP += 2)
        {
            byteP[1] ^= 0x80;
        }
    }

    /*
     * Malloc a buffer to keep track of the bays already updated.  We
     * make room for a huge number (1024) although that number is probably
     * not even achievable.  Oh well, memory is cheap... (Besides, it would
     * take days to get through all of the drive bays!)
     */
    doneListP = MallocWC(sizeof(UINT64) * 1024);
    memset(doneListP, 0, (sizeof(UINT64) * 1024));

    while (1)
    {
        /* Refresh the SES list */
        device = GetSESList();
        okToUpdate = 0;

        /* Search for the next un-updated device in the SES list */
        while (device)
        {
            /*
             * Search through the "done list" looking for this device's WWN.
             * If it is in the list, then is has been updated so go on to
             * the next one.
             */
            for (idx = 0; idx < 1024; idx++)
            {

                /*
                 * Quit looking when you get to the end of the list. This means
                 * that this device's WWN is not in the list, so go update it.
                 */
                if (memcmp(&nullEntry, &doneListP[idx], sizeof(UINT64)) == 0)
                {
                    /* Keep a copy of this bays WWN so we don't try and update it again. */
                    memcpy(&doneListP[idx], &device->WWN, sizeof(UINT64));

                    okToUpdate = 1;
                    break;
                }

                /*
                 * Also quit looking if you find this device's WWN in the list.
                 * This means that this device has been updated so go get the next
                 * device.
                 */
                if (memcmp(&device->WWN, &doneListP[idx], sizeof(UINT64)) == 0)
                {
                    break;
                }
            }

            if (okToUpdate == 0)
            {
                /* Point to the next SES device */
                device = device->NextSES;
            }
            else
            {
                break;          /* go update the bay */
            }
        }

        /*
         * Exit if through all devices.
         * This is the only way out of the 'while(1)' loop.
         */
        if (device == NULL)
        {
            break;
        }

        rcStat = SCSIWriteBufferMode5(fwPtr, device->WWN, 0);
    }

    /* Cleanup and exit */
    if (doneListP)
    {
        Free(doneListP);
    }

    return (rcStat.rc);

}

/*----------------------------------------------------------------------------
**  Function Name: GetEurologicUpdateStatus
**
**  Comments:   Get the Eurologic bay page 4 "string in" status.
**
**  Parameters: wwn - bay World Wide Name
**              lun - bay Lun
**
**  Returns:    STR_IN_DATA data structure.  The 'status' field in this
**              structure will be '99' on failure.
**
**--------------------------------------------------------------------------*/

#define DRIVE_STR     "Drive"
#define PARTNUM_STR   "Part Number"
#define KERNVER_STR   "Kernel Ver"
#define APPVER_STR    "Application Ver"
#define BUILD_STR     "Build"
#define APPSPACE_STR  "Application Space"
#define RECORDNUM_STR "Record"
#define BYTECNT_STR   "Byte"
#define CHECKSUM_STR  "Checksum"
#define STATUS_STR    "Status"
#define TAGDATA_STR   "Tag Data"

typedef struct
{
    UINT32      drive;
    UINT8       partNum[24];
    UINT8       kernVer[24];
    UINT8       appVer[24];
    UINT8       build[8];
    UINT32      appSpace;
    UINT32      recordNum;
    UINT32      byteCnt;
    UINT32      checksum;
    UINT32      status;
    UINT8       tagData[104];
} STR_IN_DATA;

#define STATUS_READY            0
#define STATUS_IN_PROGRESS      1
// #define STATUS_ERASE_PGM_ERR    2
// #define STATUS_BAD_HEADER       3
// #define STATUS_BAD_SYSTEM       4

/* myStrncpy() is used below to copy field data out of the status string */
static void myStrncpy(UINT8 *to, char *from, UINT32 len)
{
    if (len != 0)
    {
        while (*from != ' ' && *from != 0 && --len)
        {
            *to++ = *from++;
        }
        *to = 0;
    }
}

static STR_IN_DATA GetEurologicUpdateStatus(UINT64 wwn, UINT16 lun)
{
    PSES_PAGE_04 page4 = NULL;
    char       *statusStr = NULL;
    STR_IN_DATA data;
    char       *fieldP = NULL;
    const char *fNameP = NULL;

    /* Get the current FW version and status via page 4 "String In" */
    page4 = GetSESPageByWWN(SES_CMD_STRING_IN, wwn, lun);

    /* Init the local data structure */
    memset(&data, 0, sizeof(data));

    /* Assume failure unless we see otherwise */
    data.status = 99;

    /* Setup the string data pointer */
    if (page4 != NULL)
    {
        statusStr = (char *)&page4[1];
    }
    else
    {
        dprintf(DPRINTF_DEFAULT, "GetEurologicUpdateStatus: GetSES Page4 failed -- this isn't unexpected\n");
        return data;
    }

    /* Parse the input string */
    fNameP = DRIVE_STR;
    fieldP = strstr(statusStr, fNameP);
    if (fieldP)
    {
        fieldP += (strlen(fNameP) + 1);
        data.drive = strtol(fieldP, NULL, 0);
    }

    fNameP = PARTNUM_STR;
    fieldP = strstr(statusStr, fNameP);
    if (fieldP)
    {
        fieldP += (strlen(fNameP) + 1);
        myStrncpy(data.partNum, fieldP, 24);
    }

    fNameP = KERNVER_STR;
    fieldP = strstr(statusStr, fNameP);
    if (fieldP)
    {
        fieldP += (strlen(fNameP) + 1);
        myStrncpy(data.kernVer, fieldP, 24);
    }

    fNameP = APPVER_STR;
    fieldP = strstr(statusStr, fNameP);
    if (fieldP)
    {
        fieldP += (strlen(fNameP) + 1);
        myStrncpy(data.appVer, fieldP, 24);
    }

    fNameP = BUILD_STR;
    fieldP = strstr(statusStr, fNameP);
    if (fieldP)
    {
        fieldP += (strlen(fNameP) + 1);
        myStrncpy(data.build, fieldP, 8);
    }

    fNameP = APPSPACE_STR;
    fieldP = strstr(statusStr, fNameP);
    if (fieldP)
    {
        fieldP += (strlen(fNameP) + 1);
        data.appSpace = strtol(fieldP, NULL, 0);
    }

    fNameP = RECORDNUM_STR;
    fieldP = strstr(statusStr, fNameP);
    if (fieldP)
    {
        fieldP += (strlen(fNameP) + 1);
        data.recordNum = strtol(fieldP, NULL, 0);
    }

    fNameP = BYTECNT_STR;
    fieldP = strstr(statusStr, fNameP);
    if (fieldP)
    {
        fieldP += (strlen(fNameP) + 1);
        data.byteCnt = strtol(fieldP, NULL, 0);
    }

    fNameP = CHECKSUM_STR;
    fieldP = strstr(statusStr, fNameP);
    if (fieldP)
    {
        fieldP += (strlen(fNameP) + 1);
        data.checksum = strtol(fieldP, NULL, 0);
    }

    fNameP = STATUS_STR;
    fieldP = strstr(statusStr, fNameP);
    if (fieldP)
    {
        fieldP += (strlen(fNameP) + 1);
        data.status = strtol(fieldP, NULL, 0);
    }

    fNameP = TAGDATA_STR;
    fieldP = strstr(statusStr, fNameP);
    if (fieldP)
    {
        fieldP += (strlen(fNameP) + 1);
        myStrncpy(data.tagData, fieldP, 104);
    }

    dprintf(DPRINTF_CODE_UPDATE, "GetEurologicUpdateStatus: Status = %d\n", data.status);

    /* Free the data memory */
    Free(page4);

    return data;
}

/*----------------------------------------------------------------------------
**  Function Name: UpdateEurologicBaySingle
**
**  Comments:  Send the code down to a Eurologic Drive Bay for it to burn its
**             own flash.
**
**  Parameters: fwPtr - dram pointer where firmware header resides.
**              wwn - device World Wide Name
**              lun - device Lun
**
**  Returns:    0   - GOOD Completion
**              !0  - Error
**
**--------------------------------------------------------------------------*/

#define EURO_PAGE4_START_OF_DOWNLOAD    0x0C
#define EURO_PAGE4_DOWNLOAD_COMPLETE    0x0E
#define EURO_PAGE4_DOWNLOAD_FW_RECORD   0x22
#define EURO_PAGE4_SWAP_CONTROLLERS     0x24    /* Execute Failover */

#define EURO_BAY_FW_RETRY_DELAY         5000

#define EURO_BAY_FW_DOWNLOAD_SZ         2000
#define EURO_BAY_FW_RECORD_DELAY        1000

#define SATA_ES_FW_DOWNLOAD_SZ          1000
#define SATA_ES_FW_RECORD_DELAY         2000

RC_STATUS UpdateEurologicBaySingle(FW_HEADER *fwPtr, UINT64 wwn, UINT32 lun)
{
    INT32       cardNum = 0;
    INT32       retryCount;
    INT32       rc = PI_ERROR;
    INT32       length;
    UINT8      *bufP;
    UINT16      sendLen;
    UINT32      sentBytes = 0;
    RC_STATUS   rcStat = { 0, 0 };
    STR_IN_DATA status;
    SES_PAGE_04_FW *page4 = NULL;
    LOG_DRIVE_BAY_FW_UPDATE_PKT fwUpdate;
    LOG_FIRMWARE_UPDATE_PKT logMsg;
    UINT32      blockSize;
    UINT32      recordDelay;
    UINT32      initialAppSpace;

    memset(&logMsg, 0, sizeof(logMsg));
    memset(&fwUpdate, 0, sizeof(fwUpdate));

    switch (fwPtr->target)
    {
        case TARG_ID_EUROLOGIC_BAY:
            blockSize = EURO_BAY_FW_DOWNLOAD_SZ;
            recordDelay = EURO_BAY_FW_RECORD_DELAY;
            break;

        case TARG_ID_ADAPTEC_SATA_ES_BAY:
            blockSize = SATA_ES_FW_DOWNLOAD_SZ;
            recordDelay = SATA_ES_FW_RECORD_DELAY;
            break;

        default:
            dprintf(DPRINTF_CODE_UPDATE, "UpdateEurologicBaySingle: Unknown target ID (0x%X), aborting.\n",
                    fwPtr->target);
            rcStat.rc = PI_ERROR;
            rcStat.status = PARM_ERROR;
            return rcStat;
    }

    do {
        dprintf(DPRINTF_CODE_UPDATE, "%s: Downloading FW to "
                "WWN %08X%08X LUN %u, card %u\n", __func__,
                bswap_32((UINT32)wwn), bswap_32((UINT32)(wwn >> 32)),
                lun, cardNum);

        /* Malloc storage for the data xfer */
        if (!page4)
        {
            page4 = MallocSharedWC(sizeof(*page4) + blockSize);
            if (page4 == NULL)
            {
                rcStat.rc = PI_ERROR;
                rcStat.status = MALLOC_FAILURE;
                break;
            }
        }

        /* Setup initial page stuff */
        page4->pageCode = 4;
        page4->subEnclosure = 0;

        /* Get initial version string */
        status = GetEurologicUpdateStatus(wwn, lun);
        initialAppSpace = status.appSpace;
        dprintf(DPRINTF_CODE_UPDATE, "UpdateEurologicBaySingle: Initial AppVer: %s, Build: %s, AppSpace: %u\n",
                status.appVer, status.build, status.appSpace);

        /*
         * Issue "Start of Download".  Issue it twice, because if the previous
         * download ended in error, it takes twice to reset it.
         */
        page4->length = htons(1);
        page4->command = EURO_PAGE4_START_OF_DOWNLOAD;

        SendSESPageByWWN(wwn, lun, page4);
        TaskSleepMS(recordDelay);
        rc = SendSESPageByWWN(wwn, lun, page4);
        TaskSleepMS(recordDelay);

        if (rc != PI_GOOD)
        {
            dprintf(DPRINTF_DEFAULT, "UpdateEurologicBaySingle: Send \"Start of Download\" failed, rc = %d\n",
                    rc);
            rcStat.rc = rc;
            rcStat.status = PROGRAM_VERIFY_ERROR;
            break;
        }

        status = GetEurologicUpdateStatus(wwn, lun);
        if (status.status != STATUS_READY)
        {
            dprintf(DPRINTF_DEFAULT, "UpdateEurologicBaySingle: Expected status 1, actual status %d (fail)\n",
                    status.status);
            rcStat.rc = PI_ERROR;
            rcStat.status = PROGRAM_VERIFY_ERROR;
            break;
        }

        /*
         * Loop on "Download Binary FW Record" over binary length,
         * 2000 bytes at a time, 1 second pause
         */
        bufP = (UINT8 *)&fwPtr[1];
        page4->command = EURO_PAGE4_DOWNLOAD_FW_RECORD;
        length = fwPtr->length - sizeof(*fwPtr);
        while (length > 0)
        {
            /* Calcualte length to send this time */
            sendLen = ((UINT32)length > blockSize ? blockSize : (UINT32)length);
            page4->length = htons(sendLen + 1);

            /* Copy in the data */
            memcpy(page4->data, bufP, blockSize);

            /* Send it */
            rc = SendSESPageByWWN(wwn, lun, page4);
            TaskSleepMS(recordDelay);

            if (rc != PI_GOOD)
            {
                dprintf(DPRINTF_DEFAULT, "UpdateEurologicBaySingle: Send \"FW Record\" failed, rc = %d\n",
                        rc);
                rcStat.rc = rc;
                rcStat.status = PROGRAM_VERIFY_ERROR;
                break;
            }

            status = GetEurologicUpdateStatus(wwn, lun);
            if (status.status != STATUS_IN_PROGRESS)
            {
                dprintf(DPRINTF_DEFAULT, "UpdateEurologicBaySingle: Expected status 1, actual status %d (fail)\n",
                        status.status);
                rcStat.rc = PI_ERROR;
                rcStat.status = PROGRAM_VERIFY_ERROR;
                break;
            }

            /* Adjust counters */
            length -= sendLen;
            bufP += sendLen;

            sentBytes = fwPtr->length - sizeof(*fwPtr) - length;
            if ((sentBytes / blockSize) % 10 == 0)
            {
                dprintf(DPRINTF_CODE_UPDATE, "UpdateEurologicBaySingle: Bytes sent: %u, bytes recv'd: %u\n",
                        sentBytes, status.byteCnt);
            }
            if (sentBytes != status.byteCnt)
            {
                dprintf(DPRINTF_DEFAULT, "UpdateEurologicBaySingle: bytes sent != bytes recv'd (fail)\n");
                rcStat.rc = PI_ERROR;
                rcStat.status = PROGRAM_VERIFY_ERROR;
                break;
            }
        }

        /* If break in above loop, break to bottom of do..while() */
        if (rcStat.status)
        {
            break;
        }

        dprintf(DPRINTF_CODE_UPDATE, "UpdateEurologicBaySingle: Bytes sent: %u, bytes recv'd: %u\n",
                sentBytes, status.byteCnt);

        /* Issue "Download Complete" */
        page4->length = htons(1);
        page4->command = EURO_PAGE4_DOWNLOAD_COMPLETE;

        rc = SendSESPageByWWN(wwn, lun, page4);
        TaskSleepMS(recordDelay);

        if (rc != PI_GOOD)
        {
            dprintf(DPRINTF_DEFAULT, "UpdateEurologicBaySingle: Send \"Download Complete\" failed, rc = %d\n",
                    rc);
            rcStat.rc = rc;
            rcStat.status = PROGRAM_VERIFY_ERROR;
            break;
        }

        dprintf(DPRINTF_DEFAULT, "UpdateEurologicBaySingle: Restarting card %u, waiting for it to come back up ...\n",
                cardNum);

        retryCount = 12;
        while (retryCount--)
        {
            /* Wait a bit before looking for status, as the bay is rebooting */
            TaskSleepMS(EURO_BAY_FW_RETRY_DELAY);

            status = GetEurologicUpdateStatus(wwn, lun);
            if (status.status == STATUS_READY)
            {
                break;
            }
        }

        /* Get final version string. */
        status = GetEurologicUpdateStatus(wwn, lun);
        dprintf(DPRINTF_CODE_UPDATE, "UpdateEurologicBaySingle: Final AppVer: %s, Build: %s, AppSpace: %u\n",
                status.appVer, status.build, status.appSpace);

        /* Did it ever come back up? */
        if (status.status != STATUS_READY)
        {
            dprintf(DPRINTF_DEFAULT, "UpdateEurologicBaySingle: Card restart failed, rc = %d\n",
                    status.status);
            rcStat.rc = PI_ERROR;
            rcStat.status = PROGRAM_VERIFY_ERROR;
            break;
        }

        /* If initial appSpace == final appSpace, something went wrong */
        if (status.appSpace == initialAppSpace)
        {
            dprintf(DPRINTF_DEFAULT, "UpdateEurologicBaySingle: Flash update failure, rc = %d\n",
                    status.status);
            rcStat.rc = PI_ERROR;
            rcStat.status = PROGRAM_VERIFY_ERROR;
            break;
        }

        /* Bail now if not EUROLOGIC FIBRE BAY */
        if (fwPtr->target != TARG_ID_EUROLOGIC_BAY)
        {
            break;
        }

        /* Issue "Swap Controllers/Execute Failover" */
        if (cardNum == 0)
        {
            dprintf(DPRINTF_DEFAULT, "UpdateEurologicBaySingle: Swapping to 2nd card ...\n");

            page4->length = htons(1);
            page4->command = EURO_PAGE4_SWAP_CONTROLLERS;

            retryCount = 12;
            while (retryCount--)
            {
                TaskSleepMS(EURO_BAY_FW_RETRY_DELAY);
                rc = SendSESPageByWWN(wwn, lun, page4);
                if (rc == PI_GOOD)
                {
                    break;
                }
            }

            /* Did the command complete successfully? */
            if (rc != PI_GOOD)
            {
                dprintf(DPRINTF_DEFAULT, "UpdateEurologicBaySingle: Send \"Execute Failover\" failed, rc = %d\n",
                        rc);
                rcStat.rc = rc;
                rcStat.status = PROGRAM_VERIFY_ERROR;
                break;
            }

            retryCount = 12;
            while (retryCount--)
            {
                /* Wait a bit before looking for status, as the bay is failing over */
                TaskSleepMS(EURO_BAY_FW_RETRY_DELAY);

                status = GetEurologicUpdateStatus(wwn, lun);
                if (status.status == STATUS_READY)
                {
                    break;
                }
            }

            /* Did it ever come back up?  */
            if (status.status != STATUS_READY)
            {
                dprintf(DPRINTF_DEFAULT, "UpdateEurologicBaySingle: Card failover failed, rc = %d\n",
                        status.status);
                rcStat.rc = PI_ERROR;
                rcStat.status = PROGRAM_VERIFY_ERROR;
                break;
            }
        }

    } while (++cardNum < 2);

    /* cleanup */
    Free(page4);

    /* Log a PASS/FAIL message */
    if (rc == PI_GOOD)
    {
        memcpy(&fwUpdate.version, &fwPtr->revision, sizeof(fwPtr->revision));
        memcpy(&fwUpdate.sequence, &fwPtr->revCount, sizeof(fwPtr->revCount));
        memcpy(&fwUpdate.buildId, &fwPtr->buildID, sizeof(fwPtr->buildID));
        fwUpdate.targetId = fwPtr->target;
        memcpy(&fwUpdate.timeStamp, &fwPtr->timeStamp, sizeof(fwUpdate.timeStamp));
        fwUpdate.crc = fwPtr->checksum;
        memcpy(&fwUpdate.wwn, &wwn, sizeof(UINT64));
        SendAsyncEvent(LOG_DRIVE_BAY_FW_UPDATE, sizeof(fwUpdate), &fwUpdate);
    }
    else
    {
        memcpy(&logMsg.version, &fwPtr->revision, sizeof(fwPtr->revision));
        memcpy(&logMsg.sequence, &fwPtr->revCount, sizeof(fwPtr->revCount));
        memcpy(&logMsg.buildId, &fwPtr->buildID, sizeof(fwPtr->buildID));
        logMsg.targetId = fwPtr->target;
        memcpy(&logMsg.timeStamp, &fwPtr->timeStamp, sizeof(logMsg.timeStamp));
        logMsg.seqNum = 0;
        logMsg.totNum = 0;
        logMsg.crc = fwPtr->checksum;
        logMsg.reason = (char)rc;
        SendAsyncEvent(LOG_FW_UPDATE_FAILED, sizeof(logMsg), &logMsg);
    }

    return rcStat;
}


/*----------------------------------------------------------------------------
**  Function Name: UpdateXyratexBay
**
**  Comments:  Send the code down to a Xyratex Bay for it to burn its
**             own flash.
**
**  Parameters: pFwHdr - dram pointer where firmware header resides.
**              wwn - device World Wide Name
**              lun - device Lun
**
**  Returns:    0   - GOOD Completion
**              !0  - Error
**
**--------------------------------------------------------------------------*/
#define XYRATEX_FW_DOWNLOAD_SZ          468
#define XYRATEX_FW_RECORD_DELAY         100
RC_STATUS UpdateXyratexBay(FW_HEADER *pFwHdr, UINT64 wwn, UINT32 lun)
{
    RC_STATUS   rcStat;
    INT32       rc = PI_GOOD;
    INT32       status = 0;
    LOG_DRIVE_BAY_FW_UPDATE_PKT fwUpdate;
    LOG_FIRMWARE_UPDATE_PKT logMsg;
    UINT32      mutexLocked = 0;

    memset(&logMsg, 0, sizeof(logMsg));
    memset(&fwUpdate, 0, sizeof(fwUpdate));

    do
    {
        /*
         * Synchronize with the SES monitor.
         * Actually, this does almost no good since the other controller can
         * be polling the bay at the same time and it is not being blocked.
         * This should probably be removed (if it isn't needed) OR expanded to
         * stop the other controller from polling at the same time.
         */
        (void)LockMutex(&sesMutex, MUTEX_WAIT);
        mutexLocked = 1;

        /* See if bay is ready for a download */
        if (MonXyratexBayStatus(wwn, lun, MON_BAY_STATUS_START, 10) != PI_GOOD)
        {
            rc = PI_ERROR;
            status = DISKBAY_STATE_BAD;
            break;
        }

        /* Download the FW to first card */
        dprintf(DPRINTF_CODE_UPDATE, "UpdateXyratexBay: Downloading FW to WWN %08X%08X LUN %u, Pass 1/2\n",
                ntohl((UINT32)wwn), ntohl((UINT32)(wwn >> 32)), lun);
        rcStat = SendFWToXyratexBay(pFwHdr, wwn, lun);
        if (rcStat.rc != PI_GOOD)
        {
            rc = rcStat.rc;
            status = rcStat.status;
            break;
        }

        /* Wait 90 sec then get status */
        dprintf(DPRINTF_CODE_UPDATE, "%s: Sleeping 90 seconds\n", __FUNCTION__);
        TaskSleepMS(90000);     /* Sleep between controllers */

        /*
         * Unlock the mutex and let the SES monitor run. Also wait up to
         * 2 minutes for the bay to switch over and the loops to resume
         * normal operation.
         */
        UnlockMutex(&sesMutex);
        mutexLocked = 0;
        dprintf(DPRINTF_CODE_UPDATE, "UpdateXyratexBay: Pass 1/2 complete.");

        if (MonXyratexBayStatus(wwn, lun, MON_BAY_STATUS_MID, 120) != PI_GOOD)
        {
            rc = PI_ERROR;
            status = DISKBAY_STATE_BAD;
            break;
        }

        (void)LockMutex(&sesMutex, MUTEX_WAIT);
        mutexLocked = 1;

        /* Download the FW to second card */
        dprintf(DPRINTF_CODE_UPDATE, "UpdateXyratexBay: Downloading FW to WWN %08X%08X LUN %u, Pass 2/2\n",
                ntohl((UINT32)wwn), ntohl((UINT32)(wwn >> 32)), lun);
        rcStat = SendFWToXyratexBay(pFwHdr, wwn, lun);
        if (rcStat.rc != PI_GOOD)
        {
            rc = rcStat.rc;
            status = rcStat.status;
            break;
        }

        /* Wait 90 sec then get status */
        dprintf(DPRINTF_CODE_UPDATE, "%s: Sleeping 90 seconds\n", __FUNCTION__);
        TaskSleepMS(90000);

        /*
         * Wait up to 2 minutes for the bay to switch back and the loops
         * to resume normal operation.
         */
        if (MonXyratexBayStatus(wwn, lun, MON_BAY_STATUS_FINAL, 120) != PI_GOOD)
        {
            rc = PI_ERROR;
            status = DISKBAY_STATE_BAD;
        }

    } while (0);

    if (mutexLocked)
    {
        UnlockMutex(&sesMutex);
    }

    /* Log a PASS/FAIL message */
    if (rc == PI_GOOD)
    {
        memcpy(&fwUpdate.version, &pFwHdr->revision, sizeof(pFwHdr->revision));
        memcpy(&fwUpdate.sequence, &pFwHdr->revCount, sizeof(pFwHdr->revCount));
        memcpy(&fwUpdate.buildId, &pFwHdr->buildID, sizeof(pFwHdr->buildID));
        fwUpdate.targetId = pFwHdr->target;
        memcpy(&fwUpdate.timeStamp, &pFwHdr->timeStamp, sizeof(fwUpdate.timeStamp));
        fwUpdate.crc = pFwHdr->checksum;
        memcpy(&fwUpdate.wwn, &wwn, sizeof(UINT64));
        SendAsyncEvent(LOG_DRIVE_BAY_FW_UPDATE, sizeof(fwUpdate), &fwUpdate);
    }
    else
    {
        memcpy(&logMsg.version, &pFwHdr->revision, sizeof(pFwHdr->revision));
        memcpy(&logMsg.sequence, &pFwHdr->revCount, sizeof(pFwHdr->revCount));
        memcpy(&logMsg.buildId, &pFwHdr->buildID, sizeof(pFwHdr->buildID));
        logMsg.targetId = pFwHdr->target;
        memcpy(&logMsg.timeStamp, &pFwHdr->timeStamp, sizeof(logMsg.timeStamp));
        logMsg.seqNum = 0;
        logMsg.totNum = 0;
        logMsg.crc = pFwHdr->checksum;
        logMsg.reason = (char)rc;
        SendAsyncEvent(LOG_FW_UPDATE_FAILED, sizeof(logMsg), &logMsg);
    }

    /* Kick off a rescan to update the page stuff for the bay */
    SM_RescanDevices(GetMyControllerSN(), RESCAN_EXISTING);

    rcStat.rc = rc;
    rcStat.status = status;
    return rcStat;
}

/*----------------------------------------------------------------------------
**  Function Name: SendFWToXyratexBay
**
**  Comments:  Do the actual send to the Xyratex Bay.
**
**  Parameters: pFwHdr - dram pointer where firmware header resides.
**              wwn - device World Wide Name
**              lun - device Lun
**
**  Returns:    0   - GOOD Completion
**              !0  - Error
**
**--------------------------------------------------------------------------*/
RC_STATUS SendFWToXyratexBay(FW_HEADER *pFwHdr, UINT64 wwn, UINT32 lun)
{
    RC_STATUS   rcStat;
    INT32       rc = PI_GOOD;
    INT32       status = 0;
    INT32       length;
    INT32       offset = 0;
    INT32       sendCounter = 0;
    UINT8      *pBuf;
    UINT16      sendLen;
    UINT8       retries;
    UINT32      sentBytes = 0;
    SES_PAGE_04_FW_XYRATEX *pPage4 = NULL;
    UINT32      blockSize = XYRATEX_FW_DOWNLOAD_SZ;
    UINT32      recordDelay = XYRATEX_FW_RECORD_DELAY;

    do
    {
        /* Malloc storage for the data xfer */
        if (!pPage4)
        {
            pPage4 = MallocSharedWC(sizeof(*pPage4) + blockSize);
            if (pPage4 == NULL)
            {
                rc = PI_ERROR;
                status = MALLOC_FAILURE;
                break;
            }
        }

        /* Setup initial page stuff */
        pPage4->pageCode = 4;
        pPage4->subEnclosure = 0;

        /*
         * Loop on "Download Binary FW Record" over binary length,
         * 468 bytes at a time, 100 mS pause
         */
        pBuf = (UINT8 *)&pFwHdr[1];
        length = pFwHdr->length - sizeof(*pFwHdr);
        while (length > 0)
        {
            /* Calculate length to send this time */
            sendLen = ((UINT32)length > blockSize ? blockSize : (UINT32)length);
            pPage4->length = htons(sendLen + 4);        /* add in length of offset */
            pPage4->offset = htonl(offset);

            /* Copy in the data */
            memcpy(pPage4->data, pBuf, blockSize);

            /* Send it! */
            retries = 3;
            while (retries--)
            {
                rc = SendSESPageByWWN(wwn, lun, pPage4);
                if (rc == PI_GOOD)
                {
                    break;
                }
                else
                {
                    dprintf(DPRINTF_DEFAULT, "UpdateXyratexBay: Send \"FW Record\" failed, rc = %d. Retrying...\n",
                            rc);
                    TaskSleepMS(1000);  /* wait a second */
                }
            }

            if (rc != PI_GOOD)
            {
                rc = rc;
                status = PROGRAM_VERIFY_ERROR;
                break;
            }

            /* Delay between sends */
            TaskSleepMS(recordDelay);

            /* Adjust counters */
            sendCounter++;
            length -= sendLen;
            pBuf += sendLen;
            offset += sendLen;

            sentBytes = pFwHdr->length - sizeof(*pFwHdr) - length;
            if (sendCounter % 43 == 0)
            {
                dprintf(DPRINTF_CODE_UPDATE, "UpdateXyratexBay: Bytes sent: %u\n",
                        sentBytes);
            }
        }

        /* If break in above loop, break to bottom of do..while() */
        if (status)
        {
            break;
        }

        dprintf(DPRINTF_CODE_UPDATE, "UpdateXyratexBay: Bytes sent: %u\n", sentBytes);

    } while (0);

    /* cleanup */
    Free(pPage4);

    rcStat.rc = rc;
    rcStat.status = status;
    return rcStat;
}


/*----------------------------------------------------------------------------
**  Function Name: GetXyratexBayStatus
**
**  Comments:  Retrieve the  Xyratex Bay status info.
**
**  Parameters:  wwn - device World Wide Name
**               lun - device Lun
**               BAY_CONTROLLER_STATUS pointer to write the data to
**
**  Returns:    GOOD or ERROR.
**
**--------------------------------------------------------------------------*/
INT32 GetXyratexBayStatus(UINT64 wwn, UINT16 lun, BAY_CONTROLLER_STATUS *pBayStats)
{
    INT32       rc = PI_GOOD;
    SES_DEVICE *pSES = NULL;
    SES_PAGE_01 *pPage1 = NULL;
    SES_PAGE_02 *pPage2 = NULL;
    BAY_CONTROLLER_STATUS bayStats;
    SES_ENCL_DESC *pEncl;
    SES_ELEM_CTRL *pElem;

    memset(&bayStats, 0, sizeof(bayStats));

    do
    {
        if (pBayStats == NULL)
        {
            dprintf(DPRINTF_DEFAULT, "GetXyratexBayStatus: pBayStats == NULL!\n");
            rc = PI_ERROR;
            break;
        }

        /* Read the page 1 data to get the latest fw version */
        pPage1 = GetSESPageByWWN(SES_CMD_CONFIG, wwn, lun);
        if (pPage1 == NULL)
        {
            dprintf(DPRINTF_DEFAULT, "GetXyratexBayStatus: Failed to get page 1 data!\n");
            rc = PI_ERROR;
            break;
        }

        /* Set pointer to start of enclosure stuff */
        pEncl = (SES_ENCL_DESC *) & pPage1[1];

        /*
         * Get the SES_DEVICE data for this bay so that we can get at the
         * map and slot info.
         */
        pSES = GetSESList();
        dprintf(DPRINTF_SES, "GetXyratexBayStatus: This bay's  WWN %08X%08X, Lun %u\n",
                ntohl((UINT32)wwn), ntohl((UINT32)(wwn >> 32)), lun);
        while (pSES != NULL)
        {
            dprintf(DPRINTF_SES, "GetXyratexBayStatus: The SESList WWN %08X%08X, Lun %u\n",
                    ntohl((UINT32)pSES->WWN), ntohl((UINT32)(pSES->WWN >> 32)),
                    pSES->LUN);

            /* Proceed if we have the correct bay info. */
            if ((pSES->WWN == wwn) && (pSES->LUN == lun))
            {
                dprintf(DPRINTF_SES, "GetXyratexBayStatus: Found the correct bay in the list");
                break;
            }
            else
            {
                pSES = pSES->NextSES;
            }
        }
        if (pSES == NULL)
        {
            dprintf(DPRINTF_DEFAULT, "GetXyratexBayStatus: Bay NOT found!\n");
            rc = PI_ERROR;
            break;
        }

        /* Go get the page 2 element status */
        pPage2 = GetSESPageByWWN(SES_CMD_STATUS, wwn, lun);
        if (pPage2 == NULL)
        {
            dprintf(DPRINTF_DEFAULT, "GetXyratexBayStatus: Failed to get page 2 data!\n");
            rc = PI_ERROR;
            break;
        }

        if (pSES->Map[SES_ET_SES_ELEC] != SES_ET_INVALID &&
            pSES->Slots[SES_ET_SES_ELEC] >= 2)
        {
            /* Point to first individual element */
            UINT32      offset = pSES->Map[SES_ET_SES_ELEC] + 1;

            pElem = (SES_ELEM_CTRL *) & (pPage2->Control[offset]);

            if (pElem->Ctrl.Electronics.Ctrl)
            {
                bayStats.active |= 0x01;
            }

            /*
             * Get the status of the element ignoring whether or not is was
             * previously swapped.
             */
            bayStats.status0 = pElem->CommonCtrl & ~SES_CC_SWAP;

            pElem++;
            if (pElem->Ctrl.Electronics.Ctrl)
            {
                bayStats.active |= 0x02;
            }

            /*
             * Get the status of the element ignoring whether or not is was
             * previously swapped.
             */
            bayStats.status1 = pElem->CommonCtrl & ~SES_CC_SWAP;

            /* Add in FW version */
            memcpy(bayStats.fwVer0, &pEncl->ProductRev[0], 2);  /* copy in the FW level */
            bayStats.fwVer0[2] = bayStats.fwVer0[3] = 0;        /* zero out the rest */
            memcpy(bayStats.fwVer1, &pEncl->ProductRev[2], 2);  /* copy in the FW level */
            bayStats.fwVer1[2] = bayStats.fwVer1[3] = 0;        /* zero out the rest */

            /* Print this status message each pass */
            dprintf(DPRINTF_DEFAULT, "GetXyratexBayStatus: active = 0x%02hhX, fw0 = \"%s\", status0 = %u, fw1 = \"%s\", status1 = %u\n",
                    bayStats.active, bayStats.fwVer0, bayStats.status0, bayStats.fwVer1,
                    bayStats.status1);
        }
        else
        {
            dprintf(DPRINTF_DEFAULT, "GetXyratexBayStatus: No electronics elements found in page 2 data!\n");
            rc = PI_ERROR;
            break;
        }

    } while (0);

    if (pPage1)
    {
        Free(pPage1);
    }
    if (pPage2)
    {
        Free(pPage2);
    }

    /* Copy the bay stats into the callers storage */
    *pBayStats = bayStats;

    return rc;
}


/*----------------------------------------------------------------------------
**  Function Name: MonXyratexBayStatus
**
**  Comments:  Test the Xyratex Bay status info based upon what it should be
**             during a FW download.
**
**  Parameters:  wwn - device World Wide Name
**               lun - device Lun
**               how - that is, either: start, mid or finish
**
**  Returns:    GOOD or ERROR.
**
**--------------------------------------------------------------------------*/
INT32 MonXyratexBayStatus(UINT64 wwn, UINT16 lun, UINT32 how, UINT32 timeoutSec)
{
    INT32       rc = PI_ERROR;
    BAY_CONTROLLER_STATUS bayStatus;
    UINT32      startTime = RTC_GetSystemSeconds();
    UINT32      timeout = startTime + timeoutSec;
    INT32       elapsed;
    UINT32      cardNum;
    static UINT32 initialCardNum;
    char        resultMsg[256];

    dprintf(DPRINTF_DEFAULT, "MonXyratexBayStatus: Entry. how = %u, timeout = %u\n",
            how, timeoutSec);

    while (timeout >= RTC_GetSystemSeconds())
    {
        rc = PI_GOOD;

        /* Take a 5 sec nap */
        TaskSleepMS(5000);

        /* Get status. */
        rc = GetXyratexBayStatus(wwn, lun, &bayStatus);
        if (rc != PI_GOOD)
        {
            sprintf(resultMsg, "Failed to get bay status.");
            rc = PI_ERROR;
            continue;
        }

        if (bayStatus.active & ~0x3)
        {
            sprintf(resultMsg, "More than one controller active (%u).", bayStatus.active);
            rc = PI_ERROR;
            continue;
        }

        if (bayStatus.status0 != SES_CC_STAT_OK || bayStatus.status1 != SES_CC_STAT_OK)
        {
            sprintf(resultMsg, "One or more controllers not OK (%u, %u).",
                    bayStatus.status0, bayStatus.status1);
            rc = PI_ERROR;
            continue;
        }

        cardNum = bayStatus.active - 1;
        switch (how)
        {
            case MON_BAY_STATUS_START:
                initialCardNum = cardNum;
                break;

            case MON_BAY_STATUS_MID:
                if (initialCardNum == cardNum)
                {
                    sprintf(resultMsg, "Controller did not fail over to alternate (%u %u).",
                            initialCardNum, cardNum);
                    rc = PI_ERROR;
                    continue;
                }
                break;

            case MON_BAY_STATUS_FINAL:
                if (initialCardNum != cardNum)
                {
                    sprintf(resultMsg, "Controller did not fail back to original (%u %u).",
                            initialCardNum, cardNum);
                    rc = PI_ERROR;
                    continue;
                }

                if (*(UINT16 *)bayStatus.fwVer0 != *(UINT16 *)bayStatus.fwVer1)
                {
                    sprintf(resultMsg, "Final FW versions do not match (%s, %s).",
                            bayStatus.fwVer0, bayStatus.fwVer1);
                    rc = PI_ERROR;
                    continue;
                }
                break;
        }

        /* If we make it here, all is well, get out! */
        sprintf(resultMsg, "OK.");
        break;
    }

    elapsed = RTC_GetSystemSeconds() - startTime;

    dprintf(DPRINTF_DEFAULT, "MonXyratexBayStatus: Exit. Elapsed: %d sec, rc: %d, %s\n",
            elapsed, rc, resultMsg);
    return rc;
}



/*----------------------------------------------------------------------------
**  Function Name: SCSIWriteBufferMode5
**
**  Comments:  Send the code down to a Drive or Drive Bay for it to burn its
**             own flash.
**
**  Parameters: fwPtr - dram pointer where firmware header resides.
**              wwn - device World Wide Name
**              lun - device Lun
**
**  Returns:    0   - GOOD Completion
**              !0  - Error
**
**--------------------------------------------------------------------------*/
RC_STATUS SCSIWriteBufferMode5(FW_HEADER *fwPtr, UINT64 wwn, UINT32 lun)
{
    INT32       rc = PI_GOOD;
    LOG_DRIVE_BAY_FW_UPDATE_PKT fwUpdate;
    LOG_FIRMWARE_UPDATE_PKT logMsg;
    SCSI_CMD_STATUS scsiStat;
    RC_STATUS   rcStat = { 0, 0 };

    memset(&fwUpdate, 0, sizeof(fwUpdate));
    memset(&logMsg, 0, sizeof(logMsg));

    /*
     * Check to see if this data file has been XOR'ed or not. Decode if
     * it has. If it has been then the first two characters will be 0x53, 0xB0.
     * If we ever switch to a real binary format for the drive bay ima file,
     * then a change will have to be made so that it doesn't get XOR'ed when
     * the header is pre-pended.
     */
    if (fwPtr->target == TARG_ID_DRIVE_BAY && *((UINT16 *)&fwPtr[1]) == 0xB053)
    {
        UINT8      *byteP = (unsigned char *)&fwPtr[1];
        INT32       i;
        INT32       length = fwPtr->length - sizeof(*fwPtr);

        for (i = 0; i < length; i += 2, byteP += 2)
        {
            byteP[1] ^= 0x80;
        }
    }

    /* Call the low-level function that does the write */
    scsiStat = SCSIWriteBuffer((char *)&fwPtr[1], fwPtr->length - sizeof(*fwPtr), wwn, lun, 5,  /* mode */
                               5,       /* retries */
                               0,       /* offset */
                               MRP_UPDATE_DRIVE_BAY_TIMEOUT);
    rc = scsiStat.rc;

    /* Log a PASS/FAIL message */
    if (rc == PI_GOOD)
    {
        memcpy(&fwUpdate.version, &fwPtr->revision, sizeof(fwPtr->revision));
        memcpy(&fwUpdate.sequence, &fwPtr->revCount, sizeof(fwPtr->revCount));
        memcpy(&fwUpdate.buildId, &fwPtr->buildID, sizeof(fwPtr->buildID));
        fwUpdate.targetId = fwPtr->target;
        memcpy(&fwUpdate.timeStamp, &fwPtr->timeStamp, sizeof(fwUpdate.timeStamp));
        fwUpdate.crc = fwPtr->checksum;
        memcpy(&fwUpdate.wwn, &wwn, sizeof(UINT64));
        SendAsyncEvent(LOG_DRIVE_BAY_FW_UPDATE, sizeof(fwUpdate), &fwUpdate);
    }
    else
    {
        memcpy(&logMsg.version, &fwPtr->revision, sizeof(fwPtr->revision));
        memcpy(&logMsg.sequence, &fwPtr->revCount, sizeof(fwPtr->revCount));
        memcpy(&logMsg.buildId, &fwPtr->buildID, sizeof(fwPtr->buildID));
        logMsg.targetId = fwPtr->target;
        memcpy(&logMsg.timeStamp, &fwPtr->timeStamp, sizeof(logMsg.timeStamp));
        logMsg.seqNum = 0;
        logMsg.totNum = 0;
        logMsg.crc = fwPtr->checksum;
        logMsg.reason = (char)rc;
        SendAsyncEvent(LOG_FW_UPDATE_FAILED, sizeof(logMsg), &logMsg);
    }

    rcStat.rc = scsiStat.rc;
    rcStat.status = scsiStat.status;
    return rcStat;
}

/*----------------------------------------------------------------------------
**  Function Name: UpdateAdaptecSataBaySingle
**
**  Comments:  Send the code down to a Drive Bay for it to burn its
**             own flash.
**
**  Parameters: fwPtr - dram pointer where firmware header resides.
**
**  Returns:    0   - GOOD Completion
**              !0  - Error
**
**--------------------------------------------------------------------------*/
#define ARIO_MAGIC_CODE  0x10292001
#define CHUNK_SIZE       0x1000

RC_STATUS UpdateAdaptecSataBaySingle(FW_HEADER *fwPtr, UINT64 wwn, UINT32 lun)
{
    /*
     * Copied from the Ario example code:
     *
     * Ario transport header is used to carry payload such as fw download
     * segments.  Fw download segments are encapsulated in the Ario transport
     * header.  The Ario transport header is generic so that the transport is not
     * aware of the payload or the medium.  The transport is responsible for
     * delivering the payload to the target application on the board.
     *
     * Ario transport header contains:
     * MagicCode    This code somewhat ensures that the payload destined for an
     *              Ario application.
     * TargetApplicationId  This Id is either pre-assigned or is acquired at runtime.
     *              This id defines which application on the board is the recipeint
     *              for this payload.
     * BoardId      This is the target board id.  If this is a board to board transfer
     *              the board id is ignored.
     * TransportSize  This is size of the payload in bytes (includes
     *              ARIO_TRANSPORT_PAYLOAD_HDR).
     * CheckSum     is the checksum of the payload + ARIO_TRANSPORT_PAYLOAD_HDR.
     */
    typedef struct
    {
        UINT32      MagicCode;
        UINT16      TargetApplicationId;
        UINT8       BoardId;
        UINT8       Flags;
        UINT32      TransportSize;
        UINT32      CheckSum;
        UINT32      Reserved;
    } ARIO_TRANSPORT_PAYLOAD_HDR;

    /*
     * Copied from the Ario example code:
     *
     * The SCM fw image is sent to the SCM in segments.  The image is broken into
     * segments and they are reassembled in the SCM. Each segment is prefixed with
     * a fw_download_payload_hdr. The hdr contains all necessary information to
     * reassemble the image even if the segments are received out of order at the
     * SCM.  This is not a requirement.
     *
     * SegmentOffset    the byte offset of the segment relative to the beginning
     *                  of the image.
     * SegmentSize      the size of the seg in bytes (includes the
     *                  fw_download_payload_hdr).
     * SegmentId        is a running counter
     * SegmentTotal     is the total number of segments in this download
     * HostId           is the Initiator Id
     * FwDownloadContext is a unique code such as time stamp at the start of the
     *                  download operation.  This together with HostId will ensure
     *                  that only one initiator is downloading.  An error status
     *                  will be returned to a new download operation if a download
     *                  operation is already in progress.
     * Flags            flags for future use.
     */
    typedef struct
    {
        UINT32      FwDownloadContext;
        UINT16      HostId;
        UINT16      Flags;
        UINT32      SegmentId;
        UINT32      SegmentTotal;
        UINT32      SegmentSize;
        UINT32      SegmentOffset;
        UINT16      ImageId;
        UINT16      Reserved[5];

    } FW_DOWNLOAD_PAYLOAD_HDR;

    /* Copied from the Ario example code: */
    typedef struct
    {
        ARIO_TRANSPORT_PAYLOAD_HDR ArioTransportPayLoadHdr;
        FW_DOWNLOAD_PAYLOAD_HDR FwDownloadPayLoadHdr;
        UINT8       PayLoad[CHUNK_SIZE];
    } FW_DOWNLOAD_BUFFER_PAYLOAD;

    INT32       remaining;
    UINT32      offset = 0;
    UINT32      sendSize = 0;
    UINT32      timeout = RTC_GetSystemSeconds() + (10 * 60);   /* 10 min total */
    RC_STATUS   rcStat = { PI_ERROR, 0 };
    SCSI_CMD_STATUS scsiStat;
    char       *pInBuf;
    ARIO_TRANSPORT_PAYLOAD_HDR *pArio;
    LOG_DRIVE_BAY_FW_UPDATE_PKT fwUpdate;
    LOG_FIRMWARE_UPDATE_PKT logMsg;
    UINT32      count;

    memset(&fwUpdate, 0, sizeof(fwUpdate));
    memset(&logMsg, 0, sizeof(logMsg));

    /* Print an entry message */
    dprintf(DPRINTF_CODE_UPDATE, "%s: Enter -- WWN %08X%08X  LUN %u\n",
        __func__, bswap_32((UINT32)wwn), bswap_32((UINT32)(wwn >> 32)), lun);

    /* Compute initial data */
    pInBuf = (char *)&fwPtr[1];
    remaining = fwPtr->length - sizeof(*fwPtr);

    /* Loop while sending data. Approx. 1-3 sec */
    dprintf(DPRINTF_DEFAULT, "UpdateAdaptecSataBaySingle: sending fw to bay...\n");
    while (remaining > 0)
    {
        /* Check input data for proper format */
        pArio = &(((FW_DOWNLOAD_BUFFER_PAYLOAD *)pInBuf)->ArioTransportPayLoadHdr);

        if (ntohl(pArio->MagicCode) != ARIO_MAGIC_CODE)
        {
            dprintf(DPRINTF_CODE_UPDATE, "UpdateAdaptecSataBaySingle: bad magic number. Expected %08X, Found %08X\n",
                    ARIO_MAGIC_CODE, ntohl(pArio->MagicCode));
            break;
        }

        /* Extract the send size from the data */
        sendSize = ntohl(pArio->TransportSize);

        /* Decrement the remaining count */
        remaining -= sendSize;

        /* Send the Write buffer command. */
        if (remaining > 0)
        {
            scsiStat = SCSIWriteBuffer(pInBuf, sendSize, wwn, lun, 7,   /* mode */
                                       5,       /* retries */
                                       offset, MRP_PROC_CODEBURN_TIMEOUT);
        }

        /*
         * Final packet. This is handled differently, as we expect errors on
         * this command so we don't do automatic retries.
         */
        else
        {
            scsiStat = SCSIWriteBuffer(pInBuf, sendSize, wwn, lun, 7,   /* mode */
                                       0,       /* retries */
                                       offset, MRP_PROC_CODEBURN_TIMEOUT);

            /* Go wait for the burn/reboot */
            break;
        }

        if (scsiStat.rc != PI_GOOD)
        {
            dprintf(DPRINTF_DEFAULT, "UpdateAdaptecSataBaySingle: Write failed!\n");
            break;
        }

        /* Bump the offset */
        offset += sendSize;
        pInBuf += sendSize;
    }

    /*
     * At this point we expect a failure, with a returned sense of 02/38/06.
     * Keep retrying the Write Buffer for 30 seconds or until we see at least
     * one failure of this type.
     */
    count = 15;                 /* retries (15 * 2 sec = 30 sec) */
    while (count--)
    {
        if (scsiStat.rc == PI_ERROR &&
            scsiStat.sense == 0x02 && scsiStat.asc == 0x38 && scsiStat.ascq == 0x06)
        {
            /* Got one! */
            count = 99;         /* Make sure its non-zero on exit */
            break;
        }
        else
        {
            TaskSleepMS(2 * 1000);      /* 2 sec between retries */
            scsiStat = SCSIWriteBuffer(pInBuf, sendSize, wwn, lun, 7,   /* mode */
                                       0,       /* retries */
                                       offset, MRP_PROC_CODEBURN_TIMEOUT);
        }
    }

    if (!count)
    {
        dprintf(DPRINTF_DEFAULT, "UpdateAdaptecSataBaySingle: Never received an 02/38/06. This probably means trouble but we will wait a while longer...\n");
    }

    /*
     * After that, while the bay is restarting, we get an MRP error
     * with a variety of reasons:
     *
     * DENONXDEV   -- nonx-device       from MR_Defs.h
     * EC_NONX_DEV -- nonx-device       from ecodes.h (not in shared)
     * EC_LGOFF    -- port logged off   from ecodes.h (not in shared)
     * EC_CHECK    -- SCSI chk cond     from ecodes.h (not in shared)
     *
     * Also, the MRP may timeout.
     *
     * At this point, we really don't care what MRP error we got.  When the
     * bay starts talking again, we continue.  We will report a SCSI check
     * condition though. To tell if we get a SCSI check, look at the sense
     * data.
     *
     * We drop out of this loop after we get a successful RxDiag thru or we
     * timeout.
     *
     * Approx. 3.5 - 5 min
     */
    dprintf(DPRINTF_DEFAULT, "UpdateAdaptecSataBaySingle: bay is burning flash and resetting...\n");
    while ((scsiStat.rc != PI_GOOD) && (RTC_GetSystemSeconds() <= timeout))
    {
        if (scsiStat.sense == 0x0B && scsiStat.asc == 0x73 &&
            (scsiStat.ascq == 0x04 || /* pgm mem area update failure */
             scsiStat.ascq == 0x05))  /* pgm mem area is full */
        {
            /*
             * We have seen a 0B/73/04 in the lab returned from the bay. I
             * added the 0B/73/05 since it looks like a logical thing they
             * would return too.  Error out at this point since the bay has
             * failed the update.  There could be other SCSI errors that
             * indicate an error too, not sure at this point.
             */
            dprintf(DPRINTF_DEFAULT, "UpdateAdaptecSataBaySingle: bay signaled an update failure.\n");
            break;
        }
        else
        {
            TaskSleepMS(5 * 1000);      /* 5 sec between retries */
            scsiStat = SCSIReceiveDiag(wwn, lun);
        }
    }

    /* Did we timeout? */
    rcStat.rc = scsiStat.rc;
    rcStat.status = scsiStat.status;
    if ((scsiStat.rc != PI_GOOD) && (RTC_GetSystemSeconds() > timeout))
    {
        dprintf(DPRINTF_DEFAULT, "UpdateAdaptecSataBaySingle: timed out waiting for bay to come up.\n");
    }

    /* Once the bay is up, it will start talking again, then we are done. */
    else
    {
        dprintf(DPRINTF_DEFAULT, "UpdateAdaptecSataBaySingle: bay is up.\n");
    }

    /* Log a PASS/FAIL message */
    if (rcStat.rc == PI_GOOD)
    {
        memcpy(&fwUpdate.version, &fwPtr->revision, sizeof(fwPtr->revision));
        memcpy(&fwUpdate.sequence, &fwPtr->revCount, sizeof(fwPtr->revCount));
        memcpy(&fwUpdate.buildId, &fwPtr->buildID, sizeof(fwPtr->buildID));
        fwUpdate.targetId = fwPtr->target;
        memcpy(&fwUpdate.timeStamp, &fwPtr->timeStamp, sizeof(fwUpdate.timeStamp));
        fwUpdate.crc = fwPtr->checksum;
        memcpy(&fwUpdate.wwn, &wwn, sizeof(UINT64));
        SendAsyncEvent(LOG_DRIVE_BAY_FW_UPDATE, sizeof(fwUpdate), &fwUpdate);
    }
    else
    {
        memcpy(&logMsg.version, &fwPtr->revision, sizeof(fwPtr->revision));
        memcpy(&logMsg.sequence, &fwPtr->revCount, sizeof(fwPtr->revCount));
        memcpy(&logMsg.buildId, &fwPtr->buildID, sizeof(fwPtr->buildID));
        logMsg.targetId = fwPtr->target;
        memcpy(&logMsg.timeStamp, &fwPtr->timeStamp, sizeof(logMsg.timeStamp));
        logMsg.seqNum = 0;
        logMsg.totNum = 0;
        logMsg.crc = fwPtr->checksum;
        logMsg.reason = (char)rcStat.rc;
        SendAsyncEvent(LOG_FW_UPDATE_FAILED, sizeof(logMsg), &logMsg);
    }

    return rcStat;
}


/*----------------------------------------------------------------------------
**                  Private SCSI Related Functions
**--------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
**  Function Name: SCSIWriteBuffer
**
**  Comments:  Send the code down to a Drive or Drive Bay for it to burn its
**             own flash.
**
**  Parameters: pBuf - pointer to data
**              len  - length
**              wwn  - device World Wide Name
**              lun  - device Lun
**              mode - write buffer mode (probably only 5 or 7)
**              retry   - number of retries
**              offset  - load offset for mode 7
**              timeout - time to wait for command completion
**
**  Returns:    rc = 0  => GOOD
**              rc != 0 => ERROR; also returns sense/asc/ascq
**
**--------------------------------------------------------------------------*/
SCSI_CMD_STATUS SCSIWriteBuffer(char *pBuf, UINT32 len,
                                UINT64 wwn, UINT32 lun,
                                UINT8 mode, UINT8 retry, UINT32 offset, UINT32 timeout)
{
    MRSCSIIO_REQ *inPkt = NULL;
    MRSCSIIO_RSP *outPkt = NULL;
    INT32       rc = PI_GOOD;
    SCSI_CMD_STATUS scsiStat;

    memset(&scsiStat, 0, sizeof(scsiStat));

    /* Allocate the input & output packets */
    inPkt = MallocSharedWC(sizeof(*inPkt));
    outPkt = MallocSharedWC(sizeof(*outPkt));

    /* Setup input structure from the input parms. */
    inPkt->idchc = MRSCSIIO_USE_WWN;
    memcpy(&inPkt->wwn, &wwn, sizeof(UINT64));

    inPkt->cmdlen = 10;
    inPkt->func = MRSCSIIO_OUTPUT;
    inPkt->strat = MRSCSIIO_NORM;
    inPkt->timeout = 90;        /* 90 sec timeout */
    inPkt->flags = 0;
    inPkt->lun = lun;
    inPkt->retry = retry;
    inPkt->bptr = pBuf;
    inPkt->blen = len;
    inPkt->cdb[0] = 0x3B;       /* Write Buffer                         */
    inPkt->cdb[1] = mode;       /* Mode 5 = Download Microcode & Save   */
    /* Mode 7 = Download Microcode w/offset */
    inPkt->cdb[2] = 0x00;       /* Buffer ID                            */
    inPkt->cdb[3] = (offset >> 16) & 0xff;      /* Buffer Offset (MSB)        */
    inPkt->cdb[4] = (offset >> 8) & 0xff;       /* Buffer Offset              */
    inPkt->cdb[5] = offset & 0xff;      /* Buffer Offset (LSB)        */
    inPkt->cdb[6] = (len >> 16) & 0xff; /* Buffer Length (MSB)        */
    inPkt->cdb[7] = (len >> 8) & 0xff;  /* Buffer Length              */
    inPkt->cdb[8] = len & 0xff; /* Buffer Length (LSB)        */
    inPkt->cdb[9] = 0x00;       /* Control                    */

    dprintf(DPRINTF_SCSICMD, "%s: Issuing write buffer cmd to WWN %08X%08X  "
            "LUN %u\n", __func__,
            bswap_32((UINT32)wwn), bswap_32((UINT32)(wwn >> 32)), lun);

    dprintf(DPRINTF_SCSICMD, "SCSIWriteBuffer: CDBLen = %u, DataLen = %u\n",
            inPkt->cmdlen, inPkt->blen);

    dprintf(DPRINTF_SCSICMD, "SCSIWriteBuffer: CDB = "
            "%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX "
            "%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX\n",
            inPkt->cdb[0], inPkt->cdb[1], inPkt->cdb[2], inPkt->cdb[3],
            inPkt->cdb[4], inPkt->cdb[5], inPkt->cdb[6], inPkt->cdb[7],
            inPkt->cdb[8], inPkt->cdb[9], inPkt->cdb[10], inPkt->cdb[11],
            inPkt->cdb[12], inPkt->cdb[13], inPkt->cdb[14], inPkt->cdb[15]);

    if (inPkt->blen)
    {
        dprintf(DPRINTF_SCSICMD, "SCSIWriteBuffer: Input Data = "
                "%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX "
                "%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX ...\n",
                pBuf[0], pBuf[1], pBuf[2], pBuf[3],
                pBuf[4], pBuf[5], pBuf[6], pBuf[7],
                pBuf[8], pBuf[9], pBuf[10], pBuf[11],
                pBuf[12], pBuf[13], pBuf[14], pBuf[15]);
    }

    /*
     * Send the request to the device.  This function handles timeout
     * conditions and task switches while waiting.
     */
    rc = PI_ExecMRP(inPkt, sizeof(*inPkt), MRSCSIIO, outPkt, sizeof(*outPkt), timeout);

    if (rc != PI_GOOD)
    {
        dprintf(DPRINTF_DEFAULT, "SCSIWriteBuffer: command failed: rc %d, SenseKey 0x%02hhX, ASC 0x%02hhX ASCQ 0x%02hhX\n",
                rc, outPkt->sense, outPkt->asc, outPkt->ascq);

        scsiStat.sense = outPkt->sense;
        scsiStat.asc = outPkt->asc;
        scsiStat.ascq = outPkt->ascq;
    }

    scsiStat.rc = rc;
    scsiStat.status = outPkt->status;

    Free(inPkt);
    if (rc != PI_TIMEOUT)
    {
        Free(outPkt);
    }

    return scsiStat;
}

/*----------------------------------------------------------------------------
**  Function Name: SCSIReceiveDiag
**
**  Comments:  Read Diag Page 0, basically polling for completion of the
**             Adaptec code update.
**
**  Returns:    rc = 0  => GOOD
**              rc != 0 => ERROR; also returns sense/asc/ascq
**
**--------------------------------------------------------------------------*/
SCSI_CMD_STATUS SCSIReceiveDiag(UINT64 wwn, UINT32 lun)
{
    MRSCSIIO_REQ *inPkt = NULL;
    MRSCSIIO_RSP *outPkt = NULL;
    MRSCSIIO_RSP *retPkt = NULL;
    UINT16      rc = PI_GOOD;
    SCSI_CMD_STATUS scsiStat;

    memset(&scsiStat, 0, sizeof(scsiStat));

    inPkt = MallocSharedWC(sizeof(*inPkt));
    outPkt = MallocSharedWC(sizeof(*outPkt));

    /*
     * We need to allocate a minimal data buffer or things don't
     * work real well.
     */
    retPkt = MallocSharedWC(0x10);      /* minimal amount */

    /* Set input parm from the input. */
    inPkt->idchc = MRSCSIIO_USE_WWN;
    inPkt->wwn = wwn;
    inPkt->cmdlen = 6;
    inPkt->func = MRSCSIIO_INPUT;
    inPkt->strat = MRSCSIIO_NORM;
    inPkt->timeout = 30;
    inPkt->flags = 1;
    inPkt->retry = 0;
    inPkt->lun = lun;
    inPkt->blen = 0x10;
    inPkt->bptr = retPkt;
    inPkt->cdb[0] = 0x1C;       /* Receive diagnostic results   */
    inPkt->cdb[1] = 0x00;       /* Page code valid              */
    inPkt->cdb[2] = 0x00;       /* Page code                    */
    inPkt->cdb[3] = 0x00;       /* Data length                  */
    inPkt->cdb[4] = 0x10;       /* Data length                  */
    inPkt->cdb[5] = 0x00;       /* Control                      */

    dprintf(DPRINTF_SCSICMD, "%s: Issuing Read Diag cmd to WWN %08X%08X  "
            "LUN %u\n", __func__,
            bswap_32((UINT32)wwn), bswap_32((UINT32)(wwn >> 32)), lun);

    dprintf(DPRINTF_SCSICMD, "SCSIReceiveDiag: CDB = %02hhX%02hhX%02hhX%02hhX%02hhX%02hhX\n",
            inPkt->cdb[0], inPkt->cdb[1], inPkt->cdb[2], inPkt->cdb[3], inPkt->cdb[4],
            inPkt->cdb[5]);

    /*
     * Send the request to Thunderbolt.  This function handles timeout
     * conditions and task switches while waiting.
     */
    rc = PI_ExecMRP(inPkt, sizeof(*inPkt), MRSCSIIO,
                    outPkt, sizeof(*outPkt), MRP_STD_TIMEOUT);

    scsiStat.sense = outPkt->sense;
    scsiStat.asc = outPkt->asc;
    scsiStat.ascq = outPkt->ascq;

    if (rc == PI_ERROR)
    {
        dprintf(DPRINTF_DEFAULT, "SCSIReceiveDiag: MRP failed: rc: %X/%X Sense: %02hhX/%02hhX/%02hhX\n",
                rc, outPkt->status, outPkt->sense, outPkt->asc, outPkt->ascq);
    }

    if (rc == PI_GOOD)
    {
        dprintf(DPRINTF_DEFAULT, "SCSIReceiveDiag: Succeeded: rc: %X/%X Sense: %02hhX/%02hhX/%02hhX\n",
                rc, outPkt->status, outPkt->sense, outPkt->asc, outPkt->ascq);
    }

    scsiStat.rc = rc;
    scsiStat.status = outPkt->status;

    DelayedFree(MRSCSIIO, retPkt);
    Free(inPkt);
    if (rc != PI_TIMEOUT)
    {
        Free(outPkt);
    }

    return scsiStat;
}

#endif /* LOG_SIMULATOR */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:ts=4 sw=4 expandtab
**/
