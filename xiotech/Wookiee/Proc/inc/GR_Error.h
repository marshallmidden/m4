/* $Id: GR_Error.h 155893 2011-05-19 18:16:01Z m4 $ */
/**
******************************************************************************
**
**  @file       GR_Error.h
**
**  @brief      Geo-RAID -Vdisk failover/failback modules related
*               definitions and prototypes
**
**  To provide a common means of defining Geo-RAID vdisk error tracking,
**  error handling, autoswap/autoswapback components related definitions
**  and prototypes
**
**  SW components
**  -------------
**
** >>> Error Tracking Component -- GR_ErrorTrack.c
**
** >>> Error Handling Component -- GR_ErrorHandle.c
**
** >>> Auto Swap Component      -- GR_AutoSwap.c
**
** >>> Misc/API component       -- GR_Misc.c
**
**  Copyright (c) 2005-2010 XIOtech Corporation.  All rights reserved.
**
******************************************************************************
**/
#ifndef _GR_ERROR_H_
#define _GR_ERROR_H_

#include "XIO_Types.h"
#include "globalOptions.h"
#include "XIO_Macros.h"
#include "vrp.h"
#include "cor.h"
#include "ilt.h"
#include "LOG_Defs.h"
#include "MR_Defs.h"
#include "GR_AutoSwap.h"

/*
******************************************************************************
** Public defines - constants
******************************************************************************
*/
#define GR_MYDEBUG1 FALSE

/*
** Following are the values of the new fields added to VDD structure for
** --------------------------------------------------------------------
**
** handling geo raid vdisk auto failover and failback.
** ------------------------------------------------------------
*/

/*
** New members in VDD structure - vdOpState
** ---------------------------------------
** The following defines the valid values for field virtual device operational
** state. It contain the values corresponding to vd_status values and gets
** updated whenever vd_status changed from rebuild, online etc.. modules.
** This is 3 bit field -- Hence max.value allowed is 0 to 7 only
*/
#define GR_VD_INOP             0
#define GR_VD_UNINIT           1
#define GR_VD_INIT             2
#define GR_VD_OP               3
#define GR_VD_DEGRADED         4
#define GR_VD_IOSUSPEND        5

/*
** New member in VDD structure - permFlags  (allowed bits are  0 - 3)
** -----------------------------------------------------------------
**          (Saved in NVRAM)
*/

#define GR_VD_GEORAID_BIT                    0
   /*
   ** bit-0    : This  bit represents, whether the vdisk is of geo-Raid type
   **            or not. This is set/cleared during vdisk creation/expansion
   **            and bay location change.
   */
#define GR_VD_ABSOLUTE_GEOPARNTER_BIT        1
   /*
   ** bit-1    : This is the destination (copy/mirror) partner that is entirely
   **            at different location(s) as that of its source partner.
   **            This is set/cleared during copy/manager configuration change
   **            and bay location change.
   */
#define GR_VD_ASWAP_BIT                      2
   /*
   ** bit-2    : Set on source VDD(failed VDD) after auto swap is done. This
   **            is used to avoid consecutive auto swap on the same VDD.
   **            Second auto swap is not allowed, till its autoswap back is
   **            is finished.
   */
#define GR_VD_ASWAPBACK_PENDING_BIT          3
   /*
   ** bit-3    : Set on destination VDD. This is used to schedule the autoswap
   **            back when  the destination (pointed to failed raids) comes to
   **            operation and sync.
   **
   */

/*
** New member in VDD structure -- allDevMiss SYNC flag -- TBD
** A special flag that is set on the destination VDDs on the controller that is
** failing due to all BackEnd missing
*/

/*
** New member in VDD structure - tempFlags (allowed bits are 0 - 7)
** ----------------------------------------------------------------
**          (Not saved in NVRAM)
*/

#define GR_VD_ASWAP_PROGRESS_BIT       0
  /*
  ** bit - 0 : Set on Src vdd when the aswap request is initiated/submitted/
  **           autoswap in progress. It is cleared when the Aswap is finished.
  **           This is used to avoid multiple submission of aswap operation on
  **           the same VDD.
  */
#define GR_VD_ASWAPBACK_PROGRESS_BIT   1
  /*
  ** bit - 1 : Set on Src vdd when the aswapback  is initiated/submitted/
  **           autoswapback in progress. It is cleared when the aswapback is
  **           finished or failed.
  **           This is used to avoid multiple submission of aswap backs on
  **           the same VDD.
  */

#define GR_VD_ASWAP_FAILED_BIT         2
  /*
  ** bit - 2:  This is used by autoswap component to let the autoswap task knows
  **           about the autoswap operation it has issued.   This bit is set
  **           in the autoswap call back function , when autoswap  is failed. By
  **           looking at this bit, the autoswap component can decide whether to
  **           try the autoswap  with another mirror or not.
  */
#define GR_VD_AUTOSWAP_OPTYPE_BIT      3
   /*
   ** bit-3    : Indicates whether the requested operation is either autoswap
   **            or autoswap back. If it is set, this is autoswap , otherwise
   **            the requested operation is autoswap back.
   */
#define GR_VD_HYSTERESIS_BIT           4
   /*
   ** bit-4    : Set on source VDD (failed VDD) after swapback is done.
   **            This is used to apply the hysteresis concept on the
   **            sequence of swap and swapback operations.
   */
#define GR_VD_INSYNC_AT_SRCFAIL_BIT    5
   /*
   ** bit-5    : This is set on  all the destination VDDs on when its source
   **            VDD is failed at write operation.
   **
   */
#define GR_VD_ASWAPBACK_NOT_READY_BIT    6
   /*
   ** bit-6    : This is set on  the destination VDD, when autoswap is done.
   **            It is cleared only when autoswap back enteres READY state.
   **
   */

#define GR_VD_SWAP_DONE_AT_OWNER        7
  /*
  ** bit-7     : This indicates that the raid pointers swapped. This is set on
  **             source Vdisk.
  */

/*
**          VALUES  FOR  AUTO SWAP STATE TRANSITION
**          ----------------------------------------
** These are used during auto swap  state transition mechanism during vdisk
** failover/failback handling(auto swap/swapback) and auto-swap/swapback
** completion handling.
*/
#define GR_ASWAP_START                 1
#define GR_ASWAP_SUCCESS               2
#define GR_ASWAP_FAILED                3
#define GR_ASWAPBACK_RECOVERY_WAIT     4
#define GR_ASWAPBACK_READY             5
#define GR_ASWAPBACK_START             6
#define GR_ASWAPBACK_SUCCESS           7
#define GR_ASWAPBACK_FAILED            8
#define GR_ASWAPBACK_CANCELLED         9
#define GR_ASWAPBACK_HYSTERESIS_WAIT  10

/* IO Types */
#define GR_READ_REQ    1
#define GR_WRITE_REQ   2
#define GR_UNKNOWN_REQ 0

/*
**          V A L U E S  --  auto swap operation types.
**          -------------------------------------------
*/
#define GR_VD_ASWAP_OP        1
#define GR_VD_ASWAPBACK_OP    2

/*
** Vdisk Opstate APIs related Macro definitions
*/
#define GR_VDOPSTATE_CURRENT        0
#define GR_VDOPSTATE_GIVEN          0
#define GR_VDOPSTATE_BY_RDDSTAT     1
#define GR_VDOPSTATE_BY_PSDSTAT     2
#define GR_VDOPSTATE_BY_PDDSTAT     3

/*
** Geo mirror set types
*/
#define GR_MIRRORSET_NON_GEORAID      0
#define GR_MIRRORSET_ABSOLUTE_GEORAID 1
#define GR_MIRRORSET_GEORAID          2

/*
******************************************************************************
** Public defines - macros
******************************************************************************
*/
/*
** Macros used by applications
*/

#define GR_IS_VD_ASWAP_INPROGRESS(vdd)\
    (BIT_TEST((vdd->grInfo).tempFlags,GR_VD_ASWAP_PROGRESS_BIT)== TRUE)

#define GR_IS_VD_WAITING_FOR_ASWAP_COMP(vdd)\
    (BIT_TEST((vdd->grInfo).tempFlags,GR_VD_ASWAP_PROGRESS_BIT)== TRUE)

#define GR_IS_VD_ASWAPBACK_INPROGRESS(vdd)\
    (BIT_TEST((vdd->grInfo).tempFlags,GR_VD_ASWAPBACK_PROGRESS_BIT)== TRUE)
#define GR_IS_VD_WAITING_FOR_ASWAPBACK_COMP(vdd)\
    (BIT_TEST((vdd->grInfo).tempFlags,GR_VD_ASWAPBACK_PROGRESS_BIT)== TRUE)

#define GR_VD_ASWAP_OPTYPE(vdd)\
    (BIT_TEST((vdd->grInfo).tempFlags,GR_VD_AUTOSWAP_OPTYPE_BIT)== TRUE)?\
     GR_VD_ASWAP_OP : GR_VD_ASWAPBACK_OP
#define GR_VD_SET_AUTOSWAP_OPTYPE(vdd,opType)\
     (void)((opType == GR_VD_ASWAP_OP)\
     ?BIT_SET((vdd->grInfo).tempFlags,GR_VD_AUTOSWAP_OPTYPE_BIT)\
     :BIT_CLEAR((vdd->grInfo).tempFlags,GR_VD_AUTOSWAP_OPTYPE_BIT));

#define GR_IS_VD_ALREADY_ASWAPPED(vdd) \
    (BIT_TEST((vdd->grInfo).permFlags,GR_VD_ASWAP_BIT )== TRUE)

#define GR_IS_VD_HYSTERESIS_ENABLED(vdd) \
    (BIT_TEST((vdd->grInfo).tempFlags, GR_VD_HYSTERESIS_BIT )== TRUE)

#define GR_IS_PARNTER_AT_DIFFERENT_LOCATION(vdd) \
    (BIT_TEST((vdd->grInfo).permFlags,GR_VD_ABSOLUTE_GEOPARNTER_BIT)== TRUE)

#define GR_IS_VD_INSYNC_AT_SRCFAIL(vdd)\
    (BIT_TEST((vdd->grInfo).tempFlags,GR_VD_INSYNC_AT_SRCFAIL_BIT)== TRUE)

#define GR_SET_VD_INSYNC_AT_SRCFAIL(vdd)\
  BIT_SET((vdd->grInfo).tempFlags, GR_VD_INSYNC_AT_SRCFAIL_BIT)

#define GR_CLEAR_VD_INSYNC_AT_SRCFAIL(vdd)\
  BIT_CLEAR((vdd->grInfo).tempFlags, GR_VD_INSYNC_AT_SRCFAIL_BIT)

#define GR_IS_VD_ASWAPBACK_PENDING(vdd)\
    (BIT_TEST((vdd->grInfo).permFlags,GR_VD_ASWAPBACK_PENDING_BIT)== TRUE)

#define GR_IS_ASWAP_FAILED(vdd)\
    (BIT_TEST((vdd->grInfo).tempFlags,GR_VD_ASWAP_FAILED_BIT)== TRUE)

#define GR_SET_ASWAP_FAILED(vdd)\
    (BIT_SET((vdd->grInfo).tempFlags,GR_VD_ASWAP_FAILED_BIT))

#define GR_CLEAR_ASWAP_FAILED(vdd)\
    (BIT_CLEAR((vdd->grInfo).tempFlags,GR_VD_ASWAP_FAILED_BIT))

#define GR_IS_GEORAID_VDISK(vdd)\
    (BIT_TEST((vdd->grInfo).permFlags, GR_VD_GEORAID_BIT)== TRUE)

#define GR_IS_NONGEOAID_VDISK(vdd)\
    (BIT_TEST((vdd->grInfo).permFlags, GR_VD_GEORAID_BIT)== FALSE)

#define GR_IS_VD_ASWAPBACK_IN_READY_STATE(vdd)\
    (BIT_TEST((vdd->grInfo).tempFlags, GR_VD_ASWAPBACK_NOT_READY_BIT)== FALSE)

#define SET_GR_VD_SWAP_DONE_AT_OWNER(vdd)\
    (BIT_SET((vdd->grInfo).tempFlags, GR_VD_SWAP_DONE_AT_OWNER))

#define CLEAR_GR_VD_SWAP_DONE_AT_OWNER(vdd)\
    (BIT_CLEAR((vdd->grInfo).tempFlags, GR_VD_SWAP_DONE_AT_OWNER))

#define IS_GR_VD_SWAP_DONE_AT_OWNER(vdd)\
    (BIT_TEST((vdd->grInfo).tempFlags, GR_VD_SWAP_DONE_AT_OWNER) == TRUE)
/*
******************************************************************************
** Public defines - data structures
******************************************************************************
*/
struct VDD;

/*
******************************************************************************
** Public function prototypes
******************************************************************************
*/

/*
** Functions related to Error Tracking Component
** ---------------------------------------------
**          (GR_ErrorTrack.c)
*/

void   GR_ResetVRPILT                    (ILT *, VRP *, VDD *);
UINT32 GR_VerifyVdiskOpState             (VDD *, VRP *);
UINT32 GR_IsCandidateVdisk               (VDD *, VRP *);
void   GR_UpdateVdiskOpState             (VDD *, UINT8, UINT8);
UINT8  GR_GetVdiskOpState                (VDD *, UINT8);
UINT8  GR_GetVdiskStatus                 (VDD *, UINT8);

void   GR_ResetIOSuspendState            (VDD *);
void   GR_SetIOSuspendState              (VDD *);
UINT32 GR_IsInIOSuspendState             (VDD *);

/*
** Functions related to Error Handling Component
** ---------------------------------------------
**          (GR_ErrorHandle.c)
*/
void GR_InitErrorQueHandler      (void);
void GR_SubmitVdiskError         (VDD *, VRP *);
void GR_PostToVdiskErrorQueue_1  (ILT *);
void GR_PostToVdiskErrorQueue_2  (ILT *);
/*void GR_VdiskErrorHandlerTask    (void);*/
void GR_VdiskErrorCompleter      (VDD* pSrcVDD, VDD* DestVDD, UINT8 status);
void GR_InformVdiskSyncState     (VDD* pSrcVDD, VDD * pDestVDD);
void GR_InformVdiskOpState       (VDD*);
void GR_InitiateErrorRecovery    (VDD *, VDD*);
void GR_RetryPendingFailBacks(void);
void GR_SetSwapDoneFlagOnSrcVDD  (VDD *);

void GR_SetCopyMirrorInfoDCNFail(UINT8 *);
void GR_PrepareVDMapOfFailedDCN(UINT8 * );

/*** GR_GEORAID15 */

void   GR_ClearLocalImageIP          (void);
void   GR_SetVdiskInfoAtAllDevMiss   (LOG_ALL_DEV_MISSING_DAT *);
UINT32 GR_HandleAllDevMissAtOtherDCN (MR_PKT *);
void   GR_SetStillInSyncFlag         (VDD *);
void   GR_ClearStillInSyncFlag       (VDD *);
UINT32 GR_IsStillInSyncFlag          (VDD *);
UINT32 GR_IsAllDevMissSyncFlagSet    (VDD *);
void   GR_ClearAllDevMissSyncFlag    (VDD *);
void   GR_SetAllDevMissSyncFlag      (VDD *);
void   GR_SetRemovedBayMap           (UINT16);
UINT32 GR_IsDriveRemoved             (PDD *);
void   GR_ResetAllDevMissFlags       (void);
void   GR_SetSpecialAswapFlag        (VDD *);
void   GR_ClearSpecialAswapFlag      (VDD *);
UINT32 GR_IsSpecialAswapFlag         (VDD *);

/*
** Misc Functions / APIs
** --------------------------
**   (GR_Misc.c)
*/
extern void GR_UpdateAllVddGeoInfo      (UINT8 locationChgOpt);
extern void GR_UpdateVddGeoInfo         (VDD* pVDD);
extern void GR_UpdateAllVddPartners     (void);
extern void GR_UpdateVddPartners        (VDD *pVDD);
extern UINT8 GR_IsAnyAutoSwapInProgress (void);
extern UINT32 GR_IsAutoSwapInProgress   (VDD* pSrcVDD);
extern void GR_UpdateGeoInfoAtCmdMgrReq (COR* pCOR, UINT8 operation);
extern void GR_SendAswapLogEvent        (VDD* ,VDD* ,UINT8);
extern UINT8 GR_SetGeoMirrorSetType     (MR_PKT*);
extern UINT32 GR_IsValidOpStateForAutoSwap(VDD*);
extern UINT32 GR_AnyIosArePending(VDD* pSrcVDD);
extern void GR_SaveToNvram (VDD *pSrcVDD, VDD* pDestVDD);
extern void GR_NvramUpdatePkt (GR_NVRAM_PKT* nvramPkt);
extern void GR_LogEvent                 (UINT8 eventType,VDD* pSrcVDD, VDD* pDestVDD);
#endif /* GR_ERROR_H_*/

/***
** Modelines:
** Local Variables:
** tab-width: 4
** indent-tabs-mode: nil
** End:
** vi:sw=4 ts=4 expandtab
**/
