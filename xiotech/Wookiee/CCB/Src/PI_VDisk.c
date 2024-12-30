/* $Id: PI_VDisk.c 161373 2013-07-29 15:13:39Z marshall_midden $*/
/*===========================================================================
** FILE NAME:       PI_VDisk.c
** MODULE TITLE:    Packet Handler for Virtual Disk Commands
**
** DESCRIPTION:     Handler functions for PDisk request packets.
**
** Copyright (c) 2001-2009 XIOtech Corporation. All rights reserved.
**==========================================================================*/
#include "PI_VDisk.h"

#include "CacheManager.h"
#include "CacheVDisk.h"
#include "CacheRaid.h"
#include "CacheServer.h"
#include "CmdLayers.h"
#include "debug_files.h"
#include "LOG_Defs.h"
#include "misc.h"
#include "MR_Defs.h"
#include "PacketInterface.h"
#include "PI_CmdHandlers.h"
#include "PI_Target.h"
#include "PI_Utils.h"
#include "PI_VDisk.h"
#include "PktCmdHdl.h"
#include "PortServer.h"
#include "quorum_utils.h"
#include "RL_RDD.h"
#include "serial_num.h"
#include "sm.h"
#include "vdd.h"
#include "XIO_Std.h"
#include "ddr.h"
#include "LargeArrays.h"

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static UINT16 PI_VdiskGetAttributeChange(UINT16 vid, UINT16 newVdiskAtr);
static INT32 PI_VDiskInfo_VER2(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket);
static INT32 PI_VDiskInfo_VER3(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket);
static INT32 PI_VDisks_VER2(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket);
static INT32 PI_VDisks_VER3(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket);

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
** Function:    PI_VDisks - FROM CACHE
**
** Description: Virtual Disks Request Handler
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
** WARNING:     This function returns information from cache. It does NOT
**              issue MRPs.
**--------------------------------------------------------------------------*/
INT32 PI_VDisksCache(UNUSED XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    PI_VDISKS_RSP *ptrOutPkt;
    UINT32      outPktSize;
    UINT32      vdisk_info_size;
#ifndef NO_VDISK_CACHE
    UINT8       retrycount = 5;
#else   /* NO_VDISK_CACHE */
    struct DMC *entry = DMC_CCB + CCB_DMC_vdiskcache;
    UINT32      recheck_vdisk_info_size;
#endif  /* NO_VDISK_CACHE */
    PI_VDISK_INFO2_RSP *ptrInfoPkt;
    UINT32      count;
    UINT32      len;
    UINT8      *vinfo;

    dprintf(DPRINTF_PI_COMMANDS, "PI_VDisks - from CACHE\n");

    /*
     * Wait until the Virtual Disks cache is not in
     * the process of being updated. Once it is in
     * that state make sure it is set to in use so
     * a update doesn't start while it is being
     * used.
     */
#ifndef NO_VDISK_CACHE
    CacheStateWaitUpdating(cacheVirtualDisksState);
    CacheStateSetInUse(cacheVirtualDisksState);

    while (PI_IsCacheDirty(1 << PI_CACHE_INVALIDATE_VDISK) && retrycount)
    {
        CacheStateSetNotInUse(cacheVirtualDisksState);
        TaskSleepMS(200);
        retrycount--;
        CacheStateSetInUse(cacheVirtualDisksState);
    }
    if (retrycount == 0)
    {
        fprintf(stderr, "%s ATTENTION:retrycount reached zero\n", __FUNCTION__);
    }
#else   /* NO_VDISK_CACHE */
    /* Get memory lock and wait if busy. */
    Wait_DMC_Lock(entry);
#endif  /* NO_VDISK_CACHE */

    /*
     * Calculate the size of the output packet. This will be the size
     * of the Virtual Disks (multiple devices) response packet plus the
     * size of a Virtual Disk Info (single device) response packet for each
     * device.
     */
    vdisk_info_size = CalcSizeVDisksCached();
#ifdef NO_VDISK_CACHE
  retry_get:
    /* Done with lock. */
    Free_DMC_Lock(entry);
#endif  /* NO_VDISK_CACHE */
    outPktSize = sizeof(*ptrOutPkt) + vdisk_info_size;

    ptrOutPkt = MallocWC(outPktSize);

#ifdef NO_VDISK_CACHE
    /* Get memory lock and wait if busy. */
    Wait_DMC_Lock(entry);

    recheck_vdisk_info_size = CalcSizeVDisksCached();
    if (recheck_vdisk_info_size != vdisk_info_size)
    {
        Free(outPktSize);                   /* Free does not task switch. */
        vdisk_info_size = recheck_vdisk_info_size;
        goto retry_get;
    }
#endif  /* NO_VDISK_CACHE */

    /*
     * Copy the number of devices into the output packet along
     * with the data for each VDisk.
     */
    ptrOutPkt->count = VirtualDisksCount();

#ifndef NO_VDISK_CACHE
    VirtualDisksGet(&ptrOutPkt->vdisks);
    CacheStateSetNotInUse(cacheVirtualDisksState);
#else   /* NO_VDISK_CACHE */
    memcpy(&ptrOutPkt->vdisks, &cacheVDiskBuffer_DMC.cacheVDiskBuffer_DMC, vdisk_info_size);

    /* Done with lock. */
    Free_DMC_Lock(entry);
#endif  /* NO_VDISK_CACHE */

    /*
     * This logic is introduced because of the earlier CCB VDISKS command
     * implementantion is correcting the last access time. So we have to
     * do the same here
     */

    vinfo = (UINT8 *)&ptrOutPkt->vdisks;
    for (count = 0; count < ptrOutPkt->count; count++)
    {
        ptrInfoPkt = (PI_VDISK_INFO2_RSP *)vinfo;

        len = ptrInfoPkt->header.len;
        /* Update the time of last access. The time is idle seconds. */
        if (ptrInfoPkt->lastAccess == -1)
        {
            ptrInfoPkt->lastAccess = 0;
        }
        else
        {
            ptrInfoPkt->lastAccess = RTC_GetLongTimeStamp() - ptrInfoPkt->lastAccess;
        }
        vinfo += len;
    }

    /*
     * Attach the MRP return data packet to the main response packet.
     * Fill in the header length and status fields.
     */
    pRspPacket->pPacket = (UINT8 *)ptrOutPkt;
    pRspPacket->pHeader->length = outPktSize;
    pRspPacket->pHeader->status = GOOD;
    pRspPacket->pHeader->errorCode = GOOD;

    return GOOD;
}

/*----------------------------------------------------------------------------
** Function:    PI_VDisks_VER2 - MRP Requests
**
** Description: Virtual Disks Request Handler for versions 2 and lower.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
static INT32 PI_VDisks_VER2(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    MR_LIST_RSP *ptrVList = NULL;
    MR_LIST_RSP *ptrRList = NULL;
    UINT16      count = 0;
    PI_VDISKS_RSP *ptrOutPkt = NULL;
    UINT32      outPktSize = 0;
    UINT32      maxPktSize = 0;
    PI_VDISK_INFO_REQ *pInPkt = NULL;
    PI_VDISK_INFO2_RSP *pOutPkt = NULL;
    UINT8      *pBuf = NULL;
    UINT32      bufSize = 0;
    INT32       errorCode = 0;
    UINT32      i;

    dprintf(DPRINTF_PI_COMMANDS, "PI_VDisks VER2 - MRPs issued\n");

    /*
     * Get the list of objects. Always start at the beginning and return
     * the entire list.
     */
    ptrVList = PI_GetList(0, (MRGETVLIST | GET_LIST));
    ptrRList = PI_GetList(0, (MRGETRLIST | GET_NUMBER_ONLY));

    /* If we could get the lists use them, otherwise signal an error */
    if (ptrVList != NULL && ptrRList != NULL)
    {
        /* Add the static data size for each VDISK. */
        outPktSize += (sizeof(*pOutPkt) * ptrVList->ndevs);

        /*
         * Add the size of a UINT16 for each of the raids (we only
         * retrieve the ID values when getting VDISKS).
         *
         * For buffer space in case there are copies that complete while
         * retrieving this information, multiply the number of raid
         * devices by 2.
         */
        outPktSize += (sizeof(UINT16) * (ptrRList->ndevs * 2));

        /*
         * Calculate the maximum size for a given VDISK INFO request packet.
         *
         * For buffer space in case there are copies that complete while
         * retrieving this information, multiply the number of raid
         * devices by 2.
         */
        maxPktSize = (sizeof(*pOutPkt) + (sizeof(UINT16) * (ptrRList->ndevs * 2)));

        /*
         * The outPktSize variable currently contains the size (in bytes) of
         * the virtual disk information (including raid list) for all the
         * virtual disks. For our response we need to include the size of the
         * PI_VDISKS_RSP packet.
         */
        outPktSize += sizeof(*ptrOutPkt);

        /*
         * The input packet will be used in the calls to the VDisk Info MRP.
         * The output packet will be used in the calls to the VDisk Info MRP.
         * The second output packet will contain the VDISKS information.
         */
        pInPkt = MallocWC(sizeof(*pInPkt));
        pOutPkt = MallocSharedWC(maxPktSize);
        ptrOutPkt = MallocWC(outPktSize);

        /* Save the number of devices in the output packet */
        ptrOutPkt->count = ptrVList->ndevs;

        /*
         * The pBuf variable holds the location in the output packet
         * buffer to where the next VDISK INFOMATION should be placed.
         * This pointer moves through the output packet buffer for
         * each request to the VDISK INFO MRP.
         */
        pBuf = ptrOutPkt->vdisks;

        /*
         * At this time the outPktSize contains the entire output packet
         * buffer size, including the PI_VDISKS_RSP structure size. We
         * need a variable that contains the size of the buffer remaining
         * for retrieving VDISK information. After each call to the VDISK
         * INFO MRP the buffer size will be reduced by the size of the
         * information returned by the MRP.
         */
        bufSize = outPktSize - sizeof(*ptrOutPkt);

        /*
         * Loop through each of the IDs in the VDISK ID LIST and make
         * a call to the VDISK INFO MRP.
         */
        for (count = 0; count < ptrVList->ndevs; count++)
        {
            /* Setup the input packet to have the correct VDISK ID. */
            pInPkt->id = ptrVList->list[count];

            rc = PI_ExecMRP(pInPkt, sizeof(*pInPkt), MRGETVINFO,
                            pOutPkt, maxPktSize, GetGlobalMRPTimeout());

            if (rc == PI_GOOD)
            {
                if (pOutPkt->header.len <= bufSize)
                {
                    if (pReqPacket->pHeader->packetVersion == 2)
                    {
                        /*
                         * Update the time of last access. The time currently got
                         * is in number of idle seconds.
                         */
                        if (pOutPkt->lastAccess == -1)
                        {
                            pOutPkt->lastAccess = 0;
                        }
                        else
                        {
                            pOutPkt->lastAccess = RTC_GetLongTimeStamp() - pOutPkt->lastAccess;
                        }

                        /*
                         * Copy the data retrieved throught the MRP into the
                         * output packet that contains the information for
                         * all the VDISKS combined.
                         */
                        memcpy(pBuf, pOutPkt, pOutPkt->header.len);

                        /*
                         * Decrement the remaining buffer size to not include this
                         * virtual disk information.
                         */
                        bufSize -= pOutPkt->header.len;

                        /*
                         * Move the output packet pointer to be past the VDISK info
                         * we just retrieved.
                         */
                        pBuf += pOutPkt->header.len;
                    }
                    else if (pReqPacket->pHeader->packetVersion == 1 ||
                             pReqPacket->pHeader->packetVersion == 0)
                    {
                        /*
                         * Update the length of the packet to correspond to the
                         * version 1 packet definition.
                         */
                        pOutPkt->header.len = sizeof(PI_VDISK_INFO_RSP) +
                            ((pOutPkt->raidCnt + pOutPkt->draidCnt) * 2);

                        /*
                         * Copy the data retrieved throught the MRP into the
                         * output packet that contains the information for
                         * all the VDISKS combined.
                         */
                        memcpy(pBuf, pOutPkt, sizeof(PI_VDISK_INFO_RSP));
                        for (i = 0; i < (UINT32)pOutPkt->raidCnt + pOutPkt->draidCnt; i++)
                        {
                            ((PI_VDISK_INFO_RSP *)pBuf)->rid[i] = pOutPkt->rid[i];
                        }

                        /*
                         * Decrement the remaining buffer size to not include this
                         * virtual disk information.
                         */
                        bufSize -= pOutPkt->header.len;

                        /*
                         * Move the output packet pointer to be past the VDISK info
                         * we just retrieved.
                         */
                        pBuf += pOutPkt->header.len;
                    }
                }
                else
                {
                    /*
                     * The amount of data returned in this request would
                     * exceed the buffer space so return the too much
                     * data error code.
                     */
                    rc = PI_ERROR;
                    errorCode = DETOOMUCHDATA;
                }
            }
            else
            {
                /*
                 * Some sort of error occurred. We cannot even tolerate a
                 * "too much data" error in this case since we should have
                 * done the calcualation correctly before starting to retrieve
                 * the virtual disk information.
                 */
                errorCode = pOutPkt->header.status;
                break;
            }
        }
    }
    else
    {
        /*
         * One or both of the lists could not be retrieved so
         * an error must be returned.
         */
        rc = PI_ERROR;
    }

    /* Free the allocated memory */
    Free(ptrVList);
    Free(ptrRList);
    Free(pInPkt);

    if (rc != PI_TIMEOUT)
    {
        Free(pOutPkt);
    }

    if (rc == PI_GOOD)
    {
        /*
         * Attach the MRP return data packet to the main response packet.
         * Fill in the header length and status fields.
         */
        pRspPacket->pPacket = (UINT8 *)ptrOutPkt;
        pRspPacket->pHeader->length = outPktSize;
        pRspPacket->pHeader->status = rc;
        pRspPacket->pHeader->errorCode = rc;
    }
    else
    {
        Free(ptrOutPkt);

        /* Indicate an error condition and no return data in the header. */
        pRspPacket->pHeader->length = 0;
        pRspPacket->pHeader->status = rc;
        pRspPacket->pHeader->errorCode = errorCode;
    }

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PI_VDisks_VER3 - MRP Requests
**
** Description: Virtual Disks Request Handler for versiont 3
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
static INT32 PI_VDisks_VER3(UNUSED XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    MR_LIST_RSP *ptrVList = NULL;
    MR_LIST_RSP *ptrRList = NULL;
    UINT16      count = 0;
    PI_VDISKS_RSP *ptrOutPkt = NULL;
    UINT32      outPktSize = 0;
    UINT32      maxPktSize = 0;
    PI_VDISK_INFO_REQ *pInPkt = NULL;
    PI_VDISK_INFO3_RSP *pOutPkt = NULL;
    UINT8      *pBuf = NULL;
    UINT32      bufSize = 0;
    INT32       errorCode = 0;
    MREXTEXTEND_VINFO_RSP *pExtndOutPkt = NULL;
    UINT32      extndOutPktSize = 0;

    dprintf(DPRINTF_PI_COMMANDS, "PI_VDisks VER3 - MRPs issued\n");

    /*
     * Get the list of objects. Always start at the beginning and return
     * the entire list.
     */
    ptrVList = PI_GetList(0, (MRGETVLIST | GET_LIST));
    ptrRList = PI_GetList(0, (MRGETRLIST | GET_NUMBER_ONLY));

    /* If we could get the lists use them, otherwise signal an error */
    if (ptrVList != NULL && ptrRList != NULL)
    {
        /*
         * Add the static data size for each VDISK.
         */
        outPktSize += (sizeof(*pOutPkt) * ptrVList->ndevs);
        /*
         * Add the size of a UINT16 for each of the raids (we only
         * retrieve the ID values when getting VDISKS).
         *
         * For buffer space in case there are copies that complete while
         * retrieving this information, multiply the number of raid
         * devices by 2.
         */
        outPktSize += (sizeof(UINT16) * (ptrRList->ndevs * 2));
        /*
         * Add the size of MREXTENDED_VINFO_RSP_PKT for each vdisk
         * as extended vinfo request.
         */
        outPktSize += (ptrVList->ndevs * sizeof(MREXTENDED_VINFO_RSP_PKT));

        /*
         * The outPktSize variable currently contains the size (in bytes) of
         * the virtual disk information (including raid list) for all the
         * virtual disks. For our response we need to include the size of the
         * PI_VDISKS_RSP packet.
         */
        outPktSize += sizeof(*ptrOutPkt);

        /*
         * Calculate the maximum size for a given VDISK INFO request packet.
         *
         * For buffer space in case there are copies that complete while
         * retrieving this information, multiply the number of raid
         * devices by 2.
         */
        maxPktSize = (sizeof(*pOutPkt) +
                      (sizeof(UINT16) * (ptrRList->ndevs * 2)) +
                      sizeof(MREXTENDED_VINFO_RSP_PKT));

        /*
         * The input packet will be used in the calls to the VDisk Info
         * MRP.
         *
         * The output packet will be used in the calls to the VDisk Info
         * MRP.
         *
         * The second output packet will contain the VDISKS information.
         */
        pInPkt = MallocWC(sizeof(*pInPkt));
        pOutPkt = MallocSharedWC(maxPktSize);
        ptrOutPkt = MallocWC(outPktSize);

        /* Save the number of devices in the output packet */
        ptrOutPkt->count = ptrVList->ndevs;

        /*
         * The pBuf variable holds the location in the output packet
         * buffer to where the next VDISK INFOMATION should be placed.
         * This pointer moves through the output packet buffer for
         * each request to the VDISK INFO MRP.
         */
        pBuf = ptrOutPkt->vdisks;
        /*
         * At this time the outPktSize contains the entire output packet
         * buffer size, including the PI_VDISKS_RSP structure size. We
         * need a variable that contains the size of the buffer remaining
         * for retrieving VDISK information. After each call to the VDISK
         * INFO MRP the buffer size will be reduced by the size of the
         * information returned by the MRP.
         */
        bufSize = outPktSize - sizeof(*ptrOutPkt);
        /*
         * Loop through each of the IDs in the VDISK ID LIST and make
         * a call to the VDISK INFO MRP.
         */
        for (count = 0; count < ptrVList->ndevs; count++)
        {
            /* Setup the input packet to have the correct VDISK ID. */
            pInPkt->id = ptrVList->list[count];

            rc = PI_ExecMRP(pInPkt, sizeof(*pInPkt), MRGETVINFO,
                            pOutPkt, maxPktSize, GetGlobalMRPTimeout());

            if (rc == PI_GOOD)
            {
                if (pOutPkt->vinfo.header.len <= bufSize)
                {
                    MRGETVINFO_RSP *pVinfo = NULL;

                    /*
                     * Update the time of last access. The time currently got
                     * is in number of idle seconds.
                     */
                    if (pOutPkt->vinfo.lastAccess == -1)
                    {
                        pOutPkt->vinfo.lastAccess = 0;
                    }
                    else
                    {
                        pOutPkt->vinfo.lastAccess = RTC_GetLongTimeStamp() - pOutPkt->vinfo.lastAccess;
                    }

                    /*
                     * Copy the data retrieved throught the MRP into the
                     * output packet that contains the information for
                     * all the VDISKS combined.
                     */
                    memcpy(pBuf, pOutPkt, pOutPkt->vinfo.header.len);

                    pVinfo = (MRGETVINFO_RSP *)pBuf;

                    pVinfo->header.len += sizeof(MREXTENDED_VINFO_RSP_PKT);
                    /*
                     * Decrement the remaining buffer size to not include this
                     * virtual disk information.
                     */
                    bufSize -= pOutPkt->vinfo.header.len;
                    /*
                     * Move the output packet pointer to be past the VDISK info
                     * we just retrieved.
                     */
                    pBuf += pOutPkt->vinfo.header.len;

                    /*
                     * Send the extended vdisk info request and attach to
                     * the packet.
                     */
                    extndOutPktSize = sizeof(*pExtndOutPkt);
                    pExtndOutPkt = MallocSharedWC(extndOutPktSize);

                    rc = PI_ExecMRP(pInPkt, sizeof(*pInPkt), MRGETEXTENDVINFO,
                                    pExtndOutPkt, extndOutPktSize, GetGlobalMRPTimeout());
                    if (rc == PI_GOOD)
                    {

                        pOutPkt->vinfo.header.len += sizeof(MREXTENDED_VINFO_RSP_PKT);

                        /*
                         * Copy the data retrieved throught the MRP into the
                         * output packet that contains the information for
                         * all the VDISKS combined.
                         */
                        memcpy(pBuf, &(pExtndOutPkt->data), sizeof(MREXTENDED_VINFO_RSP_PKT));

                        /*
                         * Decrement the remaining buffer size to not include this
                         * virtual disk information.
                         */
                        bufSize -= sizeof(MREXTENDED_VINFO_RSP_PKT);
                        /*
                         * Move the output packet pointer to be past the VDISK info
                         * we just retrieved.
                         */
                        pBuf += sizeof(MREXTENDED_VINFO_RSP_PKT);
                    }
                    if (pExtndOutPkt)
                    {
                        DelayedFree(MRGETEXTENDVINFO, pExtndOutPkt);
                    }
                }
                else
                {
                    /*
                     * The amount of data returned in this request would
                     * exceed the buffer space so return the too much
                     * data error code.
                     */
                    rc = PI_ERROR;
                    errorCode = DETOOMUCHDATA;
                }
            }
            else
            {
                /*
                 * Some sort of error occurred. We cannot even tolerate a
                 * "too much data" error in this case since we should have
                 * done the calcualation correctly before starting to retrieve
                 * the virtual disk information.
                 */
                errorCode = pOutPkt->vinfo.header.status;
                break;
            }
        }                       /* end of for loop */
    }
    else
    {
        /*
         * One or both of the lists could not be retrieved so
         * an error must be returned.
         */
        rc = PI_ERROR;
    }

    /* Free the allocated memory */
    Free(ptrVList);
    Free(ptrRList);
    Free(pInPkt);

    if (rc != PI_TIMEOUT)
    {
        Free(pOutPkt);
    }

    if (rc == PI_GOOD)
    {
        /*
         * Attach the MRP return data packet to the main response packet.
         * Fill in the header length and status fields.
         */
        pRspPacket->pPacket = (UINT8 *)ptrOutPkt;
        pRspPacket->pHeader->length = outPktSize;
        pRspPacket->pHeader->status = rc;
        pRspPacket->pHeader->errorCode = rc;
    }
    else
    {
        Free(ptrOutPkt);

        /* Indicate an error condition and no return data in the header. */
        pRspPacket->pHeader->length = 0;
        pRspPacket->pHeader->status = rc;
        pRspPacket->pHeader->errorCode = errorCode;
    }

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PI_VDisks
**
** Description: Virtual Disks Request Handler
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_VDisks(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;

    if (pReqPacket->pHeader->packetVersion <= 2)
    {
        rc = PI_VDisks_VER2(pReqPacket, pRspPacket);
    }
    else if (pReqPacket->pHeader->packetVersion == 3)
    {
        rc = PI_VDisks_VER3(pReqPacket, pRspPacket);
    }
    else
    {
        /*
         * TODO: need to fill the packet with error cause
         * like version out of scope
         */
        rc = PI_ERROR;
    }
    return rc;
}

/*----------------------------------------------------------------------------
** Function:    PI_VDiskInfo
**
** Description: Get info for the requested virtual disk
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_VDiskInfo(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;

    if (pReqPacket->pHeader->packetVersion <= 2)
    {
        rc = PI_VDiskInfo_VER2(pReqPacket, pRspPacket);
    }
    else if (pReqPacket->pHeader->packetVersion == 3)
    {
        rc = PI_VDiskInfo_VER3(pReqPacket, pRspPacket);
    }
    else
    {
        /*
         * TODO: need to fill the packet with error cause
         * version out of scope
         */
        rc = PI_ERROR;
    }
    return rc;
}

/*----------------------------------------------------------------------------
** Function:    PI_VDiskInfo_VER2
**
** Description: Get info for the requested virtual disk for packet version
**              below 2
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
static INT32 PI_VDiskInfo_VER2(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    PI_VDISK_INFO2_RSP *ptrOutPkt = NULL;
    PI_VDISK_INFO_RSP *ptrOutPkt1 = NULL;
    UINT32      outPktSize = 0;
    UINT32      numRaids;
    UINT32      i;
    INT32       rc = PI_GOOD;

    /*
     * Start by assuming that the VDisk has the maximum RAIDs. This stops repeated calls.
     * If the MRP fails because not enough space was allocated,
     * a new allocation is done based on the actual number of RAIDs. This
     * is done in a loop in case the size of the list changes between
     * ExecMRP() calls.
     */
    numRaids = MAX_SEGMENTS+1;      /* There is an off-by-one in check. */

    do
    {
        /*
         * Determine the output packet size based on the number of
         * RAIDs in the VDisk.
         */
        outPktSize = (sizeof(*ptrOutPkt) + (numRaids * sizeof(UINT16)));

        /*
         * If an output packet was previously allocated, free it before
         * allocating a new one.
         */
        Free(ptrOutPkt);

        ptrOutPkt = MallocSharedWC(outPktSize);

        rc = PI_ExecMRP(pReqPacket->pPacket, pReqPacket->pHeader->length,
                        MRGETVINFO, ptrOutPkt, outPktSize, GetGlobalMRPTimeout());

        /*
         * Save the number of devices in case we need to make the
         * request again.
         */
        if (rc == PI_ERROR && ptrOutPkt->header.status == DETOOMUCHDATA)
        {
            /*
             * To get the number of devices make sure we account for
             * both raids and deferred raids.
             */
            numRaids = ptrOutPkt->raidCnt + ptrOutPkt->draidCnt;
        }
    } while (rc == PI_ERROR && ptrOutPkt->header.status == DETOOMUCHDATA);

    if (pReqPacket->pHeader->packetVersion == 1 ||
        pReqPacket->pHeader->packetVersion == 0)
    {
        ptrOutPkt1 = (PI_VDISK_INFO_RSP *)ptrOutPkt;

        for (i = 0; i < numRaids; i++)
        {
            ptrOutPkt1->rid[i] = ptrOutPkt->rid[i];
        }

        outPktSize = (sizeof(*ptrOutPkt1) + (numRaids * sizeof(UINT16)));
        ptrOutPkt1->header.len = outPktSize;
    }
    else if (pReqPacket->pHeader->packetVersion == 2)
    {
        /*
         * Update the time of last access. The time currently got
         * is in number of idle seconds.
         */
        if (ptrOutPkt->lastAccess == -1)
        {
            ptrOutPkt->lastAccess = 0;
        }
        else
        {
            ptrOutPkt->lastAccess = RTC_GetLongTimeStamp() - ptrOutPkt->lastAccess;
        }
    }

    /*
     * Attach the MRP return data packet to the main response packet.
     * Fill in the header length and status fields.
     */
    pRspPacket->pPacket = (UINT8 *)ptrOutPkt;
    pRspPacket->pHeader->length = outPktSize;
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = ptrOutPkt->header.status;

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PI_VDiskInfo_VER3
**
** Description: Get info for the requested virtual disk for Packet version 3
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
static INT32 PI_VDiskInfo_VER3(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    PI_VDISK_INFO3_RSP *ptrOutPkt = NULL;
    UINT32      outPktSize = 0;
    UINT32      numRaids;
    INT32       rc = PI_GOOD;
    MREXTEXTEND_VINFO_RSP *pExtndOutPkt = NULL;
    UINT32      extndOutPktSize = 0;
    MREXTENDED_VINFO_RSP_PKT *ptrExtVinfoPkt = NULL;

    /*
     * Start by assuming that VDisk has 2 RAIDs. If MRP fails because not
     * enough space was allocated, a new allocation is done based on the actual
     * number of RAIDs. This is done in a loop in case the size of the list
     * changes between ExecMRP() calls.
     */
    numRaids = 2;

    do
    {
        /* Determine output packet size based on number of RAIDs in the VDisk. */
        outPktSize = (sizeof(*ptrOutPkt) + (numRaids * sizeof(UINT16)) + sizeof(MREXTENDED_VINFO_RSP_PKT));

        /* If output packet was previously allocated, free it. */
        Free(ptrOutPkt);

        ptrOutPkt = MallocSharedWC(outPktSize);

        rc = PI_ExecMRP(pReqPacket->pPacket, pReqPacket->pHeader->length,
                        MRGETVINFO, ptrOutPkt, outPktSize, GetGlobalMRPTimeout());
        /* Save the number of devices in case we need to make request again. */
        if (rc == PI_ERROR && ptrOutPkt->vinfo.header.status == DETOOMUCHDATA)
        {
            /*
             * To get the number of devices make sure we account for
             * both raids and deferred raids.
             */
            numRaids = ptrOutPkt->vinfo.raidCnt + ptrOutPkt->vinfo.draidCnt;
        }
    } while (rc == PI_ERROR && ptrOutPkt->vinfo.header.status == DETOOMUCHDATA);

    /* Update time of last access. The time is in number of idle seconds. */
    if (ptrOutPkt->vinfo.lastAccess == -1)
    {
        ptrOutPkt->vinfo.lastAccess = 0;
    }
    else
    {
        ptrOutPkt->vinfo.lastAccess = RTC_GetLongTimeStamp() - ptrOutPkt->vinfo.lastAccess;
    }

    /* Send the extended vdisk info request and attach to the packet. */
    extndOutPktSize = sizeof(*pExtndOutPkt);
    pExtndOutPkt = MallocSharedWC(extndOutPktSize);

    rc = PI_ExecMRP(pReqPacket->pPacket, pReqPacket->pHeader->length,
                    MRGETEXTENDVINFO, pExtndOutPkt, extndOutPktSize,
                    GetGlobalMRPTimeout());
    if (rc == PI_GOOD)
    {
        ptrExtVinfoPkt = (MREXTENDED_VINFO_RSP_PKT *)(((UINT8 *)ptrOutPkt) + ptrOutPkt->vinfo.header.len);
        ptrExtVinfoPkt->breakTime = pExtndOutPkt->data.breakTime;
        ptrOutPkt->vinfo.header.len += sizeof(MREXTENDED_VINFO_RSP_PKT);
    }
    if (pExtndOutPkt)
    {
        DelayedFree(MRGETEXTENDVINFO, pExtndOutPkt);
    }

    /* Attach MRP return data packet to main response packet. Fill in fields. */
    pRspPacket->pPacket = (UINT8 *)ptrOutPkt;
    pRspPacket->pHeader->length = outPktSize;
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = ptrOutPkt->vinfo.header.status;

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PI_VDiskOwner
**
** Description: Get owner information for the requested virtual disk
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_VDiskOwner(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    PI_VDISK_OWNER_RSP *ptrOutPkt = NULL;
    UINT32      outPktSize;
    UINT16      ndevs = 0;
    INT32       rc = PI_GOOD;

    /*
     * If MRP fails because not enough space was allocated, a new allocation
     * is done based on the actual number of owners. This is done in a loop in
     * case the size of the list changes between ExecMRP() calls.
     *
     * Start with what fits in the 64 byte malloc size, subtract struct header,
     * and divide size of response structures, leaving 6.5 -- which is 6.
     */
    ndevs = (64 - sizeof(MRGETVIDOWNER_RSP)) / sizeof(MRGETVIDOWNER_RSP_INFO);

    do
    {
        /* Determine output packet size based on number of RAIDs in the VDisk. */
        outPktSize = sizeof(MRGETVIDOWNER_RSP) + (ndevs * sizeof(MRGETVIDOWNER_RSP_INFO));

        /* If output packet was previously allocated, free it. */
        if (ptrOutPkt != NULL)
        {
            Free(ptrOutPkt);
        }

        ptrOutPkt = MallocSharedWC(outPktSize);

        rc = PI_ExecMRP(pReqPacket->pPacket, pReqPacket->pHeader->length,
                        MRGETVIDOWNER, ptrOutPkt, outPktSize, GetGlobalMRPTimeout());

        if (rc == PI_ERROR && ptrOutPkt->header.status == DETOOMUCHDATA)
        {
            /* Save the number of devices to make the request again. */
            ndevs = ptrOutPkt->ndevs;
        }
    } while (rc == PI_ERROR && ptrOutPkt->header.status == DETOOMUCHDATA);

    /* Attach MRP return data packet to main response packet. Fill in fields. */
    pRspPacket->pPacket = (UINT8 *)ptrOutPkt;
    pRspPacket->pHeader->length = outPktSize;
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = ptrOutPkt->header.status;

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PI_VDiskSetAttributes
**
** Description: Set Vdisk Attributes
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_VDiskSetAttributes(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    MRSETATTR_REQ *pInPkt = NULL;
    MRSETATTR_RSP *pOutPkt = NULL;
    INT32       rc = PI_GOOD;
    UINT16      chgVdiskAtr = 0;

    /* Get the attribute change. */
    chgVdiskAtr = PI_VdiskGetAttributeChange(((PI_VDISK_SET_ATTRIBUTE_REQ *)(pReqPacket->pPacket))->attrRequest.vid,
                                             ((PI_VDISK_SET_ATTRIBUTE_REQ *)(pReqPacket->pPacket))->attrRequest.attr);

    /* Set the change attribute */
    ((PI_VDISK_SET_ATTRIBUTE_REQ *)(pReqPacket->pPacket))->change = chgVdiskAtr;

    /* Set the pointer for the MRP input packet. */
    pInPkt = &((PI_VDISK_SET_ATTRIBUTE_REQ *)(pReqPacket->pPacket))->attrRequest;

    /* Allocate memory for the MRP return data. */
    pOutPkt = MallocSharedWC(sizeof(*pOutPkt));

    /* Execute the MRP. */
    rc = PI_ExecMRP(pInPkt, sizeof(*pInPkt), MRSETATTR,
                    pOutPkt, sizeof(*pOutPkt), TMO_NONE);

    /*
     * Attach the MRP return data packet to the main response packet.
     * Fill in the header length and status fields.
     */
    pRspPacket->pPacket = (UINT8 *)pOutPkt;
    pRspPacket->pHeader->length = sizeof(PI_VDISK_SET_ATTRIBUTE_RSP);
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = pOutPkt->header.status;

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PI_GetVdiskAttributeChange
**
** Description: Get the change of vdisk attributes
**
** Inputs:      vid         - Vid to retrieve the changes
**              newVdiskAtr - Vdisk attributes to compare
**
** Returns:     Attribute change
**
**--------------------------------------------------------------------------*/
static UINT16 PI_VdiskGetAttributeChange(UINT16 vid, UINT16 newVdiskAtr)
{
    MRGETVINFO_RSP *pVDiskInfo;
    UINT16      chgVdiskAtr = 0;
#ifdef NO_VDISK_CACHE
    struct DMC *entry = DMC_CCB + CCB_DMC_vdiskcache;
#endif  /* NO_VDISK_CACHE */

#ifndef NO_VDISK_CACHE
    /*
     * Wait until the virtual disks cache is not in the process of
     * being updated. Once it is in that state make sure it is set
     * to in use so a update doesn't start while it is being used.
     */
    CacheStateWaitUpdating(cacheVirtualDisksState);
    CacheStateSetInUse(cacheVirtualDisksState);
#else   /* NO_VDISK_CACHE */
    /* Get memory lock and wait if busy. */
    Wait_DMC_Lock(entry);
#endif  /* NO_VDISK_CACHE */

    /* If we have a valid VID, Get the vdisk info from the cache. */
    if (X1ValidVid(vid) &&
#ifndef NO_VDISK_CACHE
        (pVDiskInfo = (MRGETVINFO_RSP *)cacheVirtualDiskAddr[vid]) != 0
#else   /* NO_VDISK_CACHE */
        (pVDiskInfo = (MRGETVINFO_RSP *)cacheVDiskBuffer_DMC.cacheVDiskAddr_DMC[vid]) != 0
#endif  /* NO_VDISK_CACHE */
        )
    {
        /* Calculate the delta. */
        chgVdiskAtr = (newVdiskAtr ^ pVDiskInfo->attr);
    }
    else
    {
        /* Else, the delta is the requested attributes. */
        chgVdiskAtr = newVdiskAtr;
    }

#ifndef NO_VDISK_CACHE
    /* No longer using the cache for this function. */
    CacheStateSetNotInUse(cacheVirtualDisksState);
#else   /* NO_VDISK_CACHE */
    /* Done with lock. */
    Free_DMC_Lock(entry);
#endif  /* NO_VDISK_CACHE */

    /*
     * We are only going to report the first change that we find.
     * Look to see if we have a delta on the attribute, and then
     * check to see if the attribute is being activated or deactivated.
     */

    /*
     * These two are internal flags only, they cannot
     * be set through the MRP. Therefore, we need not
     * be concerned if they are set, or are being set.
     */

    /* Check VD_LOCK. */
    if (chgVdiskAtr & VD_LOCK)
    {
        if (chgVdiskAtr & VD_LOCK & newVdiskAtr)
        {
            chgVdiskAtr = PI_VDISK_CHANGE_VDISK_LOCK;
        }
        else
        {
            chgVdiskAtr = PI_VDISK_CHANGE_VDISK_UNLOCK;
        }
    }
    /* Check VD_CACHEEN. */
    else if (chgVdiskAtr & VD_CACHEEN)
    {
        if (chgVdiskAtr & VD_CACHEEN & newVdiskAtr)
        {
            chgVdiskAtr = PI_VDISK_CHANGE_VDISK_CACHE_ENABLE;
        }
        else
        {
            chgVdiskAtr = PI_VDISK_CHANGE_VDISK_CACHE_DISABLE;
        }
    }
    /*
     * If it was none of the above, check to
     * see if it is being changed to normal,
     * hidden, or private.
     */
    /* Check VD_NORMAL. */
    else if ((newVdiskAtr & 0x0003) == VD_NORMAL)
    {
        chgVdiskAtr = PI_VDISK_CHANGE_NORMAL;
    }
    /* Check VD_HIDDEN. */
    else if ((newVdiskAtr & 0x0003) == VD_HIDDEN)
    {
        chgVdiskAtr = PI_VDISK_CHANGE_HIDDEN;
    }
    /* Check VD_PRIVATE. */
    else if ((newVdiskAtr & 0x0003) == VD_PRIVATE)
    {
        chgVdiskAtr = PI_VDISK_CHANGE_PRIVATE;
    }
    /* Check VD_ASYNCH for ALINK/APOOL changes */
    else if ((newVdiskAtr & VD_ASYNCH))
    {
        chgVdiskAtr = PI_VDISK_CHANGE_ASYNCH;
    }
    /* Else, we couldn't detect the change. */
    else
    {
        chgVdiskAtr = PI_VDISK_CHANGE_UNKNOWN;
    }

    return (chgVdiskAtr);
}

/*----------------------------------------------------------------------------
** Function:    PI_Raids - FROM CACHE
**
** Description: Raids Request Handler
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
** WARNING:     This function returns information from cache. It does NOT
**              issue MRPs.
**--------------------------------------------------------------------------*/
INT32 PI_RaidsCache(UNUSED XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    PI_RAIDS_RSP *ptrOutPkt;
    UINT32      outPktSize;
    UINT32      raid_info_size;
#ifndef NO_RAID_CACHE
    UINT8       retrycount = 5;
#else   /* NO_RAID_CACHE */
    struct DMC *entry = DMC_CCB + CCB_DMC_raidcache;
    UINT32      recheck_raid_info_size;
#endif  /* NO_RAID_CACHE */

    dprintf(DPRINTF_PI_COMMANDS, "PI_Raids - from CACHE\n");

    /* Wait until the RAIDs cache is not in locked. */

#ifndef NO_RAID_CACHE
    CacheStateWaitUpdating(cacheRaidsState);
    CacheStateSetInUse(cacheRaidsState);

    while (PI_IsCacheDirty(1 << PI_CACHE_INVALIDATE_RAID) && retrycount)
    {
        CacheStateSetNotInUse(cacheRaidsState);
        TaskSleepMS(200);
        retrycount--;
        CacheStateSetInUse(cacheRaidsState);
    }

    if (retrycount == 0)
    {
        fprintf(stderr, "%s ATTENTION: retrycount reached zero\n", __FUNCTION__);
    }
#else   /* NO_RAID_CACHE */
    /* Get memory lock and wait if busy. */
    Wait_DMC_Lock(entry);
#endif  /* NO_RAID_CACHE */

    /*
     * Calculate the size of the output packet. This will be the size
     * of the RAIDs (multiple devices) response packet plus the
     * size of a RAID Info (single device) response packet for each
     * device.
     */
    raid_info_size = CalcSizeRaidsCached();
#ifdef NO_RAID_CACHE
  retry_get:
    /* Done with lock. */
    Free_DMC_Lock(entry);
#endif  /* NO_RAID_CACHE */

    outPktSize = sizeof(*ptrOutPkt) + raid_info_size;
    ptrOutPkt = MallocWC(outPktSize);

#ifdef NO_RAID_CACHE
    /* Get memory lock and wait if busy. */
    Wait_DMC_Lock(entry);

    recheck_raid_info_size = CalcSizeRaidsCached();
    if (recheck_raid_info_size != raid_info_size)
    {
        Free(ptrOutPkt);                    /* Free does not task switch. */
        raid_info_size = recheck_raid_info_size;
        goto retry_get;
    }
#endif  /* NO_RAID_CACHE */

    /*
     * Copy the number of devices into the output packet along
     * with the data for each raid.
     */
    ptrOutPkt->count = RaidsCount();
#ifndef NO_RAID_CACHE
    RaidsGet(&ptrOutPkt->raids);

    CacheStateSetNotInUse(cacheRaidsState);
#else  /* NO_RAID_CACHE */
    memcpy(&ptrOutPkt->raids, &cacheRaidBuffer_DMC.cacheRaidBuffer_DMC, raid_info_size);

    /* Done with lock. */
    Free_DMC_Lock(entry);
#endif  /* NO_RAID_CACHE */

    /*
     * Attach the MRP return data packet to the main response packet.
     * Fill in the header length and status fields.
     */
    pRspPacket->pPacket = (UINT8 *)ptrOutPkt;
    pRspPacket->pHeader->length = outPktSize;
    pRspPacket->pHeader->status = GOOD;
    pRspPacket->pHeader->errorCode = GOOD;

    return GOOD;
}

/*----------------------------------------------------------------------------
** Function:    PI_Raids - MRP Requests
**
** Description: Raids Request Handler
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_Raids(UNUSED XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    MR_LIST_RSP *ptrRList = NULL;
    MR_LIST_RSP *ptrPList = NULL;
    UINT16      count = 0;
    PI_RAIDS_RSP *ptrOutPkt = NULL;
    UINT32      outPktSize = 0;
    UINT32      maxPktSize = 0;
    PI_RAID_INFO_REQ *pInPkt = NULL;
    PI_RAID_INFO_RSP *pOutPkt = NULL;
    UINT8      *pBuf = NULL;
    UINT32      bufSize = 0;
    INT32       errorCode = 0;

    dprintf(DPRINTF_PI_COMMANDS, "PI_Raids - MRPs issued\n");

    /*
     * Get the list of objects. Always start at the beginning and return
     * the entire list.
     */
    ptrRList = PI_GetList(0, (MRGETRLIST | GET_LIST));
    ptrPList = PI_GetList(0, (MRGETPLIST | GET_NUMBER_ONLY));

    /* If we could get the lists use them, otherwise signal an error */
    if (ptrRList != NULL && ptrPList != NULL)
    {
        /* Add the static data size for each RAID. */
        outPktSize += (sizeof(*pOutPkt) * ptrRList->ndevs);

        /*
         * Add the size of a MRGETRINFO_RSP_EXT for each of the pdisks
         * for every RAID.
         */
        outPktSize += (ptrRList->ndevs * (sizeof(MRGETRINFO_RSP_EXT) * ptrPList->ndevs));

        /*
         * Calculate the maximum size for a given RAID INFO request packet.
         */
        maxPktSize = (sizeof(*pOutPkt) + (sizeof(MRGETRINFO_RSP_EXT) * ptrPList->ndevs));

        /*
         * The outPktSize variable currently contains the size (in bytes) of
         * the raid information (including raid list) for all the
         * raids. For our response we need to include the size of the
         * PI_RAIDS_RSP packet.
         */
        outPktSize += sizeof(*ptrOutPkt);

        /*
         * The input packet will be used in the calls to the Raid Info
         * MRP.
         *
         * The output packet will be used in the calls to the Raid Info
         * MRP.
         *
         * The second output packet will contain the RAIDS information.
         */
        pInPkt = MallocWC(sizeof(*pInPkt));
        pOutPkt = MallocSharedWC(maxPktSize);
        ptrOutPkt = MallocWC(outPktSize);

        /* Save the number of devices in the output packet */
        ptrOutPkt->count = ptrRList->ndevs;

        /*
         * The pBuf variable holds the location in the output packet
         * buffer to where the next RAID INFOMATION should be placed.
         * This pointer moves through the output packet buffer for
         * each request to the RAID INFO MRP.
         */
        pBuf = ptrOutPkt->raids;

        /*
         * At this time the outPktSize contains the entire output packet
         * buffer size, including the PI_RAIDS_RSP structure size. We
         * need a variable that contains the size of the buffer remaining
         * for retrieving RAID information. After each call to the RAID
         * INFO MRP the buffer size will be reduced by the size of the
         * information returned by the MRP.
         */
        bufSize = outPktSize - sizeof(*ptrOutPkt);

        /*
         * Loop through each of the IDs in the RAID ID LIST and make
         * a call to the RAID INFO MRP.
         */
        for (count = 0; count < ptrRList->ndevs; count++)
        {
            /* Setup the input packet to have the correct RAID ID. */
            pInPkt->id = ptrRList->list[count];

            rc = PI_ExecMRP(pInPkt, sizeof(*pInPkt), MRGETRINFO,
                            pOutPkt, maxPktSize, GetGlobalMRPTimeout());

            if (rc == PI_GOOD)
            {
                if (pOutPkt->header.len <= bufSize)
                {
                    /*
                     * Copy the data retrieved throught the MRP into the
                     * output packet that contains the information for
                     * all the VDISKS combined.
                     */
                    memcpy(pBuf, pOutPkt, pOutPkt->header.len);

                    /*
                     * Decrement the remaining buffer size to not include this
                     * virtual disk information.
                     */
                    bufSize -= pOutPkt->header.len;

                    /*
                     * Move the output packet pointer to be past the VDISK info
                     * we just retrieved.
                     */
                    pBuf += pOutPkt->header.len;
                }
                else
                {
                    /*
                     * The amount of data returned in this request would
                     * exceed the buffer space so return the too much
                     * data error code.
                     */
                    rc = PI_ERROR;
                    errorCode = DETOOMUCHDATA;
                }
            }
            else
            {
                /*
                 * Some sort of error occurred. We cannot even tolerate a
                 * "too much data" error in this case since we should have
                 * done the calcualation correctly before starting to retrieve
                 * the virtual disk information.
                 */
                errorCode = pOutPkt->header.status;
                break;
            }
        }
    }
    else
    {
        /*
         * One or both of the lists could not be retrieved so
         * an error must be returned.
         */
        rc = PI_ERROR;
    }

    /* Free the allocated memory */
    Free(ptrRList);
    Free(ptrPList);
    Free(pInPkt);

    if (rc != PI_TIMEOUT)
    {
        Free(pOutPkt);
    }

    if (rc == PI_GOOD)
    {
        /*
         * Attach the MRP return data packet to the main response packet.
         * Fill in the header length and status fields.
         */
        pRspPacket->pPacket = (UINT8 *)ptrOutPkt;
        pRspPacket->pHeader->length = outPktSize;
        pRspPacket->pHeader->status = rc;
        pRspPacket->pHeader->errorCode = rc;
    }
    else
    {
        Free(ptrOutPkt);

        /* Indicate an error condition and no return data in the header. */
        pRspPacket->pHeader->length = 0;
        pRspPacket->pHeader->status = rc;
        pRspPacket->pHeader->errorCode = errorCode;
    }

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PI_RAIDInfo
**
** Description: Get info for the requested RAID
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_RAIDInfo(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    PI_RAID_INFO_RSP *ptrOutPkt = NULL;
    UINT32      outPktSize;
    UINT32      numPDisks;
    INT32       rc = PI_GOOD;

    /*
     * Allocate enough space to hold the maximum number of PDisks
     * allowed in a RAID. We can afford the memory and it saves the
     * overhead of making two calls.
     */
    numPDisks = MAX_PDISKS_PER_RAID;

    do
    {
        /*
         * Determine the output packet size based on the number of
         * RAIDs in the VDisk.
         */
        outPktSize = sizeof(*ptrOutPkt) + (numPDisks * sizeof(MRGETRINFO_RSP_EXT));

        /*
         * If an output packet was previously allocated, free it before
         * allocating a new one.
         */
        Free(ptrOutPkt);

        ptrOutPkt = MallocSharedWC(outPktSize);

        rc = PI_ExecMRP(pReqPacket->pPacket, pReqPacket->pHeader->length,
                        MRGETRINFO, ptrOutPkt, outPktSize, GetGlobalMRPTimeout());

        /*
         * Save the number of devices in case we need to make the
         * request again.
         */
        if (rc == PI_ERROR && ptrOutPkt->header.status == DETOOMUCHDATA)
        {
            numPDisks = ptrOutPkt->psdCnt;
        }
    } while (rc == PI_ERROR && ptrOutPkt->header.status == DETOOMUCHDATA);


    /*
     * Attach the MRP return data packet to the main response packet.
     * Fill in the header length and status fields.
     */
    pRspPacket->pPacket = (UINT8 *)ptrOutPkt;
    pRspPacket->pHeader->length = outPktSize;
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = ptrOutPkt->header.status;

    return (rc);
}

/*----------------------------------------------------------------------------
** Name:    VirtualDisks
**
** Desc:    This method will retrieve the virtual disk information for all
**          virtual disks.
**
** Inputs:  NONE
**
** Returns: Virtual disks response packet.
**
** WARNING: The caller of this method will need to free the response packet
**          after they have finished using it.
**--------------------------------------------------------------------------*/
PI_VDISKS_RSP *VirtualDisks(void)
{
    UINT32      rc = PI_GOOD;
    XIO_PACKET  reqPacket = { NULL, NULL };
    XIO_PACKET  rspPacket = { NULL, NULL };
    PI_VDISKS_RSP *pResponse = NULL;

    /*
     * Allocate memory for the request (header and data) and the
     * response header. The response data will be allocated in the called
     * function.
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    reqPacket.pHeader->packetVersion = 2;
    rspPacket.pHeader->packetVersion = 2;

    /* Fill in the Header */
    reqPacket.pHeader->commandCode = PI_VDISKS_CMD;
    reqPacket.pHeader->length = 0;

    /* Issue the command through the packet command handler */
    rc = PortServerCommandHandler(&reqPacket, &rspPacket);

    if (rc == PI_GOOD)
    {
        ccb_assert(rspPacket.pPacket != NULL, rspPacket.pPacket);

        pResponse = (PI_VDISKS_RSP *)rspPacket.pPacket;
        rspPacket.pPacket = NULL;
    }
    else
    {
        dprintf(DPRINTF_DEFAULT, "VirtualDisks - Failed to retrieve the virtual disks information.\n");
    }

    /* Free the allocated memory */
    Free(reqPacket.pHeader);
    Free(rspPacket.pHeader);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPacket.pPacket);
    }

    return pResponse;
}

/**
******************************************************************************
**
**  @brief      This method will retrieve the raid information for all
**              raids.
**
**  @param      none
**
**  @return     Raids response packet.
**
**  @attention  The caller of this method will need to free the response
**              packet after they have finished using it.
**
******************************************************************************
**/
PI_RAIDS_RSP *Raids(void)
{
    UINT32      rc = PI_GOOD;
    XIO_PACKET  reqPacket = { NULL, NULL };
    XIO_PACKET  rspPacket = { NULL, NULL };
    PI_RAIDS_RSP *pResponse = NULL;

    /*
     * Allocate memory for the request (header and data) and the
     * response header. The response data will be allocated in the called
     * function.
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    /* Fill in the Header */
    reqPacket.pHeader->commandCode = PI_RAIDS_CMD;
    reqPacket.pHeader->length = 0;

    /* Issue the command through the packet command handler */
    rc = PortServerCommandHandler(&reqPacket, &rspPacket);

    if (rc == PI_GOOD)
    {
        ccb_assert(rspPacket.pPacket != NULL, rspPacket.pPacket);

        pResponse = (PI_RAIDS_RSP *)rspPacket.pPacket;
        rspPacket.pPacket = NULL;
    }
    else
    {
        dprintf(DPRINTF_DEFAULT, "Raids: Failed to retrieve the raids information.\n");
    }

    /* Free the allocated memory */
    Free(reqPacket.pHeader);
    Free(rspPacket.pHeader);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPacket.pPacket);
    }

    return pResponse;
}

/**
******************************************************************************
**
**  @brief      This method will retrieve the list of raids "owned" by the
**              specified controller.
**
**  @param      UINT32 controllerSN
**
**  @return     MR_LIST_RSP of owned raids (not sorted).
**
**  @attention  The caller of this method will need to free the response
**              packet after they have finished using it.
**
******************************************************************************
**/
MR_LIST_RSP *RaidsOwnedByController(UINT32 controllerSN)
{
    MR_LIST_RSP *pRaidList = NULL;      /* malloc'd locally, returned to caller */
    PI_TARGETS_RSP *pTargetData = NULL; /* malloc'd locally */
    PI_SERVERS_RSP *pServerData = NULL; /* malloc'd locally */
    PI_VDISKS_RSP *pVdiskData = NULL;   /* malloc'd locally */

    pTargetData = Targets(GetMyControllerSN());
    pServerData = CachedServers();
    pVdiskData = CachedVDisks();

    pRaidList = SM_RaidsOwnedByController(controllerSN, pServerData,
                                          pTargetData, pVdiskData);

    /* Release memory */
    Free(pTargetData);
    Free(pServerData);
    Free(pVdiskData);

    return pRaidList;
}

/**
******************************************************************************
**
**  @brief      This method will search a list of raids, and determine if any
**              of the raid 5 raids are not mirroring.
**
**  @param      MR_LIST_RSP *pRaidList - list of owned raids (not sorted).
**
**  @return     INT32 - count of non-mirroring or degraded raid5 raids
**                      found or -1 on error.
**
******************************************************************************
**/
INT32 CheckForNotMirroringRaid5s(MR_LIST_RSP *pRaidList)
{
    MRGETRINFO_RSP *pRaidInfoCache = NULL;
    INT32       count = 0;
    UINT32      i;
    UINT8     **pCacheAddr;
#ifdef NO_RAID_CACHE
    struct DMC *entry = DMC_CCB + CCB_DMC_raidcache;
#endif  /* NO_RAID_CACHE */

#ifndef NO_RAID_CACHE
    /*
     * Wait until the RAID cache is not in the process of being updated. Once
     * it is in that state make sure it is set to in use so a update doesn't
     * start while it is being used.
     */
    CacheStateWaitUpdating(cacheRaidsState);
    CacheStateSetInUse(cacheRaidsState);

    pCacheAddr = cacheRaidAddr;
#else   /* NO_RAID_CACHE */
    /* Get memory lock and wait if busy. */
    Wait_DMC_Lock(entry);

    pCacheAddr = cacheRaidBuffer_DMC.cacheRaidAddr_DMC;
#endif  /* NO_RAID_CACHE */

    /*
     * Check the raid type -- only look at Raid-5. If Raid-5 and
     * "notMirroringCSN" is non-zeroing, this means we ARE NOT mirroring --
     * this is the failure case.
     */
    dprintf(DPRINTF_DEFAULT, "Owned Raid Count: %u\n", pRaidList->ndevs);
    for (i = 0; i < pRaidList->ndevs; i++)
    {
        UINT16 raid = pRaidList->list[i];
        dprintf(DPRINTF_DEFAULT, "Raid examined: %hu\n", raid);
        pRaidInfoCache = (MRGETRINFO_RSP *)pCacheAddr[raid];
        if (pRaidInfoCache)
        {
            if (pRaidInfoCache->type == RD_RAID5 && pRaidInfoCache->notMirroringCSN)
            {
                dprintf(DPRINTF_DEFAULT, "-- not mirroring\n");
                count++;
            }
        }
        else
        {
            dprintf(DPRINTF_DEFAULT, "Null pointer in index, aborting %s\n", __func__);
            count = -1;
            break;
        }
    }

#ifndef NO_RAID_CACHE
    /* No longer using the cache for this function. */
    CacheStateSetNotInUse(cacheRaidsState);
#else   /* NO_RAID_CACHE */
    /* Done with lock. */
    Free_DMC_Lock(entry);
#endif  /* NO_RAID_CACHE */

    dprintf(DPRINTF_DEFAULT, "Raid5 Not Mirroring Count == %d\n", count);

    return count;
}


/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
