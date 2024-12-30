/* $Id: ipc_listener.h 122127 2010-01-06 14:04:36Z m4 $ */
/*============================================================================
** FILE NAME:       ipc_listener.h
** MODULE TITLE:    Header file for ipc_listener.c
**
** DESCRIPTION:     Specification for the ipc port listener daemon.  This
**                  function creates a thread of execution that never exits.
**                  Sole purpose is to listen on all interfaces on port 3008.
**                  When a client connects the socket connection is
**                  registered with the session manager so that all can use
**                  it.
**
** Copyright (c) 2001-2009  Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _IPC_LISTENER_H_
#define _IPC_LISTENER_H_

#include "XIO_Types.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public function prototypes
*****************************************************************************/

/*----------------------------------------------------------------------------
**  Function Name: PortListener
**
**  Description:    Process that runs forever.  Basically this is the function
**                  that listens on port 3008 on all interfaces for a client
**                  connect.  When that happens is regisers the new client
**                  socket connection with the session manager.
**
**  Inputs: void *  Default signature so that we can do a pthread create in
**                  unix.  Paramter is not needed
**
**  Outputs:
**
**  Returns:    Never returns
**--------------------------------------------------------------------------*/
extern void PortListener(TASK_PARMS *parms);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _IPC_LISTENER_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
