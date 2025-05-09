/* $Id: logging.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       logging.h
**
**  @brief      Header file for logging.c
**
**  This file is derived from the contents of CCB/Inc/logging.h specifically
**  for LogSim to use. Material changes made in the CCB code may have to be
**  made here, but note that changes that are that important will very likely
**  break compatability, so it should be rare for such changes to be
**  needed here.
**
**  Copyright (c) 2001,2009-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _LOGGING_H_
#define _LOGGING_H_

#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/
#define MAX_FORMATTED_ENTRY_SIZE    (1024)

/*
** Logging States
*/
#define LOG_VALID                   0x01

/*
** Define levels above which a log entry will be classified as either an error,
** warning, or debug. Anything below these levels will be classified as info.
*/
#define LE_WARNING_LEVEL    0x200
#define LE_ERROR_LEVEL      0x400
#define LE_DEBUG_LEVEL      0x4000

/*
** Define the different classes of log events
*/
#define LOG_TYPE_INFO        0
#define LOG_TYPE_WARNING     1
#define LOG_TYPE_ERROR       2
#define LOG_TYPE_DEBUG       3
#define LOG_TYPE_FATAL       4


/*
** Logging MRP header
*/
typedef struct logMRP_t
{
    UINT32      mleEvent;
    UINT32      mleLength;
    UINT32      mleData[256];
} LOG_MRP;

typedef struct
{
    UINT16      year;           /* Year 0 -9999                        */
    UINT8       month;          /* Month 1 -12                         */
    UINT8       date;           /* Day of the month 1 - 31             */
    UINT8       day;            /* Day of the week 1 - 7 (1 = Sunday)  */
    UINT8       hours;          /* Hour 0 - 23     (0 = midnight)      */
    UINT8       minutes;        /* Minutes 0 - 59                      */
    UINT8       seconds;        /* Seconds 0 - 59                      */
} LOGTIME;

/*
** Log Entry Header
*/
typedef struct LOG_HDR
{
    union
    {
        struct
        {
            UINT16      flags;
            UINT16      reserved;
        } status;
        UINT32      statusWord;
    } le;

    UINT32      masterSequence;
    UINT32      sequence;
    UINT16      length;
    UINT16      eventCode;
    UINT32      timeStamp;
} LOG_HDR;

typedef struct
{
    UINT32      validWord;
    LOG_HDR     header;
} LOG_SECTOR;

/*
** Log Information structure
*/
typedef struct
{
    UINT32      startAddress;   /* Flash Address where logs start       */
    UINT32      startSector;    /* Starting flash sector                */
    LOG_SECTOR  *currPtr;       /* Pointer to current log sector        */
    UINT32      sectorSize;     /* Log Sector size in bytes             */
    UINT32      sectorOffset;   /* Sector offset of current entry       */
    UINT16      rsvd1;          /* RESERVED                             */
    UINT16      unAckErrorCount;        /* Unacknowledged Error Counter         */
    UINT16      unAckWarningCount;      /* Unacknowledged Warning Counter       */
    UINT16      unAckInfoCount; /* Unacknowledged Info Counter          */
    UINT8       numSectors;     /* Number of logging flash sectors      */
    UINT8       startSectNum;   /* sector number of start sector        */
    UINT8       currSectNum;    /* sector number of current sector      */
    UINT8       rsvd2;          /* RESERVED                             */
} LOG_INFO_CB;

/*
** Log ptr data (for display)
*/
typedef struct
{
    char        *sectPtr;
    UINT32      sectorOffset;
    LOG_HDR     *logPtr;
    UINT8       sectorNum;
    LOG_INFO_CB *logInfoPtr;
} LOG_PTR_DATA;

/*
** Define the different classes of MMC log events
*/
#define MMC_LOG_TYPE_INFO       0
#define MMC_LOG_TYPE_WARNING    1
#define MMC_LOG_TYPE_ERROR      2
#define MMC_LOG_TYPE_FATAL      3

/*
** Define the different classes of MMC extended log defs
**
** eType is generated by adding 1 (or more) base type with 1 extended type.
** For example if you have a fan environmental error message, the
** etype would by 0x20 + 0x08 = 0x28
*/

/* Base eType constants */
#define MMC_ELOG_NORMAL         0x00
#define MMC_ELOG_FATAL_BIT      0x80
#define MMC_ELOG_POWERUP_BIT    0x40
#define MMC_ELOG_ENV_BIT        0x20

/* Extended eType constants */
#define MMC_ELOG_USER_MSG       0
#define MMC_ELOG_PS             1
#define MMC_ELOG_HAB            2
#define MMC_ELOG_UPS            3
#define MMC_ELOG_DEBUG          4
#define MMC_ELOG_DIAG           5
#define MMC_ELOG_SCSI           6
#define MMC_ELOG_MEMPROB        7
#define MMC_ELOG_FAN            8
#define MMC_ELOG_TEMP           9
#define MMC_ELOG_PRED           10
#define MMC_ELOG_CONTROLLER     14

#define MMC_MESSAGE_SIZE    40

/*****************************************************************************
** Public variables
*****************************************************************************/


/*****************************************************************************
** Public function prototypes
*****************************************************************************/

extern void InitLogs(void);
extern void GetLogPtrList(LOG_HDR **listPtr, LOG_HDR *logPtr, LOG_INFO_CB *logInfo, int count);
extern LOG_HDR *GetFirstLogEntry(LOG_PTR_DATA *, LOG_INFO_CB *);
extern int ValidLogEntry(LOG_HDR *);
#define FindCustomerLogEntryBySequence(logSearchNum) \
                (FindLogEntryBySequence(logSearchNum, &cLogInfo))
extern LOG_HDR *FindLogEntryBySequence(UINT32 logSearchNum, LOG_INFO_CB *);
LOG_HDR *FindLogEntryByMasterSequence(UINT32 logSearchSeqNum, LOG_INFO_CB *);
extern UINT8 GetEventType(UINT16 eventCode);
extern UINT32 GetSequenceNumber(LOG_HDR *);
extern UINT32 GetMasterSequenceNumber(LOG_HDR *);

extern void GetStatusString(UINT16 status, UINT16 eventCode, char *strPtr);
extern UINT32 LogInfoRequest(UINT8 *, UINT32 size, UINT16 mode,
            UINT32 *eventCount, UINT32 *sequenceNumber, UINT32 *responseLength);
extern LOG_HDR *GetLogEntry(UINT32 seq);
extern UINT32 GetNextLogSequence(void);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _LOGGING_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
