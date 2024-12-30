/* $Id: PI_BatteryBoard.h 122127 2010-01-06 14:04:36Z m4 $*/
/**
******************************************************************************
**
**  @file       PI_BatteryBoard.c
**
**  @brief      Packet Interface and miscellaneous functions for
**              Battery Board Commands
**
**  These functions battery board requests.
**
**  Copyright (c) 2001 - 2004 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef __PI_BATTERYBOARD_H__
#define __PI_BATTERYBOARD_H__

#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern UINT8 BatteryHealthState(void);
extern void BatteryHealthTaskStart(UINT8 state);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* __PI_BATTERYBOARD_H__ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
