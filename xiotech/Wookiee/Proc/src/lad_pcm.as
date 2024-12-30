# $Id: lad_pcm.as 162753 2014-02-20 21:22:48Z marshall_midden $
#**********************************************************************
#
#  NAME: pcm.as
#
#  PURPOSE:
#       To provide support for the Secondary Copy Manager logic which
#       supports MAGNITUDE-to-MAGNITUDE functions and services.
#
#  FUNCTIONS:
#       CM$init        - Copy Manager initialization
#       cm$pexeq       - Queue SCR completions
#       CM$ctlrqstq    - Queue control request
#
#       This module employs 3 process:
#
#       CM$pexec       - copy/mirror primary process
#       cm$Vsync       - V$exec synchronization process
#       cm$Retry_RCC   - Retry Change configuration
#
#  Copyright (c) 1999-2008 XIOtech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- global function declarations ------------------------------------
#
        .globl  CM$ctlrqstq             # Enqueue control requests
        .globl  CM$pexec                # Executive for CM process
        .globl  CM$Build_RCC

        .globl  CM_pexec                # Executive for CM process

        .globl CM$PauseCopy
        .globl CM$ResumeCopy
        .globl CM$SwapRaids
        .globl CM$BreakCopy
        .globl CM$OwnershipAcquired
        .globl CM$StartTask
#
# --- assembly options ------------------------------------------------
#
        .set    DEBUG,TRUE
#
# --- Local variables and constants
#
        .set    MAX_APOOL_DATA,30000
#
        .data
cur_ap_data:
        .word   0
#
max_ap_data:
        .word   MAX_APOOL_DATA
#
max_usr_ap_data:
        .word   MAX_APOOL_DATA
#
        .text
#**********************************************************************
#
#  NAME: cm$pexep
#
#  PURPOSE:
#       To provide a common method of queuing Segment Copy Requests
#       completions to the primary copy task.
#
#  DESCRIPTION:
#       The SCR completions are queued to the cmpltq located in the CM.
#
#  CALLING SEQUENCE:
#       call    cm$pexeq
#
#  INPUT:
#        g1 = SCR
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
cm$pexeq:
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>ladpcm-cm$pexecq... putting SCR=%lx in completer queue(cmpltq)..cm$pexec process it\n", FEBEMESSAGE, __FILE__, __LINE__,g1);
.endif  # CM_IM_DEBUG
        ld      scr1_cm(g1),r4          # r4 = CM address
        lda     cm_cmpltq(r4),r11       # queue origin
        b       K$cque
#
#**********************************************************************
#
#  NAME: CM$ctlrqstq
#
#  PURPOSE:
#       To provide a common means of queuing a Process Control
#       Packet to the proper queue for processing by the
#       primary copy process
#
#  DESCRIPTION:
#       The PCP's are queued to the ctlrqstq or uderr queues located
#       in the CM.
#
#  CALLING SEQUENCE:
#       call    CM$ctlrqstq
#
#  INPUT:
#        g1 = PCP at lvl 2
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
CM$ctlrqstq:
        ldob    pcp2_function(g1),r4    # r4 = control request function
        ld      -ILTBIAS+pcp1_cm(g1),r5 # r5 = CM address
#
        lda     cm_ctlrqstq(r5),r11     # control request queue origin
        cmpobne  pcp1fc_updateerr,r4,.ctlrqstq_10
#
        lda     cm_uderrq(r5),r11       # update error queue origin
#
.ctlrqstq_10:
        b       K$cque
#
#**********************************************************************
#
#  NAME: CM$pexec
#        CM_pexec
#
#  PURPOSE:
#       To provide a means of processing SCR requests which have been
#       previously queued to this module.
#
#  DESCRIPTION:
#       The queuing routine CM$que deposits a SCR request into the queue
#       and activates this executive if necessary.  This executive
#       extracts the next SCR request from the queue and initiates that
#       request by queuing VRP requests to the VIRTUAL module.
#
#       A separate completion routine handles the completion of VRP
#       requests.
#
#  CALLING SEQUENCE:
#       process call
#
#  INPUT:
#       ASM -> g4 = cm address
#       C   -> g2 = cm address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
# --- This is the C entry point
#
CM_pexec:
        mov     g2,g4                   # place in correct register
#
# --- Do some initialization to this process
#     set up SCR completion queue and task control queue
#
CM$pexec:
        ld      cm_cor(g4),g3           # g3 = assoc. COR address
        ld      cm_pcb(g4),r15          # r15 = task pcb address
        st      r15,cm_cmpltq+qu_pcb(g4)# set up the SCR completion queue
        st      r15,cm_uderrq+qu_pcb(g4)# set up update error queue
        st      r15,cm_ctlrqstq+qu_pcb(g4)# set up request control queue
#
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>ladpcm-cm$pexec..calling CCSM$CCTrans with TaskStarted-Event...\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # CM_IM_DEBUG
        ldconst TaskStarted,g0
        call    CCSM$CCTrans            # process the event
#
# --- Set this process to not ready
#
.pex_Sleep:
        ldconst pcnrdy,r4               # Set this process to not ready
        stob    r4,pc_stat(r15)
#
# --- give up control
#
.pex_Exchange:
        mov     0,r6
        call    K$xchang                # Exchange processes
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>ladpcm-cm$pexec--.pex_Exchange---Ready to take req from the queue...\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # CM_IM_DEBUG
#
# --- Determine if there are any SCR's on the completion queue
#
        ldconst 0,r4                    # r4 = SCR completion
        lda     cm_cmpltq(g4),r11       # get address of the SCR completion queue
#
        ldq     qu_head(r11),r12        # Get queue head, tail, count and PCB
        cmpobne.t 0,r12,.pex_Work2Do    # Jif there is a completion
#
# --- Determine if there update error on the queue
#
.pex_Continue:
        ldconst 1,r4                    # r4 = update error
        lda     cm_uderrq(g4),r11       # get addr of update error queue
#
        ldq     qu_head(r11),r12        # Get queue head, tail, count
                                        #  and PCB
        cmpobne.t 0,r12,.pex_Work2Do    # Jif there is an update error
#
# --- determine if a control request is currently pending
#
        ld      cm_pctlrqst(g4),r4      # is there a pending control request
        ldconst SUPER_MAGIC_FLAG,r5
        cmpobne r5,r6,.pex_001
        cmpobne 0,r4,.pex_Exchange      # Jif yes and exchange
.pex_001:
        cmpobne 0,r4,.pex_Sleep         # Jif yes and sleep
#
# --- Determine if there control request on the queue
#
        ldconst 2,r4                    # r4 = task control
        lda     cm_ctlrqstq(g4),r11     # get addr of control request queue

        ldq     qu_head(r11),r12        # Get queue head, tail, count
                                        #  and PCB
        cmpobne r5,r6,.pex_002
        cmpobe.t 0,r12,.pex_Exchange    # Jif empty and sleep
.pex_002:
        cmpobe.t 0,r12,.pex_Sleep       # Jif empty and sleep
#
# --- Remove this request from queue ----------------------------------
#
.pex_Work2Do:
        mov     r12,g1                  # Isolate next queued ILT

        ld      il_fthd(r12),r12        # Dequeue this ILT
        cmpo    0,r12                   # Check for queue now empty
        subo    1,r14,r14               # Adjust queue count
        sele    r13,r12,r13             # Set up queue tail
        stt     r12,qu_head(r11)        # Update queue head, tail and count
        be.f    .pex_Work2Do_20         # Jif queue now empty

        st      r11,il_bthd(r12)        # Update backward thread
#
# --- enable interrupts and determine type of processing required
#
.pex_Work2Do_20:
#
# --- determine if a process control request was received
#
        cmpobe.t 0,r4,.pex_SCRcmplt     # Jif process control request
        cmpobe.t 1,r4,.pex_UpdateError  # Jif process control request

#********************************************************************
#
#     Control Request
#
        st      g1,cm_pctlrqst(g4)      # set control request pending
        ldconst pcp1st_ok,r4            # default current pcp status to OK
        stob    r4,pcp2_status(g1)
        ld      cm_cor(g4),g3           # g3 = cor address
        ldob    pcp2_function(g1),r6    # get requested function
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>ladpcm-cm$pexec_ladpcm-Requested function=%lx\n", FEBEMESSAGE, __FILE__, __LINE__,r6);
.endif  # CM_IM_DEBUG
        cmpobne  pcp1fc_poll,r6,.pex_CtlRqst_ev # Jif not a poll request
#
# --- the pcp is a poll request, process as an exec loop extention
#
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>ladpcm-cm$pexec_ladpcm-calling cm$ProcessPoll\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # CM_IM_DEBUG
        call    cm$ProcessPoll          # process poll
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>ladpcm-cm$pexec_ladpcm-returned from cm$ProcessPoll..\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # CM_IM_DEBUG
        bx      (g0)                    # branch to handler
#
# --- the PCP is not a poll request, process though the state table
#
.pex_CtlRqst_ev:
        ldob    cm$ctlrqsttbl(r6),g0    # g0 = CCTable event
        cmpobe  0,g0,.pex_CtlRqst_rtnPCP # JIf no event defined
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>ladpcm-cm$pexec-call CCSM$CCTrans with event-g0= %lx\n", FEBEMESSAGE, __FILE__, __LINE__,g0);
.endif  # CM_IM_DEBUG
        call    CCSM$CCTrans            # Process event
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>ladpcm-cm$pexec-lad_pcm ret from CSM$CCTrans\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # CM_IM_DEBUG
#
# --- return the PCP
#
.pex_CtlRqst_rtnPCP:
        ldob    pcp2_status(g1),r4      # get lvl2 status
        lda     -ILTBIAS(g1),g1         # pull back to PCP1 level
        mov     0,r3                    # clear r3
        ldob    pcp1_rtstate(g1),r5     # r5 = task mode
        stob    r4,pcp1_status(g1)      # save completion status
#
# --- determine if there is a request to alter the state of the task
#
        cmpobe.t 0,r5,.pex_CtlRqst_rtn  # JIf no special task state requested
        stob    r5,pc_stat(r15)         # set task to requested state

.pex_CtlRqst_rtn:
        ld      pcp1_cr(g1),r6          # r6 = completion handler
.if GR_GEORAID15_DEBUG
c fprintf(stderr,"%s%s:%u <GR><cm$pexec-lad_pcm> calling r6 = %x\n", FEBEMESSAGE, __FILE__, __LINE__, (UINT32)r6);
.endif  # GR_GEORAID15_DEBUG
        callx   (r6)                    # call completion routine
#
# If we are terminating, do not do anything else, as CM structure (g4) is gone.
#
        cmpobe  pcp1st_taskend,r4,.pex_EndTask # If cm task termination status
#
        st      r3,cm_pctlrqst(g4)      # clear pending control request
#
# --- If the event that was just executed is Instant Mirror' jump to instant mirror section
#     (This copy treated as completed and hence perform directly copy-mirror final steps)
#
        cmpobe  InstantMirror,g0,.pex_CpyComplt_InstantMirror
#
        b       .pex_Exchange

#********************************************************************
#
#     Update Error Handler
#
#           Note: This logic is only setup to handle 32 bit SDA's.
#                 If 64 bit SDA's are required, changes will be
#                 required.
#
#     return the pcp
#
.pex_UpdateError:
        mov     g1,r15                  # save pcp address
                                        #   g1 = pcp lvl2 address
        call    cm$ProcessUpdErr        # process the update error
                                        #   g0 = config update flag
#
# --- determine the state of the copy/mirror
#
        ldob    cm_cstate(g4),r8        # r8 = copy state
        bbs.f   cmcst_b_pause,r8,.pex_uperr_rtnPCP # Jif already paused
        bbs.f   cmcst_b_stop,r8,.pex_uperr_rtnPCP  # Jif stop pending
        bbc.f   cmcst_b_copy,r8,.pex_uperr_200 # Jif copy not active

        setbit  cmcst_b_auto,r8,r8      # request auto suspend
        setbit  cmcst_b_stop,r8,r8      # set stop copy
        stob    r8,cm_cstate(g4)        # save new state
        b       .pex_uperr_rtnPCP
#
# --- a copy is not active, suspend the copy
#
.pex_uperr_200:
        call    cm$SuspendCopy_auto     # auto suspend the copy
#
# --- Return the PCP
#
.pex_uperr_rtnPCP:
        mov     r15,g1                  # restore pcp address
        ldconst pcp1st_ok,r4            # set current pcp status to OK
        stob    r4,pcp2_status(g1)      # save current PCP status
        lda     -ILTBIAS(g1),g1         # pull back to PCP1 level
        ld      pcp1_cr(g1),r6          # r6 = completion handler
        stob    r4,pcp1_status(g1)      # save completion status
        callx   (r6)                    # call completion routine
        b       .pex_Exchange

#********************************************************************
#
#     SCR completion
#
# --- determine the completion status of the SCR
#
.pex_SCRcmplt:
        ld      scr1_cm(g1),g4          # g4 = CM address
        ldob    scr1_status(g1),r4      # r4 = completion status
        ld      cm_cor(g4),g3           # g3 = cor address

#
# --- determine if there is an SCR error
#
         cmpobne 0,r4,.pex_SCRerr       # Jif error scr error
#
# --- completion status was good. determine if copy is suspended
#
        ldob    cm_cstate(g4),r8        # r8 = copy state
        bbs.f cmcst_b_copy,r8,.pex_SCRcmplt_100 # Jif data copy is active

#********************************************************************
#
#      data copy is not active
#
# --- recalculate the remaining segment count.
#
        call    cm$CntRemSegs           # count remaining segments
        b       .pex_SCRcmplt_300       # continue

#********************************************************************
#
#     data copy is active
#
# --- decrement remaining segment count
#
.pex_SCRcmplt_100:

#
# --- Do not update segment counts if this SCR was resubmitted by apool logic
#
        ldob    scr1_flags(g1),r5
        bbc     scr1flg_b_PASS,r5,.pex_SCRcmplt_105
        clrbit  scr1flg_b_PASS,r5,r5
        stob    r5,scr1_flags(g1)       # clear the async flag bit
        b       .pex_SCRcmplt_300

.pex_SCRcmplt_105:
        ld      cm_remsegs(g4),r5       # decrement remaining segments
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>ladpcm-cm$pexec-SCRcmplt_100--remaining segments=%ld\n", FEBEMESSAGE, __FILE__, __LINE__,r5);
.endif  # CM_IM_DEBUG
        cmpobe  0,r5,.pex_SCRcmplt_300  # Jif already zero

c       r5--;
        st      r5,cm_remsegs(g4)       # save remaining segment count
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>ladpcm-cm$pexec-SCRcmplt_100--latest remaining segments=%ld\n", FEBEMESSAGE, __FILE__, __LINE__,r5);
.endif  # CM_IM_DEBUG

#********************************************************************
#
#     SCR completion common processing
#
#
# --- calculate percent complete and save in VDD
#
.pex_SCRcmplt_300:
        ldconst FALSE,g0                # normal processing
        call    cm$PctCmplt             # calculate percent complete
#
# --- Determine if data copy is currently active. If so, bypass CM, VI,
#     and COR setup.
#
        bbs.f cmcst_b_copy,r8,.pex_SCRcmplt_400 # Jif copy  state is active
#
# --- determine if copy was suspended. Is so, issue a resume message
#
#        bbc.f cmcst_b_pause,r8,.pex_SCRcmplt_305 # Jif copy NOT paused
#
#        call    CCSM$resume             # distribute copy resume
#
# --- CM copy state not active. set CM to copy state.
#
# .pex_SCRcmplt_305:
        mov     r8,r9                   # make a copy of current copy state
        clrbit  cmcst_b_mirrored,r8,r8  # clear mirrored bit
        clrbit  cmcst_b_pause,r8,r8     # clear possible pause state
        clrbit  cmcst_b_auto,r8,r8      # clear possible auto state
        setbit  cmcst_b_copy,r8,r8      # set copy state
        stob    r8,cm_cstate(g4)        # save new state
#
# --- set copy state in vdd
#     Note: vdcopyto state is now set in the copy update routines in definebe.as
#           lad
#
#        ld      cor_destvdd(g3),r4      # r4 = destination VDD address
#        cmpobe.f 0,r4,.pex_SCRcmplt_310 # Jif no dest. VDD address defined
#
#        ldconst vdcopyto,r5             # r5 = new mirror state
#        stob    r5,vd_mirror(r4)        # save new VDD mirror state
# c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
#
# --- set up the scd update handlers to start updates to remote mirror
#
# .pex_SCRcmplt_310:
        ld      cor_scd(g3),r5          # r5 = assoc. SCD address
        lda     CM_wp2_copy,r4          # r4 = handler when actively copying
                                        #      data
        cmpobe.f 0,r5,.pex_SCRcmplt_320 # Jif no SCD assoc. with COR

        st      r4,scd_p2hand(r5)       # save new scd phase 2 update handler
#
# --- set up the dcd update handlers to null
#
.pex_SCRcmplt_320:
        ld      cor_dcd(g3),r5          # r5 = assoc. DCD address
        lda     CM$wp2_null,r4          # r4 = dcd handler to null
        cmpobe.f 0,r5,.pex_SCRcmplt_350 # Jif no DCD assoc. with COR

        st      r4,dcd_p2hand(r5)       # save new dcd phase 2 update handler
#
# --- setup the COR states
#
.pex_SCRcmplt_350:
        ldconst corcrst_active,r4       # r4 = registration active
        ldconst corcst_copy,r5          # r5 = set cstate to copy
        ldconst cormst_term,r6          # r6 = region map termination
        stob    r4,cor_crstate(g3)      # save new registration state
        stob    r5,cor_cstate(g3)       # save new copy state
        stob    r6,cor_mstate(g3)       # save new map state
        call    CM$mmc_sflag            # update suspended flag for MMC
#
# --- generate copy resumed into event table
#
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>ladpcm-cm$pexec_SCRcmplt_350--call CCSM$CCtrans with Copy_Resumed (CopyResumed)event\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # CM_IM_DEBUG
        ldconst CopyResumed,g0
        call    CCSM$CCTrans            # Process event
#
# --- Force a local poll to update all copy elements
#
        call    cm$Reg_Resume           # register the resume
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>ladpcm--cm$pexec_SCRcmplt_350--call CCSM$resume\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # CM_IM_DEBUG
        call    CCSM$resume             # distribute copy resume to ccsm
#
# --- Log Copy Resumed Message to CCB
#
        ldconst cmcc_CpyRsm,g0          # set copy resumed message
.if GR_GEORAID15_DEBUG
c fprintf(stderr, "%s%s:%u <GR><p$exec-lad_pcm.as>Sending copy resumed message to CCB\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # GR_GEORAID15_DEBUG
        call    CM_Log_Completion       # send log message

#
# --- determine if the copy is to be stopped/suspended
#
.pex_SCRcmplt_400:
        bbs     cmcst_b_stop,r8,.pex_PendStop # Jif copy stop pending

#*********************************************************************
#
# --- copy is to continue, determine if this controller still owns the
#     required resources
#
#     First determine if we own the required resources
#
.pex_NextSegment:
        call    cm$find_segment_bit      # find the next segment bit
        cmpobe  FALSE,g2,.pex_CpyComplt  # Jif no segment found (synchronized)

# If the destination is an alink then make sure there is enough room in the
# apool for more copy data.
        ld      cm_cor(g4),r12          # Get the COR
        cmpobe  0,r12,.pex_SkipWait
        ld      cor_destvdd(r12),r11    # Get the dest vdd
        cmpobe  0,r11,.pex_SkipWait
        ldos    vd_attr(r11),r12        # Get the attribute
        bbc     vdbasync,r12,.pex_SkipWait  # Jif not Async
        ld      cur_ap_data,r11
        ld      max_ap_data,r12
        cmpobl  r11,r12,.pex_SkipWait
        ldob    scr1_flags(g1),r11
        setbit  scr1flg_b_PASS,r11,r11
        stob    r11,scr1_flags(g1)
        mov     g0,r11
        ldconst 1000,g0
        call    K$twait                 # delay one second
        mov     r11,g0
        call    cm$pexeq                # enqueue request to completion queue
        ldconst SUPER_MAGIC_FLAG,r6
        b       .pex_Continue           # go handle the other queues

.pex_SkipWait:
#
# --- Make sure the mirrored flag is clear
#
        ldob    cm_cstate(g4),r8        # load copy state
        clrbit  cmcst_b_mirrored,r8,r8  # clear mirrored bit
        stob    r8,cm_cstate(g4)        # save new state
#
# --- setup SCR for next segment
#
        ldconst 0,r4                    # clear r4
        st      g0,scr1_segnum(g1)      # save new segment number
        st      r4,scr1_status(g1)      # clear status, done, and abort
                                        # flags
        st      r4,scr2_status+ILTBIAS(g1) # clear scr2 status
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>ladpcm-cm$pexec-Call CM$que.to queue the SCR(for next segment) req which is processed by cm$exec process\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # CM_IM_DEBUG
        call    CM$que                  # enqueue request
        b       .pex_Exchange           # exchange the task

#*********************************************************************
#
# --- Stop copy is pending
#     release copy SCR
#
.pex_PendStop:
        call    cm$put_scr              # return the SCR(ILT)
#
# --- determine if a abort is pending. If so, end copy/mirror
#
        bbc.f   cmcst_b_abort,r8,.pex_PendStop_100 # Jif abort set

        clrbit  cmcst_b_abort,r8,r8     # clear abort state
        stob    r8,cm_cstate(g4)        # save new state
#
# --- Log Mirror/Copy Break Message to CCB
#
        ldconst cmcc_MirrorEnd,g0       # set Mirror/Copy break message
        call    CM_Log_Completion       # send log message


        call    CM$scbreak              # initiate break-off sequence
        b       .pex_EndTask            # end the task
#
# --- copy was not aborted, determine if copy is already paused
#
.pex_PendStop_100:
        bbs.f cmcst_b_pause,r8,.pex_PendStop_250 # Jif copy already paused
#
# --- clear copy and set paused state
#
        clrbit  cmcst_b_stop,r8,r8      # clear stop state
        stob    r8,cm_cstate(g4)        # save new state
#
# --- suspend the copy (either user or auto depending on auto state)
#
        bbs.f   cmcst_b_auto,r8,.pex_PendStop_200 # Jif auto is set
#
# --- user suspend copy
#
        call    cm$SuspendCopy_user     # user suspend the copy
        b       .pex_PendStop_250       # continue
#
# --- set auto suspend auto suspend copy
#
.pex_PendStop_200:
        bbs.f   cmcst_b_NRS,r8,.pex_PendStop_220 # Jif copy currently does not have
                                                 # resources

        call    cm$SuspendCopy_auto     # auto suspend the copy
        b       .pex_PendStop_250
#
# --- Resources are not owned, wait for all updates to complete
#
.pex_PendStop_220:

        ldconst cmcst_NRS,r8            # clear all but NRS
        stob    r8,cm_cstate(g4)        # save new state

        ldos    cor_uops(g3),r4         # check if any outstanding updates
        cmpobe.f 0,r4,.pex_PendStop_240 # Jif if outstanding updates

        ldconst 10,g0                   # wait 10 ms and try again
        call    K$twait
        b       .pex_PendStop_220       # continue
#
# --- All outstanding updates have been applied. Flush the control
#     queue.
#
.pex_PendStop_240:

        call    cm$FlushCtlQue          # flush the control queue
#
# --- process any outstanding update errors
#
        call    cm$ProcessUpdErrQue     # process update error queue
#
# --- Send a event back to the cctable
#
        ldconst OwnerSuspended,g0       # set "Ownership Suspended" event
        call    CCSM$CCTrans            # process the event
#
# --- Force a local poll to update all copy elements
#
.pex_PendStop_250:
        call    cm$force_LP             # force a local poll
        b       .pex_Exchange           # continue

#*********************************************************************
#
# --- SCR error has occurred
#
#     Determine if stop state is set. If so, process as
#     pending stop.
#
.pex_SCRerr:
        ldob    cm_cstate(g4),r8        # r8 = copy state
        bbs.f cmcst_b_stop,r8,.pex_PendStop # Jif stop is pending
#
# --- Stop state was not set. release copy SCR...
#
        call    cm$put_scr              # return the SCR(ILT)
#
# --- Determine if the copy is already paused. If not, clear
#     the copy state, set the auto suspend, and set pause
#     state.
#
        bbs.f cmcst_b_pause,r8,.pex_SCRerr_100 # Jif already paused

        mov     r8,r9                   # r9 = current copy state
        clrbit  cmcst_b_copy,r8,r8      # clear copy state
        stob    r8,cm_cstate(g4)        # save new state
#
# --- suspend the copy
#
        call    cm$SuspendCopy_auto     # auto suspend the copy
#
# --- determine if data copy is currently active. if so, save the current
#     auto suspend and tell everyone else (if possible).
#
        bbc.f cmcst_b_copy,r9,.pex_SCRerr_100 # Jif copy state not active
#
# --- force a local poll
#
        call    cm$force_LP             # force a local poll
#
# --- determine if a abort is pending. If so, end copy/mirror
#
.pex_SCRerr_100:
        bbc.f   cmcst_b_abort,r8,.pex_Exchange # Jif abort set

        clrbit  cmcst_b_abort,r8,r8     # clear abort state
        stob    r8,cm_cstate(g4)        # save new state
#
# --- Log Mirror/Copy Break Message to CCB
#
        ldconst cmcc_AutoTerm,g0        # set Mirror/Copy break message
        call    CM_Log_Completion       # send log message


        call    CM$scbreak              # initiate break-off sequence
        b       .pex_EndTask            # end the task

#*********************************************************************
#
# --- copy has just become synchronized
#     release copy resources
#       release the SCR(ILT)
#
.pex_CpyComplt:
        call    cm$put_scr              # return the SCR(ILT)
.pex_CpyComplt_InstantMirror:
.if CM_IM_DEBUG
        ldob  cor_cstate(g3),r4
        ldob  cor_crstate(g3),r8
        ldob  cor_mstate(g3),g0
c fprintf(stderr,"%s%s:%u <CM_IM>ladpcm-cm$pexec-pex_CpyComplt.cor-cstate=%lx crstate=%lx mstate=%lx..\n", FEBEMESSAGE, __FILE__, __LINE__,r4,r8,g0);
        ld    cor_ccseptr(g3),r4
        ld    cor_ocseptr(g3),r8
c fprintf(stderr,"%s%s:%u <CM_IM>cm$pexec>ladpcm-pex_CpyComplt.cor-ocseptr=%lx ccseptr=%lx\n", FEBEMESSAGE, __FILE__, __LINE__,r8,r4);
        ldob  cm_cstate(g4),r4
        ldob  cm_compstat(g4),r8
c fprintf(stderr,"%s%s:%u <CM_IM>cm$pexec>ladpcm-pex_CpyComplt.cm-cstate=%lx compstate=%lx\n", FEBEMESSAGE, __FILE__, __LINE__,r4,r8);
        ldob  cor_ccsecev(g3),r4
        ldob  cor_ccselev(g3),r8
c fprintf(stderr,"%s%s:%u <CM_IM>cm$pexec>ladpcm-pex_CpyComplt.cm-ccse current event=%lx lastevent=%lx\n", FEBEMESSAGE, __FILE__, __LINE__,r4,r8);
        ldob  cor_ccsecst(g3),r4
        ldob  cor_ccselst(g3),r8
c fprintf(stderr,"%s%s:%u <CM_IM>cm$pexec>ladpcm-pex_CpyComplt.cm-ccse current state=%lx last state=%lx\n", FEBEMESSAGE, __FILE__, __LINE__,r4,r8);
.endif  # CM_IM_DEBUG
#
#       return the RM
#
        ld      cor_rmaptbl(g3),g0      # g0 = possible RM
        cmpobe.f 0,g0,.pex_CpyComplt_10 # Jif none

        ldconst 0,r4                    # r4 = 0
.ifdef M4_DEBUG_RM
c fprintf(stderr, "%s%s:%u put_rm 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_RM
c       put_rm(g0);                     # Release region map table (RM)
        st      r4,cor_rmaptbl(g3)      # clear RM pointer in COR

#
# --- determine if copy started in a mirror state. If so, issues a copy
#     resumed to state machine.
#
.pex_CpyComplt_10:
        ldob    cm_cstate(g4),r8        # get current state
.if GR_GEORAID15_DEBUG
c fprintf(stderr,"%s%s:%u <GR><cm$pexec>..Current CM cstate = %lx.. COR_rmaptbl is null\n", FEBEMESSAGE, __FILE__, __LINE__, r8);
.endif  # GR_GEORAID15_DEBUG
        bbs.f   cmcst_b_copy,r8,.pex_CpyComplt_15 # Jif copy flag set

#
# --- generate copy resumed into event table
#
.if GR_GEORAID15_DEBUG
c fprintf(stderr,"%s%s:%u <GR><cm$pexec>Sending Copy_Resumed(CopyResumed)event to CCSM$CCTrans\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # GR_GEORAID15_DEBUG
        ldconst CopyResumed,g0
        call    CCSM$CCTrans            # Process event
#
# --- set state to mirrored and clear other stuff
#
.pex_CpyComplt_15:
        setbit  cmcst_b_mirrored,r8,r8  # set mirrored bit
        clrbit  cmcst_b_copy,r8,r8      # clear copy bit
        clrbit  cmcst_b_auto,r8,r8      # clear auto state
        clrbit  cmcst_b_pause,r8,r8     # clear paused state
        clrbit  cmcst_b_stop,r8,r8      # clear stop state
        stob    r8,cm_cstate(g4)        # save new state
#
# --- setup the COR states
#
        ldconst corcrst_active,r4       # r4 = registration active
        ldconst corcst_mirror,r5        # r5 = new cor_cstate value
        ldconst cormst_term,r6          # r6 = region map termination
        stob    r4,cor_crstate(g3)      # save new registration state
        stob    r5,cor_cstate(g3)       # save new cor_cstate value
        stob    r6,cor_mstate(g3)       # save new map state
        call    CM$mmc_sflag            # update suspended flag for MMC
#
# --- set up new SCD update handler
#
        ld      cor_scd(g3),r10         # r10 = scd address
        lda     CM_wp2_mirror,r7        # get new update handler
        cmpobe.f 0,r10,.pex_CpyComplt_30 # Jif no SCD defined
        st      r7,scd_p2hand(r10)      # save new scd p2 handler
.if GR_GEORAID15_DEBUG
c fprintf(stderr,"%s%s:%u <GR><p$exec>p2hand=wp2mirror.CORcrstate = %lx CORcstate=%lx CMcstate=%lx1\n", FEBEMESSAGE, __FILE__, __LINE__,r4,r5,r8);
.endif  # GR_GEORAID15_DEBUG
#
# --- set up new DCD update handler
#
.pex_CpyComplt_30:
        ld      cor_dcd(g3),r10         # r10 = dcd address
        lda     CM$wp2_null,r7          # get new update handler
        cmpobe.f 0,r10,.pex_CpyComplt_40 # Jif no DCD defined
        st      r7,dcd_p2hand(r10)      # save new dcd p2 handler
#
# --- correct the device capacity
#
.pex_CpyComplt_40:
        ld      cor_srcvdd(g3),r10      # r10 = source VDD
        cmpobe.f 0,r10,.pex_CpyComplt_50 # Jif no srcvdd defined

        ld      cor_destvdd(g3),r11     # r11 = destination VDD
        cmpobe.f 0,r11,.pex_CpyComplt_50 # Jif no destvdd defined

        ldl     vd_devcap(r10),r4       # r4/r5 = source VDD device capacity
        ldl     vd_devcap(r11),r6       # r6/r7 = dest. VDD device capacity
        cmpobne.f r4,r6,.pex_CpyComplt_45 # Jif MSW is different
        cmpobe.f  r5,r7,.pex_CpyComplt_49 # Jif capacities the same

.pex_CpyComplt_45:
        stl     r4,vd_devcap(r11)       # save source cap. in dest. VDD
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
#
.if     MAG2MAG
#
        mov     g4,r8                   # save g4
        mov     r11,g4                  # g4 = dest. VDD address
        call    DLM$chg_size            # change peer VDisk size if necessary
        mov     r8,g4                   # restore g4
#
.endif  # MAG2MAG

.pex_CpyComplt_49:
# Here we can inform the autoswap handler to initiate swapback.
# in the GR_InformVdiskSyncState.
        PushRegs(r8)
#
# --- If AllDevMiss flag is set on destination VDD, it indicates a special flag has been
#     set when all BE access and DCN lost at other site when this destination VDD is in
#     sync with its source. In this cases it is not necessary to check for autoswapback
#     operations, instead reset the special flags set on source and destination VDDs.
#
c      if (GR_IsAllDevMissSyncFlagSet((VDD*)r11) == TRUE) {
c fprintf(stderr, "%s%s:%u <GR>svid(%2x %2x),dvid(%2x %2x) in SYNC-Reset OPSTATE<<>>", FEBEMESSAGE, __FILE__, __LINE__, ((VDD*)(r10))->vid, ((VDD*)(r10))->grInfo.vdOpState, ((VDD*)(r11))->vid, ((VDD*)(r11))->grInfo.vdOpState);
c          GR_ClearAllDevMissSyncFlag((VDD *)r11);
c          GR_ResetIOSuspendState((VDD *)r10);
c fprintf(stderr, "%s%s:%u svid(%2x %2x),dvid(%2x %2x) COR (%2x %2x)-After Reset\n", FEBEMESSAGE, __FILE__, __LINE__, ((VDD*)(r10))->vid, ((VDD*)(r10))->grInfo.vdOpState, ((VDD*)(r11))->vid, ((VDD*)(r11))->grInfo.vdOpState, ((COR*)g3)->crstate, ((COR*)g3)->copystate);
c      } else {
#
# ---Inform to the Vdisk error Handler module
#
c        GR_InformVdiskSyncState((VDD*)r10, (VDD*)r11);
c      }
        PopRegsVoid(r8)

#
# --- Indicate copy completion to the CCSM task.
#

.pex_CpyComplt_50:

        ldconst TRUE,g0                 # Force an percent update
        call    cm$PctCmplt             # calculate percent complete

        call    CCSM$mirror             # distribute mirrored status
#
# --- Log Copy Mirrored Message to CCB
#
        ldconst cmcc_CpyMirror,g0       # set copy mirrored message
.if GR_GEORAID15_DEBUG
c fprintf(stderr, "%s%s:%u <GR>lad_pcm.as - pex_SCRcmplt_50 - Sending copy mirrored message to CCB\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # GR_GEORAID15_DEBUG
        call    CM_Log_Completion       # send log message
#
# --- register changes to copy components
#
        call    cm$Reg_Change           # register changes to COR at
                                        #   other copy components
        b       .pex_Exchange           # check for more work
#
# --- copy is no longer active, all resources associated with
#     this task have been deallocated. kill copy task
#
.pex_EndTask:
        ret                             # end process

#***********************************************************************
#***********************************************************************
#***********************************************************************
#
#       R E Q U E S T   C O N T R O L   H A N D L E R S
#
#***********************************************************************
#***********************************************************************
#***********************************************************************

#***********************************************************************
#  NAME: cm$pctbl
#
#  PURPOSE:
#       This table is used to translate PCP function codes to CCTable
#       events.
#
#***********************************************************************

        .data
cm$ctlrqsttbl:
        .byte   0                       # 00 #
        .byte   0                       # 01 #
        .byte   0                       # 02 #
        .byte   0                       # 03 #
        .byte   SwapRaids               # 04 # Swap mirror
        .byte   BreakCopy               # 05 # user break mirror entry
        .byte   PauseCopy               # 06 # pause copy/mirror
        .byte   ResumeCopy              # 07 # resume copy/mirror
        .byte   BreakCopy               # 08 # user Abort copy/mirror entry
        .byte   0                       # 09 #
        .byte   SourceError             # 0a # Source copy device err event, related to georaid
        .byte   0                       # 0b #
        .byte   BreakCopy               # 0c # auto break mirror entry
        .byte   BreakCopy               # 0d # auto Abort copy/mirror entry
        .byte   0                       # 0e #
        .byte   0                       # 0f #
        .byte   OwnerAcquired           # 10 # Ownership Acquired
        .byte   SuspendOwner            # 11 # Suspend Ownership
        .byte   InstantMirror           # 12 # Instant Mirror
        .byte   0                       # 13 #
        .byte   0                       # 14 #
        .byte   0                       # 15 #
        .byte   0                       # 16 #
        .byte   0                       # 17 #
        .byte   0                       # 18 #
#
        .text
#
#**********************************************************************
#
#  NAME: cm$SuspendOwnership
#
#  PURPOSE:
#       To provide a means of suspending the ownership of a secondary
#       copy task.
#
#  CALLING SEQUENCE:
#       call    cm$SuspendOwnership
#
#  INPUT:
#       g3 = COR address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None:
#
#**********************************************************************
#
cm$SuspendOwnership:
        mov     g0,r14                  # save g0
        mov     g4,r15                  # save g4
        ld      cor_cm(g3),g4           # g4 = assoc. CM address
        cmpobe  0,g4,.so_end            # Jif no CM assoc. with COR
                                        # Note: This should never happen!!!
#
# --- determine if the copy owns the required resources. If not, ignore
#     the task suspension.
#
        ldob    cm_cstate(g4),r5        # r5 = copy state
        bbs.f   cmcst_b_NRS,r5,.so_350  # Jif copy currently does not have
                                        #   any resources.
#
# --- Place the copy task in a suspended mode.
#
#     set up new SCD p2 update handlers
#
        ld      cor_scd(g3),r10         # r10 = scd address
        lda     CM$wp2_inactive,r7      # get new update handler
        cmpobe.f 0,r10,.so_060          # Jif no SCD defined

        st      r7,scd_p2hand(r10)      # save new scd p2 handler
#
# --- set up new DCD p2 update handlers
#
.so_060:
        ld      cor_dcd(g3),r10         # r10 = dcd address
        lda     CM$wp2_inactive,r7      # get new update handler
        cmpobe.f 0,r10,.so_070          # Jif no DCD defined

        st      r7,dcd_p2hand(r10)      # save new dcd p2 handler
#
# --- set the cor copy registration state to suspended and
#     set the region map state to active
#
.so_070:
        ldconst cormst_act,r3           # r3 = set region map to active
        stob    r3,cor_mstate(g3)
#
# --- copy have the required resources, set No Resoures flag.
#     determine if data copy is currently active. If so,
#     set stop state.
#
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>ladpcm_cm$SuspendOwnership -setting cmcst_b_NRS bit..\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # CM_IM_DEBUG
        setbit  cmcst_b_NRS,r5,r5       # or in No Resources
        stob    r5,cm_cstate(g4)        # save new state

        bbc.f cmcst_b_copy,r5,.so_150   # Jif copy not active

        setbit  cmcst_b_auto,r5,r5      # set auto
        setbit  cmcst_b_stop,r5,r5      # set stop pending
        stob    r5,cm_cstate(g4)        # save new state
        b       .so_end                 # continue
#
# --- copy is not active, set the cor region map state to
#     active
#
.so_150:

        ldconst cormst_act,r3           # r3 = set region map to active
        stob    r3,cor_mstate(g3)
#
# --- wait for all updates to complete
#
.so_350:
        ldos    cor_uops(g3),r4         # check if any outstanding updates
        cmpobe.f 0,r4,.so_400           # Jif if outstanding updates

        ldconst 10,g0                   # wait 10 ms and try again
        call    K$twait
        b       .so_350                 # continue
#
# --- All outstanding updates have been applied. Flush the control
#     queue.
#
.so_400:
        call    cm$FlushCtlQue          # flush the control queue
#
# --- process any outstanding update errors
#
        call    cm$ProcessUpdErrQue     # process update error queue
#
# --- Send a event back to the cctable
#
.if GR_GEORAID15_DEBUG
        ldob cor_rcsvd(g3),r5
        ldob cor_rcdvd(g3),r4
c fprintf(stderr,"%s%s:%u <GR><cm$SuspendedOwnership> call CCSM$CCtrans with ownersuspended (SuspendOwner)event to other controller srcvid=%lx, destvid=%lx\n", FEBEMESSAGE, __FILE__, __LINE__,r5,r4);
.endif  # GR_GEORAID15_DEBUG

        ldconst OwnerSuspended,g0       # set "Ownership Suspended" event
        call    CCSM$CCTrans            # process the event
#
# --- exit
#
.so_end:
        mov     r14,g0                  # restore g0
        mov     r15,g4                  # restore g4
        ret                             # return to caller

#**********************************************************************
#
#  NAME: cm$use_Resume
#
#  PURPOSE:
#       To provide a means of resuming a secondary copy operation.
#
#  DESCRIPTION:
#       Checks if the secondary copy is in a state to resume the
#       copy process and does so if able to.
#
#  CALLING SEQUENCE:
#       call    cm$usr_Resume
#
#  INPUT:
#       g1 = PCP address at lvl 2
#       g3 = COR address
#       g4 = CM address
#
#  OUTPUT:
#       g1 = PCP at lvl 2
#
#  REGS DESTROYED:
#       Reg. g1 is destroyed
#
#**********************************************************************
#
cm$usr_Resume:
        mov     g1,r15                  # save g1
#
# --- determine if the copy owns the required resources. If not, ignore
#     the resume.
#
        ldob    cm_cstate(g4),r8        # r8 = copy state
        bbc.f   cmcst_b_NRS,r8,.usrr_100 # Jif copy own required resources
#
# --- Copy does not own required resources. Clear the user pause bit in the
#     COR's cm state.
#
        ldob    cm_cstate(g4),r8        # r8 = cor's cm state byte
        clrbit  cmcst_b_auto,r8,r8      # clear possible auto
        setbit  cmcst_b_pause,r8,r8     # set user suspended state
        stob    r8,cm_cstate(g4)        # save new state
        b       .usrr_900               # exit
#
# --- The copy owns the required resources. Determine if the copy
#     is in some form of suspension. If it is, check if user paused.
#     if user paused, ignore resume. Otherwise, make sure of autopause
#     handlers and resume copy.
#
.usrr_100:
        bbc.f   cmcst_b_pause,r8,.usrr_900 # JIf not paused
#
# --- Set auto suspend
#
        call    cm$SuspendCopy_auto     # Auto suspend the copy
#
# --- register a resume with possible remote and force a poll
#
        call    cm$Reg_Resume           # register resume
        call    cm$force_LP             # force a local poll
#
# --- return exit.
#
.usrr_900:
        mov     r15,g1                  # restore pcp address
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: cm$usr_Break
#
#  PURPOSE:
#       To provide a means of manually breaking-off a secondary copy
#       operation.
#
#  DESCRIPTION:
#       Checks if secondary copy synchronized and if not sets
#       the pending break-off flag. If all updates have
#       completed, breaks the secondary copy associations, else
#       sets the secondary copy up so as not to apply any more
#       updates to the copy VD and sets the pending break-off flag
#       to break off the associations when the outstanding update
#       operations complete.
#
#  CALLING SEQUENCE:
#       call    cm$usr_break
#
#  INPUT:
#       g1 = PCP at lvl 2
#       g3 = COR address
#       g4 = CM address
#
#  OUTPUT:
#       g1 = PCP at lvl 2
#
#
#  REGS DESTROYED:
#       Reg. g4 is destroyed
#
#**********************************************************************
#
cm$usr_Break:
        mov     g1,r15                  # save pcp address
#
# --- determine if this is an abort (out of sync) or a break (in sync)
#
        ldob    cm_cstate(g4),r8        # r8 = copy state
        bbc.t   cmcst_b_copy,r8,.usrb_35 # Jif NOT in "copy in progress" state
#
#       Data copy is currently active. Set up to break copy upon
#       completion of current segment
#
        setbit  cmcst_b_stop,r8,r8      # set stop state
        setbit  cmcst_b_abort,r8,r8     # set abort state
        stob    r8,cm_cstate(g4)        # save new state
        b       .usrb100                # exit
#
# --- Log Mirror/Copy Break Message to CCB
#
.usrb_35:
        ldconst cmcc_MirrorEnd,g0       # set Mirror/Copy break message
        call    CM_Log_Completion       # send log message
#
# --- break the mirror
#
        call    CM$scbreak              # break secondary copy associations
#
# --- set completion status of PCP to indicate copyu task is terminating
#
        ldconst pcp1st_taskend,r4       # set cm task termination
        stob    r4,pcp2_status(g1)      # save lvl2 status

.usrb100:
        mov     r15,g1                  # restore pcp address
        ret
#
#**********************************************************************
#
#  NAME: cm$usr_Pause
#
#  PURPOSE:
#       To provide a means of stopping (suspending) a secondary copy operation.
#
#  DESCRIPTION:
#       If the secondary copy is synchronized, this routine does
#       nothing. Else, it sets the pending stop flag to suspend
#       the copy operation when any current I/O operations complete.
#
#  CALLING SEQUENCE:
#       call    cm$usr_Pause
#
#  INPUT:
#       g1 = PCP at lvl 2
#       g3 = COR address
#       g4 = CM address
#
#  OUTPUT:
#       None:
#
#  REGS DESTROYED:
#       None:
#
#**********************************************************************
#
cm$usr_Pause:
#
# --- determine if the copy owns the required resources. If not, ignore
#     the resume.
#
        ldob    cm_cstate(g4),r5        # r5 = copy state
        bbc.f   cmcst_b_NRS,r5,.usrp_05 # Jif copy own required resources
#
# --- Copy does not own required resources. Set the user pause bit in the
#     COR's cm state.
#
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>ladpcm-cm$usr_Pause--Copy does not own required resources..set user pause bit\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # CM_IM_DEBUG
        ldob    cm_cstate(g4),r5        # r5 = cor's cm state byte
        clrbit  cmcst_b_auto,r5,r5      # clear possible auto
        setbit  cmcst_b_pause,r5,r5     # set user suspended state
        stob    r5,cm_cstate(g4)        # save new state
        b       .usrp_40                # br
#
# --- determine if copy is already paused
#
.usrp_05:
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>ladpcm-cm$usr_Pause..copy owns resources\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # CM_IM_DEBUG
        bbc.f cmcst_b_pause,r5,.usrp_10 # Jif copy not paused
#
# --- copy already paused. determine if auto suspended
#
        bbc.f cmcst_b_auto,r5,.usrp_40  # Jif auto not set
#
# --- User suspend the copy
#
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>ladpcm-cm$usr_Pause..calling cm$SuspendCopy_user...\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # CM_IM_DEBUG
        call    cm$SuspendCopy_user     # User suspend the copy
#
# --- update configuration and force a poll
#
        call    cm$force_LP             # force a local poll
        b       .usrp_40                # br
#
# --- copy is not paused. determine if data copy is currently
#     active. If so, set stop state.
#
.usrp_10:
        bbc.f cmcst_b_copy,r5,.usrp_20  # Jif copy not active

        setbit  cmcst_b_stop,r5,r5      # set stop pending
        clrbit  cmcst_b_auto,r5,r5      # clear possible auto
        stob    r5,cm_cstate(g4)        # save new state
        b       .usrp_40                # continue
#
# --- copy is not pause nor is data copy active, user suspend the copy
#
.usrp_20:
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>ladpcm-.usrp_20--calling cm$SuspendCopy_user...\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # CM_IM_DEBUG
        call    cm$SuspendCopy_user     # User suspend the copy
#
# --- update configuration and register the suspend with other
#     copy elements
#
        call    cm$Reg_Suspend          # register suspend
#
# ---  exit
#
.usrp_40:
        ret                             # return to caller

#**********************************************************************
#
#  NAME: cm$usr_Swap
#
#  PURPOSE:
#       To provide a means of swapping the Raids of a mirrored a
#       secondary copy operation.
#
#  DESCRIPTION:
#       If the secondary copy is synchronized, this routine swaps
#       the Raids of the secondary copy. Else, it does nothing.
#
#  CALLING SEQUENCE:
#       call    cm$usr_Swap
#
#  INPUT:
#       g1 = PCP at lvl 2
#       g3 = COR address
#       g4 = CM address
#
#  OUTPUT:
#       g1 = PCP at lvl 2
#
#  REGS DESTROYED:
#       None:
#
#**********************************************************************
#
cm$usr_Swap:
c       r12 = g0;                       # save g0
c       r13 = g1;                       # save g1
c       r14 = g2;                       # save g2
c       r15 = g5;                       # save g5

        ldconst pcp1st_llfuerr,r8       # default to error condition
        stob    r8,pcp2_status(g1)
        ld      pcp1_reg2-ILTBIAS(g1),r11 # get sequence number from PCP
#
# --- default to the copy NOT owned script and determine copy ownership
#
        lda     cm$NotOwner_Vsync_script,r9 # r9 = Vsync handler script

        ldob    cm_cstate(g4),r8        # load state
        bbs     cmcst_b_NRS,r8,.usrs_100 # Jif not owner of all elements of copy
#
# --- Uses the copy owned script
#
        lda     cm$Owner_Vsync_script,r9 # r9 = Vsync handler script
#
# --- Copy is owned by this controller. determine if the source and destination
#     VDD's are still defined
#
        ld      cor_srcvdd(g3),r10      # r10 = source VDD
        cmpobe.f 0,r10,.usrs_900        # Jif no srcvdd defined

        ld      cor_destvdd(g3),r10     # r10 = destination VDD
        cmpobe.f 0,r10,.usrs_900        # Jif no destvdd defined
#
# --- collect any required remote RM's
#
        ld      K_ficb,r3               # r3 = FICB address
        ld      fi_vcgid(r3),r10        # r10 = my serial #
#
# --- determine if copy source is on another MAG. If it is, determine
#     if a region map required collection
#
                                        # g3 = COR
                                        # g4 = CM
        ld      cor_rssn(g3),g5         # g5 = source MAG S/N
        cmpobe  g5,r10,.usrs_25         # Jif me
        call    cm$collect_RM           # collect RM from remote device
#
# --- determine if copy destination is on another MAG. If it is,
#     determine if a region map required collection
#
.usrs_25:
                                        # g3 = COR
                                        # g4 = CM
        ld      cor_rdsn(g3),g5         # g5 = destination MAG S/N
        cmpobe  g5,r10,.usrs_27         # Jif me
        call    cm$collect_RM           # collect RM from remote device

.usrs_27:
#
# --- Swap the raids and wait for completion
#
.usrs_100:
c       g0 = r9;                        # g0 = Vsync handler script
                                        # g3 = COR
                                        # g4 = CM
        call    cm$qVsync               # queue CM to cm$Vsync work queue
        cmpobne  pcp1st_ok,g0,.usrs_900 # Jif swap did not occurred
#
# --- Indicate copy swap complete
#
        mov     r11,g2                  # g2 = sequence number
                                        # g3 = COR
        call    CCSM$snd_swapdone       # indicate copy swap complete
        b       .usrs_1000

.usrs_900:
c fprintf(stderr, "%s%s:%u <lad_pcm.as>: RAID-Swap FAILED in function cm$usr_Swap\n", FEBEMESSAGE, __FILE__, __LINE__);
#
# ---  exit
#
.usrs_1000:
c       g0 = r12;                       # restore g0
c       g1 = r13;                       # restore g1
c       g2 = r14;                       # restore g2
c       g5 = r15;                       # restore g5
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: cm$ProcessPoll
#
#  CALLING SEQUENCE:
#       call    cm$ProcessPoll
#
#  INPUT:
#       g1 = PCP address at lvl 2
#       g3 = COR address
#       g4 = CM address
#
#  OUTPUT:
#       g0 = completion branch handler
#
#  REGS DESTROYED:
#       Reg. g1 is destroyed
#
#**********************************************************************
#
cm$ProcessPoll:
        mov     g1,r14                  # save g1
        mov     g2,r15                  # save g2
#
# --- clear outstanding poll request flag
#
        ldob    cor_flags(g3),r4        # r4 = COR flags byte
        clrbit  CFLG_B_POLL_REQ,r4,r4   # Clear outstanding poll request flag
        stob    r4,cor_flags(g3)        # save updated COR flags byte
#
# --- determine if no resources or poll inhibit is active
#
        ldob    cm_cstate(g4),r8        # load state
        bbs     cmcst_b_NRS,r8,.ppoll_000  # Jif not owner of all elements of copy

        ldob    cm_PIC(g4),r4           # is poll inhibit active ???
        cmpobe  0,r4,.ppoll_005         # Jif no
#
# --- setup to return error status
#
.ppoll_000:
        ldconst pcp1st_pollinhibit,r10  # r10 = poll inhibit status
        stob    r10,pcp2_status(g1)     # save status
        b       .ppoll_100            # continue

.ppoll_005:
#
# --- determine if the copy has been registered
#
        ldob    cor_crstate(g3),r4
        cmpoble corcrst_active,r4,.ppoll_Registered # Jif initialized
#
# --- copy  has never been initialized
#
        ldconst 0,g1                    # g1 = register type code
        call    CM$regcopy              # register the copy operation where
                                        #  appropriate
        cmpobe.t 0,g0,.ppoll_RegSuccess # Jif copy operation registration
                                        #  was successful
        cmpobe.f 1,g0,.ppoll_StillValid # Jif may still be valid
#
# --- copy must be aborted
#
        ldconst 0x80,g0                 # force a terminate status
        mov     g0,r10
        mov     g0,g2
        b       .ppoll_010              # and process it
#
# --- Copy operation was unsuccessful but is still valid.
#       Wait awhile and then try to register it again.
#
#       NOTE:
#            Because the copy state in "uni", only the COR address
#            is required.
#
.ppoll_StillValid:
#        call    cm$Create_RCC           # place it on the retry queue
#
# --- copy registration successful. Set copy registration active.
#
.ppoll_RegSuccess:
        call    CCSM$reg                # tell CCSM about copy reg. done
        ldconst 0,r10                   # force good completion status
        b       .ppoll_100              # continue processing
#
# --- Copy have been registered before, call the polling process
#     routine
#
.ppoll_Registered:
        call    cm$poll_local_cor       # poll copy state
        mov     g0,r10                  # form completion check register
        mov     g0,g2                   # form scan register
        cmpobe  0,g0,.ppoll_100         # Jif no errors

.ppoll_010:
        scanbit g2,r4                   # find MSB
        clrbit  r4,g2,g2                # and clear it
        ldob    cm_pollerr_0[r4*1],r5   # get current error counter for this bit
        addo    1,r5,r5                 # bump it
        stob    r5,cm_pollerr_0[r4*1]   # save new counter

        ld      cm$polltbl[r4*4],r6     # get address of processing routine
        callx   (r6)                    # and do it
        cmpobe  0,g0,.ppoll_090         # JIf 0 status
#
# --- @@@@ FINISH what to do with bad status
#
        b       .ppoll_092            # br

.ppoll_090:
        clrbit  r4,r10,r10              # clear this bit from completion
                                        #  check register
.ppoll_092:
        cmpobne 0,g2,.ppoll_010       # Jif more bits
#
# --- return PCP.
#
.ppoll_100:
        mov     r14,g1                  # restore g1
        ldob    pcp2_status(g1),r4      # r4 = current pcp status
        lda     -ILTBIAS(g1),g1         # pull back to PCP1 level
        mov     0,r3                    # clear r3
        ld      pcp1_cr(g1),r6          # r6 = completion handler
        stob    r4,pcp1_status(g1)      # save completion status
        callx   (r6)                    # call completion routine

        st      r3,cm_pctlrqst(g4)      # clear pending control request
#
# --- determine if the copy is auto suspended and if there are paths
#     to all elements of the copy
#
        lda     .pex_Exchange,g0        # set completion branch handler
        cmpobne.f 0,r10,.ppoll_300      # Jif completion status not zero

        ldob    cm_cstate(g4),r8        # r5 = copy state
        cmpobe  cmcst_uni,r8,.ppoll_200 # JIf copy uninitialized

        bbs.f   cmcst_b_auto,r8,.ppoll_200  # Jif auto suspended
#
# --- the copy is not auto suspended. exit if the copy is user suspended
#     or a copy is currently active. otherwise, determine if a region
#     map has been collected. if so, set copy in auto suspend and restart
#     the copy.
#
        bbs.f   cmcst_b_pause,r8,.ppoll_300 # Jif suspended

        bbs.f   cmcst_b_copy,r8,.ppoll_300  # Jif copy is active

        ld      cor_rmaptbl(g3),r4      # is there a RM ???
        cmpobe.t 0,r4,.ppoll_300        # Jif no region map
#
# --- place copy in auto suspend mode
#
        call    cm$SuspendCopy_auto     # auto suspend the copy

# --- copy is currently auto suspended however there seems to be path
#     to all elements of the copy, allocate an SCR and set it up to
#     determine if any of those paths is a data paths.
#
.ppoll_200:
        ld      cm_scr(g4),r4           # is there already an SCR allocated
        cmpobne 0,r4,.ppoll_300         # yes - Already processing copy

        call    cm$get_scr              # get an scr
        lda     cm$pexeq,r6             # completion handler routine address
        ldob    cm_pri(g4),r5           # r5 = copy priority/strategy
        and     0x0f,r5,r7              # r7 = isolated strategy
        shro    4,r5,r5                 # r5 = isolated priority

        stob    1,scr1_segcnt(g1)       # save number of segments to copy
        stob    r5,scr1_priority(g1)    # save copy priority
        st      r6,scr1_cr(g1)          # save completion handler address
        stob    r7,scr1_strategy(g1)    # save strategy
#
# --- reset the last percent update timer value
#
c       r4 = (UINT32)(get_tsc()/(1000000 * ct_cpu_speed)); # seconds
        st      r4,cm_lasttime(g4)      # save current time
#
# --- fork off task to update FE
#
        movq    g0,r8                   # Save g0-g3
        ld      cor_srcvdd(g3),r6       # r6 = source vdd address
        ldconst 0xffff,g2               # undefined VID
        cmpobe  0,r6,.ppoll_222         # Jif NULL
        ldos    vd_vid(r6),g2           # load VID from vdd

.ppoll_222:
        ld      cor_destvdd(g3),r6      # r6 = destination vdd address
        ldconst 0xffff,g3               # undefined VID
        cmpobe  0,r6,.ppoll_224         # Jif NULL
        ldos    vd_vid(r6),g3           # load VID from vdd

.ppoll_224:
        lda     V$updFEStatus,g0        # g0 = Address of the task to start
        ldconst VUPDFESTATUS,g1         # g1 = Priority of task to run
c       CT_fork_tmp = (ulong)"V$updFEStatus-from-poll";
        call    K$tfork                 # Start up task that will update the
        movq    r8,g0                   # Restore g0-g3
#
# --- set the return handler
#
        lda     .pex_NextSegment,g0     # start the copy process

.ppoll_300:
        mov     r15,g2                  # restore g2
        ret                             # return to caller
#
# --- poll response processing table
#
        .data
cm$polltbl:
                                        # bit #
        .word   cm$poll_err_rtn         #  0  # remote MAG not polled
        .word   cm$poll_rtn             #  1  #
        .word   cm$poll_rtn             #  2  #
        .word   cm$poll_collect         #  3  # RM accumulation required
        .word   cm$poll_usrspnd         #  4  # crstate to user suspend
        .word   cm$poll_rtn             #  5  #
        .word   cm$poll_rtn             #  6  #
        .word   cm$poll_term            #  7  # termination and restart required
#
        .text
#
#**********************************************************************
#
#  NAME: cm$poll_term
#
#  CALLING SEQUENCE:
#       call    cm$poll_term
#
#  INPUT:
#       g2 = poll status
#       g3 = COR address
#       g4 = CM address
#
#  OUTPUT:
#       g0 = completion status (NON ZERO)
#       g2 = 0
#
#  REGS DESTROYED:
#       None:
#
#**********************************************************************
#
cm$poll_term:
        mov     g1,r14                  # save g1
        mov     g3,r15                  # save g3
#
# --- Request copy termination
#
        call    CCSM$ended              # request copy termination
#
# --- Log copy termination request message
#
        ldconst cmcc_AutoTerm,g0        # g0 = request copy termination message
        call    cm$Log_Completion       # *** Log Message ***
#
# --- restore environment and return to caller
#
        mov     r14,g1                  # restore g1
        mov     r15,g3                  # restore g3
        ldconst eccopycomp,g0           # return copy completed status (NON ZERO)
        mov     0,g2                    # clear g2 to test no more bits
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: cm$poll_usrspnd
#
#  CALLING SEQUENCE:
#       call    cm$poll_usrspnd
#
#  INPUT:
#       g2 = poll status
#       g3 = COR address
#       g4 = CM address
#
#  OUTPUT:
#       g0 = completion branch handler
#
#  REGS DESTROYED:
#       None:
#
#**********************************************************************
#
cm$poll_usrspnd:
        ldob    cm_cstate(g4),r5        # r5 = copy state
        bbc.f cmcst_b_pause,r5,.poluspnd_10 # Jif copy not paused
        bbc.f cmcst_b_auto,r5,.poluspnd_exit # JIf already in user suspended
        b       .poluspnd_20            # br to setup MMC stuff
#
# --- copy is not paused. determine if the copy is performing data
#     movement. if so, set it up to stop upon completion of current
#     segment.
#
.poluspnd_10:
        bbc.f cmcst_b_copy,r5,.poluspnd_20 # Jif data transfer NOT active

        setbit cmcst_b_stop,r5,r5       # set stop bit
        stob    r5,cm_cstate(g4)        # save new state
        b       .poluspnd_exit          # br
#
# --- data transfer not active. set copy to user suspended
#
.poluspnd_20:
        call    cm$SuspendCopy_user     # user suspend the copy
#
# --- exit
#
.poluspnd_exit:
         ret                            # return to caller
#
#**********************************************************************
#
#  NAME: cm$poll_collect
#
#  CALLING SEQUENCE:
#       call    cm$poll_collect
#
#  INPUT:
#       g2 = poll status
#       g3 = COR address
#       g4 = CM address
#
#  OUTPUT:
#       g0 = completion branch handler
#
#  REGS DESTROYED:
#       None:
#
#**********************************************************************
#
cm$poll_collect:
        mov     g1,r14                  # save g1
        mov     g2,r15                  # save g2
        mov     g5,r13                  # save g5
#
# --- load source MAGNITUDE s/n. determine if source is me. if not,
#     and collect it's RM
#
        ld      cor_rcsn(g3),r4         # r4 = my MAG s/n
        ld      cor_rssn(g3),g5         # g5 = MAG s/n
        cmpobe.f g5,r4,.polcol_010      # JIf source is me

        call    cm$collect_RM           # collect the remote RM
        cmpobne.t 0,g0,.polcol_050      # JIf errors
#
# --- load destination s/n. determine if source and destination the same.
#     if not, determine if destination is me.
#
.polcol_010:
        ld      cor_rdsn(g3),r5         # r4 = MAG s/n
        cmpobe.f r5,g5,.polcol_500      # JIf source and dest the same
        cmpobe.f r5,r4,.polcol_500      # JIf dest is me
#
# --- source and destination not the same nor me. collect
#     destination RM.
#
        mov     r5,g5                   # place in correct register
        call    cm$collect_RM           # collect the remote RM
        cmpobe.t 0,g0,.polcol_500       # JIf no errors
#
# --- determine if copy is already paused
#
.polcol_050:
        ldob    cm_cstate(g4),r5        # r5 = copy state
        bbc.f cmcst_b_pause,r5,.polcol_100 # Jif copy not paused
#
# --- copy already paused. determine if auto suspended
#
        bbs.f cmcst_b_auto,r5,.polcol_500  # Jif auto not set
#
# --- copy not auto suspended. set auto suspended bit, set
#     user suspend state in COR, and register changes
#
        setbit  cmcst_b_auto,r5,r5      # set user suspend
        stob    r5,cm_cstate(g4)        # save new state

#        ldconst corcrst_autosusp,r3     # set copy auto suspended
#        stob    r3,cor_crstate(g3)
#
# --- force a poll
#
        call    cm$force_LP             # force a local poll
        b       .polcol_500             # br
#
# --- copy is not paused. determine if data copy is currently
#     active. If so, set stop state.
#
.polcol_100:
        bbc.f cmcst_b_copy,r5,.polcol_200  # Jif copy not active

        setbit  cmcst_b_stop,r5,r5      # set stop pending
        stob    r5,cm_cstate(g4)        # save new state
        b       .polcol_500             # continue
#
# --- copy is not pause nor is data copy active, suspend the copy
#
.polcol_200:
        call    cm$SuspendCopy_auto     # Auto suspend the copy
#
# --- register the suspend with other copy elements
#
        call    cm$Reg_Suspend          # register suspend

.polcol_500:
        mov     r14,g1                  # restore g1
        mov     r15,g2                  # restore g2
        mov     r13,g5                  # restore g5
        ret                             # return to caller

#**********************************************************************
#
#  NAME: cm$poll_rtn
#
#  CALLING SEQUENCE:
#       call    cm$poll_rtn
#
#  INPUT:
#       g3 = COR address
#       g4 = CM address
#       g5 = poll status
#
#  OUTPUT:
#       g0 = completion branch handler
#
#  REGS DESTROYED:
#       None:
#
#**********************************************************************
#
cm$poll_rtn:
        ldconst 0,g0
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: cm$poll_err_rtn
#
#  CALLING SEQUENCE:
#       call    cm$poll_err_rtn
#
#  INPUT:
#       g3 = COR address
#       g4 = CM address
#       g5 = poll status
#
#  OUTPUT:
#       g0 = completion branch handler
#
#  REGS DESTROYED:
#       None:
#
#**********************************************************************
#
cm$poll_err_rtn:
        ldconst 1,g0
        ret                             # return to caller
#
#***********************************************************************
#***********************************************************************
#***********************************************************************
#
#       C O P Y    S W A P   R O U T I N E S
#
#***********************************************************************
#***********************************************************************
#***********************************************************************
#
#**********************************************************************
#
#  NAME: cm$Swap_Raids
#
#  PURPOSE:
#       To provide a means of swapping the virtual drive
#       definitions between orig. and copy VDs.
#
#  DESCRIPTION:
#       Sets the device capacity of the copy VDD equal
#       to the capacity of the orig. VDD if copy was successful.
#       If successful, swaps the virtual drive components so
#       that any host using the orig. VD now uses the copy VD
#       and any host assigned to the copy VD now has the orig.
#       VD assigned to it.
#
#  CALLING SEQUENCE:
#       call    cm$Swap_Raids
#
#  INPUT:
#       g3 = COR address
#       g4 = CM address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#      None.
#
#**********************************************************************
#
# --- stack frame definition
#
        .set    sfswap_srdd,0           # source rdd address
        .set    sfswap_svlop,4          # source vlop value
        .set    sfswap_drdd,8           # destination rdd address
        .set    sfswap_dvlop,12         # destination vlop value
#
cm$Swap_Raids:
        mov     sp,r15                  # Allocate stack frame...
        lda     16(sp),sp
        movq    0,r4                    # ... and clear it out
        stq     r4,sfswap_srdd(r15)

        PushRegs                        # Save all G registers (stack relative)

        ld      cor_srcvdd(g3),r10      # r10 = source VDD
        ld      cor_destvdd(g3),r11     # r11 = destination VDD

        ld      vd_rdd(r11),r12         # r12 = copy RDD thread
        ldob    rd_type(r12),r4         # raid type of dest vdisk
        cmpobne rdslinkdev,r4,.swapr_01 # NOTE: this one shouldn't be possible
c fprintf(stderr,"%s%s:%u lad_pcm.as: Swap_Raids: Destination device is SS, Inoping the SS vid = %d\n", FEBEMESSAGE, __FILE__, __LINE__, (UINT16)((VDD*)r11)->vid);
c       ss_invalidate_snapshot((UINT16)((VDD*)r11)->vid);
        b       .swapr_60
.swapr_01:

.if     MAG2MAG
        ld      K_ficb,r3               # r3 = FICB address
        ld      fi_vcgid(r3),g7         # g7 = my serial #
        mov     g4,r3                   # save g4
#
# --- Check for any VLink locks that need to be moved.
#
# --- Check Source VDD for VLink.
#
        ld      vd_rdd(r10),g4          # g4 = assoc. RDD address
        ldob    rd_type(g4),r4          # r4 = RAID type code
        cmpobne rdlinkdev,r4,.swapr_05  # Jif not linked device type RAID

        st      g4,sfswap_srdd(r15)     # save source rdd
#
# --- VLink being moved. Need to move VLink lock on destination MAG.
#
        ldconst 0,g2                    # g2 = new cluster # for orig. VDD
        ldos    vd_vid(r11),g3          # g3 = new VLink # for source VDD
        ldconst 0,g5                    # g5 = current owner cluster #
        ldos    vd_vid(r10),g6          # g6 = current owner VLink #
        call    DLM$VLmove              # start VLink move process

        ld      rd_vlop(g4),r4          # r4 = vlop value
        st      r4,sfswap_svlop(r15)    # save it
#
# --- Check Destination VDD for VLink.
#
.swapr_05:
        ld      vd_rdd(r11),g4          # g4 = assoc. RDD address
        ldob    rd_type(g4),r4          # r4 = RAID type code
        cmpobne rdlinkdev,r4,.swapr_09  # Jif not linked device type RAID

        st      g4,sfswap_drdd(r15)     # save destination rdd
#
# --- VLink being moved. Need to move VLink lock on destination MAG.
#
        ldconst 0,g2                    # g2 = new cluster # for copy VDD
        ldos    vd_vid(r10),g3          # g3 = new VLink # for dest. VDD
        ldconst 0,g5                    # g5 = current owner cluster #
        ldos    vd_vid(r11),g6          # g6 = current owner VLink #
        call    DLM$VLmove              # start VLink move process

        ld      rd_vlop(g4),r4          # r4 = vlop value
        st      r4,sfswap_dvlop(r15)    # save it

.swapr_09:
        mov     r3,g4                   # restore g4
.endif  # MAG2MAG
#
# --- set new VDISK size
#
        ldl     vd_devcap(r10),r8       # r8 = source VDD device capacity
        stl     r8,vd_devcap(r11)       # save source cap. in copy VDD
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
#
# --- Update the remote cache
#
#        movl    g0,r4
#        ldos    vd_vid(r11),g0          # Get the VID for the update
#        ldconst FALSE,g1                # Change the VDD - do not delete
#        call    D$updrmtcachesingle     # Update single
#        movl    r4,g0
#
# --- Swap VLink attribute flag bits
#
        ldos    vd_attr(r11),r4         # r4 = copy VDD attributes
        ldos    vd_attr(r10),r5         # r5 = orig. VDD attributes
        ldconst vdvlink,r8              # r8 = vlink attribute mask
        and     r8,r4,r6                # r6 = copy VDD VLink bit
        and     r8,r5,r7                # r7 = orig. VDD VLink bit
        andnot  r8,r4,r4                # r4 = copy VDD attributes-VLink bit
        andnot  r8,r5,r5                # r5 = orig. VDD attributes-VLink bit
        or      r7,r4,r4                # add orig. VLink flag to copy VDD
        or      r6,r5,r5                # add copy VLink flag to orig. VDD
        stos    r4,vd_attr(r11)         # save updated copy VDD attributes
        stos    r5,vd_attr(r10)         # save updated orig. VDD attributes
#
# --- swap the RAIDs
#
        ld      vd_rdd(r11),r12         # r12 = copy RDD thread
        ld      vd_rdd(r10),r13         # r13 = orig. RDD thread
        st      r12,vd_rdd(r10)
        st      r13,vd_rdd(r11)
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
#
# --- swap the ssms stuff if one of these vdisks is a snapshot
#
        ldob    rd_type(r12),r4         # raid type of dest vdisk
        cmpobe  rdslinkdev,r4,.swapr_24 # NOTE: this one shouldn't be possible
        ldob    rd_type(r13),r4         # raid type of source vdisk
        cmpobne rdslinkdev,r4,.swapr_25 # neither vdisk was a snapshot
        ld K_ficb, r6                   # load K_ficb
        ld fi_cserial(r6),r6            # Get my serial number
        and 0xf,r6, r6                  # Get dcn id
        ldob vd_owner(r10),r7           # Get the owner of source vdisk
        cmpobne r6,r7,.swapr_25         # Don't swap SSMS if im not the owner
                                        # (bcoz non-owner will have null pointers)

.swapr_24:                              # got a snapshot

        ldos    vd_vid(r11),r6          # ensure new SS vid is same as its ssm_ssvid
        ld      vd_incssms(r10),r7      # get ssms record of SS
        stos    r6,ssm_ssvid(r7)        # store new ss vid to the ssms

        ld      ssm_frstoger(r7),r8     # update ssvid in all the ogers related to this ssms
.swapr_24_0:
        stos    r6,ogr_ssvid(r8)
        ld      ogr_link(r8),r8
        cmpobne 0,r8,.swapr_24_0

        ld      vd_incssms(r11),r6      # swap incssms pointers
        ld      vd_incssms(r10),r7
        st      r6,vd_incssms(r10)
        st      r7,vd_incssms(r11)
#
# now need to update the SSMS internal data structure pointers
#
        ld      vd_outssms(r11),r6      # swap outssms pointers
        ld      vd_outssms(r10),r7      #  which should be 0 btw
        st      r6,vd_outssms(r10)
        st      r7,vd_outssms(r11)

        ld      vd_outssmstail(r11),r6  # swap outssmstail pointers
        ld      vd_outssmstail(r10),r7  #  which should be 0 btw
        st      r6,vd_outssmstail(r10)
        st      r7,vd_outssmstail(r11)

        ldos    vd_scorvid(r11),r6      # swap the scorvid to track ss source
        ldos    vd_scorvid(r10),r7
        stos    r6,vd_scorvid(r10)
        stos    r7,vd_scorvid(r11)

        ldob    vd_scpcomp(r11),r6      # swap the copy complete to track ss source
        ldob    vd_scpcomp(r10),r7
        stob    r6,vd_scpcomp(r10)
        stob    r7,vd_scpcomp(r11)

.swapr_25:

# c fprintf(stderr,"<SWAP RAID POINTERS> srcvid=%u destvid=%u\n",(UINT32)(((VDD*)(r10))->vid),(UINT32) (((VDD*)(r11))->vid));
#
# --- swap number of RAIDs in each vdisk
#
        ldob    vd_raidcnt(r10),r4      # # of raids in source vdisk
        ldob    vd_raidcnt(r11),r5      # # of raids in dest. vdisk
        stob    r4,vd_raidcnt(r11)
        stob    r5,vd_raidcnt(r10)
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
#
#       Change rd_vid in each RDD segment
#       source vdd (r10) with destination rdd (r12)
#
        ldos    vd_vid(r10),r4          # r4 = VID orig.

c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */

.swapr_30:
        stos    r4,rd_vid(r12)
        ld      rd_nvrdd(r12),r12       # r12 = next rdd
        cmpobne 0,r12,.swapr_30         # Jif another RDD in thread
#
# ---   destination vdd (r11) with or source rdd (r13)
#
        ldos    vd_vid(r11),r4          # r4 = VID orig.

.swapr_40:
        stos    r4,rd_vid(r13)
        ld      rd_nvrdd(r13),r13       # r12 = next rdd
        cmpobne 0,r13,.swapr_40         # Jifr another RDD in thread

.if     MAG2MAG
        mov     g4,r4                   # save g4
        mov     r11,g4                  # g4 = copy VDD address
        call    DLM$chg_size            # change peer VDisk size if necessary
        mov     r4,g4                   # restore g4
.endif  # MAG2MAG

#
# --- wait for the DLM VLmove requests to complete
#
        ldconst 10,r14                  # r14 = loop count (MAX 1 second)

.swapr_50:
        ldl     sfswap_srdd(r15),r4     # is there a source RDD defined ???
        cmpobe.f 0,r4,.swapr_56         # Jif source RDD not defined

        ld      rd_vlop(r4),r6          # r6 = current source vlop value
        cmpobe  0,r6,.swapr_54          # Jif done
        cmpobe  r5,r6,.swapr_58         # Jif still waiting for completion

.swapr_54:
        ldconst 0,r4                    # r4 = 0
        st      r4,sfswap_srdd(r15)     # clear source vdd

.swapr_56:
        ldl     sfswap_drdd(r15),r4     # is there a destination RDD defined ???
        cmpobe.f 0,r4,.swapr_60         # Jif no dest. RDD defined

        ld      rd_vlop(r4),r6          # r6 = current destination vlop value
        cmpobe  0,r6,.swapr_60          # Jif done
        cmpobne  r5,r6,.swapr_60        # Jif different vlop pending

.swapr_58:
        subo    1,r14,r14               # decrement loop count
        cmpobe  0,r14,.swapr_60         # Jif loop count expired

        ldconst 100,g0                  # delay for 100 ms.
        call    K$twait                 # try again
        b       .swapr_50
#
# --- exit
#
.swapr_60:
        PopRegsVoid                     # Restore all G registers (stack relative)
        ret
#
#**********************************************************************
#
#  NAME: cm$NotOwner_Vsync_script
#
#  PURPOSE:
#       To provide a means of validating, executing, and registering the
#       swapping the virtual drive definitions between orig. and copy VDs
#       after the completion of a copy/mirror operation.
#
#  CALLING SEQUENCE:
#       call    cm$NotOwner_Vsync_script
#
#  INPUT:
#       g4 = CM address
#       g3 = COR address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
cm$NotOwner_Vsync_script:
#
# --- lock the virtual exec
#
        call    cm$Vlock                 # lock out the Virtual exec
#
# --- swap the RAIDs
#
#c      fprintf(stderr,"<SWAP-NotOwner> calling cm$Swap_Raids\n");
       call     cm$Swap_Raids           # swap the raids
#
# --- unlock the Virtual exec
#
        call    cm$Vunlock              # unlock the Virtual exec
#
# --- register the new configuration
#
        call    cm$Modify_CORs          # modify the cors
#
# --- log RAID swap message
#
        ldconst cmcc_RAIDSwap,g0        # g0 = RAID swap Message
        call    cm$Log_Completion       # log completion
#
# --- exit
#
        mov     ecok,g0                 # set successful status
        ret                             # return to caller

#**********************************************************************
#
#  NAME: cm$Owner_Vsync_script
#
#  PURPOSE:
#       To provide a means of validating, executing, and registering the
#       swapping the virtual drive definitions between orig. and copy VDs
#       originated by a user control operation.
#
#  CALLING SEQUENCE:
#       call    cm$Owner_Vsync_script
#
#  INPUT:
#       g4 = CM address
#       g3 = COR address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
cm$Owner_Vsync_script:
#
# --- inhibit polling
#
        call    cm$Inhibit_Poll         # inhibit polling
#
# --- suspend the cors
#
        call    cm$Suspend_CORs         # suspend any associated cors
#
# --- lock the virtual exec
#
        call    cm$Vlock                # lock out the Virtual exec
#
# --- swap the RAIDs
#
#c      fprintf(stderr,"<SWAP-Owner> calling cm$Swap_Raids\n");
       call     cm$Swap_Raids           # swap the raids

        ld cor_srcvdd(g3),r10
c       GR_SetSwapDoneFlagOnSrcVDD((VDD*)r10);
#
# --- unlock the Virtual exec
#
        call    cm$Vunlock              # unlock the Virtual exec

        ld     cor_destvdd(g3),r10
        ld     vd_incssms(r10),r9
c fprintf(stderr, "%s%s:%u  Incoming SSMS of dest vdisk=%lx\n", FEBEMESSAGE, __FILE__, __LINE__,r9);
        cmpobe 0,r9,.ovs10
        ldob   ssm_prefowner(r9),r7
        ld     gnv_oger_array[r7*4],r8
        ldos   ssm_ssvid(r9),r6
c fprintf(stderr, "%s%s:%u Updating the SSMS NV after Copy Swap SSVID=%lx\n", FEBEMESSAGE, __FILE__, __LINE__,r6);
c       update_ssms_nv((SSMS*)r9,(OGER*)r8);

.ovs10:
#
# --- register the new configuration
#
        call    cm$Modify_CORs          # modify the cors
#
# --- merge the required region maps
#
        call    cm$local_merge
#
# --- reactivate any COR suspended in pcwait4swap state
#
c       TaskReadyByState(pcwait4swap);  # reactivate any COR suspended in pcwait4swap
#
# --- resume polling
#
        call    cm$Resume_Poll          # resume polling
#
# --- log RAID swap message
#
        ldconst cmcc_RAIDSwap,g0        # g0 = RAID swap Message
        call    cm$Log_Completion       # log completion
#
# --- exit
#
        mov     ecok,g0                 # set successful status
        ret                             # return to caller

#**********************************************************************
#
#  NAME: cm$qVsync
#
#  PURPOSE:
#       To provide a means of queueing a CM to the cm$seccopy process
#       work queue.
#
#  DESCRIPTION:
#       Queues the CM to the cm$seccopy work queue and makes the
#       cm$seccopy process ready if necessary.
#
#  CALLING SEQUENCE:
#       call    cm$qVsync
#
#  INPUT:
#       g0 = Vsync handler script (0=none)
#       g3 = COR address
#       g4 = CM address
#
#  OUTPUT:
#       g0 = return status
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
cm$qVsync:
        mov     g1,r15                  # save g1
c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT

        ldconst 0,r4                    # r4 = 0
        st      g0,ILTBIAS+pcp2_handler(g1)  # save Vsync handler
        st      g3,ILTBIAS+pcp2_cor(g1)      # save address of cor
        stob    r4,ILTBIAS+pcp2_function(g1) # make sure function is clear

        ldob    cm_flags(g4),r8         # r8 = flags byte
        setbit  0,r8,r8                 # set flag indicating on work queue
        stob    r8,cm_flags(g4)         # save updated flags

        lda     cm$q2cmw,g0             # g0 = address of queue routine
        call    K$qw                    # enqueue the ilt

        ldob    pcp1_status(g1),g0      # g0 = completion status

.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
        mov     r15,g1                  # restore g1
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: cm$q2cmw
#
#  PURPOSE:
#       To provide a means of queueing a CM to the cm$Vsync process
#       work queue.
#
#  DESCRIPTION:
#       Queues the CM to the cm$Vsync work queue and makes the
#       cm$seccopy process ready if necessary.
#
#  CALLING SEQUENCE:
#       call    cm$qcmw
#
#  INPUT:
#       g1 = PCP at level 2
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
cm$q2cmw:
        lda     cmw_qu,r11              # r11 = address of queue
        b       K$cque                  # enqueue the ilt
#
#**********************************************************************
#
#  NAME: cm$Vsync
#
#  PURPOSE:
#       To provide a means of processing secondary copy completion
#       processing synchronized with the v$exec process.
#
#  DESCRIPTION:
#       When this process is made ready by a secondary copy process
#       completing, this process will wait for the v$exec process
#       to be in the ready state and then switch it to the pcbsclock
#       state to insure it cannot execute while a secondary copy
#       completion occurs. This is to insure that while a VD operation
#       is in progress that a switch cannot occur affecting the VD that
#       could cause a data integrity problem to occur with the VD.
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
cm$Vsync:
        b       .vsync_020              # start process
#
# --- Set this process to not ready
#
.vsync_010:
        ldconst pcnrdy,r4               # Set this process to not ready
        stob    r4,pc_stat(r15)
#
# --- sleep
#
        call    K$xchang                # Exchange processes
#
# --- Determine if there are any work to do
#
.vsync_020:
        lda     cmw_qu,r11              # get address of the cmw request queue
        ldq     qu_head(r11),r12        # Get queue head, tail, count
                                        #  and PCB
        cmpobe.t 0,r12,.vsync_010       # Jif there is no work

        mov     r12,g1                  # Isolate next queued ILT
        ld      il_fthd(r12),r12        # Dequeue this ILT
        cmpo    0,r12                   # Check for queue now empty
        subo    1,r14,r14               # Adjust queue count
        sele    r13,r12,r13             # Set up queue tail
        stt     r12,qu_head(r11)        # Update queue head, tail and count
        be.f    .vsync_090              # Jif queue now empty

        st      r11,il_bthd(r12)        # Update backward thread
#
# --- enable interrupts and determine type of processing required
#
.vsync_090:
        ld      pcp2_cor(g1),g3         # g3 = cor address
        ld      cor_cm(g3),g4           # g4 = cm address
#
# --- call Vsync handler
#
        mov     g1,r4                   # save g1
        ld      pcp2_handler(g1),r6     # r6 = Vsync handler address
        cmpobe  0,r6,.vsync_100         # JIf no completion handler

        callx   (r6)                    # handle pre Vsync work
                                        # input:
                                        #       g1 = pcp at lvl 2
                                        #       g3 = cor address
                                        #       g4 = cm address
                                        # output:
                                        #       g0 = completion status
                                        # regs destroyed:
                                        #       g5-g15 may be destroyed
        mov     r4,g1                   # restore g1
#
# --- clear CM on Vsync queue bit
#
.vsync_100:
        ldob    cm_flags(g4),r4         # r4 = flags byte
        clrbit  0,r4,r4                 # clear on work queue flag
        stob    r4,cm_flags(g4)         # save updated flags
#
# --- call completion handler
#
        lda     -ILTBIAS(g1),g1         # restore g1 and pull it back to PCP1 level
        stob    g0,pcp1_status(g1)      # save completion status
        ld      pcp1_cr(g1),r6          # load address of completion handler
        callx   (r6)                    # call completion handler
#
# --- exchange process nad see if there is more work
#
        call    M$gpdelay               # Exchange processes
        b       .vsync_020              # look for more work
#
#**********************************************************************
#
#  NAME: cm$Vlock
#
#  PURPOSE:
#       To provide a means of placing a LOCK on the v$exec process.
#
#  DESCRIPTION:
#       This process will wait for the v$exec process to be in the
#       ready state and then switch it to the pcbsclock state to
#       insure it cannot execute. This is to insure that while a
#       VD operation is in progress that a switch cannot occur
#       affecting the VD that could cause a data integrity problem
#       to occur with the VD.
#
#  CALLING SEQUENCE:
#       call    cm$Vlock
#
#  INPUT:
#       g3 = COR address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
cm$Vlock:
#
# --- wait for an opportunity in the v$exec task
#
.Vlock_100:
        ld      V_exec_qu+qu_pcb,r7     # r7 = v$exec pcb address
        cmpobe  0,r7,.Vlock_110         # branch if v$exec not started yet
        ldob    pc_stat(r7),r8          # r8 = v$exec process status
        cmpobe.t pcnrdy,r8,.Vlock_200   # Jif v$exec in not ready state
        cmpobe.t pcrdy,r8,.Vlock_200    # Jif v$exec in ready state
.Vlock_110:
        lda     10,g0
        call    K$twait                 # delay awhile
        b       .Vlock_100              # and try this again
#
# --- place the v$exec lock
#
.Vlock_200:
        lda     pcbsclock,r8            # lock out v$exec process
        stob    r8,pc_stat(r7)
#
# --- determine if the is a CM associated with this COR. If there
#     is, clear the "on Vsync queue" flag
#
        cmpobe  0,g3,.Vlock_300         # Jif COR is  not passed

        ld      cor_cm(g3),r5           # load possible CM address
        cmpobe  0,r5,.Vlock_300         # Jif no CM defined

        ldob    cm_flags(r5),r4         # r4 = flags byte
        clrbit  0,r4,r4                 # clear on work queue flag
        stob    r4,cm_flags(r5)         # save updated flags
#
# --- exit
#
.Vlock_300:
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: cm$Vunlock
#
#  PURPOSE:
#       To provide a means of clearing a LOCK of the v$exec process.
#
#  DESCRIPTION:
#       This process will place the v$exec process  in a ready state.
#
#  CALLING SEQUENCE:
#       call    cm$Vunlock
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
cm$Vunlock:
        ld      V_exec_qu+qu_pcb,r7     # r7 = v$exec pcb address
        lda     pcrdy,r8                # unlock out v$exec process
.ifdef HISTORY_KEEP
c CT_history_pcb("cm$Vunlock setting ready pcb", r7);
.endif  # HISTORY_KEEP
        stob    r8,pc_stat(r7)
        call    K$xchang                # Exchange processes
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: cm$Inhibit_Poll
#
#  PURPOSE:
#       To provide a means of inhibiting user polling operation by
#       incrementing the poll inhibit counter (PIC).
#
#  CALLING SEQUENCE:
#       call    cm$Inhibit_Poll
#
#  INPUT:
#       g3 = cor address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
CM_InhibitPoll:
cm$Inhibit_Poll:
        ld      cor_srcvdd(g3),r6       # r6 = source vdd
        ld      cor_destvdd(g3),r7      # r7 = destination vdd

#**************
#
#     SOURCE SCD
#
# --- Inhibit polling of remote devices associated with the
#     source device
#
        ld      vd_scdhead(r6),r8       # r8 = 1st scd on link list
#
# --- find all elements that share this device as a source
#
.inhtpl_110:
        ld      scd_cor(r8),r9          # r9 = current COR
#
# --- determine if there is a CM associated with this COR
#
        ld      cor_cm(r9),r10          # r10 = possible CM
        cmpobe  0,r10,.inhtpl_120       # JIf no CM
#
# --- Increment poll inhibit count
#
        ldob    cm_PIC(r10),r4          # increment poll inhibit count
        addo    1,r4,r4
        stob    r4,cm_PIC(r10)
#
# --- find next SCD in list
#
.inhtpl_120:
        ld      scd_link(r8),r8         # find next SCD
        cmpobne 0,r8,.inhtpl_110        # JIf another cor

#**************
#
#     SOURCE DCD
#
# --- all SCD's have been checked, now check the DCD
#
        ld      vd_dcd(r6),r8          # r8 = possible dcd
        cmpobe  0,r8,.inhtpl_300       # Jif none
#
# --- get COR
#
        ld      dcd_cor(r8),r9         # r13 = current COR
#
# --- determine if there is a CM associated with this COR
#
        ld      cor_cm(r9),r10          # r14 = possible CM
        cmpobe  0,r10,.inhtpl_300       # JIf no CM
#
# --- Increment poll inhibit count
#
        ldob    cm_PIC(r10),r4          # increment poll inhibit count
        addo    1,r4,r4
        stob    r4,cm_PIC(r10)

#**************
#
#     DESTINATION SCD
#
# --- suspend any COR that use the destination device as a source
#
.inhtpl_300:
        ld      vd_scdhead(r7),r8       # r8 = 1st scd on link list
        cmpobe  0,r8,.inhtpl_exit       # JIf none
#
# --- find all elements that share this device as a source
#
.inhtpl_310:
        ld      scd_cor(r8),r9          # r9 = current COR
#
# --- determine if there is a CM associated with this COR
#
        ld      cor_cm(r9),r10          # r10 = possible CM
        cmpobe  0,r10,.inhtpl_320       # JIf no CM
#
# --- Increment poll inhibit count
#
        ldob    cm_PIC(r10),r4          # increment poll inhibit count
        addo    1,r4,r4
        stob    r4,cm_PIC(r10)
#
# --- find next SCD in list
#
.inhtpl_320:
        ld      scd_link(r8),r8         # find next SCD
        cmpobne 0,r8,.inhtpl_310        # JIf another cor
#
# --- exit
#
.inhtpl_exit:
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: cm$Resume_Poll
#
#  PURPOSE:
#       To provide a means of resuming user polling operation by
#       decrementing the poll inhibit counter (PIC).
#
#  CALLING SEQUENCE:
#       call    cm$Suspend_CORs
#
#  INPUT:
#       g3 = cor address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
CM_ResumePoll:
cm$Resume_Poll:
        ld      cor_srcvdd(g3),r6       # r6 = source vdd
        ld      cor_destvdd(g3),r7      # r7 = destination vdd

#**************
#
#     SOURCE SCD
#
# --- Inhibit polling of remote devices associated with the
#     source device
#
        ld      vd_scdhead(r6),r8       # r8 = 1st scd on link list
#
# --- find all elements that share this device as a source
#
.rsmpl_110:
        ld      scd_cor(r8),r9          # r9 = current COR
#
# --- determine if there is a CM associated with this COR
#
        ld      cor_cm(r9),r10          # r10 = possible CM
        cmpobe  0,r10,.rsmpl_120        # JIf no CM
#
# --- decrement poll inhibit count
#
        ldob    cm_PIC(r10),r4          # decrement poll inhibit count
        cmpobe  0,r4,.rsmpl_120         # JIf already zero

        subo    1,r4,r4
        stob    r4,cm_PIC(r10)
#
# --- find next SCD in list
#
.rsmpl_120:
        ld      scd_link(r8),r8         # find next SCD
        cmpobne 0,r8,.rsmpl_110         # JIf another cor

#**************
#
#     SOURCE DCD
#
# --- all SCD's have been checked, now check the DCD
#
        ld      vd_dcd(r6),r8           # r8 = possible dcd
        cmpobe  0,r8,.rsmpl_300         # Jif none
#
# --- get COR
#
        ld      dcd_cor(r8),r9          # r13 = current COR
#
# --- determine if there is a CM associated with this COR
#
        ld      cor_cm(r9),r10          # r14 = possible CM
        cmpobe  0,r10,.rsmpl_300        # JIf no CM
#
# --- decrement poll inhibit count
#
        ldob    cm_PIC(r10),r4          # decrement poll inhibit count
        cmpobe  0,r4,.rsmpl_300         # JIf already zero

        subo    1,r4,r4
        stob    r4,cm_PIC(r10)

#**************
#
#     DESTINATION SCD
#
# --- suspend any COR that use the destination device as a source
#
.rsmpl_300:
        ld      vd_scdhead(r7),r8       # r8 = 1st scd on link list
        cmpobe  0,r8,.rsmpl_exit        # JIf none
#
# --- find all elements that share this device as a source
#
.rsmpl_310:
        ld      scd_cor(r8),r9          # r9 = current COR
#
# --- determine if there is a CM associated with this COR
#
        ld      cor_cm(r9),r10          # r10 = possible CM
        cmpobe  0,r10,.rsmpl_320        # JIf no CM
#
# --- decrement poll inhibit count
#
        ldob    cm_PIC(r10),r4          # decrement poll inhibit count
        cmpobe  0,r4,.rsmpl_320         # JIf already zero

        subo    1,r4,r4
        stob    r4,cm_PIC(r10)
#
# --- find next SCD in list
#
.rsmpl_320:
        ld      scd_link(r8),r8         # find next SCD
        cmpobne 0,r8,.rsmpl_310         # JIf another cor
#
# --- exit
#
.rsmpl_exit:
        ret                             # return to caller

#**********************************************************************
#
#  NAME: cm$SuspendCopy_user
#        cm$SuspendCopy_auto
#
#  PURPOSE:
#       To provide a means of suspending a secondary copy operation.
#
#  DESCRIPTION:
#       Change the SDC and DCD write update handler to suspend.
#
#  CALLING SEQUENCE:
#       call    cm$SuspendCopy_user
#       call    cm$SuspendCopy_auto
#
#  INPUT:
#       g3 = COR address
#       g4 = CM address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None
#
#**********************************************************************
#
cm$SuspendCopy_user:
#
# --- temporarily disable the FE Write Cache of the Source device. This
#     causes the cache of the source device to be flushed.
#
        ld      cor_srcvdd(g3),r6       # r6 = source vdd address
        mov     r6,r15                  # 0 if no source - no Clear
        cmpobe  0,r6,.scpy_03           # Jif NULL

        PushRegs(r3)                    # Save all G regs for "C" call
        ldos    vd_vid(r6),g0           # g0 = load VID from vdd
        ldconst WC_SET_T_DISABLE,g1     # g1 = Function to Temp Disable WC
        call    WC_VDiskDisable         # Go Flush Write Cache and wait
        cmpobe  0,g0,.scpy_01           # Jif no errors
#
# --- there seems to be some issue flushing the cache. At the moment, just issue a
#     message indicating so....
#
c fprintf(stderr,"%s%s:%u ERROR - cm$SuspendCopy_user returned from disabling VDisk Cache - VDD = %x RC = %x\n", FEBEMESSAGE, __FILE__, __LINE__, (UINT32)r15, (UINT32)g0);
        b       .scpy_02        # continue
#
# --- cache was flushed
#
.scpy_01:
c fprintf(stderr,"%s%s:%u cm$SuspendCopy_user returned from disabling VDisk Cache - VDD = %x\n", FEBEMESSAGE, __FILE__, __LINE__, (UINT32)r15);

.scpy_02:
        PopRegsVoid(r3)                 # Restore all G regs

.scpy_03:
        ldconst corcrst_usersusp,r4     # r4 = set copy user suspended
        ldconst cmcc_UsrSpnd,r5         # r5 = set User Suspend message
        b       .scpy_10                # continue

# C access
CM_SuspendCopy_auto:
cm$SuspendCopy_auto:
.if GR_GEORAID15_DEBUG
c fprintf(stderr, "%s%s:%u <GR><lad_pcm.as in..cm$SuspendCopy_auto\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # GR_GEORAID15_DEBUG
        ldconst corcrst_autosusp,r4     # r4 = set copy auto suspended
        ldconst cmcc_AutoSpnd,r5        # r5 = set Auto Suspend message
#
# --- set up the cm state for pause
#
.scpy_10:
        mov     g0,r14                  # save g0
        ldob    cm_cstate(g4),r8        # get current state state
        setbit  cmcst_b_pause,r8,r8     # set pause state
        clrbit  cmcst_b_auto,r8,r8      # clear auto state
        clrbit  cmcst_b_copy,r8,r8      # clear copy state
        stob    r8,cm_cstate(g4)        # save new state
#
# --- set up new SCD update handlers
#

        ld      cor_scd(g3),r10         # r10 = scd address
        lda     CM$wp2_suspend,r7       # get new update handler
        cmpobe.f 0,r10,.scpy_20         # Jif no SCD defined
        st      r7,scd_p2hand(r10)      # save new scd p2 handler
#
# --- set up new DCD update handlers
#
.scpy_20:
        ld      cor_dcd(g3),r10         # r10 = dcd address
        lda     CM$wp2_suspend,r7       # get new update handler
        cmpobe.f 0,r10,.scpy_30         # Jif no DCD defined
        st      r7,dcd_p2hand(r10)      # save new dcd p2 handler
#
# --- set the cor copy registration state to user suspended and
#     set the region map state to active
#
.scpy_30:
        ldconst cormst_act,r3           # set region map to active
        stob    r3,cor_mstate(g3)
#
# --- distribute pause state to the master
#
        ldconst corcrst_autosusp,r3     # r3 = pause type (auto suspended)
        cmpobe  r3,r4,.scpy_40          # Jif auto pause

        call    CCSM$upause             # distribute user paused

        ldconst CpyPaused,g0            # set Copy paused event
        b       .scpy_50                # continue

.scpy_40:
.if GR_GEORAID15_DEBUG
c fprintf(stderr, "%s%s:%u <GR><lad_pcm.as> cm$SuspendCopy_auto... calling CCSM$apause\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # GR_GEORAID15_DEBUG
        setbit  cmcst_b_auto,r8,r8      # set auto state
        stob    r8,cm_cstate(g4)        # save new state

        call    CCSM$apause             # distribute auto paused
        ldconst CpyAutoPaused,g0        # set Copy paused event
#
# --- set the cor copy registration state to user suspended and
#     set the region map state to active
#
.scpy_50:
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>lad_pcm-cm$SuspendCopy_auto..call CCTrans with event Cpy_AutoPaused=CpyAutoPaused\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # CM_IM_DEBUG
        call    CCSM$CCTrans            # Process event
#
# --- fork off task to update FE
#
        movq    g0,r8                   # Save g0-g3
        ld      cor_srcvdd(g3),r6       # r6 = source vdd address
        ldconst 0xffff,g2               # undefined VID
        cmpobe  0,r6,.scpy_52           # Jif NULL
        ldos    vd_vid(r6),g2           # load VID from vdd

.scpy_52:
        ld      cor_destvdd(g3),r6      # r6 = destination vdd address
        ldconst 0xffff,g3               # undefined VID
        cmpobe  0,r6,.scpy_54           # Jif NULL
        ldos    vd_vid(r6),g3           # load VID from vdd

.scpy_54:
        lda     V$updFEStatus,g0        # g0 = Address of the task to start
        ldconst VUPDFESTATUS,g1         # g1 = Priority of task to run
c       CT_fork_tmp = (ulong)"V$updFEStatus-from-scpy";
        call    K$tfork                 # Start up task that will update the
        movq    r8,g0                   # Restore g0-g3
#
# --- Clear the temporarily disable of the FE Write Cache
#
        ldconst corcrst_autosusp,r3     # r3 = pause type (auto suspended)
        cmpobe  r3,r4,.scpy_60          # Jif auto pause

        cmpobe  0,r15,.scpy_60          # Jif source VID NULL

        PushRegs(r3)                    # Save all G regs for "C" call
        ldos    vd_vid(r15),g0          # g0 = load VID from vdd
        ldconst WC_CLEAR_T_DISABLE,g1   # g1 = Function to Temp Disable WC
        call    WC_VDiskDisable         # Go Clear the T Disable flag
c fprintf(stderr,"%s%s:%u cm$SuspendCopy_user returned from Enabling VDisk Cache - VDD = %x\n", FEBEMESSAGE, __FILE__, __LINE__,(UINT32)r15);
        PopRegsVoid(r3)                 # Restore all G regs
#
# --- issue Auto/User Paused Log Message
#
.scpy_60:
        mov     r5,g0                   # g0 = log message type
        call    CM_Log_Completion       # send log message

        mov     r14,g0                  # restore g0
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: CM$scbreak
#
#  PURPOSE:
#       To provide a means of breaking-off a secondary copy operation.
#
#  DESCRIPTION:
#       Change the write update handler to NULL. Wait for all
#       updates to complete. Flush any outstanding operations
#       on the control queue.
#
#  CALLING SEQUENCE:
#       call    CM$scbreak
#
#  INPUT:
#       g3 = COR address
#       g4 = CM address
#
#  OUTPUT:
#       g0 = request status code
#       g4 = 0  (CM address)
#
#  REGS DESTROYED:
#       Reg. g3 is destroyed
#
#**********************************************************************
#
CM$scbreak:
#
# --- determine if this cm is on the cm$Vsync queue
#
        ldob    cm_flags(g4),r4         # r4 = flags byte
        bbs.f   0,r4,.scbr2000          # don't process if on v$seccopy
#
# --- temporarily disable the FE Write Cache of the Source while the Break is
#     in progress
#
        ld      cor_srcvdd(g3),r6       # r6 = source vdd address
        mov     r6,r15                  # 0 if no source - no Clear
        cmpobe  0,r6,.scbr05            # Jif NULL
        PushRegs(r3)                    # Save all G regs for "C" call
        ldos    vd_vid(r6),g0           # g0 = load VID from vdd
        ldconst WC_SET_T_DISABLE,g1     # g1 = Function to Temp Disable WC
        call    WC_VDiskDisable         # Go Flush Write Cache and wait
        cmpobe  0,g0,.scbr001
#
# --- there seems to be some issue flushing the cache. At the moment, just issue a
#     message indicating so....
#
c fprintf(stderr,"%s%s:%u ERROR - SCBreak returned from disabling VDisk Cache - VDD = %x RC = %x\n", FEBEMESSAGE, __FILE__, __LINE__, (UINT32)r15, (UINT32)g0);
        b       .scbr002
#
# --- cache was flushed
#
.scbr001:
c fprintf(stderr,"%s%s:%u SCBreak returned from disabling VDisk Cache - VDD = %x\n", FEBEMESSAGE, __FILE__, __LINE__, (UINT32)r15);

.scbr002:
        PopRegsVoid(r3)                 # Restore all G regs
#
# --- set up new SCD update handlers
#
.scbr05:
        ld      cor_scd(g3),r10         # r10 = scd address
        lda     CM$wp2_null,r7          # get new update handler
        cmpobe.f 0,r10,.scbr10          # Jif no SCD defined
        st      r7,scd_p2hand(r10)      # save new scd p2 handler
#
# --- set up new DCD update handlers
#
.scbr10:
        ld      cor_dcd(g3),r10         # r10 = dcd address
        lda     CM$wp2_null,r7          # get new update handler
        cmpobe.f 0,r10,.scbr20          # Jif no SCD defined
        st      r7,dcd_p2hand(r10)      # save new dcd p2 handler
#
# --- wait for all updates to complete
#
.scbr20:
        ldos    cor_uops(g3),r4         # check if any outstanding updates
        cmpobe.f 0,r4,.scbr100          # Jif if outstanding updates

        ldconst 10,g0                   # wait 10 ms and try again
        call    K$twait
        b       .scbr20                 # continue
#
# --- Flush the control queue
#
.scbr100:
        call    cm$FlushCtlQue          # flush the control queue
#
# --- Process any outstanding update errors
#
        call    cm$ProcessUpdErrQue     # process update errors
#
# --- save source and destination vid's
#
        movq    g0,r8                   # Save g0-g3
        ld      cor_srcvdd(g3),r6       # r6 = source vdd address
        ldconst 0xffff,r4               # undefined VID
        cmpobe  0,r6,.scbr202           # Jif NULL
        ldos    vd_vid(r6),r4           # load VID from vdd

.scbr202:
        ld      cor_destvdd(g3),r6      # r6 = destination vdd address
        ldconst 0xffff,r5               # undefined VID
        cmpobe  0,r6,.scbr204           # Jif NULL
        ldos    vd_vid(r6),r5           # load VID from vdd
#
# --- terminate the copy registrations
#
.scbr204:
        call    CM$termregcopy          # terminate copy operation
#
# --- fork off task to update FE
#
        movq    g0,r8                   # Save g0-g3
        movl    r4,g2                   # move source and destination
        lda     V$updFEStatus,g0        # g0 = Address of the task to start
        ldconst VUPDFESTATUS,g1         # g1 = Priority of task to run
c       CT_fork_tmp = (ulong)"V$updFEStatus-from-scbr";
        call    K$tfork                 # Start up task that will update the
        movq    r8,g0                   # Restore g0-g3
#
# --- Send a event back to the cctable
#
        ldconst CopyEnded,g0            # set "Copy Ended" event
        call    CCSM$CCTrans            # process the event
#
# --- Clear the temporarily disable of the FE Write Cache - Break is complete
#
        cmpobe  0,r15,.scbr2000         # Jif NULL
        PushRegs(r3)                    # Save all G regs for "C" call
        ldos    vd_vid(r15),g0          # g0 = load VID from vdd
        ldconst WC_CLEAR_T_DISABLE,g1   # g1 = Function to Temp Disable WC
        call    WC_VDiskDisable         # Go Clear the T Disable flag
c fprintf(stderr,"%s%s:%u SCBreak returned from Enabling VDisk Cache - VDD = %x\n", FEBEMESSAGE, __FILE__, __LINE__,(UINT32)r15);
        PopRegsVoid(r3)                 # Restore all G regs
#
# --- Get and store the Mirror break time in seconds only if the
#     destination VDD is not null and the destination copy device
#     in NULL. Destination copy device will be NULL if the mirror
#     is broken. Needs to set the break time only if the Mirror is
#     broken.
#
        cmpobe  0,r6,.scbr2000          # Jif destination vdd is NULL
        ld      vd_dcd(r6),r5           # r8 = possible dcd
        cmpobne 0,r5,.scbr2000          # Jif DCD is not NULL
#--        ldos    vd_vid(r6),r5
c       r5 = GetSysTime();              # Get seconds since epoch
c fprintf(stderr, "%s%s:%u <MIRROR_BREAK_TIME> - Going to store the time %d in VID %d\n", FEBEMESSAGE, __FILE__, __LINE__, (UINT32)r5, ((VDD*)r6)->vid);
        st      r5,vd_breakTime(r6)     # Store the time in seconds
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        call   D$p2updateconfig         # Update the P2 NVRAM

.scbr2000:
        mov     ecok,g0                 # set successful status
        ret
#
#**********************************************************************
#
#  NAME: cm$FlushCtlQue
#
#  PURPOSE:
#       To provide a common means of Flushing the control queue.
#
#  CALLING SEQUENCE:
#       call    cm$FlushCtlQue
#
#  INPUT:
#       g3 = COR address
#       g4 = CM address
#
#  OUTPUT:
#       None:
#
#  REGS DESTROYED:
#       None:
#
#**********************************************************************

cm$FlushCtlQue:
        movl    g0,r8                   # save g0-g1
#
# --- remove any entries on the control queue
#
.fcq_50:
        lda     cm_ctlrqstq(g4),r11     # get addr of control request queue
        ldq     qu_head(r11),r12        # Get queue head, tail, count
                                        #  and PCB
        cmpobe.t 0,r12,.fcq_70          # Jif no ctl request
#
# --- Remove this request from queue ....
#
        mov     r12,g1                  # Isolate next queued ILT
        ld      il_fthd(r12),r12        # Dequeue this ILT
        cmpo    0,r12                   # Check for queue now empty
        subo    1,r14,r14               # Adjust queue count
        sele    r13,r12,r13             # Set up queue tail
        stt     r12,qu_head(r11)        # Update queue head, tail and count
        be.f    .fcq_60                 # Jif queue now empty

        st      r11,il_bthd(r12)        # Update backward thread
#
# ---   ... and return it.
#
.fcq_60:
        ldconst pcp1st_ok,r5            # @@@ DEFAULT STATUS @@@
#
        mov     g3,r14                  # save g3
        mov     g4,r15                  # save g4
        lda     -ILTBIAS(g1),g1         # pull back to PCP1 level
        ld      pcp1_cr(g1),r6          # r6 = completion handler
        stob    r5,pcp1_status(g1)      # save completion status
        callx   (r6)                    # call completion routine
        mov     r14,g3                  # restore g3
        mov     r15,g4                  # restore g4
        b       .fcq_50                 # check for more control requests
#
# --- restore environment and exit
#
.fcq_70:
        movl    r8,g0                   # restore g0-g1
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: cm$ProcessUpdErrQue
#
#  PURPOSE:
#       To provide a common means of processing the update error queue.
#
#  CALLING SEQUENCE:
#       call    cm$ProcessUpdErrQue
#
#  INPUT:
#       g3 = COR address
#       g4 = CM address
#
#  OUTPUT:
#       None:
#
#  REGS DESTROYED:
#       None:
#
#**********************************************************************
#
cm$ProcessUpdErrQue:
        movl    g0,r8                   # save g0-g1
        ldconst FALSE,r3                # default to no configuration update

.pueq_400:
        lda     cm_uderrq(g4),r11       # get addr of update error queue
        ldq     qu_head(r11),r12        # Get queue head, tail, count
                                        #  and PCB
        cmpobe.t 0,r12,.pueq_450        # Jif queue empty
#
# --- Remove this request from queue ....
#
        mov     r12,g1                  # Isolate next queued ILT
        ld      il_fthd(r12),r12        # Dequeue this ILT
        cmpo    0,r12                   # Check for queue now empty
        subo    1,r14,r14               # Adjust queue count
        sele    r13,r12,r13             # Set up queue tail
        stt     r12,qu_head(r11)        # Update queue head, tail and count
        be.f    .pueq_410               # Jif queue now empty

        st      r11,il_bthd(r12)        # Update backward thread
#
# ---   ... and return it.
#
.pueq_410:
                                        #   g1 = pcp lvl2 address
        call    cm$ProcessUpdErr        # process the update error
                                        #   g0 = config update flag
                                        #   g1 = pcp lvl2 address
        or      g0,r3,r3                # update config update flag
#
# --- return the PCP
#
        ldconst pcp1st_ok,r4            # current pcp status to OK
        stob    r4,pcp2_status(g1)      # set lvl2 PCP status
        lda     -ILTBIAS(g1),g1         # pull back to PCP1 level
        ld      pcp1_cr(g1),r6          # r6 = completion handler
        stob    r4,pcp1_status(g1)      # save completion status
        callx   (r6)                    # call completion routine
        b       .pueq_400               # continue
#
# ---  determine if configuration requires update
#
.pueq_450:
        movl    r8,g0                   # restore g0-g1
        ret                             # return to caller

#**********************************************************************
#
#  NAME: cm$ProcessUpdErr
#
#  PURPOSE:
#       To provide a common means of processing an update error.
#
#  CALLING SEQUENCE:
#       call    cm$ProcessUpdErr
#
#  INPUT:
#       g1 = PCP lvl2 address
#       g3 = COR address
#       g4 = CM address
#
#  OUTPUT:
#       g0 = update required flag
#       g1 = PCP lvl2 address
#
#  REGS DESTROYED:
#       g1
#
#**********************************************************************
#
cm$ProcessUpdErr:
        ldconst FALSE,r3                # r3 = Default update not required

        lda     -ILTBIAS(g1),r15        # pull back to PCP1 level
        ldl     pcp1_reg1(r15),r12      # r12/r13 - SDA
        ld      pcp1_reg3(r15),r14      # r14 - length
        ld      cor_srcvdd(g3),r11      # r11 is the source VDD
        cmpobe  0,r11,.pue_440          # if no vdd, exit
#
# --- set required bit(s) in Region map associated this write update
#
# Note: Use copy manager cal_seg_bit() format.
c   g1 = cal_seg_bit((UINT32*)&g0, *(UINT64*)&r12, *(UINT64*)&r12 + r14 - 1, ((VDD*)r11)->devCap, 0);
#   g0 = segment bit -- NOTE: limited to 22 bits
#   g1 = number of segments

        ld      cor_rmaptbl(g3),r7      # r7 = region map table address
        cmpobe.f 0,r7,.pue_420          # Jif region map table not defined

        call    cm$cal_region           # calculate region this segment is in
                                        # g2 = region number associated with
                                        #      this segment
        ld      RM_tbl(r7)[g2*4],r4     # r7 = segment map table associated
                                        #      with this segment
        cmpobne.t 0,r4,.pue_430         # Jif segment map table already
                                        #  defined for this segment
.pue_420:
        ldconst TRUE,r3                 # r3 = TRUE indication signifying
                                        #      configuration update required
.pue_430:
        mov     g4,r7                   # Save g4
        ld      pcp1_reg5(r15),g4       # g4 = Primary VRP
        call    cm$set_segment_bit      # set bit(s) in region map
        mov     r7,g4                   # Restore g4
.pue_440:
        lda     ILTBIAS(r15),g1         # move back to PCP2 level
        mov     r3,g0                   # set update flag
        ret                             # return to caller
#
#**********************************************************************
#**********************************************************************
#**********************************************************************
#
#         C O P Y   R E G I S T R A T I O N   R O U T I N E S
#
#**********************************************************************
#**********************************************************************
#**********************************************************************

#**********************************************************************
#
#  NAME: cm$Reg_Suspend
#
#  PURPOSE:
#       To provide a means of registering copy suspend.
#
#  CALLING SEQUENCE:
#       call    cm$Reg_Suspend
#
#  INPUT:
#       g3 = cor address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
cm$Reg_Suspend:
        movl    g0,r14                  # save g0-g1
        ld      K_ficb,r3               # r3 = FICB address
        ld      fi_vcgid(r3),r10        # r10 = my serial #
#
# --- determine if copy source is on another MAG. If it is, register the
#     suspend there.
#
        ld      cor_rssn(g3),g0         # get source MAG S/N
        cmpobe  g0,r10,.regsusp_100     # Jif me

        call    cm$pksnd_suspend        # send the registration
#
# --- determine if copy source is on another MAG. If it is, register the
#     suspend there.
#
.regsusp_100:
        ld      cor_rdsn(g3),g0         # get destination MAG S/N
        cmpobe  g0,r10,.regsusp_200     # Jif me

        call    cm$pksnd_suspend        # send the registration

.regsusp_200:
        movl    r14,g0                  # restore g0-g1
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: cm$Reg_Resume
#
#  PURPOSE:
#       To provide a means of registering copy resume.
#
#  CALLING SEQUENCE:
#       call    cm$Reg_Resume
#
#  INPUT:
#       g3 = cor address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
cm$Reg_Resume:
        movl    g0,r14                  # save g0-g1
        ld      K_ficb,r3               # r3 = FICB address
        ld      fi_vcgid(r3),r10        # r10 = my serial #
#
# --- determine if copy source is on another MAG. If it is, register the
#     resume there.
#
        ld      cor_rssn(g3),g0         # get source MAG S/N
        cmpobe  g0,r10,.regresm_100     # Jif me

        call    cm$pksnd_resume         # send the registration
#
# --- determine if copy source is on another MAG. If it is, register the
#     resume there.
#
.regresm_100:
        ld      cor_rdsn(g3),g0         # get destination MAG S/N
        cmpobe  g0,r10,.regresm_200     # Jif me

        call    cm$pksnd_resume         # send the registration

.regresm_200:
        movl    r14,g0                  # restore g0-g1
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: cm$Reg_Suspend
#
#  PURPOSE:
#       To provide a means of registering copy suspend.
#
#  CALLING SEQUENCE:
#       call    cm$Reg_Suspend
#
#  INPUT:
#       g3 = cor address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
cm$Reg_Change:
        movl    g0,r14                  # save g0-g1
        ld      K_ficb,r3               # r3 = FICB address
        ld      fi_vcgid(r3),r10        # r10 = my serial #
#
# --- determine if copy source is on another MAG. If it is, register the
#     suspend there.
#
        ld      cor_rssn(g3),g0         # get source MAG S/N
        cmpobe  g0,r10,.regchng_100     # Jif me

        call    cm$pksnd_change         # register the change in the COR
#
# --- determine if copy source is on another MAG. If it is, register the
#     suspend there.
#
.regchng_100:
        ld      cor_rdsn(g3),g0         # get destination MAG S/N
        cmpobe  g0,r10,.regchng_200     # Jif me

        call    cm$pksnd_change         # registration the change in the COR

.regchng_200:
        movl    r14,g0                  # restore g0-g1
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: cm$Modify_CORs
#
#  PURPOSE:
#       To provide a means of registering changes to the mirror
#       operations associated with a change in that operation
#       configuration (i.e. after a swap operation)
#
#  CALLING SEQUENCE:
#       call    cm$Modify_CORs
#
#  INPUT:
#       g3 = cor address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
cm$Modify_CORs:
        PushRegs                        # Save all G registers (stack relative)

        ld      cor_srcvdd(g3),r8       # r8 = source VDD
        ld      cor_destvdd(g3),r10     # r10 = destination VDD

#**********************************************************************
#
# --- swap the source and destination
#
#     determine new source device
#
        ld      K_ficb,r3               # r3 = FICB address
        ld      fi_vcgid(r3),g1         # g1 =  new source s/n (my serial #)
        ld      fi_cserial(r3),g11      # g11 = my controller serial #
        ldob    vd_vid(r8),r12          # r12 = new source VDisk # (LSB)
        ldob    vd_vid+1(r8),r13        # r13 = new source VDisk # (MSB)

        ldconst scdt_both,g8            # g8 = default to SCD both

        ld      vd_rdd(r8),r5           # r5 = 1st RDD address
        ldob    rd_type(r5),r6          # r6 = RAID type code
        cmpobne.t rdlinkdev,r6,.mcor_110 # Jif not a VLink type device

        ld      rd_psd(r5),r6           # r6 = assoc. PSD address
        ldos    ps_pid(r6),r7           # r7 = physical ID
        ld      DLM_lddindx[r7*4],r7    # r7 = LDD address
        ldob    ld_class(r7),r6         # r6 = linked device class
        cmpobne.f ldmld,r6,.mcor_110    # Jif not MAG link

        ldconst scdt_local,g8           # g8 = set scd local
        ld      ld_basesn(r7),g1        # g1 =  new source MAG serial #
        ldos    ld_altid(r7),r12        # r12 = alternate ID
        cmpobe  0,r12,.mcor_105         # Jif no alternate ID defined
#
# --- Alternate Id defined for this VLink
#
        shro    8,r12,r13               # r13 = MSB of alternate ID
        clrbit  7,r13,r13               # clear alternate ID flag bit
        ldconst 0xff,r5
        and     r12,r5,r12              # r12 = LSB of alternate ID
        b       .mcor_110
.mcor_105:
        ldob    ld_basevd(r7),r12       # r12 = new source VDisk number
        ldob    ld_basecl(r7),r13       # r13 = new source cluster number
#
# --- set new SCD type
#
.mcor_110:
        ld      cor_scd(g3),r7          # r7 = possible scd
        cmpobe.f 0,r7,.mcor_115         # JIf no scd defined

        stob    g8,scd_type(r7)         # save new type
#
# --- determine new destination device
#
.mcor_115:
        ld      K_ficb,r3               # r3 = FICB address
        ld      fi_vcgid(r3),g2         # g2 =  new destination s/n (my serial #)
        ldob    vd_vid(r10),r14         # r14 = new destination VDisk # (LSB)
        ldob    vd_vid+1(r10),r15       # r15 = new destination VDisk # (MSB)

        ldconst dcdt_both,g9            # g9 = default to DCD both
        ld      vd_rdd(r10),r5          # r5 = 1st RDD address
        ldob    rd_type(r5),r6          # r6 = RAID type code
        cmpobne.t rdlinkdev,r6,.mcor_120 # Jif not a VLink type device

        ld      rd_psd(r5),r6           # r6 = assoc. PSD address
        ldos    ps_pid(r6),r7           # r7 = Physical ID
        ld      DLM_lddindx[r7*4],r7    # r7 = LDD address
        ldob    ld_class(r7),r6         # r6 = device class
        cmpobne.f ldmld,r6,.mcor_120    # Jif not MAG link

        ldconst dcdt_local,g9           # g9 = set DCD local
        ld      ld_basesn(r7),g2        # g2 =  new destination MAG serial #
        ldos    ld_altid(r7),r14        # r14 = alternate ID
        cmpobe  0,r14,.mcor_117         # Jif no alternate ID defined
#
# --- Alternate Id defined for this VLink
#
        shro    8,r14,r15               # r15 = MSB of alternate ID
        clrbit  7,r15,r15               # clear alternate ID flag bit
        ldconst 0xff,r5
        and     r14,r5,r14              # r14 = LSB of alternate ID
        b       .mcor_120
.mcor_117:
        ldob    ld_basevd(r7),r14       # r14 = new destination VDisk number
        ldob    ld_basecl(r7),r15       # r15 = new destination cluster number
#
# --- set new DCD type
#
.mcor_120:
        ld      cor_dcd(g3),r7          # r7 = possible dcd
        cmpobe.f 0,r7,.mcor_122         # JIf no dcd defined

        stob    g9,dcd_type(r7)         # save new type
#
# --- make changes to the COR
#
.mcor_122:
        ld      cor_rssn(g3),g4         # g4 = old source MAG serial #
        st      g1,cor_rssn(g3)         # save new source MAG serial #
        stob    r12,cor_rsvd(g3)        # save new source VDisk number
        stob    r13,cor_rscl(g3)        # save new source cluster number

        ld      cor_rdsn(g3),g5         # g5 = old destination MAG serial #
        st      g2,cor_rdsn(g3)         # save new destination MAG serial #
        stob    r14,cor_rdvd(g3)        # save new destination VDisk number
        stob    r15,cor_rdcl(g3)        # save new destination cluster number
#
# --- make the required changes to registrations
#
        call    CCSM$CheckOwner         # check who's owner of this copy
                                        # g0 = copy owner serial # if known
        cmpobne g0,g11,.mcor_125        # Jif I'm not the owner of this copy
        call    cm$Abort_RCC            # abort possible RCC
        mov     g1,r6                   # save g1
        call    cm$Reg_CfgChange        # register configuration change
        mov     r6,g1                   # restore g1
        cmpobe.t 0,g0,.mcor_125         # JIf status ok

        call    cm$Create_RCC           # retry Register Config Change
        mov     r6,g1                   # restore g1

.mcor_125:

#**********************************************************************
#
# --- determine if there are any other COR's associated with the new
#     source that require changes.
#
.mcor_130:
        ld      vd_scdhead(r8),r6       # r6 = 1st SCD on list

.mcor_135:
        ld      scd_cor(r6),g3          # g3 = COR

        ld      cor_rssn(g3),r3         # is sn the same
        cmpobne g1,r3,.mcor_140         # Jif no

        ldob    cor_rsvd(g3),r3         # is Vdisk # the same
        cmpobne r12,r3,.mcor_140        # Jif no

        ldob    cor_rscl(g3),r3         # is cluster the same
        cmpobe  r13,r3,.mcor_150        # Jif yes
#
# --- this COR has not been modified, so do it.
#
.mcor_140:
        mov     g2,r4                   # save g2
        ld      cor_rssn(g3),g4         # g4 = old source MAG serial #
        ld      cor_rdsn(g3),g2         # g2 = new destination MAG serial #
        st      g1,cor_rssn(g3)         # save new source MAG serial #
        stob    r12,cor_rsvd(g3)        # save new source VDisk number
        stob    r13,cor_rscl(g3)        # save new source cluster number
        mov     g2,g5                   # g5 = old destination MAG serial #

        ld      cor_scd(g3),r7          # r7 = possible scd
        cmpobe.f 0,r7,.mcor_142         # JIf no scd defined

        stob    g8,scd_type(r7)         # save new type
#
# --- make the required changes to registrations
#
.mcor_142:
        call    CCSM$CheckOwner         # check who's owner of this copy
                                        # g0 = copy owner serial # if known
        cmpobne g0,g11,.mcor_145        # Jif I'm not the owner of this copy
        call    cm$Abort_RCC            # abort possible RCC
        mov     g1,r6                   # save g1
        call    cm$Reg_CfgChange        # register configuration change
        mov     r6,g1                   # restore g1
        cmpobe.t 0,g0,.mcor_145         # Jif status ok

        call    cm$Create_RCC           # retry Register Config Change
        mov     r6,g1                   # restore g1

.mcor_145:
        mov     r4,g2                   # restore g2
        b       .mcor_130               # start from beginning of SCD list
#
# --- this COR has been modified, use link to find next COR
#     and Check it
#
.mcor_150:
        ld      scd_link(r6),r6         # get next scd
        cmpobne 0,r6,.mcor_135          # and process it

#**********************************************************************
#
# --- determine if the new source is a destination to any other copy
#

        ld      vd_dcd(r8),r6           # r6 = possible DCD
        cmpobe  0,r6,.mcor_200          # JIf none

        ld      dcd_cor(r6),g3          # g3 = COR
        ld      cor_cm(g3),r3           # r3 = possible cm
        cmpobe.f 0,r3,.mcor_200         # JIf no CM defined
#
# --- make required changes to the COR
#
        mov     g2,r4                   # save g2
        mov     g1,g2
        ld      cor_rssn(g3),g1         # g1 = current source MAG serial #
        mov     g1,g4                   # g4 = current source MAG serial #
        ld      cor_rdsn(g3),g5         # g5 = old destination MAG serial #
        st      g2,cor_rdsn(g3)         # save new destination MAG serial #
        stob    r12,cor_rdvd(g3)        # save new destination VDisk number
        stob    r13,cor_rdcl(g3)        # save new destination cluster number
#
# --- change the DCD type
#
        ld      cor_dcd(g3),r7          # r7 = possible dcd
        cmpobe.f 0,r7,.mcor_175         # JIf no dcd defined

        or      dcdt_local,g8,r3        # change to DCD type codes
        stob    r3,dcd_type(r7)         # save new type
#
# --- make the required changes to registrations
#
.mcor_175:
        call    CCSM$CheckOwner         # check who's owner of this copy
                                        # g0 = copy owner serial # if known
        cmpobne g0,g11,.mcor_180        # Jif I'm not the owner of this copy
        call    cm$Abort_RCC            # abort possible RCC
        mov     g1,r6                   # save g1
        call    cm$Reg_CfgChange        # register configuration changes
        mov     r6,g1                   # restore g1
        cmpobe.t 0,g0,.mcor_180         # Jif status ok

        call    cm$Create_RCC           # retry Register Config Change
#
# --- restore register and save to configuration
#
.mcor_180:
        mov     r4,g2                   # restore g2

#**********************************************************************
#
#     Determine if the new destination is a source to any copies.
#
#     setup registers to use DCD as source
#
.mcor_200:
        mov     g2,g1                   # set up serial #
        movl    r14,r12                 # setup cluster and vdisk

.mcor_230:
        ld      vd_scdhead(r10),r6      # r6 = 1st SCD on list
        cmpobe  0,r6,.mcor_9000         # JIf none

.mcor_235:
        ld      scd_cor(r6),g3          # g3 = COR

        ld      cor_rssn(g3),r3         # is sn the same
        cmpobne g1,r3,.mcor_240         # Jif no

        ldob    cor_rsvd(g3),r3         # is Vdisk # the same
        cmpobne r12,r3,.mcor_240        # Jif no

        ldob    cor_rscl(g3),r3         # is cluster the same
        cmpobe  r13,r3,.mcor_250        # Jif yes
#
# --- this COR has not been modified, so do it.
#
.mcor_240:
        ld      cor_rssn(g3),g4         # g4 = old source MAG serial #
        ld      cor_rdsn(g3),g2         # g2 = new destination MAG serial #
        st      g1,cor_rssn(g3)         # save new source MAG serial #
        stob    r12,cor_rsvd(g3)        # save new source VDisk number
        stob    r13,cor_rscl(g3)        # save new source cluster number
        mov     g2,g5                   # g5 = old destination MAG serial #
#
# --- make changes to the SCD
#
        ld      cor_scd(g3),r7          # r7 = possible scd
        cmpobe.f 0,r7,.mcor_242         # JIf no scd defined

        andnot  dcdt_local,g9,r3        # change to SCD type codes
        stob    r3,scd_type(r7)         # save new type
#
# --- make the required changes to registrations
#
.mcor_242:
        call    CCSM$CheckOwner         # check who's owner of this copy
                                        # g0 = copy owner serial # if known
        cmpobne g0,g11,.mcor_246        # Jif I'm not the owner of this copy
        call    cm$Abort_RCC            # abort possible RCC
        mov     g1,r6                   # save g1
        call    cm$Reg_CfgChange        # register configuration changes
        mov     r6,g1                   # restore g1
        cmpobe.t 0,g0,.mcor_246         # Jif status ok

        call    cm$Create_RCC           # retry Register Config Change
        mov     r6,g1                   # restore g1

.mcor_246:
        b       .mcor_230               # start from beginning of SCD list
#
# --- this COR has been modified, use link to find next COR
#     and Check it
#
.mcor_250:
        ld      scd_link(r6),r6         # get next scd
        cmpobne 0,r6,.mcor_235          # and process it
#
# --- exit
#
.mcor_9000:
        PopRegsVoid                     # Restore all G registers (stack relative)
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: cm$Suspend_CORs
#
#  PURPOSE:
#       To provide a means of suspending COR's that share elements with
#       the current COR. (i.e. before a swap operation)
#
#  CALLING SEQUENCE:
#       call    cm$Suspend_CORs
#
#  INPUT:
#       g3 = cor address
#
#  OUTPUT:
#       g0 = eccopycomp
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
cm$Suspend_CORs:
#
# --- determine if there is a RM associated with this COR
#
        ld      cor_rmaptbl(g3),r4      # id there a region map for this COR
        cmpobe  0,r4,.supcor_exit       # no - br
#
# --- save the environment
#
        PushRegs                        # Save all G registers (stack relative)
#
# --- There is a RM. allocate a PCP and set up it's default structure.
#
c       g1 = get_ilt();                 # Allocate an ILT (PCP)
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
        mov     0,r3                    # r3 = 0

        ldconst pcp1fc_pause,r6         # set pause function code
        ldconst pcwait4swap,r5          # r5 = requested task state

        stob    r5,pcp1_rtstate(g1)     # set requested task state

        st      r3,pcp2_status+ILTBIAS(g1)  # clear out status,function
        st      r3,pcp2_handler+ILTBIAS(g1)# clear Vsync handler
        stob    r6,pcp2_function+ILTBIAS(g1)# save function
#
# --- Suspend any other copy with elements shared with this copy.
#
        ld      cor_srcvdd(g3),r8       # r8  = source vdd
        ld      cor_destvdd(g3),r9      # r9 = destination vdd

#**************
#
#     SOURCE SCD
#
# --- suspend any COR associated with the source device
#
.supcor_100:
        ld      vd_scdhead(r8),r12      # r12 = 1st scd on link list
#
# --- find all elements that share this device as a source
#
.supcor_110:
        ld      scd_cor(r12),r13        # r13 = current COR
        cmpobe  g3,r13,.supcor_190      # JIf orig cor = current cor

        ld      cor_cm(r13),r14         # r14 = cm address
        cmpobe  0,r14,.supcor_130       # Jif no cm defined..
#
# --- there is a CM associated with this COR
#
        ld      cm_pcb(r14),r15         # r15 = PCB address
        ldconst pcwait4swap,r5          # r5 = PCB state to check for
        ldob    pc_stat(r15),r4         # r4 = current PCB state
        cmpobe  r4,r5,.supcor_190       # JIf already been suspended
#
# --- fill in the PCP
#
        st      r13,pcp2_cor+ILTBIAS(g1) # save cor address
        st      r14,pcp1_cm(g1)          # save CM address
        st      r15,pcp1_pcb(g1)         # save PCB address
#
#  --- set up to suspend the COR and wait for the completion
#
        lda     CM$ctlrqstq,g0          # g0 = addr of queue cntrl request
                                        #      routine
        call    K$qw                    # enqueue the ilt and wait

        b       .supcor_100             # start from the beginning
#
# --- there is no CM defined for this COR, so set the cor suspended
#     manually.
#
.supcor_130:
        ldconst cormst_act,r5           # r5 = active map state code
        lda     CM$wp2_suspend,r6       # r6 = suspended write update phase
                                        #      2 handler routine
        ldconst corcrst_remsusp,r7      # r7 = user-suspended/remote reg.

        stob    r5,cor_mstate(r13)      # save new map state in COR
        st      r6,scd_p2hand(r12)      # save phase 2 update handler routine
                                        #  in SCD
        stob    r7,cor_crstate(r13)     # save new copy reg. state in COR
        movq    g0,r4                   # save g0-g3
        mov     r13,g3                  # g3 = COR address
        call    CM$mmc_sflag            # process suspended flag for MMC
        call    CCSM$cs_chged_w_msg     # generate a copy state changed event
#
# --- inform the CM of the good news
#
        ld      cor_rcsn(g3),g0         # g0 = copy MAG serial #
        call    CM$pkop_dmove           # pack a copy device moved datagram
                                        # g1 = Copy Device Moved datagram ILT at nest level 2
        ldconst 4,g0                    # g0 = error retry count
        call    DLM$just_senddg         # send datagram to copy MAG
        movq    r4,g0                   # restore g0-g3
#
# --- find next SCD in list
#
.supcor_190:
        ld      scd_link(r12),r12       # find next SCD
        cmpobne 0,r12,.supcor_110       # JIf another cor
#**************
#
#     SOURCE DCD
#
# --- all SCD's have been checked, now check the DCD
#
        ld      vd_dcd(r8),r12          # r12 = possible dcd
        cmpobe  0,r12,.supcor_300       # Jif none
#
# --- determine if there is a CM associated with this COR
#
        ld      scd_cor(r12),r13        # r13 = current COR
        ld      cor_cm(r13),r14         # r14 = cm address
        cmpobe  0,r14,.supcor_230       # Jif no cm defined..
#
# --- There is a CM, fill in the PCP
#
        ld      cm_pcb(r14),r15         # r15 = PCB address
        st      r13,pcp2_cor+ILTBIAS(g1) # save cor address
        st      r14,pcp1_cm(g1)          # save CM address
        st      r15,pcp1_pcb(g1)         # save PCB address
#
#  --- set up to suspend the COR and wait for the completion
#
        lda     CM$ctlrqstq,g0          # g0 = addr of queue cntrl request
                                        #      routine
        call    K$qw                    # enqueue the ilt and wait
        b       .supcor_300             # continue
#
# --- there is no CM defined for this COR, so set the cor suspended
#     manually.
#
.supcor_230:
        ldconst cormst_act,r5           # r5 = active map state code
        lda     CM$wp2_suspend,r6       # r6 = suspended write update phase
                                        #      2 handler routine
        ldconst corcrst_remsusp,r7      # r7 = user-suspended/remote reg.

        stob    r5,cor_mstate(r13)      # save new map state in COR
        st      r6,scd_p2hand(r12)      # save phase 2 update handler routine
                                        #  in SCD
        stob    r7,cor_crstate(r13)     # save new copy reg. state in COR
        movq    g0,r4                   # save g0-g3
        mov     r13,g3                  # g3 = COR address
        call    CM$mmc_sflag            # process suspended flag for MMC
        call    CCSM$cs_chged_w_msg     # generate a copy state changed event
#
# --- inform the CM of the good news
#
        ld      cor_rcsn(g3),g0         # g0 = copy MAG serial #
        call    CM$pkop_dmove           # pack a copy device moved datagram
                                        # g1 = Copy Device Moved datagram ILT at nest level 2
        ldconst 4,g0                    # g0 = error retry count
        call    DLM$just_senddg         # send datagram to copy MAG
        movq    r4,g0                   # restore g0-g3

#**************
#
#     DESTINATION SCD
#
# --- suspend any COR that use the destination device as a source
#
.supcor_300:
        ld      vd_scdhead(r9),r12      # r12 = 1st scd on link list
        cmpobe  0,r12,.supcor_400       # JIf none
#
# --- find all elements that share this device as a source
#
.supcor_310:
        ld      scd_cor(r12),r13        # r13 = current COR
        ld      cor_cm(r13),r14         # r14 = cm address
        cmpobe  0,r14,.supcor_330       # Jif no cm defined..
#
# --- there is a CM associated with this COR
#
        ld      cm_pcb(r14),r15         # r15 = PCB address
        ldconst pcwait4swap,r5          # r5 = PCB state to check for
        ldob    pc_stat(r15),r4         # r4 = current PCB state
        cmpobe  r4,r5,.supcor_390       # JIf already been suspended
#
# --- fill in the PCP
#
        st      r13,pcp2_cor+ILTBIAS(g1) # save cor address
        st      r14,pcp1_cm(g1)          # save CM address
        st      r15,pcp1_pcb(g1)         # save PCB address
#
#  --- set up to suspend the COR and wait for the completion
#
        lda     CM$ctlrqstq,g0          # g0 = addr of queue cntrl request
                                        #      routine
        call    K$qw                    # enqueue the ilt and wait

        b       .supcor_300             # start from the beginning
#
# --- there is no CM defined for this COR, so set the cor suspended
#     manually.
#
.supcor_330:
        ldconst cormst_act,r5           # r5 = active map state code
        lda     CM$wp2_suspend,r6       # r6 = suspended write update phase
                                        #      2 handler routine
        ldconst corcrst_remsusp,r7      # r7 = user-suspended/remote reg.

        stob    r5,cor_mstate(r13)      # save new map state in COR
        st      r6,scd_p2hand(r12)      # save phase 2 update handler routine
                                        #  in SCD
        stob    r7,cor_crstate(r13)     # save new copy reg. state in COR
        movq    g0,r4                   # save g0-g3
        mov     r13,g3                  # g3 = COR address
        call    CM$mmc_sflag            # process suspended flag for MMC
        call    CCSM$cs_chged_w_msg     # generate a copy state changed event
#
# --- inform the CM of the good news
#
        ld      cor_rcsn(g3),g0         # g0 = copy MAG serial #
        call    CM$pkop_dmove           # pack a copy device moved datagram
                                        # g1 = Copy Device Moved datagram ILT at nest level 2
        ldconst 4,g0                    # g0 = error retry count
        call    DLM$just_senddg         # send datagram to copy MAG
        movq    r4,g0                   # restore g0-g3
#
# --- find next SCD in list
#
.supcor_390:
        ld      scd_link(r12),r12       # find next SCD
        cmpobne 0,r12,.supcor_310       # JIf another cor
#
# --- release the PCP
#
.supcor_400:
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
#
# --- restore the environment
#
        PopRegs                         # Restore g1 thru g14
#
# --- exit
#
.supcor_exit:
        ldconst eccopycomp,g0           # return copy completed status (NON ZERO)
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: cm$Abort_RCC
#
#  PURPOSE:
#       To provide a means of aborting a RCC to retrying the registration
#       of changes to the copy/mirror configuration associated with a COR.
#
#  CALLING SEQUENCE:
#       call    cm$Abort_RCC
#
#  INPUT:
#       g3 = COR address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
cm$Abort_RCC:
        mov     0,r3                    # r3 = 0
        ld      cor_RCC(g3),r8          # is there a RCC registered ?
        cmpobe  0,r8,.abortRCC_100      # JIf no

        ldob    rcc1_flags(r8),r4       # setup to abort this RCC
        setbit  rcc1flg_abort,r4,r4
        stob    r4,rcc1_flags(r8)
        st      r3,cor_RCC(g3)          # clear registration

.abortRCC_100:
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: cm$Create_RCC
#
#  PURPOSE:
#       To provide a means of creating a RCC to retrying the registration
#       of changes to the copy/mirror configuration associated with a COR.
#
#  CALLING SEQUENCE:
#       call    cm$Create_RCC
#
#  INPUT:
#       g1 = New source S/N
#       g2 = New destination S/N
#       g3 = COR address
#       g4 = Current Source S/N
#       g5 = Current Destination S/N
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Reg. g1 are destroyed
#
#**********************************************************************
#
cm$Create_RCC:
        call    CM$Build_RCC            # build an rrc ...
        call    cm$Queue_RCC            #   ... and place it on queue
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: cm$Queue_RCC
#
#  PURPOSE:
#       To provide a means of en queueing a RCC.
#
#  CALLING SEQUENCE:
#       call    cm$Queue_RCC
#
#  INPUT:
#       g1 = RCC at 1st level
#       g3 = COR
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Reg. g1 is destroyed
#
#**********************************************************************
#
cm$Queue_RCC:
        ld      cor_cm(g3),r8           # r8 = cm address
        cmpobne  0,r8,.queRCC_100       # JIf there is a CM
#
# --- no CM, release RRC
#
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT (RCC)
        b       .queRCC_exit            # exit
#
# --- there is a CM, increment PIC
#
.queRCC_100:
        ldob    cm_PIC(r8),r4           # r4 = PIC
        addo    1,r4,r4                 # increment PIC
        stob    r4,cm_PIC(r8)           # save new value
#
# --- abort possible RCC registered in COR
#
        call    cm$Abort_RCC            # abort previous RCC
        st      g1,cor_RCC(g3)          # and register new one
#
# --- Link it onto the end of the queue
#
        lda     cm_RCC_qu,r11           # load queue address
        b       K$cque                  # enqueue RCC and return to caller

.queRCC_exit:
        ret                             # return to call
#
#**********************************************************************
#
#  NAME: CM$Build_RCC
#
#  PURPOSE:
#       To provide a means of building RCC.
#
#  CALLING SEQUENCE:
#       call    CM$Build_RCC
#
#  INPUT:
#       g1 = New source S/N
#       g2 = New destination S/N
#       g3 = COR address
#       g4 = Current Source S/N
#       g5 = Current Destination S/N
#
#  OUTPUT:
#       g1 = RRC at 1st level
#
#  REGS DESTROYED:
#       Reg. g1 is destroyed
#
#**********************************************************************
#
CM$Build_RCC:
        mov     0,r3                    # r3 = 0
        mov     g1,r4                   # save g1
c       g1 = get_ilt();                 # Allocate an ILT (PCP)
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
        mov     g1,r8                   # r8 = RCC lvl1
        mov     r4,g1                   # restore g1
#
# --- save the required registers
#
        lda     ILTBIAS(r8),r9          # bump to next level
        stq     g0,rcc2_g0(r9)          # save g0-g3
        stl     g4,rcc2_g4(r9)          # save g4-g5
#
# --- clear some stuff
#
        stob    r3,rcc1_flags(r8)       # clear flags
        st      r3,rcc1_lastretry(r8)   # clear time stamp
        st      r3,rcc2_status(r9)      # clear status word
        mov     r8,g1                   # g1 = RCC at 1st level
        ret                             # return to call
#
#**********************************************************************
#
#  NAME: cm$Retry_RCC
#
#  PURPOSE:
#       To provide a means of retrying the registration of changes to
#       the copy/mirror configuration associated with a COR.
#
#  CALLING SEQUENCE:
#       fork    cm$Retry_RCC
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
cm$Retry_RCC:
        b       .retryRCC_110           # start the loop
#
# --- Set this process to not ready
#
.retryRCC_100:
        ld      cm_RCC_qu+qu_pcb,r15    # r15 = task PCB
        ldconst pcnrdy,r4               # Set this process to not ready
        stob    r4,pc_stat(r15)
        call    K$xchang                # Exchange processes
#
# --- get a timestamp for this processing cycle
#
.retryRCC_110:
        ld      K_ii+ii_time,r7         # r7 = merge timestamp
#
# --- get 1st entry on queue
#
.retryRCC_120:
        ld      cm_RCC_qu,r8            # r8 = 1st RCC on queue

        cmpobe  0,r8,.retryRCC_100      # JIf none - sleep
#
# --- determine if this pcp had been processed this cycle...
#
.retryRCC_130:
        ld      rcc1_lastretry(r8),r4   # get last retry timestamp
        cmpobne r4,r7,.retryRCC_200     # jif not yet processed

        ld      il_fthd(r8),r8          # r8 = next pcp in list
        cmpobne 0,r8,.retryRCC_130      # JIf there is one
#
# --- wait to retry again
#
        ldconst retryRCC_to,g0          # g0 = time delay period (in msec.)
        call    K$twait                 # wait for time delay
        b       .retryRCC_110           # try again
#
# --- this RCC has not been processed
#
#     load up the required registers, set cycle time, and determine
#     if this RCC is to be aborted
#
.retryRCC_200:
        lda     ILTBIAS(r8),r9          # bump to next RCC level

        ldq     rcc2_g0(r9),g0          # load g0-g3
        ldl     rcc2_g4(r9),g4          # load g4-g5

        st      r7,rcc1_lastretry(r8)   # save new timestamp

        ldob    rcc1_flags(r8),r11      # has this RCC been aborted ???
        bbs     rcc1flg_abort,r11,.retryRCC_340 # Jif yes
#
# --- retry the register the configuration change
#
        ldob    cor_cstate(g3),r4
        cmpoble corcrst_active,r4,.retryRCC_220 # Jif initialized
#
# --- copy  has never been initialized
#
        ldconst 0,g1                    # g1 = register type code
        call    CM$regcopy              # register the copy operation where
                                        #  appropriate
        stob    g0,rcc2_status(r9)      # save status
        cmpobe.t 0,g0,.retryRCC_300     # Jif copy operation registration
                                        #  was successful
        cmpobe.f 1,g0,.retryRCC_120     # Jif may still be valid
#
# --- copy must be aborted
#
        call    cm$poll_term            # send PCP to abort the copy
        b       .retryRCC_300           # remove from queue and release ILT
#
# --- copy is initialized and is active
#
.retryRCC_220:
        call    cm$Reg_CfgChange        # register configuration changes
        stob    g0,rcc2_status(r9)      # save status
        cmpobne.t 0,g0,.retryRCC_120    # Jif error status
#
# --- everything is cool (status was good). remove RCC from queue,
#     unregister RCC from COR,decrement poll inhibit counter,
#     force a poll, and return RCC.
#
.retryRCC_300:
#
# --- unregister RCC from COR
#
        ldconst 0,r3                    # r3 = 0
        st      r3,cor_RCC(g3)          # clear registration
#
# --- Make sure copy registration active.
#
        ldconst corcrst_active,r4       # r4 = registration active
        stob    r4,cor_crstate(g3)      # save new registration state
#
# --- Decrement the Poll Inhibit Counter (PIC)
#
.retryRCC_340:
        ld      cor_cm(g3),g4           # get CM address
        cmpobe  0,g4,.retryRCC_350      # Jif no

        ldob    cm_PIC(g4),r4           # load PIC
        cmpobe  0,r4,.retryRCC_345      # Jif already zero

        subo    1,r4,r4                 # decrement counter
        stob    r4,cm_PIC(g4)           # save new PIC
#
# --- force a poll
#
.retryRCC_345:
        call    cm$force_LP             # force a local poll
#
# --- Remove RCC I/O ILT from active queue
#
.retryRCC_350:
        ldl     il_fthd(r8),r14         # r14 = forward thread of ILT
                                        # r15 = backward thread of ILT
        lda     cm_RCC_qu,r13           # r13 = base address of active queue
        st      r14,il_fthd(r15)        # put forward thread from removed ILT
                                        #  as forward thread of previous ILT
        cmpobne.t 0,r14,.retryRCC_360   # Jif non-zero forward thread
        mov     r13,r14                 # make base of queue the forward
                                        #  thread
        cmpobne.t r13,r15,.retryRCC_360 # Jif backward thread <> base of
                                        #  queue
        mov     0,r15                   # queue is now empty!

.retryRCC_360:
        st      r15,il_bthd(r14)        # put backward thread from removed
        ld      cm_RCC_qu+qu_qcnt,r4    # decrement queue count
        subo    1,r4,r4
        st      r4,cm_RCC_qu+qu_qcnt
#
# --- release RCC
#
        mov     r8,g1                   # move to correct register
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT (RCC)
#
# --- save configuration
#
        ld      cor_RCC(g3),r3          # is there still a RCC registered
#
# --- try next record
#
        b       .retryRCC_120           # try next entry
#
#**********************************************************************
#
#  NAME: cm$Reg_CfgChange
#
#  PURPOSE:
#       To provide a means of registering changes to the copy/mirror
#       configuration associated with a COR.
#
#  CALLING SEQUENCE:
#       call    cm$Reg_CfgChange
#
#  INPUT:
#       g1 = New source S/N
#       g2 = New destination S/N
#       g3 = COR address
#       g4 = Current Source S/N
#       g5 = Current Destination S/N
#
#  OUTPUT:
#       g0 = completion status.
#          If g0 = 0 then g1 is unchanged
#          If g0 <> 0 then g1 is EC1
#
#  REGS DESTROYED:
#       Reg. g0-g1 are destroyed
#
#**********************************************************************
#
cm$Reg_CfgChange:
        ld      K_ficb,r3               # r3 = FICB address
        ld      fi_vcgid(r3),r10        # r10 = my serial #
#
# --- determine if current source still used. If not, terminate registration
#     of current source.
#
        cmpobe  g4,r10,.regcfgchg_100   # Jif current source = my s/n
        cmpobe  g4,g1,.regcfgchg_100    # Jif current source = new source
        cmpobe  g4,g2,.regcfgchg_100    # Jif current source = new dest

        mov     g4,g0                   # set mag s/n
        mov     g1,r15                  # save g1
        call    cm$pksnd_term           # pack and send a termination DG
        mov     r15,g1                  # restore g1
#        cmpobne 0,g0,.regcfgchg_exit     # Jif error
#
# --- determine if current destination is still in use. If not,
#     terminate registration of current destination.
#
.regcfgchg_100:
        cmpobe  g5,r10,.regcfgchg_200   # Jif current dest = my s/n
        cmpobe  g5,g1,.regcfgchg_200    # Jif current dest = new source
        cmpobe  g5,g2,.regcfgchg_200    # Jif current dest = new dest

        mov     g5,g0                   # set mag s/n
        mov     g1,r15                  # save g1
        call    cm$pksnd_term           # pack and send a termination DG
        mov     r15,g1                  # restore g1
#        cmpobne 0,g0,.regcfgchg_exit     # Jif error
#
# --- determine if new source has any registration. If not,
#     create registration of new source.
#
.regcfgchg_200:
        cmpobe  g1,r10,.regcfgchg_300   # Jif new source = my s/n
        cmpobe  g1,g4,.regcfgchg_300    # Jif new source = current source
        cmpobe  g1,g5,.regcfgchg_300    # Jif new source = current dest

        ld      cor_rssn(g3),r4         # get current source from cor
        cmpobne r4,g1,.regcfgchg_300    # jif no the same
        mov     g1,g0                   # set mag s/n
        call    cm$pksnd_create         # pack and send a create DG
        cmpobne 0,g0,.regcfgchg_exit     # Jif error
#
# --- determine if new dest has any registration. If not,
#     create registration of new dest.
#
.regcfgchg_300:
        cmpobe  g2,r10,.regcfgchg_400   # Jif new dest = my s/n
        cmpobe  g2,g4,.regcfgchg_400    # Jif new dest = current source
        cmpobe  g2,g5,.regcfgchg_400    # Jif new dest = current dest
        cmpobe  g2,g1,.regcfgchg_400    # Jif new dest = new source

        ld      cor_rdsn(g3),r4         # get current dest from cor
        cmpobne r4,g2,.regcfgchg_400    # jif no the same
        mov     g2,g0                   # set mag s/n
        call    cm$pksnd_create         # pack and send a create DG
        cmpobne 0,g0,.regcfgchg_exit     # Jif error
#
# --- determine if new source has any changes to it's registration.
#     If so, register those changes to new source.
#
.regcfgchg_400:
        cmpobe  g1,r10,.regcfgchg_500   # Jif new source = my s/n
        cmpobe  g1,g4,.regcfgchg_410    # Jif new source = current source
        cmpobne g1,g5,.regcfgchg_500    # Jif new source <> current dest

.regcfgchg_410:
        ld      cor_rssn(g3),r4         # get current source from cor
        cmpobne r4,g1,.regcfgchg_500    # jif no the same
        mov     g1,g0                   # set mag s/n
        mov     g1,r15                  # save mag S/N for error recovery
        call    cm$pksnd_change         # pack and send a change DG
        cmpobe 0,g0,.regcfgchg_500      # Jif no error
#
#     determine if the error is a "destination server level error". If
#     so, determine if the EC1 is a "copy operation not defined". If it
#     is, reissue this change as a "Create Copy".
#
        cmpobne dg_st_srvr,g0,.regcfgchg_exit
        cmpobne dgec1_srvr_nocopy,g1,.regcfgchg_exit

        mov     r15,g0                  # set mag s/n
        call    cm$pksnd_create         # pack and send a create DG
        cmpobne 0,g0,.regcfgchg_exit    # Jif error
        mov     r15,g1                  # restore new source s/n
#
# --- determine if new dest has any changes to it's registration.
#     If so, register those changes to new dest.
#
.regcfgchg_500:
        cmpobe  g2,r10,.regcfgchg_600   # Jif new dest = my s/n
        cmpobe  g2,g4,.regcfgchg_510    # Jif new dest = current source
        cmpobne g2,g5,.regcfgchg_600    # Jif new dest <> current dest

.regcfgchg_510:
        cmpobe  g2,g1,.regcfgchg_600    # Jif new dest = new source

        ld      cor_rdsn(g3),r4         # get current dest from cor
        cmpobne r4,g2,.regcfgchg_600    # jif no the same
        mov     g2,g0                   # set mag s/n
        call    cm$pksnd_change         # pack and send a change DG
        cmpobe  0,g0,.regcfgchg_600      # Jif good status
#
#     determine if the error is a "destination server level error". If
#     so, determine if the EC1 is a "copy operation not defined". If it
#     is, reissue this change as a "Create Copy".
#
        cmpobne dg_st_srvr,g0,.regcfgchg_exit
        cmpobne dgec1_srvr_nocopy,g1,.regcfgchg_exit

        mov     g2,g0                   # set mag s/n
        call    cm$pksnd_create         # pack and send a create DG
        cmpobne 0,g0,.regcfgchg_exit    # Jif error
#
# --- If we get here, either nothing happened or everything went OK
#
.regcfgchg_600:
        mov     0,g0                    # set good status
#
# --- Exit
#
.regcfgchg_exit:
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: cm$pksnd_term
#
#  PURPOSE:
#       To provide a means of packing and sending a registration
#       termination datagram message to the specified MAGNITUDE.
#
#  DESCRIPTION:
#       This routine packs and send a terminate copy operation datagram
#       message to the specified MAGNITUDE. If an error occurs, it attempts
#       to perform some level of error recovery, but if still unsuccessful
#       will return to the calling routine. This routine relies on logic
#       on the destination MAGNITUDE to terminate the copy operation at some
#       later time if necessary in the event this routine cannot successfully
#       terminate the copy operation.
#
#  CALLING SEQUENCE:
#       call    cm$pksnd_term
#
#  INPUT:
#       g0 = MAG S/N
#       g3 = COR address
#
#  OUTPUT:
#       g0 = completion status.
#          If g0 = 0 then g1 is unchanged
#          If g0 <> 0 then g1 is EC1
#
#  REGS DESTROYED:
#       Reg. g0-g1 is destroyed
#
#**********************************************************************
#
cm$pksnd_term:
        movl    g0,r14                  # save g0-g1
        ldconst 4,r13                   # r13 = error retry count

.pksndtx_100:
        mov     r14,g0                  # g0 = MAG serial number to send to
        call    cm$pkop_term            # Pack establish copy oper datagram
                                        # g1 = datagram ILT at nest level 2
        lda     DLM$send_dg,g0          # g0 = datagram service provider routine
        call    K$qw                    # Queue request w/wait
        ld      dsc2_rshdr_ptr(g1),r7   # r7 = local response header address
        lda     -dsc1_ulvl(g1),g1       # g1 = datagram ILT at nest level 1
        ldob    dgrs_status(r7),g0      # g0 = request completion status
        ldob    dgrs_ec1(r7),r6         # r6 = error code byte #1
        call    DLM$put_dg              # deallocate datagram ILT

        mov     r15,g1                  # restore g1

        cmpobe  0,g0,.pksndtx_1000      # jif status is zero

        subo    1,r13,r13               # dec. error retry count
        cmpobne.t 0,r13,.pksndtx_100    # Jif error retry count has not
                                        #  expired
        mov     r6,g1                   # g1 = ec1 code

.pksndtx_1000:
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: cm$pksnd_create
#
#  PURPOSE:
#       To provide a means of packing and sending a registration
#       create datagram message to the specified MAGNITUDE.
#
#  DESCRIPTION:
#       This routine does a best effort to successfully send a define
#       new copy operation datagram to the specified MAGNITUDE. In the
#       event an error is returned in the response datagram message, it
#       processes some of the possible and retries them if appropriate.
#       If the errors returned are not retryable or the error retry count
#       expires, this routine returns the response status code and error
#       code byte #1 (ec1) to the calling routine for further error
#       processing.
#
#  CALLING SEQUENCE:
#       call    cm$pksnd_create
#
#  INPUT:
#       g0 = MAG S/N
#       g3 = COR address
#
#  OUTPUT:
#       g0 = completion status.
#          If g0 = 0 then g1 is unchanged
#          If g0 <> 0 then g1 is EC1
#
#  REGS DESTROYED:
#       Reg. g0-g1 is destroyed
#
#**********************************************************************
#
cm$pksnd_create:
        movl    g0,r14                  # save g0-g1
        ldconst 4,r13                   # r13 = error retry count

.pksndcx_100:
        mov     r14,g0                  # g0 = MAG serial number to send to
        call    cm$pkop_new             # Pack new copy oper datagram
                                        # g1 = datagram ILT at nest level 2
        lda     DLM$send_dg,g0          # g0 = datagram service provider routine
        call    K$qw                    # Queue request w/wait
        ld      dsc2_rshdr_ptr(g1),r7   # r7 = local response header address
        lda     -dsc1_ulvl(g1),g1       # g1 = datagram ILT at nest level 1
        ldob    dgrs_status(r7),g0      # g0 = request completion status
        ldob    dgrs_ec1(r7),r6         # r6 = error code byte #1
        call    DLM$put_dg              # deallocate datagram ILT

        mov     r15,g1                  # restore g1

        cmpobe  0,g0,.pksndcx_1000      # jif status is zero
        cmpobe.t dg_st_srvr,g0,.pksndcx_300 # Jif dest. server level error

.pksndcx_150:
        subo    1,r13,r13               # dec. error retry count
        cmpobne.t 0,r13,.pksndcx_100    # Jif error retry count not expired

.pksndcx_200:
        mov     r6,g1                   # g1 = EC1 code
        b       .pksndcx_1000

.pksndcx_300:
        cmpobe.f dgec1_srvr_nosvlink,r6,.pksndcx_200 # Jif source VLink not
                                        #  established

        cmpobe.f dgec1_srvr_nodvlink,r6,.pksndcx_200 # Jif dest. VLink not
                                        #  established

        cmpobe.f dgec1_srvr_inuse,r6,.pksndcx_200 # Jif specified copy device
                                        #  in use
        b       .pksndcx_150

.pksndcx_1000:
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: cm$pksnd_change
#
#  PURPOSE:
#       To provide a means of packing and sending a registration
#       change datagram message to the specified MAGNITUDE.
#
#  DESCRIPTION:
#       This routine does a best effort to successfully send an establish
#       copy operation state datagram to the specified MAGNITUDE. In the
#       event an error is returned in the response datagram message, it
#       processes some of the possible and retries them if appropriate.
#       If the errors returned are not retryable or the error retry count
#       expires, this routine returns the response status code and error
#       code byte #1 (ec1) to the calling routine for further error
#       processing.
#
#  CALLING SEQUENCE:
#       call    cm$pksnd_change
#
#  INPUT:
#       g0 = MAG S/N
#       g3 = COR address
#
#  OUTPUT:
#       g0 = completion status.
#          If g0 = 0 then g1 is unchanged
#          If g0 <> 0 then g1 is EC1
#
#  REGS DESTROYED:
#       Reg. g0-g1 is destroyed
#
#**********************************************************************
#
cm$pksnd_change:
        movl    g0,r14                  # save g0-g1
        ldconst 4,r13                   # r13 = error retry count

.pksndchgx_100:
        mov     r14,g0                  # g0 = MAG serial number to send to
        call    cm$pkop_state           # Pack change copy oper datagram
                                        # g1 = datagram ILT at nest level 2
        lda     DLM$send_dg,g0          # g0 = datagram service provider routine
        call    K$qw                    # Queue request w/wait
        ld      dsc2_rshdr_ptr(g1),r7   # r7 = local response header address
        lda     -dsc1_ulvl(g1),g1       # g1 = datagram ILT at nest level 1
        ldob    dgrs_status(r7),g0      # g0 = request completion status
        ldob    dgrs_ec1(r7),r6         # r6 = error code byte #1
        call    DLM$put_dg              # deallocate datagram ILT

        mov     r15,g1                  # restore g1

        cmpobe  0,g0,.pksndchgx_1000    # jif status is zero
        cmpobe.t dg_st_srvr,g0,.pksndchgx_300 # Jif dest. server level error

.pksndchgx_150:
        subo    1,r13,r13               # dec. error retry count
        cmpobne.t 0,r13,.pksndchgx_100  # Jif error retry count not expired

.pksndchgx_200:
        mov     r6,g1                   # g1 = EC1 code
        b       .pksndchgx_1000

.pksndchgx_300:
        cmpobe.f dgec1_srvr_nosvlink,r6,.pksndchgx_200 # Jif source VLink not
                                        #  established
        cmpobe.f dgec1_srvr_nodvlink,r6,.pksndchgx_200 # Jif dest. VLink not
                                        #  established
        cmpobe.f dgec1_srvr_inuse,r6,.pksndchgx_200 # Jif specified copy device
                                        #  in use
        b       .pksndchgx_150

.pksndchgx_1000:
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: cm$pksnd_suspend
#
#  PURPOSE:
#       To provide a means of packing and sending a suspend copy
#       datagram message to the specified MAGNITUDE.
#
#  DESCRIPTION:
#       This routine packs and send a suspend copy datagram
#       message to the specified MAGNITUDE. If an error occurs, it attempts
#       to perform some level of error recovery, but if still unsuccessful
#       will return to the calling routine. This routine relies on logic
#       on the destination MAGNITUDE to terminate the copy operation at some
#       later time if necessary in the event this routine cannot successfully
#       terminate the copy operation.
#
#  CALLING SEQUENCE:
#       call    cm$pksnd_suspend
#
#  INPUT:
#       g0 = MAG S/N
#       g3 = COR address
#
#  OUTPUT:
#       g0 = completion status.
#          If g0 = 0 then g1 is unchanged
#          If g0 <> 0 then g1 is EC1
#
#  REGS DESTROYED:
#       Reg. g0-g1 is destroyed
#
#**********************************************************************
#
cm$pksnd_suspend:
        movl    g0,r14                  # save g0-g1
        ldconst 4,r13                   # r13 = error retry count

.pksndsusp_100:
        mov     r14,g0                  # g0 = MAG serial number to send to
        call    cm$pkop_susp            # Pack suspend copy oper datagram
                                        # g1 = datagram ILT at nest level 2
        lda     DLM$send_dg,g0          # g0 = datagram service provider routine
        call    K$qw                    # Queue request w/wait
        ld      dsc2_rshdr_ptr(g1),r7   # r7 = local response header address
        lda     -dsc1_ulvl(g1),g1       # g1 = datagram ILT at nest level 1
        ldob    dgrs_status(r7),g0      # g0 = request completion status
        ldob    dgrs_ec1(r7),r6         # r6 = error code byte #1
        call    DLM$put_dg              # deallocate datagram ILT

        mov     r15,g1                  # restore g1

        cmpobe  0,g0,.pksndsusp_1000    # jif status is zero
        cmpobe.t dg_st_srvr,g0,.pksndsusp_300 # Jif dest. server level error

.pksndsusp_150:
        subo    1,r13,r13               # dec. error retry count
        cmpobne.t 0,r13,.pksndsusp_100  # Jif error retry count not expired

.pksndsusp_200:
        mov     r6,g1                   # g1 = EC1 code
        b       .pksndsusp_1000

.pksndsusp_300:
        cmpobe.f dgec1_srvr_nosvlink,r6,.pksndsusp_200 # Jif source VLink not
                                        #  established
        cmpobe.f dgec1_srvr_nodvlink,r6,.pksndsusp_200 # Jif dest. VLink not
                                        #  established
        cmpobe.f dgec1_srvr_inuse,r6,.pksndsusp_200 # Jif specified copy device
                                        #  in use
        b       .pksndsusp_150

.pksndsusp_1000:
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: cm$pksnd_resume
#
#  PURPOSE:
#       To provide a means of packing and sending a resume copy
#       datagram message to the specified MAGNITUDE.
#
#  DESCRIPTION:
#       This routine packs and send a resume copy datagram
#       message to the specified MAGNITUDE. If an error occurs, it attempts
#       to perform some level of error recovery, but if still unsuccessful
#       will return to the calling routine. This routine relies on logic
#       on the destination MAGNITUDE to terminate the copy operation at some
#       later time if necessary in the event this routine cannot successfully
#       terminate the copy operation.
#
#  CALLING SEQUENCE:
#       call    cm$pksnd_resume
#
#  INPUT:
#       g0 = MAG S/N
#       g3 = COR address
#
#  OUTPUT:
#       g0 = completion status.
#          If g0 = 0 then g1 is unchanged
#          If g0 <> 0 then g1 is EC1
#
#  REGS DESTROYED:
#       Reg. g0-g1 is destroyed
#
#**********************************************************************
#
cm$pksnd_resume:
        movl    g0,r14                  # save g0-g1
        ldconst 4,r13                   # r13 = error retry count

.pksndresm_100:
        mov     r14,g0                  # g0 = MAG serial number to send to
        call    cm$pkop_resume          # Pack resume copy oper datagram
                                        # g1 = datagram ILT at nest level 2
        lda     DLM$send_dg,g0          # g0 = datagram service provider routine
        call    K$qw                    # Queue request w/wait
        ld      dsc2_rshdr_ptr(g1),r7   # r7 = local response header address
        lda     -dsc1_ulvl(g1),g1       # g1 = datagram ILT at nest level 1
        ldob    dgrs_status(r7),g0      # g0 = request completion status
        ldob    dgrs_ec1(r7),r6         # r6 = error code byte #1
        call    DLM$put_dg              # deallocate datagram ILT

        mov     r15,g1                  # restore g1

        cmpobe  0,g0,.pksndresm_1000    # jif status is zero
        cmpobe.t dg_st_srvr,g0,.pksndresm_300 # Jif dest. server level error

.pksndresm_150:
        subo    1,r13,r13               # dec. error retry count
        cmpobne.t 0,r13,.pksndresm_100  # Jif error retry count not expired

.pksndresm_200:
        mov     r6,g1                   # g1 = EC1 code
        b       .pksndresm_1000

.pksndresm_300:
        cmpobe.f dgec1_srvr_nosvlink,r6,.pksndresm_200 # Jif source VLink not
                                        #  established
        cmpobe.f dgec1_srvr_nodvlink,r6,.pksndresm_200 # Jif dest. VLink not
                                        #  established
        cmpobe.f dgec1_srvr_inuse,r6,.pksndresm_200 # Jif specified copy device
                                        #  in use
        b       .pksndresm_150

.pksndresm_1000:
        ret                             # return to caller
#
#**********************************************************************
#**********************************************************************
#**********************************************************************
#
#   L O C A L   R E G I O N   M A P   M E R G E  R O U T I N E S
#
#**********************************************************************
#**********************************************************************
#**********************************************************************
#**********************************************************************
#
#  NAME: cm$local_merge
#
#  CALLING SEQUENCE:
#       call    cm$local_merge
#
#  INPUT:
#       g3 = COR address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
cm$local_merge:
        PushRegs                        # Save all G registers (stack relative)
#
# --- get pointer to original RM
#
        ld      cor_rmaptbl(g3),g4      # g4 = original region map
        cmpobe.f 0,g4,.lclmrg_exit      # Jif no map defined
#
# --- get a timestamp for this merge
#
        ld      K_ii+ii_time,r9         # r9 = merge timestamp

#**************
#
#     SOURCE SCD
#
# --- get the VDD of the source device
#
        ld      cor_srcvdd(g3),r7       # r7 = source vdd
#
# --- get 1st SCD from VDD SCD link list
#
.lclmrg_100:
        ld      vd_scdhead(r7),r8       # r8 = 1st SCD on list
        cmpobe.f 0,r8,.lclmrg_200       # JIf no SCD's defined
#
# --- determine if the current COR has already been processed
#
.lclmrg_150:
        ldconst 0,g6                    # clear current RM

        ld      scd_cor(r8),g5          # g5 = COR for this SCD
        cmpobe.f g3,g5,.lclmrg_190      # Jif this is the orig COR

        ld      cor_rmaptbl(g5),r12     # r12 = address of region map
        cmpobe.t 0,r12,.lclmrg_170      # JIf RM is NOT allocated

        ld      RM_lastmerge(r12),r3    # r3 = RM merge timestamp
        cmpobe.f r3,r9,.lclmrg_190      # JIf already integrated
#
# --- merge original COR into the current COR
#
.lclmrg_170:
        call    cm$merge_RM             # merge RM's
                                        #  input:
                                        #       g3 = original COR
                                        #       g4 = original RM
                                        #       g5 = current COR
                                        #  output:
                                        #       g3 = original COR
                                        #       g4 = original RM
                                        #       g5 = current COR
                                        #       g6 = current RM
#
# --- this merge is complete, so mark it so
#
        st      r9,RM_lastmerge(g6)     # save merge timestamp
        b       .lclmrg_100             # start over from the beginning
#
# --- fine next SCD
#
.lclmrg_190:
        ld      scd_link(r8),r8         # r8 = next scd in link list
        cmpobne.t 0,r8,.lclmrg_150      # Jif another SCD is defined

#**************
#
#     SOURCE DCD
#
# --- determine if there is a DCD associated with this VDD
#
.lclmrg_200:
        ld      vd_dcd(r7),r8           # r8 = DCD
        cmpobe.f 0,r8,.lclmrg_300       # JIf no DCD defined
#
# --- determine if the current COR has already been processed
#
        ldconst 0,g6                    # clear current RM

        ld      dcd_cor(r8),g5          # g5 = COR for this SCD

        ld      cor_rmaptbl(g5),r12     # r12 = address of region map
        cmpobe.t 0,r12,.lclmrg_250      # JIf RM is NOT allocated

        ld      RM_lastmerge(r12),r3    # r3 = RM merge timestamp
        cmpobe.f r3,r9,.lclmrg_300      # JIf already integrated
#
# --- merge original COR into the current COR
#
.lclmrg_250:
        call    cm$merge_RM             # merge RM's
                                        #  input:
                                        #       g3 = original COR
                                        #       g4 = original RM
                                        #       g5 = current COR
                                        #  output:
                                        #       g3 = original COR
                                        #       g4 = original RM
                                        #       g5 = current COR
                                        #       g6 = current RM
#
# --- this merge is complete, so mark it so
#
        st      r9,RM_lastmerge(g6)     # save merge timestamp

#**************
#
#     DESTINATION SCD
#
# --- get the VDD of the destination device
#
.lclmrg_300:
        ld      cor_destvdd(g3),r7      # r7 = source vdd
#
# --- get 1st SCD from VDD SCD link list
#
.lclmrg_310:
        ld      vd_scdhead(r7),r8       # r8 = 1st SCD on list
        cmpobe.f 0,r8,.lclmrg_exit      # JIf no SCD's defined
#
# --- determine if the current COR has already been processed
#
.lclmrg_350:
        ldconst 0,g6                    # clear current RM
        ld      scd_cor(r8),g5          # g5 = COR for this SCD

        ld      cor_rmaptbl(g5),r12     # r12 = address of region map
        cmpobe.t 0,r12,.lclmrg_370      # JIf RM is NOT allocated

        ld      RM_lastmerge(r12),r3    # r3 = RM merge timestamp
        cmpobe.f r3,r9,.lclmrg_390      # JIf already integrated
#
# --- merge original COR into the current COR
#
.lclmrg_370:
        call    cm$merge_RM             # merge RM's
                                        #  input:
                                        #       g3 = original COR
                                        #       g4 = original RM
                                        #       g5 = current COR
                                        #  output:
                                        #       g3 = original COR
                                        #       g4 = original RM
                                        #       g5 = current COR
                                        #       g6 = current RM
#
# --- this merge is complete, so mark it so
#
        st      r9,RM_lastmerge(g6)     # save merge timestamp
        b       .lclmrg_310             # start over from the beginning
#
# --- fine next SCD
#
.lclmrg_390:
        ld      scd_link(r8),r8         # r8 = next scd in link list
        cmpobne.t 0,r8,.lclmrg_350      # Jif another SCD is defined
#
# --- Rm merger is complete. Exit
#
.lclmrg_exit:
        PopRegsVoid                     # Restore all G registers (stack relative)
        ret                             # return to caller
#**********************************************************************
#
#  NAME: cm$merge_RM
#
#  PURPOSE:
#       To provide a common means to merge to Region Map tables
#       together.
#
#  CALLING SEQUENCE:
#       call    cm$merge_RM
#
#  INPUT:
#       g3 = original COR address
#       g4 = original RM address
#       g5 = current COR address
#
#  OUTPUT:
#       g6 = current RM address
#
#  REGS DESTROYED:
#       Reg. g0-g1 are destroyed
#
#**********************************************************************
#
cm$merge_RM:
        ldconst 0,r12                   # r12 = SM index
#
# --- determine if there is a current COR RM allocated. If not,
#     allocate one...
#
        ld      cor_rmaptbl(g5),g6      # g6 = address of region map
        cmpobne.t 0,g6,.mrgRM_120       # JIf RM is allocated
#
# --- Allocate a RM and link into the COR. After a RM has been
#     allocated, check if the RM pointer in the COR is still zero.
#     If not, deallocate the new RM and use the RM from COR pointer.
#
c       g0 = get_rm();                  # Allocate a region table
.ifdef M4_DEBUG_RM
c fprintf(stderr, "%s%s:%u get_rm 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif # M4_DEBUG_RM
        ld      cor_rmaptbl(g5),g6      # g6 = address of region map
        cmpobe.t 0,g6,.mrgRM_105        # JIf RM has been allocated

.ifdef M4_DEBUG_RM
c fprintf(stderr, "%s%s:%u put_rm 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_RM
c       put_rm(g0);                     # Release region map table (RM)
        b       .mrgRM_120              # br

.mrgRM_105:
        mov     g0,g6                   # place in correct register
        st      g6,cor_rmaptbl(g5)      # save address of RM in COR
#
# --- determine if there is a SM at this region in the original RM
#
.mrgRM_120:
        ld      RM_tbl(g4)[r12*4],r14   # r14 = SM addr from original RM
        cmpobe.t 0,r14,.mrgRM_300       # JIf no SM
#
# --- Determine if there is a SM allocated for this region in the current
#     RM. If not, allocate one
#
        ld      RM_tbl(g6)[r12*4],r15   # r15 = SM addr from current RM
        cmpobne.t 0,r15,.mrgRM_140      # JIf SM is already allocated
#
# --- allocate a SM and link it into the RM at correct region
#
c       g1 = get_sm();                  # Allocate a cleared segment map table
.ifdef M4_DEBUG_SM
c fprintf(stderr, "%s%s:%u get_sm 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_SM
        ld      RM_tbl(g6)[r12*4],r15   # r15 = address of segment map header
        cmpobe.t 0,r15,.mrgRM_135       # JIf SM is allocated

.ifdef M4_DEBUG_SM
c fprintf(stderr, "%s%s:%u put_sm 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_SM
c       put_sm(g1);                     # Deallocate segment map table
        b       .mrgRM_140

.mrgRM_135:
        mov     g1,r15                  # move to correct register
        st      r15,RM_tbl(g6)[r12*4]   # save address in RM at correct region
        movq    g0,r4                   # save g0-g3
        mov     r12,g0                  # g0 = new dirty region #
        mov     g5,g3                   # g3 = COR address being updated
        call    CCSM$reg_dirty          # set dirty region bit in state NVRAM
        movq    r4,g0                   # restore g0-g3
#
# --- merge the original RM into the current RM
#
.mrgRM_140:
        ldconst 0,r3                    # set index to zero
        ldconst SMTBLsize/16,r13        # number of quads in SM

.mrgRM_200:
        ldq     SM_tbl(r14)[r3*16],r4   # get original RM data
        ldq     SM_tbl(r15)[r3*16],r8   # get current RM data
        or      r4,r8,r8                # or maps together
        or      r5,r9,r9
        or      r6,r10,r10
        or      r7,r11,r11
        stq     r8,SM_tbl(r15)[r3*16]   # save combined maps in current RM

        addo    1,r3,r3                 # bump index
        cmpobl  r3,r13,.mrgRM_200       # JIf more to do
#
# --- find next SM to merge
#
.mrgRM_300:
        addo    1,r12,r12               # bump to next region
        ldconst maxRMcnt,r3
        cmpobl  r12,r3,.mrgRM_120       # Jif more regions to check
#
# --- determine the number of segment bit that are now set in
#     the updated current RM.
#
        mov     g3,r3                   # save g3
        mov     g5,g3                   # set COR addreess
        call    CM$update_rmap          # count updated segments
        mov     r3,g3                   # restore original COR address
#
# --- exit
#
        ret                             # return to caller
#
#**********************************************************************
#**********************************************************************
#**********************************************************************
#
#         R E M O T E   R E G I O N   M A P   R O U T I N E S
#
#**********************************************************************
#**********************************************************************
#**********************************************************************
#**********************************************************************
#
#  NAME: cm$collect_RM
#
#  PURPOSE:
#       To provide a means of collecting and integrating the
#       region map of a remote MAGNITUDE
#
#  CALLING SEQUENCE:
#       call    cm$collect_RM
#
#  INPUT:
#       g3 = COR address
#       g4 = CM address
#       g5 = MAG s/n
#
#  OUTPUT:
#       g0 = completion status.
#
#  REGS DESTROYED:
#       Reg. g1-g2 are destroyed
#
#**********************************************************************
#
cm$collect_RM:
        mov     g5,g0                   # g0 = MAG s/n
        call    cm$pksnd_RMsuspend      # try to suspend region map
        cmpobne.f 0,g0,.colRM_exit      # Jif error

        mov     0,r15                   # r15 = 0
#
# --- determine if there is anything to pull back
#
.colRM_100:
        mov     g5,g0                   # g0 = MAG s/n
        call    cm$pksnd_RMterm         # try to terminate the region map
        cmpobe.t 0,g0,.colRM_600        # JIf nothing to pull back

        cmpobne dg_st_srvr,g0,.colRM_exit # JIf not destination server error
        cmpobne dgec1_srvr_dirty,g1,.colRM_exit
#
# --- there are regions to pull back. Find out which one's
#
        mov     g5,g0                   # g0 = MAG s/n
        call    cm$pksnd_RMcheck        # determine dirty regions
        cmpobne.f 0,g0,.colRM_exit      # Jif error
#
# --- get the address of the RM response
#
        mov     g1,r15                  # r15 = address of RMcheck ILT
        ld      dsc2_rsbuf(g1),r12      # r12 = response header msg buf
        lda     dgrs_size(r12),r12      # r12 = adr of region cnt map

        ldconst 0,g2                    # g2 = zero region index
#
# --- determine if this region is dirty
#
.colRM_200:
        ld      CMsp_rs_rmchk_cnts(r12)[g2*4],r4 # r4 = segment count
        cmpobe.t 0,r4,.colRM_500        # Jif region is clean
#
# --- this region is dirty. so go collect it
#
        mov     g5,g0                   # g0 = MAG s/n
        call    cm$pksnd_RMread         # read the dirty region
        cmpobe.f 0,g0,.colRM_250        # Jif no errors
#
# --- There were problems. Return the RMcheck datagram and exit
#
.colRM_220:
        lda     -dsc1_ulvl(r15),g1      # g1 = datagram ILT at nest level 1
        call    DLM$put_dg              # deallocate datagram ILT
        b       .colRM_exit             # exit
#
# --- find the data areas
#
.colRM_250:
        ld      dsc2_rsbuf(g1),r13      # r13 = response msg buf
        lda     dgrs_size+CMsp_rs_rmrd_table(r13),r13 # r13 = adr seg map
#
# --- determine if there is a local RM allocated. If not, allocate one
#
        ld      cor_rmaptbl(g3),g0      # g0 = address of region map
        cmpobne.t 0,g0,.colRM_270       # JIf RM is allocated
#
# --- allocate a RM and link into the COR
#
c       g0 = get_rm();                  # Allocate a region table
.ifdef M4_DEBUG_RM
c fprintf(stderr, "%s%s:%u get_rm 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif # M4_DEBUG_RM
        ld      cor_rmaptbl(g3),r3      # g0 = address of region map
        cmpobe.t 0,r3,.colRM_255        # JIf RM pointer is still clear

.ifdef M4_DEBUG_RM
c fprintf(stderr, "%s%s:%u put_rm 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif # M4_DEBUG_RM
c       put_rm(g0);                     # Release region map table (RM)
        mov     r3,g0                   # place correct register
        b       .colRM_270              # br

.colRM_255:
        st      g0,cor_rmaptbl(g3)      # save address of RM in COR
#
# --- determine if there is a SM allocated for this region. If not,
#     allocate one
#
.colRM_270:
        ld      RM_tbl(g0)[g2*4],r14    # r14 = address of segment map header
        cmpobne.t 0,r14,.colRM_290      # JIf SM is allocated
#
# --- allocate a SM and link it into the RM at correct region
#
        mov     g1,r11                  # save g1
c       g1 = get_sm();                  # Allocate a cleared segment map table
.ifdef M4_DEBUG_SM
c fprintf(stderr, "%s%s:%u get_sm 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_SM

        ld      RM_tbl(g0)[g2*4],r14    # r14 = address of segment map header
        cmpobe.t 0,r14,.colRM_275       # JIf SM pointer is still zero

.ifdef M4_DEBUG_SM
c fprintf(stderr, "%s%s:%u put_sm 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_SM
c       put_sm(g1);                     # Deallocate segment map table
        mov     r11,g1                  # restore g1
        b       .colRM_290

.colRM_275:
        st      g1,RM_tbl(g0)[g2*4]     # save address in RM at correct region
        mov     g1,r14                  # place in correct register
        mov     r11,g1                  # restore g1
        mov     g0,r11                  # save g0
        mov     g2,g0                   # g0 = new dirty region #
        call    CCSM$reg_dirty          # set dirty region bit in state NVRAM
        mov     r11,g0                  # restore g0
.colRM_290:
        lda     SM_tbl(r14),r14         # r14 = address of segment map
#
# --- integrate the collected remote RM into the local RM
#
        ldconst 0,r3                    # set index to zero

.colRM_300:
        ldq     (r13)[r3*16],r4         # get remote RM data
        ldq     (r14)[r3*16],r8         # get local RM data
        or      r4,r8,r8                # or maps together
        or      r5,r9,r9
        or      r6,r10,r10
        or      r7,r11,r11
        stq     r8,(r14)[r3*16]         # save combined maps in local RM

        ldconst SMTBLsize/16,r4         # number of quads in SM
        addo    1,r3,r3                 # bump index
        cmpobl  r3,r4,.colRM_300        # JIf more to do
#
# --- this region has been integrated, release it's datagram
#
        lda     -dsc1_ulvl(g1),g1       # g1 = datagram ILT at nest level 1
        call    DLM$put_dg              # deallocate datagram ILT
#
# --- Clear the region at the remote
#
        mov     g5,g0                   # g0 = MAG s/n
        call    cm$pksnd_RMclear        # clear the remote region
        cmpobne.f 0,g0,.colRM_220       # JIf error
#
# --- determine if there are more regions to integrate into local RM
#
.colRM_500:
        ldconst maxRMcnt,r4             # r4 = max regions
        addo    1,g2,g2                 # bump the region index
        cmpobl  g2,r4,.colRM_200        # Jif more regions to check
#
# --- all regions have been checked an integrated. Return the RMcheck
#     datagram and try to terminate the region map again.
#
        lda     -dsc1_ulvl(r15),g1      # g1 = datagram ILT at nest level 1
        call    DLM$put_dg              # deallocate datagram ILT
        b       .colRM_100              # try again
#
# --- collection and integration of the remote RM is complete and the
#     remote RM has been terminated. determine the number of segment bit
#     that are now set in the updated local RM.
#
.colRM_600:
        cmpobe.f 0,r15,.colRM_exit      # JIf nothing was done

        call    CM$update_rmap          # count updated segments
#
# --- exit
#
.colRM_exit:
        ret                             # return to caller

#**********************************************************************
#
#  NAME: cm$pksnd_RMsuspend
#
#  PURPOSE:
#       To provide a means of packing and sending a suspend
#       region map datagram message to the specified MAGNITUDE.
#
#  DESCRIPTION:
#       This routine packs and send a suspend region map datagram
#       message to the specified MAGNITUDE. If an error occurs, it attempts
#       to perform some level of error recovery, but if still unsuccessful
#       will return to the calling routine.
#
#  CALLING SEQUENCE:
#       call    cm$pksnd_RMsuspend
#
#  INPUT:
#       g0 = MAG S/N
#       g3 = COR address
#
#  OUTPUT:
#       g0 = completion status.
#          If g0 = 0 then g1 is unchanged
#          If g0 <> 0 then g1 is EC1
#
#  REGS DESTROYED:
#       Reg. g0-g1 is destroyed
#
#**********************************************************************
#
cm$pksnd_RMsuspend:
        movl    g0,r14                  # save g0-g1
        ldconst 4,r13                   # r13 = error retry count

.pksndRMsp_100:
        mov     r14,g0                  # g0 = MAG serial number to send to
        call    cm$pkrm_susp            # Pack suspend region map datagram
                                        # g1 = datagram ILT at nest level 2
        lda     DLM$send_dg,g0          # g0 = datagram service provider routine
        call    K$qw                    # Queue request w/wait
        ld      dsc2_rshdr_ptr(g1),r7   # r7 = local response header address
        lda     -dsc1_ulvl(g1),g1       # g1 = datagram ILT at nest level 1
        ldob    dgrs_status(r7),g0      # g0 = request completion status
        ldob    dgrs_ec1(r7),r6         # r6 = error code byte #1
        call    DLM$put_dg              # deallocate datagram ILT
        cmpobe  0,g0,.pksndRMsp_1000    # jif status is zero

        cmpobe.t dg_st_srvr,g0,.pksndRMsp_300 # Jif dest. server level error

.pksndRMsp_150:
        subo    1,r13,r13               # dec. error retry count
        cmpobne.t 0,r13,.pksndRMsp_100  # Jif error retry count not expired

.pksndRMsp_200:
        mov     r6,r15                  # r15 = EC1 code
        b       .pksndRMsp_1000

.pksndRMsp_300:
        cmpobe.f dgec1_srvr_nosvlink,r6,.pksndRMsp_200 # Jif source VLink not
                                        #  established
        cmpobe.f dgec1_srvr_nodvlink,r6,.pksndRMsp_200 # Jif dest. VLink not
                                        #  established
        cmpobe.f dgec1_srvr_inuse,r6,.pksndRMsp_200 # Jif specified copy device
                                        #  in use
        b       .pksndRMsp_150

.pksndRMsp_1000:
        mov     r15,g1                  # restore g1
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: cm$pksnd_RMterm
#
#  PURPOSE:
#       To provide a means of packing and sending a terminate
#       region map datagram message to the specified MAGNITUDE.
#
#  DESCRIPTION:
#       This routine packs and send a terminate region map datagram
#       message to the specified MAGNITUDE. If an error occurs, it attempts
#       to perform some level of error recovery, but if still unsuccessful
#       will return to the calling routine.
#
#  CALLING SEQUENCE:
#       call    cm$pksnd_RMterm
#
#  INPUT:
#       g0 = MAG S/N
#       g3 = COR address
#
#  OUTPUT:
#       g0 = completion status.
#          If g0 = 0 then g1 is unchanged
#          If g0 <> 0 then g1 is EC1
#
#  REGS DESTROYED:
#       Reg. g0-g1 is destroyed
#
#**********************************************************************
#
cm$pksnd_RMterm:
        movl    g0,r14                  # save g0-g1
        ldconst 4,r13                   # r13 = error retry count

.pksndRMtm_100:
        mov     r14,g0                  # g0 = MAG serial number to send to
        call    cm$pkrm_term            # Pack terminate region map datagram
                                        # g1 = datagram ILT at nest level 2
        lda     DLM$send_dg,g0          # g0 = datagram service provider routine
        call    K$qw                    # Queue request w/wait
        ld      dsc2_rshdr_ptr(g1),r7   # r7 = local response header address
        lda     -dsc1_ulvl(g1),g1       # g1 = datagram ILT at nest level 1
        ldob    dgrs_status(r7),g0      # g0 = request completion status
        ldob    dgrs_ec1(r7),r6         # r6 = error code byte #1
        call    DLM$put_dg              # deallocate datagram ILT
        cmpobe  0,g0,.pksndRMtm_1000    # jif status is zero

        cmpobe.t dg_st_srvr,g0,.pksndRMtm_300 # Jif dest. server level error

.pksndRMtm_150:
        subo    1,r13,r13               # dec. error retry count
        cmpobne.t 0,r13,.pksndRMtm_100  # Jif error retry count not expired

.pksndRMtm_200:
        mov     r6,r15                  # r15 = EC1 code
        b       .pksndRMtm_1000

.pksndRMtm_300:
        cmpobe.t dgec1_srvr_dirty,r6,.pksndRMtm_200 # JIf dirty RM

        cmpobe.f dgec1_srvr_nosvlink,r6,.pksndRMtm_200 # Jif source VLink not
                                        #  established
        cmpobe.f dgec1_srvr_nodvlink,r6,.pksndRMtm_200 # Jif dest. VLink not
                                        #  established
        cmpobe.f dgec1_srvr_inuse,r6,.pksndRMtm_200 # Jif specified copy device
                                        #  in use
        b       .pksndRMtm_150

.pksndRMtm_1000:
        mov     r15,g1                  # restore g1
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: cm$pksnd_RMcheck
#
#  PURPOSE:
#       To provide a means of packing and sending a check
#       region map datagram message to the specified MAGNITUDE.
#
#  DESCRIPTION:
#       This routine packs and send a check region map datagram
#       message to the specified MAGNITUDE. If an error occurs, it attempts
#       to perform some level of error recovery, but if still unsuccessful
#       will return to the calling routine.
#
#  CALLING SEQUENCE:
#       call    cm$pksnd_RMcheck
#
#  INPUT:
#       g0 = MAG S/N
#       g3 = COR address
#
#  OUTPUT:
#       g0 = completion status.
#            if g0 = 0 then g1 = dg ILT
#            if g0 <> 0 then g1 = EC1
#
#  REGS DESTROYED:
#       Reg. g0-g1 is destroyed
#
#**********************************************************************
#
cm$pksnd_RMcheck:
        mov     g0,r14                  # save g0
        ldconst 4,r13                   # r13 = error retry count

.pksndRMck_100:
        mov     r14,g0                  # g0 = MAG serial number to send to
        call    cm$pkrm_check           # Pack terminate region map datagram
                                        # g1 = datagram ILT at nest level 2
        lda     DLM$send_dg,g0          # g0 = datagram service provider routine
        call    K$qw                    # Queue request w/wait
        ld      dsc2_rshdr_ptr(g1),r7   # r7 = local response header address
        ldob    dgrs_status(r7),g0      # g0 = request completion status
        cmpobe  0,g0,.pksndRMck_1000    # jif status is zero

        lda     -dsc1_ulvl(g1),g1       # g1 = datagram ILT at nest level 1
        ldob    dgrs_ec1(r7),r6         # r6 = error code byte #1
        call    DLM$put_dg              # deallocate datagram ILT

        cmpobe.t dg_st_srvr,g0,.pksndRMck_300 # Jif dest. server level error

.pksndRMck_150:
        subo    1,r13,r13               # dec. error retry count
        cmpobne.t 0,r13,.pksndRMck_100  # Jif error retry count not expired

.pksndRMck_200:
        mov     r6,g1                   # g1 = EC1 code
        b       .pksndRMck_1000

.pksndRMck_300:
        cmpobe.f dgec1_srvr_nosvlink,r6,.pksndRMck_200 # Jif source VLink not
                                        #  established
        cmpobe.f dgec1_srvr_nodvlink,r6,.pksndRMck_200 # Jif dest. VLink not
                                        #  established
        cmpobe.f dgec1_srvr_inuse,r6,.pksndRMck_200 # Jif specified copy device
                                        #  in use
        b       .pksndRMck_150

.pksndRMck_1000:
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: cm$pksnd_RMread
#
#  PURPOSE:
#       To provide a means of packing and sending a read
#       region map datagram message to the specified MAGNITUDE.
#
#  DESCRIPTION:
#       This routine packs and send a read region map datagram
#       message to the specified MAGNITUDE. If an error occurs, it attempts
#       to perform some level of error recovery, but if still unsuccessful
#       will return to the calling routine.
#
#  CALLING SEQUENCE:
#       call    cm$pksnd_RMread
#
#  INPUT:
#       g0 = MAG S/N
#       g2 = region map number to read
#       g3 = COR address
#
#  OUTPUT:
#       g0 = completion status.
#            if g0 = 0 then g1 = dg ILT
#            if g0 <> 0 then g1 = EC1
#
#  REGS DESTROYED:
#       Reg. g0-g1 is destroyed
#
#**********************************************************************
#
cm$pksnd_RMread:
        mov     g0,r14                  # save g0
        ldconst 4,r13                   # r13 = error retry count

.pksndRMrd_100:
        mov     r14,g0                  # g0 = MAG serial number to send to
        call    cm$pkrm_read            # Pack read region map datagram
                                        # g1 = datagram ILT at nest level 2
        lda     DLM$send_dg,g0          # g0 = datagram service provider routine
        call    K$qw                    # Queue request w/wait
        ld      dsc2_rshdr_ptr(g1),r7   # r7 = local response header address
        ldob    dgrs_status(r7),g0      # g0 = request completion status
        cmpobe  0,g0,.pksndRMrd_1000    # jif status is zero

        lda     -dsc1_ulvl(g1),g1       # g1 = datagram ILT at nest level 1
        ldob    dgrs_ec1(r7),r6         # r6 = error code byte #1
        call    DLM$put_dg              # deallocate datagram ILT

        cmpobe.t dg_st_srvr,g0,.pksndRMrd_300 # Jif dest. server level error

.pksndRMrd_150:
        subo    1,r13,r13               # dec. error retry count
        cmpobne.t 0,r13,.pksndRMrd_100  # Jif error retry count not expired

.pksndRMrd_200:
        mov     r6,g1                   # g1 = EC1 code
        b       .pksndRMrd_1000

.pksndRMrd_300:
        cmpobe.f dgec1_srvr_nosvlink,r6,.pksndRMrd_200 # Jif source VLink not
                                        #  established
        cmpobe.f dgec1_srvr_nodvlink,r6,.pksndRMrd_200 # Jif dest. VLink not
                                        #  established
        cmpobe.f dgec1_srvr_inuse,r6,.pksndRMrd_200 # Jif specified copy device
                                        #  in use
        b       .pksndRMrd_150

.pksndRMrd_1000:
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: cm$pksnd_RMclear
#
#  PURPOSE:
#       To provide a means of packing and sending a clear
#       region map datagram message to the specified MAGNITUDE.
#
#  DESCRIPTION:
#       This routine packs and send a clear region map datagram
#       message to the specified MAGNITUDE. If an error occurs, it attempts
#       to perform some level of error recovery, but if still unsuccessful
#       will return to the calling routine.
#
#  CALLING SEQUENCE:
#       call    cm$pksnd_RMclear
#
#  INPUT:
#       g0 = MAG S/N
#       g2 = region map number to read
#       g3 = COR address
#
#  OUTPUT:
#       g0 = completion status.
#          If g0 = 0 then g1 is unchanged
#          If g0 <> 0 then g1 is EC1
#
#  REGS DESTROYED:
#       Reg. g0-g1 is destroyed
#
#**********************************************************************
#
cm$pksnd_RMclear:
        movl    g0,r14                  # save g0-g1
        ldconst 4,r13                   # r13 = error retry count

.pksndRMcl_100:
        mov     r14,g0                  # g0 = MAG serial number to send to
        call    cm$pkrm_clear           # Pack clear region map datagram
                                        # g1 = datagram ILT at nest level 2
        lda     DLM$send_dg,g0          # g0 = datagram service provider routine
        call    K$qw                    # Queue request w/wait
        ld      dsc2_rshdr_ptr(g1),r7   # r7 = local response header address
        lda     -dsc1_ulvl(g1),g1       # g1 = datagram ILT at nest level 1
        ldob    dgrs_status(r7),g0      # g0 = request completion status
        ldob    dgrs_ec1(r7),r6         # r6 = error code byte #1
        call    DLM$put_dg              # deallocate datagram ILT
        cmpobe  0,g0,.pksndRMcl_1000    # jif status is zero

        cmpobe.t dg_st_srvr,g0,.pksndRMcl_300 # Jif dest. server level error

.pksndRMcl_150:
        subo    1,r13,r13               # dec. error retry count
        cmpobne.t 0,r13,.pksndRMcl_100  # Jif error retry count not expired

.pksndRMcl_200:
        mov     r6,r15                  # r15 = EC1 code
        b       .pksndRMcl_1000

.pksndRMcl_300:
        cmpobe.f dgec1_srvr_nosvlink,r6,.pksndRMcl_200 # Jif source VLink not
                                        #  established
        cmpobe.f dgec1_srvr_nodvlink,r6,.pksndRMcl_200 # Jif dest. VLink not
                                        #  established
        cmpobe.f dgec1_srvr_inuse,r6,.pksndRMcl_200 # Jif specified copy device
                                        #  in use
        b       .pksndRMcl_150

.pksndRMcl_1000:
        mov     r15,g1                  # restore g1
        ret                             # return to caller
#
#**********************************************************************
#**********************************************************************
#**********************************************************************
#
#         P C P   G E N E R A T I O N    R O U T I N E S
#
#**********************************************************************
#**********************************************************************
#**********************************************************************
#**********************************************************************
#
#  NAME: CM$PauseCopy
#
#  PURPOSE:
#       To provide a means of pausing a copy
#
#  CALLING SEQUENCE:
#       call    CM$PauseCopy
#
#  INPUT:
#       g3 = COR address
#
#  OUTPUT:
#       None:
#
#  REGS DESTROYED:
#       None:
#
#**********************************************************************
#
CM$PauseCopy:
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>ladpcm-In..CM$PauseCopy\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # CM_IM_DEBUG
        movq    g0,r12                  # save environment
        mov     g5,r8
        ld      cor_cm(g3),r4           # r4 = assoc. CM address
        cmpobne 0,r4,.pausecopy_400     # Jif local copy
#
# --- Remote copy handler routine
#
        ldconst PauseCopy,g0            # g0 = event type code
        call    CCSM$CCTrans            # generate event to C.C.S.E.
                                        # g0 = event type
                                        # g3 = COR address
        b       .pausecopy_1000
#
# --- Local copy handler routine
#
.pausecopy_400:
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>ladpcm-CM$PauseCopy- Calling sendPCP_common with pcp1fcPause event\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # CM_IM_DEBUG
        ldconst pcp1fc_pause,g0
        ldconst 0,g2                    # request new state of task
                                        #    0 = no change
        mov     0,g5                    # g5 = null
        call    cm$sendPCP_common       # send the pcp
.pausecopy_1000:
        movq    r12,g0                  # restore environment
        mov     r8,g5
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: CM$ResumeCopy
#
#  PURPOSE:
#       To provide a means of resuming a copy
#
#  CALLING SEQUENCE:
#       call    CM$ResumeCopy
#
#  INPUT:
#       g3 = COR address
#
#  OUTPUT:
#       None:
#
#  REGS DESTROYED:
#       None:
#
#**********************************************************************
#
CM$ResumeCopy:
        movq    g0,r12                  # save environment
        mov     g5,r8
        ld      cor_cm(g3),r4           # r4 = assoc. CM address
        cmpobne 0,r4,.resumecopy_400    # Jif local copy
#
# --- Remote copy handler routine
#
        ldconst ResumeCopy,g0           # g0 = event type code
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>ladpcm-CM$ResumeCopy--call CCSM$CCTrans with Resume_Copy event =%lx..remote copy\n", FEBEMESSAGE, __FILE__, __LINE__,g0);
.endif  # CM_IM_DEBUG
        call    CCSM$CCTrans            # generate event to C.C.S.E.
                                        # g0 = event type
                                        # g3 = COR address
        b       .resumecopy_1000
#
# --- Local copy handler routine
#
.resumecopy_400:
        ldconst pcp1fc_resume,g0
        ldconst 0,g2                    # request new state of task
                                        #    0 = no change
        mov     0,g5                    # g5 = null
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>ladpcm-CM$ResumeCopy--call cm$SendPCP_common with pcp1f1resume=%lx..local copy\n", FEBEMESSAGE, __FILE__, __LINE__,g0);
.endif  # CM_IM_DEBUG
        call    cm$sendPCP_common       # send the pcp
.resumecopy_1000:
        movq    r12,g0                  # restore environment
        mov     r8,g5
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: CM$SwapRaids
#
#  PURPOSE:
#       To provide a means of swapping the raids involved in a copy.
#
#  CALLING SEQUENCE:
#       call    CM$SwapRaids
#
#  INPUT:
#       g2 = sequence number
#       g3 = COR address
#
#  OUTPUT:
#       None:
#
#  REGS DESTROYED:
#       None:
#
#**********************************************************************
#
CM$SwapRaids:
        ld      cor_cm(g3),r4           # r4 = assoc. CM address
        cmpobne 0,r4,.swapraids_400     # Jif local copy
#
# --- Remote copy handler routine
#
        ldconst SwapRaids,g0            # g0 = event type code
        call    CCSM$CCTrans            # generate event to C.C.S.E.
                                        # g0 = event type
                                        # g3 = COR address
        b       .swapraids_1000_lad
#
# --- Local copy handler routine
#
.swapraids_400:
        movq    g0,r12                  # save environment
        mov     g5,r8

        ldconst pcp1fc_swap,g0          # g0 = swap raids
        mov     g2,g5                   # g5 = sequence number
        ldconst 0,g2                    # g2 = no change in task state
        call    cm$sendPCP_common       # send the pcp
.swapraids_1000_lad:
        movq    r12,g0                  # restore environment
        mov     r8,g5
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: CM$BreakCopy
#
#  PURPOSE:
#       To provide a means of breaking a copy.
#
#  CALLING SEQUENCE:
#       call    CM$BreakCopy
#
#  INPUT:
#       g3 = COR address
#
#  OUTPUT:
#       None:
#
#  REGS DESTROYED:
#       None:
#
#**********************************************************************
#
CM$BreakCopy:
        ld      cor_cm(g3),r4           # r4 = assoc. CM address
        movq    g0,r12                  # save environment
        cmpobne 0,r4,.breakcopy_400     # Jif local copy
#
# --- Remote copy handler routine
#
        ldconst BreakCopy,g0            # g0 = event type code
        call    CCSM$CCTrans            # generate event to C.C.S.E.
                                        # g0 = event type
                                        # g3 = COR address
        b       .breakcopy_1000
#
# --- Local copy handler routine
#
.breakcopy_400:
        mov     g5,r8                   # save g5 before cm$sendPCP_common
        ldconst pcp1fc_break,g0
        ldconst 0,g2                    # request new state of task
                                        #    0 = no change
        mov     0,g5                    # g5 = null
        call    cm$sendPCP_common       # send the pcp
        mov     r8,g5                   # restore g5
.breakcopy_1000:
        movq     r12,g0                 # restore environment
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: CM$OwnershipAcquired
#
#  PURPOSE:
#       To provide a means of identifying that copy ownership has been
#       acquired.
#
#  CALLING SEQUENCE:
#       call    CM$OwnershipAcquired
#
#  INPUT:
#       g3 = COR address
#
#  OUTPUT:
#       None:
#
#  REGS DESTROYED:
#       None:
#
#**********************************************************************
#
CM$OwnershipAcquired:
        movq    g0,r12                  # save environment
        mov     g5,r8
        ld      cor_cm(g3),r4           # r4 = assoc. CM address
        cmpobne 0,r4,.ownacq_400        # Jif local copy
#
# --- Remote copy handler routine
#
        ldconst OwnerAcquired,g0        # g0 = event type code
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>ladpcm-CM$OwnershipAcquired..Calling CCSM$CCTrans with Owner_Acquired(RmtCopy)=%lx\n", FEBEMESSAGE, __FILE__, __LINE__,g0);
.endif  # CM_IM_DEBUG
        call    CCSM$CCTrans            # generate event to C.C.S.E.
                                        # g0 = event type
                                        # g3 = COR address
        b       .ownacq_1000
#
# --- Local copy handler routine
#
.ownacq_400:
        ldconst pcp1fc_OwnerAcq,g0
        ld   cor_srcvdd(g3), r5        # Get source VDD
c       r6 = CM_VdiskInstantMirrorAllowed((VDD*)r5);
        cmpobe  FALSE,r6,.ownacq_450
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>ladpcm-CM$OwnershipAcquired--Inst Mirror is enabled for the copy..svid=%x dvid=%x\n", FEBEMESSAGE, __FILE__, __LINE__,((VDD*)r5)->vid, ((VDD*)(((COR*)g3)->destvdd))->vid);
.endif  # CM_IM_DEBUG
#
# --- Set event as Instant mirror
#
        ldconst pcp1fc_InstantMirror,g0

.ownacq_450:
.if GR_GEORAID15_DEBUG
c fprintf(stderr,"%s%s:%u <GR>ladpcm-CM$OwnershipAcquired>Calling cm$sendPCP_common with the event(local copy)=%lx\n", FEBEMESSAGE, __FILE__, __LINE__,g0);
.endif  # GR_GEORAID15_DEBUG
        ldconst 0,g2                    # request new state of task
                                        #    0 = no change
        mov     0,g5                    # g5 = null
        call    cm$sendPCP_common       # send the pcp
.ownacq_1000:
        movq    r12,g0                  # restore environment
        mov     r8,g5
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: cm$InstantMirror
#
#  PURPOSE:
#       To perform quick mirror processing. This function gets called
#       from CCSM$CCTrans when the instant mirror event has been sent
#       after ownership acquisition.
#
#  INPUT:
#       g1 = PCP address at lvl 2
#       g3 = COR address
#       g4 = CM address
#       (Both these addresses have been passed through CCSM$CCTrans from
#        cm$pexec task)
#
#  OUTPUT:
#       None
#
#  REGS DESTROYED:
#       None:
#
#**********************************************************************
#
cm$InstantMirror:
        movq    g1,r11                  # save environment
c       CM_InstantMirror((COR*)g3, (CM *)g4);
        movq    r11,g1                  # restore environment
        ret
#
#**********************************************************************
#
#  NAME: CM$StartTask
#
#  PURPOSE:
#       To provide a means of activating the copy task.
#
#  CALLING SEQUENCE:
#       call    CM$StartTask
#
#  INPUT:
#       g3 = COR address
#
#  OUTPUT:
#       None:
#
#  REGS DESTROYED:
#       None:
#
#**********************************************************************
#
CM$StartTask:
        movq    g0,r12                  # save environment
        ldconst StartTask,g0            # set event
        ld      cor_cm(g3),r4           # r4 = assoc. CM address
        cmpobne 0,r4,.starttask_400     # Jif local copy
#
# --- Remote copy startup handler
#
        ldconst RemoteCopy,g0           # g0 = event type code
.starttask_400:
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>ladpcm-CM$StartTask-Call CCSM$CCTrans with Start_Task (StartTask)event\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # CM_IM_DEBUG
        call    CCSM$CCTrans            # process event
        movq    r12,g0                  # restore environment
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: cm$sendPCP_common
#
#  PURPOSE:
#       To provide a means sending a PC to a specific copy.
#
#  CALLING SEQUENCE:
#       call    cm$sendPCP_common
#
#  INPUT:
#       g0 = function
#       g2 = requested new task state
#       g3 = COR address
#       g5 = misc information
#
#  OUTPUT:
#       None:
#
#  REGS DESTROYED:
#       None:
#
#**********************************************************************
#
cm$sendPCP_common:
        movq    g0,r12                  # save g0-g3
        ld      cor_cm(g3),r9           # r9 = possible CM address
        cmpobe  0,r9,.sndpcpcm_end      # Jif no CM associated with COR
#
# --- There is a CM, mail a PCP to copy task
#
c       g1 = get_ilt();                 # Allocate an ILT (PCP)
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
        mov     0,r3                    # r3 = 0

        lda     D$ctlrqst_cr,r4         # r4 = completion address
        ld      cm_pcb(r9),r5           # r4 = task PCB
        st      r9,pcp1_cm(g1)          # save cm address
        st      r4,pcp1_cr(g1)          # save completion routine address
        st      r5,pcp1_pcb(g1)         # save PCB
        st      g5,pcp1_reg2(g1)        # save misc information
        stob    r14,pcp1_rtstate(g1)    # clear requested task state

        lda     ILTBIAS(g1),g1          # g1 = pcp lvl 2
        st      r15,pcp2_cor(g1)        # save cor address
        st      r3,pcp2_status(g1)      # clear out status,function
        st      r3,pcp2_handler(g1)     # clear Vsync handler script
        stob    r12,pcp2_function(g1)   # save function

        movq    0,r4                    # clear r4-r7
        stq     r4,pcp2_rqctrlsn(g1)    # clear lots of stuff
        stt     r4,pcp2_rssn(g1)
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>lad_pcm.as-cm$sendPCP_common-Created ILT(PCP),queue it by calling CM$ctlrqstq,g0=%lx\n", FEBEMESSAGE, __FILE__, __LINE__,r12);
c fprintf(stderr,"%s%s:%u <CM_IM>lad_pcm.as-cm$sendPCP_common-This PCP  placed in Cm_ctlrqstq queue,processed by cm$pexec\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # CM_IM_DEBUG

        call    CM$ctlrqstq             # enqueue the control request


.sndpcpcm_end:
        movq    r12,g0                  # restore g0-g3
        ret                             # return to caller
#
#**********************************************************************
#**********************************************************************
#**********************************************************************
#
#         E V E N T   G E N E R A T I O N    R O U T I N E S
#
#**********************************************************************
#**********************************************************************
#**********************************************************************
#
#  NAME: cm$GOC_TaskReady
#
#  PURPOSE:
#       To provide a means generating a Copy Ready event to the OC engine.
#
#  CALLING SEQUENCE:
#       call    cm$GOC_TaskReady
#
#  INPUT:
#       g3 = COR address
#
#  OUTPUT:
#       None:
#
#  REGS DESTROYED:
#       None:
#
#**********************************************************************
#
cm$GOC_TaskReady:
        mov     g0,r15                      # save g0
        ldconst cc_Copy_Ready,g0            # set event
        call    CCSM$OCTrans                # generate event
        mov     r15,g0
        ret                                 # return to caller
#
#**********************************************************************
#
#  NAME: cm$GOC_OwnerSuspend
#
#  PURPOSE:
#       To provide a means generating a Ownership Suspended event to the OC engine.
#
#  CALLING SEQUENCE:
#       call    cm$GOC_OwnerSuspend
#
#  INPUT:
#       g3 = COR address
#
#  OUTPUT:
#       None:
#
#  REGS DESTROYED:
#       None:
#
#**********************************************************************
#
cm$GOC_OwnerSuspend:
        mov     g0,r15                      # save g0
        ldconst cc_Owner_Suspended,g0       # set event
.if GR_GEORAID15_DEBUG
c fprintf(stderr,"%s%s:%u <GR><GOC_OwnerSuspend-ladpcm>sending ccOwnerSuspened = cc_Owner_Suspended OCevent\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # GR_GEORAID15_DEBUG
        call    CCSM$OCTrans                # generate event
        mov     r15,g0
        ret                                 # return to caller
#
#**********************************************************************
#
#  NAME: cm$GOC_SuspendOwner
#
#  PURPOSE:
#       This routine sets up the write update handlers for a copy
#       in the inactive state.
#
#  CALLING SEQUENCE:
#       call    cm$GOC_SuspendOwner
#
#  INPUT:
#       g3 = COR address
#
#  OUTPUT:
#       None:
#
#  REGS DESTROYED:
#       None:
#
#**********************************************************************
#
cm$GOC_SuspendOwner:
#
#     set up new SCD p2 update handlers
#
        ld      cor_scd(g3),r10         # r10 = scd address
        lda     CM$wp2_inactive,r7      # get new update handler
        cmpobe.f 0,r10,.GOCos_060       # Jif no SCD defined
.if GR_GEORAID15_DEBUG
c fprintf(stderr,"%s%s:%u <GR><GOC_SuspendOwner-lad_pcm> setting inactive handler in scdp2hand\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # GR_GEORAID15_DEBUG
        st      r7,scd_p2hand(r10)      # save new scd p2 handler
#
# --- set up new DCD p2 update handlers
#
.GOCos_060:
        ld      cor_dcd(g3),r10         # r10 = dcd address
        lda     CM$wp2_inactive,r7      # get new update handler
        cmpobe.f 0,r10,.GOCos_070       # Jif no DCD defined

.if GR_GEORAID15_DEBUG
c fprintf(stderr,"%s%s:%u <GR><GOC_SuspendOwner-lad_pcm> setting inactive handler in dcdp2hand\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # GR_GEORAID15_DEBUG
        st      r7,dcd_p2hand(r10)      # save new dcd p2 handler
.GOCos_070:
        ret                                 # return to caller

#**********************************************************************
#
#  NAME: cm$GOC_CopyTerminated
#
#  PURPOSE:
#       To provide a means generating a Copy Terminate event to the OC engine.
#
#  CALLING SEQUENCE:
#       call    cm$GOC_CopyTerminated
#
#  INPUT:
#       g3 = COR address
#
#  OUTPUT:
#       None:
#
#  REGS DESTROYED:
#       None:
#
#**********************************************************************
#
cm$GOC_CopyTerminated:
        mov     g0,r15                      # save g0
        ldconst cc_Copy_Terminated,g0       # set event
        call    CCSM$OCTrans                # generate event
        mov     r15,g0
        ret                                 # return to caller
##
#**********************************************************************
#**********************************************************************
#**********************************************************************
#
#         N E W    S T A T E   A C T I O N   R O U T I N E S
#
#**********************************************************************
#**********************************************************************
#**********************************************************************
#
#  NAME: cm$settaskready
#
#  PURPOSE:
#       To provide a means of changing the state of a copy task from
#       not ready to ready.
#
#  CALLING SEQUENCE:
#       call    CM$settaskready
#
#  INPUT:
#       g3 = COR address
#
#  OUTPUT:
#       None:
#
#  REGS DESTROYED:
#       None:
#
#**********************************************************************
#
CM$settaskrdy:
        ld      cor_cm(g3),r14          # r14 = address of cm
        cmpobe  0,r14,.strtsk_end       # Jif no cm defined

        ld      cm_pcb(r14),r15         # r15 = PCB address
        cmpobe  0,r15,.strtsk_end       # Jif no PCB defined

        ldob    pc_stat(r15),r4         # r4 = current PCB state
        cmpobne pcnrdy,r4,.strtsk_end   # Jif in state other than "not ready"

        ldconst pcrdy ,r4               # Set this process to ready
.ifdef HISTORY_KEEP
c CT_history_pcb("CM$settaskrdy setting ready pcb", r15);
.endif  # HISTORY_KEEP
        stob    r4,pc_stat(r15)

.strtsk_end:
        ret                             # return to caller

#**********************************************************************
#
#  NAME: cm$SetnTestCpyState
#
#  PURPOSE:
#       To provide a means of changing the CM state to be inline with the
#       provided cor registration state and generating an event indicating
#       new CM state.
#
#  CALLING SEQUENCE:
#       call    cm$SetnTestCpyState
#
#  INPUT:
#       g3 = COR address
#
#  OUTPUT:
#       None:
#
#  REGS DESTROYED:
#       None:
#
#**********************************************************************
#
cm$SetnTestCpyState:
        mov     g0,r15                  # save g0
        mov     g4,r14                  # save g4
        ld      cor_cm(g3),g4           # g4 = address of cm
        cmpobe  0,g4,.scs_end           # Jif no cm defined

        ldconst cmcst_uni,r4            # set CM state to uninitialized
        stob    r4,cm_cstate(g4)

        ldob    cor_crstate(g3),r5      # r5 = cor registration stats
#
# --- initializing
#
        cmpobl  corcrst_init,r5,.scs_100 # Jif not initializing
        call    cm$SuspendCopy_auto     # process as auto pause
        b       .scs_end
#
# --- Auto Paused?
#
.scs_100:
        cmpobl  corcrst_autosusp,r5,.scs_200 # Jif auto suspended
.if GR_GEORAID15_DEBUG
c fprintf(stderr, "%s%s:%u <GR><lad_pcm.as cm$SetnTestCpyState> - Calling SuspendCopy_auto crstate=%lx\n", FEBEMESSAGE, __FILE__, __LINE__,r5);
.endif  # GR_GEORAID15_DEBUG
        call    cm$SuspendCopy_auto     # process as auto pause
        b       .scs_end
#
# --- everything else
#
.scs_200:
        call    cm$SuspendCopy_user     # process as user pause

.scs_end:
        mov     r14,g4                  # restore g4
        mov     r15,g0                  # restore g0
        ret                             # return to caller
#
#**********************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
