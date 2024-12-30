/* $Id: debug_files.c 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file   debug_files.c
**
**  @brief  Debugging function to write messages to a file
**
**  Copyright (c) 2001-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#include "debug_files.h"

#include <errno.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "i82559.h"
#include "error_handler.h"
#include "idr_structure.h"
#include "logging.h"
#include "mode.h"
#include "pcb.h"
#include "PortServerUtils.h"
#include "SerBuff.h"
#include "serial.h"
#include "stdarg.h"
#include "XIO_Std.h"
#include "XIO_Macros.h"

/*****************************************************************************
** Private defines
*****************************************************************************/
#define DEBUG_BUF_LENGTH    (SIZE_2K)

#define NUM_TO_SEND_PER_EXCHANGE 250
#define TCPReady() (XK_KernelReady() == TRUE)

#define ETH_QUEUE_WRAP_MARKER 0xFFFF
#define ETH_QUEUE_BUF_SIZE    (SIZE_64K-1)

/*
** This macro makes it a LOT easier to access the fields
** in the various ETH_WRITER_ENTRY structures.
*/
#define ETH_ENTRY_PTR(ptr) ((ETH_WRITER_ENTRY *)ethWrtQ.ptr)

typedef struct
{
    UINT16      length;         /* sizeof entire entry              */
    UINT16      sequence;       /* sequence number of this entry    */
    char        text[0];        /* the actual text to send          */
} ETH_WRITER_ENTRY;

typedef struct
{
    UINT8      *pBase;          /* base of queue                    */
    UINT8      *pEnd;           /* top of the queue                 */
    UINT8      *pIn;            /* input pointer                    */
    UINT8      *pOut;           /* output pointer                   */

    INT32       count;          /* count of outstanding messages    */
    UINT32      shutdownTask;   /* EthWriterTask shutdown flag      */
    UINT32      sockOpen;       /* UDP socket open flag             */
    INT32       sockfd;         /* socket descriptor                */

    struct sockaddr_in sin;     /* IP/port to send to               */

    UINT32      lastIPaddr;     /* last sent-to IP address          */
    PCB        *pPcb;           /* PCB of EthWriterTask             */
    ETH_WRITER_ENTRY *entry;    /* current entry to send           */
    UINT16      seqNum;         /* sequence num of this message     */
    UINT16      reserved1;

    /* Queue Statistics */
    UINT32      outOfSync;      /* The queue is hozed in one way    */
    UINT32      outOfRange;     /*   or another...                  */
    UINT32      underRun;       /* The count went negative          */
    UINT32      tossed;         /* total tossed on wrap             */

    UINT32      enqueued;       /* total enqueued                   */
    UINT32      dequeued;       /* total dequeued                   */
    UINT32      sent;           /* total sent                       */
    UINT32      wraps;          /* total queue wraps                */
} ETH_WRITER_QUEUE;

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static void EthWriterTask(TASK_PARMS *parms);
static UINT32 EthDebugAddr(void);
static void EnqueueUDPMsg(UINT8 *msg, INT32 len);
static ETH_WRITER_ENTRY *DequeueUDPMsg(void);

/*****************************************************************************
** Private variables
*****************************************************************************/
static ETH_WRITER_QUEUE ethWrtQ;
static UINT8 ethQueueBuffer[ETH_QUEUE_BUF_SIZE];

/*****************************************************************************
** Code Start
*****************************************************************************/

/**
******************************************************************************
**
**  @brief  Queue a message for UDP transfer out to a debug console.
**
**  @param  level - Message level value
**  @param  fmt - Pointer to format string
**  @param  array of arguments for format string
**
******************************************************************************
**/
void _DPrintf(UINT32 level, const char *fmt, ...)
{
    char        buf[DEBUG_BUF_LENGTH * 2];
    char       *bufP = buf;
    va_list     args;
    INT32       len;

    if ((level & modeData.ccb.bitsDPrintf) == 0)
    {
        /*
         * Early exit if not enabled
         */
        return;
    }

    /*
     * If, for some reason, DebugPrintf has been called before
     * the serial port has been initialized (fault in early startup)
     * initialize it and try and display the data.
     */
    if (TestSerialInitFlag() != TRUE)
    {
        SerialInit();
    }

#ifdef DEBUG_PRINTF_TIME
    bufP += sprintf(bufP, "%u ", K_t1cnt);
#endif  /* DEBUG_PRINTF_TIME */

#ifdef DEBUG_PRINTF_HIRES_TIME
    bufP += sprintf(bufP, ":%u ", TRR0 - TCR0);
#endif  /* DEBUG_PRINTF_HIRES_TIME */

#ifdef DEBUG_PRINTF_PCB
    bufP += sprintf(bufP, "-%p ", XK_GetPcb());
#endif  /* DEBUG_PRINTF_PCB */

    va_start(args, fmt);
    bufP += vsprintf((char *)bufP, (char *)fmt, args);
    va_end(args);

    len = bufP - buf;

    /*
     * 'buf' is twice the size we actually need.  Lop off the input string
     * if it blows over the MAX length.  Set the last character to '+' to
     * indicate that there *was* more...
     */
    if (len > DEBUG_BUF_LENGTH)
    {
        len = DEBUG_BUF_LENGTH;
        buf[DEBUG_BUF_LENGTH - 1] = '+';
        buf[DEBUG_BUF_LENGTH] = 0;
    }

    /*
     * Timestamp each line that's sent to the serial, but avoid sending
     * it over the Ethernet.  The machine receiving the debugconsole data
     * will place it's own timestamp on each line.
     */
    {
        UINT32      timeLength = 0;
        TIMESTAMP   timestamp;
        char        timeString[20];

        RTC_GetTimeStamp(&timestamp);

        timeLength = snprintf((char *)timeString, sizeof(timeString),
                              "%02x/%02x-%02x:%02x:%02x ", timestamp.month,
                              timestamp.date, timestamp.hours, timestamp.minutes,
                              timestamp.seconds);

        timeString[sizeof(timeString) - 1] = '\0';

        fprintf(stderr, "%s%s", timeString, buf);
    }

    /*
     * Enqueue the message for UDP delivery
     */
    if (EthDebugAddr())
    {
        EnqueueUDPMsg((UINT8 *)buf, len);
    }
}


/**********************************************************************
*                                                                     *
*  Name:        EnqueueUDPMsg()                                       *
*                                                                     *
*  Description: Enqueue a message.                                    *
*                                                                     *
*  Note: no TaskSwitch()'s allowed in this function.                  *
*                                                                     *
**********************************************************************/
static void EnqueueUDPMsg(UINT8 *msg, INT32 len)
{
    /*
     * The actual space req'd to store this entry is the msg size
     * plus the overhead of the structure.
     */
    INT32       spaceNeeded = len + sizeof(ETH_WRITER_ENTRY);

    /*
     * Initialize the control block
     */
    if (ethWrtQ.pBase == NULL)
    {
        ethWrtQ.pBase = ethQueueBuffer;
        ethWrtQ.pIn = ethQueueBuffer;
        ethWrtQ.pOut = ethQueueBuffer;

        /*
         * The 'end' pointer is backed up 2 bytes to leave room for the
         * wrap marker.
         */
        ethWrtQ.pEnd = ethQueueBuffer + ETH_QUEUE_BUF_SIZE - sizeof(UINT16);

        ethWrtQ.count = 0;
    }

    /*
     * Start a writer task if one not running and TCP is ready.
     */
    if (ethWrtQ.pPcb == NULL && TCPReady())
    {
        ethWrtQ.sockOpen = FALSE;
        ethWrtQ.lastIPaddr = 0;
        memset(&ethWrtQ.sin, 0, sizeof(ethWrtQ.sin));
        ethWrtQ.shutdownTask = FALSE;

        ethWrtQ.pPcb = TaskCreate(EthWriterTask, NULL);
    }

    /*
     * Enqueue the message even if a writer was not started.
     */
    while (TRUE)
    {
        /*
         * If count == 0, the in and out pointers better be the same.
         * Count should never be < 0.
         */
        if (ethWrtQ.count <= 0)
        {
            if (ethWrtQ.count < 0)
            {
                ethWrtQ.count = 0;
                ethWrtQ.underRun++;
            }

            if (ethWrtQ.pOut != ethWrtQ.pIn)
            {
                /*
                 * Fix up the in and out pointers.
                 * This is bad, but we can recover from it.
                 */
                ethWrtQ.pOut = ethWrtQ.pIn;
                ethWrtQ.outOfSync++;
            }

            /*
             * Check that we have enough room before the end of the buffer
             */
            if (ethWrtQ.pEnd - ethWrtQ.pIn >= spaceNeeded)
            {
                /*
                 * Yup, there is enough room
                 */
                break;
            }

            /*
             * Else we have to wrap to the top.
             */
            else
            {
                ETH_ENTRY_PTR(pIn)->length = ETH_QUEUE_WRAP_MARKER;
                ethWrtQ.pIn = ethWrtQ.pOut = ethWrtQ.pBase;
                ethWrtQ.wraps++;

                /*
                 * Loop back and try again
                 */
                continue;
            }
        }
        else                /* Count is > 0 */
        {
            /*
             * If the output pointer is > than the input pointer, see if there is
             * enough room for this msg between the two pointers.
             */
            if (ethWrtQ.pOut > ethWrtQ.pIn)
            {
                if (ethWrtQ.pOut - ethWrtQ.pIn >= spaceNeeded)
                {
                    /*
                     * Yup, there is enough room
                     */
                    break;
                }
                /*
                 * Else we're behind, so advance the out pointer up by 1,
                 * discarding the unwritten msg, and try again.
                 */
                else
                {
                    if (ETH_ENTRY_PTR(pOut)->length == ETH_QUEUE_WRAP_MARKER)
                    {
                        ethWrtQ.pOut = ethWrtQ.pBase;
                    }
                    else
                    {
                        ethWrtQ.pOut += ETH_ENTRY_PTR(pOut)->length;
                        ethWrtQ.count--;
                        ethWrtQ.tossed++;
                    }

                    /*
                     * Loop back and try again
                     */
                    continue;
                }
            }

            /*
             * The output pointer is equal to or behind the input pointer, so now we
             * have to see if there is enough room between in and the end.
             */
            else
            {
                if (ethWrtQ.pEnd - ethWrtQ.pIn >= spaceNeeded)
                {
                    /*
                     * Yup, there is enough room
                     */
                    break;
                }
                /*
                 * Else we have to wrap to the top.
                 */
                else
                {
                    ETH_ENTRY_PTR(pIn)->length = ETH_QUEUE_WRAP_MARKER;
                    ethWrtQ.pIn = ethWrtQ.pBase;
                    ethWrtQ.wraps++;

                    /*
                     * Loop back and try again
                     */
                    continue;
                }
            }
        }
    }

    /*
     * Sanity check the input pointer.  If bad, flush the queue.  If we knew
     * what we were doing, we shouldn't have to do this!!  We already check
     * some of this in certain cases above, but we can't be sure we will catch
     * a problem unless it is checked here.
     */
    if ((ethWrtQ.pIn < ethWrtQ.pBase) || ((ethWrtQ.pEnd - ethWrtQ.pIn) < spaceNeeded))
    {
        ethWrtQ.pOut = ethWrtQ.pIn = ethWrtQ.pBase = ethQueueBuffer;
        ethWrtQ.tossed += ethWrtQ.count;
        ethWrtQ.count = 0;
        ethWrtQ.outOfRange++;
    }

    /*
     * Check to see if the entry we are about to write in is going to blow
     * away the entry we are trying to send (in EthWriterTask).  If so,
     * terminate the send.
     */
    if (ethWrtQ.entry &&
        ((UINT8 *)ethWrtQ.entry >= ethWrtQ.pIn &&
         ((UINT8 *)ethWrtQ.entry < (ethWrtQ.pIn + spaceNeeded))))
    {
        ethWrtQ.entry = NULL;
        ethWrtQ.tossed++;
    }

    /*
     * memcpy() is faster than strcpy(), and since we know how long the string
     * is we don't need a '\0' terminator.
     */
    memcpy(ETH_ENTRY_PTR(pIn)->text, msg, len);

    /*
     * Set the correct length and sequence number in this entry
     */
    ETH_ENTRY_PTR(pIn)->sequence = ethWrtQ.seqNum++;
    ETH_ENTRY_PTR(pIn)->length = spaceNeeded;

    /*
     * Increment the outstanding msg count
     */
    ethWrtQ.enqueued++;
    ethWrtQ.count++;

    /*
     * Move pIn forward to the next entry
     */
    ethWrtQ.pIn += spaceNeeded;

    /*
     * Either shut down or make the writer task runable
     */
    if (ethWrtQ.pPcb)
    {
        /*
         * Shut down the writer task
         */
        if (EthDebugAddr() == 0)
        {
            ethWrtQ.shutdownTask = TRUE;
        }

        /*
         * In any case, make sure the writer task is runnable
         */
        if (TaskGetState(ethWrtQ.pPcb) == PCB_NOT_READY)
        {
            TaskSetState(ethWrtQ.pPcb, PCB_READY);
        }
    }

    return;
}


/**********************************************************************
*                                                                     *
*  Name:        DequeueUDPMsg()                                       *
*                                                                     *
*  Description: Dequeue a message.                                    *
*                                                                     *
**********************************************************************/
static ETH_WRITER_ENTRY *DequeueUDPMsg(void)
{
    ETH_WRITER_ENTRY *pReturn = NULL;

    if (ethWrtQ.count > 0)
    {
        if (ETH_ENTRY_PTR(pOut)->length == ETH_QUEUE_WRAP_MARKER)
        {
            ethWrtQ.pOut = ethWrtQ.pBase;
        }

        pReturn = (ETH_WRITER_ENTRY *)ethWrtQ.pOut;

        ethWrtQ.pOut += ETH_ENTRY_PTR(pOut)->length;

        ethWrtQ.count--;
        ethWrtQ.dequeued++;
    }
    /*
     * Should never happen...
     */
    else if (ethWrtQ.count < 0)
    {
        ethWrtQ.count = 0;
        ethWrtQ.underRun++;
    }

    return pReturn;
}


/**********************************************************************
*                                                                     *
*  Name:        EthDebugAddr()                                        *
*                                                                     *
*  Description: Get the Ether Writer's sendto address.                *
*                                                                     *
*  Input:       none                                                  *
*                                                                     *
*  Returns:     the address                                           *
*                                                                     *
**********************************************************************/
static UINT32 EthDebugAddr(void)
{
    UINT32      debugIPaddr;

    /*
     * First see what the nvram debug address is set to.
     * If its 0, this indicates to shut off ethernet writes entirely.
     * This way on a factory fresh system, this address will be 0,
     * and so disabled.
     */
    debugIPaddr = GetDebugConsoleIPAddr();
    switch (debugIPaddr)
    {
        case 0:
        default:
            break;

            /*
             * If all F's, then use the Packet Intf IP address to send to.
             */
        case 0xFFFFFFFF:
            debugIPaddr = GetPortServerClientAddr();
            break;
    }

    return debugIPaddr;
}


/**********************************************************************
*                                                                     *
*  Name:        EthDebugPort()                                        *
*                                                                     *
*  Description: Get the Ether Writer's channel       .                *
*                                                                     *
*  Input:       none                                                  *
*                                                                     *
*  Returns:     the port                                              *
*                                                                     *
**********************************************************************/
UINT32 EthDebugPort(void)
{
    UINT32      debugPort;
    UINT8       channel;

    /*
     * First see what the nvram debug channel is set to.
     */
    channel = GetDebugConsoleChannel();

    switch (channel)
    {
        case 0:
        default:
            break;

            /*
             * If all F's, then set the channel to 0.
             */
        case 0xFF:
            channel = 0;
            break;
    }

    debugPort = DEBUGCON_PORT_NUMBER + channel;

    return debugPort;
}


/**********************************************************************
*                                                                     *
*  Name:        EthWriterTask()                                       *
*                                                                     *
*  Description: Forked task that does the actual UDP send().          *
*                                                                     *
*  Input:       none                                                  *
*                                                                     *
*  Returns:     none                                                  *
*                                                                     *
**********************************************************************/
static void EthWriterTask(UNUSED TASK_PARMS *parms)
{
    UINT32      thisIPaddr;
    INT32       socketError;
    INT32       sendCount = 0;
    INT32       numSentBeforeExchange = 0;
    INT32       rc = PI_SOCKET_ERROR;

    while (TRUE)
    {
        /*
         * Should we shut down?
         */
        thisIPaddr = EthDebugAddr();
        if (ethWrtQ.shutdownTask == TRUE || thisIPaddr == 0)
        {
            break;
        }

        /*
         * Open up a UDP socket, if not already open
         */
        if (ethWrtQ.sockOpen == FALSE)
        {
            ethWrtQ.sockfd = socket(AF_INET, SOCK_DGRAM, 0);
            if (ethWrtQ.sockfd == PI_SOCKET_ERROR)
            {
                socketError = errno;
                ethWrtQ.sockOpen = FALSE;
                break;
            }

            ethWrtQ.sockOpen = TRUE;
        }

        /*
         * Set up the client's address to send to
         */
        if ((thisIPaddr != ethWrtQ.lastIPaddr) ||
            (ethWrtQ.sin.sin_port != htons(EthDebugPort())))
        {
            memset(&ethWrtQ.sin, 0, sizeof(ethWrtQ.sin));
            ethWrtQ.sin.sin_family = AF_INET;
            ethWrtQ.sin.sin_addr.s_addr = thisIPaddr;
            ethWrtQ.sin.sin_port = htons(EthDebugPort());

            ethWrtQ.lastIPaddr = thisIPaddr;
        }

        /*
         * Blast it out.
         */
        if (ethWrtQ.count > 0)
        {
            /*
             * Dequeue next entry
             */
            if (ethWrtQ.entry == NULL)
            {
                ethWrtQ.entry = DequeueUDPMsg();
                sendCount = 0;
            }

            /*
             * Try to send it (retry 2 times max).
             * NOTE: The possibility exists that the "entry" pointer could get
             * the rug pulled out from under it.  The pointer will still be a
             * valid pointer (so to speak) but will not point at a valid
             * message to send anymore.  This could happen if we are getting
             * messages queued faster than we can send them.  No real harm
             * will come of this other than sending garbage data, since the
             * tcp sendto() routine will only send a maximum of 64K anyway, if
             * the length field gets screwed up that badly.
             */
            if (ethWrtQ.entry && sendCount++ < 3)
            {
                rc = sendto(ethWrtQ.sockfd,
                            (UINT8 *)&ethWrtQ.entry->sequence,
                            ethWrtQ.entry->length - sizeof(UINT16),
                            MSG_DONTWAIT,
                            (struct sockaddr *)&ethWrtQ.sin, sizeof(ethWrtQ.sin));

                /*
                 * Check return code from send
                 */
                if (rc == PI_SOCKET_ERROR)
                {
                    socketError = errno;
                    switch (socketError)
                    {
                        case TM_EWOULDBLOCK:
                            break;

                        default:
                            Close(ethWrtQ.sockfd);
                            ethWrtQ.sockOpen = FALSE;
                            break;
                    }
                }

                /*
                 * Successful send
                 */
                else if (ethWrtQ.entry)
                {
                    ethWrtQ.entry = NULL;
                    ethWrtQ.sent++;
                }
            }
            /*
             * Failed to send
             */
            else
            {
                ethWrtQ.tossed++;
                ethWrtQ.entry = NULL;
            }
        }

        /*
         * Should never happen
         */
        if (ethWrtQ.count < 0)
        {
            ethWrtQ.count = 0;
            ethWrtQ.underRun++;
        }

        /*
         * exchange now or if no more work, put the task to sleep
         */
        if (ethWrtQ.count == 0)
        {
            TaskSetState(ethWrtQ.pPcb, PCB_NOT_READY);
            /* fall through to the TaskSwitch() */
        }
        else if (++numSentBeforeExchange >= NUM_TO_SEND_PER_EXCHANGE)
        {
            /* fall through to the TaskSwitch() */
        }
        else
        {
            continue;
        }

        TaskSwitch();
        numSentBeforeExchange = 0;
    }

    /*
     * If we get here, it means that we are shutting down
     * this task, so close the socket.
     */
    if (ethWrtQ.sockOpen == TRUE)
    {
        Close(ethWrtQ.sockfd);
    }

    /*
     * Clear out the ETH_WRITER_QUEUE control block.
     */
    memset(&ethWrtQ, 0, sizeof(ethWrtQ));
}



#ifndef NDEBUG

/**********************************************************************
*                                                                     *
*  Name:        _assert()                                             *
*                                                                     *
*  Description: Called by the assert() macro.                         *
*                                                                     *
*  Input:       expr - the test expression                             *
*               val - the expected val (can be an address or int)     *
*               file - the filename of the assert macro               *
*               line - the line number of the assert macro            *
*                                                                     *
*  Returns:     void                                                  *
*                                                                     *
**********************************************************************/
void _assert(const char *expr, const INT32 val, const char *file, const INT32 line)
{
#ifndef NONFATAL_ASSERT
    UINT8       msg[] = "FATAL!";
#else   /* NONFATAL_ASSERT */
    UINT8       msg[] = "NONFATAL.";
#endif  /* NONFATAL_ASSERT */

    LogMessage(LOG_TYPE_DEBUG, "ASSERT FAILED: %s (0x%08X) at %s:%d. %s",
               expr, val, file, line, msg);

#ifndef NONFATAL_ASSERT
    DeadLoop(EVENT_ASSERT_FAILED, TRUE);
#endif  /* NONFATAL_ASSERT */
}
#endif /* NDEBUG */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
