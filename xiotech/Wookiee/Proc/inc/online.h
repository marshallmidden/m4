/* $Id: online.h 161041 2013-05-08 15:16:49Z marshall_midden $ */
/**
******************************************************************************
**
**  @file       online.h
**
**  @brief      Definitions for online variables and functions.
**
**  Copyright (c) 2003-2010 Xiotech Corporation. All rights reserved.
**
******************************************************************************
**/
#ifndef _ONLINE_H_
#define _ONLINE_H_

#include "XIO_Types.h"

#include "ilt.h"
#include "nvr.h"
#include "prp.h"
#include "scsi.h"
#include "target.h"

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/
#define MODE_CACHE_SET_MASK     0x00000050  /* ABPF, DISC                   */
#define MODE_CACHE_CLR_MASK     0xFFFFFF5A  /* IC, CAO, WCE, RCD            */

#define MODE_WRER_SET_MASK      0x000000C0  /* AWRE, ARRE                   */
#define MODE_WRER_CLR_MASK      0xFFFFFFC0  /* TB, RC, EER, DTE, DCR        */

#define MODE_VER_SET_MASK       0x00000000  /* None                         */
#define MODE_VER_CLR_MASK       0xFFFFFFF0  /* EER, PER, DTE, DCR           */

#define MODE_PCNEW_SET_MASK     0x00000000  /* None                         */
#define MODE_PCNEW_CLR_MASK     0xFFFFFFFD  /* IDLE                         */

#define MODE_IEC_SET_MASK       0x00000201  /* LOGERR, MRIE = 2             */
#define MODE_IEC_CLR_MASK       0xFFFFF273  /* MRIE = 2                     */

#define MODE_FCIC_SET_MASK      0x00000000  /* None                         */
#define MODE_FCIC_CLR_MASK      0x00000000  /* All                          */

#define MODE_IEC_SET_INTERVAL_TIMER 0x00001770 /* Interval timer = 60 sec   */
#define IEC_PAGE_CODE               0x1c       /* IE control page           */

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/
struct PDD;

/*
******************************************************************************
** Public variables
******************************************************************************
*/
/* NOTE: "//XIOtech Device Label//" with a extra null byte at the end. */
extern UINT8 O_devlab[25];          /* Device label string                  */
extern UINT32 O_drvinits;           /* Drive inits in progress              */
extern UINT8 O_p2init;              /* Phase II inits complete (T/F)        */
extern NVRII* O_temp_nvram;         /* Pointer to copy of NVRAM             */
extern void* O_p_pdd_list;          /* Pointer to list of drives at POR     */
extern UINT8 O_stopcnt;             /* Count of nested stop commands        */
extern UINT8 O_seagate[];           /* 'SEAGATE ' vendor ID                 */

/*
** PRP templates for specific SCSI CDBs
*/
extern PRP_TEMPLATE gTemplateInquiry;
extern PRP_TEMPLATE gTemplateTestUnitReady;
extern PRP_TEMPLATE gTemplateStartUnit;
extern PRP_TEMPLATE gTemplateStopUnit;
extern PRP_TEMPLATE gTemplateInqSN;
extern PRP_TEMPLATE gTemplateInqDevID;
extern PRP_TEMPLATE gTemplateSendDiag;
extern PRP_TEMPLATE gTemplateWrite;
extern PRP_TEMPLATE gTemplateRead;
extern PRP_TEMPLATE gTemplateVerify;
extern PRP_TEMPLATE gTemplateVerify1;
extern PRP_TEMPLATE gTemplateVerify1_16;
extern PRP_TEMPLATE gTemplateMSAll;
extern PRP_TEMPLATE gTemplateMSCache;
extern PRP_TEMPLATE gTemplateMSRWErr;
extern PRP_TEMPLATE gTemplateMSVErr;
extern PRP_TEMPLATE gTemplateMSPower;
extern PRP_TEMPLATE gTemplateMSException;
extern PRP_TEMPLATE gTemplateMSFC;
extern PRP_TEMPLATE gTemplateMSSASsub1;
extern PRP_TEMPLATE gTemplateReadExt;
extern PRP_TEMPLATE gTemplateWriteExt;
extern PRP_TEMPLATE gTemplateReadRsvd;
extern PRP_TEMPLATE gTemplateWriteSame;
extern PRP_TEMPLATE gTemplateReadBuff;
extern PRP_TEMPLATE gTemplateWriteBuff;
extern PRP_TEMPLATE gTemplateSESP0Rd;
extern PRP_TEMPLATE gTemplateSESP1Rd;
extern PRP_TEMPLATE gTemplateSESP2Rd;
extern PRP_TEMPLATE gTemplateSESP4Rd;
extern PRP_TEMPLATE gTemplateSESP4WWN;
extern PRP_TEMPLATE gTemplateSESP7Rd;
extern PRP_TEMPLATE gTemplateSESP0ARd;
extern PRP_TEMPLATE gTemplateSESP86Rd;
extern PRP_TEMPLATE gTemplateSESP87Rd;
extern PRP_TEMPLATE gTemplateSESP80Rd;
extern PRP_TEMPLATE gTemplateSESP81Rd;
extern PRP_TEMPLATE gTemplateSESP82Rd;
extern PRP_TEMPLATE gTemplateSESP83Rd;
extern PRP_TEMPLATE gTemplateSESP82Wr;
extern PRP_TEMPLATE gTemplateSESP83Wr;

#if defined(MODEL_7000) || defined(MODEL_4700)
extern PRP_TEMPLATE gISEVolumeInfo;
extern PRP_TEMPLATE gTemplateMgmtNetworkPage85;
#endif /* MODEL_7000 || MODEL_4700 */

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/

/*
** Log Events
*/
extern void ON_LogBayInserted(struct PDD *pPDD);
extern void ON_LogBayMissing(struct PDD *pPDD);
extern void ON_LogBayMoved(struct PDD *pPDD);
extern void ON_LogDriveInserted(struct PDD *pPDD);
extern void ON_LogDriveMissing(struct PDD *pPDD);
extern void ON_LogDriveReattached(struct PDD *pPDD);
extern void ON_LogDeviceSPath(struct PDD *pPDD);
extern void ON_LogSerialChanged(struct PDD *pPDD);
extern void ON_LogError(UINT32 errorCode);
extern void ON_LogDriveDelay(PDD* pPDD);

/*
** Physical Request Handlers
*/
extern ILT *ON_GenReq(PRP_TEMPLATE* pTemplate, struct PDD *pPDD,
                void* * pDataBuffer, PRP* * pPRP);
extern void ON_QueReq(ILT* pILT);
extern void ON_RelReq(ILT* pILT);

/*
** Misc
*/
extern UINT32 ON_CreateDefaults(UINT32 cncSerial);
extern void ON_CreateTargetWWN(TGD *pTGD);
extern void ON_Init(void);
extern void ON_InitDrive(struct PDD *pPDD, UINT32 type, void *pInitCount);
extern void ON_LedChanged(struct PDD *pPDD);
extern UINT32 ON_ModeSenseSelect(PRP_TEMPLATE* pTemplate, struct PDD *pPDD,
                UINT32 orMask, UINT32 andMask);
extern void ON_Resume(void);
extern UINT32 ON_SCSICmd(void* template, void* outputBuffer, UINT32 size,
                struct PDD *pPDD, SNS **pSenseData);
extern UINT32 ON_BypassCmd(void* template, void* outputBuffer, UINT32 size,
                struct PDD *pPDD, SNS **pSenseData, UINT8 channel, UINT8 lid);
extern void ON_Stop(void);
extern void ON_MigrateOldPDDtoNewPdd(PDD * oldPDD,PDD * NewPdd , PDD *GlobalArray[] );

#endif /* _ONLINE_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
