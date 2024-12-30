/* $Id: HWM_ConditionStrings.c 143007 2010-06-22 14:48:58Z m4 $ */
/*============================================================================
** FILE NAME:       HWM_ConditionStrings.c
** MODULE TITLE:    Hardware monitor string impementation
**
** DESCRIPTION:     Hardware monitor string building functions
**
** Copyright (c) 2001-2009 Xiotech Corporation. All rights reserved.
**==========================================================================*/
#include "HWM_ConditionStrings.h"

#ifdef LOG_SIMULATOR
#include "LogSimFuncs.h"
#else   /* LOG_SIMULATOR */
#include "XIO_Std.h"
#include "XIO_Const.h"
#endif  /* LOG_SIMULATOR */

/*****************************************************************************
** Code Start
*****************************************************************************/

/*----------------------------------------------------------------------------
**  Function Name: HWM_GetLimitMonitorConditionString
**
**  Parameters:    stringPtr - pointer to store the constructed string
**                 conditionValue - condition value to decode
**                 stringLength - size of the space allocated for stringPtr
**--------------------------------------------------------------------------*/
UINT32 HWM_GetLimitMonitorConditionString(char *stringPtr, UINT8 conditionValue,
                                          UINT8 stringLength)
{
    UINT32      stringSeverity = EVENT_STRING_SEVERITY_NONE;

    if (stringPtr != NULL)
    {
        switch (conditionValue)
        {
            case LIMIT_MONITOR_UNKNOWN:
                strncpy(stringPtr, "Unknown", stringLength);
                break;

            case LIMIT_MONITOR_GOOD:
                strncpy(stringPtr, "Good", stringLength);
                stringSeverity = EVENT_STRING_SEVERITY_NORMAL;
                break;

            case LIMIT_MONITOR_TRIPPED:
                strncpy(stringPtr, "Limit", stringLength);
                stringSeverity = EVENT_STRING_SEVERITY_ERROR;
                break;

            default:
                strncpy(stringPtr, "Undefined", stringLength);
                stringSeverity = EVENT_STRING_SEVERITY_ERROR;
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

    return (stringSeverity);
}


/*----------------------------------------------------------------------------
**  Function Name: HWM_GetTemperatureConditionString
**
**  Parameters:    stringPtr - pointer to store the constructed string
**                 conditionValue - condition value to decode
**                 stringLength - size of the space allocated for stringPtr
**--------------------------------------------------------------------------*/
UINT32 HWM_GetTemperatureConditionString(char *stringPtr, UINT8 conditionValue,
                                         UINT8 stringLength)
{
    UINT32      stringSeverity = EVENT_STRING_SEVERITY_NONE;

    if (stringPtr != NULL)
    {
        switch (conditionValue)
        {
            case TEMPERATURE_CONDITION_UNKNOWN:
                strncpy(stringPtr, "Unknown", stringLength);
                break;

            case TEMPERATURE_CONDITION_NORMAL:
                strncpy(stringPtr, "Normal", stringLength);
                stringSeverity = EVENT_STRING_SEVERITY_NORMAL;
                break;

            case TEMPERATURE_CONDITION_COLD:
                strncpy(stringPtr, "Cold", stringLength);
                stringSeverity = EVENT_STRING_SEVERITY_WARNING;
                break;

            case TEMPERATURE_CONDITION_HOT:
                strncpy(stringPtr, "Hot", stringLength);
                stringSeverity = EVENT_STRING_SEVERITY_WARNING;
                break;

            case TEMPERATURE_CONDITION_COLD_CRITICAL:
                strncpy(stringPtr, "Cold Critical", stringLength);
                stringSeverity = EVENT_STRING_SEVERITY_ERROR;
                break;

            case TEMPERATURE_CONDITION_HOT_CRITICAL:
                strncpy(stringPtr, "Hot Critical", stringLength);
                stringSeverity = EVENT_STRING_SEVERITY_ERROR;
                break;

            default:
                strncpy(stringPtr, "Undefined", stringLength);
                stringSeverity = EVENT_STRING_SEVERITY_ERROR;
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

    return (stringSeverity);
}


/*----------------------------------------------------------------------------
**  Function Name: HWM_GetPowerSupplyConditionString
**
**  Parameters:    stringPtr - pointer to store the constructed string
**                 conditionValue - condition value to decode
**                 stringLength - size of the space allocated for stringPtr
**--------------------------------------------------------------------------*/
UINT32 HWM_GetPowerSupplyConditionString(char *stringPtr, UINT8 conditionValue,
                                         UINT8 stringLength)
{
    UINT32      stringSeverity = EVENT_STRING_SEVERITY_NONE;

    if (stringPtr != NULL)
    {
        switch (conditionValue)
        {
            case POWER_SUPPLY_CONDITION_UNKNOWN:
                strncpy(stringPtr, "Unknown", stringLength);
                break;

            case POWER_SUPPLY_CONDITION_GOOD:
                strncpy(stringPtr, "Good", stringLength);
                stringSeverity = EVENT_STRING_SEVERITY_NORMAL;
                break;

            case POWER_SUPPLY_CONDITION_HIGH_TEMPERATURE:
                strncpy(stringPtr, "High Temperature", stringLength);
                stringSeverity = EVENT_STRING_SEVERITY_WARNING;
                break;

            case POWER_SUPPLY_CONDITION_DC_FAILED:
                strncpy(stringPtr, "DC Failed", stringLength);
                stringSeverity = EVENT_STRING_SEVERITY_ERROR;
                break;

            case POWER_SUPPLY_CONDITION_AC_FAILED:
                strncpy(stringPtr, "AC Failed", stringLength);
                stringSeverity = EVENT_STRING_SEVERITY_ERROR;
                break;

            case POWER_SUPPLY_CONDITION_INSERTED:
                strncpy(stringPtr, "Inserted", stringLength);
                stringSeverity = EVENT_STRING_SEVERITY_WARNING;
                break;

            case POWER_SUPPLY_CONDITION_NOT_PRESENT:
                strncpy(stringPtr, "Not Present", stringLength);
                stringSeverity = EVENT_STRING_SEVERITY_ERROR;
                break;

            default:
                strncpy(stringPtr, "Undefined", stringLength);
                stringSeverity = EVENT_STRING_SEVERITY_ERROR;
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

    return (stringSeverity);
}


/*----------------------------------------------------------------------------
**  Function Name: HWM_GetCoolingFanConditionString
**
**  Parameters:    stringPtr - pointer to store the constructed string
**                 conditionValue - condition value to decode
**                 stringLength - size of the space allocated for stringPtr
**--------------------------------------------------------------------------*/
UINT32 HWM_GetCoolingFanConditionString(char *stringPtr, UINT8 conditionValue,
                                        UINT8 stringLength)
{
    UINT32      stringSeverity = EVENT_STRING_SEVERITY_NONE;

    if (stringPtr != NULL)
    {
        switch (conditionValue)
        {
            case COOLING_FAN_CONDITION_UNKNOWN:
                strncpy(stringPtr, "Unknown", stringLength);
                break;

            case COOLING_FAN_CONDITION_GOOD:
                strncpy(stringPtr, "Good", stringLength);
                stringSeverity = EVENT_STRING_SEVERITY_NORMAL;
                break;

            case COOLING_FAN_CONDITION_FAILED:
                strncpy(stringPtr, "Failed", stringLength);
                stringSeverity = EVENT_STRING_SEVERITY_ERROR;
                break;

            case COOLING_FAN_CONDITION_NOT_PRESENT:
                strncpy(stringPtr, "Not Present", stringLength);
                stringSeverity = EVENT_STRING_SEVERITY_ERROR;
                break;

            default:
                strncpy(stringPtr, "Undefined", stringLength);
                stringSeverity = EVENT_STRING_SEVERITY_ERROR;
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

    return (stringSeverity);
}


/*----------------------------------------------------------------------------
**  Function Name: HWM_GetProcessorResetConditionString
**
**  Parameters:    stringPtr - pointer to store the constructed string
**                 conditionValue - condition value to decode
**                 stringLength - size of the space allocated for stringPtr
**--------------------------------------------------------------------------*/
UINT32 HWM_GetProcessorResetConditionString(char *stringPtr, UINT8 conditionValue,
                                            UINT8 stringLength)
{
    UINT32      stringSeverity = EVENT_STRING_SEVERITY_NONE;

    if (stringPtr != NULL)
    {
        switch (conditionValue)
        {
            case PROCESSOR_RESET_CONDITION_UNKNOWN:
                strncpy(stringPtr, "Unknown", stringLength);
                break;

            case PROCESSOR_RESET_CONDITION_RUNNING:
                strncpy(stringPtr, "Running", stringLength);
                stringSeverity = EVENT_STRING_SEVERITY_NORMAL;
                break;

            case PROCESSOR_RESET_CONDITION_RESET:
                strncpy(stringPtr, "Reset", stringLength);
                stringSeverity = EVENT_STRING_SEVERITY_ERROR;
                break;

            default:
                strncpy(stringPtr, "Undefined", stringLength);
                stringSeverity = EVENT_STRING_SEVERITY_ERROR;
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

    return (stringSeverity);
}


/*----------------------------------------------------------------------------
**  Function Name: HWM_GetBatteryConditionString
**
**  Parameters:    stringPtr - pointer to store the constructed string
**                 conditionValue - condition value to decode
**                 stringLength - size of the space allocated for stringPtr
**--------------------------------------------------------------------------*/
UINT32 HWM_GetBatteryConditionString(char *stringPtr, UINT8 conditionValue,
                                     UINT8 stringLength)
{
    UINT32      stringSeverity = EVENT_STRING_SEVERITY_NONE;

    if (stringPtr != NULL)
    {
        switch (conditionValue)
        {
            case BATTERY_CONDITION_UNKNOWN:
                strncpy(stringPtr, "Unknown", stringLength);
                break;

            case BATTERY_CONDITION_GOOD:
                strncpy(stringPtr, "Good", stringLength);
                stringSeverity = EVENT_STRING_SEVERITY_NORMAL;
                break;

            case BATTERY_CONDITION_LOW_CAPACITY:
                strncpy(stringPtr, "Low Capacity", stringLength);
                stringSeverity = EVENT_STRING_SEVERITY_WARNING;
                break;

            case BATTERY_CONDITION_UNDER_VOLTAGE:
                strncpy(stringPtr, "Under Voltage", stringLength);
                stringSeverity = EVENT_STRING_SEVERITY_ERROR;
                break;

            case BATTERY_CONDITION_OVER_VOLTAGE:
                strncpy(stringPtr, "Over Voltage", stringLength);
                stringSeverity = EVENT_STRING_SEVERITY_ERROR;
                break;

            case BATTERY_CONDITION_NOT_PRESENT:
                strncpy(stringPtr, "Not Present", stringLength);
                stringSeverity = EVENT_STRING_SEVERITY_ERROR;
                break;

            default:
                strncpy(stringPtr, "Undefined", stringLength);
                stringSeverity = EVENT_STRING_SEVERITY_ERROR;
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

    return (stringSeverity);
}


/*----------------------------------------------------------------------------
**  Function Name: HWM_GetCurrentFlowConditionString
**
**  Parameters:    stringPtr - pointer to store the constructed string
**                 conditionValue - condition value to decode
**                 stringLength - size of the space allocated for stringPtr
**--------------------------------------------------------------------------*/
UINT32 HWM_GetCurrentFlowConditionString(char *stringPtr, UINT8 conditionValue,
                                         UINT8 stringLength)
{
    UINT32      stringSeverity = EVENT_STRING_SEVERITY_NONE;

    if (stringPtr != NULL)
    {
        switch (conditionValue)
        {
            case CURRENT_FLOW_CONDITION_UNKNOWN:
                strncpy(stringPtr, "Unknown", stringLength);
                break;

            case CURRENT_FLOW_CONDITION_GOOD:
                strncpy(stringPtr, "Good", stringLength);
                stringSeverity = EVENT_STRING_SEVERITY_NORMAL;
                break;

            case CURRENT_FLOW_CONDITION_ABNORMAL:
                strncpy(stringPtr, "Abnormal", stringLength);
                stringSeverity = EVENT_STRING_SEVERITY_ERROR;
                break;

            default:
                strncpy(stringPtr, "Undefined", stringLength);
                stringSeverity = EVENT_STRING_SEVERITY_ERROR;
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

    return (stringSeverity);
}


/*----------------------------------------------------------------------------
**  Function Name: HWM_GetFuelGaugeConditionString
**
**  Parameters:    stringPtr - pointer to store the constructed string
**                 conditionValue - condition value to decode
**                 stringLength - size of the space allocated for stringPtr
**--------------------------------------------------------------------------*/
UINT32 HWM_GetFuelGaugeConditionString(char *stringPtr, UINT8 conditionValue,
                                       UINT8 stringLength)
{
    UINT32      stringSeverity = EVENT_STRING_SEVERITY_NONE;

    if (stringPtr != NULL)
    {
        switch (conditionValue)
        {
            case FUEL_GAUGE_CONDITION_UNKNOWN:
                strncpy(stringPtr, "Unknown", stringLength);
                break;

            case FUEL_GAUGE_CONDITION_GOOD:
                strncpy(stringPtr, "Good", stringLength);
                stringSeverity = EVENT_STRING_SEVERITY_NORMAL;
                break;

            case FUEL_GAUGE_CONDITION_SHUTDOWN:
                strncpy(stringPtr, "Shut Down", stringLength);
                stringSeverity = EVENT_STRING_SEVERITY_ERROR;
                break;

            default:
                strncpy(stringPtr, "Undefined", stringLength);
                stringSeverity = EVENT_STRING_SEVERITY_ERROR;
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

    return (stringSeverity);
}


/*----------------------------------------------------------------------------
**  Function Name: HWM_GetMainRegulatorConditionString
**
**  Parameters:    stringPtr - pointer to store the constructed string
**                 conditionValue - condition value to decode
**                 stringLength - size of the space allocated for stringPtr
**--------------------------------------------------------------------------*/
UINT32 HWM_GetMainRegulatorConditionString(char *stringPtr, UINT8 conditionValue,
                                           UINT8 stringLength)
{
    UINT32      stringSeverity = EVENT_STRING_SEVERITY_NONE;

    if (stringPtr != NULL)
    {
        switch (conditionValue)
        {
            case MAIN_REGULATOR_CONDITION_UNKNOWN:
                strncpy(stringPtr, "Unknown", stringLength);
                break;

            case MAIN_REGULATOR_CONDITION_OPERATIONAL:
                strncpy(stringPtr, "Operational", stringLength);
                stringSeverity = EVENT_STRING_SEVERITY_NORMAL;
                break;

            case MAIN_REGULATOR_CONDITION_SHUTDOWN_GOOD:
                strncpy(stringPtr, "Shut Down OK", stringLength);
                stringSeverity = EVENT_STRING_SEVERITY_INFO;
                break;

            case MAIN_REGULATOR_CONDITION_SHUTDOWN_ERROR:
                strncpy(stringPtr, "Shut Down ERROR", stringLength);
                stringSeverity = EVENT_STRING_SEVERITY_ERROR;
                break;

            default:
                strncpy(stringPtr, "Undefined", stringLength);
                stringSeverity = EVENT_STRING_SEVERITY_ERROR;
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

    return (stringSeverity);
}


/*----------------------------------------------------------------------------
**  Function Name: HWM_GetChargerConditionString
**
**  Parameters:    stringPtr - pointer to store the constructed string
**                 conditionValue - condition value to decode
**                 stringLength - size of the space allocated for stringPtr
**--------------------------------------------------------------------------*/
UINT32 HWM_GetChargerConditionString(char *stringPtr, UINT8 conditionValue,
                                     UINT8 stringLength)
{
    UINT32      stringSeverity = EVENT_STRING_SEVERITY_NONE;

    if (stringPtr != NULL)
    {
        switch (conditionValue)
        {
            case CHARGER_CONDITION_UNKNOWN:
                strncpy(stringPtr, "Unknown", stringLength);
                break;

            case CHARGER_CONDITION_IDLE:
                strncpy(stringPtr, "Idle", stringLength);
                stringSeverity = EVENT_STRING_SEVERITY_NORMAL;
                break;

            case CHARGER_CONDITION_TRICKLE:
            case CHARGER_CONDITION_BULK:
            case CHARGER_CONDITION_OVER:
            case CHARGER_CONDITION_TOPOFF:
                strncpy(stringPtr, "Charging", stringLength);
                stringSeverity = EVENT_STRING_SEVERITY_INFO;
                break;

            default:
                strncpy(stringPtr, "Undefined", stringLength);
                stringSeverity = EVENT_STRING_SEVERITY_ERROR;
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

    return (stringSeverity);
}


/*----------------------------------------------------------------------------
**  Function Name: HWM_GetNVRAMBatteryConditionString
**
**  Parameters:    stringPtr - pointer to store the constructed string
**                 conditionValue - condition value to decode
**                 stringLength - size of the space allocated for stringPtr
**--------------------------------------------------------------------------*/
UINT32 HWM_GetNVRAMBatteryConditionString(char *stringPtr, UINT8 conditionValue,
                                          UINT8 stringLength)
{
    UINT32      stringSeverity = EVENT_STRING_SEVERITY_NONE;

    if (stringPtr != NULL)
    {
        switch (conditionValue)
        {
            case NVRAM_BATTERY_CONDITION_UNKNOWN:
                strncpy(stringPtr, "Unknown", stringLength);
                break;

            case NVRAM_BATTERY_CONDITION_GOOD:
                strncpy(stringPtr, "Good", stringLength);
                stringSeverity = EVENT_STRING_SEVERITY_NORMAL;
                break;

            case NVRAM_BATTERY_CONDITION_FAILED:
                strncpy(stringPtr, "Failed", stringLength);
                stringSeverity = EVENT_STRING_SEVERITY_ERROR;
                break;

            default:
                strncpy(stringPtr, "Undefined", stringLength);
                stringSeverity = EVENT_STRING_SEVERITY_ERROR;
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

    return (stringSeverity);
}

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
