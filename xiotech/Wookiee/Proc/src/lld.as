# $Id: lld.as 159862 2012-09-19 20:49:57Z marshall_midden $
#
.if     MAG2MAG
#******************************************************************************
#
#  NAME: lld.as
#
#  PURPOSE:
#       This module contains the logic for the link-level driver used for
#       MAGNITUDE to MAGNITUDE features. The link-level driver interfaces
#       with the initiator driver and channel driver modules on the
#       interface side and the data-link manager on the MAGNITUDE side.
#       It's main function is to manage sessions with other MAGNITUDEs
#       as well as with foreign target devices.
#
#  FUNCTIONS:
#       LLD$init        - Link-level driver initialization
#       LLD$online      - Initiator interface online notification
#       LLD$offline     - Initiator interface offline notification
#       LLD$target_ID   - Target identification notification
#       LLD$target_GONE - Target disappeared notification
#       LLD$pre_target_gone - Pre target gone check routine
#
#       This module employs the following processes:
#
#       lld$exec       - Channel driver executive
#       lld$srpx       - SRP handler process
#       lld$retryio    - Retry I/O operations process (1 copy)
#
#  Copyright (c) 1998 - 2010 Xiotech Corporation. All rights reserved.
#
#******************************************************************************
#
# --- local equates -----------------------------------------------------------
#
        .set    retryio_to,1000         # retry I/O operation request task
                                        #  timeout period in msec.
#
# --- global function declarations
#
        .globl  LLD$init                # Link-level driver initialization
        .globl  LLD$online              # Initiator interface online notification
        .globl  LLD$offline             # Initiator interface offline
                                        #  notification
        .globl  LLD$target_ID           # Target identification notification
        .globl  LLD$target_GONE         # Target disappeared notification
        .globl  LLD$pre_target_gone     # pre target gone check routine
        .globl  LLD$cmdtbl1a
        .globl  LLD_d$event_tbl
        .globl  lld$add_ilmt
#
# --- cdriver resident routines ----------------------------------------------
#
        .globl  C$findTarget            # Find target for specified virtual port
#
# --- global usage data definitions
#
#
        .globl  I_noass                 # no imt/ltmt association
        .data
#
        .align  4                       # align just in case
#
# --- local usage data definitions
#
LLD_exec_pcb:
        .word   0                       # lld$exec process pcb address
#
# --- SRP queue definitions -------------------------------------- *****
#
        .globl  lld_srp_qu
lld_srp_qu:
        .word   0                       # queue head pointer        <w>
        .word   0                       # queue tail pointer        <w>
        .word   0                       # queue count               <w>
        .word   0                       # associated pcb            <w>
#                                                                  *****
#
# --- I/O operation retry queue data structure
#
        .globl  lld_rtyio_qu
lld_rtyio_qu:
        .word   0                       # queue head pointer        <w>
        .word   0                       # queue tail pointer        <w>
#                                                                  *****
# --- lld error anchors
#
lld_inv_ILT:
        .word   0                       # invalid ILT received
#
#
#lld_abt_cnt:
#        .word   0                       # a,b,e,f
#        .word   0                       # g,h,j,k
#        .word   0                       # l,z

#
lld_reg_trace_r4:
        .word   0                       # r4
        .word   0                       # r5
        .word   0                       # r6
        .word   0                       # r7
lld_reg_trace_r8:
        .word   0                       # r8
        .word   0                       # r9
        .word   0                       # r10
        .word   0                       # r11
lld_reg_trace_r12:
        .word   0                       # r12
        .word   0                       # r13
        .word   0                       # r14
        .word   0                       # r15

lld_reg_trace_g0:
        .word   0                       # g0
        .word   0                       # g1
        .word   0                       # g2
        .word   0                       # g3
lld_reg_trace_g4:
        .word   0                       # g4
        .word   0                       # g5
        .word   0                       # g6
        .word   0                       # g7
lld_reg_trace_g8:
        .word   0                       # g8
        .word   0                       # g9
        .word   0                       # g10
        .word   0                       # g11
#
# --- beginning of code -------------------------------------------------------
#
        .text
#
#******************************************************************************
#
# ________________________ EXTERNAL ROUTINES __________________________________
#
#******************************************************************************
#
#  NAME: LLD$init
#
#  PURPOSE:
#       Initializes the link-level driver environment.
#
#  DESCRIPTION:
#       The executive process is established and made ready
#       for execution.
#
#  CALLING SEQUENCE:
#       call    LLD$init
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
#******************************************************************************
#
LLD$init:
#
# --- Clear out local data structures
#
        ldconst 0,g13                   # establish tasks with g13 = 0
        movq    0,r4
        stq     r4,lld_srp_qu           # clear SRP queue
        stl     r4,lld_rtyio_qu         # clear queue pointers
#
# --- Establish link-level driver executive process
#
        movl    g0,r14                  # save g0-g1
        lda     lld$exec,g0             # establish executive process
        lda     LLDEXECPRI,g1           # set priority
c       CT_fork_tmp = (ulong)"lld$exec";
        call    K$fork
        st      g0,LLD_exec_pcb         # save lld$exec PCB address
#
# --- Establish LLD SRP executive process
#
        lda     lld$srpx,g0             # Establish SRP handler process
        ldconst LLDSEXECPRI,g1          # set priority
c       CT_fork_tmp = (ulong)"lld$srpx";
        call    K$fork
        st      g0,lld_srp_qu+qu_pcb    # Save PCB
#
# --- Establish LLD I/O operation retry process
#
        lda     lld$retryio,g0          # Establish I/O operation retry process
        ldconst LLDRTYIOPRI,g1          # set priority
c       CT_fork_tmp = (ulong)"lld$retryio";
        call    K$fork
#
        movl    r14,g0                  # restore g0-g1
        ret
#
#******************************************************************************
#
#  NAME: LLD$online
#
#  PURPOSE:
#       Processes the initiator online and operational event.
#
#  DESCRIPTION:
#       Places initiator operational if previously not operational.
#       Starts the process of establishing links to MAGNITUDES if
#       any MAGNITUDES were identified but have not had links
#       established.
#
#  CALLING SEQUENCE:
#       call    LLD$online
#
#  INPUT:
#       g0 = chip instance
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
# void LLD_Online(UINT8 );
    .globl LLD_Online
LLD_Online:
LLD$online:
        ldconst ICIMTMAX,r4
        cmpoble.f r4,g0,.online_1000    # Jif invalid interface #
        lda     cimtDir,r11             # r11 = CIMT directory pointer
        shlo    2,g0,r4                 # chip instance * 4
        addo    r4,r11,r11
        ld      (r11),r10               # r10 = CIMT address
        cmpobe.f 0,r10,.online_1000     # Jif no CIMT defined
        ldconst cis_on,r4               # r4 = initiator online state code
        ldob    ci_istate(r10),r5       # r5 = current initiator state
        cmpobe.f r4,r5,.online_1000     # Jif initiator already online
        stob    r4,ci_istate(r10)       # set initiator state to online
.if 1
        ldconst MAXLID,r4               # r4 = Current LID
        ld      I_CIMT_dir[g0*4],r10    # r10 = ICIMT
#
.ifdef HISTORY_KEEP
c CT_HISTORY_OFF();
.endif  # HISTORY_KEEP
.online_300:
        subo    1,r4,r4                 # decrement retry count
#
        ld      ici_tmdir(r10)[r4*4],r9 # r9 = TMT address from cimt lid dir
        cmpobe.f 0,r9,.online_500       # Jif TMT is NULL to next lid
#
        ldob    tm_chipID(r9),r5        # r5 = chip ID
        ldob    ici_chpid(r10),r6
        cmpobne  r6,r5,.online_350      # Jif TMT in wrong ICIMT
#
        ldos    tm_lid(r9),r5           # r5 = LID
        cmpobe  r4,r5,.online_400       # Jif TMT lid matches
#
.online_350:
#
# --- TMT belonging to other port - this is bad!!!
#
        ldconst 0,r5                    # r5 = 0
        st      r5,ici_tmdir(r10)[r4*4] # clear TMT in CIMT dir
        b       .online_500             # Jif next lid
#
.online_400:
        ldob    tm_state(r9),r5         # r5 = current TMT state
        cmpobne.f tmstate_active,r5,.online_500 # Jif TMT not active
#
        ld      tm_ltmt(r9),g4          # g4 = LTMT
        cmpobe  0,g4,.online_500        # Jif LTMT is NULL
#
        ldob    ltmt_lst(g4),r5         # r5 = link state code
        cmpobne.t ltmt_lst_id,r5,.online_500 # Jif not in target identified state
#
        ld      ltmt_ehand(g4),r6       # r6 = LTMT event handler table
        ld      ltmt_eh_online(r6),r7   # r7 = LTMT online event handler routine
        callx   (r7)                    # and call online event handler routine
#
.online_500:
        cmpobne.t 0,r4,.online_300      # Jif more TMTs to process
.ifdef HISTORY_KEEP
c CT_HISTORY_ON();
.endif  # HISTORY_KEEP
.else   # 1
        ld      ci_ltmthd(r10),r11      # r11 = first LTMT on list
        cmpobe.f 0,r11,.online_1000     # Jif no LTMTs on list
        mov     g4,r12                  # save g4
.online_300:
        mov     r11,g4                  # g4 = LTMT to notify of online event
#        ld      ltmt_link(g4),r11       # r11 = next LTMT on list
        ld      ltmt_ehand(g4),r9       # r9 = LTMT event handler table
        ld      ltmt_eh_online(r9),r8   # r8 = LTMT online event handler routine
        callx   (r8)                    # and call online event handler routine
#
#******************************************************************************
#
# --- Interface to Online event handler routine
#
#   INPUT:
#       g4 = assoc. LTMT address
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       None.
#
#******************************************************************************
#
        ldconst cis_on,r4               # r4 = initiator online state code
        ldob    ci_istate(r10),r5       # r5 = current initiator state
        cmpobne.f r4,r5,.online_1000    # Jif initiator not online

        ld      ltmt_link(r11),r11      # r11 = next LTMT on list
        cmpobne.t 0,r11,.online_300     # Jif more LTMTs to notify of online
                                        #  event
        mov     r12,g4                  # restore g4
.endif  # 1
.online_1000:
        ret
#
#******************************************************************************
#
#  NAME: LLD$offline
#
#  PURPOSE:
#       Processes the initiator offline event.
#
#  DESCRIPTION:
#       Places the initiator in an offline and not operational
#       state.
#
#  CALLING SEQUENCE:
#       call    LLD$offline
#
#  INPUT:
#       g0 = chip instance
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
# void LLD_Offline(UINT8 );
    .globl LLD_Offline
LLD_Offline:
LLD$offline:
        ldconst ICIMTMAX,r4
        cmpoble.f r4,g0,.offline_1000   # Jif invalid interface #
        lda     cimtDir,r11             # r11 = CIMT directory pointer
        shlo    2,g0,r4                 # chip instance * 4
        addo    r4,r11,r11
        ld      (r11),r10               # r10 = CIMT address
        cmpobe.f 0,r10,.offline_1000    # Jif no CIMT defined
        ldconst cis_off,r4              # r4 = initiator offline state code
        ldob    ci_istate(r10),r5       # r5 = current initiator state
        cmpobe.f r4,r5,.offline_1000    # Jif initiator already offline
        stob    r4,ci_istate(r10)       # set initiator state to offline
        ld      ci_ltmthd(r10),r11      # r11 = first LTMT on list
        cmpobe.f 0,r11,.offline_1000    # Jif no LTMTs on list
        mov     g4,r12                  # save g4
.offline_300:
        mov     r11,g4                  # g4 = LTMT to notify of offline event
        ld      ltmt_link(g4),r11       # r11 = next LTMT on list
        ld      ltmt_ehand(g4),r9       # r9 = LTMT event handler table
        ld      ltmt_eh_offline(r9),r8  # r8 = LTMT offline event handler routine
        callx   (r8)                    # and call offline event handler routine
#
#******************************************************************************
#
# --- Interface to Offline event handler routine
#
#   INPUT:
#       g4 = assoc. LTMT address
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       None.
#
#******************************************************************************
#
        cmpobne.t 0,r11,.offline_300    # Jif more LTMTs to notify of offline
                                        #  event
        mov     r12,g4                  # restore g4
.offline_1000:
        ret
#
#******************************************************************************
#
#  NAME: LLD$target_ID
#
#  PURPOSE:
#       Processes the target identified notification event.
#
#  DESCRIPTION:
#       Determines whether the target is another MAGNITUDE or a
#       foreign target and if the link-level driver wants to keep
#       track of the target. If not, it simply returns to the caller.
#       If it does want to keep track of the target, it allocates
#       a LTMT and places the target into service.
#
#  CALLING SEQUENCE:
#       call    LLD$target_ID
#
#  INPUT:
#       g0 = chip instance
#       g5 = address of Target Management Table (TMT)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
        .data
targID_MAG1:
        .byte   ltmt_ty_MAG             # ltmt_type code
        .byte   ltmt_lst_id             # ltmt_lst code
        .byte   ltmt_dst_null           # ltmt_dlmst code
        .byte   0                       # unused
targID_FT1:
        .byte   ltmt_ty_FT              # ltmt_type code
        .byte   ltmt_lst_id             # ltmt_lst code
        .byte   ltmt_dst_null           # ltmt_dlmst code
        .byte   0                       # unused
#
        .text
#
# void LLD_TargetOnline(g5 = void *);
    .globl LLD_TargetOnline
LLD_TargetOnline:
        ldob    tm_chipID(g5),g0        # g0 = chip ID
LLD$target_ID:
#
# --- Check if LTMT already defined for this target
#
        ld      tm_ltmt(g5),r4          # r4 = assoc. LTMT from TMT
        cmpobne.f 0,r4,.targetID_1000   # Jif LTMT already defined
        ldconst ICIMTMAX,r4
        cmpoble.f r4,g0,.targetID_1000  # Jif invalid interface #
        lda     cimtDir,r11             # r11 = CIMT directory pointer
        shlo    2,g0,r4                 # chip instance * 4
        addo    r4,r11,r11
        ld      (r11),r10               # r10 = CIMT address
        cmpobe.f 0,r10,.targetID_1000   # Jif no CIMT defined
#
# --- Determine whether this target is a MAGNITUDE
#
        movl    g6,r12                  # save g6-g7
        ldl     tm_N_name(g5),g6        # g6-g7 = target node name
c       r8 = M_chk4XIO(*(UINT64*)&g6);  # is this a XIOtech Controller ???
        movl    r12,g6                  # restore g6-g7
        cmpobe.f 0,r8,.targetID_300     # Jif not a XIOtech Controller
#
# --- Target IS a MAGNITUDE -----------------------------------------------------
#
# --- Allocate a LTMT and initialize it
#
        mov     g4,r12                  # save g4
c       g4 = get_ltmt();                # allocate an LTMT for this target
.ifdef M4_DEBUG_LTMT
c fprintf(stderr, "%s%s:%u get_ltmt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g4);
.endif # M4_DEBUG_LTMT
        st      g4,tm_ltmt(g5)          # save LTMT in TMT
        ld      targID_MAG1,r6          # load type, link state, DLM state codes
        st      r6,ltmt_type(g4)        # save type, link state, DLM state codes
        st      r10,ltmt_cimt(g4)       # save assoc. CIMT address
        st      g5,ltmt_tmt(g4)         # save assoc. TMT address
        lda     ltmt_etbl1a,r6          # r6 = LTMT event handler table
        st      r6,ltmt_ehand(g4)       # save event handler table
        ld      ci_ltmttl(r10),r7       # r7 = LTMT list tail pointer
        cmpobne.t 0,r7,.targetID_120    # Jif list not empty
        st      g4,ci_ltmthd(r10)       # put LTMT as new head pointer
        b       .targetID_130
#
.targetID_120:
        st      g4,ltmt_link(r7)        # link new LTMT onto end of list
.targetID_130:
        st      g4,ci_ltmttl(r10)       # save as new list tail pointer
        ldob    ci_istate(r10),r7       # r7 = initiator state code
        cmpobne.t cis_on,r7,.targetID_150 # Jif initiator not online
#
# --- Call online event handler routine to start link establish process
#
        ld      ltmt_eh_online(r6),r7   # r7 = online event handler routine
        callx   (r7)                    # and call online handler routine
.targetID_150:
        mov     r12,g4                  # restore g4
        b       .targetID_1000
#
# --- Target is NOT a MAGNITUDE -------------------------------------------------
#
.targetID_300:
#
# --- Allocate a LTMT and initialize it
#
        mov     g4,r12                  # save g4
c       g4 = get_ltmt();                # allocate an LTMT for this target
.ifdef M4_DEBUG_LTMT
c fprintf(stderr, "%s%s:%u get_ltmt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g4);
.endif # M4_DEBUG_LTMT
        st      g4,tm_ltmt(g5)          # save LTMT in TMT
        ld      targID_FT1,r6           # load type, link state, DLM state codes
        st      r6,ltmt_type(g4)        # save type, link state, DLM state codes
        st      r10,ltmt_cimt(g4)       # save assoc. CIMT address
        st      g5,ltmt_tmt(g4)         # save assoc. TMT address
        lda     ltmt_etbl2a,r6          # r6 = LTMT event handler table
        st      r6,ltmt_ehand(g4)       # save event handler table
        ld      ci_ltmttl(r10),r7       # r7 = LTMT list tail pointer
        cmpobne.t 0,r7,.targetID_320    # Jif list not empty
        st      g4,ci_ltmthd(r10)       # put LTMT as new head pointer
        b       .targetID_330
#
.targetID_320:
        st      g4,ltmt_link(r7)        # link new LTMT onto end of list
.targetID_330:
        st      g4,ci_ltmttl(r10)       # save as new list tail pointer
        ldob    ci_istate(r10),r7       # r7 = initiator state code
        cmpobne.t cis_on,r7,.targetID_350 # Jif initiator not online
#
# --- Call online event handler routine to start link establish process
#
        ld      ltmt_eh_online(r6),r7   # r7 = online event handler routine
        callx   (r7)                    # and call online handler routine
.targetID_350:
        mov     r12,g4                  # restore g4
.targetID_1000:
        ret
#
#******************************************************************************
#
#  NAME: LLD$target_GONE
#
#  PURPOSE:
#       Processes the target disappeared notification event.
#
#  DESCRIPTION:
#       If the target does not have an associated LTMT defined,
#       this routine merely returns to the caller. If an LTMT is
#       defined, this routine starts the process of terminating
#       the association with the target.
#
#  CALLING SEQUENCE:
#       call    LLD$target_GONE
#
#  INPUT:
#       g5 = address of Target Management Table (TMT)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
# void LLD_TargetOffline(g5 = TMT *);
    .globl LLD_TargetOffline
LLD_TargetOffline:
        ldob    tm_chipID(g5),g0        # g0 = chip ID
LLD$target_GONE:
        mov     g4,r12                  # save g4
        ldconst 0,r3
#
# --- Check if LTMT already defined for this target
#
        ld      tm_ltmt(g5),g4          # g4 = assoc. LTMT from TMT
        cmpobe.f 0,g4,.targetGONE_1000  # Jif LTMT not defined
#
.if     debug_tracestop2
#
        ldob    ltmt_type(g4),r4        # r4 = target type code
        cmpobne.f ltmt_ty_MAG,r4,.trcstop2_1000 # Jif not MAG Link type target
        ld      ltmt_sn(g4),r4          # r4 = MAG serial #
        ld      K_ficb,r5               # r5 = FICB
        ld      fi_cserial(r5),r5       # r5 = Controller serial number
        cmpobne.t r4,r5,.trcstop2_1000  # Jif not assoc. with my MAG
        ld      tm_icimt(g5),r4         # r4 = assoc. ICIMT address
        stos    r3,ici_tflg(r4)         # turn initiator trace flags off
.trcstop2_1000:
#
.endif  # debug_tracestop2
#
        st      r3,tm_ltmt(g5)          # clear LTMT from TMT
        ld      ltmt_ehand(g4),r9       # r9 = LTMT event handler table
        ld      ltmt_eh_tgone(r9),r8    # r8 = LTMT target gone event
                                        #  handler routine
        callx   (r8)                    # and call target gone event
                                        #  handler routine
#
#******************************************************************************
#
# --- Interface to Target Gone event handler routine
#
#   INPUT:
#       g4 = assoc. LTMT address
#       g5 = assoc. TMT address
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       None.
#
#******************************************************************************
#
.targetGONE_1000:
        mov     r12,g4                  # restore g4
        ret
#
#******************************************************************************
#
#  NAME: LLD$pre_target_gone
#
#  PURPOSE:
#       Checks if it's OK to terminate a target and returns the
#       results to the caller.
#
#  DESCRIPTION:
#       Checks if a LTMT is associated with the specified TMT and if
#       not returns TRUE status. If so, checks if the target is a
#       MAGNITUDE link and if not returns TRUE status. If so, checks
#       if any incoming datagram messages are in progress and if not
#       returns TRUE status. If there are any incoming datagram
#       messages in progress, returns FALSE indicating it is not OK
#       to terminate the specified target.
#
#  CALLING SEQUENCE:
#       call    LLD$pre_target_gone
#
#  INPUT:
#    if g5 <> 0
#       g5 = address of Target Management Table (TMT)
#  else g4 = address of LTMT to process
#
#  OUTPUT:
#       g0 = TRUE if OK to terminate target
#       g0 = FALSE if not OK to terminate target
#
#  REGS DESTROYED:
#       Reg. g0 destroyed.
#
#******************************************************************************
#
# UINT32 LLD_CheckTargetDel(g5=void *);
    .globl LLD_CheckTargetDel
LLD_CheckTargetDel:
        ldconst 0,g4                    # g4 = LTMT preload
#
LLD$pre_target_gone:
        ldconst TRUE,g0                 # preload return value
        mov     g4,r14                  # r14 = specified LTMT address if
                                        #       specified TMT = 0
        cmpobe  0,g5,.pregone_50        # Jif specified TMT = 0
? # crash - cqt# 24349 - 2008-10-28 -- FE TMT - g5 was valid address, but tmt free, r14=acedaced. DOES TMT NEED TO BE ZERO ON FREE?
        ld      tm_ltmt(g5),r14         # r14 = assoc. LTMT address
.pregone_50:
        cmpobe.f 0,r14,.pregone_1000    # Jif no LTMT assoc. with TMT

        ldob    ltmt_lst(r14),r4        # r4 = link state code
        cmpobe.t ltmt_lst_pest,r4,.pregone_1000 # Jif not a MAGNITUDE link

        ldconst ltmt_lst_term,r4        # r4 = new link state code
        stob    r4,ltmt_lst(r14)        # save new link state code

        ldob    ltmt_type(r14),r4       # r4 = target type code
        cmpobne.f ltmt_ty_MAG,r4,.pregone_1000 # Jif not a MAGNITUDE link

        ldconst ltmt_dgmax,r13          # r13 = max. # datagram messages
                                        #  supported
        ldconst 0,r4                    # r4 = datagram ID index
.pregone_100:
        ld      ltmt_tmsg(r14)[r4*4],r5 # r5 = possible LDG area
        cmpobe.t 0,r5,.pregone_200      # Jif no incoming datagram message
                                        #  in this slot
        ldconst FALSE,g0                # indicate not OK to blow away this
        b       .pregone_1000           # and return to caller
#
.pregone_200:
        addo    1,r4,r4                 # inc. datagram ID index
        cmpobl.t r4,r13,.pregone_100    # Jif more datagram slots to check
.pregone_1000:
        ret
#
#******************************************************************************
#
#  NAME: LLD$session_term
#
#  PURPOSE:
#       Processes the session termination notification event.
#
#  DESCRIPTION:
#       If the session does not have an associated LSMT defined,
#       this routine merely returns to the caller. If an LSMT is
#       defined, this routine starts the process of terminating
#       the association with the LSMT.
#
#  CALLING SEQUENCE:
#       call    LLD$session_term
#
#  INPUT:
#       g1 = address of session requestor ID (LSMT)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
LLD$session_term:

        movq    g0,r4                   # save g0-g3 in r4-r7
        movq    g4,r12                  # save g4-g7 in r12-r15
        cmpobe.f 0,g1,.ses_term_1000    # Jif LSMT not defined
        mov     g1,g3                   # g3 = LSMT

        ld      lsmt_ehand(g3),r9       # r9 = LSMT event handler table
#        cmpobe  0,r9,.ses_term_1000     # if no handler goto exit.
        ld      lsmt_eh_term(r9),r8     # r8 = session termination event handler
                                        #  routine
        callx   (r8)                    # call session termination event handler
                                        #  routine
                                        # input:
                                        #    g3 = assoc. LSMT address
.ses_term_1000:
        movq    r12,g4                  # restore g4-g7 from r12-r15
        movq    r4,g0                   # restore g0-g3 from r4-r7
        ret
#
#******************************************************************************
#
# ________________________________ PROCESSES __________________________________
#
#******************************************************************************
#
#******************************************************************************
#
#  NAME: lld$exec
#
#  PURPOSE:
#       Link-level driver executive process.
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
#       All registers can be destroyed.
#
#******************************************************************************
#
lld$exec:
#jt? why wait again? we just came out of an exchange
#jt        call    K$xchang                # go to sleep
#
# --- Allocate and initialize LTMTs
c       init_ltmt(8);                   # Initialize LTMTs - only 8 were used on 7000.
#
# --- Allocate and initialize LSMTs
#
c       init_lsmt(16);                  # Initialize LSMTs to 16 free.
#
        call    K$xchang                # go to sleep
#
# --- Send a link-lever driver operational VRP to the data-link manager
#     for each possible path (channel) that exists. Use the ispmap to
#     determine all the paths that exist.
#
        ld      ispmap,r4               # get ISP found device bitmap
#
.exec_05:                               # Start of loop
#
        scanbit r4,g0                   # get first found device (path)
        bno     .exec_10                # jif end of list
        clrbit  g0,r4,r4                # clear found device (path)
#
        call    lld$send_LDO            # send link-level driver operational
                                        #  VRP to data-link manager
        b       .exec_05                # loop for more paths

.exec_10:
#
## Send a link-level driver operational VRP to the data-link manager for ICL
## path also.
#
        ld       iclPortExists,r4       # Check whether ICL exists
        cmpobne  TRUE,r4,.exec_20       # Jif not exists
        ldconst  ICL_PORT,g0            # g0 = ICL port
.if ICL_DEBUG
c fprintf(stderr,"%s%s:%u <lld$exec> ICL -- Sending link level driver VRP. for ICL port=%u.\n", FEBEMESSAGE, __FILE__, __LINE__,(UINT32)g0);
.endif  # ICL_DEBUG
        call     lld$send_LDO
.exec_20:

#jt? why wait if nothing happens after this?
        ldob    MAGD$targetrdy,r4       # r4 = target ready signal
        cmpobne.t 0,r4,.exec_30         # Jif target ready for service
        lda     1000,g0                 # set up to wait before trying again
        call    K$twait                 # delay task
        b       .exec_20                # and try again until target is ready
#
.exec_30:
#*** FINISH
#
# --- Task initialization completed
#
#jt? what is this doing? Loops forever
.exec_9999:
c       TaskSetMyState(pcnrdy);         # Set to process not ready
        call    K$xchang                # go to sleep
        b       .exec_9999
#
#**********************************************************************
#
#  NAME: lld$retryio
#
#  PURPOSE:
#       To provide a means of retrying I/O requests requests periodically.
#
#  DESCRIPTION:
#       Takes any I/O request messages off the I/O operation retry
#       queue and calls the specified continuation routine after the
#       timeout period has expired. The I/O operation request messages
#       have been set up already and the specified continuation routine
#       handles setting up the registers and calling the appropriate
#       routine.
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
lld$retryio:
.retryio_10:
        ldconst retryio_to,g0           # g0 = time delay period (in msec.)
        call    K$twait                 # wait for time delay
        movl    0,r4                    # r4-r5 = 0
        ldl     lld_rtyio_qu,r6         # r6 = first I/O operation on queue
                                        # r7 = last I/O operation on queue
        stl     r4,lld_rtyio_qu         # clear queue pointers
.retryio_100:
        cmpobe.t 0,r6,.retryio_10       # Jif no I/O operations to retry
        mov     r6,g1                   # g1 = I/O operation ILT at providers'
                                        #  nest level
        ld      il_fthd(r6),r6          # r6 = next datagram message on list
        ld      il_cr(g1),r4            # r4 = requestor's continuation
                                        #  routine address
        movq    g4,r8                   # save g4-g7
        movq    g8,r12                  # save g8-g11
        callx   (r4)                    # and call requestor's continuation
                                        #  routine
        movq    r8,g4                   # restore g4-g7
        movq    r12,g8                  # restore g8-g11
        b       .retryio_100            # and check for more I/O operations
                                        #  to retry
#
#******************************************************************************
#
#  NAME: lld$srpx
#
#  PURPOSE:
#       Link-level driver SRP handler process.
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
#       All registers can be destroyed.
#
#******************************************************************************
#
.srpx_00:
        call    K$xchang                # go to sleep
#
# --- Set this process to not ready
#
lld$srpx:
        ld      lld_srp_qu+qu_pcb,r15   # r15 = my process PCB address
        ldconst pcnrdy,r4               # Set this process to not ready
        stob    r4,pc_stat(r15)
#
# --- Check for SRPs to process ----------------------------------------------
#
.srpx_20:
#
# --- Get next queued request
#
        lda     lld_srp_qu,r11          # Get SRP queue pointer
        ldq     qu_head(r11),r12        # r12 = queue head pointer
                                        # r13 = queue tail pointer
                                        # r14 = queue count
                                        # r15 = PCB address
        mov     r12,g1                  # g1 = ILT being processed at nest
                                        #  level #2
        cmpobe.f 0,r12,.srpx_00         # Jif none
#
# --- Remove this request from queue ----------------------------------
#
        ld      il_fthd(r12),r12        # r12 = next ILT on queue
        cmpo    0,r12                   # Check for queue now empty
        subo    1,r14,r14               # Adjust queue count
        sele    r13,r12,r13             # Set up queue tail
        stt     r12,qu_head(r11)        # Update queue head, tail and count
#
        ld      -ILTBIAS+otl1_srp(g1),g2 # g2 = SRP address
#
# --- Validate function code
#
!       ldob    sr_func(g2),r14         # r14 = function code
        ldconst srmagst,r4              # r4 = starting SRP function code
        ldconst srmagend,r5             # r5 = ending SRP function code
        cmpobg.f r4,r14,.srpx_980       # Jif invalid SRP function code
        cmpobg.f r14,r5,.srpx_980       # Jif invalid SRP function code
        subo    r4,r14,r4               # r4 = normalized SRP function code
        shlo    2,r4,r4                 # r4 = normalized SRP function code * 4
        lda     lld$srphand,r5          # r5 = SRP handler routine table
        addo    r4,r5,r5                # r5 = pointer to SRP handler routine
        ld      (r5),r5                 # r5 = SRP handler routine
        callx   (r5)                    # and call SRP handler routine
#
#*****************************************************************************
#
#       Interface to SRP handler routines
#
#  INPUT:
#       g1 = ILT associated with SRP at nest level #2
#       g2 = SRP address to process
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Reg. g0-g12, g14 can be destroyed.
#
#*****************************************************************************
#
        b       .srpx_20                # check if any more SRPs to process
#
# ----------------------------------------------------------------------------
#
# --- Set invalid function status in SRP and return
#
.srpx_980:
        ldconst srerr,g0                # g0 = invalid function SRP status
#
# --- Complete request
#
        lda     -ILTBIAS(g1),g1         # g1 = ILT/SRP at nest level #1
        ld      il_cr(g1),r3            # r3 = ILT completion handler routine
        callx   (r3)                    # call completion handler routine
        b       .srpx_20                # check if any more SRPs to process
#
#**********************************************************************
#
#  NAME: lld$srphand
#
#  PURPOSE:
#       Link-level Driver SRP request function handler routine table.
#
#  DESCRIPTION:
#       This table contains the SRP request handler routines serviced by
#       the link-level driver.
#
#  CALLING SEQUENCE:
#       call    xxxxxxxx
#
#  INPUT:
#       g1 = ILT associated with SRP at nest level #2
#       g2 = SRP address to process
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Reg. g0-g12, g14 can be destroyed.
#
#**********************************************************************
#
        .data
lld$srphand:
        .word   lld$OML                 # 0x40 sropml - OPEN session to MAGNITUDE
        .word   lld$CML                 # 0x41 srclml - CLOSE session to MAGNITUDE
        .word   lld$OFT                 # 0x42 sropft - OPEN session to foreign target
        .word   lld$CFT                 # 0x43 srclft - CLOSE session to foreign target
        .word   lld$LRP                 # 0x44 srxlrp - execute VRP to device
        .word   lld$MSG                 # 0x45 srsmsg - send message to MAGNITUDE
        .word   lld$FDD                 # 0x46 srgfdd - get foreign target device database
        .word   I$SIF                   # 0x47 sriflgs - set initiator flags
        .word   MAGD$stf                # 0x48 srgflgs - Set target flags
        .word   lld$OFT                 # 0x49 sropft_GT2TB - OPEN session to foreign target GT2TB
#
        .text
#
#**********************************************************************
#
#  NAME: lld$OML
#
#  PURPOSE:
#       Processes an OPEN session to MAGNITUDE SRP request.
#
#  CALLING SEQUENCE:
#       call    lld$OML
#
#  INPUT:
#       g1 = ILT associated with SRP at nest level #2
#       g2 = SRP address to process
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Reg. g0-g12, g14 can be destroyed.
#
#**********************************************************************
#
lld$OML:
!       ld      sr_oml_ospb(g2),r15     # r15 = MLOSPB address
!       ld      sr_oml_lldid(g2),g4     # g4 = link-level driver session ID (LTMT address)
? # crash - cqt# 24405 - 2008-12-02 -- FE LTMT - ld 16+g4,r6 with fabafaba -- workaround?
        ld      ltmt_dlmid(g4),r6       # r6 = DLM session ID from LTMT
        cmpobne.t 0,r6,.lldOML_100      # Jif DLM Session ID established
#
# --- Return error since something's wrong.
#
.lldOML_40:
        ldconst srerr,g0                # g0 = SRP return status
        lda     -ILTBIAS(g1),g1         # g1 = ILT/SRP at nest level #1
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT/SRP completion handler routine
        b       .lldOML_1000            # and we're out of here!
#
# --- Check if the Vdisk (LUN) is within range...
#
.lldOML_100:
        ldconst 0,g7                    # g7 - lirp1_flags <=> LSMT not allocated.
!       ld      sr_oml_dlmid(g2),r6     # r6 = requestor's session ID
!       ldos    sr_oml_vid(g2),g0       # g0 = vdisk # (LUN)
        ldconst MAXLUN,r12              # r12 = max. valid LUN #
        cmpobge.f g0,r12,.lldOML_40     # Jif invalid LUN # specified
#
# --- search the LSMT list for a match. If there is a match, an LSMT doesn't
#     have to be allocated
#
        ld      ltmt_seshead(g4),g3     # g3 = lsmt
        cmpobe.t 0,g3,.lldOML_200       # Jif nothing on the queue

.lldOML_110:
? # crash - cqt# 24599 - 2008-11-18 -- BE LTMT - ldob lsmt_lun(g3),r4 - g3=0xfabafaba - workaround?
        ldos    lsmt_lun(g3),r4         # r4 = lun
        ld      lsmt_dlmid(g3),r5       # r5 = DLM ID
        cmpobne.f r4,g0,.lldOML_120     # Jif lun's don't match

        cmpobe.f r5,r6,.lldOML_300      # Jif this is the matching LSMT

.lldOML_120:
        ld      lsmt_link(g3),g3        # g3 = next lsmt on queue
        cmpobne.t 0,g3,.lldOML_110      # Jif nothing on the queue
#
# --- There isn't a LSMT that matched both LUN # and DLM ID. Check
#       for any LSMTs that match LUN # and have lsmt_dlmid cleared
#       indicating the LSMT is not in use.
#
        ld      ltmt_seshead(g4),g3     # g3 = lsmt

.lldOML_130:
        ldos    lsmt_lun(g3),r4         # r4 = lun
        ld      lsmt_dlmid(g3),r5       # r5 = DLM ID
        cmpobne.f r4,g0,.lldOML_140     # Jif lun's don't match

        cmpobe.f 0,r5,.lldOML_250       # Jif this LSMT not in use

.lldOML_140:
        ld      lsmt_link(g3),g3        # g3 = next lsmt on queue
        cmpobne.t 0,g3,.lldOML_130      # Jif nothing on the queue
#
# --- There wasn't a LSMT that matched, so allocate one and link it into
#     the LTMT.
#
.lldOML_200:
c       g3 = get_lsmt();                # allocate a LSMT for linked device session
        st 0,lsmt_link(g3)              #clear forward link
        ldconst 1,g7                    # g7 - lirp1_flags <=> LSMT allocated.
.ifdef M4_DEBUG_LSMT
c fprintf(stderr, "%s%s:%u get_lsmt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g3);
.endif # M4_DEBUG_LSMT
#
# --- Put LSMT onto LTMT session list
#
        ld      ltmt_sestail(g4),r4     # r4 = last LSMT on session list
        cmpobne.t 0,r4,.lldOML_230      # Jif list not empty
        st      g3,ltmt_seshead(g4)     # save LSMT as new head member
        b       .lldOML_240
#
.lldOML_230:
        st      g3,lsmt_link(r4)        # link LSMT onto end of list
.lldOML_240:
        st      g3,ltmt_sestail(g4)     # put new LSMT as new tail member
#
# --- fill in the LSMT with the SRP information
#
.lldOML_250:
!       ld      sr_oml_dlmid(g2),r4     # r4 = requestor's session ID
        st      r4,lsmt_dlmid(g3)       # save requestor's session ID in LSMT
!       ldos    sr_oml_vid(g2),g0       # g0 = LUN # to open session for
        stos    g0,lsmt_lun(g3)         # save LUN # in LSMT
        st      g4,lsmt_ltmt(g3)        # save assoc. LTMT address in LSMT
        lda     lsmt_etbl2a,r4          # r4 = session event handler table
        st      r4,lsmt_ehand(g3)       # save event handler table in LSMT
#
# --- Set up to send open request to initiator driver
#
.lldOML_300:
        mov     g2,g5                   # Move srp to lld$sendopen input register
        lda     OMLopn_cr,g2            # g2 = open ILT completion handler routine
        lda     LLD$session_term,g6     # g6 = session termination routine
        call    lld$sendopen            # open session for Foreign Target linked device session
                                        # Input:
                                        #  g0 = LUN # to open session to
                                        #  g1 = ILT at 2nd lvl
                                        #  g2 = ILT completion handler routine address
                                        #  g3 = LSMT associated with session being opened
                                        #  g4 = assoc. LTMT address
                                        #  g5 = SRP
                                        #  g6 = session termination routine adr
                                        #  g7 = flags for lirp1 ILT
.lldOML_1000:
        ret
#
#******************************************************************************
#
#  NAME: OMLopn_cr
#
#  PURPOSE:
#       Processes completion of the open IRP request for a MAGNITUDE
#       linked device session.
#
#  CALLING SEQUENCE:
#       call    OMLopn_cr
#
#   INPUT:
#       g1 = ILT/IRP address of OPEN at nest level 1
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       None.
#
#******************************************************************************
#
OMLopn_cr:
        movq    g0,r12                  # save g0-g3
        movl    g4,r10                  # save g4-g5
        ld      lirp1_irp(g1),g0        # g0 = IRP for OPEN request
        ld      lirp1_lldid(g1),g5      # g5 = assoc. LSMT address
        ldob    lirp1_flags(g1),r7      # r7 = flags
        ld      lirp1_g0(g1),r9         # r9 = ILT' at 2nd lvl
        ld      lirp1_g1(g1),g2         # g2 = SRP
        ld      lsmt_ltmt(g5),g4        # g4 = LTMT

        ld      lsmt_ilt(g5),r4         # r4 = assoc. ILT/IRP from LSMT
        ldob    irp_cmplt(g0),r5        # r5 = OPEN request completion status
        ld      irp_Pro_ID(g0),r6       # r6 = initiator ID
#
# --- release the IRP and ILT
#
.ifdef M4_DEBUG_IRP
c CT_history_printf("%s%s:%u put_irp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif # M4_DEBUG_IRP
c       put_irp(g0);                    # release IRP
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
#
# --- determine if ILT" is registered in LSMT
#
        ldconst srabt,r8                # r10 = SRP return status
        cmpobne.t g1,r4,.OMLopncr_err_20   # Jif ILT registered in LSMT

#
# --- determine if LTMT within LSMT was freed inbetween...
#
        cmpobe.t 0,g4,.OMLopncr_err_20   # Jif LTMT is NULL
#
# --- ILT/IRP still registered in LSMT
#
        ldconst 0,r3
        st      r3,lsmt_ilt(g5)         # clear assoc. ILT field in LSMT
        cmpobe.t cmplt_success,r5,.OMLopncr_300 # Jif OPEN successful

        ldconst srerr,r8                # r8 = SRP return status
#
# --- OPEN request failed
#
#
# --- the flags field is used to indicate if the LSMT was allocated
# --- when the open request was initiated. If an existing LSMT was used,
# --- we do not release that LSMT. If a new LSMT was allocated, only then
# --- we free the LSMT here. Fix for ME-357
#
        cmpobe.t 0,r7,.OMLopncr_err_20  # Jif flags is 0
        mov     g4,r10                  # save g4
        mov     g5,g3                   # g3 = LSMT
                                        # g4 = LTMT
        call    lld$rem_lsmt            # remove LSMT from LTMT list
.ifdef M4_DEBUG_LSMT
c fprintf(stderr, "%s%s:%u put_lsmt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g3);
.endif # M4_DEBUG_LSMT
c       put_lsmt(g3);                   # Deallocate LSMT
        mov     r10,g4                  # restore g4
.OMLopncr_err_20:
        lda     -ILTBIAS(r9),g1         # g1 = ILT/SRP at nest level #1
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        mov     r8,g0                   # g0 = SRP completion status
        callx   (r4)                    # call ILT/SRP completion handler routine
        b       .OMLopncr_1000          # return error back to requestor
#
# --- OPEN request succeeded
#
.OMLopncr_300:
        st      r6,lsmt_psid(g5)        # save initiator ID in LSMT
!       ld      sr_oml_ospb(g2),g3      # g3 = ospb base
!       st      g5,mlos_lldid(g3)       # save current lsmt in lldid
#
# --- allocate a SGL for the request and the response.
#
        ld      ltmt_seshead(g4),g5     # g5 = LSMT for lun 0
        mov     r9,g1                   # g1 = ILT' at sndg2 lvl
        st      g2,sndg2_srp(g1)        # save srp address
        ld      ltmt_cimt(g4),g3        # g3 = cimt


        lda     OMLdg_cr,r4             # r4 = completion routine
        ldconst dgrq_size+LLD0_rq_estvl_size,r5 # r5 = request length (real)
        addo    4-1,r5,r6               # round up to a word boundary
        andnot  4-1,r6,r6               # r6 = request length (adjusted)
        mov     r6,g0                   # g0 = request length
        st      r5,sndg2_reqlen(g1)     # save request length (real)

        ldconst dgrs_size+LLD0_rs_estvl_size,r7 # r7 = response length (real)
        addo    4-1,r7,r8               # round up to a word boundary
        andnot  4-1,r8,r8               # r8 = response length (adjusted)
        addo    r8,g0,g0                # g0 = request length + response length
        st      r7,sndg2_resplen(g1)    # save response length (real)
        st      r4,il_cr(g1)            # save completion routine address

c       g0 = m_asglbuf(g0);             # allocate a SGL and buffer

        ld      sg_desc0+sg_addr(g0),r4 # r4 = address of SG data area
        st      g0,sndg2_sgl(g1)        # save SGL pointer
        addo    r4,r6,r10               # r10 = pointer to response header
        st      r4,sndg2_reqptr(g1)     # save ptr to request header
        st      r10,sndg2_respptr(g1)   # save ptr to response header
        st      r3,sndg2_esgl(g1)       # clear the extended SGL pointer
        stob    r3,sndg2_cmdto(g1)      # use default command time out
        stob    r3,sndg2_rtycnt(g1)     # use default command retry count
#
# --- build the request header
#
        mov     r4,g0                   # g0 = ptr to request header
        ldq     OML_datagram_hdr,r4     # r4-r7 = 1st 16 bytes of header
        ldob    ltmt_path(g4),r3        # r3 = peer path number
        shlo    8,r3,r3                 # shift it left 8 bits
        or      r3,r5,r5                # merge in path #

        ld      K_ficb,r6               # r6 = FICB
        ld      fi_cserial(r6),r6       # r6 = controller serial number
!       ld      sr_oml_sn(g2),r7        # r4 = destination sn
        bswap   r6,r6                   # change src sn endiance
        bswap   r7,r7                   # change dest sn endiance
        stq     r4,dgrq_srvcpu(g0)      # save 1st 16 bytes of header

        ldq     OML_datagram_hdr+16,r4  # r4-r7 = 2nd 16 bytes of header
        stq     r4,dgrq_srvname(g0)     # save 2nd 16 bytes of header
#
# --- build up message area
#
        lda     dgrq_size(g0),g0        # g0 = payload area

        ldob    ci_num(g3),r4           # r4 = source path # (from CIMT)
        shlo    8,r4,r4                 # shift it over 8 bits,
                                        #   source cluster = 0
!       ldos    sr_oml_vid(g2),r5       # r5 = destination Vdisk #
        shlo    8,r5,r5                 # shift it over 8 bits
!       ldob    sr_oml_cl(g2),r3        # r3 = destination cluster
        or      r3,r5,r5                # merge them together
        stl     r4,LLD0_rq_estvl_srccl(g0) # save message area
#
# --- send the datagram
#
        lda     ILTBIAS(g1),g1          # g1 = ILT at sndg3 lvl
        call    lld$snddg               # send the datagram

.OMLopncr_1000:
        movl    r10,g4                  # restore g4-g5
        movq    r12,g0                  # restore g0-g3
        ret
#
# --- Establish Magnitude Link
#        Header and payload templates
#
        .data
OML_datagram_hdr:
        .byte   dg_cpu_interface        # server processor code
        .byte   dgrq_size               # request header size
        .short  0                       # sequence number
        .byte   LLD0_fc_estvl           # request function code
        .byte   0                       # destination path number
        .byte   0                       # gpr #0
        .byte   0                       # gpr #1
        .word   0                       # source serial #
        .word   0                       # destination serial #

        .ascii  "LLD0"                  # dest server name
        .byte   0                       ## Remaining request length
        .byte   0                       ## NOTE: This is a word field and
        .byte   0                       ## is formed in this manner for
        .byte   LLD0_rq_estvl_size      ## correct endiance
        .byte   0                       # gpr #2
        .byte   0                       # gpr #3
        .byte   0                       # gpr #4
        .byte   0                       # gpr #5
        .word   0                       # crc

OML_datagram_msg:
        .byte   0                       # source cluster #
        .byte   0                       # source path #
        .short  0                       # spare
        .byte   0                       # destination cluster #
        .byte   0                       # destination Vdisk #
        .short  0                       # 2 spare
#
        .text
#
#**********************************************************************
#
#  NAME: OMLdg_cr
#
#  PURPOSE:
#       Processes the completion for a send datagram call from OMLopn_cr.
#
#  CALLING SEQUENCE:
#       OMLdg_cr
#
#  INPUT:
#       g1 = ILT/IRP address of CDB request at nest level 2
#       g4 = LTMT
#       g5 = LSMT
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Reg. g0-g12, g14 can be destroyed.
#
#**********************************************************************
#
OMLdg_cr:
        ld      sndg2_srp(g1),g2        # g2 = SRP addr
        ldconst srok,r8                 # r8 = good srp completion status

        ld      sndg2_respptr(g1),r15   # r15 = response header
        ldob    dgrs_status(r15),r4     # r4 = status
        cmpobe.t 0,r4,.OMLdg_cr_100     # Jif everything OK

        ldconst 0,r3
        ldconst srerr,r8                # r8 = good srp completion status
!       ld      sr_oml_ospb(g2),r4      # r4 = ospb base
!       ld      mlos_lldid(r4),r5       # r5 = current lsmt
        st      r3,lsmt_dlmid(r5)       # clear DLM ID field

.OMLdg_cr_100:
        ld      sndg2_sgl(g1),g0        # g0 = SGL pointer
        st      0,sndg2_sgl(g1)         # make sure cannot use sgl pointer again
        call    M$rsglbuf               # return sgl

        mov     r8,g0                   # g0 = srp completion status
        lda     -ILTBIAS(g1),g1         # g1 = ILT/SRP at nest level #1
        ld      il_cr(g1),r8            # r4 = ILT completion handler routine
        callx   (r8)                    # call ILT/SRP completion handler routine
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: lld$CML
#
#  PURPOSE:
#       Processes a CLOSE session to MAGNITUDE SRP request.
#
#  CALLING SEQUENCE:
#       call    lld$CML
#
#  INPUT:
#       g1 = ILT associated with SRP at nest level #2
#       g2 = SRP address to process
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Reg. g0-g12, g14 can be destroyed.
#
#**********************************************************************
#
lld$CML:
!       ld      sr_cml_lldid(g2),g5     # g5 = link-level driver session ID
                                        #  (LSMT address)
        ld      lsmt_dlmid(g5),r6       # r6 = DLM session ID from LSMT
        cmpobne.t 0,r6,.lldCML_100      # Jif DLM Session ID established
#
# --- Return error since something's wrong.
#
.lldCML_err:
        ldconst srerr,g0                # g0 = SRP return status
        lda     -ILTBIAS(g1),g1         # g1 = ILT/SRP at nest level #1
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT/SRP completion handler routine
        b       .lldCML_1000            # and we're out of here!
#
# --- Check if the Vdisk (LUN) is within range...
#
.lldCML_100:

        ld      lsmt_ltmt(g5),g4        # g4 = LTMT
        cmpobe.t 0,g4,.lldCML_err       # Jif DLM Session ID established
        ldconst MAXLUN,r4               # r12 = max. valid LUN #
!       ldos    sr_cml_vid(g2),g0       # g0 = vdisk # (LUN)
        cmpobge.f g0,r4,.lldCML_err     # Jif invalid LUN # specified

        ldos    lsmt_lun(g5),r4         # r4 = lun
        ld      lsmt_dlmid(g5),r5       # r5 = DLM ID
        cmpobne.f r4,g0,.lldCML_err     # Jif lun's don't match

#        cmpobe.f r5,r6,.lldCML_300      # Jif this is the matching LSMT
#*** FINISH
        ldconst 0,r3
        st      r3,lsmt_dlmid(g5)       # clear DLM ID from LSMT
#
# --- allocate a SGL for the request and the response.
#
        ld      ltmt_seshead(g4),g5     # g5 = LSMT for lun 0
        st      g2,sndg2_srp(g1)        # save srp address
        ld      ltmt_cimt(g4),g3        # g3 = cimt

        lda     CMLdg_cr,r4             # r4 = completion routine
        ldconst dgrq_size+LLD0_rq_trmvl_size,r5 # r5 = request length (real)
        addo    4-1,r5,r6               # round up to a word boundary
        andnot  4-1,r6,r6               # r6 = request length (adjusted)
        mov     r6,g0                   # g0 = request length
        st      r5,sndg2_reqlen(g1)     # save request length (real)

        ldconst dgrs_size+LLD0_rs_trmvl_size,r7 # r7 = response length (real)
        addo    4-1,r7,r8               # round up to a word boundary
        andnot  4-1,r8,r8               # r8 = response length (adjusted)
        addo    r8,g0,g0                # g0 = request length + response length
        st      r7,sndg2_resplen(g1)    # save response length (real)
        st      r4,il_cr(g1)            # save completion routine address

c       g0 = m_asglbuf(g0);             # allocate a SGL and buffer

        ld      sg_desc0+sg_addr(g0),r4 # r4 = address of SG data area
        st      g0,sndg2_sgl(g1)        # save SGL pointer
        addo    r4,r6,r10               # r10 = pointer to response header
        st      r4,sndg2_reqptr(g1)     # save ptr to request header
        st      r10,sndg2_respptr(g1)   # save ptr to response header
        st      r3,sndg2_esgl(g1)       # clear the extended SGL pointer
        stob    r3,sndg2_cmdto(g1)      # use default command time out
        stob    r3,sndg2_rtycnt(g1)     # use default command retry count
#
# --- build the request header
#
        mov     r4,g0                   # g0 = SG Data Area
        ldq     CML_datagram_hdr,r4     # r4-r7 = 1st 16 bytes of header
        ldob    ltmt_path(g4),r3        # r3 = peer path number
        shlo    8,r3,r3                 # shift it left 8 bits
        or      r3,r5,r5                # merge in path #

        ld      K_ficb,r6               # r6 = FICB
        ld      fi_cserial(r6),r6       # r6 = Controller serial number
!       ld      sr_cml_sn(g2),r7        # r4 = destination sn
        bswap   r6,r6                   # change src sn endiance
        bswap   r7,r7                   # change dest sn endiance
        stq     r4,dgrq_srvcpu(g0)      # save 1st 16 bytes of header

        ldq     CML_datagram_hdr+16,r4  # r4-r7 = 2nd 16 bytes of header
        stq     r4,dgrq_srvname(g0)     # save 2nd 16 bytes of header
#
# --- build up message area
#
        lda     dgrq_size(g0),g0        # g0 = payload area

        ldob    ci_num(g3),r4           # r4 = source path # (from CIMT)
        shlo    8,r4,r4                 # shift it over 8 bits
                                        #   source cluster = 0
!       ldos    sr_cml_vid(g2),r5       # r5 = destination Vdisk #
        shlo    8,r5,r5                 # shift it over 8 bits
!       ldob    sr_cml_cl(g2),r3        # r3 = destination cluster
        or      r3,r5,r5                # merge them together
        stl     r4,LLD0_rq_trmvl_srccl(g0) # save message area
#
# --- send the datagram
#
        lda     ILTBIAS(g1),g1          # g1 = ILT at sndg3 lvl
        call    lld$snddg               # send the datagram

.lldCML_1000:
        ret
#
# --- Close Magnitude Link
#        Header and payload templates
#
        .data
CML_datagram_hdr:
        .byte   dg_cpu_interface        # server processor code
        .byte   dgrq_size               # request header size
        .short  0                       # sequence number
        .byte   LLD0_fc_trmvl           # request function code
        .byte   0                       # hab number
        .byte   0                       # gpr #0
        .byte   0                       # gpr #1
        .word   0                       # source serial #
        .word   0                       # destination serial #

        .ascii  "LLD0"                  # dest server name
        .byte   0                       ## Remaining request length
        .byte   0                       ## NOTE: This is a word field and
        .byte   0                       ## is formed in this manner for
        .byte   LLD0_rq_trmvl_size      ## correct endiance
        .byte   0                       # gpr #2
        .byte   0                       # gpr #3
        .byte   0                       # gpr #4
        .byte   0                       # gpr #5
        .word   0                       # crc

CML_datagram_msg:
        .byte   0                       # source cluster #
        .byte   0                       # source path #
        .short  0                       # spare
        .byte   0                       # destination cluster #
        .byte   0                       # destination Vdisk #
        .short  0                       # 2 spare
#
        .text
#
#**********************************************************************
#
#  NAME: CMLdg_cr
#
#  PURPOSE:
#       Processes the completion for a send datagram call from CMLopn_cr.
#
#  CALLING SEQUENCE:
#       CMLdg_cr
#
#  INPUT:
#       g1 = ILT/IRP address of CDB request at nest level 2
#       g4 = LTMT
#       g5 = LSMT
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Reg. g0-g12, g14 can be destroyed.
#
#**********************************************************************
#
CMLdg_cr:
        ld      sndg2_srp(g1),g2        # g2 = SRP addr

        ld      sndg2_sgl(g1),g0        # g0 = SGL pointer
        st      0,sndg2_sgl(g1)         # make sure cannot use sgl pointer again
        call    M$rsglbuf               # return sgl

        ldconst srok,g0                 # g0 = good srp completion status
        lda     -ILTBIAS(g1),g1         # g1 = ILT/SRP at nest level #1
        ld      il_cr(g1),r8            # r4 = ILT completion handler routine
        callx   (r8)                    # call ILT/SRP completion handler routine
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: lld$OFT
#
#  PURPOSE:
#       Processes an OPEN session to Foreign Target SRP request.
#
#  DESCRIPTION:
#       This routine validates that the requestor's parameters match
#       the target/LUNs parameters if the requestor has specified
#       these parameters. Specified parameters that are checked include
#       VENDOR ID, PRODUCT ID, DEVICE SERIAL NUMBER, DISK SIZE in sectors.
#       If specified parameters do not match, an error is returned to
#       the caller. The logic utilizes the device serial number as the
#       primary parameter to match up. If the device serial number
#       is not specified, the specified LUN # is used if it exists
#       in the corresponding TLMT directory from the last target/LUN
#       discovery process. If the requestor's parameters match, a LSMT
#       is allocated, initialized and a session OPENed to the initiator
#       driver. If this OPEN is successful, the session has been opened
#       successfully to the device. If the OPEN fails, the session resources
#       are returned and an error is reported to the requestor.
#
#  CALLING SEQUENCE:
#       call    lld$OFT
#
#  INPUT:
#       g1 = ILT associated with SRP at nest level #2
#       g2 = SRP address to process
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Reg. g0-g12, g14 can be destroyed.
#
#**********************************************************************
#
lld$OFT:
        ld      sr_oft_lldid(g2),r14    # r14 = link-level driver session ID (LTMT address)
        ld      ltmt_dlmid(r14),r6      # r6 = DLM session ID from LTMT
        cmpobne.t 0,r6,.lldOFT_100      # Jif DLM Session ID established
#
# --- Return error since something's wrong.
#
.lldOFT_40:
        ldconst srerr,g0                # g0 = SRP return status
        lda     -ILTBIAS(g1),g1         # g1 = ILT/SRP at nest level #1
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT/SRP completion handler routine
        b       .lldOFT_1000            # and we're out of here!
#
.lldOFT_100:
        ld      sr_oft_ospb(g2),r15     # r15 = FTOSPB address
        ld      ltmt_tmt(r14),r13       # r13 = assoc. TMT address
c   if (((SRP*)g2)->func == sropft) {
        ldl     ftos_venid(r15),r4      # r4-r5 = specified vendor ID
c   } else {
        ldl     ftos_venid_GT2TB(r15),r4 # r4-r5 = specified vendor ID
c   }
        ldl     tm_venid(r13),r6        # r6-r7 = target vendor ID
        cmpobe.f 0,r4,.lldOFT_110       # Jif vendor ID not specified
        cmpobne.f r4,r6,.lldOFT_40      # Jif vendor ID does not match
        cmpobne.f r5,r7,.lldOFT_40      # Jif vendor ID does not match
.lldOFT_110:
c   if (((SRP*)g2)->func == sropft) {
        stl     r6,ftos_venid(r15)      # save vendor ID in FTOSPB just in
                                        #  case it wasn't specified
        ldt     ftos_prid(r15),r4       # r4-r6 = specified product ID
c   } else {
        stl     r6,ftos_venid_GT2TB(r15) # save vendor ID in FTOSPB just in
                                        #  case it wasn't specified
        ldt     ftos_prid_GT2TB(r15),r4 # r4-r6 = specified product ID
c   }
        ldt     tm_proid(r13),r8        # r8-r10 = target product ID
        cmpobe.f 0,r4,.lldOFT_120       # Jif product ID not specified
        cmpobne.f r4,r8,.lldOFT_40      # Jif product ID does not match
        cmpobne.f r5,r9,.lldOFT_40      # Jif product ID does not match
        cmpobne.f r6,r10,.lldOFT_40     # Jif product ID does not match
.lldOFT_120:
c   if (((SRP*)g2)->func == sropft) {
        stt     r8,ftos_prid(r15)       # save product ID in FTOSPB just in
                                        #  case it wasn't specified
c   } else {
        stt     r8,ftos_prid_GT2TB(r15) # save product ID in FTOSPB just in
c   }
                                        #  case it wasn't specified
        ldq     ftos_devsn(r15),r4      # r4-r7 = device serial #
        ldos    sr_oft_lun(g2),r11      # r11 = requested LUN #
        cmpobe.f 0,r4,.lldOFT_200       # Jif device serial # not specified
#
# --- Device serial number specified. Find LUN # that matches.
#
        ldconst 0xff,r12
        cmpobe.f r11,r12,.lldOFT_150    # Jif no LUN specified
        ldconst MAXLUN,r12              # r12 = max. valid LUN #
        cmpobge.f r11,r12,.lldOFT_40    # Jif invalid LUN # specified
        shlo    2,r11,r9                # r9 = specified LUN # * 4
        lda     tm_tlmtdir(r13),r10     # r10 = TLMT directory pointer
        addo    r9,r10,r10              # r10 = specified TLMT pointer
        ld      (r10),r12               # r12 = assoc. TLMT address
        cmpobe.f 0,r12,.lldOFT_150      # Jif no TLMT at specified LUN #
        ldq     tlm_sn(r12),r8          # r8-r11 = device serial # for
                                        #  this LUN
        cmpobne.f r4,r8,.lldOFT_150     # Jif serial # does not match
        cmpobne.f r5,r9,.lldOFT_150     # Jif serial # does not match
        cmpobne.f r6,r10,.lldOFT_150    # Jif serial # does not match
        cmpobe.t r7,r11,.lldOFT_300     # Jif serial # does match
#
# --- Scan target LUNs for match on device serial #
#
.lldOFT_150:
        lda     tm_tlmtdir(r13),r10     # r10 = TLMT directory pointer
        ldconst MAXLUN,r8               # r8 = number of LUNs to check
.lldOFT_155:
        ld      (r10),r12               # r12 = TLMT address
        addo    4,r10,r10               # inc. to next TLMT in directory
        subo    1,r8,r8                 # dec. LUN counter
        cmpobe.t 0,r12,.lldOFT_160      # Jif no TLMT defined
        ldq     tlm_sn(r12),g4          # g4-g7 = device serial # for this LUN
        cmpobne.f r4,g4,.lldOFT_160     # Jif serial # does not match
        cmpobne.f r5,g5,.lldOFT_160     # Jif serial # does not match
        cmpobne.f r6,g6,.lldOFT_160     # Jif serial # does not match
        cmpobe.f r7,g7,.lldOFT_300      # Jif serial # does match
.lldOFT_160:
        cmpobne.t 0,r8,.lldOFT_155      # Jif more TLMTs to check
        b       .lldOFT_40              # return error to requestor
#
# --- Device serial number not specified. Use specified LUN number.
#
.lldOFT_200:
        ldconst MAXLUN,r12              # r12 = max. valid LUN #
        cmpobge.f r11,r12,.lldOFT_40    # Jif invalid LUN # specified
        shlo    2,r11,r9                # r9 = specified LUN # * 4
        lda     tm_tlmtdir(r13),r10     # r10 = TLMT directory pointer
        addo    r9,r10,r10              # r10 = specified TLMT pointer
        ld      (r10),r12               # r12 = assoc. TLMT address
        cmpobe.f 0,r12,.lldOFT_40       # Jif no TLMT at specified LUN #
        ldq     tlm_sn(r12),r4          # r4-r7 = device serial #
        stq     r4,ftos_devsn(r15)      # save device serial # in FTOSPB
#
# --- TLMT identified to use for requestor's session.
#
.lldOFT_300:
                                        # r12 = TLMT address to open session to
        ldl     tlm_blkcnt(r12),r4      # r5 = disk size in TLMT
c   if (((SRP*)g2)->func == sropft) {
c       if (r5 != 0) {
        b       .lldOFT_40              # Disk size cannot match too big for 32 bit SRP
c       }
        ld      ftos_dsize(r15),r6      # r6 = lower 32 bits of disk size in FTOSPB
c       r7 = 0;
c   } else {
        ldl     ftos_dsize_GT2TB(r15),r6 # r4 = disk size in FTOSPB
c   }

c   if (*(UINT64*)&r6 == 0) {
c       if (((SRP*)g2)->func == sropft) {
            st      r4,ftos_dsize(r15)      # save disk size in FTOSPB in case it wasn't specified
c       } else {
            stl     r4,ftos_dsize_GT2TB(r15) # save disk size in FTOSPB in case it wasn't specified
c       }
c   } else if (*(UINT64*)&r4 != *(UINT64*)&r6) {
        b       .lldOFT_40              # Jif disk size does not match
c   }
#
c       g3 = get_lsmt();                # allocate a LSMT for Foreign Target session
        st      0,lsmt_link(g3)         # clear forward link
.ifdef M4_DEBUG_LSMT
c fprintf(stderr, "%s%s:%u get_lsmt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g3);
.endif # M4_DEBUG_LSMT
        st      g3,ftos_lldid(r15)      # save LSMT in FTOSPB
        ld      sr_oft_dlmid(g2),g4     # g4 = requestor's session ID
        st      g4,lsmt_dlmid(g3)       # save requestor's session ID in LSMT
        mov     r14,g4                  # g4 = assoc. LTMT address
        ldos    tlm_lun(r12),g0         # g0 = LUN # to open session for
        stos    g0,lsmt_lun(g3)         # save LUN # in LSMT
        st      g4,lsmt_ltmt(g3)        # save assoc. LTMT address in LSMT
        lda     lsmt_etbl2a,r4          # r4 = session event handler table
        st      r4,lsmt_ehand(g3)       # save event handler table in LSMT
#
# --- Put LSMT onto LTMT session list
#
        ld      ltmt_sestail(g4),r4     # r4 = last LSMT on session list
        cmpobne.t 0,r4,.lldOFT_330      # Jif list not empty
        st      g3,ltmt_seshead(g4)     # save LSMT as new head member
        b       .lldOFT_340
#
.lldOFT_330:
        st      g3,lsmt_link(r4)        # link LSMT onto end of list
.lldOFT_340:
        st      g3,ltmt_sestail(g4)     # put new LSMT as new tail member
#
# --- Set up to send open request to initiator driver
#
        lda     OFTopen_cr,g2           # g2 = open ILT completion handler
                                        #  routine
        lda     LLD$session_term,g6     # g6 = session termination routine
        call    lld$sendopen            # open session for Foreign Target
                                        #  linked device session
                                        #  g0 = LUN # to open session to
                                        #  g1 = ILT/SRP at 2nd lvl
                                        #  g2 = ILT completion handler routine address
                                        #  g3 = LSMT associated with session being opened
                                        #  g4 = assoc. LTMT address
                                        #  g5 = n/u
                                        #  g6 = session termination routine adr
.lldOFT_1000:
        ret
#
#******************************************************************************
#
#  NAME: OFTopen_cr
#
#  PURPOSE:
#       Processes completion of the open IRP request for a Foreign
#       Target linked device session.
#
#  DESCRIPTION:
#       Checks if ILT/IRP is registered with LSMT. If not, ILT/IRP
#       was aborted. Just return ILT & IRP and return to caller. If
#       ILT/IRP is registered to LSMT, checks if OPEN request failed.
#       If it failed, returns ILT & IRP and return an error back to
#       the requestor. If the OPEN request succeeded, completes the
#       open session processing and returns a successful completion
#       status to the requestor.
#
#  CALLING SEQUENCE:
#       call    OFTopen_cr
#
#   INPUT:
#       g1 = ILT/IRP address of OPEN at nest level 1
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       None.
#
#******************************************************************************
#
OFTopen_cr:
        movq    g0,r12                  # save g0-g3
        ld      lirp1_irp(g1),g0        # g0 = IRP for OPEN request
        ld      lirp1_lldid(g1),g3      # g3 = assoc. LSMT address
        ld      lirp1_g0(g1),r11        # r11 = requestor's ILT address
        ld      lsmt_ilt(g3),r4         # r4 = assoc. ILT/IRP from LSMT
        ldob    irp_cmplt(g0),r5        # r5 = OPEN request completion status
        cmpobe.t g1,r4,.OFTopencr_200   # Jif ILT registered in LSMT
#
# --- ILT/IRP not registered in LSMT
#
        ldconst srabt,r10               # r10 = SRP return status
        b       .OFTopencr_900          # and get out
#
# --- ILT/IRP still registered in LSMT
#
.OFTopencr_200:
        mov     0,r3
        st      r3,lsmt_ilt(g3)         # clear assoc. ILT field in LSMT
        cmpobe.t cmplt_success,r5,.OFTopencr_300 # Jif OPEN successful
#
# --- OPEN request failed
#
        mov     g4,r10                  # save g4
        ld      lsmt_ltmt(g3),g4        # g4 = assoc. LTMT address
        call    lld$rem_lsmt            # remove LSMT from LTMT list
.ifdef M4_DEBUG_LSMT
c fprintf(stderr, "%s%s:%u put_lsmt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g3);
.endif # M4_DEBUG_LSMT
c       put_lsmt(g3);                   # Deallocate LSMT
        mov     r10,g4                  # restore g4
        ldconst srerr,r10               # r10 = SRP return status
        b       .OFTopencr_900          # return error back to requestor
#
# --- OPEN request succeeded
#
.OFTopencr_300:
        ld      irp_Pro_ID(g0),r6       # r6 = initiator ID
        st      r6,lsmt_psid(g3)        # save initiator ID in LSMT
#*** FINISH - may need to send CDBs to target
        ldconst srok,r10                # r10 = SRP return status
.OFTopencr_900:
.ifdef M4_DEBUG_IRP
c CT_history_printf("%s%s:%u put_irp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif # M4_DEBUG_IRP
c       put_irp(g0);                    # release IRP
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
        lda     -ILTBIAS(r11),g1        # g1 = ILT/SRP at nest level #1
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        mov     r10,g0                  # g0 = SRP completion status
        callx   (r4)                    # call ILT/SRP completion handler routine
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: lld$CFT
#
#  PURPOSE:
#       Processes a CLOSE session to Foreign Target SRP request.
#
#  CALLING SEQUENCE:
#       call    lld$CFT
#
#  INPUT:
#       g1 = ILT associated with SRP at nest level #2
#       g2 = SRP address to process
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Reg. g0-g12, g14 can be destroyed.
#
#**********************************************************************
#
lld$CFT:
        ld      sr_cft_lldid(g2),g3     # g3 = link-level driver session ID
        cmpobne.t 0,g3,.lldCFT_100      # JIf not zero
#
# --- send error status
#
        lda     -ILTBIAS(g1),g1         # g1 = ILT/SRP at nest level #1
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        ldconst    srerr,g0             # g0 = SRP completion status
        callx   (r4)                    # call ILT/SRP completion handler routine
                                        #  (LSMT address)
        b       .lldCFT_1000            # exit
#
# --- there seem to be a lsmt, so sent the open
#
.lldCFT_100:
        ld      lsmt_ltmt(g3),g4        # g4 = ltmt

        mov     g2,g5                   # save srp
        lda     CFTcls_cr,g2            # g2 = open ILT completion handler
                                        #  routine
        call    lld$sendclose           # close session for Foreign Target
                                        #  linked device session
                                        # Input:
                                        #  g2 = ILT completion handler routine address
                                        #  g3 = LSMT associated with session being opened
                                        #  g4 = assoc. LTMT address
                                        #  g5 = srp
.lldCFT_1000:
        ret
#
#******************************************************************************
#
#  NAME: CFTcls_cr
#
#  PURPOSE:
#       Processes completion of the close IRP request for a Foreign Target
#       linked device session.
#
#  CALLING SEQUENCE:
#       call    CFTcls_cr
#
#   INPUT:
#       g1 = ILT/IRP address of OPEN at nest level 1
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       None.
#
#******************************************************************************
#
CFTcls_cr:
        movq    g0,r12                  # save g0-g3
        movl    g4,r10                  # save g4-g5
        ld      lirp1_irp(g1),g0        # g0 = IRP for OPEN request
        ld      lirp1_lldid(g1),g3      # g3 = assoc. LSMT address
        ld      lirp1_g0(g1),r9         # r9 = ILT'
        ld      lirp1_g1(g1),g2         # g2 = SRP
        ld      lsmt_ltmt(g3),g4        # g4 = LTMT
        ldob    irp_cmplt(g0),r5        # r5 = close request completion status
#
# --- ILT/IRP still registered in LSMT
#
        ldconst srerr,r8                # r8 = SRP return status
        cmpobne.t cmplt_success,r5,.CFTclscr_100 # Jif CLOSE failed
#
# --- close was successful
#
        ldconst srok,r8                 # r8 = SRP return status (OK)
#
# --- release the IRP and ILT"
#
.CFTclscr_100:
.ifdef M4_DEBUG_IRP
c CT_history_printf("%s%s:%u put_irp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif # M4_DEBUG_IRP
c       put_irp(g0);                    # release IRP
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
#
# --- remove LSMT from queue and release it to free pool
#
        call    lld$rem_lsmt            # remove LSMT from LTMT list
.ifdef M4_DEBUG_LSMT
c fprintf(stderr, "%s%s:%u put_lsmt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g3);
.endif # M4_DEBUG_LSMT
c       put_lsmt(g3);                   # Deallocate LSMT
#
# --- send ILT' completion
#
        lda     -ILTBIAS(r9),g1         # g1 = ILT/SRP at nest level #1
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        mov     r8,g0                   # g0 = SRP completion status
        callx   (r4)                    # call ILT/SRP completion handler routine
#
# --- restore registers and return to caller
#
        movq    r12,g0                  # restore g0-g3
        movl    r10,g4                  # restore g4-g5
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: lld$LRP
#
#  PURPOSE:
#       Processes an Execute LRP to Device SRP request.
#
#  CALLING SEQUENCE:
#       call    lld$LRP
#
#  INPUT:
#       g1 = ILT associated with SRP at nest level #2
#       g2 = SRP address to process
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Reg. g0-g12, g14 can be destroyed.
#
#**********************************************************************
#
lld$LRP:
#
# --- validate DLM ID
#
!       ld      sr_lrp_dlmid(g2),r4     # r4 = data link manager id
        ldconst cmplt_PE,r8             # r8 = default parameter error
        ldconst pe_invreqID,r9          # r9 = default invalid requestor ID
        cmpobne.f 0,r4,.lrp_100         # Jif dlm is defined
#
# --- lrp error return
#
#       r8 = EC1 error code
#       r9 = EC2 error code
#       g1 = ILT' at 2nd lvl
#       g2 = srp
#
.lrp_err:
!       ld      sr_lrp_extstat(g2),r5   # r5 = address of extended status
        mov     r8,g0                   # g0 = set completion status
!       st      r9,(r5)                 # save extended status

        lda     -ILTBIAS(g1),g1         # g1 = ILT/SRP at nest level #1
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT/SRP completion handler routine
        b       .lrp_1000               # and we're out of here!
#
# --- validate the LLD ID
#
.lrp_100:
        ldconst pe_invproID,r9          # r9 = default invalid provider ID
!       ld      sr_lrp_lldid(g2),g5     # g5 = LSMT
        cmpobe.f 0,g5,.lrp_err          # Jif lld ID is not defined
#
# --- determine if DLM id from SRP is the same as DLM ID from LTMT
#
? # crash - cqt# 24600 - 2008-12-22 -- FE LSMT - ld 8+g5,g4 with fefefefe -- workaround?
        ld      lsmt_dlmid(g5),r5       # get DLM ID from LTMT
        ldconst pe_invreqID,r9          # r9 = default invalid requestor ID
        cmpobne.f r4,r5,.lrp_err        # JIf if not the same (free lsmt is zero)
#
# --- determine if LTMT within LSMT was freed inbetween...
#
        ld      lsmt_ltmt(g5),g4        # g4 = LTMT
        ldconst pe_notarg,r9            # r9 = default invalid provider ID
        cmpobe.t 0,g4,.lrp_err          # Jif LTMT is NULL
        ld      ltmt_tmt(g4),r6         # r6 = assoc. TMT address
        cmpobe.f 0,r6,.lrp_err          # JIf if no TMT assoc. with LTMT
        ldl     tm_P_name(r6),r4        # r4-r5 = target port name
        or      r4,r5,r4
        cmpobe  0,r4,.lrp_err           # Jif target port name has been cleared
#
# --- determine if initiator is online
#
        ld      ltmt_cimt(g4),r6        # r6 = cimt address
        ldob    ci_istate(r6),r5        # r5 = state code
        ldconst pe_IFOOS,r9             # r9 = default interface offline
        cmpobne.f cis_on,r5,.lrp_err    # Jif offline or uninitialized
#
# --- validate the LRP function code
#
!       ldob    sr_lrp_fc(g2),r10       # r10 = lrp function code
        ldconst sr_lrpfc_max,r4         # r4 = max lrp function code
        ldconst pe_invRFC,r9            # r8 = default invalid request function code
        cmpobl.f r4,r10,.lrp_err        # Jif fc out of range

        lda     -ILTBIAS(g1),r14        # r14 = ILT' at sndg2 lvl (2nd lvl)
        mov     g1,r15                  # r15 = ILT' at sndg3 lvl (3rd lvl)
#
# --- allocate ILT/IRP for the write buffer command and set up the
#     CDB.
#
#     Registers at this point are...
#       r15 = ILT at nest level 2
#       g2 = SRP address
#       g4 = LTMT address
#       g5 = LSMT address
#
        call    M$ailtirp               # allocate an ILT/IRP combo (meal?)
                                        # g0 = IRP address
                                        # g1 = ILT" address

        lda     lrp_cr,r4               # r4 = ILT" completion handler routine

        st      g0,lirp1_irp(g1)        # save IRP address in ILT"
        st      r4,lirp1_cr(g1)         # save completion routine
        st      g5,lirp1_lldid(g1)       # save LSMT address in ILT"
        st      r15,lirp1_g1(g1)        # save ILT' pointer in ILT"
        st      g2,lirp1_srp(g1)        # save srp address

        ldconst 4,r4                    # set retry count to 4
        stob    r4,lirp1_retry(g1)      # and save it

        lda     ILTBIAS(g1),g1          # g1 = ILT"/IRP at nest level 2
        st      g0,lirp2_irp(g1)        # save IRP address in nest level 2
#
# --- setup the CDB attributes and set SGL pointer
#
        ldconst rfc_SCSI_cmd,r4         # r4 = IRP request function code
        ld      lsmt_psid(g5),r5        # r5 = provider ID
        st      g5,irp_Req_ID(g0)       # save LSMT as requestor ID in IRP
        stob    r4,irp_RFC(g0)          # save request function code
        st      r5,irp_Pro_ID(g0)       # save provider ID in IRP

        lda     sr_lrp_sgl(g2),r4       # r4 = address of SGL
        ldconst ttc_simple,r5           # r5 = task type code
!       ldob    sr_lrp_retry(g2),r6     # r6 = retry count
!       ldos    sr_lrp_timeout(g2),r7   # r7 = timeout value
        shro    4,r15,r15               # form a tag ID
        stob    r5,irp_Tattr_TTC(g0)    # save task type code
        stob    r6,irp_rtycnt(g0)       # save CDB retry count
        st      r4,irp_SGLptr(g0)       # save ptr to SGL table
        stos    r7,irp_cmdTO(g0)        # save timeout value
        stos    r15,irp_tagID(g0)       # save tag ID
        mov     0,r3
        stob    r3,irp_Tattr_flags(g0)  # clear SLIE flag
#
# --- build up the CDB
#
        ld      lrpcdb_tbl[r10*4],r4    # r4 = address of CDB routine
        callx   (r4)                    # build up CDB
#
# --- send the write buffer off to the peer magnitude
#
        call    APL$pfr                 # request CDB be issued
#
# --- exit
#
.lrp_1000:
        ret

        .data
lrpcdb_tbl:
        .word   lld$lrp_write10         # 00 sr_lrpfc_write - write
        .word   lld$lrp_read10          # 01 sr_lrpfc_read - read
        .word   lld$lrp_wtvrfy10        # 02 sr_lrpfc_writevfy - write and verify
        .word   lld$lrp_vfychkwd        # 03 sr_lrpfc_vfychkword - verify check word
        .word   lld$lrp_vrfydata        # 04 sr_lrpfc_vfydata - verify data
#
        .text
#
#**********************************************************************
#
#  NAME: lrp_cr
#
#  PURPOSE:
#       Processes the completion of a LRP function.
#
#  DESCRIPTION:
#       Place the IRP completion code in g0. If the IRP completion
#       code is non zero, place the irp extended code in the area
#       pointed to by sr_lrp_extstat.
#
#       In any case, return the ILT" and the IRP.
#
#  CALLING SEQUENCE:
#        lrp_cr
#
#  INPUT:
#       g1 = ILT"/IRP address of CDB request at nest level 1
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Reg. g0-g12, g14 can be destroyed.
#
#**********************************************************************
#
lrp_cr:
        mov     g1,r15                  # r15 = ILT" at 1st next
        ld      lirp1_irp(g1),r14       # r14 = IRP
        ld      lirp1_g1(r15),g1        # g1 = ILT'
        ld      lirp1_srp(r15),g2       # g2 = SRP
        ld      irp_extcmp(r14),r9      # r9 = extended completion code
        ldob    irp_cmplt(r14),g0       # g0 = completion status

        cmpobe.t cmplt_success,g0,.lrpcr_900 # Jif zero completion
        cmpobne.t cmplt_DTU,g0,.lrpcr_200 # Jif not data transfer underrun
#
# --- underrun error
#
        ld      irp_actcnt(g2),r9       # r9 = default actual data transfer

        ldob    lirp1_retry(r15),r4     # r4 = retry count
        cmpobe  0,r4,.lrpcr_200         # JIf no more retries

        subo    1,r4,r4                 # decrement retry count
        stob    r4,lirp1_retry(r15)     # save new retry counter
        lda     ILTBIAS(r15),g1         # g1 = ILT" at 2nd lvl

        call    APL$pfr                 # request CDB be issued
        b       .lrpcr_1000             # exit
#
# ---  non underrun errors
#
.lrpcr_200:
!       ld      sr_lrp_extstat(g2),r5   # r5 = address of extended status
!       st      r9,(r5)                 # save extended status

.lrpcr_900:
        lda     -ILTBIAS(g1),g1         # g1 = ILT/SRP at nest level #1
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT/SRP completion handler routine
#
# --- release ILT and IRP
#
        movl     r14,g0                 # g0/g1 = IRP/ILT"
.ifdef M4_DEBUG_IRP
c CT_history_printf("%s%s:%u put_irp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif # M4_DEBUG_IRP
c       put_irp(g0);                    # release IRP
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT

.lrpcr_1000:
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: lld$lrp_read10
#
#  PURPOSE:
#       Build up a read10 CDB.
#
#  CALLING SEQUENCE:
#       call    lld$lrp_read10
#
#  INPUT:
#       g1 = ILT" associated with IRP at 2nd level
#       g0 = IRP address
#       g2 = SRP address
#       g4 = LTMT address
#       g5 = LSMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       none.
#
#**********************************************************************
#
        .data
read10_cdb:                             # CDB pattern to use for read(10)
        .byte   0x28                    # command (10)
read10_flag:
        .byte   00                      # flag (10)
read10_dta:
        .byte   01                      # data transfer attribute
read16_cdb:
        .byte   0x88                    # command (16)
read16_flag:
        .byte   0x00                    # flag (16)
#
        .text
#
lld$lrp_read10:
        ldob    read10_cdb,r3           # r3 = command
        ldob    read10_flag,r4          # r4 = flags
        ldob    read10_dta,r15          # r15 = DTA
        ldob    read16_cdb,r13          # r13 = command (16)
        ldob    read16_flag,r14         # r14 = flags (16)
# NOTE: Falls through.

# Input:
# r3 = command (10)
# r4 = flags (10)
# r13 = command (16)
# r14 = flags (16)
# r15 = DTA
.lrp_cdb10_build:
        ldob    sr_lrp_sda_upper4th(g2),r8
        ldob    sr_lrp_sda_upper3(g2),r9
        ldob    sr_lrp_sda_upper3+1(g2),r10
        ldob    sr_lrp_sda_upper3+2(g2),r11
c       r9 = r8 | (r9 << 8) | (r10 << 16) | (r11 << 24);   # r9 is upper 32 bits of SDA
!       ld      sr_lrp_sda(g2),r8       # r8 = starting device address (lower 32 bits)
!       ld      sr_lrp_len(g2),r10      # r10 - transfer length
c   if (r9 == 0) {
#       10 byte command follows
        stob    r3,irp_CDB(g0)          # command (10)
        stob    r4,irp_CDB+1(g0)        # flags (10)
        bswap   r8,r8                   # change endiance of SDA lower
        st      r8,irp_CDB+2(g0)        # SDA - bytes 2-5
        bswap   r10,r10                 # change endiance of TL
        shro    8,r10,r10               # shift into position
        st      r10,irp_CDB+6(g0)       # Transfer length - bytes 7-9 (note 6 next line)
        stob    0,irp_CDB+6(g0)         # clear reserved field
        stob    0,irp_CDB+9(g0)         # clear control field
c   } else {
#       16 byte command follows
        stob    r13,irp_CDB(g0)         # command (16)
        stob    r14,irp_CDB+1(g0)       # flags (16)

        bswap   r9,r6                   # change endiance of SDA
        bswap   r8,r7
        stl     r6,irp_CDB+2(g0)        # SDA - bytes 2-9

        bswap   r10,r10                 # change endiance of TL
        st      r10,irp_CDB+10(g0)      # Transfer length - bytes 10-13

        stob    0,irp_CDB+14(g0)        # clear MMC-4/reserved/Group-number field
        stob    0,irp_CDB+15(g0)        # clear control field
c   }
        stob    r15,irp_Tattr_DTA(g0)   # Save data transfer attribute
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: lld$lrp_write10
#
#  PURPOSE:
#       Build up a write10 CDB.
#
#  CALLING SEQUENCE:
#       call    lld$lrp_write10
#
#  INPUT:
#       g1 = ILT" associated with IRP at 2nd level
#       g0 = IRP address
#       g2 = SRP address
#       g4 = LTMT address
#       g5 = LSMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
        .data
write10_cdb:                            # CDB pattern to use for write(10)
        .byte   0x2a                    # command (10)
write10_flag:
        .byte   00                      # flag (10)
write10_dta:
        .byte   02                      # data transfer attribute
write16_cdb:
        .byte   0x8a                    # command (16)
write16_flag:
        .byte   0x00                    # flag (16)
#
        .text
#
lld$lrp_write10:
        ldob    write10_cdb,r3          # r3 = command
        ldob    write10_flag,r4         # r4 = flags
        ldob    write10_dta,r15         # r15 = DTA
        ldob    write16_cdb,r13         # r13 = command (16)
        ldob    write16_flag,r14        # r14 = flags (16)
        b       .lrp_cdb10_build        # continue
#
#**********************************************************************
#
#  NAME: lld$lrp_wtvrfy10
#
#  PURPOSE:
#       Build up a write and verify CDB with checkbyte compare.
#
#  CALLING SEQUENCE:
#       call    lld$lrp_wtvrfy10
#
#  INPUT:
#       g1 = ILT" associated with IRP at 2nd level
#       g0 = IRP address
#       g2 = SRP address
#       g4 = LTMT address
#       g5 = LSMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       none.
#
#**********************************************************************
#
        .data
wtvrfy10_cdb:                           # CDB pattern for write and verify
        .byte   0x2e                    # command (10)
wtvrfy10_flag:
        .byte   00                      # flag (10)
wtvrfy10_dta:
        .byte   02                      # data transfer attribute
wtvrfy16_cdb:
        .byte   0x8e                    # command (16)
wtvrfy16_flag:
        .byte   0x00                    # flag (16)
#
        .text
#
lld$lrp_wtvrfy10:
        ldob    wtvrfy10_cdb,r3         # r3 = command
        ldob    wtvrfy10_flag,r4        # r4 = flags
        ldob    wtvrfy10_dta,r15        # r15 = DTA
        ldob    wtvrfy16_cdb,r13        # r13 = command (16)
        ldob    wtvrfy16_flag,r14       # r14 = flags (16)
        b       .lrp_cdb10_build        # continue
#
#**********************************************************************
#
#  NAME: lld$lrp_vfychkwd
#
#  PURPOSE:
#       Build up a verify with checkword compare CDB.
#
#  CALLING SEQUENCE:
#       call    lld$lrp_vfychkwd
#
#  INPUT:
#       g1 = ILT" associated with IRP at 2nd level
#       g0 = IRP address
#       g2 = SRP address
#       g4 = LTMT address
#       g5 = LSMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       none.
#
#**********************************************************************
#
        .data
vfychkwd10_cdb:                         # CDB pattern for verify command
        .byte   0x2f                    # command (10)
vfychkwd10_flag:
        .byte   00                      # with BYTCHK flag = zero (10)
vfychkwd10_dta:
        .byte   00                      # data transfer attribute
vfychkwd16_cdb:
        .byte   0x8f                    # command (16)
vfychkwd16_flag:
        .byte   0x00                    # flag (16)
#
        .text
#
lld$lrp_vfychkwd:
        ldob    vfychkwd10_cdb,r3       # r3 = command
        ldob    vfychkwd10_flag,r4      # r4 = flags
        ldob    vfychkwd10_dta,r15      # r15 = DTA
        ldob    vfychkwd16_cdb,r13       # r13 = command (16)
        ldob    vfychkwd16_flag,r14      # r14 = flags (16)
        b       .lrp_cdb10_build        # continue
#
#**********************************************************************
#
#  NAME: lld$lrp_vrfydata
#
#  PURPOSE:
#       Build up a verify with data compare CDB.
#
#  CALLING SEQUENCE:
#       call    lld$lrp_vrfydata
#
#  INPUT:
#       g1 = ILT" associated with IRP at 2nd level
#       g0 = IRP address
#       g2 = SRP address
#       g4 = LTMT address
#       g5 = LSMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       none.
#
#**********************************************************************
#
        .data
vrfydata10_cdb:                         # CDB pattern for write and verify
        .byte   0x2e                    # command (10)
vrfydata10_flag:
        .byte   02                      # flag (10)
vrfydata10_dta:
        .byte   02                      # data transfer attribute
vrfydata16_cdb:
        .byte   0x8e                    # command (16)
vrfydata16_flag:
        .byte   0x02                    # flag (16)
#
        .text
#
lld$lrp_vrfydata:
        ldob    vrfydata10_cdb,r3       # r3 = command
        ldob    vrfydata10_flag,r4      # r4 = flags
        ldob    vrfydata10_dta,r15      # r15 = DTA
        ldob    vrfydata16_cdb,r13      # r13 = command (16)
        ldob    vrfydata16_flag,r14     # r14 = flags (16)
        b       .lrp_cdb10_build        # continue
#
#**********************************************************************
#
#  NAME: lld$MSG
#
#  PURPOSE:
#       Processes a Send Message to MAGNITUDE SRP request.
#
#  CALLING SEQUENCE:
#       call    lld$MSG
#
#  INPUT:
#       g1 = ILT associated with SRP at nest level #2
#       g2 = SRP address to process
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Reg. g0-g12, g14 can be destroyed.
#
#**********************************************************************
#
lld$MSG:
#
# --- make sure the request has length
#
        ldconst dgec1_slld_PE,r8        # r8 default ec1 - parameter error
!       ld      sr_msg_reqlen(g2),r4    # r4 = request length
        ldconst slld_PE_reqlen,r9       # r9 default ec2 - request len = 0
        cmpobne.f 0,r4,.msg_020         # Jif length not 0
#
# --- MSG error return
#
#       r8 = EC1 error code
#       r9 = EC2 error code
#       g1 = ILT' at 2nd lvl
#       g2 = srp
#
.msg_err:

        ldconst 0,r3
!       ld      sr_msg_respbuf(g2),r5   # r5 = response buffer address
        ldconst dg_st_slld,r4           # r4 = completion status
!       stob    r8,dgrs_ec1(r5)         # save ec1
!       stob    r9,dgrs_ec2(r5)         # save ec2
!       stob    r4,dgrs_status(r5)      # save completion status
!       st      r3,dgrs_resplen(r5)     # save a zero length
.if ICL_DEBUG
c fprintf(stderr,"%s%s:%u <lld$MSG>ICL -- setting completion status as dg_st_slld\n", FEBEMESSAGE, __FILE__, __LINE__);
c fprintf(stderr,"%s%s:%u <lld$MSG>ICL -- ec1= %u ec2=%u\n", FEBEMESSAGE, __FILE__, __LINE__,(UINT32)r8, (UINT32)r9);
.endif  # ICL_DEBUG

# .msg_cmplt:
        ldconst srok,g0                 # set good srp completion status

.msg_cmplt_2:
        lda     -ILTBIAS(g1),g1         # g1 = ILT/SRP at nest level #1
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT/SRP completion handler routine
        b       .msg_1000               # and we're out of here!
#
# --- if a response is required, make sure there is a buffer. otherwise
#     make sure the response buffer address is zero.
#
.msg_020:
        ldconst srerr,g0                # default to error srp completion status
!       ld      sr_msg_reslen(g2),r5    # r5 = response length
!       ld      sr_msg_respbuf(g2),r6   # r6 = response buffer pointer
        cmpobe.f 0,r5,.msg_cmplt_2      # Jif no response length
        ldconst slld_PE_rwoSGL,r9       # r9 default ec2 - resp len w/o SGL
        cmpobe.f 0,r6,.msg_cmplt_2      # Jif there is no response buffer ptr
#
# --- validate DLM ID
#
!       ld      sr_msg_dlmid(g2),r4     # r4 = data link manager id
        ldconst slld_PE_dlmid,r9        # r9 default ec2 - no dlm id
        cmpobe.f 0,r4,.msg_err          # Jif dlm id not defined
#
# --- validate the LLD ID
#
!       ld      sr_msg_lldid(g2),g4     # g4 = LTMT
        cmpobne.f 0,g4,.msg_110         # Jif lld is defined
#
# --- This datagram message is to be process locally.
#
c fprintf(stderr, "%s%s:%u SEMINOLE 2008-06-02 SRP supposed local datagram received, return ILT with ERROR\n", FEBEMESSAGE,__FILE__, __LINE__);
# Original code below. Since Magnitude days, the two loads were added. This
# loses ILT in g1. It was decided today that I would print out a message, and
# abort the request with error.
# !       ld      sr_msg_reqbuf(g2),g0    # r4 = request buffer address
# !       ld      sr_msg_respbuf(g2),g1   # r6 = response buffer address
#        call    lld$dg_server           # process local message
#        b       .msg_cmplt              # complete the srp
        b       .msg_cmplt_2            # complete the srp with error
#
# --- determine if DLM id from SRP is the same as DLM ID from LTMT
#
.msg_110:
? # crash - cqt# 24405 - 2008-12-04 -- FE LTMT - ld 16+g4,r5 with fabafaba - workaround?
        ld      ltmt_dlmid(g4),r5       # get DLM ID from LTMT
        ldconst slld_PE_baddlmid,r9     # r9 default ec2 - bad dlm id
        cmpobne.f r4,r5,.msg_err        # JIf if not the same
        ldconst dgec1_slld_nopath,r8    # r8 default ec1 - no path
        ldconst 0,r9            # r9 = default invalid provider ID
        ld      ltmt_tmt(g4),r4         # r6 = assoc. TMT address
        cmpobe.f 0,r4,.msg_err          # JIf if no TMT assoc. with LTMT
        ldl     tm_P_name(r4),r4        # r4-r5 = target port name
        or      r4,r5,r4
        cmpobe  0,r4,.msg_err           # Jif target port name has been cleared
#
# --- make sure if there is an active IMT for this LTMT. if there is none return error
# --- this is very much possible in case of iSCSI
#     this should be done only for iscsi port TBolt00015792
#
# ---- get the cimt from ltmt
#
        ld    ltmt_cimt(g4),r4          # r4 = cimt
#
# ---- get port number from cimt
#
        ldob    ci_num(r4),r5           # r5 = interface #

c       r4 = ICL_IsIclPort((UINT8)r5);
        cmpobe  TRUE,r4,.msg_icl01      # Jif ICL
        ld      iscsimap,r4
        bbc     r5,r4,.msg_122          #jmp if it is not iSCSI port

.msg_icl01:
        ld      ltmt_imt(g4),r5         # r5 = assoc. IMT address
        ldconst dgec1_slld_noimt,r8       # r8 default ec1 - no imt
        cmpobe.f 0,r5,.msg_121          # Jif no IMT assoc. with LTMT
        ldob    im_flags(r5),r4           # r4 = MMC work flags byte
        bbs     im_flags_inactive,r4,.msg_121  # jmp if imt is inactive
        ld      im_ltmt(r5),r4          # r4 = assoc. LTMT address
        cmpobe.f 0,r4,.msg_121          # Jif no LTMT assoc with this IMT
        b .msg_122
#
.msg_121:
        b .msg_err
#
# --- make sure the target is a magnitude
#
.msg_122:
        ldob    ltmt_type(g4),r5        # get target type code
        ldconst dgec1_slld_PE,r8        # r8 default ec1 - parameter error
        ldconst slld_PE_notmag,r9       # r9 default ec2 - not magnitude
        cmpobne.f ltmt_ty_MAG,r5,.msg_err # Jif not a magnitude
#
# --- determine if initiator is online
#
        ld      ltmt_cimt(g4),r6        # r6 = cimt address
        ldob    ci_istate(r6),r5        # r5 = state code
        ldconst dgec1_slld_offl,r8      # r8 default ec1 - interface offline
        mov     0,r9                    # r9 default ec2 - 00
        cmpobne.f cis_on,r5,.msg_err    # Jif offline or uninitialized
#
# --- determine if a session is open
#
        ld      ltmt_seshead(g4),g5     # g5 = possible LSMT
        ldconst dgec1_slld_nopath,r8    # r8 default ec1 - no paths to destination
        cmpobe.f 0,g5,.msg_err          # JIf no sessions open
#
# --- build up the datagram request level in ILT'
#
        lda     msg_cr,r10              # r10 = completion routine
!       ld      sr_msg_reqbuf(g2),r4    # r4 = request buffer address
!       ld      sr_msg_reqlen(g2),r5    # r5 = request length
!       ld      sr_msg_respbuf(g2),r6   # r6 = response buffer address
!       ld      sr_msg_reslen(g2),r7    # r7 = response length
!       ldob    sr_msg_cmdto(g2),r12    # r12 = command time out
!       ldob    sr_msg_rtycnt(g2),r13   # r13 = command retry count
!       ldos    sg_scnt+sr_msg_esgl(g2),r8 # get the count of extended sgl's
        st      r10,il_cr(g1)           # save completion address
        stq     r4,sndg2_reqptr(g1)     # save buffer information
        stob    r12,sndg2_cmdto(g1)     # save command timeout
        stob    r13,sndg2_rtycnt(g1)    # save command retry count
        cmpobe  0,r8,.msg_160           # Jif there are no extended sgl's
        lda     sg_scnt+sr_msg_esgl(g2),r8 # r8 = extended SGL address
.msg_160:
        st      r8,sndg2_esgl(g1)       # Save extended SGL address (if one)
        st      g2,sndg2_srp(g1)        # save srp address
#
# --- send the datagram
#
        lda     ILTBIAS(g1),g1          # g1 = ILT' at 3rd level
        call    lld$snddg               # send the datagram
#
# --- exit
#
.msg_1000:
        ret
#
#**********************************************************************
#
#  NAME: msg_cr
#
#  PURPOSE:
#       Processes the completion for a send datagram call from lld$MSG.
#
#  CALLING SEQUENCE:
#        msg_cr
#
#  INPUT:
#       g1 = ILT/IRP address of CDB request at nest level 2
#       g4 = LTMT
#       g5 = LSMT
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Reg. g0-g12, g14 can be destroyed.
#
#**********************************************************************
#
msg_cr:
        ldconst srok,g0                 # set good srp completion status
        ld      sndg2_srp(g1),g2        # g2 = SRP addr
        lda     -ILTBIAS(g1),g1         # g1 = ILT/SRP at nest level #1
        ld      il_cr(g1),r8            # r4 = ILT completion handler routine
        callx   (r8)                    # call ILT/SRP completion handler routine
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: lld$FDD
#
#  PURPOSE:
#       Processes a Get Foreign Target Device Database SRP request.
#
#  CALLING SEQUENCE:
#       call    lld$FDD
#
#  INPUT:
#       g1 = ILT associated with SRP at nest level #2
#       g2 = SRP address to process
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Reg. g0-g12, g14 can be destroyed.
#
#**********************************************************************
#
lld$FDD:
        ld      sr_fdd_lldid(g2),r4     # r4 = link-level driver session ID (LTMT address)
        ld      sr_fdd_dlmid(g2),r5     # r5 = data-link manager session ID
        ld      ltmt_dlmid(r4),r6       # r6 = DLM session ID from LTMT
        ld      sr_fdd_ftdd(g2),g3      # g3 = FTDD address
        ldconst ftdd_recsz,r8           # r8 = LUN record size
        cmpobe.t r5,r6,.lldFDD_100      # Jif DLM IDs match
#
# --- DLM IDs do not match. Return error since something's wrong.
#
.lldFDD_40:
        ldconst srerr,g0                # g0 = SRP return status
        stob    r8,ftdd_rsize(g3)       # save LUN record length in FTDD
        mov     0,r9
        stob    r9,ftdd_luns(g3)        # indicate no LUNs defined
        stob    r9,ftdd_lunrecs(g3)     # indicate no LUN records defined
        lda     -ILTBIAS(g1),g1         # g1 = ILT/SRP at nest level #1
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT/SRP completion handler routine
        b       .lldFDD_1000            # and we're out of here!

.lldFDD_100:
        ldob    ltmt_type(r4),r5        # r5 = LTMT type code
        cmpobne.f ltmt_ty_FT,r5,.lldFDD_40 # Jif not a foreign target type
        ld      ltmt_tmt(r4),r5         # r5 = assoc. TMT address
        cmpobe.f 0,r5,.lldFDD_40        # Jif no TMT specified
#
# --- determine if this is a fabric LID and logon to the port if
#     required.
#

#
# --- set discovery parameters and start discovery scan
#
        lda     fdd_cr,r6               # r6 = completion address
        mov     g1,r15                  # r15 = ILT'

                                        # r14 = ILT completion handler routine
        call    M$ailtirp               # allocate an ILT/IRP combo (meal?)
                                        # g0 = IRP address
                                        # g1 = ILT address
        st      g0,lirp1_irp(g1)        # save IRP address in ILT
        st      r6,lirp1_cr(g1)         # save completion routine
        st      r4,lirp1_lldid(g1)      # save LTMT address in ILT
        st      r15,lirp1_g0(g1)        # save ILT address
        st      g2,lirp1_g1(g1)         # save SRP address
        lda     ILTBIAS(g1),g1          # g1 = ILT/IRP at nest level 2
        st      g0,lirp2_irp(g1)        # save IRP address in nest level 2
        ldconst rfc_discover,r6         # r4 = IRP request function code
        st      r6,irp_RFC(g0)          # save request function code
                                        #  and clear irp_cmplt, irp_tagID fields
        st      r4,irp_Req_ID(g0)       # save LTMT as requestor ID in IRP
        st      r5,irp_Pro_ID(g0)       # save provider ID in IRP
        call    APL$pfr                 # request CLOSE session

.lldFDD_1000:
        ret
#
#**********************************************************************
#
#  NAME: fdd_cr
#
#  PURPOSE:
#
#       Processes the completion for a Get Foreign Target Device Database
#       call from lld$MSG.
#
#  DESCRIPTION:
#
#FINISH
#
#  CALLING SEQUENCE:
#
#        fdd_cr
#
#  INPUT:
#
#       g1 = ILT/IRP address of discovery at nest level 1
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       Reg. g0-g12, g14 can be destroyed.
#
#**********************************************************************
#
fdd_cr:
        ld      lirp1_irp(g1),g0        # g0 = IRP for discovery request
        ld      lirp1_lldid(g1),r4      # r4 = assoc. LTMT address
        ld      lirp1_g0(g1),r11        # r11 = requestor's ILT address
        ld      lirp1_g1(g1),g2         # g2 = SRP address
        ld      sr_fdd_ftdd(g2),g3      # g3 = FTDD address

        ldob    irp_cmplt(g0),r5        # r5 = OPEN request completion status
#
# --- release ILT and IRP
#
.ifdef M4_DEBUG_IRP
c CT_history_printf("%s%s:%u put_irp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif # M4_DEBUG_IRP
c       put_irp(g0);                    # release IRP
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT

        mov     r11,g1                  # g1 = ILT'

        ld      ltmt_tmt(r4),r5         # r5 = assoc. TMT address
        lda     tm_tlmtdir(r5),r6       # r6 = pointer into TLMT directory
        ldconst ftdd_maxluns,r7         # r7 = max. # LUNs to check for
        ldconst ftdd_recsz,r8           # r8 = LUN record size
        mov     0,r9                    # r9 = LUN count
        lda     ftdd_hdr(g3),g4         # g4 = FTDD record pointer
        ldconst 0,r10                   # r10 = LUN #

.fddcr_200:
        ld      (r6),r15                # r15 = TLMT address (0=none)
        addo    4,r6,r6                 # inc. to next TLMT in directory
        cmpobe.t 0,r15,.fddcr_300       # Jif no TLMT defined
        stob    r10,ftdd_lnum(g4)       # save LUN # in FTDD
        ldq     tlm_sn(r15),g8          # g8-g11 = device serial #
        stq     g8,ftdd_sn(g4)          # save device serial #
        ldl     tlm_blkcnt(r15),r12     # r12/r13 = sector count
        stl     r12,ftdd_size(g4)       # save sector count
        ld      tlm_blksz(r15),r14      # r14 = sector size
        stos    r14,ftdd_secsz(g4)      # save sector size
        ldob    tlm_pdt(r15),r14        # r14 = device type code
        stob    r14,ftdd_dtype(g4)      # save device type code
        ldob    tlm_snlen(r15),r14      # r14 = serial # size
        stob    r14,ftdd_snlen(g4)      # save serial # length
        addo    1,r9,r9                 # inc. LUN count
        addo    r8,g4,g4                # inc. to next LUN record

.fddcr_300:
        addo    1,r10,r10               # inc. LUN # being processed
        subo    1,r7,r7                 # dec. LUN count
        cmpobne.t 0,r7,.fddcr_200       # Jif more LUNs to check
        stob    r9,ftdd_luns(g3)        # save # LUNs in FTDD header
        stob    r9,ftdd_lunrecs(g3)     # save # LUN records in FTDD header
        stob    r8,ftdd_rsize(g3)       # save LUN record size
        ldconst srok,g0                 # return good status

        lda     -ILTBIAS(g1),g1         # g1 = ILT/SRP at nest level #1
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT/SRP completion handler routine
        ret
#
#******************************************************************************
#
# ____________________ LTMT EVENT HANDLER TABLES ______________________________
#
#******************************************************************************
#
# --- MAGNITUDE link identified event handler table ---------------------------
#
        .data
ltmt_etbl1a:
        .byte   0                       # target state code
        .byte   0,0,0                   # reserved
        .word   e1a_offline             # offline event handler routine
        .word   e1a_online              # online event handler routine
        .word   e1a_tgone               # target disappeared event handler
                                        #  routine
        .word   e1a_mrecv               # message received from MAG event
                                        #  handler routine
        .word   e1a_getdd               # get device database event handler
                                        #  routine
#
# --- Foreign Target identified event handler table ---------------------------
#
ltmt_etbl2a:
        .byte   0                       # target state code
        .byte   0,0,0                   # reserved
        .word   e2a_offline             # offline event handler routine
        .word   e2a_online              # online event handler routine
        .word   e2a_tgone               # target disappeared event handler
                                        #  routine
        .word   exx_ignore              # message received from MAG event
                                        #  handler routine
        .word   e2a_getdd               # get device database event handler
#
                                        #  routine
        .text
#
#******************************************************************************
#
# ____________________ LTMT EVENT HANDLER ROUTINES ____________________________
#
#******************************************************************************
#
# --- Generic (not specific to a process state) event handler routines ---
#
#******************************************************************************
#
#
#******************************************************************************
#
#  NAME: exx_ignore
#
#  PURPOSE:
#
#       Ignores the event.
#
#  DESCRIPTION:
#
#       Simply returns to the caller effectively ignoring the event
#       being reported.
#
#  CALLING SEQUENCE:
#
#       call    exx_ignore
#
#  INPUT:
#
#       None. (actually it doesn't matter since we simply return
#               to the caller without doing any processing)
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
exx_ignore:
        ret                             # and we're out of here!
#
#******************************************************************************
#
# --- State specific LTMT event handler routines ------------------------------
#
#******************************************************************************
#
#
#******************************************************************************
#
#  NAME: e1a_offline
#
#  PURPOSE:
#
#       Performs the processing of an offline event on a MAGNITUDE
#       link.
#
#  DESCRIPTION:
#
#       Determines if the initiator driver state was offline and if so
#       doesn't do any further processing of the event. If this state
#       is something other then offline, places it in the offline state
#       and does any further offline processing on the target and it's
#       associated components.
#
#  CALLING SEQUENCE:
#
#       call    e1a_offline
#
#   INPUT:
#
#       g4 = assoc. LTMT address
#
#  OUTPUT:
#
#       None.
#
#  REGS. DESTROYED:
#
#       None.
#
#******************************************************************************
#
#
e1a_offline:
        ld      ltmt_cimt(g4),r4        # r4 = assoc. CIMT address
        cmpobe.f 0,r4,.e1aoff_800       # Jif no CIMT defined
        ldob    ci_istate(r4),r5        # r5 = initiator driver state code
        cmpobe.f cis_off,r5,.e1aoff_800 # Jif state already offline
        ldconst cis_off,r5              # r5 = new initiator driver state
        stob    r5,ci_istate(r4)        # set initiator state to offline

#
# Clean up outstanding requests that are queued (the outstanding requests
#   already issued will be terminated in other ways)
#
.e1aoff_800:
        call    lld$returnqdg
#
#*** FINISH - may need to call offline event handler routine for each
#               associated LSMT. May need to clear up any outstanding
#               send messages, etc.....
        ret
#
#******************************************************************************
#
#  NAME: e1a_online
#
#  PURPOSE:
#
#       Performs the processing of an online event on a MAGNITUDE
#       link that has not begun to establish the link with the
#       peer MAGNITUDE.
#
#  DESCRIPTION:
#
#       Checks MAGNITUDE link state to see if the link needs to be
#       established and if not simply returns to the caller. If
#       so, it allocates and initializes a LSMT for the session,
#       places it on the session list in the LTMT, builds and sends
#       an OPEN IRP request to the initiator driver to open a session
#       to the MAGNITUDE and sets up to establish the link to the
#       MAGNITUDE.
#
#  CALLING SEQUENCE:
#
#       call    e1a_online
#
#   INPUT:
#
#       g4 = assoc. LTMT address
#
#  OUTPUT:
#
#       None.
#
#  REGS. DESTROYED:
#
#       None.
#
#******************************************************************************
#
#
e1a_online:
        movq    g0,r12                  # save g0-g3
#
# --- Check if TMT associated with LTMT and if not just ignore the event.
#
        ld      ltmt_tmt(g4),r4         # r4 = assoc. TMT address
        cmpobe  0,r4,.e1aon_1000        # Jif no TMT assoc. with LTMT
#
# --- Check if IMT associated with LTMT and if not check for a match
#
        ld      ltmt_imt(g4),r4         # r4 = assoc. IMT address
        cmpobne.t 0,r4,.e1aon_50        # Jif IMT associated with LTMT already
        ld      ltmt_tmt(g4),r4         # r4 = assoc. TMT address
        ldl     tm_P_name(r4),r10       # r10-r11 = target MAC address
        ld      ltmt_cimt(g4),r4        # r4 = assoc. CIMT address
#
# --- Check active image list
#
        ld      ci_imthead(r4),r5       # r5 = first IMT on active list
        cmpobe.f 0,r5,.e1aon_50         # Jif no IMTs on active list

.e1aon_10:
        ld      im_ltmt(r5),r6          # r6 = assoc. LTMT address
        cmpobne.f 0,r6,.e1aon_12        # Jif LTMT assoc. with IMT already
        ldl     im_mac(r5),r6           # r6-r7 = MAC address of IMT
        cmpobne.t r6,r10,.e1aon_12      # Jif not a match
        cmpobne.t r7,r11,.e1aon_12      # Jif not a match

        PushRegs(r3)
        ldos    im_vpid(r5),g1          # r6 = virtual port ID of IMT
        ldob    ci_num(r4),g0           # r3 = interface #
        call    ISP_IsPrimaryPort
        mov     g0,r9
        PopRegsVoid(r3)
        cmpobe  0,r9,.e1aon_12         # Jif not the primary port
#
# --- Got a matching IMT for LTMT
#
        st      r5,ltmt_imt(g4)         # save IMT address in LTMT
        st      g4,im_ltmt(r5)          # save LTMT address in IMT
        b       .e1aon_50               # and continue on
#
.e1aon_12:
        ld      im_link(r5),r5          # r5 = next IMT on list
        cmpobne.t 0,r5,.e1aon_10        # Jif more IMTs to check on active list
.e1aon_50:
        ldob    ltmt_lst(g4),r4         # r4 = link state code
        cmpobne.f ltmt_lst_id,r4,.e1aon_1000 # Jif link state not target
                                        #  identified
        ldconst ltmt_lst_pest,r4        # r4 = new link state code
        stob    r4,ltmt_lst(g4)         # save new link state code
c       g3 = get_lsmt();                # allocate a LSMT for Controller link
        st 0,lsmt_link(g3)              #clear forward link
.ifdef M4_DEBUG_LSMT
c fprintf(stderr, "%s%s:%u get_lsmt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g3);
.endif # M4_DEBUG_LSMT
        ldconst 0,g0                    # g0 = LUN # to open session for
        stos    g0,lsmt_lun(g3)         # save LUN # in LSMT
        st      g4,lsmt_ltmt(g3)        # save assoc. LTMT address in LSMT
        lda     lsmt_etbl1a,r4          # r4 = session event handler table
        st      r4,lsmt_ehand(g3)       # save event handler table in LSMT
#
# --- Put LSMT onto LTMT session list
#
        ld      ltmt_sestail(g4),r4     # r4 = last LSMT on session list
        cmpobne.t 0,r4,.e1aon_100       # Jif list not empty
        st      g3,ltmt_seshead(g4)     # save LSMT as new head member
        b       .e1aon_150
#
.e1aon_100:
        st      g3,lsmt_link(r4)        # link LSMT onto end of list
.e1aon_150:
        st      g3,ltmt_sestail(g4)     # put new LSMT as new tail member
#
# --- Set up to send open request to initiator driver
#
        movq    g4,r8                   # save g4-g7
        lda     e1aopen_cr,g2           # g2 = open ILT completion handler
                                        #  routine
        ldconst 16,g5                   # g5 = error retry count
        lda     LLD$session_term,g6     # g6 = session termination routine
        call    lld$sendopen            # open session for MAGNITUDE link
                                        #  g0 = LUN # to open session to
                                        #  g1 = ILT/SRP at 2nd lvl
                                        #  g2 = ILT completion handler routine
                                        #       address
                                        #  g3 = LSMT associated with session
                                        #       being opened
                                        #  g4 = assoc. LTMT address
                                        #  g5 = error retry count
                                        #  g6 = session termination routine adr
        movq    r8,g4                   # restore g4-g7

.e1aon_1000:
        movq    r12,g0                  # restore g0-g3
        ret
#
#******************************************************************************
#
#  NAME: e1aopen_cr
#
#  PURPOSE:
#
#       Processes completion of the open IRP request for the MAGNITUDE
#       link session.
#
#  DESCRIPTION:
#
#       Checks if ILT/IRP is registered with LSMT. If not, ILT/IRP
#       was aborted. Just return ILT & IRP and return to caller. If
#       ILT/IRP is registered to LSMT, checks if OPEN request failed.
#       If it failed, returns ILT & IRP and sets up to retry OPEN
#       request after a time delay. If the OPEN request succeeded,
#       sends a MAGNITUDE link INQUIRY CDB to the peer MAGNITUDE.
#
#  CALLING SEQUENCE:
#
#       call    e1aopen_cr
#
#   INPUT:
#
#       g1 = ILT/IRP address of OPEN at nest level 1
#
#  OUTPUT:
#
#       None.
#
#  REGS. DESTROYED:
#
#       None.
#
#******************************************************************************
#
#
e1aopen_cr:
        movq    g0,r12                  # save g0-g3
        ld      lirp1_irp(g1),g0        # g0 = IRP for OPEN request
        ld      lirp1_lldid(g1),g3      # g3 = assoc. LSMT address
        ld      lsmt_ilt(g3),r4         # r4 = assoc. ILT/IRP from LSMT
        ldob    irp_cmplt(g0),r5        # r5 = OPEN request completion status
        cmpobe.t g1,r4,.e1aopencr_200   # Jif ILT registered in LSMT
#
# --- ILT/IRP not registered in LSMT
#
.ifdef M4_DEBUG_IRP
c CT_history_printf("%s%s:%u put_irp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif # M4_DEBUG_IRP
c       put_irp(g0);                    # release IRP
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
        b       .e1aopencr_1000         # and get out
#
# --- ILT/IRP still registered in LSMT
#
.e1aopencr_200:
        cmpobe.t cmplt_success,r5,.e1aopencr_300 # Jif OPEN successful
#
# --- OPEN request failed
#
        ld      lirp1_g1(g1),r6         # r6 = error retry count
        cmpobe.f 0,r6,.e1aopencr_270    # Jif error retry count has expired
        subo    1,r6,r6                 # dec. error retry count
        st      r6,lirp1_g1(g1)         # save updated error retry count
        lda     e1aopen_rty,g2          # g2 = I/O operation retry
                                        #  continuation routine address
        call    lld$io_retry            # schedule to retry I/O operation
        b       .e1aopencr_1000         # and wait for the retry operation
                                        #  to occur
#
# --- OPEN retry count has expired. Terminate the target.
#
.e1aopencr_270:
        mov     0,r3
        st      r3,lsmt_ilt(g3)         # clear assoc. ILT field in LSMT
.ifdef M4_DEBUG_IRP
c CT_history_printf("%s%s:%u put_irp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif # M4_DEBUG_IRP
c       put_irp(g0);                    # release IRP
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
#
# --- Close out session for target
#
        movl    g4,r10                  # save g4-g5
        ld      lsmt_ltmt(g3),g4        # g4 = assoc. LTMT address
        call    lld$rem_lsmt            # remove LSMT from session list in LTMT
.ifdef M4_DEBUG_LSMT
c fprintf(stderr, "%s%s:%u put_lsmt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g3);
.endif # M4_DEBUG_LSMT
c       put_lsmt(g3);                   # Deallocate LSMT
#
# --- Terminate target
#
        ld      ltmt_tmt(g4),g5         # g5 = assoc. TMT address
        call    LLD$target_GONE         # terminate target
        movl    r10,g4                  # restore g4-g5
        b       .e1aopencr_1000         # and we're out of here!
#
# --- OPEN request succeeded
#
.e1aopencr_300:
        ldconst 0,r3
        st      r3,lsmt_ilt(g3)         # clear assoc. ILT field in LSMT
        ld      irp_Pro_ID(g0),r6       # r6 = initiator ID
        st      r6,lsmt_psid(g3)        # save initiator ID in LSMT
        ld      lsmt_ltmt(g3),r4        # r4 = assoc. LTMT address
#        ldconst ltmt_lst_est,r5         # r5 = new MAG link state
        stob    r5,ltmt_lst(r4)         # save new MAG link state code in LTMT
.ifdef M4_DEBUG_IRP
c CT_history_printf("%s%s:%u put_irp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif # M4_DEBUG_IRP
c       put_irp(g0);                    # release IRP
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
        mov     0,g0                    # g0 = lirp1_g0 value
        ldconst 16,g1                   # g1 = error retry count
        lda     e1ainfo_cr,g2           # g2 = MAG link INQUIRY ILT completion
                                        #  handler routine
        call    lld$sendinfo            # send MAG link INQUIRY for peer
                                        #  information page data
.e1aopencr_1000:
        movq    r12,g0                  # restore g0-g3
        ret
#
#******************************************************************************
#
#  NAME: e1aopen_rty
#
#  PURPOSE:
#
#       Processes the retry of the open IRP request for the MAGNITUDE
#       link session.
#
#  DESCRIPTION:
#
#       Checks if ILT/IRP is registered with LSMT. If not, ILT/IRP
#       was aborted. Just return ILT & IRP and return to caller. If
#       ILT/IRP is registered to LSMT, sets up to retry opening the session.
#
#  CALLING SEQUENCE:
#
#       call    e1aopen_rty
#
#   INPUT:
#
#       g1 = ILT/IRP address of OPEN at nest level 2
#
#  OUTPUT:
#
#       None.
#
#  REGS. DESTROYED:
#
#       None.
#
#******************************************************************************
#
#
e1aopen_rty:
        movq    g0,r12                  # save g0-g3
        lda     -ILTBIAS(g1),r11        # r11 = I/O ILT at nest level 1
        ld      lirp1_irp(r11),g0       # g0 = IRP for OPEN request
        ld      lirp1_lldid(r11),g3     # g3 = assoc. LSMT address
        ld      lsmt_ilt(g3),r4         # r4 = assoc. ILT/IRP from LSMT
        cmpobe.t r11,r4,.e1aopenrty_200 # Jif ILT registered in LSMT
#
# --- ILT/IRP not registered in LSMT
#
        mov     r11,g1                  # g1 = ILT at nest level 1
        call    e1aopen_cr              # terminate operation
        b       .e1aopenrty_1000        # and get out
#
# --- ILT/IRP still registered in LSMT
#
.e1aopenrty_200:
        ldconst 0,r3
        stob    r3,irp_cmplt(g0)        # clear IRP completion status field
        st      r3,irp_extcmp(g0)       # clear IRP extended completion
                                        #  status field
        call    APL$pfr                 # request OPEN session again
.e1aopenrty_1000:
        movq    r12,g0                  # restore g0-g3
        ret
#
#******************************************************************************
#
#  NAME: e1ainfo_cr
#
#  PURPOSE:
#
#       Processes completion of the peer MAGNITUDE link INQUIRY
#       CDB request for the peer information page data for the
#       MAGNITUDE link session.
#
#  DESCRIPTION:
#
#       Checks if ILT/IRP is registered with LSMT. If not, ILT/IRP
#       was aborted. Just return SGL/buffer, ILT & IRP and return
#       to caller. If ILT/IRP is registered to LSMT, checks if CDB
#       request failed. If it failed, returns SGL/buffer, ILT & IRP
#       and sets up to retry the CDB request after a time delay.
#       If the CDB request succeeded, validates the INQUIRY data
#       received from the peer MAGNITUDE and if not valid sets up to
#       retry the CDB request after a time delay. If valid, saves the
#       information off in the LTMT and sends a MAGNITUDE Link
#       Established VRP to the data-link manager.
#
#  CALLING SEQUENCE:
#
#       call    e1ainfo_cr
#
#   INPUT:
#
#       g1 = ILT/IRP address of CDB request at nest level 1
#
#  OUTPUT:
#
#       None.
#
#  REGS. DESTROYED:
#
#       None.
#
#******************************************************************************
#
#
e1ainfo_cr:
        ldconst 0,r7                    # r7 will be 0
        movq    g0,r12                  # save g0-g3
        ld      lirp1_irp(g1),r11       # r11 = IRP for CDB request
        ld      lirp1_lldid(g1),g3      # g3 = assoc. LSMT address
        ld      lsmt_ilt(g3),r4         # r4 = assoc. ILT/IRP from LSMT
        ldob    irp_cmplt(r11),r5       # r5 = CDB request completion status
        cmpobe.t g1,r4,.e1ainfocr_100   # Jif ILT registered in LSMT
#
# --- ILT/IRP not registered in LSMT
#
        ld      irp_SGLptr(r11),g0      # g0 = assoc. SGL/buffer
        st      0,irp_SGLptr(r11)       # make sure cannot use sgl pointer again
        call    M$rsglbuf               # release SGL/buffer combo
        mov     r11,g0                  # g0 = IRP address
.ifdef M4_DEBUG_IRP
c CT_history_printf("%s%s:%u put_irp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif # M4_DEBUG_IRP
c       put_irp(g0);                    # release IRP
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
        b       .e1ainfocr_1000         # and get out
#
# --- ILT/IRP still registered in LSMT
#
.e1ainfocr_100:
        st      r7,lsmt_ilt(g3)         # clear assoc. ILT field in LSMT
        cmpobe.t cmplt_success,r5,.e1ainfocr_300 # Jif CDB successful
#
# --- determine if this is a check status error
#
        cmpobne.f cmplt_LOUT,r5,.e1ainfocr_200 # JIf not "Logout" error
        ld      lsmt_ltmt(g3),g4        # r4 = assoc. LTMT address
        ldconst ltmt_lst_term,r4        # r4 = new link state code
        stob    r4,ltmt_lst(g4)         # save new link state code
        b       .e1ainfocr_205
#
# --- Information INQUIRY CDB request failed
#
.e1ainfocr_200:
        cmpobne.f cmplt_CSE,r5,.e1ainfocr_250 # Jif not check status error
#
# --- MAGNITUDE node. Check if it doesn't support MAG-to-MAG link
#       protocol and if it doesn't, treat as a Foreign Target.
#
        ldob    irp_extcmp(r11),r5      # r5 = ASC from SENSE data
        ldconst 0x24,r6                 # r6 = ASC code to check for
        cmpobne.f r5,r6,.e1ainfocr_250  # Jif not a match
        ldob    irp_extcmp+1(r11),r5    # r5 = ASCQ from SENSE data
        ldconst 0x00,r6                 # r6 = ASCQ code to check for
        cmpobne.f r5,r6,.e1ainfocr_250  # Jif not a match
#
        ld      lsmt_ltmt(g3),g4        # r4 = assoc. LTMT address
        ldconst ltmt_lst_est,r5         # r5 = new MAG link state
        stob    r5,ltmt_lst(g4)         # save new MAG link state code in LTMT
#
# --- Process to treat MAGNITUDE node as a Foreign Target.
#
.e1ainfocr_205:
        ld      irp_SGLptr(r11),g0      # g0 = assoc. SGL/buffer
        st      0,irp_SGLptr(r11)       # make sure cannot use sgl pointer again
        call    M$rsglbuf               # release SGL/buffer combo
        mov     r11,g0                  # g0 = IRP address
.ifdef M4_DEBUG_IRP
c CT_history_printf("%s%s:%u put_irp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif # M4_DEBUG_IRP
c       put_irp(g0);                    # release IRP
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
        mov     g4,r3                   # save g4
        ld      lsmt_ltmt(g3),g4        # g4 = assoc. LTMT address
        ld      lsmt_ehand(g3),r4       # r4 = LSMT event handler table
        ld      lsmt_eh_term(r4),r5     # r5 = session termination event handler routine
        callx   (r5)                    # call session termination event handler routine
#
# --- Determine if MAGNITUDE Link Terminated VRP needs to be sent and send
#
        ldob    ltmt_dlmst(g4),r4       # r4 = DLM state code
        cmpobe.f ltmt_dst_null,r4,.e1ainfocr_210 # Jif link not identified
                                        #  to DLM
        call    lld$send_MLT            # send MAG link terminated VRP
        st      r7,ltmt_dlmid(g4)       # clear DLM session ID from LTMT

#
# --- Check if ILT associated with target and flush if so.
#
.e1ainfocr_210:
        ld      ltmt_ilt(g4),r4         # r4 = assoc. ILT if defined
        cmpobe.t 0,r4,.e1ainfocr_220    # Jif no ILT assoc. with LTMT
#
# --- Clearing the ILT address from the LTMT essentially flushes the
#       ILT because ILT completion handler routines check if the ltmt_ilt
#       address in the LTMT matches the ILT and if it doesn't it
#       does not perform the normal completion handler processing.
#
        st      r7,ltmt_ilt(g4)         # clear ILT from LTMT
#
# --- Break relationship between LTMT/IMT if necessary.
#
.e1ainfocr_220:
        ld      ltmt_imt(g4),r4         # r4 = assoc. IMT address
        cmpobe.f 0,r4,.e1ainfocr_230    # Jif no IMT assoc. with LTMT
        mov     g5,r5                   # save g5
        mov     g6,r7                   # save g6
        mov     r4,g5
        ld      im_ehand(g5),r6         # r6 =  event handler table
        ld      dd_reset(r6),r6         # r6 =  reset event handler routine
        ld      im_cimt(g5),g4          # icimt
        callx   (r6)                    # call default event routine
                                        # input:
                                        #       g4 = CIMT address
                                        #       g5 = IMT address
                                        # output:
                                        #       none
                                        #       r3-r15/g6 can be destroyed.
        mov     r5,g5                   # restore g5
        mov     r7,g6                   # restore g6
? # crash - cqt# 24600 - 2008-12-01 - FE LSMT - failed @ lld.as:3673  ld 8+g3,g4 with fefefefe - workaround?
        ld      lsmt_ltmt(g3),g4        # g4 = assoc. LTMT address
        cmpobe  0,g4,.e1ainfocr_240     # Jump if LSMT free
        ldconst 0,r7                    # r7 will be 0
        st      r7,ltmt_imt(g4)         # remove IMT from LTMT
        ld      im_ltmt(r4),r5          # r5 = assoc. LTMT from IMT
        cmpobne.f r5,g4,.e1ainfocr_230  # Jif IMT not assoc. with LTMT
        st      r7,im_ltmt(r4)          # remove LTMT from IMT
#
#*** FINISH - may need to do more to demolish the IMT/ILMT relationship
#
# --- Change LTMT over to a Foreign Target.
#
.e1ainfocr_230:
        ld      targID_FT1,r6           # load type, link state, DLM state codes
        st      r6,ltmt_type(g4)        # save type, link state, DLM state codes
        lda     ltmt_etbl2a,r6          # r6 = LTMT event handler table
        st      r6,ltmt_ehand(g4)       # save event handler table
#
# --- Call online event handler routine to start link establish process
#
        ld      ltmt_eh_online(r6),r7   # r7 = online event handler routine
        callx   (r7)                    # and call online handler routine
.e1ainfocr_240:
        mov     r3,g4                   # restore g4
        b       .e1ainfocr_1000         # and we're out of here!
#
.e1ainfocr_250:
#
# --- Information INQUIRY request failed
#
        ld      lirp1_g1(g1),r6         # r6 = error retry count
        cmpobe.f 0,r6,.e1ainfocr_205    # Jif error retry count has expired
        st      g1,lsmt_ilt(g3)         # save assoc. ILT in LSMT
        subo    1,r6,r6                 # dec. error retry count
        st      r6,lirp1_g1(g1)         # save updated error retry count
        lda     e1ainfo_rty,g2          # g2 = I/O operation retry
                                        #  continuation routine address
        call    lld$io_retry            # schedule to retry I/O operation
        b       .e1ainfocr_1000         # and wait for the retry operation
                                        #  to occur
#
# --- Information INQUIRY CDB request succeeded
#
.e1ainfocr_300:
        ld      lsmt_ltmt(g3),r4        # r4 = assoc. LTMT address
        ld      irp_SGLptr(r11),g0      # g0 = assoc. SGL/buffer
        ld      sg_desc0+sg_addr(g0),r10 # r10 = assoc. buffer address
        ld      inqinfo_bnr(r10),r9     # r9 = info. page banner
        ld      info_bnr,r8             # r8 = banner to compare to
        cmpobne.f r8,r9,.e1ainfocr_250  # Jif banner does not match
        ld      inqinfo_sn(r10),r9      # r9 = MAGNITUDE serial number
        bswap   r9,r9                   # put in little endian format
        st      r9,ltmt_sn(r4)          # save serial number in LTMT
        ld      inqinfo_alias(r10),r9   # r9 = alias node serial number
        bswap   r9,r9                   # put in little endian format
        st      r9,ltmt_alias(r4)       # save alias node serial number
        ldob    inqinfo_path(r10),r9    # r9 = Path # in MAGNITUDE
        stob    r9,ltmt_path(r4)        # save path # in LTMT
        ldob    inqinfo_cl(r10),r9      # r9 = assigned cluster #
        stob    r9,ltmt_cl(r4)          # save assigned cluster # in LTMT
        ldob    inqinfo_vdcnt(r10),r9   # r9 = # VDisks defined
        stob    r9,ltmt_vdcnt(r4)       # save VDisk count
        ld      inqinfo_ip(r10),r9      # r9 = assigned IP address
        st      r9,ltmt_ip(r4)          # save assigned IP address in LTMT
        ldl     inqinfo_name(r10),r8    # r8-r9 = assigned node name
        stl     r8,ltmt_pname(r4)       # save assigned node name in LTMT
        ldob    inqinfo_flag1(r10),r9   # r9 = flag byte #1
        stob    r9,ltmt_flag1(r4)       # save flag byte #1 in LTMT
        st      0,irp_SGLptr(r11)       # make sure cannot use sgl pointer again
        call    M$rsglbuf               # release SGL/buffer combo
        mov     r11,g0                  # g0 = IRP address
.ifdef M4_DEBUG_IRP
c CT_history_printf("%s%s:%u put_irp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif # M4_DEBUG_IRP
c       put_irp(g0);                    # release IRP
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
        mov     g4,r11                  # save g4
        mov     r4,g4                   # g4 = LTMT of MAGNITUDE link
#
# --- Check if target node is operating in target-only mode
#       and if so set up the local environment appropriately.
#
#       r9 = inqinfo_flag1 byte
#
        ldconst ltmt_lst_est,r5         # r5 = new MAG link state
        stob    r5,ltmt_lst(g4)         # save new MAG link state code in LTMT
        ld      ltmt_imt(g4),r5         # r5 = assoc. IMT address
        cmpobne 0,r5,.e1ainfocr_500     # Jif IMT already assoc. with LTMT
        bbc     0,r9,.e1ainfocr_500     # Jif target not operating in
                                        #  target-only mode
        call    lld$remote_to_mode      # set up remote node operating in
                                        #  target-only mode
.e1ainfocr_500:
        call    lld$send_MLE            # send MAG link established VRP to
                                        #  MAGNITUDE CPU
        mov     r11,g4                  # restore g4
.e1ainfocr_1000:
        movq    r12,g0                  # restore g0-g3
        ret
#
#******************************************************************************
#
#  NAME: e1ainfo_rty
#
#  PURPOSE:
#
#       Processes the retry of the Information INQUIRY IRP request for
#       the MAGNITUDE link session.
#
#  DESCRIPTION:
#
#       Checks if ILT/IRP is registered with LSMT. If not, ILT/IRP
#       was aborted. Just return ILT & IRP and return to caller. If
#       ILT/IRP is registered to LSMT, sets up to retry the Information
#       INQUIRY IRP I/O request.
#
#  CALLING SEQUENCE:
#
#       call    e1ainfo_rty
#
#   INPUT:
#
#       g1 = ILT/IRP address of Information INQUIRY at nest level 2
#
#  OUTPUT:
#
#       None.
#
#  REGS. DESTROYED:
#
#       None.
#
#******************************************************************************
#
#
e1ainfo_rty:
        movq    g0,r12                  # save g0-g3
        lda     -ILTBIAS(g1),r11        # r11 = I/O ILT at nest level 1
        ld      lirp1_irp(r11),g0       # g0 = IRP for INQUIRY request
        ld      lirp1_lldid(r11),g3     # g3 = assoc. LSMT address
? # crash - cqt# 24299 - 2008-11-10 -- FE LSMT - failed @ ld 32+g3,r4 gave 0xfefefefe - workaround?
        ld      lsmt_ilt(g3),r4         # r4 = assoc. ILT/IRP from LSMT
        cmpobe.t r11,r4,.e1ainforty_200 # Jif ILT registered in LSMT
#
# --- ILT/IRP not registered in LSMT
#
        mov     r11,g1                  # g1 = ILT/IRP at nest level 1
        call    e1ainfo_cr              # terminate operation
        b       .e1ainforty_1000        # and get out
#
# --- ILT/IRP still registered in LSMT
#
.e1ainforty_200:
        ldconst 0,r3
        stob    r3,irp_cmplt(g0)        # clear IRP completion status field
        st      r3,irp_extcmp(g0)       # clear IRP extended completion
                                        #  status field
        call    APL$pfr                 # request Information INQUIRY again
.e1ainforty_1000:
        movq    r12,g0                  # restore g0-g3
        ret
#
#******************************************************************************
#
#  NAME: e1a_tgone
#
#  PURPOSE:
#
#       Performs the processing of a target disappeared event on a
#       MAGNITUDE link that has not begun to establish the link with
#       the peer MAGNITUDE.
#
#  DESCRIPTION:
#
#       Determines if MAGNITUDE Link Terminated VRP needs to be sent
#       and sends it if necessary. Checks if an ILT is associated
#       with LTMT and flushes it if so. Breaks the relationship
#       between the LTMT and an IMT if necessary. Breaks the
#       relationship between the LTMT and a TMT if necessary. Removes
#       the LTMT from the LTMT list maintained in the CIMT and
#       deallocates the LTMT back into the spare LTMT pool.
#
#  CALLING SEQUENCE:
#
#       call    e1a_tgone
#
#   INPUT:
#
#       g4 = assoc. LTMT address
#       g5 = assoc. TMT address
#
#  OUTPUT:
#
#       None.
#
#  REGS. DESTROYED:
#
#       None.
#
#******************************************************************************
#
#
e1a_tgone:
        mov     0,r3
        movl    g2,r14                  # save g2-g3
#
# --- Determine if MAGNITUDE Link Terminated VRP needs to be sent and send
#
        ldob    ltmt_dlmst(g4),r4       # r4 = DLM state code
        cmpobe.f ltmt_dst_null,r4,.e1atgone_100 # Jif link not identified to DLM
        call    lld$send_MLT            # send MAG link terminated VRP
        st      r3,ltmt_dlmid(g4)       # clear DLM session ID from LTMT
#
# --- Check if ILT associated with target and flush if so.
#
.e1atgone_100:
        ld      ltmt_ilt(g4),r4         # r4 = assoc. ILT if defined
        cmpobe.t 0,r4,.e1atgone_200     # Jif no ILT assoc. with LTMT
#
# --- Clearing the ILT address from the LTMT essentially flushes the
#       ILT because ILT completion handler routines check if the ltmt_ilt
#       address in the LTMT matches the ILT and if it doesn't it
#       does not perform the normal completion handler processing.
#
        st      r3,ltmt_ilt(g4)         # clear ILT from LTMT
#
# --- Break relationship between LTMT/IMT if necessary.
#
.e1atgone_200:
        ld      ltmt_imt(g4),r4         # r4 = assoc. IMT address
        cmpobe.f 0,r4,.e1atgone_300     # Jif no IMT assoc. with LTMT
        st      r3,ltmt_imt(g4)         # remove IMT from LTMT
        ld      im_ltmt(r4),r5          # r5 = assoc. LTMT from IMT
        # cmpobne.f r5,g4,.e1atgone_300   # Jif IMT not assoc. with LTMT
        cmpobe.t r5,g4,.e1atgone_210   # Jif IMT assoc. with LTMT
c fprintf(stderr, "%s%s:%u RAGX: .e1atgone_200: ltmt(0x%08lx) does not match with imt(0x%08lx) im_ltmt(0x%08lx)\n", FEBEMESSAGE, __FILE__, __LINE__,g4,r4,r5);
        b       .e1atgone_300           # continue
.e1atgone_210:
        st      r3,im_ltmt(r4)          # remove LTMT from IMT
#
#
#*** FINISH - may need to do more to demolish the IMT/ILMT relationship
#
# --- Terminate LTMT.
#
.e1atgone_300:

        ld      ltmt_seshead(g4),g3     # get possible LSMT
        cmpobe.f 0,g3,.e1atgone_310     # JIf no sessions open

        lda     s1aclose_cr,g2          # g2 = close completion routine
        call    lld$sendclose           # build and send for MAG session
#
# --- Close out session for target
#
        mov     0,r3
        st      r3,lsmt_dlmid(g3)       # clear out DLM ID from LSMT
        call    lld$rem_lsmt            # remove LSMT from session list in LTMT
.ifdef M4_DEBUG_LSMT
c fprintf(stderr, "%s%s:%u put_lsmt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g3);
.endif # M4_DEBUG_LSMT
c       put_lsmt(g3);                   # Deallocate LSMT
        b       .e1atgone_300           # continue
#
# --- all sessions have been closed, now release the LTMT


.e1atgone_310:
        call    lld$dem_ltmt            # demolish LTMT
        movl    r14,g2                  # restore g2-g3
        ret
#
#******************************************************************************
#
#  NAME: e1a_mrecv
#
#  PURPOSE:
#
#       Performs the processing of a message received event on a
#       MAGNITUDE link that has not begun to establish the link with
#       the peer MAGNITUDE.
#
#  DESCRIPTION:
#
#       FINISH
#
#  CALLING SEQUENCE:
#
#       call    e1a_mrecv
#
#   INPUT:
#
#       FINISH
#
#  OUTPUT:
#
#       None.
#
#  REGS. DESTROYED:
#
#       None.
#
#******************************************************************************
#
#
e1a_mrecv:
#*** FINISH
        ret
#
#******************************************************************************
#
#  NAME: e1a_getdd
#
#  PURPOSE:
#
#       Performs the processing of a get device database event on a
#       MAGNITUDE link that has not begun to establish the link with
#       the peer MAGNITUDE.
#
#  DESCRIPTION:
#
#       FINISH
#
#  CALLING SEQUENCE:
#
#       call    e1a_getdd
#
#   INPUT:
#
#       FINISH
#
#  OUTPUT:
#
#       None.
#
#  REGS. DESTROYED:
#
#       None.
#
#******************************************************************************
#
#
e1a_getdd:
#*** FINISH
        ret
#
#******************************************************************************
#
#  NAME: e2a_offline
#
#  PURPOSE:
#
#       Performs the processing of an offline event on a Foreign
#       Target.
#
#  DESCRIPTION:
#
#       FINISH
#
#  CALLING SEQUENCE:
#
#       call    e2a_offline
#
#   INPUT:
#
#       g4 = assoc. LTMT address
#
#  OUTPUT:
#
#       None.
#
#  REGS. DESTROYED:
#
#       None.
#
#******************************************************************************
#
#
e2a_offline:
        ld      ltmt_cimt(g4),r4        # r4 = assoc. CIMT address
        cmpobe.f 0,r4,.e2aoff_800       # Jif no CIMT defined
        ldob    ci_istate(r4),r5        # r5 = initiator driver state code
        cmpobe.f cis_off,r5,.e2aoff_800 # Jif state already offline
        ldconst cis_off,r5              # r5 = new initiator driver state
        stob    r5,ci_istate(r4)        # set initiator state to offline
#
# Clean up outstanding requests that are queued (the outstanding requests
#   already issued will be terminated in other ways)
#
.e2aoff_800:
        call    lld$returnqdg
#
#*** FINISH - may need to call offline event handler routine for each
#               associated LSMT. May need to clear up any outstanding
#               send messages, etc.....
        ret
#
#******************************************************************************
#
#  NAME: e2a_online
#
#  PURPOSE:
#
#       Performs the processing of an online event on a Foreign
#       Target.
#
#  DESCRIPTION:
#
#       FINISH
#
#  CALLING SEQUENCE:
#
#       call    e2a_online
#
#   INPUT:
#
#       g4 = assoc. LTMT address
#
#  OUTPUT:
#
#       None.
#
#  REGS. DESTROYED:
#
#       None.
#
#******************************************************************************
#
#
e2a_online:
        ldob    ltmt_dlmst(g4),r4       # r4 = DLM state code
        cmpobne.f ltmt_dst_null,r4,.e2aon_1000 # Jif DLM state not null
        ldconst ltmt_dst_pid,r4         # r4 = new DLM state code
        stob    r4,ltmt_dlmst(g4)       # save new DLM state code
        call    lld$send_FTI            # send Foreign Target ID VRP
.e2aon_1000:
        ret
#
#******************************************************************************
#
#  NAME: e2a_tgone
#
#  PURPOSE:
#
#       Performs the processing of a target disappeared event on a
#       Foreign Target.
#
#  DESCRIPTION:
#
#       FINISH
#
#  CALLING SEQUENCE:
#
#       call    e2a_tgone
#
#   INPUT:
#
#       g4 = assoc. LTMT address
#       g5 = assoc. TMT address
#
#  OUTPUT:
#
#       None.
#
#  REGS. DESTROYED:
#
#       None.
#
#******************************************************************************
#
#
e2a_tgone:
#
# --- Determine if Foreign Target Terminated VRP needs to be sent and send
#
        mov     0,r3
        ldob    ltmt_dlmst(g4),r4       # r4 = DLM state code
        cmpobe.f ltmt_dst_null,r4,.e2atgone_100 # Jif link not identified to DLM
        call    lld$send_FTT            # send foreign target terminated VRP
        st      r3,ltmt_dlmid(g4)       # clear DLM session ID from LTMT
#
# --- Check if ILT associated with target and flush if so.
#
.e2atgone_100:
        ld      ltmt_ilt(g4),r4         # r4 = assoc. ILT if defined
        cmpobe.t 0,r4,.e2atgone_300     # Jif no ILT assoc. with LTMT
#
# --- Clearing the ILT address from the LTMT essentially flushes the
#       ILT because ILT completion handler routines check if the ltmt_ilt
#       address in the LTMT matches the ILT and if it doesn't it
#       does not perform the normal completion handler processing.
#
        st      r3,ltmt_ilt(g4)         # clear ILT from LTMT
#
# --- Terminate LTMT.
#
.e2atgone_300:
        call    lld$dem_ltmt            # demolish LTMT
        ret
#
#******************************************************************************
#
#  NAME: e2a_getdd
#
#  PURPOSE:
#
#       Performs the processing of a get device database event on a
#       Foreign Target.
#
#  DESCRIPTION:
#
#       FINISH
#
#  CALLING SEQUENCE:
#
#       call    e2a_getdd
#
#   INPUT:
#
#       FINISH
#
#  OUTPUT:
#
#       None.
#
#  REGS. DESTROYED:
#
#       None.
#
#******************************************************************************
#
#
e2a_getdd:
#*** FINISH
        ret
#
#******************************************************************************
#
# ____________________ LSMT EVENT HANDLER TABLES ______________________________
#
#******************************************************************************
#
# --- MAGNITUDE link identified event handler table ---------------------------
#
        .data
lsmt_etbl1a:
        .byte   0                       # session state code
        .byte   0,0,0                   # reserved
        .word   s1a_term                # terminate session event handler
                                        #  routine
        .word   s1a_smsg                # send message event handler routine
        .word   s1a_vrp                 # process VRP event handler routine
#
# --- Foreign Target linked device session event handler table ----------------
#
lsmt_etbl2a:
        .byte   0                       # session state code
        .byte   0,0,0                   # reserved
        .word   s1a_term                # terminate session event handler
                                        #  routine
        .word   s2a_smsg                # send message event handler routine
        .word   s1a_vrp                 # process VRP event handler routine
#
        .text
#
#******************************************************************************
#
# ____________________ LSMT EVENT HANDLER ROUTINES ____________________________
#
#******************************************************************************
#
#******************************************************************************
#
# --- State specific LSMT event handler routines ------------------------------
#
#******************************************************************************
#
#
#******************************************************************************
#
#  NAME: s1a_term
#
#  PURPOSE:
#
#       Performs the processing of a terminate session event on a
#       MAGNITUDE link associated session.
#
#  DESCRIPTION:
#
#*** FINISH
#
#  CALLING SEQUENCE:
#
#       call    s1a_term
#
#   INPUT:
#
#       g3 = assoc. LSMT address
#
#  OUTPUT:
#
#       None.
#
#  REGS. DESTROYED:
#
#       None.
#
#******************************************************************************
#
#
s1a_term:
        movq    g0,r12                  # save g0-g3
        mov     g4,r3                   # save g4
        ld      lsmt_ltmt(g3),g4        # g4 = assoc. LTMT address

#
#*** FINISH - may need to add logic to check for outstanding Send Messages
#               and VRPs linked to LSMT and process them before shutting
#               the LSMT down.
#
        lda     s1aclose_cr,g2          # g2 = close completion routine
        call    lld$sendclose           # build and send for MAG session
#
# --- Send "Loss of Session" VRP to th DLM
#
        call    lld$send_ST             # send session terminate to DLM
#
# --- Close out session for target
#
        mov     0,r4
        st      r4,lsmt_dlmid(g3)       # clear out DLM ID from LSMT
        call    lld$rem_lsmt            # remove LSMT from session list in LTMT
.ifdef M4_DEBUG_LSMT
c fprintf(stderr, "%s%s:%u put_lsmt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g3);
.endif # M4_DEBUG_LSMT
c       put_lsmt(g3);                   # Deallocate LSMT
        mov     r3,g4                   # restore g4
        movq    r12,g0                  # restore g0-g3
        ret
#
#******************************************************************************
#
#  NAME: s1aclose_cr
#
#  PURPOSE:
#
#       Processes completion of the peer MAGNITUDE link CLOSE
#       session request for the MAGNITUDE link session.
#
#  DESCRIPTION:
#
#       Simply deallocates the resources used for the CLOSE request
#       (IRP, ILT) and returns to the caller.
#
#  CALLING SEQUENCE:
#
#       call    s1aclose_cr
#
#   INPUT:
#
#       g1 = ILT/IRP address of CLOSE request at nest level 1
#
#  OUTPUT:
#
#       None.
#
#  REGS. DESTROYED:
#
#       None.
#
#******************************************************************************
#
#
s1aclose_cr:
        movq    g0,r12                  # save g0-g3
        ld      lirp1_irp(g1),g0        # g0 = IRP for CLOSE request
.ifdef M4_DEBUG_IRP
c CT_history_printf("%s%s:%u put_irp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif # M4_DEBUG_IRP
c       put_irp(g0);                    # release IRP
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
        movq    r12,g0                  # restore g0-g3
        ret
#
#******************************************************************************
#
#  NAME: s1a_smsg
#
#  PURPOSE:
#
#       Performs the processing of a send message event on a
#       MAGNITUDE link associated session.
#
#  DESCRIPTION:
#
#*** FINISH
#
#  CALLING SEQUENCE:
#
#       call    s1a_smsg
#
#   INPUT:
#
#*** FINISH
#       g3 = assoc. LSMT address
#
#  OUTPUT:
#
#       None.
#
#  REGS. DESTROYED:
#
#       None.
#
#******************************************************************************
#
#
s1a_smsg:
#*** FINISH
        ret
#
#******************************************************************************
#
#  NAME: s2a_smsg
#
#  PURPOSE:
#
#       Performs the processing of a send message event on a
#       Foreign Target linked device associated session.
#
#  DESCRIPTION:
#
#*** FINISH
#
#  CALLING SEQUENCE:
#
#       call    s2a_smsg
#
#   INPUT:
#
#*** FINISH
#       g3 = assoc. LSMT address
#
#  OUTPUT:
#
#       None.
#
#  REGS. DESTROYED:
#
#       None.
#
#******************************************************************************
#
s2a_smsg:
#*** FINISH
        ret
#
#******************************************************************************
#
#  NAME: s1a_vrp
#
#  PURPOSE:
#       Performs the processing of a process VRP event on a
#       MAGNITUDE link associated session.
#
#  CALLING SEQUENCE:
#       call    s1a_vrp
#
#   INPUT:
#       g3 = assoc. LSMT address
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       None.
#
#******************************************************************************
#
s1a_vrp:
#*** FINISH
        ret
#
#******************************************************************************
#
# ____________ MAGNITUDE LINK DEVICE DRIVER EVENT HANDLER TABLES ______________
#
#******************************************************************************
#
#
# --- Default MAG link device driver event handler table (im_ehand)
#
        .data
LLD_d$event_tbl:                        # MAG default event table
        .word   lld$dcmdrecv            # SCSI command received
        .word   mag$abtask              # Abort task
        .word   mag$dabtaskset          # Abort task set
        .word   mag$dreset              # Reset received
        .word   mag$doffline            # Interface offline (not operational)
        .word   mag$donline             # Interface online (becoming
                                        #  operational)
#
# --- Normal MAG link device driver event handler table (ilm_ehand)
#
LLD_event_tbl:                          # Normal MAG event table
        .word   mag$cmdrecv             # SCSI command received
        .word   mag$abtask              # Abort task
        .word   mag$abtaskset           # Abort task set
        .word   mag$reset               # Reset received
        .word   mag$offline             # Interface offline (not operational)
        .word   mag$online              # Interface online (becoming
                                        #  operational)
        .word   mag$clearaca            # Clear ACA received
#
        .text
#
#******************************************************************************
#
# _______________ DEFAULT MAGNITUDE LINK DEVICE DRIVER ROUTINES _______________
#
#******************************************************************************
#
#  NAME: lld$dcmdrecv
#
#  PURPOSE:
#       Default SCSI command received event handler routine.
#
#  DESCRIPTION:
#       This routine is called by the channel driver when a SCSI
#       command is received associated with a MAGNITUDE link IMT
#       but specifying a LUN which is not defined for use by the
#       initiator.
#
#  CALLING SEQUENCE:
#       call    lld$dcmdrecv
#
#  INPUT:
#       g1 = pri. ILT at inl2 nest level
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = 0 indicating no assoc. ILMT
#       g7 = assoc. ILT param. structure
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Reg. r3-r15/g4-g7 can be destroyed.
#
#******************************************************************************
#
lld$dcmdrecv:
        movq    g8,r8                   # save g8-g11
        ld      sccdb(g7),g8            # g8 = pointer to CDB
        call    mag$init_task           # initialize task inl2 level data
                                        #  structure
        lda     task_etbl2,r3           # r3 = task event handler table
        st      r3,inl2_ehand(g1)       # save task event handler table
        lda     lld$normtbl,g11         # g11 = cmd. norm. table
        movq    g0,r4                   # save g0-g3
        ldob    (g8),g9                 # g9 = SCSI command code
        lda     lld$dcmdhand,g10        # g10 = default command handler table
        ldob    (g11)[g9*1],g9          # g9 = normalized command code
        movt    g12,r12                 # save g12-g14
        ld      (g10)[g9*4],g10         # g10= command handler routine
        mov     g1,g9                   # g9 = primary ILT address
        callx   (g10)                   # and go to handler routine
        movq    r4,g0                   # restore g0-g3
        movq    r8,g8                   # restore g8-g11
        movt    r12,g12                 # restore g12-g14
        ret
#
#*********************************************************************
#
# --- Input to command handler routines:
#
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = 0 indicating no assoc. ILMT
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#       Reg. r3-r15/g0-g14 can be destroyed.
#
#*********************************************************************
#
# --- MAGNITUDE link command normalization table
#
# --- Normal operation normalization table
#
        .data
lld$normtbl:
        .byte   1,0,0,5,0,0,0,0         # 00-07
        .byte   0,0,0,0,0,0,0,0         # 08-0f
        .byte   0,0,2,0,0,0,0,0         # 10-17
        .byte   0,0,4,3,0,0,0,0         # 18-1f
        .byte   0,0,0,0,0,0,0,0         # 20-27
        .byte   9,0,12,0,0,0,15,16      # 28-2f
        .byte   0,0,0,0,0,0,0,0         # 30-37
        .byte   0,0,0,8,7,0,0,0         # 38-3f
        .byte   0,0,0,0,0,0,0,0         # 40-47
        .byte   0,0,0,0,0,0,0,0         # 48-4f
        .byte   0,0,0,0,0,0,0,0         # 50-57
        .byte   0,0,0,0,0,0,0,0         # 58-5f
        .byte   0,0,0,0,0,0,0,0         # 60-67
        .byte   0,0,0,0,0,0,0,0         # 68-6f
        .byte   0,0,0,0,0,0,0,0         # 70-77
        .byte   0,0,0,0,0,0,0,0         # 78-7f
        .byte   0,0,0,0,0,0,0,0         # 80-87
        .byte   11,0,14,0,0,0,17,18     # 88-8f 11=read-16,14=write-16,17=write&verify-16,18=verify-16
        .byte   0,0,0,0,0,0,0,0         # 90-97
        .byte   0,0,0,0,0,0,0,0         # 98-9f
        .byte   6,0,0,0,0,0,0,0         # a0-a7
        .byte   10,0,13,0,0,0,0,0       # a8-af
        .byte   0,0,0,0,0,0,0,0         # b0-b7
        .byte   0,0,0,0,0,0,0,0         # b8-bf
        .byte   0,0,0,0,0,0,0,0         # c0-c7
        .byte   0,0,0,0,0,0,0,0         # c8-cf
        .byte   0,0,0,0,0,0,0,0         # d0-d7
        .byte   0,0,0,0,0,0,0,0         # d8-df
        .byte   0,0,0,0,0,0,0,0         # e0-e7
        .byte   0,0,0,0,0,0,0,0         # e8-ef
        .byte   0,0,0,0,0,0,0,0         # f0-f7
        .byte   0,0,0,0,0,0,0,0         # f8-ff
#
# --- MAGNITUDE link default command handler table
#
# --- Normal operation command handler table
#
lld$dcmdhand:
        .word   magd$nolun      # 0# All other commands for unsupported LUNs
        .word   magd$nolun      # 1# Test unit ready        -- 0x01
        .word   magd$inquiry    # 2# Inquiry                -- 0x12
        .word   magd$nolun      # 3# Start/Stop Unit        -- 0x1B
        .word   magd$nolun      # 4# Mode Sense (6)         -- 0x1A
        .word   magd$reqsns     # 5# Request sense          -- 0x03
        .word   magd$repluns    # 6# Report LUNs            -- 0xA0
        .word   magd$nolun      # 7# Read Buffer            -- 0x3C
        .word   magd$nolun      # 8# Write Buffer           -- 0x3B
        .word   magd$nolun      # 9# Read (10)              -- 0x28
        .word   magd$nolun      #10# Read (12)              -- 0xA8
        .word   magd$nolun      #11# Read (16)              -- 0x88
        .word   magd$nolun      #12# Write (10)             -- 0x2A
        .word   magd$nolun      #13# Write (12)             -- 0xAA
        .word   magd$nolun      #14# Write (16)             -- 0x8A
        .word   magd$nolun      #15# Write & Verify (10)    -- 0x2E
        .word   magd$nolun      #16# Verify Media (10)      -- 0x2F
        .word   magd$nolun      #17# Write & Verify (16)    -- 0x8E
        .word   magd$nolun      #18# Verify Media (16)      -- 0x8F
##
# --- MAGNITUDE link command handler table #1a ----------------------------------
#
# --- Link only operation command handler table ---------------------------------
#
#       Used when ILMT is not associated with a VDMT.
#
LLD$cmdtbl1a:
        .word   lld1$undef      # 0# All other commands for unsupported LUNs
        .word   mag1$tur        # 1# Test unit ready        -- 0x01
        .word   lld1$inquiry    # 2# Inquiry                -- 0x12
        .word   mag1$startstop  # 3# Start/Stop Unit        -- 0x1B
        .word   mag1$modesns    # 4# Mode Sense (6)         -- 0x1A
        .word   mag1$reqsns     # 5# Request sense          -- 0x03
        .word   mag1$repluns    # 6# Report LUNs            -- 0xA0
        .word   lld1$readbuff   # 7# Read Buffer            -- 0x3C
        .word   lld1$writebuff  # 8# Write Buffer           -- 0x3B
        .word   lld1$undef      # 9# Read (10)              -- 0x28
        .word   lld1$undef      #10# Read (12) (not used)   -- 0xA8
        .word   lld1$undef      #11# Read (16)              -- 0x88
        .word   lld1$undef      #12# Write (10)             -- 0x2A
        .word   lld1$undef      #13# Write (12) (not used)  -- 0xAA
        .word   lld1$undef      #14# Write (16)             -- 0x8A
        .word   lld1$undef      #15# Write & Verify (10)    -- 0x2E
        .word   lld1$undef      #16# Verify Media (10)      -- 0x2F
        .word   lld1$undef      #17# Write & Verify (16)    -- 0x8E
        .word   lld1$undef      #18# Verify Media (16)      -- 0x8F
#
# --- MAGNITUDE link command handler table #1b ----------------------------------
#
# --- Normal operation command handler table ------------------------------------
#
#       Used when ILMT associated with VDMT and device has not been reserved.
#
LLD$cmdtbl1b:
        .word   lld1$undef      # 0# All other commands for unsupported LUNs
        .word   mag1$tur        # 1# Test unit ready        -- 0x01
        .word   lld1$inquiry    # 2# Inquiry                -- 0x12
        .word   mag1$startstop  # 3# Start/Stop Unit        -- 0x1B
        .word   mag1$modesns    # 4# Mode Sense (6)         -- 0x1A
        .word   mag1$reqsns     # 5# Request sense          -- 0x03
        .word   mag1$repluns    # 6# Report LUNs            -- 0xA0
        .word   lld1$readbuff   # 7# Read Buffer            -- 0x3C
        .word   lld1$writebuff  # 8# Write Buffer           -- 0x3B
        .word   mag1$read10     # 9# Read (10)              -- 0x28
        .word   lld1$undef      #10# Read (12) (not used)   -- 0xA8
        .word   mag1$read_16    #11# Read (16)  For >2TB    -- 0x88
        .word   mag1$write10    #12# Write (10)             -- 0x2A
        .word   lld1$undef      #13# Write (12) (not used)  -- 0xAA
        .word   mag1$write_16   #14# Write (16) For >2TB    -- 0x8A
        .word   mag1$writevfy   #15# Write & Verify (10)    -- 0x2E
        .word   mag1$vfymedia   #16# Verify Media (10)      -- 0x2F
        .word   mag1$writevfy_16 #17 Write & Verify (16)    -- 0x8E
        .word   mag1$verify_16  #18# Verify Media (16)      -- 0x8F
#
# --- MAGNITUDE link command handler table #2 -----------------------------------
#
# --- Device reserved operation command handler table ---------------------------
#
#       Used when ILMT associated with VDMT and device has been reserved.
#
LLD$cmdtbl2:
        .word   lld1$undef      # 0# All other commands for unsupported LUNs
        .word   mag2$rconflict  # 1# Test unit ready        -- 0x01
        .word   lld1$inquiry    # 2# Inquiry                -- 0x12
        .word   mag2$rconflict  # 3# Start/Stop Unit        -- 0x1B
        .word   mag2$rconflict  # 4# Mode Sense (6)         -- 0x1A
        .word   mag1$reqsns     # 5# Request sense          -- 0x03
        .word   mag1$repluns    # 6# Report LUNs            -- 0xA0
        .word   lld1$readbuff   # 7# Read Buffer            -- 0x3C
        .word   lld1$writebuff  # 8# Write Buffer           -- 0x3B
        .word   mag2$rconflict  # 9# Read (10)              -- 0x28
        .word   lld1$undef      #10# Read (12) (not used)   -- 0xA8
        .word   mag2$rconflict  #11# Read (16) For >2TB     -- 0x88
        .word   mag2$rconflict  #12# Write (10)             -- 0x2A
        .word   lld1$undef      #13# Write (12) (not used)  -- 0xAA
        .word   mag2$rconflict  #14# Write (16) For >2TB    -- 0x8A
        .word   mag2$rconflict  #15# Write & Verify (10)    -- 0x2E
        .word   mag2$rconflict  #16# Verify Media (10)      -- 0x2F
        .word   mag2$rconflict  #17# Write & Verify (16)    -- 0x8E
        .word   mag2$rconflict  #18# Verify Media (16)      -- 0x8F
#
        .text
#
#******************************************************************************
#
#  NAME: lld1$undef
#
#  PURPOSE:
#       Process unknown scsi commands from other magnitudes.
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All registers may be destroyed.
#
#******************************************************************************
#
lld1$undef:
c       lld1_undef(g8, g5, g7, g9);
        ret
#
#******************************************************************************
#
#  NAME: lld1$inquiry
#
#  PURPOSE:
#       Processes INQUIRY CDBs received from other MAGNITUDEs.
#
#  DESCRIPTION:
#       Checks for special MAGNITUDE-to-MAGNITUDE Vital Product Data
#       pages and if requested processes these pages appropriately.
#       If standard INQUIRY data requested, jumps to the mag1$inquiry
#       handler routine.
#
#  CALLING SEQUENCE:
#       call    lld1$inquiry
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Regs r3-r15/g0-g14 can be destroyed.
#
#******************************************************************************
#
lld1$inquiry:
#
# --- Validate INQUIRY CDB
#
        ldob    1(g8),r4                # r4 = CDB byte with op. flags
        and     0x03,r4,r4              # mask off reserved bits
        cmpobne.f 1,r4,mag1$inquiry     # Jif not EVPD INQUIRY data requested
        ldob    4(g8),r3                # r3 = alloc. length from CDB
        cmpobe.f 0,r3,.lld1$inq10       # Jif alloc. length zero
        ldob    2(g8),r5                # r5 = Page/Operation code byte
        ldconst inqinfo,r4              # r4 = page code to compare
        cmpobe.t r4,r5,.lld1$inq100     # Jif information page code
        b       mag1$inquiry            # handle as normal INQUIRY
#
# --- Process valid INQUIRY request with zero allocation length in CDB
#
.lld1$inq10:
        ldq     inquiry_tbl3,r4         # load op. values into regs.
        ld      inquiry_tbl3+16,r8
        b       .lld1$inq10000          # and just return status
#
# --- Invalid INQUIRY CDB handler routine
#
.lld1$inq20:
        ldq     inquiry_tbl2,r4         # load op. values into regs.
        ld      inquiry_tbl2+16,r8
        b       .lld1$inq10000
#
# --- Process INQUIRY information page request ---------------------------------
#
.lld1$inq100:
        lda     inqinfo_size,g0         # g0 = SGL/buffer combo memory size
        cmpobne.f r3,g0,.lld1$inq20     # Jif CDB length invalid
        mov     g0,r9                   # r9 = my inquiry data size
c       g0 = m_asglbuf(g0);             # allocate a SGL and buffer
        ld      sghdrsiz+sg_addr(g0),g1 # g1 = buffer address
        ld      info_bnr,r4             # r4 = banner
        st      r4,inqinfo_bnr(g1)      # save banner
        ld      K_ficb,r8               # r8 = FICB
#
.if     spec_INQ_new
        ld      fi_vcgid(r8),r4         # r4 = VCG serial number
        bswap   r4,r4
        st      r4,inqinfo_sn(g1)       # save VCG serial number as system
                                        #  serial number
.endif  # spec_INQ_new
#
        ld      fi_cserial(r8),r4       # r4 = Controller serial number
        bswap   r4,r4
#
.if     spec_INQ_new
        st      r4,inqinfo_alias(g1)    # save controller serial number as
                                        #  alias serial number
        ldos    im_tid(g5),r4           # r4 = assigned Target #
        ldconst MAXTARGETS,r12          # r12 = maximum # targets supported
        cmpobl  r4,r12,.lld1$inq120     # Jif "normal" target #
        ldconst 0xffff,r4               # set assigned target # to unassigned
                                        #  value
.lld1$inq120:
.else   # spec_INQ_new
        st      r4,inqinfo_sn(g1)       # save system serial number
        ldconst 0,r4                    # r4 = assigned cluster # = 0
.endif  # spec_INQ_new
#
        stob    r4,inqinfo_cl(g1)       # save assigned cluster #
        ldob    ci_num(g4),r4           # r4 = Path # (from CIMT)
        stob    r4,inqinfo_path(g1)     # save Path #
#
        movq    g0,r12                  # Save g0-g3
        ld      K_ficb,r4               # Get the FICB
        ldl     fi_vcgname(r4),r4       # r4-r5 = Controller Name
        stl     r4,inqinfo_name(r13)    # save controller name
#
        ld      fi_ccbipaddr(r8),r4     # r4 = CCB IP Address
        st      r4,inqinfo_ip(r13)      # save IP address
        movt    0,r4                    # clear r4-r6
#
.if     spec_INQ_new
        stt     r4,inqinfo_spare(r13)   # clear spare bytes
        mov     r4,g0                   # g0 = inqinfo_flag1 byte
        ldos    idf_vpid(g7),g2         # g2 = virtual port ID that this
                                        #      command was received on
        ldob    ci_num(g4),r4           # r4 = interface #
c       r5 = ICL_IsIclPort((UINT8)r4);
        cmpobe TRUE,r5,.lld1$inq150              # Jif ICL
        ld      iscsimap,r5
        bbs     r4,r5,.lld1$inq150

        ld      ispLid[r4*4],r5         # r5 = primary port LID
        cmpobe  r5,g2,.lld1$inq150      # Jif received on primary port
        setbit  0,g0,g0                 # set flag bit indicating operating
                                        #  in target-only mode
        ld      im_ltmt(g5),g2          # g2 = assoc. LTMT address
        cmpobne 0,g2,.lld1$inq150       # Jif LTMT already assoc. with IMT
        call    lld$target_to_mode      # set up target-only mode environment
.lld1$inq150:
        stob    g0,inqinfo_flag1(r13)   # save flag byte #1
        movq    0,r4                    # clear r4-r7 again
.else   # spec_INQ_new
        stq     r4,inqinfo_spare(r13)   # clear spare bytes
        stob    r4,11(r13)              # clear unused byte
.endif  # spec_INQ_new
#
# --- Count the number of VDisks defined for this XIOtech Controller
#   and place in info. data
#
                                        # r4 = VDisk count
#
# Look for server with the same WWN and Target ID in the SDD
#
        ldl     im_mac(g5),g0           # Get the WWN for this server
        ldos    im_tid(g5),g2           # Get the Target ID for this server
        ldconst FALSE,g3                # g3 = Ignore new servers

        PushRegs(r3)
        lda     im_iname(g5),g4         # g4 = iSCSI name
        call    DEF_WWNLookup
        mov     g0,r5
        PopRegsVoid(r3)
        mov     r5,g3

        ldconst 0xffffffff,g2           # Set SID number to not found
        cmpobe  g3,g2,.lld1$inq500      # Jif a matching Server was not found
                                        #   (show 0 vdisks assigned)
#
# --- Setup pointers to the SDD
#
        ld      S_sddindx[g3*4],g3      # g3 = pointer to SDD
        ld      sd_lvm(g3),r10          # r10 = pointer to first LVM
#
# --- Loop to handle each LVM in the SDD --------------------------------------
#     Increment the count field for each valid entry
#
.lld1$inq300:
        cmpobe  0,r10,.lld1$inq500      # Jif no LUN/VID found at this LVM
        addo    1,r4,r4                 # Increment the VDisk Count
        ld      lv_nlvm(r10),r10        # r10 = next LUN/VID entry
        b       .lld1$inq300            # Check the next LUN/VID entry
#
.lld1$inq500:
        movq    r12,g0                  # Restore g0-g3
        stob    r4,inqinfo_vdcnt(g1)    # save VDisk count in info. data
        b       .lld1$inq9000           # and finish setting up I/O operation
#
        .data
info_bnr:
        .ascii  "INFO"                  # information page ID data
#
info_tmp_ip:
        .word   0                       # assigned IP address
#
        .text
#
# --- End INQUIRY information page processing ---------------------------------
#
.lld1$inq9000:
        st      r9,sghdrsiz+sg_len(g0)  # save size of data in SGL
        ldq     inquiry_tbl1,r4         # load op. values into regs.
        ld      inquiry_tbl1+16,r8
        mov     g0,r6                   # r6 = SGL pointer
.lld1$inq10000:
        b       mag1$cmdcom             # and finish processing command
#
#******************************************************************************
#
#  NAME: lld1$writebuff
#
#  PURPOSE:
#       Processes WRITE BUFFER CDBs received from other MAGNITUDEs.
#
#  CALLING SEQUENCE:
#       call    lld1$writebuff
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Regs r3-r15/g0-g14 can be destroyed.
#
#******************************************************************************
#
        .data
wb_tbl1:
        .byte   dtxferc,scnorm,0,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  1                       # SGL descriptors
        .short  0                       # sense length
#
wb_tbl2:
        .byte   dtxfern,scnorm,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  0                       # SGL descriptors
        .short  0                       # sense length
#
wb_cmdt:
        .byte   dtxfern,sccmdt,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  0                       # # SGL descriptors
        .short  0                       # sense length
#
wb_bsy1:
        .byte   dtxfern,scbusy,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  0                       # # SGL descriptors
        .short  0                       # sense length
#
wb_sns1:
        .byte   dtxfern,scechk,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   sense_invf1             # sense pointer
        .short  0                       # # SGL descriptors
        .short  sensesize               # sense length
#
        .text
#
lld1$writebuff:
#
# --- determine id LTMT is allocated
#
        ld      im_ltmt(g5),g10         # g10 = address of ltmt
        cmpobne.f 0,g10,.wb_020         # Jif IMT associated with LTMT
#
# --- try to match up a LTMT with an IMT
#
        call    lld$findLTMT            # determine if there is a possible
                                        # LTMT
        cmpobe  0,g10,.wb_060           # Jif no LTMT, send busy
#
        ld      I_noass,r3              # bump no imt/ltmt association counter
        addo    1,r3,r3
        st      r3,I_noass
#
# --- LTMT is present
#
.wb_020:
#
# --- Clear the reset count since the request was successful.
#
        ldob    ci_num(g4),r4
        ld      port_rescan_count[r4*4],r6
        cmpobe  0,r6,.wb_022
c fprintf(stderr,"%s%s:%u <--- <><> WB Port %ld recovered.\n", FEBEMESSAGE, __FILE__, __LINE__,r4);
        mov     0,r6
        st      r6,port_rescan_count[r4*4]
        ldconst 30,r6
        ld      timestamp,r14
        addo    r6,r14,r14
        st      r14,port_rescan_time[r4*4]

.wb_022:
        ldob    2(g8),r4                # r4 = CDB byte with buffer ID
        cmpobg.t ltmt_dgmax,r4,.wb_030  # Jif valid ID value
        ldq     wb_sns1,r4              # load op. values into regs.
        ld      wb_sns1+16,r8
        b       .wb_905
#
# --- update some write stats
#
#jt        ld      ci_stattbl(g4),r15      # r15 = assoc. stats. table address
#jt        ld      cdba_writes(r15),r14    # r14 = aggregate write count
#jt        ld      cdbp_writes(r15),r13    # r13 = periodic write count
#jt        addo    1,r14,r14               # add to write counts
#jt        addo    1,r13,r13
#jt        st      r14,cdba_writes(r15)    # save updated counters
#jt        st      r13,cdbp_writes(r15)
#
# --- determine if there is an operation active for this exchange ID
#
.wb_030:
        ld      ltmt_tmsg(g10)[r4*4],g1 # g1 = possible LDG area
        cmpobe.t 0,g1,.wb_100           # Jif new exchange operation
#
# --- determine if a write buffer command has already been received. If it
#     has, send a busy. The determine if a read buffer command has been
#     received. If one has not been received, the LDG is probably in some
#     form of error recovery and a busy should be sent to the initiator.
#
        ld      sdg_writeILT(g1),r10    # r10 = possible write buffer ILT
        cmpobne.t 0,r10,.wb_050         # jif WB received - send busy

        ld      sdg_readILT(g1),r10     # r11 = possible read ILT
        cmpobne.t 0,r10,.wb_200         # JIf if RB received
#
#  --- This slot is still in use, send a busy back to the initiator
#
.wb_050:
        ldq     wb_bsy1,r4              # load op. values into regs.
        ld      wb_bsy1+16,r8
        b       .wb_905
#
#  --- IMT - LTMT association missing, send a cmd terminated back to initiator
#
.wb_060:
        ldq     wb_cmdt,r4              # load op. values into regs.
        ld      wb_cmdt+16,r8
        b       .wb_905
#
# --- This is a new datagram message, allocate a LDG and place the WB ILT
#     in it.
#
.wb_100:
c       g1 = get_ilt();                 # Allocate an ILT as LDG temp storage
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
        mov     0,r3
        st      g1,ltmt_tmsg(g10)[r4*4] # save LDG temp storage area address
        st      r3,sdg_readILT(g1)      # clear read ILT address
        st      r3,sdg_sgl(g1)          # clear sgl pointer

        stob    r4,inl2_lldID(g9)       # save lld exchange ID
        st      g9,sdg_writeILT(g1)     # save write ILT address (at 2nd lvl)
        lda     task_etbl16b,r4         # r4 = event table (waiting for rb cmd)
        st      r4,inl2_ehand(g9)       # save task event handler table in
        b       .wb_1000                # exit
#
# --- read buffer command already received, set up for write data transfer
#
.wb_200:
        stob    r4,inl2_lldID(g9)       # save lld exchange ID in wb ILT
        ld      inl2_ehand(r10),r4      # r4 = current ehand for read buffer command
        lda     task_etbl16a,r5         # r5 = event table (waiting for wb cmd)
        cmpobne r4,r5,.wb_050           # Jif is state machine currently active
                                        # for this slot

        st      g9,sdg_writeILT(g1)     # save write ILT address (at 2nd lvl)
        lda     task_etbl16e,r4         # r4 = event table (waiting for wb trans cmplt)
        st      r4,inl2_ehand(r10)      # save task event handler table in rb ILT
        st      r4,inl2_ehand(g9)       # save task event handler table in wb ILT

        mov     g1,r15                  # r15 = LDG temp storage
#
# --- determine the amount of space to allocate for the SGL
#
        ld      2(g8),r5                # get the response length
        ld      6(g8),r6                # get request length
        shro    8,r5,r5                 # remove MSB
        shlo    8,r5,r5
        bswap   r5,r5                   # correct endiance
        shlo    8,r6,r6                 # remove LSB
        bswap   r6,r6                   # correct endiance
        addo    r6,r5,g0                # add response and request header
        lda     sg_desc2(g0),g0         # add SGL hdr + 2 descr's to allocation

c       g0 = m_asglbuf(g0);             # allocate a SGL and buffer

        st      g0,sdg_sgl(r15)         # save original sgl address

        lda     sghdrsiz(g0),r7         # r7 = address of 1 descriptor
        ld      sg_addr(r7),r4          # get pointer to buffer
        lda     sg_desc2(r4),r4         # add SGL hdr + 2 descr's

        st      r4,sdg_reqptr(r15)      # save in LDG temp storage
        st      r4,sdg_desc0_ptr(r15)   # save pointer to buffer is SGL descriptor
        st      r6,sdg_desc0_len(r15)   # save request length
        st      r6,sdg_reqlen(r15)      # save request length

        addo    r6,r4,r4                # calculate address of response
        addo    4-1,r4,r4               # round up to a word boundary
        andnot  4-1,r4,r4               # r4 = response buffer ptr
        st      r4,sdg_respptr(r15)     # save address of response
        st      r5,sdg_resplen(r15)     # save response length

        mov     inl2_ps_datatr,r10      # r10 = new process state code
        lda     mag2_iocr,r11           # r11 = I/O completion routine
        ldq     wb_tbl1,r4              # load op. values into regs.
        ld      wb_tbl1+16,r8
        lda     sdg_desc0_ptr(r15),r6   # r6 = ptr to SGL descriptors

        stob    r10,inl2_pstate(g9)     # save task process state code
        b       .wb_900

.wb_905:
        lda     mag1_iocr,r11           # r11 = I/O completion routine
.wb_900:
c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
#
# --- save parameters in secondary ILT
#
        stq     r4,xlcommand(g1)        # stuff op. req. record

        mov     g7,r9                   # r9 = assoc. ILT parameter structure
        mov     g9,r10                  # r10 = pri. ILT at inl2 nest level
        st      r11,otl2_cr(g1)         # save completion routine
        stt     r8,xlsgllen(g1)         # save sense length/SGL length/assoc.
                                        #  ILT param. structure/pri. ILT at
                                        #  inl2 nest level
        mov     g1,r11                  # r11 = my ILT nest level ptr.
#
        lda     ILTBIAS(g1),g1          # inc. to next nest level
        st      r11,otl3_OTL2(g1)       # save param. ptr. in next nest

        PushRegs(r3)                    # Save all G registers
        call    mag$ISP$receive_io      # issue channel directive
        PopRegsVoid(r3)                 # Restore all G registers

.wb_1000:
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: lld1$readbuff
#
#  PURPOSE:
#       Processes READ BUFFER CDBs received from other MAGNITUDEs.
#
#  CALLING SEQUENCE:
#       call    lld1$readbuff
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#       g7 = assoc. ILT param. structure
#       g8 = pointer to 16 byte SCSI CDB
#       g9 = primary ILT address at XL nest
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Regs r3-r15/g0-g14 can be destroyed.
#
#******************************************************************************
#
        .data
rb_tbl1:
        .byte   dtxferi,scnorm,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  1                       # SGL descriptors
        .short  0                       # sense length
#
rb_cmdt:
        .byte   dtxfern,sccmdt,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  0                       # # SGL descriptors
        .short  0                       # sense length
#
rb_bsy1:
        .byte   dtxfern,scbusy,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   0                       # sense pointer
        .short  0                       # # SGL descriptors
        .short  0                       # sense length
#
rb_sns1:
        .byte   dtxfern,scechk,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   sense_invf1             # sense pointer
        .short  0                       # # SGL descriptors
        .short  sensesize               # sense length

# Store the 32 bit time for the next rescan of a particular port.
port_rescan_time:
        .word  0
        .word  0
        .word  0
        .word  0
        .word  0
        .word  0
        .word  0
        .word  0
# Store the rescan count for a particular port.
port_rescan_count:
        .word  0
        .word  0
        .word  0
        .word  0
        .word  0
        .word  0
        .word  0
        .word  0
# Store the kickme flag (debounce)
port_hit_cnt:
        .word  0
        .word  0
        .word  0
        .word  0
        .word  0
        .word  0
        .word  0
        .word  0
# Store the start time of possible debounce cycle
port_hit_time:
        .word  0
        .word  0
        .word  0
        .word  0
        .word  0
        .word  0
        .word  0
        .word  0
#
        .text
#
lld1$readbuff:
#
# --- determine id LTMT is allocated
#
        ld      im_ltmt(g5),g10         # g10 = address of ltmt
        cmpobne.f 0,g10,.rb_020         # Jif IMT associated with LTMT
#
# --- try to match up a LTMT with an IMT
#
        call    lld$findLTMT            # determine if there is a possible
                                        # LTMT
        cmpobe  0,g10,.rb_060           # Jif no LTMT, send busy

        ld      I_noass,r3             # bump no imt/ltmt association counter
        addo    1,r3,r3
        st      r3,I_noass
#
# --- LTMT is present
#
.rb_020:
#
# --- Clear the reset count since the request was successful.
#
        ldob    ci_num(g4),r4
        ld      port_rescan_count[r4*4],r6
        cmpobe  0,r6,.rb_022
c fprintf(stderr,"%s%s:%u <--- <><> RB Port %ld recovered.\n", FEBEMESSAGE, __FILE__, __LINE__,r4);
        mov     0,r6
        st      r6,port_rescan_count[r4*4]
        ldconst 30,r6
        ld      timestamp,r14
        addo    r6,r14,r14
        st      r14,port_rescan_time[r4*4]
.rb_022:
        ldob    2(g8),r4                # r4 = CDB byte with buffer ID
        cmpobg.t ltmt_dgmax,r4,.rb_030  # Jif valid ID value
        ldq     rb_sns1,r4              # load op. values into regs.
        ld      rb_sns1+16,r8
        b       .rb_905
#
# --- determine if there is an operation active for this exchange ID
#
.rb_030:
        ld      ltmt_tmsg(g10)[r4*4],g1 # g1 = possible LDG area
        cmpobe.t 0,g1,.rb_100           # Jif new exchange operation
#
# --- Determine if a read buffer command has already been received. If it
#     has, send a busy. The determine if a write buffer command has been
#     received. If one has not been received, the LDG is probably in some
#     form of error recovery and a busy should be sent to the initiator.
#
        ld      sdg_readILT(g1),r10     # r10 = possible read buffer ILT
        cmpobne.t 0,r10,.rb_050         # jif read buffer received - send busy

        ld      sdg_writeILT(g1),r10    # get possible wb ILT entry
        cmpobne.f 0,r10,.rb_200         # Jif wb cmd has been received
#
#  --- This slot is still in use, send a busy back to the initiator
#
.rb_050:
        ldq     rb_bsy1,r4              # load op. values into regs.
        ld      rb_bsy1+16,r8
        b       .rb_905
#
#  --- IMT - LTMT association missing, send a cmd terminated back to initiator
#
.rb_060:
        ldq     rb_cmdt,r4              # load op. values into regs.
        ld      rb_cmdt+16,r8
        b       .rb_905
#
# --- This is a new datagram message, allocate a LDG and place the WB ILT
#     in it.
#
.rb_100:
c       g1 = get_ilt();                 # Allocate an ILT as LDG temp storage
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
        mov     0,r3
        st      g1,ltmt_tmsg(g10)[r4*4] # save LDG temp storage area address
        st      r3,sdg_writeILT(g1)     # clear write ILT address
        st      r3,sdg_sgl(g1)          # clear sgl pointer

        stob    r4,inl2_lldID(g9)       # save lld exchange ID
        st      g9,sdg_readILT(g1)      # save rb ILT address (at 2nd lvl)
        lda     task_etbl16a,r4         # r4 = event table (waiting for wb cmd)
        st      r4,inl2_ehand(g9)       # save task event handler table in
        b       .rb_1000                # exit
#
# --- write buffer command already received, set up for write data transfer
#
.rb_200:
        stob    r4,inl2_lldID(g9)       # save lld exchange ID in rb ILT
        ld      inl2_ehand(r10),r4      # r4 = current ehand for write buffer command
        lda     task_etbl16b,r5         # r5 = event table (waiting for rb cmd)
        cmpobne r4,r5,.rb_050           # Jif is state machine currently active

        st      g9,sdg_readILT(g1)      # save rb ILT address (at 2nd lvl)
        lda     task_etbl16e,r4         # r4 = event table (waiting for wb command)
        st      r4,inl2_ehand(r10)      # save task event handler table in wb ILT
        st      r4,inl2_ehand(g9)       # save task event handler table in rb ILT

        mov     r10,g9                  # g9 = wb ILT
        mov     g1,r15                  # r15 = LDG temp storage
#
# --- reconstuct the write buffer command registers
#
        lda     -ILTBIAS(g9),g7         # g7 = ILT' at 1st level
        lda     inl2_cdb(g9),g8         # g8 = address of cdb storage area in lvl2
#
# --- determine the amount of space to allocate for the SGL
#

        ld      2(g8),r5                # get the response length
        ld      6(g8),r6                # get request length
        shro    8,r5,r5                 # remove MSB
        shlo    8,r5,r5
        bswap   r5,r5                   # correct endiance
        shlo    8,r6,r6                 # remove LSB
        bswap   r6,r6                   # correct endiance
        addo    r6,r5,g0                # add response and request header
        lda     sg_desc2(g0),g0         # add SGL hdr + 2 descr's to allocation

c       g0 = m_asglbuf(g0);             # allocate a SGL and buffer

        st      g0,sdg_sgl(r15)         # save original sgl address

        lda     sghdrsiz(g0),r7         # r7 = address of 1 descriptor
        ld      sg_addr(r7),r4          # get pointer to buffer
        lda     sg_desc2(r4),r4         # add SGL hdr + 2 descr's

        st      r4,sdg_reqptr(r15)      # save in LDG temp storage
        st      r4,sdg_desc0_ptr(r15)   # save pointer to buffer is SGL descriptor
        st      r6,sdg_desc0_len(r15)   # save request length
        st      r6,sdg_reqlen(r15)      # save request length

        addo    r6,r4,r4                # calculate address of response
        addo    4-1,r4,r4               # round up to a word boundary
        andnot  4-1,r4,r4               # r4 = response buffer ptr
        st      r4,sdg_respptr(r15)     # save address of response
        st      r5,sdg_resplen(r15)     # save response length

        mov     inl2_ps_datatr,r10      # r10 = new process state code
        lda     mag2_iocr,r11           # r11 = I/O completion routine
        ldq     wb_tbl1,r4              # load op. values into regs.
        ld      wb_tbl1+16,r8
        lda     sdg_desc0_ptr(r15),r6   # r6 = ptr to SGL descriptors

        stob    r10,inl2_pstate(g9)     # save task process state code
        b       .rb_900
#
.rb_905:
        lda     mag1_iocr,r11           # r11 = I/O completion routine
.rb_900:
c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
#
# --- save parameters in secondary ILT
#
        stq     r4,xlcommand(g1)        # stuff op. req. record

        mov     g7,r9                   # r9 = assoc. ILT parameter structure
        mov     g9,r10                  # r10 = pri. ILT at inl2 nest level
        st      r11,otl2_cr(g1)         # save completion routine
        stt     r8,xlsgllen(g1)         # save sense length/SGL length/assoc.
                                        #  ILT param. structure/pri. ILT at
                                        #  inl2 nest level
        mov     g1,r11                  # r11 = my ILT nest level ptr.
#
        lda     ILTBIAS(g1),g1          # inc. to next nest level
        st      r11,otl3_OTL2(g1)       # save param. ptr. in next nest

        PushRegs(r3)                    # Save all G registers
        call    mag$ISP$receive_io      # issue channel directive
        PopRegsVoid(r3)                 # Restore all G registers

.rb_1000:
        ret                             # return to caller
#
#******************************************************************************
#
# ____________________ TASK EVENT HANDLER TABLES ______________________________
#
#******************************************************************************
#
#
# ---   Read buffer command  received - waiting for write buffer command
#
        .data
task_etbl16a:
        .byte   inl2_ts_enbl            # task state code
        .byte   0,0,0                   # reserved
        .word   teh_ignore              # task I/O completion
                                        #  handler routine
        .word   teh_ignore              # MAGNITUDE request completion
                                        #  handler routine
        .word   teh_ignore              # SRP request handler routine
        .word   teh_ignore              # SRP request completion handler
                                        #  routine
        .word   te16a_abort             # abort task handler routine
        .word   te16a_abort             # reset event handler routine
        .word   te16a_abort             # offline event handler routine
        .word   teh_ignore              # ACA occurred handler routine
        .word   teh_ignore              # ACA cleared handler routine
#
# ---   Write buffer command received - waiting for read buffer command
#
task_etbl16b:
        .byte   inl2_ts_enbl            # task state code
        .byte   0,0,0                   # reserved
        .word   teh_ignore              # task I/O completion
                                        #  handler routine
        .word   teh_ignore              # MAGNITUDE request completion
                                        #  handler routine
        .word   teh_ignore              # SRP request handler routine
        .word   teh_ignore              # SRP request completion handler
                                        #  routine
        .word   te16b_abort             # abort task handler routine
        .word   te16b_abort             # reset event handler routine
        .word   te16b_abort             # offline event handler routine
        .word   teh_ignore              # ACA occurred handler routine
        .word   teh_ignore              # ACA cleared handler routine
#
# ---   Write and Read buffer commands received, request made for write data.
#       Waiting for write buffer command data transfer completion.
#
task_etbl16e:
        .byte   inl2_ts_enbl            # task state code
        .byte   0,0,0                   # reserved
        .word   te16e_iocomp            # task I/O completion
                                        #  handler routine
        .word   teh_ignore              # MAGNITUDE request completion
                                        #  handler routine
        .word   teh_ignore              # SRP request handler routine
        .word   teh_ignore              # SRP request completion handler
                                        #  routine
        .word   te16e_abort             # abort task handler routine
        .word   te16e_abort             # reset event handler routine
        .word   te16e_abort             # offline event handler routine
        .word   teh_ignore              # ACA occurred handler routine
        .word   teh_ignore              # ACA cleared handler routine
#
# ---   Read buffer command aborted
#       Waiting for write buffer command data transfer
#
task_etbl16f:
        .byte   inl2_ts_enbl            # task state code
        .byte   0,0,0                   # reserved
        .word   te16f_iocomp            # task I/O completion
                                        #  handler routine
        .word   teh_ignore              # MAGNITUDE request completion
                                        #  handler routine
        .word   teh_ignore              # SRP request handler routine
        .word   teh_ignore              # SRP request completion handler
                                        #  routine
        .word   te16f_abort             # abort task handler routine
        .word   te16f_abort             # reset event handler routine
        .word   te16f_abort             # offline event handler routine
        .word   teh_ignore              # ACA occurred handler routine
        .word   teh_ignore              # ACA cleared handler routine
#
# --- Datagram sent to Magnitude - Waiting for VRP completion
#
task_etbl16g:
        .byte   inl2_ts_enbl            # task state code
        .byte   0,0,0                   # reserved
        .word   teh_ignore              # task I/O completion
                                        #  handler routine
        .word   te16g_MAGcomp           # MAGNITUDE request completion
                                        #  handler routine
        .word   te16g_SRPreq            # SRP request handler routine
        .word   te16g_SRPcomp           # SRP request completion handler
                                        #  routine
        .word   te16g_abort             # abort task handler routine
        .word   te16g_abort             # reset event handler routine
        .word   te16g_abort             # offline event handler routine
        .word   teh_ignore              # ACA occurred handler routine
        .word   teh_ignore              # ACA cleared handler routine
#
# --- Waiting for VRP completion after write buffer command has been aborted
#
task_etbl16h:
        .byte   inl2_ts_enbl            # task state code
        .byte   0,0,0                   # reserved
        .word   teh_ignore              # task I/O completion
                                        #  handler routine
        .word   te16h_MAGcomp           # MAGNITUDE request completion
                                        #  handler routine
        .word   te6_srpreq              # SRP request handler routine
        .word   te6_srpcomp             # SRP request completion handler
                                        #  routine
        .word   te16h_abort             # abort task handler routine
        .word   te16h_abort             # reset event handler routine
        .word   te16h_abort             # offline event handler routine
        .word   teh_ignore              # ACA occurred handler routine
        .word   teh_ignore              # ACA cleared handler routine
#
# --- Waiting for VRP completion after read buffer command has been aborted
#
task_etbl16j:
        .byte   inl2_ts_enbl            # task state code
        .byte   0,0,0                   # reserved
        .word   teh_ignore              # task I/O completion
                                        #  handler routine
        .word   te16j_MAGcomp           # MAGNITUDE request completion
                                        #  handler routine
        .word   te6_srpreq              # SRP request handler routine
        .word   te6_srpcomp             # SRP request completion handler
                                        #  routine
        .word   te16j_abort             # abort task handler routine
        .word   te16j_abort             # reset event handler routine
        .word   te16j_abort             # offline event handler routine
        .word   teh_ignore              # ACA occurred handler routine
        .word   teh_ignore              # ACA cleared handler routine
#
# --- Waiting for VRP completion after read and write buffer commands
#     have been aborted
#
task_etbl16k:
        .byte   inl2_ts_enbl            # task state code
        .byte   0,0,0                   # reserved
        .word   teh_ignore              # task I/O completion
                                        #  handler routine
        .word   te16k_MAGcomp           # MAGNITUDE request completion
                                        #  handler routine
        .word   te6_srpreq              # SRP request handler routine
        .word   te6_srpcomp             # SRP request completion handler
                                        #  routine
        .word   teh_ignore              # abort task handler routine
        .word   teh_ignore              # reset event handler routine
        .word   teh_ignore              # offline event handler routine
        .word   teh_ignore              # ACA occurred handler routine
        .word   teh_ignore              # ACA cleared handler routine
#
# ---   Waiting for read buffer data transfer completion
#
task_etbl16l:
        .byte   inl2_ts_enbl            # task state code
        .byte   0,0,0                   # reserved
        .word   te16l_iocomp            # task I/O completion
                                        #  handler routine
        .word   teh_ignore              # MAGNITUDE request completion
                                        #  handler routine
        .word   teh_ignore              # SRP request handler routine
        .word   teh_ignore              # SRP request completion handler
                                        #  routine
        .word   te16l_abort             # abort task handler routine
        .word   te16l_abort             # reset event handler routine
        .word   te16l_abort             # offline event handler routine
        .word   teh_ignore              # ACA occurred handler routine
        .word   teh_ignore              # ACA cleared handler routine
#
# ---   process io completion of command while aborted during data transfer
#       and there is a secondary ILT associated with the task.
#
task_etbl16y:
        .byte   inl2_ts_enbl            # task state code
        .byte   0,0,0                   # reserved
        .word   te16y_iocomp            # task I/O completion
                                        #  handler routine
        .word   teh_ignore              # MAGNITUDE request completion
                                        #  handler routine
        .word   teh_ignore              # SRP request handler routine
        .word   teh_ignore              # SRP request completion handler
                                        #  routine
        .word   teh_ignore              # abort task handler routine
        .word   teh_ignore              # reset event handler routine
        .word   teh_ignore              # offline event handler routine
        .word   teh_ignore              # ACA occurred handler routine
        .word   teh_ignore              # ACA cleared handler routine
#
# ---   process io completion of command while aborted during data transfer
#
task_etbl16ya:
        .byte   inl2_ts_enbl            # task state code
        .byte   0,0,0                   # reserved
        .word   te16ya_iocomp           # task I/O completion
                                        #  handler routine
        .word   teh_ignore              # MAGNITUDE request completion
                                        #  handler routine
        .word   teh_ignore              # SRP request handler routine
        .word   teh_ignore              # SRP request completion handler
                                        #  routine
        .word   teh_ignore              # abort task handler routine
        .word   teh_ignore              # reset event handler routine
        .word   teh_ignore              # offline event handler routine
        .word   teh_ignore              # ACA occurred handler routine
        .word   teh_ignore              # ACA cleared handler routine
#
# ---   Ending status sent for write buffer command - waiting for completion.
#
task_etbl16z:
        .byte   inl2_ts_enbl            # task state code
        .byte   0,0,0                   # reserved
        .word   te16z_iocomp            # task I/O completion
                                        #  handler routine
        .word   teh_ignore              # MAGNITUDE request completion
                                        #  handler routine
        .word   teh_ignore              # SRP request handler routine
        .word   teh_ignore              # SRP request completion handler
                                        #  routine
        .word   te16z_abort             # abort task handler routine
        .word   te16z_abort             # reset event handler routine
        .word   te16z_abort             # offline event handler routine
        .word   teh_ignore              # ACA occurred handler routine
        .word   teh_ignore              # ACA cleared handler routine
#
# ---   (fast version) Ending status sent for write buffer
#       command - waiting for completion.
#
task_etbl16z_fast:
        .byte   inl2_ts_enbl            # task state code
        .byte   0,0,0                   # reserved
        .word   te16z_iocomp_fast       # task I/O completion
                                        #  handler routine
        .word   teh_ignore              # MAGNITUDE request completion
                                        #  handler routine
        .word   teh_ignore              # SRP request handler routine
        .word   teh_ignore              # SRP request completion handler
                                        #  routine
        .word   te16z_abort_fast        # abort task handler routine
        .word   te16z_abort_fast        # reset event handler routine
        .word   te16z_abort_fast        # offline event handler routine
        .word   teh_ignore              # ACA occurred handler routine
        .word   teh_ignore              # ACA cleared handler routine
#
        .text
#
#******************************************************************************
#
# ____________________ TASK EVENT HANDLER ROUTINES ____________________________
#
#******************************************************************************
#
#  NAME: te16a_abort
#
#  PURPOSE:
#       Provide the processing to abort the read buffer command
#       while waiting for the write buffer command.
#
#  CALLING SEQUENCE:
#       call    te16a_abort
#       Note: called from mag2_iocr
#
#  INPUT:
#       g0 = exchange ID of aborted task
#       g1 = aborted task ILT at lvl 2 (inl2)
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       None.
#
#******************************************************************************
#
te16a_abort:
#
# --- find the LDG temp storage area for this exchange
#
        ldob    inl2_lldID(g1),r4       # r4 = exchange ID
        ld      ilm_imt(g6),r5          # r5 = get imt address
        ld      im_ltmt(r5),r5          # r5 = ltmt address
        ld      ltmt_tmsg(r5)[r4*4],r7  # r7 = LDG temp storage area

        ld      sdg_readILT(r7),r4      # r4 = read buffer ILT
        cmpobe.t r4,g1,.te16a_abrt_100  # Jif read buffer ILT

#        ldob    lld_abt_cnt+00,r4
#        addo    1,r4,r4                 # add 1
#        stob    r4,lld_abt_cnt+00

        call    te16com_rtn2owner       # return ILT to owner
        b      .te16a_abrt_1000

.te16a_abrt_100:
        call    te16com_release_all     # release dg resources
        call    mag$chknextask          # check if next task needs to be enabled

.te16a_abrt_1000:
        ret                             # return to call

#******************************************************************************
#
#  NAME: te16b_abort
#
#  PURPOSE:
#       Provide the processing to abort the write buffer command
#       while waiting for the read buffer command.
#
#  CALLING SEQUENCE:
#       call    te16b_abort
#       Note: called from mag2_iocr
#
#  INPUT:
#       g0 = exchange ID of aborted task
#       g1 = aborted task ILT at lvl 2 (inl2)
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       None.
#
#******************************************************************************
#
te16b_abort:
#
# --- find the LDG temp storage area for this exchange
#
        ldob    inl2_lldID(g1),r4       # r4 = exchange ID
        ld      ilm_imt(g6),r5          # r5 = get imt address
        ld      im_ltmt(r5),r5          # r5 = ltmt address
        ld      ltmt_tmsg(r5)[r4*4],r7  # r7 = LDG temp storage area

        ld      sdg_writeILT(r7),r4     # r4 = write buffer ILT
        cmpobe.t r4,g1,.te16b_abrt_100  # Jif write buffer ILT

        call    te16com_rtn2owner       # return ILT to owner
        b       .te16b_abrt_1000

.te16b_abrt_100:
        call    te16com_release_all     # release dg resources
        call    mag$chknextask          # check if next task needs to be enabled

.te16b_abrt_1000:
        ret                             # return to call

#******************************************************************************
#
#  NAME: te16e_iocomp
#
#  PURPOSE:
#       Provide the processing to perform the task I/O
#       complete processing for an write buffer command that was
#       performing a data transfer before continuing to process
#       the command and was using a buffer.
#
#  DESCRIPTION:
#       Deallocates the resources allocated for the data transfer
#       and then send combined SGL to magnitude.
#
#  CALLING SEQUENCE:
#       call    te16e_iocomp
#       Note: called from mag2_iocr
#
#  INPUT:
#       g0 = I/O completion status code
#       g1 = pri. ILT at inl2 nest level
#       g2 = assoc. SGL address
#       g6 = assoc. ILMT address
#       g9 = sec. ILT 1st nest lvl
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
te16e_iocomp:
#
# --- get LDG temp storage for this exchange ID
#
        ldob    inl2_lldID(g1),r4       # r4 = lld exchange ID
        ld      ilm_imt(g6),r5          # r5 = get imt address
        ld      im_ltmt(r5),g4          # g4 = ltmt address
        ld      ltmt_tmsg(g4)[r4*4],r15 # r15 = possible LDG area
#
# --- determine if there was a transfer error
#
        mov     g0,r8                   # r8 = default possible error
        mov     0,r9                    # r9 = 0
        cmpobne 0,g0,.te16e_ioc_err     # jif errors
#
# --- no transfer error. check request header CRC, the length of
#     the header, and the destination s/n.
#
        ld      sdg_reqptr(r15),r12     # r12 = address of request header
        mov     0,r10                   # r10 = 0
        ldq     (r12),r4                # r4-r7 = header words 0-3
        stq     r4,sdg_reqhdr(r15)      # save words 0-3
        xor     r4,r5,r10               # calculate LRC
        xor     r6,r10,r10
        xor     r7,r10,r10
        ldq     16(r12),r4              # r4-r7 = header words 4-7
        stq     r4,sdg_reqhdr+16(r15)   # save words 4-7
        xor     r4,r10,r10              # calculate LRC
        xor     r5,r10,r10
        xor     r6,r10,r10
        xor     r7,r10,r10

        ldconst dgec1_dlld_crc,r8       # r8 = default ec1 (crc error)
        mov     1,r9                    # r9 = default ec2
        cmpobne 0,r10,.te16e_ioc_err    # Jif CRC error
        lda     sdg_reqhdr(r15),r12     # r12 = request header base adr

        ldob    dgrq_hdrlen(r12),r4     # r4 = request header length
        ldconst dgrq_size,r5            # r5 = my request header size
        ldconst dgec1_dlld_invhdr,r8    # r8 = default ec1 (invalid hdr)
        cmpobne r5,r4,.te16e_ioc_err    # Jif hdr length error
#
# --- determine if the serial number is correct. Compare it to the controller
#     serial number and the VCG serial number. If either match, the destination
#     serial number is correct.
#
        ld      K_ficb,r6               # r6 = FICB
        ld      fi_cserial(r6),r5       # r5 = Controller serial number
        ld      dgrq_dstsn(r12),r7      # r7 = destination s/n
        bswap   r7,r7                   # change endiance
        ldconst dgec1_dlld_badsn,r8     # r8 = default ec1 (invalid s/n)

.if     spec_INQ_new
        cmpobe r5,r7,.te16e_ioc_020     # Jif controller s/n is a match
        ld      fi_vcgid(r6),r5         # r8 = VCG serial number
        cmpobne r5,r7,.te16e_ioc_err    # Jif VCG does not match
.else   # spec_INQ_new
        cmpobne r5,r7,.te16e_ioc_err    # Jif s/n's don't match
.endif  # spec_INQ_new
#
# --- determine if the request is for LLD or something else
#
.if     spec_INQ_new
.te16e_ioc_020:
.endif  # spec_INQ_new

        ldob    dgrq_srvcpu(r12),r7     # r7 = server processor code
        cmpobne dg_cpu_interface,r7,.te16e_ioc_100 # Jif DG not for the intfc
        ldconst LLD0little,r4           # r4 = LLD0 name (little endian)
        ld      dgrq_srvname(r12),r5    # r5 = server name
        cmpobne r4,r5,.te16e_ioc_100    # Jif not for LLD - send FE DLM
#******************************************************************************
#
# --- The datagram is for LLD, process the request.
#
#******************************************************************************
        ld      sdg_reqptr(r15),g0      # g0 = request pointer
        ld      sdg_respptr(r15),g1     # g1 = response pointer
        call    lld$dg_server           # process data gram request
#
# --- release ILT" and continue
#
        mov     g9,g1                   # g1 = ILT"
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
        b       .te16g_MAG_complete     # send the completion

#******************************************************************************
#
# --- The datagram was not for LLD, send request to FE DLM.
#     Start by releasing the secondary ILT.
#
#******************************************************************************
#
.te16e_ioc_100:
        mov     g1,r14                  # save ILT'
        mov     g9,g1                   # g1 = ILT" at 1st lvl
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
        mov     r14,g1                  # restore ILT'
#
# --- set up event handler addresses
#
        ld      sdg_readILT(r15),r4     # r4 =  read buffer ILT
        lda     task_etbl16g,r5         # r5 = "waiting for VRP completion"
        st      r5,inl2_ehand(g1)       # save in write buffer ILT
        st      r5,inl2_ehand(r4)       # save in read buffer ILT
#
# --- allocate a VRP and save it's address in ILT'
#
c       g2 = get_vrp();                 # Allocate a VRP
.ifdef M4_DEBUG_VRP
c CT_history_printf("%s%s:%u get_vrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_VRP
        st      g2,-ILTBIAS+vrvrp(g1)   # save VRP address in pri. ILT

        lda     mag1_MAGcomp,r5         # r5 = ILT completion routine
        st      g6,inl2_ilmt(g1)        # save the ILMT address
        lda     mag1_srpreq,r4          # r4 = SRP Request handler (calls the
                                        #   entry in the "task_etbl16g" srp req)
        st      r5,inl2_cr(g1)          # save my completion handler in pri. ILT
        ld      sdg_reqlen(r15),r6      # r6 = request header and info length
        st      r4,inl2_rcvsrp(g1)      # save the srp request handler
        st      r6,inl2_dtlen(g1)       # save how much has been transferred

        ldconst vrmsgrcv,r4             # r4 = VRP request function code
        st      r4,vr_func(g2)          # save request function code, clear
                                        #  strategy, and status

        ld      ilm_cimt(g6),r4         # r4 = associated CIMT
        ldob    ci_num(r4),r4           # r4 = interface number
        stob    r4,vr_path(g2)          # save the interface number in the vrp
        ld      ilm_imt(g6),r4          # r4 = associated IMT
        ldos    im_tid(r4),r4           # r4 = target ID request came in on.
        stos    r4,vr_tid(g2)           # save the path number in the vrp

        ld      ltmt_dlmid(g4),r4       # r4 = data-link manager session id
        st      r4,vr_mrm_dlmid(g2)     # save data-link manager session ID
        st      g4,vr_mrm_lldid(g2)     # save link_level driver session session ID

        lda     -ILTBIAS(g1),g7         # g7 = ILT at FCAL level
        lda     ILTBIAS(g1),g1          # g1 = ILT at MAG interface nest level
        st      g7,inl3_FCAL(g1)        # save FC-AL pointer in ILT
#
# --- get SGL buffer address from SGL in LDG temp storage
#
        ld      sdg_sgl(r15),r14        # get SGL pointer from the LDG temp storage
        ld      sg_desc0+sg_addr(r14),r13 # r13 = request buffer address
#
# --- build up the RDRAM SGL header
#
        lda     sghdrsiz+sgdescsiz+sgdescsiz,r6 # r6 = sgl length
        mov     2,r7                    # r7 = descriptor count
        st      r6,sg_size(r13)         # save SGL length
        stos    r7,sg_scnt(r13)         # save descriptor count
        st      r13,vr_mrm_sgl(g2)      # save pointer to SGL
#
# --- set up request and response descriptor records from save area in LDG temp
#     storage
#
        ldq     sdg_reqptr(r15),r4
        stq     r4,sg_desc0+sg_addr(r13)# save desc 0 and desc 1
#
# --- make sure not SGL is described in VRP
#
        mov     0,r3
        st      r3,vr_sglptr(g2)        # clear SGL ptr. in VRP
        st      r3,vr_sglsize(g2)       # clear SGL size in VRP
        st      r3,vrpsiz+sg_size(g2)   # clear SGL length
        st      r3,vrpsiz+sg_scnt(g2)   # clear descriptor count
        st      r3,vr_pptr(g2)          # clear packet physical address field
#
        b       MAG$submit_vrp          # send VRP to MAGNITUDE
#
# --- Release ILT" and build up a error response header
#     and send it back.
#
.te16e_ioc_err:
        mov     g1,r4                   # save ILT'
        mov     g9,g1                   # g1 = ILT"
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
        mov     r4,g1                   # restore ILT'

        ld      sdg_respptr(r15),r13    # r13 = address of response header
        ldq     te16e_ioc_errtbl,r4     # load error temp plate
        stq     r4,dgrs_status(r13)     # save it in response header
        stob    r8,dgrs_ec1(r13)        # save error code 1  (ec1)
        stob    r9,dgrs_ec2(r13)        # save error code 2  (ec2)
        b       .te16g_MAG_complete     # send the completion

        .data
te16e_ioc_errtbl:
        .byte   dg_st_dlld,dgrs_size,0,0
        .word   0
        .word   0
        .word   0
#
#******************************************************************************
#
#  NAME: te16e_abort
#
#  PURPOSE:
#       Provide the processing to abort the read  or write buffer commands
#       while waiting for the write buffer data transfer to complete.
#
#  DESCRIPTION:
#       The aborted command has already been removed from the working queue,
#       so this does not have to be done, clear the read buffer entry in the
#       LDG temp storage, and return the read buffer ILT.
#
#  CALLING SEQUENCE:
#       call    te16c_abort
#       Note: called from mag2_iocr
#
#  INPUT:
#       g0 = exchange ID of aborted task
#       g1 = aborted task ILT at lvl 2 (inl2)
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
te16e_abt_sns1:
        .byte   dtxfern,scechk,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   sense_err1              # sense pointer
        .short  0                       # # SGL descriptors
        .short  sensesize               # sense length
#
        .text
#
te16e_abort:
        mov     0,r13
#
# --- find the LDG temp storage area for this exchange
#
        ldob    inl2_lldID(g1),r4       # r4 = exchange ID
        ld      ilm_imt(g6),r5          # r5 = get imt address
        ld      im_ltmt(r5),r5          # r5 = ltmt address
        ld      ltmt_tmsg(r5)[r4*4],r7  # r7 = LDG temp storage area
#
        ld      sdg_writeILT(r7),r4      # r4 = write buffer ILT
        cmpobe.f r4,g1,.te16e_abtrd_100  # Jif write command aborted
        ld      sdg_readILT(r7),r4      # r4 = read buffer ILT
        cmpobe.t r4,g1,.te16e_abtrd_50  # Jif read buffer ILT
#
# --- Not the read or write buffer commands in the LDG temp storage,
#     place it on the abort queue and set event handler to wait for
#     transfer complete
#
#        ldob    lld_abt_cnt+02,r4
#        addo    1,r4,r4                 # add 1
#        stob    r4,lld_abt_cnt+02

        lda     task_etbl16y,r3         # r3 = new task event handler table
        st      r3,inl2_ehand(g1)       # save new task event handler table
                                        #  in task ILT
        call    mag$qtask2aq            # queue task ILT to abort queue
        b       .te16e_abtrd_1000       # exit
#
# --- Read buffer command aborted
#       Return RB ILT to owner, clear it's entry in the LDG temp
#       storage, and set event handler table in wb ILT to wait for data
#       transfer completion.
#
.te16e_abtrd_50:
        ld      sdg_writeILT(r7),r4     # r4 = write buffer ILT
        lda     task_etbl16f,r5         # r5 = "rb abrtd- waiting for wb trans cmplt"
        st      r5,inl2_ehand(r4)       # save new task event handler table

        st      r13,sdg_readILT(r7)     # clear rb entry
        call    te16com_rtn2owner       # return read buffer ILT to owner
        b       .te16e_abtrd_1000       # exit
#
# --- Write buffer command aborted
#     Place the wb ILT on the abort queue and end the rb command with an CS.
#
.te16e_abtrd_100:
        movq    g8,r8                   # save g8-g11 in r8-r11
        lda     task_etbl16y,r3         # r3 = new task event handler table
        st      r3,inl2_ehand(g1)       # save new task event handler table
                                        #  in task ILT
        st      r13,sdg_writeILT(r7)    # clear wb entry
        call    mag$qtask2aq            # queue task ILT to abort queue

        ld      sdg_readILT(r7),g9      # g9 = read buffer ILT' at 2nd lvl
        lda     task_etbl16z,r5         # r5 = "rb abrtd- waiting for wb trans cmplt"
        st      r5,inl2_ehand(g9)       # save new task event handler table
        lda     -ILTBIAS(g9),g7         # g7 = ILT' at 1st lvl

        ldob    scchipi(g7),r11         # avoid a dead lock if reseting.
        ld      resilk,r4
        bbs     r11,r4,.te16e_abtrd_1000

# below actually completes the read buffer command with an IO/process terminated check condition
# this confilicts with the original comment a bit SMW.

# now to shut down the write commnad
c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT

        ldq     te16e_abt_sns1,r4       # load op. values into regs.
        stq     r4,xlcommand(g1)        # stuff op. req. record

        ld      te16e_abt_sns1+16,r4
        mov     g7,r5                   # r5 = assoc. ILT parameter structure
        mov     g9,r6                   # r6 = pri. ILT at inl2 nest level
        lda     mag2_iocr,r7            # r7 = I/O completion routine
        stt     r4,xlsgllen(g1)         # save sense length/SGL length/assoc.
        st      r7,otl2_cr(g1)          # save completion routine
                                        #  ILT param. structure/pri. ILT at
                                        #  inl2 nest level
        mov     g1,r7                   # r11 = my ILT nest level ptr.

        lda     ILTBIAS(g1),g1          # inc. to next nest level
        st      r7,otl3_OTL2(g1)        # save param. ptr. in next nest

        PushRegs(r3)                    # Save all G registers
        call    mag$ISP$receive_io      # issue channel directive
        PopRegsVoid(r3)                 # Restore all G registers

.te16e_abtrd_1000:
        ret                             # return to caller

#******************************************************************************
#
#  NAME: te16f_iocomp
#
#  PURPOSE:
#       Provide the processing of a data transfer completion of the
#       write buffer command after the read buffer command has been
#       aborted.
#
#  DESCRIPTION:
#       End the write buffer command witha check status error
#
#  CALLING SEQUENCE:
#       call    te16f_iocomp
#       Note: called from mag2_iocr
#
#  INPUT:
#       g0 = I/O completion status code
#       g1 = pri. ILT at inl2 nest level
#       g2 = assoc. SGL address
#       g6 = assoc. ILMT address
#       g9 = sec. ILT 1st nest lvl
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
        .data
te16f_ioc_sns1:
        .byte   dtxfern,scechk,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   sense_err1              # sense pointer
        .short  0                       # # SGL descriptors
        .short  sensesize               # sense length
#
        .text
#
te16f_iocomp:
        mov     g1,r12                  # 12 = ILT' at lvl 2
        lda     task_etbl16z,r5         # r5 = "rb abrtd- waiting for wb trans cmplt"
        st      r5,inl2_ehand(g1)       # save new task event handler table

        mov     g9,g1                   # g1 = ILT" at 1st lvl
#
# --- save parameters in secondary ILT
#
        ldq     te16f_ioc_sns1,r4       # load op. values into regs.
        stq     r4,xlcommand(g1)        # stuff op. req. record

        ld      te16f_ioc_sns1+16,r4
        lda     -ILTBIAS(r12),r5        # r5 = assoc. ILT parameter structure
        mov     r12,r6                  # r6 = pri. ILT at inl2 nest level
        lda     mag2_iocr,r11           # r11 = I/O completion routine
        stt     r4,xlsgllen(g1)         # save sense length/SGL length/assoc.
        st      r11,otl2_cr(g1)         # save completion routine
                                        #  ILT param. structure/pri. ILT at
                                        #  inl2 nest level
        mov     g1,r11                  # r11 = my ILT nest level ptr.

        lda     ILTBIAS(g1),g1          # inc. to next nest level
        st      r11,otl3_OTL2(g1)       # save param. ptr. in next nest

        PushRegs(r3)                    # Save all G registers
        call    mag$ISP$receive_io      # issue channel directive
        PopRegsVoid(r3)                 # Restore all G registers
        ret                             # return to caller

#******************************************************************************
#
#  NAME: te16f_abort
#
#  PURPOSE:
#       Provide the processing to abort the write buffer command after
#       the read buffer command has been aborted and the task is
#       waiting for the write buffer data transfer to complete.
#
#  DESCRIPTION:
#       Release all dg resources and check the next task
#
#  CALLING SEQUENCE:
#       call    te16f_abort
#       Note: called from mag2_iocr
#
#  INPUT:
#       g0 = exchange ID of aborted task
#       g1 = aborted task ILT at lvl 2 (inl2)
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
te16f_abort:
        mov     0,r13
#
# --- find the LDG temp storage area for this exchange
#
        ldob    inl2_lldID(g1),r4       # r4 = exchange ID
        ld      ilm_imt(g6),r5          # r5 = get imt address
        ld      im_ltmt(r5),r5          # r5 = ltmt address
        ld      ltmt_tmsg(r5)[r4*4],r7  # r7 = LDG temp storage area

        lda     task_etbl16y,r3         # r3 = default new task event handler table
        ld      sdg_writeILT(r7),r4     # r4 = write buffer ILT
        cmpobne.f r4,g1,.te16f_abtrd_100 # Jif not write buffer ILT

        st      r13,sdg_writeILT(r7)    # clear wb entry
        lda     task_etbl16ya,r3        # r3 = new task event handler table

.te16f_abtrd_100:
        st      r3,inl2_ehand(g1)       # save new task event handler table
                                        #  in task ILT
        call    mag$qtask2aq            # queue task ILT to abort queue

        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: te16g_MAGcomp
#
#  PURPOSE:
#       Perform the processing for a MAGNITUDE request completion
#       event write buffer command.
#
#  DESCRIPTION:
#       Returns the VRP and rebuilds the read buffer primary ILT and sends the
#       response data to the requesting magnitude.
#
#  CALLING SEQUENCE:
#       call    te16g_MAGcomp
#
#  INPUT:
#       g0 = VRP request completion status code
#       g1 = primary ILT of task at the inl2 nest level
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
te16g_MAGcomp:
        ldob    inl2_ecode(g1),r4        # r4 = local error code for task
        ld      -ILTBIAS+vrvrp(g1),g2    # g2 = assoc. VRP address
        cmpobne.f 0,g0,.te16g_MAG02      # Jif error completion status
        cmpobe.t 0,r4,.te16g_MAG10       # Jif no local error indicated
#
# --- non zero VRP completion
#
.te16g_MAG02:
# %%%%%%%%%  F I N I S H  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

#
# --- release VRP
#
.te16g_MAG10:
        mov     0,r13
        st      r13,-ILTBIAS+vrvrp(g1)  # clear vrvrp field in ILT
.ifdef M4_DEBUG_VRP
c CT_history_printf("%s%s:%u put_vrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_VRP
c       put_vrp(g2);                    # Deallocate VRP
#
# --- find the LDG temp storage area for this exchange
#
        ldob    inl2_lldID(g1),r4       # r4 = exchange ID
        ld      ilm_imt(g6),r5          # r5 = get imt address
        ld      im_ltmt(r5),g4          # g5 = ltmt address
        ld      ltmt_tmsg(g4)[r4*4],r15 # r15 = LDG temp storage area
#
# --- reconstruct read buffer command ILT' registers
#
.te16g_MAG_complete:
        ld      sdg_readILT(r15),g9     # g9 = ILT' at 2nd level (inl2)
        lda     -ILTBIAS(g9),g7         # g7 = ILT' at 1st level
#
# --- set up completion and event handlers.
#
        lda     task_etbl16l,r9         # r9 = task event handler table
        ld      sdg_writeILT(r15),r4    # r4 = write ILT' address at 2nd lvl
        mov     inl2_ps_finalio,r10     # r10 = new process state code
        lda     mag2_iocr,r12           # r12 = I/O completion routine
        st      r9,inl2_ehand(g9)       # save task event handler table in
        st      r9,inl2_ehand(r4)       # save task event handler table in
        stob    r10,inl2_pstate(g9)     # save task process state code
#
# --- set the data for the XL ILT area and generate a CRC for the
#     response header.
#
        ld      sdg_respptr(r15),r8     # r8 = response pointer
        ldt     (r8),r4                 # r4-r6 = header words 0-2
        xor     r5,r4,r4                # calculate CRC
        xor     r6,r4,r4
        st      r4,dgrs_crc(r8)         # save CRC value
        ldob    dgrs_status(r8),r14     # r14 = Response Status
        ld      dgrs_resplen(r8),r9     # r9 = response length
        bswap   r9,r9                   # change endiance
        lda     dgrs_size(r9),r9        # r9 = response length + header
        ldq     rb_tbl1,r4              # r4-r7 = I/O op. setup
        lda     sdg_desc0_ptr(r15),r6   # r6 = adr of  SGL descriptor list
        stl     r8,(r6)                 # save resp ptr and length
        ld      rb_tbl1+16,r8           # clear rel. offset
#
# --- Check for a CACHE memory read datagram. If successful, return the
#     requested data and status. If an error occurred, only send the
#     response status.
#
        ld      sdg_reqptr(r15),r9      # r9 = request pointer
        ldconst CAC0name,r11            # r11 = CAC0 Server name
# SMW- I think this is triggered when the datatgram fails with an error.
?       ld      dgrq_srvname(r9),r10    # r10 = Datagram Server
        cmpobne r10,r11,.te16g_MAG20    # Jif the request is not to Cache
# SMW- I think this is triggered when the datatgram fails with an error.
?       ldob    dgrq_fc(r9),r3          # r3 = function code
        cmpobne CAC0_fc_rdmem,r3,.te16g_MAG20 # Jif not a Read Memory
        cmpobne dg_st_ok,r14,.te16g_MAG20 # Jif error in processing the read

        ldob    dgrq_hdrlen(r9),r3      # r3 = Datagram Request Length
        addo    r3,r9,r9                # r9 = pointer to the descriptors

        ld      CAC0_rq_mem_addr(r9),r10 # Set up the Buffer Address
        lda     BE_ADDR_OFFSET(r10),r10 # Translate to a BE address

        ld      CAC0_rq_mem_len(r9),r11 # Set up the Buffer Length
        stl     r10,sdg_desc1_ptr(r15)  # save buffer ptr and length

        mov     2,r8                    # Set 2 SGLs, zero sense length

.te16g_MAG20:
#
# --- allocate a secondary ILT and fill it in
#
c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
        stq     r4,xlcommand(g1)        # save I/O param. block in ILT
        mov     g7,r9                   # r9 = pri. ILT param. structure
        mov     g9,r10                  # r10 = pri. ILT at inl2 nest level
        mov     0,r11                   # r11 = residual transfer length
        stq     r8,xlsgllen(g1)         # save
        st      r12,otl2_cr(g1)         # save completion routine

        mov     g1,r11                  # r11 = my ILT nest level ptr.
#
        lda     ILTBIAS(g1),g1          # inc. to next nest level
        movt    g12,r12                 # save g12-g14
        st      r11,otl3_OTL2(g1)       # save param. ptr. in next nest
        call    ISP$receive_io          # issue channel directive
        movt    r12,g12                 # restore g12-g14
        ret
#
#******************************************************************************
#
#  NAME: te16g_SRPreq
#
#  PURPOSE:
#       Perform the processing for an SRP request event associated with
#       a datagram type command.
#
#  DESCRIPTION:
#       Sets up a request to the FC-AL level and issues the
#       I/O request to the FC-AL driver.
#
#  CALLING SEQUENCE:
#       call    te16g_SRPreq
#
#  INPUT:
#       g1 = sec. ILT at otl2 nest level
#       g2 = SRP address
#       g6 = assoc. ILMT address
#       g7 = pri. ILT at inl1 nest level
#       g9 = pri. ILT at XL nest level
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
        .data
dgwrite_tbl16g:
        .byte   dtxferc,scnorm,0,0
dgread_tbl16g:
        .byte   dtxferi,scnorm,0,0
#
        .text
#
te16g_SRPreq:
        movt    g12,r12                 # save g12-g14
        ldob    sr_func(g2),r5          # r5 = SRP function code
        cmpobe  srh2c,r5, e16g_srp10    # Jif function code is a request
        ld      dgread_tbl16g,r4        # r4 = I/O op. setup
        cmpobe  src2h,r5, e16g_srp20    # Jif function code is a response
        ldconst ecinvfunc,g0            # return SRP with error
        ldconst inl2_ps_srpcomp,r3      # r3 = new task process state code
        stob    g0,inl2_ecode(g9)       # save error code in inl2 data structure
        stob    r3,inl2_pstate(g9)      # save new task process state code
        lda     -ILTBIAS(g1),g1         # back up to previous nest level
        ld      otl1_cr(g1),r4          # r4 = MAG interface completion routine
        callx   (r4)                    # and call it.
        b       e16g_srp1000            # and we're out of here
#
 e16g_srp10:
        ld      dgwrite_tbl16g,r4       # r4 = I/O op. setup
 e16g_srp20:
        ld      sr_count(g2),r8         # r8 = SGL descriptor count
        lda     sr_desc0(g2),g3         # g3 = SRP SGL pointer
        mov     0,r7                    # r7 = sense data pointer (null)
        mov     g3,r6                   # r6 = SGL pointer for I/O operation
        ld      inl2_dtlen(g9),r5       # r5 = offset (how much already xfered)
        mov     r8,g5                   # g5 = SGL descriptor count
        mov     g3,g4                   # g4 = pointer to SRP SGL records
        mov     r5,r15                  # r15 = cumulative xfer
#
# --- Build I/O SGL list from SRP SGL
#
 e16g_srp40:
        ld      sr_dest(g4),r10         # r10 = destination address of segment
        ld      sr_len(g4),r11          # r11 = segment length
        subo    1,g5,g5                 # dec. segment count
        lda     srpesiz(g4),g4          # inc. to next SRP SGL record
        stl     r10,sg_addr(g3)         # save in I/O SGL
        lda     sgdescsiz(g3),g3        # inc. to next I/O SGL record
        addo    r11,r15,r15             # r15 = cumulative xfer so far
        cmpobne 0,g5, e16g_srp40        # Jif more segments to translate
        stq     r4,xlcommand(g1)        # save I/O param. block in ILT
        mov     g7,r9                   # r9 = pri. ILT param. structure
        mov     g9,r10                  # r10 = pri. ILT at inl2 nest level
        ldconst 0,r11                   # r11 = clear the residual length field
        stq     r8,xlsgllen(g1)
        mov     inl2_ps_srpact,r3       # r3 = new task process state code
        st      r15,inl2_dtlen(g9)      # save the new data xfer count
        lda     mag1_srpcomp,r5         # r5 = my completion handler routine
        mov     g1,r10                  # r10 = sec. ILT at otl2 nest level
        stob    r3,inl2_pstate(g9)      # save new process state code
        st      r5,otl2_cr(g1)          # save completion handler in ILT
#
        lda     ILTBIAS(g1),g1          # bump to next ILT nest level
        st      r10,otl3_OTL2(g1)       # save pointer to I/O param. block
#
        call    ISP$receive_io          # and issue I/O request to FC-AL driver
 e16g_srp1000:
        movt    r12,g12                 # restore g12-g14
        ret
#
#******************************************************************************
#
#  NAME: te16g_SRPcomp
#
#  PURPOSE:
#       Processes a FC-AL I/O request completion event associated
#       with an SRP request associated with a datagram type command.
#
#  DESCRIPTION:
#       Checks for errors and if any occurred returns the appropriate
#       error code to the requesting level. Else returns the
#       requested data to the original requester.
#
#  CALLING SEQUENCE:
#       call    te16g_SRPcomp
#
#  INPUT:
#       g0 = SRP I/O completion status
#       g1 = sec. SRP ILT address at OLT2 nest level
#       g6 = assoc. ILMT address
#       g9 = pri. ILT at inl2 nest level
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
te16g_SRPcomp:
#
# --- Check for I/O error reported from FC-AL driver
#
        cmpobe  0,g0, e16gsrpcomp_10    # Jif no error indicated
        ldconst ec_ioerr,r3             # r3 = my local error code
        stob    r3,inl2_ecode(g9)       # save local error code in inl2
#
# --- Simulate abort on task
#
        movl    g0,r14                  # save g0-g1
        ld      -ILTBIAS+scrxid(g9),g0  # g0 = exchange ID of task to abort
        ld      ilm_cimt(g6),g4         # g4 = assoc. CIMT address
        ld      ilm_imt(g6),g5          # g5 = assoc. IMT address
        mov     g9,g1                   # g1 = Primary ILT at level 2 (inl2)
        call    mag$remtask             # remove the task from the active queue
        call    te16g_abort             # abort task processing
        movl    r14,g0                  # restore g0-g1
 e16gsrpcomp_10:
#
        ldconst inl2_ps_srpcomp,r3      # r3 = new task process state code
        lda     -ILTBIAS(g1),g1         # back up to previous nest level in ILT
        ld      otl1_cr(g1),r4          # get completion handler routine from ILT
        movt    g12,r12                 # save g12-g14
        stob    r3,inl2_pstate(g9)      # save new task process state
        callx   (r4)                    # and go to it
        movt    r12,g12                 # restore g12-g14
        ret
#
#******************************************************************************
#
#  NAME: te16g_abort
#
#  PURPOSE:
#       Perform the processing for a command abort while waiting for a
#       MAGNITUDE request completion.
#
#  CALLING SEQUENCE:
#       call    te16g_abort
#       Note: called from mag2_iocr
#
#  INPUT:
#       g0 = exchange ID of aborted task
#       g1 = aborted task ILT at lvl 2 (inl2)
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
te16g_abort:
        movq    g4,r8                  # save g4-g7 in r8-r11
#
# --- find the LDG temp storage area for this exchange
#
        ldob    inl2_lldID(g1),r4       # r4 = exchange ID
        ld      ilm_imt(g6),r5          # r5 = get imt address

        ld      im_ltmt(r5),g4          # g4 = ltmt address
        ld      ltmt_tmsg(g4)[r4*4],r7  # r7 = LDG temp storage area

        ld      sdg_writeILT(r7),r4     # r4 = write buffer ILT
        cmpobe.f r4,g1,.te16g_abtrd_100 # Jif write command aborted
        ld      sdg_readILT(r7),r4      # r4 = read buffer ILT
        cmpobe.t r4,g1,.te16g_abtrd_50  # Jif rb ILT

        lda     task_etbl16k,r5         # r5 = "wait for vrp or cmd cmplt"
        st      r5,inl2_ehand(g1)       # save new task event handler table
        call    mag$qtask2aq            # queue task ILT to abort queue
        b       .te16g_abtrd_1000
#
# --- read buffer command aborted
#
.te16g_abtrd_50:
        ld      sdg_writeILT(r7),r4     # r4 = write buffer ILT
        lda     task_etbl16j,r5         # r5 = "rb abrtd- waiting for VRP cmplt"
        st      r5,inl2_ehand(r4)       # save new task event handler table

        mov     0,r13
        st      r13,sdg_readILT(r7)     # clear rb entry
        call    te16com_rtn2owner       # return read buffer ILT to owner
        b       .te16g_abtrd_1000
#
# --- write buffer command aborted
#
.te16g_abtrd_100:
        lda     task_etbl16h,r5         # r5 = "wait vrp or cmd cmplt"
        st      r5,inl2_ehand(g1)       # save new task event handler table
        call    mag$qtask2aq            # queue task ILT to abort queue

        ld      sdg_readILT(r7),r4      # r4 = read buffer ILT
        lda     task_etbl16h,r5         # r5 = "wb abrtd- waiting for VRP completion"
        st      r5,inl2_ehand(r4)       # save new task event handler table

.te16g_abtrd_1000:
        movq    r8,g4                   # restore g4-g7 from r8-r11
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: te16h_MAGcomp
#
#  PURPOSE:
#       Perform the processing for a MAGNITUDE request completion
#       event after the write buffer command has been aborted
#
#  DESCRIPTION:
#       Returns the VRP and rebuilds the read buffer primary ILT and sends the
#       response data to the requesting magnitude.
#
#  CALLING SEQUENCE:
#       call    te16h_MAGcomp
#
#  INPUT:
#       g0 = VRP request completion status code
#       g1 = primary ILT of task at the inl2 nest level
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
te16h_MAGcomp:
        mov     0,r13
#
# --- release VRP
#
        ld      -ILTBIAS+vrvrp(g1),g2    # g2 = assoc. VRP address
        st      r13,-ILTBIAS+vrvrp(g1)  # clear vrvrp field in ILT
.ifdef M4_DEBUG_VRP
c CT_history_printf("%s%s:%u put_vrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_VRP
c       put_vrp(g2);                    # Deallocate VRP
#
# --- find the LDG temp storage area for this exchange
#
        ldob    inl2_lldID(g1),r4       # r4 = exchange ID
        ld      ilm_imt(g6),r5          # r5 = get imt address
        ld      im_ltmt(r5),g4          # g5 = ltmt address
        ld      ltmt_tmsg(g4)[r4*4],r15 # r15 = LDG temp storage area
#
# --- set up to return wb to owner
#
        st      r13,sdg_writeILT(r15)   # clear wb entry
#        call    mag$remtask             # remove wb ILT from abort queue
        call    te16com_rtn2owner       # return wb ILT to owner
#
# --- reconstruct read buffer command ILT' registers
#
        ld      sdg_readILT(r15),g9     # g9 = ILT' at 2nd level (inl2)
        lda     -ILTBIAS(g9),g7         # g7 = ILT' at 1st level
#
# --- set up completion and event handlers.
#
        lda     task_etbl16z,r9         # r9 = task event handler table
        mov     inl2_ps_finalio,r10     # r10 = new process state code
        lda     mag2_iocr,r11           # r11 = I/O completion routine
        st      r9,inl2_ehand(g9)       # save task event handler table in
        stob    r10,inl2_pstate(g9)     # save task process state code
#
# --- set the data for the XL ILT area and generate a CRC for the
#     response header.
#
        ld      sdg_respptr(r15),r8     # r8 = response pointer
        ldt     (r8),r4                 # r4-r6 = header words 0-2
        xor     r5,r4,r4                # calculate CRC
        xor     r6,r4,r4
        st      r4,dgrs_crc(r8)         # save CRC value
        ld      dgrs_resplen(r8),r9     # r9 = response length
        bswap   r9,r9                   # change endiance
        lda     dgrs_size(r9),r9        # r9 = response length + header
        ldq     rb_tbl1,r4              # r4 = I/O op. setup
        lda     sdg_desc0_ptr(r15),r6   # r6 = adr of  SGL descriptor list
        stl     r8,(r6)                 # save resp ptr and length
        ldl     rb_tbl1+16,r8           # clear rel. offset
#
# --- allocate a secondary ILT and fill it in
#
c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
        stq     r4,xlcommand(g1)        # save I/O param. block in ILT
        mov     g7,r9                   # r9 = pri. ILT param. structure
        mov     g9,r10                  # r10 = pri. ILT at inl2 nest level
        stt     r8,xlsgllen(g1)         # save
        st      r11,otl2_cr(g1)         # save completion routine

        mov     g1,r11                  # r11 = my ILT nest level ptr.
#
        lda     ILTBIAS(g1),g1          # inc. to next nest level
        movt    g12,r12                 # save g12-g14
        st      r11,otl3_OTL2(g1)       # save param. ptr. in next nest
        call    ISP$receive_io          # issue channel directive
        movt    r12,g12                 # restore g12-g14
        ret

#******************************************************************************
#
#  NAME: te16h_abort
#
#  PURPOSE:
#       Perform the processing for a read buffer command abort while waiting
#       for a MAGNITUDE request completion.
#
#  CALLING SEQUENCE:
#       call    te16h_abort
#       Note: called from mag2_iocr
#
#  INPUT:
#       g0 = exchange ID of aborted task
#       g1 = aborted task ILT at lvl 2 (inl2)
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
te16h_abort:
        movq    g4,r8                  # save g4-g7 in r8-r11
        ldob    inl2_lldID(g1),r4       # r4 = exchange ID
        ld      ilm_imt(g6),r5          # r5 = get imt address
        ld      im_ltmt(r5),g4          # g4 = ltmt address
        ld      ltmt_tmsg(g4)[r4*4],r7  # r7 = LDG temp storage area

        ld      sdg_readILT(r7),r4      # r4 = read buffer ILT
        cmpobne.f r4,g1,.te16h_abtrd_100 # Jif read command not aborted


        ld      sdg_writeILT(r7),r4     # r4 =  write buffer ILT
        lda     task_etbl16k,r5         # r5 = "wb & rb abort - wait for vrp cmplt"
        st      r5,inl2_ehand(r4)       # save new task event handler table

        mov     0,r13
        st      r13,sdg_readILT(r7)     # clear rb entry

.te16h_abtrd_100:
        call    te16com_rtn2owner       # return rb ILT to owner
        movq    r8,g4                   # restore g4-g7 from r8-r11
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: te16j_MAGcomp
#
#  PURPOSE:
#       Perform the processing for a MAGNITUDE request completion
#       event after the read buffer command has been aborted
#
#  DESCRIPTION:
#       Returns the VRP and end write buffer command with check status.
#
#  CALLING SEQUENCE:
#       call    te16j_MAGcomp
#
#  INPUT:
#       g0 = VRP request completion status code
#       g1 = primary ILT of task at the inl2 nest level
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
        .data
te16j_ioc_sns1:
        .byte   dtxfern,scechk,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   sense_err1              # sense pointer
        .short  0                       # # SGL descriptors
        .short  sensesize               # sense length
#
        .text
#
te16j_MAGcomp:
#
# --- release VRP
#
        mov     0,r13
        ld      -ILTBIAS+vrvrp(g1),g2   # g2 = assoc. VRP address
        st      r13,-ILTBIAS+vrvrp(g1)  # clear vrvrp field in ILT
.ifdef M4_DEBUG_VRP
c CT_history_printf("%s%s:%u put_vrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_VRP
c       put_vrp(g2);                    # Deallocate VRP
#
# --- find the LDG temp storage area for this exchange
#
        ldob    inl2_lldID(g1),r4       # r4 = exchange ID
        ld      ilm_imt(g6),r5          # r5 = get imt address
        ld      im_ltmt(r5),r5          # r5 = ltmt address
        ld      ltmt_tmsg(r5)[r4*4],r7  # r7 = LDG temp storage area
#
# --- end write buffer command with sense.
#
        ld      sdg_writeILT(r7),g9     # g9 = write buffer ILT' at 2nd lvl
        lda     task_etbl16z,r5         # r5 = "rb abrtd-waiting for wb xfr cmp"
        st      r5,inl2_ehand(g9)       # save new task event handler table
        lda     -ILTBIAS(g9),g7         # g7 = ILT' at 1st lvl

c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT

        ldq     te16j_ioc_sns1,r4       # load op. values into regs.
        stq     r4,xlcommand(g1)        # stuff op. req. record

        ld      te16j_ioc_sns1+16,r4
        mov     g7,r5                   # r5 = assoc. ILT parameter structure
        mov     g9,r6                   # r6 = pri. ILT at inl2 nest level
        lda     mag2_iocr,r7            # r7 = I/O completion routine
        stt     r4,xlsgllen(g1)         # save sense length/SGL length/assoc.
        st      r7,otl2_cr(g1)          # save completion routine
                                        #  ILT param. structure/pri. ILT at
                                        #  inl2 nest level
        mov     g1,r7                   # r7 = my ILT nest level ptr.

        lda     ILTBIAS(g1),g1          # inc. to next nest level
        st      r7,otl3_OTL2(g1)        # save param. ptr. in next nest

        PushRegs(r3)                    # Save all G registers
        call    mag$ISP$receive_io      # issue channel directive
        PopRegsVoid(r3)                 # Restore all G registers
        ret                             # return to caller

#******************************************************************************
#
#  NAME: te16j_abort
#
#  PURPOSE:
#       Perform the processing for a write buffer command abort after a
#       read command abort while waiting for VRP completion.
#
#  CALLING SEQUENCE:
#       call    te16j_abort
#       Note: called from mag2_iocr
#
#  INPUT:
#       g0 = exchange ID of aborted task
#       g1 = aborted task ILT at lvl 2 (inl2)
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
te16j_abort:
        lda     task_etbl16k,r5         # r5 = "rb and wb abrtd- waiting for vrp cmplt"
        st      r5,inl2_ehand(g1)       # save new task event handler table
        call    mag$qtask2aq            # queue task ILT to abort queue
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: te16k_MAGcomp
#
#  PURPOSE:
#       Perform the processing for a VRP completion after the read and
#       write buffer commands have been aborted.
#
#  CALLING SEQUENCE:
#       call    te16k_MAGcomp
#
#  INPUT:
#       g0 = VRP request completion status code
#       g1 = primary ILT of task at the inl2 nest level
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
te16k_MAGcomp:
        movq    g4,r8                   # save g4-g7 in r8-r11
        mov     0,r13
        ld      -ILTBIAS+vrvrp(g1),g2   # g2 = assoc. VRP address
        st      r13,-ILTBIAS+vrvrp(g1)  # clear vrvrp field in ILT
.ifdef M4_DEBUG_VRP
c CT_history_printf("%s%s:%u put_vrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_VRP
c       put_vrp(g2);                    # Deallocate VRP

        ldob    inl2_lldID(g1),r4       # r4 = exchange ID
        ld      ilm_imt(g6),r5          # r5 = get imt address
        ld      im_ltmt(r5),g4          # g4 = ltmt address
        ld      ltmt_tmsg(g4)[r4*4],r7  # r7 = LDG temp storage area

        ld      sdg_writeILT(r7),r4     # r4 = write buffer ILT
        cmpobe.t r4,g1,.te16k_MAGcomp_100 # Jif write buffer command

        call    te16com_rtn2owner       # return ILT to owner
        b       .te16k_MAGcomp_1000

.te16k_MAGcomp_100:
        call    te16com_release_all     # release dg resources
        call    mag$chknextask          # check if next task needs to be enabled

.te16k_MAGcomp_1000:
        movq    r8,g4                   # restore g4-g7 from r8-r11
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: te16l_iocomp
#
#  PURPOSE:
#       Provide the processing to perform the task I/O complete processing
#       for an read buffer command that was performing a data transfer and
#       final completion status.
#
#  DESCRIPTION:
#       Restore the Write Buffer command ILT and send completion.
#
#  CALLING SEQUENCE:
#       call    te16l_iocomp
#       Note: called from mag2_iocr
#
#  INPUT:
#       g0 = I/O completion status code
#       g1 = pri. ILT at inl2 nest level
#       g2 = assoc. SGL address
#       g6 = assoc. ILMT address
#       g9 = sec. ILT 1st nest lvl
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
te16l_iocomp:
        movt    g12,r12                 # save g12-g14
        mov     0,r3                    # r3 = clearing register
#
# --- find the LDG temp storage area for this exchange
#
        ldob    inl2_lldID(g1),r4       # r4 = exchange ID
        ld      ilm_imt(g6),r5          # r5 = get imt address
        ld      im_ltmt(r5),g4          # g4 = ltmt address
        ld      ltmt_tmsg(g4)[r4*4],r15 # r15 = LDG temp storage area
#
# --- isolate write ILT from the SDG and release SDG.
#
        mov     g9,r4                   # save ILT" pointer at 1st LVL
        ld      sdg_writeILT(r15),g9    # g9 = write ILT' at 2nd LVL
        st      r3,sdg_writeILT(r15)    # clear SDG write ILT pointer
        call    te16com_release_all     # release SDG
#
# --- reconstruct write buffer command ILT' registers
#
        mov     r4,g1                   # g1 = ILT" at 1st level
        lda     -ILTBIAS(g9),g7         # g7 = ILT' at 1st level (xl)
#
# --- set up completion and event handlers.
#
        lda     task_etbl16z_fast,r9    # r9 = task event handler table
        mov     inl2_ps_finalio,r10     # r10 = new process state code
        lda     mag2_iocr,r11           # r11 = I/O completion routine
        st      r9,inl2_ehand(g9)       # save task event handler table in
        stob    r10,inl2_pstate(g9)     # save task process state code
#
# --- set the data for the XL ILT area
#
        ldq     wb_tbl2,r4              # r4 = I/O op. setup
        ldl     wb_tbl2+16,r8

        stq     r4,xlcommand(g1)        # save I/O param. block in ILT
        mov     g7,r9                   # r9 = pri. ILT param. structure
        mov     g9,r10                  # r10 = pri. ILT at inl2 nest level
        stt     r8,xlsgllen(g1)         # save
        st      r11,otl2_cr(g1)         # save completion routine
        st      r3,xlreslen(g1)         # clear reslen value

        mov     g1,r11                  # r11 = my ILT nest level ptr.
#
        lda     ILTBIAS(g1),g1          # inc. to next nest level
        st      r11,otl3_OTL2(g1)       # save param. ptr. in next nest
        call    ISP$receive_io          # issue channel directive
        movt    r12,g12                 # restore g12-g14

        ret                             # return to call
#
#******************************************************************************
#
#  NAME: te16l_abort
#
#  PURPOSE:
#       Provide the processing to abort the read  or write buffer commands
#       while waiting for the read buffer data transfer to complete.
#
#  DESCRIPTION:
#       The aborted command has already been removed from the working queue,
#       so this does not have to be done, clear the read buffer entry in the
#       LDG temp storage, and return the read buffer ILT.
#
#  CALLING SEQUENCE:
#       call    te16l_abort
#       Note: called from mag2_iocr
#
#  INPUT:
#       g0 = exchange ID of aborted task
#       g1 = aborted task ILT at lvl 2 (inl2)
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
        .data
te16l_abt_sns1:
        .byte   dtxfern,scechk,xl_sndsc,0
        .word   0                       # rel. offset
        .word   0                       # SGL pointer
        .word   sense_err1              # sense pointer
        .short  0                       # # SGL descriptors
        .short  sensesize               # sense length
#
        .text
#
te16l_abort:
        mov     0,r13
#
# --- find the LDG temp storage area for this exchange
#
        ldob    inl2_lldID(g1),r4       # r4 = exchange ID
        ld      ilm_imt(g6),r5          # r5 = get imt address
        ld      im_ltmt(r5),r5          # r5 = ltmt address
        ld      ltmt_tmsg(r5)[r4*4],r7  # r7 = LDG temp storage area
#
        ld      sdg_readILT(r7),r4       # r4 = read buffer ILT
        cmpobe.f r4,g1,.te16l_abtrd_100  # Jif read command aborted
        ld      sdg_writeILT(r7),r4      # r4 = write buffer ILT
        cmpobe.t r4,g1,.te16l_abtrd_50   # Jif write buffer ILT

        lda     task_etbl16y,r3         # r3 = new task event handler table
        st      r3,inl2_ehand(g1)       # save new task event handler table
                                        #  in task ILT
        call    mag$qtask2aq            # queue task ILT to abort queue
        b       .te16l_abtrd_1000       # exit
#
# --- write buffer command aborted
#       Return the write buffer ILT to owner, clear it's entry in the LDG temp
#       storage, and set event handler table in rb ILT to wait for data
#       transfer completion.
#
.te16l_abtrd_50:
        ld      sdg_readILT(r7),r4      # r4 = write buffer ILT
        lda     task_etbl16z,r5         # r5 = "waiting for ending trans cmplt"
        st      r5,inl2_ehand(r4)       # save new task event handler table
        st      r13,sdg_writeILT(r7)    # clear wb entry
        call    te16com_rtn2owner       # return wb ILT to owner
        b       .te16l_abtrd_1000       # exit
#
# --- Read buffer command aborted
#     Place the rb ILT on the abort queue and end the wb command with an CS.
#
.te16l_abtrd_100:
        movq    g8,r8                   # save g8-g11 in r8-r11
#
# --- change read buffer event handler, place rb ILT on abort queue, and clear
#     rb ILT from datagram temp storage.
#
        lda     task_etbl16y,r3         # r3 = new task event handler table
        st      r3,inl2_ehand(g1)       # save new task event handler table
                                        #  in task ILT
        st      r13,sdg_readILT(r7)     # clear rb entry
        call    mag$qtask2aq            # queue task ILT to abort queue
#
# --- end write buffer command with sense.
#
        ld      sdg_writeILT(r7),g9     # g9 = write buffer ILT' at 2nd lvl
        lda     task_etbl16z,r5         # r5 = "rb abrtd- waiting for wb trans cmplt"
        st      r5,inl2_ehand(g9)       # save new task event handler table
        lda     -ILTBIAS(g9),g7         # g7 = ILT' at 1st lvl

        ldob    scchipi(g7),r11         # avoid a dead lock if reseting.
        ld      resilk,r4
        bbs     r11,r4,.te16l_abtrd_1000

c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT

        ldq     te16l_abt_sns1,r4       # load op. values into regs.
        stq     r4,xlcommand(g1)        # stuff op. req. record

        ld      te16l_abt_sns1+16,r4
        mov     g7,r5                   # r5 = assoc. ILT parameter structure
        mov     g9,r6                   # r6 = pri. ILT at inl2 nest level
        lda     mag2_iocr,r7            # r7 = I/O completion routine
        stt     r4,xlsgllen(g1)         # save sense length/SGL length/assoc.
        st      r7,otl2_cr(g1)          # save completion routine
                                        #  ILT param. structure/pri. ILT at
                                        #  inl2 nest level
        mov     g1,r7                   # r11 = my ILT nest level ptr.

        lda     ILTBIAS(g1),g1          # inc. to next nest level
        st      r7,otl3_OTL2(g1)        # save param. ptr. in next nest

        PushRegs(r3)                    # Save all G registers
        call    mag$ISP$receive_io      # issue channel directive
        PopRegsVoid(r3)                 # Restore all G registers

.te16l_abtrd_1000:
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: te16y_iocomp
#
#  PURPOSE:
#       Provide the processing to perform the task I/O
#       complete processing for an aborted command that was
#       performing a data transfer before continuing to process
#       the command.
#
#  DESCRIPTION:
#       te16y_iocomp - deallocates ILT", removes ILT' from abort queue,
#                      and returns ILT' to owner.
#
#  CALLING SEQUENCE:
#       call    te16y_iocomp
#       call    te16x_iocomp
#       Note: called from mag2_iocr
#
#  INPUT:
#     COMMON:
#       g0 = I/O completion status code
#       g1 = pri. ILT at inl2 nest level
#       g2 = assoc. SGL address
#       g6 = assoc. ILMT address
#       g9 = sec. ILT at otl2 nest level
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
te16y_iocomp:
        mov     g1,g8                   # g8 = pri. ILT at inl2 nest level
        mov     g9,g1                   # g1 = sec. ILT to deallocate
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
        mov     g8,g1                   # g1 = pri. ILT at inl2 nest level
#        call    mag$remtask             # remove task ILT from abort queue
        call    te16com_rtn2owner       # return ILT to owner
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: te16ya_iocomp
#
#  PURPOSE:
#       Provide the processing to perform the task I/O
#       complete processing for an aborted command that was
#       performing a data transfer before continuing to process
#       the command.
#
#  DESCRIPTION:
#       Deallocates the resources allocated for the data transfer
#       and then terminates the task.
#
#  CALLING SEQUENCE:
#       call    te16ya_iocomp
#       Note: called from mag2_iocr
#
#  INPUT:
#       g0 = I/O completion status code
#       g1 = pri. ILT at inl2 nest level
#       g2 = assoc. SGL address
#       g6 = assoc. ILMT address
#       g9 = sec. ILT at otl2 nest level
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
te16ya_iocomp:
        mov     g1,g8                   # g8 = pri. ILT at inl2 nest level
        mov     g9,g1                   # g1 = sec. ILT to deallocate
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
        mov     g8,g1                   # g1 = pri. ILT at inl2 nest level

        call    te16com_release_all     # release dg resources
#        call    mag$remtask             # remove task ILT from abort queue
        call    te16com_rtn2owner       # return ILT to owner

        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: te16z_iocomp_fast
#
#  PURPOSE:
#
#       Provide the processing to perform the task I/O complete processing
#       for Write buffer commands that was final completion status
#       and require release secondary ILT and the return of the Primary
#       ILT.
#
#  CALLING SEQUENCE:
#       Note: called from mag2_iocr
#
#  INPUT:
#     COMMON:
#       g0 = I/O completion status code
#       g1 = pri. ILT at inl2 nest level
#       g2 = assoc. SGL address
#       g6 = assoc. ILMT address
#       g9 = sec. ILT at otl2 nest level
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       None.
#
#******************************************************************************
#
te16z_iocomp_fast:
        movq    g0,r8                   # save g0-g3
        movq    g4,r12                  # save g4-g7

        mov     g1,r4                   # r4 = pri. ILT at inl2 nest level
        mov     g9,g1                   # g1 = sec. ILT to deallocate
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT

        mov     r4,g1                   # g1 = pri. ILT at inl2 nest level
        call    te16com_rtn2owner       # return ILT to owner

        call    mag$chknextask          # check if next task needs to be enabled

        movq    r8,g0                   # restore g0-g3
        movq    r12,g4                  # restore g4-g7
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: te16z_abort_fast
#
#  PURPOSE:
#       Provide the processing to abort the write buffer command while
#       waiting for final transfer
#
#  DESCRIPTION:
#       Place Primary ILT on abort queue
#
#  CALLING SEQUENCE:
#       Note: called from mag2_iocr
#
#  INPUT:
#       g0 = exchange ID of aborted task
#       g1 = aborted task ILT at lvl 2 (inl2)
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       None.
#
#******************************************************************************
#
te16z_abort_fast:
        call    mag$qtask2aq            # queue task ILT to abort queue
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: te16z_iocomp
#
#  PURPOSE:
#       Provide the processing to perform the task I/O complete processing
#       for an Read or Write buffer commands that was final completion status
#       and require release of datagram resources.
#
#  DESCRIPTION:
#       Release secondary ILT, release dg resources, and check for next task.
#
#  CALLING SEQUENCE:
#       call    te16f_iocomp
#       Note: called from mag2_iocr
#
#  INPUT:
#       g0 = I/O completion status code
#       g1 = pri. ILT at inl2 nest level
#       g2 = assoc. SGL address
#       g6 = assoc. ILMT address
#       g9 = sec. ILT 1st nest lvl
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
te16z_iocomp:
        movq    g0,r8                   # save g0-g3
        movq    g4,r12                  # save g4-g7
#
# --- deallocate secondary ILT
#
        mov     g1,r4                   # save ILT'
        mov     g9,g1                   # g1 = ILT" at 1st lvl
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
        mov     r4,g1                   # restore ILT'

        call    te16com_release_all     # release dg resources

        call    mag$chknextask          # check if next task needs to be enabled

        movq    r8,g0                   # restore g0-g3
        movq    r12,g4                  # restore g4-g7
        ret                             # return to call

#******************************************************************************
#
#  NAME: te16z_abort
#
#  PURPOSE:
#       Provide the processing to abort the write or read buffer command while
#       waiting for final transfer
#
#  DESCRIPTION:
#       Release all dg resources and check the next task
#
#  CALLING SEQUENCE:
#       call    te16z_abort
#       Note: called from mag2_iocr
#
#  INPUT:
#       g0 = exchange ID of aborted task
#       g1 = aborted task ILT at lvl 2 (inl2)
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. g0-g11 may be destroyed.
#
#******************************************************************************
#
te16z_abort:
        mov     0,r13
        ldob    inl2_lldID(g1),r4       # r4 = exchange ID
        ld      ilm_imt(g6),r5          # r5 = get imt address
        ld      im_ltmt(r5),r5          # r5 = ltmt address
        ld      ltmt_tmsg(r5)[r4*4],r7  # r7 = LDG temp storage area

        lda     task_etbl16y,r3         # r3 = new task event handler table
        st      r3,inl2_ehand(g1)       # save new task event handler table
                                        #  in task ILT
        call    mag$qtask2aq            # queue task ILT to abort queue

        ld      sdg_writeILT(r7),r4     # r4 = write ILT
        cmpobne.f r4,g1,.te16z_abrt_100 # jIF not write

        st      r13,sdg_writeILT(r7)    # clear wb entry
        b       .te16z_abrt_200         # b

.te16z_abrt_100:
        ld      sdg_readILT(r7),r4      # r4 = read ILT
        cmpobne.f  r4,g1,.te16z_abrt_1000 # Jif not read buffer

        st      r13,sdg_readILT(r7)     # clear rb entry

.te16z_abrt_200:
        call    te16com_release_all     # release dg resources
        call    mag$chknextask          # check if next task needs to be enabled

.te16z_abrt_1000:
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: te16com_release_all
#
#  PURPOSE:
#       Provide the processing to release all elements of a datagram.
#
#  DESCRIPTION:
#       Deallocate the Read Buffer ILT, Write Buffer ILT, LDG temp storage ILT,
#       and the RDRAM SGL.
#
#  CALLING SEQUENCE:
#       call    te16com_release_all
#
#  INPUT:
#       g1 = pri. ILT at inl2 nest level
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       None.
#
#******************************************************************************
#
te16com_release_all:
        movq    g0,r8                   # save g0-g3
        movq    g4,r12                  # save g4-g7
        mov     0,r6
#
# --- find the LDG temp storage area for this exchange
#
        ldob    inl2_lldID(g1),r4       # r4 = exchange ID
        ld      ilm_imt(g6),r5          # r5 = get imt address
? # crash - cqt# 22710 - 2008-06-02 -- FE ILMT - g4 loads zero.
        ld      im_ltmt(r5),g4          # g4 = ltmt address
        ld      ltmt_tmsg(g4)[r4*4],r7  # r7 = LDG temp storage area
        st      r6,ltmt_tmsg(g4)[r4*4]  # clear LDG temp ptr in tmsg table
#
# --- return read buffer ILT to owner
#
        ld      sdg_readILT(r7),g1      # g1 = read buffer ILT
        cmpobe.f 0,g1,.te16com_ra100    # JIf no rb ILT

        call    mag$remtask             # remove task ILT from working queue
        lda     -ILTBIAS(g1),g1         # set up to return ILT to originator
        ld      il_cr(g1),r3            # r3 = originator's completion
                                        #  handler routine
        st      r6,sdg_readILT(r7)      # clear read ILT pointer
        callx   (r3)                    # call completion routine
#
# --- return write buffer ILT to owner
#
.te16com_ra100:
        ld      sdg_writeILT(r7),g1     # g1 = write buffer ILT
        cmpobe.f 0,g1,.te16com_ra200    # JIf no wb ILT

        call    mag$remtask             # remove task ILT from working queue
        lda     -ILTBIAS(g1),g1         # set up to return ILT to originator
        ld      il_cr(g1),r3            # r3 = originator's completion
                                        #  handler routine
        st      r6,sdg_writeILT(r7)     # clear write ILT pointer
        callx   (r3)                    # call completion routine
#
# --- return RDRAM SGL
#
.te16com_ra200:
        ld      sdg_sgl(r7),g0          # g0 = SGL
        cmpobe.f 0,g0,.te16com_ra300    # Jif no SGL

        st      r6,sdg_sgl(r7)          # clear sgl pointer
        call    M$rsglbuf               # return the SGL
#
# --- deallocate LDG temp storage ILT
#
.te16com_ra300:
        mov     r7,g1                   # g1 = ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
        movq    r8,g0                   # restore g0-g3
        movq    r12,g4                  # restore g4-g7
        ret                             # return to call

#******************************************************************************
#
#  NAME: te16com_rtn2owner
#
#  PURPOSE:
#       Provides means to return a ILT to it's owner.
#
#  CALLING SEQUENCE:
#       call    te16com_rtn2owner
#
#  INPUT:
#       g1 = primary ILT of task at the inl2 nest level
#            Note: Task ILT has been removed from the work queue
#                  prior to calling.
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       None.
#
#******************************************************************************
#
te16com_rtn2owner:
        call    mag$remtask             # remove task ILT from working queue
        PushRegs(r3)                    # Save all G registers
        lda     -ILTBIAS(g1),g1         # back up to previous nest level
        ld      inl1_cr(g1),g0          # g0 = completion routine for
        callx   (g0)                    # go to originator's completion
                                        #  routine
        PopRegsVoid(r3)                 # Restore all G registers
        ret
#
#******************************************************************************
# _________________________  DATAGRAM SERVICES ________________________________
#
#******************************************************************************
#******************************************************************************
#
#  NAME: lld$dg_server
#
#  PURPOSE:
#       Processes request from a datagram client.
#
#  CALLING SEQUENCE:
#       call    lld$dg_server
#
#  INPUT:
#       g0 = pointer to request area
#       g1 = pointer to response area
#       g4 = assoc. LTMT address
#       g6 = assoc. ILMT address
#
#  OUTPUT:
#       g0 = completion status   (ec1)
#       g1 = completion status 2 (ec2)
#
#  REGS DESTROYED:
#       Regs. g1-g12,g14 may be destroyed.
#
#******************************************************************************
#
lld$dg_server:
        PushRegs(r3)                    # Save all G registers

        movl    g0,g10                  # g10/g11 = request/response headers
#
# --- set up the header
#
        st      0,dgrs_status(g11)      # clear response header
        st      0,dgrs_ec1(g11)
        ldconst dgrs_size,g0            # g0 = response header size
        stob    g0,dgrs_hdrlen(g11)     # save response header size
        st      0,dgrs_resplen(g11)     # save a zero length
#
# --- get function code and determine if it is valid
#
        ldob    dgrq_fc(g10),g0         # g3 = function code
        cmpobg.t MAXLLDSC,g0,.dgsvr_100 # Jif request is in range
#
# --- function code was invalid
#
        ldconst dgec1_dlld_invfc,g0     # g0 = ec1 = invalid function code
        mov     0,g1                    # g1 = ec2 = 0
        mov     0,g2                    # g2 = 0 response length
        b       .dgsvr_200
#
# --- function code was valid. Set up default status and call the function
#     handler routine.
#
.dgsvr_100:
        ld      .dgstbl[g0*4],g0        # g0 = address of routine
        callx   (g0)                    # process function
        cmpobe.t 0,g0,.dgsvr_1000       # Jif no errors
#
# --- error status received from function handler routine
#
.dgsvr_200:
        stob    g0,dgrs_ec1(g11)        # save ec1 value
        stob    g1,dgrs_ec2(g11)        # save ec2 value
                                        # r5 = completion status 2 (ec2)
        ldconst dg_st_dlld,g0           # g0 = request completion status
        stob    g0,dgrs_status(g11)     # save status
#
# --- set up response data length, restore registers, and return to
#     caller
#
.dgsvr_1000:
        bswap   g2,g2                   # change data length endiance
        st      g2,dgrs_resplen(g11)    # set response length
#
c       r4 = g1;                        # Save g1 in r4
        PopRegs(r3)                     # Restore g1-g14
c       g1 = r4;                        # Restore g1 from r4
        ret                             # return to caller
#
# --- services table
#
        .data
.dgstbl:
        .word   lld$dgs_estvl           # establish Vlink
        .word   lld$dgs_trmvl           # terminate Vlink
        .word   lld$dgs_pncc            # peer node configuration change
#
        .text
#
#******************************************************************************
#
#  NAME: lld$dgs_estvl
#
#  PURPOSE:
#       Establish a virtual link.
#
#  CALLING SEQUENCE:
#       call    lld$dgs_estvl
#
#  INPUT:
#       g4 = assoc. LTMT address
#       g10 = pointer to request area
#       g11 = pointer to response area
#
#  OUTPUT:
#       g0 = ec1 status byte
#       g1 = ec2 status byte
#       g2 = response header length
#
#  REGS DESTROYED:
#       Regs. g1-g9,12,g14 may be destroyed.
#
#******************************************************************************
#
lld$dgs_estvl:

        lda     dgrq_size(g10),r15      # r15 = request data
        mov     g10,r14                 # r14 = request header

        ld      ltmt_imt(g4),r3         # r3 = IMT address
        ldos    im_sid(r3),g0           # g0 = Server ID
        ldconst 0xFFFF,g2               # g2 = No Assigned SID
        cmpobe  g2,g0,.dgsestvl_20      # No Server - no VDisks
        ld      S_sddindx[g0*4],g0      # g0 = SDD
        ldob    LLD0_rq_estvl_dstvd(r15),g1 # g1 = vdisk number (really LUN)
        call    D$lunlookup             # g3 = VID or 0xFFFFFFFF
        ldconst 0xFFFFFFFF,g2           # VID not found constant
        cmpobe.f g2,g3,.dgsestvl_20     # Jif no VID found for the LUN
        mov     g3,r4                   # r4 = VID
        shlo    2,g3,r6                 # r6 = vdisk * 4
        mov     g1,g3                   # g3 = LUN
        ld      MAG_VDMT_dir(r6),g0     # g0 = VDMT address

# -- Upper bit means offline. This is flag1<b>, flag2<b>, and attributes<s>.
        ldconst 0x80000000,g2           # g2 = ilmt attributes
        mov     r3,g1                   # g1 = IMT

        cmpobne.f 0,g0,.dgsestvl_050    # Jif there is VDMT
        cmpobe.f  0,g3,.dgsestvl_050    # jif no vlink and lun 0

#
# No VDisks found
#
.dgsestvl_20:
        ldconst dgec1_dlld_novdisk,g0   # set status ec1
        b       .dgsestvl_1000          # exit

.dgsestvl_050:
        ldos    im_cfgsiz(g1),r7        # r7 = size of image config. records
        lda     im_cfgrec(g1),r6        # r6 = pointer to image config. records
        cmpobe.f 0,r7,.dgsestvl_200     # Jif length is zero

.dgsestvl_100:
        ldos    im_cfglun(r6),r10       # r10 = LUN # from config. record
        ldos    im_cfgvid(r6),r11       # r11 = VID # from config. record
        cmpobne.f g3,r10,.dgsestvl_110  # Jif not a match
        cmpobe.t r4,r11,.dgsestvl_300   # Jif a match

.dgsestvl_110:
        addo    im_cfgrecsiz,r6,r6      # inc. to next config. record
        subo    im_cfgrecsiz,r7,r7      # dec. remaining config. record size
        cmpoble.t im_cfgrecsiz,r7,.dgsestvl_100 # Jif more image config. records
#
# --- end of configuration records and the required record was not found. So
#     let's create one...
#
.dgsestvl_200:
        mov     0,r13
        ldos    im_cfgsiz(g1),r7        # r7 = size of image config. records
        stos    g3,im_cfglun(r6)        # save lun #
        stos    r4,im_cfgvid(r6)        # save virtual disk #
        stos    r13,im_cfgattr(r6)      # clear attributes
        lda     im_cfgrecsiz(r7),r7     # add record size
        stos    r7,im_cfgsiz(g1)        # save new length
#
# --- determine if an ILMT is defined
#
.dgsestvl_300:
        ld      im_ilmtdir(g1)[g3*4],r8 # r8 = possible ILMT from directory
        cmpobe.t 0,r8,.dgsestvl_350     # Jif no ILMT defined

        ld      ilm_vdmt(r8),r9         # r9 = possible VDMT
        cmpobe.t 0,r9,.dgsestvl_350     # Jif no VDMT defined
#
# --- Check to see if the VDMT has matching VID
#
        ldos    vdm_vid(r9),r7          # r7 = VID from VDMT
        cmpobe.t r4,r7,.dgsestvl_400    # Jif a match

#
# --- Bingo!!! The St. Vincent data corruption issue : TBolt00024311/557436 - Raghu
#     Knockout the bad nexus and let the code takes its natural path to complete
#     the vlink establishment process.
#
c fprintf(stderr, "%s%s:%u St.Vincents Data corruption issue fix in VL create activated!!!\n", FEBEMESSAGE, __FILE__, __LINE__);
        mov     g2,r7                   # save g2
        mov     r8,g2                   # g2 = ILMT
        call    mag$sepilmtvdmt         # separate ilmt from vdmt
        st      r13,ilm_vdmt(g2)        # clear vdmt pointer in ILMT
        mov     r7,g2                   # restore g2
#
# --- ILMT is not defined or ILMT is defined and does not have an
#     associated VDMT.
#
#       regs at this point:
#
#          g0 = VDMT
#          g1 = IMT
#          g2 = attributes
#          g3 = LUN number
#
.dgsestvl_350:
        call    lld$add_ilmt            # add ILMT to this image

        cmpobne.f 0,g0,.dgsestvl_400    # Jif there is not VDMT

        ldconst dgec1_dlld_novdisk,g0   # set status ec1
        b       .dgsestvl_1000          # exit
#
# --- Vlink already established
#
.dgsestvl_400:
        mov     0,g0                    # clear status ec1
.dgsestvl_1000:
        mov     0,g1                    # clear status ec2
        mov     0,g2                    # set response data length
        ret                             # return to caller

#******************************************************************************
#
#  NAME: lld$dgs_trmvl
#
#  PURPOSE:
#       Terminate a virtual link.
#
#  CALLING SEQUENCE:
#       call    lld$dgs_trmvl
#
#  INPUT:
#       g4 = assoc. LTMT address
#       g10 = pointer to request area
#       g11 = pointer to response area
#
#  OUTPUT:
#       g0 = ec1 status byte
#       g1 = ec2 status byte
#       g2 = response header length
#
#  REGS DESTROYED:
#       Regs. g1-g9,g12,g14 may be destroyed.
#
#******************************************************************************
#
lld$dgs_trmvl:

        mov     g10,r4                  # r4 = request header
        lda     dgrq_size(g10),r5       # r5 = request data

        ld      ltmt_imt(g4),r3         # r3 = IMT address
        ldos    im_sid(r3),g0           # g0 = Server ID

        ldconst 0xFFFFFFFF,r12          # r12 = initialized to invalid VID
        ldob    LLD0_rq_trmvl_dstvd(r5),g1 # g1 = vdisk number (really LUN)
        ldconst 0xFFFF,g2               # g2 = No Assigned SID
        cmpobe  g2,g0,.dgstrmvl_010     # No Server - no VDisks

        ld      S_sddindx[g0*4],g0      # g0 = SDD
        call    D$lunlookup             # g3 = VID or 0xFFFFFFFF if not found
        mov     g3,r12                  # r12 = VID
#
.dgstrmvl_010:
        mov     g1,g3                   # g3 = LUN
        mov     r3,g1                   # g1 = IMT
        ld      im_ilmtdir(g1)[g3*4],g2 # g2 = possible ILMT from directory
        cmpobe.t 0,g2,.dgstrmvl_1000    # Jif no ILMT defined
#
# --- There is am ILMT. Remove it from possible VDMT queues
#
        call    lld$remove_ilmt
        ldconst 0xFFFFFFFF,r3           # r3 = VID not found constant
        cmpobne.f r3,r12,.dgstrmvl_050  # Jif VID found
#
# --- Bingo!!! The St. Vincent data corruption issue : TBolt00024311/557436 - Raghu
#
c fprintf(stderr, "%s%s:%u St.Vincents Data corruption issue fix in VL terminate activated!!!\n", FEBEMESSAGE, __FILE__, __LINE__);
        b       .dgstrmvl_1000
#
# --- Determine if there is a configuration record for this vdisk/lun
#
.dgstrmvl_050:
        ldos    im_cfgsiz(g1),r7        # r7 = size of image config. records
        lda     im_cfgrec(g1),r6        # r6 = pointer to image config. records

.dgstrmvl_100:
        cmpobe.f 0,r7,.dgstrmvl_1000    # Jif the record was not found

        ldos    im_cfglun(r6),r10       # r10 = LUN # from config. record
        ldos    im_cfgvid(r6),r11       # r11 = VID # from config. record
        cmpobne.f g3,r10,.dgstrmvl_110  # Jif not a match
        cmpobe.t r12,r11,.dgstrmvl_200  # Jif a match

.dgstrmvl_110:
        addo    im_cfgrecsiz,r6,r6      # inc. to next config. record
        subo    im_cfgrecsiz,r7,r7      # dec. remaining config. record size
        b       .dgstrmvl_100           # continue
#
# --- The correct record was found. Remove it and compress the database.
#
.dgstrmvl_200:
        subo    im_cfgrecsiz,r7,r7
        cmpobe.f 0,r7,.dgstrmvl_250     # jif no more records remain

        addo    im_cfgrecsiz,r6,r4      # address of next record

        ldos    im_cfglun(r4),r8        # r8 = lun #
        ldos    im_cfgvid(r4),r9        # r9 = virtual disk #
        ldos    im_cfgattr(r4),r10      # r10 = attributes

        stos    r8,im_cfglun(r6)        # save lun #
        stos    r9,im_cfgvid(r6)        # save virtual disk #
        stos    r10,im_cfgattr(r6)      # clear attributes
        mov     r4,r6                   # current = next
        b       .dgstrmvl_200           # continue
#
# --- set new database length
#
.dgstrmvl_250:
        ldos    im_cfgsiz(g1),r7        # r7 = size of image config. records
        subo    im_cfgrecsiz,r7,r7      # dec. config. record size
        stos    r7,im_cfgsiz(g1)        # save new length

.dgstrmvl_1000:
        mov     0,g0                    # clear status ec1
        mov     0,g1                    # clear status ec2
        mov     0,g2                    # set response data length

        ret                             # return to caller

#******************************************************************************
#
#  NAME: lld$dgs_pncc
#
#  PURPOSE:
#       Registers changes of node configuration from the peer magnitude.
#
#  CALLING SEQUENCE:
#       call    lld$dgs_pncc
#
#  INPUT:
#       g4 = assoc. LTMT address
#       g10 = pointer to request area
#       g11 = pointer to response area
#
#  OUTPUT:
#       g0 = ec1 status byte
#       g1 = ec2 status byte
#       g2 = response header length
#
#  REGS DESTROYED:
#       Regs. g1-g9,g12,g14 may be destroyed.
#
#******************************************************************************
#
lld$dgs_pncc:
        mov     g10,r4                  # r4 = request header
        lda     dgrq_size(g10),r5       # r5 = request data

        ldl     LLD0_rq_pncc_nname(r5),r8 #  r8-r9 = peer node name
        stl     r8,ltmt_pname(g4)       # save new node name
        mov     0,g0                    # clear status ec1
        mov     0,g1                    # clear status ec2
        mov     0,g2                    # set response data length

        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: lld$send_dg
#
#  PURPOSE:
#       Builds up the CDB's required to send a DG message.
#
#  DESCRIPTION:
#       The ILT must have the following definition at the sndg2 nest level:
#            il_cr = completion routine
#            sndg2_reqptr        # request data pointer       [w0] <w>
#            sndg2_reqlen        # request data length        [w1] <w>
#            sndg2_respptr       # response data pointer      [w2] <w>
#            sndg2_resplen       # response data length       [w3] <w>
#            sndg2_esgl          # extended sgl pointer       [w5] <w>
#
#       Upon entry into the completion routine, the registers have the
#       following definitions:
#            g1 = ILT at sndg2 nest level
#            g4 = LTMT
#            g5 = LSMT
#
#  CALLING SEQUENCE:
#       call    lld$send_dg
#
#  INPUT:
#       g1 = ILT' address sndg3 level
#       g4 = LTMT address
#       g5 = LSMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Regs g0 and g1 are destroyed.
#
#******************************************************************************
#
lld$snddg:
        ldob    ltmt_xchgID(g4),r10     # r10 = exchange ID
        ldconst ltmt_dgmax+1,r3         # r3 = set loop count to Max dgs + 1
        addo    16,sp,sp                # Make room on Stack for ILT pointers
#
# --- determine an exchange ID and determine if it is in use. If it is,
#     find another. Otherwise use it.
#
.snddg_100:
        addo    1,r10,r10               # bump exchange ID
        and     ltmt_dgmax-1,r10,r10    # size it
        stob    r10,ltmt_xchgID(g4)     # save it back

        ld      ltmt_imsg(g4)[r10*4],r5 # get value at this entry
        cmpobe  0,r5,.snddg_200         # Jif entry is not in use

        cmpdeco 1,r3,r3                 # decrement loop counter
        bne     .snddg_100              # Jif not all entries have been checked
#
# --- no open entries, place request on queue
#
        call    lld$qdg                 # place this message on the queue
        b       .snddg_1000             # exit
#
# --- there is an open entry in the table, send the datagram message
#
.snddg_200:
        st      g1,ltmt_imsg(g4)[r10*4] # save ILT' in exchange table

        lda     -ILTBIAS(g1),r14        # r14 = ILT' at sndg2 lvl (2nd lvl)
        mov     0,r13
        mov     g1,r15                  # r15 = ILT' at sndg3 lvl (3rd lvl)
        stob    r10,sndg3_xchgID(r15)   # save exchange ID
        st      r13,sndg3_readILT(r15)  # clear read ILT" address
        st      r13,sndg3_status(r15)   # clear error fields
#
# --- allocate ILT/IRP for the write buffer command and set up the
#     CDB.
#
#     Registers at this point are...
#
#       r10 = exchange ID
#       r14 = ILT' at sndg2 lvl (2nd lvl)
#       r15 = ILT' at sndg3 lvl (3rd lvl)
#       g4 = LTMT address
#       g5 = LSMT address
#
        call    M$ailtirp               # allocate an ILT/IRP combo (meal?)
                                        # g0 = IRP address
                                        # g1 = ILT address
                                        # Could cause a wait and LTMT freed
        ld      ltmt_imsg(g4)[r10*4],r4 # See if the ILT has been freed
        cmpobe  r4,r15,.snddg_205       # Jif the ILT is still valid
#
#       The original ILT has been completed, release the new IRP and ILT
#
.ifdef M4_DEBUG_IRP
c CT_history_printf("%s%s:%u put_irp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif # M4_DEBUG_IRP
c       put_irp(g0);                    # release IRP
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
        b       .snddg_1000             # All done
#
.snddg_205:
        st      g1,sndg3_writeILT(r15)  # save req ILT" adr in exchange table
        st      g1,-16(sp)              # save the Write Buffer ILT pointer
        lda     snddg_wb_cr,r4          # r4 = ILT" completion handler routine

        st      g0,lirp1_irp(g1)        # save IRP address in ILT"
        st      r4,lirp1_cr(g1)         # save completion routine
        st      g5,lirp1_lldid(g1)      # save LSMT address in ILT"
        st      r15,lirp1_g1(g1)        # save ILT' pointer in ILT"

        ldconst DGBSYCNT,r4             # r4 = data gram busy count
        stob    r4,lirp1_bsycnt(g1)     # save busy count

        lda     ILTBIAS(g1),g1          # g1 = ILT/IRP at nest level 2
        st      g0,lirp2_irp(g1)        # save IRP address in nest level 2
#
# --- setup the CDB attributes
#
        ldconst ttc_simple,r4           # r4 = task type code
        ldconst 0x02,r5                 # r5 = data transfer attribute
        ldconst rfc_SCSI_cmd,r6         # r6 = IRP request function code
        ldob    sndg2_rtycnt(r14),r7    # r7 = requested retry count
        cmpobne 0,r7,.snddg_210         # Jif a valid retry count is given
        ldconst SNDDG_RTYCNT,r7         # r7 = command retry count (Default)
.snddg_210:
        ldob    sndg2_cmdto(r14),r8     # r8 = requested command timeout (secs.)
        cmpobne 0,r8,.snddg_220         # Jif a valid retry count is given
        ldconst SNDDG_CMDTO,r8          # r8 = command timeout (secs.) (Default)
.snddg_220:
        ld      lsmt_psid(g5),r9        # r9 = provider ID

        stob    r4,irp_Tattr_TTC(g0)    # save task type code
        stob    r5,irp_Tattr_DTA(g0)    # save data transfer attribute
        stob    r6,irp_RFC(g0)          # save request function code
        stob    r7,irp_rtycnt(g0)       # save CDB retry count
        stos    r8,irp_cmdTO(g0)        # save command timeout
        st      r9,irp_Pro_ID(g0)       # save provider ID in IRP

        shro    4,g1,r4                 # r4 = tag ID
        stos    r4,irp_tagID(g0)        # save tag ID

        ldob    irp_Tattr_flags(g0),r4  # set SLIE flag
        setbit  tpf_SLIE,r4,r4
        stob    r4,irp_Tattr_flags(g0)

        st      g5,irp_Req_ID(g0)       # save LSMT as requestor ID in IRP
#
# --- build up the CDB and the SGL table in the IRP.
#
        ldq     wb_cdb,r4               # r4-r7 = CDB
        lda     irp_SGLhdr(g0),r9       # r9 = ptr to SGL in IRP
        stq     r4,irp_CDB(g0)          # save CDB in IRP

        ld      sndg2_reqptr(r14),r13   # r13 = address of request buffer
        ld      sndg2_reqlen(r14),r5    # r5 = request length
        ld      sndg2_resplen(r14),r11  # r11 = response len (0 = no response)
        st      r9,irp_SGLptr(g0)       # save ptr to SGL table
        st      r13,sg_desc0+sg_addr(r9) # save address of buffer
        addo    4-1,r5,r5               # round up to a word boundary
        andnot  4-1,r5,r5               # r6 = request length (adjusted)
        st      r5,sg_desc0+sg_len(r9)  # save buffer length
        ldconst 1,r3                    # r3 = descriptor count (1)
#
        ld      sndg2_esgl(r14),r4      # r4 = extended sgl pointer
        cmpobe  0,r4,.snddg_360         # Jif if there is no extended sgl
        ldos    sg_scnt(r4),r12         # r7 = extended sgl entry count
        cmpobe  0,r12,.snddg_360        # Jif there are no more entries
        lda     sg_desc0(r4),r4         # r4 = descriptor address to read
        lda     sg_desc1(r9),r8         # r8 = descriptor address to write
        ldconst SG_LEN_MASK,g1          # g1 = Mask for Length field
.snddg_300:
        ldob    sg_dir(r4),r6           # r6 = direction bit
        cmpobne SG_DIR_OUT,r6,.snddg_320 # Jif not a request data entry
        ldl     sg_addr(r4),r6          # r6-r7 = request data descriptor
        and     g1,r7,r7                # Remove the direction bit from length
        addo    1,r3,r3                 # Increment the total descriptor count
        stl     r6,sg_addr(r8)          # Store the request data descriptor
        lda     sgdescsiz(r8),r8        # r8 = point to next write descriptor
.snddg_320:
        lda     sgdescsiz(r4),r4        # r4 = point to next read descriptor
        subo    1,r12,r12               # r12 = number of descriptors left
        cmpobne 0,r12,.snddg_300        # Jif there are no more entries
#
.snddg_360:
        st      r3,sg_scnt(r9)          # save descriptor count and clear rest

        bswap   r11,r11                 # change endiance
        st      r11,irp_CDB+2(g0)       # response len in IRP CDB
        stob    r10,irp_CDB+2(g0)       # save exchange ID in IRP CDB

        bswap   r5,r5                   # change endiance
        shro    8,r5,r5
        st      r5,irp_CDB+6(g0)        # request len in IRP CDB
#
# --- generate a CRC for the request header
#
!       ldq     (r13),r4                # r4-r7 = header words 0-3
        xor     r4,r5,r3                # calculate LRC
        xor     r6,r3,r3
        xor     r7,r3,r3
!       ldt     16(r13),r4              # r4-r6 = header words 4-6
        xor     r4,r3,r3                # calculate LRC
        xor     r5,r3,r3
        xor     r6,r3,r3
!       st      r3,dgrq_crc(r13)        # save CRC value
#
# --- allocate ILT/IRP for the read buffer command and set up the CDB.
#
        call    M$ailtirp               # allocate an ILT/IRP combo (meal?)
                                        # g0 = IRP address
                                        # g1 = ILT address
                                        # Could cause a wait and LTMT freed
        ld      ltmt_imsg(g4)[r10*4],r4 # See if the ILT has been freed
        cmpobe  r4,r15,.snddg_370       # Jif the ILT is still valid
#
#       The original ILT has been completed, release the new IRP and ILT (Read
#       Buffer), and then the Write Buffer IRP/ILT.
#
.ifdef M4_DEBUG_IRP
c CT_history_printf("%s%s:%u put_irp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif # M4_DEBUG_IRP
c       put_irp(g0);                    # release IRP
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT (Read Buffer)
        ld      -16(sp),g1              # Get the Write Buffer ILT
        ld      lirp1_irp(g1),g0        # Get the Write Buffer IRP
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT (Write Buffer)
.ifdef M4_DEBUG_IRP
c CT_history_printf("%s%s:%u put_irp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif # M4_DEBUG_IRP
c       put_irp(g0);                    # release IRP
        b       .snddg_1000             # All done
#
.snddg_370:
        st      g1,sndg3_readILT(r15)   # save resp ILT" addr in exchange table
        st      g1,-12(sp)              # save the Read Buffer ILT pointer
        lda     snddg_rb_cr,r4          # r4 = ILT completion handler routine

        st      g0,lirp1_irp(g1)        # save IRP address in ILT"
        st      r4,lirp1_cr(g1)         # save completion routine
        st      g5,lirp1_lldid(g1)      # save LSMT address in ILT"
        st      r15,lirp1_g1(g1)        # save ILT' pointer in ILT"

        ldconst DGBSYCNT,r4             # r4 = datagram busy count
        stob    r4,lirp1_bsycnt(g1)     # save busy count

        lda     ILTBIAS(g1),g1          # g1 = ILT/IRP at nest level 2
        st      g0,lirp2_irp(g1)        # save IRP address in nest level 2
#
# --- setup the CDB attributes
#
        ldconst ttc_simple,r4           # r4 = task type code
        ldconst 0x01,r5                 # r5 = data transfer attribute
        ldconst rfc_SCSI_cmd,r6         # r6 = IRP request function code
        ldob    sndg2_rtycnt(r14),r7    # r7 = requested retry count
        cmpobne 0,r7,.snddg_380         # Jif a valid retry count is given
        ldconst SNDDG_RTYCNT,r7         # r7 = command retry count (Default)
.snddg_380:
        ldob    sndg2_cmdto(r14),r8     # r8 = requested command timeout (secs.)
        cmpobne 0,r8,.snddg_390         # Jif a valid retry count is given
        ldconst SNDDG_CMDTO,r8          # r8 = command timeout (secs.) (Default)
.snddg_390:
        ld      lsmt_psid(g5),r9        # r9 = provider ID

        stob    r4,irp_Tattr_TTC(g0)    # save task type code
        stob    r5,irp_Tattr_DTA(g0)    # save data transfer attribute
        stob    r6,irp_RFC(g0)          # save request function code
        stob    r7,irp_rtycnt(g0)       # save CDB retry count
        stos    r8,irp_cmdTO(g0)        # save command timeout
        st      r9,irp_Pro_ID(g0)       # save provider ID in IRP

        shro    4,g1,r4                 # r4 = tag ID
        stos    r4,irp_tagID(g0)        # save tag ID

        ldob    irp_Tattr_flags(g0),r4  # set SLIE flag
        setbit  tpf_SLIE,r4,r4
        stob    r4,irp_Tattr_flags(g0)

        st      g5,irp_Req_ID(g0)       # save LSMT as requestor ID in IRP
#
# --- build up the CDB and the SGL table in the IRP
#
        ldq     rb_cdb,r4               # r4-r7 = CDB
        lda     irp_SGLhdr(g0),r9       # r9 = ptr to SGL in IRP
        stq     r4,irp_CDB(g0)          # save CDB in IRP

        ld      sndg2_respptr(r14),r5   # r5 = address of response buffer
        ld      sndg2_resplen(r14),r6   # r6 = response length

        st      r9,irp_SGLptr(g0)       # save ptr to SGL table
        st      r5,sg_desc0+sg_addr(r9) # save address of buffer
        st      r6,sg_desc0+sg_len(r9)  # save buffer length
        ldconst 1,r3                    # r3 = descriptor count (1)
#
        ld      sndg2_esgl(r14),r4      # r4 = extended sgl pointer
        cmpobe  0,r4,.snddg_460         # Jif if there is no extended sgl
        ldos    sg_scnt(r4),r12         # r7 = extended sgl entry count
        cmpobe  0,r12,.snddg_460        # Jif there are no more entries
        lda     sg_desc0(r4),r4         # r4 = descriptor address to read
        lda     sg_desc1(r9),r8         # r8 = descriptor address to write
        ldconst SG_LEN_MASK,g1          # g1 = Mask for Length field
.snddg_400:
        ldob    sg_dir(r4),r6           # r6 = direction bit
        cmpobne SG_DIR_IN,r6,.snddg_420 # Jif not a response data entry
        ldl     sg_addr(r4),r6          # r6-r7 = response data descriptor
        and     g1,r7,r7                # Remove the direction bit from length
        addo    1,r3,r3                 # Increment the total descriptor count
        stl     r6,sg_addr(r8)          # Store the response data descriptor
        lda     sgdescsiz(r8),r8        # r8 = point to next write descriptor
.snddg_420:
        lda     sgdescsiz(r4),r4        # r4 = point to next read descriptor
        subo    1,r12,r12               # r12 = number of descriptors left
        cmpobne 0,r12,.snddg_400        # Jif there are no more entries
#
.snddg_460:
        st      r3,sg_scnt(r9)          # save descriptor count and clear rest

        stob    r10,irp_CDB+2(g0)       # save exchange ID in IRP CDB

        shro    8,r11,r11               # maximum length of 3 bytes
        st      r11,irp_CDB+6(g0)       # response len in IRP CDB
#
# --- send the write buffer off to the peer magnitude
#
        ld      sndg3_writeILT(r15),g1  # load req ILT" adr from exchange table
        lda     ILTBIAS(g1),g1          # g1 = ILT/IRP at nest level 2
        call    APL$pfr                 # request CDB be issued (could cause
                                        #  a wait and LTMT freed - apl$timer
                                        #  will handle the Write Buffer ILT/IRP)
#
# --- send the read buffer off to the peer magnitude
#
?# crash ILT @ r15 has been freed (daaddaad). Was good above 3 lines.
?       ld      sndg3_readILT(r15),g0   # load resp ILT" adr from exchange table
        ld      -12(sp),g1              # Get the Read Buffer ILT saved earlier
# The following is because the ILT could have been freed, or used for something else.
# Assumption is that it most likely won't be for something like this.
        cmpobe  g0,g1,.snddg_470        # Jif all is still OK
        ld      lirp1_irp(g1),g0        # Get the Read Buffer IRP
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT (Write Buffer)
.ifdef M4_DEBUG_IRP
c CT_history_printf("%s%s:%u put_irp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif # M4_DEBUG_IRP
c       put_irp(g0);                    # release IRP
        b       .snddg_1000             # All done
#
.snddg_470:
        lda     ILTBIAS(g1),g1          # g1 = ILT/IRP at nest level 2
        call    APL$pfr                 # request CDB be issued

.snddg_1000:
        ret
#
        .data
rb_cdb:                                 # CDB pattern to use for information
                                        #  page INQUIRY command
        .byte   0x3c,00,00,00
        .byte   00,00,00,00
        .byte   00,00,00,00
        .byte   00,00,00,00

wb_cdb:                                 # CDB pattern to use for information
                                        #  page INQUIRY command
        .byte   0x3b,00,00,00
        .byte   00,00,00,00
        .byte   00,00,00,00
        .byte   00,00,00,00
#
        .text
#
#**********************************************************************
#
#  NAME: snddg_wb_cr
#
#  PURPOSE:
#       Processes a Write Buffer response received from the peer magnitude.
#
#  CALLING SEQUENCE:
#        snddg_wb_cr
#
#  INPUT:
#       g1 = ILT/IRP address of CDB request at nest level 1
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Reg. g0-g12, g14 can be destroyed.
#
#**********************************************************************
#
snddg_wb_cr:
        ld      lirp1_irp(g1),g0        # get address of irp
        ld      lirp1_lldid(g1),g5      # g5 = LSMT address
        ldob    irp_cmplt(g0),r8        # r8 = completion status
        ld      irp_extcmp(g0),r9       # r9 = extended completion code
#
# --- determine if there is still an LSMT
#
        cmpobe  0,g5,.snddg_wbcr_610    # Jif no LSMT
        ld      lsmt_ltmt(g5),g4        # g4 = LTMT address
#
# --- Determine if there is an entry in the exchange table for this ILT.
#     If there is not or the ILT in the exchange table does not match the
#     ILT received, release the ILT" and the IRP.
#
#     The most likely cause of this condition would be that the send message
#     operation has been cancelled.
#
        ldob    irp_CDB+2(g0),r11        # r11 = exchange ID from the CDB
        ld      ltmt_imsg(g4)[r11*4],r15 # r15 = CDG pointer from exchange table
        cmpobe.f 0,r15,.snddg_wbcr_600  # Jif no entry (entry was flushed)
#
# --- determine if this is an expected response
#
        ld      sndg3_writeILT(r15),r4  # r4 = write ILT address
        cmpobe.f r4,g1,.snddg_wbcr_200  # Jif addr in table matches ILT
#
# --- Invalid or wrong ILT received. Bump the error counter and exit
#
        ld      lld_inv_ILT,r4          # get error counter
        addo    1,r4,r4                 # bump it
        st      r4,lld_inv_ILT          # save updated error counter
        b       .snddg_wbcr_610         # release ILT and exit
#
# --- This is the correct RB ILT, determine it's status.
#
.snddg_wbcr_200:
        lda     -ILTBIAS(r15),r14       # r14 = ILT' at sndg2 lvl
        ld      sndg2_respptr(r14),r13  # r13 = address of response buffer

        cmpobe.t 0,r8,.snddg_wbcr_300   # Jif zero completion

#       c fprintf(stderr,"[%s:%u]: snddg_wbcr_200 r8=%lX r9=%lX\n", __FILE__, __LINE__, r8,r9);
#
# --- determine if this is a check status error
#
        cmpobne.f cmplt_LOUT,r8,.snddg_wbcr_210 # JIf not "Logout" error
        ldconst ltmt_lst_id,r4          # r4 = new link state code
        stob    r4,ltmt_lst(g4)         # save new link state code

        b       .snddg_wbcr_400

.snddg_wbcr_210:
        cmpobne.f cmplt_CSE,r8,.snddg_wbcr_220 # JIf not "non CS" error
#
# --- CS status error - determine if there is some form of parameter change
#
        ldob    irp_extcmp(g0),r9       # r9 = ASC
        ldconst 0x2a,r4                 # r4 = ASC type
        cmpobne.t r4,r9,.snddg_wbcr_400 # Jif not ASC <> 0x2A
#
# --- Parameter change - determine if error is a mode parameter change
#
        ldob    irp_extcmp+1(g0),r9     # r9 = ASCQ
        cmpobne.f 0x01,r9,.snddg_wbcr_400 # Jif ASCQ <> 1
#
# --- mode parameter change - reissue dg. However - if the WB command
#     has completed, end the DG.
#
        ldconst cmplt_IOE,r8            # r8 = IO error
        ldconst ioe_timeout,r9          # r9 = timeout

        ld      sndg3_readILT(r15),r4   # r4 = get the RB ILT address
        cmpobe.t 0,r4,.snddg_wbcr_400   # Jif RB completion received

        lda     ILTBIAS(g1),g1          # g1 = ILT at 2nd lvl
        call    APL$pfr                 # request CDB be issued
        b       .snddg_wbcr_1000        # exit

.snddg_wbcr_220:
        cmpobne.f cmplt_non_CSE,r8,.snddg_wbcr_400 # JIf not "non CS" error
#
# --- non check status error, check for busy
#
        cmpobne.f ncs_busy,r9,.snddg_wbcr_400 # JIf not busy status
#
# --- busy status received. Decrement busy count. If busy count is zero,
#     end DG request with error. Otherwise, reissue task after a delay.
#
        ldob    lirp1_bsycnt(g1),r4     # get busy count
        subo    1,r4,r4                 # subtract 1
        stob    r4,lirp1_bsycnt(g1)     # save busy count

        ldconst cmplt_IOE,r8            # r8 = IO error
        ldconst ioe_timeout,r9          # r9 = timeout
        cmpobe.f 0,r4,.snddg_wbcr_400   # Jif busy count is zero
#
# --- fork the task to allow control to return to caller
#
        lda     .snddg_wbcr_250,g0
        mov     g1,r3                   # save g1
        mov     g2,r4                   # save g2
        mov     g1,g2                   # set up for the fork
        ldconst LLDSWBCR,g1             # set priority
c       CT_fork_tmp = (ulong)".snddg_wbcr_250";
        call    K$tfork
        mov     r3,g1                   # restore g1
        mov     r4,g2                   # restore g2
        b       .snddg_wbcr_1000        # exit
#
# --- retransmit DG task
#
.snddg_wbcr_250:
        mov     g2,g1                   # restore g1 as the ILT pointer
        lda     250,g0                  # set up to wait 250 ms
        call    K$twait                 # delay task

        ld      lirp1_irp(g1),g0        # get address of irp
        ld      lirp1_lldid(g1),g5      # g5 = LSMT address
        cmpobe  0,g5,.snddg_wbcr_610    # Jif no LSMT

        ld      lsmt_ltmt(g5),g4        # g4 = LTMT address
        ldob    irp_CDB+2(g0),r11       # r11 = exchange ID from the CDB
        ld      ltmt_imsg(g4)[r11*4],r15 # r15 = CDG pointer from exchange table
        cmpobe.f 0,r15,.snddg_wbcr_610  # Jif no entry (entry was flushed)

        lda     -ILTBIAS(r15),r14       # r14 = ILT' at sndg2 lvl
        ld      sndg2_respptr(r14),r13  # r13 = address of response buffer

        ldconst cmplt_IOE,r8            # r8 = IO error
        ldconst ioe_timeout,r9          # r9 = timeout

        ld      sndg3_readILT(r15),r4   # r4 = get the RB ILT address
        cmpobe.t 0,r4,.snddg_wbcr_400   # Jif RB completion received

        lda     ILTBIAS(g1),g1          # g1 = ILT at 2nd lvl
        call    APL$pfr                 # request CDB be issued
        b       .snddg_wbcr_1000        # exit
#
# --- zero completion status
#
.snddg_wbcr_300:
#
# --- determine if the response header CRC is correct.
#
        mov     0,r3
        st      r3,sndg3_writeILT(r15)  # clear RB entry in ILT'
        ld      sndg3_readILT(r15),r4   # r4 = get the RB ILT address
        cmpobne.t 0,r4,.snddg_wbcr_610  # Jif RB completion not received
!       ldq     (r13),r4                # r4-r7 = header words 0-3
        xor     r4,r5,r4
        xor     r6,r4,r4
        xor     r7,r4,r4
        cmpobne.f 0,r4,.snddg_wbcr_350  # Jif CRC is not OK
!       ldob    dgrs_hdrlen(r13),r4     # r4 = returned Response Header Length
        cmpobe.t dgrs_size,r4,.snddg_wbcr_500 # Jif Hdr looks good (not zeros)
.snddg_wbcr_350:
        ldconst dgec1_slld_crc,r8       # r8 = response CRC error
#       c fprintf(stderr,"[%s:%u]: snddg_wbcr_350 r8=%lX r9=%lX\n", __FILE__, __LINE__, r8,r9);
#
# --- even if there was a error presented by the peer magnitude, a
#     CRC error renders it invalid. Place the CRC error codes
#     in the response header.
#
#     save error information in response header
#
.snddg_wbcr_400:
        ldconst dg_st_slld,r10          # r10 = source LLD error

        ld      sndg3_readILT(r15),r4   # r4 = get the RB ILT address
        cmpobe.t 0,r4,.snddg_wbcr_500   # Jif RB completion received

        stob    r8,sndg3_ec1(r15)       # save ec1
        stob    r9,sndg3_ec2(r15)       # save ec2
        stob    r10,sndg3_status(r15)   # save error status
#
# --- determine if the WB cr has been received. If it has, end this DG
#     request.
#
.snddg_wbcr_500:
        mov     0,r3
        st      r3,sndg3_writeILT(r15)  # clear RB entry in ILT'

        ld      sndg3_readILT(r15),r4   # r4 = get the RB ILT address
        cmpobne.t 0,r4,.snddg_wbcr_600  # Jif RB completion not received

        ldob    sndg3_status(r15),r10   # get RB status
        cmpobe.t 0,r10,.snddg_wbcr_550  # JIf RB status = good

        ldob    sndg3_ec1(r15),r8       # r8 = ec1
        ldob    sndg3_ec2(r15),r9       # r9 = ec2

!       stob    r8,dgrs_ec1(r13)        # save ec1
!       stob    r9,dgrs_ec2(r13)        # save ec2
!       stob    r10,dgrs_status(r13)    # save error status
!       st      r3,dgrs_resplen(r13)    # save a zero length
#
# --- call ILT completion routine
#
.snddg_wbcr_550:
        mov     0,r3
        st      r3,ltmt_imsg(g4)[r11*4] # clear ILT' pointer from exchange table

        movq    g0,r4                   # save g0-g3 in r4-r7
        movq    g4,r8                   # save g4-g7 in r8-r11

        lda     -ILTBIAS(r15),g1        # g1 = ILT' to sndg2 lvl (2nd lvl)
        ld      il_cr(g1),r3            # r4 = ILT completion handler routine
        callx   (r3)                    # call ILT completion handler routine

        movq    r4,g0                   # restore g0-g3 from r4-r7
        movq    r8,g4                   # restore g4-g7 from r8-r11
#
# --- determine if there is a queued datagram message
#
.snddg_wbcr_600:
        call    lld$chkdg               # check if there is a queued dg message
#
# --- release the ILT" and IRP
#
.snddg_wbcr_610:
.ifdef M4_DEBUG_IRP
c CT_history_printf("%s%s:%u put_irp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif # M4_DEBUG_IRP
c       put_irp(g0);                    # release IRP
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT

.snddg_wbcr_1000:
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: snddg_rb_cr
#
#  PURPOSE:
#       Processes a Read Buffer response received from the peer magnitude.
#
#  CALLING SEQUENCE:
#        snddg_rb_cr
#
#  INPUT:
#       g1 = ILT/IRP address of CDB request at nest level 1
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Reg. g0-g12, g14 can be destroyed.
#
#**********************************************************************
#
snddg_rb_cr:
        ld      lirp1_irp(g1),g0        # get address of irp
        ld      lirp1_lldid(g1),g5      # g5 = LSMT address
        ldob    irp_cmplt(g0),r8        # r8 = completion status
        ld      irp_extcmp(g0),r9       # r9 = extended completion code
        mov     0,r10                   # r10 = 0
#
# --- determine if there is still an LSMT
#
        cmpobe  0,g5,.snddg_rbcr_610    # Jif no LSMT
        ld      lsmt_ltmt(g5),g4        # g4 = LTMT address
#
# --- Determine if there is an entry in the exchange table for this ILT.
#     If there is not or the ILT in the exchange table does not match the
#     ILT received, release the ILT" and the IRP.
#
#     The most likely cause of this condition would be that the send message
#     operation has been cancelled.
#
        ldob    irp_CDB+2(g0),r11        # r11 = exchange ID from the CDB
        ld      ltmt_imsg(g4)[r11*4],r15 # r15 = CDG pointer from exchange table
        cmpobe.f 0,r15,.snddg_rbcr_600  # Jif no entry (entry was flushed)
#
# --- determine if this is an expected response
#
        ld      sndg3_readILT(r15),r4   # r4 = read ILT address
        cmpobe.f r4,g1,.snddg_rbcr_200  # Jif addr in table matches ILT
#
# --- Invalid or wrong ILT received. Bump the error counter and exit
#
        ld      lld_inv_ILT,r4          # get error counter
        addo    1,r4,r4                 # bump it
        st      r4,lld_inv_ILT          # save updated error counter
        b       .snddg_rbcr_610         # release ILT and exit
#
# --- This is the correct RB ILT, determine it's status.
#
.snddg_rbcr_200:
        lda     -ILTBIAS(r15),r14       # r14 = ILT' at sndg2 lvl
        ld      sndg2_respptr(r14),r13  # r13 = address of response buffer

        cmpobe.t 0,r8,.snddg_rbcr_300   # Jif zero completion
#       c fprintf(stderr,"[%s:%u]:  snddg_rbcr_200 r8=%lX r9=%lX\n", __FILE__, __LINE__, r8,r9);
#
# --- determine if this is a check status error
#
        cmpobne.f cmplt_LOUT,r8,.snddg_rbcr_210 # JIf not "Logout" error
        ldconst ltmt_lst_id,r4          # r4 = new link state code
        stob    r4,ltmt_lst(g4)         # save new link state code

        b       .snddg_rbcr_400

.snddg_rbcr_210:
        cmpobne.f cmplt_CSE,r8,.snddg_rbcr_220 # JIf not "non CS" error
#
# --- CS status error - determine if there is some form of parameter change
#
        ldob    irp_extcmp(g0),r9       # r9 = ASC
        ldconst 0x2a,r4                 # r4 = ASC type
        cmpobne.t r4,r9,.snddg_rbcr_400 # Jif not ASC <> 0x2A
#
# --- Parameter change - determine if error is a mode parameter change
#
        ldob    irp_extcmp+1(g0),r9     # r9 = ASCQ
        cmpobne.f 0x01,r9,.snddg_rbcr_400 # Jif ASCQ <> 1
#
# --- mode parameter change - reissue dg. However - if the WB command
#     has completed, end the DG.
#
        ldconst cmplt_IOE,r8            # r8 = IO error
        ldconst ioe_timeout,r9          # r9 = timeout

        ld      sndg3_writeILT(r15),r4  # r4 = get the write ILT address
        cmpobe.t 0,r4,.snddg_rbcr_400   # Jif write buffer completion received

        lda     ILTBIAS(g1),g1          # g1 = ILT at 2nd lvl
        call    APL$pfr                 # request CDB be issued
        b       .snddg_rbcr_1000        # exit

.snddg_rbcr_220:
        cmpobne.f cmplt_non_CSE,r8,.snddg_rbcr_400 # JIf not "non CS" error
#
# --- non check status error, check for busy
#
        cmpobne.f ncs_busy,r9,.snddg_rbcr_400 # JIf not busy status
#
# --- busy status received. Decrement busy count. If busy count is zero,
#     end DG request with error. Otherwise, reissue task after a delay.
#
        ldob    lirp1_bsycnt(g1),r4     # get busy count
        subo    1,r4,r4                 # subtract 1
        stob    r4,lirp1_bsycnt(g1)     # save busy count

        ldconst cmplt_IOE,r8            # r8 = IO error
        ldconst ioe_timeout,r9          # r9 = timeout
        cmpobe.f 0,r4,.snddg_rbcr_400   # Jif busy count is zero
#
# --- fork the task to allow control to return to caller
#
        lda     .snddg_rbcr_250,g0
        mov     g1,r3                   # save g1
        mov     g2,r4                   # save g2
        mov     g1,g2                   # set up for the fork
        ldconst LLDSRBCR,g1             # set priority
c       CT_fork_tmp = (ulong)".snddg_rbcr_250";
        call    K$tfork
        mov     r3,g1                   # restore g1
        mov     r4,g2                   # restore g2
        b       .snddg_rbcr_1000        # exit
#
# --- retransmit DG task
#
.snddg_rbcr_250:
        mov     g2,g1                   # restore ILT pointer
        lda     250,g0                  # set up to wait 250 ms
        call    K$twait                 # delay task

        ld      lirp1_irp(g1),g0        # get address of irp
        ld      lirp1_lldid(g1),g5      # g5 = LSMT address
        cmpobe  0,g5,.snddg_rbcr_610    # Jif no LSMT

        ld      lsmt_ltmt(g5),g4        # g4 = LTMT address
        ldob    irp_CDB+2(g0),r11        # r11 = exchange ID from the CDB
        ld      ltmt_imsg(g4)[r11*4],r15 # r15 = CDG pointer from exchange table
        cmpobe.f 0,r15,.snddg_rbcr_610  # Jif no entry (entry was flushed)

        lda     -ILTBIAS(r15),r14       # r14 = ILT' at sndg2 lvl
        ld      sndg2_respptr(r14),r13  # r13 = address of response buffer

        ldconst cmplt_IOE,r8            # r8 = IO error
        ldconst ioe_timeout,r9          # r9 = timeout

        ld      sndg3_writeILT(r15),r4  # r4 = get the write ILT address
        cmpobe.t 0,r4,.snddg_rbcr_400   # Jif write buffer completion received

        lda     ILTBIAS(g1),g1          # g1 = ILT at 2nd lvl
        call    APL$pfr                 # request CDB be issued
        b       .snddg_rbcr_1000        # exit
#
# --- zero completion status
#
.snddg_rbcr_300:
#
# --- determine if the response header CRC is correct.
#
!       ldq     (r13),r4                # r4-r7 = header words 0-3
        xor     r4,r5,r4
        xor     r6,r4,r4
        xor     r7,r4,r4
        cmpobne.f 0,r4,.snddg_rbcr_350  # Jif CRC is not OK
!       ldob    dgrs_hdrlen(r13),r4     # r4 = returned Response Header Length
        cmpobe.t dgrs_size,r4,.snddg_rbcr_500 # Jif Hdr looks good (not zeros)
.snddg_rbcr_350:
        ldconst dgec1_slld_crc,r8       # r8 = response CRC error
#       c fprintf(stderr,"[%s:%u]: snddg_rbcr_350 r8=%lX r9=%lX\n", __FILE__, __LINE__, r8,r9);
#
# --- even if there was a error presented by the peer magnitude, a
#     CRC error renders it invalid. Place the CRC error codes
#     in the response header.
#
#     save error information in response header
#
.snddg_rbcr_400:
        ldconst dg_st_slld,r10          # r10 = source LLD error

        ld      sndg3_writeILT(r15),r4  # r4 = get the write ILT address
        cmpobe.t 0,r4,.snddg_rbcr_500   # Jif WB completion received

        stob    r8,sndg3_ec1(r15)       # save ec1
        stob    r9,sndg3_ec2(r15)       # save ec2
        stob    r10,sndg3_status(r15)   # save error status
#
# --- determine if the WB cr has been received. If it has, end this DG
#     request.
#
.snddg_rbcr_500:
        mov     0,r3
        st      r3,sndg3_readILT(r15)  # clear read entry in ILT'

        ld      sndg3_writeILT(r15),r4  # r4 = get the write ILT address
        cmpobne.t 0,r4,.snddg_rbcr_600  # Jif write buffer completion not received

        cmpobe.f 0,r10,.snddg_rbcr_550  # Jif RB status = good

        ldob    sndg3_status(r15),r4    # get WB status
        cmpobe.t 0,r4,.snddg_rbcr_510   # JIf WB status = good

        ldob    sndg3_ec1(r15),r8       # r8 = ec1
        ldob    sndg3_ec2(r15),r9       # r9 = ec2
        ldob    sndg3_status(r15),r10   # r10 = status

.snddg_rbcr_510:
        mov     0,r3
!       stob    r8,dgrs_ec1(r13)        # save ec1
!       stob    r9,dgrs_ec2(r13)        # save ec2
!       stob    r10,dgrs_status(r13)    # save error status
!       st      r3,dgrs_resplen(r13)   # save a zero length
#
# --- call ILT completion routine
#
.snddg_rbcr_550:
        mov     0,r3
        st      r3,ltmt_imsg(g4)[r11*4] # clear ILT' pointer from exchange table

        movq    g0,r4                   # save g0-g3 in r4-r7
        movq    g4,r8                   # save g4-g7 in r8-r11

        lda     -ILTBIAS(r15),g1        # g1 = ILT' to sndg2 lvl (2nd lvl)
        ld      il_cr(g1),r3            # r4 = ILT completion handler routine
        callx   (r3)                    # call ILT completion handler routine

        movq    r4,g0                   # restore g0-g3 from r4-r7
        movq    r8,g4                   # restore g4-g7 from r8-r11
#
# --- determine if there is a queued datagram message
#
.snddg_rbcr_600:
        call    lld$chkdg               # check if there is a queued dg message
#
# --- release the ILT" and IRP
#
.snddg_rbcr_610:
.ifdef M4_DEBUG_IRP
c CT_history_printf("%s%s:%u put_irp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif # M4_DEBUG_IRP
c       put_irp(g0);                    # release IRP
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT

.snddg_rbcr_1000:
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: lld$qdg
#
#  PURPOSE:
#       Queues an ILT associated with an datagram message to the
#       datagram holding queue in the associated TLMT.
#
#  DESCRIPTION:
#       Places the specified ILT onto the end of the datagram holding queue
#       of datagram messages associated with the specified TLMT.
#
#  CALLING SEQUENCE:
#       call    lld$qdg
#
#  INPUT:
#       g1 = ILT of task at 3rd level
#       g4 = LTMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Reg. g0 destroyed.
#
#*******************************************************************************
#
lld$qdg:
        ld      ltmt_dgqdepth(g4),r12   # r12 = current queue depth
        ld      ltmt_dgqdmax(g4),r11    # r11 = current maximum queue depth
        addo    1,r12,r12               # Bump the current queue depth
        ldl     ltmt_dghead(g4),r14     # r14 = first element on list
                                        # r15 = last element on list
        cmpo    r12,r11                 # Determine if a new max queue depth
        lda     ltmt_dghead(g4),r13     # r13 = base address of queue
        selg    r11,r12,r11             # Pick the largest value
        st      r12,ltmt_dgqdepth(g4)   # Save the new current queue depth
        st      r11,ltmt_dgqdmax(g4)    # Save the new maximum queue depth
        cmpobne.t 0,r14,.qdg100         # Jif queue not empty
#
# --- Case: Queue was empty.
#
        mov     r13,r15                 # set base of queue as backward thread
        stl     r14,il_fthd(g1)         # save forward/backward threads in
                                        #  ILT
        mov     g1,g0
        stl     g0,(r13)                # save ILT as head & tail pointer
        b       .qdg1000                # and we're out of here!
#
# --- Case: Queue was NOT empty. Place on end of queue.
#
.qdg100:
        mov     0,r3
        st      g1,il_fthd(r15)         # link new ILT onto end of list
        st      g1,ltmt_dgtail(g4)      # save new ILT as new tail
        st      r3,il_fthd(g1)          # clear forward thread in new ILT
        st      r15,il_bthd(g1)         # save backward thread in new ILT

.qdg1000:
        ret
#
#******************************************************************************
#
#  NAME: lld$chkdg
#
#  PURPOSE:
#       Checks if there is a datagram message on the holding queue.
#
#  DESCRIPTION:
#       Checks if there is a datagram message on the holding queue. If
#       there is and there is an available DG slot, the message is removed from
#       the queue and a call is made to lld$snddg to issue the datagram
#       message, if the initiator state has not gone offline.
#
#  CALLING SEQUENCE:
#       call    lld$chkdg
#
#  INPUT:
#       g4 = LTMT address
#       g5 = LSMT address - If the LTMT address is zero, processing is
#                           bypassed.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       No regs destroyed.
#
#******************************************************************************
#
lld$chkdg:
        cmpobe  0,g5,.chkdg1000         # Jif no LSMT

        movl    g0,r8                   # save g0-g1 in r8-r9
        ld      ltmt_dghead(g4),g1      # r14 = forward thread
        cmpobe.t 0,g1,.chkdg900         # Jif nothing on queue
        ldob    ltmt_lst(g4),r4         # r4 = link state code
        cmpobne ltmt_lst_est,r4,.chkdg500 # Jif link state established
        ld      ltmt_cimt(g4),r3        # r3 = CIMT pointer
        ldob    ci_istate(r3),r4        # r4 = Initiator State
        cmpobe  cis_on,r4,.chkdg600     # Jif the channel is still online
.chkdg500:
        call    lld$returnqdg           # Return all Queued Datagrams with No
        b       .chkdg900               #  Path error
#
# --- Determine if there is an available exchange ID. If there is, a queued DG
#     can be sent. If not, exit.
#
.chkdg600:
        ldconst ltmt_dgmax-1,r3         # r3 = Point to the last slot to test
.chkdg610:
        ld      ltmt_imsg(g4)[r3*4],r4  # get value at this entry
        cmpobe  0,r4,.chkdg650          # Jif entry is not in use
#
        cmpdeco 0,r3,r3                 # Point to the previous DG slog
        bne     .chkdg610               # Jif not all entries have been checked
        b       .chkdg900               # No space available to send DG, exit
#
.chkdg650:
        ld      ltmt_dgqdepth(g4),r12   # r12 = current queue depth
        ldl     il_fthd(g1),r14         # r14 = forward thread of ILT
                                        # r15 = backward thread
        subo    1,r12,r12               # Decrement the current queue depth
        lda     ltmt_dghead(g4),r13     # r13 = base address of waiting queue
        st      r14,il_fthd(r15)        # put forward thread from removed ILT
                                        #  as forward thread of previous ILT
        st      r12,ltmt_dgqdepth(g4)   # Store the new current queue depth
        cmpobne.t 0,r14,.chkdg700       # Jif non-zero forward thread
        mov     r13,r14                 # make base of queue the forward
                                        #  thread
        cmpobne.t r13,r15,.chkdg700     # Jif backward thread <> base of
                                        #  queue
        mov     0,r15                   # queue is now empty!
.chkdg700:
        st      r15,il_bthd(r14)        # put backward thread from removed
                                        #  ILT as backward thread of previous
                                        #  ILT.

        call    lld$snddg               # reissue the datagram message

.chkdg900:
        movl    r8,g0                   # restore g0-g1 from r8-r9

.chkdg1000:
        ret
#
#******************************************************************************
#
#  NAME: lld$returndg
#
#  PURPOSE:
#       Returns Data Gram requests with an error status.
#
#  CALLING SEQUENCE:
#       call    lld$returndg
#
#  INPUT:
#       g1 = ILT at 2nd lvl
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       No regs destroyed.
#
#******************************************************************************
#
lld$returndg:
        mov     0,r13
        lda     -ILTBIAS(g1),r15
        ld      sndg2_respptr(r15),r11  # r11 = address of response buffer

        ldconst dg_st_slld,r4           # r4 = request completion status
        ldconst dgec1_slld_nopath,r5    # r5 = ec1 status

!       stob    r4,dgrs_status(r11)     # save status
!       stob    r5,dgrs_ec1(r11)        # save ec1 value
!       stob    r13,dgrs_ec2(r11)       # clear ec2 value
!       st      r13,dgrs_resplen(r11)   # clear response length

        movq    g0,r4                   # save g0-g3 in r4-r7
        movq    g4,r8                   # save g4-g7 in r8-r11

        mov     r15,g1                  # g1 = ILT' to sndg2 lvl (2nd lvl)
        ld      il_cr(g1),r3            # r3 = ILT completion handler routine
        callx   (r3)                    # call ILT completion handler routine

        movq    r4,g0                   # restore g0-g3 from r4-r7
        movq    r8,g4                   # restore g4-g7 from r8-r11
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: lld$returnqdg
#
#  PURPOSE:
#       Returns Data Gram requests that have been queued with an error status.
#
#  CALLING SEQUENCE:
#       call    lld$returnqdg
#
#  INPUT:
#       g4 = LTMT address - If the LTMT address is zero, processing is
#                           bypassed.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       No regs destroyed.
#
#******************************************************************************
#
lld$returnqdg:
        cmpobe  0,g4,.rqdg_1000         # Jif there is no LTMT
.rqdg_200:
        ld      ltmt_dghead(g4),g1      # r14 = forward thread
        cmpobe  0,g1,.rqdg_1000         # Jif nothing on queue
        ld      ltmt_dgqdepth(g4),r12   # r12 = current queue depth
        ldl     il_fthd(g1),r14         # r14 = forward thread of ILT
                                        # r15 = backward thread
        subo    1,r12,r12               # Decrement the current queue depth
        lda     ltmt_dghead(g4),r13     # r13 = base address of aborted queue
        st      r14,il_fthd(r15)        # put forward thread from removed ILT
                                        #  as forward thread of previous ILT
        st      r12,ltmt_dgqdepth(g4)   # Save the new current queue depth
        cmpobne 0,r14,.rqdg_210         # Jif non-zero forward thread
        mov     r13,r14                 # make base of queue the forward
                                        #  thread
        cmpobne r13,r15,.rqdg_210       # Jif backward thread <> base of
                                        #  queue
        mov     0,r15                   # queue is now empty!
.rqdg_210:
        st      r15,il_bthd(r14)        # put backward thread from removed

        call    lld$returndg            # return this dg message
        b       .rqdg_200               # continue
#
.rqdg_1000:
        ret                             # return to caller
#
#******************************************************************************
# ____________________________ SUBROUTINES ____________________________________
#
#******************************************************************************
#
#  NAME: lld$send_LDO
#
#  PURPOSE:
#       Builds up and send out a Link-level driver operational
#       VRP to the data-link manager.
#
#  DESCRIPTION:
#       Allocates an ILT & VRP, builds up a Link-level driver operational
#       VRP and sends it out to the data-link manager.
#
#  CALLING SEQUENCE:
#       call    lld$send_LDO
#
#  INPUT:
#       g0 = path (channel/interface) that is operational.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       No regs. destroyed.
#
#******************************************************************************
#
lld$send_LDO:
        movq    g0,r12                  # save g0-g3
        call    M$aivw                  # allocate an ILT/VRP combo
                                        # g1 = ILT
                                        # g2 = VRP
        mov     g1,r9                   # r9 = ILT param structure ptr.
        st      g2,vrvrp(r9)            # save VRP in ILT param. area

c       r6= ICL_IsIclPort((UINT8)r12);
        cmpobne TRUE,r6,.sendLDO_icl01  # Jif not ICL

.if ICL_DEBUG
c fprintf(stderr,"%s%s:%u <lld$send_LDO>ICL -Sending operational VRP\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # ICL_DEBUG
c       ICL_SetIclMagLinkFlag((VRP*)g2);
.sendLDO_icl01:
        stob    r12,vrchipi(r9)         # set chip instance
        lda     .sendLDO_cr,r5          # r5 = completion handler routine
        lda     vrlldop,r6              # r6 = request function code
        st      r5,inl1_cr(g1)          # save completion handler in ILT
        st      r6,vr_func(g2)          # save request function code
                                        #  and clear strategy, status
        stob    r12,vr_path(g2)         # save path of request in VRP
        lda     ILTBIAS(g1),g1          # inc. to next nest level
        lda     sendLDO_cr,r5           # r5 = completion handler routine
        st      r5,inl2_cr(g1)          # save completion handler
        movl    0,r4                    # r4-r5 = 0
        st      r4,vr_sglptr(g2)        # clear SGL pointer field in VRP
        stl     r4,vrpsiz+sg_desc0+sg_addr(g2) # Clear SGL address & length
                                        #  fields in VRP
        st      r4,vr_sglsize(g2)       # clear SGL size field
        st      r4,vrpsiz+sg_size(g2)   # clear SGL size field
        st      r4,vrpsiz+sg_scnt(g2)   # clear descriptor count
        stob    r4,vrpsiz+sg_flag(g2)
        stos    r4,vr_vid(g2)           # clear vid
        st      r4,vr_vlen(g2)          # save length
        st      r4,vr_pptr(g2)          # clear pkt. phy. addr.
        stl     r4,vr_vsda(g2)          # clear SDA
        lda     lld$rcvsrp,r11          # r11 = SRP received handler routine
        st      r11,inl2_rcvsrp(g1)     # save SRP recv. handler routine
        lda     ILTBIAS(g1),g1          # inc. to next nest level
        st      r9,inl3_FCAL(g1)        # save ILT param. address
        call    MAG$submit_vrp          # send VRP to MAGNITUDE
        movq    r12,g0                  # restore g0-g3
        ret
#
#******************************************************************************
#
#  NAME: sendLDO_cr
#
#  PURPOSE:
#       Provides the ILT completion handler routine for the Link-level
#       Driver Operational VRP. Note that this routine should never be
#       called but is included in case the MAGNITUDE firmware returns
#       the VRP, possibly with an error because it does not support the
#       new VRP function code for this function.
#
#  DESCRIPTION:
#       Deallocates the ILT and VRP back into their respective pools.
#
#  CALLING SEQUENCE:
#       call    sendLDO_cr
#
#  INPUT:
#       g0 = VRP completion status
#       g1 = ILT of VRP request
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       No regs. destroyed.
#
#******************************************************************************
#
sendLDO_cr:
        movt    g0,r12                  # save g0-g2
        lda     -ILTBIAS(g1),g1         # back up to first nest level of ILT
        ld      vrvrp(g1),g2            # g2 = VRP address
c       ICL_ClearIclMagLinkFlag((VRP*)g2);
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
.ifdef M4_DEBUG_VRP
c CT_history_printf("%s%s:%u put_vrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_VRP
c       put_vrp(g2);                    # Deallocate VRP
        movt    r12,g0                  # restore g0-g2
.sendLDO_cr:
        ret
#
#******************************************************************************
#
#  NAME: lld$rcvsrp
#
#  PURPOSE:
#       Processes SRPs associated with the link-level driver operational
#       VRP.
#
#  DESCRIPTION:
#       Validates the sr_func code in the SRP and if valid calls the
#       appropriate handler routine to further process the SRP. If
#       the sr_func code is invalid, returns the SRP with the
#       appropriate SRP error code.
#
#  CALLING SEQUENCE:
#       call    lld$rcvsrp
#
#  INPUT:
#       g1 = sec. ILT at the otl2 nest level
#       g2 = SRP address
#       g7 = pri. ILT at inl1 nest level
#       g9 = pri. ILT at inl2 nest level
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       Regs. r3-r15/g0-g11 can be destroyed.
#
#******************************************************************************
#
lld$rcvsrp:
        lda     lld_srp_qu,r11          # r11 = queue base address
        b       K$cque                  # and queue ILT/SRP to task
#
#******************************************************************************
#
#  NAME: lld$send_MLE
#
#  PURPOSE:
#       Builds up and send out a MAGNITUDE Link Established
#       VRP to the data-link manager.
#
#  DESCRIPTION:
#       Allocates an ILT & VRP, builds up a MAGNITUDE Link Established
#       VRP and sends it out to the data-link manager.
#
#  CALLING SEQUENCE:
#       call    lld$send_MLE
#
#  INPUT:
#       g4 = LTMT address of MAGNITUDE link
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       No regs. destroyed.
#
#******************************************************************************
#
lld$send_MLE:
        movq    g0,r12                  # save g0-g3
        ld      ltmt_tmt(g4),r11        # r11 = assoc. TMT address
        cmpobe  0,r11,.sendMLE_1000     # Jif no TMT assoc. with LTMT
        call    M$aivw                  # allocate an ILT/VRP combo
                                        # g1 = ILT
                                        # g2 = VRP
        mov     g1,r9                   # r9 = ILT param structure ptr.
        st      g2,vrvrp(r9)            # save VRP in ILT param. area
        ld      ltmt_cimt(g4),r5        # r5 = CIMT
        ldob    ci_num(r5),r5           # r5 = Interface number

c       r6 = ICL_IsIclPort((UINT8)r5);
        cmpobne TRUE,r6,.sendMLE_icl05  # Jif not ICL
c       ICL_SetIclMagLinkFlag((VRP*)g2);
.if ICL_DEBUG
c fprintf(stderr,"%s%s:%u <lld$sendMLE>ICL..port. filling VRP\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # ICL_DEBUG
.sendMLE_icl05:
        stob    r5,vrchipi(r9)          # set the chip instance
        stob    r5,vr_path(g2)          # save the path number in the vrp
        lda     .sendMLE_cr,r5          # r5 = completion handler routine
        lda     vrmlest,r6              # r6 = request function code
        st      r5,inl1_cr(g1)          # save completion handler in ILT
        st      r6,vr_func(g2)          # save request function code
                                        #  and clear strategy, status
        lda     ILTBIAS(g1),g1          # inc. to next nest level
        lda     sendMLE_cr,r5           # r5 = completion handler routine
        st      r5,inl2_cr(g1)          # save completion handler
        movl    0,r4                    # r4-r5 = 0
        st      r4,vr_sglptr(g2)        # clear SGL pointer field in VRP
        st      r4,vr_pptr(g2)          # clear pkt. phy. addr.
        lda     sghdrsiz+sgdescsiz,r6   # r6 = SGL size
        st      r6,vr_sglsize(g2)       # save SGL size field
        st      r6,vrpsiz+sg_size(g2)   # save SGL size field
        mov     1,r6                    # r6 = descriptor count
        st      r6,vrpsiz+sg_scnt(g2)   # save descriptor count
        ldconst 0x80,r6                 # r6 = SGL flags byte
        stob    r6,vrpsiz+sg_flag(g2)   # save SGL flag byte
        ldconst magdt_size,g0           # g0 = MAGNITUDE descriptor table size
        st      g0,vrpsiz+sg_desc0+sg_len(g2) # save byte count in VRP
c       g0 = m_asglbuf(g0);             # allocate a SGL and buffer
        st      g4,lvrp2_ltmt(g1)       # save LTMT address in ILT
        st      g0,lvrp2_sgl(g1)        # save SGL/buffer address in ILT
        ld      sghdrsiz+sg_addr(g0),r10 # r10 = buffer address
        st      r10,vr_mle_magdt(g2)    # save buffer address in VRP
        st      g4,vr_mle_lldid(g2)     # save LTMT address in VRP
        ld      ltmt_dlmid(g4),r11      # r11 = DLM session ID
        st      r11,vr_mle_dlmid(g2)    # save DLM session ID in VRP
#
# --- Pack MAGNITUDE Descriptor Table
#
        st      r11,magdt_dlmid(r10)    # save DLM session ID in MAGDT
? # crash - cqt# 22533 - 2008-05-20 -- FE LTMT - failed with r11 loading a zero.
        ld      ltmt_tmt(g4),r11        # r11 = assoc. TMT address
# If m_asglbuf task switches, then the LTMT might have been freed, Free sgl and exit.
c if (r11 == 0) {
        call    M$rsglbuf               # Free sgl and buffer. (g0 is sgl input)
        b       .sendMLE_1000           # Jif no TMT assoc. with LTMT
c }

        ldq     tm_N_name(r11),r4       # r4-r7 = node and port MAC addresses
        stq     r4,magdt_nwwn(r10)      # save MAC addresses

        ld      tm_alpa(r11),r4         # r4 = AL-PA address
        st      r4,magdt_alpa(r10)      # save AL-PA address

        ld      ltmt_sn(g4),r4          # r4 = MAG serial number
        st      r4,magdt_sn(r10)        # save MAG serial number

        ldl     ltmt_pname(g4),r4       # r4-r5 = node name
        stl     r4,magdt_name(r10)      # save node name

        ld      ltmt_ip(g4),r4          # r4 = assigned IP address
        st      r4,magdt_ip(r10)        # save assigned IP address

        ldob    ltmt_path(g4),r4        # r4 = path #
        stob    r4,magdt_path(r10)      # save path #

        ldob    ltmt_cl(g4),r4          # r4 = assigned cluster #
        stob    r4,magdt_cl(r10)        # save assigned cluster #

        ldob    ltmt_vdcnt(g4),r4       # r4 = # VDisks
        stob    r4,magdt_vds(r10)       # save # VDisks

        ld      ltmt_alias(g4),r4       # r4 = alias node serial number
        st      r4,magdt_alias(r10)     # save alias node serial number

        ldob    ltmt_flag1(g4),r4       # r4 = flag byte #1 from LTMT
        stob    r4,magdt_flag1(r10)     # save flag byte #1
#
# --- End packing MAGNITUDE Descriptor Table
#
        lda     lld$rcvsrp,r11          # r11 = SRP received handler routine
        st      r11,inl2_rcvsrp(g1)     # save SRP recv. handler routine
        st      g1,ltmt_ilt(g4)         # save ILT address in LTMT
        ldob    ltmt_dlmst(g4),r4       # r4 = DLM state code
        cmpobne.f ltmt_dst_null,r4,.sendMLE_900 # Jif not null
        ldconst ltmt_dst_pid,r4         # r4 = new DLM state code
        stob    r4,ltmt_dlmst(g4)       # save new DLM state code
.sendMLE_900:
        lda     ILTBIAS(g1),g1          # inc. to next nest level
        st      r9,inl3_FCAL(g1)        # save ILT param. address
        call    MAG$submit_vrp          # send VRP to MAGNITUDE
.sendMLE_1000:
        movq    r12,g0                  # restore g0-g3
        ret
#
#******************************************************************************
#
#  NAME: sendMLE_cr
#
#  PURPOSE:
#       Provides the ILT completion handler routine for the MAGNITUDE
#       Link Established VRP. This routine saves the returned data-link
#       manager session ID from the MAGNITUDE Descriptor Table in the
#       associated LTMT and returns the resources back into their
#       pools.
#
#  DESCRIPTION:
#       Deallocates the ILT and VRP back into their respective pools.
#       Deallocates the memory used for the MAGNITUDE Descriptor
#       Table back into the memory pool. Saves the data-link manager's
#       session ID in the associated LTMT.
#
#  CALLING SEQUENCE:
#       call    sendMLE_cr
#
#  INPUT:
#       g0 = VRP completion status
#       g1 = ILT of VRP request
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       No regs. destroyed.
#
#******************************************************************************
#
sendMLE_cr:
        mov     0,r3
        movt    g0,r12                  # save g0-g2
                                        # r12 = VRP completion status
                                        # r13 = ILT of VRP request
        ld      lvrp2_ltmt(g1),r4       # r4 = assoc. LTMT address
        ld      lvrp2_sgl(g1),g0        # g0 = SGL/buffer used for MAGDT

        ld      ltmt_ilt(r4),r5         # r5 = assoc. ILT address from LTMT
        cmpobne.f g1,r5,.sendMLEcr_100  # Jif not this ILT
        st      r3,ltmt_ilt(r4)         # clear ILT from LTMT
        ldconst ltmt_dst_id,r5          # r5 = new DLM state code
        stob    r5,ltmt_dlmst(r4)       # save new DLM state code
        cmpobne.f 0,r12,.sendMLEcr_100  # Jif VRP error status reported
        ld      sghdrsiz+sg_addr(g0),r6 # r6 = MAGDT address
        ld      magdt_dlmid(r6),r7      # r7 = DLM session ID
        st      r7,ltmt_dlmid(r4)       # save DLM session ID in LTMT
.sendMLEcr_100:
        st      0,lvrp2_sgl(g1)         # make sure cannot use sgl pointer again
        call    M$rsglbuf               # release SGL/buffer back into pool
        lda     -ILTBIAS(g1),g1         # back up to first nest level of ILT
        ld      vrvrp(g1),g2            # g2 = VRP address
c       ICL_ClearIclMagLinkFlag((VRP*)g2);
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
.ifdef M4_DEBUG_VRP
c CT_history_printf("%s%s:%u put_vrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_VRP
c       put_vrp(g2);                    # Deallocate VRP
        movt    r12,g0                  # restore g0-g2
        ret

.sendMLE_cr:
        ret
#
#******************************************************************************
#
#  NAME: lld$send_MLT
#
#  PURPOSE:
#       Builds up and send out a MAGNITUDE Link Terminated
#       VRP to the data-link manager.
#
#  DESCRIPTION:
#       Allocates an ILT & VRP, builds up a MAGNITUDE Link Terminated
#       VRP and sends it out to the data-link manager.
#
#  CALLING SEQUENCE:
#       call    lld$send_MLT
#
#  INPUT:
#       g4 = assoc. LTMT being terminated
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       No regs. destroyed.
#
#******************************************************************************
#
lld$send_MLT:
        movq    g0,r12                  # save g0-g3
        ld      ltmt_dlmid(g4),r6       # r6 = assoc. DLM session ID
        cmpobe  0,r6,.sendMLT_1000      # Jif no DLM ID defined
        call    M$aivw                  # allocate an ILT/VRP combo
                                        # g1 = ILT
                                        # g2 = VRP
        mov     g1,r9                   # r9 = ILT param structure ptr.
        st      g2,vrvrp(r9)            # save VRP in ILT param. area
        ld      ltmt_cimt(g4),r5        # r5 = CIMT
        ldob    ci_num(r5),r5           # r5 = Interface number

c       r6= ICL_IsIclPort((UINT8)r5);
        cmpobne TRUE,r6,.sendMLT_icl01  # Jif not ICL
c       ICL_SetIclMagLinkFlag((VRP*)g2);
.if ICL_DEBUG
c fprintf(stderr,"%s%s:%u <lld$send_MLT> ICL...\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # ICL_DEBUG
.sendMLT_icl01:
        stob    r5,vrchipi(r9)          # set the chip instance
        stob    r5,vr_path(g2)          # save the path number in the vrp
        lda     .sendMLT_cr,r5          # r5 = completion handler routine
        lda     vrmlterm,r6             # r6 = request function code
        st      r5,inl1_cr(g1)          # save completion handler in ILT
        st      r6,vr_func(g2)          # save request function code
                                        #  and clear strategy, status
        ld      ltmt_dlmid(g4),r6       # r6 = assoc. DLM session ID
        st      r6,vr_mlt_dlmid(g2)     # save DLM session ID in VRP
        st      g4,vr_mlt_lldid(g2)     # save assoc. LTMT in VRP
        lda     ILTBIAS(g1),g1          # inc. to next nest level
        lda     sendMLT_cr,r5           # r5 = completion handler routine
        st      r5,inl2_cr(g1)          # save completion handler
        movl    0,r4                    # r4-r5 = 0
        st      r4,vr_sglptr(g2)        # clear SGL pointer field in VRP
        stl     r4,vrpsiz+sg_desc0+sg_addr(g2) # Clear SGL address & length
                                        #  fields in VRP
        st      r4,vr_sglsize(g2)       # clear SGL size field
        st      r4,vrpsiz+sg_size(g2)   # clear SGL size field
        st      r4,vrpsiz+sg_scnt(g2)   # clear descriptor count
        stob    r4,vrpsiz+sg_flag(g2)
        st      r4,vr_pptr(g2)          # clear pkt. phy. addr.
        lda     lld$rcvsrp,r11          # r11 = SRP received handler routine
        st      r11,inl2_rcvsrp(g1)     # save SRP recv. handler routine
        lda     ILTBIAS(g1),g1          # inc. to next nest level
        st      r9,inl3_FCAL(g1)        # save ILT param. address
        call    MAG$submit_vrp          # send VRP to MAGNITUDE
.sendMLT_1000:
        movq    r12,g0                  # restore g0-g3
        ret
#
#******************************************************************************
#
#  NAME: sendMLT_cr
#
#  PURPOSE:
#       Provides the ILT completion handler routine for the MAGNITUDE
#       Link Terminated VRP.
#
#  DESCRIPTION:
#       Deallocates the ILT and VRP back into their respective pools.
#
#  CALLING SEQUENCE:
#       call    sendMLT_cr
#
#  INPUT:
#       g0 = VRP completion status
#       g1 = ILT of VRP request
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       No regs. destroyed.
#
#******************************************************************************
#
sendMLT_cr:
        movt    g0,r12                  # save g0-g2
        lda     -ILTBIAS(g1),g1         # back up to first nest level of ILT
        ld      vrvrp(g1),g2            # g2 = VRP address
c       ICL_ClearIclMagLinkFlag((VRP*)g2);
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
.ifdef M4_DEBUG_VRP
c CT_history_printf("%s%s:%u put_vrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_VRP
c       put_vrp(g2);                    # Deallocate VRP
        movt    r12,g0                  # restore g0-g2
.sendMLT_cr:
        ret
#
#******************************************************************************
#
#  NAME: lld$send_FTI
#
#  PURPOSE:
#       Builds up and send out a Foreign Target Identified
#       VRP to the data-link manager.
#
#  DESCRIPTION:
#       Allocates an ILT & VRP, builds up a Foreign Target Identified
#       VRP and sends it out to the data-link manager.
#
#  CALLING SEQUENCE:
#       call    lld$send_FTI
#
#  INPUT:
#       g4 = LTMT address of Foreign Target
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       No regs. destroyed.
#
#******************************************************************************
#
lld$send_FTI:
        movq    g0,r12                  # save g0-g3
        call    M$aivw                  # allocate an ILT/VRP combo
                                        # g1 = ILT
                                        # g2 = VRP
        mov     g1,g3                   # g3 = ILT param structure ptr.
        st      g2,vrvrp(g3)            # save VRP in ILT param. area
        ld      ltmt_cimt(g4),r5        # r5 = CIMT
        ldob    ci_num(r5),r5           # r5 = Interface number
        stob    r5,vrchipi(g3)          # set the chip instance
        stob    r5,vr_path(g2)          # save the path number in the vrp
        lda     .sendFTI_cr,r5          # r5 = completion handler routine
        lda     vrftid,r6               # r6 = request function code
        st      r5,inl1_cr(g1)          # save completion handler in ILT
        st      r6,vr_func(g2)          # save request function code
                                        #  and clear strategy, status
        lda     ILTBIAS(g1),g1          # inc. to next nest level
        lda     sendFTI_cr,r5           # r5 = completion handler routine
        st      r5,inl2_cr(g1)          # save completion handler
        movl    0,r4                    # r4-r5 = 0
        st      r4,vr_sglptr(g2)        # clear SGL pointer field in VRP
        st      r4,vr_pptr(g2)          # clear pkt. phy. addr.
        lda     sghdrsiz+sgdescsiz,r6   # r6 = SGL size
        st      r6,vr_sglsize(g2)       # save SGL size field
        st      r6,vrpsiz+sg_size(g2)   # save SGL size field
        mov     1,r6                    # r6 = descriptor count
        st      r6,vrpsiz+sg_scnt(g2)   # save descriptor count
        ldconst 0x80,r6                 # r6 = SGL flags byte
        stob    r6,vrpsiz+sg_flag(g2)   # save SGL flag byte
        ldconst ftdt_size,g0            # g0 = Foreign target descriptor
                                        #  table size
        st      g0,vrpsiz+sg_desc0+sg_len(g2) # save byte count in VRP
c       g0 = m_asglbuf(g0);             # allocate a SGL and buffer
        st      g4,lvrp2_ltmt(g1)       # save LTMT address in ILT
        st      g0,lvrp2_sgl(g1)        # save SGL/buffer address in ILT
        ld      sghdrsiz+sg_addr(g0),r10 # r10 = buffer address
        st      r10,vr_fti_ftdt(g2)     # save buffer address in VRP
        st      g4,vr_fti_lldid(g2)     # save LTMT address in VRP
        ld      ltmt_dlmid(g4),r11      # r11 = DLM session ID
        st      r11,vr_fti_dlmid(g2)    # save DLM session ID in VRP
#
# --- Pack Foreign Target Descriptor Table
#
        st      r11,ftdt_dlmid(r10)     # save DLM session ID in FTDT
        ld      ltmt_tmt(g4),r11        # r11 = assoc. TMT address

        ldq     tm_N_name(r11),r4       # r4-r7 = node and port MAC addresses
        stq     r4,ftdt_nwwn(r10)       # save MAC addresses

        ld      tm_alpa(r11),r4         # r4 = AL-PA address
        st      r4,ftdt_alpa(r10)       # save AL-PA address

        ldl     tm_venid(r11),r4        # r4-r5 = vendor ID
        stl     r4,ftdt_venid(r10)      # save vendor ID

        ldq     tm_proid(r11),r4        # r4-r7 = product ID
        stq     r4,ftdt_prid(r10)       # save product ID

        ld      tm_version(r11),r4      # r4 = version number
        st      r4,ftdt_version(r10)    # save version number
#TEMPORARY - uncomment out the following instructions
#        ldob    tm_snlen(r11),r4        # r4 = serial number length
#        stob    r4,ftdt_snlen(r10)      # save serial number length
        ldq     tm_sn(r11),r4           # r4-r7 = serial number
        stq     r4,ftdt_sn(r10)         # save serial number

        ldob    tm_pdt(r11),r4          # r4 = device type code
        stob    r4,ftdt_dtype(r10)      # save device type code
#
# --- Count the number of LUNs found during the initiator driver's
#       discovery process.
#
        lda     tm_tlmtdir(r11),r9      # r9 = pointer into TMT TLMT directory
        ldconst MAXLUN,r8               # r8 = max. # LUNs to check for
        ldconst 0,r4                    # r4 = LUN counter
.sendFTI_200:
        ld      (r9),r6                 # r6 = assoc. TLMT (0 denotes no LUN)
        cmpobe.t 0,r6,.sendFTI_220      # Jif no LUN supported
        addo    1,r4,r4                 # inc. LUN counter
.sendFTI_220:
        addo    4,r9,r9                 # inc. to next TLMT in directory
        subo    1,r8,r8                 # dec. max. LUN count
        cmpobne.t 0,r8,.sendFTI_200     # Jif more LUNs to check
        stob    r4,ftdt_luns(r10)       # save # LUNs
#
# --- End packing Foreign Target Descriptor Table
#
        lda     lld$rcvsrp,r11          # r11 = SRP received handler routine
        st      r11,inl2_rcvsrp(g1)     # save SRP recv. handler routine
        st      g1,ltmt_ilt(g4)         # save ILT address in LTMT
        ldob    ltmt_dlmst(g4),r4       # r4 = DLM state code
        cmpobne.f ltmt_dst_null,r4,.sendFTI_900 # Jif not null
        ldconst ltmt_dst_pid,r4         # r4 = new DLM state code
        stob    r4,ltmt_dlmst(g4)       # save new DLM state code
.sendFTI_900:
        lda     ILTBIAS(g1),g1          # inc. to next nest level
        st      g3,inl3_FCAL(g1)        # save ILT param. address
        call    MAG$submit_vrp          # send VRP to MAGNITUDE
        movq    r12,g0                  # restore g0-g3
        ret
#
#******************************************************************************
#
#  NAME: sendFTI_cr
#
#  PURPOSE:
#       Provides the ILT completion handler routine for the Foreign
#       Target Identified VRP. This routine saves the returned data-link
#       manager session ID from the Foreign Target Descriptor Table in the
#       associated LTMT and returns the resources back into their
#       pools.
#
#  DESCRIPTION:
#       Deallocates the ILT and VRP back into their respective pools.
#       Deallocates the memory used for the Foreign Target Descriptor
#       Table back into the memory pool. Saves the data-link manager's
#       session ID in the associated LTMT.
#
#  CALLING SEQUENCE:
#       call    sendFTI_cr
#
#  INPUT:
#       g0 = VRP completion status
#       g1 = ILT of VRP request
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       No regs. destroyed.
#
#******************************************************************************
#
sendFTI_cr:
        mov     0,r3
        movt    g0,r12                  # save g0-g2
                                        # r12 = VRP completion status
                                        # r13 = ILT of VRP request
        ld      lvrp2_ltmt(g1),r4       # r4 = assoc. LTMT address
        ld      lvrp2_sgl(g1),g0        # g0 = SGL/buffer used for FTDT

        ld      ltmt_ilt(r4),r5         # r5 = assoc. ILT address from LTMT
        cmpobne.f g1,r5,.sendFTIcr_100  # Jif not this ILT
        st      r3,ltmt_ilt(r4)        # clear ILT from LTMT
        ldconst ltmt_dst_id,r5          # r5 = new DLM state code
        stob    r5,ltmt_dlmst(r4)       # save new DLM state code
        cmpobne.f 0,r12,.sendFTIcr_100  # Jif VRP error status reported
        ld      sghdrsiz+sg_addr(g0),r6 # r6 = FTDT address
        ld      ftdt_dlmid(r6),r7       # r7 = DLM session ID
        st      r7,ltmt_dlmid(r4)       # save DLM session ID in LTMT
.sendFTIcr_100:
        st      0,lvrp2_sgl(g1)         # make sure cannot use sgl pointer again
        call    M$rsglbuf               # release SGL/buffer back into pool
        lda     -ILTBIAS(g1),g1         # back up to first nest level of ILT
        ld      vrvrp(g1),g2            # g2 = VRP address
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
.ifdef M4_DEBUG_VRP
c CT_history_printf("%s%s:%u put_vrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_VRP
c       put_vrp(g2);                    # Deallocate VRP
        movt    r12,g0                  # restore g0-g2
.sendFTI_cr:
        ret
#
#******************************************************************************
#
#  NAME: lld$send_FTT
#
#  PURPOSE:
#       Builds up and send out a Foreign Target Terminated
#       VRP to the data-link manager.
#
#  DESCRIPTION:
#       Allocates an ILT & VRP, builds up a Foreign Target Terminated
#       VRP and sends it out to the data-link manager.
#
#  CALLING SEQUENCE:
#       call    lld$send_FTT
#
#  INPUT:
#       g4 = assoc. LTMT being terminated
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       No regs. destroyed.
#
#******************************************************************************
#
lld$send_FTT:
        movq    g0,r12                  # save g0-g3
        call    M$aivw                  # allocate an ILT/VRP combo
                                        # g1 = ILT
                                        # g2 = VRP
        mov     g1,r9                   # r9 = ILT param structure ptr.
        st      g2,vrvrp(r9)            # save VRP in ILT param. area
        ld      ltmt_cimt(g4),r5        # r5 = CIMT
        ldob    ci_num(r5),r5           # r5 = Interface number
        stob    r5,vrchipi(r9)          # set the chip instance
        stob    r5,vr_path(g2)          # save the path number in the vrp
        lda     .sendFTT_cr,r5          # r5 = completion handler routine
        lda     vrftterm,r6             # r6 = request function code
        st      r5,inl1_cr(g1)          # save completion handler in ILT
        st      r6,vr_func(g2)          # save request function code
                                        #  and clear strategy, status
        ld      ltmt_dlmid(g4),r6       # r6 = assoc. DLM session ID
        st      r6,vr_ftt_dlmid(g2)     # save DLM session ID in VRP
        st      g4,vr_ftt_lldid(g2)     # save assoc. LTMT in VRP
        lda     ILTBIAS(g1),g1          # inc. to next nest level
        lda     sendFTT_cr,r5           # r5 = completion handler routine
        st      r5,inl2_cr(g1)          # save completion handler
        movl    0,r4                    # r4-r5 = 0
        st      r4,vr_sglptr(g2)        # clear SGL pointer field in VRP
        stl     r4,vrpsiz+sg_desc0+sg_addr(g2) # Clear SGL address & length
                                        #  fields in VRP
        st      r4,vr_sglsize(g2)       # clear SGL size field
        st      r4,vrpsiz+sg_size(g2)   # clear SGL size field
        st      r4,vrpsiz+sg_scnt(g2)   # clear descriptor count
        stob    r4,vrpsiz+sg_flag(g2)
        st      r4,vr_pptr(g2)          # clear pkt. phy. addr.
        lda     lld$rcvsrp,r11          # r11 = SRP received handler routine
        st      r11,inl2_rcvsrp(g1)     # save SRP recv. handler routine
        lda     ILTBIAS(g1),g1          # inc. to next nest level
        st      r9,inl3_FCAL(g1)        # save ILT param. address
        call    MAG$submit_vrp          # send VRP to MAGNITUDE
        movq    r12,g0                  # restore g0-g3
        ret
#
#******************************************************************************
#
#  NAME: sendFTT_cr
#
#  PURPOSE:
#       Provides the ILT completion handler routine for the Foreign
#       Target Terminated VRP.
#
#  DESCRIPTION:
#       Deallocates the ILT and VRP back into their respective pools.
#
#  CALLING SEQUENCE:
#       call    sendFTT_cr
#
#  INPUT:
#       g0 = VRP completion status
#       g1 = ILT of VRP request
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       No regs. destroyed.
#
#******************************************************************************
#
sendFTT_cr:
        movt    g0,r12                  # save g0-g2
        lda     -ILTBIAS(g1),g1         # back up to first nest level of ILT
        ld      vrvrp(g1),g2            # g2 = VRP address
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
.ifdef M4_DEBUG_VRP
c CT_history_printf("%s%s:%u put_vrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_VRP
c       put_vrp(g2);                    # Deallocate VRP
        movt    r12,g0                  # restore g0-g2
.sendFTT_cr:
        ret
#
#******************************************************************************
#
#  NAME: lld$send_ST
#
#  PURPOSE:
#       Builds up and send out a Session Terminated VRP to the
#       data-link manager.
#
#  DESCRIPTION:
#       Allocates an ILT & VRP, builds up a Session Terminated VRP
#       and sends it out to the data-link manager.
#
#  CALLING SEQUENCE:
#       call    lld$send_ST
#
#  INPUT:
#       g3 = assoc. LSMT being terminated
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       No regs. destroyed.
#
#******************************************************************************
#
lld$send_ST:
        ld      lsmt_dlmid(g3),r6       # r6 = assoc. DLM session ID
        cmpobe.f 0,r6,.sendST_1000      # Jif no DLM session ID defined
        movq    g0,r12                  # save g0-g3
        call    M$aivw                  # allocate an ILT/VRP combo
                                        # g1 = ILT
                                        # g2 = VRP
        mov     g1,r9                   # r9 = ILT param structure ptr.
        st      g2,vrvrp(r9)            # save VRP in ILT param. area
        ld      lsmt_ltmt(g3),r6        # r5 = LTMT associated with this LSMT
        ld      ltmt_cimt(r6),r5        # r5 = CIMT
        ldob    ci_num(r5),r5           # r5 = Interface number

c       r6= ICL_IsIclPort((UINT8)r5);
        cmpobne TRUE,r6,.sendST_icl01   # Jif not ICL

.sendST_icl01:
        stob    r5,vrchipi(r9)          # set the chip instance
        stob    r5,vr_path(g2)          # save the interface number in the vrp
        lda     sendST_cr,r5            # r5 = completion handler routine
        lda     vrsterm,r6              # r6 = request function code
        st      r5,inl1_cr(g1)          # save completion handler in ILT
        st      r6,vr_func(g2)          # save request function code
                                        #  and clear strategy, status
        ld      lsmt_dlmid(g3),r6       # r6 = assoc. DLM session ID
        st      r6,vr_st_dlmid(g2)      # save DLM session ID in VRP
        st      g3,vr_st_lldid(g2)      # save assoc. LSMT in VRP
        lda     ILTBIAS(g1),g1          # inc. to next nest level
        lda     sendST_cr,r5            # r5 = completion handler routine
        st      r5,inl2_cr(g1)          # save completion handler
        movl    0,r4                    # r4-r5 = 0
        st      r4,vr_sglptr(g2)        # clear SGL pointer field in VRP
        stl     r4,vrpsiz+sg_desc0+sg_addr(g2) # Clear SGL address & length
                                        #  fields in VRP
        st      r4,vr_sglsize(g2)       # clear SGL size field
        st      r4,vrpsiz+sg_size(g2)   # clear SGL size field
        st      r4,vrpsiz+sg_scnt(g2)   # clear descriptor count
        stob    r4,vrpsiz+sg_flag(g2)
        st      r4,vr_pptr(g2)          # clear pkt. phy. addr.
        lda     lld$rcvsrp,r11          # r11 = SRP received handler routine
        st      r11,inl2_rcvsrp(g1)     # save SRP recv. handler routine
        lda     ILTBIAS(g1),g1          # inc. to next nest level
        st      r9,inl3_FCAL(g1)        # save ILT param. address
        call    MAG$submit_vrp          # send VRP to MAGNITUDE
        movq    r12,g0                  # restore g0-g3
.sendST_1000:
        ret
#
#******************************************************************************
#
#  NAME: sendST_cr
#
#  PURPOSE:
#       Provides the ILT completion handler routine for the Session
#       Terminate VRP.
#
#  DESCRIPTION:
#       Deallocates the ILT and VRP back into their respective pools.
#
#  CALLING SEQUENCE:
#       call    sendST_cr
#
#  INPUT:
#       g0 = VRP completion status
#       g1 = ILT of VRP request
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       No regs. destroyed.
#
#******************************************************************************
#
sendST_cr:
        movt    g0,r12                  # save g0-g2
        lda     -ILTBIAS(g1),g1         # back up to first nest level of ILT
        ld      vrvrp(g1),g2            # g2 = VRP address
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
.ifdef M4_DEBUG_VRP
c CT_history_printf("%s%s:%u put_vrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_VRP
c       put_vrp(g2);                    # Deallocate VRP
        movt    r12,g0                  # restore g0-g2
        ret
#
#******************************************************************************
#
#  NAME: lld$rem_lsmt
#
#  PURPOSE:
#       Removes the LSMT from the session list in the LTMT if
#       found on list.
#
#  DESCRIPTION:
#       Goes through LSMT session list in LTMT trying to find the
#       LSMT and if found removes it from the list.
#
#  CALLING SEQUENCE:
#       call    lld$rem_lsmt
#
#  INPUT:
#       g3 = LSMT address to remove from list
#       g4 = assoc. LTMT address
#
#  OUTPUT:
#       g3 = LSMT address removed, or zero if not on session queue...
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
lld$rem_lsmt:
        ld      ltmt_seshead(g4),r4     # r4 = first LSMT on list
        cmpobe.f 0,r4,.remlsmt_1000     # Jif no LSMTs on list
        cmpobne.f g3,r4,.remlsmt_100    # Jif not the first on list
        ld      lsmt_link(r4),r5        # r5 = next LSMT on list
        st      r5,ltmt_seshead(g4)     # save new head member
        cmpobne.t 0,r5,.remlsmt_1000    # Jif not the last member on list
        st      r5,ltmt_sestail(g4)     # clear tail member
        b       .remlsmt_1000

.remlsmt_100:
        mov     r4,r5                   # r5 = previous LSMT address
        ld      lsmt_link(r4),r4        # r4 = next LSMT on list
        cmpobe.f 0,r4,.remlsmt_900     # Jif no more LSMTs on list
        cmpobne.f g3,r4,.remlsmt_100    # Jif not a match
        ld      lsmt_link(r4),r6        # r6 = next LSMT on list
        st      r6,lsmt_link(r5)        # remove LSMT from list
        cmpobne.t 0,r6,.remlsmt_1000    # Jif not the last on list
        st      r5,ltmt_sestail(g4)     # save new tail member

        b       .remlsmt_1000
#
#       specified LSMT was not on this LTMT's session queue. leave
#       LSMT as-is, return a zero pointer for use by put_lsmt function,
#       immediately following...
#
.remlsmt_900:
        ldconst 0,g3
        ret

.remlsmt_1000:
        mov     0,r13
        st      r13,lsmt_link(g3)       # clear link field in LSMT
        st      r13,lsmt_ilt(g3)        # remove any assoc. ILT from LSMT
        ret
#
#******************************************************************************
#
#  NAME: lld$add_ilmt
#
#  PURPOSE:
#       Adds and initializes an ILMT associated with a specified
#       VDMT (or for no associated VDMT as specified) for the
#       specified IMT associated with a MAGNITUDE link.
#
#  DESCRIPTION:
#       Allocates an ILMT, initializes various field in the ILMT,
#       places the ILMT in the IMT LUN directory as specified,
#       links the ILMT with the associated VDT if one was specified,
#       call the ILMT online event handler routine if specified
#       with the associated attributes.
#
#       *** NOTE ***
#
#       This routine must not be called if an ILMT is defined and
#       it has an associated VDMT.
#
#  CALLING SEQUENCE:
#       call    lld$add_ilmt
#
#  INPUT:
#       g0 = VDMT address to associate ILMT with
#          = 0 if no VDMT associated with ILMT
#       g1 = IMT address to add ILMT/LUN to
#       g2 = associated attributes for ILMT (LSS)
#            + online flag (MSbit=1)
#       g3 = assigned LUN # for ILMT
#            (It is assumed that the specified LUN # is currently not in use!)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
lld$add_ilmt:
        movq    g0,r12                  # save g0-g3 in r12-r15
                                        # r14 = associated attributes for ILMT
                                        #  + online flag if Bit 31 = 1
                                        # r15 = assigned LUN # for ILMT
        ld      im_ilmtdir(g1)[g3*4],g2 # g2 = possible ILMT
        cmpobne.f 0,g2,.lldaddilmt_20   # Jif ILMT already allocated

#
#******************************************************************************
#
#  INPUT:
#       r5 = pointer to LUN/ILMT directory field to define
#               new ILMT for this IMT
#       r14 = associated attributes for ILMT
#               + online flag if Bit 31 = 1
#       g0 = specified VDMT address
#       g1 = specified IMT address
#
#******************************************************************************
#
        call    mag$get_ilmt            # allocate an ILMT
                                        #   g2 = ILMT for this LUN
        st      g1,ilm_imt(g2)          # save assoc. IMT in ILMT
        st      g2,im_ilmtdir(g1)[g3*4] # save ILMT in IMT ILMT directory
        ld      im_cimt(g1),r5          # r5 = assoc. CIMT from IMT
        lda     lld$normtbl,r6          # r6 = ILMT cmd normalization tbl addr
        st      r5,ilm_cimt(g2)         # save CIMT address in ILMT
        st      r6,ilm_cmdtbl(g2)       # save cmd normalization tbl

.lldaddilmt_20:
        mov     0,r3                    # clear link
        st      r3,ilm_link(g2)
        st      g0,ilm_vdmt(g2)         # save assoc. VDMT in ILMT
        stos    r14,ilm_attr(g2)        # save attributes in ILMT
#
# --- determine if a VDMT was specified
#
        cmpobe.f 0,g0,.lldaddilmt_100   # Jif no VDMT specified

# ************************************************************************
# --- VDMT was specified, link the ILMT in to the VDMT ILMT queue.
# ************************************************************************
        ld      vdm_itail(g0),r5        # r5 = last ILMT on VDMT/ILMT list
        cmpobne.t 0,r5,.lldaddilmt_40   # Jif elements on list

        st      g2,vdm_ihead(g0)        # save ILMT as head element
        b       .lldaddilmt_50

.lldaddilmt_40:
        st      g2,ilm_link(r5)         # link new ILMT onto end of list

.lldaddilmt_50:
        st      g2,vdm_itail(g0)        # save new list tail element
#
# --- set up the command table
#
        lda     LLD$cmdtbl1b,r3         # r3 = command handler table address
        st      r3,ilm_origcmdhand(g2)  # save as original command handler table
                                        #  address
        ld      vdm_rilmt(g0),r5        # r5 = ILMT of reserving initiator
        cmpobe.t 0,r5,.lldaddilmt_60    # Jif device not reserved
        lda     LLD$cmdtbl2,r3          # r3 = command handler table for
                                        #  other initiators for this device
                                        #  while device is reserved
.lldaddilmt_60:
        st      r3,ilm_cmdhand(g2)      # save command handler table address
        b       .lldaddilmt_200

# ************************************************************************
# --- VDMT was NOT specified, set up the command table
# ************************************************************************
.lldaddilmt_100:
        lda     LLD$cmdtbl1a,r3         # r3 = command handler table address
        st      r3,ilm_origcmdhand(g2)  # save as original command handler table
                                        #  address
        st      r3,ilm_cmdhand(g2)      # save command handler table address
#
# --- common processing
#
.lldaddilmt_200:
        lda     modesns1,r5             # r5 = default working environment
                                        #  table address
        st      r5,ilm_dfenv(g2)        # save default working environment
                                        #  table address
        ld      im_ltmt(g1),r5          # r5 = assoc. LTMT address from IMT
        st      r5,ilm_ltmt(g2)         # save LTMT address in ILMT
        lda     LLD_event_tbl,r3        # r3 = ILMT event handler table
        st      r3,ilm_ehand(g2)        # save ILMT event handler table
        bbc.f   31,r14,.lldaddilmt_1000 # Jif ILMT offline
        ld      dd_online(r3),r3        # r3 = ILMT online handler routine
        movt    g4,r4                   # save g4-g6
        ld      im_cimt(g1),g4          # g4 = assoc. CIMT address
        mov     g1,g5                   # g5 = IMT address
        mov     g2,g6                   # g6 = ILMT address
        cmpobe.f 0,g4,.lldaddilmt_900   # Jif no CIMT associated with IMT
        callx   (r3)                    # call ILMT online event routine to setup
                                        #  ILMT to be placed online
.lldaddilmt_900:
        movt    r4,g4                   # restore g4-g6

.lldaddilmt_1000:
        movq    r12,g0                  # restore g0-g3 from r12-r15
        ret
#
#******************************************************************************
#
#  NAME: lld$remove_ilmt
#
#  PURPOSE:
#       Removes an ILMT associated with a specified VDMT.
#
#  DESCRIPTION:
#       Remove the possible association of the ILMT with a VDMT. If the ILMT
#       is associated with LUN 0, the ILMT is not deallocated and the command
#       table is reset. If the ILMT is not associated with LUN 0, the LIMT is
#       deallocated.
#
#  CALLING SEQUENCE:
#       call    lld$remove_ilmt
#
#  INPUT:
#       g1 = IMT address
#       g2 = ILMT address
#       g3 = assigned LUN # for ILMT
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
lld$remove_ilmt:
        mov     0,r13
        ld      ilm_vdmt(g2),r6         # r6 = associated VDMT address
        cmpobe.f 0,r6,.remilmt_050      # Jif no vdmt

        call    mag$sepilmtvdmt         # separate ilmt from vdmt
        st      r13,ilm_vdmt(g2)        # clear vdmt pointer in ILMT
#
# --- determine if this ilmt is associated with LUN 0
#
.remilmt_050:
        cmpobe.f 0,g3,.remilmt_100      # Jif LUN 0
#
# --- Not LUN 0
#
        st      r13,im_ilmtdir(g1)[g3*4]# clear ILMT from IMT ILMT directory
.ifdef M4_DEBUG_ILMT
c fprintf(stderr, "%s%s:%u put_ilmt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_ILMT
c       put_ilmt(g2);                   # Deallocate ILMT and working environment
        b       .remilmt_1000           # exit
#
# --- LUN 0
#
.remilmt_100:
        lda     LLD$cmdtbl1a,r3         # r3 = command handler table address
        st      r3,ilm_origcmdhand(g2)  # save as original command handler table
                                        #  address
        st      r3,ilm_cmdhand(g2)      # save command handler table address
.remilmt_1000:
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: lld$dem_ltmt
#
#  PURPOSE:
#       Demolishes a LTMT.
#
#  DESCRIPTION:
#       Disassociates LTMT and TMT from each other if associated.
#       Finds LTMT on list in CIMT and removes if found. Release any
#       datagram messages that may be queued to the LTMT. Deallocates
#       the LTMT back into the spare LTMT pool.
#
#  CALLING SEQUENCE:
#       call    lld$dem_ltmt
#
#  INPUT:
#       g4 = LTMT address to demolish
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
lld$dem_ltmt:
        movq    g0,r8                   # save g0-g3 in r8-r11
        mov     0,r7

        st      r7,ltmt_dlmid(g4)       # clear DLM session ID from LTMT
        ld      ltmt_tmt(g4),r4         # r4 = assoc. TMT address
        cmpobe.f 0,r4,.demltmt_20       # Jif no TMT assoc. with LTMT
        ld      tm_ltmt(r4),r5          # r5 = assoc. LTMT in TMT
        cmpobne.f g4,r5,.demltmt_20     # Jif LTMTs don't match
        st      r7,tm_ltmt(r4)          # remove LTMT address from TMT
.demltmt_20:
        st      r7,ltmt_tmt(g4)         # remove assoc. TMT address from LTMT
        ld      ltmt_cimt(g4),r4        # r4 = assoc. CIMT address
        cmpobe.f 0,r4,.demltmt_100      # Jif no CIMT defined
#
# --- Find LTMT on list and remove if found
#
        ld      ci_ltmthd(r4),r5        # r5 = top LTMT on list
        cmpobe.f 0,r5,.demltmt_100      # Jif no LTMTs on list
        cmpobne.f g4,r5,.demltmt_40     # Jif not the first LTMT on list
#
# --- LTMT is first on list
#
        ld      ltmt_link(g4),r5        # r5 = next LTMT on list
        st      r5,ci_ltmthd(r4)        # remove LTMT from list
        cmpobne.f 0,r5,.demltmt_100     # Jif not the last on list
        st      r5,ci_ltmttl(r4)        # clear tail pointer of list
        b       .demltmt_100
#
# --- LTMT not first on list
#
.demltmt_40:
        mov     r5,r6                   # r6 = previous LTMT on list
        ld      ltmt_link(r5),r5        # r5 = next LTMT on list to check
        cmpobe.f 0,r5,.demltmt_100      # Jif no more LTMTs to check on list
        cmpobne.f g4,r5,.demltmt_40     # Jif not a match
#
# --- Found LTMT on list
#
        ld      ltmt_link(g4),r5        # r5 = next LTMT on list
        st      r5,ltmt_link(r6)        # unlink LTMT from list
        cmpobne.f 0,r5,.demltmt_100     # Jif not the last on list
        st      r6,ci_ltmttl(r4)        # save new list tail member
#
# --- return all Datagram messages on the initiator datagram
#     message holding queue.
#
.demltmt_100:
        call    lld$returnqdg
#
# --- return all Datagram messages on the initiator exchange
#     table.
#
        mov     0,r4                    # r4 = 0

.demltmt_210:
        ld      ltmt_imsg(g4)[r4*4],g1  # r5 = primary ILT
        cmpobe.t 0,g1,.demltmt_290      # Jif entry empty

        st      r7,ltmt_imsg(g4)[r4*4]  # clear this entry

        ld      sdg_writeILT(g1),r6     # load write ILT
        cmpobe  0,r6,.demltmt_220       # Jif no write ILT

        st      r7,lirp1_lldid(r6)      # clear LSMT pointer
        st      r7,sdg_writeILT(g1)     # clear write ILT pointer

.demltmt_220:
        ld      sdg_readILT(g1),r6      # load read ILT
        cmpobe  0,r6,.demltmt_230       # Jif no read ILT

        st      r7,lirp1_lldid(r6)      # clear LSMT pointer
        st      r7,sdg_readILT(g1)      # clear read ILT pointer

.demltmt_230:
        call    lld$returndg            # return this dg message

.demltmt_290:
        addo    1,r4,r4                 # r4 = r4 +1
        cmpobne ltmt_dgmax,r4,.demltmt_210 # Jif not end of table
.ifdef M4_DEBUG_LTMT
c fprintf(stderr, "%s%s:%u put_ltmt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g4);
.endif # M4_DEBUG_LTMT
c       put_ltmt(g4);                   # deallocate LTMT into pool
c       g4 = 0;
        movq    r8,g0                   # restore g0-r3 from r8-r11
        ret
#
#******************************************************************************
#
#  NAME: lld$sendopen
#
#  PURPOSE:
#       Packs and sends an open session request to the initiator driver.
#
#  DESCRIPTION:
#       Allocates an ILT/IRP combo and packs and sends an open
#       session request for the specified target/LUN.
#
#  CALLING SEQUENCE:
#       call    lld$sendopen
#
#  INPUT:
#       g0 = LUN # to open session to
#       g1 = user defined lirp1_g0 value
#       g2 = ILT completion handler routine address
#       g3 = LSMT associated with session being opened
#       g4 = assoc. LTMT address
#       g5 = user defined lirp1_g1 value
#       g6 = session termination routine addr
#       g7 = flags
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
lld$sendopen:
        movq    g0,r12                  # save g0-g3
                                        # r12 = LUN #
                                        # r13 = user defined lirp1_g0 value
                                        # r14 = ILT completion handler routine
                                        # r15 = LSMT address
        call    M$ailtirp               # allocate an ILT/IRP combo (meal?)
                                        # g0 = IRP address
                                        # g1 = ILT address
        st      g0,lirp1_irp(g1)        # save IRP address in ILT
        st      r14,lirp1_cr(g1)        # save completion routine
        st      r15,lirp1_lldid(g1)     # save LSMT address in ILT
        stob    g7,lirp1_flags(g1)      # save flags in ILT
        movl    g0,r10                  # r10 = IRP used for OPEN
                                        # r11 = ILT used for OPEN
        st      r13,lirp1_g0(g1)        # save user defined reg. g0 value
        st      g5,lirp1_g1(g1)         # save user defined reg. g1 value
        lda     ILTBIAS(g1),g1          # g1 = ILT/IRP at nest level 2
        st      g0,lirp2_irp(g1)        # save IRP address in nest level 2
        ldconst rfc_open,r4             # r4 = IRP request function code
        st      r4,irp_RFC(g0)          # save request function code
                                        #  and clear irp_cmplt, irp_tagID fields
        st      r15,irp_Req_ID(g0)      # save LSMT as requestor ID in IRP
        ldconst 30,r4                   # r4 = default timeout value
        stos    r4,irp_defTO(g0)        # save default timeout value in IRP
        ldconst stc_tmt,r5              # r5 = session type code
        stob    r5,irp_STC(g0)          # save session type code
        ld      ltmt_cimt(g4),r4        # r4 = assoc. CIMT address
        ldob    ci_num(r4),r4           # r4 = interface #
.if ICL_DEBUG
        cmpobne ICL_PORT,r4,.sendopen_icl01
c fprintf(stderr,"%s%s:%u <lld$send_open()...ICL port..\n", FEBEMESSAGE, __FILE__, __LINE__);
.sendopen_icl01:
.endif  # ICL_DEBUG
        stob    r4,irp_IFID(g0)         # save interface ID
        ldconst 64,r4                   # r4 = max. queue depth
        stob    r4,irp_maxQ(g0)         # save max. queue depth
        ldconst 16,r4                   # r4 = default retry count
        stob    r4,irp_O_rtycnt(g0)     # save default retry count
        st      g6,irp_str(g0)          # save session termination routine adr
        ld      ltmt_tmt(g4),r4         # r4 = assoc. TMT address
        st      r4,irp_Sqlfr_TMT(g0)    # save TMT in IRP
        stos    r12,irp_Sqlfr_LUN(g0)   # save LUN in IRP
        st      r11,lsmt_ilt(r15)       # save ILT address in LSMT
        call    APL$pfr                 # request OPEN session
        movq    r12,g0                  # restore g0-g3
        ret
#
#******************************************************************************
#
#  NAME: lld$sendinfo
#
#  PURPOSE:
#       Packs and sends a CDB request to the initiator driver
#       requesting the information INQUIRY page data from the
#       peer MAGNITUDE.
#
#  DESCRIPTION:
#       Allocates an ILT/IRP combo and packs and sends a CDB
#       request for the specified session.
#
#  CALLING SEQUENCE:
#       call    lld$sendinfo
#
#  INPUT:
#       g0 = lirp1_g0 value to save in ILT
#       g1 = lirp1_g1 value to save in ILT
#       g2 = ILT completion handler routine address
#       g3 = LSMT associated with session to issue CDB on
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
        .data
info_cdb:                               # CDB pattern to use for information
                                        #  page INQUIRY command
        .byte   0x12,01,inqinfo,00
        .byte   inqinfo_size,00,00,00
        .byte   00,00,00,00
        .byte   00,00,00,00
#
        .text
#
lld$sendinfo:
        movq    g0,r12                  # save g0-g3
                                        # r12 = lirp1_g0 value to save in ILT
                                        # r13 = lirp1_g1 value to save in ILT
                                        # r14 = ILT completion handler routine
                                        # r15 = LSMT address
        call    M$ailtirp               # allocate an ILT/IRP combo (meal?)
                                        # g0 = IRP address
                                        # g1 = ILT address
        st      g0,lirp1_irp(g1)        # save IRP address in ILT
        st      r14,lirp1_cr(g1)        # save completion routine
        st      r15,lirp1_lldid(g1)      # save LSMT address in ILT
        movl    g0,r10                  # r10 = IRP used for CDB
                                        # r11 = ILT used for CDB
        st      r12,lirp1_g0(g1)        # save user reg. g0 value
        st      r13,lirp1_g1(g1)        # save user reg. g1 value
        lda     ILTBIAS(g1),g1          # g1 = ILT/IRP at nest level 2
        st      g0,lirp2_irp(g1)        # save IRP address in nest level 2
        ldconst rfc_SCSI_cmd,r4         # r4 = IRP request function code
        stob    r4,irp_RFC(g0)          # save request function code
        st      r15,irp_Req_ID(g0)      # save LSMT as requestor ID in IRP
        ld      lsmt_psid(r15),r9       # r9 = provider ID
        st      r9,irp_Pro_ID(g0)       # save provider ID in IRP
        ldconst 4,r4                    # r4 = command retry count
        stob    r4,irp_rtycnt(g0)       # save CDB retry count
        ldconst 0x04,r4                 # r4 = command timeout (secs.)
        stos    r4,irp_cmdTO(g0)        # save command timeout
        ldq     info_cdb,r4             # r4-r7 = CDB
        stq     r4,irp_CDB(g0)          # save CDB in IRP
        ldconst ttc_simple,r4           # r4 = task type code
        stob    r4,irp_Tattr_TTC(g0)    # save task type code
        ldconst 0x01,r4                 # r4 = data transfer attribute
        stob    r4,irp_Tattr_DTA(g0)    # save data transfer attribute

        ldob    irp_Tattr_flags(g0),r4  # set SLIE flag
        setbit  tpf_SLIE,r4,r4
        stob    r4,irp_Tattr_flags(g0)

        ldconst inqinfo_size,g0         # g0 = information INQUIRY data size
        st      g0,irp_expcnt(r10)      # save expected length in IRP
c       g0 = m_asglbuf(g0);             # allocate a SGL and buffer
        st      g0,irp_SGLptr(r10)      # save SGL/buffer in IRP
        st      r11,lsmt_ilt(r15)       # save ILT address in LSMT
        call    APL$pfr                 # request CDB be issued
        movq    r12,g0                  # restore g0-g3
        ret
#
#******************************************************************************
#
#  NAME: lld$sendclose
#
#  PURPOSE:
#       Packs and sends a close session request to the initiator
#       driver.
#
#  DESCRIPTION:
#       Allocates an ILT/IRP combo and packs and sends a close
#       session request for the specified target/LUN.
#
#  CALLING SEQUENCE:
#       call    lld$sendclose
#
#  INPUT:
#       g1 = user defined lirp1_g0 value
#       g2 = ILT completion handler routine address
#       g3 = LSMT associated with session being closed
#       g4 = assoc. LTMT address
#       g5 = user defined lirp1_g1 value
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
lld$sendclose:
        movq    g0,r12                  # save g0-g3
                                        # r14 = ILT completion handler routine
                                        # r15 = LSMT address
        call    M$ailtirp               # allocate an ILT/IRP combo (meal?)
                                        # g0 = IRP address
                                        # g1 = ILT address

        st      g0,lirp1_irp(g1)        # save IRP address in ILT
        st      r14,lirp1_cr(g1)        # save completion routine  (g2)
        st      r15,lirp1_lldid(g1)     # save LSMT address in ILT (g3)
        st      r13,lirp1_g0(g1)        # save g1 (ILT')           (g1)
        st      g5,lirp1_g1(g1)         # save g5                  (g5)
        lda     ILTBIAS(g1),g1          # g1 = ILT/IRP at nest level 2
        st      g0,lirp2_irp(g1)        # save IRP address in nest level 2
        ldconst rfc_close,r4            # r4 = IRP request function code
        st      r4,irp_RFC(g0)          # save request function code
                                        #  and clear irp_cmplt, irp_tagID fields
        st      r15,irp_Req_ID(g0)      # save LSMT as requestor ID in IRP
        ld      lsmt_psid(r15),r4       # r4 = initiator driver ID
        st      r4,irp_Pro_ID(g0)       # save provider ID in IRP
        cmpobne.t 0,r4,.sendclose_500   # Jif provider ID registered in LSMT
        mov     0,r3
        st      r3,irp_cmplt(g0)        # clear IRP completion status field
        lda     -ILTBIAS(g1),g1         # g1 = ILT/IRP at nest level 1
        callx   (r14)                   # and call callers completion handler
                                        #  routine
        b       .sendclose_1000
#
.sendclose_500:
        call    APL$pfr                 # request CLOSE session

.sendclose_1000:
        movq    r12,g0                  # restore g0-g3
        ret
#
#******************************************************************************
#
#  NAME: lld$io_retry
#
#  PURPOSE:
#       Queues an I/O operation request message to the I/O operation retry
#       message queue.
#
#  DESCRIPTION:
#       Queues an I/O operation request message to the I/O operation retry
#       message queue.
#
#  CALLING SEQUENCE:
#       call    lld$io_retry
#
#  INPUT:
#       g1 = I/O request ILT at requestor's nest level
#       g2 = requestor's continuation routine address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
lld$io_retry:
        lda     ILTBIAS(g1),r15         # r15 = I/O operation ILT at
                                        #  provider's nest level
        mov     0,r6
        ldl     lld_rtyio_qu,r4         # r4 = first I/O operation on list
                                        # r5 = last I/O operation on list
        st      g2,il_cr(r15)           # save requestor's continuation
                                        #  routine address in ILT
        st      r6,il_fthd(r15)         # clear ILT fthd
        cmpobe.t 0,r5,.ioretry_300      # Jif list is empty
        st      r15,il_fthd(r5)         # link ILT onto end of list
        b       .ioretry_500
#
.ioretry_300:
        mov     r15,r4                  # save ILT as new head member
.ioretry_500:
        mov     r15,r5                  # save new list tail member
        stl     r4,lld_rtyio_qu         # save updated queue pointers
        ret
#
#
.if     spec_INQ_new
#
#******************************************************************************
#
#  NAME: lld$target_to_mode
#
#  PURPOSE:
#       This routine sets up the target side to operate in target-only mode.
#
#  DESCRIPTION:
#       This routine allocates and sets up an LTMT and associates it with
#       the specified IMT to operate the target interface in target-only mode.
#       This is used for IMTs associated with XIOtech initiators associated with
#       an AL-PA that does not have an initiator associated with it.
#       This routine assumes that the specified IMT is a XIOtech controller
#       that supports XIOtech peer communications protocols. It also assumes
#       that the specified IMT is not already associated with an LTMT.
#
#  CALLING SEQUENCE:
#       call    lld$target_to_mode
#
#  INPUT:
#       g4 = assoc. CIMT address
#       g5 = assoc. IMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
lld$target_to_mode:
        movl    g4,r12                  # save g4-g5
                                        # r12 = CIMT address
                                        # r13 = IMT address
c       g4 = get_ltmt();                # allocate an LTMT to use
.ifdef M4_DEBUG_LTMT
c fprintf(stderr, "%s%s:%u get_ltmt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g4);
.endif # M4_DEBUG_LTMT
        st      r12,ltmt_cimt(g4)       # save CIMT address in LTMT
        st      r13,ltmt_imt(g4)        # save IMT address in LTMT
        st      g4,im_ltmt(r13)         # save LTMT address in IMT
        ldconst ltmt_ty_MAG,r4          # r4 = target type code
        ldconst ltmt_lst_id,r5          # r5 = link state code
        ldconst ltmt_dst_null,r6        # r6 = data-link manager state code
        stob    r4,ltmt_type(g4)        # save target type code
        stob    r5,ltmt_lst(g4)         # save link state code
        stob    r6,ltmt_dlmst(g4)       # save data-link manager state code
        lda     ltmt_etbl1a,r6          # r6 = LTMT event handler table
        st      r6,ltmt_ehand(g4)       # save event handler table
        ld      ci_ltmttl(r12),r7       # r7 = LTMT list tail pointer
        cmpobne.t 0,r7,.tomode_120      # Jif list not empty
        st      g4,ci_ltmthd(r12)       # put LTMT as new head pointer
        b       .tomode_130
#
.tomode_120:
        st      g4,ltmt_link(r7)        # link new LTMT onto end of list
.tomode_130:
        st      g4,ci_ltmttl(r12)       # save as new list tail pointer
        movl    r12,g4                  # restore g4-g5
        ret
#
.endif  # spec_INQ_new
#
#******************************************************************************
#
#  NAME: lld$remote_to_mode
#
#  PURPOSE:
#       This routine sets up the initiator side to handle a XIOtech
#       target that is operating in target-only mode.
#
#  DESCRIPTION:
#       This routine will create an IMT to serve as the mate to the
#       specified LTMT and set it up to operate as if the remote
#       node had issued SCSI commands. It checks the inactive IMT
#       queue for an appropriate match and if found will activate
#       the IMT and pair it up with the specified LTMT. If no inactive
#       IMT is found, it will create a new IMT, activate it and
#       pair it up with the specified LTMT.
#       This routine assumes that the specified LTMT does not already
#       have an IMT associated with it. It also assumes that the remote
#       node has indicated that it is operating in target-only mode. It
#       also assumes that there was no matching IMT on the
#       active queue.
#
#  CALLING SEQUENCE:
#       call    lld$remote_to_mode
#
#  INPUT:
#       g4 = assoc. LTMT of target operating in target-only mode
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
lld$remote_to_mode:
        movq    g4,r12                  # save g4-g7
                                        # r12 = LTMT address
        movl    g0,r8                   # save g0-g1
        ld      ltmt_tmt(r12),r10       # r10 = assoc. TMT address
        cmpobe.f 0,r10,.remtomode_1000  # Jif no TMT assoc. with LTMT
        ld      ltmt_cimt(r12),g4       # g4 = assoc. CIMT address
        ldob    ci_num(g4),g0           # g0 = interface # (i.e. channel)
        ld      ispLid[g0*4],g1         # g1 = virtual port ID for initiator
        call    C$findTarget            # find target assigned to primary
                                        #  port on the associated interface
                                        # g0 = target structure address
        cmpobne 0,g0,.remtomode_100     # Jif target defined
#
# --- Target is not defined for the specified port.
#
        ldconst 0xffff,g0               # g0 = target ID to use
        b       .remtomode_200
#
# --- Target is defined for the specified port.
#
.remtomode_100:
        ldos    tartid(g0),g0           # g0 = assigned target ID
.remtomode_200:
        ldos    tm_lid(r10),g5          # g5 = initiator ID
        ldl     tm_P_name(r10),g6       # g6-g7 = port WWN
#
# --- Check the inactive IMT queue for an appropriate match.
#
        ld      C_imt_head,r10          # r10 = first IMT on inactive list
        cmpobe  0,r10,.remtomode_400    # Jif no IMTs on inactive list
#
# --- Check for matching WWN and Target ID
#
        mov     r10,r11                 # get working IMT list reg
.remtomode_250:
        ldl     im_mac(r11),r6          # r6-r7 = WWN address for image
        cmpobne g6,r6,.remtomode_260    # Jif WWN address does NOT match
        cmpobne g7,r7,.remtomode_260    # Jif WWN address does NOT match
        ldos    im_tid(r11),r6          # r6 = Target ID
        cmpobne g0,r6,.remtomode_260    # Jif Target ID does NOT match
#
# --- Found a matching IMT on the inactive queue.
#
        stos    g5,im_fcaddr(r11)       # save initiator ID in IMT
        mov     r11,g5                  # g5 = matching IMT to use from the
                                        #      inactive queue
        cmpobne r10,r11,.remtomode_255  # Jif not the first IMT on queue
        ld      im_link(r11),r10        # unlink from inactive list
        st      r10,C_imt_head          # save new head pointer
        cmpobne 0,r10,.remtomode_500    # Jif not last on list
        st      r10,C_imt_tail          # clear list tail pointer
        b       .remtomode_500
#
.remtomode_255:
        ld      im_link(r11),r5         # r5 = next IMT on inactive queue
        st      r5,im_link(r4)          # remove IMT from inactive list
        cmpobne 0,r5,.remtomode_500     # Jif not the last IMT on the inactive
                                        #  list
        st      r4,C_imt_tail           # save new list tail member
        b       .remtomode_500
#
.remtomode_260:
        mov     r11,r4                  # r4 = previous IMT address
        ld      im_link(r11),r11        # r11 = next IMT on list
        cmpobne 0,r11,.remtomode_250    # Jif more IMTs to check
#
# --- No matching IMT found on the inactive queue.
#
.remtomode_400:
        call    MAGD$create_image       # create an image for the target
                                        # g5 = IMT address if image created
        cmpobe.f 0,g5,.remtomode_1000   # Jif no IMT created
.remtomode_500:
        stos    g1,im_vpid(g5)          # set virtual port ID to the same as
                                        #  the initiator virtual port ID
        st      g5,ltmt_imt(r12)        # save IMT address in LTMT
        st      r12,im_ltmt(g5)         # save LTMT address in IMT
#
# --- Activate new IMT
#
        ld      ci_imthead(g4),r5       # r5 = first IMT on active list
        st      g5,ci_imthead(g4)       # save IMT as new head of list
        st      r5,im_link(g5)          # link onto head of list
        cmpobne.t 0,r5,.remtomode_1000  # Jif list not empty
        st      g5,ci_imttail(g4)       # save new IMT as tail
.remtomode_1000:
        movq    r12,g4                  # restore g4-g7
        movl    r8,g0                   # restore g0-g1
        ret
#
.endif  # MAG2MAG
#
#******************************************************************************
#
#  NAME: lld$findLTMT
#
#  PURPOSE:
#       This routine determine if a IMT has an associated LTMT
#
#  CALLING SEQUENCE:
#       call    lld$findLTMT
#
#  INPUT:
#       g5 = assoc. IMT address
#
#  OUTPUT:
#       g10 = LTMT address (0 = no LTMT association)
#
#  REGS DESTROYED:
#       g10 is destroyed.
#
#******************************************************************************
#
lld$findLTMT:
        ld      im_ltmt(g5),g10         # g10 = assoc. LTMT address
        cmpobne 0,g10,.findLTMTret      # Jif IMT already assoc. with LTMT

        ld      im_cimt(g5),r13         # 13 = CIMT address

        PushRegs(r3)
        ldob    ci_num(r13),g0          # g0 = interface #
        ldos    im_vpid(g5),g1          # g1 = IMT virtual port ID
        call    ISP_IsPrimaryPort
        mov     g0,r5
        PopRegsVoid(r3)
        cmpobne  0,r5,.findLTMT200      # Jif primary port
#
##   ICL port never comes to this place, since it is always primary port -- <TBD>
#
        ldob    ci_num(r13),r4          # r4 = interface #
        ld      iscsimap,r5
        bbs     r4,r5,.findLTMTret
#
        mov     g4,r14                  # save g4
        mov     r13,g4                  # g4 = assoc. CIMT address
        call    lld$target_to_mode      # set up LTMT to operate in
                                        #  target-only mode
        mov     r14,g4                  # restore g4
        ld      im_ltmt(g5),g10         # g10 = assoc. LTMT address
        b       .findLTMTret
#
.findLTMT200:
        ldl     im_mac(g5),r14          # r14-r15 = IMT WWN to check for
        ld      ci_ltmthd(r13),r4       # r4 = first LTMT on list

.findLTMT300:
        cmpobe  0,r4,.findLTMTret       # Jif no LTMTs on list
        ldob    ltmt_type(r4),r5        # r5 = LTMT type code
        cmpobne ltmt_ty_MAG,r5,.findLTMT330 # Jif not a MAGNITUDE type

        ld      ltmt_imt(r4),r5         # r5 = assoc. IMT address
        cmpobne 0,r5,.findLTMT330       # Jif an IMT assoc. with LTMT

        ld      ltmt_tmt(r4),r5         # r5 = assoc. TMT address
        cmpobe  0,r5,.findLTMT330       # Jif no TMT assoc. with LTMT

        ldl     tm_P_name(r5),r6        # r6-r7 = MAC address of target
        cmpobne r6,r14,.findLTMT330     # Jif MAC address does not match
        cmpobne r7,r15,.findLTMT330     # Jif MAC address does not match
#
# --- A target match is found. Associate LTMT and IMT with each other.
#
        st      g5,ltmt_imt(r4)         # save IMT address in LTMT
        st      r4,im_ltmt(g5)          # save LTMT address in IMT
        mov     r4,g10                  # g10 = LTMT address
        b       .findLTMTret            # and finish initializing IMT

.findLTMT330:
        ld      ltmt_link(r4),r4        # r4 = next LTMT on list
        b       .findLTMT300            # and check next LTMT if defined

.findLTMTret:
        ret
#
#******************************************************************************
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
