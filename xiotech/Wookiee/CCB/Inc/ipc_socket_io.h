/* $Id: ipc_socket_io.h 143020 2010-06-22 18:35:56Z m4 $ */
/*============================================================================
** FILE NAME:       ipc_socket_io.h
** MODULE TITLE:    Header file for ipc_socket_io.c
**
** DESCRIPTION:     Specifcation for the ipc socket io.  Provides an
**                  abstraction for the reading and writing of data to the
**                  sockets.
**
** Copyright (c) 2001-2010 Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _IPC_SOCKET_IO_H_
#define _IPC_SOCKET_IO_H_

#include "XIO_Types.h"
#include "ipc_packets.h"
#include "ipc_common.h"
#include "ipc_session_manager.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public function prototypes
*****************************************************************************/

extern PATH_TYPE SendPacketOverInterface(SESSION *session,
                                         PATH_TYPE requestedPath, IPC_PACKET *txPacket);

extern IPC_PACKET *ReadIpcPacket(SOCKET s);

extern bool SetIpcSocketOptions(SOCKET s);

extern int  NonBlockingConnect(SOCKET sockfd, const struct sockaddr *addressPtr,
                               int slen, UINT16 tmo_sec);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _IPC_SOCKET_IO_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
