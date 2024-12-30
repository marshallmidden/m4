/* $Id: ipc_sendpacket.h 156533 2011-06-24 22:45:20Z m4 $ */
/*============================================================================
** FILE NAME:       ipc_sendpacket.h
** MODULE TITLE:    Header file for ipc_sendpacket.c
**
** DESCRIPTION:     Specification for the ipc send packet
**
** Copyright (c) 2001-2010 Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _IPC_SENDPACKET_H_
#define _IPC_SENDPACKET_H_

#include "ipc_packets.h"
#include "XIO_Types.h"
#include "ipc_session_manager.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/

/*
** RMCMDHDL_IPC_SEND_TMO is the timeout for sending an IPC packet to another
** controller.  The value represents how many milliseconds to wait for the
** send to complete.  At the current value of 120000 the timeout is 2 minutes.
*/
#define RMCMDHDL_IPC_SEND_TMO            120000

/*----------------------------------------------------------------------------
** Macro Name:  IpcSuccessfulXfer
**
** Description: Deterimines if IPC transfer was successful, based on the
**              rerturned pathtype.
**
** Inputs:      UINT32 pathType - Return value from IpcSendPacket.
**
** Outputs:
**
** Returns:    1 = Successful transfer
**             0 = Tranfer failed.
**--------------------------------------------------------------------------*/
#define IpcSuccessfulXfer(pathType) (pathType > SENDPACKET_ANY_PATH)

/*****************************************************************************
** Public function prototypes
*****************************************************************************/

/*----------------------------------------------------------------------------
**  Function Name: IpcSendPacket
**
**  Description:    This function allows a CCB to send one packet to another
**                  CCB based on CCB serial number and what path they would
**                  like the packet to take.
**
**  Inputs: SESSION    *session         Pointer to a session, use GetSession
**          PATH_TYPE   requestedPath   Path you want the packet to go over
**          IPC_PACKET  txPacket        Pointer to a packet to transmit
**          IPC_PACKET  rxPacket        Pointer to the returning data packet
**                                      User needs to supply a valid pointer
**
**
**                      callBack        Pointer to a function to call when
**                                      completed.  If a callback is used the
**                                      callBackParameters must be non-null.
**                                      The first parameter in the call back
**                                      is reserved so that the Ipc layer can
**                                      inform the user about how the
**                                      operation finished
**
**          void *      callBackParameters  Pointer to the parameter to be
**                                          passed to the call back funtion
**
**          IPC_CALL_BACK_RESULTS      *asyncResult  -  Pointer to a result
**                                                      structure
**
**          UINT32      tmo             How long you want to wait in ms
**                                      to the nearest rounded up 125 ms
**                                      You cannot specify a value < 1000 as
**                                      we will round up to this value.
**                                      If you specify 0 there is no timeout.
**
**
**  Outputs:
**
**  Returns:    Actual path the packet was sent on.  A new path type
**              SENDPACKET_TIMEOUT has been defined to allow this information
**              to be conveyed.
**
**  Notes:  If you spec. that the callback and the txPackt are NULL, you will
**          not get any feedback if the operation was successful only that
**          someone read it on the other side
**
**          You can only use blocking or a callback, not both (This was not
**          the case before)
**
**          If rxPacket is not NULL we will block if call back is NULL.
**
**          If you do a callback the rxPacket will not be pointing to
**          anything until the callback occurs!
**
**
**          If the call returns either no path or timeout and you are using a
**          callback the callback will not be called.
**
**          If you make the call expecting to get a result and the other CCB
**          does not return one you will timeout.
**
**--------------------------------------------------------------------------*/
extern PATH_TYPE IpcSendPacket(SESSION *session,
                               PATH_TYPE requestedPath,
                               IPC_PACKET *txPacket,
                               IPC_PACKET *rxPacket,
                               void (*callBack)(void *params),
                               void *callBackParameters,
                               IPC_CALL_BACK_RESULTS *asyncResult, UINT32 tmo);

extern PATH_TYPE IpcSendPacketBySN(UINT32 serialNum,
                                   PATH_TYPE requestedPath,
                                   IPC_PACKET *txPacket,
                                   IPC_PACKET *rxPacket,
                                   void (*callBack)(void *params),
                                   void *callBackParameters,
                                   IPC_CALL_BACK_RESULTS *asyncResult, UINT32 tmo);

extern INT32 IpcSendPingWithRetries(UINT32 serialNum,
                                    PATH_TYPE desiredPath, UINT8 retries);

extern INT32 IpcSendPing(UINT32 serialNum, PATH_TYPE desiredPath);

extern INT32 IpcSendResyncPersistentData(UINT32 serialNum);

extern INT32 TunnelRequest(UINT32 ctrlSN, XIO_PACKET *pXIOReq, XIO_PACKET *pXIORsp);

extern INT32 IpcSendOffline(UINT32 serialNum, UINT32 delay);

extern void IpcSignalSlaves(UINT16 signalEvent);


#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _IPC_SENDPACKET_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
