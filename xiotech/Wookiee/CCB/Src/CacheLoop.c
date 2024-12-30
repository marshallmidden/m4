/* $Id: CacheLoop.c 146064 2010-08-20 21:15:33Z m4 $*/
/*===========================================================================
** FILE NAME:       CacheLoop.c
** MODULE TITLE:    CCB Cache - BE and FE Loop
**
** DESCRIPTION:     Cached data for BE and FE Loop Info
**
** Copyright (c) 2002-2009  XIOtech Corporation.  All rights reserved.
**==========================================================================*/

#include "CacheLoop.h"
#include "CacheManager.h"
#include "CacheServer.h"
#include "cps_init.h"
#include "debug_files.h"
#include "LOG_Defs.h"
#include "MR_Defs.h"
#include "PI_Utils.h"
#include "X1_Structs.h"
#include "XIO_Macros.h"

/*****************************************************************************
** Private variables
*****************************************************************************/
static UINT32 cacheBEFabricMode = 0;    /* Can accomodate 32 BE ports */

/*****************************************************************************
** Public variables - externed in the header file
*****************************************************************************/
UINT8      *cacheFELoopStatsAddr[MAX_FE_PORTS];
UINT8       cacheFELoopStats1[CACHE_SIZE_FE_LOOP_STATS] LOCATE_IN_SHMEM;
UINT8       cacheFELoopStats2[CACHE_SIZE_FE_LOOP_STATS] LOCATE_IN_SHMEM;

UINT8      *cacheBELoopStatsAddr[MAX_BE_PORTS];
UINT8       cacheBELoopStats1[CACHE_SIZE_BE_LOOP_STATS] LOCATE_IN_SHMEM;
UINT8       cacheBELoopStats2[CACHE_SIZE_BE_LOOP_STATS] LOCATE_IN_SHMEM;

UINT8      *cacheFELoopStats = NULL;
UINT8      *cacheTempFELoopStats = NULL;
UINT8      *cacheBELoopStats = NULL;
UINT8      *cacheTempBELoopStats = NULL;

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
** Function:    GetBELoopInfo
**
** Description: Get the Back End Loop Information for all ports
**
** Inputs:      UINT8 * pointer to buffer where response data will be copied.
**
** Returns:     none
**
**--------------------------------------------------------------------------*/
void GetBELoopInfo(X1_BE_LOOP_INFO_RSP *pBELoopInfoRsp)
{
    memcpy(pBELoopInfoRsp->portInfo, cacheBEPortInfo, CACHE_SIZE_BE_PORT_INFO);
}

/*----------------------------------------------------------------------------
** Function:    RefreshBELoopStats
**
** Description: Get Back End Loop Statistics for all available ports.
**              The data is cached and the address of each port's
**              entry is also cached.
**
** Inputs:      NONE
**
** Returns:     PI_GOOD, PI_ERROR
**--------------------------------------------------------------------------*/
INT32 RefreshBELoopStats(void)
{
    MRBELOOP_REQ *pMRPInPkt = NULL;
    UINT8      *pCacheEntry = NULL;
    UINT8      *pCache = NULL;
    INT32       rc = PI_GOOD;
    UINT32      outPktSize = 0;
    UINT32      statsCopySize = 0;
    UINT8       i = 0;
    UINT32      habMapBE = 0;
    UINT8       portCount = 0;
    MRBELOOP_RSP *pTmpPtr = NULL;

    if (PowerUpBEReady())
    {
        /*
         * The HAB map is handled as part of the Servers cache.
         * Wait until the Servers cache is not in the process of being
         * updated.  Once it is in that state make sure it is set to in
         * use so a update doesn't start while it is being used.
         */
        CacheStateWaitUpdating(cacheServersState);
        CacheStateSetInUse(cacheServersState);

        /*
         * Get a local copy of the HAB map so the cache can be released.
         */
        habMapBE = cacheHABMapBE;

        /*
         * No longer using the cache for this function...
         */
        CacheStateSetNotInUse(cacheServersState);

        /*
         * Allocate the input packet used to retrieve the
         * information from the PROC.
         */
        pMRPInPkt = MallocWC(sizeof(*pMRPInPkt));

        outPktSize = sizeof(*pTmpPtr) + (MAX_TARGETS * sizeof(UINT16));

        /*
         * Get Loop Stats for each BE port.  Place the stats in the cache.
         */
        for (i = 0; i < MAX_BE_PORTS; i++)
        {
            /*
             * Derive the valid port numbers from the habMap.
             */
            if (habMapBE & (1 << i))
            {
                /* Get the port number and increment the port count. */
                pMRPInPkt->port = i;
                portCount++;

                pCache = (cacheTempBELoopStats + statsCopySize);
                pTmpPtr = ((MRBELOOP_RSP *)(pCache + sizeof(PI_STATS_LOOP)));

                /*
                 * Try forever to get this MRP through.
                 */
                rc = PI_ExecuteMRP(pMRPInPkt, sizeof(*pMRPInPkt), MRBELOOP,
                                   pTmpPtr, outPktSize,
                                   CACHE_MAX_REFRESH_MRP_TIMEOUT,
                                   PI_COMMAND_RECORD_TIMEOUT_ACTION_OUTPUT_NONE);

                if (rc == PI_GOOD)
                {
                    /*
                     * The MRP output struct and the cache struct for a Loop
                     * Stats entry are slightly different.  The cache struct
                     * contains length and port number for the Loop Stats.
                     * The length does NOT include the length
                     * field itself.  Since the cache entries are variable
                     * length, pCache is used to point to the spot in the
                     * cache where the data value should be copied.
                     */
                    ((PI_STATS_LOOP *)(pCache))->length =
                        sizeof(((PI_STATS_LOOP *)pCache)->port) + pTmpPtr->header.len;

                    ((PI_STATS_LOOP *)(pCache))->port = pMRPInPkt->port;

                    statsCopySize += pTmpPtr->header.len + sizeof(PI_STATS_LOOP);
                }
                else
                {
                    /*
                     * Some sort of error occurred.  cacheBELoopStatsAddr for
                     * this entry is NULL as initialized.
                     */
                    dprintf(DPRINTF_DEFAULT, "RefreshBELoopStats: ERROR requesting loop stats - status=0x%02hhX\n",
                            ((MRBELOOP_RSP *)pCache)->header.status);
                    break;
                }
            }
        }

        /*
         * All the stats have been gathered and are in the temporary cache.
         * Copy them into the actual cache.
         */
        if (rc == PI_GOOD)
        {
            /*
             * Wait for good cache status before continuing.  Set the flag
             * to indicate that the cache is updating.
             */
            CacheStateWaitOkToUpdate(cacheStatsBeState);
            CacheStateSetUpdating(cacheStatsBeState);

            /*
             * Clear out the cache buffers.  Clearing cacheBELoopStatsAddr
             * should have the effect of setting all the pointers to NULL.
             * If this is done cacheBELoopStats does NOT have to be cleared.
             */
            memset(cacheBELoopStatsAddr, 0x00, CACHE_SIZE_BE_LOOP_STATS_ADDR);

            /*
             * Swap the pointers.
             */
            SwapPointersBELoopStats();
            pCacheEntry = cacheBELoopStats;

            /*
             * Build cacheBELoopStatsAddr from cacheBELoopStats.
             */
            for (i = 0; i < MAX_BE_PORTS; i++)
            {
                /*
                 * Stats data will only exist if the port exists.
                 */
                if (habMapBE & (1 << i))
                {
                    /*
                     * Save the address of this cache entry in the lookup
                     * table cacheBELoopStatsAddr[].  Advance the cache
                     * pointer to the place where the next entry starts.
                     */
                    cacheBELoopStatsAddr[i] = pCacheEntry;
                    pCacheEntry += (((PI_STATS_LOOP *)pCacheEntry)->length +
                                    sizeof(((PI_STATS_LOOP *)(pCacheEntry))->length));
                }
            }

            /*
             * Done updating cache - set good status.
             */
            CacheStateSetUpdateDone(cacheStatsBeState);
        }

        /*
         * Free the input packet now that all the information
         * has been gathered.
         */
        Free(pMRPInPkt);
    }
    else
    {
        rc = PI_ERROR;
    }

    return rc;
}

/*----------------------------------------------------------------------------
** Function:    RefreshFELoopStats
**
** Description: Get Front End Loop Statistics for all available ports.
**              The data is cached and the address of each port's
**              entry is also cached.
**
** Inputs:      NONE
**
** Returns:     PI_GOOD, PI_ERROR
**--------------------------------------------------------------------------*/
INT32 RefreshFELoopStats(void)
{
    MRFELOOP_REQ *pMRPInPkt = NULL;
    UINT8      *pCacheEntry = NULL;
    UINT8      *pCache = NULL;
    INT32       rc = PI_GOOD;
    UINT32      outPktSize = 0;
    UINT32      statsCopySize = 0;
    UINT8       i = 0;
    UINT32      habMapFE = 0;
    UINT8       portCount = 0;
    MRFELOOP_RSP *pTmpPtr = NULL;

    if (CacheFEReady())
    {
        /*
         * The HAB map is handled as part of the Servers cache.
         * Wait until the Servers cache is not in the process of being
         * updated.  Once it is in that state make sure it is set to in
         * use so a update doesn't start while it is being used.
         */
        CacheStateWaitUpdating(cacheServersState);
        CacheStateSetInUse(cacheServersState);

        /*
         * Get a local copy of the HAB map so the cache can be released.
         */
        habMapFE = cacheHABMapFE;

        /*
         * No longer using the cache for this function...
         */
        CacheStateSetNotInUse(cacheServersState);

        /*
         * Allocate the input packet used to retrieve the
         * information from the PROC.
         */
        pMRPInPkt = MallocWC(sizeof(*pMRPInPkt));

        outPktSize = sizeof(*pTmpPtr) + (MAX_TARGETS * sizeof(UINT16));

        /*
         * Get Loop Stats for each FE port.  Place the stats in the cache.
         */
        for (i = 0; i < MAX_FE_PORTS; i++)
        {
            /*
             * Derive the valid port numbers from the habMap.
             * The low nibble contains the FE port map.
             */
            if (habMapFE & (1 << i))
            {
                /* Get the port number and increment the port count. */
                pMRPInPkt->port = i;
                portCount++;

                pCache = (cacheTempFELoopStats + statsCopySize);
                pTmpPtr = ((MRFELOOP_RSP *)(pCache + sizeof(PI_STATS_LOOP)));

                /*
                 * Try forever to get this MRP through.
                 */
                rc = PI_ExecuteMRP(pMRPInPkt, sizeof(*pMRPInPkt), MRFELOOP,
                                   pTmpPtr, outPktSize,
                                   CACHE_MAX_REFRESH_MRP_TIMEOUT,
                                   PI_COMMAND_RECORD_TIMEOUT_ACTION_OUTPUT_NONE);

                if (rc == PI_GOOD)
                {
                    /*
                     * The MRP output struct and the cache struct for a Loop
                     * Stats entry are slightly different.  The cache struct
                     * contains length and port number for the Loop Stats.
                     * The length does NOT include the length
                     * field itself.  Since the cache entries are variable
                     * length, pCache is used to point to the spot in the
                     * cache where the data value should be copied.
                     */

                    ((PI_STATS_LOOP *)(pCache))->length =
                        sizeof(((PI_STATS_LOOP *)pCache)->port) + pTmpPtr->header.len;

                    ((PI_STATS_LOOP *)(pCache))->port = pMRPInPkt->port;

                    statsCopySize += pTmpPtr->header.len + sizeof(PI_STATS_LOOP);
                }
                else
                {
                    /*
                     * Some sort of error occurred.  cacheFELoopStatsAddr for
                     * this entry is NULL as initialized.
                     */
                    dprintf(DPRINTF_DEFAULT, "RefreshFELoopStats: ERROR requesting loop stats - status=0x%02hhX, rc=%d\n",
                            ((MRFELOOP_RSP *)pCache)->header.status, rc);
                    break;
                }
            }
        }

        /*
         * All the stats have been gathered and are in the temporary cache.
         * Copy them into the actual cache.
         */
        if (rc == PI_GOOD)
        {
            /*
             * Wait for good cache status before continuing.  Set the flag
             * to indicate that the cache is updating.
             */
            CacheStateWaitOkToUpdate(cacheStatsFeState);
            CacheStateSetUpdating(cacheStatsFeState);

            /*
             * Clear out the cache buffers.  Clearing cacheFELoopStatsAddr
             * should have the effect of setting all the pointers to NULL.
             * If this is done cacheFELoopStats does NOT have to be cleared.
             */
            memset(cacheFELoopStatsAddr, 0x00, CACHE_SIZE_FE_LOOP_STATS_ADDR);

            /*
             * Swap the pointers.
             */
            SwapPointersFELoopStats();
            pCacheEntry = cacheFELoopStats;

            /*
             * Build cacheFELoopStatsAddr from cacheFELoopStats.
             */
            for (i = 0; i < MAX_FE_PORTS; i++)
            {
                /*
                 * Stats data will only exist if the port exists.
                 */
                if (habMapFE & (1 << i))
                {
                    /*
                     * Save the address of this cache entry in the lookup
                     * table cacheFELoopStatsAddr[].  Advance the cache
                     * pointer to the place where the next entry starts.
                     */
                    cacheFELoopStatsAddr[i] = pCacheEntry;
                    pCacheEntry += (((PI_STATS_LOOP *)pCacheEntry)->length +
                                    sizeof(((PI_STATS_LOOP *)(pCacheEntry))->length));
                }
            }

            /*
             * Done updating cache - set good status.
             */
            CacheStateSetUpdateDone(cacheStatsFeState);
        }

        /*
         * Free the input packet now that all the information
         * has been gathered.
         */
        Free(pMRPInPkt);
    }
    else
    {
        rc = PI_ERROR;
    }

    return rc;
}

/*----------------------------------------------------------------------------
** Function:    SetBEFabricMode
**
** Description: Set the BE port fabric mode flag.
**
** Inputs:      port, mode
**
** Returns:     void
**--------------------------------------------------------------------------*/
void SetBEFabricMode(UINT32 port, UINT32 mode)
{
    if (port < MAX_BE_PORTS)
    {
        /*
         * Non-zero mode indicates fabric
         */
        if (mode)
        {
            BIT_SET(cacheBEFabricMode, port);
        }
        else
        {
            BIT_CLEAR(cacheBEFabricMode, port);
        }
    }
    else
    {
        dprintf(DPRINTF_DEFAULT, "SetBEFabricMode: port %d out-of-range!\n", port);
    }
}

/*----------------------------------------------------------------------------
** Function:    GetBEFabricMode
**
** Description: Get the BE port fabric mode flags.
**
** Inputs:      none
**
** Returns:     Bit field of port fabric flags: 0 == Loop, 1 == Fabric.
**--------------------------------------------------------------------------*/
UINT32 GetBEFabricMode(void)
{
    return cacheBEFabricMode;
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
