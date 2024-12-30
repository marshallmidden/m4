# $Id: raid5.as 161041 2013-05-08 15:16:49Z marshall_midden $
#**********************************************************************
#
#  NAME: raid5.as
#
#  PURPOSE:
#       To implement RAID functionality as a layer directly above the
#       physical SCSI I/O layer. RAID levels 0, 1, 5 and 10 are
#       directly supported by this module.
#
#  FUNCTIONS:
#       This module employs these processes:
#
#       r$r5exec    - RAID 5 Executive (1 copy)
#       r$xexec     - XOR Initiation Executive (1 copy)
#       r$cexec     - XOR Completion Executive (1 copy)
#
#  Copyright (c) 1996-2010 Xiotech Corporation. All rights reserved.
#
#**********************************************************************
#
# --- global data declarations ----------------------------------------
#
        .globl  R_r5exec_qu             # Raid 5 executive QCB - for debug
#
# --- component function declarations ---------------------------------
#
        .globl  r$r5exec                # RAID 5 exec
        .globl  R$log_R5inop            # RAID 5 Inoperative Log Message
        .globl  R$recover_R5inop        # Recover a RAID 5 in the Inop Status
        .globl  R$checkForR5Rebuilding  # Check RAID for Rebuilding State
        .globl  R$checkForR5PSDRebuilding # Check for PSD in Rebuilding State
#
# --- local usage data definitions ------------------------------------
#

#
# --- executable code -------------------------------------------------
#
        .text
#**********************************************************************
#
#  NAME: r$r5exec
#
#  PURPOSE:
#       To provide a means of initiating physical RAID 5 I/O operations
#       for RAID 5 RRP requests that have been preoptimized and reduced
#       to one or more RRB requests.
#
#  DESCRIPTION:
#       The RPN initiation queue is accessed from the oldest to newest.
#       I/O is initiated for each node with those nodes being released
#       from the initiation queue. When all possible I/Os have been
#       initiated, this process deactivates.
#
#       Each RPN node represents I/O activity for a single stripe on a
#       RAID 5 device. I/O queued to an RPN through an RRB has already
#       been constrained to lie within the boundaries established by a
#       single stripe. This technique greatly simplifies the process
#       of keeping the RAID 5 parity coherent.
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
r$r5exec:
#
# --- Preload constants
#
        lda     R_r5exec_qu,r15         # Get queue structure pointer
        ld      qu_pcb(r15),r14         # Get executive process
        ldconst pcnrdy,r13              # Get not ready status
#
# --- Allocate R5S from DRAM
#
c       r12 = s_MallocC(r5ssiz|BIT31, __FILE__, __LINE__); # Assign R5S
#
# --- Exchange processes ----------------------------------------------
#
.rx10:
        call    K$qxchang               # Exchange processes
#
# --- Prepare for examination of RPN initiation queue
#
.rx15:
        ld      qu_head(r15),g3         # Get oldest RPN from q head
        cmpobne 0,g3,.rx20              # Jif found
#
# --- Deactivate executive --------------------------------------------
#
        stob    r13,pc_stat(r14)        # Set process not ready
        b       .rx10
#
# --- Attempt to initiate requests from oldest RPN
#
.rx20:
        ldob    rp_lock(g3),r3          # Get lock status
        cmpobe  TRUE,r3,.rx30           # Jif node locked
#
        call    r$deactrpn              # Deactivate RPN
        mov     r12,g14                 # Pass R5S
        call    r$initrpn               # Initiate from RPN
        b       .rx15
#
.rx30:
        ld      rp_afthd(g3),g3         # Link to next RPN node
        cmpobne 0,g3,.rx20              # Jif available
        b       .rx15
#
#**********************************************************************
#
#  NAME: r$deactrpn
#
#  PURPOSE:
#       To provide a means of deactivating a specific RPN so that the
#       RAID 5 Executive will not consider this node for I/O initiation.
#
#  DESCRIPTION:
#       This RPN is removed from the RAID 5 Executive initiation queue
#       and the queue count is decremented by one.
#
#  INPUT:
#       g3 = RPN
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
r$deactrpn:
#
# --- Check for active RPN
#
        ldob    rp_act(g3),r3           # Get active flag
        cmpobe  FALSE,r3,.de100         # Jif not active
#
# --- Remove RPN from RAID 5 Exec initiation queue
#
        ld      rp_afthd(g3),r4         # Get forward pointers
        ld      rp_abthd(g3),r5         # Get backward pointers
        mov     0,r14                   # Set up zero constants
        mov     0,r15
        lda     R_r5exec_qu,r13         # Get RAID 5 initiation queue
        cmpobe  0,r4,.de10              # Jif forward ptr null
        cmpobe  0,r5,.de30              # Jif backward ptr null
#
# --- Remove RPN from middle of thread
#
        st      r4,rp_afthd(r5)         # Remove RPN from middle of
        st      r5,rp_abthd(r4)         #  thread
        b       .de40
#
.de10:
        cmpobe  0,r5,.de20              # Jif backward ptr null
#
# --- Remove RPN from end of thread
#
        st      r14,rp_afthd(r5)        # Close forward link
        st      r5,qu_tail(r13)         # Set up new RPN tail
        b       .de40
#
# --- Remove only RPN from thread
#
.de20:
        st      r14,qu_head(r13)        # Clear RPN queue head
        st      r15,qu_tail(r13)        # Clear RPN queue tail
        mov     0,r3                    # Clear queue count
        b       .de50
#
# --- Remove RPN from head of thread
#
.de30:
        st      r14,rp_abthd(r4)        # Close backward link
        st      r4,qu_head(r13)         # Set up new RPN head
#
# --- Update RAID 5 Exec queue count
#
.de40:
        ld      qu_qcnt(r13),r3         # Adjust queue count
        subo    1,r3,r3
.de50:
        mov     FALSE,r4                # Clear RPN active
        st      r3,qu_qcnt(r13)
        stob    r4,rp_act(g3)
#
# --- Exit
#
.de100:
        ret
#
#**********************************************************************
#
#  NAME: r$initrpn
#
#  PURPOSE:
#       To provide a means of initiating RAID 5 I/O operations defined
#       by a specific RPN node.
#
#  DESCRIPTION:
#       Each inactive RRB queued to the specified RPN node is examined
#       to see if it can be initiated. Each RRB that has no initiation
#       conflicts is initiated. A conflict exists when there is a write
#       precedence situation or a parity region overlap. When all
#       possible initiations have been processed, this routine exits.
#
#  INPUT:
#       g3  = RPN
#       g14 = R5S (uninitialized)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All
#
#**********************************************************************
#
r$initrpn:
#
.ir05:
        lda     rp_rrbhead(g3),g2       # Get RRB queue ptr
#
# --- Check next RRB candidate ----------------------------------------
#
.ir10:
        ld      rb_fthd(g2),g2          # Get next RRB
        cmpobe  0,g2,.ir100             # Jif none
#
        ldob    rb_stat(g2),r3          # Get RRB status
        bbs     rbact,r3,.ir10          # Jif request active
#
        ldob    rb_type(g2),r3          # Get request type
        lda     rp_rrbhead(g3),r14      # Prepare search
        cmpobe  rbread,r3,.ir40         # Jif read
        cmpobe  rbverifyc,r3,.ir40      # Jif verify checkword
        cmpobe  rbverify,r3,.ir40       # Jif verify data
#
# --- Process write, write/verify, parity check, rebuild, or rebuild check ---
#
# --- Check any preceding requests
#
.ir15:
        ld      rb_fthd(r14),r14        # Get next RRB
        cmpobe  g2,r14,.ir25            # Jif same as candidate
#
# --- Check precedence
#
c       if (((RRB*)g2)->lsda >= ((RRB*)r14)->leda) {
            b   .ir20                   # Jif no possible overlap
c       }
c       if (((RRB*)g2)->leda >= ((RRB*)r14)->lsda) {
            b   .ir10                   # Jif definite overlap
c       }
#
# --- Check for parity region overlap
#
.ir20:
c       if (((RRB*)g2)->psda >= ((RRB*)r14)->peda) {
            b .ir15                     # Jif no possible overlap
c       }
c       if (((RRB*)g2)->peda > ((RRB*)r14)->psda) {
            b .ir10                     # Jif definite overlap
c       }
#
        b       .ir15
#
# --- Check any following requests
#
.ir25:
        ld      rb_fthd(r14),r14        # Get next RRB
        cmpobe  0,r14,.ir30             # Jif none
#
        ldob    rb_stat(r14),r4         # Get RRB status
        bbc     rbact,r4,.ir25          # Jif inactive
#
# --- Check for parity region overlap
#
c       if (((RRB*)g2)->psda >= ((RRB*)r14)->peda) {
            b .ir25                     # Jif no possible overlap
c       }
c       if (((RRB*)g2)->peda > ((RRB*)r14)->psda) {
            b .ir10                     # Jif definite overlap
c       }
#
        b       .ir25
#
# --- OK to initiate I/O
#
.ir30:
        cmpobe  rbrebuild,r3,.ir60      # Jif rebuild
        cmpobe  rbrebuildchk,r3,.ir60   # Jif rebuild check
        cmpobe  rbparitychk,r3,.ir60    # Jif parity check
#
# --- Initiate write
#
        mov     g14,r3                  # Save g14
        mov     g2,r4                   # Save g2
        mov     g3,r5                   # Save g3
c       g0 = r_initwrite(g2, g3, g14);  # Initiate write, write/verify.
        cmpobne 0,g0,.ir100             # Exit if RPN was released
        mov     r4,g2                   # Restore g2
        mov     r5,g3                   # Restore g3
        mov     r3,g14                  # Restore g14
        b       .ir05
#
# --- Process read, verify checkword or verify data -------------------
#
# --- Check any preceding requests
#
.ir40:
        ld      rb_fthd(r14),r14        # Get next RRB
        cmpobe  g2,r14,.ir50            # Jif same as candidate
#
        ldob    rb_type(r14),r3         # Get request type
        cmpobe  rbread,r3,.ir40         # Jif read
        cmpobe  rbverifyc,r3,.ir40      # Jif verify checkword
        cmpobe  rbverify,r3,.ir40       # Jif verify data
#
# --- Check precedence
#
c       if (((RRB*)g2)->lsda >= ((RRB*)r14)->leda) {
            b   .ir45                   # Jif no possible overlap
c       }
c       if (((RRB*)g2)->leda >= ((RRB*)r14)->lsda) {
            b   .ir10                   # Jif definite overlap
c       }
#
# --- Check for parity region overlap
#
.ir45:
c       if (((RRB*)g2)->psda >= ((RRB*)r14)->peda) {
            b .ir40                     # Jif no possible overlap
c       }
c       if (((RRB*)g2)->peda > ((RRB*)r14)->psda) {
            b .ir10                     # Jif definite overlap
c       }
#
        b       .ir40
#
# --- Check any following requests
#
.ir50:
        ld      rb_fthd(r14),r14        # Get next RRB
        cmpobe  0,r14,.ir55             # Jif none
#
        ldob    rb_type(r14),r3         # Get request type
        cmpobe  rbread,r3,.ir50         # Jif read
        cmpobe  rbverifyc,r3,.ir50      # Jif verify checkword
        cmpobe  rbverify,r3,.ir50       # Jif verify data
#
        ldob    rb_stat(r14),r4         # Get RRB status
        bbc     rbact,r4,.ir50          # Jif inactive
#
# --- Check for parity region overlap
#
c       if (((RRB*)g2)->psda >= ((RRB*)r14)->peda) {
            b .ir50                     # Jif no possible overlap
c       }
c       if (((RRB*)g2)->peda > ((RRB*)r14)->psda) {
            b .ir10                     # Jif definite overlap
c       }
#
        b       .ir50
#
# --- Initiate read
#
.ir55:
        mov     g14,r3                  # Save g14
        mov     g2,r4                   # Save g2
        mov     g3,r5                   # Save g3
c       r_initread(g2, g3, g14);        # Initiate read operation
        mov     r4,g2                   # Restore g2
        mov     r5,g3                   # Restore g3
        mov     r3,g14                  # Restore g14
        b       .ir05
#
# --- Initiate rebuild
#
.ir60:
        call    r$initrbld              # Initiate rebuild
#
# --- Exit
#
.ir100:
        ret
#
#**********************************************************************
#
#  NAME: r$initrbld
#
#  PURPOSE:
#       To provide a means of initiating the rebuild of a specific
#       single drive stripe on a RAID 5 array. Or, to perform a
#       parity check/correct on a RAID 5 array.
#
#  DESCRIPTION:
#       The R5S structure which is passed along with the RRB and RPN is
#       used as a scratchpad for generating the physical I/O operations
#       which are required for I/O to a RAID level 5 stripe. This
#       structure is resident within internal SRAM solely for performance
#       considerations.
#
#       This routine generates and queues full stripe reads to all devices
#       within the stripe that are not being rebuilt. The request for
#       writing the rebuild data is generated and linked to the RRB for
#       later submission after the reads have completed and the rebuild
#       buffer has been generated.
#
#       The device to rebuild is usually the device that is not operable.
#       Otherwise, the parity device is chosen.
#
#       For a parity check/correct, the reads are generated for all devices
#       including the parity device. The completion functions are changed
#       to redirect the completion to checker/corrector specific code.
#
#       The ILTs will be placed into a queue for submission at the completion
#       of this function. This prevents a race condition in a low memory
#       condition.
#
#  INPUT:
#       g2  = RRB
#       g3  = RPN
#       g14 = R5S
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
r$initrbld:
#
        movl    g2,r14                  # Save RRB/RPN
        ldconst 0,r3
        st      r3,rb_pcsgl(g2)         # clear out rb_pcsgl field
#
# --- Clear the variable for the ILT queue maintenance.
#
        mov     0,r7
#
# --- Initialize RRB and R5S structures
#
c       r_initrrbr5s(g2, g3, g14);
#
        ld      rp_ppsd(g3),r13         # Get parity PSD
        ld      r5_fpsd(g14),r12        # Get possible failing (rebuild) PSD
#
        lda     r$r5rbrcomp,r10         # Set completion fn (assume rebuild)
#
        ldob    rb_type(r14),r3         # r3 = RRB type code
        cmpobne rbrebuildchk,r3,.ib10   # Jif not a Rebuild check
        cmpobne 0,r12,.ib05             # Jif failed PSD (rebuild needs to be
                                        #  done or cannot rebuild)
        ldconst ecok,r3                 # Nothing wrong (op below highwater
                                        #  mark or no rebuild needed)
        stob    r3,rb_rstat(r14)        # Save the good status for requestor
        call    r$rrrb                  # Return status and release RRB
        b       .ib100                  # And we're out of here!
#
.ib05:
        mov     g0,r11                  # Save g0
        mov     r12,g0                  # g0 = PSD to see if rebuilding
        call    R$checkForR5PSDRebuilding  # Determine if in Rebuilding state
        cmpo    TRUE,g0                 # In the Rebuilding state?
        mov     r11,g0                  # Restore g0
        be      .ib20                   # Jif Rebuilding - Rebuild write area
.ib07:
        ldconst ecreserved,r3           # Not Rebuilding - Show not possible
        stob    r3,rb_rstat(r14)        # Save error code for requestor
        call    r$rrrb                  # Release RRB
        b       .ib100                  # And we're out of here!
#
.ib10:
        cmpobne rbparitychk,r3,.ib20    # Jif not parity check request
#
# --- If any failed drives exist, can't perform a parity check operation.
#
        lda     r$r5pcrcomp,r10         # Set completion fn (parity checker)
#
        cmpobe  0,r12,.ib20             # Jif there is no bad PSD
#
        mov     g0,r11                  # Save g0
        mov     r12,g0                  # g0 = PSD to see if rebuilding
        call    R$checkForR5PSDRebuilding  # Determine if in Rebuilding state
        cmpo    TRUE,g0                 # In the Rebuilding state?
        mov     r11,g0                  # Restore g0
        bne     .ib07                   # Jif not Rebuilding - return error
        ldconst 0,r12                   # Since Rebuild, stripe has been rebuilt
        st      r12,r5_fpsd(g14)        #  then treat like no PSD failure
#
.ib20:
        ld      rp_wpsd(g3),r11         # Get wrap PSD
        cmpo    0,r12                   # Check for failing PSD
        sele    r12,r13,r12             # Use parity PSD for failing
                                        #  PSD if no rebuild PSD
                                        #  detected
        ldob    r5_depth(g14),r8        # Get stripe width (3, 5 or 9)
        mov     0,r9                    # Clear RRB SGL ptr offset
        ld      r5_spsd(g14),g3         # Start with first PSD
        stob    r8,rb_orc(r14)
        cmpobne rbparitychk,r3,.ib30    # Jif not parity check request
        addo    1,r8,r3                 # add 1 to I/O count
        stob    r3,rb_orc(r14)          # save updated I/O count
#
# --- Process next device in stripe
#
.ib30:
# --- Generate ILT/PRP.
c       g1 = get_ilt();                 # Allocate an ILT.
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt %p\n", FEBEMESSAGE, __FILE__, __LINE__, (void *)g1);
.endif /* M4_DEBUG_ILT */
c       g2 = get_prp();                 # Assign PRP.
.ifdef M4_DEBUG_PRP
c       CT_history_printf("%s%s:%u get_prp %p\n", FEBEMESSAGE, __FILE__, __LINE__, (void *)g2);
.endif /* M4_DEBUG_PRP */
c       ((ILT*)g1)->ilt_normal.w0 = (UINT32)g2; # Link PRP to ILT.

        st      r14,il_w3(g1)           # Set up RRB
        st      g3,il_w4(g1)            # Set up PSD
#
        ld      r5_sps(g14),g5          # Pass sector count
        ldob    r5_strategy(g14),g10    # Pass RRP strategy
        cmpobne g3,r12,.ib60            # Jif not rebuild device
#
# --- Process rebuild device
#
        ldconst rroutput,g8             # Assume output RRP function code
#
c       r_bldprp(g2, g3, ((R5S*)g14)->obpsda, g5, g8, g10);  # Build the PRP
#
        shlo    9,g5,g0                 # Allocate rebuild buffer
c       g0 = m_asglbuf(g0);
#
        ld      sg_size(g0),r3          # Link SGL to PRP
        st      g0,pr_sglptr(g2)
        st      g0,rb_sgl0(r14)         # Set up dst SGL in RRB
        setbit  31,r3,r3                # Set as borrowed
        st      g1,rb_pwilt(r14)        # Save rebuild request
        st      r3,pr_sglsize(g2)
#
# --- If parity check request, need to generate I/O request to
# --- read the parity strip.
#
        ldob    rb_type(r14),r3         # r3 = RRB type code
        cmpobne rbparitychk,r3,.ib80    # Jif not parity check request
#
# --- Build up PRP to read parity data.
#
# --- Generate ILT/PRP.
c       g1 = get_ilt();                 # Allocate an ILT.
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt %p\n", FEBEMESSAGE, __FILE__, __LINE__, (void *)g1);
.endif /* M4_DEBUG_ILT */
c       g2 = get_prp();                 # Assign PRP.
.ifdef M4_DEBUG_PRP
c       CT_history_printf("%s%s:%u get_prp %p\n", FEBEMESSAGE, __FILE__, __LINE__, (void *)g2);
.endif /* M4_DEBUG_PRP */
c       ((ILT*)g1)->ilt_normal.w0 = (UINT32)g2; # Link PRP to ILT.

        st      r14,il_w3(g1)           # Set up RRB
        st      g3,il_w4(g1)            # Set up PSD
#
        ldconst rrinput,g8              # Pass RRP function code
c       r_bldprp(g2, g3, ((R5S*)g14)->obpsda, g5, g8, g10);  # Build the PRP
#
        shlo    9,g5,g0                 # Allocate rebuild buffer
c       g0 = m_asglbuf(g0);
#
        ld      sg_size(g0),r3          # Link SGL to PRP
        st      g0,rb_pcsgl(r14)        # Save parity read SGL in RRB
        st      g0,pr_sglptr(g2)
        setbit  31,r3,r3                # Set as borrowed
        st      r3,pr_sglsize(g2)
#
# --- Save physical request (non-rebuild)
#
        st      r7,il_fthd(g1)          # Link ILT to submission queue
        mov     g1,r7
#
        b       .ib80
#
# --- Process non-rebuild device
#
.ib60:
        ldconst rrinput,g8              # Pass RRP function code
c       r_bldprp(g2, g3, ((R5S*)g14)->obpsda, g5, g8, g10);  # Build the PRP
#
        shlo    9,g5,g0                 # Allocate rebuild buffer
c       g0 = m_asglbuf(g0);
#
        ld      sg_size(g0),r3          # Link SGL to PRP
        st      g0,rb_sgl1(r14)[r9*4]   # Set up next src SGL in RRB
        addo    1,r9,r9                 # Bump for next RRB SGL
        st      g0,pr_sglptr(g2)
        setbit  31,r3,r3                # Set as borrowed
        st      r3,pr_sglsize(g2)
#
# --- Save physical request (non-rebuild)
#
        st      r7,il_fthd(g1)          # Link ILT to submission queue
        mov     g1,r7
#
# --- Advance to next device
#
.ib80:
        subo    1,r8,r8                 # Adjust remaining device count
        cmpobe  0,r8,.ib90              # Jif complete
#
        ld      ps_npsd(g3),g3          # Advance to next PSD
        cmpobne r11,g3,.ib30            # Jif not wrap PSD
#
c       ((R5S*)g14)->obpsda += ((R5S*)g14)->sps;    /* Adjust base PSDA */
        b       .ib30
#
# --- Set SGL terminator
#
.ib90:
        st      r8,rb_sgl1(r14)[r9*4]   # Set SGL terminator
#
# --- Submit the commands to the physical queue.
#
        mov     0,r3
#
.ib95:
        mov     r7,g1                   # Get next ILT in submission thread
        ld      il_fthd(r7),r7          # Save next ILT
        st      r3,il_fthd(g1)          # Clear it
#
        ld      P_que,g0                # Set physical queuing routine
        mov     r10,g2                  # Set completion routine
        call    K$q                     # Queue request w/o wait
        cmpobne 0,r7,.ib95              # Try again.
#
# --- Exit
#
.ib100:
        ret
#
#**********************************************************************
#
#  NAME: r$r5rbrcomp  (rebuild)
#
#  PURPOSE:
#       To provide a common means of handling the completion of a
#       recovery operation pursuant to a rebuild operation.
#
#  DESCRIPTION:
#       The status of the completing reconstruction read PRP is checked.
#       If an error has occurred, the RRB is flagged with an irrecoverable
#       error.
#
#       This reconstruction read ILT/PRP is released after unlinking
#       the SGL from the PRP. The outstanding request count is adjusted
#       downward to account for the reconstruction read ILT/PRP just
#       completed. If outstanding requests exist, this routine exits.
#
#       If the RRB indicates that an error has occurred, the parity
#       write and read reconstruction SGL/buffers are released after
#       releasing the parity write ILT/PRP. The RRB is then released
#       and this routine exits.
#
#       The data from all of the reconstruction reads is xor'ed together
#       to produce the new parity data. The reconstruction SGL/buffers
#       are released back to the system and the parity write ILT/PRP is
#       queued to the physical layer. The routine then exits.
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
#      CALLER AREA
#      ___________
#      W0 = PRP
#      W1 = null
#      W2 = null
#      W3 = RRB
#      W4 = PSD
#
#**********************************************************************
#
r$r5rbrcomp:
#
        movq    g0,r12                  # Save g0-g3
#
# --- Check request status
#
        ld      il_w0(g1),g2            # Get PRP
        call    M$chkstat               # Check status
        ld      il_w3(g1),g2            # Get RRB
        cmpobe  ecok,g0,.br10           # Jif OK
#
# --- Process reconstruction read error
#
        call    r$seterr                # Set irrecoverable error
#
# --- Release reconstruction read ILT/PRP
#
.br10:
        ld      rb_pwilt(g2),r10        # Get parity write ILT
        call    M$rip                   # Release ILT/PRP
#
# --- Check outstanding request count (RRB)
#
        ldob    rb_orc(g2),r3           # Get outstanding request count
        subo    1,r3,r3
        stob    r3,rb_orc(g2)           # Update outstanding request count
        cmpobne 1,r3,.br100             # Jif more
#
        ldob    rb_rstat(g2),r3         # Get request status
        cmpobe  ecok,r3,.br30           # Jif OK
#
# --- On error release parity write ILT/PRP
#
        mov     r10,g1
        call    M$rip                   # Release ILT/PRP
#
# --- Release parity write and read reconstruction SGL/buffers
#
        lda     rb_sgl0(g2),r3          # Set pointer to 1st release
.br20:
        ld      (r3),g0                 # Get next SGL/buffer
        cmpobe  0,g0,.br25              # Jif end
#
        st      0,(r3)                  # make sure cannot use sgl pointer again
        call    M$rsglbuf               # Release next SGL/buffer
        addo    4,r3,r3                 # Advance pointer
        b       .br20
#
# --- Release RRB
#
.br25:
        call    r$rrrb                  # Release RRB
        b       .br100
#
# --- Generate parity data
#

.br30:
#
#
# g2 = pRRB
#
# pRRB->pDestSGL= rb_sgl0(g2) =  SGL buffer contains the XORed result of all the
#                                following sources (partial stripe data read
#                                from good drives).
#                                After XOR, this contains that partial stripe
#                                data related to failed device.
# pRRB->pSrc1SGL = - ---|
# pRRB->pSrc2SGL =     |
# . . . . . . .  =     |    SGL buffers contains the data read from all the data
# . . . . . . .  =     |    drives (except failed one) in that partial stripe.
# . . . . . . .  =     |
# pRRB->pSrcnSGL =  ---|
#
#
# --- Get pointer to 1st SGL/buffer, containing the data read from first good
# --- data drive
#
        lda     rb_sgl1(g2),r3
#
# --- Obtain the parity for the above SGLs

        call    r$sxorsgls              # XOR the SGLs
#
# --- Release read reconstruction SGL/buffers
# --- Releasing all the buffers (pRRB->pSrc1SGL, pSrc2SGL,.....)
# This is brought from r$r5rbrcomp2() and changed the branch labels.
#
.br10_1:
        ld      (r3),g0                 # Get next SGL/buffer
        cmpobe  0,g0,.br20_1            # Jif done
#
        st      0,(r3)                  # make sure cannot use sgl pointer again
        call    M$rsglbuf               # Release next SGL/buffer
        addo    4,r3,r3                 # Advance pointer
        b       .br10_1

#
# --- Queue physical request (parity write)
#
.br20_1:
        ld      P_que,g0                # Set physical queuing routine

# --- We have already had the writeILT in r10 register......
#       ld      rb_pwilt(g2),g1         # Get parity write ILT
        mov     r10,g1                  # Get parity write ILT
        lda     r$r5rbwcomp,g2          # Set completion routine
        call    K$q                     # Queue request w/o wait


#
# --- Exit
#
.br100:
        movq    r12,g0                  # Restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: r$r5rbwcomp  (rebuild)
#
#  PURPOSE:
#       To provide a common means of handling the completion of a write
#       operation pursuant to a rebuild operation.
#
#  DESCRIPTION:
#       The status of the completing parity write PRP is checked. If an
#       error has occurred, the RRB is flagged with an irrecoverable
#       error.
#
#       The parity write SGL/buffer is released back to the system
#       followed by the ILT/PRP. The RRB is released and this routine
#       exits
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
#      CALLER AREA
#      ___________
#      W0 = PRP
#      W1 = null
#      W2 = null
#      W3 = RRB
#      W4 = PSD
#
#**********************************************************************
#
r$r5rbwcomp:
#
        movq    g0,r12                  # Save g0-g3
#
# --- Check request status
#
        ld      il_w0(g1),g2            # Get PRP
        call    M$chkstat               # Check status
        ld      il_w4(g1),r10           # Get PSD
        mov     g2,r11                  # Save PRP
        ld      il_w3(g1),g2            # Get RRB
        cmpobne ecok,g0,.bp20_5         # Jif not OK
#
# --- Update PSD
#
        ldob    ps_status(r10),r5       # Get segment status
        cmpobne psrebuild,r5,.bp30      # Jif not rebuild status
#
# --- Check for hotspared hotspare
#
        ldos    ps_pid(r10),r9          # Get PID
        ldconst 0xff,r3                 # Get 8-bit mask
        ld      P_pddindx[r9*4],r9      # Lookup PDD
        ld      pd_dev(r9),r6           # Get device from PDD
        ld      pr_dev(r11),r4          # Get device from PRP
        cmpobne r4,r6,.bp10_5           # Jif rebuild hotspared
#
# --- Update rebuild length if this is not a Rebuild Check (needed to allow
#       Writes in the rebuild area to allow resync to work on failover).
#
        ldob    rb_type(g2),r4          # Get the type of rebuild that just
                                        #  Completed
        cmpobe  rbrebuildchk,r4,.bp30   # Jif Rebuild Check (do not update
                                        #  the High Water Mark.
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
c       ((PSD*)r10)->rLen = ((PRP*)r11)->eda - ((PSD*)r10)->sda;
        b       .bp30
#
# --- Set irrecoverable error in RRB
#
.bp10_5:
        ldconst ecioerr,r3              # Set irrecoverable error
        stob    r3,rb_rstat(g2)
        b       .bp30
#
# --- Set irrecoverable error in RRB and attempt hotspare
#
.bp20_5:
        call    r$seterr                # Set irrecoverable error
#
# --- Release parity write SGL/buffer and ILT/PRP
#
.bp30:
        ld      pr_sglptr(r11),g0       # Release SGL/buffer
        st      0,pr_sglptr(r11)        # make sure cannot use sgl pointer again
        call    M$rsglbuf
#
        call    M$rip                   # Release ILT/PRP
#
# --- Release RRB
#
        call    r$rrrb                  # Release RRB
#
# --- Exit
#
        movq    r12,g0                  # Restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: r$r5pcrcomp  (parity checker/corrector)
#
#  PURPOSE:
#       To provide a common means of handling the completion of a
#       read operation for the parity checker/corrector.
#
#  DESCRIPTION:
#       The status of the completing reconstruction read PRP is checked.
#       If an error has occurred, the RRB is flagged with an irrecoverable
#       error.
#
#       The outstanding request count is adjusted downward to account
#       for the reconstruction read ILT/PRP just completed. If
#       outstanding requests exist, this routine exits.
#
#       If the RRB indicates that an error has occurred, the parity
#       write and read reconstruction SGL/buffers are released after
#       releasing the parity write ILT/PRP. The RRB is then released
#       and this routine exits.
#
#       The data from all of the reconstruction reads is xor'ed together
#       to produce the new parity data. The buffers are held until later
#       to be examined in debugging. They will be released in the final
#       completion routines.
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
#      CALLER AREA
#      ___________
#      W0 = PRP
#      W1 = null
#      W2 = null
#      W3 = RRB
#      W4 = PSD
#
#**********************************************************************
#
r$r5pcrcomp:
#
        movq    g0,r12                  # Save g0-g3
#
# --- Check request status
#
        ld      il_w0(g1),g2            # Get PRP
        call    M$chkstat               # Check status
        ld      il_w3(g1),g2            # Get RRB
        cmpobe  ecok,g0,.pcr10_5        # Jif OK
#
# --- Process reconstruction read error
#
        call    r$seterr                # Set irrecoverable error
#
# --- Release reconstruction read ILT/PRP
#
.pcr10_5:
        ld      rb_pwilt(g2),r10        # Get parity write ILT
        call    M$rip                   # Release ILT/PRP
#
# --- Check outstanding request count (RRB)
#
        ldob    rb_orc(g2),r3           # Get outstanding request count
        subo    1,r3,r3
        stob    r3,rb_orc(g2)           # Update outstanding request count
        cmpobne 1,r3,.pcr100            # Jif more
#
        ldob    rb_rstat(g2),r3         # Get request status
        cmpobe  ecok,r3,.pcr30          # Jif OK
#
# --- On error release parity write ILT/PRP
#
        mov     r10,g1
        call    M$rip                   # Release ILT/PRP
#
# --- Release parity write and read reconstruction SGL/buffers
#
        lda     rb_sgl0(g2),r3          # Set pointer to 1st release
#
.pcr20:
        ld      (r3),g0                 # Get next SGL/buffer
        cmpobe  0,g0,.pcr25             # Jif end
#
        st      0,(r3)                  # make sure cannot use sgl pointer again
        call    M$rsglbuf               # Release next SGL/buffer
        addo    4,r3,r3                 # Advance pointer
        b       .pcr20
#
# --- Release last SGL for parity read and RRB
#
.pcr25:
        ld      rb_pcsgl(g2),g0         # Get SGL
        st      0,rb_pcsgl(g2)          # make sure cannot use sgl pointer again
        call    M$rsglbuf               # Release it
#
        call    r$rrrb                  # Release RRB
        b       .pcr100
#
# --- Generate parity data
#
.pcr30:

#
#
#
# g2 = pRRB
#
# pRRB->pDestSGL= rb_sgl0(g2) =  SGL buffer contains the XORed result of all the
#                                data drives in the partial stripe
# pRRB->pSrc1SGL =  ---|
# pRRB->pSrc2SGL =     |
# . . . . . . .  =     |    SGL buffers contains the data read from all the data
# . . . . . . .  =     |    drives in that partial stripe.
# . . . . . . .  =     |
# pRRB->pSrcnSGL =  ---|
#
# --- Obtain the parity for the above SGLs
#
        call    r$sxorsgls              # XOR the SGLs
#
# --- Do the parity and data comparison.
#
#  Following code brought from r$r5pcrcomp2 and changed the branch labels.
        movq    g4,r4
        ld      rb_sgl0(g2),r3          # Get SGL for read data
        ld      sg_desc0+sg_addr(r3),g4 # Get one of the sources
        ld      rb_pcsgl(g2),r3         # Get SGL for XOR data
        ld      sg_desc0+sg_addr(r3),g5 # Get other one of the sources
        ld      sg_desc0+sg_len(r3),g6  # Get length

#   C - Linux versions fast buffer comparison.
c       g0 = RL_FastMemCmp ((unsigned long *)g4,(unsigned long *)g5,g6);

        cmpobe  TRUE,g0,.pcr50_1        # Jif same
#
# --- Update the statistics on the miscompare.
#
        ldconst eccompare,r3            # r3 = RRB error code
        stob    r3,rb_rstat(g2)
#
        ld      rb_rpn(g2),r4           # r4 = assoc. RPN address
        ld      rp_spsd(r4),r5          # r5 = starting PSD
        ld      rp_ppsd(r4),r6          # r6 = parity PSD
        ld      rb_rdd(g2),r3           # r3 = assoc. RDD address
        ldob    rd_depth(r3),r7         # r7 = stripe width (3,5,9)
#
.pcr10_1:
        ldos    ps_pid(r5),r8           # r8 = PID
        ld      P_pddindx[r8*4],r8      # r8 = PDD
#
        cmpobne r5,r6,.pcr20_1            # Jif not the parity PSD
#
        ld      pd_miscomp(r8),r9       # r9 = miscompare count for parity drive
        addo    1,r9,r9                 # Inc. count
        st      r9,pd_miscomp(r8)       # Save updated count
        b       .pcr30_1
#
.pcr20_1:
        ld      pd_devmiscomp(r8),r9    # r9 = miscompare count for data drive
        addo    1,r9,r9                 # Inc. count
        st      r9,pd_devmiscomp(r8)    # Save updated count
#
.pcr30_1:
        ld      ps_npsd(r5),r5          # r5 = next PSD in list
        subo    1,r7,r7                 # Dec. stripe width
        cmpobne 0,r7,.pcr10_1           # Jif more segments for this stripe
#
# --- Queue physical request if correction was requested. This is a write
# --- of the parity with the generated parity. If the parity clean up was
# --- not requested, just do the clean up as though there were no errors.
#
        ld      rb_ilt(g2),r3           # Get primary ILT
        ld      il_w0-ILTBIAS(r3),r3    # Get RRP
        ldob    rr_options(r3),r3       # Get the options field
        bbc     rrpchkcorrect,r3,.pcr50_1 # Jif no correction requested
#
        ld      P_que,g0                # Set physical queuing routine
        ld      rb_pwilt(g2),g1         # Get parity write ILT
        lda     r$r5pcwcomp,g2          # Set completion routine
        call    K$q                     # Queue request w/o wait
        b       .pcr100                 # Exit
#
# --- Finish cleaning up the RRB and the parity write ILT.
#
.pcr50_1:
        ld      rb_pwilt(g2),g1         # Get ILT
        call    M$rip                   # Release it
#
# --- Release read reconstruction SGL/buffers including the write buffer.
#
        ld      rb_pcsgl(g2),g0         # Get the parity read buffer
        st      0,rb_pcsgl(g2)          # make sure cannot use sgl pointer again
        call    M$rsglbuf               # Release it
#
# --- Releasing all the buffers (pRRB->pDestSGL, pSrc1SGL, pSrc2SGL,.....)
#

        lda     rb_sgl0(g2),r3          # Get ptr to 1st SGL/buffer to release
#
.pcr70_1:
        ld      (r3),g0                 # Get next SGL/buffer
        cmpobe  0,g0,.pcr80_1           # Jif done
#
        st      0,(r3)                  # make sure cannot use sgl pointer again
        call    M$rsglbuf               # Release next SGL/buffer
        addo    4,r3,r3                 # Advance pointer
        b       .pcr70_1
#
# --- Release the RRB and exit.
#
.pcr80_1:
        call    r$rrrb                  # Release it
#
#
# --- Exit
#
.pcr100:
        movq    r12,g0                  # Restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: r$r5pcwcomp  (parity checker/corrector)
#
#  PURPOSE:
#       To provide a common means of handling the completion of a write
#       operation pursuant to a correction operation.
#
#  DESCRIPTION:
#       The status of the completing correction write PRP is checked. If
#       an error has occurred, the RRB is flagged with an irrecoverable
#       error.
#
#       The parity write SGL/buffer is released back to the system
#       followed by the ILT/PRP. The RRB is released and this routine
#       exits. All buffers used to generate the parity are also released.
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
#      CALLER AREA
#      ___________
#      W0 = PRP
#      W1 = null
#      W2 = null
#      W3 = RRB
#      W4 = PSD
#
#**********************************************************************
#
r$r5pcwcomp:
#
        movq    g0,r12                  # Save g0-g3
#
# --- Check request status
#
        ld      il_w0(g1),g2            # Get PRP
        call    M$chkstat               # Check status
        ld      il_w4(g1),r10           # Get PSD
        mov     g2,r11                  # Save PRP
        ld      il_w3(g1),g2            # Get RRB
        cmpobe  ecok,g0,.pcw10          # Jif OK
#
# --- Set irrecoverable error in RRB and attempt hotspare
#
        call    r$seterr                # Set irrecoverable error
#
# --- Release parity write SGL/buffer and ILT/PRP
#
.pcw10:
        ld      il_w3(r13),g2           # Get RRB
        ld      rb_pwilt(g2),g1         # Get ILT
        call    M$rip                   # Release it
#
# --- Release read reconstruction SGL/buffers including the write buffer.
#
        ld      rb_pcsgl(g2),g0         # Get the parity read buffer
        st      0,rb_pcsgl(g2)          # make sure cannot use sgl pointer again
        call    M$rsglbuf               # Release it
#
        lda     rb_sgl0(g2),r3          # Get ptr to 1st SGL/buffer to release
#
.pcw20:
        ld      (r3),g0                 # Get next SGL/buffer
        cmpobe  0,g0,.pcw30             # Jif done
#
        st      0,(r3)                  # make sure cannot use sgl pointer again
        call    M$rsglbuf               # Release next SGL/buffer
        addo    4,r3,r3                 # Advance pointer
        b       .pcw20
#
# --- Release the RRB and exit.
#
.pcw30:
        call    r$rrrb                  # Release it
#
# --- Exit
#
        movq    r12,g0                  # Restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: r$r5frrcomp (Algorithm 1)
#
#  PURPOSE:
#       To provide a common means of handling the completion of a RAID
#       5 recovery read operation whenever a failed device has been
#       found underlying the targeted PSD during a full stripe read.
#
#  DESCRIPTION:
#       The status of the data read is checked for success. If bad,
#       the status of the RRB is set to irrecoverable.
#
#       The completing ILT/PRP is released back to the system.
#
#       The outstanding request count in the RRB is decremented by
#       one. If no outstanding recovery read requests exist, the SGLs
#       are simultaneously XOR'ed together, the SGLs are released and
#       the RRB is released.
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
#      CALLER AREA
#      ___________
#      W0 = PRP
#      W1 = null
#      W2 = SN
#      W3 = RRB
#      W4 = PSD
#
#**********************************************************************
#
r$r5frrcomp:
#
        movq    g0,r12                  # Save g0-g3
#
# --- Check request status
#
        ld      il_w0(g1),g2            # Get PRP
        call    M$chkstat               # Check status
#
# --- Release ILT/PRP
#
        ld      il_w3(g1),g2            # Get RRB
#
        cmpobe  ecok,g0,.fd10_5         # Jif status OK
#
# --- Process read error
# Needs g0=prp status, g1=ilt, and g2=rrb.
        call    r$seterr                # Set irrecoverable error
#
# --- Check outstanding request count
#
.fd10_5:
# Needs g1=ilt
        call    M$rip                   # Release ILT/PRP
        ldob    rb_orc(g2),r3           # Adjust outstanding request
        subo    1,r3,r3                 #  count
        stob    r3,rb_orc(g2)
        cmpobne 0,r3,.fd100_5           # Jif more
#
# --- Simultaneously XOR all data inputs into dst SGL buffer
# Needs g2=rrb
        call    r$sxorsgls              # XOR SGLs
#
# --- Release all SGLs
#
        lda     rb_sgl1(g2),r6          # Get ptr to SGL ptrs - sgl0 and
                                        #  sgl1 are duplicates
#
.fd20_5:
        ld      (r6),g0                 # Get next SGL
        addo    4,r6,r6                 # Advance to next SGL ptr
        cmpobe.f 0,g0,.fd30             # Jif complete
#
        ld      sg_size(g0),g1          # Pass byte count
c       s_Free(g0, g1, __FILE__, __LINE__); # Release SGL
        b       .fd20_5
#
# --- Release RRB
#
.fd30:
        call    r$rrrb                  # Release RRB
#
# --- Exit
#
.fd100_5:
        movq    r12,g0                  # Restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: r$r5drcomp (Algorithm 2)
#
#  PURPOSE:
#       To provide a means of handling the completion of a normal data
#       read or verify operation operation.
#
#  DESCRIPTION:
#       The status of the completing PRP is checked. If an error has
#       occurred and it is not a cancellation or a data verify error, an
#       attempt is made to hotspare the device.
#
#       The RRB outstanding request count is adjusted. If no more
#       outstanding requests exist and an error has occurred for any PRP,
#       the RRB is checked to see if recovery has been invoked. If
#       not, the RRB is resubmitted for another try, else the RRB is
#       released.
#
#       The completing ILT/PRP/SGL are released back to the system.
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
#      CALLER AREA
#      ___________
#      W0 = PRP
#      W1 = null
#      W2 = SN
#      W3 = RRB
#      W4 = PSD
#
#**********************************************************************
#
r$r5drcomp:
#
        mov     g0,r12                  # Save g0
        mov     g1,r13                  # Save g1
        mov     g2,r14                  # Save g2
        mov     g3,r15                  # Save g3
#
# --- Check request status
#
        ld      il_w0(g1),g2            # Get PRP
        call    M$chkstat               # Check status
#
        ld      il_w3(g1),g2            # Get RRB
        cmpobe  ecok,g0,.rn10           # Jif OK
#
# --- Process read error
#
# c printf ("***V: r5drcomp-0: g0 = %#x, g1 = %#x, g2 = %#x\n", g0, g1, g2);
        call    r$seterr                # Set irrecoverable error
#
# --- Check outstanding request count (RRB)
#
.rn10:
# c printf ("***V: r5drcomp-10: g0 = %#x, g1 = %#x, g2 = %#x\n", g0, g1, g2);
        ldob    rb_orc(g2),r3           # Get outstanding request count
## c printf ("***V: r5drcomp-15: RRB outstand = %d\n", r3);
        subo    1,r3,r3                 #  and adjust
        stob    r3,rb_orc(g2)           # Update outstanding request count
        cmpobne 0,r3,.rn20              # Jif more
# c printf ("***V: r5drcomp-16: calling r$comprrb\n");
        call    r$comprrb               # Complete/release RRB
#
# --- Release ILT/PRP/SGL
#
.rn20:
# c printf ("***V: r5drcomp-20: Calling M$rip\n");
        call    M$rip                   # Release ILT/PRP/SGL
#
# --- Exit
#
        mov     r12,g0                  # Restore g0
        mov     r13,g1                  # Restore g1
        mov     r14,g2                  # Restore g2
        mov     r15,g3                  # Restore g3
        ret
#
#**********************************************************************
#
#  NAME: r$r5prrcomp (Algorithm 2)
#
#  PURPOSE:
#       To provide a common means of handling the completion of a RAID
#       5 recovery read operation whenever a failed device has been
#       found underlying the targeted PSD during a partial stripe read
#       or a verify operation.
#
#  DESCRIPTION:
#       The status of the data read or verify is checked for success. If bad,
#       the status of the RRB is set to irrecoverable.
#
#       The completing ILT/PRP is released back to the system.
#
#       The ownership count is adjusted within the RRB. If outstanding
#       recovery read requests exist this routine exits. Otherwise the
#       SGLs are simultaneously XOR'ed together, the dst SGL is released
#       and then all recovery buffers/SGLs are released. The outstanding
#       request count in the RRB is decremented by one and the RRB is
#       released if that count has gone to zero.
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
#      CALLER AREA
#      ___________
#      W0 = PRP
#      W1 = null
#      W2 = SN
#      W3 = RRB
#      W4 = PSD
#      W5 = dst XOR SGL
#
#**********************************************************************
#
r$r5prrcomp:
#
        movq    g0,r12                  # Save g0-g3
#
# --- Check request status
#
        ld      il_w0(g1),g2            # Get PRP
        call    M$chkstat               # Check status
#
        ld      il_w3(g1),g2            # Get RRB
        cmpobe  ecok,g0,.rr10_5         # Jif status OK
#
# --- Process read or verify error
#
        call    r$seterr                # Set irrecoverable error
#
# --- Adjust and check recovery request count
#
.rr10_5:
        ldob    rb_rorc(g2),r3          # Adjust recovery request count
        subo    1,r3,r3
        stob    r3,rb_rorc(g2)
        cmpobne 0,r3,.rr100_5           # Jif more
#
# --- Determine type of operation
#
        ldob    rb_type(g2),r3          # Get request type
        cmpobe  rbread,r3,.rr20_5       # Jif read
#
        cmpobe  rbverifyc,r3,.rr50      # Jif verify checkword
#
# --- Simultaneously XOR all data inputs into 1st src SGL buffer then
#     compare to dst SGL buffer
#

        ldob    rb_rstat(g2),r3         # Check current status
        cmpobne ecok,r3,.rr30           # Jif previous error

#
# Now XOR the data buffers stored in source SGL pointers of RRB and compare
#
# g2 = pRRB
#
# pRRB->pDestSGL= rb_sgl0(g2) =  SGL buffer to be compared with XORed data.
# pRRB->pSrc1SGL = one good drive. After XOR, it contains XORed data
#                  (The routine r$sxorvsgls puts the XORed result in this buf.
# pRRB->pSrc2SGL =  ---|
# . . . . . . .  =     |    SGL buffers contains the data read from all other
# . . . . . . .  =     |    drives
# . . . . . . .  =     |
# pRRB->pSrcnSGL =  ---|
#
        call    r$sxorvsgls             # XOR the SGLs
        ld      rb_sgl0(g2),g0          # Get SGLs for comparison
        ld      rb_sgl1(g2),g1          # Get SGLs for comparison

c       g3 = RL_CompareSGL ((SGL *)g0,(SGL *)g1);  /* Linux C - Version */

        cmpobe  TRUE,g3,.rr30          # Jif compared
#
        ldconst eccompare,r3            # Set compare error
        stob    r3,rb_rstat(g2)
        b       .rr30
#
# --- Simultaneously XOR all data inputs into dst SGL buffer
#
.rr20_5:
        call    r$sxorsgls              # XOR SGLs
#
# --- Release dst SGL
#
.rr30:
        lda     rb_sgl0(g2),r6          # Get ptr to SGL ptrs
        ld      (r6),g0                 # Get dst SGL
        addo    4,r6,r6                 # Advance to next SGL ptr
        ld      sg_size(g0),g1          # Pass byte count
c       s_Free(g0, g1, __FILE__, __LINE__); # Release SGL
#
# --- Release all reconstruction buffers/SGLs
#
.rr40:
        ld      (r6),g0                 # Get next SGL
        cmpobe  0,g0,.rr50              # Jif complete
#
        st      0,(r6)                  # make sure cannot use sgl pointer again
        call    M$rsglbuf               # Release SGL and buffer
        addo    4,r6,r6                 # Advance to next SGL ptr
        b       .rr40
#
# --- Release RRB if appropriate
#
.rr50:
        ldob    rb_orc(g2),r3           # Get outstanding request count
        subo    1,r3,r3                 #  and adjust
        stob    r3,rb_orc(g2)           # Update outstanding request count
        cmpobne 0,r3,.rr100_5             # Jif more
#
        call    r$rrrb                  # Release RRB
#
# --- Release ILT/PRP
#
.rr100_5:
        mov     r13,g1                  # Get ILT
        call    M$rip                   # Release ILT/PRP
#
# --- Exit
#
        movq    r12,g0                  # Restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: r$sxorsgls
#
#  PURPOSE:
#       To provide a common means of vectoring to the correct software
#       XOR procedure based upon the number of SGLs supplied with the
#       call.
#
#  DESCRIPTION:
#       This routine vectors to the correct XOR procedure based upon
#       the number of SGLs supplied to this routine. A zero is used to
#       terminate this list.
#
#  INPUT:
#       g2 = RRB
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
r$sxorsgls:
#
        movq    g0,r12                  # Save g0-g3
#
# --- Get p0-p3 parameters
#
        ld      rb_sgl0(r14),g0         # Get p0
        ld      rb_sgl1(r14),g1         # Get p1
        ld      rb_sgl2(r14),g2         # Get p2
        ld      rb_sgl3(r14),g3         # Get p3
        cmpobe  0,g3,.xs40              # Jif 3 SGLs
#
# --- Get p4-p7 parameters
#
        movq    g4,r8                   # Save g4-g7
        ld      rb_sgl4(r14),g4         # Get p4
        ld      rb_sgl5(r14),g5         # Get p5
        ld      rb_sgl6(r14),g6         # Get p6
        ld      rb_sgl7(r14),g7         # Get p7
        cmpobe  0,g4,.xs20              # Jif 4 SGLs
        cmpobe  0,g5,.xs10              # Jif 5 SGLs
#
# --- XOR 9 SGLs
#
        mov     g8,r7                   # Save g8
        ld      rb_sgl8(r14),g8         # Get p8

c       RL_XorSGL(9,g0,g1,g2,g3,g4,g5,g6,g7,g8);

        mov     r7,g8                   # Restore g8
        b       .xs30
#
# --- XOR 5 SGLs
#
.xs10:
c       RL_XorSGL(5,g0,g1,g2,g3,g4);
        b       .xs30
#
# --- XOR 4 SGLs
#
.xs20:
c       RL_XorSGL(4,g0,g1,g2,g3);

.xs30:
        movq    r8,g4                   # Restore g4-g7
        b       .xs100
#
# --- XOR 3 SGLs
#
.xs40:
c     RL_XorSGL(3,g0,g1,g2);
#
# --- Exit
#
.xs100:
        movq    r12,g0                  # Restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: r$sxorvsgls
#
#  PURPOSE:
#       To provide a common means of vectoring to the correct software
#       XOR procedure for a verify operation based upon the number of SGLs
#       supplied with the call.
#
#  DESCRIPTION:
#       This routine vectors to the correct XOR procedure based upon
#       the number of SGLs supplied to this routine. A zero is used to
#       terminate this list. The destination SGL buffer is preserved
#       to facilitate the comparison of reconstructed data.
#
#  INPUT:
#       g2 = RRB
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
r$sxorvsgls:
#
        movq    g0,r12                  # Save g0-g3
#
# --- Get p0-p3 parameters
#
        ld      rb_sgl0(r14),g0         # Get p0
        ld      rb_sgl1(r14),g1         # Get p1
        ld      rb_sgl2(r14),g2         # Get p2
        ld      rb_sgl3(r14),g3         # Get p3
        mov     g1,g0                   # Preserve destination SGL
        cmpobe  0,g3,.xt40              # Jif 3 SGLs
#
# --- Get p4-p7 parameters
#
        movq    g4,r8                   # Save g4-g7
        ld      rb_sgl4(r14),g4         # Get p4
        cmpobe  0,g4,.xt20              # Jif 4 SGLs
        ld      rb_sgl5(r14),g5         # Get p5
        cmpobe  0,g5,.xt10              # Jif 5 SGLs
#
# --- XOR 9 SGLs
#
        mov     g8,r7                   # Save g8
        ld      rb_sgl6(r14),g6         # Get p6
        ld      rb_sgl7(r14),g7         # Get p7
        ld      rb_sgl8(r14),g8         # Get p8

c       RL_XorSGL(9,g0,g1,g2,g3,g4,g5,g6,g7,g8);

        mov     r7,g8                   # Restore g8
        b       .xt30
#
# --- XOR 5 SGLs
#
.xt10:
c       RL_XorSGL(5,g0,g1,g2,g3,g4);
        b       .xt30
#
# --- XOR 4 SGLs
#
.xt20:
c       RL_XorSGL(4,g0,g1,g2,g3);
.xt30:
        movq    r8,g4                   # Restore g4-g7
        b       .xt100
#
# --- XOR 3 SGLs
#
.xt40:
c      RL_XorSGL (3,g0,g1,g2);
#
# --- Exit
#
.xt100:
        movq    r12,g0                  # Restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: r$r5fdwcomp  (Algorithm 1 and 2)
#
#  PURPOSE:
#       To provide a means of handling the completion of a normal full
#       stripe data write operation.
#
#  DESCRIPTION:
#       The status of the completing PRP is checked. If an error has occurred
#       an attempt is made to hotspare the device.
#
#       The RRB outstanding request count is adjusted. If no more
#       outstanding requests exist and an error has occurred for any
#       PRP, the RRB is checked to see if recovery has been invoked.
#       If not, the RRB is resubmitted for another try, else the RRB
#       is released.
#
#       The completing ILT/PRP/SGL are released back to the system.
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
#      CALLER AREA
#      ___________
#      W0 = PRP
#      W1 = null
#      W2 = null
#      W3 = RRB
#      W4 = PSD
#
#**********************************************************************
#
r$r5fdwcomp:
        mov     g0,r12                  # Save g0
        mov     g1,r13                  # Save g1
        mov     g2,r14                  # Save g2
        mov     g3,r15                  # Save g3
#
# --- Check request status
#
        ld      il_w0(g1),g2            # Get PRP
        call    M$chkstat               # Check status
        ld      il_w3(r13),g2           # Get RRB
        cmpobe  ecok,g0,.rw10           # Jif OK
#
# --- Process write error
#
        call    r$seterr                # Set irrecoverable error
#
# --- Check outstanding request count (RRB)
#
.rw10:
        ldob    rb_orc(g2),r3           # Get outstanding request count
        subo    1,r3,r3                 #  and adjust
        stob    r3,rb_orc(g2)           # Update outstanding request count
        cmpobne 0,r3,.rw40              # Jif more
#
# --- Release all of the SGLs for the original data written.
#
        ld      rb_sgl0(g2),g0          # Get SGL/buffer from parity
        st      0,rb_sgl0(g2)           # make sure cannot use sgl pointer again
        call    M$rsglbuf               # Release SGL/buffer
#
        lda     rb_sgl1(g2),r3          # Get ptr to SGL to release
#
.rw20:
        ld      (r3),g0                 # Get next SGL/buffer
        addo    4,r3,r3                 # Advance pointer
        cmpobe  0,g0,.rw30              # Jif done
#
# --- Release SGL but not the buffers
#
        ld      sg_size(g0),g1          # Pass size of SGL
c       s_Free(g0, g1, __FILE__, __LINE__); # Release SGL
        b       .rw20
#
.rw30:
        mov     r13,g1                  # Restore g1
        call    r$comprrb               # Complete/release RRB
#
# --- Release ILT/PRP/SGL
#
.rw40:
        call    M$rip                   # Release ILT/PRP/SGL
#
# --- Exit
#
        mov     r12,g0                  # Restore g0
        mov     r13,g1                  # Restore g1
        mov     r14,g2                  # Restore g2
        mov     r15,g3                  # Restore g3
        ret
#
#**********************************************************************
#
#  NAME: r$r5fpwcomp  (Algorithm 1 and 2)
#
#  PURPOSE:
#       To provide a means of handling the completion of a normal full
#       stripe parity write operation.
#
#  DESCRIPTION:
#       The status of the completing PRP is checked. If an error has
#       an attempt is made to hotspare the device.
#
#       The RRB outstanding request count is adjusted. If no more
#       outstanding requests exist and an error has occurred for any PRP,
#       the RRB is checked to see if recovery has been invoked. If
#       not, the RRB is resubmitted for another try, else the RRB is
#       released.
#
#       The completing ILT/PRP and parity construction buffer/SGL are
#       released back to the system.
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
#      CALLER AREA
#      ___________
#      W0 = PRP
#      W1 = null
#      W2 = null
#      W3 = RRB
#      W4 = PSD
#
#**********************************************************************
#
r$r5fpwcomp:
        mov     g0,r12                  # Save g0
        mov     g1,r13                  # Save g1
        mov     g2,r14                  # Save g2
        mov     g3,r15                  # Save g3
#
# --- Check request status
#
        ld      il_w0(g1),g2            # Get PRP
        call    M$chkstat               # Check status
        mov     g2,r11                  # Save PRP
#
        ld      il_w3(r13),g2           # Get RRB
        cmpobe  ecok,g0,.pw10           # Jif OK
#
# --- Process read error
#
        call    r$seterr                # Set irrecoverable error
#
# --- Check outstanding request count (RRB)
#
.pw10:
        ldob    rb_orc(g2),r3           # Get outstanding request count
        subo    1,r3,r3                 #  and adjust
        stob    r3,rb_orc(g2)           # Update outstanding request count
        cmpobne 0,r3,.pw40              # Jif more
#
# --- Release all of the SGLs for the original data written.
#
        ld      rb_sgl0(g2),g0          # Get SGL/buffer from parity
        st      0,rb_sgl0(g2)           # make sure cannot use sgl pointer again
        call    M$rsglbuf               # Release SGL/buffer
#
        lda     rb_sgl1(g2),r3          # Get ptr to SGL to release
#
.pw20:
        ld      (r3),g0                 # Get next SGL/buffer
        addo    4,r3,r3                 # Advance pointer
        cmpobe  0,g0,.pw30              # Jif done
#
# --- Release SGL but not the buffers
#
        ld      sg_size(g0),g1          # Pass size of SGL
c       s_Free(g0, g1, __FILE__, __LINE__); # Release SGL
        b       .pw20
#
.pw30:
        mov     r13,g1                  # Restore ILT
        call    r$comprrb               # Complete/release RRB
#
# --- Release ILT/PRP
#
.pw40:
        call    M$rip                   # Release ILT/PRP
#
# --- Exit
#
        mov     r12,g0                  # Restore g0
        mov     r13,g1                  # Restore g1
        mov     r14,g2                  # Restore g2
        mov     r15,g3                  # Restore g3
        ret
#
#**********************************************************************
#
#  NAME: r$r5sdr3comp  (Algorithm 3)
#
#  PURPOSE:
#       To provide a common means of handling the completion of a recovery
#       read operation pursuant to a single drive write operation where
#       the original write operation was destined to a single drive of a 3
#       drive wide stripe.
#
#  DESCRIPTION:
#       The status of the completing reconstruction read PRP is checked.
#       If an error has occurred, the RRB is flagged with an irrecoverable
#       error and an attempt is made to hotspare this device. The parity
#       write ILT/PRP and associated SGL/buffer are released back to the
#       system. The data write ILT/PRP is released back to the system.
#       The RRB is completed and this routine exits after releasing the
#       reconstruction read ILT/PRP and SGL/buffer.
#
#       The parity data is generated from the reconstruction read and
#       the write data. The data and parity write ILTs are queued to
#       the physical layer. Next the reconstruction read ILT/PRP and
#       SGL/buffer are released and then this routine exits.
#
#       Note that the buffer for the reconstruction read and the parity
#       write are one in the same.
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
#      CALLER AREA
#      ___________
#      W0 = PRP
#      W1 = null
#      W2 = null
#      W3 = RRB
#      W4 = PSD
#      W5 = data write ILT
#      W6 = parity write ILT
#
#**********************************************************************
#
r$r5sdr3comp:
        mov     g0,r12                  # Save g0
        mov     g1,r13                  # Save g1
        mov     g2,r14                  # Save g2
        mov     g3,r15                  # Save g3
#
# --- Check request status
#
        ld      il_w0(g1),g2            # Get PRP
        call    M$chkstat               # Check status
        ld      il_w3(g1),g2            # Get RRB
        cmpobe  ecok,g0,.dr10           # Jif OK
#
# --- Process I/O error
#
        call    r$seterr                # Set irrecoverable error
#
# --- Release parity write ILT and SGL/buffer
#
        ld      il_w6(r13),g1           # Get parity write ILT
        ld      rb_sgl0(g2),g0          # Release parity write SGL/buffer
        st      0,rb_sgl0(g2)           # make sure cannot use sgl pointer again
        call    M$rsglbuf
        call    M$rip                   # Release ILT/PRP
#
# --- Release data write ILT
#
        ld      il_w5(r13),g1           # Get data write ILT
        call    M$rip                   # Release ILT/PRP
#
# --- Release RRB
#
        call    r$comprrb               # Complete/release RRB
        b       .dr20
#
# --- Generate parity data
#
.dr10:
#
# Data is successfully read from a device which is used to recalculate the
# new parity with the new data to be written to another single device.
#
# Now obtain the parity between the data read and the data to be written.
#
#   call r$sxor3sgls(g0= pRRB->pDstSGL, g1= pRRB->pSrc1SGL , g2=pRRB->pSrc2SGL)
#   call RL_Raid5XOR3Sgls (SGL* pDestSgl, SGL* pSrc1Sgl, SGL* pSrc2Sgl)
#
#        pRRB->pSrc1SGL  --- Contains  data to be written  to one data device
#        pRRB->pSrc2SGL  --- Existing data just read from another data device
#        pRRB->pDstSGL   --- Contains XORed data
#
# Very Important Note :- The buffer allocated for pSrc2SGL and pDstSGL is same.
#
#   Now load these 3 SGLs into the registers
#
        ld      rb_sgl0(g2),g0          # Get p0
        ld      rb_sgl1(g2),g1          # Get p1
        ld      rb_sgl2(g2),g2          # Get p2
#
#   Now perform XOR between g1 and g2 and store in g0(pRRB->dstSGL)
#
c    RL_XorSGL (3,g0,g1,g2);
#
#  Queue this parity(new parity calculated) write Request to Physical layer
#
#  r13 = Pointer to read construction ILT request
#  The w6 of this read reconstruction ILT request contains the parity write ILT,
#  the PRP  of which has SGL pointer that is pointed to destination SGL buffer.
#
        ld      P_que,g0                # Set physical queuing routine
        ld      il_w6(r13),g1           # Get parity write ILT
        lda     r$r5spdw3comp,g2        # Set completion routine
        call    K$q                     # Queue request w/o wait
#
#  Queue the data write ILT to the Physical Layer.
#  The w7 of the read reconstruction ILT request contains the data write ILT.
#  the PRP pointer of this ILT has SGL pointer that is pointed to SGL buffer
#  contains data to be written.
#
        ld      P_que,g0                # Set physical queuing routine
        ld      il_w5(r13),g1           # Get data write ILT
        call    K$q                     # Queue the request w/o wait

#
# --- Release reconstruction read ILT/PRP
#
.dr20:
        mov     r13,g1                  # Get reconstruction read ILT
        call    M$rip                   # Release ILT/PRP
#
# --- Exit
#
        mov     r12,g0                  # Restore g0
        mov     r13,g1                  # Restore g1
        mov     r14,g2                  # Restore g2
        mov     r15,g3                  # Restore g3
        ret
#
#**********************************************************************
#
#  NAME: r$r5spdw3comp  (Algorithm 3)
#
#  PURPOSE:
#       To provide a common means of handling the completion of a data
#       or parity write operation pursuant to a single drive write
#       operation where the targeted data drive is determined to be
#       operable.
#
#  DESCRIPTION:
#       The status of the completing data or parity write PRP is checked.
#       If an error has occurred, the RRB is flagged with an irrecoverable
#       error and an attempt is made to hotspare the device.
#
#       If this request is a parity write the SGL/buffer is released back
#       to the system. The ILT/PRP is released, the RRB is released and
#       this routine exits.
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
#      CALLER AREA
#      ___________
#      W0 = PRP
#      W1 = null
#      W2 = null
#      W3 = RRB
#      W4 = PSD
#      W5 = T/F for parity write
#
#**********************************************************************
#
r$r5spdw3comp:
        mov     g0,r12                  # Save g0
        mov     g1,r13                  # Save g1
        mov     g2,r14                  # Save g2
        mov     g3,r15                  # Save g3
#
# --- Check request status
#
        ld      il_w0(g1),g2            # Get PRP
        call    M$chkstat               # Check status
        mov     g2,r11                  # Save PRP
        ld      il_w3(r13),g2           # Get RRB
        cmpobe  ecok,g0,.sw10           # Jif OK
#
# --- Process  write error
#
        call    r$seterr                # Set irrecoverable error
#
# --- Release SGL/buffer if parity write
#
.sw10:
        ld      il_w5(r13),r3           # Get parity write flag
        cmpobne TRUE,r3,.sw20           # Jif data write
#
        ld      pr_sglptr(r11),g0       # Release parity SGL/buffer
        st      0,pr_sglptr(r11)        # make sure cannot use sgl pointer again
        call    M$rsglbuf
#
# --- Release ILT/PRP
#
.sw20:
        call    M$rip                   # Release ILT/PRP
#
# --- Release RRB
#
        ldob    rb_orc(g2),r3           # Adjust outstanding request cnt
        subo    1,r3,r3
        stob    r3,rb_orc(g2)
        cmpobne 0,r3,.sw100             # Jif pending I/O
#
        call    r$comprrb               # Complete/release RRB
#
# --- Exit
#
.sw100:
        mov     r12,g0                  # Restore g0
        mov     r13,g1                  # Restore g1
        mov     r14,g2                  # Restore g2
        mov     r15,g3                  # Restore g3
        ret
#
#**********************************************************************
#
# --- SCSI command equivalences to rb_type
#
        .data
r_scmd:
        .byte   0x28                    # SCSI read-10          0
        .byte   0x2a                    # SCSI write-10         1
        .byte   0x2e                    # SCSI write/verify-10  2
        .byte   0                       # rbrebuild             3
        .byte   0                       # rbverifyc             4
        .byte   0                       # rbverify              5
        .byte   0                       # rbparitychk           6
        .byte   0                       # rbconsistentchk       7
        .byte   0                       # rbrebuildchk          8
#
r_scmd_16:
        .byte   0x88                    # SCSI read-16          0
        .byte   0x8a                    # SCSI write-16         1
        .byte   0x8e                    # SCSI write/verify-16  2
        .byte   0                       # rbrebuild             3
        .byte   0                       # rbverifyc             4
        .byte   0                       # rbverify              5
        .byte   0                       # rbparitychk           6
        .byte   0                       # rbconsistentchk       7
        .byte   0                       # rbrebuildchk          8
#
        .text
#
#**********************************************************************
#
#  NAME: r$r5sdpr4comp   (Algorithm 4)
#
#  PURPOSE:
#       To provide a means of handling the completion of a read operation
#       when a read/modify/write operation is used as part of a RAID 5
#       write operation to a single data drive. This logic concentrates
#       on the read completion of both the data and parity drives.
#
#  DESCRIPTION:
#       The status of the completing data read PRP is checked and the
#       outstanding request count is decremented. If an error has occurred
#       the RRB is flagged with an error and an attempt is made to hotspare
#       the device.
#
#       If the opposite read is still pending, an exit is made. If an error
#       has occurred with either read, both the data read and parity read
#       SGL/buffers are released along with their associated ILT/PRPs, the
#       RRB is completed and an exit is made.
#
#       The data read SGL/buffer is released and both ILT/PRPs are converted
#       from reads to writes. The write data SGL is attached to the data
#       PRP. Parity data is generated from the old parity, old data and new
#       data. Both ILTs are queued to the physical layer and an exit is made.
#
#       NOTE:  Both reads must complete before issuing either write request.
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
#      CALLER AREA
#      ___________
#      W0 = PRP
#      W1 = null
#      W2 = null
#      W3 = RRB
#      W4 = PSD
#      W5 = data ILT
#      W6 = parity ILT
#
#  RRB USAGE:
#      SGL0 = parity write (new)        Note: SGL0 = SGL1
#      SGL1 = parity read (old)
#      SGL2 = data read (old)
#      SGL3 = data write (new)
#
#**********************************************************************
#
r$r5sdpr4comp:
        movq    g0,r12                  # Save g0 - g3
#
# --- Check request status
#
        ld      il_w0(g1),g2            # Get PRP
        call    M$chkstat               # Check status
#
        ld      il_w3(g1),g2            # Get RRB
        ldob    rb_orc(g2),r9           # Get outstanding request count
        subo    1,r9,r9                 #  and adjust
        stob    r9,rb_orc(g2)           # Update outstanding request count
#
        cmpobe  ecok,g0,.dx10           # Jif PRP status OK
#
# --- Process I/O error
#
        call    r$seterr                # Set irrecoverable error
#
# --- Check for opposite read completion
#
.dx10:
        cmpobe  3,r9,.dx100             # Jif opposite read pending
#
# --- Check for previous error w/ parity read operation
#
        ldob    rb_rstat(g2),r3         # Get RRB status
        ld      il_w6(g1),r10           # Save parity ILT/PRP
        ld      il_w5(g1),r7            # Save data ILT/PRP
        cmpobe  ecok,r3,.dx20           # Jif RRB status OK
#
# --- Release data read SGL/buffer and ILT/PRP
#
        ld      rb_sgl2(g2),g0          # Release data read SGL/buffer
        st      0,rb_sgl2(g2)           # make sure cannot use sgl pointer again
        call    M$rsglbuf
        mov     r7,g1                   # Get data ILT/PRP
        call    M$rip                   # Release ILT/PRP
#
# --- Release parity read/write SGL/buffer and ILT/PRP
#
        ld      rb_sgl0(g2),g0          # Release parity read/write SGL/buffer
        st      0,rb_sgl0(g2)           # make sure cannot use sgl pointer again
        call    M$rsglbuf
        mov     r10,g1                  # Get parity ILT/PRP
        call    M$rip                   # Release ILT/PRP
#
# --- Complete request
#
        call    r$comprrb               # Complete/release RRB
        b       .dx100
#
# --- Convert both ILT/PRPs from reads to writes
#
.dx20:
        ld      il_w0(r10),r9           # Get parity PRP
        ldconst proutput,r3             # Set function to O/P
        ld      il_w0(r7),r11           # Get data PRP
        stob    r3,pr_func(r9)
        stob    r3,pr_func(r11)
#
        ldob    rb_type(g2),r4          # Get request type
c   if ((((PRP*)r9)->sda & ~0xffffffffULL) != 0ULL) { # 16 byte command
        ldob    r_scmd_16(r4),r3        # Lookup SCSI command
c   } else {
        ldob    r_scmd(r4),r3           # Lookup SCSI command
c   }
        stob    r3,pr_cmd(r9)           # Update parity SCSI command
c   if ((((PRP*)r11)->sda & ~0xffffffffULL) != 0ULL) { # 16 byte command
        ldob    r_scmd_16(r4),r3        # Lookup SCSI command
c   } else {
        ldob    r_scmd(r4),r3           # Lookup SCSI command
c   }
        stob    r3,pr_cmd(r11)          # Update data SCSI command
#
# --- Attach original SGL for data write
#
        ld      rb_sgl(g2),r4           # Set up data write SGL
        ld      rb_sglsize(g2),r5       # Set up data write SGL size
        st      r4,pr_sglptr(r11)
        setbit  31,r5,r5                # Set up data write SGL size
        st      r5,pr_sglsize(r11)      #  as borrowed

#
# --- Generate parity data
#
# --- XOR the SGL buffers  SGL0 = SGL1 ^ SGL2 ^ SGL3
#
#   call r$sxor4sgls(g0= pRRB->pDstSGL, g1= pRRB->pSrc1SGL , g2=pRRB->pSrc2SGL,
#                    g3 = pRRB->pSrc3SGL)
#   call RL_Raid5XOR4Sgls (SGL* pDestSgl, SGL* pSrc1Sgl, SGL* pSrc2Sgl,
#                           SGL* pSrc3SGL)
#
#
#     Now, the buffer SGL1 contains the old parity data. Resultant XOR will
#     be stored in the same buffer , also pointed by SGL0
#
# --- Get all the XORed buffers , contained in RRB to the registers
# --- g0, g1, g2 and g3 and XOR them(g1 and g2 and g3 and put result in g0).
#
#     g0 = destination buffer pointer
#     g1 = old parity data buffer pointer
#     g2 = old data buffer pointer
#     g3 = new data buffer pointer
#
#     Very Important Note : The buffer pointed by SGL1(g0) and SGL0(g1) is same.

        ld      rb_sgl0(g2),g0          # Get p0
        ld      rb_sgl1(g2),g1          # Get p1
        ld      rb_sgl3(g2),g3          # Get p3
        ld      rb_sgl2(g2),g2          # Get p2

c       RL_XorSGL(4,g0,g1,g2,g3);

# --- Now  release the SGL/buffer allocated to read the old data
#
#     Note :- This piece is implemented in r$r5sdpr4comp2() routine, when hw
#             XOR is used
        mov     g2,g0
        call    M$rsglbuf               # Release SGL/buffer

# --- Submit the parity write and data write ILTs to the physical layer.
#     These portions brought from r$r5sdpr4comp2(), when hw XOR is used with
#     minimum modifications.
#     Note:- In Mag classic, Ist data write and then parity write are queued.
#

#
# --- Queue physical request (parity write)
#
        mov     r10,g1                  # Get parity write ILT/PRP
.if     CHKDEBUG                        # Skip the parity write
        ld      R_pcctrl,r3
        bbs     rdpccorrupt,r3,.r5sdpr4comp_10 # Check corruption bit
.endif  # CHKDEBUG
#
        ld      P_que,g0                # Set physical queuing routine
        lda     r$r5spw4comp,g2         # Set completion routine
        call    K$q                     # Queue request w/o wait
#
.if     CHKDEBUG                        # Skip the parity write
        b       .r5sdpr4comp_20
#
.r5sdpr4comp_10:
        call    r$r5spw4comp            # Call the completion routine directly
.r5sdpr4comp_20:
.endif  # CHKDEBUG

#
# --- Queue physical request (data write)
#
        ld      P_que,g0                # Set physical queuing routine
        mov     r7,g1                   # Get data write ILT.
        lda     r$r5sdw4comp,g2         # Set completion routine
        call    K$q                     # Queue request w/o wait
#
# --- Exit
#
.dx100:
        movq    r12,g0                  # Restore g0 - g3
        ret
#
#**********************************************************************
#
#  NAME: r$r5spw4comp  (Algorithm 4)
#
#  PURPOSE:
#       To provide a means of handling the completion of a write operation
#       when a read/modify/write operation is used as part of a RAID 5
#       write operation to a single data drive. This logic concentrates
#       on the write completion of the parity drive.
#
#  DESCRIPTION:
#       The status of the completing data write PRP is checked before
#       releasing the ILT/PRP. If an error has occurred the RRB is
#       flagged with an error and an attempt is made to hotspare the
#       device.
#
#       The parity SGL/buffer is released followed by the ILT/PRP. If
#       a data operation is still outstanding, this routine exits.
#
#       If the RRB status is ok, the RRB is released and this routine
#       exits. Otherwise the RRB is resubmitted and this routine exits.
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
#      CALLER AREA
#      ___________
#      W0 = PRP
#      W1 = null
#      W2 = null
#      W3 = RRB
#      W4 = PSD
#
#  RRB USAGE:
#      SGL0 = parity write (new)        Note: SGL0 = SGL1
#      SGL1 = parity read (old)
#      SGL2 = data read (old)
#      SGL3 = data write (new)
#
#**********************************************************************
r$r5sdw4comp:
        lda     FALSE,r3                # Clear parity write indicator
        b       .xp10
#
r$r5spw4comp:
        lda     TRUE,r3                 # Set parity write indicator
.xp10:
        mov     g0,r12                  # Save g0
        mov     g1,r13                  # Save g1
        mov     g2,r14                  # Save g2
        mov     g3,r15                  # Save g3
#
# --- Check request status
#
.if     CHKDEBUG
        ld      R_pcctrl,r4             # Don't chk status on skipped
                                        #  parity write
        bbs     rdpccorrupt,r4,.r5spw4comp_15
.endif  # CHKDEBUG
#
        ld      il_w0(g1),g2            # Get PRP
        call    M$chkstat               # Check status
#
.if     CHKDEBUG                        # Skip the parity write
        b       .r5spw4comp_17
#
.r5spw4comp_15:
        ldconst ecok,g0                 # Skipped parity writes always good
.r5spw4comp_17:
.endif  # CHKDEBUG
        ld      il_w3(r13),g2           # Get RRB
#
        ldob    rb_orc(g2),r10          # Get outstanding request count
        subo    1,r10,r10               #  and adjust
        stob    r10,rb_orc(g2)          # Update outstanding request count
#
        cmpobe  ecok,g0,.xp20           # Jif status OK
#
# --- Process I/O error
#
        call    r$seterr                # Set irrecoverable error
#
# --- Release parity SGL/buffer if parity write
#
.xp20:
        cmpobne TRUE,r3,.xp30           # Jif not parity write
#
        ld      rb_sgl0(g2),g0          # Release parity SGL/buffer
        st      0,rb_sgl0(g2)           # make sure cannot use sgl pointer again
        call    M$rsglbuf
#
# --- Release ILT/PRP
#
.xp30:
        call    M$rip                   # Release ILT/PRP
        cmpobne 0,r10,.xp100            # Jif opposite write outstanding
#
        call    r$comprrb               # Complete/release RRB
#
# --- Exit
#
.xp100:
        mov     r12,g0                  # Restore g0
        mov     r13,g1                  # Restore g1
        mov     r14,g2                  # Restore g2
        mov     r15,g3                  # Restore g3
        ret
#
#**********************************************************************
#
#  NAME: r$r5srrcomp  (Algorithm 5)
#
#  PURPOSE:
#       To provide a common means of handling the completion of a
#       recovery read operation pursuant to a single drive write
#       operation where the targeted data drive is determined to be
#       inoperable.
#
#  DESCRIPTION:
#       The status of the completing reconstruction read PRP is checked.
#       If an error has occurred, the RRB is flagged with an irrecoverable
#       error.
#
#       This reconstruction read ILT/PRP is released after unlinking
#       the SGL from the PRP. The outstanding request count is adjusted
#       downward to account for the reconstruction read ILT/PRP just
#       completed. If outstanding requests exist, this routine exits.
#
#       If the RRB indicates that an error has occurred, the parity
#       write and read reconstruction SGL/buffers are released after
#       releasing the parity write ILT/PRP. The RRB is then released
#       and this routine exits.
#
#       The data from the original write and the data from all of the
#       reconstruction reads is xor'ed together to produce the new
#       parity data. The reconstruction SGL/buffers are released back
#       to the system and the parity write ILT/PRP is queued to the
#       physical layer. The routine then exits.
#
#  INPUT:
#       g1 = ILT
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#  ILT USAGE:
#      CALLER AREA
#      ___________
#      W0 = PRP
#      W1 = null
#      W2 = null
#      W3 = RRB
#      W4 = PSD
#      W6 = parity write ILT
#
#**********************************************************************
#
r$r5srrcomp:
        movq    g0,r12                  # Save g0-g3
#
# --- Check request status
#
        ld      il_w0(g1),g2            # Get PRP
        call    M$chkstat               # Check status
        ld      il_w3(r13),g2           # Get RRB
        cmpobe  ecok,g0,.sr10           # Jif OK
#
# --- Process reconstruction read error
#
        call    r$seterr                # Set irrecoverable error
#
# --- Release reconstruction read ILT/PRP
#
.sr10:
        ld      il_w6(g1),r10           # Get parity write ILT
        call    M$rip                   # Release ILT/PRP
#
# --- Check outstanding request count (RRB)
#
        ldob    rb_orc(g2),r3           # Get outstanding request count
        subo    1,r3,r3
        stob    r3,rb_orc(g2)           # Update outstanding request count
        cmpobne 1,r3,.sr100             # Jif more
#
        ldob    rb_rstat(g2),r3         # Get request status
        cmpobe  ecok,r3,.sr30           # Jif OK
#
# --- On error release parity write ILT/PRP
#
        mov     r10,g1
        call    M$rip                   # Release ILT/PRP
#
# --- Release parity write and read reconstruction SGL/buffers
#
        ld      rb_sgl0(g2),g0          # Release parity write SGL/buffer
        st      0,rb_sgl0(g2)           # make sure cannot use sgl pointer again
        lda     rb_sgl2(g2),r3          # Set pointer to next release
.sr20:
        call    M$rsglbuf               # Release next SGL/buffer
        ld      (r3),g0                 # Get next SGL/buffer
        st      0,(r3)                  # make sure cannot use sgl pointer again
        addo    4,r3,r3                 # Advance pointer
        cmpobne 0,g0,.sr20              # Jif more
#
# --- Release RRB
#
        call    r$rrrb                  # Release RRB
        b       .sr100
#
# --- Generate parity data
#
.sr30:
#
# Now XOR the data buffers stored in source SGL pointers of RRB.
#
# g2 = pRRB
#
# pRRB->pDestSGL= rb_sgl0(g2) =  SGL buffer contains the XORed result of all the
#                                following sources.
# pRRB->pSrc1SGL = rb_sgl1(g2) = SGL buffer containing the data (new) to be
#                                written to target drive (that is failed)
# pRRB->pSrc2SGL =  ---|
# pRRB->pSrc3SGL =     |
# . . . . . . .  =     |    SGL buffers contains the data read from all the data
# . . . . . . .  =     |    drives (except failed one) in the stripe.
# . . . . . . .  =     |
# pRRB->pSrcnSGL =  ---|
#
#
#
# --- Get pointer to 1st SGL/buffer, containing the data read from first data
# --- drive out of (n-1) data drives.(Preparing for release of the buffers)
#
        lda     rb_sgl2(g2),r3
#
# --- Obtain the parity for the above SGLs

        call    r$sxorsgls              # XOR the SGLs
#
# --- Release read reconstruction SGL/buffers(Buffers containing the data read
# --- from all the (n-1) data drives)
#
.sr40:
        ld      (r3),g0                 # Get next SGL/buffer
        cmpobe.f 0,g0,.sr50             # Jif done
#
        st      0,(r3)                  # make sure cannot use sgl pointer again
        call    M$rsglbuf               # Release next SGL/buffer
        addo    4,r3,r3                 # Advance pointer
        b       .sr40

#
# --- Queue physical request (parity write)
#
.sr50:
        ld      P_que,g0                # Set physical queuing routine
#
# --- Parity write ILT is available is available in r10 as well as in
# --- rb_pwilt(g2)
#
        mov     r10,g1                  # Get parity write ILT
        lda     r$r5spwcomp,g2          # Set completion routine
        call    K$q                     # Queue request w/o wait
#
#
# --- Exit
#
.sr100:
        movq    r12,g0                  # Restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: r$r5spwcomp  (Algorithm 5)
#
#  PURPOSE:
#       To provide a common means of handling the completion of a parity
#       write operation pursuant to a single drive write operation where
#       the targeted data drive is determined to be inoperable.
#
#  DESCRIPTION:
#       The status of the completing parity write PRP is checked. If an
#       error has occurred, the RRB is flagged with an irrecoverable
#       error.
#
#       The parity write SGL/buffer is released back to the system
#       followed by the ILT/PRP. The RRB is released and this routine
#       exits
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
#      CALLER AREA
#      ___________
#      W0 = PRP
#      W1 = null
#      W2 = null
#      W3 = RRB
#      W4 = PSD
#
#**********************************************************************
#
r$r5spwcomp:
        movq    g0,r12                  # Save g0-g3
#
# --- Check request status
#
        ld      il_w0(g1),g2            # Get PRP
        call    M$chkstat               # Check status
        mov     g2,r11                  # Save PRP
        ld      il_w3(r13),g2           # Get RRB
        cmpobe  ecok,g0,.sp10           # Jif OK
#
# --- Process parity write error
#
        call    r$seterr                # Set irrecoverable error
#
# --- Release parity write SGL/buffer and ILT/PRP
#
.sp10:
        ld      pr_sglptr(r11),g0       # Release SGL/buffer
        st      0,pr_sglptr(r11)        # make sure cannot use sgl pointer again
        call    M$rsglbuf
#
        call    M$rip                   # Release ILT/PRP
#
# --- Release RRB
#
        call    r$rrrb                  # Release RRB
#
# --- Exit
#
        movq    r12,g0                  # Restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: r$r5a6msglrw (fore)  (algorithm 6)
#
#  PURPOSE:
#       To provide a common means of merging the SGLs for a read and
#       write request for algorithm 6.
#
#  DESCRIPTION:
#       This routine takes the SGLs from the most recently queued
#       read ILT and write ILT within their respective RRB submission
#       queues. These SGLs are merged into a single SGL and placed
#       into the fore merged SGL slot in the RRB and also into the
#       next RRB SGL ptr for use by the XOR algorithms.
#
#  INPUT:
#       g2  = RRB
#       g12 = RRB SGL ptr index
#
#  OUTPUT:
#       g12 = next RRB SGL ptr index
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
r$r5a6msglrw:
        mov     g0,r14                  # Save g0
        mov     g1,r15                  # Save g1
#
# --- Get read and write SGLs
#
        ld      rb_wsq(g2),r4           # Get write ILTs
        ld      rb_rsq(g2),r5           # Get read ILTs
#
        ld      il_w0(r4),r6            # Get write PRP
        ld      il_w0(r5),r7            # Get read PRP
#
        ld      pr_sglptr(r6),g1        # Get write SGL
        ld      pr_sglptr(r7),g0        # Get read SGL
#
# --- Merge read and write SGLs
#
c       g0 = PM_MergeSGL(&g1, g0, g1);  # Merge SGLs
        st      g0,rb_fmsgl(g2)         # Set fore merged SGL
        st      g0,rb_sgl1(g2)[g12*4]   # Set RRB SGL terminator
        addo    1,g12,g12               # Advance RRB SGL ptr index
#
# --- Exit
#
        mov     r14,g0                  # Restore g0
        mov     r15,g1                  # Restore g1
        ret
#
#**********************************************************************
#
#  NAME: r$r5a6msglwr (aft)  (algorithm 6)
#
#  PURPOSE:
#       To provide a common means of merging the SGLs for a write and
#       read request for algorithm 6.
#
#  DESCRIPTION:
#       This routine takes the SGLs from the most recently queued
#       write ILT and read ILT within their respective RRB submission
#       queues. These SGLs are merged into a single SGL and placed
#       into the fore merged SGL slot in the RRB and also into the
#       next RRB SGL ptr for use by the XOR algorithms.
#
#  INPUT:
#       g2  = RRB
#       g12 = RRB SGL ptr index
#
#  OUTPUT:
#       g12 = next RRB SGL ptr index
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
r$r5a6msglwr:
        mov     g0,r14                  # Save g0
        mov     g1,r15                  # Save g1
#
# --- Get write and read SGLs
#
        ld      rb_wsq(g2),r4           # Get write ILTs
        ld      rb_rsq(g2),r5           # Get read ILTs
#
        ld      il_w0(r4),r6            # Get write PRP
        ld      il_w0(r5),r7            # Get read PRP
#
        ld      pr_sglptr(r6),g0        # Get write SGL
        ld      pr_sglptr(r7),g1        # Get read SGL
#
# --- Merge read and write SGLs
#
c       g0 = PM_MergeSGL(&g1, g0, g1);  # Merge SGLs
        st      g0,rb_amsgl(g2)         # Set aft merged SGL
        st      g0,rb_sgl1(g2)[g12*4]   # Set RRB SGL terminator
        addo    1,g12,g12               # Advance RRB SGL ptr index
#
# --- Exit
#
        mov     r14,g0                  # Restore g0
        mov     r15,g1                  # Restore g1
        ret
#
#**********************************************************************
#
#  NAME: r$r5a6rcomp  (Algorithm 6)
#
#  PURPOSE:
#       To provide a common means of handling the completion of all
#       read operations pursuant to an algorithm 6 write operation.
#
#  DESCRIPTION:
#       The status of the completing read PRP is checked. If an error
#       has occurred, the RRB is flagged with an irrecoverable error
#       and an attempt is made to hotspare this device. The completing
#       request is inserted into the read completion queue.
#
#       The outstanding RRB request count is decremented. If any
#       requests remain, this routine exits.
#
#       If the RRB status is OK, the XOR parity calculation is performed
#       at this time with the result being placed into the parity write
#       buffer.
#
#       All read ILT/PRP/SGL/buffers are extracted from the read
#       completion queue and released back to the system. Any possible
#       merged SGLs (fore and aft) are also released at this time.
#
#       If the RRB status is not OK, all data and parity write requests
#       are released back to the system and the RRB is completed prior
#       to exiting.
#
#       All physical write requests for data and parity are queued to
#       the physical layer and the RRB outstanding request count is
#       set to the number of write requests.
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
#      CALLER AREA
#      ___________
#      W0 = PRP
#      W1 = null
#      W2 = null
#      W3 = RRB
#      W4 = PSD
#
#**********************************************************************
#
r$r5a6rcomp:
        mov     g0,r12                  # Save g0
        mov     g1,r13                  # Save g1
        mov     g2,r14                  # Save g2
        mov     g3,r15                  # Save g3
#
# --- Check request status
#
        ld      il_w0(g1),g2            # Get PRP
        call    M$chkstat               # Check status
#
        ld      il_w3(g1),g2            # Get RRB
        cmpobe  ecok,g0,.ar10           # Jif OK
#
# --- Process read error
#
        call    r$seterr                # Set irrecoverable error
#
# --- Insert ILT into head of read completion queue
#
.ar10:
        ld      rb_rsq(g2),r3           # Get completion queue head
        st      g1,rb_rsq(g2)           # Set new queue head
        st      r3,il_fthd(g1)          # Link new to previous
#
# --- Check outstanding request count (reads)
#
        ldob    rb_orc(g2),r3           # Get outstanding request count
        subo    1,r3,r3                 #  and adjust
        stob    r3,rb_orc(g2)           # Update outstanding request count
        cmpobne 0,r3,.ar100             # Jif more
#
        ldob    rb_rstat(g2),r11        # Get request status
        cmpobne ecok,r11,.ar30          # Jif not OK
#
# --- Generate parity data
#
# --- Now XOR the data buffers stored in source SGL pointers of RRB(g2).
#
        call    r$sxorsgls              # XOR the SGLs
#
# --- Release all read ILT/PRP/SGL/buffers from completion queue
#
.ar30:
        ld      rb_rsq(g2),g1           # Get 1st ILT
.ar40:
        ld      il_fthd(g1),r3          # Save next ILT
        ld      il_w0(g1),g0            # Get current PRP
        ld      pr_sglptr(g0),g0        # Get SGL
        call    M$rsglbuf               # Release combined SGL and buffer
        call    M$rip                   # Release ILT and PRP
        mov     r3,g1                   # Swap next to current ILT
        cmpobne 0,r3,.ar40              # Jif more
#
# --- Release any possible merged SGLs (fore and aft)
#
        ld      rb_fmsgl(g2),r4         # Get possible fore merged SGLs
        ld      rb_amsgl(g2),r5         # Get possible aft merged SGLs
        cmpobe  0,r4,.ar50              # Jif no fore merged SGL
#
        mov     r4,g0                   # Get fore merged SGL
        ld      sg_size(g0),g1          # Get length
c       s_Free(g0, g1, __FILE__, __LINE__); # Release SGL
#
.ar50:
        cmpobe  0,r5,.ar60              # Jif no aft merged SGL
#
        mov     r5,g0                   # Get aft merged SGL
        ld      sg_size(g0),g1          # Get length
c       s_Free(g0, g1, __FILE__, __LINE__); # Release SGL
#
# --- Check status of RRB
#
# V1 comments : Putting the RRB Status check redundantly in the following
#               section is necessary only in case of SW XOR process is used.
#               In this case, the logic should know, whether the SW XOR is done
#               or not. Only then the requests will be submitted to Physical
#               layer, otherwise the requests are terminated/released.
#             : In case of HW XOR processing, this is check is void, as in this
#               case, the  control will never comes to this place.

.ar60:
        cmpobe  ecok,r11,.ar80          # Jif RRB status OK
#
# --- Dispose of parity write request
#
        ld      rb_pwilt(g2),g1         # Get parity write ILT
        ld      il_w0(g1),g0            # Get current PRP
        ld      pr_sglptr(g0),g0        # Get SGL
        call    M$rsglbuf               # Release combined SGL and buffer
        call    M$rip                   # Release ILT and PRP
#
# --- Dispose of all data write requests
#
        ld      rb_wsq(g2),g1           # Get 1st data write ILT
.ar70:
        ld      il_fthd(g1),r3          # Save next ILT
        call    M$rip                   # Release ILT/PRP/SGL
        mov     r3,g1                   # Swap next to current ILT
        cmpobne 0,r3,.ar70              # Jif more
#
# --- Perform RRB completion
#
        call    r$comprrb               # Complete/release RRB
        b       .ar100
#
# --- Queue all physical write requests (data and parity)
#
.ar80:
        mov     g2,r10                  # Save RRB
        ld      rb_pwilt(g2),g1         # Get parity write ILT
        mov     0,r9                    # Clear request counter
        ld      rb_wsq(g2),r3           # Get 1st data write ILT
        ld      P_que,g0                # Set physical queuing routine
        lda     r$r5a6pwcomp,g2         # Set parity write completion
                                        #  routine
.ar90:
        call    K$q
        addo    1,r9,r9                 # Bump request counter
        cmpobe  0,r3,.ar95              # Jif complete
#
        lda     r$r5a6dwcomp,g2         # Set data write completion
                                        #  routine
        mov     r3,g1                   # Swap next to current ILT
        ld      il_fthd(r3),r3          # Get next ILT
        b       .ar90
#
# --- Set up outstanding request count (writes)
#
.ar95:
        stob    r9,rb_orc(r10)          # Set up outstanding request
                                        #  count inclusive of all data
                                        #  and parity writes
#
# --- Exit
#
.ar100:
        mov     r12,g0                  # Restore g0
        mov     r13,g1                  # Restore g1
        mov     r14,g2                  # Restore g2
        mov     r15,g3                  # Restore g3
        ret
#
#**********************************************************************
#
#  NAME: r$r5a6dwcomp/r$r5a6pwcomp  (Algorithm 6)
#
#  PURPOSE:
#       To provide a common means of handling the completion of all
#       write operations including data and parity pursuant to an
#       algorithm 6 write operation. The first entry point handles the
#       completion of data writes. The second entry point handles the
#       completion of parity writes.
#
#  DESCRIPTION:
#       The status of the completing write PRP is checked. If an error
#       has occurred, the RRB is flagged with an irrecoverable error
#       and an attempt is made to hotspare this device.
#
#       The ILT/PRP/SGL is released for a data write request. The ILT/PRP
#       and combined SGL/buffer is released for a parity write request.
#
#       The outstanding RRB request count is decremented. If any
#       requests remain, this routine exits. Otherwise, the NVA entry
#       is released and the RRB is completed.
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
#      CALLER AREA
#      ___________
#      W0 = PRP
#      W1 = null
#      W2 = null
#      W3 = RRB
#      W4 = PSD
#
#**********************************************************************
#
r$r5a6pwcomp:
        mov     TRUE,r11                # Set parity write completion
        b       .aw10
#
r$r5a6dwcomp:
        mov     FALSE,r11               # Clear parity write completion
.aw10:
        mov     g0,r12                  # Save g0
        mov     g1,r13                  # Save g1
        mov     g2,r14                  # Save g2
        mov     g3,r15                  # Save g3
#
# --- Check request status
#
        ld      il_w0(g1),g2            # Get PRP
        call    M$chkstat               # Check status
#
        ld      il_w3(g1),g2            # Get RRB
        cmpobe  ecok,g0,.aw20           # Jif OK
#
# --- Process read error
#
        call    r$seterr                # Set irrecoverable error
#
# --- Dispose of this request
#
.aw20:
        cmpobe  FALSE,r11,.aw30         # Jif data write
#
        ld      il_w0(g1),g0            # Get current PRP
        ld      pr_sglptr(g0),g0        # Get SGL
        call    M$rsglbuf               # Release combined SGL and buffer
.aw30:
        call    M$rip                   # Release ILT/PRP and possibly
                                        #  SGL
#
# --- Check outstanding request count (writes)
#
        ldob    rb_orc(g2),r3           # Get outstanding request count
        subo    1,r3,r3                 #  and adjust
        stob    r3,rb_orc(g2)           # Update outstanding request count
        cmpobne 0,r3,.aw100             # Jif more
#
# --- Perform RRB completion
#
        call    r$comprrb               # Complete/release RRB
#
# --- Exit
#
.aw100:
        mov     r12,g0                  # Restore g0
        mov     r13,g1                  # Restore g1
        mov     r14,g2                  # Restore g2
        mov     r15,g3                  # Restore g3
        ret
#
#**********************************************************************
#
#  NAME: r$r5fpdwcomp  (Algorithm 7)
#
#  PURPOSE:
#       To provide a means of handling the completion of a failed parity
#       (full or partial stripe) data write operation.
#
#  DESCRIPTION:
#       The RRB outstanding request count is adjusted. If no more
#       outstanding requests exist the RRB is released.
#
#       The completing ILT/PRP/SGL are then released back to the system.
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
#      CALLER AREA
#      ___________
#      W0 = PRP
#      W1 = null
#      W2 = null
#      W3 = RRB
#      W4 = PSD
#
#**********************************************************************
#
r$r5fpdwcomp:
        movt    g0,r12                  # Save g0-g2
#
# --- Check request status
#
        ld      il_w0(g1),g2            # Get PRP
        call    M$chkstat               # Check status
        ld      il_w3(r13),g2           # Get RRB
        cmpobe  ecok,g0,.fp10_5         # Jif OK
#
# --- Process write error
#
        call    r$seterr                # Set irrecoverable error
#
# --- Check outstanding request count (RRB)
#
.fp10_5:
        ldob    rb_orc(g2),r3           # Get outstanding request count
        subo    1,r3,r3                 #  and adjust
        stob    r3,rb_orc(g2)           # Update outstanding request count
        cmpobne 0,r3,.fp20_5            # Jif more
#
# --- Release RRB
#
        call    r$rrrb                  # Release RRB
#
# --- Release ILT/PRP/SGL
#
.fp20_5:
        call    M$rip                   # Release ILT/PRP/SGL
#
# --- Exit
#
        movt    r12,g0                  # Restore g0-g2
        ret
#
#**********************************************************************
#
#  NAME: r$seterr
#
#  PURPOSE:
#       To provide a common means of setting an irrecoverable I/O
#       error status within the RRB and attempting to hotspare the
#       failing device. This is only used by raid 5 devices.
#
#  DESCRIPTION:
#       The specified RRB is flagged with an irrecoverable error.
#       The PSD recorded within the specified ILT is used in an attempt
#       to hotspare this device.
#
#  CALLING SEQUENCE:
#       call    r$seterr
#
#  INPUT:
#       g0 = PRP error
#       g1 = ILT
#       g2 = RRB
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
r$seterr:
        movl     g0,r14                 # Save g0-g1
#
# --- Check for cancelled I/O
#
        ldconst eccancel,r3             # Check for I/O cancelled
        cmpobe  r3,g0,.szz100           # Jif so
#
# --- Set appropriate error in RRB
#
        ldconst ecioerr,r3              # Set irrecoverable error for now
        ld      il_w0(g1),r13           # Get PRP
        ldob    pr_sstatus(r13),r5      # Get SCSI status byte
        cmpobne 2,r5,.szz10             # Jif not check condition
#
# --- Don't try to hot spare if we had a miscompare check condition
#
        ldob    pr_sense+2(r13),r6      # Get sense key and isolate
        and     0x0f,r6,r6
        cmpobne 0xe,r6,.szz10           # Jif not miscompare
#
        ldconst eccompare,r3            # Set compare error
        stob    r3,rb_rstat(g2)         # Set error status
        b       .szz100
#
.szz10:
        call    r$hspare                # Attempt to hotspare this device
#
# --- Determine if the RAID is now in a state that a RETRY at the upper layers
#       is needed (but only for Write type operations).
#
        ldob    rb_rstat(g2),r5         # Get the current RRB Return Status
        ldconst ecretry,r6              # Determine if already in RETRY status
        ldob    rb_type(g2),r4          # Get request type
        ld      rb_rdd(g2),r7           # Get the RDD
        cmpobe  r5,r6,.szz100           # Jif in RETRY Status - do not chg

        ldconst ecspecial,r6
        cmpobe  r5,r6,.szz100           # Jif RETRY -  do not chg
        ldconst ecretry,r6              # Here r6 is expected to ecretry
#
        ldob    rd_astatus(r7),r8       # Get the RAID Additional Status
        cmpobe  rbwrite,r4,.szz20       # Jif Write
        cmpobne rbwritev,r4,.szz30      # Jif not a Write and Verify
.szz20:
        chkbit  rdalocalimageip,r8      # See if a Local Image in Progress state
        sele    r3,r6,r3                # If Local Image IP = RETRY, Else above
.szz30:
        stob    r3,rb_rstat(g2)         # Set Return Status
#
# --- Exit
#
.szz100:
        movl     r14,g0                 # Restore registers
        ret
#
#**********************************************************************
#
#  NAME: r$comprrb
#
#  PURPOSE:
#       To provide a common means of handling the completion of an
#       RRB request.
#
#  DESCRIPTION:
#       If the status of the RRB is OK the RRB is released and an exit
#       is made from this routine. Otherwise, a check is made to see
#       if this request was previously resubmitted. If so, the RRB is
#       released and an exit is made from this routine. Otherwise,
#       the RRB is resubmitted and the associated RPN is reactivated.
#
#  INPUT:
#       g2 = RRB
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g3
#
#**********************************************************************
#
r$comprrb:
#
# --- Validate request status
#
        ldob    rb_rstat(g2),r3         # Get request status
        ldconst ecretry,r4              # Determine if RETRY status
        cmpobe  ecok,r3,.ec10           # Jif OK
        cmpobe  r4,r3,.ec10             # Jif RETRY - will be retried later

        ldconst ecspecial,r4
        cmpobe  r4,r3,.ec10             # Jif special RETRY - will be retried later
#
        ldob    rb_stat(g2),r3          # Get status
        bbs     rbrecov,r3,.ec10        # Jif recovery previously invoked
#
# --- Invoke error recovery and reactivate request
#
        ld      rb_rpn(g2),g3           # Get RPN
        setbit  rbrecov,r3,r3           # Set recovery invoked
        clrbit  rbact,r3,r3             # Set request inactive
        stob    r3,rb_stat(g2)
#
c       r_actrpn(TRUE, g3);             # Reactivate RPN
        b       .ec100
#
# --- Release RRB
#
.ec10:

c       r3 = g0;                        # save g0
        call    r$rrrb                  # Release RRB
c       g0 = r3;                        # restore g0
#
# --- Exit
#
.ec100:
        ret
#
#**********************************************************************
#
#  NAME: r$rrrb
#
#  PURPOSE:
#       To provide a common means of dequeuing an RRB from an RPN and
#       releasing the RRB back to the system.
#
#  DESCRIPTION:
#       The SGL associated with the RRB is released back to the system
#       if it hasn't been borrowed. The RRB is removed from the RPN
#       RRB thread. If no other RRB entries exists for this RPN, the
#       RPN is released back to the system.
#
#       The pending RRB I/O count is decremented in the primary ILT.
#       If that count goes to zero, the primary ILT is completed.
#
#       If the RRB is a system RRB, that RRB is released and the above
#       actions are repeated for each RRB in the join thread.
#
#  INPUT:
#       g2 = RRB
#
#  OUTPUT:
#       g0 = 1 if RPN for RRB was released, else 0.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
r$rrrb:
        ldob    rb_stat(g2),r3          # Get RRB status
        movt    g1,r12                  # Save g1-g3
        ldconst 0,r8                    # Set flag that RPN not released.
#
# --- Check for existence of parent RRB
#
        bbc     rbbparent,r3,.rb20      # Jif no parent
#
# --- Process parented RRB --------------------------------------------
#
        ld      rb_prrb(g2),r11         # Get parent RRB
        ldob    rb_rstat(g2),r3         # Check status of completing RRB
        cmpobe  ecok,r3,.rb10           # Jif OK
#
        ldob    rb_rstat(r11),r4        # Get the current Parent RRB Status
        ldconst ecretry,r5              # Determine if already in Retry status
        cmpobe  r5,r4,.rb10             # Jif in RETRY - do not change status

        ldconst ecspecial,r5
        cmpobe  r5,r4,.rb10             # Jif special RETRY - do not change status

        stob    r3,rb_rstat(r11)        # Update parent RRB status
#
.rb10:
        ld      rb_prrb(r11),r10        # Get outstanding RRB count
        subo    1,r10,r10               # Adjust for this completion
        st      r10,rb_prrb(r11)
#
# --- Dequeue RRB and activate RPN
#
        call    r$dqrrb                 # Dequeue RRB and activate RPN
c       r8 = g0;                        # Set flag as to if RPN was released.
.ifdef M4_DEBUG_RRB
c CT_history_printf("%s%s:%u put_rrb 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_RRB
c       put_rrb(g2);                    # Release RRB
        cmpobne 0,r10,.rb100            # Jif other RRBs pending
#
        mov     r11,g2                  # Get parent RRB
        ld      rb_sgl(r11),g0          # Get SGL
        ld      rb_sglsize(r11),g1      # Get SGL size
        bbs     31,g1,.rb30             # Jif SGL borrowed
#
c       s_Free(g0, g1, __FILE__, __LINE__); # Release SGL
        b       .rb30
#
# --- Process non-parented RRB ----------------------------------------
#
# --- Dequeue RRB and activate RPN
#
.rb20:
        call    r$dqrrb                 # Dequeue RRB and activate RPN
c       r8 = g0;                        # Set flag as to if RPN was released.
#
# --- Check for system request ----------------------------------------
#
.rb30:
        ld      rb_jointhd(g2),r10      # Get join thread
        ldob    rb_rstat(g2),r9         # Get request status
        cmpobne 0,r10,.rb60             # Jif system request
#
# --- Update RRP status if error
#
.rb40:
        ld      rb_ilt(g2),g1           # Get primary ILT
        cmpobe  ecok,r9,.rb50           # Jif status OK
#
        ld      il_w0-ILTBIAS(g1),r3    # Get associated RRP
        ldconst ecretry,r5              # Determine if already in Retry status
        ldob    rr_status(r3),r4        # Get current RRP Status
        cmpobe  r4,r5,.rb50             # Do not change status if already Retry

        ldconst ecspecial,r5
        cmpobe  r4,r5,.rb50             # Jif Special RETRY - do not change status

        stob    r9,rr_status(r3)        # Set bad status
#
# --- Complete request if possible
#
.rb50:
        ld      il_w0(g1),r3            # Adjust outstanding RRB count
        subo    1,r3,r3
        st      r3,il_w0(g1)
        cmpibl  0,r3,.rb60              # Jif not complete
#
# --- Complete primary ILT
#
        ld      il_w5(g1),g0            # Get possible NVA
        ld      rb_rdd(g2),r3           # Pass RDD
# c printf ("***V: r$rrrb-10:calling r$comp.. g0 = %#x, g1 = %#x, g2 = %#x\n", g0, g1, g2);

# 2008-10-30 - put_vrp() is now in "c", so this no longer applies, but is ok.
##  V1 Note(Imp) :- In the context of wookie,the r$comp(above), takes control
##  from layer to layer and in Link Layer( the link layer completer task) frees
##  the VRP by calling PM_RelVRP. That function moves the VRP into g2,before
##  deletes it. So, by the time control comes here,g2 will not contain the RRB
##  as expected by the function M$rrrb. Similarly g0 also got disturbed, that will
##  used by M$rp3nva. Ensure that Link layer has preserved  these.
        balx    r$comp,r6               # Complete request
# c printf ("***V: r$rrrb-11:back from r$comp.. g0 = %#x, g1 = %#x, g2 = %#x\n", g0, g1, g2);
        cmpobe  0,g0,.rb60              # Jif null NVA
#
        call    M$rp3nva                # Release NVA
#
# --- Release RRB
#
.rb60:
.ifdef M4_DEBUG_RRB
c CT_history_printf("%s%s:%u put_rrb 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_RRB
c       put_rrb(g2);                    # Release RRB
        cmpobe  0,r10,.rb100            # Jif no additional RRB
#
        mov     r10,g2                  # Get next RRB
        ld      rb_fthd(r10),r10        # Save following RRB
#
# --- Release SGL of joined request if not borrowed
#
        ld      rb_sgl(g2),g0           # Get SGL
        ld      rb_sglsize(g2),g1       # Get SGL size
        bbs     31,g1,.rb40             # Jif SGL borrowed
#
c       s_Free(g0, g1, __FILE__, __LINE__); # Release SGL
        b       .rb40
#
# --- Exit
#
.rb100:
        movt    r12,g1                  # Restore g1-g3
c       g0 = r8;                        # Set flag as to if RPN was released.
        ret
#
#**********************************************************************
#
#  NAME: r$dqrrb
#
#  PURPOSE:
#       To provide a common means of dequeuing an RRB from its associated
#       RPN.
#
#  DESCRIPTION:
#       The SGL associated with the RRB is released if it has not been
#       borrowed. The RRB is removed from the RPN's RRB thread. If
#       no other RRBs exist, the RPN is released back to the system.
#       Otherwise the RPN is reactivated.
#
#  INPUT:
#       g2 = RRB
#
#  OUTPUT:
#       g0 = 1 if RPN for RRB was released, else 0.
#       g3 = RPN
#
#  REGS DESTROYED:
#       g0-g1
#
#**********************************************************************
#
r$dqrrb:
#
# --- Release SGL if not borrowed
#
        ld      rb_sgl(g2),g0           # Get SGL
        ld      rb_sglsize(g2),g1       # Get SGL size
        mov     0,r11                   # Set up zero constant
        bbs     31,g1,.dq10             # Jif SGL borrowed
#
c       s_Free(g0, g1, __FILE__, __LINE__); # Release SGL
#
# --- Unlink RRB from RPN
#
.dq10:
c       g1 = r_ulrrb((UINT32*)&g3, g2); # Unlink RRB from RPN
c       g0 = 0;                         # RPN not released.
        cmpobe  FALSE,g1,.dq100         # Jif RPN not empty
#
# --- Release RPN
#
        call    r$rrpn                  # Release RPN
# --- Note: g0 = 1 if RPN release, else 0.
        b       .dq1000
#
# --- Reactivate RPN
#
.dq100:
c       r_actrpn(TRUE, g3);             # Activate RPN
c       g0 = 0;                         # RPN not released.
#
# --- Exit
#
.dq1000:
# --- Note: g0 is return value.
        ret
#
#**********************************************************************
#
#  NAME: r$rrpn
#
#  PURPOSE:
#       To provide a means of releasing an RPN back to the system after
#       removal from the RDD RPN thread.
#
#  DESCRIPTION:
#       If the RPN is locked, an immediate exit is made. Otherwise,
#       the specified RPN is removed from the RDD RPN thread. Then the
#       RPN is released back to the system.
#
#  INPUT:
#       g3 = RPN
#
#  OUTPUT:
#       g0 = 1 if RPN for RRB was released, else 0.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
r$rrpn:
#
# --- Check for locked RPN
#
#??        ldob    rp_lock(g3),r6          # Get lock
        ld      rp_lock(g3),r6          # Get lock, activated, expedited, and reserved
        ld      rp_rdd(g3),r7           # Get RRD
c       g0 = 0;                         # If not releasing RPN
        cmpob   TRUE,r6                 # Check for locked RPN
        be      .rp100                  # Jif so
#
# --- Deactivate RPN
#
        call    r$deactrpn              # Deactivate RPN
#
# --- Remove RPN from RDD RPN thread
#
        ld      rp_fthd(g3),r4          # Get forward pointer
        ld      rp_bthd(g3),r5          # Get backward pointer
        cmpobe  0,r4,.rp10              # Jif forward ptr null
        cmpobe  0,r5,.rp30              # Jif backward ptr null
#
# --- Remove RPN from middle of thread
#
        st      r4,rp_fthd(r5)          # Remove RPN from middle of
        st      r5,rp_bthd(r4)          #  thread
        b       .rp40
#
.rp10:
        cmpobe  0,r5,.rp20              # Jif backward ptr null
#
# --- Remove RPN from tail of thread
#
        st      0,rp_fthd(r5)           # Close forward link
        b       .rp40
#
# --- Remove only RPN from thread
#
.rp20:
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        st      0,rd_rpnhead(r7)        # Clear RPN queue head
        b       .rp40
#
# --- Remove RPN from head of thread
#
.rp30:
        st      0,rp_bthd(r4)           # Close backward link
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        st      r4,rd_rpnhead(r7)       # Set up new RPN head
#
# --- Release RPN
#
.rp40:
.ifdef M4_DEBUG_RPN
c CT_history_printf("%s%s:%u put_rpn 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g3);
.endif # M4_DEBUG_RPN
c       put_rpn(g3);                    # Release RPN
c       g0 = 1;                         # If releasing RPN
#
# --- Exit
#
.rp100:
        ret
#
#**********************************************************************
#
#  NAME: R$log_R5inop
#
#  PURPOSE:
#       Log a message that the RAID 5 has gone Inoperative with a Parity
#       Scan Required.
#
#  DESCRIPTION:
#       Something has happened that requires a Resync of the Drive or a Stripe
#       but the RAID 5 is not in a state that allows the resync (missing drive
#       during failover, etc). Report the problem for Service to get a
#       Heads-Up on the forthcoming customer call.
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
R$log_R5inop:
#
        ldos    rd_rid(g0),r5           # Get the RID from the RDD
        ldos    rd_vid(g0),r6           # Get the VID from the RDD
        mov     g0,r12                  # Save g0
#
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst mleR5inop,r4            # Event code
        st      r4,mle_event(g0)        # Store as word to clear other bytes
        stos    r5,e_r5inop_rid(g0)     # Store the RID in the Error Log
        stos    r6,e_r5inop_vid(g0)     # Store the VID in the Error Log
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], e_r5inop_len);
        mov     r12,g0                  # Restore g0
        ret
#
#**********************************************************************
#
#  NAME: R$recover_R5inop
#
#  PURPOSE:
#       Attempt to recover the RAID 5 that has gone Inoperative with a Parity
#       Scan Required.
#
#  DESCRIPTION:
#       The code will attempt to recover the RAID 5 that has gone Inoperative
#       due to a Parity Scan Required. This requires checking to see if this
#       controller owns the VDisk, if the RAID 5 is in the correct state,
#       update the status, and execute a P2 Update to let all the other
#       controllers know of the change in RAID 5 status.
#
#  INPUT:
#       g0 = RID
#
#  OUTPUT:
#       g0 = Return Status
#
#  REGS DESTROYED:
#       g0
#
#**********************************************************************
#
R$recover_R5inop:
#
#   Check the RID to ensure it is valid
#
        ld      R_rddindx[g0*4],r4      # r4 = RDD
        ldconst deinvrid,r15            # Set up the return code as Invalid RID
        cmpobe  0,r4,.rrecoverr5inop_90 # Jif the RID is invalid
#
#   Check that we own this VDisk
#
        ldos    rd_vid(r4),g0           # g0 = VID associated with RAID
        ldconst de_vdisk_in_use,r15     # Set up the return code as Not Owned
#
        PushRegs(r3)                    # Save regs
        call    DL_AmIOwner             # Determine if we are the owner or not
        PopRegs(r3)                     # Restore regs
#
        cmpobe  FALSE,g0,.rrecoverr5inop_90 # Jif we do not own the VDisk
#
#   Ensure the RAID is in the proper state to allow recovery
#
        ldob    rd_type(r4),r5          # r5 = RAID Type
        ldob    rd_status(r4),r6        # r6 = RAID Status
        ldob    rd_astatus(r4),r7       # r7 = RAID AStatus
        ldos    rd_psdcnt(r4),r8        # r8 = Number of PSDs in this RAID
        ldob    rd_depth(r4),r13        # r13 = RAID 5 Algorithm (3, 5 or 9)
        ld      rd_psd(r4),r9           # r9 = PSD being processed (this first)
        ldconst FALSE,r10               # r10 = PSD with Rebuild not on Flag
        ldconst deinvrtype,r15          # Set return code for Invalid RAID Type
        cmpobne rdraid5,r5,.rrecoverr5inop_90 # Jif not RAID 5
        ldconst deinvop,r15             # Set return code for Invalid Operation
        mulo    r13,r8,r14              # r14 = PSD scan iterations needed
# c     r14 = r8 * r13;
        cmpobne rdinop,r6,.rrecoverr5inop_90 # Jif RAID is not Inoperative
        bbc     rdaparity,r7,.rrecoverr5inop_90 # Jif RAID not need Parity Sync
#
.rrecoverr5inop_50:
        mov     r13,r3                  # Get algorithm
        mov     FALSE,g0                # Clear double fault in same stripe flag
#
.rrecoverr5inop_55:
#
        ldob    ps_status(r9),r11       # r11 = PSD Status
        ldob    ps_astatus(r9),r12      # r12 = PSD Astatus
        cmpobe  psop,r11,.rrecoverr5inop_60 # Jif this PSD is operational
#
        cmpobe  TRUE,g0,.rrecoverr5inop_90 # Jif 2 bad PSDs in stripe - abort
#
        mov     TRUE,g0                 # Set 1st failure found
        bbs     psarebuild,r12,.rrecoverr5inop_60 # Jif Rebuild bit in PSD set
        ldconst TRUE,r10                # Set PSD found without Rebuild bit set
#
.rrecoverr5inop_60:
        ld      ps_npsd(r9),r9          # Link to next PSD
        subo    1,r14,r14               # Adjust scan iteration count
        cmpobe  0,r14,.rrecoverr5inop_70 # Jif done
#
        subo    1,r3,r3                 # Adjust remaining PSDs for algorithm
        cmpobne 0,r3,.rrecoverr5inop_55 # Jif more
#
        b       .rrecoverr5inop_50
#
#   Update the RAID Status, kick off any hotsparing that may have been held
#       up due to the Resync Required, and do a Local Image Update to let
#       other controllers know about the change
#
.rrecoverr5inop_70:
        ldconst deok,r15                # Set return code for OK
        ldconst rddegraded,r8           # Set RAID Status to Degraded
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        stob    r8,rd_status(r4)
        clrbit  rdaparity,r7,r7         # Clear the Parity Sync needed bit
        clrbit  rdar5srip,r7,r7         # Clear the Stripe Resync In Progress
                                        #  (just in case it is on).
        bbs     rdarebuild,r7,.rrecoverr5inop_80
        cmpobne FALSE,r10,.rrecoverr5inop_80 # Jif there is a PSD without Rebld
                                        #  bit set (need to get PSD in proper
                                        #  state - requires op to get to PSD and
                                        #  therefore leave so that an op will
                                        #  get to PSD)
        setbit  rdarebuild,r7,r7        # Set the Rebuild Required bit
.rrecoverr5inop_80:
        stob    r7,rd_astatus(r4)
        call    RB_SearchForFailedPSDs  # Kick off any hotspares that can occur
                                        #  since the Resync Required is fixed
        call    D$p2update              # Update the Local Image and Config
#
#   Exit
#
.rrecoverr5inop_90:
        mov     r15,g0                  # g0 is return code
        ret
#
#**********************************************************************
#
#  NAME: R$checkForR5Rebuilding
#
#  PURPOSE:
#       Determine if the RAID is the Rebuilding State
#
#  DESCRIPTION:
#       This code sets up and calls the "C" function to determine if the
#       RAID is in the Rebuilding state.
#
#  INPUT:
#       g0 = RDD
#
#  OUTPUT:
#       g0 = TRUE if the RAID is in the Rebuilding State
#            FALSE if the RAID is not in the Rebuilding State
#
#  REGS DESTROYED:
#       g0
#
#**********************************************************************
#
R$checkForR5Rebuilding:
#
#  Determine if this is a RAID 5 type or not. If not return False. Else
#   continue the investigation
#
        cmpobe  0,g0,.rchkr5rebld_10    # Jif NULL RDD passed in
        ldob    rd_type(g0),r3          # Get the RAID type
        cmpobe  rdraid5,r3,.rchkr5rebld_20 # Jif this is a RAID 5 type device
.rchkr5rebld_10:
        ldconst FALSE,g0                # Return FALSE to the requester
        b       .rchkr5rebld_100        # Done!
#
#  Determine if this RAID 5 is in the Rebuilding state or not
#
.rchkr5rebld_20:
        PushRegs                        # Save all G registers (stack relative)
# g0 (input) = RDD
        call    RB_IsRaidRebuildWriteActive # Determine if in Rebuild-Wrt State
# g0 (output) = bool (TRUE or FALSE)
        PopRegs                         # Restore g1 to g14 (stack relative)
#
#  All Done
#
.rchkr5rebld_100:
        ret
#
#**********************************************************************
#
#  NAME: R$checkForR5PSDRebuilding
#
#  PURPOSE:
#       Determine if the RAID is the Rebuilding State
#
#  DESCRIPTION:
#       This code sets up and calls the "C" function to determine if the
#       RAID is in the Rebuilding state.
#
#  INPUT:
#       g0 = PSD
#
#  OUTPUT:
#       g0 = TRUE if the RAID PSD is in the Rebuilding State
#            FALSE if the RAID PSD is not in the Rebuilding State
#
#  REGS DESTROYED:
#       g0
#
#**********************************************************************
#
R$checkForR5PSDRebuilding:
#
#  Determine if this is a RAID 5 type or not. If not return False. Else
#   continue the investigation
#
        cmpobe  0,g0,.rchkr5psdrebld_10 # Jif NULL PSD passed in
        ldos    ps_rid(g0),r3           # Get the RID from the PSD
        ld      R_rddindx[r3*4],r4      # Get the RDD
        cmpobe  0,r4,.rchkr5psdrebld_10 # Jif invalid RDD
        ldob    rd_type(r4),r5          # Get the RAID type
        cmpobe  rdraid5,r5,.rchkr5psdrebld_20 # Jif this is a RAID 5 type device
.rchkr5psdrebld_10:
        ldconst FALSE,g0                # Return FALSE to the requester
        b       .rchkr5psdrebld_100     # Done!
#
#  Determine if this RAID 5 is in the Rebuilding state or not
#
.rchkr5psdrebld_20:
        PushRegs                        # Save all G registers (stack relative)
# g0 (input) = PSD
        call    RB_IsPSDRebuildWriteActive # Determine if in Rebuild-Wrt State
# g0 (output) = bool (TRUE or FALSE)
        PopRegs                         # Restore g1 to g14 (stack relative)
#
#  All Done
#
.rchkr5psdrebld_100:
        ret
#
#****************************************************************************
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
