/* $Id: SerConLogs.c 145021 2010-08-03 14:16:38Z m4 $ */
/*============================================================================
** FILE NAME:       SerConLogs.c
** MODULE TITLE:    Serial Console Log Frames
**
** DESCRIPTION:     Interface to display logs through the serial console
**
** Copyright (c) 2002-2009 Xiotech Corporation. All rights reserved.
**==========================================================================*/
#include "SerConLogs.h"

#include "convert.h"
#include "logging.h"
#include "logview.h"
#include "PacketInterface.h"
#include "PI_CmdHandlers.h"
#include "PktCmdHdl.h"
#include "SerBuff.h"
#include "SerCon.h"
#include "serial.h"
#include "XIO_Std.h"
#include "XIO_Const.h"

extern void GotoFirstFrame(void);

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static void SerialLogsSequenceFrameDisplayFunction(void);
static void SerialLogsSequenceFrameResponseFunction(void);
static void SerialLogsCountFrameDisplayFunction(void);
static void SerialLogsCountFrameResponseFunction(void);
static void SerialLogsModeFrameDisplayFunction(void);
static void SerialLogsModeFrameResponseFunction(void);
static void SerialLogsDisplayLogs(UINT32 sequence, UINT32 count, UINT16 mode);

/*****************************************************************************
** Private variables
*****************************************************************************/
static UINT32 logSequence = 0;
static UINT32 logCount = 0;
static UINT16 logMode = 0;
static UINT32 logExtended = FALSE;
static unsigned char logBadInput = FALSE;

static CONSOLEFRAME SerialLogsSequenceFrame = {
    SerialLogsSequenceFrameDisplayFunction,     /* Start function   */
    NULL,                       /* Finish function  */
    2,                          /* # of choices     */
    /* Choice  */
    {
      {'Q', GotoFirstFrame},
      {'\0', SerialLogsSequenceFrameResponseFunction}
    },
    /* Strings */
    {
      {'$', 0}
    }
};

static CONSOLEFRAME SerialLogsCountFrame = {
    SerialLogsCountFrameDisplayFunction,        /* Start function   */
    NULL,                       /* Finish function  */
    2,                          /* # of choices     */
    /* Choice */
    {
      {'Q', GotoFirstFrame},
      {'\0', SerialLogsCountFrameResponseFunction}
    },
    /* Strings */
    {
      {'$', 0}
    }
};

static CONSOLEFRAME SerialLogsModeFrame = {
    SerialLogsModeFrameDisplayFunction, /* Start function   */
    NULL,                       /* Finish function  */
    2,                          /* # of choices     */
    /* Choice */
    {
      {'Q', GotoFirstFrame},
      {'\0', SerialLogsModeFrameResponseFunction}
    },
    /* Strings */
    {
      {'$', 0}
    }
};

/*****************************************************************************
** Code Start
*****************************************************************************/

void SerialLogsFrameChoiceFunction(void)
{
    logSequence = 0;
    logCount = 0;
    logMode = 0;
    logExtended = FALSE;
    logBadInput = FALSE;
    currentFramePtr = &SerialLogsSequenceFrame;
}


/* ------------------------------------------------------------------------ */
static void SerialLogsSequenceFrameDisplayFunction(void)
{
    unsigned char LineNumber = 0;

    if (logBadInput == TRUE)
    {
        sprintf(currentFramePtr->line[LineNumber++],
                "\r\n\nInvalid sequence number; try again.");
        logBadInput = FALSE;
    }
    else
    {
        sprintf(currentFramePtr->line[LineNumber++],
                "\r\n******** Serial Log Information Display ********\r\n");
    }

    sprintf(currentFramePtr->line[LineNumber++],
            "\r\nSequence number to begin with (No Entry take default(Last Entry)): ");
    sprintf(currentFramePtr->line[LineNumber], "$");
}


/* ------------------------------------------------------------------------ */
static void SerialLogsSequenceFrameResponseFunction(void)
{
    if (command.line[0] == CRLF)
    {
        logSequence = 0;
        currentFramePtr = &SerialLogsCountFrame;
    }
    else
    {
        if (sscanf(command.line, "%u", &logSequence) != 1)
        {
            logBadInput = TRUE;
        }
        else
        {
            logMode |= MODE_USE_SEQUENCE;
            currentFramePtr = &SerialLogsCountFrame;
        }
    }
}


/* ------------------------------------------------------------------------ */
static void SerialLogsCountFrameDisplayFunction(void)
{
    unsigned char LineNumber = 0;

    if (logBadInput == TRUE)
    {
        sprintf(currentFramePtr->line[LineNumber++],
                "\r\n\nInvalid number of logs; try again.");
        logBadInput = FALSE;
    }

    sprintf(currentFramePtr->line[LineNumber++],
            "\r\nNumber of logs to display (No Entry take default(20)): ");
    sprintf(currentFramePtr->line[LineNumber], "$");
}


/* ------------------------------------------------------------------------ */
static void SerialLogsCountFrameResponseFunction(void)
{
    if (command.line[0] == CRLF)
    {
        logCount = 20;
        currentFramePtr = &SerialLogsModeFrame;
    }
    else
    {
        if (sscanf(command.line, "%u", &logCount) != 1)
        {
            logBadInput = TRUE;
        }
        else
        {
            currentFramePtr = &SerialLogsModeFrame;
        }
    }
}


/* ------------------------------------------------------------------------ */
static void SerialLogsModeFrameDisplayFunction(void)
{
    unsigned char LineNumber = 0;

    if (logBadInput == TRUE)
    {
        sprintf(currentFramePtr->line[LineNumber++],
                "\r\n\nInvalid extended data flag; try again.");
        logBadInput = FALSE;
    }

    /*
     * Display controller node ID
     */
    sprintf(currentFramePtr->line[LineNumber++],
            "\r\nView Extended Data ('1'= Yes | '0'= No (0)) ?: ");
    sprintf(currentFramePtr->line[LineNumber++], "$");
}


/* ------------------------------------------------------------------------ */
static void SerialLogsModeFrameResponseFunction(void)
{


    if (command.line[0] == CRLF)
    {
        /* Do Nothing */
    }
    else
    {
        if (sscanf(command.line, "%u", &logExtended) != 1)
        {
            logBadInput = TRUE;
        }
        else
        {
            if (logExtended == TRUE)
            {
                logMode |= MODE_EXTENDED_MESSAGE;
            }
        }
    }

    if (logBadInput == FALSE)
    {
        SerialLogsDisplayLogs(logSequence, logCount, logMode);
        currentFramePtr = &FirstFrame;
    }
}

/* ------------------------------------------------------------------------ */
static void SerialLogsDisplayLogs(UINT32 sequence, UINT32 count, UINT16 mode)
{
    INT32       rc = PI_GOOD;
    UINT32      index1 = 0;
    PI_LOG_INFO_MODE0_RSP *ptrLogs = NULL;
    PI_LOG_EVENT *ptrLogEvt = NULL;
    XIO_PACKET  reqPacket = { NULL, NULL };
    XIO_PACKET  rspPacket = { NULL, NULL };
    UINT32      printBufferLength = 0;
    char       *printBuffer;

    /*
     * Allocate memory for the request (header and data) and the
     * response header. The response data will be allocated in the called
     * function.
     */
    reqPacket.pHeader = MallocWC(sizeof(*reqPacket.pHeader));
    reqPacket.pPacket = MallocWC(sizeof(PI_LOG_INFO_REQ));
    rspPacket.pHeader = MallocWC(sizeof(*rspPacket.pHeader));

    /*
     * Fill in the Header
     */
    reqPacket.pHeader->packetVersion = 1;
    rspPacket.pHeader->packetVersion = 1;
    reqPacket.pHeader->commandCode = PI_LOG_INFO_CMD;
    reqPacket.pHeader->length = sizeof(PI_LOG_INFO_REQ);

    do
    {
        if (rspPacket.pPacket != NULL)
        {
            Free(rspPacket.pPacket);
        }

        rspPacket.pPacket = NULL;

        /* Setup the log information Information Request */
        ((PI_LOG_INFO_REQ *)reqPacket.pPacket)->sequenceNumber = sequence;
        ((PI_LOG_INFO_REQ *)reqPacket.pPacket)->mode = mode;
        ((PI_LOG_INFO_REQ *)reqPacket.pPacket)->eventCount = count;

        /* Issue the command through the packet command handler */
        rc = PacketCommandHandler(&reqPacket, &rspPacket);

        if (rc == PI_GOOD)
        {

            /*
             * Set some useful pointers to walk through the response
             */
            ptrLogs = (PI_LOG_INFO_MODE0_RSP *)rspPacket.pPacket;
            ptrLogEvt = (PI_LOG_EVENT *)(rspPacket.pPacket + sizeof(*ptrLogs));

            /*
             * Loop through the response and print the results
             * to the serial console.
             */
            for (index1 = 0; index1 < ptrLogs->eventCount; ++index1)
            {
                /*
                 * Allocate the memory for the buffer to print               //add to comment
                 * to the serial console.
                 */
                printBufferLength = (64 + ptrLogEvt->ascii.length);
                printBuffer = MallocWC(printBufferLength);

                /*
                 * Format the string to print to the console and
                 * at the same time remember the length of the new
                 * string.
                 */
                printBufferLength = sprintf(printBuffer, "\r\n  %7lu %s 0x%04hX %s %s",
                            ptrLogEvt->ascii.sequenceNumber, ptrLogEvt->ascii.timeAndDate,
                            ptrLogEvt->ascii.eventCode, ptrLogEvt->ascii.messageType,
                            ptrLogEvt->ascii.messageDescr);

                /* Send the string to the serial console to be displayed. */
                SerialBufferedWriteString(printBuffer, printBufferLength);

                /* Flush the string to the console. */
                SerialBufferedWriteFlush(TRUE);

                /*
                 * Set our log pointer to point to the next log in the list.
                 */
                ptrLogEvt = (PI_LOG_EVENT *)((UINT8 *)ptrLogEvt +
                                             sizeof(ptrLogEvt->length) +
                                             ptrLogEvt->length);

                /*
                 * Deallocate our String and reinitialize
                 * some default values.
                 */
                Free(printBuffer);
                printBufferLength = 0;

                /*
                 * We are using a slow serial port to output the logs
                 * so make sure we exchange for every 10 we print out.
                 */
                if ((index1 % 10) == 0)
                {
                    TaskSwitch();
                }
            }

            if ((count - ptrLogs->eventCount) <= 0)
            {
                /*
                 * We're done.
                 */
                break;
            }

            count = count - ptrLogs->eventCount;
            sequence = ptrLogs->sequenceNumber - 1;
            mode |= MODE_USE_SEQUENCE;
        }

        else
        {
            break;
        }

    } while (TRUE);

    /*
     * Deallocate the packets we used to get and receive the logs.
     */

    Free(reqPacket.pHeader);
    Free(rspPacket.pHeader);
    Free(reqPacket.pPacket);

    if (rc != PI_TIMEOUT)
    {
        Free(rspPacket.pPacket);
    }
}

/* ------------------------------------------------------------------------ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
