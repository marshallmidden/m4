/* $Id: PI_WCache.c 156532 2011-06-24 21:09:44Z m4 $*/
/**
******************************************************************************
**
**  @file       PI_WCache.c
**
**  @brief      Packet Interface and miscellaneous functions for
**              Write Cache Commands
**
**  These functions support the write cache requests.
**
**  Copyright (c) 2001-2009 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#include "PI_WCache.h"
#include "CmdLayers.h"
#include "ipc_sendpacket.h"
#include "PacketInterface.h"
#include "PI_Utils.h"
#include "quorum_utils.h"
#include "XIO_Std.h"
#include "XIO_Types.h"
#include "PI_CmdHandlers.h"

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
static INT32 WCacheInvalidateBEWC(UINT32 controllerSN);
static INT32 WCacheInvalidateFEWC(UINT32 controllerSN);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      Handle the PI_WCACHE_INVALIDATE_CMD reqeuest packet
**              sent from a client.
**
**  @param      XIO_PACKET* pReqPacket - pointer to the request packet
**  @param      XIO_PACKET* pRspPacket - pointer to the response packet
**
**  @return     INT32 - PI_GOOD, PI_ERROR or one of the other PI return codes.
**
**  @attention
**
******************************************************************************
**/
INT32 PI_WCacheInvalidate(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    PI_WCACHE_INVALIDATE_REQ *pReq;
    INT32       rc = PI_GOOD;

    pReq = (PI_WCACHE_INVALIDATE_REQ *)pReqPacket->pPacket;

    if (pReq->option & PI_WCACHE_INV_OPT_BE)
    {
        rc = WCacheInvalidateBEWC(GetMyControllerSN());
    }

    if (pReq->option & PI_WCACHE_INV_OPT_FE)
    {
        rc = WCacheInvalidateFEWC(GetMyControllerSN());
    }
    /*
     * Attach the MRP return data packet to the main response packet.
     * Fill in the header length and status fields.
     */
    pRspPacket->pPacket = NULL;
    pRspPacket->pHeader->length = 0;
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = rc;

    return rc;
}


/**
******************************************************************************
**
**  @brief      Send the Invalidate Write Cache request to the controller.
**
**  @param      UINT32 controllerSN - Which controller receives the request.
**
**  @param      UINT32 option - What should be invalidated (see options in
**                              PacketInterface.h.
**
**  @return     INT32 - PI_GOOD, PI_ERROR or one of the other PI return codes.
**
**  @attention
**
******************************************************************************
**/
INT32 WCacheInvalidate(UINT32 controllerSN, UINT32 option)
{
    INT32       rc = PI_GOOD;
    XIO_PACKET  reqPacket;
    XIO_PACKET  rspPacket;

    dprintf(DPRINTF_DEFAULT, "WCacheInvalidate: ENTER (0x%x, 0x%x)\n",
            controllerSN, option);

    /*
     * Allocate memory for the request (header and data) and the
     * response header. The response data will be allocated in the called
     * function.
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    reqPacket.pPacket = MallocWC(sizeof(PI_WCACHE_INVALIDATE_REQ));
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    rspPacket.pPacket = NULL;
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    reqPacket.pHeader->commandCode = PI_WCACHE_INVALIDATE_CMD;
    reqPacket.pHeader->length = sizeof(PI_WCACHE_INVALIDATE_REQ);

    ((PI_WCACHE_INVALIDATE_REQ *)reqPacket.pPacket)->option = option;

    /*
     * If the request is for this controller send it to the port server
     * directly.  If it is for one of the other controllers then tunnel
     * the request to that controller.
     */
    if (controllerSN == GetMyControllerSN())
    {
        /*
         * Issue the command through the top-level command handler.
         * Validate the ports and generate a port bit map to be used later.
         */
        rc = PortServerCommandHandler(&reqPacket, &rspPacket);
    }
    else
    {
        UINT8   retries = 2;            /* Ethernet, Fiber(1), Disk Quorum(2) */

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

    /*
     * Free the allocated memory
     */
    Free(reqPacket.pHeader);
    Free(reqPacket.pPacket);
    Free(rspPacket.pHeader);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPacket.pPacket);
    }

    return rc;
}


/**
******************************************************************************
**
**  @brief      To provide a consistent means of sending the MRINVBEWC request
**              to the BE processor on a specific controller.  This function
**              uses the packet interface and tunnel requests to send the
**              request locally or remotely (respectively).
**
**  @param      controllerSN - controller to execute the request
**
**  @return     INT32 - Packet return status
**
******************************************************************************
**/
static INT32 WCacheInvalidateBEWC(UINT32 controllerSN)
{
    INT32       rc = PI_GOOD;
    XIO_PACKET  reqPacket;
    XIO_PACKET  rspPacket;

    dprintf(DPRINTF_DEFAULT, "WCacheInvalidateBEWC: ENTER (0x%x)\n", controllerSN);

    /*
     * Allocate memory for the request (header and data) and the
     * response header. The response data will be allocated in the called
     * function.
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    reqPacket.pPacket = MallocWC(sizeof(PI_MISC_INVALIDATE_BE_WC_REQ));
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    rspPacket.pPacket = NULL;
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    reqPacket.pHeader->commandCode = PI_MISC_INVALIDATE_BE_WC_CMD;
    reqPacket.pHeader->length = sizeof(PI_MISC_INVALIDATE_BE_WC_REQ);

    ((PI_MISC_INVALIDATE_BE_WC_REQ *)reqPacket.pPacket)->option = MIBOGLOBALINVAL;

    /*
     * If the request is for this controller send it to the port server
     * directly.  If it is for one of the other controllers then tunnel
     * the request to that controller.
     */
    if (controllerSN == GetMyControllerSN())
    {
        /*
         * Issue the command through the top-level command handler.
         * Validate the ports and generate a port bit map to be used later.
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

    /*
     * Free the allocated memory
     */
    Free(reqPacket.pHeader);
    Free(reqPacket.pPacket);
    Free(rspPacket.pHeader);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPacket.pPacket);
    }

    return rc;
}


/**
******************************************************************************
**
**  @brief      To provide a consistent means of sending the MRINVFEWC request
**              to the FE processor on a specific controller.  This function
**              uses the packet interface and tunnel requests to send the
**              request locally or remotely (respectively).
**
**  @param      controllerSN - controller to execute the request
**
**  @return     INT32 - Packet return status
**
******************************************************************************
**/
static INT32 WCacheInvalidateFEWC(UINT32 controllerSN)
{
    INT32       rc = PI_GOOD;
    XIO_PACKET  reqPacket;
    XIO_PACKET  rspPacket;

    dprintf(DPRINTF_DEFAULT, "WCacheInvalidateFEWC: ENTER (0x%x)\n", controllerSN);

    /*
     * Allocate memory for the request (header and data) and the
     * response header. The response data will be allocated in the called
     * function.
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    reqPacket.pPacket = MallocWC(sizeof(PI_MISC_INVALIDATE_FE_WC_REQ));
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    rspPacket.pPacket = NULL;
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    reqPacket.pHeader->commandCode = PI_MISC_INVALIDATE_FE_WC_CMD;
    reqPacket.pHeader->length = sizeof(PI_MISC_INVALIDATE_FE_WC_REQ);

    ((PI_MISC_INVALIDATE_FE_WC_REQ *)reqPacket.pPacket)->option = MIBOGLOBALINVAL;

    /*
     * If the request is for this controller send it to the port server
     * directly.  If it is for one of the other controllers then tunnel
     * the request to that controller.
     */
    if (controllerSN == GetMyControllerSN())
    {
        /*
         * Issue the command through the top-level command handler.
         * Validate the ports and generate a port bit map to be used later.
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

    /*
     * Free the allocated memory
     */
    Free(reqPacket.pHeader);
    Free(reqPacket.pPacket);
    Free(rspPacket.pHeader);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPacket.pPacket);
    }

    return rc;
}


/**
******************************************************************************
**
**  @brief      To provide a consistent means of sending the MRFLUSHBEWC
**              request on a specific controller.  This function uses the
**              packet interface and tunnel requests to send the request
**              locally or remotely (respectively).
**
**  @param      controllerSN - controller to execute the request
**
**  @return     INT32 - Packet return status
**
******************************************************************************
**/
INT32 WCacheFlushBEWC(UINT32 controllerSN)
{
    INT32       rc = PI_ERROR;
    XIO_PACKET  reqPacket;
    XIO_PACKET  rspPacket;

    dprintf(DPRINTF_DEFAULT, "WCacheFlushBEWC: ENTER (0x%x)\n", controllerSN);

    /*
     * Allocate memory for the request (header and data) and the
     * response header. The response data will be allocated in the called
     * function.
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    reqPacket.pPacket = MallocWC(sizeof(PI_CACHE_FLUSH_BE_REQ));
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    rspPacket.pPacket = NULL;
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    reqPacket.pHeader->commandCode = PI_CACHE_FLUSH_BE_CMD;
    reqPacket.pHeader->length = sizeof(PI_CACHE_FLUSH_BE_REQ);

    ((PI_CACHE_FLUSH_BE_REQ *)reqPacket.pPacket)->option = MFBOGLOBALINVAL;

    /*
     * If the request is for this controller send it to the port server
     * directly.  If it is for one of the other controllers then tunnel
     * the request to that controller.
     */
    if (controllerSN == GetMyControllerSN())
    {
        /*
         * Issue the command through the top-level command handler.
         * Validate the ports and generate a port bit map to be used later.
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

    /*
     * Free the allocated memory
     */
    Free(reqPacket.pHeader);
    Free(reqPacket.pPacket);
    Free(rspPacket.pHeader);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPacket.pPacket);
    }

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
