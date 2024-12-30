/* $Id: PortServerUtils.h 149941 2010-11-03 21:38:18Z m4 $ */
/*===========================================================================
** FILE NAME:       PortServerUtils.h
** MODULE TITLE:    CCB Port Server Utilities
**
** DESCRIPTION:     Utility functions to support CCB port server
**
** Copyright (c) 2001-2009  Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _PORTSERVERUTILS_H_
#define _PORTSERVERUTILS_H_

#include "XIO_Types.h"
#include "PortServer.h"
#include "ipc_common.h"
#include "i82559.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/
#define SEL_TMO_USEC        (1000 * 1000)       /* 1000 milliseconds */

/*****************************************************************************
** Public variables
*****************************************************************************/
extern UINT8 piMD5Key[];
extern MUTEX sendMutex;

struct _XIO_PACKET;

/*****************************************************************************
** Public function prototypes
*****************************************************************************/

/* low level calls */
extern INT32 ReceiveDataRaw(INT32, UINT8 *pBuffer, INT32 bytesToRead, UINT32 timeOutSec);
extern INT32 SendPacketRaw(INT32, struct _XIO_PACKET *packet, UINT32 timeOutSec);

/* PI calls */
extern INT32 ReceiveData(INT32, UINT8 *pBuffer, INT32 bytesToRead, UINT32 timeOutSec);
extern INT32 SendPacket(INT32, struct _XIO_PACKET *pPacket, UINT32 timeOutSec);

/* miscellaneous */
extern void LogSocketError(int socketError, const char *errString);
extern void SetPortServerClientAddr(UINT32 clientIPAddr);
extern UINT32 GetPortServerClientAddr(void);

extern UINT32 GetSelectTimeoutBySocket(INT32);
extern void SetSelectTimeoutBySocket(INT32, UINT32 timeout);

extern const char *GetErrnoString(UINT32 errorNumber);

extern void InetToAscii(UINT32 ipAddr, char *pOutputStr);
extern UINT32 GetIpAddressFromInterface(void *intfHandle);
extern UINT32 GetSubnetFromInterface(void *intfHandle);
extern UINT32 GetGatewayFromInterface(void *intfHandle);
extern ETHERNET_MAC_ADDRESS GetMacAddrFromInterface(void *intfHandle);
extern INT32 GetLinkStatusFromInterface(void *intfHandle);

/* 'blockingState' values to be used in the SetBlockingState() call */
#define BLOCKING_OFF 0
#define BLOCKING_ON  1
extern INT32 SetBlockingState(INT32 sockFd, INT32 blockingState);

extern void setkeepalive(SOCKET sockFd);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _PORTSERVERUTILS_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
