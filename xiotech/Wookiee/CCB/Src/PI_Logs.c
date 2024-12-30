/* $Id: PI_Logs.c 131143 2010-03-08 16:23:08Z mdr $*/
/**
******************************************************************************
**
**  @file   PI_Logs.c
**
**  @brief  Packet Handler for Logging commands
**
**  Copyright (c) 2001, 2009-2010 XIOtech Corporation.  All rights reserved.
**
******************************************************************************
**/

#include "debug_files.h"
#include "LOG_Defs.h"
#include "kernel.h"
#include "logging.h"
#include "logview.h"
#include "PacketInterface.h"
#include "PI_CmdHandlers.h"
#include "PI_Utils.h"
#include "PortServer.h"
#include "quorum.h"
#include "quorum_utils.h"
#include "XIO_Std.h"
#include "XIO_Macros.h"

/*****************************************************************************
** Private defines
*****************************************************************************/
#define LOG_MEMORY_SIZE     (MPX_MAX_TX_DATA_SIZE - sizeof(PI_LOG_INFO_RSP))

/*****************************************************************************
** Public functions - not externed in any header file
*****************************************************************************/

/*****************************************************************************
** Public variables - externed in the header file
*****************************************************************************/


/*****************************************************************************
** Private function prototypes
*****************************************************************************/

/*****************************************************************************
** Code Start
*****************************************************************************/

/**
******************************************************************************
**
**  @brief  Get requested log information
**
**  @param  pReqPacket - pointer to the request packet
**  @param  pRspPacket - pointer to the response packet
**
**              NOTE: mode support not yet implemented
**              mode    bit 0   0 = send in ASCII format
**                              1 = send in binary format
**                      bit 1   0 = no extended data (ASCII)
**                              1 = send extended data (ASCII)
**                      bit 2   0 = use newest event as start
**                              1 = use sequence number as starting event
**                      bit 3   0 = Normal logs
**                              1 = Debug Logs
**                      bit 4   0 = Use Log Sequence
**                              1 = Use Master Sequence
**
**  @return PI_GOOD or PI_ERROR
**
******************************************************************************
**/
INT32 PI_LogInfoRequest(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32               rc;
    PI_LOG_INFO_REQ     *req;
    PI_LOG_INFO_RSP     *rspDataPtr;
    UINT32              sequenceNumber;
    UINT32              responseLength;
    UINT16              mode;
    UINT32              eventCount;

    /* Grab parms from the request packet. */

    req = (PI_LOG_INFO_REQ *)(pReqPacket->pPacket);
    mode = req->mode;
    eventCount = req->eventCount;
    sequenceNumber = req->sequenceNumber;

    /* Set the response length based on the type of response requested */

    if (mode & MODE_BINARY_MESSAGE)
    {
        responseLength = sizeof(PI_LOG_INFO_MODE1_RSP);
    }
    else
    {
        responseLength = sizeof(PI_LOG_INFO_MODE0_RSP);
    }

    /*
     * Allocate space for the response data packet. Limit the message size
     * and fit as many messages as possible into what is allocated.
     */
    rspDataPtr = MallocWC(responseLength + LOG_MEMORY_SIZE);

    /* Get the log information into the response buffer */

    rc = LogInfoRequest((UINT8 *)rspDataPtr, LOG_MEMORY_SIZE, mode,
            &eventCount, &sequenceNumber, &responseLength);

    /* Fill in the response mode, count, and ending sequence number */
    rspDataPtr->logInfoMode0.mode = mode;
    rspDataPtr->logInfoMode0.eventCount = eventCount;

    if (eventCount)
    {
        rspDataPtr->logInfoMode0.sequenceNumber = sequenceNumber;
    }

    rspDataPtr->logInfoMode0.controllerSN = GetMyControllerSN();
    rspDataPtr->logInfoMode0.vcgID = Qm_GetVirtualControllerSN();

    /*
     * Copy the data into the transmit packet. Set the length of the data
     * packet and fill in the status and error code. Set the pointer to
     * the response data.
     * If an error occured, return no data.
     */
    if (rc == PI_GOOD)
    {
        pRspPacket->pHeader->length = responseLength;
        pRspPacket->pPacket = (UINT8 *)rspDataPtr;
    }
    else
    {
        pRspPacket->pHeader->length = 0;
        pRspPacket->pPacket = NULL;
    }

    pRspPacket->pHeader->errorCode = 0;
    pRspPacket->pHeader->status = rc;

    return rc;
}


/**
******************************************************************************
**
**  @brief  Clear the logs
**
**  @param  pReqPacket - pointer to the request packet
**  @param  pRspPacket - pointer to the response packet
**
**  @return PI_GOOD or PI_ERROR
**
******************************************************************************
**/
INT32 PI_LogClearRequest(UNUSED XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    LogClearRequest();

    /*
     * This function does not return anything to the caller.  Set the
     * length to 0 and the pointer to NULL.
     */
    pRspPacket->pHeader->length = 0;
    pRspPacket->pPacket = NULL;
    pRspPacket->pHeader->errorCode = 0;
    pRspPacket->pHeader->status = PI_GOOD;

    return PI_GOOD;
}


/**
******************************************************************************
**
**  @brief  Create a log entry containing the text string in the request packet
**
**  @param  pReqPacket - pointer to the request packet
**  @param  pRspPacket - pointer to the response packet
**
**  @return PI_GOOD or PI_ERROR
**
******************************************************************************
**/
INT32 PI_LogTextMessage(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    PI_LOG_TEXT_MESSAGE_REQ *req;

    /* Ensure there is a null character at the end of the string */
    req = (PI_LOG_TEXT_MESSAGE_REQ *)pReqPacket->pPacket;
    req->text[MAX_TEXT_MSG_LEN - 1] = '\0';

    /* Log the message */
    rc = LogTextMessage(req->msgType, (char *)req->text);

    /* Fill out the response packet */
    pRspPacket->pPacket = NULL;
    pRspPacket->pHeader->length = 0;
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = PI_GOOD;

    return PI_GOOD;
}


/**
******************************************************************************
**
**  @brief  Acknowledge logs
**
**  @param  pReqPacket - pointer to the request packet
**  @param  pRspPacket - pointer to the response packet
**
**  @return PI_GOOD or PI_ERROR
**
******************************************************************************
**/
INT32 PI_LogAcknowledge(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    PI_CUSTOMER_LOG_ACKNOWLEDGE_REQ *req;
    UINT32      i;

    req = (PI_CUSTOMER_LOG_ACKNOWLEDGE_REQ *)pReqPacket->pPacket;

    for (i = 0; i < req->logCount; i++)
    {
        LOG_HDR *pLog = LogAcknowledge(req->seqNos[i]);

        if (!pLog)
        {
            dprintf(DPRINTF_DEFAULT, "%s: LOG %d NOT FOUND\n",
                        __func__, req->seqNos[i]);
            continue;
        }

        /*
         * Send the changed log event
         * Create Binary and Text Events and send to all registered clients
         */
        PI_BINARY_LOG_EVENT *pbe;

        pbe = MallocWC(sizeof(*pbe) + sizeof(*pLog) + pLog->length);
        pbe->length = sizeof(*pLog) + pLog->length + sizeof(pbe->eventType);
        pbe->eventType = GetEventType(pLog->eventCode);
        memcpy(pbe->message, (char *)pLog, sizeof(*pLog) + pLog->length);
        EnqueuePIAsyncNotification(pbe);
        Free(pLog);
    }

    /* Set the length to 0 and the pointer to NULL. */

    pRspPacket->pHeader->length = 0;
    pRspPacket->pPacket = NULL;
    pRspPacket->pHeader->errorCode = 0;
    pRspPacket->pHeader->status = PI_GOOD;

    return PI_GOOD;
}


/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
