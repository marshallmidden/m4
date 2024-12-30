# $Id: raidinit.as 161128 2013-05-20 20:37:15Z marshall_midden $
#**********************************************************************
#
#  NAME: raidinit.as
#
#  PURPOSE:
#       To provide complete support for initializing RAID devices.
#
#  FUNCTIONS:
#       This module employs 1 process:
#
#       d$initialize_raid - Initialize RAID device (n copies)
#
#  RAID initialization writes zeros to every block of data in a RAID. This
#  can be called due to a user selecting to erase the data. For RAID-5 this
#  is required when a RAID is created and before it can be used so the
#  parity blocks are correct.
#
#  The initraid function is the MRP interface to erase a disk. Initializations
#  can also be queued directly to handle background processing for RAID-5
#  creations.
#
#  Since initializations take a lot of processing power, the number of
#  concurrent initializations are limited. New requests are put on a queue
#  and kicked off as previous initializations complete.
#
#  There are also external interfaces to pause, resume, and cancel
#  initializations that are in-progress.
#
#  Copyright (c) 1997-2010  Xiotech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- local equates ---------------------------------------------------
#
        .set    INIT_LOAD_FACTOR,30     # Initialize delay per queued
                                        #  system I/O (ms)
        .set    INIT_LOAD_MAX,125       # Max initialize delay (ms)
#
# --- global function declarations ------------------------------------
#
#
        .globl  D_startinitraid         # Start the processes to init a RAID (C code access)
        .globl  D_que_rinit             # Queue a raid init request (C code access)
        .globl  DEF_QueRInit            # C access
        .globl  D_pause_init            # Pause initialization
        .globl  D_resume_init           # Resume initialization
        .globl  D$delrip                # Delete any RAID inits in progress
        .globl  D$sched_rip             # RAID initialization task
        .globl  D$initraid              # Initialize RAID MRP
#
# --- global data declarations ----------------------------------------
#
        .data
        .align  4
#
#
# --- Raid Initialization data definitions ----------------------------
#
        .section        .shmem
        .align  4
        .globl  d_rip_exec_qu           # Easier for debugging
d_rip_exec_qu:
        .space  16,0                    # Executive QCB
#
        .data
rip_act:
        .word   0                       # Active Raid Init count
#
d_init_lock:
        .word   0                       # Initialization lock
#
d_init_orc:
        .word   0                       # Initialization Outstanding Request Cnt
#
g_count_raid_initialize:
        .word   0
.ifdef PERF
        .set    MAXRAIDINITS,256
.else   # PERF
        .set    MAXRAIDINITS,75
.endif  # PERF
#
# --- executable code (low usage) -------------------------------------
#
        .text
#**********************************************************************
#
#  NAME: D$initraid
#
#  PURPOSE:
#       To provide a means of processing the initialize RAID device
#       request issued by the CCB. Forks one copy of d$initialize_raid
#       for each PSD in the RDD selected.  Waits for each  process to
#       complete.  The forked processes are set to a fairly low priority
#       so as to not interfere unduly with normal system operation
#       Each subprocess will block when memory resources are exhausted,
#       then resume as memory becomes available.
#
#  CALLING SEQUENCE:
#       call    D$initraid
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = status
#       g2 = return pkt size
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
D$initraid:
        ld      mr_ptr(g0),g0           # Get parm block
#
# --- Validate parameters ( RAID ID)
#
        ldos    mid_rid(g0),g13         # Get RAID ID
        ldconst deinvrid,g1             # Prep possible error code
        ld      R_rddindx[g13*4],g6     # Check for pre-existence
                                        # g6 = RDD address
        cmpobe  0,g6,.ir500             # Jif doesn't exist
#
        ldob    rd_type(g6),r3          # r3 = raid type code
        ldconst deinvop,g1              # g1 = possible error code
        cmpobe  rdlinkdev,r3,.ir500     # Jif linked device type raid
        cmpobe  rdslinkdev,r3,.ir500    # Jif snapshot type raid
#
        ldconst deinitinprog,g1         # Prep error code
        ld      rd_iprocs(g6),r3        # Get init procs running
        cmpobne 0,r3,.ir500             # Exit since already an init going
#
# --- Check to see if it is in the queue of RAIDs waiting to be initialized.
#
        lda     d_rip_exec_qu,r6        # Load exec queue
        ldl     qu_head(r6),r12         # Get queue head (r12) and tail (r13)
#
# --- Loop until we hit the end of the queue.
#
.ir150:
        cmpobe  0,r12,.ir190            # Jif nothing to examine
#
# --- Check for init in progress.
#
        lda     -ILTBIAS(r12),r3        # Back up to originators nest
        ld      ri_rdd(r3),r3           # Get RDD from ILT
        cmpobe  r3,g6,.ir500            # Jif already programmed.
#
# --- Move to the next pointer if we are not done.
#
        cmpobe  r12,r13,.ir190          # Done with the checking
        ld      il_fthd(r12),r12        # Get the next ILT
        b       .ir150                  # Not done, process this one
#
# --- Check for copies in progress.
#
.ir190:
        ldos    rd_vid(g6),r3           # Get VID of RID
        ld      V_vddindx[r3*4],r3      # Get VDD
        ld      vd_dcd(r3),r4           # Copy in progress (destination)
        ldconst dedevused,g1            # Prep error code
        cmpobne 0,r4,.ir500             # Jif destination of a copy
        ld      vd_scdhead(r3),r4       # Copy in progress (source)
        cmpobne 0,r4,.ir500             # Jif source of a copy
        ld      vd_vlinks(r3),r4        # Vlink record
        cmpobne 0,r4,.ir500             # Jif VDisk/VLink lock applied
        ldos    vd_attr(r3),r4          # r13 = VDisk attributes
        bbs     vdbasync,r4,.ir500      # Jif asynch (apool)
        bbs     vdbspool,r4,.ir500      # jif spool
#
# --- Check for mappings.
#
        PushRegs(r3)                    # Save all G registers
        ldos    rd_vid(g6),g0           # Get VID
        call    DEF_CheckForMapped      # Check for a mapped vdisk
        mov     g0,r4                   # Save the return code
        PopRegsVoid(r3)                 # Restore all G registers
#
        ldconst dedevused,g1            # Prep error code
        ldconst 0xffff,r3               # Check for -1 (16 bit)
        cmpobne r3,r4,.ir500            # Exit if mapped
#
# --- Check for active defragmentation on this drive.  If there is a verify
# --- running, or we have set the response to move this RID and any PID, then
# --- cancel it by just setting the last response appropriately.
#
        PushRegs(r3)                    # Save registers
        mov     g13,g0                  # Get RID
        call    DF_StopDefragRID        # Stop defrag if running on this RID
        PopRegsVoid(r3)                 # Restore registers
#
# --- Init is safe to proceed.
#
        mov     g13,g0                  # Load RID
        call    RB$cancel_rebld         # Stop rebuilds for this RAID (g0=RID)
#
        mov     g6,g0                   # Load RDD
        call    D_que_rinit             # Queue the raid init request
#
        mov     ecok,g1                 # Succeeded.
#
# --- Exit
#
.ir500:
        ldconst midrsiz,g2              # Set return packet size
        ret
#
#**********************************************************************
#
#  NAME: D_que_rinit
#
#  PURPOSE:
#       This routine creates an ILT to start a raid initialization.
#
#  DESCRIPTION:
#       The RDD is placed into a new ILT.  The ILT is placed onto the
#       Raid Initialization Process (RIP) queue.
#
#  CALLING SEQUENCE:
#       call    D_que_rinit
#
#  INPUT:
#       g0 = RDD to be initialized
#
#  OUTPUT:
#       None
#
#  REGS DESTROYED:
#       None
#
#**********************************************************************
#
DEF_QueRInit:
D_que_rinit:
        movl    g0,r12                  # Save g0 and g1
#
# --- Do a double check to make sure that the RAID init is not already
# --- running or scheduled to run.
#
        ld      rd_iprocs(g0),r3        # Get init procs running
        cmpobne 0,r3,.qri100            # Exit since already an init going
#
# --- Check to see if it is in the queue of RAIDs waiting to be initialized.
#
        lda     d_rip_exec_qu,r6        # Load exec queue
        ldl     qu_head(r6),r12         # Get queue head (r12) and tail (r13)
#
# --- Loop until we hit the end of the queue.
#
.qri50:
        cmpobe  0,r12,.qri60            # Jif nothing to examine
#
# --- Check for init in progress.
#
        lda     -ILTBIAS(r12),r3        # Back up to originators nest
        ld      ri_rdd(r3),r3           # Get RDD from ILT
        cmpobe  r3,g0,.qri100           # Jif already programmed.
#
# --- Move to the next pointer if we are not done.
#
        cmpobe  r12,r13,.qri60          # Done with the checking
        ld      il_fthd(r12),r12        # Get the next ILT
        b       .qri50                  # Not done, process this one
#
# --- Prep the Raids for initializing
#
.qri60:
        ldconst rduninit,r3             # Set status to uninitialized
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        stob    r3,rd_status(g0)
#
        ldob    rd_astatus(g0),r3       # Get the alternate status
        setbit  rdauninit,r3,r3         # Set the init required bit
        clrbit  rdaparity,r3,r3         # Clear the parity scan required bit
        stob    r3,rd_astatus(g0)       # Set the alternate status
#
# --- Check for a cancellation.  If cancelled, don't enqueue.
#
        bbs     rdatermbg,r3,.qri90     # Jif already terminated
#
        ldconst 100,r3
        stob    r3,rd_pctrem(g0)        # Initialize % remaining to 100%
#
# --- Enqueue the raid for initialization
#
c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
        st      g0,ri_rdd(g1)           # Store RDD into ILT
        lda     d$rip_que,g0            # Store queue routine
# NOTE: g2 not initialized, but this queue does not use completion routines.
        call    K$q                     # Queue the raid init request
#
.qri90:
        call    RB_setvirtstat          # Update virtual device status
        call    D$p2update              # Update NVRAM part II
#
.qri100:
        movl    r12,g0                  # Restore g0 and g1
        ret
#
#**********************************************************************
#
#  NAME: d$call_sir
#
#  PURPOSE:
#       This routine calls D_startinitraid.  D_startinitraid gets
#       called from a C function and has input g0 = RDD.  Since g0 is
#       already used, D_startinitraid cannot be forked since g0 is
#       used in the forking process.  RB_setvirtstat and D$p2update
#       are also called after raid init is started.
#
#  DESCRIPTION:
#       Call D_startinitraid
#
#  CALLING SEQUENCE:
#       process call
#
#  INPUT:
#       g6 = RDD to be initialized
#
#  OUTPUT:
#       None
#
#  REGS DESTROYED:
#       None
#
#**********************************************************************
#
d$call_sir:
        mov     g6,g0                   # Load g0 with RDD
        call    D_startinitraid         # Start the raid initialization
        ret
#
#**********************************************************************
#
#  NAME: D_startinitraid
#
#  PURPOSE:
#       Does the actual start of the init raid processing.  This is a
#       function used via the MRP process and via internal raid init
#       processing for restarting raid init after a power cycle.
#
#       Forks one copy of d$initialize_raid for each PSD in the RDD
#       selected.  Waits for each  process to complete.  The forked
#       processes are set to a fairly low priority so as to not
#       interfere unduly with normal system operation
#
#       Each subprocess will block when memory resources are exhausted,
#       then resume as memory becomes available.
#
#  CALLING SEQUENCE:
#       call    D_startinitraid
#
#  INPUT:
#       g0 = RDD to be initialized
#
#  OUTPUT:
#       None
#
#  REGS DESTROYED:
#       g0, g1, g4, g8, g13, g6
#
#**********************************************************************
#
D_startinitraid:
#
# --- Set g6 to RDD
#
        mov     g0,g6
        ldos    rd_rid(g6),g13          # Get RID
        ld      R_rddindx[g13*4],g0     # Get RDD from R_rddindx
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        cmpobne g0,g6,.sir90            # Exit if not the same
#
        movl    0,g4                    # Set up a zero
        st      g4,rd_iprocs(g6)        # Clear number of running child procs
        stl     g4,rd_isectors(g6)      # Clear # sectors initialized
        st      g4,rd_ierrors(g6)       # Clear raid init error counter
#
# --- Check if we are master.
#
        ldos    K_ii+ii_status,r4       # Get current initialization status
        bbc     iimaster,r4,.sir90      # Exit if not master
#
        ld      rd_psd(g6),g9           # Get the first PSD
        mov     g9,g8
#
# --- Loop for each PSD
# --- g4/g5 is holding the amount to initialize
#
.sir10:
        ldconst psinit,r3               # Set PSD to initializing
        stob    r3,ps_status(g9)
#
        ldob    ps_astatus(g9),r3
        setbit  psauninit,r3,r3         # Set additional status to uninitialized
        stob    r3,ps_astatus(g9)
#
        ld      rd_iprocs(g6),r3
        addo    1,r3,r3
        st      r3,rd_iprocs(g6)        # Incr number of running child procs
#
        ldl     ps_slen(g9),r4          # r4 = length of this segment
        cmpo    0,1                     # Set up for long add
        addc    r4,g4,g4
        addc    r5,g5,g5                # Add to total sector count
#
# --- Fork a Raid Initialization process
#
.if     DEBUG_FLIGHTREC_O
        ldconst frt_h_misc8,r3          # Start of initialize RAID function
        st      r3,fr_parm0             # Function
        ldos    ps_pid(g9),r4           # PID
        shlo    16,g13,r3               # RID<<16
        or      r3,r4,r3
        st      r3,fr_parm1             # RID/PID
        st      g6,fr_parm2             # RDD
        st      g9,fr_parm3             # PSD
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_O
c       while (g_count_raid_initialize > MAXRAIDINITS) {
c           g0 = 1000;
            call K$twait
c       }
c       g_count_raid_initialize++;
        lda     d$initialize_raid,g0
        ldconst DINITRAIDPRIO,g1
c       CT_fork_tmp = (ulong)"d$initialize_raid";
        call    K$tfork                 # Spawn a proc for each PSD in the raid
                                        # g0  - process address
                                        # g1  - priority
                                        # g6  - RDD
                                        # g9  - PSD
                                        # g13 - RID
#
        ld      ps_npsd(g9),g9
        cmpobne g8,g9,.sir10            # Jif not the last PSD
#
# --- Now calculate one per cent of the total.  This will be used to
# --- check the progress of the RAID initialization.
#
c       *(UINT64*)&g4 = *(UINT64*)&g4 / 100;
#
# --- Now for a process to monitor for completion.  This process will take
# --- the RDD (g6) and the "one percent" value (g4/g5) as input.
#
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        ld      rd_iprocs(g6),r3
        addo    1,r3,r3
        st      r3,rd_iprocs(g6)        # Incr number of running child procs
#
        lda     d$init_raid_cmplter,g0
        ldconst DINITRAIDPRIO,g1
c       CT_fork_tmp = (ulong)"d$init_raid_cmplter";
        call    K$tfork                 # Spawn the completer process
        b       .sir100                 # Skip to exit
#
# --- Exit without initializing the Raid.  Must decrement rip_act
#
.sir90:
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        ldconst 0,r4                    # Zero percent remaining
        stob    r4,rd_pctrem(g6)
#
        ld      rip_act,r4              # r4 = # active inits in progress
        cmpobe  0,r4,.sir100            # Jif count is already at zero
        subo    1,r4,r4                 # Decrement active count
        st      r4,rip_act              # Store new count
#
# --- Exit
#
.sir100:
        ret
#
#**********************************************************************
#
#  NAME: d$rip_que
#
#  PURPOSE:
#       To provide a common means of queuing raid initialization
#       requests.
#
#  DESCRIPTION:
#       The ILT and associated RIP are queued to the executive.  The
#       executive is then activated to process this request.  This
#       routine may be called from either the process or interrupt level.
#
#  CALLING SEQUENCE:
#       call    d$rip_que
#
#  INPUT:
#       g1 = ILT
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#  ILT USAGE:
#       CALLER AREA
#       ___________
#       W0 = RIP
#
#**********************************************************************
#
d$rip_que:
        lda     d_rip_exec_qu,r11       # Get executive queue pointer
        b       K$cque
#
#**********************************************************************
#
#  NAME: D$sched_rip
#
#  PURPOSE:
#       To provide a means of scheduling raid initializations to begin.
#
#  DESCRIPTION:
#       This process takes raid initialization process RIPs and activates
#       them in the order in which they are scheduled limiting the
#       maximum number of inits taking place to eliminate the
#       potential for depleting the memory pool. This process activates
#       every second to check if any new raid inits have been
#       initiated and if so checks if the maximum number of allowed
#       inits are active. If not, it activates another raid init by
#       calling d$initialize_raid and increments the active init count.
#       If the maximum number of inits are active, it simply waits for
#       the next time period to occur and checks again.
#
#  CALLING SEQUENCE:
#       process call
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
# --- Set this process to not ready
#
.dsr_10:
        ldconst pcnrdy,r4               # Set this process to not ready
        stob    r4,pc_stat(r15)
#
# --- Exchange processes ----------------------------------------------
#
D$sched_rip:
        call    K$qxchang               # Exchange processes
#
# --- Don't start initializing a RAID until the system is initialized
#
.dsr_20:
        ldob    O_p2init,r3             # Get phase II inits complete
        cmpobe  TRUE,r3,.dsr_30         # Jif completed
        ldconst 3000,g0                 # Wait a while and try again
        call    K$twait
        b       .dsr_20
#
.dsr_30:
#
# --- Get next queued request
#
        lda     d_rip_exec_qu,r6        # Load exec queue
        ldq     qu_head(r6),r12         # Get queue head, tail, count, PCB
        mov     r12,g3                  # Isolate next queued ILT
        cmpobe  0,r12,.dsr_10           # Jif no work to do
#
# --- Work for process to do
#
        ld      rip_act,r4              # r4 = # active inits in progress
        ldconst DRIPMAXACT,r5           # r5 = max. # raid inits allowed
        cmpobge r4,r5,.dsr_10           # Jif max. # inits in progress
        addo    1,r4,r4                 # inc. active raid init count
        st      r4,rip_act              # save updated count
#
# --- Remove this request from queue
#
        ld      il_fthd(g3),r12         # Dequeue this ILT
        subo    1,r14,r14               # Adjust queue count
        cmpo    0,r12                   # Check for queue now empty
        sele    r13,r12,r13             # Set up queue tail
        stt     r12,qu_head(r6)         # Update queue head, tail & count
        be      .dsr_40                 # Jif queue now empty
#
        st      r6,il_bthd(r12)         # Update backward thread
#
.dsr_40:
        lda     -ILTBIAS(g3),g1         # Back up to originators nest
        ld      ri_rdd(g1),r4           # Get RDD from ILT
#
# --- Deallocate the ILT
#
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
#
# - Set RDD status to initializing
#
        ldconst rdinit,r3               # Set status to initializing
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        stob    r3,rd_status(r4)
#
# --- Fork off process to initialize this PSD
#
        lda     d$call_sir,g0           # Load process address
        ldconst DINITRAIDPRIO,g1        # Load priority
        mov     r4,g6                   # Load RDD
c       CT_fork_tmp = (ulong)"d$call_sir";
        call    K$tfork                 # Spawn a proc for this PSD in the raid
#
        b       D$sched_rip             # and check for more raid init ops.
                                        #  to schedule
#
#**********************************************************************
#
#  NAME: d$init_raid_cmplter
#
#  PURPOSE:
#       Watches over a RAID initialization
#
#  DESCRIPTION:
#       This process will go to sleep for a second at a time and then
#       check the completion progress.  It will update the RDD at each
#       wakeup.
#
#       If the RAID initialization was successful, the RAID will be
#       placed into an operable state and in the case of a deferred
#       RAID, will be put in the normal list.
#
#       If the RAID init was not successful due to a termination, then
#       the RAID will be left an uninitialized and the restarting of the
#       initialization will be done later.
#
#  CALLING SEQUENCE:
#       process call
#
#  INPUT:
#       g4/g5 = one percent of total
#       g6 = RDD
#       g13 = RID
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
d$init_raid_cmplter:
#
.irc10:
        ldconst 500,g0                  # Wait...
        call    K$twait
#
# --- Check if the RAID still exists.
#
        ld      R_rddindx[g13*4],r3     # Get RDD pointer
        cmpobe  0,r3,.irc170            # Exit w/o cleaning up anything
#
        ldl     rd_isectors(g6),r4      # r4 = # sectors initialized
c       *(UINT64 *)&r6 = *(UINT64 *)&r4 / *(UINT64 *)&g4;
#ifndef PERF
c       if (r7 != 0) {
c           abort();
c       }
#endif  # !PERF
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        ldconst 100,r3                  # Subtract from 100 percent
        subo    r6,r3,r6                # r6 = % remaining
        stob    r6,rd_pctrem(g6)        # Save % rem. in RDD
#
# --- Wait for all child processes to complete.  When all of the init
# --- processes are done (drive init procs), there will be one process
# --- left which is this process.
#
        ld      rd_iprocs(g6),r4        # Get number of processes still running
        cmpobne 1,r4,.irc10             # Not all completed, go back to sleep
#
        ldconst 0,r4
        stob    r4,rd_pctrem(g6)        # Zero the percentage for rounding errs
#
# --- Set the status of the RAID based upon the PSD status rather than
# --- on the error count.  This is done to get a partially initialized
# --- RAID to a status of degraded if possible.
#
        ldob    rd_astatus(g6),r4       # Load additional status
        ld      rd_ierrors(g6),r3       # Load the error count
#
        bbs     rdatermbg,r4,.irc30     # Jif initialization terminated early
        cmpobne 0,r3,.irc40             # Jif any failures
#
        clrbit  rdauninit,r4,r4         # Clear the uninitialized bit
        clrbit  rdarebuild,r4,r4        # Clear the rebuild bit
        ldconst erigood,r11             # Good complete - pass r11 in log
        b       .irc50
#
.irc30:
        ldconst eriterm,r11             # Terminated early log code
        b       .irc50
#
.irc40:
        clrbit  rdauninit,r4,r4         # Clear the uninitialized bit
        ldconst erifail,r11             # Failed with errors log code
#
.irc50:
        clrbit  rdatermbg,r4,r3         # Clear the terminate bit
        stob    r3,rd_astatus(g6)       # Save additional status
#
        mov     g6,g1
        call    RB_setraidstat          # Set the status based upon the PSDs
#
        bbs     rdatermbg,r4,.irc145    # Jif initialization terminated early
#
# --- Now decide whether or not this RAID has to be placed into service on
# --- the assigned VDD.  It will be placed into service if the status is
# --- degraded or better.  If the device is inoperable, it will be left in the
# --- deferred state.
#
        ldob    rd_status(g6),r3        # Get the status
        cmpobl  rdop,r3,.irc145         # If less than operable, don't process
#
        ldos    rd_vid(g6),r3           # Get the VDD which owns this RAID
        ld      V_vddindx[r3*4],r15     # VDD
#
        ld      vd_rdd(r15),r12         # Find the end of the current RAID
#
.irc80:
        ld      rd_nvrdd(r12),r3        # Get the next one
        cmpobe  0,r3,.irc90             # Done.  r12 has the last RDD in the VDD
        mov     r3,r12                  # Grab next one
        b       .irc80
#
.irc90:
        lda     vd_drdd(r15),r14        # Trailing pointer location
        ld      vd_drdd(r15),r13        # Deferred queue of RAIDs
#
# --- At this point, parse the list of deferred entries to see if this RAID
# --- was in the deferred list.  If it was, then it indicates that the RAID
# --- was not officially in the VDD so we have to move it in.  If it is not
# --- in the list, then just log that it completed.  This happens when a
# --- user initiated initialize is performed or the initial one in a VDD
# --- creation.
#
.irc130:
        cmpobe  0,r13,.irc145           # Jif done with list or empty
        cmpobne r13,g6,.irc140          # Not the one we are on, move on to next
#
# --- Found.
#
        ld      rd_nvrdd(r13),r3        # Get the next pointer
        st      r3,(r14)                # Deque it
#
# Add new RAID capacity into vdisk capacity.
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
c       ((VDD *)r15)->devCap += ((RDD *)r13)->devCap;
#
        ldob    vd_draidcnt(r15),r3     # Decrement the deferred count
        subo    1,r3,r3
        stob    r3,vd_draidcnt(r15)
#
        ldob    vd_raidcnt(r15),r3      # Increment the raid count
        addo    1,r3,r3
        stob    r3,vd_raidcnt(r15)
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
#
        st      g6,rd_nvrdd(r12)        # Put it in the VDD real list
        ldconst 0,r3
        st      r3,rd_nvrdd(g6)         # NULL the next pointer
#
        ldos    vd_vid(r15),g0          # Get the VID
        ldconst FALSE,g1                # Do not delete.  Update.
        call    DEF_UpdRmtCacheSingle   # Update the FE
        b       .irc145                 # Log and out
#
.irc140:
        lda     rd_nvrdd(r13),r14       # Get the next pointer to pointer
        ld      rd_nvrdd(r13),r13       # Move the pointer
        b       .irc130                 # Look again
#
.irc145:
        ldos    rd_vid(g6),r5           # Get VID
        ld      V_vddindx[r5*4],r15     # VDD
        cmpobe  0,r15,.irc150           # jif VDD is NULL
        ldos    vd_attr(r15),r3         # r5 = vdisk attributes
        bbc     vdbspool,r3,.irc150     # jif not snappool
        ldos    vd_vid(r15),g1
        call    D$update_spool_percent_used # update the percent usage on spool
.irc150:
        call    RB_setvirtstat          # Update virtual device status
#
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst mleraidinitdone,r3      # Get log code
        ldconst mleinform,r5
        ldconst mleerror,r6
        cmpo    erifail,r11             # Check fail completion status
        sele    r5,r6,r4                # Error log if it failed else info
        or      r3,r4,r3
        ldconst mlecustomer,r5
        ldconst mledebug,r6
        cmpo    eriterm,r11             # Check terminated completion status
        sele    r5,r6,r4                # Debug log if terminated else customer
        or      r3,r4,r3
        st      r3,mle_event(g0)        # Store as word to clear other bytes
        ldconst 0,r3                    # Clear status field and 3 reserved bytes
        st      r3,mle_bstream(g0)
        ldos    rd_rid(g6),r3           # Get RID
        stos    r3,eri_rid(g0)
        ldos    rd_vid(g6),r3           # Get VID
        stos    r3,eri_vid(g0)
        ld      rd_ierrors(g6),r3       # Errors during init raid ops.
        st      r3,eri_ecnt(g0)
        stob    r11,eri_status(g0)      # Completion status
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], erilen);
#
.irc170:
#
# --- Decrement raid initialization processes active count
#
        ld      rip_act,r4              # r4 = # active inits in progress
        cmpobe  0,r4,.irc180            # Jif count is already at zero
        subo    1,r4,r4                 # Decrement active count
        st      r4,rip_act              # Store new count
.irc180:
#
# --- Activate the d$sched_rip to check for next raid init request
#
        lda     d_rip_exec_qu,r4        # Get the raid init queue
        ld      qu_pcb(r4),r3           # Get d$sched_rip
        cmpobe  0,r3,.irc200            # Jif no PCB present
        ldob    pc_stat(r3),r6          # Get current process state
        cmpobne r6,pcnrdy,.irc200       # If task is current not not ready.
        ldconst pcrdy,r5                # Prepare a ready status
.ifdef HISTORY_KEEP
c CT_history_pcb(".irc180 setting ready pcb", r3);
.endif  # HISTORY_KEEP
        stob    r5,pc_stat(r3)          # Ready the process
.irc200:
        call    RB_searchforfailedpsds  # Kickoff any needed rebuilds
        call    D$p2update              # Update NVRAM
#
# --- Lastly, clear out the iprocs entry to indicate that this init is
# --- completely finished.
#
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        ldconst 0,r3
        st      r3,rd_iprocs(g6)
#
        ret
#
#**********************************************************************
#
#  NAME: d$initialize_raid
#
#  PURPOSE:
#       Write all zero bytes to all sectors in a PSD, one segment at a time
#       NOTE: Forked by d$sched_rip (one process for each PSD in the selected
#       RDD)
#
#  DESCRIPTION:
#       The write same command is utilized whenever possible to clear the
#       disk area defined by the specified PSD.  If the write same command
#       is not implemented by the device, a normal write command is used
#       instead.
#
#       The current level of system activity is used to regulate the
#       rate at which this process initiates I/O.
#
#       This is also done in two versions.  One for the 750 and one for the
#       3000.  It is done this way since the 750 has memory limitations caused
#       by the write command being used rather than a write same.  Once the
#       write same is supported, this version should be pulled and replaced
#       with the 3ooo version.
#
#  CALLING SEQUENCE:
#       process call
#
#  INPUT:
#       g6 = RDD
#       g9 = PSD
#       g13= RID
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
.ifdef DISABLE_WRITE_SAME
#
d$initialize_raid:
#
# --- First check that the RAID still exists.
#
        ld      R_rddindx[g13*4],r3     # Get RDD pointer
        cmpobe  0,r3,.inr100            # Exit w/o cleaning up anything
        ldos    ps_pid(g9),g3           # Get the PID
        ld      P_pddindx[g3*4],g3      # Get the PDD for genreq
        cmpobe  0,g3,.inr100            # Exit w/o cleaning up anything
#
# --- The RAID still exists.  Assume that all of the underlying structures
# --- are also OK.
#
!       ldl     ps_sda(g9),r6           # SDA for 1st op
c       *((UINT64*)&r10) = *((UINT64*)&r6) + DSKSALLOC/4; # EDA
c       ((PSD*)g9)->rLen = 0;           # Clear rebuild length
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
#
        ldl     ps_slen(g9),g4          # Create final EDA
c       *((UINT64*)&g4) = *((UINT64*)&g4) + *((UINT64*)&r6);
#
# --- Iterate through generating a new request for each iteration of the loop.
# --- This is done to help prevent memory starvation.
# --- Wait if the initialize tasks are locked
#
.inr20:
        ld      d_init_lock,r3          # Get lock count
        cmpobe  0,r3,.inr27             # Jif no locks - continue
c       TaskSetMyState(pcinitwait);     # Set this process to init wait state
        call    K$xchang                # Give up control
        b       .inr20                  # Check status again
#
# --- Check early exit conditions
#
.inr27:
        ld      R_rddindx[g13*4],r3     # Check RAID existence
        cmpobe  0,r3,.inr100            # Exit cleaning up command record
#
        ldob    rd_astatus(g6),r3       # Check init termination
        bbs     rdatermbg,r3,.inr60     # Exit if termination requested
#
# --- Control the amount of initialization IO based on customer IO
#
        ld      V_orc,g0                # Get outstanding request count
        ldconst INIT_LOAD_FACTOR,r4     # Calculate wait time
        mulo    r4,g0,g0
        lda     prlow,r3                # Set to low priority for now
        ldconst QUANTUM,r4
        cmpobg  r4,g0,.inr30            # Jif no wait required
#
        ldconst INIT_LOAD_MAX,r3        # Clip at maximum wait
        cmpo    r3,g0
        sell    g0,r3,g0
        call    K$twait                 # wait
#
        ld      R_rddindx[g13*4],r3     # Check RAID existence
        cmpobe  0,r3,.inr100            # Exit cleaning up command record
#
        ldconst prnorm,r3               # Set to normal priority
#
# --- Generate PRP before issuing SCSI command
#
.inr30:
#   ILT* = generate_scsi_write(PDD*, SDA, transfer_length, PRP**);
# NOTE: this "c" routine will destroy g0, g1, g2, g3 --  g2 is the 4th argument.
# g0 is output ILT.
# g1 also happens to be ILT.
# g2 is the PRP -- which we pass as address, so g2=g2 is effectively done..
c       r15 = g3;
c       g1 = generate_scsi_write(g3, *((UINT64*)&r6), DSKSALLOC/4, (UINT32*)&g2);
c       g3 = r15;
        stob    r3,pr_strategy(g2)
#
        ld      d_init_orc,r3           # Increment initialization
        addo    1,r3,r3                 #   outstanding request count
        st      r3,d_init_orc
# --- Issue SCSI command
        call    O$quereq                # Queue I/O request - wait until done
#
        ld      R_rddindx[g13*4],r3     # Check RAID existence
        cmpobe  0,r3,.inr55             # Exit cleaning up command record
#
        call    M$chkstat               # Check IO request status
        cmpobne ecok,g0,.inr50          # Jif IO request not OK
#
# --- Advance to next portion of segment
#
        call    O$relreq                # Release request, g1=ILT

        ld      d_init_orc,r3           # Decrement initialization
        subo    1,r3,r3                 #   outstanding request count
        st      r3,d_init_orc
#
        ldl     rd_isectors(g6),r4      # Adjust sectors initialized
c       *(UINT64 *)&r4 = *(UINT64 *)&r4 + (UINT64)(DSKSALLOC)/4;
        stl     r4,rd_isectors(g6)
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
c   if (*((UINT64*)&r10) >= *((UINT64*)&g4)) {
        b       .inr60                  # If complete
c   }
#
c       ((PSD*)g9)->rLen += (UINT64)DSKSALLOC/4; # Adjust rebuild length
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
c       *((UINT64*)&r6) += (UINT64)DSKSALLOC/4;  # Adjust SDA
c       *((UINT64*)&r10) += (UINT64)DSKSALLOC/4; # Adjust EDA
        b       .inr20                  # End of main loop
#
# --- Error detected during RAID initialization -----------------------
#     Flag the PDD as having an error and queue an ILT to the error handler.
#     The RAID error handler has the logic to determine if this is a single
#     drive failure that should be rebuilt or if the entire RAID has become
#     inoperable.
#
.inr50:
        ld      d_init_orc,r3           # Decrement initialization
        subo    1,r3,r3                 #   outstanding request count
        st      r3,d_init_orc           #   after error is queued
#
        call    O$relreq                # Release request, g1=ILT
#
        ld      rd_ierrors(g6),r3       # Bump init error count
        addo    1,r3,r3
        st      r3,rd_ierrors(g6)
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
#
        ldob    rd_type(g6),r3          # Get type
        cmpobe  rdstd,r3,.inr60         # Jif standard device or
        cmpobe  rdraid0,r3,.inr60       # Jif RAID 0 - ignore error & exit
#
        mov     g9,g0                   # Pass PSD
        mov     g6,g1                   # Pass RDD
        call    RB$rerror               # Flag error in this PDD, g2=PRP
#
c       g1 = get_ilt();                 # Allocate an ILT.
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif  # M4_DEBUG_ILT
c       g2 = get_rrp();                 # Allocate an RRP.
.ifdef M4_DEBUG_RRP
c CT_history_printf("%s%s:%u get_rrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif  # M4_DEBUG_RRP
c       ((ILT *)g1)->ilt_normal.w0 = g2; # Link to RRP in ILT.
        lda     ILTBIAS(g1),g1          # Advance to next ILT level
        ldconst 0,r3
        ldconst rrverify,r4
        st      r3,rr_sglptr(g2)        # Set SGL = 0 when we release ILT/RRP
        st      r3,rr_sglsize(g2)
        st      r3,il_w5(g1)            # Pass NVA entry = 0
        stos    r4,rr_func(g2)          # A verify op failed
        st      g6,il_w4(g1)            # Pass RDD
        lda     RB$rerror_comp,r3       # Pass generic completion routine
        st      r3,il_cr(g1)
        call    RB$rerror_que           # Queue this ILT to error handler
        b       .inr65                  # Exit
#
# - RDD is deleted and d_init_orc needs to be decremented
#
.inr55:
        call    O$relreq                # Release I/O request, g1=ILT
#
        ld      d_init_orc,r3           # Decrement initialization
        subo    1,r3,r3                 #   outstanding request count
        st      r3,d_init_orc           #   after error is queued
        b       .inr100
#
# --- Process termination
#
.inr60:
        ldob    ps_astatus(g9),r4
        clrbit  psauninit,r4,r4         # Clear additional status
        clrbit  psarebuild,r4,r4        # Rebuild no longer needed
        stob    r4,ps_astatus(g9)
#
        ldconst psop,r3                 # Set good status
        stob    r3,ps_status(g9)        # Set the code
#
# --- Update RDD before exit
#
.inr65:
        ld      rd_iprocs(g6),r3        # Decrement process count
        subo    1,r3,r3
        st      r3,rd_iprocs(g6)
#
c       ((PSD*)g9)->rLen = 0;           # Clear rebuild length
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
#
# --- Exit
#
.inr100:
c       g_count_raid_initialize--;
        ret

.else   # DISABLE_WRITE_SAME

d$initialize_raid:
#
# --- First check that the RAID still exists.
#
        ld      R_rddindx[g13*4],r3     # Get RDD pointer
        cmpobe  0,r3,.INR100            # Exit
        ldos    ps_pid(g9),g3           # Get the PID
        ld      P_pddindx[g3*4],g3      # Get the PDD for genreq
        cmpobe  0,g3,.INR100            # Exit
c       r8 = 0;                         # Start with writesame command.
#
# --- The RAID still exists.  Assume that all of the underlying structures
# --- are also OK.  Generate basic write same request
#
.INR10:
!       ldl     ps_sda(g9),r6           # SDA for 1st op
c       *((UINT64*)&r10) = *((UINT64*)&r6) + DSKSALLOC; # EDA
c       ((PSD*)g9)->rLen = 0;           # Clear rebuild length
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
#
        ldl     ps_slen(g9),g4          # Create final EDA
c       *((UINT64*)&g4) = *((UINT64*)&g4) + *((UINT64*)&r6);
#
# --- Loop to write all segments in the PSD ---------------------------
# --- Wait if the initialize tasks are locked
#
.INR20:
        ld      d_init_lock,r3          # Get lock count
        cmpobe  0,r3,.INR27             # Jif no locks - continue
c       TaskSetMyState(pcinitwait);     # Set this process to init wait state
        call    K$xchang                # Give up control
        b       .INR20                  # Check status again
#
# --- Check early exit conditions
#
.INR27:
        ld      R_rddindx[g13*4],r3     # Check RAID existence
        cmpobe  0,r3,.INR100            # Exit
#
        ldob    rd_astatus(g6),r3       # Check init termination
        bbs     rdatermbg,r3,.INR60     # Exit if termination requested
#
# --- Control the amount of initialization IO based on customer IO
#
        ld      V_orc,g0                # Get outstanding request count
        ldconst INIT_LOAD_FACTOR,r4     # Calculate wait time
        mulo    r4,g0,g0
        lda     prlow,r3                # Set to low priority for now
        ldconst QUANTUM,r4
        cmpobg  r4,g0,.INR30            # Jif no wait required
#
        ldconst INIT_LOAD_MAX,r3        # Clip at maximum wait
        cmpo    r3,g0
        sell    g0,r3,g0
        call    K$twait                 # wait
#
        ld      R_rddindx[g13*4],r3     # Check RAID existence
        cmpobe  0,r3,.INR100            # Exit
#
        ldconst prnorm,r3               # Set to normal priority
#
# --- Create PRP & ILT before issuing SCSI command
#
.INR30:
#   ILT* = generate_scsi_write/writesame(PDD*, SDA, transfer_length, PRP**);
# NOTE: these "c" routine calls will destroy g0, g1, g2, but that is okay.
c       r15 = g3;
c   if (r8 == 0) {
c       g1 = generate_scsi_writesame(g3, *((UINT64*)&r6), DSKSALLOC, (UINT32*)&g2);
c   } else {
c       g1 = generate_scsi_write(g3, *((UINT64*)&r6), DSKSALLOC, (UINT32*)&g2);
c   }
c       g3 = r15;
        stob    r3,pr_strategy(g2)
#
        ld      d_init_orc,r3           # Increment initialization
        addo    1,r3,r3                 #   outstanding request count
        st      r3,d_init_orc
# --- Issue SCSI command
        call    O$quereq                # Queue I/O request - wait until done
#
        ld      R_rddindx[g13*4],r3     # Check RAID existence
        cmpobe  0,r3,.INR55             # Exit cleaning up command record
#
        call    M$chkstat               # Check IO request status
        cmpobe  ecok,g0,.INR35          # Jif IO request OK
#
        cmpobne eccheck,g0,.INR50       # If not check condition
#
        ldob    pr_sense+2(g2),r3       # Get sense key
        cmpobe  5,r3,.INR40             # Jif illegal command (not supported)
        b       .INR50
#
# --- Advance to next portion of segment
#
.INR35:
        call    O$relreq                # Release I/O request, g1=ILT

        ld      d_init_orc,r3           # Decrement initialization
        subo    1,r3,r3                 #   outstanding request count
        st      r3,d_init_orc
#
        ldl     rd_isectors(g6),r4      # Adjust sectors initialized
c       *(UINT64 *)&r4 = *(UINT64 *)&r4 + (UINT64)(DSKSALLOC);
        stl     r4,rd_isectors(g6)
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
c   if (*((UINT64*)&r10) >= *((UINT64*)&g4)) {
        b       .INR60                  # If complete
c   }
#
c       ((PSD*)g9)->rLen += (UINT64)DSKSALLOC; # Adjust rebuild length
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
c       *((UINT64*)&r6) += (UINT64)DSKSALLOC;  # Adjust SDA
c       *((UINT64*)&r10) += (UINT64)DSKSALLOC; # Adjust EDA
        b       .INR20                  # End of main loop
#
# --- Generate write extended request since writesame isn't supported
#
.INR40:
        call    O$relreq                # Release writesame request, g1=ILT
#
c       r8 = 1;                         # Flag to use write command, not writesame.
#
        ld      d_init_orc,r3           # Decrement initialization
        subo    1,r3,r3                 #   outstanding request count
        st      r3,d_init_orc           #   after error is queued
#
        b       .INR10                  # Start over
#
# --- Error detected during RAID initialization -----------------------
#     Flag the PDD as having an error and queue an ILT to the error handler.
#     The RAID error handler has the logic to determine if this is a single
#     drive failure that should be rebuilt or if the entire RAID has become
#     inoperable.
#
.INR50:
        ld      d_init_orc,r3           # Decrement initialization
        subo    1,r3,r3                 #   outstanding request count
        st      r3,d_init_orc           #   after error is queued
#
        call    O$relreq                # Release writesame request, g1=ILT
#
        ld      rd_ierrors(g6),r3       # Bump init error count
        addo    1,r3,r3
        st      r3,rd_ierrors(g6)
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
#
        ldob    rd_type(g6),r3          # Get type
        cmpobe  rdstd,r3,.INR60         # Jif standard device or
        cmpobe  rdraid0,r3,.INR60       # Jif RAID 0 - ignore error & exit
#
        mov     g9,g0                   # Pass PSD
        mov     g6,g1                   # Pass RDD
        call    RB$rerror               # Flag error in this PDD, g2=PRP
#
c       g1 = get_ilt();                 # Allocate an ILT.
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif  # M4_DEBUG_ILT
c       g2 = get_rrp();                 # Allocate an RRP.
.ifdef M4_DEBUG_RRP
c CT_history_printf("%s%s:%u get_rrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif  # M4_DEBUG_RRP
c       ((ILT *)g1)->ilt_normal.w0 = g2; # Link to RRP in ILT.
        lda     ILTBIAS(g1),g1          # Advance to next ILT level
        ldconst 0,r3
        ldconst rrverify,r4
        st      r3,rr_sglptr(g2)        # Set SGL = 0 when we release ILT/RRP
        st      r3,rr_sglsize(g2)
        st      r3,il_w5(g1)            # Pass NVA entry = 0
        stos    r4,rr_func(g2)          # A verify op failed
        st      g6,il_w4(g1)            # Pass RDD
        lda     RB$rerror_comp,r3       # Pass generic completion routine
        st      r3,il_cr(g1)
        call    RB$rerror_que           # Queue this ILT to error handler
        b       .INR65                  # Exit
#
# - RDD is deleted and d_init_orc needs to be decremented
#
.INR55:
        call    O$relreq                # Release I/O request, g1=ILT
#
        ld      d_init_orc,r3           # Decrement initialization
        subo    1,r3,r3                 #   outstanding request count
        st      r3,d_init_orc           #   after error is queued
        b       .INR100
#
# --- Process termination
#
.INR60:
        ldob    ps_astatus(g9),r4
        clrbit  psauninit,r4,r4         # Clear additional status
        clrbit  psarebuild,r4,r4        # Rebuild no longer needed
        stob    r4,ps_astatus(g9)
#
        ldconst psop,r3                 # Set good status
        stob    r3,ps_status(g9)        # Set the code
#
# --- Update RDD before exit
#
.INR65:
        ld      rd_iprocs(g6),r3        # Decrement process count
        subo    1,r3,r3
        st      r3,rd_iprocs(g6)
#
c       ((PSD*)g9)->rLen = 0;           # Clear rebuild length
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
#
# --- Exit
#
.INR100:
c       g_count_raid_initialize--;
        ret
.endif  # DISABLE_WRITE_SAME

#**********************************************************************
#
#  NAME: D_pause_init
#
#  PURPOSE:
#       Pause the RAID initialization tasks.
#
#  DESCRIPTION:
#       This will increment the lock count and wait for the initialization
#       outstanding request counts to go to 0.
#
#  CALLING SEQUENCE:
#       call D_pause_init
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
D_pause_init:
#
# --- Increment the lock count
#
        ld      d_init_lock,r3
        addo    1,r3,r3
        st      r3,d_init_lock
#
# --- Wait for inits to halt
#     Another solution to waiting for the orc to go to zero would be to
#     have each task that calls d$check_init_lock to increment a count.
#     When that count reaches the active initialization task count then
#     the task that called D_pause_init can continue. We already have
#     an active init task count I believe as part of the init queueing
#     mechanism. Wish I had thought of this sooner. JT
#
.dsi10:
        ld      d_init_orc,r3           # Get outstanding request count
        cmpobe  0,r3,.dsi1000           # Jif done and exit
#
        mov     g0,r15                  # Save g0
        ldconst 500,g0                  # Wait a while
        call    K$twait
        mov     r15,g0
        b       .dsi10
#
# --- Exit
#
.dsi1000:
        ret
#
#**********************************************************************
#
#  NAME: D_resume_init
#
#  PURPOSE:
#       Resume the RAID initialization tasks that were previously paused.
#
#  DESCRIPTION:
#       This will decrement the lock count and if it goes to 0 will
#       release any locked processes.
#
#  CALLING SEQUENCE:
#       call D_resume_init
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
D_resume_init:
#
# --- Decrement the lock count
#
        ld      d_init_lock,r3
        subo    1,r3,r3
        st      r3,d_init_lock
#
# --- Unlock the processes if the lock count goes to 0
#
        cmpobne 0,r3,.dri1000           # Jif lock NE 0
#
c       TaskReadyByState(pcinitwait);   # Ready the init tasks
#
# --- Exit
#
.dri1000:
        ret
#
#**********************************************************************
#
#  NAME: D$delrip
#
#  PURPOSE:
#       To provide a means of deleting a Raid Initialization Packet (RIP)
#       from the d_rip_exec_qu.
#
#  DESCRIPTION:
#       This function looks for the RIP ILT entry on the d_rip_exec_qu
#       that matches the input RDD.  If a matching entry is found, that
#       RIP entry is deleted.
#
#  CALLING SEQUENCE:
#       call    D$delrip
#
#  INPUT:
#       g0 = RDD
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
D$delrip:
        movl    g0,r10                  # Save g0 and g1
        cmpobe  0,g0,.drip100           # Jif input RDD equals 0
#
# --- Walk the d_rip_exec_qu looking for RIP matching g0
#
        lda     d_rip_exec_qu,r6        # Load the queue address
        ldq     qu_head(r6),r12         # Load queue head,tail,cnt,pcb
        mov     r12,r5                  # Load the head into r5
.drip10:
        cmpobe  0,r5,.drip90            # Jif next RIP equal 0
        ld      ri_rdd-ILTBIAS(r5),r4   # Get the RDD for this RIP
        cmpobe  r4,r10,.drip20          # Jif an RDD match is found
        ld      il_fthd(r5),r5          # Load the next RIP ILT
        b       .drip10
#
# ---- An RDD match was found.  Now delete the RIP ILT from the queue
#
.drip20:
        ldl     il_fthd(r5),r8          # Load forward and back thread
#
# --- Change the percentage to zero to indicate it is done (dequeued)
#
        ldconst 0,r3
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        stob    r3,rd_pctrem(r10)       # Set percent remaining to zero
        st      r3,rd_iprocs(r10)       # Set proc count to zero
#
        st      r8,il_fthd(r9)          # Store fwd thread to next RIP
        cmpobe  0,r8,.drip30            # Jif we're at the end of queue
        st      r9,il_bthd(r8)          # Store back thread to prev RIP
        b       .drip40
#
.drip30:
        cmpo    r9,r6                   # Is prev the queue header
        sele    r9,0,r3                 # Determine qu_tail
        st      r3,qu_tail(r6)          # Store qu_tail
.drip40:
        subo    1,r14,r14               # Decrement qcount
        st      r14,qu_qcnt(r6)         # Store qcount
#
# --- Deallocate the ILT
#
        lda     -ILTBIAS(r5),g1         # Load ILT addr into g1
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
.drip90:
.drip100:
        movl    r10,g0                  # Restore g0 and g1
        ret
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
