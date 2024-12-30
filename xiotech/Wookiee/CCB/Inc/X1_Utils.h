/* $Id: X1_Utils.h 143845 2010-07-07 20:51:58Z mdr $*/
/*===========================================================================
** FILE NAME:       X1_Utils.h
** MODULE TITLE:    X1 Packet Utilities
**
** DESCRIPTION:     Utility functions for X1 request packets.
**
** Copyright (c) 2002-2009  Xiotech Corporation.  All rights reserved.
**==========================================================================*/

#ifndef __X1_UTILS_H__
#define __X1_UTILS_H__

#include "XIOPacket.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public variables
*****************************************************************************/

/*****************************************************************************
** Public function prototypes
*****************************************************************************/

extern void GetFirmwareVersions(X1_FWVERSIONS *fwv);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* __X1_UTILS_H__ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
