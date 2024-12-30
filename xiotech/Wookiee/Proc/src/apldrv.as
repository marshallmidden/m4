# $Id: apldrv.as 159663 2012-08-22 15:36:42Z marshall_midden $
#******************************************************************************
#
#  NAME: apldrv.as
#
#  Copyright (c) 1998 - 2010 Xiotech Corporation.  All rights reserved.
#
#******************************************************************************
.if     INITIATOR
#
# --- global function declarations
#
#******************************************************************************
#
#  NAME: apl$timer
#
#  PURPOSE:
#       Timer task.
#
#  DESCRIPTION:
#       PNT -> Process Next Timer
#
#  CALLING SEQUENCE:
#       fork
#
#  INPUT:
#       g4 = icimt address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
        .text
#
apl$timer:
.ifdef HISTORY_KEEP
c CT_HISTORY_OFF();
.endif  # HISTORY_KEEP
#
# --- this area determines if a tmt have been lost for a given amount of time.
        ld      ici_tmtQ(g4),g5         # g5 = possible TMT
        cmpobe.t 0,g5,.timer_1000       # Jif no or last entry on queue - PNT
        lda     ici_tmtQ(g4),r5         # r5 = address of tmt queue
#*****************************************************************
#
# --- Process controller loss timer
#
#*****************************************************************
#
        ldob    ici_FCTOI(g4),r3        # r3 = controller t/o inhibit flag
        cmpobe  TRUE,r3,.timer_200      # Jif timer inhibited
#
.timer_100:
        ldos    tm_tmr0(g5),r4          # r4 = timer 0 value
        cmpobe.t 0,r4,.timer_190        # Jif no timer set
        cmpobe.f 1,r4,.timer_130        # Jif timer about to expire
        subo    1,r4,r4                 # decrement timer value
        stos    r4,tm_tmr0(g5)          # save new value
        b       .timer_190              # and go to next TMT on list
#
.timer_130:
        ldob    ici_chpid(g4),r4        # r4 = chip instance
        ld      iscsimap,r3             # r3 = iscsi port map
        bbs     r4,r3,.timer_140        # Jif iSCSI port
#
        PushRegs(r3)
        mov     g5,g0
        ld      tm_link(g0),r4          # r6 = link to next tmt
        call    I_RemoveTarget
        PopRegsVoid(r3)
        mov     r4,g5                   # g5 = next TMT
        b       .timer_195              # and go to next TMT on list
#
.timer_140:
.if     MAG2MAG
        mov     g0,r4                   # save g0
        call    LLD$pre_target_gone     # check if OK to blow off target
        mov     g0,r7                   # r7 = pre-target gone flag
        mov     r4,g0                   # restore g0
        cmpobne.f TRUE,r7,.timer_190    # Jif not OK to blow off target
.endif  # MAG2MAG
        ldconst 0,r7                    # r7 = zero index
        stos    r7,tm_tmr0(g5)          # clear timer value
        ldconst MAXLUN,r13              # r13 = maxlun
#
.timer_150:
        ld      tm_tlmtdir(g5)[r7*4],g6 # g6 = possible tlmt
        cmpobe.t 0,g6,.timer_165        # jif no tlmt defined
#
        call    i$removeLUN_no          # remove this LUN
                                        # ***********************************
                                        # *** with no session termination ***
                                        # *** indication                  ***
                                        # ***********************************
#
.timer_165:
        addo    1,r7,r7                 # bump index
        cmpobg.t r13,r7,.timer_150      # Jif no end of directory
#
# --- unlink tmt from queue
#
        ld      tm_link(g5),r6          # r6 = link to next tmt
        st      r6,(r5)                 # save link in previous tmt
.if     MAG2MAG
#
.if     ITRACES
        call    it$trc_target_gone
.if     debug_tracestop4
        ldconst 0,r4                    # Use r4 as zero
        stos    r4,ici_tflg(g4)         # clear trace flags to stop traces
.endif  # debug_tracestop4
.endif  # ITRACES
        call    LLD$target_GONE         # indicate target is gone
                                        # input:
                                        #   g5 = tmt
                                        # output:
                                        #   none:
.endif  # MAG2MAG
#
# --- logoff the current port
#
        movq    g0,r8                   # save g0-g3
        ldconst 0,r4                    # Use r4 as zero
        ldos    tm_lid(g5),g1           # g1 = lid
#
        st      r4,ici_tmdir(g4)[g1*4]  # clear TMT in CIMT dir
#
#       bbc     7,g1,.timer_180         # jif not an fabric lid
#
        mov     g1,g0                   # g0 = old LID
        call    i$put_lid               # return lid
        ldob    ici_chpid(g4),g0        # g0 = chip instance
        ld      tm_alpa(g5),g2          # g2 = alpa (PID)
#
                                        # g0 = chip instance
                                        # g1 = lid from lidt
                                        # g2 = alpa
#
# ---   Handle ICL port case, where the bit is not set in the bitmap
#
c       g3 = ICL_IsIclPort((UINT8)g0);
        cmpobe  TRUE,g3,.timer_icl01
        ld      iscsimap,g3             # g3 = iscsi port map
        bbc     g0,g3,.timer_170        # Jif not iSCSI port
.timer_icl01:
        PushRegs(r3)
        call    fsl_logout
        PopRegsVoid(r3)
#
        ldconst NO_LID,g3               # g3 = invalid lid
        stos    g3,tm_lid(g5)           # set LID invalid
#
        b       .timer_180              # continue
#
.timer_170:
        ld      ici_mypid(g4),g2        # g2 = my alpa
        call    ISP_LogoutFabricPort    # logoff port
#
.timer_180:
        movq    r8,g0                   # restore g0-g3
.ifdef M4_ADDITION
c   if (((TMT *)g5)->state != tmstate_deltar) {
.endif  # M4_ADDITION
.ifdef M4_DEBUG_TMT
c fprintf(stderr, "%s%s:%u put_tmt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g5);
.endif # M4_DEBUG_TMT
c       put_tmt(g5);                    # release tmt to free pool
        mov     r5,g5                   # setup for next instruction
.ifdef M4_ADDITION
c   } else {
c   fprintf(stderr, "%s%s:%u .timer_180 TMT 0x%08lx in process of being deleted\n", FEBEMESSAGE,__FILE__, __LINE__, g5);
c   }
.endif  # M4_ADDITION
#
.timer_190:
        mov     g5,r5                   # r5 = current tmt address
        ld      tm_link(g5),g5          # g5 = next tmt in queue
.timer_195:
        cmpobne.f 0,g5,.timer_100       # jif there is another tmt
#
#*****************************************************************
#
# --- process task gross timer
#
#*****************************************************************
#
.timer_200:
        ld      ici_actqhd(g4),g6       # g6 = possible tlmt
#
.timer_205:
        cmpobe.t 0,g6,.timer_300        # Jif no or last entry on queue - PNT
        ld      tlm_whead(g6),r15       # r15 = ILT of task at 1st lvl
        cmpobe.f 0,r15,.timer_260       # Jif no tasks active
#
        ld      tlm_tmt(g6),g5          # g5 = tmt address
#
.timer_210:
        mov     r15,g1                  # g1 = ILT
        ldos    oil2_tmr1(g1),r4        # r4 = current gross t/o value
        subo    1,r4,r4                 # decrement timer value
        stos    r4,oil2_tmr1(g1)        # and save it
        ld      il_fthd(g1),r15         # r15 = possible link to next task
        cmpobne.t 0,r4,.timer_250       # jif task timer has not expired
#
# --- trace the timeout
#
.if     ITRACES
        call    it$trc_tsk_TO           # *** trace task timeout event ***
.endif  # ITRACES
#
# --- Default the completion status
#
        ld      oil2_irp(g1),g2         # g2 = irp address
        ldob    oil2_tstate(g1),r4      # r4 = Task State
        ldconst cmplt_IOE,r8            # r8 = IOE completion status
        ldconst ioe_timeout,r9          # r9 = timeout error
        cmpobne oil2_ts_dorm,r4,.timer_220 # Jif task is not dormant
        cmpobne.t TRUE,r3,.timer_220    # Jif timer not inhibited due to Discov.
        ldconst ioe_TODisc,r9           # Show Timeout occurred because
                                        #  Discovery was in progress too long
.timer_220:
        stob    r8,irp_cmplt(g2)        # save completion status
        st      r9,irp_extcmp(g2)       # save extended status info
#
# --- remove task from working queue
#
        call    apl$remtask             # remove task from queue
#
# --- Process the event
#
        movq    g4,r4                   # save g4-g7 in r4-r7
        ldconst Te_timeout,r9           # r9 = event
        ld      oil2_ehand(g1),r10      # r10 = CIMT event handler address
        addo    r10,r9,r10              # add completion event type
        ld      (r10),r10
        callx   (r10)                   # process event
        movq    r4,g4                   # restore g4-g7 from r4-r7
#
# --- check next task
#
.timer_250:
        cmpobne.t 0,r15,.timer_210      # Jif there is another task on queue
#
# --- check next TLMT
#
.timer_260:
        ld      tlm_flink(g6),g6        # g6 = next TLMT
        b       .timer_205
#
#***************************************************************************
#
# --- set wait value and exit
#
#***************************************************************************
.timer_300:
.timer_1000:
.ifdef HISTORY_KEEP
c CT_HISTORY_ON();
.endif  # HISTORY_KEEP
#
        lda     250,g0                  # set up to wait 250 ms
        call    K$twait                 # delay task
        b       apl$timer               # try again
#
#******************************************************************************
#
#  NAME:APL_TimeoutILT
#
#  PURPOSE:
#       Timeout the given ILT
#
#  CALLING SEQUENCE:
#       call    APL_TimeoutILT
#
#  INPUT:
#       g1 = ILT at nest level 2
#       g4 = CIMT
#       g5 = TMT
#       g6 = TLMT
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
# void APL_TimeoutILT(g1=pILT *, g4=ICIMT *, g5=TMT *, g6=TLMT *);
        .globl  APL_TimeoutILT
APL_TimeoutILT:
        ld      oil2_irp(g1),g2         # g2 = irp address
        ldob    oil2_tstate(g1),r4      # r4 = Task State
        ldconst cmplt_IOE,r8            # r8 = IOE completion status
        ldconst ioe_timeout,r9          # r9 = timeout error
        cmpobne oil2_ts_dorm,r4,.toILT_100 # Jif task is not dormant
        ldconst ioe_TODisc,r9           # Show Timeout occurred because
                                        #  Discovery was in progress too long
.toILT_100:
        stob    r8,irp_cmplt(g2)        # save completion status
        st      r9,irp_extcmp(g2)       # save extended status info
#
# --- remove task from working queue
#
        call    apl$remtask             # remove task from queue
#
# --- Process the event
#
        movq    g4,r4                   # save g4-g7 in r4-r7
        ldconst Te_timeout,r9           # r9 = event
        ld      oil2_ehand(g1),r10      # r10 = CIMT event handler address
        addo    r10,r9,r10              # add completion event type
        ld      (r10),r10
        callx   (r10)                   # process event
        movq    r4,g4                   # restore g4-g7 from r4-r7
        ret
#
# void APL_AbortILT(g1=pILT *, g4=ICIMT *, g5=TMT *, g6=TLMT *);
        .globl  APL_AbortILT
APL_AbortILT:
        lda     ILTBIAS(g1),g1          # g1 = ILT to oil1 level
        ldob    tm_chipID(g5),g0        # g0 = chip instance
        call    ISP$ilt_thread_find
        cmpobe.t g1,g0,.abtilt_100      # JIf ILT found
#
# --- ILT not found! This is bad!!!
#       The ILT is probably held up waiting for an IOCB so it is not
#       on any lists to be found.  reset timer and leave it alone.
#
# c fprintf(stderr,"%s%s:%u SEMINOLE: APL_AbortILT - TMT=0x%lX TLMT=0x%lX ILT=0x%lX NOT FOUND!!!\n", FEBEMESSAGE, __FILE__, __LINE__, g5, g6, g1);
        lda     -ILTBIAS(g1),g1         # g1 = ILT to oil2 level
        ldconst OIL2TOVAL,r8
        stos    r8,oil2_tmr1(g1)        # reset timer to avoid a repeat timeout
        b       .abtilt_1000
#
.abtilt_100:
        lda     -ILTBIAS(g1),g1         # g1 = restore ILT at oil2 level
        ldconst OIL2TOVAL,r3            # r3 = 2sec (sec to 250ms increments)
        stos    r3,oil2_tmr1(g1)        # save gross task t/o value
        lda     ILTBIAS(g1),g3          # g3 = ILT at 3rd lvl
        ldos    tm_lid(g5),g1           # g1 = LID
        ldos    tlm_lun(g6),g2          # g2 = LUN
        ldob    tm_chipID(g5),g0        # g0 = chip instance
        call    ISP$abort_iocb          # ABORT the TASK
.abtilt_1000:
        ret
#
#******************************************************************************
#
#  NAME:APL$pfr
#
#  PURPOSE:
#       Allow an application to execute a function on the FC-AL.
#
#  CALLING SEQUENCE:
#       call    APL$pfr
#
#  INPUT:
#       g1 = ILT at nest level 2
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
APL$pfr:
        PushRegs(r3)                    # Save all G registers
        ld      il_misc(g1),g2          # g2 = irp pointer
        ldob    irp_RFC(g2),g3          # g3 = function from the irp
        cmpobl.f MAXRFC,g3,.pfr_500     # Jif RFC is to large
        ld      .func_tbl[g3*4],g6      # g6 = address of routine
        callx    (g6)                   # process function
        b       .pfr_1000               # exit
#
# --- RFC is to large, and return IRP with process error (invalid function).
#
.pfr_500:
        ldconst cmplt_PE,g3             # g3 = completion code "process error"
        ldconst pe_invRFC,g4            # g4 = extended status "invalid RFC"
        stob    g3,irp_cmplt(g2)        # save completion status
        st      g4,irp_extcmp(g2)       # extended completion status
        lda     -ILTBIAS(g1),g1         # back originator's IRT nest level
        ld      il_cr(g1),g6            # g6 = originator's completion routine
        callx   (g6)                    # call originator's completion handler
#
.pfr_1000:
        PopRegsVoid(r3)                 ## Save all G registers
        ret                             # return to caller
#
# --- Function table
#
        .data
.func_tbl:
        .word   apl$open                # 0 rfc_open - open session
        .word   apl$close               # 1 rfc_close - close session
        .word   apl$RSP                 # 2 rfc_RSP - request session parameters
        .word   apl$send_SCSI_cmd       # 3 rfc_SCSI_cmd - send SCSI command
        .word   apl$TMF                 # 4 rfc_TMF - task management function
        .word   apl$DSC                 # 5 rfc_discover - discover
#
#******************************************************************************
#
#  NAME: apl$open
#
#  PURPOSE:
#       Allow the Magnitude to open a session with a specific device
#
#  CALLING SEQUENCE:
#       call    apl$open
#
#  INPUT:
#       g1 = ilt at 2nd lvl
#       g2 = irp at xirp2 nest level
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g1, g7
#
#******************************************************************************
#
.stc_tbl:
        .word   apl$opn_WWID            # process World Wide ID session type
        .word   apl$opn_initID          # process initial ID session type
        .word   apl$opn_WWIDinitID      # process WWID and Init ID session type
        .word   apl$opn_devID           # process Device ID session type
        .word   apl$opn_tmt             # process TMT session type
#
        .text
#
apl$open:
        mov     0,r13
        mov     0,g7                    # g7 = 0
#
# --- set default status
#
        stob    r13,irp_cmplt(g2)       # r8 = success status
        stob    r13,irp_extcmp(g2)      # r9 = no extended status
        ldconst cmplt_PE,r8             # r8 = "process error" cmplt status
#
# --- make sure requestor ID is not zero
#
        ld      irp_Req_ID(g2),r3       # r3 = requestor ID
        ldconst pe_invreqID,r9          # r9 = "invalid requestor ID"
        cmpobe.f 0,r3,.opn_error        # JIf requestor ID is zero
#
# --- determine if I/F number is valid
#
        ldob    irp_IFID(g2),r4         # r4 = interface ID
        ldconst 0xff,r5                 # r5 = all interfaces default
        ldconst pe_IFnotsup,r9          # r9 = "I/F not supported" error
        cmpobe  r4,r5,.opn_050          # Jif all interfaces
c       r7 = ICL_IsIclPort((UINT8)r4);
        cmpobe  FALSE,r7,.opn_icl05
.if ICL_DEBUG
c       fprintf(stderr,"<apl$open>ICL port..call apl$opn_tmt\n");
.endif  # ICL_DEBUG
        b .opn_icl10                    # ICL interface is identified
#
.opn_icl05:
        cmpobl.f MAXIF,r4,.opn_error    # Jif interface ID is too large
.opn_icl10:
#
.opn_050:
        ldob     irp_STC(g2),r3         # r3 = session type code
        ldconst  pe_invSTC,r9           # r9 = invalid STC
        cmpobl.f stc_MAX,r3,.opn_error  # Jif STC to large
#
        ld      .stc_tbl[r3*4],r6       # r6 = address of routine
        callx   (r6)                    # process function
        cmpobne.f 0,g0,.opn_error_t     # JIf error occurred
#
# --- after successful completion of any open type the registers
#     should contain the following:
#               g1 = ILT at 2nd level
#               g2 = IRP
#               g4 = CIMT
#               g5 = TMT
#               g6 = TLMT
#
# --- 1st, determine if this requestor already has a session open to this
#     device. If it does, report good completion on open.
#
        ld      tlm_Shead(g6),g7        # g7 = 1st ISMT at head of session q
        cmpobe.f 0,g7,.opn_110          # Jif nothing on queue
#
        ld      irp_Req_ID(g2),r5       # r5 = requestor ID
        ldconst pe_sesact,r9            # r9 = "Session already active" error
#
.opn_100:
        ld      ism_ReqID(g7),r3        # get requestor ID
        cmpobe.f r3,r5,.opn_230         # Jif already open to this ID
        ld      ism_flink(g7),g7        # get next ismt on queue
        cmpobne.t 0,g7,.opn_100         # Jif if more sessions
#
# --- no duplicate session. is it a fabric device ???
#
.opn_110:
        ldob    tm_dsrc(g5),r3          # r3 = discovery source
        cmpobne.t tmdsrc_SNS,r3,.opn_120a # Jif not discovered from SNS
#
        ldos    tm_lid(g5),r5           # r5 = lid
        ldconst NO_LID,r4               # r4 = invalid lid
        cmpobne.t r4,r5,.opn_115        # Jif lid is defined, make sure it's logged on
                                        #   must be open.
#
# -- no lid defined, so get one
#
        call    i$get_lid               # get a lid
        ldconst pe_nolid,r9             # r9 = "no more LIDs are available" error
        cmpobe.f 0,g0,.opn_error         # JIf no more lids
#
        mov     g0,r5                   # r5 = lid
        stos    g0,tm_lid(g5)           # save new lid
#
# --- try to logon to the port
#
.opn_115:
        ldob    tm_chipID(g5),r4        # r4 = chip instance
        ld      tm_alpa(g5),r6          # r6 = alpa (PID)
        bswap   r6,r6                   # change endianess of ALPA (PID)
        movq    g0,r12                  # save g0-g3 in r12-15
#
.opn_117:
.if ICL_DEBUG
c       r7 = ICL_IsIclPort((UINT8)r4);
        cmpobe  FALSE,r7,.opn_icl15
c       fprintf(stderr,"<apl$open>ICL port..call ISP$login_fabric_port()\n");
.opn_icl15:
.endif  # ICL_DEBUG
        movt    r4,g0                   # g0 = chip instance
                                        # g1 = lid from lidt
                                        # g2 = alpa
        ldconst 0x01,g3                 # g3 = no login if already logged in
        call    ISP$login_fabric_port   # g0 = completion code
        cmpobe.t 0,g0,.opn_120          # Jif normal completion
#
        cmpobe.f islferr,g0,.opn_118a   # Jif parameter error
        cmpobe.f islfpiu,g0,.opn_118b   # Jif port in use by another LID message
        cmpobe.f islfliu,g0,.opn_118a   # Jif LID in use be another port
#
# --- Default error condition handler
#
#     All ports (81 - ff) in use
#
.opn_118:
        movq    r12,g0                  # restore g0-g3 from r12-r15
        ldconst NO_LID,r4               # r4 = invalid lid
        ldconst pe_logonerr,r9          # r9 = "could not logon to port" error
        stos    r4,tm_lid(g5)           # set LID invalid in tmt
        b       .opn_error              # process error
#
# --- Parameter error or LID used by another port
#
#     Parameter error - most likely cause is that the selected loop ID is
#                       the same as the ISP2100/ISP2200 loop ID. Leave
#                       the bit associated with the selected LID clear and
#                       find a new LID.
#
#     LID in use by   - This most likely some form of programming error but
#     another port      it should be handled in the same way as a parameter
#                       error.
#
.opn_118a:
        call    i$get_lid               # get a lid
        cmpobe.f 0,g0,.opn_118          # JIf no more lids
        mov     g0,r5                   # r5 = lid
        stos    g0,tm_lid(g5)           # save new lid
        b       .opn_117                # br
#
# --- port is currently using another LID (the target side), so release current
#     LID and claim target LID as our own.
#
.opn_118b:
        mov     r5,g0                   # g0 = old LID
        call    i$put_lid               # return lid
#
        mov     g1,g0                   # g0 = new
        mov     g1,r5                   # r5 = new  lid form login process
        call    i$use_lid               # claim this LID as our own
#
        stos    r5,tm_lid(g5)           # save new lid
        b       .opn_117                # br
#
# --- port logon was successful
#
.opn_120:
        movq    r12,g0                  # restore g0-g3 from r12-r15
#
.opn_120a:
c       g7 = get_ismt();                # Allocate a ISMT
.ifdef M4_DEBUG_ISMT
c fprintf(stderr, "%s%s:%u get_ismt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g7);
.endif # M4_DEBUG_ISMT
#
        ld      tlm_Shead(g6),r4        # get current head
        st      g7,tlm_Shead(g6)        # save new head
        st      r4,ism_flink(g7)        # save old head as flink
#
        ld      irp_Req_ID(g2),r3       # r3 = requestor ID
        st      g6,ism_tlmt(g7)         # save TLMT addr in ISMT
        st      r3,ism_ReqID(g7)        # save requestor ID in ISMT
#
        ldob    irp_STC(g2),r3          # r3 = session type code
        ldob    irp_O_rtycnt(g2),r5     # r5 = retry count
        stob    r3,ism_STC(g7)          # save session type code
        stob    r5,ism_rtycnt(g7)       # save retry count
#
        ldq     irp_Sqlfr(g2),r4        # r4-r7 = session qualifiers
        stq     r4,ism_Sqlfr(g7)        # save session qualifiers
#
# --- bump session count.
#
        ldob    tlm_sescnt(g6),r3       # r3 = session count
        addo    1,r3,r3                 # bump it
        stob    r3,tlm_sescnt(g6)       # save it
#
# --- set queuing depth
#
        ldob    tlm_pdt(g6),r5          # r5 = device type
        ldconst 0,r3                    # r3 = default to no q depth
        cmpobe.t 0,r5,.opn_200          # Jif direct access device
        ldob    tlm_dvflgs(g6),r6       # r6 = device flags
        ldob    irp_maxQ(g2),r3         # r3 = maximum queue depth
        chkbit  1,r6                    # is tag queueing supported ???
        be.t    .opn_200                # Jif yes
        ldconst 1,r3                    # r3 = max queue depth of 1
#
.opn_200:
        ldos    irp_defTO(g2),r5        # r5 = default timeout value
        ldob    irp_Sattr(g2),r4        # r4 = session attributes
        cmpobne.t 0,r5,.opn_210         # JIf non zero
        ldconst 30,r5                   # default to T/O to 30 seconds
#
.opn_210:
        stob    r3,tlm_maxQ(g6)         # save queue depth
        stob    r4,ism_Sattr(g7)        # save session attributes
        stos    r5,ism_defTO(g7)        # save default timeout value
#
        ld      irp_str(g2),r5          # r5 = session termination routine from irp
        cmpobne.t 0,r5,.opn_220         # JIf one is defined
        lda     .opn_rtn,r5             # r5 = default session termination routine
#
.opn_220:
        st      r5,ism_str(g7)          # save session termination routine
#
.opn_230:
        mov     0,r13
        stob    r13,irp_cmplt(g2)       # r8 = success status
        st      r13,irp_extcmp(g2)      # r9 = no extended status
        st      g7,irp_Pro_ID(g2)       # assign provider ID
.if     ITRACES
        call    it$trc_opnses           # *** trace open session ***
.endif  # ITRACES
        b       .opn_end                # exit
#
# --- error - save error codes in irp
#
.opn_error:
        stob    r8,irp_cmplt(g2)        # save error
        st      r9,irp_extcmp(g2)       # extended completion status lvl
#
.opn_error_t:
.if     ITRACES
        call    it$trc_fcmp_irp         # *** trace function complete ***
.endif  # ITRACES
#
# --- send irp completion
#
.opn_end:
        lda     -ILTBIAS(g1),g1         # back up to originator's IRT nest
        ld      il_cr(g1),r3            # r3 = originator's completion routine
        callx   (r3)                    # call originator's completion handler
#
.opn_rtn:
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: apl$initID
#
#  PURPOSE:
#       Process the opening of an Initiator ID type session.
#
#  CALLING SEQUENCE:
#       callx    apl$initID
#
#  INPUT:
#       g1 = IRP
#       g2 = IRT at xirp2 nest level
#
#  OUTPUT:
#    if g0 = 0
#       g1 = ILT at 2nd lvl
#       g2 = IRP
#       g4 = CIMT
#       g5 = TMT
#       g6 = TLMT
#
#    if g0 <>0
#       g1 = IRP
#       g2 = IRT at xirp2 next level
#
#  REGS DESTROYED:
#       g4-g7
#
#******************************************************************************
#
apl$opn_initID:
        ld      irp_Sqlfr_InitID(g2),r7 # r7 = initiator ID (alpa)
        ldos    irp_Sqlfr_LUN(g2),r6    # r6 = requested LUN
        ldconst cmplt_PE,r8             # r8 = "process error" cmplt status
        mov     0,g0                    # g0 = default good status
#
        ldconst 0xff,r15                # r15 = all interfaces default
        ldob    irp_IFID(g2),r14        # r14 = interface ID
        ldconst pe_IFnotsup,r9          # r9 = "I/F not supported" error
        cmpobne.t r15,r14,.opniid_100   # JIf if all interfaces
        mov     0,r14                   # r14 = 0 chip instance
        mov     0,r15                   # r15 = multi chip instance scan
#
.opniid_100:
        ld      I_CIMT_dir[r14*4],g4    # g4 = CIMT
#
        ldconst Cs_online,r5            # get online states value
        ldob    ici_state(g4),r3        # get current state
        ldconst pe_IFOOS,r9             # r8 = "I/F out of service" error
        cmpobne.f r3,r5,.opniid_500     # Jif I/F not online
#
# --- Scan the database of all ports defined the requested alpa.
#
        ld      ici_tmtQ(g4),g5         # g5 = TMT from queue
        ldconst pe_notarg,r9            # r9 = "no target found" error
#
.opniid_110:
        cmpobe.f 0,g5,.opniid_500       # Jif at end of queue
#
        ld      tm_alpa(g5),r4          # r4 = alpa
        ldos    tm_lid(g5),r5           # r5 = lid
        bswap   r4,r4                   # change endiance
        cmpobe.f r7,r4,.opniid_115      # Jif request PID = current PID
        ld      tm_link(g5),g5          # g5 = next tmt
        b       .opniid_110             # br
#
.opniid_115:
        ld      tm_tlmtdir(g5)[r6*4],g6 # g6 = tlmt address
        cmpobe.f 0,g6,.opniid_500       # Jif no tlmt for specified LUN
#
# --- The port and lun are defined. Determine the device type.
#     If it is a streaming type device, only one session may be open.
#
        ldob    tlm_pdt(g6),r4          # r4 = peripheral device type
        cmpobne.t 01,r4,.opniid_exit    # Jif not a sequential-access device
#
        ld      tlm_Shead(g6),r4        # r4 = head of session queue
        cmpobe.t 0,r4,.opniid_exit      # JIf queue is empty
#
        ldconst pe_sesact,r9            # r8 = "Session already active" error
        b       .opniid_error           # br to abnormal completion
#
.opniid_500:
        cmpobne 0,r15,.opniid_error     # Jif single chip instance scan
        addo    1,r14,r14               # bump to next interface
#
# ---  ICL ... assume that this routine is not called either for  iSCSI or ICL port..
#      hence MAXIF is not changed to handle ICL port
#
        cmpobne.t MAXIF,r14,.opniid_100 # continue with next I/F
        ldconst pe_notarg,r9            # r9 = "No Target Found" error
#
.opniid_error:
        stob    r8,irp_cmplt(g2)        # save error
        st      r9,irp_extcmp(g2)       # extended completion status lvl
#
.opniid_exit:
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: apl$opn_WWID
#
#  PURPOSE:
#       Process the opening of an session with a specific World Wide
#       name.
#
#  CALLING SEQUENCE:
#       callx    apl$opn_WWID
#
#  INPUT:
#       g1 = IRP
#       g2 = IRT
#
#  OUTPUT:
#    if g0 = 0
#       g1 = ILT at 2nd lvl
#       g2 = IRP
#       g4 = CIMT
#       g5 = TMT
#       g6 = TLMT
#
#    if g0 <>0
#       g1 = IRP
#       g2 = IRT at xirp2 next level
#
#  REGS DESTROYED:
#       g4-g7
#
#******************************************************************************
#
apl$opn_WWID:
        ldl     irp_Sqlfr_WWID(g2),r12  # r12-r13 = WWID
        ldos    irp_Sqlfr_LUN(g2),r6    # r6 = LUN
        ldconst cmplt_PE,r8             # r8 = "process error" cmplt status
        mov     0,g0                    # g0 = default good status
#
        ldconst 0xff,r15                # r15 = all interfaces default
        ldob    irp_IFID(g2),r14        # r14 = interface ID
        ldconst pe_IFnotsup,r9          # r9 = "I/F not supported" error
        cmpobne.t r15,r14,.opnwwid_100  # JIf if all interfaces
        mov     0,r14                   # r14 = 0 chip instance
        mov     0,r15                   # r15 = multi chip instance scan
#
.opnwwid_100:
        ld      I_CIMT_dir[r14*4],g4    # g4 = CIMT
#
        ldconst Cs_online,r5            # get online states value
        ldob    ici_state(g4),r3        # get current state
        ldconst pe_IFOOS,r9             # r8 = "I/F out of service" error
        cmpobne.f r3,r5,.opnwwid_500    # Jif I/F not online
#
        ld      ici_tmtQ(g4),g5         # g5 = TMT from queue
        ldconst pe_notarg,r9            # r9 = target not found
#
.opnwwid_200:
        cmpobe.f 0,g5,.opnwwid_500      # Jif at end of queue
        ldl     tm_P_name(g5),r10       # r10-r11 = port name
        cmpobne.f r12,r10,.opnwwid_250  # Jif not a match
        cmpobe.t r13,r11,.opnwwid_300   # Jif a match
#
.opnwwid_250:
        ld      tm_link(g5),g5          # g5 = next tmt
        b       .opnwwid_200            # br
#
.opnwwid_300:
        ldos    tm_lid(g5),r5           # r5 = lid
        ld      tm_tlmtdir(g5)[r6*4],g6 # g6 = TLMT address
        cmpobe.f 0,g6,.opnwwid_500      # Jif no TMLT for this LUN
        ldob    tlm_pdt(g6),r4          # r4 = peripheral device type
        cmpobne.t 01,r4,.opnwwid_exit   # Jif not a sequential-access device
        ld      tlm_Shead(g6),r4        # r4 = head of session queue
        cmpobe.t 0,r4,.opnwwid_exit     # JIf queue is empty
        ldconst pe_sesact,r9            # r8 = "Session already active" error
        b       .opnwwid_error
#
.opnwwid_500:
        cmpobne 0,r15,.opnwwid_error    # Jif single chip instance scan
        addo    1,r14,r14               # bump to next interface
#
# ---  ICL ... assume that this routine is not called either for  iSCSI or ICL port..
#      hence MAXIF is not changed to handle ICL port.
#
        cmpobne.t MAXIF,r14,.opnwwid_100 # continue with next I/F
        ldconst pe_notarg,r9            # r8 = "No Target Found" error
#
.opnwwid_error:
        stob    r8,irp_cmplt(g2)        # save error
        st      r9,irp_extcmp(g2)       # extended completion status lvl
#
.opnwwid_exit:
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: apl$WWIDinitID
#
#  PURPOSE:
#       Process the opening of an World Wide ID and Initiator ID type session.
#
#  CALLING SEQUENCE:
#       callx    apl$WWIDinitID
#
#  INPUT:
#       g1 = IRP
#       g2 = IRT at xirp2 nest level
#
#  OUTPUT:
#    if g0 = 0
#       g1 = ILT at 2nd lvl
#       g2 = IRP
#       g4 = CIMT
#       g5 = TMT
#       g6 = TLMT
#
#    if g0 <>0
#       g1 = IRP
#       g2 = IRT at xirp2 next level
#
#  REGS DESTROYED:
#       g4-g7
#
#******************************************************************************
#
apl$opn_WWIDinitID:
        ld      irp_Sqlfr_InitID(g2),r7 # r7 = initiator ID (al_pa)
        ldos    irp_Sqlfr_LUN(g2),r6    # r6 = requested LUN
        ldl     irp_Sqlfr_WWID(g2),r12  # 12-13 = WWID
        ldconst cmplt_PE,r8             # r8 = "process error" cmplt status
        mov     0,g0                    # g0 = default good status
#
        ldconst 0xff,r15                # r15 = all interfaces default
        ldob    irp_IFID(g2),r14        # r14 = interface ID
        ldconst pe_IFnotsup,r9          # r9 = "I/F not supported" error
        cmpobne.t r15,r14,.opnwiid_100  # JIf if all interfaces
        mov     0,r14                   # r14 = 0 chip instance
        mov     0,r15                   # r15 = multi chip instance scan
#
.opnwiid_100:
        ld      I_CIMT_dir[r14*4],g4    # g4 = CIMT
#
        ldconst Cs_online,r5            # get online states value
        ldob    ici_state(g4),r3        # get current state
        ldconst pe_IFOOS,r9             # r8 = "I/F out of service" error
        cmpobne.f r3,r5,.opnwiid_500    # Jif I/F not online
#
        ld      ici_tmtQ(g4),g5         # g5 = TMT from queue
        ldconst pe_notarg,r9            # r9 = "no target found" error
#
.opnwiid_110:
        cmpobe.f 0,g5,.opnwiid_500      # Jif TMT not found
#
        ldl     tm_P_name(g5),r10       # r10-r11 = port name
        ld      tm_alpa(g5),r5          # r5 = current alpa
        cmpobne.f r12,r10,.opnwiid_120  # Jif not a match
        cmpobne.f r13,r11,.opnwiid_120  # Jif not a match
        cmpobe.t  r5,r7,.opnwiid_140    # JIf alpa's match
#
.opnwiid_120:
        ld      tm_link(g5),g5          # g5 = next tmt
        b       .opnwiid_110            # br
#
# --- device found
#
.opnwiid_140:
        ld      tm_tlmtdir(g5)[r6*4],g6 # g6 = tlmt address
        cmpobe.f 0,g6,.opnwiid_500      # Jif no tlmt for specified LUN
#
        ldos    tm_lid(g5),r5           # r5 = lid
        ldob    tlm_pdt(g6),r4          # r4 = peripheral device type
        cmpobne.t 01,r4,.opnwiid_exit   # Jif not a sequential-access device
        ld      tlm_Shead(g6),r4        # r4 = head of session queue
        cmpobe.t 0,r4,.opnwiid_exit     # JIf queue is empty
#
        ldconst pe_sesact,r9            # r8 = "Session already active" error
        b       .opnwiid_error
#
.opnwiid_500:
        cmpobne 0,r15,.opnwiid_error    # Jif single chip instance scan
        addo    1,r14,r14               # bump to next interface
#
# ---  ICL ... assume that this routine is not called either for  iSCSI or ICL port..
#      hence MAXIF is not changed to handle ICL port.
#
        cmpobne.t MAXIF,r14,.opnwiid_100 # continue with next I/F
        ldconst pe_notarg,r9            # r9 = "No Target Found" error
#
.opnwiid_error:
        stob    r8,irp_cmplt(g2)        # save error
        st      r9,irp_extcmp(g2)       # extended completion status lvl
#
.opnwiid_exit:
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: apl$opn_devID
#
#  PURPOSE:
#       Process the opening of an session with a device that has a specific
#       serial number.
#
#  CALLING SEQUENCE:
#       callx   apl$opn_devID
#
#  INPUT:
#       g1 = IRP
#       g2 = IRT
#
#  OUTPUT:
#    if g0 = 0
#       g1 = ILT at 2nd lvl
#       g2 = IRP
#       g4 = CIMT
#       g5 = TMT
#       g6 = TLMT
#
#    if g0 <>0
#       g1 = IRP
#       g2 = IRT at xirp2 next level
#
#  REGS DESTROYED:
#       None
#
#******************************************************************************
#
apl$opn_devID:
        ldt     irp_Sqlfr_SN(g2),r12    # r12-r14 = device id (serial number)
        ldconst cmplt_PE,r8             # r8 = "process error" cmplt status
        mov     0,g0                    # g0 = default good status
#
        ldconst 0xff,r15                # r15 = all interfaces default
        ldob    irp_IFID(g2),r14        # r14 = interface ID
        ldconst pe_IFnotsup,r9          # r9 = "I/F not supported" error
        cmpobne.t r15,r14,.opndid_100   # JIf if all interfaces
        mov     0,r14                   # r14 = 0 chip instance
        mov     0,r15                   # r15 = multi chip instance scan
#
.opndid_100:
        ld      I_CIMT_dir[r14*4],g4    # g4 = CIMT
#
        ldconst Cs_online,r5            # get online states value
        ldob    ici_state(g4),r3         # get current state
        ldconst pe_IFOOS,r9             # r8 = "I/F out of service" error
        cmpobne.f r3,r5,.opndid_500     # Jif I/F not online
#
        ld      ici_actqhd(g4),g6        # g6 = TLMT from queue
        ldconst pe_notarg,r9            # r9 = target not found
#
.opndid_200:
        cmpobe.f 0,g6,.opndid_500       # Jif at end of queue
#
        ldt     tlm_sn(g6),r4           # r4-r6 = port name
        cmpobne.t r4,r10,.opndid_250    # Jif not a match
        cmpobne.t r5,r11,.opndid_250    # Jif not a match
        cmpobe.f  r6,r12,.opndid_300    # Jif a match
#
.opndid_250:
        ld      tlm_flink(g6),g6        # g6 = next tlmt
        b       .opndid_200             # br
#
.opndid_300:
        ld      tlm_tmt(g6),g5          # g5 = TMT address
        ldob    tlm_pdt(g6),r4          # r4 = peripheral device type
        ldos    tm_lid(g5),r5           # r5 = lid
        cmpobne.t 01,r4,.opndid_exit    # Jif not a sequential-access device
        ld      tlm_Shead(g6),r4        # r4 = head of session queue
        cmpobe.t 0,r4,.opndid_exit      # JIf queue is empty
#
        ldconst pe_sesact,r9            # r8 = "Session already active" error
        b       .opndid_error
#
.opndid_500:
        cmpobne 0,r15,.opndid_error     # Jif single chip instance scan
        addo    1,r14,r14               # bump to next interface
#
# ---  ICL ... assume that this routine is not called either for  iSCSI or ICL port..
#      hence MAXIF is not changed to handle ICL port.
#
        cmpobne.t MAXIF,r14,.opndid_100 # continue with next I/F
        ldconst pe_notarg,r9            # r8 = "No Target Found" error
#
.opndid_error:
        stob    r8,irp_cmplt(g2)        # save error
        st      r9,irp_extcmp(g2)       # extended completion status lvl
#
.opndid_exit:
         ret                            # return to caller
#
#******************************************************************************
#
#  NAME: apl$opn_tmt
#
#  PURPOSE:
#       Process the opening of an session with a device that has a specific
#       serial number.
#
#  CALLING SEQUENCE:
#       callx    apl$opn_tmt
#
#  INPUT:
#       g1 = ILT
#       g2 = IRP
#
#  OUTPUT:
#    if g0 = 0
#       g1 = ILT at 2nd lvl
#       g2 = IRP
#       g4 = CIMT
#       g5 = TMT
#       g6 = TLMT
#
#    if g0 <>0
#       g1 = IRP
#       g2 = IRT at xirp2 next level
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
apl$opn_tmt:
        ld      irp_Sqlfr_TMT(g2),g5    # g5 = tmt address
        ldos    irp_Sqlfr_LUN(g2),r6    # r6 = lun
        ld      tm_icimt(g5),g4         # g4 = icimt address
        mov     0,g0                    # g0 = default good status
        ldconst cmplt_PE,r8             # r8 = "process error" cmplt status
        ldconst pe_notarg,r9            # r9 = target not found
        ldob    tm_state(g5),r3         # get current state
        cmpobne.f tmstate_active,r3,.opntmt_err # JIf not active state
#
        ld      irp_Req_ID(g2),r3       # r3 = requestor ID
        ld      tm_tlmtdir(g5)[r6*4],g6 # g6 = possible tlmt
        cmpobe.f 0,g6,.opntmt_100       # Jif no tlmt
#
        ldob    tlm_state(g6),r3        # r4 = current TLMT state
        cmpobne.f tlmstate_active,r3,.opntmt_err # Jif device not active

        b       .opntmt_exit
#
.opntmt_100:
c       g6 = get_tlmt();                # Allocate a TLMT
c       memset((void *)g6, 0, sizeof(TLMT));
.ifdef M4_DEBUG_TLMT
c fprintf(stderr, "%s%s:%u get_tlmt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g6);
.endif # M4_DEBUG_TLMT
#
        ldob    tm_chipID(g5),r8        # r8 = chip ID
.if ICL_DEBUG
        cmpobne ICL_PORT,r8,.opntmt_icl01
c       fprintf(stderr,"<apl$opn_tmt>ICL port...\n");
.opntmt_icl01:
.endif  # ICL_DEBUG
                                        # g4 = icimt address
                                        # g6 = TLMT
        call    i$qtlmt                 # place TLMT on active queue
                                        # Note: g7 destroyed
        st      g5,tlm_tmt(g6)          # save TMT address in TLMT
        st      g4,tlm_cimt(g6)         # save CIMT address in TLMT
        stos    r6,tlm_lun(g6)          # save LUN
        st      g6,tm_tlmtdir(g5)[r6*4] # save TLMT address in TMT LUN dir
#
        ldconst tlmstate_active,r5      # r5 = "active" state
        stob    r5,tlm_state(g6)        # set device active
        b       .opntmt_exit
#
.opntmt_err:
        stob    r8,irp_cmplt(g2)        # save error
        st      r9,irp_extcmp(g2)       # extended completion status lvl
        mov     r8,g0                   # g0 = bad status
#
.opntmt_exit:
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: apl$close
#
#  PURPOSE:
#       Allow the Magnitude to close a session with a specific device
#
#  CALLING SEQUENCE:
#       call    apl$close
#
#  INPUT:
#       g1 = IRT at xirp2 nest level
#       g2 = IRP
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
apl$close:
        movq    g4,r12                  # save g4-g7
        ldconst 0,g4                    # g4 = 0
        ldconst cmplt_PE,r8             # r8 = "process error" cmplt status
        ld      irp_Pro_ID(g2),g7       # g7 = provider ID (ismt)
        ldconst pe_invproID,r9          # r9 = "invalid provider ID"
        cmpobe.f 0,g7,.cls_500          # JIf provider ID is zero
        ld      i_mintbl,r4             # r4 = minimum table address
        ld      i_maxtbl,r5             # r4 = maximum table address
.ifndef PERF
c if (g7 < r4) abort();
c if (g7 > r5) abort();
.endif  #PERF
        cmpobl.f g7,r4,.cls_500         # Jip table is out of range
        cmpobg.f g7,r5,.cls_500         # Jip table is out of range
#
        ld      irp_Req_ID(g2),r4       # r4 = requestor ID from IRP
? # crash - cqt# 22308 - FE ISMT - failed @ apldrv.as:1163  ld 8+g7,g6 with decddecd        ***
?       ld      ism_tlmt(g7),g6         # g6 = tlmt
        cmpobe  0,g6,.cls_500           # In case tlmt is freed memory
.ifdef M4_DEBUG_MEMORY_WITH_PATTERNS
        cmpobe  0xdecddecd,g6,.cls_500  # Jif target freed memory pattern
.endif # M4_DEBUG_MEMORY_WITH_PATTERNS
? # crash freed ISMT                                                                        ***
?       ld      ism_ReqID(g7),r3        # r3 =requestor ID from TLMT
        ld      tlm_tmt(g6),g5          # g5 = tmt
        ld      tlm_cimt(g6),g4         # g4 = cimt
#
        ldconst pe_targnotopn,r9        # r9 = "target no open"
        cmpobe.f 0,r3,.cls_500          # Jif target not open (or freed memory)
.ifdef M4_DEBUG_MEMORY_WITH_PATTERNS
        cmpobe  0xdecddecd,r3,.cls_500  # Jif target freed memory pattern
.endif # M4_DEBUG_MEMORY_WITH_PATTERNS
#
        ldconst pe_invreqID,r9          # r9 = "invalid requestor ID"
        cmpobne.f r3,r4,.cls_500        # Jif requestor is not the open that
                                        #  opened the session
#
# --- Decrement the session count and determine if this is the last session
#     for this port. If it is, determine if this is a FL-port.
#
        ldob    tlm_sescnt(g6),r11      # r11 = session count
        ldconst NO_LID,r10              # r10 = invalid lid (also a MSB mask)
        subo    1,r11,r11               # subtract one from session count
        stob    r11,tlm_sescnt(g6)      # save new count
        cmpobe.t 0,r11,.cls_100         # Jif no sessions open
#
        and     r10,r11,r4              # r4 = session count - MSB
        cmpobne.t 0,r4,.cls_200         # Jif somthing is still open
#
# --- Last session on a FL-port, clear out the cimt tmt directory and the tmt
#     lid address.
#
.cls_100:
        ldos    tlm_lun(g6),g0          # g0 = LUN
#
# --- Remove ISMT from active session chain
#
.cls_200:
        ld      irp_Pro_ID(g2),g7       # g7 = ismt
        ldconst pe_notarg,r9            # r9 = "no target (ISMT) found"
        lda     tlm_Shead(g6),r3        # r3 = address of active session queue
        ld      tlm_Shead(g6),r4        # r4 = ismt on queue
#
.cls_210:
        cmpobe.f 0,r4,.cls_500          # Jif we didn't find ISMT
        cmpobe.t r4,g7,.cls_250         # Jif same as current
        lda     ism_flink(r4),r3        # r3 = previous ismt
        ld      ism_flink(r3),r4        # r4 = ISMT address
        b       .cls_210                # continue
#
.cls_250:
        ld      ism_flink(g7),r4        # get forward link of next ISMT
        st      r4,(r3)                 # and save it in previous ISMT
        mov     0,r4
        stob    r4,irp_cmplt(g2)        # save completion status
        st      r4,irp_extcmp(g2)       # save extended status info
.ifdef M4_DEBUG_ISMT
c fprintf(stderr, "%s%s:%u put_ismt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g7);
.endif # M4_DEBUG_ISMT
c       put_ismt(g7);                   # Release ISMT
#
.if     ITRACES
        call    it$trc_clsses           # trace session close
.endif  # ITRACES
#
        cmpobne.t 0,r11,.cls_600         # Jif session not zero
#
# --- There are no session open to this device nor is it still found in
#     the discovery scan. Let's loss it...
#
        call    i$removeLUN             # remove lun and resources
        b       .cls_600                # br
#
.cls_500:
        stob    r8,irp_cmplt(g2)        # save completion status
        st      r9,irp_extcmp(g2)       # save extended status info
#
.if     ITRACES
        cmpobe.f  0,g4,.cls_600         # Jif no CIMT defined
        cmpobne.t 0,g7,.cls_550         # JIf ismt defined
        call    it$trc_fcmp_badid
        b       .cls_600                # br
#
.cls_550:
        call    it$trc_fcmp_ismt
.endif  # ITRACES
#
.cls_600:
        lda     -ILTBIAS(g1),g1         # back up to originator's IRT nest
                                        #  level
        ld      il_cr(g1),r3            # r3 = originator's completion routine
        callx   (r3)                    # call originator's completion handler
        movq     r12,g4                 # restore g4-g7
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: apl$send_SCSI_cmd
#
#  PURPOSE:
#       Process a SCSI command received from the application.
#
#  CALLING SEQUENCE:
#       call    apl$snd_SCSI_cmd
#
#  INPUT:
#       g1 = primary ILT at xirp2 nest level
#       g2 = IRP
#       g4 = assoc. CIMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g6
#
#******************************************************************************
#
#
apl$send_SCSI_cmd:
.if     VALIDATE
        ld      irp_Pro_ID(g2),g7       # g7 = ismt
        ldconst cmplt_PE,r8             # r8 = "process error" cmplt status
#
# --- validate the requestor and provider id's.
#
        ldconst pe_invproID,r9          # r9 = "invalid Provider ID" error
#
        cmpobe.f 0,g7,.ssc_900          # Jip provider ID is zero
        ld      i_mintbl,r4             # r4 = minimum table address
        ld      i_maxtbl,r5             # r4 = maximum table address
.ifndef PERF
c if (g7 < r4) abort();
c if (g7 > r5) abort();
.endif  #PERF
        cmpobl.f g7,r4,.ssc_900         # Jip table is out of range
        cmpobg.f g7,r5,.ssc_900         # Jip table is out of range
#
        ld      ism_ReqID(g7),r4        # r4 = requestor ID from TLMT
        ld      irp_Req_ID(g2),r5       # r5 = requestor ID from IRT
        ldconst pe_targnotopn,r9        # r9 = "target not open" error
        cmpobe.f 0,r4,.ssc_900          # Jif session not open
#
        ldconst pe_invreqID,r9          # r9 = "invalid requestor ID" error
        cmpobe.f 0,r5,.ssc_900          # Jif reqID is zero
#
        cmpobne.f r4,r5,.ssc_900        # Jif requestor IDs don't match
#
# --- validate task attributes (just check for the naughty bits)
#
        ldconst TaskAttrMask,r4         # r4 = task attribute mask
        ld      irp_Tattr(g2),r3        # r3 = task attributes
        ldconst pe_invtaskattr,r9       # r9 = inval
        and     r4,r3,r3                # and
        cmpobe  0,r3,.ssc_900           # Jif invalid bits set
#
# --- validate the sense area and sense area size
#
        ld      irp_SNSptr(g2),r3       # r3 = sense area pointer
        cmpobne.f 0,r3,.ssc_050         # Jif sense area defined
#
        ldob    irp_SNSsize(g2),r3      # r12 = sense area size
        ldconst pe_noSNSarea,r9         # r9 = "no sense area defined" error
        cmpobne.f 0,r3,.ssc_900         # Jif no sense area size
        b       .ssc_080                # br
#
.ssc_050:
        ldob    irp_SNSsize(g2),r3      # r12 = sense area size
        ldconst pe_noSNSsize,r9         # r9 = "no sense area size" error
        cmpobe.f 0,r3,.ssc_900          # Jif no sense area size
#
# --- validate SGL pointer
#
.ssc_080:
        ld      irp_SGLptr(g2),r6       # r6 = SGL pointer
        ldconst pe_noSGL,r9             # r9 = "no SGL pointer" error
        cmpobe.f 0,r6,.ssc_900          # Jif no SGL pointer
#
# --- validate direction and task type
#
        ldob    irp_Tattr_DTA(g2),r4    # r4 = data transfer attributes
        ldconst pe_invDTA,r9            # r9 = "invalid transfer attributes"
        cmpobe.f 0x03,r4,.ssc_900       # Jif not valid
        ldob    irp_Tattr_TTC(g2),r3    # r3 = task type code
        ldconst pe_invTTC,r9            # r9 = "invalid task type codes"
        cmpobe.f 0x07,r3,.ssc_900       # JIf if invalid
        cmpobe.f 0x06,r3,.ssc_900       # JIf if invalid
        cmpobe.f 0x05,r3,.ssc_900       # JIf if invalid
        cmpobe.f 0x03,r3,.ssc_900       # JIf if invalid
.else                           # not VALIDATE
        ld      irp_Pro_ID(g2),g7       # g7 = ismt
.endif  # VALIDATE
#
        ld      ism_tlmt(g7),g6         # g6 = tlmt
        ld      tlm_cimt(g6),g4         # g4 = cimt
        ld      tlm_tmt(g6),g5          # g5 = tmt
#
        ldob    irp_Tattr_TTC(g2),r4    # r4 = task type code
        ldos    irp_tagID(g2),r5        # r5 = tag id
        mov     0,r13
        st      r13,oil2_xli(g1)        # clear pointer to XLI
        stl     g6,oil2_tlmt(g1)        # g6 = tlmt
                                        # g7 = ismt
        stob    r4,oil2_TTC(g1)         # save task type code
        stos    r5,oil2_tagID(g1)       # save tag id
#
# --- trace command
#
.if     ITRACES
        call    it$trc_sndcmd           # *** trace send command ***
.endif  # ITRACES
#
# --- place task on queue
#
        call    apl$qtask2wq            # place task on work queue
                                        #   g0 = queue status
#
# --- set task retry count and timeout value
#
        ldob    irp_rtycnt(g2),r4       # r4 = command retry counter
        cmpobne.t 0,r4,.ssc_100         # Jif non zero
        ldob    ism_rtycnt(g7),r4       # r4 = default retry value
#
.ssc_100:
        stob    r4,oil2_rtycnt(g1)      # save retry counter
        ldos    irp_cmdTO(g2),r3        # r3 = command T/O value from irp
        cmpobne.t 0,r3,.ssc_110         # Jif timer value defined
        ldos    ism_defTO(g7),r3        # r3 = default T/O value
#
.ssc_110:
        addo    1,r3,r3                 # add one second
        shlo    2,r3,r3                 # r3 = r3 * 4 (sec to 250ms increments)
        stos    r3,oil2_tmr1(g1)        # save gross task t/o value
#
# --- determine if tag sequence is correct
#
        cmpobe.t TRUE,g0,.ssc_800       # Jif task placed on empty queue
        ld      il_bthd(g1),g10         # g10 = ILT address of previous
        ldob    oil2_TTC(g1),r7         # r7 = task type code of new task
        ldob    oil2_tstate(g10),r13    # r13 = task state code of previous
        ldob    oil2_TTC(g10),r12       # r12 = task type of previous task
                                        #  table
        ldconst pe_invtagseq,r9         # r9 = invalid tag sequence
        cmpobe.t ttc_untaged,r7,.ssc_850 # Jif new task untagged - Error
                                        # If tasks were on working queue
                                        #  when new task was received and the
                                        #  new task is an untagged task, an
                                        #  error has occurred.
        cmpobe.t ttc_untaged,r12,.ssc_850 # Jif previous task untagged
                                        # If previous task was untagged and
                                        #  a new task is received, an error
                                        #  has occurred.
#
# --- Not mixed Untagged/Tagged tasks
#
        cmpobe.t TRUE,g0,.ssc_800       # Jif task placed on empty queue
#
        cmpobe.f ttc_ordered,r7,.ssc_1000 # Jif ordered type task since we
                                        #  must wait for all older tasks to
                                        #  complete before processing an
                                        #  ordered task
        cmpobne.f ttc_simple,r7,.ssc_800 # Jif not a simple task type
                                        # Must be either an ACA or head-of-
                                        #  queue task.
        cmpobne.f ttc_simple,r12,.ssc_1000 # Jif previous task not simple
                                        #  task type
        cmpobne.f oil2_ts_enbl,r13,.ssc_1000 # Jif previous task is not
                                        #  enabled
#
# --- The current task is simple and the previous is simple and is enabled,
#     determine if the current task can be enabled.
#
         ldos    tlm_enblcnt(g6),r3     # r3 = number of enabled tasks
         ldob    tlm_maxQ(g6),r4        # r4 = max queue depth
         cmpobe.f 0,r4,.ssc_800         # Jif no queue depth
         cmpobe  r3,r4,.ssc_1000        # JIf queue already full
#
.ssc_800:
        call    apl$enable_task         # enable the task
        b       .ssc_1000               # exit
#
# --- save error completion code, extended completion code and call
#     the level 1 completion handler. This tag should not called after
#     the secondary ILT (s-ILT) is allocated unless there is logic to
#     deallocate it.
#
.ssc_850:
        call    apl$remtask             # remove from queue
#
.ssc_900:
        stob    r8,irp_cmplt(g2)        # save completion status
        st      r9,irp_extcmp(g2)       # save extended status info
#
        lda     -ILTBIAS(g1),g1         # back up to originator's IRT nest
                                        #  level
        ld      il_cr(g1),r3            # r3 = originator's completion routine
        callx   (r3)                    # call originator's completion handler
#
.ssc_1000:
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: apl$RSP
#
#  PURPOSE:
#       Allow the Magnitude to request session parameters
#
#  DESCRIPTION:
#       Pos    Name         Length   Description
#       -----|-------------|-------|-------------------------------------
#       1      Chip ID        1b     Chip ID
#       2      ALPA           3b     FC-AL ID
#       3      LUN            1b     LUN
#       4      Dev_type       1b     Device type
#       5      Flags          2b     Misc flags
#       6      WWID           8b     World Wide ID
#       7      Ven ID         8b     Vendor ID
#       8      Pod ID        16b     Product ID
#       9      Pod lvl        4b     Product revision level
#      10      Dev SN        12b     Device serial number
#
#  CALLING SEQUENCE:
#       call    apl$RSP
#
#  INPUT:
#       g1 = IRT at xirp2 nest level
#       g2 = IRP
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g1 is destroyed.
#
#******************************************************************************
#
apl$RSP:
        ldconst cmplt_PE,r8             # r8 = "process error" cmplt status
        ld      irp_Pro_ID(g2),g7       # g7 = provider ID
        ldconst pe_invproID,r9          # r9 = "invalid provider ID"
        cmpobe.f 0,g7,.rsp_1000         # JIf provider ID is zero
        ld      i_mintbl,r4             # r4 = minimum table address
        ld      i_maxtbl,r5             # r4 = maximum table address
.ifndef PERF
c if (g7 < r4) abort();
c if (g7 > r5) abort();
.endif  #PERF
        cmpobl.f g7,r4,.rsp_1000        # Jip table is out of range
        cmpobg.f g7,r5,.rsp_1000        # Jip table is out of range
#
        ld      ism_ReqID(g7),r3        # r3 =requestor ID from TLMT
        ld      irp_Req_ID(g2),r4       # r4 = requestor ID from IRP
        ldconst pe_targnotopn,r9        # r9 = "target no open"
        cmpobe.f 0,r3,.rsp_1000         # Jif target not open
        ldconst pe_invreqID,r9          # r9 = "invalid requestor ID"
        cmpobne.f r3,r4,.rsp_1000       # Jif requestor is not the open that
#
# --- move in device serial number
#
        ld      ism_tlmt(g7),g6         # g6 = TLMT address
        ldob    tlm_snlen(g6),r3        # r3 = serial number length
        ldq     tlm_sn(g6),r12          # r12-r15 = serial number
        stq     r12,irp_SN(g3)          # save serial number
        stob    r3,irp_SNlen(g2)        # save serial number length
#
# --- move in WWID and initiator ID
#
        ld      tlm_tmt(g6),g5          # g5 = TMT address
        ld      tm_alpa(g5),r3          # r3 = alpa
        ldl     tm_P_name(g5),r10       # r10-r11 = port name
        st      r3,irp_initID(g2)       # save alpa
        stl     r10,irp_WWID(g2)        # save port name
#
.rsp_1000:
        stob    r8,irp_cmplt(g2)        # save completion status
        st      r9,irp_extcmp(g2)       # save extended status info
        lda     -ILTBIAS(g1),g1         # back up to originator's IRT nest
                                        #  level
        ld      il_cr(g1),r3            # r3 = originator's completion routine
        callx   (r3)                    # call originator's completion handler
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: apl$DSC
#
#  PURPOSE:
#       Allow the Magnitude to rediscover a target device.
#
#  CALLING SEQUENCE:
#       call    apl$DSC
#
#  INPUT:
#       g1 = ILT at xirp2 nest level
#       g2 = IRP
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g1 is destroyed.
#
#******************************************************************************
#
apl$DSC:
        ldconst cmplt_PE,r8             # r8 = "process error" cmplt status
        ld      irp_Pro_ID(g2),g5       # g5 = provider ID (TMT)
        ldconst pe_invproID,r9          # r9 = "invalid provider ID"
        cmpobe.f 0,g5,.dsc_err          # JIf provider ID is zero
#
        ld      i_mintbl,r4             # r4 = minimum table address
        ld      i_maxtbl,r5             # r4 = maximum table address
.ifndef PERF
c if (g5 < r4) abort();
c if (g5 > r5) abort();
.endif  #PERF
        cmpobl.f g5,r4,.dsc_err         # Jip table is out of range
        cmpobg.f g5,r5,.dsc_err         # Jip table is out of range
#
        ld      tm_icimt(g5),g4         # g4 = icimt
#
        ldconst pe_targnotopn,r9        # r9 = "invalid provider ID"
        ldos    tm_lid(g5),r7           # get device LID (loop ID)
        ldob    ici_mylid(g4),r6        # r6 = LID of my interface
        cmpobne.f r6,r7,.dsc_100        # JIf not a request for my LID
#
#
#
.dsc_err:
        stob    r8,irp_cmplt(g2)        # save completion status
        st      r9,irp_extcmp(g2)       # save extended status info
        lda     -ILTBIAS(g1),g1         # back up to originator's IRT nest
                                        #  level
        ld      il_cr(g1),r3            # r3 = originator's completion routine
        callx   (r3)                    # call originator's completion handler
        b       .dsc_1000               # exit
#
# --- determine if this device has a valid LID. If it does, assign a
#     LID and logon to the port.
#
.dsc_100:
#
# --- determine if there is a LID already defined
#
        ldob    tm_dsrc(g5),r3          # r3 = discovery source
        cmpobne.t tmdsrc_SNS,r3,.dsc_200a # Jif not discovered from SNS
#
        ldos    tm_lid(r5),r5           # r5 = current lid
        ldconst NO_LID,r3               # r3 = invalid lid
        cmpobne.f r3,r5,.dsc_115        # Jif lid defined
#
# --- The port is defined but does not have a LID and is not logged on.
#     So assign it a LID and attempt to log on the port.
#
#
# -- no lid defined, so get one
#
        call    i$get_lid               # get a lid
        ldconst pe_nolid,r9             # r9 = "no more LIDs are available" error
        cmpobe.f 0,g0,.dsc_err          # JIf no more lids
#
        mov     g0,r5                   # r5 = lid
        stos    g0,tm_lid(g5)           # save new lid
#
# --- try to logon to the port
#
.dsc_115:
        ldob    tm_chipID(g5),r4        # r4 = chip instance
        ld      tm_alpa(g5),r6          # r6 = alpa (PID)
        bswap   r6,r6                   # change endianess of ALPA (PID)
        movq    g0,r12                  # save g0-g3 in r12-r15
#
.dsc_117:
        movt    r4,g0                   # g0 = chip instance
                                        # g1 = lid from lidt
                                        # g2 = alpa
        ldconst 0x01,g3                 # g3 = no login if already logged in
        call    ISP$login_fabric_port   # g0 = completion code
        cmpobe.t 0,g0,.dsc_200          # Jif normal completion
#
        cmpobe.f islferr,g0,.dsc_118a   # Jif parameter error
        cmpobe.f islfpiu,g0,.dsc_118b   # Jif port in use by another LID message
        cmpobe.f islfliu,g0,.dsc_118a   # Jif LID in use be another port
#
# --- Default error condition handler
#
#     All ports (81 - ff) in use
#
.dsc_118:
        movq    r12,g0                  # restore g0-g3 from r12-r15
        ldconst NO_LID,r4               # r4 = invalid lid
        stos    r4,tm_lid(g5)           # set LID invalid in tmt
        ldconst pe_logonerr,r9          # r9 = "could not logon port"
        b       .dsc_err                # exit
#
# --- Parameter error or LID used by another port
#
#     Parameter error - most likely cause is that the selected loop ID is
#                       the same as the ISP2100/ISP2200 loop ID. Leave
#                       the bit associated with the selected LID clear and
#                       find a new LID.
#
#     LID in use by   - This most likely some of programming error but it
#     another port      should be handled in the same way as a parameter
#                       error.
#
.dsc_118a:
        call    i$get_lid               # get a lid
        cmpobe.f 0,g0,.dsc_118          # JIf no more lids
        mov     g0,r5                   # r5 = lid
        stos    g0,tm_lid(g5)           # save new lid
        b       .dsc_117                # br
#
# --- port is currently using another LID (the target side), so claim
#     target LID as our own.
#
.dsc_118b:
        mov     r5,g0                   # g0 = old LID
        call    i$put_lid               # return lid
#
        mov     g1,g0                   # g0 = new
        mov     g1,r5                   # r5 = new
        call    i$use_lid               # claim this LID as our own
#
        stos    r5,tm_lid(g5)           # save new lid
        b       .dsc_117                # br
#
# --- there is a LID and it's logged on, so set up the discovery parameters
#     and start the discovery scan.
#
.dsc_200:
        movq    r12,g0                  # restore g0-g3 from r12-r15
#
.dsc_200a:
        mov     g1,r15                  # r15 = ILT'
    ldconst 512,g0              # g0 = size for SGL
        call    i$alloc_dis_resource    # g1 = ILT for operation
        st      r15,oil1_ILT(g1)        # save primary ILT
#
        ldconst oimode_parent,r4        # r4 = parent task
        stob    r4,oil1_tmode(g1)       # save task as parent
#
        mov     g1,r15                  # r15 = ILT at 1st lvl
        lda     ILTBIAS(g1),g1          # g1 = ILT at 2nd lvl
#
        ldconst .Ts_start,r5            # r5 = start event table
        st      r5,oil2_ehand(g1)       # save it
#
# --- setup some default parameters for scan start
#
        ldob    ici_chpid(g4),r5        # r5 = chip instance
        ldos    tm_lid(g5),r12          # r12 = lid
        mov     0,r13
        lda     dsc_cr,r3               # r3 = address of completion routine
#
        stob    r13,oil1_lun(r15)       # set LUN 0
        stos    r12,oil1_lid(r15)       # save LID of target device
        stob    r5,oil1_chpid(r15)      # save chip instance
        st      r3,il_cr(r15)           # save completion return address
        st      g5,oil1_tmt(r15)        # save tmt address
#
        ldob    tm_flag(g5),r3          # r3 = flag byte
        setbit  tmflg_lundisc,r3,r3     # set LUN discovery in process flag
        stob    r3,tm_flag(g5)          # save
#
        setbit  oiflg_LID,0,r4          # set LID only in flags
        stob    r4,oil1_flag(r15)       # save flag byte
#
        lda     I$recv_cmplt_rsp,g2     # g2 = completion routine
        ldconst .Ts_turl0,g3            # g3 = "lun 0" event table
        call    i$send_tur              # send TUR command
#
.dsc_1000:
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: dsc_cr
#
#  PURPOSE:
#       Allow processing of a LID discovery completion.
#
#  CALLING SEQUENCE:
#       call    dsc_cr
#
#  INPUT:
#       g1 = ILT at 1st lvl (xirp1)
#       g2 = IRP
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g1 is destroyed.
#
#******************************************************************************
#
dsc_cr:
        PushRegs(r3)                    # Save all G registers
        ld      oil1_ILT(g1),g8         # g8 = ILT'
        ld      oil1_tmt(g1),g5         # g5 = tmt address
        lda     ILTBIAS(g1),g1          # g1 = ILT" at 2nd lvl
#
        call    i$rel_dis_resource      # release ILT and SGL
#
        mov     g8,g1                   # g1 = ILT'
        ld      il_misc(g1),g2          # g2 = IRP
        mov     0,g13
        stob    g13,irp_cmplt(g2)       # save completion status
        st      g13,irp_extcmp(g2)      # save extended status info
        lda     -ILTBIAS(g1),g1         # back to originator's IRT nest level
        ld      il_cr(g1),g8            # g8 = originator's completion routine
        callx   (g8)                    # call originator's completion handler
#
        PopRegsVoid(r3)                 # Restore all G registers.
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: apl$TMF
#
#  PURPOSE:
#       Allow the Magnitude to preform task management functions.
#
#  CALLING SEQUENCE:
#       call    apl$TMF
#
#  INPUT:
#       g1 = IRT at xirp2 nest level
#       g2 = IRP
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g1 is destroyed.
#
#******************************************************************************
#
apl$TMF:
        ldconst  cmplt_PE,r8            # r8 = default error code
#
.if     VALIDATE
        ld      irp_Pro_ID(g2),g7       # r7 = ismt
#
# --- validate the requestor and provider id's.
#
        ldconst pe_invproID,r9          # r9 = "invalid Provider ID" error
        cmpobe.f 0,g7,.ssc_900          # Jip provider ID is zero
        ld      i_mintbl,r4             # r4 = minimum table address
        ld      i_maxtbl,r5             # r4 = maximum table address
.ifndef PERF
c if (g7 < r4) abort();
c if (g7 > r5) abort();
.endif  #PERF
        cmpobl.f g7,r4,.tmf_900         # Jip table is out of range
        cmpobg.f g7,r5,.tmf_900         # Jip table is out of range
#
        ld      ism_ReqID(g7),r4        # r4 = requestor ID from TLMT
        ld      irp_Req_ID(g2),r5       # r5 = requestor ID from IRT
        ldconst pe_targnotopn,r9        # r9 = "target not open" error
        cmpobe.f 0,r4,.tmf_900          # Jif session not open
#
        ldconst pe_invreqID,r9          # r9 = "invalid requestor ID" error
        cmpobe.f 0,r5,.tmf_900          # Jif reqID is zero
        cmpobne.f r4,r5,.tmf_900        # Jif requestor IDs don't match
.else   # VALIDATE
        ld      irp_Pro_ID(g2),g7       # g7 = ismt
.endif  # VALIDATE
#
        ldob    irp_TMFC(g2),r3         # r6 = task management function code
        ldconst pe_invTMFC,r9           # r9 = invalid task man func code
        ldconst MAXTMFC,r4              # r7 = maximum TMFC
        cmpobe.f 0,r3,.tmf_900          # Jif tmfc is zero
        cmpobg.f r3,r4,.tmf_900         # Jif tmfc is to large
#
        ld      .tmfc_tbl[r3*4],r6      # r6 = address of routine
        callx   (r6)                    # process function
        b       .tmf_1000               # exit
#
# --- save error completion code, extended completion code and call
#     the level 1 completion handler. This tag should not called after
#     the secondary ILT (s-ILT) is allocated.
#
.tmf_900:
        stob    r8,irp_cmplt(g2)        # save completion status
        st      r9,irp_extcmp(g2)       # save extended status info
#
        lda     -ILTBIAS(g1),g1         # back up to originator's IRT nest
                                        #  level
        ld      il_cr(g1),r3            # r3 = originator's completion routine
        callx   (r3)                    # call originator's completion handler
#
.tmf_1000:
        ret                             # return to caller
#
        .data
.tmfc_tbl:
        .word   0                       # invalid
        .word   apl$abttsk              # abort task
        .word   apl$abttsk_set          # abort task set
        .word   apl$clrACA              # clear ACA
        .word   apl$clrtsk_set          # clear task set
        .word   apl$LUrst               # logical unit reset
        .word   apl$targrst             # target reset
#
        .text
#
#******************************************************************************
#
#  NAME: apl$abttsk
#
#  PURPOSE:
#       This routine is used to abort a task that has been presented to Qlogic
#
#  CALLING SEQUENCE:
#       call    apl$abrtsk
#
#  INPUT:
#       g2 = irp
#       g7 = ismt
#
#  OUTPUT:
#       None
#
#  REGS DESTROYED:
#      None.
#
#******************************************************************************
#
apl$abttsk:
        ld      ism_tlmt(g7),g6         # g6 = tlmt address
        ldos    irp_tagID(g2),r4        # r4 = tag ID
        ld      ism_ReqID(g7),r5        # r5 = requestor ID (session key)
        ld      tlm_whead(g6),g1        # g1 = pri. ILT of task to check
        ld      tlm_tmt(g6),g5          # g5 = tmt address
#
# --- Determine if the task is on the working queue
#
.abtsk40:
        cmpobe.f 0,g1,.abtsk1000        # Jif end of working queue
#
        ldos    oil2_tagID(g1),r3       # r3 = tag ID of task
        cmpobne.t r4,r3,.abtsk50        # Jif tags don't match
#
        ld      oil2_Req_ID(g1),r3      # r3 = requestor ID of task
        cmpobe.t r5,r3,.abtsk100        # Jif requestor ID's match
#
.abtsk50:
        ld      il_fthd(g1),g1          # inc. to next ILT on queue
        b       .abtsk40                # and check next task ILT
#
# --- Default the completion status and clear retry counter
#
.abtsk100:
        ldconst cmplt_IOE,r8            # r8 = IOE completion status
        ldconst ioe_cmdabrtd,r9         # r9 = command aborted
        stob    r8,irp_cmplt(g2)        # save completion status
        st      r9,irp_extcmp(g2)       # save extended status info
        ldconst 0,r13
        stob    r13,oil2_rtycnt(g1)     # clear retry counter
#
# --- process the abort event
#
        ldob    oil2_tstate(g1),r3      # r3 = task state
        ld      oil2_ehand(g1),r6       # r6 = event handler table addr
        call    apl$remtask             # remove task from queue
        lda     Te_TskAbort(r6),r6      # use abort task offset
        callx   (r6)                    # process aborted task
#
# --- Determine if the task was dormant. If it was, it has be returned
#     to the originator and no other processing is required. If it
#     was not dormant, it is now on the abort queue and an abort task
#     (abort_iocb) must be sent to the Qlogic.
#
        cmpobe.f oil2_ts_dorm,r3,.abtsk1000 # Jif task is dormant
#
        movq    g0,r4                   # save g0-g3
        lda     ILTBIAS(g1),g3          # g3 = ILT at 3rd lvl
        ldob    tm_chipID(g5),g0        # g0 = chip instance
        ldos    tm_lid(g5),g1           # g1 = LID
        ldos    tlm_lun(g6),g2          # g2 = LUN
        call    ISP$abort_iocb          # ABORT the TASK
        movq    r4,g0                   # restore g0-g3
#
.abtsk1000:
        ret
#
#******************************************************************************
#
#  NAME: apl$abttsk_set
#
#  PURPOSE:
#       This routine is used to abort a task set that has been presented
#       to the Qlogic
#
#  CALLING SEQUENCE:
#       call    apl$abrtsk_set
#
#  INPUT:
#       g1 = ILT at Initiator level 2.
#       g2 = irp
#       g7 = ismt
#
#  OUTPUT:
#       None
#
#  REGS DESTROYED:
#      None.
#
#******************************************************************************
#
apl$abttsk_set:
        ld      ism_tlmt(g7),g6         # g6 = tlmt address
        ldos    irp_tagID(g2),r4        # r4 = tag ID
        ld      ism_ReqID(g7),r5        # r5 = requestor ID (session key)
        ld      tlm_whead(g6),g1        # g1 = pri. ILT of task to check
        ld      tlm_tmt(g6),g5          # g5 = tmt address
        mov     0,r4                    # r4 = 0
#
# --- find all the task associated with this originator.
#
.abtsk_set50:
        cmpobe.f 0,g1,.abtsk_set900     # Jif end of working queue
#
        ldos    oil2_tagID(g1),r3       # r3 = tag ID of task
        cmpobne.t r4,r3,.abtsk_set150   # Jif tags don't match
#
        ld      oil2_Req_ID(g1),r3      # r3 = requestor ID of task
        cmpobne.t r5,r3,.abtsk_set150   # Jif requestor ID's don't match
#
# --- Default the completion status and clear retry counter
#
        ldconst cmplt_IOE,r8            # r8 = IOE completion status
        ldconst ioe_cmdabrtd,r9         # r9 = command aborted
        stob    r8,irp_cmplt(g2)        # save completion status
        st      r9,irp_extcmp(g2)       # save extended status info
        ldconst 0,r13
        stob    r13,oil2_rtycnt(g1)     # clear retry counter
#
# --- process the abort event
#
        ldob    oil2_tstate(g1),r3      # r3 = task state
        call    apl$remtask             # remove task from queue
        ld      oil2_ehand(g1),r6       # r6 = event handler table addr
        lda     Te_TskAbort(r6),r6      # use abort task offset
        callx   (r6)                    # process abort task
        or      r3,r4,r4                # r4 = all task states
#
# --- Determine if there are any more tasks
#
.abtsk_set150:
        ld      il_fthd(g1),g1          # inc. to next ILT on queue
        b       .abtsk_set50            # and check next task ILT
#
# --- If task was dormant, it has been returned to it's originator. If
#     any task was active, it has been placed on the abort queue and a
#     abort taskset (abort_task_set) request must be sent to the Qlogic.
#
.abtsk_set900:
        cmpobe.f 0,r4,.abtsk_set1000     # Jif all tasks dormant
#
        movq    g0,r4                   # save g0-g3
        ldob    tm_chipID(g5),g0        # g0 = chip instance
        ldos    tm_lid(g5),g1           # g1 = LID
        ldos    tlm_lun(g6),g2          # g2 = LUN
c       ISP_AbortTaskSet(g0,g1,g2)
        movq    r4,g0                   # restore g0-g3
#
.abtsk_set1000:
       ret
#
#******************************************************************************
#
#  NAME: apl$clrACA
#
#  PURPOSE:
#       This routine is used to clear an auto contingent allegiance condition
#       in the Qlogic.
#
#  CALLING SEQUENCE:
#       call    apl$clrACA
#
#  INPUT:
#       g1 = ILT at Initiator level 2.
#       g2 = irp
#       g7 = ismt
#
#  OUTPUT:
#       None
#
#  REGS DESTROYED:
#      None.
#
#******************************************************************************
#
apl$clrACA:
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: apl$clrtsk_set
#
#  PURPOSE:
#       This routine is used to abort all task in the specified task set.
#
#  CALLING SEQUENCE:
#       call    apl$clrtsk_set
#
#  INPUT:
#       g1 = ILT at Initiator level 2.
#       g2 = irp
#       g7 = ismt
#
#  OUTPUT:
#       None
#
#  REGS DESTROYED:
#      None.
#
#******************************************************************************
#
apl$clrtsk_set:
        ld      ism_tlmt(g7),g6         # g6 = tlmt address
        ldos    irp_tagID(g2),r4        # r4 = tag ID
        ld      ism_ReqID(g7),r5        # r5 = requestor ID (session key)
        ld      tlm_whead(g6),g1        # g1 = pri. ILT of task to check
        ld      tlm_tmt(g6),g5          # g5 = tmt address
        mov     0,r4                    # r4 = 0
#
.clrtsk_set50:
        cmpobe.f 0,g1,.clrtsk_set900    # Jif end of working queue
#
        ldos    oil2_tagID(g1),r3       # r3 = tag ID of task
        cmpobne.t r4,r3,.clrtsk_set150  # Jif tags don't match
#
# --- Default the completion status and clear retry counter
#
        ldconst cmplt_IOE,r8            # r8 = IOE completion status
        ldconst ioe_cmdabrtd,r9         # r9 = command aborted
        stob    r8,irp_cmplt(g2)        # save completion status
        st      r9,irp_extcmp(g2)       # save extended status info
        ldconst 0,r13
        stob    r13,oil2_rtycnt(g1)     # clear retry counter
#
# --- process the abort event
#
        ldob    oil2_tstate(g1),r3      # r3 = task state
        call    apl$remtask             # remove task from queue
        ld      oil2_ehand(g1),r6       # r6 = event handler table addr
        lda     Te_TskAbort(r6),r6      # use abort task offset
        callx   (r6)                    # process abort task
        or      r3,r4,r4                # r4 = all task states
#
.clrtsk_set150:
        ld      il_fthd(g1),g1          # inc. to next ILT on queue
        b       .clrtsk_set50           # and check next task ILT
#
.clrtsk_set900:
        cmpobe.f 0,r4,.clrtsk_set1000   # Jif all tasks dormant
#
        movq    g0,r4                   # save g0-g3
        ldob    tm_chipID(g5),g0        # g0 = chip instance
        ldos    tm_lid(g5),g1           # g1 = LID
        ldos    tlm_lun(g6),g2          # g2 = LUN
c       ISP_ClearTaskSet(g0,g1,g2);
        movq    r4,g0                   # restore g0-g3
#
.clrtsk_set1000:
       ret
#
#******************************************************************************
#
#  NAME: apl$LUrst
#
#  PURPOSE:
#       This routine performs a logical unit reset.
#
#  CALLING SEQUENCE:
#       call    apl$LUrst
#
#  INPUT:
#       g1 = ILT at Initiator level 2.
#       g2 = irp
#       g7 = ismt
#
#  OUTPUT:
#       None
#
#  REGS DESTROYED:
#      None.
#
#******************************************************************************
#
apl$LUrst:
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: apl$targrst
#
#  PURPOSE:
#       This routine is used to perform a target reset.
#
#  CALLING SEQUENCE:
#       call    apl$targrst
#
#  INPUT:
#       g1 = ILT at Initiator level 2.
#       g2 = irp
#       g7 = ismt
#
#  OUTPUT:
#       None
#
#  REGS DESTROYED:
#      None.
#
#******************************************************************************
#
apl$targrst:
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: apl$recv_cmplt_rsp
#
#  DESCRIPTION:
#       A small amount of convention for commonly used tables:
#               g4 = CIMT
#               g5 = TMT
#               g6 = TLMT
#               g7 = CDB
#
#  CALLING SEQUENCE:
#       call    apl$recv_cmplt_rsp
#
#  INPUT:
#       g0 = status
#       g1 = ILT at Initiator level 2.
#       g11 = status type 0 IOCB (if g0 <> 0)
#
#  OUTPUT:
#       None
#
#  REGS DESTROYED:
#      None.
#
#******************************************************************************
#
apl$recv_cmplt_rsp:
        PushRegs(r3)                    # Save all G registers
        ld      oil2_xli(g1),g7         # g7 = pointer to xli
        ldob    xlichipi(g7),g10        # g10 = chip instance
        ld      I_CIMT_dir[g10*4],g4    # g4 = CIMT dir pointer
        ldconst Te_success,g9           # g9 = default success
        cmpobe.t 0,g0,.arcr_100         # Jif cmd complete = 0
#
# --- Check if the port type is iSCSI. If yes, skip the IOCB status check
#
c       g8 = ICL_IsIclPort((UINT8)g10);
        cmpobe  TRUE,g8,.arcr_icl01
        ld      iscsimap,g8            # g8 = iscsi port map
        bbc     g10,g8,.arcr_050       # Jif not iSCSI port
.arcr_icl01:
        mov     g2,g9                  # g9 = status
        b       .arcr_100
#
.arcr_050:
        ldob    0x16(g11),g0            # g0 = SCSI status
        ldob    scsi_st_norm[g0*1],g9   # g9 = normalized SCSI status
        cmpobne.f Te_success,g9,.arcr_100 # Jif if SCSI status not OK
#
        ldob    0x08(g11),g0          # g0 = completion status
        ldob    cmplt_st_norm[g0*1],g9  # g9 = normalized cmplt status
#
.arcr_100:
        ld      oil2_ehand(g1),g10      # g10 = event handler routine
        ld      oil2_irp(g1),g2         # g2 = irp pointer
        ldl     oil2_tlmt(g1),g6        # g6 = tlmt pointer
                                        # g7 = ismt pointer
#
        addo    g10,g9,g10              # add completion event type
        ld      (g10),g10
        callx   (g10)                   # process event
                                        #   g0 = SCSI or Qlogic completion status
                                        #   g1 = ILT at initiator level 1
                                        #   g4 = CIMT
                                        #   g6 = tlmt
                                        #   g7 = ismt
                                        #   g11 = IOCB
#
        PopRegsVoid(r3)                 # Restore all G registers
        ret
#
#******************************************************************************
#
#  NAME: apl$tsk_cmplt_success
#
#  CALLING SEQUENCE:
#       call    apl$tsk_cmplt_success
#
#  INPUT:
#       g0 = SCSI or Qlogic completion status
#       g1 = ILT at Initiator level 2.
#       g2 = IRP
#       g4 = CIMT
#       g6 = TLMT
#       g7 = ISMT
#       g11 = status type 0 IOCB (if g0 <> 0)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g1 and g2 are destroyed.
#
#******************************************************************************
#
apl$tsk_cmplt_success:
        ldconst 0,r13                   # r13 = Clearing Register
        st      r13,irp_extcmp(g2)      # clear extended completion status
        ld      irp_expcnt(g2),r5       # r5 = expected transfer length
#
# --- determine if the completion status was success
#
        ldconst cmplt_success,r6        # r6 = default " success completion"
        cmpobe.t 0,g0,.ccs_110          # Jif everything OK
#
# --- there can be two errors at this point. they are data transfer underrun
#     or data transfer overrun. save the status in the extended status field
#     and calculate the true data transfer
#
#
# --- Check if the port type is iSCSI. If yes, skip the IOCB status check
#     Also if ICL port, skip the IOCP status check
#
        ld      oil2_xli(g1),g8         # g8 = pointer to xli
        ldob    xlichipi(g8),g8         # g8 = chip instance
c       r4 = ICL_IsIclPort((UINT8)g8);
        cmpobe  TRUE,r4,.ccs_110         # Jif ICL port
        ld      iscsimap,r4             # r4 = iscsi port map
        bbc     g8,r4,.ccs_100          # Jif not iSCSI port
        b       .ccs_110
#
.ccs_100:
        ldconst cmplt_DTO,r6            # r6 = default "data transfer overrun"
#
        ldob    0x08(g11),r3            # r3 = completion status
        ld      0xC(g11),r4             # r4 = residual transfer length
        stob    r3,irp_extcmp(g2)       # save true status as extended status
        cmpobne.f 0x15,r3,.ccs_110      # Jif not underrun (must be overrun)
#
        subo    r4,r5,r5                # r5 = expected - residual
#
# --- determine if a length error is requested.
#
        ldconst cmplt_DTU,r6            # r6 = default data transfer underrun
        ldob    irp_Tattr_flags(g2),r4  # r4 = irp Tattr flag
        bbc     tpf_SLIE,r4,.ccs_110    # Jif no suppression of length error
#
        ldconst cmplt_success,r6        # r6 = default " success completion"
#
# --- save the completion status and transfer length
#
.ccs_110:
        st      r5,irp_actcnt(g2)       # save actual transfer count
        stob    r6,irp_cmplt(g2)        # save completion status
#
.if     ITRACES
        call    it$trc_cmdcmplt         # *** trace cmd cmplt ***
.endif  # ITRACES
#
        call    apl$remtask             # remove task from working queue
        movq    g0,r4                   # save g0-g3
        movq    g4,r8                   # save g4-g7
        call    apl$chknextask          # determine if there is another task
        movq    r8,g4                   # restore g4-g7
        movq    r4,g0                   # restore g0-g3
#
        ld      oil2_xli(g1),g7         # g7 = address of XLI
        st      r13,oil2_xli(g1)        # clear xli pointer
        lda     -ILTBIAS(g1),g1         # ILT back to originator's level
.ifdef M4_DEBUG_XLI
c fprintf(stderr, "%s%s:%u put_xli 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g7);
.endif # M4_DEBUG_XLI
c       put_xli(g7);                    # Release xli back to pool.
        ld      il_cr(g1),r6            # r6 = originator's completion routine
        callx   (r6)                    # call originator's completion handler
                                        # to start up
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: apl$tsk_cmplt_CS
#
#  PURPOSE:
#       Process completion check status
#
#  DESCRIPTION:
#       A check status response was received from the device, This means that
#       that there is some error that requires other processing.
#
#       key|ASC|ASCQ|   description              processing
#       --------------------------------------------------------------------
#       --h 29h --h     Device reset            Retry the task
#       --h 2fh 00h     Command cleared by      Retry the task
#                        another initiator
#       --h 47h 00h     SCSI Parity Error       Retry the task
#
#  CALLING SEQUENCE:
#       call    apl$tsk_cmplt_CS
#
#  INPUT:
#       g0 = SCSI or Qlogic completion status
#       g1 = s-ILT at Initiator level 2.
#       g2 = IRP
#       g4 = CIMT
#       g6 = TLMT
#       g7 = ISMT
#       g11 = status type 0 IOCB (if g0 <> 0)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Reg. g1 and g2 are destroyed
#
#******************************************************************************
#
apl$tsk_cmplt_CS:
        ldconst cmplt_CSE,r3            # r3 = "check status" error
        stob    r3,irp_cmplt(g2)        # save completion status
#
# --- Check if the port type is iSCSI. If yes, skip the IOCB status check
#
        ld      oil2_xli(g1),g8         # g8 = pointer to xli
        ldob    xlichipi(g8),g8         # g8 = chip instance
c       r4 = ICL_IsIclPort((UINT8)g8);
        cmpobe  TRUE,r4,.cccs_icl01
        ld      iscsimap,r4             # r4 = iscsi port map
        bbc     g8,r4,.cccs_010         # Jif not iSCSI port
.cccs_icl01:
#                    # r4 = residual transfer length
        ld      -ILTBIAS+oil1_reslen(g1),r4
        ld      irp_expcnt(g2),r5       # r5 = expected transfer length
        subo    r4,r5,r5                # r5 = expected - residual
        st      r5,irp_actcnt(g2)       # save actual transfer count
#
        ld      -ILTBIAS+oil1_snsdata(g1),r7
        ldob    12(r7),r8               # r8 = ASC data
        ldob    13(r7),r9               # r9 = ASCQ data
        stob    r8,irp_extcmp(g2)       # save ASC extended status
        stob    r9,irp_extcmp+1(g2)     # save ASCQ extended status
#
        ldob    irp_SNSsize(g2),r3      # r3 = size of sense area
        cmpobe.t 0,r3,.cccs_200         # Jif no sense area defined
#
        ld      irp_SNSptr(g2),r8       # r8 = pointer to sense area
        ldconst 0x20,r4
        cmpo    r3,r4
        sell    r4,r3,r4                # select the lesser of the two
        stob    r4,irp_SNSlen(g2)       # save sense length
        mov     0,r5                    # clear r5
#
.cccs_005:
        ldob    (r7)[r5*1],r3           # get sense byte
        stob    r3,(r8)[r5*1]           # and save it
        addo    1,r5,r5                 # bump index
        cmpobne.t r4,r5,.cccs_005       # continue till all copied
#
        b       .cccs_200
#
# --- set transfer length
#
.cccs_010:
        ld      0x18(g11),r10           # r10 = residual transfer length
#
        ldos    0x16(g11),r4            # get scsi status
        bbs     9,r4,.cccs_020          # sns len valid
        ldconst 0,r12
        b       .cccs_030
#
.cccs_020:
        ld      0x1c(g11),r12           # r12 = sense data length
.cccs_030:
        lda     0x24(g11),r9            # r9 = base of sense/response data
        ld      0x20(g11),r8            # r8 = response data len 0,4, or 8
        addo    r8,r9,r11                # r9 = add response len to pointer
#
# --- set ACA and ACSQ codes in extended status
#
        mov     r11,r7                  # r7 = pointer to sense data
        ldob    12(r7),r8               # r8 = ASC data
        ldob    13(r7),r9               # r9 = ASCQ data
#
        ld      irp_expcnt(g2),r5       # r5 = expected transfer length
        mov     r10,r4                  # r4 = residual transfer length
        subo    r4,r5,r5                # r5 = expected - residual
        st      r5,irp_actcnt(g2)       # save actual transfer count
#
        stob    r8,irp_extcmp(g2)       # save ASC extended status
        stob    r9,irp_extcmp+1(g2)     # save ASCQ extended status
#
# --- determine if sense data must be moved. If it does, move it.
#
        ldob    irp_SNSsize(g2),r3      # r3 = size of sense area
        cmpobe.t 0,r3,.cccs_200         # Jif no sense area defined
#
        mov     r12,r4                  # r4 = sense data length
        ld      irp_SNSptr(g2),r7       # r7 = pointer to sense area
        cmpo    r3,r4
        sell    r4,r3,r4                # select the lesser of the two
        stob    r4,irp_SNSlen(g2)       # save sense length
        mov     0,r5                    # clear r5
#
.cccs_100:
        ldob    (r11)[r5*1],r3          # get sense byte
        stob    r3,(r7)[r5*1]           # and save it
        addo    1,r5,r5                 # bump index
        cmpobne.t r4,r5,.cccs_100       # continue till all copied
#
.cccs_200:
#
.if     ITRACES
        call    it$trc_cmdcmplt         # *** trace cmd cmplt ***
.endif  # ITRACES
#
# --- test for retry conditions
#
#     r8 = ASC
#     r9 = ASCQ
#
        ldconst 0x29,r3                 # Some form of reset notification ???
        cmpobe.f r3,r8,.cccs_300        # Jif yes
#
        ldconst 0x2f,r3                 # Some form of command cleared ???
        cmpobe.f r3,r8,.cccs_300        # Jif yes
#
        ldconst 0x47,r3                 # SCSI parity error ???
        cmpobne.f r3,r8,.cccs_950       # Jif no
#
.cccs_300:
        ldob    oil2_rtycnt(g1),r4      # r4 = retry counter
        cmpobe  0,r4,.cccs_950          # Jif last retry
        subo    1,r4,r4                 # subtract one
        stob    r4,oil2_rtycnt(g1)      # save new retry count
#
        call    apl$reschedule_task     # reschedule the task
        b       .cccs_1000              # br
#
.cccs_950:
        call    apl$remtask             # remove task from queue
#
        ld      oil2_xli(g1),g7         # g7 = address of XLI
        mov     0,r3
        st      r3,oil2_xli(g1)         # clear xli pointer
        lda     -ILTBIAS(g1),g1         # ILT back to originator's level
.ifdef M4_DEBUG_XLI
c fprintf(stderr, "%s%s:%u put_xli 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g7);
.endif # M4_DEBUG_XLI
c       put_xli(g7);                    # Release xli back to pool.
#
        ld      il_cr(g1),r6            # r6 = originator's completion routine
        callx   (r6)                    # call originator's completion handler
        call    apl$chknextask          # determine if there is another task
#
.cccs_1000:
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: apl$tsk_cmplt_nonCS
#
#  PURPOSE:
#       Process completion non check status
#
#  CALLING SEQUENCE:
#       call    apl$tsk_cmplt_nonCS
#
#  INPUT:
#       g0 = SCSI or Qlogic completion status
#       g1 = s-ILT at Initiator level 1.
#       g2 = IRP
#       g4 = ICIMT
#       g6 = TLMT
#       g11 = status type 0 IOCB (if g0 <> 0)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Reg. g1 and g2 are destroyed
#
#******************************************************************************
#
apl$tsk_cmplt_nonCS:
        ldconst cmplt_non_CSE,r3        # r3 = "non check status" error
        stob    r3,irp_cmplt(g2)        # save completion status
#
# --- set transfer length
#
        ldob    ici_chpid(g4),r5       # r5 = chip instance
c       r4 = ICL_IsIclPort((UINT8)r5) # See if it is ICL interface
        cmpobe  TRUE,r4,.atcn_icl01     # Jif ICL port
        ld      iscsimap,r4            # r4 = iscsi port map
        bbc     r5,r4,.atcn_10         # Jif NOT iSCSI port
.atcn_icl01:
        ld      -ILTBIAS+oil1_reslen(g1),r4
                                       # r4 = residual transfer length
        b       .atcn_100
#
.atcn_10:
        ld      0x18(g11),r4            # r4 = residual transfer length
.atcn_100:
        ld      irp_expcnt(g2),r5       # r5 = expected transfer length
        subo    r4,r5,r5                # r5 = expected - residual
        st      r5,irp_actcnt(g2)       # save actual transfer count
#
# --- set SCSI or Qlogic completion status
#
        st      g0,irp_extcmp(g2)       # save extended status
#
# --- set sense length.
#
        mov     0,r3
        stob    r3,irp_SNSlen(g2)       # set zero sense length
#
.if     ITRACES
        call    it$trc_cmdcmplt         # *** trace cmd cmplt ***
.endif  # ITRACES
#
        call    apl$remtask             # remove task from queue
#
        ld      oil2_xli(g1),g7         # g7 = address of XLI
        st      r3,oil2_xli(g1)         # clear xli pointer
        lda     -ILTBIAS(g1),g1         # ILT back to originator's level
.ifdef M4_DEBUG_XLI
c fprintf(stderr, "%s%s:%u put_xli 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g7);
.endif # M4_DEBUG_XLI
c       put_xli(g7);                    # Release xli back to pool.
#
        ld      il_cr(g1),r6            # r6 = originator's completion routine
        callx   (r6)                    # call originator's completion handler
        call    apl$chknextask          # determine if there is another task
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: apl$tsk_cmplt_IOE
#
#  DESCRIPTION:
#       These error would be:
#
#       error   description             process
#       -----   -----------             -----------------------------
#       0x02    DMA error               Fail task
#       0x03    transport error         Retry task
#       0x04    task aborted by LIP     Retry task
#       0x05    Command aborted         Retry task
#       0x06    Qlogic T/O              Retry task
#       0x11    Data reassembly error   Retry task
#       0x13    Aborted by target       Retry task
#
#  CALLING SEQUENCE:
#       call    apl$tsk_cmplt_IOE
#
#  INPUT:
#       g0 = SCSI or Qlogic completion status
#       g1 = s-ILT at Initiator level 1.
#       g4 = CIMT
#       g6 = TLMT
#       g11 = status type 0 IOCB (if g0 <> 0)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g1,g2, and g5 are destroyed.
#
#******************************************************************************
#
apl$tsk_cmplt_IOE:
        ldconst FALSE,r10               # r10 = no retry
? # crash - cqt# 24597 - 2008-06-25 - FE TLMT - failed @ apldrv.as:2841  ld 16+g6,g5 with caffcaff - workaround?
        ld      oil2_irp(g1),g2         # g2 = irp pointer
        ldconst cmplt_IOE,r8            # r8 = I/O error
#
# --- determine if error is a timeout
#
        ldconst ioe_timeout,r9          # r9 =  "timeout error"
        cmpobne.t 0x06,g0,.ccioe_100    # Jif not timeout
        b       .ccioe_900              # br
#
# --- determine if the error is a command aborted by LIP
#
.ccioe_100:
        ldconst ioe_lprst,r9            # r9 = "LIP reset"
        cmpobne.t 0x04,g0,.ccioe_200    # Jif LIP not reset
        b       .ccioe_900              # br
#
# --- determine if error is command aborted
#
.ccioe_200:
        ldconst ioe_cmdabrtd,r9         # r9 = "cmd aborted"
        cmpobne.t 0x05,g0,.ccioe_300    # Jif not command not aborted
        b       .ccioe_900              # br
#
# --- iSCSI sg error handling
#
.ccioe_300:
        ldob    ici_chpid(g4),r3       # r3 = chip instance
#
# --    Handle ICL port case, where the bit is not set in the bitmap
#
c       r4 = ICL_IsIclPort((UINT8)r3);
        cmpobe  TRUE,r4,.ccioe_400
#
        ld      iscsimap,r4            # r4 = iscsi port map
        bbs     r3,r4,.ccioe_400       # Jif iSCSI port
#
# --- error must be a DMA error
#
        ldconst ioe_DMAerr,r9           # r9 = "DMA error"
        ldconst FALSE,r10               # r10 = set to fail task
        b       .ccioe_905              # br
#
.ccioe_400:
        cmpobe.t 0x81,g0,.ccioe_700     # Jif No Connection
        cmpobe.t 0x82,g0,.ccioe_800     # Jif BUS Busy, retry
        cmpobe.t 0x83,g0,.ccioe_800     # Jif Timed out, retry
        cmpobe.t 0x84,g0,.ccioe_700     # Jif Target not responding
        cmpobe.t 0x85,g0,.ccioe_700     # Jif Req Abort
        cmpobe.t 0x86,g0,.ccioe_800     # Jif Parity Error, retry
        cmpobe.t 0x87,g0,.ccioe_800     # Jif Internal error, retry
        cmpobe.t 0x88,g0,.ccioe_700     # Jif SCSI bus reset
        cmpobe.t 0x89,g0,.ccioe_800     # Jif Interrupted, retry
        cmpobe.t 0x8a,g0,.ccioe_800     # Jif Passthru error, retry
        cmpobe.t 0x8b,g0,.ccioe_800     # Jif Driver error, retry
        cmpobe.t 0x8c,g0,.ccioe_800     # Jif retry
        cmpobe.t 0x8d,g0,.ccioe_800     # Jif retry
#
.ccioe_700:
        ldconst FALSE,r10               # r10 = do not retry
        b       .ccioe_910              # br
#
.ccioe_800:
        ldconst TRUE,r10                # r10 = retry
        b       .ccioe_910              # br
#
.ccioe_900:
        ldconst TRUE,r10                # r10 = retry task
.ccioe_905:
        mov    r9,g0
#
.ccioe_910:
        stob    r8,irp_cmplt(g2)        # save completion status
        st      g0,irp_extcmp(g2)       # save extended status info
#
.if     ITRACES
        call    it$trc_cmdcmplt         # *** trace cmd cmplt ***
.endif  # ITRACES
#
        cmpobe.t FALSE,r10,.ccioe_950   # Jif no retry
#
        ldob    oil2_rtycnt(g1),r4      # r4 = retry counter
        cmpobe  0,r4,.ccioe_950         # Jif last retry
        subo    1,r4,r4                 # subtract one
        stob    r4,oil2_rtycnt(g1)      # save new retry count
#
        call    apl$reschedule_task     # reschedule the task
        b       .ccioe_1000             # br
#
.ccioe_950:
        call    apl$remtask             # remove task from queue
#
        ld      oil2_xli(g1),g7         # g7 = address of XLI
        mov     0,r3
        st      r3,oil2_xli(g1)        # clear xli pointer
        lda     -ILTBIAS(g1),g1         # ILT back to originator's level
.ifdef M4_DEBUG_XLI
c fprintf(stderr, "%s%s:%u put_xli 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g7);
.endif # M4_DEBUG_XLI
c       put_xli(g7);                    # Release xli back to pool.
#
        ld      il_cr(g1),r6            # r6 = originator's completion routine
        callx   (r6)                    # call originator's completion handler
        call    apl$chknextask          # determine if there is another task
#
.ccioe_1000:
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: apl$tsk_cmplt_misc
#
#  PURPOSE:
#       Process miscellaneous error from the QLogic
#
#  DESCRIPTION:
#       These error would be:
#       error   description             process
#       -----   -----------             -----------------------------
#       28h     Port unavailable        Activate discovery process,
#                                       fail the task
#       29h     Port logged off         Activate discovery process,
#                                       fail the task
#       2ah     port configuration      Activate discovery process,
#               change                  reschedule task
#        NOTE: See fsl_sgio_cb function in fsl.c for iscsi ME values
#
#  CALLING SEQUENCE:
#       call    apl$tsk_cmplt_misc
#
#  INPUT:
#       g0 = SCSI or Qlogic completion status
#       g1 = s-ILT at Initiator level 1.
#       g4 = CIMT
#       g6 = TLMT
#       g11 = status type 0 IOCB (if g0 <> 0)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g1, g2, and g5 are destroyed.
#
#******************************************************************************
#
apl$tsk_cmplt_misc:
        ld      oil2_irp(g1),g2         # g2 = irp pointer
        ldconst cmplt_ME,r3             # r3 = miscellaneous error
        ldconst 0x2a,r4                 # r4 = port configuration changed value
        ldconst 0x29,r5                 # r5 = port logged off
#
        stob    r3,irp_cmplt(g2)        # save completion status
        st      g0,irp_extcmp(g2)       # save extended status info
#
.if     ITRACES
        call    it$trc_cmdcmplt         # *** trace cmd cmplt ***
.if     debug_tracestop5
        ldconst 0x28,r6                 # r6 = port unavailable
        cmpobne r6,g0,.ccmisc_10        # JIf not port unavailable
        ldconst 0,r13
        stos    r13,ici_tflg(g4)        # clear trace flags to stop traces
.ccmisc_10:
.endif  # debug_tracestop5
.endif  # ITRACES
        cmpobe.f 0x2a,g0,.ccmisc_100       # Jif "port configuration changed"
        cmpobe.f 0x29,g0,.ccmisc_900       # Jif not "port logged off"
        cmpobe.f 0x28,g0,.ccmisc_900       # Port unavailable
#
        ldob    ici_chpid(g4),r3         # r3 = chip instance
#
# --- Handle ICL port case, where the bit is not set in the bitmap
#
c       r4 = ICL_IsIclPort((UINT8)r3) # See if it is ICL interface
        cmpobe  TRUE,r4,.ccmisc_icl01    # Jif ICL port
        ld      iscsimap,r4              # r4 = iscsi port map
        bbc     r3,r4,.ccmisc_900        # Jif not iSCSI port
.ccmisc_icl01:
#
        cmpobe.f 0x01,g0,.ccmisc_100   # Jif Driver Busy to retry
        cmpobe.f 0x02,g0,.ccmisc_900   # Jif Driver Soft Error to retry
        cmpobe.f 0x03,g0,.ccmisc_900   # Jif Driver Media Error to retry
        cmpobe.f 0x04,g0,.ccmisc_900   # Jif Driver Error to retry
        cmpobe.f 0x05,g0,.ccmisc_900   # Jif Driver Invalid to retry
        cmpobe.f 0x06,g0,.ccmisc_900   # Jif Driver Timeout to retry
        cmpobe.f 0x07,g0,.ccmisc_900   # Jif Driver Hard Error to retry
        cmpobe.f 0x08,g0,.ccmisc_900   # Jif Driver Sense to retry
        cmpobe.f 0x10,g0,.ccmisc_100   # Jif Driver Retry to retry
        cmpobe.f 0x20,g0,.ccmisc_900   # Jif Driver Abort to retry
        cmpobe.f 0x30,g0,.ccmisc_20    # Jif Driver REMAP to retry
        cmpobe.f 0x40,g0,.ccmisc_900   # Jif Driver Die to retry
        b    .ccmisc_900               # Driver Abort / unknown - abort
#
# --- the port of the intended IO has logged off. Determine if there are
#     any other problems on the loop by performing a discovery process.
#
.ccmisc_20:
        ld      tlm_tmt(g6),g5          # g5 = TMT loaded only when needed
#        ldconst 0xff,r4                 # r4 = invalid ALPA flag
#        stob    r4,tm_alpa(g5)          # set alpa invalid
#
        ldob    tm_flag(g5),r3          # r3 = flag byte
        setbit  tmflg_lundisc,r3,r3     # set LUN discovery request flag
        stob    r3,tm_flag(g5)          # save
#
        movq    g0,r4
        movq    g4,r8
        movq    g8,r12
        ldob    ici_chpid(g4),g0        # g0 = chip instance
        ldob    ici_mylid(g4),g1        # g1 = LID of this initiator
        ldconst ispolop,g2              # g2 = event type
        call    I$recv_online           # rediscover loop
        movq    r4,g0
        movq    r8,g4
        movq    r12,g8
        b       .ccmisc_900             # br
#
# --- port configuration has changed
#
.ccmisc_100:
        ldob    oil2_rtycnt(g1),r4      # r4 = retry counter
        cmpobe.f 0,r4,.ccmisc_900       # Jif last retry
        subo    1,r4,r4                 # subtract one
        stob    r4,oil2_rtycnt(g1)      # save new retry count
#
        call    apl$reschedule_task     # reschedule the task
        b       .ccmisc_1000            # br
#
.ccmisc_900:
        call    apl$remtask             # remove task from queue
#
        ld      oil2_xli(g1),g7         # g7 = address of XLI
        mov     0,r3
        st      r3,oil2_xli(g1)         # clear xli pointer
        lda     -ILTBIAS(g1),g1         # ILT back to originator's level
.ifdef M4_DEBUG_XLI
c fprintf(stderr, "%s%s:%u put_xli 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g7);
.endif # M4_DEBUG_XLI
c       put_xli(g7);                    # Release xli back to pool.
#
        ld      il_cr(g1),r6            # r6 = originator's completion routine
        callx   (r6)                    # call originator's completion handler
        call    apl$chknextask          # determine if there is another task
#
.ccmisc_1000:
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: apl$path_logout
#
#  PURPOSE:
#       Sends a task back to it's originator.
#
#  DESCRIPTION:
#       Path for this task is no more - Task has been removed from the work queue.
#
#  CALLING SEQUENCE:
#       call    apl$path_logout
#
#  INPUT:
#       g1 = ILT at Initiator level 2.
#       g4 = CIMT
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
apl$path_logout:
        ld      oil2_irp(g1),g2         # g2 = irp pointer
        ld      tlm_tmt(g6),g5          # g5 = TMT
        ldconst cmplt_LOUT,r3           # r3 = error due to logout
#
        stob    r3,irp_cmplt(g2)        # save completion status
        st      g0,irp_extcmp(g2)       # save extended status info
#
        call    apl$remtask             # remove task from queue
#
        ld      oil2_xli(g1),g7         # g7 = address of XLI
        mov     0,r3
        st      r3,oil2_xli(g1)         # clear xli pointer
        lda     -ILTBIAS(g1),g1         # ILT back to originator's level
.ifdef M4_DEBUG_XLI
c fprintf(stderr, "%s%s:%u put_xli 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g7);
.endif # M4_DEBUG_XLI
c       put_xli(g7);                    # Release xli back to pool.
#
        ld      il_cr(g1),r6            # r6 = originator's completion routine
        callx   (r6)                    # call originator's completion handler
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: apl$domtsk_abort
#
#  PURPOSE:
#       Sends a dormant task back to it's originator.
#
#  DESCRIPTION:
#       Task has been removed from the work queue.
#
#  CALLING SEQUENCE:
#       call    apl$domtsk_abort
#
#  INPUT:
#       g1 = ILT at Initiator level 2.
#       g4 = CIMT
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
apl$domtsk_abort:
        mov     g7,r3                   # save g7
        ld      oil2_xli(g1),g7         # g7 = address of XLI
        mov     0,r4
        st      r4,oil2_xli(g1)         # clear xli pointer
#
        cmpobe.t 0,g7,.domtsk_abt100    # JIf no XLI defined
#
.ifdef M4_DEBUG_XLI
c fprintf(stderr, "%s%s:%u put_xli 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g7);
.endif # M4_DEBUG_XLI
c       put_xli(g7);                    # Release xli back to pool.
#
.domtsk_abt100:
        lda     -ILTBIAS(g1),g1         # g1 = ILT back to originator's level
        ld      il_cr(g1),r15           # r15 = originator's completion routine
        callx   (r15)                   # call originator's completion handler
        mov     r3,g7                   # restore g7
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: apl$acttsk_abort
#
#  PURPOSE:
#       Schedule an abort of an active task.
#
#  DESCRIPTION:
#       Places the ILT for the task on the abort queue and create a
#       task to abort ILT at next opportunity.
#
#  CALLING SEQUENCE:
#       call    apl$acttsk_abort
#
#  INPUT:
#       g1 = ILT of task at 2nd level.
#       g2 = IRP
#       g4 = CIMT
#       g5 = TMT
#       g6 = TLMT
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
apl$acttsk_abort:
        movq    g0,r4                   # save g0-g3 in r4-r7
        movt    g8,r8                   # save g8-g10 in r8-r10
#
# --- clear retry count
#
        mov     0,r3
        stob    r3,oil2_rtycnt(g1)      # clear retry counter
#
# --- place task on abort queue
#
        call    apl$qtask2aq            # place ILT on abort queue
#
# --- fork a task to abort ILT
#
        ldob    tm_chipID(g5),g8       # g8 = chip instance
#
# ---   Handle ICL port case, where the bit is not set in the bitmap
#
c       r3   =  ICL_IsIclPort((UINT8)g8);
        cmpobe  TRUE,r3,.aactta_100
        ld      iscsimap,r3            # r3 = iscsi port map
        bbs     g8,r3,.aactta_100      # Jif iSCSI port
#
# --- in FC case, this is called from the timer task context
#
        mov     r5,g1                   # restore ILT at oil2 level
        ldconst OIL2TOVAL,r3            # r3 = 2sec (sec to 250ms increments)
        stos    r3,oil2_tmr1(g1)        # save gross task t/o value
        lda     ILTBIAS(g1),g3          # g3 = ILT at 3rd lvl
        ldos    tm_lid(g5),g1           # g1 = LID
        ldos    tlm_lun(g6),g2          # g2 = LUN
        mov     g8,g0                   # g0 = chip instance
        call    ISP$abort_iocb          # ABORT the TASK
        ret
#
.aactta_100:
        ld      ILTBIAS+oil3_flag(g1),r11
        mov     0x01,r12
        or      r11,r12,r12
        st      r12,ILTBIAS+oil3_flag(g1)
#
        ldos    tm_lid(g5),g9           # g9 = LID
        ldos    tlm_lun(g6),g10         # g10 = LUN
        mov     g1,g3                   # place ILT in g3
        lda     apl$abort_task,g0       # routine to abort task
        ldconst APLABPRI,g1             # set priority
c       CT_fork_tmp = (ulong)"apl$abort_task";
        call    K$fork                  # create task
        movq    r4,g0                   # restore g0-g3 from r4-r7
        movt    r8,g8                   # restore g8-g10 from r8-r10
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: apl$abort_task1
#
#  PURPOSE:
#       Abort a task on the abort task list.
#
#  DESCRIPTION:
#       Check if the task is still on the ISP's ilt thread. If it is, abort the task and setup to
#       wait for the response. Otherwise, completes the ILT back to the upper layers with error.
#
#  CALLING SEQUENCE:
#       call    apl$abort_task1
#
#  INPUT:
#       g1 = ILT of task at 2nd level.
#       g2 = IRP
#       g4 = CIMT
#       g5 = TMT
#       g6 = TLMT
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g0 - g7 may be destroyed
#
#******************************************************************************
#
apl$abort_task2:
        mov     g2,r10
#                                       # g2=port
        ld      resilk,r3               # Get interlock bits
        bbc     g2,r3,.abttsk2_10       # Jif interlocked
        ldconst 100,g0                  # set up to wait 100 ms
        call    K$twait                 # delay task
        b       apl$abort_task2
#
.abttsk2_10:
        ld      I_CIMT_dir[g2*4],g4     # g4 = ICIMT dir pointer
        ld      ici_actqhd(g4),g6       # g6 = tlmt from active Q hd
#
.abttsk2_100:
        ld      resilk,r3               # Get interlock bits
        bbs     g2,r3,apl$abort_task2   # Jif reset in progress
#
        cmpobe.f 0,g6,.abttsk2_1000     # Jif no link
        ld      tlm_ahead(g6),g1        # g1 = ILT of task at 1st lvl
#
.abttsk2_200:
        ld      resilk,r3               # Get interlock bits
        bbs     g2,r3,apl$abort_task2   # Jif reset in progress
#
        cmpobe.f 0,g1,.abttsk2_300      # Jif no tasks active
        ld      tlm_tmt(g6),g5          # g5 = possible TMT
        ld      oil2_irp(g1),g2         # g2 = irp address
        ld      il_fthd(g1),r14         # r14 = forward thread of ILT
        call    apl$abort_task1
        mov     r14,g1
        mov     r10,g2                  # restore port
        b       .abttsk2_200
#
.abttsk2_300:
        ld      tlm_flink(g6),g6        # g6 = link to next TLMT
        b       .abttsk2_100
#
.abttsk2_1000:
        mov     r10,g2                  # restore port
        ldconst 0,g0
        st      g0,at2_pcb[g2*4]        # clear pcb
        ret                             # return to caller
#
#
apl$abort_task1:
        movq    g0,r4                   # save g0-g3 in r4-r7
        movq    g4,r8                   # save g4-g7 in r8-r11
#
        ldob    tm_chipID(g5),g0        # g0 = chip instance
        ld      iscsimap,r3             # r3 = iscsi port map
        bbs     g0,r3,.abttsk1_1000     # Jif iSCSI port - exit
#
        ld      resilk,r3               # Get interlock bits
        bbc     g0,r3,.abttsk1_10       # Jif interlocked
#
c fprintf(stderr,"%s%s:%u SEMINOLE: abort_task1 - TMT=0x%lX TLMT=0x%lX ILT=0x%lX Reset in Progress\n", FEBEMESSAGE, __FILE__, __LINE__, g5, g6, g1);
#
        ld      at2_pcb[g0*4],r3        # load pcb
        cmpobne 0,r3, .abttsk1_1000     # Do NOT run two abort handlers for same port at same time
#
        mov     g0,g2                   # g2=port
c       g0 = -1;                        # Flag task being created.
        st      g0,at2_pcb[g2*4]        # save pcb
        lda     apl$abort_task2,g0      # routine to abort task
        ldconst APLABPRI,g1             # set priority
c       CT_fork_tmp = (ulong)"apl$abort_task2";
        call    K$fork                  # create task
        st      g0,at2_pcb[g2*4]        # save pcb
        b       .abttsk1_1000           # done
#
.abttsk1_10:
        lda     ILTBIAS(g1),g1          # g1 = ILT back to originator's level
        call    ISP$ilt_thread_find
        cmpobe.t g1,g0,.abttsk1_100     # JIf ILT found
#
        mov     r5,g1                   # restore ILT at oil2 level
c fprintf(stderr,"%s%s:%u SEMINOLE: abort_task1 - TMT=0x%lX TLMT=0x%lX ILT=0x%lX not found\n", FEBEMESSAGE, __FILE__, __LINE__, g5, g6, g1);
        call    apl$remtask             # remove task from queue
        ld      oil2_xli(g1),g7         # g7 = address of XLI
        mov     0,r3
        st      r3,oil2_xli(g1)         # clear xli pointer
        cmpobne 0,g7,.abttsk1_50        # xli pointer not null
c fprintf(stderr,"%s%s:%u SEMINOLE: abort_task1 - ILT not found, xli already freed up\n", FEBEMESSAGE, __FILE__, __LINE__);
        b       .abttsk1_1000           # done
#
.abttsk1_50:
        lda     -ILTBIAS(g1),g1         # g1 = ILT back to originator's level
.ifdef M4_DEBUG_XLI
c fprintf(stderr, "%s%s:%u put_xli 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g7);
.endif # M4_DEBUG_XLI
c       put_xli(g7);                    # Release xli back to pool.
#
        ld      il_cr(g1),r15           # r15 = originator's completion routine
        callx   (r15)                   # call originator's completion handler
        b       .abttsk1_1000           # done
#
.abttsk1_100:
        mov     r5,g1                   # restore ILT at oil2 level
        lda     ILTBIAS(g1),g3          # g3 = ILT at 3rd lvl
        ldos    tm_lid(g5),g1           # g1 = LID
        ldos    tlm_lun(g6),g2          # g2 = LUN
        ldob    tm_chipID(g5),g0        # g0 = chip instance
c fprintf(stderr, "%s%s:%u \n", FEBEMESSAGE, __FILE__, __LINE__);
        call    ISP$abort_iocb          # ABORT the TASK
#
.abttsk1_1000:
        movq    r4,g0                   # restore g0-g3 from r4-r7
        movq    r8,g4                   # restore g4-g7 from r8-r11
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: apl$abort_task
#
#  PURPOSE:
#       Abort a task.
#
#  DESCRIPTION:
#       Check if the task is still on the abort queue. If it is, abort the task and setup to
#       wait for the response.
#
#  CALLING SEQUENCE:
#       call    apl$abort_task
#
#  INPUT:
#       g2 = IRP
#       g3 = ILT of task at 2nd level.
#       g4 = CIMT address
#       g6 = TLMT address
#       g8 = chip instance
#       g9 = LID
#       g10 = LUN
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g0 - g12 may be destroyed
#
#******************************************************************************
#
apl$abort_task:
#
# --- determine if the task is still on the abort queue
#
? # crash - cqt# 24597 - 2008-06-26 - FE TLMT - failed @ apldrv.as:3455  ld 32+g6,r4 with caffcaff
?       ld      tlm_ahead(g6),r4        # r4 = head of abort queue
.ifdef M4_DEBUG_MEMORY_WITH_PATTERNS
        cmpobe  0xcaffcaff,r4,abttsk_1000 # Jif freed memory pattern
.endif  # M4_DEBUG_MEMORY_WITH_PATTERNS
#
abttsk_100:
        cmpobe.f 0,r4,abttsk_1000       # Jif zero link - task has completed
        cmpobe.t g3,r4,abttsk_200       # Jif task found
        ld      il_fthd(r4),r4          # r4 = next task on queue
        b       abttsk_100              # continue
#
# --- the task is still on the abort queue, so sent an abort task operation
#     to the ISP level.
#
abttsk_200:
#
# ---   Handle ICL port case, where the bit is not set in the bitmap
#
c       r3 = ICL_IsIclPort((UINT8)g8);
        cmpobe  TRUE,r3,.abttsk_icl01
        ld      iscsimap,r3             # r3 = iscsi port map
        bbc     g8,r3,abttsk_500        # Jif not iSCSI port
.abttsk_icl01:
#
# --- For iSCSI, if the abort is because of timeout, then logout the
#     session. The timeout specified to the SG when issuing the cmd was the
#     same as the timeout that expired. So, the sg should have timed out by now
#     unless it ran into some serious issue.
#
        ld      oil2_irp(g3),r4         # r4 = irp address
        ldob    irp_cmplt(r4),r5        # r5 =  completion status
        ld      irp_extcmp(r4),r6       # r6 =  extended status info
        cmpobne.f cmplt_IOE,r5,abttsk_1000    # Jif not cmplt_IOE
        cmpobne.f ioe_timeout,r6,abttsk_1000  # Jif not ioe_timeout
#
c       fprintf(stderr,"APL AbortTask p=%d l=%d\n", (UINT32)g8, (UINT32)g9);
        PushRegs(r3)
        mov     g8,g0                   # g0 = chip id
        mov     g9,g1                   # g1 = lid
        call    fsl_logout
        PopRegsVoid(r3)
        b       abttsk_1000             # exit out
#
# --- sent an abort task operation to the ISP level.
#
abttsk_500:
        lda     ILTBIAS(g3),g3          # g3 = ILT at 3rd lvl
        mov     g8,g0                   # g0 = chip instance
        mov     g9,g1                   # g1 = LID
        mov     g10,g2                  # g2 = LUN
        call    ISP$abort_iocb          # ABORT the TASK
#
abttsk_1000:
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: apl$acttsk_2abortQ
#
#  PURPOSE:
#       Places a task on the abort Queue.
#
#  DESCRIPTION:
#       Places the ILT for the task on the abort queue.
#
#  CALLING SEQUENCE:
#       call    apl$acttsk_2abortQ
#
#  INPUT:
#       g1 = ILT of task at 2nd level.
#       g6 = TLMT
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
apl$acttsk_2abortQ:
        call    apl$qtask2aq            # place s-ILT on abort queue
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: apl$tsk_aborted
#
#  PURPOSE:
#       Releases the ILT back to the originator after the response has been
#       received from the ISP layer in response to the aborted task.
#
#  DESCRIPTION:
#       Releases the XLI back to the XLI pool, set the ILT to the originators
#       level, and calls the originators completion routine.
#
#  CALLING SEQUENCE:
#       call    apl$tsk_aborted
#
#  INPUT:
#       g1 = ILT of task at 2nd level
#       g6 = TLMT
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None
#
#******************************************************************************
#
apl$tsk_aborted:
        mov     g7,r12                  # save g7 in r12
        call    apl$remtask             # remove task from queue
#
        ldconst cmplt_IOE,r8            # r8 = I/O error
        ldconst ioe_cmdabrtd,r9         # r9 = "cmd aborted"
        ld      oil2_irp(g1),g7         # g7 = irp address
        stob    r8,irp_cmplt(g7)        # save completion status
        st      r9,irp_extcmp(g7)       # save extended status info
#
        ld      oil2_xli(g1),g7         # g7 = address of XLI
        mov     0,r3
        st      r3,oil2_xli(g1)         # clear xli pointer
        lda     -ILTBIAS(g1),g1         # g1 = ILT back to originator's level
.ifdef M4_DEBUG_XLI
c fprintf(stderr, "%s%s:%u put_xli 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g7);
.endif # M4_DEBUG_XLI
c       put_xli(g7);                    # Release xli back to pool.
#
        ld      il_cr(g1),r15           # r15 = originator's completion routine
        callx   (r15)                   # call originator's completion handler
#
        mov     r12,g7                  # restore g7 from r12
        ret                             # return to caller
#
#******************************************************************************
#******************************************************************************
#******************************************************************************
#
# ____________________________ SUBROUTINES ____________________________________
#
#******************************************************************************
#******************************************************************************
#******************************************************************************
#
#  NAME: alp$qtask2wq
#
#  PURPOSE:
#       Queues an ILT associated with an initiator task to the
#       working queue in the associated TLMT.
#
#  DESCRIPTION:
#       Places the specified ILT onto the end of the working list
#       of tasks associated with the specified TLMT.
#
#  CALLING SEQUENCE:
#       call    apl$qtask2wq
#
#  INPUT:
#       g1 = ILT of task at 2nd level
#       g6 = TLMT address
#
#  OUTPUT:
#       g0 = TRUE if queue was empty.
#       g0 = FALSE if queue was NOT empty.
#
#  REGS DESTROYED:
#       Reg. g0 destroyed.
#
#******************************************************************************
#
apl$qtask2wq:
        ldl     tlm_whead(g6),r14       # r14 = first element on list
                                        # r15 = last element on list
        lda     tlm_whead(g6),r13       # r13 = base address of queue
        ldconst oil2_ts_dorm,r5         # r5 = task state
        lda     .Ts_wait2activate,r6    # r6 = event handler table address
        cmpobne.t 0,r14,.qwq100_a       # Jif queue not empty
#
# --- Case: Queue was empty.
#
        mov     r13,r15                 # set base of queue as backward thread
        stl     r14,il_fthd(g1)         # save forward/backward threads in
                                        #  ILT
        mov     g1,g0
        stl     g0,(r13)                # save ILT as head & tail pointer
        mov     TRUE,g0                 # indicate queue was empty to caller
        b       .qwq900_a               # and we're out of here!
#
# --- Case: Queue was NOT empty. Place on end of queue.
#
.qwq100_a:
        mov     0,r3
        st      g1,il_fthd(r15)         # link new ILT onto end of list
        st      g1,tlm_wtail(g6)        # save new ILT as new tail
        st      r3,il_fthd(g1)          # clear forward thread in new ILT
        st      r15,il_bthd(g1)         # save backward thread in new ILT
        mov     FALSE,g0                # indicate queue was NOT empty
                                        #  to caller
.qwq900_a:
        stob    r5,oil2_tstate(g1)      # set state to dormant
        st      r6,oil2_ehand(g1)       # save new event handler address
        ret
#
#******************************************************************************
#
#  NAME: apl$qtask2aq
#
#  PURPOSE:
#       Queues an ILT associated with an initiator task to the
#       aborted queue in the associated TLMT.
#
#  DESCRIPTION:
#       Places the specified ILT onto the end of the aborted list
#       of tasks associated with the specified TLMT.
#
#  CALLING SEQUENCE:
#       call    apl$qtask2aq
#
#  INPUT:
#       g1 = ILT of task at 2nd level
#       g6 = TLMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
apl$qtask2aq:
        ldl     tlm_ahead(g6),r14       # r14 = first element on list
                                        # r15 = last element on list
        lda     tlm_ahead(g6),r13       # r13 = base address of queue
        ldconst  oil2_ts_abt,r5         # r5 = task state
        lda     .Ts_cmd_abort,r6        # r6 = event handler table address
        cmpobne.t 0,r14,.qaq100_a       # Jif queue not empty
#
# --- Case: Queue was empty.
#
        mov     r13,r15                 # set base of queue as backward thread
        stl     r14,il_fthd(g1)         # save forward/backward threads in
                                        #  ILT
        st      g1,(r13)                # save ILT as head & tail pointer
        st      g1,4(r13)
        b       .qaq900_a               # and we're out of here!
#
# --- Case: Queue was NOT empty. Place on end of queue.
#
.qaq100_a:
        mov     0,r3
        st      g1,il_fthd(r15)         # link new ILT onto end of list
        st      g1,tlm_atail(g6)        # save new ILT as new tail
        st      r3,il_fthd(g1)          # clear forward thread in new ILT
        st      r15,il_bthd(g1)         # save backward thread in new ILT
#
.qaq900_a:
        stob    r5,oil2_tstate(g1)      # set state to dormant
        st      r6,oil2_ehand(g1)       # save new event handler address
        ret
#
#******************************************************************************
#
#  NAME: apl$remtask
#
#  PURPOSE:
#       Removes the specified ILT associated with an initiator
#       task from a queue in the specified TLMT if appropriate.
#
#  DESCRIPTION:
#       Checks the flags in the specified ILT to see if it resides
#       on a queue maintained in the specified TLMT. If so,
#       removes the ILT from the appropriate queue and clears the
#       flag indicating the ILT is no longer on the queue. If the
#       ILT has no flags indicating it resides on a queue in the TLMT,
#       this routine simply returns to the caller.
#
#  CALLING SEQUENCE:
#       call    apl$remtask
#
#  INPUT:
#       g1 = ILT address at 2nd nest level
#       g6 = TLMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
apl$remtask:
? # crash - cqt# 24598 - 2008-11-03. Did "vlinkctrlvdisks 0", and got crash on ldl il_fthd
# line below. apl$remtask called from: i$fabric_DeleteTarget apldrv.as:3822.
#    g1 = 0x00000001  and that was called from idriver.as, line 8301
#    g6 = TLMT address, and the structure is whead = 0x1, ahead = 0x0,
#    flink=0x9800f590, blink=0x9801ec50, state=0x76, maxQ=0, enblcnt=0,
#    cimt=0x810001, tmt=0, Shead=0, whead=1, wtail=0, ahead=0, atail = 0,
#    lun=0xffff, tmr0=0x800, tmr1=0xef02, sescnt=0x20, pdt=0xb2, snlen=0x2,
#    dvflgs=0x7f, proid="Xiotech Virtual-", version="ISE , blkcnt =0x20202020,
#    blksz=0x0.
        ldl     il_fthd(g1),r14         # r14 = forward thread of ILT
                                        # r15 = backward thread of ILT
        ldob    oil2_tstate(g1),r5      # r5 = task state
        lda     tlm_whead(g6),r13       # r13 = base address of working queue
        cmpobne.t oil2_ts_abt,r5,.remt100 # JIf not aborted
        lda     tlm_ahead(g6),r13       # r13 = base address of aborted queue
#
.remt100:
        st      r14,il_fthd(r15)        # put forward thread from removed ILT
                                        #  as forward thread of previous ILT
        cmpobne.t 0,r14,.remt700_a      # Jif non-zero forward thread
        mov     r13,r14                 # make base of queue the forward
                                        #  thread
        cmpobne.t r13,r15,.remt700_a    # Jif backward thread <> base of
                                        #  queue
        mov     0,r15                   # queue is now empty!
#
.remt700_a:
        st      r15,il_bthd(r14)        # put backward thread from removed
                                        #  ILT as backward thread of previous
                                        #  ILT.
        ldos    tlm_enblcnt(g6),r8      # r8 = enabled task counter
        cmpobne.f oil2_ts_enbl,r5,.remt1000_a # Jif task was not enabled
#
# --- Task was on work queue and is enabled which means the task
#       counter (tlm_enblcnt) was incremented and therefore needs
#       to be decremented.
#
        cmpobe.f 0,r8,.remt1000_a       # Jif counter already 0
        subo    1,r8,r8                 # dec. counter
        stos    r8,tlm_enblcnt(g6)      # save updated counter
#
.remt1000_a:
        ret
#
#******************************************************************************
#
#  NAME: apl$enable_task
#
#  PURPOSE:
#       Enables a task as appropriate.
#
#  DESCRIPTION:
#       Places a task into the enabled state.
#
#  CALLING SEQUENCE:
#       call    apl$enable_task
#
#  INPUT:
#       g1 = pri. ILT of task at the oil2 nest level
#       g4 = CIMT address
#       g5 = TMT address
#       g6 = TLMT address
#
#  OUTPUT:
#       g1 = next task on work queue if next task can be enabled.
#       g1 = 0 if no other tasks on work queue or next
#               task on work queue cannot be enabled.
#
#  REGS DESTROYED:
#       None
#
#******************************************************************************
#
apl$enable_task:
        mov     g7,r13                  # save g7
        mov     g2,r14                  # save g2
#
        ldob    ici_state(g4),r3        # r3 = CIMT state
        ldob    tlm_state(g6),r4        # r4 = TLMT state
#
        cmpobne.f Cs_online,r3,.entask_900 # JIf loop not online
        cmpobne.f tlmstate_active,r4,.entask_900 # JIf device is not active
#
        ld      oil2_irp(g1),g2         # g2 = address of irp
        ld      irp_Pro_ID(g2),r15      # r15 = ismt
#
        lda     apl$recv_cmplt_rsp,r3   # r3 = completion routine
        lda     .Ts_cmd_rsp,r4          # r4 = event handler tbl ptr
        ldos    tlm_enblcnt(g6),r5      # r3 = enabled task counter
        ldconst oil2_ts_enbl,r6         # r5 = task state
#
        addo    1,r5,r5                 # inc. enabled task counter
#
        st      r3,oil2_cr(g1)          # Save completion routine
        st      r4,oil2_ehand(g1)       # save event handler tbl address
        stos    r5,tlm_enblcnt(g6)      # save updated counter
        stob    r6,oil2_tstate(g1)      # set state to enabled
#
        mov     0,r7
        stob    r7,irp_cmplt(g2)        # make sure completion status is zero
        st      r7,irp_extcmp(g2)       # make sure  extended status is zero
#
.if     ITRACES
        mov     r15,g7                  # g7 = ISMT address
        call    it$trc_enbltsk          # *** trace enable task ***
.endif  # ITRACES
#
# --- Determine if there is an XLI already allocated
#
        ld      oil2_xli(g1),g7         # g7 = possible pointer to XLI
        cmpobne.f 0,g7,.entask_150      # Jif already have one
#
# --- There is no XLI allocated, allocate an XLI and fill it in
#     using the IRP and ISMT.
#
c       g7 = get_xli();                 # Allocate a XLI
.ifdef M4_DEBUG_XLI
c fprintf(stderr, "%s%s:%u get_xli 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g7);
.endif # M4_DEBUG_XLI
        st      g7,oil2_xli(g1)         # save pointer to XLI
        ldob    tm_chipID(g5),r4        # r4 = chip instance
        ldos    tm_lid(g5),r5           # r5 = loop ID for target
        ldos    tlm_lun(g6),r6          # r6 = LUN for target
#
        stob    r4,xlichipi(g7)         # set chip ID
        stos    r5,xlitarget(g7)        # store target
        stos    r6,xlilun(g7)           # store LUN
#
# --- set the command timeout value
#
        ldos    irp_cmdTO(g2),r3        # r3 = command timeout
        cmpobne.t 0,r3,.entask_110      # Jif timer value defined
        ldos    ism_defTO(r15),r3       # r3 = default T/O value
#
.entask_110:
        stos    r3,xlitime(g7)          # set timeout to default
#
# --- determine if there is enough time remaining on the gross timer
#     to execute the command. If not, bump the gross timer.
#
        ldos    oil2_tmr1(g1),r4        # r4 = current gross timer
        cmpoble OIL2TOVAL,r4,.entask_112        # Jif there is at least 2 second
#
        addo    1,r3,r3                 # add one second
        shlo    2,r3,r3                 # r3 = r3 * 4 (sec to 250ms increments)
        stos    r3,oil2_tmr1(g1)        # save gross task t/o value
#
# --- set up SGL
#
.entask_112:
        ld      irp_SGLptr(g2),r6       # r6 = SGL pointer
        lda     irp_CDB(g2),r5          # r5 = CDB pointer
#        ldob   0(r5),r3
#        ldob   1(r5),r3
!       ldos    sg_scnt(r6),r3          # r3 = segment count
        lda     sghdrsiz+sg_addr(r6),r4 # r4 = address of 1st sg element
        st      r4,xlisglpt(g7)         # set SGL segment addr
        stob    r3,xlisglnum(g7)        # set number of SGL elements
        st      r5,xlicdbptr(g7)        # set CDB for command
#
        ldob    irp_Tattr_TTC(g2),r6    # r6 = task type code
        ldob    irp_Tattr_DTA(g2),r7    # r7 = data transfer attributes
        stos    r6,xlifcflgs(g7)        # store FCAL flags
        stob    r7,xlidatadir(g7)       # store data direction
#
# --- sent the task to the ISP layer
#
.entask_150:
        mov     g1,r9                   # r9 = ILT at 2nd level
        lda     ILTBIAS(g1),g1          # advance to level 3 ILT
        st      g7,oil3_ptr(g1)         # Save pointer to XLI struct
#
        call    ISP$initiate_io         # Send command to target
        cmpobne.t ecok,g0,.entask_900   # Jif if return value is not EC_OK
#
# --- determine if there is another task to start
#
        ld      il_fthd(r9),g1          # g1 = next task on work queue
        ldob    irp_Tattr_TTC(g2),r6    # r6 = task type code
        cmpobe.t 0,g1,.entask_1000      # Jif no more tasks on work queue
        cmpobne.f ttc_simple,r6,.entask_900 # Jif previous task not simple
                                            #  task
        ldob    oil2_TTC(g1),r6         # r6 = next task's task type code
        ldob    oil2_tstate(g1),r5      # r5 = next task's task state code
        cmpobne.f ttc_simple,r6,.entask_900 # Jif next task not simple task
        cmpobe.t oil2_ts_dorm,r5,.entask_1000 # Jif next task in dormant state
#
.entask_900:
        mov     0,g1                    # g1 = 0 indicating not to process
                                        #  next task on work queue
.entask_1000:
        mov     r13,g7                  # restore g7
        mov     r14,g2                  # restore g2
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: apl$reschedule_task
#
#  PURPOSE:
#       Reschedules a task.
#
#  DESCRIPTION:
#       Places a task into the dormant state.
#
#  CALLING SEQUENCE:
#       call    apl$reschedule_task
#
#  INPUT:
#       g1 = ILT of task at the oil2 nest level
#       g4 = CIMT address
#       g6 = TLMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
apl$reschedule_task:
        ld      oil2_ismt(g1),g7        # g7 = address of ISMT
#
        ldconst oil2_ts_dorm,r4         # r4 = dormant task state
        lda     .Ts_wait2activate,r5    # r5 = event handler tbl ptr
        stob    r4,oil2_tstate(g1)      # save new state
        st      r5,oil2_ehand(g1)       # save event handler tbl address
#
        ldos    tlm_enblcnt(g6),r8      # r8 = enabled task counter
        cmpobe.f 0,r8,.rstsk_100        # Jif counter already 0
        subo    1,r8,r8                 # dec. counter
        stos    r8,tlm_enblcnt(g6)      # save updated counter
#
.rstsk_100:
        ldob    ici_state(g4),r4         # get current fcal state
        ldob    tlm_state(g6),r5        # r5 = TLMT state
#
        cmpobne.t Cs_online,r4,.rstsk_1000 # Jif fcal not online
        cmpobne.t tlmstate_active,r5,.rstsk_1000 # Jif device not active
#
        call    apl$chknextask          # determine if a task can be enabled
#
.rstsk_1000:
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: apl$chknextask
#
#  PURPOSE:
#       Checks TLMT to see if the next task on the working
#       queue needs to be enabled and if so enables the task
#       and calls the handler routine to begin processing the
#       task. Also, if the next task needs to be enabled, also
#       checks subsequent tasks to see if it's appropriate to
#       enable and begin processing these tasks.
#
#  DESCRIPTION:
#       Checks the next task on the working queue (if one is
#       present). If the task is enabled already, this routine
#       simply returns to the caller. If the task is dormant,
#       enables the task and sets up to begin processing the
#       task. In this case, this routine checks the subsequent
#       task on the working queue (if there are any) and enables
#       and begins processing any more tasks if appropriate.
#
#  CALLING SEQUENCE:
#       call    mag$chknextask
#
#  INPUT:
#       g4 = CIMT address
#       g6 = assoc. TLMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#      None.
#
#******************************************************************************
#
# void APL_StartIO(g4=void *, g6=void *);
        .globl  APL_StartIO
APL_StartIO:
#
apl$chknextask:
        mov     g1,r8                   # save g1
        mov     g5,r9                   # save g5
#
? # crash - cqt# 24597 - 2008-12-22 -- FE TLMT - ld 24+g6,r15 with caffcaff
? # Happened on other 7000 controller on 2010-06-12 about time of magdrvr.as:13892.
        ld      tlm_whead(g6),r15       # r15 = next task on work queue
        cmpobe.f 0,r15,.chkt1000_a      # Jif no tasks on working queue
#
        ldob    oil2_tstate(r15),r13    # r13 = task state code
        cmpobne.t oil2_ts_dorm,r13,.chkt100_a # Jif task not dormant
#
.chkt10_a:
        mov     r15,g1                  # g1 = pri. ILT at oil2 nest level
        ld      tlm_tmt(g6),g5          # g5 = assoc. TMT address
#
.chkt30_a:
        ldob    tlm_maxQ(g6),r4         # r4 = max queue depth
        ldos    tlm_enblcnt(g6),r3      # r3 = number of enabled tasks
        cmpobe.f 0,r4,.chkt40           # Jif no queue depth
        cmpobe  r3,r4,.chkt1000_a       # JIf queue already full
#
.chkt40:
        call    apl$enable_task         # enable next task on work queue
        cmpobne.f 0,g1,.chkt30_a        # Jif more tasks to enable
        b       .chkt1000_a             # and we're out of here
#
.chkt100_a:
        ldos    tlm_enblcnt(g6),r3      # r3 = number of enabled tasks
        ldob    tlm_maxQ(g6),r4         # r4 = max queue depth
        cmpobe.f 0,r3,.chkt150          # Jif no queue depth
        cmpobe  r3,r4,.chkt1000_a       # JIf queue already full
#
.chkt150:
        ld      tlm_wtail(g6),r12       # r12 = last task on work queue
        ldob    oil2_tstate(r12),r10    # r10 = task state code
        cmpobe.t oil2_ts_enbl,r10,.chkt1000_a # Jif last task enabled
#
.chkt190:
        ld      il_fthd(r15),r15        # r15 = next ILT on work queue
        cmpobe.f 0,r15,.chkt1000_a      # Jif no more to check
#
        ldob    oil2_tstate(r15),r13    # r13 = task state code
        cmpobne.t oil2_ts_dorm,r13,.chkt200 # Jif task not dormant
#
        ldob    oil2_TTC(r15),r12       # r12 = task type code
        cmpobe.f ttc_hoq,r12,.chkt10_a  # Jif head-of-queue type task
#
        cmpobe.f ttc_ordered,r12,.chkt1000_a # Jif ordered type task
#
        ld      il_bthd(r15),r11        # r11 = previous task ILT
        ldob    oil2_TTC(r11),r10       # r10 = previous task's type code
        cmpobe.t ttc_simple,r10,.chkt10_a # Jif previous task is simple task
        b       .chkt1000_a             # We're done if previous task is not
                                        #  simple
.chkt200:
        cmpobe.t oil2_ts_enbl,r13,.chkt190 # Jif task enabled to check next
                                        #  task on work queue
.chkt1000_a:
        mov     r8,g1                   # restore g1
        mov     r9,g5                   # restore g5
        ret
#
.endif  # INITIATOR
#
#******************************************************************************
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
