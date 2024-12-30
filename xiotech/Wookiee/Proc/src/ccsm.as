# $Id: ccsm.as 159305 2012-06-16 08:00:46Z m4 $
#**********************************************************************
#
#  NAME: ccsm.as
#
#  PURPOSE:
#       To provide the logic to support Copy Configuration and
#       Status Manager (CCSM) functions and services.
#
#  FUNCTIONS:
#       CCSM$init       - CCSM initialization routine.
#       CCSM$new_master - New master CCSM (define event)
#       CCSM$not_master - No linger master CCSM (define event)
#       CCSM$start_copy - Start copy (define event)
#       CCSM$term_copy  - Terminate copy (define event)
#       CCSM$pause_copy - Pause copy (define event)
#       CCSM$resume_copy- Resume copy (define event)
#       CCSM$swap_raids - Swap RAIDs (define event)
#       CCSM$cco        - Configuration change occurred (define event)
#       CCSM$cosc       - Copy operational state changed (define event)
#       CCSM$timer      - 1 second timer event (define event)
#       CCSM$swaptimer  - 1 sec. timer event for swap RAIDs process
#       CCSM$upd_check  - Copy update process check
#       CCSM$ccbg       - CCBGram received (define event)
#       CCSM$update     - Update copy (% complete) (client I/F)
#       CCSM$upause     - Copy user paused (client I/F)
#       CCSM$apause     - Copy auto-paused (client I/F)
#       CCSM$resume     - Copy has resumed (client I/F)
#       CCSM$mirror     - Copy mirrored (client I/F)
#       CCSM$ended      - Copy ended (client I/F)
#       CCSM$reg        - Copy registered (client I/F)
#       CCSM$reg_sync   - Region synchronized (client I/F)
#       CCSM$reg_dirty  - Region dirty (client I/F)
#       CCSM$get_cwip   - Get CWIP record (client I/F)
#       CCSM$put_cwip   - Put CWIP record (client I/F)
#       CCSM$mod_init   - Initiate modify COR (client I/F)
#       CCSM$mod_done   - Modify COR completed (client I/F)
#       CCSM$cd_moved   - Copy devices moved (client I/F)
#       CCSM$cs_chged_w_msg - Copy state changed w/message (client I/F)
#       CCSM$cs_chged   - Copy state changed (client I/F)
#       CCSM$info_chged - Copy general information changed (client I/F)
#       CCSM$rpoll_term - Remote COR poll terminate copy (client I/F)
#       CCSM$snd_copyreg- Pack and send Copy Registered message
#       CCSM$snd_process- Pack and send Process message
#       CCSM$snd_update - Pack and send Update message
#       CCSM$snd_force  - Pack and send Force Ownership Change message
#       CCSM$snd_define - Pack and send Define Ownership message
#       CCSM$snd_owner  - Pack and send You Are Owner message
#       CCSM$snd_trans  - Pack and send Transfer Ownership message
#       CCSM$snd_change - Pack and send Change Ownership message
#       CCSM$snd_susp   - Pack and send Suspend Ownership message
#       CCSM$snd_sdone  - Pack and send Ownership Suspended message
#       CCSM$snd_cdone  - Pack and send Ownership Changed message
#       CCSM$snd_term   - Pack and send Terminate Ownership message
#       CCSM$snd_tdone  - Pack and send Ownership Terminated message
#       CCSM$snd_readrm - Pack and send Read Dirty Region Map message
#       CCSM$snd_rm     - Pack and send Dirty Region Map message
#       CCSM$snd_readsm - Pack and send Read Dirty Segment Map message
#       CCSM$snd_sm     - Pack and send Dirty Segment Map message
#       CCSM$snd_swap   - Pack and send Swap RAIDs message
#       CCSM$snd_swapdone - Pack and send Swap RAIDs Completed message
#       CCSM$snd_disvlchk - Pack and send Disable VLink Check message
#       CCSM$snd_disfresh - Pack and send Disable Refresh NVRAM message
#       CCSM$snd_envlchk - Pack and send Enable VLink Check message
#       CCSM$snd_enfresh - Pack and send Enable Refresh NVRAM message
#       CCSM$snd_cdmoved - Pack and send Copy Devices Moved message
#       CCSM$snd_cdmack - Pack and send Copy Devices Moved Ack message
#       CCSM$snd_csc    - Pack and send Copy State Changed message
#       CCSM$snd_cscack - Pack and send Copy State Changed Ack message
#       CCSM$snd_info   - Pack and send Copy General Info. Changed message
#       CCSM$snd_infoack- Pack and send Copy General Info. Changed Ack message
#       CCSM$CCTrans    - Event transition through the C.C.S.E.
#       CCSM$OCTrans    - Event transition through the O.C.S.E.
#       CCSM$get_seqnum - Allocate a sequence # for CCBGram being sent
#       CCSM$CheckOwner - Check who's owner of a copy
#       CCSM$chk4resources - Checks for copy resources
#       CCSM$cdm_delta  - Process differences between local COR and
#                         Copy Devices Moved device definitions
#       CCSM$strcomp    - Byte-by-byte memory compare
#       CCSM$p2update   - Schedule NVRAM update for non-critical saves
#       CCSM$savenrefresh- Save P2 NVRAM and refresh slave controllers
#
#       This module contains then following O.C.S.E. routines:
#
#       CCSM$ResetOCSE  - Reset O.C.S.E. fields in COR for the start of
#                         copy operation or transfer ownership.
#       CCSM$Ready_Copy - Ready copy event handler routine
#       CCSM$Test4Resources - Test for copy resources routine
#       CCSM$Define_own - Define self as copy owner
#       CCSM$CopyTerminated - Copy has terminated handler routine
#       CCSM$SuspCurOwn - Suspend current copy owner handler routine
#       CCSM$CurOwnSusp - Current copy owner suspended handler routine
#       CCSM$RtyReadRM  - Retry Read Region Map handler routine
#       CCSM$DirtyRM    - Dirty region map data received handler routine
#       CCSM$RtyTermOwn - Retry Terminate Ownership handler routine
#       CCSM$OwnTerm    - Previous copy ownership terminated handler routine
#       CCSM$OwnerAcq   - Ownership acquired handler routine
#       CCSM$SuspOwn    - Suspend Ownership handler routine
#       CCSM$SuspOwn2   - Suspend Ownership handler routine when in
#                         suspend ownership pending state
#       CCSM$SuspOwn3   - Suspend Ownership handler routine when in
#                         ownership suspended state
#       CCSM$OwnSusp    - Ownership Suspended handler routine
#       CCSM$RDRMap     - Read Dirty Region Map handler routine
#       CCSM$TermOwn    - Terminate Ownership handler routine
#       CCSM$OwnChanged - Ownership Changed handler routine
#       CCSM$OpTimeout  - Current operation timeout occurred handler routine
#       CCSM$ProcTimeout- Process timeout occurred handler routine
#       CCSM$ResetTransRM - Reset transfer ownership region map subroutine
#       CCSM$DeallocSM  - Deallocate dirty segment map data routine
#       CCSM$RDSMap     - Read Dirty Segment Map handler routine
#       CCSM$DirtySM    - Dirty segment map data received handler routine
#       CCSM$RtyReadSM  - Retry Read Segment Map handler routine
#       CCSM$GetNextSM  - Get next dirty region segment map routine
#       CCSM$ForceOwn   - Force ownership change handler routine
#       CCSM$IAmOwner   - I am copy owner handler routine
#       CCSM$EndProcess - Process ending cleanup routine
#       CCSM$SuspCCSE   - Suspend C.C.S.E. state machine routine
#       CCSM$ClrRMs     - Clear all region maps routine
#       CCSM$Clrflags   - Clear cor_flags bits that disable updates
#       CCSM$ClrRM_flags- Does CCSM$ClrRMs & CCSM$Clrflags
#
#       This module employs processes:
#
#       ccsm$exec       - CCSM service provider process (one copy)
#       ccsm$swapexec   - Swap RAIDs CCSM service provider process (one copy)
#       ccsm$timer      - CCSM timer process (one copy)
#
#  Copyright (c) 2004-2010 XIOtech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- assembly options ------------------------------------------------
#
        .set    ccsm_traces,TRUE        # CCSM traces assembly option flag
        .set    ccsm_trace_size,0x2000  # size of trace area if enabled
        .set    djk_testccb,TRUE        # test CCBGrams being discarded
        .set    djk_ccorip,TRUE         # save CCSM$cco return IP address
        .set    djk_deftr,TRUE          # Define Ownership owner trace

#
# --- local equates ---------------------------------------------------
#
        .set    ccsm_trace_flags,0xffffbffb # trace flag settings
                                        # Bit 15 = 1=filter incoming CCBGrams
                                        #     14 = 0=filter CIupdate events
                                        #     13 = 1=timestamps in traces
                                        #     12 =
                                        #     11 =
                                        #     10 =
                                        #      9 =
                                        #      8 =
                                        #      7 =
                                        #      6 = 1=send CCBGram event data 2
                                        #      5 = 1=send CCBGram event
                                        #      4 = 1=ccsm$exec event data 2
                                        #      3 = 1=state engine traces
                                        #      2 = 0=get/put CWIP events
                                        #      1 = 1=client I/F event received
                                        #      0 = 1=ccsm$exec event received
        .set    ccsm_cco_to,3           # # seconds after last cco event has
                                        #  been received before the event is
                                        #  processed.
        .set    ccsm_transprocrty,5     # transfer ownership process retry
                                        #  count
        .set    ccsm_transprocrtysf,1   # transfer ownership process retry
                                        # count at all BE failure at other site
                                        # GEORAID15
        .set    ccsm_readrmoprty,5      # read region map operation retry
                                        #  count
        .set    ccsm_readsmoprty,5      # read segment map operation retry
                                        #  count
        .set    ccsm_termownrty,5       # terminate ownership operation
                                        #  retry count
        .set    ccsm_vlchkto,30         # disable VLink check timeout (secs.)
        .set    ccsm_freshto,30         # disable refresh NVRAM timeout (secs.)
        .set    ccsm_swapto,10          # swap RAIDs timeout value (secs.)
        .set    ccsm_swaprty,10         # swap RAIDs retry count
        .set    ccsm_usrswaprty,60      # user_swap RAIDs retry count
        .set    ccsm_supd_to,10         # slave controller copy update
                                        #  process check timer (secs.)
        .set    ccsm_mupd_to,3          # master controller copy update
                                        #  process check timer (secs.)
        .set    ccsm_ocse_to1,10        # O.C.S.E. process timeout #1 (secs.)
                                        #  used when acquiring ownership
        .set    ccsm_ocse_to2,30        # O.C.S.E. process timeout #2 (secs.)
                                        #  used when relinquishing ownership
# --- Swap completion status
#
        .set   gr_swap_failed,3
        .set   gr_swap_success,2
#
# --- global function declarations ------------------------------------
#
        .globl  CCSM$init       # CCSM initialization routine.
        .globl  CCSM$new_master # New master CCSM (define event)
        .globl  CCSM$not_master # No linger master CCSM (define event)
        .globl  CCSM$start_copy # Start copy (define event)
        .globl  CCSM_start_copy # Start copy (define event)
        .globl  CCSM$term_copy  # Terminate copy (define event)
        .globl  CCSM_term_copy  # Terminate copy (define event)
        .globl  CCSM$pause_copy # Pause copy (define event)
        .globl  CCSM$resume_copy # Resume copy (define event)
        .globl  CCSM_resume_copy
        .globl  CCSM$swap_raids # Swap RAIDs (define event)
        .globl  CCSM_swap_raids # Swap RAIDs (define event)
        .globl  CCSM$cco        # Configuration change occurred (define event)
        .globl  CCSM_cco        # Configuration change occurred (define event)
        .globl  CCSM_cosc       # Copy operational state changed (define event)
        .globl  CCSM$timer      # 1 sec. timer event (define event)
        .globl  CCSM$swaptimer  # 1 sec. timer event for swap RAIDs process
        .globl  CCSM$upd_check  # Copy update process check
        .globl  CCSM$ccbg       # CCBGram received (define event)
        .globl  CCSM$update     # Update copy (% complete) (client I/F)
        .globl  CCSM$upause     # Copy user paused (client I/F)
        .globl  CCSM$apause     # Copy auto-paused (client I/F)
        .globl  CCSM$resume     # Copy has resumed (client I/F)
        .globl  CCSM$mirror     # Copy mirrored (client I/F)
        .globl  CCSM_Copy_ended # Copy terminated
        .globl  CCSM$ended      # Copy terminated (client I/F)
        .globl  CCSM$reg        # Copy registered (client I/F)
        .globl  CCSM$reg_sync   # Region synchronized (client I/F)
        .globl  CCSM$reg_dirty  # Region dirty (client I/F)
        .globl  CCSM$get_cwip   # Get CWIP record (client I/F)
        .globl  CCSM$put_cwip   # Put CWIP record (client I/F)
        .globl  CCSM$cd_moved   # Copy devices moved (client I/F)
        .globl  CCSM$cs_chged_w_msg # Copy state changed w/message (client I/F)
        .globl  CCSM$cs_chged   # Copy state changed (client I/F)
        .globl  CCSM$info_chged # Copy general information changed (client I/F)
        .globl  CCSM$rpoll_term # Remote COR poll terminate copy (client I/F)
        .globl  CCSM$snd_copyreg # Pack and send Copy Registered message
        .globl  CCSM$snd_process # Pack and send Process message
        .globl  CCSM$snd_update # Pack and send Update message
        .globl  CCSM$snd_force  # Pack and send Force Ownership Change message
        .globl  CCSM$snd_define # Pack and send Define Ownership message
        .globl  CCSM$snd_owner  # Pack and send You Are Owner message
        .globl  CCSM$snd_trans  # Pack and send Transfer Ownership message
        .globl  CCSM$snd_change # Pack and send Change Ownership message
        .globl  CCSM$snd_susp   # Pack and send Suspend Ownership message
        .globl  CCSM$snd_sdone  # Pack and send Ownership Suspended message
        .globl  CCSM$snd_cdone  # Pack and send Ownership Changed message
        .globl  CCSM$snd_term   # Pack and send Terminate Ownership message
        .globl  CCSM$snd_tdone  # Pack and send Ownership Terminated message
        .globl  CCSM$snd_readrm # Pack and send Read Dirty Region Map message
        .globl  CCSM$snd_rm     # Pack and send Dirty Region Map message
        .globl  CCSM$snd_readsm # Pack and send Read Dirty Segment Map message
        .globl  CCSM$snd_sm     # Pack and send Dirty Segment Map message
        .globl  CCSM$snd_swap   # Pack and send Swap RAIDs message
        .globl  CCSM$snd_swapdone # Pack and send Swap RAIDs Completed message
        .globl  CCSM$snd_disvlchk # Pack and send Disable VLink Check message
        .globl  CCSM$snd_disfresh # Pack and send Disable Refresh NVRAM message
        .globl  CCSM$snd_envlchk # Pack and send Enable VLink Check message
        .globl  CCSM$snd_enfresh # Pack and send Enable Refresh NVRAM message
        .globl  CCSM$snd_cdmoved # Pack and send Copy Devices Moved message
        .globl  CCSM$snd_cdmack # Pack and send Copy Devices Moved Ack message
        .globl  CCSM$snd_csc    # Pack and send Copy State Changed message
        .globl  CCSM$snd_cscack # Pack and send Copy State Changed Ack message
        .globl  CCSM$snd_info   # Pack and send Copy General Info. Changed
                                #  message
        .globl  CCSM$snd_infoack # Pack and send Copy General Info. Changed
                                #  Ack message
        .globl  CCSM$CCTrans    # Event transition through the C.C.S.E.
        .globl  CCSM$OCTrans    # Event transition through the O.C.S.E.
        .globl  CCSM$get_seqnum # Allocate a sequence # for CCBGram being sent
        .globl  CCSM$CheckOwner # Check who's owner of a copy
        .globl  CCSM$chk4resources # Checks for copy resources
        .globl  CCSM$cdm_delta  # Process differences between local COR and
                                #  Copy Devices Moved device definitions
        .globl  CCSM$strcomp    # Byte-by-byte memory compare
        .globl  CCSM$p2update   # Schedule NVRAM update for non-critical saves
#
        .globl  CCSM$ResetOCSE  # Reset O.C.S.E. fields in COR for the start
                                #  of copy operation or transfer ownership.
        .globl  CCSM$Ready_Copy # Ready copy event handler routine
        .globl  CCSM$Test4Resources # Test for copy resources routine
        .globl  CCSM$Define_own # Define self as copy owner
        .globl  CCSM$CopyTerminated # Copy has terminated handler routine
        .globl  CCSM$SuspCurOwn # Suspend current copy owner handler routine
        .globl  CCSM$CurOwnSusp # Current copy owner suspended handler routine
        .globl  CCSM$RtyReadRM  # Retry Read Region Map handler routine
        .globl  CCSM$DirtyRM    # Dirty region map data received handler
                                #  routine
        .globl  CCSM$RtyTermOwn # Retry Terminate Ownership handler routine
        .globl  CCSM$OwnTerm    # Previous copy ownership terminated handler
                                #  routine
        .globl  CCSM$OwnerAcq   # Ownership acquired handler routine
        .globl  CCSM$SuspOwn    # Suspend Ownership handler routine
        .globl  CCSM$SuspOwn2   # Suspend Ownership handler routine when in
                                #  suspend ownership pending state
        .globl  CCSM$SuspOwn3   # Suspend Ownership handler routine when in
                                #  ownership suspended state
        .globl  CCSM$OwnSusp    # Ownership Suspended handler routine
        .globl  CCSM$RDRMap     # Read Dirty Region Map handler routine
        .globl  CCSM$TermOwn    # Terminate Ownership handler routine
        .globl  CCSM$OwnChanged # Ownership Changed handler routine
        .globl  CCSM$OpTimeout  # Current operation timeout occurred
                                #  handler routine
        .globl  CCSM$ProcTimeout # Process timeout occurred handler routine
        .globl  CCSM$ResetTransRM # Reset transfer ownership region map
                                #   subroutine
        .globl  CCSM$DeallocSM  # Deallocate dirty segment map data routine
        .globl  CCSM$RDSMap     # Read Dirty Segment Map handler routine
        .globl  CCSM$DirtySM    # Dirty segment map data received handler
                                #  routine
        .globl  CCSM$RtyReadSM  # Retry Read Segment Map handler routine
        .globl  CCSM$GetNextSM  # Get next dirty region segment map routine
        .globl  CCSM$ForceOwn   # Force ownership change handler routine
        .globl  CCSM$IAmOwner   # I am copy owner handler routine
        .globl  CCSM$EndProcess # Process ending cleanup routine
        .globl  CCSM$SuspCCSE   # Suspend C.C.S.E. state machine routine
        .globl  CCSM$ClrRMs     # Clear all region maps routine
        .globl  CCSM$Clrflags   # Clear cor_flags bits that disable updates
        .globl  CCSM$ClrRM_flags # Does CCSM$ClrRMs & CCSM$Clrflags
        .globl  CCSM$LogAqrOwnrshp   # Log Acquired ownership Message
        .globl  CCSM$LogOwnrshpTerm  # Log Ownership Terminated Message
        .globl  CCSM$LogOwnrshpForce # Log Ownership Forced Message
#       .globl GR_NotifySwapComp   # Notify the Autoswap module that swap is complete
        .globl  CCSM_savenrefresh  # Save in master NVRAM and refresh the slaves
#
# --- Global Data Variables -------------------------------------------
#
        .globl  CCSM_fresh_flag # Disable refresh NVRAM flag
        .globl  CCSM_mupd_timer # master update process timer

        .globl  ccsm_tr_area    # CCSM trace area start
        .globl  ccsm_tr_cur     # CCSM trace (next on) pointer
        .globl  ccsm_tr_head    # CCSM trace buffer start pointer
        .globl  ccsm_tr_tail    # CCSM trace buffer end pointer
        .globl  g_ccsm_op_state # ccsm operational state
#
# --- global function used declarations -------------------------------
#
        .globl  K$cque                  # Common queuing routine
#
# --- global usage data definitions -----------------------------------
#
        .data
        .align  4                       # align just in case
#
#
# --- CCSM service provider queue data structure
#
ccsm_sp_qu:
        .word   0                       # queue head pointer        <w>
        .word   0                       # queue tail pointer        <w>
        .word   0                       # queue count               <w>
        .word   0                       # associated pcb            <w>
#
# --- Swap RAIDs CCSM service provider queue data structure
#
ccsmswap_sp_qu:
        .word   0                       # queue head pointer        <w>
        .word   0                       # queue tail pointer        <w>
        .word   0                       # queue count               <w>
        .word   0                       # associated pcb            <w>
#
ccsm_define_hand:
        .word   ccsm_define1            # current define event handler
                                        #  routine table
ccsm_ccbg_hand:
        .word   ccsm_ccbg1              # current CCBGram event handler
                                        #  routine table
ccsm_bad_etype:
        .word   0                       # bad event type code error counter
ccsm_bad_fc:
        .word   0                       # bad function code error counter
#'C' access to ccsm_op_state
g_ccsm_op_state:
ccsm_op_state:
        .byte   0                       # CCSM operational state code
#
ccsm_cco:
        .byte   0                       # configuration change occurred
                                        #  debounce timeout
                                        # 00 = no pending cco event
                                        #~00 = # seconds until cco event
                                        #      is to be processed
ccsm_seqnum:
        .short  1                       # CCBGram sequence #
ccsm_vlchk_flag:
        .byte   0                       # disable VLink check flag
ccsm_vlchk_to:
        .byte   0                       # disable VLink check timer
CCSM_fresh_flag:
        .byte   0                       # disable refresh NVRAM flag
ccsm_fresh_to:
        .byte   0                       # disable refresh NVRAM timer
ccsm_swap_to:
        .byte   0                       # swap RAIDs process timer
ccsm_supd_timer:
        .byte   ccsm_supd_to            # slave update process check timer
CCSM_mupd_timer:
        .byte   0                       # master update process check timer
ccsm_mupd_start:
        .byte   0                       # count of update process starts
ccsm_mupd_saves:
        .byte   0                       # count of times NVRAM saves performed
        .byte   0,0,0                   # unused
ccsm_swaphead:
        .word   0                       # swap RAIDs request queue head
ccsm_swaptail:
        .word   0                       # swap RAIDs request queue tail
ccsm_bad_vdown:
        .word   0                       # count of copy vd_owner mismatch
ccsm_bad_cwip:
        .word   0                       # count of bad CWIP record address
ccsm_cwip_cnt:
        .word   0                       # CWIP record count
#
# --- CCSM trace area
#
.if     ccsm_traces
#
ccsm_tr_area:
ccsm_tr_flags:
        .word   ccsm_trace_flags        # trace flags
ccsm_tr_cur:
        .word   0                       # current trace pointer
ccsm_tr_head:
        .word   0                       # trace area head pointer
ccsm_tr_tail:
        .word   0                       # trace area tail pointer
#
.endif  # ccsm_traces
#
.if     djk_testccb
#
ccsm_ccbtest:
        .word   0xffffffff              # CCBGram discard test word
                                        # The CCBGram function code is used
                                        #  to test the corresponding flag in
                                        #  this word and if it tests a 0, the
                                        #  incoming CCBGram is discarded
#
.endif  # djk_testccb
#
#**********************************************************************
#
#  NAME: ccsm$timer
#
#  PURPOSE:
#
#       To provide a means of generating a 1 second timer event for the
#       Copy Configuration and Status Manager process.
#
#  DESCRIPTION:
#
#       This routine generates a 1 second timer define event to the
#       main CCSM process (ccsm_exec) every second.
#
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
        .text
#
# --- CCSM timer process initialization code
#
ccsm$timer:
        ldconst 10,r15                  # load 10 seconds
#
# --- Put this process into a 1 second delay
#
.tex10:
        ldconst 1000,g0                 # g0 = timeout value
        call    K$twait                 # wait for timeout to expire
#
# --- These routines execute every second
#
        call    CCSM$timer              # generate a 1 second timer event
                                        #  for the main CCSM process
        call    CCSM$swaptimer          # process timer for swap RAIDs CCSM
                                        #  process
        call    CCSM$upd_check          # check copy update process
#
# --- This logic forms a 10 second loop.
#
        subo    1,r15,r15               # decrement second count
        cmpobne 0,r15,.tex10            # Jif 10 second loop not to be
                                        #     activated
#
# -- These routines execute every 10 seconds
#
        call    CCSM$RefreshTimer       # check for possible refresh loss

        b       ccsm$timer              # and sleep for another second
#
#**********************************************************************
#
#  NAME: ccsm$swapexec
#
#  PURPOSE:
#
#       To provide a means of processing Swap RAIDs requests for
#       copy operations.
#
#  DESCRIPTION:
#
#       This process provides the logic to perform swap RAIDs
#       associated with copy operations.
#
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
# --- Swap RAIDs CCSM process initialization code
#
ccsm$swapexec:
#
        b       .sex15
#
# --- Set this process to not ready
#
.sex10:
        ldconst pcnrdy,r4               # Set this process to not ready
        stob    r4,pc_stat(r15)
#
# --- Exchange processes ----------------------------------------------
#
.sex15:
        call    K$qxchang               # Exchange processes
#
# --- Get next queued request
#
        lda     ccsmswap_sp_qu,r11      # Get swap RAIDs CCSM service provider
                                        #  queue pointer
        ldq     qu_head(r11),r12        # r12 = queue head pointer
                                        # r13 = queue tail pointer
                                        # r14 = queue count
                                        # r15 = PCB address
        mov     r12,g1                  # g1 = event ILT being processed
        cmpobe.f 0,r12,.sex10           # Jif none
#
# --- Remove this request from queue ----------------------------------
#
        ld      il_fthd(r12),r12        # r12 = next ILT on queue
        cmpo    0,r12                   # Check for queue now empty
        subo    1,r14,r14               # Adjust queue count
        sele    r13,r12,r13             # Set up queue tail
        stt     r12,qu_head(r11)        # Update queue head, tail and count
        be.f    .sex30                  # Jif queue now empty
#
        st      r11,il_bthd(r12)        # Update backward thread
#
.sex30:
#
.if     ccsm_traces
#
# --- Trace record format -------------------------------------------------
#
#       <b> Trace record type code (0x44)
#       <b> 00
#       <s> Event length
#       <b> Event type code
#       <b> Event function code
#       <s> Sequence #
#      8<b> 1st 8 bytes of event
#
        ldq     ccsm_tr_flags,r4        # r4 = trace flags
                                        # r5 = current trace pointer
                                        # r6 = trace head pointer
                                        # r7 = trace tail pointer
        bbc     0,r4,.sex30z2           # Jif trace disabled
        ldq     ccsm_e_len(g1),r8       # r8-r11 = first 16 bytes of event
        ldob    ccsm_e_type(g1),r12     # r12 = event type code
        ldob    ccsm_e_fc(g1),r13       # r13 = function code
        cmpobne ccsm_et_define,r12,.sex30d # Jif not define event type
#
# --- Define event type checking
#
        cmpobe  ccsm_de_timer,r13,.sex30z2 # Ignore tracing timer events
        b       .sex30g
#
.sex30d:
#
# --- Not a define event type
#
.sex30g:
        bswap   r8,r8
        ldconst 0x44,r3                 # r3 = trace record type code
        addo    r3,r8,r8
        stq     r8,(r5)
        lda     16(r5),r5
        cmpobg  r7,r5,.sex30x
        mov     r6,r5                   # set current pointer to head
.sex30x:
        st      r5,ccsm_tr_cur
#
#
# --- Trace record format -------------------------------------------------
#
#     12<b> bytes 9-21 of event
#       <w> timestamp
#
        bbc     4,r4,.sex30z2           # Jif trace disabled
        ldq     ccsm_e_len+16(g1),r8    # r8-r11 = second 16 bytes of event
        bbc     13,r4,.sex30m           # Jif timestamps disabled
c       r11 = get_tsc_l() & ~0xf; # Get free running bus clock.
.sex30m:
        stq     r8,(r5)
        lda     16(r5),r5
        cmpobg  r7,r5,.sex30x2
        mov     r6,r5                   # set current pointer to head
.sex30x2:
        st      r5,ccsm_tr_cur
.sex30z2:
#
.endif  # ccsm_traces
#
        ldob    ccsm_e_type(g1),r4      # r4 = event type code
        cmpobe  ccsm_et_ccbg,r4,.sex60  # Jif CCBGram event type code
        cmpobe  ccsm_et_define,r4,.sex50 # Jif define event type code
        ld      ccsm_bad_etype,r6       # r6 = bad event type code error
                                        #  counter
        addo    1,r6,r6                 # inc. error counter
        st      r6,ccsm_bad_etype       # save updated error counter
.sex40:
        ld      il_cr(g1),r4            # r4 = completion handler routine
        callx   (r4)                    # return event to requestor
        b       .sex15                  # and check for more work
#
# --- Define event type code handler
#
.sex50:
        ldob    ccsm_e_fc(g1),r4        # r4 = event function code
        lda     ccsm$sex_swap,r6        # r6 = swap RAIDs handler routine
        cmpobe  ccsm_de_swap,r4,.sex100 # Jif swap RAIDs function
        lda     ccsm$sex_timer,r6       # r6 = timer event handler routine
        cmpobe  ccsm_de_timer,r4,.sex100 # Jif timer event
.sex55:
        ld      ccsm_bad_fc,r6          # r6 = bad function code error
                                        #  counter
        addo    1,r6,r6                 # inc. error counter
        st      r6,ccsm_bad_fc          # save updated error counter
        b       .sex40
#
# --- CCBGram event type code handler
#
.sex60:
        ldob    ccsm_e_fc(g1),r4        # r4 = event function code
        ldconst ccsm_gr_swapdone,r5
        lda     ccsm$sex_swapdone,r6    # r6 = swap RAIDs completed handler
                                        #      routine
        cmpobe  r4,r5,.sex100           # Jif swap RAIDs completed
        b       .sex55                  # ignore invalid CCBGram message
#
# --- Valid request function code.
#
.sex100:
        ldob    ccsm_op_state,r4        # r4 = CCSM state code
        cmpobe ccsm_st_master,r4,.sex200 # Jif master CCSM
#
# --- We are not the master CCSM. We may need to transfer any swap
#       RAIDs operations to the new master CCSM, but for now just
#       flush the local swap RAIDs queue.
#
        mov     g1,r5                   # save g1
.sex120:
        ld      ccsm_swaphead,g1        # g1 = top swap RAIDs request on queue
        cmpobe  0,g1,.sex180            # Jif no swap RAIDs ILTs on queue
        ld      il_fthd(g1),r4          # r4 = next ILT on queue
        st      r4,ccsm_swaphead        # remove ILT from queue
        cmpobne 0,r4,.sex140            # Jif more ILTs on queue
        st      r4,ccsm_swaptail        # clear queue tail pointer
.sex140:
        ldconst 0,r4
        st      r4,il_fthd(g1)          # clear forward thread of ILT
        ld      il_cr(g1),r4            # r4 = completion handler routine
        callx   (r4)                    # return event to requestor
        b       .sex120                 # and check for more ILTs on queue
#
.sex180:
        mov     r5,g1                   # restore g1
        b       .sex40
#
#******************************************************************************
#
#       Interface to Copy Manager service provider handler routines.
#
#  INPUT:
#
#       g1 = event ILT
#
#  OUTPUT:
#
#       g1 = 0 if event ILT not to be returned
#       g1 = ILT address if ILT to be returned
#
#  REGS DESTROYED:
#
#       All regs. can be destroyed.
#
#******************************************************************************
#
.sex200:
        callx   (r6)                    # and go to request handler routine
        cmpobe  0,g1,.sex15             # Jif g1 = 0
        ld      il_cr(g1),r4            # r4 = completion handler routine
        callx   (r4)                    # return event to requestor
        b       .sex15                  # check for more work to do
#
#**********************************************************************
#
#  NAME: ccsm$exec
#
#  PURPOSE:
#
#       To provide a means of processing Copy Configuration and Status
#       Manager services.
#
#  DESCRIPTION:
#
#       This process provides the copy configuration and status management
#       services for all copy operations using resync. CCSM coordinates copy
#       functions across multiple nodes as needed to service the needs of
#       the copy operations. A master CCSM service provider is defined as the
#       CCSM process that operates on the current master controller in the DSC.
#       The master CCSM moves from controller to controller as the master
#       controller changes. Slave CCSM processes communicate with the master
#       CCSM process as needed to communicate copy operation information for
#       copies that are owned by CM tasks on slave controllers. The Master
#       CCSM process is the only CCSM in the system that can save copy
#       configuration information in NVRAM. As such, any copy configuration
#       information that needs to be saved in configuration NVRAM must be
#       received by the master CCSM in order for it to be saved. Also, copy
#       operational data which is displayed by the user interface must be
#       sent to the master CCSM process since the master controller is the
#       controller where all user interface information is received. Each
#       CCSM process maintains it's own copy state NVRAM area and copy work
#       in progress (CWIP) NVRAM area. The initial phase of resync on 3D will
#       not mirror the copy state or CWIP NVRAM areas and therefore when copies
#       move from controller to controller this information must be retrieved
#       from the previous copy owner node or the copy must be restarted in
#       order to insure data integrity of the destination copy device. This
#       may change in future releases.
#
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
# --- CCSM process initialization code
#
ccsm$exec:
#
# --- Allocate memory for the trace area if enabled
#
.if     ccsm_traces
c       g0 = s_MallocC(ccsm_trace_size|BIT31, __FILE__, __LINE__);
                                        # g0 = trace area header address
        lda     16(g0),r4               # r4 = trace area current pointer
        lda     ccsm_trace_size-16(r4),r6 # r6 = trace area tail pointer
        mov     r4,r5                   # r5 = trace area head pointer
        stt     r4,ccsm_tr_cur          # setup trace area pointers
#
.endif  # ccsm_traces
#
        b       .ex15
#
# --- Set this process to not ready
#
.ex10:
        ldconst pcnrdy,r4               # Set this process to not ready
        stob    r4,pc_stat(r15)
#
# --- Exchange processes ----------------------------------------------
#
.ex15:
        call    K$qxchang               # Exchange processes
#
# --- Get next queued request
#
        lda     ccsm_sp_qu,r11          # Get CCSM service provider
                                        #  queue pointer
        ldq     qu_head(r11),r12        # r12 = queue head pointer
                                        # r13 = queue tail pointer
                                        # r14 = queue count
                                        # r15 = PCB address
        mov     r12,g1                  # g1 = event ILT being processed
        cmpobe.f 0,r12,.ex10            # Jif none
#
# --- Remove this request from queue ----------------------------------
#
        ld      il_fthd(r12),r12        # r12 = next ILT on queue
        cmpo    0,r12                   # Check for queue now empty
        subo    1,r14,r14               # Adjust queue count
        sele    r13,r12,r13             # Set up queue tail
        stt     r12,qu_head(r11)        # Update queue head, tail and count
        be.f    .ex30                   # Jif queue now empty
#
        st      r11,il_bthd(r12)        # Update backward thread
#
.ex30:
#
.if     ccsm_traces
#
# --- Trace record format -------------------------------------------------
#
#       <b> Trace record type code (0x40)
#       <b> 00
#       <s> Event length
#       <b> Event type code
#       <b> Event function code
#       <s> Sequence #
#      8<b> 1st 8 bytes of event
#
        ldq     ccsm_tr_flags,r4        # r4 = trace flags
                                        # r5 = current trace pointer
                                        # r6 = trace head pointer
                                        # r7 = trace tail pointer
        bbc     0,r4,.ex30z2            # Jif trace disabled
        ldq     ccsm_e_len(g1),r8       # r8-r11 = first 16 bytes of event
        ldob    ccsm_e_type(g1),r12     # r12 = event type code
        ldob    ccsm_e_fc(g1),r13       # r13 = function code
        cmpobne ccsm_et_define,r12,.ex30d # Jif not define event type
#
# --- Define event type checking
#
        cmpobe  ccsm_de_timer,r13,.ex30z2 # Ignore tracing timer events
        b       .ex30g
#
.ex30d:
#
# --- Not a define event type
#
        bbc     15,r4,.ex30g            # Jif not filtering CCBGram events
        cmpobne ccsm_et_ccbg,r12,.ex30g # Jif not CCBGram event
        ldob    ccsm_ccbgram_filter(r13),r12
        cmpobe  0,r12,.ex30z2           # Jif event being filtered
.ex30g:
        bswap   r8,r8
        ldconst 0x44,r3                 # r3 = trace record type code
        addo    r3,r8,r8
        stq     r8,(r5)
        lda     16(r5),r5
        cmpobg  r7,r5,.ex30x
        mov     r6,r5                   # set current pointer to head
.ex30x:
        st      r5,ccsm_tr_cur
#
#
# --- Trace record format -------------------------------------------------
#
#     12<b> bytes 9-21 of event
#       <w> timestamp
#
        bbc     4,r4,.ex30z2            # Jif trace disabled
?       ldq     ccsm_e_len+16(g1),r8    # r8-r11 = second 16 bytes of event
        bbc     13,r4,.ex30m            # Jif timestamps disabled
c       r11 = get_tsc_l() & ~0xf; # Get free running bus clock.
.ex30m:
        stq     r8,(r5)
        lda     16(r5),r5
        cmpobg  r7,r5,.ex30x2
        mov     r6,r5                   # set current pointer to head
.ex30x2:
        st      r5,ccsm_tr_cur
.ex30z2:
#
.endif  # ccsm_traces
#
        ldob    ccsm_e_type(g1),r4      # r4 = event type code
        cmpobe  ccsm_et_ccbg,r4,.ex60   # Jif CCBGram event type code
        cmpobe  ccsm_et_define,r4,.ex50 # Jif define event type code
        ld      ccsm_bad_etype,r6       # r6 = bad event type code error
                                        #  counter
        addo    1,r6,r6                 # inc. error counter
        st      r6,ccsm_bad_etype       # save updated error counter
.ex40:
        ld      il_cr(g1),r4            # r4 = completion handler routine
        callx   (r4)                    # return event to requestor
        b       .ex15                   # and check for more work
#
# --- Define event type code handler
#
.ex50:
        ldob    ccsm_e_fc(g1),r4        # r4 = event function code
        ld      ccsm_define_hand,r5     # r5 = current define request handler
                                        #      routine table
        ldconst ccsm_def_maxfc,r6       # r6 = max. valid function code
.ex55:
        cmpobge r6,r4,.ex100            # Jif valid request function code
        ld      ccsm_bad_fc,r6          # r6 = bad function code error
                                        #  counter
        addo    1,r6,r6                 # inc. error counter
        st      r6,ccsm_bad_fc          # save updated error counter
        b       .ex40
#
# --- CCBGram event type code handler
#
.ex60:
        ldob    ccsm_e_fc(g1),r4        # r4 = event function code
#
.if     djk_testccb
#
        ld      ccsm_ccbtest,r5         # r5 = CCBGram discard test word
        bbc     r4,r5,.ex40             # Jif flag for this CCBGram indicates
                                        #  to discard incoming message
#
.endif  # djk_testccb
#
        ld      ccsm_ccbg_hand,r5       # r5 = current CCBGram request handler
                                        #      routine table
        ldconst ccsm_ccbg_maxfc,r6      # r6 = max. valid function code
        b       .ex55                   # handler error
#
# --- Valid request function code.
#
.ex100:
        ld      (r5)[r4*4],r6           # r6 = request handler routine
#
#******************************************************************************
#
#       Interface to Copy Manager service provider handler routines.
#
#  INPUT:
#
#       g1 = event ILT
#
#  OUTPUT:
#
#       g1 = 0 if event ILT not to be returned
#       g1 = ILT address if ILT to be returned
#
#  REGS DESTROYED:
#
#       All regs. can be destroyed.
#
#******************************************************************************
#
        callx   (r6)                    # and go to request handler routine
        cmpobe  0,g1,.ex15              # Jif g1 = 0
        ld      il_cr(g1),r4            # r4 = completion handler routine
        callx   (r4)                    # return event to requestor
        b       .ex15                   # check for more work to do
#
.if     ccsm_traces
#
# --- Incoming CCBGram event trace filter definition table
#
        .data
ccsm_ccbgram_filter:
        .byte   1                       # start copy
        .byte   1                       # process
        .byte   0                       # update
        .byte   1                       # force ownership change
        .byte   1                       # define ownership
        .byte   1                       # you are owner
        .byte   1                       # transfer ownership
        .byte   1                       # change ownership
        .byte   1                       # suspend ownership
        .byte   1                       # ownership suspended
        .byte   1                       # ownership changed
        .byte   1                       # terminate ownership
        .byte   1                       # ownership terminated
        .byte   1                       # read dirty region map
        .byte   1                       # dirty region map data
        .byte   1                       # read segment map
        .byte   1                       # segment map data
        .byte   1                       # copy registered
        .byte   1                       # swap RAIDs
        .byte   0xff                    # unused
#
.endif  # ccsm_traces
#
# --- CCSM Define Event Handler Tables and Routines ---------------------------
#
# --- Define Event Handler Table #1
#
#       This table is used to process define events received while the
#       CCSM process is not the master CCSM.
#
ccsm_define1:
        .word   ccsm$def1_newm          # new master CCSM handler routine
        .word   ccsm$def1_nom           # no longer master CCSM handler
                                        #  routine
        .word   ccsm$def1_start         # start copy handler routine
        .word   ccsm$def1_term          # terminate copy handler routine
        .word   ccsm$def1_pause         # pause copy handler routine
        .word   ccsm$def1_resume        # resume copy handler routine
        .word   ccsm$def1_swap          # swap RAIDs handler routine
        .word   ccsm$def1_cco           # configuration change occurred
                                        #  handler routine
        .word   ccsm$def1_cosc          # copy operational state changed
                                        #  handler routine
        .word   ccsm$def1_timer         # 1 sec. timer event handler routine
#
ccsm_define1_end:
        .set    ccsm_def_maxfc,((ccsm_define1_end-ccsm_define1)/4)-1 # maximum valid define
                                                      #  event function code
        .text
#
# --- Define Event Handler Routines -------------------------------------------
#
#**********************************************************************
#
#  NAME: ccsm$def1_newm
#
#  PURPOSE:
#
#       This routine processes a new master CCSM define event.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    ccsm$def1_newm
#
#  INPUT:
#
#       g1 = new master CCSM define event ILT
#
#  OUTPUT:
#
#       g1 = ILT to return
#
#  REGS DESTROYED:
#
#       None.
#
#**********************************************************************
#
ccsm$def1_newm:
        movq    g0,r12                  # save g0-g3
        ldob    ccsm_op_state,r3        # r3 = current CCSM op. state
        cmpobe  ccsm_st_master,r3,.def1newm_1000 # Jif already master
        ldconst ccsm_st_master,r3       # set CCSM state to master
        stob    r3,ccsm_op_state
#
        call    ccsm$NVRAMrefresh       # freshen up NVRAM for the big PARTY!!
#
        call    CCSM$cco                # generate a configuration change
                                        #  occurred event
.def1newm_1000:
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: ccsm$def1_nom
#
#  PURPOSE:
#
#       This routine processes a no longer master CCSM define event.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    ccsm$def1_nom
#
#  INPUT:
#
#       g1 = no longer master CCSM define event ILT
#
#  OUTPUT:
#
#       g1 = ILT to return
#
#  REGS DESTROYED:
#
#       None.
#
#**********************************************************************
#
ccsm$def1_nom:
        movq    g0,r12                  # save g0-g3
        ldob    ccsm_op_state,r3        # r3 = current CCSM op. state
        cmpobe  ccsm_st_notmaster,r3,.def1nom_1000 # Jif already not master
        ldconst ccsm_st_notmaster,r3    # set CCSM state to not master
        stob    r3,ccsm_op_state
        call    CCSM$cco                # generate a configuration change
                                        #  occurred event
.def1nom_1000:
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: ccsm$def1_start
#
#  PURPOSE:
#
#       This routine processes a start copy define event.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    ccsm$def1_start
#
#  INPUT:
#
#       g1 = start copy define event ILT
#
#  OUTPUT:
#
#       g1 = ILT to return
#
#  REGS DESTROYED:
#
#       Reg. g0, g3 destroyed.
#
#**********************************************************************
#
ccsm$def1_start:
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>ccsm$def1_start---ccsm$exec calls this routine\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # CM_IM_DEBUG
        mov     g1,r15                  # save g1
        ld      ccsm_de_start_cor(g1),g3 # g3 = assoc. COR address
        ldconst FALSE,r11               # r11 = update P2 NVRAM flag
        ldob    cor_crstate(g3),r4      # r4 = cor_crstate
        cmpobne corcrst_undef,r4,.def1start_220 # Jif reg. state not
                                        #         initialized
        ldob    ccsm_op_state,r3        # r3 = current CCSM op. state
        cmpobne ccsm_st_master,r3,.def1start_220 # Jif not the master CCSM
        ld      cor_cm(g3),r4           # r4 = assoc. CM address if local COR
        cmpobe  0,r4,.def1start_220     # Jif remote copy
        ldconst 0,g1                    # g1 = registration type code
        call    CM$regcopy              # register copy
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>ccsm$def1_start-returns from CM$regcopy retval=%lx\n", FEBEMESSAGE, __FILE__, __LINE__,g0);
.endif  # CM_IM_DEBUG
        cmpobe  02,g0,.def1start_240    # Jif reg. unsuccessful (not valid)
        ldconst corcrst_active,r4
        cmpobe  0,g0,.def1start_210     # Jif registration was successful
        ldconst corcrst_init,r4
.def1start_210:
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>ccsm$def1_start-setting crstate as %lx... calling Cm$mmc_sflag\n", FEBEMESSAGE, __FILE__, __LINE__,r4);
.endif  # CM_IM_DEBUG
        stob    r4,cor_crstate(g3)      # save new reg. state
        call    CM$mmc_sflag            # update VDD flags

        ldconst TRUE,r11                # indicate need to save P2 NVRAM
.def1start_220:
        ld      cor_stnvram(g3),r4      # r4 = COR state NVRAM address if
                                        #      defined
        cmpobne 0,r4,.def1start_300     # Jif state NVRAM already defined
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>ccsm$def1_start-allocating nvram state record.. calling P6_AllocStRec()\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # CM_IM_DEBUG
#
# --- Allocate a State NVRAM area for this copy.
#
        mov     g3,r4                   # save g3
        PushRegs(r3)                    # Save all "g" registers
        call    P6_AllocStRec           # allocate a state record for this copy
        cmpobne 0,g0,.def1start_250     # Jif record allocated
#
#*** FINISH-Send message to CCB indicating the problem
#
        PopRegs(r3)                     # Restore registers
#
# --- Log copy termination request message and terminate the copy
#
.def1start_240:
        ldconst cmcc_AutoTerm,g0        # g0 = request copy termination message
        call    CM_Log_Completion        # *** Log Message ***



        call    CM$term_cor             # terminate copy operation
#
        PushRegs(r3)                    # Save all "g" registers
        call    DL_SetVDiskOwnership    # redo VDisk ownership after copy
                                        #  terminated
        PopRegsVoid(r3)                 # restore environment
#
        call    RB_SearchForFailedPSDs  # process failed PSDs for potential
                                        #  ownership change
        call    CCSM$cco                # generate a config. change occurred
                                        #  event
        mov     r15,g1                  # restore g1
        ret
#
.def1start_250:
        mov     r4,g3                   # restore g3
        ld      cor_cm(g3),r5           # r5 = assoc. CM address
        cmpobne 0,r5,.def1start_260     # Jif local copy being started
        call    P6_ClrAllTransRM        # clear transfer region map bitmap
                                        #  in state NVRAM
        b       .def1start_290
#
.def1start_260:
        ld   cor_srcvdd(g3), r4         # Get source VDD
c       r4 = CM_VdiskInstantMirrorAllowed((VDD*)r4);
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>ccsm$def1_start--Instant mirror allow flag=%lx...\n", FEBEMESSAGE, __FILE__, __LINE__,r4);
.endif  # CM_IM_DEBUG
        cmpobe  TRUE,r4,.def1start_290
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>ccsm$def1_start--Calling P6_SetAllTrasnRM\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # CM_IM_DEBUG
        call    P6_SetAllTransRM        # initialize the transfer region map
                                        #  bitmap in state NVRAM
.def1start_290:
        PopRegsVoid(r3)                 # restore environment
.def1start_300:
#
# --- Check if COR already on active COR list and if not put it there
#
        ld      CM_cor_act_que,r4       # r4 = first COR on active queue
.def1start_320:
        cmpobe  0,r4,.def1start_400     # Jif no CORs on active queue
        cmpobe  r4,g3,.def1start_500    # Jif COR found on active COR queue
        ld      cor_link(r4),r4         # r4 = next COR on active queue
        b       .def1start_320
#
.def1start_400:
#
# --- Put COR on active COR list
#
        call    CM$act_cor              # activate COR
        ldconst TRUE,r11                # indicate need to save P2 NVRAM
.def1start_500:
#        cmpobne TRUE,r11,.def1start_600 # Jif P2 NVRAM save not indicated
        call    CCSM$savenrefresh       # save P2 and refresh slaves
# .def1start_600:
#
# --- Note: The routine DL_SetVDiskOwnership  should have been called
#           by this point in the process to define virtual/RAID ownership
#           changes due to the newly defined copy operation. If this is
#           not the case, it needs to be called here before searching
#           failed PSDs.
#
        call    RB_SearchForFailedPSDs  # process failed PSDs for potential
                                        #  ownership change
        ldconst Start_Copy,g0           # g0 = event type code
        mov     r15,g1                  # restore g1
        b       CCSM$OCTrans            # generate event to O.C.S.E.
                                        # g0 = event type
                                        # g3 = COR address
#
#**********************************************************************
#
#  NAME: ccsm$def1_term
#
#  PURPOSE:
#
#       This routine processes a terminate copy define event.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    ccsm$def1_term
#
#  INPUT:
#
#       g1 = terminate copy define event ILT
#
#  OUTPUT:
#
#       g1 = ILT to return
#
#  REGS DESTROYED:
#
#       Reg. g0, g2-g3 destroyed.
#
#**********************************************************************
#
ccsm$def1_term:
        mov     g1,r15                  # save g1
                                        # r15 = event ILT
        ld      ccsm_de_term_reg(r15),g0 # g0 = reg. ID
        ld      ccsm_de_term_cmsn(r15),g1 # g1 = CM serial #
        call    CM$find_cor_rid         # find assoc. COR address
        cmpobe  0,g0,.def1term_1000     # Jif no COR assoc. with this event
        mov     g0,g3                   # g3 = assoc. COR address
        ldconst Term_Copy,g0            # g0 = event type code
        call    CCSM$OCTrans            # generate event to O.C.S.E.
                                        # g0 = event type
                                        # g3 = COR address
.def1term_1000:
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: ccsm$def1_pause
#
#  PURPOSE:
#
#       This routine processes a pause copy define event.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    ccsm$def1_pause
#
#  INPUT:
#
#       g1 = pause copy define event ILT
#
#  OUTPUT:
#
#       g1 = ILT to return
#
#  REGS DESTROYED:
#
#       Reg. g0, g2-g3 destroyed.
#
#**********************************************************************
#
ccsm$def1_pause:
        mov     g1,r15                  # save g1
                                        # r15 = event ILT
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>ccsm$def1_pause...entering....\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # CM_IM_DEBUG
        ld      ccsm_de_pause_reg(r15),g0 # g0 = reg. ID
        ld      ccsm_de_pause_cmsn(r15),g1 # g1 = CM serial #
        call    CM$find_cor_rid         # find assoc. COR address
        cmpobe  0,g0,.def1pause_1000    # Jif no COR assoc. with this event
        mov     g0,g3                   # g3 = assoc. COR address
        ldob    cor_crstate(g3),r4      # r4 = current cor_crstate
        cmpobg  corcrst_active,r4,.def1pause_1000 # Ignore if copy not
                                        #            registered
        cmpobe  corcrst_usersusp,r4,.def1pause_1000 # Jif already user
                                        #              suspended
        cmpobe  corcrst_remsusp,r4,.def1pause_1000 # Jif remote suspended
        ldconst corcrst_usersusp,r5     # r5 = new cor_crstate value
        stob    r5,cor_crstate(g3)      # save new cor_crstate value
        call    CM$mmc_sflag            # update VDD flags
        call    CCSM$savenrefresh       # save P2 and refresh slaves
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>ccsm$def1_pause...calling CM$PauseCopy....\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # CM_IM_DEBUG
        call    CM$PauseCopy            # generate event to CM task
                                        # g3 = COR address
.def1pause_1000:
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: ccsm$def1_resume
#
#  PURPOSE:
#
#       This routine processes a resume copy define event.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    ccsm$def1_resume
#
#  INPUT:
#
#       g1 = resume copy define event ILT
#
#  OUTPUT:
#
#       g1 = ILT to return
#
#  REGS DESTROYED:
#
#       Reg. g0, g2-g3 destroyed.
#
#**********************************************************************
#
ccsm$def1_resume:
        mov     g1,r15                  # save g1
                                        # r15 = event ILT
        ld      ccsm_de_resume_reg(r15),g0 # g0 = reg. ID
        ld      ccsm_de_resume_cmsn(r15),g1 # g1 = CM serial #
        call    CM$find_cor_rid         # find assoc. COR address
        cmpobe  0,g0,.def1resume_1000   # Jif no COR assoc. with this event
        mov     g0,g3                   # g3 = assoc. COR address
        ldob    cor_crstate(g3),r4      # r4 = current cor_crstate value
        cmpobne corcrst_usersusp,r4,.def1resume_1000 # Jif not user suspended
        ldconst corcrst_active,r5       # r5 = new cor_crstate value
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>def1_resume-setting crstate as active(2)..Generate Resume event to CM task\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # CM_IM_DEBUG
        stob    r5,cor_crstate(g3)
        call    CM$mmc_sflag            # update VDD flags
        call    CCSM$savenrefresh       # save P2 and refresh slaves
        call    CM$ResumeCopy           # generate event to CM task
                                        # g3 = COR address
.def1resume_1000:
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: ccsm$def1_swap
#
#  PURPOSE:
#
#       This routine processes a swap RAIDs define event.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    ccsm$def1_swap
#
#  INPUT:
#
#       g1 = swap copy define event ILT
#
#  OUTPUT:
#
#       g1 = ILT to return
#
#  REGS DESTROYED:
#
#       Reg. g0, g2-g3 destroyed.
#
#**********************************************************************
#
ccsm$def1_swap:
        mov     g1,r15                  # save g1
                                        # r15 = event ILT
        ld      ccsm_de_swap_reg(r15),g0 # g0 = reg. ID
        ld      ccsm_de_swap_cmsn(r15),g1 # g1 = CM serial #
        call    CM$find_cor_rid         # find assoc. COR address
        cmpobe  0,g0,.def1swap_1000     # Jif no COR assoc. with this event
        mov     g0,g3                   # g3 = assoc. COR address
        call    CM$SwapRaids            # generate event to CM task
                                        # g3 = COR address
.def1swap_1000:
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: ccsm$def1_cco
#
#  PURPOSE:
#
#       This routine processes a configuration change occurred define event.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    ccsm$def1_cco
#
#  INPUT:
#
#       g1 = configuration change occurred define event ILT
#
#  OUTPUT:
#
#       g1 = ILT to return
#
#  REGS DESTROYED:
#
#       None.
#
#**********************************************************************
#
ccsm$def1_cco:
#
.if     ccsm_traces
#
.if     djk_deftr
#
        ld      CM_cor_act_que,r12      # r12 = first COR on active queue
        cmpobe  0,r12,.ccoex30z         # Jif no CORs on active queue
        ldq     ccsm_tr_flags,r4        # r4 = trace flags
                                        # r5 = current trace pointer
                                        # r6 = trace head pointer
                                        # r7 = trace tail pointer
        ld      djk_deftr_bnr,r8        # r8 = trace record type code
.ccoex30f:
        ld      cor_rid(r12),r9
        ld      cor_rcsn(r12),r10
        ld      cor_powner(r12),r11
        stq     r8,(r5)
        lda     16(r5),r5
        cmpobg  r7,r5,.ccoex30x
        mov     r6,r5                   # set current pointer to head
.ccoex30x:
        st      r5,ccsm_tr_cur
        ld      cor_link(r12),r12       # r12 = next COR on active queue
        cmpobne 0,r12,.ccoex30f         # Jif more CORs to process
.ccoex30z:
#
.endif  # djk_deftr
#
.endif  # ccsm_traces
#
        ldconst ccsm_cco_to,r4          # r4 = configuration change occurred
                                        #  debounce timeout value
        stob    r4,ccsm_cco             # set cco timeout value
        ret
#
#**********************************************************************
#
#  NAME: ccsm$def1_cosc
#
#  PURPOSE:
#
#       This routine processes a copy operational state changed define event.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    ccsm$def1_cosc
#
#  INPUT:
#
#       g1 = copy operational state changed define event ILT
#
#  OUTPUT:
#
#       g1 = ILT to return
#
#  REGS DESTROYED:
#
#       Reg. g0, g2-g3 destroyed.
#
#**********************************************************************
#
ccsm$def1_cosc:
        mov     g1,r15                  # save g1
        ld      ccsm_de_cosc_reg(r15),g0 # g0 = reg. ID
        ld      ccsm_de_cosc_cmsn(r15),g1 # g1 = CM serial #
        call    CM$find_cor_rid         # find assoc. COR address
        cmpobe  0,g0,.def1cosc_1000     # Jif no COR assoc. with this event
        mov     g0,g3                   # g3 = assoc. COR address
        ld      cor_cm(g3),r6           # r6 = assoc. CM address
        cmpobe  0,r6,.def1cosc_1000     # Jif no CM assoc. with COR
        ldob    ccsm_de_cosc_crstate(r15),r5 # r5 = cor_crstate being invoked
        ldob    cor_crstate(g3),r4      # r4 = current local cor_crstate
        cmpobe  r4,r5,.def1cosc_1000    # Jif cor_crstates the same
        cmpoble corcrst_remsusp,r5,.def1cosc_1000 # Jif incoming cor_crstate
                                        # invalid
        cmpobge corcrst_init,r5,.def1cosc_1000 # Jif incoming cor_crstate not
                                        # registered
        ldob    cor_flags(g3),r3        # r3 = cor_flags byte
        bbs     CFLG_B_DIS_STATE,r3,.def1cosc_1000  # Jif copy state update disabled
        ld      K_ficb,r3               # FICB
        ld      fi_cserial(r3),r3       # r3 = my controller serial number
        ld      cor_powner(g3),r7       # r7 = copy pri. owner serial #
        cmpobne r3,r7,.def1cosc_30      # Jif I'm not the pri. copy owner
#
# --- Special processing for primary copy owner cor_crstate differences.
#
        cmpobne corcrst_autosusp,r4,.def1cosc_20a # Jif local cor_crstate<>
                                        #          auto-suspend
#
# --- Local cor_crstate = auto-suspend. If incoming cor_crstate = active,
#       ignore incoming state change.
#
        cmpobe  corcrst_active,r5,.def1cosc_1000 # Jif incoming cor_crstate=
                                        #          active
.def1cosc_20a:
.def1cosc_30:
        cmpobne corcrst_autosusp,r5,.def1cosc_50 # Jif incoming cor_crstate
                                        #  not auto-suspended
        ldconst corcrst_active,r5       # set incoming cor_crstate to active
.def1cosc_50:
        cmpobe  r4,r5,.def1cosc_1000    # Jif cor_crstates the same
        stob    r5,cor_crstate(g3)      # save incoming cor_crstate in local
                                        #  COR
        call    CM$mmc_sflag            # update VDD flags
        cmpobl  corcrst_init,r4,.def1cosc_100 # Jif if local cor_crstate
                                        # indicates copy is registered
#
# --- Case: Local copy not registered
#
        cmpobne corcrst_active,r5,.def1cosc_1000 # Jif incoming state is
                                        #  not active
        call    CM$ResumeCopy           # generate event to CM task
                                        # g3 = COR address
        b       .def1cosc_1000          # and we're out of here!
#
.def1cosc_100:
#
# --- Case: Local copy has been registered
#
        cmpoble corcrst_usersusp,r4,.def1cosc_200 # Jif local COR indicates
                                        # user suspended
#
# --- Case: Local copy active
#
        cmpobne corcrst_usersusp,r5,.def1cosc_1000 # Jif incoming state not
                                        #  user suspended
        call    CM$PauseCopy            # generate event to CM task
                                        # g3 = COR address
        b       .def1cosc_1000          # and we're out of here!
#
.def1cosc_200:
#
# --- Case: Local copy user suspended
#
        cmpobne corcrst_active,r5,.def1cosc_1000 # Jif incoming state not
                                        #  active
        call    CM$ResumeCopy           # generate event to CM task
                                        # g3 = COR address
.def1cosc_1000:
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: ccsm$def1_timer
#
#  PURPOSE:
#
#       This routine processes a 1 second timer define event.
#
#  DESCRIPTION:
#
#       This routine processes a 1 second timer event by performing
#       the following:
#
#       1). Checks all CORs on the active list checking if the
#           O.C.S.E. timer is enabled and decrements the timeout
#           value if it is. If this timer has expired, it generates
#           a timeout event into the corresponding O.C.S.E. state
#           engine.
#       2). Checks if the configuration change occurred (CCO) timer
#           has been set and if it has decrements the timer. If this
#           timer has expired, it generates a Configuration Change
#           Occurred Define Event for the ccsm$exec process.
#
#  CALLING SEQUENCE:
#
#       call    ccsm$def1_timer
#
#  INPUT:
#
#       g1 = 1 second timer define event ILT
#
#  OUTPUT:
#
#       g1 = ILT to return
#
#  REGS DESTROYED:
#
#       Reg. g0, g3 destroyed.
#
#**********************************************************************
#
ccsm$def1_timer:
        mov     g1,r15                  # save g1
#
# --- Check the O.C.S.E. timers for all CORs on active list
#
        ld      CM_cor_act_que,r4       # r4 = first COR on active queue
.def1timer_200:
        cmpobe  0,r4,.def1timer_400     # Jif no more CORs on active queue
        ldob    cor_ocseto(r4),r5       # r5 = O.C.S.E. timeout value
        cmpobe  0,r5,.def1timer_250     # Jif timer not set
        subo    1,r5,r5                 # decrement timeout value
        stob    r5,cor_ocseto(r4)       # save timeout value
        cmpobne 0,r5,.def1timer_250     # Jif timer has not expired
#
# --- O.C.S.E. timer has expired for this copy operation
#
        mov     r4,g3                   # g3 = COR being processed
        ldconst Timeout,g0              # g0 = event type code
        call    CCSM$OCTrans            # generate event to O.C.S.E.
                                        # g0 = event type
                                        # g3 = COR address
.def1timer_250:
        ld      cor_link(r4),r4         # r4 = next COR on active queue
        b       .def1timer_200          # and process next COR on active queue
#
.def1timer_400:
        ldob    ccsm_cco,r4             # r4 = configuration change occurred
                                        #  debounce timeout value
        cmpobe  0,r4,.def1timer_500     # Jif timeout not enabled
        subo    1,r4,r4                 # decrement timeout value
        stob    r4,ccsm_cco             # save updated timeout value
        cmpobne 0,r4,.def1timer_500     # Jif cco timeout not expired
#
# --- Generate a Copy Configuration Changed event to all CORs on active list
#
        ld      CM_cor_act_que,r4       # r4 = first COR on active queue
.def1timer_420:
        cmpobe  0,r4,.def1timer_500     # Jif no more CORs on active queue
        mov     r4,g3                   # g3 = COR being processed
        ldconst Cpy_Config_Chg,g0       # g0 = event type code
.if GR_GEORAID15_DEBUG
c fprintf(stderr,"%s%s:%u <GR><def1timer-ccsm.as>calling CCSM$OCTrans with event=CopyConfigChg(Copy_Config_Chg)\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # GR_GEORAID15_DEBUG
        call    CCSM$OCTrans            # generate event to O.C.S.E.
                                        # g0 = event type
                                        # g3 = COR address
        ld      cor_link(r4),r4         # r4 = next COR on active queue
        b       .def1timer_420           # and process next COR on active queue
#
.def1timer_500:
        mov     r15,g1                  # restore g1
        ret
#
# --- End of CCSM Define Event Handler Tables and Routines Area --------------
#
# --- CCSM CCBGram Received Event Handler Tables and Routines ----------------
#
# --- CCBGram Received Event Handler Table #1
#
#       This table is used to process CCBGram events received while the
#       CCSM process is not the master CCSM.
#
        .data
ccsm_ccbg1:
        .word   ccsm$ccb1_start         # start copy
        .word   ccsm$ccb1_process       # process
        .word   ccsm$ccb1_update        # update
        .word   ccsm$ccb1_force         # force ownership change
        .word   ccsm$ccb1_define        # define ownership
        .word   ccsm$ccb1_owner         # you are owner
        .word   ccsm$ccb1_trans         # transfer ownership
        .word   ccsm$ccb1_change        # change ownership
        .word   ccsm$ccb1_susp          # suspend ownership
        .word   ccsm$ccb1_sdone         # ownership suspended
        .word   ccsm$ccb1_cdone         # ownership changed
        .word   ccsm$ccb1_term          # terminate ownership
        .word   ccsm$ccb1_tdone         # ownership terminated
        .word   ccsm$ccb1_readrm        # read dirty region map
        .word   ccsm$ccb1_rm            # dirty region map
        .word   ccsm$ccb1_readsm        # read segment map
        .word   ccsm$ccb1_smdata        # segment map data
        .word   ccsm$ccb1_reg           # copy registered
        .word   ccsm$ccb1_swap          # swap RAIDs
        .word   ccsm$ccb1_cdmoved       # copy devices moved
        .word   ccsm$ccb1_csc           # copy state changed
        .word   ccsm$ccb1_info          # copy general info. changed
ccsm_ccbg1_end:
#
        .set    ccsm_ccbg_maxfc,((ccsm_ccbg1_end-ccsm_ccbg1)/4)-1 # maximum valid CCBGram
                                                      #  event function code
        .text
#
#**********************************************************************
#
#  NAME: ccsm$ccb1_start
#
#  PURPOSE:
#
#       This routine processes a Start Copy CCBGram event.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    ccsm$ccb1_start
#
#  INPUT:
#
#       g1 = Start Copy CCBGram event ILT
#
#  OUTPUT:
#
#       g1 = ILT to return
#
#  REGS DESTROYED:
#
#       None.
#
#**********************************************************************
#
ccsm$ccb1_start:
#
#*** FINISH
#
        ret
#
#**********************************************************************
#
#  NAME: ccsm$ccb1_process
#
#  PURPOSE:
#
#       This routine processes a Process CCBGram event.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    ccsm$ccb1_process
#
#  INPUT:
#
#       g1 = Process CCBGram event ILT
#
#  OUTPUT:
#
#       g1 = ILT to return
#
#  REGS DESTROYED:
#
#       Reg. g0, g2-g4 destroyed.
#
#**********************************************************************
#
ccsm$ccb1_process:
        mov     g1,r15                  # save g1
                                        # r15 = event ILT
        ld      ccsm_gr_process_id(r15),g0 # g0 = reg. ID
        ld      ccsm_gr_process_cmsn(r15),g1 # g1 = CM serial #
        call    CM$find_cor_rid         # find assoc. COR address
        cmpobne 0,g0,.ccb1process_200   # Jif COR assoc. with this event
        ldob    ccsm_gr_process_code(r15),r4 # r4 = process type code
        cmpobe  2,r4,.ccb1process_1000  # ignore if terminate type code
        ld      ccsm_e_sendsn(r15),g4   # g4 = sender's serial #
        ld      ccsm_gr_process_id(r15),g1 # g1 = reg. ID
        ld      ccsm_gr_process_cmsn(r15),g2 # g2 = CM serial #
        ldconst 0x02,g0                 # g0 = terminate copy process code
        call    CCSM$snd_process2       # pack and send a Process
                                        #  message to this sender
                                        #  indicating to terminate the copy
        b       .ccb1process_1000
#
.ccb1process_200:
        mov     g0,g3                   # g3 = assoc. COR address
        ldob    ccsm_gr_process_code(r15),r4 # r4 = process type code
        cmpobne 2,r4,.ccb1process_400   # Jif not terminate type code
        ldconst Term_Copy,g0            # g0 = event type code
        call    CCSM$OCTrans            # generate event to O.C.S.E.
                                        # g0 = event type
                                        # g3 = COR address
        b       .ccb1process_1000
#
.ccb1process_400:
        ld      cor_cm(g3),r5           # r5 = assoc. CM address
        cmpobe  0,r5,.ccb1process_1000  # Jif no CM assoc. with COR
        cmpobne 0,r4,.ccb1process_420   # Jif not pause process type code
        call    CM$PauseCopy            # notify CM task of request
        b       .ccb1process_1000
#
.ccb1process_420:
        cmpobne 1,r4,.ccb1process_440   # Jif not resume process type code
        call    CM$ResumeCopy           # notify CM task of request
        b       .ccb1process_1000
#
.ccb1process_440:
.ccb1process_1000:
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: ccsm$ccb1_update
#
#  PURPOSE:
#
#       This routine processes an Update CCBGram event.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    ccsm$ccb1_update
#
#  INPUT:
#
#       g1 = Update CCBGram event ILT
#
#  OUTPUT:
#
#       g1 = ILT to return
#
#  REGS DESTROYED:
#
#       Reg. g0, g2-g4 destroyed.
#
#**********************************************************************
#
ccsm$ccb1_update:
        mov     g1,r15                  # save g1
                                        # r15 = event ILT
        ldob    ccsm_op_state,r3        # r3 = CCSM operational state
        cmpobne ccsm_st_master,r3,.ccb1update_1000 # Jif not the master CCSM
        ld      ccsm_gr_update_id(r15),g0 # g0 = reg. ID
        ld      ccsm_gr_update_cmsn(r15),g1 # g1 = CM serial #
        call    CM$find_cor_rid         # find assoc. COR address
        cmpobne 0,g0,.ccb1update_100    # Jif COR assoc. with this event
        ld      ccsm_e_sendsn(r15),g4   # g4 = sender's serial #
        ld      ccsm_gr_update_id(r15),g1 # g1 = reg. ID
        ld      ccsm_gr_update_cmsn(r15),g2 # g2 = CM serial #
        ldconst 0x02,g0                 # g0 = terminate copy process code
        call    CCSM$snd_process2       # pack and send a Process
                                        #  message to this sender
                                        #  indicating to terminate the copy
        b       .ccb1update_1000
#
.ccb1update_100:
        mov     g0,g3                   # g3 = assoc. COR address
        ld      cor_destvdd(g3),r12     # r12 = assoc. dest. VDD address
        ldob    ccsm_gr_update_type(r15),r4 # r4 = update type code
        cmpobe  0,r12,.ccb1update_1000  # Jif no dest. VDD defined
        cmpobne 0,r4,.ccb1update_200    # Jif not % complete update
#
# --- % complete update handler
#
        ld      ccsm_gr_update_g0(r15),r4 # r4 = % complete value
        stob    r4,vd_scpcomp(r12)      # save % complete in VDD
        ldconst vdcopyto,r4             # r4 = new VDD copy state
        stob    r4,vd_mirror(r12)       # save new VDD copy state
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        ldconst corcst_copy,r4
        stob    r4,cor_cstate(g3)
#
# --- Check to make sure that the cor_crstate is >= corcrst_active
#
.ccb1update_180:
        ldob    cor_crstate(g3),r4      # r4 = cor_crstate byte
        cmpobe  corcrst_autosusp,r4,.ccb1update_185 # Jif auto-suspended
        cmpoble corcrst_active,r4,.ccb1update_1000 # Jif >= corcrst_active
.ccb1update_185:
        ldconst corcrst_active,r4
        stob    r4,cor_crstate(g3)      # set to active state
        call    CM$mmc_sflag            # update VDD flags
        call    D$p2updateconfig        # save P2 NVRAM
        b       .ccb1update_1000
#
.ccb1update_200:
        cmpobne 1,r4,.ccb1update_300    # Jif not mirrored update
#
# --- Mirrored update handler
#
        ldconst vdcopymirror,r4         # r4 = new VDD copy state
        stob    r4,vd_mirror(r12)       # save new VDD copy state
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        ldconst corcst_mirror,r4
        stob    r4,cor_cstate(g3)
#
# --- Log Copy Mirrored Message to CCB
#
        ldconst cmcc_CpyMirror,g0       # set copy mirrored message
        call    CM_Log_Completion       # send log message
#
        ld      cor_cm(g3),r5           # r5 = assoc. CM address
        cmpobe  0,r5,.ccb1update_180    # Jif no CM assoc. with COR
        ldob    cm_type(r5),r6          # r6 = copy type code
        cmpobe  cmty_mirror,r6,.ccb1update_180 # Jif mirror type copy
        cmpobne cmty_copybreak,r6,.ccb1update_220 # Jif not copy & break type
#
# --- Copy & break type copy just got mirrored
#
        call    CCSM$term_copy          # start the terminate copy
        b       .ccb1update_180
#
.ccb1update_220:
        cmpobne cmty_copyswap,r6,.ccb1update_240 # Jif not copy/swap/break type
#
# --- Copy & swap & break type copy just got mirrored
#
        ldconst 1,g0                    # g0 = swap RAIDs type code
        b       .ccb1update_280
#
.ccb1update_240:
        cmpobne cmty_mirrorswap,r6,.ccb1update_180 # Jif not copy/swap/mirror
#
# --- Copy & swap & mirror type copy just got mirrored
#
        ldconst 0,g0                    # g0 = swap RAIDs type code
.ccb1update_280:
        call    CCSM$swap_raids         # initiate a swap RAIDs request
        b       .ccb1update_180

.ccb1update_300:
        cmpobne 2,r4,.ccb1update_400    # Jif not paused update
#
# --- Paused update handler
#
        ldconst corcrst_usersusp,r4
        stob    r4,cor_crstate(g3)
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>ccsm-ccb1update300-Setting userpause in vdmirror..\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # CM_IM_DEBUG
        ldconst vdcopyuserpause,r4      # r4 = new VDD copy state
        stob    r4,vd_mirror(r12)       # save new VDD copy state
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        call    CM$mmc_sflag            # update VDD flags
        ldconst cmcc_UsrSpnd,g0         # g0 = set User Suspend message
        call    CM_Log_Completion       # send log message
        b       .ccb1update_1000
#
.ccb1update_400:
        cmpobne 3,r4,.ccb1update_500    # Jif not resumed update
#
# --- Resumed update handler
#
        ldconst corcrst_active,r4
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>ccsm-ccb1update400-Setting crstate as active..setting copyto in vdmirror\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # CM_IM_DEBUG
        stob    r4,cor_crstate(g3)
        ldconst vdcopyto,r4             # r4 = new VDD copy state
        stob    r4,vd_mirror(r12)       # save new VDD copy state
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        call    CM$mmc_sflag            # update VDD flags
#
# --- Log Copy Resumed Message to CCB
#
        ldconst cmcc_CpyRsm,g0          # set copy resumed message
.if GR_GEORAID15_DEBUG
        ldos    vd_vid(r12),r4
c fprintf(stderr, "%s%s:%u <GR><ccsm$ccb1_update-ccsm.as>-Sending copy resumed message to CCB dvid-%lx\n", FEBEMESSAGE, __FILE__, __LINE__,r4);
.endif  # GR_GEORAID15_DEBUG
        call    CM_Log_Completion       # send log message
        b       .ccb1update_1000
#
.ccb1update_500:
        cmpobne 4,r4,.ccb1update_1000   # Jif not auto-paused update
#
# --- Auto-paused update handler routine
#
        ldconst corcrst_autosusp,r4
        stob    r4,cor_crstate(g3)
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>ccsm-ccb1update500-Setting autopause in vdmirror\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # CM_IM_DEBUG
        ldconst vdcopyautopause,r4      # r4 = new VDD copy state
        stob    r4,vd_mirror(r12)       # save new VDD copy state
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        call    CM$mmc_sflag            # update VDD flags
        ldconst cmcc_AutoSpnd,g0        # g0 = set Auto Suspend message
        call    CM_Log_Completion       # send log message
.ccb1update_1000:
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: ccsm$ccb1_force
#
#  PURPOSE:
#
#       This routine processes a Force Ownership Change CCBGram event.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    ccsm$ccb1_force
#
#  INPUT:
#
#       g1 = Force Ownership Change CCBGram event ILT
#
#  OUTPUT:
#
#       g1 = ILT to return
#
#  REGS DESTROYED:
#
#       Reg. g0, g2-g4 destroyed.
#
#**********************************************************************
#
ccsm$ccb1_force:
        mov     g1,r15                  # save g1
                                        # r15 = event ILT
        ldob    ccsm_op_state,r3        # r3 = CCSM operational state
        cmpobne ccsm_st_master,r3,.ccb1force_1000 # Jif not the master CCSM
        ld      ccsm_gr_force_id(r15),g0 # g0 = reg. ID
        ld      ccsm_gr_force_cmsn(r15),g1 # g1 = CM serial #
        call    CM$find_cor_rid         # find assoc. COR address
        cmpobne 0,g0,.ccb1force_200     # Jif COR assoc. with this event
        ld      ccsm_e_sendsn(r15),g4   # g4 = sender's serial #
        ld      ccsm_gr_force_id(r15),g1 # g1 = reg. ID
        ld      ccsm_gr_force_cmsn(r15),g2 # g2 = CM serial #
        ldconst 0x02,g0                 # g0 = terminate copy process code
        call    CCSM$snd_process2       # pack and send a Process
                                        #  message to this sender
                                        #  indicating to terminate the copy
        b       .ccb1force_1000
#
.ccb1force_200:
        mov     g0,g3                   # g3 = assoc. COR address
        ld      ccsm_gr_force_ppo(r15),r4 # r4 = specified prev. pri. owner
        ld      ccsm_gr_force_pso(r15),r5 # r5 = specified prev. sec. owner
        ld      cor_powner(g3),r6       # r6 = current pri. owner
        ld      cor_sowner(g3),r7       # r7 = current sec. owner
        cmpobne r4,r6,.ccb1force_300    # Jif pri. owner not correct
        cmpobe  r5,r7,.ccb1force_400    # Jif sec. owner matches
.ccb1force_300:
        ld      ccsm_e_sendsn(r15),g4   # g4 = controller serial #
        ldos    ccsm_e_seq(r15),g2      # g2 = seq. #
.if GR_GEORAID15_DEBUG
        ld      cor_destvdd(g3),r12     # r12 = assoc. dest. VDD address
        ldos    vd_vid(r12),r12         # r12 = destination vid
c fprintf(stderr,"%s%s:%u <GR><ccsm$ccb1_force>call CCSM$snd_trans ownerhip message dvid=%lx\n", FEBEMESSAGE, __FILE__, __LINE__,r12);
.endif  # GR_GEORAID15_DEBUG
        call    CCSM$snd_trans          # send a Transfer Ownership message
                                        #  in response
        b       .ccb1force_1000
#
.ccb1force_400:
        ld      ccsm_gr_force_npo(r15),g0 # g0 = new pri. owner
        ld      ccsm_gr_force_nso(r15),g1 # g1 = new sec. owner
        st      g0,cor_powner(g3)       # save new pri. owner
        st      g1,cor_sowner(g3)       # save new sec. owner
        call    CCSM$savenrefresh       # save P2 and refresh slaves
        ldos    ccsm_e_seq(r15),g2      # g2 = response message seq. #
.if GR_GEORAID15_DEBUG
        ldob cor_rcsvd(g3),r5
        ldob cor_rcdvd(g3),r4
c fprintf(stderr,"%s%s:%u <GR><ccsm$ccb1_force>call CCSM$snd_owner you are owner message-svid=%lx dvid=%lx currnet powner=%lx new powner=%lx\n", FEBEMESSAGE, __FILE__, __LINE__,r5,r4,r6,g0);
.endif  # GR_GEORAID15_DEBUG
        call    CCSM$snd_owner          # send a You Are Owner message in
                                        #  response
.ccb1force_1000:
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: ccsm$ccb1_define
#
#  PURPOSE:
#
#       This routine processes a Define Ownership CCBGram event.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    ccsm$ccb1_define
#
#  INPUT:
#
#       g1 = Define Ownership CCBGram event ILT
#
#  OUTPUT:
#
#       g1 = ILT to return
#
#  REGS DESTROYED:
#
#       Reg. g0, g2-g4 destroyed.
#
#**********************************************************************
#
.if     djk_deftr
#
        .data
djk_deftr_bnr:
        .ascii  "Tdef"
        .text
#
.endif  # djk_deftr
#
ccsm$ccb1_define:
        mov     g1,r15                  # save g1
                                        # r15 = event ILT
        ldob    ccsm_op_state,r3        # r3 = CCSM operational state
        cmpobne ccsm_st_master,r3,.ccb1define_1000 # Jif not the master CCSM
        ld      ccsm_gr_define_id(r15),g0 # g0 = reg. ID
        ld      ccsm_gr_define_cmsn(r15),g1 # g1 = CM serial #
        call    CM$find_cor_rid         # find assoc. COR address
        cmpobne 0,g0,.ccb1define_200    # Jif COR assoc. with this event
        ld      ccsm_e_sendsn(r15),g4   # g4 = sender's serial #
        ld      ccsm_gr_define_id(r15),g1 # g1 = reg. ID
        ld      ccsm_gr_define_cmsn(r15),g2 # g2 = CM serial #
        ldconst 0x02,g0                 # g0 = terminate copy process code
        call    CCSM$snd_process2       # pack and send a Process
                                        #  message to this sender
                                        #  indicating to terminate the copy
        b       .ccb1define_1000
#
.ccb1define_200:
        mov     g0,g3                   # g3 = assoc. COR address
#
.if     ccsm_traces
#
.if     djk_deftr
#
        ldq     ccsm_tr_flags,r4        # r4 = trace flags
                                        # r5 = current trace pointer
                                        # r6 = trace head pointer
                                        # r7 = trace tail pointer
        ld      djk_deftr_bnr,r8        # r8 = trace record type code
        ld      cor_rid(g3),r9
        ld      cor_rcsn(g3),r10
        ld      cor_powner(g3),r11
        stq     r8,(r5)
        lda     16(r5),r5
        cmpobg  r7,r5,.tex30x
        mov     r6,r5                   # set current pointer to head
.tex30x:
        st      r5,ccsm_tr_cur
#
.endif  # djk_deftr
#
.endif  # ccsm_traces
#
        ld      ccsm_gr_define_npo(r15),r4 # r4 = new pri. owner
        ld      ccsm_gr_define_nso(r15),r5 # r5 = new sec. owner
        ld      cor_powner(g3),r6       # r6 = current pri. owner
        ld      cor_sowner(g3),r7       # r7 = current sec. owner
        ldos    ccsm_e_seq(r15),g2      # g2 = seq. #
        or      r6,r7,r8
        cmpobne 0,r8,.ccb1define_400    # Jif owners defined
        st      r4,cor_powner(g3)       # save new pri. owner
        st      r5,cor_sowner(g3)       # save new sec. owner
        call    CCSM$savenrefresh       # save P2 and refresh slaves
.ccb1define_300:
        movl    r4,g0                   # g0 = primary owner
                                        # g1 = secondary owner
.if GR_GEORAID15_DEBUG
        ld      cor_destvdd(g3),r12     # r12 = assoc. dest. VDD address
        ldos    vd_vid(r12),r12         # r12 = destination vid
c fprintf(stderr,"%s%s:%u <GR><ccsm$ccb1_define>recevied define event-- calling CCSM$snd_owner dvid=%lx\n", FEBEMESSAGE, __FILE__, __LINE__,r12);
.endif  # GR_GEORAID15_DEBUG
        call    CCSM$snd_owner          # pack and send a You Are Owner message
        b       .ccb1define_1000        # and we're done
#
.ccb1define_400:
        cmpobne r6,r4,.ccb1define_500   # Jif not the current pri. owner
        cmpobe  r7,r5,.ccb1define_300   # Jif pri. and sec. owners match
.ccb1define_500:
.if GR_GEORAID15_DEBUG
        ldob cor_rcsvd(g3),r3
        ldob cor_rcdvd(g3),r4
c fprintf(stderr,"%s%s:%u <GR><CCSM$ccb1_define>call CCSM$snd_tran transer ownerhip..srcvid=%lx,dvid=%lx\n", FEBEMESSAGE, __FILE__, __LINE__,r3,r4);
.endif  # GR_GEORAID15_DEBUG
        ld      ccsm_e_sendsn(r15),g4   # g4 = sender's serial #
        call    CCSM$snd_trans          # pack and send a Transfer Ownership
                                        #  message to this sender
.ccb1define_1000:
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: ccsm$ccb1_owner
#
#  PURPOSE:
#
#       This routine processes a You Are Owner CCBGram event.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    ccsm$ccb1_owner
#
#  INPUT:
#
#       g1 = You Are Owner CCBGram event ILT
#
#  OUTPUT:
#
#       g1 = ILT to return
#
#  REGS DESTROYED:
#
#       Reg. g0, g2-g3 destroyed.
#
#**********************************************************************
#
ccsm$ccb1_owner:
        mov     g1,r15                  # save g1
                                        # r15 = event ILT
        ld      ccsm_gr_owner_id(r15),g0 # g0 = reg. ID
        ld      ccsm_gr_owner_cmsn(r15),g1 # g1 = CM serial #
        call    CM$find_cor_rid         # find assoc. COR address
        cmpobe  0,g0,.ccb1owner_1000    # Jif no COR assoc. with this event
        mov     g0,g3                   # g3 = assoc. COR address
        ldos    ccsm_e_seq(r15),r4      # r4 = message seq. #
        ldos    cor_seqnum(g3),r5       # r5 = expected message seq. #
        cmpobne r4,r5,.ccb1owner_1000   # Jif wrong seq. #
        ld      ccsm_gr_owner_po(r15),r4 # r4 = pri. owner
        ld      ccsm_gr_owner_so(r15),r5 # r5 = sec. owner
        st      r4,cor_PMPsn(g3)        # save new pri. owner
        st      r5,cor_SMPsn(g3)        # save new sec. owner
        ldob    ccsm_op_state,r3        # r3 = CCSM operational state
.if GR_GEORAID15_DEBUG
        ldob    cor_rcsvd(g3),r7
        ldob    cor_rcdvd(g3),r8
.endif  # GR_GEORAID15_DEBUG
        cmpobe  ccsm_st_master,r3,.ccb1owner_400 # Jif the master CCSM
        st      r4,cor_powner(g3)       # save new pri. owner
        st      r5,cor_sowner(g3)       # save new sec. owner
.if GR_GEORAID15_DEBUG
c fprintf(stderr,"%s%s:%u <GR><CCSM$ccb1_owner>Slave...svid=%lx,dvid=%lx powner=%lx sowner=%lx\n", FEBEMESSAGE, __FILE__, __LINE__,r7,r8,r4,r5);
.endif  # GR_GEORAID15_DEBUG
.ccb1owner_400:
        ldob    ccsm_gr_owner_crstate(r15),r6 # r6 = current cor_crstate value
        stob    r6,cor_crstate(g3)      # save in COR
        call    CM$mmc_sflag            # update VDD flags
        ldconst You_R_Owner,g0          # g0 = event type code
        mov     r15,g1                  # g1 = event ILT address
.if GR_GEORAID15_DEBUG
c fprintf(stderr,"%s%s:%u <GR><ccsm$ccb1_owner>Received Y R Owner event.Calling CCSm$OCTrans with event = YouRowner=You_R_Owner svid=%lx dvid=%lx\n", FEBEMESSAGE, __FILE__, __LINE__,r7,r8);
.endif  # GR_GEORAID15_DEBUG
        call    CCSM$OCTrans            # generate event to O.C.S.E.
                                        # g0 = event type
                                        # g1 = event ILT address
                                        # g3 = COR address
.ccb1owner_1000:
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: ccsm$ccb1_trans
#
#  PURPOSE:
#
#       This routine processes a Transfer Ownership CCBGram event.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    ccsm$ccb1_trans
#
#  INPUT:
#
#       g1 = Transfer Ownership CCBGram event ILT
#
#  OUTPUT:
#
#       g1 = ILT to return
#
#  REGS DESTROYED:
#
#       Reg. g0, g2-g3 destroyed.
#
#**********************************************************************
#
ccsm$ccb1_trans:
        mov     g1,r15                  # save g1
                                        # r15 = event ILT
        ld      ccsm_gr_trans_id(r15),g0 # g0 = reg. ID
        ld      ccsm_gr_trans_cmsn(r15),g1 # g1 = CM serial #
        call    CM$find_cor_rid         # find assoc. COR address
        cmpobe  0,g0,.ccb1trans_1000    # Jif no COR assoc. with this event
        mov     g0,g3                   # g3 = assoc. COR address
.if GR_GEORAID15_DEBUG
        ldob cor_rcsvd(g3),r5
        ldob cor_rcdvd(g3),r4
c fprintf(stderr,"%s%s:%u <GR><ccb1_trans>Received transfer ownership event-.svid=%lx,dvid=%lx call CCSM$OCTrans with event=TransOwner= Trans_Owner\n", FEBEMESSAGE, __FILE__, __LINE__,r5,r4);
.endif  # GR_GEORAID15_DEBUG
        ldos    ccsm_e_seq(r15),r4      # r4 = message seq. #
        ldos    cor_seqnum(g3),r5       # r5 = expected message seq. #
        cmpobne r4,r5,.ccb1trans_1000   # Jif wrong seq. #
        ldconst Trans_Owner,g0          # g0 = event type code
        mov     r15,g1                  # g1 = event ILT address
        call    CCSM$OCTrans            # generate event to O.C.S.E.
                                        # g0 = event type
                                        # g1 = event ILT address
                                        # g3 = COR address
.ccb1trans_1000:
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: ccsm$ccb1_change
#
#  PURPOSE:
#
#       This routine processes a Change Ownership CCBGram event.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    ccsm$ccb1_change
#
#  INPUT:
#
#       g1 = Change Ownership CCBGram event ILT
#
#  OUTPUT:
#
#       g1 = ILT to return
#
#  REGS DESTROYED:
#
#       Reg. g0, g2-g4 destroyed.
#
#**********************************************************************
#
ccsm$ccb1_change:
        mov     g1,r15                  # save g1
                                        # r15 = event ILT
        ldob    ccsm_op_state,r3        # r3 = CCSM operational state
        cmpobne ccsm_st_master,r3,.ccb1change_1000 # Jif not the master CCSM
        ld      ccsm_gr_change_id(r15),g0 # g0 = reg. ID
        ld      ccsm_gr_change_cmsn(r15),g1 # g1 = CM serial #
        ld      ccsm_e_sendsn(r15),g4   # g4 = sender's serial #
        call    CM$find_cor_rid         # find assoc. COR address
        cmpobne 0,g0,.ccb1change_200    # Jif COR assoc. with this event
        ld      ccsm_gr_change_id(r15),g1 # g1 = reg. ID
        ld      ccsm_gr_change_cmsn(r15),g2 # g2 = CM serial #
        ldconst 0x02,g0                 # g0 = terminate copy process code
        call    CCSM$snd_process2       # pack and send a Process
                                        #  message to this sender
                                        #  indicating to terminate the copy
        b       .ccb1change_1000
#
.ccb1change_200:
        mov     g0,g3                   # g3 = assoc. COR address
.if GR_GEORAID15_DEBUG
        ldob    cor_rcsvd(g3),r5
        ldob    cor_rcdvd(g3),r4
c fprintf(stderr,"%s%s:%u <GR><ccb1_change-ccsm.as>checking to change ownershi.svid=%lx,dvid=%lx\n", FEBEMESSAGE, __FILE__, __LINE__,r5,r4);
.endif  # GR_GEORAID15_DEBUG
        ld      ccsm_gr_change_cpo(r15),r4 # r4 = current pri. owner
        ld      ccsm_gr_change_cso(r15),r5 # r5 = current sec. owner
        ld      cor_powner(g3),r6       # r6 = current pri. owner
        ld      cor_sowner(g3),r7       # r7 = current sec. owner
        ld      ccsm_gr_change_npo(r15),r8 # r8 = new pri. owner
        ld      ccsm_gr_change_nso(r15),r9 # r9 = new sec. owner
        ldos    ccsm_e_seq(r15),g2      # g2 = seq. #
        or      r6,r7,r13
        cmpobne 0,r13,.ccb1change_400   # Jif owners defined
.ccb1change_300:
.if GR_GEORAID15_DEBUG
        ldob    cor_rcsvd(g3),r5
        ldob    cor_rcdvd(g3),r4
c fprintf(stderr,"%s%s:%u <GR><ccb1_change-ccssm.as>ownership changed..svid=%lx,dvid=%lx powner=%lx sowner=%lx\n", FEBEMESSAGE, __FILE__, __LINE__,r5,r4,r8,r9);
.endif  # GR_GEORAID15_DEBUG
        st      r8,cor_powner(g3)       # save new pri. owner
        st      r9,cor_sowner(g3)       # save new sec. owner
        call    CCSM$savenrefresh       # save P2 and refresh slaves
.ccb1change_350:
        movl    r8,g0                   # g0 = new primary owner
                                        # g1 = new secondary owner
        call    CCSM$snd_cdone          # pack and send an Ownership Changed
                                        #  message to the sender
        b       .ccb1change_1000        # and we're done
#
.ccb1change_400:
        cmpobne r6,r4,.ccb1change_500   # Jif not the current pri. owner
        b       .ccb1change_300         # TEMPORARY-DO NOT check sec. owner
                                        #  for match
#        cmpobe  r7,r5,.ccb1change_300   # Jif pri. and sec. owners match
.ccb1change_500:
        cmpobne r6,r8,.ccb1change_600   # Jif not the current pri. owner
        b       .ccb1change_350         # TEMPORARY-DO NOT check sec. owner
                                        #  for match
#        cmpobe  r7,r9,.ccb1change_350   # Jif pri. and sec. owners match
.ccb1change_600:
#
#*** FINISH - May need to send an abnormal message in response indicating the
#               requested operation was not performed. Until then, we will
#               rely on the original transfer ownership node to timeout
#               and restart the transfer ownership process by requesting
#               what controller is the current copy owner.
#
.ccb1change_1000:
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: ccsm$ccb1_susp
#
#  PURPOSE:
#
#       This routine processes a Suspend Ownership CCBGram event.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    ccsm$ccb1_susp
#
#  INPUT:
#
#       g1 = Suspend Ownership CCBGram event ILT
#
#  OUTPUT:
#
#       g1 = ILT to return
#
#  REGS DESTROYED:
#
#       Reg. g0, g2-g3 destroyed.
#
#**********************************************************************
#
ccsm$ccb1_susp:
        mov     g1,r15                  # save g1
                                        # r15 = event ILT
        ld      ccsm_gr_susp_id(r15),g0 # g0 = reg. ID
        ld      ccsm_gr_susp_cmsn(r15),g1 # g1 = CM serial #
        call    CM$find_cor_rid         # find assoc. COR address
        cmpobe  0,g0,.ccb1susp_1000     # Jif no COR assoc. with this event
        mov     g0,g3                   # g3 = assoc. COR address
        ldconst Suspend_Owner,g0        # g0 = event type code
        mov     r15,g1                  # g1 = event ILT address
.if GR_GEORAID15_DEBUG
        ldob cor_rcsvd(g3),r5
        ldob cor_rcdvd(g3),r4
c fprintf(stderr, "%s%s:%u <GR><ccsm$ccb1_susp> Received the event- Calling CCSM$OCTrans with Suspend_Owner1 = Suspend_Owner svid=%lx dvid=%lx\n", FEBEMESSAGE, __FILE__, __LINE__,r5,r4);
.endif  # GR_GEORAID15_DEBUG
        call    CCSM$OCTrans            # generate event to O.C.S.E.
                                        # g0 = event type
                                        # g1 = event ILT address
                                        # g3 = COR address
.ccb1susp_1000:
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: ccsm$ccb1_sdone
#
#  PURPOSE:
#
#       This routine processes a Ownership Suspended CCBGram event.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    ccsm$ccb1_sdone
#
#  INPUT:
#
#       g1 = Ownership Suspended CCBGram event ILT
#
#  OUTPUT:
#
#       g1 = ILT to return
#
#  REGS DESTROYED:
#
#       Reg. g0, g2-g3 destroyed.
#
#**********************************************************************
#
ccsm$ccb1_sdone:
        mov     g1,r15                  # save g1
                                        # r15 = event ILT
        ld      ccsm_gr_sdone_id(r15),g0 # g0 = reg. ID
        ld      ccsm_gr_sdone_cmsn(r15),g1 # g1 = CM serial #
        call    CM$find_cor_rid         # find assoc. COR address
        cmpobe  0,g0,.ccb1sdone_1000    # Jif no COR assoc. with this event
        mov     g0,g3                   # g3 = assoc. COR address
        ldos    ccsm_e_seq(r15),r4      # r4 = message seq. #
        ldos    cor_seqnum(g3),r5       # r5 = expected message seq. #
        cmpobne r4,r5,.ccb1sdone_1000   # Jif wrong seq. #
        ld      ccsm_e_sendsn(r15),r4   # r4 = sender's serial #
        ld      cor_respsn(g3),r5       # r5 = expected sender's serial #
        cmpobne r4,r5,.ccb1sdone_1000   # Jif wrong sender's serial #
        ldconst Owner_Suspended,g0      # g0 = event type code
        mov     r15,g1                  # g1 = event ILT address
.if GR_GEORAID15_DEBUG
        ldob cor_rcsvd(g3),r5
        ldob cor_rcdvd(g3),r4
c fprintf(stderr, "%s%s:%u <GR>ccsm$ccb1_sdone>Calling CCSM$OCTrans with event Owner_Suspend1 = Owner_Suspended  svid=%lx dvid=%lx\n", FEBEMESSAGE, __FILE__, __LINE__,r5,r4);
.endif  # GR_GEORAID15_DEBUG
        call    CCSM$OCTrans            # generate event to O.C.S.E.
                                        # g0 = event type
                                        # g1 = event ILT address
                                        # g3 = COR address
.ccb1sdone_1000:
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: ccsm$ccb1_cdone
#
#  PURPOSE:
#
#       This routine processes a Ownership Changed CCBGram event.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    ccsm$ccb1_cdone
#
#  INPUT:
#
#       g1 = Ownership Changed CCBGram event ILT
#
#  OUTPUT:
#
#       g1 = ILT to return
#
#  REGS DESTROYED:
#
#       Reg. g0, g2-g3 destroyed.
#
#**********************************************************************
#
ccsm$ccb1_cdone:
        mov     g1,r15                  # save g1
                                        # r15 = event ILT
        ld      ccsm_gr_cdone_id(r15),g0 # g0 = reg. ID
        ld      ccsm_gr_cdone_cmsn(r15),g1 # g1 = CM serial #
        call    CM$find_cor_rid         # find assoc. COR address
        cmpobe  0,g0,.ccb1cdone_1000    # Jif no COR assoc. with this event
        mov     g0,g3                   # g3 = assoc. COR address
        ldos    ccsm_e_seq(r15),r4      # r4 = message seq. #
        ldos    cor_seqnum(g3),r5       # r5 = expected message seq. #
        cmpobne r4,r5,.ccb1cdone_1000   # Jif wrong seq. #
        ldconst Owner_Chg,g0            # g0 = event type code
        mov     r15,g1                  # g1 = event ILT address
        call    CCSM$OCTrans            # generate event to O.C.S.E.
                                        # g0 = event type
                                        # g1 = event ILT address
                                        # g3 = COR address
.ccb1cdone_1000:
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: ccsm$ccb1_term
#
#  PURPOSE:
#
#       This routine processes a Terminate Ownership CCBGram event.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    ccsm$ccb1_term
#
#  INPUT:
#
#       g1 = Terminate Ownership CCBGram event ILT
#
#  OUTPUT:
#
#       g1 = ILT to return
#
#  REGS DESTROYED:
#
#       Reg. g0, g2-g3 destroyed.
#
#**********************************************************************
#
ccsm$ccb1_term:
        mov     g1,r15                  # save g1
                                        # r15 = event ILT
        ld      ccsm_gr_term_id(r15),g0 # g0 = reg. ID
        ld      ccsm_gr_term_cmsn(r15),g1 # g1 = CM serial #
        call    CM$find_cor_rid         # find assoc. COR address
        cmpobe  0,g0,.ccb1term_1000     # Jif no COR assoc. with this event
        mov     g0,g3                   # g3 = assoc. COR address
        ldconst Term_Owner,g0           # g0 = event type code
        mov     r15,g1                  # g1 = event ILT address
        call    CCSM$OCTrans            # generate event to O.C.S.E.
                                        # g0 = event type
                                        # g1 = event ILT address
                                        # g3 = COR address
.ccb1term_1000:
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: ccsm$ccb1_tdone
#
#  PURPOSE:
#
#       This routine processes a Ownership Terminated CCBGram event.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    ccsm$ccb1_tdone
#
#  INPUT:
#
#       g1 = Ownership Terminated CCBGram event ILT
#
#  OUTPUT:
#
#       g1 = ILT to return
#
#  REGS DESTROYED:
#
#       Reg. g0, g2-g3 destroyed.
#
#**********************************************************************
#
ccsm$ccb1_tdone:
        mov     g1,r15                  # save g1
                                        # r15 = event ILT
        ld      ccsm_gr_tdone_id(r15),g0 # g0 = reg. ID
        ld      ccsm_gr_tdone_cmsn(r15),g1 # g1 = CM serial #
        call    CM$find_cor_rid         # find assoc. COR address
        cmpobe  0,g0,.ccb1tdone_1000    # Jif no COR assoc. with this event
        mov     g0,g3                   # g3 = assoc. COR address
        ldos    ccsm_e_seq(r15),r4      # r4 = message seq. #
        ldos    cor_seqnum(g3),r5       # r5 = expected message seq. #
        cmpobne r4,r5,.ccb1tdone_1000   # Jif wrong seq. #
        ld      ccsm_e_sendsn(r15),r4   # r4 = sender's serial #
        ld      cor_respsn(g3),r5       # r5 = expected sender's serial #
        cmpobne r4,r5,.ccb1tdone_1000   # Jif wrong sender's serial #
        ldconst Owner_Term,g0           # g0 = event type code
        mov     r15,g1                  # g1 = event ILT address
        call    CCSM$OCTrans            # generate event to O.C.S.E.
                                        # g0 = event type
                                        # g1 = event ILT address
                                        # g3 = COR address
.ccb1tdone_1000:
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: ccsm$ccb1_readrm
#
#  PURPOSE:
#
#       This routine processes a Read Dirty Region Map CCBGram event.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    ccsm$ccb1_readrm
#
#  INPUT:
#
#       g1 = Read Dirty Region Map CCBGram event ILT
#
#  OUTPUT:
#
#       g1 = ILT to return
#
#  REGS DESTROYED:
#
#       Reg. g0, g2-g3 destroyed.
#
#**********************************************************************
#
ccsm$ccb1_readrm:
        mov     g1,r15                  # save g1
                                        # r15 = event ILT
        ld      ccsm_gr_readrm_id(r15),g0 # g0 = reg. ID
        ld      ccsm_gr_readrm_cmsn(r15),g1 # g1 = CM serial #
        call    CM$find_cor_rid         # find assoc. COR address
        cmpobe  0,g0,.ccb1readrm_1000   # Jif no COR assoc. with this event
        mov     g0,g3                   # g3 = assoc. COR address
        ldconst Read_Dirty_Region_Map,g0 # g0 = event type code
        mov     r15,g1                  # g1 = event ILT address
        call    CCSM$OCTrans            # generate event to O.C.S.E.
                                        # g0 = event type
                                        # g1 = event ILT address
                                        # g3 = COR address
.ccb1readrm_1000:
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: ccsm$ccb1_rm
#
#  PURPOSE:
#
#       This routine processes a Dirty Region Map CCBGram event.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    ccsm$ccb1_rm
#
#  INPUT:
#
#       g1 = Dirty Region Map CCBGram event ILT
#
#  OUTPUT:
#
#       g1 = ILT to return
#
#  REGS DESTROYED:
#
#       Reg. g0, g2-g3 destroyed.
#
#**********************************************************************
#
ccsm$ccb1_rm:
        mov     g1,r15                  # save g1
                                        # r15 = event ILT
        ld      ccsm_gr_rm_id(r15),g0   # g0 = reg. ID
        ld      ccsm_gr_rm_cmsn(r15),g1 # g1 = CM serial #
        call    CM$find_cor_rid         # find assoc. COR address
        cmpobe  0,g0,.ccb1rm_1000       # Jif no COR assoc. with this event
        mov     g0,g3                   # g3 = assoc. COR address
        ldos    ccsm_e_seq(r15),r4      # r4 = message seq. #
        ldos    cor_seqnum(g3),r5       # r5 = expected message seq. #
        cmpobne r4,r5,.ccb1rm_1000      # Jif wrong seq. #
        ld      ccsm_e_sendsn(r15),r4   # r4 = sender's serial #
        ld      cor_respsn(g3),r5       # r5 = expected sender's serial #
        cmpobne r4,r5,.ccb1rm_1000      # Jif wrong sender's serial #
        ldconst Dirty_Region_Map,g0     # g0 = event type code
        mov     r15,g1                  # g1 = event ILT address
        call    CCSM$OCTrans            # generate event to O.C.S.E.
                                        # g0 = event type
                                        # g1 = event ILT address
                                        # g3 = COR address
.ccb1rm_1000:
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: ccsm$ccb1_readsm
#
#  PURPOSE:
#
#       This routine processes a Read Segment Map CCBGram event.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    ccsm$ccb1_readsm
#
#  INPUT:
#
#       g1 = Read Segment Map CCBGram event ILT
#
#  OUTPUT:
#
#       g1 = ILT to return
#
#  REGS DESTROYED:
#
#       Reg. g0, g2-g3 destroyed.
#
#**********************************************************************
#
ccsm$ccb1_readsm:
        mov     g1,r15                  # save g1
                                        # r15 = event ILT
        ld      ccsm_gr_readsm_id(r15),g0 # g0 = reg. ID
        ld      ccsm_gr_readsm_cmsn(r15),g1 # g1 = CM serial #
        call    CM$find_cor_rid         # find assoc. COR address
        cmpobe  0,g0,.ccb1readsm_1000   # Jif no COR assoc. with this event
        mov     g0,g3                   # g3 = assoc. COR address
        ldconst Read_Dirty_Segment_Map,g0 # g0 = event type code
        mov     r15,g1                  # g1 = event ILT address
        call    CCSM$OCTrans            # generate event to O.C.S.E.
                                        # g0 = event type
                                        # g1 = event ILT address
                                        # g3 = COR address
.ccb1readsm_1000:
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: ccsm$ccb1_smdata
#
#  PURPOSE:
#
#       This routine processes a Segment Map Data CCBGram event.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    ccsm$ccb1_smdata
#
#  INPUT:
#
#       g1 = Segment Map Data CCBGram event ILT
#
#  OUTPUT:
#
#       g1 = ILT to return
#
#  REGS DESTROYED:
#
#       Reg. g0, g2-g3 destroyed.
#
#**********************************************************************
#
ccsm$ccb1_smdata:
        mov     g1,r15                  # save g1
                                        # r15 = event ILT
        ld      ccsm_gr_sm_id(r15),g0   # g0 = reg. ID
        ld      ccsm_gr_sm_cmsn(r15),g1 # g1 = CM serial #
        call    CM$find_cor_rid         # find assoc. COR address
        cmpobne 0,g0,.ccb1sm_200        # Jif no COR assoc. with this event
.ccb1sm_100:
        ld      ccsm_gr_sm_map(r15),g1  # g1 = segment table allocated
.ifdef M4_DEBUG_SM
c fprintf(stderr, "%s%s:%u put_sm 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_SM
c       put_sm(g1);                     # Deallocate segment map table
        b       .ccb1sm_1000
#
.ccb1sm_200:
        mov     g0,g3                   # g3 = assoc. COR address
        ldos    ccsm_e_seq(r15),r4      # r4 = message seq. #
        ldos    cor_seqnum(g3),r5       # r5 = expected message seq. #
        cmpobne r4,r5,.ccb1sm_100       # Jif wrong seq. #
        ld      ccsm_e_sendsn(r15),r4   # r4 = sender's serial #
        ld      cor_respsn(g3),r5       # r5 = expected sender's serial #
        cmpobne r4,r5,.ccb1sm_100       # Jif wrong sender's serial #
        ldconst Dirty_Segment_Map,g0    # g0 = event type code
        mov     r15,g1                  # g1 = event ILT address
        call    CCSM$OCTrans            # generate event to O.C.S.E.
                                        # g0 = event type
                                        # g1 = event ILT address
                                        # g3 = COR address
.ccb1sm_1000:
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: ccsm$ccb1_reg
#
#  PURPOSE:
#
#       This routine processes a Copy Registered CCBGram event.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    ccsm$ccb1_reg
#
#  INPUT:
#
#       g1 = Copy Registered CCBGram event ILT
#
#  OUTPUT:
#
#       g1 = ILT to return
#
#  REGS DESTROYED:
#
#       Reg. g0, g2-g3 destroyed.
#
#**********************************************************************
#
ccsm$ccb1_reg:
        mov     g1,r15                  # save g1
                                        # r15 = event ILT
        ldob    ccsm_op_state,r3        # r3 = CCSM operational state
        cmpobne ccsm_st_master,r3,.ccb1reg_1000 # Jif not the master CCSM
        ld      ccsm_gr_reg_id(r15),g0  # g0 = reg. ID
        ld      ccsm_gr_reg_cmsn(r15),g1 # g1 = CM serial #
        call    CM$find_cor_rid         # find assoc. COR address
        cmpobne 0,g0,.ccb1reg_200       # Jif COR assoc. with this event
        ld      ccsm_e_sendsn(r15),g4   # g4 = sender's serial #
        ld      ccsm_gr_reg_id(r15),g1  # g1 = reg. ID
        ld      ccsm_gr_reg_cmsn(r15),g2 # g2 = CM serial #
        ldconst 0x02,g0                 # g0 = terminate copy process code
        call    CCSM$snd_process2       # pack and send a Process
                                        #  message to this sender
                                        #  indicating to terminate the copy
        b       .ccb1reg_1000
#
.ccb1reg_200:
        mov     g0,g3                   # g3 = assoc. COR address
        ldob    cor_crstate(g3),r5      # r5 = current registration state
        ldconst corcrst_active,r4       # r4 = registration active
        cmpoble r4,r5,.ccb1reg_1000     # Jif copy registration already done
        stob    r4,cor_crstate(g3)      # save new registration state
        call    CM$mmc_sflag            # update VDD flags
        call    D$p2updateconfig        # save P2 NVRAM
.ccb1reg_1000:
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: ccsm$ccb1_swap
#
#  PURPOSE:
#
#       This routine processes a Swap RAIDs CCBGram event.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    ccsm$ccb1_swap
#
#  INPUT:
#
#       g1 = Swap RAIDs CCBGram event ILT
#
#  OUTPUT:
#
#       g1 = ILT to return
#
#  REGS DESTROYED:
#
#       Reg. g0, g2-g3 destroyed.
#
#**********************************************************************
#
ccsm$ccb1_swap:
        mov     g1,r15                  # save g1
                                        # r15 = event ILT
        ld      ccsm_gr_swap_id(r15),g0 # g0 = reg. ID
        ld      ccsm_gr_swap_cmsn(r15),g1 # g1 = CM serial #
        call    CM$find_cor_rid         # find assoc. COR address
        cmpobe  0,g0,.ccb1swap_1000     # Jif no COR assoc. with this event
        mov     g0,g3                   # g3 = assoc. COR address
        ld      ccsm_gr_swap_rid(r15),r4 # r4 = new source RAID id
        ld      cor_srcvdd(g3),r5       # r5 = source VDD address
        cmpobe  0,r5,.ccb1swap_1000     # Jif no source VDD assoc. with COR
        ld      cor_destvdd(g3),r8      # r8 = destination VDD address
        cmpobe  0,r8,.ccb1swap_1000     # Jif no dest. VDD defined
        ldos    ccsm_e_seq(r15),g2      # g2 = sequence #
        ld      vd_rdd(r5),r6           # r6 = first RDD assoc. with source
                                        #      VDD
        ldos    rd_rid(r6),r7           # r7 = RAID id of first RAID assoc.
                                        #      with source copy device
        cmpobne r4,r7,.ccb1swap_400     # Jif swap not already performed
#
# --- Swap RAIDs has already been performed
#
        call    CCSM$snd_swapdone       # pack and send a Swap RAIDs Completed
                                        #  message to the master CCSM
        b       .ccb1swap_1000
#
.ccb1swap_400:
        ld      vd_rdd(r8),r6           # r6 = first RDD assoc. with dest.
                                        #      VDD
        ldos    rd_rid(r6),r7           # r7 = RAID id of first RAID assoc.
                                        #      with dest. copy device
        cmpobne r4,r7,.ccb1swap_1000    # Jif specified RAID id not assoc.
                                        #  with destination VDD
        ldos    cor_swapseq(g3),r7      # r7 = current swap RAIDs seq. #
        cmpobe  g2,r7,.ccb1swap_1000    # Jif already processing this swap
                                        #  RAIDs operation
        stos    g2,cor_swapseq(g3)      # save swap RAIDs seq. # in COR
        call    CM$SwapRaids            # tell CCSE to swap RAIDs
.ccb1swap_1000:
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: ccsm$ccb1_cdmoved
#
#  PURPOSE:
#
#       This routine processes a Copy Devices Moved CCBGram event.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    ccsm$ccb1_cdmoved
#
#  INPUT:
#
#       g1 = Copy Devices Moved CCBGram event ILT
#
#  OUTPUT:
#
#       g1 = ILT to return
#
#  REGS DESTROYED:
#
#       Reg. g0, g2-g4 destroyed.
#
#**********************************************************************
#
ccsm$ccb1_cdmoved:
        mov     g1,r15                  # save g1
                                        # r15 = event ILT
        ldob    ccsm_op_state,r3        # r3 = CCSM operational state
        cmpobne ccsm_st_master,r3,.ccb1cdm_1000 # Jif not the master CCSM
        ld      ccsm_gr_cdm_id(r15),g0  # g0 = reg. ID
        ld      ccsm_gr_cdm_cmsn(r15),g1 # g1 = CM serial #
        ld      ccsm_e_sendsn(r15),g4   # g4 = sender's serial #
        call    CM$find_cor_rid         # find assoc. COR address
        cmpobne 0,g0,.ccb1cdm_200       # Jif COR assoc. with this event
        ld      ccsm_gr_cdm_id(r15),g1  # g1 = reg. ID
        ld      ccsm_gr_cdm_cmsn(r15),g2 # g2 = CM serial #
        ldconst 0x02,g0                 # g0 = terminate copy process code
        call    CCSM$snd_process2       # pack and send a Process
                                        #  message to this sender
                                        #  indicating to terminate the copy
        b       .ccb1cdm_1000
#
.ccb1cdm_200:
        mov     g0,g3                   # g3 = assoc. COR address
        ld      cor_powner(g3),r4       # r4 = current primary owner
        cmpobne r4,g4,.ccb1cdm_1000     # Jif not from primary copy owner
        ldq     ccsm_gr_cdm_rcscl(r15),r4 # r4-r7 = requestor's device defs.
        ldq     cor_rcscl(g3),r8        # r8-r11 = local COR device defs.
        cmpobne r4,r8,.ccb1cdm_400      # Jif values don't match
        cmpobne r5,r9,.ccb1cdm_400      # Jif values don't match
        cmpobne r6,r10,.ccb1cdm_400     # Jif values don't match
        cmpobne r7,r11,.ccb1cdm_400     # Jif values don't match
#
# --- Values match with local values. Send Copy Devices Moved
#       Acknowledgement message to sender.
#
        ldos    ccsm_e_seq(r15),g2      # g2 = request seq. #
        call    CCSM$snd_cdmack         # pack and send Copy Devices Moved
                                        #  Acknowledgement message to sender
        b       .ccb1cdm_1000
#
# --- Copy devices do not match the local copy device values. Process
#       the differences.
#
.ccb1cdm_400:
        mov     r15,g1                  # g1 = Copy Devices Moved ILT
        call    CCSM$cdm_delta          # process the copy devices diffs.
        cmpobne 0,g0,.ccb1cdm_1000      # Jif not all differences resolved
        call    CCSM$savenrefresh       # save P2 and refresh slaves
.ccb1cdm_1000:
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: ccsm$ccb1_csc
#
#  PURPOSE:
#
#       This routine processes a Copy State Changed CCBGram event.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    ccsm$ccb1_csc
#
#  INPUT:
#
#       g1 = Copy State Changed CCBGram event ILT
#
#  OUTPUT:
#
#       g1 = ILT to return
#
#  REGS DESTROYED:
#
#       Reg. g0, g2-g4 destroyed.
#
#**********************************************************************
#
ccsm$ccb1_csc:
        mov     g1,r15                  # save g1
                                        # r15 = event ILT
        ldob    ccsm_op_state,r3        # r3 = CCSM operational state
        cmpobne ccsm_st_master,r3,.ccb1csc_1000 # Jif not the master CCSM
        ld      ccsm_gr_csc_id(r15),g0  # g0 = reg. ID
        ld      ccsm_gr_csc_cmsn(r15),g1 # g1 = CM serial #
        ld      ccsm_e_sendsn(r15),g4   # g4 = sender's serial #
        call    CM$find_cor_rid         # find assoc. COR address
        cmpobne 0,g0,.ccb1csc_200       # Jif COR assoc. with this event
        ld      ccsm_gr_csc_id(r15),g1  # g1 = reg. ID
        ld      ccsm_gr_csc_cmsn(r15),g2 # g2 = CM serial #
        ldconst 0x02,g0                 # g0 = terminate copy process code
        call    CCSM$snd_process2       # pack and send a Process
                                        #  message to this sender
                                        #  indicating to terminate the copy
        b       .ccb1csc_1000
#
.ccb1csc_200:
        mov     g0,g3                   # g3 = assoc. COR address
        ld      cor_powner(g3),r4       # r4 = current primary owner
        cmpobne r4,g4,.ccb1csc_1000     # Jif not from primary copy owner
        ldob    ccsm_gr_csc_cstate(r15),r4
        ldob    ccsm_gr_csc_crstate(r15),r5
        ldob    ccsm_gr_csc_mstate(r15),r6
        ldob    cor_cstate(g3),r7
        cmpobne r4,r7,.ccb1csc_400      # Jif cor_cstate differs
        ldob    cor_crstate(g3),r7
        cmpobne r5,r7,.ccb1csc_400      # Jif cor_crstate differs
        ldob    cor_mstate(g3),r7
        cmpobne r6,r7,.ccb1csc_400      # Jif cor_mstate differs
#
# --- Values match with local values. Send Copy State Changed
#       Acknowledgement message to sender.
#
        ldob    CCSM_mupd_timer,r4      # r4 = scheduled update timer
        cmpobne 0,r4,.ccb1csc_1000      # Jif an update is pending
        ldos    ccsm_e_seq(r15),g2      # g2 = request seq. #
        call    CCSM$snd_cscack         # pack and send Copy State Changed
                                        #  Acknowledgement message to sender
        b       .ccb1csc_1000
#
# --- Copy states do not match the local copy state values. Process
#       the differences.
#
.ccb1csc_400:
        stob    r4,cor_cstate(g3)
        stob    r5,cor_crstate(g3)
        stob    r6,cor_mstate(g3)
        call    CM$mmc_sflag            # update VDD flags
#
# --- Log Copy Resumed Message to CCB
#
        ldconst cmcc_CpyRsm,g0          # set copy resumed message
.if GR_GEORAID15_DEBUG
        ld      cor_destvdd(g3),r6
        ldos    vd_vid(r6),r6           # r6 = dest. VID
        ld      cor_srcvdd(g3),r7
        ldos    vd_vid(r7),r7           # r7 = source VID
c fprintf(stderr, "%s%s:%u <GR><ccb1_csc-ccsm.as>Sending copy resumed message to CCB svid=%lx dvid=%lx\n", FEBEMESSAGE, __FILE__, __LINE__,r7,r6);
.endif  # GR_GEORAID15_DEBUG
        cmpobg  corcrst_usersusp,r5,.ccb1csc_430 # Jif copy not paused
#
# --- Log Copy Paused Message to CCB
#
        ldconst cmcc_UsrSpnd,g0         # g0 = set User Suspend message
#
# --- issue Log Message to CCB
#
.ccb1csc_430:
        call    CM_Log_Completion       # send log message

#
        lda     CM$wp2_suspend,r6       # r6 = suspended write update phase
                                        #      2 handler routine
        cmpobe  corcrst_usersusp,r5,.ccb1csc_500 # Jif crstate = user suspend
        cmpobe  corcrst_remsusp,r5,.ccb1csc_500  # Jif crstate = remote susp.
        lda     CM$wp2_null,r6          # r6 = null write update phase 2
                                        #      handler routine
.ccb1csc_500:
        ld      cor_scd(g3),r4          # r4 = assoc. SCD address
        cmpobe.f 0,r4,.ccb1csc_520      # Jif no SCD assoc. with COR
        st      r6,scd_p2hand(r4)       # save phase 2 update handler routine
                                        #  in SCD
.ccb1csc_520:
        ld      cor_dcd(g3),r4          # r4 = assoc. DCD address
        cmpobe.f 0,r4,.ccb1csc_600      # Jif no DCD assoc. with COR
        st      r6,dcd_p2hand(r4)       # save phase 2 update handler routine
                                        #  in DCD
.ccb1csc_600:
        call    CCSM$p2update           # schedule timed update
.ccb1csc_1000:
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: ccsm$ccb1_info
#
#  PURPOSE:
#
#       This routine processes a Copy General Info. Changed CCBGram event.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    ccsm$ccb1_info
#
#  INPUT:
#
#       g1 = Copy General Info. Changed CCBGram event ILT
#
#  OUTPUT:
#
#       g1 = ILT to return
#
#  REGS DESTROYED:
#
#       Reg. g0, g2-g4 destroyed.
#
#**********************************************************************
#
ccsm$ccb1_info:
        mov     g1,r15                  # save g1
                                        # r15 = event ILT
        ldob    ccsm_op_state,r3        # r3 = CCSM operational state
        cmpobne ccsm_st_master,r3,.ccb1info_1000 # Jif not the master CCSM
        ld      ccsm_gr_info_id(r15),g0 # g0 = reg. ID
        ld      ccsm_gr_info_cmsn(r15),g1 # g1 = CM serial #
        ld      ccsm_e_sendsn(r15),g4   # g4 = sender's serial #
        call    CM$find_cor_rid         # find assoc. COR address
        cmpobne 0,g0,.ccb1info_200      # Jif COR assoc. with this event
        ld      ccsm_gr_info_id(r15),g1 # g1 = reg. ID
        ld      ccsm_gr_info_cmsn(r15),g2 # g2 = CM serial #
        ldconst 0x02,g0                 # g0 = terminate copy process code
        call    CCSM$snd_process2       # pack and send a Process
                                        #  message to this sender
                                        #  indicating to terminate the copy
        b       .ccb1info_1000
#
.ccb1info_200:
        mov     g0,g3                   # g3 = assoc. COR address
        ld      cor_powner(g3),r4       # r4 = current primary owner
        cmpobne r4,g4,.ccb1info_1000    # Jif not from primary copy owner
        ldq     ccsm_gr_info_label(r15),r4
        ldq     cor_label(g3),r8
        cmpobne r4,r8,.ccb1info_400     # Jif cor_label values differ
        cmpobne r5,r9,.ccb1info_400     # Jif cor_label values differ
        cmpobne r6,r10,.ccb1info_400    # Jif cor_label values differ
        cmpobne r7,r11,.ccb1info_400    # Jif cor_label values differ
        ldob    ccsm_gr_info_gid(r15),r4
        ldob    cor_gid(g3),r5
        cmpobne r4,r5,.ccb1info_400     # Jif cor_gid value differs
#
# --- Values match with local values. Send Copy General Info. Changed
#       Acknowledgement message to sender.
#
        ldob    CCSM_mupd_timer,r4      # r4 = scheduled update timer
        cmpobne 0,r4,.ccb1info_1000     # Jif an update is pending
        ldos    ccsm_e_seq(r15),g2      # g2 = request seq. #
        call    CCSM$snd_infoack        # pack and send Copy Info. Changed
                                        #  Acknowledgement message to sender
        b       .ccb1info_1000
#
# --- Copy general info. does not match the local copy info. values. Process
#       the differences.
#
.ccb1info_400:
        ldq     ccsm_gr_info_label(r15),r4
        stq     r4,cor_label(g3)        # save cor_label values
        ldob    ccsm_gr_info_gid(r15),r4
        stob    r4,cor_gid(g3)          # save cor_gid value
        call    CCSM$p2update           # schedule timed update
.ccb1info_1000:
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: ccsm$sex_swap
#
#  PURPOSE:
#       This routine processes a Swap RAIDs Define Event for the swap
#       RAIDs CCSM process.
#
#  CALLING SEQUENCE:
#       call    ccsm$sex_swap
#
#  INPUT:
#       g1 = Swap RAIDs Define event ILT
#
#  OUTPUT:
#       g1 = ILT to return
#       g1 = 0 if ILT queued to pending swap RAIDs request queue
#
#  REGS DESTROYED:
#       Reg. g1 destroyed.
#
#**********************************************************************
#
ccsm$sex_swap:
        movq    g0,r12                  # save g0-g3
        ld      ccsm_de_swap_reg(r13),g0 # g0 = reg. ID
        ld      ccsm_de_swap_cmsn(r13),g1 # g1 = CM serial #
        call    CM$find_cor_rid         # find assoc. COR address
        cmpobe  0,g0,.sexswap_1000      # Jif no COR assoc. with this event
        mov     g0,g3                   # g3 = assoc. COR address
#
# --- Validate incoming swap RAIDs request
#
        ld      cor_srcvdd(g3),r6       # r6 = source VDD address
        cmpobe  0,r6,.sexswap_1000      # Jif no source VDD defined
        ld      cor_destvdd(g3),r8      # r8 = dest. VDD address
        cmpobe  0,r8,.sexswap_1000      # Jif no dest. VDD defined
        ld      vd_rdd(r6),r7           # r7 = source RDD address
        cmpobe  0,r7,.sexswap_1000      # Jif no source RDD defined
        ldos    rd_rid(r7),r6           # r6 = source RDD id
        ld      ccsm_de_swap_rid(r13),r5 # r5 = new source RDD id
        cmpobe  r5,r6,.sexswap_1000     # Jif swap already has occurred
        ld      vd_rdd(r8),r7           # r7 = dest. RDD address
        cmpobe  0,r7,.sexswap_1000      # Jif no dest. RDD defined
        ldos    rd_rid(r7),r6           # r6 = dest. RDD id
        cmpobne r5,r6,.sexswap_1000     # Jif dest. RDD id not correct
        ldob    ccsm_de_swap_type(r13),r4 # r4 = swap type code
        cmpoble 2,r4,.sexswap_1000      # Jif invalid swap type code
#
# --- Incoming swap RAIDs request seems to be valid
#
        mov     r13,g1                  # g1 = event ILT
        ldconst 0,r4
        st      r4,il_fthd(g1)          # clear ILT forward thread
        mov     r4,r13                  # clear returned g1 value to disable
                                        #  auto return to requestor
        ldl     ccsm_swaphead,r4        # r4 = swap RAIDs request queue head
                                        # r5 = swap RAIDs request queue tail
        cmpobne 0,r5,.sexswap_200       # Jif queue not empty
#
# --- Swap RAIDs request queue empty
#
        mov     g1,r4
        mov     g1,r5
        stl     r4,ccsm_swaphead        # put event ILT on request queue
        call    cm$Validate_Swap        # check if swap RAIDs op. is valid
        cmpobe  0,g0,.sexswap_100       # Jif swap op. is valid
#
# Raids Swap validation failed, cor is in g3, mark the cor as swap failed
#
        ldconst gr_swap_failed,r4
#### DEBUG
        ldob    cor_rcsvd(g3),r6
        ldob    cor_rcdvd(g3),r7
c fprintf(stderr,"%s%s:%u <GR><ASWAP>Failed-svid=%lx dvid=%lx reason=1\n", FEBEMESSAGE, __FILE__, __LINE__,r6,r7);
#### DEBUG
        stob      r4,cor_swapcompstat(g3)
        call    ccsm$swap_end           # swap RAIDs request has ended
                                        # remove from queue and start next
                                        # swap RAIDs request if necessary
        b       .sexswap_1000
#
.sexswap_100:
        ldconst ccsm_swapto,r6          # r6 = swap RAIDs timer value
        stob    r6,ccsm_swap_to         # start swap RAIDs timer
        ldconst ccsm_swaprty,r6         # r6 = swap RAIDs retry count
        ldob    cor_userswap(g3),r10    # load user swap byte
        cmpobne 1,r10,.sexswap_105      # jif not user swap
        ldconst ccsm_usrswaprty,r6      # r6 = user swap RAIDs retry count
#
.sexswap_105:
        stob    r6,ccsm_de_swap_rty(g1) # initialize swap RAIDs retry count
        call    CCSM$get_seqnum         # get a sequence # to use
                                        # g2 = sequence # to use
        stos    g2,ccsm_e_seq(g1)       # save sequence # in ILT
        call    CCSM$snd_disvlchk       # disable VLink check on all nodes
        call    CCSM$snd_disfresh       # disable refresh NVRAM on all nodes
        ld      ccsm_de_swap_rid(g1),g0 # g0 = new source RDD id
        call    CCSM$snd_swap           # pack and send Swap RAIDs message
        b       .sexswap_1000
#
.sexswap_200:
#
# --- Swap RAIDs request queue not empty
#
        st      g1,il_fthd(r5)          # link incoming request to end of queue
        st      g1,ccsm_swaptail        # save new queue tail pointer
.sexswap_1000:
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: ccsm$sex_timer
#
#  PURPOSE:
#
#       This routine processes a Timer Define Event for the swap
#       RAIDs CCSM process.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    ccsm$sex_timer
#
#  INPUT:
#
#       g1 = Timer Define event ILT
#
#  OUTPUT:
#
#       g1 = ILT to return
#
#  REGS DESTROYED:
#
#       No regs. destroyed.
#
#**********************************************************************
#
ccsm$sex_timer:
        movq    g0,r12                  # save g0-g3
        ld      ccsm_swaphead,r11       # r11 = top swap RAIDs ILT on queue
        cmpobe  0,r11,.sextimer_1000    # Jif no swap RAIDs ILTs on queue
        ld      ccsm_de_swap_reg(r11),g0 # g0 = reg. ID
        ld      ccsm_de_swap_cmsn(r11),g1 # g1 = CM serial #
        call    CM$find_cor_rid         # find assoc. COR address
        cmpobne 0,g0,.sextimer_200      # Jif COR assoc. with this event
#
# --- No COR assoc. with this swap RAIDs request
#
.sextimer_100:
        call    CCSM$snd_envlchk        # enable VLink check on all nodes
        call    CCSM$snd_enfresh        # enable refresh NVRAM on all nodes
        call    CCSM$savenrefresh       # save P2 and refresh slaves
        ldconst 1000,g0                 # g0 = delay (in millisecs.) to wait
                                        #  to start the next swap op.
        call    K$twait                 # wait for timeout to expire
        call    ccsm$swap_end           # swap RAIDs request has ended
                                        # remove from queue and start next
                                        # swap RAIDs request if necessary
        b       .sextimer_1000
#
.sextimer_200:
        mov     g0,g3                   # g3 = assoc. COR address
        ldob    ccsm_de_swap_rty(r11),r4 # r4 = swap RAIDs retry count
        subo    1,r4,r4                 # dec. retry count
# If the retry count expires, mark on cor that the swap is failed,
# and then go to the ending process (.sextimer_100)
        cmpobe  0,r4,.sextimer_300      # Jif retry count has expired
        stob    r4,ccsm_de_swap_rty(r11) # save updated retry count
        ldos    ccsm_e_seq(r11),g2      # g2 = sequence #
        ld      ccsm_de_swap_rid(r11),g0 # g0 = new source RDD id
        call    CCSM$snd_swap           # pack and send Swap RAIDs message
        ldconst ccsm_swapto,r6          # r6 = swap RAIDs timer value
        stob    r6,ccsm_swap_to         # reset swap RAIDs timer
        b      .sextimer_1000
#
.sextimer_300:
        ldconst gr_swap_failed,r4
### DEBUG
        ldob    cor_rcsvd(g3),r6
        ldob    cor_rcdvd(g3),r7
c fprintf(stderr,"%s%s:%u <GR><ASWAP>Failed-svid=%lx dvid=%lx reason=2\n", FEBEMESSAGE, __FILE__, __LINE__,r6,r7);
### DEBUG
        stob     r4,cor_swapcompstat(g3)
        b       .sextimer_100          # retry count expired
#
.sextimer_1000:
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: ccsm$sex_swapdone
#
#  PURPOSE:
#
#       This routine processes a Swap RAIDs Completed CCBGram Event
#       for the swap RAIDs CCSM process.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    ccsm$sex_swapdone
#
#  INPUT:
#
#       g1 = Swap RAIDs Completed CCBGram event ILT
#
#  OUTPUT:
#
#       g1 = ILT to return
#
#  REGS DESTROYED:
#
#       No regs. destroyed.
#
#**********************************************************************
#
ccsm$sex_swapdone:
        movq    g0,r12                  # save g0-g3
                                        # r13 = ILT
        ld      ccsm_swaphead,r11       # r11 = top swap RAIDs request on queue
        cmpobe  0,r11,.sexdone_1000     # Jif no swap RAIDs requests on queue
        ldos    ccsm_e_seq(r11),r4      # r4 = seq. # for current swap op.
        ldos    ccsm_e_seq(r13),r5      # r5 = seq. # from swap done message
        cmpobne r4,r5,.sexdone_1000     # Jif seq. # not what is expected
        ld      ccsm_gr_swapdone_id(r13),g0 # g0 = copy reg. ID
        ld      ccsm_gr_swapdone_cmsn(r13),g1 # g1 = CM serial #
        ld      ccsm_de_swap_reg(r11),r4 # r4 = current swap op. reg. ID
        ld      ccsm_de_swap_cmsn(r11),r5 # r5 = current swap op. CM serial #
        cmpobne g0,r4,.sexdone_1000     # Jif wrong copy reg. ID
        cmpobne g1,r5,.sexdone_1000     # Jif wrong CM serial #
        call    CM$find_cor_rid         # find assoc. COR address
        cmpobne 0,g0,.sexdone_200       # Jif COR assoc. with this event
#
# --- No COR assoc. with this swap RAIDs request
#
        call    CCSM$snd_envlchk        # enable VLink check on all nodes
        call    CCSM$snd_enfresh        # enable refresh NVRAM on all nodes
        call    CCSM$savenrefresh       # save P2 and refresh slaves
        ldconst 1000,g0                 # g0 = delay (in millisecs.) to wait
                                        #  to start the next swap op.
        call    K$twait                 # wait for timeout to expire
        call    ccsm$swap_end           # swap RAIDs request has ended
                                        # remove from queue and start next
                                        # swap RAIDs request if necessary
        b       .sexdone_1000
#
.sexdone_200:
#
# --- Found the COR associated with this swap operation
#
        mov     g0,g3                   # g3 = assoc. COR address
        ldob    ccsm_de_swap_flag(r11),r8 # r8 = swap op. flag byte
        ld      ccsm_e_sendsn(r13),r4   # r4 = sender's serial #
        ld      K_ficb,r3               # r3 = FICB address
        ld      fi_cserial(r3),r5       # r5 = controller serial #
        cmpobne r4,r5,.sexdone_300      # Jif not from this node
        setbit  0,r8,r8                 # set master done flag
.sexdone_300:
        ld      cor_powner(g3),r5       # r5 = current copy owner
        cmpobne r4,r5,.sexdone_400      # Jif not from the copy owner
        setbit  1,r8,r8                 # set owner done flag
.sexdone_400:
        stob    r8,ccsm_de_swap_flag(r11) # save swap op. flag byte
        cmpobne 3,r8,.sexdone_1000      # Jif this swap op. not done
#
# --- The current swap RAIDs operation has completed
#
        call    CCSM$snd_envlchk        # enable VLink check on all nodes
        call    CCSM$snd_enfresh        # enable refresh NVRAM on all nodes
        ldob    ccsm_de_swap_type(r11),r4 # r4 = swap RAID type code
        cmpobne 1,r4,.sexdone_500       # Jif not copy/swap/break type
        call    CCSM$term_copy          # initiate terminate copy
        ldconst 2000,g0                 # g0 = delay (in millisecs.) to wait
                                        #  for copy to terminate
        call    K$twait                 # wait for timeout to expire
        b       .sexdone_700
#
.sexdone_500:
        call    CCSM$savenrefresh       # save P2 and refresh slaves
        ldconst 1000,g0                 # g0 = delay (in millisecs.) to wait
                                        #  to start the next swap op.
        call    K$twait                 # wait for timeout to expire
#
#      Here, mark on the cor that the swap operation is successful.
#      Use a reserved byte in the cor to store completion status.
#
        ldconst gr_swap_success,r4
        stob r4,cor_swapcompstat(g3)
.sexdone_700:
        call    ccsm$swap_end           # swap RAIDs request has ended
                                        # remove from queue and start next
                                        # swap RAIDs request if necessary
.sexdone_1000:
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: ccsm$swap_end
#
#  PURPOSE:
#
#       This routine ends the top swap RAIDs request by removing it
#       from the queue and calling the completion handler and then
#       initiates the next swap RAIDs request on the queue if
#       necessary.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    ccsm$swap_end
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
#       No regs. destroyed.
#
#**********************************************************************
#
ccsm$swap_end:
        movq    g0,r12                  # save g0-g3
        ldconst 0,r4
        stob    r4,ccsm_swap_to         # stop swap RAIDs timer
        ld      ccsm_swaphead,g1        # g1 = top swap RAIDs ILT on queue
        cmpobe  0,g1,.swapend_1000      # Jif no ILTs on queue

#Get COR from the swap event ilt
#Here we have to call the auto swap notify function
        mov     g1,r5                   #save g1
        ld      ccsm_de_swap_reg(r5),g0 # g0 = reg. ID
        ld      ccsm_de_swap_cmsn(r5),g1 # g1 = CM serial #
        call    CM$find_cor_rid         # find assoc. COR address
        cmpobe  0,g0,.swapend200a
        PushRegs(r4)
        call    GR_NotifySwapComp       # g0 contains COR
        PopRegsVoid(r4)
.swapend200a:
        mov     r5,g1                   #restore g1

        ld      il_fthd(g1),r4          # r4 = next ILT on queue
        st      r4,ccsm_swaphead        # remove ILT from queue
        cmpobne 0,r4,.swapend_200       # Jif more ILTs on queue
        st      r4,ccsm_swaptail        # clear queue tail pointer
.swapend_200:
        ldconst 0,r4
        st      r4,il_fthd(g1)          # clear forward thread of ILT
        ld      il_cr(g1),r4            # r4 = completion handler routine
        callx   (r4)                    # return event to requestor

        ld      ccsm_swaphead,r11       # r11 = top swap RAIDs ILT on queue
        cmpobe  0,r11,.swapend_1000     # Jif no ILTs on queue
#
# --- There's another swap RAIDs request on the queue
#
        ld      ccsm_de_swap_reg(r11),g0 # g0 = reg. ID
        ld      ccsm_de_swap_cmsn(r11),g1 # g1 = CM serial #
        call    CM$find_cor_rid         # find assoc. COR address
        cmpobne 0,g0,.swapend_400       # Jif COR assoc. with this event
        b       .swapend_300a           # this is done due to addition
                                        # of the following code
.swapend_300:
#
# Swap validation failed, mark cor (swapcompstat) as failed.
#
        ldconst gr_swap_failed,r4
#### DEBUG
        ldob    cor_rcsvd(g3),r6
        ldob    cor_rcdvd(g3),r7
c fprintf(stderr,"%s%s:%u <GR><ASWAP>Failed-svid=%lx dvid=%lx reason=4\n", FEBEMESSAGE, __FILE__, __LINE__,r6,r7);
#### DEBUG
        stob    r4,cor_swapcompstat(g3)
.swapend_300a:
        movq    r12,g0                  # restore g0-g3
        b       ccsm$swap_end           # terminate this swap RAIDs request
#
.swapend_400:
        mov     g0,g3                   # g3 = assoc. COR address
#
# --- Validate incoming swap RAIDs request
#
        ld      cor_srcvdd(g3),r6       # r6 = source VDD address
        cmpobe  0,r6,.swapend_300       # Jif no source VDD defined
        ld      cor_destvdd(g3),r8      # r8 = dest. VDD address
        cmpobe  0,r8,.swapend_300       # Jif no dest. VDD defined
        ld      vd_rdd(r6),r7           # r7 = source RDD address
        cmpobe  0,r7,.swapend_300       # Jif no source RDD defined
        ldos    rd_rid(r7),r6           # r6 = source RDD id
        ld      ccsm_de_swap_rid(r11),r5 # r5 = new source RDD id
        cmpobe  r5,r6,.swapend_300a     # Jif swap already has occurred
        ld      vd_rdd(r8),r7           # r7 = dest. RDD address
        cmpobe  0,r7,.swapend_300       # Jif no dest. RDD defined
        ldos    rd_rid(r7),r6           # r6 = dest. RDD id
        cmpobne r5,r6,.swapend_300      # Jif dest. RDD id not correct
        ldob    ccsm_de_swap_type(r11),r4 # r4 = swap type code
        cmpoble 2,r4,.swapend_300       # Jif invalid swap type code
        call    cm$Validate_Swap        # check if swap RAIDs op. is valid
        cmpobne 0,g0,.swapend_300       # Jif swap op. is not valid
#
# --- Swap RAIDs request seems to still be valid. Start the swap
#       RAIDs process.
#
        ldconst ccsm_swapto,r6          # r6 = swap RAIDs timer value
        stob    r6,ccsm_swap_to         # start swap RAIDs timer
        ldconst ccsm_swaprty,r6         # r6 = swap RAIDs retry count
        stob    r6,ccsm_de_swap_rty(r11) # initialize swap RAIDs retry count
        call    CCSM$get_seqnum         # get a sequence # to use
                                        # g2 = sequence # to use
        stos    g2,ccsm_e_seq(r11)      # save sequence # in ILT
        call    CCSM$snd_disvlchk       # disable VLink check on all nodes
        call    CCSM$snd_disfresh       # disable refresh NVRAM on all nodes
        ld      ccsm_de_swap_rid(r11),g0 # g0 = new source RDD id
        call    CCSM$snd_swap           # pack and send Swap RAIDs message
.swapend_1000:
        movq    r12,g0                  # restore g0-g3
        ret
#
# --- End of CCBGram Received Event Handler Tables and Routines Area ----------
#
#**********************************************************************
#
#  NAME: CCSM$init
#
#  PURPOSE:
#
#       This routine contains the copy configuration and status
#       manager initialization logic.
#
#  DESCRIPTION:
#
#       This routine contains the logic to initialize copy configuration
#       and status manager services.
#
#  CALLING SEQUENCE:
#
#       call    CCSM$init
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
CCSM$init:
        movl    g0,r12                  # save g0-g1
#
# --- Establish CCSM service provider executive process
#
        lda     ccsm$exec,g0            # Establish CCSM service
                                        #  provider executive process
        ldconst CMSPEXECPRI,g1
c       CT_fork_tmp = (ulong)"ccsm$exec";
        call    K$fork
        st      g0,ccsm_sp_qu+qu_pcb    # Save PCB
#
# --- Establish Swap RAIDs CCSM service provider executive process
#
        lda     ccsm$swapexec,g0        # Establish swap RAIDs CCSM service
                                        #  provider executive process
        ldconst CMSPEXECPRI,g1
c       CT_fork_tmp = (ulong)"ccsm$swapexec";
        call    K$fork
        st      g0,ccsmswap_sp_qu+qu_pcb # Save PCB
#
# --- Establish CCSM timer process
#
        lda     ccsm$timer,g0           # Establish CCSM timer process
        ldconst CMSPEXECPRI,g1
c       CT_fork_tmp = (ulong)"ccsm$timer";
        call    K$fork
#
# --- Put my controller serial # in CCBGram packet templates
#
        ld      K_ficb,r3               # r3 = FICB address
        ld      fi_cserial(r3),r5       # r5 = controller serial #
        st      r5,snd_process_sn
        st      r5,snd_update_sn
        st      r5,snd_force_sn
        st      r5,snd_define_sn
        st      r5,snd_owner_sn
        st      r5,snd_trans_sn
        st      r5,snd_change_sn
        st      r5,snd_susp_sn
        st      r5,snd_sdone_sn
        st      r5,snd_cdone_sn
        st      r5,snd_term_sn
        st      r5,snd_tdone_sn
        st      r5,snd_readrm_sn
        st      r5,snd_rm_sn
        st      r5,snd_readsm_sn
        st      r5,snd_copyreg_sn
        st      r5,snd_swap_sn
        st      r5,snd_swapdone_sn
        st      r5,snd_disvlchk_sn
        st      r5,snd_disfresh_sn
        st      r5,snd_envlchk_sn
        st      r5,snd_enfresh_sn
        st      r5,snd_cdmoved_sn
        st      r5,snd_cdmack_sn
        st      r5,snd_csc_sn
        st      r5,snd_cscack_sn
        st      r5,snd_info_sn
        st      r5,snd_infoack_sn
        movl    r12,g0                  # restore g0-g1
        ret
#
#**********************************************************************
#
#  NAME: CCSM$new_master
#
#  PURPOSE:
#
#       This routine generates a New Master CCSM define event and queues
#       it to the local CCSM process.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$new_master
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
CCSM$new_master:
        ld      ccsm_sp_qu+qu_pcb,r3    # check if CCSM$init has completed
        cmpobne 0,r3,.newmaster_200     # yes.
        ldconst ccsm_st_master,r3       # set CCSM state to master
        stob    r3,ccsm_op_state
        b       .newmaster_1000
#
.newmaster_200:
        mov     g1,r15                  # save g1
c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
        ldq     ccsm_event_base1,r4     # r4-r7 = base event "header"
        ldt     ccsm_event_new_master,r8 # r8 = new master CCSM template
        stq     r4,(g1)                 # save base event "header"
        stt     r8,ccsm_e_len(g1)       # save event data
        call    CCSM$que                # queue event to CCSM event queue
        mov     r15,g1                  # restore g1
.newmaster_1000:
        ret
#
#**********************************************************************
#
#  NAME: CCSM$not_master
#
#  PURPOSE:
#
#       This routine generates a No Longer Master CCSM define event
#       and queues it to the local CCSM process.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$not_master
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
CCSM$not_master:
        ld      ccsm_sp_qu+qu_pcb,r3    # check if CCSM$init has completed
        cmpobne 0,r3,.notmaster_200     # yes.
        ldconst ccsm_st_notmaster,r3    # set CCSM state to not master
        stob    r3,ccsm_op_state
        b       .notmaster_1000
#
.notmaster_200:
        mov     g1,r15                  # save g1
c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
        ldq     ccsm_event_base1,r4     # r4-r7 = base event "header"
        ldt     ccsm_event_no_master,r8 # r8 = no longer master CCSM template
        stq     r4,(g1)                 # save base event "header"
        stt     r8,ccsm_e_len(g1)       # save event data
        call    CCSM$que                # queue event to CCSM event queue
        mov     r15,g1                  # restore g1
.notmaster_1000:
        ret
#
#**********************************************************************
#
#  NAME: CCSM$start copy
#
#  PURPOSE:
#
#       This routine generates a Start Copy define event and queues it
#       to the local CCSM process.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$start_copy
#
#  INPUT:
#
#       g3 = assoc. COR address of copy
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
CCSM$start_copy:
CCSM_start_copy:
        mov     g1,r15                  # save g1
.if GR_GEORAID15_DEBUG
        ldob    cor_rcsvd(g3),r5
        ldob    cor_rcdvd(g3),r4
c fprintf(stderr,"%s%s:%u <GR><CCSM$start_copy>srcvid=%lx,dvid=%lx..create ILT and queue event to CCSM$que\n", FEBEMESSAGE, __FILE__, __LINE__,r5,r4);
c fprintf(stderr,"%s%s:%u <GR><CCSM$start_copy>This request put in ccsm-sp-qu is processed by ccsm$exec..\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # GR_GEORAID15_DEBUG
c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
        ldq     ccsm_event_base1,r4     # r4-r7 = base event "header"
        ldt     ccsm_event_start,r8     # r8 = start copy CCSM template
        stq     r4,(g1)                 # save base event "header"
        stt     r8,ccsm_e_len(g1)       # save event data
        st      g3,ccsm_e_ext(g1)       # save COR address
        call    CCSM$que                # queue event to CCSM event queue
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: CCSM$term copy
#
#  PURPOSE:
#
#       This routine generates a Terminate Copy define event and queues it
#       to the local CCSM process.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$term_copy
#
#  INPUT:
#
#       g3 = assoc. COR address of copy
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
CCSM$term_copy:
CCSM_term_copy:
        mov     g1,r15                  # save g1
c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
        ldq     ccsm_event_base1,r4     # r4-r7 = base event "header"
        ldt     ccsm_event_term,r8      # r8 = terminate copy CCSM template
        stq     r4,(g1)                 # save base event "header"
        stt     r8,ccsm_e_len(g1)       # save event data
        ld      cor_rid(g3),r4          # r4 = reg. ID
        ld      cor_rcsn(g3),r5         # r5 = CM serial #
        stl     r4,ccsm_e_ext(g1)       # save reg. ID & CM serial #
        call    CCSM$que                # queue event to CCSM event queue
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: CCSM$pause copy
#
#  PURPOSE:
#
#       This routine generates a Pause Copy define event and queues it
#       to the local CCSM process.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$pause_copy
#
#  INPUT:
#
#       g3 = assoc. COR address of copy
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
CCSM_pause_copy:
CCSM$pause_copy:
        mov     g1,r15                  # save g1
c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
        ldq     ccsm_event_base1,r4     # r4-r7 = base event "header"
        ldt     ccsm_event_pause,r8     # r8 = pause copy CCSM template
        stq     r4,(g1)                 # save base event "header"
        stt     r8,ccsm_e_len(g1)       # save event data
        ld      cor_rid(g3),r4          # r4 = reg. ID
        ld      cor_rcsn(g3),r5         # r5 = CM serial #
        stl     r4,ccsm_e_ext(g1)       # save reg. ID & CM serial #
        call    CCSM$que                # queue event to CCSM event queue
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: CCSM$resume copy
#
#  PURPOSE:
#
#       This routine generates a Resume Copy define event and queues it
#       to the local CCSM process.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$resume_copy
#
#  INPUT:
#
#       g3 = assoc. COR address of copy
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
#C access
# void CCSM_resume_copy(COR* p_cor);
CCSM_resume_copy:
        mov    g0,g3
CCSM$resume_copy:
        mov     g1,r15                  # save g1
c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
        ldq     ccsm_event_base1,r4     # r4-r7 = base event "header"
        ldt     ccsm_event_resume,r8    # r8 = resume copy CCSM template
        stq     r4,(g1)                 # save base event "header"
        stt     r8,ccsm_e_len(g1)       # save event data
        ld      cor_rid(g3),r4          # r4 = reg. ID
        ld      cor_rcsn(g3),r5         # r5 = CM serial #
        stl     r4,ccsm_e_ext(g1)       # save reg. ID & CM serial #
        call    CCSM$que                # queue event to CCSM event queue
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: CCSM$swap_raids
#
#  PURPOSE:
#
#       This routine generates a Swap RAIDs define event and queues it
#       to the local CCSM process.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$swap_raids
#
#  INPUT:
#
#       g0 = swap type code
#            0 = swap and mirror
#            1 = swap and break
#       g3 = assoc. COR address of copy
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
#C access
# void CCSM_swap_raids(UINT32 swapType, COR* p_cor);
CCSM_swap_raids:
        mov    g1,g3
CCSM$swap_raids:
        mov     g1,r15                  # save g1
        ldob    ccsm_op_state,r3        # r3 = CCSM state
        cmpobne ccsm_st_master,r3,.swapraids_1000 # Jif not the master CCSM
        cmpoble 2,g0,.swapraids_1000    # Jif invalid swap type code
        ld      cor_destvdd(g3),r12     # r12 = destination VDD address
        cmpobe  0,r12,.swapraids_1000   # Jif no dest. VDD defined
        ld      vd_rdd(r12),r13         # r13 = first RDD assoc. with VDD
        cmpobe  0,r13,.swapraids_1000   # Jif no RDD assoc. with VDD
        ldos    rd_rid(r13),r12         # r12 = RAID id of first RDD assoc.
                                        #  with dest. VDD
c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
        ldq     ccsm_event_base1,r4     # r4-r7 = base event "header"
        ldt     ccsm_event_swap,r8      # r8 = swap RAIDs CCSM template
        stq     r4,(g1)                 # save base event "header"
        stt     r8,ccsm_e_len(g1)       # save event data
        ld      cor_rid(g3),r4          # r4 = copy reg. ID
        ld      cor_rcsn(g3),r5         # r5 = CM serial #
        stl     r4,ccsm_de_swap_reg(g1) # save in event
        st      r12,ccsm_de_swap_rid(g1) # save new source copy device RAID id
        st      g0,ccsm_de_swap_type(g1) # save swap type code and clear
                                        #  other fields
        call    CCSM$queswap            # queue event to swap RAIDs CCSM
                                        #  event queue
.swapraids_1000:
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: CCSM$cco
#
#  PURPOSE:
#
#       This routine generates a Configuration Change Occurred define
#       event and queues it to the local CCSM process.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$cco
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
CCSM_cco:
CCSM$cco:
        mov     g1,r15                  # save g1
c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
        ldq     ccsm_event_base1,r4     # r4-r7 = base event "header"
        ldt     ccsm_event_cco,r8       # r8 = configuration change occurred
                                        #      CCSM template
        stq     r4,(g1)                 # save base event "header"
        stt     r8,ccsm_e_len(g1)       # save event data
#
.if     djk_ccorip
#
        st      rip,ccsm_e_ext(g1)      # save return address for debug
#
.endif  # djk_ccorip
#
        call    CCSM$que                # queue event to CCSM event queue
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: CCSM$cosc
#
#  PURPOSE:
#
#       This routine generates a Copy Operational State Changed define
#       event and queues it to the local CCSM process.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$cosc
#
#  INPUT:
#
#       g0 = cor_crstate being invoked
#       g3 = assoc. COR address
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
CCSM_cosc:
        mov     g1,r15                  # save g1
c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
        ldq     ccsm_event_base1,r4     # r4-r7 = base event "header"
        ldt     ccsm_event_cosc,r8      # r8 = copy operational state changed
                                        #      CCSM template
        stq     r4,(g1)                 # save base event "header"
        stt     r8,ccsm_e_len(g1)       # save event data
        ld      cor_rid(g3),r4          # r4 = reg. ID
        ld      cor_rcsn(g3),r5         # r5 = CM serial #
        mov     g0,r6                   # r6 = new cor_crstate value
        stt     r4,ccsm_e_ext(g1)       # save reg. ID & CM serial #
                                        #  and cor_crstate value
        call    CCSM$que                # queue event to CCSM event queue
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: CCSM$timer
#
#  PURPOSE:
#
#       This routine generates a 1 Second Timer define
#       event and queues it to the local CCSM process.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$timer
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
CCSM$timer:
        mov     g1,r15                  # save g1
c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
        ldq     ccsm_event_base1,r4     # r4-r7 = base event "header"
        ldt     ccsm_event_timer,r8     # r8 = 1 sec. timer event template
        stq     r4,(g1)                 # save base event "header"
        stt     r8,ccsm_e_len(g1)       # save event data
        call    CCSM$que                # queue event to CCSM event queue
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: CCSM$swaptimer
#
#  PURPOSE:
#
#       This routine checks the swap RAIDs process timer and if enabled
#       decrements the timer value. If this value goes to 00, it generates
#       a Timer Define event and queues it to the swap RAIDs CCSM process.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$swaptimer
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
CCSM$swaptimer:
        ldob    ccsm_vlchk_to,r4        # r4 = disable VLink check timer
        cmpobe  0,r4,.swaptimer_100     # Jif timer not running
        subo    1,r4,r4                 # dec. timer
        stob    r4,ccsm_vlchk_to        # save updated timer
        cmpobne 0,r4,.swaptimer_100     # Jif timer not expired
        ldob    ccsm_vlchk_flag,r5      # r5 = disable VLink check flag
        cmpobe  0,r5,.swaptimer_100     # Jif VLink check already enabled
        stob    r4,ccsm_vlchk_flag      # clear disable VLink check flag
        ldob    DLM_vlchk_flag,r4       # r4 = disable VLink check flag
        cmpobe  0,r4,.swaptimer_100     # Jif count already 0
        subo    1,r4,r4                 # dec. disable VLink check flag count
        stob    r4,DLM_vlchk_flag       # save updated flag count
.swaptimer_100:
        ldob    ccsm_fresh_to,r4        # r4 = disable refresh NVRAM timer
        cmpobe  0,r4,.swaptimer_200     # Jif timer not running
        subo    1,r4,r4                 # dec. timer
        stob    r4,ccsm_fresh_to        # save updated timer
        cmpobne 0,r4,.swaptimer_200     # Jif timer not expired
        stob    r4,CCSM_fresh_flag      # clear disable refresh NVRAM flag
.swaptimer_200:
        ldob    ccsm_swap_to,r4         # r4 = swap RAIDs process timer
        cmpobe  0,r4,.swaptimer_1000    # Jif timer not enabled
        subo    1,r4,r4                 # dec. timeout value
        stob    r4,ccsm_swap_to         # save updated timeout value
        cmpobne 0,r4,.swaptimer_1000    # Jif timer not expired
        mov     g1,r15                  # save g1
c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
        ldq     ccsm_event_base1,r4     # r4-r7 = base event "header"
        ldt     ccsm_event_timer,r8     # r8 = 1 sec. timer event template
        stq     r4,(g1)                 # save base event "header"
        stt     r8,ccsm_e_len(g1)       # save event data
        call    CCSM$queswap            # queue event to swap RAIDs CCSM
                                        #  event queue
        mov     r15,g1                  # restore g1
.swaptimer_1000:
        ret
#
#**********************************************************************
#
#  NAME: CCSM$upd_check
#
#  PURPOSE:
#
#
#       This routine checks the following copy update processes on
#       the master controller:
#
#       - save NVRAM and send refresh NVRAM message to all slave
#         controllers.
#
#       This routine checks the following copy update processes on
#       slave controllers:
#
#       - copy devices moved process
#       - copy state changed process
#       - copy general information changed process
#
#       If the update timer is expired, it checks for CORs that have
#       any of these processes active. For those CORs that have a
#       process active flag set, it packs and sends the appropriate
#       message to the master CCSM.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$upd_check
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
CCSM$upd_check:
        mov     g3,r15                  # save g3
        ldob    ccsm_op_state,r4        # r4 = CCSM state code
        cmpobne ccsm_st_master,r4,.updcheck_99 # Jif not the master CCSM
#
# --- Master controller copy update process handler routine
#
        ldob    CCSM_mupd_timer,r4      # r4 = master update process timer
        cmpobe  0,r4,.updcheck_1000     # Jif timer not set
        subo    1,r4,r4                 # dec. process timer
        stob    r4,CCSM_mupd_timer      # save updated timer
        cmpobne 0,r4,.updcheck_1000     # Jif process timer not expired
        call    CCSM$savenrefresh       # save P2 and refresh slaves
        ldob    ccsm_mupd_saves,r4      # r4 = save counter
        addo    1,r4,r4                 # inc. counter
        stob    r4,ccsm_mupd_saves      # save updated counter
        b       .updcheck_1000
#
# --- Slave controller copy update process handler routine
#
.updcheck_99:
        ldob    ccsm_supd_timer,r5      # r5 = slave copy update process
                                        #      check timer
        cmpobe  0,r5,.updcheck_100      # Jif timer expired
        subo    1,r5,r5                 # dec. process timer
        stob    r5,ccsm_supd_timer      # save slave update process timer
        cmpobne 0,r5,.updcheck_1000     # Jif process timer not expired
.updcheck_100:
        ldconst ccsm_supd_to,r5         # r5 = slave process timer value
        stob    r5,ccsm_supd_timer      # reset slave process timer
        ld      CM_cor_act_que,r4       # r4 = first COR on active queue
.updcheck_200:
        cmpobe  0,r4,.updcheck_1000     # Jif no CORs on active queue
        ldob    cor_flags(r4),r5        # r5 = cor_flags byte
        bbc     CFLG_B_DIS_DEVICE,r5,.updcheck_300  # Jif disable copy device update
                                        #  from NVRAM flag not set
        mov     r4,g3                   # g3 = COR address
        call    CCSM$snd_cdmoved        # pack and send Copy Devices Moved
                                        #  message to master CCSM
.updcheck_300:
        bbc     CFLG_B_DIS_STATE,r5,.updcheck_400   # Jif disable copy state update
                                        #  from NVRAM flag not set
        mov     r4,g3                   # g3 = COR address
        call    CCSM$snd_csc            # pack and send Copy State Changed
                                        #  message to master CCSM
.updcheck_400:
        bbc     CFLG_B_DIS_GENERAL,r5,.updcheck_600 # Jif disable copy info. update
                                        #  from NVRAM flag not set
        mov     r4,g3                   # g3 = COR address
        call    CCSM$snd_info           # pack and send Copy Info. Changed
                                        #  message to master CCSM
.updcheck_600:
        ld      cor_link(r4),r4         # r4 = next COR on active list
        b       .updcheck_200           # and check next COR on active list
#
.updcheck_1000:
        mov     r15,g3                  # restore g3
        ret
#
#**********************************************************************
#
#  NAME: CCSM$RefreshTimer
#
#  PURPOSE:
#
#       This routine checks to determine if NVRAM refresh may have been
#       lost.
#
#  DESCRIPTION:
#       The COR primary owner is checked. If it is zero, then a NVRAM
#       refresh is issues to the slave controllers
#
#  CALLING SEQUENCE:
#
#       call    CCSM$RefreshTimer
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
CCSM$RefreshTimer:
        ldob    ccsm_op_state,r4        # r4 = CCSM state code
        cmpobne ccsm_st_master,r4,.rshtmr_1000 # Jif not the master CCSM

        ld      CM_cor_act_que,r4       # r4 = first COR on active queue
        cmpobe  0,r4,.rshtmr_1000       # Jif no cors defines

.rshtmr_100:
        ld      cor_powner(r4),r5       # r5 = primary copy owner
        cmpobne 0,r5,.rshtmr_200        # Jif defined

        ld      cor_rid(r4),r6          # r6 = rid number
c fprintf(stderr,"%s%s:%u CCSM - Copy primary owner is NULL. RID = 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, r6);

        call    CCSM$savenrefresh       # tell other nodes to refresh NVRAM
        b       .rshtmr_1000            # exit

.rshtmr_200:
        ld      cor_link(r4),r4         # link to next cor
        cmpobne 0,r4,.rshtmr_100        # Jif not zero

.rshtmr_1000:
        ret
#
#**********************************************************************
#
#  NAME: CCSM$ccbg
#
#  PURPOSE:
#
#       This routine generates a CCBGram Received define
#       event and queues it to the local CCSM process.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$ccbg
#
#  INPUT:
#
#       g0 = address of CCBGram data
#       g1 = length of CCBGram
#
#  OUTPUT:
#
#       g1 = MRP status code
#       g2 = response length
#
#  REGS DESTROYED:
#
#       Reg. g1, g2 destroyed.
#
#**********************************************************************
#
CCSM$ccbg:
        mov     g0,r14                  # save g0-g1
#        movl    g0,r14                  # save g0-g1
                                        # r14 = address of CCBGram data
#                                        # r15 = length of CCBGram
!       ldob    ccsm_e_fc-CCSM_E_OFFSET(r14),r3 # r3 = event function code
        bbc     7,r3,.ccbg_70           # Jif not an immediate CCBGram function
#
# --- Check for Instant Mirror config update datagram.
#
        ldconst ccsm_gr_imcupdt,r4
        cmpobne r3,r4,.ccbg_20a         # Jif not Inst.mirror config update datagram
#
# ---   The Inst mirror config update datagram received from the other DCN (for vdisks owned by other DCN)
#
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>CCSM$ccbg--IM config datagram received from other DCN..call CM_ConfigUpdateDgmRecv\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # CM_IM_DEBUG
c       CM_ImConfigChangeDgmRecv((CM_CONFUPDT_DATAGRAM*)g0);
        b       .ccbg_1000              # and we're done!
.ccbg_20a:
        ldconst ccsm_gr_disvlchk,r4
        cmpobne r3,r4,.ccbg_20d         # Jif not disable VLink check
#
# --- Disable VLink Check message handler routine
#
        ldob    ccsm_vlchk_flag,r4      # r4 = disable VLink check flag
        cmpobne 0,r4,.ccbg_1000         # Jif VLink check already disabled
        ldconst 0xff,r4
        stob    r4,ccsm_vlchk_flag      # set disable VLink check flag
        ldob    DLM_vlchk_flag,r4       # r4 = disable VLink check flag
        addo    1,r4,r4                 # inc. disable VLink check flag count
        stob    r4,DLM_vlchk_flag       # save updated flag count
        ldconst ccsm_vlchkto,r5
        stob    r5,ccsm_vlchk_to        # start disable VLink check timer
        b       .ccbg_1000              # and we're done!
#
.ccbg_20d:
        ldconst ccsm_gr_disfresh,r4
        cmpobne r3,r4,.ccbg_20e         # Jif not disable refresh NVRAM
#
# --- Disable Refresh NVRAM message handler routine
#
        ldconst 0xff,r4
        stob    r4,CCSM_fresh_flag      # set disable refresh NVRAM flag
        ldconst ccsm_freshto,r5
        stob    r5,ccsm_fresh_to        # start disable refresh NVRAM timer
        b       .ccbg_1000              # and we're done!
#
.ccbg_20e:
        ldconst ccsm_gr_envlchk,r4
        cmpobne r3,r4,.ccbg_20f         # Jif not enable VLink check
#
# --- Enable VLink Check message handler routine
#
        ldob    ccsm_vlchk_flag,r4      # r4 = disable VLink check flag
        cmpobe  0,r4,.ccbg_1000         # Jif VLink check already enabled
        ldconst 0,r4
        stob    r4,ccsm_vlchk_flag      # clear disable VLink check flag
        stob    r4,ccsm_vlchk_to        # stop disable VLink check timer
        ldob    DLM_vlchk_flag,r4       # r4 = disable VLink check flag
        cmpobe  0,r4,.ccbg_1000         # Jif count already 0
        subo    1,r4,r4                 # dec. disable VLink check flag count
        stob    r4,DLM_vlchk_flag       # save updated flag count
        b       .ccbg_1000              # and we're done!
#
.ccbg_20f:
        ldconst ccsm_gr_enfresh,r4
        cmpobne r3,r4,.ccbg_20g         # Jif not enable refresh NVRAM
#
# --- Enable Refresh NVRAM message handler routine
#
        ldconst 0,r4
        stob    r4,CCSM_fresh_flag      # clear disable refresh NVRAM flag
        stob    r4,ccsm_fresh_to        # stop disable refresh NVRAM timer
        b       .ccbg_1000              # and we're done!
#
.ccbg_20g:
        ldconst ccsm_gr_cdmack,r4
        cmpobne r3,r4,.ccbg_20h         # Jif not copy devices moved ack
#
# --- Copy Devices Moved Acknowledgement message handler routine
#
        ld      ccsm_gr_cdmack_id-CCSM_E_OFFSET(r14),g0 # g0 = copy reg. ID
        ld      ccsm_gr_cdmack_cmsn-CCSM_E_OFFSET(r14),g1 # g1 = CM task serial #
        call    CM$find_cor_rid         # find assoc. COR address
        cmpobe  0,g0,.ccbg_1000         # Jif no COR assoc. with this event
        ldq     ccsm_gr_cdmack_rcscl-CCSM_E_OFFSET(r14),r4
        ldq     cor_rcscl(g0),r8
        cmpobne r4,r8,.ccbg_1000        # Jif values don't match
        cmpobne r5,r9,.ccbg_1000        # Jif values don't match
        cmpobne r6,r10,.ccbg_1000       # Jif values don't match
        cmpobne r7,r11,.ccbg_1000       # Jif values don't match
#
# --- Values match. Clear disable copy device update from NVRAM flag.
#
        ldob    cor_flags(g0),r4        # r4 = cor_flags byte
        clrbit  CFLG_B_DIS_DEVICE,r4,r4 # Clear flag
        stob    r4,cor_flags(g0)        # save update cor_flags byte
        b       .ccbg_1000              # and we're done!
#
.ccbg_20h:
        ldconst ccsm_gr_cscack,r4
        cmpobne r3,r4,.ccbg_20i         # Jif not copy state changed ack
#
# --- Copy State Changed Acknowledgement message handler routine
#
        ld      ccsm_gr_cscack_id-CCSM_E_OFFSET(r14),g0 # g0 = copy reg. ID
        ld      ccsm_gr_cscack_cmsn-CCSM_E_OFFSET(r14),g1 # g1 = CM task serial #
        call    CM$find_cor_rid         # find assoc. COR address
        cmpobe  0,g0,.ccbg_1000         # Jif no COR assoc. with this event
        ldob    ccsm_gr_cscack_crstate-CCSM_E_OFFSET(r14),r4
        ldob    cor_crstate(g0),r5
        cmpobne r4,r5,.ccbg_1000        # Jif cor_crstate values don't match
        ldob    ccsm_gr_cscack_mstate-CCSM_E_OFFSET(r14),r4
        ldob    cor_mstate(g0),r5
        cmpobne r4,r5,.ccbg_1000        # Jif cor_mstate values don't match
        ldob    ccsm_gr_cscack_cstate-CCSM_E_OFFSET(r14),r4
        ldob    cor_cstate(g0),r5
        cmpobne r4,r5,.ccbg_1000        # Jif cor_cstate values don't match
#
# --- Values match. Clear disable copy state update from NVRAM flag.
#
        ldob    cor_flags(g0),r4        # r4 = cor_flags byte
        clrbit  CFLG_B_DIS_STATE,r4,r4  # Clear flag
        stob    r4,cor_flags(g0)        # save update cor_flags byte
        b       .ccbg_1000              # and we're done!
#
.ccbg_20i:
        ldconst ccsm_gr_infoack,r4
        cmpobne r3,r4,.ccbg_1000        # Jif not copy info. changed ack
#
# --- Copy General Info. Changed Acknowledgement message handler routine
#
        ld      ccsm_gr_infoack_id-CCSM_E_OFFSET(r14),g0 # g0 = copy reg. ID
        ld      ccsm_gr_infoack_cmsn-CCSM_E_OFFSET(r14),g1 # g1 = CM task serial #
        call    CM$find_cor_rid         # find assoc. COR address
        cmpobe  0,g0,.ccbg_1000         # Jif no COR assoc. with this event
        ldob    ccsm_gr_infoack_gid-CCSM_E_OFFSET(r14),r4
        ldob    cor_gid(g0),r5
        cmpobne r4,r5,.ccbg_1000        # Jif cor_gid values don't match
        ldq     ccsm_gr_infoack_label-CCSM_E_OFFSET(r14),r4
        ldq     cor_label(g0),r8
        cmpobne r4,r8,.ccbg_1000
        cmpobne r5,r9,.ccbg_1000
        cmpobne r6,r10,.ccbg_1000
        cmpobne r7,r11,.ccbg_1000
#
# --- Values match. Clear disable copy info. update from NVRAM flag.
#
        ldob    cor_flags(g0),r4        # r4 = cor_flags byte
        clrbit  CFLG_B_DIS_GENERAL,r4,r4    # Clear flag
        stob    r4,cor_flags(g0)        # save update cor_flags byte
        b       .ccbg_1000              # and we're done!
#
.ccbg_70:
c       r15 = g1;                       # Length of CCBGram
c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx (CCBGram length=%ld ?)\n", FEBEMESSAGE,__FILE__, __LINE__, g1, r15);
.endif # M4_DEBUG_ILT
#++ c       memcpy((void *)g1, (void *)&ccsm_event_base1, 16);
#++ c check_local_memory_address(g1, sizeof(ILT_ALL_LEVELS));
        ldq     ccsm_event_base1,r4     # r4-r7 = base event "header"
        stq     r4,(g1)                 # save base event "header"
.ifdef M4_DEBUG_HARD
c fprintf(stderr, "copied: %08lx %08lx %08lx %08lx\n", r4, r5, r6, r7);
.endif # M4_DEBUG_HARD
#        st      r15,ccsm_e_len(g1)      # save event length
#++ c       memcpy((void *)(g1 + ccsm_e_len), (void *)r14, r15);
#++ c check_local_memory_address(g1, sizeof(ILT_ALL_LEVELS));
# -- This copies without regard for the length of the information -- boo hiss. 2008-12-10
!?      ldq     (r14),r4                # copy data into event ILT
        stq     r4,ccsm_e_len(g1)
.ifdef M4_DEBUG_HARD
c fprintf(stderr, "        %08lx %08lx %08lx %08lx\n", r4, r5, r6, r7);
.endif # M4_DEBUG_HARD
!?      ldq     16(r14),r8
        stq     r8,ccsm_e_len+16(g1)
.ifdef M4_DEBUG_HARD
c fprintf(stderr, "        %08lx %08lx %08lx %08lx\n", r8, r9, r10, r11);
.endif # M4_DEBUG_HARD
!?      ldq     32(r14),r4
        stq     r4,ccsm_e_len+32(g1)
.ifdef M4_DEBUG_HARD
c fprintf(stderr, "        %08lx %08lx %08lx %08lx\n", r4, r5, r6, r7);
.endif # M4_DEBUG_HARD
!?      ld      48(r14),r8
        st      r8,ccsm_e_len+48(g1)
.ifdef M4_DEBUG_HARD
c fprintf(stderr, "        %08lx\n", r8);
.endif # M4_DEBUG_HARD
.ifdef HISTORY_KEEP
c check_on_local_memory_address(g1, sizeof(ILT_ALL_LEVELS));
.endif  # HISTORY_KEEP
#
# --- Check for function codes that require special processing
#
        ldob    ccsm_e_fc(g1),r4        # r4 = request function code
.ifdef HISTORY_KEEP
c check_on_local_memory_address(g1, sizeof(ILT_ALL_LEVELS));
.endif  # HISTORY_KEEP
        cmpobne ccsm_gr_smdata,r4,.ccbg_900 # Jif not segment map data
c       g0 = get_sm();                  # Allocate a segment table to copy into
.ifdef M4_DEBUG_SM
c fprintf(stderr, "%s%s:%u get_sm 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif # M4_DEBUG_SM
.ifdef HISTORY_KEEP
c check_on_local_memory_address(g1, sizeof(ILT_ALL_LEVELS));
.endif  # HISTORY_KEEP
        st      g0,ccsm_gr_sm_map(g1)   # save segment map address in ILT
        lda     ccsm_gr_sm_map-ccsm_e_len(r14),r6  # r6 = incoming segment
                                        #                    map data
        lda     SM_tbl(g0),r7           # r7 = local segment map data address
        ldconst SMTBLsize/4,r5          # r5 = # words to copy
        ldconst 0,r4                    # r4 = copy index
.ccbg_100:
!       ld      (r6)[r4*4],r8
        st      r8,(r7)[r4*4]
        addo    1,r4,r4
        cmpobne r4,r5,.ccbg_100
        mov     g1,r4                   # save g1
        mov     g0,g1                   # g1 = segment table
        call    CM_cnt_smap             # count # segments set in table
        mov     r4,g1                   # restore g1
.ifdef HISTORY_KEEP
c check_on_local_memory_address(g1, sizeof(ILT_ALL_LEVELS));
.endif  # HISTORY_KEEP
.ccbg_900:
        bbc     6,r3,.ccbg_920          # Jif normal CCBGram message
        call    CCSM$queswap            # queue event to swap RAID CCSM
                                        #  event queue
        b       .ccbg_1000
#
.ccbg_920:
        call    CCSM$que                # queue event to CCSM event queue
.ccbg_1000:
        mov     r14,g0                  # restore g0
        mov     ecok,g1                 # g1 = MRP status code
        ldconst mDGrsiz,g2              # g2 = response length
        ret
#
#**********************************************************************
#
#  NAME: CCSM$update
#
#  PURPOSE:
#
#       This routine processes an update copy (% complete)
#       event for the client interface of CCSM.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$update
#
#  INPUT:
#
#       g1 = % complete
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       No regs. destroyed.
#
#**********************************************************************
#
CCSM$update:
        movq    g0,r12                  # save g0-g3
#
.if     ccsm_traces
#
        ld      ccsm_tr_flags,r4        # r4 = trace flags
        bbc     14,r4,.ciupdate_100     # Jif filter CIupdates invoked
        ld      ci_update,g0            # g0 = client I/F event type
        call    ccsm$ci_trace           # trace incoming event
.ciupdate_100:
#
.endif  # ccsm_traces
#
        ldob    cor_crstate(g3),r5      # r5 = current cor_crstate value
        ldconst corcrst_active,r3
        stob    r3,cor_crstate(g3)
        ldob    ccsm_op_state,r4        # r4 = CCSM op. state
        cmpobne ccsm_st_master,r4,.update_400 # Jif not the master CCSM
        cmpobe  r3,r5,.update_200       # Jif cor_crstate did not change
        call    CCSM$p2update           # schedule a P2 NVRAM update
        call    CM$mmc_sflag            # update VDD flags
.update_200:
        ld      cor_destvdd(g3),r11     # r11 = assoc. dest. VDD address
        cmpobe  0,r11,.update_1000      # Jif no dest. VDD defined
        stob    g1,vd_scpcomp(r11)      # save % complete in VDD
        ldconst vdcopyto,r4             # r4 = new VDD copy state
        stob    r4,vd_mirror(r11)       # save new VDD copy state
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        b       .update_1000
#
.update_400:
        ldconst 0,g0                    # g0 = update type code for message
        ldconst 0,g2
        call    CCSM$snd_update         # pack and send an Update message
.update_1000:
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: CCSM$upause
#
#  PURPOSE:
#
#       This routine processes a copy user paused
#       event for the client interface of CCSM.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$upause
#
#  INPUT:
#
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       No regs. destroyed.
#
#**********************************************************************
#
CCSM$upause:
        movq    g0,r12                  # save g0-g3
#
.if     ccsm_traces
#
        ld      ci_upause,g0            # g0 = client I/F event type
        call    ccsm$ci_trace           # trace incoming event
#
.endif  # ccsm_traces
#
# --- Check if cor_crstate already in user paused state and if not
#       set the cor_crstate to user paused and generate a call to
#       the CCSM$cs_chged routine to process this event.
#
        ldob    cor_crstate(g3),r4      # r4 = cor_crstate state
        cmpobe  corcrst_usersusp,r4,.upause_390 # Jif cor_crstate=user pause
        ldconst corcrst_usersusp,r4     # r4 = user paused state code
        stob    r4,cor_crstate(g3)      # set new state in COR
        call    CCSM$cs_chged           # generate copy state changed event
.upause_390:
        ldob    ccsm_op_state,r4        # r4 = CCSM op. state
        cmpobne ccsm_st_master,r4,.upause_400 # Jif not the master CCSM
        ld      cor_destvdd(g3),r11     # r11 = assoc. dest. VDD address
        cmpobe  0,r11,.upause_1000      # Jif no dest. VDD defined
        ldconst vdcopyuserpause,r4      # r4 = new VDD copy state
#       c fprintf(stderr, "<CCSM$upause>Setting userpause in vdmirror...\n");
        stob    r4,vd_mirror(r11)       # save new VDD copy state
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        call    CM$mmc_sflag            # update VDD flags
        b       .upause_1000
#
.upause_400:
        ldconst 0x02,g0                 # g0 = update type code for message
        ldconst 0,g1
        ldconst 0,g2
        call    CCSM$snd_update         # pack and send an Update message
.upause_1000:
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: CCSM$apause
#
#  PURPOSE:
#
#       This routine processes a copy auto-paused
#       event for the client interface of CCSM.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$apause
#
#  INPUT:
#
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       No regs. destroyed.
#
#**********************************************************************
#
CCSM$apause:
        movq    g0,r12                  # save g0-g3
#
.if     ccsm_traces
#
        ld      ci_apause,g0            # g0 = client I/F event type
        call    ccsm$ci_trace           # trace incoming event
#
.endif  # ccsm_traces
#
        ldconst corcrst_autosusp,r3     # r3 = pause type (auto suspended)
        stob    r3,cor_crstate(g3)
        ldob    ccsm_op_state,r4        # r4 = CCSM op. state
        cmpobne ccsm_st_master,r4,.apause_400 # Jif not the master CCSM
        ld      cor_destvdd(g3),r11     # r11 = assoc. dest. VDD address
        cmpobe  0,r11,.apause_1000      # Jif no dest. VDD defined
        ldconst vdcopyautopause,r4      # r4 = new VDD copy state
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>CCSM$apause-- setting autopause in vdmirror\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # CM_IM_DEBUG
        stob    r4,vd_mirror(r11)       # save new VDD copy state
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        call    CM$mmc_sflag            # update VDD flags
        b       .apause_1000
#
.apause_400:
        ldconst 0x04,g0                 # g0 = update type code for message
        ldconst 0,g1
        ldconst 0,g2
        call    CCSM$snd_update         # pack and send an Update message
.apause_1000:
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: CCSM$resume
#
#  PURPOSE:
#
#       This routine processes a copy has resumed
#       event for the client interface of CCSM.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$resume
#
#  INPUT:
#
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       No regs. destroyed.
#
#**********************************************************************
#
CCSM$resume:
        movq    g0,r12                  # save g0-g3
#
.if     ccsm_traces
#
        ld      ci_resume,g0            # g0 = client I/F event type
        call    ccsm$ci_trace           # trace incoming event
#
.endif  # ccsm_traces
#
        ldconst corcrst_active,r3
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>CCSM_resume- setting crstate as active (2) and vdmirror to copyTostate\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # CM_IM_DEBUG
        stob    r3,cor_crstate(g3)
        ldob    ccsm_op_state,r4        # r4 = CCSM op. state
        cmpobne ccsm_st_master,r4,.resume_400 # Jif not the master CCSM
        ld      cor_destvdd(g3),r11     # r11 = assoc. dest. VDD address
        cmpobe  0,r11,.resume_1000      # Jif no dest. VDD defined
        ldconst vdcopyto,r4             # r4 = new VDD copy state
        stob    r4,vd_mirror(r11)       # save new VDD copy state
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        call    CM$mmc_sflag            # update VDD flags
        b       .resume_1000
#
.resume_400:
        ldconst 0x03,g0                 # g0 = update type code for message
        ldconst 0,g1
        ldconst 0,g2
        call    CCSM$snd_update         # pack and send an Update message
.resume_1000:
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: CCSM$mirror
#
#  PURPOSE:
#
#       This routine processes a copy mirrored
#       event for the client interface of CCSM.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$mirror
#
#  INPUT:
#
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       No regs. destroyed.
#
#**********************************************************************
#
CCSM$mirror:
        movq    g0,r12                  # save g0-g3
#
.if     ccsm_traces
#
        ld      ci_mirror,g0            # g0 = client I/F event type
        call    ccsm$ci_trace           # trace incoming event
#
.endif  # ccsm_traces
#
        ldconst corcrst_active,r3
        stob    r3,cor_crstate(g3)
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>CCSM_mirror- setting crstate as active (2) vdmirror to copymirror\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # CM_IM_DEBUG
        ldob    ccsm_op_state,r4        # r4 = CCSM op. state
        cmpobne ccsm_st_master,r4,.mirror_400 # Jif not the master CCSM
        ld      cor_destvdd(g3),r11     # r11 = assoc. dest. VDD address
        cmpobe  0,r11,.mirror_200       # Jif no dest. VDD defined
        ldconst vdcopymirror,r4         # r4 = new VDD copy state
        stob    r4,vd_mirror(r11)       # save new VDD copy state
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        call    CM$mmc_sflag            # update VDD flags
.if GR_GEORAID15_DEBUG
        ldos    vd_vid(r11),r5          # r5 = destination VID
c fprintf(stderr,"%s%s:%u <GR><CCSM$mirror>set vd_mirror to sync updated vdflags dvid=%lx\n", FEBEMESSAGE, __FILE__, __LINE__,r5);
.endif  # GR_GEORAID15_DEBUG
.mirror_200:
        ld      cor_cm(g3),r5           # r5 = assoc. CM address
        cmpobe  0,r5,.mirror_1000      # Jif no CM assoc. with COR
        ldob    cm_type(r5),r6          # r6 = copy type code
        cmpobe  cmty_mirror,r6,.mirror_1000 # Jif mirror type copy
        cmpobne cmty_copybreak,r6,.mirror_220 # Jif not copy & break type
#
# --- Copy & break type copy just got mirrored
#
        call    CCSM$term_copy          # start the terminate copy
        b       .mirror_1000
#
.mirror_220:
        cmpobne cmty_copyswap,r6,.mirror_240 # Jif not copy/swap/break type
#
# --- Copy & swap & break type copy just got mirrored
#
        ldconst 1,g0                    # g0 = swap RAIDs type code
        b       .mirror_280
#
.mirror_240:
        cmpobne cmty_mirrorswap,r6,.mirror_1000 # Jif not copy/swap/mirror
#
# --- Copy & swap & mirror type copy just got mirrored
#
        ldconst 0,g0                    # g0 = swap RAIDs type code
.mirror_280:
        call    CCSM$swap_raids         # initiate a swap RAIDs request
        b       .mirror_1000
#
.mirror_400:
        ldconst 0x01,g0                 # g0 = update type code for message
        ldconst 0,g1
        ldconst 0,g2
        call    CCSM$snd_update         # pack and send an Update message
.mirror_1000:
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: CCSM$ended
#
#  PURPOSE:
#
#       This routine processes a copy ended
#       event for the client interface of CCSM.
#
#  DESCRIPTION:
#
#       The client interface copy ended event refers the case where
#       a client (CM task) has determined that the copy needs to be
#       terminated. This routine is called when this occurs to perform
#       the necessary operations to terminate the copy through the
#       master CCSM.
#
#  CALLING SEQUENCE:
#
#       call    CCSM$ended
#
#  INPUT:
#
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       No regs. destroyed.
#
#**********************************************************************
#
CCSM_Copy_ended:
CCSM$ended:
        mov     g0,r15                  # save g0
        mov     g4,r14                  # save g4
#
.if     ccsm_traces
#
        ld      ci_ended,g0             # g0 = client I/F event type
        call    ccsm$ci_trace           # trace incoming event
#
.endif  # ccsm_traces
#
        ldconst 0,g4                    # g4 = 0 to signify sending to master
                                        #      CCSM
        ldconst 0x02,g0                 # g0 = terminate copy process code
        call    CCSM$snd_process        # pack and send a Process
                                        #  message to this sender
                                        #  indicating to terminate the copy
        mov     r14,g4                  # restore g4
        mov     r15,g0                  # restore g0
        ret
#
#**********************************************************************
#
#  NAME: CCSM$reg
#
#  PURPOSE:
#
#       This routine processes a copy registered
#       event for the client interface of CCSM.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$reg
#
#  INPUT:
#
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       No regs. destroyed.
#
#**********************************************************************
#
CCSM$reg:
        mov     g0,r15                  # save g0
#
.if     ccsm_traces
#
        ld      ci_reg,g0               # g0 = client I/F event type
        call    ccsm$ci_trace           # trace incoming event
#
.endif  # ccsm_traces
        ldob    cor_crstate(g3),r5      # r5 = current registration state
        ldconst corcrst_active,r4       # r4 = registration active
        cmpoble r4,r5,.reg_1000         # Jif copy registration already done
        stob    r4,cor_crstate(g3)      # save new registration state
        call    CM$mmc_sflag            # update VDD flags

        ldob    ccsm_op_state,r4        # r4 = CCSM op. state
        cmpobne ccsm_st_master,r4,.reg_400 # Jif not the master CCSM
#
# --- Master CCSM
#
        call    D$p2updateconfig        # save P2 NVRAM
        b       .reg_1000
#
# --- Not the master CCSM
#
.reg_400:
        call    CCSM$snd_copyreg        # pack and send copy registration
                                        #  complete message to master
.reg_1000:
        mov     r15,g0                  # restore g0
        ret
#
#**********************************************************************
#
#  NAME: CCSM$reg_sync
#
#  PURPOSE:
#
#       This routine processes a region synchronized
#       event for the client interface of CCSM.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$reg_sync
#
#  INPUT:
#
#       g0 = Region # that is now synchronized
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       No regs. destroyed.
#
#**********************************************************************
#
CCSM$reg_sync:
        mov     g0,r15                  # save g0
#
.if     ccsm_traces
#
        ld      ci_reg_sync,g0          # g0 = client I/F event type
        call    ccsm$ci_trace           # trace incoming event
#
.endif  # ccsm_traces
#
.if GR_GEORAID15_DEBUG
        ldob    cor_rcsvd(g3),r5
        ldob    cor_rcdvd(g3),r4
c fprintf(stderr, "%s%s:%u <GR><CCSM$reg_sync>Calling P6_ClrMainRM(svid=%lx dvid=%lx)\n", FEBEMESSAGE, __FILE__, __LINE__,r5,r6);
.endif  # GR_GEORAID15_DEBUG
        mov     r15,g0                  # g0 = region #
        PushRegs(r3)                    # Save all "g" registers
        call    P6_ClrMainRM            # clear the region # bit in the main
                                        #  region map bitmap in state NVRAM
        PopRegsVoid(r3)                 # restore environment
        mov     r15,g0                  # restore g0
        ret
#
#**********************************************************************
#
#  NAME: CCSM$reg_dirty
#
#  PURPOSE:
#
#       This routine processes a region dirty
#       event for the client interface of CCSM.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$reg_dirty
#
#  INPUT:
#
#       g0 = Region # that is now dirty
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       No regs. destroyed.
#
#**********************************************************************
#
CCSM$reg_dirty:
        mov     g0,r15                  # save g0
#
.if     ccsm_traces
#
        ld      ci_reg_dirty,g0         # g0 = client I/F event type
        call    ccsm$ci_trace           # trace incoming event
#
.endif  # ccsm_traces
#
.if GR_GEORAID15_DEBUG
        ldob    cor_rcsvd(g3),r5
        ldob    cor_rcdvd(g3),r4
c fprintf(stderr, "%s%s:%u <GR><CCSM$reg_dirty>Calling P6_SetMainRM(svid=%lx dvid=%lx)\n", FEBEMESSAGE, __FILE__, __LINE__,r5,r6);
.endif  # GR_GEORAID15_DEBUG
        mov     r15,g0                  # g0 = region #
        PushRegs(r3)                    # Save all "g" registers
        call    P6_SetMainRM            # set the region # bit in the main
                                        #  region map bitmap in state NVRAM
        PopRegsVoid(r3)                 # restore environment
        mov     r15,g0                  # restore g0
        ret
#
#**********************************************************************
#
#  NAME: CCSM$get_cwip
#
#  PURPOSE:
#
#       This routine processes a get CWIP record
#       event for the client interface of CCSM.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$get_cwip
#
#  INPUT:
#
#       None.
#
#  OUTPUT:
#
#       g0 = CWIP record counter address
#
#  REGS DESTROYED:
#
#       Reg. g0 destroyed.
#
#**********************************************************************
#
CCSM$get_cwip:
        ldconst ccsm_cwip_cnt,g0        # g0 = CWIP record count address
        ld      (g0),r4                 # r4 = CWIP record count
        addo    1,r4,r4                 # inc. CWIP record count
        st      r4,(g0)                 # save updated CWIP record count
        cmpobne 1,r4,.getcwip_1000      # Jif no need to corrupt P6 header
.ifndef LINUX_VER_NVP6_MM
        ldob    NVSRAMP6START,r4        # r4 = 1st byte of P6 header banner
        setbit  5,r4,r4
        stob    r4,NVSRAMP6START
.else   # LINUX_VER_NVP6_MM
        call    p6_Set_CopyWorkInProcess_Bit
.endif # LINUX_VER_NVP6_M
.getcwip_1000:
        ret
#
#**********************************************************************
#
#  NAME: CCSM$put_cwip
#
#  PURPOSE:
#
#       This routine processes a put CWIP record
#       event for the client interface of CCSM.
#
#  DESCRIPTION:
#
#       Note: TEMPORARY approach for phase 1
#
#       This routine checks the address passed by the calling routine
#       for a valid address and if so decrements the outstanding write
#       update count. If this count goes to 0, it clears the Part 6
#       NVRAM flag indicating outstanding write updates.
#
#       END TEMPORARY
#
#  CALLING SEQUENCE:
#
#       call    CCSM$put_cwip
#
#  INPUT:
#
#       g0 = CWIP record address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       No regs. destroyed.
#
#**********************************************************************
#
CCSM$put_cwip:
        ldconst ccsm_cwip_cnt,r4        # r4 = expected CWIP record counter
        cmpobe  g0,r4,.putcwip_200      # Jif expected CWIP address passed
        ld      ccsm_bad_cwip,r5        # r5 = bad CWIP address count
        addo    1,r5,r5                 # inc. bad CWIP address count
        st      r5,ccsm_bad_cwip        # save updated count
        b       .putcwip_1000
#
.putcwip_200:
        ld      (g0),r5                 # r5 = current CWIP count
        cmpobe  0,r5,.putcwip_1000      # Jif count=0
        subo    1,r5,r5                 # dec. CWIP count
        st      r5,(g0)                 # save updated count
        cmpobne 0,r5,.putcwip_1000      # Jif outstanding CWIP count<>0
#
.ifndef LINUX_VER_NVP6_MM
        ldob    NVSRAMP6START,r4        # r4 = 1st byte of P6 header banner
        clrbit  5,r4,r4
        stob    r4,NVSRAMP6START
.else   # LINUX_VER_NVP6_MM
        call    p6_Clear_CopyWorkInProcess_Bit
.endif # LINUX_VER_NVP6_MM
.putcwip_1000:
        ret
#
#**********************************************************************
#
#  NAME: CCSM$cd_moved
#
#  PURPOSE:
#
#       This routine processes a copy devices moved
#       event for the client interface of CCSM.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$cd_moved
#
#  INPUT:
#
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       No regs. destroyed.
#
#**********************************************************************
#
CCSM$cd_moved:
        mov     g0,r15                  # save g0
#
.if     ccsm_traces
#
        ld      ci_cd_moved,g0          # g0 = client I/F event type
        call    ccsm$ci_trace           # trace incoming event
#
.endif  # ccsm_traces
#
        ldob    ccsm_op_state,r4        # r4 = CCSM op. state
        cmpobne ccsm_st_master,r4,.cdm_400 # Jif not the master CCSM
#
# --- Master CCSM handler routine
#
        call    CCSM$p2update           # schedule timed update
        b       .cdm_1000
#
# --- Non-master CCSM handler routine
#
.cdm_400:
        ldob    cor_flags(g3),r4        # r4 = cor_flags byte
        setbit  CFLG_B_DIS_DEVICE,r4,r4 # set disable copy device update
                                        #  from NVRAM flag
        stob    r4,cor_flags(g3)        # save updated cor_flags byte
        call    CCSM$snd_cdmoved        # pack and send Copy Devices Moved
                                        #  message to master CCSM
.cdm_1000:
        mov     r15,g0                  # restore g0
        ret
#
#**********************************************************************
#
#  NAME: CCSM$cs_chged_w_msg
#  NAME: CCSM$cs_chged
#
#  PURPOSE:
#
#       This routine processes a copy state changed
#       event for the client interface of CCSM. CCSM$cs_chged_w_msg
#       will also generate a message to the CCB indicating the state
#       of the copy.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$cs_chged_w_msg
#       call    CCSM$cs_chged
#
#  INPUT:
#
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       No regs. destroyed.
#
#**********************************************************************
#
CCSM$cs_chged_w_msg:
        mov     g0,r15                  # save g0
#
        ldob    cor_crstate(g3),r4      # r5 = cor_crstate value
#
# --- Log Copy Resumed Message to CCB
#
        ldconst cmcc_CpyRsm,g0          # set copy resumed message
        cmpobg  corcrst_usersusp,r4,.cscwmsg_700 # Jif copy not paused
#
# --- Log Copy Paused Message to CCB
#
        ldconst cmcc_UsrSpnd,g0         # g0 = set User Suspend message
#
# --- issue Log Message to CCB
#
.cscwmsg_700:
        call    CM_Log_Completion       # send log message

        mov     r15,g0                  # restore g0
#
CCSM$cs_chged:
        mov     g0,r15                  # save g0
#
.if     ccsm_traces
#
        ld      ci_cs_chged,g0          # g0 = client I/F event type
        call    ccsm$ci_trace           # trace incoming event
#
.endif  # ccsm_traces
#
        ldob    ccsm_op_state,r4        # r4 = CCSM op. state
        cmpobne ccsm_st_master,r4,.csc_400 # Jif not the master CCSM
#
# --- Master CCSM handler routine
#
        call    CCSM$p2update           # schedule timed update
        b       .csc_1000
#
# --- Non-master CCSM handler routine
#
.csc_400:
        ldob    cor_flags(g3),r4        # r4 = cor_flags byte
        setbit  CFLG_B_DIS_STATE,r4,r4  # Set disable copy state update
                                        #  from NVRAM flag
        stob    r4,cor_flags(g3)        # save updated cor_flags byte
        call    CCSM$snd_csc            # pack and send Copy State Changed
                                        #  message to master CCSM
.csc_1000:
        mov     r15,g0                  # restore g0
        ret
#
#**********************************************************************
#
#  NAME: CCSM$info_chged
#
#  PURPOSE:
#
#       This routine processes a copy general information changed
#       event for the client interface of CCSM.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$info_chged
#
#  INPUT:
#
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       No regs. destroyed.
#
#**********************************************************************
#
CCSM$info_chged:
        mov     g0,r15                  # save g0
#
.if     ccsm_traces
#
        ld      ci_info_chged,g0        # g0 = client I/F event type
        call    ccsm$ci_trace           # trace incoming event
#
.endif  # ccsm_traces
#
        ldob    ccsm_op_state,r4        # r4 = CCSM op. state
        cmpobne ccsm_st_master,r4,.info_400 # Jif not the master CCSM
#
# --- Master CCSM handler routine
#
        call    CCSM$p2update           # schedule timed update
        b       .info_1000
#
# --- Non-master CCSM handler routine
#
.info_400:
        ldob    cor_flags(g3),r4        # r4 = cor_flags byte
        setbit  CFLG_B_DIS_GENERAL,r4,r4    # Set disable copy general info.
                                        #  update from NVRAM flag
        stob    r4,cor_flags(g3)        # save updated cor_flags byte
        call    CCSM$snd_info           # pack and send Copy General Info.
                                        #  Changed message to master CCSM
.info_1000:
        mov     r15,g0                  # restore g0
        ret
#
#**********************************************************************
#
#  NAME: CCSM$rpoll_term
#
#  PURPOSE:
#
#       This routine services a terminate copy event (client I/F)
#       generated from a remote COR poll operation.
#
#  DESCRIPTION:
#
#       This routine determines if a remote copy COR should be
#       terminated by this node or not based on if this node
#       is the current CCSM master or not. Remote COR poll
#       operations that determine that a remote copy COR should
#       be terminated will only terminate the copy from the master
#       CCSM node.
#
#  CALLING SEQUENCE:
#
#       call    CCSM$rpoll_term
#
#  INPUT:
#
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       No regs. destroyed.
#
#**********************************************************************
#
CCSM$rpoll_term:
        mov     g0,r15                  # save g0
#
.if     ccsm_traces
#
        ld      ci_rpoll_term,g0        # g0 = client I/F event type
        call    ccsm$ci_trace           # trace incoming event
#
.endif  # ccsm_traces
#
        ldob    ccsm_op_state,r4        # r4 = CCSM op. state
        cmpobne ccsm_st_master,r4,.rpollterm_1000 # Jif not the master CCSM
#
# --- Master CCSM handler routine
#
        call    CCSM$term_copy          # generate a terminate copy event
                                        #  to the CCSM task
.rpollterm_1000:
        mov     r15,g0                  # restore g0
        ret
#
#       CCSM$rpoll_term - Remote COR poll terminate copy (client I/F)
#
# --- CCSM general subroutines ----------------------------------------
#
#
#**********************************************************************
#
#  NAME: CCSM$snd_copyreg
#
#  PURPOSE:
#
#       This routine packs and sends a Copy Registered message to
#       the master CCSM.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$snd_copyreg
#
#  INPUT:
#
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       No regs. destroyed.
#
#**********************************************************************
#
        .data                           # data area
        .align  2                       # align just in case
#
snd_copyreg_start:
        .word   ccsm_gr_reg_size-CCSM_E_OFFSET # message length
        .byte   ccsm_et_ccbg            # event type code
        .byte   ccsm_gr_reg             # event function code
        .short  0                       # sequence #
snd_copyreg_sn:
        .word   0
snd_copyreg_rid:
        .word   0                       # copy registration ID
snd_copyreg_cmsn:
        .word   0                       # copy CM serial #
#
        .text

CCSM$snd_copyreg:
        movq    g0,r12                  # save g0-g3
        mov     g4,r11                  # save g4
        ld      cor_rid(g3),r4          # r4 = copy reg. ID
        ld      cor_rcsn(g3),r5         # r5 = copy CM serial #
        stl     r4,snd_copyreg_rid
#
# --- send the request
#
        lda     snd_copyreg_start,g0    # g0 = ptr to packet
        ldconst ccsm_gr_reg_size-CCSM_E_OFFSET,g1  # g1 = packet size
        ldconst ebiseDG,g2              # g2 = event type
        ldconst ebibtmaster,g3          # g3 = send to master only
        ldconst 0,g4                    # g4 = serial # of controller (N/A)
#
.if     ccsm_traces
#
        call    ccsm$trc_sendCCB        # trace sending CCBGram message
#
.endif  # ccsm_traces
#
        call    D$reportEvent           # send packet

        mov     r11,g4                  # restore g4
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: CCSM$snd_process
#
#  PURPOSE:
#
#       This routine packs and sends a Process message to
#       the specified CCSM.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$snd_process
#
#  INPUT:
#
#       g0 = process code to send
#       g3 = assoc. COR address
#       g4 = serial # of controller to send to
#       g4 = 0 if sending to master CCSM
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       No regs. destroyed.
#
#**********************************************************************
#
        .data                           # data area
        .align  2                       # align just in case
#
snd_process_start:
        .word   ccsm_gr_process_size-CCSM_E_OFFSET # message length
        .byte   ccsm_et_ccbg            # event type code
        .byte   ccsm_gr_process         # event function code
        .short  0                       # sequence #
snd_process_sn:
        .word   0
snd_process_rid:
        .word   0                       # copy registration ID
snd_process_cmsn:
        .word   0                       # copy CM serial #
snd_process_code:
        .byte   0                       # process code
        .byte   0                       # unused
        .byte   0                       # unused
        .byte   0                       # unused
#
        .text

CCSM$snd_process:
        movq    g0,r12                  # save g0-g3
        mov     g4,r11                  # save g4
        ld      cor_rid(g3),r4          # r4 = copy reg. ID
        ld      cor_rcsn(g3),r5         # r5 = copy CM serial #
        stl     r4,snd_process_rid
        stob    r12,snd_process_code
#
# --- send the request
#
        lda     snd_process_start,g0    # g0 = ptr to packet
        ldconst ccsm_gr_process_size-CCSM_E_OFFSET,g1 # g1 = packet size
        ldconst ebiseDG,g2              # g2 = event type
        ldconst ebibtmaster,g3          # g3 = send to master only
        cmpobe  0,g4,.sndproc_400       # Jif master CCSM specified
        ldconst ebibtspec,g3            # g3 = send to specified controller
                                        # g4 = serial # of controller
.sndproc_400:
#
.if     ccsm_traces
#
        call    ccsm$trc_sendCCB        # trace sending CCBGram message
#
.endif  # ccsm_traces
#
        call    D$reportEvent           # send packet

        mov     r11,g4                  # restore g4
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: CCSM$snd_process2
#
#  PURPOSE:
#
#       This routine packs and sends a Process message to
#       the specified CCSM.
#
#       Note: This routine differs from CCSM$snd_process in that this
#               routine does not require a COR address to be specified.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$snd_process2
#
#  INPUT:
#
#       g0 = process code to send
#       g1 = copy registration ID
#       g2 = CM task serial # of copy
#       g4 = serial # of controller to send to
#       g4 = 0 if sending to master CCSM
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       No regs. destroyed.
#
#**********************************************************************
#
CCSM$snd_process2:
        movq    g0,r12                  # save g0-g3
        mov     g4,r11                  # save g4
        st      g1,snd_process_rid
        st      g2,snd_process_cmsn
        stob    r12,snd_process_code
#
# --- send the request
#
        lda     snd_process_start,g0    # g0 = ptr to packet
        ldconst ccsm_gr_process_size-CCSM_E_OFFSET,g1 # g1 = packet size
        ldconst ebiseDG,g2              # g2 = event type
        ldconst ebibtmaster,g3          # g3 = send to master only
        cmpobe  0,g4,.sndproc2_400      # Jif master CCSM specified
        ldconst ebibtspec,g3            # g3 = send to specified controller
                                        # g4 = serial # of controller
.sndproc2_400:
#
.if     ccsm_traces
#
        call    ccsm$trc_sendCCB        # trace sending CCBGram message
#
.endif  # ccsm_traces
#
        call    D$reportEvent           # send packet

        mov     r11,g4                  # restore g4
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: CCSM$snd_update
#
#  PURPOSE:
#
#       This routine packs and sends an Update message to
#       the master CCSM.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$snd_update
#
#  INPUT:
#
#       g0 = update type code to send
#       g1 = g0 register value to send
#       g2 = g1 register value to send
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       No regs. destroyed.
#
#**********************************************************************
#
        .data                           # data area
        .align  2                       # align just in case
#
snd_update_start:
        .word   ccsm_gr_update_size-CCSM_E_OFFSET # message length
        .byte   ccsm_et_ccbg            # event type code
        .byte   ccsm_gr_update          # event function code
        .short  0                       # sequence #
snd_update_sn:
        .word   0
snd_update_rid:
        .word   0                       # copy registration ID
snd_update_cmsn:
        .word   0                       # copy CM serial #
snd_update_code:
        .byte   0                       # update type code
        .byte   0                       # unused
        .byte   0                       # unused
        .byte   0                       # unused
snd_update_g0:
        .word   0                       # g0 register
snd_update_g1:
        .word   0                       # g1 register
#
        .text

CCSM$snd_update:
        movq    g0,r12                  # save g0-g3
        mov     g4,r11                  # save g4
        ld      cor_rid(g3),r4          # r4 = copy reg. ID
        ld      cor_rcsn(g3),r5         # r5 = copy CM serial #
        stl     r4,snd_update_rid
        stt     r12,snd_update_code
#
# --- send the request
#
        lda     snd_update_start,g0     # g0 = ptr to packet
        ldconst ccsm_gr_update_size-CCSM_E_OFFSET,g1 # g1 = packet size
        ldconst ebiseDG,g2              # g2 = event type
        ldconst ebibtmaster,g3          # g3 = send to master only
        ldconst 0,g4                    # g4 = serial # of controller (N/A)
#
.if     ccsm_traces
#
#        call    ccsm$trc_sendCCB        # trace sending CCBGram message
#
.endif  # ccsm_traces
#
        call    D$reportEvent           # send packet

        mov     r11,g4                  # restore g4
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: CCSM$snd_force
#
#  PURPOSE:
#
#       This routine packs and sends a Force Ownership Change message to
#       the master CCSM.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$snd_force
#
#  INPUT:
#
#       g1 = new secondary owner
#       g2 = message sequence #
#       g3 = assoc. COR address
#
#       Note: This routine assumes that this node is to be defined
#               as the new primary owner.
#             This routine assumes that the previous primary owner
#               is specified in cor_transcpo.
#             This routine assumes that the previous secondary owner
#               is specified in cor_transcso.
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       No regs. destroyed.
#
#**********************************************************************
#
        .data                           # data area
        .align  2                       # align just in case
#
snd_force_start:
        .word   ccsm_gr_force_size-CCSM_E_OFFSET # message length
        .byte   ccsm_et_ccbg            # event type code
        .byte   ccsm_gr_force           # event function code
snd_force_seq:
        .short  0                       # sequence #
snd_force_sn:
        .word   0
snd_force_rid:
        .word   0                       # copy registration ID
snd_force_cmsn:
        .word   0                       # copy CM serial #
snd_force_ppo:
        .word   0                       # previous pri. owner
snd_force_pso:
        .word   0                       # previous sec. owner
snd_force_npo:
        .word   0                       # new pri. owner
snd_force_nso:
        .word   0                       # new sec. owner
#
        .text

CCSM$snd_force:
        movq    g0,r12                  # save g0-g3
        mov     g4,r11                  # save g4
        ld      cor_rid(g3),r4          # r4 = copy reg. ID
        ld      cor_rcsn(g3),r5         # r5 = copy CM serial #
        ld      cor_transcpo(g3),r6     # r6 = previous pri. owner
        ld      cor_transcso(g3),r7     # r7 = previous sec. owner
        stl     r4,snd_force_rid
        stl     r6,snd_force_ppo
        st      r13,snd_force_nso
        ld      K_ficb,r3               # r3 = FICB address
        ld      fi_cserial(r3),r5       # r5 = controller serial #
        st      r5,snd_force_npo
        st      r5,cor_PMPsn(g3)
        st      g1,cor_SMPsn(g3)
        stos    g2,snd_force_seq
#
# --- send the request
#
        lda     snd_force_start,g0         # g0 = ptr to packet
        ldconst ccsm_gr_force_size-CCSM_E_OFFSET,g1 # g1 = packet size
        ldconst ebiseDG,g2              # g2 = event type
        ldconst ebibtmaster,g3          # g3 = send to master only
        ldconst 0,g4                    # g4 = serial # of controller (N/A)
#
.if     ccsm_traces
#
        call    ccsm$trc_sendCCB        # trace sending CCBGram message
#
.endif  # ccsm_traces
#
        call    D$reportEvent           # send packet

        mov     r11,g4                  # restore g4
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: CCSM$snd_define
#
#  PURPOSE:
#
#       This routine packs and sends a Define Ownership message to
#       the master CCSM.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$snd_define
#
#  INPUT:
#
#       g1 = new secondary owner
#       g2 = sequence #
#       g3 = assoc. COR address
#
#       Note: This routine assumes that this node is to be defined
#               as the new primary owner
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       No regs. destroyed.
#
#**********************************************************************
#
        .data                           # data area
        .align  2                       # align just in case
#
snd_define_start:
        .word   ccsm_gr_define_size-CCSM_E_OFFSET # message length
        .byte   ccsm_et_ccbg            # event type code
        .byte   ccsm_gr_define          # event function code
snd_define_seq:
        .short  0                       # sequence #
snd_define_sn:
        .word   0
snd_define_rid:
        .word   0                       # copy registration ID
snd_define_cmsn:
        .word   0                       # copy CM serial #
snd_define_npo:
        .word   0                       # new pri. owner
snd_define_nso:
        .word   0                       # new sec. owner
#
        .text

CCSM$snd_define:
        movq    g0,r12                  # save g0-g3
        mov     g4,r11                  # save g4
        ld      cor_rid(g3),r4          # r4 = copy reg. ID
        ld      cor_rcsn(g3),r5         # r5 = copy CM serial #
        stl     r4,snd_define_rid
        st      r13,snd_define_nso
        ld      K_ficb,r3               # r3 = FICB address
        ld      fi_cserial(r3),r5       # r5 = controller serial #
        st      r5,snd_define_npo
        st      r5,cor_PMPsn(g3)
        st      g1,cor_SMPsn(g3)
        stos    r14,snd_define_seq
#
# --- send the request
#
        lda     snd_define_start,g0     # g0 = ptr to packet
        ldconst ccsm_gr_define_size-CCSM_E_OFFSET,g1 # g1 = packet size
        ldconst ebiseDG,g2              # g2 = event type
        ldconst ebibtmaster,g3          # g3 = send to master only
        ldconst 0,g4                    # g4 = serial # of controller (N/A)
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>CCSM$snd_define-- sending define ownership msg to master CCSM\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # CM_IM_DEBUG
#
.if     ccsm_traces
#
        call    ccsm$trc_sendCCB        # trace sending CCBGram message
#
.endif  # ccsm_traces
#
        call    D$reportEvent           # send packet

        mov     r11,g4                  # restore g4
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: CCSM$snd_owner
#
#  PURPOSE:
#
#       This routine packs and sends a You Are Owner message to
#       the CCSM residing on the new primary owner's controller.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$snd_owner
#
#  INPUT:
#
#       g0 = primary owner
#       g1 = secondary owner
#       g2 = sequence #
#       g3 = assoc. COR address
#
#       Note: This routine assumes the message is to be sent to the
#               primary owner's controller.
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       No regs. destroyed.
#
#**********************************************************************
#
        .data                           # data area
        .align  2                       # align just in case
#
snd_owner_start:
        .word   ccsm_gr_owner_size-CCSM_E_OFFSET # message length
        .byte   ccsm_et_ccbg            # event type code
        .byte   ccsm_gr_owner           # event function code
snd_owner_seq:
        .short  0                       # sequence #
snd_owner_sn:
        .word   0
snd_owner_rid:
        .word   0                       # copy registration ID
snd_owner_cmsn:
        .word   0                       # copy CM serial #
snd_owner_po:
        .word   0                       # primary owner
snd_owner_so:
        .word   0                       # secondary owner
snd_owner_crstate:
        .byte   0                       # cor_crstate value
        .byte   0                       # unused
        .byte   0                       # unused
        .byte   0                       # unused
#
        .text

CCSM$snd_owner:
        movq    g0,r12                  # save g0-g3
        mov     g4,r11                  # save g4
        ld      cor_rid(g3),r4          # r4 = copy reg. ID
        ld      cor_rcsn(g3),r5         # r5 = copy CM serial #
        stl     r4,snd_owner_rid
        stl     r12,snd_owner_po
        ldob    cor_crstate(g3),r3      # r3 = current cor_crstate value
        stos    r14,snd_owner_seq
        stob    r3,snd_owner_crstate
#
# --- send the request
#
        lda     snd_owner_start,g0      # g0 = ptr to packet
        ldconst ccsm_gr_owner_size-CCSM_E_OFFSET,g1 # g1 = packet size
        ldconst ebiseDG,g2              # g2 = event type
        ldconst ebibtspec,g3            # g3 = send to specified controller
        mov     r12,g4                  # g4 = serial # of controller
#
.if     ccsm_traces
#
        call    ccsm$trc_sendCCB        # trace sending CCBGram message
#
.endif  # ccsm_traces
.if GR_GEORAID15_DEBUG
c fprintf(stderr,"%s%s:%u <GR><ccsm$snd_owner>sending you are owner msg CCBGram to CCSM on primary owner's DCN\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # GR_GEORAID15_DEBUG
#
        call    D$reportEvent           # send packet

        mov     r11,g4                  # restore g4
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: CCSM$snd_trans
#
#  PURPOSE:
#
#       This routine packs and sends a Transfer Ownership message to
#       the CCSM residing on the specified controller.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$snd_trans
#
#  INPUT:
#
#       g2 = sequence #
#       g3 = assoc. COR address
#       g4 = serial # of controller to send to
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       No regs. destroyed.
#
#**********************************************************************
#
        .data                           # data area
        .align  2                       # align just in case
#
snd_trans_start:
        .word   ccsm_gr_trans_size-CCSM_E_OFFSET # message length
        .byte   ccsm_et_ccbg            # event type code
        .byte   ccsm_gr_trans           # event function code
snd_trans_seq:
        .short  0                       # sequence #
snd_trans_sn:
        .word   0
snd_trans_rid:
        .word   0                       # copy registration ID
snd_trans_cmsn:
        .word   0                       # copy CM serial #
snd_trans_cpo:
        .word   0                       # current primary owner
snd_trans_cso:
        .word   0                       # current secondary owner
#
        .text

CCSM$snd_trans:
        movq    g0,r12                  # save g0-g3
        mov     g4,r11                  # save g4
        ld      cor_rid(g3),r4          # r4 = copy reg. ID
        ld      cor_rcsn(g3),r5         # r5 = copy CM serial #
        ld      cor_powner(g3),r8       # r8 = current pri. owner
        ld      cor_sowner(g3),r9       # r9 = current sec. owner
        stl     r4,snd_trans_rid
        stl     r8,snd_trans_cpo
        stos    r14,snd_trans_seq
#
# --- send the request
#
        lda     snd_trans_start,g0         # g0 = ptr to packet
        ldconst ccsm_gr_trans_size-CCSM_E_OFFSET,g1 # g1 = packet size
        ldconst ebiseDG,g2              # g2 = event type
        ldconst ebibtspec,g3            # g3 = send to specified controller
                                        # g4 = serial # of controller
#
.if     ccsm_traces
#
        call    ccsm$trc_sendCCB        # trace sending CCBGram message
#
.endif  # ccsm_traces
#
        call    D$reportEvent           # send packet

        mov     r11,g4                  # restore g4
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: CCSM$snd_change
#
#  PURPOSE:
#
#       This routine packs and sends a Change Ownership message to
#       the master CCSM.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$snd_change
#
#  INPUT:
#
#       g2 = sequence #
#       g3 = assoc. COR address
#       g4 = new primary owner
#       g5 = new secondary owner
#
#       Note: This routine assumes that the current primary owner is
#               specified in the cor_PMPsn field of the COR and the
#               current secondary owner is specified in the cor_SMPsn
#               field of the COR.
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       No regs. destroyed.
#
#**********************************************************************
#
        .data                           # data area
        .align  2                       # align just in case
#
snd_change_start:
        .word   ccsm_gr_change_size-CCSM_E_OFFSET # message length
        .byte   ccsm_et_ccbg            # event type code
        .byte   ccsm_gr_change          # event function code
snd_change_seq:
        .short  0                       # sequence #
snd_change_sn:
        .word   0
snd_change_rid:
        .word   0                       # copy registration ID
snd_change_cmsn:
        .word   0                       # copy CM serial #
snd_change_cpo:
        .word   0                       # current primary owner
snd_change_cso:
        .word   0                       # current secondary owner
snd_change_npo:
        .word   0                       # new primary owner
snd_change_nso:
        .word   0                       # new secondary owner
#
        .text

CCSM$snd_change:
        movq    g0,r12                  # save g0-g3
        mov     g4,r11                  # save g4
        ld      cor_rid(g3),r4          # r4 = copy reg. ID
        ld      cor_rcsn(g3),r5         # r5 = copy CM serial #
        stl     r4,snd_change_rid
        ld      cor_PMPsn(g3),r6
        ld      cor_SMPsn(g3),r7
        stl     r6,snd_change_cpo
        stl     g4,snd_change_npo
        stos    r14,snd_change_seq
#
# --- send the request
#
        lda     snd_change_start,g0     # g0 = ptr to packet
        ldconst ccsm_gr_change_size-CCSM_E_OFFSET,g1 # g1 = packet size
        ldconst ebiseDG,g2              # g2 = event type
        ldconst ebibtmaster,g3          # g3 = send to master only
        ldconst 0,g4                    # g4 = serial # of controller
#
.if     ccsm_traces
#
        call    ccsm$trc_sendCCB        # trace sending CCBGram message
#
.endif  # ccsm_traces
#
        call    D$reportEvent           # send packet

        mov     r11,g4                  # restore g4
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: CCSM$snd_susp
#
#  PURPOSE:
#
#       This routine packs and sends a Suspend Ownership message to
#       the CCSM residing on the specified controller.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$snd_susp
#
#  INPUT:
#
#       g2 = sequence #
#       g3 = assoc. COR address
#       g4 = serial # of controller to send to (current pri. owner)
#
#       Note: This routine assumes that the new primary and secondary
#               owners are specified in the cor_PMPsn & cor_SMPsn fields
#               of the specified COR.
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       No regs. destroyed.
#
#**********************************************************************
#
        .data                           # data area
        .align  2                       # align just in case
#
snd_susp_start:
        .word   ccsm_gr_susp_size-CCSM_E_OFFSET # message length
        .byte   ccsm_et_ccbg            # event type code
        .byte   ccsm_gr_susp            # event function code
snd_susp_seq:
        .short  0                       # sequence #
snd_susp_sn:
        .word   0
snd_susp_rid:
        .word   0                       # copy registration ID
snd_susp_cmsn:
        .word   0                       # copy CM serial #
snd_susp_npo:
        .word   0                       # new primary owner
snd_susp_nso:
        .word   0                       # new secondary owner
#
        .text

CCSM$snd_susp:
        movq    g0,r12                  # save g0-g3
        mov     g4,r11                  # save g4
        ld      cor_rid(g3),r4          # r4 = copy reg. ID
        ld      cor_rcsn(g3),r5         # r5 = copy CM serial #
        stl     r4,snd_susp_rid
        ld      cor_PMPsn(g3),r4
        ld      cor_SMPsn(g3),r5
        stl     r4,snd_susp_npo
        stos    r14,snd_susp_seq
#
# --- send the request
#
        lda     snd_susp_start,g0       # g0 = ptr to packet
        ldconst ccsm_gr_susp_size-CCSM_E_OFFSET,g1 # g1 = packet size
        ldconst ebiseDG,g2              # g2 = event type
        ldconst ebibtspec,g3            # g3 = send to specified controller
                                        # g4 = serial # of controller
#
.if     ccsm_traces
#
        call    ccsm$trc_sendCCB        # trace sending CCBGram message
#
.endif  # ccsm_traces
#
        call    D$reportEvent           # send packet

        mov     r11,g4                  # restore g4
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: CCSM$snd_sdone
#
#  PURPOSE:
#
#       This routine packs and sends an Ownership Suspended message to
#       the CCSM residing on the specified controller.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$snd_sdone
#
#  INPUT:
#
#       g2 = sequence #
#       g3 = assoc. COR address
#       g4 = serial # of controller to send to
#
#       Note: This routine assumes that the current primary owner is
#               specified in the cor_PMPsn field of the COR and the
#               current secondary owner is specified in the cor_SMPsn
#               field of the COR.
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       No regs. destroyed.
#
#**********************************************************************
#
        .data                           # data area
        .align  2                       # align just in case
#
snd_sdone_start:
        .word   ccsm_gr_sdone_size-CCSM_E_OFFSET # message length
        .byte   ccsm_et_ccbg            # event type code
        .byte   ccsm_gr_sdone           # event function code
snd_sdone_seq:
        .short  0                       # sequence #
snd_sdone_sn:
        .word   0
snd_sdone_rid:
        .word   0                       # copy registration ID
snd_sdone_cmsn:
        .word   0                       # copy CM serial #
snd_sdone_cpo:
        .word   0                       # current primary owner
snd_sdone_cso:
        .word   0                       # current secondary owner
#
        .text

CCSM$snd_sdone:
        movq    g0,r12                  # save g0-g3
        mov     g4,r11                  # save g4
        ld      cor_rid(g3),r4          # r4 = copy reg. ID
        ld      cor_rcsn(g3),r5         # r5 = copy CM serial #
        stl     r4,snd_sdone_rid
        ld      cor_PMPsn(g3),r6
        ld      cor_SMPsn(g3),r7
        stl     r6,snd_sdone_cpo
        stos    r14,snd_sdone_seq
#
# --- send the request
#
        lda     snd_sdone_start,g0      # g0 = ptr to packet
        ldconst ccsm_gr_sdone_size-CCSM_E_OFFSET,g1 # g1 = packet size
        ldconst ebiseDG,g2              # g2 = event type
        ldconst ebibtspec,g3            # g3 = send to specified controller
                                        # g4 = serial # of controller
#
.if     ccsm_traces
#
        call    ccsm$trc_sendCCB        # trace sending CCBGram message
#
.endif  # ccsm_traces
#
        call    D$reportEvent           # send packet

        mov     r11,g4                  # restore g4
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: CCSM$snd_cdone
#
#  PURPOSE:
#
#       This routine packs and sends an Ownership Changed message to
#       the CCSM residing on the specified controller.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$snd_cdone
#
#  INPUT:
#
#       g0 = new primary owner
#       g1 = new secondary owner
#       g2 = sequence #
#       g3 = assoc. COR address
#       g4 = serial # of controller to send to
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       No regs. destroyed.
#
#**********************************************************************
#
        .data                           # data area
        .align  2                       # align just in case
#
snd_cdone_start:
        .word   ccsm_gr_cdone_size-CCSM_E_OFFSET # message length
        .byte   ccsm_et_ccbg            # event type code
        .byte   ccsm_gr_cdone           # event function code
snd_cdone_seq:
        .short  0                       # sequence #
snd_cdone_sn:
        .word   0
snd_cdone_rid:
        .word   0                       # copy registration ID
snd_cdone_cmsn:
        .word   0                       # copy CM serial #
snd_cdone_npo:
        .word   0                       # new primary owner
snd_cdone_nso:
        .word   0                       # new secondary owner
#
        .text

CCSM$snd_cdone:
        movq    g0,r12                  # save g0-g3
        mov     g4,r11                  # save g4
        ld      cor_rid(g3),r4          # r4 = copy reg. ID
        ld      cor_rcsn(g3),r5         # r5 = copy CM serial #
        stl     r4,snd_cdone_rid
        stl     r12,snd_cdone_npo
        stos    r14,snd_cdone_seq
#
# --- send the request
#
        lda     snd_cdone_start,g0         # g0 = ptr to packet
        ldconst ccsm_gr_cdone_size-CCSM_E_OFFSET,g1 # g1 = packet size
        ldconst ebiseDG,g2              # g2 = event type
        ldconst ebibtspec,g3            # g3 = send to specified controller
                                        # g4 = serial # of controller
#
.if     ccsm_traces
#
        call    ccsm$trc_sendCCB        # trace sending CCBGram message
#
.endif  # ccsm_traces
#
        call    D$reportEvent           # send packet

        mov     r11,g4                  # restore g4
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: CCSM$snd_term
#
#  PURPOSE:
#
#       This routine packs and sends a Terminate Ownership message to
#       the CCSM residing on the specified controller.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$snd_term
#
#  INPUT:
#
#       g2 = sequence #
#       g3 = assoc. COR address
#       g4 = serial # of controller to send to (current pri. owner)
#
#       Note: This routine assumes that the new primary owner is
#               specified in the cor_PMPsn field of the COR and the
#               new secondary owner is specified in the cor_SMPsn
#               field of the COR.
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       No regs. destroyed.
#
#**********************************************************************
#
        .data                           # data area
        .align  2                       # align just in case
#
snd_term_start:
        .word   ccsm_gr_term_size-CCSM_E_OFFSET # message length
        .byte   ccsm_et_ccbg            # event type code
        .byte   ccsm_gr_term            # event function code
snd_term_seq:
        .short  0                       # sequence #
snd_term_sn:
        .word   0
snd_term_rid:
        .word   0                       # copy registration ID
snd_term_cmsn:
        .word   0                       # copy CM serial #
snd_term_npo:
        .word   0                       # new primary owner
snd_term_nso:
        .word   0                       # new secondary owner
#
        .text

CCSM$snd_term:
        movq    g0,r12                  # save g0-g3
        mov     g4,r11                  # save g4
        ld      cor_rid(g3),r4          # r4 = copy reg. ID
        ld      cor_rcsn(g3),r5         # r5 = copy CM serial #
        stl     r4,snd_term_rid
        ld      cor_PMPsn(g3),r6
        ld      cor_SMPsn(g3),r7
        stl     r6,snd_term_npo
        stos    r14,snd_term_seq
#
# --- send the request
#
        lda     snd_term_start,g0          # g0 = ptr to packet
        ldconst ccsm_gr_term_size-CCSM_E_OFFSET,g1 # g1 = packet size
        ldconst ebiseDG,g2              # g2 = event type
        ldconst ebibtspec,g3            # g3 = send to specified controller
                                        # g4 = serial # of controller
#
.if     ccsm_traces
#
        call    ccsm$trc_sendCCB        # trace sending CCBGram message
#
.endif  # ccsm_traces
#
        call    D$reportEvent           # send packet

        mov     r11,g4                  # restore g4
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: CCSM$snd_tdone
#
#  PURPOSE:
#
#       This routine packs and sends an Ownership Terminate message to
#       the CCSM residing on the specified controller.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$snd_tdone
#
#  INPUT:
#
#       g0 = new primary owner
#       g1 = new secondary owner
#       g2 = sequence #
#       g3 = assoc. COR address
#       g4 = serial # of controller to send to
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       No regs. destroyed.
#
#**********************************************************************
#
        .data                           # data area
        .align  2                       # align just in case
#
snd_tdone_start:
        .word   ccsm_gr_tdone_size-CCSM_E_OFFSET # message length
        .byte   ccsm_et_ccbg            # event type code
        .byte   ccsm_gr_tdone           # event function code
snd_tdone_seq:
        .short  0                       # sequence #
snd_tdone_sn:
        .word   0
snd_tdone_rid:
        .word   0                       # copy registration ID
snd_tdone_cmsn:
        .word   0                       # copy CM serial #
snd_tdone_npo:
        .word   0                       # new primary owner
snd_tdone_nso:
        .word   0                       # new secondary owner
#
        .text

CCSM$snd_tdone:
        movq    g0,r12                  # save g0-g3
        mov     g4,r11                  # save g4
        ld      cor_rid(g3),r4          # r4 = copy reg. ID
        ld      cor_rcsn(g3),r5         # r5 = copy CM serial #
        stl     r4,snd_tdone_rid
        stl     r12,snd_tdone_npo
        stos    r14,snd_tdone_seq
#
# --- send the request
#
        lda     snd_tdone_start,g0      # g0 = ptr to packet
        ldconst ccsm_gr_tdone_size-CCSM_E_OFFSET,g1 # g1 = packet size
        ldconst ebiseDG,g2              # g2 = event type
        ldconst ebibtspec,g3            # g3 = send to specified controller
                                        # g4 = serial # of controller
#
.if     ccsm_traces
#
        call    ccsm$trc_sendCCB        # trace sending CCBGram message
#
.endif  # ccsm_traces
#
        call    D$reportEvent           # send packet

        mov     r11,g4                  # restore g4
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: CCSM$snd_readrm
#
#  PURPOSE:
#
#       This routine packs and sends a Read Dirty Region Map message to
#       the CCSM residing on the specified controller.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$snd_readrm
#
#  INPUT:
#
#       g2 = sequence #
#       g3 = assoc. COR address
#       g4 = serial # of controller to send to
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       No regs. destroyed.
#
#**********************************************************************
#
        .data                           # data area
        .align  2                       # align just in case
#
snd_readrm_start:
        .word   ccsm_gr_readrm_size-CCSM_E_OFFSET # message length
        .byte   ccsm_et_ccbg            # event type code
        .byte   ccsm_gr_readrm          # event function code
snd_readrm_seq:
        .short  0                       # sequence #
snd_readrm_sn:
        .word   0
snd_readrm_rid:
        .word   0                       # copy registration ID
snd_readrm_cmsn:
        .word   0                       # copy CM serial #
#
        .text

CCSM$snd_readrm:
        movq    g0,r12                  # save g0-g3
        mov     g4,r11                  # save g4
        ld      cor_rid(g3),r4          # r4 = copy reg. ID
        ld      cor_rcsn(g3),r5         # r5 = copy CM serial #
        stl     r4,snd_readrm_rid
        stos    r14,snd_readrm_seq
#
# --- send the request
#
        lda     snd_readrm_start,g0     # g0 = ptr to packet
        ldconst ccsm_gr_readrm_size-CCSM_E_OFFSET,g1 # g1 = packet size
        ldconst ebiseDG,g2              # g2 = event type
        ldconst ebibtspec,g3            # g3 = send to specified controller
                                        # g4 = serial # of controller
#
.if     ccsm_traces
#
        call    ccsm$trc_sendCCB        # trace sending CCBGram message
#
.endif  # ccsm_traces
#
        call    D$reportEvent           # send packet

        mov     r11,g4                  # restore g4
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: CCSM$snd_rm
#
#  PURPOSE:
#
#       This routine packs and sends a Dirty Region Map message to
#       the CCSM residing on the specified controller.
#
#  DESCRIPTION:
#
#       This routine copies the main region map bitmap from the
#       state NVRAM record and sends it to the requesting controller.
#
#  CALLING SEQUENCE:
#
#       call    CCSM$snd_rm
#
#  INPUT:
#
#       g2 = sequence #
#       g3 = assoc. COR address
#       g4 = serial # of controller to send to
#
#       Note: This routine assumes that the transfer region map bitmap
#               and the main region map bitmap have been merged prior
#               to calling.
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       No regs. destroyed.
#
#**********************************************************************
#
        .data                           # data area
        .align  2                       # align just in case
#
snd_rm_start:
        .word   ccsm_gr_rm_size-CCSM_E_OFFSET # message length
        .byte   ccsm_et_ccbg            # event type code
        .byte   ccsm_gr_rm              # event function code
snd_rm_seq:
        .short  0                       # sequence #
snd_rm_sn:
        .word   0
snd_rm_rid:
        .word   0                       # copy registration ID
snd_rm_cmsn:
        .word   0                       # copy CM serial #
snd_rm_data:
        .word   0,0,0,0,0,0,0,0         # dirty region map data area
#
        .text

CCSM$snd_rm:
        movq    g0,r12                  # save g0-g3
        mov     g4,r11                  # save g4
        ld      cor_rid(g3),r4          # r4 = copy reg. ID
        ld      cor_rcsn(g3),r5         # r5 = copy CM serial #
        stl     r4,snd_rm_rid
        stos    r14,snd_rm_seq
#
# --- Copy the local main region map bitmap into the data area to send
#       to requestor.
#
        lda     snd_rm_data,g0          # g0 = user area to copy transfer
                                        #  region map bitmap to
        ld      cor_stnvram(g3),r6      # r6 = address of state NVRAM record
        cmpobe  0,r6,.sndrm_1000        # Jif no state NVRAM record defined
                                        # This should never occur but if it
                                        # has a career change is likely!!!
        ld      p6st_strcmirror(r6),r7  # r7 = state NVRAM record mirror
        cmpobe  0,r7,.sndrm_1000        # Jif no mirror defined. Career change
                                        # likely!!!
        lda     p6st_rm(r7),r6          # r6 = state mirror region map bitmap
        ldconst 8,r7                    # r7 = # words to copy
.sndrm_300:
        ld      (r6),r4
        subo    1,r7,r7
        addo    4,r6,r6
        st      r4,(g0)
        addo    4,g0,g0
        cmpobne 0,r7,.sndrm_300         # Jif more bytes to copy
#
# --- send the request
#
        lda     snd_rm_start,g0         # g0 = ptr to packet
        ldconst ccsm_gr_rm_size-CCSM_E_OFFSET,g1   # g1 = packet size
        ldconst ebiseDG,g2              # g2 = event type
        ldconst ebibtspec,g3            # g3 = send to specified controller
                                        # g4 = serial # of controller
#
.if     ccsm_traces
#
        call    ccsm$trc_sendCCB        # trace sending CCBGram message
#
.endif  # ccsm_traces
#
        call    D$reportEvent           # send packet

.sndrm_1000:
        mov     r11,g4                  # restore g4
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: CCSM$snd_readsm
#
#  PURPOSE:
#
#       This routine packs and sends a Read Dirty Segment Map message to
#       the CCSM residing on the specified controller.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$snd_readsm
#
#  INPUT:
#
#       g0 = region # to read
#       g2 = sequence #
#       g3 = assoc. COR address
#       g4 = serial # of controller to send to
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       No regs. destroyed.
#
#**********************************************************************
#
        .data                           # data area
        .align  2                       # align just in case
#
snd_readsm_start:
        .word   ccsm_gr_readsm_size-CCSM_E_OFFSET # message length
        .byte   ccsm_et_ccbg            # event type code
        .byte   ccsm_gr_readsm          # event function code
snd_readsm_seq:
        .short  0                       # sequence #
snd_readsm_sn:
        .word   0
snd_readsm_rid:
        .word   0                       # copy registration ID
snd_readsm_cmsn:
        .word   0                       # copy CM serial #
snd_readsm_regnum:
        .word   0                       # region # to read
#
        .text

CCSM$snd_readsm:
        movq    g0,r12                  # save g0-g3
        mov     g4,r11                  # save g4
        ld      cor_rid(g3),r4          # r4 = copy reg. ID
        ld      cor_rcsn(g3),r5         # r5 = copy CM serial #
        stl     r4,snd_readsm_rid
        stos    r14,snd_readsm_seq
        st      r12,snd_readsm_regnum
#
# --- send the request
#
        lda     snd_readsm_start,g0     # g0 = ptr to packet
        ldconst ccsm_gr_readsm_size-CCSM_E_OFFSET,g1 # g1 = packet size
        ldconst ebiseDG,g2              # g2 = event type
        ldconst ebibtspec,g3            # g3 = send to specified controller
                                        # g4 = serial # of controller
#
.if     ccsm_traces
#
        call    ccsm$trc_sendCCB        # trace sending CCBGram message
#
.endif  # ccsm_traces
#
        call    D$reportEvent           # send packet

        mov     r11,g4                  # restore g4
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: CCSM$snd_sm
#
#  PURPOSE:
#
#       This routine packs and sends a Dirty Segment Map message to
#       the CCSM residing on the specified controller.
#
#  DESCRIPTION:
#
#       This routine copies the main region map bitmap from the
#       state NVRAM record and sends it to the requesting controller.
#
#  CALLING SEQUENCE:
#
#       call    CCSM$snd_rm
#
#  INPUT:
#
#       g0 = address of segment map data to send
#       g1 = segment # being sent
#       g2 = sequence #
#       g3 = assoc. COR address
#       g4 = serial # of controller to send to
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       No regs. destroyed.
#
#**********************************************************************
#
CCSM$snd_sm:
        movq    g0,r12                  # save g0-g3
        mov     g4,r11                  # save g4
        ldconst ccsm_et_ccbg,r8
        ldconst ccsm_gr_smdata,r9
        ld      cor_rid(g3),r4          # r4 = copy reg. ID
        ld      cor_rcsn(g3),r5         # r5 = copy CM serial #
        mov     g1,r6                   # r6 = segment #
        stob    r8,SM_ccsm_type(g0)
        stob    r9,SM_ccsm_fc(g0)
        stos    r14,SM_ccsm_seq(g0)
        stt     r4,SM_ccsm_ext_id(g0)
        ld      K_ficb,r3               # r3 = FICB address
        ld      fi_cserial(r3),r5       # r5 = controller serial #
        st      r5,SM_ccsm_sendsn(g0)
        ldconst ccsm_gr_sm_size-CCSM_E_OFFSET,g1   # g1 = packet size
        st      g1,SM_ccsm_len(g0)      # save length
#
# --- send the request
#
        lda     SM_ccsm_len(g0),g0      # g0 ptr to packet
                                        # g1 = packet size
        ldconst ebiseDG,g2              # g2 = event type
        ldconst ebibtspec,g3            # g3 = send to specified controller
                                        # g4 = serial # of controller
#
.if     ccsm_traces
#
        call    ccsm$trc_sendCCB        # trace sending CCBGram message
#
.endif  # ccsm_traces
#
        call    D$reportEvent           # send packet

        mov     r11,g4                  # restore g4
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: CCSM$snd_swap
#
#  PURPOSE:
#
#       This routine packs and sends a Swap RAIDs message to
#       all controllers.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$snd_swap
#
#  INPUT:
#
#       g0 = new source RAID id
#       g2 = sequence #
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       No regs. destroyed.
#
#**********************************************************************
#
        .data                           # data area
        .align  2                       # align just in case
#
snd_swap_start:
        .word   ccsm_gr_swap_size-CCSM_E_OFFSET # message offset
        .byte   ccsm_et_ccbg            # event type code
        .byte   ccsm_gr_swap            # event function code
snd_swap_seq:
        .short  0                       # sequence #
snd_swap_sn:
        .word   0
snd_swap_rid:
        .word   0                       # copy registration ID
snd_swap_cmsn:
        .word   0                       # copy CM serial #
snd_swap_raidid:
        .word   0                       # new source RAID id
#
        .text

CCSM$snd_swap:
        movq    g0,r12                  # save g0-g3
        mov     g4,r11                  # save g4
        ld      cor_rid(g3),r4          # r4 = copy reg. ID
        ld      cor_rcsn(g3),r5         # r5 = copy CM serial #
        stl     r4,snd_swap_rid
        stos    r14,snd_swap_seq
        st      r12,snd_swap_raidid
#
# --- send the request
#
        lda     snd_swap_start,g0       # g0 = ptr to packet
        ldconst ccsm_gr_swap_size-CCSM_E_OFFSET,g1 # g1 = packet size
        ldconst ebiseDG,g2              # g2 = event type
        ldconst ebibtallslaves,g3       # g3 = send to all slave controllers
        ldconst 0,g4                    # g4 = serial # of controller
#
.if     ccsm_traces
#
        call    ccsm$trc_sendCCB        # trace sending CCBGram message
#
.endif  # ccsm_traces
#
        call    D$reportEvent           # send packet

        mov     r11,g4                  # restore g4
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: CCSM$snd_swapdone
#
#  PURPOSE:
#
#       This routine packs and sends a Swap RAIDs Completed message to
#       the master controller.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$snd_swapdone
#
#  INPUT:
#
#       g2 = sequence #
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       No regs. destroyed.
#
#**********************************************************************
#
        .data                           # data area
        .align  2                       # align just in case
#
snd_swapdone_start:
        .word   ccsm_gr_swapdone_size-CCSM_E_OFFSET # message length
        .byte   ccsm_et_ccbg            # event type code
        .byte   ccsm_gr_swapdone        # event function code
snd_swapdone_seq:
        .short  0                       # sequence #
snd_swapdone_sn:
        .word   0
snd_swapdone_rid:
        .word   0                       # copy registration ID
snd_swapdone_cmsn:
        .word   0                       # copy CM serial #
#
        .text

CCSM$snd_swapdone:
        movq    g0,r12                  # save g0-g3
        mov     g4,r11                  # save g4
        ldconst 0,r3
        ld      cor_rid(g3),r4          # r4 = copy reg. ID
        ld      cor_rcsn(g3),r5         # r5 = copy CM serial #
        stos    r3,cor_swapseq(g3)      # clear swap RAIDs seq. # in COR
        stl     r4,snd_swapdone_rid
        stos    r14,snd_swapdone_seq
#
# --- send the request
#
        lda     snd_swapdone_start,g0   # g0 = ptr to packet
        ldconst ccsm_gr_swapdone_size-CCSM_E_OFFSET,g1 # g1 = packet size
        ldconst ebiseDG,g2              # g2 = event type
        ldconst ebibtmaster,g3          # g3 = send to master controller
        ldconst 0,g4                    # g4 = serial # of controller
#
.if     ccsm_traces
#
        call    ccsm$trc_sendCCB        # trace sending CCBGram message
#
.endif  # ccsm_traces
#
        call    D$reportEvent           # send packet

        mov     r11,g4                  # restore g4
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: CCSM$snd_disvlchk
#
#  PURPOSE:
#
#       This routine packs and sends a Disable VLink Check message
#       to all controllers.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$snd_disvlchk
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
#       No regs. destroyed.
#
#**********************************************************************
#
        .data                           # data area
        .align  2                       # align just in case
#
snd_disvlchk_start:
        .word   ccsm_gr_disvlchk_size-CCSM_E_OFFSET # message start
        .byte   ccsm_et_ccbg            # event type code
        .byte   ccsm_gr_disvlchk        # event function code
snd_disvlchk_seq:
        .short  0                       # sequence #
snd_disvlchk_sn:
        .word   0
#
        .text

CCSM$snd_disvlchk:
        movq    g0,r12                  # save g0-g3
        mov     g4,r11                  # save g4
#
# --- send the request
#
        lda     snd_disvlchk_start,g0   # g0 = ptr to packet
        ldconst ccsm_gr_disvlchk_size-CCSM_E_OFFSET,g1 # g1 = packet size
        ldconst ebiseDG,g2              # g2 = event type
        ldconst ebibtallslaves,g3       # g3 = send to all slave controllers
        ldconst 0,g4                    # g4 = serial # of controller
#
.if     ccsm_traces
#
        call    ccsm$trc_sendCCB        # trace sending CCBGram message
#
.endif  # ccsm_traces
#
        call    D$reportEvent           # send packet

        mov     r11,g4                  # restore g4
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: CCSM$snd_disfresh
#
#  PURPOSE:
#
#       This routine packs and sends a Disable Refresh NVRAM message
#       to all controllers.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$snd_disfresh
#
#  INPUT:
#
#       g3 = COR
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       No regs. destroyed.
#
#**********************************************************************
#
        .data                           # data area
        .align  2                       # align just in case
#
snd_disfresh_start:
        .word   ccsm_gr_disfresh_size-CCSM_E_OFFSET # message length
        .byte   ccsm_et_ccbg            # event type code
        .byte   ccsm_gr_disfresh        # event function code
snd_disfresh_seq:
        .short  0                       # sequence #
snd_disfresh_sn:
        .word   0
snd_disfresh_usrswap:
        .byte   0
#
        .text

CCSM$snd_disfresh:
        movq    g0,r12                  # save g0-g3
        mov     g4,r11                  # save g4
        ldob    cor_userswap(g3),r10
        stob    r10,snd_disfresh_usrswap
#       c fprintf(stderr,"<CCSM$snd_disfresh:>Sending Disable RefreshNVRAM, userswap=%u\n", (UINT32)(r10));
#
# --- send the request
#
        lda     snd_disfresh_start,g0   # g0 = ptr to packet
        ldconst ccsm_gr_disfresh_size-CCSM_E_OFFSET,g1 # g1 = packet size
        ldconst ebiseDG,g2              # g2 = event type
        ldconst ebibtallslaves,g3       # g3 = send to all slave controllers
        ldconst 0,g4                    # g4 = serial # of controller
#
.if     ccsm_traces
#
        call    ccsm$trc_sendCCB        # trace sending CCBGram message
#
.endif  # ccsm_traces
#
        call    D$reportEvent           # send packet

        mov     r11,g4                  # restore g4
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: CCSM$snd_envlchk
#
#  PURPOSE:
#
#       This routine packs and sends an Enable VLink Check message
#       to all controllers.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$snd_envlchk
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
#       No regs. destroyed.
#
#**********************************************************************
#
        .data                           # data area
        .align  2                       # align just in case
#
snd_envlchk_start:
        .word   ccsm_gr_envlchk_size-CCSM_E_OFFSET # message length
        .byte   ccsm_et_ccbg            # event type code
        .byte   ccsm_gr_envlchk         # event function code
snd_envlchk_seq:
        .short  0                       # sequence #
snd_envlchk_sn:
        .word   0
#
        .text

CCSM$snd_envlchk:
        movq    g0,r12                  # save g0-g3
        mov     g4,r11                  # save g4
#
# --- send the request
#
        lda     snd_envlchk_start,g0    # g0 = ptr to packet
        ldconst ccsm_gr_envlchk_size-CCSM_E_OFFSET,g1 # g1 = packet size
        ldconst ebiseDG,g2              # g2 = event type
        ldconst ebibtallslaves,g3       # g3 = send to all slave controllers
        ldconst 0,g4                    # g4 = serial # of controller
#
.if     ccsm_traces
#
        call    ccsm$trc_sendCCB        # trace sending CCBGram message
#
.endif  # ccsm_traces
#
        call    D$reportEvent           # send packet

        mov     r11,g4                  # restore g4
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: CCSM$snd_enfresh
#
#  PURPOSE:
#
#       This routine packs and sends an Enable Refresh NVRAM message
#       to all controllers.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$snd_enfresh
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
#       No regs. destroyed.
#
#**********************************************************************
#
        .data                           # data area
        .align  2                       # align just in case
#
snd_enfresh_start:
        .word   ccsm_gr_enfresh_size-CCSM_E_OFFSET # message length
        .byte   ccsm_et_ccbg            # event type code
        .byte   ccsm_gr_enfresh         # event function code
snd_enfresh_seq:
        .short  0                       # sequence #
snd_enfresh_sn:
        .word   0
snd_enfresh_usrswap:
        .byte   0
#
        .text

CCSM$snd_enfresh:
        movq    g0,r12                  # save g0-g3
        mov     g4,r11                  # save g4
#
        ldob    cor_userswap(g3),r10
        stob    r10,snd_enfresh_usrswap
#c  fprintf(stderr,"<CCSM$snd_enfresh:>Sending Enable RefreshNVRAM, userswap=%u\n", (UINT32)(r10));
#
# clear out the userswap field of COR
#
        ldconst 0,r10
        stob    r10,cor_userswap(g3)
# --- send the request
#
        lda     snd_enfresh_start,g0    # g0 = ptr to packet
        ldconst ccsm_gr_enfresh_size-CCSM_E_OFFSET,g1 # g1 = packet size
        ldconst ebiseDG,g2              # g2 = event type
        ldconst ebibtallslaves,g3       # g3 = send to all slave controllers
        ldconst 0,g4                    # g4 = serial # of controller
#
.if     ccsm_traces
#
        call    ccsm$trc_sendCCB        # trace sending CCBGram message
#
.endif  # ccsm_traces
#
        call    D$reportEvent           # send packet

        mov     r11,g4                  # restore g4
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: CCSM$snd_cdmoved
#
#  PURPOSE:
#
#       This routine packs and sends a Copy Devices Moved message to
#       the master controller.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$snd_cdmoved
#
#  INPUT:
#
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       No regs. destroyed.
#
#**********************************************************************
#
        .data                           # data area
        .align  2                       # align just in case
#
snd_cdmoved_start:
        .word   ccsm_gr_cdm_size-CCSM_E_OFFSET # message length
        .byte   ccsm_et_ccbg            # event type code
        .byte   ccsm_gr_cdmove          # event function code
snd_cdmoved_seq:
        .short  0                       # sequence #
snd_cdmoved_sn:
        .word   0
snd_cdmoved_rid:
        .word   0                       # copy registration ID
snd_cdmoved_cmsn:
        .word   0                       # copy CM serial #
snd_cdmoved_rcscl:
        .byte   0                       # cor_rcscl
snd_cdmoved_rcsvd:
        .byte   0                       # cor_rcsvd
snd_cdmoved_rcdcl:
        .byte   0                       # cor_rcdcl
snd_cdmoved_rcdvd:
        .byte   0                       # cor_rcdvd
snd_cdmoved_rssn:
        .word   0                       # cor_rssn
snd_cdmoved_rdsn:
        .word   0                       # cor_rdsn
snd_cdmoved_rscl:
        .byte   0                       # cor_rscl
snd_cdmoved_rsvd:
        .byte   0                       # cor_rsvd
snd_cdmoved_rdcl:
        .byte   0                       # cor_rdcl
snd_cdmoved_rdvd:
        .byte   0                       # cor_rdvd
#
        .text

CCSM$snd_cdmoved:
        movq    g0,r12                  # save g0-g3
        ld      cor_rid(g3),r4          # r4 = copy reg. ID
        ld      cor_rcsn(g3),r5         # r5 = copy CM serial #
        stl     r4,snd_cdmoved_rid
        ldq     cor_rcscl(g3),r8
        stq     r8,snd_cdmoved_rcscl
        mov     g4,r11                  # save g4
#
# --- send the request
#
        lda     snd_cdmoved_start,g0    # g0 = ptr to packet
        ldconst ccsm_gr_cdm_size-CCSM_E_OFFSET,g1  # g1 = packet size
        ldconst ebiseDG,g2              # g2 = event type
        ldconst ebibtmaster,g3          # g3 = send to master controller
        ldconst 0,g4                    # g4 = serial # of controller
#
.if     ccsm_traces
#
        call    ccsm$trc_sendCCB        # trace sending CCBGram message
#
.endif  # ccsm_traces
#
        call    D$reportEvent           # send packet

        mov     r11,g4                  # restore g4
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: CCSM$snd_cdmack
#
#  PURPOSE:
#
#       This routine packs and sends a Copy Devices Moved
#       Acknowledgement message to the specified controller.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$snd_cdmack
#
#  INPUT:
#
#       g2 = sequence #
#       g3 = assoc. COR address
#       g4 = serial # of controller to send to
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       No regs. destroyed.
#
#**********************************************************************
#
        .data                           # data area
        .align  2                       # align just in case
#
snd_cdmack_start:
        .word   ccsm_gr_cdmack_size-CCSM_E_OFFSET # message length
        .byte   ccsm_et_ccbg            # event type code
        .byte   ccsm_gr_cdmack          # event function code
snd_cdmack_seq:
        .short  0                       # sequence #
snd_cdmack_sn:
        .word   0
snd_cdmack_rid:
        .word   0                       # copy registration ID
snd_cdmack_cmsn:
        .word   0                       # copy CM serial #
snd_cdmack_rcscl:
        .byte   0                       # cor_rcscl
snd_cdmack_rcsvd:
        .byte   0                       # cor_rcsvd
snd_cdmack_rcdcl:
        .byte   0                       # cor_rcdcl
snd_cdmack_rcdvd:
        .byte   0                       # cor_rcdvd
snd_cdmack_rssn:
        .word   0                       # cor_rssn
snd_cdmack_rdsn:
        .word   0                       # cor_rdsn
snd_cdmack_rscl:
        .byte   0                       # cor_rscl
snd_cdmack_rsvd:
        .byte   0                       # cor_rsvd
snd_cdmack_rdcl:
        .byte   0                       # cor_rdcl
snd_cdmack_rdvd:
        .byte   0                       # cor_rdvd
#
        .text

CCSM$snd_cdmack:
        movq    g0,r12                  # save g0-g3
        ld      cor_rid(g3),r4          # r4 = copy reg. ID
        ld      cor_rcsn(g3),r5         # r5 = copy CM serial #
        stl     r4,snd_cdmack_rid
        stos    r14,snd_cdmack_seq
        ldq     cor_rcscl(g3),r8
        stq     r8,snd_cdmack_rcscl
        mov     g4,r11                  # save g4
#
# --- send the request
#
        lda     snd_cdmack_start,g0     # g0 = ptr to packet
        ldconst ccsm_gr_cdmack_size-CCSM_E_OFFSET,g1 # g1 = packet size
        ldconst ebiseDG,g2              # g2 = event type
        ldconst ebibtspec,g3            # g3 = send to specified controller
#
.if     ccsm_traces
#
        call    ccsm$trc_sendCCB        # trace sending CCBGram message
#
.endif  # ccsm_traces
#
        call    D$reportEvent           # send packet

        mov     r11,g4                  # restore g4
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: CCSM$snd_csc
#
#  PURPOSE:
#
#       This routine packs and sends a Copy State Changed message to
#       the master controller.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$snd_csc
#
#  INPUT:
#
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       No regs. destroyed.
#
#**********************************************************************
#
        .data                           # data area
        .align  2                       # align just in case
#
snd_csc_start:
        .word   ccsm_gr_csc_size-CCSM_E_OFFSET # message length
        .byte   ccsm_et_ccbg            # event type code
        .byte   ccsm_gr_csc             # event function code
snd_csc_seq:
        .short  0                       # sequence #
snd_csc_sn:
        .word   0
snd_csc_rid:
        .word   0                       # copy registration ID
snd_csc_cmsn:
        .word   0                       # copy CM serial #
snd_csc_cstate:
        .byte   0                       # cor_cstate
snd_csc_crstate:
        .byte   0                       # cor_crstate
snd_csc_mstate:
        .byte   0                       # cor_mstate
        .byte   0                       # unused
#
        .text

CCSM$snd_csc:
        movq    g0,r12                  # save g0-g3
        mov     g4,r11                  # save g4
        ld      cor_rid(g3),r4          # r4 = copy reg. ID
        ld      cor_rcsn(g3),r5         # r5 = copy CM serial #
        stl     r4,snd_csc_rid
        ldob    cor_cstate(g3),r6       # r6 = cor_cstate
        ldob    cor_crstate(g3),r7      # r7 = cor_crstate
        ldob    cor_mstate(g3),r8       # r8 = cor_mstate
        stob    r6,snd_csc_cstate
        stob    r7,snd_csc_crstate
        stob    r8,snd_csc_mstate
#
# --- send the request
#
        lda     snd_csc_start,g0        # g0 = ptr to packet
        ldconst ccsm_gr_csc_size-CCSM_E_OFFSET,g1  # g1 = packet size
        ldconst ebiseDG,g2              # g2 = event type
        ldconst ebibtmaster,g3          # g3 = send to master controller
        ldconst 0,g4                    # g4 = serial # of controller
#
.if     ccsm_traces
#
        call    ccsm$trc_sendCCB        # trace sending CCBGram message
#
.endif  # ccsm_traces
#
        call    D$reportEvent           # send packet

        mov     r11,g4                  # restore g4
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: CCSM$snd_cscack
#
#  PURPOSE:
#
#       This routine packs and sends a Copy State Changed
#       Acknowledgement message to the specified controller.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$snd_cscack
#
#  INPUT:
#
#       g2 = sequence #
#       g3 = assoc. COR address
#       g4 = serial # of controller to send to
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       No regs. destroyed.
#
#**********************************************************************
#
        .data                           # data area
        .align  2                       # align just in case
#
snd_cscack_start:
        .word   ccsm_gr_cscack_size-CCSM_E_OFFSET # message length
        .byte   ccsm_et_ccbg            # event type code
        .byte   ccsm_gr_cscack          # event function code
snd_cscack_seq:
        .short  0                       # sequence #
snd_cscack_sn:
        .word   0
snd_cscack_rid:
        .word   0                       # copy registration ID
snd_cscack_cmsn:
        .word   0                       # copy CM serial #
snd_cscack_cstate:
        .byte   0                       # cor_cstate
snd_cscack_crstate:
        .byte   0                       # cor_crstate
snd_cscack_mstate:
        .byte   0                       # cor_mstate
        .byte   0                       # unused
#
        .text

CCSM$snd_cscack:
        movq    g0,r12                  # save g0-g3
        ld      cor_rid(g3),r4          # r4 = copy reg. ID
        ld      cor_rcsn(g3),r5         # r5 = copy CM serial #
        stl     r4,snd_cscack_rid
        stos    r14,snd_cscack_seq
        ldob    cor_cstate(g3),r6       # r6 = cor_cstate
        ldob    cor_crstate(g3),r7      # r7 = cor_crstate
        ldob    cor_mstate(g3),r8       # r8 = cor_mstate
        stob    r6,snd_cscack_cstate
        stob    r7,snd_cscack_crstate
        stob    r8,snd_cscack_mstate
#
# --- send the request
#
        lda     snd_cscack_start,g0     # g0 = ptr to packet
        ldconst ccsm_gr_cscack_size-CCSM_E_OFFSET,g1 # g1 = packet size
        ldconst ebiseDG,g2              # g2 = event type
        ldconst ebibtspec,g3            # g3 = send to specified controller
                                        # g4 = serial # of controller
#
.if     ccsm_traces
#
        call    ccsm$trc_sendCCB        # trace sending CCBGram message
#
.endif  # ccsm_traces
#
        call    D$reportEvent           # send packet

        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: CCSM$snd_info
#
#  PURPOSE:
#
#       This routine packs and sends a Copy General Info. Changed message to
#       the master controller.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$snd_info
#
#  INPUT:
#
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       No regs. destroyed.
#
#**********************************************************************
#
        .data                           # data area
        .align  2                       # align just in case
#
snd_info_start:
        .word   ccsm_gr_info_size-CCSM_E_OFFSET # message length
        .byte   ccsm_et_ccbg            # event type code
        .byte   ccsm_gr_info            # event function code
snd_info_seq:
        .short  0                       # sequence #
snd_info_sn:
        .word   0
snd_info_rid:
        .word   0                       # copy registration ID
snd_info_cmsn:
        .word   0                       # copy CM serial #
snd_info_label:
        .word   0,0,0,0                 # cor_label
snd_info_gid:
        .byte   0                       # cor_gid
        .byte   0,0,0                   # unused
#
        .text

CCSM$snd_info:
        movq    g0,r12                  # save g0-g3
        ld      cor_rid(g3),r4          # r4 = copy reg. ID
        ld      cor_rcsn(g3),r5         # r5 = copy CM serial #
        stl     r4,snd_info_rid
        ldq     cor_label(g3),r8        # r8-r11 = cor_label
        stq     r8,snd_info_label
        ldob    cor_gid(g3),r4          # r4 = cor_gid
        stob    r4,snd_info_gid
        mov     g4,r11                  # save g4
#
# --- send the request
#
        lda     snd_info_start,g0       # g0 = ptr to packet
        ldconst ccsm_gr_info_size-CCSM_E_OFFSET,g1  # g1 = packet size
        ldconst ebiseDG,g2              # g2 = event type
        ldconst ebibtmaster,g3          # g3 = send to master controller
        ldconst 0,g4                    # g4 = serial # of controller
#
.if     ccsm_traces
#
        call    ccsm$trc_sendCCB        # trace sending CCBGram message
#
.endif  # ccsm_traces
#
        call    D$reportEvent           # send packet

        mov     r11,g4                  # restore g4
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: CCSM$snd_infoack
#
#  PURPOSE:
#
#       This routine packs and sends a Copy General Info. Changed
#       Acknowledgement message to the specified controller.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$snd_infoack
#
#  INPUT:
#
#       g2 = sequence #
#       g3 = assoc. COR address
#       g4 = serial # of controller to send to
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       No regs. destroyed.
#
#**********************************************************************
#
        .data                           # data area
        .align  2                       # align just in case
#
snd_infoack_start:
        .word   ccsm_gr_infoack_size-CCSM_E_OFFSET # message length
        .byte   ccsm_et_ccbg            # event type code
        .byte   ccsm_gr_infoack         # event function code
snd_infoack_seq:
        .short  0                       # sequence #
snd_infoack_sn:
        .word   0
snd_infoack_rid:
        .word   0                       # copy registration ID
snd_infoack_cmsn:
        .word   0                       # copy CM serial #
snd_infoack_label:
        .word   0,0,0,0                 # cor_label
snd_infoack_gid:
        .byte   0                       # cor_gid
        .byte   0,0,0                   # unused
#
        .text

CCSM$snd_infoack:
        movq    g0,r12                  # save g0-g3
        ld      cor_rid(g3),r4          # r4 = copy reg. ID
        ld      cor_rcsn(g3),r5         # r5 = copy CM serial #
        stl     r4,snd_infoack_rid
        stos    r14,snd_infoack_seq
        ldq     cor_label(g3),r8        # r8-r11 = cor_label
        stq     r8,snd_infoack_label
        ldob    cor_gid(g3),r4          # r4 = cor_gid
        stob    r4,snd_infoack_gid
#
# --- send the request
#
        lda     snd_infoack_start,g0    # g0 = ptr to packet
        ldconst ccsm_gr_infoack_size-CCSM_E_OFFSET,g1 # g1 = packet size
        ldconst ebiseDG,g2              # g2 = event type
        ldconst ebibtspec,g3            # g3 = send to specified controller
                                        # g4 = serial # of controller
#
.if     ccsm_traces
#
        call    ccsm$trc_sendCCB        # trace sending CCBGram message
#
.endif  # ccsm_traces
#
        call    D$reportEvent           # send packet

        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: CCSM$CCTrans
#
#  PURPOSE:
#
#       This routine processes a transition through the C.C.S.E.
#       state table.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$CCTrans
#
#  INPUT:
#
#       g0 = event type
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       No regs. destroyed.
#
#**********************************************************************
#
CCSM$CCTrans:
        PushRegs                        # Save all G registers (stack relative)
#
# --- save the current and new event
#
        ldob    cor_ccsecev(g3),r4      # save last event
        stob    r4,cor_ccselev(g3)
        stob    g0,cor_ccsecev(g3)      # save new event
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>CCSM$CCTrans-- COR ccsecev last event= %lx new event=%lx\n", FEBEMESSAGE, __FILE__, __LINE__,r4,g0);
.endif  # CM_IM_DEBUG
        mov     g0,r9                   # save g0

.if     ccsm_traces
#
        ldconst 0xcc,g0                 # g0 = trace type code
        call    ccsm$trc_trans          # trace event
#
.endif  # ccsm_traces
#
# --- locate the state and action definition table
#
        ld      cor_ccseptr(g3),r8
        ld      0(r8),r10               # r10 = state definition table
        ld      4(r8),r11               # r11 = action definition table
#
# --- index to transition node
#
        ldob    cor_ccsecst(g3),r4      # r4 = current state
        ld      (r10)[r4*1],r10         # find address of column
        addo    r9,r10,r10              # add event to find node
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>CCSM$CCTrans-- COR ccsecst (current)state %lx\n", FEBEMESSAGE, __FILE__, __LINE__,r4);
.endif  # CM_IM_DEBUG
#
# --- extract action routines
#
        ldob    0(r10),r4               # action index 1
        ld      (r11)[r4*1],r4
        ldob    1(r10),r5               # action index 2
        ld      (r11)[r5*1],r5
        ldob    2(r10),r6               # action index 3
        ld      (r11)[r6*1],r6
#
# --- Get new state from node
#
        ldconst s.cs,r8                 # r8 = current state
        ldob    3(r10),r7               # r7 = new state
        cmpobe  r8,r7,.CCTrans_cs       # Jif current state

        ldob    cor_ccsecst(g3),r8      # r8 = current state
        stob    r8,cor_ccselst(g3)      # save as last state
        stob    r7,cor_ccsecst(g3)      # change state
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>CCSM$CCTrans-- COR ccsecst (current)state changing to %lx\n", FEBEMESSAGE, __FILE__, __LINE__,r7);
.endif  # CM_IM_DEBUG
        cmpobe  r7,r8,.CCTrans_cs       # Jif state did not change
        ldob    cor_cstindx(g3),r8      # r8 = trace area index
        stob    r7,cor_cstarea(g3)[r8*1] # save new state in trace area
        addo    1,r8,r8                 # inc. trace area index
        and     0x0f,r8,r8
        stob    r8,cor_cstindx(g3)      # save updated trace index
#
# --- Call action routines
#
.CCTrans_cs:
.if GR_GEORAID15_DEBUG
        ld      cor_srcvdd(g3),r8
        ld      cor_destvdd(g3),r9
        ldos    vd_vid(r8),r8
        ldos    vd_vid(r9),r9
c fprintf(stderr,"%s%s:%u <GR><CCSM$CCTrans>svid=%lx dvid=%lx Action routines r4= %lx r5=%lx r6=%lx\n", FEBEMESSAGE, __FILE__, __LINE__,r8,r9,r4,r5,r6);
.endif  # GR_GEORAID15_DEBUG
        callx   (r4)                    # do action 1
        callx   (r5)                    # do action 2
        callx   (r6)                    # do action 3
#
# --- restore environment and get out
#
        PopRegsVoid                     # Restore all G registers (stack relative)
        ret
#
#**********************************************************************
#
#  NAME: CCSM$OCTrans
#
#  PURPOSE:
#       This routine processes a transition through the O.C.S.E.
#       state table.
#
#  CALLING SEQUENCE:
#       call    CCSM$OCTrans
#
#  INPUT:
#       g0 = event type
#       g3 = assoc. COR address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       No regs. destroyed.
#
#**********************************************************************
#
CCSM$OCTrans:
        PushRegs                        # Save all G registers (stack relative)
#
# --- save the current and new event
#
        ldob    cor_ocsecev(g3),r4      # save last event
        stob    r4,cor_ocselev(g3)
        stob    g0,cor_ocsecev(g3)      # save new event
        mov     g0,r9                   # save g0

.if     ccsm_traces
#
        ldconst Chk4Owner,g0
        cmpobe  r9,g0,.octrans_xx1      # Don't trace check for owner events
        ldconst 0x0c,g0                 # g0 = trace type code
        call    ccsm$trc_trans          # trace event
.octrans_xx1:
#
.endif  # ccsm_traces
#
# --- locate the state and action definition table
#
        ld      cor_ocseptr(g3),r8
        ld      0(r8),r10               # r10 = state definition table
        ld      4(r8),r11               # r11 = action definition table
#
# --- index to transition node
#
        ldob    cor_ocsecst(g3),r4      # r4 = current state
        ld      (r10)[r4*1],r10         # find address of column
        addo    r9,r10,r10              # add event to find node
#
# --- extract action routines
#
        ldob    0(r10),r4               # action index 1
        ld      (r11)[r4*1],r4
        ldob    1(r10),r5               # action index 2
        ld      (r11)[r5*1],r5
        ldob    2(r10),r6               # action index 3
        ld      (r11)[r6*1],r6

#
# --- Get new state from node
#
        ldconst s.cs,r8                 # r8 = current state
        ldob    3(r10),r7               # r7 = new state
        cmpobe  r8,r7,.OCTrans_cs       # Jif current state

        ldob    cor_ocsecst(g3),r8      # r8 = current state
        stob    r8,cor_ocselst(g3)      # save as last state
        stob    r7,cor_ocsecst(g3)      # change state
.if GR_GEORAID15_DEBUG
#c       fprintf(stderr,"<GR><CCSM$OCTrans>setting corOcsecst to =%0lx(hex)\n",r7);
.endif  # GR_GEORAID15_DEBUG
        cmpobe  r7,r8,.OCTrans_cs       # Jif state did not change
        ldob    cor_ostindx(g3),r8      # r8 = trace area index
        stob    r7,cor_ostarea(g3)[r8*1] # save new state in trace area
        addo    1,r8,r8                 # inc. trace area index
        and     0x0f,r8,r8
        stob    r8,cor_ostindx(g3)      # save updated trace index
#
# --- Call action routines
#
.OCTrans_cs:
.if GR_GEORAID15_DEBUG
        ld      cor_srcvdd(g3),r8
        ld      cor_destvdd(g3),r9
# -- Imp:- During I/O and mirroring, when vdisks are forcibly deleted, some times DCN crashes here..
        ldos    vd_vid(r8),r8
        ldos    vd_vid(r9),r9
c fprintf(stderr,"%s%s:%u <GR><CCSM$OCTrans>svid=%lx dvid=%lx Action routines r4= %lx r5=%lx r6=%lx\n", FEBEMESSAGE, __FILE__, __LINE__,r8,r9,r4,r5,r6);
.endif  # GR_GEORAID15_DEBUG
        callx   (r4)                    # do action 1
        callx   (r5)                    # do action 2
        callx   (r6)                    # do action 3
#
# --- restore environment and get out
#
        PopRegsVoid                     # Restore all G registers (stack relative)
        ret
#
.if     ccsm_traces
#
#******************************************************************************
#
#  NAME: ccsm$trc_trans
#
#  PURPOSE:
#
#       This routine allows the tracing of transitions though the
#       O.C.S.E. and C.C.S.E. state tables.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    ccsm$trc_trans
#
#  INPUT:
#
#       g0 = trace type code
#       g3 = cor address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
# Trace Formats
#
# Online (0x01)
#
#    0      1      2      3      4      5      6      7
# |------|------|------|------|------|------|------|-------|
# |  0c  |      |     RID     | ccse | ccse | ccse | ccse  |
# |  cc  |      |             | cev  | lst  | lev  | lst   |
# |------|------|------|------|------|------|------|-------|
#           8      9      a      b      c      d      e      f
#        |------|------|------|------|------|------|------|-------|
#        | ocse | ocse | ocse | ocse |         Time stamp         |
#        | cev  | lst  | lev  | lst  |                            |
#        |------|------|------|------|------|------|------|-------|
#
#******************************************************************************
#
ccsm$trc_trans:
        ldq     ccsm_tr_flags,r4        # r4 = trace flags
                                        # r5 = current trace pointer
                                        # r6 = trace head pointer
                                        # r7 = trace tail pointer
        bbc     3,r4,.trans_tr1000      # Jif trace disabled
        ld      cor_rid(g3),r8          # r8 = COR RID
        ld      cor_ccsecev(g3),r9
        ld      cor_ocsecev(g3),r10
c       r11 = get_tsc_l() & ~0xf; # Get free running bus clock.
        bswap   r8,r8
        stq     r8,(r5)
        stob    g0,(r5)
        lda     16(r5),r5
        cmpobg  r7,r5,.trans_tr200
        mov     r6,r5                   # set current pointer to head
.trans_tr200:
        st      r5,ccsm_tr_cur
.trans_tr1000:
        ret
#
.endif  # ccsm_traces
#
.if     ccsm_traces
#
#******************************************************************************
#
#  NAME: ccsm$trc_sendCCB
#
#  PURPOSE:
#
#       This routine traces outgoing CCBGram messages as specified in the
#       trace flags.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    ccsm$trc_sendCCB
#
#  INPUT:
#
#       g0 = CCBGram message address
#       g1 = CCBGram message length
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
# Trace Formats
#
#******************************************************************************
#
ccsm$trc_sendCCB:
        ldq     ccsm_tr_flags,r4        # r4 = trace flags
                                        # r5 = current trace pointer
                                        # r6 = trace head pointer
                                        # r7 = trace tail pointer
        bbc     5,r4,.sendCCB_1000      # Jif trace disabled
        bswap   g1,r8
        ldconst 0x53,r9
        or      r9,r8,r8
        ld      4(g0),r9
        ldl     8(g0),r10
        stq     r8,(r5)
        lda     16(r5),r5
        cmpobg  r7,r5,.sendCCB_200
        mov     r6,r5                   # set current pointer to head
.sendCCB_200:
        st      r5,ccsm_tr_cur
        bbc     6,r4,.sendCCB_1000      # Jif trace disabled
        ldq     16(g0),r8
        bbc     13,r4,.sendCCB_250      # Jif timestamps disabled
c       r11 = get_tsc_l() & ~0xf; # Get free running bus clock.
.sendCCB_250:
        stq     r8,(r5)
        lda     16(r5),r5
        cmpobg  r7,r5,.sendCCB_400
        mov     r6,r5                   # set current pointer to head
.sendCCB_400:
        st      r5,ccsm_tr_cur
.sendCCB_1000:
        ret
#
.endif  # ccsm_traces
#
#**********************************************************************
#
#  NAME: CCSM$get_seqnum
#
#  PURPOSE:
#
#       This routine allocates the next sequence number that can be
#       used when sending a CCBGram message.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$get_seqnum
#
#  INPUT:
#
#       None.
#
#  OUTPUT:
#
#       g2 = sequence #
#
#  REGS DESTROYED:
#
#       Reg. g2 destroyed.
#
#**********************************************************************
#
CCSM$get_seqnum:
        ldos    ccsm_seqnum,g2          # g2 = next sequence #
        cmpobne 0,g2,.getseqnum_100     # Jif not 0000
        addo    1,g2,g2                 # do not allow 0000 as a sequence #
.getseqnum_100:
        addo    1,g2,r4                 # inc. next seq. #
        stos    r4,ccsm_seqnum          # save next sequence #
        ret
#
#**********************************************************************
#
#  NAME: CCSM$CheckOwner
#
#  PURPOSE:
#
#       This routine is called when the current owner of a copy
#       operation is needed or the controller serial # to check
#       with to determine copy ownership.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$CheckOwner
#
#  INPUT:
#
#       g3 = COR address of copy
#
#  OUTPUT:
#
#       g0 = copy owner controller serial # if known
#       g0 = 0 if copy owner unknown or there is none
#
#  REGS DESTROYED:
#
#       Reg. g0 destroyed.
#
#**********************************************************************
#
        .data
chk_owner:
        .word   0
        .text
#
CCSM$CheckOwner:
        mov     g1,r15                  # save g1
        lda     chk_owner,g1            # g1 = return buffer
        ldconst 0,r7
        st      r7,(g1)
        ldconst Chk4Owner,g0            # g0 = event type code
        call    CCSM$OCTrans            # generate event to O.C.S.E.
                                        # g0 = event type
                                        # g1 = return value buffer address
                                        # g3 = COR address
        ld      (g1),g0                 # g0 = return value
        cmpobne 0,g0,.checkowner_1000   # Jif value returned
        ldob    ccsm_op_state,r3        # r3 = current CCSM op. state
        cmpobne ccsm_st_master,r3,.checkowner_1000 # Jif not the master CCSM
#
# --- I'm the master CCSM, therefore I know who the current copy owner is
#
        ld      cor_powner(g3),g0       # g0 = current pri. owner of this copy
.checkowner_1000:
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: CCSM$chk4resources
#
#  PURPOSE:
#
#       This routine performs a test for resources for the
#       specified copy operation.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$chk4resources
#
#  INPUT:
#
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       g0 = FALSE if resources not owned
#       g0 = TRUE if resources owned
#
#  REGS DESTROYED:
#
#       Reg. g0 destroyed.
#
#**********************************************************************
#
CCSM$chk4resources:
        mov     g1,r15                  # save g1
        ldconst FALSE,r14               # preset return value to FALSE
        ld      cor_srcvdd(g3),r4       # r4 = source VDD address
        ld      cor_destvdd(g3),r5      # r5 = destination VDD address
        cmpobe  0,r4,.chk4res_600       # Jif source VDD not defined
.if GR_GEORAID15_DEBUG
        ldos    vd_vid(r4),r6
c fprintf(stderr,"%s%s:%u <GR><CCSM$chk4resources>src vdd existing..vid=%lx(hex)\n", FEBEMESSAGE, __FILE__, __LINE__,r6);
.endif  # GR_GEORAID15_DEBUG
        ldob    vd_owner(r4),r10        # r10 = source VDD owner
        cmpobe  0,r5,.chk4res_800       # Jif no dest. VDD defined
.if GR_GEORAID15_DEBUG
        ldos    vd_vid(r5),r7
c fprintf(stderr,"%s%s:%u <GR><CCSM$chk4resources>dest vdd existing..vid=%lx(hex)\n", FEBEMESSAGE, __FILE__, __LINE__,r7);
.endif  # GR_GEORAID15_DEBUG
        ldob    vd_owner(r5),r9         # r9 = dest. VDD owner
        cmpobe  r9,r10,.chk4res_800     # Jif VDD owners the same
        ld      ccsm_bad_vdown,r8       # r8 = mismatch count
        addo    1,r8,r8                 # inc. mismatch count
        st      r8,ccsm_bad_vdown       # save updated count
        b       .chk4res_1000
#
# --- Source VDD not defined
#
.chk4res_600:
        ldconst 0xff,r10
        cmpobe  0,r5,.chk4res_800       # Jif dest. VDD not defined
        ldob    vd_owner(r5),r10        # r10 = dest. VDD owner
.chk4res_800:
        ldconst 0xff,r8
        cmpobne r8,r10,.chk4res_830     # Jif owned explicitly
.if GR_GEORAID15_DEBUG
c fprintf(stderr,"%s%s:%u <GR><CCSM$chk4resources>copy explicitly not owned by anybody\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # GR_GEORAID15_DEBUG
#
# --- No explicit ownership of virtual devices associated with copy.
#       Copy should be owned by the master controller.
#
        ldos    K_ii+ii_status,r4       # Get initialization status
        bbc     iimaster,r4,.chk4res_1000 # Jif not master controller
        ldconst TRUE,r14                # return the good news to the caller
.if GR_GEORAID15_DEBUG
c fprintf(stderr,"%s%s:%u <GR><CCSM$chk4resources>Master is the owner..\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # GR_GEORAID15_DEBUG
        b       .chk4res_1000
#
.chk4res_830:
        ld      K_ficb,r3
        ldob    fi_cserial(r3),r3       # r3 = my controller #
        and     0x0f,r3,r3              # mask off serial # digit
        cmpobne r3,r10,.chk4res_1000    # Jif virtual devices not owned by
                                        #  my controller
.if GR_GEORAID15_DEBUG
c fprintf(stderr,"%s%s:%u <GR><CCSM$chk4resources>I am owning the copy\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # GR_GEORAID15_DEBUG
        ldconst TRUE,r14                # return the good news to the caller
.chk4res_1000:
        movl    r14,g0                  # g0 = return value to caller
                                        # restore g1
        ret
#
#**********************************************************************
#
#  NAME: CCSM$cdm_delta
#
#  PURPOSE:
#
#       This routine attempts to resolve all copy device definition
#       differences between the local COR copy device definitions
#       and the copy device definitions received in a Copy Devices
#       Moved message received from the current copy owner.
#
#  DESCRIPTION:
#
#       This routine determines what differences exist in the local
#       COR copy device definitions from those defined in a Copy
#       Devices Moved message received from the current copy owner.
#       When a difference is identified, the local device variables
#       are checked to determine if those devices are in the same
#       state as defined by the current copy owner. When the devices
#       are in the same state, this routine will perform the necessary
#       operations to place the local devices in the same state. If
#       the local devices are not in the appropriate state to setup
#       as defined by the copy owner, the local copy devices are left
#       in their current state and the appropriate status code is
#       returned to the calling routine.
#
#  CALLING SEQUENCE:
#
#       call    CCSM$cdm_delta
#
#  INPUT:
#
#       g1 = Copy Devices Moved ILT from copy owner
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       g0 = difference resolution status code
#            00000000 = all differences resolved
#        Bits 31-24 = remote copy source copy device error code
#                31 = invalid source VDisk #
#                30 = source VDD not defined
#                29 = copy MAG does not own source VDisk
#                28 =
#                27 =
#                26 =
#                25 =
#                24 = source/dest. copy devices either the same or not defined
#             23-16 = remote copy destination copy device error code
#                23 = invalid destination VDisk #
#                22 = destination VDD not defined
#                21 = copy MAG does not own destination VDisk
#                20 =
#                19 =
#                18 =
#                17 =
#                16 = new dest. copy device is already a dest. copy device
#         Bits 15-8 = local copy source copy device error code
#                15 = invalid rcscl/rcsvd defined (rcscl/rcsvd mismatch)
#                14 = rcscl/rcsvd VDD not defined (rcscl/rcsvd mismatch)
#                13 = rcscl/rcsvd VDD not VLink (rcscl/rcsvd mismatch)
#                12 = VLink data mismatch (rcscl/rcsvd mismatch)
#                11 = invalid rcscl/rcsvd defined (rcscl/rcsvd matched)
#                10 = rcscl/rcsvd VDD not defined (rcscl/rcsvd matched)
#                 9 = rcscl/rcsvd VDD not VLink (rcscl/rcsvd matched)
#                 8 = VLink data mismatch (rcscl/rcsvd matched)
#               7-0 = local copy destination copy device error code
#                 7 = invalid rcdcl/rcdvd defined (rcdcl/rcdvd mismatch)
#                 6 = rcdcl/rcdvd VDD not defined (rcdcl/rcdvd mismatch)
#                 5 = rcdcl/rcdvd VDD not VLink (rcdcl/rcdvd mismatch)
#                 4 = VLink data mismatch (rcdcl/rcdvd mismatch)
#                 3 = invalid rcdcl/rcdvd defined (rcdcl/rcdvd matched)
#                 2 = rcdcl/rcdvd VDD not defined (rcdcl/rcdvd matched)
#                 1 = rcdcl/rcdvd VDD not VLink (rcdcl/rcdvd matched)
#                 0 = VLink data mismatch (rcdcl/rcdvd matched)
#
#  REGS DESTROYED:
#
#       Reg. g0 destroyed.
#
#**********************************************************************
#
CCSM$cdm_delta:
        mov     0,r14                   # preset status to successful
        ld      K_ficb,r3               # FICB
        ld      fi_vcgid(r3),r3         # r3 = group serial number
#
# --- Check if local or remote copy
#
        ld      cor_cm(g3),r4           # check if local or remote copy
        cmpobe  0,r4,.cdmdelta_700      # Jif remote copy
#
# --- Local copy differences checking logic ----------------------------------
#
#
# --- Check for source copy device differences for local copy
#
        ldos    ccsm_gr_cdm_rcscl(g1),r4 # r4 = rcscl/rcsvd from owner
        ldos    cor_rcscl(g3),r5        # r5 = rcscl/rcsvd from COR
        ld      cor_scd(g3),g0          # g0 = assoc. SCD address
        cmpobe  r4,r5,.cdmdelta_200     # Jif same
#
# --- The rcscl/rcsvd values differ. Terminate association with the
#       local source copy device if necessary.
#
        call    CM$deact_scd            # deactivate SCD from VDD
        ld      cor_destvdd(g3),r8      # r8 = dest. VDD address
        ldconst 0xffff,r11
        cmpobe.f 0,r8,.cdmdelta_50      # Jif no dest. VDD address defined
        ld      vd_dcd(r8),r9           # r9 = DCD assoc. with VDD
        ld      cor_dcd(g3),r10         # r10 = DCD assoc. with COR
        cmpobne.f r9,r10,.cdmdelta_50   # Jif not the same DCDs
        stos    r11,vd_scorvid(r8)      # "clear" dest. copy virtual ID
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
.cdmdelta_50:
        stos    r11,cor_rcscl(g3)       # "clear" rcscl/rcsvd in COR
        ldconst 0,r8
        st      r8,cor_srcvdd(g3)       # remove source VDD from COR
        cmpobne r4,r11,.cdmdelta_60     # Jif rcscl/rcsvd defined
#
# --- Owner rcscl/rcsvd undefined. Set COR values = owner values and get on
#       with life.
#
        ld      ccsm_gr_cdm_rssn(g1),r4 # r4 = owner rssn
        ldos    ccsm_gr_cdm_rscl(g1),r5 # r5 = owner rscl/rsvd
        st      r4,cor_rssn(g3)
        stos    r5,cor_rscl(g3)
        b       .cdmdelta_300           # check destination copy device
#
.cdmdelta_60:
        ldob    ccsm_gr_cdm_rcscl(g1),r4 # r4 = owner rcscl
        ldob    ccsm_gr_cdm_rcsvd(g1),r5 # r5 = owner rcsvd
#
# --- form and validate VID then locate the associated VDD.
#
        shlo    8,r4,r9                 # r9 = form VID from MS and LS bytes
        addo    r5,r9,r9

        ldconst MAXVIRTUALS,r8          # r8 = max. VDisk #
        cmpobl  r9,r8,.cdmdelta_70      # Jif VDisk # is valid
#
# --- Invalid VID for rcscl/rcsvd from owner. Set error bit and go on.
#
        setbit  15,r14,r14              # Bit 15 = invalid rcscl/rcsvd defined.
        b       .cdmdelta_300           # check destination copy device
#
.cdmdelta_70:
        ld      V_vddindx[r9*4],r9      # r9 = corresponding VDD
        cmpobne 0,r9,.cdmdelta_80       # Jif VDisk defined
#
# --- VDD not defined for specified rcscl/rcsvd. Set error bit and go on.
#
        setbit  14,r14,r14              # Bit 14 = rcscl/rcsvd VDD not defined
        b       .cdmdelta_300           # check destination copy device
#
.cdmdelta_80:
        ld      ccsm_gr_cdm_rssn(g1),r4 # r4 = owner rssn
        cmpobne r4,r3,.cdmdelta_100     # Jif source copy device on remote
#
# --- Source copy device is local. Set local values to owner values,
#       associate COR with VDD as source copy device and continue.
#
#       g0 = assoc. SCD address
#       g1 = Copy Device Moved ILT
#       g3 = COR address
#       r4 = owner rssn value
#       r9 = source copy device VDD address
#
.cdmdelta_85:
        ldos    ccsm_gr_cdm_rcscl(g1),r6 # r6 = owner rcscl/rcsvd
        ldos    ccsm_gr_cdm_rscl(g1),r5 # r5 = owner rscl/rsvd
        st      r4,cor_rssn(g3)
        stos    r6,cor_rcscl(g3)
        stos    r5,cor_rscl(g3)
        st      r9,scd_vdd(g0)          # save VDD address in SCD
        st      r9,cor_srcvdd(g3)       # save VDD address in COR
        call    CM$act_scd              # activate SCD with new VDD
        ld      cor_destvdd(g3),r8      # r8 = dest. VDD address
        cmpobe.f 0,r8,.cdmdelta_90      # Jif no dest. VDD address defined
        ld      vd_dcd(r8),r11          # r11 = DCD assoc. with VDD
        ld      cor_dcd(g3),r10         # r10 = DCD assoc. with COR
        cmpobne.f r11,r10,.cdmdelta_90  # Jif not the same DCDs
        ldos    vd_vid(r9),r10          # r10 = source VDisk #
        stos    r10,vd_scorvid(r8)
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
.cdmdelta_90:
        b       .cdmdelta_300           # check destination copy device
#
.cdmdelta_100:
#
# --- Source copy device is a remote device. Check if specified VDD is
#       a VLink and if so, if it goes to the same virtual device on the
#       remote node as specified by the owner.
#
        ld      vd_rdd(r9),r8           # r8 = RDD address
        ldob    rd_type(r8),r7          # r7 = RAID type code
        cmpobe  rdlinkdev,r7,.cdmdelta_110 # Jif VDD is VLink type device
#
# --- Specified source copy device (on remote) not a VLink. Error.
#
        setbit  13,r14,r14              # Bit 13 = rcscl/rcsvd VDD not VLink
        b       .cdmdelta_300           # check destination copy device
#
.cdmdelta_110:
        ld      rd_psd(r8),r10          # r10 = assoc. PSD address
        ldos    ps_pid(r10),r11         # r11 = Physical ID
        ld      DLM_lddindx[r11*4],r11  # r11 = LDD address
        ldob    ld_class(r11),r10       # r10 = linked device class
        cmpobe  ldmld,r10,.cdmdelta_150 # Jif a MAGNITUDE link device
                                        #  type
.cdmdelta_120:
#
# --- Specified source copy device (on remote) not correct. Error.
#
        setbit  12,r14,r14              # Bit 12 = VLink data mismatch
        b       .cdmdelta_300           # check destination copy device
#
.cdmdelta_150:
        ld      ld_basesn(r11),r10      # r10 = source MAG serial #
        cmpobne.f r10,r4,.cdmdelta_120  # Jif VLink to a different MAG
        ldob    ccsm_gr_cdm_rscl(g1),r6 # r6 = owner rscl
        ldob    ccsm_gr_cdm_rsvd(g1),r7 # r7 = owner rsvd
        ldos    ld_altid(r11),r10       # r10 = alternate ID if defined
        cmpobne 0,r10,.cdmdelta_160     # Jif alternate ID defined
        ldob    ld_basecl(r11),r10      # r10 = source cluster number
        cmpobne.f r6,r10,.cdmdelta_120  # Jif source cluster # different
        ldob    ld_basevd(r11),r10      # r10 = source VDisk number
        cmpobne.f r7,r10,.cdmdelta_120  # Jif source VDisk # different
        b       .cdmdelta_85
#
.cdmdelta_160:
        ldob    ld_altid+1(r11),r10     # r10 = MSB of alternate ID
        clrbit  7,r10,r10               # clear alternate ID flag
        cmpobne r6,r10,.cdmdelta_120    # Jif source alt. ID different
        ldob    ld_altid(r11),r10       # r10 = LSB of alternate ID
        cmpobne r7,r10,.cdmdelta_120    # Jif source alt. ID different
        b       .cdmdelta_85
#
# --- rcscl/rcsvd values match. Check the other source copy device values
#       for a match.
#
.cdmdelta_200:
        ld      ccsm_gr_cdm_rssn(g1),r4 # r4 = owner rssn
        ldconst 0xffff,r6
        cmpobe  r5,r6,.cdmdelta_230     # Jif source copy device not defined
        ldos    ccsm_gr_cdm_rscl(g1),r5 # r5 = owner rscl/rsvd
        ld      cor_rssn(g3),r6         # r6 = local rssn
        ldos    cor_rscl(g3),r7         # r7 = local rscl/rsvd
        cmpobne r4,r6,.cdmdelta_205     # Jif rssn differs
        cmpobe  r5,r7,.cdmdelta_300     # Jif rscl/rsvd the same
#
# --- Either rssn or rscl/rsvd differ.
#
.cdmdelta_205:
        ldob    ccsm_gr_cdm_rcscl(g1),r4 # r4 = owner rcscl
        ldob    ccsm_gr_cdm_rcsvd(g1),r5 # r5 = owner rcsvd
#
# --- form and validate VID then locate the associated VDD.
#
        shlo    8,r4,r9                 # r9 = form VID from MS and LS bytes
        addo    r5,r9,r9

        ldconst MAXVIRTUALS,r8          # r8 = max. VDisk #
        cmpobl  r9,r8,.cdmdelta_210     # Jif VDisk # is valid
#
# --- Invalid VID for rcscl/rcsvd from owner. Set error bit and go on.
#
        setbit  11,r14,r14              # Bit 11 = invalid rcscl/rcsvd defined.
        b       .cdmdelta_300           # check destination copy device
#
.cdmdelta_210:
        ld      V_vddindx[r9*4],r9      # r9 = corresponding VDD
        cmpobne 0,r9,.cdmdelta_220      # Jif VDisk defined
#
# --- VDD not defined for specified rcscl/rcsvd. Set error bit and go on.
#
        setbit  10,r14,r14              # Bit 10 = rcscl/rcsvd VDD not defined
        b       .cdmdelta_300           # check destination copy device
#
.cdmdelta_220:
        ld      ccsm_gr_cdm_rssn(g1),r4 # r4 = owner rssn
        cmpobne r4,r3,.cdmdelta_240     # Jif source copy device on remote
#
# --- Source copy device is local. Set local values to owner values,
#       associate COR with VDD as source copy device and continue.
#
#       g0 = assoc. SCD address
#       g1 = Copy Device Moved ILT
#       g3 = COR address
#       r4 = owner rssn value
#
.cdmdelta_230:
        ldos    ccsm_gr_cdm_rcscl(g1),r6 # r6 = owner rcscl/rcsvd
        ldos    ccsm_gr_cdm_rscl(g1),r5 # r5 = owner rscl/rsvd
        st      r4,cor_rssn(g3)
        stos    r6,cor_rcscl(g3)
        stos    r5,cor_rscl(g3)
        b       .cdmdelta_300           # check destination copy device
#
.cdmdelta_240:
#
# --- Source copy device is a remote device. Check if specified VDD is
#       a VLink and if so, if it goes to the same virtual device on the
#       remote node as specified by the owner.
#
        ld      vd_rdd(r9),r8           # r8 = RDD address
        ldob    rd_type(r8),r7          # r7 = RAID type code
        cmpobe  rdlinkdev,r7,.cdmdelta_250 # Jif VDD is VLink type device
#
# --- Specified source copy device (on remote) not a VLink. Error.
#
        setbit  9,r14,r14               # Bit 9 = rcscl/rcsvd VDD not VLink
        b       .cdmdelta_300           # check destination copy device
#
.cdmdelta_250:
        ld      rd_psd(r8),r10          # r10 = assoc. PSD address
        ldos    ps_pid(r10),r11         # r11 = Physical ID
        ld      DLM_lddindx[r11*4],r11  # r11 = LDD address
        ldob    ld_class(r11),r10       # r10 = linked device class
        cmpobe  ldmld,r10,.cdmdelta_260 # Jif a MAGNITUDE link device
                                        #  type
.cdmdelta_255:
#
# --- Specified source copy device (on remote) not correct. Error.
#
        setbit  8,r14,r14               # Bit 8 = VLink data mismatch
        b       .cdmdelta_300           # check destination copy device
#
.cdmdelta_260:
        ld      ld_basesn(r11),r10      # r10 = source MAG serial #
        cmpobne.f r10,r4,.cdmdelta_255  # Jif VLink to a different MAG
        ldob    ccsm_gr_cdm_rscl(g1),r6 # r6 = owner rscl
        ldob    ccsm_gr_cdm_rsvd(g1),r7 # r7 = owner rsvd
        ldos    ld_altid(r11),r10       # r10 = alternate ID if defined
        cmpobne 0,r10,.cdmdelta_270     # Jif alternate ID defined
        ldob    ld_basecl(r11),r10      # r10 = source cluster number
        cmpobne.f r6,r10,.cdmdelta_255  # Jif source cluster # different
        ldob    ld_basevd(r11),r10      # r10 = source VDisk number
        cmpobne.f r7,r10,.cdmdelta_255  # Jif source VDisk # different
        b       .cdmdelta_230
#
.cdmdelta_270:
        ldob    ld_altid+1(r11),r10     # r10 = MSB of alternate ID
        clrbit  7,r10,r10               # clear alternate ID flag
        cmpobne r6,r10,.cdmdelta_255    # Jif source alt. ID different
        ldob    ld_altid(r11),r10       # r10 = LSB of alternate ID
        cmpobne r7,r10,.cdmdelta_255    # Jif source alt. ID different
        b       .cdmdelta_230
#
# --- Check for destination copy device differences for local copy
#
.cdmdelta_300:
        ldos    ccsm_gr_cdm_rcdcl(g1),r4 # r4 = rcdcl/rcdvd from owner
        ldos    cor_rcdcl(g3),r5        # r5 = rcdcl/rcdvd from COR
        ld      cor_dcd(g3),g0          # g0 = assoc. DCD address
        cmpobe  r4,r5,.cdmdelta_500     # Jif same
#
# --- The rcdcl/rcdvd values differ. Terminate association with the
#       local destination copy device if necessary.
#
        ld      dcd_vdd(g0),r10         # r10 = assoc. VDD of DCD
        cmpobe.f 0,r10,.cdmdelta_360    # Jif no VDD assoc. with DCD
        ld      vd_dcd(r10),r11         # r11 = DCD assoc. with VDD
        cmpobne.f g0,r11,.cdmdelta_330  # Jif DCD not assoc. with VDD
        lda     vdnomirror,r8           # set vd_mirror status
        stob    r8,vd_mirror(r10)       # to not mirrored
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
.cdmdelta_330:
        call    CM$deact_dcd            # deactivate DCD from VDD
        ldconst 0xffff,r11
        stos    r11,cor_rcdcl(g3)       # "clear" rcdcl/rcdvd in COR
        ldconst 0,r8
        st      r8,cor_destvdd(g3)      # remove dest. VDD from COR
        cmpobne r4,r11,.cdmdelta_360    # Jif rcdcl/rcdvd defined
#
# --- Owner rcdcl/rcdvd undefined. Set COR values = owner values and get on
#       with life.
#
        ld      ccsm_gr_cdm_rdsn(g1),r4 # r4 = owner rdsn
        ldos    ccsm_gr_cdm_rdcl(g1),r5 # r5 = owner rdcl/rdvd
        st      r4,cor_rdsn(g3)
        stos    r5,cor_rdcl(g3)
        b       .cdmdelta_1000          # and we're done
#
.cdmdelta_360:
        ldob    ccsm_gr_cdm_rcdcl(g1),r4 # r4 = owner rcdcl
        ldob    ccsm_gr_cdm_rcdvd(g1),r5 # r5 = owner rcdvd
#
# --- form and validate VID then locate the associated VDD.
#
        shlo    8,r4,r9                 # r9 = form VID from MS and LS bytes
        addo    r5,r9,r9

        ldconst MAXVIRTUALS,r8          # r8 = max. VDisk #
        cmpobl  r9,r8,.cdmdelta_370     # Jif VDisk # is valid
#
# --- Invalid VID for rcdcl/rcdvd from owner. Set error bit and go on.
#
        setbit  7,r14,r14               # Bit 7 = invalid rcdcl/rcdvd defined.
        b       .cdmdelta_1000          # and we're done
#
.cdmdelta_370:
        ld      V_vddindx[r9*4],r9      # r9 = corresponding VDD
        cmpobne 0,r9,.cdmdelta_380      # Jif VDisk defined
#
# --- VDD not defined for specified rcdcl/rcdvd. Set error bit and go on.
#
        setbit  6,r14,r14               # Bit 6 = rcdcl/rcdvd VDD not defined
        b       .cdmdelta_1000          # and we're done
#
.cdmdelta_380:
        ld      ccsm_gr_cdm_rdsn(g1),r4 # r4 = owner rdsn
        cmpobne r4,r3,.cdmdelta_400     # Jif dest. copy device on remote
#
# --- Destination copy device is local. Set local values to owner values,
#       associate COR with VDD as destination copy device and continue.
#
#       g0 = assoc. DCD address
#       g1 = Copy Device Moved ILT
#       g3 = COR address
#       r4 = owner rdsn value
#       r9 = destination copy device VDD address
#
.cdmdelta_385:
        ldos    ccsm_gr_cdm_rcdcl(g1),r6 # r6 = owner rcdcl/rcdvd
        ldos    ccsm_gr_cdm_rdcl(g1),r5 # r5 = owner rdcl/rdvd
        st      r4,cor_rdsn(g3)
        stos    r6,cor_rcdcl(g3)
        stos    r5,cor_rdcl(g3)
        st      r9,dcd_vdd(g0)          # save VDD address in DCD
        st      r9,cor_destvdd(g3)      # save VDD address in COR
        call    CM$act_dcd              # activate DCD with new VDD
        ldconst 0xffff,r6
        ld      cor_srcvdd(g3),r7       # r7 = source VDD address
        cmpobe  0,r7,.cdmdelta_385d     # Jif no source VDD defined
        ldos    vd_vid(r7),r6
.cdmdelta_385d:
        stos    r6,vd_scorvid(r9)
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        ldob    cor_crstate(g3),r7      # r7 = cor_crstate value
        cmpobg  corcrst_usersusp,r7,.cdmdelta_386 # Jif not user suspended
        ldconst vdcopyuserpause,r8      # r8 = vd_mirror status
        b       .cdmdelta_389
#
.cdmdelta_386:
        ldob    cor_cstate(g3),r7       # r7 = cor_cstate value
        cmpobne corcst_mirror,r7,.cdmdelta_387 # Jif copy not mirrored
        ldconst vdcopymirror,r8         # r8 = vd_mirror status
        b       .cdmdelta_389
#
.cdmdelta_387:
        ldconst vdcopyto,r8             # r8 = vd_mirror status
.cdmdelta_389:
        stob    r8,vd_mirror(r9)        # save new vd_mirror status
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        call    CM$mmc_sflag            # update VDD flags
        b       .cdmdelta_1000          # and we're done
#
.cdmdelta_400:
#
# --- Destination copy device is a remote device. Check if specified VDD is
#       a VLink and if so, if it goes to the same virtual device on the
#       remote node as specified by the owner.
#
        ld      vd_rdd(r9),r8           # r8 = RDD address
        ldob    rd_type(r8),r7          # r7 = RAID type code
        cmpobe  rdlinkdev,r7,.cdmdelta_410 # Jif VDD is VLink type device
#
# --- Specified destination copy device (on remote) not a VLink. Error.
#
        setbit  5,r14,r14               # Bit 5 = rcdcl/rcdvd VDD not VLink
        b       .cdmdelta_1000          # and we're done
#
.cdmdelta_410:
        ld      rd_psd(r8),r10          # r10 = assoc. PSD address
        ldos    ps_pid(r10),r11         # r11 = Physical ID
        ld      DLM_lddindx[r11*4],r11  # r11 = LDD address
        ldob    ld_class(r11),r10       # r10 = linked device class
        cmpobe  ldmld,r10,.cdmdelta_450 # Jif a MAGNITUDE link device
                                        #  type
.cdmdelta_420:
#
# --- Specified destination copy device (on remote) not correct. Error.
#
        setbit  4,r14,r14               # Bit 4 = VLink data mismatch
        b       .cdmdelta_1000          # and we're done
#
.cdmdelta_450:
        ld      ld_basesn(r11),r10      # r10 = destination MAG serial #
        cmpobne.f r10,r4,.cdmdelta_420  # Jif VLink to a different MAG
        ldob    ccsm_gr_cdm_rdcl(g1),r6 # r6 = owner rdcl
        ldob    ccsm_gr_cdm_rdvd(g1),r7 # r7 = owner rdvd
        ldos    ld_altid(r11),r10       # r10 = alternate ID if defined
        cmpobne 0,r10,.cdmdelta_460     # Jif alternate ID defined
        ldob    ld_basecl(r11),r10      # r10 = destination cluster number
        cmpobne.f r6,r10,.cdmdelta_420  # Jif destination cluster # different
        ldob    ld_basevd(r11),r10      # r10 = destination VDisk number
        cmpobne.f r7,r10,.cdmdelta_420  # Jif destination VDisk # different
        b       .cdmdelta_385
#
.cdmdelta_460:
        ldob    ld_altid+1(r11),r10     # r10 = MSB of alternate ID
        clrbit  7,r10,r10               # clear alternate ID flag
        cmpobne r6,r10,.cdmdelta_420    # Jif destination alt. ID different
        ldob    ld_altid(r11),r10       # r10 = LSB of alternate ID
        cmpobne r7,r10,.cdmdelta_420    # Jif destination alt. ID different
        b       .cdmdelta_385
#
# --- rcdcl/rcdvd values match. Check the other destination copy device
#       values for a match.
#
.cdmdelta_500:
        ld      ccsm_gr_cdm_rdsn(g1),r4 # r4 = owner rdsn
        ldconst 0xffff,r6
        cmpobe  r5,r6,.cdmdelta_530     # Jif dest. copy device not defined
        ldos    ccsm_gr_cdm_rdcl(g1),r5 # r5 = owner rdcl/rdvd
        ld      cor_rdsn(g3),r6         # r6 = local rdsn
        ldos    cor_rdcl(g3),r7         # r7 = local rdcl/rdvd
        cmpobne r4,r6,.cdmdelta_505     # Jif rdsn differs
        cmpobe  r5,r7,.cdmdelta_1000    # Jif rdcl/rdvd the same
#
# --- Either rdsn or rdcl/rdvd differ.
#
.cdmdelta_505:
        ldob    ccsm_gr_cdm_rcdcl(g1),r4 # r4 = owner rcdcl
        ldob    ccsm_gr_cdm_rcdvd(g1),r5 # r5 = owner rcdvd
#
# --- form and validate VID then locate the associated VDD.
#
        shlo    8,r4,r9                 # r9 = form VID from MS and LS bytes
        addo    r5,r9,r9

        ldconst MAXVIRTUALS,r8          # r8 = max. VDisk #
        cmpobl  r9,r8,.cdmdelta_510     # Jif VDisk # is valid
#
# --- Invalid VID for rcdcl/rcdvd from owner. Set error bit and go on.
#
        setbit  3,r14,r14               # Bit 3 = invalid rcdcl/rcdvd defined.
        b       .cdmdelta_1000          # and we're done
#
.cdmdelta_510:
        ld      V_vddindx[r9*4],r9      # r9 = corresponding VDD
        cmpobne 0,r9,.cdmdelta_520      # Jif VDisk defined
#
# --- VDD not defined for specified rcdcl/rcdvd. Set error bit and go on.
#
        setbit  2,r14,r14               # Bit 2 = rcdcl/rcdvd VDD not defined
        b       .cdmdelta_1000          # and we're done
#
.cdmdelta_520:
        ld      ccsm_gr_cdm_rdsn(g1),r4 # r4 = owner rdsn
        cmpobne r4,r3,.cdmdelta_540     # Jif dest. copy device on remote
#
# --- Destination copy device is local. Set local values to owner values,
#       associate COR with VDD as destination copy device and continue.
#
#       g0 = assoc. DCD address
#       g1 = Copy Device Moved ILT
#       g3 = COR address
#       r4 = owner rdsn value
#
.cdmdelta_530:
        ldos    ccsm_gr_cdm_rcdcl(g1),r6 # r6 = owner rcdcl/rcdvd
        ldos    ccsm_gr_cdm_rdcl(g1),r5 # r5 = owner rdcl/rdvd
        st      r4,cor_rdsn(g3)
        stos    r6,cor_rcdcl(g3)
        stos    r5,cor_rdcl(g3)
        b       .cdmdelta_1000          # and we're done
#
.cdmdelta_540:
#
# --- Destination copy device is a remote device. Check if specified VDD is
#       a VLink and if so, if it goes to the same virtual device on the
#       remote node as specified by the owner.
#
        ld      vd_rdd(r9),r8           # r8 = RDD address
        ldob    rd_type(r8),r7          # r7 = RAID type code
        cmpobe  rdlinkdev,r7,.cdmdelta_550 # Jif VDD is VLink type device
#
# --- Specified destination copy device (on remote) not a VLink. Error.
#
        setbit  1,r14,r14               # Bit 1 = rcdcl/rcdvd VDD not VLink
        b       .cdmdelta_1000          # and we're done
#
.cdmdelta_550:
        ld      rd_psd(r8),r10          # r10 = assoc. PSD address
        ldos    ps_pid(r10),r11         # r11 = Physical ID
        ld      DLM_lddindx[r11*4],r11  # r11 = LDD address
        ldob    ld_class(r11),r10       # r10 = linked device class
        cmpobe  ldmld,r10,.cdmdelta_560 # Jif a MAGNITUDE link device
                                        #  type
.cdmdelta_555:
#
# --- Specified destination copy device (on remote) not correct. Error.
#
        setbit  0,r14,r14               # Bit 0 = VLink data mismatch
        b       .cdmdelta_1000          # and we're done
#
.cdmdelta_560:
        ld      ld_basesn(r11),r10      # r10 = dest. MAG serial #
        cmpobne.f r10,r4,.cdmdelta_555  # Jif VLink to a different MAG
        ldob    ccsm_gr_cdm_rdcl(g1),r6 # r6 = owner rdcl
        ldob    ccsm_gr_cdm_rdvd(g1),r7 # r7 = owner rdvd
        ldos    ld_altid(r11),r10       # r10 = alternate ID if defined
        cmpobne 0,r10,.cdmdelta_570     # Jif alternate ID defined
        ldob    ld_basecl(r11),r10      # r10 = dest. cluster number
        cmpobne.f r6,r10,.cdmdelta_555  # Jif dest. cluster # different
        ldob    ld_basevd(r11),r10      # r10 = dest. VDisk number
        cmpobne.f r7,r10,.cdmdelta_555  # Jif dest. VDisk # different
        b       .cdmdelta_530
#
.cdmdelta_570:
        ldob    ld_altid+1(r11),r10     # r10 = MSB of alternate ID
        clrbit  7,r10,r10               # clear alternate ID flag
        cmpobne r6,r10,.cdmdelta_555    # Jif dest. alt. ID different
        ldob    ld_altid(r11),r10       # r10 = LSB of alternate ID
        cmpobne r7,r10,.cdmdelta_555    # Jif dest. alt. ID different
        b       .cdmdelta_530
#
# --- Remote copy differences checking logic ---------------------------------
#
.cdmdelta_700:
#
# --- Check for differences in source copy device
#
        movl    g4,r12                  # save g4-g5
        ldconst 0,g4                    # g4 = VDD address if source copy
                                        #      device
        ldconst 0,g5                    # g5 = VDD address if destination
                                        #      copy device
        ld      ccsm_gr_cdm_rssn(g1),r4 # r4 = owner rssn
        ldos    ccsm_gr_cdm_rscl(g1),r5 # r5 = owner rscl/rsvd
#
# --- Check if owner has us as the source copy device
#
        cmpobne r4,r3,.cdmdelta_750     # Jif owner does not have us as new
                                        #  source copy device
#
# --- We're defined as the new source copy device
#
        bswap   r5,r5
        shro    16,r5,r5                # r5 = VDisk #
        ldconst MAXVIRTUALS,r10         # r10 = max. VDisk #
        cmpobl  r5,r10,.cdmdelta_710    # Jif valid VDisk #
        setbit  31,r14,r14              # Bit 31 = invalid source VDisk #
        b       .cdmdelta_750           # and check destination copy device
#
.cdmdelta_710:
        ld      V_vddindx[r5*4],r8      # r8 = corresponding VDD
        cmpobne 0,r8,.cdmdelta_715      # Jif VDD defined
        setbit  30,r14,r14              # Bit 30 = source VDD not defined
        b       .cdmdelta_750           # and check destination copy device
#
.cdmdelta_715:
        ldconst 0xff,r9
        ldob    ccsm_gr_cdm_rcscl(g1),r10 # r10 = copy MAG source cluster #
        cmpobe  r9,r10,.cdmdelta_730    # Jif source VLink is deleted
#
# --- Validate that the copy manager owns this VDisk
#
        ld      cor_rcsn(g3),r4         # r4 = copy manager serial #
        ld      vd_vlinks(r8),r10       # r10 = first VLAR assoc. with VDD
        cmpobne 0,r10,.cdmdelta_720     # Jif VLARs assoc. with VDD
.cdmdelta_718:
        setbit  29,r14,r14              # Bit 29 = copy MAG does not own VDisk
        b       .cdmdelta_750           # and check destination copy device
#
.cdmdelta_720:
        ld      vlar_srcsn(r10),r7      # r7 = source serial # assoc. with
                                        #      VLAR
        cmpobne r4,r7,.cdmdelta_725     # Jif serial # does not match
        ldob    ccsm_gr_cdm_rcscl(g1),r9 # r9 = source cluster # of CM
        ldob    vlar_srccl(r10),r7      # r7 = source cluster # assoc. with
                                        #      VLAR
        cmpobne r7,r9,.cdmdelta_725     # Jif cluster # does not match
        ldob    ccsm_gr_cdm_rcsvd(g1),r9 # r9 = source VDisk # of CM
        ldob    vlar_srcvd(r10),r7      # r7 = source VDisk # assoc. with VLAR
        cmpobe  r7,r9,.cdmdelta_730     # Jif owned by copy manager node
.cdmdelta_725:
        ld      vlar_link(r10),r10      # r10 = next VLAR on list
        cmpobne 0,r10,.cdmdelta_720     # Jif more VLARs to check
        b       .cdmdelta_718           # return error to caller
#
# --- New source copy device is valid.
#
.cdmdelta_730:
        mov     r8,g4                   # g4 = VDD address of source copy
                                        #      device
#
# --- Check for differences in destination copy device
#
.cdmdelta_750:
        ld      ccsm_gr_cdm_rdsn(g1),r4 # r4 = owner rdsn
        ldos    ccsm_gr_cdm_rdcl(g1),r5 # r5 = owner rdcl/rdvd
#
# --- Check if owner has us as the destination copy device
#
        cmpobne r4,r3,.cdmdelta_800     # Jif owner does not have us as new
                                        #  destination copy device
#
# --- We're defined as the new destination copy device
#
        bswap   r5,r5
        shro    16,r5,r5                # r5 = VDisk #
        ldconst MAXVIRTUALS,r10         # r10 = max. VDisk #
        cmpobl  r5,r10,.cdmdelta_760    # Jif valid VDisk #
        setbit  23,r14,r14              # Bit 23 = invalid dest. VDisk #
        b       .cdmdelta_800           # and we're done with validation
#
.cdmdelta_760:
        ld      V_vddindx[r5*4],r8      # r8 = corresponding VDD
        cmpobne 0,r8,.cdmdelta_765      # Jif VDD defined
        setbit  22,r14,r14              # Bit 22 = dest. VDD not defined
        b       .cdmdelta_800           # and we're done with validation
#
.cdmdelta_765:
        ldconst 0xff,r9
        ldob    ccsm_gr_cdm_rcdcl(g1),r10 # r10 = copy MAG dest. cluster #
        cmpobe  r9,r10,.cdmdelta_780    # Jif dest. VLink is deleted
#
# --- Validate that the copy manager owns this VDisk
#
        ld      cor_rcsn(g3),r4         # r4 = copy manager serial #
        ld      vd_vlinks(r8),r10       # r10 = first VLAR assoc. with VDD
        cmpobne 0,r10,.cdmdelta_770     # Jif VLARs assoc. with VDD
.cdmdelta_768:
        setbit  21,r14,r14              # Bit 21 = copy MAG does not own VDisk
        b       .cdmdelta_800           # and we're done with validation
#
.cdmdelta_770:
        ld      vlar_srcsn(r10),r7      # r7 = source serial # assoc. with
                                        #      VLAR
        cmpobne r4,r7,.cdmdelta_775     # Jif serial # does not match
        ldob    ccsm_gr_cdm_rcdcl(g1),r9 # r9 = dest. cluster # of CM
        ldob    vlar_srccl(r10),r7      # r7 = cluster # assoc. with
                                        #      VLAR
        cmpobne r7,r9,.cdmdelta_775     # Jif cluster # does not match
        ldob    ccsm_gr_cdm_rcdvd(g1),r9 # r9 = dest. VDisk # of CM
        ldob    vlar_srcvd(r10),r7      # r7 = VDisk # assoc. with VLAR
        cmpobe  r7,r9,.cdmdelta_780     # Jif owned by copy manager node
.cdmdelta_775:
        ld      vlar_link(r10),r10      # r10 = next VLAR on list
        cmpobne 0,r10,.cdmdelta_770     # Jif more VLARs to check
        b       .cdmdelta_768           # return error to caller
#
# --- New destination copy device is valid.
#
.cdmdelta_780:
        mov     r8,g5                   # g5 = VDD address of dest. copy
                                        #      device
.cdmdelta_800:
        cmpobne g4,g5,.cdmdelta_805     # Jif source and dest. copy devices
                                        #  defined and different
        setbit  24,r14,r14              # Bit 24 = source/dest. copy devices
                                        #  either the same or not defined
        b       .cdmdelta_990           # and we're done
#
.cdmdelta_805:
        cmpobne 0,r14,.cdmdelta_990     # Jif errors indicated in validation
                                        #  checking
        ld      cor_srcvdd(g3),r5       # r5 = source VDD address from COR
        ld      cor_destvdd(g3),r6      # r6 = dest. VDD address from COR
        cmpobe.t 0,g5,.cdmdelta_810     # Jif dest. VDD not specified
        cmpobe.t g5,r6,.cdmdelta_810    # Jif dest. VDD has not changed
#
# --- Check if new destination VDD is already a destination copy device
#       for another copy operation.
#
        ld      vd_dcd(g5),r7           # r7 = DCD associated with new dest.
                                        #      VDD specified in request
        cmpobe.t 0,r7,.cdmdelta_810     # Jif new dest. VDD does not have a
                                        #  copy operation already associated
                                        #  with it
        setbit  16,r14,r14              # Bit 16 = new dest. copy device is
                                        #  already a dest. copy device of copy
        b       .cdmdelta_990           # and we're done
#
.cdmdelta_810:
#
# --- Update the source copy device information in the COR.
#
        ldos    ccsm_gr_cdm_rcscl(g1),r7 # r7 = copy MAG source cluster #
                                        #  and VDisk #
        stos    r7,cor_rcscl(g3)        # save in COR
        ld      ccsm_gr_cdm_rssn(g1),r7 # r7 = new source MAG serial #
        st      r7,cor_rssn(g3)         # save new source MAG serial #
        ldos    ccsm_gr_cdm_rscl(g1),r7 # r7 = new source cluster/VDisk #
        stos    r7,cor_rscl(g3)         # save new source cluster/VDisk #
        cmpobne.f g4,r5,.cdmdelta_820   # Jif source VDD has changed
        ldconst 0,r5                    # r5 = old SCD address that needs to
                                        #      be terminated (0 indicates no
                                        #      SCD needs to be terminated)
        b       .cdmdelta_850           # and process changes to dest. copy
                                        #  device
#
# --- A change in the source VDD is indicated --------------------------------
#
.cdmdelta_820:
        cmpobe  0,r5,.cdmdelta_830      # Jif source was not defined before
        ldconst 0,r7
        ld      cor_scd(g3),r5          # r5 = old SCD address that needs to
                                        #      be terminated
        st      r7,cor_scd(g3)          # clear SCD address from COR
        st      r7,cor_srcvdd(g3)       # clear source VDD address from COR
.cdmdelta_830:
        cmpobe.f 0,g4,.cdmdelta_850     # Jif source VDD not being defined
#
# --- A new source VDD has been defined. Set up a new SCD to service it ------
#
c       g0 = get_scd();                 # Allocate a SCD data structure
.ifdef M4_DEBUG_SCD
c fprintf(stderr, "%s%s:%u get_scd 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif # M4_DEBUG_SCD
        st      g3,scd_cor(g0)          # save COR address in SCD
        st      g0,cor_scd(g3)          # save SCD address in COR
        st      g4,scd_vdd(g0)          # save source VDD address in SCD
        st      g4,cor_srcvdd(g3)       # save source VDD address in COR
        ldconst scdt_remote,r7          # r7 = scd_type code
        stob    r7,scd_type(g0)         # save scd_type code
        ldob    cor_mstate(g3),r8       # r8 = region/segment map table
                                        #      state code
        lda     CM$wp2_null,r10         # r10 = phase 2 write update handler
                                        #       routine
        cmpobne.t cormst_susp,r8,.cdmdelta_840 # Jif not in suspended state
        lda     CM$wp2_suspend,r10      # r10 = phase 2 write update handler
                                        #       routine
.cdmdelta_840:
        st      r10,scd_p2hand(g0)      # save phase 2 write update handler
                                        #  routine in SCD
        call    CM$act_scd              # activate SCD
.cdmdelta_850:
#
# --- Update the destination copy device information in the COR.
#
        ldos    ccsm_gr_cdm_rcdcl(g1),r7 # r7 = copy MAG dest. cluster #
                                        #  and VDisk #
        stos    r7,cor_rcdcl(g3)        # save in COR
        ld      ccsm_gr_cdm_rdsn(g1),r7 # r7 = new dest. MAG serial #
        st      r7,cor_rdsn(g3)         # save new dest. MAG serial #
        ldos    ccsm_gr_cdm_rdcl(g1),r7 # r7 = new dest. cluster/VDisk #
        stos    r7,cor_rdcl(g3)         # save new dest. cluster/VDisk #
        cmpobne g5,r6,.cdmdelta_860     # Jif dest. VDD has changed
        ldconst 0,r6                    # r6 = old DCD address that needs to
                                        #      be terminated (0 indicates no
                                        #      DCD needs to be terminated)
        b       .cdmdelta_900           # and continue processing request
#
# --- A change in the destination VDD is indicated ---------------------------
#
.cdmdelta_860:
        cmpobe.f 0,r6,.cdmdelta_870     # Jif dest. was not defined before
        ldconst 0,r7
        ld      cor_dcd(g3),r6          # r6 = old DCD address that needs to
                                        #      be terminated
        st      r7,cor_dcd(g3)          # clear DCD address from COR
        st      r7,cor_destvdd(g3)      # clear dest. VDD address from COR
.cdmdelta_870:
        cmpobe.f 0,g5,.cdmdelta_900     # Jif dest. VDD not being defined
c       g0 = get_dcd();                 # Allocate a DCD data structure
.ifdef M4_DEBUG_DCD
c fprintf(stderr, "%s%s:%u get_dcd 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif # M4_DEBUG_DCD
        st      g3,dcd_cor(g0)          # save COR address in DCD
        st      g0,cor_dcd(g3)          # save DCD address in COR
        st      g5,dcd_vdd(g0)          # save dest. VDD address in DCD
        st      g5,cor_destvdd(g3)      # save dest. VDD address in COR
        ldconst dcdt_remote,r7          # r7 = dcd_type code
        stob    r7,dcd_type(g0)         # save dcd_type code
        ldob    cor_mstate(g3),r8       # r8 = region/segment map table
                                        #      state code
        lda     CM$wp2_null,r10         # r10 = phase 2 write update handler
                                        #       routine
        cmpobne.t cormst_susp,r8,.cdmdelta_880 # Jif not in suspended state
        lda     CM$wp2_suspend,r10      # r10 = phase 2 write update handler
                                        #       routine
.cdmdelta_880:
        st      r10,dcd_p2hand(g0)      # save phase 2 write update handler
                                        #  routine in DCD
        call    CM$act_dcd              # activate DCD
.cdmdelta_900:
#
# --- Check if any old SCD/DCD needs to be terminated ------------------------
#
#       r5 = old SCD address that needs to be terminated
#       r6 = old DCD address that needs to be terminated
#
        cmpobe.f 0,r5,.cdmdelta_920     # Jif no old SCD needs to be
                                        #  terminated
        mov     r5,g0                   # g0 = SCD address to deactivate
        call    CM$deact_scd            # deactivate SCD from service
.ifdef M4_DEBUG_SCD
c fprintf(stderr, "%s%s:%u put_scd 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif # M4_DEBUG_SCD
c       put_scd(g0);                    # Deallocate a SCD data structure
.cdmdelta_920:
        cmpobe.f 0,r6,.cdmdelta_990     # Jif no old DCD needs to be
                                        #  terminated
        mov     r6,g0                   # g0 = DCD address to deactivate
        call    CM$deact_dcd            # deactivate DCD from service
.ifdef M4_DEBUG_DCD
c fprintf(stderr, "%s%s:%u put_dcd 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif # M4_DEBUG_DCD
c       put_dcd(g0);                    # Deallocate a DCD data structure
.cdmdelta_990:
        movl    r12,g4                  # restore g4-g5
.cdmdelta_1000:
        mov     r14,g0                  # g0 = difference resolution status
        ret
#
#**********************************************************************
#
#  NAME: CCSM$strcomp
#
#  PURPOSE:
#
#       This routine compares memory a byte at a time.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$strcomp
#
#  INPUT:
#
#       g0 = memory address #1
#       g1 = memory address #2
#       g2 = length of memory to compare
#
#  OUTPUT:
#
#       g0 = FALSE if different
#       g0 = TRUE if the same
#
#  REGS DESTROYED:
#
#       Reg. g0 destroyed.
#
#**********************************************************************
#
CCSM$strcomp:
        movt    g0,r12                  # save g0-g2
                                        # r12 = memory address #1
                                        # r13 = memory address #2
                                        # r14 = length
        ldconst FALSE,r12               # preload return value to FALSE
        shro    2,g2,r11                # r11 = # words to compare
        and     0x03,g2,r10             # r10 = # bytes to compare at end of
                                        #       string
        ldconst 0,r3                    # r3 = index into buffers
.strcomp_100:
        cmpobe  0,r11,.strcomp_300      # Jif no more words to compare
        ld      (g0)[r3*1],r4
!       ld      (g1)[r3*1],r5
        addo    4,r3,r3                 # inc. buffer index
        cmpobne r4,r5,.strcomp_1000     # Jif miscompare detected
        subo    1,r11,r11               # dec. word count
        b       .strcomp_100
#
.strcomp_300:
        cmpobe  0,r10,.strcomp_900      # Jif no bytes at end to check
        ldob    (g0)[r3*1],r4
        ldob    (g1)[r3*1],r5
        addo    1,r3,r3                 # inc. buffer index
        cmpobne r4,r5,.strcomp_1000
        subo    1,r10,r10               # dec. byte count
        b       .strcomp_300
#
.strcomp_900:
        ldconst TRUE,r12                # buffers match response
.strcomp_1000:
        movt    r12,g0                  # restore g0-g2
        ret
#
#**********************************************************************
#
#  NAME: CCSM$p2update
#
#  PURPOSE:
#
#       This routine will schedule a NVRAM save and refresh NVRAM to be
#       performed by the master CCSM at a later time. This routine can be
#       called in place of calling D$p2updateconfig & D$SendRefreshNV for
#       updates that are non-critical. This is meant to reduce the number
#       of NVRAM saves/refreshes for updates that are not critical to copy
#       operation integrity.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$p2update
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
#       No regs. destroyed.
#
#**********************************************************************
#
CCSM$p2update:
        ldob    ccsm_op_state,r4        # r4 = CCSM state code
        cmpobne ccsm_st_master,r4,.p2update_1000 # Jif not master CCSM
        ldob    CCSM_mupd_timer,r4      # r4 = update process timer
        cmpobne 0,r4,.p2update_1000     # Jif update already scheduled
        ldconst ccsm_mupd_to,r4
        stob    r4,CCSM_mupd_timer      # schedule update
        ldob    ccsm_mupd_start,r4      # r4 = count of times update has
                                        #      been scheduled
        addo    1,r4,r4                 # inc. count
        stob    r4,ccsm_mupd_start      # save updated count
.p2update_1000:
        ret
#
#**********************************************************************
#
#  NAME: CCSM$savenrefresh
#
#  PURPOSE:
#
#       This routine checks if the master CCSM and does nothing if not.
#       This routine calls D$p2updateconfig to save P2 NVRAM. Upon return
#       from this call, it checks a status byte (D_p2update_flag) to see
#       if the P2 NVRAM save was performed or is pending. If pending, this
#       routine waits until the P2 NVRAm save completes, at which point it
#       calls D$SendRefreshNV to update P2 NVRAM on the slave controllers.
#       When this has completed, it returns to the calling routine.
#
#       Note: This routine can stall the calling routine waiting for the
#               P2 NVRAM save to complete!
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$savenrefresh
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
#       No regs. destroyed.
#
#**********************************************************************
#
CCSM_savenrefresh:
CCSM$savenrefresh:
        ldob    ccsm_op_state,r4        # r4 = CCSM state code
        cmpobne ccsm_st_master,r4,.savenref_1000 # Jif not master CCSM
        call    D$p2updateconfig        # save P2 NVRAM
.savenref_200:
        ldob    D_p2update_flag,r5      # r5 = save P2 NVRAM flag
        cmpobe  0,r5,.savenref_300      # Jif P2 NVRAM save completed
#
# --- Stall waiting for P2 NVRAM save to complete.
#
        mov     g0,r15                  # save g0
        ldconst 1,g0                    # g0 = delay time (msec.)
        call    K$twait                 # wait awhile for save to complete
        mov     r15,g0                  # restore g0
        b       .savenref_200           # check again if P2 NVRAM save done
#
.savenref_300:
        call    D$SendRefreshNV         # tell other nodes to refresh NVRAM
.savenref_1000:
        ret
#
# --- O.C.S.E Event Routines ------------------------------------------
#
#
#**********************************************************************
#
#  NAME: CCSM$ResetOCSE
#
#  PURPOSE:
#
#       This routine reset O.C.S.E. operational fields in preparation
#       for copy initialization or transfer ownership processing.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$ResetOCSE
#
#  INPUT:
#
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       All regs. except g3 can be destroyed.
#
#**********************************************************************
#
CCSM$ResetOCSE:
        ldconst 0,r5
        stob    r5,cor_ocsert1(g3)      # reset cor. op. retry count
        stob    r5,cor_ocsert2(g3)      # reset process retry count
        stob    r5,cor_ocseto(g3)       # reset timeout value
        ret
#
#**********************************************************************
#
#  NAME: CCSM$Ready_Copy
#
#  PURPOSE:
#
#       This routine processes a ready copy event when the copy is
#       in the SB-Idle state.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$Ready_Copy
#
#  INPUT:
#
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       All regs. except g3 can be destroyed.
#
#**********************************************************************
#
CCSM$Ready_Copy:
#
#*** FINISH-May need to do some initialization before starting CM task.
#
        b       CM$StartTask            # start CM task
#
#**********************************************************************
#
#  NAME: CCSM$Test4Resources
#
#  PURPOSE:
#
#       This routine performs a test for resources for the
#       specified copy operation.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$Test4Resources
#
#  INPUT:
#
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       All regs. except g3 can be destroyed.
#
#**********************************************************************
#
CCSM$Test4Resources:
.if GR_GEORAID15_DEBUG
        ld      cor_srcvdd(g3),r4
        ld      cor_destvdd(g3),r5
        ldos    vd_vid(r4),r4
        ldos    vd_vid(r5),r5
.endif  # GR_GEORAID15_DEBUG
        call    CCSM$chk4resources      # check if we control the resources
                                        #  associated with this copy op.
        cmpobne TRUE,g0,.Test4Res_900   # Jif not the owner of this device
#
# --- Have Copy Resources Handler
#
.if GR_GEORAID15_DEBUG
c fprintf(stderr,"%s%s:%u <GR><CCSM$Test4Resources>svid=%lx dvid = %lx call CCSM$OCTrans with event=HaveResources1=HaveResources\n", FEBEMESSAGE, __FILE__, __LINE__,r4,r5);
.endif  # GR_GEORAID15_DEBUG
        ldconst HaveResources,g0        # g0 = event type code
        call    CCSM$OCTrans            # generate event to O.C.S.E.
                                        # g0 = event type
                                        # g3 = COR address
        b       .Test4Res_1000
#
# --- Do Not Have Copy Resources Handler
#
.Test4Res_900:
        ldconst NotHaveResources,g0     # g0 = event type code
.if GR_GEORAID15_DEBUG
c fprintf(stderr,"%s%s:%u <GR><CCSM$Test4Resources>svid=%lx dvid=%lx call CCSM$OCTrans with event=NotHaveResources1=NotHaveResources\n", FEBEMESSAGE, __FILE__, __LINE__,r4,r5);
.endif  # GR_GEORAID15_DEBUG
        call    CCSM$OCTrans            # generate event to O.C.S.E.
                                        # g0 = event type
                                        # g3 = COR address
.Test4Res_1000:
        ret
#
#**********************************************************************
#
#  NAME: CCSM$Define_own
#
#  PURPOSE:
#
#       This routine sends a Define Ownership CCBGram to the master
#       CCSM.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$Define_own
#
#  INPUT:
#
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       All regs. except g3 can be destroyed.
#
#**********************************************************************
#
CCSM$Define_own:
        ldconst 0,g1                    # g1 = new secondary owner
        call    CCSM$get_seqnum         # allocate a sequence # for message
                                        # g2 = sequence #
        stos    g2,cor_seqnum(g3)       # save seq. # in COR
.if GR_GEORAID15_DEBUG
        ldob cor_rcsvd(g3),r3
        ldob cor_rcdvd(g3),r4
c fprintf(stderr,"%s%s:%u <GR><action routine-r5=CCSM$Define_own->calling CCSm$snd_define..svid=%lx,dvid=%lx...", FEBEMESSAGE, __FILE__, __LINE__,r3,r4);
        ldob cor_cstate(g3),r4
        ldob cor_crstate(g3),r5
c fprintf(stderr,"%s%s:%u CorCstate=%lx,CorCrstate=%lx\n", FEBEMESSAGE, __FILE__, __LINE__,r4,r5);
.endif  # GR_GEORAID15_DEBUG
        call    CCSM$snd_define         # pack and send Define Ownership
        ldconst ccsm_ocse_to1,r4        # r4 = timer setting (secs.)
        stob    r4,cor_ocseto(g3)       # set O.C.S.E. timer
        ldob    cor_ocsert2(g3),r5      # r5 = process retry count
        cmpobne 0,r5,.defineown_1000    # Jif process retry count set

        ldconst ccsm_transprocrty,r5    # r5 = transfer ownership process
                                        #  retry counter value

        ld     cor_cm(g3),r7            # r7 = assoc. CM address
        cmpobe 0,r7,.defineown_01       # Jif remote copy op. being processed
#
# ----  Code added for All BE switch(es) failure at other site
#
        ld   cor_destvdd(g3),r7         # Get destination VDD
c       r4 = GR_IsAllDevMissSyncFlagSet((VDD*)r7);
        cmpobe  FALSE,r4,.defineown_01
.if 1 #GR_GEORAID15_DEBUG
        ldos  vd_vid(r7),r7
c fprintf(stderr,"%s%s:%u <GR><CCSM$Define_own>device miss sync flag =%lx dvid=%lx Retrycnt=1\n", FEBEMESSAGE, __FILE__, __LINE__,r4,r7);
.endif  # 1
        ldconst ccsm_transprocrtysf,r5  # r5 = reset transfer ownership process
                                        # retry counter value for site failure
.defineown_01:
        stob    r5,cor_ocsert2(g3)      # initialize process retry count
.defineown_1000:
        ret
#
#**********************************************************************
#
#  NAME: CCSM$CopyTerminated
#
#  PURPOSE:
#
#       This routine processes the copy terminated event from the
#       CM task.
#
#  DESCRIPTION:
#
#       This routine breaks down all data structures associated with the
#       copy operation. If this routine is called on the master CCSM,
#       this routine saves P2 NVRAM after the copy resources have been
#       deallocated and then sends a notice to all of the other nodes
#       to refresh from NVRAM which should cause the copy operation
#       on all of the other nodes to terminate from the refresh NVRAM
#       processing.
#
#  CALLING SEQUENCE:
#
#       call    CCSM$CopyTerminated
#
#  INPUT:
#
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#       Note: When this routine returns from the calling routine, all
#       resources related to this copy operation have been terminated.
#       The calling routine can then terminate their task (CM task)
#       when we return from this routine.
#
#  REGS DESTROYED:
#
#       No regs. destroyed.
#
#**********************************************************************
#
CCSM$CopyTerminated:
        movq    g0,r12                  # save g0-g3

#
# --- determine if a cache flushing operation is active. if so,
#     disassociate the cache flush ilt and the cor.
#
        ld      cor_flushilt(g3),g2     # is there a flush taking place?
        cmpobe  0,g2,.copyterm_050      # Jif no flush active

        ld      il_pcb(g2),r4           # is the flush task still active?
        cmpobne 0,r4,.copyterm_040      # Jif yes
#
# --- flush task no longer active, disassociate the cor, re enable cache
#     on the source and destination devices, and release the ILT
#
        ldconst 0,r3                    # set null
        st      r3,cor_flushilt(g3)     # clear ilt reference in cor

        ldos    il_w2(g2),r6            # r6 = source VID
        ldos    il_w3(g2),r7            # r7 = destination VID
        ldconst 0xffff,r5               # r4 = null VID
#
#     Clear the temporarily disable of the FE Write Cache of source
#
        cmpobe  r5,r6,.copyterm_020     # Jif NULL
        ld      V_vddindx[r6*4],r3      # r3 = corresponding VDD
        cmpobe  0,r3,.copyterm_020      # jif null

        PushRegs(r3)                    # Save all G regs for "C" call
        mov     r6,g0                   # g0 = load VID
        ldconst WC_CLEAR_T_DISABLE,g1   # g1 = Function to Temp Disable WC
        call    WC_VDiskDisable         # Go Clear the T Disable flag
c fprintf(stderr,"%s%s:%u CCSM$CopyTerminated(src) returned from Enabling VDisk Cache - VID = %x\n", FEBEMESSAGE, __FILE__, __LINE__,(UINT32)r6);
        PopRegsVoid(r3)                 # Restore all G regs

#
#     Clear the temporarily disable of the FE Write Cache of destination
#
.copyterm_020:
        cmpobe  r5,r7,.copyterm_030     # Jif NULL
        ld      V_vddindx[r7*4],r3      # r3 = corresponding VDD
        cmpobe  0,r3,.copyterm_030      # jif null

        PushRegs(r3)                    # Save all G regs for "C" call
        mov     r7,g0                   # g0 = load VID
        ldconst WC_CLEAR_T_DISABLE,g1   # g1 = Function to Temp Disable WC
        call    WC_VDiskDisable         # Go Clear the T Disable flag
c fprintf(stderr,"%s%s:%u CCSM$CopyTerminated(dest) returned from Enabling VDisk Cache - VID = %x\n", FEBEMESSAGE, __FILE__, __LINE__,(UINT32)r7);
        PopRegsVoid(r3)                 # Restore all G regs

.copyterm_030:
        mov     g2,g1                   # restore ilt address
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
        b       .copyterm_050           # br
#
# --- clear cor/ilt association and allow task to reenable cache on source
#     and destination devices.
#
.copyterm_040:
        ldconst 0,r3                    # set null
        st      r3,il_w0(g2)            # clear cor reference in flushing struc
        st      r3,cor_flushilt(g3)     # clear ilt reference in cor

#
# --- cache flush operation not active
#
.copyterm_050:
        ld      cor_destvdd(g3),r4      # r4 = destination VDD address
        call    CM$term_cor             # terminate all data structures
                                        #  associated with this copy
        ldob    ccsm_op_state,r3        # r3 = current CCSM op. state
        cmpobe  ccsm_st_master,r3,.copyterm_100 # Jif the master CCSM
        PushRegs(r3)                    # Save all "g" registers
        call    DL_SetVDiskOwnership    # redo VDisk ownership after copy
                                        #  terminated
        PopRegsVoid(r3)                 # restore environment
#
        b       .copyterm_200
#
.copyterm_100:
        call    CCSM$savenrefresh       # save P2 and refresh slaves
.copyterm_200:
        call    RB_SearchForFailedPSDs  # process failed PSDs for potential
                                        #  ownership change
        call    CCSM$cco                # generate a config. change occurred
                                        #  event
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: CCSM$SuspCurOwn
#
#  PURPOSE:
#
#       This routine starts the process of requesting that the
#       current copy owner suspend ownership.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$SuspCurOwn
#
#  INPUT:
#
#       g1 = Transfer Ownership event ILT
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       All regs. except g1 & g3 can be destroyed.
#
#**********************************************************************
#
CCSM$SuspCurOwn:
        mov     g1,r15                  # save g1
        call    CCSM$get_seqnum         # get a sequence # to use
                                        # g2 = sequence #
        stos    g2,cor_seqnum(g3)       # save sequence # in COR for response
        ld      ccsm_gr_trans_cpo(r15),g4 # g4 = current pri. owner to send
                                        #  suspend ownership to
        ld      ccsm_gr_trans_cso(r15),r5 # r5 = current sec. owner
        st      g4,cor_respsn(g3)       # save controller serial # for
                                        #  response
        st      g4,cor_transcpo(g3)     # save current pri. owner in COR
        st      r5,cor_transcso(g3)     # save current sec. owner in COR
.if GR_GEORAID15_DEBUG
        ldob    cor_rcsvd(g3),r5
        ldob    cor_rcdvd(g3),r4
c fprintf(stderr,"%s%s:%u <GR><CCSM$SuspCurOwn>calling CCSM$snd_susp event to other controller svid=%lx dvid=%lx\n", FEBEMESSAGE, __FILE__, __LINE__,r5,r4);
.endif  # GR_GEORAID15_DEBUG
        call    CCSM$snd_susp           # pack and send Suspend Ownership
        ldconst ccsm_ocse_to1,r4        # r4 = timer setting (secs.)
        stob    r4,cor_ocseto(g3)       # set O.C.S.E. timer
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: CCSM$CurOwnSusp
#
#  PURPOSE:
#
#       This routine processes an Ownership Suspended event when
#       transferring ownership.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$CurOwnSusp
#
#  INPUT:
#
#       g1 = Ownership Suspended event ILT
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       All regs. except g1 & g3 can be destroyed.
#
#**********************************************************************
#
CCSM$CurOwnSusp:
        mov     g1,r15                  # save g1
        call    CCSM$get_seqnum         # get a sequence # to use
                                        # g2 = sequence #
        ld      cor_respsn(g3),g4       # g4 = controller serial #
        stos    g2,cor_seqnum(g3)       # save sequence # in COR for response
.if GR_GEORAID15_DEBUG
        ldob    cor_rcsvd(g3),r5
        ldob    cor_rcdvd(g3),r4
c fprintf(stderr,"%s%s:%u <GR><CCSM$CurOwnSusp> Send read Dirty Region Map event src=%lx,dvid=%lx\n", FEBEMESSAGE, __FILE__, __LINE__,r5,r4);
.endif  # GR_GEORAID15_DEBUG
        call    CCSM$snd_readrm         # pack and send Read Dirty Region Map
        ldconst ccsm_ocse_to1,r4        # r4 = timer setting (secs.)
        stob    r4,cor_ocseto(g3)       # set O.C.S.E. timer
        ldconst ccsm_readrmoprty,r5     # r5 = error retry count
        stob    r5,cor_ocsert1(g3)      # set O.C.S.E. current op. retry count
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: CCSM$RtyReadRM
#
#  PURPOSE:
#
#       This routine resends a Read Dirty Region Map request to the
#       current copy owner.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$RtyReadRM
#
#  INPUT:
#
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       All regs. except g3 can be destroyed.
#
#**********************************************************************
#
CCSM$RtyReadRM:
        ld      cor_transcpo(g3),g4     # g4 = current pri. owner
        call    CCSM$get_seqnum         # get a sequence # to use
                                        # g2 = sequence #
        st      g4,cor_respsn(g3)       # save response serial # in COR
        stos    g2,cor_seqnum(g3)       # save sequence # in COR for response
        call    CCSM$snd_readrm         # pack and send Read Dirty Region Map
        ldconst ccsm_ocse_to1,r4        # r4 = timer setting (secs.)
        stob    r4,cor_ocseto(g3)       # set O.C.S.E. timer
        ret
#
#**********************************************************************
#
#  NAME: CCSM$DirtyRM
#
#  PURPOSE:
#
#       This routine processes a Dirty Region Map Data event when
#       transferring ownership.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$DirtyRM
#
#  INPUT:
#
#       g1 = Dirty Region Map Data event ILT
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       All regs. except g1 & g3 can be destroyed.
#
#**********************************************************************
#
CCSM$DirtyRM:
        mov     g1,r15                  # save g1
#
# --- Copy the Dirty Region Map into my transfer ownership region map bitmap.
#
        lda     ccsm_gr_rm_map(r15),g0  # g0 = address of dirty region map from
                                        #  current copy owner
        PushRegs(r3)                    # Save all "g" registers
        call    P6_CopyTransRM          # copy the region map data over top
                                        #  of the local transfer region map
                                        #  bitmap in state NVRAM
        PopRegsVoid(r3)                 # restore environment
#
        ld      ccsm_e_sendsn(r15),g4   # g4 = controller s/n to send message to
        st      g4,cor_respsn(g3)       # save response serial # in COR
        ldconst maxRMcnt-1,r4           # r4 = max. region # allowed
        ldconst 0,r5                    # r5 = region # being checked
.dirtyRM_300:
        mov     r5,g0                   # g0 = region # to check
        PushRegs(r3)                    # Save all "g" registers
        call    P6_TestTransRM          # check if this region is dirty
        PopRegs(r3)                     # restore environment
        cmpobe  TRUE,g0,.dirtyRM_400    # Jif region is dirty
        addo    1,r5,r5                 # inc. to next region #
        cmpoble r5,r4,.dirtyRM_300      # Jif more regions to check
#
# --- No dirty regions were detected.
#
        ldconst ccsm_termownrty,r5      # r5 = operation retry count
        stob    r5,cor_ocsert1(g3)      # set operation retry count
        ldconst Done,g0                 # g0 = event type code
        call    CCSM$OCTrans            # generate event to O.C.S.E.
                                        # g0 = event type
                                        # g3 = COR address
        b       .dirtyRM_1000
#
.dirtyRM_400:
        call    CCSM$get_seqnum         # allocate a sequence # for message
                                        # g2 = sequence #
        stos    g2,cor_seqnum(g3)       # save seq. # in COR
        mov     r5,g0                   # g0 = region # to read
        st      r5,cor_tregnum(g3)      # save region # in COR
        ld      cor_respsn(g3),g4       # g4 = controller s/n to send message to
        call    CCSM$snd_readsm         # pack and send Read Dirty Segment Map
                                        #  message
        ldconst ccsm_readsmoprty,r5     # r5 = operation retry count
        stob    r5,cor_ocsert1(g3)      # set operation retry count
        ldconst ccsm_ocse_to1,r4        # r4 = timer setting (secs.)
        stob    r4,cor_ocseto(g3)       # set O.C.S.E. timer
.dirtyRM_1000:
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: CCSM$RtyTermOwn
#
#  PURPOSE:
#
#       This routine resends a Terminate Ownership message to
#       the current copy owner.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$RtyTermOwn
#
#  INPUT:
#
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       All regs. except g3 can be destroyed.
#
#**********************************************************************
#
CCSM$RtyTermOwn:
        ld      cor_respsn(g3),g4       # g4 = controller s/n to send message to
        call    CCSM$get_seqnum         # allocate a sequence # for message
                                        # g2 = sequence #
        stos    g2,cor_seqnum(g3)       # save seq. # in COR
        call    CCSM$snd_term           # pack and send Terminate Ownership
        ldconst ccsm_ocse_to1,r4        # r4 = timer setting (secs.)
        stob    r4,cor_ocseto(g3)       # set O.C.S.E. timer
        ret
#
#**********************************************************************
#
#  NAME: CCSM$OwnTerm
#
#  PURPOSE:
#
#       This routine processes an Ownership Terminated event when
#       transferring ownership.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$OwnTerm
#
#  INPUT:
#
#       g1 = Ownership Terminated event ILT
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       All regs. except g1 & g3 can be destroyed.
#
#**********************************************************************
#
CCSM$OwnTerm:
        mov     g1,r15                  # save g1
        ldconst 0,g1                    # g1 = new secondary owner
        call    CCSM$get_seqnum         # allocate a sequence # for message
                                        # g2 = sequence #
        stos    g2,cor_seqnum(g3)       # save seq. # in COR
        call    CCSM$snd_define         # pack and send Define Ownership
        ldconst ccsm_ocse_to1,r4        # r4 = timer setting (secs.)
        stob    r4,cor_ocseto(g3)       # set O.C.S.E. timer
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: CCSM$OwnerAcq
#
#  PURPOSE:
#
#       This routine processes an Ownership Acquired event.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$OwnerAcq
#
#  INPUT:
#
#       g1 = You Are Owner event ILT
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       All regs. except g1 & g3 can be destroyed.
#
#**********************************************************************
#
CCSM$OwnerAcq:
        mov     g1,r15                  # save g1
#
# --- At this point in the transfer ownership process, the transfer ownership
#       region map bitmap represents the dirty regions that have been acquired
#       from the previous copy owner or indicate that all regions are dirty.
#       Also, any segment maps associated with dirty regions may have been
#       acquired from the previous copy owner. Any transfer ownership dirty
#       regions that do not have a corresponding segment map that was acquired
#       from the previous copy owner need to be completely recopied. While the
#       transfer ownership process was in progress, the main region map and
#       any associated segment maps have been used to keep track of the
#       write updates occurring on the source and destination copy devices.
#       At this point, we need to merge the main region/segment maps and
#       transfer ownership region/segment maps into the main region/segment
#       maps and then use this composite map to determine what needs to be
#       resynchronized. After this merge has taken place, the transfer
#       ownership region/segment maps must be cleared/deallocated.
#
.if GR_GEORAID15_DEBUG
        ldob  cor_rcsvd(g3),r5
        ldob  cor_rcdvd(g3),r4
c fprintf(stderr,"%s%s:%u <GR><action routine r5=CCSM$OwnerAcq->svid=%lx,dvid=%lx...", FEBEMESSAGE, __FILE__, __LINE__,r5,r4);
        ldob  cor_cstate(g3),r4
        ldob  cor_crstate(g3),r5
c fprintf(stderr,"%s%s:%u CorCstate=%lx,CorCrstate=%lx calling p6_MergeRM\n", FEBEMESSAGE, __FILE__, __LINE__,r4,r5);
.endif  # GR_GEORAID15_DEBUG

        PushRegs(r3)                    # Save all "g" registers
        call    P6_MergeRM              # merge both region maps to create
                                        #  a composite region/segment map that
                                        #  defines all segments that need to
                                        #  be copied in order to resynchronize
                                        #  these two copy devices.
        PopRegsVoid(r3)                 # restore environment

.if GR_GEORAID15_DEBUG
c fprintf(stderr,"%s%s:%u <GR><CCSM$OwnerAcq>..Call OwnershipAcquired event. calling CM$OwnershipAcquired..\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # GR_GEORAID15_DEBUG
        call    CM$OwnershipAcquired    # generate ownership acquired event
                                        #  to the C.C.S.E. engine
        ld      cor_destvdd(g3),r4      # r4 = destination VDD address
        cmpobe  0,r4,.owneracq_1000     # Jif no dest. VDD defined
        ld      vd_scdhead(r4),r5       # r5 = dest. VDD assoc. SCD address
.owneracq_300:
        cmpobe  0,r5,.owneracq_1000     # Jif no SCD
        ld      scd_cor(r5),r6          # r6 = COR assoc. with SCD
        cmpobe  0,r6,.owneracq_400      # Jif no COR assoc. with SCD
        mov     g3,r8                   # save g3
        mov     r6,g3                   # g3 = COR address of next copy in
                                        #      chain
        ldconst Cpy_Config_Chg,g0       # g0 = event type code
.if GR_GEORAID15_DEBUG
c fprintf(stderr,"%s%s:%u <GR><CCSM$OwnerAcq>Calling CCSm$OCTrans with event CpyConfigChg=Cpy_Config_chg\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # GR_GEORAID15_DEBUG
        call    CCSM$OCTrans            # generate event to O.C.S.E. of next
                                        #  copy
                                        # g0 = event type
                                        # g3 = COR address
        mov     r8,g3                   # restore g3
.owneracq_400:
        ld      scd_link(r5),r5         # r5 = next SCD assoc. with VDD
        b       .owneracq_300
#
.owneracq_1000:
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: CCSM$SuspOwn
#
#  PURPOSE:
#
#       This routine processes a Suspend Ownership event.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$SuspOwn
#
#  INPUT:
#
#       g1 = Suspend Ownership event ILT
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       All regs. except g1 & g3 can be destroyed.
#
#**********************************************************************
#
CCSM$SuspOwn:
.if GR_GEORAID15_DEBUG
        ldob    cor_rcsvd(g3),r5
        ldob    cor_rcdvd(g3),r4
c fprintf(stderr,"%s%s:%u <GR><CCSM$SuspOwn>Call CCSM$CCtrans with ownersuspended(SuspendOwner)event to other controller svid=%lx,dvid=%lx\n", FEBEMESSAGE, __FILE__, __LINE__,r5,r4);
.endif  # GR_GEORAID15_DEBUG
        mov     g1,r15                  # save g1
        ld      ccsm_e_sendsn(r15),r4   # r4 = sender's controller serial #
        st      r4,cor_respsn(g3)       # save in COR
        ldos    ccsm_e_seq(r15),r5      # r5 = request sequence #
        stos    r5,cor_seqnum(g3)       # save in COR
        ldconst SuspendOwner,g0         # g0 = event type code
        call    CCSM$CCTrans            # generate event to C.C.S.E.
                                        # g0 = event type
                                        # g3 = COR address
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: CCSM$SuspOwn2
#
#  PURPOSE:
#
#       This routine processes a Suspend Ownership event when in
#       suspend ownership pending state.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$SuspOwn2
#
#  INPUT:
#
#       g1 = Suspend Ownership event ILT
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       All regs. except g1 & g3 can be destroyed.
#
#**********************************************************************
#
CCSM$SuspOwn2:
        ld      ccsm_e_sendsn(g1),r4    # r4 = sender's controller serial #
        st      r4,cor_respsn(g3)       # save in COR
        ldos    ccsm_e_seq(g1),r5       # r5 = request sequence #
        stos    r5,cor_seqnum(g3)       # save in COR
        ret
#
#**********************************************************************
#
#  NAME: CCSM$SuspOwn3
#
#  PURPOSE:
#
#       This routine processes a Suspend Ownership event when in
#       ownership suspended state.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$SuspOwn3
#
#  INPUT:
#
#       g1 = Suspend Ownership event ILT
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       All regs. except g1 & g3 can be destroyed.
#
#**********************************************************************
#
CCSM$SuspOwn3:
        mov     g1,r15                  # save g1
        ldos    ccsm_e_seq(r15),g2      # g2 = response sequence #
        ld      ccsm_e_sendsn(r15),g4   # g4 = dest. controller serial #
        call    CCSM$snd_sdone          # pack and send Ownership Suspended
                                        #  message
        ldconst ccsm_ocse_to2,r4        # r4 = timeout value
        stob    r4,cor_ocseto(g3)       # set timer
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: CCSM$OwnSusp
#
#  PURPOSE:
#
#       This routine processes an Ownership Suspended event from the
#       C.C.S.E. machine when in suspending ownership state.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$OwnSusp
#
#  INPUT:
#
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       All regs. except g3 can be destroyed.
#
#**********************************************************************
#
CCSM$OwnSusp:
#
# --- When copy ownership has been suspended, merge the two region/segment
#       maps in preparation for the new owner reading the dirty region/segment
#       maps during the continuation of the transfer ownership process.
#
.if GR_GEORAID15_DEBUG
        ldob    cor_rcsvd(g3),r5
        ldob    cor_rcdvd(g3),r4
c fprintf(stderr,"%s%s:%u <GR><CCSM$OwnSusp>Calling P6_MergeRM.svid=%lx dvid=%lx\n", FEBEMESSAGE, __FILE__, __LINE__,r5,r4);
.endif  # GR_GEORAID15_DEBUG
        PushRegs(r3)                    # Save all "g" registers
        call    P6_MergeRM              # merge both region maps to create
                                        #  a composite region/segment map that
                                        #  defines all segments that need to
                                        #  be copied in order to resynchronize
                                        #  these two copy devices.
        PopRegsVoid(r3)                 # restore environment

        ldos    cor_seqnum(g3),g2       # g2 = response sequence #
        ld      cor_respsn(g3),g4       # g4 = dest. controller serial #
.if GR_GEORAID15_DEBUG
c fprintf(stderr,"%s%s:%u <GR><CCSM$OwnSusp>Calling CCSM$snd_sdone svid=%lx dvid=%lx\n", FEBEMESSAGE, __FILE__, __LINE__,r5,r4);
.endif  # GR_GEORAID15_DEBUG
        call    CCSM$snd_sdone          # pack and send Ownership Suspended
                                        #  message
        ldconst ccsm_ocse_to2,r4        # r4 = timeout value
        stob    r4,cor_ocseto(g3)       # set timer
        ret
#
#**********************************************************************
#
#  NAME: CCSM$RDRMap
#
#  PURPOSE:
#
#       This routine processes a Read Dirty Region Map event.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$RDRMap
#
#  INPUT:
#
#       g1 = Read Dirty Region Map event ILT
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       All regs. except g1 & g3 can be destroyed.
#
#**********************************************************************
#
CCSM$RDRMap:
        mov     g1,r15                  # save g1
        ldos    ccsm_e_seq(r15),g2      # g2 = sequence # of response message
        ld      ccsm_e_sendsn(r15),g4   # g4 = controller to send response to
        call    CCSM$snd_rm             # pack and send a Dirty Region Map
                                        #  Data message back to the requestor
        ldconst ccsm_ocse_to2,r4        # r4 = timeout value
        stob    r4,cor_ocseto(g3)       # set timer
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: CCSM$TermOwn
#
#  PURPOSE:
#
#       This routine processes a Terminate Ownership event.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$TermOwn
#
#  INPUT:
#
#       g1 = Terminate Ownership event ILT
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       All regs. except g1 & g3 can be destroyed.
#
#**********************************************************************
#
CCSM$TermOwn:
        mov     g1,r15                  # save g1
        ldos    ccsm_e_seq(r15),r5      # r5 = seq. #
        ld      ccsm_e_sendsn(r15),r4   # r4 = sender's controller serial #
        ld      ccsm_gr_term_npo(r15),g4 # g4 = new pri. owner
        ld      ccsm_gr_term_nso(r15),g5 # g5 = new sec. owner
        stos    r5,cor_transcpo(g3)     # save seq. # in cor_transcpo field
        st      r4,cor_respsn(g3)       # save sender's controller serial #
        call    CCSM$get_seqnum         # allocate a sequence # for message
                                        # g2 = sequence #
        stos    g2,cor_seqnum(g3)       # save seq. # in COR
        call    CCSM$snd_change         # pack and send an Ownership Changed
                                        #  message to the master CCSM
        ldconst ccsm_ocse_to2,r4        # r4 = timeout value
        stob    r4,cor_ocseto(g3)       # set timer
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: CCSM$OwnChanged
#
#  PURPOSE:
#
#       This routine processes an Ownership Changed event when the
#       copy is in the ownership suspended state.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$OwnChanged
#
#  INPUT:
#
#       g1 = Ownership Changed event ILT
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       All regs. except g1 & g3 can be destroyed.
#
#**********************************************************************
#
CCSM$OwnChanged:
        mov     g1,r15                  # save g1
        ldos    cor_transcpo(g3),g2     # g2 = sequence # for response message
        ld      ccsm_gr_cdone_npo(r15),g0 # g0 = new pri. owner
        ld      ccsm_gr_cdone_nso(r15),g1 # g1 = new sec. owner
        ld      cor_respsn(g3),g4       # g4 = controller serial # to send
                                        #  message to
        call    CCSM$snd_tdone          # pack and send an Ownership
                                        #  Terminated message
        ldob    cor_flags(g3),r4        # r4 = cor_flags byte
        clrbit  CFLG_B_DIS_DEVICE,r4,r4 # Clear disable copy devices update
                                        #  flag
        clrbit  CFLG_B_DIS_STATE,r4,r4  # Clear disable copy state update flag
        clrbit  CFLG_B_DIS_GENERAL,r4,r4    # Clear disable copy info. update flag
        stob    r4,cor_flags(g3)        # save update cor_flags byte
#
# --- Clear local region map bitmaps and region maps since we no longer
#       own the copy and thus the local resources no longer count.
#
        call    CM$dealloc_RM           # deallocate main region/segment map
        call    CM$dealloc_transRM      # deallocate transfer region/segment
                                        #  map
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: CCSM$OpTimeout
#
#  PURPOSE:
#
#       This routine processes a current operation timeout event.
#
#  DESCRIPTION:
#
#       This routine decrements the current operation retry counter
#       (cor_ocsert1) and if it is expired drives the current operation
#       retry count expired event through the current O.C.S.E. state.
#       If the retry counter has not expired, it drives the current
#       operation retry count not expired event through the current
#       O.C.S.E. state.
#
#  CALLING SEQUENCE:
#
#       call    CCSM$OpTimeout
#
#  INPUT:
#
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       All regs. except g3 can be destroyed.
#
#**********************************************************************
#
CCSM$OpTimeout:
        ldob    cor_ocsert1(g3),r4      # r4 = current op. retry count
        subo    1,r4,r4                 # dec. retry count
        stob    r4,cor_ocsert1(g3)      # save updated retry count
        cmpobe  0,r4,.optimeout_400     # Jif retry count has expired
#
# --- Current Operation retry counter has not expired
#
        ldconst CurOpRtyNotExp,g0       # g0 = event type code
        call    CCSM$OCTrans            # generate event to O.C.S.E.
                                        # g0 = event type
                                        # g3 = COR address
        b       .optimeout_1000
#
.optimeout_400:
#
# --- Current Operation retry counter has expired
#
        ldconst CurOpRtyExp,g0          # g0 = event type code
        call    CCSM$OCTrans            # generate event to O.C.S.E.
                                        # g0 = event type
                                        # g3 = COR address
.optimeout_1000:
        ret
#
#**********************************************************************
#
#  NAME: CCSM$ProcTimeout
#
#  PURPOSE:
#
#       This routine processes a process timeout event.
#
#  DESCRIPTION:
#
#       This routine decrements the process retry counter
#       (cor_ocsert2) and if it is expired drives the process
#       retry count expired event through the current O.C.S.E. state.
#       If the retry counter has not expired, it drives the process
#       retry count not expired event through the current
#       O.C.S.E. state.
#
#  CALLING SEQUENCE:
#
#       call    CCSM$ProcTimeout
#
#  INPUT:
#
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       All regs. except g3 can be destroyed.
#
#**********************************************************************
#
CCSM$ProcTimeout:
.if GR_GEORAID15_DEBUG
        ld      cor_srcvdd(g3),r5
        ld      cor_destvdd(g3),r6
        ldos    vd_vid(r5),r5
        ldos    vd_vid(r6),r6
.endif  # GR_GEORAID15_DEBUG
        ldob    cor_ocsert2(g3),r4      # r4 = process retry count
        subo    1,r4,r4                 # dec. retry count
        stob    r4,cor_ocsert2(g3)      # save updated retry count
        cmpobe  0,r4,.proctimeout_400   # Jif retry count has expired
#
# --- Process retry counter has not expired
#
.if GR_GEORAID15_DEBUG
c fprintf(stderr,"%s%s:%u <GR><CCSM$ProcTimeout>svid=%lx dvid=%lx..call CCSM$OCTrans with event ProcRtyNotExp1=ProcRtyNotExp\n", FEBEMESSAGE, __FILE__, __LINE__,r5,r6);
.endif  # GR_GEORAID15_DEBUG
        ldconst ProcRtyNotExp,g0        # g0 = event type code
        call    CCSM$OCTrans            # generate event to O.C.S.E.
                                        # g0 = event type
                                        # g3 = COR address
        b       .proctimeout_1000
#
.proctimeout_400:
#
# --- Process retry counter has expired
#
.if GR_GEORAID15_DEBUG
c fprintf(stderr,"%s%s:%u <GR><CCSM$ProcTimeout>svid=%lx dvid=%lx..call CCSM$OCTrans with event ProcRtyExp1=ProcRtyExp\n", FEBEMESSAGE, __FILE__, __LINE__,r5,r6);
.endif  # GR_GEORAID15_DEBUG
        ldconst ProcRtyExp,g0           # g0 = event type code
        call    CCSM$OCTrans            # generate event to O.C.S.E.
                                        # g0 = event type
                                        # g3 = COR address
.proctimeout_1000:
        ret
#
#**********************************************************************
#
#  NAME: CCSM$ResetTransRM
#
#  PURPOSE:
#
#       This routine sets up the transfer ownership region map
#       in preparation to transfer ownership to the local node.
#
#  DESCRIPTION:
#
#       This routine deallocates any transfer ownership region/segment
#       maps and sets all bits in the transfer ownership region map
#       bitmap area of the state NVRAM record in preparation of
#       transferring ownership from the current owner to this node.
#
#  CALLING SEQUENCE:
#
#       call    CCSM$ResetTransRM
#
#  INPUT:
#
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       All regs. except g3 can be destroyed.
#
#**********************************************************************
#
CCSM$ResetTransRM:
#
# --- Deallocate any previous Transfer Ownership region map because it
#       is no longer valid.
#
.if GR_GEORAID15_DEBUG
c fprintf(stderr, "%s%s:%u <GR>ccsm-ResetTransRM-Calling dealloc_transRM..\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # GR_GEORAID15_DEBUG
        call    CM$dealloc_transRM      # deallocate transfer RM
        ld      cor_cm(g3),r4           # r4 = assoc. CM address
        cmpobne 0,r4,.resetransRM_200   # Jif local copy op. being processed
.if GR_GEORAID15_DEBUG
c fprintf(stderr, "%s%s:%u <GR><CCSM$ResetTransRM>cor->cm is NULL <=> Remote Copy!!!\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # GR_GEORAID15_DEBUG
        ldob    cor_crstate(g3),r6      # r6 = cor_crstate value
        cmpobg  corcrst_usersusp,r6,.resetransRM_1000 # Jif remote copy
                                        #               not paused
        b       .resetransRM_300        # jif remote copy being paused
#
.resetransRM_200:
#
# --- Setup Transfer Ownership region map bitmap in state NVRAM record
#       in case region map bitmap is not transferred from the current
#       copy owner.
#
#
# ---   Total BE loss at site (UAMS)
#
        ld   cor_destvdd(g3),r4
c       r5 = GR_IsAllDevMissSyncFlagSet((VDD*)r4);
.if GR_GEORAID15_DEBUG
        ldos  vd_vid(r4),r4
c fprintf(stderr,"%s%s:%u <GR><CCSM$ResetTransRM>dvid=%lx all dev miss flag=%lx-bypass P6_SetAllTransRM\n", FEBEMESSAGE, __FILE__, __LINE__,r4,r5);
.endif  # GR_GEORAID15_DEBUG
        cmpobe TRUE,r5,.resetransRM_1000
.resetransRM_300:
        ld   cor_srcvdd(g3), r4        # Get source VDD
c       r4 = CM_VdiskInstantMirrorAllowed((VDD*)r4);
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>CCSM$ResetTransRM--Instant mirror allow flg=%lx\n", FEBEMESSAGE, __FILE__, __LINE__,r4);
.endif  # CM_IM_DEBUG
        cmpobe  TRUE,r4,.resetransRM_1000
        PushRegs(r3)                    # Save all "g" registers
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>CCSM$ResetTransRM--call SetAllTransRM\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # CM_IM_DEBUG
        call    P6_SetAllTransRM        # initialize the transfer region map
                                        #  bitmap in state NVRAM
        PopRegsVoid(r3)                 # restore environment
#
.resetransRM_1000:
        ret
#
#**********************************************************************
#
#  NAME: CCSM$DeallocSM
#
#  PURPOSE:
#
#       This routine processes a Segment Map Data event when it
#       is not wanted.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$DeallocSM
#
#  INPUT:
#
#       g1 = Segment Map Data event ILT
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       All regs. except g1 & g3 can be destroyed.
#
#**********************************************************************
#
CCSM$DeallocSM:
        ld      ccsm_gr_sm_map(g1),r15  # r15 = segment table address from event
.ifdef M4_DEBUG_SM
c fprintf(stderr, "%s%s:%u put_sm 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_SM
c       put_sm(r15);                    # Deallocate segment map table
        ret
#
#**********************************************************************
#
#  NAME: CCSM$RDSMap
#
#  PURPOSE:
#
#       This routine processes a Read Dirty Segment Map event.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$RDSMap
#
#  INPUT:
#
#       g1 = Read Dirty Segment Map event ILT
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       All regs. except g1 & g3 can be destroyed.
#
#**********************************************************************
#
CCSM$RDSMap:
        mov     g1,r15                  # save g1
        ld      cor_rmaptbl(g3),r4      # r4 = region map table address
        cmpobe  0,r4,.RDSMap_1000       # Jif no region map table defined
        ld      ccsm_gr_readsm_regnum(r15),g1 # g1 = region # to read
        ldconst maxRMcnt-1,r6           # r6 = max. valid segment #
        cmpobl  r6,g1,.RDSMap_1000      # Jif invalid segment # specified
        ld      RM_tbl(r4)[g1*4],g0     # g0 = segment table address from
                                        #  region map table
        cmpobe  0,g0,.RDSMap_1000       # Jif no segment table defined
        ldos    ccsm_e_seq(r15),g2      # g2 = sequence # of response message
        ld      ccsm_e_sendsn(r15),g4   # g4 = controller to send response to
        call    CCSM$snd_sm             # pack and send a Dirty Segment Map
                                        #  Data message back to the requestor
.RDSMap_1000:
        ldconst ccsm_ocse_to2,r4        # r4 = timeout value
        stob    r4,cor_ocseto(g3)       # set timer
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: CCSM$DirtySM
#
#  PURPOSE:
#
#       This routine processes a Dirty Segment Map data event.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$DirtySM
#
#  INPUT:
#
#       g1 = Dirty Segment Map event ILT
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       All regs. except g1 & g3 can be destroyed.
#
#**********************************************************************
#
CCSM$DirtySM:
        mov     g1,r15                  # save g1
        ld      ccsm_gr_sm_regnum(r15),r6 # r6 = region #
        ld      cor_tregnum(g3),r5      # r5 = region # being processed
        cmpobe  r6,r5,.dirtySM_200      # Jif correct region # specified
        mov     r15,g1                  # g1 = Dirty Segment Map event ILT
        call    CCSM$DeallocSM          # deallocate segment table
        b       .dirtySM_1000
#
.dirtySM_200:
        ld      ccsm_gr_sm_map(r15),r4  # r4 = segment table address from event
        ld      cor_transrmap(g3),r5    # r5 = transfer ownership region map
        cmpobne 0,r5,.dirtySM_300       # Jif transfer ownership region map
                                        #  defined
.if GR_GEORAID15_DEBUG
        ldob    cor_rcsvd(g3),r9
        ldob    cor_rcdvd(g3),r10
c fprintf(stderr,"%s%s:%u <GR><CCSM$DirtySM>Calling CCSM$allocrmtbl svid=%lx dvid=%lx\n", FEBEMESSAGE, __FILE__, __LINE__,r9,r10);
.endif  # GR_GEORAID15_DEBUG
c       g0 = get_rm();                  # Allocate a region table
.ifdef M4_DEBUG_RM
c fprintf(stderr, "%s%s:%u get_sm 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif # M4_DEBUG_RM
        st      g0,cor_transrmap(g3)    # save in COR
        mov     g0,r5                   # r5 = transfer ownership region map
.dirtySM_300:
        ld      RM_tbl(r5)[r6*4],g1     # g1 = segment table address from
                                        #  region map table
        cmpobe  0,g1,.dirtySM_350       # Jif no segment table defined
.ifdef M4_DEBUG_SM
c fprintf(stderr, "%s%s:%u put_sm 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_SM
c       put_sm(g1);                     # Deallocate segment map table
.dirtySM_350:
        st      r4,RM_tbl(r5)[r6*4]     # save incoming segment table in
                                        #  region table
        call    CCSM$GetNextSM          # request next dirty region segment map
                                        # or generate Done event
.dirtySM_1000:
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: CCSM$RtyReadSM
#
#  PURPOSE:
#
#       This routine retries sending a Read Dirty Segment Map message
#       for the same region that one was already sent.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$RtyReadSM
#
#  INPUT:
#
#       g3 = assoc. COR address
#
#       Note: The following values are used in the COR to resend the
#               request:
#
#       cor_respsn
#       cor_tregnum
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       All regs. except g3 can be destroyed.
#
#**********************************************************************
#
CCSM$RtyReadSM:
        call    CCSM$get_seqnum         # allocate a sequence # for message
                                        # g2 = sequence #
        stos    g2,cor_seqnum(g3)       # save seq. # in COR
        ld      cor_tregnum(g3),g0      # g0 = region # to read
        ld      cor_respsn(g3),g4       # g4 = controller s/n to send message to
        call    CCSM$snd_readsm         # pack and send Read Dirty Segment Map
                                        #  message
        ldconst ccsm_ocse_to1,r4        # r4 = timeout value
        stob    r4,cor_ocseto(g3)       # set timer
        ret
#
#**********************************************************************
#
#  NAME: CCSM$GetNextSM
#
#  PURPOSE:
#
#       This routine sends a Read Dirty Segment Map request for the next
#       dirty region indicated in the transfer ownership region map.
#       If no more dirty regions are indicated, it drives a "Done"
#       event into the current O.C.S.E. state machine.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$GetNextSM
#
#  INPUT:
#
#       g3 = assoc. COR address
#
#       Note: The following values are used in the COR to resend the
#               request:
#
#       cor_respsn
#       cor_tregnum
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       All regs. except g3 can be destroyed.
#
#**********************************************************************
#
CCSM$GetNextSM:
        ld      cor_tregnum(g3),r5      # r5 = last region # read

        ldconst maxRMcnt-1,r4           # r4 = max. region # allowed
        b       .getnextSM_380
#
.getnextSM_360:
        mov     r5,g0                   # g0 = region # to check
        PushRegs(r3)                    # Save all "g" registers
        call    P6_TestTransRM          # check if this region is dirty
        PopRegs(r3)                     # restore environment
        cmpobe  TRUE,g0,.getnextSM_400  # Jif region is dirty
.getnextSM_380:
        addo    1,r5,r5                 # inc. to next region #
        cmpoble r5,r4,.getnextSM_360    # Jif more regions to check
#
# --- No more dirty regions were detected.
#
        ldconst ccsm_termownrty,r5      # r5 = operation retry count
        stob    r5,cor_ocsert1(g3)      # set operation retry count
.if GR_GEORAID15_DEBUG
c fprintf(stderr,"%s%s:%u <GR><CCSM$GetNextM>generate done(Done) event to CCSM$OCTrans.....\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # GR_GEORAID15_DEBUG
        ldconst Done,g0                 # g0 = event type code
        call    CCSM$OCTrans            # generate event to O.C.S.E.
                                        # g0 = event type
                                        # g3 = COR address
        b       .getnextSM_1000
#
.getnextSM_400:
        call    CCSM$get_seqnum         # allocate a sequence # for message
                                        # g2 = sequence #
        stos    g2,cor_seqnum(g3)       # save seq. # in COR
        mov     r5,g0                   # g0 = region # to read
        st      r5,cor_tregnum(g3)      # save region # in COR
        ld      cor_respsn(g3),g4       # g4 = controller s/n to send message to
.if GR_GEORAID15_DEBUG
c fprintf(stderr,"%s%s:%u <GR><CCSM$GetNextM>Send pack and send Read Dirty Segment Map event to other controller\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # GR_GEORAID15_DEBUG
        call    CCSM$snd_readsm         # pack and send Read Dirty Segment Map
                                        #  message
        ldconst ccsm_ocse_to1,r4        # r4 = timeout value
        stob    r4,cor_ocseto(g3)       # set timer
        ldconst ccsm_readsmoprty,r5     # r5 = operation retry count
        stob    r5,cor_ocsert1(g3)      # set operation retry count
.getnextSM_1000:
        ret
#
#**********************************************************************
#
#  NAME: CCSM$ForceOwn
#
#  PURPOSE:
#
#       This routine performs the operations to force the copy ownership
#       to be changed to this node.
#
#  DESCRIPTION:
#
#       This routine packs and sends a Force Ownership Change message
#       to be sent to the master CCSM and resets the process retry
#       count to 1 and sets the O.C.S.E. timer.
#
#  CALLING SEQUENCE:
#
#       call    CCSM$ForceOwn
#
#  INPUT:
#
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       All regs. except g3 can be destroyed.
#
#**********************************************************************
#
CCSM$ForceOwn:
        call    CCSM$get_seqnum         # allocate a sequence # for message
                                        # g2 = sequence #
        stos    g2,cor_seqnum(g3)       # save seq. # in COR
        ldconst 0,g1                    # g1 = new sec. owner
.if GR_GEORAID15_DEBUG
        ldob    cor_rcsvd(g3),r5
        ldob    cor_rcdvd(g3),r4
c fprintf(stderr,"%s%s:%u <GR><CCSM$ForceOwn>..svid=%lx,dvid=%lx call CCSM$snd_force\n", FEBEMESSAGE, __FILE__, __LINE__,r5,r4);
.endif  # GR_GEORAID15_DEBUG
        call    CCSM$snd_force          # send Force Ownership Change message
                                        #  to master CCSM
        ldconst 1,r4
        stob    r4,cor_ocsert2(g3)      # set process retry count to 1
        ldconst ccsm_ocse_to1,r4        # r4 = timeout value
        stob    r4,cor_ocseto(g3)       # set timer
        ret
#
#**********************************************************************
#
#  NAME: CCSM$IAmOwner
#
#  PURPOSE:
#
#       This routine returns the indication to the caller that this
#       controller is the current owner of a copy.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$IAmOwner
#
#  INPUT:
#
#       g1 = return buffer address to place my controller serial #
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       All regs. except g3 can be destroyed.
#
#**********************************************************************
#
CCSM$IAmOwner:
        ld      K_ficb,r3               # FICB
        ld      fi_cserial(r3),r4       # g0 = my controller serial number
        st      r4,(g1)                 # save in return buffer
        ret
#
#**********************************************************************
#
#  NAME: CCSM$EndProcess
#
#  PURPOSE:
#
#       This routine performs the end of process cleanup operations
#       for the O.C.S.E. state machine.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$EndProcess
#
#  INPUT:
#
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       All regs. except g3 can be destroyed.
#
#**********************************************************************
#
CCSM$EndProcess:
.if GR_GEORAID15_DEBUG
        ldob    cor_rcsvd(g3),r5
        ldob    cor_rcdvd(g3),r6
c fprintf(stderr,"%s%s:%u <GR><action routine r4=CCSM$EndPRocess>svid=%lx,dvid=%lx End of process-cleanup ops\n", FEBEMESSAGE, __FILE__, __LINE__,r5,r6);
.endif  # GR_GEORAID15_DEBUG
        ldconst 0,r4
        stob    r4,cor_ocsert1(g3)
        stob    r4,cor_ocsert2(g3)
        stob    r4,cor_ocseto(g3)
        stos    r4,cor_seqnum(g3)
        ret
#
#**********************************************************************
#
#  NAME: CCSM$SuspCCSE
#
#  PURPOSE:
#
#       This routine performs an ownership suspended event to the
#       C.C.S.E. state machine to force it to setup the write
#       update handlers correctly for a copy operation that is
#       not owned by this controller.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$SuspCCSE
#
#  INPUT:
#
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       All regs. except g3 can be destroyed.
#
#**********************************************************************
#
CCSM$SuspCCSE:
        ldconst SuspendOwner,g0         # g0 = event type code
        call    CCSM$CCTrans            # generate event to C.C.S.E.
                                        # g0 = event type
                                        # g3 = COR address
        ret
#
#**********************************************************************
#
#  NAME: CCSM$ClrRMs
#
#  PURPOSE:
#
#       This routine clears the main and transfer ownership region maps
#       and any associated region map bitmaps. This routine will be used
#       mainly when a node gives up checking/getting/having copy ownership.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$ClrRMs
#
#  INPUT:
#
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       All regs. except g3 can be destroyed.
#
#**********************************************************************
#
CCSM$ClrRMs:
#
# --- Clear local region map bitmaps and region maps since we no longer
#       own the copy and thus the local resources no longer count.
#
        call    CM$dealloc_RM           # deallocate main region/segment map
        call    CM$dealloc_transRM      # deallocate transfer region/segment
                                        #  map
        ret
#
#**********************************************************************
#
#  NAME: CCSM$Clrflags
#
#  PURPOSE:
#
#       This routine clears the disable update flags in the cor_flags
#       This routine will be used mainly when a node gives up
#       checking/getting/having copy ownership.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$Clrflags
#
#  INPUT:
#
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       All regs. except g3 can be destroyed.
#
#**********************************************************************
#
CCSM$Clrflags:
        ldob    cor_flags(g3),r4        # r4 = cor_flags byte
        clrbit  CFLG_B_DIS_DEVICE,r4,r4 # Clear disable copy devices update
                                        #  flag
        clrbit  CFLG_B_DIS_STATE,r4,r4  # Clear disable copy state update flag
        clrbit  CFLG_B_DIS_GENERAL,r4,r4    # Clear disable copy info. update flag
        stob    r4,cor_flags(g3)        # save update cor_flags byte
        ret
#
#**********************************************************************
#
#  NAME: CCSM$ClrRM_flags
#
#  PURPOSE:
#
#       This routine combines the CCSM$ClrRMs & CCSM$Clrflags routines.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$ClrRM_flags
#
#  INPUT:
#
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       All regs. except g3 can be destroyed.
#
#**********************************************************************
#
CCSM$ClrRM_flags:
        call    CCSM$ClrRMs             # clear region maps
        call    CCSM$Clrflags           # clear disable update flags
        ret
#
#**********************************************************************
#
#  NAME: CCSM$LogAqrdOwnrshp
#
#  PURPOSE:
#
#       This routine logs a Acquired ownership event to the CCB.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$LogAqrdOwnrshp
#
#  INPUT:
#
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       All regs. except g3 can be destroyed.
#
#**********************************************************************
#
CCSM$LogAqrOwnrshp:
        ldconst cmcc_AqrdOwnrshp,g0      # g0 = Acquired Ownership log message
        call    CM_Log_Completion       # *** log message ***
c        DEF_ChgRAIDNotMirroringState_2((COR*)g3);
        ret
#
#**********************************************************************
#
#  NAME: CCSM$LogOwnrshpTerm
#
#  PURPOSE:
#
#       This routine logs a Ownership Terminated event to the CCB.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$LogOwnrshpTerm
#
#  INPUT:
#
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       All regs. except g3 can be destroyed.
#
#**********************************************************************
#
CCSM$LogOwnrshpTerm:
        ldconst cmcc_OwnrshpTerm,g0     # g0 = Ownership terminated log message
        call    CM_Log_Completion       # *** log message ***
        ret
#
#**********************************************************************
#
#  NAME: CCSM$LogForceOwnrshp
#
#  PURPOSE:
#
#       This routine logs a Force Ownership event to the CCB.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    CCSM$LogForceOwnrshp
#
#  INPUT:
#
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       All regs. except g3 can be destroyed.
#
#**********************************************************************
#
CCSM$LogOwnrshpForce:
        ldconst cmcc_ForceOwnrshp,g0    # g0 = Force Ownership log message
        call    CM_Log_Completion       # *** log message ***
        ret
#
# --- End of O.C.S.E. Event Routines ----------------------------------
#
#
# --- CCSM task trace routines ----------------------------------------
#
.if     ccsm_traces
#
#**********************************************************************
#
#  NAME: ccsm$ci_trace
#
#  PURPOSE:
#
#       This routine generate a trace record for the specified
#       client interface event.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    ccam$ci_trace
#
#  INPUT:
#
#       g0 = client interface event type <w>
#       g3 = assoc. COR address
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       No regs. destroyed.
#
#**********************************************************************
#
        .data
ci_update:
        .ascii  "Cupt"
ci_upause:
        .ascii  "Cups"
ci_apause:
        .ascii  "Caps"
ci_resume:
        .ascii  "Cres"
ci_mirror:
        .ascii  "Cmir"
ci_ended:
        .ascii  "Cend"
ci_reg:
        .ascii  "Creg"
ci_reg_sync:
        .ascii  "Crsy"
ci_reg_dirty:
        .ascii  "Crdy"
ci_mod_init:
        .ascii  "Cmoi"
ci_mod_done:
        .ascii  "Cmod"
ci_cd_moved:
        .ascii  "Ccdm"
ci_cs_chged:
        .ascii  "Ccsc"
ci_info_chged:
        .ascii  "Cinf"
ci_rpoll_term:
        .ascii  "Crpt"
#
        .text
ccsm$ci_trace:
#
# --- Trace record format -------------------------------------------------
#
#       <w> Trace record type code (passed from caller)
#       <w> assoc. COR address
#       <w> copy registration ID
#       <w> copy CM serial #
#
        ldq     ccsm_tr_flags,r4        # r4 = trace flags
                                        # r5 = current trace pointer
                                        # r6 = trace head pointer
                                        # r7 = trace tail pointer
        bbc     1,r4,.citrace_1000      # Jif trace disabled
        st      g0,(r5)                 # save trace record type code
        st      g3,4(r5)                # save assoc. COR address
        ld      cor_rid(g3),r8          # r8 = reg. ID
        ld      cor_rcsn(g3),r9         # r9 = CM serial #
        stl     r8,8(r5)                # save RID and CM serial #
        lda     16(r5),r5
        cmpobg  r7,r5,.citrace_200
        mov     r6,r5                   # set current pointer to head
.citrace_200:
        st      r5,ccsm_tr_cur
.citrace_1000:
        ret
#
.endif  # ccsm_traces
#
#**********************************************************************
#
#  NAME: ccsm$NVRAMrefresh
#
#  PURPOSE:
#
#       This routine refreshes NVRAM from disk.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    ccsm$NVRAMrefresh
#
#  INPUT:
#
#       None.
#
#  OUTPUT:
#
#       g1 = return status code
#
#  REGS DESTROYED:
#
#       Reg. g1 destroyed.
#
#**********************************************************************
#
ccsm$NVRAMrefresh:
        mov     g0,r12                  # save g0
        mov     g3,r13                  # save g3
        mov     g4,r14                  # save g4
        mov     g6,r10                  # save g6
        mov     g14,r15                 # save g14

c       g0 = s_MallocW(NVSRAMP2SIZ, __FILE__, __LINE__);
        mov     g0,r9                   # r9 = buffer for NVRAM refresh

        ldconst fidbenvram,r4           # FID
        mov     r4,g0                   # place in correct register
        mov     r9,g1                   # Pointer to buffer
        ldconst 1,g2                    # Length in blocks (just the header)
        ldconst 1,g3                    # Confirmation
        ldconst 1,g4                    # Start at block one
        ldconst 0,g6                    # Set pid bitmap to zero
        call    FS$MultiRead            # Read
        cmpobne 0,g0,.NVRrefsh_error    # Jif the read failed
#
# --- Now do the read for the proper amount of data.
#
        mov     r4,g0                   # restore FID
        mov     r9,g1                   # Pointer to buffer
        ld      12(g1),g2               # Length in bytes
        ldconst SECSIZE,r3              # Bump up a sector
        addo    r3,g2,g2
        divo    r3,g2,g2                # Block count (rounded)
        ldconst 1,g3                    # Confirmation
        ldconst 1,g4                    # Start at block one
        call    FS$MultiRead            # Read
        cmpobe  0,g0,.NVRrefsh_rcmplt   # Jif the read worked
#
# --- Error out of reads for file system operation.
#
.NVRrefsh_error:
        ldconst deioerr,r11             # Error code
        b       .NVRrefsh_return        # Exit
#
# --- read is complete. Check the checksum from the buffer.
#
.NVRrefsh_rcmplt:
        ldconst debadnvrec,r11          # Get possible error code set up
        mov     r9,g0                   # Address of data buffer
        call    NV_P2ChkSumChk          # Check P2 integrity
        cmpobne TRUE,g0,.NVRrefsh_return # JIf P2 error
#
# --- refresh NVRAM
#
        call    P6_LockAllCor           # Lock All Cors
        mov     r9,g0                   # Address of data buffer
        call    NV_RefreshNvram         # refresh configuration from NVRAM
        mov     ecok,r11                # r11 = completion code
        call    P6_UnlockAllCor         # Unlock All Cors

.NVRrefsh_return:
c       s_Free(r9, NVSRAMP2SIZ, __FILE__, __LINE__); # Free refresh buffer

        mov     r12,g0                  # restore g0
        mov     r13,g3                  # restore g3
        mov     r14,g4                  # restore g4
        mov     r10,g6                  # restore g6
        mov     r15,g14                 # restore g14
        mov     r11,g1                  # g1 = completion status
        ret
#
#**********************************************************************
#
#  NAME: CCSM$que
#
#  PURPOSE:
#
#       To provide a common means of queuing an event ILT
#       to the CCSM event queue.
#
#  DESCRIPTION:
#
#       The event ILT is queued to the tail of the CCSM executive queue.
#       The executive is activated to process this request. This routine
#       may be called from either the process or interrupt level.
#
#  CALLING SEQUENCE:
#
#       call    CCSM$que
#
#  INPUT:
#
#       g1 = event ILT
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
CCSM$que:
        lda     ccsm_sp_qu,r11          # Get queue origin
        b       K$cque
#
#**********************************************************************
#
#  NAME: CCSM$queswap
#
#  PURPOSE:
#
#       To provide a common means of queuing an event ILT
#       to the swap RAIDs CCSM event queue.
#
#  DESCRIPTION:
#
#       The event ILT is queued to the tail of the swap RAIDs CCSM
#       executive queue. The swap RAIDs executive is activated to
#       process this request. This routine may be called from either
#       the process or interrupt level.
#
#  CALLING SEQUENCE:
#
#       call    CCSM$queswap
#
#  INPUT:
#
#       g1 = event ILT
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
CCSM$queswap:
        lda     ccsmswap_sp_qu,r11      # Get queue origin
        b       K$cque
#
#**********************************************************************
#
#  NAME: CCSM$release_ilt
#
#  PURPOSE:
#
#       To provide a common means for CCSM to release an ILT.
#
#  DESCRIPTION:
#
#       The event ILT is released.
#
#  CALLING SEQUENCE:
#
#       call    CCSM$release_ilt
#
#  INPUT:
#
#       g1 = event ILT
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
CCSM$release_ilt:
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
        ret
#
#**********************************************************************
#
#  NAME: Constant functions
#
#  PURPOSE:
#
#       To provide a common means returning a value of the constants
#       in the state tables.
#
#  DESCRIPTION:
#
#  CALLING SEQUENCE:
#
#  INPUT:
#
#  OUTPUT:
#
#       g0 - the constant
#
#  REGS DESTROYED:
#
#       g0.
#
#**********************************************************************
#
        .globl CCSM_ST_CpyOwned
#
CCSM_ST_CpyOwned:
        ldconst s.ST_CpyOwned,g0
        ret
#
# --- CCBGram Templates ------------------------------------------------------
#
# --- CCBGram base "header" template
#
        .data
ccsm_event_base1:
        .word   0
        .word   0
        .word   0
        .word   CCSM$release_ilt
#
# --- New Master CCSM Define Event template
#
ccsm_event_new_master:
        .word   8                       # event length
        .byte   ccsm_et_define          # event type code
        .byte   ccsm_de_newm            # event function code
        .short  0                       # sequence #
        .word   0                       # sender's serial #
#
# --- No Longer Master CCSM Define Event template
#
ccsm_event_no_master:
        .word   8                       # event length
        .byte   ccsm_et_define          # event type code
        .byte   ccsm_de_nom             # event function code
        .short  0                       # sequence #
        .word   0                       # sender's serial #
#
# --- Start Copy Define Event template
#
ccsm_event_start:
        .word   12                      # event length
        .byte   ccsm_et_define          # event type code
        .byte   ccsm_de_start           # event function code
        .short  0                       # sequence #
        .word   0                       # sender's serial #
#
# --- Terminate Copy Define Event template
#
ccsm_event_term:
        .word   16                      # event length
        .byte   ccsm_et_define          # event type code
        .byte   ccsm_de_term            # event function code
        .short  0                       # sequence #
        .word   0                       # sender's serial #
#
# --- Pause Copy Define Event template
#
ccsm_event_pause:
        .word   16                      # event length
        .byte   ccsm_et_define          # event type code
        .byte   ccsm_de_pause           # event function code
        .short  0                       # sequence #
        .word   0                       # sender's serial #
#
# --- Resume Copy Define Event template
#
ccsm_event_resume:
        .word   16                      # event length
        .byte   ccsm_et_define          # event type code
        .byte   ccsm_de_resume          # event function code
        .short  0                       # sequence #
        .word   0                       # sender's serial #
#
# --- Swap RAIDs Define Event template
#
ccsm_event_swap:
        .word   24                      # event length
        .byte   ccsm_et_define          # event type code
        .byte   ccsm_de_swap            # event function code
        .short  0                       # sequence #
        .word   0                       # sender's serial #
#
# --- Configuration Change Occurred Define Event template
#
ccsm_event_cco:
        .word   8                       # event length
        .byte   ccsm_et_define          # event type code
        .byte   ccsm_de_cco             # event function code
        .short  0                       # sequence #
        .word   0                       # sender's serial #
#
# --- Copy Operational State Changed Define Event template
#
ccsm_event_cosc:
        .word   20                      # event length
        .byte   ccsm_et_define          # event type code
        .byte   ccsm_de_cosc            # event function code
        .short  0                       # sequence #
        .word   0                       # sender's serial #
#
# --- 1 Second Timer Define Event template
#
ccsm_event_timer:
        .word   12                      # event length
        .byte   ccsm_et_define          # event type code
        .byte   ccsm_de_timer           # event function code
        .short  0                       # sequence #
        .word   0                       # sender's serial #
#
        .text
#
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
