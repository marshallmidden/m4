/* $Id: CmdLayers.h 122127 2010-01-06 14:04:36Z m4 $ */
/*===========================================================================
** FILE NAME:       CmdLayers.h
** MODULE TITLE:    CCB Command Layers
**
** DESCRIPTION:     Defines the command layering withing the CCB code
**
** Copyright (c) 2001-2009  Xiotech Corporation.  All rights reserved.
**==========================================================================*/

#ifndef _CMDLAYERS_H_
#define _CMDLAYERS_H_

#include "XIO_Types.h"
#include "PortServer.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
*** --- NOTE: Only the top command layer is made public by this header file.
**            If other layers need to be made public please let me know.
*/

struct _XIO_PACKET;

/*----------------------------------------------------------------------------
** Function:    PortServerCommandHandler
**
** Description: This is the top of the CCB packet interface command chain.
**              In general all users should call through this layer unless
**              they have a valid reason not to along with a signed note
**              from the powers that be and their friends.
**
** Inputs:      pReqPacket - pointer to the request packet
**              pRspPacket - pointer to the response packet
**
** Returns:     GOOD or ERROR
**
** WARNING:     none
**
**--------------------------------------------------------------------------*/
extern INT32 PortServerCommandHandler(struct _XIO_PACKET *pReqPacket,
                                      struct _XIO_PACKET *pRspPacket);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _CMDLAYERS_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
