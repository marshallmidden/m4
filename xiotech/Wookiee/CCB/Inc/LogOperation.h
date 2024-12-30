/* $Id: LogOperation.h 122127 2010-01-06 14:04:36Z m4 $ */
/*============================================================================
** FILE NAME:       LogOperation.h
** MODULE TITLE:    Header file for LogOperation.c
**
** DESCRIPTION:     Log operations initiated through the packet interface
**
** Copyright (c) 2001-2008  Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _LOGOPERATION_H_
#define _LOGOPERATION_H_

#include "PortServer.h"
#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/
struct _XIO_PACKET;

/*****************************************************************************
** Public function prototypes
*****************************************************************************/
extern INT32 LogOperation(struct _XIO_PACKET *pReqPacket, struct _XIO_PACKET *pRspPacket);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _LOGOPERATION_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
