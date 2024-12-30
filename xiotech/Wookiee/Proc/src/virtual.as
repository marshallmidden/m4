# $Id: virtual.as 159305 2012-06-16 08:00:46Z m4 $
#**********************************************************************
#
#  NAME: virtual.as (Thunderbolt)
#
#  PURPOSE:
#
#       To provide support for virtual I/O as a layer between the cache
#       and RAID layers.
#
#  FUNCTIONS:
#
#       This module employs 4 processes:
#
#       v$exec         - Executive (1 copy)
#       v$stats        - Statistics (1 copy)
#       v$seccopy      - Secondary copy completion (1 copy)
#       v$sched_copy   - Schedule secondary copy (1 copy)
#
#  Copyright (c) 1996-2008 XIOtech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- global function declarations ------------------------------------
#
        .globl  V$init                  # Module initialization
        .globl  V$que                   # Queuing routine
        .globl  V$updFEStatus           # Update the FE with the Mirror/Copy status
        .globl  V_updFEStatus           # Update the FE with the Mirror/Copy status-- 'C' interface
#
# --- global data declarations ----------------------------------------
#
        .globl  V_vddindx               # Index to VDD index
        .globl  vsc_qu_head
        .globl  vsched_qu_head
        .globl  V_orc
        .globl  VPri_enable             # Priority Enable/Disabled
        .globl  V_skipthrsh             # Skip threshold for Vdisk pri
        .globl  V_skptblpu              # Skip table WRT % proc util
        .globl  v$vmcomp
        .globl  v$vscomp
        .globl  v_callx                 # Special routine for C to do a callx.
#
# --- global usage data definitions -----------------------------------
#
        .data
#
# --- Secondary copy data definitions ---------------------------------
#
vsc_qu_head:
        .word   0                       # sec. copy process work queue head
vsc_qu_tail:
        .word   0                       # sec. copy process work queue tail
vsc_pcb:
        .word   0                       # sec. copy process pcb address
#
# --- Scheduled secondary copy process data definitions ----------------
#
#
# --- Start secondary copy process data definitions --------------------
#
vstart_qu_head:
        .word   0                       # start sec. copy process work queue
                                        #  head
vstart_qu_tail:
        .word   0                       # start sec. copy process work queue
                                        #  tail
vsched_qu_head:
        .word   0                       # scheduled sec. copy process work
                                        #  queue head
vsched_qu_tail:
        .word   0                       # scheduled sec. copy process work
                                        #  queue tail
vsched_pcb:
        .word   0                       # scheduled sec. copy process pcb
                                        #  address
vsched_act:
        .word   0                       # active secondary copy count
#
# --- Error counters
#
vsc_rcpstate:
        .word   0                       # copy read op. completion event
                                        #  process state error counter
vsc_rcpstatus:
        .word   0                       # copy read op. completion event
                                        #  status error counter
vsc_wcpstate:
        .word   0                       # copy write op. completion event
                                        #  process state error counter
vsc_wcpstatus:
        .word   0                       # copy write op. completion event
                                        #  status error counter
vsc_ucpstatus:
        .word   0                       # copy update op. completion event
                                        #  status error counter
VPri_enable:
        .byte   1
v_skipcnt:
        .word   0                       # Skip counter for v$exec

V_skipthrsh:
        .byte   0                       # Threshold loaded from % table below
        .byte   0                       # Threshold loaded from MB/sec table below.

v_skptblmb:                             # Table for skip counter threshold WRT MB/sec
        .byte   0                       # 0-10 MB/sec
        .byte   0                       # 10-20 MB/sec
        .byte   0                       # 20-30 MB/sec
        .byte   0                       # 30-40 MB/sec
        .byte   0                       # 40-50 MB/sec
        .byte   0                       # 50-60 MB/sec
        .byte   0                       # 60-63 MB/sec

V_skptblpu:                             # Table for skip counter threshold WRT % proc
        .byte   3                       # 0-9 %
        .byte   0                       # 10-19 %
        .byte   0                       # 20-29 %
        .byte   0                       # 30-39 %
        .byte   0                       # 40-49 %
        .byte   0                       # 50-59 %
        .byte   0                       # 60-69 %
        .byte   0                       # 70-79 %
        .byte   0                       # 80-89 %
        .byte   0                       # 90-99 %
        .byte   0                       # 100-109 %

v_hpmb:                                 # Total sect/sec for all vdisks
        .word   0
#
# --- local usage data definitions ------------------------------------
#
dummyvar:       .word   V$que           # Needed to prevent C compiler from
                                        # optimizing out the routine!
#
# --- executable code -------------------------------------------------
#
#**********************************************************************
#
#  NAME: V$init
#
#  PURPOSE:
#
#       To provide a means of initializing this module.
#
#  DESCRIPTION:
#
#       The executive and statistics processes for this module are
#       established and made ready for execution.
#
#  CALLING SEQUENCE:
#
#       call    V$init
#
#  INPUT:
#
#       g3 = FICB
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       g0
#       g1
#       g2
#       g3
#
#**********************************************************************
#
        .text
V$init:
c       GR_InitErrorQueHandler();
#
# --- Establish executive process
#
        lda     v$exec,g0               # Establish executive process
        ldconst VEXECPRI,g1
c       CT_fork_tmp = (ulong)"v$exec";
        call    K$fork
        st      g0,V_exec_qu+qu_pcb     # Save PCB
        st      g0,V_exec_mqu+qu_pcb    # Save PCB
        st      g0,V_exec_hqu+qu_pcb    # Save PCB
#
# --- Initialize Debug Data Retrieval (DDR) V_exec_qu PCB entry
#
c       M_addDDRentry(de_vdpcb, g0, pcbsiz);
#
# --- Initialize Debug Data Retrieval (DDR) V_exec_qu entry
#
        lda     V_exec_qu,g1            # Load address of V_exec_qu header
c       M_addDDRentry(de_vdeque, g1, 16);   # Size of V_exec_qu header
#
# --- Establish statistics process
#
        lda     v$stats,g0              # Establish statistics process
        ldconst VSTATSPRI,g1
c       CT_fork_tmp = (ulong)"v$stats";
        call    K$fork
#
# --- Establish copy on write queue handler for snapshot
#
        lda     cow_q_task,g0           # Establish executive process
        ldconst VEXECPRI,g1
c       CT_fork_tmp = (ulong)"cow_q_task";
        call    K$fork
        st      g0,SS_cow_qu+qu_pcb     # Save PCB

#
# --- Exit
#
        ret
#**********************************************************************
#
#  NAME: V$que
#
#  PURPOSE:
#
#       To provide a common means of queuing virtual I/O requests
#       to this module.
#
#  DESCRIPTION:
#
#       The ILT and associated VRP are queued to the tail of the
#       executive queue.  The executive is activated to process this
#       request.  This routine may be called from either the process or
#       interrupt level.
#
#  CALLING SEQUENCE:
#
#       call    V$que
#
#  INPUT:
#
#       g1 = ILT
#
#        il_w0 = VRP
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#**********************************************************************
#
V$que:
V_que:
c       asm("   .globl  V$que");
c       asm("V$que:     ");
        ldob    VPri_enable,r3
        cmpobe 0,r3,.vq10               # Jif VPri disabled

        ld      il_w0-ILTBIAS(g1),r3    # Get the VRP
        cmpobe.f 0,r3,.vq10             # Jif no VRP

# Added
#        ldob    vr_strategy(r3),r8
#        ldconst vrhigh,r9
#        cmpobne r9,r8,.vq03             # Jif not already High
#        ld      V_exec_hqu, r11
#        b       .vq05
#.vq03:
# Added

        ldos    vr_vid(r3),r5           # Get the VID
        ldconst MAXVIRTUALS,r11
        cmpobge r5,r11,.vq10            # Jif probably bogus ILT.
        ld      V_primap[r5*4],r11      # Get the proper queue

#        ldob    vr_strategy(r3),r8
#        ldconst vrhigh,r9
#        cmpobe  r9,r8,.vq05            # Push into high pri que
#        ld      V_vddindx[r5*4],r6     # Get VDD
#        ldob    vd_strategy(r6),r10
#        stob    r10,vr_strategy(r3)
# .vq05:
        b       K$cque
#
.vq10:
        lda     V_exec_qu,r11           # Get queue origin
        b       K$cque
#
#**********************************************************************
#
#  NAME: V$xque
#
#  PURPOSE:
#
#       To provide a common means of queuing expedited virtual I/O
#       requests to this module.
#
#  DESCRIPTION:
#
#       The ILT and associated VRP are queued to the tail of the expedited
#       executive queue.  The executive is activated to process this
#       request.  This routine may be called from either the process or
#       interrupt level.
#
#  CALLING SEQUENCE:
#
#       call    V$xque
#
#  INPUT:
#
#       g1 = ILT
#
#        il_w0 = VRP
#        il_w2 = Session Node pointer if != 0
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#**********************************************************************
#
V$xque:
        lda     V_exec_xqu,r11          # Get queue origin
        b       K$cque
#
#**********************************************************************
#
#  NAME: v$exec
#
#  PURPOSE:
#
#       To provide a means of processing VRP requests which have been
#       previously queued to this module.
#
#  DESCRIPTION:
#
#       The queuing routine V$que deposits a VRP request into the queue
#       and activates this executive if necessary.  This executive
#       extracts the next VRP request from the queue and initiates that
#       request by queuing RRP requests to the RAID module.
#
#       A separate completion routine handles the completion of RRP
#       requests.
#
#  CALLING SEQUENCE:
#
#       process call
#
#  INPUT:
#
#       None.
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#**********************************************************************
#
        .globl NHPRI
        .globl NMPRI
        .globl NLPRI
        .data
NHPRI:
        .word   0
NMPRI:
        .word   0
NLPRI:
        .word   0
#
        .text
#
# --- Set this process to not ready
#
.vex10:
        ldconst pcnrdy,r4               # Set this process to not ready
        stob    r4,pc_stat(r15)
#
# --- Exchange processes ----------------------------------------------
#
v$exec:
        call    K$qxchang               # Exchange processes
.vex20:
#
# --- Get next expedited queued request
#
        ldconst 5,r8
        lda     V_exec_xqu,r11          # Get expedited queue pointer
        ldq     qu_head(r11),r12        # Get queue head, tail, count
                                        #  and PCB
        cmpobne.f 0,r12,.ex25_v         # Jif expedited request found
#
# --- Get next queued request
#
        ldob    VPri_enable,r3
        cmpobe  0,r3,.vexnopri1         # Jif VPri disabled
#
# --- Get next high priority queued request
#
        lda     V_exec_hqu,r11          # Get high pri queue pointer
        ldq     qu_head(r11),r12        # Get queue head, tail, count
                                        #  and PCB
        cmpobne 0,r12,.ex24             # Process high pri queue entry.

        call    K$qxchang               # Exchange processes
#
# --- Allow high priority queue entry to be put in by another task, then process it.
#
        lda     V_exec_hqu,r11          # Get high pri queue pointer
        ldq     qu_head(r11),r12        # Get queue head, tail, count
                                        #  and PCB
        cmpobne.f 0,r12,.ex24           # Jif expedited request found
#
# --- Get next medium priority queued request
#
        lda     V_exec_mqu,r11          # Get med pri queue pointer
        ldq     qu_head(r11),r12        # Get queue head, tail, count
                                        #  and PCB
        cmpobne.f 0,r12,.ex23_1         # Jif expedited request found
#
# --- Check normal queue for requests
#
        lda     V_exec_qu,r11           # Get normal queue pointer
        ldq     qu_head(r11),r12        # Get queue head, tail, count
                                        # and PCB
        cmpobe.f 0,r12,.vex10           # Jif none
#
# --- Get next normal queued request as long as the system is not too busy.
#     Check for a skip count threshold set by condition of processor load
#     or high priority MB/sec.  Take the max of the two values as the skip
#     count.
#
        ld      v_hpmb,r5
        cmpobe  0,r5,.ex22              # Jif high priority traffic < 1MB/Sec
        ldos    V_skipthrsh,r5
        cmpobe.t 0,r5,.ex22             # Jif no skip count
        ldconst 0xff,r6
        and     r6,r5,r7                # Extract the % threshold
        shro    8,r5,r5                 # Extract the MB/s threshold
        cmpobge r5,r7,.ex21             # Keep the biggest one (usually %)
        mov     r7,r5                   # Update r5
        cmpobe  0,r5,.ex22
.ex21:
        ld      v_skipcnt,r6            # Get and increment skip counter
        addo    1,r6,r6
        st      r6,v_skipcnt
        cmpoble.t r5,r6,.ex22           # Jif skip count threshold is met
        b       .vex20

.ex22:
        mov     0,r6
        st      r6,v_skipcnt            # Clear skip count
        ld      NLPRI,r6
        addo    r6,1,r6
        st      r6,NLPRI
        ldconst visetlow,r8
        b       .vex25

.ex23_1:
        ld      NMPRI,r6
        addo    r6,1,r6
        st      r6,NMPRI
        ldconst visetmed,r8
        b       .vex25

.ex24:
        ld      NHPRI,r6
        addo    r6,1,r6
        ldconst visethigh,r8
        st      r6,NHPRI
        b       .vex25
#
# --- Priority Disabled
.vexnopri1:
        lda     V_exec_qu,r11           # Get executive queue pointer
        ldq     qu_head(r11),r12        # Get queue head, tail, count
                                        #  and PCB
        cmpobe  0,r12,.vex10            # Jif none
.vex25:
#
# --- Remove this request from queue ----------------------------------
#
.ex25_v:
        mov     r12,g14                 # Isolate next queued ILT
        ld      il_fthd(r12),r12        # Dequeue this ILT
        cmpo    0,r12                   # Check for queue now empty
        subo    1,r14,r14               # Adjust queue count
        sele    r13,r12,r13             # Set up queue tail
        stt     r12,qu_head(r11)        # Update queue head, tail and count
        be      .vex30                  # Jif queue now empty
#
        st      r11,il_bthd(r12)        # Update backward thread
#
.vex30:
#
        ld      il_w0-ILTBIAS(g14),r15  # Get VRP request
        ldob    VPri_enable,r4
        cmpobe  0,r4,.vex35             # Jif VPri disabled
        cmpobe  5,r8,.vex35
        ldob    vr_strategy(r15),r3
        cmpobe  2,r3,.vex35             # Jif already high
        stob    r8,vr_strategy(r15)
.vex35:
#
# --- Validate function code
#
        ldos    vr_func(r15),r14        # Get function code
#
.if     MAG2MAG
        ldconst vrmagst,r3              # r3 = first VRP function code for
                                        #  DLM functions
        cmpobg  r3,r14,.vex40           # Jif not a DLM function code
        ldconst vrmagend,r3             # r3 = last VRP function code for
                                        #  DLM functions
        cmpobg  r14,r3,.vex40           # Jif not a DLM function code
        mov     g14,g1                  # g1 = ILT
#       g1 = ILT at nest level #1
#             dlmi_vrp  (il_w0) = VRP address
        call    DLM$vque                # Queue the request to DLM
        b       .vex20
#
.vex40:
.endif  # MAG2MAG
#
        mov     0,r3                    # Clear the vr_path and vr_options
#       stob    r3,vr_path(r15)         #   field for RAID and other code
#       stob    r3,vr_options(r15)      #   that follows
        stob    r3,vr_path(r15)         # Clear vr_path

        ldconst FALSE,r7                # clear read/write flag
        clrbit  vrspecial,r14,r14       # clear special processing flag
        clrbit  vrssproc,r14,r14        # clear snapshot processing flag
        clrbit  apool_bypass,r14,r14    # clear the apool bypass bit
        mov     0,g13                   # Clear out the SGL Pointer for Copies
        cmpobe  vrinput,r14,.vex50      # Jif read
        cmpobe  vroutput,r14,.vex50     # Jif write
        cmpobe  vroutputv,r14,.vex50    # Jif write/verify
        cmpobe  vrverifyc,r14,.vex55    # Jif verify checkword
        cmpobe  vrrebuildchk,r14,.vex55 # Jif rebuild check
        cmpobne vrverify,r14,.vex980    # Jif not verify data
#
# --- Validate SGL
#
.vex50:
        ldconst TRUE,r7                 # set read/write flag
        ld      vr_sglptr(r15),g13      # Get SGL pointer
c       if (g13 == 0xfeedf00d) {
c           fprintf(stderr,"%s%s:%u v$exec sgl 0xfeedf00d\n", FEBEMESSAGE, __FILE__, __LINE__);
c           abort();
c       }
        cmpobe  0,g13,.vex950           # Jif null
#
# --- Validate VID and obtain VDD
#
.vex55:
c       record_virtual(FR_VRP_EXEC, (void *)r15);
.if     DEBUG_FLIGHTREC_V
        ldconst frt_v_exec,r3           # Type
        shlo    16,r14,r4               # Get the Function code
        or      r4,r3,r3                # Get Function code and FR code
        st      r3,fr_parm0             # Virtual - v$exec
        st      g14,fr_parm1            # ILT
        st      r15,fr_parm2            # VRP
#        st      g13,fr_parm3            # SGL
        ldos    vr_func(r15),r3         # Get function code
        stos    r3,fr_parm3
        ldos    vr_vid(r15),r3          # get vid
        stos    r3,fr_parm3+2
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_V

        ldos    vr_vid(r15),r3          # Get VID
        ldconst MAXVIRTUALS,r4          # Validate VID
        cmpobge r3,r4,.vex970           # Jif invalid
#
        ld      V_vddindx[r3*4],g12     # Get VDD
        cmpobe  0,g12,.vex990           # Jif VDD not defined

c       GR_ResetVRPILT((ILT*)g14, (VRP*)r15, (VDD *)g12);
#
# --- Check to see if the Vdisk is BUSY
#
        ldos    vd_attr(g12),r3         # Get the attributes
        ldconst ecbebusy,r5             # Set BUSY error
        bbs     vdbebusy,r3,.vex1000    # jif VID BUSY
#
# --- Check to see if the Vdisk is a Vlink and an Alink
#
        ldconst vdbvlink,r5
        bbc     r5,r3,.vex56            # Jif not Vlink
        bbc     vdbasync,r3,.vex56      # jif not alink
        ldos    vr_func(r15),r3         # Get function code
        ldconst apool_bypass,r5
        bbs     r5,r3,.vex56            # Jif bypass bit is set
        mov     g14,g1
        PushRegs(r5)                    # Save g registers.
#       g1 = ilt         - ilt to use.
#       g2 = apool_id    - Apool working on.    -- NOTDONEYET
        call    apool_put
        PopRegs(r5)                     # Restore g registers (except g0).
c       r5 = g0;                        # Error value must be in r5.
        cmpobne ecok,g0,.vex1000        # If error occurred
        ldob    vr_strategy(r15),r8     # Restore strategy (vdisk priority).
        b       .vex30

.vex56:
        ldos    vr_func(r15),r3         # Get function code
        ldconst apool_bypass,r5
        clrbit  r5,r3,r3
        stos    r3,vr_func(r15)         # Clear the async bypass bit
#
# --- Update the last access time on VDD unless it is an alink/apool
#
        ldos    vd_attr(g12),r3         # Get the attributes
        bbs     vdbasync,r3,.vex57      # jif alink
        cmpobe FALSE,r7,.vex57          # Jif not read/write/write verify
        ld      K_ii+ii_time,r3         # Get current time
        st      r3,vd_lastaccess(g12)   # Store the last access time
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
.vex57:
#
# --- Validate disk addressing and device status
#
c   if (((VRP *)r15)->startDiskAddr >= ((VDD *)g12)->devCap) {
        b       .vex940                 # if SDA > device capacity
c   }
        ld      vr_vlen(r15),g10        # Get length
        cmpobe  0,g10,.vex930           # Jif null length
c   if (((VRP *)r15)->startDiskAddr + g10 > ((VDD *)g12)->devCap) {
        b       .vex920                 # Jump if ending address > device capacity
c   }
## Verify the operating state of vdisk and if necessary perform auto swap
c       r5 = GR_VerifyVdiskOpState((VDD*)g12, (VRP*)r15);
        cmpobne ecok,r5,.vex1000        # Jif device not operational
#
        ld      vr_func(r15),r14        # Get function/strategy
        ldconst 0xffffff,r4             # mask for to isolate function/strategy
        ldconst 0xffff,r5               # mask for to isolate function
        and     r4,r14,g4               # g4 = function/strategy
        and     r5,r14,r14              # r14 = function
        clrbit  vrspecial,g4,g4         # clear special processing flag
        clrbit  vrssproc,g4,g4          # Clear the snapshot processing flag
#
# --- Look for snapshot related virtual device
#
        bbc     vrssproc,r14,.ex57a     # If snapshot bit is clear do
                                        # snapshot processing.
        clrbit  vrssproc,r14,r14
        stos    r14,vr_func(r15)        # Clear the Snapshot bit
        b       .ex58                   # If set, clear and don't process.

.ex57a:
        ld      vd_incssms(g12),r3      # r3 = Snapshot SSMS pointer
        ld      vd_outssms(g12),r4      # r4 = first source for snapshot pointer
        or      r3,r4,r5
        cmpobe  0,r5,.ex58              # Jif no assoc snapshot
        cmpobne 0,r3,.ex57c             # Jif access to snapshot
        mov     r15,g3                  # g3 = VRP
        mov     r3,g1                   # g1 = Incoming SSMS
        mov     r4,g2                   # g2 = First outgoing SSMS
        ldos    vr_func(r15),g4         # g4 = function/strategy

        ldconst 0xff,r4
        and     r4,g4,r5                # r5 = VRP function code
        clrbit  vrspecial,r5,r5
        cmpobe  vroutput,r5,.ex57b      # Jif write
        cmpobe  vroutputv,r5,.ex57b     # Jif write and verify

.ex57aa:
        ldos    vr_func(g3),r5
        setbit  vrssproc,r5,r5
        stos    r5,vr_func(g3)          # Update the ss special processing flag
        b       .ex58                   # Read to the source goes to the original code

.ex57b:
c       g11 = r15;                      # Put VRP pointer into g11 for callx.
        mov     g14,r5                  # Save g14
        PushRegs(r6)                    # Save g registers
c       r3 = write_ss_source((SSMS *)g2, (ILT *)r5, ((VRP*)g11)->startDiskAddr);
        PopRegsVoid(r6)                 # Restore g registers
        mov     r5,g14                  # Restore g14
        cmpobe  FALSE,r3,.ex57aa
        b       .vex20                  # Go check for more VRPs

.ex57c:
c       g11 = r15;                      # Put VRP pointer into g11
        mov     g14,r5                  # Save g14
        PushRegs(r6)                    # Save g registers
c       access_snapshot((SSMS *)r3, (ILT *)r5, ((VRP*)g11)->startDiskAddr);
        PopRegsVoid(r6)                 # Restore g registers
        b       .vex20                  # Go check for more VRPs

.ex58:
#
# --- Check for any phase 1 special processing that may be required
#     if a copy is active or suspended.
#
        ld      vd_dcd(g12),r3          # r3 = assoc. DCD address
        cmpobe.t 0,r3,.vex58            # Jif not the dest. copy device
                                        #  for any copy operations
        bbs.f   vrspecial,r14,.vex58    # Jif special processing VRP function
                                        #  code flag set
        ldob    dcd_type(r3),r10        # r10 = DCD type code
        cmpobe.f dcdt_remote,r10,.vex58 # Jif remote DCD (no special
                                        #  processing)
        ld      dcd_cor(r3),r4          # r4 = assoc. COR address
        ldob    cor_crstate(r4),r10     # r10 = COR reg. state
        cmpobe.t corcrst_usersusp,r10,.vex58 # Jif copy operation is user
                                        #  suspended
        cmpobne.t corcrst_remsusp,r10,.vex990 # Jif copy operation is not
                                        #  remote suspended
#
# --- Bump outstanding request count
#
.vex58:
        ld      V_orc,r3                # Bump outstanding request count
        addo    1,r3,r3
        st      r3,V_orc

#****************************************************************************
#
#  Reg. Definitions:
#
#       r14 = VRP function code
#       r15 = VRP address
#       g4  = VRP function/strategy
#       g10 = I/O length
#       g12 = VDD address
#       g13 = VRP SGL
#       g14 = primary ILT/VRP address
#
#***************************************************************************
#
        cmpobe.f vroutput,r14,.vex60     # Jif write VRP
        cmpobne.t vroutputv,r14,.vex100  # Jif not write/verify VRP
#
# --- VRP is write or write/verify. Send through input processes.
#
#
.vex60:
#
                                        #  1st segment
#        ldl     vd_sprc(g12),r6         # Get sample period request and
                                        #  sector counts
# --- VRP is not write or write/verify
#
.vex100:
        movl    r14,g0                  # g0 = VRP function code
                                        # g1 = VRP address
#       g0 = VRP function code
#       g1 = VRP address
#       g4  = VRP function/strategy
#       g9  = primary SN address
#       g10 = I/O length
#       g12 = VDD address
#       g13 = VRP SGL
#       g14 = primary ILT/VRP address
c       v_exec_2(g0,g1,g4,g9,g10,g12,g13,g14);  # Go Process VRP phase 2
        b       .vex20                  # and go check for more VRPs
#
# --- Error returns ---------------------------------------------------
#
# --- Set inoperative virtual device
#
#
# --- Set invalid SDA + length
#
.vex920:
        ldconst ecinvda,r5              # Set invalid SDA+length
        b       .vex1000
#
# --- Set invalid length
#
.vex930:
        ldconst ecinvlen,r5             # Set invalid length
        b       .vex1000
#
# --- Set invalid SDA
#
.vex940:
        ldconst ecinvsda,r5             # Set invalid SDA
        b       .vex1000
#
# --- Set null S/G list
#
.vex950:
        ldconst ecnulsgl,r5             # Set null s/g list
        b       .vex1000
#
# --- Set invalid virtual ID
#
.vex970:
        ldconst ecinvvid,r5             # Set invalid VID
        b       .vex1000
#
# --- Set invalid function
#
.vex980:
        ldconst ecinvfunc,r5            # Set invalid function
        b       .vex1000
#
# --- Set nonx device
#
.vex990:
        ldconst ecnonxdev,r5            # Set non-existent device
#
# --- Complete request
#
.vex1000:
        stob    r5,vr_status(r15)       # Save error code
        mov     g14,g1
c       record_virtual(FR_VRP_COMPLETE, (void *)r15);
#       g1 = ILT
        call    K$comp                  # Complete this request
        b       .vex20
#
#**********************************************************************
#
#  NAME: v$vscomp
#
#  PURPOSE:
#
#       To provide a means of handling the completion of a single RRP
#       request issued in response to a VRP request.
#
#  DESCRIPTION:
#
#       The queue depth within the VDI is updated, the outstanding
#       request is updated and the ILT is completed.
#
#  CALLING SEQUENCE:
#
#       call    v$vscomp
#
#  INPUT:
#
#       g1 = ILT
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#**********************************************************************
#
v$vscomp:
#
# --- Update queue depth in VDD and outstanding request count
#
        ld      il_w3(g1),r15           # Get VDD
        ld      V_orc,r3                # Adjust outstanding request count
        ld      vd_qd(r15),r4           # Adjust queue depth
        subo    1,r3,r3
        subo    1,r4,r4
        st      r3,V_orc
        st      r4,vd_qd(r15)
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
c       CM_ResetInstantMirrorFlags((VDD*)r15, (RRP*)((((ILT*)g1)-1)->ilt_normal.w0));
#
.if     DEBUG_FLIGHTREC_V
        ld      il_w0-ILTBIAS(g1),r5    # Get corresponding RRP
        ldconst frt_v_vscomp,r4         # Type
        ldob    rr_status(r5),r6        # Get Status
        shlo    24,r6,r6                # Shift to show status in FR
        or      r6,r4,r4                # RRP Status and FR Code
        st      r4,fr_parm0             # Virtual - v$vscomp
        st      g1,fr_parm1             # ILT
        st      r5,fr_parm2             # RRP
        st      r15,fr_parm3            # VDD
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_V
#
# --- Exit
#
c       record_virtual_ilt(FR_VRP_SCOMPLETE, (ILT *)g1);
        b       K$comp                  # Complete request
#
#**********************************************************************
#
#  NAME: v$vmcomp
#
#  PURPOSE:
#
#       To provide a means of handling the completion of RRP requests
#       issued in response to a VRP request.
#
#  DESCRIPTION:
#
#       The completing RRP has its SGL released whenever it hasn't
#       been borrowed from the primary VRP.  The composite status is
#       updated within the primary ILT, the outstanding RRP count is
#       decremented and the ILT/RRP is released.  If the outstanding
#       RRP count has gone to zero, the primary VRP status is updated
#       and that request is completed.
#
#  CALLING SEQUENCE:
#
#       call    v$vmcomp
#
#  INPUT:
#
#       g1 = ILT
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#**********************************************************************
#
v$vmcomp:
        ldconst 0,r9
        ldconst 0,r8
        ld      il_w0(g1),r3            # Get RRP
        movt    g0,r12                  # Save g0-g2
        ldob    rr_status(r3),r4        # Obtain status
        ld      il_w3(g1),r11           # Get primary ILT
c       CM_ResetInstantMirrorFlags((VDD*)(((ILT*)g1)->ilt_normal.w4), (RRP*)r3);
        cmpobe  ecok,r4,.vc10           # Jif status OK
#
        ldob    il_w5(r11),r15          # Get current composite status
        ldconst ecbebusy,r5             # Determine if composite is BUSY
        cmpobe  r5,r15,.vc10            # Jif already set to BUSY
#
        cmpobne r5,r4,.vc02             # Jif status not BUSY
        stob    r4,il_w5(r11)           # Save composite stat in primary ILT
        b       .vc10
#
.vc02:
        ldconst ecretryrc,r5            # Determine if composite is RETRYRC
        cmpobe  r5,r15,.vc10            # Jif already set to RETRYRC
        cmpobne r5,r4,.vc03             # Jif status not RETRYRC
        stob    r4,il_w5(r11)           # Save composite stat in primary ILT
        b       .vc10
#
.vc03:
        ldconst ecretry,r5              # Determine if composite is RETRY
        cmpobe  r5,r15,.vc04            # Jif already set to RETRY

.if 1 #VIJAY_MC
        ldconst ecspecial,r5
        cmpobe  r5,r15,.vc04            # Jif already set to RETRY
.endif  # 1
        stob    r4,il_w5(r11)           # Save composite stat in primary ILT
        cmpobne.f 0,r15,.vc10           # Jif error already reported in
                                        #  primary ILT
#
# --- Error occurred on a VRP that potentially is associated with a
#       source copy device of a copy operation. If the VRP is a write
#       type VRP, check if the VDD is the source copy device for any
#       associated copy operations that are active and if so send a
#       ILT/PCP to the copy manager task indicating the error occurred
#       so that the copy operation can be placed in the auto-suspended
#       state and the segments marked as being out of sync.
#
.vc04:
        ld      il_w0-ILTBIAS(r11),r4   # r4 = primary VRP
        ldos    vr_func(r4),r5          # r5 = VRP function code
        ldconst rrbase,r6               # r6 = base RRP function code
        cmpobl.f r5,r6,.vc05c           # Jif function code is VRP based
        subo    r6,r5,r5                # subtract RRP function code base
                                        #  from vr_func value in r5
#
# --- Check if function code is a write or write & verify
#
.vc05c:
        ld      il_w4(r11),r8           # r8 = assoc. VDD field in ILT/VRP
c       r7 = GR_IsAutoSwapInProgress((VDD *)r8);
#c     fprintf(stderr,"<VmComp>..ret = %x from  GR_IsAutoSwapInProgress vid=%x\n",(UINT32)r7,(UINT32)(((VDD*)r8)->vid));
        cmpobe TRUE,r7,.vc06            # Jif autoswap in progress..return with retry
c       r9 = GR_IsCandidateVdisk((VDD*)r8, (VRP*)r4);
#c     fprintf(stderr,"<VmComp>..ret = %x from  GR_IsCandidateVdisk vid=%x\n",(UINT32)r9,(UINT32)(((VDD*)r8)->vid));

       cmpobe FALSE,r9,.vc05d           # Jif not GeoRaid eligible candidate (normal logic)

# c     fprintf(stderr,"<VmComp>..calling GR_IsValidOpStateForAutoSwap vid=%u\n",(UINT32)(((VDD*)r8)->vid));
c       r9 = GR_IsValidOpStateForAutoSwap((VDD*)r8);
#c     fprintf(stderr,"<VmComp>ret = %x from GR_IsValidOpStateForAutoSwap vid=%x\n",(UINT32)r9,(UINT32)(((VDD*)r8)->vid));
        cmpobe FALSE,r9,.vc10           # Jif vdisk is not in INOP state
#
# --- Submit error to Geo Raid error handling component.
#
        mov    g1,r7
c       GR_SubmitVdiskError((VDD*)r8, (VRP*)r4);
        mov    r7,g1

.vc06:
#
# --- Autoswap already in progress (or) GeoRaid failover is kicked-in.
# --- Set return value as ECRETRY,and don't perform autopause of any mirros.
#
        ldconst ecretry,r9
        stob    r9,il_w5(r11)           # Save composite stat in primary ILT
        b .vc10

.vc05d:
        cmpobe.t vroutput,r5,.vc05e     # Jif write function code
        cmpobne.t vroutputv,r5,.vc10    # Jif not write & verify function code
#
# --- Operation is a write type function
#
.vc05e:
#       g1 = ILT/RRP with error
        call    CM$srcerr               # process error for source copy device
                                        # g0 = ILT/PCP count of copy
                                        #      operations needing to process
                                        #      the error before completing
                                        #      this I/O
        cmpobne.t 0,g0,.vc100_v         # Jif ILT/PCPs generated to process
                                        #  the error on a source copy device
.vc10:
.if     DEBUG_FLIGHTREC_V
        ldconst frt_v_vmcomp,r15        # Type
        shlo    24,r4,r4                # Show the status in the FR entry
        or      r4,r15,r15              # RRP Status and FR Code
        st      r15,fr_parm0            # Virtual - v$vmcomp
        st      r11,fr_parm1            # Primary ILT
        ld      il_w0-ILTBIAS(r11),r4
        st      r4,fr_parm2             # VRP
        st      r3,fr_parm3             # RRP
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_V
#
# --- Release ILT/RRP/SGL if necessary
#
        cmpobe  g1,r11,.vc30_v          # Jif completing ILT = primary ILT
#       g1 = ILT
        call    M$rir                   # Release ILT/RRP/SGL
#
# --- Check pending I/O count within primary ILT
#
.vc30_v:
        ld      il_w1(r11),r3           # Adjust pending I/O count
        subo    1,r3,r3
        st      r3,il_w1(r11)
#
        ld      il_w0-ILTBIAS(r11),r4   # Get primary VRP
        cmpobne 0,r3,.vc100_v           # Jif additional pending RRPs
#
# --- Check if CWIP record needs to be deallocated
#
        ld      il_w6(r11),g0           # g0 = CWIP record address
        cmpobe.t 0,g0,.vc35             # Jif no CWIP record used
        mov     0,r4
#       g0 = CWIP record address
        call    CCSM$put_cwip           # Release CWIP record
        st      r4,il_w6(r11)           # Clear CWIP record from ILT
#
# --- Check if primary ILT is queued to associated VDD and if so
#       remove it from the queue.
#
#       r11 = primary ILT
#
.vc35:
        ld      il_w7(r11),r4           # r4 = assoc. VDD field in ILT/VRP
        cmpobe.t 0,r4,.vc40_v           # Jif no assoc. VDD defined indicating
                                        #  the ILT/VRP is not queued to the
                                        #  assoc. VDD
        ldconst 0,r3                    # r3 = 0
        ld      il_fthd(r11),r5         # r5 = forward pointer
        ld      il_bthd(r11),r6         # r6 = backward pointer
        lda     vd_outhead-il_fthd(r4),r7 # r7 = address of head of list
        st      r3,il_w7(r11)           # clear assoc. VDD field in ILT
        st      r5,il_fthd(r6)          # unlink ILT/VRP from list
        cmpobne.f 0,r5,.vc37            # Jif not at the tail of the list
        cmpo    r6,r7                   # check if request was at the head
                                        #  of the list
        lda     vd_outtail-il_bthd(r4),r5 # r5 = list tail pointer address
        sele    r6,0,r6                 # clear tail pointer if true
.vc37:
        st      r6,il_bthd(r5)          # set backward pointer to next ILT
        st      r3,il_bthd(r11)         # clear backward pointer in ILT
        st      r3,il_fthd(r11)         # clear forward pointer in ILT
#
# --- Complete primary ILT
#
#       r11 = primary ILT
#       r12 = caller's g0
#       r13 = caller's g1
#       r14 = caller's g2
.vc40_v:
        ld      il_w0-ILTBIAS(r11),r4   # Get primary VRP
        mov     r11,g1                  # Pass primary ILT
        ldob    il_w5(r11),r5           # Get composite status
        ld      il_w4(r11),r10          # Get VDD
        stob    r5,vr_status(r4)        # Update primary VRP status
c       record_virtual(FR_VRP_COMPLETE, (void *)r4);
#       g1 = ILT
        call    K$comp                  # Complete primary ILT
#
# --- Adjust outstanding request count & update queue depth in VDD
#
        ld      V_orc,r3                # Adjust outstanding request count
        ld      vd_qd(r10),r4           # Adjust queue depth
        subo    1,r3,r3
        subo    1,r4,r4
        st      r3,V_orc
        st      r4,vd_qd(r10)
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
#
# --- Exit
#
.vc100_v:
        movt    r12,g0                  # Restore g0-g2
        ret
#
#**********************************************************************
#
#  NAME: v$stats
#
#  PURPOSE:
#
#       To provide a means of computing virtual device statistics for
#       this module.
#
#  DESCRIPTION:
#
#       For each defined virtual device the VDD is updated to provide the
#       average sector count and request per second statistics.  This
#       logic is performed within a loop that contains a 1 second
#       timeout.
#
#  CALLING SEQUENCE:
#
#       process call
#
#  INPUT:
#
#       None.
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#**********************************************************************
#
v$stats:
        movl    0,g12                   # Preload zero constants
#
# --- Delay for 1 second
#
.sa10_v:
        ldconst 1000,g0                 # Delay for 1000 ms
        call    K$twait
        ldob    VPri_enable,r3
        cmpobe 0,r3,.sa11_v             # Jif VPri disabled
        mov     0,r3
        st      r3,v_hpmb               # Clear out total sect count
.sa11_v:
#
        lda     V_vddindx,r14           # Get VDD index
        ldconst MAXVIRTUALS,r15         # Get maximum devices
#
# --- Examine next VDD
#
.sa20_v:
        ld      (r14),r13               # Get next VDD
        cmpobe  0,r13,.sa40_v           # Jif none
#
# --- Update requests per second
#
        ldl     vd_sprc(r13),r4         # Get sample period request and
                                        #  sector counts
        stl     g12,vd_sprc(r13)        # Clear sample counts for next
                                        #  iteration
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        ldob    VPri_enable,r7
        cmpobe 0,r7,.sa21_v             # Jif VPri disabled

        ldob    vd_strategy(r13),r7
        ldconst vdhigh,r6
        cmpobne r6,r7,.sa21_v
        ld      v_hpmb,g4               # Get tot sect count for high pri vdisks
        addo    g4,r5,g4
        st      g4,v_hpmb               # Update total count
.sa21_v:
        cmpobe  0,r4,.sa30_v            # Jif no activity
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
.sa30_v:
        stl     r4,vd_rps(r13)          # Update average RPS and sector
                                        #  count
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
#
# --- Accumulate the I/O per sec and MB per sec
#
c       DEF_VdiskLastHrStats((VDD*)r13);
#
# --- Advance to next VDD
#
.sa40_v:
        subo    1,r15,r15               # Adjust remaining VDD count
        lda     4(r14),r14              # Advance index to next VDD ptr
        cmpobl  0,r15,.sa20_v           # Jif more to go
        ldob    VPri_enable,r4
        cmpobe 0,r4,.sa41_v             # Jif VPri disabled
# --- Use the High priority sector count for this second to set the
#     skip counter for v$exec
#
        ld      v_hpmb,r4               # Get the total sect count
        ldconst 0x3f,r5
        shro    11,r4,r4                # Convert to MB/sec
        and     r5,r4,r4                # Truncate to 63 MB
        divo    10,r4,r4                # r4 should be 0-6 now
        ldob    v_skptblmb(r4),r5
        stob    r5,V_skipthrsh+1
.sa41_v:
#
        b       .sa10_v
#
#**********************************************************************
#
#  NAME: V$updFEStatus
#
#  PURPOSE:
#
#       This task will update the FE with the Mirror/Copy status
#
#  DESCRIPTION:
#       This task is a separate process that will update the FE with the latest
#       Copy/Mirror Status.
#
#       It must be in a separate task because it can be called from
#       Completion routines and may result in a FE C$Stop situation
#       that would lock up the two processors waiting on each other.
#
#  CALLING SEQUENCE:
#       Process call (temporary task startup)
#
#  INPUT:
#       g2 = Source VID
#       g3 = Destination VID
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
V_updFEStatus:  # for VIJAY instant mirror
V$updFEStatus:
        mov     g2,r14                  # r14 = Source VID
        mov     g3,r15                  # r15 = Destination VID
        ldconst FALSE,r11               # Flag to show to update servers
        ldconst FALSE,g1                # g1 = Do not delete (Update)
        ldconst 0xffff,r4               # set up mask and compare
        and     r4,r14,r5               # mask off possible upper bits
        cmpobe  r4,r5,.vupfes20         # Jif undefined

        ld      V_vddindx[r14*4],r3     # Ensure Source VID still exists
        mov     r14,g0                  # g0 = Source VID
        cmpobe  0,r3,.vupfes20          # Jif the Source VID has disappeared
#       g0 - VID to be updated.
#       g1 - Boolean delete flag (FALSE = Do not delete [Update])
        call    D$updrmtcachesingle     # Update the FE Status
        ldconst TRUE,r11                # Show Servers need updated

.vupfes20:
        and     r4,r15,r5               # mask off possible upper bits
        cmpobe  r4,r5,.vupfes40         # Jif undefined

        ld      V_vddindx[r15*4],r4     # Ensure Destination VID still exists
        mov     r15,g0                  # g0 = Destination VID
        cmpobe  0,r4,.vupfes40          # Jif the Destination VID is gone
#       g0 - VID to be updated.
#       g1 - Boolean delete flag (FALSE = Do not delete [Update])
        call    D$updrmtcachesingle     # Source VID
        ldconst TRUE,r11                # Show Servers need updated

.vupfes40:
        cmpobe  FALSE,r11,.vupfes100    # Jif do not need to update servers
#       No inputs.
        call    D$signalserverupdate    # Update servers too (signal update)

.vupfes100:
        ret
#
#**********************************************************************
#
#  This stub routine is used to call an assembly completion routine from C
#  This routine is only called from apool.c.
#
#  Input:
#       g0 = completion routine to execute.
#       g1 = ILT to pass to completion routine.
#
call_comp_routine:
#c fprintf(stderr, "Got to call_comp_routine\n");
#       g1 = ILT.
        callx   (g0)
        ret

#**********************************************************************
#
#  This stub routine is used to allow C to callx a copy hander routine.
#  This routine is only called from virtual.c.
#
#  Input:
#   g2 = routine to callx.
# --- Interface to copy operation phase 2 update source handler routines.
#   g0 = FALSE
#   g1 = 0
#   g3 = assoc. SCD/DCD address
#   g4 = VRP function/strategy
#   g9 = primary SN address
#   g10 = I/O length
#   g11 = VRP
#   g12 = assoc. VDD address
#   g13 = assoc. VRP SGL address
#   g14 = primary ILT/VRP address
#
v_callx:
        callx   (g2)
        ret

#**********************************************************************
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
