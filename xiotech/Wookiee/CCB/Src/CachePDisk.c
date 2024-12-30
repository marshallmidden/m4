/* $Id: CachePDisk.c 160656 2011-05-10 08:47:57Z marshall_midden $ */
/**
******************************************************************************
**
**  @file   CachePDisk.c
**
**  @brief  Cached data for Physical Disks
**
**  Copyright (c) 2002-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/

#include "LOG_Defs.h"
#include "CachePDisk.h"
#include "CacheManager.h"
#include "cps_init.h"
#include "debug_files.h"
#include "globalOptions.h"
#include "LargeArrays.h"
#include "misc.h"
#include "MR_Defs.h"
#include "PI_Utils.h"
#include "serial_num.h"
#include "X1_Structs.h"

/*****************************************************************************
 ** Local variables
 *****************************************************************************/
static UINT16 cachePDiskLocations[MAX_LOCATION_BAYS][MAX_LOCATION_DISKS_PER_BAY];
static UINT16 cachePhysicalDisksCount = 0;
static UINT8 *cacheGoodPdiskAddr[MAX_PHYSICAL_DISKS];
static CACHE_GOOD_DEVICE cacheGoodPdisk[MAX_PHYSICAL_DISKS];

/*****************************************************************************
 ** Public variables - externed in the header file
 *****************************************************************************/
UINT8      *cachePhysicalDiskAddr[MAX_PHYSICAL_DISKS];
UINT8       cachePDiskMap[CACHE_SIZE_PDISK_MAP];        /* This is a bit map */
UINT8       cachePDiskFailMap[CACHE_SIZE_PDISK_MAP];    /* This is a bit map */
UINT8       cachePDiskRebuildMap[CACHE_SIZE_PDISK_MAP]; /* This is a bit map */
UINT8       cachePDiskPaths[CACHE_SIZE_PDISK_PATHS];

UINT8      *cachePhysicalDisks = NULL;
UINT8      *cacheTempPhysicalDisks = NULL;

/*****************************************************************************
 ** Private function prototypes
 *****************************************************************************/
static void UpdateGoodPdisks(void);

/*****************************************************************************
 ** Code Start
 *****************************************************************************/

/*----------------------------------------------------------------------------
** Function:    PhysicalDisksCount
**
** Inputs:      NONE
**
** Returns:     Count of the physical disks in the system using the cached
**              value.
**
** Warning:     THIS FUNCTION DOES NOT CHANGE THE CACHE STATE FOR THIS
**              DEVICE TYPE.  THIS FUNCTION IS NORMALLY USED IN CONJUNCTION
**              WITH "PhysicalDisksGet" AND IF THAT IS THE CASE THE USER
**              ***MUST*** SET THE CACHE STATE TO "IN USE" TO ENSURE THE
**              VALUES DO NOT CHANGE.  IF THIS IS DONE, REMEMBER TO SET
**              THE STATE BACK TO "OK" SO THAT THINGS WORK PROPERLY.
**--------------------------------------------------------------------------*/
UINT16 PhysicalDisksCount(void)
{
    return cachePhysicalDisksCount;
}

/*----------------------------------------------------------------------------
** Function:    CalcSizePDisksCached
**
** Description: Calculate the size (in bytes) of the buffer required to
**              hold all the information for all the PDisks.
**
**--------------------------------------------------------------------------*/
INT32 CalcSizePDisksCached(void)
{
    UINT16      i;
    UINT32      pdiskCacheSize = 0;

    /*
     * Walk the list of addresses to cache entries, pulling out the
     * length of each valid entry.
     */
    for (i = 0; i < MAX_PHYSICAL_DISKS; i++)
    {
        if (cachePhysicalDiskAddr[i] != NULL)
        {
            pdiskCacheSize += ((MRGETPINFO_RSP *)(cachePhysicalDiskAddr[i]))->header.len;
        }
    }

    return (pdiskCacheSize);
}

/*----------------------------------------------------------------------------
** Function:    PhysicalDisksGet
**
** Returns:     NONE
**
** Warning:     THIS FUNCTION DOES NOT CHANGE THE CACHE STATE FOR THIS
**              DEVICE TYPE.  THIS FUNCTION IS NORMALLY USED IN CONJUNCTION
**              WITH "PhysicalDisksCount" AND IF THAT IS THE CASE THE USER
**              ***MUST*** SET THE CACHE STATE TO "IN USE" TO ENSURE THE
**              VALUES DO NOT CHANGE.  IF THIS IS DONE, REMEMBER TO SET
**              THE STATE BACK TO "OK" SO THAT THINGS WORK PROPERLY.
**--------------------------------------------------------------------------*/
void PhysicalDisksGet(void *buf)
{
    memcpy(buf, cachePhysicalDisks, CalcSizePDisksCached());
}

/*----------------------------------------------------------------------------
** Function:    GetPDiskInfoFromPid
**
** Description: Get the PDisk Info associated with the input pid
**
** Inputs:      UINT16  pid         pid of the PDisk
**
** Outputs:     MRGETPINFO_RSP*     pointer to PDisk Info for the requested
**                                  pid.  NULL if this WWN not found.
**
** Returns:     GOOD or ERROR
**
** WARNING:     The called must allocate memory to hold MRGETPINFO_RSP
**--------------------------------------------------------------------------*/
INT32 GetPDiskInfoFromPid(UINT16 pid, MRGETPINFO_RSP *pPInfoOut)
{
    MRGETPINFO_RSP *pPDiskInfoCache = NULL;
    INT32       rc = ERROR;

    /*
     * If the pid of the PDisk is out of range of the array then return
     * and error.
     */
    if (pid >= MAX_PHYSICAL_DISKS)
    {
        return (rc);
    }

    /*
     * Wait until the physical disks cache is not in the process of
     * being updated.  Once it is in that state make sure it is set
     * to in use so a update doesn't start while it is being used.
     */
    CacheStateWaitUpdating(cachePhysicalDisksState);
    CacheStateSetInUse(cachePhysicalDisksState);

    /*
     * Get a pointer from the PDisk cache to the info for the input PDisk.
     * Also cast a pointer for the response buffer.
     */
    pPDiskInfoCache = (MRGETPINFO_RSP *)cachePhysicalDiskAddr[pid];

    /*
     * If the pointer is valid continue.
     */
    if (pPDiskInfoCache != NULL)
    {
        memcpy(pPInfoOut, pPDiskInfoCache, sizeof(*pPInfoOut));
        rc = GOOD;
    }

    /*
     * No longer using the cache for this function...
     */
    CacheStateSetNotInUse(cachePhysicalDisksState);

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PhysicalDiskGet
**
** Returns:     NONE
**--------------------------------------------------------------------------*/
void PhysicalDiskGet(UINT16 pid, MRGETPINFO_RSP *pPhyDevOut)
{
    /*
     * Wait until the physical disks cache is not in
     * the process of being updated.  Once it is in
     * that state make sure it is set to in use so
     * a update doesn't start while it is being
     * used.
     */
    CacheStateWaitUpdating(cachePhysicalDisksState);
    CacheStateSetInUse(cachePhysicalDisksState);

    /*
     * Check that this is a valid pid.
     */
    if (cachePhysicalDiskAddr[pid])
    {
        memcpy(pPhyDevOut, cachePhysicalDiskAddr[pid], sizeof(*pPhyDevOut));
    }

    /*
     * No longer using the cache for this function...
     */
    CacheStateSetNotInUse(cachePhysicalDisksState);
}


/**
******************************************************************************
**
**  @brief  Get the PDisk Good Device Info associated with the input WWN
**
**  @param  wwn - WWN of the PDisk
**
**  @param  pGoodDeviceOut - Pointer to PDisk Info for the requested WWN
**
**  @return GOOD or ERROR
**
**  @attention  The called must allocate memory to hold MRGETPINFO_RSP
**
******************************************************************************
**/
INT32 GetPDiskGoodDeviceFromWwn(UINT64 wwn, CACHE_GOOD_DEVICE *pGoodDeviceOut)
{
    CACHE_GOOD_DEVICE *pPDiskGoodDeviceCache;
    CACHE_GOOD_DEVICE *pPDiskGoodDevice = NULL;
    INT32       rc = GOOD;
    UINT32      count = 0;

    /*
     * Get a pointer to the start of the PDisk cache.
     */
    pPDiskGoodDeviceCache = (CACHE_GOOD_DEVICE *)cacheGoodPdisk;

    /*
     * Wait until the physical disks cache is not in the process of
     * being updated.  Once it is in that state make sure it is set
     * to in use so a update doesn't start while it is being used.
     */
    CacheStateWaitUpdating(cachePhysicalDisksState);
    CacheStateSetInUse(cachePhysicalDisksState);

    /*
     * Loop through the PDisks and find the entry associated with
     * the input wwn.
     */
    for (count = 0; count < cachePhysicalDisksCount; ++count)
    {
        if (pPDiskGoodDeviceCache[count].wwn == wwn)
        {
            pPDiskGoodDevice = &pPDiskGoodDeviceCache[count];
            break;
        }
    }

    /*
     * If the requested entry was found, copy it into the pointer
     * passed by the called.
     */
    if (pPDiskGoodDevice != NULL)
    {
        memcpy(pGoodDeviceOut, pPDiskGoodDevice, sizeof(*pGoodDeviceOut));
        rc = GOOD;
    }
    else
    {
        rc = ERROR;
    }

    /*
     * No longer using the cache for this function...
     */
    CacheStateSetNotInUse(cachePhysicalDisksState);

    return (rc);
}


/**
******************************************************************************
**
**  @brief  Get PDisk Good Device Info associated with a WWN with no wait
**
**  @param  wwn - WWN of the PDisk
**  @param  pGoodDeviceOut - Pointer to PDisk Info for the requested WWN
**
**  @return GOOD or ERROR
**
**  @attention  The called must allocate memory to hold MRGETPINFO_RSP
**
******************************************************************************
**/
INT32 GetPDiskGoodDeviceFromWwnNOW(UINT64 wwn, CACHE_GOOD_DEVICE *pGoodDeviceOut)
{
    CACHE_GOOD_DEVICE *pPDiskGoodDeviceCache;
    UINT32      count;

    if (!pGoodDeviceOut || CacheStateUpdating(cachePhysicalDisksState))
    {
        return ERROR;
    }

    /*
     * Get a pointer to the start of the PDisk cache.
     */
    pPDiskGoodDeviceCache = (CACHE_GOOD_DEVICE *)cacheGoodPdisk;

    /*
     * Loop through the PDisks and find the entry associated with
     * the input wwn.
     */
    for (count = 0; count < cachePhysicalDisksCount; ++count)
    {
        if (pPDiskGoodDeviceCache[count].wwn == wwn)
        {
            *pGoodDeviceOut = pPDiskGoodDeviceCache[count];
            return GOOD;
        }
    }

    return ERROR;
}

#if defined(MODEL_7000) || defined(MODEL_4700)
/**
******************************************************************************
**
**  @brief  Retrieve good pdisk info for a particular Pdisk ID
**
**  @param  pid -   pdisk ID
**  @param  pGoodDeviceOut - Pointer to area to receive PDisk info
**
**  @return CACHE_GOOD_DEVICE*  pointer to good pdisk for the requested
**                                  pid.  NULL if this pid not found.
**
**  @return GOOD if the pdisk at "pid" was found, ERROR otherwise.
**
******************************************************************************
**/
INT32 GetPDiskGoodDeviceFromPid(UINT16 pid, CACHE_GOOD_DEVICE *pGoodDeviceOut)
{
    CACHE_GOOD_DEVICE *pGoodPiskCache;
    INT32       rc = ERROR;

    /*
     * Wait until the pdisk cache is not in the process of
     * being updated.  Once it is in that state make sure it is set
     * to in use so a update doesn't start while it is being used.
     */
    CacheStateWaitUpdating(cachePhysicalDisksState);
    CacheStateSetInUse(cachePhysicalDisksState);

    /*
     * Get a pointer to the CACHE_GOOD_DEVICE.
     */
    pGoodPiskCache = (CACHE_GOOD_DEVICE *)cacheGoodPdiskAddr[pid];

    /*
     * If the pointer is valid continue.
     */
    if (pGoodPiskCache != NULL)
    {
        /*
         * Copy only the main CACHE_GOOD_DEVICE structure.
         */
        memcpy(pGoodDeviceOut, pGoodPiskCache, sizeof(*pGoodDeviceOut));
        rc = GOOD;
    }

    /*
     * No longer using the cache for this function...
     */
    CacheStateSetNotInUse(cachePhysicalDisksState);

    return (rc);
}

/**
******************************************************************************
**
**  @brief  Retrieve good pdisk info for a particular Pdisk ID with no wait
**
**  @param  pid -   pdisk ID
**  @param  pGoodDeviceOut - Pointer to area to receive PDisk info
**
**  @return CACHE_GOOD_DEVICE*  pointer to good pdisk for the requested
**                                  pid.  NULL if this pid not found.
**
**  @return GOOD if the pdisk at "pid" was found, ERROR otherwise.
**
******************************************************************************
**/
INT32 GetPDiskGoodDeviceFromPidNOW(UINT16 pid, CACHE_GOOD_DEVICE *pGoodDeviceOut)
{
    CACHE_GOOD_DEVICE *pGoodPiskCache;

    if (!pGoodDeviceOut || CacheStateUpdating(cachePhysicalDisksState))
    {
        return ERROR;
    }

    /*
     * Get a pointer to the CACHE_GOOD_DEVICE.
     */
    pGoodPiskCache = (CACHE_GOOD_DEVICE *)cacheGoodPdiskAddr[pid];
    if (!pGoodPiskCache)
    {
        return ERROR;
    }

    *pGoodDeviceOut = *pGoodPiskCache;

    return GOOD;
}
#endif /* MODEL_7000 || MODEL_4700 */

/*----------------------------------------------------------------------------
** Function:    GetTenPDisk
**
** Description: Get Statistics on top PDisks usage for qd, mbps, iops.
**
**--------------------------------------------------------------------------*/
void GetTenPDisk(UINT16 *max_pdisk4qd, UINT32 *max_p_qd,
                 UINT16 *max_pdisk4mbps, UINT32 *max_p_mbps,
                 UINT16 *max_pdisk4iops, UINT32 *max_p_iops)
{
    UINT8      *pCacheEntry;
    UINT16      pdisk_count = PhysicalDisksCount();
    UINT16      count;
#ifdef NO_PDISK_CACHE
    struct DMC *entry = DMC_CCB + CCB_DMC_pdiskcache;
#endif  /* NO_PDISK_CACHE */
    UINT32      mbps;

    *max_pdisk4qd = 0xffff;
    *max_p_qd = 0;
    *max_pdisk4mbps = 0xffff;
    *max_p_mbps = 0;
    *max_pdisk4iops = 0xffff;
    *max_p_iops = 0;

    /*
     * Wait for good cache status before continuing. Set the flag
     * to indicate that the cache is updating.
     */
#ifndef NO_PDISK_CACHE
    CacheStateWaitUpdating(cachePhysicalDisksState);
    CacheStateSetInUse(cachePhysicalDisksState);
    pCacheEntry = cachePhysicalDisks;
#else   /* NO_PDISK_CACHE */
    /* Get memory lock and wait if busy. */
    Wait_DMC_Lock(entry);
    pCacheEntry = (UINT8 *)&cachePDiskBuffer_DMC.cachePDiskBuffer_DMC;
#endif  /* NO_PDISK_CACHE */

    /* Fill out the maps. */
    for (count = 0; count < pdisk_count; count++)
    {
        MRGETPINFO_RSP *pdinfo = (MRGETPINFO_RSP *)pCacheEntry;
        if (pdinfo->pdd.qd >= *max_p_qd)
        {
            *max_p_qd = pdinfo->pdd.qd;
            *max_pdisk4qd = pdinfo->pdd.pid;
        }
        if (pdinfo->pdd.rps >= *max_p_iops)
        {
            *max_p_iops = pdinfo->pdd.rps;
            *max_pdisk4iops = pdinfo->pdd.pid;
        }
        /* Calculate mbps from: number of sectors * number of sectors */
        mbps = (pdinfo->pdd.rps * pdinfo->pdd.avgSC * 100.0) / 2048;
        if (mbps >= *max_p_mbps)
        {
            *max_p_mbps = mbps;
            *max_pdisk4mbps = pdinfo->pdd.pid;
        }
        pCacheEntry += pdinfo->header.len;
    }

    /* Done with lock. */
#ifndef NO_PDISK_CACHE
    CacheStateSetNotInUse(cachePhysicalDisksState);
#else   /* NO_PDISK_CACHE */
    Free_DMC_Lock(entry);
#endif  /* NO_PDISK_CACHE */
}   /* End of GetTenPDisk */

/*----------------------------------------------------------------------------
** Function:    RefreshPhysicalDisks
**
** Inputs:      NONE
**
** Returns:     PI_GOOD, PI_ERROR
**--------------------------------------------------------------------------*/
INT32 RefreshPhysicalDisks(void)
{
    MR_LIST_RSP *ptrList = NULL;
    MRGETPINFO_REQ *pMRPInPkt = NULL;
    UINT8      *pCacheEntry;
    INT32       rc = PI_GOOD;
    UINT16      count = 0;
    MRGETPINFO_RSP *pTmpPtr = NULL;

    /*
     * dprintf(DPRINTF_CACHEMGR, "RefreshPhysicalDisks - ENTER\n");
     */
    /*
     * Wait for good cache status before continuing.  Set the flag
     * to indicate that the cache is updating.
     */
    CacheStateWaitOkToUpdate(cachePhysicalDisksState);
    CacheStateSetUpdating(cachePhysicalDisksState);

    /*
     * Get the list of objects.  Always start at the beginning and return
     * the entire list.
     */
    ptrList = CacheGetList(MRGETPLIST);

    /*
     * If we could not get the list, signal an error
     */
    if (ptrList == NULL)
    {
        dprintf(DPRINTF_DEFAULT, "RefreshPhysicalDisks - NULL pointer list returned\n");

        rc = PI_ERROR;
    }

    if (rc == PI_GOOD)
    {
        /*
         * Allocate the input packet used to retrieve the
         * information from the PROC.
         */
        pMRPInPkt = MallocWC(sizeof(*pMRPInPkt));

        /*
         * Get Physial Disk information for each bay in the list.  Place
         * the info in the cache.
         */
        for (count = 0; count < ptrList->ndevs; count++)
        {
            pMRPInPkt->id = ptrList->list[count];

            pTmpPtr = (MRGETPINFO_RSP *)(cacheTempPhysicalDisks + (sizeof(*pTmpPtr) * count));

            /*
             * Try forever to get this MRP through.
             */
            rc = PI_ExecuteMRP(pMRPInPkt, sizeof(*pMRPInPkt), MRGETPINFO,
                               pTmpPtr, sizeof(*pTmpPtr),
                               CACHE_MAX_REFRESH_MRP_TIMEOUT,
                               PI_COMMAND_RECORD_TIMEOUT_ACTION_OUTPUT_NONE);

            if (rc != PI_GOOD)
            {
                /*
                 * Some sort of error occurred.
                 */
                dprintf(DPRINTF_DEFAULT, "RefreshPhysicalDisks - Error returned from MRGETPINFO (0x%x)\n",
                        rc);
                break;
            }
        }

        if (rc == PI_GOOD)
        {

            /*
             * Clear out the cache information.
             */
            memset(cachePhysicalDiskAddr, 0, sizeof(cachePhysicalDiskAddr));
            memset(cachePDiskMap, 0, sizeof(cachePDiskMap));
            memset(cachePDiskFailMap, 0, sizeof(cachePDiskFailMap));
            memset(cachePDiskRebuildMap, 0, sizeof(cachePDiskRebuildMap));
            memset(cachePDiskLocations, 0xFF, sizeof(cachePDiskLocations));

            /*
             * Set the Physical Disk count.
             */
            cachePhysicalDisksCount = ptrList->ndevs;

            /*
             * Swap the pointers.
             */
            SwapPointersPdisks();
            pCacheEntry = cachePhysicalDisks;

            /*
             * Fill out the maps.
             */
            for (count = 0; count < cachePhysicalDisksCount; count++)
            {
                pTmpPtr = (MRGETPINFO_RSP *)pCacheEntry;

                /*
                 * Using the MRP data generate 3 maps.
                 */

                /* Map of existing PDisks */
                cachePDiskMap[ptrList->list[count] / 8] |= (1 << (ptrList->list[count] & 7));

                /* Map of failed PDisks */
                if (pTmpPtr->pdd.devStat == PD_INOP)
                {
                    cachePDiskFailMap[ptrList->list[count] / 8] |= (1 << (ptrList->list[count] & 7));
                }

                /* Map of rebuilding PDisks */
                if (pTmpPtr->pdd.miscStat == PD_M_REBUILDING)
                {
                    cachePDiskRebuildMap[ptrList->list[count] / 8] |= (1 << (ptrList->list[count] & 7));
                }

                /*
                 * If the SES and SLOT for this physical disk are valid
                 * and match the information the in the DNAME, fill in
                 * the location for this disk.
                 */
                if ((*(UINT32 *)(pTmpPtr->pdd.devName)) != 0 &&
                    pTmpPtr->pdd.devName[PD_DNAME_CSES] == pTmpPtr->pdd.ses &&
                    pTmpPtr->pdd.devName[PD_DNAME_CSLOT] == pTmpPtr->pdd.slot)
                {
                    cachePDiskLocations[pTmpPtr->pdd.devName[PD_DNAME_CSES]][pTmpPtr->pdd.devName[PD_DNAME_CSLOT]] = ptrList->list[count];
                }

                /*
                 * Save the address of this cache entry in the lookup
                 * table cachePhysicalDiskAddr[].  Advance the cache
                 * pointer to the place where the next entry will go.
                 * cachePhysicalDiskAddr[] is NOT packed.
                 */
                cachePhysicalDiskAddr[ptrList->list[count]] = pCacheEntry;
                pCacheEntry += sizeof(*pTmpPtr);
            }

            /*
             * Update the Good Pdisk Cache if discovery is complete.
             */
            if (DiscoveryComplete())
            {
                UpdateGoodPdisks();
            }

        }

        /*
         * Free the input packet now that all the information
         * has been gathered.
         */
        Free(pMRPInPkt);

        /*
         * Done with the list so it can now be freed.
         */
        Free(ptrList);
    }

    /*
     * We are done getting the Physical Disk Info for all PDisks.
     * Now get the back end path info. for all PDisks.  This only
     * requires one MRP call.
     */
    RefreshBEDevicePaths(PATH_PHYSICAL_DISK);

    /*
     * make the vdisks cache valid
     */
    PI_MakeCacheValid(1 << PI_CACHE_INVALIDATE_PDISK);

    /*
     * Done updating cache - set good status.
     */
    CacheStateSetUpdateDone(cachePhysicalDisksState);

    /*
     * dprintf(DPRINTF_CACHEMGR, "RefreshPhysicalDisks - EXIT\n");
     */

    return rc;
}


/*----------------------------------------------------------------------------
** Function:    UpdateGoodPdisks
**
** Description: Update the list of good disks.
**
** Inputs:      pid - pid to update.
**
** Returns:     NONE
**--------------------------------------------------------------------------*/
static void UpdateGoodPdisks(void)
{
    MRGETPINFO_RSP *pTmpPtr = NULL;
    CACHE_GOOD_DEVICE *pTmpDev = NULL;
    UINT32      count;
    UINT32      index2 = 0;

/*
  char dmy[32];
*/
    /*
     * Get a pointer from the PDisk cache to the info for the input PDisk.
     * Also cast a pointer for the response buffer.
     */
    pTmpPtr = (MRGETPINFO_RSP *)cachePhysicalDisks;

/*
  dprintf(DPRINTF_DEFAULT, "UpdateGoodPdisk - Enter\n");
  WWNToString(pTmpPtr->pdd.wwn, dmy);
  dprintf(DPRINTF_DEFAULT, "UpdateGoodPdisk - wwn:      %s\n", dmy);
  memcpy(dmy, pTmpPtr->pdd.serial, sizeof(pTmpPtr->pdd.serial));
  dmy[sizeof(pTmpPtr->pdd.serial)] = '\0';
  dprintf(DPRINTF_DEFAULT, "UpdateGoodPdisk - serial    %s\n", dmy);
  dprintf(DPRINTF_DEFAULT, "UpdateGoodPdisk - pid:      %d\n", pTmpPtr->pdd.pid);
  dprintf(DPRINTF_DEFAULT, "UpdateGoodPdisk - ses       %d\n", pTmpPtr->pdd.ses);
  dprintf(DPRINTF_DEFAULT, "UpdateGoodPdisk - slot      %d\n", pTmpPtr->pdd.slot);
*/

    /*
     * Loop through the pdisk cache and update the good pdisk cache.
     */
    for (count = 0; count < cachePhysicalDisksCount; ++count, ++pTmpPtr)
    {
        /*
         * If we have a valid ses and slot continue...
         */
        if ((pTmpPtr->pdd.ses < MAX_DISK_BAYS) &&
            (pTmpPtr->pdd.slot < MAX_DISK_BAY_SLOTS))
        {

            /*
             * Initialize the index.
             */
            index2 = 0;

            /*
             * Get a pointer in our Good Pdisk list to the pid...
             */
            pTmpDev = (CACHE_GOOD_DEVICE *)cacheGoodPdiskAddr[pTmpPtr->pdd.pid];

            /*
             * If the wwn's do not match, we need to search for it...
             */
            if ((pTmpDev == NULL) || (pTmpDev->wwn != pTmpPtr->pdd.wwn))
            {
                /*
                 * Our wwn's did not match, see if we can find it in
                 * our good pdisk table.  If we can't, create a new
                 * entry for the good pdisk data.
                 */
                pTmpDev = (CACHE_GOOD_DEVICE *)cacheGoodPdisk;

                for (index2 = 0; index2 < MAX_PHYSICAL_DISKS; ++index2, ++pTmpDev)
                {
                    /*
                     * We found an open slot, hence the wwn is not in the
                     * good pdisk table.  We will create an entry for the
                     * good pdisk below.  Or found the wwn.  Set the address
                     * pointer to this pid.
                     */
                    if ((pTmpDev->wwn == 0) || (pTmpDev->wwn == pTmpPtr->pdd.wwn))
                    {
                        cacheGoodPdiskAddr[pTmpPtr->pdd.pid] = (UINT8 *)pTmpDev;
                        break;
                    }
                }
            }

            /*
             * If we found it or have an open slot, copy the information.
             */
            if (index2 < MAX_PHYSICAL_DISKS)
            {
                /*
                 * WWNToString(pTmpPtr->pdd.wwn, dmy);
                 * dprintf(DPRINTF_DEFAULT, "UpdateGoodPdisk - Adding %s cacheGoodPdisk\n", dmy);
                 */
                pTmpDev->wwn = pTmpPtr->pdd.wwn;
                pTmpDev->pid = pTmpPtr->pdd.pid;
                pTmpDev->ses = pTmpPtr->pdd.ses;
                pTmpDev->slot = pTmpPtr->pdd.slot;
                memcpy(pTmpDev->serial, pTmpPtr->pdd.serial, sizeof(pTmpDev->serial));
            }
        }

/*
  dprintf(DPRINTF_DEFAULT, "UpdateGoodPdisk - Exit\n");
  WWNToString(pTmpDev->wwn, dmy);
  dprintf(DPRINTF_DEFAULT, "UpdateGoodPdisk - wwn:      %s\n", dmy);
  memcpy(dmy, pTmpDev->ps_serial, sizeof(pTmpDev->ps_serial));
  dmy[sizeof(pTmpDev->ps_serial)] = '\0';
  dprintf(DPRINTF_DEFAULT, "UpdateGoodPdisk - serial    %s\n", dmy);
  dprintf(DPRINTF_DEFAULT, "UpdateGoodPdisk - pid:      %d\n", pTmpDev->pid);
  dprintf(DPRINTF_DEFAULT, "UpdateGoodPdisk - ses       %d\n", pTmpDev->ses);
  dprintf(DPRINTF_DEFAULT, "UpdateGoodPdisk - slot      %d\n", pTmpDev->slot);
*/
    }
}

/*----------------------------------------------------------------------------
** Function:    RemovePdiskFromGoodPdisks
**
** Description: Remove a pdisk from the Good Pdisk table indicated by wwn.
**
** Inputs:      wwn - wwn to remove.
**
** Returns:     NONE
**--------------------------------------------------------------------------*/
void RemovePdiskFromGoodPdisks(UINT64 wwn)
{
    CACHE_GOOD_DEVICE *pTmpDev = NULL;
    UINT32      count = 0;

    CacheStateWaitUpdating(cachePhysicalDisksState);
    CacheStateSetInUse(cachePhysicalDisksState);
    CacheStateSetUpdating(cachePhysicalDisksState);

    pTmpDev = (CACHE_GOOD_DEVICE *)cacheGoodPdisk;

    for (count = 0; count < MAX_PHYSICAL_DISKS; ++count, ++pTmpDev)
    {
        /*
         * We found an open slot, hence the wwn is not in the
         * good pdisk table.  We will create an entry for the
         * good pdisk below.  Or found the wwn.  Set the address
         * pointer to this pid.
         */
        if (pTmpDev->wwn == wwn)
        {
            pTmpDev->ses = 0xFFFF;
            pTmpDev->slot = 0xFF;
            break;
        }
    }

    /*
     * Done updating cache - set good status.
     * No longer using the cache for this function...
     */
    CacheStateSetUpdateDone(cachePhysicalDisksState);
    CacheStateSetNotInUse(cachePhysicalDisksState);
}


/*----------------------------------------------------------------------------
** Function:    PI_MakePDisksCacheDirty
**
** Description: Make the pdisks cache dirty
**
** Inputs:      None
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_MakePDisksCacheDirty(void)
{
    dprintf(DPRINTF_DEFAULT, "%s Enter\n", __FUNCTION__);

    /*
     * Wait until the Physical Disks cache is not in
     * the process of being updated.  Once it is in
     * that state make sure it is set to in use so
     * a update doesn't start while it is being
     * used.
     */
    CacheStateWaitUpdating(cachePhysicalDisksState);
    CacheStateSetInUse(cachePhysicalDisksState);

    /*
     * make the vdisk cache dirty
     */
    PI_MakeCacheDirty(1 << PI_CACHE_INVALIDATE_PDISK);

    CacheStateSetNotInUse(cachePhysicalDisksState);
    return PI_GOOD;
}


/***
 ** Modelines:
 ** Local Variables:
 ** tab-width: 4
 ** indent-tabs-mode: nil
 ** End:
 ** vi:sw=4 ts=4 expandtab
 **/
