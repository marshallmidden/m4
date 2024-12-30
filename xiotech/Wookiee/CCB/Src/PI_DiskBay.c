/* $Id: PI_DiskBay.c 143020 2010-06-22 18:35:56Z m4 $*/
/*===========================================================================
** FILE NAME:       PI_DiskBay.c
** MODULE TITLE:    Packet Interface for Disk Bay Commands
**
** DESCRIPTION:     Handler functions for Disk Bay request packets.
**
** Copyright (c) 2002-20010 Xiotech Corporation. All rights reserved.
**==========================================================================*/
#include "PI_DiskBay.h"
#include "CacheMisc.h"
#include "CmdLayers.h"
#include "convert.h"
#include "debug_files.h"
#include "LOG_Defs.h"
#include "misc.h"
#include "MR_Defs.h"
#include "PacketInterface.h"
#include "PI_CmdHandlers.h"
#include "PI_Utils.h"
#include "PktCmdHdl.h"
#include "ses.h"
#include "X1_Packets.h"
#include "X1_Structs.h"
#include "XIO_Std.h"
#include "XIO_Const.h"

#include <byteswap.h>


/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
** Function:    PI_DiskBays
**
** Description: Disk Bays Request Handler
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_DiskBays(UNUSED XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    MR_LIST_RSP *ptrList;
    UINT16      count = 0;
    PI_DISK_BAYS_RSP *ptrOutPkt = NULL;
    UINT32      outPktSize = 0;
    XIO_PACKET  reqPacket = { NULL, NULL };
    XIO_PACKET  rspPacket = { NULL, NULL };

    /*
     * Get the list of objects.  Always start at the beginning and return
     * the entire list.
     */
    ptrList = PI_GetList(0, (MRGETELIST | GET_LIST));

    /* If we could not get the list, signal an error */
    if (ptrList == NULL)
    {
        rc = PI_ERROR;
    }

    if (rc == PI_GOOD)
    {
        /*
         * Calculate the size of the output packet.  This will be the size
         * of the physical disks (multiple devices) response packet plus the
         * size of a physical disk info (single device) response packet for each
         * device.
         */
        outPktSize = sizeof(*ptrOutPkt) + (ptrList->ndevs * sizeof(PI_DISK_BAY_INFO_RSP));

        ptrOutPkt = MallocWC(outPktSize);

        /* Save the number of devices in the output packet */
        ptrOutPkt->count = ptrList->ndevs;

        /*
         * Allocate memory for the request (header and data) and the
         * response header. The response data will be allocated in the called
         * function.
         */
        reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
        reqPacket.pPacket = MallocWC(sizeof(PI_DISK_BAY_INFO_REQ));
        rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
        reqPacket.pHeader->packetVersion = 1;
        rspPacket.pHeader->packetVersion = 1;

        /*
         * Fill in the Header
         */
        reqPacket.pHeader->commandCode = PI_DISK_BAY_INFO_CMD;
        reqPacket.pHeader->length = sizeof(PI_DISK_BAY_INFO_REQ);

        for (count = 0; count < ptrList->ndevs; count++)
        {
            rspPacket.pPacket = NULL;

            /* Setup the ID for this Virtual Disk Information Request */
            ((PI_PDISK_INFO_REQ *)reqPacket.pPacket)->id = ptrList->list[count];

            /*
             * Issue the command through the packet command handler
             */
            rc = PortServerCommandHandler(&reqPacket, &rspPacket);

            /*
             * If the request for the information was successful
             * copy the new data into the response packet.  Otherwise
             * break out of the loop and return an error to the caller.
             */
            if (rc == PI_GOOD)
            {
                memcpy(&ptrOutPkt->bayInfo[count],
                       rspPacket.pPacket, sizeof(PI_DISK_BAY_INFO_RSP));
            }
            else
            {
                break;
            }

            if ((rspPacket.pPacket != NULL) && (rc != PI_TIMEOUT))
            {
                Free(rspPacket.pPacket);
            }
        }
    }

    /*
     * Free the allocated memory
     */
    Free(ptrList);
    Free(reqPacket.pHeader);
    Free(reqPacket.pPacket);
    Free(rspPacket.pHeader);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPacket.pPacket);
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
        pRspPacket->pHeader->errorCode = rc;
    }

    return (rc);
}


/*----------------------------------------------------------------------------
** Function:    PI_DiskBayAlarmControl
**
** Description: Disk bay alarm control
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_DiskBayAlarmControl(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    PSES_DEVICE pSES;
    INT32       rc = PI_GOOD;

    /*
     * Get the pointer to the list of disk bays (SES enclosures).
     */
    pSES = GetSESList();

    /*
     * Search the SES list to find the entry that matches the requested ID.
     */
    while ((pSES != NULL) &&
           (pSES->PID != ((PI_DISK_BAY_ALARM_CONTROL_REQ *)(pReqPacket->pPacket))->id))
    {
        pSES = pSES->NextSES;
    }

    /*
     * If a valid entry was found issue the alarm request.
     */
    if (pSES != NULL)
    {
        SESAlarmCtrl(pSES,
                     ((PI_DISK_BAY_ALARM_CONTROL_REQ *)(pReqPacket->pPacket))->setting);
    }
    else
    {
        rc = PI_ERROR;
    }

    /*
     * Attach the return data packet (if applicable) to the main response
     * packet.  Fill in the header length and status fields.
     */
    pRspPacket->pPacket = NULL;
    pRspPacket->pHeader->length = 0;
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = PI_GOOD;

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PI_DiskBaySESEnviro
**
** Description: Extract information and environmental data from the SES
**              data structures in the CCB
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_DiskBaySESEnviro(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    PSES_DEVICE pSES;
    UINT8      *pOutPkt = NULL;
    PI_DISK_BAY_ENVIRO_PART2_RSP *pPart2 = NULL;
    SES_PAGE_02 *ptrPage2Out = NULL;
    SES_WWN_TO_SES *pDevMap = NULL;
    INT32       rc = PI_ERROR;
    UINT32      responseLength = 0;
    UINT32      numElements = 0;
    UINT32      mapLength = 0;

    /*
     * Initialize return variables.  These values will change if the
     * requested device is found.
     */
    pRspPacket->pPacket = NULL;
    pRspPacket->pHeader->length = 0;
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = DEINVPID;

    /*
     * Get the pointer to the list of disk bays (SES enclosures).
     */
    pSES = GetSESList();

    /*
     * Search the SES list to find the entry that matches the requested ID.
     */
    while ((pSES != NULL) &&
           (pSES->PID != ((PI_DISK_BAY_ENVIRO_REQ *)(pReqPacket->pPacket))->id))
    {
        pSES = pSES->NextSES;
    }

    /*
     * If a valid entry was found package up the return data.
     */
    if (pSES != NULL)
    {
        /*
         * Get the device map for this disk bay.  The map contains
         * the WWN, PID and slot for each PDisk in the bay.
         */
        mapLength = GetDeviceMap(pSES, &pDevMap);

        /*
         * The number of Control Elements is returned in the Device structure
         * as TotalSlots.  This value is only valid if pOldPage2 is valid.
         */
        if (pSES->OldPage2 != NULL)
        {
            numElements = pSES->TotalSlots;
        }

        /*
         * This is a PI command, do the same as if we had
         * X1PktGetRmcCompatibility() > 9.
         */
        responseLength = sizeof(SES_DEVICE) +
            sizeof(*pPart2) + mapLength + (numElements * sizeof(SES_ELEM_CTRL));

        pOutPkt = MallocWC(responseLength);

        /*
         * Copy the SES device structure into the output buffer.
         */
        memcpy(pOutPkt, pSES, sizeof(SES_DEVICE));

        /*
         * Get a pointer to the memory following SES_DEVICE.
         * This is where the remaining data will be copied.
         */
        /* (UINT8*)pPart2 = pOutPkt + sizeof(SES_DEVICE); */
        pPart2 = (PI_DISK_BAY_ENVIRO_PART2_RSP *)(pOutPkt + sizeof(SES_DEVICE_OLD_2));

        /*
         * Fill in the lengths of the 2 structs that follow.
         */
        pPart2->mapLength = mapLength;
        pPart2->page2Length = numElements * sizeof(SES_ELEM_CTRL);

        /*
         * Copy the device map into the output buffer.
         */
        if (mapLength > 0)
        {
            memcpy(pPart2->devMap, pDevMap, mapLength);
        }

        /*
         * The response data structure PI_DISK_BAY_ENVIRO_RSP contains
         * 2 variable length structures.  This means that page2 can not
         * be referenced using standard structure references.
         * Page2 is located immediately after the device map.
         * Make sure there are valid page 2 elements before executing
         * the memcpy.
         */

        /*
         * Point to the start of the page2 atructure.
         */
        /* (UINT8*)ptrPage2Out = ((UINT8*)(pPart2->devMap)) + mapLength; */
        ptrPage2Out = (SES_PAGE_02 *)(((UINT8 *)(pPart2->devMap)) + mapLength);

        if (numElements > 0)
        {
            memcpy(ptrPage2Out, pSES->OldPage2,
                   sizeof(SES_PAGE_02) + (numElements * sizeof(SES_ELEM_CTRL)));

            /*
             * The Page 2 data is returned in SCSI byte order so we
             * need to swap bytes in certain cases.  This gets a little
             * sticky for UINT16 values down in the page 2 control
             * element data.  Since I am just passing the data up and
             * not parsing through it, I will leave the UMC code to
             * handle the swapping of the control element data as needed.
             */
            ptrPage2Out->Length = bswap_16(ptrPage2Out->Length);
            ptrPage2Out->Generation = bswap_32(ptrPage2Out->Generation);
        }
        else
        {
            /*
             * If page 2 is not available fill in default values.
             */
            ptrPage2Out->PageCode = 0x02;
            ptrPage2Out->Status = 0;
            ptrPage2Out->Length = 0;
            ptrPage2Out->Generation = 0;
        }

        /*
         * Fill in the response packet values.
         */
        pRspPacket->pPacket = pOutPkt;
        pRspPacket->pHeader->length = responseLength;
        pRspPacket->pHeader->status = PI_GOOD;
        pRspPacket->pHeader->errorCode = PI_GOOD;

        /*
         * Free the memory allocated inside GetDeviceMap()
         */
        Free(pDevMap);
    }

    return (pRspPacket->pHeader->status);
}

/*----------------------------------------------------------------------------
** Function:    PI_CtrlAndBayEnviro
**
** Description: This function returns the controller and bays stats using the
**              packet defined for the X1 interface (X1_ENV_STATS_RSP).
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**--------------------------------------------------------------------------*/
INT32 PI_CtrlAndBayEnviro(UNUSED XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    UINT8      *pBuf;

    /* Allocate enough memory to hold the entire X1_ENV_STATS_RSP struct. */
    pBuf = MallocWC(sizeof(X1_ENV_STATS_RSP));

    /*
     * TBolt00019426
     * Cache information is not being populated on the PIGETX1ENV PI
     * request but it is being populated on the X1 request. Therefore, cache
     * is being used to get refreshed information on PI request.
     */
    CacheSetRefreshFlag(CACHE_INVALIDATE_ENV);
    CacheSetRefreshFlag(CACHE_INVALIDATE_DISKBAY);      /*MRS - Is this needed? */

    EnvironmentalGetStats(pBuf);

    /* Allocate a new buffer for the actual response size (minus header). */
    pRspPacket->pHeader->length = sizeof(X1_ENV_STATS_RSP_PKT);
    pRspPacket->pPacket = MallocWC(pRspPacket->pHeader->length);

    /* NOTE: the structure is the same, but the header is missing in the response. */
    /* To fix this requires changing all references to the structure in SPAL. */
    memcpy(pRspPacket->pPacket, pBuf + sizeof(X1_RSP_HDR), pRspPacket->pHeader->length);

    pRspPacket->pHeader->status = PI_GOOD;
    pRspPacket->pHeader->errorCode = PI_GOOD;

    /* Free the temporary buffer. The packet code will free pRspPacket->pPacket. */
    Free(pBuf);

    return (PI_GOOD);
}

/*----------------------------------------------------------------------------
** Name:    DiskBays
**
** Desc:    This method will retrieve disk bay information for all disk bays.
**
** Inputs:  NONE
**
** Returns: Disk Bays response packet.
**
** WARNING: The caller of this method will need to free the response packet
**          after they have finished using it.
**--------------------------------------------------------------------------*/
PI_DISK_BAYS_RSP *DiskBays(void)
{
    INT32       rc = PI_GOOD;
    XIO_PACKET  reqPacket;
    XIO_PACKET  rspPacket;
    PI_DISK_BAYS_RSP *pResponse = NULL;

    /*
     * Allocate memory for the request (header and data) and the
     * response header. The response data will be allocated in the called
     * function.
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    reqPacket.pPacket = NULL;
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    rspPacket.pPacket = NULL;
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    /*
     * Fill in the Header
     */
    reqPacket.pHeader->commandCode = PI_DISK_BAYS_CMD;
    reqPacket.pHeader->length = 0;

    /*
     * Issue the command through the packet command handler
     */
    rc = PortServerCommandHandler(&reqPacket, &rspPacket);

    if (rc == PI_GOOD)
    {
        ccb_assert(rspPacket.pPacket != NULL, rspPacket.pPacket);

        pResponse = (PI_DISK_BAYS_RSP *)rspPacket.pPacket;
        rspPacket.pPacket = NULL;
    }
    else
    {
        /* WELL I FAILED TO GET THE LIST, WHATCHA GONA DO NOW? */
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

    return pResponse;
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
