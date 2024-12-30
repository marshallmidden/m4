/* $Id: def_con.h 159129 2012-05-12 06:25:16Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       Def_con.h
**
**  @brief      VDisk/RAID/PDisk configuration for the define component
**
**  Copyright (c) 2003-2010 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _DEF_CON_H_
#define _DEF_CON_H_

#include "XIO_Types.h"

#include "cev.h"
#include "DEF_Workset.h"
#include "globalOptions.h"
#include "RL_PSD.h"
#include "RL_RDD.h"
#include "vdd.h"

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
extern DEF_WORKSET  gWorksetTable[DEF_MAX_WORKSETS];
extern UINT8 VPri_enable;

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/

/* MRPs */
extern UINT8 DEF_GetWorkset(struct MR_PKT *pMRP);
extern UINT8 DEF_SetWorkset(struct MR_PKT *pMRP);

#if defined(MODEL_7000) || defined(MODEL_4700)
extern UINT8 DEF_GetISEIP(struct MR_PKT *pMRP);
#endif /* MODEL_7000 || MODEL_4700 */

/* Misc */
extern struct PDD *DC_AllocPDD(void);
extern PSD   *DC_AllocPSD(void);
extern RDD   *DC_AllocRDD(UINT32 numPSDs);
extern VDD   *DC_AllocVDD(void);
extern PSD   *DC_ConvPDD2PSD(struct PDD *pPDD);
extern void   DC_DeleteRAID(RDD *pRDD);
extern void   DC_RelPDD(struct PDD *pPDD);
extern void   DC_RelRDDPSD(RDD *pRDD);
extern void   DEF_GetRDA(CEV *pCEV);
extern UINT32 DEF_CheckDeviceType(CEV *pCEV);
extern UINT8  DEF_IsOrphanRAID(UINT16 raidID);
extern void   DEF_LogOrphanRAID(UINT16 raidID);
extern void   D_SetVPri (UINT32 n, UINT16 Vid, UINT8 pri);
extern UINT8  DEF_SetVPri(struct MR_PKT *pMRP);
extern UINT8  DEF_VPriEnable(struct MR_PKT *pMRP);
extern UINT32 GetSysTime(void);
extern void   DEF_AllocMemoryForVDStats (VDD*);
extern void   DEF_DeallocVDStatsMemory (VDD*);
extern void   DEF_VdiskLastHrStats (VDD*);
extern UINT8  check_vdisk_XIO_attached(UINT16);
extern UINT32 dlm_cnt_servers(UINT16);

#endif /* _DEF_CON_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
