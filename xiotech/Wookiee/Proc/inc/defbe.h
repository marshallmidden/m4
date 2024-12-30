/* $Id: defbe.h 145021 2010-08-03 14:16:38Z m4 $ */
/**
******************************************************************************
**
**  @file       defbe.h
**
**  @brief      BE Define operations
**
**  Copyright (c) 2003-2010 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _DEFBE_H_
#define _DEFBE_H_

#include "XIO_Types.h"

#include "ldd.h"
#include "nvr.h"
#include "RL_PSD.h"
#include "RL_RDD.h"
#include "sdd.h"
#include "target.h"
#include "vdd.h"
#include "vlar.h"

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/

struct PDD;
struct MR_PKT;

/*
******************************************************************************
** Public variables
******************************************************************************
*/
extern UINT8  D_glcache;
extern UINT8  D_gpri;
extern UINT8  D_ften;
extern UINT8  D_p2writefail_flag;  /* set when the P2 write verify fails */
extern UINT16 D_moveinprogress;
extern MPMAP  D_mpmaps[MAX_CTRL];
extern UINT8  D_mpcnt;

extern LDD*    DLM_lddindx[MAX_LDDS];

extern struct PDD   *P_pddindx[MAX_PHYSICAL_DISKS];
extern RDD*    R_rddindx[MAX_RAIDS];
extern VDD*    V_vddindx[MAX_VIRTUAL_DISKS];
extern SDD*    S_sddindx[MAX_SERVERS];
extern struct PDD   *E_pddindx[MAX_DISK_BAYS];
extern struct PDD   *M_pddindx[MAX_MISC_DEVICES];
extern TGD*    T_tgdindx[MAX_TARGETS];

extern SDX     gSDX;
extern VDX     gVDX;
extern RDX     gRDX;
extern PDX     gPDX;
extern TDX     gTDX;
extern EDX     gEDX;
extern MDX     gMDX;

extern UINT32  d_resync_paskey;

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/

extern UINT32 DEF_AmIOwner(UINT16 vid);
extern UINT16 DEF_CheckForMapped(UINT16 vid);
extern struct PDD *DEF_FindPDD(UINT8 port, UINT32 fcid, UINT16 lun);
extern struct PDD *DEF_FindPDDWWN(UINT64 wwn, UINT16 lun);
extern void DEF_InsertPDD(struct PDD *pPDD);
extern void DEF_QueRInit(RDD* pRDD);

extern void DEF_ReportEvent(void* nvram, UINT32 extSize, UINT32 event,
                    UINT32 broadcastType, UINT32 serialNumber);

extern void DEF_SignalServerUpdate(void);
extern void DEF_SignalVDiskUpdate(void);
extern void DEF_UMiscStat(void);
extern void DEF_UpdRmtCache(void);
extern void DEF_UpdRmtCacheGlobal(void);
extern void DEF_ResetConfig(void);
extern void DEF_TerminateBackground(void);
extern void DEF_UpdRmtCacheSingle(UINT16 vid, UINT16 options);
extern void DEF_UpdRmtServer(UINT16 sid, UINT16 options);
extern void DEF_UpdRmtSysInfo(void);
extern void DEF_UpdRmtTarg(UINT16 sid, UINT16 options);
extern UINT8 DEF_CacheStop(UINT8 options, UINT8 user);
extern void DEF_CacheResume(UINT8 options, UINT8 user);
extern UINT16 DEF_GetVDiskAndCopyTreeAttrs(UINT16 vid);
extern void DEF_SndConfigOpt(void);
extern void DEF_ValidateTarget(TGD *pTGD, UINT8 status);
extern UINT8 DEF_AsyncRep(struct MR_PKT *pMRP);
extern UINT8 DEF_BEQlogicTimeout(struct MR_PKT *pMRP);

#endif /* _DEFBE_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
