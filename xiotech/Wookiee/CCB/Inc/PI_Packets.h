/* $Id: PI_Packets.h 144342 2010-07-19 21:14:53Z m4 $*/
/**
******************************************************************************
**
**  @file       PI_Packets.h
**
**  @brief      Command Handlers for PI Packet Interface Commands
**
**  Copyright (c) 2001-2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _PI_PACKETS_H_
#define _PI_PACKETS_H_

#include "globalOptions.h"
#include "XIO_Types.h"
#include "XIOPacket.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/

/**
**  @name PI Compatibility Index
**
**  Roll this compatibility number based on code level.
**
**  @{
**/
#define PI_COMPAT_3000              0x01
#define PI_COMPAT_750               0x02
#define PI_COMPAT_750_2             0x03
#define PI_COMPAT_4000              0x04

#define PI_COMPATIBILITY            PI_COMPAT_4000

#define PI_PACKET_VERSION_1         1
#define PI_PACKET_VERSION_2         2

/* @} */

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _PI_PACKETS_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
