/* $Id: serial_num.h 122127 2010-01-06 14:04:36Z m4 $ */
/*============================================================================
** FILE NAME:       serial_num.h
** MODULE TITLE:    Header file for serial_num.c
**
** DESCRIPTION:     Routines to Get & Set the system serial number.
**
** Copyright (c) 2001-2009 Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _SERIAL_NUM_H_
#define _SERIAL_NUM_H_

#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/
#define CONTROLLER_SN   1
#define SYSTEM_SN       2

/*****************************************************************************
** Public function prototypes
*****************************************************************************/
extern UINT32 GetSerialNumber(int which);
extern UINT16 GetProcSerialNumbers(UINT32 *pSystemSN, UINT32 *pControllerSN);
extern UINT16 UpdateProcSerialNumber(UINT8 which, UINT32 serNum);
extern UINT32 InitIPAddress(UINT32 ipaddr);
extern void InitCachedMirrorPartnerSN(void);
extern UINT32 GetCachedMirrorPartnerSN(void);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _SERIAL_NUM_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
