/* $Id: logging.c 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       logging.c
**
**  @brief      Logging implementation for LogSim
**
**  The contents of this file is derived from CCB/Src/logging.c. Material
**  changes made there may have to be made here as well. However note that
**  usually that should not be the case, as changes made to things like the
**  structure of the log flash sectors will affect backward compatability.
**  Any fixes made in the CCB version for things like locking are completely
**  irrelevant to this version.
**
**  Copyright (c) 2001-2010 XIOtech Corporation.  All rights reserved.
**
******************************************************************************
**/
#include "logging.h"

#include "LogSimFuncs.h"


/*****************************************************************************
** Private defines
*****************************************************************************/

/* #define CLEAR_LOGS_ON_CCB_NVRAM_CLEAR */
#define LOGS_CORRUPT_RETRY_COUNT    10

/*
** Log sector definitions
** The first 32 bit word of the flash sector indicates whether the information
** in the sector is valid. Log entries start immediately after that valid word.
*/
#define LS_VALID                    0xFEDC0008
#define LS_ERASED                   0xFFFFFFFF

/*
** Log entry status flags - flags are negative active
*/
#define LE_STARTED                  0x0001
#define LE_COMPLETE                 0x0002
#define LE_ACKED                    0x4000
#define LE_DELETED                  0x8000
#define LE_ERASED                   0xFFFFFFFF

#define FirstLogEntryByLogInfo(logInfoPtr) (GetFirstLogEntry(&gLogEntry, logInfoPtr))
#define NextLogEntry() (GetNextLogEntry(&gLogEntry))
#define DeletedLogEntry(logPtr) ((logPtr->le.status.flags & LE_DELETED)== 0)
#define ReadLogEntry(logPtr)    ((logPtr->le.status.flags & LE_ACKED)== 0)
#define  NotReadOrDeleted(logPtr)  \
    ((logPtr->le.status.flags & (LE_DELETED|LE_ACKED)) == (LE_DELETED|LE_ACKED))
#define  GetUnAckAccumCount()  (cLogInfo.unAckErrorCount)

/**
******************************************************************************
** Private structure definitions
******************************************************************************
**/


/*****************************************************************************
** Private variables
*****************************************************************************/
/* Log Event Mutex */

static UINT16   logState;

/*****************************************************************************
** Public variables - externed in the header file
*****************************************************************************/

/* Log Information structure */
LOG_INFO_CB cLogInfo;
LOG_INFO_CB dLogInfo;

/*****************************************************************************
** Public variables - not externed in any header file
*****************************************************************************/

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
UINT8 ValidateLogEntry(LOG_INFO_CB *, LOG_HDR *);    /* Check for full log entry */
UINT32 InitLogPtrs(LOG_INFO_CB *);       /* Initializes log pointer to next log entry */
static void EraseLogSector(UINT8 sectorNum, UINT32 offset); /* Erase log sector */
static void MarkLogSectorValid(UINT8 sectorNum, UINT32 offset);/* Mark the sector as valid */
static UINT8 ValidLogSectors(LOG_INFO_CB *logInfo); /* Do valid log sectors exist */
static UINT32 NewerLogTime(UINT32 time1, UINT32 time2);
static LOG_HDR *GetLastLogEntry(LOG_PTR_DATA *, LOG_INFO_CB *);
static LOG_HDR *GetNextLogEntry(LOG_PTR_DATA *);
static UINT32 SetCurrLogEntry(LOG_HDR *, LOG_PTR_DATA *);

/**
******************************************************************************
** Code Start
******************************************************************************
**/


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
**
**  @return none
**
******************************************************************************
**/
void InitLogs(void)
{
    UINT32  rc;
    UINT32  index1;

    /* Load up information about the logs */

    LogSimSetMemSpace(&cLogInfo);
    LogSimSetDebugMemSpace(&dLogInfo);

    /*
     * Set the logging state variable, based on the requested initialization.
     */
    logState = LOG_VALID;

    /*
     * Look to see if any valid log sectors exist. If not, erase the log area
     * and mark the first sector as valid.
     */
    if (!ValidLogSectors(&cLogInfo))
    {
        int i;

        for (i = 0; i < cLogInfo.numSectors; ++i)
        {
            EraseLogSector(i, cLogInfo.startSector);
        }

        /* Mark first sector valid */
        MarkLogSectorValid(0, cLogInfo.startSector);
    }

    if (!ValidLogSectors(&dLogInfo))
    {
        int i;

        for (i = 0; i < dLogInfo.numSectors; ++i)
        {
            EraseLogSector(i, dLogInfo.startSector);
        }

        /* Mark first sector valid */
        MarkLogSectorValid(0, dLogInfo.startSector);
    }

    /*
     * Initialize the log pointers, finding the first valid entry, the last entry,
     * and the next entry. Initialize the unacknowledged event counters.
     */

    /*
     * Intitialize the Debug logs. If InitLogPtrs returns ERROR, it has
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
        fprintf(stderr, "%s: Debug Log Flash was Corrupted - Trying Again\n",
                __func__);
    }

    /*
     * If we have an ERROR here we are in trouble. For some reason we
     * could not correct the corrupt sector in the flash. This could
     * be caused by the inability to program the flash device. For now
     * we will carry on.
     */
    if (rc == ERROR)
    {
        /*
         * For now we will not take any action on a failure to initialize
         * the logs.  If something needs to be done in the future if we hit
         * this case, this is where it needs to happen.
         */
        fprintf(stderr, "%s: Debug Log Flash Corrupted!\n", __func__);
    }

    /*
     * Intitialize the Customer logs. If InitLogPtrs returns ERROR, it has
     * detected and corrected a corrupt sector in the flash where the logs
     * are stored. We must call InitLogPtrs again to correctly initialize
     * the log pointers. We will retry this LOGS_CORRUPT_RETRY_COUNT times.
     */
    for (index1 = 0; index1 < LOGS_CORRUPT_RETRY_COUNT; ++index1)
    {
        rc = InitLogPtrs(&cLogInfo);
        if (rc != ERROR)
        {
            break;
        }
        fprintf(stderr, "%s: Customer Log Flash was Corrupted - Trying Again\n",
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
        fprintf(stderr, "%s: Customer Log Flash Corrupted!\n", __func__);
    }
}


/**
******************************************************************************
**
**  @brief  Return the first valid log entry (oldest entry in system).
**
**  @param  logEntry - pointer to LOG_PTR_DATA structure
**  @param  logInfo - pointer to information for the logs location in Flash
**                      and current location.
**
**  @return logPtr - pointer to log entry, NULL if no valid entry
**
******************************************************************************
**/
LOG_HDR *GetFirstLogEntry(LOG_PTR_DATA *logEntry, LOG_INFO_CB *logInfo)
{
    LOG_HDR *logPtr;

    /*
     * The first valid log entry (the oldest log entry in the system) is
     * located at the base address of the flash logs, plus the offset of
     * the starting sector times the size of a log sector. The offset of that
     * entry is located after the sector header information.
     */
    logEntry->sectPtr = (char *)(logInfo->startAddress +
                    (logInfo->startSectNum * logInfo->sectorSize));
    logEntry->sectorOffset = sizeof(LOG_SECTOR) - sizeof(LOG_HDR);
    logEntry->sectorNum = logInfo->startSectNum;
    logEntry->logInfoPtr = logInfo;

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


/*----------------------------------------------------------------------------
**  Function Name: GetNextLogEntry
**
**  Comments:   Return the next valid log entry.
**
**  Parameters: Pointer to LOG_PTR_DATA structure
**
**  Returns:  logPtr - pointer to log entry
**                   - NULL if no valid entry
**
**--------------------------------------------------------------------------*/
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
        else
        {
            /*
             * Proceed to the next sector, wrapping if we are at the end of the log
             * sectors. Reset the sector pointer and the sector offset.
             */
            logEntry->sectorNum = (logEntry->sectorNum + 1) % logEntry->logInfoPtr->numSectors;
            logEntry->sectPtr = (char *)(logEntry->logInfoPtr->startAddress + (logEntry->sectorNum * logEntry->logInfoPtr->sectorSize));
            logEntry->sectorOffset = sizeof(LOG_SECTOR) - sizeof(LOG_HDR);
        }

    }
    else
    {
        /*
         * Save away the changed log entry pointer information
         */
        logEntry->sectorOffset = sectorOffset;
    }

    /*
     *  Compute the new log pointer
     */
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


/*----------------------------------------------------------------------------
**  Function Name: SetCurrLogEntry
**
**  Comments: Set the current log data information based on the passed log
**            pointer.
**
**
**  Parameters: Pointer to a log entry
**              Pointer to LOG_PTR_DATA structure
**
**  Returns:  GOOD - log entry set successfully
**            ERROR - invalid logPtr
**
**--------------------------------------------------------------------------*/
static UINT32 SetCurrLogEntry(LOG_HDR *logPtr, LOG_PTR_DATA *logEntry)
{
    int         i;

    /* Intialize the log pointer data to the start of the logs */
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
**  @brief  Determines the validity of a log entry
**
**  @param  logPtr  - pointer to the log in flash
**
**  @return TRUE    - Valid Log Entry, FALSE   - Corrupted Log Entry
**
******************************************************************************
**/
UINT8 ValidateLogEntry(LOG_INFO_CB *logInfo, LOG_HDR *logPtr)
{
    /*
     * These are some fields that should not be all F's.  If any
     * of these fields meet this condition, this would indicate
     * a corrupted log entry (Entry that was not entirely written
     * to flash).  set rc = FALSE (Corrupted Log Entry).
     */
    if (logPtr->length == 0xFFFF ||
        logPtr->eventCode == 0xFFFF || logPtr->timeStamp == 0xFFFFFFFF)
    {
        return FALSE;
    }

    /*
     * Check that this entry points to another valid log message.
     * First check that we are not peeking beyond the end of the sector.
     */
    if (((UINT32)logPtr + logPtr->length + sizeof(LOG_HDR)) -
              (UINT32)logInfo->currPtr < logInfo->sectorSize)
    {
        /*
         * Check that this entry points to another valid log message.
         * If it does not, this also indicates a corrupted log
         * entry (Entry that was not entirely written to flash).
         */
        if (!ValidLogEntry((LOG_HDR *)((UINT32)logPtr + logPtr->length + sizeof(LOG_HDR))))
        {
            /*
             * It is possible that the log entry could point to a log entry
             * that has yet to be written.  This would be the case if this
             * particular log message was the last entry written.  If the next
             * entry is inavalid and not LE_ERASED, set rc = FALSE (Corrupted
             * Log Entry).
             */
            if (((LOG_HDR *)((UINT32)logPtr +
                          logPtr->length + sizeof(LOG_HDR)))->le.statusWord != LE_ERASED)
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
**  This function finds the first valid entry, the last entry,
**  and the next entry.
**
**  @param  logInfo - the logs location in Flash and current position
**
**  @note   Log Information structure should be set-up and the logs initialized
**          before calling.
**
**  @return GOOD    - InitLogPtrs was successful
**  @return ERROR   - InitLogPtrs found a corrupt log entry and attempted
**                    to fix the sector.  InitLogPtrs must be called again
**                    to initialize the log pointers for logInfo.
**
******************************************************************************
**/
UINT32 InitLogPtrs(LOG_INFO_CB *logInfo)
{
    LOG_SECTOR  *currPtr;
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
    oldPtr = newPtr = (LOG_SECTOR *)logInfo->startAddress;
    newSector = oldSector = 0;

    for (i = 0; i < logInfo->numSectors; ++i)
    {
        /* Set a pointer to the next log sector */
        currPtr = (LOG_SECTOR *)(logInfo->startAddress +
                        (i * logInfo->sectorSize));

        /* Determine if the sector we are looking at has a valid log entry */
        if (currPtr->validWord == LS_VALID &&
            (currPtr->header.le.status.flags & (LE_STARTED | LE_COMPLETE)) == 0)
        {
            /*
             * If this is the first valid sector we found,
             * then mark this sector as having both
             * the oldest and newest log entries.
             */
            if (firstSectorFound == FALSE)
            {
                oldPtr = newPtr = currPtr;
                oldSector = newSector = i;

                firstSectorFound = TRUE;
            }
            else
            {
                /*
                 * Compare the time of the first entry in the log sectors and
                 * keep the oldest
                 */
                if (NewerLogTime(oldPtr->header.timeStamp, currPtr->header.timeStamp) ==
                    oldPtr->header.timeStamp)
                {
                    oldPtr = currPtr;
                    oldSector = i;
                }

                /*
                 * Compare the time of the first entry in the log sectors and
                 * keep the newest
                 */
                if (NewerLogTime(newPtr->header.timeStamp, currPtr->header.timeStamp) ==
                    currPtr->header.timeStamp)
                {
                    newPtr = currPtr;
                    newSector = i;
                }
            }
        }
    }

    /*
     * Save the sector number of the sector with the oldest log entry (the start of
     * the log).
     */
    logInfo->startSectNum = oldSector;

    /* Set pointer to the first entry in the newest log sector */
    logInfo->currPtr = newPtr;
    logInfo->currSectNum = newSector;

    char    *logPtr;
    UINT32  offset;
    UINT32  lastOffset;

    /*
     * Find the offset of the last entry in the current log sector
     * Init pointer and offset to the first entry in the sector.
     */
    offset = lastOffset = sizeof(LOG_SECTOR) - sizeof(LOG_HDR);
    logPtr = (char *)logInfo->currPtr;

    while (offset < logInfo->sectorSize &&
           ((LOG_HDR *)(logPtr + offset))->le.statusWord != LE_ERASED)
    {
        /*
         * Check that we have a complete log entry in order
         * to continue.
         */
        if ((ValidateLogEntry(logInfo, ((LOG_HDR *)(logPtr + offset)))) == TRUE)
        {
            lastOffset = offset;        /* Save the previous offset */

            /* Point to the next entry (size of data plus the header) */
            offset += ((LOG_HDR *)(logPtr + offset))->length + sizeof(LOG_HDR);
        }
        else
        {
            /*
             * If we do not, we need to fix the current sector.
             */
            fprintf(stderr, "%s - Invalid Log Entry Found!\n", __func__);
            fprintf(stderr, "-> statusWord: 0x%08X\n",
                    ((LOG_HDR *)(logPtr + offset))->le.statusWord);
            fprintf(stderr, "-> masterSeq:  0x%08X\n",
                    ((LOG_HDR *)(logPtr + offset))->masterSequence);
            fprintf(stderr, "-> sequence:   0x%08X\n",
                    ((LOG_HDR *)(logPtr + offset))->sequence);
            fprintf(stderr, "-> length:     0x%04hX\n",
                    ((LOG_HDR *)(logPtr + offset))->length);
            fprintf(stderr, "-> eventCode:  0x%04hX\n",
                    ((LOG_HDR *)(logPtr + offset))->eventCode);
            fprintf(stderr, "-> timeStamp:  0x%08X\n",
                    ((LOG_HDR *)(logPtr + offset))->timeStamp);

            /*
             * Fill the rest of the sector with all F's. This will fill from
             * the corrupted log to the end of the sector.
             */
            memset(logPtr + offset, 0xFF, logInfo->sectorSize - offset);

            /* Return the error to the caller and let them try again */
            return ERROR;
        }
    }

    /*
    ** Find the offset of the last entry in the current log sector by
    ** finding the previous entry to the end of the sector
    */
    logInfo->sectorOffset = lastOffset;

    return GOOD;
}


/**
******************************************************************************
**
**  @brief  Erases a single log sector
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
**  @brief  Mark a logging sector as valid
**
**  Sets the "magic word" at the start of the logging sector to
**  indicate the sector contains valid data or is erased and ready
**  to accept log entries.
**
**  @param  sectorNum - sector number to mark valid
**
******************************************************************************
**/
static void MarkLogSectorValid(UINT8 sectorNum, UINT32 offset)
{
    CCB_FLASH  *flashPtr = NULL;
    CCB_FLASH   source = LS_VALID;

    sectorNum += offset;    /* Compute the sector offset for the flash device */

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
**  @param  logInfo - information for the logs location in Flash and
**                    current location
**
**  @return 1 - if valid sectors exist, 0 - if no valid sectors
**
******************************************************************************
**/
static UINT8 ValidLogSectors(LOG_INFO_CB *logInfo)
{
    LOG_SECTOR  *currPtr;
    int         i;

    for (i = 0; i < logInfo->numSectors; ++i)
    {
        /* Set a pointer to the next log sector */
        currPtr = (LOG_SECTOR *)(logInfo->startAddress +
                    (i * logInfo->sectorSize));

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
**  @param  logPtr - pointer to a log entry
**
**  @return 1 - valid log entry, 0 - invalid log entry
**
******************************************************************************
**/
int ValidLogEntry(LOG_HDR *logPtr)
{
    if (((UINT8 *)logPtr < logSimMemSpace ||
        (UINT8 *)logPtr >= logSimMemSpace + sizeof(logSimMemSpace)) &&
        ((UINT8 *)logPtr < logSimDebugMemSpace ||
        (UINT8 *)logPtr >= logSimDebugMemSpace + sizeof(logSimDebugMemSpace)))
    {
        return 0;
    }

    /*
     * If both the started and complete bits have been set (negative active),
     * then indicate the entry is valid.
     */
    if ((logPtr->le.status.flags & (LE_STARTED | LE_COMPLETE)) == 0)
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
**  @brief  Get a list of the last N log event pointers
**
**  @param  listPtr - location to store the list of pointers
**  @param  logPtr  - log pointer to start list (if NULL start with last
**                    entry)
**  @param  logInfo - information for the logs location in Flash and
**                    current location
**  @param  count   - number of event pointers to find
**
**  @return none
**
******************************************************************************
**/
void GetLogPtrList(LOG_HDR **listPtr, LOG_HDR *logPtr, LOG_INFO_CB *logInfo,
                int count)
{
    LOG_PTR_DATA logEntry;
    UINT32      begSequence;
    UINT32      endSequence;
    INT32       diff;

    /* If no events were requested, just return */
    if (count == 0 || !listPtr)
    {
        return;
    }

    /* Initialize the list to zero (NULL pointers) */
    memset(listPtr, 0, count * sizeof(LOG_HDR **));

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
    endSequence = GetSequenceNumber(logPtr);

    /*
     * Determine the starting sequence numbers in the list.
     *
     * Defect 22431
     *
     * Added a check to make sure that a NULL pointer returned from
     * GetFirstLogEntry is handled properly and not sent to
     * GetSequenceNumber.
     */
    logPtr = GetFirstLogEntry(&logEntry, logInfo);

    if (logPtr == NULL || !ValidLogEntry(logPtr))
    {
        /* Failed to identify the first log entry in the system */
        return;
    }

    begSequence = GetSequenceNumber(logPtr);

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
        /*
         * Add the log pointer to the list and advance the
         * list pointer.
         */
        *listPtr-- = logPtr;

        /* Get the previous entry in the log */
        logPtr = GetNextLogEntry(&logEntry);

        --count;
    }
}


/*----------------------------------------------------------------------------
**  Function Name: FindLogEntryBySequence
**
**  Comments:   Searches for a log entry with a specified sequence number,
**              returning a pointer to that log event.
**
**  Parameters: logSearchSeqNum     - log search sequence number
**              logInfo - information for the logs location in Flash and
**                        current location.
**
**  Returns:    logPtr - pointer to matching log event (NULL if not found)
**
**--------------------------------------------------------------------------*/
LOG_HDR *FindLogEntryBySequence(UINT32 logSearchSeqNum, LOG_INFO_CB *logInfo)
{
    LOG_HDR         *logPtr;
    LOG_PTR_DATA    logData;

    /*
     * Initialize pointer to start of the logs. This is the faster search
     * direction, given the forward pointers.
     */
    logPtr = GetFirstLogEntry(&logData, logInfo);

    /*
     * If we have the fist log entry and the sequence number is
     * less then or equal to the sequence number we are searching
     * for continue.  Otherwise, set logPtr = NULL and return.
     */
    if (logPtr && logPtr->sequence <= logSearchSeqNum)
    {
        /*
         * Search until the matching entry is found or the end of the logs
         * is reached.
         */
        while (logPtr && logPtr->sequence != logSearchSeqNum)
        {
            logPtr = GetNextLogEntry(&logData);
        }
    }

    /*
     * Either logPtr is already NULL or the sequence number we are searching
     * for will not be found since it is less than the first Log Entry.
     * Set logPtr to NULL indicating we did not find the Log Entry.
     */
    else
    {
        logPtr = NULL;
    }

    return logPtr;
}


/*----------------------------------------------------------------------------
**  Function Name: FindLogEntryByMasterSequence
**
**  Comments:   Searches for a log entry with a specified master sequence number,
**              returning a pointer to that log event.
**
**  Parameters: logSearchSeqNum     - log search sequence number
**              logInfo - information for the logs location in Flash and
**                        current location.
**
**  Returns:    logPtr - pointer to matching log event (NULL if not found)
**
**--------------------------------------------------------------------------*/
LOG_HDR *FindLogEntryByMasterSequence(UINT32 logSearchSeqNum, LOG_INFO_CB *logInfo)
{
    LOG_HDR         *logPtr;
    LOG_PTR_DATA    logData;

    /*
     * Initialize pointer to start of the logs. This is the faster search
     * direction, given the forward pointers.
     */
    logPtr = GetFirstLogEntry(&logData, logInfo);

    /*
     * Search until the matching entry is found or the end of the logs
     * is reached.
     */
    while (logPtr && logPtr->masterSequence < logSearchSeqNum)
    {
        logPtr = GetNextLogEntry(&logData);
    }

    return logPtr;
}

/*----------------------------------------------------------------------------
**  Function Name: GetEventType
**
**  Comments:  From the log event code, determine the class of the event.
**
**  Parameters:   eventCode  - log entry event code
**
**  Returns:      eventType - LOG_TYPE_INFO,
**                            LOG_TYPE_WARNING,
**                            LOG_TYPE_ERROR,
**                            LOG_TYPE_FATAL,
**                            LOG_TYPE_DEBUG
**
**--------------------------------------------------------------------------*/
UINT8 GetEventType(UINT16 eventCode)
{
    UINT8       eventType;

    /*
     * If the event code is above the WARNING level, report a warning.
     * Otherwise, if it is above the ERROR level, report an error. Otherwise,
     * report it as informational.
     */
    if (LOG_IsDebug(eventCode))
    {
        eventType = LOG_TYPE_DEBUG;
    }
    else if ((LOG_IsError(eventCode)) || (LOG_IsFatal(eventCode)))
    {
        eventType = LOG_TYPE_ERROR;
    }
    else if (LOG_IsWarning(eventCode))
    {
        eventType = LOG_TYPE_WARNING;
    }
    else
    {
        eventType = LOG_TYPE_INFO;
    }

    return (eventType);
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
UINT32 GetNextLogSequence(void)
{
    LOG_INFO_CB *li;
    CCB_FLASH   *flashptr;

    li = &cLogInfo;     /* Access customer log info */

    /* Set the pointer to the last written entry */
    flashptr = (CCB_FLASH *)((char *)li->currPtr + li->sectorOffset);

    /* Return the previous sequence number and add one to get the next */
    return ((LOG_HDR *)flashptr)->sequence + 1;
}


/*----------------------------------------------------------------------------
**  Function Name: GetSequenceNumber
**
**  Comments:  Returnd the sequence number associated with the log event
**
**  Parameters:   logPtr     - log entry pointer
**
**  Returns:      sequence number
**
**--------------------------------------------------------------------------*/
UINT32 GetSequenceNumber(LOG_HDR *logPtr)
{
    /*
     * Return the Log event sequence number
     */
    return logPtr->sequence;
}

/**
******************************************************************************
**
**  @brief  Return the master sequence number associated with the log event
**
**  @param  logPtr     - log entry pointer
**
**  @return Master sequence number
**
**--------------------------------------------------------------------------*/
UINT32 GetMasterSequenceNumber(LOG_HDR *logPtr)
{
    return logPtr->masterSequence;
}


/**
******************************************************************************
**
**  @brief  Get log event status string
**
**  Get the current status of the log event and convert it to one of
**  the following strings:
**      " "     - unread (not acknowledged)
**      "A"     - acknoledged
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
    /*
    ** If the entry has been deleted, then report that. Otherwise, if it has
    ** been acknoledged, report that. Otherwise report nothing.
    */
    if ((status & LE_DELETED) == 0)
    {
        strPtr[0] = 'D';
    }
    else if ((status & LE_ACKED) == 0)
    {
        strPtr[0] = 'A';
    }
    else
    {
        /*
        ** If an error event has not been acknowledged or deleted, highlight
        ** it with a "!". If a warning has not been acknowledged or deleted,
        ** highlight it with a "*".
        */
        if (eventCode >= LE_ERROR_LEVEL)
        {
            strPtr[0] = '!';
        }
        else if (eventCode >= LE_WARNING_LEVEL)
        {
            strPtr[0] = '*';
        }
        else
        {
            strPtr[0] = ' ';
        }
    }

    strPtr[1] = '\0';       /* Terminate the string */
}


/**
******************************************************************************
**
**  @brief  Merge the debug and customer logs by master sequence number
**
**  @param  pLogList        - Pointer to array of log message pointers
**  @param  mode            - LogInfo Request mode
**  @param  sequenceNumber  - sequence number to start with
**  @param  eventCount      - Number of events to return
**
**  @return PI_GOOD or PI_ERROR
**
******************************************************************************
**/
static INT32 LogInfoMergeLogs(LOG_HDR **pLogList, UINT16 mode,
                       UINT32 sequenceNumber, UINT32 eventCount)
{
    LOG_HDR **pLogListCustomer;
    LOG_HDR **pLogListDebug;
    LOG_HDR *pLogStartCustomer = NULL;
    LOG_HDR *pLogStartDebug = NULL;
    INT64   customerSequence;
    INT64   debugSequence;
    UINT32  count;
    UINT32  customerIndex;
    UINT32  debugIndex;

    /*
    ** If the pointer is not valid, return error.
    */
    if (!pLogList)
    {
        return PI_ERROR;
    }

    /*
    ** Allocate the memory to for the lists.
    */
    pLogListCustomer = (LOG_HDR **)MallocWC(sizeof(LOG_HDR *) * eventCount);
    pLogListDebug = (LOG_HDR **)MallocWC(sizeof(LOG_HDR *) * eventCount);

    /*
    ** Set the indexes and sequence numbers to 0.
    */
    customerIndex = 0;
    debugIndex = 0;
    customerSequence= 0;
    debugSequence = 0;

    /*
    ** If we are using a sequence number, find the closest log entry to that
    ** sequence number.
    */
    if (mode & MODE_USE_SEQUENCE)
    {
        pLogStartDebug =
            FindLogEntryByMasterSequence(sequenceNumber, &dLogInfo);
        pLogStartCustomer =
            FindLogEntryByMasterSequence(sequenceNumber, &cLogInfo);
    }

    /*
    ** Get a list of debug and customer logs
    */
    GetLogPtrList(pLogListDebug, pLogStartDebug,
                  &dLogInfo, eventCount);
    GetLogPtrList(pLogListCustomer, pLogStartCustomer,
                  &cLogInfo, eventCount);

    /*
    ** If we are using sequence numbers, we need to
    ** skip the garbage at the front that we do not need.
    */
    if (mode & MODE_USE_SEQUENCE)
    {
        while (pLogListCustomer[customerIndex] &&
               customerIndex < eventCount &&
               GetMasterSequenceNumber(pLogListCustomer[customerIndex]) >
               sequenceNumber)
        {
            ++customerIndex;
        }

        while (pLogListDebug[debugIndex] &&
               debugIndex < eventCount &&
               GetMasterSequenceNumber(pLogListDebug[debugIndex]) >
               sequenceNumber)
        {
            ++debugIndex;
        }
    }


    /*
    ** Now we need to traverse the lists and merge them into
    ** one list by mastersequence number.
    */
    for (count = 0; count < eventCount; ++count)
    {
        /*
        ** If both input lists are NULL at there current index
        ** we are done, so break out of the loop.
        */
        if (!pLogListCustomer[customerIndex] && !pLogListDebug[debugIndex])
        {
           break;
        }

        /*
        ** If the customer loglist still has
        ** entries left get the Master Sequence number.
        */
        if (customerIndex < eventCount && pLogListCustomer[customerIndex])
        {
            customerSequence =
                GetMasterSequenceNumber(pLogListCustomer[customerIndex]);
        }
        /*
        ** Else set it to -1 so that it will always be less than
        ** the debug loglist Master Sequence Number
        */
        else
        {
            customerSequence = -1;
        }

        /*
        ** If the debug loglist still has
        ** entries left get the Master Sequence number.
        */
        if (debugIndex < eventCount && pLogListDebug[debugIndex])
        {
            debugSequence =
                GetMasterSequenceNumber(pLogListDebug[debugIndex]);
        }
        /*
        ** Else set it to -1 so that it will always be less than
        ** the customer loglist Master Sequence Number
        */
        else
        {
            debugSequence = -1;
        }

        /*
        ** Figure out which loglist has the highest
        ** Master Sequence number and place it into
        ** the mered loglist
        */
        if (customerSequence > debugSequence)
        {
            pLogList[count] = pLogListCustomer[customerIndex];
            ++customerIndex;
        }
        else if (debugSequence != -1)
        {
            pLogList[count] = pLogListDebug[debugIndex];
            ++debugIndex;
        }
        else
        {
            dprintf(DPRINTF_DEFAULT, " debugSequence and customersequence are -1\n");
            break;
        }
    }

    /*
    ** If we stopped before we filled out the loglist
    ** set the next entry to NULL
    */
    if (count != eventCount)
    {
        pLogList[count] = NULL;
    }

    /*
    ** Free the log lists
    */
    if (pLogListCustomer)
    {
        Free(pLogListCustomer);
    }
    if (pLogListDebug)
    {
        Free(pLogListDebug);
    }

    return PI_GOOD;
}


/**
******************************************************************************
**
**  @brief  Get requested log information
**
**  @param  rspDataPtr  - Pointer to response area
**  @param  size        - Size of response area
**  @param  mode        - bit 0   0 = send in ASCII format
**                                1 = send in binary format
**                        bit 1   0 = no extended data (ASCII)
**                                1 = send extended data (ASCII)
**                        bit 2   0 = use newest event as start
**                                1 = use sequence number as starting event
**                        bit 3   0 = Normal logs
**                                1 = Debug Logs
**                        bit 4   0 = Use Log Sequence
**                                1 = Use Master Sequence
**  @param  eventCount  - Pointer to count of events to return
**  @param  sequenceNumber  - Pointer to sequence number to start at
**  @param  responseLength  - Pointer to response length, initial offset
**
**  @return PI_GOOD or PI_ERROR, sequenceNumber and eventCount updated
**
******************************************************************************
**/
UINT32   LogInfoRequest(UINT8 *rspDataPtr, UINT32 size, UINT16 mode,
                        UINT32 *eventCount, UINT32 *sequenceNumber,
                        UINT32 *responseLength)
{
    INT32               rc = PI_GOOD;
    LOG_HDR             **pLogList;
    LOG_HDR             *pLogStart = NULL;
    PI_LOG_EVENT        *eventPtr;
    PI_BINARY_LOG_EVENT *bEventPtr;
    LOG_INFO_CB         *logInfo;
    LOG_PTR_DATA        logData;
    UINT32              count;
    UINT32              extMsgLngth = 0;
    UINT32              binaryDataLen;

    /*
    ** Allocate space for the list of log event pointers and then get the
    ** list.
    */
    pLogList = MallocWC(sizeof(LOG_HDR *) * *eventCount);

    /* Set the type of logs */
    if (mode & MODE_DEBUG_LOGS)
    {
        logInfo = &dLogInfo;
    }
    else
    {
        logInfo = &cLogInfo;
    }

    if (mode & MODE_MASTER_SEQUENCE_LOGS)
    {
        /*
        ** If we are using master sequence number, we need to generate it
        ** ourselves
        */
        rc = LogInfoMergeLogs(pLogList, mode, *sequenceNumber, *eventCount);
    }
    else if (mode & MODE_USE_SEQUENCE)
    {
        /*
        ** If the passed sequence number is to be used, find the corresponding
        ** entry, otherwise start with the newest log entry.
        */
        pLogStart = FindLogEntryBySequence(*sequenceNumber, logInfo);
        if (pLogStart == NULL && *sequenceNumber == 0)
        {
            /*
            ** Get Oldest Log
            */
            pLogStart = GetFirstLogEntry(&logData, logInfo);
        }
        GetLogPtrList(pLogList, pLogStart, logInfo, *eventCount);
    }
    else
    {
        GetLogPtrList(pLogList, pLogStart, logInfo, *eventCount);
    }

    /*
    ** Fill in the log events.
    */
    for (count = 0; count < *eventCount && rc == PI_GOOD; ++count)
    {
        /*
        ** If the log pointer is NULL, we have reached the end of the
        ** list of valid log events. Adjust the event count and exit the
        ** loop. Or if we are approaching the end of the memory allocated
        ** for transferring the logs, adjust count and exit.
        */
        if (pLogList[count] ||
                (*responseLength + MAX_LOG_MESSAGE_SIZE) > size)
        {
            /*
            ** If we receive a NULL pointer, before the end of the requested
            ** count, then we have reached the end of the logs.
            */
            dprintf(DPRINTF_DEFAULT, "%s: pLogList[%d] may be null or rsponseLength = %d is higher value,\n",
                        __func__, count, *responseLength);
            if (!pLogList[count])
            {
                mode |=  MODE_END_OF_LOGS;
                dprintf(DPRINTF_DEFAULT, "%s: reached mode end of logs\n",
                            __func__);
            }

            if ((*responseLength + MAX_LOG_MESSAGE_SIZE) > size)
            {
                if (count < 2)
                {
                    *eventCount = count;
                }
                else
                {
                    *eventCount = count - 2;
                }
            }
            else
            {
                *eventCount = count;
            }
            break;
        }

        if (!ValidLogEntry(pLogList[count]))
        {
            /*
            ** Validation of log entry is failed at the specified index
            **/
            dprintf(DPRINTF_DEFAULT, "%s: validation of log entry is failed index = %d\n",
                        __func__, count);
            if (count < 1)
            {
               *eventCount = count;
            }
            else
            {
               *eventCount = count - 1;
            }
            break;
        }

        /*
        ** If binary logs were requested, just compute the length (including
        ** the header) and transfer the raw data. Otherwise, convert the
        ** messages to ascii strings.
        */
        if (mode & MODE_BINARY_MESSAGE)
        {
            /*
            ** Set the event pointer based on the where we are in the response
            ** buffer.
            */
            bEventPtr = (PI_BINARY_LOG_EVENT *)(rspDataPtr + *responseLength);

            /*
            ** Compute the length of the binary log event.  This is the
            ** length of the event (PI_BINARY_LOG_EVENT) -
            ** size of the length field +
            ** length of the standard header (LOG_HDR) +
            ** the binary data (mleData[] from LOG_MRP)
            */
            bEventPtr->length = sizeof(PI_BINARY_LOG_EVENT) -
                                sizeof(bEventPtr->length) +
                                sizeof(LOG_HDR) + pLogList[count]->length;
            /*
            ** Determine the event type from the event code.
            */
            bEventPtr->eventType = GetEventType(pLogList[count]->eventCode);

            /*
            ** Copy the binary data (header and data).
            */
            memcpy(bEventPtr->message, pLogList[count],
                   sizeof(LOG_HDR) + pLogList[count]->length);

            /*
            ** Compute the total length of the response data packet.
            ** This includes the size of the length field which was
            ** subtracted off the length calculation above.
            ** Add it back here.
            */
            *responseLength += bEventPtr->length + sizeof(bEventPtr->length);
        }
        else
        {
            /* ------ ASCII EVENT ----- */

            /*
            ** Set the event pointer based on the where we are in the response
            ** buffer.
            */
            eventPtr = (PI_LOG_EVENT *)(rspDataPtr + *responseLength);

            /*
            ** Fill in the data associated with this log event.
            */
            eventPtr->ascii.eventCode = pLogList[count]->eventCode;
            eventPtr->ascii.masterSequenceNumber = GetMasterSequenceNumber(pLogList[count]);
            eventPtr->ascii.sequenceNumber = GetSequenceNumber(pLogList[count]);
            eventPtr->ascii.statusWord = pLogList[count]->le.statusWord;

            GetTimeString((&(pLogList[count]->timeStamp)),
                          eventPtr->ascii.timeAndDate);
            GetEventTypeString(pLogList[count]->eventCode,
                               eventPtr->ascii.messageType);
            GetMessageString(pLogList[count], eventPtr->ascii.messageDescr, 0);

            /*
            ** Compute the length of the message string
            */
            eventPtr->ascii.length = strlen(eventPtr->ascii.messageDescr) + 1;

            /*
            ** If the mode indicates to send the extended message information,
            ** tack it on to the end of the message and adjust the the
            ** message length.
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
            ** The length field includes the length of all the fields
            ** in the ASCII log event except the length field itself.
            */
            eventPtr->ascii.length += sizeof(eventPtr->ascii.eventCode) +
                                      sizeof(eventPtr->ascii.masterSequenceNumber) +
                                      sizeof(eventPtr->ascii.sequenceNumber) +
                                      sizeof(eventPtr->ascii.statusWord) +
                                      sizeof(eventPtr->ascii.timeAndDate) +
                                      sizeof(eventPtr->ascii.messageType);

            /*
            ** Add the length of the ASCII event to the cumulative response
            ** length.  Also include the size of the ASCII length field and
            ** the size of the event length field.
            */
            *responseLength += eventPtr->ascii.length +
                              sizeof(eventPtr->ascii.length) +
                              sizeof(eventPtr->length);
            /*
            ** Tack the binary event on at the end of the ASCII event.
            ** Set the event pointer to the start of the binary portion
            ** of the data.  We must calculate the start of the binary
            ** data instead of using structure references since the
            ** preceding ASCII data is variable length.
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

            /*
            ** Determine the event type from the event code.
            */
            bEventPtr->eventType = GetEventType(pLogList[count]->eventCode);

            /*
            ** Copy the binary data ONLY (no header).
            */
            memcpy(bEventPtr->message,
                    (char *)pLogList[count] + sizeof(LOG_HDR),
                    pLogList[count]->length);

            /*
            ** Add the length of the binary event to the cumulative response
            ** length.  Also include the size of the binary length field.
            */
            *responseLength += bEventPtr->length + sizeof(bEventPtr->length);
            /*
            ** The length for the complete event includes the ASCII and
            ** binary event lengths and the size of their respective
            ** length fields.
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

    if (pLogList)       /* If LogList was allocated */
    {
        Free(pLogList); /* Free the log list */
    }

    return rc;
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
LOG_HDR *GetLogEntry(UINT32 seq)
{
    LOG_HDR *pLog;
    LOG_HDR *copy;
    UINT32  length;

    copy = MallocWC(sizeof(LOG_HDR) + MAX_LOG_MESSAGE_SIZE);

    /* ??? Take mutex here ??? */

    /*
    ** Get Log entry. If log found, mark it as read.
    */
    pLog = FindLogEntryBySequence(seq, &cLogInfo);
    if (!pLog)
    {
        dprintf(DPRINTF_DEFAULT, "%s: LOG %d NOT FOUND\n", __func__, seq);
        Free(copy);
        /* ??? Release mutex here ??? */
        return NULL;
    }

    /* Save a copy of the log message in allocated memory */

    length = pLog->length;
    if (length > MAX_LOG_MESSAGE_SIZE)
    {
        length = MAX_LOG_MESSAGE_SIZE;
    }
    memcpy(copy, pLog, sizeof(LOG_HDR) + length);

    /* ??? Release mutex here ??? */

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
