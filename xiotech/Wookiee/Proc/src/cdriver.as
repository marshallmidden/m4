# $Id: cdriver.as 159305 2012-06-16 08:00:46Z m4 $
#******************************************************************************
#
#  NAME: cdriver.as
#
#  PURPOSE:
#
#   To provide a general purpose interface with the Fibre Channel
#   for multiple independent uses including a MAGNITUDE translation
#   device driver, a MAGNITUDE-to-MAGNITUDE device driver, etc...
#
#  FUNCTIONS:
#
#       CD$init        - Channel driver initialization
#       C_recv_scsi_io - Receive a SCSI event and pass it on to the
#                        appropriate device driver for processing
#       C$regimt       - Register IMT
#       C$receive_srp  - Process SRPs received from MAGNITUDE
#       C$mmcput       - Send message to MMC common subroutine
#       C$findTarget   - Find target for specified virtual port
#
#       This module employs one process:
#
#       cd$exec        - Channel driver executive
#
#  Copyright (c) 1998 - 2009 Xiotech Corporation.  All rights reserved.
#
#******************************************************************************
#
# --- local equates -----------------------------------------------------------
#
        .set    cd_statwait,999         # timed wait (in msec.) for CIMT local
                                        #  stats processing to be performed
#
# --- Channel driver event table index definitions
#
        .set    cde_undef,0             # Undefined event code
        .set    cde_cdb,4               # SCSI CDB received
        .set    cde_imno,8              # Immediate notify message
        .set    cde_onl,12              # Interface online
        .set    cde_offl,16             # Interface offline
#
# --- global function declarations
#
        .globl  C_recv_scsi_io          # Receive a SCSI event and pass it on to
                                        #  the appropriate device driver for
                                        #  processing
        .globl  CD$init                 # Channel driver initialization
        .globl  C$regimt                # Register IMT
        .globl  C$receive_srp           # Process SRPs received from MAGNITUDE
        .globl  C$mmcput                # Send packet to MMC common subroutine
        .globl  C$findTarget            # Find target for specified virtual port
#
# --- global data declarations
#
        .globl  cimtDir                 # Base address of CIMT directory
        .globl  C_imt_head               # Inactive IMT list head pointer
        .globl  C_imt_tail               # Inactive IMT list tail pointer
        .globl  MAGD_SCSI_WHQL          # SCSI WHQL compliance
#
# --- global usage data definitions
#
        .data
        .align  2                       # align just in case
#
# --- Trace support
#
.ifdef TRACES
C_temp_trace:
        .space  trr_recsize,0           # trace record build area
.endif # TRACES
#
        .align  2                       # align just in case
#
# --- Statistical counters
#
C_undef_event:
        .word   0                       # # undefined events counter
#
# --- MMC message management area
#
c_mmcput_flg:
        .byte   0                       # MMC message discard flag
c_mmcput_dis:
        .byte   20                      # MMC message discard threshold
        .byte   0                       # unused
#
# --- Channel driver event handler tables
#
C_event_tbl1:                           # No CIMT event table
        .word   c$undef1                # Event undefined
        .word   c$cdb1                  # SCSI CDB received
        .word   c$imno1                 # Immediate notify message
        .word   c$onl1                  # Interface online
        .word   c$offl1                 # Interface offline
        .word   c$plo1                  # Port logged out
        .word   c$abort1                # Abort task
        .word   c$rinit1                # Reset and initialize port
        .word   c$disvp1                # Disable Virtual port
#
C_event_tbl2:                           # CIMT uninitialized event table
C_event_tbl3:                           # CIMT offline event table
        .word   c$undef2                # Event undefined
        .word   c$cdb2                  # SCSI CDB received
        .word   c$imno2                 # Immediate notify message
        .word   c$onl2                  # Interface online
        .word   c$offl2                 # Interface offline
        .word   c$plo2                  # Port logged out
        .word   c$abort2                # Abort task
        .word   c$rinit1                # Reset and initialize port
        .word   c$disvp1                # Disable Virtual port
#
C_event_tbl4:                           # CIMT online event table
        .word   c$undef4                # Event undefined
        .word   c$cdb4                  # SCSI CDB received
        .word   c$imno4                 # Immediate notify message
        .word   c$onl4                  # Interface online
        .word   c$offl4                 # Interface offline
        .word   c$plo4                  # Port logged out
        .word   c$abort4                # Abort task
        .word   c$rinit1                # Reset and initialize port
        .word   c$disvp1                # Disable Virtual port
#
        .align  4                       # quad align the stats
        .globl  im_peragg
im_peragg:
        .space  imst_size,0             # imst stats structure for aggregate
                                        #  of all IMTs
#
.if     ERRLOG
#
# --- Error log management data structure
#
        .globl  C_eflg                  # Error log enabled flags
        .globl  C_elog_current          # Error log current record pointer
        .globl  C_elog_beg              # Error log beginning record pointer
        .globl  C_elog_end              # Error log ending record pointer
        .globl  C$geterrrec             # Get error log record
#
.ifdef TRACES
.if TRACE_EL
        .globl  C$traceerr              # Copy traces into error log record
.endif # TRACE_EL
.endif # TRACES
#
C_eflg:
        .word   0                       # Error log enabled flags
C_elog_current:
        .word   0                       # Error log current record pointer
C_elog_beg:
        .word   0                       # Error log beginning record pointer
C_elog_end:
        .word   0                       # Error log ending record pointer
#
.endif                          # ERRLOG
#
# --- beginning of code -------------------------------------------------------
#
#
#******************************************************************************
#
#  NAME: CD$init
#
#  PURPOSE:
#
#       Initializes the channel driver environment.
#
#  DESCRIPTION:
#
#       The executive process is established and made ready
#       for execution.
#
#  CALLING SEQUENCE:
#
#       call    CD$init
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
#******************************************************************************
#
        .text
#
CD$init:
#
# --- Clear out local data structures
#
        movl    0,r4
        stl     r4,C_imt_head           # clear IMT queue pointers
        st      r4,C_undef_event        # clear undef. event counter
        lda     cimtDir,r9              # r9 = CIMT directory pointer
c       memset((void*)r9, 0, CIMTMAX*4);
#
# --- Establish executive process
#
        movl    g0,r14                  # save g0-g1
        lda     cd$exec,g0              # establish executive process
        lda     CDEXECPRI,g1
c       CT_fork_tmp = (ulong)"cd$exec";
        call    K$fork
        movl    r14,g0                  # restore g0-g1
#
        ret
#
#******************************************************************************
#
#  NAME: cd$exec
#
#  PURPOSE:
#
#       Channel driver executive process.
#
#  DESCRIPTION:
#
#       This allocates and initializes the CIMT data then loops every second
#       to copy the periodic statistics from in the in-progress area to the
#       static area for data collection.
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
#       All registers can be destroyed.
#
#******************************************************************************
#
cd$exec:
#
# --- Allocate and initialize CIMTs
#
        mov     CIMTMAX,r4              # r4 = max. # CIMTs to initialize
        lda     cimtsize,r5             # r5 = size of each CIMT
        mulo    r4,r5,g0                # g0 = size of CIMT allocation
c       g0 = s_MallocC(g0, __FILE__, __LINE__);
        lda     cis_init,r7             # r7 = CIMT state
        mov     0,r6                    # r6 = CIMT #
        lda     C_event_tbl2,r8         # r8 = cdriver event table
        lda     cimtDir,r9              # r9 = base address of CIMT dir.
.cex10:
        st      g0,(r9)[r6*4]           # save CIMT address in directory
        stob    r6,ci_num(g0)           # save CIMT #
        stob    r7,ci_state(g0)         # save CIMT state
        st      r8,ci_ehand(g0)         # save cdriver event handler table
#
.if     MAG2MAG
        stob    r7,ci_istate(g0)        # save initiator state
.endif  # MAG2MAG
#
        lda     1(r6),r6                # inc. CIMT #
        addo    r5,g0,g0                # inc. to next CIMT
        cmpobne CIMTMAX,r6,.cex10       # Jif more CIMTs to initialize
#
.ifdef TRACES
#
# --- Allocate trace areas and initialize
#
        lda     TRACESIZE,r8            # r8 = trace area size
        mov     CIMTMAX,r4              # r4 = # trace areas to initialize
        lda     cimtDir,r9              # r9 = base address of CIMT dir.
        mov     0,r6                    # r6 = CIMT dir. index trace area
                                        #  is being initialized
        ldconst DEF_TFLG,r15            # r15 = default trace flags
        ldconst de_trace0,r12           # r12 = offset into DDR
.cex20:
c       g0 = s_MallocC(r8, __FILE__, __LINE__); # Allocate and clear trace memory
        mov     g0,r11                  # save buffer pointer
        addo    r6,r9,r10               # r10 = CIMT_dir pointer
        ld      (r10),r10               # r10 = CIMT trace area is being
                                        #  initialized for
        st      g0,ci_curtr(r10)        # save trace area pointer as current
        st      g0,ci_begtr(r10)        # save trace area pointer as beginning
        lda     TRACESIZE-trr_recsize(g0),g0 # g0 = end trace area pointer
        st      g0,ci_endtr(r10)        # save trace area pointer as ending
        stos    r15,ci_tflg(r10)        # save trace flag settings
        stos    r15,ci_dftflg(r10)      # save default trace flag settings
#
# --- Initialize a Debug Data Retrieval (DDR) itrace entry
#
        mov     r12,g0                  # Load DDR table offset
        mov     r11,g1                  # Load trace address
        mov     r8,g2                   # Load trace size
c       M_addDDRentry(g0, g1, g2);
        addo    1,r12,r12               # Increment DDR table offset
#
        subo    1,r4,r4                 # dec. CIMT counter
        addo    4,r6,r6                 # inc. CIMT_dir index
        cmpobne 0,r4,.cex20             # Jif more CIMTs to initialize trace
                                        #  area for
.endif # TRACES
#
.if     ERRLOG
#
# --- Allocate error log area and initialize
#
.ifdef TRACES
  .if TRACE_EL
        lda     err_recsize+elog_trsize,r9 # r9 = size of each error log record
  .else   # TRACE_EL
        lda     err_recsize,r9          # r9 = size of each error log record
  .endif  # TRACE_EL
.else   # TRACES
        lda     err_recsize,r9          # r9 = size of each error log record
.endif  # TRACES
#
        ldconst elog_num,r8             # r8 = # error log records
        mulo    r8,r9,r8                # r8 = error log area size
        mov     r8,g0                   # g0 = memory allocation size
        ldconst DEF_EFLG,r15            # r15 = default error log flags
c       g0 = s_MallocC(g0, __FILE__, __LINE__); # Allocate and clear error log memory
        st      g0,C_elog_current       # save beginning record pointer as
                                        #  current
        st      g0,C_elog_beg           # save beginning record pointer
        addo    r8,g0,g0                # g0 = end of error log area
        subo    r9,g0,g0                # g0 = end record pointer
        st      g0,C_elog_end           # save ending record pointer
        st      r15,C_eflg              # save error log flag settings
#
.endif  # ERRLOG
#
# --- Initialize HBA Statistics counters
#
        PushRegs(r12)                   # save g registers (g14 = 0)
        call    InitHBAStats
        PopRegsVoid(r12)                # restore g registers
#
# --- Main Exec Loop ----------------------------------------------------------
#
# --- Copy the in-progress periodic stats to the static periodic stats area.
#
        movq    0,r12                   # always zero
        mov     1,g9                    # loop counter
.cex1000:
        ldconst cd_statwait,g0          # g0 = process delay time (msec.)
        call    K$twait                 # delay task
#
        mov     CIMTMAX,r4              # r4 = max. # CIMTs to process
        lda     cimtDir,r5              # r5 = pointer to CIMT directory
        ld      K_ficb,r3               # load K_ficb
        ld      fi_cserial(r3),r3       # load system serial number
        ldconst 0xff00000f,r7           # load mask to get CNC
        andnot  r7,r3,r3                # mask CNC part of serial number
#
        lda     im_peragg,g10           # point to aggregate periodic stats
        stq     r12,imst_cmds(g10)      # clear all stats counts
        stq     r12,imst_writes(g10)
        stq     r12,imst_reads(g10)
#
# --- Walk through each CIMT
#
.cex1010:
        ld      (r5),g0                 # g0 = CIMT to process
        subo    1,r4,r4                 # dec. CIMT count
        addo    4,r5,r5                 # inc. CIMT pointer
        cmpobe  0,g0,.cex1100           # Jif no CIMT defined
#
# --- Walk through each IMT on the active list
#
        ld      ci_imthead(g0),r6       # get first IMT on active list
.cex1015:
        cmpobe  0,r6,.cex1100           # Jif no IMTs on active list
#
# --- Copy in-progress periodic stats to the periodic stats area
#
        lda     im_stinprog(r6),r7      # point to in-progress stats area
        lda     im_stper(r6),r8         # point to periodic stats area
        lda     im_peragg,g10           # point to periodic aggregate stats area
        ldconst imst_size/8-1,r9        # load loop count
                                        # assumes multiple of 8 bytes in struct
.cex1017:
        ldl     (r7),r10                # get in-progress values
        ldl     (g10),g12               # get aggregate values
        stl     r10,(r8)                # save them in periodic area
        stl     r14,(r7)                # zero out the in-progress count
#
        cmpo    1,0                     # Clear carry
        addc    r10,g12,g12             # calc aggregate periodic stats
        addc    r11,g13,g13
        stl     g12,(g10)               # save updated counts
#
        addo    8,r7,r7                 # increment in-progress pointer
        addo    8,r8,r8                 # increment periodic pointer
        addo    8,g10,g10               # increment aggregate periodic pointer
        cmpdeco 0,r9,r9                 # decrement loop count
        bne     .cex1017                # branch if more to do
#
        ld      im_link(r6),r6          # get next IMT on active list
        b       .cex1015                # do the next IMT
#
.cex1100:
#
        cmpobne 0,r4,.cex1010           # Jif more CIMTs to process
#
# --- Do once per second tasks that are not time critical -- and we do not
#     want to create another process to do. See djk_cm.as, cm$pollexec for
#     the BE equivalent (at the end).
#
        PushRegs(r4)                    # save g registers (g14 = 0)
        call    CalcHBAStats            # Calculate HBA Statistics
c       DL_check_delayed();             # Check for delayed Path-Lost messages.
c       MAG_check_delayed();            # Delayed scsi reservation conflict message processing.
.if 0   # There is nothing to do in the FE for Direct Memory Copy -- yet.
c       Process_DMC_delayed();          # Do routine Process_DMC every 5 seconds.
.endif  # 0
        PopRegsVoid(r4)                 # restore g registers
#
# --- Clear the mag and host sense error log counters for MagDriver
#
        subo    1,g9,g9                 # Decrement loop counter
        cmpobne 0,g9,.cex1000           # Log every n seconds
        ldconst 15,g9                   # reset n
#
        st      r12,magerr_events       # Clear the current counters
        st      r13,host_events
#
        b       .cex1000                # go wait for next stats window
#
#******************************************************************************
#
#  NAME: C_recv_scsi_io
#
#  PURPOSE:
#
#       Receives SCSI events, determines which elements they affect, and notifies
#       the appropriate elements that they occurred. Also handles SCSI events
#       which no other element has identified to handle.
#
#  DESCRIPTION:
#
#       This routine is called when a SCSI event has been received.  It is
#       passed an ILT pointed to by register g1, which contains the following
#       parameters:
#
#       il_misc = Pointer to param structure [see iltdefs.inc]
#
#       Param structure varies depending on command byte.
#
#       <scsicmd> - SCSI CDB format
#       <immncmd> - Immediate Notify format
#
#
#  CALLING SEQUENCE:
#
#       call    C_recv_scsi_io
#
#  INPUT:
#
#       g1 = ILT at Channel Driver nest.
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
C_recv_scsi_io:
        lda     0,g13                   # Set g13 to zero for Magdriver
        movq    g4,r12                  # save g4-g7
        st      g13,il_cr(g1)           # clear completion routine field
        ld      il_misc(g1),g7          # g7 = param structure address
        ldob    idf_ci(g7),r4           # r4 = interface #
        st      g13,idf_imt(g7)         # clear the IMT pointer
        cmpobg  CIMTMAX,r4,.rs20        # Jif valid interface #
.rs10:
        mov     0,g4                    # g4 = CIMT (null)
        lda     C_event_tbl1,r11        # r11 = event handler table
        b       .rs100                  # and process using default table
#
.rs20:
        lda     cimtDir,r11             # r11 = CIMT directory pointer
        shlo    2,r4,r4                 # r4 * 4
        addo    r4,r11,r11
        ld      (r11),g4                # g4 = CIMT address
        cmpobe   0,g4,.rs10             # Jif no CIMT defined
        ld      ci_ehand(g4),r11        # r11 = event handler table
#
.ifdef TRACES
#
# --- Trace incoming event if appropriate
#
        ldos    ci_tflg(g4),r4          # r4 = trace flags
        lda     C_temp_trace,r10        # r10 = trace record build pointer
        ld      ci_curtr(g4),r3         # r3 = current trace record pointer
        bbc     tflg_XL$recv,r4,.rs100  # Jif event trace disabled
        ldob    ci_num(g4),r6           # r6 = chip instance
        ldconst trt_XL$recv,r5          # r5 = trace record type code
        ldos    idf_exid(g7),r7         # r7 = exchange ID
        stob    r5,trr_trt(r10)         # save trace record type code
        ldob    idf_cmd(g7),r8          # r8 = command byte
        stob    r6,trr_ci(r10)          # save chip instance
        ldob    idf_taskf(g7),r5        # r5 = task flags
        stos    r7,trr_exid(r10)        # save chip instance
        ldob    sctaskc(g7),r4          # r4 = task code byte
        stob    r8,4(r10)               # save command byte
        ldos    idf_lun(g7),r6          # r6 = LUN
        stob    r5,5(r10)               # save task flags
        ld      idf_init(g7),r7         # r7 = initiator ID
        stob    r4,6(r10)               # save task code byte
c       r5 = get_tsc_l() & ~0xf;        # Get free running bus clock.
        stob    r6,7(r10)               # save LUN
#        st      g1,12(r10)              # save ILT address
        st      r7,8(r10)               # save initiator ID
        st      r5,12(r10)              # save timestamp
        ldq     (r10),r4                # r4-r7 = trace record
        ldl     ci_begtr(g4),r8         # r8 = trace area beginning pointer
                                        # r9 = trace area ending pointer
        lda     trr_recsize(r3),r10     # r10 = next trace record pointer
        stq     r4,(r3)                 # save trace record in CIMT trace area
        cmpoble r10,r9,.rs20a         # Jif trace record pointer has not
                                        #  exceeded end of trace area
        ldos    ci_tflg(g4),r9          # r9 = trace flags
        mov     r8,r10                  # next trace record pointer = beginning
                                        #  trace record pointer
        bbc     tflg_wrapoff,r9,.rs20a  # Jif wrap off flag not set
        mov     0,r9                    # turn off traces due to trace area
                                        #  wrapped.
        stos    r9,ci_tflg(g4)
.rs20a:
        st      r10,ci_curtr(g4)        # save new current trace record pointer
#
.endif # TRACES
#
.rs100:
        ldob    idf_cmd(g7),g5          # g5 = event code
        cmpobne immncmd,g5,.rs200     # Jif not immediate notify command
#
.ifdef TRACES
#
# --- Trace incoming event if appropriate
#
        ldos    ci_tflg(g4),r4          # r4 = trace flags
        lda     C_temp_trace,r10        # r10 = trace record build pointer
        ld      ci_curtr(g4),r3         # r3 = current trace record pointer
        bbc     tflg_c$imno4,r4,.rs105z # Jif event trace disabled
        ldob    ci_num(g4),r6           # r6 = chip instance
        ldconst trt_c$imno4,r5          # r5 = trace record type code
        ldos    idf_exid(g7),r7         # r7 = exchange ID
        stob    r5,trr_trt(r10)         # save trace record type code
        ldos    idf_taskf(g7),r8        # r8 = flags value
        stob    r6,trr_ci(r10)          # save chip instance
        ldos    instatus(g7),r5         # r5 = status value
        stos    r7,trr_exid(r10)        # save chip instance
        ld      idf_init(g7),r7         # r7 = initiator ID
        st      r7,8(r10)               # save initiator ID
c       r7 = get_tsc_l() & ~0xf;        # Get free running bus clock.
        stos    r8,4(r10)               # save flags value
        stos    r5,6(r10)               # save status value
        ldt     (r10),r4                # r4-r6 = (add r7 above) trace record
        ldl     ci_begtr(g4),r8         # r8 = trace area beginning pointer
                                        # r9 = trace area ending pointer
        lda     trr_recsize(r3),r10     # r10 = next trace record pointer
        stq     r4,(r3)                 # save trace record in CIMT trace area
        cmpoble r10,r9,.rs105a        # Jif trace record pointer has not
                                        #  exceeded end of trace area
        ldos    ci_tflg(g4),r9          # r9 = trace flags
        mov     r8,r10                  # next trace record pointer = beginning
                                        #  trace record pointer
        bbc     tflg_wrapoff,r9,.rs105a # Jif wrap off flag not set
        mov     0,r9                    # turn off traces due to trace area
                                        #  wrapped.
        stos    r9,ci_tflg(g4)
.rs105a:
        st      r10,ci_curtr(g4)        # save new current trace record pointer
.rs105z:
#
.endif # TRACES
#
.if     ERRLOG
#
        call    c$elog_imno             # log immediate notify event
#
.endif  # ERRLOG
#
.if     CDRIVER_MSGS
#
        call    c$mmcimno               # send message to MMC about this event
#
.endif  # CDRIVER_MSGS
#
# --- Special Immediate Notify event processing per QLogic specification
#
        ldos    instatus(g7),r6         # r6 = status code
        ldconst 0x36,r7                 # r7 = message received status code
        cmpobe  r6,r7,.rs200            # Jif message received status code
        ldconst 0x0e,r7                 # r7 = LIP reset status code
        cmpobe  r6,r7,.rs110            # Jif LIP reset status code
        ldconst 0x29,r7                 # r7 = port logged out status code
        cmpobe  r6,r7,.rs120            # Jif port logged out
        ldconst 0x20,r7                 # r7 = abort task status code
        cmpobe  r6,r7,.rs130            # Jif abort task status code
        call    c$compimno              # process immediate notify ack
        b       .rs1000                 # and end request processing
#
# --- Process LIP reset status code event
#
.rs110:
        ldconst 16,r6                   # change normalized event code to
                                        #  indicate offline event
        lda     c$compimno,r4           # get std. completion routine
        b       .rs135
#
# --- Process port logged out status code event
#
.rs120:
        ldconst 20,r6                   # change normalized event code to
                                        #  indicate port logged out event
        lda     c$compimno,r4           # get std. completion routine
        b       .rs135
#
# --- Process abort task status code event
#
.rs130:
        ldconst 24,r6                   # change normalized event code to
                                        #  indicate abort task event
        lda     c$compimno,r4           # get std. completion routine
.rs135:
        st      r4,il_cr(g1)            # save completion routine in ILT
        b       .rs300

.rs200:
        lda     normtbl(g5),r5          # r5 = event code normalization table
        ldob    (r5),r6                 # r6 = normalized event code
.rs300:
        addo    r6,r11,r11
        ld      (r11),r6
        callx   (r6)                    # go to event handler routine
.rs1000:
        movq    r12,g4                  # restore g4-g7
        ret
#
# --- Channel driver event code normalization table
#
        .data
normtbl:
        .byte   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0         # 00-0f
        .byte   4,8,16,20,28,32,0,0,0,0,0,0,0,0,0,0     # 10-1f
        .byte   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0         # 20-2f
        .byte   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0         # 30-3f
        .byte   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0         # 40-4f
        .byte   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0         # 50-5f
        .byte   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0         # 60-6f
        .byte   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0         # 70-7f
        .byte   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0         # 80-8f
        .byte   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0         # 90-9f
        .byte   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0         # a0-af
        .byte   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0         # b0-bf
        .byte   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0         # c0-cf
        .byte   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0         # d0-df
        .byte   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0         # e0-ef
        .byte   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0         # f0-ff
#
#******************************************************************************
#
#  NAME: C$regimt
#
#  PURPOSE:
#
#       Registers an IMT on the inactive IMT list.
#
#  DESCRIPTION:
#
#       Links the IMT on the inactive IMT list.
#       Note: Callers to this routine need to insure that no
#               conflicts exist with a previously registered IMT.
#
#  CALLING SEQUENCE:
#
#       call    C$regimt
#
#  INPUT:
#
#       g5 = IMT address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
        .text
#
C$regimt:
        mov     0,r4
        ld      C_imt_tail,r5            # r5 = inactive list tail record
        st      r4,im_link(g5)          # clear new IMT link list field
        cmpobne 0,r5,.regimt10          # Jif list not empty
        st      g5,C_imt_head            # save new IMT as head guy
        b       .regimt20
#
.regimt10:
        st      g5,im_link(r5)          # link new IMT onto end of list
.regimt20:
        ldob    im_flags(g5),r7         # r7 = IMT flags
        setbit  im_flags_inactive,r7,r7    # set inactive bit
        stob    r7,im_flags(g5)         # save updated flags byte
        st      g5,C_imt_tail            # save new IMT as new tail record
        ret                             # and we're out of here!
#
#******************************************************************************
#
#  NAME: C$receive_srp
#
#  PURPOSE:
#
#       Processes SRPs received from the MAGNITUDE processor.
#
#  DESCRIPTION:
#
#       If a primary ILT associated with the SRP, gets the original
#       requestors SRP handler routine from inl2_rcvsrp field of
#       primary ILT and calls this routine to process the SRP request.
#       If no primary ILT associated with the SRP, returns an error
#       to the requestor.
#
#  CALLING SEQUENCE:
#
#       call    C$receive_srp
#
#  INPUT:
#
#       g1 = ILT address nested at otl2 level
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
C$receive_srp:
        movq    g4,r8                           # save g4-g7
        ld      -ILTBIAS+otl1_FCAL(g1),g7       # g7 = primary ILT
        movq    g8,r12                          # save g8-g11
        cmpobne 0,g7,.recsrp50                  # Jif no primary ILT
#
# --- No primary ILT associated with SRP. Return to requestor
#       with error indication.
#
        movq    g0,r4                           # save g0-g3
        ldconst eccancel,g0                     # g0 = returned error code
        lda     -ILTBIAS(g1),g1                 # return ILT to caller
        ld      otl1_cr(g1),r3                  # r3 = caller's completion routine
        callx   (r3)
        movq    r4,g0                           # restore g0-g3
        b       .recsrp100                      # and we're out of here!
#
.recsrp50:
        lda     ILTBIAS(g7),g9                  # g7 = primary ILT at inl2 level
        movq    g0,r4                           # save g0-g3
        ld      inl2_rcvsrp(g9),g6              # g6 = SRP received routine
        ld      -ILTBIAS+otl1_srp(g1),g2        # g2 = SRP address
        callx   (g6)                            # call SRP received handler routine
        movq    r4,g0                           # restore g0-g3
#
#*****************************************************************************
#
# --- Interface to SRP handler routine
#
#  INPUT:
#
#       g1 = sec. ILT at otl2 nest level
#       g2 = SRP address
#       g7 = pri. ILT at inl1 nest level
#       g9 = pri. ILT at XL nest level
#
#  OUTPUT:
#
#       None.
#
#  REGS. DESTROYED.
#
#       Reg. r3-r15/g0-g11 can be destroyed.
#
#******************************************************************************
#
.recsrp100:
        movq    r12,g8                          # restore g8-g11
        movq    r8,g4                           # restore g4-g7
        ret
#
#******************************************************************************
#
#  NAME: c$mmcimno
#
#  PURPOSE:
#
#       Formats a message packet to be sent to the MMC indicating
#       an Immediate Notify event occurred.
#
#  DESCRIPTION:
#
#       Formats the message packet (using the standard Immediate Notify
#       event message packet template mmc_imno) and then calls the routine
#       to send the packet to the MMC.
#
#  CALLING SEQUENCE:
#
#       call    c$mmcimno
#
#  INPUT:
#
#       g4 = CIMT address (0 if none)
#       g7 = ILT of Immediate Notify event at inl1 nest level
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
        .text
#
c$mmcimno:
        mov     g0,r15                  # save g0
# --- Send log message for initialization failed
#
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
# Save header info
        ldconst mlehostimmednotify,r3   # save event type
        stos    r3,mle_event(g0)
        cmpobne 0,g4,.mmcimno10         # Jif CIMT specified
#
# --- No CIMT specified by caller
#
        ldconst 0xff,r9                 # dummy channel #
        b       .mmcimno20              # save the data in packet
#
# --- CIMT specified by caller
#
.mmcimno10:
        ldob    ci_num(g4),r9           # r9 = channel #
#
.mmcimno20:
        ldob    intaskf(g7),r10         # r10 = event task flags
        ldos    instatus(g7),r11        # r11 = event status
        stob    r9,ein_cnum(g0)         # save channel # in packet
        stob    r10,ein_taskflags(g0)   # save event task flags
        stob    r11,ein_qstatus(g0)     # save event status byte
#
c       MSC_LogMessageStack(&TmpStackMessage[0], einlen);

        mov     r15,g0                  # restore g0
        ret
#
#******************************************************************************
#
#  NAME: c$mmcoffline
#
#  PURPOSE:
#
#       Formats a message packet to be sent to the MMC indicating
#       an Interface offline event occurred.
#
#  DESCRIPTION:
#
#       Formats the message packet (using the standard Interface offline
#       event message packet template mmc_offline) and then calls the routine
#       to send the packet to the MMC.
#
#  CALLING SEQUENCE:
#
#       call    c$mmcoffline
#
#  INPUT:
#
#       g4 = CIMT address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
.if     CDRIVER_MSGS
c$mmcoffline:
        mov     g0,r15                  # save g0
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
# Save header info
        ldconst mlehostoffline,r3
        stos    r3,mle_event(g0)        # save event type
# Save offline unique info
        ldob    ci_num(g4),r9           # r9 = channel #
        stob    r9,eol_cnum(g0)         # save channel # in packet
#
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], eollen);
        mov     r15,g0                  # restore g0
        ret
.endif /* CDRIVER_MSGS */
#
.if     ERRLOG
#
#******************************************************************************
#
#  NAME: C$geterrrec
#
#  PURPOSE:
#
#       Allocate an error log record.
#
#  DESCRIPTION:
#
#       Allocates an error log record for the caller. Adjusts the
#       current pointer to the next error log record in the log
#       and saves the timestamp in the error log record.
#
#  CALLING SEQUENCE:
#
#       call    C$geterrrec
#
#  INPUT:
#
#       None.
#
#  OUTPUT:
#
#       g0 = error log record address
#
#  REGS DESTROYED:
#
#       Reg. g0 destroyed.
#
#******************************************************************************
#
C$geterrrec:
        ld      C_elog_current,g0       # g0 = allocated error log record
#
.ifdef TRACES
  .if TRACE_EL
        ldconst err_recsize+elog_trsize,r4 # r4 = error log record size
  .else   # TRACE_EL
        ldconst err_recsize,r4          # r4 = error log record size
  .endif # TRACE_EL
.else   # TRACES
        ldconst err_recsize,r4          # r4 = error log record size
.endif # TRACES
#
        ldl     C_elog_beg,r6           # r6 = beginning record address
                                        # r7 = ending record address
        addo    r4,g0,r4                # r4 = next error log record address
#
        ld      K_ii+ii_time,r5         # read timestamp (in 1/8 secs.)
        cmpobl  r4,r7,.geterec_10       # Jif records have not wrapped
        mov     r6,r4                   # r4 = beginning record address
.geterec_10:
        st      r4,C_elog_current       # save new current record address
        shro    3,r5,r5                 # r5 = timestamp in secs.
        st      r5,err_time(g0)         # save timestamp
        ret
#
.ifdef TRACES
  .if TRACE_EL
#******************************************************************************
#
#  NAME: C$traceerr
#
#  PURPOSE:
#
#       Copy traces over into an error log record.
#
#  DESCRIPTION:
#
#       Uses the chip instance saved in the specified error log
#       record to get the trace area for the interface and copy
#       the last n traces into the trace area of the error log
#       record.
#
#  CALLING SEQUENCE:
#
#       call    C$traceerr
#
#  INPUT:
#
#       g0 = error log record address
#       Note: Chip instance field (err_ci) in error log record
#               MUST be valid before calling!
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
C$traceerr:
        ldos    err_ci(g0),r4           # r4 = chip instance for event
        shlo    2,r4,r4                 # r4 = chip instance * 4
        ld      cimtDir(r4),r4          # r4 = assoc. CIMT address
        cmpobe  0,r4,.tracerr_1000      # Jif no assoc. CIMT (should never
                                        #  happen!!!)
        ld      ci_curtr(r4),r5         # r5 = current trace pointer
        ldl     ci_begtr(r4),r6         # r6 = beginning trace record addr.
                                        # r7 = ending trace record addr.
        cmpobe  0,r5,.tracerr_1000      # Jif current trace pointer = 0
        lda     err_recsize+elog_trsize(g0),r15 # r15 = end of event record
        ldconst 16,r3                   # r3 = size of trace record being
                                        #  copied
        ldconst elog_trsize,r14         # r14 = size of trace area in event
                                        #  log record
        subo    r3,r5,r5                # back current trace record pointer up
                                        #  by one trace record
.tracerr_10:
        cmpobge r5,r6,.tracerr_20     # Jif current trace pointer >=
                                        #  beginning trace record
        mov     r7,r5                   # current trace pointer = ending
                                        #  trace record pointer
.tracerr_20:
        ldq     (r5),r8                 # r8-r11 = trace record
        subo    r3,r15,r15              # r15 = address where to put trace
                                        #  record
        subo    r3,r14,r14              # dec. event record trace area byte
                                        #  count
        subo    r3,r5,r5                # back up current trace record pointer
        stq     r8,(r15)                # save trace record in event record
                                        #  trace area
        cmpobne 0,r14,.tracerr_10     # Jif more trace records to copy into
                                        #  event record trace area
.tracerr_1000:
        ret
#
  .endif # TRACE_EL
.endif # TRACES
#
#******************************************************************************
#
#  NAME: c$elog_imno
#
#  PURPOSE:
#
#       Logs an immediate notify event in the error log.
#
#  DESCRIPTION:
#
#       Places an immediate notify event in the error log and saves
#       the associated traces if enabled.
#
#  CALLING SEQUENCE:
#
#       call    c$elog_imno
#
#  INPUT:
#
#       g7 = pri. ILT of immediate notify event at inl1 nest level
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
IMNO_elog:
        .ascii  "IMNO"
#
c$elog_imno:
        ld      C_eflg,r4               # r4 = error log flags
        bbc     eflg_imno,r4,.elogimno_1000 # Jif event not to be logged
        mov     g0,r14                  # save g0
        call    C$geterrrec             # allocate an error log record
        ld      IMNO_elog,r4            # r4 = record type ID
        ldob    inchipi(g7),r5          # r5 = chip instance
        ld      inseqid(g7),r6          # r6 = sequence ID/task flags/unused
        ldos    inlun(g7),r9            # r9 = LUN
        ld      ininitiator(g7),r8      # r8 = initiator ID
        ld      inflags(g7),r7          # r7 = flags/status
        st      r4,err_rtc(g0)          # save record type ID
        stos    r5,err_ci(g0)           # save chip instance
        st      r6,err_exid(g0)         # save sequence ID/task flags/unused
        stos    r9,err_free+2(g0)       # save LUN
        st      r8,err_free+4(g0)       # save initiator ID
        st      r7,err_free+8(g0)       # save flags/status
        movl    0,r4
        stl     r4,err_free+12(g0)      # clear unused bytes
#
.ifdef TRACES
  .if TRACE_EL
        call    C$traceerr              # copy traces into error log record
  .endif # TRACE_EL
.endif # TRACES
#
        mov     r14,g0                  # restore g0
.elogimno_1000:
        ret
#
.endif # ERRLOG
#
#******************************************************************************
#
#  NAME: c$undef1
#        c$undef2
#        c$undef4
#
#  PURPOSE:
#
#       c$undef1 - Handles an undefined channel driver event when
#       the interface is undefined.
#       c$undef2 - Handles an undefined channel driver event when
#       the interface is offline or uninitialized.
#       c$undef4 - Handles an undefined channel driver event when
#       the interface is online.
#
#  DESCRIPTION:
#
#       Basically counts the event and ignores it.
#
#  CALLING SEQUENCE:
#
#       call    c$undef1
#       call    c$undef2
#       call    c$undef4
#
#  INPUT:
#
#       g1 = ILT address
#       g4 = 0 (usually CIMT address)
#       g7 = ILT param structure
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
c$undef1:
c$undef2:
c$undef4:
        ld      C_undef_event,r4        # r4 = undefined event count
        addo    1,r4,r4                 # inc. counter
        st      r4,C_undef_event        # save updated count
        ld      il_cr(g1),r4            # r4 = my completion handler routine
        cmpobne 0,r4,.undef4_100      # Jif a completion handler routine
                                        #  defined
        lda     c$comptmgnt,r4          # r4 = completion handler routine to
                                        #  call
.undef4_100:
        bx      (r4)                    # branch to completion handler routine
#
#******************************************************************************
#
#  NAME: c$cdb1
#
#  PURPOSE:
#
#       Handles a SCSI CDB received on an undefined interface.
#       Note: This should never occur!!!!!
#
#  DESCRIPTION:
#
#       Rejects the command with check condition status and sense
#       data telling the initiator to go away.
#
#  CALLING SEQUENCE:
#
#       call    c$cdb1
#
#  INPUT:
#
#       g1 = ILT address
#       g4 = assoc. CIMT address (0=none defined)
#       g7 = ILT param structure
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
c$cdb1:
        b       c$d$cmdrecv            # Use default scsi command handler
#
#******************************************************************************
#
#  NAME: c$cdb2
#
#  PURPOSE:
#
#       Handles a SCSI CDB received when the interface is
#       uninitialized or offline.
#
#  DESCRIPTION:
#
#       Puts the interface online, does offline-to-online processing
#       and then processes the event as if the interface was online.
#
#  CALLING SEQUENCE:
#
#       call    c$cdb2
#
#  INPUT:
#
#       g1 = ILT address
#       g4 = assoc. CIMT address
#       g7 = ILT param structure
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       g4-g7 can be destroyed.
#
#******************************************************************************
#
c$cdb2:
        call    c$onl2          # go through online processing
        b       c$cdb4          # and handle as though online
#
#******************************************************************************
#
#  NAME: c$cdb4
#
#  PURPOSE:
#
#       Handles a SCSI CDB received when the interface is
#       online.
#
#  DESCRIPTION:
#
#       Finds the appropriate device driver and calls the command
#       received routine if defined. Else it handles the command the
#       same as if received for an undefined interface.
#
#  CALLING SEQUENCE:
#
#       call    c$cdb4
#
#  INPUT:
#
#       g1 = ILT address
#       g4 = assoc. CIMT address
#       g7 = ILT param structure
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       g4-g7 can be destroyed.
#
#******************************************************************************
#
c$cdb4:
        ldob    idf_taskf(g7),r7        # r7 = task management flags
        cmpobe  0,r7,.cdb05             # Jif none set
        lda     c$comptmgnt,r15         # set up appropriate completion routine
        st      r15,il_cr(g1)           # save in ILT
        ld      ci_ehand(g4),r6
        ld      cde_imno(r6),r6         # set up to go to immediate notify
                                        #  event handler routine
        bx      (r6)                    # and go to it
.cdb05:
#
.ifdef TRACES
#
# --- Trace incoming event if appropriate
#
        ldos    ci_tflg(g4),r4          # r4 = trace flags
        lda     C_temp_trace,r10        # r10 = trace record build pointer
        ld      ci_curtr(g4),r3         # r3 = current trace record pointer
        bbc     tflg_c$cdb4,r4,.cdb05z  # Jif event trace disabled
        ld      sccdb(g7),r4            # r4 = pointer to CDB
        ldob    ci_num(g4),r6           # r6 = chip instance
        ldconst trt_c$cdb4,r5           # r5 = trace record type code
        ldos    idf_exid(g7),r7         # r7 = exchange ID
        ldt     (r4),r12                # r12-r14 = first 12 bytes of CDB
        stob    r5,trr_trt(r10)         # save trace record type code
        stob    r6,trr_ci(r10)          # save chip instance
        stos    r7,trr_exid(r10)        # save chip instance
        stt     r12,4(r10)              # save first 12 bytes of CDB
        ldq     (r10),r4                # r4-r7 = trace record
        ldl     ci_begtr(g4),r8         # r8 = trace area beginning pointer
                                        # r9 = trace area ending pointer
        lda     trr_recsize(r3),r10     # r10 = next trace record pointer
        stq     r4,(r3)                 # save trace record in CIMT trace area
        cmpoble r10,r9,.cdb05a          # Jif trace record pointer has not
                                        #  exceeded end of trace area
        ldos    ci_tflg(g4),r9          # r9 = trace flags
        mov     r8,r10                  # next trace record pointer = beginning
                                        #  trace record pointer
        bbc     tflg_wrapoff,r9,.cdb05a # Jif wrap off flag not set
        mov     0,r9                    # turn off traces due to trace area
                                        #  wrapped.
        stos    r9,ci_tflg(g4)
.cdb05a:
        st      r10,ci_curtr(g4)        # save new current trace record pointer
.cdb05z:
#
.endif # TRACES
#
# --- Check the active IMT list for a matching loop ID
#
        ld      ci_imthead(g4),g5       # g5 = first IMT on active list
        cmpobe  0,g5,.cdb20             # Jif no active IMTs defined
        ld      idf_init(g7),r5         # r5 = initiator ID / virtual port ID
.cdb10:
        ld      im_fcaddr(g5),r6        # get IMT ID / virtual port ID
        cmpobe  r5,r6,.cdb30            # Jif the one
        ld      im_link(g5),g5          # get next IMT on active list
        cmpobne 0,g5,.cdb10             # Jif more IMTs to check
#
# --- Check for inactive IMT list
#
.cdb20:
        call    c$findimt               # find an appropriate IMT for initiator
        cmpobe  FINDIMT_INCONSISTENT,g5,.cdb22   # server tables out of synch discard IO.
        cmpobne 0,g5,.cdb25             # Jif appropriate IMT found
        mov     0,g5                    # g5 = 0 (IMT address)
        mov     0,g6                    # g6 = 0 (ILMT address)
        lda     c$d$cmdrecv,r6          # Use default scsi command handler
        b       .cdb70                  # and go to handler routine

.cdb22:
        ldob    ci_num(g4),r6           # r6 = chip instance
c fprintf(stderr,"%s%s:%u c$findimt returns -1 port %lu discarding ILT %08lX \n", FEBEMESSAGE, __FILE__, __LINE__,r6,g1)
        lda     -ILTBIAS(g1),g1         # put ilt back to isp level
        PushRegs(r3)
c       isp_abort_exchange_ilt(r6,(struct ILT*)g1);
        PopRegsVoid(r3)
        ld      il_cr(g1),r3            # get completion handler
        bx      (r3)                    # Invoke completion handler/return

.cdb25:
        ldob    im_flags(g5),r5         # r5 = IMT flags
        clrbit  im_flags_inactive,r5,r5    # clear inactive bit
        stob    r5,im_flags(g5)         # save updated flags byte

#
# --- Call IMT online event handler routine
#
        ld      im_ehand(g5),r5         # r5 = IMT event handler table
        ld      dd_online(r5),r5        # r5 = online event handler routine
        callx   (r5)                    # call IMT online event handler
                                        #  routine
#
# --- Activate new IMT
#
        ldos    ci_numhosts(g4),r5      # get number of hosts
        addo    1,r5,r5                 # increment the count by one
        stos    r5,ci_numhosts(g4)      # save it back
#
        ld      ci_imthead(g4),r5       # r5 = first IMT on active list
        st      g5,ci_imthead(g4)       # save IMT as new head of list
        st      r5,im_link(g5)          # link onto head of list
        cmpobne 0,r5,.cdb30             # Jif list not empty
        st      g5,ci_imttail(g4)       # save new IMT as tail
.cdb30:
        ldos    idf_lun(g7),r7          # r7 = LUN
        lda     LUNMAX,r8               # r8 = max. LUN supported
        cmpobl  r7,r8,.cdb50            # Jif LUN within range
        mov     0,g6                    # g6 = 0 (ILMT address)
.cdb40:
        ld      im_ehand(g5),r6         # r6 = default device driver event
                                        #      handler table
        b       .cdb60
#
.cdb50:
        ld      im_ilmtdir(g5)[r7*4],g6 # g6 = assoc. ILMT address
        cmpobe  0,g6,.cdb40             # Jif no ILMT defined
        ld      ilm_ehand(g6),r6        # r6 = device driver event handler table
.cdb60:
        ld      dd_cmdrcv(r6),r6        # r6 = command recv. event routine
#
        call    c$add2cmd               # inc. command counters
#
# --- Increment server queue depth
#
        ldos    im_qdepth(g5),r3        # get current queue depth
        addo    1,r3,r3                 # increment it
        stos    r3,im_qdepth(g5)        # save it
.cdb70:
        st      g5,idf_imt(g7)          # save IMT in ILT for queue depth
                                        #   decrement when the command completes
#
#********************************************************************************
#
#       Interface to device driver handler routine:
#       -------------------------------------------
#
#       Input:
#               g1 = ILT address
#               g4 = CIMT address
#               g5 = IMT address
#               g6 = ILMT address (0=LUN not supported)
#               g7 = ILT param structure
#
#       Output:
#               None.
#
#       r3-r15/g4-g7 can be destroyed.
#
#********************************************************************************
#
        bx      (r6)                    # and go to handler routine
#
#******************************************************************************
#
#  NAME: c$imno1
#
#  PURPOSE:
#
#       Handles an immediate notify message received on an undefined interface.
#       Note: This should never occur!!!!!
#
#  DESCRIPTION:
#
#       Clears out the event and returns to caller.
#
#  CALLING SEQUENCE:
#
#       call    c$imno1
#
#  INPUT:
#
#       g1 = ILT address
#       g4 = assoc. CIMT address (0=none defined)
#       g7 = ILT param structure
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
c$imno1:
        call    c$mmcimno               # send message to MMC about the event
        call    c$compimno              # process event completion
        ret
#
#******************************************************************************
#
#  NAME: c$imno2
#
#  PURPOSE:
#
#       Handles an immediate notify message received on an uninitialized
#       or offline interface.
#
#  DESCRIPTION:
#
#       Puts the interface in an online state. Handles the event the same
#       as when the interface is online.
#
#  CALLING SEQUENCE:
#
#       call    c$imno2
#
#  INPUT:
#
#       g1 = ILT address
#       g4 = assoc. CIMT address
#       g7 = ILT param structure
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       Reg. r3-r15/g4-g7 can be destroyed.
#
#******************************************************************************
#
c$imno2:
        call    c$onl2                  # Put interface in online state
        b       c$imno4                 # and handle as if in the online state
#
#******************************************************************************
#
#  NAME: c$imno4
#
#  PURPOSE:
#
#       Handles an immediate notify message received on an online interface.
#
#  DESCRIPTION:
#
#       Finds the ILMT associated with the event. If one was defined, calls
#       the device driver's associated handler routine. Otherwise calls the
#       default device driver's associated handler routine.
#
#  CALLING SEQUENCE:
#
#       call    c$imno4
#
#  INPUT:
#
#       g1 = ILT address
#       g4 = assoc. CIMT address
#       g7 = ILT param structure
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       r3-r15/g4-g7 can be destroyed.
#
#******************************************************************************
#
c$imno4:
        ld      il_cr(g1),r15           # r15 = my completion handler routine
        cmpobne 0,r15,.imno05           # Jif handler already defined
        lda     c$compimno,r15          # get std. completion routine
        st      r15,il_cr(g1)           # save completion routine in ILT
.imno05:
        ld      ci_imthead(g4),g5       # g5 = first IMT on active list
        cmpobe  0,g5,.imno15            # Jif no active IMTs defined
        ld      idf_init(g7),r5         # r5 = initiator ID / virtual port
.imno10:
        ld      im_fcaddr(g5),r6        # get IMT ID / virtual port
        cmpobe  r5,r6,.imno15           # Jif the one
        ld      im_link(g5),g5          # get next IMT on active list
        cmpobne 0,g5,.imno10            # Jif more IMTs to check
.imno15:
#
#       g5 = assoc. IMT address
#       g5 = 0 if no assoc. IMT found
#
        ldos    idf_lun(g7),r9          # r9 = LUN
        lda     LUNMAX,r8               # r8 = max. LUN supported
#
#********************************************************************************
#
# --- Target cold reset handler routine
#
# --- Aborts all tasks for all LUNs for all initiators
#
#********************************************************************************
#
        ldob    idf_taskf(g7),r7        # r7 = task management flags
        bbc     tf_reset,r7,.imno25     # Jif not target cold reset set

        ld      ci_imthead(g4),g5       # g5 = current IMT

.imno20:
        cmpobe  0,g5,.imno100           # we're done when no more IMTs
        ld      im_ehand(g5),r6         # r6 = default event handler table
        ld      dd_reset(r6),r6         # r6 = default event handler routine

        callx   (r6)                    # call default event routine
                                        # input:
                                        #       g4 = CIMT address
                                        #       g5 = IMT address
                                        # output:
                                        #       none
                                        #       r3-r15/g6 can be destroyed.

        ld      im_link(g5),g5          # g5 = next IMT to process
        b       .imno20                 # and process next one
#
#********************************************************************************
#
# --- Target warm reset handler routine for iSCSI
#
# --- Aborts all tasks for all LUNs for all initiators
#
#********************************************************************************
#
.imno25:
        ldob    idf_taskf(g7),r7        # r7 = task management flags
        bbc     tf_warmreset,r7,.imno30 # Jif not target warm reset set
#
# --- get corresponding initiator (IMT) and cleanup
#
        PushRegs(r3)                    # Save all the registers
        ldos    idf_vpid(g7),r7         # Get virtual port ID
        mov     r7,g0                   # (vpID)Set input parameter
        mov     r5,g1                   # (init ID) Set input parameter
        call    iSCSI_GetISID           # Call the C code
        movl    g0,r10                  # Save the return value r10-r11
        cmpobne 0,g0,.imno26            # Jif not NULL
        cmpobne 0,g1,.imno26            # Jif not NULL
        PopRegsVoid(r3)                 # Restore the registers
        b .imno100                      # we're done when no more IMTs
#
.imno26:
        mov     r7,g2                   # (vpID)Set input parameter
        mov     r5,g3                   # (initID)Set input parameter
        call    fsl_findIMT             # Call the C code
        mov     g0,r7                   # Save the return value
        PopRegsVoid(r3)                 # Restore the registers

        cmpobe  0,r7,.imno100           # Jif no IMTs
        cmpobe  0xffffffff,r7,.imno100  # Jif 0xffffffff

        mov     r7,g5                   # IMT in g5

        ld      im_ehand(g5),r6         # r6 = default event handler table
        ld      dd_reset(r6),r6         # r6 = default event handler routine

        callx   (r6)                    # call default event routine
                                        # input:
                                        #       g4 = CIMT address
                                        #       g5 = IMT address
                                        # output:
                                        #       none
                                        #       r3-r15/g6 can be destroyed.

        b .imno100                      # we're done when no more IMTs
#
#********************************************************************************
#
# --- LUN reset handler routine
#
# --- Aborts all tasks for specified LUN
#
#********************************************************************************
#
.imno30:
        cmpobge r9,r8,.imno100          # Jif LUN # out of range

        bbc     tf_lunreset,r7,.imno40  # Jif not LUN reset flag

        cmpobe  0,g5,.imno100           # Jif no assoc. IMT

        ld      im_ilmtdir(g5)[r9*4],g6 # g6 = ILMT address
        cmpobe  0,g6,.imno100           # Jif no assoc. ILMT

        ld      ilm_ehand(g6),r6        # r6 = driver event handler table
        ld      dd_reset(r6),r6         # r6 = reset event handler routine

        callx   (r6)                    # call event handler routine
                                        # Input:
                                        #       g1 = ILT address
                                        #       g4 = CIMT address
                                        #       g5 = IMT address
                                        #       g6 = ILMT address
                                        # Output:
                                        #       None.
                                        # r3-r15/g6 can be destroyed.

        b       .imno100                # and we're done!
#
#********************************************************************************
#
# --- Abort task set handler routine
#
# --- Aborts all tasks for specified LUN from specified initiator.
#
#********************************************************************************
#
.imno40:
        bbc     tf_abtaskset,r7,.imno50 # Jif not abort task set flag

        cmpobe  0,g5,.imno100           # Jif no assoc. LMT
#
# --- Check if image is pending
#
        ld      im_ilmtdir(g5)[r9*4],g6 # g6 = assoc. ILMT
        cmpobe  0,g6,.imno100           # Jif no ILMT defined

        ld      ilm_ehand(g6),r6        # r6 = event handler table
        ld      dd_abtaskset(r6),r6     # r6 = event handler routine

        callx   (r6)                    # call event handler routine
                                        # Input:
                                        #       g1 = ILT address
                                        #       g4 = CIMT address
                                        #       g5 = IMT address
                                        #       g6 = ILMT address
                                        #       g7 = ILT param structure
                                        # Output:
                                        #       None.
                                        # r3-r15/g6 can be destroyed.

        b       .imno100                # and we're done!
#
#********************************************************************************
#
# --- Clear task set handler routine
#
# --- Aborts all tasks for specified LUN for all initiators
#
#********************************************************************************
#
.imno50:
        bbc     tf_cltaskset,r7,.imno60 # Jif not clear task set flag

        ld      ci_imthead(g4),g5       # g5 = current IMT

.imno52:
        cmpobe  0,g5,.imno100           # we're done when no more IMTs
        ld      im_ilmtdir(g5)[r9*4],g6 # g6 = assoc. ILMT address
        cmpobe  0,g6,.imno54            # Jif ILMT not defined
        ld      ilm_ehand(g6),r6        # r6 = driver event handler table
        ld      dd_abtaskset(r6),r6     # r6 = reset event handler routine

        callx   (r6)                    # call event handler routine
                                        # Input:
                                        #       g1 = ILT address
                                        #       g4 = CIMT address
                                        #       g5 = IMT address
                                        #       g6 = ILMT address
                                        #       g7 = ILT param structure
                                        # Output:
                                        #       None.
                                        # r3-r15/g6 can be destroyed.
.imno54:
        ld      im_link(g5),g5          # g5 = next IMT to process
        b       .imno52                 # and process next one
#
#********************************************************************************
#
# --- Terminate task handler routine
#
# --- Terminate only specified exchange ID task from initiator
#
#********************************************************************************
#
.imno60:
        bbc     tf_termtask,r7,.imno70  # Jif not terminate task flag

        cmpobe  0,g5,.imno100           # Jif no assoc. IMT

        ld      im_ehand(g5),r6         # r6 = default event handler table
        ld      dd_abtask(r6),r6        # r6 = abort event handler routine

        callx   (r6)                    # and go to handler routine
                                        # INPUT:
                                        #       g1 = ILT address of event at inl2 nest level
                                        #       g4 = CIMT address
                                        #       g5 = IMT address
                                        #       g7 = ILT address of event at inl1 nest level
                                        # Output:
                                        #       None.
                                        # r3-r15/g6 can be destroyed.

        b       .imno100                # and we're done!
#
#********************************************************************************
#
# --- Clear ACA handler routine
#
#********************************************************************************
#
.imno70:
        bbc     tf_clearaca,r7,.imno100 # Jif no clear ACA flag

        cmpobe  0,g5,.imno100           # Jif no assoc. IMT

        ld      im_ilmtdir(g5)[r9*4],g6 # g6 = assoc. ILMT address
        cmpobe  0,g6,.imno100          # Jif no ILMT defined
        ld      ilm_ehand(g6),r6        # r6 = device driver event handler table
        ld      dd_clearaca(r6),r6      # r6 = Clear ACA event handler routine

        callx   (r6)                    # and go to handler routine
                                        # Input:
                                        #       g1 = ILT address
                                        #       g4 = CIMT address
                                        #       g5 = IMT address
                                        #       g6 = ILMT address
                                        #       g7 = ILT param structure
                                        # Output:
                                        #       None.
                                        # r3-r15/g6 can be destroyed.

#
#********************************************************************************
#
# --- Exit
#
#********************************************************************************
#
.imno100:
        bx      (r15)                   # and go to my level completion routine
#
#******************************************************************************
#
#  NAME: c$onl1
#
#  PURPOSE:
#
#       Handles an interface online event received on an undefined interface.
#       Note: This should never occur!!!!!
#
#  DESCRIPTION:
#
#       Just returns to caller.
#
#  CALLING SEQUENCE:
#
#       call    c$onl1
#
#  INPUT:
#
#       g1 = ILT address
#       g4 = assoc. CIMT address (0=none defined)
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
c$onl1:
        b       c$offl1                 # handle the same since this should
                                        #  never occur.
#
#******************************************************************************
#
#  NAME: c$onl2
#
#  PURPOSE:
#
#       Handles an interface online event received on an uninitialized
#       or offline interface.
#
#  DESCRIPTION:
#
#       Puts the interface online and returns to caller.
#
#  CALLING SEQUENCE:
#
#       call    c$onl2
#
#  INPUT:
#
#       g1 = ILT address
#       g4 = assoc. CIMT address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
c$onl2:
        mov     cis_on,r4               # r4 = new CIMT state
        lda     C_event_tbl4,r5         # r5 = new channel driver event
                                        #      handler table
        stob    r4,ci_state(g4)         # save new CIMT state code
        st      r5,ci_ehand(g4)         # save new event handler table
        ret                             # and we're out of here!
#
#******************************************************************************
#
#  NAME: c$onl4
#
#  PURPOSE:
#

#       Handles an interface online event received on an online interface.
#       Note: This should never occur!!!!!
#
#  DESCRIPTION:
#
#       Just returns to caller.
#
#  CALLING SEQUENCE:
#
#       call    c$onl4
#
#  INPUT:
#
#       g1 = ILT address
#       g4 = assoc. CIMT address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
c$onl4:
        b       c$offl1                 # handle the same since this should
                                        #  never occur.
#
#******************************************************************************
#
#  NAME: c$offl1
#
#  PURPOSE:
#
#       Handles an interface offline event received on an undefined interface.
#       Note: This should never occur!!!!!
#
#  DESCRIPTION:
#
#       Just returns to caller.
#
#  CALLING SEQUENCE:
#
#       call    c$offl1
#
#  INPUT:
#
#       g1 = ILT address
#       g4 = assoc. CIMT address (0=none defined)
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
c$offl1:
        ld      il_cr(g1),r4            # r4 = my completion handler routine
        cmpobne 0,r4,.coffl1_100        # Jif a completion handler defined
        lda     c$comptmgnt,r4          # r4 = completion handler routine to
                                        #  call
.coffl1_100:
        callx   (r4)                    # call my completion handler routine
        ret
#
#******************************************************************************
#
#  NAME: c$offl2
#
#  PURPOSE:
#
#       Handles an interface offline event received on an uninitialized
#       or offline interface.
#       Note: This should never occur!!!!!
#
#  DESCRIPTION:
#
#       Just returns to caller.
#
#  CALLING SEQUENCE:
#
#       call    c$offl2
#
#  INPUT:
#
#       g1 = ILT address
#       g4 = assoc. CIMT address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
c$offl2:
        b       c$offl1                 # handle the same since this should
                                        #  never occur.
#
#******************************************************************************
#
#  NAME: c$offl4
#
#  PURPOSE:
#
#       Handles an interface offline event received on an online interface.
#       Note that an offline event can not only be an offline event but also
#       represents a link failure, connection recovery or any other event
#       that causes the interface to become not operational.
#
#  DESCRIPTION:
#
#       Notifies all active device drivers of the offline event and then
#       places the interface offline.
#
#  CALLING SEQUENCE:
#
#       call    c$offl4
#
#  INPUT:
#
#       g1 = ILT address
#       g4 = assoc. CIMT address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       g5-g6 can be destroyed.
#
#******************************************************************************
#
        .data
OFFL_elog:
        .ascii  "OFFL"
#
        .text
#
c$offl4:
#
.ifdef TRACES
#
# --- Trace incoming event if appropriate
#
        ldos    ci_tflg(g4),r4          # r4 = trace flags
        ld      ci_curtr(g4),r10        # r10 = current trace record pointer
        bbc     tflg_c$offl4,r4,.offl10z # Jif event trace disabled
        ldob    ci_num(g4),r6           # r6 = chip instance
        ldconst trt_c$offl4,r5          # r5 = trace record type code
        ldos    idf_exid(g7),r7         # r7 = exchange ID
c       r14 = get_tsc_l() & ~0xf;       # Get free running bus clock.
        stob    r5,trr_trt(r10)         # save trace record type code
        stob    r6,trr_ci(r10)          # save chip instance
        stos    r7,trr_exid(r10)        # save chip instance
        movl    0,r12                   # r12-r13 = unused (0)
        stt     r12,4(r10)              # save unused bytes, timestamp
        ldl     ci_begtr(g4),r8         # r8 = trace area beginning pointer
                                        # r9 = trace area ending pointer
        lda     trr_recsize(r10),r10    # r10 = next trace record pointer
        cmpoble r10,r9,.offl10a         # Jif trace record pointer has not
                                        #  exceeded end of trace area
        ldos    ci_tflg(g4),r9          # r9 = trace flags
        mov     r8,r10                  # next trace record pointer = beginning
                                        #  trace record pointer
        bbc     tflg_wrapoff,r9,.offl10a # Jif wrap off flag not set
        mov     0,r9                    # turn off traces due to trace area
                                        #  wrapped.
        stos    r9,ci_tflg(g4)
.offl10a:
        st      r10,ci_curtr(g4)        # save new current trace record pointer
.offl10z:
#
.endif # TRACES
#
.if     ERRLOG
        ld      C_eflg,r4               # r4 = error log flags
        bbc     eflg_offl,r4,.offl11    # Jif event not to be logged
        mov     g0,r14                  # save g0
        call    C$geterrrec             # allocate an error log record
        ld      OFFL_elog,r4            # r4 = record type ID
        ldob    ci_num(g4),r5           # r5 = chip instance
        st      r4,err_rtc(g0)          # save record type ID
        st      r5,err_ci(g0)           # save chip instance/clear exchange ID
        movq    0,r4
        st      r4,err_free(g0)         # clear unused bytes
        stq     r4,err_free+4(g0)       # clear unused bytes
.ifdef TRACES
  .if TRACE_EL
        call    C$traceerr              # copy traces into error log record
  .endif # TRACE_EL
.endif # TRACES
        mov     r14,g0                  # restore g0
.offl11:
.endif  # ERRLOG
#
.if     CDRIVER_MSGS
        call    c$mmcoffline            # send message to MMC regarding the offline event
.endif  # CDRIVER_MSGS
#
.offl10:
#
# --- Go through all active IMTs and notify each associated device driver
#       of the event.
#
        ld      ci_imthead(g4),g5       # g5 = IMT address
        cmpobe  0,g5,.offl100           # Jif no active IMTs
        ld      im_link(g5),r4          # r4 = next active IMT
        st      r4,ci_imthead(g4)       # remove IMT from active list
        cmpobne 0,r4,.offl20            # Jif more active IMTs on list
        st      r4,ci_imttail(g4)       # clear list tail pointer
.offl20:
        mov     0,r4                    # r4 = 0 = LUN index
        st      r4,im_link(g5)
        ld      im_ehand(g5),r6         # get default event handler table
        ld      dd_offline(r6),r6       # get offline event handler routine
#
#*******************************************************************************
#
#       Interface to device driver routine:
#       -----------------------------------
#
#       Input:
#               g4 = CIMT address
#               g5 = IMT address
#
#       Output:
#               None.
#
#       reg. g5-g6/r3-r15 can be destroyed.
#
#*******************************************************************************
#
        callx   (r6)                    # call default routine
        b       .offl10                 # process the next active IMT
#
# --- Set CIMT in offline state
#
.offl100:
        ldconst 0,r4                    # zero it
        stos    r4,ci_numhosts(g4)      # no more active hosts
        mov     cis_off,r4              # r4 = new CIMT state
        lda     C_event_tbl3,r5         # r5 = new event handler routine table
        stob    r4,ci_state(g4)         # save new state code
        st      r5,ci_ehand(g4)         # save new event handler routine table
        ld      il_cr(g1),r4            # r4 = my completion handler routine
        cmpobne 0,r4,.coffl4_100      # Jif a completion handler defined
        lda     c$comptmgnt,r4          # r4 = completion handler routine to
                                        #  call
.coffl4_100:
        callx   (r4)                    # call my completion handler routine
        ret
#
#******************************************************************************
#
#  NAME: c$plo1
#
#  PURPOSE:
#
#       Handles a port logged out event received on an undefined
#       interface.
#       Note: This should never occur!!!!!
#
#  DESCRIPTION:
#
#       Just returns to caller.
#
#  CALLING SEQUENCE:
#
#       call    c$plo1
#
#  INPUT:
#
#       g1 = ILT address of event at inl2 nest level
#       g4 = assoc. CIMT address (0=none defined)
#       g7 = ILT of event at inl1 nest level
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
c$plo1:
        b       c$offl1                 # handle the same since this should
                                        #  never occur.
#
#******************************************************************************
#
#  NAME: c$plo2
#
#  PURPOSE:
#
#       Handles a port logged out event received on an uninitialized
#       or offline interface.
#       Note: This should never occur!!!!!
#
#  DESCRIPTION:
#
#       Just returns to caller.
#
#  CALLING SEQUENCE:
#
#       call    c$plo2
#
#  INPUT:
#
#       g1 = ILT address of event at inl2 nest level
#       g4 = assoc. CIMT address
#       g7 = ILT of event at inl1 nest level
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
c$plo2:
        b       c$plo4                  # handle the same as if the interface
                                        #  is online.
#
#******************************************************************************
#
#  NAME: c$plo4
#
#  PURPOSE:
#
#       Handles a port logged out event received on an online interface.
#
#  DESCRIPTION:
#
#       Notifies the associated active device driver for the specified
#       initiator (if active) of the event. If no active device driver
#       then ignores the event.
#
#  CALLING SEQUENCE:
#
#       call    c$plo4
#
#  INPUT:
#
#       g1 = ILT address of event at inl2 nest level
#       g4 = assoc. CIMT address
#       g7 = ILT of event at inl1 nest level
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       g5-g6 can be destroyed.
#
#******************************************************************************
#
c$plo4:
#
# --- Find the active IMT associated with the initiator ID and notify
#       the associated device driver of an offline event.
#
        ldos    ininitiator(g7),r15     # r15 = assoc. initiator ID
        ld      ci_imthead(g4),g5       # g5 = IMT address
        cmpobe  0,g5,.plo100            # Jif no active IMTs
        lda     ci_imthead(g4),r5       # r5 = pseudo last IMT checked
        mov     0,r6                    # r6 = value to store as tail if
                                        #  IMT is current tail member
        ld      ci_imttail(g4),r7       # r7 = current tail member
.plo03:
        ldos    im_fcaddr(g5),r4        # r4 = IMT initiator ID
        cmpobe  r4,r15,.plo10           # Jif IDs match
        mov     g5,r5                   # r5 = last IMT checked
        mov     g5,r6                   # r6 = new tail member if match
        ld      im_link(g5),g5          # g5 = next active IMT
        cmpobne 0,g5,.plo03             # Jif more active IMTs to check
        b       .plo100                 # No more active IMTs to check
#
# --- An IMT match has been found
#
#  INPUT:
#       g1 = ILT address of event at inl2 nest level
#       g4 = assoc. CIMT address
#       g5 = IMT that matched
#       g7 = ilt address of event at inl1 nest level
#
.plo10:
        ld      im_link(g5),r3          # r3 = next IMT address on list
        st      r3,im_link(r5)          # unlink IMT from list
        cmpobne g5,r7,.plo20            # Jif not last IMT on list
        st      r6,ci_imttail(g4)       # save new tail member
.plo20:
        mov     0,r4                    # r4 = 0 = LUN index
        st      r4,im_link(g5)
        ld      im_ehand(g5),r6         # get default event handler table
        ld      dd_offline(r6),r6       # get offline event handler routine
#
#*******************************************************************************
#
#       Interface to device driver routine:
#       -----------------------------------
#
#       Input:
#               g4 = CIMT address
#               g5 = IMT address
#
#       Output:
#               None.
#
#       reg. g5-g6/r3-r15 can be destroyed.
#
#*******************************************************************************
#
        callx   (r6)                    # call default routine
.ifdef MULTI_ID
        mov     r3,g5                   # Get next IMT
        cmpobne 0,g5,.plo03             # Jif more IMTs
.endif  # MULTI_ID

.plo100:
        ld      il_cr(g1),r4            # r4 = my completion handler routine
        cmpobne 0,r4,.plo200            # Jif a completion handler defined
        lda     c$comptmgnt,r4          # r4 = completion handler routine to
                                        #  call
.plo200:
        callx   (r4)                    # call my completion handler routine
        ret
#
#******************************************************************************
#
#  NAME: c$abort1
#
#  PURPOSE:
#
#       Handles an abort task event received on an undefined
#       interface.
#       Note: This should never occur!!!!!
#
#  DESCRIPTION:
#
#       Just returns to caller.
#
#  CALLING SEQUENCE:
#
#       call    c$abort1
#
#  INPUT:
#
#       g1 = ILT address of event at inl2 nest level
#       g4 = assoc. CIMT address (0=none defined)
#       g7 = ILT of event at inl1 nest level
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
c$abort1:
        b       c$offl1                 # handle the same since this should
                                        #  never occur.
#
#******************************************************************************
#
#  NAME: c$abort2
#
#  PURPOSE:
#
#       Handles an abort task event received on an uninitialized
#       or offline interface.
#       Note: This should never occur!!!!!
#
#  DESCRIPTION:
#
#       Just returns to caller.
#
#  CALLING SEQUENCE:
#
#       call    c$abort2
#
#  INPUT:
#
#       g1 = ILT address of event at inl2 nest level
#       g4 = assoc. CIMT address
#       g7 = ILT of event at inl1 nest level
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
c$abort2:
        b       c$offl1                 # handle the same since this should
                                        #  never occur.
#
#******************************************************************************
#
#  NAME: c$abort4
#
#  PURPOSE:
#
#       Handles an abort task event received on an online interface.
#
#  DESCRIPTION:
#
#       Notifies the associated active device driver for the specified
#       initiator (if active) of the event. If no active device driver
#       then ignores the event.
#
#  CALLING SEQUENCE:
#
#       call    c$abort4
#
#  INPUT:
#
#       g1 = ILT address of event at inl2 nest level
#       g4 = assoc. CIMT address
#       g7 = ILT of event at inl1 nest level
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       g5-g6 can be destroyed.
#
#******************************************************************************
#
c$abort4:
#
# --- Find the active IMT associated with the initiator ID and notify
#       the associated device driver of an abort task event.
#
        ldos    ininitiator(g7),r15     # r15 = assoc. initiator ID
        ldos    invpid(g7),r14          # r14 = assoc. virtual port ID
        ld      ci_imthead(g4),g5       # g5 = IMT address
        cmpobe  0,g5,.abort100          # Jif no active IMTs
.abort03:
        ldos    im_fcaddr(g5),r4        # r4 = IMT initiator ID
        cmpobne r4,r15,.abort05         # Jif IDs mismatch
        ldos    im_vpid(g5),r3          # r4 = IMT virtual port ID
        cmpobe  r3,r14,.abort10         # Jif virtual port IDs match
.abort05:
        ld      im_link(g5),g5          # g5 = next active IMT
        cmpobne 0,g5,.abort03           # Jif more active IMTs to check
        b       .abort100               # No more active IMTs to check
#
# --- An IMT match has been found
#
#  INPUT:
#       g1 = ILT address of event at inl2 nest level
#       g4 = assoc. CIMT address
#       g5 = IMT that matched
#       g7 = ILT address of event at inl1 nest level
#
.abort10:
        ld      im_ehand(g5),r6         # get default event handler table
        ld      dd_abtask(r6),r6        # get abort task event handler routine
#
#*******************************************************************************
#
#       Interface to device driver routine:
#       -----------------------------------
#
# Input:
#       g1 = ILT of event at inl2 nest level
#       g4 = CIMT address
#       g5 = IMT address
#       g7 = ILT of event at inl1 nest level
#
# Output:
#       None.
#
#       reg. g6/r3-r15 can be destroyed.
#
#*******************************************************************************
#
        callx   (r6)                    # call default routine
.abort100:
        ld      il_cr(g1),r4            # r4 = my completion handler routine
        cmpobne 0,r4,.abort200        # Jif a completion handler defined
        lda     c$comptmgnt,r4          # r4 = completion handler routine to
                                        #  call
.abort200:
        callx   (r4)                    # call my completion handler routine
        ret
#
#******************************************************************************
#
#  NAME: c$rinit1
#
#  PURPOSE:
#
#       Handles an Reset and Initialize event received.
#
#  DESCRIPTION:
#
#       Delete all IMTs for the specified port.
#
#  CALLING SEQUENCE:
#
#       call    c$rinit1
#
#  INPUT:
#
#       g1 = ILT address of event at inl2 nest level
#       g4 = assoc. CIMT address
#       g7 = ILT of event at inl1 nest level
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
#
c$rinit1:
        mov     g0,r15                  # Save g0
        ldob    inchipi(g7),g0          # g0 = port number
        call    MAG$del_all_inact_imt
#
        ld      il_cr(g1),r4            # r4 = my completion handler routine
        cmpobne 0,r4,.rinit100          # Jif a completion handler defined
        lda     c$comptmgnt,r4          # r4 = completion handler routine to
                                        #  call
.rinit100:
        callx   (r4)                    # call my completion handler routine
        mov     r15,g0                  # Restore g0
        ret

#
#******************************************************************************
#
#  NAME: c$disvp1
#
#  PURPOSE:
#
#       Handles an Reset and Initialize event received.
#
#  DESCRIPTION:
#
#       Delete all IMTs for the specified virtual port.
#
#  CALLING SEQUENCE:
#
#       call    c$rinit1
#
#  INPUT:
#
#       g1 = ILT address of event at inl2 nest level
#       g4 = assoc. CIMT address
#       g7 = ILT of event at inl1 nest level
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
#
c$disvp1:
#
# --- Find the active IMT associated with the virtual port ID and notify
#       the associated device driver of an offline event.
#
        ldos    invpid(g7),r15          # r15 = virtual port ID
        ld      ci_imthead(g4),g5       # g5 = IMT address
        cmpobe  0,g5,.disvp100          # Jif no active IMTs
        lda     ci_imthead(g4),r5       # r5 = pseudo last IMT checked
        mov     0,r6                    # r6 = value to store as tail if
                                        #  IMT is current tail member
        ld      ci_imttail(g4),r7       # r7 = current tail member
.disvp10:
        ldos    im_vpid(g5),r4          # r4 = IMT Virtual port ID
        cmpobe  r4,r15,.disvp20         # Jif IDs match
#
        mov     g5,r5                   # r5 = last IMT checked
        mov     g5,r6                   # r6 = new tail member if match
        ld      im_link(g5),g5          # g5 = next active IMT
        cmpobne 0,g5,.disvp10           # Jif more active IMTs to check
        b       .disvp100               # No more active IMTs to check
#
# --- An IMT match has been found
#
#  INPUT:
#       g1 = ILT address of event at inl2 nest level
#       g4 = assoc. CIMT address
#       g5 = IMT that matched
#       g7 = ilt address of event at inl1 nest level
#
.disvp20:
        ld      im_link(g5),r3          # r3 = next IMT address on list
        st      r3,im_link(r5)          # unlink IMT from list
        cmpobne g5,r7,.disvp30          # Jif not last IMT on list
        st      r6,ci_imttail(g4)       # save new tail member
.disvp30:
        mov     0,r4                    # r4 = 0 = LUN index
        st      r4,im_link(g5)
        ld      im_ehand(g5),r6         # get default event handler table
        ld      dd_offline(r6),r6       # get offline event handler routine
#
#*******************************************************************************
#
#       Interface to device driver routine:
#       -----------------------------------
#
#       Input:
#               g4 = CIMT address
#               g5 = IMT address
#
#       Output:
#               None.
#
#       reg. g5-g6/r3-r15 can be destroyed.
#
#*******************************************************************************
#
        callx   (r6)                    # call default routine
        mov     r3,g5                   # Get next IMT
        cmpobne 0,g5,.disvp10           # Jif more IMTs

.disvp100:
        ld      il_cr(g1),r4            # r4 = my completion handler routine
        cmpobne 0,r4,.disvp_100         # Jif a completion handler defined
        lda     c$comptmgnt,r4          # r4 = completion handler routine to
                                        #  call
.disvp_100:
        callx   (r4)                    # call my completion handler routine
        ret
#
#******************************************************************************
#
#  NAME: c$d$cmdrecv
#
#  PURPOSE:
#
#       Handles a SCSI command received in the case where no CIMT
#       can be associated with the initiator.
#
#  DESCRIPTION:
#
#       Returns check condition status with appropriate SENSE data.
#
#  CALLING SEQUENCE:
#
#       call    c$d$cmdrecv
#
#  INPUT:
#
#       g1 = ILT address
#       g7 = ILT param structure
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       g5-g6 can be destroyed.
#
#******************************************************************************
#
        .data
#
c$inquiry_tbl:
        .byte   dtxferi,scnorm,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  1                       # # SGL descriptors
        .short  0                       # sense length
#
c$reqsns_tbl:
        .byte   dtxferi,scnorm,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  1                       # # SGL descriptors
        .short  0                       # sense length
#
c$other_tbl:
        .byte   dtxfern,scechk,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   MAGD$sense_nolun        # sense pointer
        .short  0                       # # SGL descriptors
        .short  sensesize               # sense length
#
        .text
#
c$d$cmdrecv:
        PushRegs(r3)                    # Save all G registers
        ldob    scexecc(g7),g8          # g8 = execution flags from IOCB
        ldconst 0,g0                    # Use g0 as zero
        and     3,g8,g8                 # isolate data direction flags
        cmpobne 0,g8,.cdcmdrecv10       # Jif data direction flag(s) set
        st      g0,scdatalen(g7)        # clear data length field
.cdcmdrecv10:
        ld      sccdb(g7),g8            # g8 = pointer to CDB
        ldob    (g8),r4                 # r4 = command byte
        ldconst inquiry,r5              # r5 = INQUIRY command code
        ldconst reqsense,r6             # r6 = REQUEST SENSE command code
        cmpobne r4,r5,.cdcmdrecv50    # Jif not INQUIRY command
#
# --- Process INQUIRY command if it is a regular Inquiry (not EVPD)
#

        ldob    1(g8),r4                # r4 = CDB byte with op. flags
        ldob    2(g8),r5                # r5 = Page/Operation code byte
        and     0x03,r4,r4              # mask off reserved bits
        cmpobe  0,r4,.cdcmdrecv20       # Jif a regular INQUIRY request
        ldq     inquiry_tbl2,r4         # load op. values into regs.
        ld      inquiry_tbl2+16,r8

        cmpobe  0x01,r4,.cdcmdrecv11    # Jif EVPD flag set
        b       .cdcmdrecv200           # Report the Invalid Field in CDB
#
.cdcmdrecv11:
        cmpobe  0,r5,.cdcmdrecv18       # Jif Page 0 VPD
        ldconst 0x80,r4                 # r4 = page 80 VPD code
        cmpobe  r4,r5,.cdcmdrecv14      # Jif Page 0x80 VPD
        ldconst 0x83,r4                 # r4 = page 83 VPD code
        cmpobne r4,r5,.cdcmdrecv200     # Jif not Page 0x83 VPD
#
# --- Process INQUIRY Vital Product Data Page 0x83 request
# ---
        mov     0,g0                    # g0 = buffer size for allocation
        lda     inqpg_83_size,r9        # g0 = SGL/buffer combo memory size
# Determine if WHQL compliance is active and set parameter accordantly
        ldob    MAGD_SCSI_WHQL,r4       # is WHQL compliance active?
        cmpobe  0,r4,.cdcmdrecv11_5
        lda     inqpg_83_WHQLsize,r9   # g0 = SGL/buffer combo memory size
.cdcmdrecv11_5:
c       g0 = m_asglbuf(g0);             # allocate a SGL and buffer
        lda     inqpg_00,r4             # r4 = INQUIRY data base address
        st      r4,sghdrsiz+sg_addr(g0) # save INQUIRY data pointer in SGL
        ld      sghdrsiz+sg_addr(g0),g5 # g5 = buffer address
        ldconst 0x7f,r4                 # load default for dev type and perph qualifier
        stob    r4,(g5)                 # indicate LUN not supported
.cdcmdrecv11_6:
        ldob    4(g8),r4                # r4 = alloc. length from CDB
        cmpo    r4,r9                   # check if alloc. length < INQUIRY size
        sell    r9,r4,r9                # r9 = size of transfer to host
        st      r9,sghdrsiz+sg_len(g0)  # save size of data in SGL
        ldq     c$inquiry_tbl,r4        # load op. values into regs.
        ld      c$inquiry_tbl+16,r8
        mov     g0,r6                   # r6 = SGL pointer
        b       .cdcmdrecv200           # and complete processing of I/O req.
#
# --- Process INQUIRY Vital Product data Page 0x80 request
#
.cdcmdrecv14:
        mov     0,g0                    # g0 = buffer size for allocation
        lda     inqpg_80_size,r9        # r9 = my inquiry data size
        b       .cdcmdrecv11_5
#
# --- Process INQUIRY Vital Product Data Page 0x00 request
#
.cdcmdrecv18:
        mov     0,g0                    # g0 = buffer size for allocation
        lda     inqpg_00_size,r9       # r9 = my inquiry data size
        b       .cdcmdrecv11_5

.cdcmdrecv20:
        mov     0,g0                    # g0 = buffer size for allocation
        lda     c$inquiry_size,r9       # r9 = my inquiry data size
c       g0 = m_asglbuf(g0);             # allocate a SGL and buffer
        lda     c$inquiry1,r4           # r4 = INQUIRY data base address
        st      r4,sghdrsiz+sg_addr(g0) # save INQUIRY data pointer in SGL
        b       .cdcmdrecv11_6

.cdcmdrecv50:
        cmpobne r4,r6,.cdcmdrecv100   # Jif not REQUEST SENSE command
#
# --- Process REQUEST SENSE command
#
        mov     0,g0                    # g0 = buffer size for allocation
        lda     sensesize,r9            # r9 = my sense data size
c       g0 = m_asglbuf(g0);             # allocate a SGL and buffer
        lda     MAGD$sense_nolun,r4     # r4 = SENSE data base address
        ldob    MAGD$targetrdy,r5       # r5 = target driver ready to go
                                        #  indication
        cmpobne 0,r5,.cdcmdrecv55       # Jif target driver is ready to go
        lda     MAGD$sense_uninit,r4    # r4 = SENSE data base address if
                                        #  still initializing.
.cdcmdrecv55:
        st      r4,sghdrsiz+sg_addr(g0) # save SENSE data pointer in SGL
        b       .cdcmdrecv11_6
#
# --- Process all other type commands
#
.cdcmdrecv100:
        ldq     c$other_tbl,r4          # load op. values into regs.
        ld      c$other_tbl+16,r8
        ldob    MAGD$targetrdy,r9       # r9 = target driver ready to go
                                        #  indication
        cmpobne 0,r9,.cdcmdrecv110    # Jif target driver is ready to go
        lda     MAGD$sense_uninit,r7    # r7 = SENSE data base address if
                                        #  still initializing.
.cdcmdrecv110:
#
# --- Common code for processing I/O request for c$d$cmdrecv routine
#
#  INPUT:
#
#       r4 = xlcommand-xlscsist-xlfcflgs-00
#       r5 = rel. offset (xlreloff)
#       r6 = SGL pointer (xlsglptr)
#       r7 = Sense data pointer (xlsnsptr)
#       r8 = Sense length (xlsnslen)-SGL length (xlsgllen)
#       g1 = primary ILT at inl2 nest level
#       g7 = assoc. ILT param. structure
#
.cdcmdrecv200:
        mov     g1,g9                   # g9 = pri. ILT at inl2 nest level
        lda     c$mag1_iocr,r11         # r11 = I/O completion routine
c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
        cmpobe  0,r6,.cdcmdrecv210      # Jif no SGL specified
        lda     sghdrsiz(r6),r6         # point to descriptor area
.cdcmdrecv210:
        stq     r4,xlcommand(g1)        # stuff op. req. record
        mov     g7,r9                   # r9 = assoc. ILT parameter structure
        mov     g9,r10                  # r10 = pri. ILT at inl2 nest level
        st      r11,otl2_cr(g1)         # save completion routine
        stt     r8,xlsgllen(g1)         # save sense length/SGL length/assoc.
                                        #  ILT param. structure/pri. ILT at
                                        #  inl2 nest level
        ld      scdatalen(g7),r11       # r11 = expected data length
        bbc     xlsndsc+16,r4,.cdcmdrecv300 # Jif ending status not being
                                        #  presented
        cmpobe 0,r6,.cdcmdrecv300       # Jif no data being transferred
        ld      sg_len(r6),r10          # r10 = length of data transfer
        subo    r10,r11,r11             # r11 = residual length
.cdcmdrecv300:
        st      r11,xlreslen(g1)        # save residual length in ILT
        mov     g1,r11                  # r11 = my ILT nest level ptr.
        lda     ILTBIAS(g1),g1          # inc. to next nest level
        st      r11,otl3_OTL2(g1)       # save param. ptr. in next nest
        call    ISP$receive_io          # issue channel directive
#
        PopRegsVoid(r3)                 # Restore all G registers
        ret
#
#************************************************************************
#
#  NAME: c$mag1_iocr
#
#  PURPOSE:
#       Provides general task I/O completion processing for c$d$cmdrecv
#       processed commands.
#
#  DESCRIPTION:
#       Gets the associated primary ILT from the secondary
#       ILT and calls the originator's completion handler
#       routine. It then releases any resources associated
#       with the secondary ILT, including the secondary ILT.
#
#  CALLING SEQUENCE:
#       call    c$mag1_iocr
#
#  INPUT:
#       g0 = I/O completion status code
#       g1 = sec. ILT address at my nest level
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       None.
#
#************************************************************************
#
c$mag1_iocr:
        PushRegs(r3)                    # Save all G registers
        mov     g1,r13                  # r13 = sec. ILT address
        ld      xl_INL2(g1),g1          # g1 = primary ILT at inl2 nest
        lda     -ILTBIAS(g1),g1         # back to originator's ILT nest level
        ld      il_cr(g1),g2            # g2 = originator's completion routine
        callx   (g2)                    # call originator's completion handler
        ld      xlsglptr(r13),g0        # check if SGL/buffer assigned to ILT
        cmpobe  0,g0,.c$iocr10          # Jif no SGL/buffer assigned
        lda     -sghdrsiz(g0),g0        # back up to start of SGL
        st      0,xlsglptr(r13)         # Make sure cannot use it again.
        call    M$rsglbuf               # release SGL/buffer combo
.c$iocr10:
        PopRegsVoid(r3)                 # Restore all G registers
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate secondary ILT
        ret
#
#******************************************************************************
#
#  NAME: c$compimno
#
#  PURPOSE:
#
#       Handles the completion processing of an immediate notify
#       event with respect to the channel interface.
#
#  DESCRIPTION:
#
#       Sets up and returns to the appropriate routine to complete
#       the processing of an immediate notify event.
#
#  CALLING SEQUENCE:
#
#       call    c$compimno
#
#  INPUT:
#
#       g1 = ILT address nested to channel driver level
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       g1 destroyed.
#
#******************************************************************************
#
c$compimno:
        lda     -ILTBIAS(g1),g1         # g1 = previous ILT nest level
        b       ISP$notify_ack          # deliver ack completion
#
#******************************************************************************
#
#  NAME: c$comptmgnt
#
#  PURPOSE:
#
#       Handles the completion processing of a SCSI task management
#       event with respect to the channel interface.
#
#  DESCRIPTION:
#
#       Sets up to return to the caller's completion routine specified
#       in the ILT to complete the processing of the event.
#
#  CALLING SEQUENCE:
#
#       call    c$comptmgnt
#
#  INPUT:
#
#       g1 = ILT address nested to channel driver level
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       g1 destroyed.
#
#******************************************************************************
#
c$comptmgnt:
        lda     -ILTBIAS(g1),g1         # g1 = previous ILT nest level
        ld      il_cr(g1),r6            # r6 = caller's completion routine
c       if (r6 == 0) {                  # If no handler defined
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c           put_ilt(g1);                # deallocate ILT.
            ret;
c       }
        bx      (r6)                    # return to caller's routine
#
#******************************************************************************
#
#  NAME: c$findimt
#
#  PURPOSE:
#       Looks for an appropriate IMT to pair with an incoming
#       interface event from a new initiator.
#
#  DESCRIPTION:
#       Looks on the inactive IMT list for a match.
#       6). An IMT non-specific for the channel interface the event was
#           received on and is MAC address specific and matches the
#           initiator's MAC address.
#
#       If none of the preceding checks finds an appropriate match
#       the request is denied.
#
#  CALLING SEQUENCE:
#       call    c$findimt
#
#  INPUT:
#       g1 = ILT address of incoming event
#       g4 = assoc. CIMT address
#       g7 = ILT param structure
#
#  OUTPUT:
#       g5 = IMT address if appropriate match found
#            IMT is removed from the inactive IMT list
#       g5 = 0 if no appropriate match found (request denied)
#       g5 = FINDIMT_INCONSISTENT serverdb and IMT table mismatch discard IO

#  REGS DESTROYED:
#       g5 destroyed.
#
#******************************************************************************
#
c$findimt:
#
        ldconst 0,g5                    # Clear IMT pointer (return value)
#
# --- Use the initiator ID and channel # to index into the
#     servord and servdb tables to get the WWN for this server.
#
        ldos    idf_init(g7),r11        # r11 = initiator ID
        ldob    idf_ci(g7),r12          # r12 = initiator's interface #
        ld      C_imt_head,r10          # r10 = first IMT on inactive list
#
.if FE_ISCSI_CODE
#
# --- if iSCSI Interfaces - get the ISID
#
c       r6 = ICL_IsIclPort((UINT8)r12);
        cmpobe   TRUE,r6,.findimt_icl01 # Jif ICL port
        ld      iscsimap,r6             # Get iscsimap bitmap
        bbc     r12,r6,.findimt03       # Jif not iSCSI port
.findimt_icl01:
#
        PushRegs(r3)                    # Save all the registers
        ldos    idf_vpid(g7),r5         # Get virtual port ID
        mov     r5,g0                   # Set input parameter
        mov     r11,g1                  # Set input parameter
        call    iSCSI_GetISID           # Call the C code
        movl    g0,r14                  # Save the return value
        cmpobne 0,g0,.findimt02         # Jif not NULL
        cmpobne 0,g1,.findimt02         # Jif not NULL
        PopRegsVoid(r3)                 # Restore the registers
        b       .findimtret
#
.findimt02:
        mov     r5,g2                   # Set input parameter
        mov     r11,g3                  # Set input parameter
        call    fsl_findIMT             # Call the C code
        mov     g0,r9                   # Save the return value
        PopRegsVoid(r3)                 # Restore the registers

        cmpobe  0,r9,.findimt90         # Jif no IMTs

        cmpobe  0xffffffff,r9,.findimtret  # Jif IMT creation not allowed

        mov     r9,g5                   # IMT in g5
        ld      C_imt_head,r10          # r10 = first IMT on inactive list
        b       .foundimt1              # and use this IMT
#
.findimt03:
#
.endif  # FE_ISCSI_CODE
#
        movl    0,r14                   # r14-r15 = default initiator WWN
        shlo    srvdbshift,r11,r7       # r7 = index into Port ID/Server Name
                                        #  translation table (ordinal * size)
        ld      servdb[r12*4],r6        # r6 = Port ID/Server Name translation
                                        #  table address to use
        addo    r7,r6,r6                # r6 = translation table record for
                                        #  initiator
        ldl     ieeeaddr(r6),r14        # r14-r15 = initiator WWN
#                                       #  from translation table
#
        cmpobne 0,r14,.findimt05        # Jif WWN is non-zero
        cmpobe  0,r15,.findimtret       # Jif WWN is zero
#
# --- Check if this is a XIOtech Controller
#
.findimt05:
c       r3 = M_chk4XIO(*(UINT64*)&r14); # Check if this is a XIOtech controller
#
# --- Check what type of port this request was received on
#
        movl    g0,r8                   # save g0-g1
        mov     r12,g0                  # g0 = initiator's interface #
        ldos    idf_vpid(g7),g1         # g1 = virtual port (target) ID
        call    C$findTarget            # find target ID for this virtual port
        mov     g0,r4                   # r4 = ptr to target structure
        movl    r8,g0                   # restore g0-g1
#
.if     DEBUG_FLIGHTREC_CD
        ldconst frt_cd_findimt,r8       # Type
        st      r8,fr_parm0             # cdriver - c$findimt
        st      g0,fr_parm1             # Chip ID
        st      g1,fr_parm2             # virtual port (target) ID
        st      r11,fr_parm3            # Initiator's LID
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_CD
#
# --- Check if a target structure was found.  If none found, then the
#     target structure which reflect the hardware does not match the
#     TGD structure which reflects the desired configuration.
#
        cmpobe  0,r4,.findimtret        # Jif no target structure
#
# --- Check if this request was received on the control port
#
        ldos    tartid(r4),r8           # Get the Target ID
        ldconst MAXTARGETS,r9
        cmpobl  r8,r9,.findimt20        # Jif not control port
#
.if     MAG2MAG
#
# --- The request came in on the control port.  Since Magnitude communications
#     are supported, an IMT is created only for XIOtech MAGNITUDE or Bigfoot
#     controller FE ports.
#
        cmpobe  0,r3,.findimtret        # Jif not a XIOtech controller
        mov     r14,r13                 # Get MSW WWM
        extract 12,4,r13                # Get 'e' nibble from WWN
        cmpobe  0,r13,.findimt10        # Jif a MAGNITUDE port
        ldconst (WWNFPort>>20)&0xF,r3   # Value for XIOtech FE port
        cmpobe   r3,r13,.findimt10      # Jif a XIOtech FE target port
        ldconst (WWNCPort>>20)&0xF,r3   # Value for XIOtech FE control port
        cmpobe   r3,r13,.findimt10      # Jif a XIOtech FE control port
        b       .findimtret
#
# --- Assign a target number to the control port
#
.findimt10:
        ldconst MAXTARGETS,r5           # Add channel to MAXTARGETS
        addo    r12,r5,r5               # r5 = Target ID for control port
        b       .findimt40
.else   # MAG2MAG
#
# --- The request came in on the control port. Since Magnitude communications
#     are not supported, an IMT is not created.
#
        b       .findimtret
.endif  # MAG2MAG
#
# --- The request was not received on the control port.
#
.findimt20:
        ldos    tartid(r4),r5           # Get the Target ID
        ldos    tarentry(r4),r6         # Get the entry number
#
.if     MAG2MAG
.findimt40:
.endif  # MAG2MAG
        cmpobe  0,r10,.findimt90        # Jif no IMTs on inactive list
#
# --- Check for matching WWN and Target ID
#
        mov     r10,r9                  # get working IMT list reg
.findimt56:
        ldl     im_mac(r9),r6           # r6-r7 = WWN address for image
        cmpobne r14,r6,.findimt58       # Jif WWN address does NOT match
        cmpobne r15,r7,.findimt58       # Jif WWN address does NOT match
        ldos    im_tid(r9),r8           # r8 = Target ID
        cmpobne r5,r8,.findimt58        # Jif Target ID NOT match
        b       .foundimt1              # and use this IMT
#
.findimt58:
        ld      im_link(r9),r9          # r9 = next IMT on list
        cmpobne 0,r9,.findimt56         # Jif more IMTs to check
#
# --- Now check the active IMT list (sanity check)
#
        ld      ci_imthead(g4),r9       # Get first active IMT
        cmpobe  0,r9,.findimt90         # Jif no IMTs on active list
#
.findimt66:
        ldl     im_mac(r9),r6           # r6-r7 = WWN address for image
        cmpobne r14,r6,.findimt68       # Jif WWN address does NOT match
        cmpobne r15,r7,.findimt68       # Jif WWN address does NOT match
        ldos    im_tid(r9),r8           # r8 = Target ID
        cmpobne r5,r8,.findimt68        # Jif Target ID NOT match
#
# --- Hey, what's this IMT doing here.  This is a fatal error.  We could be
#       passing data from one server's VDisks to a different one (LIDs could
#       be mixed up).
#
#       SMW - This is no longer fatal to the system we are going to simply discard this IO
#
        ldconst FINDIMT_INCONSISTENT,g5           # set up -1 return code
        b       .findimtret

.findimt68:
        ld      im_link(r9),r9          # r9 = next IMT on list
        cmpobne 0,r9,.findimt66         # Jif more IMTs to check
#
# --- No appropriate IMT found ------------------------------------------------
#
.findimt90:
#
        mov     g0,r8                   # save g0
        movl    g6,r6                   # save g6-g7
        mov     r5,g0                   # g0 = target ID
        mov     r11,g5                  # g5 = initiator ID
        movl    r14,g6                  # g6-g7 = initiator's WWN
        call    MAGD$create_image       # try to create a new image for this
                                        #  initiator
        movl    r6,g6                   # restore g6-g7
        mov     r8,g0                   # restore g0
#
        b       .foundimt110            # and return to caller
#
# --- Found an IMT ------------------------------------------------------------
#     Unlink the IMT from the inactive list
#
#  INPUT:
#
#       r9 = address of IMT to use for initiator
#       r10 = first IMT on inactive IMT list
#       r11 = initiator ID
#       r12 = initiator's interface #
#       r14-r15 = WWN
#
.foundimt1:
        cmpobne r9,r10,.foundimt10      # Jif not first on list
        ld      im_link(r9),r8          # unlink from inactive list
        st      r8,C_imt_head           # save new head pointer
        cmpobne 0,r8,.foundimt100       # Jif not last on list
        st      r8,C_imt_tail           # clear list tail pointer
        b       .foundimt100
#
.foundimt10:
        mov     r10,r8                  # r8 = previous IMT address
        ld      im_link(r10),r7         # r7 = current match check reg.
.foundimt20:
        cmpobe  r9,r7,.foundimt30       # Jif a match
        mov     r7,r8                   # save new previous IMT address
        ld      im_link(r7),r7          # r7 = next IMT on list
        cmpobne 0,r7,.foundimt20        # Jif another IMT to check
#
# --- Didn't find IMT on list. Should never occur!!! --------------------------
#
        b       .foundimt100            # Just keep going
#
.foundimt30:
        ld      im_link(r9),r7          # r7 = next IMT address
        st      r7,im_link(r8)          # unlink IMT from list
        cmpobne 0,r7,.foundimt100       # Jif not the last on list
        st      r8,C_imt_tail           # save new tail pointer
.foundimt100:
        mov     r9,g5                   # put winner in return reg. for caller
.foundimt110:
        cmpobe  0,g5,.findimtret        # Jif no IMT found
#
# --- Save CIMT info in IMT
#
.if MULTI_ID
        ldos    idf_vpid(g7),r13        # Get virtual port ID
.else   # MULTI_ID
        ldconst 0,r13                   # Use virtual port ID = 0
.endif  # MULTI_ID
        st      g4,im_cimt(g5)          # save assoc. CIMT in IMT
        stos    r11,im_fcaddr(g5)       # save initiator's FC address
        stos    r13,im_vpid(g5)         # save virtual port ID
#
        PushRegs(r3)
        ldob    ci_num(g4),g0           # g0 = interface #
        mov     r13,g1                  # g1 = virtual port ID for initiator
        call    ISP_IsPrimaryPort
        mov     g0,r5
        PopRegsVoid(r3)
        cmpobe  0,r5,.findimtret        # Jif not the primary port
#
# --- We either took an IMT off the inactive queue and placed it
#       on the active queue or we created a new IMT. Check if
#       the IMT is a XIO-to-XIO IMT and if so check to see if
#       it might need to be associated with a LTMT.
#
        ldob    im_flags(g5),r4         # r4 = IMT flags byte
        bbc     im_flags_mtml,r4,.findimtret # Jif not a XIO-to-XIO link
        ld      im_ltmt(g5),r4          # r4 = assoc. LTMT address
        cmpobne 0,r4,.findimtret        # Jif IMT already assoc. with LTMT
        ldl     im_mac(g5),r14          # r14-r15 = IMT WWN to check for
#
        ld      ci_ltmthd(g4),r4        # r4 = first LTMT on list
.foundimt300:
        cmpobe  0,r4,.findimtret        # Jif no LTMTs on list
        ldob    ltmt_type(r4),r5        # r5 = LTMT type code
        cmpobne ltmt_ty_MAG,r5,.foundimt330 # Jif not a MAGNITUDE type
        ld      ltmt_imt(r4),r5         # r5 = assoc. IMT address
        cmpobne 0,r5,.foundimt330       # Jif an IMT assoc. with LTMT
        ld      ltmt_tmt(r4),r5         # r5 = assoc. TMT address
        cmpobe  0,r5,.foundimt330       # Jif no TMT assoc. with LTMT
#
        ldl     tm_P_name(r5),r6        # r6-r7 = MAC address of target
        cmpobne r6,r14,.foundimt330     # Jif MAC address does not match
        cmpobne r7,r15,.foundimt330     # Jif MAC address does not match
#
        ldob    tm_state(r5),r6         # get current state
        cmpobe  tmstate_active,r6,.foundimt310 # JIf active state
c fprintf(stderr,"%s%s:%u RAGX: .foundimt300: TMT(0x%lx) state(0x%lx) not active ltmt(0x%08lx) imt(0x%08lx)\n", FEBEMESSAGE, __FILE__, __LINE__,r5,r6,r4,g5);
        b       .findimtret             # and finish initializing IMT
#
.foundimt310:
#
# --- A target match is found. Associate LTMT and IMT with each other.
#
        st      g5,ltmt_imt(r4)         # save IMT address in LTMT
        st      r4,im_ltmt(g5)          # save LTMT address in IMT
        movq    g4,r12
        mov     r4,g4
        call    lld$send_MLE            # send MAG link established VRP to
        movq    r12,g4
        b       .findimtret             # and finish initializing IMT
#
.foundimt330:
        ld      ltmt_link(r4),r4        # r4 = next LTMT on list
        b       .foundimt300            # and check next LTMT if defined
#
.findimtret:
        ret
#
#******************************************************************************
#
#  NAME: c$add2cmd
#
#  PURPOSE:
#
#       Increments the command counters for the associated interface
#       in the statistics area.
#
#  DESCRIPTION:
#
#       Updates the statistics counters of the specified interface for
#       a new incoming CDB.
#
#  CALLING SEQUENCE:
#
#       call    c$add2cmd
#
#  INPUT:
#
#       g5 = assoc. IMT address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
c$add2cmd:
#
# --- Adjust aggregate command count
#
        lda     im_stagg(g5),r5         # point to stats area
?       ldl     imst_cmds(r5),r6        # get total # cmds
        cmpo    1,0                     # Clear carry
        addc    1,r6,r6                 # inc. aggregate total # cmds
        addc    0,r7,r7                 # inc. aggregate total # cmds
        stl     r6,imst_cmds(r5)        # save updated counts
#
# --- Adjust periodic command count
#
        lda     im_stinprog(g5),r5      # point to stats area
?       ldl     imst_cmds(r5),r6        # get total # cmds
        cmpo    1,0                     # Clear carry
        addc    1,r6,r6                 # inc. aggregate total # cmds
        addc    0,r7,r7                 # inc. aggregate total # cmds
        stl     r6,imst_cmds(r5)        # save updated counts
        ret
#
#
        .data
#
#******************************************************************************
#
# _____________________ Non-SENSE DATA DEFINITIONS ____________________________
#
#******************************************************************************
#
# --- Basic Inquiry command data
#
        START_SH_DATA_SECTION
c$inquiry1:
        .byte   0x7f            # peripheral qualifier/device type
        .byte   00              # RMB/reserved
        .byte   02              # ISO/ECMA/ANSI version
        .byte   02              # AERC,TrmTsk,NormACA,HiSupport,response data
                                #  format
        .byte   36-4            # additional length (n-4)
        .byte   00              # SCCS,reserved
        .byte   02              # BQue,EncServ,VS,MultiP,MChngr,AckReqQ,Addr32,
                                #  Addr16
        .byte   00              # RelAdr,WBus32,WBus16,Sync,Linked,TranDis,
                                #  Cmdque,VS
        .ascii  "XIOtech "      # vendor identification
        .ascii  "Magnitude 3D    " # product identification field
c$inquiry1_rev:
        .ascii  "1.00"          # product revision level
c$inquiry1end:
#
        .set    c$inquiry_size,c$inquiry1end-c$inquiry1 # size of c$inquiry1
                                                        #  data
        END_SH_DATA_SECTION
        .align  4
#
#******************************************************************************
#
#  NAME: C$findTarget
#
#  PURPOSE:
#
#       Looks for a target for a specified Virtual port ID (Loop ID).
#
#  DESCRIPTION:
#
#       The controller serial number is obtained.  The list of TAR
#       structures are search for the target with a matching
#       owner (controller serial number), channel, and virtual port ID.
#       The target ID for that TAR structure is returned.
#
#  CALLING SEQUENCE:
#
#       call    C$findTarget
#
#  INPUT:
#
#       g0 = channel
#       g1 = Virtual port ID
#
#  OUTPUT:
#
#       g0 = pointer to target structure
#
#  REGS DESTROYED:
#
#      None.
#
#******************************************************************************
#
        .text
#
C$findTarget:
#
        ld      tar[g0*4],g0            # Get TAR anchor
#
.if MULTI_ID
#
.cft10:
        cmpobe  0,g0,.cft100            # Jif no targets left
#
#
# --- Check Virtual port ID in the target structure
#
        ld      tarvpid(g0),r5          # Get virtual port ID
        cmpobe  r5,g1,.cft100           # Jif virtual port matches
#
        ld      tarfthd(g0),g0          # Get the next TAR structure
        b       .cft10
.endif  # MULTI_ID
#
# --- Exit
#
.cft100:
        ret
#
#******************************************************************************
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
