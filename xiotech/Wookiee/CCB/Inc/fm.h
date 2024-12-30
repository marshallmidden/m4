/* $Id: fm.h 143766 2010-07-06 12:06:32Z m4 $ */
/*============================================================================
** FILE NAME:       fm.h
** MODULE TITLE:    Header file for both fm_master.c and fm_slave.c
**
** Copyright (c) 2001-2009 Xiotech Corporation. All rights reserved.
**==========================================================================*/
#ifndef _FM_H_
#define _FM_H_

#include "XIO_Types.h"
#include "XIO_Std.h"
#include "ipc_packets.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
******************************************************************************
** Private internal function prototypes
******************************************************************************
*/
extern void MasterFailureManager(IPC_REPORT_CONTROLLER_FAILURE *, UINT32 length);
extern void FM_ClearWorkingControllers(void);
extern void QueueFailurePacket(IPC_REPORT_CONTROLLER_FAILURE *, UINT32 length);
extern void PrintFailurePacket(const IPC_REPORT_CONTROLLER_FAILURE *, bool mfm);

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern void EnqueueToFailureManagerQueue(IPC_REPORT_CONTROLLER_FAILURE *, UINT32 length);
extern void FailureManager(IPC_REPORT_CONTROLLER_FAILURE *, UINT32 length);
extern void SlaveFailureManagerInit(TASK_PARMS *parms);
extern IPC_PACKET *IpcMasterFailureManager(IPC_PACKET *Packet);
extern unsigned char FM_MasterController(void);
extern UINT8 SlaveFailure_ElectionNotify(UINT8 electionState);
extern IPC_PACKET *FM_SlaveOffline(IPC_PACKET *packet);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _FM_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
