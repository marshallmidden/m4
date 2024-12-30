/* $Id: CacheServer.c 143845 2010-07-07 20:51:58Z mdr $ */
/**
******************************************************************************
**
**  @file   CacheServer.c
**
**  @brief  CCB Cache - Cached data for Servers
**
**  Copyright (c) 2002, 2003, 2008-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#include <byteswap.h>
#include "CacheManager.h"
#include "CacheMisc.h"
#include "CacheServer.h"
#include "cps_init.h"
#include "debug_files.h"
#include "LOG_Defs.h"
#include "LargeArrays.h"
#include "misc.h"
#include "MR_Defs.h"
#include "PI_Utils.h"
#include "X1_Structs.h"
#include "X1_Utils.h"

/*****************************************************************************
** Private variables
*****************************************************************************/
static UINT16 cacheServersCount = 0;

static UINT16   cacheServerLookup[MAX_SERVERS_UNIQUE];

/*****************************************************************************
** Public variables - externed in the header file
*****************************************************************************/
UINT8       cacheServerMap[CACHE_SIZE_SERVER_MAP];

UINT8      *cacheServers = NULL;
UINT8      *cacheTempServers = NULL;
UINT32      cacheHABMapFE;
UINT32      cacheHABMapBE;

static UINT8    *cacheServerAddr[MAX_SERVERS];

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static WWN_TO_TARGET_LIST *ServerWwnToTargetList(void);

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
** Function:    ServersCount
**
** Inputs:      NONE
**
** Returns:     Count of the Servers in the system using the cached
**              value.
**
** Warning:     THIS FUNCTION DOES NOT CHANGE THE CACHE STATE FOR THIS
**              DEVICE TYPE.  THIS FUNCTION IS NORMALLY USED IN CONJUNCTION
**              WITH "ServersGet" AND IF THAT IS THE CASE THE USER
**              ***MUST*** SET THE CACHE STATE TO "IN USE" TO ENSURE THE
**              VALUES DO NOT CHANGE.  IF THIS IS DONE, REMEMBER TO SET
**              THE STATE BACK TO "OK" SO THAT THINGS WORK PROPERLY.
**--------------------------------------------------------------------------*/
static UINT16 ServersCount(void)
{
    return cacheServersCount;
}


/*----------------------------------------------------------------------------
** Function:    CalcSizeServersCached
**
** Description: Calculate the size (in bytes) of the buffer required to
**              hold all the information for all the Servers.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
static INT32 CalcSizeServersCached(void)
{
    UINT16      i;
    UINT32      serverCacheSize = 0;

    /*
     * Walk the list of addresses to cache entries, pulling out the
     * length of each valid entry.
     */
    for (i = 0; i < MAX_SERVERS; i++)
    {
        if (cacheServerAddr[i] != NULL)
        {
            serverCacheSize += ((MRGETSINFO_RSP *)(cacheServerAddr[i]))->header.len;
        }
    }

    return (serverCacheSize);
}


/*----------------------------------------------------------------------------
** Function:    ServersGet
**
** Returns:     NONE
**
** Warning:     THIS FUNCTION DOES NOT CHANGE THE CACHE STATE FOR THIS
**              DEVICE TYPE.  THIS FUNCTION IS NORMALLY USED IN CONJUNCTION
**              WITH "ServersCount" AND IF THAT IS THE CASE THE USER
**              ***MUST*** SET THE CACHE STATE TO "IN USE" TO ENSURE THE
**              VALUES DO NOT CHANGE.  IF THIS IS DONE, REMEMBER TO SET
**              THE STATE BACK TO "OK" SO THAT THINGS WORK PROPERLY.
**--------------------------------------------------------------------------*/
static void ServersGet(void *buf)
{
    memcpy(buf, cacheServers, CalcSizeServersCached());
}


/**
******************************************************************************
**
**  @brief  Get the server info associated with the input WWN
**
**  @param  wwn - WWN of the server
**  @param  pServerInfoOut - pointer to server info for the requested WWN
**                                  NULL if this WWN not found.
**
**  @return GOOD if the server with "wwn" was found, ERROR otherwise.
**
**  @attention  This method copies only the static portion of the server
**              information (MRGETSINFO_RSP).  It will not copy the LUN
**              MAP portion of the server information.
**
******************************************************************************
**/
INT32 GetServerInfoFromWwn(UINT64 wwn, MRGETSINFO_RSP *pServerInfoOut)
{
    MRGETSINFO_RSP *pServerInfo = NULL;
    INT32       rc = ERROR;
    UINT32      count = 0;

    /*
     * Wait until the disk bay cache is not in the process of
     * being updated.  Once it is in that state make sure it is set
     * to in use so a update doesn't start while it is being used.
     */
    CacheStateWaitUpdating(cacheServersState);
    CacheStateSetInUse(cacheServersState);

    /*
     * Loop through all of the servers in the cache and find the
     * first one that matches the WWN.
     */
    for (count = 0; count < MAX_SERVERS; count++)
    {
        pServerInfo = (MRGETSINFO_RSP *)cacheServerAddr[count];

        /*
         * If this server address is valid and it matches the
         * WWN we are searching for, copy the server information
         * into the outputpacket and quit looking.
         */
        if (pServerInfo != NULL && pServerInfo->wwn == wwn)
        {
            /*
             * Copy only the main MRGETSINFO_RSP structure, this will
             * not copy the LUNMAP.
             */
            memcpy(pServerInfoOut, pServerInfo, sizeof(*pServerInfoOut));
            rc = GOOD;
            break;
        }
    }

    /*
     * No longer using the cache for this function...
     */
    CacheStateSetNotInUse(cacheServersState);

    return (rc);
}


/**
******************************************************************************
**
**  @brief  Get the server info associated with the input WWN with no wait
**
**  @param  wwn - WWN of the server
**  @param  pServerInfoOut - pointer to server info for the requested WWN
**                                  NULL if this WWN not found.
**
**  @return GOOD if the server with "wwn" was found, ERROR otherwise.
**
**  @attention  This method copies only the static portion of the server
**              information (MRGETSINFO_RSP).  It will not copy the LUN
**              MAP portion of the server information.
**
******************************************************************************
**/
INT32 GetServerInfoFromWwnNOW(UINT64 wwn, MRGETSINFO_RSP *pServerInfoOut)
{
    MRGETSINFO_RSP *pServerInfo;
    UINT32      count;

    /*
     * Wait until the disk bay cache is not in the process of
     * being updated.  Once it is in that state make sure it is set
     * to in use so a update doesn't start while it is being used.
     */
    if (!pServerInfoOut || CacheStateUpdating(cacheServersState))
    {
        return ERROR;
    }

    /*
     * Loop through all of the servers in the cache and find the
     * first one that matches the WWN.
     */
    for (count = 0; count < MAX_SERVERS; count++)
    {
        pServerInfo = (MRGETSINFO_RSP *)cacheServerAddr[count];

        /*
         * If this server address is valid and it matches the
         * WWN we are searching for, copy the server information
         * into the outputpacket and quit looking.
         */
        if (pServerInfo && pServerInfo->wwn == wwn)
        {
            /*
             * Copy only the main MRGETSINFO_RSP structure, this will
             * not copy the LUNMAP.
             */
            *pServerInfoOut = *pServerInfo;
            return GOOD;
        }
    }

    return ERROR;
}


/*----------------------------------------------------------------------------
** Function:    GetServerInfoFromSid
**
** Description: Retrieve server info for a particular Server ID
**
** Inputs:      sid -   server ID
**
** Outputs:     MRGETSINFO_RSP*     pointer to server info for the requested
**                                  WWN.  NULL if this WWN not found.
**
** Returns:     GOOD if the server at "sid" was found, ERROR otherwise.
**
** Warning:     This method copy only the static portion of the server
**              information (MRGETSINFO_RSP).  It will not copy the LUN
**              MAP portion of the server information.
**--------------------------------------------------------------------------*/
INT32 GetServerInfoFromSid(UINT16 sid, MRGETSINFO_RSP *pServerInfoOut)
{
    MRGETSINFO_RSP *pServerInfoCache;
    INT32       rc = ERROR;

    /*
     * Wait until the disk bay cache is not in the process of
     * being updated.  Once it is in that state make sure it is set
     * to in use so a update doesn't start while it is being used.
     */
    CacheStateWaitUpdating(cacheServersState);
    CacheStateSetInUse(cacheServersState);

    /*
     * Get a pointer to the start of the Server cache.
     */
    pServerInfoCache = (MRGETSINFO_RSP *)cacheServerAddr[sid];

    /*
     * If the pointer is valid continue.
     */
    if (pServerInfoCache != NULL)
    {
        /*
         * Copy only the main MRGETSINFO_RSP structure, this will
         * not copy the LUNMAP.
         */
        memcpy(pServerInfoOut, pServerInfoCache, sizeof(*pServerInfoOut));
        rc = GOOD;
    }

    /*
     * No longer using the cache for this function...
     */
    CacheStateSetNotInUse(cacheServersState);

    return (rc);
}


/*----------------------------------------------------------------------------
** Function:    ServerWwnToTargetList
**
** Description: Generate a list of physical Servers (unique Server WWNs)
**              along with a bit map of their associated targets
**
** Inputs:      none
**
** Returns:     PI_GOOD or PI_ERROR
**
** WARNINGS:    1. Caller must insure cache is locked.
**              2. Caller must free pListOut memory
**
**--------------------------------------------------------------------------*/
static WWN_TO_TARGET_LIST *ServerWwnToTargetList(void)
{
    WWN_TO_TARGET_LIST *pListOut = NULL;
    UINT8      *pServerInfoCache = NULL;
    UINT8      *pName = NULL;
    UINT64      wwn;
    UINT16      sid;
    UINT16      listIndex = 0;
    UINT16      targetMap = 0;
    UINT16      targetId = 0;
    UINT16      selectedTarget = 0;
    UINT16      i;
    bool        wwnFound = FALSE;

    /*
     * Get a pointer to the cached server info.  Since each Server Info
     * record is variable length, this is a BYTE pointer.
     */
    pServerInfoCache = cacheServers;

    /*
     * Allocate memory for the output list.
     * We don't know in advance how many unique Servers (i.e. WWNs)
     * we will find but it must be <= the number of servers in the
     * Server cache.
     */
    pListOut = MallocWC(sizeof(*pListOut) + (ServersCount() * sizeof(WWN_TO_TARGET)));

    /*
     * Initialize the count of list items and the wwnFound flag.
     * Some details on other variables -
     *
     *  listIndex        - is used to walk through the output data to see
     *                     if there is already an entry for the current WWN.
     *  pListOut->count  - The place where the next list entry goes in the
     *                     output data.
     *  wwnFound         - This WWN was found in the output data.
     */
    pListOut->count = 0;
    wwnFound = FALSE;

    /*
     * Walk the Server cache creating a WWN_TO_TARGET entry for each
     * unique WWN.
     */
    for (i = 0; i < ServersCount(); i++)
    {
        /*
         * Reset listIndex for this time through the loop.
         */
        listIndex = 0;

        /*
         * Get the WWN and sid for this server record.
         * Create a target bit map.
         */
        wwn = ((MRGETSINFO_RSP *)pServerInfoCache)->wwn;
        sid = ((MRGETSINFO_RSP *)pServerInfoCache)->sid;
        targetId = ((MRGETSINFO_RSP *)pServerInfoCache)->targetId;
        pName = ((MRGETSINFO_RSP *)pServerInfoCache)->i_name;
        targetMap = 1 << (targetId & 15);

        /*
         * If this server record has the select target bit set, save the
         * targetID + 1.  This is done because X1 uses targetID=0 to
         * indicate that no target is selected.
         */
        if (((MRGETSINFO_RSP *)pServerInfoCache)->attrib & MRSERVERPROP_SELECT_TARGET)
        {
            selectedTarget = targetId + 1;
        }
        else
        {
            selectedTarget = 0;
        }

        /*
         * Loop through the current output list to find a match for this wwn.
         * If a match does not exist a new list entry will be created below.
         */
        while (listIndex < pListOut->count)
        {
            /*
             * If we have already saved an entry for this WWN, "OR" the new
             * target map with the existing map.  Also make sure we have
             * the lowest sid for this wwn.
             */
            if (strcmp((char *)pListOut->list[listIndex].i_name, (char *)pName) == 0)
            {
                pListOut->list[listIndex].targetBitmap |= targetMap;

                /*
                 * See if this sid is lower than the one we already
                 * have saved.  We use the lowest number sid to identify
                 * this server to X1.  (In other places this is called
                 * x1Sid).
                 */
                if (sid < pListOut->list[listIndex].sid)
                {
                    pListOut->list[listIndex].sid = sid;
                }

                /*
                 * If this server record contains the selected target, save
                 * the target value.
                 */
                if (selectedTarget > 0)
                {
                    pListOut->list[listIndex].selectedTarget = selectedTarget;
                }

                wwnFound = TRUE;        /* Do NOT create a new list entry */
                break;          /* Exit the while loop */
            }
            listIndex++;        /* Move to the next entry in the output data */
        }

        /*
         * If no entry exists in the output data for this WWN,
         * create one now.
         */
        if (!wwnFound)
        {
            pListOut->list[pListOut->count].sid = sid;
            pListOut->list[pListOut->count].targetBitmap = targetMap;
            pListOut->list[pListOut->count].selectedTarget = selectedTarget;
            pListOut->list[pListOut->count].wwn = wwn;
            strcpy((char *)pListOut->list[pListOut->count].i_name, (char *)pName);

            /*
             * Indicate that an entry was added to the response data.
             */
            pListOut->count++;
        }

        /*
         * Reset the wwnFound flag.
         * Point to the next Server cache entry.
         */
        wwnFound = FALSE;
        pServerInfoCache += ((MRGETSINFO_RSP *)pServerInfoCache)->header.len;
    }

    return (pListOut);
}


/*----------------------------------------------------------------------------
** Function:    RefreshServers
**
** Description: Get info on all Serverss.  The data is cached and the
**              address of each Server's entry is also cached.
**
** Inputs:      NONE
**
** Returns:     PI_GOOD, PI_ERROR
**--------------------------------------------------------------------------*/
INT32 RefreshServers(void)
{
    INT32       rc = PI_GOOD;
    MR_LIST_RSP *ptrList = NULL;
    MRGETSINFO_REQ *pMRPInPkt = NULL;
    UINT8      *pCacheEntry;
    UINT32      serversCopySize = 0;
    UINT32      outPktSize = 0;
    UINT16      count = 0;
    WWN_TO_TARGET_LIST *pServerList;
    UINT16      i;
    MRGETSINFO_RSP *pTmpPtr = NULL;

    /*
     * dprintf(DPRINTF_CACHEMGR, "RefreshServers - ENTER (0x%x)\n", K_timel);
     */

    if (CacheFEReady())
    {
        /*
         * Get the list of objects.  Always start at the beginning and return
         * the entire list.
         */
        ptrList = CacheGetList(MRGETSLIST);

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

            outPktSize = sizeof(*pTmpPtr) + (MAX_LUNS * sizeof(MRGETSINFO_RSP_LM));

            /*
             * Get Server Info for each Server in the list.  Place the info
             * in the cache.
             */
            for (count = 0; count < ptrList->ndevs; count++)
            {
                /*
                 * Get the next input ID from the list.
                 */
                pMRPInPkt->id = ptrList->list[count];

                pTmpPtr = (MRGETSINFO_RSP *)(cacheTempServers + serversCopySize);

                /*
                 * Try forever to get this MRP through.
                 */
                rc = PI_ExecuteMRP(pMRPInPkt, sizeof(*pMRPInPkt), MRGETSINFO,
                                   pTmpPtr, outPktSize,
                                   CACHE_MAX_REFRESH_MRP_TIMEOUT,
                                   PI_COMMAND_RECORD_TIMEOUT_ACTION_OUTPUT_NONE);
                if (rc == PI_GOOD)
                {
                    serversCopySize += pTmpPtr->header.len;
                }
                else
                {
                    break;
                    /*
                     * Some sort of error occurred.  cacheServerAddr for
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
                CacheStateWaitOkToUpdate(cacheServersState);
                CacheStateSetUpdating(cacheServersState);

                /*
                 * Clear out the cache buffers.  Clearing cacheServerAddr
                 * should have the effect of setting all the pointers to NULL.  If
                 * this is done cacheServers does NOT have to be cleared.
                 * Set up a pointer to the cache entry and initialize the array
                 * of pointers to each cache entry.
                 */
                memset(cacheServerMap, 0x00, CACHE_SIZE_SERVER_MAP);
                memset(cacheServerAddr, 0x00, CACHE_SIZE_SERVER_ADDR);

                /*
                 * Initialize the cacheServerLookup to an invalid sid
                 * value that allows for validity checking.
                 */
                memset(cacheServerLookup, (UINT8)INVALID_SID, (MAX_SERVERS_UNIQUE * sizeof(UINT16)));

                /*
                 * Get the Server count from the PI_GetList call.
                 */
                cacheServersCount = ptrList->ndevs;

                /*
                 * Swap the pointers.
                 */
                SwapPointersServers();
                pCacheEntry = cacheServers;

                /*
                 * Fill out the maps.
                 */
                for (count = 0; count < ptrList->ndevs; count++)
                {
                    /*
                     * Save the address of this cache entry in the lookup
                     * table cacheVirtualDiskAddr[].  Advance the cache
                     * pointer to the place where the next entry will go.
                     */
                    cacheServerAddr[ptrList->list[count]] = pCacheEntry;
                    pCacheEntry += ((MRGETSINFO_RSP *)pCacheEntry)->header.len;
                }

                /*
                 * Now that all the Server Info has been cached, determine the count
                 * of unique Servers (WWNs) and create the Server Map.
                 */
                pServerList = ServerWwnToTargetList();

                for (i = 0; i < pServerList->count && i < MAX_SERVERS_UNIQUE; i++)
                {
                    cacheServerLookup[i] = pServerList->list[i].sid;
                    cacheServerMap[i / 8] |= (1 << (i & 7));
                }

                Free(pServerList);

                /*
                 * Done updating cache - set good status.
                 */
                CacheStateSetUpdateDone(cacheServersState);
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
     * dprintf(DPRINTF_CACHEMGR, "RefreshServers - EXIT (0x%x)\n", K_timel);
     */

    return rc;
}

/*----------------------------------------------------------------------------
** Function:    RefreshHABs
**
** Description: Get info on all HABs.
**
** Inputs:      NONE
**
** Returns:     PI_GOOD, PI_ERROR
**--------------------------------------------------------------------------*/
INT32 RefreshHABs(void)
{
    INT32       rc = PI_GOOD;
    MR_LIST_RSP *bePtrList = NULL;
    MR_LIST_RSP *fePtrList = NULL;
    UINT16      count = 0;

    /*
     * The controller must be power-up complete before attempting to get
     * the HAB information.
     */
    if (PowerUpBEReady() && CacheCheckFEReady())
    {
        /*
         * Get the front end and back end port lists.
         */
        if (rc == PI_GOOD)
        {
            fePtrList = CacheGetList(MRFEGETPORTLIST);

            if (fePtrList == NULL)
            {
                rc = PI_ERROR;
            }
        }

        if (rc == PI_GOOD)
        {
            bePtrList = CacheGetList(MRBEGETPORTLIST);

            if (bePtrList == NULL)
            {
                rc = PI_ERROR;
            }
        }

        if (rc == PI_GOOD)
        {
            /*
             * Clearing the cached information.
             */
            cacheHABMapFE = 0;

            /*
             * Loop through the FE list and determine which
             * HABs are available.
             */
            for (count = 0; count < fePtrList->ndevs && count < 15; count++)
            {
                /*
                 * Get the next input ID from the list.  Do the math
                 * to set the bit for this ID in the HAB map.
                 * ptrList->list[count] is the HAB ID.  The code
                 * currently only supports IDs 0-15 for the front
                 * end and 0-15 for the back end.
                 */
                cacheHABMapFE |= (0x01 << fePtrList->list[count]);
            }

            cacheHABMapBE = 0;
            /*
             * Loop through the BE list and determine which
             * HABs are available.
             */
            for (count = 0; count < bePtrList->ndevs && count < 15; count++)
            {
                /*
                 * Get the next input ID from the list.  Do the math
                 * to set the bit for this ID in the HAB map.
                 * ptrList->list[count] is the HAB ID.  The code
                 * currently only supports IDs 0-15 for the front
                 * end and 0-15 for the back end.
                 */
                cacheHABMapBE |= (0x01 << bePtrList->list[count]);
            }
        }

        /*
         * Done with the lists so they can now be freed.
         */
        Free(fePtrList);
        Free(bePtrList);
    }
    else
    {
        rc = PI_ERROR;
    }

    return rc;
}

/*----------------------------------------------------------------------------
** Function:    CachedServers()
**
** Description: Get the information for all the servers.
**
** Inputs:      NONE
**
** Returns:     Pointer to a servers response packet or NULL if they
**              could not be retrieved.
**
** WARNING:     The pointer to the response packet needs to be freed by the
**              caller or it will be leaked.
**--------------------------------------------------------------------------*/
PI_SERVERS_RSP *CachedServers(void)
{
    PI_SERVERS_RSP *pOutData;
    UINT32      size;

    /*
     * Wait until the Servers cache is not in
     * the process of being updated.  Once it is in
     * that state make sure it is set to in use so
     * a update doesn't start while it is being
     * used.
     */
    CacheStateWaitUpdating(cacheServersState);
    CacheStateSetInUse(cacheServersState);

    /*
     * Calculate the size of the output packet.  This will be the size
     * of the Servers (multiple devices) response packet plus the
     * size of a Server Info (single device) response packet for each
     * device.
     */
    size = sizeof(*pOutData) + CalcSizeServersCached();

    pOutData = MallocWC(size);

    /*
     * Copy the number of devices into the output packet along
     * with the data for each Server.
     */
    if (pOutData)
    {
        pOutData->count = ServersCount();
        ServersGet(&pOutData->servers);
    }

    CacheStateSetNotInUse(cacheServersState);

    return pOutData;
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
