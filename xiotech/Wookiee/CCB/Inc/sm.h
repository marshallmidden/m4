/* $Id: sm.h 122127 2010-01-06 14:04:36Z m4 $ */
/*============================================================================
** FILE NAME:       sm.h
** MODULE TITLE:    Sequence manager headers
**
** DESCRIPTION:     Various headers and definitions for SM
**
** Copyright (c) 2002-2009 Xiotech Corporation.  All rights reserved.
**==========================================================================*/

#ifndef __SM_H__
#define __SM_H__

#include "ipc_packets.h"
#include "PacketInterface.h"

#ifdef __cplusplus
#pragma pack(push,1)
#endif

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/

/*
** Default stop IO operation and intent
*/
#define SM_STOP_IO_OP       (STOP_FLUSH | STOP_WAIT_FOR_FLUSH | STOP_NO_BACKGROUND)
#define SM_STOP_IO_INTENT   (STOP_NO_SHUTDOWN)

/*
** StopIO timeout constants
*/
#define TMO_RCU_STOP_IO                     40000       /* 40 sec */
#define TMO_INACTIVATE_STOP_IO              40000       /* 40 sec */
#define TMO_MPC_STOP_IO                     40000       /* 40 sec */
#define TMO_DEFRAG_ORPHAN_STOP_IO           40000       /* 40 sec */
#define TMO_DEFRAG_MOVE_STOP_IO             40000       /* 40 sec */

/* wait this many ms for mirror records to sync */
#define SYNC_RAIDS_TIMEOUT          4500000

/* Wait this long for continue w/o mirror partner */
#define CONTINUE_WO_MIRROR_TIMEOUT  20000

/* Wait this long to set the fe fibre list */
#define SET_FE_FIBRE_LIST_TIMEOUT   800000

/*
******************************************************************************
** Public variables
******************************************************************************
*/
extern UINT32 gCurrentMirrorPartnerSN;
extern MUTEX SM_mpMutex;

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/
extern void SM_Init(TASK_PARMS *parms);
extern void SM_Cleanup(void);

extern void SM_RebuildHeartbeatListStart(void);

extern INT32 SM_FlushWithoutMirrorPartnerMRP(void);
extern INT32 ResetInterfaceFE(UINT32 controllerSN, UINT8 channel, UINT8 option);

extern INT32 StartIO(UINT32 controllerSN, UINT8 option, UINT8 user, UINT32 tmo);

extern INT32 StopIO(UINT32 controllerSN,
                    UINT8 operation, UINT8 intent, UINT8 user, UINT32 tmo);

extern MP_MIRROR_PARTNER_INFO *SM_GetMirrorPartnerConfig(UINT32 partnerSN);

extern UINT8 AssignMirrorPartnerMRP(UINT32 partnerSN,
                                    UINT32 *oldPartnerSN,
                                    MP_MIRROR_PARTNER_INFO * pMPInfo);

extern INT32 SM_RescanDevices(UINT32 controllerSN, UINT8 scanType);

extern IPC_PACKET *SM_IPCFlushBECache(IPC_PACKET *pPacket);
extern IPC_PACKET *SM_IPCAssignFEMirrorPartner(IPC_PACKET *pPacket);
extern IPC_PACKET *SM_IPCContinueWithoutMirrorPartner(IPC_PACKET *pPacket);
extern IPC_PACKET *SM_IPCRescanDevices(IPC_PACKET *pPacket);
extern IPC_PACKET *SM_IPCSetHeartbeatList(IPC_PACKET *pPacket);
extern IPC_PACKET *SM_IPCGetMirrorPartner(IPC_PACKET *pPacket);
extern IPC_PACKET *SM_IPCFlushCompleted(IPC_PACKET *pPacket);

extern PI_PROC_TARGET_CONTROL_RSP *SM_TargetControl(UINT32 controllerSN, UINT16 option);
extern INT32 SM_ResyncMirrorRecords(UINT32 controllerSN, UINT8 type, UINT16 rid);
extern INT32 SM_RaidResyncController(UINT32 controllerSN, MR_LIST_RSP * pList);
extern PI_VCG_GET_MP_LIST_RSP *SM_GetMirrorPartnerList(void);
extern INT32 SM_MRResyncWithRetry(UINT8 type, UINT16 ridOrNumRIDs, void *pList);
extern INT32 SM_MRReset(UINT32 controllerSN, UINT32 type);

extern INT32 SM_MirrorPartnerControl(UINT32 controllerSN,
                                     UINT32 partnerSN,
                                     UINT32 option, MP_MIRROR_PARTNER_INFO * pMPInfo);

extern INT32 SM_ContinueWithoutMP(UINT32 controllerSN);
extern INT32 SM_MRResumeCache(UINT8 userResp);

extern void SM_HandleLostMirrorPartnerTask(TASK_PARMS *parms);
extern void SM_RestoreMirrorPartnerTask(TASK_PARMS *parms);

extern INT32 SM_ModifyRaid5MirrorStatus(bool option);

extern MR_LIST_RSP *SM_RaidsOwnedByController(UINT32 controllerSN,
                                              PI_SERVERS_RSP *pServerData,
                                              PI_TARGETS_RSP *pTargetData,
                                              PI_VDISKS_RSP *pVdiskData);

extern INT32 SM_PutDevConfig(void);

extern PI_MISC_QUERY_MP_CHANGE_RSP *SM_QueryMirrorPartnerChange(UINT32 controllerSN,
                                                                UINT32 mirrorPartner);

extern PI_MISC_RESYNCDATA_RSP *SM_ResyncData(UINT32 controllerSN);

extern INT32 SM_NVRAMRestoreWithRetries(UINT8 option, UINT8 *pNVRAM, UINT8 retries);
extern INT32 SM_NVRAMRestore(UINT8 option, UINT8 *pNVRAM);

extern INT32 SM_TempDisableCache(UINT32 controllerSN,
                                 UINT32 commandCode, UINT8 user, UINT8 option);

extern PI_MISC_QTDISCACHE_RSP *SM_QueryTempDisableCache(UINT32 controllerSN);


#ifdef __cplusplus
#pragma pack(pop)
#endif

#endif /* __SM_H__ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
