/* $Id: ipc_server.h 143020 2010-06-22 18:35:56Z m4 $ */
/*============================================================================
** FILE NAME:       ipc_server.h
** MODULE TITLE:    Header file for ipc_server.c
**
** DESCRIPTION:     Specfication for the ipc server process.  This is the
**                  process that monitors a set of incoming sockets for data
**                  to read.  When it has data to read is reads the data in
**                  and if it is all ok, enqueues it to the session manager.
**
** Copyright (c) 2001-2010  Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _IPC_SERVER_H_
#define _IPC_SERVER_H_

#include "XIO_Types.h"
#include "ipc_session_manager.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public function prototypes
*****************************************************************************/

extern void IpcServer(TASK_PARMS *parms);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _IPC_SERVER_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
