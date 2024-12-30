# $Id: misc.as 148044 2010-09-23 16:16:59Z m4 $
#**********************************************************************
#
#  NAME: misc.as
#
#  PURPOSE:
#       To provide a means of handling miscellaneous common procedures
#       that are required from multiple modules.
#
#  FUNCTIONS:
#       M$init     - Misc. initialization
#       M$chkstat  - Check PRP status
#       M$findtgt  - Find a Target given a WWN
#       M$soft_flt - Handle a Software Detected Fault
#       M$flight_rec    - Flight recorder
#
#       This module employs no processes.
#
#  Copyright (c) 1996-2010 Xiotech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- global function declarations ------------------------------------
#
        .globl  M$init                  # Misc. initialization
#
.ifdef FRONTEND
        .globl  M$NMIclear              # Clear the NMI counts in NVRAM
.endif  # FRONTEND
#
        .globl  MSC_MemCmp              # Memory compare
        .globl  M$que_hbeat             # Heartbeat MRP handler
#
.if     DEBUG_FLIGHTREC
        .globl  M$flight_rec            # Flight recorder
.endif  # DEBUG_FLIGHTREC
#
.if MAG2MAG
        .globl  M$findtgt               # Find a Target ID given a WWN
.endif  # MAG2MAG
        .globl  M$soft_flt              # Software Detected Fault handler
#
# --- global data declarations ----------------------------------------
#
.if     DEBUG_FLIGHTREC
        .globl  fr_parm
        .globl  fr_parm0
        .globl  fr_parm1
        .globl  fr_parm2
        .globl  fr_parm3
.endif  # DEBUG_FLIGHTREC
#
# --- global usage data definitions -----------------------------------
#
        .data
        .align  2                       # Word aligned
#
# --- Heartbeat control
#
        .globl  hbeat_count             # For easier debug
        .globl  hbeat_disable
        .globl  ctrl_shutdown
        .globl  hbeat_timestamp
hbeat_count:                            # Incremented by hbeat MRPs
        .word   0
hbeat_time:
        .word   HBEATTIME               # heartbeat checking time in milliseconds
                                        # MAXTWAIT means no checking
ctrl_shutdown:                          # Controller is shutting down
        .word   0
hbeat_disable:                          # Disable heartbeat checking
        .word   FALSE
hbeat_timestamp:                        # Timestamp from CCB
        .word   0,0,0
#
# --- local usage data definitions ------------------------------------
#
.if     DEBUG_FLIGHTREC
fr_parm:
fr_parm0:                               # Flight Recorder data
        .word   0                       # Parm 0 and Type code
fr_parm1:
        .word   0                       # Parm 1
fr_parm2:
        .word   0                       # Parm 2
fr_parm3:
        .word   0                       # Parm 3
#
fr_queue:
        .word   0                       # Pointer to flight recorder queue
.endif  # DEBUG_FLIGHTREC
#
.if     DEBUG_HBEATREC
        .globl  hbeatTraceQue
        .align  4

hbeatTraceQue:
        .word   0
        .word   0
        .word   0
        .word   0                       # Disable it
#
# --- executable code -------------------------------------------------
#
        .text
#
#******************************************************************************
#
#  NAME: M$TraceEvent
#
#  PURPOSE:
#       To trace and timestamp define events.
#
#  DESCRIPTION:
#       This function will flight record trace events in a circular queue.
#
#  CALLING SEQUENCE:
#       ld      id, g0
#       ld      data, g1
#       call    M$TraceEvent
#
#  INPUT:
#       Load g0 with the event id, g1 with the associated data you want to
#       save, then call this function. The parms will be saved in a circular
#       queue.
#
#  OUTPUT:
#       None.
#
#******************************************************************************
#
        .set    trHBArrive, 0x80000000
        .set    trHBExec,   0x800000FF

M$TraceEvent:
        lda     hbeatTraceQue,r3        # load the event queue structure ptr
        ldq     (r3),r4                 # r4=evBaseP, r5=evNextP,
                                        # r6=evEndP, r7=runFlag
        cmpibe  0,r7,.mte20             # if runFlag clear, just exit
#
        ld      K_ii+ii_time,r8         # read gross timer
c       static struct itimerval te_current_time;
c       if (getitimer (ITIMER_REAL, &te_current_time) == 0) {
c               r9 = te_current_time.it_interval.tv_usec - te_current_time.it_value.tv_usec;
c       } else {
c               r9 = 0;
c       }
        stl     g0,(r5)                 # store event data in the queue
        stl     r8,8(r5)
#
        addo    r5,16,r5                # bump evNextP (in r5)
        cmpi    r5,r6                   # see if we are at the end of the queue
        st      r5,4(r3)                # save new evNextP out
        bne     .mte20                  # if not at the end of queue, exit
        st      r4,4(r3)                # else store evBaseP to evNextP
#
.mte20:
        ret
.endif  # DEBUG_HBEATREC
#
        .text
#**********************************************************************
#
#  NAME: M$init
#
#  PURPOSE:
#       To provide a means of initializing this module.
#
#  DESCRIPTION:
#       An initial quantity of ILT, PRP, RRP, SCB, RPN and RRB packets
#       are preallocated from the appropriate system pool of memory.
#       These individual quantities are defined within system.inc.
#
#  CALLING SEQUENCE:
#       call    M$init
#
#  INPUT:
#       g3 = FICB
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g0-g3
#       g14
#
#**********************************************************************
#
M$init:
#
# --- pre-allocate and initialize any packets
#
        call    pm$init
#
# --- Add IRAM entry into the Debug Data Retrieval Table --------------
#
.ifdef FRONTEND
        ldconst de_feiram,g0            # Load DDR table offset
.else   # FRONTEND
        ldconst de_beiram,g0            # Load DDR table offset
.endif  # FRONTEND
c       g1 = (ulong)&IRAMBASE;
c       g2 = (ulong)&IRAMEND-g1;
c       M_addDDRentry(g0, g1, g2);
#
# --- Preallocate initial ILT pool ------------------------------------
#
c       init_ilt(IILTS);                # Preallocate IILTS ILTs.
#
.if     DEBUG_FLIGHTREC
#
# --- Allocate flight recorder space ----------------------------------
#
c       g0 = s_MallocC(fr_asize, __FILE__, __LINE__); # allocation sizeof flight recorder
#
        ldconst fr_asize,r3             # allocation sizeof flight recorder
        addo    r3,g0,g7                # point to last entry + 1
        addo    qcb_size,g0,g4          # leave space at front for queue ptrs
        mov     g4,g5                   # initialize IN to BEGIN pointer
        mov     g4,g6                   # initialize OUT to BEGIN pointer
        stq     g4,(g0)                 # save BEGIN, IN, OUT, END pointers
        st      g0,fr_queue             # save pointer to the queue
#
# --- Add entry into the Debug Data Retrieval Table -------------------
#
        mov     g0,g1                   # Load data address to g1
.if     DEBUG_FLIGHTREC_TIME
        ldconst de_frects,g0            # Load DDR table offset
.else   # DEBUG_FLIGHTREC_TIME
        ldconst de_frec,g0              # Load DDR table offset
.endif  # DEBUG_FLIGHTREC_TIME
c       M_addDDRentry(g0, g1, fr_asize);
.endif  # DEBUG_FLIGHTREC

.if     DEBUG_HBEATREC
#
# --- Allocate heatbeat trace space ----------------------------------
#
        ldconst hbt_asize,r6            # allocation sizeof heat  recorder
c       g0 = s_MallocC(r6, __FILE__, __LINE__); # allocation sizeof head recorder
#
        mov     g0,r4                   # Start pointer
        mov     g0,r5                   # Current pointer
        addo    g0,r6,r6                # End pointer
        ldconst 1,r7                    # Enable trace
        stq     r4,hbeatTraceQue
.endif  # DEBUG_HBEATREC
#
# --- Start the heartbeat tasks
#
        lda     m$exec_hbeatproc,g0     # Establish process to process ping MRPs
        ldconst HBEATPRI,g1             # Priority
c       CT_fork_tmp = (ulong)"m$exec_hbeatproc";
        call    K$fork
        st      g0,m_hbeat_qu+qu_pcb    # Save PCB in qu
#
        lda     m$exec_hbeatmon,g0      # Establish process to monitor pings
        ldconst HBEATPRI,g1             # Priority
c       CT_fork_tmp = (ulong)"m$exec_hbeatmon";
        call    K$fork
#
# --- Exit
#
        ret
#
#**********************************************************************
#
#  NAME: M_memcmp
#
#  PURPOSE:
#       To provide a common mechanism of comparing two byte strings for
#       equality.
#
#  DESCRIPTION:
#       Source string 1 is compared with source string 2 a byte at a
#       time.  An inequality prior to the expiration of the byte count
#       constitutes a mismatch.
#
#  CALLING SEQUENCE:
#       call    M_memcmp
#
#  INPUT:
#       g0 = source1 ptr
#       g1 = source2 ptr
#       g2 = length
#
#  OUTPUT:
#       g0 = < 0    buf1 less than buf2
#            = 0    buf1 identical to buf2
#            > 0    buf1 greater than buf2
#
#**********************************************************************
#
# C access
# int MSC_MemCmp(void* pSource1, void* pSource2, UINT32 length);
#
MSC_MemCmp:
        mov     g0,r12                  # r12=src1
        shro    4,g2,r14                # Get number of quads
        mov     g1,r13                  # r13=src2
        cmpobe  0,r14,.mcp20            # Jif no words to compare
#
        ldconst 16,r15                  # Number of bytes in a quad word
#
# --- Check next quad word in string
#
.mcp10:
        ldq     (r12),r4                # Get next source 1 byte
        ldq     (r13),r8                # Get next source 2 byte
        cmpobne r4,r8,.mcp30            # Jif mismatched
        cmpobne r5,r9,.mcp30            # Jif mismatched
        cmpobne r6,r10,.mcp30           # Jif mismatched
        cmpobne r7,r11,.mcp30           # Jif mismatched
#
        subo    1,r14,r14               # Adjust remaining length
        lda     16(r12),r12             # Bump source 1 address
        addo    16,r13,r13              # Bump source 2 address
        cmpibl  0,r14,.mcp10            # Jif comparison not complete
#
.mcp20:
        and     0xF,g2,r15              # Get remaining number of bytes
        cmpobe  0,r15,.mcp40            # Jif no bytes to compare
#
# --- Check next byte in string
#
.mcp30:
        ldob    (r12),r4                # Get next source 1 byte
        ldob    (r13),r8                # Get next source 2 byte
        cmpobne r4,r8,.mcp50            # Jif mismatched
#
        subo    1,r15,r15               # Adjust remaining length
        lda     1(r12),r12              # Bump source 1 address
        addo    1,r13,r13               # Bump source 2 address
        cmpibl  0,r15,.mcp30            # Jif comparison not complete
#
# --- Return equal comparison
#
.mcp40:
        ldconst 0,g0                    # Set equal compare
        b       .mcp100
#
# --- Return no comparison
#
.mcp50:
        subi    r8,r4,g0                # Set no compare
#
# --- Exit
#
.mcp100:
        ret
#
#**********************************************************************
#
#  NAME: M$NMIclear
#
#  PURPOSE:
#       To provide a common means of zeroing the ECC NMI counts in
#       NVRAM.
#
#  DESCRIPTION:
#       This function will read the total ECC error count, the single-
#       bit ECC error count, the multiple-bit ECC error count, and the
#       > 2 ECC error count from NVRAM into DRAM.  The counts will be
#       set back to zero and rewritten back to NVRAM.
#
#  CALLING SEQUENCE:
#       call    M$NMIclear
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       None.
#
#**********************************************************************
#
# C access
# void MSC_NMIClear(void);
        .globl  MSC_NMIClear
MSC_NMIClear:
.ifdef FRONTEND
M$NMIclear:
.endif  # FRONTEND
        movq    g0,r8                   # Save register g0, g1, g2, g3
        movl    g4,r12                  # Save register g4, g5
#
c       r15 = s_MallocC(nmisiz, __FILE__, __LINE__); # Allocate buffer space
#
# --- Save controller and unit serial numbers, byte by byte
#
        lda     nmi_cntl_sn(r15),g5     # Load destination address
        ldconst NVSRAMNMISTART+nmi_cntl_sn,g4   # Load source address
        ldconst 8,g3                    # Copy 8 bytes
c       memcpy((void*)g5,(void*)g4, g3);    # Perform byte by byte copy to DRAM
#
c       r3 = getpagesize();
#
c       r4 = g5%r3;
c       r5 = msync((void *)(g5 - r4), g3 + r4, MS_SYNC);
c       if (r5 != 0) fprintf(stderr, "MSC_NMIClear:  msync failed, errno = %d\n", errno);
#
# --- Copy the counters back to NVRAM
#
        ldconst NVSRAMNMISTART,g5       # Load NVRAM counter address
        mov     r15,g4                  # Load DRAM address
        ldconst nmisiz,g3               # Load buffer size
c       memcpy((void*)g5,(void*)g4, g3);    # Copy counters back into NVRAM
#
c       r4 = g5%r3;
c       r5 = msync((void *)(g5 - r4), g3 + r4, MS_SYNC);
c       if (r5 != 0) fprintf(stderr, "MSC_NMIClear:  msync failed, errno = %d\n", errno);
#
c       s_Free(r15, nmisiz, __FILE__, __LINE__); # Release the buffer
#
        movq    r8,g0                   # Restore register g0, g1, g2, g3
        movl    r12,g4                  # Restore register g4, g5
        ret
#
.if     DEBUG_FLIGHTREC
#******************************************************************************
#
#  NAME: M$flight_rec
#
#  PURPOSE:
#       To trace ILTs and VRP/RRP/PRP etc as they are being used.
#
#  DESCRIPTION:
#       This function will flight record entries in a circular queue. This
#       feature is enabled with a compiler flag. It can be disabled if the
#       OUT pointer is set to 0. The OUT pointer in the queue is initialized
#       to the beginning of the queue.
#
#  CALLING SEQUENCE:
#       call    M$flight_rec
#
#  INPUT:
#       Load the fr_parmx variables with the data you want to save then
#       call this function. The parms will be saved in a circular queue.
#       See fr.inc for more details.
#
#  OUTPUT:
#       None.
#
#******************************************************************************
#
# C access
# void MSC_FlightRec(UINT32 parm0, UINT32 parm1, UINT32 parm2, UINT32 parm3);
# A dummy version of this exists in misc.c that allows all the other C code
# to call it with the correct notation. But it just calls the following function
# for now...
M$flight_rec:
        ld      fr_queue,r10            # get pointer to BEGIN, IN, OUT, END ptrs
        cmpobe  0,r10,.lf10             # don't record if queue not initialized
        ldq     (r10),r12               # load them into r12-r15

        cmpobe  0,r14,.lf10             # don't record if FR disabled by OUT ptr

        lda     fr_parm0,r9             # point at users fr data
# Junk can be in fr_parm0 structure, allow anything to be in it.
?       ldq     (r9),r4                 # save the type code into next entry
?       stq     r4,fr_parms(r13)        # save 4 parms
#
.if     DEBUG_FLIGHTREC_TIME
        ld      (pfp),r7                # Get PFP (back up two levels)
        ld      K_ii+ii_time,r4         # Get current s/w timer
        ld      K_xpcb,r6               # Get Executing PCB
        ld      8(r7),r7                # Get the RIP from the caller to Fl Rec
#
c       static struct itimerval fr_current_time;
c       if (getitimer (ITIMER_REAL, &fr_current_time) == 0) {
c               r5 = fr_current_time.it_interval.tv_usec - fr_current_time.it_value.tv_usec;
c       } else {
c               r5 = 0;
c       }
#
        stq     r4,fr_time(r13)         # Store both timers, Xpcb, and RIP
#
.endif                                  # DEBUG_FLIGHTREC_TIME
#
# --- Update the next entry pointer
#
        ldconst fr_size,r3              # size of a flight recorder entry
        addo    r3,r13,r3               # increment the IN pointer
        cmpo    r3,r15                  # check for wrap: IN = END?
        selge   r3,r12,r3               # wrap if necessary: IN = BEGIN
#
        st      r3,qc_in(r10)           # save the updated IN
.lf10:
        ret
.endif                                  # DEBUG_FLIGHTREC
#
.ifdef FRONTEND
#******************************************************************************
#
#  NAME: M$findtgt
#
#  PURPOSE:
#       Find a Target ID based on a WWN
#
#  DESCRIPTION:
#       Finds which Target ID is associated with which WWN.  The check is done
#       on both the Node WWN and the Port WWN.
#
#  CALLING SEQUENCE:
#       call    M$findtgt
#
#  INPUT:
#       g0-g1 = WWN address to check
#
#  OUTPUT:
#       g2 = Target ID, if found
#          = 0xFFFFFFFF, if not found
#
#******************************************************************************
#
M$findtgt:
        lda     T_tgdindx,r10           # Get pointer to TGX structures
        ld      tgx_ecnt(r10),r8        # Get configured number of targets
        cmpobe  0,r8,.mft30             # Jif list is empty
#
        ldconst 0,r3                    # Start at top of list
        ldconst MAXTARGETS,r8           # Maximum number of targets
#
.mft10:
        ld      tgx_tgd(r10)[r3*4],r6   # Get next target
        cmpobe  0,r6,.mft25             # Jif no target at this slot
#
# --- Check for matching WWN
#
        ldl     tgd_pname(r6),r12       # r12-r13 = Port WWN
        cmpobne g0,r12,.mft15           # Jif there is not a match
        cmpobe  g1,r13,.mft40           # Jif a match has been found
.mft15:
        ldl     tgd_nname(r6),r14       # r14-r15 = Node WWN
        cmpobne g0,r14,.mft20           # Jif there is not a match
        cmpobe  g1,r15,.mft40           # Jif there is a match
#
# --- Convert node name to port name & compare ( needed for iSCSI )
#
.mft20:
        ldl     XIO_WWN_p2n,r12         # r6-r7 = node name mask
        and     r12,g0,g0                # r6-r7 = masked target node name
        and     r13,g1,g1
#  c fprintf(stderr, "mft20: g0= 0x%X, g1= 0x%X, r14= 0x%X r15 = 0x%X\n", (UINT32)g0, (UINT32)g1, (UINT32)r14, (UINT32)r15);
        cmpobne g0,r14,.mft25           # Jif there is not a match
        cmpobe  g1,r15,.mft40           # Jif there is a match
#
# --- Increment and check loop bounds
#
.mft25:
        addo    1,r3,r3                 # Increment loop counter
        cmpobl  r3,r8,.mft10            # Jif not end of list
#
# --- A target was not found.
#
.mft30:
        ldconst 0xFFFFFFFF,g2           # No target found
        b       .mft100
#
# --- A target was found, return target ID.
#
.mft40:
        ldos    tgd_tid(r6),g2          # Get Target ID
#
# --- Exit
#
.mft100:
        ret
#
        .data
XIO_WWN_p2n:
        .byte   0xf0,0xff,0xff,0xff     # name mask
        .byte   0xff,0xff,0xff,0xff
#
        .text
.endif  # FRONTEND
#
#******************************************************************************
#
#  NAME: M$soft_flt
#
#  PURPOSE:
#       Handle a Software Detected Fault
#
#  DESCRIPTION:
#       Depending on the SFTDEBUG flag, this function will Log a Firmware
#       Alert message with the data requested or will Error Trap.
#
#       Note: see def.inc for Error Code assignments
#
#  CALLING SEQUENCE:
#       call    M$soft_flt
#
#  INPUT:
#       g0 = Address of data to log (mle log buffer structure and size)
#
#  OUTPUT:
#       None
#
#******************************************************************************
#
M$soft_flt:
        movt    g4,r12                  # Save g4-g6
        movq    g0,r8                   # Save g0-g3

#
# Send a Fimrware Alert Log Event to note the problem
#
c       MSC_SoftFault((void*)g0);
#
# --- Exit
#
        movt    r12,g4                  # restore g4-g6
        movq    r8,g0                   # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: M$que_hbeat
#
#  PURPOSE:
#       To provide a common means of queuing heartbeat MRPs to
#       m$exec_hbeatproc.
#
#  DESCRIPTION:
#       The ILT and associated MRP are queued to the tail of the
#       executive queue.  The executive is activated to process this
#       request.  This routine may be called from either the process or
#       interrupt level.
#
#  CALLING SEQUENCE:
#       call    M$que_hbeat
#
#  INPUT:
#       g1 = ILT
#
#  OUTPUT:
#       None.
#
#**********************************************************************
#
# Note: There is a side effect of this being in be_proto - it allows this
# routine to exist ("c" goto in array table), and the fall through to
# M$que_hbeat, which then allows the asm's to exist. Otherwise gcc tosses.
#
        .globl  MSC_QueHBeat
MSC_QueHBeat:
        mov     g0,g1
# fall through
#
# M$que_hbeat:
c       asm("   .globl  M$que_hbeat");
c       asm("M$que_hbeat:     ");
        ld      K_ii+ii_time,r8         # read gross timer
#        c       fprintf (stderr, "Received heartbeat! ####################### %lu\n", r8);
#
# --- Trace arrival of MRP
#
.if     DEBUG_HBEATREC

        ldconst trHBArrive,g0            # Trace arrival of Heartbeat MRP
        call    M$TraceEvent
.endif  # DEBUG_HBEATREC

        lda     m_hbeat_qu,r11           # Get queue origin
        b       K$cque
#
#**********************************************************************
#
#  NAME: m$exec_hbeatproc
#
#  PURPOSE:
#       To provide a means of processing heartbeat MRP requests which have been
#       previously queued to this module.
#
#  DESCRIPTION:
#       The queuing routine M$que_hbeat deposits a MRP request into the queue
#       and activates this executive if necessary.  This executive
#       extracts the next MRP request from the queue and initiates that
#       request.
#
#       The CCB expects this MRP to be returned within several seconds. If
#       it's not, then the CCB will reset the main processor. This task
#       should have a high priority.
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
#**********************************************************************
#
m$exec_hbeatproc:
#
# --- Setup exec constants
#
        lda     hbeat_count,r7          # Address of counter
        ldconst deok,r8                 # Set OK status
        ldconst mhbrsiz,r9              # Return packet size
        b       .mexp20                 # Exchange to start off
#
# --- Set this process to not ready
#
.mexp10:
        ldconst pcnrdy,r4               # Set this process to not ready
        stob    r4,pc_stat(r15)
#
.mexp20:
        call    K$qxchang               # Exchange processes
#
# --- Get next queued request
#
        lda     m_hbeat_qu,r11          # Get executive queue pointer
        ldq     qu_head(r11),r12        # Get queue head, tail, count and PCB
        mov     r12,g1                  # Isolate next queued ILT
        cmpobe  0,r12,.mexp10           # Jif none
#
# --- Remove this request from queue
#
        ld      il_fthd(r12),r12        # Dequeue this ILT
        cmpo    0,r12                   # Check for queue now empty
        subo    1,r14,r14               # Adjust queue count
        sele    r13,r12,r13             # Set up queue tail
        stt     r12,qu_head(r11)        # Update queue head, tail and count
        be      .mexp30                 # Jif queue now empty
#
        st      r11,il_bthd(r12)        # Update backward thread
#
.mexp30:

.if     DEBUG_HBEATREC
#
# --- Trace execution of MRP
#
        ldconst trHBExec,g0             # Trace execution of MRP
        call    M$TraceEvent
.endif  # DEBUG_HBEATREC
#
# --- Handle the heartbeat
#
        ld      il_w0-ILTBIAS(g1),g2    # Get request ptr
        ld      mr_rptr(g2),g3          # Get response packet ptr
        ld      mr_ptr(g2),g4           # Get packet pointer
        ldt     mhb_timestamp(g4),g12   # Get timestamp
        stt     g12,hbeat_timestamp     # Save timestamp
        atadd   r7,1,r3                 # Increment heartbeat count
#
.ifdef FRONTEND
?       ldl     im_peragg+imst_cmds,g8  # Get periodic aggregate stats
?       ldl     im_peragg+imst_bytes,g10
!       stl     g8,mhb_iopersec(g3)     # Save periodic aggregate stats
!       stl     g10,mhb_bpersec(g3)
.endif  # FRONTEND
!       stob    r8,mr_status(g3)        # Plug return status code
!       st      r9,mr_rlen(g3)          # Set return packet size
#
# --- Send status response
#
        call    K$comp                  # Complete this request
        b       .mexp20
#
#******************************************************************************
#
#  NAME: m$exec_hbeatmon
#
#  PURPOSE:
#       Monitor the heartbeats coming from the CCB.
#
#  DESCRIPTION:
#       The function monitors heartbeats from the CCB to both the FE and BE.
#       If there are no heartbeats within time hbeat_time then the controller
#       is put into a hardware reset (for an n-way system)
#
#       The mode page MRP has a bit to disable the heartbeat checking for debug.
#       hbeat_time is a variable so it can be easily modified for debug.
#
#  CALLING SEQUENCE:
#       fork    m$hbeat
#
#  INPUT:
#       g0 = Address of data to log (mle log buffer structure and size)
#
#  OUTPUT:
#       None
#
#******************************************************************************
#
m$exec_hbeatmon:
        ldconst 0,g14                   # Initialize constants
        ldconst FALSE,g13               # G13 = log msg sent
#
# --- Don't check heartbeats until CCB has a chance to come ready
#
.mp10:
        ldconst 5*1000,g0               # Delay a few seconds
        call    K$twait
        ldos    K_ii+ii_status,r10      # Get current status
.ifdef  BACKEND
        bbc     iifulldef,r10,.mp10     # Jif full define running - continue
.else   # BACKEND
        bbc     iiserver,r10,.mp10      # Jif servers setup - continue
.endif  # BACKEND
#
# --- Loop forever checking hbeat counter -----------------------------
#
.mp20:
        ld      hbeat_time,g0           # Wait for designated hbeat time
        call    K$twait
#
        ld      hbeat_count,r3          # Get latest count
        cmpobe  0,r3,.mp30              # Jif count = 0: no hbeats have occurred
#
.mp50:
        st      g14,hbeat_count         # Clear the count
        ldconst FALSE,g13               # Clear 'log msg sent'
        b       .mp20                   # Wait again
#
# --- Handle missing heartbeats ---------------------------------------
#
.mp30:
        ld      ctrl_shutdown,r3        # Is controller shutting down?
        cmpobe  TRUE,r3,.mp20           # Jif controller shutdown enabled
#
# --- See if a msg has already been sent
#
        cmpobe  TRUE,g13,.mp40          # Jif if we already have sent up msg
        ldconst TRUE,g13                # Set g13, log msg sent to true
#
# --- On the first missing heartbeat: log an event, call Link960, disable cache
#
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst mlehbeatstop,r4         # Event code
        st      r4,mle_event(g0)        # Store as word to clear other bytes
.ifdef FRONTEND
        ldconst ehsprocfe,r4            # Load proc field with FE
.else   # FRONTEND
        ldconst ehsprocbe,r4            # Load proc field with BE
.endif  # FRONTEND
        stob    r4,ehs_proc(g0)         # Store proc field
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], ehslen);
#
# --- Decide if this is a hard error
#
.mp40:
        ldos    K_ii+ii_status,r10      # Is CCB required? (n-way)
        bbc     iiccbreq,r10,.mp20      # Jif only one controller present
#
        ld      hbeat_disable,r3        # Is this check disabled?
        cmpobe  FALSE,r3,.mp50          # Jif if not disabled - Suicide FE/BE
#
# During development, Error Trap when the Debug Flag is set
#
.if     SFTDEBUG
        ldconst 3000,g0                 # Wait so the log can get to the CCB
        call    K$twait
        b       .err04                  # Error Trap as requested
.else   /* SFTDEBUG */
        b       .mp20                   # Return to watching for heartbeats
.endif  /* SFTDEBUG */
#
#**********************************************************************
#
#  NAME: m$logMsgRelCmplt
#
#  PURPOSE:
#       To provide a completion function for MSC_LogMessageRel.
#
#  DESCRIPTION:
#       A log message has been sent and received by the CCB.  It is the
#       responsibility of this function to deallocate the memory used
#       by MSC_LogMessageRel.
#
#  CALLING SEQUENCE:
#       completion function for MSC_LogMessageRel.
#
#  INPUT:
#       g3 - address of buffer to be freed.
#
#  OUTPUT:
#       None.
#
#**********************************************************************
#
m$logMsgRelCmplt:
#
# --- Pass the buffer and length into the memory release function
#
        ld      mle_len(g3),r3          # Get parameter length
        addo    mle_bstream,r3,r3       # Calc Packet length
c       s_Free(g3, r3, __FILE__, __LINE__); # Release the memory
#
        ret
#
#**********************************************************************
#
#  NAME: MSC_LogMessageRel
#
#  PURPOSE:
#       To provide a means of sending log message to the CCB.
#
#  DESCRIPTION:
#       An error log message code is constructed with the supplied
#       error code and that message is sent to the CCB.
#
#       The buffer is assume to be in non-cachable DRAM and is released
#       after the packet is sent.
#
#  CALLING SEQUENCE:
#       call    MSC_LogMessageRel
#
#  INPUT:
#       g0 = pointer to log entry
#       g1 = size of log entry
#
#  OUTPUT:
#       None.
#
#**********************************************************************
#
# C access
# void MSC_LogMessageRel(void* pLogEntry, UINT32 size);
        .globl  MSC_LogMessageRel
MSC_LogMessageRel:
#
        mov     g0,r12                  # Save g0
        mov     g1,r13                  # Save g1
        mov     g2,r14                  # Save g2
        mov     g3,r15                  # Save g3
        mov     g4,r10                  # Save g4
        mov     g5,r11                  # Save g5
        mov     g6,r9                   # Save g6
#
.ifdef BACKEND
# --- Process any updates to DMC data structures that CCB might need for
#     complete processing of this request by old PI data Cache layer.
#
c       if (LOG_NotDebug(((LOG_HEADER_PKT*)g0)->event)) {
c           Process_DMC();              # Process possible DMC requests.
c       }
.endif  # BACKEND
#
        subo    mle_bstream,g1,r4       # Get parameter length
        st      r4,mle_len(g0)          # Set parameter length
.ifdef FRONTEND
        ldconst mrlogfe,g2              # MRP function code
.else   # FRONTEND
        ldconst mrlogbe,g2              # MRP function code
.endif  # FRONTEND
        ldconst 0,g3                    # Return data address (no data)
        ldconst 0,g4                    # No data allowed
        ldconst m$logMsgRelCmplt,g5     # Completion function
        mov     g0,g6                   # g6 has local buffer address
#
        call    L$send_packet           # Send the packet
#
# --- Exit
#
        mov     r12,g0                  # Restore g0
        mov     r13,g1                  # Restore g1
        mov     r14,g2                  # Restore g2
        mov     r15,g3                  # Restore g3
        mov     r10,g4                  # Restore g4
        mov     r11,g5                  # Restore g5
        mov     r9,g6                   # Restore g6
        ret
#
#****************************************************************************
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
