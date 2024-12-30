/* $Id: CacheRaid.c 145021 2010-08-03 14:16:38Z m4 $ */
/*===========================================================================
** FILE NAME:       CacheRaid.c
** MODULE TITLE:    CCB Cache - RAIDs
**
** DESCRIPTION:     Cached data for RAIDs
**
** Copyright (c) 2002-2009  Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#include "CachePDisk.h"
#include "CacheRaid.h"
#include "CacheManager.h"
#include "CacheVDisk.h"
#include "cps_init.h"
#include "debug_files.h"
#include "LOG_Defs.h"
#include "LargeArrays.h"
#include "MR_Defs.h"
#include "PI_Utils.h"
#include "RL_PSD.h"
#include "X1_Structs.h"
#include "ddr.h"

/*****************************************************************************
 ** Private variables
 *****************************************************************************/
#ifndef NO_RAID_CACHE
static UINT16 cacheRaidsCount = 0;
#endif  /* NO_RAID_CACHE */

/*****************************************************************************
 ** Public variables - externed in the header file
 *****************************************************************************/
#ifndef NO_RAID_CACHE
/* A bit for each raid. */
UINT8       cacheRaidMap[CACHE_SIZE_RAID_MAP];
UINT8      *cacheRaidAddr[MAX_RAIDS];

UINT8      *cacheRaids = NULL;
UINT8      *cacheTempRaids = NULL;
#endif  /* NO_RAID_CACHE */

/*****************************************************************************
 ** Code Start
 *****************************************************************************/

/*----------------------------------------------------------------------------
** Function:    RaidsCount
**
** Inputs:      NONE
**
** Returns:     Count of the RAIDs in the system using the cached
**              value.
**
** Warning:     THIS FUNCTION DOES NOT CHANGE THE CACHE STATE FOR THIS
**              DEVICE TYPE.  THIS FUNCTION IS NORMALLY USED IN CONJUNCTION
**              WITH "RaidsGet" AND IF THAT IS THE CASE THE USER
**              ***MUST*** SET THE CACHE STATE TO "IN USE" TO ENSURE THE
**              VALUES DO NOT CHANGE.  IF THIS IS DONE, REMEMBER TO SET
**              THE STATE BACK TO "OK" SO THAT THINGS WORK PROPERLY.
**--------------------------------------------------------------------------*/
#ifndef NO_RAID_CACHE
UINT16 RaidsCount(void)
{
    return cacheRaidsCount;
}
#else   /* NO_RAID_CACHE */
UINT16 RaidsCount(void)
{
    struct DMC         *entry = DMC_CCB + CCB_DMC_raidcache;
    struct DMC_raid_structure *rs;

    /* Validate entry has data. */
    if (entry->copy_length == 0)
    {
        return 0;
    }

    rs = entry->copy_addr;                  /* Void stored into any pointer is ok. */
    return rs->cacheRaidCount_DMC;
}
#endif  /* NO_RAID_CACHE */

/*----------------------------------------------------------------------------
** Function:    CalcSizeRaidsCached
**
** Description: Calculate the size (in bytes) of the buffer required to
**              hold all the information for all the RAIDs.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
#ifndef NO_RAID_CACHE
INT32 CalcSizeRaidsCached(void)
{
    UINT16      i;
    UINT32      raidCacheSize = 0;

    /*
     * Walk the list of addresses to cache entries, pulling out the
     * length of each valid entry.
     */
    for (i = 0; i < MAX_RAIDS; i++)
    {
        if (cacheRaidAddr[i] != NULL)
        {
            raidCacheSize += ((MRGETRINFO_RSP *)(cacheRaidAddr[i]))->header.len;
        }
    }
    return raidCacheSize;
}
#else   /* NO_RAID_CACHE */
INT32 CalcSizeRaidsCached(void)
{
    struct DMC         *entry = DMC_CCB + CCB_DMC_raidcache;
    struct DMC_raid_structure *rs;
    UINT16              i;
    UINT32              raidCacheSize;

    /* Validate entry has data. */
    if (entry->copy_length == 0)
    {
        return 0;
    }

    rs = entry->copy_addr;                  /* Void stored into any pointer is ok. */
    for (i = 0, raidCacheSize = 0; i < MAX_RAIDS; i++)
    {
        if (rs->cacheRaidAddr_DMC[i] != NULL)
        {
            raidCacheSize += ((MRGETRINFO_RSP *)(rs->cacheRaidAddr_DMC[i]))->header.len;
        }
    }
    return raidCacheSize;
}
#endif  /* NO_RAID_CACHE */

#ifndef NO_RAID_CACHE
/*----------------------------------------------------------------------------
** Function:    RaidsGet
**
** Returns:     NONE
**
** Warning:     THIS FUNCTION DOES NOT CHANGE THE CACHE STATE FOR THIS
**              DEVICE TYPE.  THIS FUNCTION IS NORMALLY USED IN CONJUNCTION
**              WITH "RaidsCount" AND IF THAT IS THE CASE THE USER
**              ***MUST*** SET THE CACHE STATE TO "IN USE" TO ENSURE THE
**              VALUES DO NOT CHANGE.  IF THIS IS DONE, REMEMBER TO SET
**              THE STATE BACK TO "OK" SO THAT THINGS WORK PROPERLY.
**--------------------------------------------------------------------------*/
void RaidsGet(void *buf)
{
    memcpy(buf, cacheRaids, CalcSizeRaidsCached());
}
#endif  /* NO_RAID_CACHE */

#ifndef NO_RAID_CACHE
/*----------------------------------------------------------------------------
** Function:    RefreshRaids
**
** Description: Get info on all RAIDs.  The data is cached and the
**              address of each RAID's entry is also cached.
**
** Inputs:      NONE
**
** Returns:     PI_GOOD, PI_ERROR
**--------------------------------------------------------------------------*/
INT32 RefreshRaids(void)
{
    INT32       rc = PI_GOOD;
    MR_LIST_RSP *ptrList = NULL;
    MRGETRINFO_REQ *pMRPInPkt = NULL;
    UINT8      *pCacheEntry;
    UINT32      raidsCopySize = 0;
    UINT32      outPktSize = 0;
    UINT16      count = 0;
    MRGETRINFO_RSP *pTmpPtr = NULL;

    /*
     * dprintf(DPRINTF_CACHEMGR, "RefreshRaids - ENTER (0x%x)\n", K_timel);
     */
    if (PowerUpBEReady())
    {
        /*
         * Wait for good cache status before continuing.  Set the flag
         * to indicate that the cache is updating.
         */
        CacheStateWaitOkToUpdate(cacheRaidsState);
        CacheStateSetUpdating(cacheRaidsState);

        /*
         * A little of math to start.  If we want to cache the MRGETRINFO_RSP
         * struct and MRGETRINFO_RSP_EXT for each PDisk in the RAID we need
         * a lot of memory.  Using MAX_PDISKS_PER_RAID=1024 and MAX_RAIDS=2048
         * we get
         * Base struct = 132 bytes
         * PSD_EXT     = 8*1024 = 8192
         * ---------------
         * Subtotal    = 8324 per RAID
         * x2048 MAX_RAIDS
         * ----------------
         * 17,047,552 bytes
         * Clearly a 17MB cache isn't practical.  If we cut MAX_PDISKS_PER_RAID
         * to 256 we still need 4,464,640 bytes.
         *
         * We probably need to look at only caching the data that the GUI
         * will request.  For now I will allocate the 4.5MB and see what
         * happens.
         */

        /*
         * Get the list of objects.  Always start at the beginning and return
         * the entire list.
         */
        ptrList = CacheGetList(MRGETRLIST);

        /* If we could not get the list, signal an error */
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

            outPktSize = sizeof(*pTmpPtr) + (MAX_PHYSICAL_DISKS * sizeof(MRGETRINFO_RSP_EXT));

            /*
             * Get Raid Info for each Raid in the list.  Place the info
             * in the cache.
             */
            for (count = 0; count < ptrList->ndevs; count++)
            {
                /*
                 * Get the next input ID from the list.  Do the math to set the
                 * bit for this ID in the RAID map.  id/8 gets to the correct
                 * byte, (1 << (id & 7)) gets the bit within the byte.
                 */
                pMRPInPkt->id = ptrList->list[count];

                pTmpPtr = (MRGETRINFO_RSP *)(cacheTempRaids + raidsCopySize);

                /* Try forever to get this MRP through. */
                rc = PI_ExecuteMRP(pMRPInPkt, sizeof(pMRPInPkt), MRGETRINFO,
                                   pTmpPtr, outPktSize,
                                   CACHE_MAX_REFRESH_MRP_TIMEOUT,
                                   PI_COMMAND_RECORD_TIMEOUT_ACTION_OUTPUT_NONE);
                if (rc == PI_GOOD)
                {
                    raidsCopySize += pTmpPtr->header.len;
                }
                else
                {
                    /*
                     * Some sort of error occurred.  cacheRaidAddr for
                     * this entry is NULL as initialized.
                     */
                    dprintf(DPRINTF_DEFAULT, "RefreshRaids - Error returned from MRGETRINFO (0x%x)\n", rc);
                    break;
                }
            }

            if (rc == PI_GOOD)
            {

                /*
                 * Clear out the cache buffers.  Clearing cacheRaidAddr
                 * should have the effect of setting all the pointers to NULL.  If
                 * this is done cacheRaids does NOT have to be cleared.
                 * Set up a pointer to the cache entry and initialize the array
                 * of pointers to each cache entry.
                 */
                memset(cacheRaidMap, 0x00, CACHE_SIZE_RAID_MAP);
                memset(cacheRaidAddr, 0x00, CACHE_SIZE_RAID_ADDR);

                /*
                 * Set the RAID count.
                 */
                cacheRaidsCount = ptrList->ndevs;

                /*
                 * Swap the pointers.
                 */
                SwapPointersRaids();
                pCacheEntry = cacheRaids;

                for (count = 0; count < ptrList->ndevs; count++)
                {
                    /*
                     * Get the next input ID from the list.  Do the math to set the
                     * bit for this ID in the RAID map.  id/8 gets to the correct
                     * byte, (1 << (id & 7)) gets the bit within the byte.
                     */
                    cacheRaidMap[ptrList->list[count] / 8] |= (1 << (ptrList->list[count] & 7));

                    /*
                     * Save the address of this cache entry in the lookup
                     * table cacheRaidAddr[].  Advance the cache
                     * pointer to the place where the next entry will go.
                     */
                    cacheRaidAddr[ptrList->list[count]] = pCacheEntry;
                    pCacheEntry += ((MRGETRINFO_RSP *)pCacheEntry)->header.len;
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

            /*
             * Make the raid cache as valid
             */
            PI_MakeCacheValid(1 << PI_CACHE_INVALIDATE_RAID);
            /*
             * Done updating cache - set good status.
             */
            CacheStateSetUpdateDone(cacheRaidsState);
        }
    }
    else
    {
        rc = PI_ERROR;
    }

    /*
     * dprintf(DPRINTF_CACHEMGR, "RefreshRaids - EXIT (0x%x)\n", K_timel);
     */
    return rc;
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
