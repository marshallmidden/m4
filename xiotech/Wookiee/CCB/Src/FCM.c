/* $Id: FCM.c 156020 2011-05-27 16:18:33Z m4 $ */
/*============================================================================
** FILE NAME:       FCM.c
** MODULE TITLE:    Fibre Channel Health Monitor
**
** Copyright (c) 2001-2009 XIOtech Corporation.  All rights reserved.
**==========================================================================*/
#include "FCM.h"

#include "FCM_Counters.h"

#include "AsyncEventHandler.h"
#include "debug_files.h"
#include "LOG_Defs.h"
#include "kernel.h"
#include "logdef.h"

#include "MR_Defs.h"
#include "PI_Utils.h"
#include "rtc.h"
#include "XIO_Std.h"
#include "XIO_Const.h"
#include "XIO_Macros.h"

/*****************************************************************************
** Private defines
*****************************************************************************/
#define NextEventIndex(x)     ( (x + 1) % MAXIMUM_NUMBER_OF_EVENTS )
#define PreviousEventIndex(x) ( (x + MAXIMUM_NUMBER_OF_EVENTS - 1) % MAXIMUM_NUMBER_OF_EVENTS )

#define MAXIMUM_NUMBER_OF_PORTS                 4
#define MAXIMUM_NUMBER_OF_EVENTS                30
#define SECONDS_PER_BURST                       60
#define TIME_TO_RESTORE_MILLISECONDS            600000  /* Ten minutes */

/* The EVENTS_PER_BURST numbers need to be less than NUMBER_OF_EVENTS */
#define MAXIMUM_DROPPED_FRAME_EVENTS_PER_BURST  8
#define MAXIMUM_LOOP_RESET_EVENTS_PER_BURST     8
#define MAXIMUM_RSCN_EVENTS_PER_BURST           8

#define MAXIMUM_DROPPED_FRAME_EVENTS_PER_HOUR   60
#define MAXIMUM_LOOP_RESET_EVENTS_PER_HOUR      60
#define MAXIMUM_RSCN_EVENTS_PER_HOUR            60

typedef struct FCM_HEALTH_FLAGS_STRUCT
{
    UINT32      degraded:1;
    UINT32      reserved:31;
} FCM_HEALTH_FLAGS,
           *FCM_HEALTH_FLAGS_PTR;

typedef struct FCM_EVENT_ENTRY_STRUCT
{
    UINT32      timeStamp;
} FCM_EVENT_ENTRY;

typedef struct FCM_EVENT_ARRAY_STRUCT
{
    UINT32      inIndex;
    UINT32      outIndex;
    UINT32      burstLimit;
    UINT32      rateLimit;
    FCM_EVENT_ENTRY event[MAXIMUM_NUMBER_OF_EVENTS];
} FCM_EVENT_ARRAY,
           *FCM_EVENT_ARRAY_PTR;

typedef struct FCM_EVENT_TYPE_ARRAYS_STRUCT
{
    FCM_EVENT_ARRAY droppedFrameArray;
    FCM_EVENT_ARRAY loopResetArray;
    FCM_EVENT_ARRAY rscnArray;
    FCM_HEALTH_FLAGS healthFlags;
} FCM_EVENT_TYPE_ARRAYS,
           *FCM_EVENT_TYPE_ARRAYS_PTR;

typedef struct FCM_STRUCT
{
    FCM_EVENT_TYPE_ARRAYS frontEndPort[MAXIMUM_NUMBER_OF_PORTS];
    FCM_EVENT_TYPE_ARRAYS backEndPort[MAXIMUM_NUMBER_OF_PORTS];
} FCM      ,
           *FCM_PTR;

/*****************************************************************************
** Private variables
*****************************************************************************/
static PCB *fcalHealthMonitorTaskPCBPtr = NULL;
static FCM  fcalMonitor;        /* Initialized at run time */

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static UINT32 FCM_InitializeMonitorStructure(FCM_PTR monitorPtr);
static UINT32 FCM_RemoveOldEvents(FCM_EVENT_ARRAY_PTR eventArrayPtr,
                                  UINT32 oldestTimestamp);
static UINT32 FCM_GetEventCount(FCM_EVENT_ARRAY_PTR eventArrayPtr, UINT32 eventAge);
static UINT32 FCM_GetEventRate(FCM_EVENT_ARRAY_PTR eventArrayPtr);
static UINT32 FCM_FilterDualPortEvents(FCM_EVENT_ARRAY_PTR lowPortArrayPtr,
                                       FCM_EVENT_ARRAY_PTR highPortArrayPtr);
static void FCM_RemoveEvent(FCM_EVENT_ARRAY_PTR portEventArrayPtr,
                            UINT32 removeEventIndex);
static void FCM_GenerateAlert(LOG_FCM_PKT *logEventPtr, INT32 eventType);
static UINT32 FCM_SetPortHealth(FCM_PROCESSOR whichEnd, UINT8 portNumber,
                                FCM_HEALTH_FLAGS_PTR healthFlagsPtr);
static void FCM_PortRestoreTask(TASK_PARMS *parms);
static UINT32 FCM_PortDegrade(FCM_PROCESSOR whichEnd, UINT8 portNumber,
                              FCM_EVENT_TYPE_ARRAYS_PTR eventTypeArraysPtr);

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
**  Function Name:  FCM_MonitorTask
**
**  Returns:        Nothing
**--------------------------------------------------------------------------*/
NORETURN void FCM_MonitorTask(UNUSED TASK_PARMS *parms)
{
    dprintf(DPRINTF_FCM, "MonitorTask\n");

    /*
     * Initialize the monitor structure before entering the main monitor loop
     */
    FCM_InitializeMonitorStructure(&fcalMonitor);

    InitMutex(&counterMap.header.busyMutex);
    /*
     * Save PCB of this task
     */
    fcalHealthMonitorTaskPCBPtr = XK_GetPcb();

    while (1)
    {
        /*
         * Go to sleep and wait for an FCM event
         */
        TaskSetState(fcalHealthMonitorTaskPCBPtr, PCB_NOT_READY);
        TaskSwitch();

        /*
         * This is where we'd request all of the drive counters and do
         * some processing to see if we can narrow down the suspect component.
         */
        dprintf(DPRINTF_FCM, "MonitorTask - Check FCM health (nonfunctional)\n");
    }
}


/*----------------------------------------------------------------------------
**  Function Name:  FCM_PortRestoreTask
**
**  Description:    This is designed to be forked and left alone.  It is
**                  to be called after a port is degraded.  After a certain
**                  amount of time, this task will attempt to restore the
**                  degraded port.
**
**  Inputs:         dummy
**                  whichEnd
**                  portNumber
**                  healthFlagsPtr
**                  waitToRestoreMS
**
**  Returns:        Nothing
**--------------------------------------------------------------------------*/
static void FCM_PortRestoreTask(TASK_PARMS *parms)
{
    FCM_PROCESSOR whichEnd = (FCM_PROCESSOR)parms->p1;
    UINT8       portNumber = (UINT8)parms->p2;
    FCM_EVENT_TYPE_ARRAYS_PTR eventTypeArraysPtr = (FCM_EVENT_TYPE_ARRAYS_PTR)parms->p3;
    UINT32      waitToRestoreMS = (UINT32)parms->p4;
    LOG_FCM_PKT fcalLogEvent = { 0, 0, 0, 0 };
    UINT32      currentTime;
    UINT32      returnCode;

    dprintf(DPRINTF_FCM, "PortRestoreTask\n");
    dprintf(DPRINTF_FCM, "  whichEnd:           %d\n", whichEnd);
    dprintf(DPRINTF_FCM, "  portNumber:         %d\n", portNumber);
    dprintf(DPRINTF_FCM, "  eventTypeArraysPtr: %p\n", eventTypeArraysPtr);
    dprintf(DPRINTF_FCM, "  waitToRestoreMS:    %d\n", waitToRestoreMS);

    if (eventTypeArraysPtr != NULL)
    {
        while (eventTypeArraysPtr->healthFlags.degraded == TRUE)
        {
            /*
             * Wait the specified amount of time before restoring the specified port
             */
            TaskSleepMS(waitToRestoreMS);

            dprintf(DPRINTF_FCM, "PortRestoreTask - Attempt to retore port %d\n",
                    portNumber);

            /*
             * Grab a new systemSeconds reading
             */
            currentTime = RTC_GetSystemSeconds();

            /*
             * Remove all events that are more than one hour old, if the CCB
             * has been powered on that long.
             */
            if (currentTime > (SECONDS_IN_ONE_MINUTE * MINUTES_IN_ONE_HOUR))
            {
                FCM_RemoveOldEvents(&eventTypeArraysPtr->droppedFrameArray,
                                    currentTime -
                                    (SECONDS_IN_ONE_MINUTE * MINUTES_IN_ONE_HOUR));

                FCM_RemoveOldEvents(&eventTypeArraysPtr->loopResetArray,
                                    currentTime -
                                    (SECONDS_IN_ONE_MINUTE * MINUTES_IN_ONE_HOUR));
            }

            /*
             * Check the limits to see if the port is healthy again, so that it
             * can be restored.
             */
            if ((FCM_GetEventCount(&eventTypeArraysPtr->droppedFrameArray,
                                   (currentTime > SECONDS_PER_BURST ? currentTime - SECONDS_PER_BURST : 0)) >=
                 eventTypeArraysPtr->droppedFrameArray.burstLimit) ||
                (FCM_GetEventCount (&eventTypeArraysPtr->loopResetArray,
                  (currentTime > SECONDS_PER_BURST ? currentTime - SECONDS_PER_BURST : 0)) >=
                 eventTypeArraysPtr->loopResetArray.burstLimit) ||
                (FCM_GetEventCount(&eventTypeArraysPtr->rscnArray,
                  (currentTime > SECONDS_PER_BURST ? currentTime - SECONDS_PER_BURST : 0)) >=
                 eventTypeArraysPtr->rscnArray.burstLimit) ||
                (FCM_GetEventRate(&eventTypeArraysPtr->droppedFrameArray) >=
                 eventTypeArraysPtr->droppedFrameArray.rateLimit) ||
                (FCM_GetEventRate(&eventTypeArraysPtr->loopResetArray) >=
                 eventTypeArraysPtr->loopResetArray.rateLimit) ||
                (FCM_GetEventRate(&eventTypeArraysPtr->rscnArray) >=
                 eventTypeArraysPtr->rscnArray.rateLimit))
            {
                /*
                 * Degrade the port, again.  Retry a restore later.
                 */
                eventTypeArraysPtr->healthFlags.degraded = TRUE;
                FCM_SetPortHealth(whichEnd, portNumber, &eventTypeArraysPtr->healthFlags);
                dprintf(DPRINTF_FCM, "PortRestoreTask - Port %d still unhealthy - degrade again & go back to sleep\n",
                        portNumber);
            }
            else
            {
                dprintf(DPRINTF_FCM, "PortRestoreTask - Port %d healthy - restore the port\n",
                        portNumber);

                /*
                 * Create 'restored' log message
                 */
                fcalLogEvent.proc = (whichEnd == FCM_PROCESSOR_FE ? 0 : 1);
                fcalLogEvent.port = portNumber;
                fcalLogEvent.eventType = FCM_ET_RESTORED;
                fcalLogEvent.conditionCode = FCM_CONDITION_CODE_GOOD;
                FCM_GenerateAlert(&fcalLogEvent, LOG_Info(LOG_FCM));

                /*
                 * Restore the port.  If there's any error in restoring the port,
                 * then set it back to degraded and retry again later.
                 * NOTE:  Both PI_GOOD and PI_TIMEOUT are considered success,
                 *        since the proc typically eventually executes the MRP
                 *        in the PI_TIMEOUT case.
                 */
                eventTypeArraysPtr->healthFlags.degraded = FALSE;

                returnCode = FCM_SetPortHealth(whichEnd, portNumber, &eventTypeArraysPtr->healthFlags);
                if ((returnCode != PI_GOOD) && (returnCode != PI_TIMEOUT))
                {
                    eventTypeArraysPtr->healthFlags.degraded = TRUE;
                }
            }
        }                       /* while degraded */
    }
}


/*----------------------------------------------------------------------------
**  Function Name:  FCM_InitializeMonitorStructure
**
**  Inputs:         monitorPtr
**
**  Returns:        GOOD
**                  ERROR
**--------------------------------------------------------------------------*/
static UINT32 FCM_InitializeMonitorStructure(FCM_PTR monitorPtr)
{
    UINT32      returnCode = ERROR;
    UINT32      portCounter = 0;
    UINT32      eventCounter = 0;

    dprintf(DPRINTF_FCM, "InitializeMonitorStructure\n");

    if (monitorPtr != NULL)
    {
        /*
         * Initialize the event arrays for each port
         */
        for (portCounter = 0; portCounter < MAXIMUM_NUMBER_OF_PORTS; portCounter++)
        {
            /* Loop health flags */
            monitorPtr->frontEndPort[portCounter].healthFlags.degraded = FALSE;
            monitorPtr->frontEndPort[portCounter].healthFlags.reserved = 0;

            monitorPtr->backEndPort[portCounter].healthFlags.degraded = FALSE;
            monitorPtr->backEndPort[portCounter].healthFlags.reserved = 0;

            /* Head and Tail indexes flags */
            /* Front end */
            monitorPtr->frontEndPort[portCounter].droppedFrameArray.inIndex = 0;
            monitorPtr->frontEndPort[portCounter].droppedFrameArray.outIndex = 0;
            monitorPtr->frontEndPort[portCounter].droppedFrameArray.rateLimit = MAXIMUM_DROPPED_FRAME_EVENTS_PER_HOUR;
            monitorPtr->frontEndPort[portCounter].droppedFrameArray.burstLimit = MAXIMUM_DROPPED_FRAME_EVENTS_PER_BURST;

            monitorPtr->frontEndPort[portCounter].loopResetArray.inIndex = 0;
            monitorPtr->frontEndPort[portCounter].loopResetArray.outIndex = 0;
            monitorPtr->frontEndPort[portCounter].loopResetArray.rateLimit = MAXIMUM_LOOP_RESET_EVENTS_PER_HOUR;
            monitorPtr->frontEndPort[portCounter].loopResetArray.burstLimit = MAXIMUM_LOOP_RESET_EVENTS_PER_BURST;

            monitorPtr->frontEndPort[portCounter].rscnArray.inIndex = 0;
            monitorPtr->frontEndPort[portCounter].rscnArray.outIndex = 0;
            monitorPtr->frontEndPort[portCounter].rscnArray.rateLimit = MAXIMUM_RSCN_EVENTS_PER_HOUR;
            monitorPtr->frontEndPort[portCounter].rscnArray.burstLimit = MAXIMUM_RSCN_EVENTS_PER_BURST;

            /* Back end */
            monitorPtr->backEndPort[portCounter].droppedFrameArray.inIndex = 0;
            monitorPtr->backEndPort[portCounter].droppedFrameArray.outIndex = 0;
            monitorPtr->backEndPort[portCounter].droppedFrameArray.rateLimit = MAXIMUM_DROPPED_FRAME_EVENTS_PER_HOUR;
            monitorPtr->backEndPort[portCounter].droppedFrameArray.burstLimit = MAXIMUM_DROPPED_FRAME_EVENTS_PER_BURST;

            monitorPtr->backEndPort[portCounter].loopResetArray.inIndex = 0;
            monitorPtr->backEndPort[portCounter].loopResetArray.outIndex = 0;
            monitorPtr->backEndPort[portCounter].loopResetArray.rateLimit = MAXIMUM_LOOP_RESET_EVENTS_PER_HOUR;
            monitorPtr->backEndPort[portCounter].loopResetArray.burstLimit = MAXIMUM_LOOP_RESET_EVENTS_PER_BURST;

            monitorPtr->backEndPort[portCounter].rscnArray.inIndex = 0;
            monitorPtr->backEndPort[portCounter].rscnArray.outIndex = 0;
            monitorPtr->backEndPort[portCounter].rscnArray.rateLimit = MAXIMUM_RSCN_EVENTS_PER_HOUR;
            monitorPtr->backEndPort[portCounter].rscnArray.burstLimit = MAXIMUM_RSCN_EVENTS_PER_BURST;

            /* Event timestamps */
            for (eventCounter = 0; eventCounter < MAXIMUM_NUMBER_OF_EVENTS;
                 eventCounter++)
            {
                monitorPtr->frontEndPort[portCounter].droppedFrameArray.event[eventCounter].timeStamp = 0;
                monitorPtr->frontEndPort[portCounter].loopResetArray.event[eventCounter].timeStamp = 0;
                monitorPtr->frontEndPort[portCounter].rscnArray.event[eventCounter].timeStamp = 0;

                monitorPtr->backEndPort[portCounter].droppedFrameArray.event[eventCounter].timeStamp = 0;
                monitorPtr->backEndPort[portCounter].loopResetArray.event[eventCounter].timeStamp = 0;
                monitorPtr->backEndPort[portCounter].rscnArray.event[eventCounter].timeStamp = 0;
            }
        }

        returnCode = GOOD;
    }

    return (returnCode);
}


/*----------------------------------------------------------------------------
**  Function Name:  FCM_AddEvent
**
**  Inputs:         whichEnd
**                  portNumber
**                  eventType
**
**  Returns:        None
**--------------------------------------------------------------------------*/
void FCM_AddEvent(FCM_PROCESSOR whichEnd, UINT8 portNumber, FCM_EVENT_TYPE eventType)
{
    FCM_EVENT_TYPE_ARRAYS_PTR eventTypeArraysPtr = NULL;
    FCM_EVENT_ARRAY_PTR eventArrayPtr = NULL;
    FCM_EVENT_ARRAY_PTR pairEventArrayPtr = NULL;
    UINT32      currentTime = RTC_GetSystemSeconds();
    UINT32      filterCount = 0;
    LOG_FCM_PKT fcalLogEvent = { 0, 0, 0, 0 };

    UINT32      eventCount = 0;

    dprintf(DPRINTF_FCM, "AddEvent\n");

    if (fcalHealthMonitorTaskPCBPtr != NULL)
    {
        switch (whichEnd)
        {
            case FCM_PROCESSOR_FE:
                dprintf(DPRINTF_FCM, "  FCM_PROCESSOR_FE\n");

                eventTypeArraysPtr = fcalMonitor.frontEndPort;
                break;

            case FCM_PROCESSOR_BE:
                dprintf(DPRINTF_FCM, "  FCM_PROCESSOR_BE\n");

                eventTypeArraysPtr = fcalMonitor.backEndPort;
                break;

            case FCM_PROCESSOR_UNDEFINED:
            default:
                dprintf(DPRINTF_FCM, "  FCM_PROCESSOR_UNDEFINED\n");
                break;
        }

        if (eventTypeArraysPtr != NULL)
        {
            if (portNumber < MAXIMUM_NUMBER_OF_PORTS)
            {
                /*
                 * Set eventArrayPtr to the array for the specific type of event
                 */
                switch (eventType)
                {
                    case FCM_ET_DROPPED_FRAME:
                        dprintf(DPRINTF_FCM, "  FCM_ET_DROPPED_FRAME\n");

                        eventArrayPtr = &eventTypeArraysPtr[portNumber].droppedFrameArray;
                        pairEventArrayPtr = &eventTypeArraysPtr[portNumber ^ 1].droppedFrameArray;
                        break;

                    case FCM_ET_LOOP_RESET:
                        dprintf(DPRINTF_FCM, "  FCM_ET_LOOP_RESET\n");

                        eventArrayPtr = &eventTypeArraysPtr[portNumber].loopResetArray;
                        pairEventArrayPtr = &eventTypeArraysPtr[portNumber ^ 1].loopResetArray;
                        break;

                    case FCM_ET_RSCN:
                        dprintf(DPRINTF_FCM, "  FCM_ET_RSCN\n");

                        eventArrayPtr = &eventTypeArraysPtr[portNumber].rscnArray;
                        pairEventArrayPtr = &eventTypeArraysPtr[portNumber ^ 1].rscnArray;
                        break;

                    case FCM_ET_UNDEFINED:
                    case FCM_ET_RESTORED:
                    default:
                        dprintf(DPRINTF_FCM, "  FCM_ET_UNDEFINED\n");
                        break;
                }

                dprintf(DPRINTF_FCM, "  PORT: %d\n", portNumber);

                /*
                 * Move eventTypeArraysPtr to point to the specific port that
                 * created the event.
                 */
                eventTypeArraysPtr = &eventTypeArraysPtr[portNumber];
            }
        }

        if ((eventArrayPtr != NULL) && (pairEventArrayPtr != NULL))
        {
            /*
             * Remove all events that are more than one hour old
             */
            if (currentTime > (SECONDS_IN_ONE_MINUTE * MINUTES_IN_ONE_HOUR))
            {
                FCM_RemoveOldEvents(eventArrayPtr,
                                    currentTime - (SECONDS_IN_ONE_MINUTE * MINUTES_IN_ONE_HOUR));
            }

            if (NextEventIndex(eventArrayPtr->inIndex) == eventArrayPtr->outIndex)
            {
                /*
                 * Array is full, so throw away the oldest entry.  Set the entry
                 * to initialized value for easier debugability.
                 */
                dprintf(DPRINTF_FCM, "  Removing entry with time: %d\n",
                        eventArrayPtr->event[eventArrayPtr->outIndex].timeStamp);

                eventArrayPtr->event[eventArrayPtr->outIndex].timeStamp = 0;
                eventArrayPtr->outIndex = NextEventIndex(eventArrayPtr->outIndex);
            }

            /*
             * Add the new event entry
             */
            eventArrayPtr->event[eventArrayPtr->inIndex].timeStamp = currentTime;

            dprintf(DPRINTF_FCM, "  Entry added at time:      %d\n",
                    eventArrayPtr->event[eventArrayPtr->inIndex].timeStamp);

            /*
             * Move the inIndex value to the next entry
             */
            eventArrayPtr->inIndex = NextEventIndex(eventArrayPtr->inIndex);

            dprintf(DPRINTF_FCM, "  inIndex: %2u  outIndex: %2u\n",
                    eventArrayPtr->inIndex, eventArrayPtr->outIndex);

            for (eventCount = 0; eventCount < MAXIMUM_NUMBER_OF_EVENTS; eventCount++)
            {
                dprintf(DPRINTF_FCM, "  Array[%02u]: %u\n",
                        eventCount, eventArrayPtr->event[eventCount].timeStamp);
            }

            /*
             * Remove any events that occurred on the 0-1, or 2-3 pairs at the
             * same time.  We're only interested in events that are happening
             * to only one of the two ports.
             */
            filterCount = FCM_FilterDualPortEvents(eventArrayPtr, pairEventArrayPtr);

            dprintf(DPRINTF_FCM, "  %d items filtered\n", filterCount);
            dprintf(DPRINTF_FCM, "  %d items in the array\n",
                    FCM_GetEventCount(eventArrayPtr, 0));
            dprintf(DPRINTF_FCM, "  %d burst items in the array\n",
                    FCM_GetEventCount(eventArrayPtr,
                      (currentTime > SECONDS_PER_BURST ? currentTime - SECONDS_PER_BURST : 0)));

            /*
             * Check for event burst alert
             */
            if (FCM_GetEventCount(eventArrayPtr,
                  (currentTime > SECONDS_PER_BURST ? currentTime - SECONDS_PER_BURST : 0)) >=
                    eventArrayPtr->burstLimit)
            {
                dprintf(DPRINTF_FCM, "** Burst limit exceeded **\n");

                if (FCM_PortDegrade(whichEnd, portNumber, eventTypeArraysPtr) == GOOD)
                {
                    /*
                     * Create log event to inform the user
                     */
                    fcalLogEvent.port = portNumber;
                    fcalLogEvent.eventType = eventType;
                    fcalLogEvent.conditionCode = FCM_CONDITION_CODE_BURST;

                    if (whichEnd == FCM_PROCESSOR_FE)
                    {
                        fcalLogEvent.proc = 0;
                        FCM_GenerateAlert(&fcalLogEvent, LOG_Warning(LOG_FCM));
                    }
                    else
                    {
                        fcalLogEvent.proc = 1;
                        FCM_GenerateAlert(&fcalLogEvent, LOG_Error(LOG_FCM));
                    }
                }
            }

            /*
             * Check for 'too high of rate' alert
             */
            if (FCM_GetEventRate(eventArrayPtr) >= eventArrayPtr->rateLimit)
            {
                dprintf(DPRINTF_FCM, "** Rate limit exceeded **\n");

                if (FCM_PortDegrade(whichEnd, portNumber, eventTypeArraysPtr) == GOOD)
                {
                    /*
                     * Create log event to inform the user
                     */
                    fcalLogEvent.port = portNumber;
                    fcalLogEvent.eventType = eventType;
                    fcalLogEvent.conditionCode = FCM_CONDITION_CODE_RATE;

                    if (whichEnd == FCM_PROCESSOR_FE)
                    {
                        fcalLogEvent.proc = 0;
                        FCM_GenerateAlert(&fcalLogEvent, LOG_Warning(LOG_FCM));
                    }
                    else
                    {
                        fcalLogEvent.proc = 1;
                        FCM_GenerateAlert(&fcalLogEvent, LOG_Error(LOG_FCM));
                    }
                }
            }
        }
    }
    else
    {
        dprintf(DPRINTF_FCM, "FCM Monitor not yet running - event ignored\n");
    }
}


/*----------------------------------------------------------------------------
**  Function Name:  FCM_RemoveOldEvents
**
**  Inputs:         eventArrayPtr
**                  oldestTimestamp
**
**  Returns:        GOOD
**                  ERROR
**--------------------------------------------------------------------------*/
static UINT32 FCM_RemoveOldEvents(FCM_EVENT_ARRAY_PTR eventArrayPtr,
                                  UINT32 oldestTimestamp)
{
    UINT32      returnCode = ERROR;

    dprintf(DPRINTF_FCM, "RemoveOldEvents\n");

    if (eventArrayPtr != NULL)
    {
        while ((eventArrayPtr->outIndex != eventArrayPtr->inIndex) &&
               (eventArrayPtr->event[eventArrayPtr->outIndex].timeStamp < oldestTimestamp))
        {
            dprintf(DPRINTF_FCM, "  Removing entry with time: %d\n",
                    eventArrayPtr->event[eventArrayPtr->outIndex].timeStamp);

            eventArrayPtr->event[eventArrayPtr->outIndex].timeStamp = 0;
            eventArrayPtr->outIndex = NextEventIndex(eventArrayPtr->outIndex);
        }

        returnCode = GOOD;
    }

    return (returnCode);
}


/*----------------------------------------------------------------------------
**  Function Name:  FCM_GetEventCount
**
**  Inputs:         eventArrayPtr
**                  eventAge
**
**  Returns:        Number of events in the event array
**--------------------------------------------------------------------------*/
static UINT32 FCM_GetEventCount(FCM_EVENT_ARRAY_PTR eventArrayPtr, UINT32 eventAge)
{
    UINT32      eventCount = 0;
    UINT32      oldestEventWithinAgeIndex = 0;
    UINT32      tempIndex = 0;
    UINT32      tempCount = 0;

    dprintf(DPRINTF_FCM, "GetEventCount\n");

    if (eventArrayPtr != NULL)
    {
        oldestEventWithinAgeIndex = eventArrayPtr->outIndex;

        /*
         * Walk through the array to find the first entry within the age limit
         */
        while ((oldestEventWithinAgeIndex != eventArrayPtr->inIndex) &&
               (eventArrayPtr->event[oldestEventWithinAgeIndex].timeStamp < eventAge))
        {
            oldestEventWithinAgeIndex = NextEventIndex(oldestEventWithinAgeIndex);
            tempCount++;
        }

        tempIndex = oldestEventWithinAgeIndex;
        while (tempIndex != eventArrayPtr->inIndex)
        {
            tempIndex = NextEventIndex(tempIndex);
            tempCount++;
        }

        /*
         * Count the number of events between the 'oldest' event and the inIndex
         */
        if (eventArrayPtr->inIndex < oldestEventWithinAgeIndex)
        {
            eventCount = eventArrayPtr->inIndex + MAXIMUM_NUMBER_OF_EVENTS - oldestEventWithinAgeIndex;
        }
        else
        {
            eventCount = eventArrayPtr->inIndex - oldestEventWithinAgeIndex;
        }

        dprintf(DPRINTF_FCM, "  Found %d events younger than %d\n", eventCount, eventAge);
    }

    return (eventCount);
}


/*----------------------------------------------------------------------------
**  Function Name:  FCM_GetEventRate
**
**  Inputs:         eventArrayPtr
**                  minumumEventCount
**
**  Returns:        Number of events in the event array
**--------------------------------------------------------------------------*/
static UINT32 FCM_GetEventRate(FCM_EVENT_ARRAY_PTR eventArrayPtr)
{
    UINT32      oldestEventIndex = 0;
    UINT32      youngestEventIndex = 0;
    UINT32      rate = 0;
    UINT32      currentTime = RTC_GetSystemSeconds();
    UINT32      eventCount = FCM_GetEventCount(eventArrayPtr, 0);

    dprintf(DPRINTF_FCM, "GetEventRate\n");

    if (eventArrayPtr != NULL)
    {
        if (eventCount >= eventArrayPtr->burstLimit)
        {
            /*
             * Walk through the array to find the first entry within the age limit
             */
            oldestEventIndex = eventArrayPtr->outIndex;

            /*
             * The youngest index is the entry just before the inIndex
             */
            youngestEventIndex = PreviousEventIndex(eventArrayPtr->inIndex);

            dprintf(DPRINTF_FCM, "  COUNT:    %u total events\n", eventCount);
            dprintf(DPRINTF_FCM, "  OLDEST:   %u within time %u\n",
                    oldestEventIndex, eventArrayPtr->event[oldestEventIndex].timeStamp);
            dprintf(DPRINTF_FCM, "  YOUNGEST: %u within time %u\n",
                    youngestEventIndex, eventArrayPtr->event[youngestEventIndex].timeStamp);
            dprintf(DPRINTF_FCM, "  CURRENT:  time %u\n", currentTime);

            /*
             * Calculate rate in 'events per hour'
             *   3600 seconds   1 timeslice    ? events     ? events
             *   ------------ * ----------- * ----------- = --------
             *      1 hour       ? seconds    1 timeslice    1 hour
             */

            /* rate = seconds per hour, scaled by 1000 (integer math) */
            rate = 3600 * 1000;

            /* rate = timeslices per hour, scaled by 1000 */
            if (eventArrayPtr->event[oldestEventIndex].timeStamp < currentTime)
            {
                rate /= currentTime - eventArrayPtr->event[oldestEventIndex].timeStamp;
            }

            dprintf(DPRINTF_FCM, "  Timeslices per hour: %u (scaled by 1000)\n", rate);

            /* rate = events per hour, scaled by 1000 */
            rate *= eventCount;

            /* rate = events per hour */
            rate /= 1000;

            dprintf(DPRINTF_FCM, "  Rate:     %u\n", rate);
        }
    }

    return (rate);
}


/*----------------------------------------------------------------------------
**  Function Name: FCM_FilterDualPortEvents
**
**  Comments:   Removes events from one port that occur within +/- one
**              second of an event on the opposite port.  This should
**              screen out the drive insertion and removal events, leaving
**              only the events that are randomly generated while running.
**
**  Parameters: lowPortEventArrayPtr
**              highPortEventArrayPtr
**
**  Returns:    Number of events filtered (removed from event arrays)
**--------------------------------------------------------------------------*/
static UINT32 FCM_FilterDualPortEvents(FCM_EVENT_ARRAY_PTR lowPortEventArrayPtr,
                                       FCM_EVENT_ARRAY_PTR highPortEventArrayPtr)
{
    UINT32      currentLowPortEventIndex = 0;
    UINT32      currentHighPortEventIndex = 0;
    UINT32      removeCount = 0;

    dprintf(DPRINTF_FCM, "FilterDualPortEvents\n");

    if ((lowPortEventArrayPtr != NULL) && (highPortEventArrayPtr != NULL))
    {
        /*
         * Set up the starting indexes.  These are the 'oldest', or lowest
         * numbers in the event arrays.
         */
        currentLowPortEventIndex = lowPortEventArrayPtr->outIndex;
        currentHighPortEventIndex = highPortEventArrayPtr->outIndex;

        dprintf(DPRINTF_FCM, "  LowArray:  in[%02u] out[%02u]\n",
                lowPortEventArrayPtr->inIndex, lowPortEventArrayPtr->outIndex);

        dprintf(DPRINTF_FCM, "  HighArray: in[%02u] out[%02u]\n",
                highPortEventArrayPtr->inIndex, highPortEventArrayPtr->outIndex);

        /*
         * Scan the event arrays for entries that are within one second
         * of each other and remove them if found.
         */
        while ((currentLowPortEventIndex != lowPortEventArrayPtr->inIndex) &&
               (currentHighPortEventIndex != highPortEventArrayPtr->inIndex))
        {
            dprintf(DPRINTF_FCM, "  Low[%02u]: %-10u High:[%2u]: %-10u\n",
                    currentLowPortEventIndex,
                    lowPortEventArrayPtr->event[currentLowPortEventIndex].timeStamp,
                    currentHighPortEventIndex,
                    highPortEventArrayPtr->event[currentHighPortEventIndex].timeStamp);

            if (lowPortEventArrayPtr->event[currentLowPortEventIndex].timeStamp <
                highPortEventArrayPtr->event[currentHighPortEventIndex].timeStamp)
            {
                /* lowPort less than highPort */
                if (highPortEventArrayPtr->event[currentHighPortEventIndex].timeStamp -
                    lowPortEventArrayPtr->event[currentLowPortEventIndex].timeStamp <= 1)
                {
                    dprintf(DPRINTF_FCM, "FDPE: (l<h) Remove low[%d] and high[%d]\n",
                            currentLowPortEventIndex, currentHighPortEventIndex);

                    FCM_RemoveEvent(lowPortEventArrayPtr, currentLowPortEventIndex);
                    FCM_RemoveEvent(highPortEventArrayPtr, currentHighPortEventIndex);
                    removeCount++;
                }
                else
                {
                    /* Move lowPort to the next higher entry */
                    currentLowPortEventIndex = NextEventIndex(currentLowPortEventIndex);
                }
            }
            else if (lowPortEventArrayPtr->event[currentLowPortEventIndex].timeStamp ==
                     highPortEventArrayPtr->event[currentHighPortEventIndex].timeStamp)
            {
                dprintf(DPRINTF_FCM, "FDPE: (l=h) Remove low[%d] and high[%d]\n",
                        currentLowPortEventIndex, currentHighPortEventIndex);

                /* lowPort equal to highPort */
                FCM_RemoveEvent(lowPortEventArrayPtr, currentLowPortEventIndex);
                FCM_RemoveEvent(highPortEventArrayPtr, currentHighPortEventIndex);
                removeCount++;
            }
            else
            {
                /* lowPort higher than highPort */
                if (lowPortEventArrayPtr->event[currentLowPortEventIndex].timeStamp -
                    highPortEventArrayPtr->event[currentHighPortEventIndex].timeStamp <= 1)
                {
                    dprintf(DPRINTF_FCM, "FDPE: (l>h) Remove low[%d] and high[%d]\n",
                            currentLowPortEventIndex, currentHighPortEventIndex);

                    FCM_RemoveEvent(lowPortEventArrayPtr, currentLowPortEventIndex);
                    FCM_RemoveEvent(highPortEventArrayPtr, currentHighPortEventIndex);
                    removeCount++;
                }
                else
                {
                    /* Move highPort to the next higher entry */
                    currentHighPortEventIndex = NextEventIndex(currentHighPortEventIndex);
                }
            }
        }
    }
    else
    {
        dprintf(DPRINTF_FCM, "FDPE: At least one of the PortEventArrayPtr's is NULL\n");
    }

    return (removeCount);
}


/*----------------------------------------------------------------------------
**  Function Name: FCM_RemoveEvent
**
**  Comments:   Removes events from one port that occur within +/- one
**              second of an event on the opposite port.  This should
**              screen out the drive insertion and removal events, leaving
**              only the events that are randomly generated while running.
**
**  Parameters: portEventArrayPtr
**              whichEvent
**
**  Returns:    Nothing
**--------------------------------------------------------------------------*/
static void FCM_RemoveEvent(FCM_EVENT_ARRAY_PTR portEventArrayPtr,
                            UINT32 removeEventIndex)
{
    dprintf(DPRINTF_FCM, "RemoveEvent\n");

    if (portEventArrayPtr != NULL)
    {
        if ((removeEventIndex < MAXIMUM_NUMBER_OF_EVENTS) &&
            (removeEventIndex != portEventArrayPtr->inIndex))
        {
            dprintf(DPRINTF_FCM, "RE: Removing index %d\n", removeEventIndex);

            while (removeEventIndex != portEventArrayPtr->inIndex)
            {
                portEventArrayPtr->event[removeEventIndex] = portEventArrayPtr->event[NextEventIndex(removeEventIndex)];

                removeEventIndex = NextEventIndex(removeEventIndex);
            }

            portEventArrayPtr->inIndex = PreviousEventIndex(portEventArrayPtr->inIndex);
        }
        else
        {
            dprintf(DPRINTF_FCM, "RE: Invalid removeEventIndex (%d)\n", removeEventIndex);
        }
    }
    else
    {
        dprintf(DPRINTF_FCM, "RE: portEventArrayPtr is NULL\n");
    }
}


/*----------------------------------------------------------------------------
**  Function Name: FCM_GenerateAlert
**
**  Comments:   Generates and sends an alert message.
**
**  Parameters: logEventPtr
**              eventType - Must be one of the MLE event codes
**
**  Returns:    Nothing
**--------------------------------------------------------------------------*/
static void FCM_GenerateAlert(LOG_FCM_PKT *logEventPtr, INT32 eventType)
{
    dprintf(DPRINTF_FCM, "GenerateAlert\n");

    LOG_SetCode(eventType, LOG_FCM);

    if (logEventPtr != NULL)
    {
        dprintf(DPRINTF_FCM, "GA: %s-port %d (E:%d, C:%d)\n",
                logEventPtr->proc ? "STORAGE" : "HOST",
                logEventPtr->port, logEventPtr->eventType, logEventPtr->conditionCode);

        SendAsyncEvent(eventType, sizeof(*logEventPtr), logEventPtr);
        TaskSwitch();
    }
    else
    {
        dprintf(DPRINTF_FCM, "GA: logEventPtr is NULL\n");
    }
}


/*----------------------------------------------------------------------------
**  Function Name: FCM_SetPortHealth
**
**  Comments:   Issues an MRP to degrade or restore a port as requested.
**
**  Parameters: whichEnd
**              portNumber
**              healthFlagsPtr
**
**  Returns:    PI_GOOD
**              PI_ERROR, PI_TIMEOUT, or other unsuccessful MRP status
**--------------------------------------------------------------------------*/
static UINT32 FCM_SetPortHealth(FCM_PROCESSOR whichEnd, UINT8 portNumber,
                                FCM_HEALTH_FLAGS_PTR healthFlagsPtr)
{
    UINT32      returnCode = PI_ERROR;
    MRDEGRADEPORT_REQ *ptrInPkt = NULL;
    MRDEGRADEPORT_RSP *ptrOutPkt = NULL;

    dprintf(DPRINTF_FCM, "SetPortHealth\n");

    if (healthFlagsPtr != NULL)
    {
        if (whichEnd == FCM_PROCESSOR_BE)
        {
            /*
             * Allocate memory for the MRP input and output packets.
             */
            ptrInPkt = MallocWC(sizeof(*ptrInPkt));
            ptrOutPkt = MallocSharedWC(sizeof(*ptrOutPkt));

            /*
             * Load the port health information and call the MRP
             */
            ptrInPkt->port = portNumber;

            if (healthFlagsPtr->degraded == TRUE)
            {
                dprintf(DPRINTF_FCM, "  SPH: Degrade\n");
                ptrInPkt->action = DRP_ACTION_DEGRADE;
            }
            else
            {
                dprintf(DPRINTF_FCM, "  SPH: Restore\n");
                ptrInPkt->action = DRP_ACTION_RESTORE;
            }

            /*
             * Send the request to the proc.  This function handles timeout
             * conditions and task switches while waiting.
             */
            returnCode = PI_ExecMRP(ptrInPkt, sizeof(*ptrInPkt), MRDEGRADEPORT,
                                    ptrOutPkt, sizeof(*ptrOutPkt), MRP_STD_TIMEOUT);

            /*
             * Free the allocated memory
             */
            Free(ptrInPkt);

            if (returnCode != PI_TIMEOUT)
            {
                Free(ptrOutPkt);
            }

            if (returnCode == PI_GOOD)
            {
                dprintf(DPRINTF_FCM, "  SPH: MRP returned with GOOD status\n");
            }
            else
            {
                dprintf(DPRINTF_FCM, "  SPH: Error returned from MRP\n");
            }
        }
        else
        {
            dprintf(DPRINTF_FCM, "  SPH: FE health not supported\n");
            returnCode = PI_GOOD;
        }
    }
    else
    {
        dprintf(DPRINTF_FCM, "  SPH: healthFlagsPtr is NULL\n");
    }

    return (returnCode);
}


/*----------------------------------------------------------------------------
**  Function Name: FCM_PortDegrade
**
**  Parameters: whichEnd
**              portNumber
**              healthFlagsPtr
**
**  Returns:    PI_GOOD  - port is now degraded
**              PI_ERROR - bad input parameters, or port already degraded
**--------------------------------------------------------------------------*/
static UINT32 FCM_PortDegrade(FCM_PROCESSOR whichEnd, UINT8 portNumber,
                              FCM_EVENT_TYPE_ARRAYS_PTR eventTypeArraysPtr)
{
    UINT32      returnCode = PI_ERROR;
    FCM_HEALTH_FLAGS restoreHealthFlags;
    TASK_PARMS  parms;

    if (eventTypeArraysPtr != NULL)
    {
        /*
         * Degrade the port, if it's not already
         */
        if (eventTypeArraysPtr->healthFlags.degraded == FALSE)
        {
            restoreHealthFlags = eventTypeArraysPtr->healthFlags;
            eventTypeArraysPtr->healthFlags.degraded = TRUE;

            /*
             * Issue MRP to degrade the troubled port.
             * NOTE:  Both PI_GOOD and PI_TIMEOUT are considered success,
             *        since the proc typically eventually executes the MRP
             *        in the PI_TIMEOUT case.
             */
            returnCode = FCM_SetPortHealth(whichEnd, portNumber, &eventTypeArraysPtr->healthFlags);
            if ((returnCode == GOOD) || (returnCode == PI_TIMEOUT))
            {
                /*
                 * Fork task to restore the port in a while
                 */
                parms.p1 = (UINT32)whichEnd;
                parms.p2 = (UINT32)portNumber;
                parms.p3 = (UINT32)eventTypeArraysPtr;
                parms.p4 = (UINT32)TIME_TO_RESTORE_MILLISECONDS;
                parms.p5 = (UINT32)whichEnd;
                TaskCreate(FCM_PortRestoreTask, &parms);
            }
            else
            {
                /*
                 * Got an error setting health flags, so set the flags back
                 * to what they were originally.
                 */
                eventTypeArraysPtr->healthFlags = restoreHealthFlags;
            }
        }
        else
        {
            dprintf(DPRINTF_FCM, "  Port is already degraded - discarding\n");
        }
    }

    /*
     * Wake the FCM health monitor task
     */
    TaskReadyState(fcalHealthMonitorTaskPCBPtr);

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
