/* $Id: fibreshim.c 144191 2010-07-15 20:23:53Z steve_wirtz $ */
/**
******************************************************************************
**
**  @file   fibreshim.c
**
**  @brief  Fibre Channnel shim communications implementation
**
**  The fibre channel shim is responsible for interprocessor
**  communincations using the Front End Fibre connection.
**  This is meant as a back-up to the ethernet communications.
**
** Copyright (c) 2001-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#include "fibreshim.h"

#include "XIO_Std.h"
#include "debug_files.h"
#include "i82559.h"
#include "ipc_session_manager.h"
#include "kernel.h"
#include "MR_Defs.h"
#include "PI_Utils.h"
#include "PortServer.h"
#include "quorum.h"
#include "quorum_utils.h"

#include <byteswap.h>

/*****************************************************************************
** Private defines
*****************************************************************************/
#define FE_PROC             0x00
#define CCB_PROC            0xFE

#define DLM_IPC             0x02

#define DLM_SP_ERROR_STATUS 0x05

typedef struct _DG_RESPONSE_HEADER
{
    UINT8       status;         /* Server Processor Code                */
    UINT8       headerLength;   /* Length of Header                     */
    UINT16      sequenceNumber; /* Message Sequence Number              */
    UINT8       errorCode1;     /* Error Code   byte 1                  */
    UINT8       errorCode2;     /* Error Code   byte 2                  */
    UINT8       userSpecific1;  /* User Specific 1                      */
    UINT8       userSpecific2;  /* User Specific 2                      */
    UINT32      responseLength; /* Response length                      */
    UINT32      headerCRC;      /* Header CRC - filled in by LLD        */
} DG_RESPONSE_HEADER;

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static void SendDLMPacket(TASK_PARMS *parms);

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
**  Function Name: FibreShimHandler
**
**  Description:
**      Receive a DLM packet, strip the DLM header and pass it on to the
**      TCP/IP stack.
**
**  Inputs:    dlmPacketPtr     = DLM Packet pointer
**
**  Returns:   status           - 0 = Good
**
**--------------------------------------------------------------------------*/
int FibreShimHandler(DLM_REQUEST_PACKET *dlmRequestPtr)
{
    IPC_PACKET_HEADER *ipcHeader = NULL;
    IPC_PACKET        *rx = NULL;
    DLM_PACKET        *dlmPacketPtr = NULL;
    DG_RESPONSE_HEADER *dlmRespPtr = NULL;

    if (dlmRequestPtr == NULL)
    {
        dprintf(DPRINTF_SHIM, "ERROR: NULL DLM Request Pointer\n");
        return (-1);
    }

    /*
     * Get the DLM request packet pointer
     */
    dlmPacketPtr = (DLM_PACKET *)dlmRequestPtr->requestAddr;
    dlmRespPtr = (DG_RESPONSE_HEADER *)dlmRequestPtr->responseAddr;

    if (dlmPacketPtr == NULL)
    {
        dprintf(DPRINTF_SHIM, "ERROR: NULL DLM Packet Pointer\n");
        return (-1);
    }

    /*
     * Determine the type of packet and handle accordingly
     */
    if (dlmPacketPtr->header.userSpecific == DLM_IPC)
    {
        dprintf(DPRINTF_SHIM, "==== Shim Packet Received (%d:%hu:%d)\n",
                ((IPC_PACKET_HEADER *)(dlmPacketPtr->data))->commandCode,
                ((IPC_PACKET_HEADER *)(dlmPacketPtr->data))->sequenceNumber,
                ((UINT32 *)&((IPC_PACKET_HEADER *)(dlmPacketPtr->data))->timeStamp)[1]);

        /*
         * Point to the data portion of the DLM  (header for IPC packet).
         */
        ipcHeader = (IPC_PACKET_HEADER *)dlmPacketPtr->data;

        /*
         * Strip the DLM header and form into an IPC packet
         */
        rx = CreatePacket(ipcHeader->commandCode, ipcHeader->length, __FILE__, __LINE__);

        /*
         * copy the header into the receive header
         */
        memcpy(rx->header, ipcHeader, sizeof(IPC_PACKET_HEADER));

        /*
         * copy the data into the location of the receive data
         */
        memcpy(rx->data, ((char *)ipcHeader) + sizeof(IPC_PACKET_HEADER),
               ipcHeader->length);

        /*
         * Queue the packet to the IPC session manager
         */
        SendResponseQueueDataPacketToSm(rx, SENDPACKET_FIBRE);

        /*
         * Clear out the response packet header and set the header length
         */
        if (dlmRespPtr != NULL)
        {
            memset(dlmRespPtr, 0x00, sizeof(DG_RESPONSE_HEADER));
            dlmRespPtr->headerLength = sizeof(DG_RESPONSE_HEADER);
        }
    }
    else
    {
        dprintf(DPRINTF_SHIM, "Unsupported Packet Type (0x%x) detected by Shim Driver\n",
                dlmPacketPtr->header.userSpecific);

        /*
         * Report DLM error
         */
        if (dlmRespPtr != NULL)
        {
            /*
             * Clear out the response packet header and set the header length
             */
            memset(dlmRespPtr, 0x00, sizeof(DG_RESPONSE_HEADER));
            dlmRespPtr->headerLength = sizeof(DG_RESPONSE_HEADER);
            dlmRespPtr->status = DLM_SP_ERROR_STATUS;
        }
        return (-1);
    }
    return (0);
}


/*----------------------------------------------------------------------------
**  Function Name: BuildDLMPacket
**
**  Description:
**      Builds a DLM packet for sending across the Fibre interface and forks
**      task to transmit to the link layer.
**
**  Inputs:     destSN      = serail number of destination controller
**              headerPtr   = pointer to the header
**              headerLen   = length of header
**              dataPtr     = pointer to the data
**              dataLen     = length of data
**              packetType  = type of packet
**  Returns:    0 = Good
**
**--------------------------------------------------------------------------*/
static void BuildDLMPacket(UINT32 destSN, char *headerPtr, UINT32 headerLen,
                           char *dataPtr, UINT32 dataLen, char packetType)
{
    DLM_PACKET *packetPtr;
    static UINT16 gSeqNumber = 0;       /* Fibre packet sequence Number     */
    TASK_PARMS  parms;

    /*
     * Check Parameters
     */
    if ((headerLen && !headerPtr) ||
        (dataLen && !dataPtr) || (!headerLen && !dataLen) || (!destSN))
    {
        dprintf(DPRINTF_SHIM, "ERROR: Invalid Parms in BuildDLMPacket (destSN: %u, headerPtr: %p, headerLen: %u, dataPtr: %p, dataLen: %u)\n",
                destSN, headerPtr, headerLen, dataPtr, dataLen);
        return;
    }
    /*
     * Allocate memory for the DRP packet (header and data)
     */
    packetPtr = MallocSharedWC(sizeof(DLM_PACKET_HEADER) + headerLen + dataLen);

    /*
     * Fill in the header
     */
    packetPtr->header.serverProcCode = CCB_PROC;
    packetPtr->header.headerLength = sizeof(DLM_PACKET_HEADER);
    packetPtr->header.sequenceNumber = gSeqNumber++;
    packetPtr->header.functionCode = DLM_SEND_ETHERNET;
    packetPtr->header.pathNumber = 0xFF;
    packetPtr->header.destSerialNumber = bswap_32(destSN);
    memcpy(packetPtr->header.destServerName, "CCB0", 4);
    packetPtr->header.dataLength = headerLen + dataLen;
    packetPtr->header.userSpecific = packetType;
    packetPtr->header.headerCRC = 0;

    /*
     * Copy the passed header and data into the DLM payload.
     */
    if (headerLen)
    {
        memcpy(packetPtr->data, headerPtr, headerLen);
    }

    if (dataLen)
    {
        memcpy(packetPtr->data + headerLen, dataPtr, dataLen);
    }

    /*
     * Send the packet to the link layer
     */
    dprintf(DPRINTF_SHIM, "Forking SendDLMPacket (%d:%hu)\n",
            ((IPC_PACKET_HEADER *)(headerPtr))->commandCode,
            ((IPC_PACKET_HEADER *)(headerPtr))->sequenceNumber);

    parms.p1 = (UINT32)packetPtr;
    parms.p2 = ((UINT32)(headerLen + dataLen + sizeof(DLM_PACKET_HEADER)));
    TaskCreate(SendDLMPacket, &parms);
}


/*----------------------------------------------------------------------------
**  Function Name: SendDLMPacket
**
**  Description:
**      Sends a DLM packet to the Link Layer.
**
**  Inputs:    dummy        = required to fork
**             packetPtr    = pointer to the packet to send
**             length       = length of packet to send
**
**--------------------------------------------------------------------------*/
static void SendDLMPacket(TASK_PARMS *parms)
{
    DG_RESPONSE_HEADER *ptrRespPkt;
    int         rc;
    DLM_PACKET *packetPtr = (DLM_PACKET *)parms->p1;
    int         length = (int)parms->p2;

    dprintf(DPRINTF_SHIM, "===== Shim Packet MRP Issued (%d:%hu)\n",
            ((IPC_PACKET_HEADER *)(packetPtr->data))->commandCode,
            ((IPC_PACKET_HEADER *)(packetPtr->data))->sequenceNumber);

    /*
     * Allocate memory for MRP output packets.
     */
    ptrRespPkt = MallocSharedWC(sizeof(*ptrRespPkt));

    /*
     * Send the request to Thunderbolt.  This function handles timeout
     * conditions and task switches while waiting.
     */
    rc = PI_ExecMRP(packetPtr, length, MRCCBTODLM,
                    ptrRespPkt, sizeof(*ptrRespPkt), MRP_STD_TIMEOUT);

    dprintf(DPRINTF_SHIM, "===== Shim Packet MRP Completed (%d:%hu)\n",
            ((IPC_PACKET_HEADER *)(packetPtr->data))->commandCode,
            ((IPC_PACKET_HEADER *)(packetPtr->data))->sequenceNumber);

    /*
     * Only free the memory if the request did NOT timeout.  On a timeout
     * the memory must remain available in case the request eventually
     * completes.
     */
    if (rc != PI_TIMEOUT)
    {
        if (ptrRespPkt->status)
        {
            /*
             * Communications failed - log message (TWSDEBUG)
             */
            LogMessage(LOG_TYPE_DEBUG, "DLM Communications Failure (status: %u, err1: %u, err2: %u)",
                       ptrRespPkt->status,
                       ptrRespPkt->errorCode1, ptrRespPkt->errorCode2);

        }

        Free(ptrRespPkt);

        if (packetPtr)
        {
            Free(packetPtr);
        }
    }

    if (rc)
    {
        LogMessage(LOG_TYPE_DEBUG, "Send DLM Packet Failure (0x%x)", rc);
    }
}


/*----------------------------------------------------------------------------
**  Function Name: SendDLMDirect
**
**  Description:
**      This routine sends an IPC packet directly through the DLM, bypassing
**      the TCP/IP stack.
**
**  Inputs:    destinationSN    = destination Serial Number
**             tx               = transmit packet
**  Returns:   status           - 0 = Good
**--------------------------------------------------------------------------*/
int SendDLMDirect(UINT32 destinationSN, IPC_PACKET *tx)
{
    dprintf(DPRINTF_IPC_MSG_DELIVERY,
            "Sending IPC cmd %u to crtlSn: %u over Fibre\n",
            tx->header->commandCode, destinationSN);

    /*
     * Build DLM packet and send the packet to the link layer
     */
    BuildDLMPacket(destinationSN,
                   (char *)tx->header,
                   sizeof(IPC_PACKET_HEADER),
                   (char *)tx->data, tx->header->length, DLM_IPC);

    return (0);
}

/*----------------------------------------------------------------------------
**  Function Name: CheckFibrePath
**
**  Description:
**      This request checks fibre communications with another controller.
**
**  Inputs:    controllerSN = controller to establish connection with
**
**  Returns:   GOOD  - communications path is up
**             ERROR - communications path is down
**
**--------------------------------------------------------------------------*/
UINT32 CheckFibrePath(UINT32 controllerSN)
{
    MRQFECC_REQ *packetPtr = NULL;
    MRQFECC_RSP *ptrRespPkt = NULL;
    UINT32      rc;
    UINT32      retVal = ERROR;

    /*
     * Allocate memory for the DRP packet header
     */
    packetPtr = MallocWC(sizeof(*packetPtr));
    ptrRespPkt = MallocSharedWC(sizeof(*ptrRespPkt));

    /*
     * Fill in the request
     */
    packetPtr->serial = controllerSN;

    /*
     * Send the request to Thunderbolt.  This function handles timeout
     * conditions and task switches while waiting.
     */
    rc = PI_ExecMRP(packetPtr, sizeof(*packetPtr), MRQFECC,
                    ptrRespPkt, sizeof(*ptrRespPkt), MRP_STD_TIMEOUT);

    Free(packetPtr);

    if (rc != PI_TIMEOUT)
    {
        /*
         * Set the return value based on the status from the MRP (if
         * the fibre connection is GOOD).
         */

        if ((rc == PI_GOOD) && (ptrRespPkt->header.status == GOOD))
        {
            retVal = GOOD;
        }

        /*
         * Only free the memory if the request did NOT timeout.  On a timeout
         * the memory must remain available in case the request eventually
         * completes.
         */
        Free(ptrRespPkt);
    }

    return (retVal);
}

#if 0

/*----------------------------------------------------------------------------
**  Function Name: FibrePath
**
**  Description:
**      This request establishes communications with another controller.
**
**  Inputs:    controllerSN = controller to establish connection with
**             opType       = operation code
**                              0x00 = Poll a datagram path
**                              0x01 = Establish a communications path
**                              0x02 = Terminate a communications path
**
**  Outputs:   0 = successful completion
**
**--------------------------------------------------------------------------*/
int FibrePath(UINT32 controllerSN, UINT8 opType)
{
    DLM_PACKET *packetPtr = NULL;
    DG_RESPONSE_HEADER *ptrRespPkt = NULL;
    UINT32      destSN = GetMyControllerSN();

    int         rc;

    /*
     * Allocate memory for the DRP packet header
     */
    packetPtr = MallocWC(sizeof(DLM_PACKET_HEADER));    /* This is right. */
    ptrRespPkt = MallocSharedWC(sizeof(DG_RESPONSE_HEADER));

    /*
     * Fill in the header
     */
    packetPtr->header.serverProcCode = FE_PROC;
    packetPtr->header.headerLength = sizeof(DLM_PACKET_HEADER);
    packetPtr->header.sequenceNumber = 0;
    packetPtr->header.functionCode = opType;
    packetPtr->header.pathNumber = 0xFF;
    packetPtr->header.destSerialNumber = bswap_32(destSN);
    memcpy(packetPtr->header.destServerName, "DLM1", 4);
    packetPtr->header.dataLength = 0;
    packetPtr->header.userSpecific = controllerSN;
    packetPtr->header.headerCRC = 0;
    /*
     * Send the request to Thunderbolt.  This function handles timeout
     * conditions and task switches while waiting.
     */
    rc = PI_ExecMRP(packetPtr, sizeof(DLM_PACKET_HEADER), MRCCBTODLM,
                    ptrRespPkt, sizeof(DG_RESPONSE_HEADER), MRP_STD_TIMEOUT);

    /*
     * Only free the memory if the request did NOT timeout.  On a timeout
     * the memory must remain available in case the request eventually
     * completes.
     */
    if (rc != PI_TIMEOUT)
    {
        if (ptrRespPkt != NULL)
        {
            if (ptrRespPkt->status)
            {
                /*
                 * Communications failed - log message (TWSDEBUG)
                 */
                dprintf(DPRINTF_SHIM, "DLM Path Failure (status: %u, err1: %u, err2: %u)\n",
                        ptrRespPkt->status,
                        ptrRespPkt->errorCode1, ptrRespPkt->errorCode2);

            }
            if (ptrRespPkt)
            {
                Free(ptrRespPkt);
                ptrRespPkt = NULL;
            }
            if (packetPtr)
            {
                Free(packetPtr);
            }
        }
    }

    if (rc)
    {
        dprintf(DPRINTF_SHIM, "Fibre Path Packet Failure (0x%x)\n", rc);
    }

    return (rc);
}
#endif  /* 0 */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
