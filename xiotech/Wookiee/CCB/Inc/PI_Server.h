/* $Id: PI_Server.h 122127 2010-01-06 14:04:36Z m4 $*/
/**
******************************************************************************
**
**  @file       PI_Server.h
**
**  @brief      Server Commands
**
**  These functions handle requests for server information.
**
**  Copyright (c) 1996-2004 Xiotech Corporation.  All rights reserved.
**
******************************************************************************
**/

#ifndef __PI_SERVER_H__
#define __PI_SERVER_H__

#include "XIO_Types.h"
#include "PacketInterface.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern PI_SERVERS_RSP *Servers(UINT32 controllerSN);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* __PI_SERVER_H__ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
