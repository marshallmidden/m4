/* $Id: EL_BayMap.c 122127 2010-01-06 14:04:36Z m4 $ */
/**
******************************************************************************
**
**  @file       EL_BayMap.c
**
**  @brief      Election code worker functions to track bayMap connectivity
**
**  Election code worker functions to track bayMap connectivity.  These
**  should only be called by the election code.
**
**  Copyright (c) 2003-2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#include "EL_BayMap.h"

#include "CacheBay.h"
#include "CachePDisk.h"
#include "convert.h"
#include "cps_init.h"
#include "debug_files.h"
#include "EL.h"
#include "misc.h"
#include "pdd.h"
#include "quorum_utils.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
static void EL_BayMapGetDiskStateString(char *stringPtr, BAY_MAP_DISK_STATE diskState,
                                        UINT8 stringLength);
static UINT32 EL_BayMapDumpData(ELECTION_BAY_DATA *bayDataPtr, UINT32 bayIndex);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      EL_BayMapAddDisk -
**
**  @param      bayMapPtr
**  @param      bayIndex
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
static UINT32 EL_BayMapAddDisk(ELECTION_BAY_MAP *bayMapPtr, UINT32 bayIndex,
                               BAY_MAP_DISK_STATE diskState)
{
    UINT32      returnCode = ERROR;

    ccb_assert(bayMapPtr != NULL, bayMapPtr);

    if (bayMapPtr != NULL)
    {
        /*
         * Any drives that are added will increment the totalCount, but only
         * writable drives will increment the goodCount.  Also mark the bay
         * as having disksPresent for either writable or unwritable types.
         */
        if (bayIndex < dimension_of(bayMapPtr->map))
        {
            switch (diskState)
            {
                case BAY_MAP_DISK_STATE_UNKNOWN:
                    bayMapPtr->map[bayIndex].unknownCount++;
                    break;

                case BAY_MAP_DISK_STATE_WRITABLE:
                    bayMapPtr->map[bayIndex].goodCount++;
                    break;

                case BAY_MAP_DISK_STATE_UNWRITABLE:
                    break;

                case BAY_MAP_DISK_STATE_UNLABELED:
                    bayMapPtr->map[bayIndex].unlabeledCount++;
                    break;

                default:
                    break;
            }

            bayMapPtr->map[bayIndex].totalCount++;
            bayMapPtr->map[bayIndex].flags.bits.disksPresent = TRUE;
        }
        else
        {
            switch (diskState)
            {
                case BAY_MAP_DISK_STATE_UNKNOWN:
                    bayMapPtr->unknownBay.unknownCount++;
                    break;

                case BAY_MAP_DISK_STATE_WRITABLE:
                    bayMapPtr->unknownBay.goodCount++;
                    break;

                case BAY_MAP_DISK_STATE_UNWRITABLE:
                    break;

                case BAY_MAP_DISK_STATE_UNLABELED:
                    bayMapPtr->unknownBay.unlabeledCount++;
                    break;

                default:
                    break;
            }

            bayMapPtr->unknownBay.totalCount++;
            bayMapPtr->unknownBay.flags.bits.disksPresent = TRUE;
        }

        returnCode = GOOD;
    }
    else
    {
        dprintf(DPRINTF_ELECTION, "BMAD: bayMapPtr is NULL\n");
    }

    return (returnCode);
}


/**
******************************************************************************
**
**  @brief      EL_BayMapDump - Dump entire bayMap to display
**
**              Dump the entire bayMap to the debug output
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
UINT32 EL_BayMapDump(ELECTION_BAY_MAP *bayMapPtr)
{
    UINT32      returnCode = ERROR;
    UINT32      bayCounter = 0;

    ccb_assert(bayMapPtr != NULL, bayMapPtr);

    if (bayMapPtr != NULL)
    {
        /*
         * Scan all of the bayMaps, looking for disks that are writable
         * on all controllers.
         */
        dprintf(DPRINTF_ELECTION, "BMD: Dump entire bayMap\n");
        dprintf(DPRINTF_ELECTION, "BMD: Flags: 0x%02hhx\n", bayMapPtr->flags.value);

        /*
         * Display the unknownBay if it's valid
         */
        if ((bayMapPtr->unknownBay.totalCount != 0) ||
            (bayMapPtr->unknownBay.flags.value != 0))
        {
            EL_BayMapDumpData(&bayMapPtr->unknownBay, -1);
        }

        /*
         * Display the bayMap 'map' array
         */
        for (bayCounter = 0; bayCounter < dimension_of(bayMapPtr->map); bayCounter++)
        {
            /*
             * Don't dump bayMaps for invalid bays
             */
            if ((bayMapPtr->map[bayCounter].totalCount != 0) ||
                (bayMapPtr->map[bayCounter].flags.value != 0))
            {
                EL_BayMapDumpData(&bayMapPtr->map[bayCounter], bayCounter);
            }
        }

        returnCode = GOOD;
    }
    else
    {
        dprintf(DPRINTF_ELECTION, "BMD: bayMapPtr is NULL\n");
    }

    return (returnCode);
}


/**
******************************************************************************
**
**  @brief      EL_BayMapDumpData - Dump bayMapData entry to display
**
**              Dumps a specified bayMapData item to the debug output
**
**  @param      bayDataPtr - Pointer to the bayMap data
**  @param      bayIndex - Index to which the bayMap data belongs
**
**  @return     none
**
******************************************************************************
**/
static UINT32 EL_BayMapDumpData(ELECTION_BAY_DATA *bayDataPtr, UINT32 bayIndex)
{
    UINT32      returnCode = ERROR;

    ccb_assert(bayDataPtr != NULL, bayDataPtr);

    if (bayDataPtr != NULL)
    {
        /*
         * Watch for the bayIndex being in range.  If it's out of range, display
         * it as an unknown bay.
         */
        if (bayIndex < MAX_DISK_BAYS)
        {
            dprintf(DPRINTF_ELECTION, "BMDD: %cbayMap[%02d] flags: 0x%02hhx, good: %02d, unknown: %02d, unlabeled: %02d, total: %02d\n",
                    ((bayDataPtr->goodCount > 0) ? ' ' : '*'), bayIndex,
                    bayDataPtr->flags.value, bayDataPtr->goodCount,
                    bayDataPtr->unknownCount, bayDataPtr->unlabeledCount,
                    bayDataPtr->totalCount);
        }
        else
        {
            dprintf(DPRINTF_ELECTION, "BMDD: %cbayMap[??] flags: 0x%02hhx, good: %02d, unknown: %02d, unlabeled: %02d, total: %02d\n",
                    ((bayDataPtr->goodCount > 0) ? ' ' : '*'), bayDataPtr->flags.value,
                    bayDataPtr->goodCount, bayDataPtr->unknownCount,
                    bayDataPtr->unlabeledCount, bayDataPtr->totalCount);
        }

        returnCode = GOOD;
    }
    else
    {
        dprintf(DPRINTF_ELECTION, "BMDD: [%02d]-Bad bayDataPtr\n", bayIndex);
    }

    return (returnCode);
}


/**
******************************************************************************
**
**  @brief      EL_BayMapInitialize - Initialize the specified bayMap
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
static UINT32 EL_BayMapInitialize(ELECTION_BAY_MAP *bayMapPtr)
{
    UINT32      returnCode = ERROR;

    ccb_assert(bayMapPtr != NULL, bayMapPtr);
    dprintf(DPRINTF_ELECTION, "BMI: Initialize the bayMap\n");

    if (bayMapPtr != NULL)
    {
        memset(bayMapPtr, 0, sizeof(*bayMapPtr));
        returnCode = GOOD;
    }
    else
    {
        dprintf(DPRINTF_ELECTION, "BMI: bayMapPtr is NULL\n");
    }

    return (returnCode);
}


/**
******************************************************************************
**
**  @brief      EL_BayMapLocateDisks - Associate each pdisk to a bay
**
**  @param      bayMapPtr
**  @param      diskMapPtr
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
static UINT32 EL_BayMapLocateDisks(ELECTION_BAY_MAP *bayMapPtr,
                                   ELECTION_DISK_MAP *diskMapPtr)
{
    UINT32      returnCode = ERROR;
    UINT32      diskNumber = 0;
    BAY_MAP_DISK_STATE diskState = BAY_MAP_DISK_STATE_UNKNOWN;
    MRGETPINFO_RSP deviceInfo;  /* Uninitialized */
    char        diskStateString[40] = { 0 };

    ccb_assert(bayMapPtr != NULL, bayMapPtr);
    ccb_assert(diskMapPtr != NULL, diskMapPtr);

    if ((bayMapPtr != NULL) && (diskMapPtr != NULL))
    {
        /*
         * Walk through this controller's pdisks to see which disks are
         * currently writable.  Once a writable disk is found, look at the SES
         * and SLOT information or DNAME to determine which bay it belongs to.
         * Once the bay is determined, increment the bay's counter.
         */
        for (diskNumber = 0; diskNumber < MAX_PHYSICAL_DISKS; diskNumber++)
        {
            /*
             * Look to see if the bit is set in the diskMap.
             * If the diskMap is not valid, then treat all of the pdisks
             * visible in the cache as writable.  Otherwise, check if it's
             * in the diskMap.  If it's not in the diskMap, then this pdisk
             * isn't writable from this controller and should be counted as
             * such (below).
             */
            if (diskMapPtr->flags.bits.mapValid == TRUE)
            {
                if ((diskMapPtr->local[diskNumber / (sizeof(diskMapPtr->local[0]) * 8)]) &
                    (1 << (diskNumber % (sizeof(diskMapPtr->local[0]) * 8))))
                {
                    diskState = BAY_MAP_DISK_STATE_WRITABLE;
                }
                else
                {
                    diskState = BAY_MAP_DISK_STATE_UNWRITABLE;
                }
            }

            if (GetPDiskInfoFromPid(diskNumber, &deviceInfo) == GOOD)
            {
                /*
                 * Override the diskState of unlabeled drives
                 */
                if (deviceInfo.pdd.devClass == PD_UNLAB)
                {
                    diskState = BAY_MAP_DISK_STATE_UNLABELED;
                }

                /*
                 * Decode the diskState string
                 */
                EL_BayMapGetDiskStateString(diskStateString, diskState, sizeof(diskStateString));

                /*
                 * Display the bayMap to the console
                 */
                dprintf(DPRINTF_ELECTION_VERBOSE, "BMLD: %cDiskID %04d, pd_ses: %05d, dname[ses]: %03d, pd_slot: %03d, dname[slot]: %03d (%s)\n",
                        ((diskState == BAY_MAP_DISK_STATE_WRITABLE) ? ' ' : '*'),
                        diskNumber, deviceInfo.pdd.ses,
                        deviceInfo.pdd.devName[PD_DNAME_CSES], deviceInfo.pdd.slot,
                        deviceInfo.pdd.devName[PD_DNAME_CSLOT], diskStateString);

                /*
                 * Check for valid SES data.  If the SES data is not valid, look
                 * at the DNAME data.  If the DNAME data is not valid, then go
                 * on to the next drive.  The ses data should be ignored until
                 * discovery has been run.  Don't use DNAME if the device is
                 * unlabeled, as it will always have a 'name' for bay zero.
                 */
                if ((DiscoveryComplete() == true) && (deviceInfo.pdd.ses < MAX_DISK_BAYS))
                {
                    EL_BayMapAddDisk(bayMapPtr, deviceInfo.pdd.ses, diskState);
                }
                else if ((deviceInfo.pdd.devClass != PD_UNLAB) &&
                         ((deviceInfo.pdd.devStat == PD_OP) ||
                          (deviceInfo.pdd.devStat == PD_INOP)) &&
                         (deviceInfo.pdd.devName[PD_DNAME_CSES] < MAX_DISK_BAYS))
                {
                    EL_BayMapAddDisk(bayMapPtr, deviceInfo.pdd.devName[PD_DNAME_CSES], diskState);
                }
                else
                {
                    /*
                     * Couldn't find SES/Slot number for this PID.  Throw this
                     * disk into the 'unknown bay' category.
                     */
                    EL_BayMapAddDisk(bayMapPtr, -1, diskState);
                }
            }
            else
            {
                /*
                 * The cache has no knowledge of this PID.  If the pDisk is
                 * writable, throw this disk into the 'unknown bay' category.
                 */
                if (diskState == BAY_MAP_DISK_STATE_WRITABLE)
                {
                    dprintf(DPRINTF_ELECTION_VERBOSE, "BMLD: PID %d not found in CCB cache, but is writable\n",
                            diskNumber);

                    EL_BayMapAddDisk(bayMapPtr, -1, diskState);
                }
            }
        }

        returnCode = GOOD;
    }
    else
    {
        dprintf(DPRINTF_ELECTION, "BMLD: bayMapPtr or diskMapPtr is NULL\n");
    }

    return (returnCode);
}

/**
******************************************************************************
**
**  @brief      EL_BayMapUpdate -
**
**  @param      none
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
UINT32 EL_BayMapUpdate(ELECTION_BAY_MAP *bayMapPtr)
{
    UINT32      returnCode = ERROR;
    UINT32      bayIndex = 0;
    MRGETEINFO_RSP deviceInfo;          /* Uninitialized */
    ELECTION_DISK_MAP myDiskMap;        /* Uninitialized */

    ccb_assert(bayMapPtr != NULL, bayMapPtr);

    if (bayMapPtr != NULL)
    {
        EL_BayMapInitialize(bayMapPtr);

        for (bayIndex = 0; bayIndex < dimension_of(bayMapPtr->map); bayIndex++)
        {
            if (GetDiskBayInfoFromBid(bayIndex, &deviceInfo) == GOOD)
            {
                /*
                 * Display the bay data being used for the update
                 */
                dprintf(DPRINTF_ELECTION, "BMU: Index: %04x, BayID: %04d, pd_ses: %05d, dname[ses]: %03d, pd_slot: %03d, dname[slot]: %03d\n",
                        bayIndex, deviceInfo.pdd.pid, deviceInfo.pdd.ses,
                        deviceInfo.pdd.devName[PD_DNAME_CSES], deviceInfo.pdd.slot,
                        deviceInfo.pdd.devName[PD_DNAME_CSLOT]);

                /*
                 * Mark the bay as being 'present' in the system
                 */
                if (deviceInfo.pdd.pid < dimension_of(bayMapPtr->map))
                {
                    bayMapPtr->map[deviceInfo.pdd.pid].flags.bits.bayPresent = TRUE;
                }
                else
                {
                    dprintf(DPRINTF_ELECTION, "BMU: ERROR - pdd.pid exceeds map array dimension\n");
                }
            }
        }

        bayMapPtr->flags.bits.mapValid = TRUE;

        /*
         * Locate all of the physical disks
         */
        if (EL_DiskMapGet(&myDiskMap) == GOOD)
        {
            returnCode = EL_BayMapLocateDisks(bayMapPtr, &myDiskMap);
        }
    }
    else
    {
        dprintf(DPRINTF_ELECTION, "BMU: bayMapPtr is NULL\n");
    }

    return (returnCode);
}


/*----------------------------------------------------------------------------
**  Function Name: EL_BayMapGetDiskStateString
**
**  Parameters:    stringPtr    - pointer to store the constructed string
**                 diskState    - state number to decode
**                 stringLength - size of the space allocated for stringPtr
**
**--------------------------------------------------------------------------*/
static void EL_BayMapGetDiskStateString(char *stringPtr, BAY_MAP_DISK_STATE diskState,
                                        UINT8 stringLength)
{
    if (stringPtr != NULL)
    {
        switch (diskState)
        {
            case BAY_MAP_DISK_STATE_WRITABLE:
                strncpy(stringPtr, "WRITABLE", stringLength);
                break;

            case BAY_MAP_DISK_STATE_UNWRITABLE:
                strncpy(stringPtr, "UNWRITABLE", stringLength);
                break;

            case BAY_MAP_DISK_STATE_UNLABELED:
                strncpy(stringPtr, "UNLABELED", stringLength);
                break;

            case BAY_MAP_DISK_STATE_UNKNOWN:
            default:
                strncpy(stringPtr, "UNKNOWN", stringLength);
                break;
        }

        /*
         * Make sure the strncpy is terminated
         */
        if (stringLength > 0)
        {
            stringPtr[stringLength - 1] = '\0';
        }
    }
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
