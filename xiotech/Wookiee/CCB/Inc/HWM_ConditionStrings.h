/* $Id: HWM_ConditionStrings.h 122127 2010-01-06 14:04:36Z m4 $ */
/*============================================================================
** FILE NAME:       HWM_ConditionStrings.h
** MODULE TITLE:    Header file for HWM_ConditionStrings.c
**
** Copyright (c) 2003,2009  Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _HWM_CONDITION_STRINGS_H_
#define _HWM_CONDITION_STRINGS_H_

#include "HWM.h"
#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public function prototypes
*****************************************************************************/
extern UINT32 HWM_GetLimitMonitorConditionString(char *stringPtr, UINT8 conditionValue,
                                                 UINT8 stringLength);
extern UINT32 HWM_GetTemperatureConditionString(char *stringPtr, UINT8 conditionValue,
                                                UINT8 stringLength);
extern UINT32 HWM_GetPowerSupplyConditionString(char *stringPtr, UINT8 conditionValue,
                                                UINT8 stringLength);
extern UINT32 HWM_GetCoolingFanConditionString(char *stringPtr, UINT8 conditionValue,
                                               UINT8 stringLength);
extern UINT32 HWM_GetProcessorResetConditionString(char *stringPtr, UINT8 conditionValue,
                                                   UINT8 stringLength);
extern UINT32 HWM_GetBatteryConditionString(char *stringPtr, UINT8 conditionValue,
                                            UINT8 stringLength);
extern UINT32 HWM_GetCurrentFlowConditionString(char *stringPtr, UINT8 conditionValue,
                                                UINT8 stringLength);
extern UINT32 HWM_GetFuelGaugeConditionString(char *stringPtr, UINT8 conditionValue,
                                              UINT8 stringLength);
extern UINT32 HWM_GetMainRegulatorConditionString(char *stringPtr, UINT8 conditionValue,
                                                  UINT8 stringLength);
extern UINT32 HWM_GetChargerConditionString(char *stringPtr, UINT8 conditionValue,
                                            UINT8 stringLength);
extern UINT32 HWM_GetNVRAMBatteryConditionString(char *stringPtr, UINT8 conditionValue,
                                                 UINT8 stringLength);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _HWM_CONDITION_STRINGS_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
