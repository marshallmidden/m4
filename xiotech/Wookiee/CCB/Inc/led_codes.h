/* $Id: led_codes.h 122127 2010-01-06 14:04:36Z m4 $ */
/******************************************************************************
** NAME:        led_codes.h
**
** PURPOSE:     Define the LED error codes.
**
** NOTE:        Any changes to this file MUST also be made in led_codes.inc
**
** Copyright (c) 2001-2009 Xiotech Corporation.  All rights reserved.
******************************************************************************/

/******************************************************************************
** Include files
******************************************************************************/
#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/******************************************************************************
** Public defines
******************************************************************************/

/*
** LED display bitfield definitions
*/
#define LED_CONSOLE_CONNECTION  3

/*
** General definitions
*/
#define LED_ALL_OFF             0x00

/********************************************************************
** Begin error codes used in the runtime code base
*********************************************************************
** RUNTIME   - LEVELS 8x through Dx EVENT CODES
** NOTE: These error codes MUST match the definitions in led_codes.inc
*/
#define EVENT_KERNEL_ERROR      0x80    /* Kernel reported error                */

/*
** Error codes start at <PROCERRB> to allow for kernel error codes
*/
#define PROCERRB                0xA0    /* Give kernel 32 error codes           */

/*
** Process-level error codes -----------------------------------------------
*/
#define LED_CODE_ERR08          (PROCERRB + 8)  /* L$que: invalid function number           */

#define LED_CODE_ERR11          (PROCERRB + 11) /* l$exec_irp: invalid function number      */

#ifdef __cplusplus
#pragma pack(pop)
#endif

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
