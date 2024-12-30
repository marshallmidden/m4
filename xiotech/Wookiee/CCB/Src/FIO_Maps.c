/* $Id: FIO_Maps.c 122127 2010-01-06 14:04:36Z m4 $ */
/**
******************************************************************************
**
**  @file       FIO_Maps.c
**
**  Copyright (c) 2003-2009 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#include "FIO_Maps.h"

#include "convert.h"
#include "cps_init.h"
#include "debug_files.h"
#include "EL_DiskMap.h"
#include "misc.h"
#include "XIO_Const.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"

/*
******************************************************************************
** Private variables
******************************************************************************
*/

/* Array to track the writable physical disks */
static UINT8 readableDiskMapValid = FALSE;
static UINT8 writableDiskMapValid = FALSE;
static FIO_DISK_MAP readableDiskMap = { 0 };
static FIO_DISK_MAP writableDiskMap = { 0 };

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      FIO_GetReadableDiskMap - Get copy of readable disk map
**
**  @param      diskMapPtr - Pointer to where the map is to be copied
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
UINT32 FIO_GetReadableDiskMap(FIO_DISK_MAP *diskMapPtr)
{
    UINT32      returnCode = ERROR;

    ccb_assert(diskMapPtr != NULL, diskMapPtr);

    if (diskMapPtr != NULL)
    {
        /*
         * First, look to see if the user-specified (CCBE) diskMap is valid.
         * If it is, then use it.  Otherwise, look for the 'best' disks
         * to be used for the read.  If that fails, then zero out the diskMap
         * and return an ERROR.
         */
        if (readableDiskMapValid == TRUE)
        {
            /*
             * Copy the file system diskMap into the structure pointed to
             * by the diskMapPtr variable.
             */
            memcpy(diskMapPtr, &readableDiskMap, sizeof(*diskMapPtr));
            returnCode = GOOD;
        }
        else
        {
            memset(diskMapPtr, -1, sizeof(*diskMapPtr));
        }
    }

    return (returnCode);
}


/**
******************************************************************************
**
**  @brief      FIO_GetWritableDiskMap - Get copy of writable disk map
**
**  @param      diskMapPtr - Pointer to where the map is to be copied
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
UINT32 FIO_GetWritableDiskMap(FIO_DISK_MAP *diskMapPtr)
{
    UINT32      returnCode = ERROR;

    ccb_assert(diskMapPtr != NULL, diskMapPtr);

    if (diskMapPtr != NULL)
    {
        if (writableDiskMapValid == TRUE)
        {
            /*
             * Copy the file system diskMap into the structure pointed to
             * by the diskMapPtr variable.
             */
            memcpy(diskMapPtr, &writableDiskMap, sizeof(*diskMapPtr));
            returnCode = GOOD;
        }
        else
        {
            memset(diskMapPtr, -1, sizeof(*diskMapPtr));
        }
    }

    return (returnCode);
}


/**
******************************************************************************
**
**  @brief      FIO_SetReadableDiskMap - Set readable disk map
**
**  @param      newDiskMapPtr - Pointer to where the map is copied from
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
UINT32 FIO_SetReadableDiskMap(FIO_DISK_MAP *newDiskMapPtr)
{
    UINT32      returnCode = ERROR;

    ccb_assert(newDiskMapPtr != NULL, newDiskMapPtr);

    if (newDiskMapPtr != NULL)
    {
        /*
         * Only save the diskMap if BE is 'ready'.  The BE will return an
         * arbitrary diskMap until it's reached the full define phase, and we
         * want to avoid saving an inaccurate diskMap at all cost.  Doing so
         * might cause the elections to fail on all controllers (fragmentation).
         */
        if (PowerUpBEReady() == true)
        {
            /*
             * Copy the structure pointed to the diskMapPtr variable into
             * the file system diskMap.
             */
            memcpy(&readableDiskMap, newDiskMapPtr, sizeof(readableDiskMap));
            readableDiskMapValid = TRUE;
            returnCode = GOOD;
        }
        else
        {
            readableDiskMapValid = FALSE;
        }
    }

    return (returnCode);
}


/**
******************************************************************************
**
**  @brief      FIO_SetWritableDiskMap - Set writable disk map
**
**  @param      newDiskMapPtr - Pointer to where the map is copied from
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
UINT32 FIO_SetWritableDiskMap(FIO_DISK_MAP *newDiskMapPtr)
{
    UINT32      returnCode = ERROR;

    ccb_assert(newDiskMapPtr != NULL, newDiskMapPtr);

    if (newDiskMapPtr != NULL)
    {
        /*
         * Copy the structure pointed to the diskMapPtr variable into
         * the file system diskMap.
         */
        memcpy(&writableDiskMap, newDiskMapPtr, sizeof(writableDiskMap));
        writableDiskMapValid = TRUE;
        returnCode = GOOD;
    }

    return (returnCode);
}


/**
******************************************************************************
**
**  @brief      FIO_GetNumDiskMapDisks - Get Number of disks in a map
**
**  @param      diskMapPtr - Pointer to where the map is to be copied
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
UINT32 FIO_GetNumDiskMapDisks(FIO_DISK_MAP *diskMapPtr)
{
    UINT32      numDisks = 0;
    UINT8      *pMap;
    UINT32      i,
                j;

    ccb_assert(diskMapPtr != NULL, diskMapPtr);

    if (diskMapPtr != NULL)
    {
        pMap = (UINT8 *)diskMapPtr;

        for (i = 0; i < sizeof(*diskMapPtr); i += 8)
        {
            for (j = 0; j < 8; ++j)
            {
                if (pMap[i] & (1 << j))
                {
                    ++numDisks;
                }
            }
        }
    }

    return (numDisks);
}


/**
******************************************************************************
**
**  @brief      FIO_ResetReadableDiskMap - Reset readable disk map
**
**  @param      none
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
UINT32 FIO_ResetReadableDiskMap(void)
{
    readableDiskMapValid = FALSE;
    memset(&readableDiskMap, 0, sizeof(readableDiskMap));

    return (GOOD);
}


/**
******************************************************************************
**
**  @brief      FIO_ResetWritableDiskMap - Reset writable disk map
**
**  @param      none
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
UINT32 FIO_ResetWritableDiskMap(void)
{
    writableDiskMapValid = FALSE;
    memset(&writableDiskMap, 0, sizeof(writableDiskMap));

    return (GOOD);
}


/**
******************************************************************************
**
**  @brief      FIO_MapDump - Dump diskMap entry to display
**
**              Dumps a diskMap item to the debug output
**
**  @param      diskMapPtr - Pointer to the diskMap data
**
**  @return     none
**
******************************************************************************
**/
void FIO_MapDump(FIO_DISK_MAP *diskMapPtr)
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
        /*
         * Find the maximum configured pDisk ID
         */
        maxDisk = dimension_of(*diskMapPtr);

        while ((maxDisk > 0) && ((*diskMapPtr)[maxDisk - 1] == 0))
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
            swap = (((*diskMapPtr)[mapIndex] & 0x80) >> 7) |
                (((*diskMapPtr)[mapIndex] & 0x40) >> 5) |
                (((*diskMapPtr)[mapIndex] & 0x20) >> 3) |
                (((*diskMapPtr)[mapIndex] & 0x10) >> 1) |
                (((*diskMapPtr)[mapIndex] & 0x08) << 1) |
                (((*diskMapPtr)[mapIndex] & 0x04) << 3) |
                (((*diskMapPtr)[mapIndex] & 0x02) << 5) |
                (((*diskMapPtr)[mapIndex] & 0x01) << 7);

            charval(swap >> 4, &mapText[(mapIndex * 2)]);
            charval(swap & 0x0F, &mapText[(mapIndex * 2) + 1]);
        }

        mapText[mapIndex * 2] = '\0';
        dprintf(DPRINTF_ELECTION, "MD: %s\n", mapText);
    }
    else
    {
        dprintf(DPRINTF_ELECTION, "MD: diskMapPtr is NULL\n");
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
