/* $Id: EL_DiskMap.c 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       EL_DiskMap.c
**
**  @brief      Election code worker functions to track diskMap connectivity
**
**  Election code worker functions to track diskMap connectivity.  These
**  should only be called by the election code.
**
**  Copyright (c) 2003-2009 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#include "EL_DiskMap.h"

#include "CachePDisk.h"
#include "convert.h"
#include "debug_files.h"
#include "EL.h"
#include "EL_BayMap.h"
#include "EL_KeepAlive.h"
#include "FIO_Maps.h"
#include "misc.h"
#include "quorum_utils.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"

/*
******************************************************************************
** Private variables
******************************************************************************
*/
static ELECTION_DISK_MAP_LIST diskMapList;

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
static void EL_DiskMapDumpItem(ELECTION_DISK_MAP *diskMapPtr, UINT32 slotNumber);
static UINT32 EL_DiskMapIntersect(FIO_DISK_MAP *intersectMapPtr);
static UINT32 EL_DiskMapInvalidateSlot(UINT16 slotNumber);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      EL_DiskMapReceive - Handle disk map from other controllers
**
**              This function processes a received diskMap entry.  This
**              should be called by the packet reception handler or by an
**              election callback handler.
**
**  @param      diskMapPtr - Pointer to the diskMap data to process
**  @param      packetCommSlot - CommSlot which created the diskMap
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
UINT32 EL_DiskMapReceive(ELECTION_DISK_MAP *diskMapPtr, UINT32 packetCommSlot)
{
    UINT32      returnCode = ERROR;
    UINT32      mapIndex = 0;

    ccb_assert(diskMapPtr != NULL, diskMapPtr);
    dprintf(DPRINTF_ELECTION, "DMR: Processing received diskMap\n");

    if (diskMapPtr != NULL)
    {
        /*
         * Check that the packetCommSlot is within the valid range.
         */
        if (packetCommSlot < MAX_CONTROLLERS)
        {
            dprintf(DPRINTF_ELECTION, "DMR: diskMap received for slot %d\n",
                    packetCommSlot);

            if (diskMapPtr->flags.bits.mapValid == TRUE)
            {
                /*
                 * If this is the first time that we're writing this slot's diskMap,
                 * then do a straight copy of the packet disk map into the
                 * election diskMapList.  If it's not the fist time, then intersect
                 * the packet diskMap with our previous diskMap so that we know
                 * only the drives that were writable for the entire election.
                 */
                if (diskMapList.slotList[packetCommSlot].flags.bits.mapValid != TRUE)
                {
                    /*
                     * Copy the map from the packet data to the global structure.
                     */
                    memcpy(&diskMapList.slotList[packetCommSlot].local,
                           &diskMapPtr->local,
                           sizeof(diskMapList.slotList[packetCommSlot].local));

                    memcpy(&diskMapList.slotList[packetCommSlot].intersect,
                           &diskMapPtr->intersect,
                           sizeof(diskMapList.slotList[packetCommSlot].intersect));

                    /*
                     * Mark the data as valid, so we know it's usable.
                     */
                    diskMapList.slotList[packetCommSlot].flags.bits.mapValid = TRUE;
                }
                else
                {
                    /*
                     * Intersect the packet diskMap with the diskMapList array.
                     * We're only remembering drives that have consistently
                     * reported as being written.
                     */
                    for (mapIndex = 0;
                         mapIndex < dimension_of(diskMapList.slotList[packetCommSlot].local);
                         mapIndex++)
                    {
                        diskMapList.slotList[packetCommSlot].local[mapIndex] &= diskMapPtr->local[mapIndex];

                        diskMapList.slotList[packetCommSlot].intersect[mapIndex] &= diskMapPtr->intersect[mapIndex];
                    }
                }
            }
            else
            {
                dprintf(DPRINTF_ELECTION, "DMR: Received diskMap for slot %d is not valid\n",
                        packetCommSlot);
            }

            EL_DiskMapUpdateFIOMap();
            EL_DiskMapDumpItem(diskMapPtr, packetCommSlot);
            returnCode = GOOD;
        }
        else
        {
            dprintf(DPRINTF_ELECTION, "DMR: Invalid packetCommSlot: %d\n",
                    packetCommSlot);
        }
    }
    else
    {
        dprintf(DPRINTF_ELECTION, "DMR: diskMapPtr is NULL\n");
    }

    return (returnCode);
}


/**
******************************************************************************
**
**  @brief      EL_DiskMapIntersect - Look for pdisk intersections
**
**              This function intersects all of the valid diskMap entries
**              in diskMapList and returns the intersected output to the
**              address specified by diskMapPtr.  If none of the diskMapList
**              is valid (as would occur at powerup) then return a map
**              that contains all ones.
**
**  @param      intersectMapPtr - pointer to where intersection data is written
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
static UINT32 EL_DiskMapIntersect(FIO_DISK_MAP *intersectMapPtr)
{
    UINT32      returnCode = ERROR;
    UINT32      mapIndex = 0;
    UINT16      slotCounter = ACM_NODE_UNDEFINED;

    ccb_assert(intersectMapPtr != NULL, intersectMapPtr);

    if (intersectMapPtr != NULL)
    {
        /*
         * Initialize the intersect map
         * NOTE: We're setting the map values to all ones to make the
         *       intersection logic easier.
         */
        for (mapIndex = 0; mapIndex < dimension_of(*intersectMapPtr); mapIndex++)
        {
            (*intersectMapPtr)[mapIndex] = -1;
        }

        /*
         * Do the diskMap intersection stuff
         */
        for (slotCounter = 0; slotCounter < MAX_CONTROLLERS; slotCounter++)
        {
            /*
             * Intersect with all controllers that have a valid maps.
             */
            if (diskMapList.slotList[slotCounter].flags.bits.mapValid == TRUE)
            {
                /*
                 * Intersect the diskMapList into the intersect map
                 * dprintf( DPRINTF_ELECTION, "DMI: Intersecting with slot %d maps\n",
                 * slotCounter );
                 */
                for (mapIndex = 0; mapIndex < dimension_of(*intersectMapPtr); mapIndex++)
                {
                    (*intersectMapPtr)[mapIndex] &= diskMapList.slotList[slotCounter].local[mapIndex];

                    (*intersectMapPtr)[mapIndex] &= diskMapList.slotList[slotCounter].intersect[mapIndex];
                }
            }
        }

        /*
         * All valid diskMaps were intersected, so return GOOD.
         */
        returnCode = GOOD;
    }
    else
    {
        dprintf(DPRINTF_ELECTION, "DMI: intersectMapPtr is NULL\n");
    }

    return (returnCode);
}


/**
******************************************************************************
**
**  @brief      EL_DiskMapCount - Look for pdisk intersections
**
**              This function counts the number of pdisk bits that are
**              set in the diskMap.
**
**  @param      mapPtr
**
**  @return     Number of bits set in the diskMap
**
******************************************************************************
**/
static UINT32 EL_DiskMapCount(FIO_DISK_MAP *diskMapPtr)
{
    UINT32      count = 0;
    UINT32      mapIndex = 0;
    UINT32      bitIndex = 0;

    ccb_assert(diskMapPtr != NULL, diskMapPtr);

    if (diskMapPtr != NULL)
    {
        /*
         * Count the number of bits set in the diskMap array
         */
        for (mapIndex = 0; mapIndex < dimension_of(*diskMapPtr); mapIndex++)
        {
            for (bitIndex = 0; bitIndex < sizeof((*diskMapPtr)[0]) * 8; bitIndex++)
            {
                if ((*diskMapPtr)[mapIndex] & (1 << bitIndex))
                {
                    count++;
                }
            }
        }
    }

    dprintf(DPRINTF_ELECTION, "DMC: diskMapCount: %d\n", count);

    return (count);
}


/**
******************************************************************************
**
**  @brief      EL_DiskMapUpdate - Update diskMapList from file system data
**
**              This function retrieves the current diskMap data from the
**              file system and merges it into the diskMapList array.
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
static UINT32 EL_DiskMapUpdate(ELECTION_DISK_MAP_LIST *diskMapListPtr)
{
    UINT32      returnCode = ERROR;
    UINT32      mapIndex = 0;
    UINT16      thisControllerCommSlot = ACM_NODE_UNDEFINED;
    FIO_DISK_MAP currentDiskMap;

    ccb_assert(diskMapListPtr != NULL, diskMapListPtr);

    memset(&currentDiskMap, 0, sizeof(currentDiskMap));

    if (diskMapListPtr != NULL)
    {
        /*
         * Load up a copy of this controller's commSlot.  Since packetReception
         * might be occurring outside of the election task, the global
         * 'myControllerCommSlot' might not be valid.  Also, avoid the possibility
         * of modifying the myControllerCommSlot while the election task is running.
         */
        thisControllerCommSlot = GetCommunicationsSlot(GetMyControllerSN());

        /*
         * Update the current diskMap for this controller.  Grab the
         * file system's diskMap and merge it with the election diskMap.
         */
        if (FIO_GetWritableDiskMap(&currentDiskMap) == GOOD)
        {
            /*
             * If this is the first time that we're reading the diskMap, then
             * do a straight copy of the file system's disk map into the
             * election diskMapList.  If it's not the fist time, then intersect
             * the file system diskMap with our previous diskMap so that we know
             * only the drives that were writable for the entire election.
             */
            if (diskMapListPtr->slotList[thisControllerCommSlot].flags.bits.mapValid != TRUE)
            {
                dprintf(DPRINTF_ELECTION, "DMU: First update of local diskMap\n");

                memcpy(&diskMapListPtr->slotList[thisControllerCommSlot].local,
                       &currentDiskMap,
                       sizeof(diskMapListPtr->slotList[thisControllerCommSlot].local));

                diskMapListPtr->slotList[thisControllerCommSlot].flags.bits.mapValid = TRUE;
            }
            else
            {
                dprintf(DPRINTF_ELECTION, "DMU: Update local diskMap - intersected\n");

                /*
                 * Intersect the currentDiskMap into this controller's
                 * portion of the diskMapList array.  We're only remembering
                 * drives that have consistently reported as being written.
                 */
                for (mapIndex = 0;
                     mapIndex < dimension_of(diskMapListPtr->slotList[thisControllerCommSlot].local);
                     mapIndex++)
                {
                    diskMapListPtr->slotList[thisControllerCommSlot].local[mapIndex] &= currentDiskMap[mapIndex];
                }
            }

            /*
             * Now that the local map is updated and valid, make the new intersect map
             */
            returnCode = EL_DiskMapIntersect(&diskMapListPtr->slotList[thisControllerCommSlot].intersect);
        }
        else
        {
            dprintf(DPRINTF_ELECTION, "DMU: FIO_GetWriteableDiskMap returned ERROR\n");

            returnCode = EL_DiskMapInvalidateSlot(thisControllerCommSlot);
        }
    }
    else
    {
        dprintf(DPRINTF_ELECTION, "DMI: diskMapListPtr is NULL\n");
    }

    return (returnCode);
}

/**
******************************************************************************
**
**  @brief      EL_DiskMapDumpItem - Dump diskMap entry to display
**
**              Dumps a specified diskMap item to the debug output
**
**  @param      diskMapPtr - Pointer to the diskMap data
**  @param      slotNumber - Slot number to which the diskMap data belongs
**
**  @return     none
**
******************************************************************************
**/
static void EL_DiskMapDumpItem(ELECTION_DISK_MAP *diskMapPtr, UINT32 slotNumber)
{
    UINT32      mapIndex = 0;
    UINT32      maxDisk = 0;
    UINT8       swap = 0;
    char        mapText[(2 * (MAX_PHYSICAL_DISKS + 7) / 8) + 1] = { 0 };

    ccb_assert(diskMapPtr != NULL, diskMapPtr);

    /*
     * Scan all of the diskMaps, looking for disks that are writable
     * on all controllers.
     */
    if (diskMapPtr != NULL)
    {
        if (diskMapPtr->flags.bits.mapValid == TRUE)
        {
            /*
             * Local map
             *
             * Find the maximum configured pDisk ID
             */
            maxDisk = dimension_of(diskMapPtr->local);

            while ((maxDisk > 0) && (diskMapPtr->local[maxDisk - 1] == 0))
            {
                maxDisk--;
            }

            /*
             * Bit swap each byte in the map
             */
            for (mapIndex = 0; mapIndex < maxDisk; mapIndex++)
            {
                /*
                 * Swap bit order, so that the lowest pdisk is in the leftmost position
                 */
                swap = ((diskMapPtr->local[mapIndex] & 0x80) >> 7) |
                    ((diskMapPtr->local[mapIndex] & 0x40) >> 5) |
                    ((diskMapPtr->local[mapIndex] & 0x20) >> 3) |
                    ((diskMapPtr->local[mapIndex] & 0x10) >> 1) |
                    ((diskMapPtr->local[mapIndex] & 0x08) << 1) |
                    ((diskMapPtr->local[mapIndex] & 0x04) << 3) |
                    ((diskMapPtr->local[mapIndex] & 0x02) << 5) |
                    ((diskMapPtr->local[mapIndex] & 0x01) << 7);

                charval(swap >> 4, &mapText[(mapIndex * 2)]);
                charval(swap & 0x0F, &mapText[(mapIndex * 2) + 1]);
            }

            mapText[mapIndex * 2] = '\0';

            dprintf(DPRINTF_ELECTION, "DMDI:   (L) [%02d]-0x%02hhx:%s\n",
                    slotNumber, diskMapPtr->flags.value, mapText);

            /*
             * Intersected map
             *
             * Bit swap each byte in the map
             */
            for (mapIndex = 0; mapIndex < maxDisk; mapIndex++)
            {
                /*
                 * Swap bit order, so that the lowest pdisk is in the leftmost position
                 */
                swap = ((diskMapPtr->intersect[mapIndex] & 0x80) >> 7) |
                    ((diskMapPtr->intersect[mapIndex] & 0x40) >> 5) |
                    ((diskMapPtr->intersect[mapIndex] & 0x20) >> 3) |
                    ((diskMapPtr->intersect[mapIndex] & 0x10) >> 1) |
                    ((diskMapPtr->intersect[mapIndex] & 0x08) << 1) |
                    ((diskMapPtr->intersect[mapIndex] & 0x04) << 3) |
                    ((diskMapPtr->intersect[mapIndex] & 0x02) << 5) |
                    ((diskMapPtr->intersect[mapIndex] & 0x01) << 7);

                charval(swap >> 4, &mapText[(mapIndex * 2)]);
                charval(swap & 0x0F, &mapText[(mapIndex * 2) + 1]);
            }

            mapText[mapIndex * 2] = '\0';

            dprintf(DPRINTF_ELECTION, "DMDI:   (I) [%02d]-0x%02hhx:%s\n",
                    slotNumber, diskMapPtr->flags.value, mapText);
        }
        else
        {
            dprintf(DPRINTF_ELECTION, "DMDI:       [%02d]-0x%02hhx:Not valid\n",
                    slotNumber, diskMapPtr->flags.value);
        }
    }
    else
    {
        dprintf(DPRINTF_ELECTION, "DMDI: [%02d]-diskMapPtr is NULL\n", slotNumber);
    }
}


/**
******************************************************************************
**
**  @brief      EL_DiskMapResetList - Initialize the diskMapList array
**
**              Clears the map only if the valid flag is FALSE.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void EL_DiskMapResetList(void)
{
    UINT32      counter = 0;

    dprintf(DPRINTF_ELECTION, "DMRL: Reset the diskMapList\n");

    for (counter = 0; counter < dimension_of(diskMapList.slotList); counter++)
    {
        /*
         * Reset the maps if they're not valid
         */
        if (diskMapList.slotList[counter].flags.bits.mapValid == FALSE)
        {
            diskMapList.slotList[counter].flags.value = 0;

            memset(&diskMapList.slotList[counter].local, 0,
                   sizeof(diskMapList.slotList[counter].local));

            memset(&diskMapList.slotList[counter].intersect, 0,
                   sizeof(diskMapList.slotList[counter].intersect));
        }
        else
        {
            dprintf(DPRINTF_ELECTION, "DMRL: Slot %d still valid - skipping\n", counter);
        }
    }
}


/**
******************************************************************************
**
**  @brief      EL_DiskMapInvalidateList - Invalidates the diskMapList array
**
**              Sets the valid bit for each entry to FALSE, but leaves
**              the map data untouched.  Call this function and follow it
**              with a call to DiskMapListInitialize if you want to clear.
**
**  @param      none
**
**  @return     none
**
******************************************************************************
**/
void EL_DiskMapInvalidateList(void)
{
    UINT32      slotCounter = 0;

    dprintf(DPRINTF_ELECTION, "DMIL: Invalidate the diskMapList\n");

    for (slotCounter = 0; slotCounter < dimension_of(diskMapList.slotList); slotCounter++)
    {
        EL_DiskMapInvalidateSlot(slotCounter);
    }
}


/**
******************************************************************************
**
**  @brief      EL_DiskMapInvalidateSlot - Invalidates a diskMap entry based on slot
**
**              Sets the valid bit for each entry to FALSE, but leaves
**              the map data untouched.  Call this function and follow it
**              with a call to DiskMapListInitialize if you want to clear.
**
**  @param      diskMapPtr - Pointer to diskMap to invalidate
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
static UINT32 EL_DiskMapInvalidateSlot(UINT16 slotNumber)
{
    UINT32      returnCode = GOOD;

    ccb_assert(slotNumber < dimension_of(diskMapList.slotList), slotNumber);

    if (slotNumber < dimension_of(diskMapList.slotList))
    {
        diskMapList.slotList[slotNumber].flags.value = 0;
    }
    else
    {
        dprintf(DPRINTF_ELECTION, "DMIS: slotNumber is out-of-range\n");
        returnCode = ERROR;
    }

    return (returnCode);
}


/**
******************************************************************************
**
**  @brief      EL_DiskMapGet - Gets a copy of this controller's local diskMap
**
**              Gets a copy of this controller's diskMap
**
**  @param      diskMapPtr
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
UINT32 EL_DiskMapGet(ELECTION_DISK_MAP *diskMapPtr)
{
    UINT16      thisControllerCommSlot;

    ccb_assert(diskMapPtr != NULL, diskMapPtr);

    /* Check for valid input */
    if (diskMapPtr == NULL)
    {
        return ERROR;
    }

    /* Get this controller's commSlot. */
    thisControllerCommSlot = GetCommunicationsSlot(GetMyControllerSN());

    if ((thisControllerCommSlot != ACM_NODE_UNDEFINED) &&
        (thisControllerCommSlot < dimension_of(diskMapList.slotList)))
    {
        EL_DiskMapUpdate(&diskMapList);

        memcpy(diskMapPtr, &diskMapList.slotList[thisControllerCommSlot],
               sizeof(*diskMapPtr));

        return GOOD;
    }

    /* commSlot out of range - set diskMapPtr to all zeros */
    memset(diskMapPtr, 0, sizeof(*diskMapPtr));
    return ERROR;
}


/**
******************************************************************************
**
**  @brief      EL_DiskMapUpdateFIOMap -
**
**              Gets a copy of this controller's diskMap
**
**  @param      none
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
UINT32 EL_DiskMapUpdateFIOMap(void)
{
    UINT32      returnCode = GOOD;
    UINT32      mapIndex = 0;
    UINT16      thisControllerCommSlot = ACM_NODE_UNDEFINED;
    FIO_DISK_MAP currentMap;    /* Uninitialized */
    FIO_DISK_MAP intersectMap;  /* Uninitialized */
    UINT8       mapChange = FALSE;

    /* Get this controller's commSlot. */
    thisControllerCommSlot = GetCommunicationsSlot(GetMyControllerSN());

    if ((thisControllerCommSlot != ACM_NODE_UNDEFINED) &&
        (thisControllerCommSlot < dimension_of(diskMapList.slotList)))
    {
        EL_DiskMapUpdate(&diskMapList);

        if (diskMapList.slotList[thisControllerCommSlot].flags.bits.mapValid == TRUE)
        {
            if (EL_DiskMapCount(&diskMapList.slotList[thisControllerCommSlot].intersect) > 0)
            {
                /* Scan for a change in the readable diskMap */
                if (FIO_GetReadableDiskMap(&currentMap) == GOOD)
                {
                    for (mapIndex = 0;
                         (mapIndex < dimension_of(currentMap)) && (mapChange == FALSE);
                         mapIndex++)
                    {
                        if (diskMapList.slotList[thisControllerCommSlot].intersect[mapIndex] != currentMap[mapIndex])
                        {
                            mapChange = TRUE;
                        }
                    }
                }
                else
                {
                    /* The readable diskMap isn't valid, so make sure we update it. */
                    mapChange = TRUE;
                }

                /* Update the readable diskMap if a change was detected. */
                if (mapChange == TRUE)
                {
                    dprintf(DPRINTF_ELECTION, "DMUFM: Updating FIO diskMap\n");
                    FIO_SetReadableDiskMap(&diskMapList.slotList[thisControllerCommSlot].intersect);
                    FIO_MapDump(&diskMapList.slotList[thisControllerCommSlot].intersect);
                }
            }
            else
            {
                dprintf(DPRINTF_ELECTION, "DMUFM: No good pdisks from which to read - !! DISASTER !!\n");
                returnCode = EL_DisasterTakeAction("Storage fragmented");
            }
        }
        else
        {
            /*
             * McMaster - more work to do here - go with the pdisks that are
             * visible by the most controllers, giving priority to any pdisks
             * visible from a keepAlive system.
             */
            if ((EL_DiskMapIntersect(&intersectMap) == GOOD) &&
                (EL_DiskMapCount(&intersectMap) > 0))
            {
                dprintf(DPRINTF_ELECTION, "DMUFM: Updating FIO diskMap - local diskMap not yet valid\n");
                FIO_SetReadableDiskMap(&intersectMap);

                /* Dump the new diskMap, if it's not all ones */
                if (EL_DiskMapCount(&intersectMap) < MAX_PHYSICAL_DISKS)
                {
                    FIO_MapDump(&intersectMap);
                }
            }
            else
            {
                dprintf(DPRINTF_ELECTION, "DMUFM: No good pdisks from which to read - reset FIO diskMap\n");
                FIO_ResetReadableDiskMap();
            }
        }
    }

    return (returnCode);
}


/**
******************************************************************************
**
**  @brief      EL_DiskMapSlotOverlap - Look for overlap with a slot
**
**              Look for overlap with a slot
**
**  @param      slotNumber - slot to look for overlaps with
**
**  @return     overlap count
**
******************************************************************************
**/
UINT32 EL_DiskMapSlotOverlap(UINT16 slotNumber)
{
    UINT32            mapIndex;
    ELECTION_DISK_MAP thisSlotDiskMap;

    ccb_assert(slotNumber < dimension_of(diskMapList.slotList), slotNumber);

    if (EL_DiskMapGet(&thisSlotDiskMap) != GOOD)
    {
        return 0;
    }
    if (diskMapList.slotList[slotNumber].flags.bits.mapValid == TRUE)
    {
        for (mapIndex = 0; mapIndex < dimension_of(thisSlotDiskMap.local); mapIndex++)
        {
            thisSlotDiskMap.local[mapIndex] &= diskMapList.slotList[slotNumber].local[mapIndex];
        }
    }
    return (EL_DiskMapCount(&thisSlotDiskMap.local));
}


/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
