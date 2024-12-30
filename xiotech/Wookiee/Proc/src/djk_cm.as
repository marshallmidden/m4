# $Id: djk_cm.as 160802 2013-03-22 16:39:33Z marshall_midden $
#**********************************************************************
#
#  NAME: djk_cm.as
#
#  PURPOSE:
#
#       To provide the logic to support Copy Manager functions
#       and services.
#
#  FUNCTIONS:
#
#       CM$init2        - Copy manager initialization routine #2.
#       CM$regcopy      - Register/register copy operation with
#                         remote copy devices.
#       CM$termregcopy  - Terminate copy operation registration with
#                         remote copy devices.
#       CM$setsmtbl     - Allocate a segment mapping table and set all
#                         segment bits.
#       CM$cntremsegs   - Count the remaining # of segments that need
#                         to be copied.
#       CM$act_cm       - Activate CM.
#       CM$deact_cm     - Deactivate CM.
#       CM$act_cor      - Activate COR.
#       CM$deact_cor    - Deactivate COR.
#       CM$term_cor     - Terminate copy operation.
#       CM$act_scd      - Activate SCD.
#       CM$deact_scd    - Deactivate SCD.
#       CM$act_dcd      - Activate DCD.
#       CM$deact_dcd    - Deactivate DCD.
#       CM$scstart      - Start secondary copy.
#       CM$wp2_null     - Null phase 2 write update handler routine.
#       CM_wp2_copy     - Phase 2 write update handler routine when copying.
#       CM_wp2_mirror   - Phase 2 write update handler routine when mirrored.
#       CM$wp2_suspend  - Phase 2 write update handler routine when suspended.
#       CM$wp2_inactive - Phase 2 write update handler routine when copy task inactive.
#       CM$qsp          - Queue copy manager service provider request message.
#       CM$valid_cor    - Validate COR is still active with a specified
#                         copy operation ID.
#       CM$pksnd_local_poll - Pack and send a local poll request for the
#                               specified copy operation.
#       CM$update_rmap  - Update region/segment map.
#       CM$pkop_dmove   - Pack a Copy Device Moved datagram message
#       CM$whack_rcor   - Whack remote CORs associated with VLAR
#                         pack/send routine.
#       CM$find_cor_rid - Find a COR associated with a S/N,ID pair
#       CM$mmc_sflag    - Set/clear MMC copy active flag routine.
#       CM$restart_cor  - Restart copy operation routine.
#       CM$srcerr       - Process error on source copy device
#
#       This module employs n processes:
#
#       cm$spexec       - Copy Manager service provider process (one copy)
#       cm$pollexec     - Copy Operation poll process (one copy)
#
#  Copyright (c) 2001-2008 XIOtech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- global function declarations ------------------------------------
#
        .globl  CM$init2        # Copy manager initialization routine #2.
        .globl  CM$regcopy      # Register/register copy operation with
                                #  remote copy devices.
        .globl  CM$termregcopy  # Terminate copy operation registration with
                                #  remote copy devices.
        .globl  CM$setsmtbl     # Allocate a segment mapping table and
                                #  set all segment bits.
        .globl  CM$act_cm       # Activate CM.
        .globl  CM$deact_cm     # Deactivate CM.
        .globl  CM$act_cor      # Activate COR.
        .globl  CM$deact_cor    # Deactivate COR.
        .globl  CM$term_cor     # Terminate copy operation.
        .globl  CM$act_scd      # Activate SCD.
        .globl  CM$deact_scd    # Deactivate SCD.
        .globl  CM$act_dcd      # Activate DCD.
        .globl  CM$deact_dcd    # Deactivate DCD.
        .globl  CM$scstart      # Start secondary copy.
        .globl  cm$up1comp      # Service copy write update operations
        .globl  CM$wp2_null     # Null phase 2 write update handler routine.
        .globl  CM_wp2_copy     # Phase 2 write update handler routine when
                                #  copying.
        .globl  CM$wp2_suspend  # Phase 2 write update handler routine when
                                #  suspended.
        .globl  CM$wp2_inactive # Phase 2 write update handler routine when
                                #  copy task is inactive.
        .globl  CM$qsp          # Queue copy manager service provider
                                #  request message
        .globl  CM$valid_cor    # Validate COR is still active with a
                                #  specified copy operation ID.
        .globl  CM$pksnd_local_poll # Pack and send a local poll request for
                                #  the specified copy operation.
        .globl  CM$update_rmap  # Update region/segment map.
        .globl  CM$pkop_dmove   # Pack a Copy Device Moved datagram message
        .globl  CM$whack_rcor   # Whack remote CORs associated with VLAR
        .globl  CMsp$srvr_inuse # Specified copy device in use response
                                #  pack/send routine.
        .globl  CM$find_cor_rid # Find a COR associated with a S/N,ID pair
        .globl CM_find_cor_rid  # Find a COR associated with a S/N, ID pair
        .globl  CM$mmc_sflag    # Set/clear MMC copy active flag routine.
        .globl  CM$srcerr       # Process error on source copy device
        .globl  CM$get_cor_rid  # assign RID

        .globl  CM_act_cm       # Activate CM.
        .globl  CM_act_cor      # Activate COR.
        .globl  CM_act_scd      # Activate SCD.
        .globl  CM_deact_scd    # Deactivate SCD.
        .globl  CM_act_dcd      # Activate DCD.
        .globl  CM_deact_dcd    # Deactivate DCD.
        .globl  CM_wp2_null     # Null phase 2 write update handler routine.
        .globl  CM_wp2_mirror   # Phase 2 write update handler routine when
                                #  mirrored.
        .globl  CM_wp2_suspend  # Phase 2 write update handler routine when
                                #  suspended.
        .globl  CM_wp2_inactive#  Phase 2 write update handler routine when
                                #  copy task is inactive.

        .globl  CM_setsmtbl     # Allocate a segment mapping table and
                                #  set all segment bits.
        .globl  CM_cnt_smap     # count segment bits
        .globl  CM_mmc_sflag    # Set/clear MMC copy active flag routine.

#
#
# --- Define module resident routines
#
        .globl  D$p2updateconfig        # update configuration
        .globl  D$ctlrqst_cr            # PCP request ILT completion handler
                                        #  routine

#
# --- kernel resident routines
#
        .globl  K$twait                 # Process timed wait
        .globl  Malloc                  # Assign Cacheable Read DRAM memory
        .globl  K$fork                  # Process fork

#
# --- RAID resident routines ------------------------------------------
#

#
# --- linkepc resident routines ---------------------------------------
#

#
# --- data-link manager resident modules ------------------------------
#
        .globl  DLM$srvr_invfc          # Pack and return an invalid function
                                        #   code datagram response message
        .globl  DLM$srvr_invparm        # Pack and return an invalid parameter
                                        #  datagram response message
        .globl  DLM_srvr_ok             # Good completion datagram response
                                        #  message template
        .globl  DLM$get_dg              # Get datagram resources
        .globl  DLM$put_dg              # Put datagram resources
        .globl  DLM$send_dg             # Send datagram message
        .globl  DLM$reg_size            # Change VDisk size on peer MAGNITUDE
                                        #  (completion status returned to
                                        #   caller)
        .globl  DLM$chk_master          # Check if I'm the current group master
                                        #  controller. Return the current group
                                        #  master controller serial number if
                                        #  not.
        .globl  DLM$srvr_reroute        # Re-route datagram to specified
                                        #  controller response routine
#
# --- copy configuration and status manager resident modules -----------
#
        .globl  CCSM$update     # Update copy (% complete) (client I/F)
        .globl  CCSM$upause     # Copy user paused (client I/F)
        .globl  CCSM$apause     # Copy auto-paused (client I/F)
        .globl  CCSM$mirror     # Copy mirrored (client I/F)
        .globl  CCSM$ended      # Copy terminated (client I/F)
        .globl  CCSM$reg        # Copy registered (client I/F)
        .globl  CCSM$reg_sync   # Region synchronized (client I/F)
        .globl  CCSM$reg_dirty  # Region dirty (client I/F)
        .globl  CCSM$get_cwip   # Get CWIP record (client I/F)
        .globl  CCSM$put_cwip   # Put CWIP record (client I/F)
#
# --- global data declarations ----------------------------------------
#
        .globl  CM_cor_act_que          # The active COR queue
        .globl  CM_cm_act_que           # The active CM queue
        .globl  cm_cm_act_cnt           # CM active queue count
        .globl  cm_cor_act_cnt          # COR active queue count
        .globl  CM_proc_pri             # Priority for the CM process

        .globl  CM_cor_act_que          # The active COR queue
        .globl  CM_cm_act_que           # The active CM queue
        .globl  CM_proc_pri             # Priority for the CM process
        .globl  CM_cor_rid
        .globl  CM_cm_pri

#
# --- host resident data ----------------------------------------------
#

#
# --- kernel resident data --------------------------------------------
#
        .globl  K$qw                    # Queue request w/ wait

#
# --- global usage data definitions -----------------------------------
#
        .data
        .align  4                       # align just in case
#
#
#
# --- Copy manager service provider queue data structure
#
cm_sp_qu:
        .word   0                       # queue head pointer        <w>
        .word   0                       # queue tail pointer        <w>
        .word   0                       # queue count               <w>
        .word   0                       # associated pcb            <w>
cm_poll_pcb:
        .word   0                       # copy operation poll executive
                                        #  process PCB address
CM_cor_rid:
        .word   1                       # next copy registration ID
CM_cm_act_que:
        .word   0                       # list of active CMs
CM_cor_act_que:
        .word   0                       # list of active CORs
        .globl  cm_upret_head
cm_upret_head:                          # cm$up_common routine ILT return
        .word   0                       #  queue head pointer
        .globl  cm_upret_tail
cm_upret_tail:                          # cm$up_common routine ILT return
        .word   0                       #  queue tail pointer
        .globl  cm_upret_flag
cm_upret_flag:                          # cm$up_common routine ILT return
        .byte   FALSE                   #  loop flag (T/F)
cm_cm_act_cnt:
        .byte   0                       # number of CMs on active queue
cm_cor_act_cnt:
        .short  0                       # number of CORs on active queue
CM_proc_pri:
        .byte   CMPCMEXECPRI            # copy manager process priority
CM_cm_pri:
        .byte   (cmp_norm<<4)+vrnorm    # CM copy priority/strategy values
#
CM_rem_uperr:
        .short  0                       # remote copy update error count
CM_cor_label:
        .ascii  "                "      # Initial cor_label value
#
djk_save1:
        .word   0
#
#
#**********************************************************************
#
# _____________________ Datagram Request Templates ____________________
#
# --- Define New Copy Operation request header template
#
cm_opnew_hdr:
        .byte   dg_cpu_main             # server processor code         <b>
        .byte   dgrq_size               # request header length         <b>
        .short  0                       # message sequence #            <s>
        .byte   CMsp_fc_op_new          # request function code         <b>
        .byte   dg_path_any              # path HAB #                    <b>
        .byte   0                       # general purpose reg. #0       <b>
                                        # requested cluster #
        .byte   0                       # general purpose reg. #1       <b>
#
        .word   0                       # source serial #               <w>
        .word   0                       # destination serial #          <w>
#                                                               ******0x10****
        .ascii  "CMsp"                  # dest. server name             <w>
        .word   CMsp_rq_opnew_size      # remaining message length      <w>
#
        .byte   0                       # general purpose reg. #2       <b>
        .byte   0                       # general purpose reg. #3       <b>
        .byte   0                       # general purpose reg. #4       <b>
        .byte   0                       # general purpose reg. #5       <b>
#
        .word   0                       # request message header CRC    <w>
#                                                               ******0x20****
#
# --- Establish Copy Operation State request header template
#
cm_opstate_hdr:
        .byte   dg_cpu_main             # server processor code         <b>
        .byte   dgrq_size               # request header length         <b>
        .short  0                       # message sequence #            <s>
        .byte   CMsp_fc_op_state        # request function code         <b>
        .byte   dg_path_any              # path HAB #                    <b>
        .byte   0                       # general purpose reg. #0       <b>
                                        # requested cluster #
        .byte   0                       # general purpose reg. #1       <b>
#
        .word   0                       # source serial #               <w>
        .word   0                       # destination serial #          <w>
#                                                               ******0x10****
        .ascii  "CMsp"                  # dest. server name             <w>
        .word   CMsp_rq_opst_size       # remaining message length      <w>
#
        .byte   0                       # general purpose reg. #2       <b>
        .byte   0                       # general purpose reg. #3       <b>
        .byte   0                       # general purpose reg. #4       <b>
        .byte   0                       # general purpose reg. #5       <b>
#
        .word   0                       # request message header CRC    <w>
#                                                               ******0x20****
#
# --- Terminate Copy Operation request header template
#
cm_opterm_hdr:
        .byte   dg_cpu_main             # server processor code         <b>
        .byte   dgrq_size               # request header length         <b>
        .short  0                       # message sequence #            <s>
        .byte   CMsp_fc_op_term         # request function code         <b>
        .byte   dg_path_any              # path HAB #                    <b>
        .byte   0                       # general purpose reg. #0       <b>
                                        # requested cluster #
        .byte   0                       # general purpose reg. #1       <b>
#
        .word   0                       # source serial #               <w>
        .word   0                       # destination serial #          <w>
#                                                               ******0x10****
        .ascii  "CMsp"                  # dest. server name             <w>
        .word   CMsp_rq_optrm_size      # remaining message length      <w>
#
        .byte   0                       # general purpose reg. #2       <b>
        .byte   0                       # general purpose reg. #3       <b>
        .byte   0                       # general purpose reg. #4       <b>
        .byte   0                       # general purpose reg. #5       <b>
#
        .word   0                       # request message header CRC    <w>
#                                                               ******0x20****
#
# --- Check Copy Operation State/Status request header template
#
cm_opcheck_hdr:
        .byte   dg_cpu_main             # server processor code         <b>
        .byte   dgrq_size               # request header length         <b>
        .short  0                       # message sequence #            <s>
        .byte   CMsp_fc_op_check        # request function code         <b>
        .byte   dg_path_any              # path HAB #                    <b>
        .byte   0                       # general purpose reg. #0       <b>
                                        # requested cluster #
        .byte   0                       # general purpose reg. #1       <b>
#
        .word   0                       # source serial #               <w>
        .word   0                       # destination serial #          <w>
#                                                               ******0x10****
        .ascii  "CMsp"                  # dest. server name             <w>
        .word   CMsp_rq_opchk_size      # remaining message length      <w>
#
        .byte   0                       # general purpose reg. #2       <b>
        .byte   0                       # general purpose reg. #3       <b>
        .byte   0                       # general purpose reg. #4       <b>
        .byte   0                       # general purpose reg. #5       <b>
#
        .word   0                       # request message header CRC    <w>
#                                                               ******0x20****
#
# --- Maintain/Start Region Map Table request header template
#
cm_rmstart_hdr:
        .byte   dg_cpu_main             # server processor code         <b>
        .byte   dgrq_size               # request header length         <b>
        .short  0                       # message sequence #            <s>
        .byte   CMsp_fc_rm_start        # request function code         <b>
        .byte   dg_path_any              # path HAB #                    <b>
        .byte   0                       # general purpose reg. #0       <b>
                                        # requested cluster #
        .byte   0                       # general purpose reg. #1       <b>
#
        .word   0                       # source serial #               <w>
        .word   0                       # destination serial #          <w>
#                                                               ******0x10****
        .ascii  "CMsp"                  # dest. server name             <w>
        .word   CMsp_rq_rmst_size       # remaining message length      <w>
#
        .byte   0                       # general purpose reg. #2       <b>
        .byte   0                       # general purpose reg. #3       <b>
        .byte   0                       # general purpose reg. #4       <b>
        .byte   0                       # general purpose reg. #5       <b>
#
        .word   0                       # request message header CRC    <w>
#                                                               ******0x20****
#
# --- Suspend Region Map Table request header template
#
cm_rmsusp_hdr:
        .byte   dg_cpu_main             # server processor code         <b>
        .byte   dgrq_size               # request header length         <b>
        .short  0                       # message sequence #            <s>
        .byte   CMsp_fc_rm_susp         # request function code         <b>
        .byte   dg_path_any              # path HAB #                    <b>
        .byte   0                       # general purpose reg. #0       <b>
                                        # requested cluster #
        .byte   0                       # general purpose reg. #1       <b>
#
        .word   0                       # source serial #               <w>
        .word   0                       # destination serial #          <w>
#                                                               ******0x10****
        .ascii  "CMsp"                  # dest. server name             <w>
        .word   CMsp_rq_rmsusp_size     # remaining message length      <w>
#
        .byte   0                       # general purpose reg. #2       <b>
        .byte   0                       # general purpose reg. #3       <b>
        .byte   0                       # general purpose reg. #4       <b>
        .byte   0                       # general purpose reg. #5       <b>
#
        .word   0                       # request message header CRC    <w>
#                                                               ******0x20****
#
# --- Terminate Region Map Table request header template
#
cm_rmterm_hdr:
        .byte   dg_cpu_main             # server processor code         <b>
        .byte   dgrq_size               # request header length         <b>
        .short  0                       # message sequence #            <s>
        .byte   CMsp_fc_rm_term         # request function code         <b>
        .byte   dg_path_any              # path HAB #                    <b>
        .byte   0                       # general purpose reg. #0       <b>
                                        # requested cluster #
        .byte   0                       # general purpose reg. #1       <b>
#
        .word   0                       # source serial #               <w>
        .word   0                       # destination serial #          <w>
#                                                               ******0x10****
        .ascii  "CMsp"                  # dest. server name             <w>
        .word   CMsp_rq_rmterm_size     # remaining message length      <w>
#
        .byte   0                       # general purpose reg. #2       <b>
        .byte   0                       # general purpose reg. #3       <b>
        .byte   0                       # general purpose reg. #4       <b>
        .byte   0                       # general purpose reg. #5       <b>
#
        .word   0                       # request message header CRC    <w>
#                                                               ******0x20****
#
# --- Read Region/Segment Map Table request header template
#
cm_rmread_hdr:
        .byte   dg_cpu_main             # server processor code         <b>
        .byte   dgrq_size               # request header length         <b>
        .short  0                       # message sequence #            <s>
        .byte   CMsp_fc_rm_read         # request function code         <b>
        .byte   dg_path_any              # path HAB #                    <b>
        .byte   0                       # general purpose reg. #0       <b>
                                        # requested cluster #
        .byte   0                       # general purpose reg. #1       <b>
#
        .word   0                       # source serial #               <w>
        .word   0                       # destination serial #          <w>
#                                                               ******0x10****
        .ascii  "CMsp"                  # dest. server name             <w>
        .word   CMsp_rq_rmrd_size       # remaining message length      <w>
#
        .byte   0                       # general purpose reg. #2       <b>
        .byte   0                       # general purpose reg. #3       <b>
        .byte   0                       # general purpose reg. #4       <b>
        .byte   0                       # general purpose reg. #5       <b>
#
        .word   0                       # request message header CRC    <w>
#                                                               ******0x20****
#
# --- Clear Region/Segment Map Table request header template
#
cm_rmclear_hdr:
        .byte   dg_cpu_main             # server processor code         <b>
        .byte   dgrq_size               # request header length         <b>
        .short  0                       # message sequence #            <s>
        .byte   CMsp_fc_rm_clear        # request function code         <b>
        .byte   dg_path_any              # path HAB #                    <b>
        .byte   0                       # general purpose reg. #0       <b>
                                        # requested cluster #
        .byte   0                       # general purpose reg. #1       <b>
#
        .word   0                       # source serial #               <w>
        .word   0                       # destination serial #          <w>
#                                                               ******0x10****
        .ascii  "CMsp"                  # dest. server name             <w>
        .word   CMsp_rq_rmclr_size      # remaining message length      <w>
#
        .byte   0                       # general purpose reg. #2       <b>
        .byte   0                       # general purpose reg. #3       <b>
        .byte   0                       # general purpose reg. #4       <b>
        .byte   0                       # general purpose reg. #5       <b>
#
        .word   0                       # request message header CRC    <w>
#                                                               ******0x20****
#
# --- Check Region Map Table State/Status request header template
#
cm_rmcheck_hdr:
        .byte   dg_cpu_main             # server processor code         <b>
        .byte   dgrq_size               # request header length         <b>
        .short  0                       # message sequence #            <s>
        .byte   CMsp_fc_rm_check        # request function code         <b>
        .byte   dg_path_any              # path HAB #                    <b>
        .byte   0                       # general purpose reg. #0       <b>
                                        # requested cluster #
        .byte   0                       # general purpose reg. #1       <b>
#
        .word   0                       # source serial #               <w>
        .word   0                       # destination serial #          <w>
#                                                               ******0x10****
        .ascii  "CMsp"                  # dest. server name             <w>
        .word   CMsp_rq_rmchk_size      # remaining message length      <w>
#
        .byte   0                       # general purpose reg. #2       <b>
        .byte   0                       # general purpose reg. #3       <b>
        .byte   0                       # general purpose reg. #4       <b>
        .byte   0                       # general purpose reg. #5       <b>
#
        .word   0                       # request message header CRC    <w>
#                                                               ******0x20****
#
# --- Suspend Copy Operation request header template
#
cm_opsusp_hdr:
        .byte   dg_cpu_main             # server processor code         <b>
        .byte   dgrq_size               # request header length         <b>
        .short  0                       # message sequence #            <s>
        .byte   CMsp_fc_op_susp         # request function code         <b>
        .byte   dg_path_any              # path HAB #                    <b>
        .byte   0                       # general purpose reg. #0       <b>
                                        # requested cluster #
        .byte   0                       # general purpose reg. #1       <b>
#
        .word   0                       # source serial #               <w>
        .word   0                       # destination serial #          <w>
#                                                               ******0x10****
        .ascii  "CMsp"                  # dest. server name             <w>
        .word   CMsp_rq_opsusp_size     # remaining message length      <w>
#
        .byte   0                       # general purpose reg. #2       <b>
        .byte   0                       # general purpose reg. #3       <b>
        .byte   0                       # general purpose reg. #4       <b>
        .byte   0                       # general purpose reg. #5       <b>
#
        .word   0                       # request message header CRC    <w>
#                                                               ******0x20****
#
# --- Resume Copy Operation request header template
#
cm_opresume_hdr:
        .byte   dg_cpu_main             # server processor code         <b>
        .byte   dgrq_size               # request header length         <b>
        .short  0                       # message sequence #            <s>
        .byte   CMsp_fc_op_resume       # request function code         <b>
        .byte   dg_path_any              # path HAB #                    <b>
        .byte   0                       # general purpose reg. #0       <b>
                                        # requested cluster #
        .byte   0                       # general purpose reg. #1       <b>
#
        .word   0                       # source serial #               <w>
        .word   0                       # destination serial #          <w>
#                                                               ******0x10****
        .ascii  "CMsp"                  # dest. server name             <w>
        .word   CMsp_rq_opresm_size     # remaining message length      <w>
#
        .byte   0                       # general purpose reg. #2       <b>
        .byte   0                       # general purpose reg. #3       <b>
        .byte   0                       # general purpose reg. #4       <b>
        .byte   0                       # general purpose reg. #5       <b>
#
        .word   0                       # request message header CRC    <w>
#                                                               ******0x20****
#
# --- Copy Device Moved request header template
#
cm_opdmove_hdr:
        .byte   dg_cpu_main             # server processor code         <b>
        .byte   dgrq_size               # request header length         <b>
        .short  0                       # message sequence #            <s>
        .byte   CMsp_fc_op_dmove        # request function code         <b>
        .byte   dg_path_any              # path HAB #                    <b>
        .byte   0                       # general purpose reg. #0       <b>
                                        # requested cluster #
        .byte   0                       # general purpose reg. #1       <b>
#
        .word   0                       # source serial #               <w>
        .word   0                       # destination serial #          <w>
#                                                               ******0x10****
        .ascii  "CMsp"                  # dest. server name             <w>
        .word   CMsp_rq_opdmov_size     # remaining message length      <w>
#
        .byte   0                       # general purpose reg. #2       <b>
        .byte   0                       # general purpose reg. #3       <b>
        .byte   0                       # general purpose reg. #4       <b>
        .byte   0                       # general purpose reg. #5       <b>
#
        .word   0                       # request message header CRC    <w>
#                                                               ******0x20****
#
#**********************************************************************
#
# _____________________ Datagram Response Templates ___________________
#
#
# --- Destination server/VLink not established to specified
#       source copy device response template
#
CMsp_srvr_nosvlink:
        .byte   dg_st_srvr              # request completion status
        .byte   dgrs_size               # response header length
        .short  0                       # message sequence #
        .byte   dgec1_srvr_nosvlink     # error code #1
        .byte   0                       # error code #2
        .byte   0                       # general purpose reg. #0
        .byte   0                       # general purpose reg. #1
        .word   0                       # remaining message length
        .word   0                       # response message header CRC
#
# --- Destination server/VLink not established to specified
#       destination copy device response template
#
CMsp_srvr_nodvlink:
        .byte   dg_st_srvr              # request completion status
        .byte   dgrs_size               # response header length
        .short  0                       # message sequence #
        .byte   dgec1_srvr_nodvlink     # error code #1
        .byte   0                       # error code #2
        .byte   0                       # general purpose reg. #0
        .byte   0                       # general purpose reg. #1
        .word   0                       # remaining message length
        .word   0                       # response message header CRC
#
# --- Destination server/Specified copy device in use response template
#
CMsp_srvr_inuse:
        .byte   dg_st_srvr              # request completion status
        .byte   dgrs_size               # response header length
        .short  0                       # message sequence #
        .byte   dgec1_srvr_inuse        # error code #1
        .byte   0                       # error code #2
        .byte   0                       # general purpose reg. #0
        .byte   0                       # general purpose reg. #1
        .word   0                       # remaining message length
        .word   0                       # response message header CRC
#
# --- Destination server/Specified copy operation not defined response
#       template
#
CMsp_srvr_nocopy:
        .byte   dg_st_srvr              # request completion status
        .byte   dgrs_size               # response header length
        .short  0                       # message sequence #
        .byte   dgec1_srvr_nocopy       # error code #1
        .byte   0                       # error code #2
        .byte   0                       # general purpose reg. #0
        .byte   0                       # general purpose reg. #1
        .word   0                       # remaining message length
        .word   0                       # response message header CRC
#
# --- Destination server/Dirty region/segment map table response
#       template
#
CMsp_srvr_dirty:
        .byte   dg_st_srvr              # request completion status
        .byte   dgrs_size               # response header length
        .short  0                       # message sequence #
        .byte   dgec1_srvr_dirty        # error code #1
        .byte   0                       # error code #2
        .byte   0                       # general purpose reg. #0
        .byte   0                       # general purpose reg. #1
        .word   0                       # remaining message length
        .word   0                       # response message header CRC
#
# --- Destination server/Region map state still active response
#       template
#
CMsp_srvr_rmact:
        .byte   dg_st_srvr              # request completion status
        .byte   dgrs_size               # response header length
        .short  0                       # message sequence #
        .byte   dgec1_srvr_rmact        # error code #1
        .byte   0                       # error code #2
        .byte   0                       # general purpose reg. #0
        .byte   0                       # general purpose reg. #1
        .word   0                       # remaining message length
        .word   0                       # response message header CRC
#
# --- Insufficient Resources response template
#
CMsp_srvr_insres:
        .byte   dg_st_srvr              # request completion status
        .byte   dgrs_size               # response header length
        .short  0                       # message sequence #
        .byte   dgec1_srvr_insres       # error code #1
        .byte   0                       # error code #2
        .byte   0                       # general purpose reg. #0
        .byte   0                       # general purpose reg. #1
        .word   0                       # remaining message length
        .word   0                       # response message header CRC
#
# --- executable code -------------------------------------------------
        .text
#**********************************************************************
#
#  NAME: CM$regcopy
#
#  PURPOSE:
#       This routine registers a copy process with the remote copy
#       device(s) as needed.
#
#  DESCRIPTION:
#       This routine either registers a newly created copy operation
#       with the remote copy device(s) or reregisters a copy
#       operation with the remote copy device(s) as specified by
#       the calling routine.
#       Note: This routine will stall the calling routine waiting for
#             datagram responses from remote MAGNITUDES housing
#             remote copy devices.
#
#  CALLING SEQUENCE:
#       call    CM$regcopy
#
#  INPUT:
#       g1 = register type code
#            00 = newly register copy operation
#            01 = reregister copy operation
#       g3 = COR address of copy session to register
#
#  OUTPUT:
#       g0 = registration request status code
#            00 = registration successful (copy operation registered)
#            01 = registration unsuccessful (copy operation not
#                    registered but may still be valid)
#            02 = registration unsuccessful (copy operation not
#                    valid; terminate immediately)
#
#  REGS DESTROYED:
#       Reg. g0 destroyed.
#
#**********************************************************************
#
CM$regcopy:
        movq    g0,r12                  # save g1-g3
        ldconst 01,r12                  # r12 = starting registration request
                                        #       status code
        ld      cor_rcsn(g3),r4         # r4 = copy MAG (me) serial number
        ld      cor_rssn(g3),r5         # r5 = source MAG serial number
        ld      cor_rdsn(g3),r6         # r6 = dest. MAG serial number
        cmpobne.f 0,g1,.regcopy_500     # Jif not newly register copy op.
#
# --- Register new copy operation process ------------------------------------
#
#
# --- Check if source device capacity is different then the
#       destination device capacity. If they are, need to make
#       them the same. If this fails, the copy operation cannot
#       be registered.
#
        ld      cor_srcvdd(g3),r7       # r7 = source VDD address
        ld      cor_destvdd(g3),r8      # r8 = dest. VDD address
        cmpobe.f 0,r7,.regcopy_09       # Jif no source VDD defined
        cmpobe.f 0,r8,.regcopy_09       # Jif no dest. VDD defined
c       if (((VDD*)r7)->devCap == ((VDD*)r8)->devCap) {
        b       .regcopy_09             # Jif src & dst capacity the same
c       }
        ld      vd_scdhead(r8),r3       # r3 = first SCD assoc. with dest. VDD
        cmpobe.t 0,r3,.regcopy_03       # Jif no SCDs assoc. with dest. VDD
.regcopy_02:
        ldconst 02,r12                  # r12 = registration request status
                                        #  indicating unsuccessful.
                                        #  (copy operation not valid)
        b       .regcopy_1000

.regcopy_03:
c       ((VDD*)r8)->devCap = ((VDD*)r7)->devCap;    # Make dst cap match src.
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        cmpobe.t r4,r6,.regcopy_08      # Jif dest. copy device on local MAG
#
# --- Destination copy device is remote. Adjust local VDD size and attempt
#       to change the VDisk size at the remote MAGNITUDE. If successful,
#       save the changes to configuration and continue with the copy operation
#       registration. If unsuccessful, registration is unsuccessful and
#       copy operation is not valid.
#
        mov     g4,r3                   # save g4
        mov     r8,g4                   # g4 = VDD address to change size
                                        #  on remote MAGNITUDE
        ldl     vd_devcap(r8),r10       # r10/r11 = saved dest. dev. cap.
        call    DLM$reg_size            # change VDisk size on remote MAG
                                        # g0 = change size completion status
        mov     r3,g4                   # restore g4
        cmpobe.t TRUE,g0,.regcopy_09    # Jif size change on remote MAG was
                                        #  successful
        stl     r10,vd_devcap(r8)       # restore orig. cap. in dest. VDD
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        b       .regcopy_02             # and indicate copy operation not
                                        #  valid back to requestor
#
# --- Destination copy device is either local or it was changed
#       successfully on the remote MAGNITUDE. Update configurations
#       to save the changes.
#
.regcopy_08:
        PushRegs(r3)                    # save environment
        call    P6_Update_Config        # update configuration
        PopRegsVoid(r3)                 # restore environment

.regcopy_09:
#
# --- Check if source copy device is remote
#
        cmpobe.t r4,r5,.regcopy_100     # Jif source copy device is not remote
#
# --- Register the copy operation on the remote MAGNITUDE that
#       houses the source copy device. If the remote source copy
#       device is housed in the same MAGNITUDE as the remote
#       destination copy device, both will be registered during this
#       phase.
#
        mov     r5,g0                   # g0 = MAG serial # to register new
                                        #      copy operation on
        call    cm$pkop_new             # pack a Define New Copy Operation
                                        #  datagram
                                        # g1 = datagram ILT at nest level 2
        lda     DLM$send_dg,g0          # g0 = datagram service provider
                                        #  routine
        call    K$qw                    # Queue request w/wait
        ld      dsc2_rshdr_ptr(g1),r7   # r7 = local response header address
        lda     -dsc1_ulvl(g1),g1       # g1 = datagram ILT at nest level 1
        ldob    dgrs_status(r7),r8      # r8 = request completion status
        ldob    dgrs_ec1(r7),r9         # r9 = error code byte #1
        call    DLM$put_dg              # deallocate datagram ILT
        cmpobe.t dg_st_ok,r8,.regcopy_100 # Jif no error reported on request
        cmpobne.f dg_st_srvr,r8,.regcopy_20 # Jif not dest. server level
                                        #  error
#
# --- Destination server level error returned
#
        cmpobe.t dgec1_srvr_invparm,r9,.regcopy_1000 # Jif invalid
                                        #  parameter error reported
.regcopy_10:
        ldconst 02,r12                  # r12 = unsuccessful status; copy
                                        #  operation not valid; terminate
                                        #  immediately status code
        b       .regcopy_1000

.regcopy_20:
        cmpobne.t dg_st_ddsp,r8,.regcopy_1000 # Jif not dest. datagram
                                        #  service provider level error
#
# --- Destination datagram service provider level error returned
#
        cmpobe.t dgec1_ddsp_nosrvr,r9,.regcopy_10 # Jif requested server not
                                        #  defined
        b       .regcopy_1000

.regcopy_100:
#
# --- Check if the destination copy device is remote
#
        cmpobe.t r4,r6,.regcopy_200     # Jif dest. copy device is not
                                        #  remote
#
# --- Check if source copy device and destination copy device reside
#       on the same MAGNITUDE. If the source copy device was remote,
#       the previous logic would have registered both the source
#       copy device and destination copy device of the copy operation.
#
        cmpobe.t r5,r6,.regcopy_200     # Jif source copy device and dest.
                                        #  copy device are on the same MAG
#
# --- Register the copy operation on the remote MAGNITUDE that
#       houses the destination copy device.
#
        mov     r6,g0                   # g0 = MAG serial # to register new
                                        #      copy operation on
        call    cm$pkop_new             # pack a Define New Copy Operation
                                        #  datagram
                                        # g1 = datagram ILT at nest level 2
        lda     DLM$send_dg,g0          # g0 = datagram service provider
                                        #  routine
        call    K$qw                    # Queue request w/wait
        ld      dsc2_rshdr_ptr(g1),r7   # r7 = local response header address
        lda     -dsc1_ulvl(g1),g1       # g1 = datagram ILT at nest level 1
        ldob    dgrs_status(r7),r8      # r8 = request completion status
        ldob    dgrs_ec1(r7),r9         # r9 = error code byte #1
        call    DLM$put_dg              # deallocate datagram ILT
        cmpobe.t dg_st_ok,r8,.regcopy_200 # Jif no error reported on request
        cmpobne.f dg_st_srvr,r8,.regcopy_120 # Jif not dest. server level
                                        #  error
#
# --- Destination server level error returned
#
        cmpobe.t dgec1_srvr_invparm,r9,.regcopy_1000 # Jif invalid
                                        #  parameter error reported
        b       .regcopy_10             # all other errors fatal

.regcopy_120:
        cmpobne.t dg_st_ddsp,r8,.regcopy_1000 # Jif not dest. datagram
                                        #  service provider level error
#
# --- Destination datagram service provider level error returned
#
        cmpobe.t dgec1_ddsp_nosrvr,r9,.regcopy_10 # Jif requested server not
                                        #  defined
        b       .regcopy_1000

.regcopy_200:
        ldconst 00,r12                  # r12 = successful registration
                                        #      request status code
        b       .regcopy_1000           # and return the good news to the
                                        #  requesting routine
#
# --- Re-register copy operation process -------------------------------------
#
.regcopy_500:
        ldconst 00,r12                  # r12 = successful registration
                                        #      request status code
.regcopy_1000:
        movq    r12,g0                  # g0 = registration request status
                                        #      code
                                        # restore g1-g3
        ret
#
#**********************************************************************
#
#  NAME: CM$termregcopy
#
#  PURPOSE:
#       This routine terminates a copy operation registration with the
#       remote copy device(s).
#
#  DESCRIPTION:
#       This routine does the necessary processing to terminate a copy
#       operation registration at the remote copy device(s).
#
#  CALLING SEQUENCE:
#       call    CM$termregcopy
#
#  INPUT:
#       g3 = COR address of copy session to terminate registration
#            with the remote copy device(s).
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
CM$termregcopy:
        movq    g0,r12                  # save g0-g3
        ld      cor_rcsn(g3),r4         # r4 = copy MAG (me) serial number
        ld      cor_rssn(g3),r5         # r5 = source MAG serial number
        ld      cor_rdsn(g3),r6         # r6 = dest. MAG serial number
#
# --- Check if source copy device is remote
#
        cmpobe.t r4,r5,.termreg_100     # Jif source copy device is not remote
#
# --- Terminate the copy operation on the remote MAGNITUDE that
#       houses the source copy device. If the remote source copy
#       device is housed in the same MAGNITUDE as the remote
#       destination copy device, both will be terminated during this
#       phase.
#
        mov     r5,g0                   # g0 = MAG serial # to terminate copy operation with
        call    cm$pkop_term            # pack a Terminate Copy Operation datagram
                                        # g1 = datagram ILT at nest level 2
        ldconst 4,g0                    # g0 = datagram error retry count
        call    DLM$just_senddg         # send out datagram message without waiting for response
.termreg_100:
#
# --- Check if the destination copy device is remote
#
        cmpobe.t r4,r6,.termreg_1000    # Jif dest. copy device is not
                                        #  remote
#
# --- Check if source copy device and destination copy device reside
#       on the same MAGNITUDE. If the source copy device was remote,
#       the previous logic would have terminated both the source
#       copy device and destination copy device of the copy operation.
#
        cmpobe.t r5,r6,.termreg_1000    # Jif source copy device and dest.
                                        #  copy device are on the same MAG
#
# --- Terminate the copy operation on the remote MAGNITUDE that
#       houses the destination copy device.
#
        mov     r6,g0                   # g0 = MAG serial # to terminate copy operation with
        call    cm$pkop_term            # pack a Terminate Copy Operation datagram
                                        # g1 = datagram ILT at nest level 2
        ldconst 4,g0                    # g0 = datagram error retry count
        call    DLM$just_senddg         # send out datagram message without waiting for response
.termreg_1000:
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: CM$init2
#
#  PURPOSE:
#       This routine contains the copy manager initialization #2
#       logic.
#
#  DESCRIPTION:
#       This routine contains the logic to initialize copy manager
#       services for djk.
#
#  CALLING SEQUENCE:
#       call    CM$init2
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
CM$init2:
        movl    g0,r12                  # save g0-g1
#
# --- Establish Copy Manager service provider executive process
#
        lda     cm$spexec,g0            # Establish copy manager service
                                        #  provider executive process
        ldconst CMSPEXECPRI,g1
c       CT_fork_tmp = (ulong)"cm$spexec";
        call    K$fork
        st      g0,cm_sp_qu+qu_pcb      # Save PCB
#
# --- Establish Copy Operation poll executive process
#
        lda     cm$pollexec,g0          # Establish copy operation poll
                                        #  executive process
        ldconst CMPOLLEXECPRI,g1
c       CT_fork_tmp = (ulong)"cm$pollexec";
        call    K$fork
        st      g0,cm_poll_pcb          # Save PCB
#
# --- Check if extended NVRAM resides on this hardware platform.
#       FINISH @@@
#
#        ld      NVRAMLEN,r4             # r4 = length of NVRAM
#        ldconst 0x80000,r5              # r5 = length with extended NVRAM
#        cmpobge.t r4,r5,.init2_1000     # Jif extended NVRAM present
#        ldconst p4_maxcors,r4           # set active COR count to the max.
#        stos    r4,cm_cor_act_cnt
# .init2_1000:
        movl    r12,g0                  # restore g0-g1
        ret
#
#**********************************************************************
#
#  NAME: CM$setsmtbl
#
#  PURPOSE:
#       Allocates memory for a copy operation segment mapping table
#       and sets all the bits in it.
#
#  DESCRIPTION:
#       Allocates memory to be used for a segment mapping table and
#       initializes it for the calling routine by setting all the bits
#       in it.
#
#  CALLING SEQUENCE:
#       call    CM$setsmtbl
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       g1 = allocated segment mapping table address
#
#  REGS DESTROYED:
#       Reg. g1 destroyed.
#
#**********************************************************************
#
CM$setsmtbl:
c       g1 = get_sm();                  # Allocate a segment map table
.ifdef M4_DEBUG_SM
c fprintf(stderr, "%s%s:%u get_sm 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_SM
        ldconst REGSIZE_seg,r3          # r3 = dirty segment count
        st      r3,SM_cnt(g1)           # save dirty segment count
c       memset((void *)(g1+SM_tbl), 0xff, SMTBLsize);
        ret
#
#**********************************************************************
#
#  NAME: CM_setsmtbl
#
#  PURPOSE:
#       The following is a C envelope for the CM$setsmtbl routine
#
#  CALLING SEQUENCE:
#       call    CM_setsmtbl
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       g0 = SM table address if allocated
#       g0 = 0 if not able to allocate a SM table
#
#  REGS DESTROYED:
#       Reg. g0 destroyed.
#
#**********************************************************************
#
CM_setsmtbl:
        mov     g1,r12                  # save g1
        call    CM$setsmtbl             # get a set sm table
        mov     g1,g0                   # place in correct register
        mov     r12,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: CM$act_cm
#
#  PURPOSE:
#       Activates the specified CM.
#
#  DESCRIPTION:
#       Puts the specified CM on the active CM queue.
#
#  CALLING SEQUENCE:
#       call    CM$act_cm
#
#  INPUT:
#       g4 = CM address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
CM$act_cm:
        ld      CM_cm_act_que,r4        # r4 = top CM on active queue
        st      r4,cm_link(g4)          # link new CM onto top of active
                                        #  queue
        st      g4,CM_cm_act_que        # save new head of CM active queue

        ldob    cm_cm_act_cnt,r5        # r5 = current active CM count
        addo    1,r5,r5                 # increment active cm count
        stob    r5,cm_cm_act_cnt        # save updated count
        ret
#
#**********************************************************************
#
#  NAME: CM_act_cm
#
#  PURPOSE:
#       The following is a C envelope for the CM$act_cm routine
#
#  CALLING SEQUENCE:
#       call    CM_act_cm
#
#  INPUT:
#       g0 = CM address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None:
#
#**********************************************************************
#
CM_act_cm:
        mov     g4,r12                  # save g4
        mov     g0,g4                   # place in correct register
        call    CM$act_cm               # activate a cm
        mov     r12,g4                  # restore g4
        ret
#
#**********************************************************************
#
#  NAME: CM$deact_cm
#
#  PURPOSE:
#       Deactivates the specified CM.
#
#  DESCRIPTION:
#       Deactivates the specified CM by finding and removing the CM
#       from the active CM queue.
#
#  CALLING SEQUENCE:
#       call    CM$deact_cm
#
#  INPUT:
#       g4 = CM address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
CM$deact_cm:
        lda     CM_cm_act_que,r4        # r4 = CM active queue base
        mov     r4,r5                   # r5 = last CM processed on queue
.deactcm_100:
        ld      cm_link(r4),r4          # r4 = next CM on active queue
        cmpobe.f g4,r4,.deactcm_200     # Jif specified CM found on active
                                        #  queue
        mov     r4,r5                   # r5 = last CM processed on queue
        cmpobne.t 0,r4,.deactcm_100     # Jif not the end of active queue
        b       .deactcm_1000           # Jif not found on active queue

.deactcm_200:
        ld      cm_link(g4),r6          # r6 = next CM after CM being
                                        #  deactivated
        st      r6,cm_link(r5)          # unlink specified CM from queue

        ldob    cm_cm_act_cnt,r5        # r5 = current active CM count
        subo    1,r5,r5                 # decrement active cm count
        stob    r5,cm_cm_act_cnt        # save updated count
.deactcm_1000:
        ret
#
#**********************************************************************
#
#  NAME: CM$act_cor
#
#  PURPOSE:
#       Activates the specified COR.
#
#  DESCRIPTION:
#       Puts the specified COR on the end of the active COR queue.
#
#  CALLING SEQUENCE:
#       call    CM$act_cor
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
CM$act_cor:
        lda     CM_cor_act_que,r4       # r4 = COR active queue base
.actcor_100:
        ld      cor_link(r4),r7         # r7 = next COR on active queue
        cmpobe.f 0,r7,.actcor_200       # Jif end of queue found
        mov     r7,r4                   # r4 = last COR found on active queue
        b       .actcor_100             # and check the next COR on list

.actcor_200:
        st      r7,cor_link(g3)         # clear link list field in new COR
        st      g3,cor_link(r4)         # link new COR to last member found
                                        #  on the active list
        ldos    cm_cor_act_cnt,r5       # r5 = current active COR count
        addo    1,r5,r5                 # increment active cor count
        stos    r5,cm_cor_act_cnt       # save updated count
        ret
#
#**********************************************************************
#
#  NAME: CM_act_cor
#
#  PURPOSE:
#       The following is a C envelope for the CM$act_cor routine
#
#  CALLING SEQUENCE:
#       call    CM_act_cor
#
#  INPUT:
#       g0 = COR address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None:
#
#**********************************************************************
#
CM_act_cor:
        mov     g3,r12                  # save g3
        mov     g0,g3                   # place in correct register
        call    CM$act_cor              # activate a cor
        mov     r12,g3                  # restore g3
        ret
#
#**********************************************************************
#
#  NAME: CM$deact_cor
#
#  PURPOSE:
#       Deactivates the specified COR.
#
#  DESCRIPTION:
#       Deactivates the specified COR by finding and removing the COR
#       from the active COR queue.
#
#  CALLING SEQUENCE:
#       call    CM$deact_cor
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
CM$deact_cor:
        lda     CM_cor_act_que,r4       # r4 = COR active queue base
        mov     r4,r5                   # r5 = last COR processed on queue
.deactcor_100:
        ld      cor_link(r4),r4         # r4 = next COR on active queue
        cmpobe.f g3,r4,.deactcor_200    # Jif specified COR found on active
                                        #  queue
        mov     r4,r5                   # r5 = last COR processed on queue
        cmpobne.t 0,r4,.deactcor_100    # Jif not the end of active queue
        b       .deactcor_1000          # Jif not found on active queue

.deactcor_200:
        ld      cor_link(g3),r6         # r6 = next COR after COR being
                                        #  deactivated
        st      r6,cor_link(r5)         # unlink specified COR from queue

        ldos    cm_cor_act_cnt,r5       # r5 = current active COR count
        subo    1,r5,r5                 # decrement active cor count
        stos    r5,cm_cor_act_cnt       # save updated count
.deactcor_1000:
        ret
#
#**********************************************************************
#
#  NAME: CM$term_cor
#
#  PURPOSE:
#       Terminates the specified copy operation.
#
#  DESCRIPTION:
#       Performs all of the logic to terminate the specified
#       copy operation including:
#
#       - remove COR from active COR queue.
#       - deallocate region map and any segment maps as necessary.
#       - deallocate state NVRAM record if necessary.
#       - remove CM from active CM queue if necessary.
#       - deallocate CM if necessary.
#       - remove SCD from association with VDD if necessary.
#       - deallocate SCD if necessary.
#       - remove DCD from association with VDD if necessary.
#       - deallocate DCD if necessary.
#       - deallocate COR.
#
#       Note: This routine assumes that all related operations
#             to the specified copy operation have completed and
#             that all related copy operation resources can be
#             dispensed. It also assumes that the copy operation
#             has been unregistered on all associated nodes where
#             appropriate.
#             This routine DOES NOT update configuration with these changes!
#
#  CALLING SEQUENCE:
#       call    CM$term_cor
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
CM$term_cor:
        movl    g0,r14                  # save g0-g1
        mov     g4,r13                  # save g4
        ld      cor_cm(g3),g4           # g4 = assoc. CM address
        ld      cor_destvdd(g3),r4      # r4 = assoc. dest. VDD address
        cmpobe  0,r4,.termcor_50        # Jif no dest. VDD defined
#      c fprintf(stderr, "<CM$term_cor>Calling GR_UpdateGeoInfoAtCmdMgrReq()...\n");
c       GR_UpdateGeoInfoAtCmdMgrReq ((COR*)g3,(UINT8)MVCXSPECCOPY);
        ldconst vdnomirror,r5           # r5 = no mirror status
        stob    r5,vd_mirror(r4)        # update dest. VDD mirror status
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
.termcor_50:
        call    CM$deact_cor            # deactivate COR
        call    CM$dealloc_RM           # deallocate main region map table and
                                        #  segment map tables if necessary
        call    CM$dealloc_transRM      # deallocate transfer region/segment
                                        #  map tables if necessary
        PushRegs(r3)                    # Save all "g" registers and g14 = 0
        call    P6_DeallocStRec         # deallocate state NVRAM record for
                                        #  this copy
        PopRegsVoid(r3)                 # restore environment

        mov     0,r3                    # r3 = 0

        cmpobe.f 0,g4,.termcor_100      # Jif no CM defined for this COR
        call    CM$deact_cm             # deactivate CM
.ifdef M4_DEBUG_CM
c fprintf(stderr, "%s%s:%u put_cm 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g4);
.endif # M4_DEBUG_CM
c       put_cm(g4);                     # Deallocate a CM data structure
.termcor_100:
        st      r3,cor_cm(g3)           # remove CM from COR
        ld      cor_scd(g3),g0          # g0 = assoc. SCD address
        cmpobe.f 0,g0,.termcor_200      # Jif no SCD associated with COR
        call    CM$deact_scd            # deactivate SCD
.ifdef M4_DEBUG_SCD
c fprintf(stderr, "%s%s:%u put_scd 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif # M4_DEBUG_SCD
c       put_scd(g0);                    # Deallocate a SCD data structure
.termcor_200:
        st      r3,cor_scd(g3)          # remove SCD from COR
        st      r3,cor_srcvdd(g3)       # remove source copy device VDD
                                        #  from COR
        ld      cor_dcd(g3),g0          # g0 = assoc. DCD address
        cmpobe.f 0,g0,.termcor_300      # Jif no DCD associated with VDD
        call    CM$deact_dcd            # deactivate DCD
.ifdef M4_DEBUG_DCD
c fprintf(stderr, "%s%s:%u put_dcd 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif # M4_DEBUG_DCD
c       put_dcd(g0);                    # Deallocate a DCD data structure
.termcor_300:
        call    cm$Abort_RCC            # Abort any RCC assoc. with COR
        st      r3,cor_dcd(g3)          # remove DCD from COR
        st      r3,cor_destvdd(g3)      # remove dest. copy device VDD
                                        #  from COR
.ifdef M4_DEBUG_COR
c fprintf(stderr, "%s%s:%u put_cor 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g3);
.endif # M4_DEBUG_COR
c       put_cor(g3);                    # Deallocate COR data structure
        mov     r13,g4                  # restore g4
        movl    r14,g0                  # restore g0-g1
        ret
#
#**********************************************************************
#
#  NAME: CM$act_scd
#        CM_act_scd
#
#  PURPOSE:
#       Activates the specified SCD.
#
#  DESCRIPTION:
#       Activates the specified SCD by placing it on the active
#       SCD queue for the associated VDD.
#
#  CALLING SEQUENCE:
#       call    CM$act_scd
#               CM_act_scd
#
#  INPUT:
#       g0 = SCD address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
CM$act_scd:
CM_act_scd:
        ldconst 0,r3
        ld      scd_vdd(g0),r13         # r13 = assoc. VDD address
        st      r3,scd_link(g0)         # clear forward link list field
        cmpobe.f 0,r13,.actscd_1000     # Jif no VDD assoc. with SCD
#
# ---   Check for duplicate entry in the SCD list
#
        ld      vd_scdhead(r13),r8     # Get scd head of the vdd
.actscd_10:
        cmpobe  r8,g0,.actscd_1000      # scd is already present in the list
        cmpobe  0,r8,.actscd_20         # End of list
        ld      scd_link(r8),r8         # Get next SCD
        b       .actscd_10

.actscd_20:
        ld      vd_scdtail(r13),r4      # r4 = SCD list tail member
        ldconst vdvdscd,r7              # r7 = VDisk is source copy device
                                        #  flag
        ldos    vd_attr(r13),r6         # r6 = vd_attr
        cmpobne.f 0,r4,.actscd_100      # Jif SCD list not empty
        st      g0,vd_scdhead(r13)      # save SCD as list head member
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        b       .actscd_200

.actscd_100:
        st      g0,scd_link(r4)         # link SCD onto end of list
.actscd_200:
        st      g0,vd_scdtail(r13)      # save new SCD as list tail member
        or      r7,r6,r6                # set VDisk is source copy device
                                        #  flag
        stos    r6,vd_attr(r13)         # save updated vd_attr
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
.actscd_1000:
        ret
#
#**********************************************************************
#
#  NAME: CM$deact_scd
#        CM_deact_scd
#
#  PURPOSE:
#       Deactivates the specified SCD.
#
#  DESCRIPTION:
#       Deactivates the specified SCD by finding and removing the SCD
#       from the active SCD queue in the associated source copy device
#       VDD data structure.
#
#  CALLING SEQUENCE:
#       call    CM$deact_scd
#               CM_deact_scd
#
#  INPUT:
#       g0 = SCD address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
CM$deact_scd:
CM_deact_scd:
        ld      scd_vdd(g0),r4          # r4 = assoc. VDD address
        mov     0,r3
        cmpobe.f 0,r4,.deactscd_1000    # Jif no VDD assoc. with SCD
        lda     vd_scdhead(r4),r5       # r5 = SCD being checked
        mov     r5,r6                   # r6 = last SCD checked
.deactscd_100:
        ld      scd_link(r5),r5         # r5 = next SCD on active list
        cmpobe.f g0,r5,.deactscd_200    # Jif SCD found on active list
        mov     r5,r6                   # r6 = last SCD checked
        cmpobne.t 0,r5,.deactscd_100    # Jif more SCDs on active list
        b       .deactscd_1000          # return if not found on the
                                        #  active list
.deactscd_200:
        ld      scd_link(r5),r7         # r7 = next SCD on list
        st      r7,scd_link(r6)         # remove SCD from list
        cmpobne.t 0,r7,.deactscd_1000   # Jif not the last SCD on list
        ld      vd_scdhead(r4),r8       # r8 = first SCD on list
        cmpobne.t 0,r8,.deactscd_300    # Jif last SCD not removed from list
        mov     r7,r6                   # set up to clear list tail member
        ldos    vd_attr(r4),r9          # r9 = vd_attr
        ldconst vdvdscd,r10             # r10 = VDisk is source copy device
                                        #  flag
        andnot  r10,r9,r9               # clear flag in attribute
        stos    r9,vd_attr(r4)          # save updated attribute byte
.deactscd_300:
        st      r6,vd_scdtail(r4)       # save new list tail member
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
.deactscd_1000:
        st      r3,scd_vdd(g0)          # remove VDD from SCD
        st      r3,scd_link(g0)         # clear forward link field in SCD
        ret
#
#**********************************************************************
#
#  NAME: CM$act_dcd
#        CM_act_dcd
#
#  PURPOSE:
#       Activates the specified DCD.
#
#  DESCRIPTION:
#       Activates the specified DCD by placing it on the active
#       DCD queue for the associated VDD.
#
#  CALLING SEQUENCE:
#       call    CM$act_dcd
#               CM_act_dcd
#
#  INPUT:
#       g0 = DCD address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
CM$act_dcd:
CM_act_dcd:
        ld      dcd_vdd(g0),r14         # r14 = assoc. VDD address
        cmpobe.f 0,r14,.actdcd_1000     # Jif no VDD assoc. with DCD
        ldconst vdvddcd,r6              # r6 = VDisk is destination copy
                                        #  device flag
        ldos    vd_attr(r14),r5         # r5 = vd_attr
        st      g0,vd_dcd(r14)          # save DCD address in dest. copy VDD
        or      r6,r5,r5                # set VDisk is destination copy
                                        #  device flag
        stos    r5,vd_attr(r14)         # save updated vd_attr
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        mov     g3,r12                  # save g3
        ld      dcd_cor(g0),g3          # g3 = assoc. COR address
        call    CM$mmc_sflag            # process suspended flag in VDI
        mov     r12,g3                  # restore g3
.actdcd_1000:
        ret
#
#**********************************************************************
#
#  NAME: CM$deact_dcd
#        CM_deact_dcd
#
#  PURPOSE:
#       Deactivates the specified DCD.
#
#  DESCRIPTION:
#       Deactivates the specified DCD by finding and removing the DCD
#       from the active DCD queue in the associated source copy device
#       VDD data structure.
#
#  CALLING SEQUENCE:
#       call    CM$deact_dcd
#               CM_deact_dcd
#  INPUT:
#       g0 = DCD address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
CM$deact_dcd:
CM_deact_dcd:
        ld      dcd_vdd(g0),r15         # r15 = assoc. VDD address
        mov     0,r3
        cmpobe.f 0,r15,.deactdcd_1000   # Jif no VDD assoc. with DCD
        ld      vd_dcd(r15),r4          # r4 = active DCD for this VDD
        cmpobne.f g0,r4,.deactdcd_1000  # Jif the active DCD not the
                                        #  specified DCD
        ldconst vdvddcd+vdvdsusp,r7     # r7 = VDisk is destination copy
                                        #  device flag + suspended flag
        ldos    vd_attr(r15),r6         # r6 = attribute byte
        st      r3,vd_dcd(r15)          # remove DCD from VDD
        andnot  r7,r6,r6                # clear VDisk is destination copy
                                        #  device flag
        stos    r6,vd_attr(r15)         # save updated attribute byte
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
.deactdcd_1000:
        st      r3,dcd_vdd(g0)          # remove VDD from DCD
        ret
#
#**********************************************************************
#
#  NAME: CM$mmc_sflag
#        CM_mmc_sflags
#
#  PURPOSE:
#       Sets/clears the MMC suspended flag in the vi_attribute byte
#       of the destination copy device VDI.
#
#  DESCRIPTION:
#       This routine checks if a destination copy device VDD is defined
#       for the specified COR. If not, it returns to the caller. If it
#       does, it checks the cor_crstate byte to see if it is active. If
#       it is, it clears the suspended flag in the vi_attribute byte of
#       the destination copy device VDI. If it is not active, it sets the
#       suspended flag in the vi_attribute byte of the destination copy
#       device VDI.
#
#  CALLING SEQUENCE:
#       call    CM$mmc_sflag
#               CM_mmc_sflags
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
CM$mmc_sflag:
CM_mmc_sflag:
        ld      cor_destvdd(g3),r15     # r15 = assoc. dest. VDD address
        cmpobe.f 0,r15,.mmcsflag_1000   # Jif no dest. VDD address defined

        ldob    cor_crstate(g3),r4      # r4 = cor_crstate byte
        ldos    vd_attr(r15),r5         # r5 = assoc. vi_attribute byte
        ldconst vdvdsusp,r6             # r6 = suspended flag byte
.if GR_GEORAID15_DEBUG
        ldos    vd_vid(r15),r7
c fprintf(stderr,"%s%s:%u <GR><mmc_sflag-djkcm>vid=%lx crcstate=%lx vdd_attr=%lx\n", FEBEMESSAGE, __FILE__, __LINE__,r7,r4,r5);
.endif  # GR_GEORAID15_DEBUG
        cmpobne.t corcrst_active,r4,.mmcsflag_400 # Jif COR state not active

        andnot  r6,r5,r5                # clear suspended flag in vi_attribute
        b       .mmcsflag_500

.mmcsflag_400:
        or      r6,r5,r5                # set suspended flag in vi_attribute

.mmcsflag_500:
        stos    r5,vd_attr(r15)         # save updated vi_attribute byte
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
.if GR_GEORAID15_DEBUG
c fprintf(stderr,"%s%s:%u <GR><mmc_sflag-2-djkcm>vid=%lx crcstate=%lx vdd_attr=%lx\n", FEBEMESSAGE, __FILE__, __LINE__,r7,r4,r5);
.endif  # GR_GEORAID15_DEBUG

.mmcsflag_1000:
        ret
#
#**********************************************************************
#
#  NAME: CM$scstart
#
#  PURPOSE:
#       To provide a means of starting a secondary copy operation.
#
#  DESCRIPTION:
#       This routine expects the caller to check for the following:
#       - Destination copy device not already a destination copy device
#         in a secondary copy process.
#       - Source copy device is in an operational state.
#       - Destination copy device is in a proper state to be copied to.
#       - Destination copy device size is >= source copy device size.
#
#       Note: This routine can block the calling routine if resources
#               are not sufficient.
#
#  CALLING SEQUENCE:
#       call    CM$scstart
#
#  INPUT:
#       g0 = copy type code
#       g1 = source copy device VDD address
#       g2 = destination copy device VDD address
#       g3 = mirror type code
#
#  OUTPUT:
#       g0 = request status code
#
#  REGS DESTROYED:
#       Reg. g0 destroyed.
#
#**********************************************************************
#
CM$scstart:
        mov     g4,r11                  # save g4
        movq    g0,r12                  # save g0-g3
                                        # r12 = copy type code
                                        # r13 = source copy device VDD address
                                        # r14 = destination copy device
                                        #       VDD address
                                        # r15 = mirror type code
#
# --- Try to allocate a COR --------------------------------------------------
#
.if CM_IM_DEBUG
c       if( r12 == cmty_mirror)fprintf(stderr,"<CM_IM>djkcm-CM$scstart-Calling CM$get_cor\n");
.endif  # CM_IM_DEBUG
c       g3 = get_cor();                 # Allocate a COR data structure
.ifdef M4_DEBUG_COR
c fprintf(stderr, "%s%s:%u get_cor 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g3);
.endif # M4_DEBUG_COR
        st      r11,cor_rid(g3)         # save RID
        ldconst corcst_uni,r4           # r4 = COR copy state code
        stob    r4,cor_cstate(g3)       # save COR copy state code
        ldconst corcrst_undef,r5        # r5 = COR copy reg. state
        stob    r5,cor_crstate(g3)      # save COR copy reg. state
.if CM_IM_DEBUG
c       if( r12 == cmty_mirror)fprintf(stderr,"<CM_IM>djkcm-CM$scstart-Setting cstate,crstate as uninitialize(0), undefined(0)\n");
.endif  # CM_IM_DEBUG
        st      r13,cor_srcvdd(g3)      # save source VDD address in COR
        st      r14,cor_destvdd(g3)     # save dest. VDD address in COR
        ldq     CM_cor_label,r4         # r4-r7 = initial cor_label value
        stq     r4,cor_label(g3)        # save initial cor_label value
# VIJAY -- r12 contains cmty_copyswap for copy/swap not mvccopyswap-- TBD
# VIJAY -- luckily both have the same value.. However to change it..
#
        cmpobne mvccopyswap,r12,.scst50g    # HAZARD with cmty_copyswap
        ldconst TRUE,r5
        stob    r5,cor_userswap(g3)     # Indicate this as User requested swap

.scst50g:
#       Number of segments for device.
c       r8 = ((((VDD*)r13)->devCap + SEGSIZE_sec - 1) / SEGSIZE_sec) >> (((VDD*)r13)->devCap >> 32);
        st      r8,cor_totalsegs(g3)    # save total # segments in COR
#
# --- Allocate a copy registration ID to use for this copy operation ---------
#
.if CM_IM_DEBUG
c       if( r12 == cmty_mirror)fprintf(stderr,"<CM_IM>djkcm-CM$scstart-calling CM$get_cor_rid\n");
.endif  # CM_IM_DEBUG
        call    CM$get_cor_rid          # get a copy reg. ID to use
                                        # g0 = copy reg. ID to use
        st      g0,cor_rid(g3)          # save copy reg. ID in COR
        ld      K_ficb,r3               # r3 = FICB address
        ld      fi_vcgid(r3),r3         # r3 = my serial #
        st      r3,cor_rcsn(g3)         # save my MAG serial # as copy
                                        #  MAG serial # in COR
        mov     0,r7
        lda     vdcopyautopause,r4      # r4 = vdd mirror status
        stob    r7,vd_scpcomp(r14)      # set copy % comp. to 0%
        ldos    vd_vid(r13),r7
        stos    r7,vd_scorvid(r14)      # save source VDD vid in
                                        #  dest. copy device VDI
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>djkcm-setting autopause in vd_mirror\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # CM_IM_DEBUG
        stob    r4,vd_mirror(r14)       # save VDD mirror state of dest.
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
                                        #  copy device VDD
        ldob    vd_vid(r13),r4          # r4 = source copy device VDisk
                                        #      number
        ldob    vd_vid+1(r13),r5        # r5 = source copy device cluster
                                        #      number
        stob    r4,cor_rcsvd(g3)        # save source copy device VDisk
                                        #  number in COR
        stob    r5,cor_rcscl(g3)        # save source copy device cluster
                                        #  number in COR
        ld      vd_rdd(r13),r7          # r7 = RDD address of first RAID
        ldob    rd_type(r7),r6          # r6 = RAID type code
        cmpobne.t rdlinkdev,r6,.scst100 # Jif not a VLink type device

        ld      rd_psd(r7),r6           # r6 = assoc. PSD address
        ldos    ps_pid(r6),r7           # r7 = Physical ID
        ld      DLM_lddindx[r7*4],r7    # r7 = LDD address
        ldob    ld_class(r7),r8         # r8 = linked device class
        cmpobne.f ldmld,r8,.scst100     # Jif not a MAGNITUDE link device type
#
# --- Check if alternate ID defined for this LDD.
#
        ld      ld_basesn(r7),r3        # r3 = source MAG serial #
        ldos    ld_altid(r7),r4         # r4 = alternate ID
        cmpobe  0,r4,.scst90            # Jif no alternate ID defined
#
# --- Alternate Id defined for this VLink
#
        shro    8,r4,r5                 # r5 = MSB of alternate ID
        clrbit  7,r5,r5                 # clear alternate ID flag bit
        b       .scst100

.scst90:
        ldob    ld_basevd(r7),r4        # r4 = source VDisk number
        ldob    ld_basecl(r7),r5        # r5 = source cluster number
.scst100:
        st      r3,cor_rssn(g3)         # save source MAG serial #
        stob    r4,cor_rsvd(g3)         # save source VDisk number
        stob    r5,cor_rscl(g3)         # save source cluster number
        ldob    vd_vid(r14),r4          # r4 = dest. copy device VDisk
                                        #  number
        ldob    vd_vid+1(r14),r5        # r5 = dest. copy device cluster
                                        #  number
        stob    r4,cor_rcdvd(g3)        # save dest. copy device VDisk
                                        #  number in COR
        stob    r5,cor_rcdcl(g3)        # save dest. copy device cluster
                                        #  number in COR
        ld      K_ficb,r3               # r3 = FICB address
        ld      fi_vcgid(r3),r3         # r3 = my serial #
        ld      vd_rdd(r14),r7          # r7 = RDD address of first RAID
        ldob    rd_type(r7),r6          # r6 = RAID type code
        cmpobne.t rdlinkdev,r6,.scst200 # Jif not a VLink type device

        ld      rd_psd(r7),r6           # r6 = assoc. PSD address
        ldos    ps_pid(r6),r7           # r7 = Physical ID
        ld      DLM_lddindx[r7*4],r7    # r7 = LDD address
        ldob    ld_class(r7),r8         # r8 = linked device class
        cmpobne.f ldmld,r8,.scst200     # Jif not a MAGNITUDE link device type

        ld      ld_basesn(r7),r3        # r3 = source MAG serial #
        ldos    ld_altid(r7),r4         # r4 = alternate ID
        cmpobe  0,r4,.scst190           # Jif no alternate ID defined
#
# --- Alternate Id defined for this VLink
#
        shro    8,r4,r5                 # r5 = MSB of alternate ID
        clrbit  7,r5,r5                 # clear alternate ID flag bit
        b       .scst200

.scst190:
        ldob    ld_basevd(r7),r4        # r4 = source VDisk number
        ldob    ld_basecl(r7),r5        # r5 = source cluster number
.scst200:
        st      r3,cor_rdsn(g3)         # save dest. MAG serial #
        stos    r4,cor_rdvd(g3)         # save dest. VDisk number
        stob    r5,cor_rdcl(g3)         # save dest. cluster number
#
# --- Set up DCD -------------------------------------------------------------
#
c       g0 = get_dcd();                 # Allocate a DCD data structure
.ifdef M4_DEBUG_DCD
c fprintf(stderr, "%s%s:%u get_dcd 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif # M4_DEBUG_DCD
        st      g3,dcd_cor(g0)          # save COR address in DCD
        st      g0,cor_dcd(g3)          # save DCD address in COR
        st      r14,dcd_vdd(g0)         # save dest. VDD address in DCD
        ldconst dcdt_both,r4            # r4 = dcd_type code
        ld      cor_rcsn(g3),r5         # r5 = copy MAG serial #
        ld      cor_rdsn(g3),r6         # r6 = dest. copy device MAG serial #
        cmpobe.t r5,r6,.scst300         # Jif dest. copy device is both local
                                        #  and remote dest. copy device
        ldconst dcdt_local,r4           # r4 = dcd_type code

.scst300:
        stob    r4,dcd_type(g0)         # save dcd_type code
        lda     CM$wp2_null,r4          # r4 = phase 2 write update handler
                                        #      routine
        st      r4,dcd_p2hand(g0)       # save phase 2 write update handler
                                        #  routine in DCD
        mov     g0,g1                   # g1 = DCD address
#
# --- Set up SCD -------------------------------------------------------------
#
c       g0 = get_scd();                 # Allocate a SCD data structure
.ifdef M4_DEBUG_SCD
c fprintf(stderr, "%s%s:%u get_scd 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif # M4_DEBUG_SCD
        st      g3,scd_cor(g0)          # save COR address in SCD
        st      g0,cor_scd(g3)          # save SCD address in COR
        st      r13,scd_vdd(g0)         # save source VDD address in SCD
        ldconst scdt_both,r4            # r4 = scd_type code
        ld      cor_rcsn(g3),r5         # r5 = copy MAG serial #
        ld      cor_rssn(g3),r6         # r6 = source copy device MAG serial #
        cmpobe.t r5,r6,.scst400         # Jif source copy device is both local
                                        #  and remote source copy device
        ldconst scdt_local,r4           # r4 = scd_type code

.scst400:
        stob    r4,scd_type(g0)         # save scd_type code
        lda     CM$wp2_null,r4          # r4 = phase 2 write update handler
                                        #      routine
        st      r4,scd_p2hand(g0)       # save phase 2 write update handler
                                        #  routine in SCD
        ldconst CCtbl,r8                # r8 = C.C.S.E. state table
        st      r8,cor_ccseptr(g3)      # save C.C.S.E. state table pointer
        ldconst OCtbl,r8                # r8 = O.C.S.E. state table
        st      r8,cor_ocseptr(g3)      # save O.C.S.E. state table pointer
        ldconst 0,r8
        st      r8,cor_ccsecev(g3)
        st      r8,cor_ocsecev(g3)
        stob    r8,cor_ostindx(g3)
        stob    r8,cor_cstindx(g3)
#
# --- Activate SCD, DCD and COR ----------------------------------------------
#
        call    CM$act_scd              # activate SCD
        mov     g1,g0                   # g0 = DCD address to activate
        call    CM$act_dcd              # activate DCD
#        call    CM$act_cor              # activate COR
        ldconst cmcst_NRS,r8            # r8 = default CM copy state
.ifdef M4_DEBUG_CM
c fprintf(stderr, "%s%s:%u get_cm 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g4);
.endif # M4_DEBUG_CM
c       g4 = get_cm();                  # Allocate a CM data structure

        ldob    CM_cm_pri,r9            # r9 = CM copy priority code

        st      g3,cm_cor(g4)           # save COR address in CM
        st      g4,cor_cm(g3)           # save CM address in COR

        stob    r12,cm_type(g4)         # save copy type code in CM
        stob    r15,cm_mtype(g4)        # save mirror type code in CM
.if CM_IM_DEBUG
c       if( r12 == cmty_mirror)fprintf(stderr,"<VM_IM>djkcm-CM$scstart-setting cmcstate as no resources\n");
.endif  # CM_IM_DEBUG
        stob    r8,cm_cstate(g4)        # save CM copy state
        stob    r9,cm_pri(g4)           # save CM copy priority code
        ld      cor_totalsegs(g3),r8    # get total # segments from COR
        st      r8,cm_totalsegs(g4)     # save total # segments in CM
#
# --- Put  CM on active queues -----------------------------------------------
#
        call    CM$act_cm               # activate CM
.if CM_IM_DEBUG
c       if( r12 == cmty_mirror)fprintf(stderr,"<CM_IM>djkcm-fork CM$pexec/generate start copy evnt to CCSM(call CCSM$start_copy\n");
.endif  # CM_IM_DEBUG
#
# --- Schedule Copy Manager task to handler this copy operation --------------
#
        lda     CM$pexec,g0             # Establish copy executive process
        ldob    CM_proc_pri,g1          # g1 = copy process priority
c       CT_fork_tmp = (ulong)"CM$pexec";
        call    K$fork
        st      g0,cm_pcb(g4)           # Save PCB in CM
        ldconst pcnrdy,r3               # r3 = not ready status for PCB
        stob    r3,pc_stat(g0)          # set CM task to not ready
#
# --- Generate Start Copy event to CCSM
#
        call    CCSM$start_copy
        ldconst deok,r12                # return good status

        mov     r11,g4                  # restore g4
        movq    r12,g0                  # restore g0-g3
        ret

#**********************************************************************
#
#  NAME: CM$qsp
#
#  PURPOSE:
#       To provide a common means of queuing datagram service requests
#       to the copy manager service provider.
#
#  DESCRIPTION:
#       The datagram message is queued to the tail of the copy manager
#       service provider executive queue. The executive is activated to
#       process this request. This routine may be called from either the
#       process or interrupt level.
#
#  CALLING SEQUENCE:
#       call    CM$qsp
#
#  INPUT:
#       g1 = datagram ILT at nest level #4
#            nest level #3 contains dss3_* data structure as defined
#              in the dspif.inc file
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
CM$qsp:
        lda     cm_sp_qu,r11            # Get queue origin
        b       K$cque
#
#**********************************************************************
#
#  NAME: CM$valid_cor
#
#  PURPOSE:
#       Validate a COR is still active for a specified copy operation.
#
#  DESCRIPTION:
#       This routine checks if the specified COR address is still on the
#       active COR list. If not, it returns a FALSE indicator to the
#       calling routine. If it is, it validates the specified cor_rid
#       and cor_rcsn match the specified values signifying the COR
#       still pertains to the same copy operation. If either value
#       does not match, it returns a FALSE indicator to the calling
#       routine. If they still match, it returns a TRUE indicator to
#       the calling routine.
#
#  CALLING SEQUENCE:
#       call    CM$valid_cor
#
#  INPUT:
#       g0 = cor_rid value to check for
#       g1 = cor_rcsn value to check for
#       g3 = COR address to validate
#
#  OUTPUT:
#       g0 = FALSE if COR no longer active or is associated with
#               a different copy operation
#       g0 = TRUE if COR still active and is associated with the
#               same copy operation
#
#  REGS DESTROYED:
#       Reg. g0 destroyed.
#
#**********************************************************************
#
CM$valid_cor:
        mov     g0,r14                  # r14 = cor_rid value to check
        ldconst FALSE,g0                # g0 = FALSE return code
#
# --- Check if specified COR is on the active COR list.
#
        ld      CM_cor_act_que,r4       # r4 = first COR on active queue
        cmpobe.f 0,r4,.validcor_1000    # Jif no CORs on active queue
.validcor_100:
        cmpobe.f g3,r4,.validcor_300    # Jif COR matches
        ld      cor_link(r4),r4         # r4 = next COR on active COR list
        cmpobne.t 0,r4,.validcor_100    # Jif more active CORs to check
        b       .validcor_1000          # return FALSE indicator to calling
                                        #  routine
.validcor_300:
#
# --- Specified COR still resident on active COR queue.
#       Check if COR still pertains to the specified copy operation.
#
        ldl     cor_rid(g3),r4          # r4 = cor_rid
                                        # r5 = cor_rcsn
        cmpobne.f r14,r4,.validcor_1000 # Jif cor_rid does not match
        cmpobne.f g1,r5,.validcor_1000  # Jif cor_rcsn does not match
        ldconst TRUE,g0                 # g0 = TRUE return code
.validcor_1000:
        ret
#
#**********************************************************************
#
#  NAME: cm$spexec
#
#  PURPOSE:
#       To provide a means of processing Copy Manager service provider
#       requests which have been previously queued to the Copy Manager
#       service provider queue.
#
#  DESCRIPTION:
#       The queuing routine CM$qsp deposits a Copy Manager datagram
#       request into the queue and activates this executive if necessary.
#       This executive extracts the next datagram request from the queue
#       and directs it to the appropriate Copy Manager service provider
#       handler routine based on the datagram request function code.
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
# --- Set this process to not ready
#
.spex10:
        ldconst pcnrdy,r4               # Set this process to not ready
        stob    r4,pc_stat(r15)
#
# --- Exchange processes ----------------------------------------------
#
cm$spexec:
        call    K$qxchang               # Exchange processes
#
# --- Get next queued request
#
        lda     cm_sp_qu,r11            # Get Copy Manager service provider
                                        #  queue pointer
        ldq     qu_head(r11),r12        # r12 = queue head pointer
                                        # r13 = queue tail pointer
                                        # r14 = queue count
                                        # r15 = PCB address
        mov     r12,g1                  # g1 = datagram ILT being processed
                                        #  at nest level #4
        cmpobe.f 0,r12,.spex10          # Jif none
#
# --- Remove this request from queue ----------------------------------
#
        ld      il_fthd(r12),r12        # r12 = next ILT on queue
        cmpo    0,r12                   # Check for queue now empty
        subo    1,r14,r14               # Adjust queue count
        sele    r13,r12,r13             # Set up queue tail
        stt     r12,qu_head(r11)        # Update queue head, tail and count
        be.f    .spex30                 # Jif queue now empty
#
        st      r11,il_bthd(r12)        # Update backward thread
#
.spex30:
        ldl     -ILTBIAS+dss3_rqhdr_ptr(g1),g2 # g2 = local request message
                                        #             header address
                                        # g3 = local response message header
                                        #      address
        ldq     -ILTBIAS+dss3_rqbuf(g1),g4 # g4 = request message buffer addr.
                                        #      (does NOT include header)
                                        # g5 = request message length
                                        #      (does NOT include header)
                                        # g6 = response message buffer address
                                        #      (does NOT include header)
                                        # g7 = response buffer length
                                        #      (does NOT include header)
        ldob    dgrq_fc(g2),r4          # r4 = request function code
        ldconst CMsp$maxfc,r5           # r5 = max. valid function code
        cmpobge.t r5,r4,.spex100        # Jif valid request function code
#
# --- Invalid request function code. Return error to requestor.
#
        call    DLM$srvr_invfc          # pack and return invalid function
                                        #  code response to requestor
        b       cm$spexec               # and check for more work
#
# --- Valid request function code.
#
.spex100:
        shlo    2,r4,r4                 # r4 = function code * 4
        ld      CMsp$hand(r4),r4        # r4 = request handler routine
#
#******************************************************************************
#
#       Interface to Copy Manager service provider handler routines.
#
#  INPUT:
#       g1 = datagram ILT at nest level #4
#       g2 = local request message header address
#       g3 = local response message header address
#       g4 = request message buffer address (does NOT include header)
#       g5 = request message length (does NOT include header)
#       g6 = response message buffer address (does NOT include header)
#       g7 = response buffer length (does NOT include header)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All regs. can be destroyed.
#
#******************************************************************************
#
        callx   (r4)                    # and go to request handler routine
        b       cm$spexec               # check for more work to do
#
#******************************************************************************
#
#  NAME:  CMsp$hand
#
#  PURPOSE:
#       This table contains the valid request handler routines for
#       the CMsp datagram service provider.
#
#  DESCRIPTION:
#       This table contains the request handler routines for the CMsp
#       datagram server in order based on the request function code.
#
#  INPUT:
#       g1 = datagram ILT at nest level #4
#       g2 = local request message header address
#       g3 = local response message header address
#       g4 = request message buffer address (does NOT include header)
#       g5 = request message length (does NOT include header)
#       g6 = response message buffer address (does NOT include header)
#       g7 = response buffer length (does NOT include header)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All regs. can be destroyed.
#
#******************************************************************************
#
        .data
CMsp$hand:
        .word   CMsp$op_new             # 0 CMsp_fc_op_new - Define new copy operation
        .word   CMsp$op_state           # 1 CMsp_fc_op_state - Establish copy operation state
        .word   CMsp$op_term            # 2 CMsp_fc_op_term - Terminate copy operation
        .word   CMsp$op_check           # 3 CMsp_fc_op_check - Check copy operation state/status
        .word   CMsp$rm_start           # 4 CMsp_fc_rm_start - Maintain/start region map table
        .word   CMsp$rm_susp            # 5 CMsp_fc_rm_susp - Suspend region map table
        .word   CMsp$rm_term            # 6 CMsp_fc_rm_term - Terminate region map table
        .word   CMsp$rm_read            # 7 CMsp_fc_rm_read - Read region/segment map table
        .word   CMsp$rm_clear           # 8 CMsp_fc_rm_clear - Clear region/segment map table
        .word   CMsp$rm_check           # 9 CMsp_fc_rm_check - Check region map table state/status
        .word   CMsp$op_susp            # 10 CMsp_fc_op_susp - Suspend copy operation
        .word   CMsp$op_resume          # 11 CMsp_fc_op_resume - Resume copy operation
        .word   CMsp$op_dmove           # 12 CMsp_fc_op_dmove - Copy device moved
#
CMsp$hand_end:
        .set    CMsp$maxfc,((CMsp$hand_end-CMsp$hand)/4)-1 # maximum valid function code
#
        .text
#
#**********************************************************************
#
#  NAME: CMsp$op_new
#
#  PURPOSE:
#       To process inbound Define New Copy Operation datagram messages.
#
#  DESCRIPTION:
#       This routine validates the incoming datagram message to determine
#       if the request is to be performed. If any invalid message
#       parameters are identified, it returns an error to the requestor.
#       This routine checks if a copy operation matching the copy
#       registration ID and copy MAG serial number is currently defined
#       and if there is will immediately terminate the copy operation
#       in the appropriate manner. It then sets up to define a new copy
#       operation to service the request and returns a good completion
#       response to the requestor.
#
#  CALLING SEQUENCE:
#       call    CMsp$op_new
#
#  INPUT:
#       g1 = datagram ILT at nest level #4
#       g2 = local request message header address
#       g3 = local response message header address
#       g4 = request message buffer address (does NOT include header)
#       g5 = request message length (does NOT include header)
#       g6 = response message buffer address (does NOT include header)
#       g7 = response buffer length (does NOT include header)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All regs. can be destroyed.
#
#**********************************************************************
#
CMsp$op_new:
        call    CMsp$trans              # translate standard data fields
                                        #  in request message buffer
        ldconst CMsp_rq_opnew_size,r5   # r5 = expected request message size
        cmpoble.t r5,g5,.opnew_100      # Jif remaining request
                                        #  message length is correct
.opnew_50:
        call    DLM$srvr_invparm        # return invalid parameter response
        b       .opnew_1000             # and get out of here!

.opnew_100:
!       ld      CMsp_rq_opnew_rcsn(g4),r4 # r4 = specified copy MAG serial #
                                        #  in little-endian format
        ld      K_ficb,r3               # r3 = FICB address
        ld      fi_vcgid(r3),r3         # r3 = my serial #
        cmpobe.f r3,r4,.opnew_50        # Jif copy MAG serial # is me
        call    DLM$chk_master          # check if I'm the group master
                                        #  controller
                                        # g0 = 0 if I'm the master
        cmpobe  0,g0,.opnew_102         # Jif I'm the group master
        call    DLM$srvr_reroute        # return re-route datagram response
        b       .opnew_1000             # and get out of here!

.opnew_102:
!       ld      CMsp_rq_opnew_rssn(g4),r5 # r5 = source MAG serial # in
                                        #  little-endian format
!       ld      CMsp_rq_opnew_rdsn(g4),r6 # r6 = dest. MAG serial # in
                                        #  little-endian format
        ldconst 0,r12                   # r12 = VDD address if source copy
                                        #       device
        ldconst 0,r13                   # r13 = VDD address if dest. copy
                                        #       device
        ldconst MAXVIRTUALS,r15         # r15 = max. VDisk #
        cmpobne.f r5,r3,.opnew_120      # Jif not the source copy device
#
# --- We're defined as the source copy device. Form and
#     validate VID then locate the associated VDD.
#
!       ldob    CMsp_rq_opnew_rscl(g4),r7 # r7 = source device cluster #
!       ldob    CMsp_rq_opnew_rsvd(g4),r8 # r8 = source device VDisk #
        shlo    8,r7,r7
        addo    r8,r7,r7
        cmpobge.f r7,r15,.opnew_50      # Jif VDisk # invalid
        ld      V_vddindx[r7*4],r12     # r12 = corresponding VDD
        cmpobe.f 0,r12,.opnew_50        # Jif no VDisk defined
#
# --- Validate that the copy manager MAG owns this VDisk
#
        ld      vd_vlinks(r12),r10      # r10 = first VLAR assoc. with
                                        #       specified source copy device
        cmpobne.t 0,r10,.opnew_110      # Jif VLAR assoc. with device
.opnew_105:
        call    CMsp$srvr_nosvlink      # format and send response to
                                        #  requestor
        b       .opnew_1000

.opnew_110:
        ld      vlar_srcsn(r10),r7      # r7 = source MAG serial # assoc.
                                        #      with VLAR
        cmpobne.t r4,r7,.opnew_115      # Jif not owned by copy manager MAG
        ldob    vlar_srccl(r10),r7      # r7 = source MAG cluster # assoc.
                                        #      with VLAR
!       ldob    CMsp_rq_opnew_rcscl(g4),r8 # r8 = source MAG cluster # of
                                        #         copy manager
        cmpobne.f r7,r8,.opnew_115      # Jif not owned by copy manager MAG
        ldob    vlar_srcvd(r10),r7      # r7 = source MAG VDisk # assoc.
                                        #      with VLAR
!       ldob    CMsp_rq_opnew_rcsvd(g4),r8 # r8 = source MAG VDisk # of
                                        #         copy manager
        cmpobe.t r7,r8,.opnew_120       # Jif owned by copy manager MAG
.opnew_115:
        ld      vlar_link(r10),r10      # r10 = next VLAR on list
        cmpobne.f 0,r10,.opnew_110      # Jif more VLARs assoc. with VDisk
        b       .opnew_105              # reject copy request because copy
                                        #  manager does not own this VDisk
.opnew_120:
        cmpobne.f r6,r3,.opnew_140      # Jif not the dest. copy device
#
# --- We're defined as the destination copy device. Form and
#     validate VID then locate the associated VDD.
#
        ldob    CMsp_rq_opnew_rdcl(g4),r7 # r7 = Destination device cluster #
        ldob    CMsp_rq_opnew_rdvd(g4),r8 # r8 = Destination device VDisk #
        shlo    8,r7,r7
        addo    r8,r7,r7
        cmpobge.f r7,r15,.opnew_50      # Jif VDisk # invalid
        ld      V_vddindx[r7*4],r13     # r13 = corresponding VDD
        cmpobe.f 0,r13,.opnew_50        # Jif no VDisk defined
#
# --- Validate that the copy manager MAG owns this VDisk
#
        ld      vd_vlinks(r13),r10      # r10 = first VLAR assoc. with
                                        #       specified source copy device
        cmpobne.t 0,r10,.opnew_130      # Jif VLAR assoc. with device
.opnew_125:
        call    CMsp$srvr_nodvlink      # format and send response to
                                        #  requestor
        b       .opnew_1000

.opnew_130:
        ld      vlar_srcsn(r10),r7      # r7 = source MAG serial # assoc.
                                        #      with VLAR
        cmpobne.t r4,r7,.opnew_135      # Jif not owned by copy manager MAG
        ldob    vlar_srccl(r10),r7      # r7 = source MAG cluster # assoc.
                                        #      with VLAR
        ldob    CMsp_rq_opnew_rcdcl(g4),r8 # r8 = source MAG cluster # of
                                        #         copy manager
        cmpobne.f r7,r8,.opnew_135      # Jif not owned by copy manager MAG
        ldob    vlar_srcvd(r10),r7      # r7 = source MAG VDisk # assoc.
                                        #      with VLAR
        ldob    CMsp_rq_opnew_rcdvd(g4),r8 # r8 = source MAG VDisk # of
                                        #         copy manager
        cmpobe.t r7,r8,.opnew_140       # Jif owned by copy manager MAG
.opnew_135:
        ld      vlar_link(r10),r10      # r10 = next VLAR on list
        cmpobne.f 0,r10,.opnew_130      # Jif more VLARs assoc. with VDisk
        b       .opnew_125              # reject copy request because copy
                                        #  manager does not own this VDisk
.opnew_140:
        cmpobe.f r12,r13,.opnew_50      # Jif source and dest. copy devices
                                        #  the same or neither copy device
                                        #  is defined to my node.
        cmpobe  0,r12,.opnew_170        # Jif not source copy device
        cmpobe  0,r13,.opnew_170        # Jif not destination copy device
#
# --- We're defined as both source and destination copy devices.
#       Make sure these virtual devices are both owned by the same controller.
#
        ldob    vd_owner(r12),r14       # r14 = source copy device owner
        ldob    vd_owner(r13),r15       # r15 = dest. copy device owner
        cmpobne r14,r15,.opnew_50       # reject request if owned by
                                        #  different controllers
.opnew_170:
#
# --- Check if a copy operation matching the specified ID of the new
#       inbound copy operation request is already defined and if
#       so, terminate the "old" copy operation.
#
        mov     g1,r15                  # save g1
!       ld      CMsp_rq_opnew_rid(g4),g0 # g0 = specified copy reg. ID
        mov     r4,g1                   # g1 = specified copy MAG serial #
        call    CM$find_cor_rid         # check if copy already active with
                                        #  the specified ID
                                        # g0 = COR address if match found
        cmpobe.t 0,g0,.opnew_200        # Jif no copy active with this ID
        mov     g3,r14                  # save g3
        mov     g0,g3                   # g3 = COR to terminate because we're
                                        #      defining a new copy op. with
                                        #      this same ID
        call    CCSM$CopyTerminated     # terminate the "old" COR
        mov     r14,g3                  # restore g3
.opnew_200:
        mov     r15,g1                  # restore g1
#
# --- If we are the destination copy device specified in the new copy
#       operation request, check if the specified destination copy
#       device is already a destination copy device of another copy
#       operation and if so reject the inbound copy operation request.
#
        cmpobe.t 0,r13,.opnew_300       # Jif not the specified dest. copy
                                        #  device
        ld      vd_dcd(r13),r10         # r10 = DCD assoc. with specified
                                        #       dest. copy device
        cmpobe.t 0,r10,.opnew_300       # Jif not already the dest. copy
                                        #  device of another copy operation
        call    CMsp$srvr_inuse         # format and send response to
                                        #  requestor
        b       .opnew_1000

.opnew_300:
#
# --- Set up COR -------------------------------------------------------------
#
        mov     g3,r15                  # save g3
c       g3 = get_cor();                 # Allocate a COR data structure
.ifdef M4_DEBUG_COR
c fprintf(stderr, "%s%s:%u get_cor 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g3);
.endif # M4_DEBUG_COR
!       ldob    CMsp_rq_opnew_cstate(g4),r7 # r7 = specified copy state code
        stob    r7,cor_cstate(g3)       # save COR copy state code
!       ldob    CMsp_rq_opnew_crstate(g4),r7 # r7 = specified copy reg. state
        stob    r7,cor_crstate(g3)      # save COR copy reg. state
        st      r12,cor_srcvdd(g3)      # save source VDD address in COR
        st      r13,cor_destvdd(g3)     # save dest. VDD address in COR
!       ldq     CMsp_rq_opnew_rid(g4),r8 # copy reg. ID values into COR
        stq     r8,cor_rid(g3)
!       ldl     CMsp_rq_opnew_rdsn(g4),r8
        stl     r8,cor_rdsn(g3)
!       ldq     CMsp_rq_opnew_label(g4),r8 # r8-r11 = specified copy label
        stq     r8,cor_label(g3)        # save initial cor_label value
!       ld      CMsp_rq_opnew_totalsegs(g4),r8 # r8 = total # segments
        bswap   r8,r8
        st      r8,cor_totalsegs(g3)    # save total # segments in COR
!       ldob    CMsp_rq_opnew_gid(g4),r8 # r8 = user defined group ID
        stob    r8,cor_gid(g3)          # save user defined group ID
#
# --- Allocate a State NVRAM area for this copy.
#
        PushRegs(r3)                    # Save all "g" registers and g14 = 0
        call    P6_AllocStRec           # allocate a state record for this copy
        cmpobne 0,g0,.opnew_340         # Jif record allocated
#
        PopRegs(r3)                     # Restore registers
.ifdef M4_DEBUG_COR
c fprintf(stderr, "%s%s:%u put_cor 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g3);
.endif # M4_DEBUG_COR
c       put_cor(g3);                    # Deallocate COR data structure
        mov     r15,g3                  # restore g3
        call    CMsp$srvr_insres        # format and send response to
                                        #  requestor
        b       .opnew_1000

.opnew_340:
        PopRegs(r3)                     # Restore registers
        lda     CM$wp2_null,r8          # r8 = phase 2 write update handler
                                        #      routine (resumed)
        cmpobe  corcrst_usersusp,r7,.opnew_350 # Jif in user suspended state
        cmpobne corcrst_remsusp,r7,.opnew_360 # Jif not remote suspended state
.opnew_350:
        lda     CM$wp2_suspend,r8       # r8 = phase 2 write update handler
                                        #      routine (suspended)
.opnew_360:
        cmpobe.f 0,r12,.opnew_400       # Jif not the source copy device
#
# --- Set up SCD -------------------------------------------------------------
#
c       g0 = get_scd();                 # Allocate a SCD data structure
.ifdef M4_DEBUG_SCD
c fprintf(stderr, "%s%s:%u get_scd 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif # M4_DEBUG_SCD
        st      g3,scd_cor(g0)          # save COR address in SCD
        st      g0,cor_scd(g3)          # save SCD address in COR
        st      r12,scd_vdd(g0)         # save source VDD address in SCD
        ldconst scdt_remote,r7          # r7 = scd_type code
        stob    r7,scd_type(g0)         # save scd_type code
        st      r8,scd_p2hand(g0)       # save phase 2 write update handler
                                        #  routine in SCD
        mov     g0,r12                  # r12 = SCD address
.opnew_400:
        cmpobe.f 0,r13,.opnew_500       # Jif not the dest. copy device
#
# --- Set up DCD -------------------------------------------------------------
#
c       g0 = get_dcd();                 # Allocate a DCD data structure
.ifdef M4_DEBUG_DCD
c fprintf(stderr, "%s%s:%u get_dcd 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif # M4_DEBUG_DCD
        st      g3,dcd_cor(g0)          # save COR address in DCD
        st      g0,cor_dcd(g3)          # save DCD address in COR
        st      r13,dcd_vdd(g0)         # save dest. VDD address in DCD
        ldconst dcdt_remote,r7          # r7 = dcd_type code
        stob    r7,dcd_type(g0)         # save dcd_type code
        st      r8,dcd_p2hand(g0)       # save phase 2 write update handler
                                        #  routine in DCD
        mov     g0,r13                  # r13 = DCD address
.opnew_500:
#
# --- Activate SCD & DCD as needed
#
        cmpobe.f 0,r12,.opnew_520       # Jif not the source copy device
        mov     r12,g0                  # g0 = SCD address to activate
        call    CM$act_scd              # activate SCD
.opnew_520:
        cmpobe.f 0,r13,.opnew_540       # Jif not the dest. copy device
        mov     r13,g0                  # g0 = DCD address to activate
        call    CM$act_dcd              # activate DCD
.opnew_540:
        ldconst CCtbl,r8                # r8 = C.C.S.E. state table
        st      r8,cor_ccseptr(g3)      # save C.C.S.E. state table pointer
        ldconst OCtbl,r8                # r8 = O.C.S.E. state table
        st      r8,cor_ocseptr(g3)      # save O.C.S.E. state table pointer
        ldconst 0,r8
        st      r8,cor_ccsecev(g3)
        st      r8,cor_ocsecev(g3)
        stob    r8,cor_ostindx(g3)
        stob    r8,cor_cstindx(g3)
#
# --- Put COR on active queue ----------------------------------------
#
        call    CM$act_cor              # activate COR
#
# --- Generate Start Copy event to CCSM
#
        call    CCSM$start_copy
        mov     r15,g3                  # restore g3
        call    D$p2updateconfig        # save P2 NVRAM
        call    D$SendRefreshNV         # tell other nodes to refresh NVRAM
        ldq     DLM_srvr_ok,r4          # r4-r7 = good response header
        stq     r4,(g3)                 # save response message header
        lda     -ILTBIAS(g1),g1         # g1 = previous ILT nest level
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT completion handler routine
.opnew_1000:
        ret
#
#**********************************************************************
#
#  NAME: CMsp$op_state
#
#  PURPOSE:
#       To process inbound Establish Copy Operation State datagram
#       messages.
#
#  DESCRIPTION:
#       This routine validates the incoming datagram message to determine
#       if the request is to be performed. If any invalid message
#       parameters are identified, it returns an error to the requestor.
#       This routine checks if a copy operation matching the copy
#       registration ID and copy MAG serial number is currently defined
#       and if there isn't will return an error back to the requestor.
#       If the specified copy operation exists, it then goes through
#       the specified copy operation state data and makes the necessary
#       updates to the local copy operation to align it to the specified
#       request if possible. If any specified state changes are not
#       allowed because of the current local state of the copy operation,
#       the appropriate error is returned to the requestor. If no error
#       is returned to the requestor, all of the necessary state changes
#       have occurred successfully. The logic processing the incoming copy
#       operation information in the following order:
#
#       - copy MAG information (rcscl, rcsvd, rcdcl, rcdvd)
#       - source MAG information (rssn, rscl, rsvd)
#       - destination MAG information (rdsn, rdcl, rdvd)
#       - user defined copy operation label (label)
#       - user defined group ID (gid)
#       - copy state (cstate)
#       - copy registration state (crstate)
#
#  CALLING SEQUENCE:
#       call    CMsp$op_state
#
#  INPUT:
#       g1 = datagram ILT at nest level #4
#       g2 = local request message header address
#       g3 = local response message header address
#       g4 = request message buffer address (does NOT include header)
#       g5 = request message length (does NOT include header)
#       g6 = response message buffer address (does NOT include header)
#       g7 = response buffer length (does NOT include header)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All regs. can be destroyed.
#
#**********************************************************************
#
CMsp$op_state:
        call    CMsp$trans              # translate standard data fields
                                        #  in request message buffer
        ldconst CMsp_rq_opst_size,r5    # r5 = expected request message size
        cmpoble.t r5,g5,.opst_100       # Jif remaining request
                                        #  message length is correct
        call    DLM$srvr_invparm        # return invalid parameter response
        b       .opst_1000              # and get out of here!

.opst_100:
        call    CM$chk_owner            # check if we're the current copy
                                        #  owner
        cmpobe  0,g0,.opst_1000         # Jif not the current copy owner
        call    cmsp$up_info            # update copy operation information
                                        #  as needed
                                        # g0 = COR address if update
                                        #      processing was successful
                                        # g8 = TRUE/FALSE indicator if copy
                                        #      devices changed
        cmpobe.f 0,g0,.opst_1000        # Jif error indicated processing
                                        #  copy operation information
!       ldob    CMsp_rq_opst_cstate(g4),r4 # r4 = requestor's copy state code
        ldob    cor_cstate(g0),r5       # r5 = my current copy state code
        cmpobe.t r4,r5,.opst_200        # Jif copy state has not changed
        stob    r4,cor_cstate(g0)       # save updated copy state code in COR
#        ldconst TRUE,g8                 # g8 = TRUE indicator that non-
                                        #      volatile save needed
.opst_200:
!       ldob    CMsp_rq_opst_crstate(g4),r4 # r4 = requestor's copy reg.
                                        #          state code
        ldob    cor_crstate(g0),r5      # r5 = my current copy reg. state code
        cmpobe.t r4,r5,.opst_300        # Jif copy reg. state has not changed
#
# --- Determine what state is indicated in the datagram request
#       message and set up the appropriate environment to handle it.
#
        cmpobe.f corcrst_remsusp,r5,.opst_300 # Jif local is remote suspended
        stob    r4,cor_crstate(g0)      # save updated copy reg. state code
                                        #  in COR
#
# --- Copy registration state has changed. Process incoming state.
#
        ldconst TRUE,g8                 # g8 = TRUE indicator that non-
                                        #      volatile save needed
        cmpobne.t corcrst_usersusp,r4,.opst_270 # Jif not user suspended
        ldconst cormst_act,r5           # r5 = active map state code
        lda     CM$wp2_suspend,r6       # r6 = suspended write update phase
                                        #      2 handler routine
        stob    r5,cor_mstate(g0)       # set map state to active
        ld      cor_scd(g0),r4          # r4 = assoc. SCD address
        cmpobe.f 0,r4,.opst_260         # Jif no SCD assoc. with COR
        st      r6,scd_p2hand(r4)       # save phase 2 update handler routine
                                        #  in SCD
.opst_260:
        ld      cor_dcd(g0),r4          # r4 = assoc. DCD address
        cmpobe.f 0,r4,.opst_270         # Jif no DCD assoc. with COR
        st      r6,dcd_p2hand(r4)       # save phase 2 update handler routine

.opst_270:
        mov     g3,r4                   # save g3
        mov     g0,g3                   # g3 = COR address
        call    CM$mmc_sflag            # process suspended flag for MMC
        mov     r4,g3                   # restore g3
                                        #  in DCD
.opst_300:
        cmpobne.t TRUE,g8,.opst_400     # Jif non-volatile save not needed
        mov     g3,r6                   # save g3
        mov     g0,g3                   # g3 = assoc. COR address
        call    CCSM$cs_chged_w_msg     # generate a copy state changed event
        mov     r6,g3                   # restore g3
.opst_400:
#
# --- Request processed successfully. Return good response message.
#
        ldq     DLM_srvr_ok,r4          # r4-r7 = good response header
        stq     r4,(g3)                 # save response message header
        lda     -ILTBIAS(g1),g1         # g1 = previous ILT nest level
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT completion handler routine
.opst_1000:
        ret
#
#**********************************************************************
#
#  NAME: CMsp$op_term
#
#  PURPOSE:
#       To process inbound Terminate Copy Operation datagram
#       messages.
#
#  DESCRIPTION:
#       This routine validates the inbound Terminate Copy Operation
#       datagram request and if valid will terminate the associated
#       copy operation as appropriate.
#
#  CALLING SEQUENCE:
#       call    CMsp$op_term
#
#  INPUT:
#       g1 = datagram ILT at nest level #4
#       g2 = local request message header address
#       g3 = local response message header address
#       g4 = request message buffer address (does NOT include header)
#       g5 = request message length (does NOT include header)
#       g6 = response message buffer address (does NOT include header)
#       g7 = response buffer length (does NOT include header)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All regs. can be destroyed.
#
#**********************************************************************
#
CMsp$op_term:
        call    CMsp$trans              # translate standard data fields
                                        #  in request message buffer
        ldconst CMsp_rq_optrm_size,r5   # r5 = expected request message size
        cmpoble.t r5,g5,.optrm_100      # Jif remaining request
                                        #  message length is correct
        call    DLM$srvr_invparm        # return invalid parameter response
        b       .optrm_1000             # and get out of here!

.optrm_100:
        call    DLM$chk_master          # check if I'm the group master
                                        #  controller
                                        # g0 = 0 if I'm the master
        cmpobe  0,g0,.optrm_120         # Jif I'm the group master
        call    DLM$srvr_reroute        # return re-route datagram response
        b       .optrm_1000             # and get out of here!

.optrm_120:
#
# --- Check if a copy operation matching the specified ID of the
#       inbound terminate copy operation request is defined and if
#       so, terminate the copy operation.
#
        mov     g1,r15                  # save g1
!       ld      CMsp_rq_optrm_rid(g4),g0 # g0 = specified copy reg. ID
!       ld      CMsp_rq_opchk_rcsn(g4),g1 # g1 = specified copy MAG serial #
        call    CM$find_cor_rid         # check if copy operation active with
                                        #  the specified ID
                                        # g0 = COR address if match found
        cmpobe.t 0,g0,.optrm_200        # Jif no copy active with this ID
        mov     g3,r14                  # save g3
        mov     g0,g3                   # g3 = COR address to terminate
        call    CCSM$term_copy          # terminate the COR
        mov     r14,g3                  # restore g3
.optrm_200:
        mov     r15,g1                  # restore g1
        ldq     DLM_srvr_ok,r4          # r4-r7 = good response header
        stq     r4,(g3)                 # save response message header
        lda     -ILTBIAS(g1),g1         # g1 = previous ILT nest level
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT completion handler routine
.optrm_1000:
        ret
#
#**********************************************************************
#
#  NAME: CMsp$op_check
#
#  PURPOSE:
#       To process inbound Check Copy Operation State/Status datagram
#       messages.
#
#  DESCRIPTION:
#       This routine locates the specified copy operation and if found
#       will return the local copy operation information back to the
#       requestor. If the copy operation is not found, the appropriate
#       error response message will be returned.
#
#  CALLING SEQUENCE:
#       call    CMsp$op_check
#
#  INPUT:
#       g1 = datagram ILT at nest level #4
#       g2 = local request message header address
#       g3 = local response message header address
#       g4 = request message buffer address (does NOT include header)
#       g5 = request message length (does NOT include header)
#       g6 = response message buffer address (does NOT include header)
#       g7 = response buffer length (does NOT include header)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All regs. can be destroyed.
#
#**********************************************************************
#
CMsp$op_check:
        call    CMsp$trans              # translate standard data fields
                                        #  in request message buffer
        ldconst CMsp_rq_opchk_size,r5   # r5 = expected request message size
        cmpoble.t r5,g5,.opchk_100      # Jif remaining request
                                        #  message length is correct
.opchk_50:
        call    DLM$srvr_invparm        # return invalid parameter response
        b       .opchk_1000             # and get out of here!

.opchk_100:
        call    CM$chk_owner            # check if we're the current copy
                                        #  owner
        cmpobe  0,g0,.opchk_1000        # Jif not the current copy owner
        ldconst CMsp_rs_opchk_size,r4   # r4 = expected remaining response
                                        #  message length
        cmpobl.f g7,r4,.opchk_50        # Jif size < expected
        ld      cor_cm(g0),r3           # r3 = assoc. CM address
        cmpobne.f 0,r3,.opchk_400       # Jif specified COR has copy manager
                                        #  functions on this MAGNITUDE
        movt    g0,r12                  # save g0-g2
        lda     cor_label(g0),g0
        lda     CMsp_rq_opchk_label(g4),g1
        ldconst 16,g2
        call    CCSM$strcomp
        mov     g0,r10                  # r10 = compare result
        movt    r12,g0                  # restore g0-g2
        cmpobne TRUE,r10,.opchk_205     # Jif copy label differs
!       ldob    CMsp_rq_opchk_gid(g4),r12
        ldob    cor_gid(g0),r13
        cmpobe  r12,r13,.opchk_207      # Jif cor_gid matches
.opchk_205:
        ldq     CMsp_rq_opchk_label(g4),r12 # r12-r15 = user defined copy
                                        #               operation label
        stq     r12,cor_label(g0)       # save user defined copy operation
                                        #  label in COR
        ldob    CMsp_rq_opchk_gid(g4),r12 # r12 = user defined group ID
        stob    r12,cor_gid(g0)         # save user defined group ID
        mov     g3,r10                  # save g3
        mov     g0,g3                   # g3 = assoc. COR address
        call    CCSM$info_chged         # generate copy info. changed event
        mov     r10,g3                  # restore g3
.opchk_207:
        ldconst FALSE,r3                # r3 = copy devices moved flag
        ldconst 0,r10
        ld      cor_rcsn(g0),r6         # r6 = copy MAG serial #
        stos    r10,cor_tmr1(g0)        # clear/reset timer #1 in COR
        ld      cor_scd(g0),r4          # r4 = assoc. SCD address if source
                                        #  copy device
        cmpobe.f 0,r4,.opchk_250        # Jif not the source copy device for
                                        #  this copy operation
        ldconst 0xff,r10                # r10 = unassigned cluster #
        ldconst 0xff,r11                # r11 = unassigned VDisk #
        ld      scd_vdd(r4),r5          # r5 = assoc. VDD address
        cmpobe.f 0,r5,.opchk_240        # Jif no VDD assoc. with SCD
        ld      vd_vlinks(r5),r12       # r12 = first VLAR assoc. with
                                        #       source copy device
        cmpobe.f 0,r12,.opchk_240       # Jif no VLARs assoc. with device
.opchk_210:
        ld      vlar_srcsn(r12),r7      # r7 = source MAG serial # assoc.
                                        #      with VLAR
        cmpobne.t r6,r7,.opchk_215      # Jif not owned by copy manager MAG
        ldob    vlar_srccl(r12),r10     # r10 = source MAG cluster # assoc.
                                        #      with VLAR
        ldob    vlar_srcvd(r12),r11     # r11 = source MAG VDisk # assoc.
                                        #      with VLAR
        b       .opchk_240              # and place these values in the COR

.opchk_215:
        ld      vlar_link(r12),r12      # r12 = next VLAR on list
        cmpobne.f 0,r12,.opchk_210      # Jif more VLARs assoc. with VDisk
.opchk_240:
        ldob    cor_rcscl(g0),r8        # r8 = current cor_rcscl value
        cmpobe  r8,r10,.opchk_242       # Jif cor_rcscl the same
        ldconst TRUE,r3                 # set copy devices moved flag
        stob    r10,cor_rcscl(g0)       # save copy MAG cluster # in COR
.opchk_242:
        ldob    cor_rcsvd(g0),r8        # r8 = current cor_rcsvd value
        cmpobe  r8,r11,.opchk_250       # Jif cor_rcsvd the same
        ldconst TRUE,r3                 # set copy devices moved flag
        stob    r11,cor_rcsvd(g0)       # save copy MAG VDisk # in COR
.opchk_250:
        ld      cor_dcd(g0),r4          # r4 = assoc. DCD address if dest.
                                        #  copy device
        cmpobe.f 0,r4,.opchk_300        # Jif not the dest. copy device for
                                        #  this copy operation
        ldconst 0xff,r10                # r10 = unassigned cluster #
        ldconst 0xff,r11                # r11 = unassigned VDisk #
        ld      dcd_vdd(r4),r5          # r5 = assoc. VDD address
        cmpobe.f 0,r5,.opchk_290        # Jif no VDD assoc. with DCD
        ld      vd_vlinks(r5),r12       # r12 = first VLAR assoc. with
                                        #       dest. copy device
        cmpobe.f 0,r12,.opchk_290       # Jif no VLARs assoc. with device
.opchk_260:
        ld      vlar_srcsn(r12),r7      # r7 = source MAG serial # assoc.
                                        #      with VLAR
        cmpobne.t r6,r7,.opchk_265      # Jif not owned by copy manager MAG
        ldob    vlar_srccl(r12),r10     # r10 = source MAG cluster # assoc.
                                        #      with VLAR
        ldob    vlar_srcvd(r12),r11     # r11 = source MAG VDisk # assoc.
                                        #      with VLAR
        b       .opchk_290              # and place these values in the COR

.opchk_265:
        ld      vlar_link(r12),r12      # r12 = next VLAR on list
        cmpobne.f 0,r12,.opchk_260      # Jif more VLARs assoc. with VDisk
.opchk_290:
        ldob    cor_rcdcl(g0),r8        # r8 = current cor_rcdcl value
        cmpobe  r8,r10,.opchk_292       # Jif cor_rcdcl the same
        ldconst TRUE,r3                 # set copy devices moved flag
        stob    r10,cor_rcdcl(g0)       # save copy MAG cluster # in COR
.opchk_292:
        ldob    cor_rcdvd(g0),r8        # r8 = current cor_rcdvd value
        cmpobe  r8,r11,.opchk_300       # Jif cor_rcdvd the same
        ldconst TRUE,r3                 # set copy devices moved flag
        stob    r11,cor_rcdvd(g0)       # save copy MAG VDisk # in COR
.opchk_300:
!       ldob    CMsp_rq_opchk_cstate(g4),r4 # r4 = cor_cstate value from copy
                                        #          owner
        stob    r4,cor_cstate(g0)       # save cor_cstate value in COR
        cmpobne TRUE,r3,.opchk_400      # Jif copy devices did not move
        mov     g3,r8                   # save g3
        mov     g0,g3                   # g3 = COR address being processed
        call    CCSM$cd_moved           # generate copy devices moved event
        mov     r8,g3                   # restore g3
.opchk_400:
        ldq     cor_rid(g0),r4          # r4 = cor_rid
                                        # r5 = cor_rcsn
                                        # r6 = cor_rcscl,cor_rcsvd,
                                        #      cor_rcdcl,cor_rcdvd
                                        # r7 = cor_rssn
        ldl     cor_rdsn(g0),r8         # r8 = cor_rdsn
                                        # r9 = cor_rscl,cor_rsvd,
                                        #      cor_rdcl,cor_rdvd
        bswap   r4,r4
        bswap   r5,r5
        bswap   r7,r7
        bswap   r8,r8
!       stq     r4,CMsp_rs_opchk_rid(g6)
!       stl     r8,CMsp_rs_opchk_rdsn(g6)
        ldob    cor_cstate(g0),r4       # r4 = cor_cstate
        ldob    cor_crstate(g0),r5      # r5 = cor_crstate
        ldob    cor_mstate(g0),r6       # r6 = cor_mstate
!       st      r4,CMsp_rs_opchk_cstate(g6)
!       stob    r5,CMsp_rs_opchk_crstate(g6)
!       stob    r6,CMsp_rs_opchk_mstate(g6)
        ldq     DLM_srvr_ok,r4          # r4-r7 = good response header
        ldconst CMsp_rs_opchk_size,r6   # r6 = remaining response message
                                        #  length
        bswap   r6,r6
        stq     r4,(g3)                 # save response message header
        lda     -ILTBIAS(g1),g1         # g1 = previous ILT nest level
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT completion handler routine
.opchk_1000:
        ret
#
#**********************************************************************
#
#  NAME: CMsp$rm_start
#
#  PURPOSE:
#       To process inbound Maintain/Start Region Map Table datagram
#       messages.
#
#  CALLING SEQUENCE:
#       call    CMsp$rm_start
#
#  INPUT:
#       g1 = datagram ILT at nest level #4
#       g2 = local request message header address
#       g3 = local response message header address
#       g4 = request message buffer address (does NOT include header)
#       g5 = request message length (does NOT include header)
#       g6 = response message buffer address (does NOT include header)
#       g7 = response buffer length (does NOT include header)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All regs. can be destroyed.
#
#**********************************************************************
#
CMsp$rm_start:
#
#*** FINISH - add logic to process this request datagram
#
        b       DLM$srvr_invfc          #*** TEMPORARY***
#
#**********************************************************************
#
#  NAME: CMsp$rm_susp
#
#  PURPOSE:
#       To process inbound Suspend Region Map Table datagram
#       messages.
#
#  DESCRIPTION:
#       Validates incoming datagram message and locates the associated
#       COR for the specified copy operation. If any errors detected,
#       returns the appropriate error response to the requestor. If no
#       errors are detected, it validates that the copy manager MAGNITUDE
#       has control of the remote copy device(s) associated with the
#       copy operation and rejects the request if does not. If it does
#       have control of the copy device(s), it checks if the region map
#       logic is active and if so sets up to suspend the region map logic
#       and set the update handlers appropriately. The requestor needs to
#       provide the appropriate protection is in place prior to issuing this
#       request to insure that all writes to the associated virtual
#       device are accounted for in another location before suspending
#       the region/segment map table updating at the remote node.
#
#  CALLING SEQUENCE:
#       call    CMsp$rm_susp
#
#  INPUT:
#       g1 = datagram ILT at nest level #4
#       g2 = local request message header address
#       g3 = local response message header address
#       g4 = request message buffer address (does NOT include header)
#       g5 = request message length (does NOT include header)
#       g6 = response message buffer address (does NOT include header)
#       g7 = response buffer length (does NOT include header)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All regs. can be destroyed.
#
#**********************************************************************
#
CMsp$rm_susp:
        call    CMsp$trans              # translate standard data fields
                                        #  in request message buffer
        ldconst CMsp_rq_rmsusp_size,r5  # r5 = expected request message size
        cmpoble.t r5,g5,.rmsusp_100     # Jif remaining request
                                        #  message length is correct
        call    DLM$srvr_invparm        # return invalid parameter response
        b       .rmsusp_1000            # and get out of here!

.rmsusp_100:
        call    CM$chk_owner            # check if we're the current copy
                                        #  owner
        cmpobe  0,g0,.rmsusp_1000       # Jif not the current copy owner

        ldob    cor_mstate(g0),r4       # r4 = current map state
        cmpobe.f cormst_term,r4,.rmsusp_600 # Jif map terminated
        cmpobe.f cormst_susp,r4,.rmsusp_600 # Jif map already suspended

#****************************************************************************
#     REMOTE CACHE FLUSH CHANGE
#
# --- determine if there is a flush task active
#
        ld      cor_flushilt(g0),r4     # is there a flush operation active ???
        cmpobne 0,r4,.rmsusp_130        # Jif yes
#
# --- no flash cache task active. so start one.
#     determine if there is a local source associated with this copy
#
        ldconst 0xffff,r6               # default r6 to no src
        ld      cor_srcvdd(g0),r5       # r5 = dest vdd address
        cmpobe  0,r5,.rmsusp_110        # Jif NULL - process normally
        ldos    vd_vid(r5),r6           # r6 = source vid
#
# --- determine if there is a local destination associated with this copy
#
.rmsusp_110:
        ldconst 0xffff,r7               # default r7 to no src
        ld      cor_destvdd(g0),r5      # r5 = dest vdd address
        cmpobe  0,r5,.rmsusp_120        # Jif NULL - process normally
        ldos    vd_vid(r5),r7           # r7 = destination vid

#
# --- start flush task
#
.rmsusp_120:
        movq    g0,r8                   # save g0-g3
c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT

        mov     g1,g2                   # g2 = ilt address

        stos    r6,il_w2(g2)            # save source vid of vdisk to flush
        stos    r7,il_w3(g2)            # save destination vid of vdisk to flush
        st      g0,il_w0(g2)            # save cor address in ilt
        st      g1,cor_flushilt(g0)     # save ilt in cor

        lda     cm$FlushCache,g0        # g0 = Address of the task to start
        ldconst VUPDFESTATUS,g1         # g1 = Priority of task to run
c       CT_fork_tmp = (ulong)"cm$FlushCache";
        call    K$tfork                 # Start up task that will update the
        st      g0,il_pcb(g2)           # save pcb address

        movq    r8,g0                   # restore g0-g3
        call    CMsp$srvr_rmact         # return region map active response
        b       .rmsusp_1000            # and get out of here!
#
# --- a flush operation active. determine if it is completed
#
.rmsusp_130:
        ld      il_pcb(r4),r5           # is the flush complete???
        cmpobe  0,r5,.rmsusp_140        # Jif yes

#
# --- flush not complete, send error
#
        call    CMsp$srvr_rmact         # return region map active response
        b       .rmsusp_1000            # and get out of here!
#
# --- flush complete, release ilt and process suspend RM
#
.rmsusp_140:
        mov     g1,r8                   # save g1
        ldconst 0,r3                    # set null
        mov     r4,g1                   # g1 = ilt
        st      r3,cor_flushilt(g0)     # clear ilt form cor

.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT

        mov     r8,g1                   # restore g1

#******************************************************************************
#
# --- process suspend RM
#
        ldob    cor_mstate(g0),r4       # r4 = current map state
        cmpobe.f cormst_term,r4,.rmsusp_600 # Jif map terminated
        cmpobe.f cormst_susp,r4,.rmsusp_600 # Jif map already suspended
        ld      cor_scd(g0),r4          # r4 = assoc. SCD address
        cmpobe.t 0,r4,.rmsusp_300       # Jif not the source copy device for
                                        #  this copy operation
        ld      scd_vdd(r4),r12         # r12 = assoc. VDD address
        cmpobe.f 0,r12,.rmsusp_205      # Jif no VDD assoc. with SCD
#
# --- Validate that the copy manager MAG owns this VDisk
#
        ld      cor_rcsn(g0),r4         # r4 = copy MAG serial #
        ld      vd_vlinks(r12),r10      # r10 = first VLAR assoc. with
                                        #       specified source copy device
        cmpobne.t 0,r10,.rmsusp_210     # Jif VLAR assoc. with device
.rmsusp_205:
        call    CMsp$srvr_nosvlink      # format and send response to
                                        #  requestor
        b       .rmsusp_1000

.rmsusp_210:
        ld      vlar_srcsn(r10),r7      # r7 = source MAG serial # assoc.
                                        #      with VLAR
        cmpobne.t r4,r7,.rmsusp_215     # Jif not owned by copy manager MAG
        ldob    vlar_srccl(r10),r7      # r7 = source MAG cluster # assoc.
                                        #      with VLAR
        ldob    CMsp_rq_rmsusp_rcscl(g4),r8 # r8 = source MAG cluster # of
                                        #         copy manager
        cmpobne.f r7,r8,.rmsusp_215     # Jif not owned by copy manager MAG
        ldob    vlar_srcvd(r10),r7      # r7 = source MAG VDisk # assoc.
                                        #      with VLAR
        ldob    CMsp_rq_rmsusp_rcsvd(g4),r8 # r8 = source MAG VDisk # of
                                        #         copy manager
        cmpobe.t r7,r8,.rmsusp_300      # Jif owned by copy manager MAG
.rmsusp_215:
        ld      vlar_link(r10),r10      # r10 = next VLAR on list
        cmpobne.f 0,r10,.rmsusp_210     # Jif more VLARs assoc. with VDisk
        b       .rmsusp_205             # reject copy request because copy
                                        #  manager does not own this VDisk
.rmsusp_300:
        ld      cor_dcd(g0),r4          # r4 = assoc. DCD address
        cmpobe.f 0,r4,.rmsusp_400       # Jif not the dest. copy device for
                                        #  this copy operation
        ld      dcd_vdd(r4),r12         # r12 = assoc. VDD address
        cmpobe.f 0,r12,.rmsusp_305      # Jif no VDD assoc. with SCD
#
# --- Validate that the copy manager MAG owns this VDisk
#
        ld      cor_rcsn(g0),r4         # r4 = copy MAG serial #
        ld      vd_vlinks(r12),r10      # r10 = first VLAR assoc. with
                                        #       specified source copy device
        cmpobne.t 0,r10,.rmsusp_310     # Jif VLAR assoc. with device
.rmsusp_305:
        call    CMsp$srvr_nodvlink      # format and send response to
                                        #  requestor
        b       .rmsusp_1000

.rmsusp_310:
        ld      vlar_srcsn(r10),r7      # r7 = source MAG serial # assoc.
                                        #      with VLAR
        cmpobne.t r4,r7,.rmsusp_315     # Jif not owned by copy manager MAG
        ldob    vlar_srccl(r10),r7      # r7 = source MAG cluster # assoc.
                                        #      with VLAR
        ldob    CMsp_rq_rmsusp_rcdcl(g4),r8 # r8 = source MAG cluster # of
                                        #         copy manager
        cmpobne.f r7,r8,.rmsusp_315     # Jif not owned by copy manager MAG
        ldob    vlar_srcvd(r10),r7      # r7 = source MAG VDisk # assoc.
                                        #      with VLAR
        ldob    CMsp_rq_rmsusp_rcdvd(g4),r8 # r8 = source MAG VDisk # of
                                        #         copy manager
        cmpobe.t r7,r8,.rmsusp_400      # Jif owned by copy manager MAG
.rmsusp_315:
        ld      vlar_link(r10),r10      # r10 = next VLAR on list
        cmpobne.f 0,r10,.rmsusp_310     # Jif more VLARs assoc. with VDisk
        b       .rmsusp_305             # reject copy request because copy
                                        #  manager does not own this VDisk
.rmsusp_400:
        ldconst cormst_susp,r4          # r4 = map suspended state code
        lda     CM$wp2_null,r5          # r5 = map suspended phase 2 update
                                        #  handler routine
        stob    r4,cor_mstate(g0)       # set COR map state to suspended
        ld      cor_scd(g0),r6          # r6 = assoc. SCD address
        cmpobe.f 0,r6,.rmsusp_430       # Jif no SCD assoc. with COR
        st      r5,scd_p2hand(r6)       # save phase 2 update handler routine
                                        #  in SCD
.rmsusp_430:
        ld      cor_dcd(g0),r6          # r6 = assoc. DCD address
        cmpobe.f 0,r6,.rmsusp_500       # Jif no DCD assoc. with COR
        st      r5,dcd_p2hand(r6)       # save phase 2 update handler routine
                                        #  in DCD
#
#     Clear the temporarily disable of the FE Write Cache of source
#
.rmsusp_500:
        ld      cor_srcvdd(g0),r5
        cmpobe  0,r5,.rmsusp_510        # Jif NULL

        PushRegs(r3)                    # Save all G regs for "C" call
        ldos    vd_vid(r5),r6           # r6 = vid
        mov     r6,g0                   # g0 = load VID
        ldconst WC_CLEAR_T_DISABLE,g1   # g1 = Function to Temp Disable WC
        call    WC_VDiskDisable         # Go Clear the T Disable flag
c fprintf(stderr,"%s%s:%u CMsp$rm_susp(src) returned from Enabling VDisk Cache - VDD = %x\n", FEBEMESSAGE, __FILE__, __LINE__,(UINT32)r6);
        PopRegsVoid(r3)                 # Restore all G regs

#
#     Clear the temporarily disable of the FE Write Cache of destination
#
.rmsusp_510:
        ld      cor_destvdd(g0),r5
        cmpobe  0,r5,.rmsusp_600        # Jif NULL

        PushRegs(r3)                    # Save all G regs for "C" call
        ldos    vd_vid(r5),r6           # r6 = vid
        mov     r6,g0                   # g0 = load VID
        ldconst WC_CLEAR_T_DISABLE,g1   # g1 = Function to Temp Disable WC
        call    WC_VDiskDisable         # Go Clear the T Disable flag
c fprintf(stderr,"%s%s:%u CMsp$rm_susp(dest) returned from Enabling VDisk Cache - VDD = %x\n", FEBEMESSAGE, __FILE__, __LINE__,(UINT32)r6);
        PopRegsVoid(r3)                 # Restore all G regs

.rmsusp_600:
        ldq     DLM_srvr_ok,r4          # r4-r7 = good response header
        stq     r4,(g3)                 # save response message header
        lda     -ILTBIAS(g1),g1         # g1 = previous ILT nest level
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT completion handler routine
.rmsusp_1000:
        ret
#
#**********************************************************************
#
#  NAME: CMsp$rm_term
#
#  PURPOSE:
#       To process inbound Terminate Region Map Table datagram
#       messages.
#
#  DESCRIPTION:
#       This routine validates that the specified copy operation is
#       still valid and all request parameters are valid. If not,
#       the appropriate error is returned to the requestor. If valid,
#       it checks that the region map state of the copy operation is
#       terminated. If so, it just returns a successful response to
#       the requestor. If the region map state is suspended, it checks
#       if all region/segment maps have been collected. If not, it
#       returns an error to the requestor indicating there are segment
#       maps that have not been collected. If so, it sets the region
#       map state to terminated. If the region map state is active,
#       it returns an error indicating so to the requestor. If the
#       region map state is accumulating, it rejects the request since
#       this should never occur.
#
#  CALLING SEQUENCE:
#       call    CMsp$rm_term
#
#  INPUT:
#       g1 = datagram ILT at nest level #4
#       g2 = local request message header address
#       g3 = local response message header address
#       g4 = request message buffer address (does NOT include header)
#       g5 = request message length (does NOT include header)
#       g6 = response message buffer address (does NOT include header)
#       g7 = response buffer length (does NOT include header)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All regs. can be destroyed.
#
#**********************************************************************
#
CMsp$rm_term:
        call    CMsp$trans              # translate standard data fields
                                        #  in request message buffer
        ldconst CMsp_rq_rmterm_size,r5  # r5 = expected request message size
        cmpoble.t r5,g5,.rmterm_100     # Jif remaining request
                                        #  message length is correct
        call    DLM$srvr_invparm        # return invalid parameter response
        b       .rmterm_1000            # and get out of here!

.rmterm_100:
        call    CM$chk_owner            # check if we're the current copy
                                        #  owner
        cmpobe  0,g0,.rmterm_1000       # Jif not the current copy owner
        ldob    cor_mstate(g0),r4       # r4 = COR map state
        cmpobe.f cormst_term,r4,.rmterm_900 # Jif map state already terminated
        cmpobe.t cormst_susp,r4,.rmterm_300 # Jif map state is suspended
        call    CMsp$srvr_rmact         # return error response message
                                        #  indicating region map state still
                                        #  active
        b       .rmterm_1000

.rmterm_300:
#
# --- Set up to check if any segment tables are dirty
#
        mov     g3,r4                   # save g3
        mov     g0,g3                   # g3 = COR address being processed
        call    CM$update_rmap          # update region/segment map
                                        # cor_rmaptbl = 0 if no dirty
                                        #  segments found
        mov     r4,g3                   # restore g3
        ld      cor_rmaptbl(g0),r4      # r4 = region map table pointer
        cmpobe.f 0,r4,.rmterm_500       # Jif no region map table defined
        call    CMsp$srvr_dirty         # return error response message
                                        #  indicating segments still dirty
        b       .rmterm_1000

.rmterm_500:
        ldconst cormst_term,r4
        stob    r4,cor_mstate(g0)       # set region map state to terminated
.rmterm_900:
        ldq     DLM_srvr_ok,r4          # r4-r7 = good response header
        stq     r4,(g3)                 # save response message header
        lda     -ILTBIAS(g1),g1         # g1 = previous ILT nest level
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT completion handler routine
.rmterm_1000:
        ret
#
#**********************************************************************
#
#  NAME: CMsp$rm_read
#
#  PURPOSE:
#       To process inbound Read Region/Segment Map Table datagram
#       messages.
#
#  DESCRIPTION:
#       Validates incoming datagram message and locates the associated
#       COR for the specified copy operation. If any errors detected,
#       returns the appropriate error response to the requestor. If no
#       errors are detected, it packs the specified segment map table
#       data in the response data area and returns it to the requestor.
#
#  CALLING SEQUENCE:
#       call    CMsp$rm_read
#
#  INPUT:
#       g1 = datagram ILT at nest level #4
#       g2 = local request message header address
#       g3 = local response message header address
#       g4 = request message buffer address (does NOT include header)
#       g5 = request message length (does NOT include header)
#       g6 = response message buffer address (does NOT include header)
#       g7 = response buffer length (does NOT include header)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All regs. can be destroyed.
#
#**********************************************************************
#
CMsp$rm_read:
        call    CMsp$trans              # translate standard data fields
                                        #  in request message buffer
        ldconst CMsp_rq_rmrd_size,r5    # r5 = expected request message size
        cmpoble.t r5,g5,.rmread_100     # Jif remaining request
                                        #  message length is correct
.rmread_50:
        call    DLM$srvr_invparm        # return invalid parameter response
        b       .rmread_1000            # and get out of here!

.rmread_100:
        call    CM$chk_owner            # check if we're the current copy
                                        #  owner
        cmpobe  0,g0,.rmread_1000       # Jif not the current copy owner
        ldconst CMsp_rs_rmrd_size,r4    # r4 = expected remaining response
                                        #  message length
        cmpobl.f g7,r4,.rmread_50       # Jif size < expected
        ld      CMsp_rq_rmrd_reg(g4),r4 # r4 = specified region map number
        ldconst maxRMcnt-1,r5           # r5 = max. # regions supported - 1
        bswap   r4,r4
        cmpobg.f r4,r5,.rmread_50       # Jif invalid region # specified
        ldob    cor_mstate(g0),r6       # r6 = local cor_mstate value
        ld      cor_rmaptbl(g0),r7      # r7 = cor_rmaptbl value
        ldconst 0,r5                    # r5 = index into table being read
        st      r6,CMsp_rs_rmrd_mstate(g6) # save cor_mstate in response data
        ldconst SMTBLsize,r3            # r3 = size of segment map table
        cmpobe.f 0,r7,.rmread_500       # Jif no region map table defined
        ld      RM_tbl(r7)[r4*4],r7     # r7 = segment map table address for
                                        #  specified region map
        cmpobe.f 0,r7,.rmread_500       # Jif no segment map table defined
                                        #  for the specified map number
        ld      SM_cnt(r7),r6           # r6 = number of segments from table
        ldconst 16,r4                   # r4 = count of bytes being copied
                                        #  per loop
        bswap   r6,r6
        st      r6,CMsp_rs_rmrd_cnt(g6) # save number of segments count in
                                        #  response data
.rmread_300:
        ldq     SM_tbl(r7)[r5*1],r12    # r12-r15 = 16 segment bytes from
                                        #           table
        stq     r12,CMsp_rs_rmrd_table(g6)[r5*1] # save in response data
        addo    r4,r5,r5                # inc. table index value
        cmpobne.t r3,r5,.rmread_300     # Jif more segment table data needs
                                        #  to be copied to response data
        b       .rmread_900

.rmread_500:
        st      r7,CMsp_rs_rmrd_cnt(g6) # clear dirty segment count in
                                        #  response data
.rmread_550:
        st      r7,CMsp_rs_rmrd_table(g6)[r5*1] # clear segment word in
                                        #  response data table area
        addo    4,r5,r5                 # inc. table index value
        cmpobne.t r3,r5,.rmread_550     # Jif table has not been all
                                        #  cleared out
.rmread_900:
        ldq     DLM_srvr_ok,r4          # r4-r7 = good response header
        ldconst CMsp_rs_rmrd_size,r6    # r6 = remaining response message
                                        #  length
        bswap   r6,r6
        stq     r4,(g3)                 # save response message header
        lda     -ILTBIAS(g1),g1         # g1 = previous ILT nest level
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT completion handler routine
.rmread_1000:
        ret
#
#**********************************************************************
#
#  NAME: CMsp$rm_clear
#
#  PURPOSE:
#       To process inbound Clear Region/Segment Map Table datagram
#       messages.
#
#  DESCRIPTION:
#       Validates incoming datagram message and locates the associated
#       COR for the specified copy operation. If any errors detected,
#       returns the appropriate error response to the requestor. If no
#       errors are detected, it checks if the current cor_mstate is
#       either suspended or terminated. If not, it returns an error
#       to the requestor. Otherwise, it checks if the specified region
#       map number is defined and if so deallocates the map table,
#       clears it from the region map table and saves configuration to
#       reflect the region that no longer needs to be restored.
#
#  CALLING SEQUENCE:
#       call    CMsp$rm_clear
#
#  INPUT:
#       g1 = datagram ILT at nest level #4
#       g2 = local request message header address
#       g3 = local response message header address
#       g4 = request message buffer address (does NOT include header)
#       g5 = request message length (does NOT include header)
#       g6 = response message buffer address (does NOT include header)
#       g7 = response buffer length (does NOT include header)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All regs. can be destroyed.
#
#**********************************************************************
#
CMsp$rm_clear:
        call    CMsp$trans              # translate standard data fields
                                        #  in request message buffer
        ldconst CMsp_rq_rmclr_size,r5   # r5 = expected request message size
        cmpoble.t r5,g5,.rmclr_100      # Jif remaining request
                                        #  message length is correct
.rmclr_50:
        call    DLM$srvr_invparm        # return invalid parameter response
        b       .rmclr_1000             # and get out of here!

.rmclr_100:
        call    CM$chk_owner            # check if we're the current copy
                                        #  owner
        cmpobe  0,g0,.rmclr_1000        # Jif not the current copy owner
        ld      CMsp_rq_rmclr_reg(g4),r4 # r4 = specified region map number
        ldconst maxRMcnt-1,r5           # r5 = max. # regions supported - 1
        bswap   r4,r4
        cmpobg.f r4,r5,.rmclr_50        # Jif invalid region # specified
        ldob    cor_mstate(g0),r6       # r6 = local cor_mstate value
        cmpobe.t cormst_susp,r6,.rmclr_300 # Jif map state suspended
        cmpobe.t cormst_term,r6,.rmclr_300 # Jif map state terminated
        call    CMsp$srvr_rmact         # return region map active response
                                        #  to requestor
        b       .rmclr_1000

.rmclr_300:
        ld      cor_rmaptbl(g0),r6      # r6 = region map table address
        cmpobe.f 0,r6,.rmclr_900        # Jif no region map table defined
        ld      RM_tbl(r6)[r4*4],r7     # r7 = segment map table address of
                                        #  specified segment map
        cmpobe.f 0,r7,.rmclr_900        # Jif no segment map table defined
                                        #  for specified segment map #
        ldconst 0,r3
        mov     g1,r5                   # save g1
        st      r3,RM_tbl(r6)[r4*4]     # clear segment map table from
                                        #  region map table
        mov     r7,g1                   # g1 = segment map table being cleared
.ifdef M4_DEBUG_SM
c fprintf(stderr, "%s%s:%u put_sm 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_SM
c       put_sm(g1);                     # Deallocate segment map table
        mov     r5,g1                   # restore g1
        mov     g3,r7                   # save g3
        mov     g0,g3                   # g3 = COR address
        mov     r4,g0                   # g0 = region # being cleared
        call    CCSM$reg_sync           # clear region bit in state NVRAM
        mov     r7,g3                   # restore g3
.rmclr_900:
        ldq     DLM_srvr_ok,r4          # r4-r7 = good response header
        stq     r4,(g3)                 # save response message header
        lda     -ILTBIAS(g1),g1         # g1 = previous ILT nest level
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT completion handler routine
.rmclr_1000:
        ret
#
#**********************************************************************
#
#  NAME: CMsp$rm_check
#
#  PURPOSE:
#       To process inbound Check Region Map Table State/Status
#       datagram messages.
#
#  DESCRIPTION:
#       Validates incoming datagram message and locates the associated
#       COR for the specified copy operation. If any errors detected,
#       returns the appropriate error response to the requestor. If no
#       errors are detected, it packs the appropriate response data
#       and returns it to the requestor.
#
#  CALLING SEQUENCE:
#       call    CMsp$rm_check
#
#  INPUT:
#       g1 = datagram ILT at nest level #4
#       g2 = local request message header address
#       g3 = local response message header address
#       g4 = request message buffer address (does NOT include header)
#       g5 = request message length (does NOT include header)
#       g6 = response message buffer address (does NOT include header)
#       g7 = response buffer length (does NOT include header)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All regs. can be destroyed.
#
#**********************************************************************
#
CMsp$rm_check:
        call    CMsp$trans              # translate standard data fields
                                        #  in request message buffer
        ldconst CMsp_rq_rmchk_size,r5   # r5 = expected request message size
        cmpoble.t r5,g5,.rmchk_100      # Jif remaining request
                                        #  message length is correct
.rmchk_50:
        call    DLM$srvr_invparm        # return invalid parameter response
        b       .rmchk_1000             # and get out of here!

.rmchk_100:
        call    CM$chk_owner            # check if we're the current copy
                                        #  owner
        cmpobe  0,g0,.rmchk_1000        # Jif not the current copy owner
        ldconst CMsp_rs_rmchk_size,r4   # r4 = expected remaining response
                                        #  message length
        cmpobl.f g7,r4,.rmchk_50        # Jif size < expected
#
# --- Set up to check if any segment tables are dirty
#
        mov     g3,r4                   # save g3
        mov     g0,g3                   # g3 = COR address being processed
        call    CM$update_rmap          # update region/segment map
                                        # cor_rmaptbl = 0 if no dirty
                                        #  segments found
        mov     r4,g3                   # restore g3
        ldob    cor_mstate(g0),r5       # r5 = local cor_mstate value
        ld      cor_rmaptbl(g0),r4      # r4 = region map table pointer
        ldconst maxRMcnt,r6             # r6 = max. # regions supported
        st      r5,CMsp_rs_rmchk_mstate(g6) # save cor_mstate in response data
        cmpobne.t 0,r4,.rmchk_500       # Jif region map table defined
.rmchk_300:
        subo    1,r6,r6                 # dec. region count
        st      r4,CMsp_rs_rmchk_cnts(g6)[r6*4] # clear count field in
                                        #  response field
        cmpobne.t 0,r6,.rmchk_300       # Jif more region counts to clear
        b       .rmchk_900              # finish up and get out of here!

.rmchk_500:
        subo    1,r6,r6                 # dec. region count
        ld      RM_tbl(r4)[r6*4],r7     # r7 = segment map table address
        cmpobe.t 0,r7,.rmchk_550        # Jif no segment map table defined
        ld      SM_cnt(r7),r7           # r7 = current segment count
.rmchk_550:
        bswap   r7,r7                   # put in little-endian format
        st      r7,CMsp_rs_rmchk_cnts(g6)[r6*4] # clear count field in
                                        #  response field
        cmpobne.t 0,r6,.rmchk_500       # Jif more region counts to clear
.rmchk_900:
        ldq     DLM_srvr_ok,r4          # r4-r7 = good response header
        ldconst CMsp_rs_rmchk_size,r6   # r6 = remaining response message
                                        #  length
        bswap   r6,r6
        stq     r4,(g3)                 # save response message header
        lda     -ILTBIAS(g1),g1         # g1 = previous ILT nest level
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT completion handler routine

.rmchk_1000:
        ret
#
#**********************************************************************
#
#  NAME: CMsp$op_susp
#
#  PURPOSE:
#       To process inbound Suspend Copy Operation datagram messages.
#
#  DESCRIPTION:
#       This routine locates the specified copy operation and if found
#       will perform the necessary operations to place the copy operation
#       in a suspended state. If the copy operation is not found, or some
#       other error is detected in the request data, the appropriate error
#       response message will be returned to the requestor.
#
#  CALLING SEQUENCE:
#       call    CMsp$op_susp
#
#  INPUT:
#       g1 = datagram ILT at nest level #4
#       g2 = local request message header address
#       g3 = local response message header address
#       g4 = request message buffer address (does NOT include header)
#       g5 = request message length (does NOT include header)
#       g6 = response message buffer address (does NOT include header)
#       g7 = response buffer length (does NOT include header)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All regs. can be destroyed.
#
#**********************************************************************
#
CMsp$op_susp:
        call    CMsp$trans              # translate standard data fields
                                        #  in request message buffer
        ldconst CMsp_rq_opsusp_size,r5  # r5 = expected request message size
        cmpoble.t r5,g5,.opsusp_100     # Jif remaining request
                                        #  message length is correct
        call    DLM$srvr_invparm        # return invalid parameter response
        b       .opsusp_1000            # and get out of here!

.opsusp_100:
        call    CM$chk_owner            # check if we're the current copy
                                        #  owner
        cmpobe  0,g0,.opsusp_1000       # Jif not the current copy owner
        call    cmsp$up_info            # update copy operation information
                                        #  as needed
                                        # g0 = COR address if update
                                        #      processing was successful
                                        # g8 = TRUE/FALSE indicator if copy
                                        #      devices changed
        cmpobe.f 0,g0,.opsusp_1000      # Jif error indicated processing
                                        #  copy operation information
        ldob    CMsp_rq_opsusp_cstate(g4),r4 # r4 = requestor's copy state code
        ldob    cor_cstate(g0),r5       # r5 = my current copy state code
        cmpobe.t r4,r5,.opsusp_200      # Jif copy state has not changed
        stob    r4,cor_cstate(g0)       # save updated copy state code in COR
        ldconst TRUE,g8                 # g8 = TRUE indicator that non-
                                        #      volatile save needed
.opsusp_200:
        ldob    CMsp_rq_opsusp_crstate(g4),r4 # r4 = requestor's copy reg.
                                        #          state code
        ldob    cor_crstate(g0),r5      # r5 = my current copy reg. state code
        cmpobe.t r4,r5,.opsusp_300      # Jif copy reg. state has not changed
#
# --- Copy registration state has changed. Process incoming state.
#
        cmpobe.f corcrst_remsusp,r5,.opsusp_300 # Jif local is remote
                                        #  suspended
        stob    r4,cor_crstate(g0)      # save updated copy reg. state code
                                        #  in COR
        ldconst TRUE,g8                 # g8 = TRUE indicator that non-
                                        #      volatile save needed
        ldconst cormst_act,r5           # r5 = active map state code
        lda     CM$wp2_suspend,r6       # r6 = suspended write update phase
                                        #      2 handler routine
        stob    r5,cor_mstate(g0)       # set map state to active
        ld      cor_scd(g0),r4          # r4 = assoc. SCD address
        cmpobe.f 0,r4,.opsusp_260       # Jif no SCD assoc. with COR
        st      r6,scd_p2hand(r4)       # save phase 2 update handler routine
                                        #  in SCD
.opsusp_260:
        ld      cor_dcd(g0),r4          # r4 = assoc. DCD address
        cmpobe.f 0,r4,.opsusp_270       # Jif no DCD assoc. with COR
        st      r6,dcd_p2hand(r4)       # save phase 2 update handler routine

.opsusp_270:
        mov     g3,r4                   # save g3
        mov     g0,g3                   # g3 = COR address
        call    CM$mmc_sflag            # process suspended flag for MMC
        mov     r4,g3                   # restore g3
                                        #  in DCD
.opsusp_300:
        cmpobne.t TRUE,g8,.opsusp_400   # Jif non-volatile save not needed
        mov     g3,r6                   # save g3
        mov     g0,g3                   # g3 = assoc. COR address
        call    CCSM$cs_chged_w_msg     # generate a copy state changed event
        mov     r6,g3                   # restore g3
.opsusp_400:
#
# --- Request processed successfully. Return good response message.
#
        ldq     DLM_srvr_ok,r4          # r4-r7 = good response header
        stq     r4,(g3)                 # save response message header
        lda     -ILTBIAS(g1),g1         # g1 = previous ILT nest level
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT completion handler routine
.opsusp_1000:
        ret
#
#**********************************************************************
#
#  NAME: CMsp$op_resume
#
#  PURPOSE:
#       To process inbound Resume Copy Operation datagram messages.
#
#  DESCRIPTION:
#       This routine locates the specified copy operation and if found
#       will perform the necessary operations to place the copy operation
#       in an active state. If the copy operation is not found, or some
#       other error is detected in the request data, the appropriate error
#       response message will be returned to the requestor.
#
#  CALLING SEQUENCE:
#       call    CMsp$op_resume
#
#  INPUT:
#       g1 = datagram ILT at nest level #4
#       g2 = local request message header address
#       g3 = local response message header address
#       g4 = request message buffer address (does NOT include header)
#       g5 = request message length (does NOT include header)
#       g6 = response message buffer address (does NOT include header)
#       g7 = response buffer length (does NOT include header)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All regs. can be destroyed.
#
#**********************************************************************
#
CMsp$op_resume:
        call    CMsp$trans              # translate standard data fields
                                        #  in request message buffer
        ldconst CMsp_rq_opresm_size,r5  # r5 = expected request message size
        cmpoble.t r5,g5,.opresm_100     # Jif remaining request
                                        #  message length is correct
        call    DLM$srvr_invparm        # return invalid parameter response
        b       .opresm_1000            # and get out of here!

.opresm_100:
        call    CM$chk_owner            # check if we're the current copy
                                        #  owner
        cmpobe  0,g0,.opresm_1000       # Jif not the current copy owner
        call    cmsp$up_info            # update copy operation information
                                        #  as needed
                                        # g0 = COR address if update
                                        #      processing was successful
                                        # g8 = TRUE/FALSE indicator if copy
                                        #      devices changed
        cmpobe.f 0,g0,.opresm_1000      # Jif error indicated processing
                                        #  copy operation information
!       ldob    CMsp_rq_opresm_cstate(g4),r4 # r4 = requestor's copy state code
        ldob    cor_cstate(g0),r5       # r5 = my current copy state code
        cmpobe.t r4,r5,.opresm_200      # Jif copy state has not changed
        stob    r4,cor_cstate(g0)       # save updated copy state code in COR
        ldconst TRUE,g8                 # g8 = TRUE indicator that non-
                                        #      volatile save needed
.opresm_200:
!       ldob    CMsp_rq_opresm_crstate(g4),r4 # r4 = requestor's copy reg.
                                        #          state code
        ldob    cor_crstate(g0),r5      # r5 = my current copy reg. state code
        cmpobe.t r4,r5,.opresm_300      # Jif copy reg. state has not changed
#
# --- Copy registration state has changed. Process incoming state.
#
        ldconst TRUE,g8                 # g8 = TRUE indicator that non-
                                        #      volatile save needed
        stob    r4,cor_crstate(g0)      # save updated copy reg. state code
                                        #  in COR
        mov     g3,r4                   # save g3
        mov     g0,g3                   # g3 = COR address
        call    CM$mmc_sflag            # process suspended flag for MMC
        mov     r4,g3                   # restore g3

.opresm_300:
        cmpobne.t TRUE,g8,.opresm_400   # Jif non-volatile save not needed
        mov     g3,r6                   # save g3
        mov     g0,g3                   # g3 = assoc. COR address
        call    CCSM$cs_chged_w_msg     # generate a copy state changed event
        mov     r6,g3                   # restore g3
.opresm_400:
#
# --- Request processed successfully. Return good response message.
#
        ldq     DLM_srvr_ok,r4          # r4-r7 = good response header
        stq     r4,(g3)                 # save response message header
        lda     -ILTBIAS(g1),g1         # g1 = previous ILT nest level
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT completion handler routine
.opresm_1000:
        ret
#
#**********************************************************************
#
#  NAME: CMsp$op_dmove
#
#  PURPOSE:
#       To process inbound Copy Device Moved datagram messages.
#
#  DESCRIPTION:
#       This routine validates the inbound datagram and ignores the
#       request message if any errors are detected. Otherwise, it
#       locates the specified copy operation if defined and checks
#       if the copy manager function resides on the local node. If
#       not, it ignores the request. If it does, it schedules a local
#       poll operation for the specified copy operation. If any errors
#       are detected, it simply ignores the request.
#
#  CALLING SEQUENCE:
#       call    CMsp$op_dmove
#
#  INPUT:
#       g1 = datagram ILT at nest level #4
#       g2 = local request message header address
#       g3 = local response message header address
#       g4 = request message buffer address (does NOT include header)
#       g5 = request message length (does NOT include header)
#       g6 = response message buffer address (does NOT include header)
#       g7 = response buffer length (does NOT include header)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All regs. can be destroyed.
#
#**********************************************************************
#
CMsp$op_dmove:
        call    CMsp$trans              # translate standard data fields
                                        #  in request message buffer
        ldconst CMsp_rq_opdmov_size,r5  # r5 = expected request message size
        cmpobg.f r5,g5,.opdmov_900      # Jif remaining request
                                        #  message length is not correct
        call    CM$chk_owner            # check if we're the current copy
                                        #  owner
        cmpobe  0,g0,.opdmov_1000       # Jif not the current copy owner
        mov     g3,r12                  # save g3
        mov     g4,r13                  # save g4
        ld      cor_cm(g0),g4           # g4 = assoc. CM address
        cmpobe.f 0,g4,.opdmov_500       # Jif copy manager function does not
                                        #  exist on this node
        mov     g0,g3                   # g3 = COR address
        call    CM$pksnd_local_poll     # issue directive to perform a local
                                        #  poll operation
.opdmov_500:
        mov     r12,g3                  # restore g3
        mov     r13,g4                  # restore g4
.opdmov_900:
        ldq     DLM_srvr_ok,r4          # r4-r7 = good response header
        stq     r4,(g3)                 # save response message header
        lda     -ILTBIAS(g1),g1         # g1 = previous ILT nest level
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT completion handler routine
.opdmov_1000:
        ret
#
#******************************************************************************
#
#  NAME:  CMsp$srvr_nosvlink
#
#  PURPOSE:
#       Pack and return a VLink not established to specified source
#       copy device function code response to requestor.
#
#  DESCRIPTION:
#       Packs the VLink not established to specified source copy device
#       function code response header in the local response header and
#       returns the datagram ILT back to the requestor.
#
#  CALLING SEQUENCE:
#       call    CMsp$srvr_nosvlink
#
#  INPUT:
#       g1 = datagram ILT at nest level #4
#       g3 = local response message header address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Reg. g1 destroyed.
#
#******************************************************************************
#
CMsp$srvr_nosvlink:
        ldq     CMsp_srvr_nosvlink,r4   # r4-r7 = response header for VLink
                                        #  not established to specified source
                                        #  copy device function code
        stq     r4,(g3)                 # save response header in memory
        lda     -ILTBIAS(g1),g1         # g1 = previous ILT nest level
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT completion handler routine
        ret
#
#******************************************************************************
#
#  NAME:  CMsp$srvr_nodvlink
#
#  PURPOSE:
#       Pack and return a VLink not established to specified destination
#       copy device function code response to requestor.
#
#  DESCRIPTION:
#       Packs the VLink not established to specified dest. copy device
#       function code response header in the local response header and
#       returns the datagram ILT back to the requestor.
#
#  CALLING SEQUENCE:
#       call    CMsp$srvr_nodvlink
#
#  INPUT:
#       g1 = datagram ILT at nest level #4
#       g3 = local response message header address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Reg. g1 destroyed.
#
#******************************************************************************
#
CMsp$srvr_nodvlink:
        ldq     CMsp_srvr_nodvlink,r4   # r4-r7 = response header for VLink
                                        #  not established to specified dest.
                                        #  copy device function code
        stq     r4,(g3)                 # save response header in memory
        lda     -ILTBIAS(g1),g1         # g1 = previous ILT nest level
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT completion handler routine
        ret
#
#******************************************************************************
#
#  NAME:  CMsp$srvr_inuse
#
#  PURPOSE:
#       Pack and return a specified copy device in use function
#       code response to requestor.
#
#  DESCRIPTION:
#       Packs the specified copy device in use function code response
#       header in the local response header and returns the datagram
#       ILT back to the requestor.
#
#  CALLING SEQUENCE:
#       call    CMsp$srvr_inuse
#
#  INPUT:
#       g1 = datagram ILT at nest level #4
#       g3 = local response message header address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Reg. g1 destroyed.
#
#******************************************************************************
#
CMsp$srvr_inuse:
        ldq     CMsp_srvr_inuse,r4      # r4-r7 = response header for
                                        #  specified copy device in use
                                        #  function code
        stq     r4,(g3)                 # save response header in memory
        lda     -ILTBIAS(g1),g1         # g1 = previous ILT nest level
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT completion handler routine
        ret
#
#******************************************************************************
#
#  NAME:  CMsp$srvr_nocopy
#
#  PURPOSE:
#       Pack and return a specified copy operation not defined function
#       code response to requestor.
#
#  DESCRIPTION:
#       Packs the specified copy operation not defined function code response
#       header in the local response header and returns the datagram
#       ILT back to the requestor.
#
#  CALLING SEQUENCE:
#       call    CMsp$srvr_nocopy
#
#  INPUT:
#       g1 = datagram ILT at nest level #4
#       g3 = local response message header address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Reg. g1 destroyed.
#
#******************************************************************************
#
CMsp$srvr_nocopy:
        ldq     CMsp_srvr_nocopy,r4     # r4-r7 = response header for
                                        #  specified copy operation not
                                        #  defined function code
        stq     r4,(g3)                 # save response header in memory
        lda     -ILTBIAS(g1),g1         # g1 = previous ILT nest level
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT completion handler routine
        ret
#
#******************************************************************************
#
#  NAME:  CMsp$srvr_dirty
#
#  PURPOSE:
#       Pack and return a dirty region map table function
#       code response to requestor.
#
#  DESCRIPTION:
#       Packs the dirty region map table function code response
#       header in the local response header and returns the datagram
#       ILT back to the requestor.
#
#  CALLING SEQUENCE:
#       call    CMsp$srvr_dirty
#
#  INPUT:
#       g1 = datagram ILT at nest level #4
#       g3 = local response message header address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Reg. g1 destroyed.
#
#******************************************************************************
#
CMsp$srvr_dirty:
        ldq     CMsp_srvr_dirty,r4      # r4-r7 = response header for
                                        #  dirty region map table
                                        #  function code
        stq     r4,(g3)                 # save response header in memory
        lda     -ILTBIAS(g1),g1         # g1 = previous ILT nest level
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT completion handler routine
        ret
#
#******************************************************************************
#
#  NAME:  CMsp$srvr_rmact
#
#  PURPOSE:
#       Pack and return a region map state still active function
#       code response to requestor.
#
#  DESCRIPTION:
#       Packs the region map state still active function code response
#       header in the local response header and returns the datagram
#       ILT back to the requestor.
#
#  CALLING SEQUENCE:
#       call    CMsp$srvr_rmact
#
#  INPUT:
#       g1 = datagram ILT at nest level #4
#       g3 = local response message header address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Reg. g1 destroyed.
#
#******************************************************************************
#
CMsp$srvr_rmact:
        ldq     CMsp_srvr_rmact,r4      # r4-r7 = response header for
                                        #  region map still active
                                        #  function code
        stq     r4,(g3)                 # save response header in memory
        lda     -ILTBIAS(g1),g1         # g1 = previous ILT nest level
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT completion handler routine
        ret
#
#******************************************************************************
#
#  NAME:  CMsp$srvr_insres
#
#  PURPOSE:
#       Pack and return an insufficient resources function
#       code response to requestor.
#
#  DESCRIPTION:
#       Packs the insufficient resources function code response
#       header in the local response header and returns the datagram
#       ILT back to the requestor.
#
#  CALLING SEQUENCE:
#       call    CMsp$srvr_insres
#
#  INPUT:
#       g1 = datagram ILT at nest level #4
#       g3 = local response message header address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Reg. g1 destroyed.
#
#******************************************************************************
#
CMsp$srvr_insres:
        ldq     CMsp_srvr_insres,r4     # r4-r7 = response header for
                                        #  insufficient resources
                                        #  function code
        stq     r4,(g3)                 # save response header in memory
        lda     -ILTBIAS(g1),g1         # g1 = previous ILT nest level
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT completion handler routine
        ret
#
#******************************************************************************
#
#  NAME:  CMsp$trans
#
#  PURPOSE:
#       Translate standard data fields for incoming CMsp request
#       datagram messages.
#
#  DESCRIPTION:
#       This routine performs bswap's on the following standard
#       data fields of an incoming CMsp request datagram message:
#
#       CMsp_rq_xxxxx_rid
#       CMsp_rq_xxxx_rcsn
#       CMsp_rq_xxxx_rssn
#       CMsp_rq_xxxx_rdsn
#
#  CALLING SEQUENCE:
#       call    CMsp$trans
#
#  INPUT:
#       g4 = request message buffer address (does NOT include header)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
CMsp$trans:
!       ldq     CMsp_rq_opnew_rid(g4),r4 # r4 = copy reg. ID
                                        # r5 = copy MAG serial #
                                        # r6 = copy MAG source cluster #
                                        #      copy MAG source VDisk #
                                        #      copy MAG dest. cluster #
                                        #      copy MAG dest. VDisk #
                                        # r7 = source MAG serial #
!       ldl     CMsp_rq_opnew_rdsn(g4),r8 # r8 = dest. MAG serial #
                                        # r9 = source MAG cluster #
                                        #      source MAG VDisk #
                                        #      dest. MAG cluster #
                                        #      dest. MAG VDisk #
        bswap   r4,r4                   # bswap copy reg. ID
        bswap   r5,r5                   # bswap copy MAG serial #
        bswap   r7,r7                   # bswap source MAG serial #
        bswap   r8,r8                   # bswap dest. MAG serial #
!       stq     r4,CMsp_rq_opnew_rid(g4)
!       stl     r8,CMsp_rq_opnew_rdsn(g4)
        ret
#
#**********************************************************************
#
#  NAME: cm$pollexec
#
#  PURPOSE:
#       To provide a means of checking the status/state of copy operations
#       periodically checking the validity of active copy operations.
#
#  DESCRIPTION:
#       This process periodically (once a second) updates the timer field
#       in all of the CORs queued on the active COR queue. Every 30 seconds
#       this process checks the status of the local copy operations (those
#       that have a copy manager function on this MAGNITUDE) on any
#       associated remote MAGNITUDEs. It processes any errors returned
#       during these poll requests to remote MAGNITUDEs and also checks
#       if the remote copy operations need to be updated with current
#       copy operation information and does so if necessary. Every 60
#       seconds this process checks the status of the remote copy operations
#       (those that do not have a local copy manager function on this
#       MAGNITUDE) with that of the copy manager MAGNITUDE. It processes
#       any errors returned during these poll requests and also checks
#       if the copy operation information needs to be updated and does so
#       if necessary.
#
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
cm$pollexec:
        ldconst 1000,g0                 # delay for 1 sec.
        call    K$twait
#
# --- Update the timer #1 field for all active CORs
#
        ld      CM_cor_act_que,g0       # g0 = first active COR on list
        cmpobe.f 0,g0,.pollex_1000      # Jif no active CORs on list
        ld      K_ficb,r3               # r3 = FICB address
        ld      fi_vcgid(r3),r3         # r3 = my serial #
.pollex_100:
        ldos    cor_tmr1(g0),r4         # r4 = timer #1 value
        addo    1,r4,r4                 # inc. timer #1 value
        stos    r4,cor_tmr1(g0)         # save updated timer #1 value
        ld      cor_link(g0),g0         # g0 = next COR on list
        cmpobne.t 0,g0,.pollex_100      # Jif more CORs to update
        ldconst 0,r8                    # r8 = 0
#
# --- Check for any local CORs that need to poll associated remote
#       MAGNITUDES. A local COR is defined as a copy operation where
#       the copy manager function is local to the MAGNITUDE.
#
.pollex_190:
        ld      CM_cor_act_que,g3       # g3 = first active COR on list
        cmpobe.f 0,g3,.pollex_300       # Jif no active CORs on list
.pollex_200:
        ldconst 30,r15                  # r15 = local COR timeout value
        ld      cor_cm(g3),g4           # g4 = assoc. CM address
        cmpobe.f 0,g4,.pollex_240       # Jif not a local COR
        ldos    cor_tmr1(g3),r4         # r4 = timer #1 value
# If alink, wait longer.
c if (((COR*)g3)->destvdd != NULL) {
c   if (BIT_TEST((((COR*)g3)->destvdd)->attr, VD_BVLINK) && (BIT_TEST((((COR*)g3)->destvdd)->attr, VD_BASYNCH))) {
c     r15 = 120;                        # Make 2 minutes for alink.
c   }
c }
        cmpobg.t r15,r4,.pollex_240     # Jif timer #1 has not expired
#
# --- A local COR has timed out. Check if any remote MAGNITUDES
#       are associated with this copy operation. If so, check if
#       a poll request has already been issued to the copy manager
#       and has not been completed yet. If not, issue a poll request
#       to the copy manager. If so, simply go on to the next copy
#       operation on the list.
#
        ld      cor_rcsn(g3),r4         # r4 = copy MAG serial #
        ld      cor_rssn(g3),r5         # r5 = source MAG serial #
        ld      cor_rdsn(g3),r6         # r6 = dest. MAG serial #
        stos    r8,cor_tmr1(g3)         # clear/reset timer #1
        cmpobe.t r4,r5,.pollex_220      # Jif source copy device is local
.pollex_210:
        ldob    cor_flags(g3),r7        # r7 = flags byte from COR
        bbs.t   CFLG_B_POLL_REQ,r7,.pollex_240  # Jif outstanding poll request
                                        #  indicated
        setbit  CFLG_B_POLL_REQ,r7,r7   # Set outstanding poll request flag
        stob    r7,cor_flags(g3)        # save updated flags byte
        call    cm$pksnd_local_poll     # pack and send a local COR poll
                                        #  request to the corresponding
                                        #  copy manager
        b       .pollex_190             # check for any more local CORs
                                        #  that have timed out
.pollex_220:
        cmpobne.f r4,r6,.pollex_210     # Jif destination copy device is
                                        #  remote
#
# --- This copy operation does not have any remote MAGNITUDES associated
#       with it. Normally, no polling operation is necessary. However,
#       check if the copy operation registration state is auto-suspended
#       and if so force a local poll request to facilitate auto-resuming
#       the copy operation.
#
        ldob    cor_crstate(g3),r7      # r7 = cor_crstate
        cmpobe.f corcrst_autosusp,r7,.pollex_210 # Jif auto-suspended state
.pollex_240:
        ld      cor_link(g3),g3         # g3 = next COR on active list
        cmpobne.t 0,g3,.pollex_200      # Jif another COR to process
.pollex_300:
#
# --- Check for any remote CORs that need to poll their associated
#       copy manager MAGNITUDE.
#
        ldconst 60,r15                  # r15 = remote COR timeout value
.pollex_310:
        ld      CM_cor_act_que,g3       # g3 = first active COR on list
        cmpobe.f 0,g3,.pollex_1000      # Jif no active CORs on list
.pollex_320:
        ld      cor_cm(g3),r4           # r4 = assoc. CM address
                                        #      0 indicates a remote COR
        cmpobne.t 0,r4,.pollex_340      # Jif a local COR
        ldos    cor_tmr1(g3),r4         # r4 = timer #1 value
        cmpobg.t r15,r4,.pollex_340     # Jif timer #1 has not expired
#
# --- A remote COR has timed out. Need to poll the main copy manager
#       MAGNITUDE and find out what's going on.
#
        stos    r8,cor_tmr1(g3)         # clear/reset timer #1
        call    cm$poll_remote_cor      # poll remote COR routine
        b       .pollex_310             # check for any more remote CORs
                                        #  that have timed out
.pollex_340:
        ld      cor_link(g3),g3         # g3 = next COR on active list
        cmpobne.t 0,g3,.pollex_320      # Jif another COR to process
.pollex_1000:
#
# Process other low cpu tasks that need to happen every now and then in the Backend.
# See "cdriver.as" process cd$exec (end) for FE equivalent.
#
c       Process_DMC_delayed();          # Do routine Process_DMC every 5 seconds.
#
        b       cm$pollexec
#
#******************************************************************************
#
#  NAME:  cmsp$up_info
#
#  PURPOSE:
#       Processes inbound datagram request messages to validate request
#       information and to update copy operation information as necessary.
#
#  DESCRIPTION:
#       This routine can be called by various inbound datagram handler
#       routines to validate the general copy operation information
#       contained in the request buffer and to make any required changes
#       to the local copy operation as specified in the request data.
#       If any errors are found in the request data, the appropriate
#       error response message will be formatted and returned to the
#       requestor and notification of this occurring will be returned
#       to the calling routine.
#
#       This routine processes the following data fields in the request
#       datagram buffer:
#
#       rid    - copy registration number
#       rcsn   - copy MAG serial number
#       rcscl  - copy MAG source cluster number
#       rcsvd  - copy MAG source VDisk number
#       rcdcl  - copy MAG dest. cluster number
#       rcdvd  - copy MAG dest. VDisk number
#       rssn   - source MAG serial number
#       rdsn   - dest. MAG serial number
#       rscl   - source MAG cluster number
#       rsvd   - source MAG VDisk number
#       rdcl   - dest. MAG cluster number
#       rdvd   - dest. MAG VDisk number
#       label  - user defined copy operation label
#       gid    - user defined group ID
#
#  CALLING SEQUENCE:
#       call    cmsp$up_info
#
#  INPUT:
#       g1 = datagram ILT at nest level #4
#       g3 = local response message header address
#       g4 = request message buffer address (does NOT include header)
#       g5 = request message length (does NOT include header)
#            Note: The request data must be processed by CM$sp_trans
#                  prior to calling this routine.
#
#  OUTPUT:
#       g0 = COR address of copy operation if no errors detected
#       g0 = 0 if an error was detected and an error response message
#              was returned to the original requestor.
#       g8 = TRUE/FALSE indicator if copy devices changed
#
#  REGS DESTROYED:
#       Reg. g0, g8 destroyed.
#
#******************************************************************************
#
cmsp$up_info:
        movq    g0,r12                  # save g0-g3
                                        # r13 = datagram ILT at nest level #4
                                        # r15 = local response message header
                                        #      address
        ldconst FALSE,g8                # preload non-volatile save needed
                                        #  flag
!       ld      CMsp_rq_opnew_rcsn(g4),r4 # r4 = specified copy MAG serial #
                                        #  in little-endian format
        ld      K_ficb,r3               # r3 = FICB address
        ld      fi_vcgid(r3),r3         # r3 = my serial #
        b       .upinfo_100

.upinfo_50:
        movq    r12,g0                  # restore g0-g3
        call    DLM$srvr_invparm        # return invalid parameter response
.upinfo_60:
        ldconst 0,r12                   # return 0 in g0 indicating error
                                        #  was detected
        b       .upinfo_1000            # and get out of here!

.upinfo_100:
#
# --- Check if specified copy operation is registered here.
#
!       ld      CMsp_rq_opnew_rid(g4),g0 # g0 = specified copy reg. ID
        mov     r4,g1                   # g1 = specified copy MAG serial #
        call    CM$find_cor_rid         # check if copy operation active with
                                        #  the specified ID
                                        # g0 = COR address if match found
        cmpobne.t 0,g0,.upinfo_150      # Jif copy operation active with
                                        #  the specified ID
#
# --- Specified copy operation is not defined here.
#
        movq    r12,g0                  # restore g0-g3
        call    CMsp$srvr_nocopy        # return specified copy operation
                                        #  not defined response
        b       .upinfo_60              # and get out of here!

.upinfo_150:
        mov     g0,r12                  # r12 = COR address of specified copy operation
#
# --- Check if any changes indicated in request datagram buffer
#       compared to current COR values.
#
        ldconst CMsp_rq_opnew_size,r8   # r8 = size of request datagram that
                                        #  includes a user defined label
        cmpobl.f g5,r8,.upinfo_170      # Jif label not included
        movt    g0,r8                   # save g0-g2
        lda     cor_label(g0),g0
        lda     CMsp_rq_opnew_label(g4),g1
        ldconst 16,g2
        call    CCSM$strcomp
        mov     g0,r11                  # r11 = compare result
        movt    r8,g0                   # restore g0-g2
        cmpobne TRUE,r11,.upinfo_160    # Jif copy label differs
        ldob    CMsp_rq_opchk_gid(g4),r8
        ldob    cor_gid(g0),r9
        cmpobe  r8,r9,.upinfo_170       # Jif cor_gid matches
.upinfo_160:
        ldq     CMsp_rq_opnew_label(g4),r8 # r9-r11 = user defined copy
                                        #             operation label
        stq     r8,cor_label(g0)        # save user defined copy operation
                                        #  label in COR
        ldob    CMsp_rq_opchk_gid(g4),r8 # r8 = user defined group ID
        stob    r8,cor_gid(g0)          # save user defined group ID
        mov     g3,r10                  # save g3
        mov     g0,g3                   # g3 = assoc. COR address
        call    CCSM$info_chged         # generate copy info. changed event
        mov     r10,g3                  # restore g3
.upinfo_170:
!       ldq     CMsp_rq_opnew_rcscl(g4),g0 # g0-g3 = values from datagram
        ldq     cor_rcscl(r12),r8       # r8-r11 = current COR values
        cmpobne.f g0,r8,.upinfo_200     # Jif not the same
        cmpobne.f g1,r9,.upinfo_200     # Jif not the same
        cmpobne.f g2,r10,.upinfo_200    # Jif not the same
        cmpobne.f g3,r11,.upinfo_200    # Jif not the same
        b       .upinfo_1000            # nothing has changed. Simply return
                                        #  to the calling routine.
#
# --- Some copy operation value has changed
#
.upinfo_200:
!       ld      CMsp_rq_opnew_rssn(g4),r5 # r5 = source MAG serial # in
                                        #  little-endian format
        ldconst 0,g2                    # g2 = VDD address if source copy
                                        #       device
!       ld      CMsp_rq_opnew_rdsn(g4),r6 # r6 = dest. MAG serial # in
                                        #  little-endian format
        ldconst 0,g3                    # g3 = VDD address if dest. copy
                                        #       device
        ldconst MAXVIRTUALS,r11         # r11 = max. VDisk #
        cmpobne.f r5,r3,.upinfo_220     # Jif not the source copy device
#
# --- We're defined as the source copy device
#
        ldob    CMsp_rq_opnew_rscl(g4),r7 # r7 = source MAG cluster #
        ldob    CMsp_rq_opnew_rsvd(g4),r8 # r8 = source MAG VDisk #
        shlo    8,r7,r7                 # form MS byte of VID
        addo    r8,r7,r7                # form VID
        cmpobge.f r7,r11,.upinfo_50     # Jif VDisk # invalid

        ld      V_vddindx[r7*4],g2      # g2 = corresponding VDD
        cmpobe.f 0,g2,.upinfo_205       # Jif no VDisk defined

        ldob    CMsp_rq_opnew_rcscl(g4),r7 # r7 = copy MAG source cluster #
        ldconst 0xff,r8                 # r8 = cluster # when VLink deleted
        cmpobe.f r7,r8,.upinfo_220      # Jif source VLink is deleted
#
# --- Validate that the copy manager MAG owns this VDisk
#
        ld      vd_vlinks(g2),r10       # r10 = first VLAR assoc. with
                                        #       specified source copy device
        cmpobne.t 0,r10,.upinfo_210     # Jif VLAR assoc. with device

.upinfo_205:
        movq    r12,g0                  # restore g0-g3
        call    CMsp$srvr_nosvlink      # format and send response to
                                        #  requestor
        b       .upinfo_60

.upinfo_210:
        ld      vlar_srcsn(r10),r7      # r7 = source MAG serial # assoc.
                                        #      with VLAR
        cmpobne.t r4,r7,.upinfo_215     # Jif not owned by copy manager MAG
        ldob    vlar_srccl(r10),r7      # r7 = source MAG cluster # assoc.
                                        #      with VLAR
        ldob    CMsp_rq_opnew_rcscl(g4),r8 # r8 = source MAG cluster # of
                                        #         copy manager
        cmpobne.f r7,r8,.upinfo_215     # Jif not owned by copy manager MAG
        ldob    vlar_srcvd(r10),r7      # r7 = source MAG VDisk # assoc.
                                        #      with VLAR
        ldob    CMsp_rq_opnew_rcsvd(g4),r8 # r8 = source MAG VDisk # of
                                        #         copy manager
        cmpobe.t r7,r8,.upinfo_220      # Jif owned by copy manager MAG
.upinfo_215:
        ld      vlar_link(r10),r10      # r10 = next VLAR on list
        cmpobne.f 0,r10,.upinfo_210     # Jif more VLARs assoc. with VDisk
        b       .upinfo_205             # reject copy request because copy
                                        #  manager does not own this VDisk
.upinfo_220:
        cmpobne.f r6,r3,.upinfo_240     # Jif not the dest. copy device
#
# --- We're defined as the destination copy device
#       Validate the destination copy device parameters
#
!       ldob    CMsp_rq_opnew_rdcl(g4),r7 # r7 = dest. MAG cluster #
!       ldob    CMsp_rq_opnew_rdvd(g4),r8 # r8 = dest. MAG VDisk #
        shlo    8,r7,r7                 # form MS byte of VID
        addo    r8,r7,r7                # form VID
        cmpobge.f r7,r11,.upinfo_50     # Jif VDisk # invalid

        ld      V_vddindx[r7*4],g3      # g3 = corresponding VDD
        cmpobe.f 0,g3,.upinfo_225       # Jif no VDisk defined

!       ldob    CMsp_rq_opnew_rcdcl(g4),r7 # r7 = copy MAG dest. cluster #
        ldconst 0xff,r8                 # r8 = cluster # when VLink deleted
        cmpobe.f r7,r8,.upinfo_240      # Jif dest. VLink is deleted
#
# --- Validate that the copy manager MAG owns this VDisk
#
        ld      vd_vlinks(g3),r10       # r10 = first VLAR assoc. with
                                        #       specified source copy device
        cmpobne.t 0,r10,.upinfo_230     # Jif VLAR assoc. with device
.upinfo_225:
        movq    r12,g0                  # restore g0-g3
        call    CMsp$srvr_nodvlink      # format and send response to
                                        #  requestor
        b       .upinfo_60

.upinfo_230:
        ld      vlar_srcsn(r10),r7      # r7 = source MAG serial # assoc.
                                        #      with VLAR
        cmpobne.t r4,r7,.upinfo_235     # Jif not owned by copy manager MAG
        ldob    vlar_srccl(r10),r7      # r7 = source MAG cluster # assoc.
                                        #      with VLAR
!       ldob    CMsp_rq_opnew_rcdcl(g4),r8 # r8 = source MAG cluster # of
                                        #         copy manager
        cmpobne.f r7,r8,.upinfo_235     # Jif not owned by copy manager MAG
        ldob    vlar_srcvd(r10),r7      # r7 = source MAG VDisk # assoc.
                                        #      with VLAR
!       ldob    CMsp_rq_opnew_rcdvd(g4),r8 # r8 = source MAG VDisk # of
                                        #         copy manager
        cmpobe.t r7,r8,.upinfo_240      # Jif owned by copy manager MAG
.upinfo_235:
        ld      vlar_link(r10),r10      # r10 = next VLAR on list
        cmpobne.f 0,r10,.upinfo_230     # Jif more VLARs assoc. with VDisk
        b       .upinfo_225             # reject copy request because copy
                                        #  manager does not own this VDisk
.upinfo_240:
        cmpobe.f g2,g3,.upinfo_50       # Jif source and dest. copy devices
                                        #  the same or neither is defined
        ld      cor_srcvdd(r12),r5      # r5 = source VDD address from COR
        ld      cor_destvdd(r12),r6     # r6 = dest. VDD address from COR
        cmpobe.t 0,g3,.upinfo_260       # Jif dest. VDD not specified
        cmpobe.t g3,r6,.upinfo_260      # Jif dest. VDD has not changed
#
# --- Check if new destination VDD is already a destination copy device
#       for another copy operation.
#
        ld      vd_dcd(g3),r7           # r7 = DCD associated with new dest.
                                        #      VDD specified in request
        cmpobe.t 0,r7,.upinfo_260       # Jif new dest. VDD does not have a
                                        #  copy operation already associated
                                        #  with it
        movq    r12,g0                  # restore g0-g3
        call    CMsp$srvr_inuse         # format and send response to
                                        #  requestor
        b       .upinfo_60

.upinfo_260:
#
# --- Update the source copy device information in the COR.
#
!       ldos    CMsp_rq_opnew_rcscl(g4),r7 # r7 = copy MAG source cluster #
                                        #  and VDisk #
        stos    r7,cor_rcscl(r12)       # save in COR
!       ld      CMsp_rq_opnew_rssn(g4),r7 # r7 = new source MAG serial #
        st      r7,cor_rssn(r12)        # save new source MAG serial #
!       ldos    CMsp_rq_opnew_rscl(g4),r7 # r7 = new source cluster/VDisk #
        stos    r7,cor_rscl(r12)        # save new source cluster/VDisk #
        cmpobne.f g2,r5,.upinfo_300     # Jif source VDD has changed
        ldconst 0,r5                    # r5 = old SCD address that needs to
                                        #      be terminated (0 indicates no
                                        #      SCD needs to be terminated)
        b       .upinfo_400             # and process changes to dest. copy
                                        #  device
#
# --- A change in the source VDD is indicated --------------------------------
#
.upinfo_300:
        cmpobe.f 0,r5,.upinfo_320       # Jif source was not defined before
        ldconst 0,r7
        ld      cor_scd(r12),r5         # r5 = old SCD address that needs to
                                        #      be terminated
        st      r7,cor_scd(r12)         # clear SCD address from COR
        st      r7,cor_srcvdd(r12)      # clear source VDD address from COR
.upinfo_320:
        cmpobe.f 0,g2,.upinfo_400       # Jif source VDD not being defined
#
# --- A new source VDD has been defined. Set up a new SCD to service it ------
#
c       g0 = get_scd();                 # Allocate a SCD data structure
.ifdef M4_DEBUG_SCD
c fprintf(stderr, "%s%s:%u get_scd 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif # M4_DEBUG_SCD
        st      r12,scd_cor(g0)         # save COR address in SCD
        st      g0,cor_scd(r12)         # save SCD address in COR
        st      g2,scd_vdd(g0)          # save source VDD address in SCD
        st      g2,cor_srcvdd(r12)      # save source VDD address in COR
        ldconst scdt_remote,r7          # r7 = scd_type code
        stob    r7,scd_type(g0)         # save scd_type code
        ldob    cor_mstate(r12),r8      # r8 = region/segment map table
                                        #      state code
        lda     CM$wp2_null,r10         # r10 = phase 2 write update handler
                                        #       routine
        cmpobne.t cormst_susp,r8,.upinfo_330 # Jif not in suspended state
        lda     CM$wp2_suspend,r10      # r10 = phase 2 write update handler
                                        #       routine
.upinfo_330:
        st      r10,scd_p2hand(g0)      # save phase 2 write update handler
                                        #  routine in SCD
        call    CM$act_scd              # activate SCD
.upinfo_400:
#
# --- Update the destination copy device information in the COR.
#
!       ldos    CMsp_rq_opnew_rcdcl(g4),r7 # r7 = copy MAG dest. cluster #
                                        #  and VDisk #
        stos    r7,cor_rcdcl(r12)       # save in COR
!       ld      CMsp_rq_opnew_rdsn(g4),r7 # r7 = new dest. MAG serial #
        st      r7,cor_rdsn(r12)        # save new dest. MAG serial #
!       ldos    CMsp_rq_opnew_rdcl(g4),r7 # r7 = new dest. cluster/VDisk #
        stos    r7,cor_rdcl(r12)        # save new dest. cluster/VDisk #
        cmpobne.f g3,r6,.upinfo_420     # Jif dest. VDD has changed
        ldconst 0,r6                    # r6 = old DCD address that needs to
                                        #      be terminated (0 indicates no
                                        #      DCD needs to be terminated)
        b       .upinfo_500             # and continue processing request
#
# --- A change in the destination VDD is indicated ---------------------------
#
.upinfo_420:
        cmpobe.f 0,r6,.upinfo_440       # Jif dest. was not defined before
        ldconst 0,r7
        ld      cor_dcd(r12),r6         # r6 = old DCD address that needs to
                                        #      be terminated
        st      r7,cor_dcd(r12)         # clear DCD address from COR
        st      r7,cor_destvdd(r12)     # clear dest. VDD address from COR
.upinfo_440:
        cmpobe.f 0,g3,.upinfo_500       # Jif dest. VDD not being defined
c       g0 = get_dcd();                 # Allocate a DCD data structure
.ifdef M4_DEBUG_DCD
c fprintf(stderr, "%s%s:%u get_dcd 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif # M4_DEBUG_DCD
        st      r12,dcd_cor(g0)         # save COR address in DCD
        st      g0,cor_dcd(r12)         # save DCD address in COR
        st      g3,dcd_vdd(g0)          # save dest. VDD address in DCD
        st      g3,cor_destvdd(r12)     # save dest. VDD address in COR
        ldconst dcdt_remote,r7          # r7 = dcd_type code
        stob    r7,dcd_type(g0)         # save dcd_type code
        ldob    cor_mstate(r12),r8      # r8 = region/segment map table
                                        #      state code
        lda     CM$wp2_null,r10         # r10 = phase 2 write update handler
                                        #       routine
        cmpobne.t cormst_susp,r8,.upinfo_450 # Jif not in suspended state
        lda     CM$wp2_suspend,r10      # r10 = phase 2 write update handler
                                        #       routine
.upinfo_450:
        st      r10,dcd_p2hand(g0)      # save phase 2 write update handler
                                        #  routine in DCD
        call    CM$act_dcd              # activate DCD
.upinfo_500:
#
# --- Check if any old SCD/DCD needs to be terminated ------------------------
#
#       r5 = old SCD address that needs to be terminated
#       r6 = old DCD address that needs to be terminated
#
        cmpobe.f 0,r5,.upinfo_520       # Jif no old SCD needs to be
                                        #  terminated
        mov     r5,g0                   # g0 = SCD address to deactivate
        call    CM$deact_scd            # deactivate SCD from service
.ifdef M4_DEBUG_SCD
c fprintf(stderr, "%s%s:%u put_scd 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif # M4_DEBUG_SCD
c       put_scd(g0);                    # Deallocate a SCD data structure
.upinfo_520:
        cmpobe.f 0,r6,.upinfo_600       # Jif no old DCD needs to be
                                        #  terminated
        mov     r6,g0                   # g0 = DCD address to deactivate
        call    CM$deact_dcd            # deactivate DCD from service
.ifdef M4_DEBUG_DCD
c fprintf(stderr, "%s%s:%u put_dcd 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif # M4_DEBUG_DCD
c       put_dcd(g0);                    # Deallocate a DCD data structure
.upinfo_600:
        ldconst TRUE,g8                 # return copy devices changed
                                        #  flag indicator
        mov     r12,g3                  # g3 = COR address being processed
        call    CCSM$cd_moved           # tell the rest of the universe
                                        #  about the copy device changes
.upinfo_1000:
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: CM$get_cor_rid
#
#  PURPOSE:
#       This routine allocates a unique copy registration ID.
#
#  DESCRIPTION:
#       This routine uses the next available copy registration ID
#       and checks if this ID is in use (i.e. the copy registration
#       IDs have wrapped). If the ID is not in use, it returns the ID
#       to the calling routine and increments the next available ID
#       for the next time. If the ID is in use, it increments the ID
#       and then checks if that ID is in use. It continues to do this
#       until it finds the next available unused copy registration ID
#       to return to the calling routine.
#
#  CALLING SEQUENCE:
#       call    CM$get_cor_rid
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       g0 = copy registration ID to use
#
#  REGS DESTROYED:
#       Reg. g0 destroyed.
#
#**********************************************************************
#
CM$get_cor_rid:
        movl    g0,r14                  # save g0-g1
        ld      K_ficb,g1               # g1 = FICB address
        ld      CM_cor_rid,g0           # g0 = next available copy reg. ID
        ld      fi_vcgid(g1),g1         # g1 = my serial #
.getcorrid_100:
        mov     g0,r14                  # r14 = copy reg. ID being checked
        call    CM$find_cor_rid         # check if this copy reg. ID in use
        cmpobe.t 0,g0,.getcorrid_1000   # Jif a matching active COR not
                                        #  identified
        addo    1,r14,g0                # g0 = next copy reg. ID to check for
        b       .getcorrid_100          # and check if the next ID is
                                        #  available
.getcorrid_1000:
        addo    1,r14,r4                # inc. to next available copy reg. ID
        st      r4,CM_cor_rid           # save next available copy reg. ID
        movl    r14,g0                  # restore g0-g1
        ret
#
#**********************************************************************
#
#  NAME: CM$find_cor_rid
#
#  PURPOSE:
#       This routine attempts to locate a copy registration ID
#       on the active COR queue.
#
#  DESCRIPTION:
#       This routine scans the active COR queue for a match to the
#       specified copy registration ID and if a match is found,
#       returns the COR address of the matching copy operation. If
#       no match is found, a zero COR address is returned indicating
#       the copy registration ID was not found.
#
#  CALLING SEQUENCE:
#       call    CM$find_cor_rid
#
#  INPUT:
#       g0 = copy registration ID to find
#       g1 = MAG serial number of copy registration ID to find
#
#  OUTPUT:
#       g0 = COR address of active copy operation matching the
#            specified copy registration ID
#       g0 = 0 if no matching COR on active queue
#
#  REGS DESTROYED:
#       Reg. g0 destroyed.
#
#**********************************************************************
#
#C access
# cor* CM_find_cor_rid(UINT32 rid, UINT32 sn);
CM_find_cor_rid:
CM$find_cor_rid:
        mov     g0,r14                  # save g0
                                        # r14 = copy registration ID to find
        ld      CM_cor_act_que,g0       # g0 = top COR on active queue
.findcorrid_100:
        cmpobe.f 0,g0,.findcorrid_1000  # Jif no more active CORs to check
        ld      cor_rid(g0),r4          # r4 = copy registration ID from
                                        #      active COR
        cmpobne.t r14,r4,.findcorrid_200 # Jif not a match
        ld      cor_rcsn(g0),r5         # r5 = copy MAG serial #
        cmpobe.f g1,r5,.findcorrid_1000 # Jif a matching COR identified
.findcorrid_200:
        ld      cor_link(g0),g0         # g0 = next active COR on queue
        b       .findcorrid_100         # and check the next active COR

.findcorrid_1000:
        ret
#
#******************************************************************************
#
#  NAME: cm$pkop_new
#
#  PURPOSE:
#       Packs a Define New Copy Operation datagram for the caller.
#
#  DESCRIPTION:
#       Allocates an ILT and request/response message buffers to pack
#       and send a Define New Copy Operation datagram to the specified
#       MAGNITUDE link.
#
#  CALLING SEQUENCE:
#       call    cm$pkop_new
#
#  INPUT:
#       g0 = MAGNITUDE serial # to send datagram request to
#       g3 = assoc. COR address
#
#  OUTPUT:
#       g1 = Define New Copy Operation datagram ILT at nest level 2
#
#  REGS DESTROYED:
#       Reg. g1 destroyed.
#
#******************************************************************************
#
cm$pkop_new:
        movq    g8,r12                  # save g8-g11
        ldconst CMsp_rq_opnew_size,g10  # g10 = request message size
        ldconst 0,g11                   # g11 = response message size
        call    DLM$get_dg              # allocate datagram resources
                                        # g1 = datagram ILT at nest level 1
        lda     dsc1_ulvl(g1),g1        # g1 = datagram ILT at nest level 2
        ld      dsc2_rqhdr_ptr(g1),r8   # r8 = local req. msg. header
        ld      dsc2_rqbuf(g1),r10      # r10 = request buffer address
        ldq     cm_opnew_hdr,g8         # g8-g11 = bytes 0-15 of req. header
        bswap   g0,g11                  # g11 = dest. serial # in big-endian
                                        #       format
        stq     g8,(r8)                 # save bytes 0-15 of req. header
        ldq     cm_opnew_hdr+16,g8      # g8-g11 = bytes 16-31 of req. header
        bswap   g9,g9                   # swap remaining length value
        stq     g8,16(r8)               # save bytes 16-31 of req. header
        lda     dgrq_size(r10),g8       # g8 = pointer to remaining req.
                                        #  message
        call    cm$pk_cor_data          # pack standard request message data
                                        #  from COR
        ld      cor_totalsegs(g3),r8    # r8 = total # segments
        ldob    cor_cstate(g3),r4       # r4 = cor_cstate value
        ldob    cor_crstate(g3),r5      # r5 = cor_crstate value
        bswap   r8,r8
        ldob    cor_gid(g3),r6          # r6 = cor_gid value
        st      r8,CMsp_rq_opnew_totalsegs(g8)
        ldq     cor_label(g3),r8        # r8-r11 = cor_label value
        ldob    cor_mstate(g3),r3       # r3 = cor_mstate value
        st      r4,CMsp_rq_opnew_cstate(g8)
        stob    r5,CMsp_rq_opnew_crstate(g8)
        stob    r6,CMsp_rq_opnew_gid(g8)
        stq     r8,CMsp_rq_opnew_label(g8)
        stob    r3,CMsp_rq_opnew_mstate(g8)
        movq    r12,g8                  # restore g8-g11
        ret
#
#******************************************************************************
#
#  NAME: cm$pkop_state
#
#  PURPOSE:
#       Packs an Establish Copy Operation State datagram for the caller.
#
#  DESCRIPTION:
#       Allocates an ILT and request/response message buffers to pack
#       and send an Establish Copy Operation State datagram to the specified
#       MAGNITUDE link.
#
#  CALLING SEQUENCE:
#       call    cm$pkop_state
#
#  INPUT:
#       g0 = MAGNITUDE serial # to send datagram request to
#       g3 = assoc. COR address
#
#  OUTPUT:
#       g1 = Establish Copy Operation State datagram ILT at nest level 2
#
#  REGS DESTROYED:
#       Reg. g1 destroyed.
#
#******************************************************************************
#
cm$pkop_state:
        st      rip,djk_save1           # save return address for debug
        movq    g8,r12                  # save g8-g11
        ldconst CMsp_rq_opst_size,g10   # g10 = request message size
        ldconst 0,g11                   # g11 = response message size
        call    DLM$get_dg              # allocate datagram resources
                                        # g1 = datagram ILT at nest level 1
        lda     dsc1_ulvl(g1),g1        # g1 = datagram ILT at nest level 2
        ld      dsc2_rqhdr_ptr(g1),r8   # r8 = local req. msg. header
        ld      dsc2_rqbuf(g1),r10      # r10 = request buffer address
        ldq     cm_opstate_hdr,g8       # g8-g11 = bytes 0-15 of req. header
        bswap   g0,g11                  # g11 = dest. serial # in big-endian
                                        #       format
        stq     g8,(r8)                 # save bytes 0-15 of req. header
        ldq     cm_opstate_hdr+16,g8    # g8-g11 = bytes 16-31 of req. header
        bswap   g9,g9                   # swap remaining length value
        stq     g8,16(r8)               # save bytes 16-31 of req. header
        lda     dgrq_size(r10),g8       # g8 = pointer to remaining req.
                                        #  message
        call    cm$pk_cor_data          # pack standard request message data
                                        #  from COR
        ldob    cor_cstate(g3),r4       # r4 = cor_cstate value
        ldob    cor_crstate(g3),r5      # r5 = cor_crstate value
        ldob    cor_gid(g3),r6          # r6 = cor_gid value
        ldq     cor_label(g3),r8        # r8-r11 = cor_label value
        ldob    cor_mstate(g3),r3       # r3 = cor_mstate value
        st      r4,CMsp_rq_opst_cstate(g8)
        stob    r5,CMsp_rq_opst_crstate(g8)
        stob    r6,CMsp_rq_opst_gid(g8)
        stq     r8,CMsp_rq_opst_label(g8)
        stob    r3,CMsp_rq_opst_mstate(g8)
        movq    r12,g8                  # restore g8-g11
        ret
#
#******************************************************************************
#
#  NAME: cm$pkop_term
#
#  PURPOSE:
#       Packs a Terminate Copy Operation datagram for the caller.
#
#  DESCRIPTION:
#       Allocates an ILT and request/response message buffers to pack
#       and send a Terminate Copy Operation datagram to the specified
#       MAGNITUDE link.
#
#  CALLING SEQUENCE:
#       call    cm$pkop_term
#
#  INPUT:
#       g0 = MAGNITUDE serial # to send datagram request to
#       g3 = assoc. COR address
#
#  OUTPUT:
#       g1 = Terminate Copy Operation datagram ILT at nest level 2
#
#  REGS DESTROYED:
#       Reg. g1 destroyed.
#
#******************************************************************************
#
cm$pkop_term:
        movq    g8,r12                  # save g8-g11
        ldconst CMsp_rq_optrm_size,g10  # g10 = request message size
        ldconst 0,g11                   # g11 = response message size
        call    DLM$get_dg              # allocate datagram resources
                                        # g1 = datagram ILT at nest level 1
        lda     dsc1_ulvl(g1),g1        # g1 = datagram ILT at nest level 2
        ld      dsc2_rqhdr_ptr(g1),r8   # r8 = local req. msg. header
        ld      dsc2_rqbuf(g1),r10      # r10 = request buffer address
        ldq     cm_opterm_hdr,g8        # g8-g11 = bytes 0-15 of req. header
        bswap   g0,g11                  # g11 = dest. serial # in big-endian
                                        #       format
        stq     g8,(r8)                 # save bytes 0-15 of req. header
        ldq     cm_opterm_hdr+16,g8     # g8-g11 = bytes 16-31 of req. header
        bswap   g9,g9                   # swap remaining length value
        stq     g8,16(r8)               # save bytes 16-31 of req. header
        lda     dgrq_size(r10),g8       # g8 = pointer to remaining req.
                                        #  message
        call    cm$pk_cor_data          # pack standard request message data
                                        #  from COR
        movq    r12,g8                  # restore g8-g11
        ret
#
#******************************************************************************
#
#  NAME: cm$pkop_check
#
#  PURPOSE:
#       Packs a Check Copy Operation State/Status datagram for the caller.
#
#  DESCRIPTION:
#       Allocates an ILT and request/response message buffers to pack
#       and send a Check Copy Operation State/Status datagram to the
#       specified MAGNITUDE link.
#
#  CALLING SEQUENCE:
#       call    cm$pkop_check
#
#  INPUT:
#       g0 = MAGNITUDE serial # to send datagram request to
#       g3 = assoc. COR address
#
#  OUTPUT:
#       g1 = Check Copy Operation State/Status datagram ILT at nest level 2
#
#  REGS DESTROYED:
#       Reg. g1 destroyed.
#
#******************************************************************************
#
cm$pkop_check:
        movq    g8,r12                  # save g8-g11
        ldconst CMsp_rq_opchk_size,g10  # g10 = request message size
        ldconst CMsp_rs_opchk_size,g11  # g11 = response message size
        call    DLM$get_dg              # allocate datagram resources
                                        # g1 = datagram ILT at nest level 1
        lda     dsc1_ulvl(g1),g1        # g1 = datagram ILT at nest level 2
        ld      dsc2_rqhdr_ptr(g1),r8   # r8 = local req. msg. header
        ld      dsc2_rqbuf(g1),r10      # r10 = request buffer address
        ldq     cm_opcheck_hdr,g8       # g8-g11 = bytes 0-15 of req. header
        bswap   g0,g11                  # g11 = dest. serial # in big-endian
                                        #       format
        stq     g8,(r8)                 # save bytes 0-15 of req. header
        ldq     cm_opcheck_hdr+16,g8    # g8-g11 = bytes 16-31 of req. header
        bswap   g9,g9                   # swap remaining length value
        stq     g8,16(r8)               # save bytes 16-31 of req. header
        lda     dgrq_size(r10),g8       # g8 = pointer to remaining req.
                                        #  message
        call    cm$pk_cor_data          # pack standard request message data
                                        #  from COR
        ldob    cor_cstate(g3),r4       # r4 = cor_cstate value
        ldob    cor_crstate(g3),r5      # r5 = cor_crstate value
        ldob    cor_gid(g3),r6          # r6 = cor_gid value
        ldq     cor_label(g3),r8        # r8-r11 = cor_label value
        ldob    cor_mstate(g3),r3       # r3 = cor_mstate value
        st      r4,CMsp_rq_opchk_cstate(g8)
        stob    r5,CMsp_rq_opchk_crstate(g8)
        stob    r6,CMsp_rq_opchk_gid(g8)
        stq     r8,CMsp_rq_opchk_label(g8)
        stob    r3,CMsp_rq_opchk_mstate(g8)
        movq    r12,g8                  # restore g8-g11
        ret
#
#******************************************************************************
#
#  NAME: cm$pkop_susp
#
#  PURPOSE:
#       Packs a Suspend Copy Operation datagram for the caller.
#
#  DESCRIPTION:
#       Allocates an ILT and request/response message buffers to pack
#       and send a Suspend Copy Operation datagram to the
#       specified MAGNITUDE link.
#
#  CALLING SEQUENCE:
#       call    cm$pkop_susp
#
#  INPUT:
#       g0 = MAGNITUDE serial # to send datagram request to
#       g3 = assoc. COR address
#
#  OUTPUT:
#       g1 = Suspend Copy Operation datagram ILT at nest level 2
#
#  REGS DESTROYED:
#       Reg. g1 destroyed.
#
#******************************************************************************
#
cm$pkop_susp:
        movq    g8,r12                  # save g8-g11
        ldconst CMsp_rq_opsusp_size,g10 # g10 = request message size
        ldconst 0,g11                   # g11 = response message size
        call    DLM$get_dg              # allocate datagram resources
                                        # g1 = datagram ILT at nest level 1
        lda     dsc1_ulvl(g1),g1        # g1 = datagram ILT at nest level 2
        ld      dsc2_rqhdr_ptr(g1),r8   # r8 = local req. msg. header
        ld      dsc2_rqbuf(g1),r10      # r10 = request buffer address
        ldq     cm_opsusp_hdr,g8        # g8-g11 = bytes 0-15 of req. header
        bswap   g0,g11                  # g11 = dest. serial # in big-endian
                                        #       format
        stq     g8,(r8)                 # save bytes 0-15 of req. header
        ldq     cm_opsusp_hdr+16,g8     # g8-g11 = bytes 16-31 of req. header
        bswap   g9,g9                   # swap remaining length value
        stq     g8,16(r8)               # save bytes 16-31 of req. header
        lda     dgrq_size(r10),g8       # g8 = pointer to remaining req.
                                        #  message
        call    cm$pk_cor_data          # pack standard request message data
                                        #  from COR
        ldob    cor_cstate(g3),r4       # r4 = cor_cstate value
        ldob    cor_crstate(g3),r5      # r5 = cor_crstate value
        ldob    cor_mstate(g3),r3       # r3 = cor_mstate value
        st      r4,CMsp_rq_opsusp_cstate(g8)
        stob    r5,CMsp_rq_opsusp_crstate(g8)
        stob    r3,CMsp_rq_opsusp_mstate(g8)
        movq    r12,g8                  # restore g8-g11
        ret
#
#******************************************************************************
#
#  NAME: cm$pkop_resume
#
#  PURPOSE:
#       Packs a Resume Copy Operation datagram for the caller.
#
#  DESCRIPTION:
#       Allocates an ILT and request/response message buffers to pack
#       and send a Resume Copy Operation datagram to the
#       specified MAGNITUDE link.
#
#  CALLING SEQUENCE:
#       call    cm$pkop_resume
#
#  INPUT:
#       g0 = MAGNITUDE serial # to send datagram request to
#       g3 = assoc. COR address
#
#  OUTPUT:
#       g1 = Resume Copy Operation datagram ILT at nest level 2
#
#  REGS DESTROYED:
#       Reg. g1 destroyed.
#
#******************************************************************************
#
cm$pkop_resume:
        movq    g8,r12                  # save g8-g11
        ldconst CMsp_rq_opresm_size,g10 # g10 = request message size
        ldconst 0,g11                   # g11 = response message size
        call    DLM$get_dg              # allocate datagram resources
                                        # g1 = datagram ILT at nest level 1
        lda     dsc1_ulvl(g1),g1        # g1 = datagram ILT at nest level 2
        ld      dsc2_rqhdr_ptr(g1),r8   # r8 = local req. msg. header
        ld      dsc2_rqbuf(g1),r10      # r10 = request buffer address
        ldq     cm_opresume_hdr,g8      # g8-g11 = bytes 0-15 of req. header
        bswap   g0,g11                  # g11 = dest. serial # in big-endian
                                        #       format
        stq     g8,(r8)                 # save bytes 0-15 of req. header
        ldq     cm_opresume_hdr+16,g8   # g8-g11 = bytes 16-31 of req. header
        bswap   g9,g9                   # swap remaining length value
        stq     g8,16(r8)               # save bytes 16-31 of req. header
        lda     dgrq_size(r10),g8       # g8 = pointer to remaining req.
                                        #  message
        call    cm$pk_cor_data          # pack standard request message data
                                        #  from COR
        ldob    cor_cstate(g3),r4       # r4 = cor_cstate value
        ldob    cor_crstate(g3),r5      # r5 = cor_crstate value
        ldob    cor_mstate(g3),r3       # r3 = cor_mstate value
        st      r4,CMsp_rq_opresm_cstate(g8)
        stob    r5,CMsp_rq_opresm_crstate(g8)
        stob    r3,CMsp_rq_opresm_mstate(g8)
        movq    r12,g8                  # restore g8-g11
        ret
#
#******************************************************************************
#
#  NAME: CM$pkop_dmove
#
#  PURPOSE:
#       Packs a Copy Device Moved datagram for the caller.
#
#  DESCRIPTION:
#       Allocates an ILT and request/response message buffers to pack
#       and send a Copy Device Moved datagram to the
#       specified MAGNITUDE link.
#
#  CALLING SEQUENCE:
#       call    CM$pkop_dmove
#
#  INPUT:
#       g0 = MAGNITUDE serial # to send datagram request to
#       g3 = assoc. COR address
#
#  OUTPUT:
#       g1 = Copy Device Moved datagram ILT at nest level 2
#
#  REGS DESTROYED:
#       Reg. g1 destroyed.
#
#******************************************************************************
#
CM$pkop_dmove:
        movq    g8,r12                  # save g8-g11
        ldconst CMsp_rq_opdmov_size,g10 # g10 = request message size
        ldconst 0,g11                   # g11 = response message size
        call    DLM$get_dg              # allocate datagram resources
                                        # g1 = datagram ILT at nest level 1
        lda     dsc1_ulvl(g1),g1        # g1 = datagram ILT at nest level 2
        ld      dsc2_rqhdr_ptr(g1),r8   # r8 = local req. msg. header
        ld      dsc2_rqbuf(g1),r10      # r10 = request buffer address
        ldq     cm_opdmove_hdr,g8       # g8-g11 = bytes 0-15 of req. header
        bswap   g0,g11                  # g11 = dest. serial # in big-endian
                                        #       format
        stq     g8,(r8)                 # save bytes 0-15 of req. header
        ldq     cm_opdmove_hdr+16,g8    # g8-g11 = bytes 16-31 of req. header
        bswap   g9,g9                   # swap remaining length value
        stq     g8,16(r8)               # save bytes 16-31 of req. header
        lda     dgrq_size(r10),g8       # g8 = pointer to remaining req.
                                        #  message
        call    cm$pk_cor_data          # pack standard request message data
                                        #  from COR
        ldob    cor_cstate(g3),r4       # r4 = cor_cstate value
        ldob    cor_crstate(g3),r5      # r5 = cor_crstate value
        ldob    cor_mstate(g3),r3       # r3 = cor_mstate value
        st      r4,CMsp_rq_opdmov_cstate(g8)
        stob    r5,CMsp_rq_opdmov_crstate(g8)
        stob    r3,CMsp_rq_opdmov_mstate(g8)
        movq    r12,g8                  # restore g8-g11
        ret
#
#******************************************************************************
#
#  NAME: CM$whack_rcor
#
#  PURPOSE:
#       Suspends remote CORs associated with a VLAR.
#
#  DESCRIPTION:
#       This routine identifies remote CORs associated with the
#       specified VLAR and remote suspends them.
#
#  CALLING SEQUENCE:
#       call    CM$whack_rcor
#
#  INPUT:
#       g6 = VDD address associated with specified VLAR
#       g7 = VLAR being whacked
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
CM$whack_rcor:
        mov     g3,r15                  # save g3
#
# --- Check if any copy operations associated with virtual device
#       and if so force them into a suspended mode of operation.
#
        ld      vlar_srcsn(g7),r10      # r10 = VLAR source MAG serial #
        ldob    vlar_srccl(g7),r11      # r11 = VLAR source MAG cluster #
        ldconst 0xff,r14                # r14 = unassigned cluster #
        ldob    vlar_srcvd(g7),r12      # r12 = VLAR source MAG VDisk #
        ldconst cormst_act,r5           # r5 = active map state code
        lda     CM$wp2_suspend,r6       # r6 = suspended write update phase
                                        #      2 handler routine
        ldconst corcrst_remsusp,r7      # r7 = user-suspended/remote reg.
                                        #  state code
        ld      vd_scdhead(g6),r8       # r8 = first SCD assoc. with VDD
        cmpobe.t 0,r8,.whackrcor_150    # Jif no SCDs assoc. with VDD
.whackrcor_100:
        ld      scd_cor(r8),r4          # r4 = assoc. COR address
        ld      cor_rcsn(r4),r13        # r13 = copy MAG serial #
        cmpobne.t r10,r13,.whackrcor_120 # Jif serial # does not match
        ldob    cor_rcscl(r4),r13       # r13 = copy MAG source cluster #
        cmpobe.f r13,r14,.whackrcor_110 # Jif unassigned cluster #
        cmpobne.t r11,r13,.whackrcor_120 # Jif cluster # does not match
        ldob    cor_rcsvd(r4),r13       # r13 = copy MAG source VDisk #
        cmpobne.t r12,r13,.whackrcor_120 # Jif VDisk # does not match
.whackrcor_110:
        stob    r5,cor_mstate(r4)       # save new map state in COR
        st      r6,scd_p2hand(r8)       # save phase 2 update handler routine
                                        #  in SCD
        stob    r7,cor_crstate(r4)      # save new copy reg. state in COR
        mov     r4,g3                   # g3 = COR address
        call    CM$mmc_sflag            # process suspended flag for MMC
.whackrcor_120:
        ld      scd_link(r8),r8         # r8 = next SCD assoc. with VDD
        cmpobne.f 0,r8,.whackrcor_100   # Jif more SCDs assoc. with VDD
.whackrcor_150:
        ld      vd_dcd(g6),r8           # r8 = DCD assoc. with VDD
        cmpobe.t 0,r8,.whackrcor_1000   # Jif no DCD assoc. with VDD
        ld      dcd_cor(r8),r4          # r4 = assoc. COR address
        ld      cor_rcsn(r4),r13        # r13 = copy MAG serial #
        cmpobne.t r10,r13,.whackrcor_1000 # Jif serial # does not match
        ldob    cor_rcdcl(r4),r13       # r13 = copy MAG dest. cluster #
        cmpobe.f r13,r14,.whackrcor_200 # Jif unassigned cluster #
        cmpobne.t r11,r13,.whackrcor_1000 # Jif cluster # does not match
        ldob    cor_rcdvd(r4),r13       # r13 = copy MAG dest. VDisk #
        cmpobne.t r12,r13,.whackrcor_1000 # Jif VDisk # does not match
.whackrcor_200:
        stob    r5,cor_mstate(r4)       # save new map state in COR
        st      r6,dcd_p2hand(r8)       # save phase 2 update handler routine
                                        #  in DCD
        stob    r7,cor_crstate(r4)      # save new copy reg. state in COR
        mov     r4,g3                   # g3 = COR address
        call    CM$mmc_sflag            # process suspended flag for MMC
.whackrcor_1000:
        mov     r15,g3                  # restore g3
        ret
#
#******************************************************************************
#
#  NAME: cm$pkrm_susp
#
#  PURPOSE:
#       Packs a Suspend Region Map Table datagram for the caller.
#
#  DESCRIPTION:
#       Allocates an ILT and request/response message buffers to pack
#       and send a Suspend Region Map Table datagram to the
#       specified MAGNITUDE link.
#
#  CALLING SEQUENCE:
#       call    cm$pkrm_susp
#
#  INPUT:
#       g0 = MAGNITUDE serial # to send datagram request to
#       g3 = assoc. COR address
#
#  OUTPUT:
#       g1 = Suspend Region Map Table datagram ILT at nest level 2
#
#  REGS DESTROYED:
#       Reg. g1 destroyed.
#
#******************************************************************************
#
cm$pkrm_susp:
        movq    g8,r12                  # save g8-g11
        ldconst CMsp_rq_rmsusp_size,g10 # g10 = request message size
        ldconst 0,g11                   # g11 = response message size
        call    DLM$get_dg              # allocate datagram resources
                                        # g1 = datagram ILT at nest level 1
        lda     dsc1_ulvl(g1),g1        # g1 = datagram ILT at nest level 2
        ld      dsc2_rqhdr_ptr(g1),r8   # r8 = local req. msg. header
        ld      dsc2_rqbuf(g1),r10      # r10 = request buffer address
        ldq     cm_rmsusp_hdr,g8        # g8-g11 = bytes 0-15 of req. header
        bswap   g0,g11                  # g11 = dest. serial # in big-endian
                                        #       format
        stq     g8,(r8)                 # save bytes 0-15 of req. header
        ldq     cm_rmsusp_hdr+16,g8     # g8-g11 = bytes 16-31 of req. header
        bswap   g9,g9                   # swap remaining length value
        stq     g8,16(r8)               # save bytes 16-31 of req. header
        lda     dgrq_size(r10),g8       # g8 = pointer to remaining req.
                                        #  message
        call    cm$pk_cor_data          # pack standard request message data
                                        #  from COR
        ldob    cor_cstate(g3),r4       # r4 = cor_cstate value
        ldob    cor_crstate(g3),r5      # r5 = cor_crstate value
        ldob    cor_mstate(g3),r3       # r3 = cor_mstate value
        st      r4,CMsp_rq_rmsusp_cstate(g8)
        stob    r5,CMsp_rq_rmsusp_crstate(g8)
        stob    r3,CMsp_rq_rmsusp_mstate(g8)
        movq    r12,g8                  # restore g8-g11
        ret
#
#******************************************************************************
#
#  NAME: cm$pkrm_term
#
#  PURPOSE:
#       Packs a Terminate Region Map Table datagram for the caller.
#
#  DESCRIPTION:
#       Allocates an ILT and request/response message buffers to pack
#       and send a Terminate Region Map Table datagram to the
#       specified MAGNITUDE link.
#
#  CALLING SEQUENCE:
#       call    cm$pkrm_term
#
#  INPUT:
#       g0 = MAGNITUDE serial # to send datagram request to
#       g3 = assoc. COR address
#
#  OUTPUT:
#       g1 = Terminate Region Map Table datagram ILT at nest level 2
#
#  REGS DESTROYED:
#       Reg. g1 destroyed.
#
#******************************************************************************
#
cm$pkrm_term:
        movq    g8,r12                  # save g8-g11
        ldconst CMsp_rq_rmterm_size,g10 # g10 = request message size
        ldconst 0,g11                   # g11 = response message size
        call    DLM$get_dg              # allocate datagram resources
                                        # g1 = datagram ILT at nest level 1
        lda     dsc1_ulvl(g1),g1        # g1 = datagram ILT at nest level 2
        ld      dsc2_rqhdr_ptr(g1),r8   # r8 = local req. msg. header
        ld      dsc2_rqbuf(g1),r10      # r10 = request buffer address
        ldq     cm_rmterm_hdr,g8        # g8-g11 = bytes 0-15 of req. header
        bswap   g0,g11                  # g11 = dest. serial # in big-endian
                                        #       format
        stq     g8,(r8)                 # save bytes 0-15 of req. header
        ldq     cm_rmterm_hdr+16,g8     # g8-g11 = bytes 16-31 of req. header
        bswap   g9,g9                   # swap remaining length value
        stq     g8,16(r8)               # save bytes 16-31 of req. header
        lda     dgrq_size(r10),g8       # g8 = pointer to remaining req.
                                        #  message
        call    cm$pk_cor_data          # pack standard request message data
                                        #  from COR
        ldob    cor_cstate(g3),r4       # r4 = cor_cstate value
        ldob    cor_crstate(g3),r5      # r5 = cor_crstate value
        st      r4,CMsp_rq_rmterm_cstate(g8)
        stob    r5,CMsp_rq_rmterm_crstate(g8)
        movq    r12,g8                  # restore g8-g11
        ret
#
#******************************************************************************
#
#  NAME: cm$pkrm_read
#
#  PURPOSE:
#       Packs a Read Region/Segment Map Table datagram for the caller.
#
#  DESCRIPTION:
#       Allocates an ILT and request/response message buffers to pack
#       and send a Read Region/Segment Map Table datagram to the
#       specified MAGNITUDE link.
#
#  CALLING SEQUENCE:
#       call    cm$pkrm_read
#
#  INPUT:
#       g0 = MAGNITUDE serial # to send datagram request to
#       g2 = region map number to read
#       g3 = assoc. COR address
#
#  OUTPUT:
#       g1 = Read Region/Segment Map Table datagram ILT at nest level 2
#
#  REGS DESTROYED:
#       Reg. g1 destroyed.
#
#******************************************************************************
#
cm$pkrm_read:
        movq    g8,r12                  # save g8-g11
        ldconst CMsp_rq_rmrd_size,g10   # g10 = request message size
        ldconst CMsp_rs_rmrd_size,g11   # g11 = response message size
        call    DLM$get_dg              # allocate datagram resources
                                        # g1 = datagram ILT at nest level 1
        lda     dsc1_ulvl(g1),g1        # g1 = datagram ILT at nest level 2
        ld      dsc2_rqhdr_ptr(g1),r8   # r8 = local req. msg. header
        ld      dsc2_rqbuf(g1),r10      # r10 = request buffer address
        ldq     cm_rmread_hdr,g8        # g8-g11 = bytes 0-15 of req. header
        bswap   g0,g11                  # g11 = dest. serial # in big-endian
                                        #       format
        stq     g8,(r8)                 # save bytes 0-15 of req. header
        ldq     cm_rmread_hdr+16,g8     # g8-g11 = bytes 16-31 of req. header
        bswap   g9,g9                   # swap remaining length value
        stq     g8,16(r8)               # save bytes 16-31 of req. header
        lda     dgrq_size(r10),g8       # g8 = pointer to remaining req.
                                        #  message
        call    cm$pk_cor_data          # pack standard request message data
                                        #  from COR
        ldob    cor_cstate(g3),r4       # r4 = cor_cstate value
        ldob    cor_crstate(g3),r5      # r5 = cor_crstate value
        ldob    cor_mstate(g3),r3       # r3 = cor_mstate value
        bswap   g2,r8                   # r8 = region map # in little-endian
                                        #      format
        st      r4,CMsp_rq_rmrd_cstate(g8)
        stob    r5,CMsp_rq_rmrd_crstate(g8)
        stob    r3,CMsp_rq_rmrd_mstate(g8)
        st      r8,CMsp_rq_rmrd_reg(g8)
        movq    r12,g8                  # restore g8-g11
        ret
#
#******************************************************************************
#
#  NAME: cm$pkrm_clear
#
#  PURPOSE:
#       Packs a Clear Region/Segment Map Table datagram for the caller.
#
#  DESCRIPTION:
#       Allocates an ILT and request/response message buffers to pack
#       and send a Clear Region/Segment Map Table datagram to the
#       specified MAGNITUDE link.
#
#  CALLING SEQUENCE:
#       call    cm$pkrm_clear
#
#  INPUT:
#       g0 = MAGNITUDE serial # to send datagram request to
#       g2 = region map number to clear
#       g3 = assoc. COR address
#
#  OUTPUT:
#       g1 = Clear Region/Segment Map Table datagram ILT at nest level 2
#
#  REGS DESTROYED:
#       Reg. g1 destroyed.
#
#******************************************************************************
#
cm$pkrm_clear:
        movq    g8,r12                  # save g8-g11
        ldconst CMsp_rq_rmclr_size,g10  # g10 = request message size
        ldconst 0,g11                   # g11 = response message size
        call    DLM$get_dg              # allocate datagram resources
                                        # g1 = datagram ILT at nest level 1
        lda     dsc1_ulvl(g1),g1        # g1 = datagram ILT at nest level 2
        ld      dsc2_rqhdr_ptr(g1),r8   # r8 = local req. msg. header
        ld      dsc2_rqbuf(g1),r10      # r10 = request buffer address
        ldq     cm_rmclear_hdr,g8       # g8-g11 = bytes 0-15 of req. header
        bswap   g0,g11                  # g11 = dest. serial # in big-endian
                                        #       format
        stq     g8,(r8)                 # save bytes 0-15 of req. header
        ldq     cm_rmclear_hdr+16,g8    # g8-g11 = bytes 16-31 of req. header
        bswap   g9,g9                   # swap remaining length value
        stq     g8,16(r8)               # save bytes 16-31 of req. header
        lda     dgrq_size(r10),g8       # g8 = pointer to remaining req.
                                        #  message
        call    cm$pk_cor_data          # pack standard request message data
                                        #  from COR
        ldob    cor_cstate(g3),r4       # r4 = cor_cstate value
        ldob    cor_crstate(g3),r5      # r5 = cor_crstate value
        bswap   g2,r8                   # r8 = region map # in little-endian
                                        #      format
        st      r4,CMsp_rq_rmclr_cstate(g8)
        stob    r5,CMsp_rq_rmclr_crstate(g8)
        st      r8,CMsp_rq_rmclr_reg(g8)
        movq    r12,g8                  # restore g8-g11
        ret
#
#******************************************************************************
#
#  NAME: cm$pkrm_check
#
#  PURPOSE:
#       Packs a Check Region Map Table State/Status datagram for the caller.
#
#  DESCRIPTION:
#       Allocates an ILT and request/response message buffers to pack
#       and send a Check Region Map Table State/Status datagram to the
#       specified MAGNITUDE link.
#
#  CALLING SEQUENCE:
#       call    cm$pkrm_check
#
#  INPUT:
#       g0 = MAGNITUDE serial # to send datagram request to
#       g3 = assoc. COR address
#
#  OUTPUT:
#       g1 = Check Region Map Table State/Status datagram ILT at nest level 2
#
#  REGS DESTROYED:
#       Reg. g1 destroyed.
#
#******************************************************************************
#
cm$pkrm_check:
        movq    g8,r12                  # save g8-g11
        ldconst CMsp_rq_rmchk_size,g10  # g10 = request message size
        ldconst CMsp_rs_rmchk_size,g11  # g11 = response message size
        call    DLM$get_dg              # allocate datagram resources
                                        # g1 = datagram ILT at nest level 1
        lda     dsc1_ulvl(g1),g1        # g1 = datagram ILT at nest level 2
        ld      dsc2_rqhdr_ptr(g1),r8   # r8 = local req. msg. header
        ld      dsc2_rqbuf(g1),r10      # r10 = request buffer address
        ldq     cm_rmcheck_hdr,g8       # g8-g11 = bytes 0-15 of req. header
        bswap   g0,g11                  # g11 = dest. serial # in big-endian
                                        #       format
        stq     g8,(r8)                 # save bytes 0-15 of req. header
        ldq     cm_rmcheck_hdr+16,g8    # g8-g11 = bytes 16-31 of req. header
        bswap   g9,g9                   # swap remaining length value
        stq     g8,16(r8)               # save bytes 16-31 of req. header
        lda     dgrq_size(r10),g8       # g8 = pointer to remaining req.
                                        #  message
        call    cm$pk_cor_data          # pack standard request message data
                                        #  from COR
        ldob    cor_cstate(g3),r4       # r4 = cor_cstate value
        ldob    cor_crstate(g3),r5      # r5 = cor_crstate value
        ldob    cor_mstate(g3),r3       # r3 = cor_mstate value
        st      r4,CMsp_rq_rmchk_cstate(g8)
        stob    r5,CMsp_rq_rmchk_crstate(g8)
        stob    r3,CMsp_rq_rmchk_mstate(g8)
        movq    r12,g8                  # restore g8-g11
        ret
#
#******************************************************************************
#
#  NAME: cm$pk_cor_data
#
#  PURPOSE:
#       Packs standard COR related data into a CMsp type datagram
#       request message.
#
#  DESCRIPTION:
#       This routine packs the following data values from the COR
#       into a CMsp datagram request message buffer:
#
#       - cor_rid
#       - cor_rcsn
#       - cor_rcscl
#       - cor_rcsvd
#       - cor_rcdcl
#       - cor_rcdvd
#       - cor_rssn
#       - cor_rdsn
#       - cor_rscl
#       - cor_rsvd
#       - cor_rdcl
#       - cor_rdvd
#
#       This routine changes the endianess of the appropriate COR
#       data values.
#
#  CALLING SEQUENCE:
#       call    cm$pk_cor_data
#
#  INPUT:
#       g3 = assoc. COR address
#       g8 = CMsp datagram request message buffer address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
cm$pk_cor_data:
        ldq     cor_rid(g3),r4          # r4 = cor_rid
                                        # r5 = cor_rcsn
                                        # r6 = cor_rcscl,cor_rcsvd,
                                        #      cor_rcdcl,cor_rcdvd
                                        # r7 = cor_rssn
        ldl     cor_rdsn(g3),r8         # r8 = cor_rdsn
                                        # r9 = cor_rscl,cor_rsvd,
                                        #      cor_rdcl,cor_rdvd
        bswap   r4,r4
        bswap   r5,r5
        bswap   r7,r7
        bswap   r8,r8
        stq     r4,CMsp_rq_opnew_rid(g8)
        stl     r8,CMsp_rq_opnew_rdsn(g8)
        ret
#
#******************************************************************************
#
#  NAME: cm$poll_local_cor
#
#  PURPOSE:
#       Poll routine for local CORs.
#
#  DESCRIPTION:
#       This routine determines if any remote MAGNITUDES associated
#       with the specified COR need to be polled and if so will
#       poll them and perform any appropriate logic to process
#       errors returned in response to the poll requests. It updates
#       and copy operation information as appropriately and informs
#       any remote MAGNITUDES of the current copy operation state/status
#       as needed.
#
#       The polling logic falls into one of five possible cases as
#       defined below:
#
#       Case 1 - Source copy device = VDisk
#                Destination copy device = VDisk
#                 - No polling is performed.
#       Case 2 - Source copy device = VDisk
#                Destination copy device = VLink to MAG "A"
#                 - Poll MAG "A". Source/destination copy device
#                   as defined locally, remote cluster/VDisk # as
#                   returned by MAG "A".
#       Case 3 - Source copy device = VLink to MAG "A"
#                Destination copy device = VDisk
#                 - Same as Case 2.
#       Case 4 - Source copy device = VLink to MAG "A"
#                Destination copy device = VLink to MAG "B"
#                 - Poll MAG "A" and MAG "B". Source/destination
#                   copy devices as defined locally, remote
#                   cluster/VDisk #'s as returned by MAG "A"
#                   and MAG "B" respectively.
#       Case 5 - Source copy device = VLink to MAG "A"
#                Destination copy device = VLink to MAG "A"
#                 - Poll MAG "A". Source/destination copy devices
#                   as defined remotely.
#
#  CALLING SEQUENCE:
#       call    cm$poll_local_cor
#
#  INPUT:
#       g3 = COR address of local copy operation to process
#
#       Note: This routine stalls the calling routine's process waiting
#               for responses to datagram messages sent to peer nodes.
#
#  OUTPUT:
#       g0 = completion status code
#            00 = successful/no operational changes made
#            Bit 7 = copy operation needs to be terminated and restarted
#                6 =
#                5 =
#                4 = cor_crstate changed to user-suspend
#                3 = segment map accumulation needed
#                2 =
#                1 =
#                0 = a remote MAGNITUDE was not polled
#
#  REGS DESTROYED:
#       Reg. g0 destroyed.
#
#******************************************************************************
#
cm$poll_local_cor:
        movl    g0,r14                  # save g0-g1
        ldconst 0,r14                   # r14 = successful completion status
                                        #       code
        ldob    cor_crstate(g3),r4      # r4 = copy reg. state
        cmpobne.t corcrst_undef,r4,.plcor_40 # Jif reg. state <> undefined
.plcor_20:
        setbit  0,r14,r14               # set flag indicating remote MAG was
                                        #  not polled
        b       .plcor_1000

.plcor_40:
        cmpobe.f corcrst_init,r4,.plcor_20 # Jif reg. state = initializing
        ldq     cor_rid(g3),r8          # r8 = cor_rid
                                        # r9 = cor_rcsn
                                        # r10 = cor_rcscl, cor_rcsvd
                                        #       cor_rcdcl, cor_rcdvd
                                        # r11 = cor_rssn
        ld      cor_rdsn(g3),r12        # r12 = cor_rdsn
#
# --- Determine which case we are processing.
#
        cmpobne.t r11,r12,.plcor_100    # Jif source/dest. copy devices on
                                        #  different MAGNITUDES
#
# --- Source copy device and Destination copy device are on the same
#       MAGNITUDE.
#
        cmpobne.t r9,r11,.plcor_600     # Jif source/dest. copy devices are
                                        #  not on this MAGNITUDE (Case 5)
#
# ********************** Case 1  processing **********************************
#
# --- Source copy device and Destination copy device are on this
#       MAGNITUDE.
#
#     temporarily disable the FE Write Cache of the destination device. This
#     causes the cache of the destination device to be flushed.
#
        ld      cor_destvdd(g3),r6      # r6 = destination vdd address
        cmpobe  0,r6,.plcor_1000        # Jif NULL

        PushRegs(r3)                    # Save all G regs for "C" call
        ldos    vd_vid(r6),g0           # g0 = load VID from vdd
        ldconst WC_SET_T_DISABLE,g1     # g1 = Function to Temp Disable WC
        call    WC_VDiskDisable         # Go Flush Write Cache and wait
        cmpobe  0,g0,.plcor_80          # jif cache was flushed
#
# --- there seems to be some issue flushing the cache. At the moment, just issue a
#     message indicating so....
#
c fprintf(stderr,"%s%s:%u ERROR - cm$poll_local_cor returned from disabling VDisk Cache - VDD = %x RC = %x\n", FEBEMESSAGE, __FILE__, __LINE__, (UINT32)r6, (UINT32)g0);
#
# --- Clear the temporarily disable of the FE Write Cache
#
.plcor_80:
        ldos    vd_vid(r6),g0           # g0 = load VID from vdd
        ldconst WC_CLEAR_T_DISABLE,g1   # g1 = Function to Temp Disable WC
        call    WC_VDiskDisable         # Go Clear the T Disable flag
        PopRegsVoid(r3)                 # Restore all G regs

        b       .plcor_1000             #  exit
#
# --- Source copy device and Destination copy device are on different
#       MAGNITUDES.
#
.plcor_100:
        mov     r12,r13                 # r13 = dest. MAG serial #
        cmpobe.f r9,r11,.plcor_200      # Jif source MAG is me (Case 2)
        mov     r11,r13                 # r13 = source MAG serial #
        cmpobe.f r9,r12,.plcor_200      # Jif dest. MAG is me (Case 3)
        b       .plcor_400              # go to (Case 4) logic
#
# ********************** Case 2 and Case 3 processing ************************
#
#  INPUT:
#       r8 = cor_rid
#       r9 = cor_rcsn
#       r10 = cor_rcscl, cor_rcsvd
#             cor_rcdcl, cor_rcdvd
#       r11 = cor_rssn
#       r12 = cor_rdsn
#       r13 = remote MAG serial # to process
#       r14 = completion status code (0)
#       g3 = COR address being processed
#
.plcor_200:
        ldconst 4,r3                    # r3 = error retry count
.plcor_205:
        mov     r13,g0                  # g0 = MAGNITUDE serial # to poll
        call    cm$pkop_check           # pack a Check Copy Operation State/
                                        # Status datagram
                                        # g1 = datagram ILT at nest level #2
        lda     DLM$send_dg,g0          # g0 = datagram service provider
                                        #  routine
        call    K$qw                    # Queue request w/wait
        ld      dsc2_rshdr_ptr(g1),r7   # r7 = local response header address
        ldob    dgrs_status(r7),r4      # r4 = request completion status
        ldob    dgrs_ec1(r7),r5         # r5 = error code byte #1
        cmpobe.t dg_st_ok,r4,.plcor_250 # Jif no error reported on request
        lda     -dsc1_ulvl(g1),g1       # g1 = datagram ILT at nest level 1
        call    DLM$put_dg              # deallocate datagram ILT
        cmpobne.f dg_st_srvr,r4,.plcor_240 # Jif not dest. server level
                                        #  error
#
# --- Destination server level error returned
#
        cmpobne.f dgec1_srvr_nocopy,r5,.plcor_220 # Jif not specified copy
                                        #  operation not defined
#
# --- Specified copy operation is no longer defined.
#
#.plcor_210:
        setbit  7,r14,r14               # set terminate and restart copy
                                        #  bit in completion status code
        b       .plcor_1000             # and we're out of here!

.plcor_220:
        cmpobne.f 0,r3,.plcor_230       # Jif retry count has not expired
        b       .plcor_990              # and we're out of here!
#
.plcor_230:
        subo    1,r3,r3                 # dec. error retry count
        b       .plcor_205              # and check it again

.plcor_240:
#       cmpobe.f dg_st_ddsp,r4,.plcor_220 # Jif dest. datagram service
                                        #  provider level error
#
# --- Datagram never made it to the destination server. Do not terminate
#       the copy operation for these types of errors. Can't communicate
#       with the remote MAGNITUDE so just ignore for now and move on with
#       other copy operation processing.
#
        cmpobne.t 0,r3,.plcor_230       # Jif error retry count has not
                                        #  expired
        setbit  0,r14,r14               # set flag indicating remote MAG was
                                        #  not polled
        b       .plcor_1000

.plcor_250:
#
# --- Datagram response was successful. Check if response data matches
#       what we expect to see.
#       When the VL check flag is not zero, ignore the response data because the
#       Copy-swap process is progress which can result in inconsistent data.
#
        ldob    DLM_vlchk_flag,r7       # get vlink check flag
        cmpobne 0,r7,.plcor_340         # Jif there is count, ignore the response

        call    cm$tr_chk_resp          # transpose response data fields
        ld      dsc2_rsbuf(g1),r7       # r7 = response message buffer
        lda     dgrs_size(r7),r7        # r7 = response data address
        ldl     CMsp_rs_opchk_rcscl(r7),r4 # r4 = cor_rcscl, cor_rcsvd
                                        #  cor_rcdcl, cor_rcdvd from remote
                                        # r5 = cor_rssn from remote
        cmpobe.t r4,r10,.plcor_270      # Jif copy MAG data matches
        setbit  31,r14,r14              # set send Establish Copy Operation
                                        #  State datagram to remote MAG flag
        mov     r13,g0                  # g0 = remote MAG serial # being
                                        #  processed
        ldob    cor_crstate(g3),r4      # r4 = current cor_crstate value
        call    cm$cmag_delta           # process copy MAG device differences
                                        #  between local COR values and check
                                        #  copy operation response values
        ldob    cor_crstate(g3),r6      # r6 = new cor_crstate value
        cmpobe.t r4,r6,.plcor_265       # Jif cor_crstate did not change
        cmpobne.f corcrst_usersusp,r6,.plcor_265 # Jif not user suspended state
        setbit  4,r14,r14               # set cor_crstate set to user suspend
                                        #  flag
.plcor_265:
        mov     r10,r4                  # r4 = old COR values
        ld      cor_rcscl(g3),r10       # reload values to reflect any changes
                                        #  that may have been made
        cmpobe.f r4,r10,.plcor_270      # Jif no COR values changed
        setbit  30,r14,r14              # set flag indicating need to save
                                        #  configuration
.plcor_270:
        cmpobne.f r5,r11,.plcor_275     # Jif source MAG serial # does not
                                        #  match
        ldl     CMsp_rs_opchk_rdsn(r7),r4 # r4 = cor_rdsn from remote
                                        # r5 = cor_rscl, cor_rsvd
                                        #      cor_rdcl, cor_rdvd from remote
        ld      cor_rscl(g3),r6         # r6 = cor_rscl, cor_rsvd
                                        #      cor_rdcl, cor_rdvd
        cmpobne.f r4,r12,.plcor_275     # Jif dest. MAG serial # does not
                                        #  match
        cmpobe.t r5,r6,.plcor_335       # Jif all values match
.plcor_275:
        setbit  31,r14,r14              # set send Establish Copy Operation
                                        #  State datagram to remote MAG flag
#
# --- Some copy operation component values do not match. Determine what
#       changes need to be made between the two MAGNITUDES, make them
#       and then communicate them to the remote MAGNITUDE.
#
        cmpobne.f r9,r11,.plcor_300     # Jif source copy device on remote MAG
#
# --- Destination copy device is on remote MAG.
#
        ldl     CMsp_rs_opchk_rssn(r7),r4 # r4 = cor_rssn from remote
                                        # r5 = cor_rdsn from remote
        cmpobne.f r5,r12,.plcor_285     # Jif dest. MAG serial # different
#
# --- Destination copy device is on remote MAG.
# --- Destination MAG serial # the same.
#
        ldos    CMsp_rs_opchk_rdcl(r7),r6 # r6 = cor_rdcl, cor_rdvd from
                                        #        remote
        ldos    cor_rdcl(g3),r5         # r5 = cor_rdcl, cor_rdvd
        cmpobe.f r6,r5,.plcor_335       # Jif values the same
#
# --- Destination copy device is on remote MAG.
# --- Destination MAG serial # the same.
# --- Destination MAG cluster/VDisk # different.
#
        stos    r6,cor_rdcl(g3)         # save remote values in COR
        setbit  30,r14,r14              # set flag indicating need to save
                                        #  configuration
        b       .plcor_335              # and we're done here

.plcor_285:
#
# --- Destination copy device is on remote MAG.
# --- Destination MAG serial # different.
#
        cmpobe.t r4,r12,.plcor_287      # Jif remote source MAG serial #
                                        #  matches my dest. MAG serial #
                                        #  indicating a swap occurred.
#
# --- Destination copy device is on remote MAG.
# --- Destination MAG serial # different.
# --- The remote source MAG serial # and the remote destination
#       MAG serial # do not match my destination MAG serial #.
#       This indicates that this remote MAGNITUDE is not associated
#       with this copy operation. This should never occur!!! Just
#       try to establish the copy operation as defined locally by
#       sending an Establish Copy Operation State datagram to the
#       remote MAGNITUDE and try to get it back into sync.
#
        b       .plcor_340              # and we're done here

.plcor_287:
#
# --- Destination copy device is on remote MAG.
# --- Destination MAG serial # different.
# --- Remote source MAG serial # matches the local destination MAG
#       serial #. A copy device swap needs to occur.
#
        ldos    CMsp_rs_opchk_rscl(r7),r4 # r4 = cor_rscl, cor_rsvd from
                                        #  remote
        ldos    cor_rdcl(g3),r5         # r5 = local cor_rdcl, cor_rdvd
        cmpobe.t r4,r5,.plcor_335       # Jif values the same
        stos    r4,cor_rdcl(g3)         # save remote values in COR
        setbit  30,r14,r14              # set flag indicating need to save
                                        #  configuration
        b       .plcor_335

.plcor_300:
#
# --- Source copy device is on remote MAG.
#
        ldl     CMsp_rs_opchk_rssn(r7),r4 # r4 = cor_rssn from remote
                                        # r5 = cor_rdsn from remote
        cmpobne.f r4,r11,.plcor_315     # Jif source MAG serial # different
#
# --- Source copy device is on remote MAG.
# --- Source MAG serial # the same.
#
        ldos    CMsp_rs_opchk_rscl(r7),r6 # r6 = cor_rscl, cor_rsvd from
                                        #        remote
        ldos    cor_rscl(g3),r4         # r4 = cor_rscl, cor_rsvd
        cmpobe.f r6,r4,.plcor_335       # Jif values the same
#
# --- Source copy device is on remote MAG.
# --- Source MAG serial # the same.
# --- Source MAG cluster/VDisk # different.
#
        stos    r6,cor_rscl(g3)         # save remote values in COR
        setbit  30,r14,r14              # set flag indicating need to save
                                        #  configuration
        b       .plcor_335

.plcor_315:
#
# --- Source copy device is on remote MAG.
# --- Source MAG serial # different.
#
        cmpobe.t r5,r11,.plcor_317      # Jif remote dest. MAG serial #
                                        #  matches my source MAG serial #
                                        #  indicating a swap occurred.
#
# --- Source copy device is on remote MAG.
# --- Source MAG serial # different.
# --- The remote source MAG serial # and the remote destination
#       MAG serial # do not match my source MAG serial #.
#       This indicates that this remote MAGNITUDE is not associated
#       with this copy operation. This should never occur!!! Just
#       try to establish the copy operation as defined locally by
#       sending an Establish Copy Operation State datagram to the
#       remote MAGNITUDE and try to get it back into sync.
#
        b       .plcor_340              # and we're done here

.plcor_317:
#
# --- Source copy device is on remote MAG.
# --- Source MAG serial # different.
# --- Remote destination MAG serial # matches the local source MAG
#       serial #. A copy device swap needs to occur.
#
        ldos    CMsp_rs_opchk_rdcl(r7),r4 # r4 = cor_rdcl, cor_rdvd from
                                        #  remote
        ldos    cor_rscl(g3),r5         # r5 = local cor_rscl, cor_rsvd
        cmpobe.t r4,r5,.plcor_335       # Jif values the same
        stos    r4,cor_rscl(g3)         # save remote values in COR
        setbit  30,r14,r14              # set flag indicating need to save
                                        #  configuration
.plcor_335:
        ldob    CMsp_rs_opchk_crstate(r7),r4 # r7 = copy reg. state of remote
                                        #  MAGNITUDE
        ldob    cor_crstate(g3),r5      # r5 = my copy reg. state
        cmpobe.t r4,r5,.plcor_338       # Jif copy reg. states the same
#
# --- The copy registration states do not match. Check which state is the
#       dominate one and establish this one as the current registration
#       state of the copy operation.
#
# ---------------------------------------------------------------
#  Remote Copy  |          Local Copy Registration State        |
#  Registration |-----------------------------------------------|
#     State     |    active     |  auto-suspend |  user-suspend |
# ==============|===============|===============|===============|=============
#     active    |    active     |  auto-suspend |  user-suspend |    New     |
# --------------|---------------|---------------|---------------|            |
#  auto-suspend |    active     |  auto-suspend |  user-suspend |Registration|
# --------------|---------------|---------------|---------------|            |
#  user-suspend |    active     |  auto-suspend |  user-suspend |    State   |
# --------------|---------------|---------------|---------------|            |
#  rem-suspend  |  user-suspend |  user-suspend |  user-suspend |            |
# ----------------------------------------------------------------------------
#
        cmpobne.t corcrst_remsusp,r4,.plcor_337 # Jif remote not rem-
                                        #  suspend registration state
        ldob    cor_crstate(g3),r4      # r4 = current cor_crstate value
        cmpobe  corcrst_usersusp,r4,.plcor_338 # Jif already in user suspended
                                        #        state
        ldconst corcrst_usersusp,r4     # r4 = user-suspend/local state code
        stob    r4,cor_crstate(g3)      # save new copy registration state
        setbit  4,r14,r14               # set cor_crstate changed to user-
                                        #  suspend state flag in return status
        call    CM$mmc_sflag            # process suspended flag for MMC
        b       .plcor_338

.plcor_337:
        setbit  31,r14,r14              # set send Establish Copy Operation
                                        #  State datagram to remote MAG flag
.plcor_338:
        ldob    CMsp_rs_opchk_mstate(r7),r4 # r4 = remote map table state
        cmpobe.t cormst_term,r4,.plcor_340 # Jif map table is terminated
        ldob    cor_crstate(g3),r5      # r5 = COR registration state
        cmpobe.t corcrst_active,r5,.plcor_339 # Jif in active state
        cmpobne.t corcrst_autosusp,r5,.plcor_340 # Jif not in auto-suspend
                                        #  state
.plcor_339:
        setbit  3,r14,r14               # set segment map accumulation flag
.plcor_340:
        lda     -dsc1_ulvl(g1),g1       # g1 = datagram ILT at nest level 1
        call    DLM$put_dg              # deallocate datagram ILT
        b       .plcor_800              # check if need to establish copy
                                        #  operation state to remote MAG
#
# ******************** End Case 2 and Case 3 processing **********************
#
# ***************************** Case 4 processing ****************************
#
#  INPUT:
#
#       r8 = cor_rid
#       r9 = cor_rcsn
#       r10 = cor_rcscl, cor_rcsvd
#             cor_rcdcl, cor_rcdvd
#       r11 = cor_rssn
#       r12 = cor_rdsn
#       r14 = completion status code (0)
#       g3 = COR address being processed
#
.plcor_400:
        mov     r11,r13                 # r13 = source MAG serial #
.plcor_403:
        ldconst 4,r3                    # r3 = error retry count
.plcor_405:
        mov     r13,g0                  # g0 = MAGNITUDE serial # to poll
        call    cm$pkop_check           # pack a Check Copy Operation State/
                                        # Status datagram
                                        # g1 = datagram ILT at nest level #2
        lda     DLM$send_dg,g0          # g0 = datagram service provider
                                        #  routine
        call    K$qw                    # Queue request w/wait
        ld      dsc2_rshdr_ptr(g1),r7   # r7 = local response header address
        ldob    dgrs_status(r7),r4      # r4 = request completion status
        ldob    dgrs_ec1(r7),r5         # r5 = error code byte #1
        cmpobe.t dg_st_ok,r4,.plcor_450 # Jif no error reported on request
        lda     -dsc1_ulvl(g1),g1       # g1 = datagram ILT at nest level 1
        call    DLM$put_dg              # deallocate datagram ILT
        cmpobne.f dg_st_srvr,r4,.plcor_440 # Jif not dest. server level
                                        #  error
#
# --- Destination server level error returned
#
        cmpobne.f dgec1_srvr_nocopy,r5,.plcor_420 # Jif not specified copy
                                        #  operation not defined
#
# --- Specified copy operation is no longer defined.
#
#.plcor_410:
        setbit  7,r14,r14               # set terminate and restart copy
                                        #  bit in completion status code
        b       .plcor_1000             # and we're out of here!

.plcor_420:
        cmpobne.f 0,r3,.plcor_430        # Jif retry count has not expired
        b       .plcor_990              # and we're out of here!

.plcor_430:
        subo    1,r3,r3                 # dec. error retry count
        b       .plcor_405              # and check it again

.plcor_440:
#       cmpobe.f dg_st_ddsp,r4,.plcor_420 # Jif dest. datagram service
                                        #  provider level error
#
# --- Datagram never made it to the destination server. Do not terminate
#       the copy operation for these types of errors. Can't communicate
#       with the remote MAGNITUDE so just ignore for now and move on with
#       other copy operation processing.
#
        cmpobne.t 0,r3,.plcor_430       # Jif error retry count has not
                                        #  expired
        setbit  0,r14,r14               # set flag indicating remote MAG was
                                        #  not polled
        b       .plcor_550

.plcor_450:
#
# --- Datagram response was successful. Check if response data matches
#       what we expect to see.
#
        call    cm$tr_chk_resp          # transpose response data fields
        ld      dsc2_rsbuf(g1),r7       # r7 = response message buffer
        lda     dgrs_size(r7),r7        # r7 = response data address
        ldl     CMsp_rs_opchk_rcscl(r7),r4 # r4 = cor_rcscl, cor_rcsvd
                                        #  cor_rcdcl, cor_rcdvd from remote
                                        # r5 = cor_rssn from remote
        cmpobe.t r4,r10,.plcor_470      # Jif copy MAG data matches
        setbit  31,r14,r14              # set send Establish Copy Operation
                                        #  State datagram to remote MAG flag
        mov     r13,g0                  # g0 = remote MAG serial # being
                                        #  processed
        ldob    cor_crstate(g3),r4      # r4 = current cor_crstate value
        call    cm$cmag_delta           # process copy MAG device differences
                                        #  between local COR values and check
                                        #  copy operation response values
        ldob    cor_crstate(g3),r6      # r6 = new cor_crstate value
        cmpobe.t r4,r6,.plcor_465       # Jif cor_crstate did not change
        cmpobne.f corcrst_usersusp,r6,.plcor_465 # Jif not user suspended state
        setbit  4,r14,r14               # set cor_crstate set to user suspend
                                        #  flag
.plcor_465:
        mov     r10,r4                  # r4 = old COR values
        ld      cor_rcscl(g3),r10       # reload values to reflect any changes
                                        #  that may have been made
        cmpobe.f r4,r10,.plcor_470      # Jif no COR values changed
        setbit  30,r14,r14              # set flag indicating need to save
                                        #  configuration
.plcor_470:
        cmpobne.f r5,r11,.plcor_475     # Jif source MAG serial # does not
                                        #  match
        ldl     CMsp_rs_opchk_rdsn(r7),r4 # r4 = cor_rdsn from remote
                                        # r5 = cor_rscl, cor_rsvd
                                        #      cor_rdcl, cor_rdvd from remote
        ld      cor_rscl(g3),r6         # r6 = cor_rscl, cor_rsvd
                                        #      cor_rdcl, cor_rdvd
        cmpobne.f r4,r12,.plcor_475     # Jif dest. MAG serial # does not
                                        #  match
        cmpobe.t r5,r6,.plcor_535       # Jif all values match
.plcor_475:
        setbit  31,r14,r14              # set send Establish Copy Operation
                                        #  State datagram to remote MAG flag
#
# --- Some copy operation component values do not match. Determine what
#       changes need to be made between the two MAGNITUDES, make them
#       and then communicate them to the remote MAGNITUDE.
#
        cmpobe.f r13,r11,.plcor_500     # Jif processing source copy device
                                        #  on remote MAG
#
# --- Processing destination copy device on remote MAG.
#
        ldl     CMsp_rs_opchk_rssn(r7),r4 # r4 = cor_rssn from remote
                                        # r5 = cor_rdsn from remote
        cmpobne.f r5,r12,.plcor_485     # Jif dest. MAG serial # different
#
# --- Processing destination copy device on remote MAG.
# --- Destination MAG serial # the same.
#
        ldos    CMsp_rs_opchk_rdcl(r7),r6 # r6 = cor_rdcl, cor_rdvd from
                                        #        remote
        ldos    cor_rdcl(g3),r5         # r5 = cor_rdcl, cor_rdvd
        cmpobe.f r6,r5,.plcor_535       # Jif values the same
#
# --- Processing destination copy device on remote MAG.
# --- Destination MAG serial # the same.
# --- Destination MAG cluster/VDisk # different.
#
        stos    r6,cor_rdcl(g3)         # save remote values in COR
        setbit  30,r14,r14              # set flag indicating need to save
                                        #  configuration
        b       .plcor_535              # and we're done here

.plcor_485:
#
# --- Processing destination copy device on remote MAG.
# --- Destination MAG serial # different.
#
        cmpobe.t r4,r12,.plcor_487      # Jif remote source MAG serial #
                                        #  matches my dest. MAG serial #
                                        #  indicating a swap occurred.
#
# --- Processing destination copy device on remote MAG.
# --- Destination MAG serial # different.
# --- The remote source MAG serial # and the remote destination
#       MAG serial # do not match my destination MAG serial #.
#       This indicates that this remote MAGNITUDE is not associated
#       with this copy operation. This should never occur!!! Just
#       try to establish the copy operation as defined locally by
#       sending an Establish Copy Operation State datagram to the
#       remote MAGNITUDE and try to get it back into sync.
#
        b       .plcor_540              # and we're done here

.plcor_487:
#
# --- Processing destination copy device on remote MAG.
# --- Destination MAG serial # different.
# --- Remote source MAG serial # matches the local destination MAG
#       serial #. A copy device swap needs to occur.
#
        ldos    CMsp_rs_opchk_rscl(r7),r4 # r4 = cor_rscl, cor_rsvd from
                                        #  remote
        ldos    cor_rdcl(g3),r5         # r5 = local cor_rdcl, cor_rdvd
        cmpobe.t r4,r5,.plcor_535       # Jif values the same
        stos    r4,cor_rdcl(g3)         # save remote values in COR
        setbit  30,r14,r14              # set flag indicating need to save
                                        #  configuration
        b       .plcor_535

.plcor_500:
#
# --- Processing source copy device on remote MAG.
#
        ldl     CMsp_rs_opchk_rssn(r7),r4 # r4 = cor_rssn from remote
                                        # r5 = cor_rdsn from remote
        cmpobne.f r4,r11,.plcor_515     # Jif source MAG serial # different
#
# --- Processing source copy device on remote MAG.
# --- Source MAG serial # the same.
#
        ldos    CMsp_rs_opchk_rscl(r7),r6 # r6 = cor_rscl, cor_rsvd from
                                        #        remote
        ldos    cor_rscl(g3),r4         # r4 = cor_rscl, cor_rsvd
        cmpobe.f r6,r4,.plcor_535       # Jif values the same
#
# --- Processing source copy device on remote MAG.
# --- Source MAG serial # the same.
# --- Source MAG cluster/VDisk # different.
#
        stos    r6,cor_rscl(g3)         # save remote values in COR
        setbit  30,r14,r14              # set flag indicating need to save
                                        #  configuration
        b       .plcor_535

.plcor_515:
#
# --- Processing source copy device on remote MAG.
# --- Source MAG serial # different.
#
        cmpobe.t r5,r11,.plcor_517      # Jif remote dest. MAG serial #
                                        #  matches my source MAG serial #
                                        #  indicating a swap occurred.
#
# --- Processing source copy device on remote MAG.
# --- Source MAG serial # different.
# --- The remote source MAG serial # and the remote destination
#       MAG serial # do not match my source MAG serial #.
#       This indicates that this remote MAGNITUDE is not associated
#       with this copy operation. This should never occur!!! Just
#       try to establish the copy operation as defined locally by
#       sending an Establish Copy Operation State datagram to the
#       remote MAGNITUDE and try to get it back into sync.
#
        b       .plcor_540              # and we're done here

.plcor_517:
#
# --- Processing source copy device on remote MAG.
# --- Source MAG serial # different.
# --- Remote destination MAG serial # matches the local source MAG
#       serial #. A copy device swap needs to occur.
#
        ldos    CMsp_rs_opchk_rdcl(r7),r4 # r4 = cor_rdcl, cor_rdvd from
                                        #  remote
        ldos    cor_rscl(g3),r5         # r5 = local cor_rscl, cor_rsvd
        cmpobe.t r4,r5,.plcor_535       # Jif values the same
        stos    r4,cor_rscl(g3)         # save remote values in COR
        setbit  30,r14,r14              # set flag indicating need to save
                                        #  configuration
.plcor_535:
        ldob    CMsp_rs_opchk_crstate(r7),r4 # r7 = copy reg. state of remote
                                        #  MAGNITUDE
        ldob    cor_crstate(g3),r5      # r5 = my copy reg. state
        cmpobe.t r4,r5,.plcor_538       # Jif copy reg. states the same
#
# --- The copy registration states do not match. Check which state is the
#       dominate one and establish this one as the current registration
#       state of the copy operation.
#
# ---------------------------------------------------------------
#  Remote Copy  |          Local Copy Registration State        |
#  Registration |-----------------------------------------------|
#     State     |    active     |  auto-suspend |  user-suspend |
# ==============|===============|===============|===============|=============
#     active    |    active     |  auto-suspend |  user-suspend |    New     |
# --------------|---------------|---------------|---------------|            |
#  auto-suspend |    active     |  auto-suspend |  user-suspend |Registration|
# --------------|---------------|---------------|---------------|            |
#  user-suspend |    active     |  auto-suspend |  user-suspend |    State   |
# --------------|---------------|---------------|---------------|            |
#  rem-suspend  |  user-suspend |  user-suspend |  user-suspend |            |
# ----------------------------------------------------------------------------
#
        cmpobne.t corcrst_remsusp,r4,.plcor_537 # Jif remote not rem-
                                        #  suspend registration state
        ldob    cor_crstate(g3),r4      # r4 = current cor_crstate value
        cmpobe  corcrst_usersusp,r4,.plcor_538 # Jif already in user suspended
                                        #        state
        ldconst corcrst_usersusp,r4     # r4 = user-suspend/local state code
        stob    r4,cor_crstate(g3)      # save new copy registration state
        setbit  4,r14,r14               # set cor_crstate changed to user-
                                        #  suspend state flag in return status
        call    CM$mmc_sflag            # process suspended flag for MMC
        b       .plcor_538

.plcor_537:
        setbit  31,r14,r14              # set send Establish Copy Operation
                                        #  State datagram to remote MAG flag
.plcor_538:
        ldob    CMsp_rs_opchk_mstate(r7),r4 # r4 = remote map table state
        cmpobe.t cormst_term,r4,.plcor_540 # Jif map table is terminated
        ldob    cor_crstate(g3),r5      # r5 = COR registration state
        cmpobe.t corcrst_active,r5,.plcor_539 # Jif in active state
        cmpobne.t corcrst_autosusp,r5,.plcor_540 # Jif not in auto-suspend
                                        #  state
.plcor_539:
        setbit  3,r14,r14               # set segment map accumulation flag
.plcor_540:
        lda     -dsc1_ulvl(g1),g1       # g1 = datagram ILT at nest level 1
        call    DLM$put_dg              # deallocate datagram ILT
.plcor_550:
        cmpobe.f r12,r13,.plcor_800     # Jif polled both remote MAGNITUDES
        mov     r12,r13                 # r13 = dest. MAG serial #
        b       .plcor_403              # go poll destination MAG
#
#**************************** End Case 4 processing **************************
#
# ***************************** Case 5 processing ****************************
#
#  INPUT:
#       r8 = cor_rid
#       r9 = cor_rcsn
#       r10 = cor_rcscl, cor_rcsvd
#             cor_rcdcl, cor_rcdvd
#       r11 = cor_rssn
#       r12 = cor_rdsn
#       r14 = completion status code (0)
#       g3 = COR address being processed
#
.plcor_600:
        ldconst 4,r3                    # r3 = error retry count
.plcor_605:
        mov     r11,g0                  # g0 = MAGNITUDE serial # to poll
        call    cm$pkop_check           # pack a Check Copy Operation State/
                                        # Status datagram
                                        # g1 = datagram ILT at nest level #2
        lda     DLM$send_dg,g0          # g0 = datagram service provider
                                        #  routine
        call    K$qw                    # Queue request w/wait
        ld      dsc2_rshdr_ptr(g1),r7   # r7 = local response header address
        ldob    dgrs_status(r7),r4      # r4 = request completion status
        ldob    dgrs_ec1(r7),r5         # r5 = error code byte #1
        cmpobe.t dg_st_ok,r4,.plcor_650 # Jif no error reported on request
        lda     -dsc1_ulvl(g1),g1       # g1 = datagram ILT at nest level 1
        call    DLM$put_dg              # deallocate datagram ILT
        cmpobne.f dg_st_srvr,r4,.plcor_640 # Jif not dest. server level
                                        #  error
#
# --- Destination server level error returned
#
        cmpobne.f dgec1_srvr_nocopy,r5,.plcor_620 # Jif not specified copy
                                        #  operation not defined
#
# --- Specified copy operation is no longer defined.
#
#.plcor_610:
        setbit  7,r14,r14               # set terminate and restart copy
                                        #  bit in completion status code
        b       .plcor_1000             # and we're out of here!

.plcor_620:
        cmpobne.f 0,r3,.plcor_630        # Jif retry count has not expired
        b       .plcor_990              # and we're out of here!

.plcor_630:
        subo    1,r3,r3                 # dec. error retry count
        b       .plcor_605              # and check it again

.plcor_640:
#       cmpobe.f dg_st_ddsp,r4,.plcor_620 # Jif dest. datagram service
                                        #  provider level error
#
# --- Datagram never made it to the destination server. Do not terminate
#       the copy operation for these types of errors. Can't communicate
#       with the remote MAGNITUDE so just ignore for now and move on with
#       other copy operation processing.
#
        cmpobne.t 0,r3,.plcor_630       # Jif error retry count has not
                                        #  expired
        setbit  0,r14,r14               # set flag indicating remote MAG was
                                        #  not polled
        b       .plcor_750

.plcor_650:
#
# --- Datagram response was successful. Check if response data matches
#       what we expect to see.
#
        call    cm$tr_chk_resp          # transpose response data fields
        ld      dsc2_rsbuf(g1),r7       # r7 = response message buffer
        lda     dgrs_size(r7),r7        # r7 = response data address
        ldl     CMsp_rs_opchk_rcscl(r7),r4 # r4 = cor_rcscl, cor_rcsvd
                                        #  cor_rcdcl, cor_rcdvd from remote
                                        # r5 = cor_rssn from remote
        cmpobe.t r4,r10,.plcor_670      # Jif copy MAG data matches
        setbit  31,r14,r14              # set send Establish Copy Operation
                                        #  State datagram to remote MAG flag
        mov     r11,g0                  # g0 = remote MAG serial # being
                                        #  processed
        ldob    cor_crstate(g3),r4      # r4 = current cor_crstate value
        call    cm$cmag_delta           # process copy MAG device differences
                                        #  between local COR values and check
                                        #  copy operation response values
        ldob    cor_crstate(g3),r6      # r6 = new cor_crstate value
        cmpobe.t r4,r6,.plcor_665       # Jif cor_crstate did not change
        cmpobne.f corcrst_usersusp,r6,.plcor_665 # Jif not user suspended state
        setbit  4,r14,r14               # set cor_crstate set to user suspend
                                        #  flag
.plcor_665:
        mov     r10,r4                  # r4 = old COR values
        ld      cor_rcscl(g3),r10       # reload values to reflect any changes
                                        #  that may have been made
        cmpobe.f r4,r10,.plcor_670      # Jif no COR values changed
        setbit  30,r14,r14              # set flag indicating need to save
                                        #  configuration
.plcor_670:
        cmpobne.f r5,r11,.plcor_675     # Jif source MAG serial # does not
                                        #  match
        ldl     CMsp_rs_opchk_rdsn(r7),r4 # r4 = cor_rdsn from remote
                                        # r5 = cor_rscl, cor_rsvd
                                        #      cor_rdcl, cor_rdvd from remote
        ld      cor_rscl(g3),r6         # r6 = cor_rscl, cor_rsvd
                                        #      cor_rdcl, cor_rdvd
        cmpobne.f r4,r12,.plcor_675     # Jif dest. MAG serial # does not
                                        #  match
        cmpobe.t r5,r6,.plcor_735       # Jif all values match
.plcor_675:
        ld      CMsp_rs_opchk_rssn(r7),r6 # r6 = cor_rssn from remote
        cmpobe.t r4,r6,.plcor_680       # Jif remote cor_rssn & cor_rdsn
                                        #  the same MAGNITUDE
.plcor_677:
#
# --- We expect the remote cor_rssn & cor_rdsn to be the same and to be
#       the polled remote MAG serial #. If this is not the case, what
#       else to do but terminate the copy operation???
#
        setbit  7,r14,r14               # set terminate and restart copy
                                        #  bit in completion status code
        b       .plcor_740              # and we're out of here!

.plcor_680:
        cmpobne.f r4,r11,.plcor_677     # Jif remote cor_rssn & cor_rdsn
                                        #  not the same as expected
        setbit  30,r14,r14              # set flag indicating need to save
                                        #  configuration
#
# --- We have to rely on the remote COR values. If they have changed,
#       the logic on the remote MAGNITUDE should maintain the correct
#       values and we need to adjust the local COR values to match
#       those returned from the remote MAGNITUDE.
#
        st      r5,cor_rscl(g3)         # save remote values in COR
.plcor_735:
        ldob    CMsp_rs_opchk_crstate(r7),r4 # r7 = copy reg. state of remote
                                        #  MAGNITUDE
        ldob    cor_crstate(g3),r5      # r5 = my copy reg. state
        cmpobe.t r4,r5,.plcor_738       # Jif copy reg. states the same
#
# --- The copy registration states do not match. Check which state is the
#       dominate one and establish this one as the current registration
#       state of the copy operation.
#
# ---------------------------------------------------------------
#  Remote Copy  |          Local Copy Registration State        |
#  Registration |-----------------------------------------------|
#     State     |    active     |  auto-suspend |  user-suspend |
# ==============|===============|===============|===============|=============
#     active    |    active     |  auto-suspend |  user-suspend |    New     |
# --------------|---------------|---------------|---------------|            |
#  auto-suspend |    active     |  auto-suspend |  user-suspend |Registration|
# --------------|---------------|---------------|---------------|            |
#  user-suspend |    active     |  auto-suspend |  user-suspend |    State   |
# --------------|---------------|---------------|---------------|            |
#  rem-suspend  |  user-suspend |  user-suspend |  user-suspend |            |
# ----------------------------------------------------------------------------
#
        cmpobne.t corcrst_remsusp,r4,.plcor_737 # Jif remote not rem-
                                        #  suspend registration state
        ldob    cor_crstate(g3),r4      # r4 = current cor_crstate value
        cmpobe  corcrst_usersusp,r4,.plcor_738 # Jif already in user suspended
                                        #        state
        ldconst corcrst_usersusp,r4     # r4 = user-suspend/local state code
        stob    r4,cor_crstate(g3)      # save new copy registration state
        setbit  4,r14,r14               # set cor_crstate changed to user-
                                        #  suspend state flag in return status
        call    CM$mmc_sflag            # process suspended flag for MMC
        b       .plcor_738

.plcor_737:
        setbit  31,r14,r14              # set send Establish Copy Operation
                                        #  State datagram to remote MAG flag
.plcor_738:
        ldob    CMsp_rs_opchk_mstate(r7),r4 # r4 = remote map table state
        cmpobe.t cormst_term,r4,.plcor_740 # Jif map table is terminated
        ldob    cor_crstate(g3),r5      # r5 = COR registration state
        cmpobe.t corcrst_active,r5,.plcor_739 # Jif in active state
        cmpobne.t corcrst_autosusp,r5,.plcor_740 # Jif not in auto-suspend
                                        #  state
.plcor_739:
        setbit  3,r14,r14               # set segment map accumulation flag
.plcor_740:
        lda     -dsc1_ulvl(g1),g1       # g1 = datagram ILT at nest level 1
        call    DLM$put_dg              # deallocate datagram ILT
.plcor_750:
        b       .plcor_800              # check if need to establish copy
                                        #  operation state to remote MAG
#
# **************************** End Case 5 processing *************************
#
# --- Check if need to send Establish Copy Operation State datagrams
#       to either the source copy device MAGNITUDE or the destination
#       copy device MAGNITUDE.
#
.plcor_800:
        bbc.t   31,r14,.plcor_1000      # Jif flag not set indicating to send
                                        #  an Establish Copy Operation State
                                        #  datagram to remote MAGNITUDES
        cmpobe.t r9,r11,.plcor_900      # Jif source copy device is local
#
# --- Source copy device is remote MAG. Pack and send an Establish
#       Copy Operation State datagram message to the source MAGNITUDE.
#
        ldconst 4,r3                    # r3 = error retry count
.plcor_805:
        mov     r11,g0                  # g0 = MAGNITUDE serial # to send to
        call    cm$pkop_state           # pack an Establish Copy Operation
                                        # State datagram
                                        # g1 = datagram ILT at nest level #2
        lda     DLM$send_dg,g0          # g0 = datagram service provider
                                        #  routine
        call    K$qw                    # Queue request w/wait
        ld      dsc2_rshdr_ptr(g1),r7   # r7 = local response header address
        ldob    dgrs_status(r7),r4      # r4 = request completion status
        ldob    dgrs_ec1(r7),r5         # r5 = error code byte #1
        lda     -dsc1_ulvl(g1),g1       # g1 = datagram ILT at nest level 1
        call    DLM$put_dg              # deallocate datagram ILT
        cmpobe.t dg_st_ok,r4,.plcor_900 # Jif no error reported on request
        cmpobne.f dg_st_srvr,r4,.plcor_880 # Jif not dest. server level
                                        #  error
#
# --- Destination server level error returned
#
        cmpobne.f dgec1_srvr_nocopy,r5,.plcor_820 # Jif not specified copy
                                        #  operation not defined
#
# --- Specified copy operation is no longer defined.
#
# .plcor_810:
        setbit  7,r14,r14               # set terminate and restart copy
                                        #  bit in completion status code
        b       .plcor_1000             # and we're out of here!

.plcor_820:
        cmpobne.f dgec1_srvr_nosvlink,r5,.plcor_830 # Jif not VLink not
                                        #  established to specified source
                                        #  copy device error
#
# --- VLink not established to specified source copy device.
#
        b       .plcor_900

.plcor_830:
        cmpobne.f dgec1_srvr_nodvlink,r5,.plcor_840 # Jif not VLink not
                                        #  established to specified dest.
                                        #  copy device error
#
# --- VLink not established to specified destination copy device.
#
        b       .plcor_900

.plcor_840:
        cmpobne.f dgec1_srvr_inuse,r5,.plcor_850 # Jif not specified copy
                                        #  device in use error
#
# --- Specified copy device in use.
#
        b       .plcor_900

.plcor_850:
        cmpobe.f 0,r3,.plcor_900        # Jif retry count has expired
        subo    1,r3,r3                 # dec. error retry count
        b       .plcor_805              # and check it again

# .plcor_870:
#         cmpobe.f 0,r3,.plcor_810        # Jif retry count has expired
.plcor_875:
        subo    1,r3,r3                 # dec. error retry count
        b       .plcor_805              # and check it again

.plcor_880:
#       cmpobe.f dg_st_ddsp,r4,.plcor_870 # Jif dest. datagram service
                                        #  provider level error
#
# --- Datagram never made it to the destination server. Do not terminate
#       the copy operation for these types of errors. Can't communicate
#       with the remote MAGNITUDE so just ignore for now and move on with
#       other copy operation processing.
#
        cmpobne.t 0,r3,.plcor_875       # Jif error retry count has not
                                        #  expired
.plcor_900:
        cmpobe.t r9,r12,.plcor_1000     # Jif dest. copy device is local
#
# --- Destination copy device is remote MAG. Pack and send an Establish
#       Copy Operation State datagram message to the destination MAGNITUDE.
#
        ldconst 4,r3                    # r3 = error retry count
.plcor_905:
        mov     r12,g0                  # g0 = MAGNITUDE serial # to send to
        call    cm$pkop_state           # pack an Establish Copy Operation
                                        # State datagram
                                        # g1 = datagram ILT at nest level #2
        lda     DLM$send_dg,g0          # g0 = datagram service provider
                                        #  routine
        call    K$qw                    # Queue request w/wait
        ld      dsc2_rshdr_ptr(g1),r7   # r7 = local response header address
        ldob    dgrs_status(r7),r4      # r4 = request completion status
        ldob    dgrs_ec1(r7),r5         # r5 = error code byte #1
        lda     -dsc1_ulvl(g1),g1       # g1 = datagram ILT at nest level 1
        call    DLM$put_dg              # deallocate datagram ILT
        cmpobe.t dg_st_ok,r4,.plcor_1000 # Jif no error reported on request
        cmpobne.f dg_st_srvr,r4,.plcor_980 # Jif not dest. server level
                                        #  error
#
# --- Destination server level error returned
#
        cmpobne.f dgec1_srvr_nocopy,r5,.plcor_920 # Jif not specified copy
                                        #  operation not defined
#
# --- Specified copy operation is no longer defined.
#
#.plcor_910:
        setbit  7,r14,r14               # set terminate and restart copy
                                        #  bit in completion status code
        b       .plcor_1000             # and we're out of here!

.plcor_920:
        cmpobne.f dgec1_srvr_nosvlink,r5,.plcor_930 # Jif not VLink not
                                        #  established to specified source
                                        #  copy device error
#
# --- VLink not established to specified source copy device.
#
        b       .plcor_1000

.plcor_930:
        cmpobne.f dgec1_srvr_nodvlink,r5,.plcor_940 # Jif not VLink not
                                        #  established to specified dest.
                                        #  copy device error
#
# --- VLink not established to specified destination copy device.
#
        b       .plcor_1000

.plcor_940:
        cmpobne.f dgec1_srvr_inuse,r5,.plcor_950 # Jif not specified copy
                                        #  device in use error
#
# --- Specified copy device in use.
#
        b       .plcor_1000

.plcor_950:
        cmpobe.f 0,r3,.plcor_1000       # Jif retry count has expired
        subo    1,r3,r3                 # dec. error retry count
        b       .plcor_905              # and check it again

.plcor_970:
        cmpobne.f 0,r3,.plcor_975        # Jif retry count has not expired
        b       .plcor_990              # and we're out of here!

.plcor_975:
        subo    1,r3,r3                 # dec. error retry count
        b       .plcor_905              # and check it again

.plcor_980:
        cmpobe.f dg_st_ddsp,r4,.plcor_970 # Jif dest. datagram service
                                        #  provider level error
#
# --- Datagram never made it to the destination server. Do not terminate
#       the copy operation for these types of errors. Can't communicate
#       with the remote MAGNITUDE so just ignore for now and move on with
#       other copy operation processing.
#
        cmpobne.t 0,r3,.plcor_975       # Jif error retry count has not
                                        #  expired
        b       .plcor_1000
#
.plcor_990:
        setbit  0,r14,r14               # set remote MAG not polled
#
.plcor_1000:
        bbc.t   30,r14,.plcor_1050      # Jif configuration save not specified
        call    CCSM$cd_moved           # notify copy management as needed
.plcor_1050:
        bbc     4,r14,.plcor_1100       # Jif cor_crstate did not change
        call    CCSM$cs_chged_w_msg     # generate a copy state changed event
.plcor_1100:
        ldob    cor_flags(g3),r4        # r4 = COR flags byte
        ldconst 0xff,r5
        clrbit  CFLG_B_POLL_REQ,r4,r4   # Clear outstanding poll request flag
        stob    r4,cor_flags(g3)        # save updated COR flags byte
        and     r5,r14,g0               # g0 = completion status code
        mov     r15,g1                  # restore g1
        ret
#
#******************************************************************************
#
#  NAME: cm$poll_remote_cor
#
#  PURPOSE:
#       Poll routine for remote CORs.
#
#  DESCRIPTION:
#       This routine polls the copy manager MAGNITUDE associated with
#       the specified copy operation to determine the status/state of
#       the copy operation since it has not been polled from the copy
#       manager node in a long time. It packs and sends a poll request
#       to the copy manager MAGNITUDE and processes any errors returned
#       from the copy manager MAGNITUDE.
#
#  CALLING SEQUENCE:
#       call    cm$poll_remote_cor
#
#  INPUT:
#       g3 = COR address of remote copy operation to process
#
#       Note: This routine stalls the calling routine's process waiting
#               for responses to datagram messages sent to peer nodes.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
cm$poll_remote_cor:
        movl    g0,r14                  # save g0-g1
        ldq     cor_rid(g3),r8          # r8 = cor_rid
                                        # r9 = cor_rcsn
                                        # r10 = cor_rcscl, cor_rcsvd
                                        #       cor_rcdcl, cor_rcdvd
                                        # r11 = cor_rssn
        ldl     cor_rdsn(g3),r12        # r12 = cor_rdsn
                                        # r13 = cor_rscl, cor_rsvd
                                        #       cor_rdcl, cor_rdvd
        ldconst 4,r3                    # r3 = error retry count
.prcor_50:
        mov     r9,g0                   # g0 = copy manager MAGNITUDE serial #
        call    cm$pkop_check           # pack a Check Copy Operation State/
                                        # Status datagram
                                        # g1 = datagram ILT at nest level #2
        lda     DLM$send_dg,g0          # g0 = datagram service provider
                                        #  routine
        call    K$qw                    # Queue request w/wait
        ld      dsc2_rshdr_ptr(g1),r7   # r7 = local response header address
        ldob    dgrs_status(r7),r4      # r4 = request completion status
        ldob    dgrs_ec1(r7),r5         # r5 = error code byte #1
        cmpobe.t dg_st_ok,r4,.prcor_500 # Jif no error reported on request
        lda     -dsc1_ulvl(g1),g1       # g1 = datagram ILT at nest level 1
        call    DLM$put_dg              # deallocate datagram ILT
        cmpobne.f dg_st_srvr,r4,.prcor_200 # Jif not dest. server level
                                        #  error
#
# --- Destination server level error returned
#
        cmpobne.f dgec1_srvr_nocopy,r5,.prcor_110 # Jif not specified copy
                                        #  operation not defined
#
# --- Specified copy operation is no longer defined. Validate that the
#       copy operation is still active and if so terminate it.
#
.prcor_100:
        movl    r8,g0                   # g0 = cor_rid
                                        # g1 = cor_rcsn
        call    CM$valid_cor            # validate COR is still active
        cmpobe.f FALSE,g0,.prcor_1000   # COR is no longer valid. We're
                                        #  done and out of here!
        call    CCSM$rpoll_term         # terminate copy operation
        b       .prcor_1000             # and we're out of here!

.prcor_110:
        cmpobe.f 0,r3,.prcor_100        # Jif retry count has expired
.prcor_150:
        subo    1,r3,r3                 # dec. error retry count
        b       .prcor_50               # and check it again

.prcor_200:
        cmpobe.f dg_st_ddsp,r4,.prcor_110 # Jif dest. datagram service
                                        #  provider level error
#
# --- Datagram never made it to the destination server. Do not terminate
#       the copy operation for these types of errors.
#
        cmpobe.f 0,r3,.prcor_1000       # Jif error retry count has expired
        b       .prcor_150              # dec. retry count and try it again

.prcor_500:
#
# --- Datagram response was successful. Check if my node is still
#       associated with this copy operation.
#
        call    cm$tr_chk_resp          # transpose response data fields
        ld      dsc2_rsbuf(g1),r7       # r7 = response message buffer
        lda     dgrs_size(r7),r7        # r7 = response data address
        ld      CMsp_rs_opchk_rssn(r7),r4 # r4 = source MAG serial #
        ld      CMsp_rs_opchk_rdsn(r7),r5 # r5 = dest. MAG serial #
        ldob    CMsp_rs_opchk_cstate(r7),r6 # r6 = cor_cstate value
        lda     -dsc1_ulvl(g1),g1       # g1 = datagram ILT at nest level 1
        call    DLM$put_dg              # deallocate datagram ILT
        movl    r8,g0                   # g0 = cor_rid
                                        # g1 = cor_rcsn
        call    CM$valid_cor            # validate COR is still active
        cmpobe.f FALSE,g0,.prcor_1000   # COR is no longer valid. We're
                                        #  done and out of here!
        stob    r6,cor_cstate(g3)       # save copy owner cor_cstate value
        ld      K_ficb,r3               # r3 = FICB address
        ld      fi_vcgid(r3),r3         # r3 = my serial #
        cmpobe.t r4,r3,.prcor_1000      # Jif my MAG contains the source
                                        #  copy device
        cmpobe.t r5,r3,.prcor_1000      # Jif my MAG contains the dest.
                                        #  copy device
#
# --- My MAGNITUDE is no longer associated with this copy operation.
#       Terminate the copy operation on my node.
#
        call    CCSM$rpoll_term         # terminate copy operation
.prcor_1000:
        movl    r14,g0                  # restore g0-g1
        ret
#
#******************************************************************************
#
#  NAME: CM$pksnd_local_poll
#  NAME: cm$pksnd_local_poll
#
#  PURPOSE:
#       Packs and sends a poll request PCP to the copy manager task
#       associated with the specified local copy operation.
#
#  DESCRIPTION:
#       This routine packs and sends a poll request PCP to the copy
#       manager associated with the specified local copy operation.
#
#  CALLING SEQUENCE:
#       call    CM$pksnd_local_poll
#       call    cm$pksnd_local_poll
#
#  INPUT:
#       g3 = COR address of local copy operation to process
#       g4 = CM address of local copy operation to process
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
# C access
CM_pksnd_local_poll:
CM$pksnd_local_poll:
cm$pksnd_local_poll:
        movl    g0,r14                  # save g0-g1
c       g1 = get_ilt();                 # Allocate an ILT (PCP)
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
        mov     0,r3                    # r3 = 0
        lda     D$ctlrqst_cr,r4         # r4 = completion address
        ld      cm_pcb(g4),r5           # r4 = task PCB
        st      g4,pcp1_cm(g1)          # save CM address
        st      r4,pcp1_cr(g1)          # save completion routine address
        st      r5,pcp1_pcb(g1)         # save PCB
        lda     ILTBIAS(g1),g1          # g1 = PCP level 2
        st      r3,pcp2_status(g1)      # clear out status,function
        st      r3,pcp2_handler(g1)     # clear Vsync handler
        ldconst pcp1fc_poll,r3          # set poll function code
        stob    r3,pcp2_function(g1)    # save function
        ldq     cor_rid(g3),r4          # r4-r7 = COR reg. info
        ldl     cor_rdsn(g3),r8         # r8-r9
        stq     r4,pcp2_rid(g1)         # save COR reg. info
        stl     r8,pcp2_rdsn(g1)
        st      g3,pcp2_cor(g1)         # save COR address
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>djkcm_cm$pksnd_local_poll-- posting PCP/ILT with pcp1fcpoll(pcp1fc_poll) function to CM task CM$pexec\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # CM_IM_DEBUG
        call    CM$ctlrqstq             # enqueue the control request
        movl    r14,g0                  # restore g0-g1
        ret
#
#******************************************************************************
#
#  NAME: CM$update_rmap
#
#  PURPOSE:
#       Totally updates the region/segment map table for the specified
#       COR.
#
#  DESCRIPTION:
#       This routine checks if a region map is defined for the specified
#       COR. If not, it is done. If one exists, it looks at each segment
#       map table and if defined, counts the segment bits for the table
#       and saves the updated count in the segment table. If no segment
#       bits are set for a defined segment table, it deallocates the
#       segment map table and clears it from the region map table. If
#       it ends up that no segment map tables are associated with the
#       region map table, it deallocates the region map table and clears
#       it from the specified COR.
#
#  CALLING SEQUENCE:
#       call    CM$update_rmap
#
#  INPUT:
#       g3 = COR address to process
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
CM$update_rmap:
        movl    g0,r14                  # save g0-g1
        ld      cor_rmaptbl(g3),g0      # g0 = assoc. region map table
                                        #  address
        ldconst FALSE,r12               # r12 = update configuration flag
        cmpobe.f 0,g0,.uprmap_1000      # Jif no region map table defined
        ldconst 0,r3                    # r3 = 0
        mov     r3,r13                  # r13 = number of segment map
                                        #  tables still allocated
        mov     r3,r11                  # r11 = current region # being
                                        #  processed
        ldconst maxRMcnt,r10            # r10 = max. # regions supported
.uprmap_200:
        ld      RM_tbl(g0)[r11*4],g1    # g1 = segment map table for current
                                        #      region
        cmpobe.t 0,g1,.uprmap_300       # Jif segment map table not defined
        call    CM$cnt_smap             # count segment bits in segment map
        ld      SM_cnt(g1),r4           # r4 = # segment bits set in segment
                                        #  map table
        cmpobne.t 0,r4,.uprmap_250      # Jif segment map table not empty
        st      r3,RM_tbl(g0)[r11*4]    # clear segment map table from region
                                        #  map table
.ifdef M4_DEBUG_SM
c fprintf(stderr, "%s%s:%u put_sm 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_SM
c       put_sm(g1);                     # Deallocate segment map table
        mov     g0,r12                  # save g0
        mov     r11,g0                  # g0 = region # being cleared
        call    CCSM$reg_sync           # clear region bit in state NVRAM
        mov     r12,g0                  # restore g0
        ldconst TRUE,r12                # r12 = flag indicating configuration needs
                                        #  to be saved
        b       .uprmap_300

.uprmap_250:
        addo    1,r13,r13               # inc. segment map table count
.uprmap_300:
        addo    1,r11,r11               # inc. current region map #
        cmpobl.t r11,r10,.uprmap_200    # Jif more regions to check
        cmpobne.t 0,r13,.uprmap_1000    # Jif some segment bits set and need
                                        #  to maintain the region map table
        st      r3,cor_rmaptbl(g3)      # clear region map table from COR
.ifdef M4_DEBUG_RM
c fprintf(stderr, "%s%s:%u put_rm 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif # M4_DEBUG_RM
c       put_rm(g0);                     # Release region map table (RM)
.uprmap_1000:
        cmpobe.t FALSE,r12,.uprmap_2000 # Jif configuration update flag FALSE
.uprmap_2000:
        movl    r14,g0                  # restore g0-g1
        ret
#
#******************************************************************************
#
#  NAME: CM$cnt_smap
#        CM_cnt_smap
#
#  PURPOSE:
#       Counts the segment bits set in the specified segment map table.
#
#  DESCRIPTION:
#       This routine counts the segment bits in the specified segment
#       map table. It sets the count in the SM_cnt field of the
#       specified segment map table.
#
#  CALLING SEQUENCE:
#       call    CM$cnt_smap
#
#  INPUT:
#       g1 = segment map table to process
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
CM$cnt_smap:
CM_cnt_smap:
        mov     0,r12                   # r12 = segment bit count
        ldconst 0xffffffff,r11          # r11 = all segment bits set value
        ldconst 32,r10                  # r10 = segment bit count when all
                                        #  segment bits set in word
        mov     0,r9                    # r9 = word index into segment map
                                        #  table
        ldconst SMTBLsize,r8            # max. segment map table index
.cntsmap_200:
        ld      SM_tbl(g1)[r9*1],r4     # r4 = segment map word
        addo    4,r9,r9                 # inc. segment map index word
        cmpobe.t 0,r4,.cntsmap_300      # Jif segment word all zeros
        cmpobne.t r4,r11,.cntsmap_250   # Jif segment word not all ones
        addo    r10,r12,r12             # update segment bit count
        b       .cntsmap_300

.cntsmap_250:
        scanbit r4,r5                   # r5 = MSbit # that is set
        addo    1,r12,r12               # inc. segment bit count
        clrbit  r5,r4,r4                # clear MSbit # that is set
        cmpobne.t 0,r4,.cntsmap_250     # Jif more bits set in segment word
.cntsmap_300:
        cmpobl.t r9,r8,.cntsmap_200     # Jif more segment words to check
        st      r12,SM_cnt(g1)          # save segment bit count in segment
                                        #  map table
        ret
#
#******************************************************************************
#
#  NAME: cm$tr_chk_resp
#
#  PURPOSE:
#       Transpose Check Copy Operation response data.
#
#  DESCRIPTION:
#       This routine transposes the response data fields of a Check
#       Copy Operation datagram message.
#
#  CALLING SEQUENCE:
#       call    cm$tr_chk_resp
#
#  INPUT:
#       g1 = Check Copy Operation datagram ILT at nest level #2
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
cm$tr_chk_resp:
        ld      dsc2_rsbuf(g1),r3       # r3 = response message buffer
        lda     dgrs_size(r3),r3        # r3 = response data address
        ldq     CMsp_rs_opchk_rid(r3),r4 # r4 = copy reg. ID
                                        # r5 = copy MAG serial #
                                        # r6 = copy MAG source cluster #
                                        #      copy MAG source VDisk #
                                        #      copy MAG dest. cluster #
                                        #      copy MAG dest. VDisk #
                                        # r7 = source MAG serial #
        ldl     CMsp_rs_opchk_rdsn(r3),r8 # r8 = dest. MAG serial #
                                        # r9 = source MAG cluster #
                                        #      source MAG VDisk #
                                        #      dest. MAG cluster #
                                        #      dest. MAG VDisk #
        bswap   r4,r4                   # bswap copy reg. ID
        bswap   r5,r5                   # bswap copy MAG serial #
        bswap   r7,r7                   # bswap source MAG serial #
        bswap   r8,r8                   # bswap dest. MAG serial #
        stq     r4,CMsp_rs_opchk_rid(r3)
        stl     r8,CMsp_rs_opchk_rdsn(r3)
        ret
#
#******************************************************************************
#
#  NAME: cm$cmag_delta
#
#  PURPOSE:
#       Process differences in the copy MAG COR registration values
#       between the local COR values and the corresponding values
#       received from a remote MAGNITUDE with different values.
#
#  DESCRIPTION:
#       This routine processes differences in the following copy operation
#       registration values between the local COR values and the values
#       returned from a remote MAGNITUDE associated with the specified
#       copy operation.
#
#       - cor_rcscl
#       - cor_rcsvd
#       - cor_rcdcl
#       - cor_rcdvd
#
#       This routine determines the relationship of the responding
#       MAGNITUDE and checks those returned values with the local
#       COR values. If they are the same, no local COR changes are
#       made. If they are different, the following cases are identified
#       and the appropriate changes are made to the local copy operation
#       environment.
#
#       Case 1 - The local COR is associated with an active VLink but
#                the VLink is not registered at the remote MAGNITUDE.
#                The association with the VLink is terminated and the
#                copy MAG values are set to the unassigned values.
#       Case 2 - The local COR is not associated with an active VLink
#                (i.e. unassigned) but the copy operation is associated
#                with an active VLink from the local node from the
#                specified cluster/VDisk # from the remote MAGNITUDE.
#                The specified VLink is validated to be active to the
#                specified copy device and if valid, the copy operation
#                is associated with the appropriate VLink. If the
#                specified VLink is not valid, no changes are made.
#       Case 3 - The local COR is associated with an active VLink that
#                is different then the VLink associated with the remote
#                MAGNITUDE copy device. The association with the VLink
#                is terminated, the local copy MAG values are set to the
#                unassigned values, the specified VLink is validated to
#                be active to the specified copy device and if valid the
#                copy operation is associated with the appropriate VLink.
#                If the specified VLink is not valid, no further changes
#                are made.
#
#  CALLING SEQUENCE:
#       call    cm$cmag_delta
#
#  INPUT:
#       g0 = remote MAGNITUDE serial # of datagram response data
#       g1 = Check Copy Operation datagram ILT at nest level #2
#            Note: datagram response data must be transposed prior
#                  to calling.
#       g3 = assoc. COR address
#
#  OUTPUT:
#       None.
#       Note: This routine can change the cor_crstate value to user suspended.
#               The calling routine may need to check if this has occurred
#               to process this event appropriately.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
cm$cmag_delta:
        movl    g0,r14                  # save g0-g1
                                        # r14 = remote MAG serial #
                                        # r15 = datagram ILT
        ld      dsc2_rsbuf(g1),r3       # r3 = response message buffer
        lda     dgrs_size(r3),r3        # r3 = response data address
        ldl     cor_rssn(g3),r12        # r12 = local cor_rssn value
                                        # r13 = local cor_rdsn value
        cmpobne.t r12,r14,.cmagdelta_500 # Jif specified MAG not the source
                                        #  copy device of copy operation
#
# --- Local COR has the specified remote MAGNITUDE as the source copy device.
#
        ld      CMsp_rs_opchk_rssn(r3),r4 # r4 = remote cor_rssn value
        cmpobne.f r4,r14,.cmagdelta_100 # Jif remote MAG does not indicate
                                        #  the source copy device
#
# --- The remote MAGNITUDE indicates being the source copy device.
#
        ldob    CMsp_rs_opchk_rcscl(r3),r4 # r4 = remote cor_rcscl value
        ldob    CMsp_rs_opchk_rcsvd(r3),r5 # r5 = remote cor_rcsvd value
        ldob    CMsp_rs_opchk_rscl(r3),r6  # r6 = remote cor_rscl value
        ldob    CMsp_rs_opchk_rsvd(r3),r7  # r7 = remote cor_rsvd value
        b       .cmagdelta_150

.cmagdelta_100:
        ld      CMsp_rs_opchk_rdsn(r3),r4 # r4 = remote cor_rdsn value
        cmpobe.t r4,r14,.cmagdelta_120  # Jif remote MAG indicates being the
                                        #  destination copy device
#
# --- The remote MAG does not indicate being either copy device.
#       Career change likely!
#
        b       .cmagdelta_500          # ignore until career change occurs

.cmagdelta_120:
#
# --- The remote MAGNITUDE indicates being the destination copy device.
#
        ldob    CMsp_rs_opchk_rcdcl(r3),r4 # r4 = remote cor_rcdcl value
        ldob    CMsp_rs_opchk_rcdvd(r3),r5 # r5 = remote cor_rcdvd value
        ldob    CMsp_rs_opchk_rdcl(r3),r6  # r6 = remote cor_rdcl value
        ldob    CMsp_rs_opchk_rdvd(r3),r7  # r7 = remote cor_rdvd value
#
#       r3 = response data buffer address
#       r4 = remote copy MAG cluster #
#       r5 = remote copy MAG VDisk #
#       r6 = remote MAG copy device cluster #
#       r7 = remote MAG copy device VDisk #
#       r12 = local cor_rssn value
#       r13 = local cor_rdsn value
#       r14 = specified remote MAG serial #
#
.cmagdelta_150:
        ldob    cor_rcscl(g3),r8        # r8 = local cor_rcscl value
        ldob    cor_rcsvd(g3),r9        # r9 = local cor_rcsvd value
        cmpobne.f r4,r8,.cmagdelta_200  # Jif cluster values different
        cmpobe.t r5,r9,.cmagdelta_500   # Jif VDisk # values the same
.cmagdelta_200:
#
# --- Terminate association with the local source copy device VLink
#       if necessary.
#
        ld      cor_scd(g3),g0          # g0 = assoc. SCD address
        call    CM$deact_scd            # deactivate SCD from VDD
        ld      cor_destvdd(g3),r8      # r8 = dest. VDD address
        cmpobe.f 0,r8,.cmagdelta_230    # Jif no dest. VDD address defined
        ld      vd_dcd(r8),r9           # r9 = DCD assoc. with VDD
        ld      cor_dcd(g3),r10         # r10 = DCD assoc. with COR
        cmpobne.f r9,r10,.cmagdelta_230 # Jif not the same DCDs
        ldconst 0xffff,r10
        stos    r10,vd_scorvid(r8)      # "clear" dest. copy virtual ID
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
.cmagdelta_230:
        ldob    cor_crstate(g3),r8      # r8 = local copy registration state
        cmpobe.t corcrst_usersusp,r8,.cmagdelta_330 # Jif copy op. is user
                                        #  suspended
        ldconst corcrst_usersusp,r8     # r8 = user suspended reg. state
        stob    r8,cor_crstate(g3)      # set reg. state to user suspended
.cmagdelta_330:
        ldconst 0,r8
        st      r8,cor_srcvdd(g3)       # clear source VDD from COR
        ldconst 0xff,r8
        stob    r8,cor_rcscl(g3)        # set copy MAG cluster to unassigned
        stob    r8,cor_rcsvd(g3)        # set copy MAG VDisk # to unassigned
        cmpobe.f r4,r8,.cmagdelta_500   # Jif remote copy MAG cluster # is
                                        #  set to unassigned
#
# --- form and validate VID then locate the associated VDD.
#
        shlo    8,r4,r9                 # r9 = form VID from MS and LS bytes
        addo    r5,r9,r9

        ldconst MAXVIRTUALS,r8          # r8 = max. VDisk #
        cmpobge.f r9,r8,.cmagdelta_500  # Jif VDisk # invalid
        ld      V_vddindx[r9*4],r9      # r9 = corresponding VDD
        cmpobe.f 0,r9,.cmagdelta_500    # Jif no VDisk defined
#
# --- Validate VLink being reported from remote MAGNITUDE matches
#       what's defined locally.
#
        ld      vd_rdd(r9),r8           # r8 = RDD address of first RAID
        ldob    rd_type(r8),r10         # r10 = RAID type code
        cmpobne.t rdlinkdev,r10,.cmagdelta_500 # Jif not a VLink type device

        ld      rd_psd(r8),r10          # r10 = assoc. PSD address
        ldos    ps_pid(r10),r11         # r11 = Physical ID
        ld      DLM_lddindx[r11*4],r11  # r11 = LDD address
        ldob    ld_class(r11),r10       # r10 = linked device class
        cmpobne.f ldmld,r10,.cmagdelta_500 # Jif not a MAGNITUDE link device
                                        #  type
        ld      ld_basesn(r11),r10      # r10 = source MAG serial #
        cmpobne.f r10,r14,.cmagdelta_500 # Jif VLink to a different MAG
        ldos    ld_altid(r11),r10       # r10 = alternate ID if defined
        cmpobne 0,r10,.cmagdelta_400    # Jif alternate ID defined
        ldob    ld_basecl(r11),r10      # r10 = source cluster number
        cmpobne.f r6,r10,.cmagdelta_500 # Jif source cluster # different
        ldob    ld_basevd(r11),r10      # r10 = source VDisk number
        cmpobne.f r7,r10,.cmagdelta_500 # Jif source VDisk # different
        b       .cmagdelta_420

.cmagdelta_400:
        ldob    ld_altid+1(r11),r10     # r10 = MSB of alternate ID
        clrbit  7,r10,r10               # clear alternate ID flag
        cmpobne r6,r10,.cmagdelta_500   # Jif source alt. ID different
        ldob    ld_altid(r11),r10       # r10 = LSB of alternate ID
        cmpobne r7,r10,.cmagdelta_500   # Jif source alt. ID different
#
# --- Local VLink definition matches remote MAG information
#
.cmagdelta_420:
        stob    r4,cor_rcscl(g3)        # save new cluster # in COR
        stob    r5,cor_rcsvd(g3)        # save new VDisk # in COR
        st      r9,scd_vdd(g0)          # save VDD address in SCD
        st      r9,cor_srcvdd(g3)       # save VDD address in COR
        call    CM$act_scd              # activate SCD with new VDD
        ld      cor_destvdd(g3),r8      # r8 = dest. VDD address
        cmpobe.f 0,r8,.cmagdelta_500    # Jif no dest. VDD address defined
        ld      vd_dcd(r8),r11          # r11 = DCD assoc. with VDD
        ld      cor_dcd(g3),r10         # r10 = DCD assoc. with COR
        cmpobne.f r11,r10,.cmagdelta_500 # Jif not the same DCDs
        ldos    vd_vid(r9),r10          # r10 = source VDisk #
        stos    r10,vd_scorvid(r8)
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
.cmagdelta_500:
        cmpobne.t r13,r14,.cmagdelta_1000 # Jif specified MAG not the dest.
                                        #  copy device of copy operation
#
# --- Local COR has the specified remote MAGNITUDE as the dest. copy device.
#
        ld      CMsp_rs_opchk_rdsn(r3),r4 # r4 = remote cor_rdsn value
        cmpobne.f r4,r14,.cmagdelta_600 # Jif remote MAG does not indicate
                                        #  the dest. copy device
#
# --- The remote MAGNITUDE indicates being the dest. copy device.
#
        ldob    CMsp_rs_opchk_rcdcl(r3),r4 # r4 = remote cor_rcdcl value
        ldob    CMsp_rs_opchk_rcdvd(r3),r5 # r5 = remote cor_rcdvd value
        ldob    CMsp_rs_opchk_rdcl(r3),r6  # r6 = remote cor_rdcl value
        ldob    CMsp_rs_opchk_rdvd(r3),r7  # r7 = remote cor_rdvd value
        b       .cmagdelta_650

.cmagdelta_600:
        ld      CMsp_rs_opchk_rssn(r3),r4 # r4 = remote cor_rssn value
        cmpobe.t r4,r14,.cmagdelta_620  # Jif remote MAG indicates being the
                                        #  source copy device
#
# --- The remote MAG does not indicate being either copy device.
#       Career change likely!
#
        b       .cmagdelta_1000         # ignore until career change occurs

.cmagdelta_620:
#
# --- The remote MAGNITUDE indicates being the source copy device.
#
        ldob    CMsp_rs_opchk_rcscl(r3),r4 # r4 = remote cor_rcscl value
        ldob    CMsp_rs_opchk_rcsvd(r3),r5 # r5 = remote cor_rcsvd value
        ldob    CMsp_rs_opchk_rscl(r3),r6  # r6 = remote cor_rscl value
        ldob    CMsp_rs_opchk_rsvd(r3),r7  # r7 = remote cor_rsvd value
#
#       r3 = response data buffer address
#       r4 = remote copy MAG cluster #
#       r5 = remote copy MAG VDisk #
#       r6 = remote MAG copy device cluster #
#       r7 = remote MAG copy device VDisk #
#       r12 = local cor_rssn value
#       r13 = local cor_rdsn value
#       r14 = specified remote MAG serial #
#
.cmagdelta_650:
        ldob    cor_rcdcl(g3),r8        # r8 = local cor_rcdcl value
        ldob    cor_rcdvd(g3),r9        # r9 = local cor_rcdvd value
        cmpobne.f r4,r8,.cmagdelta_700  # Jif cluster values different
        cmpobe.t r5,r9,.cmagdelta_1000  # Jif VDisk # values the same
.cmagdelta_700:
#
# --- Terminate association with the local destination copy device VLink
#       if necessary.
#
        ld      cor_dcd(g3),g0          # g0 = assoc. DCD address
        ld      dcd_vdd(g0),r10         # r10 = assoc. VDD of DCD
        cmpobe.f 0,r10,.cmagdelta_800   # Jif no VDD assoc. with DCD
        ld      vd_dcd(r10),r11         # r11 = DCD assoc. with VDD
        cmpobne.f g0,r11,.cmagdelta_730 # Jif DCD not assoc. with VDD
        lda     vdnomirror,r8           # set vd_mirror status
        stob    r8,vd_mirror(r10)       # to not mirrored
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
.cmagdelta_730:
        call    CM$deact_dcd            # deactivate DCD from VDD
.cmagdelta_800:
        ldob    cor_crstate(g3),r8      # r8 = local copy registration state
        cmpobe.t corcrst_usersusp,r8,.cmagdelta_830 # Jif copy op. is user
                                        #  suspended
        ldconst corcrst_usersusp,r8     # r8 = user suspended reg. state
        stob    r8,cor_crstate(g3)      # set reg. state to user suspended
.cmagdelta_830:
        ldconst 0,r8
        st      r8,cor_destvdd(g3)      # clear destination VDD from COR
        ldconst 0xff,r8
        stob    r8,cor_rcdcl(g3)        # set copy MAG cluster to unassigned
        stob    r8,cor_rcdvd(g3)        # set copy MAG VDisk # to unassigned
#
# --- Send user suspend message to MMC
#
        mov     g0,r10                  # save g0
        ldconst cmcc_UsrSpnd,g0         # g0 = User Suspended message
        call    cm$Log_Completion       # log completion
        mov     r10,g0                  # restore g0
        cmpobe.f r4,r8,.cmagdelta_1000  # Jif remote copy MAG cluster # is
                                        #  set to unassigned
#
# --- form and validate VID then locate the associated VDD.
#
        shlo    8,r4,r9                 # r9 = form VID from MS and LS bytes
        addo    r5,r9,r9

        ldconst MAXVIRTUALS,r8          # r8 = max. VDisk #
        cmpobge.f r9,r8,.cmagdelta_1000 # Jif VDisk # invalid
        ld      V_vddindx[r9*4],r9      # r9 = corresponding VDD
        cmpobe.f 0,r9,.cmagdelta_1000   # Jif no VDisk defined
#
# --- Validate VLink being reported from remote MAGNITUDE matches
#       what's defined locally.
#
        ld      vd_rdd(r9),r8           # r8 = RDD address of first RAID
        ldob    rd_type(r8),r10         # r10 = RAID type code
        cmpobne.t rdlinkdev,r10,.cmagdelta_1000 # Jif not a VLink type device

        ld      rd_psd(r8),r10          # r10 = assoc. PSD address
        ldos    ps_pid(r10),r11         # r11 = Physical ID
        ld      DLM_lddindx[r11*4],r11  # r11 = LDD address
        ldob    ld_class(r11),r10       # r10 = linked device class
        cmpobne.f ldmld,r10,.cmagdelta_1000 # Jif not a MAGNITUDE link device
                                        #  type
        ld      ld_basesn(r11),r10      # r10 = source MAG serial #
        cmpobne.f r10,r14,.cmagdelta_1000 # Jif VLink to a different MAG
#
# --- Local VLink definition matches remote MAG information
#
        ldos    ld_altid(r11),r10       # r10 = alternate ID if defined
        cmpobne 0,r10,.cmagdelta_850    # Jif alternate ID defined
        ldob    ld_basecl(r11),r10      # r10 = source cluster number
        cmpobne.f r6,r10,.cmagdelta_1000 # Jif source cluster # different
        ldob    ld_basevd(r11),r10      # r10 = source VDisk number
        cmpobne.f r7,r10,.cmagdelta_1000 # Jif source VDisk # different
        b       .cmagdelta_860

.cmagdelta_850:
        ldob    ld_altid+1(r11),r10     # r10 = MSB of alternate ID
        clrbit  7,r10,r10               # clear alternate ID flag
        cmpobne r6,r10,.cmagdelta_1000  # Jif source alt. ID different
        ldob    ld_altid(r11),r10       # r10 = LSB of alternate ID
        cmpobne r7,r10,.cmagdelta_1000  # Jif source alt. ID different
#
# --- Local VLink definition matches remote MAG information
#
.cmagdelta_860:
        stob    r4,cor_rcdcl(g3)        # save new cluster # in COR
        stob    r5,cor_rcdvd(g3)        # save new VDisk # in COR
        st      r9,dcd_vdd(g0)          # save VDD address in DCD
        st      r9,cor_destvdd(g3)      # save VDD address in COR
        call    CM$act_dcd              # activate DCD with new VDD
        lda     vdcopyuserpause,r8      # r8 - user pause state
        stob    r8,vd_mirror(r9)        # set new VDD mirror state to paused
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        ld      cor_srcvdd(g3),r8       # r8 = source VDD address
        cmpobe.f 0,r8,.cmagdelta_900    # Jif no source VDD address defined
        ldos    vd_vid(r8),r7           # r7 = source VDisk #
        stos    r7,vd_scorvid(r9)
.cmagdelta_900:
#
# --- Send user suspend message to MMC
#
        ldconst cmcc_UsrSpnd,g0         # g0 = User Suspended message
        call    cm$Log_Completion       # log completion
.cmagdelta_1000:
        movl    r14,g0                  # restore g0-g1
        ret
#
# --- Phase 2 WRITE update handler routines ---------------------------
#
#**********************************************************************
#
#  NAME: CM$wp2_null
#        CM_wp2_null
#
#  PURPOSE:
#       This routine performs nothing for phase 2 write update
#       processing.
#
#  DESCRIPTION:
#       This routine returns to the calling routine. Nothing needs to be done
#       to service the copy operation for phase 2 write update processing.
#
#  CALLING SEQUENCE:
#       call    CM$wp2_null
#               CM_wp2_null
#
#  INPUT:
#       g0 = FALSE
#       g1 = 0
#       g3 = assoc. SCD/DCD address
#       g4 = VRP function/strategy
#       g10 = I/O length
#       g11 = VRP pointer
#       g12 = assoc. VDD address
#       g13 = assoc. VRP SGL address
#       g14 = primary ILT/VRP address
#
#**********************************************************************
#
CM$wp2_null:
CM_wp2_null:
        ret

#**********************************************************************
#
#  NAME: CM_wp2_mirror
#
#  PURPOSE:
#       This routine performs phase 2 write update processing for
#       copy operations that have are in a mirrored state.
#
#  DESCRIPTION:
#       This routine builds up an ILT/VRP to write the associated
#       data out to the destination copy device.
#
#  CALLING SEQUENCE:
#       call    CM_wp2_mirror
#
#  INPUT:
#       g0 = FALSE
#       g1 = 0
#       g3 = assoc. SCD/DCD address
#       g4 = VRP function/strategy
#       g10 = I/O length
#       g11 = VRP pointer
#       g12 = assoc. VDD address
#       g13 = assoc. VRP SGL address
#       g14 = primary ILT/VRP address
#
#  OUTPUT:
#       g0 = TRUE/FALSE indicator if need to save configuration
#       g1 = secondary update ILT/VRP if write needs to be written
#            to another virtual device
#       g1 = 0 if no secondary ILT/VRP built
#
#  REGS. DESTROYED:
#       Reg. g0, g1 destroyed.
#
#**********************************************************************

CM_wp2_mirror:
c       g1 = build_sec_vrp((SCD *)g3, g4, (VRP *)g11, (ILT *)g14);
        ret

#**********************************************************************
#
#  NAME: CM_wp2_copy
#
#  PURPOSE:
#       This routine performs phase 2 write update processing for
#       copy operations that are actively copying data.
#
#  CALLING SEQUENCE:
#       call    CM_wp2_copy
#
#  INPUT:
#       g0 = FALSE
#       g1 = 0
#       g3 = assoc. SCD/DCD address
#       g4 = VRP function/strategy
#       g10 = I/O length
#       g11 = VRP pointer
#       g12 = assoc. VDD address
#       g13 = assoc. VRP SGL address
#       g14 = primary ILT/VRP address
#
#  OUTPUT:
#       g0 = FALSE
#       g1 = secondary update ILT/VRP if write needs to be written
#            to another virtual device
#       g1 = 0 if no secondary ILT/VRP built
#
#  REGS. DESTROYED:
#       Reg. g0, g1 destroyed.
#
#**********************************************************************

CM_wp2_copy:
c       g1 = cm_wp2_copy((SCD *)g3, g4, (VRP *)g11, (ILT *)g14);

        ret

#**********************************************************************
#
#  NAME: CM$wp2_suspend
#        CM_wp2_suspend
#
#  PURPOSE:
#       This routine performs phase 2 write update processing for
#       copy operations that have are in a suspended state.
#
#  DESCRIPTION:
#       This routine calculates the segment number(s) the write update
#       is associated with and determines if these segment bit(s) are
#       already set indicating that the segment already needs to be
#       resynchronized. If they are not already set, this routine
#       performs the operations necessary to set the associated segment
#       bit(s). It also determines if a segment map table is newly
#       defined and if so indicates back to the calling routine that
#       configuration needs to be saved.
#
#  CALLING SEQUENCE:
#       call    CM$wp2_suspend
#               CM_wp2_suspend
#
#  INPUT:
#       g0 = FALSE
#       g1 = 0
#       g3 = assoc. SCD/DCD address
#       g4 = VRP function/strategy
#       g10 = I/O length
#       g11 = VRP pointer
#       g12 = assoc. VDD address
#       g13 = assoc. VRP SGL address
#       g14 = primary ILT/VRP address
#
#  OUTPUT:
#       g0 = TRUE/FALSE indicator if need to save configuration
#       g1 = secondary update ILT/VRP if write needs to be written
#            to another virtual device
#       g1 = 0 if no secondary ILT/VRP built
#
#  REGS. DESTROYED:
#       Reg. g0, g1 destroyed.
#
#**********************************************************************
#
CM$wp2_suspend:
CM_wp2_suspend:

CM$wp2_inactive:
CM_wp2_inactive:
        movq    g0,r12                  # save g0-g3
                                        # r12 = FALSE
                                        # r13 = 0
                                        # r15 = assoc. SCD/DCD address
        ld      scd_cor(g3),g3          # g3 = assoc. COR address
        cmpobe.f 0,g3,.wp2suspend_1000  # Jif no COR associated with SCD/DCD
        ld      cor_srcvdd(g3),r8       # r8 is the source VDD
        cmpobe  0,r8,.wp2suspend_1000   # if no vdd, exit
# NOTE: use copy manager cal_seg_bit() method.
c       r11 = cal_seg_bit((UINT32*)&r10, ((VRP*)g11)->startDiskAddr, ((VRP*)g11)->startDiskAddr + g10 - 1, ((VDD*)r8)->devCap, 0);
#   r10 = segment bit
# NOTE: r10 is maximum of 22 bits.
#   r11 = number of segments

.wp2suspend_100:
        mov     r10,g0                  # g0 = segment # to check
        call    cm$chk_segment_bit      # check if segment bit is set/cleared
                                        # g2 = TRUE/FALSE (set/cleared)
        cmpobe.t TRUE,g2,.wp2suspend_400 # Jif segment bit already set
        ld      cor_rmaptbl(g3),r8      # r8 = region map table address
        cmpobe.f 0,r8,.wp2suspend_150   # Jif region map table not defined
        call    cm$cal_region           # calculate region this segment is in
                                        # g2 = region number associated with
                                        #      this segment
        ld      RM_tbl(r8)[g2*4],r7     # r7 = segment map table associated
                                        #      with this segment
        cmpobne.t 0,r7,.wp2suspend_200  # Jif segment map table already
                                        #  defined for this segment
.wp2suspend_150:
        ldconst TRUE,r12                # r12 = TRUE indication signifying
                                        #       configuration needs to be saved
.wp2suspend_200:
        ldconst 1,g1                    # g1 = number of segment bits to set
#c       fprintf(stderr,"<wp2_suspend-djkcm> calling cm$call_set_segment_bit....\n");
        mov     g4,r5                   # save g4
        ldconst 0,g4                    # Clear out primary VRP
        call    cm$set_segment_bit      # set segment bit
        mov     r5,g4                   # Restore g4
.wp2suspend_400:
        subo    1,r11,r11               # dec. number of segments associated
                                        #  with this write update
        addo    1,r10,r10               # inc. segment number being processed
        cmpobne.f 0,r11,.wp2suspend_100 # Jif more segments associated with
                                        #  this write update
.wp2suspend_1000:
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: cm$up1comp
#
#  PURPOSE:
#       This routine contains the logic to service an ILT/VRP/SN
#       completion processing for a write update operation associated
#       with a copy operation.
#
#  DESCRIPTION:
#       This routine checks for an error reported for the ILT/VRP/SN
#       and if one is indicated will report the event to the associated
#       copy manager task to be processed. If no errors are reported
#       for the operation, it deallocates the resources associated with
#       the ILT/VRP/SN, decrements the outstanding count in the primary
#       ILT and if this count has expired will perform the necessary
#       processing of the primary ILT to complete it and return it back
#       to the originator.
#
#  CALLING SEQUENCE:
#       call    cm$up1comp
#
#  INPUT:
#       g1 = ILT address
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       None.
#
#**********************************************************************
#
cm$up1comp:
        ld      il_w0(g1),r3            # r3 = VRP
        ld      il_w5(g1),r11           # r11 = assoc. COR address
        ldob    vr_status(r3),r4        # r4 = VRP completion status
        cmpobe.f 0,r11,cm$up_common     # Jif no COR associated with ILT
        ldos    cor_uops(r11),r5        # r5 = outstanding update count
        subo    1,r5,r5                 # dec. outstanding update count
        stos    r5,cor_uops(r11)        # save updated outstanding update
                                        #  count
.if  0 #GR_GEORAID15_DEBUG
       ld      il_w3(g1),r12            # r12 = primary ILT
       ld      il_w4(r12),r12           # source VDD
       ld      il_w4(g1),r13            # dest vdd
       ldos    vr_func(r3),r10
c fprintf(stderr,"%s%s:%u <GR><up1comp-djkcm>status = %lx  svid=%x dvid=%x func=%lx\n", FEBEMESSAGE, __FILE__, __LINE__,r4,(((VDD*)r12)->vid), (((VDD*)r13)->vid),r10);
.endif  # 0
        cmpobe.t ecok,r4,cm$up_common   # Jif ok

        ld      cor_cm(r11),r10         # r10 = assoc. CM address (if local
                                        #  copy operation)
                                        # 0 = remote copy operation
        cmpobne.t 0,r10,.up1_400        # Jif local copy operation
#
# --- Update error occurred for a remote copy operation.
#
        ldos    CM_rem_uperr,r10        # r10 = remote update error count
        addo    1,r10,r10               # inc. counter
        stos    r10,CM_rem_uperr        # save updated counter
        b       cm$up_common            # and finish processing update
                                        #  completion
#
# --- Update error occurred for a local copy operation.
#
.up1_400:
        mov     g1,r12                  # save g1
        ld      il_w3(g1),r8            # r11 = primary ILT
        ld      il_w0-ILTBIAS(r8),r6    # r6 = primary VRP
        lda     ILTBIAS(g1),g1          # inc. ILT to next nest level
        ldl     vr_vsda(r3),r8          # r8/9 = update SDA address
        ld      vr_vlen(r3),r7          # r7 = update length in sectors
        mov     0,r3                    # r3 = 0
        lda     cm$uperr_pcp_cr,r4      # r4 = completion address
        ld      cm_pcb(r10),r5          # r4 = task PCB

        st      r10,pcp1_cm(g1)         # save CM address
        stl     r8,pcp1_reg1(g1)        # pcp1_reg1/reg2 = update SDA
        st      r7,pcp1_reg3(g1)        # pcp1_reg3 = update length in sectors
        st      r6,pcp1_reg5(g1)        # pcp1_reg5 = update primary VRP
                                        # Used for Geo-RAID(BE switch failure case,
                                        # to track for write failure on dest vdd)
        st      r4,pcp1_cr(g1)          # save completion routine address
        st      r5,pcp1_pcb(g1)         # save PCB

        lda     ILTBIAS(g1),g1          # g1 = PCP level 2
        st      r3,pcp2_status(g1)      # clear out status,function
        st      r3,pcp2_handler(g1)     # clear Vsync handler
        ldconst pcp1fc_updateerr,r3     # set update error function code
        stob    r3,pcp2_function(g1)    # save function
        ldq     cor_rid(r11),r4         # r4-r7 = COR reg. info
        ldl     cor_rdsn(r11),r8        # r8-r9
        stq     r4,pcp2_rid(g1)         # save COR reg. info
        stl     r8,pcp2_rdsn(g1)
        st      r11,pcp2_cor(g1)        # save COR address
        call    CM$ctlrqstq             # enqueue the control request
        mov     r12,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: cm$up_common
#
#  PURPOSE:
#       This routine contains the common logic to service a VRP
#       completion associated with a write update for a copy operation.
#
#  DESCRIPTION:
#       This routine deallocates the resources associated with
#       the ILT/VRP, decrements the outstanding count in the primary
#       ILT and if this count has expired will perform the necessary
#       processing of the primary ILT to complete it and return it back
#       to the originator.
#
#  CALLING SEQUENCE:
#       call    cm$up_common
#
#  INPUT:
#       g1 = ILT address
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       None.
#
#**********************************************************************
#
cm$up_common:
        movt    g0,r12                  # save g0-g2
        ld      il_w3(g1),r11           # r11 = primary ILT

        call    cm$riv                  # release ILT/VRP
#
# --- Check pending I/O count within primary ILT
#
        ld      il_w1(r11),r3           # Adjust pending I/O count in
        subo    1,r3,r3                 # dec. pending I/O count
        st      r3,il_w1(r11)           # save updated pending I/O count
.if 0 #GR_GEORAID15_DEBUG
       ld      il_w4(r11),r4  # VDD # source
       ldos    vd_vid(r4),r4
       ld      il_w4(g1),r5   # dest vdd
       ldos    vd_vid(r5),r5
c fprintf(stderr,"%s%s:%u <GR><up_common-djkcm>pending count = %lx  svid=%lx dvid=%lx\n", FEBEMESSAGE, __FILE__, __LINE__,r3,r4,r5);
.endif  # 0
#
        cmpobne.f 0,r3,.upcommon_1000   # Jif additional pending I/O
                                        #  operations
#
# --- Check if CWIP record needs to be deallocated
#
        ld      il_w6(r11),g0           # g0 = CWIP record address
        cmpobe.t 0,g0,.upcommon_35      # Jif no CWIP record used
        mov     0,r4
        call    CCSM$put_cwip           # Release CWIP record
        st      r4,il_w6(r11)           # Clear CWIP record from ILT
#
.upcommon_35:
#
# --- Check if primary ILT is queued to associated VDD and if so
#       remove it from the queue.
#
#       r11 = primary ILT
#
        ld      il_w7(r11),r4           # r4 = assoc. VDD field in ILT/VRP
        cmpobe.t 0,r4,.upcommon_150     # Jif no assoc. VDD defined indicating
                                        #  the ILT/VRP is not queued to the
                                        #  assoc. VDD
        ldconst 0,r3                    # r3 = 0
        ld      il_fthd(r11),r5         # r5 = forward pointer
        ld      il_bthd(r11),r6         # r6 = backward pointer
        lda     vd_outhead-il_fthd(r4),r7 # r7 = address of head of list
        st      r3,il_w7(r11)           # clear assoc. VDD field in ILT
        st      r5,il_fthd(r6)          # unlink ILT/VRP from list
        cmpobne.f 0,r5,.upcommon_100    # Jif not at the tail of the list
        cmpo    r6,r7                   # check if request was at the head
                                        #  of the list
        lda     vd_outtail-il_bthd(r4),r5 # r5 = list tail pointer address
        sele    r6,0,r6                 # clear tail pointer if true
.upcommon_100:
        st      r6,il_bthd(r5)          # set backward pointer to next ILT
        st      r3,il_bthd(r11)         # clear backward pointer in ILT
        st      r3,il_fthd(r11)         # clear forward pointer in ILT
.upcommon_150:
        ld      il_w4(r11),r10          # r10 = VDD
#
# --- Adjust outstanding request count & update
#       queue depth in VDD
#
        ld      V_orc,r3                # Adjust outstanding request count
        ld      vd_qd(r10),r4           # Adjust queue depth
        subo    1,r3,r3
        subo    1,r4,r4
        st      r3,V_orc
        st      r4,vd_qd(r10)
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
#
# --- Link primary ILT onto end of return queue
#
        ldconst 0,r5
        ld      cm_upret_tail,r6        # r6 = last ILT on return queue
        st      r5,il_fthd(r11)         # clear ILT forward thread
        cmpobne.f 0,r6,.upcommon_300    # Jif queue tail member <>0
        st      r11,cm_upret_head       # save ILT as queue head member
        b       .upcommon_400

.upcommon_300:
        st      r11,il_fthd(r6)         # link ILT onto end of return queue
.upcommon_400:
        st      r11,cm_upret_tail       # save ILT as new queue tail member
        ldob    cm_upret_flag,r7        # r7 = return ILT loop flag
        cmpobe.f TRUE,r7,.upcommon_1000 # Jif already in return ILT loop
        ldconst TRUE,r7
        stob    r7,cm_upret_flag        # set return ILT loop flag to TRUE
        ldconst 0,r3                    # r3 = 0
.upcommon_500:
        ld      cm_upret_head,g1        # g1 = top return ILT on list
        cmpobe.f 0,g1,.upcommon_900     # Jif no more ILTs on return queue
        ld      il_fthd(g1),r6          # r6 = next ILT on return list
#
# --- Set up to return primary ILT/VRP back to originator
#
        ld      il_w0-ILTBIAS(g1),r4    # r4 = primary VRP
        ldob    il_w5(g1),r5            # r5 = composite status
        st      r6,cm_upret_head        # save new list head member
        cmpobne.f 0,r6,.upcommon_600    # Jif not the last member on list
        st      r6,cm_upret_tail        # clear tail member if last on list
                                        #  was just removed
.upcommon_600:
        stob    r5,vr_status(r4)        # Update primary VRP status
        st      r3,il_fthd(g1)          # clear forward thread field in ILT
        call    K$comp                  # Complete primary ILT
        b       .upcommon_500           # and check for more ILTs to return

.upcommon_900:
        ldconst FALSE,r7
        stob    r7,cm_upret_flag        # clear return ILT loop flag to FALSE
.upcommon_1000:
        movt    r12,g0                  # restore g0-g2
        ret
#
#**********************************************************************
#
#  NAME: cm$uperr_pcp_cr
#
#  PURPOSE:
#       Process an update error PCP completion event.
#
#  DESCRIPTION:
#       This routine processes an update error PCP completion event.
#       It adjusts the ILT nest level back to the normal update
#       handler level and calls the common update handler
#       routine to complete the update processing.
#
#  CALLING SEQUENCE:
#       call    cm$uperr_pcp_cr
#
#  INPUT:
#       g1 = ILT/PCP address at PCP nest level #1
#            (ILT nest level #2)
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       None.
#
#**********************************************************************
#
cm$uperr_pcp_cr:
        mov     g1,r15                  # save g1
        lda     -ILTBIAS(g1),g1         # g1 = ILT/VRP/SN at nest level #1
        call    cm$up_common            # complete the update processing
        mov     r15,g1                  # restore g1
        ret
#
#**********************************************************************
#
#  NAME: CM$srcerr
#
#  PURPOSE:
#       To provide a means of handling the error completion of a source
#       device VRP.
#
#  CALLING SEQUENCE:
#       call    CM$srcerr
#
#  INPUT:
#       g1 = ILT/RRP with error
#
#  OUTPUT:
#       g0 = count of ILT/PCP
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
CM$srcerr:
        ldconst 0,g0                    # clear count
        ld      il_w3(g1),r11           # r11 = primary ILT

        ld      il_w4(r11),r4           # r4 = assoc. VDD field in ILT/VRP
        cmpobe.f 0,r4,.srcerr_1000      # JIf no vdd defined

# --- Because the VDD address is now in VRP, the this logic is no
#     longer needed -lad
#
#        ldob    vi_cluster(r4),r7       # r7 = cluster #
#        ld      V_vdx_indx[r7*4],r8     # r8 = corresponding VDX address
#        cmpobe.f 0,r8,.srcerr_1000      # Jif cluster not supported
#        ldob    vi_vid(r4),r7           # r7 = virtual ID #        NOTE: vid as a byte!
#        ld      vx_indx(r8)[r7*8],r4    # r4 = corresponding VDD
#        cmpobe.f 0,r4,.srcerr_1000      # Jif no VDisk defined

        ld      vd_scdhead(r4),r14      # r14 = 1st possible scd
        cmpobe.f 0,r14,.srcerr_1000     # Jif no scd defined
#
# --- there is a SCD chain, setup to process chain
#
        ld      il_w0-ILTBIAS(r11),r13  # r13 = primary VRP
        cmpobe.f 0,r13,.srcerr_1000     # Jif no vrp defined

        mov     g1,r15                  # save r15
        mov     0,r3                    # r3 = 0
#
# --- run the SCD chain in search of local copies
#
.srcerr_100:
        ld      scd_cor(r14),r11        # r11 = assoc. COR address
        cmpobe.f 0,r11,.srcerr_900      # Jif no COR associated with SCD/DCD

        ldob    cor_crstate(r11),r8     # get cor cr state
        cmpobne.t corcrst_active,r8,.srcerr_900 # Jif copy not active

        ld      cor_cm(r11),r10         # r10 = assoc. CM address (if local
                                        #  copy operation)
                                        # 0 = remote copy operation
        cmpobe.f 0,r10,.srcerr_900      # Jif not local copy operation
#
# --- local copy found, set up a PCP to suspend copy
#
c       g1 = get_ilt();                 # Allocate an ILT (PCP)
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT

        ldl     vr_vsda(r13),r8         # r8/9 = update SDA address
        ld      vr_vlen(r13),r7         # r7 = update length in sectors

        lda     cm$srcerr_pcp_cr,r4     # r4 = completion address
        ld      cm_pcb(r10),r5          # r5 = task PCB

        st      r10,pcp1_cm(g1)         # save CM address
        stl     r8,pcp1_reg1(g1)        # pcp1_reg1/reg2 = update SDA
        st      r7,pcp1_reg3(g1)        # pcp1_reg3 = update length in sectors
        st      r4,pcp1_cr(g1)          # save completion routine address
        st      r5,pcp1_pcb(g1)         # save PCB
        st      r15,pcp1_reg4(g1)       # save ilt/rrp
        mov     0,r6                    # r6=0
        st      r6,pcp1_reg5(g1)        # clear out primary VRP for src error

        lda     ILTBIAS(g1),g1          # g1 = PCP level 2
        st      r3,pcp2_status(g1)      # clear out status,function
        st      r3,pcp2_handler(g1)     # clear Vsync handler
        ldconst pcp1fc_updateerr,r4     # set update error function code
        stob    r4,pcp2_function(g1)    # save function
        ldq     cor_rid(r11),r4         # r4-r7 = COR reg. info
        ldl     cor_rdsn(r11),r8        # r8-r9
        stq     r4,pcp2_rid(g1)         # save COR reg. info
        stl     r8,pcp2_rdsn(g1)
        st      r11,pcp2_cor(g1)        # save COR address
        call    CM$ctlrqstq             # enqueue the control request
        addo    1,g0,g0                 # bump pcp count
#
# --- get next SCD from chain
#
.srcerr_900:
        ld      scd_link(r14),r14       # get next scd
        cmpobne 0,r14,.srcerr_100       # continue

        stob    g0,vr_status(r13)       # set pcp count in vr status
        mov     r15,g1                  # restore g1

.srcerr_1000:
        ret
#
#**********************************************************************
#
#  NAME: cm$srcerr_pcp_cr
#
#  PURPOSE:
#       Process an update error PCP completion event related to an ILT/RRP
#       error reported for a source copy device of one or more copy
#       operations.
#
#  DESCRIPTION:
#       This routine gets the related ILT/RRP from the ILT/PCP
#       (saved in the pcp1_reg3 field of the ILT/PCP) and decrements
#       the outstanding ILT/PCP count saved in the rr_status field
#       of the ILT/RRP. It deallocates the ILT/PCP and if the outstanding
#       ILT/PCP count is zero, will call the original ILT completion
#       handler routine to complete processing of the ILT/RRP as if
#       it did not have an error (the error was saved in a field of the
#       primary ILT/VRP when the completion handler was first called).
#
#  CALLING SEQUENCE:
#       call    cm$srcerr_pcp_cr
#
#  INPUT:
#       g1 = ILT/PCP address at PCP nest level #1
#            (ILT nest level #1)
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       None.
#
#**********************************************************************
#
cm$srcerr_pcp_cr:
        mov     g1,r15                  # save g1
        ld      pcp1_reg4(g1),r14       # r14 = ILT/RRP of original error
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT (PCP)
        ld      il_w0(r14),r4           # r4 = RRP address
        ldob    rr_status(r4),r5        # r5 = outstanding ILT/PCP count
        subo    1,r5,r5                 # decrement count
        stob    r5,rr_status(r4)        # save updated count
        cmpobne.t 0,r5,.srcerrpcpcr_1000 # Jif not all ILT/PCPs completed
        mov     r14,g1                  # g1 = original ILT/RRP
        ld      il_cr(g1),r4            # r4 = original completion routine
        callx   (r4)                    # call original completion routine
.srcerrpcpcr_1000:
        mov     r15,g1                  # restore g1
        ret
#
#******************************************************************************
#
#  NAME: CM$chk_owner
#
#  PURPOSE:
#       Checks an inbound copy operation datagram for receipt by the
#       current copy operation owning controller. If we're not the
#       current owning controller, we return the appropriate reroute
#       response message to help the requestor get the datagram to the
#       correct controller for processing.
#
#  DESCRIPTION:
#       Performs the following logic:
#
#       Is the copy op. defined?
#         - YES. Am I the current copy op. owner?
#             - YES. Continue processing datagram.
#             - NO. Am I the master controller?
#                 - YES. Is current copy op. owner defined?
#                     - YES. Reroute to current copy op. owner.
#                     - NO. FINISH
#                 - NO. Reroute to current master controller.
#         - NO. Am I the master controller?
#             - YES. Return no copy found error.
#             - NO. Reroute to current master controller.
#
#  CALLING SEQUENCE:
#       call    CM$chk_owner
#
#  INPUT:
#       g1 = datagram ILT at nest level #4
#       g3 = local response message header address
#       g4 = request message buffer address (does NOT include header)
#
#  OUTPUT:
#       g0 = 0 if the datagram has been responded to
#       g0 = assoc. COR address if datagram to be processed further
#
#  REGS DESTROYED:
#       Reg. g0 destroyed.
#
#******************************************************************************
#
CM$chk_owner:
        mov     g1,r15                  # save g1
        mov     0,r14                   # r14 = returning g0 value (set to 0)
        mov     g3,r13                  # save g3
#
# --- Check if a copy operation matching the specified ID of the
#       inbound datagram is defined and if not return the appropriate
#       error to the requestor.
#
!       ld      CMsp_rq_rmsusp_rid(g4),g0 # g0 = specified copy reg. ID
!       ld      CMsp_rq_rmsusp_rcsn(g4),g1 # g1 = specified copy MAG serial #
        call    CM$find_cor_rid         # check if copy operation active with
                                        #  the specified ID
                                        # g0 = COR address if match found
        mov     r15,g1                  # restore g1
        cmpobne.t 0,g0,.chkowner_200    # Jif copy operation active with this
                                        #  ID
        ldos    K_ii+ii_status,r4       # Get initialization status
        bbs     iimaster,r4,.chkowner_100 # Jif I'm the current master
.chkowner_40:
        ld      DLM_master_sn,g0        # g0 = current group master controller
                                        #  serial number if known
        cmpobne 0,g0,.chkowner_50       # Jif group master controller serial
                                        #  number is known
        ld      K_ficb,r3               # FICB
        ld      fi_vcgid(r3),g0         # g0 = group serial number
.chkowner_50:
        mov     r13,g3                  # restore g3
        call    DLM$srvr_reroute        # return reroute datagram response
        b       .chkowner_1000          # and we're out of here!

.chkowner_100:
        call    CMsp$srvr_nocopy        # return error response message
                                        #  indicating copy not active
        b       .chkowner_1000          # and we're out of here!

.chkowner_200:
        mov     g0,g3                   # g3 = copy COR address
        ld      K_ficb,r3               # FICB
        ld      fi_cserial(r3),r4       # g0 = my controller serial number
        call    CCSM$CheckOwner         # check for current owner of copy
                                        # g0 = copy owner serial #
        cmpobe  0,g0,.chkowner_40       # Jif don't know/have an owner
        cmpobe  r4,g0,.chkowner_500     # Jif I'm the current owner of the
                                        #  copy
        b       .chkowner_50            # reroute datagram to copy owner

.chkowner_500:
        mov     g3,r14                  # r14 = assoc. COR address to return
                                        #  to caller
.chkowner_1000:
        movl    r14,g0                  # restore g0-g1
        mov     r13,g3                  # restore g3
        ret
#
#**********************************************************************
#
#  NAME: cm$FlushCache
#
#  PURPOSE:
#       Provide a means of flushing the cache of devices associated with
#       a copy.
#
#  CALLING SEQUENCE:
#       process call
#
#  INPUT:
#       g2 = ILT at lvl 1
#
#  OUTPUT:
#       None.
#
#  REGS. DESTROYED:
#       None.
#
#**********************************************************************
#
cm$FlushCache:
        ld      il_w0(g2),g3            # g3 = possible cor
        cmpobe  0,g3,.flshChch_900      # no cor defined  - just release ilt

        ldos    il_w2(g2),r6            # r6 = source VID
        ldos    il_w3(g2),r7            # r7 = destination VID
        ldconst 0xffff,r4               # r4 = null VID
#
# --- temporarily disable the FE Write Cache of the source
#
        cmpobe  r4,r6,.flshChch_200     # Jif NULL
        ld      V_vddindx[r6*4],r3      # r3 = corresponding VDD
        cmpobe  0,r3,.flshChch_200      # Jif NULL

        PushRegs(r3)                    # Save all G regs for "C" call
        mov     r6,g0                   # g0 = vid
        ldconst WC_SET_T_DISABLE,g1     # g1 = Function to Temp Disable WC
        call    WC_VDiskDisable         # Go Flush Write Cache and wait
        cmpobe  0,g0,.flshChch_110
#
# --- there seems to be some issue flushing the cache. At the moment, just issue a
#     message indicating so....
#
c fprintf(stderr,"%s%s:%u ERROR - cm$FlushCache(src) returned from disabling VDisk Cache - VID = %x RC = %x\n", FEBEMESSAGE, __FILE__, __LINE__, (UINT32)r6, (UINT32)g0);
        b       .flshChch_120
#
# --- cache was flushed
#
.flshChch_110:
c fprintf(stderr,"%s%s:%u cm$FlushCache(src) returned from disabling VDisk Cache - VID = %x\n", FEBEMESSAGE, __FILE__, __LINE__, (UINT32)r6);

.flshChch_120:
        PopRegsVoid(r3)                 # Restore all G regs
#
# --- temporarily disable the FE Write Cache of the destination.
#
.flshChch_200:
        ld      il_w0(g2),g3            # g3 = possible cor
        cmpobe  0,g3,.flshChch_400      # if NULL - cor has been remove
                                        #           turn back on cacheing

        cmpobe  r4,r7,.flshChch_300     # Jif NULL
        ld      V_vddindx[r7*4],r3      # r3 = corresponding VDD
        cmpobe  0,r3,.flshChch_300      # Jif NULL

        PushRegs(r3)                    # Save all G regs for "C" call
        mov     r7,g0                   # g0 = vid
        ldconst WC_SET_T_DISABLE,g1     # g1 = Function to Temp Disable WC
        call    WC_VDiskDisable         # Go Flush Write Cache and wait
        cmpobe  0,g0,.flshChch_210
#
# --- there seems to be some issue flushing the cache. At the moment, just issue a
#     message indicating so....
#
c fprintf(stderr,"%s%s:%u ERROR - cm$FlushCache(dest) returned from disabling VDisk Cache - VID = %x RC = %x\n", FEBEMESSAGE, __FILE__, __LINE__, (UINT32)r7, (UINT32)g0);
        b       .flshChch_220
#
# --- cache was flushed
#
.flshChch_210:
c fprintf(stderr,"%s%s:%u cm$FlushCache(dest) returned from disabling VDisk Cache - VID = %x\n", FEBEMESSAGE, __FILE__, __LINE__, (UINT32)r7);

.flshChch_220:
        PopRegsVoid(r3)                 # Restore all G regs
#
# --- determine if the copy is still active, if so, clear the pcb in the ilt
#     and end task
#
.flshChch_300:
        ld      il_w0(g2),g3            # g3 = possible cor
        cmpobe  0,g3,.flshChch_400      # no - turn back on cacheing

        ldconst 0,r3                    # r3 = zero
        st      r3,il_pcb(g2)           # clear the pcb in ilt
        b       .flshChch_1000          # end task
#
# --- otherwise, turn the caching back on for the devices
#     Clear the temporarily disable of the FE Write Cache of source
#
.flshChch_400:
        cmpobe  r4,r6,.flshChch_420     # Jif NULL
        ld      V_vddindx[r6*4],r3      # r3 = corresponding VDD
        cmpobe  0,r3,.flshChch_420      # jif null

        PushRegs(r3)                    # Save all G regs for "C" call
        mov     r6,g0                   # g0 = load VID
        ldconst WC_CLEAR_T_DISABLE,g1   # g1 = Function to Temp Disable WC
        call    WC_VDiskDisable         # Go Clear the T Disable flag
c fprintf(stderr,"%s%s:%u cm$FlushCache(src) returned from Enabling VDisk Cache - VDD = %x\n", FEBEMESSAGE, __FILE__, __LINE__,(UINT32)r6);
        PopRegsVoid(r3)                 # Restore all G regs

#
#     Clear the temporarily disable of the FE Write Cache of destination
#
.flshChch_420:
        cmpobe  r4,r7,.flshChch_900     # Jif NULL
        ld      V_vddindx[r7*4],r3      # r3 = corresponding VDD
        cmpobe  0,r3,.flshChch_900      # jif null

        PushRegs(r3)                    # Save all G regs for "C" call
        mov     r7,g0                   # g0 = load VID
        ldconst WC_CLEAR_T_DISABLE,g1   # g1 = Function to Temp Disable WC
        call    WC_VDiskDisable         # Go Clear the T Disable flag
c fprintf(stderr,"%s%s:%u cm$FlushCache(dest) returned from Enabling VDisk Cache - VDD = %x\n", FEBEMESSAGE, __FILE__, __LINE__,(UINT32)r7);
        PopRegsVoid(r3)                 # Restore all G regs

.flshChch_900:
        mov     g2,g1                   # restore ilt address
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT

.flshChch_1000:
        ret
#
#**********************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
