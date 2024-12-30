/* $Id: PI_Server.c 156532 2011-06-24 21:09:44Z m4 $*/
/**
******************************************************************************
**
**  @file       PI_Server.c
**
**  @brief      Packet Interface for Server Commands
**
**  Handler functions for Server request packets.
**
**  Copyright (c) 1996-2009 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#include "PI_Server.h"

#include "AsyncEventHandler.h"
#include "CacheSize.h"
#include "CacheManager.h"
#include "CacheServer.h"
#include "CmdLayers.h"
#include "convert.h"
#include "LOG_Defs.h"
#include "ipc_common.h"
#include "ipc_sendpacket.h"
#include "misc.h"
#include "MR_Defs.h"
#include "PacketInterface.h"
#include "PktCmdHdl.h"
#include "PI_CmdHandlers.h"
#include "PI_Target.h"
#include "PI_Utils.h"
#include "quorum_utils.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
** Function:    PI_Servers - MRP Requests
**
** Description: Servers Request Handler
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_Servers(UNUSED XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    MR_LIST_RSP *ptrList = NULL;
    UINT16      count = 0;
    PI_SERVERS_RSP *ptrOutPkt = NULL;
    UINT32      outPktSize = 0;
    UINT32      maxPktSize = 0;

    PI_SERVER_INFO_REQ *pInPkt = NULL;
    PI_SERVER_INFO_RSP *pOutPkt = NULL;
    UINT8      *pBuf = NULL;
    UINT32      bufSize = 0;
    INT32       errorCode = 0;

    dprintf(DPRINTF_PI_COMMANDS, "PI_Servers - MRPs issued\n");

    /*
     * Get the list of objects.  Always start at the beginning and return
     * the entire list.
     */
    ptrList = PI_GetList(0, (MRGETSLIST | GET_LIST));

    /* If we could get the list use it, otherwise signal an error */
    if (ptrList != NULL)
    {
        /*
         * Add the static data size for each SERVER.
         */
        outPktSize += (sizeof(*pOutPkt) * ptrList->ndevs);

        /*
         * Add the size for the maximum number of lun maps for
         * each of the servers.
         */
        outPktSize += (ptrList->ndevs * (sizeof(MRGETSINFO_RSP_LM) * MAX_LUNS));

        /*
         * Calculate the maximum size for a given SERVER INFO request packet.
         */
        maxPktSize = (sizeof(*pOutPkt) + (sizeof(MRGETSINFO_RSP_LM) * MAX_LUNS));

        /*
         * The outPktSize variable currently contains the size (in bytes) of
         * the server information (including lun map) for all the
         * servers.  For our response we need to include the size of the
         * PI_SERVERS_RSP packet.
         */
        outPktSize += sizeof(*ptrOutPkt);

        /*
         * Allocate the MRP input packet and the output packet that will
         * contain the SERVERS information.  The input packet will be used
         * in the calls to the Server Info MRP and the output packet is the
         * buffer to hold the server information for all the servers.
         */
        pInPkt = MallocWC(sizeof(*pInPkt));
        pOutPkt = MallocSharedWC(maxPktSize);
        ptrOutPkt = MallocWC(outPktSize);

        /* Save the number of devices in the output packet */
        ptrOutPkt->count = ptrList->ndevs;

        /*
         * The pBuf variable holds the location in the output packet
         * buffer to where the next SERVER INFOMATION should be placed.
         * This pointer moves through the output packet buffer for
         * each request to the SERVER INFO MRP.
         */
        pBuf = (UINT8 *)ptrOutPkt->servers;

        /*
         * At this time the outPktSize contains the entire output packet
         * buffer size, including the PI_SERVERS_RSP structure size.  We
         * need a variable that contains the size of the buffer remaining
         * for retrieving SERVER information.  After each call to the SERVER
         * INFO MRP the buffer size will be reduced by the size of the
         * information returned by the MRP.
         */
        bufSize = outPktSize - sizeof(*ptrOutPkt);

        /*
         * Loop through each of the IDs in the SERVER ID LIST and make
         * a call to the SERVER INFO MRP.
         */
        for (count = 0; count < ptrList->ndevs; count++)
        {
            /*
             * Setup the input packet to have the correct VDISK ID.
             */
            pInPkt->id = ptrList->list[count];

            rc = PI_ExecMRP(pInPkt, sizeof(*pInPkt), MRGETSINFO,
                            pOutPkt, maxPktSize, GetGlobalMRPTimeout());

            if (rc == PI_GOOD)
            {
                if (MAX(pOutPkt->header.len, sizeof(*pOutPkt)) <= bufSize)
                {
                    /*
                     * Copy the data retrieved throught the MRP into the
                     * output packet that contains the information for
                     * all the SERVERS combined.
                     */
                    memcpy(pBuf, pOutPkt, MAX(pOutPkt->header.len, sizeof(*pOutPkt)));

                    /*
                     * Decrement the remaining buffer size to not include this
                     * server information.
                     */
                    bufSize -= MAX(pOutPkt->header.len, sizeof(*pOutPkt));

                    /*
                     * Move the output packet pointer to be past the SERVER info
                     * we just retrieved.
                     */
                    pBuf += MAX(pOutPkt->header.len, sizeof(*pOutPkt));
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
                 * Some sort of error occurred.  We cannot even tolerate a
                 "" "too much data" error in this case since we should have
                 * done the calcualation correctly before starting to retrieve
                 * the server information.
                 */
                errorCode = pOutPkt->header.status;
                break;
            }
        }
    }
    else
    {
        /*
         * The list could not be retrieved so an error must be returned.
         */
        rc = PI_ERROR;
    }

    /*
     * Free the allocated memory
     */
    Free(ptrList);
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

        /*
         * Indicate an error condition and no return data in the header.
         */
        pRspPacket->pHeader->length = 0;
        pRspPacket->pHeader->status = rc;
        pRspPacket->pHeader->errorCode = errorCode;
    }

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PI_ServerInfo
**
** Description: Get info for the requested server
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_ServerInfo(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    PI_SERVER_INFO_RSP *ptrOutPkt = NULL;
    UINT32      outPktSize;
    UINT32      numLuns;
    INT32       rc = PI_GOOD;

    /*
     * Assume MAX_LUNS so we should only have to make the MRP call once.
     */
    numLuns = MAX_LUNS;

    do
    {
        /*
         * Determine the output packet size based on the number of
         * Luns mapped to the server.
         */
        outPktSize = sizeof(*ptrOutPkt) + (numLuns * sizeof(MRGETSINFO_RSP_LM));

        /*
         * If an output packet was previously allocated, free it before
         * allocating a new one.
         */
        Free(ptrOutPkt);

        ptrOutPkt = MallocSharedWC(outPktSize);

        rc = PI_ExecMRP(pReqPacket->pPacket, pReqPacket->pHeader->length,
                        MRGETSINFO, ptrOutPkt, outPktSize, GetGlobalMRPTimeout());

        /*
         * Save the number of devices in case we need to make the
         * request again.
         */
        if (rc == PI_ERROR && ptrOutPkt->header.status == DETOOMUCHDATA)
        {
            numLuns = ptrOutPkt->nluns;
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
** Function:    PI_ServerWwnToTargetMap
**
** Description: Generate a bit map of the targets each server is visible on.
**              The Target ID is "normalized" to a value between 0 and
**              MAX_FE_PORTS-1.  This assumes that tid=0 on one controller
**              corresponds to tid=MAX_FE_PORTS on slave 1, tid=2*MAX_FE_PORTS
**              on slave 2, etc.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_ServerWwnToTargetMap(UNUSED XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    PI_WWN_TO_TARGET_MAP_RSP *ptrOutPkt = NULL;
    PI_TARGET_RESOURCE_LIST_RSP *pTargetResList = NULL;
    MRGETTRLIST_RSP_WWNFMT *targetRecord = NULL;
    UINT32      responseLength = 0;
    INT32       rc = PI_GOOD;
    UINT16      mapIndex = 0;
    UINT16      i;
    bool        wwnFound = FALSE;

    /*
     * 1. Get the active servers using Target Resource List.
     */
    pTargetResList = TargetResourceList(0xFFFF, (SERVERS_ACTIVE + WWN_FORMAT));

    /*
     * 2. Compile a list of unique physical servers (WWNs) from the active
     *    list.  Only active servers will be validated.
     */
    if (pTargetResList != NULL)
    {
        /*
         * We don't know in advance how many unique Servers (i.e. WWNs)
         * we will find but it must be <= the number of items in the
         * Target Resource List.  The actual return length is calculated
         * as we walk through the data.
         */
        ptrOutPkt = MallocWC(sizeof(*ptrOutPkt) + (pTargetResList->ndevs * sizeof(WWN_TO_TARGET_MAP)));

        /*
         * Initialize the count of WWN to target maps.
         *  mapIndex         - is used to walk through the response data to see
         *                     if there is already an entry for the current WWN.
         *  ptrOutPkt->count - The place where the next map entry goes in the
         *                     response data.
         *  wwnFound         - This WWN was found in the response data.
         */
        ptrOutPkt->count = 0;
        targetRecord = (MRGETTRLIST_RSP_WWNFMT *)(pTargetResList->list);

        for (i = 0; i < pTargetResList->ndevs; i++)
        {
            /*
             * Reset mapIndex prior to looping through the existing maps.
             * Reset the wwnFound flag before looking at this entry.
             */
            mapIndex = 0;
            wwnFound = FALSE;

            /*
             * Loop through existing maps to find a match for this wwn.
             * If a match does not exist a new map will be created below.
             */
            while (mapIndex < ptrOutPkt->count)
            {
                /*
                 * If we have already created an entry for this WWN -
                 * (1) add the target ID to the bitmap and
                 * (2) indicate the WWN as found and break out of the loop.
                 */
                if (ptrOutPkt->map[mapIndex].wwn == targetRecord[i].wwn)
                {
                    /*
                     * A single "pool" of target ID values is used across
                     * all controllers in the group.  For example if the
                     * max number of FE ports is 4 then controller 0 will
                     * have targets 0-3, controller 1 will have targets
                     * 4-7, etc.  For this reason the target ID value is
                     * normalized to a value between 0 and MAX_FE_PORTS-1
                     * to allow the target maps to be compared between
                     * controllers.
                     */
                    ptrOutPkt->map[mapIndex].targetBitmap |= (0x01 << (targetRecord[i].tid % MAX_FE_PORTS));

                    wwnFound = TRUE;    /* Do NOT create a new entry    */
                    break;      /* Exit the while loop          */
                }
                mapIndex++;     /* Move to the next entry in the output data */
            }

            /*
             * If no entry exists in the output data for this WWN,
             * create one now.
             */
            if (!wwnFound)
            {
                ptrOutPkt->map[ptrOutPkt->count].wwn = targetRecord[i].wwn;

                ptrOutPkt->map[ptrOutPkt->count].targetBitmap |= (0x01 << (targetRecord[i].tid % MAX_FE_PORTS));

                /*
                 * Indicate that a map was added to the response data.
                 */
                ptrOutPkt->count++;
            }
        }

        /*
         * Now that we're done we can calculate the actual response
         * data length.
         */
        responseLength = sizeof(*ptrOutPkt) + (ptrOutPkt->count * sizeof(WWN_TO_TARGET_MAP));
    }
    else
    {
        /*
         * Indicate that an error occurred on the request.
         */
        rc = PI_ERROR;
    }


#if 0

/*
**  This 2nd check for logged on servers is being removed by CQ 9036.
**  I'm leaving the code in for now to make it easier to put back if we
**  change our minds.
*/

    /*
     * Continue if previous request was successful.
     */
    if (rc == PI_GOOD)
    {
        /*
         * Free memory used for the Target Resource List.
         */
        Free(pTargetResList);

        /*
         * 3. Issue the Target Resource List command again, this time request
         *    logged on servers.  For each WWN found previously create a map
         *    of the targets found.
         */
        pTargetResList = TargetResourceList(0xFFFF, (SERVERS_LOGON + WWN_FORMAT));

        if (pTargetResList != NULL)
        {
            /*
             * Walk the list of logged on servers.  If a WWN is found that
             * is in the previous list of active servers, add its target
             * to the output packet.
             */
            targetRecord = (MRGETTRLIST_RSP_WWNFMT *)(pTargetResList->list);

            for (i = 0; i < pTargetResList->ndevs; i++)
            {
                /*
                 * Reset mapIndex prior to looping through the existing maps.
                 */
                mapIndex = 0;

                /*
                 * Loop through existing maps to find a match for this wwn.
                 * If the wwn is found add the target to the target map.
                 */
                while (mapIndex < ptrOutPkt->count)
                {
                    /*
                     * If there is a map for this WWN, "OR" the new target ID
                     * with the existing map.
                     */
                    if (ptrOutPkt->map[mapIndex].wwn == targetRecord[i].wwn)
                    {
                        /*
                         * A single "pool" of target ID values is used across
                         * all controllers in the group.  For example if the
                         * max number of FE ports is 4 then controller 0 will
                         * have targets 0-3, controller 1 will have targets
                         * 4-7, etc.  For this reason the target ID value is
                         * normalized to a value between 0 and MAX_FE_PORTS-1
                         * to allow the target maps to be compared between
                         * controllers.
                         */
                        ptrOutPkt->map[mapIndex].targetBitmap |= (0x01 << (targetRecord[i].tid % MAX_FE_PORTS));
                        break;  /* Exit the while loop */
                    }
                    mapIndex++; /* Move to the next entry in the output data */
                }
            }
        }
        else
        {
            /*
             * Indicate that an error occurred on the request.
             */
            rc = PI_ERROR;
        }
    }
#endif  /* 0 */


    /*
     * Attach the MRP return data packet to the main response packet.
     * Fill in the header length and status fields.
     */
    pRspPacket->pPacket = (UINT8 *)ptrOutPkt;
    pRspPacket->pHeader->length = responseLength;
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = PI_GOOD;

    /*
     * Free all memory.
     */
    Free(pTargetResList);

    return (rc);
}


/**
******************************************************************************
**
**  @brief      Get the information for all the servers on the given
**              controller.
**
**  @param      UINT32 controllerSN - Serial number for the controller
**
**  @return     PI_SERVERS_RSP* - Pointer to a servers response packet
**                                or NULL if they could not be retrieved.
**
**  @attention  The pointer to the response packet needs to be freed by the
**              caller or it will be leaked.
**
******************************************************************************
**/
PI_SERVERS_RSP *Servers(UINT32 controllerSN)
{
    INT32       rc = PI_GOOD;
    XIO_PACKET  reqPacket = { NULL, NULL };
    XIO_PACKET  rspPacket = { NULL, NULL };
    PI_SERVERS_RSP *pResponse = NULL;

    /*
     * Allocate memory for the request (header and data) and the
     * response header. The response data will be allocated in the called
     * function.
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    /*
     * Fill in the Header
     */
    reqPacket.pHeader->commandCode = PI_SERVERS_CMD;
    reqPacket.pHeader->length = 0;

    /*
     * If the targets request is for this controller make the
     * request to the port server directly.  If it is for one
     * of the slave controllers then tunnel the request to
     * that controller.
     */
    if (controllerSN == GetMyControllerSN())
    {
        /*
         * Issue the command through the top-level command handler.
         */
        rc = PortServerCommandHandler(&reqPacket, &rspPacket);
    }
    else
    {
        UINT8   retries = 2;                /* Ethernet, Fiber(1), Disk Quorum(2) */

        do
        {
            if (rc != PI_TIMEOUT)
            {
                Free(rspPacket.pPacket);
            }
            else
            {
                rspPacket.pPacket = NULL;
            }
            rc = TunnelRequest(controllerSN, &reqPacket, &rspPacket);
        } while (rc != GOOD && (retries--) > 0);
    }

    if (rc == PI_GOOD)
    {
        pResponse = (PI_SERVERS_RSP *)rspPacket.pPacket;
        rspPacket.pPacket = NULL;
    }

    /*
     * Free the allocated memory
     */
    Free(reqPacket.pHeader);
    Free(rspPacket.pHeader);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPacket.pPacket);
    }

    return (pResponse);
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
