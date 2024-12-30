/* $Id: xk_raidmon.c 156532 2011-06-24 21:09:44Z m4 $ */
/**
******************************************************************************
**
**  @file       xk_raidmon.c
**
**  @brief      Module to monitor raid status of local controller scsi disks
**
**  Copyright (c) 2004-2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/

/* Xiotech includes */
#include "xk_raidmon.h"

#include "CmdLayers.h"
#include "debug_files.h"
#include "ipc_common.h"
#include "ipc_sendpacket.h"
#include "logging.h"
#include "md5.h"
#include "PacketInterface.h"
#include "PortServerUtils.h"
#include "quorum.h"
#include "quorum_utils.h"
#include "xk_mapmemfile.h"
#include "XIO_Const.h"
#include "XIO_Std.h"
#include "XIO_Macros.h"
#include "PI_CmdHandlers.h"

/* Linux includes */
#include "asm/bitops.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

/*
******************************************************************************
** Private defines
******************************************************************************
*/

#define XK_RAIDMON_CMD_BUFFER_SIZE          256
#define XK_LOCAL_RAID_FLUX_TMO              120

/*
******************************************************************************
** Private variables
******************************************************************************
*/
static char xk_raidmonCmdBuffer[XK_RAIDMON_CMD_BUFFER_SIZE];

/* Raid devices by slots */
static XK_RAIDMON_INFO *localRaids = NULL;

/*
** Mapped to a file
** 1 = yes.
** 0 = no.
*/
static UINT8 xkRaidmonMapped = 0;

/*
** Do we have a minimum amount of devices.
** 1 = yes.
** 0 = no.
*/
static UINT8 xkRaidmonHaveMinScsis = 1;

/*
** Force check even if md5's match.
** 1 = yes.
** 0 = no.
*/
static UINT8 xkRaidmonForceCheck = 0;

/*
******************************************************************************
** Public variables - externed in the header file
******************************************************************************
*/
UINT8       gLocalRaidFlux = 0;

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
static INT32 XK_RaidMonitorRetrieveRaidInfo(void);
static UINT8 XK_RaidMonitorMdstatMD5Change(void);
static INT32 RaidMonitorCheckStatus(void);
static INT32 XK_RaidMonitorSetDriveStatus(XK_RAIDMON_DEVICE *pDisk,
                                          UINT8 diskNum, UINT8 status);
static INT32 XK_RaidMonitorRetrieveDevices(void);
static XK_RAIDMON_DEVICE *XK_RaidMonitorRetrieveDeviceByName(char *);
static INT32 XK_RaidMonitorExecCommand(void);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      Task to monitor/report the local SCSI raid subsystem.
**
**              This task will monitor the status of the local raid subsystem
**              and log anything that may be of interest.
**
**  @param      tParams    - Unused, (NULL is fine).
**
**  @return     never
**
******************************************************************************
**/
NORETURN void XK_RaidMonitorTask(UNUSED TASK_PARMS *tParams)
{
    UINT8       rIndex = 0;

    /*
     * Initialize Raid Data from Mem mapped File.
     * If it fails, try and malloc the data.
     * Else, set the xkRaidmonMapped flag to true (1).
     */
    dprintf(DPRINTF_DEFAULT, "LOCAL RAIDMON - Initializing RaidData...\n");
    if (!(localRaids = (XK_RAIDMON_INFO *)MEM_InitMapFile("/opt/xiotech/ccbdata/RAIDMON.mmf",
                                             sizeof(*localRaids), 0x00, NULL)))
    {
        LogMessage(LOG_TYPE_DEBUG, "XK_RaidMonitorTask - Error opening mapped file, revert to Malloc");
        localRaids = MallocWC(sizeof(*localRaids));
    }
    else
    {
        xkRaidmonMapped = 1;
    }

    /*
     * Check the magic number to see if something has changed.
     * If it has, zero out all of the data.
     */
    if (localRaids->magic != XK_RAIDMON_MAGIC)
    {
        /*
         * Clear out the raid structures.
         */
        bzero(localRaids, sizeof(*localRaids));
        localRaids->magic = XK_RAIDMON_MAGIC;
    }

    /*
     * Force a check when we start up.
     */
    xkRaidmonForceCheck = 1;

    /*
     * Refresh data on disks.
     */
    XK_RaidMonitorRetrieveDevices();

    /*
     * Clear out fail, resync data.
     */
    for (rIndex = 0; rIndex < XK_RAIDMON_MAX_DEVS; ++rIndex)
    {
        localRaids->disks[rIndex].failDevs = 0;
        localRaids->disks[rIndex].resyncDevs = 0;
    }

    /*
     * If mapped, Flush Now.
     */
    if (xkRaidmonMapped)
    {
        MEM_FlushMapFile(localRaids, sizeof(*localRaids));
    }

    /*
     * Main control loop.
     */
    while (1)
    {
        /*
         * Sleep for 10 seconds and then check status.
         */
        TaskSleepMS(10000);
        dprintf(DPRINTF_RAIDMON, "LOCAL RAIDMON - Local Raid Monitor running\n");

        /*
         * Get the data.
         */
        XK_RaidMonitorRetrieveRaidInfo();


        /*
         * If RaidFlux, decrement it.
         */
        if (gLocalRaidFlux)
        {
            gLocalRaidFlux -= 10;
        }

        /*
         * Hey, let's do it again.
         */
    }

    /*
     * Should never get here.
     */
}

/**
******************************************************************************
**
**  @brief      Checks whether a local disk is resyncing or not
**
**              Checks whether a local disk is resyncing or not
**
**  @param      none
**
**  @return     TRUE if resyncing, FALSE if not.
**
******************************************************************************
**/
UINT8 XK_RaidMonitorIsResyncing(void)
{
    UINT8       rdIndex;

    if (!localRaids)
    {
        return FALSE;
    }

    for (rdIndex = 0; rdIndex < XK_RAIDMON_MAX_DEVS; ++rdIndex)
    {
        if (localRaids->disks[rdIndex].resyncDevs)
        {
            return TRUE;
        }
    }

    return FALSE;
}

/*----------------------------------------------------------------------------
**
**  @brief      Checks whether a DSC controller disk is resyncing or not
**
**              Checks whether a DSC controller disk is resyncing or not
**
**  @param      none
**
**  @return     return 0 if none,
**              else returns controller slot bitmap of resyncing controllers.
**
**--------------------------------------------------------------------------*/
UINT32 XK_RaidMonitorResyncingControllers(void)
{
    XIO_PACKET  reqPacket;
    XIO_PACKET  rspPacket;
    INT32       rc = GOOD;
    UINT8       cont = 0;
    UINT8       rdIndex = 0;
    XK_RAIDMON_INFO *rdData = NULL;
    unsigned long resyncDevs = 0;
    QM_FAILURE_DATA *qmFailureData;

    /*
     * Allocate memory for the request (header and data) and the
     * response header.  Memory for the response data is allocated
     * in TunnelRequest().
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    reqPacket.pPacket = NULL;
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    /*
     * Fill in the request header
     */
    reqPacket.pHeader->commandCode = PI_MISC_LOCAL_RAID_INFO_CMD;
    reqPacket.pHeader->length = 0;

    qmFailureData = MallocSharedWC(sizeof(*qmFailureData));
    for (cont = 0; cont < MAX_CONTROLLERS; ++cont)
    {
        /*
         * If the serial number is 0 (unused) we can skip the rest
         * of this loop and start over with the next record.
         */
        if (CCM_ControllerSN(cont) == 0)
        {
            continue;
        }

        /*
         * Set the response data to NULL,
         * and clear out the response header.
         */
        rspPacket.pPacket = NULL;
        bzero(rspPacket.pHeader, sizeof(*rspPacket.pHeader));

        /*
         * If we are asking ourselves, do not tunnel,
         * Else tunnel the packet to the controller.
         */
        if (CCM_ControllerSN(cont) == GetMyControllerSN())
        {
            /*
             * Issue the command through the top-level command handler.
             */
            rc = PortServerCommandHandler(&reqPacket, &rspPacket);
        }
        else
        {
            UINT8   retries = 2;        /* Ethernet, Fiber(1), Disk Quorum(2) */

            do
            {
                if (rc != PI_TIMEOUT)
                {
                    Free(rspPacket.pPacket);
                }
                else
                {
                    rspPacket.pPacket = NULL;
                }
                rc = TunnelRequest(CCM_ControllerSN(cont), &reqPacket, &rspPacket);
            } while (rc != GOOD && (retries--) > 0);
        }

        /*
         * Check for a successful command.
         */
        if (rc == PI_GOOD)
        {
            /*
             * Look for resyncing drives.
             */
            rdData = (XK_RAIDMON_INFO *)rspPacket.pPacket;

            /*
             * Look at the data for resyncing drives.
             */
            for (rdIndex = 0; rdIndex < XK_RAIDMON_MAX_DEVS; ++rdIndex)
            {
                if (rdData->disks[rdIndex].resyncDevs != 0)
                {
                    dprintf(DPRINTF_DEFAULT, "XK_RaidMonitorResyncingControllers - CN%d: currently resyncing.\n",
                            cont);

                    set_bit(cont, &resyncDevs);
                    break;
                }
            }
        }
        /*
         * If the command failed, and it was due to an invalid command code,
         * assume this is a Bigfoot controller and carry on.
         */
        else if (rspPacket.pHeader->status == PI_INVALID_CMD_CODE)
        {
            dprintf(DPRINTF_DEFAULT, "XK_RaidMonitorResyncingControllers - Skipping CN%d: 1000 Controller\n",
                    cont);
        }
        /*
         * These are OK.
         */
        else if (ReadFailureData(CCM_ControllerSN(cont), qmFailureData) == GOOD)
        {
            switch (qmFailureData->state)
            {
                    /*
                     * These states are ok, if we cannot talk to the controller.
                     */
                case FD_STATE_UNUSED:
                case FD_STATE_FAILED:
                case FD_STATE_FIRMWARE_UPDATE_INACTIVE:
                case FD_STATE_INACTIVATED:
                case FD_STATE_DISASTER_INACTIVE:
                case FD_STATE_VCG_SHUTDOWN:

                    dprintf(DPRINTF_DEFAULT, "XK_RaidMonitorResyncingControllers - Skipping CN%d: Failure State %d\n",
                            cont, qmFailureData->state);

                    break;

                    /*
                     * These are not.
                     */
                case FD_STATE_OPERATIONAL:
                case FD_STATE_POR:
                case FD_STATE_ADD_CONTROLLER_TO_VCG:
                case FD_STATE_STRANDED_CACHE_DATA:
                case FD_STATE_FIRMWARE_UPDATE_ACTIVE:
                case FD_STATE_UNFAIL_CONTROLLER:
                case FD_STATE_ACTIVATE:
                default:

                    dprintf(DPRINTF_DEFAULT, "XK_RaidMonitorResyncingControllers - Marking CN%d: Resyncing,  Failure State %d\n",
                            cont, qmFailureData->state);

                    set_bit(cont, &resyncDevs);

                    break;
            }
        }
        else
        {
            dprintf(DPRINTF_DEFAULT, "XK_RaidMonitorResyncingControllers - Error retrieving data for CN%d: Marking as Resyncing\n", cont);

            set_bit(cont, &resyncDevs);
        }

        /*
         * Clear out the memory, if it was allocated
         * and no timeout occured.
         */
        if ((rc != PI_TIMEOUT) && (rspPacket.pPacket != NULL))
        {
            Free(rspPacket.pPacket);
        }
    }
    Free(qmFailureData);

    /*
     * Free the allocated memory
     */
    Free(reqPacket.pHeader);
    Free(rspPacket.pHeader);

    /*
     * return the bitmask of resyncing controllers
     */
    return (UINT32)resyncDevs;
}

/*----------------------------------------------------------------------------
** Function:    PI_MiscGetLocalRaidInfo
**
** Description: Retrieve Raid Information.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_MiscGetLocalRaidInfo(UNUSED XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    /*
     * Allocate the space, fill in the data, and return to caller.
     */
    pRspPacket->pPacket = MallocWC(sizeof(*localRaids));
    if (localRaids)
    {
        memcpy(pRspPacket->pPacket, localRaids, sizeof(*localRaids));
    }
    else
    {
        memset(pRspPacket->pPacket, 0, sizeof(*localRaids));
    }
    pRspPacket->pHeader->length = sizeof(*localRaids);
    pRspPacket->pHeader->status = PI_GOOD;
    pRspPacket->pHeader->errorCode = PI_GOOD;

    return PI_GOOD;
}

/*****************************************************************************
** Private Functions.
*****************************************************************************/

/**
******************************************************************************
**
**  @brief      Check for a change in the mdstat file.
**
**              Look at the MD5 sum to determine whether
**              or not the raid status has changed.
**
**  @param      None.
**
**  @return     TRUE/FALSE.
**
******************************************************************************
**/
static INT32 XK_RaidMonitorRetrieveRaidInfo(void)
{
    INT32       rc = GOOD;
    UINT8       rIndex = 0;
    UINT8       onlineCount = 0;
    UINT8       onlineIndex = 0;

    /*
     * Check the MD5 status of mdstat to see if data has changed.
     * If it has, gather the data and analyze it.
     */
    if (xkRaidmonForceCheck || (XK_RaidMonitorMdstatMD5Change() == TRUE))
    {
        /*
         * Reset the forced flag.
         */
        xkRaidmonForceCheck = 0;

        /*
         * Check the status.
         */
        rc = RaidMonitorCheckStatus();

        /*
         * Check for consistency.
         */
        for (rIndex = 0; rIndex < XK_RAIDMON_MAX_DEVS; ++rIndex)
        {
            /*
             * Is this a disk we think is present?
             */
            if ((localRaids->disks[rIndex].present == XK_RAIDMON_DEVICE_PRESENT) &&
                (localRaids->disks[rIndex].type == XK_RAIDMON_TYPE_DISK))
            {
                /*
                 * Is it really present.
                 */
                if ((1 << rIndex) & localRaids->actDevices)
                {
                    /*
                     * If it is online/resyncing bump the count.
                     */
                    if ((localRaids->disks[rIndex].status == XK_RAIDMON_STATUS_ONLINE) ||
                        (localRaids->disks[rIndex].status == XK_RAIDMON_STATUS_RESYNCING))
                    {
                        ++onlineCount;
                        onlineIndex = rIndex;
                    }
                }

                /*
                 * Else, we thought this was present, but it is now missing,
                 * send a log message, and clear out the structure.
                 */
                else
                {
                    LogMessage(LOG_TYPE_WARNING, "Local SCSI, Disk (slot %d) Missing",
                               rIndex);
                    bzero(&localRaids->disks[rIndex], sizeof(XK_RAIDMON_DEVICE));
                }
            }

            /*
             * Else we do not think this is present, clear out the structure.
             */
            else if (localRaids->disks[rIndex].type != XK_RAIDMON_TYPE_BACKPLANE)
            {
                bzero(&localRaids->disks[rIndex], sizeof(XK_RAIDMON_DEVICE));
            }
        }

        /*
         * See if we have enough devices.
         */
        if (onlineCount < XK_RAIDMON_MIN_DRIVE_THRESHOLD)
        {
            /*
             * If we had enough disks, log this.
             */
            if (xkRaidmonHaveMinScsis)
            {
                xkRaidmonHaveMinScsis = 0;

                LogMessage(LOG_TYPE_ERROR, "Local SCSI, 1 active Disk (slot %d)",
                           onlineIndex);
            }
        }
        /*
         * If we did not have the minimum amount of drives,
         * and we now do, log this information.
         */
        else if (!xkRaidmonHaveMinScsis)
        {
            LogMessage(LOG_TYPE_INFO, "Local SCSI %d active Disks", onlineCount);
            xkRaidmonHaveMinScsis = 1;
        }

        /*
         * If mapped, Flush Now.
         */
        if (xkRaidmonMapped)
        {
            MEM_FlushMapFile(localRaids, sizeof(*localRaids));
        }
    }

    return rc;
}

/**
******************************************************************************
**
**  @brief      Check for a change in the mdstat file.
**
**              Look at the MD5 sum to determine whether
**              or not the raid status has changed.
**
**  @param      None.
**
**  @return     TRUE/FALSE.
**
******************************************************************************
**/
static UINT8 XK_RaidMonitorMdstatMD5Change(void)
{
    UINT8       rc = TRUE;
    UINT32      sizeRead = 0;
    UINT32      totalRead = 0;
    FILE       *pFH;
    static char pBuffer[SIZE_16K];
    MD5_CONTEXT md5;

    /* Open the mdstat file and check the status. */

    pFH = fopen("/proc/mdstat", "r");

    memset(pBuffer, 0, sizeof(pBuffer));

    /* If we were able to open the file, proceed. */

    if (!pFH)
    {
        dprintf(DPRINTF_DEFAULT, "LOCAL RAIDMON - Could not open /proc/mdstat\n");
        return rc;
    }

    /* Read in the entire file */

    sizeRead = fread(pBuffer, 1, SIZE_16K, pFH);
    while (sizeRead)
    {
        totalRead += sizeRead;
        sizeRead = fread((pBuffer + totalRead), 1, SIZE_16K - totalRead, pFH);
    }

    /* MD5 the data, and look for change. */

    if (!totalRead)
    {
        dprintf(DPRINTF_DEFAULT, "LOCAL RAIDMON - mdstat, NO DATA!\n");
        goto out;
    }

    /* Do the MD5 dirty work. */

    MD5Init(&md5);
    MD5Update(&md5, (UINT8 *)pBuffer, totalRead);
    MD5Final(&md5);

    /*
     * If the value is equal to what we have,
     * nothing has changed since we last checked.
     */
    if (memcmp(localRaids->dataMD5, md5.digest, IPC_DIGEST_LENGTH) == 0)
    {
        dprintf(DPRINTF_RAIDMON, "LOCAL RAIDMON - mdstat, no MD5 change\n");
        rc = FALSE;
    }
    /* Else, data has changed, must be a change in the status. */
    else
    {
        dprintf(DPRINTF_RAIDMON, "LOCAL RAIDMON - mdstat, MD5 change\n");
        memcpy(localRaids->dataMD5, md5.digest, IPC_DIGEST_LENGTH);
    }

  out:
    /* Close the file. */

    Fclose(pFH);

    return rc;
}

/**
******************************************************************************
**
**  @brief      Set the drive status for the slot.
**
**              This will set the local status and if the drive is failed,
**              will turn on the led and alarm.  If OK, will turn off the led
**              and alarm.
**
**  @param      slot    - slot to set status on.
**              status  - status to change drive to.
**
**  @return     GOOD on Success,  ERROR on Failure.
**
******************************************************************************
**/
static INT32 RaidMonitorCheckStatus(void)
{
    INT32       rc = GOOD;
    UINT8       diskResync = 0;
    UINT8       pctCmp = 0;
    UINT8       dskNum = 0;
    char       *pChar = NULL;
    char       *pCharFail = NULL;
    char       *curIndex = NULL;
    FILE       *pFH = NULL;
    char        pTmp[16] = { 0 };
    char        pBuffer[SIZE_1K] = { 0 };
    XK_RAIDMON_DEVICE *pDisk = NULL;

    /* Open the mdstat file and check the status. */

    pFH = fopen("/proc/mdstat", "r");

    if (!pFH)
    {
        /* Force a check the next time around. */

        xkRaidmonForceCheck = 1;

        dprintf(DPRINTF_DEFAULT, "LOCAL RAIDMON - RaidMonitorCheckStatus - Could not open /proc/mdstat\n");
        return ERROR;
    }

    /* If we were able to open the file, proceed. */

    /*
     * Read in the entire file.  256 bytes per line should do it. :-)
     * 3 lines per device
     *
     * Normal:
     *  md3 : active raid1 sdb5[1] sda5[0]
     *  7341568 blocks [6/2] [UU____]
     *
     *  md0 : active raid1 sdb1[1] sda1[0]
     *  2104384 blocks [6/2] [UU____]
     *
     *
     * Failed:
     *  md3 : active raid1 sdb5[6](F) sda5[0]
     *  7341568 blocks [6/1] [U_____]
     *
     *  md0 : active raid1 sdb1[6](F) sda1[0]
     *  2104384 blocks [6/1] [U_____]
     *
     *
     * Resyncing:
     *  md3 : active raid1 sdb5[6] sda5[0]
     *  7341568 blocks [6/1] [U_____]
     *  resync=DELAYED
     *  md0 : active raid1 sdb1[6] sda1[0]
     *  2104384 blocks [6/1] [U_____]
     *  [==>..................]  recovery = 10.9% (230912/2104384) finish=0.6min speed=46182K/sec
     */

    /* Clear out the actvDevices. */

    localRaids->actDevices = 0;

    /* Loop on the data */

    while (fgets(pBuffer, 256, pFH))
    {
        diskResync = 0;
        pctCmp = 0;

        /*
         * The first two characters should be 'm','d' (see above)
         * If the are not, continue and get the next line.
         */
        if ((pBuffer[0] != 'm') || (pBuffer[1] != 'd'))
        {
            continue;
        }

        dprintf(DPRINTF_RAIDMON, "LOCAL RAIDMON - pBuffer: %s", pBuffer);

        /*
         * Look for an indication that
         * a resync may be taking place.
         */
        /*
         * Skip through the next line, and get line 3 of the record.
         * We will use this to check the resync status.
         */
        pChar = NULL;
        curIndex = pBuffer + 256;
        if (fgets(curIndex, 256, pFH))
        {
            pChar = fgets(curIndex, 256, pFH);
        }

        if (!pChar)
        {
            dprintf(DPRINTF_RAIDMON, "LOCAL RAIDMON - %s No Line 3\n", __func__);
            continue;
        }

        dprintf(DPRINTF_RAIDMON, "LOCAL RAIDMON - pChar: %s", pChar);

        /*
         * Look for the word resync, if it is there,
         * Resync is delayed for this drive.
         */
        if (strstr(pChar, "resync"))
        {
            diskResync = 1;
        }

        /*
         * Also check for a '%' sign, this indicates
         * a rebuild currently in progress.
         */
        if ((curIndex = strchr(pChar, '%')))
        {
            diskResync = 1;

            /*
             * Resync currently in progress,
             * get the percent complete.
             */
            pChar = curIndex;
            while (*pChar != ' ')
            {
                --pChar;
            }
            ++pChar;
            curIndex -= 2;
            *curIndex = '\0';
            pctCmp = atoi(pChar);
        }

        /*
         * Search for the disk names from the first line.
         * May be multiple per line.
         */
        while ((pChar = strchr(pBuffer, '[')))
        {
            /*
             * Set pDisk to NULL;
             */
            pDisk = NULL;

            /*
             * pinpoint on the disk name.
             */
            curIndex = pChar;
            while (*curIndex != ' ')
            {
                --curIndex;
            }
            ++curIndex;
            *pChar = '\0';

            /*
             * Copy the name to a tmp variable,
             * so we can manipulate it later.
             */
            strcpy(pTmp, curIndex);

            /*
             * Retrieve the information for this particular disk.
             */
            pDisk = XK_RaidMonitorRetrieveDeviceByName(pTmp);

            /*
             * Try twice to get the data.
             */
            dskNum = 2;
            while ((dskNum != 0) && (pDisk == NULL))
            {
                dprintf(DPRINTF_DEFAULT, "LOCAL RAIDMON - %s - Refreshing disk info for %s\n",
                        __func__, pTmp);
                /*
                 * If we could not retrieve the data, refresh our data.
                 */
                XK_RaidMonitorRetrieveDevices();

                /*
                 * Decrement the tries;
                 */
                --dskNum;

                /*
                 * Retrieve the information for this particular disk.
                 */
                pDisk = XK_RaidMonitorRetrieveDeviceByName(pTmp);
            }

            /*
             * If we couldn't get the data,
             * force a check at the next interval.
             */
            if (pDisk == NULL)
            {
                /*
                 * Force a check the next time around.
                 */
                xkRaidmonForceCheck = 1;

                dprintf(DPRINTF_DEFAULT, "LOCAL RAIDMON - %s - Error Refreshing disk info for %s\n",
                        __func__, pTmp);

                /*
                 * Continue.
                 */
                continue;
            }

            /*
             * Retrieve the partition number of the disk.
             */
            curIndex = pTmp;
            while ((*curIndex != '\0') && ((*curIndex < '0') || (*curIndex > '9')))
            {
                ++curIndex;
            }

            /*
             * If we got it, get the number.
             */
            if (*curIndex != '\0')
            {
                dskNum = (UINT8)atoi(curIndex);
            }

            /*
             * Else, set the flag to check next time.
             */
            else
            {
                dprintf(DPRINTF_DEFAULT, "LOCAL RAIDMON - %s - Could not get partition number for slot %d\n",
                        __func__, pDisk->slot);
                /*
                 * Force a check the next time around.
                 */
                xkRaidmonForceCheck = 1;

                /*
                 * Continue.
                 */
                continue;
            }

            /*
             * Set the bit saying we know about this drive.
             * and set its present bit to tadah PRESENT.
             */
            pDisk->present = XK_RAIDMON_DEVICE_PRESENT;
            set_bit(pDisk->slot, &localRaids->actDevices);

            /*
             * Check for a failed disk.
             */
            pCharFail = pChar + 1;
            while ((*pCharFail != '\0') && (*pCharFail != ' ') && (*pCharFail != 'F'))
            {
                ++pCharFail;
            }

            /*
             * If failed, set its status to such.
             */
            if ((*pCharFail == 'F') &&
                (*(pCharFail - 1) == '(') && (*(pCharFail + 1) == ')'))
            {
                /*
                 * Set the status.
                 */
                XK_RaidMonitorSetDriveStatus(pDisk, dskNum, XK_RAIDMON_STATUS_FAILED);
            }
            /*
             * If resyncing, do likewise.
             */
            else if (diskResync)
            {
                /*
                 * Set the % complete on the disk for partion.
                 */
                if (pctCmp != 0)
                {
                    pDisk->resyncPC = pctCmp;

                    dprintf(DPRINTF_DEFAULT, "LOCAL RAIDMON - RESYNC Partition %d Disk (slot %d), %dpct complete\n",
                            dskNum, pDisk->slot, pctCmp);
                }

                /*
                 * Set the status.
                 */
                XK_RaidMonitorSetDriveStatus(pDisk, dskNum, XK_RAIDMON_STATUS_RESYNCING);
            }
            /*
             * Set the status to OK.
             */
            else
            {
                /*
                 * Set the status.
                 */
                XK_RaidMonitorSetDriveStatus(pDisk, dskNum, XK_RAIDMON_STATUS_ONLINE);
            }

            /*
             * Up above, we overwrote pChar with a
             * EOL '\0', replace with a dash, so we
             * can proceed to parse the string.
             */
            *pChar = '-';
        }
    }

    /*
     * Close the file.
     */
    if (pFH)
    {
        Fclose(pFH);
        pFH = NULL;
    }

    return rc;
}

/**
******************************************************************************
**
**  @brief      Set the drive status for the slot.
**
**              This will set the local status and if the drive is failed,
**              will turn on the led and alarm.  If OK, will turn off the led
**              and alarm.
**
**  @param      slot    - slot to set status on.
**              status  - status to change drive to.
**
**  @return     GOOD on Success,  ERROR on Failure.
**
******************************************************************************
**/
static INT32 XK_RaidMonitorRetrieveDevices(void)
{
    INT32       rc = GOOD;
    INT16       slot = -1;
    char       *pChar = NULL;
    FILE       *pFH = NULL;
    char        pBuffer[SIZE_1K] = { 0 };
    char        rdDiskName[32] = { 0 };
    XK_RAIDMON_DEVICE *pDisk = NULL;

    /*
     * First lets send the commands to get the data we need
     */
    if (rc == GOOD)
    {
        /*
         * Get the sgsafte output
         */
        sprintf(xk_raidmonCmdBuffer, "/sbin/sgsafte >/tmp/sgsafte.out");
        rc = XK_RaidMonitorExecCommand();
    }

    /*
     * Get the devices from sgsafte.
     */
    if (rc == GOOD)
    {
        char       *curIndex;

        /*
         * Open the mdstat file and check the status.
         */
        pFH = fopen("/tmp/sgsafte.out", "r");

        /*
         * If we were able to open the file, proceed.
         */
        if (pFH)
        {
            /*
             * Read in the entire file, line by line
             */
            while (fgets(pBuffer, 256, pFH))
            {
                /*
                 * Skip the first 3 lines.
                 */
                if ((pBuffer[0] >= '0') && (pBuffer[0] <= '9'))
                {
                    slot = -1;
                    curIndex = pBuffer;

                    /*
                     * Find the slot/ID.
                     */
                    if ((curIndex = strchr(curIndex, ':')))
                    {
                        ++curIndex;
                        if ((curIndex = strchr(curIndex, ':')))
                        {
                            ++curIndex;
                            if ((pChar = strchr(curIndex, ':')))
                            {
                                *pChar = '\0';
                                slot = (UINT8)atoi(curIndex);
                                *pChar = ':';
                            }
                        }
                    }

                    /*
                     * We have our slot, fill in the data.
                     */
                    if ((slot >= 0) && (slot <= XK_RAIDMON_MAX_DEVS))
                    {
                        curIndex = pBuffer;

                        /*
                         * Set the slot.
                         */
                        localRaids->disks[slot].oldSlot = localRaids->disks[slot].slot;
                        localRaids->disks[slot].slot = slot;

                        /*
                         * Find our Name entry.
                         */
                        curIndex += 11;
                        if ((pChar = strchr(curIndex, ' ')))
                        {
                            *pChar = '\0';
                            if (*curIndex != '\0')
                            {
                                while (*pChar != '/')
                                {
                                    --pChar;
                                }
                                ++pChar;

                                strncpy(rdDiskName, pChar, 32);
                                pDisk = XK_RaidMonitorRetrieveDeviceByName(rdDiskName);

                                /*
                                 * Look for a duplicate entry.
                                 */
                                if ((pDisk != NULL) &&
                                    (pDisk != &localRaids->disks[slot]))
                                {
                                    /*
                                     * If we found one, clear out the name of the old one.
                                     */
                                    pDisk->name[0] = '\0';

                                    dprintf(DPRINTF_DEFAULT, "LOCAL RAIDMON - XK_RaidMonitor Found duplicate entry for %s, old slot %d, new slot %d\n",
                                            rdDiskName, pDisk->slot, slot);
                                }

                                strncpy((char *)localRaids->disks[slot].name, pChar, 32);

                                /*
                                 * reset pDisk and rdDiskName.
                                 */
                                pDisk = NULL;
                                rdDiskName[0] = '\0';
                            }
                        }
                        else
                        {
                            dprintf(DPRINTF_DEFAULT, "LOCAL RAIDMON - XK_RaidMonitor RetrieveDevices Error Name\n");
                            rc = ERROR;
                            xkRaidmonForceCheck = 1;
                            break;
                        }

                        /*
                         * Find our Type.
                         */
                        curIndex += 17;
                        if ((pChar = strchr(curIndex, ' ')))
                        {
                            *pChar = '\0';
                            if (strncmp(curIndex, "Disk", 4) == 0)
                            {
                                localRaids->disks[slot].type = XK_RAIDMON_TYPE_DISK;
                            }
                            else
                            {
                                localRaids->disks[slot].type = XK_RAIDMON_TYPE_BACKPLANE;
                            }
                        }
                        else
                        {
                            dprintf(DPRINTF_DEFAULT, "LOCAL RAIDMON - XK_RaidMonitor RetrieveDevices Error Type\n");
                            rc = ERROR;
                            xkRaidmonForceCheck = 1;
                            break;
                        }

                        /*
                         * Find our vendor
                         */
                        curIndex += 5;
                        if ((pChar = strchr(curIndex, ' ')))
                        {
                            *pChar = '\0';
                            if (*curIndex != '\0')
                            {
                                strncpy((char *)localRaids->disks[slot].vendor, curIndex,
                                        32);
                            }
                        }
                        else
                        {
                            dprintf(DPRINTF_DEFAULT, "LOCAL RAIDMON - XK_RaidMonitor RetrieveDevices Error Vendor\n");
                            rc = ERROR;
                            xkRaidmonForceCheck = 1;
                            break;
                        }

                        /*
                         * Find our model.
                         */
                        curIndex += 9;
                        if ((pChar = strchr(curIndex, ' ')))
                        {
                            *pChar = '\0';
                            if (*curIndex != '\0')
                            {
                                strncpy((char *)localRaids->disks[slot].model, curIndex,
                                        32);
                            }
                        }
                        else
                        {
                            dprintf(DPRINTF_DEFAULT, "LOCAL RAIDMON - XK_RaidMonitor RetrieveDevices Error Model\n");
                            rc = ERROR;
                            xkRaidmonForceCheck = 1;
                            break;
                        }

                        /*
                         * Find our Firmware version
                         */
                        curIndex += 17;
                        if ((pChar = strchr(curIndex, ' ')))
                        {
                            *pChar = '\0';
                            if (*curIndex != '\0')
                            {
                                strncpy((char *)localRaids->disks[slot].fwV, curIndex, 8);
                            }
                        }
                        else
                        {
                            dprintf(DPRINTF_DEFAULT, "LOCAL RAIDMON - XK_RaidMonitor RetrieveDevices Error FWV\n");
                            rc = ERROR;
                            xkRaidmonForceCheck = 1;
                            break;
                        }

                        /*
                         * Find our Serial Number
                         */
                        curIndex += 5;
                        if ((pChar = strchr(curIndex, ' ')))
                        {
                            *pChar = '\0';
                            if (*curIndex != '\0')
                            {
                                strncpy((char *)localRaids->disks[slot].serial, curIndex,
                                        32);
                            }
                        }
                        else
                        {
                            dprintf(DPRINTF_DEFAULT, "LOCAL RAIDMON - XK_RaidMonitor RetrieveDevices Error Serial#\n");
                            rc = ERROR;
                            xkRaidmonForceCheck = 1;
                            break;
                        }

                        dprintf(DPRINTF_RAIDMON, "LOCAL RAIDMON - Raid Monitor sgsafte data slot (%d)\n"
                                "Name:          %s\n"
                                "Vendor:        %s\n"
                                "Model:         %s\n"
                                "Serial #:      %s\n"
                                "FWV:           %s\n"
                                "Type:          %d\n"
                                "Status:        %d\n",
                                slot,
                                localRaids->disks[slot].name,
                                localRaids->disks[slot].vendor,
                                localRaids->disks[slot].model,
                                localRaids->disks[slot].serial,
                                localRaids->disks[slot].fwV, localRaids->disks[slot].type,
                                localRaids->disks[slot].status);
                    }

                    /*
                     * Couldn't find slot.
                     */
                    else
                    {
                        dprintf(DPRINTF_DEFAULT, "LOCAL RAIDMON - XK_RaidMonitor RetrieveDevices Error Slot\n");
                        rc = ERROR;
                        xkRaidmonForceCheck = 1;
                        break;
                    }
                }
            }
        }

        /*
         * Else could not open file.  Log it.
         */
        else
        {
            dprintf(DPRINTF_DEFAULT, "LOCAL RAIDMON - XK_RaidMonitor RetrieveDevices Error Open file\n");
            xkRaidmonForceCheck = 1;
            rc = ERROR;
        }
    }
    else
    {
        xkRaidmonForceCheck = 1;
    }

    /*
     * Close the file.
     */
    if (pFH)
    {
        Fclose(pFH);
        pFH = NULL;
    }

    return rc;
}

/**
******************************************************************************
**
**  @brief      Set the drive status for the slot.
**
**              This will set the local status and if the drive is failed,
**              will turn on the led and alarm.  If OK, will turn off the led
**              and alarm.
**
**  @param      slot    - slot to set status on.
**              status  - status to change drive to.
**
**  @return     GOOD on Success,  ERROR on Failure.
**
******************************************************************************
**/
static INT32 XK_RaidMonitorSetDriveStatus(XK_RAIDMON_DEVICE *pDisk,
                                          UINT8 diskNum, UINT8 status)
{
    INT32       rc = GOOD;
    UINT8       execCmd = 0;

    /*
     * New status is ONLINE.
     */
    if (status == XK_RAIDMON_STATUS_ONLINE)
    {
        /*
         * Create the command to restore the drive.
         */
        sprintf(xk_raidmonCmdBuffer, "/sbin/sgsafte -d %d -r", pDisk->slot);

        /*
         * If the prior status is failed, dig a little deeper.
         */
        if (pDisk->status == XK_RAIDMON_STATUS_FAILED)
        {
            if ((1 << diskNum) & pDisk->failDevs)
            {
                dprintf(DPRINTF_DEFAULT, "LOCAL RAIDMON - Partition %d Disk (slot %d), OK\n",
                        diskNum, pDisk->slot);
            }

            /*
             * Clear the fail bit for this partition.
             */
            clear_bit(diskNum, &pDisk->failDevs);

            /*
             * If all the partitions are OK,
             * set the status of the disk to ONLINE
             * and log a message to the user.
             */
            if (pDisk->failDevs == 0)
            {
                LogMessage(LOG_TYPE_INFO, "Local SCSI Disk (slot %d) is OK", pDisk->slot);
                /*
                 * Set the new status.
                 */
                pDisk->status = status;

                /*
                 * Execute the command to set drive to ready.
                 */
                execCmd = 1;
            }
        }

        /*
         * Else, if the prior status is resyncing, dig a little deeper.
         */
        else if (pDisk->status == XK_RAIDMON_STATUS_RESYNCING)
        {
            if ((1 << diskNum) & pDisk->resyncDevs)
            {
                dprintf(DPRINTF_DEFAULT, "LOCAL RAIDMON - RESYNC Partition %d Disk (slot %d), OK\n",
                        diskNum, pDisk->slot);
            }

            /*
             * Clear the resync bit for this partition.
             */
            clear_bit(diskNum, &pDisk->resyncDevs);

            /*
             * If all the partitions are OK,
             * set the status of the disk to ONLINE
             * and log a message to the user that resync is done.
             */
            if (pDisk->resyncDevs == 0)
            {
                LogMessage(LOG_TYPE_INFO, "Local SCSI Disk (slot %d) RESYNC OK",
                           pDisk->slot);
                /*
                 * Clear resync Percent complete.
                 */
                pDisk->resyncPC = 0;

                /*
                 * Set the new status.
                 */
                pDisk->status = status;

                /*
                 * Execute the command to set drive to ready.
                 */
                execCmd = 1;
            }
        }

        /*
         * Else, if the prior status is none, this is a new drive.
         */
        else if (pDisk->status == XK_RAIDMON_STATUS_NONE)
        {
            LogMessage(LOG_TYPE_INFO, "Local SCSI Disk (slot %d) Inserted", pDisk->slot);
            /*
             * Set the new status.
             */
            pDisk->status = status;

            /*
             * Execute the command to set drive to ready.
             */
            execCmd = 1;
        }
    }

    /*
     * New status is RESYNCING.
     */
    else if (status == XK_RAIDMON_STATUS_RESYNCING)
    {
        /*
         * If prior status was failed, clear the fail bits.
         */
        if (pDisk->status == XK_RAIDMON_STATUS_FAILED)
        {
            pDisk->failDevs = 0;
        }

        /*
         * if this is the first partition to resync,
         * Log the resync and execute the ready command.
         */
        if (pDisk->resyncDevs == 0)
        {
            sprintf(xk_raidmonCmdBuffer, "/sbin/sgsafte -d %d -r", pDisk->slot);

            LogMessage(LOG_TYPE_WARNING, "Local SCSI Disk (slot %d) RESYNCING",
                       pDisk->slot);

            /*
             * Execute the command to set drive to ready.
             */
            execCmd = 1;

            /*
             * Set the global flag.
             */
            gLocalRaidFlux = XK_LOCAL_RAID_FLUX_TMO;
        }

        /*
         * Set the resync bit.
         */
        if (test_and_set_bit(diskNum, &pDisk->resyncDevs) == 0)
        {
            dprintf(DPRINTF_DEFAULT, "LOCAL RAIDMON - RESYNC Partition %d Disk (slot %d), scheduled.\n",
                    diskNum, pDisk->slot);
        }

        /*
         * Set the new status.
         */
        pDisk->status = status;

    }

    /*
     * New status is FAILED.
     */
    else
    {
        /*
         * If prior status was rsyning, clear the resync bits.
         */
        if (pDisk->status == XK_RAIDMON_STATUS_RESYNCING)
        {
            pDisk->resyncDevs = 0;
        }

        /*
         * if this is the first partition to fail,
         * Log the failure and execute the fail command.
         */
        if (pDisk->failDevs == 0)
        {
            sprintf(xk_raidmonCmdBuffer, "/sbin/sgsafte -d %d -f", pDisk->slot);

            LogMessage(LOG_TYPE_ERROR, "Local SCSI Disk (slot %d) FAIL", pDisk->slot);

            /*
             * Execute the command to set drive to ready.
             */
            execCmd = 1;

            /*
             * Set the global flag.
             */
            gLocalRaidFlux = XK_LOCAL_RAID_FLUX_TMO;
        }

        /*
         * Set the new status.
         */
        pDisk->status = status;

        /*
         * Set the fail bit.
         */
        if (test_and_set_bit(diskNum, &pDisk->failDevs) == 0)
        {
            dprintf(DPRINTF_DEFAULT, "LOCAL RAIDMON - Partition %d Disk (slot %d), FAILED.\n",
                    diskNum, pDisk->slot);
        }
    }

    if (execCmd)
    {
        /*
         * Execute the command.
         */
        rc = XK_RaidMonitorExecCommand();

        /*
         * If the command failed, force another check.
         */
        if (rc == ERROR)
        {
            xkRaidmonForceCheck = 1;
        }
    }

    return rc;
}

/**
******************************************************************************
**
**  @brief      Find a device by name in our local disk information.
**
**              Find a device by name in our local disk information.
**
**  @param      diskName    -   Name of disk to retrieve.
**
**  @return     Device pointer on Success,  NULL on Failure.
**
******************************************************************************
**/
static XK_RAIDMON_DEVICE *XK_RaidMonitorRetrieveDeviceByName(char *diskName)
{
    XK_RAIDMON_DEVICE *pDisk = NULL;
    UINT8       diskIndex = 0;
    char        tmpStr[128] = { 0 };
    char       *curIndex = tmpStr;

    if ((diskName != NULL) && (diskName[0] != '\0'))
    {
        strcpy(tmpStr, diskName);

        while ((*curIndex != '\0') && ((*curIndex < '0') || (*curIndex > '9')))
        {
            ++curIndex;
        }

        *curIndex = '\0';

        for (diskIndex = 0; diskIndex < XK_RAIDMON_MAX_DEVS; ++diskIndex)
        {
            if (strncmp(tmpStr,
                        (char *)localRaids->disks[diskIndex].name, strlen(tmpStr)) == 0)
            {
                dprintf(DPRINTF_RAIDMON, "LOCAL RAIDMON - %s - Found match for %s\n",
                        __func__, tmpStr);
                pDisk = &localRaids->disks[diskIndex];
                break;
            }
        }

        if (diskIndex == XK_RAIDMON_MAX_DEVS)
        {
            dprintf(DPRINTF_DEFAULT, "LOCAL RAIDMON - %s - Could not find match for %s\n",
                    __func__, tmpStr);
        }
    }

    return pDisk;
}


/**
******************************************************************************
**
**  @brief      Execute the command in the xk_raidmonCmdBuffer.
**
**              Execute the command in the xk_raidmonCmdBuffer.
**
**  @param      NONE.
**
**  @return     GOOD on Success,  ERROR on Failure.
**
******************************************************************************
**/
static INT32 XK_RaidMonitorExecCommand(void)
{
    INT32       rc;

    /* Execute the command in the command buffer. */

    rc = XK_System(xk_raidmonCmdBuffer);

    /* Clear out the buffer. */

    memset(xk_raidmonCmdBuffer, 0, sizeof(xk_raidmonCmdBuffer));

    /* Set the error code appropriately. */

    if (rc != GOOD)
    {
        dprintf(DPRINTF_DEFAULT, "LOCAL RAIDMON - %s FAILED\n", __func__);
        return ERROR;
    }

    return GOOD;
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
