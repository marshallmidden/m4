/* $Id: PktCmdHdl.h 122127 2010-01-06 14:04:36Z m4 $ */
/*===========================================================================
** FILE NAME:       PktCmdHdl.h
** MODULE TITLE:    Packet Command Handler Layer
**
** DESCRIPTION:     Implements the Packet Command Handler layer
**                  in the CCB code
**
** Copyright (c) 2001-2009  Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef __PKTCMDHDL_H__
#define __PKTCMDHDL_H__

#include "XIO_Types.h"
#include "PortServer.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

struct _XIO_PACKET;

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern INT32 PacketCommandHandler(struct _XIO_PACKET *pReqPacket,
                                  struct _XIO_PACKET *pRspPacket);
extern INT32 PacketCommandHandlerPreProcessImpl(struct _XIO_PACKET *pReqPacket,
                                                struct _XIO_PACKET *pRspPacket);
extern INT32 PacketCommandHandlerImpl(struct _XIO_PACKET *pReqPacket,
                                      struct _XIO_PACKET *pRspPacket);
extern INT32 PacketCommandHandlerPostProcessImpl(struct _XIO_PACKET *pReqPacket,
                                                 struct _XIO_PACKET *pRspPacket);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* __PKTCMDHDL_H__ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
