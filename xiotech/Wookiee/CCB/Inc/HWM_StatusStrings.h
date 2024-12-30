/* $Id: HWM_StatusStrings.h 122127 2010-01-06 14:04:36Z m4 $ */
/*============================================================================
** FILE NAME:       HWM_StatusStrings.h
** MODULE TITLE:    Header file for HWM_StatusStrings.c
**
** Copyright (c) 2003-2009  Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _HWM_STATUS_STRINGS_H_
#define _HWM_STATUS_STRINGS_H_

#include "HWM.h"
#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public function prototypes
*****************************************************************************/
extern void HWM_GetStatusCodeString(char *stringPtr, HWM_STATUS_CODE statusCode,
                                    UINT8 stringLength);
extern void HWM_GetComponentIDString(char *stringPtr, UINT8 conditionValue,
                                     UINT8 stringLength);
extern UINT32 HWM_GetMonitorStatusString(char *stringPtr, HWM_STATUS *monitorStatusPtr,
                                         UINT8 stringLength);
extern UINT32 HWM_GetNVRAMBatteryStatusString(char *stringPtr,
                                              NVRAM_BATTERY_STATUS_PTR
                                              nvramBatteryStatusPtr, UINT8 stringLength);
extern UINT32 HWM_GetTemperatureStatusString(char *stringPtr,
                                             TEMPERATURE_STATUS_PTR temperatureStatusPtr,
                                             UINT8 stringLength);
extern UINT32 HWM_GetCCBStatusString(char *stringPtr, CCB_STATUS_PTR ccbStatusPtr,
                                     UINT8 stringLength);
extern UINT32 HWM_GetProcessorBoardStatusString(char *stringPtr,
                                                PROC_BOARD_STATUS_PTR procBoardStatusPtr,
                                                UINT8 stringLength);
extern UINT32 HWM_GetPowerSupplyVoltagesStatusString(char *stringPtr,
                                                     POWER_SUPPLY_VOLTAGES_STATUS_PTR
                                                     powerSupplyVoltagesStatusPtr,
                                                     UINT8 stringLength);
extern UINT32 HWM_GetProcessorStatusString(char *stringPtr,
                                           PROC_BOARD_PROCESSOR_STATUS_PTR
                                           processorStatusPtr, UINT8 stringLength);
extern UINT32 HWM_GetPowerSupplyStatusString(char *stringPtr,
                                             POWER_SUPPLY_STATUS_PTR powerSupplyStatusPtr,
                                             UINT8 stringLength);
extern UINT32 HWM_GetBufferBoardStatusString(char *stringPtr,
                                             BUFFER_BOARD_STATUS_PTR bufferBoardStatusPtr,
                                             UINT8 stringLength);
extern UINT32 HWM_GetBatteryStatusString(char *stringPtr,
                                         BATTERY_STATUS_PTR batteryStatusPtr,
                                         UINT8 stringLength);
extern UINT32 HWM_GetChargerStatusString(char *stringPtr,
                                         CHARGER_STATUS_PTR chargerStatusPtr,
                                         UINT8 stringLength);
extern UINT32 HWM_GetFuelGaugeStatusString(char *stringPtr,
                                           FUEL_GAUGE_STATUS_PTR fuelGaugeStatusPtr,
                                           UINT8 stringLength);
extern UINT32 HWM_GetMainRegulatorStatusString(char *stringPtr,
                                               MAIN_REGULATOR_STATUS_PTR
                                               mainRegulatorStatusPtr,
                                               UINT8 stringLength);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _HWM_STATUS_STRINGS_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
