/* $Id: ipc_heartbeat.h 143845 2010-07-07 20:51:58Z mdr $ */
/*============================================================================
** FILE NAME:       ipc_heartbeat.h
** MODULE TITLE:    Header file for ipc_heartbeat.c
**
** DESCRIPTION:     Interprocessor heartbeat definitions
**
** Copyright (c) 2001-2009 Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _IPC_HEARTBEAT_H_
#define _IPC_HEARTBEAT_H_

#include "XIO_Types.h"
#include "XIO_Std.h"
#include "ipc_common.h"
#include "ipc_packets.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/
#ifndef M4_ABORT
#define HB_MRP_TIMEOUT                  5000    /* 5 second timeout         */
#else   /* M4_ABORT */
#define HB_MRP_TIMEOUT                  50000    /* 50 second timeout         */
#endif  /* M4_ABORT */

/*****************************************************************************
** Public function prototypes
*****************************************************************************/
extern void IpcHeartbeatAndStatsTask(TASK_PARMS *parms);
extern IPC_PACKET *HandleIpcHeartbeat(IPC_PACKET *ptrPacket);
extern UINT8 HealthMonitor_ElectionNotify(UINT8 newControllerState);

extern void UpdateLinkStatus(TASK_PARMS *parms);

extern UINT64 GetServerIOPerSecond(void);
extern UINT64 GetServerBytesPerSecond(void);
extern UINT32 GetFEHeartbeatFlag(void);
extern UINT32 GetBEHeartbeatFlag(void);
extern void IPC_BringUpInterfacesTask(TASK_PARMS *parms);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _IPC_HEARTBEAT_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
