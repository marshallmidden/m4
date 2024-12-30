/* $Id: HWM_Happy.c 122127 2010-01-06 14:04:36Z m4 $ */
/*============================================================================
** FILE NAME:       HWM_Happy.c
** MODULE TITLE:    Happy hardware monitor initialization impementation
**
** DESCRIPTION:     Hardware monitor initialization data
**
** Copyright (c) 2008-2009 Xiotech Corporation. All rights reserved.
**==========================================================================*/
#include "HWM.h"

#include "debug_files.h"
#include "XIO_Std.h"
#include "XIO_Const.h"

/*****************************************************************************
** Public variables externed in header file
*****************************************************************************/
HWM_DATA updateMonitorData =
{
    .eventProperties = {
        .statusCode = STATUS_CODE_VALID,
        .componentID = HWM_OVERALL_ID,
        .monitorStatistics = {
            .eventCounterWarning = 0,
            .eventCounterError = 0
        },
        .flags = {
            .value = FALSE
        },
        .description = { 0 }
    },

    .monitorStatus = {
        .eventProperties = {
            .statusCode = STATUS_CODE_VALID,
            .componentID = HARDWARE_MONITOR_STATUS_ID,
            .monitorStatistics = {
                .eventCounterWarning = 0,
                .eventCounterError = 0
            },
            .flags = {
                .value = FALSE
            },
            .description = { "NO_EVENT" }
        },

        .ccbStatus = {
            .eventProperties = {
                .statusCode = STATUS_CODE_VALID,
                .componentID = CCB_BOARD_STATUS_ID,
                .monitorStatistics = {
                    .eventCounterWarning = 0,
                    .eventCounterError = 0
                },
                .flags = {
                    .value = FALSE
                },
                .description = { "NO_EVENT" }
            },

            .nvramBatteryStatus = {
                .eventProperties = {
                    .statusCode = STATUS_CODE_VALID,
                    .componentID = CCB_NVRAM_BATTERY_STATUS_ID,
                    .monitorStatistics = {
                        .eventCounterWarning = 0,
                        .eventCounterError = 0
                    },
                    .flags = {
                        .value = FALSE
                    },
                    .description = { "NO_EVENT" }
                },
                .nvramBatteryCondition = NVRAM_BATTERY_CONDITION_GOOD
            },

            .ccbBoardEEPROMStatus = {
                .eventProperties = {
                    .statusCode = STATUS_CODE_VALID,
                    .componentID = CCB_BOARD_EEPROM_STATUS_ID,
                    .monitorStatistics = {
                        .eventCounterWarning = 0,
                        .eventCounterError = 0
                    },
                    .flags = {
                        .value = FALSE
                    },
                    .description = { "NO_EVENT" }
                },
                .eepromCondition = STATUS_CODE_NOT_PRESENT,
                .xciData = {
                    .manuJedecId = { 0 },
                    .manuLocation = 0,
                    .manuPartNumber = {
                        .modulePartNumber = { 0 },
                        .moduleDashNumber = { 0 },
                        .moduleRevisionLetters = { 0 },
                        .reserved = { 0 }
                    },
                    .revisionCode = { 0 },
                    .manuYear = 0,
                    .manuWeek = 0,
                    .asmSerialNumber = 0,
                    .manuSpecificData = {
                        .reserved = { 0 },
                        .crc = 0
                    },
                    .vendorSpecific = { 0 }
                }
            },

            .ccbMemoryModuleEEPROMStatus = {
                .eventProperties = {
                    .statusCode = STATUS_CODE_VALID,
                    .componentID = CCB_MEMORY_MODULE_EEPROM_STATUS_ID,
                    .monitorStatistics = {
                        .eventCounterWarning = 0,
                        .eventCounterError = 0
                    },
                    .flags = {
                        .value = FALSE
                    },
                    .description = { "NO_EVENT" }
                },
                .eepromCondition = STATUS_CODE_NOT_PRESENT,
                .xciData = {
                    .manuJedecId = { 0 },
                    .manuLocation = 0,
                    .manuPartNumber = {
                        .modulePartNumber = { 0 },
                        .moduleDashNumber = { 0 },
                        .moduleRevisionLetters = { 0 },
                        .reserved = { 0 }
                    },
                    .revisionCode = { 0 },
                    .manuYear = 0,
                    .manuWeek = 0,
                    .asmSerialNumber = 0,
                    .manuSpecificData = {
                        .reserved = { 0 },
                        .crc = 0
                    },
                    .vendorSpecific = { 0 }
                }
            }
        },

        .procBoardStatus = {
            .eventProperties = {
                .statusCode = STATUS_CODE_VALID,
                .componentID = PROC_BOARD_STATUS_ID,
                .monitorStatistics = {
                    .eventCounterWarning = 0,
                    .eventCounterError = 0
                },
                .flags = {
                    .value = FALSE
                },
                .description = { "NO_EVENT" }
            },

            .powerSupplyVoltagesStatus = {
                .eventProperties = {
                    .statusCode = STATUS_CODE_VALID,
                    .componentID = POWER_SUPPLY_VOLTAGES_STATUS_ID,
                    .monitorStatistics = {
                        .eventCounterWarning = 0,
                        .eventCounterError = 0
                    },
                    .flags = {
                        .value = FALSE
                    },
                    .description = { "NO_EVENT" }
                },

                .twelveVoltReading = {
                    .maximumMillivolts = 0x3177,
                    .currentMillivolts = 0x30f9,
                    .minimumMillivolts = 0x30f9,
                    .limitMonitorValue = LIMIT_MONITOR_GOOD
                },

                .fiveVoltReading = {
                    .maximumMillivolts = 0x1402,
                    .currentMillivolts = 0x1402,
                    .minimumMillivolts = 0x1402,
                    .limitMonitorValue = LIMIT_MONITOR_GOOD
                },

                .threePointThreeVoltReading = {
                    .maximumMillivolts = 0xd2b,
                    .currentMillivolts = 0xd2b,
                    .minimumMillivolts = 0xd2b,
                    .limitMonitorValue = LIMIT_MONITOR_GOOD
                },

                .standbyVoltageReading = {
                    .maximumMillivolts = 0xd4d,
                    .currentMillivolts = 0xd4d,
                    .minimumMillivolts = 0xd4d,
                    .limitMonitorValue = LIMIT_MONITOR_GOOD
                }
            },

            .frontEndProcessorStatus = {
                .eventProperties = {
                    .statusCode = STATUS_CODE_VALID,
                    .componentID = FE_PROCESSOR_STATUS_ID,
                    .monitorStatistics = {
                        .eventCounterWarning = 0,
                        .eventCounterError = 0
                    },
                    .flags = {
                        .value = FALSE
                    },
                    .description = { "NO_EVENT" }
                },

                .temperatureStatus = {
                    .eventProperties = {
                        .statusCode = STATUS_CODE_VALID,
                        .componentID = FE_PROCESSOR_TEMPERATURE_STATUS_ID,
                        .monitorStatistics = {
                            .eventCounterWarning = 0,
                            .eventCounterError = 0
                        },
                        .flags = {
                            .value = FALSE
                        },
                        .description = { "91 dF (IS OK NOW)" }
                    },

                    .maximumDegreesCelsius = 0x21,
                    .currentDegreesCelsius = 0x21,
                    .minimumDegreesCelsius = 0x20,
                    .conditionValue = TEMPERATURE_CONDITION_NORMAL
                },

                .memorySocketSupplyVoltageReading = {
                    .maximumMillivolts = 0,
                    .currentMillivolts = 0,
                    .minimumMillivolts = -1,
                    .limitMonitorValue = LIMIT_MONITOR_UNKNOWN
                },

                .processorResetConditionValue = PROCESSOR_RESET_CONDITION_RUNNING
            },

            .backEndProcessorStatus = {
                .eventProperties = {
                    .statusCode = STATUS_CODE_VALID,
                    .componentID = BE_PROCESSOR_STATUS_ID,
                    .monitorStatistics = {
                        .eventCounterWarning = 0,
                        .eventCounterError = 0
                    },
                    .flags = {
                        .value = FALSE
                    },
                    .description = { "NO_EVENT" }
                },

                .temperatureStatus = {
                    .eventProperties = {
                        .statusCode = STATUS_CODE_VALID,
                        .componentID = BE_PROCESSOR_TEMPERATURE_STATUS_ID,
                        .monitorStatistics = {
                            .eventCounterWarning = 0,
                            .eventCounterError = 0
                        },
                        .flags = {
                            .value = FALSE
                        },
                        .description = { "91 dF (IS OK NOW)" }
                    },

                    .maximumDegreesCelsius = 0x21,
                    .currentDegreesCelsius = 0x21,
                    .minimumDegreesCelsius = 0x20,
                    .conditionValue = TEMPERATURE_CONDITION_NORMAL
                },

                .memorySocketSupplyVoltageReading = {
                    .maximumMillivolts = 0,
                    .currentMillivolts = 0,
                    .minimumMillivolts = -1,
                    .limitMonitorValue = LIMIT_MONITOR_UNKNOWN
                },

                .processorResetConditionValue = PROCESSOR_RESET_CONDITION_RUNNING
            },

            .chassisEEPROMStatus = {
                .eventProperties = {
                    .statusCode = STATUS_CODE_VALID,
                    .componentID = CHASSIS_EEPROM_STATUS_ID,
                    .monitorStatistics = {
                        .eventCounterWarning = 0,
                        .eventCounterError = 0
                    },
                    .flags = {
                        .value = FALSE
                    },
                    .description = { "NO_EVENT" }
                },

                .eepromCondition = STATUS_CODE_NOT_PRESENT,
                .xciData = {
                    .manuJedecId = { 0 },
                    .manuLocation = 0,
                    .manuPartNumber = {
                        .modulePartNumber = { 0 },
                        .moduleDashNumber = { 0 },
                        .moduleRevisionLetters = { 0 },
                        .reserved = { 0 }
                    },
                    .revisionCode = { 0 },
                    .manuYear = 0,
                    .manuWeek = 0,
                    .asmSerialNumber = 0,
                    .manuSpecificData = {
                        .reserved = { 0 },
                        .crc = 0
                    },
                    .vendorSpecific = { 0 }
                }
            },

            .procBoardEEPROMStatus = {
                .eventProperties = {
                    .statusCode = STATUS_CODE_VALID,
                    .componentID = PROC_BOARD_EEPROM_STATUS_ID,
                    .monitorStatistics = {
                        .eventCounterWarning = 0,
                        .eventCounterError = 0
                    },
                    .flags = {
                        .value = FALSE
                    },
                    .description = { "NO_EVENT" }
                },

                .eepromCondition = STATUS_CODE_NOT_PRESENT,
                .xciData = {
                    .manuJedecId = { 0 },
                    .manuLocation = 0,
                    .manuPartNumber = {
                        .modulePartNumber = { 0 },
                        .moduleDashNumber = { 0 },
                        .moduleRevisionLetters = { 0 },
                        .reserved = { 0 }
                    },
                    .revisionCode = { 0 },
                    .manuYear = 0,
                    .manuWeek = 0,
                    .asmSerialNumber = 0,
                    .manuSpecificData = {
                        .reserved = { 0 },
                        .crc = 0
                    },
                    .vendorSpecific = { 0 }
                }
            }
        },

        .frontEndPowerSupply = {
            .eventProperties = {
                .statusCode = STATUS_CODE_VALID,
                .componentID = FE_POWER_SUPPLY_STATUS_ID,
                .monitorStatistics = {
                    .eventCounterWarning = 0,
                    .eventCounterError = 0
                },
                .flags = {
                    .value = FALSE
                },
                .description = { "NO_EVENT" }
            },

            .powerSupplyCondition = {
                .acFailedSampleCounter = AC_FAILED_VALID_SAMPLE_COUNT,
                .dcFailedSampleCounter = DC_FAILED_VALID_SAMPLE_COUNT,
                .value = POWER_SUPPLY_CONDITION_GOOD,
            },

            .coolingFanConditionValue = COOLING_FAN_CONDITION_GOOD,

            .assemblyEEPROMStatus = {
                .eventProperties = {
                    .statusCode = STATUS_CODE_VALID,
                    .componentID = FE_POWER_SUPPLY_ASM_EEPROM_STATUS_ID,
                    .monitorStatistics = {
                        .eventCounterWarning = 0,
                        .eventCounterError = 0
                    },
                    .flags = {
                        .value = FALSE
                    },
                    .description = { 0 }
                },

                .eepromCondition = STATUS_CODE_NOT_PRESENT,
                .xciData = {
                    .manuJedecId = { 0 },
                    .manuLocation = 0,
                    .manuPartNumber = {
                        .modulePartNumber = { 0 },
                        .moduleDashNumber = { 0 },
                        .moduleRevisionLetters = { 0 },
                        .reserved = { 0 }
                    },
                    .revisionCode = { 0 },
                    .manuYear = 0,
                    .manuWeek = 0,
                    .asmSerialNumber = 0,
                    .manuSpecificData = {
                        .reserved = { 0 },
                        .crc = 0
                    },
                    .vendorSpecific = { 0 }
                }
            },

            .interfaceEEPROMStatus = {
                .eventProperties = {
                    .statusCode = STATUS_CODE_VALID,
                    .componentID = FE_POWER_SUPPLY_INT_EEPROM_STATUS_ID,
                    .monitorStatistics = {
                        .eventCounterWarning = 0,
                        .eventCounterError = 0
                    },
                    .flags = {
                        .value = FALSE
                    },
                    .description = { 0 }
                },

                .eepromCondition = STATUS_CODE_NOT_PRESENT,
                .xciData = {
                    .manuJedecId = { 0 },
                    .manuLocation = 0,
                    .manuPartNumber = {
                        .modulePartNumber = { 0 },
                        .moduleDashNumber = { 0 },
                        .moduleRevisionLetters = { 0 },
                        .reserved = { 0 }
                    },
                    .revisionCode = { 0 },
                    .manuYear = 0,
                    .manuWeek = 0,
                    .asmSerialNumber = 0,
                    .manuSpecificData = {
                        .reserved = { 0 },
                        .crc = 0
                    },
                    .vendorSpecific = { 0 }
                }
            }
        },

        .backEndPowerSupply = {
            .eventProperties = {
                .statusCode = STATUS_CODE_VALID,
                .componentID = BE_POWER_SUPPLY_STATUS_ID,

                .monitorStatistics = {
                    .eventCounterWarning = 0,
                    .eventCounterError = 0
                },
                .flags = {
                    .value = FALSE
                },
                .description = { "NO_EVENT" }
            },

            .powerSupplyCondition = {
                .acFailedSampleCounter = AC_FAILED_VALID_SAMPLE_COUNT,
                .dcFailedSampleCounter = DC_FAILED_VALID_SAMPLE_COUNT,
                .value = POWER_SUPPLY_CONDITION_GOOD,
            },

            .coolingFanConditionValue = COOLING_FAN_CONDITION_GOOD,

            .assemblyEEPROMStatus = {
                .eventProperties = {
                    .statusCode = STATUS_CODE_VALID,
                    .componentID = BE_POWER_SUPPLY_ASM_EEPROM_STATUS_ID,
                    .monitorStatistics = {
                        .eventCounterWarning = 0,
                        .eventCounterError = 0
                    },
                    .flags = {
                        .value = FALSE
                    },
                    .description = { 0 }
                },

                .eepromCondition = STATUS_CODE_NOT_PRESENT,
                .xciData = {
                    .manuJedecId = { 0 },
                    .manuLocation = 0,
                    .manuPartNumber = {
                        .modulePartNumber = { 0 },
                        .moduleDashNumber = { 0 },
                        .moduleRevisionLetters = { 0 },
                        .reserved = { 0 }
                    },
                    .revisionCode = { 0 },
                    .manuYear = 0,
                    .manuWeek = 0,
                    .asmSerialNumber = 0,
                    .manuSpecificData = {
                        .reserved = { 0 },
                        .crc = 0
                    },
                    .vendorSpecific = { 0 }
                }
            },

            .interfaceEEPROMStatus = {
                .eventProperties = {
                    .statusCode = STATUS_CODE_VALID,
                    .componentID = BE_POWER_SUPPLY_INT_EEPROM_STATUS_ID,
                    .monitorStatistics = {
                        .eventCounterWarning = 0,
                        .eventCounterError = 0
                    },
                    .flags = {
                        .value = FALSE
                    },
                    .description = { 0 }
                },

                .eepromCondition = STATUS_CODE_NOT_PRESENT,
                .xciData = {
                    .manuJedecId = { 0 },
                    .manuLocation = 0,
                    .manuPartNumber = {
                        .modulePartNumber = { 0 },
                        .moduleDashNumber = { 0 },
                        .moduleRevisionLetters = { 0 },
                        .reserved = { 0 }
                    },
                    .revisionCode = { 0 },
                    .manuYear = 0,
                    .manuWeek = 0,
                    .asmSerialNumber = 0,
                    .manuSpecificData = {
                        .reserved = { 0 },
                        .crc = 0
                    },
                    .vendorSpecific = { 0 }
                }
            }
        },

        .frontEndBufferBoardStatus = {
            .eventProperties = {
                .statusCode = STATUS_CODE_VALID,
                .componentID = FE_BUFFER_BOARD_STATUS_ID,
                .monitorStatistics = {
                    .eventCounterWarning = 0,
                    .eventCounterError = 0
                },
                .flags = {
                    .value = FALSE
                },
                .description = { "NO_EVENT" }
            },

            .temperatureStatus = {
                .eventProperties = {
                    .statusCode = STATUS_CODE_VALID,
                    .componentID = FE_BUFFER_BOARD_TEMPERATURE_STATUS_ID,
                    .monitorStatistics = {
                        .eventCounterWarning = 0,
                        .eventCounterError = 0
                    },
                    .flags = {
                        .value = FALSE
                    },
                    .description = { 0 }
                },

                .maximumDegreesCelsius = 0x32,
                .currentDegreesCelsius = 0x2c,
                .minimumDegreesCelsius = 0x2b,
                .conditionValue = TEMPERATURE_CONDITION_UNKNOWN
            },

            .batteryStatus = {
                .eventProperties = {
                    .statusCode = STATUS_CODE_VALID,
                    .componentID = FE_BUFFER_BOARD_BATTERY_STATUS_ID,
                    .monitorStatistics = {
                        .eventCounterWarning = 0,
                        .eventCounterError = 0
                    },
                    .flags = {
                        .value = FALSE
                    },
                    .description = { "NO_EVENT" }
                },

                .terminalVoltageReading = {
                    .maximumMillivolts = 0x1034,
                    .currentMillivolts = 0x1034,
                    .minimumMillivolts = 0x1034,
                    .limitMonitorValue = LIMIT_MONITOR_GOOD
                },

                .batteryCondition = BATTERY_CONDITION_UNKNOWN
            },

            .fuelGaugeStatus = {
                .eventProperties = {
                    .statusCode = STATUS_CODE_VALID,
                    .componentID = FE_BUFFER_BOARD_FUEL_GAUGE_STATUS_ID,
                    .monitorStatistics = {
                        .eventCounterWarning = 0,
                        .eventCounterError = 0
                    },
                    .flags = {
                        .value = FALSE
                    },
                    .description = { "NO_EVENT" }
                },

                .currentFlowRate = 0,

                .regulatorOutputVoltageReading = {
                    .maximumMillivolts = 0,
                    .currentMillivolts = 0,
                    .minimumMillivolts = -1,
                    .limitMonitorValue = LIMIT_MONITOR_UNKNOWN
                },

                .fuelGaugeCondition = FUEL_GAUGE_CONDITION_GOOD,
                .currentFlowCondition = CURRENT_FLOW_CONDITION_GOOD
            },

            .mainRegulatorStatus = {
                .eventProperties = {
                    .statusCode = STATUS_CODE_VALID,
                    .componentID = FE_BUFFER_BOARD_MAIN_REGULATOR_STATUS_ID,
                    .monitorStatistics = {
                        .eventCounterWarning = 0,
                        .eventCounterError = 0
                    },
                    .flags = {
                        .value = FALSE
                    },
                    .description = { "NO_EVENT" }
                },

                .inputVoltageReading = {
                    .maximumMillivolts = 0,
                    .currentMillivolts = 0,
                    .minimumMillivolts = -1,
                    .limitMonitorValue = LIMIT_MONITOR_UNKNOWN
                },

                .outputVoltageReading = {
                    .maximumMillivolts = 0,
                    .currentMillivolts = 0,
                    .minimumMillivolts = -1,
                    .limitMonitorValue = LIMIT_MONITOR_UNKNOWN
                },

                .procBoardSupplyVoltageReading = {
                    .maximumMillivolts = 0,
                    .currentMillivolts = 0,
                    .minimumMillivolts = -1,
                    LIMIT_MONITOR_UNKNOWN
                },

                .mainRegulatorCondition = MAIN_REGULATOR_CONDITION_UNKNOWN
            },

            .chargerStatus = {
                .eventProperties = {
                    .statusCode = STATUS_CODE_VALID,
                    .componentID = FE_BUFFER_BOARD_CHARGER_STATUS_ID,
                    .monitorStatistics = {
                        .eventCounterWarning = 0,
                        .eventCounterError = 0
                    },
                    .flags = {
                        .value = FALSE
                    },
                    .description = { "NO_EVENT" }
                },

                .chargerCondition = CHARGER_CONDITION_UNKNOWN
            },

            .eepromStatus = {
                .eventProperties = {
                    .statusCode = STATUS_CODE_VALID,
                    .componentID = FE_BUFFER_BOARD_EEPROM_STATUS_ID,
                    .monitorStatistics = {
                        .eventCounterWarning = 0,
                        .eventCounterError = 0
                    },
                    .flags = {
                        .value = FALSE
                    },
                    .description = { "NO_EVENT" }
                },

                .eepromCondition = STATUS_CODE_NOT_PRESENT,
                .xciData = {
                    .manuJedecId = { 0 },
                    .manuLocation = 0,
                    .manuPartNumber = {
                        .modulePartNumber = { 0 },
                        .moduleDashNumber = { 0 },
                        .moduleRevisionLetters = { 0 },
                        .reserved = { 0 }
                    },
                    .revisionCode = { 0 },
                    .manuYear = 0,
                    .manuWeek = 0,
                    .asmSerialNumber = 0,
                    .manuSpecificData = {
                        .reserved = { 0 },
                        .crc = 0
                    },
                    .vendorSpecific = { 0 }
                }
            }
        },

        .backEndBufferBoardStatus = {
            .eventProperties = {
                .statusCode = STATUS_CODE_VALID,
                .componentID = BE_BUFFER_BOARD_STATUS_ID,
                .monitorStatistics = {
                    .eventCounterWarning = 0,
                    .eventCounterError = 0
                },
                .flags = {
                    .value = FALSE
                },
                .description = { "NO_EVENT" }
            },

            .temperatureStatus = {
                .eventProperties = {
                    .statusCode = STATUS_CODE_VALID,
                    .componentID = BE_BUFFER_BOARD_TEMPERATURE_STATUS_ID,
                    .monitorStatistics = {
                        .eventCounterWarning = 0,
                        .eventCounterError = 0
                    },
                    .flags = {
                        .value = FALSE
                    },
                    .description = { 0 }
                },

                .maximumDegreesCelsius = 0x2c,
                .currentDegreesCelsius = 0x2b,
                .minimumDegreesCelsius = 0x2b,
                .conditionValue = TEMPERATURE_CONDITION_UNKNOWN
            },

            .batteryStatus = {
                .eventProperties = {
                    .statusCode = STATUS_CODE_VALID,
                    .componentID = BE_BUFFER_BOARD_BATTERY_STATUS_ID,
                    .monitorStatistics = {
                        .eventCounterWarning = 0,
                        .eventCounterError = 0
                    },
                    .flags = {
                        .value = FALSE
                    },
                    .description = { "NO_EVENT" }
                },

                .terminalVoltageReading = {
                    .maximumMillivolts = 0x1034,
                    .currentMillivolts = 0x1034,
                    .minimumMillivolts = 0x1034,
                    .limitMonitorValue = LIMIT_MONITOR_GOOD
                },

                .batteryCondition = BATTERY_CONDITION_UNKNOWN
            },

            .fuelGaugeStatus = {
                .eventProperties = {
                    .statusCode = STATUS_CODE_VALID,
                    .componentID = BE_BUFFER_BOARD_FUEL_GAUGE_STATUS_ID,
                    .monitorStatistics = {
                        .eventCounterWarning = 0,
                        .eventCounterError = 0
                    },
                    .flags = {
                        .value = FALSE
                    },
                    .description = { "NO_EVENT" }
                },

                .currentFlowRate = 0,

                .regulatorOutputVoltageReading = {
                    .maximumMillivolts = 0,
                    .currentMillivolts = 0,
                    .minimumMillivolts = -1,
                    .limitMonitorValue = LIMIT_MONITOR_UNKNOWN
                },

                .fuelGaugeCondition = FUEL_GAUGE_CONDITION_GOOD,
                .currentFlowCondition = CURRENT_FLOW_CONDITION_GOOD
            },

            .mainRegulatorStatus = {
                .eventProperties = {
                    .statusCode = STATUS_CODE_VALID,
                    .componentID = BE_BUFFER_BOARD_MAIN_REGULATOR_STATUS_ID,
                    .monitorStatistics = {
                        .eventCounterWarning = 0,
                        .eventCounterError = 0
                    },
                    .flags = {
                        .value = FALSE
                    },
                    .description = { "NO_EVENT" }
                },

                .inputVoltageReading = {
                    .maximumMillivolts = 0,
                    .currentMillivolts = 0,
                    .minimumMillivolts = -1,
                    .limitMonitorValue = LIMIT_MONITOR_UNKNOWN
                },

                .outputVoltageReading = {
                    .maximumMillivolts = 0,
                    .currentMillivolts = 0,
                    .minimumMillivolts = -1,
                    .limitMonitorValue = LIMIT_MONITOR_UNKNOWN
                },

                .procBoardSupplyVoltageReading = {
                    .maximumMillivolts = 0,
                    .currentMillivolts = 0,
                    .minimumMillivolts = -1,
                    .limitMonitorValue = LIMIT_MONITOR_UNKNOWN
                },

                .mainRegulatorCondition = MAIN_REGULATOR_CONDITION_UNKNOWN
            },

            .chargerStatus = {
                .eventProperties = {
                    .statusCode = STATUS_CODE_VALID,
                    .componentID = BE_BUFFER_BOARD_CHARGER_STATUS_ID,
                    .monitorStatistics = {
                        .eventCounterWarning = 0,
                        .eventCounterError = 0
                    },
                    .flags = {
                        .value = FALSE
                    },
                    .description = { "NO_EVENT" }
                },

                .chargerCondition = CHARGER_CONDITION_UNKNOWN
            },

            .eepromStatus = {
                .eventProperties = {
                    .statusCode = STATUS_CODE_VALID,
                    .componentID = BE_BUFFER_BOARD_EEPROM_STATUS_ID,
                    .monitorStatistics = {
                        .eventCounterWarning = 0,
                        .eventCounterError = 0
                    },
                    .flags = {
                        .value = FALSE
                    },
                    .description = { "NO_EVENT" }
                },

                .eepromCondition = STATUS_CODE_NOT_PRESENT,
                .xciData = {
                    .manuJedecId = { 0 },
                    .manuLocation = 0,
                    .manuPartNumber = {
                        .modulePartNumber = { 0 },
                        .moduleDashNumber = { 0 },
                        .moduleRevisionLetters = { 0 },
                        .reserved = { 0 }
                    },
                    .revisionCode = { 0 },
                    .manuYear = 0,
                    .manuWeek = 0,
                    .asmSerialNumber = 0,
                    .manuSpecificData = {
                        .reserved = { 0 },
                        .crc = 0
                    },
                    .vendorSpecific = { 0 }
                }
            }
        }
    }
};

/*****************************************************************************
** Code Start
*****************************************************************************/

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
