/* $Id: HWM_Analyze.c 143007 2010-06-22 14:48:58Z m4 $ */
/*============================================================================
** FILE NAME:       HWM_Analyze.c
** MODULE TITLE:    Hardware monitor data analysis impementation
**
** DESCRIPTION:     Hardware monitor data analysis functions
**
** Copyright (c) 2003-2009 Xiotech Corporation. All rights reserved.
**==========================================================================*/

#include "AsyncEventHandler.h"
#include "debug_files.h"
#include "LOG_Defs.h"
#include "HWM_ConditionStrings.h"
#include "HWM_StatusStrings.h"
#include "XIO_Std.h"
#include "XIO_Const.h"

/*****************************************************************************
** Private defines
*****************************************************************************/
#define PREVENT_ASYNC_EVENT     0
#define DEBUG_ASYNC_EVENT       1
#define ALLOW_ASYNC_EVENT       2

/*****************************************************************************
** Private variables
*****************************************************************************/
static char analyzeDebugString1[30];

/*****************************************************************************
** Private function prototypes
*****************************************************************************/
static UINT32 HWM_AnalyzeMonitorStatus(HWM_STATUS *current, HWM_STATUS *previous);

static UINT32 HWM_AnalyzeCCBStatus(CCB_STATUS *current, CCB_STATUS *previous);

static UINT32 HWM_AnalyzePowerSupplyStatus(POWER_SUPPLY_STATUS *current,
                                           POWER_SUPPLY_STATUS *previous);

static UINT32 HWM_AnalyzeProcessorBoardStatus(PROC_BOARD_STATUS *current,
                                              PROC_BOARD_STATUS *previous);

static UINT32 HWM_AnalyzeProcessorStatus(PROC_BOARD_PROCESSOR_STATUS *current,
                                         PROC_BOARD_PROCESSOR_STATUS *previous);

static UINT32 HWM_AnalyzePowerSupplyVoltagesStatus(POWER_SUPPLY_VOLTAGES_STATUS *current,
                                                   POWER_SUPPLY_VOLTAGES_STATUS
                                                   *previous);

static UINT32 HWM_AnalyzeBufferBoardStatus(BUFFER_BOARD_STATUS *current,
                                           BUFFER_BOARD_STATUS *previous);

static UINT32 HWM_AnalyzeBatteryStatus(BATTERY_STATUS *current, BATTERY_STATUS *previous);

static UINT32 HWM_AnalyzeChargerStatus(CHARGER_STATUS *current, CHARGER_STATUS *previous);

static UINT32 HWM_AnalyzeFuelGaugeStatus(FUEL_GAUGE_STATUS *current,
                                         FUEL_GAUGE_STATUS *previous);

static UINT32 HWM_AnalyzeMainRegulatorStatus(MAIN_REGULATOR_STATUS *current,
                                             MAIN_REGULATOR_STATUS *previous);

static UINT32 HWM_AnalyzeTemperatureStatus(TEMPERATURE_STATUS *current,
                                           TEMPERATURE_STATUS *previous);

static UINT32 HWM_AnalyzeNVRAMBatteryStatus(NVRAM_BATTERY_STATUS *current,
                                            NVRAM_BATTERY_STATUS
                                            *previousNVRAMBatteryStatusPtr);

static UINT32 HWM_AnalyzeStatusFlags(UINT32 createAsyncEvent,
                                     HWM_EVENT_PROPERTIES *current,
                                     HWM_EVENT_PROPERTIES *previous, UINT32 statusSize);

static void HWM_GenerateAlert(UINT32 alertType, HWM_EVENT_PROPERTIES *eventPropertiesPtr,
                              UINT32 statusSize);

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
**  Function Name: HWM_AnalyzeMonitorData
**
**  Parameters: currentMonitorDataPtr
**              previousMonitorDataPtr
**
**  Returns:    GOOD  -
**              ERROR -
**--------------------------------------------------------------------------*/
UINT32 HWM_AnalyzeMonitorData(HWM_DATA *currentMonitorDataPtr,
                              HWM_DATA *previousMonitorDataPtr)
{
    dprintf(DPRINTF_HWM_FUNCTIONS, "%s\n", __func__);

    if (!currentMonitorDataPtr || !previousMonitorDataPtr)
    {
        dprintf(DPRINTF_HWM, "%s: currentMonitorDataPtr and/or previousMonitorDataPtr is NULL\n",
                __func__);

        return ERROR;
    }

    HWM_AnalyzeMonitorStatus(&currentMonitorDataPtr->monitorStatus,
                             &previousMonitorDataPtr->monitorStatus);

    /* Copy the event flags from the child devices */

    currentMonitorDataPtr->eventProperties.flags.value =
        currentMonitorDataPtr->monitorStatus.eventProperties.flags.value;

    /* Update the error and warning counters for the child devices */

    currentMonitorDataPtr->eventProperties.monitorStatistics.eventCounterError =
        currentMonitorDataPtr->monitorStatus.eventProperties.monitorStatistics.eventCounterError;

    currentMonitorDataPtr->eventProperties.monitorStatistics.eventCounterWarning =
        currentMonitorDataPtr->monitorStatus.eventProperties.monitorStatistics.eventCounterWarning;

    /*
     * Overall I2C Monitor status - The children will generate their own events
     * so generating one here only serves to consume (much) more log space.
     */
    HWM_AnalyzeStatusFlags(PREVENT_ASYNC_EVENT,
                           &currentMonitorDataPtr->eventProperties,
                           &previousMonitorDataPtr->eventProperties,
                           sizeof(*currentMonitorDataPtr));

    return GOOD;
}


/*----------------------------------------------------------------------------
**  Function Name: HWM_AnalyzeMonitorStatus
**
**  Parameters: currentMonitorStatusPtr
**              previousMonitorStatusPtr
**
**  Returns:    GOOD  -
**              ERROR -
**--------------------------------------------------------------------------*/
static UINT32 HWM_AnalyzeMonitorStatus(HWM_STATUS *currentMonitorStatusPtr,
                                       HWM_STATUS *previousMonitorStatusPtr)
{
    dprintf(DPRINTF_HWM_FUNCTIONS, "%s\n", __func__);

    if (!currentMonitorStatusPtr || !previousMonitorStatusPtr)
    {
        dprintf(DPRINTF_HWM, "%s: currentMonitorStatusPtr and/or previousMonitorStatusPtr is NULL\n",
                __func__);

        return ERROR;
    }

    HWM_AnalyzeCCBStatus(&currentMonitorStatusPtr->ccbStatus,
                         &previousMonitorStatusPtr->ccbStatus);

    HWM_AnalyzeProcessorBoardStatus(&currentMonitorStatusPtr->procBoardStatus,
                                    &previousMonitorStatusPtr->procBoardStatus);

    HWM_AnalyzePowerSupplyStatus(&currentMonitorStatusPtr->frontEndPowerSupply,
                                 &previousMonitorStatusPtr->frontEndPowerSupply);

    HWM_AnalyzePowerSupplyStatus(&currentMonitorStatusPtr->backEndPowerSupply,
                                 &previousMonitorStatusPtr->backEndPowerSupply);

    HWM_AnalyzeBufferBoardStatus(&currentMonitorStatusPtr->frontEndBufferBoardStatus,
                                 &previousMonitorStatusPtr->frontEndBufferBoardStatus);

    HWM_AnalyzeBufferBoardStatus(&currentMonitorStatusPtr->backEndBufferBoardStatus,
                                 &previousMonitorStatusPtr->backEndBufferBoardStatus);

    /* Copy the event flags from the child devices */

    currentMonitorStatusPtr->eventProperties.flags.value =
        currentMonitorStatusPtr->ccbStatus.eventProperties.flags.value |
        currentMonitorStatusPtr->procBoardStatus.eventProperties.flags.value |
        currentMonitorStatusPtr->frontEndPowerSupply.eventProperties.flags.value |
        currentMonitorStatusPtr->backEndPowerSupply.eventProperties.flags.value |
        currentMonitorStatusPtr->frontEndBufferBoardStatus.eventProperties.flags.value |
        currentMonitorStatusPtr->backEndBufferBoardStatus.eventProperties.flags.value;

    /* Update the error and warning counters for the child devices */

    currentMonitorStatusPtr->eventProperties.monitorStatistics.eventCounterError =
        currentMonitorStatusPtr->ccbStatus.eventProperties.monitorStatistics.eventCounterError +
        currentMonitorStatusPtr->procBoardStatus.eventProperties.monitorStatistics.eventCounterError +
        currentMonitorStatusPtr->frontEndPowerSupply.eventProperties.monitorStatistics.eventCounterError +
        currentMonitorStatusPtr->backEndPowerSupply.eventProperties.monitorStatistics.eventCounterError +
        currentMonitorStatusPtr->frontEndBufferBoardStatus.eventProperties.monitorStatistics.eventCounterError +
        currentMonitorStatusPtr->backEndBufferBoardStatus.eventProperties.monitorStatistics.eventCounterError;

    currentMonitorStatusPtr->eventProperties.monitorStatistics.eventCounterWarning =
        currentMonitorStatusPtr->ccbStatus.eventProperties.monitorStatistics.eventCounterWarning +
        currentMonitorStatusPtr->procBoardStatus.eventProperties.monitorStatistics.eventCounterWarning +
        currentMonitorStatusPtr->frontEndPowerSupply.eventProperties.monitorStatistics.eventCounterWarning +
        currentMonitorStatusPtr->backEndPowerSupply.eventProperties.monitorStatistics.eventCounterWarning +
        currentMonitorStatusPtr->frontEndBufferBoardStatus.eventProperties.monitorStatistics.eventCounterWarning +
        currentMonitorStatusPtr->backEndBufferBoardStatus.eventProperties.monitorStatistics.eventCounterWarning;

    /* Update the status string */

    HWM_GetMonitorStatusString((char *)currentMonitorStatusPtr->eventProperties.description,
                               currentMonitorStatusPtr,
                               sizeof(currentMonitorStatusPtr->eventProperties.description));

    /*
     * Monitor status - The children will generate their own events
     * so generating one here only serves to consume more log space,
     * so prevent the async event generation.
     */
    HWM_AnalyzeStatusFlags(PREVENT_ASYNC_EVENT,
                           &currentMonitorStatusPtr->eventProperties,
                           &previousMonitorStatusPtr->eventProperties,
                           sizeof(*currentMonitorStatusPtr));

    return GOOD;
}


/*----------------------------------------------------------------------------
**  Function Name: HWM_AnalyzeCCBStatus
**
**  Parameters: currentCCBStatusPtr
**              previousCCBStatusPtr
**
**  Returns:    GOOD  -
**              ERROR -
**--------------------------------------------------------------------------*/
static UINT32 HWM_AnalyzeCCBStatus(CCB_STATUS *currentCCBStatusPtr,
                                   CCB_STATUS *previousCCBStatusPtr)
{
    dprintf(DPRINTF_HWM_FUNCTIONS, "%s\n", __func__);

    if (!currentCCBStatusPtr || !previousCCBStatusPtr)
    {
        dprintf(DPRINTF_HWM, "%s: currentCCBStatusPtr and/or previousCCBStatusPtr is NULL\n",
                __func__);

        return ERROR;
    }

    HWM_AnalyzeNVRAMBatteryStatus(&currentCCBStatusPtr->nvramBatteryStatus,
                                  &previousCCBStatusPtr->nvramBatteryStatus);

    /* Copy the event flags from the child devices */

    currentCCBStatusPtr->eventProperties.flags.value =
        currentCCBStatusPtr->nvramBatteryStatus.eventProperties.flags.value |
        currentCCBStatusPtr->ccbBoardEEPROMStatus.eventProperties.flags.value |
        currentCCBStatusPtr->ccbMemoryModuleEEPROMStatus.eventProperties.flags.value;

    /* Update the status string */

    HWM_GetCCBStatusString((char *)currentCCBStatusPtr->eventProperties.description,
                           currentCCBStatusPtr,
                           sizeof(currentCCBStatusPtr->eventProperties.description));

    /*
     * CCBStatus is a collection object, so we need it to check
     * for any alert conditions that the children might have.
     */
    HWM_AnalyzeStatusFlags(ALLOW_ASYNC_EVENT,
                           &currentCCBStatusPtr->eventProperties,
                           &previousCCBStatusPtr->eventProperties,
                           sizeof(*currentCCBStatusPtr));

    return GOOD;
}


/*----------------------------------------------------------------------------
**  Function Name: HWM_AnalyzePowerSupplyStatus
**
**  Parameters: currentPowerSupplyStatusPtr
**              previousPowerSupplyStatusPtr
**
**  Returns:    GOOD  -
**              ERROR -
**--------------------------------------------------------------------------*/
static UINT32 HWM_AnalyzePowerSupplyStatus(POWER_SUPPLY_STATUS
                                           *currentPowerSupplyStatusPtr,
                                           POWER_SUPPLY_STATUS
                                           *previousPowerSupplyStatusPtr)
{
    char        conditionString[MMC_MESSAGE_SIZE] = "\0";

    dprintf(DPRINTF_HWM_FUNCTIONS, "%s\n", __func__);

    if (!currentPowerSupplyStatusPtr || !previousPowerSupplyStatusPtr)
    {
        dprintf(DPRINTF_HWM, "%s: currentPowerSupplyStatusPtr and/or previousPowerSupplyStatusPtr is NULL\n",
                __func__);

        return ERROR;
    }

    /* Clear the event flags before doing any analysis */

    currentPowerSupplyStatusPtr->eventProperties.flags.value = 0;

    /*
     * Look for condition status changes and set the appropriate bits
     * in the eventProperties.flags for the condition.
     */
    switch (currentPowerSupplyStatusPtr->eventProperties.statusCode)
    {
        case STATUS_CODE_VALID:
            break;

        case STATUS_CODE_BUSY:
        case STATUS_CODE_NOT_READY:
            dprintf(DPRINTF_HWM, "%s: Status condition is not ready\n", __func__);
            break;

        case STATUS_CODE_NOT_PRESENT:
            dprintf(DPRINTF_HWM, "%s: Status condition is not present\n", __func__);

            currentPowerSupplyStatusPtr->eventProperties.flags.bits.conditionWarning = TRUE;

            if (currentPowerSupplyStatusPtr->eventProperties.statusCode !=
                previousPowerSupplyStatusPtr->eventProperties.statusCode)
            {
                currentPowerSupplyStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
            }
            break;

        default:
            dprintf(DPRINTF_HWM, "%s: Status condition is ERROR\n", __func__);

            currentPowerSupplyStatusPtr->eventProperties.flags.bits.conditionError = TRUE;

            if (currentPowerSupplyStatusPtr->eventProperties.statusCode !=
                previousPowerSupplyStatusPtr->eventProperties.statusCode)
            {
                currentPowerSupplyStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
            }
            break;
    }

    /* Look for condition changes */

    switch (HWM_GetPowerSupplyConditionString(conditionString,
                                              currentPowerSupplyStatusPtr->powerSupplyCondition.value,
                                              sizeof(conditionString)))
    {
        case EVENT_STRING_SEVERITY_NONE:
        case EVENT_STRING_SEVERITY_NORMAL:
            if (previousPowerSupplyStatusPtr->eventProperties.statusCode != STATUS_CODE_UNKNOWN &&
                previousPowerSupplyStatusPtr->eventProperties.statusCode != STATUS_CODE_NOT_READY &&
                previousPowerSupplyStatusPtr->powerSupplyCondition.value !=
                currentPowerSupplyStatusPtr->powerSupplyCondition.value)
            {
                currentPowerSupplyStatusPtr->eventProperties.flags.bits.conditionChangeInformational = TRUE;
                currentPowerSupplyStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
                currentPowerSupplyStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
            }
            break;

        case EVENT_STRING_SEVERITY_INFO:
            currentPowerSupplyStatusPtr->eventProperties.flags.bits.conditionInformational = TRUE;

            if (previousPowerSupplyStatusPtr->powerSupplyCondition.value !=
                currentPowerSupplyStatusPtr->powerSupplyCondition.value)
            {
                currentPowerSupplyStatusPtr->eventProperties.flags.bits.conditionChangeInformational = TRUE;
            }
            break;

        case EVENT_STRING_SEVERITY_WARNING:
            currentPowerSupplyStatusPtr->eventProperties.flags.bits.conditionWarning = TRUE;

            if (previousPowerSupplyStatusPtr->powerSupplyCondition.value !=
                currentPowerSupplyStatusPtr->powerSupplyCondition.value)
            {
                currentPowerSupplyStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
            }
            break;

        case EVENT_STRING_SEVERITY_ERROR:
            currentPowerSupplyStatusPtr->eventProperties.flags.bits.conditionError = TRUE;

            if (previousPowerSupplyStatusPtr->powerSupplyCondition.value !=
                currentPowerSupplyStatusPtr->powerSupplyCondition.value)
            {
                currentPowerSupplyStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
            }
            break;

        case EVENT_STRING_SEVERITY_DEBUG:
        default:
            currentPowerSupplyStatusPtr->eventProperties.flags.bits.conditionDebug = TRUE;

            if (previousPowerSupplyStatusPtr->powerSupplyCondition.value !=
                currentPowerSupplyStatusPtr->powerSupplyCondition.value)
            {
                currentPowerSupplyStatusPtr->eventProperties.flags.bits.conditionChangeDebug = TRUE;
            }
            break;
    }

    dprintf(DPRINTF_HWM, "%s: Power supply condition is %s\n", __func__, conditionString);

    switch (HWM_GetCoolingFanConditionString(conditionString,
                                             currentPowerSupplyStatusPtr->coolingFanConditionValue,
                                             sizeof(conditionString)))
    {
        case EVENT_STRING_SEVERITY_NONE:
        case EVENT_STRING_SEVERITY_NORMAL:
            if (previousPowerSupplyStatusPtr->eventProperties.statusCode != STATUS_CODE_UNKNOWN &&
                previousPowerSupplyStatusPtr->eventProperties.statusCode != STATUS_CODE_NOT_READY &&
                previousPowerSupplyStatusPtr->coolingFanConditionValue !=
                currentPowerSupplyStatusPtr->coolingFanConditionValue)
            {
                currentPowerSupplyStatusPtr->eventProperties.flags.bits.conditionChangeInformational = TRUE;
                currentPowerSupplyStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
                currentPowerSupplyStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
            }
            break;

        case EVENT_STRING_SEVERITY_INFO:
            currentPowerSupplyStatusPtr->eventProperties.flags.bits.conditionInformational = TRUE;

            if (previousPowerSupplyStatusPtr->coolingFanConditionValue !=
                currentPowerSupplyStatusPtr->coolingFanConditionValue)
            {
                currentPowerSupplyStatusPtr->eventProperties.flags.bits.conditionChangeInformational = TRUE;
            }
            break;

        case EVENT_STRING_SEVERITY_WARNING:
            currentPowerSupplyStatusPtr->eventProperties.flags.bits.conditionWarning = TRUE;

            if (previousPowerSupplyStatusPtr->coolingFanConditionValue !=
                currentPowerSupplyStatusPtr->coolingFanConditionValue)
            {
                currentPowerSupplyStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
            }
            break;

        case EVENT_STRING_SEVERITY_ERROR:
            currentPowerSupplyStatusPtr->eventProperties.flags.bits.conditionError = TRUE;

            if (previousPowerSupplyStatusPtr->coolingFanConditionValue !=
                currentPowerSupplyStatusPtr->coolingFanConditionValue)
            {
                currentPowerSupplyStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
            }
            break;

        case EVENT_STRING_SEVERITY_DEBUG:
        default:
            currentPowerSupplyStatusPtr->eventProperties.flags.bits.conditionDebug = TRUE;

            if (previousPowerSupplyStatusPtr->coolingFanConditionValue !=
                currentPowerSupplyStatusPtr->coolingFanConditionValue)
            {
                currentPowerSupplyStatusPtr->eventProperties.flags.bits.conditionChangeDebug = TRUE;
            }
            break;
    }

    dprintf(DPRINTF_HWM, "%s: Cooling fan condition is %s\n", __func__, conditionString);

    /* Copy the event flags from the child devices */

    currentPowerSupplyStatusPtr->eventProperties.flags.value =
        currentPowerSupplyStatusPtr->eventProperties.flags.value |
        currentPowerSupplyStatusPtr->assemblyEEPROMStatus.eventProperties.flags.value |
        currentPowerSupplyStatusPtr->interfaceEEPROMStatus.eventProperties.flags.value;

    /* Update the status string */

    HWM_GetPowerSupplyStatusString((char *)currentPowerSupplyStatusPtr->eventProperties.description,
                                   currentPowerSupplyStatusPtr,
                                   sizeof(currentPowerSupplyStatusPtr->eventProperties.description));

    /*
     * PowerSupplyStatus is a collection object, so we need it to check
     * for any alert conditions that the children might have.
     */
    HWM_AnalyzeStatusFlags(ALLOW_ASYNC_EVENT,
                           &currentPowerSupplyStatusPtr->eventProperties,
                           &previousPowerSupplyStatusPtr->eventProperties,
                           sizeof(*currentPowerSupplyStatusPtr));

    return GOOD;
}


/*----------------------------------------------------------------------------
**  Function Name: HWM_AnalyzeProcessorBoardStatus
**
**  Parameters: currentProcBoardStatusPtr
**              previousProcBoardStatusPtr
**
**  Returns:    GOOD  -
**              ERROR -
**--------------------------------------------------------------------------*/
static UINT32 HWM_AnalyzeProcessorBoardStatus(PROC_BOARD_STATUS
                                              *currentProcBoardStatusPtr,
                                              PROC_BOARD_STATUS
                                              *previousProcBoardStatusPtr)
{
    UINT32      returnCode = GOOD;

    dprintf(DPRINTF_HWM_FUNCTIONS, "HWM: AnalyzeProcessorBoardStatus\n");

    if ((currentProcBoardStatusPtr != NULL) && (previousProcBoardStatusPtr != NULL))
    {
        HWM_AnalyzeProcessorStatus(&currentProcBoardStatusPtr->frontEndProcessorStatus,
                                   &previousProcBoardStatusPtr->frontEndProcessorStatus);

        HWM_AnalyzeProcessorStatus(&currentProcBoardStatusPtr->backEndProcessorStatus,
                                   &previousProcBoardStatusPtr->backEndProcessorStatus);

        HWM_AnalyzePowerSupplyVoltagesStatus(&currentProcBoardStatusPtr->powerSupplyVoltagesStatus,
                                             &previousProcBoardStatusPtr->powerSupplyVoltagesStatus);

        /*
         * Copy the event flags from the child devices
         */
        currentProcBoardStatusPtr->eventProperties.flags.value =
            currentProcBoardStatusPtr->frontEndProcessorStatus.eventProperties.flags.value |
            currentProcBoardStatusPtr->backEndProcessorStatus.eventProperties.flags.value |
            currentProcBoardStatusPtr->powerSupplyVoltagesStatus.eventProperties.flags.value |
            currentProcBoardStatusPtr->chassisEEPROMStatus.eventProperties.flags.value |
            currentProcBoardStatusPtr->procBoardEEPROMStatus.eventProperties.flags.value;

        /*
         * Update the status string
         */
        HWM_GetProcessorBoardStatusString((char *)currentProcBoardStatusPtr->eventProperties.description,
                                          currentProcBoardStatusPtr,
                                          sizeof(currentProcBoardStatusPtr->eventProperties.description));

        /*
         * ProcessorBoardStatus is a collection object, so we need it to check
         * for any alert conditions that the children might have.
         */
        HWM_AnalyzeStatusFlags(ALLOW_ASYNC_EVENT,
                               &currentProcBoardStatusPtr->eventProperties,
                               &previousProcBoardStatusPtr->eventProperties,
                               sizeof(*currentProcBoardStatusPtr));
    }
    else
    {
        dprintf(DPRINTF_HWM, "APBS: currentProcBoardStatusPtr and/or previousProcBoardStatusPtr is NULL\n");

        returnCode = ERROR;
    }

    return (returnCode);
}


/*----------------------------------------------------------------------------
**  Function Name: HWM_AnalyzeProcessorStatus
**
**  Parameters: currentProcessorStatusPtr
**              previousProcessorStatusPtr
**
**  Returns:    GOOD  -
**              ERROR -
**--------------------------------------------------------------------------*/
static UINT32 HWM_AnalyzeProcessorStatus(PROC_BOARD_PROCESSOR_STATUS
                                         *currentProcessorStatusPtr,
                                         PROC_BOARD_PROCESSOR_STATUS
                                         *previousProcessorStatusPtr)
{
    UINT32      returnCode = GOOD;
    char        conditionString[MMC_MESSAGE_SIZE] = "\0";

    dprintf(DPRINTF_HWM_FUNCTIONS, "HWM: AnalyzeProcessorStatus\n");

    if ((currentProcessorStatusPtr != NULL) && (previousProcessorStatusPtr != NULL))
    {
        /*
         * Analyze the processor's temperature condition
         */
        HWM_AnalyzeTemperatureStatus(&currentProcessorStatusPtr->temperatureStatus,
                                     &previousProcessorStatusPtr->temperatureStatus);

#ifdef I2C_MAG_STYLE_TEMP_EVENTS
        /*
         * Suppress the temperature related messages for the FE and BE
         * processors from the processor events. They are sent out as
         * MAG style TEMP related messages.
         */
        currentProcessorStatusPtr->eventProperties.flags.value = 0;
#else   /* I2C_MAG_STYLE_TEMP_EVENTS */
        /*
         * Copy the event flags from the child devices.
         * Since temperatureStatus is checked before the other devices, we can
         * just overwrite the processorStatus flags with the temperatureStatus flags.
         */
        currentProcessorStatusPtr->eventProperties.flags.value =
            currentProcessorStatusPtr->temperatureStatus.eventProperties.flags.value;
#endif  /* I2C_MAG_STYLE_TEMP_EVENTS */

        /*
         * Look for condition status changes and set the appropriate bits
         * in the eventProperties.flags for the condition.
         */
        switch (currentProcessorStatusPtr->eventProperties.statusCode)
        {
            case STATUS_CODE_VALID:
                break;

            case STATUS_CODE_BUSY:
            case STATUS_CODE_NOT_READY:
                dprintf(DPRINTF_HWM, "APS: Status condition is not ready\n");
                break;

            case STATUS_CODE_NOT_PRESENT:
                dprintf(DPRINTF_HWM, "APS: Status condition is not present\n");

                currentProcessorStatusPtr->eventProperties.flags.bits.conditionWarning = TRUE;

                if (currentProcessorStatusPtr->eventProperties.statusCode !=
                    previousProcessorStatusPtr->eventProperties.statusCode)
                {
                    currentProcessorStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
                }
                break;

            default:
                dprintf(DPRINTF_HWM, "APS: Status condition is ERROR\n");

                currentProcessorStatusPtr->eventProperties.flags.bits.conditionError = TRUE;

                if (currentProcessorStatusPtr->eventProperties.statusCode !=
                    previousProcessorStatusPtr->eventProperties.statusCode)
                {
                    currentProcessorStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
                }
                break;
        }

        /*
         * Look for condition changes
         */
        switch (HWM_GetLimitMonitorConditionString(conditionString,
                                                   currentProcessorStatusPtr->memorySocketSupplyVoltageReading.limitMonitorValue,
                                                   sizeof(conditionString)))
        {
            case EVENT_STRING_SEVERITY_NONE:
            case EVENT_STRING_SEVERITY_NORMAL:
                if ((previousProcessorStatusPtr->eventProperties.statusCode != STATUS_CODE_UNKNOWN) &&
                    (previousProcessorStatusPtr->eventProperties.statusCode != STATUS_CODE_NOT_READY) &&
                    (previousProcessorStatusPtr->memorySocketSupplyVoltageReading.limitMonitorValue !=
                     currentProcessorStatusPtr->memorySocketSupplyVoltageReading.limitMonitorValue))
                {
                    currentProcessorStatusPtr->eventProperties.flags.bits.conditionChangeInformational = TRUE;
                    currentProcessorStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
                    currentProcessorStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_INFO:
                currentProcessorStatusPtr->eventProperties.flags.bits.conditionInformational = TRUE;

                if (previousProcessorStatusPtr->memorySocketSupplyVoltageReading.limitMonitorValue !=
                    currentProcessorStatusPtr->memorySocketSupplyVoltageReading.limitMonitorValue)
                {
                    currentProcessorStatusPtr->eventProperties.flags.bits.conditionChangeInformational = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_WARNING:
                currentProcessorStatusPtr->eventProperties.flags.bits.conditionWarning = TRUE;

                if (previousProcessorStatusPtr->memorySocketSupplyVoltageReading.limitMonitorValue !=
                    currentProcessorStatusPtr->memorySocketSupplyVoltageReading.limitMonitorValue)
                {
                    currentProcessorStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_ERROR:
                currentProcessorStatusPtr->eventProperties.flags.bits.conditionError = TRUE;

                if (previousProcessorStatusPtr->memorySocketSupplyVoltageReading.limitMonitorValue !=
                    currentProcessorStatusPtr->memorySocketSupplyVoltageReading.limitMonitorValue)
                {
                    currentProcessorStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_DEBUG:
            default:
                currentProcessorStatusPtr->eventProperties.flags.bits.conditionDebug = TRUE;

                if (previousProcessorStatusPtr->memorySocketSupplyVoltageReading.limitMonitorValue !=
                    currentProcessorStatusPtr->memorySocketSupplyVoltageReading.limitMonitorValue)
                {
                    currentProcessorStatusPtr->eventProperties.flags.bits.conditionChangeDebug = TRUE;
                }
                break;
        }

        dprintf(DPRINTF_HWM, "APS: Memory socket supply voltage limit is %s\n",
                conditionString);

        switch (HWM_GetProcessorResetConditionString(conditionString,
                                                     currentProcessorStatusPtr->processorResetConditionValue,
                                                     sizeof(conditionString)))
        {
            case EVENT_STRING_SEVERITY_NONE:
            case EVENT_STRING_SEVERITY_NORMAL:
                if ((previousProcessorStatusPtr->eventProperties.statusCode != STATUS_CODE_UNKNOWN) &&
                    (previousProcessorStatusPtr->eventProperties.statusCode != STATUS_CODE_NOT_READY) &&
                    (previousProcessorStatusPtr->processorResetConditionValue !=
                     currentProcessorStatusPtr->processorResetConditionValue))
                {
                    currentProcessorStatusPtr->eventProperties.flags.bits.conditionChangeInformational = TRUE;
                    currentProcessorStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
                    currentProcessorStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_INFO:
                currentProcessorStatusPtr->eventProperties.flags.bits.conditionInformational = TRUE;

                if (previousProcessorStatusPtr->processorResetConditionValue !=
                    currentProcessorStatusPtr->processorResetConditionValue)
                {
                    currentProcessorStatusPtr->eventProperties.flags.bits.conditionChangeInformational = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_WARNING:
                currentProcessorStatusPtr->eventProperties.flags.bits.conditionWarning = TRUE;

                if (previousProcessorStatusPtr->processorResetConditionValue !=
                    currentProcessorStatusPtr->processorResetConditionValue)
                {
                    currentProcessorStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_ERROR:
                currentProcessorStatusPtr->eventProperties.flags.bits.conditionError = TRUE;

                if (previousProcessorStatusPtr->processorResetConditionValue !=
                    currentProcessorStatusPtr->processorResetConditionValue)
                {
                    currentProcessorStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_DEBUG:
            default:
                currentProcessorStatusPtr->eventProperties.flags.bits.conditionDebug = TRUE;

                if (previousProcessorStatusPtr->processorResetConditionValue !=
                    currentProcessorStatusPtr->processorResetConditionValue)
                {
                    currentProcessorStatusPtr->eventProperties.flags.bits.conditionChangeDebug = TRUE;
                }
                break;
        }

        dprintf(DPRINTF_HWM, "APS: Processor reset condition is %s\n", conditionString);

        /*
         * Update the status string
         */
        HWM_GetProcessorStatusString((char *)currentProcessorStatusPtr->eventProperties.description,
                                     currentProcessorStatusPtr,
                                     sizeof(currentProcessorStatusPtr->eventProperties.description));

        /*
         * ProcessorStatus is part of the ProcessorBoardStatus collection object,
         * so we'll let the collection object check for the alert condition.
         */
#ifdef HWM_CHILD_DEVICE_LOG_MESSAGES
        HWM_AnalyzeStatusFlags(ALLOW_ASYNC_EVENT,
#else   /* HWM_CHILD_DEVICE_LOG_MESSAGES */
        HWM_AnalyzeStatusFlags(DEBUG_ASYNC_EVENT,
#endif  /* HWM_CHILD_DEVICE_LOG_MESSAGES */
                               &currentProcessorStatusPtr->eventProperties,
                               &previousProcessorStatusPtr->eventProperties,
                               sizeof(*currentProcessorStatusPtr));
    }
    else
    {
        dprintf(DPRINTF_HWM, "APS: currentProcessorStatusPtr and/or previousProcessorStatusPtr is NULL\n");

        returnCode = ERROR;
    }

    return (returnCode);
}


/*----------------------------------------------------------------------------
**  Function Name: HWM_AnalyzePowerSupplyVoltagesStatus
**
**  Parameters: currentPowerSupplyVoltagesStatus
**              previousPowerSupplyVoltagesStatus
**
**  Returns:    GOOD  -
**              ERROR -
**--------------------------------------------------------------------------*/
static UINT32 HWM_AnalyzePowerSupplyVoltagesStatus(POWER_SUPPLY_VOLTAGES_STATUS
                                                   *currentPowerSupplyVoltagesStatusPtr,
                                                   POWER_SUPPLY_VOLTAGES_STATUS
                                                   *previousPowerSupplyVoltagesStatusPtr)
{
    UINT32      returnCode = GOOD;
    char        conditionString[MMC_MESSAGE_SIZE] = "\0";

    dprintf(DPRINTF_HWM_FUNCTIONS, "HWM: AnalyzePowerSupplyVoltagesStatus\n");

    if ((currentPowerSupplyVoltagesStatusPtr != NULL) &&
        (previousPowerSupplyVoltagesStatusPtr != NULL))
    {
        /*
         * Clear the event flags before doing any analysis
         */
        currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.value = 0;

        /*
         * Look for condition status changes and set the appropriate bits
         * in the eventProperties.flags for the condition.
         */
        switch (currentPowerSupplyVoltagesStatusPtr->eventProperties.statusCode)
        {
            case STATUS_CODE_VALID:
                break;

            case STATUS_CODE_BUSY:
            case STATUS_CODE_NOT_READY:
                dprintf(DPRINTF_HWM, "APSVS: Status condition is not ready\n");
                break;

            case STATUS_CODE_NOT_PRESENT:
                dprintf(DPRINTF_HWM, "APSVS: Status condition is not present\n");

                currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionWarning = TRUE;

                if (currentPowerSupplyVoltagesStatusPtr->eventProperties.statusCode !=
                    previousPowerSupplyVoltagesStatusPtr->eventProperties.statusCode)
                {
                    currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
                }
                break;

            default:
                dprintf(DPRINTF_HWM, "APSVS: Status condition is ERROR\n");

                currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionError = TRUE;

                if (currentPowerSupplyVoltagesStatusPtr->eventProperties.statusCode !=
                    previousPowerSupplyVoltagesStatusPtr->eventProperties.statusCode)
                {
                    currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
                }
                break;
        }

        /*
         * Look for condition changes
         */
        switch (HWM_GetLimitMonitorConditionString(conditionString,
                                                   currentPowerSupplyVoltagesStatusPtr->twelveVoltReading.limitMonitorValue,
                                                   sizeof(conditionString)))
        {
            case EVENT_STRING_SEVERITY_NONE:
            case EVENT_STRING_SEVERITY_NORMAL:
                if ((previousPowerSupplyVoltagesStatusPtr->eventProperties.statusCode != STATUS_CODE_UNKNOWN) &&
                    (previousPowerSupplyVoltagesStatusPtr->eventProperties.statusCode != STATUS_CODE_NOT_READY) &&
                    (previousPowerSupplyVoltagesStatusPtr->twelveVoltReading.limitMonitorValue !=
                     currentPowerSupplyVoltagesStatusPtr->twelveVoltReading.limitMonitorValue))
                {
                    currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionChangeInformational = TRUE;
                    currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
                    currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_INFO:
                currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionInformational = TRUE;

                if (previousPowerSupplyVoltagesStatusPtr->twelveVoltReading.limitMonitorValue !=
                    currentPowerSupplyVoltagesStatusPtr->twelveVoltReading.limitMonitorValue)
                {
                    currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionChangeInformational = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_WARNING:
                currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionWarning = TRUE;

                if (previousPowerSupplyVoltagesStatusPtr->twelveVoltReading.limitMonitorValue !=
                    currentPowerSupplyVoltagesStatusPtr->twelveVoltReading.limitMonitorValue)
                {
                    currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_ERROR:
                currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionError = TRUE;

                if (previousPowerSupplyVoltagesStatusPtr->twelveVoltReading.limitMonitorValue !=
                    currentPowerSupplyVoltagesStatusPtr->twelveVoltReading.limitMonitorValue)
                {
                    currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_DEBUG:
            default:
                currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionDebug = TRUE;

                if (previousPowerSupplyVoltagesStatusPtr->twelveVoltReading.limitMonitorValue !=
                    currentPowerSupplyVoltagesStatusPtr->twelveVoltReading.limitMonitorValue)
                {
                    currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionChangeDebug = TRUE;
                }
                break;
        }

        dprintf(DPRINTF_HWM, "APSVS: 12 volt supply voltage limit is %s\n",
                conditionString);

        switch (HWM_GetLimitMonitorConditionString(conditionString,currentPowerSupplyVoltagesStatusPtr->fiveVoltReading.limitMonitorValue,
                                                   sizeof(conditionString)))
        {
            case EVENT_STRING_SEVERITY_NONE:
            case EVENT_STRING_SEVERITY_NORMAL:
                if ((previousPowerSupplyVoltagesStatusPtr->eventProperties.statusCode != STATUS_CODE_UNKNOWN) &&
                    (previousPowerSupplyVoltagesStatusPtr->eventProperties.statusCode != STATUS_CODE_NOT_READY) &&
                    (previousPowerSupplyVoltagesStatusPtr->fiveVoltReading.limitMonitorValue !=
                     currentPowerSupplyVoltagesStatusPtr->fiveVoltReading.limitMonitorValue))
                {
                    currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionChangeInformational = TRUE;
                    currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
                    currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_INFO:
                currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionInformational = TRUE;

                if (previousPowerSupplyVoltagesStatusPtr->fiveVoltReading.limitMonitorValue !=
                    currentPowerSupplyVoltagesStatusPtr->fiveVoltReading.limitMonitorValue)
                {
                    currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionChangeInformational = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_WARNING:
                currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionWarning = TRUE;

                if (previousPowerSupplyVoltagesStatusPtr->fiveVoltReading.limitMonitorValue !=
                    currentPowerSupplyVoltagesStatusPtr->fiveVoltReading.limitMonitorValue)
                {
                    currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_ERROR:
                currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionError = TRUE;

                if (previousPowerSupplyVoltagesStatusPtr->fiveVoltReading.limitMonitorValue !=
                    currentPowerSupplyVoltagesStatusPtr->fiveVoltReading.limitMonitorValue)
                {
                    currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_DEBUG:
            default:
                currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionDebug = TRUE;

                if (previousPowerSupplyVoltagesStatusPtr->fiveVoltReading.limitMonitorValue !=
                    currentPowerSupplyVoltagesStatusPtr->fiveVoltReading.limitMonitorValue)
                {
                    currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionChangeDebug = TRUE;
                }
                break;
        }

        dprintf(DPRINTF_HWM, "APSVS: 5 volt supply voltage limit is %s\n", conditionString);

        switch (HWM_GetLimitMonitorConditionString(conditionString,
                                                   currentPowerSupplyVoltagesStatusPtr->threePointThreeVoltReading.limitMonitorValue,
                                                   sizeof(conditionString)))
        {
            case EVENT_STRING_SEVERITY_NONE:
            case EVENT_STRING_SEVERITY_NORMAL:
                if ((previousPowerSupplyVoltagesStatusPtr->eventProperties.statusCode != STATUS_CODE_UNKNOWN) &&
                    (previousPowerSupplyVoltagesStatusPtr->eventProperties.statusCode != STATUS_CODE_NOT_READY) &&
                    (previousPowerSupplyVoltagesStatusPtr->threePointThreeVoltReading.limitMonitorValue !=
                     currentPowerSupplyVoltagesStatusPtr->threePointThreeVoltReading.limitMonitorValue))
                {
                    currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionChangeInformational = TRUE;
                    currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
                    currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_INFO:
                currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionInformational = TRUE;

                if (previousPowerSupplyVoltagesStatusPtr->threePointThreeVoltReading.limitMonitorValue !=
                    currentPowerSupplyVoltagesStatusPtr->threePointThreeVoltReading.limitMonitorValue)
                {
                    currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionChangeInformational = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_WARNING:
                currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionWarning = TRUE;

                if (previousPowerSupplyVoltagesStatusPtr->threePointThreeVoltReading.limitMonitorValue !=
                    currentPowerSupplyVoltagesStatusPtr->threePointThreeVoltReading.limitMonitorValue)
                {
                    currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_ERROR:
                currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionError = TRUE;

                if (previousPowerSupplyVoltagesStatusPtr->threePointThreeVoltReading.limitMonitorValue !=
                    currentPowerSupplyVoltagesStatusPtr->threePointThreeVoltReading.limitMonitorValue)
                {
                    currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_DEBUG:
            default:
                currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionDebug = TRUE;

                if (previousPowerSupplyVoltagesStatusPtr->threePointThreeVoltReading.limitMonitorValue !=
                    currentPowerSupplyVoltagesStatusPtr->threePointThreeVoltReading.limitMonitorValue)
                {
                    currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionChangeDebug = TRUE;
                }
                break;
        }

        dprintf(DPRINTF_HWM, "APSVS: 3.3 volt supply voltage limit is %s\n",
                conditionString);

        switch (HWM_GetLimitMonitorConditionString(conditionString,
                                                   currentPowerSupplyVoltagesStatusPtr->standbyVoltageReading.limitMonitorValue,
                                                   sizeof(conditionString)))
        {
            case EVENT_STRING_SEVERITY_NONE:
            case EVENT_STRING_SEVERITY_NORMAL:
                if ((previousPowerSupplyVoltagesStatusPtr->eventProperties.statusCode != STATUS_CODE_UNKNOWN) &&
                    (previousPowerSupplyVoltagesStatusPtr->eventProperties.statusCode != STATUS_CODE_NOT_READY) &&
                    (previousPowerSupplyVoltagesStatusPtr->standbyVoltageReading.limitMonitorValue !=
                     currentPowerSupplyVoltagesStatusPtr->standbyVoltageReading.limitMonitorValue))
                {
                    currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionChangeInformational = TRUE;
                    currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
                    currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_INFO:
                currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionInformational = TRUE;

                if (previousPowerSupplyVoltagesStatusPtr->standbyVoltageReading.limitMonitorValue !=
                    currentPowerSupplyVoltagesStatusPtr->standbyVoltageReading.limitMonitorValue)
                {
                    currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionChangeInformational = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_WARNING:
                currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionWarning = TRUE;

                if (previousPowerSupplyVoltagesStatusPtr->standbyVoltageReading.limitMonitorValue !=
                    currentPowerSupplyVoltagesStatusPtr->standbyVoltageReading.limitMonitorValue)
                {
                    currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_ERROR:
                currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionError = TRUE;

                if (previousPowerSupplyVoltagesStatusPtr->standbyVoltageReading.limitMonitorValue !=
                    currentPowerSupplyVoltagesStatusPtr->standbyVoltageReading.limitMonitorValue)
                {
                    currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_DEBUG:
            default:
                currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionDebug = TRUE;

                if (previousPowerSupplyVoltagesStatusPtr->standbyVoltageReading.limitMonitorValue !=
                    currentPowerSupplyVoltagesStatusPtr->standbyVoltageReading.limitMonitorValue)
                {
                    currentPowerSupplyVoltagesStatusPtr->eventProperties.flags.bits.conditionChangeDebug = TRUE;
                }
                break;
        }

        dprintf(DPRINTF_HWM, "APSVS: 5 volt standby voltage limit is %s\n",
                conditionString);

        /*
         * Update the status string
         */
        HWM_GetPowerSupplyVoltagesStatusString((char *)currentPowerSupplyVoltagesStatusPtr->eventProperties.description,
                                               currentPowerSupplyVoltagesStatusPtr,
                                               sizeof (currentPowerSupplyVoltagesStatusPtr->eventProperties.description));

        /*
         * PowerSupplyVoltagesStatus is part of the ProcessorBoardStatus
         * collection object, so we'll let the collection object check for the
         * alert condition.
         */
#ifdef HWM_CHILD_DEVICE_LOG_MESSAGES
        HWM_AnalyzeStatusFlags(ALLOW_ASYNC_EVENT,
#else   /* HWM_CHILD_DEVICE_LOG_MESSAGES */
        HWM_AnalyzeStatusFlags(DEBUG_ASYNC_EVENT,
#endif  /* HWM_CHILD_DEVICE_LOG_MESSAGES */
                               &currentPowerSupplyVoltagesStatusPtr->eventProperties,
                               &previousPowerSupplyVoltagesStatusPtr->eventProperties,
                               sizeof(*currentPowerSupplyVoltagesStatusPtr));
    }
    else
    {
        dprintf(DPRINTF_HWM, "APSVS: currentPowerSupplyVoltagesStatusPtr and/or previousPowerSupplyVoltagesStatusPtr is NULL\n");

        returnCode = ERROR;
    }

    return (returnCode);
}


/*----------------------------------------------------------------------------
**  Function Name: HWM_AnalyzeBufferBoardStatus
**
**  Parameters: currentBufferBoardStatusPtr
**              previousBufferBoardStatusPtr
**
**  Returns:    GOOD  -
**              ERROR -
**--------------------------------------------------------------------------*/
static UINT32 HWM_AnalyzeBufferBoardStatus(BUFFER_BOARD_STATUS
                                           *currentBufferBoardStatusPtr,
                                           BUFFER_BOARD_STATUS
                                           *previousBufferBoardStatusPtr)
{
    UINT32      returnCode = GOOD;

    dprintf(DPRINTF_HWM_FUNCTIONS, "HWM: AnalyzeBufferBoardStatus\n");

    if ((currentBufferBoardStatusPtr != NULL) && (previousBufferBoardStatusPtr != NULL))
    {
        HWM_AnalyzeBatteryStatus(&currentBufferBoardStatusPtr->batteryStatus,
                                 &previousBufferBoardStatusPtr->batteryStatus);

        HWM_AnalyzeChargerStatus(&currentBufferBoardStatusPtr->chargerStatus,
                                 &previousBufferBoardStatusPtr->chargerStatus);

        HWM_AnalyzeFuelGaugeStatus(&currentBufferBoardStatusPtr->fuelGaugeStatus,
                                   &previousBufferBoardStatusPtr->fuelGaugeStatus);

        HWM_AnalyzeMainRegulatorStatus(&currentBufferBoardStatusPtr->mainRegulatorStatus,
                                       &previousBufferBoardStatusPtr->mainRegulatorStatus);

        HWM_AnalyzeTemperatureStatus(&currentBufferBoardStatusPtr->temperatureStatus,
                                     &previousBufferBoardStatusPtr->temperatureStatus);

        /* Copy the event flags from the child devices */

        currentBufferBoardStatusPtr->eventProperties.flags.value =
            currentBufferBoardStatusPtr->batteryStatus.eventProperties.flags.value |
            currentBufferBoardStatusPtr->chargerStatus.eventProperties.flags.value |
            currentBufferBoardStatusPtr->fuelGaugeStatus.eventProperties.flags.value |
            currentBufferBoardStatusPtr->mainRegulatorStatus.eventProperties.flags.value |
#ifndef I2C_MAG_STYLE_TEMP_EVENTS
            currentBufferBoardStatusPtr->temperatureStatus.eventProperties.flags.value |
#endif  /* I2C_MAG_STYLE_TEMP_EVENTS */
            currentBufferBoardStatusPtr->eepromStatus.eventProperties.flags.value;

        /*
         * Update the status string
         */
        HWM_GetBufferBoardStatusString((char *)currentBufferBoardStatusPtr->eventProperties.description,
                                       currentBufferBoardStatusPtr,
                                       sizeof(currentBufferBoardStatusPtr->eventProperties.description));

        /*
         * BufferBoardStatus is a collection object, so we need it to check
         * for any alert conditions that the children might have.
         * Make a normal async event only if the event string has changed,
         * otherwise allow only a DEBUG type event to be created.
         */
        /*
         * The proc code is now responsible for generating all of the
         * pertinent log messages for the MicroMemory board.  The CCB
         * only monitors the board for statistical purposes (snapshot/debug).
         */
        HWM_AnalyzeStatusFlags(DEBUG_ASYNC_EVENT,
                               &currentBufferBoardStatusPtr->eventProperties,
                               &previousBufferBoardStatusPtr->eventProperties,
                               sizeof(*currentBufferBoardStatusPtr));
    }
    else
    {
        dprintf(DPRINTF_HWM, "ACBS: currentBufferBoardStatusPtr and/or previousBufferBoardStatusPtr is NULL\n");

        returnCode = ERROR;
    }

    return (returnCode);
}


/*****************************************************************************
**  FUNCTION NAME: HWM_AnalyzeBatteryStatus
**
**  PARAMETERS: currentBatteryStatusPtr
**              previousBatteryStatusPtr
**
**  RETURNS:    GOOD  -
**              ERROR -
******************************************************************************/
static UINT32 HWM_AnalyzeBatteryStatus(BATTERY_STATUS *currentBatteryStatusPtr,
                                       BATTERY_STATUS *previousBatteryStatusPtr)
{
    UINT32      returnCode = GOOD;
    char        conditionString[MMC_MESSAGE_SIZE] = "\0";

    dprintf(DPRINTF_HWM_FUNCTIONS, "HWM: AnalyzeBatteryStatus\n");

    if ((currentBatteryStatusPtr != NULL) && (previousBatteryStatusPtr != NULL))
    {
        /*
         * Clear the event flags before doing any analysis
         */
        currentBatteryStatusPtr->eventProperties.flags.value = 0;

        /*
         * Look for condition status changes and set the appropriate bits
         * in the eventProperties.flags for the condition.
         */
        switch (currentBatteryStatusPtr->eventProperties.statusCode)
        {
            case STATUS_CODE_VALID:
                break;

            case STATUS_CODE_BUSY:
            case STATUS_CODE_NOT_READY:
                dprintf(DPRINTF_HWM, "ABS: Status condition is not ready\n");
                break;

            case STATUS_CODE_NOT_PRESENT:
                dprintf(DPRINTF_HWM, "ABS: Status condition is not present\n");

                currentBatteryStatusPtr->eventProperties.flags.bits.conditionWarning = TRUE;

                if (currentBatteryStatusPtr->eventProperties.statusCode !=
                    previousBatteryStatusPtr->eventProperties.statusCode)
                {
                    currentBatteryStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
                }
                break;

            case STATUS_CODE_ERROR:
            default:
                dprintf(DPRINTF_HWM, "APSVS: Status condition is ERROR\n");

                currentBatteryStatusPtr->eventProperties.flags.bits.conditionError = TRUE;

                if (currentBatteryStatusPtr->eventProperties.statusCode !=
                    previousBatteryStatusPtr->eventProperties.statusCode)
                {
                    currentBatteryStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
                }
                break;
        }

        /*
         * Look for condition changes
         */
        switch (HWM_GetBatteryConditionString(conditionString,
                                              currentBatteryStatusPtr->batteryCondition,
                                              sizeof(conditionString)))
        {
            case EVENT_STRING_SEVERITY_NONE:
            case EVENT_STRING_SEVERITY_NORMAL:
                if ((previousBatteryStatusPtr->eventProperties.statusCode != STATUS_CODE_UNKNOWN) &&
                    (previousBatteryStatusPtr->eventProperties.statusCode != STATUS_CODE_NOT_READY) &&
                    (previousBatteryStatusPtr->batteryCondition !=
                     currentBatteryStatusPtr->batteryCondition))
                {
                    currentBatteryStatusPtr->eventProperties.flags.bits.conditionChangeInformational = TRUE;
                    currentBatteryStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
                    currentBatteryStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_INFO:
                currentBatteryStatusPtr->eventProperties.flags.bits.conditionInformational = TRUE;

                if (previousBatteryStatusPtr->batteryCondition !=
                    currentBatteryStatusPtr->batteryCondition)
                {
                    currentBatteryStatusPtr->eventProperties.flags.bits.conditionChangeInformational = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_WARNING:
                currentBatteryStatusPtr->eventProperties.flags.bits.conditionWarning = TRUE;

                if (previousBatteryStatusPtr->batteryCondition !=
                    currentBatteryStatusPtr->batteryCondition)
                {
                    currentBatteryStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_ERROR:
                currentBatteryStatusPtr->eventProperties.flags.bits.conditionError = TRUE;

                if (previousBatteryStatusPtr->batteryCondition !=
                    currentBatteryStatusPtr->batteryCondition)
                {
                    currentBatteryStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_DEBUG:
            default:
                currentBatteryStatusPtr->eventProperties.flags.bits.conditionDebug = TRUE;

                if (previousBatteryStatusPtr->batteryCondition !=
                    currentBatteryStatusPtr->batteryCondition)
                {
                    currentBatteryStatusPtr->eventProperties.flags.bits.conditionChangeDebug = TRUE;
                }
                break;
        }

        dprintf(DPRINTF_HWM, "ABS: Battery condition is %s\n", conditionString);

        switch (HWM_GetLimitMonitorConditionString(conditionString,
                                                   currentBatteryStatusPtr->terminalVoltageReading.limitMonitorValue,
                                                   sizeof(conditionString)))
        {
            case EVENT_STRING_SEVERITY_NONE:
            case EVENT_STRING_SEVERITY_NORMAL:
                if ((previousBatteryStatusPtr->eventProperties.statusCode !=
                     STATUS_CODE_UNKNOWN) &&
                    (previousBatteryStatusPtr->eventProperties.statusCode !=
                     STATUS_CODE_NOT_READY) &&
                    (previousBatteryStatusPtr->terminalVoltageReading.limitMonitorValue !=
                     currentBatteryStatusPtr->terminalVoltageReading.limitMonitorValue))
                {
                    currentBatteryStatusPtr->eventProperties.flags.bits.conditionChangeInformational = TRUE;
                    currentBatteryStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
                    currentBatteryStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_INFO:
                currentBatteryStatusPtr->eventProperties.flags.bits.conditionInformational = TRUE;

                if (previousBatteryStatusPtr->terminalVoltageReading.limitMonitorValue !=
                    currentBatteryStatusPtr->terminalVoltageReading.limitMonitorValue)
                {
                    currentBatteryStatusPtr->eventProperties.flags.bits.conditionChangeInformational = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_WARNING:
                currentBatteryStatusPtr->eventProperties.flags.bits.conditionWarning = TRUE;

                if (previousBatteryStatusPtr->terminalVoltageReading.limitMonitorValue !=
                    currentBatteryStatusPtr->terminalVoltageReading.limitMonitorValue)
                {
                    currentBatteryStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_ERROR:
                currentBatteryStatusPtr->eventProperties.flags.bits.conditionError = TRUE;

                if (previousBatteryStatusPtr->terminalVoltageReading.limitMonitorValue !=
                    currentBatteryStatusPtr->terminalVoltageReading.limitMonitorValue)
                {
                    currentBatteryStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_DEBUG:
            default:
                currentBatteryStatusPtr->eventProperties.flags.bits.conditionDebug = TRUE;

                if (previousBatteryStatusPtr->terminalVoltageReading.limitMonitorValue !=
                    currentBatteryStatusPtr->terminalVoltageReading.limitMonitorValue)
                {
                    currentBatteryStatusPtr->eventProperties.flags.bits.conditionChangeDebug = TRUE;
                }
                break;
        }

        dprintf(DPRINTF_HWM, "ABS: Battery terminal voltage %s\n", conditionString);

        /*
         * Update the status string
         */
        HWM_GetBatteryStatusString((char *)currentBatteryStatusPtr->eventProperties.description, currentBatteryStatusPtr,
                                   sizeof(currentBatteryStatusPtr->eventProperties.description));

        /*
         * BatteryStatus is part of the BufferBoardStatus collection object, so
         * we'll let the collection object check for the alert condition.
         */
#ifdef HWM_CHILD_DEVICE_LOG_MESSAGES
        HWM_AnalyzeStatusFlags(ALLOW_ASYNC_EVENT,
#else   /* HWM_CHILD_DEVICE_LOG_MESSAGES */
        HWM_AnalyzeStatusFlags(DEBUG_ASYNC_EVENT,
#endif  /* HWM_CHILD_DEVICE_LOG_MESSAGES */
                               &currentBatteryStatusPtr->eventProperties,
                               &previousBatteryStatusPtr->eventProperties,
                               sizeof(*currentBatteryStatusPtr));
    }
    else
    {
        dprintf(DPRINTF_HWM, "ABS: currentBatteryStatusPtr and/or previousBatteryStatusPtr is NULL\n");

        returnCode = ERROR;
    }

    return (returnCode);
}


/*****************************************************************************
**  FUNCTION NAME: HWM_AnalyzeChargerStatus
**
**  PARAMETERS: currentChargerStatusPtr
**              previousChargerStatusPtr
**
**  RETURNS:    GOOD  -
**              ERROR -
******************************************************************************/
static UINT32 HWM_AnalyzeChargerStatus(CHARGER_STATUS *currentChargerStatusPtr,
                                       CHARGER_STATUS *previousChargerStatusPtr)
{
    UINT32      returnCode = GOOD;
    char        conditionString[MMC_MESSAGE_SIZE] = "\0";

    dprintf(DPRINTF_HWM_FUNCTIONS, "HWM: AnalyzeChargerStatus\n");

    if ((currentChargerStatusPtr != NULL) && (previousChargerStatusPtr != NULL))
    {
        /*
         * Clear the event flags before doing any analysis
         */
        currentChargerStatusPtr->eventProperties.flags.value = 0;

        /*
         * Look for condition status changes and set the appropriate bits
         * in the eventProperties.flags for the condition.
         */
        switch (currentChargerStatusPtr->eventProperties.statusCode)
        {
            case STATUS_CODE_VALID:
                break;

            case STATUS_CODE_BUSY:
            case STATUS_CODE_NOT_READY:
                dprintf(DPRINTF_HWM, "ACS: Status condition is not ready\n");
                break;

            case STATUS_CODE_NOT_PRESENT:
                dprintf(DPRINTF_HWM, "ACS: Status condition is not present\n");

                currentChargerStatusPtr->eventProperties.flags.bits.conditionWarning = TRUE;

                if (currentChargerStatusPtr->eventProperties.statusCode !=
                    previousChargerStatusPtr->eventProperties.statusCode)
                {
                    currentChargerStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
                }
                break;

            case STATUS_CODE_ERROR:
            default:
                dprintf(DPRINTF_HWM, "ACS: Status condition is ERROR\n");

                currentChargerStatusPtr->eventProperties.flags.bits.conditionError = TRUE;

                if (currentChargerStatusPtr->eventProperties.statusCode !=
                    previousChargerStatusPtr->eventProperties.statusCode)
                {
                    currentChargerStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
                }
                break;
        }

        /*
         * Look for condition changes
         */
        switch (HWM_GetChargerConditionString(conditionString,
                                              currentChargerStatusPtr->chargerCondition,
                                              sizeof(conditionString)))
        {
            case EVENT_STRING_SEVERITY_NONE:
            case EVENT_STRING_SEVERITY_NORMAL:
                if ((previousChargerStatusPtr->eventProperties.statusCode !=
                     STATUS_CODE_UNKNOWN) &&
                    (previousChargerStatusPtr->eventProperties.statusCode !=
                     STATUS_CODE_NOT_READY) &&
                    (previousChargerStatusPtr->chargerCondition !=
                     currentChargerStatusPtr->chargerCondition))
                {
                    currentChargerStatusPtr->eventProperties.flags.bits.conditionChangeInformational = TRUE;
                    currentChargerStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
                    currentChargerStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_INFO:
                currentChargerStatusPtr->eventProperties.flags.bits.conditionInformational = TRUE;

                if (previousChargerStatusPtr->chargerCondition !=
                    currentChargerStatusPtr->chargerCondition)
                {
                    currentChargerStatusPtr->eventProperties.flags.bits.conditionChangeInformational = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_WARNING:
                currentChargerStatusPtr->eventProperties.flags.bits.conditionWarning = TRUE;

                if (previousChargerStatusPtr->chargerCondition !=
                    currentChargerStatusPtr->chargerCondition)
                {
                    currentChargerStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_ERROR:
                currentChargerStatusPtr->eventProperties.flags.bits.conditionError = TRUE;

                if (previousChargerStatusPtr->chargerCondition !=
                    currentChargerStatusPtr->chargerCondition)
                {
                    currentChargerStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_DEBUG:
            default:
                currentChargerStatusPtr->eventProperties.flags.bits.conditionDebug = TRUE;

                if (previousChargerStatusPtr->chargerCondition !=
                    currentChargerStatusPtr->chargerCondition)
                {
                    currentChargerStatusPtr->eventProperties.flags.bits.conditionChangeDebug = TRUE;
                }
                break;
        }

        dprintf(DPRINTF_HWM, "ACS: Charger condition is %s\n", conditionString);

        /*
         * Update the status string
         */
        HWM_GetChargerStatusString((char *)currentChargerStatusPtr->eventProperties.description,
                                   currentChargerStatusPtr,
                                   sizeof(currentChargerStatusPtr->eventProperties.description));

        /*
         * ChargerStatus is part of the BufferBoardStatus collection object, so
         * we'll let the collection object check for the alert condition.
         */
#ifdef HWM_CHILD_DEVICE_LOG_MESSAGES
        HWM_AnalyzeStatusFlags(ALLOW_ASYNC_EVENT,
#else   /* HWM_CHILD_DEVICE_LOG_MESSAGES */
        HWM_AnalyzeStatusFlags(DEBUG_ASYNC_EVENT,
#endif  /* HWM_CHILD_DEVICE_LOG_MESSAGES */
                               &currentChargerStatusPtr->eventProperties,
                               &previousChargerStatusPtr->eventProperties,
                               sizeof(*currentChargerStatusPtr));
    }
    else
    {
        dprintf(DPRINTF_HWM, "ACS: currentChargerStatusPtr and/or previousChargerStatusPtr is NULL\n");

        returnCode = ERROR;
    }

    return (returnCode);
}


/*****************************************************************************
**  FUNCTION NAME: HWM_AnalyzeFuelGaugeStatus
**
**  PARAMETERS: currentFuelGaugeStatusPtr
**              previousFuelGaugeStatusPtr
**
**  RETURNS:    GOOD  -
**              ERROR -
******************************************************************************/
static UINT32 HWM_AnalyzeFuelGaugeStatus(FUEL_GAUGE_STATUS *currentFuelGaugeStatusPtr,
                                         FUEL_GAUGE_STATUS *previousFuelGaugeStatusPtr)
{
    UINT32      returnCode = GOOD;
    char        conditionString[MMC_MESSAGE_SIZE] = "\0";

    dprintf(DPRINTF_HWM_FUNCTIONS, "HWM: AnalyzeFuelGaugeStatus\n");

    if ((currentFuelGaugeStatusPtr != NULL) && (previousFuelGaugeStatusPtr != NULL))
    {
        /*
         * Clear the event flags before doing any analysis
         */
        currentFuelGaugeStatusPtr->eventProperties.flags.value = 0;

        /*
         * Look for condition status changes and set the appropriate bits
         * in the eventProperties.flags for the condition.
         */
        switch (currentFuelGaugeStatusPtr->eventProperties.statusCode)
        {
            case STATUS_CODE_VALID:
                break;

            case STATUS_CODE_BUSY:
            case STATUS_CODE_NOT_READY:
                dprintf(DPRINTF_HWM, "AFGS: Status condition is not ready\n");
                break;

            case STATUS_CODE_NOT_PRESENT:
                dprintf(DPRINTF_HWM, "AFGS: Status condition is not present\n");

                currentFuelGaugeStatusPtr->eventProperties.flags.bits.conditionWarning = TRUE;

                if (currentFuelGaugeStatusPtr->eventProperties.statusCode !=
                    previousFuelGaugeStatusPtr->eventProperties.statusCode)
                {
                    currentFuelGaugeStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
                }
                break;

            case STATUS_CODE_ERROR:
            default:
                dprintf(DPRINTF_HWM, "AFGS: Status condition is ERROR\n");

                currentFuelGaugeStatusPtr->eventProperties.flags.bits.conditionError = TRUE;

                if (currentFuelGaugeStatusPtr->eventProperties.statusCode !=
                    previousFuelGaugeStatusPtr->eventProperties.statusCode)
                {
                    currentFuelGaugeStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
                }
                break;
        }

        /*
         * Look for condition changes
         */
        switch (HWM_GetFuelGaugeConditionString(conditionString,
                                                currentFuelGaugeStatusPtr->fuelGaugeCondition,
                                                sizeof(conditionString)))
        {
            case EVENT_STRING_SEVERITY_NONE:
            case EVENT_STRING_SEVERITY_NORMAL:
                if ((previousFuelGaugeStatusPtr->eventProperties.statusCode != STATUS_CODE_UNKNOWN) &&
                    (previousFuelGaugeStatusPtr->eventProperties.statusCode != STATUS_CODE_NOT_READY) &&
                    (previousFuelGaugeStatusPtr->fuelGaugeCondition != currentFuelGaugeStatusPtr->fuelGaugeCondition))
                {
                    currentFuelGaugeStatusPtr->eventProperties.flags.bits.conditionChangeInformational = TRUE;
                    currentFuelGaugeStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
                    currentFuelGaugeStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_INFO:
                currentFuelGaugeStatusPtr->eventProperties.flags.bits.conditionInformational = TRUE;

                if (previousFuelGaugeStatusPtr->fuelGaugeCondition != currentFuelGaugeStatusPtr->fuelGaugeCondition)
                {
                    currentFuelGaugeStatusPtr->eventProperties.flags.bits.conditionChangeInformational = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_WARNING:
                currentFuelGaugeStatusPtr->eventProperties.flags.bits.conditionWarning = TRUE;

                if (previousFuelGaugeStatusPtr->fuelGaugeCondition !=
                    currentFuelGaugeStatusPtr->fuelGaugeCondition)
                {
                    currentFuelGaugeStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_ERROR:
                currentFuelGaugeStatusPtr->eventProperties.flags.bits.conditionError = TRUE;

                if (previousFuelGaugeStatusPtr->fuelGaugeCondition != currentFuelGaugeStatusPtr->fuelGaugeCondition)
                {
                    currentFuelGaugeStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_DEBUG:
            default:
                currentFuelGaugeStatusPtr->eventProperties.flags.bits.conditionDebug = TRUE;

                if (previousFuelGaugeStatusPtr->fuelGaugeCondition != currentFuelGaugeStatusPtr->fuelGaugeCondition)
                {
                    currentFuelGaugeStatusPtr->eventProperties.flags.bits.conditionChangeDebug = TRUE;
                }
                break;
        }

        dprintf(DPRINTF_HWM, "AFGS: Fuel gauge condition is %s\n", conditionString);

        switch (HWM_GetCurrentFlowConditionString(conditionString,
                                                  currentFuelGaugeStatusPtr->currentFlowCondition,
                                                  sizeof(conditionString)))
        {
            case EVENT_STRING_SEVERITY_NONE:
            case EVENT_STRING_SEVERITY_NORMAL:
                if ((previousFuelGaugeStatusPtr->eventProperties.statusCode != STATUS_CODE_UNKNOWN) &&
                    (previousFuelGaugeStatusPtr->eventProperties.statusCode != STATUS_CODE_NOT_READY) &&
                    (previousFuelGaugeStatusPtr->currentFlowCondition != currentFuelGaugeStatusPtr->currentFlowCondition))
                {
                    currentFuelGaugeStatusPtr->eventProperties.flags.bits.conditionChangeInformational = TRUE;
                    currentFuelGaugeStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
                    currentFuelGaugeStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_INFO:
                currentFuelGaugeStatusPtr->eventProperties.flags.bits.conditionInformational = TRUE;

                if (previousFuelGaugeStatusPtr->currentFlowCondition != currentFuelGaugeStatusPtr->currentFlowCondition)
                {
                    currentFuelGaugeStatusPtr->eventProperties.flags.bits.conditionChangeInformational = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_WARNING:
                currentFuelGaugeStatusPtr->eventProperties.flags.bits.conditionWarning = TRUE;

                if (previousFuelGaugeStatusPtr->currentFlowCondition != currentFuelGaugeStatusPtr->currentFlowCondition)
                {
                    currentFuelGaugeStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_ERROR:
                currentFuelGaugeStatusPtr->eventProperties.flags.bits.conditionError = TRUE;

                if (previousFuelGaugeStatusPtr->currentFlowCondition != currentFuelGaugeStatusPtr->currentFlowCondition)
                {
                    currentFuelGaugeStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_DEBUG:
            default:
                currentFuelGaugeStatusPtr->eventProperties.flags.bits.conditionDebug = TRUE;

                if (previousFuelGaugeStatusPtr->currentFlowCondition != currentFuelGaugeStatusPtr->currentFlowCondition)
                {
                    currentFuelGaugeStatusPtr->eventProperties.flags.bits.conditionChangeDebug = TRUE;
                }
                break;
        }

        dprintf(DPRINTF_HWM, "AFGS: Current flow condition is %s\n", conditionString);

        switch (HWM_GetLimitMonitorConditionString(conditionString,
                                                   currentFuelGaugeStatusPtr->regulatorOutputVoltageReading.limitMonitorValue,
                                                   sizeof(conditionString)))
        {
            case EVENT_STRING_SEVERITY_NONE:
            case EVENT_STRING_SEVERITY_NORMAL:
                if ((previousFuelGaugeStatusPtr->eventProperties.statusCode != STATUS_CODE_UNKNOWN) &&
                    (previousFuelGaugeStatusPtr->eventProperties.statusCode != STATUS_CODE_NOT_READY) &&
                    (previousFuelGaugeStatusPtr->regulatorOutputVoltageReading.limitMonitorValue !=
                     currentFuelGaugeStatusPtr->regulatorOutputVoltageReading.limitMonitorValue))
                {
                    currentFuelGaugeStatusPtr->eventProperties.flags.bits.conditionChangeInformational = TRUE;
                    currentFuelGaugeStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
                    currentFuelGaugeStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_INFO:
                currentFuelGaugeStatusPtr->eventProperties.flags.bits.conditionInformational = TRUE;

                if (previousFuelGaugeStatusPtr->regulatorOutputVoltageReading.limitMonitorValue !=
                    currentFuelGaugeStatusPtr->regulatorOutputVoltageReading.limitMonitorValue)
                {
                    currentFuelGaugeStatusPtr->eventProperties.flags.bits.conditionChangeInformational = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_WARNING:
                currentFuelGaugeStatusPtr->eventProperties.flags.bits.conditionWarning = TRUE;

                if (previousFuelGaugeStatusPtr->regulatorOutputVoltageReading.limitMonitorValue !=
                    currentFuelGaugeStatusPtr->regulatorOutputVoltageReading.limitMonitorValue)
                {
                    currentFuelGaugeStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_ERROR:
                currentFuelGaugeStatusPtr->eventProperties.flags.bits.conditionError = TRUE;

                if (previousFuelGaugeStatusPtr->regulatorOutputVoltageReading.limitMonitorValue !=
                    currentFuelGaugeStatusPtr->regulatorOutputVoltageReading.limitMonitorValue)
                {
                    currentFuelGaugeStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_DEBUG:
            default:
                currentFuelGaugeStatusPtr->eventProperties.flags.bits.conditionDebug = TRUE;

                if (previousFuelGaugeStatusPtr->regulatorOutputVoltageReading.limitMonitorValue !=
                    currentFuelGaugeStatusPtr->regulatorOutputVoltageReading.limitMonitorValue)
                {
                    currentFuelGaugeStatusPtr->eventProperties.flags.bits.conditionChangeDebug = TRUE;
                }
                break;
        }

        dprintf(DPRINTF_HWM, "AFGS: Fuel gauge regulator output voltage limit is %s\n",
                conditionString);

        /*
         * Update the status string
         */
        HWM_GetFuelGaugeStatusString((char *)currentFuelGaugeStatusPtr->eventProperties.description,
                                     currentFuelGaugeStatusPtr,
                                     sizeof(currentFuelGaugeStatusPtr->eventProperties.description));

        /*
         * FuelGaugeStatus is part of the BufferBoardStatus collection object, so
         * we'll let the collection object check for the alert condition.
         */
#ifdef HWM_CHILD_DEVICE_LOG_MESSAGES
        HWM_AnalyzeStatusFlags(ALLOW_ASYNC_EVENT,
#else   /* HWM_CHILD_DEVICE_LOG_MESSAGES */
        HWM_AnalyzeStatusFlags(DEBUG_ASYNC_EVENT,
#endif  /* HWM_CHILD_DEVICE_LOG_MESSAGES */
                               &currentFuelGaugeStatusPtr->eventProperties,
                               &previousFuelGaugeStatusPtr->eventProperties,
                               sizeof(*currentFuelGaugeStatusPtr));
    }
    else
    {
        dprintf(DPRINTF_HWM, "AFGS: currentFuelGaugeStatusPtr and/or previousFuelGaugeStatusPtr is NULL\n");

        returnCode = ERROR;
    }

    return (returnCode);
}


/*****************************************************************************
**  FUNCTION NAME: HWM_AnalyzeMainRegulatorStatus
**
**  PARAMETERS: currentMainRegulatorStatusPtr
**              previousMainRegulatorStatusPtr
**
**  RETURNS:    GOOD  -
**              ERROR -
******************************************************************************/
static UINT32 HWM_AnalyzeMainRegulatorStatus(MAIN_REGULATOR_STATUS
                                             *currentMainRegulatorStatusPtr,
                                             MAIN_REGULATOR_STATUS
                                             *previousMainRegulatorStatusPtr)
{
    UINT32      returnCode = GOOD;
    char        conditionString[MMC_MESSAGE_SIZE] = "\0";

    dprintf(DPRINTF_HWM_FUNCTIONS, "HWM: AnalyzeMainRegulatorStatus\n");

    if ((currentMainRegulatorStatusPtr != NULL) &&
        (previousMainRegulatorStatusPtr != NULL))
    {
        /*
         * Clear the event flags before doing any analysis
         */
        currentMainRegulatorStatusPtr->eventProperties.flags.value = 0;

        /*
         * Look for condition status changes and set the appropriate bits
         * in the eventProperties.flags for the condition.
         */
        switch (currentMainRegulatorStatusPtr->eventProperties.statusCode)
        {
            case STATUS_CODE_VALID:
                break;

            case STATUS_CODE_BUSY:
            case STATUS_CODE_NOT_READY:
                dprintf(DPRINTF_HWM, "AMRS: Status condition is not ready\n");
                break;

            case STATUS_CODE_NOT_PRESENT:
                dprintf(DPRINTF_HWM, "AMRS: Status condition is not present\n");

                currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionWarning = TRUE;

                if (currentMainRegulatorStatusPtr->eventProperties.statusCode !=
                    previousMainRegulatorStatusPtr->eventProperties.statusCode)
                {
                    currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
                }
                break;

            case STATUS_CODE_ERROR:
            default:
                dprintf(DPRINTF_HWM, "AMRS: Status condition is ERROR\n");

                currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionError = TRUE;

                if (currentMainRegulatorStatusPtr->eventProperties.statusCode !=
                    previousMainRegulatorStatusPtr->eventProperties.statusCode)
                {
                    currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
                }
                break;
        }

        /*
         * Look for condition changes
         */
        switch (HWM_GetMainRegulatorConditionString(conditionString,
                                                    currentMainRegulatorStatusPtr->mainRegulatorCondition,
                                                    sizeof(conditionString)))
        {
            case EVENT_STRING_SEVERITY_NONE:
            case EVENT_STRING_SEVERITY_NORMAL:
                if ((previousMainRegulatorStatusPtr->eventProperties.statusCode != STATUS_CODE_UNKNOWN) &&
                    (previousMainRegulatorStatusPtr->eventProperties.statusCode != STATUS_CODE_NOT_READY) &&
                    (previousMainRegulatorStatusPtr->mainRegulatorCondition != currentMainRegulatorStatusPtr->mainRegulatorCondition))
                {
                    currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionChangeInformational = TRUE;
                    currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
                    currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_INFO:
                currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionInformational = TRUE;

                if (previousMainRegulatorStatusPtr->mainRegulatorCondition !=
                    currentMainRegulatorStatusPtr->mainRegulatorCondition)
                {
                    currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionChangeInformational = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_WARNING:
                currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionWarning = TRUE;

                if (previousMainRegulatorStatusPtr->mainRegulatorCondition !=
                    currentMainRegulatorStatusPtr->mainRegulatorCondition)
                {
                    currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_ERROR:
                currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionError = TRUE;

                if (previousMainRegulatorStatusPtr->mainRegulatorCondition !=
                    currentMainRegulatorStatusPtr->mainRegulatorCondition)
                {
                    currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_DEBUG:
            default:
                currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionDebug = TRUE;

                if (previousMainRegulatorStatusPtr->mainRegulatorCondition !=
                    currentMainRegulatorStatusPtr->mainRegulatorCondition)
                {
                    currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionChangeDebug = TRUE;
                }
                break;
        }

        dprintf(DPRINTF_HWM, "AMRS: Main regulator condition is %s\n", conditionString);

        switch (HWM_GetLimitMonitorConditionString(conditionString,
                                                   currentMainRegulatorStatusPtr->inputVoltageReading.limitMonitorValue,
                                                   sizeof(conditionString)))
        {
            case EVENT_STRING_SEVERITY_NONE:
            case EVENT_STRING_SEVERITY_NORMAL:
                if ((previousMainRegulatorStatusPtr->eventProperties.statusCode != STATUS_CODE_UNKNOWN) &&
                    (previousMainRegulatorStatusPtr->eventProperties.statusCode != STATUS_CODE_NOT_READY) &&
                    (previousMainRegulatorStatusPtr->inputVoltageReading.limitMonitorValue !=
                     currentMainRegulatorStatusPtr->inputVoltageReading.limitMonitorValue))
                {
                    currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionChangeInformational = TRUE;
                    currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
                    currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_INFO:
                currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionInformational = TRUE;

                if (previousMainRegulatorStatusPtr->inputVoltageReading.limitMonitorValue !=
                    currentMainRegulatorStatusPtr->inputVoltageReading.limitMonitorValue)
                {
                    currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionChangeInformational = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_WARNING:
                currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionWarning = TRUE;

                if (previousMainRegulatorStatusPtr->inputVoltageReading.limitMonitorValue !=
                    currentMainRegulatorStatusPtr->inputVoltageReading.limitMonitorValue)
                {
                    currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_ERROR:
                currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionError = TRUE;

                if (previousMainRegulatorStatusPtr->inputVoltageReading.limitMonitorValue !=
                    currentMainRegulatorStatusPtr->inputVoltageReading.limitMonitorValue)
                {
                    currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_DEBUG:
            default:
                currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionDebug = TRUE;

                if (previousMainRegulatorStatusPtr->inputVoltageReading.limitMonitorValue !=
                    currentMainRegulatorStatusPtr->inputVoltageReading.limitMonitorValue)
                {
                    currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionChangeDebug = TRUE;
                }
                break;
        }

        dprintf(DPRINTF_HWM, "AMRS: Input voltage limit is %s\n", conditionString);

        switch (HWM_GetLimitMonitorConditionString(conditionString,
                                                   currentMainRegulatorStatusPtr->outputVoltageReading.limitMonitorValue,
                                                   sizeof(conditionString)))
        {
            case EVENT_STRING_SEVERITY_NONE:
            case EVENT_STRING_SEVERITY_NORMAL:
                if ((previousMainRegulatorStatusPtr->eventProperties.statusCode != STATUS_CODE_UNKNOWN) &&
                    (previousMainRegulatorStatusPtr->eventProperties.statusCode != STATUS_CODE_NOT_READY) &&
                    (previousMainRegulatorStatusPtr->outputVoltageReading.limitMonitorValue !=
                     currentMainRegulatorStatusPtr->outputVoltageReading.limitMonitorValue))
                {
                    currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionChangeInformational = TRUE;
                    currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
                    currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_INFO:
                currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionInformational = TRUE;

                if (previousMainRegulatorStatusPtr->outputVoltageReading.limitMonitorValue !=
                    currentMainRegulatorStatusPtr->outputVoltageReading.limitMonitorValue)
                {
                    currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionChangeInformational = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_WARNING:
                currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionWarning = TRUE;

                if (previousMainRegulatorStatusPtr->outputVoltageReading.limitMonitorValue !=
                    currentMainRegulatorStatusPtr->outputVoltageReading.limitMonitorValue)
                {
                    currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_ERROR:
                currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionError = TRUE;

                if (previousMainRegulatorStatusPtr->outputVoltageReading.limitMonitorValue !=
                    currentMainRegulatorStatusPtr->outputVoltageReading.limitMonitorValue)
                {
                    currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_DEBUG:
            default:
                currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionDebug = TRUE;

                if (previousMainRegulatorStatusPtr->outputVoltageReading.limitMonitorValue !=
                    currentMainRegulatorStatusPtr->outputVoltageReading.limitMonitorValue)
                {
                    currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionChangeDebug = TRUE;
                }
                break;
        }

        dprintf(DPRINTF_HWM, "AMRS: Output voltage limit is %s\n", conditionString);

        switch (HWM_GetLimitMonitorConditionString(conditionString,
                                                   currentMainRegulatorStatusPtr->procBoardSupplyVoltageReading.limitMonitorValue,
                                                   sizeof(conditionString)))
        {
            case EVENT_STRING_SEVERITY_NONE:
            case EVENT_STRING_SEVERITY_NORMAL:
                if ((previousMainRegulatorStatusPtr->eventProperties.statusCode != STATUS_CODE_UNKNOWN) &&
                    (previousMainRegulatorStatusPtr->eventProperties.statusCode != STATUS_CODE_NOT_READY) &&
                    (previousMainRegulatorStatusPtr->procBoardSupplyVoltageReading.limitMonitorValue !=
                     currentMainRegulatorStatusPtr->procBoardSupplyVoltageReading.limitMonitorValue))
                {
                    currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionChangeInformational = TRUE;
                    currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
                    currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_INFO:
                currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionInformational = TRUE;

                if (previousMainRegulatorStatusPtr->procBoardSupplyVoltageReading.limitMonitorValue !=
                    currentMainRegulatorStatusPtr->procBoardSupplyVoltageReading.limitMonitorValue)
                {
                    currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionChangeInformational = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_WARNING:
                currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionWarning = TRUE;

                if (previousMainRegulatorStatusPtr->procBoardSupplyVoltageReading.limitMonitorValue !=
                    currentMainRegulatorStatusPtr->procBoardSupplyVoltageReading.limitMonitorValue)
                {
                    currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_ERROR:
                currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionError = TRUE;

                if (previousMainRegulatorStatusPtr->procBoardSupplyVoltageReading.limitMonitorValue !=
                    currentMainRegulatorStatusPtr->procBoardSupplyVoltageReading.limitMonitorValue)
                {
                    currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_DEBUG:
            default:
                currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionDebug = TRUE;

                if (previousMainRegulatorStatusPtr->procBoardSupplyVoltageReading.limitMonitorValue !=
                    currentMainRegulatorStatusPtr->procBoardSupplyVoltageReading.limitMonitorValue)
                {
                    currentMainRegulatorStatusPtr->eventProperties.flags.bits.conditionChangeDebug = TRUE;
                }
                break;
        }

        dprintf(DPRINTF_HWM, "AMRS: Proc board supply voltage limit is %s\n",
                conditionString);

        /*
         * Update the status string
         */
        HWM_GetMainRegulatorStatusString((char *)currentMainRegulatorStatusPtr->eventProperties.description,
                                         currentMainRegulatorStatusPtr,
                                         sizeof(currentMainRegulatorStatusPtr->eventProperties.description));

        /*
         * MainRegulatorStatus is part of the BufferBoardStatus collection object, so
         * we'll let the collection object check for the alert condition.
         */
#ifdef HWM_CHILD_DEVICE_LOG_MESSAGES
        HWM_AnalyzeStatusFlags(ALLOW_ASYNC_EVENT,
#else   /* HWM_CHILD_DEVICE_LOG_MESSAGES */
        HWM_AnalyzeStatusFlags(DEBUG_ASYNC_EVENT,
#endif  /* HWM_CHILD_DEVICE_LOG_MESSAGES */
                               &currentMainRegulatorStatusPtr->eventProperties,
                               &previousMainRegulatorStatusPtr->eventProperties,
                               sizeof(*currentMainRegulatorStatusPtr));
    }
    else
    {
        dprintf(DPRINTF_HWM, "AMRS: currentMainRegulatorStatusPtr and/or previousMainRegulatorStatusPtr is NULL\n");

        returnCode = ERROR;
    }

    return (returnCode);
}


/*****************************************************************************
**  FUNCTION NAME: HWM_AnalyzeTemperatureStatus
**
**  PARAMETERS: currentTemperatureStatusPtr
**              previousTemperatureStatusPtr
**
**  RETURNS:    GOOD  -
**              ERROR -
******************************************************************************/
static UINT32 HWM_AnalyzeTemperatureStatus(TEMPERATURE_STATUS
                                           *currentTemperatureStatusPtr,
                                           TEMPERATURE_STATUS
                                           *previousTemperatureStatusPtr)
{
    UINT32      returnCode = GOOD;
    char        conditionString[MMC_MESSAGE_SIZE] = "\0";

    dprintf(DPRINTF_HWM_FUNCTIONS, "HWM: AnalyzeTemperatureStatus\n");

    if ((currentTemperatureStatusPtr != NULL) && (previousTemperatureStatusPtr != NULL))
    {
        /*
         * Clear the event flags before doing any analysis
         */
        currentTemperatureStatusPtr->eventProperties.flags.value = 0;

        /*
         * Look for condition status changes and set the appropriate bits
         * in the eventProperties.flags for the condition.
         */
        switch (currentTemperatureStatusPtr->eventProperties.statusCode)
        {
            case STATUS_CODE_VALID:
                break;

            case STATUS_CODE_BUSY:
            case STATUS_CODE_NOT_READY:
                dprintf(DPRINTF_HWM, "ATS: Status condition is not ready\n");
                break;

            case STATUS_CODE_NOT_PRESENT:
                dprintf(DPRINTF_HWM, "ATS: Status condition is not present\n");

                currentTemperatureStatusPtr->eventProperties.flags.bits.conditionWarning = TRUE;

                if (currentTemperatureStatusPtr->eventProperties.statusCode !=
                    previousTemperatureStatusPtr->eventProperties.statusCode)
                {
                    currentTemperatureStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
                }
                break;

            case STATUS_CODE_ERROR:
            default:
                dprintf(DPRINTF_HWM, "ATS: Status condition is ERROR\n");

                currentTemperatureStatusPtr->eventProperties.flags.bits.conditionError = TRUE;

                if (currentTemperatureStatusPtr->eventProperties.statusCode !=
                    previousTemperatureStatusPtr->eventProperties.statusCode)
                {
                    currentTemperatureStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
                }
                break;
        }

        /*
         * Look for condition changes
         */
        switch (HWM_GetTemperatureConditionString(conditionString,
                                                  currentTemperatureStatusPtr->conditionValue,
                                                  sizeof(conditionString)))
        {
            case EVENT_STRING_SEVERITY_NONE:
            case EVENT_STRING_SEVERITY_NORMAL:
                if ((previousTemperatureStatusPtr->eventProperties.statusCode != STATUS_CODE_UNKNOWN) &&
                    (previousTemperatureStatusPtr->eventProperties.statusCode != STATUS_CODE_NOT_READY) &&
                    (previousTemperatureStatusPtr->conditionValue !=
                     currentTemperatureStatusPtr->conditionValue))
                {
                    currentTemperatureStatusPtr->eventProperties.flags.bits.conditionChangeInformational = TRUE;
                    currentTemperatureStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
                    currentTemperatureStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_INFO:
                currentTemperatureStatusPtr->eventProperties.flags.bits.conditionInformational = TRUE;

                if (previousTemperatureStatusPtr->conditionValue !=
                    currentTemperatureStatusPtr->conditionValue)
                {
                    currentTemperatureStatusPtr->eventProperties.flags.bits.conditionChangeInformational = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_WARNING:
                currentTemperatureStatusPtr->eventProperties.flags.bits.conditionWarning = TRUE;

                if (previousTemperatureStatusPtr->conditionValue !=
                    currentTemperatureStatusPtr->conditionValue)
                {
                    currentTemperatureStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_ERROR:
                currentTemperatureStatusPtr->eventProperties.flags.bits.conditionError = TRUE;

                if (previousTemperatureStatusPtr->conditionValue !=
                    currentTemperatureStatusPtr->conditionValue)
                {
                    currentTemperatureStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_DEBUG:
            default:
                currentTemperatureStatusPtr->eventProperties.flags.bits.conditionDebug = TRUE;

                if (previousTemperatureStatusPtr->conditionValue !=
                    currentTemperatureStatusPtr->conditionValue)
                {
                    currentTemperatureStatusPtr->eventProperties.flags.bits.conditionChangeDebug = TRUE;
                }
                break;
        }

        dprintf(DPRINTF_HWM, "ATS: Temperature condition is %s\n", conditionString);

        /*
         * Update the status string
         */
        HWM_GetTemperatureStatusString((char *)currentTemperatureStatusPtr->eventProperties.description,
                                       currentTemperatureStatusPtr,
                                       sizeof(currentTemperatureStatusPtr->eventProperties.description));

        /*
         * TemperatureStatus is part of the BufferBoardStatus or
         * ProcessorBoardStatus collection object, so we'll let the collection
         * object check for the alert condition.
         */
#ifdef HWM_CHILD_DEVICE_LOG_MESSAGES
#ifdef I2C_MAG_STYLE_TEMP_EVENTS
        /*
         * Since MAG style messages display the temperature reading
         * in the message string, the AnalyzeStatus function would
         * generate a new message for each change in temperature
         * during a WARNING or ERROR condition.  Only allow the async
         * event when there's a change in the temperature condition.
         */
        if (currentTemperatureStatusPtr->conditionValue !=
            previousTemperatureStatusPtr->conditionValue)
#endif  /* I2C_MAG_STYLE_TEMP_EVENTS */
        {

            HWM_AnalyzeStatusFlags(ALLOW_ASYNC_EVENT,
                                   &currentTemperatureStatusPtr->eventProperties,
                                   &previousTemperatureStatusPtr->eventProperties,
                                   sizeof(*currentTemperatureStatusPtr));
        }
#else   /* HWM_CHILD_DEVICE_LOG_MESSAGES */
#ifdef I2C_MAG_STYLE_TEMP_EVENTS
        /*
         * Since MAG style messages display the temperature reading
         * in the message string, the AnalyzeStatus function would
         * generate a new message for each change in temperature
         * during a WARNING or ERROR condition.  Only allow the async
         * event when there's a change in the temperature condition.
         */
        if (currentTemperatureStatusPtr->conditionValue !=
            previousTemperatureStatusPtr->conditionValue)
        {
            HWM_AnalyzeStatusFlags(ALLOW_ASYNC_EVENT,
                                   &currentTemperatureStatusPtr->eventProperties,
                                   &previousTemperatureStatusPtr->eventProperties,
                                   sizeof(*currentTemperatureStatusPtr));
        }
        else
        {
            HWM_AnalyzeStatusFlags(PREVENT_ASYNC_EVENT,
                                   &currentTemperatureStatusPtr->eventProperties,
                                   &previousTemperatureStatusPtr->eventProperties,
                                   sizeof(*currentTemperatureStatusPtr));
        }
#else   /* I2C_MAG_STYLE_TEMP_EVENTS */
        HWM_AnalyzeStatusFlags(DEBUG_ASYNC_EVENT,
                               &currentTemperatureStatusPtr->eventProperties,
                               &previousTemperatureStatusPtr->eventProperties,
                               sizeof(*currentTemperatureStatusPtr));
#endif  /* I2C_MAG_STYLE_TEMP_EVENTS */
#endif  /* HWM_CHILD_DEVICE_LOG_MESSAGES */
    }
    else
    {
        dprintf(DPRINTF_HWM, "ATS: currentTemperatureStatusPtr and/or previousTemperatureStatusPtr is NULL\n");

        returnCode = ERROR;
    }

    return (returnCode);
}


/*----------------------------------------------------------------------------
**  FUNCTION NAME: HWM_AnalyzeNVRAMBatteryStatus
**
**  PARAMETERS: currentNVRAMBatteryStatusPtr
**              previousNVRAMBatteryStatusPtr
**
**  RETURNS:    GOOD  -
**              ERROR -
**--------------------------------------------------------------------------*/
static UINT32 HWM_AnalyzeNVRAMBatteryStatus(NVRAM_BATTERY_STATUS_PTR
                                            currentNVRAMBatteryStatusPtr,
                                            NVRAM_BATTERY_STATUS_PTR
                                            previousNVRAMBatteryStatusPtr)
{
    UINT32      returnCode = GOOD;
    char        conditionString[MMC_MESSAGE_SIZE] = "\0";

    dprintf(DPRINTF_HWM_FUNCTIONS, "HWM: AnalyzeNVRAMBatteryStatus\n");

    if ((currentNVRAMBatteryStatusPtr != NULL) && (previousNVRAMBatteryStatusPtr != NULL))
    {
        /*
         * Clear the event flags before doing any analysis
         */
        currentNVRAMBatteryStatusPtr->eventProperties.flags.value = 0;

        /*
         * Look for condition status changes and set the appropriate bits
         * in the eventProperties.flags for the condition.
         */
        switch (currentNVRAMBatteryStatusPtr->eventProperties.statusCode)
        {
            case STATUS_CODE_VALID:
                break;

            case STATUS_CODE_BUSY:
            case STATUS_CODE_NOT_READY:
                dprintf(DPRINTF_HWM, "ANBS: Status condition is not ready\n");
                break;

            case STATUS_CODE_NOT_PRESENT:
                dprintf(DPRINTF_HWM, "ANBS: Status condition is not present\n");

                currentNVRAMBatteryStatusPtr->eventProperties.flags.bits.conditionWarning = TRUE;

                if (currentNVRAMBatteryStatusPtr->eventProperties.statusCode !=
                    previousNVRAMBatteryStatusPtr->eventProperties.statusCode)
                {
                    currentNVRAMBatteryStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
                }
                break;

            case STATUS_CODE_ERROR:
            default:
                dprintf(DPRINTF_HWM, "ANBS: Status condition is ERROR\n");

                currentNVRAMBatteryStatusPtr->eventProperties.flags.bits.conditionError = TRUE;

                if (currentNVRAMBatteryStatusPtr->eventProperties.statusCode !=
                    previousNVRAMBatteryStatusPtr->eventProperties.statusCode)
                {
                    currentNVRAMBatteryStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
                }
                break;
        }

        /*
         * Look for condition changes
         */
        switch (HWM_GetNVRAMBatteryConditionString(conditionString,
                                                   currentNVRAMBatteryStatusPtr->nvramBatteryCondition,
                                                   sizeof(conditionString)))
        {
            case EVENT_STRING_SEVERITY_NONE:
            case EVENT_STRING_SEVERITY_NORMAL:
                if ((previousNVRAMBatteryStatusPtr->eventProperties.statusCode != STATUS_CODE_UNKNOWN) &&
                    (previousNVRAMBatteryStatusPtr->eventProperties.statusCode != STATUS_CODE_NOT_READY) &&
                    (previousNVRAMBatteryStatusPtr->nvramBatteryCondition !=
                     currentNVRAMBatteryStatusPtr->nvramBatteryCondition))
                {
                    currentNVRAMBatteryStatusPtr->eventProperties.flags.bits.conditionChangeInformational = TRUE;
                    currentNVRAMBatteryStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
                    currentNVRAMBatteryStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_INFO:
                currentNVRAMBatteryStatusPtr->eventProperties.flags.bits.conditionInformational = TRUE;

                if (previousNVRAMBatteryStatusPtr->nvramBatteryCondition !=
                    currentNVRAMBatteryStatusPtr->nvramBatteryCondition)
                {
                    currentNVRAMBatteryStatusPtr->eventProperties.flags.bits.conditionChangeInformational = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_WARNING:
                currentNVRAMBatteryStatusPtr->eventProperties.flags.bits.conditionWarning = TRUE;

                if (previousNVRAMBatteryStatusPtr->nvramBatteryCondition !=
                    currentNVRAMBatteryStatusPtr->nvramBatteryCondition)
                {
                    currentNVRAMBatteryStatusPtr->eventProperties.flags.bits.conditionChangeWarning = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_ERROR:
                currentNVRAMBatteryStatusPtr->eventProperties.flags.bits.conditionError = TRUE;

                if (previousNVRAMBatteryStatusPtr->nvramBatteryCondition !=
                    currentNVRAMBatteryStatusPtr->nvramBatteryCondition)
                {
                    currentNVRAMBatteryStatusPtr->eventProperties.flags.bits.conditionChangeError = TRUE;
                }
                break;

            case EVENT_STRING_SEVERITY_DEBUG:
            default:
                currentNVRAMBatteryStatusPtr->eventProperties.flags.bits.conditionDebug = TRUE;

                if (previousNVRAMBatteryStatusPtr->nvramBatteryCondition !=
                    currentNVRAMBatteryStatusPtr->nvramBatteryCondition)
                {
                    currentNVRAMBatteryStatusPtr->eventProperties.flags.bits.conditionChangeDebug = TRUE;
                }
                break;
        }

        dprintf(DPRINTF_HWM, "ANBS: NVRAM battery condition is %s\n", conditionString);

        /*
         * Update the status string
         */
        HWM_GetNVRAMBatteryStatusString((char *)currentNVRAMBatteryStatusPtr->eventProperties.description,
                                        currentNVRAMBatteryStatusPtr,
                                        sizeof(currentNVRAMBatteryStatusPtr->eventProperties.description));

        /*
         * NVRAMBatteryStatus is part of the ccbStatus collection object,
         * so we'll let the collection object check for the alert condition.
         */
#ifdef HWM_CHILD_DEVICE_LOG_MESSAGES
        HWM_AnalyzeStatusFlags(ALLOW_ASYNC_EVENT,
#else   /* HWM_CHILD_DEVICE_LOG_MESSAGES */
        HWM_AnalyzeStatusFlags(DEBUG_ASYNC_EVENT,
#endif  /* HWM_CHILD_DEVICE_LOG_MESSAGES */
                               &currentNVRAMBatteryStatusPtr->eventProperties,
                               &previousNVRAMBatteryStatusPtr->eventProperties,
                               sizeof(*currentNVRAMBatteryStatusPtr));
    }
    else
    {
        dprintf(DPRINTF_HWM, "ANBS: currentNVRAMBatteryStatusPtr and/or previousNVRAMBatteryStatusPtr is NULL\n");

        returnCode = ERROR;
    }

    return (returnCode);
}


/*----------------------------------------------------------------------------
**  Function Name: HWM_AnalyzeStatusFlags
**
**  Comments:   Generates and sends an alert message, if needed.
**
**  Parameters: createAsyncEvent (ALLOW_ASYNC_EVENT or PREVENT_ASYNC_EVENT)
**              currentEventPropertiesPtr
**              previousEventPropertiesPtr
**              statusSize
**
**  Returns:    TRUE  - Alert generated
**              FALSE - Alert not generated
**--------------------------------------------------------------------------*/
static UINT32 HWM_AnalyzeStatusFlags(UINT32 isAsyncEventAllowed,
                                     HWM_EVENT_PROPERTIES *currentEventPropertiesPtr,
                                     HWM_EVENT_PROPERTIES *previousEventPropertiesPtr,
                                     UINT32 statusSize)
{
    UINT32      returnCode = FALSE;
    char        eventStringChange = TRUE;

    dprintf(DPRINTF_HWM_FUNCTIONS, "HWM: AnalyzeStatusFlags\n");

    if (!currentEventPropertiesPtr || !previousEventPropertiesPtr)
    {
        dprintf(DPRINTF_HWM, "ASF: currentEventPropertiesPtr and/or previousEventPropertiesPtr is NULL\n");
        return FALSE;
    }

    HWM_GetComponentIDString(analyzeDebugString1,
                             currentEventPropertiesPtr->componentID,
                             sizeof(analyzeDebugString1));

    dprintf(DPRINTF_I2C_DEVICE_STATUS, "%s: %s\n", __func__, analyzeDebugString1);

    /*
     * Only allow a normal asyncEvent to be generated if the eventString
     * has changed.  This prevents the user from seeing many events with
     * the same log text and saves on CCB log space.
     */
    if (strcmp((char *)previousEventPropertiesPtr->description,
               (char *)currentEventPropertiesPtr->description) == 0)
    {
        eventStringChange = FALSE;

        if (isAsyncEventAllowed == ALLOW_ASYNC_EVENT)
        {
            isAsyncEventAllowed = PREVENT_ASYNC_EVENT;
        }
    }
    else
    {
        dprintf(DPRINTF_I2C_DEVICE_STATUS, "  Description changed\n");
        dprintf(DPRINTF_I2C_DEVICE_STATUS, "    Previous: [%s]\n",
                previousEventPropertiesPtr->description);
        dprintf(DPRINTF_I2C_DEVICE_STATUS, "    Current:  [%s]\n",
                currentEventPropertiesPtr->description);
    }

    dprintf(DPRINTF_I2C_DEVICE_STATUS, "  Previous: 0x%02hhx  Current: 0x%02hhx\n",
            previousEventPropertiesPtr->flags.value,
            currentEventPropertiesPtr->flags.value);

    /*
     * Look for a condition change when coming from WARNING or ERROR
     * and flag is as an informational condition.
     */
    if (previousEventPropertiesPtr->flags.bits.conditionWarning == TRUE ||
        previousEventPropertiesPtr->flags.bits.conditionError == TRUE)
    {
        dprintf(DPRINTF_I2C_DEVICE_STATUS, "  Coming from WARNING or ERROR condition\n");

        currentEventPropertiesPtr->flags.bits.conditionInformational = TRUE;
    }

    if (currentEventPropertiesPtr->flags.bits.conditionError == TRUE)
    {
        /*
         * Generate an error event if one had not been generated previously
         * or the error status has changed.
         */
        if (previousEventPropertiesPtr->flags.bits.conditionError == FALSE ||
            currentEventPropertiesPtr->flags.bits.conditionChangeError == TRUE ||
            eventStringChange == TRUE)
        {
            dprintf(DPRINTF_I2C_DEVICE_STATUS, "  Alert-%s: ERROR\n",
                    analyzeDebugString1);

            if (isAsyncEventAllowed == ALLOW_ASYNC_EVENT)
            {
                HWM_GenerateAlert(LOG_HARDWARE_MONITOR_ERROR, currentEventPropertiesPtr, statusSize);

                currentEventPropertiesPtr->monitorStatistics.eventCounterError++;
            }
            else if (isAsyncEventAllowed == DEBUG_ASYNC_EVENT)
            {
                HWM_GenerateAlert(LOG_HARDWARE_MONITOR_DEBUG, currentEventPropertiesPtr, statusSize);
            }
            else
            {
                if (currentEventPropertiesPtr->statusCode != STATUS_CODE_ERROR)
                {
                    currentEventPropertiesPtr->monitorStatistics.eventCounterError++;
                }
            }

            returnCode = TRUE;
        }
        else
        {
            dprintf(DPRINTF_I2C_DEVICE_STATUS, "  Alert-%s: CONTINUED ERROR\n",
                    analyzeDebugString1);
        }
    }
    else if (currentEventPropertiesPtr->flags.bits.conditionWarning == TRUE)
    {
        /*
         * Generate a warning event if one had not been generated previously
         * or the warning status has changed.
         */
        if (previousEventPropertiesPtr->flags.bits.conditionWarning == FALSE ||
            previousEventPropertiesPtr->flags.bits.conditionError == TRUE ||
            currentEventPropertiesPtr->flags.bits.conditionChangeWarning == TRUE ||
            eventStringChange == TRUE)
        {
            dprintf(DPRINTF_I2C_DEVICE_STATUS, "  Alert-%s: WARNING\n",
                    analyzeDebugString1);

            if (isAsyncEventAllowed == ALLOW_ASYNC_EVENT)
            {
                HWM_GenerateAlert(LOG_HARDWARE_MONITOR_WARN, currentEventPropertiesPtr, statusSize);

                currentEventPropertiesPtr->monitorStatistics.eventCounterWarning++;
            }
            else if (isAsyncEventAllowed == DEBUG_ASYNC_EVENT)
            {
                HWM_GenerateAlert(LOG_HARDWARE_MONITOR_DEBUG, currentEventPropertiesPtr, statusSize);
            }
            else
            {
                if (currentEventPropertiesPtr->statusCode != STATUS_CODE_ERROR)
                {
                    currentEventPropertiesPtr->monitorStatistics.eventCounterWarning++;
                }
            }

            returnCode = TRUE;
        }
        else
        {
            dprintf(DPRINTF_I2C_DEVICE_STATUS, "  Alert-%s: CONTINUED WARNING\n",
                    analyzeDebugString1);
        }
    }
    else if (currentEventPropertiesPtr->flags.bits.conditionInformational == TRUE)
    {
        /*
         * Generate an informational event if one had not been generated previously.
         */
        if (previousEventPropertiesPtr->flags.bits.conditionInformational == FALSE ||
            previousEventPropertiesPtr->flags.bits.conditionWarning == TRUE ||
            previousEventPropertiesPtr->flags.bits.conditionError == TRUE ||
            currentEventPropertiesPtr->flags.bits.conditionChangeInformational == TRUE ||
            eventStringChange == TRUE)
        {
            dprintf(DPRINTF_I2C_DEVICE_STATUS, "  Alert-%s: INFORMATIONAL\n",
                    analyzeDebugString1);

            if (isAsyncEventAllowed == ALLOW_ASYNC_EVENT)
            {
                HWM_GenerateAlert(LOG_HARDWARE_MONITOR_INFO, currentEventPropertiesPtr, statusSize);
            }
            else if (isAsyncEventAllowed == DEBUG_ASYNC_EVENT)
            {
                HWM_GenerateAlert(LOG_HARDWARE_MONITOR_DEBUG, currentEventPropertiesPtr, statusSize);
            }

            returnCode = TRUE;
        }
        else
        {
            dprintf(DPRINTF_I2C_DEVICE_STATUS, "  Alert-%s: CONTINUED INFORMATIONAL\n",
                    analyzeDebugString1);
        }
    }
    else if (currentEventPropertiesPtr->flags.bits.conditionChangeInformational == TRUE)
    {
        /* Generate an informational event for the status change. */

        dprintf(DPRINTF_I2C_DEVICE_STATUS, "  Alert-%s: STATUS CHANGE\n",
                analyzeDebugString1);

        if (isAsyncEventAllowed == ALLOW_ASYNC_EVENT)
        {
            HWM_GenerateAlert(LOG_HARDWARE_MONITOR_INFO, currentEventPropertiesPtr, statusSize);
        }
        else if (isAsyncEventAllowed == DEBUG_ASYNC_EVENT)
        {
            HWM_GenerateAlert(LOG_HARDWARE_MONITOR_DEBUG, currentEventPropertiesPtr, statusSize);
        }

        returnCode = TRUE;
    }

    /* Generate a debug event if one has not already been generated */

    if (returnCode != TRUE &&
        (isAsyncEventAllowed == ALLOW_ASYNC_EVENT ||
         isAsyncEventAllowed == DEBUG_ASYNC_EVENT) &&
        (currentEventPropertiesPtr->flags.bits.conditionDebug == TRUE ||
         currentEventPropertiesPtr->flags.bits.conditionChangeDebug == TRUE))
    {
        dprintf(DPRINTF_I2C_DEVICE_STATUS, "  Alert-%s: DEBUG\n", analyzeDebugString1);

        HWM_GenerateAlert(LOG_HARDWARE_MONITOR_DEBUG, currentEventPropertiesPtr, statusSize);

        return TRUE;
    }

    return returnCode;
}


/*----------------------------------------------------------------------------
**  Function Name: HWM_GenerateAlert
**
**  Comments:   Generates and sends an alert message.
**
**  Parameters: alertType
**              eventPropertiesPtr
**              statusSize
**
**  Returns:    Nothing
**--------------------------------------------------------------------------*/
static void HWM_GenerateAlert(UINT32 alertType,
                              HWM_EVENT_PROPERTIES *eventPropertiesPtr, UINT32 statusSize)
{
    dprintf(DPRINTF_HWM_FUNCTIONS, "HWM: GenerateAlert\n");

    if (!eventPropertiesPtr)
    {
        dprintf(DPRINTF_HWM, "%s: eventPropertiesPtr is NULL\n", __func__);
        return;
    }

    /*
     * Override the alertType for the field-replaceable-units, such that
     * each FRU has its own alert code.
     */
    switch (eventPropertiesPtr->componentID)
    {
        case CCB_PROCESSOR_ID:
            /* CCB i960 I2C hardware or bus problem */
            switch (alertType)
            {
                case LOG_HARDWARE_MONITOR_INFO:
                    alertType = LOG_CCB_PROCESSOR_HW_INFO;
                    break;

                case LOG_HARDWARE_MONITOR_WARN:
                    alertType = LOG_CCB_PROCESSOR_HW_WARN;
                    break;

                case LOG_HARDWARE_MONITOR_ERROR:
                    alertType = LOG_CCB_PROCESSOR_HW_ERROR;
                    break;

                default:
                    alertType = LOG_HARDWARE_MONITOR_DEBUG;
            }
            break;

        case CCB_MEMORY_MODULE_ID:
            /* CCB memory module problem */
            switch (alertType)
            {
                case LOG_HARDWARE_MONITOR_INFO:
                    alertType = LOG_CCB_MEMORY_MODULE_HW_INFO;
                    break;

                case LOG_HARDWARE_MONITOR_WARN:
                    alertType = LOG_CCB_MEMORY_MODULE_HW_WARN;
                    break;

                case LOG_HARDWARE_MONITOR_ERROR:
                    alertType = LOG_CCB_MEMORY_MODULE_HW_ERROR;
                    break;

                default:
                    alertType = LOG_HARDWARE_MONITOR_DEBUG;
            }
            break;

        case CCB_MEMORY_MODULE_EEPROM_ID:
        case CCB_BOARD_EEPROM_ID:
//    case PROC_BOARD_LM80_ID:
//    case PROC_BOARD_LM87_ID:
//    case PROC_BOARD_LM80_LM87_ID:
//    case PROC_BOARD_LM75_ID:
//    case PROC_BOARD_LM92_ID:
//    case PROC_BOARD_LM75_LM92_ID:
//    case RESET_CONTROL_PCF8574_ID:
//    case POWER_SUPPLY_PCF8574_ID:
//    case CHASSIS_EEPROM_ID:
//    case PROC_BOARD_EEPROM_ID:
//    case FE_POWER_SUPPLY_ASSEMBLY_EEPROM_ID:
//    case FE_POWER_SUPPLY_INTERFACE_EEPROM_ID:
//    case BE_POWER_SUPPLY_ASSEMBLY_EEPROM_ID:
//    case BE_POWER_SUPPLY_INTERFACE_EEPROM_ID:
//    case FE_BUFFER_BOARD_LM80_ID:
//    case FE_BUFFER_BOARD_LM87_ID:
//    case FE_BUFFER_BOARD_LM80_LM87_ID:
//    case FE_BUFFER_BOARD_PCF8574_ID:
//    case FE_BUFFER_BOARD_MAX1660_ID:
//    case FE_BUFFER_BOARD_EEPROM_ID:
//    case BE_BUFFER_BOARD_LM80_ID:
//    case BE_BUFFER_BOARD_LM87_ID:
//    case BE_BUFFER_BOARD_LM80_LM87_ID:
//    case BE_BUFFER_BOARD_PCF8574_ID:
//    case BE_BUFFER_BOARD_MAX1660_ID:
//    case BE_BUFFER_BOARD_EEPROM_ID:
        case CHASSIS_EEPROM_STATUS_ID:
        case PROC_BOARD_EEPROM_STATUS_ID:
        case FE_BUFFER_BOARD_EEPROM_STATUS_ID:
        case BE_BUFFER_BOARD_EEPROM_STATUS_ID:
        case FE_POWER_SUPPLY_ASM_EEPROM_STATUS_ID:
        case FE_POWER_SUPPLY_INT_EEPROM_STATUS_ID:
        case BE_POWER_SUPPLY_ASM_EEPROM_STATUS_ID:
        case BE_POWER_SUPPLY_INT_EEPROM_STATUS_ID:
        case CCB_BOARD_EEPROM_STATUS_ID:
        case CCB_MEMORY_MODULE_EEPROM_STATUS_ID:
            alertType = LOG_HARDWARE_MONITOR_DEBUG;
            break;

        case PROC_BOARD_STATUS_ID:
        case POWER_SUPPLY_VOLTAGES_STATUS_ID:
        case FE_PROCESSOR_STATUS_ID:
        case BE_PROCESSOR_STATUS_ID:
            /* Processor board status problem */
            switch (alertType)
            {
                case LOG_HARDWARE_MONITOR_INFO:
                    alertType = LOG_PROC_BOARD_STATUS_INFO;
                    break;

                case LOG_HARDWARE_MONITOR_WARN:
                    alertType = LOG_PROC_BOARD_STATUS_WARN;
                    break;

                case LOG_HARDWARE_MONITOR_ERROR:
                    alertType = LOG_PROC_BOARD_STATUS_ERROR;
                    break;

                default:
                    alertType = LOG_HARDWARE_MONITOR_DEBUG;
            }
            break;

        case FE_BUFFER_BOARD_STATUS_ID:
        case FE_BUFFER_BOARD_TEMPERATURE_STATUS_ID:
        case FE_BUFFER_BOARD_BATTERY_STATUS_ID:
        case FE_BUFFER_BOARD_FUEL_GAUGE_STATUS_ID:
        case FE_BUFFER_BOARD_MAIN_REGULATOR_STATUS_ID:
        case FE_BUFFER_BOARD_CHARGER_STATUS_ID:
            /* Front-end buffer board status problem */
            switch (alertType)
            {
                case LOG_HARDWARE_MONITOR_INFO:
                    alertType = LOG_FE_BUFFER_BOARD_STATUS_INFO;
                    break;

                case LOG_HARDWARE_MONITOR_WARN:
                    alertType = LOG_FE_BUFFER_BOARD_STATUS_WARN;
                    break;

                case LOG_HARDWARE_MONITOR_ERROR:
                    alertType = LOG_FE_BUFFER_BOARD_STATUS_ERROR;
                    break;

                default:
                    alertType = LOG_HARDWARE_MONITOR_DEBUG;
            }
            break;

        case BE_BUFFER_BOARD_STATUS_ID:
        case BE_BUFFER_BOARD_TEMPERATURE_STATUS_ID:
        case BE_BUFFER_BOARD_BATTERY_STATUS_ID:
        case BE_BUFFER_BOARD_FUEL_GAUGE_STATUS_ID:
        case BE_BUFFER_BOARD_MAIN_REGULATOR_STATUS_ID:
        case BE_BUFFER_BOARD_CHARGER_STATUS_ID:
            /* Back-end buffer board status problem */
            switch (alertType)
            {
                case LOG_HARDWARE_MONITOR_INFO:
                    alertType = LOG_BE_BUFFER_BOARD_STATUS_INFO;
                    break;

                case LOG_HARDWARE_MONITOR_WARN:
                    alertType = LOG_BE_BUFFER_BOARD_STATUS_WARN;
                    break;

                case LOG_HARDWARE_MONITOR_ERROR:
                    alertType = LOG_BE_BUFFER_BOARD_STATUS_ERROR;
                    break;

                default:
                    alertType = LOG_HARDWARE_MONITOR_DEBUG;
            }
            break;

        case FE_POWER_SUPPLY_STATUS_ID:
            /* Front-end power supply status problem */
            switch (alertType)
            {
                case LOG_HARDWARE_MONITOR_INFO:
                    alertType = LOG_FE_POWER_SUPPLY_STATUS_INFO;
                    break;

                case LOG_HARDWARE_MONITOR_WARN:
                    alertType = LOG_FE_POWER_SUPPLY_STATUS_WARN;
                    break;

                case LOG_HARDWARE_MONITOR_ERROR:
                    alertType = LOG_FE_POWER_SUPPLY_STATUS_ERROR;
                    break;

                default:
                    alertType = LOG_HARDWARE_MONITOR_DEBUG;
            }
            break;

        case BE_POWER_SUPPLY_STATUS_ID:
            /* Back-end power supply status problem */
            switch (alertType)
            {
                case LOG_HARDWARE_MONITOR_INFO:
                    alertType = LOG_BE_POWER_SUPPLY_STATUS_INFO;
                    break;

                case LOG_HARDWARE_MONITOR_WARN:
                    alertType = LOG_BE_POWER_SUPPLY_STATUS_WARN;
                    break;

                case LOG_HARDWARE_MONITOR_ERROR:
                    alertType = LOG_BE_POWER_SUPPLY_STATUS_ERROR;
                    break;

                default:
                    alertType = LOG_HARDWARE_MONITOR_DEBUG;
            }
            break;

        case CCB_BOARD_STATUS_ID:
        case CCB_NVRAM_BATTERY_STATUS_ID:
            switch (alertType)
            {
                case LOG_HARDWARE_MONITOR_INFO:
                    alertType = LOG_CCB_STATUS_INFO;
                    break;

                case LOG_HARDWARE_MONITOR_WARN:
                    alertType = LOG_CCB_STATUS_WARN;
                    break;

                case LOG_HARDWARE_MONITOR_ERROR:
                    alertType = LOG_CCB_STATUS_ERROR;
                    break;

                default:
                    alertType = LOG_HARDWARE_MONITOR_DEBUG;
            }
            break;

        case HARDWARE_MONITOR_STATUS_ID:
            /* CCB hardware monitor status problem */
            switch (alertType)
            {
                case LOG_HARDWARE_MONITOR_INFO:
                    alertType = LOG_HARDWARE_MONITOR_STATUS_INFO;
                    break;

                case LOG_HARDWARE_MONITOR_WARN:
                    alertType = LOG_HARDWARE_MONITOR_STATUS_WARN;
                    break;

                case LOG_HARDWARE_MONITOR_ERROR:
                    alertType = LOG_HARDWARE_MONITOR_STATUS_ERROR;
                    break;

                default:
                    alertType = LOG_HARDWARE_MONITOR_DEBUG;
            }
            break;

        default:
            dprintf(DPRINTF_HWM, "GA: Unknown componentID: %d\n",
                    eventPropertiesPtr->componentID);
            break;
    }

    dprintf(DPRINTF_HWM, "GA: alertType: 0x%08x   statusPtr: %p   (%d bytes)\n",
            alertType, eventPropertiesPtr, statusSize);

    SendAsyncEvent(alertType, statusSize, eventPropertiesPtr);
    TaskSwitch();
}


/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
