/* $Id: FCM_Counters.h 122127 2010-01-06 14:04:36Z m4 $ */
/*============================================================================
** FILE NAME:       FCM_Counters.h
** MODULE TITLE:    Header file for the FCM_Counters component.
**
** DESCRIPTION:     Fibre Channel Monitor - Drive counter tracking
**
** Copyright (c) 2001-2009  Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _FCM_COUNTERS_H_
#define _FCM_COUNTERS_H_

#include "mutex.h"
#include "XIO_Const.h"
#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/
#define FCM_COUNTER_MAP_VERSION     1
#define FCM_COUNTER_BACKUP_LISTS    4

#define FCM_EURO_NUM_BAY_PORTS      2
#define FCM_NUM_BAY_PORTS           4

/* Slot data */
typedef struct
{
    UINT32      drivePresent:1;
    UINT32      countersValid:1;
    UINT32      reserved:30;
} FCM_SLOT_FLAGS_BITS;

typedef union
{
    UINT32      value;
    FCM_SLOT_FLAGS_BITS bits;
} FCM_SLOT_FLAGS;

typedef struct
{
    UINT32      linkFail;
    UINT32      lostSync;
    UINT32      invalidTransmit;
    UINT32      invalidCRC;
    UINT32      lipF7Initiated;
    UINT32      lipF7Received;
    UINT32      lipF8Initiated;
    UINT32      lipF8Received;
    UINT32      WordErrorCount;
    UINT32      CRCErrorCount;
    INT32       ClockDelta;
    UINT32      LoopUpCount;
    UINT32      InsertionCount;
    UINT32      StallCount;
    UINT32      Utilization;
} FCM_FCAL_ERROR_COUNTERS;

typedef struct
{
    FCM_FCAL_ERROR_COUNTERS portA;
    FCM_FCAL_ERROR_COUNTERS portB;
    UINT32      powerOnMinutes;
    UINT8       commandInitiatePort;
} FCM_SLOT_DATA_COUNTERS;

typedef struct
{
    UINT32      linkFail;
    UINT32      lostSync;
    UINT32      lostSignal;
    UINT32      sequenceError;
    UINT32      invalidTransmit;
    UINT32      invalidCRC;
    UINT32      WordErrorCount;
    UINT32      CRCErrorCount;
    INT32       ClockDelta;
    UINT32      LoopUpCount;
    UINT32      InsertionCount;
    UINT32      StallCount;
    UINT32      Utilization;
} FCM_BAY_PORT_ERROR_COUNTERS;

typedef struct
{
    FCM_BAY_PORT_ERROR_COUNTERS port[FCM_NUM_BAY_PORTS][2];
} FCM_BAY_ERROR_COUNTERS;

typedef struct
{
    FCM_SLOT_FLAGS flags;
    FCM_SLOT_DATA_COUNTERS counters;
    UINT64      wwn;
    UINT8       channel;
    UINT8       devName[4];
} FCM_SLOT_DATA;

typedef struct
{
    UINT16      bayId;
    UINT8       type;
    UINT8       valid;
    UINT64      wwn;
} FCM_BAY_INFO;

/* Bay data */
typedef struct
{
    UINT32      bayPresent:1;
    UINT32      portsUnknown:1;
    UINT32      ports0and1:1;
    UINT32      ports2and3:1;
    UINT32      counterValid:1;
    UINT32      reserved:27;
} FCM_BAY_FLAGS_BITS;

typedef union
{
    UINT32      value;
    FCM_BAY_FLAGS_BITS bits;
} FCM_BAY_FLAGS;

typedef struct
{
    FCM_BAY_FLAGS flags;
    FCM_SLOT_DATA slotDataList[MAX_DISK_BAY_SLOTS];
    FCM_BAY_INFO bayInfo;
    FCM_BAY_ERROR_COUNTERS errorCounts;
} FCM_BAY_DATA;

/* HAB error data */
typedef struct
{
    UINT32      linkFail;
    UINT32      lostSync;
    UINT32      lostSignal;
    UINT32      sequenceError;
    UINT32      invalidTransmit;
    UINT32      invalidCRC;
} FCM_HAB_ERROR_COUNTERS;

typedef struct
{
    UINT32      habPresent:1;
    UINT32      countersValid:1;
    UINT32      reserved:30;
} FCM_HAB_FLAGS_BITS;

typedef union
{
    UINT32      value;
    FCM_HAB_FLAGS_BITS bits;
} FCM_HAB_FLAGS;

typedef struct
{
    FCM_HAB_FLAGS flags;
    FCM_HAB_ERROR_COUNTERS habCounters;
} FCM_HAB_DATA;

/* Overall FCM error data */
typedef struct
{
    TIMESTAMP   beginTimestamp;
    TIMESTAMP   endTimestamp;
    FCM_HAB_DATA *habDataList;
    FCM_BAY_DATA *bayDataList;
} FCM_ERROR_DATA;

/* Counter map data */
typedef struct
{
    UINT32      baselineMapValid:1;
    UINT32      deltaMapValid:1;
    UINT32      updateMapValid:1;
    UINT32      backup0MapValid:1;
    UINT32      backup1MapValid:1;
    UINT32      backup2MapValid:1;
    UINT32      backup3MapValid:1;
    UINT32      backupMapIndex:2;
    UINT32      reserved:23;
} FCM_COUNTER_MAP_FLAGS_BITS;

typedef union
{
    UINT32      value;
    FCM_COUNTER_MAP_FLAGS_BITS bits;
} FCM_COUNTER_MAP_FLAGS;

typedef struct
{
    UINT32      version;
    MUTEX       busyMutex;
    UINT32      numberOfBackEndHabs;
    UINT32      numberOfBays;
    UINT32      numberOfSlotsInBay;

    /* Substructure sizes */
    UINT32      sizeCounterMap;
    UINT32      sizeErrorData;
    UINT32      sizeHabData;
    UINT32      sizeBayData;
} FCM_COUNTER_MAP_HEADER;

typedef struct
{
    FCM_COUNTER_MAP_HEADER header;
    FCM_COUNTER_MAP_FLAGS flags;
    FCM_ERROR_DATA baselineData;        /* Hack to get around 64k COFF limitation */
    FCM_ERROR_DATA updateData;  /* Hack to get around 64k COFF limitation */
    FCM_ERROR_DATA deltaData;   /* Hack to get around 64k COFF limitation */

    FCM_ERROR_DATA backupData0; /* Hack to get around 64k COFF limitation */
    FCM_ERROR_DATA backupData1; /* Hack to get around 64k COFF limitation */
    FCM_ERROR_DATA backupData2; /* Hack to get around 64k COFF limitation */
    FCM_ERROR_DATA backupData3; /* Hack to get around 64k COFF limitation */
} FCM_COUNTER_MAP;


#define FCM_COUNTER_HAB_DATA_SIZE   sizeof(habDataList0)
#define FCM_COUNTER_BAY_DATA_SIZE   sizeof(bayDataList0)

#define FCM_COUNTER_ERROR_DATA_SIZE ((2 * sizeof(TIMESTAMP))               +   \
                                     FCM_COUNTER_HAB_DATA_SIZE             +   \
                                     FCM_COUNTER_BAY_DATA_SIZE)

#define FCM_COUNTER_MAP_SIZE        (sizeof(FCM_COUNTER_MAP_HEADER)        +   \
                                     sizeof(FCM_COUNTER_MAP_FLAGS)         +   \
                                     (7 * FCM_COUNTER_ERROR_DATA_SIZE))


typedef struct
{
    UINT8       pgaeCode;
    UINT8       reserved;
    UINT16      pageLength;
} FMC_LOG_SENSE_HEADER;

typedef struct
{
    UINT64      timeSinceReset;
    UINT64      transmittedFrames;
    UINT64      receivedFrames;
    UINT64      trasmittedWords;
    UINT64      receivedWords;
    UINT64      lipCount;
    UINT64      nosCount;
    UINT64      errorFrames;
    UINT64      dumpedFrames;
    UINT64      linkFailureCount;
    UINT64      lossOfSyncCount;
    UINT64      lossOfSignalCount;
    UINT64      primitiveSequenceErrorCount;
    UINT64      invalidTransmittedWordCount;
    UINT64      invalidCRCCount;
    UINT64      fcgInitiatorIOCount;
} FMC_ARIO_LOG_SENSE_DATA;

typedef struct
{
    FMC_LOG_SENSE_HEADER logSenseHeader;
    FMC_ARIO_LOG_SENSE_DATA logSenseDataPort[2];
} FCM_ARIO_LOG_SENSE;

/*****************************************************************************
** Public variables
*****************************************************************************/
extern FCM_COUNTER_MAP counterMap;

/*****************************************************************************
** Public function prototypes
*****************************************************************************/
extern UINT32 FCM_CountersGetBinaryData(UINT8 *bufferPtr, UINT32 bufferSize);
extern void FCM_CountersMajorStorageEvent(void);
extern void FCM_CountersMinorStorageEvent(void);

extern UINT32 FCM_CountersBaselineMap(FCM_COUNTER_MAP *counterMapPtr);
extern UINT32 FCM_CountersDeltaMap(FCM_COUNTER_MAP *counterMapPtr);
extern UINT32 FCM_CountersUpdateMap(FCM_COUNTER_MAP *counterMapPtr);
extern UINT32 FCM_CountersDumpMap(FCM_COUNTER_MAP *counterMapPtr);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _FCM_COUNTERS_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
