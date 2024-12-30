/* $Id: FCM_Counters.c 160482 2013-01-28 14:02:08Z marshall_midden $ */
/*============================================================================
** FILE NAME:       FCM_Counters.c
** MODULE TITLE:    Fibre Channel Health Monitor - Drive counter mapping
**
** Copyright (c) 2001-2010 XIOtech Corporation. All rights reserved.
**==========================================================================*/
#include "FCM_Counters.h"
#include "CachePDisk.h"
#include "CacheBay.h"
#include "convert.h"
#include "cps_init.h"
#include "debug_files.h"
#include "LargeArrays.h"
#include "misc.h"
#include "pcb.h"
#include "PI_PDisk.h"
#include "PktCmdHdl.h"
#include "rtc.h"
#include "XIO_Macros.h"
#include "XIO_Std.h"
#include "EL.h"

#include <byteswap.h>


/*****************************************************************************
** Private defines
*****************************************************************************/
#define COUNTER_STRING_LENGTH               15
#define COUNTER_MAJOR_EVENT_DELAY_MSEC      60000       /* 1 minute delay   */
#define COUNTER_MINOR_EVENT_DELAY_MSEC      300000      /* 5 minute delay   */

/* FCM task flag macros */
#define FCM_STARTING                        ( 1 << 0 )
#define FCM_IN_PROGRESS                     ( 1 << 1 )
#define FCM_ENDING                          ( 1 << 2 )
#define FCM_UPDATE_ABORT                    ( 1 << 3 )
#define FCM_MINOR_EVENT                     ( 1 << 4 )

static UINT32 gFCMUpdateTaskInProgressCount = 0;        /* Need to verify task started. */
#define SetFCMUpdateTaskStartingFlag(x)     ( (x == TRUE) ? (fcmUpdateTaskFlags |= FCM_STARTING) : (fcmUpdateTaskFlags &= ~FCM_STARTING) )
#define SetFCMUpdateTaskInProgressFlag(x)   ( (x == TRUE) ? (fcmUpdateTaskFlags |= FCM_IN_PROGRESS) : (fcmUpdateTaskFlags &= ~FCM_IN_PROGRESS) )
#define SetFCMUpdateTaskEndingFlag(x)       ( (x == TRUE) ? (fcmUpdateTaskFlags |= FCM_ENDING) : (fcmUpdateTaskFlags &= ~FCM_ENDING) )
#define SetFCMUpdateTaskAbortFlag(x)        ( (x == TRUE) ? (fcmUpdateTaskFlags |= FCM_UPDATE_ABORT) : (fcmUpdateTaskFlags &= ~FCM_UPDATE_ABORT) )
#define SetFCMMinorEvent(x)                 ( (x == TRUE) ? (fcmUpdateTaskFlags |= FCM_MINOR_EVENT) : (fcmUpdateTaskFlags &= ~FCM_MINOR_EVENT) )

#define TestFCMUpdateTaskStartingFlag()     ( (fcmUpdateTaskFlags & FCM_STARTING) ? TRUE : FALSE )
#define TestFCMUpdateTaskInProgressFlag()   ( (fcmUpdateTaskFlags & FCM_IN_PROGRESS) ? TRUE : FALSE )
#define TestFCMUpdateTaskEndingFlag()       ( (fcmUpdateTaskFlags & FCM_ENDING) ? TRUE : FALSE )
#define TestFCMUpdateTaskAbortFlag()        ( (fcmUpdateTaskFlags & FCM_UPDATE_ABORT) ? TRUE : FALSE )
#define TestFCMMinorEvent()                 ( (fcmUpdateTaskFlags & FCM_MINOR_EVENT) ? TRUE : FALSE )

#define SCSI_SES_DIAGNOSTIC_PAGE_80 "\x1c\x01\x80\x01\x00\x00"
#define SCSI_SES_DIAGNOSTIC_PAGE_81 "\x1c\x01\x81\x01\x00\x00"

/*****************************************************************************
** Private variables
*****************************************************************************/
static UINT32 fcmUpdateTaskFlags = 0;   /* Variable to retain the task flags    */
static PCB *majorEventTaskPcbPtr = NULL;
static PCB *minorEventTaskPcbPtr = NULL;
static PCB *updateTaskPcbPtr = NULL;

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static UINT32 FCM_CountersDoUpdate(void);
static void FCM_CountersUpdateTask(TASK_PARMS *parms);
static void FCM_CountersUpdateTaskAbort(void);
static UINT32 FCM_CountersUpdateTaskStart(void);

static void FCM_CountersMajorStorageEventTask(TASK_PARMS *parms);
static void FCM_CountersMinorStorageEventTask(TASK_PARMS *parms);

static UINT32 FCM_CountersUpdateHabDataList(FCM_HAB_DATA *habDataList);
static UINT32 FCM_CountersUpdateBayDataList(FCM_BAY_DATA *bayDataList);
static UINT32 FCM_CountersParseLogSense(FCM_SLOT_DATA_COUNTERS *countersPtr,
                                        UINT8 *logSensePtr, UINT32 dataLength);
static UINT32 FCM_CountersParseArioLogSense(FCM_BAY_ERROR_COUNTERS *countersPtr,
                                            UINT8 *packetPtr, UINT32 dataLength);
static UINT32 FCM_CountersParseFactoryLog(FCM_SLOT_DATA_COUNTERS *countersPtr,
                                          UINT8 *factoryLogPtr, UINT32 dataLength);
static UINT32 FCM_CountersInitializeErrorData(FCM_ERROR_DATA *errorDataPtr);
static UINT32 FCM_CountersInitializeHabDataList(FCM_HAB_DATA *habDataListPtr);
static UINT32 FCM_CountersInitializeHabData(FCM_HAB_DATA *habDataPtr);
static UINT32 FCM_CountersInitializeBayDataList(FCM_BAY_DATA *bayDataListPtr);
static UINT32 FCM_CountersInitializeBayData(FCM_BAY_DATA *bayDataPtr);
static UINT32 FCM_CountersInitializeBayInfo(FCM_BAY_INFO *infoPtr);
static UINT32 FCM_CountersInitializeBayErrorCounters(FCM_BAY_ERROR_COUNTERS *countersPtr);
static UINT32 FCM_CountersInitializeSlotData(FCM_SLOT_DATA *slotDataPtr);
static UINT32 FCM_CountersInitializeSlotDataCounters(FCM_SLOT_DATA_COUNTERS *countersPtr);
static UINT32 FCM_CountersDumpErrorData(FCM_ERROR_DATA *errorDataPtr);
static UINT32 FCM_CountersDumpHabDataList(FCM_HAB_DATA *habDataListPtr);
static UINT32 FCM_CountersDumpBayDataList(FCM_BAY_DATA *bayDataListPtr);
static void FCM_CountersToString(char *stringPtr, UINT32 counterValue,
                                 UINT8 stringLength);
static void FCM_SignedCounterToString(char *stringPtr, INT32 counterValue,
                                      UINT8 stringLength);

static UINT32 FCM_CountersDeltaHabDataList(FCM_HAB_DATA *oldHabDataList,
                                           FCM_HAB_DATA *newHabDataList,
                                           FCM_HAB_DATA *deltaHabDataList);
static UINT32 FCM_CountersDeltaHabData(FCM_HAB_DATA *oldHabDataPtr,
                                       FCM_HAB_DATA *newHabDataPtr,
                                       FCM_HAB_DATA *deltaHabDataPtr);
static UINT32 FCM_CountersDeltaBayDataList(FCM_BAY_DATA *oldBayDataList,
                                           FCM_BAY_DATA *newBayDataList,
                                           FCM_BAY_DATA *deltaBayDataList);
static UINT32 FCM_CountersDeltaBayData(FCM_BAY_DATA *oldBayDataPtr,
                                       FCM_BAY_DATA *newBayDataPtr,
                                       FCM_BAY_DATA *deltaBayDataPtr);
static UINT32 FCM_CountersDeltaBayInfo(FCM_BAY_INFO *oldInfoPtr,
                                       FCM_BAY_INFO *newInfoPtr,
                                       FCM_BAY_INFO *deltaInfoPtr);
static UINT32 FCM_CountersDeltaBayErrorCounters(UINT8 type,
                                                FCM_BAY_ERROR_COUNTERS *oldBayDataPtr,
                                                FCM_BAY_ERROR_COUNTERS *newBayDataPtr,
                                                FCM_BAY_ERROR_COUNTERS *deltaBayDataPtr);
static UINT32 FCM_CountersDeltaSlotData(UINT32 slotCounter, FCM_BAY_DATA *oldBayPtr,
                                        FCM_BAY_DATA *newBayPtr,
                                        FCM_BAY_DATA *deltaBayPtr);
static UINT32 FCM_CountersDeltaSlotDataCounters(UINT32 slotCounter,
                                                FCM_BAY_DATA *oldCountersPtr,
                                                FCM_BAY_DATA *newCountersPtr,
                                                FCM_BAY_DATA *deltaCountersPtr);
static UINT32 FCM_CountersDeltaCalculateValue(UINT32 oldValue, UINT32 newValue);
static INT32 FCM_CountersDeltaCalculateSignedValue(INT32 oldValue, INT32 newValue);
static UINT32 FCM_CountersDeltaBackup(FCM_COUNTER_MAP *counterMapPtr);

static UINT32 FCM_Xyratex_Parse(int, FCM_BAY_DATA *, UINT8 *packetPtr, UINT32);

/*****************************************************************************
** Public variables defined in header file.
*****************************************************************************/
FCM_COUNTER_MAP counterMap = {
    /* FCM_COUNTER_MAP_HEADER header */
    {
     FCM_COUNTER_MAP_VERSION,   /* version                              */
     Unlocked,                  /* busyMutex                            */
     MAX_BE_PORTS,              /* numberOfBackEndHabs                  */
     MAX_DISK_BAYS,             /* numberOfBays                         */
     MAX_DISK_BAY_SLOTS,        /* numberOfSlotsInBay                   */

     FCM_COUNTER_MAP_SIZE,      /* sizeCounterMap                       */
     FCM_COUNTER_ERROR_DATA_SIZE,       /* sizeErrorData                        */
     FCM_COUNTER_HAB_DATA_SIZE, /* sizeHabData                          */
     FCM_COUNTER_BAY_DATA_SIZE, /* sizeBayData                          */
     },

    /* FCM_COUNTER_MAP_FLAGS flags */
    {                           /* FCM_COUNTER_MAP_FLAGS_BITS bits */
     0                          /* value                                */
     },

    /* FCM_ERROR_DATA *baselineData */
    {
     {0, 0, 0, 0, 0, 0, 0, 0},  /* beginTimestamp                       */
     {0, 0, 0, 0, 0, 0, 0, 0},  /* endTimestamp                         */
     habDataList0,              /* FCM_HAB_DATA *habDataList            */
     bayDataList0,              /* FCM_BAY_DATA *bayDataList            */
     },

    /* FCM_ERROR_DATA *updateBayData */
    {
     {0, 0, 0, 0, 0, 0, 0, 0},  /* beginTimestamp                       */
     {0, 0, 0, 0, 0, 0, 0, 0},  /* endTimestamp                         */
     habDataList1,              /* FCM_HAB_DATA *habDataList            */
     bayDataList1,              /* FCM_BAY_DATA *bayDataList            */
     },

    /* FCM_ERROR_DATA *deltaData */
    {
     {0, 0, 0, 0, 0, 0, 0, 0},  /* beginTimestamp                       */
     {0, 0, 0, 0, 0, 0, 0, 0},  /* endTimestamp                         */
     habDataList2,              /* FCM_HAB_DATA *habDataList            */
     bayDataList2,              /* FCM_BAY_DATA *bayDataList            */
     },

    /* FCM_ERROR_DATA *backupData0 */
    {
     {0, 0, 0, 0, 0, 0, 0, 0},  /* beginTimestamp                       */
     {0, 0, 0, 0, 0, 0, 0, 0},  /* endTimestamp                         */
     backupHabDataList0,        /* FCM_HAB_DATA *habDataList            */
     backupBayDataList0,        /* FCM_BAY_DATA *bayDataList            */
     },

    /* FCM_ERROR_DATA *backupData1 */
    {
     {0, 0, 0, 0, 0, 0, 0, 0},  /* beginTimestamp                       */
     {0, 0, 0, 0, 0, 0, 0, 0},  /* endTimestamp                         */
     backupHabDataList1,        /* FCM_HAB_DATA *habDataList            */
     backupBayDataList1,        /* FCM_BAY_DATA *bayDataList            */
     },

    /* FCM_ERROR_DATA *backupData2 */
    {
     {0, 0, 0, 0, 0, 0, 0, 0},  /* beginTimestamp                       */
     {0, 0, 0, 0, 0, 0, 0, 0},  /* endTimestamp                         */
     backupHabDataList2,        /* FCM_HAB_DATA *habDataList            */
     backupBayDataList2,        /* FCM_BAY_DATA *bayDataList            */
     },

    /* FCM_ERROR_DATA *backupData3 */
    {
     {0, 0, 0, 0, 0, 0, 0, 0},  /* beginTimestamp                       */
     {0, 0, 0, 0, 0, 0, 0, 0},  /* endTimestamp                         */
     backupHabDataList3,        /* FCM_HAB_DATA *habDataList            */
     backupBayDataList3,        /* FCM_BAY_DATA *bayDataList            */
     }
};

/*****************************************************************************
** Public function prototypes
*****************************************************************************/

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
**  Function Name: FCM_CountersGetBinaryData
**
**  Comments:   This is the interfacing function to the rest of the code.
**              This function copies the counterMap into the specified buffer.
**
**  Parameters: bufferPtr  -
**              bufferSize -
**
**  Returns:    Number of bytes copied into buffer
**--------------------------------------------------------------------------*/
UINT32 FCM_CountersGetBinaryData(UINT8 *bufferPtr, UINT32 bufferSize)
{
    UINT8      *startingBufferPtr = bufferPtr;

    if (bufferSize >= FCM_COUNTER_MAP_SIZE)
    {
        /*
         * Wait for any update already in progress to complete
         */
        while ((TestFCMUpdateTaskStartingFlag() == TRUE) ||
               (TestFCMUpdateTaskInProgressFlag() == TRUE))
        {
            TaskSleepMS(20);
        }

        /*
         * Acquire the mutex
         */
        (void)LockMutex(&counterMap.header.busyMutex, MUTEX_WAIT);

        dprintf(DPRINTF_FCM, "GBD: Copying binary FCM counter data\n");

        /* counterMap */
        memcpy(bufferPtr, &counterMap.header, sizeof(counterMap.header));
        bufferPtr += sizeof(counterMap.header);

        memcpy(bufferPtr, &counterMap.flags, sizeof(counterMap.flags));
        bufferPtr += sizeof(counterMap.flags);

        /* baselineData */
        memcpy(bufferPtr, &counterMap.baselineData.beginTimestamp,
               sizeof(counterMap.baselineData.beginTimestamp));
        bufferPtr += sizeof(counterMap.baselineData.beginTimestamp);

        memcpy(bufferPtr, &counterMap.baselineData.endTimestamp,
               sizeof(counterMap.baselineData.endTimestamp));
        bufferPtr += sizeof(counterMap.baselineData.endTimestamp);

        memcpy(bufferPtr, counterMap.baselineData.habDataList, sizeof(habDataList0));
        bufferPtr += sizeof(habDataList0);

        memcpy(bufferPtr, counterMap.baselineData.bayDataList, sizeof(bayDataList0));
        bufferPtr += sizeof(bayDataList0);

        /* updateData */
        memcpy(bufferPtr, &counterMap.updateData.beginTimestamp,
               sizeof(counterMap.updateData.beginTimestamp));
        bufferPtr += sizeof(counterMap.updateData.beginTimestamp);

        memcpy(bufferPtr, &counterMap.updateData.endTimestamp,
               sizeof(counterMap.updateData.endTimestamp));
        bufferPtr += sizeof(counterMap.updateData.endTimestamp);

        memcpy(bufferPtr, counterMap.updateData.habDataList, sizeof(habDataList0));
        bufferPtr += sizeof(habDataList0);

        memcpy(bufferPtr, counterMap.updateData.bayDataList, sizeof(bayDataList0));
        bufferPtr += sizeof(bayDataList0);

        /* deltaData */
        memcpy(bufferPtr, &counterMap.deltaData.beginTimestamp,
               sizeof(counterMap.deltaData.beginTimestamp));
        bufferPtr += sizeof(counterMap.deltaData.beginTimestamp);

        memcpy(bufferPtr, &counterMap.deltaData.endTimestamp,
               sizeof(counterMap.deltaData.endTimestamp));
        bufferPtr += sizeof(counterMap.deltaData.endTimestamp);

        memcpy(bufferPtr, counterMap.deltaData.habDataList, sizeof(habDataList0));
        bufferPtr += sizeof(habDataList0);

        memcpy(bufferPtr, counterMap.deltaData.bayDataList, sizeof(bayDataList0));
        bufferPtr += sizeof(bayDataList0);

        /* backupData0 */
        memcpy(bufferPtr, &counterMap.backupData0.beginTimestamp,
               sizeof(counterMap.backupData0.beginTimestamp));
        bufferPtr += sizeof(counterMap.backupData0.beginTimestamp);

        memcpy(bufferPtr, &counterMap.backupData0.endTimestamp,
               sizeof(counterMap.backupData0.endTimestamp));
        bufferPtr += sizeof(counterMap.backupData0.endTimestamp);

        memcpy(bufferPtr, counterMap.backupData0.habDataList, sizeof(habDataList0));
        bufferPtr += sizeof(habDataList0);

        memcpy(bufferPtr, counterMap.backupData0.bayDataList, sizeof(bayDataList0));
        bufferPtr += sizeof(bayDataList0);

        /* backupData1 */
        memcpy(bufferPtr, &counterMap.backupData1.beginTimestamp,
               sizeof(counterMap.backupData1.beginTimestamp));
        bufferPtr += sizeof(counterMap.backupData1.beginTimestamp);

        memcpy(bufferPtr, &counterMap.backupData1.endTimestamp,
               sizeof(counterMap.backupData1.endTimestamp));
        bufferPtr += sizeof(counterMap.backupData1.endTimestamp);

        memcpy(bufferPtr, counterMap.backupData1.habDataList, sizeof(habDataList0));
        bufferPtr += sizeof(habDataList0);

        memcpy(bufferPtr, counterMap.backupData1.bayDataList, sizeof(bayDataList0));
        bufferPtr += sizeof(bayDataList0);

        /* backupData2 */
        memcpy(bufferPtr, &counterMap.backupData2.beginTimestamp,
               sizeof(counterMap.backupData2.beginTimestamp));
        bufferPtr += sizeof(counterMap.backupData2.beginTimestamp);

        memcpy(bufferPtr, &counterMap.backupData2.endTimestamp,
               sizeof(counterMap.backupData2.endTimestamp));
        bufferPtr += sizeof(counterMap.backupData2.endTimestamp);

        memcpy(bufferPtr, counterMap.backupData2.habDataList, sizeof(habDataList0));
        bufferPtr += sizeof(habDataList0);

        memcpy(bufferPtr, counterMap.backupData2.bayDataList, sizeof(bayDataList0));
        bufferPtr += sizeof(bayDataList0);

        /* backupData3 */
        memcpy(bufferPtr, &counterMap.backupData3.beginTimestamp,
               sizeof(counterMap.backupData3.beginTimestamp));
        bufferPtr += sizeof(counterMap.backupData3.beginTimestamp);

        memcpy(bufferPtr, &counterMap.backupData3.endTimestamp,
               sizeof(counterMap.backupData3.endTimestamp));
        bufferPtr += sizeof(counterMap.backupData3.endTimestamp);

        memcpy(bufferPtr, counterMap.backupData3.habDataList, sizeof(habDataList0));
        bufferPtr += sizeof(habDataList0);

        memcpy(bufferPtr, counterMap.backupData3.bayDataList, sizeof(bayDataList0));
        bufferPtr += sizeof(bayDataList0);

        /*
         * Release the mutex
         */
        UnlockMutex(&counterMap.header.busyMutex);

        dprintf(DPRINTF_FCM, "GBD: Wrote %u bytes into %u space\n",
                (bufferPtr - startingBufferPtr), bufferSize);
    }
    else
    {
        dprintf(DPRINTF_FCM, "GBD: Buffer too smalll - getting binary FCM counter data\n");
        dprintf(DPRINTF_FCM, "GBD:   Need: %u bytes    Got: %u bytes\n",
                FCM_COUNTER_MAP_SIZE, bufferSize);
    }

    return (bufferPtr - startingBufferPtr);
}


/*----------------------------------------------------------------------------
**  Function Name: FCM_CountersMajorStorageEvent
**
**  Comments:   This is the interfacing function to the rest of the code.
**              It can be called from multiple sources, and it also performs
**              task switches, so it needs to be reentrant.
**
**  Parameters: None
**
**  Returns:    GOOD  - Event processed
**              ERROR - Event not processed
**--------------------------------------------------------------------------*/
void FCM_CountersMajorStorageEvent(void)
{

    dprintf(DPRINTF_DEFAULT, "MaSE: Major storage FCAL event\n");

    /*
     * If the baseline update task is pending, then discard this event, as we're
     * still waiting for things to settle out after the previous major event.
     */
    if (majorEventTaskPcbPtr == NULL)
    {
        TaskCreate(FCM_CountersMajorStorageEventTask, NULL);
        TaskSwitch();
    }
    else
    {
        dprintf(DPRINTF_FCM, "MiSE: MinorStorageEvent task already running\n");

        /*
         * Before resetting the delay timer, make sure the updateTask is
         * still in the 'wait for timer' state.
         */
        if (TaskGetState(majorEventTaskPcbPtr) == PCB_TIMED_WAIT)
        {
            /*
             * Reset the baseline update delay timer
             */
            majorEventTaskPcbPtr->pc_time = COUNTER_MAJOR_EVENT_DELAY_MSEC;
            dprintf(DPRINTF_FCM, "MiSE: Extending MinorStorageEvent sleep duration\n");
        }
    }
}


/*----------------------------------------------------------------------------
**  Function Name: FCM_CountersMajorStorageEventTask
**
**  Comments:   This function is not to be called!  This is a task, and needs
**              to be forked in order to run correctly.
**
**  Parameters: None
**
**  Returns:    Nothing
**--------------------------------------------------------------------------*/
static void FCM_CountersMajorStorageEventTask(UNUSED TASK_PARMS *parms)
{
    UINT32      countersUpdated = FALSE;

    /*
     * Store the a pointer to the current pcb into majorEventTaskPcbPtr.
     * This will allow the other tasks to adjust the sleep time (delay)
     * of this task directly.
     */
    majorEventTaskPcbPtr = XK_GetPcb();
    dprintf(DPRINTF_FCM, "MaSET: MajorStorageEventTask created\n");

    /*
     * Keep trying to update the baseline counters until it's successful,
     * but pause between update attempts.
     */
    while (countersUpdated == FALSE)
    {
        /*
         * Stop any updates that are already in progress.  The new baseline
         * that we'll be taking will make the data being collected useless, so
         * it's best that we stop sending out SCSI commands to get the counters.
         */
        if (TestFCMUpdateTaskInProgressFlag() == TRUE)
        {
            FCM_CountersUpdateTaskAbort();
        }

        /*
         * Go to sleep.  This will hopefully give the BE fibre time
         * to settle out, but this time could be extended by more incoming
         * requests to reset the baseline values.
         */
        dprintf(DPRINTF_FCM, "MaSET: MajorStorageEventTask going to sleep - wait for BE to settle\n");
        TaskSleepMS(COUNTER_MAJOR_EVENT_DELAY_MSEC);
        dprintf(DPRINTF_FCM, "MaSET: MajorStorageEventTask running\n");

        /*
         * If it's valid, save the current deltaMap into a backup array
         */
        FCM_CountersDeltaBackup(&counterMap);

        /*
         * Perform the counter update.  If the update completes without error,
         * then copy the new counters into the baselineMap.
         */
        if (FCM_CountersDoUpdate() == GOOD)
        {
            dprintf(DPRINTF_FCM, "MaSET: DoUpdate returned GOOD\n");

            /*
             * If the baseline is valid, then calculate the delta between the readings
             * that were just taken and the baseline readings.  If the baseline is not
             * valid, then make sure that the readings we just took are assigned to be
             * the baseline values.
             */
            if (counterMap.flags.bits.updateMapValid == TRUE)
            {
                dprintf(DPRINTF_FCM, "MaSET: DoUpdate returned GOOD and updateMap is valid\n");
                FCM_CountersBaselineMap(&counterMap);
                countersUpdated = TRUE;
            }
            else
            {
                dprintf(DPRINTF_FCM, "MaSET: DoUpdate returned GOOD but updateMap is not valid\n");
            }
        }
        else
        {
            dprintf(DPRINTF_FCM, "MaSET: DoUpdate returned ERROR\n");
        }
    }

    /*
     * Since we just updated the baseline, if a minor event is in the pipe,
     * extend the 'wait' delay of the minor event update task so that it
     * is able to get a meaningful delta timespan.
     */
    if ((minorEventTaskPcbPtr != NULL) &&
        (TaskGetState(minorEventTaskPcbPtr) == PCB_TIMED_WAIT))
    {
        /*
         * Reset the baseline update delay timer
         */
        dprintf(DPRINTF_FCM, "MaSET: Minor event update delay extended\n");
        minorEventTaskPcbPtr->pc_time = COUNTER_MINOR_EVENT_DELAY_MSEC;
    }

    /*
     * Reset the majorEventTaskPcbPtr, since it's shutting down
     */
    dprintf(DPRINTF_FCM, "MaSET: MajorStorageEventTask ending\n");
    majorEventTaskPcbPtr = NULL;
}


/*----------------------------------------------------------------------------
**  Function Name: FCM_CountersMinorStorageEvent
**
**  Comments:   This is the interfacing function to the rest of the code.
**              It can be called from multiple sources, and it also performs
**              task switches, so it needs to be reentrant.
**
**  Parameters: None
**
**  Returns:    GOOD  - Event processed
**              ERROR - Event not processed
**--------------------------------------------------------------------------*/
void FCM_CountersMinorStorageEvent(void)
{
    dprintf(DPRINTF_DEFAULT, "MiSE: Minor storage FCAL event\n");

    /*
     * Set the flag to indicate that a minor storage event has occurred
     */
    SetFCMMinorEvent(TRUE);

    /*
     * Spawn the event task if it's not already running
     */
    if (minorEventTaskPcbPtr == NULL)
    {
        TaskCreate(FCM_CountersMinorStorageEventTask, NULL);
        TaskSwitch();
    }
    else
    {
        dprintf(DPRINTF_FCM, "MiSE: MinorStorageEvent task already running\n");
    }
}


/*----------------------------------------------------------------------------
**  Function Name: FCM_CountersMinorStorageEventTask
**
**  Parameters: None
**
**  Returns:    Nothing
**--------------------------------------------------------------------------*/
static void FCM_CountersMinorStorageEventTask(UNUSED TASK_PARMS *parms)
{
    /*
     * Store the a pointer to the current pcb into minorEventTaskPcbPtr.
     */
    minorEventTaskPcbPtr = XK_GetPcb();
    dprintf(DPRINTF_FCM, "MiSET: MinorStorageEventTask created\n");

    /*
     * Keep trying to update the counters until it's successful, but pause
     * between update attempts.  Bail out if there's a major event that's
     * in progress.
     */
    while (TestFCMMinorEvent() == TRUE)
    {
        /*
         * If the major event task is running, then don't try to update
         * the events on a minor event.  Instead, go to sleep and update
         * the counters some time after the major event has been handled.
         */
        if (majorEventTaskPcbPtr == NULL)
        {
            dprintf(DPRINTF_FCM, "MiSET: MinorStorageEventTask running\n");

            /*
             * Clear the minor event flag
             */
            SetFCMMinorEvent(FALSE);

            if (FCM_CountersDoUpdate() == GOOD)
            {
                /*
                 * If the baseline is valid, then calculate the delta between the readings
                 * that were just taken and the baseline readings.  If the baseline is not
                 * valid, then make sure that the readings we just took are assigned to be
                 * the baseline values.
                 */
                if (counterMap.flags.bits.updateMapValid == TRUE)
                {
                    dprintf(DPRINTF_FCM, "MiSET: UpdateMap returned GOOD and updateMap is valid\n");

                    if (counterMap.flags.bits.baselineMapValid == TRUE)
                    {
                        FCM_CountersDeltaMap(&counterMap);
                    }
                    else
                    {
                        FCM_CountersBaselineMap(&counterMap);
                    }
                }
                else
                {
                    dprintf(DPRINTF_FCM, "MiSET: UpdateMap returned GOOD but updateMap is not valid\n");
                }
            }
        }
        else
        {
            dprintf(DPRINTF_FCM, "MiSET: Major event already in progress\n");
        }

        /*
         * Go to sleep for a while.  This prevents the code from updating
         * the counters on *every* minor event.  At most, the code will
         * update every COUNTER_MINOR_EVENT_DELAY_MSEC.
         */
        dprintf(DPRINTF_FCM, "MiSET: MinorStorageEventTask going to sleep\n");
        TaskSleepMS(COUNTER_MINOR_EVENT_DELAY_MSEC);
    }

    /*
     * Reset the minorEventTaskPcbPtr, since it's shutting down
     */
    dprintf(DPRINTF_FCM, "MiSET: MinorStorageEventTask exiting\n");
    SetFCMMinorEvent(FALSE);
    minorEventTaskPcbPtr = NULL;
}


/*----------------------------------------------------------------------------
**  Function Name: FCM_CountersDoUpdate
**
**  Parameters: None
**
**  Returns:    GOOD  - Update succeeded
**              ERROR - Update failed
**--------------------------------------------------------------------------*/
static UINT32 FCM_CountersDoUpdate(void)
{
    UINT32      returnCode = ERROR;

    dprintf(DPRINTF_FCM, "DU: Doing update\n");

    if (FCM_CountersUpdateTaskStart() == GOOD)
    {
        dprintf(DPRINTF_FCM, "DU: Update task started - waiting for completion\n");

        /*
         * Wait for the updateTask to complete
         */
        while (TestFCMUpdateTaskInProgressFlag() == TRUE)
        {
            TaskSleepMS(20);
        }

        /*
         * Check to see if the updateMap contains valid data
         */
        if (counterMap.flags.bits.updateMapValid == TRUE)
        {
            dprintf(DPRINTF_FCM, "DU: updateMap is valid\n");
            returnCode = GOOD;
        }
        else
        {
            dprintf(DPRINTF_FCM, "DU: updateMap is NOT valid\n");
        }
    }
    else
    {
        dprintf(DPRINTF_FCM, "DU: ERROR starting update task\n");
    }

    return (returnCode);
}


/*----------------------------------------------------------------------------
**  Function Name: FCM_CountersUpdateTaskStart
**
**  Comments:   This is the interfacing function to the rest of the code.
**              It can be called from multiple sources, and it also performs
**              task switches, so it needs to be reentrant.
**
**  Parameters: None
**
**  Returns:    GOOD  - Event processed
**              ERROR - Event not processed
**--------------------------------------------------------------------------*/
static UINT32 FCM_CountersUpdateTaskStart(void)
{
    UINT32      returnCode = GOOD;

    /*
     * Wait for the controller to reach the POWER_UP_COMPLETE state before
     * we start to issue commands to the BE
     */
    if (GetPowerUpState() == POWER_UP_COMPLETE)
    {
        /*
         * Check if another task is already attempting to start an update
         */
        if (TestFCMUpdateTaskStartingFlag() != TRUE)
        {
            /*
             * Set the starting flag
             */
            SetFCMUpdateTaskStartingFlag(TRUE);

            /*
             * Check if the FCMUpdateTask control task is already started before forking
             * the control task.
             */
            if (TestFCMUpdateTaskInProgressFlag() == FALSE)
            {
                /*
                 * Semaphore - wait for the previous FCMUpdateTask to finish
                 * before starting the new FCMUpdateTask.  Without this, it an
                 * FCMUpdateTask packet comes in at just the right time, the
                 * PacketReceptionHandler would start a new FCMUpdateTask
                 * while the EndControlTask was waiting for it to finish.
                 */
                while (TestFCMUpdateTaskEndingFlag() == TRUE)
                {
                    dprintf(DPRINTF_FCM, "UTS: Waiting for the previous FCM UpdateTask to end\n");
                    TaskSleepMS(20);
                }

                /*
                 * Fork the FCM update task
                 */
                UINT32 count = gFCMUpdateTaskInProgressCount;        /* Need to verify task started. */
                TaskCreate(FCM_CountersUpdateTask, NULL);

                /*
                 * Wait for the control task to set the IN_PROGRESS flag
                 * to TRUE.  This flag indicates when the control task
                 * is forked and running.
                 */
                do
                {
                    TaskSleepMS(20);
                }
                while (TestFCMUpdateTaskInProgressFlag() == FALSE && count == gFCMUpdateTaskInProgressCount);

                dprintf(DPRINTF_FCM, "UTS: FCM UpdateTask task forked and running\n");
            }
            else
            {
                /*
                 * FCMUpdateTask already in progress.
                 */
                dprintf(DPRINTF_FCM, "UTS: FCM UpdateTask task is already in progress\n");
            }

            /*
             * Clear the starting flag
             */
            SetFCMUpdateTaskStartingFlag(FALSE);
        }
        else
        {
            dprintf(DPRINTF_FCM, "UTS: Another task is already starting the UpdateTask\n");
            dprintf(DPRINTF_FCM, "UTS: Waiting for another task to start the UpdateTask\n");

            /*
             * Don't return until the other task has finished starting the FCMUpdateTask.
             */
            while (TestFCMUpdateTaskStartingFlag() == TRUE)
            {
                TaskSleepMS(20);
            }

            /*
             * At the point where the FCMUpdateTask is no longer
             * starting, if the FCMUpdateTaskInProgress flag isn't TRUE, then
             * the other task got an error when it tried to start the
             * FCMUpdateTask.  In this case, to be consistent, we need to also
             * return an error to our task.
             */
            if (TestFCMUpdateTaskInProgressFlag() == TRUE)
            {
                dprintf(DPRINTF_FCM, "UTS: Another task started the FCMUpdateTask\n");
            }
            else
            {
                dprintf(DPRINTF_FCM, "UTS: Another task had problems starting the FCMUpdateTask\n");
                returnCode = ERROR;
            }
        }
    }
    else
    {
        dprintf(DPRINTF_FCM, "UTS: Controller hasn't reached POWER_UP_COMPLETE\n");
        returnCode = ERROR;
    }

    return (returnCode);
}


/*----------------------------------------------------------------------------
**  Function Name: FCM_CountersUpdateTask
**
**  Parameters: None
**
**  Returns:    GOOD  - Event processed
**              ERROR - Event not processed
**--------------------------------------------------------------------------*/
static void FCM_CountersUpdateTask(UNUSED TASK_PARMS *parms)
{
    /*
     * Store the a pointer to the current pcb into updateTaskPcbPtr.
     * This will allow the other tasks to adjust the sleep time (delay)
     * of this task directly.
     */
    updateTaskPcbPtr = XK_GetPcb();

    dprintf(DPRINTF_FCM, "UT: UpdateTask running\n");

    /*
     * Set the IN_PROGRESS flag to indicate that the task is running, and
     * task switch so that the other tasks are allowed to run, and see that
     * the updateTask is also running.
     */
    SetFCMUpdateTaskInProgressFlag(TRUE);
    gFCMUpdateTaskInProgressCount++;            /* Need to verify task really started. */
    TaskSwitch();

    if (FCM_CountersUpdateMap(&counterMap) != GOOD)
    {
        dprintf(DPRINTF_FCM, "UT: UpdateMap returned ERROR\n");
    }

    /*
     * Clear the IN_PROGRESS flag to indicate that the control task is ending.
     */
    dprintf(DPRINTF_FCM, "UT: UpdateTask ending\n");
    SetFCMUpdateTaskInProgressFlag(FALSE);

    /*
     * Reset the updateTaskPcbPtr, since it's shutting down
     */
    updateTaskPcbPtr = NULL;
}


/*----------------------------------------------------------------------------
**  Function Name: FCM_CountersUpdateTaskAbort
**
**  Parameters: None
**
**  Returns:    Nothing
**--------------------------------------------------------------------------*/
static void FCM_CountersUpdateTaskAbort(void)
{
    dprintf(DPRINTF_FCM, "UA: Update abort - End update task\n");

    /*
     * Set the ending flag
     */
    SetFCMUpdateTaskEndingFlag(TRUE);

    /*
     * Semaphore - Make sure that a task isn't trying to start an FCMUpdateTask
     * when another is trying to end it.  If this does happen for some
     * reason, let StartUpdateTask return before killing it... otherwise
     * we could hang inside the StartUpdateTask function.
     */
    while (TestFCMUpdateTaskStartingFlag() == TRUE)
    {
        TaskSleepMS(20);
    }

    /*
     * Check that the update task is forked and running before
     * trying to shut it down.
     */
    if (TestFCMUpdateTaskInProgressFlag() == TRUE)
    {
        /*
         * Set the 'update abort' flag, to exit the counter update routines
         */
        SetFCMUpdateTaskAbortFlag(TRUE);

        /*
         * Wait for the update task to end itself before returning.
         * The update task sets the IN_PROGRESS flag to FALSE
         * immediately prior to exiting.
         */
        while (TestFCMUpdateTaskInProgressFlag() == TRUE)
        {
            TaskSleepMS(20);
        }
    }

    /*
     * Clear the 'update abort' flag, since the update is no longer in progress
     */
    SetFCMUpdateTaskAbortFlag(FALSE);

    /*
     * Clear the ending flag
     */
    SetFCMUpdateTaskEndingFlag(FALSE);
}


/*----------------------------------------------------------------------------
**  Function Name: FCM_CountersUpdateMap
**
**  Parameters: None
**
**  Returns:    GOOD  - Table built successfully
**              ERROR - Table not built
**--------------------------------------------------------------------------*/
UINT32 FCM_CountersUpdateMap(FCM_COUNTER_MAP *counterMapPtr)
{
    UINT32      returnCode = GOOD;

    ccb_assert(counterMapPtr != NULL, counterMapPtr);

    if (counterMapPtr != NULL)
    {
        /*
         * Acquire the mutex
         */
        (void)LockMutex(&counterMapPtr->header.busyMutex, MUTEX_WAIT);

        /*
         * Invalidate the map before the update occurs
         */
        counterMapPtr->flags.bits.updateMapValid = FALSE;

        /*
         * Update the counterMap components (HABs and drives)
         * Update both the HAB and the bay lists, even if one returns an error.
         */
        if (FCM_CountersUpdateHabDataList(counterMapPtr->updateData.habDataList) != GOOD)
        {
            dprintf(DPRINTF_FCM, "UM: UpdateHabDataList returned ERROR\n");
            returnCode = ERROR;
        }

        if (FCM_CountersUpdateBayDataList(counterMapPtr->updateData.bayDataList) != GOOD)
        {
            dprintf(DPRINTF_FCM, "UM: UpdateBayDataList returned ERROR\n");
            returnCode = ERROR;
        }

        /*
         * Store the timestamp of when this measurement was taken
         */
        RTC_GetTimeStamp(&counterMapPtr->updateData.beginTimestamp);
        counterMapPtr->updateData.endTimestamp = counterMapPtr->updateData.beginTimestamp;

        /*
         * Indicate the validity of the counterMap
         */
        if (returnCode == GOOD)
        {
            dprintf(DPRINTF_DEFAULT, "UM: counterMap updated\n");
            counterMapPtr->flags.bits.updateMapValid = TRUE;
        }

        /*
         * Release the mutex
         */
        UnlockMutex(&counterMapPtr->header.busyMutex);
    }
    else
    {
        dprintf(DPRINTF_FCM, "UM: counterMapPtr is NULL\n");
        returnCode = ERROR;
    }

    return (returnCode);
}


/*----------------------------------------------------------------------------
**  Function Name: FCM_CountersBaselineMap
**
**  Comments:   This function swaps the updateList and baselineList pointers
**
**  Returns:    GOOD  - Pointers swapped
**              ERROR - Error swapping pointers
**--------------------------------------------------------------------------*/
UINT32 FCM_CountersBaselineMap(FCM_COUNTER_MAP *counterMapPtr)
{
    UINT32      returnCode = ERROR;
    FCM_HAB_DATA *tempHabList = NULL;
    FCM_BAY_DATA *tempBayList = NULL;

    ccb_assert(counterMapPtr != NULL, counterMapPtr);

    if (counterMapPtr != NULL)
    {
        /*
         * Acquire the mutex
         */
        (void)LockMutex(&counterMapPtr->header.busyMutex, MUTEX_WAIT);

        dprintf(DPRINTF_FCM, "BM: Setting the FCAL counter baseline\n");

        if (counterMapPtr->flags.bits.updateMapValid == TRUE)
        {
            /*
             * Swap the data pointers
             */
            tempHabList = counterMapPtr->baselineData.habDataList;
            tempBayList = counterMapPtr->baselineData.bayDataList;

            counterMapPtr->baselineData.habDataList = counterMapPtr->updateData.habDataList;
            counterMapPtr->baselineData.bayDataList = counterMapPtr->updateData.bayDataList;
            counterMapPtr->baselineData.beginTimestamp = counterMapPtr->updateData.beginTimestamp;
            counterMapPtr->baselineData.endTimestamp = counterMapPtr->updateData.endTimestamp;

            counterMapPtr->updateData.habDataList = tempHabList;
            counterMapPtr->updateData.bayDataList = tempBayList;

            /*
             * Set the baselineValid flag and invalidate the update list.  We
             * just 'moved' the update data into the baseline space, and we
             * don't need two copies.
             * Invalidate delta data, since a new baseline was just set
             */
            counterMapPtr->flags.bits.baselineMapValid = TRUE;
            counterMapPtr->flags.bits.updateMapValid = FALSE;
            counterMapPtr->flags.bits.deltaMapValid = FALSE;

            /*
             * Reinitialize the invalidated errorData structures
             */
            FCM_CountersInitializeErrorData(&counterMapPtr->updateData);
            FCM_CountersInitializeErrorData(&counterMapPtr->deltaData);

            /*
             * Baseline has been captured - return GOOD
             */
            returnCode = GOOD;
        }
        else
        {
            dprintf(DPRINTF_FCM, "BM: Update map not valid - baseline not set\n");
        }

        /*
         * Release the mutex
         */
        UnlockMutex(&counterMapPtr->header.busyMutex);
    }
    else
    {
        dprintf(DPRINTF_FCM, "BM: counterMapPtr is NULL\n");
    }

    return (returnCode);
}


/*----------------------------------------------------------------------------
**  Function Name: FCM_CountersDeltaMap
**
**  Comments:   This function calculates the delta values between the
**              baselineList and the updateList, where the baselineList
**              contains the older data.
**
**  Returns:    GOOD  - Delta calculated
**              ERROR - Error calculating delta
**--------------------------------------------------------------------------*/
UINT32 FCM_CountersDeltaMap(FCM_COUNTER_MAP *counterMapPtr)
{
    UINT32      returnCode = ERROR;

    ccb_assert(counterMapPtr != NULL, counterMapPtr);

    if (counterMapPtr != NULL)
    {
        ccb_assert(counterMapPtr->baselineData.bayDataList != NULL, counterMapPtr->baselineData.bayDataList);
        ccb_assert(counterMapPtr->updateData.bayDataList != NULL, counterMapPtr->updateData.bayDataList);
        ccb_assert(counterMapPtr->deltaData.bayDataList != NULL, counterMapPtr->deltaData.bayDataList);

        /*
         * Acquire the mutex
         */
        (void)LockMutex(&counterMapPtr->header.busyMutex, MUTEX_WAIT);

        /*
         * Indicate that the deltaMapList is not yet valid
         */
        counterMapPtr->flags.bits.deltaMapValid = FALSE;

        /*
         * Before running the delta, make sure we a valid baseline map
         * and delta map.
         */
        if ((counterMapPtr->flags.bits.baselineMapValid == TRUE) &&
            (counterMapPtr->flags.bits.updateMapValid == TRUE))
        {
            dprintf(DPRINTF_FCM, "DM: Calculating the FCAL counter delta\n");

            /*
             * Do the delta calculations
             */
            if (counterMapPtr->deltaData.habDataList != NULL)
            {
                /*
                 * Process the habData and make the delta calculations
                 */
                if ((counterMapPtr->baselineData.habDataList != NULL) &&
                    (counterMapPtr->updateData.habDataList != NULL))
                {
                    if (FCM_CountersDeltaHabDataList(counterMapPtr->baselineData.habDataList,
                         counterMapPtr->updateData.habDataList, counterMapPtr->deltaData.habDataList)
                         == GOOD)
                    {
                        returnCode = GOOD;
                    }
                }
                else
                {
                    FCM_CountersInitializeHabDataList(counterMapPtr->deltaData.habDataList);
                }
            }

            if (counterMapPtr->deltaData.bayDataList != NULL)
            {
                /*
                 * Process the bayData and make the delta calculations
                 */
                if ((counterMapPtr->baselineData.bayDataList != NULL) &&
                    (counterMapPtr->updateData.bayDataList != NULL))
                {
                    if (FCM_CountersDeltaBayDataList(counterMapPtr->baselineData.bayDataList,
                         counterMapPtr->updateData.bayDataList,
                         counterMapPtr->deltaData.bayDataList) == GOOD)
                    {
                        returnCode = GOOD;
                    }
                }
                else
                {
                    FCM_CountersInitializeBayDataList(counterMapPtr->deltaData.bayDataList);
                }
            }

            /*
             * Store the timestamps over which this delta was taken
             */
            counterMapPtr->deltaData.beginTimestamp = counterMapPtr->baselineData.beginTimestamp;
            counterMapPtr->deltaData.endTimestamp = counterMapPtr->updateData.beginTimestamp;

            /*
             * If the deltaMap was generated correctly, then indicate that it's valid
             */
            if (returnCode == GOOD)
            {
                counterMapPtr->flags.bits.deltaMapValid = TRUE;
            }
        }
        else
        {
            dprintf(DPRINTF_FCM, "DM: Unable to calculate delta (baseline/update not valid)\n");
            FCM_CountersInitializeErrorData(&counterMapPtr->deltaData);
        }

        /*
         * Release the mutex
         */
        UnlockMutex(&counterMapPtr->header.busyMutex);
    }
    else
    {
        dprintf(DPRINTF_FCM, "DM: counterMapPtr is NULL\n");
    }

    return (returnCode);
}


/*----------------------------------------------------------------------------
**  Function Name: FCM_CountersDeltaHabDataList
**
**  Parameters: oldHabDataListPtr   - Pointer to older habDataList
**              newHabDataListPtr   - Pointer to newer habDataList
**              deltaHabDataListPtr - Pointer to delta habDataList
**
**  Returns:    GOOD  - Delta map generated
**              ERROR - Delta map not generated
**--------------------------------------------------------------------------*/
static UINT32 FCM_CountersDeltaHabDataList(FCM_HAB_DATA *oldHabDataList,
                                           FCM_HAB_DATA *newHabDataList,
                                           FCM_HAB_DATA *deltaHabDataList)
{
    UINT32      returnCode = ERROR;
    UINT32      habCounter = 0;

    ccb_assert(oldHabDataList != NULL, oldHabDataList);
    ccb_assert(newHabDataList != NULL, newHabDataList);
    ccb_assert(deltaHabDataList != NULL, deltaHabDataList);

    if (deltaHabDataList != NULL)
    {
        if ((oldHabDataList != NULL) && (newHabDataList != NULL))
        {
            for (habCounter = 0; habCounter < dimension_of(habDataList0); habCounter++)
            {
                if (FCM_CountersDeltaHabData(&oldHabDataList[habCounter],
                                             &newHabDataList[habCounter],
                                             &deltaHabDataList[habCounter]) == GOOD)
                {
                    returnCode = GOOD;
                }
            }
        }

        if (returnCode != GOOD)
        {
            FCM_CountersInitializeHabDataList(deltaHabDataList);
        }
    }

    return (returnCode);
}


/*----------------------------------------------------------------------------
**  Function Name: FCM_CountersDeltaHabData
**
**  Parameters: oldHabDataPtr   - Pointer to older habData
**              newHabDataPtr   - Pointer to newer habData
**              deltaHabDataPtr - Pointer to delta habData
**
**  Returns:    GOOD  - habData delta generated
**              ERROR - habData delta not generated
**--------------------------------------------------------------------------*/
static UINT32 FCM_CountersDeltaHabData(FCM_HAB_DATA *oldHabDataPtr,
                                       FCM_HAB_DATA *newHabDataPtr,
                                       FCM_HAB_DATA *deltaHabDataPtr)
{
    UINT32      returnCode = ERROR;

    ccb_assert(oldHabDataPtr != NULL, oldHabDataPtr);
    ccb_assert(newHabDataPtr != NULL, newHabDataPtr);
    ccb_assert(deltaHabDataPtr != NULL, deltaHabDataPtr);

    if (deltaHabDataPtr != NULL)
    {
        if ((oldHabDataPtr != NULL) &&
            (newHabDataPtr != NULL) &&
            (oldHabDataPtr->flags.bits.habPresent == TRUE) &&
            (newHabDataPtr->flags.bits.habPresent == TRUE))
        {
            /*
             * Copy the newest HAB data into the delta data
             */
            *deltaHabDataPtr = *newHabDataPtr;

            /*
             * Recalculate the counter deltas between old and new data
             */
            deltaHabDataPtr->habCounters.linkFail =
                FCM_CountersDeltaCalculateValue(oldHabDataPtr->habCounters.linkFail,
                                                newHabDataPtr->habCounters.linkFail);

            deltaHabDataPtr->habCounters.lostSync =
                FCM_CountersDeltaCalculateValue(oldHabDataPtr->habCounters.lostSync,
                                                newHabDataPtr->habCounters.lostSync);

            deltaHabDataPtr->habCounters.lostSignal =
                FCM_CountersDeltaCalculateValue(oldHabDataPtr->habCounters.lostSignal,
                                                newHabDataPtr->habCounters.lostSignal);

            deltaHabDataPtr->habCounters.sequenceError =
                FCM_CountersDeltaCalculateValue(oldHabDataPtr->habCounters.sequenceError,
                                                newHabDataPtr->habCounters.sequenceError);

            deltaHabDataPtr->habCounters.invalidTransmit =
                FCM_CountersDeltaCalculateValue(oldHabDataPtr->habCounters.invalidTransmit,
                                                newHabDataPtr->habCounters.invalidTransmit);

            deltaHabDataPtr->habCounters.invalidCRC =
                FCM_CountersDeltaCalculateValue(oldHabDataPtr->habCounters.invalidCRC,
                                                newHabDataPtr->habCounters.invalidCRC);

            returnCode = GOOD;
        }

        if (returnCode == GOOD)
        {
            deltaHabDataPtr->flags.value = oldHabDataPtr->flags.value | newHabDataPtr->flags.value;
        }
        else
        {
            FCM_CountersInitializeHabData(deltaHabDataPtr);
        }
    }

    return (returnCode);
}


/*----------------------------------------------------------------------------
**  Function Name: FCM_CountersDeltaBayDataList
**
**  Parameters: oldBayDataListPtr   - Pointer to older bayDataList
**              newBayDataListPtr   - Pointer to newer bayDataList
**              deltaBayDataListPtr - Pointer to delta bayDataList
**
**  Returns:    GOOD  - Delta map generated
**              ERROR - Delta map not generated
**--------------------------------------------------------------------------*/
static UINT32 FCM_CountersDeltaBayDataList(FCM_BAY_DATA *oldBayDataList,
                                           FCM_BAY_DATA *newBayDataList,
                                           FCM_BAY_DATA *deltaBayDataList)
{
    UINT32      returnCode = ERROR;
    UINT32      bayCounter = 0;

    ccb_assert(oldBayDataList != NULL, oldBayDataList);
    ccb_assert(newBayDataList != NULL, newBayDataList);
    ccb_assert(deltaBayDataList != NULL, deltaBayDataList);

    if (deltaBayDataList != NULL)
    {
        if ((oldBayDataList != NULL) && (newBayDataList != NULL))
        {
            for (bayCounter = 0; bayCounter < dimension_of(bayDataList0); bayCounter++)
            {
                if (FCM_CountersDeltaBayData(&oldBayDataList[bayCounter],
                                             &newBayDataList[bayCounter],
                                             &deltaBayDataList[bayCounter]) == GOOD)
                {
                    returnCode = GOOD;
                }
            }
        }

        if (returnCode != GOOD)
        {
            FCM_CountersInitializeBayDataList(deltaBayDataList);
        }
    }

    return (returnCode);
}


/*----------------------------------------------------------------------------
**  Function Name: FCM_CountersDeltaBayData
**
**  Parameters: oldBayDataPtr   - Pointer to older bayData
**              newBayDataPtr   - Pointer to newer bayData
**              deltaBayDataPtr - Pointer to delta bayData
**
**  Returns:    GOOD  - slotData initialized
**              ERROR - slotData not initialized (bad input pointer)
**--------------------------------------------------------------------------*/
static UINT32 FCM_CountersDeltaBayData(FCM_BAY_DATA *oldBayDataPtr,
                                       FCM_BAY_DATA *newBayDataPtr,
                                       FCM_BAY_DATA *deltaBayDataPtr)
{
    UINT32      returnCode = ERROR;
    UINT32      slotCounter = 0;

    ccb_assert(oldBayDataPtr != NULL, oldBayDataPtr);
    ccb_assert(newBayDataPtr != NULL, newBayDataPtr);
    ccb_assert(deltaBayDataPtr != NULL, deltaBayDataPtr);

    if (deltaBayDataPtr != NULL)
    {
        if ((oldBayDataPtr != NULL) &&
            (newBayDataPtr != NULL) &&
            (oldBayDataPtr->flags.bits.bayPresent == TRUE) &&
            (newBayDataPtr->flags.bits.bayPresent == TRUE))
        {
            for (slotCounter = 0; slotCounter < dimension_of(bayDataList1->slotDataList);
                 slotCounter++)
            {
                if (FCM_CountersDeltaSlotData(slotCounter, oldBayDataPtr,
                                              newBayDataPtr, deltaBayDataPtr) == GOOD)
                {
                    returnCode = GOOD;
                }
            }


            if ((oldBayDataPtr->bayInfo.valid == TRUE) &&
                (newBayDataPtr->bayInfo.valid == TRUE))
            {
                if (FCM_CountersDeltaBayInfo(&oldBayDataPtr->bayInfo,
                                             &newBayDataPtr->bayInfo,
                                             &deltaBayDataPtr->bayInfo) == GOOD)
                {
                    returnCode = GOOD;
                }
                else
                {
                    returnCode = ERROR;
                }
            }

            if ((oldBayDataPtr->flags.bits.counterValid == TRUE) &&
                (newBayDataPtr->flags.bits.counterValid == TRUE))
            {
                if (FCM_CountersDeltaBayErrorCounters(oldBayDataPtr->bayInfo.type,
                                                      &oldBayDataPtr->errorCounts,
                                                      &newBayDataPtr->errorCounts,
                                                      &deltaBayDataPtr->errorCounts) == GOOD)
                {
                    returnCode = GOOD;
                }
                else
                {
                    returnCode = ERROR;
                }

            }
        }

        if (returnCode == GOOD)
        {
            deltaBayDataPtr->flags.value = oldBayDataPtr->flags.value | newBayDataPtr->flags.value;
        }
        else
        {
            FCM_CountersInitializeBayData(deltaBayDataPtr);
        }
    }

    return (returnCode);
}


/*----------------------------------------------------------------------------
**  Function Name: FCM_CountersDeltaSlotData
**
**  Comments:   This function calculates the delta values between the
**              baselineList and the updateList, where the baselineList
**              contains the older data.
**
**  Returns:    GOOD  - Delta calculated
**              ERROR - Error calculating delta
**--------------------------------------------------------------------------*/
static UINT32 FCM_CountersDeltaSlotData(UINT32 slotCounter, FCM_BAY_DATA *oldBayPtr,
                                        FCM_BAY_DATA *newBayPtr,
                                        FCM_BAY_DATA *deltaBayPtr)
{
    UINT32      returnCode = ERROR;
    FCM_SLOT_DATA *oldSlotDataPtr = &oldBayPtr->slotDataList[slotCounter];
    FCM_SLOT_DATA *newSlotDataPtr = &newBayPtr->slotDataList[slotCounter];
    FCM_SLOT_DATA *deltaSlotDataPtr = &deltaBayPtr->slotDataList[slotCounter];

    ccb_assert(oldSlotDataPtr != NULL, oldSlotDataPtr);
    ccb_assert(newSlotDataPtr != NULL, newSlotDataPtr);
    ccb_assert(deltaSlotDataPtr != NULL, deltaSlotDataPtr);

    if (deltaSlotDataPtr != NULL)
    {
        if ((oldSlotDataPtr != NULL) &&
            (newSlotDataPtr != NULL) &&
            (oldSlotDataPtr->flags.bits.drivePresent == TRUE) &&
            (newSlotDataPtr->flags.bits.drivePresent == TRUE))
        {
            /*
             * Process the slotData and make the delta calculations
             */
            if (oldSlotDataPtr->wwn == newSlotDataPtr->wwn)
            {
                /*
                 * Copy the newest slot data into the delta data
                 */
                *deltaSlotDataPtr = *newSlotDataPtr;

                /*
                 * Recalculate the counter deltas between old and new data
                 */
                returnCode = FCM_CountersDeltaSlotDataCounters(slotCounter, oldBayPtr,
                                                               newBayPtr, deltaBayPtr);
            }
        }

        if (returnCode == GOOD)
        {
            deltaSlotDataPtr->flags.value = oldSlotDataPtr->flags.value | newSlotDataPtr->flags.value;
        }
        else
        {
            FCM_CountersInitializeSlotData(deltaSlotDataPtr);
        }
    }

    return (returnCode);
}


/*----------------------------------------------------------------------------
**  Function Name: FCM_CountersDeltaBayInfo
**
**  Comments:   This function calculates the delta values between the
**              baselineList and the updateList, where the baselineList
**              contains the older data.
**
**  Returns:    GOOD  - Delta calculated
**              ERROR - Error calculating delta
**--------------------------------------------------------------------------*/
static UINT32 FCM_CountersDeltaBayInfo(FCM_BAY_INFO *oldInfoPtr,
                                       FCM_BAY_INFO *newInfoPtr,
                                       FCM_BAY_INFO *deltaInfoPtr)
{
    UINT32      returnCode = ERROR;

    ccb_assert(oldInfoPtr != NULL, oldInfoPtr);
    ccb_assert(newInfoPtr != NULL, newInfoPtr);
    ccb_assert(deltaInfoPtr != NULL, deltaInfoPtr);

    if (deltaInfoPtr != NULL)
    {
        if ((oldInfoPtr != NULL) && (newInfoPtr != NULL))
        {
            deltaInfoPtr->bayId = newInfoPtr->bayId;
            deltaInfoPtr->type = newInfoPtr->type;
            deltaInfoPtr->valid = newInfoPtr->valid;
            deltaInfoPtr->wwn = newInfoPtr->wwn;
            returnCode = GOOD;
        }
        else
        {
            FCM_CountersInitializeBayInfo(deltaInfoPtr);
        }
    }

    return (returnCode);
}

/*----------------------------------------------------------------------------
**  Function Name: FCM_CountersDeltaBayErrorCounters
t**
**  Comments:   This function calculates the delta values between the
**              baselineList and the updateList, where the baselineList
**              contains the older data.
**
**  Returns:    GOOD  - Delta calculated
**              ERROR - Error calculating delta
**--------------------------------------------------------------------------*/
static UINT32 FCM_CountersDeltaBayErrorCounters(UINT8 type,
                                                FCM_BAY_ERROR_COUNTERS *oldCountersPtr,
                                                FCM_BAY_ERROR_COUNTERS *newCountersPtr,
                                                FCM_BAY_ERROR_COUNTERS *deltaCountersPtr)
{
    UINT32      returnCode = ERROR;
    UINT8       j;
    UINT8       i;

    ccb_assert(oldCountersPtr != NULL, oldCountersPtr);
    ccb_assert(newCountersPtr != NULL, newCountersPtr);
    ccb_assert(deltaCountersPtr != NULL, deltaCountersPtr);

    if (deltaCountersPtr != NULL)
    {
        if ((oldCountersPtr != NULL) && (newCountersPtr != NULL))
        {
            for (j = 0; j < ((type == PD_DT_SBOD_SES) ? 2 : 1); j++)
            {
                for (i = 0; i < FCM_EURO_NUM_BAY_PORTS; i++)
                {
                    deltaCountersPtr->port[i][j].invalidCRC =
                        FCM_CountersDeltaCalculateValue(oldCountersPtr->port[i][j].invalidCRC,
                                                        newCountersPtr->port[i][j].invalidCRC);
                    deltaCountersPtr->port[i][j].invalidTransmit =
                        FCM_CountersDeltaCalculateValue(oldCountersPtr->port[i][j].invalidTransmit,
                                                        newCountersPtr->port[i][j].invalidTransmit);
                    deltaCountersPtr->port[i][j].linkFail =
                        FCM_CountersDeltaCalculateValue(oldCountersPtr->port[i][j].linkFail,
                                                        newCountersPtr->port[i][j].linkFail);
                    deltaCountersPtr->port[i][j].lostSignal =
                        FCM_CountersDeltaCalculateValue(oldCountersPtr->port[i][j].lostSignal,
                                                        newCountersPtr->port[i][j].lostSignal);
                    deltaCountersPtr->port[i][j].lostSync =
                        FCM_CountersDeltaCalculateValue(oldCountersPtr->port[i][j].lostSync,
                                                        newCountersPtr->port[i][j].lostSync);
                    deltaCountersPtr->port[i][j].sequenceError =
                        FCM_CountersDeltaCalculateValue(oldCountersPtr->port[i][j].sequenceError,
                                                        newCountersPtr->port[i][j].sequenceError);
                    if (type == PD_DT_SBOD_SES)
                    {
                        deltaCountersPtr->port[i][j].WordErrorCount =
                            FCM_CountersDeltaCalculateValue(oldCountersPtr->port[i][j].WordErrorCount,
                                                            newCountersPtr->port[i][j].WordErrorCount);
                        deltaCountersPtr->port[i][j].CRCErrorCount =
                            FCM_CountersDeltaCalculateValue(oldCountersPtr->port[i][j].CRCErrorCount,
                                                            newCountersPtr->port[i][j].CRCErrorCount);
                        deltaCountersPtr->port[i][j].ClockDelta =
                            FCM_CountersDeltaCalculateSignedValue(oldCountersPtr->port[i][j].ClockDelta,
                                                                  newCountersPtr->port[i][j].ClockDelta);
                        deltaCountersPtr->port[i][j].LoopUpCount =
                            FCM_CountersDeltaCalculateValue(oldCountersPtr->port[i][j].LoopUpCount,
                                                            newCountersPtr->port[i][j].LoopUpCount);
                        deltaCountersPtr->port[i][j].InsertionCount =
                            FCM_CountersDeltaCalculateValue(oldCountersPtr->port[i][j].InsertionCount,
                                                            newCountersPtr->port[i][j].InsertionCount);
                        deltaCountersPtr->port[i][j].StallCount =
                            FCM_CountersDeltaCalculateValue(oldCountersPtr->port[i][j].StallCount,
                                                            newCountersPtr->port[i][j].StallCount);
                        deltaCountersPtr->port[i][j].Utilization =
                            FCM_CountersDeltaCalculateValue(oldCountersPtr->port[i][j].Utilization,
                                                            newCountersPtr->port[i][j].Utilization);
                    }
                }
            }

            returnCode = GOOD;
        }
        else
        {
            FCM_CountersInitializeBayErrorCounters(deltaCountersPtr);
        }
    }

    return (returnCode);
}


/*----------------------------------------------------------------------------
**  Function Name: FCM_CountersDeltaSlotDataCounters
**
**  Comments:   This function calculates the delta values between the
**              baselineList and the updateList, where the baselineList
**              contains the older data.
**
**  Returns:    GOOD  - Delta calculated
**              ERROR - Error calculating delta
**--------------------------------------------------------------------------*/
static UINT32 FCM_CountersDeltaSlotDataCounters(UINT32 slotCounter,
                                                FCM_BAY_DATA *oldBayPtr,
                                                FCM_BAY_DATA *newBayPtr,
                                                FCM_BAY_DATA *deltaBayPtr)
{
    UINT32      returnCode = ERROR;
    FCM_SLOT_DATA_COUNTERS *oldCountersPtr = &oldBayPtr->slotDataList[slotCounter].counters;
    FCM_SLOT_DATA_COUNTERS *newCountersPtr = &newBayPtr->slotDataList[slotCounter].counters;
    FCM_SLOT_DATA_COUNTERS *deltaCountersPtr = &deltaBayPtr->slotDataList[slotCounter].counters;

    ccb_assert(oldCountersPtr != NULL, oldCountersPtr);
    ccb_assert(newCountersPtr != NULL, newCountersPtr);
    ccb_assert(deltaCountersPtr != NULL, deltaCountersPtr);

    if (deltaCountersPtr != NULL)
    {
        if ((oldCountersPtr != NULL) && (newCountersPtr != NULL))
        {
            deltaCountersPtr->powerOnMinutes =
                FCM_CountersDeltaCalculateValue(oldCountersPtr->powerOnMinutes,
                                                newCountersPtr->powerOnMinutes);

            /* Port A */
            deltaCountersPtr->portA.linkFail =
                FCM_CountersDeltaCalculateValue(oldCountersPtr->portA.linkFail,
                                                newCountersPtr->portA.linkFail);
            deltaCountersPtr->portA.lostSync =
                FCM_CountersDeltaCalculateValue(oldCountersPtr->portA.lostSync,
                                                newCountersPtr->portA.lostSync);
            deltaCountersPtr->portA.invalidTransmit =
                FCM_CountersDeltaCalculateValue(oldCountersPtr->portA.invalidTransmit,
                                                newCountersPtr->portA.invalidTransmit);
            deltaCountersPtr->portA.invalidCRC =
                FCM_CountersDeltaCalculateValue(oldCountersPtr->portA.invalidCRC,
                                                newCountersPtr->portA.invalidCRC);
            deltaCountersPtr->portA.lipF7Initiated =
                FCM_CountersDeltaCalculateValue(oldCountersPtr->portA.lipF7Initiated,
                                                newCountersPtr->portA.lipF7Initiated);
            deltaCountersPtr->portA.lipF7Received =
                FCM_CountersDeltaCalculateValue(oldCountersPtr->portA.lipF7Received,
                                                newCountersPtr->portA.lipF7Received);
            deltaCountersPtr->portA.lipF8Initiated =
                FCM_CountersDeltaCalculateValue(oldCountersPtr->portA.lipF8Initiated,
                                                newCountersPtr->portA.lipF8Initiated);
            deltaCountersPtr->portA.lipF8Received =
                FCM_CountersDeltaCalculateValue(oldCountersPtr->portA.lipF8Received,
                                                newCountersPtr->portA.lipF8Received);

            if (oldBayPtr->bayInfo.type == PD_DT_SBOD_SES)
            {
                deltaCountersPtr->portA.WordErrorCount =
                    FCM_CountersDeltaCalculateValue(oldCountersPtr->portA.WordErrorCount,
                                                    newCountersPtr->portA.WordErrorCount);
                deltaCountersPtr->portA.CRCErrorCount =
                    FCM_CountersDeltaCalculateValue(oldCountersPtr->portA.CRCErrorCount,
                                                    newCountersPtr->portA.CRCErrorCount);
                deltaCountersPtr->portA.ClockDelta =
                    FCM_CountersDeltaCalculateSignedValue(oldCountersPtr->portA.ClockDelta,
                                                          newCountersPtr->portA.ClockDelta);
                deltaCountersPtr->portA.LoopUpCount =
                    FCM_CountersDeltaCalculateValue(oldCountersPtr->portA.LoopUpCount,
                                                    newCountersPtr->portA.LoopUpCount);
                deltaCountersPtr->portA.InsertionCount =
                    FCM_CountersDeltaCalculateValue(oldCountersPtr->portA.InsertionCount,
                                                    newCountersPtr->portA.InsertionCount);
                deltaCountersPtr->portA.StallCount =
                    FCM_CountersDeltaCalculateValue(oldCountersPtr->portA.StallCount,
                                                    newCountersPtr->portA.StallCount);
                deltaCountersPtr->portA.Utilization =
                    FCM_CountersDeltaCalculateValue(oldCountersPtr->portA.Utilization,
                                                    newCountersPtr->portA.Utilization);
            }

            /* Port B */
            deltaCountersPtr->portB.linkFail =
                FCM_CountersDeltaCalculateValue(oldCountersPtr->portB.linkFail,
                                                newCountersPtr->portB.linkFail);
            deltaCountersPtr->portB.lostSync =
                FCM_CountersDeltaCalculateValue(oldCountersPtr->portB.lostSync,
                                                newCountersPtr->portB.lostSync);
            deltaCountersPtr->portB.invalidTransmit =
                FCM_CountersDeltaCalculateValue(oldCountersPtr->portB.invalidTransmit,
                                                newCountersPtr->portB.invalidTransmit);
            deltaCountersPtr->portB.invalidCRC =
                FCM_CountersDeltaCalculateValue(oldCountersPtr->portB.invalidCRC,
                                                newCountersPtr->portB.invalidCRC);
            deltaCountersPtr->portB.lipF7Initiated =
                FCM_CountersDeltaCalculateValue(oldCountersPtr->portB.lipF7Initiated,
                                                newCountersPtr->portB.lipF7Initiated);
            deltaCountersPtr->portB.lipF7Received =
                FCM_CountersDeltaCalculateValue(oldCountersPtr->portB.lipF7Received,
                                                newCountersPtr->portB.lipF7Received);
            deltaCountersPtr->portB.lipF8Initiated =
                FCM_CountersDeltaCalculateValue(oldCountersPtr->portB.lipF8Initiated,
                                                newCountersPtr->portB.lipF8Initiated);
            deltaCountersPtr->portB.lipF8Received =
                FCM_CountersDeltaCalculateValue(oldCountersPtr->portB.lipF8Received,
                                                newCountersPtr->portB.lipF8Received);

            if (oldBayPtr->bayInfo.type == PD_DT_SBOD_SES)
            {
                deltaCountersPtr->portB.WordErrorCount =
                    FCM_CountersDeltaCalculateValue(oldCountersPtr->portB.WordErrorCount,
                                                    newCountersPtr->portB.WordErrorCount);
                deltaCountersPtr->portB.CRCErrorCount =
                    FCM_CountersDeltaCalculateValue(oldCountersPtr->portB.CRCErrorCount,
                                                    newCountersPtr->portB.CRCErrorCount);
                deltaCountersPtr->portB.ClockDelta =
                    FCM_CountersDeltaCalculateSignedValue(oldCountersPtr->portB.ClockDelta,
                                                          newCountersPtr->portB.ClockDelta);
                deltaCountersPtr->portB.LoopUpCount =
                    FCM_CountersDeltaCalculateValue(oldCountersPtr->portB.LoopUpCount,
                                                    newCountersPtr->portB.LoopUpCount);
                deltaCountersPtr->portB.InsertionCount =
                    FCM_CountersDeltaCalculateValue(oldCountersPtr->portB.InsertionCount,
                                                    newCountersPtr->portB.InsertionCount);
                deltaCountersPtr->portB.StallCount =
                    FCM_CountersDeltaCalculateValue(oldCountersPtr->portB.StallCount,
                                                    newCountersPtr->portB.StallCount);
                deltaCountersPtr->portB.Utilization =
                    FCM_CountersDeltaCalculateValue(oldCountersPtr->portB.Utilization,
                                                    newCountersPtr->portB.Utilization);
            }
            returnCode = GOOD;
        }
        else
        {
            FCM_CountersInitializeSlotDataCounters(deltaCountersPtr);
        }
    }

    return (returnCode);
}

/*----------------------------------------------------------------------------
**  Function Name: FCM_CountersDeltaCalculateSignedValue
**
**  Comments:   This function calculates the difference between the new and
**              old counter values.  If the old value is greater than the new
**              value, then this function returns -1.
**
**  Parameters: oldValue
**              newValue
**
**  Returns:    Delta value
**--------------------------------------------------------------------------*/
static INT32 FCM_CountersDeltaCalculateSignedValue(INT32 oldValue, INT32 newValue)
{
    INT32       returnValue = -1;

    if ((oldValue != -1) && (newValue != -1))
    {
        returnValue = newValue - oldValue;
    }
    return (returnValue);
}


/*----------------------------------------------------------------------------
**  Function Name: FCM_CountersDeltaCalculateValue
**
**  Comments:   This function calculates the difference between the new and
**              old counter values.  If the old value is greater than the new
**              value, then this function returns -1.
**
**  Parameters: oldValue
**              newValue
**
**  Returns:    Delta value
**--------------------------------------------------------------------------*/
static UINT32 FCM_CountersDeltaCalculateValue(UINT32 oldValue, UINT32 newValue)
{
    UINT32      returnValue = (UINT32)(-1);

    if ((oldValue != (UINT32)(-1)) &&
        (newValue != (UINT32)(-1)) && (newValue >= oldValue))
    {
        returnValue = newValue - oldValue;
    }

    return (returnValue);
}


/*----------------------------------------------------------------------------
**  Function Name: FCM_CountersUpdateHabDataList
**
**  Parameters: habDataList
**
**  Returns:    GOOD  - Table built successfully
**              ERROR - Table not built
**--------------------------------------------------------------------------*/
static UINT32 FCM_CountersUpdateHabDataList(FCM_HAB_DATA *habDataList)
{
    UINT32      returnCode = GOOD;
    UINT32      loopCount = 0;
    PI_PACKET_HEADER reqHdr;
    PI_PACKET_HEADER rspHdr;
    PI_STATS_LOOPS_REQ reqData = { PORT_STATS_RLS, {0, 0, 0} }; /* extended loop data */
    XIO_PACKET  reqPacket = { &reqHdr, (UINT8 *)&reqData };
    XIO_PACKET  rspPacket = { &rspHdr, NULL };
    PI_STATS_LOOPS_RSP *statsPtr = NULL;
    PI_STATS_LOOP *statsLoopPtr = NULL;
    MRPORT_RSP *portRspPtr = NULL;

    ccb_assert(habDataList != NULL, habDataList);

    memset(&reqHdr, 0, sizeof(reqHdr));
    memset(&rspHdr, 0, sizeof(rspHdr));

    reqHdr.packetVersion = 1;
    rspHdr.packetVersion = 1;

    if (habDataList != NULL)
    {
        /*
         * Reinitialize the bayData list
         */
        FCM_CountersInitializeHabDataList(habDataList);

        /*
         * Set up for the stats call
         */
        reqPacket.pHeader->commandCode = PI_STATS_BACK_END_LOOP_CMD;
        reqPacket.pHeader->length = sizeof(reqData);

        /*
         * Issue the command through the packet command handler
         */
        if (PacketCommandHandler(&reqPacket, &rspPacket) == PI_GOOD)
        {
            statsPtr = (PI_STATS_LOOPS_RSP *)rspPacket.pPacket;

            if (statsPtr != NULL)
            {
                /*
                 * Decode the data
                 * Set the statsLoopPtr
                 */
                statsLoopPtr = statsPtr->stats;

                for (loopCount = 0; loopCount < statsPtr->count; loopCount++)
                {
                    if (statsLoopPtr != NULL)
                    {
                        /*
                         * Mark the HAB as being present
                         */
                        if (statsLoopPtr->port < dimension_of(habDataList0))
                        {
                            habDataList[statsLoopPtr->port].flags.bits.habPresent = TRUE;
                            portRspPtr = statsLoopPtr->stats;

                            if (portRspPtr != NULL)
                            {
                                /*
                                 * Parse the counters from the RLS data
                                 */
                                habDataList[statsLoopPtr->port].habCounters.linkFail =
                                    portRspPtr->rls.linkFailureCnt;

                                habDataList[statsLoopPtr->port].habCounters.lostSync =
                                    portRspPtr->rls.lossSyncCnt;

                                habDataList[statsLoopPtr->port].habCounters.lostSignal =
                                    portRspPtr->rls.lossSignalCnt;

                                habDataList[statsLoopPtr->port].habCounters.sequenceError = portRspPtr->rls.primSeqErrCnt;

                                habDataList[statsLoopPtr->port].habCounters.invalidTransmit = portRspPtr->rls.invTWCnt;

                                habDataList[statsLoopPtr->port].habCounters.invalidCRC = portRspPtr->rls.invCRCCnt;

                                /*
                                 * Flag the counters as being valid
                                 */
                                habDataList[statsLoopPtr->port].flags.bits.countersValid = TRUE;

                                /*
                                 * Move the statsLoop pointer
                                 * The STATS_LOOP structure contains a variable length
                                 * field called 'target' - the size of which depends upon
                                 * the 'numtarg' field.
                                 */
                                statsLoopPtr = (PI_STATS_LOOP *)(portRspPtr->target + portRspPtr->numtarg);
                            }
                            else
                            {
                                dprintf(DPRINTF_FCM, "UHDL: portRspPtr is NULL\n");
                                returnCode = ERROR;
                            }
                        }
                        else
                        {
                            dprintf(DPRINTF_FCM, "UHDL: statsLoopPtr->port is too high (%u)\n",
                                    statsLoopPtr->port);
                            returnCode = ERROR;
                        }
                    }
                    else
                    {
                        dprintf(DPRINTF_FCM, "UHDL: statsLoopPtr is NULL\n");
                        returnCode = ERROR;
                    }
                }
            }
            else
            {
                dprintf(DPRINTF_FCM, "UHDL: statsPtr is NULL\n");
                returnCode = ERROR;
            }
        }

        /*
         * Done
         */
        Free(rspPacket.pPacket);
    }
    else
    {
        returnCode = ERROR;
    }

    return (returnCode);
}


/*----------------------------------------------------------------------------
**  Function Name: FCM_CountersUpdateBayDataList
**
**  Parameters: bayDataList
**
**  Returns:    GOOD  - Table built successfully
**              ERROR - Table not built
**--------------------------------------------------------------------------*/
static UINT32 FCM_CountersUpdateBayDataList(FCM_BAY_DATA *bayDataList)
{
    UINT32      returnCode = GOOD;
    UINT32      commandReturnCode = GOOD;
    UINT32      diskNumber = 0;
    PI_PACKET_HEADER reqHdr;
    PI_PACKET_HEADER rspHdr;
    XIO_PACKET  reqPacket = { &reqHdr, NULL };
    XIO_PACKET  rspPacket = { &rspHdr, NULL };
    PI_DEBUG_SCSI_CMD_REQ scsiReq;
    MRGETPINFO_RSP deviceInfo;
    MRGETPINFO_RSP bayInfo;
    MRGETPINFO_RSP *ptrDeviceInfo = NULL;
    PI_LIST_RSP *pDevList = NULL;
    UINT16      deviceCount = 0;
    UINT16      pid = 0;
    FCM_BAY_DATA *bayDataPtr = NULL;
    FCM_SLOT_DATA *slotDataPtr = NULL;

    memset(&reqHdr, 0, sizeof(reqHdr));
    memset(&rspHdr, 0, sizeof(rspHdr));
    memset(&scsiReq, 0, sizeof(scsiReq));

    reqHdr.packetVersion = 1;
    rspHdr.packetVersion = 1;

    /* Reinitialize the bayData list */
    FCM_CountersInitializeBayDataList(bayDataList);

    /* Get the list of PDisks */
    reqPacket.pHeader->commandCode = PI_PDISK_LIST_CMD;
    reqPacket.pHeader->length = 0;

    /* Issue the command through the packet command handler */
    commandReturnCode = PacketCommandHandler(&reqPacket, &rspPacket);

    if (commandReturnCode == GOOD)
    {
        /* Find the number of devices */
        pDevList = (PI_LIST_RSP *)rspPacket.pPacket;
        deviceCount = pDevList->count;

        /* Retrieve the FCAL counters for each device, copy data in */
        reqPacket.pHeader->commandCode = PI_DEBUG_SCSI_COMMAND_CMD;
        reqPacket.pHeader->length = sizeof(scsiReq);
        reqPacket.pPacket = (UINT8 *)&scsiReq;
        scsiReq.cdbLen = 10;

        for (diskNumber = 0;
             (diskNumber < deviceCount) && (TestFCMUpdateTaskAbortFlag() == FALSE);
             diskNumber++)
        {
            while (EL_TestInProgress())
            {
                TaskSleepMS(1000);      /* Wait for a second before continuing. */
            }

            /* Reset slot and bay data pointers */
            slotDataPtr = NULL;
            bayDataPtr = NULL;

            /* Get the pid */
            pid = pDevList->list[diskNumber];

            /* Go get the actual pdisk info if available, else use the cache */
            ptrDeviceInfo = PhysicalDisk(pid);
            if (ptrDeviceInfo != NULL)
            {
                deviceInfo = *ptrDeviceInfo;
                Free(ptrDeviceInfo);
                commandReturnCode = GOOD;
            }
            else
            {
                commandReturnCode = GetPDiskInfoFromPid(pid, &deviceInfo);
            }

            /* Go issue the scsi log sense command */
            if (commandReturnCode == GOOD)
            {
                /*
                 * Display the deviceInfo to the console
                 * dprintf( DPRINTF_FCM, "UBDL: DiskID %04d, pd_ses: %05d, dname[ses]: %03d, pd_slot: %03d, dname[slot]: %03d, Channel: 0x%02hhx\n",
                 * diskNumber,
                 * deviceInfo.pdd.ses,
                 * deviceInfo.pdd.devName[PD_DNAME_CSES],
                 * deviceInfo.pdd.slot,
                 * deviceInfo.pdd.devName[PD_DNAME_CSLOT],
                 * deviceInfo.pdd.channel );
                 */

                /*
                 * Check for valid SES data.  If the SES data is not valid, look
                 * at the DNAME data.  If the DNAME data is not valid, then go
                 * on to the next drive.  The ses data should be ignored until
                 * discovery has been run.  Don't use DNAME if the device is
                 * unlabeled, as it will always have a 'name' for bay zero.
                 */
                if ((DiscoveryComplete() == true) &&
                    (deviceInfo.pdd.ses < dimension_of(bayDataList1)) &&
                    (deviceInfo.pdd.slot < dimension_of(bayDataList1->slotDataList)))
                {
                    bayDataPtr = &bayDataList[deviceInfo.pdd.ses];
                    slotDataPtr = &bayDataPtr->slotDataList[deviceInfo.pdd.slot];
                }
                else if ((deviceInfo.pdd.devClass != PD_UNLAB) &&
                         ((deviceInfo.pdd.devStat == PD_OP) ||
                          (deviceInfo.pdd.devStat == PD_INOP)) &&
                         (deviceInfo.pdd.devName[PD_DNAME_CSES] < dimension_of(bayDataList1)) &&
                         (deviceInfo.pdd.devName[PD_DNAME_CSLOT] < dimension_of(bayDataList1->slotDataList)))
                {
                    bayDataPtr = &bayDataList[deviceInfo.pdd.devName[PD_DNAME_CSES]];
                    slotDataPtr = &bayDataPtr->slotDataList[deviceInfo.pdd.devName[PD_DNAME_CSLOT]];
                }
                else
                {
                    /*
                     * Couldn't find SES/Slot number for this PID, and the DNAME
                     * information can't be trusted.  Look for a previously known
                     * location for the drive.
                     */
                    dprintf(DPRINTF_FCM, "UBDL: Disk location could not be found for pid %d\n", pid);
                }

                /* Retrieve the FCAL counters from the drive */
                if ((bayDataPtr != NULL) && (slotDataPtr != NULL))
                {
                    /* Record the Bay Information, if it is not already there */
                    if (bayDataPtr->bayInfo.valid != TRUE)
                    {
                        /*
                         * Get the bay infor associated with this device. If the data is
                         * GOOD, record it. Otherwise skip it and go on.
                         */
                        if ((deviceInfo.pdd.ses < dimension_of(bayDataList1)) &&
                            (GetDiskBayInfoFromBid(deviceInfo.pdd.ses, &bayInfo) == GOOD))
                        {
                            bayDataPtr->bayInfo.bayId = deviceInfo.pdd.ses;
                            bayDataPtr->bayInfo.type = bayInfo.pdd.devType;
                            bayDataPtr->bayInfo.wwn = bayInfo.pdd.wwn;
                            bayDataPtr->bayInfo.valid = TRUE;
                        }
                    }

                    /* Record the drive information into the bayData elements */
                    bayDataPtr->flags.bits.bayPresent = TRUE;
                    slotDataPtr->flags.bits.drivePresent = TRUE;
                    slotDataPtr->wwn = deviceInfo.pdd.wwn;
                    slotDataPtr->channel = deviceInfo.pdd.channel;
                    memcpy(slotDataPtr->devName, deviceInfo.pdd.devName,
                           sizeof(slotDataPtr->devName));

                    switch (deviceInfo.pdd.channel)
                    {
                        case 0:
                        case 1:
                            bayDataPtr->flags.bits.ports0and1 = TRUE;
                            break;

                        case 2:
                        case 3:
                            bayDataPtr->flags.bits.ports2and3 = TRUE;
                            break;

                        default:
                            bayDataPtr->flags.bits.portsUnknown = TRUE;
                            break;
                    }

                    /*
                     * Get the counter data from the supported FCAL drives.
                     * SATA and SSD drives don't provide any counters, so skip
                     * this step for those types of devices.
                     */
                    if (deviceInfo.pdd.devType == PD_DT_FC_DISK ||
                        deviceInfo.pdd.devType == PD_DT_ECON_ENT)
                    {
                        /* Build up the SCSI command CDB to the selected device */
                        memcpy(scsiReq.cdb, "\x4D\x00\x4D\x00\x00\x00\x00\x01\x00\x00", scsiReq.cdbLen);
                        scsiReq.wwnLun.wwn = deviceInfo.pdd.wwn;
                        scsiReq.wwnLun.lun = deviceInfo.pdd.lun;

                        /* Read the log sense page to get the FCAL error counts */
                        if ((commandReturnCode == GOOD) &&
                            (TestFCMUpdateTaskAbortFlag() == FALSE))
                        {
                            commandReturnCode = PacketCommandHandler(&reqPacket, &rspPacket);

                            if (commandReturnCode == GOOD)
                            {
                                commandReturnCode =
                                    FCM_CountersParseLogSense(&slotDataPtr->counters,
                                                              rspPacket.pPacket,
                                                              rspPacket.pHeader->length);

                                if (commandReturnCode != GOOD)
                                {
                                    dprintf(DPRINTF_FCM, "UBDL: Log sense page parsing failed for pid %d\n",
                                            pid);
                                }
                            }
                            else
                            {
                                dprintf(DPRINTF_FCM, "UBDL: Failed to retrieve log sense page from pid: %d, commandReturnCode: %d\n",
                                        pid, commandReturnCode);
                            }

                            /* Free the response data from the logSense command, if it exists */
                            Free(rspPacket.pPacket);
                        }

                        /* Read the factory log page to find the power-on minutes count */
                        if ((commandReturnCode == GOOD) &&
                            (TestFCMUpdateTaskAbortFlag() == FALSE))
                        {
                            memcpy(scsiReq.cdb, "\x4D\x00\x7E\x00\x00\x00\x00\x01\x00\x00",
                                   scsiReq.cdbLen);
                            commandReturnCode = PacketCommandHandler(&reqPacket, &rspPacket);

                            if (commandReturnCode == GOOD)
                            {
                                commandReturnCode = FCM_CountersParseFactoryLog(&slotDataPtr->counters,
                                                                rspPacket.pPacket, rspPacket.pHeader->length);

                                if (commandReturnCode != GOOD)
                                {
                                    dprintf(DPRINTF_FCM, "UBDL: Factory log page parsing failed for pid %d\n",
                                            pid);
                                }
                            }
                            else
                            {
                                dprintf(DPRINTF_FCM, "UBDL: Failed to retrieve factory log page from pid: %d, commandReturnCode: %d\n",
                                        pid, commandReturnCode);
                            }

                            /* Free the response data from the factoryLog command, if it exists */
                            Free(rspPacket.pPacket);
                        }

                        /*
                         * If the counters were retrieved and parsed successfully,
                         * then indicate that the counters are valid.
                         */
                        if (commandReturnCode == GOOD)
                        {
                            slotDataPtr->flags.bits.countersValid = TRUE;
                        }

                        /* Handle SBOD drive bays */
                        if (commandReturnCode == GOOD &&
                            bayDataPtr->bayInfo.type == PD_DT_SBOD_SES)
                        {
                            int         i;

/* The Xyratex SBOD has additional SES data available for each disk drive */
/* in Scsi SES Receive Diagnostics (0x1C) page 0x80 and 0x81.  This is laid */
/* out in structure SES_P80_XTEX (Shared/Inc/SES_Structs.h).  It can only  */
/* be reached on the disk in slot 0 or slot 15. */

                            if (deviceInfo.pdd.slot == 0 || deviceInfo.pdd.slot == 15)
                            {
/* dprintf(DPRINTF_DEFAULT, "UBDL: type 0x%x, disk index %d, BEport/bay/slot %d/%d/%d, SES from disk %d\n", deviceInfo.pdd.devType, diskNumber, deviceInfo.pdd.channel, deviceInfo.pdd.ses, deviceInfo.pdd.slot, deviceInfo.pdd.devName[PD_DNAME_CSLOT]); */
/* dprintf(DPRINTF_DEFAULT, "UBDL: bayInfo  bayId=%u, type=%u, valid=%u, wwn=%qx\n", bayDataPtr->bayInfo.bayId, bayDataPtr->bayInfo.type, bayDataPtr->bayInfo.valid, bayDataPtr->bayInfo.wwn); */

                                for (i = 0x80; i <= 0x81 && commandReturnCode == GOOD; i++)
                                {
                                    /* Build up the SCSI command CDB to the selected device */
                                    if (i == 0x80)
                                    {
                                        scsiReq.cdbLen = sizeof(SCSI_SES_DIAGNOSTIC_PAGE_80) - 1;
                                        memcpy(scsiReq.cdb, SCSI_SES_DIAGNOSTIC_PAGE_80, scsiReq.cdbLen);
                                    }
                                    else
                                    {
                                        scsiReq.cdbLen = sizeof(SCSI_SES_DIAGNOSTIC_PAGE_81) - 1;
                                        memcpy(scsiReq.cdb, SCSI_SES_DIAGNOSTIC_PAGE_81, scsiReq.cdbLen);
                                    }
                                    scsiReq.wwnLun.wwn = deviceInfo.pdd.wwn;
                                    scsiReq.wwnLun.lun = deviceInfo.pdd.lun;
                                    if (TestFCMUpdateTaskAbortFlag() == FALSE)
                                    {
                                        commandReturnCode = PacketCommandHandler(&reqPacket, &rspPacket);
                                        if (commandReturnCode == GOOD)
                                        {
                                            commandReturnCode = FCM_Xyratex_Parse(i, bayDataPtr,
                                                                                  rspPacket.pPacket,
                                                                                  rspPacket.pHeader->length);
                                            if (commandReturnCode != GOOD)
                                            {
                                                dprintf(DPRINTF_FCM, "UBDL: FCM_Xyratex_Parse page parsing failed for pid %d\n",
                                                        pid);
                                            }
                                        }
                                        else
                                        {
                                            dprintf(DPRINTF_FCM, "UBDL: Failed to retrieve Xyratex page 0x%x from pid: %d, commandReturnCode: %d\n",
                                                    i, pid, commandReturnCode);
                                            break;      /* don't do both if first fails. */
                                        }
                                        /* Free the response data from the logSense command, if it exists. */
                                        Free(rspPacket.pPacket);
                                    }
                                }
                            }
                        }
                    }

                    /* Handle SATA drive bays */
                    else if (deviceInfo.pdd.devType == PD_DT_SATA)
                    {
                        if (bayDataPtr->bayInfo.type == PD_DT_SAS_SES)
                        {
                            /* need to add code here */
                        }
                        else
                        {
                            dprintf(DPRINTF_FCM, "UBDL: Processing SATA bay for pid %d\n", pid);

                            /*
                             * Only aquire data if the information is not already
                             * valid for this bay
                             */
                            if (bayDataPtr->flags.bits.counterValid != TRUE)
                            {
                                dprintf(DPRINTF_FCM, "UBDL: Issueing Logsense for pid %d\n", pid);

                                /* Build up the SCSI command CDB to the selected device */
                                memcpy(scsiReq.cdb, "\x4D\x00\x32\x00\x00\x00\x00\x01\x00\x00",
                                       scsiReq.cdbLen);
                                scsiReq.wwnLun.wwn = deviceInfo.pdd.wwn;
                                scsiReq.wwnLun.lun = deviceInfo.pdd.lun;

                                /* Read the log sense page to get the SATA Bay FC error counts */
                                if ((commandReturnCode == GOOD) &&
                                    (TestFCMUpdateTaskAbortFlag() == FALSE))
                                {
                                    commandReturnCode = PacketCommandHandler(&reqPacket, &rspPacket);

                                    if (commandReturnCode == GOOD)
                                    {
                                        commandReturnCode =
                                            FCM_CountersParseArioLogSense(&bayDataPtr->errorCounts,
                                                                          rspPacket.pPacket,
                                                                          rspPacket.pHeader->length);

                                        if (commandReturnCode != GOOD)
                                        {
                                            dprintf(DPRINTF_FCM, "UBDL: Ario Log sense page parsing failed for pid %d\n",
                                                    pid);
                                        }
                                        else
                                        {
                                            /* Flag the counters as being valid */
                                            bayDataPtr->flags.bits.counterValid = TRUE;
                                        }
                                    }
                                    else
                                    {
                                        dprintf(DPRINTF_FCM, "UBDL: Failed to retrieve Ario log sense page from pid: %d, commandReturnCode: %d\n",
                                                pid, commandReturnCode);
                                    }

                                    /* Free the response data from the logSense command, if it exists */
                                    Free(rspPacket.pPacket);
                                }
                            }
                        }       /*bayDataPtr->bayInfo.type != PD_DT_SAS_SES */
                    }           /*if (deviceInfo.pdd.devType == PD_DT_SATA) */
                    else
                    {
                        /*
                         * Device doesn't support any FCAL error counts, so
                         * there's nothing that we need to do here.  This isn't
                         * an ERROR condition... it's simply a 'dumb' device.
                         */
                    }
                }
            }
            else
            {
                dprintf(DPRINTF_FCM, "UBDL: Couldn't get diskInfo for pid: %d\n", pid);
            }
        }

        Free(pDevList);

        /* Return an error if the counter update was aborted */
        if (TestFCMUpdateTaskAbortFlag() == TRUE)
        {
            dprintf(DPRINTF_FCM, "UBDL: Counter update aborted\n");
            returnCode = ERROR;
        }
    }
    else
    {
        dprintf(DPRINTF_FCM, "UBDL: Failed to retrieve PDisk list\n");
        returnCode = ERROR;
    }

    return (returnCode);
}


/*----------------------------------------------------------------------------
**  Function Name: FCM_CountersParseLogSense
**
**  Parameters: countersPtr - Pointer to where counters are to be stored
**              logSensePtr - Pointer to log sense information to be parsed
**              dataLength  - Length of log sense data
**
**  Returns:    GOOD  - Data parsed successfully
**              ERROR - Data not parsed
**--------------------------------------------------------------------------*/
static UINT32 FCM_CountersParseLogSense(FCM_SLOT_DATA_COUNTERS *countersPtr,
                                        UINT8 *logSensePtr, UINT32 dataLength)
{
    UINT32      returnCode = GOOD;
    UINT32      parseValue = 0;
    UINT32      currentByte = 0;
    UINT16      parameterCode = 0;

    ccb_assert(countersPtr != NULL, countersPtr);
    ccb_assert(logSensePtr != NULL, logSensePtr);

    if (countersPtr != NULL)
    {
        /*
         * dprintf( DPRINTF_FCM, "Parse log sense data\n" );
         * for( parseValue = 0; (parseValue < dataLength) && (parseValue < 0x100); parseValue += 16 )
         * {
         * dprintf( DPRINTF_FCM, "  Data[%03x]: %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx  %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx\n",
         * parseValue,
         * logSensePtr[parseValue+0],  logSensePtr[parseValue+1],  logSensePtr[parseValue+2],  logSensePtr[parseValue+3],
         * logSensePtr[parseValue+4],  logSensePtr[parseValue+5],  logSensePtr[parseValue+6],  logSensePtr[parseValue+7],
         * logSensePtr[parseValue+8],  logSensePtr[parseValue+9],  logSensePtr[parseValue+10], logSensePtr[parseValue+11],
         * logSensePtr[parseValue+12], logSensePtr[parseValue+13], logSensePtr[parseValue+14], logSensePtr[parseValue+15] );
         * }
         */

        /*
         * Go through the sense data and look for the counters
         */
        if ((logSensePtr != NULL) &&
            (dataLength >= sizeof(PI_DEBUG_SCSI_CMD_RSP) + (2 * sizeof(UINT16))))
        {
            /*
             * Move the logSense pointer past the header, as this is not
             * included in the length given back to us from the drive.
             */
            logSensePtr += sizeof(PI_DEBUG_SCSI_CMD_RSP);

            /*
             * SCSI command succeeded - grab the counters from the returned data
             * Read and adjust the data length
             */
            currentByte = 2;
            parseValue = bswap_16(*((UINT16 *)&logSensePtr[currentByte]));
            /* dprintf( DPRINTF_FCM, "Page length: %d bytes\n", parseValue );               */

            if (parseValue < dataLength)
            {
                dataLength = parseValue;
            }

            currentByte += 2;

            /*
             * Parse the counters
             */
            while (currentByte < dataLength)
            {
                parameterCode = bswap_16(*((UINT16 *)&logSensePtr[currentByte]));

                switch (parameterCode)
                {
                    case 0x0000:
                        /* dprintf( DPRINTF_FCM, "Temperature data\n" );                    */
                        currentByte += 6;
                        break;

                    case 0x0001:
                        /* dprintf( DPRINTF_FCM, "Reference temperature data\n" );          */
                        currentByte += 6;
                        break;

                    case 0x0002:
                        /* dprintf( DPRINTF_FCM, "Undocumented temperature data\n" );       */
                        currentByte += 6;
                        break;

                    case 0x80FF:
                        parseValue = *((UINT8 *)(&logSensePtr[currentByte + 5]));
                        countersPtr->commandInitiatePort = (UINT8)parseValue;
                        /* dprintf( DPRINTF_FCM, "CIP:      0x%0x\n", parseValue );         */
                        currentByte += 6;
                        break;

                    case 0x8100:
                        parseValue = bswap_32(*((UINT32 *)&logSensePtr[currentByte + 4]));
                        countersPtr->portA.linkFail = parseValue;
                        /* dprintf( DPRINTF_FCM, "LFCNT_A:  0x%0x\n", parseValue );         */
                        currentByte += 8;
                        break;

                    case 0x8101:
                        parseValue = bswap_32(*((UINT32 *)&logSensePtr[currentByte + 4]));
                        countersPtr->portA.lostSync = parseValue;
                        /* dprintf( DPRINTF_FCM, "LSCNT_A:  0x%0x\n", parseValue );         */
                        currentByte += 8;
                        break;

                    case 0x8102:
                    case 0x8103:
                        /* dprintf( DPRINTF_FCM, "Undocumented parameter data\n" );         */
                        currentByte += 8;
                        break;

                    case 0x8104:
                        parseValue = bswap_32(*((UINT32 *)&logSensePtr[currentByte + 4]));
                        countersPtr->portA.invalidTransmit = parseValue;
                        /* dprintf( DPRINTF_FCM, "ITCNT_A:  0x%0x\n", parseValue );         */
                        currentByte += 8;
                        break;

                    case 0x8105:
                        parseValue = bswap_32(*((UINT32 *)&logSensePtr[currentByte + 4]));
                        countersPtr->portA.invalidCRC = parseValue;
                        /* dprintf( DPRINTF_FCM, "ICCNT_A:  0x%0x\n", parseValue );         */
                        currentByte += 8;
                        break;

                    case 0x8106:
                        parseValue = bswap_32(*((UINT32 *)&logSensePtr[currentByte + 4]));
                        countersPtr->portA.lipF7Initiated = parseValue;
                        /* dprintf( DPRINTF_FCM, "LIPF7I_A: 0x%0x\n", parseValue );         */
                        currentByte += 8;
                        break;

                    case 0x8107:
                        parseValue = bswap_32(*((UINT32 *)&logSensePtr[currentByte + 4]));
                        countersPtr->portA.lipF7Received = parseValue;
                        /* dprintf( DPRINTF_FCM, "LIPF7R_A: 0x%0x\n", parseValue );         */
                        currentByte += 8;
                        break;

                    case 0x8108:
                        parseValue = bswap_32(*((UINT32 *)&logSensePtr[currentByte + 4]));
                        countersPtr->portA.lipF8Initiated = parseValue;
                        /* dprintf( DPRINTF_FCM, "LIPF8I_A: 0x%0x\n", parseValue );         */
                        currentByte += 8;
                        break;

                    case 0x8109:
                        parseValue = bswap_32(*((UINT32 *)&logSensePtr[currentByte + 4]));
                        countersPtr->portA.lipF8Received = parseValue;
                        /* dprintf( DPRINTF_FCM, "LIPF8R_A: 0x%0x\n", parseValue );         */
                        currentByte += 8;
                        break;

                    case 0x8110:
                        parseValue = bswap_32(*((UINT32 *)&logSensePtr[currentByte + 4]));
                        countersPtr->portB.linkFail = parseValue;
                        /* dprintf( DPRINTF_FCM, "LFCNT_B:  0x%0x\n", parseValue );         */
                        currentByte += 8;
                        break;

                    case 0x8111:
                        parseValue = bswap_32(*((UINT32 *)&logSensePtr[currentByte + 4]));
                        countersPtr->portB.lostSync = parseValue;
                        /* dprintf( DPRINTF_FCM, "LSCNT_B:  0x%0x\n", parseValue );         */
                        currentByte += 8;
                        break;

                    case 0x8112:
                    case 0x8113:
                        /* dprintf( DPRINTF_FCM, "Undocumented parameter data\n" );         */
                        currentByte += 8;
                        break;

                    case 0x8114:
                        parseValue = bswap_32(*((UINT32 *)&logSensePtr[currentByte + 4]));
                        countersPtr->portB.invalidTransmit = parseValue;
                        /* dprintf( DPRINTF_FCM, "ITCNT_B:  0x%0x\n", parseValue );         */
                        currentByte += 8;
                        break;

                    case 0x8115:
                        parseValue = bswap_32(*((UINT32 *)&logSensePtr[currentByte + 4]));
                        countersPtr->portB.invalidCRC = parseValue;
                        /* dprintf( DPRINTF_FCM, "ICCNT_B:  0x%0x\n", parseValue );         */
                        currentByte += 8;
                        break;

                    case 0x8116:
                        parseValue = bswap_32(*((UINT32 *)&logSensePtr[currentByte + 4]));
                        countersPtr->portB.lipF7Initiated = parseValue;
                        /* dprintf( DPRINTF_FCM, "LIPF7I_B: 0x%0x\n", parseValue );         */
                        currentByte += 8;
                        break;

                    case 0x8117:
                        parseValue = bswap_32(*((UINT32 *)&logSensePtr[currentByte + 4]));
                        countersPtr->portB.lipF7Received = parseValue;
                        /* dprintf( DPRINTF_FCM, "LIPF7R_B: 0x%0x\n", parseValue );         */
                        currentByte += 8;
                        break;

                    case 0x8118:
                        parseValue = bswap_32(*((UINT32 *)&logSensePtr[currentByte + 4]));
                        countersPtr->portB.lipF8Initiated = parseValue;
                        /* dprintf( DPRINTF_FCM, "LIPF8I_B: 0x%0x\n", parseValue );         */
                        currentByte += 8;
                        break;

                    case 0x8119:
                        parseValue = bswap_32(*((UINT32 *)&logSensePtr[currentByte + 4]));
                        countersPtr->portB.lipF8Received = parseValue;
                        /* dprintf( DPRINTF_FCM, "LIPF8R_B: 0x%0x\n", parseValue );         */
                        currentByte += 8;
                        break;

                    default:
                        /* dprintf( DPRINTF_FCM, "Unknown:  0x%04x\n", parameterCode );     */
                        currentByte += 2;
                        break;
                }
            }
        }
        else
        {
            dprintf(DPRINTF_FCM, "CPLS: Too little data to parse\n");
            returnCode = ERROR;
        }
    }
    else
    {
        dprintf(DPRINTF_FCM, "CPLS: countersPtr is NULL\n");
        returnCode = ERROR;
    }

    return (returnCode);
}

/*----------------------------------------------------------------------------
**  Function Name: FCM_CountersParseArioLogSense
**
**  Parameters: countersPtr - Pointer to where counters are to be stored
**              logSensePtr - Pointer to log sense information to be parsed
**              dataLength  - Length of log sense data
**
**  Returns:    GOOD  - Data parsed successfully
**              ERROR - Data not parsed
**--------------------------------------------------------------------------*/
static UINT32 FCM_CountersParseArioLogSense(FCM_BAY_ERROR_COUNTERS *countersPtr,
                                            UINT8 *packetPtr, UINT32 dataLength)
{
    UINT32      returnCode = GOOD;
    FCM_ARIO_LOG_SENSE *logSensePtr;
    UINT16      i;

    ccb_assert(countersPtr != NULL, countersPtr);
    ccb_assert(packetPtr != NULL, packetPtr);

    if (countersPtr != NULL)
    {

        /*
         * Validity check the packet and log sense header
         */
        if (dataLength >= (sizeof(PI_DEBUG_SCSI_CMD_RSP) + sizeof(FCM_ARIO_LOG_SENSE)))
        {
            /*
             * Get the logsense data from within the returned packet.
             */
            logSensePtr = (FCM_ARIO_LOG_SENSE *)(packetPtr + sizeof(PI_DEBUG_SCSI_CMD_RSP));

            /*
             * Copy out the log data for each of the 2 ports
             */

            for (i = 0; i < FCM_EURO_NUM_BAY_PORTS; ++i)
            {
                countersPtr->port[i][0].linkFail =
                    (UINT32)(bswap_64(logSensePtr->logSenseDataPort[i].linkFailureCount));
                countersPtr->port[i][0].lostSync =
                    (UINT32)(bswap_64(logSensePtr->logSenseDataPort[i].lossOfSyncCount));
                countersPtr->port[i][0].lostSignal =
                    (UINT32)(bswap_64(logSensePtr->logSenseDataPort[i].lossOfSignalCount));
                countersPtr->port[i][0].sequenceError =
                    (UINT32)(bswap_64(logSensePtr->logSenseDataPort[i].primitiveSequenceErrorCount));
                countersPtr->port[i][0].invalidTransmit =
                    (UINT32)(bswap_64(logSensePtr->logSenseDataPort[i].invalidTransmittedWordCount));
                countersPtr->port[i][0].invalidCRC =
                    (UINT32)(bswap_64(logSensePtr->logSenseDataPort[i].invalidCRCCount));

            }
        }
        else
        {
            dprintf(DPRINTF_FCM, "CPLS: Too little data to parse\n");
            returnCode = ERROR;
        }
    }
    else
    {
        dprintf(DPRINTF_FCM, "CPLS: countersPtr is NULL\n");
        returnCode = ERROR;
    }

    return (returnCode);
}


/*----------------------------------------------------------------------------
**  Function Name: FCM_CountersParseFactoryLog
**
**  Parameters: countersPtr - Pointer to where counters are to be stored
**              logSensePtr - Pointer to log sense information to be parsed
**              dataLength  - Length of factory log data
**
**  Returns:    GOOD  - Data parsed successfully
**              ERROR - Data not parsed
**--------------------------------------------------------------------------*/
static UINT32 FCM_CountersParseFactoryLog(FCM_SLOT_DATA_COUNTERS *countersPtr,
                                          UINT8 *factoryLogPtr, UINT32 dataLength)
{
    UINT32      returnCode = GOOD;
    UINT32      parseValue = 0;
    UINT32      currentByte = 0;
    UINT32      fieldLength = 0;
    UINT16      parameterCode = 0;

    ccb_assert(countersPtr != NULL, countersPtr);
    ccb_assert(factoryLogPtr != NULL, factoryLogPtr);

    if (countersPtr != NULL)
    {
        /*
         * dprintf( DPRINTF_FCM, "Parse factory log data\n" );
         * for( parseValue = 0; (parseValue < dataLength) && (parseValue < 0x100); parseValue += 16 )
         * {
         * dprintf( DPRINTF_FCM, "  Data[%03x]: %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx  %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx\n",
         * parseValue,
         * factoryLogPtr[parseValue+0],  factoryLogPtr[parseValue+1],  factoryLogPtr[parseValue+2],  factoryLogPtr[parseValue+3],
         * factoryLogPtr[parseValue+4],  factoryLogPtr[parseValue+5],  factoryLogPtr[parseValue+6],  factoryLogPtr[parseValue+7],
         * factoryLogPtr[parseValue+8],  factoryLogPtr[parseValue+9],  factoryLogPtr[parseValue+10], factoryLogPtr[parseValue+11],
         * factoryLogPtr[parseValue+12], factoryLogPtr[parseValue+13], factoryLogPtr[parseValue+14], factoryLogPtr[parseValue+15] );
         * }
         */

        /*
         * Go through the factory log data and look for the counters
         */
        if ((factoryLogPtr != NULL) &&
            (dataLength >= sizeof(PI_DEBUG_SCSI_CMD_RSP) + (2 * sizeof(UINT16))))
        {
            /*
             * Move the factoryLog pointer past the header, as this is not
             * included in the length given back to us from the drive.
             */
            factoryLogPtr += sizeof(PI_DEBUG_SCSI_CMD_RSP);

            /*
             * SCSI command succeeded - grab the counters from the returned data
             * Read and adjust the data length
             */
            currentByte = 2;
            parseValue = bswap_16(*((UINT16 *)&factoryLogPtr[currentByte]));
            /* dprintf( DPRINTF_FCM, "Page length: %d bytes\n", parseValue );               */

            if (parseValue < dataLength)
            {
                dataLength = parseValue;
            }

            currentByte += 2;

            /*
             * Parse the counters
             */
            while (currentByte < dataLength)
            {
                parameterCode = bswap_16(*((UINT16 *)&factoryLogPtr[currentByte]));

                fieldLength = *((UINT8 *)(&factoryLogPtr[currentByte + 3]));
                /* dprintf( DPRINTF_FCM, "Length:   0x%0x\n", fieldLength );                */

                switch (fieldLength)
                {
                    case 1:
                        parseValue = *((UINT8 *)(&factoryLogPtr[currentByte + 4]));
                        currentByte += 5;
                        break;

                    case 2:
                        parseValue = bswap_16(*((UINT16 *)&factoryLogPtr[currentByte + 4]));
                        currentByte += 6;
                        break;

                    case 4:
                        parseValue = bswap_32(*((UINT32 *)&factoryLogPtr[currentByte + 4]));
                        currentByte += 8;
                        break;

                    default:
                        parameterCode = -1;
                        parseValue = 0;
                        currentByte += 4;
                        break;
                }

                switch (parameterCode)
                {
                    case 0x0000:
                        countersPtr->powerOnMinutes = parseValue;
                        /* dprintf( DPRINTF_FCM, "POM:      0x%0x\n", parseValue );         */
                        break;

                    case 0x0008:
                        /* dprintf( DPRINTF_FCM, "SMART:    0x%0x\n", parseValue );         */
                        break;

                    default:
                        /* dprintf( DPRINTF_FCM, "Unknown:  0x%04x\n", parameterCode );     */
                        break;
                }
            }
        }
        else
        {
            dprintf(DPRINTF_FCM, "CPFL: Too little data to parse\n");
            returnCode = ERROR;
        }
    }
    else
    {
        dprintf(DPRINTF_FCM, "CPFL: countersPtr is NULL\n");
        returnCode = ERROR;
    }

    return (returnCode);
}


/*----------------------------------------------------------------------------
**  Function Name: Xyratex2int
**
**  Parameters: page       - Page 0x80 or 0x81 being processed.
**              bayDataPtr - Pointer to where bay counters are to be stored.
**              packet     - Pointer to SES information to be parsed.
**              dataLength - Length of SES data.
**
**  Returns:    GOOD  - Data parsed successfully
**              ERROR - Data not parsed
**--------------------------------------------------------------------------*/

static UINT32 Xyratex2int(UINT8 input)
{
    int         exponent;
    int         mantissa;

    exponent = (input >> 4);    /* upper four bits are exponent */
    exponent = (exponent == 0) ? 1 : (exponent << 1);
    mantissa = input & 0xf;     /* lower four bits are mantissa */
    return (exponent * mantissa);
}


/*----------------------------------------------------------------------------
**  Function Name: FCM_Xyratex_Parse
**
**  Parameters: page       - Page 0x80 or 0x81 being processed.
**              bayDataPtr - Pointer to where bay counters are to be stored.
**              packet     - Pointer to SES information to be parsed.
**              dataLength - Length of SES data.
**
**  Returns:    GOOD  - Data parsed successfully
**              ERROR - Data not parsed
**--------------------------------------------------------------------------*/

static UINT32 FCM_Xyratex_Parse(int page, FCM_BAY_DATA *bayDataPtr, UINT8 *packet,
                                UINT32 dataLength)
{

    UINT16      i;
    SES_P80XTEXPort *s;
    FCM_FCAL_ERROR_COUNTERS *linkstats;
    FCM_BAY_PORT_ERROR_COUNTERS *baystats;
    UINT16      count;
    PSES_P80_XTEX packetPtr;

    if (dataLength < (sizeof(PI_DEBUG_SCSI_CMD_RSP) + sizeof(FCM_ARIO_LOG_SENSE)))
    {
        dprintf(DPRINTF_FCM, "dataLength (%u) not < %u\n", dataLength,
                sizeof(PI_DEBUG_SCSI_CMD_RSP) + sizeof(FCM_ARIO_LOG_SENSE));
        return (ERROR);
    }

    /*
     * Get the data from within the returned packet.
     */
    packetPtr = (PSES_P80_XTEX)(packet + sizeof(PI_DEBUG_SCSI_CMD_RSP));

    /*
     * A few simple checks.
     */
    if (packetPtr->PageCode != page)
    {
        dprintf(DPRINTF_FCM, "PageCode (0x%x) != 0x%x\n", packetPtr->PageCode, page);
        return (ERROR);
    }
    count = bswap_16(packetPtr->Length);
    if (count == 0)
    {
        dprintf(DPRINTF_FCM, "Length (0x%x) == 0\n", packetPtr->Length);
        return (ERROR);
    }
    count = (count - 10) / 8;
    if (count != 20)
    {
        dprintf(DPRINTF_FCM, "Count (%d) != 20\n", count);
        return (ERROR);
    }

/* 0=hostport 0, 1=hostport 1, 2=hostport 2, 3=hostport 3, */
    for (i = 0; i < 4; i++)
    {
        s = (SES_P80XTEXPort *)((char *)&(packetPtr->Port[0]) +
                                 (i * sizeof(SES_P80XTEXPort)));
        if (page == 0x80)
        {
            baystats = &bayDataPtr->errorCounts.port[i][0];
        }
        else
        {
            baystats = &bayDataPtr->errorCounts.port[i][1];
        }
        baystats->WordErrorCount = Xyratex2int(s->WordErrorCount);
        baystats->CRCErrorCount = Xyratex2int(s->CRCErrorCount);
        baystats->ClockDelta = ((signed char)s->ClockDelta) * 8;
        baystats->LoopUpCount = s->LoopUpCount;
        baystats->InsertionCount = s->InsertionCount;
        baystats->StallCount = s->StallCount;
        baystats->Utilization = s->Utilization;
    }

/* 4=drive slot 0, ... */

    for (i = 4; i < count; i++)
    {
        s = (SES_P80XTEXPort *)((char *)&(packetPtr->Port[0]) +
                                 (i * sizeof(SES_P80XTEXPort)));

/* dprintf(DPRINTF_DEFAULT, "SBOD   %2d State=0x%x, Word=%d CRC=%d Clock=%d LoopUp=%d Insert=%d Stall=%d Util=%d\n", i-4, s->StateCode, s->WordErrorCount, s->CRCErrorCount, s->ClockDelta, s->LoopUpCount, s->InsertionCount, s->StallCount, s->Utilization); */
        if (page == 0x80)
        {
            linkstats = &bayDataPtr->slotDataList[i - 4].counters.portA;
        }
        else
        {
            linkstats = &bayDataPtr->slotDataList[i - 4].counters.portB;
        }
        linkstats->WordErrorCount = Xyratex2int(s->WordErrorCount);
        linkstats->CRCErrorCount = Xyratex2int(s->CRCErrorCount);
        linkstats->ClockDelta = ((signed char)s->ClockDelta) * 8;
        linkstats->LoopUpCount = s->LoopUpCount;
        linkstats->InsertionCount = s->InsertionCount;
        linkstats->StallCount = s->StallCount;
        linkstats->Utilization = s->Utilization;

/* dprintf(DPRINTF_DEFAULT, "SBOD   %2d            Word=%d CRC=%d Clock=%d LoopUp=%d Insert=%d Stall=%d Util=%d\n", i-4, linkstats->WordErrorCount, linkstats->CRCErrorCount, linkstats->ClockDelta, linkstats->LoopUpCount, linkstats->InsertionCount, linkstats->StallCount, linkstats->Utilization); */
    }

    return (GOOD);
}


/*----------------------------------------------------------------------------
**  Function Name: FCM_CountersInitializeErrorData
**
**  Parameters: bayDataListPtr - Pointer to bayData list to be initialized
**
**  Returns:    GOOD  - Map initialized
**              ERROR - Map not initialized (bad input pointer)
**--------------------------------------------------------------------------*/
static UINT32 FCM_CountersInitializeErrorData(FCM_ERROR_DATA *errorDataPtr)
{
    UINT32      returnCode = GOOD;

    ccb_assert(errorDataPtr != NULL, errorDataPtr);

    if (errorDataPtr != NULL)
    {
        /*
         * Initialize all of the fields in errorData
         */
        memset(&errorDataPtr->beginTimestamp, 0, sizeof(errorDataPtr->beginTimestamp));
        memset(&errorDataPtr->endTimestamp, 0, sizeof(errorDataPtr->endTimestamp));

        if ((errorDataPtr->habDataList == NULL) ||
            (FCM_CountersInitializeHabDataList(errorDataPtr->habDataList) != GOOD))
        {
            returnCode = ERROR;
        }

        if ((errorDataPtr->bayDataList == NULL) ||
            (FCM_CountersInitializeBayDataList(errorDataPtr->bayDataList) != GOOD))
        {
            returnCode = ERROR;
        }
    }
    else
    {
        returnCode = ERROR;
    }

    return (returnCode);
}


/*----------------------------------------------------------------------------
**  Function Name: FCM_CountersInitializeHabDataList
**
**  Parameters: habDataListPtr - Pointer to habData list to be initialized
**
**  Returns:    GOOD  - Map initialized
**              ERROR - Map not initialized (bad input pointer)
**--------------------------------------------------------------------------*/
static UINT32 FCM_CountersInitializeHabDataList(FCM_HAB_DATA *habDataListPtr)
{
    UINT32      returnCode = GOOD;
    UINT32      habCounter = 0;

    ccb_assert(habDataListPtr != NULL, habDataListPtr);

    if (habDataListPtr != NULL)
    {
        /*
         * Initialize all of the fields in habDataList
         */
        for (habCounter = 0; habCounter < dimension_of(habDataList0); habCounter++)
        {
            FCM_CountersInitializeHabData(&habDataListPtr[habCounter]);
        }
    }
    else
    {
        returnCode = ERROR;
    }

    return (returnCode);
}


/*----------------------------------------------------------------------------
**  Function Name: FCM_CountersInitializeHabData
**
**  Parameters: habDataPtr - Pointer to habData to be initialized
**
**  Returns:    GOOD  - habData initialized
**              ERROR - habData not initialized (bad input pointer)
**--------------------------------------------------------------------------*/
static UINT32 FCM_CountersInitializeHabData(FCM_HAB_DATA *habDataPtr)
{
    UINT32      returnCode = GOOD;

    ccb_assert(habDataPtr != NULL, habDataPtr);

    if (habDataPtr != NULL)
    {
        /*
         * Initialize all of the fields in habData
         */
        habDataPtr->flags.value = 0;
        memset(&habDataPtr->habCounters, -1, sizeof(habDataPtr->habCounters));
    }
    else
    {
        returnCode = ERROR;
    }

    return (returnCode);
}


/*----------------------------------------------------------------------------
**  Function Name: FCM_CountersInitializeBayDataList
**
**  Parameters: bayDataListPtr - Pointer to bayData list to be initialized
**
**  Returns:    GOOD  - Map initialized
**              ERROR - Map not initialized (bad input pointer)
**--------------------------------------------------------------------------*/
static UINT32 FCM_CountersInitializeBayDataList(FCM_BAY_DATA *bayDataListPtr)
{
    UINT32      returnCode = GOOD;
    UINT32      bayCounter = 0;

    ccb_assert(bayDataListPtr != NULL, bayDataListPtr);

    if (bayDataListPtr != NULL)
    {
        /*
         * Initialize all of the fields in bayDataList
         */
        for (bayCounter = 0; bayCounter < dimension_of(bayDataList1); bayCounter++)
        {
            FCM_CountersInitializeBayData(&bayDataListPtr[bayCounter]);
        }
    }
    else
    {
        returnCode = ERROR;
    }

    return (returnCode);
}


/*----------------------------------------------------------------------------
**  Function Name: FCM_CountersInitializeBayData
**
**  Parameters: countersPtr - Pointer to bayDataPtr to be initialized
**
**  Returns:    GOOD  - slotData initialized
**              ERROR - slotData not initialized (bad input pointer)
**--------------------------------------------------------------------------*/
static UINT32 FCM_CountersInitializeBayData(FCM_BAY_DATA *bayDataPtr)
{
    UINT32      returnCode = GOOD;
    UINT32      slotCounter = 0;

    ccb_assert(bayDataPtr != NULL, bayDataPtr);

    if (bayDataPtr != NULL)
    {
        /*
         * Initialize all of the fields in bayDataPtr
         */
        bayDataPtr->flags.value = 0;

        for (slotCounter = 0; slotCounter < dimension_of(bayDataPtr->slotDataList); slotCounter++)
        {
            FCM_CountersInitializeSlotData(&bayDataPtr->slotDataList[slotCounter]);
        }

        FCM_CountersInitializeBayInfo(&bayDataPtr->bayInfo);
        FCM_CountersInitializeBayErrorCounters(&bayDataPtr->errorCounts);
    }
    else
    {
        returnCode = ERROR;
    }

    return (returnCode);
}


/*----------------------------------------------------------------------------
**  Function Name: FCM_CountersInitializeSlotData
**
**  Parameters: countersPtr - Pointer to slotData to be initialized
**
**  Returns:    GOOD  - slotData initialized
**              ERROR - slotData not initialized (bad input pointer)
**--------------------------------------------------------------------------*/
static UINT32 FCM_CountersInitializeSlotData(FCM_SLOT_DATA *slotDataPtr)
{
    UINT32      returnCode = GOOD;

    ccb_assert(slotDataPtr != NULL, slotDataPtr);

    if (slotDataPtr != NULL)
    {
        /*
         * Initialize all of the fields in slotDataPtr
         */
        slotDataPtr->flags.value = 0;
        slotDataPtr->wwn = 0;
        FCM_CountersInitializeSlotDataCounters(&slotDataPtr->counters);
    }
    else
    {
        returnCode = ERROR;
    }

    return (returnCode);
}


/*----------------------------------------------------------------------------
**  Function Name: FCM_CountersInitializeSlotDataCounters
**
**  Parameters: countersPtr - Pointer to counters to be initialized
**
**  Returns:    GOOD  - Counters initialized
**              ERROR - Counters not initialized (bad input pointer)
**--------------------------------------------------------------------------*/
static UINT32 FCM_CountersInitializeSlotDataCounters(FCM_SLOT_DATA_COUNTERS *countersPtr)
{
    UINT32      returnCode = GOOD;

    ccb_assert(countersPtr != NULL, countersPtr);

    if (countersPtr != NULL)
    {
        /*
         * All fields are initialized to all ones
         */
        memset(countersPtr, -1, sizeof(*countersPtr));
    }
    else
    {
        returnCode = ERROR;
    }

    return (returnCode);
}

/*----------------------------------------------------------------------------
**  Function Name: FCM_CountersInitializeBayInfo
**
**  Parameters: infoPtr - Pointer to data to be initialized
**
**  Returns:    GOOD  - Counters initialized
**              ERROR - Counters not initialized (bad input pointer)
**--------------------------------------------------------------------------*/
static UINT32 FCM_CountersInitializeBayInfo(FCM_BAY_INFO *infoPtr)
{
    UINT32      returnCode = GOOD;

    ccb_assert(infoPtr != NULL, infoPtr);

    if (infoPtr != NULL)
    {
        /*
         * All fields are initialized to all zeros.
         */
        memset(infoPtr, 0, sizeof(*infoPtr));
    }
    else
    {
        returnCode = ERROR;
    }

    return (returnCode);
}


/*----------------------------------------------------------------------------
**  Function Name: FCM_CountersInitializeBayErrorCounters
**
**  Parameters: countersPtr - Pointer to counters to be initialized
**
**  Returns:    GOOD  - Counters initialized
**              ERROR - Counters not initialized (bad input pointer)
**--------------------------------------------------------------------------*/
static UINT32 FCM_CountersInitializeBayErrorCounters(FCM_BAY_ERROR_COUNTERS *countersPtr)
{
    UINT32      returnCode = GOOD;

    ccb_assert(countersPtr != NULL, countersPtr);

    if (countersPtr != NULL)
    {
        /*
         * All fields are initialized to all ones
         */
        memset(countersPtr, -1, sizeof(*countersPtr));
    }
    else
    {
        returnCode = ERROR;
    }

    return (returnCode);
}



/*----------------------------------------------------------------------------
**  Function Name: FCM_CountersDumpMap
**
**  Parameters: counterMapPtr - Pointer to counterMap to be dumped
**
**  Returns:    GOOD  - Map dumped
**              ERROR - Map not dumped (bad input pointer)
**--------------------------------------------------------------------------*/
UINT32 FCM_CountersDumpMap(FCM_COUNTER_MAP *counterMapPtr)
{
    UINT32      returnCode = GOOD;

    ccb_assert(counterMapPtr != NULL, counterMapPtr);

    if (counterMapPtr != NULL)
    {
        /*
         * Acquire the mutex
         */
        (void)LockMutex(&counterMapPtr->header.busyMutex, MUTEX_WAIT);

        dprintf(DPRINTF_DEFAULT, "CDM: Dumping the counterMap\n");
        dprintf(DPRINTF_DEFAULT, "  Version:           %u\n",
                counterMapPtr->header.version);
        dprintf(DPRINTF_DEFAULT, "  Number of BE HABs: %u\n",
                counterMapPtr->header.numberOfBackEndHabs);
        dprintf(DPRINTF_DEFAULT, "  Number of bays:    %u\n",
                counterMapPtr->header.numberOfBays);
        dprintf(DPRINTF_DEFAULT, "  Number of slots:   %u\n",
                counterMapPtr->header.numberOfSlotsInBay);
        dprintf(DPRINTF_DEFAULT, "  sizeCounterMap:    %u\n",
                counterMapPtr->header.sizeCounterMap);
        dprintf(DPRINTF_DEFAULT, "  sizeErrorData:     %u\n",
                counterMapPtr->header.sizeErrorData);
        dprintf(DPRINTF_DEFAULT, "  sizeHabData:       %u\n",
                counterMapPtr->header.sizeHabData);
        dprintf(DPRINTF_DEFAULT, "  sizeBayData:       %u\n",
                counterMapPtr->header.sizeBayData);
        dprintf(DPRINTF_DEFAULT, "  Flags:             0x%08x\n",
                counterMapPtr->flags.value);

        if (counterMapPtr->flags.bits.baselineMapValid == TRUE)
        {
            dprintf(DPRINTF_DEFAULT, "CDM: Dumping the baseline map\n");
            FCM_CountersDumpErrorData(&counterMapPtr->baselineData);
        }

        if (counterMapPtr->flags.bits.updateMapValid == TRUE)
        {
            dprintf(DPRINTF_DEFAULT, "CDM: Dumping the update map\n");
            FCM_CountersDumpErrorData(&counterMapPtr->updateData);
        }

        if (counterMapPtr->flags.bits.deltaMapValid == TRUE)
        {
            dprintf(DPRINTF_DEFAULT, "CDM: Dumping the delta map\n");
            FCM_CountersDumpErrorData(&counterMapPtr->deltaData);
        }

        if (counterMapPtr->flags.bits.backup0MapValid == TRUE)
        {
            dprintf(DPRINTF_DEFAULT, "CDM: Dumping the backup map 0\n");
            FCM_CountersDumpErrorData(&counterMapPtr->backupData0);
        }

        if (counterMapPtr->flags.bits.backup1MapValid == TRUE)
        {
            dprintf(DPRINTF_DEFAULT, "CDM: Dumping the backup map 1\n");
            FCM_CountersDumpErrorData(&counterMapPtr->backupData1);
        }

        if (counterMapPtr->flags.bits.backup2MapValid == TRUE)
        {
            dprintf(DPRINTF_DEFAULT, "CDM: Dumping the backup map 2\n");
            FCM_CountersDumpErrorData(&counterMapPtr->backupData2);
        }

        if (counterMapPtr->flags.bits.backup3MapValid == TRUE)
        {
            dprintf(DPRINTF_DEFAULT, "CDM: Dumping the backup map 3\n");
            FCM_CountersDumpErrorData(&counterMapPtr->backupData3);
        }

        /*
         * Release the mutex
         */
        UnlockMutex(&counterMapPtr->header.busyMutex);
    }
    else
    {
        returnCode = ERROR;
    }

    return (returnCode);
}


/*----------------------------------------------------------------------------
**  Function Name: FCM_CountersDumpErrorData
**
**  Parameters: errorDataPtr - Pointer to errorData to be dumped
**
**  Returns:    GOOD  - Map dumped
**              ERROR - Map not dumped (bad input pointer)
**--------------------------------------------------------------------------*/
static UINT32 FCM_CountersDumpErrorData(FCM_ERROR_DATA *errorDataPtr)
{
    UINT32      returnCode = GOOD;

    ccb_assert(errorDataPtr != NULL, errorDataPtr);

    if (errorDataPtr != NULL)
    {
        dprintf(DPRINTF_DEFAULT, "  Timestamps:          %02X/%02X/%04X %02X:%02X:%02X  -to-  %02X/%02X/%04X %02X:%02X:%02X\n",
                errorDataPtr->beginTimestamp.month, errorDataPtr->beginTimestamp.date,
                errorDataPtr->beginTimestamp.year, errorDataPtr->beginTimestamp.hours,
                errorDataPtr->beginTimestamp.minutes,
                errorDataPtr->beginTimestamp.seconds, errorDataPtr->endTimestamp.month,
                errorDataPtr->endTimestamp.date, errorDataPtr->endTimestamp.year,
                errorDataPtr->endTimestamp.hours, errorDataPtr->endTimestamp.minutes,
                errorDataPtr->endTimestamp.seconds);

        FCM_CountersDumpHabDataList(errorDataPtr->habDataList);
        FCM_CountersDumpBayDataList(errorDataPtr->bayDataList);
    }
    else
    {
        returnCode = ERROR;
    }

    return (returnCode);
}


/*----------------------------------------------------------------------------
**  Function Name: FCM_CountersDumpHabDataList
**
**  Parameters: habDataListPtr - Pointer to habDataList to be dumped
**
**  Returns:    GOOD  - habDataList dumped
**              ERROR - habDataList not dumped (bad input pointer)
**--------------------------------------------------------------------------*/
static UINT32 FCM_CountersDumpHabDataList(FCM_HAB_DATA *habDataListPtr)
{
    UINT32      returnCode = GOOD;
    UINT32      habCounter = 0;
    FCM_HAB_ERROR_COUNTERS *errorCountersPtr = NULL;
    UINT8       linePrinted = FALSE;
    char        linkFailString[COUNTER_STRING_LENGTH] = { 0 };
    char        lostSyncString[COUNTER_STRING_LENGTH] = { 0 };
    char        lostSignalString[COUNTER_STRING_LENGTH] = { 0 };
    char        sequenceErrorString[COUNTER_STRING_LENGTH] = { 0 };
    char        invXmitString[COUNTER_STRING_LENGTH] = { 0 };
    char        invCrcString[COUNTER_STRING_LENGTH] = { 0 };

    ccb_assert(habDataListPtr != NULL, habDataListPtr);

    if (habDataListPtr != NULL)
    {
        dprintf(DPRINTF_DEFAULT, "         PORT LINK_FAIL  LOST_SYNC   INV_XMIT   INV_CRC    LOST_SIG  SEQ_ERROR\n");
        dprintf(DPRINTF_DEFAULT, "         ---- ---------- ---------- ---------- ---------- ---------- ----------\n");

        for (habCounter = 0; habCounter < dimension_of(habDataList0); habCounter++)
        {
            linePrinted = FALSE;

            if (habDataListPtr[habCounter].flags.bits.habPresent == TRUE)
            {
                if (habDataListPtr[habCounter].flags.bits.countersValid == TRUE)
                {
                    errorCountersPtr = &habDataListPtr[habCounter].habCounters;

                    if (errorCountersPtr != NULL)
                    {
                        FCM_CountersToString(linkFailString, errorCountersPtr->linkFail,
                                             sizeof(linkFailString));
                        FCM_CountersToString(lostSyncString, errorCountersPtr->lostSync,
                                             sizeof(lostSyncString));
                        FCM_CountersToString(lostSignalString,
                                             errorCountersPtr->lostSignal,
                                             sizeof(lostSignalString));
                        FCM_CountersToString(sequenceErrorString,
                                             errorCountersPtr->sequenceError,
                                             sizeof(sequenceErrorString));
                        FCM_CountersToString(invXmitString,
                                             errorCountersPtr->invalidTransmit,
                                             sizeof(invXmitString));
                        FCM_CountersToString(invCrcString, errorCountersPtr->invalidCRC,
                                             sizeof(invCrcString));

                        dprintf(DPRINTF_DEFAULT, "            %1d %10s %10s %10s %10s %10s %10s\n",
                                habCounter, linkFailString, lostSyncString, invXmitString,
                                invCrcString, lostSignalString, sequenceErrorString);

                        linePrinted = TRUE;

                        /*
                         * Allow other tasks to run, since we might be
                         * flooding the serial port here.
                         */
                        TaskSwitch();
                    }
                    else
                    {
                        dprintf(DPRINTF_DEFAULT, "errorCountersPtr is NULL\n");
                    }
                }
                else
                {
                    dprintf(DPRINTF_DEFAULT, "            %1d        N/A        N/A        N/A        N/A        N/A        N/A\n",
                            habCounter);
                }
            }

            if (linePrinted == FALSE)
            {
                dprintf(DPRINTF_DEFAULT, "         None        N/A        N/A        N/A        N/A        N/A        N/A\n");
            }
        }

        dprintf(DPRINTF_DEFAULT, "\n");
    }
    else
    {
        dprintf(DPRINTF_FCM, "habDataListPtr is NULL\n");
        returnCode = ERROR;
    }

    return (returnCode);
}


/*----------------------------------------------------------------------------
**  Function Name: FCM_CountersDumpBayDataList
**
**  Parameters: bayDataListPtr - Pointer to bayDataList to be dumped
**
**  Returns:    GOOD  - bayDataList dumped
**              ERROR - bayDataList not dumped (bad input pointer)
**--------------------------------------------------------------------------*/
static UINT32 FCM_CountersDumpBayDataList(FCM_BAY_DATA *bayDataListPtr)
{
    UINT32      returnCode = GOOD;
    UINT32      portCounter = 0;
    UINT32      bayCounter = 0;
    UINT32      slotCounter = 0;
    FCM_SLOT_DATA *slotDataPtr = NULL;
    FCM_FCAL_ERROR_COUNTERS *errorCountersPtr = NULL;
    UINT8       portLinePrinted = 0;
    UINT8       bayLinePrinted = 0;
    UINT8       activePort = FALSE;
    char        linkFailString[COUNTER_STRING_LENGTH] = { 0 };
    char        lostSyncString[COUNTER_STRING_LENGTH] = { 0 };
    char        invXmitString[COUNTER_STRING_LENGTH] = { 0 };
    char        invCrcString[COUNTER_STRING_LENGTH] = { 0 };
    char        lipf7InitString[COUNTER_STRING_LENGTH] = { 0 };
    char        lipf7RecvString[COUNTER_STRING_LENGTH] = { 0 };
    char        lipf8InitString[COUNTER_STRING_LENGTH] = { 0 };
    char        lipf8RecvString[COUNTER_STRING_LENGTH] = { 0 };

    char        WordErrorCount[COUNTER_STRING_LENGTH] = { 0 };
    char        CRCErrorCount[COUNTER_STRING_LENGTH] = { 0 };
    char        ClockDelta[COUNTER_STRING_LENGTH] = { 0 };
    char        LoopUpCount[COUNTER_STRING_LENGTH] = { 0 };
    char        InsertionCount[COUNTER_STRING_LENGTH] = { 0 };
    char        StallCount[COUNTER_STRING_LENGTH] = { 0 };
    char        Utilization[COUNTER_STRING_LENGTH] = { 0 };

    char        powerOnMinString[COUNTER_STRING_LENGTH] = { 0 };

    ccb_assert(bayDataListPtr != NULL, bayDataListPtr);

    if (bayDataListPtr != NULL)
    {
        dprintf(DPRINTF_DEFAULT, "SES SLOT PORT LINK_FAIL  LOST_SYNC   INV_XMIT   INV_CRC   LIPF7_Init LIPF7_Recv LIPF8_Init LIPF8_Recv PowerOnMin\n");
        dprintf(DPRINTF_DEFAULT, "--- ---- ---- ---------- ---------- ---------- ---------- ---------- ---------- ---------- ---------- ----------\n");

        for (portCounter = 0; portCounter < MAX_BE_PORTS; portCounter++)
        {
            portLinePrinted = FALSE;

            for (bayCounter = 0; bayCounter < dimension_of(bayDataList1); bayCounter++)
            {
                bayLinePrinted = FALSE;

                if (bayDataListPtr[bayCounter].flags.bits.bayPresent == TRUE)
                {
                    if ((bayDataListPtr[bayCounter].flags.bits.portsUnknown == TRUE) ||
                        (((portCounter | 1) == 1) &&
                         (bayDataListPtr[bayCounter].flags.bits.ports0and1 == TRUE)) ||
                        (((portCounter | 1) == 3) &&
                         (bayDataListPtr[bayCounter].flags.bits.ports2and3 == TRUE)))
                    {
                        for (slotCounter = 0;
                             slotCounter < dimension_of(bayDataList1->slotDataList);
                             slotCounter++)
                        {
                            slotDataPtr = &bayDataListPtr[bayCounter].slotDataList[slotCounter];

                            if (slotDataPtr->flags.bits.drivePresent == TRUE)
                            {
                                /*
                                 * Check if the portCounter is looking at the same
                                 * port as the SCSI command was issued on.
                                 * Figure out the drive port-to-BE port mapping
                                 */
                                if ((UINT32)(slotDataPtr->channel & 1) == (portCounter & 1))
                                {
                                    activePort = TRUE;

                                    if (slotDataPtr->counters.commandInitiatePort == 0)
                                    {
                                        errorCountersPtr = &slotDataPtr->counters.portA;
                                    }
                                    else
                                    {
                                        errorCountersPtr = &slotDataPtr->counters.portB;
                                    }
                                }
                                else
                                {
                                    activePort = FALSE;

                                    if (slotDataPtr->counters.commandInitiatePort == 0)
                                    {
                                        errorCountersPtr = &slotDataPtr->counters.portB;
                                    }
                                    else
                                    {
                                        errorCountersPtr = &slotDataPtr->counters.portA;
                                    }
                                }

                                if ((UINT32)(slotDataPtr->channel | 1) == (portCounter | 1))
                                {
                                    FCM_CountersToString(linkFailString,
                                                         errorCountersPtr->linkFail,
                                                         sizeof(linkFailString));
                                    FCM_CountersToString(lostSyncString,
                                                         errorCountersPtr->lostSync,
                                                         sizeof(lostSyncString));
                                    FCM_CountersToString(invXmitString,
                                                         errorCountersPtr->invalidTransmit,
                                                         sizeof(invXmitString));
                                    FCM_CountersToString(invCrcString,
                                                         errorCountersPtr->invalidCRC,
                                                         sizeof(invCrcString));
                                    FCM_CountersToString(lipf7InitString,
                                                         errorCountersPtr->lipF7Initiated,
                                                         sizeof(lipf7InitString));
                                    FCM_CountersToString(lipf7RecvString,
                                                         errorCountersPtr->lipF7Received,
                                                         sizeof(lipf7RecvString));
                                    FCM_CountersToString(lipf8InitString,
                                                         errorCountersPtr->lipF8Initiated,
                                                         sizeof(lipf8InitString));
                                    FCM_CountersToString(lipf8RecvString,
                                                         errorCountersPtr->lipF8Received,
                                                         sizeof(lipf8RecvString));
                                    FCM_CountersToString(powerOnMinString,
                                                         slotDataPtr->counters.powerOnMinutes,
                                                         sizeof(powerOnMinString));

                                    if (bayDataListPtr[bayCounter].bayInfo.type == PD_DT_SBOD_SES)
                                    {
                                        FCM_CountersToString(WordErrorCount,
                                                             errorCountersPtr->WordErrorCount,
                                                             sizeof(WordErrorCount));
                                        FCM_CountersToString(CRCErrorCount,
                                                             errorCountersPtr->CRCErrorCount,
                                                             sizeof(CRCErrorCount));
                                        FCM_SignedCounterToString(ClockDelta,
                                                                  errorCountersPtr->ClockDelta,
                                                                  sizeof(ClockDelta));
                                        FCM_CountersToString(LoopUpCount,
                                                             errorCountersPtr->LoopUpCount,
                                                             sizeof(LoopUpCount));
                                        FCM_CountersToString(InsertionCount,
                                                             errorCountersPtr->InsertionCount,
                                                             sizeof(InsertionCount));
                                        FCM_CountersToString(StallCount,
                                                             errorCountersPtr->StallCount,
                                                             sizeof(StallCount));
                                        FCM_CountersToString(Utilization,
                                                             errorCountersPtr->Utilization,
                                                             sizeof(Utilization));
                                        dprintf(DPRINTF_DEFAULT, "%3d %4d %c%1d-%c %10s %10s %10s %10s %10s %10s %10s %10s %10s %10s %10s %10s %10s %10s %10s %10s\n",
                                                bayCounter, slotCounter, ((activePort == TRUE) ? '*' : ' '),
                                                portCounter, ((errorCountersPtr == &slotDataPtr->counters.portA) ? 'A' : 'B'),
                                                linkFailString, lostSyncString, invXmitString, invCrcString, lipf7InitString,
                                                lipf7RecvString, lipf8InitString, lipf8RecvString, powerOnMinString,
                                                WordErrorCount, CRCErrorCount, ClockDelta, LoopUpCount, InsertionCount,
                                                StallCount, Utilization);
                                    }
                                    else
                                    {
                                        dprintf(DPRINTF_DEFAULT, "%3d %4d %c%1d-%c %10s %10s %10s %10s %10s %10s %10s %10s %10s\n",
                                                bayCounter, slotCounter, ((activePort == TRUE) ? '*' : ' '),
                                                portCounter, ((errorCountersPtr == &slotDataPtr->counters.portA) ? 'A' : 'B'),
                                                linkFailString, lostSyncString, invXmitString, invCrcString, lipf7InitString,
                                                lipf7RecvString, lipf8InitString, lipf8RecvString, powerOnMinString);
                                    }

                                    bayLinePrinted = TRUE;
                                    portLinePrinted = TRUE;

                                    /*
                                     * Allow other tasks to run, since we might be
                                     * flooding the serial port here.
                                     */
                                    TaskSwitch();
                                }
                            }
                            else
                            {
                                dprintf(DPRINTF_DEFAULT, "%3d %4d None        N/A        N/A        N/A        N/A        N/A        N/A        N/A        N/A        N/A\n",
                                        bayCounter, slotCounter);
                            }
                        }
                    }
                }

                if (bayLinePrinted == TRUE)
                {
                    dprintf(DPRINTF_DEFAULT, "\n");

                    /*
                     * Allow other tasks to run, since we might be flooding the
                     * serial port or debugconsole here.
                     */
                    TaskSleepMS(500);
                }
            }

            if (portLinePrinted == TRUE)
            {
                dprintf(DPRINTF_DEFAULT, "\n");
            }
        }
    }
    else
    {
        dprintf(DPRINTF_FCM, "bayDataListPtr is NULL\n");
        returnCode = ERROR;
    }

    return (returnCode);
}


/*----------------------------------------------------------------------------
**  Function Name: FCM_SignedCounterToString
**
**  Comments:   Creates signed string for the specified eventType
**
**  Parameters: stringPtr
**              counterValue
**              stringLength
**
**  Returns:    Nothing
**--------------------------------------------------------------------------*/
static void FCM_SignedCounterToString(char *stringPtr, INT32 counterValue,
                                      UINT8 stringLength)
{
    char        tempString[40] = { 0 };

    sprintf(tempString, "%d", counterValue);
    strncpy(stringPtr, tempString, stringLength);

    /*
     * Make sure the strncpy is terminated
     */
    stringPtr[stringLength - 1] = '\0';
}


/*----------------------------------------------------------------------------
**  Function Name: FCM_CountersToString
**
**  Comments:   Creates string for the specified eventType
**
**  Parameters: stringPtr
**              counterValue
**              stringLength
**
**  Returns:    Nothing
**--------------------------------------------------------------------------*/
static void FCM_CountersToString(char *stringPtr, UINT32 counterValue, UINT8 stringLength)
{
    char        tempString[40] = { 0 };

    if (stringPtr != NULL)
    {
        if (counterValue == (UINT32)(-1))
        {
            strncpy(stringPtr, "N/A", stringLength);
        }
        else
        {
            sprintf(tempString, "%u", counterValue);
            strncpy(stringPtr, tempString, stringLength);
        }

        /*
         * Make sure the strncpy is terminated
         */
        if (stringLength > 0)
        {
            stringPtr[stringLength - 1] = '\0';
        }
    }
}


/*----------------------------------------------------------------------------
**  Function Name: FCM_CountersDeltaBackup
**
**  Parameters: counterMapPtr
**
**  Returns:    GOOD  - Delta data saved to backup
**              ERROR - Delta data not saved to backup
**--------------------------------------------------------------------------*/
static UINT32 FCM_CountersDeltaBackup(FCM_COUNTER_MAP *counterMapPtr)
{
    UINT32      returnCode = ERROR;
    FCM_ERROR_DATA *tempErrorDataPtr = NULL;

    ccb_assert(counterMapPtr != NULL, counterMapPtr);

    if (counterMapPtr != NULL)
    {
        /*
         * Acquire the mutex
         */
        (void)LockMutex(&counterMapPtr->header.busyMutex, MUTEX_WAIT);

        /*
         * Check to see if the delta data is valid
         */
        if (counterMapPtr->flags.bits.deltaMapValid == TRUE)
        {
            /*
             * Boundary check the backupMapIndex value
             */
            if (counterMapPtr->flags.bits.backupMapIndex > FCM_COUNTER_BACKUP_LISTS)
            {
                counterMapPtr->flags.bits.backupMapIndex = 0;
            }

            /*
             * Determine which backup slot the delta data goes into
             */
            switch (counterMapPtr->flags.bits.backupMapIndex)
            {
                case 3:
                    tempErrorDataPtr = &counterMapPtr->backupData3;
                    counterMapPtr->flags.bits.backup3MapValid = TRUE;
                    break;

                case 2:
                    tempErrorDataPtr = &counterMapPtr->backupData2;
                    counterMapPtr->flags.bits.backup2MapValid = TRUE;
                    break;

                case 1:
                    tempErrorDataPtr = &counterMapPtr->backupData1;
                    counterMapPtr->flags.bits.backup1MapValid = TRUE;
                    break;

                case 0:
                default:
                    tempErrorDataPtr = &counterMapPtr->backupData0;
                    counterMapPtr->flags.bits.backup0MapValid = TRUE;
                    break;
            }

            /*
             * Copy the data into the backup array and bump the index
             */
            if ((tempErrorDataPtr != NULL) &&
                (tempErrorDataPtr->habDataList != NULL) &&
                (tempErrorDataPtr->bayDataList != NULL))
            {
                dprintf(DPRINTF_DEFAULT, "DB: Saving delta data into backup index %d\n",
                        counterMapPtr->flags.bits.backupMapIndex);

                tempErrorDataPtr->beginTimestamp = counterMapPtr->deltaData.beginTimestamp;
                tempErrorDataPtr->endTimestamp = counterMapPtr->deltaData.endTimestamp;
                memcpy(tempErrorDataPtr->habDataList,
                       counterMapPtr->deltaData.habDataList, sizeof(backupHabDataList0));
                memcpy(tempErrorDataPtr->bayDataList,
                       counterMapPtr->deltaData.bayDataList, sizeof(backupBayDataList0));

                counterMapPtr->flags.bits.backupMapIndex++;

                /*
                 * We've backed up the deltaMap.  Invalidate the current
                 * deltaMap so that we don't back the same map up again.
                 */
                counterMapPtr->flags.bits.deltaMapValid = FALSE;
                returnCode = GOOD;
            }
        }
        else
        {
            dprintf(DPRINTF_FCM, "DB: Delta data not valid - skipping backup\n");
        }

        /*
         * Release the mutex
         */
        UnlockMutex(&counterMapPtr->header.busyMutex);
    }

    return (returnCode);
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
