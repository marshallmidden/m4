#c $Id: raid.as 161041 2013-05-08 15:16:49Z marshall_midden $
#**********************************************************************
#
#  NAME: raid.as
#
#  PURPOSE:
#       To implement RAID functionality as a layer directly above the
#       physical SCSI I/O layer.  RAID levels 0, 1, 5 and 10 are
#       directly supported by this module.  Software support for the
#       (AAU) Application Accelerator Unit is also included for RAID 5.
#
#  FUNCTIONS:
#       R$init      - RAID initialization
#       R$que       - Queue RAID I/O request
#       r$comp      - RAID I/O common completion handler routine
#
#       This module employs these processes:
#
#       r$exec      - Executive (1 copy)
#       r$stats     - Statistics (1 copy)
#
#  Copyright (c) 1996-2010 Xiotech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- global function declarations ------------------------------------
#
        .globl  R$init                  # RAID initialization
        .globl  R$que                   # Queue RAID I/O request
        .globl  r$comp                  # RAID I/O common completion handler
#
# --- global data declarations ----------------------------------------
#
        .globl  R_exec_qu               # Main executive QCB - for debug
        .globl  R_orc
#
# --- component function declarations ---------------------------------
#
        .globl  r$hspare                # Hotspare a device
        .globl  r$comp                  # RRP completion
        .globl  r$rcd                   # Release a chain descriptor
#
# --- component data declarations -------------------------------------
#
# Debug
        .globl  R_errlock               # Error handling in progress lock
#
# --- global usage data definitions -----------------------------------
#
        .data
#
# --- local usage data definitions ------------------------------------
#
# --- Error handling
#
R_errlock:
        .word   0                       # Error lock - TRUE or FALSE
#
# --- executable code -------------------------------------------------
#
        .text
#**********************************************************************
#
#  NAME: R$init
#
#  PURPOSE:
#       To provide a means of initializing the RAID module at system
#       start up time.
#
#  DESCRIPTION:
#       The executive process is established and tabled for later use
#       when requests become queued to this module.
#
#       The RAID-5 related execs and initialization should be done in a new
#       r5$init function in raid5.as. JT
#
#  CALLING SEQUENCE:
#       call    R$init
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       g3 = FICB
#
#  REGS DESTROYED:
#       g0
#       g1
#       g2
#       g3
#
#**********************************************************************
#
R$init:
#
# --- Establish RAID executive process
#
        lda     r_exec,g0               # Establish executive process
        ldconst REXECPRI,g1
c       CT_fork_tmp = (ulong)"r_exec";
        call    K$fork
        st      g0,R_exec_qu+qu_pcb     # Save assigned PCB
#
# --- Initialize Debug Data Retrieval (DDR) R_exec_qu PCB entry
#
c       M_addDDRentry(de_rpcb, g0, pcbsiz);
#
# --- Initialize Debug Data Retrieval (DDR) R_exec_qu entry
#
        lda     R_exec_qu,g1            # Load address of R_exec_qu header
c       M_addDDRentry(de_reque, g1, 16); # Size of R_exec_qu header
#
# --- Establish RAID 5 executive process
#
        lda     r$r5exec,g0             # Establish RAID 5 process
        ldconst R5EXECPRI,g1
c       CT_fork_tmp = (ulong)"r$r5exec";
        call    K$fork
        st      g0,R_r5exec_qu+qu_pcb   # Save assigned PCB
#
# --- Initialize Debug Data Retrieval (DDR) R_r5exec_qu PCB entry
#
c       M_addDDRentry(de_r5pcb, g0, pcbsiz);
#
# --- Initialize DDR R_r5exec_qu entry
#
        lda     R_r5exec_qu,g1          # Load address of R_r5exec_qu header
c       M_addDDRentry(de_r5eque, g1, 16);    # Size of R_r5exec_qu header
#
# --- Establish statistics process
#
        lda     r$stats,g0              # Establish statistics process
        ldconst RSTATSPRI,g1
c       CT_fork_tmp = (ulong)"r$stats";
        call    K$fork
#
# --- Establish scrub process
#
        lda     r$scrub,g0              # Establish scrub process
        ldconst RSCRUBPRI,g1
c       CT_fork_tmp = (ulong)"r$scrub";
        call    K$fork
        st      g0,r_scrub_pcb
#
# --- Establish checker process
#
        lda     r$checker,g0            # Establish checker process
        ldconst RCHECKERPRI,g1
c       CT_fork_tmp = (ulong)"r$checker";
        call    K$fork
        st      g0,r_checker_pcb
#
# --- Exit
#
        ret
#
#**********************************************************************
#
#  NAME: r$stats
#
#  PURPOSE:
#       To provide a means of computing RAID device statistics for
#       this module.
#
#  DESCRIPTION:
#       For each defined RAID device the RDD is updated to provide the
#       average sector count and request per second statistics.  This
#       logic is performed within a loop that contains a 1 second
#       timeout.
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
r$stats:
        ldconst 0,g13                   # Preload zero constants
        ldconst 0,g12
#
# --- Delay for 1 second
#
.sa10_r:
        ldconst 1000,g0                 # Delay for 1000 ms
        call    K$twait
#
        lda     R_rddindx,r14           # Get RDD index
        ldconst MAXRAIDS/4,r15          # Get max groups of RAID devs
#
# --- Examine next group of RDDs
#
.sa20:
        ldq     (r14),r8                # Get next group of RDDs
        cmpobe  0,r8,.sa30              # Jif 0 null
#
        mov     r8,g0                   # Update this RDD
        bal     .sa100_r
#
.sa30:
        cmpobe  0,r9,.sa40              # Jif 1 null
#
        mov     r9,g0                   # Update this RDD
        bal     .sa100_r
#
.sa40:
        cmpobe  0,r10,.sa50             # Jif 2 null
#
        mov     r10,g0                  # Update this RDD
        bal     .sa100_r
#
.sa50:
        cmpobe  0,r11,.sa60             # Jif 3 null
#
        mov     r11,g0                  # Update this RDD
        bal     .sa100_r
#
# --- Advance to next group of RDDs
#
.sa60:
        lda     16(r14),r14             # Advance to next group
#
# --- Check for additional group
#
        subo    1,r15,r15               # Adjust remaining group count
        cmpobl  0,r15,.sa20             # Jif more to go
#
        b       .sa10_r
#
# --- Local procedure -------------------------------------------------
#
# --- Update requests per second
#
.sa100_r:
        ldl     rd_sprc(g0),r4          # Get sample period request and
                                        #  sector counts
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        stl     g12,rd_sprc(g0)         # Clear sample counts
        cmpobe  0,r4,.sa110             # Jif no samples
#
# --- Compute average sector count
#
        mov     0,r7                    # Divide total sector count by
        mov     r5,r6                   #  total request count
        ediv    r4,r6,r6
        shro    1,r4,r3                 # Calculate .5 of divisor
        cmpo    r6,r3                   # Round up quotient if
        selge   0,1,r3                  #  necessary
        addo    r3,r7,r5
#
.sa110:
        stl     r4,rd_rps(g0)           # Update average rps and sector
                                        #  count
#
# --- Exit local procedure
#
        bx      (g14)
#
#**********************************************************************
#
#  NAME: R$que
#
#  PURPOSE:
#       To provide a common method of queuing RAID I/O requests to this
#       module.
#
#  DESCRIPTION:
#       The ILT and associated RRP are queued to the tail of the
#       executive queue.  The executive is activated to process this
#       request.  This routine may be called from either the process or
#       interrupt level.
#
#  CALLING SEQUENCE:
#       call    R$que
#
#  INPUT:
#       g1 = ILT
#
#        il_w0 = PRP
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
R$que:
        lda     R_exec_qu,r11           # Get queue origin
        b       K$cque
#
#**********************************************************************
#
#  NAME: r$stdcomp
#
#  PURPOSE:
#       To provide a means of processing the completion of a physical
#       I/O operation in support of a "standard device".  Support is
#       also provided for RAID 0 operations which resolve to a single
#       device.
#
#  DESCRIPTION:
#       The status of the RRP is updated based upon the status of the
#       completing PRP.  The primary ILT is completed back to the caller
#       and the PRP packet is released back to the system.
#
#  CALLING SEQUENCE:
#       call    r$stdcomp
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
r$stdcomp:
        mov     g0,r12                  # Save g0
        mov     g1,r13                  # Save g1
        mov     g2,r14                  # Save g2
#
# --- Check status
#
        ld      il_w0(g1),g2            # Get PRP
        call    M$chkstat               # Get PRP status
        ld      il_w4(g1),r3            # Get RDD
        cmpobe  ecok,g0,.sd10           # Jif OK

.ifndef MODEL_3000
.ifndef  MODEL_7400
c       r_setrrpec_r0rstd(g0, g1, g2);  # Set RRP error code
.endif  # MODEL_7400
.endif  # MODEL_3000
.ifndef  MODEL_7000
.ifndef  MODEL_4700
c       r_setrrpec(g0, g1, g2);         # Set RRP error code
.endif  # MODEL_4700
.endif  # MODEL_7000
.sd10:

#-  2008-10-30 - put_vrp() is now in "c", so this no longer applies, but is ok.
##  V1 Note(Imp) :- In the context of wookie,the r$comp(above), takes control
##  from layer to layer and in Link Layer( the link layer completer task) frees
##  the VRP by calling PM_RelVRP. That function moves the VRP into g2,before
##  deletes it. So, by the time control comes here,g2 will not contain the RRP
##  as expected by the function M$rprp. Ensure that Link layer has preserved this
##  status.

.if 1   # Following is original version , before July 15th, i.e. version  <= 1.37.4.12
#
# --- Complete RRP request
#
        balx    r$comp,r6               # Complete request
#
# --- Release PRP
#
.ifdef M4_DEBUG_PRP
c CT_history_printf("%s%s:%u put_prp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_PRP
c       put_prp(g2);                    # Release PRP
.else   # 1
## Raghu's fix is below. This might be solved with the Link Layer change.
#
# --- Release PRP
#
.ifdef M4_DEBUG_PRP
c CT_history_printf("%s%s:%u put_prp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_PRP
c       put_prp(g2);                    # Release PRP
#
# --- Complete RRP request
#
        balx    r$comp,r6               # Complete request
.endif  # 1

# .ifdef MODEL_7000
# .sd11:
# .endif  # MODEL_7000
#
# --- Exit
#
        mov     r12,g0                  # Restore g0
        mov     r13,g1                  # Restore g1
        mov     r14,g2                  # Restore g2
        ret
#
#**********************************************************************
#
#  NAME: r$r0comp
#
#  PURPOSE:
#       To handle the completion of multiple individual physical I/O
#       requests that were previously issued to perform a complex RAID 0
#       I/O request.
#
#  DESCRIPTION:
#       If the completing PRP has an error return, the error status is
#       directly transferred to the original RRP.  The ILT/PRP/SGL is
#       released back to the system.  The pending count in the primary
#       ILT is decremented.  If that count goes to zero, the primary ILT
#       is completed back to the caller and the RRP queue depth within
#       the RDD is updated.
#
#  CALLING SEQUENCE:
#       call    r$r0comp
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
r$r0comp:
        mov     g0,r12                  # Save g0
        mov     g1,r13                  # Save g1
        mov     g2,r14                  # Save g2
#
# --- Check request status
#
        ld      il_w0(g1),g2            # Get PRP
        call    M$chkstat               # Check status
        ld      il_w3(g1),g1            # Get primary ILT
        cmpobe  ecok,g0,.rc20           # Jif status ok

.ifndef MODEL_3000
.ifndef  MODEL_7400
c       r_setrrpec_r0rstd(g0, g1, g2);  # Set RRP error code
.endif  # MODEL_7400
.endif  # MODEL_3000
.ifndef  MODEL_7000
.ifndef  MODEL_4700
c       r_setrrpec(g0, g1, g2);         # Set RRP error code
.endif  # MODEL_4700
.endif  # MODEL_7000
#
# --- Adjust pending request count
#
.rc20:
        ld      il_w0(g1),r4            # Decrement pending request
        subo    1,r4,r4                 #  count in primary ILT
        st      r4,il_w0(g1)
        cmpobne 0,r4,.rc30              # Jif more requests
#
# --- Complete primary ILT/RRP
#
        ld      il_w4(g1),r3            # Get RDD
        balx    r$comp,r6               # Complete primary ILT/RRP
#
# --- Release ILT/PRP/SGL
#
.rc30:
        mov     r13,g1                  # Restore secondary ILT
        call    M$rip                   # Release ILT/PRP/SGL
# .ifdef MODEL_7000
# .rc31:
# .endif  # MODEL_7000
#
# --- Exit
#
        mov     r12,g0                  # Restore g0
        mov     r13,g1                  # Restore g1
        mov     r14,g2                  # Restore g2
        ret
#
#**********************************************************************
#
#  NAME: r$r1rdcomp
#
#  PURPOSE:
#       To provide a means of processing the completion of a physical
#       read request which was originally issued in support of all RAID
#       1 and RAID 10 (simple) read operations.
#
#  DESCRIPTION:
#       If the physical request has completed normally, the associated
#       RRP is completed and the RDD statistics are updated.  All
#       associated ILT/PRPs are automatically released back to the
#       system.
#
#       If the physical request has not completed normally, that
#       request is removed from the associated link and released back
#       to the system.  If any associated requests have survived, they
#       are requeued, else, the associated RRP is completed and the RDD
#       statistics are updated.
#
#  CALLING SEQUENCE:
#       call    r$r1rdcomp
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
#      W1 = mirrored ILT circular link
#      W2 = null
#      W3 = Primary ILT
#      W4 = PSD
#
#**********************************************************************
#
r$r1rdcomp:
        mov     g0,r12                  # Save g0
        mov     g1,r13                  # Save g1
        mov     g2,r14                  # Save g2
#
# --- Check request status
#
        ld      il_w0(g1),g2            # Get PRP and obtain the
        call    M$chkstat               #  status
        ld      il_w3(g1),r11           # Get primary ILT
        cmpobe  ecok,g0,.re60           # Jif status OK
.ifndef MODEL_3000
.ifndef  MODEL_7400
        cmpobe  ecbebusy,g0,.re40       # Jif ISE BUSY
.endif  # MODEL_7400
.endif  # MODEL_3000
#
        ldconst eccancel,r3             # Check for cancelled status
        cmpobe  r3,g0,.re40             # Jif so
#
# --- Attempt to hotspare this device
#
        call    r$hspare                # Attempt to hotspare this device
#
# --- Remove this request from associated link if possible
#
        ld      il_w1(g1),r3            # Get link to next ILT
        cmpobe  r3,g1,.re40             # Jif no additional ILTs left
#
        mov     r3,r5                   # Save ILT following ILT w/ err
#
.re10:
        ld      il_w1(r3),r4            # Search for preceding ILT
        cmpobe  g1,r4,.re20             # Jif found
#
        mov     r4,r3                   # Continue search
        b       .re10
#
.re20:
        st      r5,il_w1(r3)            # Unlink request w/ error
        call    M$rip                   # Release request w/ error
#
# --- Requeue associated ILTs excluding the ILT that failed
#
        mov     r3,g1
        ld      P_que,g0                # Pass physical queuing routine
        lda     r$r1rdcomp,g2           # Pass read completion routine
.re30:
        ld      il_w1(g1),r4            # Get next request
        call    K$q                     # Queue request
        mov     r4,g1
#
        cmpobne r3,r4,.re30             # Jif more
        b       .re100
#
# --- Report irrecoverable I/O error - status is initialized to ecok
#     with a transition to ecioerr if a successful read can not be
#     performed
.re40:
c       r_setrrpec(g0, r11, g2);        # Set RRP error code
        mov     r13,g1                  # Restore secondary ILT
        b       .re60
#
# --- Release all remaining associated mirrored read ILT/PRPs if any
#
.re60:
        mov     g1,r10                  # Save 1st ILT
.re70_r:
        ld      il_w1(g1),r3            # Get next ILT
        call    M$rip                   # Release ILT/PRP
        mov     r3,g1
#
        cmpobne r10,g1,.re70_r          # Jif more
#
# --- Complete primary ILT/RRP
#
        ld      il_w4(r11),r3           # Get RDD
        mov     r11,g1                  # Complete primary ILT/RRP
        balx    r$comp,r6
#
# --- Exit
#
.re100:
        mov     r12,g0                  # Restore g0
        mov     r13,g1                  # Restore g1
        mov     r14,g2                  # Restore g2
        ret
#
#**********************************************************************
#
#  NAME: r$r1wrcomp
#
#  PURPOSE:
#       To provide a means of processing the completion of all physical
#       write requests which were originally issued in support of all
#       RAID 1 and simple RAID 10 write operations.
#
#  DESCRIPTION:
#       If the status within the PRP indicates an error, an attempt is
#       made to hotspare the device, else, the status within the RRP is
#       set to OK.
#
#       The pending I/O count is decremented.  If the count goes to
#       zero, the corresponding RRP is completed and the NVA entry is
#       released.
#
#       The ILT/PRP for the completing request are released back to the
#       system.
#
#  CALLING SEQUENCE:
#       call    r$r1wrcomp
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
#      W3 = Primary ILT
#      W4 = PSD
#      W5 = NVA
#
#**********************************************************************
#
r$r1wrcomp:
        mov     g0,r12                  # Save g0
        mov     g1,r13                  # Save g1
        mov     g2,r14                  # Save g2
#
# --- Check request status
#
        ld      il_w0(g1),g2            # Get PRP and obtain the
        ld      il_w3(g1),r11           # Get primary ILT
        call    M$chkstat               #  status
        ld      il_w4(r11),r3           # Get RDD
        ld      il_w0-ILTBIAS(r11),r10  # Get RRP
        ld      il_w0(r11),r8           # Get the Pending I/O Count
        cmpobe  ecok,g0,.rf10           # Jif status OK
.ifndef MODEL_3000
.ifndef  MODEL_7400
        cmpobe  ecbebusy,g0,.rf20       # Jif ISE BUSY
.endif  # MODEL_7400
.endif  # MODEL_3000

#       check return code to make sure it agrees with the pdd busy state
#       if not set err code to busy
c       r6 = Raid10BusyCorruptionCheck(g2,g0,0x12);
        cmpobe 0,r6,.rf05
        ldconst ecbebusy,g0             # set error code to busy
        b       .rf20
.rf05:
#
# --- Attempt to hotspare this device. If the RDD is in a Local Image In
#       Progress state - set the RRP Status to Retry, else do not save status
#       (one good write is leave good, else status already has ECIOERR)
#
        call    r$hspare                # Attempt to hotspare this device
        ldob    rd_astatus(r3),r6       # Get the RAID Additional Status
        ldconst ecretry,g0              # Show Write Op needs to be retried
        bbc     rdalocalimageip,r6,.rf30 # Jif no need to set Retry Status
        b       .rf20                   # Store new status
#
# --- Set RRP status (in required order) (started all with ECIOERROR):
#       If original status is ECRETRY - leave alone
#       else new status is ECOK - set status = ECOK
#       Else leave status as is
#
.rf10:
        ldob    rr_status(r10),r7       # Get the current RRP Status
.ifndef MODEL_3000
.ifndef  MODEL_7400
        ldconst ecbebusy,r5            # set up ise busy status
        cmpobe  r7,r5,.rf30             # Status already set to ise busy, continue
.endif  # MODEL_7400
.endif  # MODEL_3000
        ldconst ecretry,r5              # Set up possible Retry Status
        cmpobe  r7,r5,.rf30             # Status already set to retry, continue
.rf20:
        stob    g0,rr_status(r10)       # Update status to OK
#
# --- Adjust pending I/O count
#
.rf30:
        cmpdeco 1,r8,r8                 # Adjust pending I/O count
        st      r8,il_w0(r11)
#
        bne     .rf40                   # Jif more requests
#
# --- Complete primary ILT/RRP and release NVA
#
#        ld      il_w5(r11),g0           # Get NVA
        mov     r11,g1                  # Complete primary ILT/RRP
                                        # r3 = RDD
        balx    r$comp,r6
        mov     r13,g1                  # Restore g1
#
# --- Release ILT/PRP - the SGL is owned by the primary ILT so it will
#     not be released at this time
#
.rf40:
        call    M$rip                   # Release ILT/PRP
#
# --- Exit
#
        mov     r12,g0                  # Restore g0
        mov     r13,g1                  # Restore g1
        mov     r14,g2                  # Restore g2
        ret
#
#**********************************************************************
#
#  NAME: r$r10concomp
#
#  PURPOSE:
#       To provide a means of processing the completion of a physical
#       compare request for RAID 10 compare operation.
#
#  CALLING SEQUENCE:
#       call    r$r10concomp
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
#      W3 = Primary ILT
#      W4 = PSD
#      W5 = mirrored ILT circular link
#
#**********************************************************************
#
r$r10concomp:
        mov     g0,r12                  # Save g0
        mov     g1,r13                  # Save g1
        mov     g2,r14                  # Save g2
#
# --- Check request status.  In the case of an IO error, we will just
# --- mark the parent ILT as having an IO error and will skip the compare
# --- step.
#
        ld      il_w0(g1),g2            # Get PRP and obtain the
        call    M$chkstat               #  status
        ld      il_w3(g1),r11           # Get primary ILT
#
        ld      il_w0(r11),r15          # Get pending I/O count
        subo    1,r15,r15               # Adjust for I/O
        st      r15,il_w0(r11)          # Record pending I/O count
#
        cmpobe  ecok,g0,.rcc30          # Jif status OK
#
# --- Mark the parent ILT as having an IO error
#
        ldconst ecioerr,r3              # r3 = IO error
        ld      il_w3(g1),r4            # Parent ILT
        ld      il_w0-ILTBIAS(r4),r4    # RRB
        stob    r3,rr_status(r4)
#
# --- Check for remaining requests completed.  Once they are all done, then
# --- compare the data.  If a miscompare takes place, record it in the
# --- drive records.
#
.rcc30:
        mov     g1,r10                  # Start at the last IO completed
        cmpobne 0,r15,.rcc100           # Exit if not all commands completed
#
        ld      il_w3(g1),r4            # Parent ILT
        ld      il_w0-ILTBIAS(r4),r4    # RRB
        ldob    rr_status(r4),r4
        cmpobne ecok,r4,.rcc60          # If an error, don't byte check
#
.rcc40:
        ld      il_w5(g1),r15           # Get next ILT (g1 and r3 SGL compare)
        cmpobe  r15,r10,.rcc60          # Done, release all buffers
#
        ld      il_w0(g1),g4            # Get PRP
        ld      pr_sglptr(g4),g4        # Get data pointer into g4
        ld      sg_desc0+sg_addr(g4),g4 # Data pointer for current leading IO
        ld      il_w0(r15),g5           # Get PRP
        ld      pr_sglptr(g5),g5        # Get data pointer into g5
        ld      sg_desc0+sg_len(g5),g6  # Data size
        ld      sg_desc0+sg_addr(g5),g5 # Data pointer for current trailing IO
c       if (memcmp((void*)g4, (void*)g5, g6) == 0) {
            b   .rcc50                  # Jump if strings match
c       }
#
# --- Bump error counts on both devices.
#
        ld      il_w4(g1),r3            # Get PSD of leading
        ldos    ps_pid(r3),r3           # Get PID
        ld      P_pddindx[r3*4],r3      # Get PDD
        ld      pd_r10misc(r3),r4       # Get miscompare count
        addo    1,r4,r4                 # Bump it
        st      r4,pd_r10misc(r3)
#
        ld      il_w4(r15),r3           # Get PSD of trailing
        ldos    ps_pid(r3),r3           # Get PID
        ld      P_pddindx[r3*4],r3      # Get PDD
        ld      pd_r10misc(r3),r4       # Get miscompare count
        addo    1,r4,r4                 # Bump it
        st      r4,pd_r10misc(r3)
#
        ldconst eccompare,r3            # r3 = parity check miscompare error
        ld      il_w3(r15),r4           # Parent ILT
        ld      il_w0-ILTBIAS(r4),r4    # RRB
        stob    r3,rr_status(r4)
#
.rcc50:
        mov     r15,g1                  # Try another pass
        b       .rcc40
#
# --- Release the ILT/PRP/SGLs
#
.rcc60:
        mov     r10,g1                  # Starting point for ILT/PRP/SGL
#
.rcc70:
        ld      il_w5(g1),r15           # Get next ILT (g1 and r3 SGL compare)
        ld      il_w0(g1),r3            # Get PRP
        ld      pr_sglptr(r3),r4        # Get SGL
        ldconst 0,r5
        st      r5,pr_sglptr(r3)
        mov     r4,g0
        call    M$rsglbuf               # Release the buffer
#
        call    M$rip                   # Release ILT/PRP
        mov     r15,g1                  # Set leading to the former trailing IO
        cmpobne r10,g1,.rcc70           # Jif more
#
# --- Complete primary ILT/RRP
#
        ld      il_w4(r11),r3           # Get RDD
        mov     r11,g1                  # Complete primary ILT/RRP
        balx    r$comp,r6
#
# --- Exit
#
.rcc100:
        mov     r12,g0                  # Restore g0
        mov     r13,g1                  # Restore g1
        mov     r14,g2                  # Restore g2
        ret
#
#**********************************************************************
#
#  NAME: r$r10rdcomp
#
#  PURPOSE:
#       To provide a means of processing the completion of a physical
#       read request which was originally issued in support of a RAID
#       10 (complex) read operation.
#
#  DESCRIPTION:
#       If the physical request has not completed normally, that
#       request is removed from the associated link and released back
#       to the system.  If any associated requests have survived, they
#       are requeued, else, the associated RRP is updated with an error
#       status.
#
#       If the physical request has completed normally, all associated
#       ILT/PRPs are automatically released back to the system.
#
#       If the pending I/O count has gone to zero, the RRP is completed
#       and the RDD queue depth is adjusted.
#
#  CALLING SEQUENCE:
#       call    r$r10rdcomp
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
#      W1 = mirrored ILT circular link
#      W2 = null
#      W3 = Primary ILT
#      W4 = PSD
#
#**********************************************************************
#
r$r10rdcomp:
        mov     g0,r12                  # Save g0
        mov     g1,r13                  # Save g1
        mov     g2,r14                  # Save g2
#
# --- Check request status
#
        ld      il_w0(g1),g2            # Get PRP and obtain the
        call    M$chkstat               #  status
        ld      il_w3(g1),r11           # Get primary ILT
        cmpobe  ecok,g0,.rh60           # Jif status OK
.ifndef MODEL_3000
.ifndef  MODEL_7400
        cmpobe  ecbebusy,g0,.rh40       # Jif ISE BUSY
.endif  # MODEL_7400
.endif  # MODEL_3000
#
        ldconst eccancel,r3             # Check for cancelled status
        cmpobe  r3,g0,.rh40             # Jif so
#
# --- Attempt to hotspare this device
#
        call    r$hspare                # Attempt to hotspare this device
#
# --- Remove this request from associated link if possible
#
        ld      il_w1(g1),r3            # Get link to next ILT
        cmpobe  r3,g1,.rh40             # Jif no additional ILTs left
#
        mov     r3,r5                   # Save ILT following ILT w/ err
#
.rh10:
        ld      il_w1(r3),r4            # Search for preceding ILT
        cmpobe  g1,r4,.rh20             # Jif found
#
        mov     r4,r3                   # Continue search
        b       .rh10
#
.rh20:
        st      r5,il_w1(r3)            # Unlink request w/ error
        call    M$rip                   # Release request w/ error
#
        ld      il_w0(r11),r4           # Get pending I/O count
        subo    1,r4,r4                 # Adjust for failed I/O
        st      r4,il_w0(r11)           # Record pending I/O count
#
# --- Requeue associated ILTs excluding the ILT that just failed
#
        mov     r3,g1
        ld      P_que,g0                # Pass physical queuing routine
        lda     r$r10rdcomp,g2          # Pass read completion routine
.rh30:
        ld      il_w1(g1),r4            # Get next request
        call    K$q                     # Queue request
        mov     r4,g1
#
        cmpobne r3,r4,.rh30             # Jif more
#
        b       .rh100
#
# --- Report irrecoverable I/O error - status is initialized to ecok
#     with a transition to ecioerr if a successful read can not be
#     performed
.rh40:
c       r_setrrpec(g0, r11, g2);        # Set RRP error code
        mov     r13,g1                  # Restore secondary ILT
        b       .rh60
#
# --- Release all remaining associated mirrored read ILT/PRPs if any
#
.rh60:
        mov     g1,r10                  # Save 1st ILT
        ld      il_w0(r11),r4           # Get pending I/O count
.rh70:
        ld      il_w1(g1),r3            # Get next ILT
        call    M$rip                   # Release ILT/PRP
        mov     r3,g1
        subo    1,r4,r4                 # Adjust pending I/O count
#
        cmpobne r10,g1,.rh70            # Jif more
#
# --- Update pending I/O count
#
        st      r4,il_w0(r11)           # Record pending I/O count
#
        cmpobne 0,r4,.rh100             # Jif more I/O
#
# --- Complete primary ILT/RRP
#
        ld      il_w4(r11),r3           # Get RDD
        mov     r11,g1                  # Complete primary ILT/RRP
        balx    r$comp,r6
#
# --- Exit
#
.rh100:
        mov     r12,g0                  # Restore g0
        mov     r13,g1                  # Restore g1
        mov     r14,g2                  # Restore g2
        ret
#
#**********************************************************************
#
#  NAME: r$r10wrcomp
#
#  PURPOSE:
#       To provide a means of processing the completion of a physical
#       write request which was originally issued in support of a RAID
#       10 (complex) write operation.
#
#  DESCRIPTION:
#       If an error has occurred, an attempt is made to hotspare the
#       failing physical device, else, all other requests within the
#       set of mirrored writes inherit an OK status.  The completing
#       request is removed from the associated link of mirrored writes
#       and released.  If this request constitutes the last request of
#       a mirrored write set, a check is made to see if all of the
#       mirrored writes have failed.  If so, the RRP inherits an error
#       status.
#
#       If the pending I/O count has gone to zero, the RRP is completed,
#       the NVA is released and the RDD queue depth is adjusted.
#
#  CALLING SEQUENCE:
#       call    r$r10wrcomp
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
#      W3 = Primary ILT
#      W4 = PSD
#      W6 = associated secondary ILT
#      W7 = inherited status
#
#**********************************************************************
#
r$r10wrcomp:
        mov     g0,r12                  # Save g0
        mov     g1,r13                  # Save g1
        mov     g2,r14                  # Save g2
#
# --- Check request status
#
        ld      il_w0(g1),g2            # Get PRP and obtain the
        ld      il_w3(g1),r11           # Get primary ILT
        call    M$chkstat               #  status
        mov     g0,r7                   # Save status
        ld      il_w0(r11),r10          # Get Pending I/O counter
        ld      il_w4(r11),r3           # Get RDD
        ld      il_w0-ILTBIAS(r11),r8   # Get RRP
        cmpobe  ecok,g0,.ri10_l           # Jif status OK
        ldos    rd_rid(r3),r6
.ifndef MODEL_3000
.ifndef  MODEL_7400
        cmpobne  ecbebusy,g0,.ri00_l    # Jif not ISE BUSY
        stob    g0,rr_status(r8)        # Update RRP status
        b       .ri10_l
.ri00_l:
.endif  # MODEL_7400
.endif  # MODEL_3000
#       check return code to make sure it agrees with the pdd busy state
#       if not set err code to busy
c       r6 = Raid10BusyCorruptionCheck(g2,g0,0x12);
        cmpobe 0,r6,.ri05_l
        ldconst ecbebusy,g0             # set error code to busy
        stob    g0,rr_status(r8)        # Update RRP status
        b       .ri10_l
.ri05_l:
#
# --- Attempt to hotspare this device. If the RDD is in a Local Image In
#       Progress state - set the RRP Status to Retry, else do not save status
#       (one good write is leave good, else status already has ECIOERR)
#
        call    r$hspare                # Attempt to hotspare this device
        ldob    rd_astatus(r3),r6       # Get the RAID Additional Status
        ldconst ecretry,g0              # Prep Write Op needs to be retried
        bbc     rdalocalimageip,r6,.ri10_l # Jif no need to set Retry Status
        stob    g0,rr_status(r8)        # Update RRP status
#
# --- Update pending I/O count
#
.ri10_l:
        subo    1,r10,r10               # Update pending I/O count
        st      r10,il_w0(r11)
#
# --- Remove this request from associated link if possible
#
        ld      il_w6(g1),r6            # Get link to next ILT
        cmpobe  r6,g1,.ri50_l           # Jif no additional ILTs left
#
        mov     r6,r5                   # Save ILT following this ILT
.ri20_l:
        cmpobne ecok,r7,.ri30_l         # Jif bad status
#
        st      r7,il_w7(r6)            # Update inherited status
.ri30_l:
        ld      il_w6(r6),r4            # Search for preceding ILT
        cmpobe  g1,r4,.ri40_l           # Jif found
#
        mov     r4,r6                   # Continue search
        b       .ri20_l
#
.ri40_l:
        st      r5,il_w6(r6)            # Unlink this ILT
        b       .ri60_l
#
# --- Process status for this mirror set
#
#       The RRP status is preinitialized to OK (but may have been changed to
#       RETRY above).  If all requests to any particular mirror set produce
#       an error, then the RRP inherits an error unless already set to RETRY.
#
.ri50_l:
        cmpobe  ecok,r7,.ri60_l         # Jif current status OK - Leave status
                                        #  as OK or RETRY
        ld      il_w7(g1),r7            # Get inherited status for this
                                        #  mirror set
        cmpobe  ecok,r7,.ri60_l         # Jif inherited status OK - Leave status
                                        #  as OK or RETRY
        ldob    rr_status(r8),r6        # Get the current status
.ifndef MODEL_3000
.ifndef  MODEL_7400
        ldconst ecbebusy,g0             # ISE Busy
        cmpobe  r6,g0,.ri60_l           # Jif status already set to ISE Busy
.endif  # MODEL_7400
.endif  # MODEL_3000
        ldconst ecretry,g0              # Prep for EC Retry check
        cmpobe  r6,g0,.ri60_l           # Jif status already set to RETRY
        stob    r7,rr_status(r8)        # Update RRP status
#
# --- Release ILT/PRP
#
.ri60_l:
        call    M$rip                   # Release request
#
# --- Check for additional outstanding I/O operations
#
        cmpobne  0,r10,.ri100_l         # Jif outstanding I/O
#
# --- Complete primary ILT/RRP
#
#        ld      il_w5(r11),g0           # Get NVA
        mov     r11,g1                  # Get primary ILT
                                        # r3 = RDD
        balx    r$comp,r6               # Complete request
#
# --- Exit
#
.ri100_l:
        mov     r12,g0                  # Restore g0
        mov     r13,g1                  # Restore g1
        mov     r14,g2                  # Restore g2
        ret
#
#**********************************************************************
#
#  NAME: r$hspare
#
#  PURPOSE:
#       To provide a common means of invoking the hotspare procedure
#       when appropriate.
#
#  DESCRIPTION:
#       The original PRP is obtained from the ILT so that the SCSI
#       channel and ID can be extracted.  If these fields do not match
#       the corresponding fields within the PSD this routine merely
#       exits.  Otherwise the formal hotspare procedure is invoked.
#
#  CALLING SEQUENCE:
#       call    r$hspare
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
r$hspare:
# c fprintf(stderr, "r$hspare: g0=0x%lx, g1=0x%lx, g2=0x%lx\n", g0, g1, g2);
        movq    g0,r12                  # Save regs
#
# --- Locate corresponding PDD
#
        ld      il_w4(g1),g0            # Get PSD
        ld      il_w0(g1),g2            # Get original PRP
        ldos    ps_pid(g0),r10          # Get PID
        ld      P_pddindx[r10*4],r10    # Lookup PDD
#
# --- Check if hotspare has already occurred
#
        ld      pr_dev(g2),r4           # Get device from PRP
        ld      pd_dev(r10),r6          # Get device from PDD
        cmpobne r4,r6,.he100            # Jif hotspared
#
# --- Check for SCSI check w/ miscompare
#
        ldob    pr_sstatus(g2),r5       # Get SCSI status byte
        cmpobne 2,r5,.he10              # Jif not check condition
#
        ldob    pr_sense+2(g2),r6       # Get sense key and isolate
        and     0x0f,r6,r6
        cmpobe  0xe,r6,.he100           # Jif miscompare
#
# --- Attempt to hotspare device
# --- Mark this PSD and RDD as potential hotspare candidates
#
.he10:
        mov     g1,g3                   # Pass ILT for flight recorder
        ldos    ps_rid(g0),r3           # Get RID
        ld      R_rddindx[r3*4],g1      # Pass RDD
# c fprintf(stderr, "r$hspare-10: calling RB$rerror g0=0x%lx, g1=0x%lx, g2=0x%lx\n", g0, g1, g2);
        call    RB$rerror                # Mark RDD in error state, g0=PSD, g2=PRP
#
# --- Exit
#
.he100:
        movq    r12,g0                  # Restore regs
# c fprintf(stderr, "r$hspare-100: g0=0x%lx, g1=0x%lx, g2=0x%lx\n", g0, g1, g2);
        ret
#
#**********************************************************************
#
#  NAME: r$comp
#
#  PURPOSE:
#       To provide a common means of completing STD, RAID 0, 1, 5 and 10
#       RRP requests back to the caller.
#
#  DESCRIPTION:
#       The specified request is first completed then the RDI queue
#       depth and the outstanding request count are decremented.
#
#  CALLING SEQUENCE:
#       balx    r$comp,r6
#
#  INPUT:
#       g1 = ILT
#       r3 = RDD
#
#  OUTPUT:
#       g1 ILT is backed up 1 level
#
#  REGS DESTROYED:
#       r3-r5
#
#**********************************************************************
#
r$comp:
        ld      il_w0-ILTBIAS(g1),r4    # Get corresponding RRP
        ldob    rr_status(r4),r5        # Get completion status
c       record_raid(FR_RRP_COMPLETE, (void *)r4, r5);
        cmpobe  ecok,r5,.rco10          # Jif no error
#.ifdef MODEL_7000 # ISE_BUSY
#        cmpobe  ecbebusy,r5,.rco50
#.endif  # MODEL_7000

#
# --- Error Handler
#
        ld      rd_error(r3),r4         # Bump error count
        addo    1,r4,r4
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        st      r4,rd_error(r3)
#
# --- Complete request
#     If the RAID is in a "Locked" State then queue the request to the
#     error handler.
#
.rco10:
        ld      R_errlock,r4            # Get the lock
        cmpobne TRUE,r4,.rco50          # Jif no lock in place
#
        st      r3,il_w4(g1)            # Pass RDD in ILT for rerror exec
        lda     K$comp,r4               # Pass completion routine
        st      r4,il_cr(g1)
        call    RB$rerror_que           # Que this ILT to error handler
        b       .rco60                  # Don't complete the request
#
.rco50:
        call    K$comp                  # Complete request
#
# --- Adjust RDI queue depth and outstanding request count
#
.rco60:
        ld      R_orc,r5                # Get outstanding request count
        subo    1,r5,r5                 # Adjust outstanding req count
        st      r5,R_orc
        ld      rd_qd(r3),r4            # Get queue depth
        subo    1,r4,r4                 # Adjust queue depth
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        st      r4,rd_qd(r3)
#
# --- If the new queue depth is zero, wake up possible blocked
# --- defragmentation locks.
#
        cmpobne 0,r4,.rco1000           # Jif non-zero
#
c       TaskReadyByState(pcblklock);    # Ready any process in block lockout wait
#
# --- Exit
#
.rco1000:
        bx      (r6)
#
#****************************************************************************
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
