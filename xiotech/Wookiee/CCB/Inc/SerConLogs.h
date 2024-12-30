/* $Id: SerConLogs.h 122127 2010-01-06 14:04:36Z m4 $ */
/*============================================================================
** FILE NAME:       SerConLogs.h
** MODULE TITLE:    Header file for SerConLogs.c
**
** DESCRIPTION:     Interface to display logs through the serial console
**
** Copyright (c) 2002-2009  Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _SERIAL_CONSOLE_LOGS_H_
#define _SERIAL_CONSOLE_LOGS_H_

#include "XIO_Types.h"
#include "SerCon.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public function prototypes
*****************************************************************************/
extern void SerialLogsFrameChoiceFunction(void);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _SERIAL_CONSOLE_LOGS_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
