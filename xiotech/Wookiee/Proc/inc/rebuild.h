/* $Id: rebuild.h 143007 2010-06-22 14:48:58Z m4 $ */
/**
******************************************************************************
**
**  @file       rebuild.h
**
**  @brief      Rebuild and hotspare code
**
**  Contains the prototype definitions and data to support hotsparing,
**  rebuilds, and device status updates.
**
**  Copyright (c) 2003-2010 XIOtech Corporation.  All rights reserved.
**
******************************************************************************
**/
#ifndef _REBUILD_H
#define _REBUILD_H

#include "XIO_Types.h"

#include "ilt.h"
#include "options.h"
#include "MR_Defs.h"
#include "pcb.h"
#include "prp.h"
#include "rbr.h"
#include "RL_PSD.h"
#include "RL_RDD.h"

/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/
extern bool    gAutoFailBackEnable;

/**
** This structure is used to put the HotSpare drive back in operational after
** the failback or AutoFailBack is done.
**/
typedef struct CCSM_E_RB
{
    UINT32      len;            /* event length                             */
    UINT8       type;           /* event type code                          */
    UINT8       fc;             /* event function code                      */
    UINT16      seq;            /* sequence #                               */
    UINT32      sendsn;         /* sender's serial #                        */

}CCSM_E_RB;

typedef struct RB_FAIL_BACK
{
    UINT16      HotSparePid;   /* PID of the HotSpare                       */
    UINT16      DestPid;       /* PID of the Destination PID                */
} RB_FAIL_BACK;


/**
** This structure is used to send datagram message to the Master controller
** to do filesystem update in the case of AutoFailBack/FailBack feature
**/
typedef struct RB_FAILBACK_UPDATE
{
    CCSM_E_RB      ccsm_event;   /* CCSM Event Header                       */
    RB_FAIL_BACK   failBack;     /* PIDs for the FailBack                   */

}RB_FAILBACK_UPDATE;

/*
** FailBack event type definition
*/
#define RB_EVT_FAILBACK    7   /* FailBack event                            */

/*
** Non-op event type definition
*/
#define RB_INOP_EVT         8   /* FailBack event                            */

/*
**  FailBack event function code
*/
#define RB_FAILBACK_COMP  0x50 /** FailBack complete                        */

/*
** Non-Op event function code
*/
#define RB_HS_INOP        0x51 /** Drive Non-Op                             */

/*
******************************************************************************
** Public variables
******************************************************************************
*/

extern RBR*         gRBRHead;
extern UINT32       gHotspareWait;
extern PCB*         gHotspareWaitPCB;
extern PCB*         gRLLocalImageIPCheckPCB;

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/

/*
** Device Status
*/
extern void RB_setpsdstat(void);
extern void RB_setraidstat(RDD* pRDD);
extern void RB_SetVirtStat(void);
extern void RB_CalcPercentRemaining(PDD* pPDD);

/*
** RAID Error Handling
*/
extern void RB_AcceptIOError(void);
extern void RB_RAIDError(PSD* pPSD, RDD* pRDD, PRP* pPRP, ILT* pILT);
extern void RB$rerror_que(ILT* pILT);
extern void RB_RAIDErrorComp(ILT* pILT);
extern void RL_LocalImageIPCheck(void);

/*
** MRPs
*/
extern UINT8 RB_FailDevice(MR_PKT* pMRP);

/*
** Rebuild Control
*/
/* void RB_PauseRebuild(void); */
extern void RB_RedirectPSD(PSD* pFailingPSD, PDD* pHotSpare);
extern void RB_HotspareWaitTask(void);

/*
** Misc
*/
extern UINT8 RB_CanSpare(PSD* pPSD);
extern UINT8 RB_ActDefCheck(PDD* pPDD);
extern UINT8 RB_InOpCheck(UINT8 cSES, UINT8 cSlot, PDD* pPDD, UINT8 hsOnly, PDD** ppPDD);
extern PDD* RB_FindHotSpare(UINT64 capacity, UINT16 pid);
extern void RB_SearchForFailedPSDs(void);
extern void RB_LogHSDepleted(UINT16 type, UINT16 devID, UINT8 devType);
extern void RB_CheckHSDepletion(PDD* pInputPDD);
extern void RB_CheckHSCapacity(PDD* pInputPDD);

extern void RB_CalcAddressRange(UINT16 rid, UINT32 length, UINT64 sda, UINT64* pSDA, UINT64* pEDA);
extern bool RB_IsRAIDRebuildWriteActive(RDD* pRDD);
extern bool RB_IsPSDRebuildWriteActive(PSD* pPSD);
extern void RB_UpdateRebuildWriteState (bool updateRMTCache,  bool p2Update);

extern void RB_CheckRBRemainingIntegrity (void);

/* Added following prototype for GEORAID features */
extern UINT8 rb$getraiderrorstat(RDD* pRDD);

/*
** Pdisk failback from hotspare to data drive related functions.
*/
extern UINT8  RB_pdiskFailBack (UINT16 hspid,  UINT8 options);
extern UINT8  RB_AutoFailBackEnableDisable (MR_PKT *);
void RB_LabelDestDrive (PDD*);

#endif      /* _REBUILD_H_ */

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
