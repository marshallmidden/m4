/* $Id: SerBuff.c 156020 2011-05-27 16:18:33Z m4 $ */
/*============================================================================
** FILE NAME:       SerBuff.c
** MODULE TITLE:    Buffered serial port I/O routines
**
** Copyright (c) 2001-2009 Xiotech Corporation. All rights reserved.
**==========================================================================*/
#include "SerBuff.h"

#include "ccb_hw.h"
#include "idr_structure.h"
#include "kernel.h"
#include "mutex.h"
#include "pcb.h"
#include "SerCon.h"
#include "serial.h"
#include "nvram.h"
#include "XIO_Std.h"
#include "XIO_Macros.h"
#include "debug_files.h"

/*****************************************************************************
** Private defines
*****************************************************************************/
typedef struct SERIAL_BUFFER_ITEM_STRUCT
{
    char        item;
} SERIAL_BUFFER_ITEM, *SERIAL_BUFFER_ITEM_PTR;

/*
** Some development tools use this header format, so any changes need to
** take into account these external tools.  The begin, in, out, and end pointers
** should remain at the beginning of the structure and stay in that order.
*/
typedef struct SERIAL_BUFFER_HEADER_STRUCT
{
    SERIAL_BUFFER_ITEM_PTR beginPtr;    /* Points to the begining of buffer */
    SERIAL_BUFFER_ITEM_PTR inPtr;       /* Points to where the next character will be queued to */
    SERIAL_BUFFER_ITEM_PTR outPtr;      /* Points to where the next character will be dequeued from */
    SERIAL_BUFFER_ITEM_PTR endPtr;      /* Points to the end of the buffer (one beyond the last entry) */
    MUTEX       bufferEnqueueMutex;     /* Mutex for enqueueing to this buffer */
    MUTEX       bufferDequeueMutex;     /* Mutex for dequeueing from this buffer */
} SERIAL_BUFFER_HEADER, *SERIAL_BUFFER_HEADER_PTR;

typedef struct SERIAL_INPUT_BUFFER_STRUCT
{
    SERIAL_BUFFER_HEADER header;
    SERIAL_BUFFER_ITEM buffer[SIZE_16];
} SERIAL_INPUT_BUFFER, *SERIAL_INPUT_BUFFER_PTR;

typedef struct SERIAL_OUTPUT_BUFFER_STRUCT
{
    SERIAL_BUFFER_HEADER header;
    SERIAL_BUFFER_ITEM buffer[SIZE_64K -                        /* Total structure size */
                              sizeof(SERIAL_BUFFER_HEADER) -    /* Minus header size */
                              1];       /* Minus one to avoid COFF structure limitation */
} SERIAL_OUTPUT_BUFFER, *SERIAL_OUTPUT_BUFFER_PTR;

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static QUEUE_RETURN_TYPE SerialBufferedEnqueue(SERIAL_BUFFER_HEADER_PTR bufferHeaderPtr,
                                               const char *pData);
static QUEUE_RETURN_TYPE SerialBufferedDequeue(SERIAL_BUFFER_HEADER_PTR bufferHeaderPtr,
                                               char *pData);
static UINT32 SerialBufferedGetSizeUsed(SERIAL_BUFFER_HEADER_PTR bufferHeaderPtr,
                                        UINT32 *queueSizeUsedPtr);
static UINT32 SerialBufferedInitializeInputBuffer(SERIAL_INPUT_BUFFER_PTR inputBufferPtr);
static UINT32 SerialBufferedInitializeOutputBuffer(SERIAL_OUTPUT_BUFFER_PTR
                                                   outputBufferPtr);
static void SerialInterruptHandlerTask(TASK_PARMS *parms);

/*****************************************************************************
** Private variables
*****************************************************************************/

/* Buffers are initialized in init functions to reduce image footprint */
static SERIAL_INPUT_BUFFER userPortReceiveBuffer;
static SERIAL_OUTPUT_BUFFER userPortTransmitBuffer;

static PCB *userPortReceiveTaskPCBPtr = NULL;
static PCB *userPortTransmitTaskPCBPtr = NULL;

static const char transmitBufferErrorMessage[] = "\r\n*** Serial transmit buffer error ***\r\n";

/*****************************************************************************
** Code Start
*****************************************************************************/

/******************************************************************************
** NAME:        SerialBufferedInitializeInputBuffer
**
** INPUT:       inputBufferPtr
*******************************************************************************/
static UINT32 SerialBufferedInitializeInputBuffer(SERIAL_INPUT_BUFFER_PTR inputBufferPtr)
{
    UINT32      returnCode = GOOD;

    if (inputBufferPtr != NULL)
    {
        /*
         * Initialize the diagnostic port transmit mutexes
         */
        InitMutex(&inputBufferPtr->header.bufferEnqueueMutex);
        InitMutex(&inputBufferPtr->header.bufferDequeueMutex);

        /*
         * Initialize the header pointers
         */
        inputBufferPtr->header.beginPtr = &inputBufferPtr->buffer[0];
        inputBufferPtr->header.inPtr = &inputBufferPtr->buffer[0];
        inputBufferPtr->header.outPtr = &inputBufferPtr->buffer[0];
        inputBufferPtr->header.endPtr = &inputBufferPtr->buffer[sizeof(inputBufferPtr->buffer)];
    }
    else
    {
        returnCode = ERROR;
    }

    return (returnCode);
}


/******************************************************************************
** NAME:        SerialBufferedInitializeOutputBuffer
**
** INPUT:       outputBufferPtr
*******************************************************************************/
static UINT32 SerialBufferedInitializeOutputBuffer(SERIAL_OUTPUT_BUFFER_PTR
                                                   outputBufferPtr)
{
    UINT32      returnCode = GOOD;

    if (outputBufferPtr != NULL)
    {
        /*
         * Initialize the diagnostic port transmit mutexes
         */
        InitMutex(&outputBufferPtr->header.bufferEnqueueMutex);
        InitMutex(&outputBufferPtr->header.bufferDequeueMutex);

        /*
         * Initialize the header pointers
         */
        outputBufferPtr->header.beginPtr = &outputBufferPtr->buffer[0];
        outputBufferPtr->header.inPtr = &outputBufferPtr->buffer[0];
        outputBufferPtr->header.outPtr = &outputBufferPtr->buffer[0];
        outputBufferPtr->header.endPtr = &outputBufferPtr->buffer[sizeof(outputBufferPtr->buffer)];
    }
    else
    {
        returnCode = ERROR;
    }

    return (returnCode);
}


/******************************************************************************
** NAME:        SerialUserPortReceiveTask
**
** DESCRIPTION: Called from the serial input driver
**
** INPUT:       framePtr
**
** OUTPUT:      None
*******************************************************************************/
NORETURN void SerialUserPortReceiveTask(UNUSED TASK_PARMS *parms)
{
    UINT32      bufferSizeUsed;

    /*
     * Initialize the user port receive buffer
     */
    SerialBufferedInitializeInputBuffer(&userPortReceiveBuffer);

    /*
     * Save the PCB data into module variables
     */
    userPortReceiveTaskPCBPtr = XK_GetPcb();

    TaskCreate(SerialInterruptHandlerTask, NULL);

    /*
     * Receive task is awakened only when the receive buffer is not empty
     */
    while (1)
    {
        /*
         * Check if the receive task can go to sleep
         */
        if ((SerialBufferedGetSizeUsed(&userPortReceiveBuffer.header, &bufferSizeUsed) == GOOD) &&
            (bufferSizeUsed == 0))
        {
            /*
             * Input buffer is empty, so sleep until reawakened
             */
            TaskSetState(userPortReceiveTaskPCBPtr, PCB_NOT_READY);
        }
        else
        {
            /*
             * Call the function that cares about user port input characters
             */
            SerialConsoleHandleReceiveChar();
            SerialBufferedGetSizeUsed(&userPortReceiveBuffer.header, &bufferSizeUsed);
        }

        TaskSwitch();
    }
}


/******************************************************************************
** NAME:        SerialUserPortTransmitTask
**
** DESCRIPTION: Called when user serial port output is needed
**
** INPUT:       framePtr
**
** OUTPUT:      None
*******************************************************************************/
NORETURN void SerialUserPortTransmitTask(UNUSED TASK_PARMS *parms)
{
    UINT32      bufferSizeUsed;
    char        transmitChar;

    /*
     * Initialize the user port transmit buffer mutexes
     */
    SerialBufferedInitializeOutputBuffer(&userPortTransmitBuffer);

    /*
     * Save the PCB data into module variables
     */
    userPortTransmitTaskPCBPtr = XK_GetPcb();

    /*
     * Transmit task is awakened only when the transmit buffer is not empty.
     */
    while (1)
    {
        /*
         * Check that the channel isn't already busy sending data and that
         * the buffered serial task is initialized and has buffered data.
         * WARNING: This is sequence sensitive - do not reorder!
         */
        if (SerialBufferedGetSizeUsed(&userPortTransmitBuffer.header, &bufferSizeUsed) == GOOD)
        {
            /*
             * Pump out as many characters as possible, until we need to wait for
             * the DUART to catch up.
             */
            while (bufferSizeUsed > 0 &&
                   SerialBufferedDequeue(&userPortTransmitBuffer.header,
                                         &transmitChar) == QUEUE_RETURN_GOOD)
            {
                SerialWrite(&transmitChar, sizeof(transmitChar));
                bufferSizeUsed--;
            }

            /*
             * Check if the buffer is empty.  If it is, put this task to sleep.
             */
            if (bufferSizeUsed == 0)
            {
                TaskSetState(userPortTransmitTaskPCBPtr, PCB_NOT_READY);
            }
        }

        TaskSwitch();
    }
}


/*****************************************************************************
** FUNCTION NAME: SerialInterruptHandler
**
** PARAMETERS:  None
**
** DESCRIPTION: Handle DUART channel A's Rx interrupt.  This routine gets
**              characters from the DUART receive FIFO and places them into
**              userPortReceiveBuffer.buffer
**
** NOTE:        SerialInterruptHandler should be connected XINT5
**
** RETURNS:     Nothing
******************************************************************************/
static NORETURN void SerialInterruptHandlerTask(UNUSED TASK_PARMS *parms)
{
    INT32       selectRC = 0;
    char        readChar = 0;
    fd_set      readSet;        /* Uninitialized */

    while (1)
    {
        /*
         * Set up the select and timeout period
         */
        FD_ZERO(&readSet);
        FD_SET(userPortHandle, &readSet);

        /*
         * Wait for the select() to fire - blocking
         */
        selectRC = Select(userPortHandle + 1, &readSet, NULL, NULL, NULL);

        if (selectRC > 0)
        {
            if (read(userPortHandle, &readChar, sizeof(readChar)) > 0)
            {
                SerialBufferedEnqueue(&userPortReceiveBuffer.header, &readChar);

                /*
                 * Find where the new character will be inserted and wake up
                 * the user serial port receive task
                 */
                if (userPortReceiveTaskPCBPtr != NULL)
                {
                    /*
                     * Wake up DUART channel A's receive task
                     */
                    if (TaskGetState(userPortReceiveTaskPCBPtr) != PCB_READY)
                    {
                        TaskReadyState(userPortReceiveTaskPCBPtr);
                    }
                }
            }
        }
    }
}


/******************************************************************************
** NAME:        SerialBufferedReadChar
**
** DESCRIPTION: Get a character from buffer
**
** INPUT:       pData   - Pointer to where character is to be returned
**
** OUTPUT:      GOOD
**              ERROR
*******************************************************************************/
UINT32 SerialBufferedReadChar(char *pData)
{
    UINT32      bufferSizeUsed = 0;

    if (!pData)
    {
        return ERROR;
    }

    *pData = '\0';              /* Set default return value */

    if (SerialBufferedGetSizeUsed(&userPortReceiveBuffer.header, &bufferSizeUsed) == GOOD &&
        bufferSizeUsed > 0 &&
        SerialBufferedDequeue(&userPortReceiveBuffer.header, pData) == QUEUE_RETURN_GOOD)
    {
        return GOOD;
    }

    return ERROR;
}


/******************************************************************************
** NAME:        SerialBufferedWriteChar
**
** DESCRIPTION: Put a character into output buffer
**
** INPUT:       pData
**
** OUTPUT:      GOOD    - Character written to specified port(s)
**              ERROR   - Character not queued
*******************************************************************************/
UINT32 SerialBufferedWriteChar(const char *pData)
{
    if (userPortTransmitTaskPCBPtr)
    {
        switch (SerialBufferedEnqueue(&userPortTransmitBuffer.header, pData))
        {
            case QUEUE_RETURN_GOOD:
            case QUEUE_RETURN_BUSY:
            case QUEUE_RETURN_FULL:
                /* Wake up serial channel transmit task */
                if (TaskGetState(userPortTransmitTaskPCBPtr) != PCB_READY)
                {
                    TaskReadyState(userPortTransmitTaskPCBPtr);
                }
                return GOOD;

            case QUEUE_RETURN_ERROR:
            default:
                return ERROR;
        }
    }
    else
    {
        /*
         * Serial transmit task NOT initialized, so send it
         * out directly from here, bypassing the buffer.
         */
        return SerialWrite(pData, 1);
    }
}


/******************************************************************************
** NAME:        SerialBufferedWriteString
**
** INPUT:       *buffer
**              length
**
** OUTPUT:      GOOD  - The entire buffer was written (queued) successfully
**              ERROR - Nothing was written
*******************************************************************************/
UINT32 SerialBufferedWriteString(const char *pBuffer, UINT32 length)
{
    UINT32      returnCode = GOOD;
    UINT32      outputCharNum = 0;

    if (length == 0)
    {
        return ERROR;
    }

    if (!userPortTransmitTaskPCBPtr)
    {
        /*
         * Serial transmit task NOT initialized, so send it
         * out directly from here, bypassing the buffer.
         */
        return SerialWrite(pBuffer, length);
    }

    while (outputCharNum < length && returnCode == GOOD)
    {
        /* Insert CR before doing LF */

        if (pBuffer[outputCharNum] == '\n' &&
            SerialBufferedWriteChar((char *)"\r") == QUEUE_RETURN_ERROR)
        {
            returnCode = ERROR;
        }

        if (returnCode == GOOD &&
            SerialBufferedWriteChar(&pBuffer[outputCharNum]) == QUEUE_RETURN_ERROR)
        {
            returnCode = ERROR;
        }

        if (returnCode == GOOD)
        {
            /*
             * Throw away character on BUSY state... can't exchange
             * to allow other task to free up queue, but this
             * shouldn't occur in actual practice.
             */
            outputCharNum++;
        }
        else
        {
            /*
             * Channel A's transmit buffer is broken.  Send out a
             * warning message, bypassing the buffer.
             */
            SerialWrite(transmitBufferErrorMessage, strlen(transmitBufferErrorMessage));
        }
    }

    return returnCode;
}


/******************************************************************************
** NAME:        SerialBufferedWriteFlush
**
** INPUT:       doExchange  -   TRUE (Exchange while flushing the buffer and
**                                    do not force unlock of user or diagnostic
**                                    PortTransmitBuffer)
**                              FALSE (Do not Exchange while flushing the buffer.
**                                    Force unlock of user or diagnostic
**                                    PortTransmitBuffer)
**
** OUTPUT:      GOOD  - The entire buffer was flushed successfully
**              ERROR - Error flushing the buffer
*******************************************************************************/
UINT32 SerialBufferedWriteFlush(UINT8 doExchange)
{
    UINT32      queueReturnType = QUEUE_RETURN_GOOD;
    UINT32      returnCode = GOOD;
    char        transmitChar = 0;

    /*
     * Force the user port transmit buffer mutex to be unlocked
     * if we are not able to exchange.  We're hijacking the buffers.
     */
    if (doExchange == FALSE)
    {
        UnlockMutex(&userPortTransmitBuffer.header.bufferDequeueMutex);
    }

    if (userPortTransmitTaskPCBPtr)
    {
        while (returnCode == GOOD &&
               (queueReturnType == QUEUE_RETURN_GOOD ||
                queueReturnType == QUEUE_RETURN_BUSY))
        {
            queueReturnType = SerialBufferedDequeue(&userPortTransmitBuffer.header, &transmitChar);

            if (queueReturnType == QUEUE_RETURN_GOOD)
            {
                returnCode = SerialPutChar(transmitChar);
            }

            if (doExchange == TRUE)
            {
                TaskSwitch();
            }
        }
    }

    return returnCode;
}


/******************************************************************************
** NAME:        SerialBufferedEnqueue
**
** INPUT:       bufferHeaderPtr
**              pData
**
** OUTPUT:      QUEUE_RETURN_GOOD  - A character was enqueued successfully
**              QUEUE_RETURN_ERROR - A character was not enqueued (unknown)
**              QUEUE_RETURN_BUSY  - A character was not enqueued (busy)
**              QUEUE_RETURN_FULL  - A character was not enqueued (full)
*******************************************************************************/
static QUEUE_RETURN_TYPE SerialBufferedEnqueue(SERIAL_BUFFER_HEADER_PTR bufferHeaderPtr,
                                               const char *pData)
{
    QUEUE_RETURN_TYPE returnType = QUEUE_RETURN_ERROR;
    SERIAL_BUFFER_ITEM_PTR nextPtr = NULL;

    /*
     * To protect the accesses to the transmit queue from multiple
     * tasks, lock the mutex.  Mutex must be NOWAIT type, since we don't
     * want debug print statements to exchange.
     * WARNING: This is sequence sensitive - do not reorder!
     */
    if ((bufferHeaderPtr != NULL) && (pData != NULL))
    {
        if (LockMutex(&bufferHeaderPtr->bufferEnqueueMutex, MUTEX_NOWAIT) == TRUE)
        {
            if ((bufferHeaderPtr->inPtr + 1) < bufferHeaderPtr->endPtr)
            {
                nextPtr = bufferHeaderPtr->inPtr + 1;
            }
            else
            {
                nextPtr = bufferHeaderPtr->beginPtr;
            }

            /*
             * Check for the buffer being full before placing
             * this new character into the buffer.
             */
            if (nextPtr != bufferHeaderPtr->outPtr)
            {
                /*
                 * Insert the new character
                 */
                bufferHeaderPtr->inPtr->item = *pData;

                /*
                 * Change the "in" pointer
                 */
                bufferHeaderPtr->inPtr = nextPtr;

                returnType = QUEUE_RETURN_GOOD;
            }
            else
            {
                returnType = QUEUE_RETURN_FULL;
            }

            /*
             * We're done accessing the transmit queue, so unlock the mutex.
             */
            UnlockMutex(&bufferHeaderPtr->bufferEnqueueMutex);
        }
        else
        {
            returnType = QUEUE_RETURN_BUSY;
        }
    }

    return (returnType);
}


/******************************************************************************
** NAME:        SerialBufferedDequeue
**
** INPUT:       bufferHeaderPtr
**              pData
**
** OUTPUT:      QUEUE_RETURN_GOOD  - A character was dequeued successfully
**              QUEUE_RETURN_ERROR - A character was not dequeued (unknown)
**              QUEUE_RETURN_BUSY  - A character was not dequeued (busy)
**              QUEUE_RETURN_EMPTY - A character was not dequeued (empty)
*******************************************************************************/
static QUEUE_RETURN_TYPE SerialBufferedDequeue(SERIAL_BUFFER_HEADER_PTR bufferHeaderPtr,
                                               char *pData)
{
    QUEUE_RETURN_TYPE returnType = QUEUE_RETURN_ERROR;

    /*
     * To protect the accesses to the transmit queue from multiple
     * tasks, lock the mutex.  Mutex must be NOWAIT type, since we don't
     * want debug print statements to exchange.
     * WARNING: This is sequence sensitive - do not reorder!
     */
    if ((bufferHeaderPtr != NULL) && (pData != NULL))
    {
        if (LockMutex(&bufferHeaderPtr->bufferDequeueMutex, MUTEX_NOWAIT) == TRUE)
        {
            if (bufferHeaderPtr->outPtr != bufferHeaderPtr->inPtr)
            {
                /*
                 * Send the next character in the output buffer
                 */
                *pData = bufferHeaderPtr->outPtr->item;

                /*
                 * Check that the output pointer hasn't gone past the end
                 * of the output buffer.
                 */
                if ((bufferHeaderPtr->outPtr + 1) < bufferHeaderPtr->endPtr)
                {
                    /*
                     * Increment the output pointer
                     */
                    bufferHeaderPtr->outPtr++;
                }
                else
                {
                    bufferHeaderPtr->outPtr = bufferHeaderPtr->beginPtr;
                }

                returnType = QUEUE_RETURN_GOOD;
            }
            else
            {
                returnType = QUEUE_RETURN_EMPTY;
            }

            /*
             * We're done accessing the transmit queue, so unlock the mutex.
             */
            UnlockMutex(&bufferHeaderPtr->bufferDequeueMutex);
        }
        else
        {
            returnType = QUEUE_RETURN_BUSY;
        }
    }

    return (returnType);
}

/******************************************************************************
** NAME:        SerialBufferedGetSizeUsed
**
** INPUT:       bufferHeaderPtr
**              queueSizeUsedPtr
**
** OUTPUT:      GOOD
**              ERROR
*******************************************************************************/
static UINT32 SerialBufferedGetSizeUsed(SERIAL_BUFFER_HEADER_PTR bufferHeaderPtr,
                                        UINT32 *queueSizeUsedPtr)
{
    UINT32      returnCode = ERROR;
    SERIAL_BUFFER_HEADER tempBufferHeader;

    /*
     * The compiler copies the bufferHeaderPtr information into a temporary
     * header, and does so with load-quads... so the begin, in, out, and end
     * pointers are loaded atomically.  Use this temporary information to
     * determine the used size of the buffer.
     */
    if (queueSizeUsedPtr != NULL)
    {
        if (bufferHeaderPtr != NULL)
        {
            /*
             * Atmoic copy of header pointer information
             */
            tempBufferHeader = *bufferHeaderPtr;

            /*
             * outPtr always trails inPtr, but we need to account for wrap conditions
             */
            if (tempBufferHeader.outPtr <= tempBufferHeader.inPtr)
            {
                /*
                 * inPtr has not wrapped
                 * Distance from outPtr to the inPtr
                 */
                *queueSizeUsedPtr = tempBufferHeader.inPtr - tempBufferHeader.outPtr;
            }
            else
            {
                /*
                 * inPtr has wrapped and outPtr hasn't
                 * Distance from outPtr to the end of the buffer
                 */
                *queueSizeUsedPtr = tempBufferHeader.endPtr - tempBufferHeader.outPtr;

                /* Distance from beginPtr to inPtr */
                *queueSizeUsedPtr += tempBufferHeader.inPtr - tempBufferHeader.beginPtr;
            }

            /* Scale by the size of one buffer entry */
            *queueSizeUsedPtr /= sizeof(*(tempBufferHeader.beginPtr));

            returnCode = GOOD;
        }
        else
        {
            *queueSizeUsedPtr = 0;
        }
    }

    return (returnCode);
}


/******************************************************************************
** NAME:        SerialBufferedGetQueueDepth
**
** OUTPUT:      GOOD
**              ERROR
*******************************************************************************/
UINT32 SerialBufferedGetQueueDepth(UINT8 whichDirection, UINT32 *queueDepthPtr)
{
    if (!queueDepthPtr)
    {
        return ERROR;
    }

    switch (whichDirection)
    {
        case TRANSMIT:
            return SerialBufferedGetSizeUsed(&userPortTransmitBuffer.header,
                                             queueDepthPtr);

        case RECEIVE:
            return SerialBufferedGetSizeUsed(&userPortReceiveBuffer.header,
                                             queueDepthPtr);

        default:
            return ERROR;
    }
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
