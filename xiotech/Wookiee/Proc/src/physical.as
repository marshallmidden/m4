# $Id: physical.as 161678 2013-09-18 19:25:16Z marshall_midden $
#**********************************************************************
#
#  NAME: physical.as
#
#  PURPOSE:
#       To provide a means of reentrantly controlling concurrent
#       physical device operations.  This version supports a Fibre Channel
#       interface via the QLogic ISP2300 or 2400 controller.
#
#  FUNCTIONS:
#       P$init          - Physical initialization
#       P$que           - Queue physical I/O request
#       P$fail_dev_cmds - Fail all enqueud commands with ecnonxdev status.
#       P$balanceLoad   - Balances the load among the ports.
#       P$purgeDevices  - Removes device with no path to specified port.
#       P$notifyOnline  - Build temp PDD list and send to Online for processing.
#
#  This module employs up to 7 processes:
#       p$exec     - Executive (1 copy)
#       p$init     - Logical initiator (1 copy per available port, 4 max.)
#       p$comp_ilt - Complete  (1 copy)
#       p$escalate - Escalate  (1 copy)
#
#  Copyright (c) 1996-2010 Xiotech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- local equates ---------------------------------------------------
#
        .set    P_COMP_MAX_TIME,500/QUANTUM # Allow p$comp_ilt .5 sec of running
        .set    P_EXEC_MAX_TIME,500/QUANTUM # Allow p$exec .5 sec of running

#
        .set    HEADOFQ,2               # instead of 0x20,21,22 in classic
        .set    ORDERED,4               # Ordered queue tag message
        .set    SIMPLE,8                # Simple queue tag message
#
# --- assembly options ------------------------------------------------
#
        .set    ENABLE_PHYSICAL_IO_OPT,TRUE    # This turns off IO merging/joining/canceling
                                               # eleveator queueing that attempts to predict head movement
#
# --- global function declarations ------------------------------------
#
        .globl  P$init                  # Physical initialization
        .globl  P$que                   # Queue physical I/O request
        .globl  P$fail_dev_cmds         # Fail all DEV cmds as ecnonxdev
        .globl  P$rescanDevice
        .globl  p$wakeup
        .globl  P$DegradePerf
#
# --- kernel resident routines
#
        .globl  K$fork                  # Process fork
        .globl  K$xchang                # Process exchange
        .globl  K$qxchang               # Process quick exchange
        .globl  K$twait                 # Process timed wait
#
        .globl  K$cque                  # Common queuing routine
        .globl  K$comp                  # Complete request
#
# --- misc resident routines
#
        .globl  M$rip                   # Release ILT/PRP combination
#
# --- global data declarations ----------------------------------------
#
        .globl  P_chn_ind
        .globl  P_orc
#
# --- local usage data definitions ------------------------------------
#
        .globl  P_drvinits
        .data
P_drvinits:
        .word   0

        .globl  P_dvlist
P_dvlist:                               # DEV structs for targets
        .word   0
#
# --- Counter and constants for Write & Verify performance tuning
#     MSHDEBUG
.set    MAX_WV_COUNT,10
.set    WV_THRESH,2                    # 2=20% Write & Verify (0=0%, 10=100%)
p_wv_count:
        .word   0
#
p_waitTable:
        .byte   0
        .byte   1500/QUANTUM
        .byte   1000/QUANTUM
        .byte   750/QUANTUM
        .byte   625/QUANTUM
        .byte   500/QUANTUM
#
# --- Template for SCSI Inquiry - (standard inquiry data)
#     Standard inquiry command with 56 byte return of standard inquiry data.
#
P_t_inquiry:
        .word   BTIMEOUT                # pr_timeout
        .word   56                      # pr_rqbytes
        .byte   prinput                 # pr_func
        .byte   6                       # pr_cbytes
        .byte   prSLIb+prSNXb+prBLPb+prBNOb # pr_flags
.ifndef MODEL_3000
.ifndef  MODEL_7400
        .byte   IORETRY                 # pr_retry
.endif  # MODEL_7400
.endif  # MODEL_3000
.ifndef  MODEL_7000
.ifndef  MODEL_4700
        .byte   1                       # pr_retry
.endif  # MODEL_4700
.endif  # MODEL_7000
        .byte   0x12                    # pr_cmd (inquiry)
        .byte   0
        .byte   0
        .byte   0
        .byte   56
        .byte   0
        .space  10,0                    # pad to 16 bytes
#
# --- Template for SCSI Test Unit Ready
#
P_t_testurdy:
        .word   BTIMEOUT                # pr_timeout
        .word   0                       # pr_rqbytes
        .byte   prctl                   # pr_func
        .byte   6                       # pr_cbytes
        .byte   prSPSb+prSNXb+prBLPb+prBNOb # pr_flags
.ifndef MODEL_3000
.ifndef  MODEL_7400
        .byte   IORETRY                 # pr_retry
.endif  # MODEL_7400
.endif  # MODEL_3000
.ifndef  MODEL_7000
.ifndef  MODEL_4700
        .byte   1                       # pr_retry
.endif  # MODEL_4700
.endif  # MODEL_7000
        .byte   0x00                    # pr_cmd
        .byte   0
        .byte   0
        .byte   0
        .byte   0
        .byte   0
        .space  10,0                    # pad to 16 bytes
#
# --- Template for SCSI Start/Stop Unit - (start unit)
#
P_t_startunit:
.ifndef MODEL_3000
.ifndef  MODEL_7400
        .word   BTIMEOUT                # pr_timeout
.endif  # MODEL_7400
.endif  # MODEL_3000
.ifndef  MODEL_7000
.ifndef  MODEL_4700
        .word   5                       # pr_timeout
.endif  # MODEL_4700
.endif  # MODEL_7000
        .word   0                       # pr_rqbytes
        .byte   prctl                   # pr_func
        .byte   6                       # pr_cbytes
        .byte   prSLIb+prSNXb+prBCCb+prBLPb+prBNOb  # pr_flags
.ifndef MODEL_3000
.ifndef  MODEL_7400
        .byte   IORETRY                 # pr_retry
.endif  # MODEL_7400
.endif  # MODEL_3000
.ifndef  MODEL_7000
.ifndef  MODEL_4700
        .byte   1                       # pr_retry
.endif  # MODEL_4700
.endif  # MODEL_7000
        .byte   0x1b                    # pr_cmd
        .byte   0x01                    # immediate bit
        .byte   0
        .byte   0
        .byte   0x01                    # Start bit
        .byte   0
        .space  10,0                    # pad to 16 bytes
#
# --- Template for SCSI Report Luns
#
        .globl  P_t_reportluns
P_t_reportluns:
.ifndef MODEL_3000
.ifndef  MODEL_7400
        .word   BTIMEOUT                # pr_timeout
.endif  # MODEL_7400
.endif  # MODEL_3000
.ifndef  MODEL_7000
.ifndef  MODEL_4700
        .word   5                       # pr_timeout
.endif  # MODEL_4700
.endif  # MODEL_7000
p_t_rqbytes:
        .word   0x10                    # pr_rqbytes
        .byte   prinput                 # pr_func
        .byte   12                      # pr_cbytes
        .byte   prSLIb+prSNXb+prBLPb+prBNOb # pr_flags
.ifndef MODEL_3000
.ifndef  MODEL_7400
        .byte   IORETRY                 # pr_retry
.endif  # MODEL_7400
.endif  # MODEL_3000
.ifndef  MODEL_7000
.ifndef  MODEL_4700
        .byte   1                       # pr_retry
.endif  # MODEL_4700
.endif  # MODEL_7000
        .byte   0xA0                    # pr_cmd
        .byte   0
        .byte   0
        .byte   0
        .byte   0
        .byte   0
p_t_alloc:
        .byte   0                       # Allocation Length MSB
        .byte   0
        .byte   0
        .byte   0x10                    # Allocation Length LSB
        .byte   0
        .byte   0                       # control byte
        .space  4,0                     # pad to 16 bytes

.set    rl_rqbytes, p_t_rqbytes-P_t_reportluns
.set    rl_alloc,   p_t_alloc-P_t_reportluns
#
# --- executable code ------------------------------------------------
        .text
#
#**********************************************************************
#
#  NAME: P$init
#
#  PURPOSE:
#       To provide a means of initializing the physical I/O module at
#       system startup.
#
#  DESCRIPTION:
#       This routine initializes the channel chip hardware and
#       the associated software table structures.  Processes for
#       handling I/O operations are also established.
#
#  CALLING SEQUENCE:
#       call    P$init
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g0 - g3
#       g4
#       g10
#       g11 = origin of port index
#       g12 = port number
#
#**********************************************************************
#
P$init:
#
# --- Initialize IRAM constants
#
        lda     P$que,r3                # Initialize P_que
        st      r3,P_que
#
# --- Clear pointer to POR PDD list
#
        ldconst 0,g0
        st      g0,O_p_pdd_list         # NULL the pointer
        ldob    ispmap,r3               # Get port bitmap
        lda     P_chn_ind,g11           # Get origin of port index
#
# --- Wakeup DEFINE/ONLINE layer if no ports are present
#
        cmpobne 0,r3,.i20               # JIf ports are present
        lda     K_ii,r3                 # Get ptr to Internal Information
        ldos    ii_status(r3),r4        # Get initialization status
        setbit  iiphy,r4,r4             # Set Physical Ready bit
        stos    r4,ii_status(r3)        # Save it
        b       .i30
#
# --- Loop through and setup tables for existing port
#
.i20:
        scanbit r3,g12                  # Locate next port
        bno.f   .i30                    # Jif none
        clrbit  g12,r3,r3               # Clear bitmap for this port
        call    p$bld_tabs              # Build this port's tables and
                                        #  spawn a process (initializes chip)
        b       .i20                    # Loopback to handle next port
#
# --- Establish one Physical layer executive process for all ports
#
.i30:
        lda     p$exec,g0               # Establish executive process
        ldconst PEXECPRI,g1
c       CT_fork_tmp = (ulong)"p$exec";
        call    K$fork
        st      g0,P_exec_qu+qu_pcb     # Save PCB
#
# --- Initialize Debug Data Retrieval (DDR) P_exec_qu PCB entry
#
c       M_addDDRentry(de_ppcb, g0, pcbsiz);
#
# --- Initialize Debug Data Retrieval (DDR) P_exec_qu entry
#
        lda     P_exec_qu,g1            # Load address of P_exec_qu header
c       M_addDDRentry(de_peque, g1, 16);    # Size of P_exec_qu header
#
# --- Establish one Physical layer completion process for all ports
#
        lda     p$comp_ilt,g0           # Establish completion process
        ldconst PCOMPPRI,g1
c       CT_fork_tmp = (ulong)"p$comp_ilt";
        call    K$fork
        st      g0,P_comp_qu+qu_pcb     # Save PCB

#       Create snapshot completer tasks.
c       create_snapshot_completers();

#
# --- Initialize Debug Data Retrieval (DDR) P_comp_qu & P_comp_pcb entries
#
        lda     P_comp_qu,g1            # Load address of P_comp_qu header
c       M_addDDRentry(de_pcque, g1, 16);    # Size of P_comp_qu header
#
        ld      qu_pcb(g1),g1           # Load address of P_comp_pcb
c       M_addDDRentry(de_pcpcb, g1, pcbsiz);
.ifndef MODEL_3000
.ifndef  MODEL_7400
#
# --- Establish One Path monitor
#
        lda     FAB_PathMonitor,g0
        ldconst PESCPRI,g1
c       CT_fork_tmp = (ulong)"FAB_PathMonitor";
        call    K$fork
.endif  # MODEL_7400
.endif  # MODEL_3000
#
# --- Establish one escalation process for all ports
#
        lda     PHY_escalate,g0            # Establish escalation process
        ldconst PESCPRI,g1
c       CT_fork_tmp = (ulong)"PHY_escalate";
        b       K$fork
# End of P$init ***************************************************************
#
#**********************************************************************
#
#  NAME: p$bld_tabs
#
#  PURPOSE:
#       To provide a means of dynamically assigning and constructing
#       the low-level table structures required by the PHYSICAL layer
#       to work with ports and devices.
#
#  DESCRIPTION:
#       This routine dynamically constructs the CHN table and associated
#       DEV tables for a single port.  The executive process for that port
#       is also constructed.
#
#  CALLING SEQUENCE:
#       call    p$bld_tabs
#
#  INPUT:
#       g11 = origin of port index
#       g12 = port number
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g0
#       g1
#       g4
#
#**********************************************************************
#
p$bld_tabs:
#
# --- Allocate CHN structure for this port
#
c       r4 = s_MallocC(chnsiz|BIT31, __FILE__, __LINE__); # Allocate/clear CHN
        st      r4,(g11)[g12*4]         # Link CHN to port index
#
# --- Create logical (Physical layer) initiator processes for each port chip
#
        lda     p$init,g0               # Establish initiator process (logical)
        ldconst PINITPRI,g1             # Set priority of initiator process
        mov     r4,g4                   # g4 = CHN
c       CT_fork_tmp = (ulong)"p$init";
        call    K$fork
        st      g0,ch_init_pcb(r4)      # Save PCB
#
# --- Complete initialization of structure for this port
#
        stob    g12,ch_channel(r4)      # Set up port number
        ret
#
# End of p$bld_tabs ***********************************************************
#
#******************************************************************************
#
#  NAME: P$fail_dev_cmds
#
#  PURPOSE:
#       Complete/fail all enqueued DEV commands with ecnonxdev PRP status.
#
#  DESCRIPTION:
#       Put all the ILTs from the DEV linked list and failed queue into the
#       completion queue. Reset the retry count for all ILTs for the failing
#       DEV.
#
#
#  CALLING SEQUENCE:
#       call    P$fail_dev_cmds
#
#  INPUT:
#       g0 = DEV pointer
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
P$fail_dev_cmds:
        ldconst ecnonxdev,r8            # Preset nonexistent device error code
#
        movt    g0,r12                  # Preserve regs
        mov     g14,r15
#
        ld      dv_pdd(r12),r9          # get pdd
        cmpobe  0,r9,.pfdc10            # pdd is null
        ldob    pd_flags(r9),r3         # get flags
        bbc     pdbebusy,r3,.pfdc10     # is the pdd set to busy
        ldconst ecbebusy,r8             # Preset be busy error code
.pfdc10:
        ldconst 0,r9                    # Preload with zero
#
# --- Process primary device queue
#
        ld      dv_qcnt(r12),r3         # Get queue count
        cmpobe  0,r3,.p_fail50          # Jif no commands on device queue
#
        lda     dv_hdatoken(r12),g0     # Get HDA Token ILT
        ld      dv_iltq_fhead(r12),g1   # Get queue head ILT
        cmpobe  0,g1,.p_fail50          # Jif no cmds on device queue (overkill)
#
        bal     p$failcmds              # fail cmds on device queue
#
        lda     dv_hdatoken(r12),g0     # g0 = hdatoken address
        st      r9,il_fthd(g0)          # Clear hdatoken forward pointer
        st      r12,il_bthd(g0)         # Set back link of token to point to DEV
        st      g0,dv_iltq_fhead(r12)   # Set iltq head pointer
        st      g0,dv_iltq_tail(r12)    # Set iltq tail pointer
        st      r9,dv_qcnt(r12)         # Clear ILT queue count
.p_fail50:
#
# --- Process fail/retry queue
#
        ldconst 0,g0                    # No HDA token
        ld      dv_failq_hd(r12),g1     # Get Fail/retry queue head
        cmpobe  0,g1,.p_fail100         # Jif no cmds on failed queue
#
        bal     p$failcmds              # fail cmds on failed queue
#
        st      r9,dv_failq_hd(r12)     # Clear failed queue
        st      r9,dv_failq_tl(r12)
.p_fail100:
#
# --- Process completion queue.
#
        lda     P_comp_qu,r3            # Get completion queue pointer
        ld      qu_head(r3),r4          # Get queue head
        cmpobe  0,r4,.p_faildone        # Jif none
#
# --- Zero the retry count in the PRP if the DEVice pointer
#     is equal to the specified device.
#
.p_fail110:
        ld      p_qhd(r4),r6            # Get corresponding DEV
        cmpobne r12,r6,.p_fail120       # Jif not specified device
        ld      r_prp-ILTBIAS(r4),r10   # Get PRP from queued ILT
        stob    r8,pr_rstatus(r10)      # Set error code in PRP request status
        stob    r9,pr_retry(r10)        # Set retry count to 0
#
# --- Get next ILT in queue and loop if not empty.
#
.p_fail120:

        ld      il_fthd(r4),r4          # Get next ILT
        cmpobne 0,r4,.p_fail110         # Jif NOT end of queue

.p_faildone:
        movt    r12,g0                  # Restore regs
        mov     r15,g14
        ret
#
#******************************************************************************
#
#  NAME: p$failcmds
#
#  PURPOSE: Given a head ILT pointer, fail all commands in that chain.
#
#  CALLING SEQUENCE:
#       bal P$failcmds
#
#  INPUT:
#       g0  = HDA token
#       g1  = head of ILT chain
#       g14 = return link
#       r8  = error code
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       r9, r10, and r11.
#
#******************************************************************************
#
p$failcmds:
        mov     g0,r6                   # Save HDA token
        ldconst 0,r9                    # Clear
#
# --- On each ILT in the linked list, set nonexistent device error code
#     in the PRP, clear the device pointer in the ILT, and call the
#     completion routine for the ILT.
#
.pfc10:
        ld      il_fthd(g1),r11         # Get next ILT
        cmpobe  r6,g1,.pfc20            # Jif this is the HDA token
        ld      r_prp-ILTBIAS(g1),r10   # Get PRP from queued ILT
        stob    r8,pr_rstatus(r10)      # Set error code in PRP request status
        stob    r9,pr_retry(r10)        # Set retry count to 0
        mov     g1,g7                   # Suspend possible mirrored reads
        call    p$susp_rr
        call    p$qcomp_ilt             # Complete this request
.pfc20:
        mov     r11,g1                  # Advance to next ILT
        cmpobne 0,g1,.pfc10             # Jif not end of queue
#
# --- Exit
#
        bx      (g14)
#
# End of p$failcmds ***********************************************************
#
#**********************************************************************
#
#  NAME: P$DegradePerf
#
#  PURPOSE:
#       To provide a common means of designating a particular device
#       as a requiring a detune to degrade performance.
#
#  DESCRIPTION:
#       The dvdegradeperf flag in the DEV structure is set to
#       the desired setting.
#
#  CALLING SEQUENCE:
#       call    P$degradePerf
#
#  INPUT:
#       g3 = PDD
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
P$DegradePerf:
        ld      pd_dev(g3),r3           # Get DEVice from PDD
        cmpobe  0,r3,.dp100             # Exit if invalid
#
# --- Update DEV
#
        ldob    dv_flags(r3),r4         # Get flags from DEV
        setbit  dvdegradeperf,r4,r4     # Set degrade perf flag
        stob    r4,dv_flags(r3)         # Store flags in DEV
#
# --- Exit
#
.dp100:
        ret
#
#**********************************************************************
#
#  NAME: P$que
#
#  PURPOSE:
#       To provide a common means of queuing physical I/O requests.
#
#  DESCRIPTION:
#       The ILT and associated PRP are queued to the executive.  The
#       executive is then activated to process this request.  This
#       routine may be called from either the process or interrupt level.
#
#  CALLING SEQUENCE:
#       call    P$que
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
#       W0 = PRP
#       W1 = associated ILT (optional for reads only) consists of a
#            circular link when non-zero to other mirrors of the read
#       W2 = SN (optional)
#
#**********************************************************************
#
P$que:
#
# --- Initialize ILT
#
        movt    0,r8                    # Clear join count, thread, flags
        lda     P_exec_qu,r11           # Get executive queue pointer
c       ((ILT*)g1)->phy_io.phy_priority = 0;
c       ((ILT*)g1)->phy_io.phy_overlap = 0;
c       ((ILT*)g1)->phy_io.phy_writeflag = 0;
        st      0,p_jct(g1)             # Join count -- Also p_scb(g1).
        st      0,p_jth(g1)             # Join thread
c       ((ILT*)g1)->phy_io.phy_tag = 0;
        st      r11,p_qhd(g1)           # Queue or DEV head
#
c       record_physical_ilt(FR_PRP_QUEUED_AT_TAIL, (ILT *)g1, 0);
        b       K$cque
#
#**********************************************************************
#
#  NAME: p$exec
#
#  PURPOSE:
#       To provide a means of processing physical I/O requests which
#       have been previously queued to this module.
#
#  DESCRIPTION:
#       The next request previously queued to this module is dequeued
#       and validated.  If an error is found with the request, the
#       appropriate error code is inserted and the request is returned
#       to the caller.
#
#       If the request passes validation, the request is inserted into
#       the device queue based upon ascending disk addresses.
#       Join optimization and write elimination optimization are also
#       performed if possible.
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
#       g3-g11.
#
#  ILT USAGE:
#       CALLEE AREA
#       ___________
#       W0 = Aging control
#       W1 = Byte 0 - SCSI opcode
#       W2 = SDA when non-zero
#       W3 = EDA when non-zero
#       W4 = Join count before initiation
#            SCB after initiation
#       W5 = Join thread when non-zero
#       W6 = Byte 0 - Overlap indicator T/F
#            Byte 1 - Queue tag
#            Byte 2 - Write indicator T/F
#            Byte 3 - Executable indicator T/F
#       W7 = EXEC or DEV queue head before initiation when != 0
#
#**********************************************************************
#
# --- Set this process to not ready
#
.pex10:
        ldconst pcnrdy,r4               # Set this process to not ready
        stob    r4,pc_stat(r15)
#
# --- Exchange processes ----------------------------------------------
#
p$exec:
.pex20:
        call    K$qxchang               # Exchange processes
        ld      K_ii+ii_time,r6         # Get current timer
        addo    P_EXEC_MAX_TIME,r6,r6   # r6 = next timer to do Xchange to
                                        #  prevent other task starvation
#
# --- Get next queued request
#
.pex30:
        lda     P_exec_qu,r11           # Get executive queue pointer
        ldq     qu_head(r11),r12        # Get queue head, tail, count
                                        #  and PCB
        mov     r12,g3                  # Isolate next queued ILT
        cmpobe.f 0,r12,.pex10           # Jif none
#
# --- Remove this request from queue ----------------------------------
#
        ld      il_fthd(g3),r12         # Dequeue this ILT
        subo    1,r14,r14               # Adjust queue count
        cmpo    0,r12                   # Check for queue now empty
        sele    r13,r12,r13             # Set up queue tail
        stt     r12,qu_head(r11)        # Update queue head, tail and count
        be.f    .pex40                  # Jif queue now empty
#
        st      r11,il_bthd(r12)        # Update backward thread
#
.pex40:
        ld      r_prp-ILTBIAS(g3),r4    # Get PRP from ILT

#
# --- Bump outstanding request count
#
        ld      P_orc,r5                # Bump outstanding request count
        addo    1,r5,r5
        st      r5,P_orc
#
# --- Validate DEV exists
#
        ld      pr_dev(r4),g10          # Get DEVice
        ldob    pr_func(r4),r12         # Get function code

.if     DEBUG_FLIGHTREC_P
        ldconst frt_p_exec,r3           # Type
        shlo    24,r12,r14              # Get Function code in FR Code field
        or      r14,r3,r3               # Function Code + FR Code
        st      r3,fr_parm0             # Physical - p$exec
        st      g3,fr_parm1             # ILT
        st      r4,fr_parm2             # PRP
        st      g10,fr_parm3            # DEVice
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_P
#
# --- Validate that the device exists
#
        cmpobe  0,g10,.pex350           # Jif nonexistent device
        ldob    pr_flags(r4),r14        # Get PRP flags
        bbs     prBNO,r14,.pex60        # Jif bypass inoperable check
        ldob    dv_physerr(g10),r14     # Get Physical Device Error Count
        cmpobne 0,r14,.pex310           # Jif errors, return invalid device
.pex60:
#
        ldob    dv_chn(g10),r14         # Get port number
        cmpoble.f MAXCHN,r14,.pex300    # Jif invalid port (ID too large)
        ld      P_chn_ind[r14*4],g11    # Lookup CHN anchor
#
# --- Validate function code
#
        ldob    pr_strategy(r4),r13     # Get strategy
        cmpibl.f proutput,r12,.pex320   # Jif too large
        cmpibg.f prctl,r12,.pex320      # Jif too small
#
# --- Validate strategy
#
        cmpobl.f prhigh,r13,.pex330     # If invalid, jump
#
# --- Validate SGL based on function requested
#
        ld      pr_sglptr(r4),r7        # Get SGL pointer
#        cmpobe.f prctl,r12,.pex100      # Jif control operation
c if (prctl != r12) {
.ifdef BACKEND
# -- If special emulated PAB (time greater than 90 seconds) is happening.
c   if (((DEV*)g10)->pdd != 0) {
c     if (BIT_TEST(((DEV*)g10)->pdd->flags, PD_BEBUSY)) {
c       if (((DEV*)g10)->TimetoFail > 90) { # Special emulated PAB
          b       .pex300               # If PDD is busy, return busy
c       }
c     }
c   }
.endif  # BACKEND
c } else {
        b       .pex100                 # Jif control operation
c }
#
        cmpobe.f 0,r7,.pex340           # Jif SGL not present
#
# --- Initialize ILT parameters ---------------------------------------
#
.pex100:
# Get current priority to account for time already spent in the EXEC queue
c       r8 = ((ILT*)g3)->phy_io.phy_priority;
c       r8 = 1+ r8 + (r13 * 16);
c       if (r8 > MAX_PHY_PRIORITY) {
c           r8 = MAX_PHY_PRIORITY;
c       }
c       ((ILT*)g3)->phy_io.phy_priority = r8;
        ldob    pr_cmd(r4),r9           # Get SCSI opcode byte
        stob    r9,p_cmd(g3)            # Update ILT -- r9 used below
!       ldl     pr_sda(r4),r10          # Get SDA
        stl     r10,p_sda(g3)           # Update ILT
!       ldl     pr_eda(r4),r8           # Get EDA
        stl     r8,p_eda(g3)            # Update ILT
#
c   if (r9 == SCC_WRITEXT || r9 == SCC_WRITE_16) {
        b       .pex105                 # If SCSI write
c   }
c   if (r9 != SCC_WRTVRFY_10 && r9 != SCC_WRTVRFY_16) {
        b       .pex110                 # If not SCSI write/verify
c   }
#
.pex105:
c       ((ILT*)g3)->phy_io.phy_writeflag = TRUE; # Set write flag
#
# --- Added code here to degrade performance for devices that have the
#     degradeperf flag set in the dv_flags. The online code will set this
#     flag for all devices whose performance should be crippled
#     based on their Product ID in the Inquiry Data. This code will then
#     convert Write ops to Write & Verify ops a fixed percentage of the time
#     for these devices.  The counter  p_wv_count is used to count the
#     Write ops up to the constant MAX_WV_COUNT (10).  The constant
#     WV_THRESH determines the percentage of time that the Write ops are
#     converted to Write & Verify ops.  The queue insertion performance
#     optimization code below will also be skipped for these devices.
#
        ldob    dv_flags(g10),r15       # Get device flags
        bbc     dvdegradeperf,r15,.pex110  # Jif not degrading performance
        ld      p_wv_count,r15          # Load degrade performance op counter
        addo    1,r15,r15               # increment count of Write type ops
        cmpobg  MAX_WV_COUNT,r15,.pex106 # Jif not a max count value
        ldconst 0,r15                   # Reset counter to zero
.pex106:
        st      r15,p_wv_count          # Save the counter
#
        cmpoble WV_THRESH,r15,.pex110   # Jif not degraded perf for this op
c   if (r9 == SCC_WRITEXT || r9 == SCC_WRTVRFY_10) {    # 10 byte command
        stob    writeverify,pr_cmd(r4)  # Store SCSI write/verify opcode byte
c   } else {
        stob    SCC_WRTVRFY_16,pr_cmd(r4) # Store SCSI write/verify (16)
c   }
#
# --- Check for optimization required
#
.pex110:
        lda     dv_hdatoken(g10),g12    # g12 = HDA token
        st      g10,p_qhd(g3)           # Update DEV link in ILT
#
        mov     TRUE,r15                # Lock this device
        stob    r15,dv_taglock(g10)
#
.if ENABLE_PHYSICAL_IO_OPT
c   if (0ULL == *((UINT64*)&r8)) {
        b       .pex115                 # Jump if non-optimizable op
c   }
        ldob    dv_blk(g10),r15         # Get block device indicator
        cmpobe.t TRUE,r15,.pex120       # Jump if a block device
#
.pex115:
.endif  # ENABLE_PHYSICAL_IO_OPT
        ld      dv_iltq_tail(g10),r12   # Get current queue tail
        mov     0,r13                   # Append to tail of queue
        b       .pex180
#
# --- Search queue for proper insertion position
#
.if ENABLE_PHYSICAL_IO_OPT
.pex120:
        mov     g10,r13                 # Start search from que origin
.pex130:
        mov     r13,r12                 # Move next to previous ILT
        ld      il_fthd(r13),r13        # Get next forward ILT
        cmpobe.f 0,r13,.pex180          # Jump if end of queue
#
!       ldl      p_sda(r13),r4          # Get next SDA
!       ldl      p_eda(r13),r14         # Get next EDA
c   if (*((UINT64*)&r10) > *((UINT64*)&r14)) {
        b       .pex130                 # If insertion follows
c   }
c   if (*((UINT64*)&r8) < *((UINT64*)&r4)) {
        b       .pex175                 # If insertion probably precedes
c   }
        cmpobe.f g12,r13,.pex130        # Jif HDA token encountered
#
# --- Operations either overlap or are adjacent
# --- Check for join possibility or write cancellation ----------------
#
        ldob    p_cmd(g3),r3            # Get new SCSI command
        ldob    p_cmd(r13),r7           # Get next SCSI command
        cmpobne.f r3,r7,.pex150         # Jif commands don't match
#
# --- Process matching SCSI commands ----------------------------------
# --- Check for adjacent operations
#
c   if (*((UINT64*)&r10) == *((UINT64*)&r14)) {
        b       .pex160                 # If probably backend join
c   }
c   if (*((UINT64*)&r8) == *((UINT64*)&r4)) {
        b       .pex160                 # If probably frontend join
c   }
#
# --- Overlapping operations - same commands
# --- Check for possible write cancellation
#
c   if (r3 == SCC_WRITEXT || r3 == SCC_WRITE_16) {
        b       .pex140                 # If both are SCSI write
c   }
c   if (r3 == SCC_WRTVRFY_10 && r3 == SCC_WRTVRFY_16) {
        b       .pex140                 # If both are SCSI write/verify
c   }
#
# --- Overlapping operations - READ commands
#
c   if (*((UINT64*)&r10) > *((UINT64*)&r4)) {
        b       .pex130                 # If new req has higher SDA
c   }
        b       .pex175
#
# --- Overlapping operations - WRITE commands
# --- Check for write cancellation on dual writes
#
.pex140:
#
# --- Branch to .pex190 if incoming operation does NOT completely
#       overlap queued operation.
#
c   if (*((UINT64*)&r10) > *((UINT64*)&r4)) {
        b       .pex190                 # If new req has higher SDA
c   }
c   if (*((UINT64*)&r8) == *((UINT64*)&r14)) {
        b       .pex190                 # If new req has lower EDA
c   }
#
# --- Incoming operation completely overlaps queued operation
#
        ld      il_fthd(r13),g1         # g1 = remaining queue
        mov     g3,g0                   # g0 = new ILT
        call    p$aftpchk               # Check for precedence
        cmpobe.f 0,g0,.pex190           # Jif so
#
# --- Cancel redundant write if sufficient resources are available
#
        call    p$rchk                  # Check for sufficient resources
        cmpobe.f FALSE,g0,.pex190       # Jif not
#
        mov     r13,g2                  # g2 = next ILT
c       r10 = ((ILT*)g3)->phy_io.phy_overlap;  # r10 = overlap indicator from new op.
        ld      il_bthd(r13),r13        # Establish new next ILT for insert
        call    p$cancel                # Attempt to cancel this request
                                        # g3 = system ILT
c       ((ILT*)g3)->phy_io.phy_overlap = r10;  # r10 = overlap indicator from new op.
!       ldl     p_sda(g3),r10           # Establish new SDA/EDA
!       ldl     p_eda(g3),r8            # Establish new SDA/EDA
        b       .pex130
#
# --- Process non-matching SCSI commands ------------------------------
# --- Operations either overlap or are adjacent
#
.pex150:
c   if (*((UINT64*)&r10) == *((UINT64*)&r14)) {
        b       .pex130                 # If no backend overlap
c   }
c   if (*((UINT64*)&r8) > *((UINT64*)&r4)) {
        b       .pex175                 # If no frontend overlap
c   }
#
c       r7 = ((ILT*)r13)->phy_io.phy_writeflag; # Get write flag
        cmpobe.t FALSE,r7,.pex130       # Jif not write
#
        b       .pex190
#
# --- Process possible join -------------------------------------------
# --- Adjacent operations - commands the same
#
.pex160:
c   if (r3 == SCC_READEXT || r3 == SCC_READ_16) {
        b       .pex170                 # If SCSI read
c   }
c   if (r3 == SCC_WRITEXT || r3 == SCC_WRITE_16) {
        b       .pex170                 # If SCSI write
c   }
c   if (r3 == SCC_WRTVRFY_10 && r3 == SCC_WRTVRFY_16) {
        b       .pex170                 # If SCSI write/verify
c   }
#
c   if (*((UINT64*)&r10) >= *((UINT64*)&r4)) {
        b       .pex130                 # If insertion follows
c   }
        b       .pex175
#
# --- Adjacent operations - READ/WRITE commands (same commands)
#
.pex170:
        ld      il_fthd(r13),g1         # g1 = remaining queue
        mov     g3,g0                   # g0 = new ILT
        call    p$aftpchk               # Check for precedence
        cmpobe.f 0,g0,.pex190           # Jif so
#
        mov     r13,g0                  # g0 = next ILT
        call    p$aftpchk               # Check for precedence
        cmpobe.f 0,g0,.pex130           # Jif so
#
# --- Join requests if sufficient resources are available
#
        call    p$rchk                  # Check for sufficient resources
        cmpobe.f FALSE,g0,.pex130       # Jif not
#
        mov     r13,g2                  # g2 = next ILT
c       r10 = ((ILT*)g2)->phy_io.phy_overlap;  # r10 = overlap indicator from queued op.
c       r11 = ((ILT*)g3)->phy_io.phy_overlap;  # r11 = overlap indicator from new op.
        or      r10,r11,r10             # r10 = overlap indicator for joined operation
        ld      il_bthd(r13),r13        # Establish new next ILT for insert
        call    p$join                  # Attempt to join these requests
                                        # g3 = system ILT of joined op.
c       ((ILT*)g3)->phy_io.phy_overlap = r10;  # r10 = save overlap indicator
!       ldl     p_sda(g3),r10           # Establish new SDA/EDA
!       ldl     p_eda(g3),r8            # Establish new SDA/EDA
        b       .pex130
#
# --- Probable insertion point, however, check for precedence
#       If this is a read, check for writes following
#       If this is a write, check for either reads or writes following
#
.pex175:
        ld      il_fthd(r13),g1         # g1 = remaining queue
        mov     g3,g0                   # g0 = new ILT
        call    p$aftpchk               # Check for precedence
        cmpobe.f 0,g0,.pex190           # Jif so
#
# --- Insert ILT into this position after adjusting queue counts
#
.endif  # ENABLE_PHYSICAL_IO_OPT

.pex180:
        ld      dv_qcnt(g10),r7         # Bump DEV queue count
        addo    1,r7,r7
        st      r7,dv_qcnt(g10)
#
        cmpobe.f 0,r13,.pex210          # Jif queue append
        cmpobe.f g10,r12,.pex200        # Jif insert queue head
#
# --- Insert at queue midpoint
#
        st      g3,il_fthd(r12)         # Fwd link previous to ILT
        st      g3,il_bthd(r13)         # Bwd link next to ILT
        st      r12,il_bthd(g3)         # Bwd link ILT to previous
        b       .pex220
#
.if ENABLE_PHYSICAL_IO_OPT
#
# --- Set overlap indicator in new request
#
.pex190:
c       ((ILT*)g3)->phy_io.phy_overlap = TRUE; # Set overlap indicator in new request
        b       .pex130
.endif #ENABLE_PHYSICAL_IO_OPT
#
# --- Insert at head of existing queue
#
.pex200:
        st      g3,il_bthd(r13)         # Bwd link next to ILT
        st      g10,il_bthd(g3)         # Bwd link ILT to DEV queue head
        st      g3,dv_iltq_fhead(g10)   # Update DEV queue head
        b       .pex220
#
# --- Append to end of existing queue
#
#     In all circumstances this queue will minimally have at least the
#     HDA token present.
#
.pex210:
        st      g3,il_fthd(r12)         # Fwd link previous to ILT
        st      r12,il_bthd(g3)         # Bwd link ILT to previous
        st      g3,dv_iltq_tail(g10)    # Update DEV queue tail
.pex220:
        st      r13,il_fthd(g3)         # Fwd link ILT to next
#
# --- Activate initiator only if appropriate
#
        ldconst FALSE,r4                # Unlock this device
        stob    r4,dv_taglock(g10)
        ldconst TRUE,r5                 # Resync with initiator
        stob    r5,ch_sync(g11)
#
c       if (((DEV *)g10)->tMapAsgn == ((DEV *)g10)->tMapMask) {
            b .pex230
c       }
#
        ld      ch_init_pcb(g11),r3     # Get initiator PCB
        ldob    pc_stat(r3),r4          # Get process status
        cmpobne.f pcnrdy,r4,.pex230     # Jump if not (not ready)
#
        ldconst pcrdy,r4                # Set process ready
.ifdef HISTORY_KEEP
c CT_history_pcb(".pex220 setting ready pcb", r3);
.endif  # HISTORY_KEEP
        stob    r4,pc_stat(r3)
#
.pex230:
#
#   Determine if this task has exceeded the time allowed to run.  If so,
#       do a context switch.
#
        ld      K_ii+ii_time,r5         # Get the current timer
        cmpobl  r5,r6,.pex30            # Jif still more time to handle queue
        b       .pex20                  # Time exceeded, jump to exchange
#
# --- Error procedures ------------------------------------------------
#
# --- Set invalid port
#
.pex300:
        ld      dv_pdd(g10),r5
        cmpobe  0,r5,.pex305
        ldob    pd_flags(r5),r6
        bbc     pdbebusy,r6,.pex305
        ldconst ecbebusy,r5            # Set ise busy
        b       .pex370
.pex305:
        ldconst ecnonxdev,r5            # Set non-existent device
        b       .pex370
#
# --- Set invalid device
#
.pex310:
        ldconst ecinvdev,r5             # Set invalid device
        b       .pex370
#
# --- Set invalid function
#
.pex320:
        ldconst ecinvfunc,r5            # Set invalid function
        b       .pex370
#
# --- Set invalid strategy
#
.pex330:
        ldconst ecinvstr,r5             # Set invalid strategy
        b       .pex370
#
# --- Set null scatter/gather list
#
.pex340:
        ldconst ecnulsgl,r5             # Set null s/g list
        b       .pex370
#
# --- Set nonx device
#
.pex350:
        ldconst ecnonxdev,r5            # Set non-existent device
        b       .pex370
#
# --- Complete request with error
#
.pex370:
        stob    r5,pr_rstatus(r4)       # Save error code
        mov     g3,g7                   # Suspend possible mirrored reads
        call    p$susp_rr
        mov     g3,g1
        call    p$qcomp_ilt             # Complete this request
        b       .pex20
# End of p$exec ***************************************************************
#
.if ENABLE_PHYSICAL_IO_OPT
#
#**********************************************************************
#
#  NAME: p$aftpchk
#
#  PURPOSE:
#       To provide a means of determining whether or not a given ILT
#       has a precedent in the device queue.
#
#  DESCRIPTION:
#       The device queue is searched from the point supplied to the
#       end of the queue to determine if a precedent exists.  If
#       precedence exists, a zero value is returned.
#
#       Precedence exists whenever encountering an overlapping write
#       request when the stated request is a read or a write.  Precedence
#       also exists when encountering an overlapping read request when
#       the stated request is a write.
#
#  CALLING SEQUENCE:
#       call    p$aftpchk
#
#  INPUT:
#       g0 = ILT
#       g1 = queue starting point
#
#  OUTPUT:
#       g0 = 0 if ILT has precedent
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
p$aftpchk:
        cmpobe.f 0,g1,.af100            # Jif queue empty
# r4/r5 = p_sda
# r6/r7 = p_eda
# r8/r9 = OTHER p_sda
# r10/r11 = OTHER p_eda
!       ldl     p_sda(g0),r4            # Get candidate SDA/EDA
!       ldl     p_eda(g0),r6            # Get candidate SDA/EDA
#
# --- Search device queue from stated position
#
        mov     g1,r3                   # r3 = queue starting point
c       r15 = ((ILT*)g0)->phy_io.phy_writeflag; # Get write flag
        cmpobe.f FALSE,r15,.af30        # Jif not write
#
# --- Examine next entry (stated request was a write) -----------------
#
.af10:
!       ldl     p_sda(r3),r8            # Get SDA/EDA
!       ldl     p_eda(r3),r10           # Get SDA/EDA
c   if (0ULL == *((UINT64*)&r10)) {
        b       .af20                   # If invalid
c   }
#
c   if (*((UINT64*)&r6) <= *((UINT64*)&r8)) {
        b       .af90                   # If absolutely no overlap
c   }
c   if (*((UINT64*)&r4) < *((UINT64*)&r10)) {
        b       .af90                   # If overlap
c   }
#
# --- Link to next request
#
.af20:
        ld      il_fthd(r3),r3          # Get next entry
        cmpobne.t 0,r3,.af10            # Jif valid
        b       .af100
#
# --- Examine next entry (stated request was a read) ------------------
#
.af30:
!       ldl     p_sda(r3),r8            # Get SDA/EDA
!       ldl     p_eda(r3),r10           # Get SDA/EDA
c   if (0ULL == *((UINT64*)&r10)) {
        b       .af40                   # If invalid
c   }
c   if (*((UINT64*)&r6) <= *((UINT64*)&r8)) {
        b       .af40                   # If absolutely no overlap
c   }
c   if (*((UINT64*)&r4) >= *((UINT64*)&r10)) {
        b       .af40                   # If no overlap
c   }
c       r15 = ((ILT*)r3)->phy_io.phy_writeflag; # Get write flag
        cmpobe.f TRUE,r15,.af90         # Jif so
#
# --- Link to next request
#
.af40:
        ld      il_fthd(r3),r3          # Get next entry
        cmpobne.t 0,r3,.af30            # Jif valid
        b       .af100
#
# --- Indicate precedence currently exists
#
.af90:
        ldconst 0,g0                    # Indicate precedence exists
#
# --- Exit
#
.af100:
        ret
# End of p$aftpchk
#
#**********************************************************************
#
#  NAME: p$forpchk
#
#  PURPOSE:
#       To provide a means of determining whether or not a given ILT
#       has a precedent in the device queue.
#
#  DESCRIPTION:
#       The device queue is searched from the beginning to the requested
#       ILT to determine if a precedent exists.  If one exists, a zero value
#       is returned.
#
#       Precedence exists whenever encountering an overlapping write
#       request when the stated request is a read.  Precedence also
#       exists when encountering an overlapping read or write
#       request when the stated request is a write.
#
#  CALLING SEQUENCE:
#       call    p$forpchk
#
#  INPUT:
#       g5 = HDA ILT token
#       g6 = DEV
#       g7 = ILT
#
#  OUTPUT:
#       g7 = 0 if ILT has precedent
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
p$forpchk:
#
# --- Search device queue backwards from the specified ILT
#
# r4/r5 = p_sda (was r4)
# r6/r7 = p_eda
# r8/r9 = next p_sda
# r10/r11 = next p_eda
# r12/r13 = temp p_eda+MAXIO
# r14 = starting ILT
# r15 = temp
!       ldl     p_sda(g7),r4            # Get candidate SDA
!       ldl     p_eda(g7),r6            # Get candidate EDA
c       r15 = ((ILT*)g7)->phy_io.phy_writeflag; # Get write flag
        mov     g7,r14                  # Set starting ILT
        cmpobe.t FALSE,r15,.fo20_p      # Jif not a write
#
# --- Examine next entry (stated request was a write) -----------------
#
.fo10_p:
        ld      il_bthd(r14),r14        # Get previous entry
        cmpobe.f g6,r14,.fo100_p        # Jump if search complete
        cmpobe.f g5,r14,.fo10_p         # Jump if HDA ILT token
#
!       ldl     p_sda(r14),r8           # Get SDA/EDA
!       ldl     p_eda(r14),r10          # Get SDA/EDA
c   if (0ULL == *((UINT64*)&r10)) {
        b       .fo10_p                 # If invalid
c   }
#
#        Check for sufficient search
#
c       *((UINT64*)&r12) = *((UINT64*)&r10) + MAXIO;
c   if (*((UINT64*)&r4) > *((UINT64*)&r12)) {
        b       .fo100_p                # If sufficient search
c   }
c   if (*((UINT64*)&r6) <= *((UINT64*)&r8)) {
        b       .fo10_p                 # If no possible overlap
c   }
c   if (*((UINT64*)&r4) >= *((UINT64*)&r10)) {
        b       .fo10_p                 # If no overlap
c   }
        b       .fo90
#
# --- Examine next entry (stated request was a read) ------------------
#
.fo20_p:
        ld      il_bthd(r14),r14        # Get next entry
        cmpobe.f g6,r14,.fo100_p        # Jump if search complete
        cmpobe.f g5,r14,.fo20_p         # Jump if HDA ILT token
#
!       ldl     p_sda(r14),r8           # Get SDA/EDA
!       ldl     p_eda(r14),r10          # Get SDA/EDA
c   if (0ULL == *((UINT64*)&r10)) {
        b       .fo20_p                 # If invalid
c   }
c       *((UINT64*)&r12) = *((UINT64*)&r10) + MAXIO;
c   if (*((UINT64*)&r4) > *((UINT64*)&r12)) {
        b       .fo100_p                # If sufficient search
c   }
c   if (*((UINT64*)&r6) <= *((UINT64*)&r8)) {
        b       .fo20_p                 # If no possible overlap
c   }
c   if (*((UINT64*)&r4) >= *((UINT64*)&r10)) {
        b       .fo20_p                 # If no overlap
c   }
c       r15 = ((ILT*)r14)->phy_io.phy_writeflag; # Get write flag
        cmpobe.t FALSE,r15,.fo20_p      # Jif not
#
# --- Indicate precedence currently exists
#
.fo90:
        ldconst 0,g7                    # Indicate precedence exists
#
# --- Exit
#
.fo100_p:
        ret
# End of p$forpchk
#
#**********************************************************************
#
#  NAME: p$cancel
#
#  PURPOSE:
#       To provide a means of cancelling a specific redundant write
#       request that has been previously identified.
#
#  DESCRIPTION:
#       This routine has the ability to cancel a previously issued
#       write request whenever a newer write request totally encompasses
#       any previous request(s) disk addresses.  The method by which
#       this is accomplished is such that a system ILT is generated
#       with the newer request's data.  The system ILT has a thread
#       containing two or more non-system ILTs representing the
#       individual requests originally issued.
#
#       Cancellations may be recursive in that a system ILT generated by
#       this routine may be feed back in if another ILT candidate is
#       ever found.
#
#       A system request is a special request which has a thread
#       containing two or more non-system ILTs.  These non-system ILTs
#       may have been consolidated by:
#
#       1)  Joining - consolidation of smaller consecutive read or
#           write requests into a single larger system request to
#           reduce average request latency.
#       2)  Cancellation - Preemption of one or more write requests by
#           a single newer write request.
#
#       When a system ILT has finished execution, the non-system ILTs
#       that are associated with this request are completed at the same
#       time.
#
#  CALLING SEQUENCE:
#       call    p$cancel
#
#  INPUT:
#       g2  = ILT to cancel
#       g3  = ILT to supercede
#       g10 = DEV
#
#  OUTPUT:
#       g3  = system ILT
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
p$cancel:
        addo    16,sp,sp                # Save g0-g2
#
c       record_physical_ilt(FR_PRP_CANCELED, (ILT *)g2, (UINT32)g3);
.if     DEBUG_FLIGHTREC_P
        ldconst frt_p_cancel,r3         # Type
        st      r3,fr_parm0             # Physical - p$cancel
        st      g2,fr_parm1             # ILT
        st      g3,fr_parm2             # Supercedeing ILT
        st      g10,fr_parm3            # DEV
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_P
#
# --- Retain highest priority
#
c       r3 = ((ILT*)g2)->phy_io.phy_priority;
c       r4 = ((ILT*)g3)->phy_io.phy_priority;

        stt     g0,-16(sp)
        cmpobge.f r4,r3,.ca05           # Jif superceding priority OK
#
c       ((ILT*)g3)->phy_io.phy_priority = r3; # Set up superceding priority
#
# --- Remove cancelled ILT from DEV queue
#
.ca05:
        ld      il_fthd(g2),r4          # Remove ILT from queue
        ld      il_bthd(g2),r5          # Remove ILT from queue
        st      r4,il_fthd(r5)          # Update forward thread
        cmpobne.t 0,r4,.ca10            # Jif not last entry
#
        st      r5,dv_iltq_tail(g10)    # Update queue tail
        b       .ca20
#
.ca10:
        st      r5,il_bthd(r4)          # Update backward thread
#
# --- Update DEV and PDD queue count ----------------------------------
#
.ca20:
        ld      dv_qcnt(g10),r3         # Bump DEV queue count down for
        subo    1,r3,r3                 #  the request removed
        st      r3,dv_qcnt(g10)
#
# --- Build join count and thread from cancelled ILT
#
        ld      p_jct(g2),r14           # Get possible join count/thd
        ld      p_jth(g2),r15           # Get possible join count/thd
        cmpobne.f 0,r14,.ca30           # Jif system ILT
#
        ldconst 1,r14                   # Initialize join count
        st      r15,il_fthd(g2)         # Close link
        mov     g2,r15                  # Initialize join thread
        b       .ca40
#
# --- Release cancelled system ILT/PRP/SGL
#
.ca30:
        lda     -ILTBIAS(g2),g1         # Release system ILT/PRP/SGL
        call    M$rip
#
# --- Check superceding ILT for being a system ILT
#
.ca40:
        ld      p_jct(g3),r12           # Get super ILT join info
        ld      p_jth(g3),r13           # Get super ILT join info
        cmpobne.f 0,r12,.ca80           # Jif system ILT
#
# --- Build join count/thd for superceding ILT ------------------------
#
        st      r12,il_fthd(g3)         # Close link
        mov     g3,r13                  # Set thread origin
        ldconst 1,r12                   # Set join count
#
# --- Generate new system ILT/PRP/SGL
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
        movl    0,r4                    # Initialize ILT
        stl     r4,r_mir(g1)            # Clear Assoc. ILT and SN
#
# --- Initialize new ILT
#
        lda     ILTBIAS(g1),g1          # Advance to callee area
        ldq     il_w0(g3),r4            # Copy over il_w0 thru il_w3
        stq     r4,il_w0(g1)
        ldl     il_w6(g3),r4            # Copy over il_w6 and il_w7
        stl     r4,il_w6(g1)
#
# --- Initialize new PRP
#
        ld      r_prp-ILTBIAS(g3),r3    # Get original PRP
        ldq     pr_func(r3),r4          # Copy over PRP verbatim
        stq     r4,pr_func(g2)
!       ldq     pr_sda(r3),r4
        stq     r4,pr_sda(g2)
        ldq     pr_sglptr(r3),r4
        setbit  31,r5,r5                # Indicate SGL as borrowed
        stq     r4,pr_sglptr(g2)
        ldl     pr_rsbytes(r3),r4
        stl     r4,pr_rsbytes(g2)
        ldq     pr_cmd(r3),r4
        stq     r4,pr_cmd(g2)
#
        mov     g1,g3                   # Set new system ILT
#
# --- Update superceding system ILT (merge both join threads) ---------
#
.ca80:
        mov     r13,r4                  # Scan for end of thread
.ca90:
        mov     r4,r5                   # Save previous ILT
        ld      il_fthd(r4),r4          # Get next ILT
        cmpobne.f 0,r4,.ca90            # Jif valid
#
        st      r15,il_fthd(r5)         # Merge both threads
        addo    r12,r14,r12             # Calculate combined join count
        st      r12,p_jct(g3)           # Set up join count/thd
        st      r13,p_jth(g3)           # Set up join count/thd
#
# --- Exit
#
        ldt     -16(sp),g0              # Restore g0-g2
        ret
# End of p$cancel *************************************************************
#
#**********************************************************************
#
#  NAME: p$rchk
#
#  PURPOSE:
#       To provide a common means of determining whether or not sufficient
#       resources exist to attempt a join or cancel at this time.
#       We need to avoid blocking at the physical layer at any cost.
#
#  DESCRIPTION:
#       A check is made to determine if there are sufficient resources
#       for an ILT, PRP and possible SGL.  If so, this routine returns
#       successfully.
#
#  CALLING SEQUENCE:
#       call    p$rchk
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       g0 = TRUE if sufficient resources
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
p$rchk:
#
# --- Get current resources
#
        ld      K_ii+ii_nccur,r4        # Get current available local
                                        #  noncache SDRAM
        ldconst (MINNCDRAM/4)*3,r3      # Get minimum SDRAM req'd
        ld      K_ii+ii_iltcur,r5       # Get current available ILTs
        mov     FALSE,g0                # Set insufficient resources for now
        ld      K_ii+ii_prpcur,r6       # Get current available PRPs
#
# --- Check current resources
#
        cmpobl.f r4,r3,.rc100_p         # Jif insufficient local noncache SDRAM
        cmpobe.f 0,r5,.rc100_p          # Jif insufficient ILTs
        cmpobe.f 0,r6,.rc100_p          # Jif insufficient PRPs
#
# --- Flag sufficient resources
#
        mov     TRUE,g0                 # Set sufficient resources
#
# --- Exit
#
.rc100_p:
        ret
#
#**********************************************************************
#
#  NAME: p$join
#
#  PURPOSE:
#       To provide a common means of merging two like requests into a
#       single larger request.
#
#  DESCRIPTION:
#       This routine has the ability to join the following types of
#       requests:
#
#       1)  Non-system request to non-system request.
#       2)  A non-system request with a system request.
#       3)  A system request with a non-system request.
#       4)  A system request with a system request.
#
#       Joins may be recursive in that a system ILT generated by this
#       routine may be feed back in if another ILT candidate is
#       found.
#
#       A system request is a special request which has a thread
#       containing two or more non-system ILTs.  These non-system ILTs
#       may have been consolidated by:
#
#       1)  Joining - consolidation of smaller consecutive read or
#           write requests into a single larger system request to
#           reduce average request latency.
#       2)  Cancellation - Preemption of one or more write requests by
#           a single newer write request.
#
#       When a system ILT has finished execution, the non-system ILTs
#       that are associated with this request are completed at the same
#       time.
#
#  CALLING SEQUENCE:
#       call    p$join
#
#  INPUT:
#       g2  = existing ILT to join
#       g3  = new ILT
#       g10 = DEV
#
#  OUTPUT:
#       g3  = system ILT
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
p$join:
        addo    16,sp,sp                # Save g0-g2, g7
        stt     g0,-16(sp)
        st      g7,-4(sp)
#
c       record_physical_ilt(FR_PRP_JOINED, (ILT *)g2, (UINT32)g3);
.if     DEBUG_FLIGHTREC_P
        ldconst frt_p_join,r3           # Type
        st      r3,fr_parm0             # Physical - p$join
        st      g2,fr_parm1             # ILT
        st      g3,fr_parm2             # Supercedeing ILT
        st      g10,fr_parm3            # DEV
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_P

#
# --- Remove existing ILT from DEV queue
#
        ld      il_fthd(g2),r4          # Remove ILT from queue
        ld      il_bthd(g2),r5          # Remove ILT from queue
        st      r4,il_fthd(r5)          # Update forward thread
        cmpobne.t 0,r4,.jo10            # Jif not last entry
#
        st      r5,dv_iltq_tail(g10)    # Update queue tail
        b       .jo20
#
.jo10:
        st      r5,il_bthd(r4)          # Update backward thread
#
# --- Check existing ILT for redundant reads
#
.jo20:
        ld      r_mir-ILTBIAS(g2),r3    # Get mirrored read link
        cmpobe.t 0,r3,.jo30             # Jif none
#
        mov     g2,g7                   # Suspend mirrored reads
        call    p$susp_rr
#
# --- Check new ILT for redundant reads
#
.jo30:
        ld      r_mir-ILTBIAS(g3),r3    # Get mirrored read link
        cmpobe.t 0,r3,.jo40             # Jif none
#
        mov     g3,g7                   # Suspend mirrored reads
        call    p$susp_rr
#
# --- Adjust DEV queue counts
#
.jo40:
        ld      dv_qcnt(g10),r3         # Adjust DEV queue count for
        subo    1,r3,r3                 #  the request removed
        st      r3,dv_qcnt(g10)
#
# --- Determine low and high ILT requests (by LBA)
#
c   if (((ILT*)g2)->phy_io.phy_eda != ((ILT*)g3)->phy_io.phy_sda) {
        mov     g2,r15                  # r15 = high ILT
        mov     g3,r14                  # r14 = low ILT
c   } else {
        movl    g2,r14                  # r14 = low ILT  r15 = high ILT
c   }
#
# --- Allocate and initialize new system ILT/PRP
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

        mov     g1,g3                   # g3 = final system ILT
        movl    0,r4                    # Initialize ILT
        stl     r4,r_mir(g3)            # Clear Assoc. ILT and SN
#
# --- Initialize new system ILT
#
        lda     ILTBIAS(g3),g3          # Advance to callee area

c       r4 = ((ILT*)r14)->phy_io.phy_priority; # Copy over priority
c       r8 = ((ILT*)r15)->phy_io.phy_priority; # Other ILTs age
        cmpo    r4,r8                   # Retain oldest age
        selg    r8,r4,r4
c       ((ILT*)g3)->phy_io.phy_priority = r4; # Save oldest priority

        ldob    p_cmd(r14),r5           # Copy over scsi command
        stob    r5,p_cmd(g3)

        ldl     p_sda(r14),r6           # Copy over sda
        stl     r6,p_sda(g3)

!       ldl     p_eda(r15),r6           # Using correct EDA
        stl     r6,p_eda(g3)
#
c       ((ILT*)g3)->phy_io.phy_overlap = ((ILT*)r14)->phy_io.phy_overlap | ((ILT*)r15)->phy_io.phy_overlap;
c       ((ILT*)g3)->phy_io.phy_writeflag = ((ILT*)r14)->phy_io.phy_writeflag | ((ILT*)r15)->phy_io.phy_writeflag;
# NOTDONEYET -- following cannot be this way.
.ifndef PERF
c if (((ILT*)r14)->phy_io.phy_tag != ((ILT*)r15)->phy_io.phy_tag) {
c   fprintf(stderr, "p$join - attempting to merge different phy_tag (%d & %d)\n", ((ILT*)r14)->phy_io.phy_tag, ((ILT*)r15)->phy_io.phy_tag);
c   abort();
c }
.endif  # PERF
c       ((ILT*)g3)->phy_io.phy_tag = ((ILT*)r14)->phy_io.phy_tag | ((ILT*)r15)->phy_io.phy_tag;

        st      g10,p_qhd(g3)           # Set up DEV
#
# --- Initialize new system PRP
#
        ld      r_prp-ILTBIAS(r14),r3   # r3 = low PRP
        ld      r_prp-ILTBIAS(r15),r8   # r8 = high PRP
        ldq     pr_func(r3),r4          # Copy over PRP
        stq     r4,pr_func(g2)
#
!       ldl     pr_sda(r3),r4           # Use SDA from low request
        stl     r4,pr_sda(g2)
!       ldl     pr_eda(r8),r6           # Use correct EDA from high request
        stl     r6,pr_eda(g2)
#
        ldq     pr_sglptr(r3),r4
        ld      pr_rqbytes(r8),r11      # Get pr_rqbytes
        addo    r11,r7,r7               # Sum pr_rqbytes
        shro    20,r7,r6                # One second timeout for each 2048 blocks
        addo    BTIMEOUT,r6,r6          # Compute timeout
        stq     r4,pr_sglptr(g2)
#
        ldl     pr_rsbytes(r3),r4
        stl     r4,pr_rsbytes(g2)
#
        ldq     pr_cmd(r3),r4
        stq     r4,pr_cmd(g2)
#
# --- Update SCSI command length
#
!       ldl     pr_sda(g2),r4           # Get SDA
!       ldl     pr_eda(g2),r6           # Get EDA
c       *((UINT64*)&r6) = *((UINT64*)&r6) - *((UINT64*)&r4);
        ldob    pr_cbytes(g2),r4        # Get command length
        cmpobe  16,r4,.jo85             # Jif 16 byte command
        cmpobe  10,r4,.jo80             # Jif 10 byte command
        stob    r6,pr_cmd+4(g2)         # Update 6 byte command length
        b       .jo90
#
.jo80:
        bswap   r6,r6                   # Convert endians
        shro    16,r6,r6
        stos    r6,pr_cmd+7(g2)         # Update 10 byte command length
        b       .jo90
#
.jo85:
        bswap   r6,r6                   # Convert endians
        st      r6,pr_cmd+10(g2)        # Update 10 byte command length
#
# --- Merge old SGLs into new SGL and link to new system PRP
#
.jo90:
        ld      pr_sglptr(r3),g0        # Pass 1st SGL
        ld      pr_sglptr(r8),g1        # Pass 2nd SGL
c       g0 = PM_MergeSGL(&g1, g0, g1);  # Merge SGLs
        st      g0,pr_sglptr(g2)        # Set up SGL pointer
        st      g1,pr_sglsize(g2)       #  and size
#
# --- Merge join threads and counts
#
        ld      p_jct(r14),r4           # r4 = low count  r5 = low thread
        ld      p_jth(r14),r5           # r4 = low count  r5 = low thread
        ld      p_jct(r15),r6           # r6 = hi count   r7 = hi thread
        ld      p_jth(r15),r7           # r6 = hi count   r7 = hi thread
        addo    r4,r6,r3                # Sum join counts
#
# --- Process low LBA request
#
        cmpobe.t 0,r4,.jo110            # Jif low request non-system
#
        st      r5,p_jth(g3)            # Set origin of merged join thread
.jo100:
        subo    1,r4,r4                 # Search for end of join thread
        cmpobe.f 0,r4,.jo120            # Jif end
        ld      il_fthd(r5),r5          # Continue search
        b       .jo100
#
.jo110:
        addo    1,r3,r3                 # Adjust join count for non-system
        st      r14,p_jth(g3)           # Set origin of merged join thread
        mov     r14,r5
#
# --- Process high LBA request
#
.jo120:
        cmpobe.t 0,r6,.jo130            # Jif high request non-system
#
        st      r7,il_fthd(r5)          # Merge threads
        b       .jo140
#
.jo130:
        addo    1,r3,r3                 # Adjust join count for non-system
        st      r6,il_fthd(r15)         # Close link
        st      r15,il_fthd(r5)         # Link with previous joins
#
.jo140:
        st      r3,p_jct(g3)            # Set up join count
#
# --- Release all original system ILT/PRP/SGLs back to system
#
        ld      p_jct(r14),r3           # Check low ILT original
        cmpobe.t 0,r3,.jo150            # Jif not system request
#
        lda     -ILTBIAS(r14),g1        # Release low ILT original
        call    M$rip
#
.jo150:
        ld      p_jct(r15),r3           # Check high ILT original
        cmpobe.t 0,r3,.jo1000           # Jif not system request
#
        lda     -ILTBIAS(r15),g1        # Release high ILT original
        call    M$rip
#
# --- Exit
#
.jo1000:
        ldt     -16(sp),g0              # Restore g0-g2, g7
        ld      -4(sp),g7
        ret
# End of p$join ***************************************************************
.endif #ENABLE_PHYSICAL_IO_OPT
#
#**********************************************************************
#
#  NAME: p$init
#
#  PURPOSE:
#       To provide a means of initiating a request to a physical device.
#
#  DESCRIPTION:
#       The executive process stimulates this process whenever a queue tag
#       is available and a new request exists in the DEV queue.
#       The completion process also stimulates this process whenever a
#       queue tag is deassigned and the following conditions exist:
#
#        1) all of the possible queue tags were previously assigned, and
#        2) at least one request exists in the DEV queue.
#
#       Each DEV associated with this CHN is checked for a possible queue
#       tag assignment.  If so, the command is initiated at the ISP layer.
#
#  CALLING SEQUENCE:
#       process call
#
#  INPUT:
#       g4 = CHN
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g2, g3, g5, g6, g7
#
#**********************************************************************
#
p$init:
#
# --- Preload constants during initialization
#
        ld      ch_init_pcb(g4),r15     # r15 = this PCB
        ldconst FALSE,r14               # r14 = FALSE/zero
        ldconst pcnrdy,r13              # r13 = process not ready status
        mov     g4,r8                   # r8  = CHN
        b       .in15                   # Awake, branch to main loop
#
# --- Sleep until new work arrives
#
.in10:
        stob    r13,pc_stat(r15)        # Set this process not ready
.in15:
        call    K$qxchang               # Exchange processes
#
# --- MAIN LOOP Awaken here.  Resynchronize initiator and process new work
#
        ld      ch_devcnt(r8),r12       # r12 = number of devices attached.
        ld      ch_startdev(r8),g6      # g6 = ptr to starting DEVice
        cmpobe  0,r12,.in10             # Jif no device attached
#
# --- determine if there is a device available
#
        cmpobe  0,g6,.in10              # Jif start device doesn't exists
#
# --- Setup starting DEV for next rotation
#
        ld      dv_ndev(g6),r3          # Rotate starting DEV for next
                                        #  balanced initiation
        stob    r14,ch_sync(r8)         # Set initiator sync to FALSE
        st      r3,ch_startdev(r8)      # Save start device
        mov     r8,g4                   # Get CHN
#
# --- Determine if request wait for this device is in effect.
#
.in19:
        ldos    dv_wait(g6),r3          # Get request wait count
        cmpobne 0,r3,.in50              # Jif wait
        ldob    dv_setupretryactive(g6),r3
        cmpobne 0,r3,.in50              # Jif wait
#
# --- determine if there is a task on the failed queue
#
        ld      dv_failq_hd(g6),g7      # is there anything on failed q?
        cmpobe  0,g7,.in25              # no - br
#
        ld      il_fthd(g7),r4          # get forward thread
        st      r4,dv_failq_hd(g6)      # save new head
        cmpobne 0,r4,.in40              # JIf q not empty
#
        st      r4,dv_failq_tl(g6)      # clear tail pointer
        b       .in40                   # br
#
# --- Check for commands on device queue
#
.in25:
        ld      dv_qcnt(g6),r4          # Get queue count
        cmpobe.t 0,r4,.in50             # Jif empty
#
# --- Command found.  Check for available queue tag
#
c       if (((DEV*)g6)->tMapAsgn == ((DEV*)g6)->tMapMask) {
            b .in50
c       }
#
# --- Check for locked device
#
        ldob    dv_taglock(g6),r3       # Check for locked device
        cmpobe.f TRUE,r3,.in50          # Jif so
#
# --- Attempt assignment of queue tag making sure that there are no other
#     initiations currently pending with a lower queue tag number
#
#; LSW NOTE - initiation linkage has been removed for ISP.
        lda     dv_hdatoken(g6),g5      # Get HDA token
c       g2 = ffsll(~((DEV*)g6)->tMapAsgn);
c       if (g2 == 0 ) {
            b .in50
c       }
#
# --- Locate next ILT to initiate
#
                                        # g2 = tag number
                                        # g5 = HDA token
                                        # g6 = DEV
        call    p$get_ilt               # Get next ILT (result in g7)
        cmpobe.f 0,g7,.in50             # If none, jump
#
# --- Reload the loop ID and Channel in case position on loop changed, unless
#     specified using the UCL flag in pr_flags
#
.in40:
        ld      dv_id(g6),r4            # Get Loop ID
        ld      r_prp-ILTBIAS(g7),r5    # Get the PRP
        ldob    dv_chn(g6),r3           # Get channel
c if (r3 >= MAXCHN) fprintf(stderr,"INVDEVPORT-.in40 - dv_chn out of range %ld\n", r3);
        cmpoble.f MAXCHN,r3,.in50       # Jif invalid port (ID too large)
        ldob    pr_flags(r5),r11        # Get PRP flags
        bbs     prUCL,r11,.in45         # Jif use prp channel/lid, not device
        stob    r3,pr_channel(r5)       # Set new channel
        st      r4,pr_id(r5)            # Set new device LID
.in45:
#
# --- Initiate selected I/O operation
#
c       record_pdriver(FR_PDRIVER_ISSUED, (void *)r5, 0);
        lda     p$qcomp_ilt,r3
        st      r3,il_cr(g7)            # Set completion routine
        lda     ILTBIAS(g7),g7          # advance to next nest level of ILT
        call    ISP$initiate_io         # Call ISP layer to initiate command
        b       .in19                   # Loop back and check for more work
#
# --- Done with this device.
# --- Check for work on other devices, or new work due to resynchronization
#
.in50:
        subo    1,r12,r12               # decrement device count
        ld      dv_ndev(g6),g6          # Link to next DEV
        cmpobe  0,g6,.in15              # if nextdev is null start over at beginning
        cmpobne.t 0,r12,.in19           # Jif more
        ldob    ch_sync(r8),r3          # Check resynchronization
        cmpobe.f TRUE,r3,.in15          # If so, jump
        b       .in10                   # Loop back and sleep
# End of p$init ***************************************************************
#
#**********************************************************************
#
#  NAME: p$get_ilt
#
#  PURPOSE:
#       To provide a common means of locating the next ILT to be initiated
#       for the given device.
#
#  DESCRIPTION:
#       The next ILT is located adjacent to the HDA token for this device.
#       The ILT is removed from the device queue.  This routine may be
#       called from the process level.
#
#       A check is made to see if the chosen request has any overlap
#       with previously issued requests.
#
#       The base priority for a physical request is directly derived from
#       the request strategy.
#
#                         Strategy  Class  Priority
#                             0      Low      0
#                             1     Normal    8
#                             2      High     16
#
#       The base priority is then modified by adding the HAB priority.
#
#                                SERVER PRIORITY
#                              0     1     2     3
#                              -     -     -     -
#                    Low       1     3     5     7
#                    Normal    9    11    13    15
#                    High     17    19    21    23
#
#
#       The priority is aged based upon the amount of time spent in the
#       queue waiting to be initiated.
#
#                             TIME IN QUEUE (MS)
#
#                     125   250   375   500   625   750  etc.
#                     ---   ---   ---   ---   ---   ---
#     Priority Adder    3     6     9    12    15    18  ...
#
#
#             TIME IN QUEUE BEFORE REACHING HIGH PRIORITY (MS)
#
#                                 SERVER PRIORITY
#                               0     1     2     3
#                               -     -     -     -
#                    Low      750   625   625   500
#                    Normal   375   375   250   125
#                    High       0     0     0     0
#
#  CALLING SEQUENCE:
#       call    p$get_ilt
#
#  INPUT:
#       g2 = tag number
#       g5 = HDA token
#       g6 = DEV
#
#  OUTPUT:
#       g7 = ILT if != 0
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
p$get_ilt:
        ldob    dv_pri(g6),r12          # Get priority criteria
        ldconst SIMPLE,r8               # Set simple queue tag for now
.if ENABLE_PHYSICAL_IO_OPT
        ldob    dv_blk(g6),r3           # Get block device indicator
        cmpobe.f FALSE,r3,.ge200        # Jif non-block device
#
# --- Prepare to locate next ILT for block device ---------------------
#
        ldob    dv_bwdseek(g6),r13      # Get current seek direction
#        ld      dv_tmap_mask(g6),r14    # Get tagged command mask
        ld      dv_qcnt(g6),r5          # Get current queue depth
        ldl     dv_iltq_fhead(g6),r6    # Get head & tail pointer for check
c       if (r6 == r7) {
c fprintf(stderr, "%s%s:%u get$ilt: No ILT, dev=%08lx, qcnt=%ld, head&tail=%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g6, r5, r6);
c           abort();    /* Get the dump now */
c       }
#
# --- Search for the next candidate ILT
#
.ge10:
        ld      il_fthd(g5)[r13*4],r6   # Get 1st ILT from HDA token
        cmpobe.f 0,r6,.ge35             # Jif null
        cmpobe.f g6,r6,.ge35            # Jif DEV
#
.ge20:
c       r9 = ((ILT*)r6)->phy_io.phy_priority; # Get next candidate priority
        subo    1,r5,r5                 # Adjust search limit
        cmpobge.t r9,r12,.ge39          # Jif ILT meets criteria
#
.ge25:
        cmpobe.f 0,r5,.ge30             # Jif entire queue searched
        ld      il_fthd(r6)[r13*4],r6   # Get next ILT
        cmpobe.f 0,r6,.ge35             # Jif null
        cmpobe.f g6,r6,.ge35            # Jif DEV
        b       .ge20
#
# --- Candidate not yet found, relax priority criteria if possible
#
.ge30:
        ld      dv_qcnt(g6),r5          # Get current queue depth
        mov     0,g7                    # Indicate candidate not found for now
        cmpobe.f 0,r12,.ge1000          # Jif all priorities searched
#
        mov     0,r12                   # Relax priority criteria
        stob    r12,dv_pri(g6)
#
# --- Invert search direction
#
.ge35:
        xor     1,r13,r13               # Invert search direction
        stob    r13,dv_bwdseek(g6)      # Save new direction
        b       .ge10
#
# --- Found an ILT at or above the threshold.  Now test the last pri submitted
#     such that if the pri of this request is higher it will be HOQ else simple.
#
.ge39:
        ldob    dv_lastpri(g6),r11      # Get pri of last submission and stall flag
        cmpobg  r9,r11,.ge40

        ldconst HEADOFQ,r8              # Set head of queue
#
# --- Candidate found, validate for precedence
#
.ge40:
.if     DEBUG_FLIGHTREC_P
        ldconst frt_p_getilt,r11        # Type
        st      r11,fr_parm0            # Physical - p$get_ilt
        st      r6,fr_parm1             # ILT
        st      g5,fr_parm2             # HDA token
        st      g6,fr_parm3             # DEV
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_P
        mov     r6,g7                   # Get selected candidate
c       r3 = ((ILT*)r6)->phy_io.phy_overlap;   # Get possible overlap indicator
        cmpobe.t FALSE,r3,.ge45         # Jif not
#
        call    p$forpchk               # Perform precedence check
        cmpobe.f 0,g7,.ge25             # Jif so
#
# --- Ensure overlap does not exist w/ any previously issued tag commands
#
.ge45:
# NOTE: this tag may be anything, including a free memory pattern -- ignore it.
!       ldl     p_sda(g7),r6            # Get candidate SDA
!       ldl     p_eda(g7),r10           # Get candidate EDA
c   if (0ULL == *((UINT64*)&r10)) {
        b       .ge80                   # Jump if non data xfer
c   }
c   {
c       UINT64 bitfield;
c       bitfield = ((DEV*)g6)->tMapMask & ((DEV*)g6)->tMapAsgn;
c       r5 = ffsll(bitfield);
c       while ( r5 != 0) {
c           bitfield &= ~(1 << r5);
c           r3 = 0;
c           if (((DEV*)g6)->tagIlt[r5] != NULL && ((DEV*)g6)->tagIlt[r5]->phy_io.phy_eda != 0) {
c               if ((*((UINT64*)&r10) < ((DEV*)g6)->tagIlt[r5]->phy_io.phy_sda) || (*((UINT64*)&r6) > ((DEV*)g6)->tagIlt[r5]->phy_io.phy_eda)) {
c                   r3 = 1;
c               }
c           }
c           if (r3 == 0) {
                ldconst 0,g7                    # Indicate overlap
                b       .ge1000                 # Reject
c           }
c       }
c   }
#
# --- Update HDA token to reflect last I/O initiated
#
        # Set SDA = EDA
c       *((UINT64*)&r10) = *((UINT64*)&r6);
        stl     r6,p_sda(g5)            # Update HDA token SDA
        stl     r10,p_eda(g5)           # Update HDA token EDA
#
# --- Remove selected ILT entry from queue
#
.ge80:
        ld      il_fthd(g7),r4          # Remove ILT from queue
        ld      il_bthd(g7),r5          # Remove ILT from queue
        st      r4,il_fthd(r5)          # Update forward thread
        cmpobne.t 0,r4,.ge90            # Jif not last entry
#
        st      r5,dv_iltq_tail(g6)     # Update queue tail
        b       .ge95
#
.ge90:
        st      r5,il_bthd(r4)          # Update backward thread
#
# --- Remove HDA token ILT entry from queue if necessary
#
.ge95:
        cmpobe.t r4,g5,.ge300           # Jif HDA immediately follows
        cmpobe.t r5,g5,.ge300           # Jif HDA immediately precedes
#
        ld      il_fthd(g5),r6          # Remove ILT from queue
        ld      il_bthd(g5),r7          # Remove ILT from queue
        st      r6,il_fthd(r7)          # Update forward thread
        cmpobne.t 0,r6,.ge100           # Jif not last entry
#
        st      r7,dv_iltq_tail(g6)     # Update queue tail
        b       .ge110
#
.ge100:
        st      r7,il_bthd(r6)          # Update backward thread
#
# --- Reinsert HDA token ILT into queue replacing selected ILT
#
.ge110:
        st      r4,il_fthd(g5)          # Update HDA fwd/bwd links
        st      r5,il_bthd(g5)          # Update HDA fwd/bwd links
        st      g5,il_fthd(r5)          # Update fwd link from prev
        cmpobe.f 0,r4,.ge115            # Jif end of queue
#
        st      g5,il_bthd(r4)          # Update bwd link from next
        b       .ge300
#
.ge115:
        st      g5,dv_iltq_tail(g6)     # Update queue tail
        b       .ge300
#
# --- Locate next ILT for non-block device ----------------------------
#
.ge200:
.endif #ENABLE_PHYSICAL_IO_OPT
        lda     dv_iltq_fhead(g6),g7    # Locate 1st ILT
.ge210:
        ld      il_fthd(g7),g7          # Check for HDA token
        cmpobe.f g5,g7,.ge210           # If so, jump
#
        ld      il_fthd(g7),r4          # Dequeue 1st ILT
        ld      il_bthd(g7),r5          # Dequeue 1st ILT
        st      r4,il_fthd(r5)          # Update fwd thread
        cmpobe.t 0,r4,.ge220            # Jif last request
#
        st      r5,il_bthd(r4)          # Update bwd thread
        b       .ge300
#
.ge220:
        st      r5,dv_iltq_tail(g6)     # Update queue tail
#
# --- Check for possible redundant mirrored reads
#
.ge300:
        ld      r_mir-ILTBIAS(g7),r3    # Get mirrored read link
        cmpobe.t 0,r3,.ge305            # Jif null
#
        call    p$susp_rr               # Suspend mirrored reads
#
# --- Adjust DEV queue count ------------------------------------------
#
.ge305:
        ld      dv_qcnt(g6),r6          # Adjust DEV queue count
        subo    1,r6,r6
        st      r6,dv_qcnt(g6)
#
# --- Generate simple queue tag only
#
# NOTE: this tag may be anything, including a free memory pattern -- ignore it.
?       ld      dv_tmap_asgn(g6),r10    # Get tag cmd map assignments
        stob    r8,p_scb+ILTBIAS(g7)    # Record simple/ordered message in SCB
#
# --- Mark queue tag as assigned and uninitiated
#
        st      g7,dv_tag_ilt(g6)[g2*4] # Record ILT related to this tag
        setbit  g2,r10,r10              # Set tag as assigned
c       ((ILT*)g7)->phy_io.phy_tag = g2; # Record tag number in ILT
# NOTE: this tag may be anything, including a free memory pattern -- ignore it.
?       st      r10,dv_tmap_asgn(g6)
#
# --- Exit
#
.if ENABLE_PHYSICAL_IO_OPT
.ge1000:
.endif  # ENABLE_PHYSICAL_IO_OPT
        ret
# End of p$get_ilt ************************************************************
#
#**********************************************************************
#
#  NAME: p$susp_rr
#
#  PURPOSE:
#       To provide a common means of suspending mirrored reads whenever
#       the first read of a set of mirrored reads gets initiated or
#       the first read gets joined.
#
#  DESCRIPTION:
#       Each mirrored read, if any, is removed from the CHN or DEV queue.
#
#  CALLING SEQUENCE:
#       call    p$susp_rr
#
#  INPUT:
#       g7 = surviving ILT (callee level)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
p$susp_rr:
        lda     -ILTBIAS(g7),r14        # Form surviving caller's ILT
        mov     0,r15
        ld      r_mir(r14),r13          # Get 1st mirrored read link
        cmpobe.f 0,r13,.su100           # Jif none
        cmpobe.f r13,r14,.su100         # Jif only one
#
# --- Remove next ILT/PRP from its corresponding queue (executive
#     queue or DEV queue)
#
.su10:
        lda     ILTBIAS(r13),r12        # Form callee's ILT
        ld      p_qhd(r12),r3           # Get corresponding queue
        cmpobe.f 0,r3,.su90             # Jif none - previously suspended
#
        ld      qu_qcnt(r3),r4          # Adjust queue count
        st      r15,p_qhd(r12)          # Clear queue
        subo    1,r4,r4
        st      r4,qu_qcnt(r3)
c       record_physical_ilt(FR_PRP_MIRROR_DISCARD, (ILT *)r12, 0);
#
        lda     P_exec_qu,r6            # Check for executive queue
        cmpobe.t r3,r6,.su20            # Jif so
#
# --- Adjust outstanding request count
#
        ld      P_orc,r7                # Get outstanding request count
        subo    1,r7,r7                 # Adjust outstanding req count
        st      r7,P_orc
#
# --- Remove request from executive queue or DEV queue
#
.su20:
        ld      il_fthd(r12),r4         # Remove ILT from queue
        ld      il_bthd(r12),r5         # Remove ILT from queue
        st      r4,il_fthd(r5)          # Update forward thread
        cmpobne.t 0,r4,.su40            # Jif not last entry
#
        cmpobe.f r5,r3,.su30            # Jif backward link to queue
                                        #  head
        st      r5,qu_tail(r3)          # Update queue tail
        b       .su90
#
.su30:
        mov     r4,r5                   # Clear queue head/tail
        stl     r4,qu_head(r3)
        b       .su90
#
.su40:
        st      r5,il_bthd(r4)          # Update backward thread
#
# --- Link to next ILT/PRP
#
.su90:
        ld      r_mir(r13),r13          # Link to next ILT/PRP
        cmpobne.f r14,r13,.su10         # Jif more to process
#
# --- Exit
#
.su100:
        ret
#
#**********************************************************************
#
#  NAME: p$qcomp_ilt
#
#  PURPOSE:
#       To provide a common means of queueing an ILT for completion by
#       the p$comp_ilt process.
#
#  DESCRIPTION:
#       The ILT is inserted into the tail of the completion queue.
#       If the completion process is not ready, it is made ready.
#
#  CALLING SEQUENCE:
#       call    p$qcomp_ilt
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
        .globl  p$qcomp_ilt
p$qcomp_ilt:
        lda     P_comp_qu,r11           # Get completion queue pointer
c       record_pdriver_ilt(FR_PDRIVER_COMPLETE, (ILT *)g1, 0);
        b       K$cque
#
#**********************************************************************
#
#  NAME: p$comp_ilt
#
#  PURPOSE:
#       To provide a means of completing processed requests back to
#       their original caller.
#
#  DESCRIPTION:
#       The queue of completed requests is processed in its entirety
#       by being completed back to the original callers.  When the
#       queue becomes empty, this process temporarily deactivates.
#
#       If the completed request is a system request, the system
#       request is released after completing the associated requests
#       back to their original callers.
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
# --- Exchange processes
#
p$comp_ilt:
.co10:
        lda     pcnrdy,r3               # Set this process to not ready
        ld      K_xpcb,r15
        stob    r3,pc_stat(r15)
        call    K$qxchang               # Exchange processes
        ld      K_ii+ii_time,r8         # Get current timer
        addo    P_COMP_MAX_TIME,r8,r8   # r8 = next timer to do Xchange to
                                        #  prevent other task starvation
#
# --- Close down queue
#
.co20:
        lda     P_comp_qu,r11           # Get completion queue pointer
        ld      qu_head(r11),r12        # Get queue head
        cmpobe.f 0,r12,.co10            # Jif none
#
        ld      qu_tail(r11),r13        # Get tail
        ld      qu_qcnt(r11),r14        # Get count
        movt    0,r4
        stt     r4,qu_head(r11)         # Clear queue head, tail and count
        mov     r12,g1                  # Isolate 1st queued ILT
        ld      il_fthd(g1),r15         # Get next ILT
#
# --- Process next completion request ---------------------------------
#
.co30:
#
# --- Activate initiator if a request is available
#
        ld      p_qhd(g1),g3            # Get corresponding DEV
        cmpobe  g3,0,.co90              # Jif DEV does not exist.
        lda     P_exec_qu,r4            # Check if DEV assigned to request
        ld      r_prp-ILTBIAS(g1),g10   # Get PRP
        ldob    pr_rstatus(g10),g12     # Get request status
.if     DEBUG_FLIGHTREC_P
        shlo    24,g12,r5               # Shift PRP status to byte 3
        ldconst frt_p_comp,r3           # Physical completion code
        or      r5,r3,r3                # Type in byte 0
        st      r3,fr_parm0             # Physical - p$comp_ilt
        st      g1,fr_parm1             # ILT
        st      g10,fr_parm2            # PRP
        st      g3,fr_parm3             # DEV
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_P
        cmpobe.f g3,r4,.co90            # Jif DEV not assigned to request
        cmpobe.t ecok,g12,.co50         # Jif request status is OK

.if 0
#--- SPECIAL -- ignore check conditions from ISE of 3/11 (medium error, 11 is data unrecoverable from SAN).
c if (((PRP*)g10)->scsiStatus == SCS_ECHK) {
c   if ((((PRP*)g10)->sense[2] & 0xff) == SCK_MEDIUM) { # key = 03, MEDIUM error
c       if (((PRP*)g10)->func == PRP_INPUT) {
c          if (((PRP*)g10)->pSGL != 0) {
c             print_scsi_cmd((DEV*)g3, (PRP*)g10, "SCSI Check Condition, medium error-3");
c             g12 = (UINT32)((((PRP*)g10)->pSGL) + 1);
c             if (((SGL_DESC*)g12)->addr != 0) {
c                 memset(((SGL_DESC*)g12)->addr, 0, ((SGL_DESC*)g12)->len);
c             }
c          }
c       }
c       g12 = ecok;
c       ((PRP*)g10)->reqStatus = g12;   # Make it appear okay.
        b       .co50                   # Pretend request status is really okay.
c   }
c }
.endif # 0

#
# --- Device request status is not OK.
#     Check whether recovery is feasible and desired.
#
        PushRegs(r3)
c       g0 = PHY_CheckForRetry((struct ILT*)g1,(struct DEV*)g3,(struct PRP*)g10);
        PopRegs(r3)
#
        ldob    dv_chn(g3),r3           # r3 = Port number
        cmpoble MAXCHN,r3,.co62         # Jif DEVice not attached to port
        cmpobne 0,g0,.co52              # JIf task is being retried
        b       .co51
#
# --- Device request status is OK.
#
.co50:
        ldob    pr_channel(g10),r3      # r3 = Port number
        ldob    dv_unavail(g3),r4
        bbc.t   r3,r4,.co51             # jif port is available
        clrbit  r3,r4,r4
        stob    r4,dv_unavail(g3)       # Indicate port as available
        ldconst 0,r3
        stos    r3,dv_recoveryflags(g3)

.co51:
        ld      dv_qcnt(g3),r3          # Get queue count
        cmpobne.t 0,r3,.co52            # If not empty, jump

        ld      dv_failq_hd(g3),r3      # Is there anything on failed queue?
        cmpobe  0,r3,.co60              # Jif failed queue empty
#
        ldob    dv_flags(g3),r3         # Check if the drive had
        bbc.t   dvqueuefull,r3,.co52    #   previously returned queue full
#
        clrbit  dvqueuefull,r3,r3
        stob    r3,dv_flags(g3)         # Clear queue full flag
        ldconst 0,r3
        stos    r3,dv_wait(g3)          # Clear the wait count
#
.co52:
        ldob    dv_chn(g3),r6           # Get port number
        ld      P_chn_ind[r6*4],r6      # Get CHN ptr
        ld      ch_init_pcb(r6),r3      # Get initiator PCB
        ldob    pc_stat(r3),r5          # Get process status
        cmpobe.t pcnrdy,r5,.co55        # Jif not ready
#
        ldconst TRUE,r3                 # Resync initiator
        stob    r3,ch_sync(r6)
        b       .co60
#
.co55:
        ldconst pcrdy,r5                # Set process ready
.ifdef HISTORY_KEEP
c CT_history_pcb(".co55 setting ready pcb", r3);
.endif  # HISTORY_KEEP
        stob    r5,pc_stat(r3)
#
# --- Log event if error has occurred
#
.co60:
        cmpobe.t ecok,g12,.co75         # Jif OK
        cmpobne  0,g0,.co200            # Jif task is being retried
#
.co62:
#
        cmpobne.t ecnonxdev,g12,.co65   # Jif not nonx device
#
        ldob    pr_flags(g10),r3        # Get PRP flags
        bbs     prSNX,r3,.co75          # Jif suppress nonexistent device
#
        ld      dv_pdd(g3),r3           # Get associated PDD
        cmpobe  r3,0,.co75              # check for null
#
        ldob    pd_devstat(r3),r9       # Get status
        cmpobe.t pdnonx,r9,.co75        # Jif already non-existent
#
.co65:
        ld      dv_pdd(g3),r3           # Get associated PDD
        ldob    pd_flags(r3),r3         # Get PDD flags
        bbs     pdbebusy,r3,.co75       # Jif device busy
#
# NOTE: The message is <80 bytes, and thus is copied by L$send_packet.
c       g0 = (UINT32)TmpStackMessage;

        ldconst mleshortscsi,r3         # Event code
        st      r3,mle_event(g0)        # Store as word to clear other bytes
#
        stob    g12,ess_prpstat(g0)     # Set up PRP status
        ldob    pr_sstatus(g10),r3      # Set up SCSI status
        stob    r3,ess_scsistat(g0)
        ldob    pr_qstatus(g10),r4      # Set up Qlogic status
        stob    r4,ess_qstatus(g0)
#
        ldob    pr_channel(g10),r4      # Set up port number
        stob    r4,ess_port(g0)
        ldos    pr_lun(g10),r4          # Set up LUN
        stob    r4,ess_lun(g0)
        ld      pr_id(g10),r5           # Set up ID
        st      r5,ess_id(g0)
#
        ld      dv_pdd(g3),r7           # Get associated PDD
        cmpobe  r7,0,.co65_1            # check for null
        ldos    pd_pid(r7),r4
        b      .co65_2
#
.co65_1:
        ldconst 0xffff,r4
.co65_2:
        stos    r4,ess_pid(g0)          #store pid if it exists
#
        ldl     dv_wwn(g3),r4           # Set up WWN
        stl     r4,ess_wwn(g0)
#
        ldq     pr_cmd(g10),r4          # Set up SCSI command
        stq     r4,ess_cdb(g0)
#
        ldconst esslen,r10              # Size of short SCSI packet, used later
        cmpobne.t 2,r3,.co68            # Jif no ck cond status (no sense info)
#
        ldconst mlelongscsi,r4          # Change to long SCSI log event
        stos    r4,mle_event(g0)
        ldconst esllen,r10              # Size of long SCSI packet, used later
        ldq     pr_sense+16(g10),r4     # Copy sense data
        stq     r4,esl_sense+16(g0)
        ldq     pr_sense(g10),r4
        stq     r4,esl_sense(g0)
#
        extract 16,4,r4                 # Isolate sense key
        cmpobe.f 5,r4,.co75             # Jif illegal command - don't log
#
        extract 0,8,r7                  # Isolate additional sense code
        ldconst 0x5d,r3                 # Get failure prediction threshold
        cmpobe.f r7,r3,.co75            # Jif smart - don't log
#
        ldob    pr_flags(g10),r3        # Get PRP flags
        bbc     prSPS,r3,.co67          # Jif no pre spinup
        cmpobne.t 6,r4,.co67            # Jif not unit attention
        ldconst 0x29,r3                 # ASC for power on/reset
        cmpobe  r7,r3,.co75             # Jif sense code is power on/reset
#
.co67:
        cmpobne.t 2,r4,.co70            # Jif not (not ready) - log
#
# --- If 02/35/02 sense code happens from a Receive Diag cmd,
#     don't log the message
#
        ld      pr_sense+12(g10),r7     # Get additional sense code
#c       fprintf(stderr, "p$comp_ilt: Sense code = 2, sense key/qual = %08lx\n", r7);
        extract 0,16,r7                 # Isolate lower 2 bytes
        ldconst 0x0235,r3               # ASC for Enclosure services unavailable
        cmpobne r7,r3,.co67_1           # Jif sense code is not SES unavail
        ldob    pr_cmd(g10),r7          # Get SCSI command
#c       fprintf(stderr, "p$comp_ilt: SCSI command = %08lx\n", r7);
        ldconst 0x1C,r3                 # Receive Diagnostic SCSI command
        cmpobe  r3,r7,.co75             # Jif Receive Diag command
.co67_1:
        ldob    pr_cmd(g10),r3          # Get SCSI command
        cmpobe.t 0,r3,.co75             # Jif test unit ready - don't log
#
.co68:
        ldob    pr_cmd(g10),r3          # Get SCSI command
        cmpobne.t 0x1b,r3,.co70         # Jif not start unit - log
        ldob    pr_cmd+4(g10),r3        # Get CDB byte 4 to examine start bit
        bbc     0,r3,.co70              # Jif start bit not set
        ldconst mlespinupfailed,r4      # Change to spinup failed log event
        stos    r4,mle_event(g0)
#
.co70:
c       MSC_LogMessageStack(&TmpStackMessage[0], r10);
#
# --- Cleanup tag
#
.co75:
#
# --- Cleanup backend tag status
#
c       r4 = ((ILT*)g1)->phy_io.phy_tag; # Get tag for this op
        st      0,dv_tag_ilt(g3)[r4*4]  # Clear ILT for this tag in DEV

c       ((DEV *)g3)->tMapAsgn &= ~(1 << r4);    # Clear bit.
#
# --- Check for system request (joined)
#
        ld      p_jth(g1),r10           # Get join thread
        cmpobne.f 0,r10,.co100          # Jump if defined
#
# --- Complete this non-system request --------------------------------
#
        call    p$upstats               # Update statistics
.co90:
c       record_physical(FR_PRP_COMPLETE, (void *)g10, g12);
        call    K$comp                  # Complete this request
#
#       Adjust outstanding request count
#
        ld      P_orc,r3                # Adjust outstanding request count
        subo    1,r3,r3
        st      r3,P_orc
        b       .co200
#
# --- Process completion of system request ----------------------------
#
.co100:
        mov     g1,r3                   # Save system request
        ld      r_prp-ILTBIAS(g1),r4    # Get system PRP
        ldob    pr_rstatus(r4),r6       # Get PRP request status
        ldob    pr_sstatus(r4),r7       # Get PRP SCSI status
        ldob    pr_rsbytes(r4),r5       # Get request sense length
        lda     pr_sense(r4),r9         # Get req sense source
#
# --- Process next user request
#
.co110:
        mov     r10,g1                  # g1 = user request
        ld      r_prp-ILTBIAS(g1),r11   # Get user PRP
        stob    r6,pr_rstatus(r11)      # Update request status
        cmpobe.t ecok,r6,.co130         # Jump if ok status
#
# --- Copy over error status
#
        stob    r7,pr_sstatus(r11)      # Update SCSI status
#
        ldob    pr_rsbytes(r11),r12     # Get receiving req sense length
        lda     pr_sense(r11),r13       # Get req sense destination
        cmpoble r12,r5,.co120           # Jump if destination length ok
#
        mov     r5,r12                  # Use destination length
.co120:
        subo    1,r12,r12               # Adjust length
        ldob    (r9)[r12*1],r14         # Move next byte
        stob    r14,(r13)[r12*1]
        cmpobne.t 0,r12,.co120          # Jump if more to go
#
# --- Complete this user request
#
.co130:
        ld      il_fthd(g1),r10         # Get link to next user request
        mov     g3,r11                  # Save DEV pointer
        call    p$upstats               # Update statistics
c       record_physical(FR_PRP_COMPLETE, (void *)g10, g12);

        call    K$comp                  # Complete this request
#
#       Adjust outstanding request count
#
        ld      P_orc,g1                # Adjust outstanding request count
        subo    1,g1,g1
        st      g1,P_orc
        mov     r11,g3                  # Restore g3
        cmpobe  0,r10,.co140            # Jump if no more requests
#
#   Determine if this task has exceeded the time allowed to run.  If so,
#       do a context switch.
#
        ld      K_ii+ii_time,r11        # Get the current timer
        cmpobl  r11,r8,.co110           # Jif still more time to handle queue
        call    K$xchang                # Exchange processes
        ld      K_ii+ii_time,r8         # Get current timer
        addo    P_COMP_MAX_TIME,r8,r8   # r8 = next timer to do Xchange to
                                        #  prevent other task starvation
        b       .co110                  # Process more requests
#
# --- Release system ILT/PRP/SGL
#
.co140:
        lda     -ILTBIAS(r3),g1         # Release system ILT/PRP/SGL
        call    M$rip
#
# --- Link to next request
#
.co200:
        cmpobe.t 0,r15,.co20            # Jif end of queue
#
#   Determine if this task has exceeded the time allowed to run.  If so,
#       do a context switch.
#
        ld      K_ii+ii_time,r3         # Get the current timer
        cmpobl  r3,r8,.co210            # Jif still more time to handle queue
        call    K$qxchang               # Exchange processes
        ld      K_ii+ii_time,r8         # Get current timer
        addo    P_COMP_MAX_TIME,r8,r8   # r8 = next timer to do Xchange to
                                        #  prevent other task starvation
.co210:
        mov     r15,g1                  # Move next to current ILT
        ld      il_fthd(r15),r15        # Get next ILT
        b       .co30
# End of p$comp_ilt ***********************************************************
#
#**********************************************************************
#
#  NAME: p$upstats
#
#  PURPOSE:
#       To provide a common means of updating the PDD statistics
#       following the completion of a physical I/O operation.
#
#  DESCRIPTION:
#       The sample period total request and total sector counts are
#       updated within the DEV structure.  The statistics within the
#       PDD structure are then updated.
#
#  CALLING SEQUENCE:
#       call    p$upstats
#
#  INPUT:
#       g1 = ILT
#       g3 = DEV
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
p$upstats:
#
# --- Update sample period totals in DEV
#
        ld      r_prp-ILTBIAS(g1),r15   # Get PRP
        ldob    pr_func(r15),r9         # Get function code
        cmpobe  prctl,r9,.ups10         # Jif no data xfer.
        ldt     dv_sprc(g3),r12         # Get sample period request
                                        #  count, sample period sector
                                        #  count and PDD ptr
!       ldl     pr_sda(r15),r4          # Get SDA
!       ldl     pr_eda(r15),r6          # Get EDA
        addo    1,r12,r12               # Update sample period request count
c       *((UINT64*)&r8) = *((UINT64*)&r6) - *((UINT64*)&r4) + r13;;
c       r13 = *((UINT64*)&r8) & 0xffffffffULL;
        stl     r12,dv_sprc(g3)
#
# --- Update R/W statistics in PDD
#
        cmpo    proutput,r9             # Check for output operation
        lda     pd_rreq(r14),r8         # Get read req count ptr
        lda     pd_wreq(r14),r7         # Get write req count ptr
        sele    r8,r7,r6                # Set correct count ptr
        ld      (r6),r3                 # Bump total count
        lda     1(r3),r3
        st      r3,(r6)
#
# --- Exit
#
.ups10:
        ret
# End of p$upstats ************************************************************
#
#************************************************************************
#
#  NAME: p$wakeup
#
#  PURPOSE:  Wakes p$init when an ILT exists in a device queue.
#
#  DESCRIPTION:  Check the device ILT queue count.  Check the failed
#                device queue for an ILT.  Wakes up p$init by either
#                making this process ready, or if it already is ready,
#                setting the sync bit, so all devices get processed.
#
#  CALLING SEQUENCE:
#       call    p$wakeup
#
#  INPUT:
#       g0 = DEV
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#***********************************************************************
p$wakeup:
        ld      dv_qcnt(g0),r3          # Get queue count
        cmpobne 0,r3,.pwp10             # Jif queue not empty
#
        ld      dv_failq_hd(g0),r3      # Get failed queue head
        cmpobe  0,r3,.pwp30             # Jif failed queue empty
#
.pwp10:
        ldob    dv_flags(g0),r3
        clrbit  dvqueuefull,r3,r3
        stob    r3,dv_flags(g0)         # Clear queue full flag
#
        ldob    dv_chn(g0),r3           # Get port number
c if (r3 >= MAXCHN) fprintf(stderr,"<INVDEVPORT>-.pwp10 - dv_chn out of range %ld\n", r3);
        cmpoble MAXCHN,r3,.pwp30        # Jif DEVice port not valid
        ld      P_chn_ind[r3*4],r6      # Get CHN ptr
        ld      ch_init_pcb(r6),r3      # Get initiator PCB
        ldob    pc_stat(r3),r5          # Get process status
        cmpobe  pcnrdy,r5,.pwp20        # Jif not ready
#
        ldconst TRUE,r3                 # Resync initiator
        stob    r3,ch_sync(r6)
        b       .pwp30
#
.pwp20:
        ldconst pcrdy,r5                # Set process ready
.ifdef HISTORY_KEEP
c CT_history_pcb(".pwp20 setting ready pcb", r3);
.endif  # HISTORY_KEEP
        stob    r5,pc_stat(r3)
#
.pwp30:
        ret
#
#**********************************************************************
#
#  NAME: p$rescanDevice
#
#  PURPOSE:  The function initiates the rescanning of physical devices.
#
#  DESCRIPTION: The monitor loop process is started.  This process
#               notifies online of the list devices attached to
#               the back end.
#
#  CALLING SEQUENCE:
#       call    p$rescanDevice
#
#  INPUT:
#       g0 - Scan Type 0 = rescan existing devices
#                      1 = rescan LUNS
#                      2 = rediscover devices
#
#  OUTPUT:
#       g1 = status
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
P$rescanDevice:
        PushRegs(r3)                    # save all the registers
        call   F_rescanDevice
        mov     g0,r4                   # save return value in temp
        PopRegsVoid(r3)                 # Restore all registers
        mov     r4,g1                   # update return value
        ret
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
