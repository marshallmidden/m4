/* $Id: PI_RegisterClients.c 143020 2010-06-22 18:35:56Z m4 $*/
/*===========================================================================
** FILE NAME:       PI_RegisterClients.c
** MODULE TITLE:    Managing Clients for Async Events
**
** DESCRIPTION:     Allows persistent data from ewoks to be saved and
**                  retrieved from the CCB
**
** Copyright (c) 2002-2009 Xiotech Corporation. All rights reserved.
**==========================================================================*/

#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include "PacketInterface.h"
#include "X1_AsyncEventHandler.h"
#include "PortServerUtils.h"

#include "debug_files.h"
#include "errorCodes.h"
#include "ipc_common.h"
#include "logview.h"
#include "PI_Clients.h"
#include "quorum_utils.h"
#include "slink.h"
#include "XIOPacket.h"
#include "XIO_Std.h"
#include "XIO_Const.h"
#include "XIO_Types.h"
#include "XIO_Macros.h"
#include "PI_CmdHandlers.h"

/*****************************************************************************
** Private variables
*****************************************************************************/
static PCB *pPIAsyncTablePcb = NULL;
static S_LIST *pPIAsyncEventsQueue = NULL;

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static void SendLogEventToRegisteredClients(PI_BINARY_LOG_EVENT *pBinEvent);
static void ProcessPIAsyncNotificationsTask(TASK_PARMS *parms);

/*****************************************************************************
** Public functions - not externed in a header file
*****************************************************************************/
NORETURN extern void AsyncEventPingTask(UNUSED TASK_PARMS *pparms);
extern void SendChangedEventToRegisteredClients(XIO_PACKET *asyncPkt);

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
** Function:    PI_RegisterEvents
**
** Description: PI handler for event registration
**
** Inputs:      request, response packets
**
** Returns:     PI_GOOD, errorcode otherwise
**
**--------------------------------------------------------------------------*/
INT32 PI_RegisterEvents(XIO_PACKET *pReqPacket, XIO_PACKET *pRspPacket)
{
    INT32       rc = PI_GOOD;
    INT32       error = 0;
    PI_REGISTER_EVENTS_REQ *pDataReqP;
    PI_REGISTER_EVENTS_RSP *pDataRspP;

    pDataReqP = (PI_REGISTER_EVENTS_REQ *)(pReqPacket->pPacket);

    /*
     * Allocate the memory for the response.
     */
    pDataRspP = MallocWC(sizeof(*pDataRspP));
    pRspPacket->pPacket = (UINT8 *)pDataRspP;
    pRspPacket->pHeader->length = sizeof(*pDataRspP);

    if (PI_REGISTER_EVENTS_SET == pDataReqP->opt)
    {
        /*
         * register events for this client
         */
        pi_clients_register_events_set(pReqPacket->pHeader->socket,
                                       pDataReqP->registerEvents);
    }

    /*
     * Get registration bitmap into the response
     */
    pDataRspP->eventsRegisteredOn = pi_clients_register_events_get(pReqPacket->pHeader->socket);

    pRspPacket->pPacket = (UINT8 *)pDataRspP;
    pRspPacket->pHeader->length = sizeof(*pDataRspP);
    pRspPacket->pHeader->status = rc;
    pRspPacket->pHeader->errorCode = error;

    return (rc);
}


/*----------------------------------------------------------------------------
** Function:    SendChangedEventToRegisteredClients
**
** Description: Send a changed event packet to all
**              registered clients that registered for the event.
**
** Inputs:      async packet.
**
** Returns:     none
**
**--------------------------------------------------------------------------*/
void SendChangedEventToRegisteredClients(XIO_PACKET *asyncPkt)
{
    S_LIST     *plist;
    pi_client_info *curr;
    ASYNC_CHANGED_EVENT *pChgEvent;
    UINT64      eventType;
    INT32       rc;
    UINT32      granularEvents;
    UINT32      otherRegEvents;

    if (asyncPkt->pHeader->commandCode == PI_ASYNC_CHANGED_EVENT)
    {
        /*
         * Changed event
         */
        pChgEvent = (ASYNC_CHANGED_EVENT *)(asyncPkt->pPacket);
        eventType = pChgEvent->eventType;

//        fprintf (stderr, "KM %s: eventType = %llx\n",__FUNCTION__, eventType);
        /*
         * Look up for all clients that registered for changed events
         */
        plist = pi_clients_get_list();

        if (plist != NULL)
        {
            /*
             * Setup the iterator for the client list.
             */
            SetIterator(plist);

            while (NULL != (curr = Iterate(plist)))
            {
                /*
                 * Several conditions keep us from sending an event to a
                 * connection:
                 *  - the connection is in an error state (berror).
                 *  - The socket for the client is closed (sockfd).
                 *  - The client is not registered for any event.
                 */
                if (curr->berror == TRUE ||
                    curr->sockfd == SOCKET_ERROR ||
                    curr->regEvents == 0)
                {
                    continue;
                }

                /*
                 * Look up for each changed event the client registered for
                 * and mask off the remaining events
                 */
                pChgEvent->eventType = 0;
                otherRegEvents = curr->regEvents >> 32;
                granularEvents = eventType >> 32;

                if ((curr->regEvents & X1_ASYNC_PCHANGED) &&
                    (eventType & X1_ASYNC_PCHANGED))
                {
                    pChgEvent->eventType |= X1_ASYNC_PCHANGED;
                }

                if ((curr->regEvents & X1_ASYNC_RCHANGED) &&
                    (eventType & X1_ASYNC_RCHANGED))
                {
                    pChgEvent->eventType |= X1_ASYNC_RCHANGED;
                }

                if ((curr->regEvents & X1_ASYNC_VCHANGED) &&
                    (eventType & X1_ASYNC_VCHANGED))
                {
                    pChgEvent->eventType |= X1_ASYNC_VCHANGED;
                }

                if ((curr->regEvents & X1_ASYNC_HCHANGED) &&
                    (eventType & X1_ASYNC_HCHANGED))
                {
                    pChgEvent->eventType |= X1_ASYNC_HCHANGED;
                }

                if ((curr->regEvents & X1_ASYNC_ACHANGED) &&
                    (eventType & X1_ASYNC_ACHANGED))
                {
                    pChgEvent->eventType |= X1_ASYNC_ACHANGED;
                }

                if ((curr->regEvents & SNAPPOOL_CHANGED) &&
                    (eventType & SNAPPOOL_CHANGED))
                {
                    pChgEvent->eventType |= SNAPPOOL_CHANGED;
                }

                if ((curr->regEvents & X1_ASYNC_ZCHANGED) &&
                    (eventType & X1_ASYNC_ZCHANGED))
                {
                    pChgEvent->eventType |= X1_ASYNC_ZCHANGED;
                }

                if ((curr->regEvents & X1_ASYNC_VCG_ELECTION_STATE_CHANGE) &&
                    (eventType & X1_ASYNC_VCG_ELECTION_STATE_CHANGE))
                {
                    pChgEvent->eventType |= X1_ASYNC_VCG_ELECTION_STATE_CHANGE;
                }

                if ((curr->regEvents & X1_ASYNC_VCG_ELECTION_STATE_ENDED) &&
                    (eventType & X1_ASYNC_VCG_ELECTION_STATE_ENDED))
                {
                    pChgEvent->eventType |= X1_ASYNC_VCG_ELECTION_STATE_ENDED;
                }

                if ((curr->regEvents & X1_ASYNC_VCG_POWERUP) &&
                    (eventType & X1_ASYNC_VCG_POWERUP))
                {
                    pChgEvent->eventType |= X1_ASYNC_VCG_POWERUP;
                }

                if ((curr->regEvents & X1_ASYNC_VCG_CFG_CHANGED) &&
                    (eventType & X1_ASYNC_VCG_CFG_CHANGED))
                {
                    pChgEvent->eventType |= X1_ASYNC_VCG_CFG_CHANGED;
                }

                if ((curr->regEvents & X1_ASYNC_VCG_WORKSET_CHANGED) &&
                    (eventType & X1_ASYNC_VCG_WORKSET_CHANGED))
                {
                    pChgEvent->eventType |= X1_ASYNC_VCG_WORKSET_CHANGED;
                }

                if ((curr->regEvents & X1_ASYNC_BE_PORT_CHANGE) &&
                    (eventType & X1_ASYNC_BE_PORT_CHANGE))
                {
                    pChgEvent->eventType |= X1_ASYNC_BE_PORT_CHANGE;
                }

                if ((curr->regEvents & X1_ASYNC_FE_PORT_CHANGE) &&
                    (eventType & X1_ASYNC_FE_PORT_CHANGE))
                {
                    pChgEvent->eventType |= X1_ASYNC_FE_PORT_CHANGE;
                }

                if ((curr->regEvents & ASYNC_PDATA_CREATE) &&
                    (eventType & ASYNC_PDATA_CREATE))
                {
                    pChgEvent->eventType |= ASYNC_PDATA_CREATE;
                }

                if ((curr->regEvents & ASYNC_PDATA_REMOVE) &&
                    (eventType & ASYNC_PDATA_REMOVE))
                {
                    pChgEvent->eventType |= ASYNC_PDATA_REMOVE;
                }

                if ((curr->regEvents & ASYNC_PDATA_MODIFY) &&
                    (eventType & ASYNC_PDATA_MODIFY))
                {
                    pChgEvent->eventType |= ASYNC_PDATA_MODIFY;
                }

                if ((curr->regEvents & ASYNC_ISNS_MODIFY) &&
                    (eventType & ASYNC_ISNS_MODIFY))
                {
                    pChgEvent->eventType |= ASYNC_ISNS_MODIFY;
                }

                if ((curr->regEvents & ASYNC_BUFFER_BOARD_CHANGE) &&
                    (eventType & ASYNC_BUFFER_BOARD_CHANGE))
                {
                    pChgEvent->eventType |= ASYNC_BUFFER_BOARD_CHANGE;
                }

                if ((curr->regEvents & ASYNC_GLOBAL_CACHE_CHANGE) &&
                    (eventType & ASYNC_GLOBAL_CACHE_CHANGE))
                {
                    pChgEvent->eventType |= ASYNC_GLOBAL_CACHE_CHANGE;
                }

                if ((curr->regEvents & ASYNC_DEFRAG_CHANGE) &&
                    (eventType & ASYNC_DEFRAG_CHANGE))
                {
                    pChgEvent->eventType |= ASYNC_DEFRAG_CHANGE;
                }

#ifdef ENABLE_NG_HWMON
                if ((curr->regEvents & ASYNC_ENV_CHANGE) &&
                    (eventType & ASYNC_ENV_CHANGE))
                {
                    pChgEvent->eventType |= ASYNC_ENV_CHANGE;
                }
#endif  /* ENABLE_NG_HWMON */

                if ((curr->regEvents & ASYNC_PRES_CHANGED) &&
                    (eventType & ASYNC_PRES_CHANGED))
                {
                    pChgEvent->eventType |= ASYNC_PRES_CHANGED;
                }

                if ((curr->regEvents & ASYNC_APOOL_CHANGED) &&
                    (eventType & ASYNC_APOOL_CHANGED))
                {
                    pChgEvent->eventType |= ASYNC_APOOL_CHANGED;
                }

                if ((curr->regEvents & ISE_ENV_CHANGED) && (eventType & ISE_ENV_CHANGED))
                {
                    pChgEvent->eventType |= ISE_ENV_CHANGED;
                }
                /*
                 * Granular events higher 32 bits
                 */
                if ((otherRegEvents & PI_EVENT_ISE_CTRLR_CHANGED) &&
                    (granularEvents & PI_EVENT_ISE_CTRLR_CHANGED))
                {
                    pChgEvent->extendEvnt.eventType2 |= PI_EVENT_ISE_CTRLR_CHANGED;
                }
                if ((otherRegEvents & PI_EVENT_ISE_DPAC_CHANGED) &&
                    (granularEvents & PI_EVENT_ISE_DPAC_CHANGED))
                {
                    pChgEvent->extendEvnt.eventType2 |= PI_EVENT_ISE_DPAC_CHANGED;
                }
                if ((otherRegEvents & PI_EVENT_ISE_PS_CHANGED) &&
                    (granularEvents & PI_EVENT_ISE_PS_CHANGED))
                {
                    pChgEvent->extendEvnt.eventType2 |= PI_EVENT_ISE_PS_CHANGED;
                }
                if ((otherRegEvents & PI_EVENT_ISE_BATTERY_CHANGED) &&
                    (granularEvents & PI_EVENT_ISE_BATTERY_CHANGED))
                {
                    pChgEvent->extendEvnt.eventType2 |= PI_EVENT_ISE_BATTERY_CHANGED;
                }
                if ((otherRegEvents & PI_EVENT_ISE_CHASSIS_CHANGED) &&
                    (granularEvents & PI_EVENT_ISE_CHASSIS_CHANGED))
                {
                    pChgEvent->extendEvnt.eventType2 |= PI_EVENT_ISE_CHASSIS_CHANGED;
                }

                /*
                 * If there are any events that the client registered for,
                 * send the packet
                 */
                if (pChgEvent->eventType != X1_ASYNC_NONE)
                {
//                   fprintf (stderr, "KM %s: event type after if ladder = %llx\n", __FUNCTION__, pChgEvent->eventType);
                    rc = SendPacket(curr->sockfd, asyncPkt, TMO_PI_SEND);

                    if (rc == SOCKET_ERROR)
                    {
                        /*
                         * Print a debug message to the serial console.
                         */
                        dprintf(DPRINTF_DEFAULT, "SendChangedEventToRegisteredClients: Error sending changed event on socket %hu.\n",
                                curr->sockfd);

                        /*
                         * Set the client into an error state to prevent additional
                         * processing.
                         */
                        pi_clients_error(curr);
                    }
                }
            }
            pi_clients_release_list();
        }
    }
}

/*----------------------------------------------------------------------------
** Function:    SendLogEventToRegisteredClients
**
** Description: Send a log event packet to all
**              registered clients that registered for the event.
**
** Inputs:      log packet, binary and ascii events.
**
** Returns:     none
**
**--------------------------------------------------------------------------*/
static void SendLogEventToRegisteredClients(PI_BINARY_LOG_EVENT *pBinEvent)
{
    S_LIST     *plist;
    pi_client_info *curr;
    XIO_PACKET *logPkt;
    LOG_HDR    *pHdr;
    PI_LOG_INFO_MODE0_RSP *pEvent;
    PI_LOG_INFO_MODE1_RSP *pEvent1;
    PI_ASCII_LOG_EVENT *pTEvent;
    PI_ASCII_LOG_EVENT *pTextEvent;
    PI_BINARY_LOG_EVENT *pBEvent;
    INT32       offset;
    INT32       rc;

    pTextEvent = MallocWC(sizeof(*pTextEvent) + MAX_LOG_MESSAGE_SIZE);
    pHdr = (LOG_HDR *)(pBinEvent->message);
    pTextEvent->eventCode = pHdr->eventCode;
    pTextEvent->masterSequenceNumber = pHdr->masterSequence;
    pTextEvent->sequenceNumber = pHdr->sequence;
    pTextEvent->statusWord = pHdr->le.statusWord;

    GetTimeString(&(pHdr->timeStamp), pTextEvent->timeAndDate);
    GetEventTypeString(pTextEvent->eventCode, pTextEvent->messageType);
    GetMessageString(pHdr, pTextEvent->messageDescr, 20);

    /*
     * Copy length of standard message
     */
    pTextEvent->length = strlen(pTextEvent->messageDescr);
    ExtendedMessage(pTextEvent->messageDescr, pHdr, ASCII_TYPE, INDENT);

    /*
     * Create the packet and fill in the data
     */
    logPkt = MallocWC(sizeof(*logPkt));
    logPkt->pHeader = MallocWC(sizeof(*logPkt->pHeader));
    logPkt->pHeader->packetVersion = 1;
    logPkt->pHeader->headerLength = sizeof(*logPkt->pHeader);

    logPkt->pPacket = MallocWC(sizeof(*pEvent) +
                                        sizeof(PI_LOG_EVENT) + pBinEvent->length +
                                        strlen(pTextEvent->messageDescr) + 1);

    logPkt->pHeader->commandCode = PI_LOG_EVENT_MESSAGE;

    pEvent = (PI_LOG_INFO_MODE0_RSP *)(logPkt->pPacket);
    pEvent->eventCount = 1;
    pEvent->sequenceNumber = pHdr->sequence;
    pEvent->controllerSN = GetMyControllerSN();
    pEvent->vcgID = Qm_GetVirtualControllerSN();

    /*
     * Log message packet
     */
    if (logPkt->pHeader->commandCode == PI_LOG_EVENT_MESSAGE)
    {
        pEvent = (PI_LOG_INFO_MODE0_RSP *)(logPkt->pPacket);
        pEvent1 = (PI_LOG_INFO_MODE1_RSP *)(logPkt->pPacket);

        /*
         * Look up for all clients that registered for the events
         */
        plist = pi_clients_get_list();

        if (plist != NULL)
        {
            /*
             * Setup the iterator for the client list.
             */
            SetIterator(plist);

            while (NULL != (curr = Iterate(plist)))
            {
                /*
                 * Several conditions keep us from sending an event to a
                 * connection:
                 *  - the connection is in an error state (berror).
                 *  - The socket for the client is closed (sockfd).
                 *  - The client is not registered for any event.
                 */
                if (curr->berror == TRUE ||
                    curr->sockfd == SOCKET_ERROR || curr->regEvents == 0)
                {
                    continue;
                }

                offset = 0;
                logPkt->pHeader->length = sizeof(*pEvent);
                pEvent->mode = 0;

                /*
                 * If the client registered for ascii message
                 */
                if ((curr->regEvents & LOG_XTD_MSG) || (curr->regEvents & LOG_STD_MSG))
                {
                    /*
                     * Fill in the ascii event header
                     */
                    pTEvent = &(pEvent->logEvent[0].ascii);

                    pTEvent->eventCode = pTextEvent->eventCode;
                    pTEvent->masterSequenceNumber = pTextEvent->masterSequenceNumber;
                    pTEvent->sequenceNumber = pTextEvent->sequenceNumber;
                    pTEvent->statusWord = pTextEvent->statusWord;
                    strcpy(pTEvent->timeAndDate, pTextEvent->timeAndDate);
                    strcpy(pTEvent->messageType, pTextEvent->messageType);
                    pTEvent->length = 0;

                    /*
                     * Registration is for extended message
                     */
                    if (curr->regEvents & LOG_XTD_MSG)
                    {
                        strcpy(pTEvent->messageDescr, pTextEvent->messageDescr);
                        pTEvent->length = strlen(pTextEvent->messageDescr) + 1;
                        offset = sizeof(*pTEvent) + pTEvent->length;
                        pEvent->mode |= MODE_EXTENDED_MESSAGE;
                        logPkt->pHeader->length += offset + sizeof(pEvent->logEvent[0].length);
                        pTEvent->length += (sizeof(*pTEvent) - sizeof(pTEvent->length));
                    }
                    else        /* std msg only */
                    {
                        /*
                         * Registration is for standard message only
                         */
                        memcpy(pTEvent->messageDescr, pTextEvent->messageDescr,
                               pTextEvent->length);
                        pTEvent->messageDescr[pTextEvent->length] = '\0';
                        pTEvent->length = pTextEvent->length + 1;
                        offset = sizeof(*pTEvent) + pTEvent->length;
                        logPkt->pHeader->length += offset + sizeof(pEvent->logEvent[0].length);
                        pTEvent->length += (sizeof(*pTEvent) - sizeof(pTEvent->length));
                    }
                    pEvent->logEvent[0].length = pTEvent->length + sizeof(pTEvent->length);
                }
                else
                {
                    pTEvent = NULL;
                }

                /*
                 * If the client registered for binary event
                 */
                if (curr->regEvents & LOG_BIN_MSG)
                {
                    /*
                     * Fill in the binary event header
                     */
                    if (pTEvent)
                    {
                        pBEvent = (PI_BINARY_LOG_EVENT *)(((UINT8 *)pTEvent) + offset);
                        logPkt->pHeader->length += sizeof(pBEvent->length);
                        /* pEvent->mode |= MODE_BINARY_MESSAGE; */
                    }
                    else
                    {
                        pBEvent = &(pEvent1->logEvent[0]);
                        pEvent1->mode |= MODE_BINARY_MESSAGE;
                        logPkt->pHeader->length = sizeof(*pEvent1) + sizeof(*pBEvent);
                    }

                    pBEvent->length = pBinEvent->length;
                    pBEvent->eventType = pBinEvent->eventType;
                    memcpy(pBEvent->message,
                           pBinEvent->message,
                           (pBinEvent->length) - sizeof(pBEvent->eventType));

                    logPkt->pHeader->length += pBEvent->length;

                    if (pTEvent)
                    {
                        pEvent->logEvent[0].length += pBEvent->length + sizeof(pBEvent->length);
                    }
                }

                /*
                 * Send the packet
                 */
                if ((curr->regEvents & LOG_BIN_MSG) ||
                    (curr->regEvents & LOG_STD_MSG) ||
                    (curr->regEvents & LOG_XTD_MSG))
                {
                    rc = SendPacket(curr->sockfd, logPkt, TMO_PI_SEND);

                    if (rc == SOCKET_ERROR)
                    {
                        /*
                         * Print a debug message to the serial console.
                         */
                        dprintf(DPRINTF_DEFAULT, "SendLogEventToRegisteredClients: Error sending log event on socket %hu.\n",
                                curr->sockfd);

                        /*
                         * Set the client into an error state to prevent additional
                         * processing.
                         */
                        pi_clients_error(curr);
                    }
                }
            }
            pi_clients_release_list();
        }
    }

    /*
     * Event sent to all clients, free up the memory
     */
    if (logPkt->pHeader)
    {
        Free(logPkt->pHeader);
    }

    if (logPkt->pPacket)
    {
        Free(logPkt->pPacket);
    }

    if (logPkt)
    {
        Free(logPkt);
    }

    Free(pBinEvent);
    Free(pTextEvent);
}

/*----------------------------------------------------------------------------
** Function:    EnqueuePIAsyncNotification
**
** Description: Enque an event to the client notification task
**
** Inputs:      log event
**
** Returns:     none
**
**--------------------------------------------------------------------------*/
void EnqueuePIAsyncNotification(PI_BINARY_LOG_EVENT *pBinEvent)
{
    /*
     * If the Broadcast event list has not yet been created, create it.
     */
    if (pPIAsyncEventsQueue == NULL)
    {
        pPIAsyncEventsQueue = CreateList();
    }

    /*
     * Create the PI async notification task if not already done.
     */
    if (pPIAsyncTablePcb == NULL)
    {
        pPIAsyncTablePcb = TaskCreate(ProcessPIAsyncNotificationsTask, NULL);
    }

    /*
     * Enqueue this event on the event list so it will be
     * processed by the task.
     */
    Enqueue(pPIAsyncEventsQueue, pBinEvent);

    /*
     * if the PI Async task exists and is not ready, wake it up.
     */
    if (pPIAsyncTablePcb != NULL && TaskGetState(pPIAsyncTablePcb) == PCB_NOT_READY)
    {
        TaskSetState(pPIAsyncTablePcb, PCB_READY);
    }
}

/*----------------------------------------------------------------------------
** Function:    ProcessX1AsyncTable
**
** Description: Forked task to process X1 async Notifications.
**
** Inputs:      Dummy   -   Makes it forkable
**
** Returns:     none
**
**--------------------------------------------------------------------------*/
static NORETURN void ProcessPIAsyncNotificationsTask(UNUSED TASK_PARMS *parms)
{
    PI_BINARY_LOG_EVENT *pBinEvent;

    while (1)
    {
        /*
         * While there are servers to process continue looping.
         */
        while (NumberOfItems(pPIAsyncEventsQueue) > 0)
        {
            /*
             * Get the next event off the queue.
             */
            pBinEvent = (PI_BINARY_LOG_EVENT *)Dequeue(pPIAsyncEventsQueue);

            /*
             * Send the log event to the clients.  This will free the binary
             * event object.
             */
            SendLogEventToRegisteredClients(pBinEvent);

            /*
             * Go back to the start of the WHILE loop that checks if there are
             * more items in the list to be processed.
             */
        }

        TaskSetState(pPIAsyncTablePcb, PCB_NOT_READY);
        TaskSwitch();
    }
}


/**
******************************************************************************
**
**  @brief      Task that sends the asynchronous ping event to registered
**              clients at a predetermined time interval.
**
**  @return     none
**
******************************************************************************
**/
NORETURN void AsyncEventPingTask(UNUSED TASK_PARMS *pparms)
{
    XIO_PACKET  packet;
    S_LIST     *plist;
    pi_client_info *curr;
    INT32       rc;

    packet.pHeader = MallocWC(sizeof(*packet.pHeader));
    packet.pHeader->packetVersion = 1;
    packet.pHeader->commandCode = PI_ASYNC_PING_EVENT;
    packet.pHeader->headerLength = sizeof(*packet.pHeader);
    packet.pHeader->length = 0;
    packet.pPacket = NULL;

    while (1)
    {
        TaskSleepMS(20000);

        /*
         * Look up for all clients that registered for changed events
         */
        plist = pi_clients_get_list();

        if (plist != NULL)
        {
            /*
             * Setup the iterator for the client list.
             */
            SetIterator(plist);

            while (NULL != (curr = Iterate(plist)))
            {
                /*
                 * Several conditions keep us from sending an event to a
                 * connection:
                 *  - the connection is in an error state (berror).
                 *  - The socket for the client is closed (sockfd).
                 *  - The client is not registered for this event.
                 */
                if (curr->berror == TRUE ||
                    curr->sockfd == SOCKET_ERROR ||
                    (curr->regEvents & ASYNC_PING_EVENT) == 0)
                {
                    continue;
                }

                rc = SendPacket(curr->sockfd, &packet, TMO_PI_SEND);

                if (rc == SOCKET_ERROR)
                {
                    /*
                     * Print a debug message to the serial console.
                     */
                    dprintf(DPRINTF_DEFAULT, "AsyncEventPingTask: Error sending changed event on socket %hu.\n",
                            curr->sockfd);

                    /*
                     * Set the client into an error state to prevent additional
                     * processing.
                     */
                    pi_clients_error(curr);
                }
            }

            pi_clients_release_list();
        }
    }

//   Free(packet.pHeader);
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
