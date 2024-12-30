/* $Id: logging.c 143845 2010-07-07 20:51:58Z mdr $ */
/**
******************************************************************************
**
**  @file       logging.c
**
**  @brief      Logging implementation
**
**  Copyright (c) 2001-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#include "logging.h"

#include "AsyncEventHandler.h"
#include "AsyncClient.h"
#include "ccb_flash.h"
#include "crc32.h"
#include "debug_files.h"
#include "LOG_Defs.h"
#include "kernel.h"
#include "led.h"
#include "logdef.h"
#include "logview.h"
#include "mutex.h"
#include "nvram.h"
#include "nvram_structure.h"
#include "quorum.h"
#include "rtc.h"
#include "serial_num.h"
#include "stdarg.h"
#include "ccb_flash.h"
#include "rtc.h"
#include "quorum_utils.h"
#include "XIO_Std.h"


/*****************************************************************************
** Private defines
*****************************************************************************/

/* #define CLEAR_LOGS_ON_CCB_NVRAM_CLEAR */
#define LOGS_CORRUPT_RETRY_COUNT    10

/*
 * Log sector definitions
 * The first 32 bit word of the flash sector indicates whether the information
 * in the sector is valid. Log entries start immediately after that valid word.
 */
#define LS_VALID                    0xFEDC0008
#define LS_ERASED                   0xFFFFFFFF

#define NotReadOrDeleted(logPtr)  \
    ((logPtr->le.status.flags & (LE_DELETED|LE_ACKED)) == (LE_DELETED|LE_ACKED))
#define GetUnAckAccumCount()    cLogInfo.unAckErrorCount

/**
******************************************************************************
** Private structure definitions
******************************************************************************
**/
typedef struct
{
    UINT32  validWord;
    LOG_HDR header;
} LOG_SECTOR;


/* Log Information structure */

typedef struct
{
    UINT32  startAddress;           /* Flash Address where logs start       */
    UINT32  startSector;            /* Starting flash sector                */
    LOG_SECTOR  *currPtr;           /* Pointer to current log sector        */
    UINT32  sectorSize;             /* Log Sector size in bytes             */
    UINT32  sectorOffset;           /* Sector offset of current entry       */
    UINT16  rsvd1;                  /* RESERVED                             */
    UINT16  unAckErrorCount;        /* Unacknowledged Error Counter         */
    UINT16  unAckWarningCount;      /* Unacknowledged Warning Counter       */
    UINT16  unAckInfoCount;         /* Unacknowledged Info Counter          */
    UINT8   numSectors;             /* Number of logging flash sectors      */
    UINT8   startSectNum;           /* sector number of start sector        */
    UINT8   currSectNum;            /* sector number of current sector      */
    UINT8   rsvd2;                  /* RESERVED                             */
} LOG_INFO_CB;

/* Log ptr data (for display) */

typedef struct
{
    char        *sectPtr;
    UINT32      sectorOffset;
    LOG_HDR     *logPtr;
    UINT8       sectorNum;
    LOG_INFO_CB *logInfoPtr;
} LOG_PTR_DATA;

/* Log message, used as filler */

typedef struct LOG_MSG
{
    LOG_HDR     header;
    UINT8       data[1024];
}   LOG_MSG;


/**
******************************************************************************
** Private variables
******************************************************************************
**/
/* Log Event Mutex */
static MUTEX    logMutex;

/* Log Information structure */
static LOG_INFO_CB cLogInfo;
static LOG_INFO_CB dLogInfo;

static UINT16   logState;
static bool     newClient;

#ifdef ENABLE_NG_LED
  UINT8 ng_led_state = 0;
#endif  /* ENABLE_NG_LED */

/**
******************************************************************************
** Public variables - externed in the header file
******************************************************************************
**/

/**
******************************************************************************
** Public variables - not externed in any header file
******************************************************************************
**/
time_t      skip_login_logout_msg_until = 0;

/**
******************************************************************************
** Private function prototypes
******************************************************************************
**/
static UINT8 ValidateLogEntry(LOG_INFO_CB *, LOG_HDR *);    /* Check for full log entry */
static UINT32 InitLogPtrs(LOG_INFO_CB *);       /* Initializes log pointer to next log entry */
static void EraseLogSector(UINT8 sectorNum, UINT32 offset); /* Erase log sector */
static void MarkLogSectorValid(UINT8 sectorNum, LOG_INFO_CB *);/* Mark the sector as valid */
static UINT8 ValidLogSectors(LOG_INFO_CB *); /* Do valid log sectors exist */
static UINT32 NewerLogTime(UINT32 time1, UINT32 time2);
static void InitEventCounters(void);
static void ClearEventCounters(void);
static void IncrEventCounter(LOG_HDR *);
static void DecrEventCounter(LOG_HDR *);
static SEQ32 GetNextMasterSequence(void);
static LOG_HDR *GetLastLogEntry(LOG_PTR_DATA *, LOG_INFO_CB *);
static LOG_HDR *GetNextLogEntry(LOG_PTR_DATA *);
static LOG_HDR *FindLogEntryBySequence(SEQ32 seq, LOG_INFO_CB *);
static UINT32 SetCurrLogEntry(LOG_HDR *, LOG_PTR_DATA *);
static void GetLogPtrList(LOG_HDR **, LOG_HDR *, LOG_INFO_CB *, int count);
static void PrintLogEntry(LOG_HDR *);


/**
******************************************************************************
** Code Start
******************************************************************************
**/

/**
******************************************************************************
**
**  @brief  Initialize logMutex
**
**  @param  none
**
**  @return none
**
******************************************************************************
**/
void InitLogMutex(void)
{
    InitMutex(&logMutex);
}

/**
******************************************************************************
**
**  @brief  Check if log message acknowledged
**
**  @param  le - Pointer to log entry
**
**  @return TRUE if log entry acknowledged
**
******************************************************************************
**/
int LogIsAcked(LOG_HDR *le)
{
    return (le->le.status.flags & LE_ACKED) == 0;
}


/**
******************************************************************************
**
**  @brief  Erases all flash log sectors
**
**  @param  none
**
**  @return none
**
******************************************************************************
**/
static void EraseEventLog(void)
{
    UINT8 i;

    /* Erase each logging sector */

    for (i = 0; i < cLogInfo.numSectors; ++i)
    {
        EraseLogSector(i, cLogInfo.startSector);
    }

    for (i = 0; i < dLogInfo.numSectors; ++i)
    {
        EraseLogSector(i, dLogInfo.startSector);
    }

    ClearEventCounters();   /* Clear the unacknowledeged event counters */
}


/**
******************************************************************************
**
**  @brief  Initialize log information structure
**
**  Initializes the log information structure and sets the log
**  pointer to the last log entry. If no valid log sectors exists
**  (i.e. this is the first time the logs have been created), the
**  log sectors will be erased and the first sector marked as
**  valid.
**  Flash operations can take a relatively long period of time and
**  thus these functions may perform task exchanges. To avoid
**  conflicts with initializing the logs (when the flash sectors
**  need to be written) and receiving new log events (through a
**  process exchange), initialization of flash logs can be delayed
**  until the first log event is received. This is accomplished
**  thru the initType input. In these cases the log structure will
**  be initialized, but the flash will not be written until the
**  function is invoked again (with the FIRST_LOG_EVENT input).
**  If the user is sure not conflict will exist with other processes
**  trying to create log events, the _NOW inputs can be used.
**  The CLEAR_ inputs force the logs back to an initial (erased)
**  state.
**
**  @param  initType
**                 - INIT_LOGS_NOW - initialize log pointers now, including
**                                   flash writes (if required)
**                 - INIT_ON_FIRST_EVENT - initialize log pointers now, but
**                                   delay flash writes (if required) until
**                                   the first log event
**                 - CLEAR_LOGS_NOW - clear (erase) flash logs and initialize
**                                   log pointers now.
**                 - CLEAR_LOGS_ON_FIRST_EVENT - delay clearing (erase) of flash
**                                   logs and initialization log pointers
**                                   until first log event.
**                 - FIRST_LOG_EVENT - performed delayed log initialization as
**                                   saved in the logState variable
**
**  @return none
**
******************************************************************************
**/
void InitLogs(UINT16 initType)
{
    UINT32  rc;
    UINT32  index1;

#ifdef CLEAR_LOGS_ON_CCB_NVRAM_CLEAR
    UINT32  crc = 0;
#endif  /* CLEAR_LOGS_ON_CCB_NVRAM_CLEAR */

    /* Load up information about the logs */

    cLogInfo.startAddress = (UINT32)FLASH_LOG_START_ADDRESS;
    cLogInfo.sectorSize = LS_SECTOR_SIZE;
    cLogInfo.numSectors = LS_NUM_SECTORS;
    cLogInfo.startSector = FLASH_LOG_SECTOR_OFFSET;

    dLogInfo.startAddress = (UINT32)FLASH_DEBUG_LOG_START_ADDRESS;
    dLogInfo.sectorSize = LS_SECTOR_SIZE_DEBUG;
    dLogInfo.numSectors = LS_NUM_SECTORS_DEBUG;
    dLogInfo.startSector = FLASH_DEBUG_LOG_SECTOR_OFFSET;

    dprintf(DPRINTF_DEFAULT, "cLogInfo.startAddress: 0x%x\n", cLogInfo.startAddress);
    dprintf(DPRINTF_DEFAULT, "dLogInfo.startAddress: 0x%x\n", dLogInfo.startAddress);

    /*
     * Set the logging state variable, based on the requested initialization.
     * If An erase will need to be performed on the logs, set the PENDING
     * condition. If this not the first log event (and an erase is not
     * PENDING), mark the logs as valid, since all initialization will be
     * completed on this call.
     */

    /* Check to see if we should initialize the Logs */

#ifdef CLEAR_LOGS_ON_CCB_NVRAM_CLEAR
    if ((NVRAMData.cntlSetup.configFlags & CF_INIT_LOGS_MASK) == 0)
    {
        initType = CLEAR_LOGS_NOW;      /* Clear the logs now */

        /* Set the init logs bit so we will not initialize it again. */
        NVRAMData.cntlSetup.configFlags |= CF_INIT_LOGS_MASK;

        /* Calculate the new crc. */
        NVRAMData.cntlSetup.crc = CRC32((UINT8 *)&NVRAMData.cntlSetup,
                    sizeof(CONTROLLER_SETUP) - sizeof(unsigned long));
    }
#endif  /* CLEAR_LOGS_ON_CCB_NVRAM_CLEAR */

    if (initType == CLEAR_ON_FIRST_EVENT ||
        ((!ValidLogSectors(&cLogInfo) || !ValidLogSectors(&dLogInfo)) &&
         initType == INIT_ON_FIRST_EVENT))
    {
        logState = LOG_ERASE_PENDING;
    }
    else if (initType != FIRST_LOG_EVENT)
    {
        logState = LOG_VALID;
    }

    /*
     * Look to see if any valid log sectors exist. If not, erase the log area
     * and mark the first sector as valid.
     */
    if (((!ValidLogSectors(&cLogInfo) || !ValidLogSectors(&dLogInfo)) &&
        initType == INIT_LOGS_NOW) ||
        initType == CLEAR_LOGS_NOW  ||
        (initType == FIRST_LOG_EVENT && logState == LOG_ERASE_PENDING))
    {
        EraseEventLog();                    /* Erase entire log */
        /* Mark first sector valid */
        MarkLogSectorValid(0, &cLogInfo);
        MarkLogSectorValid(0, &dLogInfo);
        logState = LOG_VALID;
    }

    /*
     * Initialize the log pointers, finding the first valid entry,
     * the last entry, and the next entry.
     * Initialize the unacknowledged event counters.
     */
    if (logState != LOG_ERASE_PENDING)
    {
        /*
         * Intitialize the Debug logs.  If InitLogPtrs returns ERROR, it has
         * detected and corrected a corrupt sector in the flash where the logs
         * are stored.  We must call InitLogPtrs again to correctly initialize
         * the log pointers.  We will retry this LOGS_CORRUPT_RETRY_COUNT times.
         */
        for (index1 = 0; index1 < LOGS_CORRUPT_RETRY_COUNT; ++index1)
        {
            rc = InitLogPtrs(&dLogInfo);
            if (rc != ERROR)
            {
                break;
            }
            dprintf(DPRINTF_DEFAULT,
                    "%s: Debug Log Flash was Corrupted - Trying Again\n",
                    __func__);
        }

        /*
         * If we have an ERROR here we are in trouble.  For some reason we
         * could not correct the corrupt sector in the flash.  This could
         * be caused by the inability to program the flash device.  For now
         * we will carry on.
         */
        if (rc == ERROR)
        {
            /*
             * For now we will not take any action on a failure to initialize
             * the logs.  If something needs to be done in the future if we hit
             * this case, this is where it needs to happen.
             */
            dprintf(DPRINTF_DEFAULT, "%s: Debug Log Flash Corrupted!\n",
                    __func__);
        }

        /*
         * Intitialize the Customer logs.  If InitLogPtrs returns ERROR, it has
         * detected and corrected a corrupt sector in the flash where the logs
         * are stored.  We must call InitLogPtrs again to correctly initialize
         * the log pointers.  We will retry this LOGS_CORRUPT_RETRY_COUNT times.
         */
        for (index1 = 0; index1 < LOGS_CORRUPT_RETRY_COUNT; ++index1)
        {
            rc = InitLogPtrs(&cLogInfo);
            if (rc != ERROR)
            {
                break;
            }
            dprintf(DPRINTF_DEFAULT,
                    "%s: Customer Log Flash was Corrupted - Trying Again\n",
                    __func__);
        }

        /*
         * If we have an ERROR here we are in trouble.  For some reason we
         * could not correct the corrupt sector in the flash.  This could
         * be caused by the inability to program the flash device.  For now
         * we will carry on.
         */
        if (rc == ERROR)
        {
            /*
             * For now we will not take any action on a failure to initialize the
             * logs.  If something needs to be done in the future if we hit
             * this case, this is where it needs to happen.
             */
            dprintf(DPRINTF_DEFAULT, "%s: Customer Log Flash Corrupted!\n",
                    __func__);
        }

        InitEventCounters();
    }
}


/**
******************************************************************************
**
**  @brief  Logs a text message as info, error, warning, or debug.
**
**  @param  type - type of message:
**                  LOG_TYPE_INFO        0
**                  LOG_TYPE_WARNING     1
**                  LOG_TYPE_ERROR       2
**                  LOG_TYPE_DEBUG       3
**  @param  fmt - Format string
**  @param  Additional parameters as dictated by format string
**
**  @return none
**
******************************************************************************
**/
void LogMessage(UINT32 msgType, const char *fmt, ...)
{
    char    buf[MAX_TEXT_MSG_LEN * 2];
    char    *bufP = buf;
    va_list args;

    memset(buf, 0, sizeof(buf));    /* Clear out the message buffer */

    va_start(args, fmt);
    bufP += vsprintf(bufP, fmt, args);
    va_end(args);

    /* Get rid of any trailing newlines (and whitespace) */

    --bufP;     /* Back up to one before the string terminator */
    while (bufP > buf &&
        (*bufP == '\n' || *bufP == '\r' || *bufP == ' ' || *bufP == '\t'))
    {
        bufP--;
    }

    /* Move back to the string terminator and re-terminate it */

    ++bufP;
    *bufP = 0;

    /*
     * 'buf' is twice the size we actually need.  Lop off the input string
     * if it blows over the MAX length.  Set the last character to '+' to
     * indicate that there *was* more...
     */
    if ((bufP - buf) > MAX_TEXT_MSG_LEN)
    {
        buf[MAX_TEXT_MSG_LEN - 1] = '+';
    }

    LogTextMessage(msgType, buf);
}


/**
******************************************************************************
**
**  @brief  Logs a text message as info, error, warning, or debug.
**
**  @param  type - type of message:
**                  LOG_TYPE_INFO        0
**                  LOG_TYPE_WARNING     1
**                  LOG_TYPE_ERROR       2
**                  LOG_TYPE_DEBUG       3
**  @param  msg  -  String message to log.
**
**  @return GOOD or ERROR.
**
******************************************************************************
**/
INT32 LogTextMessage(UINT32 msgType, char *msg)
{
    char    *tmpMsg     = msg;
    int     offset      = 0;
    LOG_MRP logEvent;

    if (!msg)       /* If the data is null, flag it as an error */
    {
        return ERROR;
    }

    /*
     * Set the log event type.  If the event type
     * is unknown flag it as an error.
     */
    switch (msgType)
    {
    case LOG_TYPE_INFO:
        if (msg[0] == '!' && msg[1] == '$')
        {
           offset = 2;
           skip_login_logout_msg_until = time(NULL) + 1800;
        }
        else if (msg[0] == '$' && msg[1] == '!')
        {
           offset = 2;
           skip_login_logout_msg_until = 0;
        }
        logEvent.mleEvent = LOG_LOG_TEXT_MESSAGE_INFO;
        break;

    case LOG_TYPE_WARNING:
        logEvent.mleEvent = LOG_LOG_TEXT_MESSAGE_WARNING;
        break;

    case LOG_TYPE_FATAL:
    case LOG_TYPE_ERROR:
        logEvent.mleEvent = LOG_LOG_TEXT_MESSAGE_ERROR;
        break;

    case LOG_TYPE_DEBUG:
        logEvent.mleEvent = LOG_LOG_TEXT_MESSAGE_DEBUG;
        break;

    default:
        return ERROR;
    }

    /*
     * Set the log event length, no greater than our MAX_TEXT_MSG_LEN
     * defined in PacketInterface.h.
     */
    logEvent.mleLength = 0;

    while (*tmpMsg && logEvent.mleLength < MAX_TEXT_MSG_LEN)
    {
        ++logEvent.mleLength;
        ++tmpMsg;
    }

    if (logEvent.mleLength < MAX_TEXT_MSG_LEN)
    {
        ++logEvent.mleLength;
    }

    /* Copy the string to the log event */
    memcpy((UINT8 *)logEvent.mleData, &msg[offset], logEvent.mleLength);

    /*
     * Ensure we place the null terminator in the last
     * character of our string just in case it was
     * truncated above at MAX_TEXT_MSG_LEN.
     */
    ((UINT8 *)logEvent.mleData)[MAX_TEXT_MSG_LEN - 1] = '\0';

    LogEvent(&logEvent);

    return GOOD;
}


/**
******************************************************************************
**
**  @brief  Enter log event into flash
**
**  Moves the log data from the MRP to the flash. Adds a log entry
**  header with a timestamp. Checkpoints the writing of the log.
**  Advances next log entry pointer.
**
**  @param  mrp - mrp pointer
**  @param  buf - Pointer to buffer to hold log message
**
**  @return none
**
**  @attention  This function is only used in very particular cases, such
**              as from the normal LogEvent function and from test functions.
**
******************************************************************************
**/
static void _log_event(LOG_MRP *mrp, UINT8 *buf)
{
    CCB_FLASH   *flashptr;
    CCB_FLASH   *eventptr;
    SEQ32       masterSequence;
    SEQ32       sequence;
    LOG_INFO_CB *li;
    LOG_HDR     header =
            {
                .eventCode = (UINT16)mrp->mleEvent,
                /* Round up the length to a multiple of the flash word size */
                .length = ((mrp->mleLength + sizeof(CCB_FLASH) - 1) /
                        sizeof(CCB_FLASH)) * sizeof(CCB_FLASH),
                .le =
                    {
                        .status =
                            {
                                .flags = ~(LE_STARTED | LE_COMPLETE),
                                .reserved = ~0
                            }
                    }
            };

    memset(buf, 0, header.length + sizeof(LOG_HDR));

    header.timeStamp = RTC_GetLongTimeStamp();  /* Fill in in the timestamp */

    eventptr = (CCB_FLASH *)buf;

    /*
     * If the flash logs are not in the VALID state, finish the initialization
     * now.
     */
    if (logState != LOG_VALID)
    {
        InitLogs(FIRST_LOG_EVENT);
    }

    /* Retrieve the correct LOG_INFO */

    if (mrp && GetEventType(mrp->mleEvent) == LOG_TYPE_DEBUG)
    {
        li = &dLogInfo;
    }
    else
    {
        li = &cLogInfo;
    }

    masterSequence = GetNextMasterSequence();

    /*
     * Test to see if the current log entry is erased, if so, this is first
     * entry to be written in this sector. Otherwise, bump the offset past
     * the current log entry.
     */
    if (li->sectorOffset == (sizeof(LOG_SECTOR) - sizeof(LOG_HDR)) &&
        ((LOG_SECTOR *)li->currPtr)->header.le.statusWord == LE_ERASED)
    {
        /*
         * The offset is correct. This must be the first event to be written,
         * so set the sequence number to 0.
         */
        sequence = 0;
    }
    else
    {
        LOG_HDR *lp;

        /* Set the pointer to the last written entry */
        lp = (LOG_HDR *)((char *)li->currPtr + li->sectorOffset);

        /* Get the previous sequence number and add one to get the next */
        sequence = lp->sequence + 1;

        /* Bump offset to location where new entry will go */
        li->sectorOffset += lp->length + sizeof(LOG_HDR);
    }

    /*
     * Look to see if the log entry will fit in the current sector. The amount
     * of data to be written is the sum of the header length and length in the
     * data field.
     * If not, move to the next sector, erase it, mark it as valid, and
     * then write the new entry.
     */
    if ((li->sectorOffset + mrp->mleLength + sizeof(LOG_HDR)) > li->sectorSize)
    {
        /*
         * Proceed to the next sector, wrapping if we are at the end of the log
         * sectors. Reset the sector pointer and the sector offset.
         */
        li->currSectNum = (li->currSectNum + 1) % li->numSectors;
        li->currPtr = (LOG_SECTOR *)(li->startAddress +
                            (li->currSectNum * li->sectorSize));
        li->sectorOffset = sizeof(LOG_SECTOR) - sizeof(LOG_HDR);

        /*
         * If the current log sector wrapped around to the starting log sector,
         * bump the starting log sector ahead
         */
        if (li->currSectNum == li->startSectNum)
        {
            li->startSectNum = (li->startSectNum + 1) % li->numSectors;
        }

        /* Erase new sector            */
        EraseLogSector(li->currSectNum, li->startSector);

        /* Mark new sector valid       */
        MarkLogSectorValid(li->currSectNum, li);

        /*
         * Since unacknowledged events may have just been erased,
         * reinitialize the unacknowledged event counters.
         */
        InitEventCounters();
    }

    /*
     * Continue building the log header,
     * 5. Initialize the sequence numbers
     */

    header.masterSequence = masterSequence;
    header.sequence = sequence;

    /* Copy the event header and event data */
    memcpy(eventptr, &header, sizeof(header));
    memcpy(((char *)eventptr) + sizeof(header), &mrp->mleData, mrp->mleLength);

    /*
     * Program the header and data with the log entry status flags
     * started and complete asserted.
     */
    flashptr = (CCB_FLASH *)((char *)li->currPtr + li->sectorOffset);

    CCBFlashProgramData(flashptr, eventptr,
                  (sizeof(LOG_HDR) + header.length) / sizeof(CCB_FLASH));

    /* Increment the unacknowledged event counter due to the new log event */
    if (GetEventType(mrp->mleEvent) != LOG_TYPE_DEBUG)
    {
        IncrEventCounter((LOG_HDR *)eventptr);
    }
}


/**
******************************************************************************
**
**  @brief  Process log message
**
**  Puts the log data from the MRP into the flash and send the message
**  to registered clients. Also send the message to the Linux logs.
**  This function also uses the logMutex to protect the flash from
**  corruption.
**
**  @param  ilt  - ilt pointer
**  @param  mrp  - mrp pointer
**
**  @return none
**
******************************************************************************
**/
void LogEvent(LOG_MRP *mrp)
{
    union
    {
        LOG_HDR hdr;
        UINT8   logbuf[MAX_LOG_MESSAGE_SIZE + sizeof(LOG_HDR)];
    }       buf;

    /*
     * Since there is global data associated with pointers into the logs,
     * the flash functions within can perform task exchanges, and
     * the fact that this function can be invoked by more than task at a time,
     * this function is protected with a Mutex.
     */
    LockMutex(&logMutex, MUTEX_WAIT);

    _log_event(mrp, buf.logbuf);    /* Call internal logging function */

    UnlockMutex(&logMutex);     /* We are done, free the log Mutex */

/* ASYNCEVENTS */
    /*
     * Create the binary event packet and queue it to be sent
     * to all registered clients.
     */
    if (GetEventType(mrp->mleEvent) != LOG_TYPE_DEBUG)
    {
        PI_BINARY_LOG_EVENT *pbe;

        pbe = MallocWC(sizeof(*pbe) + sizeof(buf.hdr) + mrp->mleLength);
        pbe->length = sizeof(buf.hdr) + buf.hdr.length + sizeof(pbe->eventType);
        pbe->eventType = GetEventType(mrp->mleEvent);
        memcpy(pbe->message, buf.logbuf, sizeof(buf.hdr) + mrp->mleLength);

        /* Create log event packet and send it to registered clients */
        EnqueuePIAsyncNotification(pbe);
    }
/* ASYNCEVENTS */

    /* Print the log event message to the serial debug port */
    PrintLogEntry(&buf.hdr);
}


/**
******************************************************************************
**
**  @brief  Return the first valid log entry (oldest entry in system).
**
**  @param  logEntry - pointer to LOG_PTR_DATA structure
**  @param  li - pointer to the logs location in Flash and current position
**
**  @return logPtr - pointer to log entry, NULL if no valid entry
**
******************************************************************************
**/
static LOG_HDR *GetFirstLogEntry(LOG_PTR_DATA *logEntry, LOG_INFO_CB *li)
{
    LOG_HDR *logPtr;

    /*
     * The first valid log entry (the oldest log entry in the system) is
     * located at the base address of the flash logs, plus the offset of
     * the starting sector times the size of a log sector. The offset of that
     * entry is located after the sector header information.
     */
    logEntry->sectPtr = (char *)(li->startAddress +
                    (li->startSectNum * li->sectorSize));
    logEntry->sectorOffset = sizeof(LOG_SECTOR) - sizeof(LOG_HDR);
    logEntry->sectorNum = li->startSectNum;
    logEntry->logInfoPtr = li;

    /* Convert the sector and offset into a log pointer */
    logPtr = (LOG_HDR *)(logEntry->sectPtr + logEntry->sectorOffset);

    /*
     * Determine if the log entry stored at the pointer location is valid.
     * If not, return a NULL indicator.
     */
    if (!ValidLogEntry(logPtr))
    {
        logPtr = NULL;
    }

    logEntry->logPtr = logPtr;

    return logPtr;
}


/**
******************************************************************************
**
**  @brief  Return the last valid log entry.
**
**  @param  logEntry - pointer to LOG_PTR_DATA structure
**  @param  logInfo - information for the logs location in Flash and
**                        current location.
**
**  @return logPtr - pointer to log entry, NULL if no valid entry
**
******************************************************************************
**/
static LOG_HDR *GetLastLogEntry(LOG_PTR_DATA *logEntry, LOG_INFO_CB *logInfo)
{
    LOG_HDR *logPtr;

    /*
     * The last valid log entry should be in the currently active log sector
     * at the current sector offset.
     */
    logPtr = (LOG_HDR *)((char *)logInfo->currPtr + logInfo->sectorOffset);

    /* Save away the log entry pointer information */
    logEntry->sectPtr = (char *)logInfo->currPtr;
    logEntry->sectorOffset = logInfo->sectorOffset;
    logEntry->sectorNum = logInfo->currSectNum;
    logEntry->logInfoPtr = logInfo;

    /*
     * Determine if the log entry stored at the pointer location is valid.
     * If not, return a NULL indicator.
     */
    if (!ValidLogEntry(logPtr))
    {
        logPtr = NULL;
    }

    logEntry->logPtr = logPtr;

    return logPtr;
}


/**
******************************************************************************
**
**  @brief  Return the next valid log entry.
**
**  @param  Pointer to LOG_PTR_DATA structure
**
**  @return logPtr - pointer to log entry, NULL if no valid entry
**
******************************************************************************
**/
static LOG_HDR *GetNextLogEntry(LOG_PTR_DATA *logEntry)
{
    LOG_HDR *logPtr = (LOG_HDR *)(logEntry->sectPtr + logEntry->sectorOffset);
    UINT32  sectorOffset;

    /*
     * Increment the log pointer to the next entry. Determine if the
     * entry is valid (not erased). If it is , return the pointer and advance
     * the log entry structure. If the advanced pointer points to an erased
     * location, look to see if this is the last valid sector. If this is
     * the last valid sector, return a NULL pointer, otherwise advance to the
     * next log sector and look for a valid entry there.
     */

    sectorOffset = logEntry->sectorOffset + logPtr->length + sizeof(LOG_HDR);
    logPtr = (LOG_HDR *)(logEntry->sectPtr + sectorOffset);

    /*
     * Look to see if we have advanced past the end of the current sector or if
     * no more entries exist in this sector.
     */
    if (sectorOffset >= logEntry->logInfoPtr->sectorSize ||
        logPtr->le.statusWord == LE_ERASED)
    {
        /*
         * If we are in the sector where the newest entries are added, then we
         * are at the end of the logs. Otherwise move to the next sector.
         */
        if (logEntry->sectorNum == logEntry->logInfoPtr->currSectNum)
        {
            return logEntry->logPtr = NULL;
        }

        /*
         * Proceed to the next sector, wrapping if we are at the end of the log
         * sectors. Reset the sector pointer and the sector offset.
         */
        logEntry->sectorNum = (logEntry->sectorNum + 1) %
                                    logEntry->logInfoPtr->numSectors;
        logEntry->sectPtr = (char *)(logEntry->logInfoPtr->startAddress +
                    (logEntry->sectorNum * logEntry->logInfoPtr->sectorSize));
        logEntry->sectorOffset = sizeof(LOG_SECTOR) - sizeof(LOG_HDR);
    }
    else
    {
        /* Save away the changed log entry pointer information */

        logEntry->sectorOffset = sectorOffset;
    }

    /* Compute the new log pointer */

    logPtr = (LOG_HDR *)(logEntry->sectPtr + logEntry->sectorOffset);


    /*
     * Determine if the log entry stored at the pointer location is valid.
     * If not, return a NULL indicator.
     */
    if (ValidLogEntry(logPtr))
    {
        return logEntry->logPtr = logPtr;
    }

    return logEntry->logPtr = NULL;
}


/**
******************************************************************************
**
**  @brief  Set current log data information based on the passed log pointer
**
**  @param  Pointer to a log entry
**  @param  Pointer to LOG_PTR_DATA structure
**
**  @return GOOD - log entry set successfully, ERROR - invalid logPtr
**
******************************************************************************
**/
static UINT32 SetCurrLogEntry(LOG_HDR *logPtr, LOG_PTR_DATA *logEntry)
{
    int         i;

    /* Initialize the log pointer data to the start of the logs */
    logEntry->sectPtr = (char *)logEntry->logInfoPtr->startAddress;
    logEntry->sectorOffset = 0;
    logEntry->sectorNum = 0;
    logEntry->logPtr = logPtr;

    /*
     * If the passed log pointer does not indicate a valid entry,
     * return now with an error.
     */
    if (!ValidLogEntry(logPtr))
    {
        return ERROR;
    }

    /*
     * Loop through the log sectors to find where the passed log pointer is
     * located.
     */
    for (i = 0; i < logEntry->logInfoPtr->numSectors &&
        (char *)logPtr > ((char *)logEntry->sectPtr + logEntry->logInfoPtr->sectorSize);
                                                                    ++i)
    {
        /* Set a pointer to the next log sector */
        logEntry->sectPtr += logEntry->logInfoPtr->sectorSize;
        ++logEntry->sectorNum;
    }

    /* Set up the sector offset from the start of the sector located */
    logEntry->sectorOffset = (char *)logPtr - (char *)logEntry->sectPtr;

    return GOOD;
}


/**
******************************************************************************
**
**  @brief  Changes status to indicate the log entry has been read
**
**  @param  logPtr - pointer to log entry
**
******************************************************************************
**/
static void MarkLogEntryRead(LOG_HDR *logPtr)
{
    CCB_FLASH source = logPtr->le.statusWord & ~LE_ACKED;

    DecrEventCounter(logPtr);   /* Decrement the Unacknowledge event counter */

    /* Write the status field */
    CCBFlashProgramData((CCB_FLASH *)logPtr, (CCB_FLASH *)&source, 1);
}


/**
******************************************************************************
**
**  @brief  Determines the validity of a log entry
**
**  @param  logPtr  - pointer to the log in flash
**
**  @return TRUE    - Valid Log Entry, FALSE   - Corrupted Log Entry
**
******************************************************************************
**/
static UINT8 ValidateLogEntry(LOG_INFO_CB *logInfo, LOG_HDR *logPtr)
{
    UINT32  laddr = (UINT32)logPtr;

    /*
     * These are some fields that should not be all F's.  If any
     * of these fields meet this condition, this would indicate
     * a corrupted log entry (Entry that was not entirely written
     * to flash).  Return FALSE (Corrupted Log Entry).
     */
    if ((logPtr->length & (sizeof(CCB_FLASH) - 1)) != 0 ||
        logPtr->eventCode == 0xFFFF || logPtr->timeStamp == 0xFFFFFFFF)
    {
        return FALSE;
    }

    LOG_HDR *next = (LOG_HDR *)(laddr + logPtr->length + sizeof(LOG_HDR));

    /*
     * Check that this entry points to another valid log message.
     * First check that we are not peeking beyond the end of the sector.
     */
    if ((UINT32)next - (UINT32)logInfo->currPtr < logInfo->sectorSize)
    {
        /*
         * Check that this entry points to another valid log message.
         * If it does not, this also indicates a corrupted log
         * entry (Entry that was not entirely written to flash).
         */
        if (!ValidLogEntry(next))
        {
            /*
             * It is possible that the log entry could point to a log entry
             * that has yet to be written.  This would be the case if this
             * particular log message was the last entry written.  If the next
             * entry is invalid and not LE_ERASED, return FALSE (Corrupted
             * Log Entry).
             */
            if (next->le.statusWord != LE_ERASED)
            {
                return FALSE;
            }
        }
    }

    return TRUE;
}


/**
******************************************************************************
**
**  @brief  Initialize the log pointers
**
**  Initialize the log pointers, finding the first valid entry,
**  the last entry, and the next entry.
**
**  @param  li - pointer to the location in Flash and current location.
**
**  @return GOOD    - InitLogPtrs was successful
**  @return ERROR   - InitLogPtrs found a corrupt log entry and attempted
**                    to fix the sector.  InitLogPtrs must be called again
**                    to initialize the log pointers for logInfo.
**
**  @attention  Log Information structure should be set-up and the logs
**              initialized before calling. Also the log mutex must be held.
**
******************************************************************************
**/
static UINT32 InitLogPtrs(LOG_INFO_CB *li)
{
    LOG_SECTOR  *oldPtr;
    LOG_SECTOR  *newPtr;
    UINT8   newSector;
    UINT8   oldSector;
    UINT8   i;
    UINT8   firstSectorFound    = FALSE;

    /*
     * Scan through all of the log sectors, noting the sector with the oldest
     * and the newest log entries. The oldest sector will hold the start of the
     * the logs and the newest sector will hold the last entry made.
     *
     * Since logs entries can only be erased
     * a sector at a time, the first entry must be at the start of a sector
     * (following the validation word).
     */
    oldPtr = newPtr = (LOG_SECTOR *)li->startAddress;
    newSector = oldSector = 0;

    for (i = 0; i < li->numSectors; ++i)
    {
        LOG_SECTOR  *ls;

        /* Set a pointer to the next log sector */
        ls = (LOG_SECTOR *)(li->startAddress + (i * li->sectorSize));

        /* Determine if the sector we are looking at has a valid log entry */

        if (ls->validWord != LS_VALID ||
            (ls->header.le.status.flags & (LE_STARTED | LE_COMPLETE)) != 0)
        {
            continue;   /* Skip sector if not valid */
        }

        dprintf(DPRINTF_DEFAULT, "%s: Sect %d ts=%08X seq=%08X mseq=%08X\n",
                __func__, i, ls->header.timeStamp, ls->header.sequence,
                ls->header.masterSequence);

        /*
         * If this is the first valid sector we found,
         * then mark this sector as having both
         * the oldest and newest log entries.
         */
        if (firstSectorFound == FALSE)
        {
            oldPtr = newPtr = ls;
            oldSector = newSector = i;

            firstSectorFound = TRUE;
            continue;   /* Continue with next sector */
        }

        /*
         * Compare the time of the first entry in the log sectors and
         * keep the oldest
         */
        if (NewerLogTime(oldPtr->header.timeStamp, ls->header.timeStamp) ==
            oldPtr->header.timeStamp)
        {
            oldPtr = ls;
            oldSector = i;
        }

        /*
         * Compare the time of the first entry in the log sectors and
         * keep the newest
         */
        if (NewerLogTime(newPtr->header.timeStamp, ls->header.timeStamp) ==
            ls->header.timeStamp)
        {
            newPtr = ls;
            newSector = i;
        }
    }

    /*
     * Save the sector number of the sector with the oldest log entry
     * (the start of the log).
     */
    li->startSectNum = oldSector;

    /* Set pointer to the first entry in the newest log sector */
    li->currPtr = newPtr;
    li->currSectNum = newSector;

    /*
     * Find the offset of the last entry in the current log sector
     * Init pointer and offset to the first entry in the sector.
     */

    UINT32  offset = sizeof(LOG_SECTOR) - sizeof(LOG_HDR);
    UINT32  lastOffset = offset;
    char    *logPtr = (char *)li->currPtr;

    LOG_HDR *lp = (LOG_HDR *)(logPtr + offset);

    while (offset < li->sectorSize && lp->le.statusWord != LE_ERASED)
    {
        /*
         * Check that we have a complete log entry in order
         * to continue.
         */
        if (ValidateLogEntry(li, lp))
        {
            lastOffset = offset;    /* Save the previous offset */

            /* Point to the next entry (size of data plus the header) */
            offset += lp->length + sizeof(LOG_HDR);
            lp = (LOG_HDR *)(logPtr + offset);
            continue;
        }

        /* If not valid, we need to fix the current sector */

        dprintf(DPRINTF_DEFAULT, "%s: Invalid Log Entry Found!\n", __func__);
        dprintf(DPRINTF_DEFAULT, "-> statusWord: 0x%08X\n", lp->le.statusWord);
        dprintf(DPRINTF_DEFAULT, "-> masterSeq:  0x%08X\n", lp->masterSequence);
        dprintf(DPRINTF_DEFAULT, "-> sequence:   0x%08X\n", lp->sequence);
        dprintf(DPRINTF_DEFAULT, "-> length:     0x%04hX\n", lp->length);
        dprintf(DPRINTF_DEFAULT, "-> eventCode:  0x%04hX\n", lp->eventCode);
        dprintf(DPRINTF_DEFAULT, "-> timeStamp:  0x%08X\n", lp->timeStamp);

        /*
         * Fill the rest of the sector with all F's.  This will fill from
         * the corrupted log to the end of the sector.
         */
        memset(lp, 0xFF, li->sectorSize - offset);

        dprintf(DPRINTF_DEFAULT, "%s: Correcting corrupt Flash!\n",
                    __func__);

        /* Return the error to the caller and let them try again */
        return ERROR;
    }

    /*
     * Find the offset of the last entry in the current log sector by
     * finding the previous entry to the end of the sector
     */
    li->sectorOffset = lastOffset;

    return GOOD;
}


/**
******************************************************************************
**
**  @brief  Erases a single log sector
**
**  Sectors are numbered 0 to n-1, where n is the number of logging sectors.
**
**  @param  sectorNum - sector number to erase
**
******************************************************************************
**/
static void EraseLogSector(UINT8 sectorNum, UINT32 offset)
{
    /*
     * Compute the sector offset for the flash device and invoke the flash
     * erase function.
     */
    CCBFlashEraseSector(sectorNum + offset);
}


/**
******************************************************************************
**
**  @brief  Marks a log sector as being valid
**
**  Sets the "magic word" at the start of the logging sector to
**  indicate the sector contains valid data or is erased and ready
**  to accept log entries. Sectors are numbered 0 to n-1, where n
**  is the number of logging sectors.
**
**  @param  sectorNum - sector number to mark as valid
**  @param  li - Pointer to log info structure for the associated log
**
**  @return none
**
******************************************************************************
**/
static void MarkLogSectorValid(UINT8 sectorNum, LOG_INFO_CB *li)
{
    CCB_FLASH  *flashPtr;
    CCB_FLASH   source = LS_VALID;

    sectorNum += li->startSector;       /* Compute the sector offset */

    /* Get the starting address of the sector to mark as valid */

    if (CCBFlashGetAddressFromSector(sectorNum, &flashPtr) == GOOD)
    {
        /* Write one word at the start of the sector */

        CCBFlashProgramData(flashPtr, (CCB_FLASH *)&source, 1);
    }
}


/**
******************************************************************************
**
**  @brief  Checks to see if any valid log sectors exist
**
**  @param  li - Pointer to log information
**
**  @return 1 - if valid sectors exist, 0 - if no valid sectors
**
******************************************************************************
**/
static UINT8 ValidLogSectors(LOG_INFO_CB *li)
{
    int         i;

    for (i = 0; i < li->numSectors; ++i)
    {
        LOG_SECTOR  *currPtr;

        /* Set a pointer to the next log sector */

        currPtr = (LOG_SECTOR *)(li->startAddress + (i * li->sectorSize));

        /*
         * If sector contains valid log data, then compare the first entry
         * to that of the saved pointer, keeping the oldest.
         */
        if (currPtr->validWord == LS_VALID)
        {
            return 1;
        }
    }

    return 0;
}


/**
******************************************************************************
**
**  @brief  Test to see if the Log Entry is valid
**
**  @param  lp - pointer to a log entry
**
**  @return 1 - valid log entry, 0 - invalid log entry
**
******************************************************************************
**/
int ValidLogEntry(LOG_HDR *lp)
{
    UINT32  laddr = (UINT32)lp;

    /* Ensure the passed pointer is within the range of the log sectors */

    if ((laddr < FLASH_LOG_START_ADDRESS ||
        laddr >= FLASH_LOG_START_ADDRESS + (LS_SECTOR_SIZE * LS_NUM_SECTORS)) &&
        (laddr < FLASH_DEBUG_LOG_START_ADDRESS ||
         laddr >= FLASH_DEBUG_LOG_START_ADDRESS + (LS_SECTOR_SIZE_DEBUG * LS_NUM_SECTORS_DEBUG)))
    {
        return 0;
    }

    if ((lp->length & (sizeof(CCB_FLASH) - 1)) != 0 ||
            lp->length > MAX_LOG_MESSAGE_SIZE)
    {
        /* The following is a hack to allow an illegal length message to work. */
        if (lp->eventCode != LOG_ISCSI_GENERIC)
        {
            return 0;
        }
    }

    /*
     * If both the started and complete bits have been set (negative active),
     * then indicate the entry is valid.
     */
    if ((lp->le.status.flags & (LE_STARTED | LE_COMPLETE)) == 0)
    {
        return 1;
    }

    return 0;
}


/**
******************************************************************************
**
**  @brief  Checks two timestamps and returns the newest
**
**  @param  time1 - first timestamp
**  @param  time2 - second timestamp
**
**  @return newest timestamp
**
******************************************************************************
**/
static UINT32 NewerLogTime(UINT32 time1, UINT32 time2)
{
    if (time1 > time2)
    {
        return time1;
    }
    return time2;
}


/**
******************************************************************************
**
**  @brief  Initialize event counters
**
**  Initializes the counters for the number of unacknowledged ERRORs,
**  WARNINGs, and INFO messages contained in the logs. Walks through
**  the entire log.
**
**  @param  none
**
**  @return none
**
******************************************************************************
**/
static void InitEventCounters(void)
{
    LOG_HDR         *lp;
    LOG_PTR_DATA    lpd;

    ClearEventCounters();       /* Start with the event counters zeroed */

    /* Walk through the entire log from oldest entry to latest */
    for (lp = GetFirstLogEntry(&lpd, &cLogInfo); lp; lp = GetNextLogEntry(&lpd))
    {
        /*
         * Increment the event counter. This will check if it has
         * already been read or deleted.
         */
        IncrEventCounter(lp);
    }
}


/**
******************************************************************************
**
**  @brief  Update LED status based on event counts
**
**  @param  none
**
**  @return none
**
******************************************************************************
**/
static void CheckUnackLogStatus(void)
{
    LEDSetAttention(GetUnAckAccumCount() != 0);
}


/**
******************************************************************************
**
**  @brief  Clear event counters
**
**  Clear the counters for the number of unacknowledged ERRORs,
**  WARNINGs, and INFO messages contained in the logs.
**
**  @param  none
**
**  @return none
**
******************************************************************************
**/
static void ClearEventCounters(void)
{
    /* Set each counter to zero */

    cLogInfo.unAckErrorCount = 0;
    cLogInfo.unAckWarningCount = 0;
    cLogInfo.unAckInfoCount = 0;

    /* Check the status to see if we need to turn the Attention LED */

    CheckUnackLogStatus();
}


/**
******************************************************************************
**
**  @brief  Increment event counter
**
**  If the log event is not acknowledged or deleted, increment the
**  the appropriate log event counter
**
**  @param  logPtr - pointer to log entry
**
**  @return none
**
******************************************************************************
**/
static void IncrEventCounter(LOG_HDR *logPtr)
{
    UINT8       eventType;

    eventType = GetEventType(logPtr->eventCode);

    /*
     * If the entry has not been deleted or acknowledged, then increment
     * the appropriate counter
     */
    if (!NotReadOrDeleted(logPtr))
    {
        return;
    }

    switch (eventType)
    {
    case LOG_TYPE_ERROR:
        ++cLogInfo.unAckErrorCount;
        break;

    case LOG_TYPE_WARNING:
        ++cLogInfo.unAckWarningCount;
        break;

    case LOG_TYPE_INFO:
        ++cLogInfo.unAckInfoCount;
        break;
    }

    /* Check the status to see if we need to turn the Attention LED */

    CheckUnackLogStatus();
}


/**
******************************************************************************
**
**  @brief  Decrement event counter
**
**  If the log event is not acknowledged or deleted, decrement the
**  the appropriate log event counter
**
**  @param  logPtr - pointer to log entry
**
**  @return none
**
******************************************************************************
**/
static void DecrEventCounter(LOG_HDR *logPtr)
{
    UINT8   eventType;

    eventType = GetEventType(logPtr->eventCode);

    /*
     * If the entry has not been deleted or acknowledged, then decrement
     * the appropriate counter
     */
    if (!NotReadOrDeleted(logPtr))
    {
        return;
    }

    switch (eventType)
    {
    case LOG_TYPE_ERROR:
        --cLogInfo.unAckErrorCount;
        break;

    case LOG_TYPE_WARNING:
        --cLogInfo.unAckWarningCount;
        break;

    case LOG_TYPE_INFO:
        --cLogInfo.unAckInfoCount;
        break;
    }

    /* Check the status to see if we need to turn the Attention LED */

    CheckUnackLogStatus();
}


/**
******************************************************************************
**
**  @brief  Get a list of the last N log event pointers
**
**  @param  listPtr - location to store the list of pointers
**  @param  logPtr  - log pointer to start list (if NULL start with last entry)
**  @param  logInfo - information for the logs location in Flash and position
**  @param  count   - number of event pointers to find
**
**  @return none
**
******************************************************************************
**/
static void GetLogPtrList(LOG_HDR **listPtr, LOG_HDR *logPtr,
                            LOG_INFO_CB *logInfo, int count)
{
    LOG_PTR_DATA logEntry;
    SEQ32       begSequence;
    SEQ32       endSequence;
    INT32       diff;

    /* If no events were requested, just return */
    if (count == 0 || !listPtr)
    {
        return;
    }

    /* Initialize the list to zero (NULL pointers) */
    memset(listPtr, 0, count * sizeof(LOG_HDR *));

    /*
     * If the passed pointer is NULL, init the log pointer to the end of
     * the logs
     */
    if (!logPtr)
    {
        logPtr = GetLastLogEntry(&logEntry, logInfo);
    }

    /*
     * Ensure the passed log pointer is valid, otherwise return a null
     * list
     */
    if (!ValidLogEntry(logPtr))
    {
        return;
    }

    /* Determine the ending log sequence numbers in list */

    endSequence = logPtr->sequence;

    /* Determine the starting sequence numbers in the list */

    logPtr = GetFirstLogEntry(&logEntry, logInfo);

    if (!logPtr || !ValidLogEntry(logPtr))
    {
        /* Failed to identify the first log entry in the system */
        return;
    }

    begSequence = logPtr->sequence;

    /*
     * If the more entries available then requested, adjust the
     * beginning sequence number to match the count. Otherwise
     * limit the count to the number available.
     */
    diff = endSequence - begSequence;
    if (diff >= count)
    {
        begSequence = endSequence - count + 1;
    }
    else if (diff < 0)          /* Really should not happen */
    {
        count = 1;              /* Force to only 1 */
    }
    else
    {
        count = diff + 1;
    }

    /*
     * Set the log pointer to the beginning of the sequence to generate
     * the list for. If the entry is not valid, return an empty list.
     */
    logPtr = FindLogEntryBySequence(begSequence, logInfo);
    if (SetCurrLogEntry(logPtr, &logEntry) != GOOD)
    {
        return;
    }

    /*
     * Fill in the list. The list will be ordered from newest to
     * oldest, but retrieved in opposite order. So load the list
     * backwards.
     */
    listPtr += count - 1;

    while (count > 0)
    {
        /* Add the log pointer to the list and advance the list pointer */

        *listPtr-- = logPtr;

        /* Get the previous entry in the log */

        logPtr = GetNextLogEntry(&logEntry);

        --count;
    }
}


/**
******************************************************************************
**
**  @brief  Find log entry with a given sequence number
**
**  Searches for a log entry with a specified sequence number,
**  returning a pointer to that log event.
**
**  @param  logSearchSeqNum     - log search sequence number
**  @param  li  - information for the logs location in Flash and position
**
**  @return logPtr - pointer to matching log event (NULL if not found)
**
******************************************************************************
**/
static LOG_HDR *FindLogEntryBySequence(SEQ32 logSearchSeqNum, LOG_INFO_CB *li)
{
    LOG_HDR         *logPtr;
    LOG_PTR_DATA    logData;
    INT32           seqdiff;

    /*
     * Initialize pointer to start of the logs. This is the faster search
     * direction, given the forward pointers.
     */
    logPtr = GetFirstLogEntry(&logData, li);

    /*
     * If we have the fist log entry and the sequence number is
     * less then or equal to the sequence number we are searching
     * for continue.  Otherwise, set logPtr = NULL and return.
     */
    seqdiff = logPtr->sequence - logSearchSeqNum;
    if (!logPtr || seqdiff > 0)
    {
        /*
         * Either logPtr is already NULL or the sequence number we are searching
         * for will not be found since it is less than the first Log Entry.
         * Set logPtr to NULL indicating we did not find the Log Entry.
         */
        return NULL;
    }

    /*
     * Search until the matching entry is found or the end of the logs
     * is reached.
     */
    while (logPtr && logPtr->sequence != logSearchSeqNum)
    {
        logPtr = GetNextLogEntry(&logData);
    }

    return logPtr;
}


/**
******************************************************************************
**
**  @brief  Find log entry with given master sequence number
**
**  Searches for a log entry with a specified master sequence number,
**  returning a pointer to that log event or, if that doesn't exist,
**  the greatest one less than the given one or, if that doesn't exist,
**  the first log event greater than the given one.
**
**  @param  seq - log search sequence number
**  @param  li  - information for the logs location in Flash and position
**
**  @return logPtr - pointer to matching log event
**
******************************************************************************
**/
static LOG_HDR *FindLogEntryByMasterSequence(SEQ32 seq, LOG_INFO_CB *li)
{
    LOG_HDR         *logPtr;
    LOG_PTR_DATA    logData;
    INT32           seqdiff;

    /*
     * Initialize pointer to start of the logs. This is the faster search
     * direction, given the forward pointers.
     */
    logPtr = GetFirstLogEntry(&logData, li);
    if (!logPtr)
    {
        return NULL;
    }

    /*
     * Search until the matching entry is found or the end of the logs
     * is reached.
     */
    seqdiff = logPtr->masterSequence - seq;
    while (logPtr && seqdiff < 0)
    {
        logPtr = GetNextLogEntry(&logData);
        if (!logPtr)
        {
            return NULL;
        }
        seqdiff = logPtr->masterSequence - seq;
    }

    return logPtr;
}


/**
******************************************************************************
**
**  @brief  From the log event code, determine the class of the event.
**
**  @param  eventCode  - log entry event code
**
**  @return eventType - LOG_TYPE_INFO, LOG_TYPE_WARNING, LOG_TYPE_ERROR,
**                            LOG_TYPE_FATAL, LOG_TYPE_DEBUG
**
******************************************************************************
**/
UINT8 GetEventType(UINT16 eventCode)
{
    /*
     * If the event code is above the WARNING level, report a warning.
     * Otherwise, if it is above the ERROR level, report an error. Otherwise,
     * report it as informational.
     */
    if (LOG_IsDebug(eventCode))
    {
        return LOG_TYPE_DEBUG;
    }

    if (LOG_IsError(eventCode) || LOG_IsFatal(eventCode))
    {
        return LOG_TYPE_ERROR;
    }

    if (LOG_IsWarning(eventCode))
    {
        return LOG_TYPE_WARNING;
    }

    return LOG_TYPE_INFO;
}


/**
******************************************************************************
**
**  @brief  Return the next log event sequence number
**
**  @param  none
**
**  @return sequence number
**
******************************************************************************
**/
SEQ32 GetNextLogSequence(void)
{
    LOG_HDR     *lp;

    /* Set the pointer to the last written entry */
    lp = (LOG_HDR *)((char *)cLogInfo.currPtr + cLogInfo.sectorOffset);

    /* Return the previous sequence number and add one to get the next */
    return lp->sequence + 1;
}


/**
******************************************************************************
**
**  @brief  Return the next master sequence number.
**
**  @param  none
**
**  @return Sequence number
**
******************************************************************************
**/
static SEQ32 GetNextMasterSequence(void)
{
    LOG_HDR *clp;
    LOG_HDR *dlp;
    SEQ32   cSequence;
    SEQ32   dSequence;
    INT32   seqdiff;
    UINT8   cfirstEvent = FALSE;
    UINT8   dfirstEvent = FALSE;

    if ((cLogInfo.sectorOffset == (sizeof(LOG_SECTOR) - sizeof(LOG_HDR))) &&
        (((LOG_SECTOR *)cLogInfo.currPtr)->header.le.statusWord == LE_ERASED))
    {
        /*
         * The offset is correct. This must be the first event to be written,
         * so set the sequence number to 0.
         */
        cfirstEvent = TRUE;
        cSequence = 0;
    }
    else
    {
        /* Set the pointer to the last written entry */

        clp = (LOG_HDR *)((char *)cLogInfo.currPtr + cLogInfo.sectorOffset);
        cSequence = clp->masterSequence;
    }

    if ((dLogInfo.sectorOffset == (sizeof(LOG_SECTOR) - sizeof(LOG_HDR))) &&
        (((LOG_SECTOR *)dLogInfo.currPtr)->header.le.statusWord == LE_ERASED))
    {
        /*
         * The offset is correct. This must be the first event to be written,
         * so set the sequence number to 0.
         */
        dfirstEvent = TRUE;
        dSequence = 0;
    }
    else
    {
        /* Set the pointer to the last written entry */

        dlp = (LOG_HDR *)((char *)dLogInfo.currPtr + dLogInfo.sectorOffset);
        dSequence = dlp->masterSequence;
    }

    if (cfirstEvent == TRUE && dfirstEvent == TRUE)
    {
        return 0;
    }

    seqdiff = cSequence - dSequence;
    if (seqdiff >= 0)
    {
        return cSequence + 1;
    }

    return dSequence + 1;
}


/**
******************************************************************************
**
**  @brief  Print the Log Event to the serial debug port
**
**  @param  logPtr  - Pointer to log entry
**
**  @return none
**
******************************************************************************
**/
static void PrintLogEntry(LOG_HDR *logPtr)
{
    char    buffer[MAX_LOG_MESSAGE_SIZE];

    /*
     * Debug code for now. Allows log messages to be sent to the apps log
     * and to a specified UDP client.
     */
    FormatLogEntry(buffer, logPtr, LONG_MESSAGE);
    dprintf(DPRINTF_PRINT_LOGS, "%s", buffer);
}


/**
******************************************************************************
**
**  @brief  Get log event status string
**
**  Get the current status of the log event and convert it to one of
**  the following strings:
**      " "     - unread (not acknowledged)
**      "A"     - acknowledged
**      "D"     - deleted
**
**  @param  status  - log entry status
**  @param  strPtr  - pointer to a string to store the result
**
**  @return none
**
******************************************************************************
**/
void GetStatusString(UINT16 status, UINT16 eventCode, char *strPtr)
{
    strPtr[1] = '\0';       /* Terminate the string */

    /*
     * If the entry has been deleted, then report that. Otherwise, if it has
     * been acknowledged, report that. Otherwise report nothing.
     */
    if ((status & LE_DELETED) == 0)
    {
        strPtr[0] = 'D';
        return;
    }

    if ((status & LE_ACKED) == 0)
    {
        strPtr[0] = 'A';
        return;
    }

    /*
     * If an error event has not been acknowledged or deleted, highlight
     * it with a "!". If a warning has not been acknowledged or deleted,
     * highlight it with a "*".
     */
    if (eventCode >= LE_ERROR_LEVEL)
    {
        strPtr[0] = '!';
        return;
    }

    if (eventCode >= LE_WARNING_LEVEL)
    {
        strPtr[0] = '*';
        return;
    }

    strPtr[0] = ' ';
}


/**
******************************************************************************
**
**  @brief  Find the starting log message by sequence number
**
**  @param  logs - Pointer to array of LOG_HDR pointers
**  @param  eventCount - Number of log messages
**  @param  seq - sequence number to start with
**
**  @return Index to starting log
**
******************************************************************************
**/
static UINT32 FindStartingIndex(LOG_HDR *logs[], UINT32 eventCount, SEQ32 seq)
{
    INT32   seqdiff;
    UINT32  ix;

    for (ix = 0; logs[ix] && ix < eventCount; ++ix)
    {
        seqdiff = logs[ix]->masterSequence - seq;
        if (seqdiff <= 0)
        {
            break;
        }
    }

    return ix;
}


/**
******************************************************************************
**
**  @brief  Merge the debug and customer logs by master sequence number
**
**  @param  logs    - Pointer to array of log message pointers
**  @param  mode    - LogInfo Request mode
**  @param  seq     - sequence number to start with
**  @param  maxLogs - Maximum number of logs to return
**  @param  cLogs   - Pointer to array of customer log pointers as scratch area
**  @param  dLogs   - Pointer to array of debug log pointers as scratch area
**
**  @return PI_GOOD or PI_ERROR
**
******************************************************************************
**/
static INT32 LogInfoMergeLogs(LOG_HDR **logs, UINT16 mode, SEQ32 seq,
                UINT32 maxLogs, LOG_HDR **cLogs, LOG_HDR **dLogs)
{
    LOG_HDR *cLogStart = NULL;
    LOG_HDR *dLogStart = NULL;
    LOG_PTR_DATA    logData;
    SEQ32   cSequence = 0;
    SEQ32   dSequence = 0;
    UINT32  count;
    UINT32  cIndex = 0;
    UINT32  dIndex = 0;

    if (!logs || !maxLogs)  /* If the pointer is not valid, return error */
    {
        return PI_ERROR;
    }

    if (mode & MODE_START_OF_LOGS)
    {
        /* Get oldest logs */
        dLogStart = GetFirstLogEntry(&logData, &dLogInfo);
        cLogStart = GetFirstLogEntry(&logData, &cLogInfo);
    }
    else if (mode & MODE_USE_SEQUENCE)
    {
        /*
         * If we are using a sequence number, find the closest log entry to that
         * sequence number.
         */
        dLogStart = FindLogEntryByMasterSequence(seq, &dLogInfo);
        cLogStart = FindLogEntryByMasterSequence(seq, &cLogInfo);
    }

    /* Get a list of debug and customer logs */

    GetLogPtrList(dLogs, dLogStart, &dLogInfo, maxLogs);
    GetLogPtrList(cLogs, cLogStart, &cLogInfo, maxLogs);

    /*
     * If we are using sequence numbers, we need to
     * skip the garbage at the front that we do not need.
     */
    if (mode & MODE_USE_SEQUENCE)
    {
        cIndex = FindStartingIndex(cLogs, maxLogs, seq);
        dIndex = FindStartingIndex(dLogs, maxLogs, seq);
    }

    /*
     * Now we need to traverse the lists and merge them into
     * one list by mastersequence number.
     */
    for (count = 0; count < maxLogs; ++count)
    {
        UINT8   cSkip = FALSE;
        UINT8   dSkip = FALSE;
        INT32   seqdiff;

        /*
         * If both input lists are NULL at their current index
         * we are done, so break out of the loop.
         */
        if (!cLogs[cIndex] && !dLogs[dIndex])
        {
           break;
        }

        /*
         * If the customer loglist still has
         * entries left get the Master Sequence number.
         */
        if (cIndex < maxLogs && cLogs[cIndex])
        {
            cSequence = cLogs[cIndex]->masterSequence;
        }
        else
        {
            cSkip = TRUE;
        }

        /*
         * If the debug loglist still has
         * entries left get the Master Sequence number.
         */
        if (dIndex < maxLogs && dLogs[dIndex])
        {
            dSequence = dLogs[dIndex]->masterSequence;
        }
        else
        {
            dSkip = TRUE;
        }

        /*
         * Figure out which loglist has the highest
         * Master Sequence number and place it into
         * the merged loglist.
         */
        seqdiff = cSequence - dSequence;
        if (dSkip || (!cSkip && seqdiff > 0))
        {
            logs[count] = cLogs[cIndex];
            ++cIndex;
        }
        else if (!dSkip)
        {
            logs[count] = dLogs[dIndex];
            ++dIndex;
        }
        else
        {
            dprintf(DPRINTF_DEFAULT, "%s: Unexpected condition, "
                    "dSkip=%d, cSkip=%d\n", __func__, dSkip, cSkip);
            break;
        }
    }

    /*
     * If we stopped before we filled out the loglist
     * set the next entry to NULL
     */
    if (count != maxLogs)
    {
        logs[count] = NULL;
    }

    return PI_GOOD;
}


/**
******************************************************************************
**
**  @brief  Initialize log sector for test
**
**  @param  sect    - Sector number to initialize
**  @param  li      - Pointer to corresponding LOG_INFO_CB structure
**  @param  lm      - Pointer to initial log message
**
******************************************************************************
**/
static void init_test_sect(UINT8 sect, LOG_INFO_CB *li, LOG_MSG *lm)
{
    CCB_FLASH   *fp;

    li->currSectNum = sect;
    li->currPtr = (LOG_SECTOR *)(li->startAddress + (sect * li->sectorSize));
    li->sectorOffset = sizeof(LOG_SECTOR) - sizeof(LOG_HDR);
    MarkLogSectorValid(sect, li);               /* Validate it */

    /* Place initial log entry to establish initial message IDs */
    fp = (CCB_FLASH *)((char *)li->currPtr + li->sectorOffset);
    CCBFlashProgramData(fp, (CCB_FLASH *)lm, sizeof(*lm) / sizeof(CCB_FLASH));
}


/**
******************************************************************************
**
**  @brief  Generate interesting random sequence number
**
**  This function returns a random sequence number in an "interesting"
**  range intended to test various number-wrapping scenarios.
**
**  @param  none
**
**  @return Interesting random 32-bit number
**
******************************************************************************
**/
static SEQ32    random_seq(void)
{
    UINT32  num = random();
    SEQ32   seq = 0x7FFFFC00;

    if (num & 0x400)
    {
        seq |= 0x80000000;
    }

    num &= 0x3FF;
    seq |= num;

    dprintf(DPRINTF_DEFAULT, "%s: Generated %08x\n", __func__, seq);

    return seq;
}


/**
******************************************************************************
**
**  @brief  Scramble logs
**
**  This is a test routine that scrambles the logs. It wiptes all of the
**  log flash sectors, picks new sequence numbers in "interesting" ranges,
**  randomly chooses a log flash sector to begin with, and reumes logging
**  from that state.
**
**  @param  none
**
**  @return none
**
******************************************************************************
**/
static void scramble_logs(void)
{
    UINT32  rc;
    static LOG_MSG  l =
    {
        .header =
        {
            .length = sizeof(l.data),
            .le =
            {
                .status =
                {
                    .flags = ~(LE_STARTED | LE_COMPLETE),
                    .reserved = ~0
                }
            }
        }
    };

    dprintf(DPRINTF_DEFAULT, "%s: Beginning log scramble\n", __func__);

    EraseEventLog();                            /* Erase all log sectors */

    /* Place initial log entries to establish initial message IDs */

    l.header.timeStamp = RTC_GetLongTimeStamp();
    l.header.eventCode = LOG_SCRAMBLE_INFO;
    l.header.masterSequence = random_seq();
    l.header.sequence = random_seq();

    init_test_sect(random() % LS_NUM_SECTORS, &cLogInfo, &l);

    l.header.timeStamp = RTC_GetLongTimeStamp();
    l.header.eventCode = LOG_SCRAMBLE_DEBUG;
    ++l.header.masterSequence;
    l.header.sequence = random_seq();

    init_test_sect(random() % LS_NUM_SECTORS_DEBUG, &dLogInfo, &l);

    logState = LOG_VALID;

    rc = InitLogPtrs(&dLogInfo);
    if (rc == ERROR)
    {
        dprintf(DPRINTF_DEFAULT, "%s: Error from InitLogPtrs on debug logs\n",
                    __func__);
    }

    rc = InitLogPtrs(&cLogInfo);
    if (rc == ERROR)
    {
        dprintf(DPRINTF_DEFAULT, "%s: Error from InitLogPtrs on cust logs\n",
                    __func__);
    }

    InitEventCounters();
}


/**
******************************************************************************
**
**  @brief  Fill log space with messages
**
**  @param  count - Number of messages to generate into log
**  @param  debuglog - TRUE if to go into debug log, else to customer log
**
**  @return none
**
******************************************************************************
**/
static void fill_logs(UINT32 count, UINT8 debuglog)
{
    UINT32  i;
    LOG_MRP mrp;
    union
    {
        LOG_HDR hdr;
        UINT8   logbuf[MAX_LOG_MESSAGE_SIZE];
    }       buf;

    if (debuglog)
    {
        mrp.mleEvent = LOG_FILL_DEBUG;
    }
    else
    {
        mrp.mleEvent = LOG_FILL_INFO;
    }
    mrp.mleLength = sizeof(mrp.mleData);

    dprintf(DPRINTF_DEFAULT, "%s: Filling %s log with %d messages\n",
            __func__, debuglog ? "debug" : "customer", count);
    for (i = 0; i < count; ++i)
    {
        _log_event(&mrp, buf.logbuf);
    }
}


/**
******************************************************************************
**
**  @brief  Get requested log information
**
**  @param  rspDataPtr  - Pointer to response area
**  @param  size        - Size of response area
**  @param  mode    - Logging mode bits (see PacketInterface.h)
**  @param  eventCount  - Pointer to count of events to return
**  @param  sequenceNumber  - Pointer to sequence number to start at
**  @param  responseLength  - Pointer to response length, initial offset
**
**  @return PI_GOOD or PI_ERROR, sequenceNumber and eventCount updated
**
******************************************************************************
**/
UINT32   LogInfoRequest(UINT8 *rspDataPtr, UINT32 size, UINT16 mode,
                        UINT32 *eventCount, SEQ32 *sequenceNumber,
                        UINT32 *responseLength)
{
    INT32               rc = PI_GOOD;
    LOG_HDR             **pLogList;
    LOG_HDR             *pLogStart = NULL;
    LOG_HDR             **cLogs = NULL;
    LOG_HDR             **dLogs = NULL;
    PI_LOG_EVENT        *eventPtr;
    PI_BINARY_LOG_EVENT *bEventPtr;
    LOG_INFO_CB         *li;
    LOG_PTR_DATA        logData;
    UINT32              count;
    UINT32              extMsgLngth = 0;
    UINT32              binaryDataLen;

    /* Set the type of logs */
    if (mode & MODE_DEBUG_LOGS)
    {
        li = &dLogInfo;
    }
    else
    {
        li = &cLogInfo;
    }

    /* Allocate space for the list of log event pointers, then get the list */

    pLogList = MallocWC(sizeof(LOG_HDR *) * *eventCount);

    if (mode & MODE_MASTER_SEQUENCE_LOGS)
    {
        /* Allocate the memory for the lists */

        cLogs = MallocWC(sizeof(LOG_HDR *) * *eventCount);
        dLogs = MallocWC(sizeof(LOG_HDR *) * *eventCount);
    }

    LockMutex(&logMutex, MUTEX_WAIT);   /* Take mutex */

    if (mode & MODE_TEST_LOGS)
    {
        switch (*sequenceNumber)
        {
        case 11111111:          /* Scramble logs */
            scramble_logs();
            rc = PI_GOOD;
            break;

        case 22222222:          /* Fill logs */
            fill_logs(*eventCount, FALSE);
            rc = PI_GOOD;
            break;

        case 33333333:          /* Fill debug logs */
            fill_logs(*eventCount, TRUE);
            rc = PI_GOOD;
            break;

        default:
            dprintf(DPRINTF_DEFAULT, "%s: Unknown log test, code=%d\n",
                        __func__, *sequenceNumber);
            rc = PI_ERROR;
            break;
        }

        *eventCount = 0;
        goto out;
    }

    if (mode & MODE_MASTER_SEQUENCE_LOGS)
    {
        /*
         * If we are using master sequence number, we need to generate it
         * ourselves.
         */
        rc = LogInfoMergeLogs(pLogList, mode, *sequenceNumber, *eventCount,
                                cLogs, dLogs);
        if (rc != PI_GOOD)
        {
            goto out;
        }
    }
    else if (mode & MODE_START_OF_LOGS)
    {
        pLogStart = GetFirstLogEntry(&logData, li); /* Get oldest log */
        GetLogPtrList(pLogList, pLogStart, li, *eventCount);
        newClient = TRUE;
    }
    else if (mode & MODE_USE_SEQUENCE)
    {
        /*
         * If the passed sequence number is to be used, find the corresponding
         * entry, otherwise start with the newest log entry.
         */
        pLogStart = FindLogEntryBySequence(*sequenceNumber, li);
        if (!newClient && !pLogStart && *sequenceNumber == 0)
        {
            /*  Get Oldest Log */

            pLogStart = GetFirstLogEntry(&logData, li);
        }
        GetLogPtrList(pLogList, pLogStart, li, *eventCount);
    }
    else
    {
        GetLogPtrList(pLogList, pLogStart, li, *eventCount);
    }

    /* Fill in the log events */

    for (count = 0; count < *eventCount && rc == PI_GOOD; ++count)
    {
        /*
         * If the log pointer is NULL, we have reached the end of the
         * list of valid log events. Adjust the event count and exit the
         * loop. Or if we are approaching the end of the memory allocated
         * for transferring the logs, adjust count and exit.
         */
        if (!pLogList[count] || (*responseLength + MAX_LOG_MESSAGE_SIZE) > size)
        {
            /*
             * If we receive a NULL pointer, before the end of the requested
             * count, then we have reached the end of the logs.
             */
            if (!pLogList[count])
            {
                mode |= MODE_END_OF_LOGS;
                dprintf(DPRINTF_DEFAULT, "%s: reached mode end of logs\n",
                            __func__);
            }

            *eventCount = count;
            break;
        }

        if (!ValidLogEntry(pLogList[count]))
        {
            /* Validation of log entry is failed at the specified index */

            dprintf(DPRINTF_DEFAULT, "%s: validation of log entry is failed index = %d\n",
                        __func__, count);
            if (count < 1)
            {
               *eventCount = 0;
            }
            else
            {
               *eventCount = count - 1;
            }
            break;
        }

        /*
         * If binary logs were requested, just compute the length (including
         * the header) and transfer the raw data. Otherwise, convert the
         * messages to ascii strings.
         */
        if (mode & MODE_BINARY_MESSAGE)
        {
            /*
             * Set the event pointer based on the where we are in the response
             * buffer.
             */
            bEventPtr = (PI_BINARY_LOG_EVENT *)(rspDataPtr + *responseLength);

            /*
             * Compute the length of the binary log event.  This is the
             * length of the event (PI_BINARY_LOG_EVENT) -
             * size of the length field +
             * length of the standard header (LOG_HDR) +
             * the binary data (mleData[] from LOG_MRP)
             */
            bEventPtr->length = sizeof(PI_BINARY_LOG_EVENT) -
                                sizeof(bEventPtr->length) +
                                sizeof(LOG_HDR) + pLogList[count]->length;

            /* Determine the event type from the event code */

            bEventPtr->eventType = GetEventType(pLogList[count]->eventCode);

            /* Copy the binary data (header and data) */

            memcpy(bEventPtr->message, pLogList[count],
                   sizeof(LOG_HDR) + pLogList[count]->length);

            /*
             * Compute the total length of the response data packet.
             * This includes the size of the length field which was
             * subtracted off the length calculation above.
             * Add it back here.
             */
            *responseLength += bEventPtr->length + sizeof(bEventPtr->length);
        }
        else
        {
            /* ------ ASCII EVENT ----- */

            /*
             * Set the event pointer based on the where we are in the response
             * buffer.
             */
            eventPtr = (PI_LOG_EVENT *)(rspDataPtr + *responseLength);

            /* Fill in the data associated with this log event */

            eventPtr->ascii.eventCode = pLogList[count]->eventCode;
            eventPtr->ascii.masterSequenceNumber = pLogList[count]->masterSequence;
            eventPtr->ascii.sequenceNumber = pLogList[count]->sequence;
            eventPtr->ascii.statusWord = pLogList[count]->le.statusWord;

            GetTimeString((&(pLogList[count]->timeStamp)),
                          eventPtr->ascii.timeAndDate);
            GetEventTypeString(pLogList[count]->eventCode,
                               eventPtr->ascii.messageType);
            GetMessageString(pLogList[count], eventPtr->ascii.messageDescr, 0);

            /* Compute the length of the message string */

            eventPtr->ascii.length = strlen(eventPtr->ascii.messageDescr) + 1;

            /*
             * If the mode indicates to send the extended message information,
             * tack it on to the end of the message and adjust the
             * message length.
             */
            if (mode & MODE_EXTENDED_MESSAGE)
            {
                extMsgLngth = eventPtr->ascii.length;

                eventPtr->ascii.length +=
                    HTML_ExtendedMessage(((char *)eventPtr->ascii.messageDescr) +
                    eventPtr->ascii.length,  pLogList[count]);

                if (extMsgLngth != eventPtr->ascii.length)
                {
                    eventPtr->ascii.length += 1;
                    eventPtr->ascii.messageDescr[extMsgLngth - 1] = '\n';
                }
            }

            /*
             * The length field includes the length of all the fields
             * in the ASCII log event except the length field itself.
             */
            eventPtr->ascii.length += sizeof(eventPtr->ascii.eventCode) +
                                      sizeof(eventPtr->ascii.masterSequenceNumber) +
                                      sizeof(eventPtr->ascii.sequenceNumber) +
                                      sizeof(eventPtr->ascii.statusWord) +
                                      sizeof(eventPtr->ascii.timeAndDate) +
                                      sizeof(eventPtr->ascii.messageType);

            /*
             * Add the length of the ASCII event to the cumulative response
             * length.  Also include the size of the ASCII length field and
             * the size of the event length field.
             */
            *responseLength += eventPtr->ascii.length +
                              sizeof(eventPtr->ascii.length) +
                              sizeof(eventPtr->length);
            /*
             * Tack the binary event on at the end of the ASCII event.
             * Set the event pointer to the start of the binary portion
             * of the data.  We must calculate the start of the binary
             * data instead of using structure references since the
             * preceding ASCII data is variable length.
             */
            bEventPtr = (PI_BINARY_LOG_EVENT *)(rspDataPtr +
                                                           *responseLength);
            /*
            ** Compute the length of the binary data.
            ** The length calculation is -
            **
            ** size of the event (PI_BINARY_LOG_EVENT) -
            ** size of the length field +
            ** the binary data (mleData[] from LOG_MRP)
            */
            binaryDataLen = sizeof(PI_BINARY_LOG_EVENT) -
                                 sizeof(bEventPtr->length) +
                                 pLogList[count]->length;

            if (*responseLength + binaryDataLen > size)
            {
                 *responseLength += binaryDataLen + sizeof(bEventPtr->length);
                 dprintf(DPRINTF_DEFAULT, "%s: reached the max size response = %d binarydatalen = %d\n",
                            __func__, *responseLength, binaryDataLen);
                 continue;
            }

            bEventPtr->length = binaryDataLen;

            /* Determine the event type from the event code */

            bEventPtr->eventType = GetEventType(pLogList[count]->eventCode);

            /* Copy the binary data ONLY (no header) */

            memcpy(bEventPtr->message,
                    (char *)pLogList[count] + sizeof(LOG_HDR),
                    pLogList[count]->length);

            /*
             * Add the length of the binary event to the cumulative response
             * length.  Also include the size of the binary length field.
             */
            *responseLength += bEventPtr->length + sizeof(bEventPtr->length);

            /*
             * The length for the complete event includes the ASCII and
             * binary event lengths and the size of their respective
             * length fields.
             */
            eventPtr->length = eventPtr->ascii.length +
                               sizeof(eventPtr->ascii.length) +
                               bEventPtr->length +
                               sizeof(bEventPtr->length);
        }
    }

    if (*eventCount)
    {
        if (mode & MODE_MASTER_SEQUENCE_LOGS)
        {
            *sequenceNumber = pLogList[*eventCount - 1]->masterSequence;
        }
        else
        {
            *sequenceNumber = pLogList[*eventCount - 1]->sequence;
        }
    }

out:
    UnlockMutex(&logMutex);     /* Release mutex */

    Free(pLogList); /* Free the log list */
    if (cLogs)
    {
        Free(cLogs);
    }
    if (dLogs)
    {
        Free(dLogs);
    }

    return rc;
}


/**
******************************************************************************
**
**  @brief  Clear all logs
**
**  @param  none
**
**  @return none
**
******************************************************************************
**/
void LogClearRequest(void)
{
    LockMutex(&logMutex, MUTEX_WAIT);

    InitLogs(CLEAR_LOGS_NOW);   /* Initialize the logs */

    UnlockMutex(&logMutex);

    return;
}


/**
******************************************************************************
**
**  @brief  Acknowledge a log entry
**
**  @param  seq - Sequence number of log message to acknowledge
**
**  @return Pointer to allocated copy of log message, or NULL
**
**  @attention  The caller must free any returned pointer
**
******************************************************************************
**/
LOG_HDR *LogAcknowledge(SEQ32 seq)
{
    LOG_HDR *pLog;
    LOG_HDR *copy;
    UINT32  length;

    copy = MallocWC(sizeof(LOG_HDR) + MAX_LOG_MESSAGE_SIZE);

    LockMutex(&logMutex, MUTEX_WAIT);

    /*  Get Log entry. If log found, mark it as read */

    pLog = FindLogEntryBySequence(seq, &cLogInfo);
    if (!pLog)
    {
        UnlockMutex(&logMutex);
        dprintf(DPRINTF_DEFAULT, "%s: LOG %d NOT FOUND\n", __func__, seq);
        Free(copy);
        return NULL;
    }

    MarkLogEntryRead(pLog);

    /* Save a copy of the log message in allocated memory */

    length = pLog->length;
    if (length > MAX_LOG_MESSAGE_SIZE)
    {
        length = MAX_LOG_MESSAGE_SIZE;
    }
    memcpy(copy, pLog, sizeof(LOG_HDR) + length);

    UnlockMutex(&logMutex);

    return copy;
}


/**
******************************************************************************
**
**  @brief  Return a log entry
**
**  @param  seq - Sequence number of log entry to return
**
**  @return Pointer to log entry, or NULL if not found
**
******************************************************************************
**/
LOG_HDR *GetLogEntry(SEQ32 seq)
{
    LOG_HDR *pLog;
    LOG_HDR *copy;
    UINT32  length;

    copy = MallocWC(sizeof(LOG_HDR) + MAX_LOG_MESSAGE_SIZE);

    LockMutex(&logMutex, MUTEX_WAIT);

    /* Get Log entry. If log found, mark it as read */

    pLog = FindLogEntryBySequence(seq, &cLogInfo);
    if (!pLog)
    {
        UnlockMutex(&logMutex);
        dprintf(DPRINTF_DEFAULT, "%s: LOG %d NOT FOUND\n", __func__, seq);
        Free(copy);
        return NULL;
    }

    /* Save a copy of the log message in allocated memory */

    length = pLog->length;
    if (length > MAX_LOG_MESSAGE_SIZE)
    {
        length = MAX_LOG_MESSAGE_SIZE;
    }
    memcpy(copy, pLog, sizeof(LOG_HDR) + length);

    UnlockMutex(&logMutex);

    return copy;
}


/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
