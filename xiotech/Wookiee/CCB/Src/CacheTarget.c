/* $Id: CacheTarget.c 143845 2010-07-07 20:51:58Z mdr $*/
/*===========================================================================
** FILE NAME:       CacheTarget.c
** MODULE TITLE:    CCB Cache - Targets
**
** DESCRIPTION:     Cached data for Targets
**
** Copyright (c) 2002-2009  XIOtech Corporation.  All rights reserved.
**==========================================================================*/

#include "CacheLoop.h"
#include "CacheTarget.h"
#include "CacheManager.h"
#include "debug_files.h"
#include "LOG_Defs.h"
#include "MR_Defs.h"
#include "PI_Utils.h"
#include "X1_Structs.h"

/*****************************************************************************
** Private variables
*****************************************************************************/
static UINT8 *cacheTargetAddr[MAX_TARGETS];

/*****************************************************************************
** Public variables - externed in the header file
*****************************************************************************/
UINT16      cacheTargetsCount = 0;
UINT8       cacheTargetMap[CACHE_SIZE_TARGET_MAP];
UINT8       cacheTargets1[CACHE_SIZE_TARGETS] LOCATE_IN_SHMEM;
UINT8       cacheTargets2[CACHE_SIZE_TARGETS] LOCATE_IN_SHMEM;

UINT8      *cacheTargets = NULL;
UINT8      *cacheTempTargets = NULL;

/*****************************************************************************
** Code Start
*****************************************************************************/


/*----------------------------------------------------------------------------
** Function:    RefreshTargets
**
** Description: Get info on all Targets.  The data is cached and the
**              address of each Target's entry is also cached.
**
** Inputs:      NONE
**
** Returns:     PI_GOOD, PI_ERROR
**--------------------------------------------------------------------------*/
INT32 RefreshTargets(void)
{
    INT32       rc = PI_GOOD;
    MR_LIST_RSP *ptrList = NULL;
    MRGETTARG_REQ *pMRPInPkt = NULL;
    UINT8      *pCacheEntry = NULL;
    UINT32      targetsCopySize = 0;
    UINT16      count = 0;
    MRGETTARG_RSP *pTmpPtr = NULL;

    /*
     * dprintf(DPRINTF_CACHEMGR, "RefreshTargets - ENTER (0x%x)\n", K_timel);
     */

    if (CacheFEReady())
    {
        /*
         * Get the list of objects.  Always start at the beginning and return
         * the entire list.
         */
        ptrList = CacheGetList(MRGETTLIST);

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

            /*
             * Get Target Info for each Target in the list.  Place the info
             * in the cache.
             */
            for (count = 0; count < ptrList->ndevs; count++)
            {
                /*
                 * Get the next input ID from the list.  Do the math to set the
                 * bit for this ID in the Target map.  id/8 gets to the correct
                 * byte, (1 << (id & 7)) gets the bit within the byte.
                 */
                pMRPInPkt->id = ptrList->list[count];

                pTmpPtr = (MRGETTARG_RSP *)(cacheTempTargets + targetsCopySize);

                /*
                 * Try forever to get this MRP through.
                 */
                rc = PI_ExecuteMRP(pMRPInPkt, sizeof(*pMRPInPkt), MRGETTARG,
                                   pTmpPtr, sizeof(*pTmpPtr),
                                   CACHE_MAX_REFRESH_MRP_TIMEOUT,
                                   PI_COMMAND_RECORD_TIMEOUT_ACTION_OUTPUT_NONE);
                if (rc == PI_GOOD)
                {
                    targetsCopySize += pTmpPtr->header.len;
                }
                else
                {
                    break;
                    /*
                     * Some sort of error occurred.  cacheTargetAddr for
                     * this entry is NULL as initialized.
                     */
                }
            }

            if (rc == PI_GOOD)
            {
                /*
                 * Wait for good cache status before continuing.  Set the flag
                 * to indicate that the cache is updating.
                 */
                CacheStateWaitOkToUpdate(cacheTargetsState);
                CacheStateSetUpdating(cacheTargetsState);

                /*
                 * Clear out the cache buffers.  Clearing cacheTargetAddr
                 * should have the effect of setting all the pointers to NULL.  If
                 * this is done cacheTargets does NOT have to be cleared.
                 * Set up a pointer to the cache entry and initialize the array
                 * of pointers to each cache entry.
                 */
                memset(cacheTargetMap, 0x00, CACHE_SIZE_TARGET_MAP);
                memset(cacheTargetAddr, 0x00, CACHE_SIZE_TARGET_ADDR);

                /*
                 * Get the Target count from the PI_GetList call.
                 */
                cacheTargetsCount = ptrList->ndevs;

                /*
                 * Swap the pointers.
                 */
                SwapPointersTargets();
                pCacheEntry = cacheTargets;

                for (count = 0; count < ptrList->ndevs; count++)
                {
                    /*
                     * Add this target to the map.
                     */
                    cacheTargetMap[ptrList->list[count] / 8] |= (1 << (ptrList->list[count] & 7));

                    /*
                     * Save the address of this cache entry in the lookup
                     * table cacheTargetAddr[].  Advance the cache
                     * pointer to the place where the next entry will go.
                     */
                    cacheTargetAddr[ptrList->list[count]] = pCacheEntry;
                    pCacheEntry += ((MRGETTARG_RSP *)pCacheEntry)->header.len;
                }

                /*
                 * Done updating cache - set good status.
                 */
                CacheStateSetUpdateDone(cacheTargetsState);
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
    }
    else
    {
        rc = PI_ERROR;
    }

    /*
     * dprintf(DPRINTF_CACHEMGR, "RefreshTargets - EXIT (0x%x)\n", K_timel);
     */

    return rc;
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
