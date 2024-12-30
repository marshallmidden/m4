# $Id: CCstate.as 88786 2009-06-11 16:39:39Z m4 $
#**********************************************************************
#
#  NAME:
#
#  PURPOSE:
#
#  FUNCTIONS:
#
#  Copyright (c) 2004-2008 XIOtech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- assembly options ------------------------------------------------
#
        .data

        tablename(CCtbl)


#**********************************************************************
#
#   State definitions
#
#**********************************************************************

    state(Idle)
    state(StartingTask)
    state(CPNoOwner)
    state(CPTstPause)

    state(CopyUPaused)
    state(CopyAPaused)
    state(CopyActive)
    state(SuspendingOwner)

    state(PausePending)
    state(SOwCP)
    state(BreakPending)
    state(RemoteOwner)

    state(RemoteNotOwner)
    state(ErrorOnGeoraidMirror)
#**********************************************************************
#
#   Event Definitions
#
#**********************************************************************

    event(StartTask)
    event(PauseCopy)
    event(ResumeCopy)
    event(SwapRaids)

    event(BreakCopy)
    event(TaskStarted)
    event(OwnerAcquired)
    event(CopyRegistered)

    event(CopyResumed)
    event(SuspendOwner)
    event(OwnerSuspended)
    event(CopyEnded)

    event(RegionDirty)
    event(RegionSync)
    event(CpyAutoPaused)
    event(CpyPaused)

    event(RemoteCopy)
    event(SourceError)
    event(InstantMirror)
#**********************************************************************
#
#   Action Definitions
#
#**********************************************************************

    action(i)
    action(CM$settaskrdy)
    action(cm$usr_Swap)
    action(cm$usr_Break)
    action(cm$usr_Pause)
    action(cm$usr_Resume)
    action(cm$SuspendOwnership)
    action(cm$GOC_TaskReady)
    action(cm$GOC_OwnerSuspend)
    action(cm$GOC_CopyTerminated)
    action(cm$SetnTestCpyState)
    action(cm$force_LP)
    action(cm$GOC_SuspendOwner)
    action(cm$InstantMirror)

#**********************************************************************
#
#    Copy not initialized
#
#**********************************************************************

    column(Idle)
        node(CM$settaskrdy, i, i, StartingTask)     # Start Task
        node(i, i, i, cs)                           # Pause Copy
        node(i, i, i, cs)                           # Resume Copy
        node(i, i, i, cs)                           # Swap Raids

        node(i, i, i, cs)                           # Break Copy
        node(i, i, i, cs)                           # Task Started
        node(i, i, i, cs)                           # Ownership Acquired
        node(i, i, i, cs)                           # Copy Registered

        node(i, i, i, cs)                           # Copy Resumed
        node(i, i, i, cs)                           # Suspend Ownership
        node(i, i, i, cs)                           # Ownership Suspended
        node(i, i, i, cs)                           # Copy Ended

        node(i, i, i, cs)                           # Region Dirty
        node(i, i, i, cs)                           # Region Sync
        node(i, i, i, cs)                           # Copy Enter Auto Paused
        node(i, i, i, cs)                           # Copy User Paused

        node(cm$GOC_TaskReady, i, i, RemoteNotOwner) # Remote Copy
        node(i, i, i, cs)                           # Source Error
        node(i, i, i, cs)                           # instant mirror

#**********************************************************************
#
#    Wainting for task to start
#
#**********************************************************************

    column(StartingTask)
        node(i, i, i, cs)                           # Start Task
        node(i, i, i, cs)                           # Pause Copy
        node(i, i, i, cs)                           # Resume Copy
        node(i, i, i, cs)                           # Swap Raids

        node(i, i, i, cs)                           # Break Copy
        node(cm$GOC_TaskReady, i, i, CPNoOwner)     # Task Started
        node(i, i, i, cs)                           # Ownership Acquired
        node(i, i, i, cs)                           # Copy Registered

        node(i, i, i, cs)                           # Copy Resumed
        node(i, i, i, cs)                           # Suspend Ownership
        node(i, i, i, cs)                           # Ownership Suspended
        node(i, i, i, cs)                           # Copy Ended

        node(i, i, i, cs)                           # Region Dirty
        node(i, i, i, cs)                           # Region Sync
        node(i, i, i, cs)                           # Copy Enter Auto Paused
        node(i, i, i, cs)                           # Copy User Paused

        node(i, i, i, cs)                           # Remote Copy
        node(i, i, i, cs)                           # Source Error
        node(i, i, i, cs)                           # instant mirror

#**********************************************************************
#
#    Copy Paused Without Ownership
#
#**********************************************************************

    column(CPNoOwner)
        node(i, i, i, cs)                           # Start Task
        node(cm$usr_Pause, i, i, cs)                # Pause Copy
        node(i, i, i, cs)                           # Resume Copy
        node(cm$usr_Swap, i, i, cs)                 # Swap Raids

        node(cm$usr_Break, i, i, BreakPending)      # Break Copy
        node(i, i, i, cs)                           # Task Started
        node(cm$SetnTestCpyState,i , i, CPTstPause) # Ownership Acquired
        node(i, i, i, cs)                           # Copy Registered

        node(i, i, i, cs)                           # Copy Resumed
        node(cm$GOC_SuspendOwner, cm$GOC_OwnerSuspend, i, cs) # Suspend Ownership
        node(i, i, i, cs)                           # Ownership Suspended
        node(i, i, i, cs)                           # Copy Ended

        node(i, i, i, cs)                           # Region Dirty
        node(i, i, i, cs)                           # Region Sync
        node(i, i, i, cs)                           # Copy Enter Auto Paused
        node(i, i, i, cs)                           # Copy User Paused

        node(i, i, i, cs)                           # Remote Copy
        node(i, i, i, cs)                           # Source Error
#       node(i, i, i, ErrorOnGeoraidMirror)         # Source Error
        node(cm$InstantMirror, i, i, CopyActive)    # instant mirror

#**********************************************************************
#
#    Testing cm Suspension state
#
#**********************************************************************

    column(CPTstPause)
        node(i, i, i, cs)                           # Start Task
        node(i, i, i, cs)                           # Pause Copy
        node(i, i, i, cs)                           # Resume Copy
        node(i, i, i, cs)                           # Swap Raids

        node(i, i, i, cs)                           # Break Copy
        node(i, i, i, cs)                           # Task Started
        node(i, i, i, cs)                           # Ownership Acquired
        node(i, i, i, cs)                           # Copy Registered

        node(i, i, i, cs)                           # Copy Resumed
        node(i, i, i, cs)                           # Suspend Ownership
        node(i, i, i, cs)                           # Ownership Suspended
        node(i, i, i, cs)                           # Copy Ended

        node(i, i, i, cs)                           # Region Dirty
        node(i, i, i, cs)                           # Region Sync
        node(cm$force_LP, i, i, CopyAPaused)        # Copy Enter Auto Paused
        node(i, i, i, CopyUPaused)                  # Copy User Paused

        node(i, i, i, cs)                           # Remote Copy
        node(i, i, i, cs)                           # Source Error
        node(i, i, i, cs)                           # instant mirror

#**********************************************************************
#
#    Copy User Paused
#
#**********************************************************************

    column(CopyUPaused)
        node(i, i, i, cs)                           # Start Task
        node(i, i, i, cs)                           # Pause Copy
        node(cm$usr_Resume, i, i, CopyAPaused)      # Resume Copy
        node(cm$usr_Swap, i, i, cs)                 # Swap Raids

        node(cm$usr_Break, i, i,  BreakPending)     # Break Copy
        node(i, i, i, cs)                           # Task Started
        node(i, i, i, cs)                           # Ownership Acquired
        node(i, i, i, cs)                           # Copy Registered

        node(i, i, i, cs)                           # Copy Resumed
        node(cm$SuspendOwnership, i, i, SuspendingOwner) # Suspend Ownership
        node(i, i, i, cs)                           # Ownership Suspended
        node(i, i, i, cs)                           # Copy Ended

        node(i, i, i, cs)                           # Region Dirty
        node(i, i, i, cs)                           # Region Sync
        node(i, i, i, cs)                           # Copy Enter Auto Paused
        node(i, i, i, cs)                           # Copy User Paused

        node(i, i, i, cs)                           # Remote Copy
        node(i, i, i, cs)                           # Source Error
        node(i, i, i, cs)                           # instant mirror

#**********************************************************************
#
#    Copy Auto Paused
#
#**********************************************************************

    column(CopyAPaused)
        node(i, i, i, cs)                           # Start Task
        node(cm$usr_Pause, i, i, PausePending)      # Pause Copy
        node(i, i, i, cs)                           # Resume Copy
        node(cm$usr_Swap, cm$usr_Pause, i, cs)      # Swap Raids

        node(cm$usr_Break, i, i,  BreakPending)     # Break Copy
        node(i, i, i, cs)                           # Task Started
        node(i, i, i, cs)                           # Ownership Acquired
        node(i, i, i, cs)                           # Copy Registered

        node(i, i, i, CopyActive)                   # Copy Resumed
        node(cm$SuspendOwnership, i, i, SuspendingOwner) # Suspend Ownership
        node(i, i, i, cs)                           # Ownership Suspended
        node(i, i, i, cs)                           # Copy Ended

        node(i, i, i, cs)                           # Region Dirty
        node(i, i, i, cs)                           # Region Sync
        node(i, i, i, cs)                           # Copy Enter Auto Paused
        node(i, i, i, CopyUPaused)                  # Copy User Paused

        node(i, i, i, cs)                           # Remote Copy
        node(i, i, i, ErrorOnGeoraidMirror)         # Source Error
        node(i, i, i, cs)                           # instant mirror

#**********************************************************************
#
#    Error on Georaid Mirror
#
#**********************************************************************

    column(ErrorOnGeoraidMirror)
        node(i, i, i, cs)                           # Start Task
        node(cm$usr_Pause, i, i, PausePending)      # Pause Copy
        node(i, i, i, cs)                           # Resume Copy
        node(cm$usr_Swap, i, i, CopyAPaused)      # Swap Raids

        node(cm$usr_Break, i, i,  BreakPending)     # Break Copy
        node(i, i, i, cs)                           # Task Started
        node(i, i, i, cs)                           # Ownership Acquired
        node(i, i, i, cs)                           # Copy Registered

        node(i, i, i, CopyActive)                   # Copy Resumed
        node(cm$SuspendOwnership, i, i, SuspendingOwner) # Suspend Ownership
        node(i, i, i, cs)                           # Ownership Suspended
        node(i, i, i, cs)                           # Copy Ended

        node(i, i, i, cs)                           # Region Dirty
        node(i, i, i, cs)                           # Region Sync
        node(i, i, i, CopyAPaused)                  # Copy Enter Auto Paused
        node(i, i, i, CopyUPaused)                  # Copy User Paused

        node(i, i, i, cs)                           # Remote Copy
        node(i, i, i, cs)                           # Source Error
        node(i, i, i, cs)                           # instant mirror

#**********************************************************************
#
#    Copy Active
#
#**********************************************************************

    column(CopyActive)
        node(i, i, i, cs)                           # Start Task
        node(cm$usr_Pause, i, i, PausePending)      # Pause Copy
        node(i, i, i, cs)                           # Resume Copy
        node(cm$usr_Swap, i, i, cs)                 # Swap Raids

        node(cm$usr_Break, i, i, BreakPending)      # Break Copy
        node(i, i, i, cs)                           # Task Started
        node(i, i, i, cs)                           # Ownership Acquired
        node(i, i, i, cs)                           # Copy Registered

        node(i, i, i, cs)                           # Copy Resumed
        node(cm$SuspendOwnership, i, i, SuspendingOwner) # Suspend Ownership
        node(i, i, i, cs)                           # Ownership Suspended
        node(i, i, i, cs)                           # Copy Ended

        node(i, i, i, cs)                           # Region Dirty
        node(i, i, i, cs)                           # Region Sync
        node(i, i, i, CopyAPaused)                  # Copy Enter Auto Paused
        node(i, i, i, CopyUPaused)                  # Copy User Paused

        node(i, i, i, cs)                           # Remote Copy
        node(i, i, i, cs)                           # Source Error
#        node(i, i, i, ErrorOnGeoraidMirror)         # Source Error
        node(i, i, i, cs)                           # instant mirror

#**********************************************************************
#
#    Suspending Ownership
#
#**********************************************************************

    column(SuspendingOwner)
        node(i, i, i, cs)                           # Start Task
        node(i, i, i, cs)                           # Pause Copy
        node(i, i, i, cs)                           # Resume Copy
        node(i, i, i, cs)                           # Swap Raids

        node(i, i, i, cs)                           # Break Copy
        node(i, i, i, cs)                           # Task Started
        node(i, i, i, cs)                           # Ownership Acquired
        node(i, i, i, cs)                           # Copy Registered

        node(i, i, i, cs)                           # Copy Resumed
        node(i, i, i, cs)                           # Suspend Ownership
        node(cm$GOC_OwnerSuspend, i, i, CPNoOwner)  # Ownership Suspended
        node(i, i, i, cs)                           # Copy Ended

        node(i, i, i, cs)                           # Region Dirty
        node(i, i, i, cs)                           # Region Sync
        node(i, i, i, cs)                           # Copy Enter Auto Paused
        node(i, i, i, cs)                           # Copy User Paused

        node(i, i, i, cs)                           # Remote Copy
        node(i, i, i, cs)                           # Source Error
        node(i, i, i, cs)                           # instant mirror

#**********************************************************************
#
#    Copy Pause Pending
#
#**********************************************************************

    column(PausePending)
        node(i, i, i, cs)                           # Start Task
        node(i, i, i, cs)                           # Pause Copy
        node(cm$usr_Resume, i, i, CopyActive)       # Resume Copy
        node(i, i, i, cs)                           # Swap Raids

        node(i, i, i, BreakPending)                 # Break Copy
        node(i, i, i, cs)                           # Task Started
        node(i, i, i, cs)                           # Ownership Acquired
        node(i, i, i, cs)                           # Copy Registered

        node(i, i, i, cs)                           # Copy Resumed
        node(i, i, i, SOwCP)                        # Suspend Ownership
        node(i, i, i, cs)                           # Ownership Suspended
        node(i, i, i, cs)                           # Copy Ended

        node(i, i, i, cs)                           # Region Dirty
        node(i, i, i, cs)                           # Region Sync
        node(i, i, i, CopyAPaused)                  # Copy Enter Auto Paused
        node(i, i, i, CopyUPaused)                  # Copy User Paused

        node(i, i, i, cs)                           # Remote Copy
        node(i, i, i, cs)                           # Source Error
#        node(i, i, i, ErrorOnGeoraidMirror)         # Source Error
        node(i, i, i, cs)                           # instant mirror

#**********************************************************************
#
#    Suspend ownership while pause pending
#
#**********************************************************************

    column(SOwCP)
        node(i, i, i, cs)                           # Start Task
        node(i, i, i, cs)                           # Pause Copy
        node(i, i, i, cs)                           # Resume Copy
        node(i, i, i, cs)                           # Swap Raids

        node(cm$usr_Break, i, i, cs)                # Break Copy
        node(i, i, i, cs)                           # Task Started
        node(i, i, i, cs)                           # Ownership Acquired
        node(i, i, i, cs)                           # Copy Registered

        node(i, i, i, cs)                           # Copy Resumed
        node(i, i, i, cs)                           # Suspend Ownership
        node(i, i, i, cs)                           # Ownership Suspended
        node(i, i, i, cs)                           # Copy Ended

        node(i, i, i, cs)                           # Region Dirty
        node(i, i, i, cs)                           # Region Sync
        node(cm$SuspendOwnership, i, i, SuspendingOwner) # Copy Enter Auto Paused
        node(cm$SuspendOwnership, i, i, SuspendingOwner) # Copy User Paused

        node(i, i, i, cs)                           # Remote Copy
        node(i, i, i, cs)                           # Source Error
        node(i, i, i, cs)                           # instant mirror

#**********************************************************************
#
#    Copy Break Pending
#
#**********************************************************************

    column(BreakPending)
        node(i, i, i, cs)                           # Start Task
        node(i, i, i, cs)                           # Pause Copy
        node(i, i, i, cs)                           # Resume Copy
        node(i, i, i, cs)                           # Swap Raids

        node(i, i, i, cs)                           # Break Copy
        node(i, i, i, cs)                           # Task Started
        node(i, i, i, cs)                           # Ownership Acquired
        node(i, i, i, cs)                           # Copy Registered

        node(i, i, i, cs)                           # Copy Resumed
        node(i, i, i, cs)                           # Suspend Ownership
        node(i, i, i, cs)                           # Ownership Suspended
        node(cm$GOC_CopyTerminated, i, i, cs)       # Copy Ended

        node(i, i, i, cs)                           # Region Dirty
        node(i, i, i, cs)                           # Region Sync
        node(cm$usr_Break, i, i, cs)                # Copy Enter Auto Paused
        node(cm$usr_Break, i, i, cs)                # Copy User Paused

        node(i, i, i, cs)                           # Remote Copy
        node(i, i, i, cs)                           # Source Error
        node(i, i, i, cs)                           # instant mirror

#**********************************************************************
#
#    Remote Copy Owner
#
#**********************************************************************

    column(RemoteOwner)
        node(i, i, i, cs)                           # Start Task
        node(i, i, i, cs)                           # Pause Copy
        node(i, i, i, cs)                           # Resume Copy
        node(i, i, i, cs)                           # Swap Raids

        node(cm$GOC_CopyTerminated, i, i, cs)       # Break Copy
        node(i, i, i, cs)                           # Task Started
        node(i, i, i, cs)                           # Ownership Acquired
        node(i, i, i, cs)                           # Copy Registered

        node(i, i, i, cs)                           # Copy Resumed
        node(cm$GOC_OwnerSuspend, i, i, RemoteNotOwner) # Suspend Ownership
        node(i, i, i, cs)                           # Ownership Suspended
        node(cm$GOC_CopyTerminated, i, i, cs)       # Copy Ended

        node(i, i, i, cs)                           # Region Dirty
        node(i, i, i, cs)                           # Region Sync
        node(i, i, i, cs)                           # Copy Enter Auto Paused
        node(i, i, i, cs)                           # Copy User Paused

        node(i, i, i, cs)                           # Remote Copy
        node(i, i, i, cs)                           # Source Error
        node(i, i, i, cs)                           # instant mirror

#**********************************************************************
#
#    Remote Copy Not Owner
#
#**********************************************************************

    column(RemoteNotOwner)
        node(i, i, i, cs)                           # Start Task
        node(i, i, i, cs)                           # Pause Copy
        node(i, i, i, cs)                           # Resume Copy
        node(i, i, i, cs)                           # Swap Raids

        node(cm$GOC_CopyTerminated, i, i, cs)       # Break Copy
        node(i, i, i, cs)                           # Task Started
        node(i, i, i, RemoteOwner)                  # Ownership Acquired
        node(i, i, i, cs)                           # Copy Registered

        node(i, i, i, cs)                           # Copy Resumed
        node(i, i, i, cs)                           # Suspend Ownership
        node(i, i, i, cs)                           # Ownership Suspended
        node(cm$GOC_CopyTerminated, i, i, cs)       # Copy Ended

        node(i, i, i, cs)                           # Region Dirty
        node(i, i, i, cs)                           # Region Sync
        node(i, i, i, cs)                           # Copy Enter Auto Paused
        node(i, i, i, cs)                           # Copy User Paused

        node(i, i, i, cs)                           # Remote Copy
        node(i, i, i, cs)                           # Source Error
        node(i, i, i, cs)                           # instant mirror

#**********************************************************************
#
#   --- set next table to equates only
#
#**********************************************************************

    .set    EQU_ONLY,TRUE
