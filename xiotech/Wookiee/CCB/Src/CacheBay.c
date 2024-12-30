/* $Id: CacheBay.c 160832 2013-03-27 18:05:21Z marshall_midden $*/
/**
******************************************************************************
**
**  @file   CacheBay.c
**
**  @brief  CCB Cache - Cached data for Disk Bays
**
**  Copyright (c) 2002-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/

#include "CacheBay.h"
#include "CacheManager.h"
#include "CachePDisk.h"
#include "cps_init.h"
#include "debug_files.h"
#include "ipc_sendpacket.h"
#include "LOG_Defs.h"
#include "MR_Defs.h"
#include "PacketInterface.h"
#include "PI_Utils.h"
#include "rm_val.h"
#include "X1_Structs.h"


/*****************************************************************************
** Private variables
*****************************************************************************/
static UINT16 cacheDiskBaysCount = 0;
static UINT8 *cacheDiskBayAddr[MAX_DISK_BAYS];

/*****************************************************************************
** Public variables - externed in the header file
*****************************************************************************/
UINT8       cacheDiskBays1[CACHE_SIZE_DISK_BAYS] LOCATE_IN_SHMEM;
UINT8       cacheDiskBays2[CACHE_SIZE_DISK_BAYS] LOCATE_IN_SHMEM;
UINT8       cacheDiskBayMap[CACHE_SIZE_DISK_BAY_MAP];   /* This is a bit map */
UINT8       cacheFibreBayMap[CACHE_SIZE_DISK_BAY_MAP];  /* This is a bit map */
UINT8       cacheSATABayMap[CACHE_SIZE_DISK_BAY_MAP];   /* This is a bit map */
UINT8       cacheDiskBayPaths[CACHE_SIZE_DISK_BAY_PATHS];
UINT8      *cacheDiskBays = NULL;
UINT8      *cacheTempDiskBays = NULL;

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static void GetBayPathInfo(UINT16 bid, X1_BAY_INFO_RSP *pBayInfo);

/*****************************************************************************
** Code Start
*****************************************************************************/

/**
******************************************************************************
**
**  @brief  Get bay info from bid
**
**  @param  bid - Bay ID
**  @param  buf - Pointer to buffer to receive bay info
**
**  @return none
**
******************************************************************************
**/
void BayGetInfo(UINT16 bid, X1_BAY_INFO_RSP *pBayInfoRsp)
{
    MRGETEINFO_RSP *pBayInfoCache = NULL;

#if defined(MODEL_3000) || defined(MODEL_7400)
    SES_ELEM_CTRL page2Element;
#endif /* MODEL_3000 || MODEL_7400 */

    if (bid >= MAX_DISK_BAYS)
    {
        dprintf(DPRINTF_DEFAULT, "%s: bid %d out of range\n", __func__, bid);
        return;
    }

    /*
     * Wait until the Disk Bay cache is not in the process of being
     * updated. Once it is in that state make sure it is set to in
     * use so a update doesn't start while it is being used.
     */
    CacheStateWaitUpdating(cacheDiskBaysState);
    CacheStateSetInUse(cacheDiskBaysState);

    /*
     * Get a pointer from the Bay Info cache to the info for the input Bay.
     * Also cast a pointer for the response buffer.
     */
    pBayInfoCache = (MRGETEINFO_RSP *)cacheDiskBayAddr[bid];

    pBayInfoRsp->bayNumber = bid;

    /* Translate devStat values from proc (PI) to X1 constants */
    switch (pBayInfoCache->pdd.devStat)
    {
        case PD_NONX:
            pBayInfoRsp->devStat = X1_BAY_DEVSTAT_NON_EXISTENT;
            break;

        case PD_INOP:
            pBayInfoRsp->devStat = X1_BAY_DEVSTAT_INOPERATIVE;
            break;

        case PD_OP:
            pBayInfoRsp->devStat = X1_BAY_DEVSTAT_OK;
            break;

        default:
            pBayInfoRsp->devStat = X1_BAY_DEVSTAT_NON_EXISTENT;
    }

    memcpy(&pBayInfoRsp->prodId, pBayInfoCache->pdd.prodID, X1_PRODID_LEN);
    memcpy(&pBayInfoRsp->vendId, pBayInfoCache->pdd.vendID, X1_VENDID_LEN);
    memcpy(&pBayInfoRsp->rev, pBayInfoCache->pdd.prodRev, X1_REV_LEN);
    memcpy(&pBayInfoRsp->serial, pBayInfoCache->pdd.serial, X1_SERIAL_LEN);

    pBayInfoRsp->wwn = pBayInfoCache->pdd.wwn;

#if defined(MODEL_7000) || defined(MODEL_4700)
    if (pBayInfoCache->pdd.devType != PD_DT_ISE_SES)
    {
        fprintf(stderr, "%s: Unexpected bay type=%d\n",
                __FUNCTION__, pBayInfoCache->pdd.devType);
    }
#else  /* MODEL_7000 || MODEL_4700 */
    if (pBayInfoCache->pdd.devType == PD_DT_SBOD_SES)
    {
        /*
         * Xyratex enclosure type box. Put the information into
         * the two bytes for the vendor unique items. There is one
         * status for each port that will be combined in the loopStatus
         * field and the extended status for the ioModuleStatus field.
         */

        /* Get the ioModuleStatus1 from the SES Page2 Control Element data. */
        if (GetSESControlElement(bid, SES_ET_SBOD_STAT, 1, &page2Element) == GOOD)
        {
            pBayInfoRsp->ioModuleStatus1 = page2Element.Ctrl.SBODStat.ExtStatus & SES_C0XS_ES_MASK;
            pBayInfoRsp->loopStatus = page2Element.CommonCtrl & SES_CC_STAT_MASK;
        }

        /* Get the ioModuleStatus2 from the SES Page2 Control Element data. */
        if (GetSESControlElement(bid, SES_ET_SBOD_STAT, 2, &page2Element) == GOOD)
        {
            pBayInfoRsp->ioModuleStatus2 = page2Element.Ctrl.SBODStat.ExtStatus & SES_C0XS_ES_MASK;
            pBayInfoRsp->loopStatus |= (page2Element.CommonCtrl & SES_CC_STAT_MASK) << 4;
        }
    }
    else
    {
        /* Get the loopStatus from the SES Page2 Control Element data. */
        if (GetSESControlElement(bid, SES_ET_LOOP_STAT, 1, &page2Element) == GOOD)
        {
            pBayInfoRsp->loopStatus = page2Element.Ctrl.Generic.Ctrl2;
        }

        /* Get the ioModuleStatus1 from the SES Page2 Control Element data. */
        if (GetSESControlElement(bid, SES_ET_CTRL_STAT, 1, &page2Element) == GOOD)
        {
            pBayInfoRsp->ioModuleStatus1 = page2Element.Ctrl.Generic.Ctrl2;
        }

        /* Get the ioModuleStatus2 from the SES Page2 Control Element data. */
        if (GetSESControlElement(bid, SES_ET_CTRL_STAT, 2, &page2Element) == GOOD)
        {
            pBayInfoRsp->ioModuleStatus2 = page2Element.Ctrl.Generic.Ctrl2;
        }
    }
#endif /* MODEL_7000 || MODEL_4700 */

    /* Initialize the path count and path array values. */
    pBayInfoRsp->pathCount = 0;
    memset(pBayInfoRsp->path, 0xFF, (sizeof(UINT8) * DEV_PATH_MAX));

    /*
     * Get the path count and path array for the bay. This information
     * is used to determine connectivity to the bay.
     */
    GetBayPathInfo(bid, pBayInfoRsp);

    /* Disk bay shelf ID is in the slot field in the proc Disk Bay Info. */
    pBayInfoRsp->shelfId = pBayInfoCache->pdd.slot;

    /* The data element below was added at X1_COMPATIBILITY = 0x14 */
    pBayInfoRsp->devType = pBayInfoCache->pdd.devType;

    /* No longer using the cache for this function. */
    CacheStateSetNotInUse(cacheDiskBaysState);
}

/*----------------------------------------------------------------------------
** Function:    GetBayPathInfo
**
** Description: Get the path count and path array info for a Disk Bay.
**              This is done by looking at the BE Device Path info for
**              each drive in the bay.
**
** Inputs:      bid         Bay ID
**              pBayInfo    Pointer to bay info structure where path info
**                          will be copied.
**
** Returns:     NONE
**--------------------------------------------------------------------------*/
static void GetBayPathInfo(UINT16 bid, X1_BAY_INFO_RSP *pBayInfo)
{
    MRGETEINFO_RSP *pPDiskInfoCache;
    MRGETBEDEVPATHS_RSP *pPathInfo = NULL;
    MRGETBEDEVPATHS_RSP_ARRAY *pPathList = NULL;
    UINT16      pid;
    UINT16      i;
    UINT16      j;

    /*
     * Wait for good cache status before continuing. Set the flag
     * to indicate that the cache is updating.
     */
    CacheStateWaitOkToUpdate(cachePhysicalDisksState);
    CacheStateSetUpdating(cachePhysicalDisksState);

    /* Get a pointer to the start of the PDisk cache. */
    pPDiskInfoCache = (MRGETEINFO_RSP *)cachePhysicalDisks;

    /* Get a pointers to the path info for all PDisks. */
    pPathInfo = (MRGETBEDEVPATHS_RSP *)cachePDiskPaths;
    pPathList = (MRGETBEDEVPATHS_RSP_ARRAY *)(pPathInfo->list);

    /* Look through the entire PDisk cache. */
    for (i = 0; i < PhysicalDisksCount(); i++)
    {
        /* If this PDisk is in the input bay, check the paths. */
        if (pPDiskInfoCache[i].pdd.ses == bid)
        {
            /* Get the ID for this PDisk. */
            pid = pPDiskInfoCache[i].pdd.pid;

            /* Search the path array for this PDisk. */
            for (j = 0; j < DEV_PATH_MAX; j++)
            {
                /* If the path is valid. */
                if (pPathList[pid].path[j] != DEV_PATH_NO_CONN)
                {
                    /*
                     * If this path element in the Bay Info structure
                     * is invalid then this is the first PDisk record
                     * we have seen with a valid path at this index.
                     * Copy the value into the output structureand
                     * increment the path count.
                     */
                    if (pBayInfo->path[j] == DEV_PATH_NO_CONN)
                    {
                        pBayInfo->path[j] = pPathList[pid].path[j];
                        pBayInfo->pathCount++;
                    }
                }
            }
        }
    }

    /* Done updating cache - set good status. */
    CacheStateSetUpdateDone(cachePhysicalDisksState);
}

/*----------------------------------------------------------------------------
** Function:    GetCachedBayMap
**
** Description: Copy the cached disk bay map into the caller's buffer.
**              This function copies the entire cached map.
**
** Returns:     NONE
**--------------------------------------------------------------------------*/
void GetCachedBayMap(void *pBuf)
{
    /*
     * Wait until the Disk Bay cache is not in the process of being
     * updated. Once it is in that state make sure it is set to in
     * use so a update doesn't start while it is being used.
     */
    CacheStateWaitUpdating(cacheDiskBaysState);
    CacheStateSetInUse(cacheDiskBaysState);

    /*
     * Copy the cached map into the buffer.
     * The size of the map in cache is different than
     * the size defined in the X1 packet. Copy only as much as
     * the X1 packet can handle. This assumes that the X1 map size
     * is less than or equal to the cache map size.
     */
    memcpy(pBuf, &cacheDiskBayMap, CACHE_SIZE_DISK_BAY_MAP);

    /* No longer using the cache for this function. */
    CacheStateSetNotInUse(cacheDiskBaysState);
}


/**
******************************************************************************
**
**  @brief  Get the disk bay info associated with the input WWN
**
**  @param  wwn - WWN of the bay
**
**  @param  pPInfoOut - Pointer to bay Info for the requested WWN.
**
**  @return Good if info returned, NULL if no info returned
**
******************************************************************************
**/
INT32 GetDiskBayInfoFromWwn(UINT64 wwn, MRGETEINFO_RSP *pPInfoOut)
{
    MRGETEINFO_RSP *pDiskBayInfoCache;
    MRGETEINFO_RSP *diskBayInfo = NULL;
    INT32       rc = GOOD;
    UINT32      count;

    /* Get a pointer to the start of the Disk Bay cache. */
    pDiskBayInfoCache = (MRGETEINFO_RSP *)cacheDiskBays;

    /*
     * Wait until the disk bay cache is not in the process of
     * being updated. Once it is in that state make sure it is set
     * to in use so a update doesn't start while it is being used.
     */
    CacheStateWaitUpdating(cacheDiskBaysState);
    CacheStateSetInUse(cacheDiskBaysState);

    /*
     * Loop through the disk bays and find the entry associated with
     * the input wwn. The disk bay Info cache (cacheDiskBays[])
     * is indexed by bay ID. There is no guarantee that the list
     * is packed or that all bids are used therefore the entire
     * list is searched until a match is found or the end of the
     * list is reached.
     */
    for (count = 0; count < cacheDiskBaysCount; count++)
    {
        if (pDiskBayInfoCache[count].pdd.wwn == wwn)
        {
            diskBayInfo = &pDiskBayInfoCache[count];
            break;
        }
    }

    /*
     * If the requested entry was found, copy it into the pointer
     * passed by the called.
     */
    if (diskBayInfo != NULL)
    {
        memcpy(pPInfoOut, diskBayInfo, sizeof(*diskBayInfo));
        rc = GOOD;
    }
    else
    {
        rc = ERROR;
    }

    /* No longer using the cache for this function. */
    CacheStateSetNotInUse(cacheDiskBaysState);

    return rc;
}


/**
******************************************************************************
**
**  @brief  Get the disk bay info associated with the input WWN with no wait
**
**  @param  wwn - WWN of the bay
**  @param  pPInfoOut - Pointer to bay Info for the requested WWN.
**
**  @return GOOD if info returned, ERROR if no info returned
**
******************************************************************************
**/
INT32 GetDiskBayInfoFromWwnNOW(UINT64 wwn, MRGETEINFO_RSP *pPInfoOut)
{
    MRGETEINFO_RSP *pDiskBayInfoCache;
    UINT32      count;

    if (!pPInfoOut || CacheStateUpdating(cacheDiskBaysState))
    {
        return ERROR;
    }

    /* Get a pointer to the start of the Disk Bay cache. */
    pDiskBayInfoCache = (MRGETEINFO_RSP *)cacheDiskBays;

    /*
     * Loop through the disk bays and find the entry associated with
     * the input wwn. The disk bay Info cache (cacheDiskBays[])
     * is indexed by bay ID. There is no guarantee that the list
     * is packed or that all bids are used therefore the entire
     * list is searched until a match is found or the end of the
     * list is reached.
     */
    for (count = 0; count < cacheDiskBaysCount; count++)
    {
        if (pDiskBayInfoCache[count].pdd.wwn == wwn)
        {
            *pPInfoOut = pDiskBayInfoCache[count];
            return GOOD;
        }
    }

    return ERROR;
}


/**
******************************************************************************
**
**  @brief  Retrieve disk bay info for a particular Bay ID
**
**  @param  bid -   bay ID
**  @param  pPInfoOut - Pointer to MRGETEINFO_RSP to receive info
**
**  @return GOOD if found, ERROR if not found
**
******************************************************************************
**/
INT32 GetDiskBayInfoFromBid(UINT16 bid, MRGETEINFO_RSP *pPInfoOut)
{
    MRGETEINFO_RSP *pDiskBayInfoCache;
    INT32       rc = ERROR;

    /*
     * Wait until the disk bay cache is not in the process of
     * being updated. Once it is in that state make sure it is set
     * to in use so a update doesn't start while it is being used.
     */
    CacheStateWaitUpdating(cacheDiskBaysState);
    CacheStateSetInUse(cacheDiskBaysState);

    /* Get a pointer to the start of the Disk Bay cache. */
    if (bid < MAX_DISK_BAYS)
    {
        pDiskBayInfoCache = (MRGETEINFO_RSP *)cacheDiskBayAddr[bid];
    }
    else
    {
        pDiskBayInfoCache = NULL;
        dprintf(DPRINTF_DEFAULT, "%s: bid %d out of range\n", __func__, bid);
    }

    /* If the pointer is valid continue. */
    if (pDiskBayInfoCache != NULL)
    {
        memcpy(pPInfoOut, pDiskBayInfoCache, sizeof(*pDiskBayInfoCache));
        rc = GOOD;
    }

    /* No longer using the cache for this function. */
    CacheStateSetNotInUse(cacheDiskBaysState);

    return rc;
}


/**
******************************************************************************
**
**  @brief  Retrieve disk bay info for a particular Bay ID with no wait
**
**  @param  bid -   bay ID
**  @param  pPInfoOut - Pointer to area to receive info
**
**  @return ERROR if no data returned, GOOD if data was returned
**
******************************************************************************
**/
INT32 GetDiskBayInfoFromBidNOW(UINT16 bid, MRGETEINFO_RSP *pPInfoOut)
{
    MRGETEINFO_RSP *pDiskBayInfoCache;

    if (!pPInfoOut || CacheStateUpdating(cacheDiskBaysState))
    {
        return ERROR;
    }

    /* Get a pointer to the start of the Disk Bay cache. */
    if (bid < MAX_DISK_BAYS)
    {
        pDiskBayInfoCache = (MRGETEINFO_RSP *)cacheDiskBayAddr[bid];
    }
    else
    {
        pDiskBayInfoCache = NULL;
        dprintf(DPRINTF_DEFAULT, "%s: bid %d out of range\n", __func__, bid);
    }

    if (!pDiskBayInfoCache)
    {
        return ERROR;
    }

    *pPInfoOut = *pDiskBayInfoCache;

    return GOOD;
}


/*----------------------------------------------------------------------------
** Function:    GetPortOnRemoteCN
**
** Description: Use Disk Bay Path info on the local and remote controllers
**              to determine back end connectivity for the input localPort.
**
** Inputs:      port        - Port on local controller
**
** Outputs:     pRemoteID   - Low byte is the Remote Controller Node ID.
**                            High byte is the remote port number.
**
** Returns:     none
**--------------------------------------------------------------------------*/
void GetPortOnRemoteCN(UINT8 port, UINT16 *pRemoteID)
{
    XIO_PACKET                 reqPkt;
    XIO_PACKET                 rspPkt;
    MRGETBEDEVPATHS_RSP       *pPathInfo;
    MRGETBEDEVPATHS_RSP_ARRAY *pPathList;
    MRGETBEDEVPATHS_RSP       *pPathInfoRmt;
    MRGETBEDEVPATHS_RSP_ARRAY *pPathListRmt;
    INT32                      rc = PI_ERROR;
    UINT32                     remoteSN = 0;
    INT16                      remoteIndex = 0;
    UINT16                     remotePort = 0;
    UINT16                     bay = 0;
    UINT8                      remoteCount = 0;
    UINT8                      bid = 0;
    UINT8                      portValue = 0;
    bool                       bayFound = FALSE;

    dprintf(DPRINTF_CACHE_REFRESH, "GetPortOnRemoteCN - ENTER\n");

    /*
     * Allocate memory for the request (header and data) and the
     * response header. Memory for the response data is allocated
     * in TunnelRequest().
     */
    reqPkt.pHeader = MallocWC(sizeof(*reqPkt.pHeader));
    reqPkt.pPacket = MallocWC(sizeof(PI_PROC_BE_DEVICE_PATH_REQ));
    rspPkt.pHeader = MallocWC(sizeof(*rspPkt.pHeader));
    rspPkt.pPacket = NULL;
    reqPkt.pHeader->packetVersion = 1;
    rspPkt.pHeader->packetVersion = 1;

    /* Fill in the request header */
    reqPkt.pHeader->commandCode = PI_PROC_BE_DEVICE_PATH_CMD;
    reqPkt.pHeader->length = sizeof(PI_PROC_BE_DEVICE_PATH_REQ);

    /* Fill in the request parms. */
    ((PI_PROC_BE_DEVICE_PATH_REQ *)(reqPkt.pPacket))->type = PATH_ENCLOSURES;
    ((PI_PROC_BE_DEVICE_PATH_REQ *)(reqPkt.pPacket))->format = FORMAT_PID_PATH_ARRAY;

    /* Loop through all the remote controllers. */
    while ((remoteSN = GetNextRemoteControllerSN(&remoteIndex)) != 0)
    {
        UINT8   retries = 2;      /* Ethernet, Fiber(1), Disk Quorum(2) */

        /*
         * Memory for the response data for the previous request was
         * allocated by PortServerCommandHandler(). Free it here before
         * making the tunnel request.
         */
        do
        {
            if (rc != PI_TIMEOUT)
            {
                Free(rspPkt.pPacket);
            }
            else
            {
                rspPkt.pPacket = NULL;
            }

            /* Send BE Device Path request to the remote controller. */
            rc = TunnelRequest(remoteSN, &reqPkt, &rspPkt);
        } while (rc != GOOD && (retries--) > 0);

        /* If the request was successful continue. */
        if (rc == PI_GOOD)
        {
            /*
             * Wait until the Disk Bay cache is not in the process of being
             * updated. Once it is in that state make sure it is set to in
             * use so a update doesn't start while it is being used.
             */
            CacheStateWaitUpdating(cacheDiskBaysState);
            CacheStateSetInUse(cacheDiskBaysState);

            /*
             * cacheDiskBayPaths contains data from MRGETBEDEVPATHS_RSP
             * which contains an array of MRGETBEDEVPATHS_RSP_ARRAY
             * structures. Within each MRGETBEDEVPATHS_RSP_ARRAY is a
             * path list showing which port(s) the bay is connected on.
             */
            pPathInfo = (MRGETBEDEVPATHS_RSP *)cacheDiskBayPaths;
            pPathList = (MRGETBEDEVPATHS_RSP_ARRAY *)(pPathInfo->list);

            /*
             * Initialize the bayFound flag. Get the Bay ID and associated
             * portValue for the first bay in the list attached to the
             * input port.
             */
            bayFound = FALSE;
            for (bay = 0; bay < MAX_DISK_BAYS; bay++)
            {
                /*
                 * If the Bay ID is 0xFFFF an empty entry in the list
                 * has been found, skip it.
                 */
                if (pPathList[bay].pid == 0xFFFF)
                {
                    continue;
                }

                /*
                 * Get the portValue associated with the input port on
                 * the current bay.
                 */
                portValue = pPathList[bay].path[port];

                /*
                 * If portValue is valid use this Bay ID. Indicate a
                 * valid Bay ID was found and break out of the loop.
                 */
                if (portValue != 0xFF)
                {
                    bid = pPathList[bay].pid;
                    bayFound = TRUE;

                    dprintf(DPRINTF_CACHE_REFRESH, "  local bid=%d  port=%d  portValue=%d\n",
                            bid, port, portValue);
                    break;
                }
            }

            /* Continue if a bay was found on the input port. */
            if (bayFound)
            {
                /* Get pointers to the remote controller path info. */
                pPathInfoRmt = (MRGETBEDEVPATHS_RSP *)(rspPkt.pPacket);
                pPathListRmt = (MRGETBEDEVPATHS_RSP_ARRAY *)(pPathInfoRmt->list);

                if (pPathInfoRmt != NULL)
                {
                    /* Search all Bays on the remote controller. */
                    for (bay = 0; bay < pPathInfoRmt->ndevs; bay++)
                    {
                        /* If the Bay ID is found on the remote controller. */
                        if (pPathListRmt[bay].pid == bid)
                        {
                            dprintf(DPRINTF_CACHE_REFRESH, "   Found remote bid=%d on SN=%d\n", bid, remoteSN);

                            /* Search the ports on the remote controller. */
                            for (remotePort = 0; remotePort < DEV_PATH_MAX; remotePort++)
                            {
                                /*
                                 * If the remote port value matches the local value
                                 * we are done. This remotePort is the one we are
                                 * looking for.
                                 */
                                if (pPathListRmt[bay].path[remotePort] == portValue)
                                {
                                    /*
                                     * Set this port number along with the remote
                                     * controller node ID in the RemoteID for this
                                     * controller.
                                     */
                                    pRemoteID[remoteCount] = GetCNIDFromSN(remoteSN) | (remotePort << 8);

                                    dprintf(DPRINTF_CACHE_REFRESH, "   portValue=%d on remotePort=%d\n",
                                            portValue, remotePort);

                                    /* Increment the controller count and break out. */
                                    remoteCount++;
                                    break;
                                }
                            }
                        }
                    }
                }
                else
                {
                    dprintf(DPRINTF_DEFAULT, "Null pPathInfoRmt, probably cannot connect to remote controller.\n");
                }
            }

            /* No longer using the cache for this function. */
            CacheStateSetNotInUse(cacheDiskBaysState);
            break;
        }                       /* END if TunnelRequest successful */
        else
        {
            dprintf(DPRINTF_CACHE_REFRESH, "  TunnelRequest FAILED rc=0x%X  ec=0x%X\n",
                    rc, rspPkt.pHeader->errorCode);
        }
    }                           /* END Loop through all the remote controllers. */

    /*
     * Free the request and response headers and packets.
     * If a timeout occurred keep the response packet around -
     * the timeout code will free it.
     */
    Free(reqPkt.pHeader);
    Free(reqPkt.pPacket);

    Free(rspPkt.pHeader);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPkt.pPacket);
    }

    dprintf(DPRINTF_CACHE_REFRESH, "GetPortOnRemoteCN - EXIT\n");
}


/*----------------------------------------------------------------------------
** Function:    RefreshDiskBays
**
** Inputs:      NONE
**
** Returns:     PI_GOOD, PI_ERROR
**--------------------------------------------------------------------------*/
INT32 RefreshDiskBays(void)
{
    MR_LIST_RSP *ptrList = NULL;
    MRGETEINFO_REQ *pMRPInPkt = NULL;
    UINT8      *pCacheEntry;
    INT32       rc = PI_GOOD;
    UINT16      count = 0;
    MRGETEINFO_RSP *pTmpPtr = NULL;
    UINT8       bayId;

    /*
     * dprintf(DPRINTF_CACHEMGR, "RefreshDiskBays - ENTER (0x%x)\n", cacheDiskBaysState);
     */

    if (PowerUpBEReady())
    {
        /*
         * Get the list of objects. Always start at the beginning and return
         * the entire list.
         */
        ptrList = CacheGetList(MRGETELIST);

        /* If we could not get the list, signal an error */
        if (ptrList == NULL)
        {
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
             * Get disk bay information for each bay in the list. Place
             * the info in the cache.
             */
            for (count = 0; count < ptrList->ndevs; count++)
            {
                pMRPInPkt->id = ptrList->list[count];

                pTmpPtr = (MRGETEINFO_RSP *)(cacheTempDiskBays + (sizeof(*pTmpPtr) * count));

                /* Try forever to get this MRP through. */
                rc = PI_ExecuteMRP(pMRPInPkt, sizeof(*pMRPInPkt), MRGETEINFO,
                                   pTmpPtr, sizeof(*pTmpPtr),
                                   CACHE_MAX_REFRESH_MRP_TIMEOUT,
                                   PI_COMMAND_RECORD_TIMEOUT_ACTION_OUTPUT_NONE);

                if (rc != PI_GOOD)
                {
                    /* Some sort of error occurred. */
                    dprintf(DPRINTF_DEFAULT, "  MRP error: bay=%d status=0x%02hhX\n",  pMRPInPkt->id, pTmpPtr->header.status);
                }
            }

            if (rc == PI_GOOD)
            {
                /*
                 * Wait for good cache status before continuing. Set the flag
                 * to indicate that the cache is updating.
                 */
                CacheStateWaitOkToUpdate(cacheDiskBaysState);
                CacheStateSetUpdating(cacheDiskBaysState);

                /* Clear out the cache information. */
                memset(cacheDiskBays, 0x00, CACHE_SIZE_DISK_BAYS);
                memset(cacheDiskBayAddr, 0x00, CACHE_SIZE_DISK_BAYS_ADDR);
                memset(cacheDiskBayMap, 0x00, CACHE_SIZE_DISK_BAY_MAP);

                memset(cacheFibreBayMap, 0x00, CACHE_SIZE_DISK_BAY_MAP);
                memset(cacheSATABayMap, 0x00, CACHE_SIZE_DISK_BAY_MAP);

                /* Get the disk bays count from the PI_GetList call. */
                cacheDiskBaysCount = ptrList->ndevs;

                /* Swap the pointers. */
                SwapPointersDiskBays();
                pCacheEntry = cacheDiskBays;

                /* Fill out the map. */
                for (count = 0; count < cacheDiskBaysCount; count++)
                {
                    bayId = ptrList->list[count];

                    /* Map of existing Disk Bays */
                    cacheDiskBayMap[bayId / 8] |= (1 << (bayId & 7));


                    /* If this is a fibre bay add it to the Fibre Bay Map */
                    if ((((MRGETEINFO_RSP *)pCacheEntry)->pdd.devType == PD_DT_FC_SES) ||
                        (((MRGETEINFO_RSP *)pCacheEntry)->pdd.devType == PD_DT_SBOD_SES))
                    {
                        cacheFibreBayMap[bayId / 8] |= (1 << (bayId & 7));
                    }

                    /* If this is a SATA bay add it to the SATA Bay Map */
                    else if (((MRGETEINFO_RSP *)pCacheEntry)->pdd.devType == PD_DT_SATA_SES)
                    {
                        cacheSATABayMap[bayId / 8] |= (1 << (bayId & 7));
                    }

                    /*
                     * Save the address of this cache entry in the lookup
                     * table cacheDiskBayAddr[]. Advance the cache
                     * pointer to the place where the next entry will go.
                     * cacheDiskBayAddr[] is NOT packed.
                     */
                    cacheDiskBayAddr[bayId] = pCacheEntry;
                    pCacheEntry += sizeof(*pTmpPtr);
                }

                /* Done updating cache - set good status. */
                CacheStateSetUpdateDone(cacheDiskBaysState);
            }

            /* Free input packet, all the information has been gathered. */
            Free(pMRPInPkt);

            /* Done with the list so it can now be freed. */
            Free(ptrList);
        }

        /*
         * We are done getting the Disk Bay Info for all Bays.
         * Now get the back end path info for all Bays. This only
         * requires one MRP call.
         */
        RefreshBEDevicePaths(PATH_ENCLOSURES);
    }

    else
    {
        rc = PI_ERROR;
    }

    /*
     * dprintf(DPRINTF_CACHEMGR, "RefreshDiskBays - EXIT (0x%x)\n", K_timel);
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
