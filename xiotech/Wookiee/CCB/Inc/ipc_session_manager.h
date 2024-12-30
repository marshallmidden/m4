/* $Id: ipc_session_manager.h 143020 2010-06-22 18:35:56Z m4 $ */
/*============================================================================
** FILE NAME:       ipc_session_manager.h
** MODULE TITLE:    Header file for ipc_session_manager.c
**
** DESCRIPTION:     The session manager is the central place for managing all
**                  all the sockets to a CCB and for handling all the packets
**                  that arive to the CCB.  It also does the unblocking and
**                  callbacks for the IpcSendPacket.
**
** Copyright (c) 2001-2010 Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _IPC_SESSION_MANAGER_H_
#define _IPC_SESSION_MANAGER_H_

#include "XIO_Types.h"
#include "ipc_packets.h"
#include "pcb.h"
#include "ipc_common.h"
#include "quorum.h"
#include "XIO_Std.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/
#define IPC_PORT_ETH        3008

#define ETH_GOOD(a)         ((a)->eth != SOCKET_ERROR)
#define FC_GOOD(a)          ((a)->fcDirect == TRUE)

/*
** Structure used to define each of the sessions that we may have
*/
typedef struct _SESSION
{
    UINT32      sn;
    SOCKET      eth;
    UINT16      fcDirect;
    UINT16      sendLock;
    UINT16      protocolLevel;
    UINT16      status;
} SESSION;

/*
** Information that is stored for the duration of life span of the
** users send packet request
*/
typedef struct _SENDER_INFORMATION
{
    SESSION    *session;
    PATH_TYPE   requestedPath;
    PATH_TYPE  *ipc_send_rc;
    PATH_TYPE   actualPath;
    IPC_PACKET *txPacket;
    IPC_PACKET *rxPacket;
    void        (*callBack)(void *parms);
    void       *callBackParams;
    IPC_CALL_BACK_RESULTS *callBackResult;
    UINT32      timeout;
    UINT32      expirationTime;
    PCB        *pcbID;
    UINT16      seqNum;
    UINT16      stateMachine;
    UINT16      haveWokenSender;
    UINT16      numRetries;
} SENDER_INFORMATION;

/*****************************************************************************
** Public macros
*****************************************************************************/

/*
** void   IPC_SetSessionProtocolLevel(SESSION* session, IPC_PACKET* packetPtr);
*/
#define IPC_SetSessionProtocolLevel(s, p) (s->protocolLevel = p->header->protocolLevel)

/*
** UINT16 IPC_GetSessionProtocolLevel(SESSION* session);
*/
#define  IPC_GetSessionProtocolLevel(s) (s->protocolLevel)

/*****************************************************************************
** Public variables
*****************************************************************************/
extern MUTEX   sessionListMutex;
extern MUTEX   packetQueueMutex;
extern MUTEX   outStandingMutex;
extern MUTEX   inProgressMutex;
extern MUTEX   sequenceNumberMutex;
extern MUTEX   historyMutex;
extern MUTEX   runningTasksMutex;

/*****************************************************************************
** Public function prototypes
*****************************************************************************/

extern SESSION *GetSession(UINT32);
extern bool QueueSenderInformation(SESSION *, PATH_TYPE *, IPC_PACKET *, IPC_PACKET *,
                                   void (*callBack)(void *), void *,
                                   IPC_CALL_BACK_RESULTS *, UINT32, PCB *);
extern bool BringUpInterface(SESSION *, PATH_TYPE);
extern bool CheckInterfaceStatuses(void);
extern SOCKET *GetEthernetSocket(SESSION *);
extern void TakeNap(void);
extern void CreateSessionManager(TASK_PARMS *);
extern void GetSendLock(SESSION *);
extern void FreeSendLock(SESSION *);
extern UINT32 NextSequenceNumber(void);
extern bool ClientConnect(SOCKET, UINT32);
extern void CloseSocket(SOCKET *);
extern void CloseEthernetSocket(SESSION *);
extern void SendResponseQueueDataPacketToSm(IPC_PACKET *, PATH_TYPE);
extern void IpcShutDown(void);
extern bool IpcShutDownRequested(void);
extern void RemoveSessionBindings(SESSION *, bool);
extern UINT8 IPC_ElectionNotify(UINT8);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _IPC_SESSION_MANAGER_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
