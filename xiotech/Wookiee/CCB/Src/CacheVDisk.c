/* $Id: CacheVDisk.c 157423 2011-08-02 20:30:41Z m4 $ */
/*===========================================================================
** FILE NAME:       CacheVDisk.c
** MODULE TITLE:    CCB Cache - VDisk
**
** DESCRIPTION:     Cached data for Virtual Disks
**
** Copyright (c) 2002-2010  Xiotech Corporation. All rights reserved.
**==========================================================================*/

#include "CacheVDisk.h"
#include "CacheManager.h"
#include "CacheRaid.h"
#include "cps_init.h"
#include "debug_files.h"
#include "globalOptions.h"
#include "LOG_Defs.h"
#include "LargeArrays.h"
#include "MR_Defs.h"
#include "PacketInterface.h"
#include "PI_Stats.h"
#include "PI_Utils.h"
#include "vdd.h"
#include "X1_Structs.h"

/*****************************************************************************
 ** Private variables
 *****************************************************************************/
#ifndef NO_VCD_CACHE
static UINT8 *cacheVDiskCacheInfoAddr[MAX_VIRTUAL_DISKS];
static UINT8 cacheVDiskCacheInfo[CACHE_SIZE_VDISK_CACHE_INFO];
static UINT16 cacheVDiskCacheInfoCount = 0;
#endif  /* NO_VCD_CACHE */
#ifndef NO_VDISK_CACHE
static UINT16 cacheVirtualDisksCount = 0;
#endif  /* NO_VDISK_CACHE */

/*****************************************************************************
 ** Public variables - externed in the header file
 *****************************************************************************/
#ifndef NO_VDISK_CACHE
UINT8       cacheVDiskCopyMap[CACHE_SIZE_VDISK_MAP];
UINT8       cacheVDiskMirrorMap[CACHE_SIZE_VDISK_MAP];
UINT8       cacheVirtualDiskMap[CACHE_SIZE_VDISK_MAP];
UINT8      *cacheVirtualDiskAddr[MAX_VIRTUAL_DISKS];
UINT8      *cacheVirtualDisks = NULL;
UINT8      *cacheTempVirtualDisks = NULL;
#endif  /* NO_VDISK_CACHE */

/*****************************************************************************
 ** Code Start
 *****************************************************************************/

/*----------------------------------------------------------------------------
** Function:    VirtualDisksCount
**
** Inputs:      NONE
**
** Returns:     Count of the VDisks in the system using the cached value.
**
** Warning:     THIS FUNCTION DOES NOT CHANGE THE CACHE STATE FOR THIS
**              DEVICE TYPE. THIS FUNCTION IS NORMALLY USED IN CONJUNCTION
**              WITH "VirtualDisksGet" AND IF THAT IS THE CASE THE USER
**              ***MUST*** SET THE CACHE STATE TO "IN USE" TO ENSURE THE
**              VALUES DO NOT CHANGE. IF THIS IS DONE, REMEMBER TO SET
**              THE STATE BACK TO "OK" SO THAT THINGS WORK PROPERLY.
**--------------------------------------------------------------------------*/
#ifndef NO_VDISK_CACHE
UINT16 VirtualDisksCount(void)
{
    return cacheVirtualDisksCount;
}
#else   /* NO_VDISK_CACHE */
UINT16 VirtualDisksCount(void)
{
    struct DMC         *entry = DMC_CCB + CCB_DMC_vdiskcache;
    struct DMC_vdisk_structure *rs;

    /* Validate entry has data. */
    if (entry->copy_length == 0)
    {
        return 0;
    }

    rs = entry->copy_addr;                  /* Void stored into any pointer is ok. */
    return rs->cachevdiskCount_DMC;
}
#endif  /* NO_VDISK_CACHE */

/*----------------------------------------------------------------------------
** Function:    CalcSizeVDisksCached
**
** Description: Calculate the size (in bytes) of the buffer required to
**              hold all the information for all the virutal disks.
**
**--------------------------------------------------------------------------*/
#ifndef NO_VDISK_CACHE
INT32 CalcSizeVDisksCached(void)
{
    UINT16      i;
    UINT32      vdiskCacheSize = 0;

    /*
     * Walk the list of addresses to cache entries, pulling out the
     * length of each valid entry.
     */
    for (i = 0; i < MAX_VIRTUAL_DISKS; i++)
    {
        if (cacheVirtualDiskAddr[i] != NULL)
        {
            vdiskCacheSize += ((MRGETVINFO_RSP *)(cacheVirtualDiskAddr[i]))->header.len;
        }
    }
    return (vdiskCacheSize);
}
#else   /* NO_VDISK_CACHE */
INT32 CalcSizeVDisksCached(void)
{
    struct DMC         *entry = DMC_CCB + CCB_DMC_vdiskcache;
    struct DMC_vdisk_structure *rs;
    UINT16              i;
    UINT32              vdiskCacheSize = 0;

    rs = entry->copy_addr;                  /* Void stored into any pointer is ok. */
    for (i = 0; i < MAX_VIRTUAL_DISKS; i++)
    {
        if (rs->cacheVDiskAddr_DMC[i] != NULL)
        {
            vdiskCacheSize += ((MRGETVINFO_RSP *)(rs->cacheVDiskAddr_DMC[i]))->header.len;
        }
    }
    return vdiskCacheSize;
}
#endif  /* NO_VDISK_CACHE */

#ifndef NO_VDISK_CACHE
/*----------------------------------------------------------------------------
** Function:    VirtualDisksGet
**
** Returns:     NONE
**
** Warning:     THIS FUNCTION DOES NOT CHANGE THE CACHE STATE FOR THIS
**              DEVICE TYPE. THIS FUNCTION IS NORMALLY USED IN CONJUNCTION
**              WITH "VirtualDisksCount" AND IF THAT IS THE CASE THE USER
**              ***MUST*** SET THE CACHE STATE TO "IN USE" TO ENSURE THE
**              VALUES DO NOT CHANGE. IF THIS IS DONE, REMEMBER TO SET
**              THE STATE BACK TO "OK" SO THAT THINGS WORK PROPERLY.
**--------------------------------------------------------------------------*/
void VirtualDisksGet(void *buf)
{
    memcpy(buf, cacheVirtualDisks, CalcSizeVDisksCached());
}
#endif  /* NO_VDISK_CACHE */

/*----------------------------------------------------------------------------
** Function:    GetTenVDisk
**
** Description: Get Statistics on top VDisks usage for qd, mbps, iops.
**
**--------------------------------------------------------------------------*/
void GetTenVDisk(UINT16 *max_vdisk4qd, UINT32 *max_v_qd,
                 UINT16 *max_vdisk4mbps, UINT32 *max_v_mbps,
                 UINT16 *max_vdisk4iops, UINT32 *max_v_iops)
{
    UINT8      *pCacheEntry;
    UINT16      vdisk_count = VirtualDisksCount();
    UINT16      count;
#ifdef NO_VDISK_CACHE
    struct DMC *entry = DMC_CCB + CCB_DMC_vdiskcache;
#endif  /* NO_VDISK_CACHE */
    UINT32      mbps;

    *max_vdisk4qd = 0xffff;
    *max_v_qd = 0;
    *max_vdisk4mbps = 0xffff;
    *max_v_mbps = 0;
    *max_vdisk4iops = 0xffff;
    *max_v_iops = 0;

    /*
     * Wait for good cache status before continuing. Set the flag
     * to indicate that the cache is updating.
     */
#ifndef NO_VDISK_CACHE
    CacheStateWaitUpdating(cacheVirtualDisksState);
    CacheStateSetInUse(cacheVirtualDisksState);
    pCacheEntry = cacheVirtualDisks;
#else   /* NO_VDISK_CACHE */
    /* Get memory lock and wait if busy. */
    Wait_DMC_Lock(entry);
    pCacheEntry = (UINT8 *)&cacheVDiskBuffer_DMC.cacheVDiskBuffer_DMC;
#endif  /* NO_VDISK_CACHE */

    /* Fill out the maps. */
    for (count = 0; count < vdisk_count; count++)
    {
        MRGETVINFO_RSP *vdinfo = (MRGETVINFO_RSP *)pCacheEntry;
        if (vdinfo->qd >= *max_v_qd)
        {
            *max_v_qd = vdinfo->qd;
            *max_vdisk4qd = vdinfo->vid;
        }
        if (vdinfo->rps >= *max_v_iops)
        {
            *max_v_iops = vdinfo->rps;
            *max_vdisk4iops = vdinfo->vid;
        }
        /* Calculate mbps from: number of sectors * number of sectors */
        mbps = (vdinfo->rps * vdinfo->avgSC * 100.0) / 2048;
        if (mbps >= *max_v_mbps)
        {
            *max_v_mbps = mbps;
            *max_vdisk4mbps = vdinfo->vid;
        }
        pCacheEntry += vdinfo->header.len;
    }

    /* Done with lock. */
#ifndef NO_VDISK_CACHE
    CacheStateSetNotInUse(cacheVirtualDisksState);
#else   /* NO_VDISK_CACHE */
    Free_DMC_Lock(entry);
#endif  /* NO_VDISK_CACHE */
}   /* End of GetTenVDisk */

#ifndef NO_VDISK_CACHE
/*----------------------------------------------------------------------------
** Function:    RefreshVirtualDisks
**
** Description: Get info on all Virtual Disks. The data is cached and the
**              addressof each VDisk's entry is also cached.
**
** Inputs:      NONE
**
** Returns:     PI_GOOD, PI_ERROR
**--------------------------------------------------------------------------*/
INT32 RefreshVirtualDisks(void)
{
    INT32       rc = PI_GOOD;
    MR_LIST_RSP *ptrList = NULL;
    MRGETVINFO_REQ *pMRPInPkt = NULL;
    UINT8      *pCacheEntry;
    UINT32      vDisksCopySize = 0;
    UINT32      outPktSize = 0;
    UINT16      count = 0;
    MRGETVINFO_RSP *pTmpPtr = NULL;

    /*
     * dprintf(DPRINTF_CACHEMGR, "RefreshVirtualDisks - ENTER (0x%x)\n", K_timel);
     */
    if (PowerUpBEReady())
    {
        /*
         * Wait for good cache status before continuing. Set the flag
         * to indicate that the cache is updating.
         */
        CacheStateWaitOkToUpdate(cacheVirtualDisksState);
        CacheStateSetUpdating(cacheVirtualDisksState);
        /*
         * Get the list of objects. Always start at the beginning and return
         * the entire list.
         */
        ptrList = CacheGetList(MRGETVLIST);

        /* If we could not get the list, signal an error */
        if (ptrList == NULL)
        {
            rc = PI_ERROR;
        }
        else
        {
            /*
             * Allocate the input packet used to retrieve the
             * information from the PROC.
             */
            pMRPInPkt = MallocWC(sizeof(*pMRPInPkt));

            outPktSize = sizeof(*pTmpPtr) + (MAX_RAIDS * sizeof(UINT16));

            /*
             * Get VDisk Info for each VDisk in the list. Place the info
             * in the cache.
             */
            for (count = 0; count < ptrList->ndevs; count++)
            {
                /*
                 * Create a map of existing VDisks. Get the next ID from the list,
                 * then do the math to set the bit for this ID in the VDisk map.
                 * id/8 gets to the correct byte, (1 << (id & 7)) gets the bit
                 * within the byte.
                 */
                pMRPInPkt->id = ptrList->list[count];

                pTmpPtr = (MRGETVINFO_RSP *)(cacheTempVirtualDisks + vDisksCopySize);

                /* Try forever to get this MRP through. */
                rc = PI_ExecuteMRP(pMRPInPkt, sizeof(*pMRPInPkt), MRGETVINFO,
                                   pTmpPtr, outPktSize,
                                   CACHE_MAX_REFRESH_MRP_TIMEOUT,
                                   PI_COMMAND_RECORD_TIMEOUT_ACTION_OUTPUT_NONE);
                if (rc == PI_GOOD)
                {
                    vDisksCopySize += pTmpPtr->header.len;
                }
                else
                {
                    break;
                    /*
                     * Some sort of error occurred. cacheVirtualDiskAddr for
                     * this entry is NULL as initialized.
                     */
                }
            }

            if (rc == PI_GOOD)
            {

                /*
                 * Clear out the cache buffers. Clearing cacheVirtualDiskAddr
                 * should have the effect of setting all the pointers to NULL. If
                 * this is done cacheVirtualDisks does NOT have to be cleared.
                 * Set up a pointer to the cache entry and initialize the array
                 * of pointers to each cache entry.
                 */
                memset(cacheVirtualDiskMap, 0x00, CACHE_SIZE_VDISK_MAP);
                memset(cacheVDiskCopyMap, 0x00, CACHE_SIZE_VDISK_MAP);
                memset(cacheVDiskMirrorMap, 0x00, CACHE_SIZE_VDISK_MAP);
                memset(cacheVirtualDiskAddr, 0x00, CACHE_SIZE_VDISK_ADDR);

                /* Set the VDisk count. */
                cacheVirtualDisksCount = ptrList->ndevs;

                /* Swap the pointers. */
                SwapPointersVdisks();
                pCacheEntry = cacheVirtualDisks;

                /* Fill out the maps. */
                for (count = 0; count < ptrList->ndevs; count++)
                {
                    cacheVirtualDiskMap[ptrList->list[count] / 8] |= (1 << (ptrList->list[count] & 7));

                    if (((MRGETVINFO_RSP *)pCacheEntry)->mirror == VD_COPYTO ||
                        ((MRGETVINFO_RSP *)pCacheEntry)->mirror == VD_COPYUSERPAUSE ||
                        ((MRGETVINFO_RSP *)pCacheEntry)->mirror == VD_COPYAUTOPAUSE)
                    {
                        cacheVDiskCopyMap[ptrList->list[count] / 8] |= (1 << (ptrList->list[count] & 7));
                    }

                    if (((MRGETVINFO_RSP *)pCacheEntry)->mirror == VD_COPYMIRROR)
                    {
                        cacheVDiskMirrorMap[ptrList->list[count] / 8] |= (1 << (ptrList->list[count] & 7));
                    }

                    /*
                     * Save the address of this cache entry in the lookup
                     * table cacheVirtualDiskAddr[]. Advance the cache
                     * pointer to the place where the next entry will go.
                     */
                    cacheVirtualDiskAddr[ptrList->list[count]] = pCacheEntry;
                    pCacheEntry += ((MRGETVINFO_RSP *)pCacheEntry)->header.len;
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

        /* make the vdisks cache valid */
        PI_MakeCacheValid(1 << PI_CACHE_INVALIDATE_VDISK);

        /* Done updating cache - set good status.  */
        CacheStateSetUpdateDone(cacheVirtualDisksState);

    }
    else
    {
        rc = PI_ERROR;
    }

    /*
     * dprintf(DPRINTF_CACHEMGR, "RefreshVirtualDisks - EXIT (0x%x)\n", K_timel);
     */

    return rc;
}
#endif  /* NO_VDISK_CACHE */

/*----------------------------------------------------------------------------
** Function:    RefreshVDiskCacheInfo
**
** Description: Get cache info on all Virtual Disks. The data is cached
**              and the address of each VDisk's entry is also cached.
**
** Inputs:      NONE
**
** Returns:     PI_GOOD, PI_ERROR
**--------------------------------------------------------------------------*/
INT32 RefreshVDiskCacheInfo(void)
{
    PI_STATS_CACHE_DEVICES_RSP *pRsp;
    UINT8      *pCacheEntry;
    INT32       rc = PI_GOOD;
    UINT16      i = 0;

    dprintf(DPRINTF_CACHE_REFRESH, "RefreshVDiskCacheInfo - ENTER\n");

    if (PowerUpComplete())
    {
        /* Get cache device info for all VDisks */
        pRsp = StatsCacheDevices();

        /* Proceed if data is available */
        if (pRsp != NULL)
        {
            /*
             * Wait for good cache status before continuing. Set the flag
             * to indicate that the cache is updating.
             */
            CacheStateWaitOkToUpdate(cacheVDiskCacheInfoState);
            CacheStateSetUpdating(cacheVDiskCacheInfoState);

            /*
             * Clear out the cache buffers. Clearing cacheVDiskCacheInfoAddr
             * should have the effect of setting all the pointers to NULL. If
             * this is done cacheVirtualDisks does NOT have to be cleared.
             * Set up a pointer to the cache entry and initialize the array
             * of pointers to each cache entry.
             */
            memset(cacheVDiskCacheInfoAddr, 0x00, CACHE_SIZE_VDISK_ADDR);

            /* Set the VDisk count. Initialize the cache pointer. */
            cacheVDiskCacheInfoCount = pRsp->count;
            pCacheEntry = cacheVDiskCacheInfo;

            /* Only a portion of the data for each device is cached. */
            for (i = 0; i < cacheVDiskCacheInfoCount; i++)
            {
                memcpy(pCacheEntry, &(pRsp->cacheDev[i].vid), sizeof(CACHE_VDISK_CACHE_INFO));

                /*
                 * Save the address of this cache entry in the lookup
                 * table cacheVDiskCacheInfoAddr[]. Advance the cache
                 * pointer to the place where the next entry will go.
                 */
                cacheVDiskCacheInfoAddr[pRsp->cacheDev[i].vid] = pCacheEntry;
                pCacheEntry += sizeof(CACHE_VDISK_CACHE_INFO);
            }

            /* Done updating cache - set good status. */
            CacheStateSetUpdateDone(cacheVDiskCacheInfoState);

            Free(pRsp);
        }
        else
        {
            rc = PI_ERROR;
        }
    }

    dprintf(DPRINTF_CACHE_REFRESH, "RefreshVDiskCacheInfo - &cacheVDiskCacheInfoAddr=0x%08X  &cacheVDiskCacheInfo=0x%08X\n",
            (UINT32)cacheVDiskCacheInfoAddr, (UINT32)cacheVDiskCacheInfo);

    return rc;
}

/*----------------------------------------------------------------------------
** Function:    CachedVDisks()
**
** Description: Get the information for all the vdisks.
**
** Inputs:      NONE
**
** Returns:     Pointer to a targets response packet or NULL if they
**              could not be retrieved.
**
** WARNING:     The pointer to the response packet needs to be freed by the
**              caller or it will be leaked.
**--------------------------------------------------------------------------*/
PI_VDISKS_RSP *CachedVDisks(void)
{
    PI_VDISKS_RSP *pOutData;
    UINT32      size;
    UINT32      vdisk_info_size;
#ifdef NO_VDISK_CACHE
    struct DMC *entry = DMC_CCB + CCB_DMC_vdiskcache;
    UINT32      recheck_vdisk_info_size;
#endif  /* NO_VDISK_CACHE */

    /*
     * Wait until the VDisks cache is not in
     * the process of being updated. Once it is in
     * that state make sure it is set to in use so
     * a update doesn't start while it is being
     * used.
     */
#ifndef NO_VDISK_CACHE
    CacheStateWaitUpdating(cacheVirtualDisksState);
    CacheStateSetInUse(cacheVirtualDisksState);
#else   /* NO_VDISK_CACHE */
    /* Get memory lock and wait if busy. */
    Wait_DMC_Lock(entry);
#endif  /* NO_VDISK_CACHE */

    /*
     * Calculate the size of the output packet. This will be the size
     * of the VDisks (multiple devices) response packet plus the
     * size of a VDisk Info (single device) response packet for each
     * device.
     */
    vdisk_info_size = CalcSizeVDisksCached();
#ifdef NO_VDISK_CACHE
  retry_get:
    /* Done with lock. */
    Free_DMC_Lock(entry);
#endif  /* NO_VDISK_CACHE */
    size = sizeof(*pOutData) + vdisk_info_size;

    pOutData = MallocWC(size);

#ifdef NO_VDISK_CACHE
    /* Get memory lock and wait if busy. */
    Wait_DMC_Lock(entry);

    recheck_vdisk_info_size = CalcSizeVDisksCached();
    if (recheck_vdisk_info_size != vdisk_info_size)
    {
        Free(pOutData);                    /* Free does not task switch. */
        vdisk_info_size = recheck_vdisk_info_size;
        goto retry_get;
    }
#endif  /* NO_VDISK_CACHE */

    /*
     * Copy the number of devices into the output packet along
     * with the data for each VDisk.
     */
    pOutData->count = VirtualDisksCount();

#ifndef NO_VDISK_CACHE
    VirtualDisksGet(&pOutData->vdisks);
    CacheStateSetNotInUse(cacheVirtualDisksState);
#else   /* NO_VDISK_CACHE */
    memcpy(&pOutData->vdisks, &cacheVDiskBuffer_DMC.cacheVDiskBuffer_DMC, vdisk_info_size);

    /* Done with lock. */
    Free_DMC_Lock(entry);
#endif  /* NO_VDISK_CACHE */

    return pOutData;
}

#ifndef NO_VDISK_CACHE
/*----------------------------------------------------------------------------
** Function:    PI_MakeVDisksCacheDirty
**
** Description: Make the vdisks cache dirty
**
** Inputs:      None
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_MakeVDisksCacheDirty(void)
{
    dprintf(DPRINTF_DEFAULT, "%s Enter\n", __FUNCTION__);

    /*
     * Wait until the Virtual Disks cache is not in
     * the process of being updated. Once it is in
     * that state make sure it is set to in use so
     * a update doesn't start while it is being
     * used.
     */
    CacheStateWaitUpdating(cacheVirtualDisksState);
    CacheStateSetInUse(cacheVirtualDisksState);

    /*
     * make the vdisk cache dirty
     */
    PI_MakeCacheDirty(1 << PI_CACHE_INVALIDATE_VDISK);

    CacheStateSetNotInUse(cacheVirtualDisksState);
    return PI_GOOD;
}
#endif  /* NO_VDISK_CACHE */

#ifndef NO_RAID_CACHE
/*----------------------------------------------------------------------------
** Function:    PI_MakeRaidsCacheDirty
**
** Description: Make the raid cache dirty
**
** Inputs:      None
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_MakeRaidsCacheDirty(void)
{
    dprintf(DPRINTF_DEFAULT, "%s Enter\n", __FUNCTION__);

    /*
     * Wait until the Virtual Disks cache is not in
     * the process of being updated. Once it is in
     * that state make sure it is set to in use so
     * a update doesn't start while it is being
     * used.
     */
    CacheStateWaitUpdating(cacheRaidsState);
    CacheStateSetInUse(cacheRaidsState);

    /*
     * make the raids cache dirty
     */
    PI_MakeCacheDirty(1 << PI_CACHE_INVALIDATE_RAID);

    CacheStateSetNotInUse(cacheRaidsState);
    return PI_GOOD;
}
#endif  /* NO_RAID_CACHE */

/***
 ** Modelines:
 ** Local Variables:
 ** tab-width: 4
 ** indent-tabs-mode: nil
 ** End:
 ** vi:sw=4 ts=4 expandtab
 **/
