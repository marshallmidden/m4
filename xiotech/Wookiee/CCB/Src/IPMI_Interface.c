/* $Id: IPMI_Interface.c 159129 2012-05-12 06:25:16Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       IPMI_Interface.c
**
**  @brief      Interface to IPMI
**
**  Copyright (c) 2002-2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/

#include "IPMI_Defines.h"

#include "debug_files.h"
#include "LOG_Defs.h"
#include "logdef.h"
#include "XIO_Std.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/time.h>

/*
******************************************************************************
** Private defines - constants
******************************************************************************
*/
#define IPMI_IO_DEVICE_NAME     "/dev/ipmi0"
#define IPMI_SMI_ADDRESS        { 0x0C, 0x0F, 0, 0 }
#define IPMI_IOCTL_MAGIC        'i'

/*
******************************************************************************
** Private defines - data structures
******************************************************************************
*/
struct IPMI_INTERFACE_PRIVATE
{
    INT32       fileDescriptor;
};

typedef struct
{
    UINT32      addressType;
    UINT16      channel;
    UINT8       lun;
    UINT8       reserved;
} IPMI_INTERFACE_ADDRESS;

typedef struct
{
    UINT8      *pAddress;
    UINT32      addressLength;
    UINT32      messageID;
    IPMI_MESSAGE message;
} IPMI_REQUEST;

typedef struct
{
    INT32       receiveType;
#define IPMI_RECEIVE_TYPE_RESPONSE      1
#define IPMI_RECEIVE_TYPE_ASYNC_EVENT   2
#define IPMI_RECEIVE_TYPE_COMMAND       3
    UINT8      *pAddress;
    UINT32      addressLength;
    UINT32      messageID;
    IPMI_MESSAGE message;
} IPMI_RECEIVE;

/*
******************************************************************************
** Private variables
******************************************************************************
*/
static COMMAND_HEADER *gpCurrentCommandHeader = NULL;

/*
******************************************************************************
** Private function prototypes
******************************************************************************
*/
static void Interface_SelectTask(TASK_PARMS *pTaskParms);
static UINT32 InterfaceGenerateAlert(IPMI_EVENT *pEvent);
static void dump_hex(UINT8 *pData, UINT32 length, UINT8 verboseFlag);

/*
******************************************************************************
** Code Start
******************************************************************************
*/

/**
******************************************************************************
**
**  @brief      Initializes and starts a new IPMI connection
**
**  @param      pNewInterface - pointer to an IPMI interface structure
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
UINT32 IPMI_InterfaceCreate(IPMI_INTERFACE **pInterface)
{
    int         save_errno;
    TASK_PARMS  taskParms;

    if (!pInterface)
    {
        dprintf(DPRINTF_IPMI, "Input pointer is NULL\n");
        return ERROR;
    }

    /* Allocate space for a new IPMI interface structure */

    *pInterface = MallocWC(sizeof(**pInterface));

    /* Open the IPMI connection */

    (*pInterface)->fileDescriptor = open(IPMI_IO_DEVICE_NAME, O_RDWR);
    save_errno = errno;
    dprintf(DPRINTF_IPMI, "Open IPMI interface: %d\n", (*pInterface)->fileDescriptor);

    /* Check for valid open */

    if ((*pInterface)->fileDescriptor < 0)
    {
        char        aBuffer[80];
        IPMI_EVENT  event = { EVENT_STRING_SEVERITY_NONE, sizeof(aBuffer), aBuffer };
        /* Log and take action - IPMI isn't functional */

#if defined(MODEL_7000) || defined(MODEL_4700)
        event.eventSeverity = EVENT_STRING_SEVERITY_INFO;
#else  /* MODEL_7000 || MODEL_4700 */
        event.eventSeverity = EVENT_STRING_SEVERITY_ERROR;
#endif /* MODEL_7000 || MODEL_4700 */
        snprintf(event.pBuffer, event.bufferSize, "IPMI Failure (%d)-%s",
                 save_errno, strerror(save_errno));
        aBuffer[79] = 0;
        InterfaceGenerateAlert(&event);

        return ERROR;
    }

    memset(&taskParms, 0, sizeof(taskParms));

    taskParms.p1 = (*pInterface)->fileDescriptor;
    TaskCreate(Interface_SelectTask, &taskParms);

    dprintf(DPRINTF_IPMI, "IPMI interface opened: %d\n", (*pInterface)->fileDescriptor);

    return GOOD;
}


/**
******************************************************************************
**
**  @brief      Send a command to the BMC, and waits for the response
**
**  @param      pHeader - pointer to command header
**
**  @return     GOOD or ERROR
**
******************************************************************************
**/
UINT32 IPMI_InterfaceSend(IPMI_INTERFACE *pInterface, COMMAND_HEADER *pHeader)
{
    /* Unique ID for each message */

    static UINT32 sequenceCounter = 0;
    IPMI_INTERFACE_ADDRESS smiAddress = IPMI_SMI_ADDRESS;
    IPMI_REQUEST request;       /* Uninitialized */
    INT32       ioRC = 0;

    if (!pInterface || !pHeader)
    {
        dprintf(DPRINTF_IPMI, "Input pointer is NULL\n");
        return ERROR;
    }

    /*
     * Add our claim to the send count
     * TODO: Allow multiple outstanding commands
     */
    while (gpCurrentCommandHeader)
    {
        TaskSleepMS(20);
    }

    /* Remember which command is in progress */

    gpCurrentCommandHeader = pHeader;

    /* Assemble the command into well formed request */

    memset(&request, 0, sizeof(request));
    request.pAddress = (UINT8 *)&smiAddress;
    request.addressLength = sizeof(smiAddress);
    request.messageID = sequenceCounter;
    request.message.netFn = pHeader->message.netFn;
    request.message.command = pHeader->message.command;
    request.message.pData = pHeader->message.pData;
    request.message.dataLength = pHeader->message.dataLength;

    /* Bump the sequence counter - unique ID for each message */

    sequenceCounter++;

    dprintf(DPRINTF_IPMI_IO, "*** IPMI Transmit *****************************************************************\n");
    dprintf(DPRINTF_IPMI_IO, "  fileDescriptor:             %d\n",
            pInterface->fileDescriptor);
    dprintf(DPRINTF_IPMI_IO, "  request.pAddress:           %p\n", request.pAddress);
    dprintf(DPRINTF_IPMI_IO, "  request.addressLength:      %d\n", request.addressLength);
    dprintf(DPRINTF_IPMI_IO, "  request.messageID:          %d\n", request.messageID);
    dprintf(DPRINTF_IPMI_IO, "  request.message.netFn:      %d\n", request.message.netFn);
    dprintf(DPRINTF_IPMI_IO, "  request.message.command:    %d\n",
            request.message.command);
    dprintf(DPRINTF_IPMI_IO, "  request.message.pData:      %p\n", request.message.pData);
    dprintf(DPRINTF_IPMI_IO, "  request.message.dataLength: %d\n",
            request.message.dataLength);

    dprintf(DPRINTF_IPMI_IO, "Request\n");
    dump_hex((void *)request.message.pData, request.message.dataLength, 1);

    /* Send the command */

    ioRC = ioctl(pInterface->fileDescriptor, _IOR(IPMI_IOCTL_MAGIC, 13,
                                                  IPMI_REQUEST), &request);
    if (ioRC < 0)
    {
        dprintf(DPRINTF_IPMI_IO, "Error sending command: %d [%d - %s]\n",
                ioRC, errno, strerror(errno));

        /* Flag the send error */

        pHeader->flags.bits.sendCommandError = TRUE;

        /* Flag the command as complete */

        pHeader->flags.bits.commandComplete = TRUE;
#ifdef PAM
        dprintf(DPRINTF_DEFAULT, "ioctl(%d,, %p) returned %d\n",
                pInterface->fileDescriptor, &request, ioRC);
#endif /* PAM */

        /* If error occurred, unset the outstanding request (locking flag).  */

        gpCurrentCommandHeader = NULL;
        return ERROR;
    }

    dprintf(DPRINTF_IPMI_IO, "  Message sent (%d)\n", request.messageID);
    return GOOD;
}


/**
******************************************************************************
**
**  @brief      This is a task that performs the select on the connection
**
**  @param      pTaskParms (p1) - file descriptor value
**
**  @return     none
**
******************************************************************************
**/
static void Interface_SelectTask(TASK_PARMS *pTaskParms)
{
    char        aBuffer[80] = { 0 };
    IPMI_EVENT  event = { EVENT_STRING_SEVERITY_NONE, sizeof(aBuffer), aBuffer };

    dprintf(DPRINTF_IPMI, "** Select Task **\n");

    if (pTaskParms != NULL)
    {
        INT32       ioRC = 0;
        int         dummy = 1;  /* Must match OS 'int' type */
        INT32       fileDescriptor = pTaskParms->p1;

        ioRC = ioctl(fileDescriptor, _IOR(IPMI_IOCTL_MAGIC, 16, INT32), &dummy);

        if (ioRC == 0)
        {
            /*
             * Sit and spin
             */
            while (1)
            {
                INT32       selectRC = 0;
                IPMI_INTERFACE_ADDRESS smiAddress = IPMI_SMI_ADDRESS;
                IPMI_RECEIVE response;  /* Uninitialized */
                UINT8       aData[IPMI_MAX_MESSAGE_SIZE];       /* Uninitialized */
                fd_set      readSet;    /* Uninitialized */

                FD_ZERO(&readSet);
                FD_SET(fileDescriptor, &readSet);

                /*
                 * Wait for the command to complete
                 */
                selectRC = Select((fileDescriptor + 1), &readSet, NULL, NULL, NULL);

                dprintf(DPRINTF_IPMI_IO, "  SelectTask Select: [0x%0x]\n", selectRC);

                if (selectRC >= 0)
                {
                    /*
                     * Check for a timeout condition
                     */
                    if (FD_ISSET(fileDescriptor, &readSet) != 0)
                    {
                        response.pAddress = (UINT8 *)&smiAddress;
                        response.addressLength = sizeof(smiAddress);
                        response.message.pData = aData;
                        response.message.dataLength = sizeof(aData);

                        ioRC = ioctl(fileDescriptor, _IOWR(IPMI_IOCTL_MAGIC, 11, IPMI_RECEIVE), &response);

                        if (ioRC >= 0)
                        {
                            switch (response.receiveType)
                            {
                                case IPMI_RECEIVE_TYPE_RESPONSE:
                                    dprintf(DPRINTF_IPMI_IO, "  Command response\n");
                                    break;

                                case IPMI_RECEIVE_TYPE_ASYNC_EVENT:
                                    dprintf(DPRINTF_IPMI_IO, "  Async event\n");
                                    break;

                                case IPMI_RECEIVE_TYPE_COMMAND:
                                    dprintf(DPRINTF_IPMI_IO, "  Command receive\n");
                                    break;

                                default:
                                    dprintf(DPRINTF_IPMI_IO, "  Unknown receive type: %d\n",
                                            response.receiveType);
                                    break;
                            }

                            dprintf(DPRINTF_IPMI_IO, "Response\n");
                            dump_hex(response.message.pData, response.message.dataLength,
                                     1);
                        }
                        else
                        {
                            dprintf(DPRINTF_IPMI, "Error receiving response: %s\n",
                                    strerror(errno));

                            if (errno == EMSGSIZE)
                            {
                                response.message.pData[0] = IPMI_COMPLETION_CODE_REQUEST_DATA_LENGTH_EXCEEDED;
                                response.message.dataLength = sizeof(response.message.pData[0]);
                            }
                        }

                        /*
                         * Pass the data back to the outstanding command
                         */
                        if (gpCurrentCommandHeader != NULL)
                        {
                            memcpy(gpCurrentCommandHeader->pResponse,
                                   aData, response.message.dataLength);

                            gpCurrentCommandHeader->responseLength = response.message.dataLength;
                            gpCurrentCommandHeader->flags.bits.commandComplete = TRUE;
                            gpCurrentCommandHeader = NULL;
                        }
                        else
                        {
                            dprintf(DPRINTF_IPMI, "Select fired with gpCurrentCommandHeader NULL\n");
                        }
                    }
                    else
                    {
                        dprintf(DPRINTF_IPMI, "SelectTask Select timeout\n");

                        /*
                         * Log an event - IPMI isn't healthy
                         */
                        event.eventSeverity = EVENT_STRING_SEVERITY_DEBUG;
                        snprintf(event.pBuffer, event.bufferSize,
                                 "IPMI-Interface timeout");
                        InterfaceGenerateAlert(&event);
                    }
                }
            }
        }
        else
        {
            dprintf(DPRINTF_IPMI, "Error while registering for events: %d\n", ioRC);

            /*
             * Log an event - IPMI isn't healthy
             */
            event.eventSeverity = EVENT_STRING_SEVERITY_ERROR;
            snprintf(event.pBuffer, event.bufferSize, "IPMI-Interface events disabled");
            InterfaceGenerateAlert(&event);
        }
    }
    else
    {
        dprintf(DPRINTF_IPMI, "No parameters passed to SelectTask\n");
    }

    /*
     * Log an event - IPMI select task is ending
     */
    event.eventSeverity = EVENT_STRING_SEVERITY_ERROR;
    snprintf(event.pBuffer, event.bufferSize, "IPMI-Interface closed");
    InterfaceGenerateAlert(&event);

    dprintf(DPRINTF_IPMI, "** SelectTask - Returning **\n");
}


/**
******************************************************************************
**
**  @brief      Dump data in hex format, along with ASCII values
**
**  @param      pData       - pointer to the data
**  @param      length      - number of bytes to dump
**  @param      verboseFlag - flag for the dprintf type (0 = standard)
**
**  @return     none
**
******************************************************************************
**/
static void dump_hex(UINT8 *pData, UINT32 length, UINT8 verboseFlag)
{
    UINT32      bufferOffset = 0;
    UINT32      counter = 0;
    char        printChar = '0';
    char        buffer[80];

    for (counter = 0; counter < length; counter++)
    {
        if ((counter > 0) && ((counter % 8) == 0))
        {
            buffer[bufferOffset] = '\0';

            if (verboseFlag == 0)
            {
                dprintf(DPRINTF_IPMI, "%s\n", buffer);
            }
            else
            {
                dprintf(DPRINTF_IPMI_IO, "%s\n", buffer);
            }

            bufferOffset = 0;
        }

        if ((pData[counter] < ' ') || (pData[counter] > '~'))
        {
            printChar = ' ';
        }
        else
        {
            printChar = pData[counter];
        }

        bufferOffset += sprintf(buffer + bufferOffset, " %2.2x[%c]", pData[counter], printChar);
    }

    if (bufferOffset > 0)
    {
        buffer[bufferOffset] = '\0';

        if (verboseFlag == 0)
        {
            dprintf(DPRINTF_IPMI, "%s\n", buffer);
        }
        else
        {
            dprintf(DPRINTF_IPMI_IO, "%s\n", buffer);
        }
    }
}


/*----------------------------------------------------------------------------
**  Function Name: InterfaceGenerateAlert
**
**  Comments:   Generates and sends an alert message.
**
**  Parameters: pEvent      - Pointer to an IPMI event structure
**
**  Returns:    GOOD or ERROR
**
**  Note:       Any ERROR severity type log event generated by the interface
**              code will be treated as a fatal IPMI communication error.
**--------------------------------------------------------------------------*/
static UINT32 InterfaceGenerateAlert(IPMI_EVENT *pEvent)
{
    UINT32      eventCode = LOG_GetCode(LOG_IPMI_EVENT);
    LOG_IPMI_EVENT_PKT logEvent;        /* Uninitialized */

    if (!pEvent || !pEvent->pBuffer)
    {
        dprintf(DPRINTF_IPMI, "Input pointer is NULL\n");
        return ERROR;
    }

    switch (pEvent->eventSeverity)
    {
        case EVENT_STRING_SEVERITY_INFO:
            eventCode = LOG_Info(eventCode);
            break;

        case EVENT_STRING_SEVERITY_WARNING:
            eventCode = LOG_Warning(eventCode);
            break;

        case EVENT_STRING_SEVERITY_ERROR:
            eventCode = LOG_Error(eventCode);
            break;

        case EVENT_STRING_SEVERITY_DEBUG:
            eventCode = LOG_Debug(eventCode);
            break;

        case EVENT_STRING_SEVERITY_NONE:
        case EVENT_STRING_SEVERITY_NORMAL:
            return GOOD;

        default:
            return ERROR;
    }

    /* Initialize the log event structure */

    memset(&logEvent, 0, sizeof(logEvent));

    /* Fill in the log event data */

    logEvent.eventType = LOG_IPMI_EVENT_TYPE_INTERFACE;
    strncpy((char *)logEvent.text, pEvent->pBuffer, sizeof(logEvent.text));
    logEvent.text[sizeof(logEvent.text) - 1] = 0;

    /* Log the event */

    SendAsyncEvent(eventCode, sizeof(logEvent), &logEvent);
    return GOOD;
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
