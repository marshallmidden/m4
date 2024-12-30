/* $Id: HWM.h 143020 2010-06-22 18:35:56Z m4 $ */
/**
******************************************************************************
**
**  @file   HWM.h
**
**  @brief  Header file for all hardware monitor components
**
**  Defines the data and functions associated with the hardware
**  monitor on the CCB board.
**
** WARNING:         This code has contracts with the CCBE and XSSA, through
**                  the packet interface.  Any changes to these structures or
**                  definitions must also be reflected in the CCB and XSSA.
**
**  Copyright (c) 2001-2010 XIOtech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _HWM_H_
#define _HWM_H_

#include <logging.h>
#include "xci_structure.h"
#include "XIO_Types.h"

#include "IPMI_Defines.h"
#include "xk_kernel.h"

#ifdef __cplusplus
#pragma pack(push, 1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/
typedef UINT16 MILLIVOLTS;
typedef INT8 DEGREES_CELSIUS;
typedef UINT32 HWM_STATUS_CODE;

                                            /* #define HWM_CHILD_DEVICE_LOG_MESSAGES *//* Log event control                */
#define I2C_MAG_STYLE_TEMP_EVENTS       /* Change the look of temp events   */
#define HWM_PERIOD              5000    /* Monitor every 5 seconds          */

#define AC_FAILED_VALID_SAMPLE_COUNT            1
#define DC_FAILED_VALID_SAMPLE_COUNT            2

/*
** Status code pertaining to readings taken from the I2C devices.
*/
enum HWM_STATUS_CODE_VALUE_ENUM
{
    STATUS_CODE_UNKNOWN = 0,
    STATUS_CODE_NOT_PRESENT = 1,
    STATUS_CODE_VALID = 2,
    STATUS_CODE_BUSY = 3,
    STATUS_CODE_NOT_READY = 4,
    STATUS_CODE_ERROR = 5
};

enum HWM_EVENT_STRING_SEVERITY_VALUE_ENUM
{
    EVENT_STRING_SEVERITY_NONE = 0,
    EVENT_STRING_SEVERITY_NORMAL = 1,
    EVENT_STRING_SEVERITY_DEBUG = 2,
    EVENT_STRING_SEVERITY_INFO = 3,
    EVENT_STRING_SEVERITY_WARNING = 4,
    EVENT_STRING_SEVERITY_ERROR = 5,
};

/**************************************
** Event tracking
**************************************/
typedef struct HWM_EVENT_STATISTICS_STRUCT
{
    UINT32      eventCounterWarning;
    UINT32      eventCounterError;
} HWM_EVENT_STATISTICS;

typedef struct HWM_EVENT_PROPERTIES_FLAGS_BITS_STRUCT
{
    UINT8       conditionInformational:1;
    UINT8       conditionWarning:1;
    UINT8       conditionError:1;
    UINT8       conditionDebug:1;
    UINT8       conditionChangeInformational:1;
    UINT8       conditionChangeWarning:1;
    UINT8       conditionChangeError:1;
    UINT8       conditionChangeDebug:1;
} HWM_EVENT_PROPERTIES_FLAGS_BITS;

typedef union HWM_EVENT_PROPERTIES_FLAGS_UNION
{
    HWM_EVENT_PROPERTIES_FLAGS_BITS bits;
    UINT8       value;
} HWM_EVENT_PROPERTIES_FLAGS;

enum HWM_COMPONENT_ID_VALUE_ENUM
{
    UNKNOWN_ID = 0,
    HWM_OVERALL_ID = 1,
    I2C_HARDWARE_OVERALL_ID = 2,

    /* CCB Hardware */
    CCB_PROCESSOR_ID = 3,
    CCB_BOARD_EEPROM_ID = 4,
    CCB_MEMORY_MODULE_ID = 5,
    CCB_MEMORY_MODULE_EEPROM_ID = 6,

    /* Status */
    HARDWARE_MONITOR_STATUS_ID = 34,
    CCB_BOARD_STATUS_ID = 35,
    CCB_NVRAM_BATTERY_STATUS_ID = 36,
    CCB_BOARD_EEPROM_STATUS_ID = 37,
    CCB_MEMORY_MODULE_EEPROM_STATUS_ID = 38,
    PROC_BOARD_STATUS_ID = 39,
    POWER_SUPPLY_VOLTAGES_STATUS_ID = 40,
    FE_PROCESSOR_STATUS_ID = 41,
    FE_PROCESSOR_TEMPERATURE_STATUS_ID = 42,
    BE_PROCESSOR_STATUS_ID = 43,
    BE_PROCESSOR_TEMPERATURE_STATUS_ID = 44,
    CHASSIS_EEPROM_STATUS_ID = 45,
    PROC_BOARD_EEPROM_STATUS_ID = 46,
    FE_POWER_SUPPLY_STATUS_ID = 47,
    FE_POWER_SUPPLY_ASM_EEPROM_STATUS_ID = 48,
    FE_POWER_SUPPLY_INT_EEPROM_STATUS_ID = 49,
    BE_POWER_SUPPLY_STATUS_ID = 50,
    BE_POWER_SUPPLY_ASM_EEPROM_STATUS_ID = 51,
    BE_POWER_SUPPLY_INT_EEPROM_STATUS_ID = 52,
    FE_BUFFER_BOARD_STATUS_ID = 53,
    FE_BUFFER_BOARD_TEMPERATURE_STATUS_ID = 54,
    FE_BUFFER_BOARD_BATTERY_STATUS_ID = 55,
    FE_BUFFER_BOARD_FUEL_GAUGE_STATUS_ID = 56,
    FE_BUFFER_BOARD_MAIN_REGULATOR_STATUS_ID = 57,
    FE_BUFFER_BOARD_CHARGER_STATUS_ID = 58,
    FE_BUFFER_BOARD_EEPROM_STATUS_ID = 59,
    BE_BUFFER_BOARD_STATUS_ID = 60,
    BE_BUFFER_BOARD_TEMPERATURE_STATUS_ID = 61,
    BE_BUFFER_BOARD_BATTERY_STATUS_ID = 62,
    BE_BUFFER_BOARD_FUEL_GAUGE_STATUS_ID = 63,
    BE_BUFFER_BOARD_MAIN_REGULATOR_STATUS_ID = 64,
    BE_BUFFER_BOARD_CHARGER_STATUS_ID = 65,
    BE_BUFFER_BOARD_EEPROM_STATUS_ID = 66
};

typedef struct HWM_PROPERTIES_STRUCT
{
    HWM_STATUS_CODE statusCode;
    UINT32      componentID;
    HWM_EVENT_STATISTICS monitorStatistics;
    HWM_EVENT_PROPERTIES_FLAGS flags;
    UINT8       description[MMC_MESSAGE_SIZE];
} HWM_EVENT_PROPERTIES;

/**************************************
** EEPROM Status
**************************************/

/* Stuff that we know about the EEPROMs */
enum EEPROM_CONDITION_VALUE_ENUM
{
    EEPROM_CONDITION_UNKNOWN = 0,
};

typedef struct EEPROM_STATUS_STRUCT
{
    /* Things pertaining to condition reporting of the object */
    HWM_EVENT_PROPERTIES eventProperties;

    /* Things pertaining to condition of the object */
    UINT8       eepromCondition;
    XCI_DATA    xciData;
} EEPROM_STATUS;

/**************************************
** Device Temperature
**************************************/
typedef enum TEMPERATURE_CONDITION_VALUE_ENUM
{
    TEMPERATURE_CONDITION_UNKNOWN = 0,
    TEMPERATURE_CONDITION_NORMAL = 1,
    TEMPERATURE_CONDITION_COLD = 2,
    TEMPERATURE_CONDITION_HOT = 3,
    TEMPERATURE_CONDITION_COLD_CRITICAL = 4,
    TEMPERATURE_CONDITION_HOT_CRITICAL = 5
} TEMPERATURE_CONDITION_VALUE;

typedef struct TEMPERATURE_STATUS_STRUCT
{
    /* Things pertaining to condition reporting of the object */
    HWM_EVENT_PROPERTIES eventProperties;

    /* Things pertaining to condition of the object */
    DEGREES_CELSIUS maximumDegreesCelsius;
    DEGREES_CELSIUS currentDegreesCelsius;
    DEGREES_CELSIUS minimumDegreesCelsius;
    UINT8       conditionValue;
} TEMPERATURE_STATUS, *TEMPERATURE_STATUS_PTR;

/**************************************
** Voltage Input Monitor
**************************************/
typedef enum LIMIT_MONITOR_VALUE_ENUM
{
    LIMIT_MONITOR_UNKNOWN = 0,
    LIMIT_MONITOR_GOOD = 1,
    LIMIT_MONITOR_TRIPPED = 2
} LIMIT_MONITOR_VALUE;

typedef struct VOLTAGE_INPUT_READING_STRUCT
{
    MILLIVOLTS  maximumMillivolts;
    MILLIVOLTS  currentMillivolts;
    MILLIVOLTS  minimumMillivolts;
    UINT8       limitMonitorValue;
} VOLTAGE_INPUT_READING;

/**************************************
** Power Supplies
**************************************/
enum POWER_SUPPLY_CONDITION_VALUE_ENUM
{
    POWER_SUPPLY_CONDITION_UNKNOWN = 0,
    POWER_SUPPLY_CONDITION_GOOD = 1,
    POWER_SUPPLY_CONDITION_HIGH_TEMPERATURE = 2,
    POWER_SUPPLY_CONDITION_DC_FAILED = 3,
    POWER_SUPPLY_CONDITION_AC_FAILED = 4,
    POWER_SUPPLY_CONDITION_INSERTED = 5,
    POWER_SUPPLY_CONDITION_NOT_PRESENT = 6
};

typedef enum COOLING_FAN_CONDITION_VALUE_ENUM
{
    COOLING_FAN_CONDITION_UNKNOWN = 0,
    COOLING_FAN_CONDITION_GOOD = 1,
    COOLING_FAN_CONDITION_FAILED = 2,
    COOLING_FAN_CONDITION_NOT_PRESENT = 3
} COOLING_FAN_CONDITION_VALUE;

typedef struct POWER_SUPPLY_CONDITION_STRUCT
{
    UINT8       acFailedSampleCounter;
    UINT8       dcFailedSampleCounter;
    UINT8       value;
} POWER_SUPPLY_CONDITION;

typedef struct POWER_SUPPLY_STATUS_STRUCT
{
    /* Things pertaining to condition reporting of the object */
    HWM_EVENT_PROPERTIES eventProperties;

    /* Things pertaining to condition of the object */
    POWER_SUPPLY_CONDITION powerSupplyCondition;
    UINT8       coolingFanConditionValue;
    EEPROM_STATUS assemblyEEPROMStatus;
    EEPROM_STATUS interfaceEEPROMStatus;
} POWER_SUPPLY_STATUS, *POWER_SUPPLY_STATUS_PTR;

/**************************************
** Front-end and Back-end Processors
**************************************/
typedef struct POWER_SUPPLY_VOLTAGES_STATUS_STRUCT
{
    /* Things pertaining to condition reporting of the object */
    HWM_EVENT_PROPERTIES eventProperties;

    /* Things pertaining to condition of the object */
    VOLTAGE_INPUT_READING twelveVoltReading;
    VOLTAGE_INPUT_READING fiveVoltReading;
    VOLTAGE_INPUT_READING threePointThreeVoltReading;
    VOLTAGE_INPUT_READING standbyVoltageReading;
} POWER_SUPPLY_VOLTAGES_STATUS, *POWER_SUPPLY_VOLTAGES_STATUS_PTR;

enum PROCESSOR_RESET_CONDITION_VALUE_ENUM
{
    PROCESSOR_RESET_CONDITION_UNKNOWN = 0,
    PROCESSOR_RESET_CONDITION_RUNNING = 1,
    PROCESSOR_RESET_CONDITION_RESET = 2
};

typedef struct PROC_BOARD_PROCESSOR_STATUS_STRUCT
{
    /* Things pertaining to condition reporting of the object */
    HWM_EVENT_PROPERTIES eventProperties;

    /* Things pertaining to condition of the object */
    TEMPERATURE_STATUS temperatureStatus;
    VOLTAGE_INPUT_READING memorySocketSupplyVoltageReading;
    UINT8       processorResetConditionValue;
} PROC_BOARD_PROCESSOR_STATUS, *PROC_BOARD_PROCESSOR_STATUS_PTR;

typedef struct PROC_BOARD_STATUS_STRUCT
{
    /* Things pertaining to condition reporting of the object */
    HWM_EVENT_PROPERTIES eventProperties;

    /* Things pertaining to condition of the object */
    POWER_SUPPLY_VOLTAGES_STATUS powerSupplyVoltagesStatus;
    PROC_BOARD_PROCESSOR_STATUS frontEndProcessorStatus;
    PROC_BOARD_PROCESSOR_STATUS backEndProcessorStatus;
    EEPROM_STATUS chassisEEPROMStatus;
    EEPROM_STATUS procBoardEEPROMStatus;
} PROC_BOARD_STATUS, *PROC_BOARD_STATUS_PTR;

/**************************************
** Front-end and Back-end buffer boards
**************************************/

/* Stuff that we know about the battery */
enum BATTERY_CONDITION_VALUE_ENUM
{
    BATTERY_CONDITION_UNKNOWN = 0,
    BATTERY_CONDITION_GOOD = 1,
    BATTERY_CONDITION_LOW_CAPACITY = 2,
    BATTERY_CONDITION_UNDER_VOLTAGE = 3,
    BATTERY_CONDITION_OVER_VOLTAGE = 4,
    BATTERY_CONDITION_NOT_PRESENT = 5
};

typedef struct BATTERY_STATUS_STRUCT
{
    /* Things pertaining to condition reporting of the object */
    HWM_EVENT_PROPERTIES eventProperties;

    /* Things pertaining to condition of the object */
    VOLTAGE_INPUT_READING terminalVoltageReading;
    UINT8       batteryCondition;
} BATTERY_STATUS, *BATTERY_STATUS_PTR;

/* Stuff that we know about the fuel gauge */
enum CURRENT_FLOW_CONDITION_VALUE_ENUM
{
    CURRENT_FLOW_CONDITION_UNKNOWN = 0,
    CURRENT_FLOW_CONDITION_GOOD = 1,
    CURRENT_FLOW_CONDITION_ABNORMAL = 2
};

enum FUEL_GAUGE_CONDITION_VALUE_ENUM
{
    FUEL_GAUGE_CONDITION_UNKNOWN = 0,
    FUEL_GAUGE_CONDITION_GOOD = 1,
    FUEL_GAUGE_CONDITION_SHUTDOWN = 2
};

typedef struct FUEL_GAUGE_STATUS_STRUCT
{
    /* Things pertaining to condition reporting of the object */
    HWM_EVENT_PROPERTIES eventProperties;

    /* Things pertaining to condition of the object */
    UINT32      currentFlowRate;
    VOLTAGE_INPUT_READING regulatorOutputVoltageReading;
    UINT8       fuelGaugeCondition;
    UINT8       currentFlowCondition;
} FUEL_GAUGE_STATUS, *FUEL_GAUGE_STATUS_PTR;

/* Stuff that we know about the main regulator */
enum MAIN_REGULATOR_CONDITION_VALUE_ENUM
{
    MAIN_REGULATOR_CONDITION_UNKNOWN = 0,
    MAIN_REGULATOR_CONDITION_OPERATIONAL = 1,
    MAIN_REGULATOR_CONDITION_SHUTDOWN_ERROR = 2,
    MAIN_REGULATOR_CONDITION_SHUTDOWN_GOOD = 3
};

typedef struct MAIN_REGULATOR_STATUS_STRUCT
{
    /* Things pertaining to condition reporting of the object */
    HWM_EVENT_PROPERTIES eventProperties;

    /* Things pertaining to condition of the object */
    VOLTAGE_INPUT_READING inputVoltageReading;
    VOLTAGE_INPUT_READING outputVoltageReading;
    VOLTAGE_INPUT_READING procBoardSupplyVoltageReading;
    UINT8       mainRegulatorCondition;
} MAIN_REGULATOR_STATUS, *MAIN_REGULATOR_STATUS_PTR;

/* Stuff that we know about the charger circuitry */
enum CHARGER_CONDITION_VALUE_ENUM
{
    CHARGER_CONDITION_UNKNOWN = 0,
    CHARGER_CONDITION_IDLE = 1,
    CHARGER_CONDITION_TRICKLE = 2,
    CHARGER_CONDITION_BULK = 3,
    CHARGER_CONDITION_OVER = 4,
    CHARGER_CONDITION_TOPOFF = 5
};

typedef struct CHARGER_STATUS_STRUCT
{
    /* Things pertaining to condition reporting of the object */
    HWM_EVENT_PROPERTIES eventProperties;

    /* Things pertaining to condition of the object */
    UINT8       chargerCondition;
} CHARGER_STATUS, *CHARGER_STATUS_PTR;

/* Stuff that we know about the buffer board */
typedef struct BUFFER_BOARD_STATUS_STRUCT
{
    /* Things pertaining to condition reporting of the object */
    HWM_EVENT_PROPERTIES eventProperties;

    /* Things pertaining to condition of the object */
    TEMPERATURE_STATUS temperatureStatus;
    BATTERY_STATUS batteryStatus;
    FUEL_GAUGE_STATUS fuelGaugeStatus;
    MAIN_REGULATOR_STATUS mainRegulatorStatus;
    CHARGER_STATUS chargerStatus;
    EEPROM_STATUS eepromStatus;
} BUFFER_BOARD_STATUS, *BUFFER_BOARD_STATUS_PTR;

/**************************************
** NVRAM battery
**************************************/

/* Stuff that we know about the NVRAM battery */
enum NVRAM_BATTERY_CONDITION_VALUE_ENUM
{
    NVRAM_BATTERY_CONDITION_UNKNOWN = 0,
    NVRAM_BATTERY_CONDITION_GOOD = 1,
    NVRAM_BATTERY_CONDITION_FAILED = 2
};

typedef struct NVRAM_BATTERY_STATUS_STRUCT
{
    /* Things pertaining to condition reporting of the object */
    HWM_EVENT_PROPERTIES eventProperties;

    /* Things pertaining to condition of the object */
    UINT8       nvramBatteryCondition;
} NVRAM_BATTERY_STATUS, *NVRAM_BATTERY_STATUS_PTR;

/**************************************
** CCB status
**************************************/
typedef struct CCB_STATUS_STRUCT
{
    /* Things pertaining to condition reporting of the object */
    HWM_EVENT_PROPERTIES eventProperties;

    /* Things pertaining to condition of the object */
    NVRAM_BATTERY_STATUS nvramBatteryStatus;
    EEPROM_STATUS ccbBoardEEPROMStatus;
    EEPROM_STATUS ccbMemoryModuleEEPROMStatus;
} CCB_STATUS, *CCB_STATUS_PTR;

/* I2C Status Definitions */
typedef struct HWM_STATUS_STRUCT
{
    /* Things pertaining to condition reporting of the object */
    HWM_EVENT_PROPERTIES eventProperties;

    /* Things pertaining to condition of the object */
    CCB_STATUS  ccbStatus;
    PROC_BOARD_STATUS procBoardStatus;
    POWER_SUPPLY_STATUS frontEndPowerSupply;
    POWER_SUPPLY_STATUS backEndPowerSupply;
    BUFFER_BOARD_STATUS frontEndBufferBoardStatus;
    BUFFER_BOARD_STATUS backEndBufferBoardStatus;
} HWM_STATUS;

/**************************************
** Hardware monitor data - updated on each pass thru the monitor
**************************************/
typedef struct HWM_DATA_STRUCT
{
    /* Things pertaining to condition reporting of the object */
    HWM_EVENT_PROPERTIES eventProperties;

    /* Things pertaining to condition of the object */
    HWM_STATUS  monitorStatus;
} HWM_DATA;

/**************************************
** Buffer board validation
**************************************/
enum HWM_BUFFER_VALIDATION_VALUE_ENUM
{
    BUFFER_VALIDATION_VALUE_UNKNOWN = 0,
    BUFFER_VALIDATION_VALUE_GOOD = 1,
    BUFFER_VALIDATION_VALUE_SHUTDOWN = 2,
    BUFFER_VALIDATION_VALUE_ERROR = 3
};

/**************************************
** Platform definitions
*/

enum
{
    PLATFORM_FLAG_DISABLE_BMC_NET = 1,
    PLATFORM_FLAG_CONFIG_BMC_NET = 2,
    PLATFORM_FLAG_NO_FIRST_SEL = 4,
    PLATFORM_FLAG_ALL_EVENTS = 8,
    PLATFORM_FLAG_LOCAL_RAID = 16,
};

typedef struct platform_t
{
    const char *name;           /* Platform name */
    UINT16      flags;          /* Platform flags */
    UINT8       cpu_fans;       /* Number of CPU fans */
    UINT8       case_fans;      /* Number of case fans */
    struct hw_mon_envdevice_internal **root_dev;
    void        (*analyze)(void);   /* Pointer to function to analyze sensors */
} HWM_PLATFORM;


/*****************************************************************************
** Public variables
*****************************************************************************/
extern const HWM_PLATFORM *hwm_platform;
extern HWM_DATA updateMonitorData;
extern HWM_DATA currentMonitorData;
extern void analyze_x7dwe(void);
extern void analyze_x6dh8_xg2(void);

/*****************************************************************************
** Public function prototypes
*****************************************************************************/
extern void HWM_ConfigureEthernet(void);
extern void HWM_MonitorTask(TASK_PARMS *parms);
extern UINT32 HWM_GetMonitorStatus(HWM_STATUS *monitorStatusPtr);
extern UINT32 HWM_MonitorDisable(void);
extern UINT32 HWM_AnalyzeMonitorData(HWM_DATA *current, HWM_DATA *previous);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _HWM_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
