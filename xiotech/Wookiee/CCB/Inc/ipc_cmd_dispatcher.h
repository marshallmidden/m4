/* $Id: ipc_cmd_dispatcher.h 143020 2010-06-22 18:35:56Z m4 $ */
/*============================================================================
** FILE NAME:       ipc_command_dispatcher.h
** MODULE TITLE:    Header file for ipc_command_dispatcher.c
**
** DESCRIPTION:     Code that handles the actual IPC packet processing
**
** Copyright (c) 2001-2009 Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _IPC_COMMAND_DISPATCHER_H_
#define _IPC_COMMAND_DISPATCHER_H_

#include "XIO_Types.h"
#include "ipc_packets.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public function prototypes
*****************************************************************************/
extern IPC_PACKET *IpcCommandDispatcher(IPC_PACKET *packet);

extern UINT16 MRP_FEPortGo(void);

extern void MRP_NegotiateMPInfo(void);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _IPC_COMMAND_DISPATCHER_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
