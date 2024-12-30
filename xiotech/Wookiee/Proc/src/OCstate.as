# $Id: OCstate.as 143007 2010-06-22 14:48:58Z m4 $
#**********************************************************************
#
#  Copyright (c) 2004-2010 XIOtech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- assembly options ------------------------------------------------
#
        .data

#**********************************************************************
#
#   --- testing variables
#
#**********************************************************************

        .set    s.cs,0xff

.ifndef I_IS_DEFINED
.set I_IS_DEFINED,1
        .text
i:
        ret
        .data
.endif  /* I_IS_DEFINED */
#**********************************************************************
#
#   --- Table Name
#
#**********************************************************************

        tablename(OCtbl)

#**********************************************************************
#
#   State definitions
#
#**********************************************************************

    state(SB_Idle)
    state(SC_PendTaskRdy)
    state(SD_DefnOwnership)
    state(SE_SuspendCurOwner)

    state(SF_ReqRegionMap)
    state(SF2_ReqSegmentMap)
    state(SG_TermCurOwner)
    state(SH_RetestOwnership)

    state(SI_NoResources)
    state(SP_ChgOwnership)
    state(SR_OwnerSuspended)
    state(SS_SuspendingOwner)

    state(ST_CpyOwned)
    state(SZ_TermCopy)

#**********************************************************************
#
#   Event Definitions
#
#**********************************************************************

    event(Cpy_Config_Chg)
    event(Start_Copy)
    event(Term_Copy)
    event(cc_Copy_Ready)

    event(cc_Owner_Suspended)
    event(Read_Dirty_Segment_Map)
    event(Dirty_Segment_Map)
    event(Suspend_Owner)

    event(Trans_Owner)
    event(Term_Owner)
    event(Read_Dirty_Region_Map)
    event(You_R_Owner)

    event(Owner_Chg)
    event(Owner_Suspended)
    event(Owner_Term)
    event(Dirty_Region_Map)

    event(HaveResources)
    event(NotHaveResources)
    event(cc_Copy_Terminated)
    event(Timeout)

    event(CurOpRtyExp)
    event(CurOpRtyNotExp)
    event(ProcRtyExp)
    event(ProcRtyNotExp)

    event(Done)
    event(Chk4Owner)

#**********************************************************************
#
#   Action Definitions
#
#**********************************************************************

    action(i)
    action(CCSM$Ready_Copy)
    action(CCSM$Test4Resources)
    action(CCSM$Define_own)
    action(CCSM$OwnerAcq)
    action(CM$BreakCopy)
    action(CCSM$CopyTerminated)
    action(CCSM$SuspCurOwn)
    action(CCSM$CurOwnSusp)
    action(CCSM$DirtyRM)
    action(CCSM$OwnTerm)
    action(CCSM$SuspOwn)
    action(CCSM$SuspOwn2)
    action(CCSM$SuspOwn3)
    action(CCSM$OwnSusp)
    action(CCSM$RDRMap)
    action(CCSM$TermOwn)
    action(CCSM$OwnChanged)
    action(CCSM$OpTimeout)
    action(CCSM$ProcTimeout)
    action(CCSM$ResetOCSE)
    action(CCSM$RtyReadRM)
    action(CCSM$RtyTermOwn)
    action(CCSM$ResetTransRM)
    action(CCSM$DeallocSM)
    action(CCSM$RDSMap)
    action(CCSM$DirtySM)
    action(CCSM$RtyReadSM)
    action(CCSM$GetNextSM)
    action(CCSM$ForceOwn)
    action(CCSM$IAmOwner)
    action(CCSM$EndProcess)
    action(CCSM$SuspCCSE)
    action(CCSM$Clrflags)
    action(CCSM$ClrRM_flags)
    action(CCSM$LogAqrOwnrshp)
    action(CCSM$LogOwnrshpTerm)
    action(CCSM$LogOwnrshpForce)

#**********************************************************************
#
#    Copy not initialized
#
#**********************************************************************

    column(SB_Idle)
        node(i, i, i, cs)                           # Copy Configuration Changed
        node(CCSM$Ready_Copy, i, i, SC_PendTaskRdy) # Start Copy
        node(CM$BreakCopy, i, i, SZ_TermCopy)       # Terminate Copy
        node(i, i, i, cs)                           # (cc) Copy Ready

        node(i, i, i, cs)                           # (cc) Ownership Suspended
        node(i, i, i, cs)                           # (gm) Read Dirty Segment Map
        node(CCSM$DeallocSM, i, i, cs)              # (gm) Dirty Segment Map
        node(i, i, i, cs)                           # (gm) Suspend Ownership

        node(i, i, i, cs)                           # (gm) Transfer Ownership
        node(i, i, i, cs)                           # (gm) Terminate Ownership
        node(i, i, i, cs)                           # (gm) Read Dirty Region Map
        node(i, i, i, cs)                           # (gm) You Are Owner

        node(i, i, i, cs)                           # (gm) Ownership Changed
        node(i, i, i, cs)                           # (gm) Ownership Suspended
        node(i, i, i, cs)                           # (gm) Ownership Terminated
        node(i, i, i, cs)                           # (gm) Dirty Region Map

        node(i, i, i, cs)                           # Have Resources
        node(i, i, i, cs)                           # Not Have Resources
        node(CCSM$CopyTerminated, i, i, cs)         # (cc) Copy Terminated
        node(i, i, i, cs)                           # Timeout

        node(i, i, i, cs)                           # Current Op. Retry Count Expired
        node(i, i, i, cs)                           # Current Op. Rty. Cnt. Not Exp.
        node(i, i, i, cs)                           # Process Retry Count Expired
        node(i, i, i, cs)                           # Process Rty. Cnt. Not Exp.

        node(i, i, i, cs)                           # Done
        node(i, i, i, cs)                           # Check if owner

#**********************************************************************
#
#   --- Pending Task Ready
#
#**********************************************************************

    column(SC_PendTaskRdy)
        node(i, i, i, cs)                           # Copy Configuration Changed
        node(i, i, i, cs)                           # Start Copy
        node(CM$BreakCopy, i, i, SZ_TermCopy)       # Terminate Copy
        node(CCSM$Test4Resources, i, i, cs)         # (cc) Copy Ready

        node(i, i, i, cs)                           # (cc) Ownership Suspended
        node(i, i, i, cs)                           # (gm) Read Dirty Segment Map
        node(CCSM$DeallocSM, i, i, cs)              # (gm) Dirty Segment Map
        node(i, i, i, cs)                           # (gm) Suspend Ownership

        node(i, i, i, cs)                           # (gm) Transfer Ownership
        node(i, i, i, cs)                           # (gm) Terminate Ownership
        node(i, i, i, cs)                           # (gm) Read Dirty Region Map
        node(i, i, i, cs)                           # (gm) You Are Owner

        node(i, i, i, cs)                           # (gm) Ownership Changed
        node(i, i, i, cs)                           # (gm) Ownership Suspended
        node(i, i, i, cs)                           # (gm) Ownership Terminated
        node(i, i, i, cs)                           # (gm) Dirty Region Map

        node(CCSM$ResetOCSE, CCSM$Define_own, i, SD_DefnOwnership) # Have Resources
        node(CCSM$EndProcess, CCSM$ClrRM_flags, CCSM$SuspCCSE, SI_NoResources)
                                                    # Not Have Resources
        node(CCSM$CopyTerminated, i, i, cs)         # (cc) Copy Terminated
        node(i, i, i, cs)                           # Timeout

        node(i, i, i, cs)                           # Current Op. Retry Count Expired
        node(i, i, i, cs)                           # Current Op. Rty. Cnt. Not Exp.
        node(i, i, i, cs)                           # Process Retry Count Expired
        node(i, i, i, cs)                           # Process Rty. Cnt. Not Exp.

        node(i, i, i, cs)                           # Done
        node(i, i, i, cs)                           # Check if owner

#**********************************************************************
#
#   --- Define ownership
#
#**********************************************************************

    column(SD_DefnOwnership)
        node(CCSM$Test4Resources, i, i, cs)         # Copy Configuration Changed
        node(i, i, i, cs)                           # Start Copy
        node(CM$BreakCopy, i, i, SZ_TermCopy)       # Terminate Copy
        node(i, i, i, cs)                           # (cc) Copy Ready

        node(i, i, i, cs)                           # (cc) Ownership Suspended
        node(i, i, i, cs)                           # (gm) Read Dirty Segment Map
        node(CCSM$DeallocSM, i, i, cs)              # (gm) Dirty Segment Map
        node(i, i, i, cs)                           # (gm) Suspend Ownership

        node(CCSM$ResetTransRM, CCSM$SuspCurOwn, i, SE_SuspendCurOwner) #
                                                    # (gm) Transfer Ownership
        node(i, i, i, cs)                           # (gm) Terminate Ownership
        node(i, i, i, cs)                           # (gm) Read Dirty Region Map
        node(CCSM$EndProcess, CCSM$OwnerAcq, CCSM$LogAqrOwnrshp, ST_CpyOwned) # (gm) You Are Owner

        node(i, i, i, cs)                           # (gm) Ownership Changed
        node(i, i, i, cs)                           # (gm) Ownership Suspended
        node(i, i, i, cs)                           # (gm) Ownership Terminated
        node(i, i, i, cs)                           # (gm) Dirty Region Map

        node(i, i, i, cs)                           # Have Resources
        node(CCSM$EndProcess, CCSM$ClrRM_flags, i, SI_NoResources) # Not Have
                                                    #                Resources
        node(CCSM$CopyTerminated, i, i, cs)         # (cc) Copy Terminated
        node(CCSM$Define_own, i, i, cs)             # Timeout

        node(i, i, i, cs)                           # Current Op. Retry Count Expired
        node(i, i, i, cs)                           # Current Op. Rty. Cnt. Not Exp.
        node(i, i, i, cs)                           # Process Retry Count Expired
        node(i, i, i, cs)                           # Process Rty. Cnt. Not Exp.

        node(i, i, i, cs)                           # Done
        node(i, i, i, cs)                           # Check if owner

#**********************************************************************
#
#   --- Suspending Current Owner
#
#       This controller has requested that the current owner of the
#       copy suspend ownership.
#
#**********************************************************************

    column(SE_SuspendCurOwner)
        node(CCSM$Test4Resources, i, i, cs)         # Copy Configuration Changed
        node(i, i, i, cs)                           # Start Copy
        node(CM$BreakCopy, i, i, SZ_TermCopy)       # Terminate Copy
        node(i, i, i, cs)                           # (cc) Copy Ready

        node(i, i, i, cs)                           # (cc) Ownership Suspended
        node(i, i, i, cs)                           # (gm) Read Dirty Segment Map
        node(CCSM$DeallocSM, i, i, cs)              # (gm) Dirty Segment Map
        node(i, i, i, cs)                           # (gm) Suspend Ownership

        node(i, i, i, cs)                           # (gm) Transfer Ownership
        node(i, i, i, cs)                           # (gm) Terminate Ownership
        node(i, i, i, cs)                           # (gm) Read Dirty Region Map
        node(i, i, i, cs)                           # (gm) You Are Owner

        node(i, i, i, cs)                           # (gm) Ownership Changed
        node(CCSM$CurOwnSusp, i, i, SF_ReqRegionMap) # (gm) Ownership Suspended
        node(i, i, i, cs)                           # (gm) Ownership Terminated
        node(i, i, i, cs)                           # (gm) Dirty Region Map

        node(i, i, i, cs)                           # Have Resources
        node(CCSM$EndProcess, CCSM$ClrRM_flags, i, SI_NoResources) # Not Have
                                                    #                Resources
        node(CCSM$CopyTerminated, i, i, cs)         # (cc) Copy Terminated
        node(CCSM$ProcTimeout, i, i, cs)            # Timeout

        node(i, i, i, cs)                           # Current Op. Retry Count Expired
        node(i, i, i, cs)                           # Current Op. Rty. Cnt. Not Exp.
        node(CCSM$ForceOwn, CCSM$LogOwnrshpForce, i, SD_DefnOwnership) # Process Retry Count Expired
        node(CCSM$Define_own, i, i, SD_DefnOwnership) # Process Rty. Cnt. Not Exp.

        node(i, i, i, cs)                           # Done
        node(i, i, i, cs)                           # Check if owner

#**********************************************************************
#
#   --- Requesting Dirty Region Map
#
#       This controller has requested a region map from the current
#       owner of the copy.
#
#**********************************************************************

    column(SF_ReqRegionMap)
        node(CCSM$Test4Resources, i, i, cs)         # Copy Configuration Changed
        node(i, i, i, cs)                           # Start Copy
        node(CM$BreakCopy, i, i, SZ_TermCopy)       # Terminate Copy
        node(i, i, i, cs)                           # (cc) Copy Ready

        node(i, i, i, cs)                           # (cc) Ownership Suspended
        node(i, i, i, cs)                           # (gm) Read Dirty Segment Map
        node(CCSM$DeallocSM, i, i, cs)              # (gm) Dirty Segment Map
        node(i, i, i, cs)                           # (gm) Suspend Ownership

        node(i, i, i, cs)                           # (gm) Transfer Ownership
        node(i, i, i, cs)                           # (gm) Terminate Ownership
        node(i, i, i, cs)                           # (gm) Read Dirty Region Map
        node(i, i, i, cs)                           # (gm) You Are Owner

        node(i, i, i, cs)                           # (gm) Ownership Changed
        node(i, i, i, cs)                           # (gm) Ownership Suspended
        node(i, i, i, cs)                           # (gm) Ownership Terminated
        node(CCSM$DirtyRM, i, i, SF2_ReqSegmentMap) # (gm) Dirty Region Map

        node(i, i, i, cs)                           # Have Resources
        node(CCSM$EndProcess, CCSM$ClrRM_flags, i, SI_NoResources) # Not Have
                                                    #                Resources
        node(CCSM$CopyTerminated, i, i, cs)         # (cc) Copy Terminated
        node(CCSM$OpTimeout, i, i, cs)              # Timeout

        node(CCSM$ProcTimeout, i, i, cs)            # Current Op. Retry Count Expired
        node(CCSM$RtyReadRM, i, i, cs)              # Current Op. Rty. Cnt. Not Exp.
        node(CCSM$ForceOwn, i, i, SD_DefnOwnership) # Process Retry Count Expired
        node(CCSM$Define_own, i, i, SD_DefnOwnership) # Process Rty. Cnt. Not Exp.

        node(CCSM$RtyTermOwn, i, i, SG_TermCurOwner) # Done
        node(i, i, i, cs)                           # Check if owner

#**********************************************************************
#
#   --- Requesting Dirty Segment Maps
#
#       This controller is requesting dirty segment maps from the current
#       owner of the copy.
#
#**********************************************************************

    column(SF2_ReqSegmentMap)
        node(CCSM$Test4Resources, i, i, cs)         # Copy Configuration Changed
        node(i, i, i, cs)                           # Start Copy
        node(CM$BreakCopy, i, i, SZ_TermCopy)       # Terminate Copy
        node(i, i, i, cs)                           # (cc) Copy Ready

        node(i, i, i, cs)                           # (cc) Ownership Suspended
        node(i, i, i, cs)                           # (gm) Read Dirty Segment Map
        node(CCSM$DirtySM, i, i, cs)                # (gm) Dirty Segment Map
        node(i, i, i, cs)                           # (gm) Suspend Ownership

        node(i, i, i, cs)                           # (gm) Transfer Ownership
        node(i, i, i, cs)                           # (gm) Terminate Ownership
        node(i, i, i, cs)                           # (gm) Read Dirty Region Map
        node(i, i, i, cs)                           # (gm) You Are Owner

        node(i, i, i, cs)                           # (gm) Ownership Changed
        node(i, i, i, cs)                           # (gm) Ownership Suspended
        node(i, i, i, cs)                           # (gm) Ownership Terminated
        node(i, i, i, cs)                           # (gm) Dirty Region Map

        node(i, i, i, cs)                           # Have Resources
        node(CCSM$EndProcess, CCSM$ClrRM_flags, i, SI_NoResources) # Not Have
                                                    #                Resources
        node(CCSM$CopyTerminated, i, i, cs)         # (cc) Copy Terminated
        node(CCSM$OpTimeout, i, i, cs)              # Timeout

        node(CCSM$GetNextSM, i, i, cs)              # Current Op. Retry Count Expired
        node(CCSM$RtyReadSM, i, i, cs)              # Current Op. Rty. Cnt. Not Exp.
        node(i, i, i, cs)                           # Process Retry Count Expired
        node(i, i, i, cs)                           # Process Rty. Cnt. Not Exp.

        node(CCSM$RtyTermOwn, i, i, SG_TermCurOwner) # Done
        node(i, i, i, cs)                           # Check if owner

#**********************************************************************
#
#   --- Terminating Current Owner
#
#   This controller has request that the current owner of the copy
#   terminate ownership.
#
#**********************************************************************

    column(SG_TermCurOwner)
        node(i, i, i, cs)                           # Copy Configuration Changed
        node(i, i, i, cs)                           # Start Copy
        node(CM$BreakCopy, i, i, SZ_TermCopy)       # Terminate Copy
        node(i, i, i, cs)                           # (cc) Copy Ready

        node(i, i, i, cs)                           # (cc) Ownership Suspended
        node(i, i, i, cs)                           # (gm) Read Dirty Segment Map
        node(CCSM$DeallocSM, i, i, cs)              # (gm) Dirty Segment Map
        node(i, i, i, cs)                           # (gm) Suspend Ownership

        node(i, i, i, cs)                           # (gm) Transfer Ownership
        node(i, i, i, cs)                           # (gm) Terminate Ownership
        node(i, i, i, cs)                           # (gm) Read Dirty Region Map
        node(i, i, i, cs)                           # (gm) You Are Owner

        node(i, i, i, cs)                           # (gm) Ownership Changed
        node(i, i, i, cs)                           # (gm) Ownership Suspended
        node(CCSM$OwnTerm, i, i, SH_RetestOwnership) # (gm) Ownership Terminated
        node(i, i, i, cs)                           # (gm) Dirty Region Map

        node(i, i, i, cs)                           # Have Resources
        node(i, i, i, cs)                           # Not Have Resources
        node(CCSM$CopyTerminated, i, i, cs)         # (cc) Copy Terminated
        node(CCSM$OpTimeout, i, i, cs)              # Timeout

        node(CCSM$ProcTimeout, i, i, cs)            # Current Op. Retry Count Expired
        node(CCSM$RtyTermOwn, i, i, cs)             # Current Op. Rty. Cnt. Not Exp.
        node(CCSM$ForceOwn, i, i, SD_DefnOwnership) # Process Retry Count Expired
        node(CCSM$Define_own, i, i, SD_DefnOwnership) # Process Rty. Cnt. Not Exp.

        node(i, i, i, cs)                           # Done
        node(i, i, i, cs)                           # Check if owner

#**********************************************************************
#
#   --- Retesting for Copy Ownership
#
#       This controller has terminated the previous copy ownership
#       and is retesting that it is registered as the current copy owner.
#
#**********************************************************************

    column(SH_RetestOwnership)
        node(i, i, i, cs)                           # Copy Configuration Changed
        node(i, i, i, cs)                           # Start Copy
        node(CM$BreakCopy, i, i, SZ_TermCopy)       # Terminate Copy
        node(i, i, i, cs)                           # (cc) Copy Ready

        node(i, i, i, cs)                           # (cc) Ownership Suspended
        node(i, i, i, cs)                           # (gm) Read Dirty Segment Map
        node(CCSM$DeallocSM, i, i, cs)              # (gm) Dirty Segment Map
        node(i, i, i, cs)                           # (gm) Suspend Ownership

        node(CCSM$ResetTransRM, CCSM$SuspCurOwn, i, SE_SuspendCurOwner) #
                                                    # (gm) Transfer Ownership
        node(i, i, i, cs)                           # (gm) Terminate Ownership
        node(i, i, i, cs)                           # (gm) Read Dirty Region Map
        node(CCSM$EndProcess, CCSM$OwnerAcq, CCSM$LogAqrOwnrshp, ST_CpyOwned)
                                                    # (gm) You Are Owner

        node(i, i, i, cs)                           # (gm) Ownership Changed
        node(i, i, i, cs)                           # (gm) Ownership Suspended
        node(i, i, i, cs)                           # (gm) Ownership Terminated
        node(i, i, i, cs)                           # (gm) Dirty Region Map

        node(i, i, i, cs)                           # Have Resources
        node(i, i, i, cs)                           # Not Have Resources
        node(CCSM$CopyTerminated, i, i, cs)         # (cc) Copy Terminated
        node(CCSM$OpTimeout, i, i, cs)              # Timeout

        node(CCSM$ProcTimeout, i, i, cs)            # Current Op. Retry Count Expired
        node(CCSM$Define_own, i, i, cs)             # Current Op. Rty. Cnt. Not Exp.
        node(CCSM$ForceOwn, i, i, SD_DefnOwnership) # Process Retry Count Expired
        node(CCSM$Define_own, i, i, SD_DefnOwnership) # Process Rty. Cnt. Not Exp.

        node(i, i, i, cs)                           # Done
        node(i, i, i, cs)                           # Check if owner

#**********************************************************************
#
#   --- Copy Does Not Have Resources
#
#       This controller does not have the copy resources.
#
#**********************************************************************

    column(SI_NoResources)
        node(CCSM$Test4Resources, i, i, cs)         # Copy Configuration Changed
        node(i, i, i, cs)                           # Start Copy
        node(CM$BreakCopy, i, i, SZ_TermCopy)       # Terminate Copy
        node(i, i, i, cs)                           # (cc) Copy Ready

        node(i, i, i, cs)                           # (cc) Ownership Suspended
        node(i, i, i, cs)                           # (gm) Read Dirty Segment Map
        node(CCSM$DeallocSM, i, i, cs)              # (gm) Dirty Segment Map
        node(i, i, i, cs)                           # (gm) Suspend Ownership

        node(i, i, i, cs)                           # (gm) Transfer Ownership
        node(i, i, i, cs)                           # (gm) Terminate Ownership
        node(i, i, i, cs)                           # (gm) Read Dirty Region Map
        node(i, i, i, cs)                           # (gm) You Are Owner

        node(i, i, i, cs)                           # (gm) Ownership Changed
        node(i, i, i, cs)                           # (gm) Ownership Suspended
        node(i, i, i, cs)                           # (gm) Ownership Terminated
        node(i, i, i, cs)                           # (gm) Dirty Region Map

        node(CCSM$ResetOCSE, CCSM$Define_own, i, SD_DefnOwnership) # Have Resources
        node(i, i, i, cs)                           # Not Have Resources
        node(CCSM$CopyTerminated, i, i, cs)         # (cc) Copy Terminated
        node(i, i, i, cs)                           # Timeout

        node(i, i, i, cs)                           # Current Op. Retry Count Expired
        node(i, i, i, cs)                           # Current Op. Rty. Cnt. Not Exp.
        node(i, i, i, cs)                           # Process Retry Count Expired
        node(i, i, i, cs)                           # Process Rty. Cnt. Not Exp.

        node(i, i, i, cs)                           # Done
        node(i, i, i, cs)                           # Check if owner

#**********************************************************************
#
#   --- Changing Ownership
#
#**********************************************************************

    column(SP_ChgOwnership)
        node(i, i, i, cs)                           # Copy Configuration Changed
        node(i, i, i, cs)                           # Start Copy
        node(CM$BreakCopy, i, i, SZ_TermCopy)       # Terminate Copy
        node(i, i, i, cs)                           # (cc) Copy Ready

        node(i, i, i, cs)                           # (cc) Ownership Suspended
        node(CCSM$RDSMap, i, i, cs)                 # (gm) Read Dirty Segment Map
        node(CCSM$DeallocSM, i, i, cs)              # (gm) Dirty Segment Map
        node(CCSM$SuspOwn3, i, i, cs)               # (gm) Suspend Ownership

        node(i, i, i, cs)                           # (gm) Transfer Ownership
        node(CCSM$TermOwn, i, i, cs)                # (gm) Terminate Ownership
        node(CCSM$RDRMap, i, i, cs)                 # (gm) Read Dirty Region Map
        node(i, i, i, cs)                           # (gm) You Are Owner

        node(CCSM$OwnChanged, CCSM$EndProcess, CCSM$LogOwnrshpTerm, SI_NoResources) #
                                                    # (gm) Ownership Changed
        node(i, i, i, cs)                           # (gm) Ownership Suspended
        node(i, i, i, cs)                           # (gm) Ownership Terminated
        node(i, i, i, cs)                           # (gm) Dirty Region Map

        node(CCSM$ResetOCSE, CCSM$Define_own, i, SD_DefnOwnership) # Have Resources
        node(CCSM$EndProcess, CCSM$ClrRM_flags, i, SI_NoResources) # Not Have
                                                    #                Resources
        node(CCSM$CopyTerminated, i, i, cs)         # (cc) Copy Terminated
        node(CCSM$Test4Resources, i, i, cs)         # Timeout

        node(i, i, i, cs)                           # Current Op. Retry Count Expired
        node(i, i, i, cs)                           # Current Op. Rty. Cnt. Not Exp.
        node(i, i, i, cs)                           # Process Retry Count Expired
        node(i, i, i, cs)                           # Process Rty. Cnt. Not Exp.

        node(i, i, i, cs)                           # Done
        node(i, i, i, cs)                           # Check if owner

#**********************************************************************
#
#   --- Ownership Suspended
#
#       The copy task has suspended ownership
#
#**********************************************************************

    column(SR_OwnerSuspended)
        node(i, i, i, cs)                           # Copy Configuration Changed
        node(i, i, i, cs)                           # Start Copy
        node(CM$BreakCopy, i, i, SZ_TermCopy)       # Terminate Copy
        node(i, i, i, cs)                           # (cc) Copy Ready

        node(i, i, i, cs)                           # (cc) Ownership Suspended
        node(CCSM$RDSMap, i, i, cs)                 # (gm) Read Dirty Segment Map
        node(CCSM$DeallocSM, i, i, cs)              # (gm) Dirty Segment Map
        node(CCSM$SuspOwn3, i, i, cs)               # (gm) Suspend Ownership

        node(i, i, i, cs)                           # (gm) Transfer Ownership
        node(CCSM$TermOwn, i, i, SP_ChgOwnership)   # (gm) Terminate Ownership
        node(CCSM$RDRMap, i, i, cs)                 # (gm) Read Dirty Region Map
        node(i, i, i, cs)                           # (gm) You Are Owner

        node(i, i, i, cs)                           # (gm) Ownership Changed
        node(i, i, i, cs)                           # (gm) Ownership Suspended
        node(i, i, i, cs)                           # (gm) Ownership Terminated
        node(i, i, i, cs)                           # (gm) Dirty Region Map

        node(CCSM$ResetOCSE, CCSM$Define_own, i, SD_DefnOwnership) # Have Resources
        node(CCSM$EndProcess, CCSM$Clrflags, i, SI_NoResources) # Not Have Resources
        node(CCSM$CopyTerminated, i, i, cs)         # (cc) Copy Terminated
        node(CCSM$Test4Resources, i, i, cs)         # Timeout

        node(i, i, i, cs)                           # Current Op. Retry Count Expired
        node(i, i, i, cs)                           # Current Op. Rty. Cnt. Not Exp.
        node(i, i, i, cs)                           # Process Retry Count Expired
        node(i, i, i, cs)                           # Process Rty. Cnt. Not Exp.

        node(i, i, i, cs)                           # Done
        node(i, i, i, cs)                           # Check if owner

#**********************************************************************
#
#   --- Suspending Ownership
#
#       Requesting the copy task suspends ownership of the copy
#
#**********************************************************************

    column(SS_SuspendingOwner)
        node(i, i, i, cs)                           # Copy Configuration Changed
        node(i, i, i, cs)                           # Start Copy
        node(CM$BreakCopy, i, i, SZ_TermCopy)       # Terminate Copy
        node(i, i, i, cs)                           # (cc) Copy Ready

        node(CCSM$OwnSusp, i, i, SR_OwnerSuspended) # (cc) Ownership Suspended
        node(i, i, i, cs)                           # (gm) Read Dirty Segment Map
        node(CCSM$DeallocSM, i, i, cs)              # (gm) Dirty Segment Map
        node(CCSM$SuspOwn2, i, i, cs)               # (gm) Suspend Ownership

        node(i, i, i, cs)                           # (gm) Transfer Ownership
        node(i, i, i, cs)                           # (gm) Terminate Ownership
        node(i, i, i, cs)                           # (gm) Read Dirty Region Map
        node(i, i, i, cs)                           # (gm) You Are Owner

        node(i, i, i, cs)                           # (gm) Ownership Changed
        node(i, i, i, cs)                           # (gm) Ownership Suspended
        node(i, i, i, cs)                           # (gm) Ownership Terminated
        node(i, i, i, cs)                           # (gm) Dirty Region Map

        node(i, i, i, cs)                           # Have Resources
        node(i, i, i, cs)                           # Not Have Resources
        node(CCSM$CopyTerminated, i, i, cs)         # (cc) Copy Terminated
        node(i, i, i, cs)                           # Timeout

        node(i, i, i, cs)                           # Current Op. Retry Count Expired
        node(i, i, i, cs)                           # Current Op. Rty. Cnt. Not Exp.
        node(i, i, i, cs)                           # Process Retry Count Expired
        node(i, i, i, cs)                           # Process Rty. Cnt. Not Exp.

        node(i, i, i, cs)                           # Done
        node(i, i, i, cs)                           # Check if owner

#**********************************************************************
#
#   --- Copy is Owned
#
#       The copy is owned by this controller
#
#**********************************************************************

    column(ST_CpyOwned)
        node(i, i, i, cs)                           # Copy Configuration Changed
        node(i, i, i, cs)                           # Start Copy
        node(CM$BreakCopy, i, i, SZ_TermCopy)       # Terminate Copy
        node(i, i, i, cs)                           # (cc) Copy Ready

        node(i, i, i, cs)                           # (cc) Ownership Suspended
        node(i, i, i, cs)                           # (gm) Read Dirty Segment Map
        node(CCSM$DeallocSM, i, i, cs)              # (gm) Dirty Segment Map
        node(CCSM$Test4Resources, i, i, cs)         # (gm) Suspend Ownership

        node(i, i, i, cs)                           # (gm) Transfer Ownership
        node(i, i, i, cs)                           # (gm) Terminate Ownership
        node(i, i, i, cs)                           # (gm) Read Dirty Region Map
        node(i, i, i, cs)                           # (gm) You Are Owner

        node(i, i, i, cs)                           # (gm) Ownership Changed
        node(i, i, i, cs)                           # (gm) Ownership Suspended
        node(i, i, i, cs)                           # (gm) Ownership Terminated
        node(i, i, i, cs)                           # (gm) Dirty Region Map

        node(i, i, i, cs)                           # Have Resources
        node(CCSM$SuspOwn, i, i, SS_SuspendingOwner) # Not Have Resources
        node(CCSM$CopyTerminated, i, i, cs)         # (cc) Copy Terminated
        node(i, i, i, cs)                           # Timeout

        node(i, i, i, cs)                           # Current Op. Retry Count Expired
        node(i, i, i, cs)                           # Current Op. Rty. Cnt. Not Exp.
        node(i, i, i, cs)                           # Process Retry Count Expired
        node(i, i, i, cs)                           # Process Rty. Cnt. Not Exp.

        node(i, i, i, cs)                           # Done
        node(CCSM$IAmOwner, i, i, cs)               # Check if owner

#**********************************************************************
#
#   --- Copy is Being Terminated
#
#
#**********************************************************************

    column(SZ_TermCopy)
        node(i, i, i, cs)                           # Copy Configuration Changed
        node(i, i, i, cs)                           # Start Copy
        node(i, i, i, cs)                           # Terminate Copy
        node(i, i, i, cs)                           # (cc) Copy Ready

        node(i, i, i, cs)                           # (cc) Ownership Suspended
        node(i, i, i, cs)                           # (gm) Read Dirty Segment Map
        node(CCSM$DeallocSM, i, i, cs)              # (gm) Dirty Segment Map
        node(i, i, i, cs)                           # (gm) Suspend Ownership

        node(i, i, i, cs)                           # (gm) Transfer Ownership
        node(i, i, i, cs)                           # (gm) Terminate Ownership
        node(i, i, i, cs)                           # (gm) Read Dirty Region Map
        node(i, i, i, cs)                           # (gm) You Are Owner

        node(i, i, i, cs)                           # (gm) Ownership Changed
        node(i, i, i, cs)                           # (gm) Ownership Suspended
        node(i, i, i, cs)                           # (gm) Ownership Terminated
        node(i, i, i, cs)                           # (gm) Dirty Region Map

        node(i, i, i, cs)                           # Have Resources
        node(i, i, i, cs)                           # Not Have Resources
        node(CCSM$CopyTerminated, i, i, cs)         # (cc) Copy Terminated
        node(i, i, i, cs)                           # Timeout

        node(i, i, i, cs)                           # Current Op. Retry Count Expired
        node(i, i, i, cs)                           # Current Op. Rty. Cnt. Not Exp.
        node(i, i, i, cs)                           # Process Retry Count Expired
        node(i, i, i, cs)                           # Process Rty. Cnt. Not Exp.

        node(i, i, i, cs)                           # Done
        node(i, i, i, cs)                           # Check if owner

#**********************************************************************
#
#   --- set next table to equates only
#
#**********************************************************************

    .set    EQU_ONLY,TRUE
