/* $Id: SerConNetwork.h 122127 2010-01-06 14:04:36Z m4 $ */
/*============================================================================
** FILE NAME:       SerConNetwork.h
** MODULE TITLE:    Header file for serial_console_network.c
**
** Copyright (c) 2001-2009  Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _SERIAL_CONSOLE_NETWORK_H_
#define _SERIAL_CONSOLE_NETWORK_H_

#include "XIO_Types.h"
#include "SerCon.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public function prototypes
*****************************************************************************/
extern unsigned char ShowEthernet(unsigned char LineNumber);
extern void ControllerSetupFrameChoiceFunction(void);
extern void NetstatDisplayFunction(void);
extern void SerSetupCtrlClean(void);

extern void ToggleDiagPortsChoiceFunction(void);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _SERIAL_CONSOLE_NETWORK_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
