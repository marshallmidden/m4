/* $Id: quorum_comm.h 143020 2010-06-22 18:35:56Z m4 $ */
/*============================================================================
** FILE NAME:       quorumcomm.h
** MODULE TITLE:    Header file for quorumcomm.c
**
** Copyright (c) 2001-2010 Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _QUORUMCOMM_H_
#define _QUORUMCOMM_H_

#include "XIO_Types.h"
#include "ipc_packets.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public function prototypes
*****************************************************************************/

extern void QuorumManager(TASK_PARMS *parms);
extern INT32 SendQuorumPacket(UINT32 controllerSN, IPC_PACKET *txPacket);
extern bool QuorumSendablePacket(UINT16 commandCode);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _QUORUMCOMM_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
