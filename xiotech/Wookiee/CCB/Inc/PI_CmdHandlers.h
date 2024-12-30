/* $Id: PI_CmdHandlers.h 159391 2011-07-09 21:01:56Z m4 $ */
/*===========================================================================
** FILE NAME:       PI_CmdHandlers.h
** MODULE TITLE:    Command Handlers for Packet Interface Commands
**
** DESCRIPTION:     Function prototypes for all command handlers used by the
**                  packet interface.
**
** Copyright (c) 2001-2009 Xiotech Corporation.  All rights reserved.
**==========================================================================*/
#ifndef _PI_CMDHANDLERS_H_
#define _PI_CMDHANDLERS_H_

#include "XIO_Types.h"
#include "PortServer.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*****************************************************************************
** Public defines
*****************************************************************************/

struct _XIO_PACKET;

/*
 * The following macro makes it clearer that all of the PI command handler
 * functions are truly declared with identical prototypes. This also calls
 * more attention to the prototypes here that are different.
 */

#define PI_CMD_HANDLER(p)   INT32 p(struct _XIO_PACKET *pReq, \
            struct _XIO_PACKET *pRsp)

/*****************************************************************************
** Public function prototypes
*****************************************************************************/

PI_CMD_HANDLER(PI_GetObjectCount);
PI_CMD_HANDLER(PI_GetObjectList);
PI_CMD_HANDLER(PI_ProcBeDevicePathList);
PI_CMD_HANDLER(PI_Connect);
PI_CMD_HANDLER(PI_Ping);
PI_CMD_HANDLER(PI_Reset);
PI_CMD_HANDLER(PI_PowerUpState);
PI_CMD_HANDLER(PI_PowerUpResponse);
PI_CMD_HANDLER(PI_X1CompatibilityIndex);
PI_CMD_HANDLER(PI_EnableX1Server);
PI_CMD_HANDLER(PI_RegisterEvents);

PI_CMD_HANDLER(PI_PDisks);
PI_CMD_HANDLER(PI_PDiskAutoFailback);
PI_CMD_HANDLER(PI_PDisksCache);
PI_CMD_HANDLER(PI_PDiskLabel);
PI_CMD_HANDLER(PI_PDiskBeacon);
PI_CMD_HANDLER(PI_PDiskBypass);
PI_CMD_HANDLER(PI_PDiskQLTimeout);
PI_CMD_HANDLER(PI_PDiskDefrag);
PI_CMD_HANDLER(PI_PDiskDefragStatus);

PI_CMD_HANDLER(PI_VDisks);
PI_CMD_HANDLER(PI_VDisksCache);
PI_CMD_HANDLER(PI_VDiskInfo);
PI_CMD_HANDLER(PI_VDiskOwner);
PI_CMD_HANDLER(PI_VDiskSetAttributes);

PI_CMD_HANDLER(PI_Raids);
PI_CMD_HANDLER(PI_RaidsCache);
PI_CMD_HANDLER(PI_RAIDInfo);

PI_CMD_HANDLER(PI_Servers);
PI_CMD_HANDLER(PI_ServerInfo);
PI_CMD_HANDLER(PI_ServerWwnToTargetMap);

PI_CMD_HANDLER(PI_Targets);
PI_CMD_HANDLER(PI_TargetResourceList);

PI_CMD_HANDLER(PI_FirmwareVersionInfo);
PI_CMD_HANDLER(PI_FirmwareSystemReleaseLevel);
PI_CMD_HANDLER(PI_FWDownload);
PI_CMD_HANDLER(PI_SetTime);
PI_CMD_HANDLER(PI_GetTime);
PI_CMD_HANDLER(PI_LedControl);
PI_CMD_HANDLER(PI_SetIpAddress);
PI_CMD_HANDLER(PI_GetIpAddress);
PI_CMD_HANDLER(PI_MultiPartXfer);

PI_CMD_HANDLER(PI_DebugMemRdWr);
PI_CMD_HANDLER(PI_DebugInitCCBNVRAM);
PI_CMD_HANDLER(PI_DebugGetSerNum);
PI_CMD_HANDLER(PI_DebugStructDisplay);
PI_CMD_HANDLER(PI_DebugGetElecSt);
PI_CMD_HANDLER(PI_DebugGetState_RM);

PI_CMD_HANDLER(PI_SCSICmd);
PI_CMD_HANDLER(PI_READWRITECmd);

PI_CMD_HANDLER(PI_VCGValidation);
PI_CMD_HANDLER(PI_VCGPrepareSlave);
PI_CMD_HANDLER(PI_VCGAddSlave);
PI_CMD_HANDLER(PI_VCGPing);
PI_CMD_HANDLER(PI_VCGInfo);
PI_CMD_HANDLER(PI_VCGInactivateController);
PI_CMD_HANDLER(PI_VCGApplyLicense);
PI_CMD_HANDLER(PI_VCGUnfailController);
PI_CMD_HANDLER(PI_VCGFailController);
PI_CMD_HANDLER(PI_VCGRemoveController);
PI_CMD_HANDLER(PI_VCGShutdown);
PI_CMD_HANDLER(PI_GetCpuCount);
PI_CMD_HANDLER(PI_GetBackendType);

PI_CMD_HANDLER(PI_LogInfoRequest);
PI_CMD_HANDLER(PI_LogClearRequest);
PI_CMD_HANDLER(PI_LogTextMessage);

PI_CMD_HANDLER(PI_GenericCommand);
PI_CMD_HANDLER(PI_Generic2Command);
PI_CMD_HANDLER(PI_GenericMRP);

PI_CMD_HANDLER(PI_EnvIIRequest);
PI_CMD_HANDLER(PI_EnvStatsRequest);

#if defined(MODEL_7000) || defined(MODEL_4700)
PI_CMD_HANDLER(PI_ISEStatus);
PI_CMD_HANDLER(PI_BeaconIseComponent);
#endif /* MODEL_7000 || MODEL_4700 */

PI_CMD_HANDLER(PI_StatsLoop);
PI_CMD_HANDLER(PI_StatsVDisk);
PI_CMD_HANDLER(PI_StatsProc);
PI_CMD_HANDLER(PI_StatsPCI);
PI_CMD_HANDLER(PI_StatsCacheDevices);
PI_CMD_HANDLER(PI_DiskBaySESEnviro);
PI_CMD_HANDLER(PI_CtrlAndBayEnviro);
PI_CMD_HANDLER(PI_StatsServers);
PI_CMD_HANDLER(PI_ConfigurationMemoryUsage);

PI_CMD_HANDLER(PI_DiskBays);
PI_CMD_HANDLER(PI_DiskBayAlarmControl);

PI_CMD_HANDLER(PI_FileSystemRead);
PI_CMD_HANDLER(PI_FileSystemWrite);

PI_CMD_HANDLER(PI_FailureStateSet);
PI_CMD_HANDLER(PI_UnfailInterface);
PI_CMD_HANDLER(PI_FailInterface);

PI_CMD_HANDLER(PI_RollingUpdatePhase);

PI_CMD_HANDLER(PI_MiscGetMode);
PI_CMD_HANDLER(PI_MiscSetMode);

PI_CMD_HANDLER(PI_TakeSnapshot);
PI_CMD_HANDLER(PI_LoadSnapshot);
PI_CMD_HANDLER(PI_ChangeSnapshot);
PI_CMD_HANDLER(PI_ReadSnapshotDirectory);

PI_CMD_HANDLER(PI_PersistentDataControl);

PI_CMD_HANDLER(PI_WCacheInvalidate);

PI_CMD_HANDLER(PI_MiscSerialNumberSet);
PI_CMD_HANDLER(PI_MiscResyncMirrorRecords);
PI_CMD_HANDLER(PI_MiscMirrorPartnerControl);

PI_CMD_HANDLER(PI_ConfigCtrl);
PI_CMD_HANDLER(PI_MfgCtrlClean);

PI_CMD_HANDLER(PI_GetWorksetInfo);

PI_CMD_HANDLER(PI_CacheRefreshCCB);
PI_CMD_HANDLER(PI_DLMHeartbeatList);
PI_CMD_HANDLER(PI_MiscResyncRaids);
PI_CMD_HANDLER(PI_MiscPutDevConfig);
PI_CMD_HANDLER(PI_MiscGetDevConfig);

PI_CMD_HANDLER(PI_BatteryHealthSet);

PI_CMD_HANDLER(PI_MiscResyncData);
PI_CMD_HANDLER(PI_MiscMirrorPartnerGetCfg);
PI_CMD_HANDLER(PI_MiscAssignMirrorPartner);

PI_CMD_HANDLER(PI_MiscGetLocalRaidInfo);

PI_CMD_HANDLER(PI_ClientPersistentDataControl);
PI_CMD_HANDLER(PI_LogAcknowledge);
PI_CMD_HANDLER(PI_RegisterClientType);
PI_CMD_HANDLER(PI_QuickBreakPauseResumeMirrorStart);
PI_CMD_HANDLER(PI_QuickBreakPauseResumeMirrorSequence);
PI_CMD_HANDLER(PI_QuickBreakPauseResumeMirrorExecute);
PI_CMD_HANDLER(PI_BatchSnapshotStart);
PI_CMD_HANDLER(PI_BatchSnapshotSequence);
PI_CMD_HANDLER(PI_BatchSnapshotExecute);


extern INT32 PI_MRPPassThrough(struct _XIO_PACKET *pReqPacket,
                               struct _XIO_PACKET *pRspPacket,
                               UINT16 mrpCmd, UINT32 rspDataSz, UINT32 timeout);

extern INT32 PI_GenericFunc(UINT32 p0, UINT32 p1, UINT32 p2, UINT32 p3,
                            UINT32 p4, UINT32 p5, UINT32 p6, UINT32 p7);

#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* _PI_CMDHANDLERS_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
