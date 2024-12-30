/* $Id: LogSimFuncs.c 140034 2010-05-06 18:11:58Z mdr $ */
/**
******************************************************************************
**
**  @file   LogSimFuncs.c
**
**  @brief  Log Simulator functions
**
**  Copyright (c) 2001-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#include "LogSimFuncs.h"    

#include <stdlib.h>
#include <time.h>
#include <fcntl.h>

#include "logging.h"
#include "logview.h"
#include "ccb_flash.h"
#include "rtc.h"
#include "nvram.h"
#include "i82559.h"
#include "rtc.h"

/*****************************************************************************
** Private defines
*****************************************************************************/

/*****************************************************************************
** Private variables
*****************************************************************************/
static char     logType = SHORT_MESSAGE;

/* Log Information structure */
extern LOG_INFO_CB cLogInfo;
extern LOG_INFO_CB dLogInfo;


/*****************************************************************************
** Public variables - externed in the header file
*****************************************************************************/
UINT8   logSimMemSpace[LS_SECTOR_SIZE * LS_NUM_SECTORS];
UINT8   logSimDebugMemSpace[LS_SECTOR_SIZE_DEBUG * LS_NUM_SECTORS_DEBUG];

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static void  LogSimInitMemFromFile(FILE *, UINT8 *memSpace, UINT32 size);
static FILE *LogSimOpenFileRWDestroy(const char *fileToOpen);
static FILE *LogSimOpenFileBinaryReadOnly(const char *fileToOpen);
INT32 LogSimMergeLogs(LOG_HDR **pLogList, UINT16 mode, 
                      UINT32 sequenceNumber, UINT32 eventCount);

/*****************************************************************************
** Code Start
*****************************************************************************/


/**
******************************************************************************
**
**  @brief  Set the memory space for the logs
**
**  @param  logInfo - Structure to store data in
**
**  @return none
**
******************************************************************************
**/
void LogSimSetMemSpace(LOG_INFO_CB *logInfo)
{
    logInfo->startAddress = (UINT32)logSimMemSpace;
    logInfo->sectorSize = LS_SECTOR_SIZE;
    logInfo->numSectors = LS_NUM_SECTORS;
    logInfo->startSector = 0;
}


/**
******************************************************************************
**
**  @brief  Set the memory space for the debug logs
**
**  @param  logInfo - Structure to store data in
**
**  @return none
**
******************************************************************************
**/
void LogSimSetDebugMemSpace(LOG_INFO_CB *logInfo)
{
    logInfo->startAddress = (UINT32)logSimDebugMemSpace;
    logInfo->sectorSize = LS_SECTOR_SIZE_DEBUG;
    logInfo->numSectors = LS_NUM_SECTORS_DEBUG;
    logInfo->startSector = 4;
}


/**
******************************************************************************
**
**  @brief  Format the log entry
**
**  @param  buffer  - buffer to fill
**  @param  logPtr  - Pointer to a log entry
**
**  @return none
**
******************************************************************************
**/
static UINT32 LogSimFormatLogEntry(char *buffer, LOG_HDR *logPtr)
{
    char    tmpBuffer[MAX_LOG_MESSAGE_SIZE];
    char    *newline = NULL;
    UINT32  rc = 0;

    FormatLogEntry(tmpBuffer, logPtr, logType);

    /* Strip the carriage returns */

    while ((newline = strstr(tmpBuffer, "\r")) != NULL)
    {
        newline[0] = ' ';
    }

    rc = sprintf(buffer, "%5.5d %s", logPtr->masterSequence, tmpBuffer);

    return rc;
}


/**
******************************************************************************
**
**  @brief  Print a log entry
**
**  @param  logPtr  - Pointer to a log entry
**
**  @return none
**
******************************************************************************
**/
void LogSimPrintLogEntry(LOG_HDR *logPtr)
{
    char    buffer[MAX_LOG_MESSAGE_SIZE];

    LogSimFormatLogEntry(buffer, logPtr);

    printf("%s", buffer);
}


/**
******************************************************************************
**
**  @brief  Loads customer and debug memory from binary log files
**
**  @param  cfile    -   customer file to load
**  @param  dfile    -   debug file to load
**
**  @return GOOD | ERROR
**
******************************************************************************
**/
UINT32 LogSimLoadCDFile(char *cfile, char *dfile)
{
    FILE    *clogFile;
    FILE    *dlogFile;

    clogFile = LogSimOpenFileBinaryReadOnly(cfile);
    dlogFile = LogSimOpenFileBinaryReadOnly(dfile);

    if (!clogFile || !dlogFile)
    {
        return ERROR;
    }

    LogSimInitMemFromFile(clogFile, logSimMemSpace, sizeof(logSimMemSpace));
    LogSimInitMemFromFile(dlogFile, logSimDebugMemSpace, 
                          sizeof(logSimDebugMemSpace));
    InitLogs();

    fclose(clogFile);
    fclose(dlogFile);

    return GOOD;
}


/**
******************************************************************************
**
**  @brief  Loads customer memory from binary customer log file
**
**  @param  myfile    -   file to load
**
**  @return GOOD | ERROR
**
******************************************************************************
**/
UINT32 LogSimLoadFile(char *myfile)
{
    FILE    *logFile;

    logFile = LogSimOpenFileBinaryReadOnly(myfile);
    if (!logFile)
    {
        return ERROR;
    }

    LogSimInitMemFromFile(logFile, logSimMemSpace, sizeof(logSimMemSpace));

    InitLogs();

    fclose(logFile);

    return GOOD;
}


/**
******************************************************************************
**
**  @brief  Loads debug memory from binary debug log file
**
**  @param  myfile    -   file to load
**
**  @return GOOD | ERROR
**
******************************************************************************
**/
UINT32 LogSimLoadDebugFile(char *myfile)
{
    FILE    *logFile;

    logFile = LogSimOpenFileBinaryReadOnly(myfile);
    if (!logFile)
    {
        return ERROR;
    }

    LogSimInitMemFromFile(logFile, logSimDebugMemSpace, 
                          sizeof(logSimDebugMemSpace));
    InitLogs();

    fclose(logFile);

    return GOOD;
}


/**
******************************************************************************
**
**  @brief  Print all the logs from memory merged
**
**  @param  outFile - file to output data.  NULL = STDOUT
**  @param  reverse - reverse the output
**              
**  @return none
**
******************************************************************************
**/
void LogSimPrintAllLogs(char *outFile, char *needle, bool reverse)
{
    LogSimPrintLogs(MODE_MASTER_SEQUENCE_LOGS | MODE_START_OF_LOGS |
                    //MODE_USE_SEQUENCE |
                    MODE_EXTENDED_MESSAGE, 
                    0, 0xFFFF, outFile, needle, reverse);
}


/*----------------------------------------------------------------------------
** Function:  LogSimPrintCustomerLogs
**
** Description: Print all the customer logs from memory.
**
** Inputs:      outFile - file to output data.  NULL = STDOUT
**              reverse - reverse the output
**
** Returns:     NONE
**
** WARNING:     NONE
**
**--------------------------------------------------------------------------*/
void LogSimPrintCustomerLogs(char *outFile, char *needle, bool reverse)
{
    LogSimPrintLogs(//MODE_USE_SEQUENCE |
                    MODE_START_OF_LOGS | MODE_EXTENDED_MESSAGE, 
                    0, 0xFFFF, outFile, needle, reverse);
}

/*----------------------------------------------------------------------------
** Function:  LogSimPrintDebugLogs
**
** Description: Print all the debug logs from memory.
**
** Inputs:      outFile - file to output data.  NULL = STDOUT
**              reverse - reverse the output
**
** Returns:     NONE
**
** WARNING:     NONE
**
**--------------------------------------------------------------------------*/
void LogSimPrintDebugLogs(char *outFile, char *needle, bool reverse)
{
    LogSimPrintLogs(MODE_DEBUG_LOGS | MODE_START_OF_LOGS |
                    //MODE_USE_SEQUENCE |
                    MODE_EXTENDED_MESSAGE, 
                    0, 0xFFFF, outFile, needle, reverse);
}


/*----------------------------------------------------------------------------
**  Function Name: LogSimSetLogType
**
**  Comments:   Sets log type.
**
**  Parameters: type (SHORT_MESSAGE | LONG_MESSAGE)
**
**  Returns:    NONE
**--------------------------------------------------------------------------*/
void LogSimSetLogType(char type)
{
    logType = type;
}


/**
******************************************************************************
**
**  @brief  Prints the logs from memory
**
**  @param  mode            - MODE_BINARY_MESSAGE   |
**                            MODE_EXTENDED_MESSAGE |
**                            MODE_USE_SEQUENCE     |
**                            MODE_DEBUG_LOGS       |
**                            MODE_MASTER_SEQUENCE_LOGS |
**                            MODE_START_OF_LOGS
**  @param  sequenceNumber  - Starting sequence number
**  @param  eventCount      - Number of logs to print
**  @param  outFile         - Output file (NULL = STDOUT)
**  @param  reverse - reverse the output
**
**  @return none
**
******************************************************************************
**/
UINT32 LogSimPrintLogs(UINT16 mode, UINT32 sequenceNumber, UINT32 eventCount,
                        char *outFile, char *needle, bool reverse)
{
    UINT32      rc = 0;
    LOG_HDR     **pLogList = NULL;
    LOG_HDR     *pLog     = NULL;
    LOG_HDR     *pLogStart = NULL;
    LOG_INFO_CB *logInfo;
    UINT32      ix;
    UINT32      tail;
    FILE        *logPrintFile = NULL;
    char        buffer[MAX_LOG_MESSAGE_SIZE];
    char        buffer2[MAX_LOG_MESSAGE_SIZE];

    if (outFile)
    {
        logPrintFile = LogSimOpenFileRWDestroy(outFile);
    }

    if (needle)
    {
        strtoupper(needle);
    }

    /* Set the type of logs */
    if (mode & MODE_DEBUG_LOGS)
    {
        logInfo = &dLogInfo;
    }
    else
    {
        logInfo = &cLogInfo;
    }

    /*
    ** Allocate space for the list of log event pointers and then get the
    ** list.
    */
    pLogList = malloc(sizeof(LOG_HDR **) * eventCount);

    /*
    ** If we are using master sequence number, we need to generate it
    ** ourselves
    */
    if (pLogList && (mode & MODE_MASTER_SEQUENCE_LOGS))
    {
        LogSimMergeLogs(pLogList, mode, sequenceNumber, eventCount);
    }
    /*
    ** If we have a valid list pointer, determine the starting entry and
    ** generate the requested number of entries.
    */
    else if (pLogList)
    {
        /*
        ** If the passed sequence number is to be used, find the corresponding
        ** entry, otherwise start with the newest log entry.
        */
        if (mode & MODE_USE_SEQUENCE)
        {
            pLogStart = FindLogEntryBySequence(sequenceNumber, logInfo);
            printf("%s: Finding sequence number\n", __func__);
        }

        GetLogPtrList(pLogList, pLogStart, logInfo, eventCount);
    }

    /* Fill in the log events */

    if (!reverse)
    {
        tail = 0;

        while (pLogList[tail] && tail < eventCount)
        {
            ++tail;
        }

        if (tail > 1)
        {
            --tail;

            for (ix = 0; ix < tail; ++ix, --tail)
            {
                pLog = pLogList[tail];
                pLogList[tail] = pLogList[ix];
                pLogList[ix] = pLog;
            }
        }
    }

    for (ix = 0; ix < eventCount; ++ix)
    {
        /*
        ** If the log pointer is NULL, we have reached the end of the
        ** list of valid log events. Adjust the event count and exit the
        ** loop. Or if we are approaching the end of the memory allocated
        ** for transferring the logs, adjust count and exit.
        */
        if (!pLogList[ix])
        {
            mode |= MODE_END_OF_LOGS;
            break;
        }

        /* ------ ASCII EVENT ----- */

        LogSimFormatLogEntry(buffer, pLogList[ix]);

        strcpy(buffer2, buffer);
        strtoupper(buffer2);

        if (!needle || (needle && strstr(buffer2, needle) != NULL))
        {
            if (logPrintFile)
            {
                fprintf(logPrintFile, "%s", buffer);
            }
            else
            {
                printf("%s", buffer);
            }
        }

        if (mode & MODE_MASTER_SEQUENCE_LOGS)
        {
            rc = GetMasterSequenceNumber(pLogList[ix]);
        }
        else
        {
            rc = GetSequenceNumber(pLogList[ix]);
        }
    }

    /* Free the log list */

    if (pLogList)
    {
        free(pLogList);
    }
    
    if (logPrintFile)
    {
        fclose(logPrintFile);
    }

    return rc;
}


/**
******************************************************************************
**
**  @brief  Merges the debug and customer logs by master sequence number
**
**  @param  mode            - LogInfo Request mode
**  @param  masterSeqNum    - sequence number to start with
**  @param  masterSeqNum    - sequence number to start with
**
**  @return PI_GOOD or PI_ERROR
**
******************************************************************************
**/
INT32 LogSimMergeLogs(LOG_HDR **pLogList, UINT16 mode, 
                      UINT32 sequenceNumber, UINT32 eventCount)
{
    INT32       rc = PI_GOOD;
    LOG_HDR     **pLogListCustomer;
    LOG_HDR     **pLogListDebug;
    LOG_HDR     *pLogStartCustomer = NULL;
    LOG_HDR     *pLogStartDebug = NULL;
    INT64       customerSequence, debugSequence;
    UINT32      ix;
    UINT32      customerIndex, debugIndex;

    /* Set the indexes and sequence numbers to 0 */
    customerIndex = 0;
    debugIndex = 0;
    customerSequence= 0;
    debugSequence = 0;

    /* Allocate the memory to for the lists */
    pLogListCustomer = malloc(sizeof(LOG_HDR *) * eventCount);
    pLogListDebug = malloc(sizeof(LOG_HDR *) * eventCount);

    /* If all our pointers are valid carry on */
    if (pLogList && pLogListCustomer && pLogListDebug)
    {
        /*
        ** If we are using a sequence number, find the closest
        ** log entry to that sequence number.
        */
        if (mode & MODE_USE_SEQUENCE)
        {
            pLogStartDebug = 
                FindLogEntryByMasterSequence(sequenceNumber, &dLogInfo);
            pLogStartCustomer = 
                FindLogEntryByMasterSequence(sequenceNumber, &cLogInfo);
        }

        /* Get a list of debug and customer logs */

        GetLogPtrList(pLogListDebug, pLogStartDebug, &dLogInfo, eventCount);
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
        for (ix = 0; ix < eventCount && rc == PI_GOOD; ++ix)
        {
            /*
            ** If both input lists are NULL at their current index
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
            else
            {
                /*
                ** Else set it to -1 so that it will always be less than
                ** the debug loglist Master Sequence Number
                */
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
            else
            {
                /*
                ** Else set it to -1 so that it will always be less than
                ** the customer loglist Master Sequence Number
                */
                debugSequence = -1;
            } 

            /*
            ** Figure out which loglist has the highest
            ** Master Sequence number and place it into 
            ** the mered loglist
            */
            if (customerSequence > debugSequence)
            {
                pLogList[ix] = pLogListCustomer[customerIndex];
                ++customerIndex;
            }
            else
            {
                pLogList[ix] = pLogListDebug[debugIndex];
                ++debugIndex;
            }
        }
        
        /*
        ** If we stopped before we filled out the loglist
        ** set the next entry to NULL
        */
        if (ix != eventCount)
        {
            pLogList[ix] = NULL;
        }
    }
    else
    {
        rc = PI_ERROR;
    }

    /* Free the log lists */
    if (pLogListCustomer)
    {
        free(pLogListCustomer);
    }
    if (pLogListDebug)
    {
        free(pLogListDebug);
    }

    return rc;
}


/**
******************************************************************************
**
**  @brief  Initialize the memory space from an open file
**
**  @param  memFile     - Open file to load binary logs
**  @param  memSpace    - buffer to store binary logs
**  @param  size        - length of file to copy to memSpace
**
**  @return none
**
******************************************************************************
**/
static void LogSimInitMemFromFile(FILE *memFile, UINT8 *memSpace, UINT32 size)
{
    int numread = 0;
    int totalread = 0;
    unsigned int headeroffset;

    memset(memSpace, 0, size);

    if (!memFile)
    {
        return;
    }

    fseek(memFile, 0, SEEK_SET);

    /* Look for a header */

    if (fread(&headeroffset, 1, sizeof(headeroffset), memFile) == sizeof(headeroffset))
    {
        headeroffset = (headeroffset == 0x312A2A66) ? 32 : 0;

        fseek(memFile, headeroffset, SEEK_SET);
        while ((numread = fread(memSpace + totalread, 1, size - totalread,
                memFile)) != 0)
        {
            totalread += numread;
        }
    }
}


/**
******************************************************************************
**
**  @brief  Opens a file (w). Destroys if already exists.
**
**  @param  fileToOpen  - File to open
**
**  @return Handle to the fileToOpen.  NULL if Error.
**
******************************************************************************
**/
static FILE *LogSimOpenFileRWDestroy(const char *fileToOpen)
{
    FILE    *retFile;

    retFile = fopen(fileToOpen, "w+");
    if (retFile)
    {
        fseek(retFile, 0, SEEK_SET);
    }

    return retFile;
}


/**
******************************************************************************
**
**  @brief  Opens a file (read only)
**
**  @param  fileToOpen  - File to open
**
**  @return Handle to the fileToOpen.  NULL if Error.
**
******************************************************************************
**/
static FILE *LogSimOpenFileBinaryReadOnly(const char *fileToOpen)
{
    FILE *retFile;

    retFile = fopen(fileToOpen, "rb");

    return retFile;
}


/**
******************************************************************************
**
**  @brief  Program the flash devices
**
**  Of course there isn't really flash in LogSim, so just sort of pretend.
**
**  @param  flashAddressPtr - CCBFlash destination address
**  @param  source          - source of flash data
**  @param  length          - length of data to program (in flash words)
**
**  @return GOOD status
**
******************************************************************************
**/
UINT32 CCBFlashProgramData(CCB_FLASH *flashAddressPtr, CCB_FLASH *sourcePtr, UINT32 wordLength)
{
    UINT32 offset = 0;
    UINT8 *memSpace;

    if (((UINT32)flashAddressPtr >= ((UINT32)logSimMemSpace + (LS_SECTOR_SIZE * LS_NUM_SECTORS))) ||
        ((UINT32)flashAddressPtr < (UINT32)logSimMemSpace))
    {
        memSpace = logSimDebugMemSpace;
        offset = (UINT32)flashAddressPtr - (UINT32)logSimDebugMemSpace;
    }
    else
    {
        memSpace = logSimMemSpace;
        offset = (UINT32)flashAddressPtr - (UINT32)logSimMemSpace;
    }

    memcpy(memSpace + offset, sourcePtr, wordLength * sizeof(CCB_FLASH));

    return GOOD;
}


/**
******************************************************************************
**
**  @brief  Erase a single flash sector
**
**  @param  sectorNum - flash sector number
**
**  @return GOOD
**
******************************************************************************
**/
UINT32 CCBFlashEraseSector(UINT32 sectorNum)
{
    CCB_FLASH *flashPtr = NULL;

    if (CCBFlashGetAddressFromSector(sectorNum, &flashPtr) == GOOD)
    {
        memset((flashPtr), 0xFF, LS_SECTOR_SIZE);
    }

    return GOOD;
}


/**
******************************************************************************
**
**  @brief  Calculate address from sector number
**
**  Pass in the sector you wish to program or erase, and the base
**  address of the flash memory, get back a flash address to pass to
**  the rest of the routines.
**
**  @param  FlashBase - CCBFlashBase base address
**  @param  Sector - flash sector number
**
**  @return GOOD or ERROR
**  @return *flashAddressPtrPtr - Address corresponding to the sector number
**
******************************************************************************
**/
UINT32 CCBFlashGetAddressFromSector(UINT32 sector, CCB_FLASH **flashAddressPtrPtr)
{
    UINT8 *memSpace;

    if (sector >= CCB_FLASH_NUMBER_OF_SECTORS)
    {
        *flashAddressPtrPtr = NULL;
        return ERROR;
    }

    if (sector >= 4)
    {
        memSpace = logSimDebugMemSpace;
        sector -= 4;
    }
    else
    {
        memSpace = logSimMemSpace;
    }

    /*
    ** Takes adresses of sectors from table, multiplies by 4 (the flash is set up word wide,
    ** but the table is for byte wide devices.
    */
    *flashAddressPtrPtr = ((CCB_FLASH *)((sector * LS_SECTOR_SIZE) + memSpace));

    return GOOD;
}


/**
******************************************************************************
**
**  @brief  Allocates and Clears Memory (Waits for memory)
**
**  @param  length - Length in bytes of memory to allocate
**
**  @return pointer to allocated memory
**
******************************************************************************
**/
void *MallocWC(UINT32 length)
{ 
    void *mem = NULL; 

    while (!mem)
    {
        mem = calloc(1, length);
    }  

    return mem;
}


/*
 * Convert an IP address to a dotted string 
 * The input must be in network byte order
 */
void InetToAscii(UINT32 ipAddress, char *outputIpAddressString)
{
    UINT8 size = 0;

    size += sprintf(outputIpAddressString + size, "%d.", (UINT8)(ipAddress & 0x000000FF));
    size += sprintf(outputIpAddressString + size, "%d.", (UINT8)((ipAddress >> 8) & 0x000000FF));
    size += sprintf(outputIpAddressString + size, "%d.", (UINT8)((ipAddress >> 16) & 0x000000FF));
    sprintf(outputIpAddressString + size, "%d", (UINT8)(ipAddress >> 24));
    
   return; 
}


/**
******************************************************************************
**
**  @brief  Queue a message for UDP transfer out to a debug console
**
**  @param  none
**
**  @return none
**
******************************************************************************
**/
void _DPrintf(UNUSED UINT32 level, const char *fmt, ...)
{ 
    va_list args;

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}


/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
