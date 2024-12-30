/* $Id: PI_Target.c 156532 2011-06-24 21:09:44Z m4 $*/
/*===========================================================================
** FILE NAME:       PI_Target.c
** MODULE TITLE:    Packet Interface for Target Commands
**
** DESCRIPTION:     Handler functions for Target request packets.
**
** Copyright (c) 2001-2009 XIOtech Corporation. All rights reserved.
**==========================================================================*/

#include "AsyncEventHandler.h"
#include "CacheSize.h"
#include "CacheTarget.h"
#include "CmdLayers.h"
#include "debug_files.h"
#include "ipc_sendpacket.h"
#include "LOG_Defs.h"
#include "misc.h"
#include "mode.h"
#include "MR_Defs.h"
#include "PacketInterface.h"
#include "PI_CmdHandlers.h"
#include "PI_Utils.h"
#include "PktCmdHdl.h"
#include "quorum_utils.h"
#include "XIO_Std.h"
#include "PI_Target.h"

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
** Function:    PI_Targets
**
** Description: Targets Request Handler
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_Targets(UNUSED XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    MR_LIST_RSP *ptrList = NULL;
    UINT16      count = 0;
    PI_TARGETS_RSP *ptrOutPkt = NULL;
    UINT32      outPktSize = 0;
    XIO_PACKET  reqPacket = { NULL, NULL };
    XIO_PACKET  rspPacket = { NULL, NULL };

    dprintf(DPRINTF_PI_COMMANDS, "PI_Targets - MRPs issued\n");

    /*
     * Get the list of objects.  Always start at the beginning and return
     * the entire list.
     */
    ptrList = PI_GetList(0, (MRGETTLIST | GET_LIST));

    /* If we could get the list use it, otherwise signal an error */
    if (ptrList != NULL)
    {
        /*
         * Calculate the size of the output packet.  This will be the size
         * of the targets (multiple targets) response packet plus the size
         * of a target info (single target) response packet for each target.
         */
        outPktSize = sizeof(*ptrOutPkt) + (ptrList->ndevs * sizeof(PI_TARGET_INFO_RSP));

        ptrOutPkt = MallocWC(outPktSize);

        /* Save the number of devices in the output packet */
        ptrOutPkt->count = ptrList->ndevs;

        /*
         * Allocate memory for the request (header and data) and the
         * response header. The response data will be allocated in the called
         * function.
         */
        reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
        reqPacket.pPacket = MallocWC(sizeof(PI_TARGET_INFO_REQ));
        rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
        reqPacket.pHeader->packetVersion = 1;
        rspPacket.pHeader->packetVersion = 1;

        /*
         * Fill in the Header
         */
        reqPacket.pHeader->commandCode = PI_TARGET_INFO_CMD;
        reqPacket.pHeader->length = sizeof(PI_TARGET_INFO_REQ);

        for (count = 0; count < ptrList->ndevs; count++)
        {
            rspPacket.pPacket = NULL;

            /* Setup the ID for this Target Information Request */
            ((PI_TARGET_INFO_REQ *)reqPacket.pPacket)->id = ptrList->list[count];

            /*
             * Issue the command through the packet command handler
             */
            rc = PortServerCommandHandler(&reqPacket, &rspPacket);

            /*
             * If the request for the target information was successful
             * copy the new data into the response packet.  Otherwise
             * break out of the loop and return an error to the caller.
             */
            if (rc == PI_GOOD)
            {
                memcpy(&ptrOutPkt->targetInfo[count],
                       rspPacket.pPacket, sizeof(PI_TARGET_INFO_RSP));
            }
            else
            {
                break;
            }

            /*
             * Free the response packet memory here.  It gets allocated
             * on each call to PacketCommandHandler().
             */
            if (rc != PI_TIMEOUT)
            {
                Free(rspPacket.pPacket);
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

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    PI_TargetResourceList
**
** Description: Get the list of target resources
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     PI_GOOD or PI_ERROR
**
**--------------------------------------------------------------------------*/
INT32 PI_TargetResourceList(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    MRGETTRLIST_RSP *ptrListOut = NULL;
    UINT32      listOutSize;
    UINT16      numDevs;
    UINT16      entrySize;
    INT32       rc = PI_GOOD;

    /*
     * Assume the max number of targets when allocating memory (hey we've
     * got plenty!).  This should eliminate multiple requests and improve
     * performance (somewhat).
     */
    numDevs = MAX_TARGETS;
    entrySize = MRGETTRLIST_MAX_ENTRY_SIZE;

    do
    {
        listOutSize = sizeof(*ptrListOut) + (numDevs * entrySize);

        /*
         * If an output list was previously allocated, free it before
         * allocating a new one.
         */
        Free(ptrListOut);

        ptrListOut = MallocSharedWC(listOutSize);

        /*
         * Send the request to Thunderbolt.  This function handles timeout
         * conditions and task switches while waiting.
         */
        rc = PI_ExecMRP(pReqPacket->pPacket, pReqPacket->pHeader->length,
                        MRGETTRLIST, ptrListOut, listOutSize, GetGlobalMRPTimeout());

        /*
         * Save the number of devices in case we need to make the
         * request again.  Also grab the size of each entry from the
         * response packet.
         */
        numDevs = ptrListOut->ndevs;
        entrySize = ptrListOut->entrySize;

    } while (rc == PI_ERROR && ptrListOut->header.status == DETOOMUCHDATA);

    if (rc == PI_GOOD)
    {
        /*
         * Recalculate the size of the response data - we need the amount
         * to actually return since the original allocation was for the
         * max amount.
         */
        listOutSize = sizeof(*ptrListOut) + (numDevs * entrySize);

        pRspPacket->pHeader->length = listOutSize;
        pRspPacket->pHeader->status = PI_GOOD;
        pRspPacket->pHeader->errorCode = ptrListOut->header.status;
        pRspPacket->pPacket = (UINT8 *)ptrListOut;
    }
    else
    {
        /*
         * Indicate an error condition and no return data in the header.
         */
        pRspPacket->pHeader->length = 0;
        pRspPacket->pHeader->status = PI_ERROR;

        /*
         * If we return NULL the caller can't free the memory so do it here.
         * Only free the memory if the request did NOT timeout.  On a timeout
         * the memory must remain available in case the request eventually
         * completes.
         */
        if (rc != PI_TIMEOUT)
        {
            Free(ptrListOut);
        }
    }

    return (rc);
}


/*----------------------------------------------------------------------------
** Function:    Targets
**
** Description: Get the information for all the targets on the given
**              controller.
**
** Inputs:      UINT32 controllerSN - Serial number for the controller
**
** Returns:     Pointer to a targets response packet or NULL if they
**              could not be retrieved.
**
** WARNING:     The pointer to the response packet needs to be freed by the
**              caller or it will be leaked.
**--------------------------------------------------------------------------*/
PI_TARGETS_RSP *Targets(UINT32 controllerSN)
{
    INT32       rc = PI_GOOD;
    XIO_PACKET  reqPacket = { NULL, NULL };
    XIO_PACKET  rspPacket = { NULL, NULL };
    PI_TARGETS_RSP *pResponse = NULL;

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
    reqPacket.pHeader->commandCode = PI_TARGETS_CMD;
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
        pResponse = (PI_TARGETS_RSP *)rspPacket.pPacket;
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

/*----------------------------------------------------------------------------
** Function:    TargetResourceList
**
** Description: Get the target resource list for the given target and list type.
**
** Inputs:      tid - Target ID
**              type - Type of resource list
**
** Returns:     Pointer to a target resource response packet or NULL if the
**              list could not be retrieved.
**
** WARNING:     The pointer to the response packet needs to be freed by the
**              caller or it will be leaked.
**--------------------------------------------------------------------------*/
PI_TARGET_RESOURCE_LIST_RSP *TargetResourceList(UINT16 tid, UINT8 type)
{
    INT32       rc = PI_GOOD;
    XIO_PACKET  reqPacket = { NULL, NULL };
    XIO_PACKET  rspPacket = { NULL, NULL };
    PI_TARGET_RESOURCE_LIST_RSP *pResponse = NULL;

    /*
     * Allocate memory for the request (header and data) and the
     * response header. The response data will be allocated in the called
     * function.
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    reqPacket.pPacket = MallocWC(sizeof(PI_TARGET_RESOURCE_LIST_REQ));
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    /*
     * Fill in the Header
     */
    reqPacket.pHeader->commandCode = PI_TARGET_RESOURCE_LIST_CMD;
    reqPacket.pHeader->length = sizeof(PI_TARGET_RESOURCE_LIST_REQ);

    /* Setup the packet for this target resource list request */
    ((PI_TARGET_RESOURCE_LIST_REQ *)reqPacket.pPacket)->sid = 0;
    ((PI_TARGET_RESOURCE_LIST_REQ *)reqPacket.pPacket)->tid = tid;
    ((PI_TARGET_RESOURCE_LIST_REQ *)reqPacket.pPacket)->listType = type;

    /*
     * Issue the command through the packet command handler
     */
    rc = PortServerCommandHandler(&reqPacket, &rspPacket);

    if (rc == PI_GOOD)
    {
        pResponse = (PI_TARGET_RESOURCE_LIST_RSP *)rspPacket.pPacket;
        rspPacket.pPacket = NULL;
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

    return (pResponse);
}

/*----------------------------------------------------------------------------
** Function:    FailPort
**
** Description: Get the information for a target.
**
** Inputs:      UINT16 port - Port ID
**
** Returns:     Pointer to a target response packet or NULL if it
**              could not be retrieved.
**
** WARNING:     The pointer to the response packet needs to be freed by the
**              caller or it will be leaked.
**--------------------------------------------------------------------------*/
INT32 FailPort(UINT32 port)
{
    INT32       rc = PI_GOOD;
    XIO_PACKET  reqPacket = { NULL, NULL };
    XIO_PACKET  rspPacket = { NULL, NULL };

    /*
     * Allocate memory for the request (header and data) and the
     * response header. The response data will be allocated in the called
     * function.
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    reqPacket.pPacket = MallocWC(sizeof(PI_PROC_FAIL_PORT_REQ));
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    /*
     * Fill in the Header
     */
    reqPacket.pHeader->commandCode = PI_PROC_FAIL_PORT_CMD;
    reqPacket.pHeader->length = sizeof(PI_PROC_FAIL_PORT_REQ);

    /* Setup the packet for this request */
    ((PI_PROC_FAIL_PORT_REQ *)reqPacket.pPacket)->option = 0;
    ((PI_PROC_FAIL_PORT_REQ *)reqPacket.pPacket)->fail = TRUE;
    ((PI_PROC_FAIL_PORT_REQ *)reqPacket.pPacket)->qualifier = 0;
    ((PI_PROC_FAIL_PORT_REQ *)reqPacket.pPacket)->port = (UINT16)port;

    /*
     * Issue the command through the packet command handler
     */
    rc = PortServerCommandHandler(&reqPacket, &rspPacket);

    dprintf(DPRINTF_RM, "FailPort: port %u rc %u\n", port, rc);

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

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    UnfailPort
**
** Description: Get the information for a target.
**
** Inputs:      UINT16 port - Port ID
**
** Returns:     Pointer to a target response packet or NULL if it
**              could not be retrieved.
**
** WARNING:     The pointer to the response packet needs to be freed by the
**              caller or it will be leaked.
**--------------------------------------------------------------------------*/
INT32 UnfailPort(UINT32 port)
{
    INT32       rc = PI_GOOD;
    XIO_PACKET  reqPacket = { NULL, NULL };
    XIO_PACKET  rspPacket = { NULL, NULL };

    /*
     * Allocate memory for the request (header and data) and the
     * response header. The response data will be allocated in the called
     * function.
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    reqPacket.pPacket = MallocWC(sizeof(PI_PROC_FAIL_PORT_REQ));
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    /*
     * Fill in the Header
     */
    reqPacket.pHeader->commandCode = PI_PROC_FAIL_PORT_CMD;
    reqPacket.pHeader->length = sizeof(PI_PROC_FAIL_PORT_REQ);

    /* Setup the packet for this request */
    ((PI_PROC_FAIL_PORT_REQ *)reqPacket.pPacket)->option = 0;
    ((PI_PROC_FAIL_PORT_REQ *)reqPacket.pPacket)->fail = FALSE;
    ((PI_PROC_FAIL_PORT_REQ *)reqPacket.pPacket)->qualifier = 0;
    ((PI_PROC_FAIL_PORT_REQ *)reqPacket.pPacket)->port = (UINT16)port;

    /*
     * Issue the command through the packet command handler
     */
    rc = PortServerCommandHandler(&reqPacket, &rspPacket);

    dprintf(DPRINTF_RM, "UnfailPort: port %u rc %u\n", port, rc);

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

    return (rc);
}

/*----------------------------------------------------------------------------
** Function:    FailCtrl
**
** Description: Get the information for a target.
**
** Inputs:      UINT16 port - Port ID
**
** Returns:     Pointer to a target response packet or NULL if it
**              could not be retrieved.
**
** WARNING:     The pointer to the response packet needs to be freed by the
**              caller or it will be leaked.
**--------------------------------------------------------------------------*/
INT32 FailCtrl(UINT32 oldOwner, UINT32 newOwner, UINT32 failBack, UINT32 ports)
{
    INT32       rc = PI_GOOD;
    XIO_PACKET  reqPacket = { NULL, NULL };
    XIO_PACKET  rspPacket = { NULL, NULL };

    /*
     * Allocate memory for the request (header and data) and the
     * response header. The response data will be allocated in the called
     * function.
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    reqPacket.pPacket = MallocWC(sizeof(PI_PROC_FAIL_CTRL_REQ));
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;

    /*
     * Fill in the Header
     */
    reqPacket.pHeader->commandCode = PI_PROC_FAIL_CTRL_CMD;
    reqPacket.pHeader->length = sizeof(PI_PROC_FAIL_CTRL_REQ);

    /* Setup the packet for this request */
    if (failBack != FALSE)
    {
        ((PI_PROC_FAIL_CTRL_REQ *)reqPacket.pPacket)->option = FC_FB;
    }
    else
    {
        ((PI_PROC_FAIL_CTRL_REQ *)reqPacket.pPacket)->option = 0;
    }

    if (ports != 0)
    {
        ((PI_PROC_FAIL_CTRL_REQ *)reqPacket.pPacket)->option |= FC_PORTS;
    }

    ((PI_PROC_FAIL_CTRL_REQ *)reqPacket.pPacket)->qualifier = ports;
    ((PI_PROC_FAIL_CTRL_REQ *)reqPacket.pPacket)->oldOwner = oldOwner;
    ((PI_PROC_FAIL_CTRL_REQ *)reqPacket.pPacket)->newOwner = newOwner;

    /*
     * Issue the command through the packet command handler
     */
    rc = PortServerCommandHandler(&reqPacket, &rspPacket);

    if (failBack != FALSE)
    {
        dprintf(DPRINTF_RM, "UnfailCtrl: 0x%x -> 0x%x rc 0x%x\n", oldOwner, newOwner, rc);
    }
    else
    {
        dprintf(DPRINTF_RM, "FailCtrl: 0x%x -> 0x%x rc 0x%x\n", oldOwner, newOwner, rc);
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

    return (rc);
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
