/* $Id: RMCmdHdl.h 122127 2010-01-06 14:04:36Z m4 $*/
/**
******************************************************************************
**
**  @file       RMCmdHdl.h
**
**  @brief      Resource Manager Command Handler Layer
**
**  Implements the Resource Manager Command Handler layer in the CCB code
**
**  Copyright (c) 2001 - 2009 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/

#ifndef __RMCMDHDL_H__
#define __RMCMDHDL_H__

#include "XIO_Types.h"
#include "PortServer.h"
#include "ipc_packets.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
******************************************************************************
** Public variables
******************************************************************************
*/
extern MUTEX configUpdateMutex;

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/

extern INT32 RMCommandHandlerPreProcessImpl(XIO_PACKET *pReqPacket,
                                            XIO_PACKET *pRspPacket);
extern INT32 RMCommandHandlerPostProcessImpl(XIO_PACKET *pReqPacket,
                                             XIO_PACKET *pRspPacket);

extern void RMSlavesConfigurationUpdate(UINT32 reason, UINT32 needToLock);
extern void RMSlavesRefreshNVRAM(UINT32 reason);

extern UINT32 RMSendIpcConfigurationUpdate(UINT32 serialNum, UINT8 restoreOption,
                                           UINT32 reason);

extern UINT16 UpdateLocalImage(UINT8 *pLocalImage);
extern void DumpLocalImage(const char *location, UINT32 imageSize, void *pLocalImage);

extern void LocalImageInitSequence(void);
extern bool LocalImageStillProcess(void *pLocalImage);

extern INT32 RMTempDisableCache(UINT32 commandCode, UINT8 user, UINT8 option);
extern void RMWaitForCacheFlush(void);
extern void RMWaitForCacheFlushController(UINT32 controllerSN);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* __RMCMDHDL_H__ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
