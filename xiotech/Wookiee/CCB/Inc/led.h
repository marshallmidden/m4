/* $Id: led.h 122127 2010-01-06 14:04:36Z m4 $ */
/*============================================================================
** FILE NAME:       led.h
** MODULE TITLE:    Header file for led.c
**
** Copyright (c) 2001-2009 Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef LED_H
#define LED_H

#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public function prototypes
*****************************************************************************/

/*****************************************************************************
** FUNCTION NAME: LEDSetBit
**
** PARAMETERS:  whichBit - (0 to 7)
**
** DESCRIPTION:
**
** RETURNS:     Nothing
******************************************************************************/
extern void LEDSetBit(UINT8 whichBit);

/*****************************************************************************
** FUNCTION NAME: LEDClearBit
**
** PARAMETERS:  whichBit - (0 to 7)
**
** DESCRIPTION:
**
** RETURNS:     Nothing
******************************************************************************/
extern void LEDClearBit(UINT8 whichBit);

/*****************************************************************************
** FUNCTION NAME: LEDClearCode
******************************************************************************/
extern void LEDClearCode(void);

/*****************************************************************************
** FUNCTION NAME: LEDSetAttention
**
** PARAMETERS:  ledSetting - (TRUE = Turn LED on, FALSE = Turn LED off)
**
** DESCRIPTION:
**
** RETURNS:     Nothing
******************************************************************************/
extern void LEDSetAttention(UINT8 ledSetting);

/*****************************************************************************
** FUNCTION NAME: LEDSetHeartbeat
**
** PARAMETERS:  ledSetting
**                TRUE  - Turn LED on
**                FALSE - Turn LED off
**
** DESCRIPTION:
**
** RETURNS:     Nothing
******************************************************************************/
extern void LEDSetHeartbeat(UINT8 ledSetting);

/*****************************************************************************
** FUNCTION NAME: LEDSetCommFault
**
** PARAMETERS:  ledSetting - (TRUE = Turn LED on, FALSE = Turn LED off)
**
** DESCRIPTION:
**
** RETURNS:     Nothing
******************************************************************************/
extern void LEDSetCommFault(UINT8 ledSetting);

/*****************************************************************************
** FUNCTION NAME: LEDSetOffline
**
** PARAMETERS:  ledSetting
**                TRUE  - Turn LED on
**                FALSE - Turn LED off
**
** DESCRIPTION:
**
** RETURNS:     Nothing
******************************************************************************/
extern void LEDSetOffline(UINT8 ledSetting);

/*****************************************************************************
** Public variables
*****************************************************************************/
extern UINT32 GPOD;             /* Emulate the i960 GPOD register */


#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* LED_H */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
