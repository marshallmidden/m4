# $Id: rebld.as 161128 2013-05-20 20:37:15Z marshall_midden $
#**********************************************************************
#
#  NAME: rebld.as
#
#  PURPOSE:
#       To provide a means of hotsparing and rebuilding drives.
#
#  Copyright (c) 1996-2011 Xiotech Corp. All rights reserved.
#
#**********************************************************************
#
# --- global function declarations ------------------------------------
#
        .globl  RB$canspare             # returns true if PDD can be spared
        .globl  RB$faildev              # Fail device MRP
        .globl  RB$failbackdev          # Failback the designated hotspare
                                        # back to its previous data driv
        .globl  RB$autofailback         # Auto failback feature enable / disable
#
        .globl  RB$hotspareinfo         # Hotspare info MRP
        .globl  RB$cancel_rebld         # Cancel rebuilds
        .globl  RB$pause_rebld          # Pause rebuilds
        .globl  RB$resume_rebld         # Resume rebuilds
#
        .globl  RB$rerror               # RAID error handler
        .globl  RB$rerror_que           # RAID error que
        .globl  RB$rerror_comp          # RAID error completion handler
        .globl  rb$rerror_exec          # RAID error exec
#
        .globl  RB_searchforfailedpsds  # Check for needed rebuilds or hotspares
        .globl  RB_setpsdstat           # Update PSD status
        .globl  RB_setraidstat          # Update RAID device status
        .globl  RB_setvirtstat          # Update virtual device status
        .globl  RB_LogHSDepleted        # Log hot spares depleted
#
# --- global data declarations ----------------------------------------
#
        .globl  RB_rerror_qu            # Raid error QCB
        .globl  gRBRHead                # Rebuild list pointer
        .globl  gHotspareWait           # Hotspare Wait time
        .globl  gHotspareWaitPCB        # Hotspare Wait task
        .globl  gPSDRebuilderPCB        # psd_rebuilder  task
#
# --- global usage data definitions -----------------------------------
#
        .section        .shmem
        .align  4
RB_rerror_qu:
        .space  16,0                    # Raid error executive QCB
#
# ----------------------------------------------------------------------------
        .data
        .align  4
gRBRHead:
RB_rbrhead:
        .word   0                       # pointer to head of rebuild list
#
# --- local usage data definitions ------------------------------------
#
        .align  2
#
gHotspareWait:                          # Delay before attempting to hotspare
rb_hotsparewait:                        # Delay before attempting to hotspare
        .word   0
#
gHotspareWaitPCB:
rb_hotsparewaitpcb:
        .word   0
#
# --- Rebuild synchronization control
#
rb_stopcnt:
        .byte   0                       # Stop counter
rb_rlock:
        .byte   FALSE                   # Rebuild lock (T/F)
#
rb_rerrorbusy:
        .word   FALSE                   # RAID error exec busy
#
gPSDRebuilderPCB:
        .word   0                       # psd_rebuilder task
#
# --- executable code (low usage) -------------------------------------
#
        .text
#
#**********************************************************************
#
#  NAME: rb$findcapacity
#
#  PURPOSE:
#       To provide a means of finding the total allocated capacity on
#       the designated device.
#
#  DESCRIPTION:
#       The RDDs with their associated PSDs are searched to determine
#       the capacity in use for the designated device.
#
#  CALLING SEQUENCE:
#       call    rb$findcapacity
#
#  INPUT:
#       g4 = PID
#
#  OUTPUT:
#       g0/g1 = capacity used by all RAIDs on this PID
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
rb$findcapacity:
        PushRegs(r3)                    # Save all the registers
        mov     g4,g0                   # Set input parameter
        call    RB_FindCapacity         # Call the C code
        movl    g0,r8                   # Save the return value
        PopRegsVoid(r3)                 # Restore the registers
        movl    r8,g0                   # Return values
        ret
#
#**********************************************************************
#
#  NAME: rb$findhotspare
#
#  PURPOSE:
#       To provide a means of finding the best hotspare for the
#       given required capacity.
#
#  DESCRIPTION:
#       The RDDs with their associated PSDs are searched
#       to find an inactive hotspare with the least amount
#       of excess capacity.
#
#  CALLING SEQUENCE:
#       call    rb$findhotspare
#
#  INPUT:
#       g0/g1 = required capacity
#       g4 = PID
#
#  OUTPUT:
#       g5 = hotspare PDD if non-zero
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
rb$findhotspare:
        PushRegs(r3)                    # Save all the registers
        mov     g4,g2                   # Set input parameter
        call    RB_FindHotSpare         # Call the C code
        mov     g0,r4
        PopRegsVoid(r3)                 # Restore the registers (g0 has ret val)
        mov     r4,g5                   # Set hot spare PDD
        ret
#
#**********************************************************************
#
#  NAME: RB$faildev
#
#  PURPOSE:
#       To provide a means of processing the fail device MRP (mrfail)
#       issued by the CCB.
#
#  CALLING SEQUENCE:
#       call    RB$faildev
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = status: deok, deinvpid, denohotspare, deinsuffredund,
#               depidnotused or debusy
#       g2 = return pkt size
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
RB$faildev:
#
# --- Setup
#
        mov     g3,r11                  # Save registers
        movl    g4,r14
#
        ld      mr_ptr(g0),g0           # Get parm block pointer
        ldos    mfd_pid(g0),r7          # Get failing PID
        ldos    mfd_hspid(g0),r8        # Get hotspare PID
        ldob    mfd_options(g0),r9      # Get option bits:
                                        # - Fail regardless of redundancy
                                        # - Use given hotspare pid
                                        # - Write the fail label
                                        # - Just spindown
                                        # - Just log a raid error
                                        # - Just check PSD status
#
.if     DEBUG_FLIGHTREC_D
        ldconst frt_h_miscb,r3          # Misc RB$faildev function
        st      r3,fr_parm0             # Function rb$canrebuild
        st      r7,fr_parm1             # Input PID
        st      r8,fr_parm2             # Hotspare PID
        st      r9,fr_parm3             # Options
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_D
#
# --- PDD sanity checks
#
        ldconst deinvpid,g1             # Prep error code
        ldconst MAXDRIVES,r4
        cmpoble r4,r7,.dff1000          # Jif failing PID out of range error
#
        ld      P_pddindx[r7*4],r5      # r5 = failing PDD pointer
        cmpobe  0,r5,.dff1000           # Jif no PDD error
#
        bbs     mfdspindown,r9,.dff110  # Jif just a spindown requested
#
        bbs     mfdchecksys,r9,.dff130  # Jif just a system check
#
        bbc     mfdusehspid,r9,.dff30   # Jif hotspare PID not used - continue
#
        cmpoble r4,r8,.dff1000          # Jif hotspare PID out of range error
#
        ld      P_pddindx[r8*4],g5      # g5 = hotspare PDD pointer
        cmpobe  0,g5,.dff1000           # Jif no PDD error
#
        ldob    pd_class(g5),r3         # Get class
        cmpobne pdhotlab,r3,.dff1000    # Jif not labelled as hotspare
#
# --- Write the fail label
#
.dff30:
        ldconst pdlitefail,r3           # Turn fault light on
        stob    r3,pd_fled(r5)
#
        mov     r5,g3                   # Get the PDD
        call    O$ledchanged            # Change the LED
#
        bbc     mfdfaillabel,r9,.dff40  # Jif fail label not requested
#
        ldob    pd_devstat(r5),r3       # Get current status
        cmpobe  pdinop,r3,.dff40        # Jif already failed
#
        cmpo    pdnonx,r3               # Check if nonexistent
        sele    pdinop,pdnonx,r3        # Either set to inop or nonexistent
        stob    r3,pd_devstat(r5)
#
c fprintf(stderr,"%s%s:%u <PDD-BUSY>RB$faildev-- writing failed label on pid=%d\n", FEBEMESSAGE, __FILE__, __LINE__,((PDD*)r5)->pid);
        lda     O$writefailedlabel,g0   # Fork write fail label process
        ldconst OSPINDOWNPRIO,g1
        ldconst xdfailgen,g2            # Load failure type
        mov     r5,g3                   # Pass PDD
c       CT_fork_tmp = (ulong)"O$writefailedlabel";
        call    K$tfork
#
# --- Determine if failing PID is used in a RAID on any controller
#
.dff40:
        ldconst depidnotused,g1         # Assume PID not used by any RAIDs
        mov     r5,g4                   # Pass PDD
        call    D_convpdd2psd           # Convert PDD to PSD
        cmpobe  0,g0,.dff1000           # Jif PDD not used - error
        mov     g0,r10                  # Save the PSD
#
        bbs     mfdlograiderror,r9,.dff120  # Jif raid error log requested
#
# --- Check for raid error exec in-progress
#
        ldconst debusy,g1               # Assume RAID error exec is busy
        ld      rb_rerrorbusy,r3        # Get raid error task state
        cmpobe  TRUE,r3,.dff1000        # Jif busy
        ld      O_retryhotswap_pcb,r3   # Get retry hotswap PCB
        cmpobne 0,r3,.dff1000           # Jif active or waiting
#
# --- Determine if sufficient redundancy exists on this controller
#
        bbs     mfdforce,r9,.dff50      # Jif we are going to force a rebuild
        ldconst deinsuffredund,g1       # Assume insufficient redundancy
        call    RB$canspare             # Check for sufficient redundancy
                                        #   Pass g0=PSD, returns g0=T/F
        cmpobne deok,g0,.dff1000        # Jif not redundancy error
#
# --- Make sure hotspare exists and has enough capacity
#
.dff50:
        mov     r7,g4                   # Pass failing PID
        call    rb$findcapacity         # Find required capacity - pass g4=PID
                                        #  returns g0/g1 (g1=high order)
        bbs     mfdusehspid,r9,.dff60   # Jif a hotspare passed in MRP
#
        call    rb$findhotspare         # Find a hotspare - pass g0/g1
        cmpobe  0,g5,.dff70             # Jif no hotspares available
        b       .dff80                  # Do the rebuild
#
.dff60:
        ldl     pd_devcap(g5),r12       # Get hotspare capacity: r13=high order
        or      r12,r13,r3              # Check for no capacity
        cmpobe  0,r3,.dff70             # Jif null - error
#
        cmpobl  r13,g1,.dff70           # Jif hotspare too small - error
        cmpobg  r13,g1,.dff80           # Jif high order bigger - rebuild
        cmpobge r12,g0,.dff80           # Jif low order bigger - rebuild
.dff70:
        ldconst denohotspare,g1         # Assume no hotspare available
        b       .dff1000
#
# --- Rebuild it
#
.dff80:
        mov     r10,g0                  # Pass failing PSD
        call    rb$redirectpsd          # Rebuild it, pass g0/g5 = PSD/PDD
        ldconst ecok,g1                 # Indicate OK
#
.dff100:
        ldob    pd_devstat(r5),r3       # Get the device status
        cmpo    pdnonx,r3               # Check if nonexistent
        sele    pdinop,pdnonx,r3        # Either set to inop or nonexistent
        stob    r3,pd_devstat(r5)
#
        ldob    pd_flags(r5),r3         # get pd flags
        setbit  pduserfailed,r3,r3      # set user failed pdisk bit
        stob    r3,pd_flags(r5)         # store in pd flags
        call  D$p2updateconfig
#
        ldconst ecok,g1                 # Indicate OK
        b       .dff1000
#
# --- Just a spindown
#
.dff110:
        mov     r5,g3                   # Pass PDD
        call    O$spindowndrive         # Spindown
                                        # This will NOT wait for the stop unit
                                        #   command to complete
        ldconst ecok,g1                 # Indicate OK
        b       .dff1000
#
# --- Just log a RAID error
#
.dff120:
        mov     r10,g0                  # Pass failing PSD
        ldconst mledebug,g1             # Pass log type - for debug
        call    rb$log_raid_error       # This will cause the CCB to rebuild it
        b       .dff100                 # Also mark PDD inoperative then exit
#
# --- Just check the system and handle any hotswaps or rebuilds
#
.dff130:
        call    RB_searchforfailedpsds
        ldconst ecok,g1                 # Indicate OK
        b       .dff1000
#
# --- Exit
#
.dff1000:
        ldconst mfdrsiz,g2              # Set return packet size
        movl    r14,g4                  # Restore registers
        mov     r11,g3
        ret
#
#**********************************************************************
#
#  NAME: RB$autofailback
#
#  PURPOSE:
#       To provide a means of processing the auto failback enable / disable
#       MRP (mrautofailbackenabledisable)
#       issued by the CCB.
#
#  CALLING SEQUENCE:
#       call    RB$autofailback
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = status: deok, deinvpid, denohotspare, deinsuffredund,
#               depidnotused or debusy
#       g2 = return pkt size
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
RB$autofailback:
        PushRegs(r3)                    # Save registers
        call    RB_AutoFailBackEnableDisable
        mov     g0,r8
        PopRegsVoid(r3)
        mov     r8,g1                   # Set return status
        ldconst mafbrsiz,g2             # Set return packet size
        ret
#
#**********************************************************************
#
#  NAME: RB$failbackdev
#
#  PURPOSE:
#       To provide a means of processing the failback device MRP)
#       issued by the CCB.
#
#  CALLING SEQUENCE:
#       call    RB$failbackdev
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = status: deok, deinvpid, denohotspare, deinsuffredund,
#               depidnotused or debusy
#       g2 = return pkt size
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
RB$failbackdev:
        PushRegs(r3)                    # Save registers
#
        ld      mr_ptr(g0),g0           # Get parm block pointer
        ldos    mfbd_hspid(g0),r8       # Get hotspare PID
        ldob    mfbd_options(g0),r9     # Get option bits:
                                        #  Bit -1 - Cancel the unfailing of specified HS(auto/manual)
                                        #  other bits -- unused (currently)
        mov r8,g0
        mov r9,g1
        call    RB_pdiskFailBack
        mov g0,r8
        PopRegsVoid(r3)
        mov r8,g1                       # Set return status
        ldconst mfbdrsiz,g2             # Set return packet size
        ret
#
#**********************************************************************
#
#  NAME: RB$hotspareinfo
#
#  PURPOSE:
#       To implement the hotspare MRP. The input is a PID. The output
#       is a flag indicating if the PID can be rebuilt and if so, the
#       capacity required.
#
#  CALLING SEQUENCE:
#       call    RB$hotspareinfo
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = status: deok, deinvpid, depidnotused, deinsuffredund,
#                    derebuildnotreq, or debusy
#       g2 = return pkt size
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
RB$hotspareinfo:
        movq    g0,r12                  # Save regs
        mov     g4,r11
#
        ld      mr_rptr(g0),r9          # Get return data pointer
        ld      mr_ptr(g0),g0           # Parm block address within MRP
        ldos    mrh_pid(g0),r10         # r10 = PID
#
# --- Sanity checks
#
        ldconst deinvpid,r7             # Assume invalid PID
        ldconst MAXDRIVES,r4
        cmpoble r4,r10,.dhi100          # Jif PID out of range - error
#
        ld      P_pddindx[r10*4],r8     # r8 = PDD
        cmpobe  0,r8,.dhi100            # Jif no PDD - error
#
# --- Check for raid error exec in-progress
#
        ldconst debusy,r7               # Assume RAID error exec is busy
        ld      rb_rerrorbusy,r3        # Get raid error task state
        cmpobe  TRUE,r3,.dhi100         # Jif busy
#
# --- Return capacity
#
        mov     r10,g4                  # Pass PID
        call    rb$findcapacity         # Get required capacity in g0/g1
        stl     g0,mrh_cap(r9)          # Save required capacity
#
# --- Check if PID used, rebuild required, and sufficient redundancy
#
        ldconst depidnotused,r7         # Assume PID not used
        mov     r8,g4                   # Pass failing PDD
        call    D_convpdd2psd           # PID used by RAIDs on any controller?
                                        #   output g0 is 0 or PSD
        cmpobe  0,g0,.dhi100            # Jif PID not used - return
#
        ldconst deinsuffredund,r7       # Assume can't spare
        call    RB$canspare             # Sufficient redundancy to spare PSD?
                                        #   input g0 is PSD
        cmpobne deok,g0,.dhi100         # Exit if can't rebuild all using RAIDs
#
        ldconst derebuildnotreq,r7      # Assume rebuild not required
        mov     r8,g3                   # Pass failing PDD
# jlw        call    d$inopchk               # Check for any inoperative PSDs
# jlw                                        #  Returns g2=T/F
# jlw        cmpobe  TRUE,g2,.dhi50          # Jif PDD needs to be spared
        call    RB_searchforfailedpsds  # Make sure we're rebuilding it in case
                                        #   RM tosses the hotspare record
        b       .dhi100                 # Return
#
# --- Exit
#
.dhi100:
        movq    r12,g0                  # restore regs
        mov     r11,g4
#
        mov     r7,g1                   # Set return code
        ldconst mrhrsiz,g2              # Set return packet size
        ret
#
#**********************************************************************
#
#  NAME: RB_setpsdstat
#
#  PURPOSE:
#       To provide a common means of updating the status and additional
#       status fields of the PSDs and RDDs.
#
#  DESCRIPTION:
#       This is the main state machine for the PSD status.
#       This will check each RAID in the system.
#
#  CALLING SEQUENCE:
#       call    RB_setpsdstat
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
RB_setpsdstat:
        mov     g1,r15                  # Save registers
#
# --- Check each RAID in the system
#
        lda     R_rddindx,r11           # Prepare search
        ldconst MAXRAIDS-1,r6           # Load count
        lda     P_pddindx,r14           # Get the PDD pointer
#
.ors10:
        ld      rx_rdd(r11)[r6*4],g1    # g1 = RDD
        cmpobe  0,g1,.ors70             # Jif undefined - get next RDD
#
# --- If this is a vlink, do not update the status field.  They are always
# --- considered operational.
#

        ldob    rd_type(g1),r4          # Get the type
        cmpobe  rdlinkdev,r4,.ors70     # Jif linked device
#
# --- also skip snapshot devices
        cmpobe  rdslinkdev,r4,.ors70    # Jif snapshot device
#
# --- Set sync mode if the RDD is in the error state and clear any rebuilding
#     (Rebuilding and parity out of sync will cause data corruption.  Require
#     a resync which will cause the RAID to go inoperative until the original
#     drive can be reinserted).
#     These will only apply to RAID 5.
#     We can get here if a power cycle happens while the PSD is in the
#     error state.
#
# --- Update PSD status
#     Walk the PSD linked list
#     Make the new status match the PDD status
#
        ldos    rd_psdcnt(g1),r7        # Get PSD count
        ld      rd_psd(g1),r13          # Get 1st PSD
.ors30:
#
# Make PSD status = PDD status
#
        ldos    ps_pid(r13),r3
        ld      px_pdd(r14)[r3*4],r3    # This is the PDD pointer
c   if (r3 == 0) {                      # Ignore if PDD is a null pointer.
        ldconst pdnonx,r5               # Else change to non-existant
        b       .ors31                  # Next PSD
c   }
        ldob    pd_devstat(r3),r5       # r5 = PDD dev_stat
#
# --- Check if we are still spinning up the drive.  If so, set the type to
# --- nonexistent to prevent it from thinking the drive is inoperable and
# --- really broken.
#
        cmpobne pdinop,r5,.ors31        # Jump if not inoperable
#
        ldob    pd_flags(r3),r4
        bbs     pdbebusy,r4,.ors60
        ldob    pd_poststat(r3),r4      # Get the post stat
        ldconst pdaspin,r3              # Check for spinning up/down
        cmpobne r3,r4,.ors31            # If not spin up/down use current status
#
        ldconst pdnonx,r5               # Else change to non-existant
#

.ors31:
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        stob    r5,ps_status(r13)       # Make this the PSD status also
#
.if     DEBUG_FLIGHTREC_OHSPARE
        shlo    8,r5,r3                 # PDD stat<<8
        ldob    ps_astatus(r13),r4      # Get additional status
        or      r3,r4,r3                # Or it with the dev status
        st      r3,fr_parm3             # PDD stat / PSD astatus
        ldconst frt_h_misc6,r4          # Misc function, RB_setpsdstat
        st      r4,fr_parm0             # Function
        ldos    ps_rid(r13),r3          # RID
        ldos    ps_pid(r13),r4          # PID
        shlo    16,r3,r3
        or      r3,r4,r3
        st      r3,fr_parm1             # RID/PID
        st      r13,fr_parm2            # PSD
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_OHSPARE
#
        ldob    ps_astatus(r13),r4      # Get additional status
#
# Handle PSDs in the error state
#
        bbc     psaerror,r4,.ors35      # Jif not in the error state
        ldconst pserror,r3
        stob    r3,ps_status(r13)       # Change to error
        b       .ors60                  # Next PSD
#
# Handle PSDs that were being rebuilt
#
.ors35:
        bbc     psarebuild,r4,.ors40    # Jif rebuild not scheduled
        ldconst psrebuild,r3
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        stob    r3,ps_status(r13)       # Change to rebuild required
#
.ors40:
        cmpobne psop,r5,.ors60          # Jif not operational
#
# Handle operational PSDs that were being initialized
#
        bbc     psauninit,r4,.ors60     # Jif initialize not required
        ldconst psuninit,r3
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        stob    r3,ps_status(r13)       # Change operable to uninitialized
#
# Do next PSD
#
.ors60:
        subo    1,r7,r7                 # Decrement PSD loop counter
        ld      ps_npsd(r13),r13        # Get next PSD
        cmpobne 0,r7,.ors30             # Jif more
#
# --- Advance to next RAID device
#
        call    RB_setraidstat          # Update RAID status, input g1 = RDD
#
.ors70:
        subo    1,r6,r6                 # Bump to next device
        cmpible 0,r6,.ors10             # Jif more
#
# --- Exit
#
        mov     r15,g1                  # restore registers
        ret
#
#**********************************************************************
#
#  NAME: RB_setraidstat
#
#  PURPOSE:
#       To provide a common means of updating the status field within the
#       RDD structure.
#
#  DESCRIPTION:
#       The PSDs are examined for the given RDD and a determination is
#       made as to whether or not the array is operational.  Based upon
#       this determination the RDD is updated accordingly.
#
#  CALLING SEQUENCE:
#       call    RB_setraidstat
#
#  INPUT:
#       g1 = RDD
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
RB_setraidstat:
#
# Common checking for initializing RAID.
#
        ldob    rd_astatus(g1),r3       # Get additional status
        bbc     rdauninit,r3,.ss01      # Jif raid is initialized
#
        ldob    rd_status(g1),r3        # Get present raid status
        cmpobe  rduninit,r3,.ss1000     # Jif ready for init but not started
        cmpobe  rdinit,r3,.ss1000       # Jif already getting inited
        b       .ss920                  # Set status to uninitialized
#
# --- For Slink Device (Snapshot)
#
# --- Check if this is a snapshot raid (raid type 6), build the operational
# --- state from PSD.
#
.ss01:
        ldob     rd_type(g1),r4          # Get RAID type
        cmpobne  rdslinkdev,r4,.ss00     # Jif not slinkdev
        ld       rd_psd(g1),r10          # Get 1st PSD
        ldob     ps_status(r10),r5       # Get PSD status
        cmpobne  psop,r5,.ss950          # Jif not operative
        b        .ss930                  # Done jmp to set op state
#
# --- STD or RAID 0 ---------------------------------------------------
#
# --- Check for and process STD or RAID-0 array.  If any device in the
# --- RAID is non-operational, then the entire device is non-operational
# --- since these two RAID types have no redundancy.
#

.ss00:
        ldob    rd_type(g1),r4          # Get RAID type
        cmpobe  rdstd,r4,.ss10          # Jif STD
        cmpobne rdraid0,r4,.ss100_r     # Jif not RAID 0
#
.ss10:
        ld      rd_psd(g1),r10          # Get 1st PSD
        mov     r10,r11
#
.ss20:
        ldob    ps_status(r10),r5       # Get PSD status
        cmpobne psop,r5,.ss950          # Jif not operative
#
        ld      ps_npsd(r10),r10        # Link to next PSD
        cmpobne r10,r11,.ss20           # Jif more
#
# --- All devices in the RAID were operational, so set the
# --- status to operational.
#
        b       .ss930
#
# --- RAID 1 ----------------------------------------------------------
#
# --- Check for and process RAID-1 array.  For this RAID type, if any
# --- device is operational, then the RAID can operate.  If all devices
# --- are non-operational, then the RAID is non-operational.  If there
# --- is a mix, then the RAID is degraded.
#
.ss100_r:
        cmpobne rdraid1,r4,.ss200       # Jif not RAID 1
#
        ldconst FALSE,r6                # Were there any non-operational devices
        ldconst FALSE,r7                # Were there any operational devices
#
        ld      rd_psd(g1),r10          # Get 1st PSD
        mov     r10,r11
#
.ss110:
        ldob    ps_status(r10),r5       # Get PSD status
        cmpobne psop,r5,.ss120          # Jif not operative
#
# --- An operable device was found, set the operable indicator.
#
        ldconst TRUE,r7                 # There were operational devices
        b       .ss130
#
.ss120:
        ldconst TRUE,r6                 # There were non-operational devices
#
.ss130:
        ld      ps_npsd(r10),r10        # Link to next PSD
        cmpobne r10,r11,.ss110          # Jif more
#
# --- Now determine the status based upon the operational and non-operational
# --- indicators.  If all are operational, then operational.  If all non-op,
# --- then non-op.  If a mix, then degraded.
#
        cmpobne TRUE,r7,.ss950          # Jif no operational devices
        cmpobne TRUE,r6,.ss930          # Jif no non-operational devices
        b       .ss900                  # Jif a mix
#
# --- RAID 5 ----------------------------------------------------------
#
# --- Check for and process RAID-5 array.  The checking here will be to
# --- look at each subset of "depth" number of drives.  If more than one
# --- of them is inoperable, then the RAID is inoperable.  This checking
# --- is done psdcnt times in order to pick up all rotations of the depth
# --- size.  If any device in the list is found to be inoperable but the
# --- RAID status is still operable, then the RAID is degraded.
# --- If any PSD is scheduled for initialization then the RAID will be
# --- uninitialized.
#
.ss200:
        cmpobne rdraid5,r4,.ss300       # Jif not RAID 5
#
        ld      rd_psd(g1),r3           # Get 1st PSD
        ldos    rd_psdcnt(g1),r15       # Get number of PSDs
        ldob    rd_depth(g1),r14        # Get algorithm (3, 5 or 9)
        mulo    r14,r15,r15             # Form PSD scan iterations
        ldconst FALSE,r6                # Any failure
#
.ss220:
        mov     r14,r4                  # Get algorithm
        mov     FALSE,r5                # Clear inoperative/rebuild found
#
.ss230:
#
        ldob    ps_status(r3),r7        # Get PSD status
        cmpobe  psop,r7,.ss240          # Jif operative
#
        cmpobe  TRUE,r5,.ss950          # Jif double failure found - inoperable
#
        mov     TRUE,r5                 # Set 1st failure found
        mov     TRUE,r6                 # Set any failure found
#
.ss240:
        ld      ps_npsd(r3),r3          # Link to next PSD
        subo    1,r15,r15               # Adjust scan iteration count
        cmpobe  0,r15,.ss250            # Jif done
#
        subo    1,r4,r4                 # Adjust remaining PSDs for algorithm
        cmpobne 0,r4,.ss230             # Jif more
#
        b       .ss220
#
# --- All done checking so set status fields.
# --- Check if there were any single failures in order to set degraded.
#
.ss250:
        cmpobe  FALSE,r6,.ss930         # Jif all PSDs operable
#
        ldob    rd_astatus(g1),r3       # Get additional status
        bbs     rdaparity,r3,.ss950     # Can't operate if we need to sync
        b       .ss900                  # Degraded
#
# --- RAID 10 ---------------------------------------------------------
#
# --- Check for and process RAID-10 array.  The check is very similar to
# --- the RAID 5 check except that there is more tolerance for failed
# --- devices.  Only if ALL the devices in a "depth" set is inoperable, is
# --- the RAID inoperable.
#
.ss300:
        cmpobne rdraid10,r4,.ss930      # Jif not RAID 10
#
        ld      rd_psd(g1),r3           # Get 1st PSD
        ldos    rd_psdcnt(g1),r15       # Get number of PSDs
        ldob    rd_depth(g1),r14        # Get mirror depth
        mulo    r14,r15,r15             # Form PSD scan iterations
        mov     FALSE,r6                # Clear inoperative found
#
.ss310:
        mov     r14,r4                  # Get mirror depth
        mov     FALSE,r5                # Clear operative found
#
.ss320:
        ldob    ps_status(r3),r7        # Get PSD status
        cmpobne psop,r7,.ss330          # Jif not operative
#
        mov     TRUE,r5                 # Set operative found
        b       .ss340
#
.ss330:
        mov     TRUE,r6                 # Set inoperable found
#
.ss340:
        ld      ps_npsd(r3),r3          # Link to next PSD
        subo    1,r15,r15               # Adjust scan iteration count
#
        subo    1,r4,r4                 # Adjust remaining PSDs for mirror
        cmpobne 0,r4,.ss320             # Jif more mirrors
#
        cmpobe  FALSE,r5,.ss950         # Jif operative mirror not found
        cmpobne 0,r15,.ss310            # Jif not done
#
# --- The RAID is operable.
# --- Check if any device was inoperable.  If so, then we are degraded.
#
        cmpobe  FALSE,r6,.ss930         # Jif all devices operable
#
# --- Common ----------------------------------------------------------
#
#
# --- Set RDD status as degraded
#
.ss900:
        ldconst rddegraded,r3           # Set RDD status degraded
        b       .ss990
#
# --- Set RDD status as uninitialized
#
.ss920:
        ldconst rduninit,r3             # Set RDD status uninitialized
        b       .ss990
#
# --- Set RDD status as operational
#
.ss930:
        ldconst rdop,r3                 # Set RDD status operative
        b       .ss990
#
# --- Set RDD status as inoperative
#
.ss950:
        ldconst rdinop,r3               # Set RDD status inoperative
#
.ss990:
        ldob    rd_status(g1),r6        # Get the current RAID Status
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        stob    r3,rd_status(g1)        # Set the new status

.ifndef MODEL_3000
.ifndef  MODEL_7400
#
# --- Log an error message if a non-raid 5 (raid 5 has its own message)
# --- goes inoperative #ATS-131 skip the log messages if the ISE is in
#     Busy state (PAB/ISE upgrade)
#
        ldob    rd_type(g1),r5          # Get raid type
        cmpobe  rdraid5,r5,.ss995       # Jif raid5
        ldos    rd_vid(g1),r4           # Get the VID
        ld      V_vddindx[r4*4],r5      # Get VDD
        cmpobe  0,r5,.ss1001            # Jif undefined
        ldob    vd_status(r5),r5        # Get the status
        cmpobe  vdisebusy,r5, .ss994    # No log message if ISE(Vdisk) is in busy state
        cmpobe  r3,r6,.ss995            # Jif no change in status
        cmpobne rdinop,r3,.ss995        # Jif new stat is not Raid Inoperative
        mov     g0,r5                   # Save g0
        mov     g1,g0                   # Input parameter = RDD
        call    r$log_Raidinop          # Log the failure
        mov     r5,g0                   # Restore g0
        b       .ss995                  # Branch

.ss994:
c fprintf(stderr, "%s%s:%u rebld.as:%u ISEBUSY-DEBUG: VDisk 0x%x RAID 0x%x Inoperative\n", FEBEMESSAGE, __FILE__, __LINE__, __LINE__, ((RDD*)g1)->rid, ((RDD*)g1)->vid);

.ss995:
.endif  # MODEL_7400
.endif  # MODEL_3000

#
#
# --- Kick off a Parity Scan if the RDD is marked and something has changed so
#       that the scan can complete
#
        ldob    rd_astatus(g1),r5       # Get the RAID AStatus byte
        bbc     rdaparity,r5,.ss1000    # Jif no Parity Resync is needed
        ld      R_pcctrl,r4             # Get parity checker control word
        setbit  rdpcnewcmd,r4,r4        # Indicate a new parity command
        setbit  rdpcmarked,r4,r4        # Indicate there is a marked RDD
        clrbit  rdpcspecific,r4,r4      # Don't do a specific RID
        setbit  rdpccorrect,r4,r4       # Correct any out of sync parity stripes
        setbit  rdpc1pass,r4,r4         # Just do 1 pass
        setbit  rdpcenable,r4,r4        # Enable parity checking
        st      r4,R_pcctrl
        cmpobe  r3,r6,.ss1000           # Jif no change in status (limits number
                                        #  of messages reported to CCB)
        cmpobne rdinop,r3,.ss1000       # Jif new stat is not RAID Inoperative
        mov     g0,r3                   # Save g0
        mov     g1,g0                   # Set up the RDD as the input Parameter
        call    R$log_R5inop            # Log the failure
        mov     r3,g0                   # Restore g0
#
# --- Exit
#
.ss1000:
        ldos    rd_vid(g1),r3
        ld      V_vddindx[r3*4],r4      # Get VDD
        cmpobe  0,r4,.ss1001            # Jif NULL
        PushRegs(r3)
c       GR_UpdateVdiskOpState((VDD*)r4,1,0);
        PopRegsVoid(r3)
.ss1001:
#
.if     DEBUG_FLIGHTREC_OHSPARE
        ldob    rd_status(g1),r3        # Get additional status
        ldob    rd_astatus(g1),r4       # Get additional status
        shlo    24,r3,r3                # rd_status<<24
        shlo    16,r4,r4                # rd_status<<26
        or      r3,r4,r3                #
        st      r3,fr_parm3             # RDD status/astatus << 16
        ldconst frt_h_misc5,r3          # Misc function RB_setraidstat
        st      r3,fr_parm0             # Function
        ld      rd_rid(g1),r3
        shlo    16,r3,r3
        st      r3,fr_parm1             # RID
        st      g1,fr_parm2             # RDD being checked
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_OHSPARE
        ret
#
#**********************************************************************
#
#  NAME: rb$getraiderrorstat
#
#  PURPOSE:
#       To provide a common means of updating the status field within the
#       RDD structure.
#
#  DESCRIPTION:
#       The PSDs are examined for the given RDD and a determination is
#       made as to whether or not the array is operational.
#
#  CALLING SEQUENCE:
#       call    rb$getraiderrorstat
#
#  INPUT:
#       g1 = RDD
#
#  OUTPUT:
#       g0 = rd_status: nonx, inop, op, degraded, etc
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
rb$getraiderrorstat:
#
# --- STD or RAID 0 ---------------------------------------------------
#
# --- Check for and process STD or RAID-0 array.  If any device in the
# --- RAID is non-operational, then the entire device is non-operational
# --- since these two RAID types have no redundancy.
#
        ldob    rd_type(g1),r4          # Get RAID type
        cmpobe  rdstd,r4,.oes10         # Jif STD
        cmpobne rdraid0,r4,.oes100      # Jif not RAID 0
#
.oes10:
        ld      rd_psd(g1),r10          # Get 1st PSD
        mov     r10,r11
#
.oes20:
        ldob    ps_status(r10),r5       # Get current PSD status
        cmpobne psop,r5,.oes950         # Jif PSD not operative - inoperative
#
        ldob    ps_astatus(r10),r5      # Get PSD status when error occurred
        bbs     psaerror,r5,.oes950     # Jif an error had existed - inoperative
#
        ld      ps_npsd(r10),r10        # Link to next PSD
        cmpobne r10,r11,.oes20          # Jif more
#
# --- All devices in the RAID were operational, so set the
# --- status to operational.
#
        b       .oes930                 # Operational
#
# --- RAID 1 ----------------------------------------------------------
#
# --- Check for and process RAID-1 array.  For this RAID type, if any
# --- device is operational, then the RAID can operate.  If all devices
# --- are non-operational, then the RAID is non-operational.  If there
# --- is a mix, then the RAID is degraded.
#
.oes100:
        cmpobne rdraid1,r4,.oes200      # Jif not RAID 1
#
        ldconst FALSE,r6                # Were there any non-operational devices
        ldconst FALSE,r7                # Were there any operational devices
#
        ld      rd_psd(g1),r3           # Get 1st PSD
        mov     r3,r4
#
.oes110:
        ldob    ps_status(r3),r5        # Get PSD status
        cmpobne psop,r5,.oes120         # Jif not operative
#
        ldob    ps_astatus(r3),r5       # Get PSD status when error occurred
        bbs     psaerror,r5,.oes120     # Jif an error had existed - inoperative
#
# --- An operable device was found, set the operable indicator.
#
        ldconst TRUE,r7                 # There were operational devices
        b       .oes130
#
.oes120:
        ldconst TRUE,r6                 # There were non-operational devices
#
.oes130:
        ld      ps_npsd(r3),r3          # Link to next PSD
        cmpobne r3,r4,.oes110           # Jif more
#
# --- Now determine the status based upon the operational and non-operational
# --- indicators.  If all are operational, then operational.  If all non-op,
# --- then non-op.  If a mix, then degraded.
#
        cmpobne TRUE,r7,.oes950         # Jif no operational devices
        cmpobne TRUE,r6,.oes930         # Jif no non-operational devices
        b       .oes900                 # Jif a mix - degraded
#
# --- RAID 5 ----------------------------------------------------------
#
# --- Check for and process RAID-5 array.  The checking here will be to
# --- look at each subset of "depth" number of drives.  If more than one
# --- of them is inoperable, then the RAID is inoperable.  This checking
# --- is done psdcnt times in order to pick up all rotations of the depth
# --- size.  If any device in the list is found to be inoperable but the
# --- RAID status is still operable, then the RAID is degraded.
# --- If any PSD is scheduled for initialization then the RAID will be
# --- uninitialized.
#
.oes200:
        cmpobne rdraid5,r4,.oes300      # Jif not RAID 5
#
        ldob    rd_astatus(g1),r3       # Get additional status
        bbc     rdauninit,r3,.oes205    # Jif raid is initialized
        b       .oes920                 # Set status to uninitialized
#
.oes205:
        ld      rd_psd(g1),r3           # Get 1st PSD
        ldos    rd_psdcnt(g1),r15       # Get number of PSDs
        ldob    rd_depth(g1),r14        # Get algorithm (3, 5 or 9)
        mulo    r14,r15,r15             # Form PSD scan iterations
        ldconst FALSE,r6                # Any failure
#
.oes210:
        mov     r14,r4                  # Get algorithm (depth)
        mov     FALSE,r5                # Clear inoperative/rebuild found
#
.oes220:
        ldob    ps_status(r3),r7        # Get PSD status
        cmpobne psop,r7,.oes225         # Jif not operative
#
        ldob    ps_astatus(r3),r7       # Get PSD status when error occurred
        bbc     psaerror,r7,.oes230     # Jif an error did not exist - operative
#
.oes225:
        cmpobe  TRUE,r5,.oes950         # Jif double failure found - inoperable
#
        mov     TRUE,r5                 # Set 1st failure found
        mov     TRUE,r6                 # Set any failure found
#
.oes230:
        ld      ps_npsd(r3),r3          # Link to next PSD
        subo    1,r15,r15               # Adjust scan iteration count
        cmpobe  0,r15,.oes250           # Jif done
#
        subo    1,r4,r4                 # Adjust remaining PSDs for algorithm
        cmpobne 0,r4,.oes220            # Jif more
#
        b       .oes210
#
# --- All done checking so set status fields.
# --- Check if there were any single failures in order to set degraded.
#
.oes250:
        cmpobe  FALSE,r6,.oes930        # Jif all PSDs operable
#
        ldob    rd_astatus(g1),r3       # Get additional status
        bbs     rdaparity,r3,.oes950    # Can't operate if we need to sync
        b       .oes900                 # Degraded
#
# --- RAID 10 ---------------------------------------------------------
#
# --- Check for and process RAID-10 array.  The check is very similar to
# --- the RAID 5 check except that there is more tolerance for failed
# --- devices. Only if ALL the devices in a "depth" set is inoperable, is
# --- the RAID inoperable.
#
.oes300:
        cmpobne rdraid10,r4,.oes930     # Jif not R10 - assume others operable
#
        ld      rd_psd(g1),r3           # Get 1st PSD
        ldos    rd_psdcnt(g1),r15       # Get number of PSDs
        ldob    rd_depth(g1),r14        # Get mirror depth
        mulo    r14,r15,r15             # Form PSD scan iterations
        mov     FALSE,r6                # Clear inoperative found
#
.oes310:
        mov     r14,r4                  # Get mirror depth
        mov     FALSE,r5                # Clear operative found
#
.oes320:
        ldob    ps_status(r3),r7        # Get PSD status
        cmpobe  psnonx,r7,.oes330       # Jif not operative - init's are OK
        cmpobe  psinop,r7,.oes330
        cmpobe  pserror,r7,.oes330
        cmpobe  psrebuild,r7,.oes330
        ldob    ps_astatus(r3),r7       # Get PSD status when error occurred
        bbs     psaerror,r7,.oes330     # Jif an error did exist - inoperative
#
        mov     TRUE,r5                 # Set operative PSD found
        b       .oes340
#
.oes330:
        mov     TRUE,r6                 # Set inoperable PSD found
#
.oes340:
        ld      ps_npsd(r3),r3          # Link to next PSD
        subo    1,r15,r15               # Adjust scan iteration count
#
        subo    1,r4,r4                 # Adjust remaining PSDs for mirror
        cmpobne 0,r4,.oes320            # Jif more mirrors
#
        cmpobe  FALSE,r5,.oes950        # Jif operative mirror not found
        cmpobne 0,r15,.oes310           # Jif not done
#
# --- The RAID is operable.  Check if any device was inoperable.  If so,
# --- then we are degraded.
#
        cmpobe  FALSE,r6,.oes930        # Jif all devices operable
#
# --- Common ----------------------------------------------------------
#
#
# --- Set RDD status as degraded
#
.oes900:
        ldconst rddegraded,g0           # Set RDD status degraded
        b       .oes990
#
# --- Set RDD status as uninitialized
#
.oes920:
        ldconst rduninit,g0             # Set RDD status uninitialized
        b       .oes990
#
# --- Set RDD status as operational
#
.oes930:
        ldconst rdop,g0                 # Set RDD status operative
        b       .oes990
#
# --- Set RDD status as inoperative
#
.oes950:
        ldconst rdinop,g0               # Set RDD status inoperative
#
.oes990:
#
# --- Exit ------------------------------------------------------------
#
.if     DEBUG_FLIGHTREC_OHSPARE
        ldconst frt_h_misc7,r3          # Misc function o$setraiderrorstat
        st      r3,fr_parm0             # Function
        ldos    rd_rid(g1),r3
        shlo    16,r3,r3
        st      r3,fr_parm1             # RID being checked
        st      g0,fr_parm2             # Return Status
        ldob    rd_status(g1),r3
        ldob    rd_astatus(g1),r4
        shlo    24,r3,r3
        shlo    16,r4,r4
        or      r3,r4,r3
        st      r3,fr_parm3             # RDD Status/astatus<<16
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_OHSPARE
        ret
#
#**********************************************************************
#
#  NAME: RB_setvirtstat
#
#  PURPOSE:
#       To provide a common means of updating the status field within
#       VDD structures. This only applies to VDisks using RAID 0.
#
#  DESCRIPTION:
#       The RDDs are examined for all VDD's and a determination is
#       made as to whether or not the virtual disk is operational.
#       Based upon this determination the VDD's are updated accordingly.
#
#       This could apply to all RAID types but is limited to RAID 0.
#       The precedence for status is uninitialized, inoperable, degraded,
#       then operable.
#
#  CALLING SEQUENCE:
#       call    RB_setvirtstat
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
# C access
# void RB_SetVirtStat(void)
        .globl  RB_SetVirtStat
RB_SetVirtStat:
#
RB_setvirtstat:
#
# --- Check each VDD.
#
        ldconst MAXVIRTUALS-1,r15       # VID counter (r14 will be VDD pointer)
#
.sv10:
        ld      V_vddindx[r15*4],r14    # Get VDD
        cmpobe  0,r14,.sv100            # Jif NULL
.ifndef MODEL_3000
.ifndef  MODEL_7400
        ldos    vd_attr(r14),r4         # Get the attributes
        bbs     vdbebusy,r4,.sv100      # Jif VID marked as BUSY
.endif  # MODEL_7400
.endif  # MODEL_3000
#
# --- Update vdisk opstate for all the vdisks in the system. For GEO
# --- and non-GEO
#
#        c   GR_UpdateVdiskOpState((VDD*)r14,1,0);
#
# --- There is a VDD here.  Check the status of the RAIDs in the VDD.
#
        ldconst vdop,r13                # Assume operable
#
        ld      vd_rdd(r14),r12         # Get first RDD pointer
.sv40:
        cmpobe  0,r12,.sv90             # All RAIDs checked, set VDD status
        ldob    rd_type(r12),r4         # Get RAID type
#
# --- Don't set the states of vdds that are in io suspended state
#
        ldob    vd_status(r14), r11     # get vdd status
        cmpobne ssiosuspend,r11, .sv41  # Don't update if the state is suspend
        ldconst ssiosuspend,r13         # Put iosuspend state
#
# --- Don't undo our 'failed' snapshots
#
.sv41:
.ifndef MODEL_3000 #ISE_BUSY
.ifndef MODEL_7400 #ISE_BUSY
        cmpobne vdisebusy,r11,.sv42
        ldconst vdisebusy,r13
# Don't update vdisk status if it is already set to special state
#        b .sv45
        b .sv80                         # ignore snapshot check
                                        # Since we don't handle
                                        # ISE busy and DCN failure
                                        # at the same time..
.sv42:
.endif  # MODEL_7400
.endif  # MODEL_3000
        cmpobne rdslinkdev,r4,.sv45     # snapshot shouldn't be overridden
        ldob    vd_status(r14), r11     # get vdd status
        cmpobne ssiosuspend,r11, .sv46  # Don't update if the state is suspend
        ldconst ssiosuspend,r13         # Put iosuspend state
        b  .sv45
#
.sv46:
        ldob    rd_status(r12),r11
        ldob    rd_rid(r12),r3
        cmpobne rdinop,r11,.sv45
        ldconst vdinop,r13
#
.sv45:
        cmpobne rdraid0,r4,.sv80        # Jif NOT a RAID 0 - check next RDD
#
# --- Determine new status
#
        ldob    rd_status(r12),r11      # Get the status
        cmpobe  rdinit,r11,.sv55        # Jif initializing
        cmpobne rduninit,r11,.sv60      # Jif not uninitialized
#
        ldconst vduninit,r13            # Set to uninitialized
        b       .sv80                   # Check next RDD
#
.sv55:
        ldconst vdinit,r13              # Set to scheduled for initialization
        b       .sv80
#
.sv60:
        cmpobne rdinop,r11,.sv70        # Jif not inoperable
        cmpobe  vduninit,r13,.sv80      # If already uninit, then continue
#
        ldconst vdinop,r13              # Set to inoperable
        b       .sv80                   # Check next RDD
#
.sv70:
        cmpobne rddegraded,r11,.sv80    # Jif not degraded - check next RDD
        cmpobne vdop,r13,.sv80          # Jif never changed from inop
#
        ldconst vddegraded,r13          # Set to degraded
#
# --- Check next RDD in the VDD
#
.sv80:
        ld      rd_nvrdd(r12),r12       # Get the next RAID
        b       .sv40                   # Check it
#
# --- Done with the VDD - set status and go to next VDD
#
.sv90:
        stob    r13,vd_status(r14)      # Set new status
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
#
.sv100:
        subo    1,r15,r15               # Decrement VDD index
        cmpible 0,r15,.sv10             # Jif more VDDs to do
        ret
#
#**********************************************************************
#
#  NAME: RB$rerror
#
#  PURPOSE:
#       This function and rb$rerror_exec work together to determine if a
#       failing physical operation should result in a rebuild of the
#       physical device or if the RAID has gone inoperable and needs
#       to have a parity scan.
#
#       This function should be called when an error is detected that may
#       require a rebuild. At the completion of the op from the RAID layer,
#       if the RDD is still in the error state then that ILT/RRP should be
#       queued to rb$rerror_exec with RB$rerror_que.
#
#  DESCRIPTION:
#       Given a failing PSD and RDD, this function puts the failing raid and
#       PSD in the error state. The PDD status will go to inoperable.
#       This will stop the RAID layer from processing any additional commands.
#
#  INPUT:
#       g0 = PSD
#       g1 = RDD
#       g2 = PRP
#       g3 = ILT (not required - for flight recorder ONLY)
#
#  OUTPUT:
#       rd_status, ps_status, pd_devstat, and pd_poststat updated.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
# C access
# void RB_RAIDError(PSD* pPSD, RDD *pRDD, PRP* pPRP, ILT* pILT)
        .globl  RB_RAIDError
RB_RAIDError:
#
RB$rerror:
.if     DEBUG_FLIGHTREC_O
        ldconst frt_h_rerr3,r4          # Type & subtype
        st      r4,fr_parm0             # RAID error - rb$rerror_exec
        st      g3,fr_parm1             # ILT
        st      g2,fr_parm2             # PRP
? # crash - cqt# 24595 - 2008-12-01 -- BE PRP - failed @ rebld.as:1562  ld 48+g2,r3 with feedfeed
?       ld      pr_rsbytes(g2),r3       # PRP status: sensebytes/q/r/s
        st      r3,fr_parm3             # status
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_O
#
# --- Lock the RAID layer to prevent new commands from being processed
#
        ldconst TRUE,r3
        st      r3,R_errlock            # Set the lock
#
# --- Setup
#
        ldconst FALSE,r11               # Assume NVRAM update not needed
        ldos    ps_pid(g0),r12          # Get failing PID
        ld      rd_psd(g1),r13          # Get 1st PSD from RDD
        mov     r13,r10                 # Save starting PSD
#
# --- Locate the PSD linked to requested PID
#
.om20:
        ldos    ps_pid(r13),r8          # Get corresponding PID
        cmpobe  r8,r12,.om30            # Jif matches the failing PID
        ld      ps_npsd(r13),r13        # Advance to next PSD
        cmpobe  r13,r10,.om1000         # Jif device not found - return
                                        #   this shouldn't ever happen
        b       .om20                   # Check this PSD for a match
#
# --- Set the PSD status and astatus to error
#
.om30:

        ldos    ps_pid(r13),r3          # Get PID
        ld      P_pddindx[r3*4],r5      # Get PDD
        ldob    ps_astatus(r13),r4      # Get additional status
#
# --- Force hotspare for SCSI check conditions
#     with sense key 3 (media) and 4 (hardware)
#
        ldob    pd_flags(r5),r3
        bbs     pdbebusy,r3,.om1000
        ldob    pr_sstatus(g2),r3       # Get the PRP status
        cmpobne scechk,r3,.om35         # Only HAVE to hotspare on real drive
                                        #   media and hardware errors
        ldob    pr_sense+sskey(g2),r3   # Get byte with sense key
        and     skmask,r3,r3            # Mask the key
        cmpobe  skmedium,r3,.om32       # Jif media error and hotspare
        cmpobe  skhardware,r3,.om32     # Jim hardware error and hotspare
        b       .om35                   # Else normal error handling
#
.om32:
        ldob    pd_miscstat(r5),r3      # Change PDD misc status to replace
.ifndef MODEL_3000
.ifndef  MODEL_7400
        setbit  psarebuild,r4,r4     # Hotspare is required - check condition
.endif  # MODEL_7400
.endif  # MODEL_3000
.ifndef  MODEL_7000
.ifndef  MODEL_4700
        setbit  psahsrequired,r4,r4     # Hotspare is required - check condition
        setbit  pdmbreplace,r3,r3       # Set the replace bit
.endif  # MODEL_4700
.endif  # MODEL_7000
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        stob    r4,ps_astatus(r13)      # Mark PSD astatus with check cond
        stob    r3,pd_miscstat(r5)
#
.om35:
        bbs     psaerror,r4,.om40       # Jif status already in error
        setbit  psaerror,r4,r4          # Set the error bit
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        stob    r4,ps_astatus(r13)      # Mark PSD astatus in error state
        ldconst TRUE,r11                # NVRAM update needed
        ldconst pserror,r3
        stob    r3,ps_status(r13)       # Mark PSD status in error state
#
# --- Set the PDD post and dev status to inoperable/nonx
#
        ldob    pd_poststat(r5),r3      # Get the device POST status
        cmpo    pdnonx,r3               # Check if nonexistent
        sele    pdinop,pdnonx,r3        # Either set to inop or nonexistent
        stob    r3,pd_poststat(r5)
#
# --- If non-existant, leave it
#
        ldob    pd_devstat(r5),r3       # Get the device status
        cmpo    pdnonx,r3               # Check if nonexistent
        sele    pdinop,pdnonx,r3        # Either set to inop or nonexistent
        stob    r3,pd_devstat(r5)
#
# --- If a defrag was in progress then we must rebuild it
#
        ldob    ps_astatus(r13),r3      # Get additional status
        bbc     psadefrag,r3,.om40      # Jif no defrag in progress
        clrbit  psadefrag,r3,r3         # Clear the defrag bit
        setbit  psarebuild,r3,r3        # Set the rebuild bit
        clrbit  psauninit,r3,r3         # Clear the uninitialized bit
        stob    r3,ps_astatus(r13)
#
# --- Set RDD status to error if it's operable or degraded.  Also set the bit
#       to disable Writes while a local image is being processed. Then start
#       a task to ensure the local image is seen by the other controllers.
#
.om40:
        ldob    rd_astatus(g1),r4       # Get the current raid additional status
        ldob    rd_status(g1),r3        # Get current raid status
        bbs     rdalocalimageip,r4,.om45 # Jif the Local Image in Progress set
        ld      gRLLocalImageIPCheckPCB,r5 # Get the Checker Task PCB
        ldos    rd_rid(g1),r11
#c       fprintf(stderr,"<GR><RB_RaidError.>IMPORTANT.RID=%0lx.Setting the Local Image in Progress Flag\n",r11);
#c       fprintf(stderr,"<GR><RB_RaidError TBD...>IMPORTANT..This temporarily stops the write request on this Raid\n");
        ldconst TRUE,r11                # Show a NVRAM update is needed
        setbit  rdalocalimageip,r4,r4   # Set the Local Image in Progress Flag
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        stob    r4,rd_astatus(g1)       # Save the new additional status
        cmpobne 0,r5,.om45              # Jif the Checker Task is already going
        PushRegs(r4)                    # Save all 'g' registers
c       g0 = -1;                        # Flag task is being created.
        st      g0,gRLLocalImageIPCheckPCB
        lda     RL_LocalImageIPCheck,g0 # Task to start
        ldconst RLLOCALIMGIPPRI,g1      # Task Priority
c       CT_fork_tmp = (ulong)"RL_LocalImageIPCheck";
        call    K$tfork                 # Fork the task and continue
        st      g0,gRLLocalImageIPCheckPCB # Save the PCB for other checking
        PopRegsVoid(r4)                 # Restore 'g' registers
.om45:
        cmpobg  rdop,r3,.om50           # Jif status < operable
#
        ldconst TRUE,r11                # NVRAM update needed
        ldconst rderror,r3
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        stob    r3,rd_status(g1)        # Mark RDD status in error state
.om50:
#
        cmpobe  FALSE,r11,.om1000       # Don't update NVRAM if no changes
        call    D$p2update
#
# --- Exit
#
.om1000:
#
.if     DEBUG_FLIGHTREC_O
        ldconst frt_h_rerr0,r4          # Type & subtype
        st      r4,fr_parm0             # RAID error - rb$rerror_exec
        ldos    ps_rid(g0),r3           # RID
        ldos    ps_pid(g0),r4           # PID
        shlo    16,r3,r3
        or      r3,r4,r3
        st      r3,fr_parm1             # RID/PID
        st      g0,fr_parm2             # PSD
        ldob    rd_status(g1),r3        # Get RDD status
        ldob    rd_astatus(g1),r4       # Get RDD astatus
        ldob    ps_status(g0),r5        # Get PSD status
        ldob    ps_astatus(g0),r6       # Get PSD astatus
        shlo    24,r3,r3
        shlo    16,r4,r4
        shlo    8,r5,r5
        or      r4,r3,r3
        or      r5,r3,r3
        or      r6,r3,r3
        st      r3,fr_parm3             # status
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_O
        ret
#
#**********************************************************************
#
#  NAME: RB$rerror_que
#
#  PURPOSE:
#       To provide a common means of queuing raid 5 error I/O requests.
#
#  DESCRIPTION:
#       The ILT is queued to the tail of the executive queue.
#       The executive is activated to process this request.
#       This routine may be called from either the process or interrupt level.
#
#  CALLING SEQUENCE:
#       call    RB$rerror_que
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
#**********************************************************************
#
RB$rerror_que:
        lda     RB_rerror_qu,r11        # Get queue origin
        b       K$cque

#**********************************************************************
#
#  NAME: D$del_RB_qu
#
#  PURPOSE:
#       To provide a means of deleting a raid entry from the RB_rerror_qu.
#
#  DESCRIPTION:
#       This function looks for an ILT entry on the RB_rerror_qu that
#       matches the input RDD.  If a matching entry is found, that
#       entry is deleted.
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

R$del_RB_qu:
        mov     g1,r10                  # Save g1
        cmpobe  0,g0,.del_RB_qu90       # Jif input RDD equals 0
        lda     RB_rerror_qu,r6         # Get executive queue pointer
        ldq     qu_head(r6),r12         # Get queue head, tail, count
                                        #  and PCB in r12-r15
        mov     r12,r5                  # r5 = next queued ILT
#
# --- Walk the RB_rerror_qu looking for raid ID matching RDD in g0.
#
.del_RB_qu10:
        cmpobe  0,r5,.del_RB_qu90       # Jif next entry is 0 (end of queue).
c fprintf(stderr,"%s%s:%u <delRBqu-1>More ILTs in error queue...\n", FEBEMESSAGE, __FILE__, __LINE__);
        ld      il_w4(r5),r8            # Get the RDD address
        cmpobe  r8,g0,.del_RB_qu20      # Jif RDDs match
c fprintf(stderr,"%s%s:%u <delRBqu-2>ILT with our RDD exists...\n", FEBEMESSAGE, __FILE__, __LINE__);
        ld      il_fthd(r5),r5          # Load the next ILT
        b       .del_RB_qu10
#
# --- Remove this request from queue
#
.del_RB_qu20:
c fprintf(stderr, "%s%s:%u R$del_RB_qu removing ILT associated with raid %d\n", FEBEMESSAGE, __FILE__, __LINE__, ((RDD *)g0)->rid);
        ldl     il_fthd(r5),r8          # Forward and backward link (r8,r9).
        st      r8,il_fthd(r9)          # Store fwd thread to next queue entry
        cmpobe  0,r8,.del_RB_qu30       # Jif we're at the end of queue
        st      r9,il_bthd(r8)          # Store back thread to prev link
        b       .del_RB_qu40
#
.del_RB_qu30:
        cmpo    r9,r6                   # Is prev the queue header
        sele    r9,0,r9                 # Determine qu_tail
        st      r9,qu_tail(r6)          # Store qu_tail
.del_RB_qu40:
        subo    1,r14,r14               # Decrement qcount
        st      r14,qu_qcnt(r6)         # Store qcount
#
# --- Complete the original RRP request.
# The completion path will take care of Releasing the ILT/RRP/VRP
# When the error ILT is posted, the completion routine contains the address:
#       RB$rerror_comp - from raidinit/scrub/rebld.
#       K$comp         - from raid layer (originated from I/O path).
#
        ld      il_w0-ILTBIAS(r5),r7    # Get RRP request
        ldconst ecnonxdev,r4
        stob    r4,rr_status(r7)        # Set the status as non X device as this case
                                        # occurs only when device (RDD/VDD) is deleted.
        mov     r5,g1                   # Pass g1 = ILT to completion routine
        ld      il_cr(r5),r3            # Get completion routine
c fprintf(stderr, "%s%s:%u R$del_RB_qu completion function %lx - associated with raid %d\n", FEBEMESSAGE, __FILE__, __LINE__, r3, ((RDD *)g0)->rid);

        callx   (r3)                    # Call completion

        mov     r8,r5                   # To next entry in list
        b       .del_RB_qu10            # Walk the rest of the queue.
#
# --- Exit
#
.del_RB_qu90:
        mov     r10,g1                  # Restore g1
        ret

#**********************************************************************
#
#  NAME: rb$rerror_exec
#
#  PURPOSE:
#       Check all PSDs on a RAID and mark operable or inoperable as required.
#       Start rebuild if appropriate.
#
#  DESCRIPTION:
#       This process accepts ILTs for commands that have failed while
#       trying to do ops to a RAID device. This will walk through all the
#       PDDs on the failing RAID and do an inquiry and update the PDD status.
#
#       The physical device will be rebuilt if possible. If the RAID
#       has gone inoperable then the completion code will be changed and
#       the completion routine will be called. A parity scan is flagged for
#       a RAID 5 device that goes inoperable.
#
#  CALLING SEQUENCE:
#       process call
#
#  INPUT:
#       ILT      incoming to raid layer
#           il_w0-ILTBIAS   RRP
#           il_w4           RDD
#           il_cr           completion routine (will be passed ILT in g1)
#
#  OUTPUT:
#       rr_status is updated
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
rb$rerror_exec:
#
# --- At start of exec only
#
        ld      K_xpcb,g1               # get executing PCB of task
        st      g1,RB_rerror_qu+qu_pcb  # Save PCB
#
# --- Initialize Debug Data Retrieval (DDR) Raid error exec PCB entry
#
        # g1 = K_xpcb set above
c       M_addDDRentry(de_repcb, g1, pcbsiz);
#
# --- Initialize Debug Data Retrieval (DDR) RB_rerror_qu entry
#
        lda     RB_rerror_qu,g1         # Load address of o_rexec_qu header
c       M_addDDRentry(de_reeque, g1, qusiz);
#
#
# --- Top of main RAID error handling loop
#     This process is enabled when an ILT is added to the queue
#
.ore05:
        ldconst FALSE,r3
        st      r3,rb_rerrorbusy        # Mark the RAID exec as busy
#
.if     DEBUG_FLIGHTREC_O
        ldconst frt_h_rerr1,r4          # Type & subtype
        ldconst 0,r5
        st      r4,fr_parm0             # RAID error - rb$rerror_exec
        st      r5,fr_parm1
        st      r5,fr_parm2
        st      r3,fr_parm3
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_O
#
        lda     RB_rerror_qu,r11        # Get PCB of this process
c       if (((QU*)r11)->head == 0) {    # if nothing in queue
            ld      qu_pcb(r11),r15     # Get PCB of this process
            ldconst pcnrdy,r4           # Set this process to not ready
            stob    r4,pc_stat(r15)
            call    K$qxchang           # Exchange processes
c       }
#
        ldconst TRUE,r3
        st      r3,rb_rerrorbusy        # Mark the RAID exec as busy
#
.if     DEBUG_FLIGHTREC_O
        ldconst frt_h_rerr1,r4          # Type & subtype
        ldconst 0,r5
        st      r4,fr_parm0             # RAID error - rb$rerror_exec
        st      r5,fr_parm1
        st      r5,fr_parm2
        st      r3,fr_parm3
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_O
#
# --- Exchange processes until there are no outstanding RAID or
#     initialization requests
#
        call    D_pause_init
        ldconst 1000,g0                 # Wait a while
.ore10:
        call    K$twait
#jt
#jt - possible future use to avoid multiple rerror handling
#jt     ldconst TRUE,r3
#jt     st      r3,R_errlock            # Set the lock
#
        ld      R_orc,r3                # Get RAID outstanding request count
c       if (r3 != 0) {
        ld      RB_rerror_qu+qu_qcnt,r4 # Get queue count
c fprintf(stderr, "%s: %s%s:%u rb$rerror_exec[%ld] Waiting for R_orc (%ld) to become zero\n", millitime_string(), FEBEMESSAGE, __FILE__, __LINE__, r4, r3);
          ldconst 500,g0                # Wait a while
          b     .ore10                  # Jif still outstanding RAID requests
c       }
#
# --- Update NVRAM and get latest status on all PDDs
#
        ldconst mrdexisting,g0          # Rescan existing devices
                                        #  This will results in a call to
                                        #  O$hotswap to update device status
        call    P$rescanDevice          # look for all PDDs
#
        PushRegs(r4)                    # save registers
        call    RB_CheckOperablePDisks  # Log event if no PDDs are operable
                                        # returns GOOD or ERROR in g0
        PopRegs(r4)                     # Restore all registers except g0
#
        cmpobe  GOOD,g0,.ore30          # Jif operable devices
        ldos    K_ii+ii_status,r4       # Get status bits
        bbc     iiccbreq,r4,.ore30      # Jif only one controller present
#
        mov     g3,r4
        ldconst 0,g3
        call    cm$Vlock                # Lock virtual layer
        mov     r4,g3
c       GR_ClearLocalImageIP();
#
c fprintf(stderr, "%s%s:%u <GR><rerror_exec> Lost Storage. Going to error trap in 15 seconds...\n", FEBEMESSAGE, __FILE__, __LINE__);
# -- A 30 second wait allows c$stop, but not election, regardless this is a long time.
        ldconst 15000,g0                # Wait till other DCN takes over
        call    K$twait
        b       .err18                  # Error trap
#
# --- Handle each queued request
#
.ore30:
        call    K$xchang                # Exchange processes between entries
#
        lda     RB_rerror_qu,r11        # Get executive queue pointer
        ldq     qu_head(r11),r12        # Get queue head, tail, count
                                        #  and PCB in r12-r15
        mov     r12,g12                 # g12 = next queued ILT
        cmpobne 0,r12,.ore40            # Jif there is an ILT to process
#
# --- Done processing the queue - prepare to go back to sleep ---------
#
        call    RB_searchforfailedpsds  # Kickoff any hotspares
        call    rb$clearraiderrors      # Clear all error flags
        call    RB_setpsdstat           # Update latest PSD & RAID states
        call    RB_setvirtstat          # Update latest VDisk status
#
        call    D$p2update              # Update NVRAM to save device states
#
        ldconst FALSE,r3
        st      r3,R_errlock            # Unlock the RAID layer
c       TaskReadyByState(pcrerrorwait); # Ready the RAID exec
#
        call    D_resume_init           # Resume the initialization tasks
#
        ldconst erlexeccomp,g0
        call    rb$log_raid_event       # Log event to XSSA to get latest states
#
        b       .ore05                  # All done - wait for more ILTs
#
# --- Remove this request from queue
#
.ore40:
        ld      il_fthd(r12),r12        # Dequeue this ILT
        cmpo    0,r12                   # Check for queue now empty
        subo    1,r14,r14               # Adjust queue count
        sele    r13,r12,r13             # Set up queue tail
        stt     r12,qu_head(r11)        # Update queue head, tail and count
        be      .ore50                  # Jif queue now empty
#
        st      r11,il_bthd(r12)        # Update backward thread
#
.ore50:
#
# --- Handle the ILT in g12 -------------------------------------------
#
#
# --- Get the RDD
#
        ld      il_w4(g12),g10          # g10 = RDD
#
.if     DEBUG_FLIGHTREC_O
        ldconst frt_h_rerr2,r3          # Type & subtype
        st      r3,fr_parm0             # RAID error - rb$rerror_exec
        ldos    rd_rid(g10),r3
        shlo    16,r3,r3
        st      r3,fr_parm1             # RID/0
        st      g10,fr_parm2            # RDD
        st      g12,fr_parm3            # ILT
#       ld      il_w0-ILTBIAS(g12),r3   #         Debug only
#       st      r3,fr_parm3             # RRP
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_O
#
# --- For each PSD - check RAID status and decide if we should hotspare it
#
        ld      rd_psd(g10),r9          # r9 = 1st PSD from RDD
        ldob    rd_status(g10),r8       # r8 = Preloaded RAID Status
        mov     r9,g8                   # g8 = current working PSD
#
.ore100:
        ldos    rd_type(g10),r6         # get raid type
        cmpobg  r6,rdraid10,.ore105     # don't check raid types vlink and snapshot for busy.
        ldos    ps_pid(g8),r6           # Get PID
        ld      P_pddindx[r6*4],r3      # Get PDD
        ldob    pd_flags(r3),r6         # get flags
        bbs     pdbebusy,r6,.ore120     # if busy go to next PSD
.ore105:
        ldob    ps_astatus(g8),r6       # Get PSD additional status
        bbc     psaerror,r6,.ore120     # Jif not a pending error - next PSD
        mov     g10,g1
        call    rb$getraiderrorstat     # Get raid status at time of error
        mov     g0,r8                   # Keep results for later check
        cmpobg  rdop,g0,.ore110         # Jif < operable - clear required bit
                                        #   should never be operable
#
        setbit  psahotspare,r6,r6       # Hotspare is needed if available
        clrbit  psauninit,r6,r6         # Init no longer needed
        b       .ore115
#
# Don't fail this drive since it makes the RAID inoperable
#
.ore110:
        clrbit  psahsrequired,r6,r6     # Clear the hotspare required bit
#
.ore115:
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        stob    r6,ps_astatus(g8)       # Save updated astatus
#
.ore120:
        ld      ps_npsd(g8),g8          # g8 = next PSD in list
        cmpobne g8,r9,.ore100           # Jif more PSDs to check
#
# --- Mark RAIDs that now need a resync
# --- Fail the RRP
#
        cmpobe  rdinop,r8,.ore210       # Jif inoperative
        cmpobe  rdnonx,r8,.ore220       # Jif nonexistent - shouldn't happen
        b       .ore240                 # Keep existing status
#
.ore210:
        ldconst ecinop,r4               # Inoperative
        b       .ore230
#
.ore220:
        ldconst ecnonxdev,r4            # Non-existent
.ore230:
        ld      il_w0-ILTBIAS(g12),g13  # Get RRP request
        ldob    rr_status(g13),r3       # Get the current RRP Status
        ldconst ecretry,r5              # Determine if in RETRY status already
        cmpobe  r3,r5,.ore235           # Jif already set to RETRY status

.if 1 #VIJAY_MC -- This may not be needed here...check one more time (TBD)
        ldconst ecspecial,r5            # Determine if in special RETRY status already
        cmpobe  r3,r5,.ore235           # Jif already set to RETRY status
.endif  # 1

.ifndef MODEL_3000
.ifndef  MODEL_7400
        cmpobe  ecbebusy,r3,.ore235
.endif  # MODEL_7400
.endif  # MODEL_3000
        stob    r4,rr_status(g13)       # Save the correct status
#
# --- Start a parity check if this is RAID 5
#
.ore235:
        ldob    rd_type(g10),r3         # Get RAID type
        cmpobne rdraid5,r3,.ore240      # Jif not RAID 5
#
        ldob    rd_astatus(g10),r3      # Get RAID additional status
        bbs     rdarebuild,r3,.ore240   # Jif already scheduled for rebuild
        setbit  rdaparity,r3,r3         # Enable parity scan bit in RDD
        clrbit  rdar5srip,r3,r3         # Clear the R5 Stripe Resync In Progress
                                        #  - Mutually exclusive with full scan
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        stob    r3,rd_astatus(g10)
#
        ld      R_pcctrl,r3             # Get parity checker control word
        setbit  rdpcnewcmd,r3,r3        # Indicate a new parity command
        setbit  rdpcmarked,r3,r3        # Indicate there is a marked RDD
        clrbit  rdpcspecific,r3,r3      # Don't do a specific RID
        setbit  rdpccorrect,r3,r3       # Correct any out of sync parity stripes
        setbit  rdpc1pass,r3,r3         # Just do 1 pass
        setbit  rdpcenable,r3,r3        # Enable parity checking
        st      r3,R_pcctrl
#
        call    rb$log_parityscanrequired # log it
#
# --- Complete RRP request
#
.ore240:
        mov     g12,g1                  # Pass g1 = ILT to completion routine

        ld      il_cr(g12),r3           # Get completion routine
        callx   (r3)                    # Call completion

        b       .ore30                  # Do next ILT on the que
#
#**********************************************************************
#
#  NAME: RB$rerror_comp
#
#  PURPOSE:
#       Completion routine for a RAID error that failed and needs no callback.
#
#  DESCRIPTION:
#       This is called from the RAID error handler. It releases the ILT/RRP
#       that were previously queued.
#
#  CALLING SEQUENCE:
#       call RB$rerror_comp
#
#  INPUT:
#       g1      ILT with RRP
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
# C Access
# void RB_RAIDErrorComp(UINT32 unused, ILT* pILT)
        .globl  RB_RAIDErrorComp
RB_RAIDErrorComp:
RB$rerror_comp:
        lda     -ILTBIAS(g1),g1         # Return to previous ILT level
        call    M$rir                   # Release the ILT/RRP
        ret
#
#**********************************************************************
#
#  NAME: rb$clearraiderrors
#
#  PURPOSE:
#       This routine clears the error bit the additional status field of
#       each PSD in the system.
#
#  DESCRIPTION:
#       This searches each PSD in each RAID in the system.
#       Do I need to check for ownership?
#
#  CALLING SEQUENCE:
#       call    rb$clearraiderrors
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
rb$clearraiderrors:
        movq    g0,r12                  # Save registers
#
# --- Check next RAID device
#
        lda     R_rddindx,r11           # Prepare search
        ldconst MAXRAIDS-1,r6           # Load count
#
.ocr10:
        ld      rx_rdd(r11)[r6*4],g1    # Get next RDD
        cmpobe  0,g1,.ocr50             # Jif undefined - next RAID
#
# --- Check for non-redundant RAID type
#
        ldob    rd_type(g1),r3          # Get RAID type
        cmpobe  rdstd,r3,.ocr50         # Jif STD - next RDD
        cmpobe  rdraid0,r3,.ocr50       # Jif RAID 0 - next RDD
        cmpobe  rdlinkdev,r3,.ocr50     # Jif linked device - next RDD
        cmpobe  rdslinkdev,r3,.ocr50    # Jif snapshot device - next RDD
#
        ldos    rd_psdcnt(g1),r7        # Get PSD count
        ld      rd_psd(g1),g0           # Get 1st PSD
#
# --- Clear the error bit
#
.ocr20:
        ldob    ps_astatus(g0),r3       # Get PSD additional status
        clrbit  psaerror,r3,r3          # Clear the error bit
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        stob    r3,ps_astatus(g0)       # Save it
#
# --- Advance to next PSD
#
        subo    1,r7,r7
        ld      ps_npsd(g0),g0          # Get next PSD
        cmpobne 0,r7,.ocr20             # Jif more
#
# --- Advance to next RAID device
#
.ocr50:
        subo    1,r6,r6                 # Bump to next device
        cmpible 0,r6,.ocr10             # Jif more
#
# --- Exit
#
        movq    r12,g0                  # Restore registers
        ret
#
#**********************************************************************
#
#  NAME: RB_searchforfailedpsds
#
#  PURPOSE:
#       To provide a common means of initiating rebuilds and hotspares.
#
#  DESCRIPTION:
#       This routine searches all RAID devices looking for physical devices
#       that need to be rebuilt or hotspared.
#
#       This will update the local status for all RAIDs but will only do
#       a rebuild for RAIDs that this controller owns.
#
#  CALLING SEQUENCE:
#       call    RB_searchforfailedpsds
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
# C access
# void RB_SearchForFailedPSDs(void);
        .globl  RB_SearchForFailedPSDs # C access
RB_SearchForFailedPSDs:
#
RB_searchforfailedpsds:
.if     DEBUG_FLIGHTREC_OHSPARE
        ldconst frt_h_rbld8,r3          # Rebuilding functions
        st      r3,fr_parm0             # Function -searchforfailedpsds- start
        ldconst 0,r3                    # Clear
        st      r3,fr_parm1             # Clear
        st      r3,fr_parm2             # Clear
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_OHSPARE
#
# --- Don't check PSD status if the error handler is still running or a
#     target move is in-progress.
#
        ldconst FALSE,r11               # Prep update required to FALSE
#
        ld      RB_rerror_qu+qu_qcnt,r3 # Get queue count of the raid error task
        cmpobne 0,r3,.osf100
        ld      D_moveinprogress,r3     # Get target move indicator
        cmpobe  TRUE,r3,.osf100
#
        movq    g0,r12                  # Save registers
#
# --- Check next RAID device
#
        ldconst MAXRAIDS-1,r6           # Load count
#
.osf10:
        ld      R_rddindx[r6*4],g1      # Get next RDD
        cmpobe  0,g1,.osf90             # Jif undefined
#
# --- Check for non-redundant RAID type
#
        ldob    rd_type(g1),r3          # Get RAID type
        cmpobe  rdstd,r3,.osf80         # Jif STD - skip it
        cmpobe  rdraid0,r3,.osf80       # Jif RAID 0 - skip it
        cmpobe  rdlinkdev,r3,.osf80     # Jif linked device - skip it
        cmpobe  rdslinkdev,r3,.osf80    # Jif snapshot device - skip it
#
        ldos    rd_psdcnt(g1),r7        # Get PSD count
        ld      rd_psd(g1),g2           # Get 1st PSD
#
# --- Check next PSD
#
.osf20:
        ldos    ps_pid(g2),r4           # Get PID
        ld      P_pddindx[r4*4],r5      # Get PDD
c if (r5 != 0) {
        ldob    pd_flags(r5),r10         # get flags
        bbs     pdbebusy,r10,.osf70      # if busy go to next PSD
c }
#
        ldob    ps_status(g2),r10       # Get PSD status
        ldob    ps_astatus(g2),r3       # Get PSD additional status
# --- Attempt to rebuild or hotspare
#
        bbs     psahsrequired,r3,.osf30 # HS required
        bbc     psahotspare,r3,.osf40   # Jif hotspare not needed - check rebld
#
.osf30:
        mov     g2,g0                   # Put PSD in g0
        call    rb$hsparepsd            # Hotspare it - g0 is PSD
        ldconst TRUE,r11                # Update is required
        b       .osf70                  # Do next PSD
#
.osf40:
        bbc     psarebuild,r3,.osf70    # Jif rebuild not required - next PSD
#
        ldos    ps_pid(g2),r4           # Get PID
        ld      P_pddindx[r4*4],r5      # Get PDD
c if (r5 != 0) {
        ldob    pd_poststat(r5),r4      # Get physical device POST status
        ldob    pd_devstat(r5),r5       # Get physical device status
        cmpobg  pdop,r5,.osf60          # Jif PDD not operable - hotspare
c }
#
        call    rb$rebuildpsd           # Rebuild it - g2 is PSD
        ldconst TRUE,r11                # Update is required
        b       .osf70                  # Do next PSD
#
# --- The PSD is inoperative but needs a rebuild so try to hotspare it
#     to a different drive.
#
.osf60:                                 # PDD inoperable - find a hotspare
        cmpobne pdinop,r5,.osf65        # Jif other than inoperable (nonx)
        ldconst pdaspin,r5              # If in the process of spinning up, skip
        cmpobe  r4,r5,.osf70
#
.osf65:
        mov     g2,g0                   # Put PSD in g0
        call    rb$hsparepsd            # Hotspare it - g0 is PSD
        ldconst TRUE,r11                # Update is required
#
# --- Advance to next PSD
#
.osf70:
        subo    1,r7,r7                 # Decrement PSD count
        ld      ps_npsd(g2),g2          # Get next PSD
        cmpobne 0,r7,.osf20             # Jif more
#
# --- Update the RAID status
#
.osf80:
        call    RB_setraidstat
#
# --- Advance to next RAID device
#
.osf90:
        subo    1,r6,r6                 # Bump to next device
        cmpible 0,r6,.osf10             # Jif more
#
# --- Wrap up
#
        call    RB_setvirtstat          # Update virtual device status
#
        cmpobne TRUE,r11,.osf95         # Jif no update happened
#
#
# Update the state of the raids rebuild before write flags
#
        ldconst TRUE,g0                 # Remote Cache update required
        ldconst TRUE,g1                 # Force P2 update when completed
# Need to save and restore G-regs, to fix the bug# TBolt00010965 (Wookiee)
        PushRegs(r3)
        call    RB_UpdateRebuildWriteState
        PopRegsVoid(r3)
#
.osf95:
        ld      R_pcctrl,r3             # Get parity checker control word
        setbit  rdpcnewcmd,r3,r3        # Indicate a new parity command
        setbit  rdpcmarked,r3,r3        # Indicate there is a marked RDD
        clrbit  rdpcspecific,r3,r3      # Don't do a specific RID
        setbit  rdpccorrect,r3,r3       # Correct any out of sync parity stripes
        setbit  rdpc1pass,r3,r3         # Just do 1 pass
        setbit  rdpcenable,r3,r3        # Enable parity checking
        st      r3,R_pcctrl
#
        movq    r12,g0                  # Restore registers
#
# --- Exit
#
.osf100:
.if     DEBUG_FLIGHTREC_OHSPARE
        ldconst frt_h_rbld9,r3          # Rebuilding functions
        st      r3,fr_parm0             # Function -searchforfailedpsds -end
        ldconst 0,r3                    # Clear
        st      r3,fr_parm1             # Clear
        st      r3,fr_parm2             # Clear
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_OHSPARE
        ret
#
#**********************************************************************
#
#  NAME: rb$hsparepsd
#
#  PURPOSE:
#       To provide a common means of dynamically assigning a hotspare
#       device for a PSD device that has failed.
#
#  DESCRIPTION:
#       The PSD is checked to determine if the specified device is
#       designated as operative or rebuilding.  If so, the device
#       failure is logged and an attempt is made to assign a hotspare.
#       The caller is responsible for making a call to d$p2update after
#       calling this function.
#
#  CALLING SEQUENCE:
#       call    rb$hsparepsd            # Write failed device label
#
#  INPUT:
#       g0 = PSD
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
rb$hsparepsd:
        mov     g0,r12                  # Save PSD passed in
        PushRegs(r15)                   # Save all registers on stack for "C"
#
# --- Log the hotspare error but only the 1st time
#
        ldob    ps_astatus(r12),r8      # Get additional status
        ldconst mledebug,g1             # Pass log type - for debug
        bbc     psahotspare,r8,.hs10    # Jif this is NOT a first time hotspare
#
# --- First time failure ----------------------------------------------
#     Maybe this should be a separate function later
#
        call    rb$log_raid_error       # Log error to CCB, g0=PSD, g1=type
#
# --- Setup delay for hotspare
#
        PushRegs(r3)                    # save registers
        call    RB_AcceptIOError        # Prepare to handle hotspare
        PopRegsVoid(r3)                 # Restore all  registers
#
# --- Locate corresponding PDD from information in PSD
#
        lda     P_pddindx,g3            # Get address of table
        ldos    ps_pid(r12),r3          # Get the PID
        ld      px_pdd(g3)[r3*4],g3     # g3 has PDD pointer
#
        ldob    pd_miscstat(g3),r3      # Mark PDD as suspect file system
        setbit  pdmbfserror,r3,r3
c fprintf(stderr, "%s%s:%u Setting pdmbfserror into miscstat for pid=%d -- mark suspect file system, miscstat=0x%02lx\n", FEBEMESSAGE, __FILE__, __LINE__,((PDD*)g3)->pid,r3);
        stob    r3,pd_miscstat(g3)
#
# --- Hotspare is REQUIRED
#     This occurs on hardware and media check conditions
#     - Update failure LED & PDD status
#     - Attempt to write failed label
#
        bbc     psahsrequired,r8,.hs10  # Jif hotspare NOT required
#
        ldconst pdlitefail,r3           # Turn fault light on
        stob    r3,pd_fled(g3)
        call    O$ledchanged            # Inform CCB of LED change
#
# --- If the device is non-existant, skip trying to write fail label
#
        ldconst pdnonx,r4
        ldob    pd_devstat(g3),r3       # Get the device status
        cmpobe  r4,r3,.hs10             # jif if nonexistent
#
c fprintf(stderr,"%s%s:%u <PDD-BUSY>RB$hsparepsd-- writing failed label on pid=%d\n", FEBEMESSAGE, __FILE__, __LINE__,((PDD*)g3)->pid);
        lda     O$writefailedlabel,g0   # Get write failed label process
        ldconst OSPINDOWNPRIO,g1
        ldconst xdfailgen,g2            # Load failure type
c       CT_fork_tmp = (ulong)"O$writefailedlabel";
        call    K$tfork
#
# --- All failures ----------------------------------------------------
#
.hs10:
#
.if     DEBUG_FLIGHTREC_O
        ldconst frt_h_misc0,r3          # Misc hot swap function
        st      r3,fr_parm0             # Function - rb$hsparepsd
        ldos    ps_rid(r12),r3          # RID
        ldos    ps_pid(r12),r4          # PID
        shlo    16,r3,r3
        or      r3,r4,r3
        st      r3,fr_parm1             # RID/PID
        st      g0,fr_parm2             # PSD being hot spared
        ldob    ps_status(r12),r3       # Get PSD status
        ldob    ps_astatus(r12),r4      # Get additional status
        shlo    8,r3,r3
        or      r3,r4,r3
        st      r3,fr_parm3             # Drive status
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_O
#
# --- Adjust the device status flags
#
        ldos    ps_rid(r12),r4
        ldob    ps_astatus(r12),r5
        ld      R_rddindx[r4*4],r6      # Get RDD
        clrbit  psahotspare,r5,r5       # Clear the hotspare bit
#
        ldob    rd_astatus(r6),r3       # Get the AStatus for the RDD
        bbs     rdaparity,r3,.hs050     # Jif Parity Scan Already required
        bbc     rdar5srip,r3,.hs030     # Jif not doing a Stripe Resync
#
        setbit  rdaparity,r3,r3         # Stripe Resync in progress, turn into
        clrbit  rdar5srip,r3,r3         #  a full parity scan
        ld      R_pcctrl,r4             # Get parity checker control word
        setbit  rdpcnewcmd,r4,r4        # Indicate a new parity command
        setbit  rdpcmarked,r4,r4        # Indicate there is a marked RDD
        clrbit  rdpcspecific,r4,r4      # Don't do a specific RID
        setbit  rdpccorrect,r4,r4       # Correct any out of sync parity stripes
        setbit  rdpc1pass,r4,r4         # Just do 1 pass
        setbit  rdpcenable,r4,r4        # Enable parity checking
        st      r4,R_pcctrl
        b       .hs040
#
.hs030:
        setbit  rdarebuild,r3,r3        # Indicate a rebuild on this RAID
#
        setbit  psarebuild,r5,r5        # Set the rebuild bit in the PSD AStatus
        clrbit  psauninit,r5,r5         # Clear the uninitialized bit
        ldconst psrebuild,r7            # Schedule a rebuild on this PSD
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        stob    r7,ps_status(r12)       # Update the status
.hs040:
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        stob    r3,rd_astatus(r6)       # Store the new RDD AStatus
#
.hs050:
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        stob    r5,ps_astatus(r12)      # Store the PSD AStatus adjusted above
        ldos    K_ii+ii_status,r4       # Get status bits
        bbs     iislave,r4,.hs1000      # Jif this CN is a slave - exit
#
# --- Check current status for this device
#
        mov     r6,g1                   # Pass RDD
        call    RB_setraidstat          # Update RAID status
#
        ld      rb_hotsparewait,r3      # Get current hotsparewait time
        cmpobne 0,r3,.hs120             # Jif time not elapsed
#
                                        # PSD passed in g0
        call    RB$canspare             # Check for sufficient redundancy
        cmpobne deok,g0,.hs120          # Jif not able to rebuild
#
# --- Locate corresponding PDD from information in PSD
#
        lda     P_pddindx,g3            # Get address of table
        ldos    ps_pid(r12),r3          # Get the PID
        ld      px_pdd(g3)[r3*4],g3     # g3 has PDD pointer
#
# --- Update failure LED & PDD status
#
        ldconst pdlitefail,r3           # Turn fault light on
        stob    r3,pd_fled(g3)
        call    O$ledchanged            # Inform CCB of LED change
#
        ldob    pd_devstat(g3),r3       # Get the device status
        cmpo    pdnonx,r3               # Check if nonexistent
        sele    pdinop,pdnonx,r3        # Either set to inop or nonexistent
        stob    r3,pd_devstat(g3)
#
        ldob    pd_miscstat(g3),r3      # Mark PDD as suspect file system
        setbit  pdmbfserror,r3,r3
c fprintf(stderr, "%s%s:%u Setting pdmbfserror into miscstat for pid=%d -- mark suspect file system, miscstat=0x%02lx\n", FEBEMESSAGE, __FILE__, __LINE__,((PDD*)g3)->pid,r3);
        stob    r3,pd_miscstat(g3)
#
# --- If the device is non-existant, skip trying to write fail label
#
        ldconst pdnonx,r4
        ldob    pd_devstat(g3),r3       # Get the device status
        cmpobe  r4,r3,.hs060            # jif if nonexistent
#
# --- Attempt to write failed label
#
c fprintf(stderr,"%s%s:%u <PDD-BUSY>RB$hsparepsd2-- writing failed laben on pid=%d\n", FEBEMESSAGE, __FILE__, __LINE__,((PDD*)g3)->pid);
        lda     O$writefailedlabel,g0   # Get write failed label process
        ldconst OSPINDOWNPRIO,g1
        ldconst xdfailgen,g2            # Load failure type
c       CT_fork_tmp = (ulong)"O$writefailedlabel";
        call    K$tfork
#
# --- Redirect activity to hotspare if possible
#
.hs060:
        mov     r12,g0                  # Pass PSD
        mov     g5,r3                   # Save g5
        ldconst 0,g5                    # Allocate a new hotspare
        call    rb$redirectpsd          # Redirect PSD to hotspare
        mov     r3,g5
#
# --- Update the RDD status after a PSD has been redirected
#     But this runs even if no hotspares are available...
#
        ldconst MAXRAIDS,r4             # Initialize index into RDX
        lda     R_rddindx,r5            # Get RDX
#
.hs110:
        subo    1,r4,r4                 # Decrement index
        cmpibg  0,r4,.hs120             # Jif done
        ld      rx_rdd(r5)[r4*4],g1     # Get RDD
        cmpobe  0,g1,.hs110             # Jif nonx
        call    RB_setraidstat          # Update RAID status
        b       .hs110                  # Get next RDD
#
# --- Update Virtual status for RAID 0 devices only
#
.hs120:
        call    RB_setvirtstat          # Update the VDD status
#
# --- Exit
#
.hs1000:
        PopRegsVoid(r15)                # Restore registers
        ret
#
#**********************************************************************
#
#  NAME: RB$canspare
#
#  PURPOSE:
#       To provide a means of determining if a drive has sufficient
#       redundancy to be spared safely.
#
#  DESCRIPTION:
#       Given a PSD to obtain the PID, searches RDD's for references, and
#       verifies enough redundancy exists to rebuild the data for this
#       device.
#
#  CALLING SEQUENCE:
#       call    RB$canspare
#
#  INPUT:
#       g0 = PSD
#
#  OUTPUT:
#       g0 = 0 if can spare
#            deinsuffredund if cannot spare due to inoperable drives
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
# C access
# UINT8 RB_CanSpare(PSD* pPSD);
        .globl  RB_CanSpare
RB_CanSpare:
RB$canspare:
        PushRegs                        # Save all G registers (stack relative)
.if     DEBUG_FLIGHTREC_OHSPARE
        st      g0,fr_parm2             # PSD being checked
.endif  # DEBUG_FLIGHTREC_OHSPARE
#
# --- Prepare to check all RDDs
#
        ldos    ps_pid(g0),g12          # Get designated PID
        ldconst MAXRAIDS-1,r15          # Get maximum RAID index
        ldconst deinsuffredund,g0       # Assume inoperable drives return
#
# --- Check next RDD
#
.cs10_r:
        ld      R_rddindx[r15*4],r14    # Get next RDD
        cmpobe  0,r14,.cs900            # Jif nonx
#
        ld      rd_psd(r14),r13         # Get 1st PSD from RDD
        ldob    rd_type(r14),r4         # Get raid type
        mov     r13,r10                 # Save starting PSD
#
# --- Locate PSD following the PSD linked to requested PID
#
.cs20_r:
        ldos    ps_pid(r13),g8          # Get corresponding PID
        ld      ps_npsd(r13),r13        # Advance to next PSD
        cmpobe  g8,g12,.cs25            # Jif match
#
        cmpobne r13,r10,.cs20_r         # Jif PSD search incomplete
        b       .cs900                  # Check next RDD
#
# --- Transfer control according to RAID level
#
.cs25:
        cmpobe  rdraid5,r4,.cs200       # Jif RAID 5
        cmpobe  rdraid10,r4,.cs700      # Jif RAID 10
        cmpobe  rdlinkdev,r4,.cs900     # Just skip linked devices
        cmpobe  rdslinkdev,r4,.cs900    # Just skip snapshot devices
        cmpobne rdraid1,r4,.cs1000      # Jif not RAID 1
#
# --- Check RAID 1 array ----------------------------------------------
#
# --- Check if any drive in the RAID is operable.  If it is, then the
# --- RAID can be rebuilt.  Loop through the PSDs until the target PSD
# --- is found.  If there were no operable drives, but there was a
# --- PSD in the defragmentation state, then we would be rebuildable
# --- but only following the completion of the defragmentation.
#
        mov     r10,r5                  # Set up current PSD
#
.cs30:
        ldos    ps_pid(r5),g8           # Get corresponding PID
        cmpobe  g8,g12,.cs40            # Jif this is the PID being checked
#
# --- Check operability
#
        ldob    ps_status(r5),r4        # Get status of segment
        cmpobe  psop,r4,.cs900          # Jif OK
#
# --- Advance to next PSD
#
.cs40:
        ld      ps_npsd(r5),r5          # Advance to next PSD
        cmpobne r10,r5,.cs30            # Jif more
#
# --- Done checking for operable drives.  None were found.
#
        b       .cs1000                 # Report non-redundancy
#
# --- Check RAID 5 array ----------------------------------------------
#
# --- For RAID 5, we check to see if all the drives in the stripe are
# --- operable.  If any of them are inoperable, then the RAID cannot
# --- be rebuilt.
#
.cs200:
        ldob    rd_astatus(r14),r3      # Get additional status
        bbs     rdaparity,r3,.cs1000    # Can't spare if we need to sync
#
        ldos    rd_psdcnt(r14),r12      # Get number of physical segments
        ldob    rd_depth(r14),r11       # Get stripe width
        remo    r11,r12,r3              # Check for evenly divisible
        cmpobne 0,r3,.cs300             # Jif not
#
# --- Process regular array -------------
#
#       i.e,    0 0 0   1 1 1   2 2 2
#               3 3 3   4 4 4    etc.
#
# --- Since this is a regular stripe, we only have to make one pass
# --- through the RAIDs drives and find the one stripe the target PSD is
# --- in.  If that stripe is rebuildable, no more checking is to be done.
#
        mov     r10,r3                  # Set up current PSD
        mov     FALSE,r5                # Clear specified device found
#
# --- Process next group
#
.cs210:
        mov     0,r4                    # Clear non-operable count for this group
        mov     r11,r6                  # Get stripe width
#
.cs220:
        ldos    ps_pid(r3),g8           # Get corresponding PID
        cmpobne g8,g12,.cs230           # Jif no match
#
        mov     TRUE,r5                 # Set specified device found
        b       .cs240
#
.cs230:
        ldob    ps_status(r3),r8        # Get status of segment
        cmpobe  psop,r8,.cs240          # Jif OK
        addo    1,r4,r4                 # Bump non-operable count
#
.cs240:
        ld      ps_npsd(r3),r3          # Advance to next PSD
        subo    1,r6,r6                 # Adjust remaining devices in group
        cmpobne 0,r6,.cs220             # Jif more
#
# --- Advance to next group
#
        cmpobne TRUE,r5,.cs210          # Jif device not found in last group
#
        cmpobe  0,r4,.cs900             # Jif redundancy exists
        b       .cs1000                 # Else, report non-redundant
#
# --- Process irregular array -----------
#
#       i.e,    0 0 0 1 1 1 2
#               2 2 3 3 3 4 4
#               4 5 5 5 6 6 6
#                  etc.
#
# --- Since this is an irregular stripe, the PSD we are checking may be in
# --- a number of different stripes.  Note above, that the second drive is
# --- in stripe 0, 2, and 5 in one maxi-stripe.  A shortcut to doing this
# --- check is to make one pass through the RAID, but check the n-1 drives
# --- before and the n-1 drives after the drive we are checking.  Again,
# --- from the example, if we are checking the second drive, we need to
# --- check the third and fourth (post segments) and the first and last
# --- drive (pre segments).
#
.cs300:
        mov     0,r3                    # Clear error accumulator
        ldconst 3,g9                    # Set up 3 drive mask
        cmpobe  3,r11,.cs310            # Jif 3 drive stripe
#
        ldconst 0xf,g9                  # Set up 5 drive mask
        cmpobe  5,r11,.cs310            # Jif 5 drive stripe
#
        ldconst 0xff,g9                 # Set up 9 drive mask
#
.cs310:
        subo    1,r11,r11               # Adjust stripe width to n-1
#
# --- Process pre n-1 segments
#
.cs320:
        ldos    ps_pid(r13),g8          # Get corresponding PID
        cmpobe  g8,g12,.cs350           # Jif match
#
        ldob    ps_status(r13),r6       # Get status of segment
        shlo    1,r3,r3                 # Shift error accumulation
        and     g9,r3,r3                # Limit to significant portion
        cmpobe  psop,r6,.cs330          # Jif OK
        or      1,r3,r3                 # Mark error
#
.cs330:
        ld      ps_npsd(r13),r13        # Link to next PSD
        b       .cs320
#
# --- Process post n-1 segments
#
.cs350:
        ld      ps_npsd(r13),r13        # Link to next PSD
        ldob    ps_status(r13),r6       # Get status of segment
        cmpobe  psop,r6,.cs360          # Jif OK
        or      1,r3,r3                 # Mark error
#
.cs360:
        subo    1,r11,r11               # Adjust remaining seg count
        cmpobne 0,r11,.cs350            # Jif more
#
        cmpobe  0,r3,.cs900             # Jif redundant
        b       .cs1000                 # Report non-redundant
#
# --- Check RAID 10 array ---------------------------------------------
#
.cs700:
        ldos    rd_psdcnt(r14),r12      # Get number of physical segments
        ldob    rd_depth(r14),r11       # Get mirror depth
        remo    r11,r12,r3              # Check for evenly divisible
        cmpobne 0,r3,.cs800             # Jif not
#
# --- Process regular array -------------
#
#       i.e,    0 0  1 1  2 2
#               3 3  4 4  5 5  etc.
#
# --- For a regular array, make one pass through the PSD list and check
# --- the PSDs in any one stripe.  If the drive we are checking happens
# --- to be in the stripe, then the check is done.
#
        mov     r10,r3                  # Set up current PSD
        mov     FALSE,r5                # Clear specified device found
#
# --- Process next group
#
.cs710:
        mov     0,r4                    # Clear operable count for this group
        mov     r11,r6                  # Get mirror depth
#
.cs720:
        ldos    ps_pid(r3),g8           # Get corresponding PID
        cmpobne g8,g12,.cs730           # Jif no match
#
        mov     TRUE,r5                 # Set specified device found
        b       .cs750
#
.cs730:
        ldob    ps_status(r3),r8        # Get status of segment
        cmpobne psop,r8,.cs750          # Jif not OK
        addo    1,r4,r4                 # Bump operable count
#
.cs750:
        ld      ps_npsd(r3),r3          # Advance to next PSD
        subo    1,r6,r6                 # Adjust remaining devices in group
        cmpobne 0,r6,.cs720             # Jif more
#
# --- Advance to next group
#
        cmpobne TRUE,r5,.cs710          # Jif device not found in last group
#
        cmpobne 0,r4,.cs900             # Jif redundancy exists
        b       .cs1000                 # Report non-redundant
#
# --- Process irregular array -----------
#
#       i.e,    0 0 1 1 2
#               2 3 3 4 4
#                  etc.
#
# --- For an irregular array, we need to find one drive operable in the
# --- n-1 drives preceeding this drive and in the n-1 drives following
# --- the one being tested.
#
.cs800:
        mov     0,r3                    # Clear operable map
        subo    1,r11,r11               # Adjust mirror depth to n-1
#
# --- Build pre n-1 mask
#
        mov     1,g9                    # Prime mask
        mov     r11,r5                  # Get n-1
#
.cs810:
        subo    1,r5,r5                 # Decrement mirror depth
        cmpobe  0,r5,.cs820             # Jif complete
#
        shlo    1,g9,g9                 # Shift mask by 1
        or      1,g9,g9                 # Create next mask bit
        b       .cs810
#
# --- Process pre n-1 segments
#
.cs820:
        ldos    ps_pid(r13),g8          # Get corresponding PID
        cmpobe  g8,g12,.cs850           # Jif match
#
        ldob    ps_status(r13),r6       # Get status of segment
        shlo    1,r3,r3                 # Shift operable map
        and     g9,r3,r3                # Limit to significant portion
        cmpobne psop,r6,.cs840          # Jif not OK
        or      1,r3,r3                 # Mark operable
#
.cs840:
        ld      ps_npsd(r13),r13        # Link to next PSD
        b       .cs820
#
# --- Check for redundancy in pre n-1 segments
#
.cs850:
        cmpobe  0,r3,.cs1000            # Jif no operable drives in the n-1 seg
#
# --- Process post n-1 segments
#
.cs870:
        ld      ps_npsd(r13),r13        # Link to next PSD
        ldob    ps_status(r13),r6       # Get status of segment
        cmpobe  psop,r6,.cs900          # Jif OK
#
        subo    1,r11,r11               # Adjust remaining seg count
        cmpobne 0,r11,.cs870            # Jif more
#
        b       .cs1000                 # No redundancy
#
# --- Advance to next RDD ---------------------------------------------
#
.cs900:
        subo    1,r15,r15               # Bump to next RAID index
        cmpible 0,r15,.cs10_r           # Jif more
#
# --- Now that we made it this far, it says that we did not drop out due
# --- to a failure to be redundant.
#
        ldconst 0,g0                    # Set return to show we can spare
#
# --- Exit
#
.cs1000:
.if     DEBUG_FLIGHTREC_OHSPARE
        ldconst frt_h_misc3,r3          # Misc hot swap function
        st      r3,fr_parm0             # Function - d$canspare
        ld      fr_parm2,r5             # PSD being checked
        ldos    ps_rid(r5),r3           # RID
        ldos    ps_pid(r5),r4           # PID
        shlo    16,r3,r3
        or      r3,r4,r3
        st      r3,fr_parm1             # RID/PID
                                        # PSD being checked (filled at top)
        st      g0,fr_parm3             # Answer
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_OHSPARE
        PopRegs                         # Restore g1 thru g14
        ret
#
#**********************************************************************
#
#  NAME: rb$redirectpsd
#
#  PURPOSE:
#       To provide a common means of causing the assignment of a
#       hotspare to all failing PSDs referencing the same physical
#       device.
#
#  DESCRIPTION:
#       An attempt is made to locate a suitable hotspare for the failing
#       device.  If one is found, all PSDs are linked to this device
#       and a rebuild is initiated.
#
#  CALLING SEQUENCE:
#       call    rb$redirectpsd
#
#  INPUT:
#       g0 = failing PSD
#       g5 = hotspare PDD, 0 means this routine should find a good hotspare
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
        .globl  RB_RedirectPSD
RB_RedirectPSD:
        mov     g1,g5
# NOTE: fall through.
rb$redirectpsd:
        PushRegs                        # Save all G registers (stack relative)
#
        mov     g0,r15                  # Save failing PSD for later
        ldos    ps_pid(g0),g4           # Get failing PID
        ld      P_pddindx[g4*4],r14     # Save failing PDD
#
.if     DEBUG_FLIGHTREC_OHSPARE
        ldconst frt_h_rbld2,r3          # Rebuilding functions
        st      r3,fr_parm0             # Function - rb$redirectpsd
        ldos    ps_rid(g0),r3           # Failing RID
        ldos    ps_pid(g0),r4           # Failing PID
        shlo    16,r3,r3
        or      r3,r4,r3
        st      r3,fr_parm1             # RID/PID
        st      g0,fr_parm2             # PSD
        ldconst 0,r3                    # Constant 0
        cmpobe  0,g5,.r05               # JIF if g5 (PDD) is 0
        ldos    pd_pid(g5),r3           # Hotspare pid
.r05:
        st      r3,fr_parm3             # Input hotspare PID
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_OHSPARE
#
# --- Locate hotspare
#
        cmpobne 0,g5,.r30               # Don't do this if hotspare passed in
        call    rb$findcapacity         # Find required capacity - pass g4=PID
        call    rb$findhotspare         # Attempt to find suitable hotspare
                                        #  hotspare PDD returned in g5
        cmpobne 0,g5,.r30               # Jif - hotspares available
#
# --- Prepare to send a log message - Device Failed No Hotspare
# Do not need to save registers, they are restored in .r1000 as we leave.
#
# --- Notify CCB that no hotspare is available
#
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst mledevfailnhs,r4        # Event code
        st      r4,mle_event(g0)        # Store as word to clear other bytes
#
        ld      pd_channel(r14),r4      # FC channel and LUN of failed device
        ldos    pd_pid(r14),r10         # PID
        ldos    ps_rid(r15),r5          # RID
        ld      R_rddindx[r5*4],r3      # RDD
        ldos    rd_vid(r3),r6           # VID
        ld      pd_id(r14),r7           # LID
        ldl     pd_wwn(r14),r8          # FC WWN of failed device (r8, r9)
        st      r4,edn_channel(g0)      # Store all
        stos    r10,edn_pid(g0)
        stos    r5,edn_rid(g0)
        stos    r6,edn_vid(g0)
        st      r7,edn_id(g0)
        stl     r8,edn_wwn(g0)          # (r8, r9)
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], ednlen);
# Do not need to restore registers, they are restored in .r1000 as we leave.
        b       .r1000                  # Exit - no hotspare available
#
# --- Hotspare available, continue
#
.r30:
c       *(UINT64*)&g6 = SYSSRESERVE;    # Get 1st available data sector
#
# --- Dirty both the source and destination PDDs to make sure the DAM
# --- are not used until rebuilt.
#
        mov     g4,g0
        call    D$damdirtyshell         # Dirty the source PDD DAM
#
        ldos    pd_pid(g5),g0           # Get the destination PID
        call    D$damdirtyshell         # Dirty it
#
# --- Prepare search for affected PSDs --------------------------------
#     Search for a matching PID in each RDD in the system
#
        ldconst 0,r6                    # Starting RAID ID
        ldconst MAXRAIDS-1,r9           # Get maximum RAID ID
#
# --- Examine next RDD
#
.r40:
        ld      R_rddindx[r6*4],r5      # Get next RDD
        cmpobe  0,r5,.r80               # Jif undefined - do next RDD
#
        ldob    rd_type(r5),r7          # Get RAID type
        cmpobe  rdraid0,r7,.r80         # Jif RAID 0 - do next RDD
        cmpobe  rdstd,r7,.r80           # Jif STD - do next RDD
        cmpobe  rdlinkdev,r7,.r80       # Jif linked device - do next RDD
        cmpobe  rdslinkdev,r7,.r80      # Jif snapshot device - do next RDD
#
        ldos    rd_psdcnt(r5),r7        # Get number of PSD's to process
        ld      rd_psd(r5),r8           # Get 1st PSD
#
# --- Examine next PSD
#
.r50:
        ldos    ps_pid(r8),g12          # Get PID
        cmpobne g12,g4,.r70             # Jif no match - next PSD
#
# --- Found a matching PSD
#
        ldob    rd_astatus(r5),r3       # Set rebuild in RDD additional status
        setbit  rdarebuild,r3,r3
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        stob    r3,rd_astatus(r5)
#
        ldob    ps_astatus(r8),r3       # Set rebuild in PSD additional status
        setbit  psarebuild,r3,r3        # Set the rebuild needed bit
        clrbit  psahotspare,r3,r3       # Don't need to hotspare it later
        clrbit  psauninit,r3,r3         # Don't need to initialize it later
#
        cmpobe  0,g5,.r60               # Jif no hotspare available
        clrbit  psahsrequired,r3,r3     # Not required to hotspare later
.r60:
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        stob    r3,ps_astatus(r8)
#
        ldconst psrebuild,r3
        stob    r3,ps_status(r8)        # Set status to rebuild required
#
        cmpobe  0,g5,.r80               # Jif no hotspare available - next RDD
#
# --- Setup for a rebuild on this PSD - defrag & reorder RIDs
#
c       ((PSD*)r8)->rLen = 0;           # Clear rebuild length
#
c       ((PSD*)r8)->sda = *(UINT64*)&g6; # Update SDA
c       *(UINT64*)&g6 += ((PSD*)r8)->sLen; # Adjust SDA for next segment
#
        ldos    pd_pid(g5),r3           # Get PID
        stos    r3,ps_pid(r8)           # Set up to use new PID
#
        mov     r8,g2                   # Schedule PSD for rebuild
        call    rb$rebuildpsd
        b       .r80                    # Goto next RDD
#
# --- Advance to next PSD
#
.r70:
        ld      ps_npsd(r8),r8          # Link to next PSD
        subo    1,r7,r7                 # Adjust remaining PSD count
        cmpobne 0,r7,.r50               # Jif more
#
# --- Advance to next RDD
#
.r80:
        addo    1,r6,r6                 # Bump to next RAID ID
        cmpoble r6,r9,.r40              # Jif more
#
# --- Update space allocations in hotspare PDD & failed PDD -----------
#
        cmpobe  0,g5,.r1000             # Jif hotspare not assigned - exit
#
        ldos    pd_pid(g5),g0           # Get the Hotspare PID
        ldconst TRUE,g1                 # Force on
        call    D$calcspaceshell        # Force new capacities into the PDD
#
        ldos    pd_pid(r14),g0          # Get the failing PID
        ldconst TRUE,g1                 # Force on
        call    D$calcspaceshell        # Force new capacities into the PDD
#
        PushRegs(r3)                    # Save registers
        mov     g5,g0
        call    RB_CalcPercentRemaining # Calculate the percent remaining
        PopRegsVoid(r3)
#
c       ((PDD*)r14)->rbRemain = 0;      # Clear the remaining blocks to rebuild
c       ((PDD*)r14)->pctRem = 0;        # Clear percent rebuilding remaining on drive
#
# --- Set the hot spare dname in the hot spare to indicate where the data
# --- came from.
#
        ldob    pd_ses(r14),r4
        ldob    pd_slot(r14),r12
        ldconst 0xff,r5
        cmpobe  r5,r4,.r90           # Jif SES invalid
        cmpobe  r5,r12,.r90          # Jif SLOT is invalid
c       r4 = 0x00004450 | ( r4  << 16) | (r12  << 24);  /* Current name (failing drive) */
        b       .r91
#
# Normally won't occur. A failing drive must have SES and SLOT available within its PDD
# The probable case for this may be 'pdisk bypass'-- In this case for fail back we will track the
# dname instead of ses and slot as they are invalid.. so fail back may not happen properly, since
# the 'dname' contain its previous location. The case where failback fails is when a drive from
# a slot(&ses) the dname is of which is not this ses(&slot) as it might have been brought from other slot,
# is failed to the hotspare. In this scenario, the failback will not happen to the slot (when new drive is
# placed)  from where it is failed, instead failed to the slot (i.e. previous location of the drive when it
# failed in this slot).
.r90:
        ld     pd_dname(r14),r4         # Track the old name
.r91:
        st      r4,pd_hsdname(g5)       # Save it
#
# Necessary to save all information for message, before D$p2updateconfig call.
# It will do task switches, and the PDD structure might disappear (other
# structures too?).
#
# Needed after call: g5 and r14.  Preload registers with message data.
#
        ld      pd_channel(r14),r3      # FC channel and LUN of failed device
        ldos    pd_pid(r14),r5          # PID
        ld      pd_id(r14),r6           # FC ID of failed device
        ldl     pd_wwn(r14),r8          # FC WWN of failed device (r8,r9)
        ld      pd_channel(g5),r7       # FC channel and LUN of spare device
        ldos    pd_pid(g5),r10          # PID
        ld      pd_id(g5),r11           # FC ID of spare device
        ldl     pd_wwn(g5),r12          # FC WWN of spare device (r12, r13)
#
# Now do the call that task switches and might allow PDD structure to disappear.
#
        call    D$p2updateconfig        # Update NVRAM
#
# --- This log message signals the CCB to initiate an NVRAM restore for all
# --- controllers, which will cause slave controllers to start rebuilds.
#
# --- Notify CCB that hotspare is available and PSDs are redirected
#
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst mledevfailhs,r4         # Event code
        st      r4,mle_event(g0)        # Store as word to clear other bytes
        st      r3,edh_channel(g0)      # Store
        stos    r5,edh_pid(g0)
        st      r6,edh_id(g0)
        stl     r8,edh_wwn(g0)          # r8, r9
        st      r7,edh_hschan(g0)       # Store
        stos    r10,edh_hspid(g0)
        st      r11,edh_hsid(g0)
        stl     r12,edh_hswwn(g0)       # (r12, r13)
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], edhlen);
# Do not need to restore registers, they are restored in .r1000 as we leave.
#
# --- Exit
#
.r1000:
        PopRegsVoid                     # Restore all G registers (stack relative)
        ret
#
#**********************************************************************
#
#  NAME: rb$rebuildpsd
#
#  PURPOSE:
#       Bring a PSD to the attention of the PSD rebuild task.  If the
#       task is not running, start the task and add the PSD to the list
#       of PSD's to rebuild.  If it is running, just add the PSD to the
#       list of PSD's to work on.
#
#  DESCRIPTION:
#       Checks RB_rbrhead for a non-null value.  if not 0, traverses list
#       to end and attaches PSD (g2) to activity list.  if RB_rbrhead is
#       null, starts the task and sets up the activity list.
#
#  CALLING SEQUENCE:
#       call    rb$rebuildpsd
#
#  INPUT:
#       g2 = PSD
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
rb$rebuildpsd:
        PushRegs                        # Save all G registers (stack relative)
#
        ldos    ps_rid(g2),r4           # Get RID
        ld      R_rddindx[r4*4],g1      # g1 = RDD
#
.if     DEBUG_FLIGHTREC_OHSPARE
        ldconst frt_h_rbld0,r3          # Rebuilding functions
        st      r3,fr_parm0             # Function - start of rb$rebuildpsd
        ldos    ps_rid(g2),r3           # RID
        ldos    ps_pid(g2),r4           # PID
        shlo    16,r3,r3
        or      r3,r4,r3
        st      r3,fr_parm1             # RID/PID
        st      g2,fr_parm2             # PSD
        st      g1,fr_parm3             # RDD
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_OHSPARE
#
# --- Check for rebuildable device
#
        call    rb$canrebuild           # Check for rebuildable device
        cmpobe  FALSE,g0,.drb1000       # Jif not and return
#
        ldos    rd_vid(g1),g0           # Get the VID
#
        PushRegs(r3)                    # Save regs
        call    DL_AmIOwner             # Determine if we are the owner or not
        PopRegs(r3)                     # Restore regs
#
        cmpobe  FALSE,g0,.drb1000       # Do next RDD if not owned
#
        call    RB_setraidstat          # Set raid status
#
        ldos    ps_pid(g2),r15          # Get corresponding PID
        ld      P_pddindx[r15*4],r15    # Get PDD
        cmpobe  0,r15,.drb1000          # Jif PDD is NULL
#
        ldob    pd_miscstat(r15),r14    # Get the current misc status
        setbit  pdmbschedrebuild,r14,r14 # Prep PDD misc status with scheduled
#
        ldos    ps_rid(g2),r13          # Get corresponding RAID ID
#
# --- Search the RBR list to see if the PSD we are trying to rebuild is already
# --- in the list.
#     - if rebuild list is empty then allocate new RBR
#     - If the PSD and the RBR have the same PDD and RBR not aborted just exit.
#     - If the PDDs are different then abort that RBR + allocate new RBR.
#     - If the PSD is not in the list then allocate an RBR for it.
#
        ld      RB_rbrhead,r12          # r12 = pointer to head of rebuild list
        cmpobe  0,r12,.drb130           # If no head, must start thread
#
.drb100:
        mov     r12,r5                  # r5 = RBR
        ld      rbr_psd(r5),r4          # Get the PSD for this RBR
        cmpobe  r4,g2,.drb120           # Jif the PSD already in the list
#
.drb110:
        ld      rbr_next(r5),r12        # Get pointer to next RBR node
        cmpobne 0,r12,.drb100           # If not end of list, keep following it
#
        mov     r5,r12                  # r12 = last RBR on list
        b       .drb130                 # Drop down to allocate new RBR
#
.drb120:
        ld      rbr_pdd(r5),r3          # Valid RBR will have same PDD as PSD
        cmpobe  r3,r15,.drb125          # Jif valid RBR in the list - chk status
#
        mov     r5,g0                   # g0 = rbr to cancel
        call    rb$cancel_rbr           # cancel this rebuild
        b       .drb110                 # Continue checking remaining RBRs
#
.drb125:
        ldob    rbr_status(r5),r3       # Get status
        cmpobe  strbcancel,r3,.drb110   # Jif this entry already cancelled to
                                        #   check remaining RBRs
        b       .drb1000                # Exit without a new RBR
#
# --- Check if already rebuilding this PDD and if so don't change miscstat.
#
.drb130:
        ldob    pd_miscstat(r15),g0     # g0 = current pd_miscstat
        bbs     pdmbrebuilding,g0,.drb150 # Jif currently rebuilding PDD
        stob    r14,pd_miscstat(r15)    # Set scheduled for rebuild
#
# --- As a shortcut to calling the update misc status function, just set the
# --- status of the pdisk to defragging if the status of this PSD is defrag.
#
        ldob    ps_astatus(g2),r3       # Get alternate status
        bbc     psadefrag,r3,.drb150    # Jif not a defrag
#
        setbit  pdmbdefragging,r14,r14  # Set defragging
        stob    r14,pd_miscstat(r15)    # Set it
#
# --- Allocate a new RBR ---
#
.drb150:
c       g0 = s_MallocC(rbrsiz, __FILE__, __LINE__); # Build ourselves an RBR to use
#
        stos    r13,rbr_rid(g0)         # Set up RAID ID
        st      g2,rbr_psd(g0)          # Point this entry at PSD in question
        ldconst psrebuild,r4            # r4 = new ps_status
        cmpobe  0,r12,.drb160           # Jif RBR list empty
#
        st      g0,rbr_next(r12)        # Link previous entry to this one
#
# --- Set up the RBR just allocated.  Note that a clear was done on allocation,
# --- so fields like the next link are already NULL.
#
.drb160:
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        stob    r4,ps_status(g2)        # Save new ps_status (psrebuild)
#
        ldob    ps_astatus(g2),r3       # Set rebuild in PSD additional status
        setbit  psarebuild,r3,r3
        clrbit  psauninit,r3,r3         # Clear the uninitialized bit
        stob    r3,ps_astatus(g2)
#
        ldob    rd_astatus(g1),r3       # Set rebuild in RDD additional status
        setbit  rdarebuild,r3,r3
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        stob    r3,rd_astatus(g1)
#
c       ((PSD*)g2)->rLen = 0;           # Clear (initialize) rebuild length
        mov     strbnorm,r3
        stob    r3,rbr_status(g0)       # Initialize RBR status
#
c       ((PDD*)r15)->rbRemain += ((PSD*)g2)->sLen; # subtract sectors in this segment
#
        PushRegs(r3)                    # Save registers
        mov     r15,g0
        call    RB_CalcPercentRemaining  # Calculate the percent remaining
        PopRegsVoid(r3)
#
        ldconst rbrdelaylog,r4
        st      r4,rbr_delaylogtime(g0) # Reset the time until logging
#
        st      r15,rbr_pdd(g0)         # Save PDD pointer in case of RDD deletion
c       ((RBR*)g0)->rlen = ((PSD*)g2)->sLen # Save # of blocks in this segment
#
.if     DEBUG_FLIGHTREC_OHSPARE
        ldconst frt_h_rbld1,r3          # Rebuilding functions
        st      r3,fr_parm0             # Function - rebuild started
        ldos    ps_rid(g2),r3           # RID
        ldos    ps_pid(g2),r4           # PID
        shlo    16,r3,r3
        or      r3,r4,r3
        st      r3,fr_parm1             # RID/PID
        st      g2,fr_parm2             # PSD
        st      g0,fr_parm3             # RBR
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_OHSPARE
#
# --- If RBR list is empty,  start up the rebuilder task to perform the actual
# --- rebuild operations.
#
        cmpobne 0,r12,.drb1000          # Jif RBR list not empty
#
        st      g0,RB_rbrhead           # Store the pointer to the first node
#
        ld      gPSDRebuilderPCB,  g0   # Get Rebuilder task
        cmpobne  0,g0,.drb1000          # Jif Rebuilder task is active
#
c       g0 = -1;                        # Flag task being created.
        st      g0,gPSDRebuilderPCB
        lda     rb$psd_rebuilder,g0
        ldconst DPSDREBUILDPRIO,g1
c       CT_fork_tmp = (ulong)"rb$psd_rebuilder";
        call    K$tfork                 # Spawn the rebuild process
        st      g0,gPSDRebuilderPCB     # save Rebuilder task PCB
#
# --- Exit
#
.drb1000:
        PopRegsVoid                     # Restore all G registers (stack relative)
        ret
#
#**********************************************************************
#
#  NAME: rb$canrebuild
#
#  PURPOSE:
#       To provide a common means of determining whether or not a
#       specific PSD can be rebuilt.
#
#  DESCRIPTION:
#       The specified PSD is checked to determine whether or not there
#       is enough redundancy to reconstruct this device.  The result of
#       this check is returned to the caller.
#
#  CALLING SEQUENCE:
#       call    rb$canrebuild
#
#  INPUT:
#       g2 = PSD
#
#  OUTPUT:
#       g0 = T/F
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
rb$canrebuild:
#
# --- Check for redundant RAID type
#
        ldos    ps_rid(g2),r3           # Get corresponding RAID ID
        mov     FALSE,g0                # Set false return for now
        ld      R_rddindx[r3*4],r14     # Get RDD
        ld      rd_psd(r14),r13         # Get 1st PSD from RDD
        ldob    rd_type(r14),r3         # Get RAID type
        cmpobe  rdstd,r3,.cb1000        # Jif STD
        cmpobe  rdraid0,r3,.cb1000      # Jif RAID 0
        cmpobe  rdlinkdev,r3,.cb1000    # Jif linked device
        cmpobe  rdslinkdev,r3,.cb1000   # Jif snapshot device
        cmpobe  rdraid5,r3,.cb100       # Jif RAID 5
        cmpobe  rdraid10,r3,.cb20       # Jif RAID 10
#
# --- Process RAID 1 --------------------------------------------------
#
        mov     g2,r4                   # Get target PSD
#
.cb10:
        ld      ps_npsd(r4),r4          # Get next PSD
        cmpobe  g2,r4,.cb1000           # Jif done
#
        ldob    ps_status(r4),r5        # Get status
        cmpobne psop,r5,.cb10           # Jif not operational
#
        mov     TRUE,g0                 # Set true return
        b       .cb1000
#
# --- Process RAID 10 -------------------------------------------------
#
.cb20:
        ldos    rd_psdcnt(r14),r12      # Get number of physical segments
        ldob    rd_depth(r14),r11       # Get mirror depth
        remo    r11,r12,r3              # Check for evenly divisible
        cmpobne 0,r3,.cb45              # Jif not
#
# --- Process regular array -------------
#
#       i.e,    0 0  1 1  2 2
#               3 3  4 4  5 5  etc.
#
        mov     r13,r3                  # Get 1st PSD
        mov     FALSE,r5                # Clear specified device found
#
# --- Process next group
#
.cb25:
        mov     0,r4                    # Clear operable count for this group
        mov     r11,r6                  # Get mirror depth
#
.cb30:
        cmpobne r3,g2,.cb35             # Jif not specified device
#
        mov     TRUE,r5                 # Set specified device found
        b       .cb40
#
.cb35:
        ldob    ps_status(r3),r8        # Get status of segment
        cmpobne psop,r8,.cb40           # Jif not OK
#
        addo    1,r4,r4                 # Bump operable count
#
.cb40:
        ld      ps_npsd(r3),r3          # Advance to next PSD
        subo    1,r6,r6                 # Adjust remaining devices in group
        cmpobne 0,r6,.cb30              # Jif more
#
# --- Advance to next group
#
        cmpobne TRUE,r5,.cb25           # Continue if specified device
                                        #  was not found in last group
        cmpobe  0,r4,.cb1000            # Jif no redundancy
#
        mov     TRUE,g0                 # Set true return
        b       .cb1000
#
# --- Process irregular array -----------
#
#       i.e,    0 0 1 1 2
#               2 3 3 4 4
#                  etc.
#
.cb45:
        ld      ps_npsd(g2),r13         # Get PSD following designated PSD
        mov     0,r3                    # Clear operable map
        subo    1,r11,r11               # Adjust mirror depth to n-1
#
# --- Build pre n-1 mask
#
        mov     1,r4                    # Prime mask
        mov     r11,r5                  # Get n-1
#
.cb50:
        subo    1,r5,r5                 # Decrement mirror depth
        cmpobe  0,r5,.cb55              # Jif complete
#
        shlo    1,r4,r4                 # Shift mask by 1
        or      1,r4,r4                 # Create next mask bit
        b       .cb50
#
# --- Process pre n-1 segments
#
.cb55:
        cmpobe  r13,g2,.cb65            # Jif match
#
        ldob    ps_status(r13),r6       # Get status of segment
        shlo    1,r3,r3                 # Shift operable map
        and     r4,r3,r3                # Limit to significant portion
        cmpobne psop,r6,.cb60           # Jif not OK
#
        or      1,r3,r3                 # Mark operable
#
.cb60:
        ld      ps_npsd(r13),r13        # Link to next PSD
        b       .cb55
#
# --- Check for redundancy in pre n-1 segments
#
.cb65:
        cmpobe  0,r3,.cb1000            # Jif not redundant
#
# --- Process post n-1 segments
#
.cb70:
        ld      ps_npsd(r13),r13        # Link to next PSD
        ldob    ps_status(r13),r6       # Get status of segment
        cmpobe  psop,r6,.cb75           # Jif OK
#
        subo    1,r11,r11               # Adjust remaining seg count
        cmpobne 0,r11,.cb70             # Jif more
#
        b       .cb1000
#
.cb75:
        mov     TRUE,g0                 # Set true return
        b       .cb1000
#
# --- Process RAID 5 --------------------------------------------------
#
.cb100:
        ldos    rd_psdcnt(r14),r12      # Get number of physical segments
        ldob    rd_depth(r14),r11       # Get stripe width
        remo    r11,r12,r3              # Check for evenly divisible
        cmpobne 0,r3,.cb200             # Jif not
#
# --- Process regular array -------------
#
#       i.e,    0 0 0   1 1 1   2 2 2
#               3 3 3   4 4 4    etc.
#
        mov     r13,r3                  # Set up current PSD
        mov     FALSE,r5                # Clear specified device found
#
# --- Process next group
#
.cb110:
        mov     0,r4                    # Clear non-operable cnt for this group
        mov     r11,r6                  # Get stripe width
#
.cb120:
        cmpobne r3,g2,.cb130            # Jif no match
#
        mov     TRUE,r5                 # Set specified device found
        b       .cb140
#
.cb130:
        ldob    ps_status(r3),r8        # Get status of segment
        cmpobe  psop,r8,.cb140          # Jif OK
#
        addo    1,r4,r4                 # Bump non-operable count
#
.cb140:
        ld      ps_npsd(r3),r3          # Advance to next PSD
        subo    1,r6,r6                 # Adjust remaining devices in group
        cmpobne 0,r6,.cb120             # Jif more
#
# --- Advance to next group
#
        cmpobne TRUE,r5,.cb110          # Continue if specified device
                                        #  was not found in last group
        cmpobne 0,r4,.cb1000            # Jif no redundancy exists
#
        mov     TRUE,g0                 # Set true return
        b       .cb1000
#
# --- Process irregular array -----------
#
#       i.e,    0 0 0 1 1 1 2
#               2 2 3 3 3 4 4
#               4 5 5 5 6 6 6
#                  etc.
#
.cb200:
        ld      ps_npsd(g2),r13         # Get PSD following designated
                                        #  PSD
        mov     0,r3                    # Clear error accumulator
        ldconst 3,r4                    # Set up 3 drive mask
        cmpobe  3,r11,.cb210            # Jif 3 drive stripe
#
        ldconst 0xf,r4                  # Set up 5 drive mask
        cmpobe  5,r11,.cb210            # Jif 5 drive stripe
#
        ldconst 0xff,r4                 # Set up 9 drive mask
#
.cb210:
        subo    1,r11,r11               # Adjust stripe width to n-1
#
# --- Process pre n-1 segments
#
.cb220:
        cmpobe  r13,g2,.cb250           # Jif match
#
        ldob    ps_status(r13),r6       # Get status of segment
        shlo    1,r3,r3                 # Shift error accumulation
        and     r4,r3,r3                # Limit to significant portion
        cmpobe  psop,r6,.cb230          # Jif OK
#
        or      1,r3,r3                 # Mark error
#
.cb230:
        ld      ps_npsd(r13),r13        # Link to next PSD
        b       .cb220
#
# --- Process post n-1 segments
#
.cb250:
        ld      ps_npsd(r13),r13        # Link to next PSD
        ldob    ps_status(r13),r6       # Get status of segment
        cmpobe  psop,r6,.cb260          # Jif OK
#
        or      1,r3,r3                 # Mark error
#
.cb260:
        subo    1,r11,r11               # Adjust remaining seg count
        cmpobne 0,r11,.cb250            # Jif more
#
        cmpobne 0,r3,.cb1000            # Jif no redundancy exists
#
        mov     TRUE,g0                 # Set true return
#
# --- Exit
#
.cb1000:
#
.if     DEBUG_FLIGHTREC_OHSPARE
        ldconst frt_h_misc2,r3          # Misc hot swap function
        st      r3,fr_parm0             # Function rb$canrebuild
        ldos    ps_rid(g2),r3           # RID
        ldos    ps_pid(g2),r4           # PID
        shlo    16,r3,r3
        or      r3,r4,r3
        st      r3,fr_parm1             # RID/PID
        st      r3,fr_parm1             # NULL
        st      g2,fr_parm2             # PSD being checked
        st      g0,fr_parm3             # Answer
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_OHSPARE
        ret
#
#**********************************************************************
#
#  NAME: rb$hotsparewaittask
#
#  PURPOSE:
#       This task delays x seconds after an IO error then kicks off
#       any hotspare activity. It is forked by rb_acceptioerror.
#
#  DESCRIPTION:
#       This waits until the hotspare delay count goes to 0 then
#       searches for failed PSDs and exits.
#
#  CALLING SEQUENCE:
#      None.  Task started by RB_acceptioerror when necessary.
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       Decrements rb_hotsparewait
#
#  REGS DESTROYED:
#       g0
#
#**********************************************************************
# C access
# void RB_HotspareWaitTask(void)
        .globl  RB_HotspareWaitTask
RB_HotspareWaitTask:
rb$hotsparewaittask:
        ldconst 1000,g0                 # Wait 1 second and try again
        call    K$twait
#
        ld      rb_hotsparewait,r8      # Get timer
        cmpobe  0,r8,.rbw1000           # Jif timer expired
        subo    1,r8,r8                 # Decrement time
        st      r8,rb_hotsparewait
        b       rb$hotsparewaittask     # Wait again
#
.rbw1000:
        ldconst 0,r3
        st      r3,rb_hotsparewaitpcb
#
        call    RB_searchforfailedpsds
        ret
#
#**********************************************************************
#
#  NAME: rb$psd_rebuilder
#
#  PURPOSE:
#       Rebuild any PSD's in the RBR list.  If no more entries exist,
#       terminate the task.
#
#  CALLING SEQUENCE:
#      None.  Task started by d$rebuild when necessary.
#
#  INPUT:
#       List of PSD's to rebuild, anchor at RB_rbrhead.
#
#  OUTPUT:
#       none.
#
#  REGS DESTROYED:
#       g0
#
#**********************************************************************
#
        .set    rbrdelay,60*2           # Seconds to delay rebuild process
        .set    rbrdelaylog,rbrdelay*30 # Seconds to log a rebuild delay
                                        # Must be multiple of rbrdelay
#
rb$psd_rebuilder:
#
# --- Check for stop active,  system not yet initialized, or resync in progress
#
        ldob    rb_stopcnt,r3           # Get stop counter
        cmpobne 0,r3,.dbr10             # Jif active
#
        ldob    O_p2init,r3             # Get phase II inits complete
        cmpobne TRUE,r3,.dbr10          # Jif not completed
#
        PushRegs(r3)                    # save all registers
        call    NVA_CheckReSyncInProgress # Check for resyncs in progress
        PopRegs(r3)                     # restore all registers except G0
        cmpobe  FALSE,g0,.dbr20         # Jif no resyncs in progress
#
.dbr10:
        ldconst 1000,g0                 # Wait 1 second and try again
        call    K$twait
        b       rb$psd_rebuilder
#
# --- System is running, we're active, are there any PSD's queued to rebuild?
#
.dbr20:
        ld      RB_rbrhead,r15          # Get head of list
        cmpobe  0,r15,.dbr1000          # If nothing there, quit
        ldconst eprecok,g6              # Assume rebuild completes OK
#
        ldob    rbr_status(r15),r5      # r5 = current RBR status
        cmpobe  strbcancel,r5,.dbr800   # Jif cancelled - don't process RBR
#
        setbit  eprecdone,g6,g6         # Get ready to log rebuild already done
        ld      rbr_psd(r15),r3
        ldob    ps_astatus(r3),r3
        bbc     psarebuild,r3,.dbr810   # Don't start a rebuild if already done
        clrbit  eprecdone,g6,g6
#
        cmpobe  strbnorm,r5,.dbr70      # Jif normal status
#
# --- Must be delay status. Delay awhile, process all delay
# --- status RBRs and then continue.  To process a delayed RBR,
# --- decrement the delay counter and if zero, then change the
# --- status to normal (versus delayed).
#
        ldconst 1000,g0                 # Delay 1 sec.
        call    K$twait
#
.dbr30:
        ldob    rbr_status(r15),r5      # r5 = current RBR status
        cmpobe  strbnorm,r5,.dbr40      # Jif normal
#
        ld      rbr_delaylogtime(r15),r4 # Decrement delay log time
        subo    1,r4,r4
        st      r4,rbr_delaylogtime(r15)
        ldob    rbr_delaytime(r15),r4   # Delay seconds remaining
        subo    1,r4,r4                 # Dec. count
        stob    r4,rbr_delaytime(r15)   # Save updated delay time
        cmpobne 0,r4,.dbr40             # Jif delay count not expired
        cmpobe  strbcancel,r5,.dbr40    # Jif cancelled - don't change rbr_status
#
        ldconst strbnorm,r5             # Set RBR status to normal
        stob    r5,rbr_status(r15)
#
.dbr40:
        ld      rbr_next(r15),r15       # Get next RBR on list
        cmpobne 0,r15,.dbr30            # Jif more RBRs to process
                                        # else drop through
#
# --- All of the delays have been checked.  Process the list again, looking
# --- for an RBR to process.
#
        ld      RB_rbrhead,r15          # r15 = top RBR on list
        ld      rbr_next(r15),r5        # r5 = next RBR on list
        cmpobe  0,r5,.dbr20             # Jif only RBR on list - do it
#
        st      r5,RB_rbrhead           # Save next RBR as head honcho!
#
.dbr50:
        ld      rbr_next(r5),r4         # Find end of list
        cmpobe  0,r4,.dbr60             # Jif end of list
        mov     r4,r5
        b       .dbr50                  # And check next one
#
.dbr60:
        st      r4,rbr_next(r15)        # Clear link field
        st      r15,rbr_next(r5)        # Save RBR on end of list
        b       .dbr20                  # And process next RBR
#
# --- Process the RBR -------------------------------------------------
#
# --- At this point, r15 has the RBR that we will process.
# --- Also set up the following long term registers.
#
#     r15 = RBR being processed
#     r14 = PSD for that RBR
#     r13 = RDD for that RBR
#     r12 = PDD for that RBR
#     r11 = Ordinal count of the PSD within the RAID
#     r10 = PSD for R1/10 read operation (set later)
#     g6  = error code
#
.dbr70:
#
# --- Set up the RDD and check for existence.  If it does not exist, toss the
# --- rebuild record.
#
        ld      rbr_psd(r15),r14        # Get PSD to work on
        ldos    rbr_rid(r15),r13        # Get RAID ID
        ld      R_rddindx[r13*4],r13    # Get RDD for the PSD in question
        ld      rbr_pdd(r15),r12        # Get PDD to work on
        ldconst eprecok,g6              # Assume rebuild completes OK
        cmpobne  0,r13,.dbr75           # JIF valid RDD
        call    .err10                  # Fault since this cannot happen
#
# --- Find the PSD ordinal for this PSD/RDD
#
.dbr75:
        mov     0,r11
        ldos    rd_psdcnt(r13),r3       # Get count of PSD's to check
#
.dbr80:
        ld      rd_psd(r13)[r11*4],r5   # Get a PSD pointer
        cmpobe  r5,r14,.dbr90
#
        subo    1,r3,r3                 # Decrement number of PSD's to check
        lda     1(r11),r11              # Increment ordinal by one
        cmpobne 0,r3,.dbr80             # If not out of PSD's to check, go look
        call    .err10                  # PSDs cannot be deleted from a RAID
#
.dbr90:
.if     DEBUG_FLIGHTREC_OHSPARE
        ldconst frt_h_rbld3,r3          # Rebuilding functions
        st      r3,fr_parm0             # Function - rb$psd_rebuilder start
        ldos    ps_rid(r14),r3          # RID
        ldos    ps_pid(r14),r4          # PID
        shlo    16,r3,r3
        or      r3,r4,r3
        st      r3,fr_parm1             # RID/PID
        st      r14,fr_parm2            # PSD being rebuilt
        st      r15,fr_parm3            # RBR
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_OHSPARE
#
        PushRegs(r3)                    # Save registers
        call    DEF_UMiscStat           # Update PDD misc status
        PopRegsVoid(r3)                 # Restore all the registers.
#
# --- Log the start of the RAID/PID rebuild.
#
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst mlepsdrebuildstrt,r3    # Get log code
        st      r3,mle_event(g0)        # Store as word to clear other bytes
        ldos    rbr_rid(r15),r3         # Get RID
        stos    r3,epr_rid(g0)          # Save it
        ldos    rd_vid(r13),r4          # Get VID
        stos    r4,epr_vid(g0)          # Save it
        ldos    pd_lun(r12),r3          # Get the LUN
        st      r3,epr_lun(g0)          # Save it, clearing the reserved short
        ldl     pd_wwn(r12),g2          # Get the WWN
        stl     g2,epr_wwn(g0)          # Save it
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], eprlen);
#
# --- Checks are in the logic to not schedule a rebuild operation on a
# --- non-rebuildable type of RAID. Therefore, this case should never
# --- occur unless something has changed to cause this inconsistency.
#
        ldob    rd_type(r13),r3         # Get type of device
        cmpobe  rdraid1,r3,.dbr100      # If RAID 1 or 10 then use this algorithm
        cmpobe  rdraid10,r3,.dbr100
        cmpobe  rdraid5,r3,.dbr500      # If RAID 5, use this algorithm
#
        setbit  eprecbadtype,g6,g6      # Get ready to log this error code
        b       .dbr810                 # Rebuilding non-rebuildable, toss it
#
# --- Rebuild a RAID 1 or 10 device -----------------------------------
#
.dbr100:
#
# --- This lock prevents the RAID deletion while we are doing a
# --- pass through the rebuild.
#
        call    rb$clr_rlock            # Clear rebuild lock
        call    M$gpdelay               # Give up control
        call    rb$set_rlock            # Set rebuild lock
#
        ldob    rbr_status(r15),r4      # Check latest status
        cmpobe  strbcancel,r4,.dbr800   # Jif RBR is cancelled
#
        ldos    rbr_rid(r15),r5         # Get RAID ID
        ld      R_rddindx[r5*4],r5      # Get RDD for the PSD in question
        cmpobne  0,r5,.dbr110           # JIF valid RDD
        call    .err10                  # Fault if zero.  This cannot happen
#
.dbr110:
        ldob    pd_devstat(r12),r5      # Get status of target device
        setbit  eprecinop,g6,g6         # Get ready to log this error code
        cmpobne pdop,r5,.dbr700         # If device has failed, retry later
        clrbit  eprecinop,g6,g6         # Not this error type

        setbit  eprecbusy,g6,g6         # Get ready to log this error code
        ldob    pd_flags(r12),r5
        bbs     pdbebusy,r5,.dbr700     # If device is BUSY, retry later
        clrbit  eprecbusy,g6,g6         # Not this error type
#
# --- Verify ownership in case the VDisk has been remapped
#
        ldos    rd_vid(r13),g0          # Pass VID
#
        PushRegs(r3)                    # Save regs
        call    DL_AmIOwner             # Determine if we are the owner or not
        PopRegs(r3)                     # Restore regs
#
        setbit  eprecowner,g6,g6        # Get ready to log this error code
        cmpobe  FALSE,g0,.dbr810        # Jif we don't own this RAID
        clrbit  eprecowner,g6,g6        # Not this error type
#
# --- This needs to have work done if we ever get to the point to pass in
# --- the LSW of the addresses and lengths. (LONG MATH REQ)
#
# --- Find a good mirror of stripe to rebuild.
#
        ld      rd_sps(r13),r3          # Sectors per stripe
c       *((UINT64*)&r4) = ((PSD*)r14)->rLen / r3; # Starting Stripe within device
        ldos    rd_psdcnt(r13),r10      # Physical devices per RAID
c       *((UINT64*)&r4) = (r10 * *((UINT64*)&r4)) + r11;
        ldob    rd_depth(r13),r8
c       *((UINT64*)&r4) = *((UINT64*)&r4) / r8; # r4 = stripe to rebuild
c       *((UINT64*)&r6) = *((UINT64*)&r4) * r3; # lsda of this stripe
c       g7 = r6;                        # NEED TO COMBINE THESE LATER
c       g5 = r7;                        # NEED TO COMBINE THESE LATER

c       r3 = (*((UINT64*)&r4) * r8) % r10; # r3 = psdstart
        ld      rd_psd(r13)[r3*4],r10   # Get PSD to start with
        mov     r10,g13                 # g13=stripe first psd
# r8=depth
# r10=PSD to start with (and g13=psd to stop when matching)
#
# --- Find first PSD in stripe that is operable
#
.dbr150:
        cmpobe  r10,r14,.dbr160         # Jif this PSD is being rebuilt
#
        ldos    ps_pid(r10),r3          # Get PDD
        ld      P_pddindx[r3*4],r3
        ldob    pd_devstat(r3),r5       # Get status of this device
        cmpobne pdop,r5,.dbr160         # If not an OK device, keep looking
        ldob    pd_flags(r3),r5
        bbs     pdbebusy,r5,.dbr160     # If device is BUSY, keep looking
#
        ldob    ps_status(r10),r3
        cmpobe  psop,r3,.dbr170         # If device and psd are good, OK mirror
#
.dbr160:
        ld      ps_npsd(r10),r10        # Point to next PSD
        lda     -1(r8),r8               # One less PSD in stripe remains
#
        cmpobne 0,r8,.dbr150            # Jif more PSDs to try - check next PSD
        b       .dbr700                 # None operable, try again later
#
# --- We have a source and target PSD, read the stripe, then write it
#
.dbr170:
#
# --- r10 = PSD to use for read
#
# --- Read a stripe from the selected mirror ---
#
        ldos    ps_pid(r10),g3          # Get PDD
        ld      P_pddindx[g3*4],g3
#
# --- Assign ILT/PRP
#
c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       g2 = get_prp();                 # Assign PRP
.ifdef M4_DEBUG_PRP
c CT_history_printf("%s%s:%u get_prp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_PRP
        st      g2,il_w0(g1)            # Link PRP to ILT
#
# --- Initialize PRP
#
        movq    0,g8                    # Clear out PRP
        stq     g8,pr_func(g2)
        stq     g8,pr_sda(g2)
        stq     g8,pr_sglptr(g2)
        stq     g8,pr_cmd(g2)
        stq     g8,pr_sense(g2)
        stq     g8,pr_sense+16(g2)
#
        ldob    pd_channel(g3),r3       # Set up SCSI channel
        stob    r3,pr_channel(g2)
#
        ldos    pd_lun(g3),r3           # Set up SCSI Lun
        stos    r3,pr_lun(g2)
#
        ld      pd_id(g3),r3            # Set up SCSI ID
        st      r3,pr_id(g2)
#
        ld      pd_dev(g3),r3            # Set up DEVice
        st      r3,pr_dev(g2)
#
        ldconst BTIMEOUT,r3              # Set up timeout
        st      r3,pr_timeout(g2)
#
        ldob    D_gpri,r3               # Get global priority
        ldob    D_gp2strat[r3*1],r3     # Lookup strategy
        stob    r3,pr_strategy(g2)
#
        ldconst prinput,r3              # Set up function
        stob    r3,pr_func(g2)
#
        ldconst IORETRY<<8,r4           # Clear SLI and setup retry count
        stos    r4,pr_flags(g2)
#
        ldconst 0,r4
        stos    r4,pr_qstatus(g2)       # Clear the Q-Logic status
#
!       ldl     ps_sda(r10),r4          # r4/r5
c       *((UINT64*)&r4) += ((PSD*)r14)->rLen; # This is the base address
        ld      rd_sps(r13),r3          # May or may not add or subtract this...
        mov     r10,g11                 # Source PSD
        mov     r14,g12                 # Target PSD
        ld      rd_psd(r13),g8          # First PSD of list
        ldob    rd_depth(r13),g10
        call    rb$checkpsdorder
        cmpobne 1,g8,.dbr190            # If 1, psd's are split across step
#
        cmpobne 1,g9,.dbr180
#
# --- If this PSD is before the break, and dest is after,
#
c       *(UINT64*)&r4 = *(UINT64*)&r4 - r3;
        b       .dbr190
#
# --- If this PSD is after the break, and dest is before,
#
.dbr180:
c       *(UINT64*)&r4 = *(UINT64*)&r4 + r3;
#
.dbr190:
c       ((PRP*)g2)->sda = *(UINT64*)&r4; # Set up starting disk address
c   if (((*(UINT64*)&r4) & ~0xffffffffULL) != 0ULL) { # 16 byte command
        stob    16,pr_cbytes(g2)        # Set up SCSI command length
        stob    0x88,pr_cmd(g2)         # read-16
c       *((UINT64*)&r6) = bswap_64(((PRP*)g2)->sda);
        stq     r6,pr_cmd+2(g2)         # save LBA in command
        ld      rd_sps(r13),r3
        bswap   r3,r3
        st      r3,pr_cmd+10(g2)        # Store read length
c   } else {                            # 10 byte command
        stob    10,pr_cbytes(g2)        # Set up SCSI command length
        stob    0x28,pr_cmd(g2)         # read-10
c       r3 = bswap_32(((PRP*)g2)->sda);
        st      r3,pr_cmd+2(g2)         # save LBA in command
        ld      rd_sps(r13),r3
        shlo    16,r3,r3                # Move to upper two bytes
        bswap   r3,r3
        stos    r3,pr_cmd+7(g2)         # Store read length
c   }
        ld      rd_sps(r13),r3
c       ((PRP*)g2)->eda = *(UINT64*)&r4 + r3; # Store ending disk address
#
        ld      rd_sps(r13),r4
        ldconst SECSIZE,r3
        mulo    r3,r4,r4                # This many bytes
        st      r4,pr_rqbytes(g2)
        mov     r4,g0
#
# --- Allocate combined SGL/buffer
#
c       g0 = m_asglbuf(g0);             # allocate a SGL and buffer
        st      g0,pr_sglptr(g2)        # Link SGL to PRP
#
        ld      sg_size(g0),r3          # Set up size of SGL
        setbit  31,r3,r3                # Indicate as borrowed
        st      r3,pr_sglsize(g2)
#
        PushRegs                        # Save all G registers (stack relative)
        ldos    ps_rid(r14),g0          # Set up raid id
        ld      rd_sps(r13),g1          # Set up number of sectors per stripe
c       g2 = g7;
c       g3 = g5;                        # recombine g7&g5 into 64 bit value
c       r_blklock(g0, g1, *(UINT64*)&g2);   # NOTE: g2/g3 = lsda
        PopRegsVoid                     # Restore all G registers (stack relative)
#
        call    O$quereq                # Queue the request
#
        call    M$chkstat               # Check the status of the device read
        cmpobne ecok,g0,.dbr400         # Jif error
#
# --- Write the stripe to the target drive ---
#
        mov     r12,g3                  # Get PDD
        ldob    pd_channel(g3),r3       # Set up SCSI channel
        stob    r3,pr_channel(g2)
#
        ldos    pd_lun(g3),r3           # Set up SCSI Lun
        stos    r3,pr_lun(g2)
#
        ld      pd_id(g3),r3            # Set up SCSI ID
        st      r3,pr_id(g2)
#
        ld      pd_dev(g3),r3           # Set up DEVice
        st      r3,pr_dev(g2)
#
        ldconst BTIMEOUT,r3
        st      r3,pr_timeout(g2)
#
        ldconst proutput,r3             # Set up function
        stob    r3,pr_func(g2)
#
#
        ldconst IORETRY<<8,r4           # clear SLI and setup retry count
        stos    r4,pr_flags(g2)
#
        ldconst 0,r4
        stos    r4,pr_qstatus(g2)       # Clear the Q-Logic status
#
c       ((PRP*)g2)->sda = ((PSD*)r14)->sda + ((PSD*)r14)->rLen;

c   if ((((PRP*)g2)->sda & ~0xffffffffULL) != 0ULL) { # 16 byte command below
        stob    16,pr_cbytes(g2)        # SCSI command length
        stob    0x8a,pr_cmd(g2)         # write-16
c       *((UINT64*)&r4) = bswap_64(((PRP*)g2)->sda);
        stq     r4,pr_cmd+2(g2)         # Patch LBA into command
        ld      rd_sps(r13),r3          # Write this many sectors (one stripe)
        bswap   r3,r3
        st      r3,pr_cmd+10(g2)        # Store write-16 length
c   } else {                            # 10 byte command below
        stob    10,pr_cbytes(g2)        # SCSI command length
        stob    0x2a,pr_cmd(g2)         # write-10
c       r4 = ((PRP*)g2)->sda;
        bswap   r4,r3
        st      r3,pr_cmd+2(g2)         # Patch LBA into command
#
        ld      rd_sps(r13),r3          # Write this many sectors (one stripe)
        shlo    16,r3,r3                # Move to upper two bytes
        bswap   r3,r3
        stos    r3,pr_cmd+7(g2)         # Store write length
c   }
#
        ld      rd_sps(r13),r3
c       ((PRP*)g2)->eda = r3 + ((PRP*)g2)->sda; # Store ending disk address
#
        call    O$quereq                # Queue the request
        call    M$chkstat               # Check the status of the device write
        cmpobne ecok,g0,.dbr410         # Jif error
#
        call    O$relreq                # Release the request buffer
#
        ldos    ps_rid(r14),g0
c       r_blkunlock(g0);
#
# --- Update the RBR and PSD remaining rebuild lengths for the good path
# --- Add to ps_rlen and subtract from rbr_rbremain and pd_rbremain
#
        ldob    rbr_status(r15),r3      # Check latest status
        cmpobe  strbcancel,r3,.dbr800   # Jif RBR is cancelled
#
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
c       ((PSD*)r14)->rLen += ((RDD*)r13)->sps; # Inc rebuild length by one stripe
#
c       ((RBR*)r15)->rlen -= ((RDD*)r13)->sps; # Remaining # of blocks.
#
# --- Check for the subtract value being larger than the current value.
# --- If it is, just zero the rebuild remaining count.
#
        ld      rd_sps(r13),r3          # Size of one stripe ...
        ldl     pd_rbremain(r12),r6     # r6 = remaining # sectors to go
c       if (r3 < (*(UINT64*)&r6)) {
c           *(UINT64*)&r6 -= r3;        # Update counter
c       } else {
c           *(UINT64*)&r6 = 0;          # Zero it.
c       }
        stl     r6,pd_rbremain(r12)     # Save remaining # of blocks
#
        PushRegs(r3)                    # Save registers
        mov     r12,g0
        call    RB_CalcPercentRemaining # Calculate the percent remaining
        PopRegsVoid(r3)
#
# --- Do another stripe or else this RBR is done
#
        ldl     ps_rlen(r14),r6         # Get rebuilt length
        ldl     ps_slen(r14),r8         # Get total length of segment
        cmpobne r9,r7,.dbr100           # Not done, do another stripe
        cmpobne r8,r6,.dbr100           # Not done, do another stripe
        b       .dbr600                 # Done with this RBR
#
# --- RAID 1/10 I/O error to device during read operation
#
.dbr400:
        call    O$relreq                # Release the request that failed
        ldos    ps_rid(r14),g0          # Get RID
c       r_blkunlock(g0);
        b       .dbr700                 # Delay & retry rebuild again
#
# --- RAID 1/10 I/O error to device during write operation
#     Try to hot spare this device
#     If any psds are pending to be rebuilt they will automatically
#     reflect this failure.
#
.dbr410:
        cmpobe ecbebusy,g0,.dbr400      # Jif not ISE BUSY
#
        ldob    rbr_status(r15),r4      # Check latest status
        cmpobe  strbcancel,r4,.dbr420   # Jif RBR is cancelled
        ldos    ps_rid(r14),g0          # Get RID
        mov     g1,r3                   # Save ILT pointer
        ld      R_rddindx[g0*4],g1      # Pass RDD
        mov     g1,r5                   # Save RDD for RB$rerror_que call
        mov     r14,g0                  # Pass PSD
        call    RB$rerror               # Flag error in this PDD, g2=PRP
        mov     r3,g1                   # Restore ILT pointer
.dbr420:
        call    O$relreq                # Release the request that failed
        ldos    ps_rid(r14),g0          # Pass RID
c       r_blkunlock(g0);
#       r4 is rbr_status, loaded above.
        cmpobe  strbcancel,r4,.dbr800   # Jif RBR is cancelled
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
.if 1  #VIJAY_MC
        st      r3,rr_flags(g2)
.endif  # 1
        st      r3,il_w5(g1)            # Pass NVA entry = 0
        stos    r4,rr_func(g2)          # A verify op failed
        st      r5,il_w4(g1)            # Pass RDD
        lda     RB$rerror_comp,r3       # Pass generic completion routine
        st      r3,il_cr(g1)
        call    RB$rerror_que           # Queue this ILT to error handler
#
        setbit  eprecwrfault,g6,g6      # Log this error code
        b       .dbr810                 # PSD not rebuilding-
                                        #  discard this rebuild request
#
# --- RAID 5 ----------------------------------------------------------
#
#     r15 = RBR being processed
#     r14 = PSD for that RBR
#     r13 = RDD for that RBR
#     r12 = PDD for that RBR
#     r11 = ordinal for this PSD
#
.dbr500:
        call    rb$clr_rlock            # Clear rebuild lock
        call    M$gpdelay               # Give up control
        call    rb$set_rlock            # Set rebuild lock
#
# --- Following an unlock and a lock operation, the RBR may have been
# --- deleted.  This lock prevents the deletion while we are doing a
# --- pass through the rebuild.  Check the head of the rebuild list.  If
# --- the pointer to the current RBR (r15) is not equal to the head of the
# --- rebuild list (RB_rbrhead) then the RAID was deleted and we should
# --- exit without updating any statistics since the RBR was deleted and
# --- the deletion code took care of the statistics.
#
        ld      RB_rbrhead,r5           # Get the head of the queue
        cmpobne r15,r5,.dbr900          # Exit since the RAID was deleted
#
        ldob    rbr_status(r15),r4      # Check latest status
        cmpobe  strbcancel,r4,.dbr800   # Jif RBR is cancelled
#
        ldos    rbr_rid(r15),r5         # Get RAID ID
#
        ld      R_rddindx[r5*4],r5      # Get RDD for the PSD in question
        cmpobe  0,r5,.err10             # This can't ever be 0 - error trap
#
        setbit  eprecnotready,g6,g6     # Get ready to log this error code
        ldob    rd_status(r13),r5       # Get status for this raid device
        cmpobg  rdop,r5,.dbr700         # If device status is < operative, retry later
        clrbit  eprecnotready,g6,g6     # Clear that error type
#
# --- Verify ownership in case the VDisk has been remapped
#
        ldos    rd_vid(r13),g0          # Pass VID
#
        PushRegs(r3)                    # Save regs
        call    DL_AmIOwner             # Determine if we are the owner or not
        PopRegs(r3)                     # Restore regs
#
        setbit  eprecowner,g6,g6        # Get ready to log this error code
        cmpobe  FALSE,g0,.dbr810        # Jif we don't own this RAID
        clrbit  eprecowner,g6,g6        # Not this error type
#
        setbit  eprecinop,g6,g6         # Get ready to log this error code
        ldob    pd_devstat(r12),r5      # Get status of target device
        cmpobne pdop,r5,.dbr700         # If device has failed, retry later
#
        ldob    ps_status(r14),r5       # Get status of target device
        cmpobne psrebuild,r5,.dbr810    # If not being rebuilt, target device
                                        #  must have died.
        clrbit  eprecinop,g6,g6         # Clear this error type
#
# --- Generate and issue rebuild request
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
        mov     0,r5                    # Clear SGL link
        st      r5,rr_sglptr(g2)
        st      r5,rr_sglsize(g2)
.if 1  #VIJAY_MC
        st      r5,rr_flags(g2)
.endif  # 1

        ldconst rrrebuild,r5            # Rebuild function
        stos    r5,rr_func(g2)
#
        ldob    D_gpri,r5               # Get global priority
        ldob    D_gp2strat[r5*1],r5     # Lookup strategy
        stob    r5,rr_strategy(g2)
#
        ldos    ps_rid(r14),r5
        stos    r5,rr_rid(g2)           # This RAID id
        ld      rd_spu(r13),r8
        st      r8,rr_rlen(g2)          # Length in sectors (one stripe)
#
        ld      rd_sps(r13),r8
c       *((UINT64*)&r8) = ((PSD*)r14)->rLen / r8;
        ldos    rd_psdcnt(r13),r3
c       *((UINT64*)&r8) = (r3 * *((UINT64*)&r8)) + r11; # Add in ordinal of this psd
        ldob    rd_depth(r13),r3
c       *((UINT64*)&r8) = *((UINT64*)&r8) / r3; # r8 = stripe to rebuild
        ld      rd_spu(r13),r6
c       *((UINT64*)&r5) = r6 * *((UINT64*)&r8); # Make a starting disk address
        stl     r5,rr_rsda(g2)          # Set starting disk address
#
        lda     R$que,g0
        call    K$qw                    # Queue the request with wait
#
# --- Check status of rebuild request
#
        ldob    rr_status(g2),r5
        call    M$rir                   # Release the request
        cmpobe  ecok,r5,.dbr520         # Jif no error
#
        ldob    ps_status(r14),r3       # r3 = PSD status
        setbit  eprecr5inop,g6,g6       # Get ready to log this error code
        cmpobne psrebuild,r3,.dbr810    # Terminate rebuild if PSD <> rebuild
        clrbit  eprecr5inop,g6,g6       # Clear this error type
        b       .dbr700                 # Delay and restart rebuild operation
#
# --- Update the rebuild record and the device table.
#
.dbr520:
        ldob    rbr_status(r15),r4      # Check latest status
        cmpobe  strbcancel,r4,.dbr800   # Jif RBR is cancelled
#
        movl    g2,r4                   # Save regs for division
        movl    g4,r6
#
# --- rLen has the amount of the PSD that was rebuilt before we did
# --- the last rebuild operation.  The PSD now has the current amount.
# --- The difference is the amount done on the last operation.
#
        ldl     ps_rlen(r14),g2         # Get rebuilt length
c       *((UINT64*)&g2) = *((UINT64*)&g2) - ((PSD*)r14)->rLen;    # Amount of the last op
#
        ldl     rbr_rbremain(r15),g4    # g4/g5 = remaining # of blocks
c       *((UINT64*)&g4) = *((UINT64*)&g4) - *((UINT64*)&g2);
        stl     g4,rbr_rbremain(r15)    # Save remaining # of blocks
#
# --- Check for wraparound subtraction.  If we are trying to subtract more
# --- than is in the count, just set it to zero.
#
        ldl     pd_rbremain(r12),g4     # g4/g5 = total # sectors remaining
c   if (*((UINT64*)&g2) < *((UINT64*)&g4)) {
c       *((UINT64*)&g4) = *((UINT64*)&g4) - *((UINT64*)&g2);
c   } else {
c       *((UINT64*)&g4) = 0;            # zero it
c   }
        stl     g4,pd_rbremain(r12)     # Save remaining # of blocks
#
        PushRegs(r3)                    # Save registers
        mov     r12,g0
        call    RB_CalcPercentRemaining # Calculate the percent remaining
        PopRegsVoid(r3)
#
        movl    r4,g2                   # Restore regs from division
        movl    r6,g4
#
        ldl     ps_slen(r14),r6         # Get total length of segment
        ldl     ps_rlen(r14),r4
        cmpobne r4,r6,.dbr500           # Not done, do another segment
        cmpobne r5,r7,.dbr500           # Not done, do another segment
#
# --- Common RAID code ------------------------------------------------
#
#
# --- Rebuild completed successfully
#
.dbr600:
        movl    0,r6
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        stl     r6,ps_rlen(r14)         # Reset rebuild length
        ldconst psop,r7
        stob    r7,ps_status(r14)       # Set status to operable
        mov     FALSE,r6                # Clear previous boot fail
        stob    r6,ps_pbfail(r14)
#
        ldob    ps_astatus(r14),r6
        clrbit  psarebuild,r6,r6        # Indicate rebuild done on this PSD
        clrbit  psadefrag,r6,r6         # Indicate defrag done on this PSD
        stob    r6,ps_astatus(r14)
#
#
# ---  Clear the Raid rebuild astatus, then loop through the
#      PSDs for this Raid. Set if any PSDS still need rebuilding.
#
        ldob    rd_astatus(r13),r6
        clrbit  rdarebuild,r6,r6        # Indicate rebuild done on this RAID
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        stob    r6,rd_astatus(r13)
        ldos    rd_psdcnt(r13),r3       # Get number of PSD's to process
        ld      rd_psd(r13),r4          # Get 1st PSD
#
# --- Examine next PSD
#
.dbr610:
#
        ldob    ps_astatus(r4),r6       # Get PSD additional status
        bbc     psarebuild,r6,.dbr620
#
        ldob    rd_astatus(r13),r6
        setbit  rdarebuild,r6,r6        # Indicate rebuild still required
        stob    r6,rd_astatus(r13)
        b       .dbr630
#
# --- Advance to next PSD
#
.dbr620:
        ld      ps_npsd(r4),r4          # Link to next PSD
        subo    1,r3,r3                 # Adjust remaining PSD count
        cmpobne 0,r3,.dbr610            # Jif more
#
.dbr630:
#
.if     DEBUG_FLIGHTREC_OHSPARE
        ldconst frt_h_rbld4,r3          # Rebuilding functions
        st      r3,fr_parm0             # Function - rb$psd_rebuilder done
        ldos    ps_rid(r14),r3          # RID
        ldos    ps_pid(r14),r4          # PID
        shlo    16,r3,r3
        or      r3,r4,r3
        st      r3,fr_parm1             # RID/PID
        st      r14,fr_parm2            # PSD being rebuilt
        st      r15,fr_parm3            # RBR
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_OHSPARE
#
        mov     r13,g1                  # Get RDD for the PSD in question
        call    RB_setraidstat          # Update attached RAID status
        call    RB_setvirtstat          # Updated virtual devices status
#
        ldconst eprecok,g6              # No error on this completion
        b       .dbr900                 # Do another RBR.  No stats update reqd
#
# --- Can't complete this rebuild right now, attach this RBR to the end of the
# --- list and wait for a while
#
.dbr700:
        ldob    rbr_status(r15),r4      # Check latest status
        cmpobe  strbcancel,r4,.dbr800   # Jif RBR is cancelled
#
# --- Clear out the fence.
#
        ld      rbr_psd(r15),r14        # Get the PSD
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
c       ((PSD*)r14)->rLen = 0;          # Clear the rebuild length (fence)
#
        ldconst strbdelay,r4
        stob    r4,rbr_status(r15)      # RBR status = delay
        ldconst rbrdelay,r4
        stob    r4,rbr_delaytime(r15)   # Reset the delay time
        ld      rbr_next(r15),r4        # Get next record (if there is one)
        cmpobe  0,r4,.dbr730            # If only record remaining, just wait
        st      r4,RB_rbrhead           # Store into root
#
.dbr710:
        ld      rbr_next(r4),r5         # Point to next record
        cmpobe  0,r5,.dbr720            # If end of list, set up pointers
        mov     r5,r4
        b       .dbr710
#
.dbr720:
        st      r5,rbr_next(r15)        # Make this the last record
        st      r15,rbr_next(r4)        # Point last record to this one
#
# --- Log a warning if the rebuild is stuck too long
#
.dbr730:
        ld      rbr_delaylogtime(r15),r3 # Get time remaining before delay log
        cmpobne 0,r3,.dbr990            # Jif the delay count is not expired
        ldconst rbrdelaylog,r4
        st      r4,rbr_delaylogtime(r15)# Reset the time until logging
#
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst (mlepsdrebuilddone|mlewarn),r3 # Get log code
        st      r3,mle_event(g0)        # Store as word to clear other bytes
        ldos    rbr_rid(r15),r3         # Get RID
        stos    r3,epr_rid(g0)          # Save it
        ldconst 0xffff,r4               # Assume bad VID
        ld      R_rddindx[r3*4],r3      # Get RDD
        cmpobe  0,r3,.dbr740            # Jif NULL
        ldos    rd_vid(r3),r4           # Get VID
#
.dbr740:
        stos    r4,epr_vid(g0)          # Save it
        ld      rbr_pdd(r15),r12        # Get PDD pointer
        ldos    pd_lun(r12),r3          # Get the LUN
        stos    r3,epr_lun(g0)          # Save it
        setbit  eprecdelay,g6,g6        # Set the long delay bit
        stos    g6,epr_errcode(g0)      # Save error code
        ldl     pd_wwn(r12),g2          # Get the WWN
        stl     g2,epr_wwn(g0)          # Save it
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], eprlen);
        b       .dbr990                 # Process RBR list again
#
# --- RBR was cancelled -----------------------------------------------
#
#     dbr800 is the entry point for RBRs that were cancelled from outside of
#     this function - like initializations, deletions, pause BE processes.
#     In this case the PDD counts have already been cleaned up and we just
#     need to set the cancel bit in the error code.
#
#     dbr810 is the entry point for RBRs that are cancelled within this function.
#     The PDD remaining counts have to be updated.
#
.dbr800:
        setbit  epreccancel,g6,g6       # Indicate a cancel in the error code
        b       .dbr900
#
.dbr810:
        mov     r15,g0
        call    rb$cancel_rbr           # Cancel the RBR and update PDD info
#
# --- Continue to next RBR --------------------------------------------
#
# --- Finished with this RBR. Detach and process another if it exists.
# --- Note that we may have dropped out due to a RAID deletion. In this
# --- case, the RBR pointer will no longer be on the head of the queue.
# --- If this is true, then skip the logging of the end of the rebuild
# --- and do not deque the RBR (since it is gone).
#
.dbr900:
        ld      rbr_pdd(r15),r12        # Grab the PDD in case a cancel was done
#
        ld      rbr_next(r15),r4        # Get pointer to next RBR
        st      r4,RB_rbrhead           # If we finished the last element, r4=0
#
# --- Clear out the fence.
#
        ldos    rbr_rid(r15),r3         # Get RID
        ld      R_rddindx[r3*4],r3      # Get RDD
        cmpobe  0,r3,.dbr910            # Jif NULL
#
        ld      rbr_psd(r15),r14        # Get the PSD
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
c       ((PSD*)r14)->rLen = 0;          # Clear the rebuild length (fence)
#
# --- Log the completion of the RAID/PSD rebuild.
#     Log cancels/deleted/badtype/owner/done as 'debug' instead of 'errors'
#
.dbr910:
        ldconst mlepsdrebuilddone+mleinform,r3 # Get log code - default to info
        cmpobe  0,g6,.dbr912            # Save event type if completed good
#
        ldconst mlepsdrebuilddone+mledebug,r3 # Change type to debug
        bbs     eprecbadtype,g6,.dbr912 # Hide these from user
        bbs     eprecdeleted,g6,.dbr912
        bbs     epreccancel,g6,.dbr912
        bbs     eprecowner,g6,.dbr912
        bbs     eprecdone,g6,.dbr912
#
        ldconst mlepsdrebuilddone+mleerror,r3 # Change type to error
#
# --- Check if this was a defrag initiated operation.  If so, log the defrag
# --- step completed.
#
.dbr912:
        ldob    pd_miscstat(r12),r4     # Get misc status
        bbc     pdmbdefragging,r4,.dbr913 # Jif not a defrag
#
        PushRegs(r4)
        ldos    pd_pid(r12),g0          # PID
        ldos    rbr_rid(r15),g1         # RID
        mov     g6,g2                   # Error code
        call    DF_LogDefragDone        # Log it
        PopRegs(r4)
        b       .dbr930                 # Continue
#
.dbr913:
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        st      r3,mle_event(g0)        # Store as word to clear other bytes
        ldos    rbr_rid(r15),r3         # Get RID
        stos    r3,epr_rid(g0)          # Save it
        ldconst 0xffff,r4               # Assume bad VID
        ld      R_rddindx[r3*4],r3      # Get RDD
        cmpobe  0,r3,.dbr920            # Jif NULL
        ldos    rd_vid(r3),r4           # Get VID
#
.dbr920:
        stos    r4,epr_vid(g0)          # Save it
        ldos    pd_lun(r12),r3          # Get the LUN
        stos    r3,epr_lun(g0)          # Save it
        stos    g6,epr_errcode(g0)      # Save error code
        ldl     pd_wwn(r12),g2          # Get the WWN
        stl     g2,epr_wwn(g0)          # Save it
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], eprlen);
#
.dbr930:
.if     DEBUG_FLIGHTREC_OHSPARE
        ldconst frt_h_rbld5,r3          # Rebuilding functions
        st      r3,fr_parm0             # Function - rb$psd_rebuilder psd done
        ld      rbr_psd(r15),r4         # PSD
        st      r4,fr_parm2             # PSD being rebuilt
        ldos    ps_rid(r4),r3           # RID
        ldos    ps_pid(r4),r4           # PID
        shlo    16,r3,r3
        or      r3,r4,r3
        st      r3,fr_parm1             # RID/PID
        st      g6,fr_parm3             # Error code
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_OHSPARE
#
# --- Need to check if any more rebuild operations associated with
# --- the device of the operation that just completed.
#
        ldl     pd_rbremain(r12),r6     # Get remaining block count
        or      r6,r7,r7                # Check for zero
        cmpobne 0,r7,.dbr980            # Jif not done with this device
#
# --- No more blocks to process for the PDD just finished.
# --- Check the status of the device and modify it to operable
# --- if the status was rebuilding.
#
        ldob    pd_miscstat(r12),r5     # r5 = current misc status
        PushRegs(r3)                    # Save registers
        call    DEF_UMiscStat           # Update PDD misc status
        PopRegsVoid(r3)                 # Restore registers
        bbc     pdmbrebuilding,r5,.dbr980 # Log only once
        bbs     pdmbdefragging,r5,.dbr980 # Do not log for defrag
c       RB_ReInitUsedHotSpare ((PDD*)r12);
#
# --- Log the completion of the PID rebuild.
#
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst mlepddrebuilddone,r3    # Get log event code
        st      r3,mle_event(g0)        # Store as word to clear other bytes
        ldconst 0,r3                    # Clear reserved
        st      r3,mle_bstream(g0)
        ldos    pd_lun(r12),r3          # LUN
        stos    r3,eph_lun(g0)
        ldos    pd_pid(r12),r3          # PID
        stos    r3,eph_pid(g0)
        ldl     pd_wwn(r12),g2          # WWN
        stl     g2,eph_wwn(g0)
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], ephlen);
#
.if     DEBUG_FLIGHTREC_OHSPARE
        ldconst frt_h_rbld6,r3          # Rebuilding functions
        st      r3,fr_parm0             # Function - rb$psd_rebuilder PID done
        ldos    pd_pid(r12),r3
        st      r3,fr_parm1             # PID being rebuilt
        st      r12,fr_parm2            # PDD
        st      r15,fr_parm3            # RBR
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_OHSPARE
#
# --- Release the RBR and update the NVRAM if the RBR is still valid.
#     Should any of the other error types also be avoided here?
#
.dbr980:
        bbs     eprecowner,g6,.dbr985   # Don't update for these endings
        call    D$p2update              # Update NVRAM on devices
#
.dbr985:                                # Entry point for cancelled RBRs
        cmpobe  0,r15,.dbr990           # Jif RBR already gone
c       s_Free(r15, rbrsiz, __FILE__, __LINE__);
#
# --- Always unlock before we jump back to the top
#
.dbr990:
        call    rb$clr_rlock            # Clear rebuild lock
        call    K$xchang                # Allow other processes to run
        b       rb$psd_rebuilder        # Go check if there are more to do
#
# --- Exit
#
.dbr1000:
        ldconst 0,g0                    # clear the rebuilder task
        st      g0,gPSDRebuilderPCB
        #
        # Update the state of the raids rebuild before write flags
        #
        ldconst TRUE,g0                 # Remote Cache update required
        ldconst TRUE,g1                 # Force P2 update when completed
        PushRegs(r4)                    # Save regs
        call    RB_UpdateRebuildWriteState
        PopRegsVoid(r4)                 # Restore regs
        call    RB_searchforfailedpsds  # One last shot to clean up any drives
                                        # that weren't rebuilt.
                                        # Always exit - task will be restarted
                                        # if needed.
        ret
#
#**********************************************************************
#
#  NAME: rb$checkpsdorder
#
#  PURPOSE:
#       checks the order of the source and target psd's in a rebuilding
#       stripe to determine if addresses need to be offset for read
#
#  DESCRIPTION:
#       if the source is before a stripe break, and dest is after, or vice
#       versa returns a true condition
#
#  CALLING SEQUENCE:
#       call    rb$checkpsdorder
#
#  INPUT:
#       g8=first psd,
#       g10=depth,
#       g11=source psd,
#       g12=target psd,
#       g13=first psd of stripe
#
#  OUTPUT:
#       if source and target are not on same address, g8=1 else g8=0
#       if target after source and different addresses, g9=1, else g9=0
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
rb$checkpsdorder:
        ldconst 0,r7
#
# --- Find last PSD of stripe
#
        mov     g13,r8
#
.dpc50:
        subo    1,g10,g10               # Remove one from count
        cmpobe  0,g10,.dpc100
#
        ld      ps_npsd(r8),r8          # Traverse to next PSD
        b       .dpc50
#
# --- Traverse PSD list, looking for wraparound of list
#
.dpc100:
        cmpobe  g13,g8,.dpc1000         # End of check, pointing to first PSD
        cmpobe  g13,g12,.dpc600         # Found target prior to first psd
#
.dpc300:
        cmpobe  g13,g11,.dpc700         # Found source prior to first psd
#
.dpc400:
        cmpobe  3,r7,.dpc1000           # If source and target found, we can quit
        cmpobe  g13,r8,.dpc1000         # End of check (last PSD of stripe)
#
        ld      ps_npsd(g13),g13        # Point to next PSD
        b       .dpc100
#
# --- Found the target PSD
#
.dpc600:
        addo    1,r7,r7
        b       .dpc300                 # Found target
#
# --- Found the source PSD
#
.dpc700:
        addo    2,r7,r7                 # found source
        b       .dpc400
#
# --- Found a break, or end of stripe, decide if target and source are split
#
.dpc1000:
        cmpobe  1,r7,.dpc1050           # Split, target first?
        cmpobe  2,r7,.dpc1090           # Split, source first?
#
        ldconst 0,g8
        mov     g8,g9
        b       .dpc10000
#
.dpc1050:                               # Split, Target prior, Source after break
        ldconst 1,g8                    # source
        ldconst 0,g9                    # target
        b        .dpc10000
#
.dpc1090:
        ldconst 0,g8                    # split, Target after, Source prior to break
        ldconst 1,g9
#
# --- Exit
#
.dpc10000:
        addo    g8,g9,g8
        ret
#
#**********************************************************************
#
#  NAME: RB$pause_rebld
#
#  PURPOSE:
#       To provide a common means of stopping all I/O activity invoked
#       through the rebuild process
#
#  DESCRIPTION:
#       The stop counter is incremented and a check is made for any
#       outstanding I/O.  When all outstanding I/O completes, this
#       routine returns to the caller.  While the stop counter is
#       non-zero, the executive is effectively blocked.
#
#       This routine may only be called from the process level.
#
#  CALLING SEQUENCE:
#       call    RB$pause_rebld
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
RB$pause_rebld:
        mov     g0,r15                  # Save g0
#
# --- Bump stop counter
#
        ldob    rb_stopcnt,r3           # Increment stop counter
        addo    1,r3,r3
        stob    r3,rb_stopcnt
#
# --- Stall until pending I/Os complete
#
.st10_r:
        ldob    rb_rlock,r4             # Get rebuild lock
        cmpobe  FALSE,r4,.st100_r       # Jif unlocked
#
        ldconst 1,g0                    # Delay for minimum time
        call    K$twait
        b       .st10_r
#
# --- Exit
#
.st100_r:
        mov     r15,g0                  # Restore g0
        ret
#
#**********************************************************************
#
#  NAME: RB$resume_rebld
#
#  PURPOSE:
#       To provide a common means of resuming all I/O activity invoked
#       through the rebuild process.
#
#  DESCRIPTION:
#       The stop counter is decremented and an immediate return is
#       made to the caller.  When this counter has returned to zero,
#       the rebuilder is unblocked.
#
#  CALLING SEQUENCE:
#       call    RB$resume_rebld
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
RB$resume_rebld:
#
# --- Adjust stop counter
#
        ldob    rb_stopcnt,r3           # Decrement stop counter
        subo    1,r3,r3
        stob    r3,rb_stopcnt
        ret
#
#**********************************************************************
#
#  NAME: rb$set_rlock
#
#  PURPOSE:
#       To provide a common means of setting the rebuild lock and
#       synchronizing with the rebuild procedure.
#
#  DESCRIPTION:
#       The stop counter is examined to determine if synchronization
#       has been requested.  If so, an 8 second delay occurs with
#       the stop counter being reexamined.  When the stop counter
#       is zero, the rebuild lock is set and this routine exits.
#
#  CALLING SEQUENCE:
#       call    rb$set_rlock
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
rb$set_rlock:
        mov     g0,r15                  # Save g0
#
.dsr10:
        ldob    rb_stopcnt,r3           # Get stop counter
        cmpobe  0,r3,.dsr20             # Jif clear
#
        ldconst 8000,g0                 # Wait for 8 seconds
        call    K$twait
        b       .dsr10
#
.dsr20:
        mov     TRUE,r4                 # Set rebuild lock
        stob    r4,rb_rlock
#
# --- Exit
#
        mov     r15,g0                  # Restore g0
        ret
#
#**********************************************************************
#
#  NAME: rb$clr_rlock
#
#  PURPOSE:
#       To provide a common means of clearing the rebuild lock.
#
#  DESCRIPTION:
#       The rebuild lock is unconditionally cleared and this routine
#       exits.
#
#  CALLING SEQUENCE:
#       call    rb$clr_rlock
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
rb$clr_rlock:
        mov     FALSE,r3                # Clear rebuild lock
        stob    r3,rb_rlock
        ret
#
#**********************************************************************
#
#  NAME: RB$cancel_rebld
#
#  PURPOSE:
#       To provide a common means of cancelling any potential rebuilds
#       based upon RAID ID.
#
#  DESCRIPTION:
#       The list of RBRs is searched to locate and release all records
#       pertaining to the specified RAID ID.
#
#  CALLING SEQUENCE:
#       call    RB$cancel_rebld
#
#  INPUT:
#       g0 = RAID ID
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
RB$cancel_rebld:
#
        mov     g0,r8                   # Save g0
#
# --- Fake out the starting address to get the next pointer into
# --- the right spot.  r3 will actually point rbr_next bytes in front
# --- of the RB_rbrhead variable.  This gets that address into the
# --- proper alignment for deletion of either the head of the queue or
#
        lda     RB_rbrhead-rbr_next,r7  # Get origin of rebuild list
#
# --- Examine next RBR for specified RAID ID
#
.bx10:
        ld      rbr_next(r7),r6         # Get next RBR
        cmpobe  0,r6,.bx100             # Jif none - return
#
        ldos    rbr_rid(r6),r5          # Get RAID ID
        cmpobne r8,r5,.bx30             # Jif not a match - do next RBR
#
        mov     r6,g0
        call    rb$cancel_rbr           # Cancel this RBR
#
# --- Loop to next RDD
#
.bx30:
        mov     r6,r7                   # Move current to previous
        b       .bx10                   # Loop again
#
# --- Exit
#
.bx100:
        mov     r8,g0                   # Restore g0
        ret
#
#**********************************************************************
#
#  NAME: rb$cancel_rbr
#
#  PURPOSE:
#       To provide a common means of cancelling a particular RBR.
#
#  DESCRIPTION:
#       The PDD rbremaining count is decremented by the number of blocks
#       remaining in the RBR. The PDD % rebuild is updated. The cancel
#       bit is also turned on in the RBR to prevent psd_rebuilder from using
#       the RBR in the future.
#
#  CALLING SEQUENCE:
#       call    rb$cancel_rbr
#
#  INPUT:
#       g0 = RBR
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
rb$cancel_rbr:
        mov     g1,r15                  # Save g1
#
# --- If the RBR is already cancelled, just exit
#
        ldob    rbr_status(g0),r5      # r5 = current RBR status
        cmpobe  strbcancel,r5,.crb1000 # Jif cancelled - don't process RBR
#
# --- Update remaining blocks to rebuild in PDD
        ld      rbr_pdd(g0),r8          # Get PDD pointer
#
        ldl     pd_rbremain(r8),r4      # PDD rebuild blocks remaining minus
        ldl     rbr_rbremain(g0),r6     # Remaining # of blocks
#
# --- Check for wrap around subtract and zero if we wrap.
#
c   if (*(UINT64 *)&r4 <= (*(UINT64 *)&r6)) {
c       *(UINT64 *)&r4 = 0;             # Else zero it
c   } else {
c       *(UINT64 *)&r4 = *(UINT64 *)&r4 - *(UINT64 *)&r6;
c   }
        stl     r4,pd_rbremain(r8)      # Equals PDD blocks remaining in g4/g5
#
        PushRegs(r3)                    # Save registers
        mov     r8,g0
        call    RB_CalcPercentRemaining # Calculate the percent remaining
        PopRegsVoid(r3)
#
# --- Indicate an abort in the RBR
#
        ldconst strbcancel,r6           # Cancel
        stob    r6,rbr_status(g0)       # Cancel this rebuild
#
.if     DEBUG_FLIGHTREC_OHSPARE
        ldconst frt_h_rbld7,r3          # Rebuilding functions
        st      r3,fr_parm0             # Function - rb$psd_rebuilder start
        ldos    rbr_rid(g0),r4
        shlo    16,r4,r4
        st      r4,fr_parm1             # RID
        ldconst 0,r3
        st      r3,fr_parm2
        st      g0,fr_parm3             # RBR
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_OHSPARE
#
.crb1000:
        mov     r15,g1                  # Restore g1
        ret
#
#**********************************************************************
#
#  NAME: rb$log_raid_error
#
#  PURPOSE:
#       To provide a common means of reporting that a drive error has
#       been detected at the RAID level.
#
#  DESCRIPTION:
#       The raid error log message is
#       constructed with information taken from the PSD and PDD.
#
#  CALLING SEQUENCE:
#       call    rb$log_raid_error
#
#  INPUT:
#       g0 = PSD
#       g1 = log type (debug or customer)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
rb$log_raid_error:
        mov     g0,r12                  # Save g0
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst mleraiderror,r4         # Event code
        or      g1,r4,r4                # Combine log type
        st      r4,mle_event(g0)        # Store as word to clear other bytes
#
        ldos    ps_rid(r12),r3          # RID
        stos    r3,ere_rid(g0)
        ld      R_rddindx[r3*4],r4      # RDD
        ldos    rd_vid(r4),r8           # r8 = VID
        ldos    ps_pid(r12),r7          # r7 = PID
        ld      P_pddindx[r7*4],r6      # PDD
        ldos    pd_lun(r6),r9           # r9 = LUN
        ldl     pd_wwn(r6),r4           # r4/r5 = WWN
        stos    r8,ere_vid(g0)
        stos    r9,ere_lun(g0)
        stos    r7,ere_pid(g0)
        stl     r4,ere_wwn(g0)
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], erelen);
        mov     r12,g0                  # Restore g0
        ret
#
#**********************************************************************
#
#  NAME: RB_LogHSDepleted
#
#  PURPOSE:
#       To report that there are no remaining hotspares.
#
#  DESCRIPTION:
#       The log message is constructed and sent to the CCB.
#
#  CALLING SEQUENCE:
#       RB_LogHSDepleted
#
#  INPUT:
#       g0 - Type.
#       g1 - Bay/pool ID.
#       g2 - Drive Type
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       G registers.
#
#**********************************************************************
#
RB_LogHSDepleted:
        mov     g0,r12                  # Save g0
#
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst mlehsdepleted,r4        # Event code
        st      r4,mle_event(g0)        # Store as word to clear other bytes
        stos    r12,hsd_type(g0)        # Save type (input in g0)
        stos    g1,hsd_dev(g0)          # Save pool or bay ID
        stob    g2,hsd_devType(g0)      # Save device type
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], hsdlen);
        mov     r12,g0                  # Restore g0
        ret
#
#**********************************************************************
#
#  NAME: rb$log_raid_event
#
#  PURPOSE:
#       To provide a common means of reporting debug logs for RAID
#       error handling, rebuilding, and hotsparing.
#
#  DESCRIPTION:
#       The raid error log message is
#       constructed with information taken from the PSD and PDD.
#
#  CALLING SEQUENCE:
#       call    rb$log_raid_event
#
#  INPUT:
#       g0 = debug type (short)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
rb$log_raid_event:
        mov     g0,r12                  # Save g0
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst mleraidevent,r4         # Event code
        st      r4,mle_event(g0)        # Store as word to clear other bytes
        stos    r12,erl_type(g0)        # Save the debug type
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], erllen);
        mov     r12,g0                  # Restore g0
        ret
#
#**********************************************************************
#
#  NAME: rb$log_parityscanrequired
#
#  PURPOSE:
#       To provide a common means of reporting that a parity scan is
#       required for the given RDD.
#
#  DESCRIPTION:
#       This is constructed with information taken from the RDD.
#
#  CALLING SEQUENCE:
#       call    rb$log_parityscanrequired
#
#  INPUT:
#       g10 = RDD
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
rb$log_parityscanrequired:
        mov     g0,r12                  # Save g0
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst mleparityscanrequired,r4 # Event code
        st      r4,mle_event(g0)        # Store as word to clear other bytes
        ldos    rd_rid(g10),r3          # RID
        stos    r3,eps_rid(g0)
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], epslen);
        mov     r12,g0                  # Restore g0
        ret

.ifndef MODEL_3000
.ifndef MODEL_7400
#**********************************************************************
#
#  NAME: r$log_Raidinop
#
#  PURPOSE:
#       Log a message that a RAID has gone Inoperative
#
#  DESCRIPTION:
#       Report the inoperativeness of a Raid to alert Service
#       to start working with the customer.
#
#  CALLING SEQUENCE:
#       call    r$log_Raidinop
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
r$log_Raidinop:
#
        ldos    rd_rid(g0),r5           # Get the RID from the RDD
        ldos    rd_vid(g0),r6           # Get the VID from the RDD
        mov     g0,r12                  # Save g0
#
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst mleRaidinop,r4          # Event code
        st      r4,mle_event(g0)        # Store as word to clear other bytes
        stos    r5,e_rinop_rid(g0)      # Store the RID in the Error Log
        stos    r6,e_rinop_vid(g0)      # Store the VID in the Error Log
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], e_rinop_len);
        mov     r12,g0                  # Restore g0
        ret
.endif  # MODEL_7400
.endif  # MODEL_3000
#
#
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
