/* $Id: HWM_StatusStrings.c 143007 2010-06-22 14:48:58Z m4 $ */
/*============================================================================
** FILE NAME:       HWM_StatusStrings.c
** MODULE TITLE:    Hardware monitor string implementation
**
** DESCRIPTION:     Hardware monitor string building functions
**
** Copyright (c) 2003-2009 Xiotech Corporation. All rights reserved.
**==========================================================================*/
#include "HWM_StatusStrings.h"

#ifdef LOG_SIMULATOR
#include "LogSimFuncs.h"
#else   /* LOG_SIMULATOR */
#include "convert.h"
#include "HWM_ConditionStrings.h"
#include "logging.h"
#include "XIO_Std.h"
#include "XIO_Const.h"
#endif  /* LOG_SIMULATOR */

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
**  Function Name: HWM_GetEventSeverityString
**
**  Parameters:    eventPropertiesPtr
**--------------------------------------------------------------------------*/
static UINT32 HWM_GetEventSeverityString(char *stringPtr,
                                         HWM_EVENT_PROPERTIES *eventPropertiesPtr,
                                         UINT8 stringLength)
{
    UINT32      stringSeverity = EVENT_STRING_SEVERITY_NONE;

    if (!stringPtr)
    {
        return EVENT_STRING_SEVERITY_NONE;
    }

    if (eventPropertiesPtr != NULL)
    {
        /* Find the event severity */

        if (eventPropertiesPtr->flags.bits.conditionError == TRUE)
        {
            strncpy(stringPtr, "ERROR", stringLength);
            stringSeverity = EVENT_STRING_SEVERITY_ERROR;
        }
        else if (eventPropertiesPtr->flags.bits.conditionWarning == TRUE)
        {
            strncpy(stringPtr, "ALERT", stringLength);
            stringSeverity = EVENT_STRING_SEVERITY_WARNING;
        }
        else if (eventPropertiesPtr->flags.bits.conditionInformational == TRUE ||
                 eventPropertiesPtr->flags.bits.conditionChangeInformational == TRUE)
        {
            strncpy(stringPtr, "OK", stringLength);
            stringSeverity = EVENT_STRING_SEVERITY_INFO;
        }
        else if (eventPropertiesPtr->flags.bits.conditionDebug == TRUE ||
                 eventPropertiesPtr->flags.bits.conditionChangeDebug == TRUE)
        {
            strncpy(stringPtr, "DEBUG", stringLength);
            stringSeverity = EVENT_STRING_SEVERITY_DEBUG;
        }
        else
        {
            strncpy(stringPtr, "NO_EVENT", stringLength);
        }
    }
    else
    {
        strncpy(stringPtr, "Bad Event", stringLength);
    }

    /* Make sure the strncpy is terminated */

    if (stringLength > 0)
    {
        stringPtr[stringLength - 1] = '\0';
    }

    return stringSeverity;
}


/*----------------------------------------------------------------------------
**  Function Name: HWM_GetMonitorStatusString
**
**  Parameters:    monitorStatusPtr
**
**--------------------------------------------------------------------------*/
UINT32 HWM_GetMonitorStatusString(char *stringPtr, HWM_STATUS *monitorStatusPtr,
                                  UINT8 stringLength)
{
    UINT32      stringSeverity = EVENT_STRING_SEVERITY_NONE;
    UINT32      tempStringSeverity = EVENT_STRING_SEVERITY_NONE;
    char        tempEventString[MMC_MESSAGE_SIZE] = { 0 };
    char        componentIDString[MMC_MESSAGE_SIZE] = { 0 };

    /*
     * Things pertaining to condition of the object
     *   CCB_STATUS ccbStatus;
     *   PROC_BOARD_STATUS procBoardStatus;
     *   POWER_SUPPLY_STATUS frontEndPowerSupply;
     *   POWER_SUPPLY_STATUS backEndPowerSupply;
     *   BUFFER_BOARD_STATUS frontEndBufferBoardStatus;
     *   BUFFER_BOARD_STATUS backEndBufferBoardStatus;
     */
    if (!stringPtr)
    {
        return EVENT_STRING_SEVERITY_NONE;
    }

    if (monitorStatusPtr != NULL)
    {
        /*
         * Find the string severity and create the string
         */
        /* Check the CCB status */
        tempStringSeverity = HWM_GetCCBStatusString(tempEventString,
                                                    &monitorStatusPtr->ccbStatus,
                                                    sizeof(tempEventString));

        if (tempStringSeverity > stringSeverity)
        {
            stringSeverity = tempStringSeverity;

            HWM_GetComponentIDString(componentIDString,
                                     monitorStatusPtr->ccbStatus.eventProperties.componentID,
                                     sizeof(componentIDString));

            strncpy(stringPtr, componentIDString, stringLength);

            strncpy(stringPtr + strlen(stringPtr), " ", stringLength - strlen(stringPtr));

            strncpy(stringPtr + strlen(stringPtr),
                    tempEventString, stringLength - strlen(stringPtr));
        }

        /* Check the processor board status */
        tempStringSeverity = HWM_GetProcessorBoardStatusString(tempEventString,
                                                               &monitorStatusPtr->procBoardStatus,
                                                               sizeof(tempEventString));

        if (tempStringSeverity > stringSeverity)
        {
            stringSeverity = tempStringSeverity;

            HWM_GetComponentIDString(componentIDString,
                                     monitorStatusPtr->procBoardStatus.eventProperties.componentID,
                                     sizeof(componentIDString));

            strncpy(stringPtr, componentIDString, stringLength);

            strncpy(stringPtr + strlen(stringPtr), " ", stringLength - strlen(stringPtr));

            strncpy(stringPtr + strlen(stringPtr),
                    tempEventString, stringLength - strlen(stringPtr));
        }

        /* Check the front end power supply status */
        tempStringSeverity = HWM_GetPowerSupplyStatusString(tempEventString,
                                                            &monitorStatusPtr->frontEndPowerSupply,
                                                            sizeof(tempEventString));

        if (tempStringSeverity > stringSeverity)
        {
            stringSeverity = tempStringSeverity;

            HWM_GetComponentIDString(componentIDString,
                                     monitorStatusPtr->frontEndPowerSupply.eventProperties.componentID,
                                     sizeof(componentIDString));

            strncpy(stringPtr, componentIDString, stringLength);

            strncpy(stringPtr + strlen(stringPtr), " ", stringLength - strlen(stringPtr));

            strncpy(stringPtr + strlen(stringPtr),
                    tempEventString, stringLength - strlen(stringPtr));
        }

        /* Check the back end power supply status */
        tempStringSeverity = HWM_GetPowerSupplyStatusString(tempEventString,
                                                            &monitorStatusPtr->backEndPowerSupply,
                                                            sizeof(tempEventString));

        if (tempStringSeverity > stringSeverity)
        {
            stringSeverity = tempStringSeverity;

            HWM_GetComponentIDString(componentIDString,
                                     monitorStatusPtr->backEndPowerSupply.eventProperties.componentID,
                                     sizeof(componentIDString));

            strncpy(stringPtr, componentIDString, stringLength);

            strncpy(stringPtr + strlen(stringPtr), " ", stringLength - strlen(stringPtr));

            strncpy(stringPtr + strlen(stringPtr),
                    tempEventString, stringLength - strlen(stringPtr));
        }

        /* Check the front end buffer board status */
        tempStringSeverity = HWM_GetBufferBoardStatusString(tempEventString,
                                                            &monitorStatusPtr->frontEndBufferBoardStatus,
                                                            sizeof(tempEventString));

        if (tempStringSeverity > stringSeverity)
        {
            stringSeverity = tempStringSeverity;

            HWM_GetComponentIDString(componentIDString,
                                     monitorStatusPtr->frontEndBufferBoardStatus.eventProperties.componentID,
                                     sizeof(componentIDString));

            strncpy(stringPtr, componentIDString, stringLength);

            strncpy(stringPtr + strlen(stringPtr), " ", stringLength - strlen(stringPtr));

            strncpy(stringPtr + strlen(stringPtr),
                    tempEventString, stringLength - strlen(stringPtr));
        }

        /* Check the back end buffer board status */
        tempStringSeverity = HWM_GetBufferBoardStatusString(tempEventString,
                                                            &monitorStatusPtr->backEndBufferBoardStatus,
                                                            sizeof(tempEventString));

        if (tempStringSeverity > stringSeverity)
        {
            stringSeverity = tempStringSeverity;

            HWM_GetComponentIDString(componentIDString,
                                     monitorStatusPtr->backEndBufferBoardStatus.eventProperties.componentID,
                                     sizeof(componentIDString));

            strncpy(stringPtr, componentIDString, stringLength);

            strncpy(stringPtr + strlen(stringPtr), " ", stringLength - strlen(stringPtr));

            strncpy(stringPtr + strlen(stringPtr),
                    tempEventString, stringLength - strlen(stringPtr));
        }

        /* Check the monitor overall status */
        tempStringSeverity = HWM_GetEventSeverityString(tempEventString,
                                                        &monitorStatusPtr->eventProperties,
                                                        sizeof(tempEventString));

        if ((tempStringSeverity > stringSeverity) ||
            (stringSeverity == EVENT_STRING_SEVERITY_NONE) ||
            (stringSeverity == EVENT_STRING_SEVERITY_NORMAL))
        {
            stringSeverity = tempStringSeverity;
            strncpy(stringPtr, tempEventString, stringLength);
        }
    }
    else
    {
        strncpy(stringPtr, "Bad Event", stringLength);
    }

    /*
     * Make sure the strncpy is terminated
     */
    if (stringLength > 0)
    {
        stringPtr[stringLength - 1] = '\0';
    }

    return (stringSeverity);
}


/*----------------------------------------------------------------------------
**  Function Name: HWM_GetNVRAMBatteryStatusString
**
**  Parameters:    nvramBatteryStatusPtr
**--------------------------------------------------------------------------*/
UINT32 HWM_GetNVRAMBatteryStatusString(char *stringPtr,
                                       NVRAM_BATTERY_STATUS_PTR nvramBatteryStatusPtr,
                                       UINT8 stringLength)
{
    UINT32      stringSeverity = EVENT_STRING_SEVERITY_NONE;
    UINT32      tempStringSeverity = EVENT_STRING_SEVERITY_NONE;
    char        tempEventString[MMC_MESSAGE_SIZE] = { 0 };

    if (stringPtr != NULL)
    {
        if (nvramBatteryStatusPtr != NULL)
        {
            /*
             * Find the string severity and create the string
             */
            /* Check the NVRAM battery condition */
            tempStringSeverity = HWM_GetNVRAMBatteryConditionString(tempEventString,
                                                                    nvramBatteryStatusPtr->nvramBatteryCondition,
                                                                    sizeof(tempEventString));

            if (tempStringSeverity > stringSeverity)
            {
                stringSeverity = tempStringSeverity;
                strncpy(stringPtr, tempEventString, stringLength);
            }

            /* Check the NVRAM battery status */
            tempStringSeverity = HWM_GetEventSeverityString(tempEventString,
                                                            &nvramBatteryStatusPtr->eventProperties,
                                                            sizeof(tempEventString));

            if ((tempStringSeverity > stringSeverity) ||
                (stringSeverity == EVENT_STRING_SEVERITY_NONE) ||
                (stringSeverity == EVENT_STRING_SEVERITY_NORMAL))
            {
                stringSeverity = tempStringSeverity;
                strncpy(stringPtr, tempEventString, stringLength);
            }
        }
        else
        {
            strncpy(stringPtr, "Bad Event", stringLength);
        }

        /*
         * Make sure the strncpy is terminated
         */
        if (stringLength > 0)
        {
            stringPtr[stringLength - 1] = '\0';
        }
    }

    return (stringSeverity);
}


/*----------------------------------------------------------------------------
**  Function Name: HWM_GetTemperatureStatusString
**
**  Parameters:    temperatureStatusPtr
**--------------------------------------------------------------------------*/
UINT32 HWM_GetTemperatureStatusString(char *stringPtr,
                                      TEMPERATURE_STATUS_PTR temperatureStatusPtr,
                                      UINT8 stringLength)
{
    UINT32      stringSeverity = EVENT_STRING_SEVERITY_NONE;
    UINT32      tempStringSeverity = EVENT_STRING_SEVERITY_NONE;
    char        tempEventString[MMC_MESSAGE_SIZE] = { 0 };
    char        printEventString[MMC_MESSAGE_SIZE] = { 0 };

    if (stringPtr != NULL)
    {
        if (temperatureStatusPtr != NULL)
        {
            /* Check the temperature condition */
            tempStringSeverity = HWM_GetTemperatureConditionString(tempEventString,
                                                                   temperatureStatusPtr->conditionValue,
                                                                   sizeof(tempEventString));

#ifdef I2C_MAG_STYLE_TEMP_EVENTS
            /*
             * If the severity is ERROR, WARNING, or NORMAL, override
             * the generated string.
             */
            switch (tempStringSeverity)
            {
                case EVENT_STRING_SEVERITY_NONE:
                case EVENT_STRING_SEVERITY_NORMAL:
                case EVENT_STRING_SEVERITY_INFO:
                    strncpy(tempEventString, "IS OK NOW", sizeof(tempEventString));
                    break;

                case EVENT_STRING_SEVERITY_WARNING:
                    strncpy(tempEventString, "WARNING", sizeof(tempEventString));
                    break;

                case EVENT_STRING_SEVERITY_ERROR:
                    strncpy(tempEventString, "ERROR", sizeof(tempEventString));
                    break;

                case EVENT_STRING_SEVERITY_DEBUG:
                default:
                    break;
            }

            if (tempStringSeverity > stringSeverity)
            {
                stringSeverity = tempStringSeverity;

                sprintf(printEventString,
                        "%d dF (%s)",
                        C_to_F(temperatureStatusPtr->currentDegreesCelsius),
                        tempEventString);
                strncpy(stringPtr, printEventString, stringLength);
            }
#else   /* I2C_MAG_STYLE_TEMP_EVENTS */
            /*
             * Find the string severity and create the string
             */
            if (tempStringSeverity > stringSeverity)
            {
                stringSeverity = tempStringSeverity;
                strncpy(stringPtr, tempEventString, stringLength);
            }

            /* Check the overall temperature status */
            tempStringSeverity = HWM_GetEventSeverityString(tempEventString,
                                                            &temperatureStatusPtr->eventProperties,
                                                            sizeof(tempEventString));

            if ((tempStringSeverity > stringSeverity) ||
                (stringSeverity == EVENT_STRING_SEVERITY_NONE) ||
                (stringSeverity == EVENT_STRING_SEVERITY_NORMAL))
            {
                stringSeverity = tempStringSeverity;
                strncpy(stringPtr, tempEventString, stringLength);
            }
#endif  /* I2C_MAG_STYLE_TEMP_EVENTS */
        }
        else
        {
            strncpy(stringPtr, "Bad Event", stringLength);
        }

        /*
         * Make sure the strncpy is terminated
         */
        if (stringLength > 0)
        {
            stringPtr[stringLength - 1] = '\0';
        }
    }

    return (stringSeverity);
}


/*----------------------------------------------------------------------------
**  Function Name: HWM_GetCCBStatusString
**
**  Parameters:    ccbStatusPtr
**--------------------------------------------------------------------------*/
UINT32 HWM_GetCCBStatusString(char *stringPtr, CCB_STATUS_PTR ccbStatusPtr,
                              UINT8 stringLength)
{
    UINT32      stringSeverity = EVENT_STRING_SEVERITY_NONE;
    UINT32      tempStringSeverity = EVENT_STRING_SEVERITY_NONE;
    char        tempEventString[MMC_MESSAGE_SIZE] = { 0 };

    if (stringPtr != NULL)
    {
        if (ccbStatusPtr != NULL)
        {
            /*
             * Find the string severity and create the string
             */
            /* Check the NVRAM battery status */
            tempStringSeverity = HWM_GetNVRAMBatteryStatusString(tempEventString,
                                                                 &ccbStatusPtr->nvramBatteryStatus,
                                                                 sizeof(tempEventString));

            if (tempStringSeverity > stringSeverity)
            {
                stringSeverity = tempStringSeverity;
                strncpy(stringPtr, "NVRAM Battery ", stringLength);
                strncpy(stringPtr + strlen(stringPtr),
                        tempEventString, stringLength - strlen(stringPtr));
            }

            /* Check the CCB overall status */
            tempStringSeverity = HWM_GetEventSeverityString(tempEventString,
                                                            &ccbStatusPtr->eventProperties,
                                                            sizeof(tempEventString));

            if ((tempStringSeverity > stringSeverity) ||
                (stringSeverity == EVENT_STRING_SEVERITY_NONE) ||
                (stringSeverity == EVENT_STRING_SEVERITY_NORMAL))
            {
                stringSeverity = tempStringSeverity;
                strncpy(stringPtr, tempEventString, stringLength);
            }
        }
        else
        {
            strncpy(stringPtr, "Bad Event", stringLength);
        }

        /*
         * Make sure the strncpy is terminated
         */
        if (stringLength > 0)
        {
            stringPtr[stringLength - 1] = '\0';
        }
    }

    return (stringSeverity);
}


/*----------------------------------------------------------------------------
**  Function Name: HWM_GetProcessorBoardStatusString
**
**  Parameters:    procBoardStatusPtr
**--------------------------------------------------------------------------*/
UINT32 HWM_GetProcessorBoardStatusString(char *stringPtr,
                                         PROC_BOARD_STATUS_PTR procBoardStatusPtr,
                                         UINT8 stringLength)
{
    UINT32      stringSeverity = EVENT_STRING_SEVERITY_NONE;
    UINT32      tempStringSeverity = EVENT_STRING_SEVERITY_NONE;
    char        tempEventString[MMC_MESSAGE_SIZE] = { 0 };

    if (stringPtr != NULL)
    {
        if (procBoardStatusPtr != NULL)
        {
            /*
             * Find the string severity and create the string
             */
            /* Check the power supply voltages status */
            tempStringSeverity = HWM_GetPowerSupplyVoltagesStatusString(tempEventString,
                                                                        &procBoardStatusPtr->powerSupplyVoltagesStatus,
                                                                        sizeof(tempEventString));

            if (tempStringSeverity > stringSeverity)
            {
                stringSeverity = tempStringSeverity;
                strncpy(stringPtr, "Voltages ", stringLength);
                strncpy(stringPtr + strlen(stringPtr),
                        tempEventString, stringLength - strlen(stringPtr));
            }

            /* Check the FE processor status */
            tempStringSeverity = HWM_GetProcessorStatusString(tempEventString,
                                                              &procBoardStatusPtr->frontEndProcessorStatus,
                                                              sizeof(tempEventString));

            if (tempStringSeverity > stringSeverity)
            {
                stringSeverity = tempStringSeverity;
                strncpy(stringPtr, "ProcB ", stringLength);
                strncpy(stringPtr + strlen(stringPtr),
                        tempEventString, stringLength - strlen(stringPtr));
            }

            /* Check the BE processor status */
            tempStringSeverity = HWM_GetProcessorStatusString(tempEventString,
                                                              &procBoardStatusPtr->backEndProcessorStatus,
                                                              sizeof(tempEventString));

            if (tempStringSeverity > stringSeverity)
            {
                stringSeverity = tempStringSeverity;
                strncpy(stringPtr, "ProcA ", stringLength);
                strncpy(stringPtr + strlen(stringPtr),
                        tempEventString, stringLength - strlen(stringPtr));
            }

            /* Check the processor board overall status */
            tempStringSeverity = HWM_GetEventSeverityString(tempEventString,
                                                            &procBoardStatusPtr->eventProperties,
                                                            sizeof(tempEventString));

            if ((tempStringSeverity > stringSeverity) ||
                (stringSeverity == EVENT_STRING_SEVERITY_NONE) ||
                (stringSeverity == EVENT_STRING_SEVERITY_NORMAL))
            {
                stringSeverity = tempStringSeverity;
                strncpy(stringPtr, tempEventString, stringLength);
            }
        }
        else
        {
            strncpy(stringPtr, "Bad Event", stringLength);
        }

        /*
         * Make sure the strncpy is terminated
         */
        if (stringLength > 0)
        {
            stringPtr[stringLength - 1] = '\0';
        }
    }

    return (stringSeverity);
}


/*----------------------------------------------------------------------------
**  Function Name: HWM_GetPowerSupplyVoltagesStatusString
**
**  Parameters:    powerSupplyVoltagesStatusPtr
**--------------------------------------------------------------------------*/
UINT32 HWM_GetPowerSupplyVoltagesStatusString(char *stringPtr,
                                              POWER_SUPPLY_VOLTAGES_STATUS_PTR
                                              powerSupplyVoltagesStatusPtr,
                                              UINT8 stringLength)
{
    UINT32      stringSeverity = EVENT_STRING_SEVERITY_NONE;
    UINT32      tempStringSeverity = EVENT_STRING_SEVERITY_NONE;
    char        tempEventString[MMC_MESSAGE_SIZE] = { 0 };

    if (stringPtr != NULL)
    {
        if (powerSupplyVoltagesStatusPtr != NULL)
        {
            /*
             * Find the string severity and create the string
             */
            /* Check the 3.3 volt supply voltage condition */
            tempStringSeverity = HWM_GetLimitMonitorConditionString(tempEventString,
                                                                    powerSupplyVoltagesStatusPtr->threePointThreeVoltReading.limitMonitorValue,
                                                                    sizeof(tempEventString));

            if (tempStringSeverity > stringSeverity)
            {
                stringSeverity = tempStringSeverity;
                strncpy(stringPtr, "3.3V ", stringLength);
                strncpy(stringPtr + strlen(stringPtr),
                        tempEventString, stringLength - strlen(stringPtr));
            }

            /* Check the 5 volt supply voltage condition */
            tempStringSeverity = HWM_GetLimitMonitorConditionString(tempEventString,
                                                                    powerSupplyVoltagesStatusPtr->fiveVoltReading.limitMonitorValue,
                                                                    sizeof(tempEventString));

            if (tempStringSeverity > stringSeverity)
            {
                stringSeverity = tempStringSeverity;
                strncpy(stringPtr, "5V ", stringLength);
                strncpy(stringPtr + strlen(stringPtr),
                        tempEventString, stringLength - strlen(stringPtr));
            }

            /* Check the 12 volt supply voltage condition */
            tempStringSeverity = HWM_GetLimitMonitorConditionString(tempEventString,
                                                                    powerSupplyVoltagesStatusPtr->twelveVoltReading.limitMonitorValue,
                                                                    sizeof(tempEventString));

            if (tempStringSeverity > stringSeverity)
            {
                stringSeverity = tempStringSeverity;
                strncpy(stringPtr, "12V ", stringLength);
                strncpy(stringPtr + strlen(stringPtr),
                        tempEventString, stringLength - strlen(stringPtr));
            }

            /* Check the 5 volt standby supply voltage condition */
            tempStringSeverity = HWM_GetLimitMonitorConditionString(tempEventString,
                                                                    powerSupplyVoltagesStatusPtr->standbyVoltageReading.limitMonitorValue,
                                                                    sizeof(tempEventString));

            if (tempStringSeverity > stringSeverity)
            {
                stringSeverity = tempStringSeverity;
                strncpy(stringPtr, "SBV ", stringLength);
                strncpy(stringPtr + strlen(stringPtr),
                        tempEventString, stringLength - strlen(stringPtr));
            }

            /* Check the power supply voltages overall status */
            tempStringSeverity = HWM_GetEventSeverityString(tempEventString,
                                                            &powerSupplyVoltagesStatusPtr->eventProperties,
                                                            sizeof(tempEventString));

            if ((tempStringSeverity > stringSeverity) ||
                (stringSeverity == EVENT_STRING_SEVERITY_NONE) ||
                (stringSeverity == EVENT_STRING_SEVERITY_NORMAL))
            {
                stringSeverity = tempStringSeverity;
                strncpy(stringPtr, tempEventString, stringLength);
            }
        }
        else
        {
            strncpy(stringPtr, "Bad Event", stringLength);
        }

        /*
         * Make sure the strncpy is terminated
         */
        if (stringLength > 0)
        {
            stringPtr[stringLength - 1] = '\0';
        }
    }

    return (stringSeverity);
}


/*----------------------------------------------------------------------------
**  Function Name: HWM_GetProcessorStatusString
**
**  Parameters:    processorStatusPtr
**--------------------------------------------------------------------------*/
UINT32 HWM_GetProcessorStatusString(char *stringPtr,
                                    PROC_BOARD_PROCESSOR_STATUS_PTR processorStatusPtr,
                                    UINT8 stringLength)
{
    UINT32      stringSeverity = EVENT_STRING_SEVERITY_NONE;
    UINT32      tempStringSeverity = EVENT_STRING_SEVERITY_NONE;
    char        tempEventString[MMC_MESSAGE_SIZE] = { 0 };

    if (stringPtr != NULL)
    {
        if (processorStatusPtr != NULL)
        {
            /*
             * Find the string severity and create the string
             */

#ifdef I2C_MAG_STYLE_TEMP_EVENTS
            /*
             * Suppress the temperature related messages for the FE and BE
             * processors from the processor events. They are sent out as
             * MAG style TEMP related messages.
             */
#else   /* I2C_MAG_STYLE_TEMP_EVENTS */
            /*
             * Check the temperature condition
             */
            tempStringSeverity = HWM_GetTemperatureConditionString(tempEventString,
                                                                   processorStatusPtr->temperatureStatus.conditionValue,
                                                                   sizeof(tempEventString));

            if (tempStringSeverity > stringSeverity)
            {
                stringSeverity = tempStringSeverity;
                strncpy(stringPtr, "Temp ", stringLength);
                strncpy(stringPtr + strlen(stringPtr),
                        tempEventString, stringLength - strlen(stringPtr));
            }
#endif  /* I2C_MAG_STYLE_TEMP_EVENTS */

            /* Check the memory socket voltage condition */
            tempStringSeverity = HWM_GetLimitMonitorConditionString(tempEventString,
                                                                    processorStatusPtr->memorySocketSupplyVoltageReading.limitMonitorValue,
                                                                    sizeof(tempEventString));

            if (tempStringSeverity > stringSeverity)
            {
                stringSeverity = tempStringSeverity;
                strncpy(stringPtr, "MemV ", stringLength);
                strncpy(stringPtr + strlen(stringPtr),
                        tempEventString, stringLength - strlen(stringPtr));
            }

            /* Check the reset condition */
            tempStringSeverity = HWM_GetProcessorResetConditionString(tempEventString,
                                                                      processorStatusPtr->processorResetConditionValue,
                                                                      sizeof(tempEventString));

            if (tempStringSeverity > stringSeverity)
            {
                stringSeverity = tempStringSeverity;
                strncpy(stringPtr, "Rst ", stringLength);
                strncpy(stringPtr + strlen(stringPtr),
                        tempEventString, stringLength - strlen(stringPtr));
            }

            /* Check processor overall status */
            tempStringSeverity = HWM_GetEventSeverityString(tempEventString,
                                                            &processorStatusPtr->eventProperties,
                                                            sizeof(tempEventString));

            if ((tempStringSeverity > stringSeverity) ||
                (stringSeverity == EVENT_STRING_SEVERITY_NONE) ||
                (stringSeverity == EVENT_STRING_SEVERITY_NORMAL))
            {
                stringSeverity = tempStringSeverity;
                strncpy(stringPtr, tempEventString, stringLength);
            }
        }
        else
        {
            strncpy(stringPtr, "Bad Event", stringLength);
        }

        /*
         * Make sure the strncpy is terminated
         */
        if (stringLength > 0)
        {
            stringPtr[stringLength - 1] = '\0';
        }
    }

    return (stringSeverity);
}


/*----------------------------------------------------------------------------
**  Function Name: HWM_GetPowerSupplyStatusString
**
**  Parameters:    powerSupplyStatusPtr
**--------------------------------------------------------------------------*/
UINT32 HWM_GetPowerSupplyStatusString(char *stringPtr,
                                      POWER_SUPPLY_STATUS_PTR powerSupplyStatusPtr,
                                      UINT8 stringLength)
{
    UINT32      stringSeverity = EVENT_STRING_SEVERITY_NONE;
    UINT32      tempStringSeverity = EVENT_STRING_SEVERITY_NONE;
    char        tempEventString[MMC_MESSAGE_SIZE] = { 0 };

    if (stringPtr != NULL)
    {
        if (powerSupplyStatusPtr != NULL)
        {
            /*
             * Find the string severity and create the string
             */
            /* Check the power supply condition */
            tempStringSeverity = HWM_GetPowerSupplyConditionString(tempEventString,
                                                                   powerSupplyStatusPtr->powerSupplyCondition.value,
                                                                   sizeof(tempEventString));

            if (tempStringSeverity > stringSeverity)
            {
                stringSeverity = tempStringSeverity;
                strncpy(stringPtr, "Supply ", stringLength);
                strncpy(stringPtr + strlen(stringPtr),
                        tempEventString, stringLength - strlen(stringPtr));
            }

            /* Check the cooling fan condition */
            tempStringSeverity = HWM_GetCoolingFanConditionString(tempEventString,
                                                                  powerSupplyStatusPtr->coolingFanConditionValue,
                                                                  sizeof(tempEventString));

            if (tempStringSeverity > stringSeverity)
            {
                stringSeverity = tempStringSeverity;
                strncpy(stringPtr, "Fan ", stringLength);
                strncpy(stringPtr + strlen(stringPtr),
                        tempEventString, stringLength - strlen(stringPtr));
            }

            /* Check the power supply overall status */
            tempStringSeverity = HWM_GetEventSeverityString(tempEventString,
                                                            &powerSupplyStatusPtr->eventProperties,
                                                            sizeof(tempEventString));

            if ((tempStringSeverity > stringSeverity) ||
                (stringSeverity == EVENT_STRING_SEVERITY_NONE) ||
                (stringSeverity == EVENT_STRING_SEVERITY_NORMAL))
            {
                stringSeverity = tempStringSeverity;
                strncpy(stringPtr, tempEventString, stringLength);
            }
        }
        else
        {
            strncpy(stringPtr, "Bad Event", stringLength);
        }

        /*
         * Make sure the strncpy is terminated
         */
        if (stringLength > 0)
        {
            stringPtr[stringLength - 1] = '\0';
        }
    }

    return (stringSeverity);
}


/*----------------------------------------------------------------------------
**  Function Name: HWM_GetBufferBoardStatusString
**
**  Parameters:    bufferBoardStatusPtr
**--------------------------------------------------------------------------*/
UINT32 HWM_GetBufferBoardStatusString(char *stringPtr,
                                      BUFFER_BOARD_STATUS_PTR bufferBoardStatusPtr,
                                      UINT8 stringLength)
{
    UINT32      stringSeverity = EVENT_STRING_SEVERITY_NONE;
    UINT32      tempStringSeverity = EVENT_STRING_SEVERITY_NONE;
    char        tempEventString[MMC_MESSAGE_SIZE] = { 0 };

    if (stringPtr != NULL)
    {
        if (bufferBoardStatusPtr != NULL)
        {
            /*
             * Find the string severity and create the string
             */

#ifdef I2C_MAG_STYLE_TEMP_EVENTS
            /*
             * Suppress the temperature related messages for the buffer boards.
             * They are sent out as MAG style TEMP related messages.
             */
#else   /* I2C_MAG_STYLE_TEMP_EVENTS */
            /* Check the temperature status */
            tempStringSeverity = HWM_GetTemperatureStatusString(tempEventString,
                                                                &bufferBoardStatusPtr->temperatureStatus,
                                                                sizeof(tempEventString));

            if (tempStringSeverity > stringSeverity)
            {
                stringSeverity = tempStringSeverity;
                strncpy(stringPtr, "Temp ", stringLength);
                strncpy(stringPtr + strlen(stringPtr),
                        tempEventString, stringLength - strlen(stringPtr));
            }
#endif  /* I2C_MAG_STYLE_TEMP_EVENTS */

            /* Check the battery status */
            tempStringSeverity = HWM_GetBatteryStatusString(tempEventString,
                                                            &bufferBoardStatusPtr->batteryStatus,
                                                            sizeof(tempEventString));

            if (tempStringSeverity > stringSeverity)
            {
                stringSeverity = tempStringSeverity;
                strncpy(stringPtr, "Battery ", stringLength);
                strncpy(stringPtr + strlen(stringPtr),
                        tempEventString, stringLength - strlen(stringPtr));
            }

            /* Check the charger status */
            tempStringSeverity = HWM_GetChargerStatusString(tempEventString,
                                                            &bufferBoardStatusPtr->chargerStatus,
                                                            sizeof(tempEventString));

            if (tempStringSeverity > stringSeverity)
            {
                stringSeverity = tempStringSeverity;
                strncpy(stringPtr, "Charger ", stringLength);
                strncpy(stringPtr + strlen(stringPtr),
                        tempEventString, stringLength - strlen(stringPtr));
            }

            /* Check the main regulator status */
            tempStringSeverity = HWM_GetMainRegulatorStatusString(tempEventString,
                                                                  &bufferBoardStatusPtr->mainRegulatorStatus,
                                                                  sizeof(tempEventString));

            if (tempStringSeverity > stringSeverity)
            {
                stringSeverity = tempStringSeverity;
                strncpy(stringPtr, "MainReg ", stringLength);
                strncpy(stringPtr + strlen(stringPtr),
                        tempEventString, stringLength - strlen(stringPtr));
            }

            /* Check the fuel gauge status */
            tempStringSeverity = HWM_GetFuelGaugeStatusString(tempEventString,
                                                              &bufferBoardStatusPtr->fuelGaugeStatus,
                                                              sizeof(tempEventString));

            if (tempStringSeverity > stringSeverity)
            {
                stringSeverity = tempStringSeverity;
                strncpy(stringPtr, "FuelGg ", stringLength);
                strncpy(stringPtr + strlen(stringPtr),
                        tempEventString, stringLength - strlen(stringPtr));
            }

            /* Check the buffer board overall status */
            tempStringSeverity = HWM_GetEventSeverityString(tempEventString,
                                                            &bufferBoardStatusPtr->eventProperties,
                                                            sizeof(tempEventString));

            if ((tempStringSeverity > stringSeverity) ||
                (stringSeverity == EVENT_STRING_SEVERITY_NONE) ||
                (stringSeverity == EVENT_STRING_SEVERITY_NORMAL))
            {
                stringSeverity = tempStringSeverity;
                strncpy(stringPtr, tempEventString, stringLength);
            }
        }
        else
        {
            strncpy(stringPtr, "Bad Event", stringLength);
        }

        /*
         * Make sure the strncpy is terminated
         */
        if (stringLength > 0)
        {
            stringPtr[stringLength - 1] = '\0';
        }
    }

    return (stringSeverity);
}


/*----------------------------------------------------------------------------
**  Function Name: HWM_GetBatteryStatusString
**
**  Parameters:    batteryStatusPtr
**--------------------------------------------------------------------------*/
UINT32 HWM_GetBatteryStatusString(char *stringPtr, BATTERY_STATUS_PTR batteryStatusPtr,
                                  UINT8 stringLength)
{
    UINT32      stringSeverity = EVENT_STRING_SEVERITY_NONE;
    UINT32      tempStringSeverity = EVENT_STRING_SEVERITY_NONE;
    char        tempEventString[MMC_MESSAGE_SIZE] = { 0 };

    if (stringPtr != NULL)
    {
        if (batteryStatusPtr != NULL)
        {
            /*
             * Find the string severity and create the string
             */
            /* Check the battery condition */
            tempStringSeverity = HWM_GetBatteryConditionString(tempEventString,
                                                               batteryStatusPtr->batteryCondition,
                                                               sizeof(tempEventString));

            if (tempStringSeverity > stringSeverity)
            {
                stringSeverity = tempStringSeverity;
                strncpy(stringPtr, tempEventString, stringLength);
            }

            /* Check the battery terminal voltage condition */
            tempStringSeverity = HWM_GetLimitMonitorConditionString(tempEventString,
                                                                    batteryStatusPtr->terminalVoltageReading.limitMonitorValue,
                                                                    sizeof(tempEventString));

            if (tempStringSeverity > stringSeverity)
            {
                stringSeverity = tempStringSeverity;
                strncpy(stringPtr, "Voltage ", stringLength);
                strncpy(stringPtr + strlen(stringPtr),
                        tempEventString, stringLength - strlen(stringPtr));
            }

            /* Check the battery overall status */
            tempStringSeverity = HWM_GetEventSeverityString(tempEventString,
                                                            &batteryStatusPtr->eventProperties,
                                                            sizeof(tempEventString));

            if ((tempStringSeverity > stringSeverity) ||
                (stringSeverity == EVENT_STRING_SEVERITY_NONE) ||
                (stringSeverity == EVENT_STRING_SEVERITY_NORMAL))
            {
                stringSeverity = tempStringSeverity;
                strncpy(stringPtr, tempEventString, stringLength);
            }
        }
        else
        {
            strncpy(stringPtr, "Bad Event", stringLength);
        }

        /*
         * Make sure the strncpy is terminated
         */
        if (stringLength > 0)
        {
            stringPtr[stringLength - 1] = '\0';
        }
    }

    return (stringSeverity);
}


/*----------------------------------------------------------------------------
**  Function Name: HWM_GetChargerStatusString
**
**  Parameters:    chargerStatusPtr
**--------------------------------------------------------------------------*/
UINT32 HWM_GetChargerStatusString(char *stringPtr, CHARGER_STATUS_PTR chargerStatusPtr,
                                  UINT8 stringLength)
{
    UINT32      stringSeverity = EVENT_STRING_SEVERITY_NONE;
    UINT32      tempStringSeverity = EVENT_STRING_SEVERITY_NONE;
    char        tempEventString[MMC_MESSAGE_SIZE] = { 0 };

    if (stringPtr != NULL)
    {
        if (chargerStatusPtr != NULL)
        {
            /*
             * Find the string severity and create the string
             */
            /* Check the charger condition */
            tempStringSeverity = HWM_GetChargerConditionString(tempEventString,
                                                               chargerStatusPtr->chargerCondition,
                                                               sizeof(tempEventString));

            if (tempStringSeverity > stringSeverity)
            {
                stringSeverity = tempStringSeverity;
                strncpy(stringPtr, tempEventString, stringLength);
            }

            /* Check the charger overall status */
            tempStringSeverity = HWM_GetEventSeverityString(tempEventString,
                                                            &chargerStatusPtr->eventProperties,
                                                            sizeof(tempEventString));

            if ((tempStringSeverity > stringSeverity) ||
                (stringSeverity == EVENT_STRING_SEVERITY_NONE) ||
                (stringSeverity == EVENT_STRING_SEVERITY_NORMAL))
            {
                stringSeverity = tempStringSeverity;
                strncpy(stringPtr, tempEventString, stringLength);
            }
        }
        else
        {
            strncpy(stringPtr, "Bad Event", stringLength);
        }

        /*
         * Make sure the strncpy is terminated
         */
        if (stringLength > 0)
        {
            stringPtr[stringLength - 1] = '\0';
        }
    }

    return (stringSeverity);
}


/*----------------------------------------------------------------------------
**  Function Name: HWM_GetFuelGaugeStatusString
**
**  Parameters:    fuelGaugeStatusPtr
**--------------------------------------------------------------------------*/
UINT32 HWM_GetFuelGaugeStatusString(char *stringPtr,
                                    FUEL_GAUGE_STATUS_PTR fuelGaugeStatusPtr,
                                    UINT8 stringLength)
{
    UINT32      stringSeverity = EVENT_STRING_SEVERITY_NONE;
    UINT32      tempStringSeverity = EVENT_STRING_SEVERITY_NONE;
    char        tempEventString[MMC_MESSAGE_SIZE] = { 0 };

    if (stringPtr != NULL)
    {
        if (fuelGaugeStatusPtr != NULL)
        {
            /*
             * Find the string severity and create the string
             */
            /* Check the fuel gauge condition */
            tempStringSeverity = HWM_GetFuelGaugeConditionString(tempEventString,
                                                                 fuelGaugeStatusPtr->fuelGaugeCondition,
                                                                 sizeof(tempEventString));

            if (tempStringSeverity > stringSeverity)
            {
                stringSeverity = tempStringSeverity;
                strncpy(stringPtr, tempEventString, stringLength);
            }

            /* Check the current flow condition */
            tempStringSeverity = HWM_GetCurrentFlowConditionString(tempEventString,
                                                                   fuelGaugeStatusPtr->currentFlowCondition,
                                                                   sizeof(tempEventString));

            if (tempStringSeverity > stringSeverity)
            {
                stringSeverity = tempStringSeverity;
                strncpy(stringPtr, "Current ", stringLength);
                strncpy(stringPtr + strlen(stringPtr),
                        tempEventString, stringLength - strlen(stringPtr));
            }

            /* Check the fuel gauge overall status */
            tempStringSeverity = HWM_GetEventSeverityString(tempEventString,
                                                            &fuelGaugeStatusPtr->eventProperties,
                                                            sizeof(tempEventString));

            if ((tempStringSeverity > stringSeverity) ||
                (stringSeverity == EVENT_STRING_SEVERITY_NONE) ||
                (stringSeverity == EVENT_STRING_SEVERITY_NORMAL))
            {
                stringSeverity = tempStringSeverity;
                strncpy(stringPtr, tempEventString, stringLength);
            }
        }
        else
        {
            strncpy(stringPtr, "Bad Event", stringLength);
        }

        /*
         * Make sure the strncpy is terminated
         */
        if (stringLength > 0)
        {
            stringPtr[stringLength - 1] = '\0';
        }
    }

    return (stringSeverity);
}


/*----------------------------------------------------------------------------
**  Function Name: HWM_GetMainRegulatorStatusString
**
**  Parameters:    mainRegulatorStatusPtr
**--------------------------------------------------------------------------*/
UINT32 HWM_GetMainRegulatorStatusString(char *stringPtr,
                                        MAIN_REGULATOR_STATUS_PTR mainRegulatorStatusPtr,
                                        UINT8 stringLength)
{
    UINT32      stringSeverity = EVENT_STRING_SEVERITY_NONE;
    UINT32      tempStringSeverity = EVENT_STRING_SEVERITY_NONE;
    char        tempEventString[MMC_MESSAGE_SIZE] = { 0 };

    if (stringPtr != NULL)
    {
        if (mainRegulatorStatusPtr != NULL)
        {
            /*
             * Find the string severity and create the string
             */
            /* Check the main regulator condition */
            tempStringSeverity = HWM_GetMainRegulatorConditionString(tempEventString,
                                                                     mainRegulatorStatusPtr->mainRegulatorCondition,
                                                                     sizeof(tempEventString));

            if (tempStringSeverity > stringSeverity)
            {
                stringSeverity = tempStringSeverity;
                strncpy(stringPtr, tempEventString, stringLength);
            }

            /* Check the main regulator input voltage condition */
            tempStringSeverity = HWM_GetLimitMonitorConditionString(tempEventString,
                                                                    mainRegulatorStatusPtr->inputVoltageReading.limitMonitorValue,
                                                                    sizeof(tempEventString));

            if (tempStringSeverity > stringSeverity)
            {
                stringSeverity = tempStringSeverity;
                strncpy(stringPtr, "InputV ", stringLength);
                strncpy(stringPtr + strlen(stringPtr),
                        tempEventString, stringLength - strlen(stringPtr));
            }

            /* Check the main regulator output voltage condition */
            tempStringSeverity = HWM_GetLimitMonitorConditionString(tempEventString,
                                                                    mainRegulatorStatusPtr->outputVoltageReading.limitMonitorValue,
                                                                    sizeof(tempEventString));

            if (tempStringSeverity > stringSeverity)
            {
                stringSeverity = tempStringSeverity;
                strncpy(stringPtr, "OutputV ", stringLength);
                strncpy(stringPtr + strlen(stringPtr),
                        tempEventString, stringLength - strlen(stringPtr));
            }

            /* Check the processor board supply voltage condition */
            tempStringSeverity = HWM_GetLimitMonitorConditionString(tempEventString,
                                                                    mainRegulatorStatusPtr->procBoardSupplyVoltageReading.limitMonitorValue,
                                                                    sizeof(tempEventString));

            if (tempStringSeverity > stringSeverity)
            {
                stringSeverity = tempStringSeverity;
                strncpy(stringPtr, "SupplyV ", stringLength);
                strncpy(stringPtr + strlen(stringPtr),
                        tempEventString, stringLength - strlen(stringPtr));
            }

            /* Check the main regulator overall status */
            tempStringSeverity = HWM_GetEventSeverityString(tempEventString,
                                                            &mainRegulatorStatusPtr->eventProperties,
                                                            sizeof(tempEventString));

            if ((tempStringSeverity > stringSeverity) ||
                (stringSeverity == EVENT_STRING_SEVERITY_NONE) ||
                (stringSeverity == EVENT_STRING_SEVERITY_NORMAL))
            {
                stringSeverity = tempStringSeverity;
                strncpy(stringPtr, tempEventString, stringLength);
            }
        }
        else
        {
            strncpy(stringPtr, "Bad Event", stringLength);
        }

        /*
         * Make sure the strncpy is terminated
         */
        if (stringLength > 0)
        {
            stringPtr[stringLength - 1] = '\0';
        }
    }

    return (stringSeverity);
}


/*----------------------------------------------------------------------------
**  Function Name: HWM_GetStatusCodeString
**
**  Parameters:    stringPtr - pointer to store the constructed string
**                 conditionValue - condition value to decode
**                 stringLength - size of the space allocated for stringPtr
**--------------------------------------------------------------------------*/
void HWM_GetStatusCodeString(char *stringPtr, HWM_STATUS_CODE statusCode,
                             UINT8 stringLength)
{
    if (stringPtr != NULL)
    {
        switch (statusCode)
        {
            case STATUS_CODE_UNKNOWN:
                strncpy(stringPtr, "Unknown", stringLength);
                break;

            case STATUS_CODE_NOT_PRESENT:
                strncpy(stringPtr, "Not Present", stringLength);
                break;

            case STATUS_CODE_VALID:
                strncpy(stringPtr, "Valid", stringLength);
                break;

            case STATUS_CODE_BUSY:
                strncpy(stringPtr, "Busy", stringLength);
                break;

            case STATUS_CODE_NOT_READY:
                strncpy(stringPtr, "Not Ready", stringLength);
                break;

            case STATUS_CODE_ERROR:
                strncpy(stringPtr, "Not Readable", stringLength);
                break;

            default:
                strncpy(stringPtr, "Undefined", stringLength);
                break;
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
**  Function Name: HWM_GetComponentIDString
**
**  Parameters:    stringPtr - pointer to store the constructed string
**                 componentValue - condition value to decode
**                 stringLength - size of the space allocated for stringPtr
**--------------------------------------------------------------------------*/
void HWM_GetComponentIDString(char *stringPtr, UINT8 componentValue, UINT8 stringLength)
{
    if (stringPtr != NULL)
    {
        switch (componentValue)
        {
            case UNKNOWN_ID:
                strncpy(stringPtr, "Unknown", stringLength);
                break;

            case HWM_OVERALL_ID:
                strncpy(stringPtr, "General Status", stringLength);
                break;

            case I2C_HARDWARE_OVERALL_ID:
                strncpy(stringPtr, "General Hardware", stringLength);
                break;

                /* CCB Hardware */
            case CCB_PROCESSOR_ID:
                strncpy(stringPtr, "CCB Processor", stringLength);
                break;

            case CCB_BOARD_EEPROM_ID:
                strncpy(stringPtr, "CCB Brd EEPROM", stringLength);
                break;

            case CCB_MEMORY_MODULE_ID:
                strncpy(stringPtr, "CCB Memory", stringLength);
                break;

            case CCB_MEMORY_MODULE_EEPROM_ID:
                strncpy(stringPtr, "CCB Mem EEPROM", stringLength);
                break;

#if 0
                /* Processor Board Hardware */
            case PCA9548_ID:
                strncpy(stringPtr, "PCA9548", stringLength);
                break;

            case PROC_BOARD_LM80_LM87_ID:
                strncpy(stringPtr, "Proc LM80/LM87", stringLength);
                break;

            case PROC_BOARD_LM80_ID:
                strncpy(stringPtr, "Proc Board LM80", stringLength);
                break;

            case PROC_BOARD_LM87_ID:
                strncpy(stringPtr, "Proc Board LM87", stringLength);
                break;

            case PROC_BOARD_LM75_LM92_ID:
                strncpy(stringPtr, "Proc LM75/LM92", stringLength);
                break;

            case PROC_BOARD_LM75_ID:
                strncpy(stringPtr, "Proc Board LM75", stringLength);
                break;

            case PROC_BOARD_LM92_ID:
                strncpy(stringPtr, "Proc Board LM92", stringLength);
                break;

            case RESET_CONTROL_PCF8574_ID:
                strncpy(stringPtr, "Reset PCF8574", stringLength);
                break;

            case POWER_SUPPLY_PCF8574_ID:
                strncpy(stringPtr, "Power PCF8574", stringLength);
                break;

            case CHASSIS_EEPROM_ID:
                strncpy(stringPtr, "Chassis EEPROM", stringLength);
                break;

            case PROC_BOARD_EEPROM_ID:
                strncpy(stringPtr, "Proc Bd EEPROM", stringLength);
                break;

                /* FE Power Supply Hardware */
            case FE_POWER_SUPPLY_ASSEMBLY_EEPROM_ID:
                strncpy(stringPtr, "CN PSB Asm EEPROM", stringLength);
                break;

            case FE_POWER_SUPPLY_INTERFACE_EEPROM_ID:
                strncpy(stringPtr, "CN PSB Int EEPROM", stringLength);
                break;

                /* BE Power Supply Hardware */
            case BE_POWER_SUPPLY_ASSEMBLY_EEPROM_ID:
                strncpy(stringPtr, "CN PSA Asm EEPROM", stringLength);
                break;

            case BE_POWER_SUPPLY_INTERFACE_EEPROM_ID:
                strncpy(stringPtr, "CN PSA Int EEPROM", stringLength);
                break;

                /* FE Buffer Board Hardware */
            case FE_BUFFER_BOARD_LM80_LM87_ID:
                strncpy(stringPtr, "BufferB LM80/LM87", stringLength);
                break;

            case FE_BUFFER_BOARD_LM80_ID:
                strncpy(stringPtr, "BufferB LM80", stringLength);
                break;

            case FE_BUFFER_BOARD_LM87_ID:
                strncpy(stringPtr, "BufferB LM87", stringLength);
                break;

            case FE_BUFFER_BOARD_PCF8574_ID:
                strncpy(stringPtr, "BufferB PCF8574", stringLength);
                break;

            case FE_BUFFER_BOARD_MAX1660_ID:
                strncpy(stringPtr, "BufferB MAX1660", stringLength);
                break;

            case FE_BUFFER_BOARD_EEPROM_ID:
                strncpy(stringPtr, "BufferB EEPROM", stringLength);
                break;

                /* BE Buffer Board Hardware */
            case BE_BUFFER_BOARD_LM80_LM87_ID:
                strncpy(stringPtr, "BufferA LM80/LM87", stringLength);
                break;

            case BE_BUFFER_BOARD_LM80_ID:
                strncpy(stringPtr, "BufferA LM80", stringLength);
                break;

            case BE_BUFFER_BOARD_LM87_ID:
                strncpy(stringPtr, "BufferA LM87", stringLength);
                break;

            case BE_BUFFER_BOARD_PCF8574_ID:
                strncpy(stringPtr, "BufferA PCF8574", stringLength);
                break;

            case BE_BUFFER_BOARD_MAX1660_ID:
                strncpy(stringPtr, "BufferA MAX1660", stringLength);
                break;

            case BE_BUFFER_BOARD_EEPROM_ID:
                strncpy(stringPtr, "BufferA EEPROM", stringLength);
                break;
#endif /* 0 */

                /*
                 * Statuses
                 */
            case HARDWARE_MONITOR_STATUS_ID:
                strncpy(stringPtr, "Monitor", stringLength);
                break;

            case CCB_BOARD_STATUS_ID:
                strncpy(stringPtr, "CCB Board", stringLength);
                break;

            case CCB_NVRAM_BATTERY_STATUS_ID:
                strncpy(stringPtr, "CCB NVRAM Battery", stringLength);
                break;

            case CCB_BOARD_EEPROM_STATUS_ID:
                strncpy(stringPtr, "CCB Board ID", stringLength);
                break;

            case CCB_MEMORY_MODULE_EEPROM_STATUS_ID:
                strncpy(stringPtr, "CCB MEM ID", stringLength);
                break;

            case PROC_BOARD_STATUS_ID:
                strncpy(stringPtr, "Proc Board", stringLength);
                break;

            case POWER_SUPPLY_VOLTAGES_STATUS_ID:
                strncpy(stringPtr, "PS Voltages", stringLength);
                break;

            case FE_PROCESSOR_STATUS_ID:
                strncpy(stringPtr, "ProcB", stringLength);
                break;

            case FE_PROCESSOR_TEMPERATURE_STATUS_ID:
#ifdef I2C_MAG_STYLE_TEMP_EVENTS
                strncpy(stringPtr, "Temp-ProcB", stringLength);
#else   /* I2C_MAG_STYLE_TEMP_EVENTS */
                strncpy(stringPtr, "ProcB Temp", stringLength);
#endif  /* I2C_MAG_STYLE_TEMP_EVENTS */
                break;

            case BE_PROCESSOR_STATUS_ID:
                strncpy(stringPtr, "ProcA", stringLength);
                break;

            case BE_PROCESSOR_TEMPERATURE_STATUS_ID:
#ifdef I2C_MAG_STYLE_TEMP_EVENTS
                strncpy(stringPtr, "Temp-ProcA", stringLength);
#else   /* I2C_MAG_STYLE_TEMP_EVENTS */
                strncpy(stringPtr, "ProcA Temp", stringLength);
#endif  /* I2C_MAG_STYLE_TEMP_EVENTS */
                break;

            case CHASSIS_EEPROM_STATUS_ID:
                strncpy(stringPtr, "CN Chassis ID", stringLength);
                break;

            case PROC_BOARD_EEPROM_STATUS_ID:
                strncpy(stringPtr, "Proc Board ID", stringLength);
                break;

            case FE_POWER_SUPPLY_STATUS_ID:
                strncpy(stringPtr, "CN PSB", stringLength);
                break;

            case FE_POWER_SUPPLY_ASM_EEPROM_STATUS_ID:
                strncpy(stringPtr, "CN PSB Asm ID", stringLength);
                break;

            case FE_POWER_SUPPLY_INT_EEPROM_STATUS_ID:
                strncpy(stringPtr, "CN PSB Int ID", stringLength);
                break;

            case BE_POWER_SUPPLY_STATUS_ID:
                strncpy(stringPtr, "CN PSA", stringLength);
                break;

            case BE_POWER_SUPPLY_ASM_EEPROM_STATUS_ID:
                strncpy(stringPtr, "CN PSA Asm ID", stringLength);
                break;

            case BE_POWER_SUPPLY_INT_EEPROM_STATUS_ID:
                strncpy(stringPtr, "CN PSA Int ID", stringLength);
                break;

            case FE_BUFFER_BOARD_STATUS_ID:
                strncpy(stringPtr, "BufferB", stringLength);
                break;

            case FE_BUFFER_BOARD_TEMPERATURE_STATUS_ID:
#ifdef I2C_MAG_STYLE_TEMP_EVENTS
                strncpy(stringPtr, "Temp-BufferB", stringLength);
#else   /* I2C_MAG_STYLE_TEMP_EVENTS */
                strncpy(stringPtr, "BufferB Temp", stringLength);
#endif  /* I2C_MAG_STYLE_TEMP_EVENTS */
                break;

            case FE_BUFFER_BOARD_BATTERY_STATUS_ID:
                strncpy(stringPtr, "BufferB Battery", stringLength);
                break;

            case FE_BUFFER_BOARD_FUEL_GAUGE_STATUS_ID:
                strncpy(stringPtr, "BufferB Fuel Gauge", stringLength);
                break;

            case FE_BUFFER_BOARD_MAIN_REGULATOR_STATUS_ID:
                strncpy(stringPtr, "BufferB Main Reg", stringLength);
                break;

            case FE_BUFFER_BOARD_CHARGER_STATUS_ID:
                strncpy(stringPtr, "BufferB Charger", stringLength);
                break;

            case FE_BUFFER_BOARD_EEPROM_STATUS_ID:
                strncpy(stringPtr, "BufferB ID", stringLength);
                break;

            case BE_BUFFER_BOARD_STATUS_ID:
                strncpy(stringPtr, "BufferA", stringLength);
                break;

            case BE_BUFFER_BOARD_TEMPERATURE_STATUS_ID:
#ifdef I2C_MAG_STYLE_TEMP_EVENTS
                strncpy(stringPtr, "Temp-BufferA", stringLength);
#else   /* I2C_MAG_STYLE_TEMP_EVENTS */
                strncpy(stringPtr, "BufferA Temp", stringLength);
#endif  /* I2C_MAG_STYLE_TEMP_EVENTS */
                break;

            case BE_BUFFER_BOARD_BATTERY_STATUS_ID:
                strncpy(stringPtr, "BufferA Battery", stringLength);
                break;

            case BE_BUFFER_BOARD_FUEL_GAUGE_STATUS_ID:
                strncpy(stringPtr, "BufferA Fuel Gauge", stringLength);
                break;

            case BE_BUFFER_BOARD_MAIN_REGULATOR_STATUS_ID:
                strncpy(stringPtr, "BufferA Main Reg", stringLength);
                break;

            case BE_BUFFER_BOARD_CHARGER_STATUS_ID:
                strncpy(stringPtr, "BufferA Charger", stringLength);
                break;

            case BE_BUFFER_BOARD_EEPROM_STATUS_ID:
                strncpy(stringPtr, "BufferA ID", stringLength);
                break;

            default:
                strncpy(stringPtr, "Undefined", stringLength);
                break;
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

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
