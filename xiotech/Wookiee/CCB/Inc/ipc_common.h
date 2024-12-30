/* $Id: ipc_common.h 143020 2010-06-22 18:35:56Z m4 $ */
/*============================================================================
** FILE NAME:       ipc_common.h
** MODULE TITLE:    Header file for ipc_common.c
**
** DESCRIPTION:     Shared #defined etc for IPC port server
**
** Copyright (c) 2001-2009 Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _IPC_COMMON_H_
#define _IPC_COMMON_H_

#include "XIO_Types.h"
#include "debug_files.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/

/*
** If you want the command dispatcher to fork define this (adds complexity)
*/
#define FORKING_COMMAND_DISPATCHER

/*
** All the different timeouts are defined here
**
*/
#define IPC_CLOCK_RES       125

#define CONNECT_TMO         2   /* Connection timeout in seconds */

/*
** This is how long we will wait to send/receive a single transfer over a socket.
*/
#define IPC_TX_TMO_MIN      1   /* seconds */
#define IPC_TX_TMO_MAX      5   /* seconds */

#define IPC_RX_TMO_MIN      3   /* seconds */
#define IPC_RX_TMO_MAX      15  /* seconds */

/*
** How long we are willing to wait for a command in progress before timing out
*/
#define COMMAND_IN_PROGRESS_TMO_MS 40   /* 40 * 125ms = 5000 */

#ifndef SOCKET_TYPE_DEF
#define SOCKET_TYPE_DEF
typedef int SOCKET;
#endif

#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1
#endif

#define     IPC_LISTENING_QUEUE_SIZE    5       /* Queue for listening sockets */

#define IPC_DIGEST_LENGTH 16    /* Md5 digest length            */

/*
** Different paths the IPC can send a packet over
**
** You can add to this list, but keep SENDPACKET_NO_PATH through
** SENDPACKET_QUORUM sequential in the values, see #define for
** ENUM_DIFFERENCE in ipc_packets.h
*/
typedef enum _PATH_TYPE
{
    SENDPACKET_TIME_OUT = 0,
    SENDPACKET_NO_PATH = 1,
    SENDPACKET_ANY_PATH = 2,
    SENDPACKET_ETHERNET = 3,
    SENDPACKET_FIBRE = 4,
    SENDPACKET_QUORUM = 5
} PATH_TYPE;

/*
** Structure used for async IpcSendPacket function calls
** we can add a lot more data later here if we want
*/
typedef struct _IPC_CALL_BACK_RESULTS
{
    PATH_TYPE   result;         /*  Same as the rc for a blocking   */
} IPC_CALL_BACK_RESULTS;

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _IPC_COMMON_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
