# $Id: dlm.as 159861 2012-09-19 20:00:49Z marshall_midden $
#**********************************************************************
#
#  NAME: dlm.as
#
#  PURPOSE:
#       To provide support for the Data-link Manager logic which
#       supports XIOtech Controller-to-XIOtech Controller functions
#       and services that are common to both the Front-End and Back-End
#       processors.
#
#  FUNCTIONS:
#       DLM$vque       - Queue Data-link Manager VRP requests
#       DLM$get_dg     - Get datagram resources
#       DLM$put_dg     - Put datagram resources
#       DLM$send_dg    - Send datagram message
#       DLM$just_senddg - Send datagram message w/retry and deallocate
#                        resources when done.
#       DLM$find_controller - Find a XIOtech Controller based on the controllers
#                               serial number
#
#       This module employs 2 process:
#
#       dlm$vrpx       - VRP executive (1 copy)
#       dlm$retrydg    - Retry datagram message request (1 copy)
#
#  Copyright (c) 1999-2010 XIOtech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- assembly options ------------------------------------------------
#
        .set    DEBUG_DLM1,FALSE
#
# --- local equates ---------------------------------------------------
#
        .set    retrydg_to,2000         # retry datagram request task
                                        #  timeout period in msec.
#
.if     MAG2MAG
#
# --- global function declarations ------------------------------------
#
        .globl  DLM$vque                # Queuing routine
        .globl  DLM$get_dg              # Get datagram resources
        .globl  DLM$put_dg              # Put datagram resources
        .globl  DLM$send_dg             # Send datagram message
.ifdef BACKEND
        .globl  DLM$just_senddg         # Send datagram message w/retry and
.endif  # BACKEND
                                        #  deallocate resources when done
.endif  # MAG2MAG
        .globl  DLM$find_controller     # Find a controller based on its
        .globl  DLM_FindController      #  serial number
.ifdef BACKEND
        .globl  DLM$pk_master           # Pack a Group Master Controller
                                        #  Definition datagram
        .globl  DLM$def_master          # Notify all XIOtech controllers of the
                                        #  group master
        .globl  DLM$srvr_reroute        # Re-route datagram to specified
                                        #  controller response routine
.endif # .ifdef BACKEND
        .globl  DLM$srvr_invfc          # Pack and return destination server/invalid
                                        #  function code
        .globl  DLM$srvr_invparm        # Pack and return destination server/invalid
                                        #  parameter response

#
# --- global usage data definitions -----------------------------------
#
        .globl  dlm_mlmthd              # Magnitude Link Mgmt table Head
        .globl  dlm_lldmt_dir           # DTMT link list head

        .globl  DLM_sdsp_nopath
        .globl  DLM_ddsp_nosrvr
        .globl  DLM_srvr_ok
        .globl  DLM_srvr_invfc
        .globl  DLM_srvr_invparm
        .globl  DLM_srvr_reroute
        .globl  DLM_srvr_reroute
.ifdef BACKEND
        .globl  DLM_srvr_vlconflt
.endif  # BACKEND

#
# --- DLM resident data definitions
#
        .data
        .align 3
#
# --- Unknown VDisk or Controller Name Constant
#
dlm_unknown:
        .ascii  "Unknown "
#
#
# --- VRP queue definitions -------------------------------------- *****
#
DLM_vrp_qu:
        .word   0                       # queue head pointer        <w>
        .word   0                       # queue tail pointer        <w>
        .word   0                       # queue count               <w>
        .word   0                       # associated pcb            <w>
#
# --- Datagram retry queue data structure
#
        .globl  dlm_rtydg_qu
dlm_rtydg_qu:
        .word   0                       # queue head pointer        <w>
        .word   0                       # queue tail pointer        <w>
#
.if DLMFE_DRIVER
dlm_fedriver_qht:
        .space  8,0                     # Queue head/tail
dlm_fedriver_pcb:
        .word   0                       # Driver Executive PCB
dlm_fedriver_cqd:
        .word   0                       # Driver current queue depth
DLMT_1st_rand_seed:
        .word   0                       # Save the first random seed
DLMT_rand_seed:
        .word   0                       # Random number seed
DLMT_drp_in:
        .word   0                       # DRP Counter coming in
DLMT_drp_out:
        .word   0                       # DRP Counter going out
DLMT_nerr:
        .word   0                       # Next Error count to force an error on
#
        .set    MAX_DRP_ERR,0xF         # Maximum DRP count to force error on

.endif  # DLMFE_DRIVER
#                                                                  *****
#
# --- LLDMT Directory
#
dlm_lldmt_dir:                          # LLDMT directory
dlm_op_dir:                             # link-level driver operational
                                        #  ILT/VRP directory
        .space (MAXISP+MAXICL)*4,0
#
dlm_mlmthd:
        .word   0                       # MLMT list head member
dlm_mlmttl:
        .word   0                       # MLMT list tail member
#
dtmt_banner:
        .ascii  "DTMT"                  # DTMT banner pattern
#
.if     DEBUG_DLM1
#
debug_dlm1:
        .word   0
#
.endif  # DEBUG_DLM1
#
# --- CCB packet to notify of a path established/lost.
#
# <s> Type of message
#   mlenewpath = 0x0035 (new path)
#   mlelostpath = 0x0212 (lost path)
#   mlelostallpaths = 0x0434 (lost all)
# <2> Reserved
# <w> Length of data following
# <w> Other XIOtech serial number
        .set    log_dlm_ccb_path_sn,mle_event+8
# ...
        .set    log_dlm_ccb_path_length,8   # Length of remaining data for new
                                            # path and lost all path messages
        .set    log_dlm_ccb_lost_path_length,12 # Length of remaining data for
                                                # lost path message
# <b> Other XIOtech cluster number
        .set    log_dlm_ccb_path_cl,log_dlm_ccb_path_sn+4
# <b> Other XIOtech Path communications on
        .set    log_dlm_ccb_path_other_path,log_dlm_ccb_path_cl+1
# <b> This XIOtech Path communications on
        .set    log_dlm_ccb_path_this_path,log_dlm_ccb_path_other_path+1
# <b> This indicates whether this is ICL path or not.
        .set    log_dlm_ccb_path_icl_flag,log_dlm_ccb_path_this_path+1
        .set    log_dlm_ccb_path_size,16    # Size = 16 bytes (new or lost all)
# <b> Datagram Status
        .set    log_dlm_ccb_path_dg_status,log_dlm_ccb_path_icl_flag+1
# <b> Datagram Error Code 1
        .set    log_dlm_ccb_path_dg_ec1,log_dlm_ccb_path_dg_status+1
# <b> Datagram Error Code 2
        .set    log_dlm_ccb_path_dg_ec2,log_dlm_ccb_path_dg_ec1+1
# <1> Reserved
# Size = 20 bytes (lost path)
        .set    log_dlm_ccb_lost_path_size,log_dlm_ccb_path_dg_ec2+2

        .align  2
.if MAG2MAG
#
#**********************************************************************
#
# _____________________ Datagram Request Templates ____________________
#
.ifdef BACKEND
#
# --- Get MAGNITUDE Device Database request header template
#
dlm_getmdd_hdr:
        .byte   dg_cpu_main             # server processor code         <b>
        .byte   dgrq_size               # request header length         <b>
        .short  0                       # message sequence #            <s>
        .byte   DLM0_fc_magdd           # request function code         <b>
        .byte   dg_path_any             # path #                        <b>
        .byte   0                       # general purpose reg. #0       <b>
                                        # requested cluster #
        .byte   0                       # general purpose reg. #1       <b>
#
        .word   0                       # source serial #               <w>
        .word   0                       # destination serial #          <w>
#                                                               ******0x10****
        .ascii  "DLM0"                  # dest. server name             <w>
        .word   0                       # remaining message length      <w>
#
        .byte   0                       # general purpose reg. #2       <b>
        .byte   0                       # general purpose reg. #3       <b>
        .byte   0                       # general purpose reg. #4       <b>
        .byte   0                       # general purpose reg. #5       <b>
#
        .word   0                       # request message header CRC    <w>
#                                                               ******0x20****
dlm_getmdd_hdr_GT2TB:
        .byte   dg_cpu_main             # server processor code         <b>
        .byte   dgrq_size               # request header length         <b>
        .short  0                       # message sequence #            <s>
        .byte   DLM0_fc_magdd_GT2TB     # request function code         <b>
        .byte   dg_path_any             # path #                        <b>
        .byte   0                       # general purpose reg. #0       <b>
                                        # requested cluster #
        .byte   0                       # general purpose reg. #1       <b>
#
        .word   0                       # source serial #               <w>
        .word   0                       # destination serial #          <w>
#                                                               ******0x10****
        .ascii  "DLM0"                  # dest. server name             <w>
        .word   0                       # remaining message length      <w>
#
        .byte   0                       # general purpose reg. #2       <b>
        .byte   0                       # general purpose reg. #3       <b>
        .byte   0                       # general purpose reg. #4       <b>
        .byte   0                       # general purpose reg. #5       <b>
#
        .word   0                       # request message header CRC    <w>
#                                                               ******0x20****
#
# --- Establish VLink request header template
#
dlm_estvl_hdr:
        .byte   dg_cpu_main             # server processor code         <b>
        .byte   dgrq_size               # request header length         <b>
        .short  0                       # message sequence #            <s>
        .byte   DLM0_fc_estvl           # request function code         <b>
        .byte   dg_path_any             # path #                        <b>
        .byte   0                       # general purpose reg. #0       <b>
                                        # requested cluster #
        .byte   0                       # general purpose reg. #1       <b>
#
        .word   0                       # source serial #               <w>
        .word   0                       # destination serial #          <w>
#                                                               ******0x10****
        .ascii  "DLM0"                  # dest. server name             <w>
        .word   DLM0_rq_estvl_size      # remaining message length      <w>
#
        .byte   0                       # general purpose reg. #2       <b>
        .byte   0                       # general purpose reg. #3       <b>
        .byte   0                       # general purpose reg. #4       <b>
        .byte   0                       # general purpose reg. #5       <b>
#
        .word   0                       # request message header CRC    <w>
#                                                               ******0x20****
dlm_estvl_hdr_GT2TB:
        .byte   dg_cpu_main             # server processor code         <b>
        .byte   dgrq_size               # request header length         <b>
        .short  0                       # message sequence #            <s>
        .byte   DLM0_fc_estvl_GT2TB     # request function code         <b>
        .byte   dg_path_any             # path #                        <b>
        .byte   0                       # general purpose reg. #0       <b>
                                        # requested cluster #
        .byte   0                       # general purpose reg. #1       <b>
#
        .word   0                       # source serial #               <w>
        .word   0                       # destination serial #          <w>
#                                                               ******0x10****
        .ascii  "DLM0"                  # dest. server name             <w>
        .word   DLM0_rq_estvl_size      # remaining message length      <w>
#
        .byte   0                       # general purpose reg. #2       <b>
        .byte   0                       # general purpose reg. #3       <b>
        .byte   0                       # general purpose reg. #4       <b>
        .byte   0                       # general purpose reg. #5       <b>
#
        .word   0                       # request message header CRC    <w>
#                                                               ******0x20****
#
# --- Terminate VLink request header template
#
dlm_trmvl_hdr:
        .byte   dg_cpu_main             # server processor code         <b>
        .byte   dgrq_size               # request header length         <b>
        .short  0                       # message sequence #            <s>
        .byte   DLM0_fc_trmvl           # request function code         <b>
        .byte   dg_path_any             # path #                        <b>
        .byte   0                       # general purpose reg. #0       <b>
                                        # requested cluster #
        .byte   0                       # general purpose reg. #1       <b>
#
        .word   0                       # source serial #               <w>
        .word   0                       # destination serial #          <w>
#                                                               ******0x10****
        .ascii  "DLM0"                  # dest. server name             <w>
        .word   DLM0_rq_trmvl_size      # remaining message length      <w>
#
        .byte   0                       # general purpose reg. #2       <b>
        .byte   0                       # general purpose reg. #3       <b>
        .byte   0                       # general purpose reg. #4       <b>
        .byte   0                       # general purpose reg. #5       <b>
#
        .word   0                       # request message header CRC    <w>
#                                                               ******0x20****
#
# --- VDisk/VLink Name Changed request header template
#
dlm_vdchg_hdr:
        .byte   dg_cpu_main             # server processor code         <b>
        .byte   dgrq_size               # request header length         <b>
        .short  0                       # message sequence #            <s>
        .byte   DLM0_fc_vdname          # request function code         <b>
        .byte   dg_path_any             # path #                        <b>
        .byte   0                       # general purpose reg. #0       <b>
                                        # requested cluster #
        .byte   0                       # general purpose reg. #1       <b>
#
        .word   0                       # source serial #               <w>
        .word   0                       # destination serial #          <w>
#                                                               ******0x10****
        .ascii  "DLM0"                  # dest. server name             <w>
        .word   DLM0_rq_vdchg_size      # remaining message length      <w>
#
        .byte   0                       # general purpose reg. #2       <b>
        .byte   0                       # general purpose reg. #3       <b>
        .byte   0                       # general purpose reg. #4       <b>
        .byte   0                       # general purpose reg. #5       <b>
#
        .word   0                       # request message header CRC    <w>
#                                                               ******0x20****
#
# --- XIOtech Controller Node Name Changed request header template
#
dlm_nnchg_hdr:
        .byte   dg_cpu_main             # server processor code         <b>
        .byte   dgrq_size               # request header length         <b>
        .short  0                       # message sequence #            <s>
        .byte   DLM0_fc_magname         # request function code         <b>
        .byte   dg_path_any             # path #                        <b>
        .byte   0                       # general purpose reg. #0       <b>
                                        # requested cluster #
        .byte   0                       # general purpose reg. #1       <b>
#
        .word   0                       # source serial #               <w>
        .word   0                       # destination serial #          <w>
#                                                               ******0x10****
        .ascii  "DLM0"                  # dest. server name             <w>
        .word   DLM0_rq_nnchg_size      # remaining message length      <w>
#
        .byte   0                       # general purpose reg. #2       <b>
        .byte   0                       # general purpose reg. #3       <b>
        .byte   0                       # general purpose reg. #4       <b>
        .byte   0                       # general purpose reg. #5       <b>
#
        .word   0                       # request message header CRC    <w>
#                                                               ******0x20****
#
# --- Swap VLink Lock request header template
#
dlm_swpvl_hdr:
        .byte   dg_cpu_main             # server processor code         <b>
        .byte   dgrq_size               # request header length         <b>
        .short  0                       # message sequence #            <s>
        .byte   DLM0_fc_swpvl           # request function code         <b>
        .byte   dg_path_any             # path #                        <b>
        .byte   0                       # general purpose reg. #0       <b>
                                        # requested cluster #
        .byte   0                       # general purpose reg. #1       <b>
#
        .word   0                       # source serial #               <w>
        .word   0                       # destination serial #          <w>
#                                                               ******0x10****
        .ascii  "DLM0"                  # dest. server name             <w>
        .word   DLM0_rq_swpvl_size      # remaining message length      <w>
#
        .byte   0                       # general purpose reg. #2       <b>
        .byte   0                       # general purpose reg. #3       <b>
        .byte   0                       # general purpose reg. #4       <b>
        .byte   0                       # general purpose reg. #5       <b>
#
        .word   0                       # request message header CRC    <w>
#                                                               ******0x20****
dlm_swpvl_hdr_GT2TB:
        .byte   dg_cpu_main             # server processor code         <b>
        .byte   dgrq_size               # request header length         <b>
        .short  0                       # message sequence #            <s>
        .byte   DLM0_fc_swpvl_GT2TB     # request function code         <b>
        .byte   dg_path_any             # path #                        <b>
        .byte   0                       # general purpose reg. #0       <b>
                                        # requested cluster #
        .byte   0                       # general purpose reg. #1       <b>
#
        .word   0                       # source serial #               <w>
        .word   0                       # destination serial #          <w>
#                                                               ******0x10****
        .ascii  "DLM0"                  # dest. server name             <w>
        .word   DLM0_rq_swpvl_size      # remaining message length     <w>
#
        .byte   0                       # general purpose reg. #2       <b>
        .byte   0                       # general purpose reg. #3       <b>
        .byte   0                       # general purpose reg. #4       <b>
        .byte   0                       # general purpose reg. #5       <b>
#
        .word   0                       # request message header CRC    <w>
#                                                               ******0x20****
#
#
# --- VDisk/VLink Query request header template
#
dlm_vqury_hdr:
        .byte   dg_cpu_main             # server processor code         <b>
        .byte   dgrq_size               # request header length         <b>
        .short  0                       # message sequence #            <s>
        .byte   DLM0_fc_vquery          # request function code         <b>
        .byte   dg_path_any             # path #                        <b>
        .byte   0                       # general purpose reg. #0       <b>
                                        # requested cluster #
        .byte   0                       # general purpose reg. #1       <b>
#
        .word   0                       # source serial #               <w>
        .word   0                       # destination serial #          <w>
#                                                               ******0x10****
        .ascii  "DLM0"                  # dest. server name             <w>
        .word   DLM0_rq_vqury_size      # remaining message length      <w>
#
        .byte   0                       # general purpose reg. #2       <b>
        .byte   0                       # general purpose reg. #3       <b>
        .byte   0                       # general purpose reg. #4       <b>
        .byte   0                       # general purpose reg. #5       <b>
#
        .word   0                       # request message header CRC    <w>
#                                                               ******0x20****
dlm_vqury_hdr_GT2TB:
        .byte   dg_cpu_main             # server processor code         <b>
        .byte   dgrq_size               # request header length         <b>
        .short  0                       # message sequence #            <s>
        .byte   DLM0_fc_vquery_GT2TB    # request function code         <b>
        .byte   dg_path_any             # path #                        <b>
        .byte   0                       # general purpose reg. #0       <b>
                                        # requested cluster #
        .byte   0                       # general purpose reg. #1       <b>
#
        .word   0                       # source serial #               <w>
        .word   0                       # destination serial #          <w>
#                                                               ******0x10****
        .ascii  "DLM0"                  # dest. server name             <w>
        .word   DLM0_rq_vqury_size      # remaining message length      <w>
#
        .byte   0                       # general purpose reg. #2       <b>
        .byte   0                       # general purpose reg. #3       <b>
        .byte   0                       # general purpose reg. #4       <b>
        .byte   0                       # general purpose reg. #5       <b>
#
        .word   0                       # request message header CRC    <w>
#                                                               ******0x20****
#
# --- Change VDisk Size request header template
#
dlm_chgsz_hdr:
        .byte   dg_cpu_main             # server processor code         <b>
        .byte   dgrq_size               # request header length         <b>
        .short  0                       # message sequence #            <s>
        .byte   DLM0_fc_chgsiz          # request function code         <b>
        .byte   dg_path_any             # path #                        <b>
        .byte   0                       # general purpose reg. #0       <b>
                                        # requested cluster #
        .byte   0                       # general purpose reg. #1       <b>
#
        .word   0                       # source serial #               <w>
        .word   0                       # destination serial #          <w>
#                                                               ******0x10****
        .ascii  "DLM0"                  # dest. server name             <w>
        .word   DLM0_rq_chgsz_size      # remaining message length      <w>
#
        .byte   0                       # general purpose reg. #2       <b>
        .byte   0                       # general purpose reg. #3       <b>
        .byte   0                       # general purpose reg. #4       <b>
        .byte   0                       # general purpose reg. #5       <b>
#
        .word   0                       # request message header CRC    <w>
#                                                               ******0x20****
dlm_chgsz_hdr_GT2TB:
        .byte   dg_cpu_main             # server processor code         <b>
        .byte   dgrq_size               # request header length         <b>
        .short  0                       # message sequence #            <s>
        .byte   DLM0_fc_chgsiz_GT2TB    # request function code         <b>
        .byte   dg_path_any             # path #                        <b>
        .byte   0                       # general purpose reg. #0       <b>
                                        # requested cluster #
        .byte   0                       # general purpose reg. #1       <b>
#
        .word   0                       # source serial #               <w>
        .word   0                       # destination serial #          <w>
#                                                               ******0x10****
        .ascii  "DLM0"                  # dest. server name             <w>
        .word   DLM0_rq_chgsz_size      # remaining message length      <w>
#
        .byte   0                       # general purpose reg. #2       <b>
        .byte   0                       # general purpose reg. #3       <b>
        .byte   0                       # general purpose reg. #4       <b>
        .byte   0                       # general purpose reg. #5       <b>
#
        .word   0                       # request message header CRC    <w>
#                                                               ******0x20****
#
# --- VLink Poll request header template
#
dlm_vlpol_hdr:
        .byte   dg_cpu_main             # server processor code         <b>
        .byte   dgrq_size               # request header length         <b>
        .short  0                       # message sequence #            <s>
        .byte   DLM0_fc_vlpoll          # request function code         <b>
        .byte   dg_path_any             # path #                        <b>
        .byte   0                       # general purpose reg. #0       <b>
                                        # requested cluster #
        .byte   0                       # general purpose reg. #1       <b>
#
        .word   0                       # source serial #               <w>
        .word   0                       # destination serial #          <w>
#                                                               ******0x10****
        .ascii  "DLM0"                  # dest. server name             <w>
        .word   DLM0_rq_vlpol_size      # remaining message length      <w>
#
        .byte   0                       # general purpose reg. #2       <b>
        .byte   0                       # general purpose reg. #3       <b>
        .byte   0                       # general purpose reg. #4       <b>
        .byte   0                       # general purpose reg. #5       <b>
#
        .word   0                       # request message header CRC    <w>
#                                                               ******0x20****
#
# --- VDisk Size Changed request header template
#
dlm_vdsize_hdr:
        .byte   dg_cpu_main             # server processor code         <b>
        .byte   dgrq_size               # request header length         <b>
        .short  0                       # message sequence #            <s>
        .byte   DLM0_fc_vdsize          # request function code         <b>
        .byte   dg_path_any             # path #                        <b>
        .byte   0                       # general purpose reg. #0       <b>
                                        # requested cluster #
        .byte   0                       # general purpose reg. #1       <b>
#
        .word   0                       # source serial #               <w>
        .word   0                       # destination serial #          <w>
#                                                               ******0x10****
        .ascii  "DLM0"                  # dest. server name             <w>
        .word   DLM0_rq_vdsize_size     # remaining message length      <w>
#
        .byte   0                       # general purpose reg. #2       <b>
        .byte   0                       # general purpose reg. #3       <b>
        .byte   0                       # general purpose reg. #4       <b>
        .byte   0                       # general purpose reg. #5       <b>
#
        .word   0                       # request message header CRC    <w>
#                                                               ******0x20****
dlm_vdsize_hdr_GT2TB:
        .byte   dg_cpu_main             # server processor code         <b>
        .byte   dgrq_size               # request header length         <b>
        .short  0                       # message sequence #            <s>
        .byte   DLM0_fc_vdsize_GT2TB    # request function code         <b>
        .byte   dg_path_any             # path #                        <b>
        .byte   0                       # general purpose reg. #0       <b>
                                        # requested cluster #
        .byte   0                       # general purpose reg. #1       <b>
#
        .word   0                       # source serial #               <w>
        .word   0                       # destination serial #          <w>
#                                                               ******0x10****
        .ascii  "DLM0"                  # dest. server name             <w>
        .word   DLM0_rq_vdsize_size_GT2TB # remaining message length    <w>
#
        .byte   0                       # general purpose reg. #2       <b>
        .byte   0                       # general purpose reg. #3       <b>
        .byte   0                       # general purpose reg. #4       <b>
        .byte   0                       # general purpose reg. #5       <b>
#
        .word   0                       # request message header CRC    <w>
#                                                               ******0x20****
#
# --- Group master controller definition request header template
#
dlm_master_hdr:
        .byte   dg_cpu_main             # server processor code         <b>
        .byte   dgrq_size               # request header length         <b>
        .short  0                       # message sequence #            <s>
        .byte   DLM0_fc_master          # request function code         <b>
        .byte   dg_path_any             # path #                        <b>
        .byte   0                       # general purpose reg. #0       <b>
                                        # requested cluster #
        .byte   0                       # general purpose reg. #1       <b>
#
        .word   0                       # source serial #               <w>
        .word   0                       # destination serial #          <w>
#                                                               ******0x10****
        .ascii  "DLM0"                  # dest. server name             <w>
        .word   DLM0_rq_master_size     # remaining message length      <w>
#
        .byte   0                       # general purpose reg. #2       <b>
        .byte   0                       # general purpose reg. #3       <b>
        .byte   0                       # general purpose reg. #4       <b>
        .byte   0                       # general purpose reg. #5       <b>
#
        .word   0                       # request message header CRC    <w>
#                                                               ******0x20****
#
# --- Async Replication nvram packet request header template
#
dlm_async_nva_hdr:
        .byte   dg_cpu_main             # server processor code         <b>
        .byte   dgrq_size               # request header length         <b>
        .short  0                       # message sequence #            <s>
        .byte   DLM0_fc_async_nva       # request function code         <b>
        .byte   dg_path_any             # path #                        <b>
        .byte   0                       # general purpose reg. #0       <b>
                                        # requested cluster #
        .byte   0                       # general purpose reg. #1       <b>
#
        .word   0                       # source serial #               <w>
        .word   0                       # destination serial #          <w>
#                                                               ******0x10****
        .ascii  "DLM0"                  # dest. server name             <w>
        .word   DLM0_rq_apool_nv_size   # remaining message length      <w>
#
        .byte   0                       # general purpose reg. #2       <b>
        .byte   0                       # general purpose reg. #3       <b>
        .byte   0                       # general purpose reg. #4       <b>
        .byte   0                       # general purpose reg. #5       <b>
#
        .word   0                       # request message header CRC    <w>
#                                                               ******0x20****


.else   # FRONTEND
#
# --- Poll Datagram Path request header template
#
dlm_polldgp_hdr:
        .byte   dg_cpu_interface        # server processor code         <b>
        .byte   dgrq_size               # request header length         <b>
        .short  0                       # message sequence #            <s>
        .byte   DLM1_fc_polldgp         # request function code         <b>
        .byte   0                       # path #                        <b>
        .byte   0                       # general purpose reg. #0       <b>
        .byte   0                       # general purpose reg. #1       <b>
#
        .word   0                       # source serial #               <w>
        .word   0                       # destination serial #          <w>
#                                                               ******0x10****
        .ascii  "DLM1"                  # dest. server name             <w>
        .word   0                       # remaining message length      <w>
#
        .byte   0                       # general purpose reg. #2       <b>
        .byte   0                       # general purpose reg. #3       <b>
        .byte   0                       # general purpose reg. #4       <b>
        .byte   0                       # general purpose reg. #5       <b>
#
        .word   0                       # request message header CRC    <w>
#                                                               ******0x20****
.endif  # ifdef BACKEND
#
#**********************************************************************
#
# _____________________ Datagram Response Templates ___________________
#
# --- Source DSP/No path to server response template
#
dlm$sdsp_nopath:
DLM_sdsp_nopath:
        .byte   dg_st_sdsp              # request completion status
        .byte   dgrs_size               # response header length
        .short  0                       # message sequence #
        .byte   dgec1_sdsp_nopath       # error code #1
        .byte   0                       # error code #2
        .byte   0                       # general purpose reg. #0
        .byte   0                       # general purpose reg. #1
        .word   0                       # remaining message length
        .word   0                       # response message header CRC
#
# --- Destination DSP/Requested server not defined response template
#
dlm$ddsp_nosrvr:
DLM_ddsp_nosrvr:
        .byte   dg_st_ddsp              # request completion status
        .byte   dgrs_size               # response header length
        .short  0                       # message sequence #
        .byte   dgec1_ddsp_nosrvr       # error code #1
        .byte   0                       # error code #2
        .byte   0                       # general purpose reg. #0
        .byte   0                       # general purpose reg. #1
        .word   0                       # remaining message length
        .word   0                       # response message header CRC
#
# --- Destination server/successful response template
#
dlm$srvr_ok:
DLM_srvr_ok:
        .byte   dg_st_ok                # request completion status
        .byte   dgrs_size               # response header length
        .short  0                       # message sequence #
        .byte   0                       # error code #1
        .byte   0                       # error code #2
        .byte   0                       # general purpose reg. #0
        .byte   0                       # general purpose reg. #1
        .word   0                       # remaining message length
        .word   0                       # response message header CRC
#
# --- Destination server/Invalid request function code response template
#
dlm$srvr_invfc:
DLM_srvr_invfc:
        .byte   dg_st_srvr              # request completion status
        .byte   dgrs_size               # response header length
        .short  0                       # message sequence #
        .byte   dgec1_srvr_invfc        # error code #1
        .byte   0                       # error code #2
        .byte   0                       # general purpose reg. #0
        .byte   0                       # general purpose reg. #1
        .word   0                       # remaining message length
        .word   0                       # response message header CRC
#
# --- Destination server/Invalid request parameter response template
#
dlm$srvr_invparm:
DLM_srvr_invparm:
        .byte   dg_st_srvr              # request completion status
        .byte   dgrs_size               # response header length
        .short  0                       # message sequence #
        .byte   dgec1_srvr_invparm      # error code #1
        .byte   0                       # error code #2
        .byte   0                       # general purpose reg. #0
        .byte   0                       # general purpose reg. #1
        .word   0                       # remaining message length
        .word   0                       # response message header CRC
#
.ifdef BACKEND
#
# --- Destination server/VLink access conflict parameter response template
#
dlm$srvr_vlconflt:
DLM_srvr_vlconflt:
        .byte   dg_st_srvr              # request completion status
        .byte   dgrs_size               # response header length
        .short  0                       # message sequence #
        .byte   dgec1_srvr_vlconflt     # error code #1
        .byte   0                       # error code #2
        .byte   0                       # general purpose reg. #0
        .byte   0                       # general purpose reg. #1
        .word   0                       # remaining message length
        .word   0                       # response message header CRC
.else   # FRONTEND
.endif  # ifdef BACKEND
#
# --- Destination server/server access conflict parameter response template
#
dlm$srvr_srconflt:
DLM_srvr_srconflt:
        .byte   dg_st_srvr              # request completion status
        .byte   dgrs_size               # response header length
        .short  0                       # message sequence #
        .byte   dgec1_srvr_srconflt     # error code #1
        .byte   0                       # error code #2
        .byte   0                       # general purpose reg. #0
        .byte   0                       # general purpose reg. #1
        .word   0                       # remaining message length
        .word   0                       # response message header CRC
#
# --- Re-route datagram to specified controller response template
#
dlm$srvr_reroute:
DLM_srvr_reroute:
        .byte   dg_st_reroute           # request completion status
        .byte   dgrs_size               # response header length
        .short  0                       # message sequence #
        .byte   0                       # error code #1
        .byte   0                       # error code #2
        .byte   0                       # general purpose reg. #0
        .byte   0                       # general purpose reg. #1
        .word   0                       # remaining message length
        .word   0                       # response message header CRC
#
#
# --- Shared memory data
#
        .section    .shmem
        .align  2
#
# --- Software Detected Fault data structure
#
dlm_sft:
        .space  mlemaxsiz,0
#
        .text
#**********************************************************************
#
# --- executable code -------------------------------------------------
#
#**********************************************************************
#
#  NAME: DLM$find_controller
#
#  PURPOSE:
#       To provide a common means of finding a controller based on its
#       serial number.
#
#  DESCRIPTION:
#       The MLMT list is searched for the specified controller and the
#       associated MLMT is returned to the requester if it was found.
#       If no controller was found, zero is returned.
#
#  CALLING SEQUENCE:
#       call    DLM$find_controller
#
#  INPUT:
#       g0 = controller serial number
#
#  OUTPUT:
#       g0 = MLMT address if the controller was found
#          = 0 if no matching controller was found
#
#  REGS DESTROYED:
#       g0
#
#**********************************************************************
#
DLM_FindController:
DLM$find_controller:
        mov     g0,r15                  # r15 = Controller Serial Number
        ld      dlm_mlmthd,g0           # g0 = first MLMT on list
.dlmfc50:
        cmpobe  0,g0,.dlmfc100          # Jif no more MLMTs on list
        ld      mlmt_sn(g0),r3          # r3 = MLMT Controller serial #
        cmpobe  r15,r3,.dlmfc100        # Jif MLMT for assoc. Controller
        ld      mlmt_link(g0),g0        # r12 = next MLMT on list
        b       .dlmfc50                # and check next MLMT for match
#
.dlmfc100:
        ret
#
#**********************************************************************
#
#  NAME: DLM$vque
#
#  PURPOSE:
#       To provide a common means of queuing VRP requests
#       to the data-link manager.
#
#  DESCRIPTION:
#       The ILT and associated VRP are queued to the tail of the
#       VRP executive queue.  The executive is activated to process this
#       request.  This routine may be called from either the process or
#       interrupt level.
#
#  CALLING SEQUENCE:
#       call    DLM$vque
#
#  INPUT:
#       g1 = ILT at nest level #1
#
# ---  ILT nest level #1 data structure ------------------------------
.ifdef BACKEND
#       dlmi_vrp  (il_w0) = VRP address
.else   # FRONTEND
#       vrvrp     (il_w4) = VRP address
.endif  # ifdef BACKEND
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
DLM$vque:
        lda     DLM_vrp_qu,r11          # Get queue origin
        b       K$cque
#
#**********************************************************************
#
#  NAME: DLM$get_dg
#
#  PURPOSE:
#       Allocates the resources to send a datagram message.
#
#  DESCRIPTION:
#       Allocates an ILT and CDRAM buffer to use in sending a
#       datagram message. Sets up the ILT for use by the caller.
#
#  CALLING SEQUENCE:
#       call    DLM$get_dg
#
#  INPUT:
#       g10 = request message length (not including header length)
#       g11 = response message length (not including header length)
#
#  OUTPUT:
#       g1 = ILT to use for datagram at nest level #1.
#            Nest level #2 has been set up with header pointers,
#            buffer pointers and lengths. These pointers
#            and lengths must NOT be changed!
#
#  REGS DESTROYED:
#       Reg. g1 destroyed.
#
#**********************************************************************
#
DLM$get_dg:
        mov     g0,r12                  # save g0
c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
        lda     dsc1_ulvl(g1),r14       # r14 = ILT at nest level #2
        lda     dsc1_rqhdr(g1),r4       # r4 = pointer to local request
                                        # header area
        lda     dsc1_rshdr(g1),r5       # r5 = pointer to local response
                                        #  header area
        stl     r4,dsc2_rqhdr_ptr(r14)  # save pointers to local headers
        addo    3,g10,r5                # r5 = request message length
        andnot  3,r5,r5                 #      rounded up to word length
        ldconst dgrq_size,r4            # r4 = request header length
        addo    3,g11,r7                # r7 = response message length
        andnot  3,r7,r7                 #      rounded up to word length
        ldconst dgrs_size,r6            # r6 = response header length
        addo    r4,r5,r5                # r5 = request buffer length
        addo    r6,r7,r7                # r7 = response buffer length
        addo    r5,r7,g0                # g0 = total datagram buffer length
c       g0 = s_MallocC(g0, __FILE__, __LINE__); # allocate datagram buffer memory
        mov     g0,r4                   # r4 = request buffer address
        addo    g0,r5,r6                # r6 = response buffer address
        stq     r4,dsc2_rqbuf(r14)      # save request message buffer address
                                        # + request message length
                                        # + response message buffer address
                                        # + response buffer length
        movq    0,r4                    # r4-r7 = 0
        ldconst dsc_rtycnt,r8           # r8 = default retry count
        ldconst dsc_timeout,r9          # r9 = default timeout (seconds)
        st      r4,dsc2_sglptr(r14)     # clear out the SGL pointer
        stq     r4,ILTBIAS+il_w0(r14)   # clear out ILT nest level 3 fields
        stq     r4,ILTBIAS+il_w4(r14)   # clear out ILT nest level 3 fields
        stob    r8,ILTBIAS+dsc3_retry(r14) # initialize default retry count
        stob    r9,ILTBIAS+dsc3_timeout(r14) # init default timeout (seconds)
        mov     r12,g0                  # restore g0
        ret
#
#**********************************************************************
#
#  NAME: DLM$put_dg
#
#  PURPOSE:
#       Deallocates the resources to used to send a datagram message.
#
#  DESCRIPTION:
#       Deallocates the ILT and CDRAM buffer used in sending a
#       datagram message. This routine undoes what DLM$get_dg
#       did.
#
#  CALLING SEQUENCE:
#       call    DLM$put_dg
#
#  INPUT:
#       g1 = ILT used for datagram at nest level #1.
#            Nest level #2 has been set up with header pointers,
#            buffer pointers and lengths in DLM$get_dg. These
#            pointers and lengths must NOT be changed!
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       No regs. destroyed.
#
#**********************************************************************
#
DLM$put_dg:
        movl    g0,r12                  # save g0-g1
        ldq     dsc1_ulvl+dsc2_rqbuf(g1),r4 # r4 = datagram buffer memory
                                        #  address
                                        # r5 = request buffer length
                                        # r6 = response buffer address
                                        # r7 = response buffer length
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
        addo    r7,r5,g1                # g1 = datagram buffer length
c       s_Free(r4, g1, __FILE__, __LINE__);
        movl    r12,g0                  # restore g0-g1
        ret
#
#**********************************************************************
#
#  NAME: DLM$send_dg
#
#  PURPOSE:
#       Sends out a datagram message and returns the response to
#       callers ILT completion routine.
#
#  DESCRIPTION:
#       Checks if a path exists to send the datagram over and if not
#       builds a response header and returns the ILT to the requestor.
#       Determines the best path to send the request over.
#       If a path exists, fills in the source serial # in the request
#       header and copies the request header into the request buffer.
#       Allocates an SRP, builds it up to send the Send Message to
#       XIOtech Controller SRP.
#
#  CALLING SEQUENCE:
#       call    DLM$send_dg
#
#  INPUT:
#       g1 = ILT used for datagram at nest level #3.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       No regs. destroyed.
#
#**********************************************************************
#
DLM$send_dg:
        movq    g0,r4                   # save g0-g3
        movq    g4,r8                   # save g4-g7
        ldl     -ILTBIAS+dsc2_rqhdr_ptr(g1),g2 # g2 = local request message
                                        #  header address
                                        # g3 = local response message
                                        #  header address
        ldq     -ILTBIAS+dsc2_rqbuf(g1),g4 # g4 = request message buffer
                                        #  address
                                        # g5 = request message length
                                        # g6 = response message buffer
                                        #  address
                                        # g7 = response buffer length
        ld      K_ficb,r3               # r3 = FICB address
        ld      fi_cserial(r3),r3       # r3 = my MAG serial #
        bswap   r3,r12                  # r12 = my MAG serial # in big-endian
        st      r12,dgrq_srcsn(g2)      # save my serial # in request header
        lda     dlm$senddg1_iltcr,r13   # r13 = ILT completion handler routine
        st      r13,il_cr(g1)           # save ILT completion handler routine
                                        #  in ILT
        ld      dgrq_dstsn(g2),r13      # r13 = dest. MAG serial #
        cmpobne r12,r13,.senddg_500     # Jif dest. serial # not mine
#
# --- Case: Datagram message destination serial # is ME!
#
        ldob    dgrq_srvcpu(g2),r12     # r12 = server processor code
.ifdef BACKEND
        ldconst dg_cpu_main,r13         # r13 = BE CPU code
.else   # FRONTEND
        ldconst dg_cpu_interface,r13    # r13 = FE CPU code
.endif  # ifdef BACKEND
        cmpobne r12,r13,.senddg_200     # Jif dest. not main CPU
#
# --- Case: Destination ME, my processor.
#
        stl     g2,dss3_rqhdr_ptr(g1)   # save local header addresses in ILT
        ldconst dgrq_size,r12           # r12 = size of request header
        addo    r12,g4,g4               # g4 = request message address not
                                        #  including the header
        subo    r12,g5,g5               # g5 = request message length not
                                        #  including the header
        cmpobe  0,g6,.senddg_50         # Jif no response buffer specified
        ldconst dgrs_size,r12           # r12 = response header size
        addo    r12,g6,g6               # g6 = response message buffer not
                                        #  including the header
        subo    r12,g7,g7               # g7 = response buffer length not
                                        #  including the header
.senddg_50:
        stq     g4,dss3_rqbuf(g1)       # save buffer addresses & lengths
        ld      dgrq_srvname(g2),r12    # r12 = specified server name
        lda     DLM_servers,r13         # r13 = supported server table
.senddg_110:
        ld      (r13),r14               # r14 = server name from table
        cmpobe  0,r14,.senddg_120       # Jif no more servers in table
        cmpobe  r12,r14,.senddg_130     # Jif name matches
        addo    8,r13,r13               # inc. to next entry in table
        b       .senddg_110             # and check next entry in table
#
.senddg_120:
        mov     0,g0                    # set request received OK (no logging)
        ldq     dlm$ddsp_nosrvr,r12     # r12-r15 = error response header
        stq     r12,(g3)                # save response header
        ld      il_cr(g1),r12           # r12 = ILT completion handler routine
        callx   (r12)                   # and go through normal ILT completion
                                        #  handler routine
        b       .senddg_1000            # and we're out of here!
#
.senddg_130:
        ld      4(r13),r14              # r14 = service handler routine
        lda     ILTBIAS(g1),g1          # g1 = datagram ILT at nest level 4
#
#******************************************************************************
#
#       Interface to datagram service handler routines.
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
#       Regs. g0-g7 can be destroyed.
#
#******************************************************************************
#
        callx   (r14)                   # and call service handler routine
        b       .senddg_1000            # and we're out of here!
#
# --- Case: Destination ME, not my processor.
#
.senddg_200:
        ldconst dg_cpu_mmc,r13          # r13 = MMC(CCB) CPU code
        cmpobne r12,r13,.senddg_300     # Jif dest. not CCB CPU
.ifdef BACKEND
#
# --- Case: Destination ME, CCB processor (not supported on BE now)
#
        b       .senddg_900             # return server not supported
                                        #  response for now!
.else   # FRONTEND
#
# --- Case: Destination ME, CCB processor
#
        lda     ILTBIAS(g1),g1          # point to the next level of ILT
        ldconst drdlmtoccb,g0           # g0 = function code (DLM to CCB)
                                        # g4 = request address
                                        #  including header and data
                                        # g5 = request length
                                        #  including header and data
                                        # g6 = response address
                                        #  including header and data
                                        # g7 = response length
                                        #  including header and data
        call    dlm$create_drp          # Create a DRP with all the necessary
                                        #   values to forward to the CCB
                                        # g0 = address of DRP
        lda     dlm$drp_comp,r12        # r12 = completion routine address
        st      r12,il_cr(g1)           # save this completion routine away
        mov     0,r3
        st      g0,vrvrp(g1)            # save the DRP in the ILT
        st      r3,il_w2(g1)            # show no VRP
        st      r3,il_fthd(g1)          # Close link
        st      g3,il_w1(g1)            # save the local response header
.if  DLMFE_DRIVER
        call    dlmt$dlmfe_que          # Queue the request to Debug Driver
.else   # No DLMFE_DRIVER
        mov     g14,r3                  # Save g14
        call    L$que                   # Send this to the CCB
                                        #   Destroys g0-g3 and g14
        mov     r3,g14                  # Restore g14
.endif  # DLMFE_DRIVER
        b       .senddg_1000            # restore g0-g7 and exit
.endif  # ifdef BACKEND
#
.senddg_300:
.ifdef BACKEND
        cmpobne dg_cpu_interface,r12,.senddg_900 # Jif not Interface processor
#
# --- Case: Destination ME, Interface processor.
#
        ldob    dgrq_path(g2),r13       # r13 = specified path #
        cmpobl  MAXISP,r13,.senddg_900  # Jif invalid path #
        shlo    2,r13,r12               # r12 = path # * 4
        ld      dlm_lldmt_dir(r12),r12  # r12 = assoc. LLDMT address for
                                        #  specified path
        cmpobe  0,r12,.senddg_800       # Jif no LLDMT for specified path
        stob    r13,dsc3_path(g1)       # save specified path # in ILT
#
# --- Allocate, build and send a Send Message to XIOtech Controller SRP
#
        ldconst vrpsiz,r13              # r13 = size of VRP header
        ldconst sr_msg_size,r14         # r14 = size of SRP
        addo    r13,r14,g0              # g0 = size of the VRP and SRP
c       g0 += 2;                        # Add 2 extra bytes so SRP isn't immediately after VRP
        mov     g0,r13                  # r13 = size of the VRP and SRP
c       g0 = s_MallocC(g0, __FILE__, __LINE__); # allocate memory for SRP
                                        # g0 = memory address
        mov     g0,r3                   # r3 = VRP
        lda     vr_sglhdr(g0),g0        # g0 = SRP
        st      r13,vr_blen(r3)         # save the size in the VRP
        st      r14,vr_sglsize(r3)      # save the size of the SRP
        ldconst vrbefesrp,r14           # set the function as a BE to FE SRP
        stos    r14,vr_func(r3)
        # If the SRP address immediately follows the VRP, then the LL_LinuxLinkLayer.c
        # code will adjust the addresses so that the Target will see the SRP in it's
        # local memory, rather than the SRP in the Initiator memory. Skip 2 bytes
        # between the VRP and SRP to force the address to be in Initiator memory
        # (the equivalent of changing the address to a PCI address for Bigfoot).
c       g0 += 2;                        # Add 2 bytes for "PCI Address translation"
        mov     g0,r14                  # Translate to global address
        st      r14,vr_sglptr(r3)       # save the SRP address in the VRP
#
        ldconst 0,r14
        st      g0,dsc3_srp(g1)         # save SRP address in ILT
        lda     ILTBIAS(g1),g1          # g1 = ILT at nest level 4
        st      r3,dsc4_vrpsrp(g1)      # save the address of the VRP/SRP
        st      r3,ILTBIAS+vrvrp(g1)    # save the VRP in the next level also
        st      r14,sr_msg_dlmid(g0)    # clear DLM session ID in SRP
        ldob    dlmi_path-ILTBIAS(r12),r13 # r13 = path #
        st      r13,dsc4_path(g1)       # save path # in ILT
        st      g0,dsc4_srp(g1)         # save SRP address in ILT
        st      r12,dsc4_iltvrp(g1)     # save assoc. ILT/VRP address in ILT
        st      r14,sr_msg_lldid(g0)    # clear LLD session ID in SRP
        mov     r12,r15                 # r15 = assoc. ILT/VRP address
        b       .senddg_700             # and finish setting up ILT/SRP
.else   # FRONTEND below
#
# --- Case: Datagram from FE to BE processor - not supported!
        b       .senddg_900             # Unsupported Server
.endif  # ifdef BACKEND
#
# --- Case: Datagram message destination serial # is NOT me!
#
.senddg_500:
        ldconst FALSE,r3                # r3 = DTMT start-over flag
        ld      dsc3_mlmt(g1),r12       # r12 = MLMT from ILT
        cmpobne 0,r12,.senddg_600       # Jif MLMT specified in ILT
#
# --- Find associated MLMT to destination Controller
#
        bswap   r13,r13                 # r13 = dest. Controller serial # in
                                        #  little-endian format
        mov     r13,g0                  # g0 = Serial Number
        call    DLM$find_controller     # See if the controller is available
                                        # g0 = MLMT of controller
        cmpobe  0,g0,.senddg_800        # Jif no controller was found
        mov     g0,r12                  # r12 = MLMT of controller
#
# --- Associated MLMT identified to destination XIOtech Controller.
#       Find appropriate DTMT to send datagram over/to.
#
.senddg_600:
                                        # r12 = assoc. MLMT address
        st      r12,dsc3_mlmt(g1)       # save assoc. MLMT address in ILT
        ldob    dgrq_path(g2),r13       # r13 = path #
        stob    r13,dsc3_path(g1)       # save specified path # in ILT
.if FE_ICL ##  Load Balance
#
# --- Load balancing of DLM paths
#     g1= pILT;
#     g2 = Request message header address = pReqMsgHdr = (DATAGRAM_REQ *)((pILT-1)->ilt_normal.w0)
#
c       r14 = ICL_GetTargetPath((ILT*)g1);
        cmpobne  0,r14,.senddg_640         # Jif  DTMT found
        b       .senddg_800                # reject request with no path
.else # Load Balance

        ld      dsc3_dtmt(g1),r14       # r14 = last DTMT used from ILT
        cmpobne 0,r14,.senddg_603       # Jif last used DTMT defined in ILT
        ld      dsc3_reqdtmt(g1),r14    # r14 = requested DTMT to use for DG
        cmpobne 0,r14,.senddg_615       # Jif a requested DTMT to use
        ld      mlmt_dgdtmt(r12),r14    # r14 = last DTMT used for datagram
        cmpobe  0,r14,.senddg_608       # Jif no last DTMT defined
#
# --- Find last DTMT used in MLMT/DTMT list
#
.senddg_603:
                                        # r14 = last used DTMT
        ld      mlmt_dtmthd(r12),r15    # r15 = first DTMT on list
.senddg_605:
        cmpobe  0,r15,.senddg_608       # Jif last DTMT
        cmpobe  r14,r15,.senddg_620     # Jif DTMT found in list
        ld      dml_mllist(r15),r15     # r15 = next DTMT on list
        b       .senddg_605             # and check next DTMT on list
#
.senddg_608:
        ld      mlmt_dtmthd(r12),r14    # r14 = first DTMT on list
        ldconst TRUE,r3                 # indicate DTMT start-over occurred
.senddg_610:
        cmpobne 0,r14,.senddg_618       # Jif DTMT found
#
# --- DTMT not found. Check if start-over flag is FALSE and if so
#       start over from the top of the list
#
        cmpobe  FALSE,r3,.senddg_608    # Jif start-over flag FALSE
        b       .senddg_800             # reject request with no path
                                        #  available
#
# --- In requested DTMT case, the code was not checking to see if the DTMT
#       still exists in the MLMT. This resulted in SAN #1479 crash!!! Raghu
#
.senddg_615:
        ld      mlmt_dtmthd(r12),r15    # r15 = first DTMT on list
#
.senddg_616:
        cmpobe  0,r15,.senddg_800       # Jif last DTMT
        cmpobe  r14,r15,.senddg_618     # Jif DTMT found in list
        ld      dml_mllist(r15),r15     # r15 = next DTMT on list
        b       .senddg_616             # and check next DTMT on list
#
.senddg_618:
        ldob    dtmt_state(r14),r15     # r15 = target state code
        cmpobe  dtmt_st_op,r15,.senddg_630 # Jif target operational
.ifdef  FRONTEND
#
#       Determine if this is a poll on a not operational path.  If so, let
#           the poll continue, else look for another path.
#
        cmpobne dtmt_st_notop,r15,.senddg_620 # Jif the path is not "not op"
        ld      dgrq_srvname(g2),r15    # get the Server for this Datagram
        ldconst DLM1name,g0             # Get DLM1 server name
        cmpobne g0,r15,.senddg_620      # Jif Not for DLM1 Remote Server
        ldob    dgrq_fc(g2),r15         # Get the Function Code
        cmpobe  DLM1_fc_polldgp,r15,.senddg_630 # Jif it is a Poll Datagram
.endif  # ifdef FRONTEND
.senddg_620:
? # crash - cqt# 24592 - 2008-11-26 -- BE DTMT - failed @ dlm.as:1216  ld 68+r14,r14 with dafedafe - workaround?
        ld      dml_mllist(r14),r14     # r14 = next DTMT on list
        b       .senddg_610             # and check next DTMT
#
.senddg_630:
        ldconst dg_path_any,r15         # r15 = any path code
        cmpobe  r15,r13,.senddg_640     # Jif not a specific path
        ldob    dml_path(r14),r15       # r15 = DTMT assoc. path #
        cmpobne r13,r15,.senddg_620     # Jif path # does not match
.endif ## Load Balance

.senddg_640:
                                        # r14 = chosen path DTMT
        st      r14,dsc3_dtmt(g1)       # save chosen DTMT in ILT
        st      r14,mlmt_dgdtmt(r12)    # save DTMT used in MLMT
# .if ICL_DEBUG
#       c  ICL_SendDg_Dump1((void*)g1, (void*)r14);
# .endif
.if FE_ICL
c       ICL_UpdateDmlPathStats((void*)r14);
.endif  # FE_ICL
#
# --- Allocate, build and send a Send Message to XIOtech Controller SRP
#
        ld      dtmt_pri_dtmt(r14),g0   # g0 = assoc. primary DTMT
        cmpobe.t 0,g0,.senddg_650       # Jif no primary DTMT defined
                                        #  (i.e. not an alias DTMT)
#
# --- An alias DTMT is chosen. Use the associated primary DTMT
#       from here on out.
#
        mov     g0,r14                  # r14 = assoc. primary DTMT address
.senddg_650:
.ifdef BACKEND
        ldconst vrpsiz,r13              # r13 = size of VRP header
        ldconst sr_msg_size,r15         # r15 = size of SRP
        addo    r13,r15,g0              # g0 = size of the VRP/SRP combo
c       g0 += 2;                        # Add 2 extra bytes so SRP isn't immediately after VRP
        mov     g0,r13                  # r13 = size of the VRP/SRP combo
c       g0 = s_MallocC(g0, __FILE__, __LINE__); # allocate memory for SRP
                                        # g0 = memory address
        mov     g0,r3                   # r3 = VRP
        lda     vr_sglhdr(g0),g0        # g0 = SRP
        # If the SRP address immediately follows the VRP, then the LL_LinuxLinkLayer.c
        # code will adjust the addresses so that the Target will see the SRP in it's
        # local memory, rather than the SRP in the Initiator memory. Skip 2 bytes
        # between the VRP and SRP to force the address to be in Initiator memory
        # (the equivalent of changing the address to a PCI address for Bigfoot).
c       g0 += 2;                        # Add 2 bytes for "PCI Address translation"
        mov     g0,r12                  # Translate to global address
        st      r12,vr_sglptr(r3)       # save the SRP address in the VRP
        ldconst vrbefesrp,r12           # set the function as a BE to FE SRP
        stos    r12,vr_func(r3)
        ldconst 0xff,r12
        stob    r12,vr_status(r3)
        st      r13,vr_blen(r3)         # save the size in the VRP
        st      r15,vr_sglsize(r3)      # save the size of the SRP
.else   # FRONTEND
c       g0 = s_MallocC(sr_msg_size, __FILE__, __LINE__); # allocate memory for SRP
        ldconst 0,r3                    # r3 = VRP (none)
.endif  # ifdef BACKEND
#
        st      g0,dsc3_srp(g1)         # save SRP address in ILT
        lda     ILTBIAS(g1),g1          # g1 = ILT at nest level 4
        st      r3,dsc4_vrpsrp(g1)      # save the address of the VRP/SRP
        st      r3,ILTBIAS+vrvrp(g1)    # save the VRP in the next level also
.ifdef BACKEND
        st      r14,sr_msg_dlmid(g0)    # save DLM session ID in SRP
.else   # FRONTEND
        ld      dml_bedlmid(r14),r13    # r13 = BE DLM Session ID
        st      r13,sr_msg_dlmid(g0)    # save the BE DLM Session ID in the SRP
.endif  # ifdef BACKEND
        ld      dtmt_lldmt(r14),r15     # r15 = assoc. LLDMT (ILT/VRP)
? # Crash with r14 pointing to freed dtmt.
        ldob    dlmi_path-ILTBIAS(r15),r13 # r13 = path #
        st      r13,dsc4_path(g1)       # save path # in ILT
        st      g0,dsc4_srp(g1)         # save SRP address in ILT
        st      r15,dsc4_iltvrp(g1)     # save assoc. ILT/VRP address in ILT
        ld      dtmt_lldid(r14),r14     # r14 = LLD session ID
        st      r14,sr_msg_lldid(g0)    # save LLD session ID in SRP
.ifdef BACKEND
.senddg_700:
.endif  # ifdef BACKEND
        ld      lldmt_vrp(r15),r15      # get the vrp
        ldob    dsc3_timeout-ILTBIAS(g1),r12 # get the requested timeout value
        ldob    dsc3_retry-ILTBIAS(g1),r13 # get the requested retry count
        ld      vr_ilt(r15),r15         # get the ILT/VRP on the FE
        stob    r12,sr_msg_cmdto(g0)    # save the timeout value
        stob    r13,sr_msg_rtycnt(g0)   # save the retry counter
        st      r15,sr_vrpilt(g0)       # save the ILT/VRP in the srp
#
        st      g5,sr_msg_reqlen(g0)    # save the request buffer length
.ifdef BACKEND
        mov     g4,r12                  # Translate to global address
        st      r12,sr_msg_reqbuf(g0)   # save the request buffer address (pci)
        mov     g6,r12                  # Translate to global address
        st      r12,sr_msg_respbuf(g0)  # save the response buffer address (pci)
.else   # FRONTEND
        st      g4,sr_msg_reqbuf(g0)    # save the request buffer address
        st      g6,sr_msg_respbuf(g0)   # save the response buffer address
.endif  # ifdef BACKEND
        st      g7,sr_msg_reslen(g0)    # save the response buffer length
                                        #  in SRP
#
# Create the extended SGL List to pass to LLD to set up the hardware, if there
#   is one
#
        lda     sr_msg_esgl(g0),r14     # r14 = out sgl pointer
        ld      dsc2_sglptr-ILTBIAS-ILTBIAS(g1),r15 # r15 = in sgl pointer
        movl    0,r12                   # zero the SGL header
        stl     r12,sg_scnt(r14)
        cmpobe  0,r15,.senddg_760       # Jif there are is no input SGL pointer
        ld      sg_size(r15),r3         # r3 = size of the SGL packet
        shro    3,r3,r3                 # divide by 8 to get number of longs
        cmpobge 5,r3,.senddg_740        # Jif 40 bytes or less - OK
#
        ldconst dlm_sft15,r12           # r12 = error code to log
        mov     g0,r3                   # Save g0
        lda     dlm_sft,g0              # g0 = Software Fault Log Area
        st      r12,efa_ec(g0)          # Save the Error Code
        ldl     sg_scnt(r15),r12        # get the count and size
        stl     r12,efa_data(g0)        # Save the bad count and size
        st      r15,efa_data+8(g0)      # Save the SGL pointer
        ldconst 16,r12                  # Number of bytes saved (ec + data)
        st      r12,mle_len(g0)         # Save the number of bytes to send
        call    M$soft_flt              # Error Trap or Log failure
        mov     r3,g0                   # Restore g0
        b       .senddg_760             # Ignore this SGL
#
.senddg_740:
        cmpobe  0,r3,.senddg_760        # Jif there are no more longs to copy
        subo    1,r3,r3                 # Decr the number left (use as index)
        ldl     sg_scnt(r15)[r3*8],r12  # Get the SGL descriptor/header
        stl     r12,sg_scnt(r14)[r3*8]  # Save the SGL descriptor/header
        b       .senddg_740             # Copy more if needed
#
.senddg_760:
#
        ldconst srsmsg,r12              # r12 = SRP function code
        ldconst srerr,r13
        shlo    24,r13,r13
        or      r13,r12,r12
        ldconst sr_msg_size,r13         # r13 = size of SRP
        mov     0,r14
        mov     g1,r15
        stq     r12,sr_func(g0)         # save sr_func
                                        #  + sr_plen
                                        #  + sr_count
                                        #  + sr_ilt
        st      r14,srpbsiz+sr_len(g0)  # clear sr_len field
        st      r14,srpbsiz+sr_rsvd(g0) # sr_rsvd
        ldq     (g2),r12                # r12-r15 = bytes 0-15 of request
                                        #  header
!       stq     r12,(g4)                # copy request header from local to
                                        #  buffer memory

.if     DEBUG_FLIGHTREC_DLM
        ldconst frt_dlm_dgsend,r14      # DLM - Send Datagram
        ldconst 0x000000ff,r15          # Mask the Server Processor Code
        and     r15,r12,r15             # Get the Server Processor Code
        shlo    8,r15,r15               # Shift the Server Processor Code
        or      r15,r14,r14             # r14 = Server Code, Flight Recorder ID
        shlo    16,r13,r15              # Get the path and Function Code
        or      r15,r14,r14             # r14 = Path, Function Code, Server
        st      r14,fr_parm0            #   Processor Code, Flight Recorder ID
        st      g1,fr_parm1             # ILT
        st      g0,fr_parm2             # SRP
        ld      16(g2),r14              # Get the Server Name (DLM0, LLD0, etc.)
        st      r14,fr_parm3            # Server Name
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_DLM

        ldq     16(g2),r12              # r12-r15 = bytes 16-31 of request
                                        #  header
!       stq     r12,16(g4)
        lda     dlm$senddg2_iltcr,g2    # g2 = ILT completion routine
        mov     0,r3
        st      g2,il_cr(g1)            # Save the completion routine
        st      r3,il_fthd(g1)          # Clear the ILT Forward thread
.ifdef BACKEND
        mov     g14,r3                  # save g14
        call    L$que                   # Queue the ILT to the Link 960 Queue
                                        #  (wipes g0-g3, g14)
        mov     r3,g14                  # restore g14
.else   # FRONTEND
        st      r3,ILTBIAS+il_fthd(g1)  # Clear the next ILT Forward thread
        ld      dsc4_iltvrp(g1),r13     # r13 = assoc. ILT/VRP address in ILT
        lda     ILTBIAS(g1),g1          # g1 = ILT at nest level 5 (otl1 level)
        ld      il_misc(r13),r13        # r13 = the ILT Parms Pointer
        mov     g1,r12                  # Save this level of ILT
        st      r13,otl1_FCAL(g1)       # Save the Parms Pointer at this level
        st      g0,otl1_srp(g1)         # Save the SRP at this level
        ldconst srsmsg,r13              # r13 = SRP function code
        lda     K$comp,r14              # Set completion routine to bump levels
        st      r13,otl1_cmd(g1)        # Save the Function code at this level
        st      r3,otl1_cmpcode(g1)     # Set the completion code to good
        st      r14,il_cr(g1)           # Set the next completion routine
#
        lda     ILTBIAS(g1),g1          # advance to next nesting level
        st      r12,il_misc(g1)         #  and point back to struct
        st      r3,il_cr(g1)            # Clear next nest completion routine
#
# --- Submit SRP to Translation Layer.
#
        call    C$receive_srp
.endif  # ifdef BACKEND
        b       .senddg_1000            # and get out of here!
#
.senddg_800:
        ldq     dlm$sdsp_nopath,r12     # r12-r15 = error response header
                                        #  handler routine
        b       .senddg_910             # and we're out of here!
#
.senddg_900:
        ldq     dlm$ddsp_nosrvr,r12     # r12-r15 = error response header
.senddg_910:
        stq     r12,(g3)                # save response header
        mov     0,g0                    # set request received OK (no logging)
        ld      il_cr(g1),r12           # r12 = ILT completion handler routine
        callx   (r12)                   # and go through normal ILT completion
                                        #  handler routine
.senddg_1000:
        movq    r4,g0                   # restore g0-g3
        movq    r8,g4                   # restore g4-g7
        ret
#
#**********************************************************************
#
#  NAME: dlm$senddg1_iltcr
#
#  PURPOSE:
#       Processes a datagram ILT completion event setup by the DLM$send_dg
#       routine to a local server.
#
#  DESCRIPTION:
#       Adjusts the ILT pointer to nest level 2 and returns the ILT
#       back to the original caller.
#
#  CALLING SEQUENCE:
#       call    dlm$senddg1_iltcr
#
#  INPUT:
#       g1 = datagram ILT at nest level 3
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
dlm$senddg1_iltcr:
        movq    g0,r12                  # save g0-g3
        lda     -ILTBIAS(g1),g1         # back ILT up to nest level 2
        ldl     dsc2_rqhdr_ptr(g1),r4   # r4 = local request header address
                                        # r5 = local response header address
        ld      dsc2_rsbuf(g1),r6       # r6 = response buffer address
        cmpobe  0,r6,.senddg1iltcr_200  # Jif no response buffer address
        ldos    dgrq_seq(r4),r8         # r8 = request message sequence #
        stos    r8,dgrs_seq(r5)         # save sequence # in response header
        ldq     (r5),r8                 # r8-r11 = response message header
!       stq     r8,(r6)                 # save response message header in
                                        #  response buffer
.senddg1iltcr_200:
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT completion handler routine
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: dlm$senddg2_iltcr
#
#  PURPOSE:
#       Processes a datagram ILT completion event setup by the DLM$send_dg
#       routine to a remote server.
#
#  DESCRIPTION:
#       Checks for VRP status errors and handles appropriately if found.
#       Copies the response header into the local response header area.
#       Checks for response header errors and if found checks for
#       recoverable errors and if found attempts to recovery the datagram.
#       Adjusts the ILT pointer to nest level 2 and returns the ILT
#       back to the original caller.
#
#  CALLING SEQUENCE:
#       call    dlm$senddg2_iltcr
#
#  INPUT:
.ifdef FRONTEND
#       g0 = completion code
.endif  # ifdef FRONTEND
#       g1 = datagram ILT at nest level 4
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
dlm$senddg2_iltcr:
        movq    g0,r12                  # save g0-g3
.if ICL_DEBUG
        ldconst 0,r7
.endif  # ICL_DEBUG
.ifdef BACKEND
        ld      dsc4_vrpsrp(g1),g0      # get the VRP/SRP address
        lda     -ILTBIAS(g1),g1         # g1 = ILT at nest level 3
        ldob    vr_status(g0),r4        # r4 = SRP completion status (really
                                        #   in the VRP)
.else   # FRONTEND
        lda     -ILTBIAS(g1),g1         # g1 = ILT at nest level 3
        ld      dsc3_srp(g1),g0         # g0 = SRP address
.if  ICL_DEBUG
        ld      dsc3_dtmt(g1),r4        # Get DTMT
        cmpobe  0,r4,.senddg2iltcr_icl01
        ldob    dtmt_icl(r4),r7         # Get ICL flag
        cmpobe FALSE,r7,.senddg2iltcr_icl01
c fprintf(stderr,"%s%s:%u <dlm$senddg2_iltcr>ICL srp comp.status = %u\n", FEBEMESSAGE, __FILE__, __LINE__,(UINT32)r12);
.senddg2iltcr_icl01:
.endif  # ICL_DEBUG
        mov     r12,r4                  # r4 = Completion status
        stob    r4,sr_status(g0)        # save the SRP completion status
.endif  # ifdef BACKEND
        lda     -ILTBIAS(g1),r5         # r5 = ILT at nest level 2
        cmpobe  0,r4,.senddg2iltcr_100  # Jif successful completion status
        ld      dsc2_rsbuf(r5),r6       # r6 = response buffer address
        cmpobe  0,r6,.senddg2iltcr_100  # Jif no response buffer address
        ldq     dlm$sdsp_nopath,r8      # r8-r11 = response header template
        stq     r8,(r6)                 # save response header template in
                                        #  response buffer
.senddg2iltcr_100:
.ifdef BACKEND
        ld      vr_blen(g0),g1          # g1 = size of the VRP/SRP combo
.else   # FRONTEND
        ldconst sr_msg_size,g1          # g1 = size of SRP
.endif  # ifdef BACKEND
.if     DEBUG_FLIGHTREC_DLM
        mov     g0,r3                   # r3 = SRP Address for FR entry
.endif  # DEBUG_FLIGHTREC_DLM
c       s_Free_and_zero(g0, g1, __FILE__, __LINE__);
        ld      dsc2_rsbuf(r5),g0       # g0 = response buffer address
        cmpobe  0,g0,.senddg2iltcr_300  # Jif no response buffer address
        mov     g0,r6                   # r6 = response buffer address
        ld      dsc2_rshdr_ptr(r5),r4   # r4 = local response header address
!       ldq     (r6),r8                 # r8-r11 = response message header
                                        # r10 = remaining response message
                                        #  length in big-endian
        stq     r8,(r4)                 # save response message header in
                                        #  local response header area
        ldob    dgrs_status(r4),r8      # r8 = datagram completion status byte
.if  ICL_DEBUG
        cmpobe FALSE,r7,.senddg2iltcr_icl05
c fprintf(stderr,"%s%s:%u dlm$senddg2_iltcr>ICL dlm packet  completion status = %u\n", FEBEMESSAGE, __FILE__, __LINE__,(UINT32)r8);
.senddg2iltcr_icl05:
.endif  # ICL_DEBUG
        cmpobe  dg_st_ok,r8,.senddg2iltcr_300 # Jif datagram successful

.if     DEBUG_FLIGHTREC_DLM
        ld      dsc2_rqbuf(r5),r9       # r9 = Request Buffer Address
        ldconst frt_dlm_snddg2,r7       # DLM - message handler exec
!       ldos    dgrq_seq(r9),r11        # r11 = Response Sequence Number
!       ldob    dgrq_fc(r9),r10         # r10 = Request Function Code
        shlo    16,r11,r11              # Prepare for multiple values in parm0
        shlo    8,r10,r10
        or      r11,r7,r7
        or      r10,r7,r7               # Sequence #, Function Code, FR ID
        ldob    dgrs_ec1(r4),r9         # r9 = Error Code #1
        ldob    dgrs_ec2(r4),r10        # r10 = Error Code #2
        ldob    ILTBIAS+dsc3_retry(r5),r11 # r11 = datagram error retry count
        shlo    16,r9,r9                # Prepare for multiple values in parm3
        shlo    8,r10,r10
        or      r9,r11,r11
        shlo    24,r8,r9
        or      r10,r11,r11
        or      r9,r11,r11              # DG Status, EC1, EC2, Retry Count
        st      r7,fr_parm0             # Sequence #, Function Code, FR ID
        st      r13,fr_parm1            # ILT
        st      r3,fr_parm2             # SRP
        st      r11,fr_parm3            # DG Status, EC1, EC2, Retry Count
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_DLM

        cmpobe  dg_st_sdsp,r8,.senddg2iltcr_240 # Jif error from source
                                        #  datagram service provider
        cmpobe  dg_st_slld,r8,.senddg2iltcr_240 # Jif error from source LLD
                                        #  level
        cmpobe  dg_st_dlld,r8,.senddg2iltcr_200 # Jif a dest LLD error
        cmpobne dg_st_reroute,r8,.senddg2iltcr_300 # Jif not datagram reroute
                                        #  status
#
# --- Process datagram reroute completion status
#
        ldob    ILTBIAS+dsc3_reroute(r5),r7 # r7 = datagram reroute retry
                                        #          count
        subo    1,r7,r7                 # dec. retry count
        and     0x0f,r7,r7
        stob    r7,ILTBIAS+dsc3_reroute(r5) # save updated retry count
        cmpobe  0,r7,.senddg2iltcr_240  # Jif datagram reroute retry expired
#
# --- Datagram reroute retry count has not expired
#
        ld      ILTBIAS+dsc3_orig_dsn(r5),r3 # r3 = original dest. serial #
        ld      dsc2_rqhdr_ptr(r5),r8   # r8 = local req. header addr.
        ld      dsc2_rqbuf(r5),r9       # r9 = request message buffer addr.
        ld      dgrs_csn(r4),r7         # r7 = controller serial # to reroute
                                        #      datagram to
        cmpobne 0,r3,.senddg2iltcr_180  # Jif original datagram serial #
                                        #  already saved
        ld      dgrq_dstsn(r8),r3       # r3 = original dest. serial #
        st      r3,ILTBIAS+dsc3_orig_dsn(r5) # save original dest. serial #
                                        #  in ILT just in case
.senddg2iltcr_180:
        ldconst 0,r3
        st      r7,dgrq_dstsn(r8)       # save serial # in local req. header
        st      r7,dgrq_dstsn(r9)       # save serial # in request buffer
        st      r3,ILTBIAS+dsc3_mlmt(r5) # clear assoc. MLMT address field
        st      r3,ILTBIAS+dsc3_dtmt(r5) # clear assoc. DTMT address field
        b       .senddg2iltcr_280       # and set up to resend to new
                                        #  destination
#
.senddg2iltcr_200:
        ldob    dgrs_ec1(r4),r3         # r3 = Error Code 1 status
        cmpobe  dgec1_dlld_crc,r3,.senddg2iltcr_240 # Jif possible transport -
                                        #  retry error
        cmpobge dgec1_dlld_novdisk,r3,.senddg2iltcr_300 # Jif not a possible
                                        #  transport error from the ISP layer
#
.senddg2iltcr_240:
        ld      ILTBIAS+dsc3_orig_dsn(r5),r3 # r3 = original dest. serial #
        cmpobe  0,r3,.senddg2iltcr_250  # Jif original dest. serial # not
                                        #  changed
        ld      dsc2_rqhdr_ptr(r5),r8   # r8 = local req. header addr.
        ld      dsc2_rqbuf(r5),r9       # r9 = request message buffer addr.
        ldconst 0,r7
        st      r3,dgrq_dstsn(r8)       # save original datagram serial #
                                        #  in local req. header
        st      r3,dgrq_dstsn(r9)       # save original datagram serial #
                                        #  in request buffer
        st      r7,ILTBIAS+dsc3_mlmt(r5) # clear assoc. MLMT address field
        st      r7,ILTBIAS+dsc3_dtmt(r5) # clear assoc. DTMT address field
.senddg2iltcr_250:
        lda     ILTBIAS(r5),g1          # g1 = datagram ILT at nest level 3
        ldob    dsc3_retry(g1),r8       # r8 = datagram error retry count
        subo    1,r8,r8                 # dec. error retry count
        cmpobe  0,r8,.senddg2iltcr_300  # Jif retry count expired
        stob    r8,dsc3_retry(g1)       # save updated error retry count
.senddg2iltcr_280:
        mov     r5,g1                   # g1 datagram ILT at nest level 2
        call    dlm$dg_retry            # queue up to retry after delay
        b       .senddg2iltcr_1000      # and we've done our job for now!
#
.senddg2iltcr_300:
        mov     r12,g0                  # g0 = ILT Completion status
        mov     r5,g1                   # g1 = ILT at nest level 2
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT completion handler routine
.senddg2iltcr_1000:
        movq    r12,g0                  # restore g0-g3
        ret
#
.ifdef BACKEND
#**********************************************************************
#
#  NAME: DLM$just_senddg
#
#  PURPOSE:
#       Sends out a datagram message for a client that does not need
#       response completion processing.
#
#  DESCRIPTION:
#       Sends out the specified/formatted datagram message. Upon
#       completion, checks if an error occurred. If so, decrements
#       the retry count specified by the client and if non-zero
#       delays for a period of time and then retries sending the
#       datagram until either it completes successfully or the retry
#       count expires. When processing of the datagram message has
#       completed, it returns the datagram resources.
#
#  CALLING SEQUENCE:
#       call    DLM$just_senddg
#
#  INPUT:
#       g0 = datagram error retry count
#       g1 = ILT used for datagram at nest level #2.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       No regs. destroyed.
#
#**********************************************************************
#
DLM$just_senddg:
        mov     g1,r12                  # save g1
        st      g0,dsc2_g0(g1)          # save error retry count
        lda     dlm$justdg_iltcr,r4     # r4 = datagram completion handler
        st      r4,il_cr(g1)            # save completion handler routine
        call    dlm$dg_retry            # queue up to retry after delay
        mov     r12,g1                  # restore g1
        ret
.endif  /* BACKEND */
#
.ifdef BACKEND
#**********************************************************************
#
#  NAME: dlm$justdg_iltcr
#
#  PURPOSE:
#       Processes a datagram ILT completion event setup by the
#       DLM$just_senddg routine.
#
#  DESCRIPTION:
#       Checks for an error reported on the datagram message. If
#       no error is reported, returns the datagram resources back
#       into their respective pools. If an error is reported, tests
#       the retry count and if expired returns the datagram resources
#       back into their respective pools. If the retry count has not
#       expired, decrements it, delay for some period of time and then
#       resends the datagram until either it completes successfully
#       or the error retry count expires.
#
#  CALLING SEQUENCE:
#       call    dlm$justdg_iltcr
#
#  INPUT:
#       g1 = datagram ILT at nest level 2
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
dlm$justdg_iltcr:
        movq    g0,r12                  # save g0-g3
        ld      dsc2_rshdr_ptr(g1),r6   # r6 = local response header address
        ldob    dgrs_status(r6),r11     # r11 = request completion status
        cmpobe  dg_st_ok,r11,.justdgiltcr_500 # Jif successful
        ld      dsc2_g0(g1),r4          # r4 = error retry count
        cmpobe  0,r4,.justdgiltcr_500   # Jif error retry count expired
        subo    1,r4,r4                 # dec. error retry count
        st      r4,dsc2_g0(g1)          # save updated error retry count
        call    dlm$dg_retry            # queue up to retry after delay
        b       .justdgiltcr_1000       # and get out of here!
#
.justdgiltcr_500:
        lda     -ILTBIAS(g1),g1         # g1 = datagram ILT at nest level 1
        call    DLM$put_dg              # deallocate datagram resources
.justdgiltcr_1000:
        movq    r12,g0                  # restore g0-g3
        ret
.endif  /* BACKEND */
#
#**********************************************************************
#
# ----------------- Data-link Manager Processes -----------------------
#
#**********************************************************************
#
#  NAME: dlm$vrpx
#
#  PURPOSE:
#       To provide a means of processing VRP requests which have been
#       previously queued to the data-link manager.
#
#  DESCRIPTION:
#       The queuing routine DLM$vque deposits a VRP request into the queue
#       and activates this executive if necessary.  This executive
#       extracts the next VRP request from the queue and directs it to the
#       appropriate VRP handler routine based on the VRP function code.
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
dlm$vrpx:
        call    K$qxchang               # Exchange processes
#
# --- Initialize DTMT pool
#
c       init_dtmt(16);                  # Initialize 16 DTMTs
#
# --- Initialize MLMT pool
#
c       init_mlmt(16);                  # Initialize 16 MLMTs
#
.if DLMFE_DRIVER
#
# --- Create a random number seed based on the timer if the init seed is zero
#
        ld      DLMT_1st_rand_seed,r10  # Get the 1st random seed number
        cmpobne 0,r10,.dlmti100         # Jif a seed already has been set
        ld      TCR0,r6                 # Get the timer value and generate a
        and     0x1,r3,r7               #  random seed to start with all the
        cmpobne 0,r7,.dlmti50           #  numbers read to date
        xor     r4,r5,r7
        mulo    r6,r7,r10
        b       .dlmti100
#
.dlmti50:
        and     0xF,r5,r5
        subo    r4,r3,r7
        cmpo    0,r7
        sele    r7,r5,r7
        divo    r7,r6,r10
.dlmti100:
        st      r10,DLMT_1st_rand_seed  # Save the generated seed to do again
        st      r10,DLMT_rand_seed      # store the random seed to start with
#
        ldconst MAX_DRP_ERR,g0          # Pick a random DRP to report an
        call    dlmt$getscaledrand      #  error on
        cmpo    0,g0                    # Ensure the count is not 0
        sele    g0,1,g0
        st      g0,DLMT_nerr            # Save the DRP number to report error on
#
# --- Establish the FE DLM Debug Driver before other code is available
#
        lda     dlmt$dlmfe_driver,g0    # Establish the FE DLM Driver process
        ldconst DLMVEXECPRI-1,g1        # Make a higher priority than VRP exec
c       CT_fork_tmp = (ulong)"dlmt$dlmfe_driver";
        call    K$fork
        st      g0,dlm_fedriver_pcb     # Save the PCB
.endif  # DLMFE_DRIVER
#
        b       .ex20_c
#
# --- Set this process to not ready
#
.ex10_c:
        ldconst pcnrdy,r4               # Set this process to not ready
        stob    r4,pc_stat(r15)
#
# --- Exchange processes ----------------------------------------------
#
.ex20_c:
        call    K$qxchang               # Exchange processes
#
# --- Get next queued request
#
        lda     DLM_vrp_qu,r11          # Get VRP queue pointer
        ldq     qu_head(r11),r12        # r12 = queue head pointer
                                        # r13 = queue tail pointer
                                        # r14 = queue count
                                        # r15 = PCB address
        mov     r12,g14                 # g14 = ILT being processed at nest
                                        #  level #2
        cmpobe  0,r12,.ex10_c           # Jif none
#
# --- Remove this request from queue ----------------------------------
#
        ld      il_fthd(r12),r12        # r12 = next ILT on queue
        cmpo    0,r12                   # Check for queue now empty
        subo    1,r14,r14               # Adjust queue count
        sele    r13,r12,r13             # Set up queue tail
        stt     r12,qu_head(r11)        # Update queue head, tail and count
        be     .ex30_c                  # Jif queue now empty
#
        st      r11,il_bthd(r12)        # Update backward thread
#
.ex30_c:
.ifdef BACKEND
        ld      dlmi_vrp-ILTBIAS(g14),g13 # g13 =  VRP request
.else   # FRONTEND
        ld      vrvrp-ILTBIAS(g14),g13  # g13 = VRP request
.endif  # ifdef BACKEND
#
# --- Validate function code
#
        ldos    vr_func(g13),r14        # r14 = function code

.if     DEBUG_FLIGHTREC_DLM
        ldconst frt_dlm_msge,r5         # DLM - message handler exec
        shlo    16,r14,r4               # Have several values in parm0
        or      r4,r5,r5                # r5 = Function, Flight Recorder ID
        st      r5,fr_parm0             # Function, Flight Recorder ID
        st      g14,fr_parm1            # ILT
        st      g13,fr_parm2            # VRP
        ld      vr_ilt(g13),r4          # FE ILT for this VRP
        st      r4,fr_parm3
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_DLM

        ldconst vrmagst,r4              # r4 = DLM starting VRP function code
        ldconst vrmagend,r5             # r5 = DLM ending VRP function code
        cmpobg  r4,r14,.ex980           # Jif invalid VRP function code
        cmpobg  r14,r5,.ex980           # Jif invalid VRP function code
        subo    r4,r14,r4               # r4 = normalized VRP function code
        ldob    vr_path(g13),r6         # r6 = get the path out of the VRP
        stob    r6,dlmi_path-ILTBIAS(g14) # Save the path number in the ILT
        lda     dlm$vrphand,r5          # r5 = VRP handler routine table
        ld      (r5)[r4*4],r5           # r5 = VRP handler routine
#
.if     DEBUG_DLM1
        st      g13,debug_dlm1
.endif  # DEBUG_DLM1
#
        callx   (r5)                    # and call VRP handler routine
#
#*****************************************************************************
#
#       Interface to VRP handler routines
#
#  INPUT:
#       g13 = VRP address to process
#       g14 = ILT associated with VRP at nest level #2
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#*****************************************************************************
#
.if     DEBUG_DLM1
        ldconst 0,r4
        st      r4,debug_dlm1
.endif  # DEBUG_DLM1
        b       .ex20_c                 # check if any more VRPs to process
#
# ----------------------------------------------------------------------------
# --- Set invalid function status in VRP and return
#
.ex980:
        ldconst ecinvfunc,r5            # r5 = invalid function VRP status
#
# --- Complete request
#
        stob    r5,vr_status(g13)       # Save error code
        mov     g14,g1                  # g1 = ILT being completed
        call    K$comp                  # Complete this request
        b       .ex20_c                 # and check for more work
#
#**********************************************************************
#
#  NAME: dlm$LOP
#
#  PURPOSE:
#       Processes a link-level driver operational VRP request.
#
#  DESCRIPTION:
#       Check if an ILT/VRP is registered in the link-level driver
#       operational ILT/VRP directory for the same path and if so
#       it performs the necessary processing to terminate this ILT/VRP
#       before registering a new ILT/VRP in it's place. It then
#       registers the ILT/VRP in the link-level driver operational
#       ILT/VRP directory.
#
#  CALLING SEQUENCE:
#       call    dlm$LOP
#
#  INPUT:
#       g13 = VRP address to process
#       g14 = ILT associated with VRP at nest level 2
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
dlm$LOP:
        ldconst eccancel,r7             # r7 = cancelled VRP status
        stob    r7,vr_status(g13)       # Save error code
        ldob    -ILTBIAS+dlmi_path(g14),r4 # r4 = path #
# .if ICL_DEBUG
# c       if ((UINT32)r4 == ICL_PORT)fprintf(stderr,"<dlm$LOP> ICL.... port =%u\n",(UINT32)r4);
# .endif

        lda     dlm_op_dir,r5           # r5 = base of LLD op. ILT/VRP
                                        #  directory
        ld      (r5)[r4*4],r6           # r6 = old ILT/VRP registered
        cmpobe  0,r6,.dlmLOP_300        # Jif no ILT/VRP registered
        movq    g4,r12                  # save g4-g7
        mov     r6,g6                   # g6 = assoc. LLDMT address
.dlmLOP_100:
        ld      lldmt_dtmthd(g6),g4     # g4 = top DTMT on list
        cmpobe  0,g4,.dlmLOP_200        # Jif no more DTMTs assoc. with LLDMT
        ld      dtmt_ehand(g4),r3       # r3 = DTMT event handler table
        ld      dtmt_eh_tgone(r3),r7    # r7 = target disappeared event
                                        #  handler routine
        callx   (r7)                    # call target disappeared event
                                        #  handler routine
        b       .dlmLOP_100             # and loop till all DTMTs terminated
#
.dlmLOP_200:
.ifdef  BACKEND
        mov     g1,r7                   # save g1
        mov     r6,g1                   # g1 = ILT being completed
        call    K$comp                  # Complete this request
        mov     r7,g1                   # restore g1
.endif  # ifdef BACKEND
        movq    r12,g4                  # restore g4-g7
.dlmLOP_300:
# .if ICL_DEBUG
# c       if ((UINT32)r4 == ICL_PORT)fprintf(stderr,"<dlm$LOP> ICL....updating  LLDMT dir .. port =%u\n",(UINT32)r4);
# .endif
        movq    0,r8                    # r8-r11 = 0
        stq     r8,il_w0(g14)           # clear out ILT for use as LLDMT
        stq     r8,il_w4(g14)
        st      g14,(r5)[r4*4]          # save ILT/VRP in LLD op. directory
        st      g13,lldmt_vrp(g14)      # save VRP in LLDMT
        stob    r4,lldmt_channel(g14)   # save channel associated with LLDMT
.ifdef BACKEND
        movl    g0,r12                  # save g0-g1
        ldob    DLM_channel_op_flags[r4*1],g0 # g0 = Channel op. flags
        mov     r4,g1                   # g1 = Channel # going operational
        call    dlm$send_sif            # send Set Initiator Flags SRP
        movl    r12,g0                  # restore g0-g1
.else   # FRONTEND
c       r7 =    ICL_IsIclMagLinkVRP((VRP *)g13);
        cmpobe  TRUE,r7,.dlmLOP_icl05   # Jif  ICL Mag Link Req.
        call    dlm$SBE                 # Send the VRP on to the Back End
.dlmLOP_icl05:
#
# ---  completing the request without being sent to Back End
#
.endif  # ifdef BACKEND

        ret
#
#**********************************************************************
#
#  NAME: dlm$MLE
#
#  PURPOSE:
#       Processes a XIOtech Controller link established VRP request.
#
#  DESCRIPTION:
#       If the DLM session ID field is zero, scans the DTMT list
#       associated with the LLDMT to see if the target has already
#       been identified and if so uses the DTMT associated with the
#       previously identified target. If not, allocates a new DTMT
#       and saves the target information from the VRP in the DTMT.
#       It updates the DTMT list in the associated MLMT if necessary.
.ifdef BACKEND
#       It updates the storage controller list for the CCB if
#       necessary. It schedules an LDD scan to determine if the target
#       is associated with any VLinks that should open a path to this
#       target. It then returns completion status to the requesting path
#       with the results of the VRP processing.
.else   # FRONTEND
#       The request is then forwarded to the Back-End processor to do the
#       necessary processing in that processor.
.endif  # ifdef BACKEND
#
#  CALLING SEQUENCE:
#       call    dlm$MLE
#
#  INPUT:
#       g13 = VRP address to process
#       g14 = ILT associated with VRP at nest level 2
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
dlm$MLE:
        movq    g4,r12                  # save g4-g7
        ldob    -ILTBIAS+dlmi_path(g14),r4 # r4 = path #
# .if ICL_DEBUG
#         c       r5 = ICL_IsIclPort((UINT8)r4);
#         cmpobe  FALSE,r5,.dlmMLE_icl01
# c       fprintf(stderr,"<DLM$MLE>ICL.. Entering for  port no=%u\n",(UINT32)r4);
# .dlmMLE_icl01:
# .endif
        lda     dlm_lldmt_dir,r5        # r5 = base of LLDMT directory
        ld      (r5)[r4*4],g6           # g6 = LLDMT for this interface
        cmpobe  0,g6,.dlmMLE_900        # Jif no LLDMT defined for this
                                        #  interface
        ld      vr_mle_dlmid(g13),g4    # g4 = DLM session ID from VRP
        cmpobe  0,g4,.dlmMLE_100        # Jif DLM session ID not specified
.ifdef FRONTEND
#
# --- Find the FE DTMT that matches the BE DTMT
#
                                        # g4 = BE DLM ID passed in
                                        # g6 = LLDMT to use to find FE DLM ID
        call    dlm$findbedlmid         # g4 = 0 or FE DLM ID that matches
        cmpobe  0,g4,.dlmMLE_100        # Jif no matching FE DLM session ID

.endif  # ifdef FRONTEND
        ld      dtmt_bnr(g4),r7         # r7 = banner value from DTMT
        ld      dtmt_banner,r8          # r8 = DTMT banner pattern
        cmpobne r7,r8,.dlmMLE_100       # Jif specified DTMT not in use
        ldob    dtmt_type(g4),r7        # r7 = DTMT type code
        cmpobne dtmt_ty_MAG,r7,.dlmMLE_100 # Jif DTMT not MAG Link type
        ld      dtmt_lldmt(g4),r7       # r7 = assoc. LLDMT address
        ldob    -ILTBIAS+dlmi_path(r7),r7 # r7 = path # that DTMT is assoc. with
        cmpobne r4,r7,.dlmMLE_100       # Jif specified DTMT in use with a
                                        #  different path
        b       .dlmMLE_150             # use the specified DTMT
#
# --- Scan DTMT list for a match on the MAC address
#
.dlmMLE_100:
        ld      vr_mle_magdt(g13),r5    # r5 = MAGDT address (FE local address)
        movl    g8,r6                   # save g8-g9
!       ldl     magdt_pwwn(r5),g4       # g4-g5 = port WWN of target
!       ldl     magdt_nwwn(r5),g8       # g8-g9 = node WWN of target
        call    dlm$chk4dtmt            # check for match on port WWN
        movl    r6,g8                   # restore g8-g9
        cmpobe  0,g4,.dlmMLE_120        # Jif no match found
        ldob    dtmt_type(g4),r7        # r7 = DTMT type code
        cmpobne dtmt_ty_MAG,r7,.dlmMLE_120 # Jif DTMT not MAG Link type
        b       .dlmMLE_150             # use the identified DTMT
#
.dlmMLE_120:
# .if ICL_DEBUG
# c       if((UINT32)r4 == ICL_PORT)fprintf(stderr,"<DLM$MLE>ICL.. allocating DTMT for port = %u..\n",(UINT32)r4);
# .endif
c       g4 = get_dtmt();                # allocate a DTMT for this target
.ifdef M4_DEBUG_DTMT
c fprintf(stderr, "%s%s:%u get_dtmt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g4);
.endif # M4_DEBUG_DTMT
        st      g6,dtmt_lldmt(g4)       # save assoc. LLDMT in DTMT
        lda     dtmt_etbl1,r7           # r7 = DTMT event handler table
        st      r7,dtmt_ehand(g4)       # save DTMT event handler table
        ldconst dtmt_ty_MAG,r7          # r7 = DTMT type code
        stob    r7,dtmt_type(g4)        # save DTMT type code
        ld      lldmt_dtmttl(g6),r7     # r7 = last DTMT on list
        cmpobne 0,r7,.dlmMLE_130        # Jif list not empty
        st      g4,lldmt_dtmthd(g6)     # save DTMT as new head member
        b       .dlmMLE_140
#
.dlmMLE_130:
        st      g4,dtmt_link(r7)        # link DTMT onto end of list
.dlmMLE_140:
        st      g4,lldmt_dtmttl(g6)     # put DTMT as new tail member
#
# --- Update DTMT info. from MAGDT
#
.dlmMLE_150:
        ld      vr_mle_lldid(g13),r4    # r4 = link-level driver session ID
        st      r4,dtmt_lldid(g4)       # save link-level driver session ID
                                        #  in DTMT
        ld      vr_mle_magdt(g13),r5    # r5 = MAGDT address (FE local address)
!       ld      magdt_alpa(r5),r7       # r7 = AL-PA address
        st      r7,dtmt_alpa(g4)        # save AL-PA address in DTMT
!       ldl     magdt_nwwn(r5),r8       # r8-r9 = node WWN
        stl     r8,dtmt_nwwn(g4)        # save node WWN in DTMT
!       ldl     magdt_pwwn(r5),r8       # r8-r9 = port WWN
        stl     r8,dtmt_pwwn(g4)        # save port WWN in DTMT
!       ld      magdt_sn(r5),r7         # r7 = MAGNITUDE serial number
        st      r7,dml_sn(g4)           # save serial # in DTMT
!       ldob    magdt_path(r5),r7       # r7 = path #
        stob    r7,dml_path(g4)         # save path #
!       ldob    magdt_cl(r5),r7         # r7 = assigned cluster #
        stob    r7,dml_cl(g4)           # save cluster #
!       ldob    magdt_vds(r5),r7        # r7 = # VDisks
        stob    r7,dml_vdcnt(g4)        # save VDisk count
!       ldl     magdt_name(r5),r8       # r8-r9 = node name
        stl     r8,dml_pname(g4)        # save node name
!       ld      magdt_ip(r5),r7         # r7 = assigned IP address
        st      r7,dml_ip(g4)           # save IP address
!       st      g4,magdt_dlmid(r5)      # save DTMT address in MAGDT
!       ldob    magdt_flag1(r5),r7      # r7 = flag byte #1
        stob    r7,dml_flag1(g4)        # save flag byte #1 in DTMT
!       ld      magdt_alias(r5),r7      # r7 = alias node serial number
        st      r7,dml_alias(g4)        # save alias node serial number
                                        #  in DTMT
        ldconst 0,r4                    # Clear the Successful Poll Counter
        stos    r4,dml_poll_cnt(g4)

.if  FE_ICL  #ICL handling
#
# --- Check Mag link option bit whether it is of ICL
#     Note:- Also the port number is available in vr_path of g13, can also
#     be checked to see whether it is ICL port or not.
#
c       r7 =   ICL_IsIclMagLinkVRP((VRP *)g13);
        cmpobe FALSE,r7,.dlmMLE_icl05   # Jif not ICL Mag Link Req.
.if ICL_DEBUG
c fprintf(stderr,"%s%s:%u <dlm$MLE> Creating DTMT related to ICL- dtmt = %x\n", FEBEMESSAGE, __FILE__, __LINE__,(UINT32)g4);
.endif  # ICL_DEBUG
#
# --- Set a flag in DTMT, indicating that this ICL related DTMT
#
        ldconst TRUE,r6
        stob    r6,dtmt_icl(g4)         #  Identify as ICL DTMT
.dlmMLE_icl05:
.endif     # ICL handling

        call    dlm$upmlmt              # update MLMT structures with DTMT
                                        #  as appropriate
.ifdef BACKEND
        call    dlm$upsul               # update storage unit list info.
        call    dlm$sched_lddx          # schedule the appropriate LDD scan
                                        #  processes to add paths if
                                        #  appropriate
.endif  # ifdef BACKEND
#
# --- Process alias node serial number.
#
        call    dlm$proc_alias_         # process any alias node defined
                                        #  in the MAG data
        ldconst 0,r4                    # r4 = VRP status
        b       .dlmMLE_910             # and we're out of here!
#
.dlmMLE_900:
        ldconst eccancel,r4             # r4 = VRP error status to return
.dlmMLE_910:
        stob    r4,vr_status(g13)       # Save error status
.ifdef BACKEND
        mov     g1,r11                  # save g1
        mov     g14,g1                  # g1 = ILT being completed
        call    K$comp                  # Complete this request
        mov     r11,g1                  # restore g1
.else   # FRONTEND
#
# --- Set the DTMT to initializing until the BE DLM ID is received
#       This prevents messages being sent on this path until all the
#       necessary information has been properly set up.
#
        ldconst dtmt_st_init,r8         # Show the DTMT as being initializing
        stob    r8,dtmt_state(g4)
#
# --- Queue request
#
        movq    g0,r8                   # Save g0-g3
        mov     g14,r7                  # Save g14
        mov     g13,r6                  # save g13
        mov     g14,g1                  # g1 = ILT to send to the Back end
#
# --- Check if the path is related to ICL port. In such case don't
#     create any DTMT path for this port in BE. Hence don't send
#     any message to BE
#
c       r7 =   ICL_IsIclMagLinkVRP((VRP *)g13);
        cmpobe TRUE,r7,.dlmMLE_icl10   # Jif not ICL Mag Link Req.
#
# --- Queue request
#
        st      g13,vrvrp(g14)          # vrvrp points to the VRP in the ILT
        lda     L$que,g0                # Call Link960 to send the request
        call    K$qwlink                # Wait for the response
#
# --- This sends the VRP (containing MAGDT=r5 in VRP) to the Back End. The
#     virtual layer in BackEnd,  in turn queues (DLM$vque) this VRP
#     to DLM which is processed by dlm$vrpx. The dlm$vrpx () in turn calls
#     this function(dlm$MLE) in back end and creates the DTMT. When the FE
#     gets back the control here, the magdt_dlmid(r5) contains the dtmt
#     created in Back End processor.  However in case of ICL, we are not
#     passing any request down to BE to create the DTMT. Hence in ICL
#     DTMT case, the magdt_dlmid(r5) contains the frontEnd DTMT that is
#     created now. This value is stored in dml_bedlmid of Front end DTMT(g4)
#     that is created now.
#     So, as far as ICL concerned , the dml_bedlmid member
#     of DTMT contains the same DTMT(front End) address ,created just now,
#     rather than the BackEnd DTMT  as in other cases (iSCSI/FE ports DTMTs).
#     This is because the ICL related DTMT need not be known to BackEnd, as
#     this is not supposed to be used by Vlink/copymanager I/O requests.
#     However this member (dml_bedlmid) needs to be non-NULL, as it is
#     being used by send_dg function to create and forward SRPs. Hence same
#     DTMT address has been set in dml_bedlmid member of DTMT in case of ICL.
#
# --- Save the BE DLM ID for later use by the FE Datagram requests
#
.dlmMLE_icl10:
        ld      magdt_dlmid(r5),r3      # r3 = DTMT address in MAGDT from BE
        mov     r4,g0                   # Set up the completion status
        st      r3,dml_bedlmid(g4)      # Save the BE DLM ID for later use
        ldconst dtmt_st_op,r4           # Set the DTMT as being operational
        mov     0,g13                   # Ensure g13 is zero on the FE
        stob    r4,dtmt_state(g4)       # Save the new DTMT state
        call    K$comp                  # Complete this request
#
# --- When  this requested is completed (sendMLE_cr),the sendMLE_cr()
#     function stores the DLM session session ID (magdt_dlmid) in the
#     LTMT structure. So later at any time when sending the Mag Link
#     Establish request again, the lld$send_MLE function takes the
#     DLM session ID from LTMT and sets this in the VRP(in vr_mle_dlmid)
#     and in MAGDT(magdt_dlmid). Except in ICL cases,in all cases  this
#     contain the the BackEnd DTMT (dlm session Id). Also this value from
#     LTMT is being used in lld$MSG while sending a datagram through send_dg
#
        movq    r8,g0                   # Restore g0-g3
        mov     r7,g14                  # Restore g14
        mov     r6,g13                  # Restore g13
.endif  # ifdef BACKEND
        movq    r12,g4                  # restore g4-g7
        ret
#
#**********************************************************************
#
#  NAME: dlm$MLT
#
#  PURPOSE:
#       Processes a XIOtech Controller link terminated VRP request.
#
#  DESCRIPTION:
#       Validates the target has been registered with the DLM and if
#       not valid simply ignores the request. If valid, calls the
#       target disappeared event handler routine associated with the
#       DTMT.
.ifdef BACKEND
#       It then returns completion status for the VRP request
#       to the requestor.
.else   # FRONTEND
#       It then forwards the VRP to the BE Processor for its processing needs.
.endif  # ifdef BACKEND
#
#  CALLING SEQUENCE:
#       call    dlm$MLT
#
#  INPUT:
#       g13 = VRP address to process
#       g14 = ILT associated with VRP at nest level 2
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
dlm$MLT:
        movq    g4,r12                  # save g4-g7
        ldob    -ILTBIAS+dlmi_path(g14),r4 # r4 = path #
        lda     dlm_lldmt_dir,r5        # r5 = base of LLDMT directory
        ld      (r5)[r4*4],g6           # g6 = LLDMT for this interface
        cmpobe  0,g6,.dlmMLT_900        # Jif no LLDMT defined for this
                                        #  interface
        ld      vr_mlt_dlmid(g13),g4    # g4 = DLM session ID from VRP
        cmpobe  0,g4,.dlmMLT_90         # Jif DLM session ID not specified
.ifdef FRONTEND
#
# --- Find the FE DTMT that matches the BE DTMT
#
                                        # g4 = BE DLM ID passed in
                                        # g6 = LLDMT to use to find FE DLM ID
        call    dlm$findbedlmid         # g4 = 0 or FE DLM ID that matches
        cmpobe  0,g4,.dlmMLT_90         # Jif no matching FE DLM session ID
.endif  # ifdef FRONTEND
#
# --- Validate association of DTMT with LLD session ID
#
        ld      dtmt_bnr(g4),r7         # r7 = DTMT banner value
        ld      dtmt_banner,r8          # r8 = DTMT banner pattern
        cmpobne r7,r8,.dlmMLT_900       # Jif DTMT not active
        ld      dtmt_lldmt(g4),r7       # r7 = DTMT assoc. LLDMT address
.ifdef M4_ADDITION
 cmpobe 0,r7,.dlmMLT_900 # Jump if DTMT does not exist.
.endif  # M4_ADDITION
        ldob    dlmi_path-ILTBIAS(r7),r8 # r8 = path # assoc. with DTMT
        cmpobne r4,r8,.dlmMLT_900       # Jif VRP came from different path #
                                        #  then assoc. with DTMT
        ld      vr_mlt_lldid(g13),r7    # r7 = link-level driver ID
        ld      dtmt_lldid(g4),r8       # r8 = LLDID from DTMT
        cmpobe  r7,r8,.dlmMLT_150       # Jif match found
        b       .dlmMLT_900             # ignore VRP request.
#
# --- Scan DTMT list for a match on the link-level driver ID
#
.dlmMLT_90:
        ld      vr_mlt_lldid(g13),r7    # r7 = link-level driver ID
        ld      lldmt_dtmthd(g6),g4     # g4 = first DTMT on list
        cmpobe  0,g4,.dlmMLT_900        # Jif no DTMTs on active list
.dlmMLT_100:
        ld      dtmt_lldid(g4),r8       # r8 = LLDID from DTMT
        cmpobe  r7,r8,.dlmMLT_150       # Jif match found
        ld      dtmt_link(g4),g4        # g4 = next DTMT on active list
        cmpobne 0,g4,.dlmMLT_100        # Jif more DTMTs to check
        b       .dlmMLT_900             # ignore. no match found.
#
.dlmMLT_150:
        ld      dtmt_lldmt(g4),g6       # g6 = assoc. LLDMT address
#
# --- Check if DTMT is active. If not, ignore request.
#
        ld      lldmt_dtmthd(g6),r4     # r4 = first DTMT on list
.dlmMLT_200:
        cmpobe  0,r4,.dlmMLT_900        # Jif DTMT not found on list
        cmpobe  g4,r4,.dlmMLT_300       # Jif DTMT found on list
        ld      dtmt_link(r4),r4        # r4 = next DTMT on list
        b       .dlmMLT_200             # and go check next DTMT on list
#
.dlmMLT_300:
        ld      dtmt_ehand(g4),r4       # r4 = DTMT event handler table
        ld      dtmt_eh_tgone(r4),r4    # r4 = target disappeared event
                                        #  handler routine
        callx   (r4)                    # call target disappeared event
                                        #  handler routine
.dlmMLT_900:
.ifdef BACKEND
        ldconst 0,r5                    # r5 = VRP status to return
        stob    r5,vr_status(g13)       # Save error status
        mov     g1,r11                  # save g1
        mov     g14,g1                  # g1 = ILT being completed
        call    K$comp                  # Complete this request
        mov     r11,g1                  # restore g1
.else   # FRONTEND
c       r7 =   ICL_IsIclMagLinkVRP((VRP *)g13);
        cmpobe  TRUE,r7,.dlmMLT_icl05   # Jif  ICL Mag Link Req.
        call    dlm$SBE                 # Forward the VRP to the BE processor
                                        # After BE processor execution this
                                        # request is completed.
        b       .dlmMLT_1000

.dlmMLT_icl05:
#
## Completing the request here without being sent to BackEnd.
## ICL change
#
        mov    g1,r11                  # save g1
        mov    g14,g1                  # g1= ILT being completed
        call   K$comp                  # complete this request
        mov    r11,g1                  # restore g1

.dlmMLT_1000:
.endif  # ifdef BACKEND
        movq    r12,g4                  # restore g4-g7
        ret
#
#**********************************************************************
#
#  NAME: dlm$MRC
#
#  PURPOSE:
#       Processes a datagram message received from XIOtech Cntr VRP request.
#
#  DESCRIPTION:
#       Validates the LLD session ID and if invalid returns a VRP
#       error completion status to the LLD. If valid, pulls the
#       request message buffer address/length and response message
#       buffer address/length out of the VRP and builds up a datagram
#       ILT to process the datagram request with. Gets the specified
#       server name from the datagram header and attempts to forward
#       the datagram message to the specified server. If the server
#       is not supported, returns the appropriate error response
#       message to the requestor. If the server is supported, it
#       calls the server handler routine with the datagram ILT.
#
#  CALLING SEQUENCE:
#       call    dlm$MRC
#
#  INPUT:
#       g13 = VRP address to process
#       g14 = ILT associated with VRP at nest level 2
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
dlm$MRC:
        movq    g0,r4                   # save g0-g3
        movq    g4,r8                   # save g4-g7
        ld      vr_mrm_sgl(g13),r12     # r12 = SGL address (FE local address)
!       ldos    sg_scnt(r12),r13        # r13 = SGL segment count
        lda     sghdrsiz(r12),r12       # r12 = first SGL segment
!       ldl     sg_addr(r12),g4         # g4 = request buffer address
                                        # g5 = request buffer length
        lda     ILTBIAS(g14),g1         # g1 = datagram ILT at nest level #3
        mov     0,r14
        lda     dss2_rqhdr(g14),g2      # g2 = local request header address
        stob    r14,vr_status(g13)      # save good VRP status
        lda     dss2_rshdr(g14),g3      # g3 = local response header address
        stl     g2,dss3_rqhdr_ptr(g1)   # save local header addresses in ILT
#        ld      il_misc(g14),g6         # Save the ILT PARMS pointer in the
#        st      g6,il_misc(g1)          #   next level of the ILT
c       ((ILT*)g1)->misc = ((ILT*)g14)->misc;
        movl    0,g6                    # clear response buffer address
                                        #  & length
        cmpobe  1,r13,.dlmMRC_100       # Jif no response buffer specified
!       ldl     sgdescsiz+sg_addr(r12),g6 # g6 = response buffer address
                                        # g7 = response buffer length
        ldconst dgrs_size,r12           # r12 = response header size
        addo    r12,g6,g6               # g6 = response buffer address not
                                        #  including header
        subo    r12,g7,g7               # g7 = response buffer length not
                                        #  including header
.dlmMRC_100:
!       ldq     (g4),r12                # r12-r15 = bytes 0-15 of request
                                        #  header
        stq     r12,(g2)                # copy request header into local
                                        #  request header area
!       ldq     16(g4),r12              # r12-r15 = bytes 16-31 of request
                                        #  header
        stq     r12,16(g2)
        ldconst dgrq_size,r12           # r12 = request header size
        addo    r12,g4,g4               # g4 = request buffer address not
                                        #  including header
        subo    r12,g5,g5               # g5 = request message length not
                                        #  including header
        stq     g4,dss3_rqbuf(g1)       # save request & response buffer
                                        #  addresses & lengths in ILT
        lda     dlm$MRC_iltcr,r12       # r12 = ILT completion handler routine
        st      r12,il_cr(g1)           # save ILT completion handler routine
                                        #  in ILT
        ldob    dgrq_srvcpu(g2),r12     # r12 = dest. server CPU code
.ifdef BACKEND
        ldconst dg_cpu_main,r13         # r13 = BE Processor code
        cmpobne r12,r13,.dlmMRC_400     # Jif not main CPU specified
.else   # FRONTEND
        ldconst dg_cpu_interface,r13    # r13 = FE Processor code
        cmpobe  r12,r13,.dlmMRC_280     # Jif this is for the FE Processor Code
        ldconst dg_cpu_mmc,r13          # r13 = CCB Processor code
        cmpobe  r12,r13,.dlmMRC_220     # Jif this is for the CCB
        call    dlm$SBE                 # send the request to the back end
        b       .dlmMRC_1000            #  to handle everything else
#
.dlmMRC_220:
#
# A message to the CCB has been received.  Forward it to the CCB.
#
        lda     ILTBIAS(g1),g1          # point to the next level of ILT
        ldconst drdlmtoccb,g0           # g0 = function code (DLM to CCB)
        ldconst dgrq_size,r12           # r12 = request header size
        subo    r12,g4,g4               # g4 = request address
                                        #  including header and data

        addo    r12,g5,g5               # g5 = request length
                                        #  including header and data
        cmpobe  0,g6,.dlmMRC_240        # Jif there is no response address
        ldconst dgrs_size,r12           # r12 = response header size
        subo    r12,g6,g6               # g6 = response address
                                        #  including header and data
        addo    r12,g7,g7               # g7 = response length
                                        #  including header and data
.dlmMRC_240:
        call    dlm$create_drp          # Create a DRP with all the necessary
                                        #   values to forward to the CCB
                                        # g0 = address of DRP
        lda     dlm$drp_comp,r12        # r12 = completion routine address
        st      r12,il_cr(g1)           # save this completion routine away
        mov     0,r3
        st      g0,vrvrp(g1)            # save the DRP in the ILT
        st      g13,il_w2(g1)           # save the original VRP in the ILT
        st      r3,il_fthd(g1)          # Close link
        st      g3,il_w1(g1)            # save the local response header
.if  DLMFE_DRIVER
        call    dlmt$dlmfe_que          # Queue the request to Debug Driver
.else   # No DLMFE_DRIVER
        mov     g14,r3                  # Save g14
        call    L$que                   # Send this to the CCB
                                        #   Destroys g0-g3 and g14
        mov     r3,g14                  # Restore g14
.endif  # DLMFE_DRIVER
        b       .dlmMRC_1000            # Done!
.dlmMRC_280:
.endif  # ifdef BACKEND
#
# A message for this processor has been received
#
        ld      dgrq_srvname(g2),r12    # r12 = specified server name
        lda     DLM_servers,r13         # r13 = supported server table
.dlmMRC_300:
        ld      (r13),r14               # r14 = server name from table
        cmpobe  0,r14,.dlmMRC_400       # Jif no more servers in table
        cmpobe  r12,r14,.dlmMRC_500     # Jif name matches
        addo    8,r13,r13               # inc. to next entry in table
        b       .dlmMRC_300             # and check next entry in table
#
.dlmMRC_400:
        mov     0,g0                    # set request received OK (no logging)
        ldq     dlm$ddsp_nosrvr,r12     # r12-r15 = error response header
        stq     r12,(g3)                # save response header
        ld      il_cr(g1),r12           # r12 = ILT completion handler routine
        callx   (r12)                   # and go through normal ILT completion
                                        #  handler routine
        b       .dlmMRC_1000            # and we're out of here!
#
.dlmMRC_500:
        ld      4(r13),r14              # r14 = service handler routine
        mov     g13,r15                 # save g13
        mov     0,g13                   # zero g13 for called functions
        lda     ILTBIAS(g1),g1          # g1 = datagram ILT at nest level 4
#
#******************************************************************************
#
#       Interface to datagram service handler routines.
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
#       Regs. g0-g7 can be destroyed.
#
#******************************************************************************
#
        callx   (r14)                   # and call service handler routine
        mov     r15,g13                 # restore g13
.dlmMRC_1000:
        movq    r4,g0                   # restore g0-g3
        movq    r8,g4                   # restore g4-g7
        ret
#
#**********************************************************************
#
#  NAME: dlm$MRC_iltcr
#
#  PURPOSE:
#       Processes a datagram ILT completion event.
#
#  DESCRIPTION:
#       Completes the response header and copies it into the response
#       message buffer if one exists. Adjusts the ILT pointer to nest
#       level 1 and returns the ILT/VRP back to the original caller.
#
#  CALLING SEQUENCE:
#       call    dlm$MRC_iltcr
#
#  INPUT:
#       g1 = datagram ILT at nest level 3
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
dlm$MRC_iltcr:
        movq    g0,r12                  # save g0-g3
        ldl     dss3_rqhdr_ptr(g1),r4   # r4 = local request header address
                                        # r5 = local response header address
        ld      dss3_rsbuf(g1),r6       # r6 = response buffer address not
                                        #  including header
        cmpobe  0,r6,.MRCiltcr_200      # Jif no response buffer address
        ldos    dgrq_seq(r4),r8         # r8 = request message sequence #
        stos    r8,dgrs_seq(r5)         # save sequence # in response header
        ldq     (r5),r8                 # r8-r11 = response message header
        lda     -dgrs_size(r6),r6       # r6 = response buffer address at
                                        #  response header
!       stq     r8,(r6)                 # save response message header in
                                        #  response buffer
.MRCiltcr_200:
        lda     -2*ILTBIAS(g1),g1       # back ILT up to nest level 1
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT completion handler routine
        movq    r12,g0                  # restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: dlm$retrydg
#
#  PURPOSE:
#       To provide a means of retrying datagram requests periodically.
#
#  DESCRIPTION:
#       Takes any datagram request messages off the datagram retry
#       queue and resends them after the delay timeout period has
#       expired. The datagram request messages have been set up
#       already and are simply resent.
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
dlm$retrydg:
.retrydg_10:
        ldconst retrydg_to,g0           # g0 = time delay period (in msec.)
        call    K$twait                 # wait for time delay
        movl    0,r4                    # r4-r5 = 0
        ld      dlm_rtydg_qu,r6         # r6 = first datagram on queue
# Do not need the tail for our loop.
        stl     r4,dlm_rtydg_qu         # clear queue pointers
.retrydg_100:
        cmpobe  0,r6,.retrydg_10        # Jif no datagrams to resend
        lda     ILTBIAS(r6),g1          # g1 = datagram ILT at nest level 3
        ld      il_fthd(r6),r6          # r6 = next datagram message on list
#
# --- Find a path to the other controller that has not been tried yet
#       Set paths just tried in the retried fields
#
        ld      dsc3_mlmt(g1),r3        # r3 = MLMT associated with this path
        ld      dsc3_dtmt(g1),r10       # r10 = pair of paths just used
        cmpobe  0,r3,.retrydg_900       # Jif no MLMT is specified yet
                                        #  (DLM$just_senddg and datagram to
                                        #   self (BE -> FE))
        cmpobe  0,r10,.retrydg_900      # Jif no previous DTMT used
                                        #  (DLM$just_senddg will not have
                                        #  set up yet)
        ldob    dsc3_thispaths(g1),r4   # r4 = paths tried already this cntrl
        ldob    dsc3_otherpaths(g1),r5  # r5 = paths tried on other controller
? # crash - cqt# 22171 - FE DTMT - failed @ dlm.as:2808  ld 8+r10,r14 with dafedafe - workaround?
        ld      dtmt_lldmt(r10),r14     # r14 = LLDMT pointer
        cmpobe  0,r14,.retrydg_900      # If DTMT has been freed.
        ldob    dml_path(r10),r15       # r15 = just tried path on other cntrl
        ldob    lldmt_channel(r14),r14  # r14 = just tried path on this cntrl
        setbit  r15,r5,r5               # Show this path tried already other cnt
        setbit  r14,r4,r4               # Show this path tried already this cntr
        stob    r5,dsc3_otherpaths(g1)  # Save the new retried path fields for
        stob    r4,dsc3_thispaths(g1)   #  this and the other controller
#
#   r4 = Paths tried already on this controller
#   r5 = Paths tried already on the other controller
#   r7 = Test Path
#   r8 = This controllers best channel from Best path
#   r9 = Other controllers best channel from Best Path
#   r10 = Best path so far
#   r11 = Test path channel (used for both this and other controllers channel)
#   r14 = This controllers channel from last path (DTMT)
#   r15 = Other controllers channel from last path (DTMT)
#
#       Find the next operational path to use from this controller
#           Find path not tried yet
#           If all paths have been tried, pick the next in order
#               if Testpath = Lastpath
#                 then ignore
#                 else if Bestpath = Lastpath
#                   then Bestpath = Testpath
#                   else if Bestpath > Lastpath
#                       if ((Testpath < Lastpath) or (Testpath >= Bestpath))
#                          then ignore
#                          else Bestpath = Testpath
#                   else if ((Testpath > Lastpath) or (Testpath < Bestpath))
#                           then Bestpath = Testpath
#                           else ignore
#
        ld      mlmt_dtmthd(r3),r3      # r3 = Head DTMT for this MLMT
        mov     r3,r7                   # r7 = DTMT being tested
        mov     r14,r8                  # r8 = Set up path to use if not found
        mov     r15,r9                  # r9 = Set up path to use if not found
.retrydg_200:
        cmpobe  0,r7,.retrydg_300       # Jif no more DTMTs to check
        ld      dtmt_lldmt(r7),r11      # r11 = LLDMT associated with path
        ldob    dtmt_state(r7),r12      # r12 = State of this path
        ldob    lldmt_channel(r11),r11  # r11 = this controllers path (Testpath)
        cmpobne dtmt_st_op,r12,.retrydg_290 # Jif this path is not operational
        bbs     r11,r4,.retrydg_220     # Jif this path already tested
        mov     r7,r10                  # r10 = Best DTMT found so far
        mov     r11,r8                  # r8 = This path is the one to use
                                        #  for this controller
        b       .retrydg_300            # Go find other controllers best path
#
.retrydg_220:
        cmpobe  r11,r14,.retrydg_290    # Jif Testpath = Lastpath (ignore)
        cmpobe  r8,r14,.retrydg_280     # Jif Bestpath = Lastpath (Best=Test)
        bg      .retrydg_240            # Jif Bestpath > Lastpath
        cmpobg  r11,r14,.retrydg_280    # Jif Testpath > Lastpath (Best=Test)
        cmpobl  r11,r8,.retrydg_280     # Jif Testpath < Bestpath (Best=Test)
        b       .retrydg_290            # Go check the next DTMT (else ignore)
#
.retrydg_240:
        cmpobl  r11,r14,.retrydg_290    # Jif Testpath < Lastpath (ignore)
        cmpobge r11,r8,.retrydg_290     # Jif Testpath >= Bestpath (ignore)
.retrydg_280:
        mov     r7,r10                  # Best DTMT found so far, it is in the
        mov     r11,r8                  #  next path order for this controller
                                        #  (Bestpath = Testpath)
.retrydg_290:
        ld      dml_mllist(r7),r7       # Get the next DTMT in the list
        b       .retrydg_200            # Keep looking for the best DTMT to use
#
#       Find the next operational path to use on the other controller
#           Find path not tried yet and has this controllers picked path
#           If all paths have been tried, pick the next in order
#               if Testpath = Lastpath
#                 then ignore
#                 else if Bestpath = Lastpath
#                   then Bestpath = Testpath
#                   else if Bestpath > Lastpath
#                       if ((Testpath < Lastpath) or (Testpath >= Bestpath))
#                          then ignore
#                          else Bestpath = Testpath
#                   else if ((Testpath > Lastpath) or (Testpath < Bestpath))
#                           then Bestpath = Testpath
#                           else ignore
#
.retrydg_300:
        ldob    dml_path(r10),r11       # Quick Test - Get the picked DTMT
        bbc     r11,r5,.retrydg_400     # Jif path has not been tested (Found
                                        #  the best pair of paths to use)
        mov     r3,r7                   # r7 = Begin at the Head DTMT
.retrydg_320:
        cmpobe  0,r7,.retrydg_400       # Jif no more DTMTs to check
        ld      dtmt_lldmt(r7),r11      # r11 = LLDMT associated with path
        ldob    dtmt_state(r7),r12      # r12 = State of this path
        ldob    lldmt_channel(r11),r11  # r11 = this controllers path
        cmpobne dtmt_st_op,r12,.retrydg_390 # Jif this path is not operational
        cmpobne r11,r8,.retrydg_390     # Jif this DTMT is not for the picked
                                        #  path on this controller
        ldob    dml_path(r7),r11        # r11 = Test path on other controller
        bbs     r11,r5,.retrydg_340     # Jif this path has been tried before
        mov     r7,r10                  # r10 = the DTMT to use
        b       .retrydg_400            # Go test this DTMT pair of paths
#
.retrydg_340:
        cmpobe  r11,r15,.retrydg_390    # Jif Testpath = Lastpath (ignore)
        cmpobe  r9,r15,.retrydg_380     # Jif Bestpath = Lastpath (Best=Test)
        bg      .retrydg_360            # Jif Bestpath > Lastpath
        cmpobg  r11,r15,.retrydg_380    # Jif Testpath > Lastpath (Best=Test)
        cmpobl  r11,r9,.retrydg_380     # Jif Testpath < Bestpath (Best=Test)
        b       .retrydg_390            # Go check the next DTMT (else ignore)
#
.retrydg_360:
        cmpobl  r11,r15,.retrydg_390    # Jif Testpath < Lastpath (ignore)
        cmpobge r11,r9,.retrydg_390     # Jif testpath >= Bestpath (ignore)
.retrydg_380:
        mov     r7,r10                  # Best DTMT found so far, is is in the
        mov     r11,r9                  #  next path order for the other
                                        #  controller (Bestpath = Testpath)
.retrydg_390:
        ld      dml_mllist(r7),r7       # Get the next DTMT in the list
        b       .retrydg_320            # Keep looking for the best DTMT to use
#
#       DTMT found, set up DG to use this new DTMT
#
.retrydg_400:
        ld      dsc3_dtmt(g1),r11       # r11 = original (last) DTMT used
        mov     0,r12                   # r12 = zero (prep for clearing)
        cmpobe  r11,r10,.retrydg_900    # Jif same (do normal round robin retry)
        st      r10,dsc3_reqdtmt(g1)    # Save this best DTMT as the one to use
        st      r12,dsc3_dtmt(g1)       # Clear old DTMT used
#
# --- Send the Datagram after finding the next path to use
#
.retrydg_900:
        call    DLM$send_dg             # send out datagram message again
        b       .retrydg_100            # and check for more datagrams to
                                        #  resend
#
#******************************************************************************
#
# ____________________ DTMT EVENT HANDLER TABLES ______________________________
#
#******************************************************************************
#
# --- XIOtech Controller target event handler table -------------------------
#
        .data
dtmt_etbl1:
        .word   dlm$term_ML             # Target disappeared event
#
        .text
#
#******************************************************************************
#
# ____________________ DTMT EVENT HANDLER ROUTINES ____________________________
#
#******************************************************************************
#
# --- XIOtech Controller Link target disappeared event handler routine ------
#
#******************************************************************************
#
#  NAME:  dlm$term_ML
#
#  PURPOSE:
#       Processes a XIOtech Controller Link disappeared event.
#
#  DESCRIPTION:
#       Demolishes all associated TPMTs (if applicable), disassociates the
#       DTMT with the MLMT and then terminates the DTMT associated with
#       the target.
#
#  CALLING SEQUENCE:
#       call    dlm$term_ML
#
#  INPUT:
#       g4 = assoc. DTMT of target being terminated
#       g6 = assoc. LLDMT address for DTMT
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
dlm$term_ML:
        movq    g0,r12                  # save g0-g3
        mov     g4,g1                   # g1 = DTMT address
.ifdef  BACKEND
        ldos    dtmt_sulindx(g4),r4     # r4 = index into storage unit list
        cmpobe  0,r4,.termML_200        # Jif not listed in storage unit list
        ld      vl_sulst,r5             # r5 = base address of storage unit
                                        #  list
        addo    r4,r5,r5                # r5 = pointer into storage unit list
                                        #  where this target is registered
        mov     0,r6
        st      r6,vl_sulr_dtmt(r5)     # clear assoc. DTMT from SUL record
        stos    r6,dtmt_sulindx(g4)     # clear index field in DTMT
#
# --- Demolish all associated TPMTs
#
.termML_200:
        ld      dtmt_tpmthd(g1),g3      # g3 = first TPMT on list
        cmpobe  0,g3,.termML_300        # Jif no TPMTs on list
        ldob    tpm_state(g3),r4        # r4 = TPMT state
        ld      tpm_ldd(g3),g0          # g0 = assoc. LDD address
        cmpobne tpm_st_op,r4,.termML_220 # Jif path not in operational state
        ldconst tpm_st_notop,r4         # r4 = path not operational state code
        stob    r4,tpm_state(g3)        # save new TPMT state
.termML_220:
        call    DLM$dem_path            # demolish this path
#         call    dlm$rem_tpmt            # remove TPMT from LDD & DTMT
# c       put_tpmt(g3);                   # Deallocate TPMT
        call    DLM$VLreopen            # schedule VLink open process to
                                        #  re-establish VLink and paths
        b       .termML_200             # check for more TPMTs to demolish
#
# --- Demolish DTMT
#
.termML_300:
        call    dlm$sched_lddx          # schedule LDD scan process if
                                        #  necessary
.endif  # ifdef BACKEND
.if ICL_DEBUG
        ldob    dtmt_icl(g4),r5
        cmpobne TRUE,r5,.termML_icl05
c fprintf(stderr,"%s%s:%u <dlm$term_ML>ICL..calling term_alias_dtmt() for  DTMT =%x\n", FEBEMESSAGE, __FILE__, __LINE__,(UINT32)g4);
.termML_icl05:
.endif  # ICL_DEBUG
        call    dlm$term_alias_dtmt     # terminate any associated alias DTMT
.if ICL_DEBUG
        cmpobne TRUE,r5,.termML_icl08
c fprintf(stderr,"%s%s:%u <dlm$term_ML>ICL..removing DTMT =%x\n", FEBEMESSAGE, __FILE__, __LINE__,(UINT32)g4);
.termML_icl08:
.endif  # ICL_DEBUG
        call    dlm$rem_dtmt            # remove DTMT from LLDMT & MLMT
.ifdef M4_DEBUG_DTMT
c fprintf(stderr, "%s%s:%u put_dtmt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g4);
.endif # M4_DEBUG_DTMT
c       put_dtmt(g4);                   # deallocate DTMT
        movq    r12,g0                  # restore g0-g3
        ret
#
#******************************************************************************
# _______________________ DLM Server Handler Routines _________________________
#
#******************************************************************************
#
#******************************************************************************
#
#  NAME:  DLM$srvr_invfc
#
#  PURPOSE:
#       Pack and return destination server/invalid function code response
#       to requestor.
#
#  DESCRIPTION:
#       Packs the destination server/invalid function code response header
#       in the local response header and returns the datagram ILT back to the
#       requestor.
#
#  CALLING SEQUENCE:
#       call    DLM$srvr_invfc
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
DLM$srvr_invfc:
        mov     g0,r12                  # save g0
        mov     0,g0                    # set request received OK (no logging)
        ldq     dlm$srvr_invfc,r4       # r4-r7 = response header for invalid
                                        #  function code
        stq     r4,(g3)                 # save response header in memory
        lda     -ILTBIAS(g1),g1         # g1 = previous ILT nest level
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT completion handler routine
        mov     r12,g0                  # restore g0
        ret
#
#******************************************************************************
#
#  NAME:  DLM$srvr_invparm
#
#  PURPOSE:
#       Pack and return destination server/invalid parameter response
#       to requestor.
#
#  DESCRIPTION:
#       Packs the destination server/invalid parameter response header
#       in the local response header and returns the datagram ILT back
#       to the requestor.
#
#  CALLING SEQUENCE:
#       call    DLM$srvr_invparm
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
DLM$srvr_invparm:
        mov     g0,r12                  # save g0
        mov     0,g0                    # set request received OK (no logging)
        ldq     dlm$srvr_invparm,r4     # r4-r7 = response header for invalid
                                        #  parameter
        stq     r4,(g3)                 # save response header in memory
        lda     -ILTBIAS(g1),g1         # g1 = previous ILT nest level
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT completion handler routine
        mov     r12,g0                  # restore g0
        ret
#
#******************************************************************************
#
#  NAME:  DLM$srvr_srconflt
#
#  PURPOSE:
#       Pack and return destination server/server access conflict
#       response to requestor.
#
#  DESCRIPTION:
#       Packs the destination server/server access conflict response header
#       in the local response header and returns the datagram ILT back
#       to the requestor.
#
#  CALLING SEQUENCE:
#       call    DLM$srvr_srconflt
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
.ifdef BACKEND
DLM$srvr_srconflt:
        mov     g0,r12                  # save g0
        mov     0,g0                    # set request received OK (no logging)
        ldq     dlm$srvr_srconflt,r4    # r4-r7 = response header for server
                                        #  access conflict
        stq     r4,(g3)                 # save response header in memory
        lda     -ILTBIAS(g1),g1         # g1 = previous ILT nest level
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT completion handler routine
        mov     r12,g0                  # restore g0
        ret
.endif /* BACKEND */
#
#******************************************************************************
#
#  NAME:  DLM$srvr_reroute
#
#  PURPOSE:
#       Pack and return re-route datagram to specified controller
#       response to requestor.
#
#  DESCRIPTION:
#       Packs the re-route datagram to specified controller response header
#       in the local response header, saves the specified controller serial
#       number in the response and returns the datagram ILT back
#       to the requestor.
#
#  CALLING SEQUENCE:
#       call    DLM$srvr_reroute
#
#  INPUT:
#       g0 = controller serial number to reroute datagram to
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
.ifdef BACKEND
DLM$srvr_reroute:
        mov     g0,r12                  # save g0
                                        # r12 = controller serial number to
                                        #      reroute datagram to
        mov     0,g0                    # set request received OK (no logging)
        ldq     dlm$srvr_reroute,r4     # r4-r7 = response header for reroute
                                        #  datagram response
        bswap   r12,r5                  # r5 = controller serial # in
                                        #      big-endian format
        stq     r4,(g3)                 # save response header in memory
        lda     -ILTBIAS(g1),g1         # g1 = previous ILT nest level
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT completion handler routine
        mov     r12,g0                  # restore g0
        ret
.endif /* BACKEND */
#
#******************************************************************************
# ____________________________ SUBROUTINES ____________________________________
#******************************************************************************
#
#  NAME:  dlm$chk4dtmt
#
#  PURPOSE:
#       Checks for a DTMT on the active DTMT list that matches the
#       specified port WWN and node WWN.
#
#  DESCRIPTION:
#       Scans the active DTMT list in the LLDMT for a match with the
#       specified port WWN & node WWN and if found returns the
#       matching DTMT address.
#
#  CALLING SEQUENCE:
#       call    dlm$chk4dtmt
#
#  INPUT:
#       g4-g5 = port WWN to match
#       g6 = assoc. LLDMT to scan
#       g8-g9 = node WWN to match
#
#  OUTPUT:
#       g4 = DTMT address if match found
#       g4 = 0 if no match found
#
#  REGS DESTROYED:
#       Reg. g4 destroyed.
#
#******************************************************************************
#
dlm$chk4dtmt:
        movl    g4,r8                   # r8-r9 = MAC address to match
        ld      lldmt_dtmthd(g6),g4     # g4 = first DTMT on active list
        cmpobe  0,g4,.chk4dtmt_1000     # Jif no DTMTs on active list
.chk4dtmt_100:
        ldl     dtmt_pwwn(g4),r6        # r6-r7 = DTMT port WWN
        cmpobne r8,r6,.chk4dtmt_150     # Jif port WWN doesn't match
        cmpobne r9,r7,.chk4dtmt_150     # Jif port WWN doesn't match
        ldl     dtmt_nwwn(g4),r6        # r6-r7 = DTMT node WWN
        cmpobne g8,r6,.chk4dtmt_150     # Jif node WWN doesn't match
        cmpobe  g9,r7,.chk4dtmt_1000    # Jif node WWN matches
.chk4dtmt_150:
        ld      dtmt_link(g4),g4        # g4 = next DTMT on list
        cmpobne 0,g4,.chk4dtmt_100      # Jif more DTMTs to check
.chk4dtmt_1000:
        ret
#
#******************************************************************************
#
#  NAME: dlm$rem_dtmt
#
#  PURPOSE:
#       Removes the DTMT from the active DTMT list in the specified LLDMT if
#       found on list. Also removes the DTMT from the associated MLMT
#       if needed.
#
#  DESCRIPTION:
#       Goes through the active DTMT list in the LLDMT trying to find the
#       DTMT and if found removes it from the list. If DTMT is a
#       XIOtech Controller link, finds associated MLMT and removes DTMT
#       from MLMT list if found.
#
#  CALLING SEQUENCE:
#       call    dlm$rem_dtmt
#
#  INPUT:
#       g4 = DTMT address to remove from list
#       g6 = assoc. LLDMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
dlm$rem_dtmt:
        ld      lldmt_dtmthd(g6),r4     # r4 = first DTMT on list
        cmpobe  0,r4,.remdtmt_200       # Jif no DTMTs on list
        cmpobne g4,r4,.remdtmt_100      # Jif not the first on list
        ld      dtmt_link(r4),r5        # r5 = next DTMT on list
        st      r5,lldmt_dtmthd(g6)     # save new head member
        cmpobne 0,r5,.remdtmt_200       # Jif not the last member on list
        st      r5,lldmt_dtmttl(g6)     # clear tail member
        b       .remdtmt_200
#
.remdtmt_100:
        mov     r4,r5                   # r5 = previous DTMT address
        ld      dtmt_link(r4),r4        # r4 = next DTMT on list
        cmpobe  0,r4,.remdtmt_200       # Jif no more DTMTs on list
        cmpobne g4,r4,.remdtmt_100      # Jif not a match
        ld      dtmt_link(r4),r6        # r6 = next DTMT on list
        st      r6,dtmt_link(r5)        # remove DTMT from list
        cmpobne 0,r6,.remdtmt_200       # Jif not the last on list
        st      r5,lldmt_dtmttl(g6)     # save new tail member
.remdtmt_200:
        ldob    dtmt_type(g4),r4        # r4 = target type code
        cmpobne dtmt_ty_MAG,r4,.remdtmt_1000 # Jif not a MAGNITUDE link
.ifdef FRONTEND
#
# --- Send message to CCB about path being lost to the XIOtech Controller if
#       it has not already been reported by the FE DLM HeartBeats.  If the
#       controller is having polling done and the Path is "Not Operational",
#       then the Message has already been sent.  If not, then a "Lost Path"
#       message needs to be sent.  Also, do not send the "Lost Path" message
#       if the Port is down (the ISP Layer has already reported the problem)
#
        ld      dml_mlmt(g4),r4         # r4 = assoc. MLMT address
        cmpobe  0,r4,.remdtmt_260       # Jif no MLMT associated and report loss
        ldob    mlmt_flags(r4),r5       # r5 = Flags byte of MLMT - Polling?
        bbc     MLMT_POLL_PATH,r5,.remdtmt_260 # Jif Not Polling to this Cntrl
        ldob    dtmt_state(g4),r9       # r9 = State of the DTMT
        cmpobe  dtmt_st_notop,r9,.remdtmt_280 # Jif Lost Path Msg already sent
.remdtmt_260:
        ld      dml_alias(g4),r8        # r8 = assoc. alias serial number
.if ICL_DEBUG
        ldob    dtmt_icl(g4),r4
        cmpobe  FALSE,r4,.remdtmt_icl01
c fprintf(stderr,"%s%s:%u <rem_dtmt>ICL.. DTMT = %x assoc alias serial no=%x\n", FEBEMESSAGE, __FILE__, __LINE__,(UINT32)g4,(UINT32)r8);
.remdtmt_icl01:
.endif  # ICL_DEBUG
        cmpobne.f 0,r8,.remdtmt_280     # Jif primary DTMT with an alias
                                        #  defined
        ld      dtmt_lldmt(g4),r8       # r8 = LLDMT associated with DTMT
        ld      dml_sn(g4),r4           # r4 = XIOtech Controller serial #
        ldob    dml_path(g4),r5         # r5 = XIOtech Controller path #
        ldob    dml_cl(g4),r6           # r6 = XIOtech Controller cluster #
        ldob    lldmt_channel(r8),r9    # r9 = This controllers path
.if FE_ICL
        ldob    dtmt_icl(g4),r3         # Get ICL flag
        cmpobe  TRUE,r3,.remdtmt_icl05  # for ICL ,forcibly sending log message.
.endif  # FE_ICL

        PushRegs                        # Save all G registers (stack relative)
        mov     r9,g0                   # g0 = Port being interrogated
        call    ISP_IsReady             # g0 = TRUE if the Port is ready
                                        #    = FALSE if the Port is not ready
        mov     g0,r7                   # r7 = Port Status
        PopRegsVoid                     # Restore all G registers (stack relative)
        cmpobe  FALSE,r7,.remdtmt_280   # Jif Port is not ready (do not report
                                        #  the error - already done by ISP)
.if FE_ICL
.remdtmt_icl05:
.endif  # FE_ICL
        ldconst mlelostpathdebug,r7     # r7 = Lost an established path message
        ldconst 0xff,r8                 # r8 = Show Lost Path due to demolish
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       r10 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        st      r4,log_dlm_ccb_path_sn(r10) # save other serial # for CCB
        stob    r5,log_dlm_ccb_path_other_path(r10) # save other path # for CCB
        stob    r6,log_dlm_ccb_path_cl(r10) # save other cluster # for CCB
        stos    r7,mle_event(r10)       # save the message type
        stob    r9,log_dlm_ccb_path_this_path(r10) # save this controllers path number

        /*
        ** Add a flag if the path is of ICL type.
        */
        ldob    dtmt_icl(g4),r7
c       ICL_NotifyIclPathLost((void *)g4);
.if ICL_DEBUG
        cmpobe  FALSE,r7,.remdtmt_icl10
c fprintf(stderr,"%s%s:%u <dlm$remdtmt>ICL sending ICL path lost message\n", FEBEMESSAGE, __FILE__, __LINE__);
.remdtmt_icl10:
.endif  # ICL_DEBUG
        stob    r7,log_dlm_ccb_path_icl_flag(r10) # Set ICL path flag

        stob    r8,log_dlm_ccb_path_dg_status(r10) # Show status as path disappeared
        stob    r8,log_dlm_ccb_path_dg_ec1(r10)
        stob    r8,log_dlm_ccb_path_dg_ec2(r10)
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], log_dlm_ccb_lost_path_size);
#                             (event, controllerSN, opath, cluster, tpath, iclPathFlag);
        PushRegs(r8)
c       DL_add_delayed_message(mlelostpathwarn, r4, r5, r6, r9, r7);
        PopRegsVoid(r8)
#
.remdtmt_280:
.endif  # ifdef FRONTEND
        ld      dml_mlmt(g4),r4         # r4 = assoc. MLMT address
        cmpobe  0,r4,.remdtmt_400       # Jif no MLMT associated
        ld      mlmt_dtmthd(r4),r5      # r5 = first DTMT on MLMT list
        cmpobe  0,r5,.remdtmt_400       # JIf no DTMTs on list
        cmpobne g4,r5,.remdtmt_300      # Jif not the first on list
        ld      dml_mllist(g4),r6       # r6 = next DTMT on list
        st      r6,mlmt_dtmthd(r4)      # save new list head member
        cmpobne 0,r6,.remdtmt_400       # Jif not the last on list
        st      r6,mlmt_dtmttl(r4)      # clear list tail pointer
.ifdef FRONTEND
#
# --- Send message to CCB about all paths being lost to the XIOtech Controller
#       if the controller is not being polled.  If it is polled and the "Lost
#       All Paths" message has not been sent, then send the message.  Unless
#       all the Ports are down, in which case, do not send the message (ISP has
#       already handled this).  Also do not report if the MLMT is the Group
#       (DSC, CNC, VCG) (since not a real controller).
#
        ldob    mlmt_flags(r4),r5       # r5 = Flags byte of MLMT
        bbs     MLMT_CONTROLLER_GROUP,r5,.remdtmt_400 # Jif Group MLMT (no log)
        bbc     MLMT_POLL_PATH,r5,.remdtmt_290 # Jif not Polling to this Cntrl
        bbs     MLMT_LOST_ALL_SENT,r5,.remdtmt_400 # Jif reported already
.remdtmt_290:
        ld      dtmt_lldmt(g4),r8       # r8 = LLDMT associated with DTMT
        ld      dml_sn(g4),r4           # r4 = XIOtech Controller serial #
        ldob    dml_path(g4),r5         # r5 = XIOtech Controller path #
        ldob    dml_cl(g4),r6           # r6 = XIOtech Controller cluster #
        ldob    lldmt_channel(r8),r9    # r9 = This controllers path
        PushRegs                        # Save all G registers (stack relative)
        ldconst 0xFF,g0                 # g0 = Are any ports up
        call    ISP_IsReady             # g0 = FALSE if no ports are up
        mov     g0,r7                   # r7 = Port Status
        PopRegsVoid                     # Restore all G registers (stack relative)
        cmpobe  FALSE,r7,.remdtmt_400   # Jif All Ports are not ready (do not
                                        #  report the error - done by ISP)
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       r10 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        st      r4,log_dlm_ccb_path_sn(r10) # save serial # for CCB
        stob    r5,log_dlm_ccb_path_other_path(r10) # save other controllers path #
        stob    r6,log_dlm_ccb_path_cl(r10) # save assigned cluster # for CCB
        stob    r9,log_dlm_ccb_path_this_path(r10) # save this controllers path number
        ldconst mlelostallpaths,r7      # Lost all paths to a controller message
        stos    r7,mle_event(r10)       # save the message type
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], log_dlm_ccb_lost_path_size);
.endif  # ifdef FRONTEND
        b       .remdtmt_400
#
.remdtmt_300:
        mov     r5,r6                   # r6 = previous DTMT on list
        ld      dml_mllist(r5),r5       # r5 = next DTMT on list
        cmpobe  0,r5,.remdtmt_400       # Jif no more DTMTs on list
        cmpobne g4,r5,.remdtmt_300      # Jif not a match
        ld      dml_mllist(g4),r5       # r5 = next DTMT on list after
                                        #  specified DTMT
        st      r5,dml_mllist(r6)       # remove DTMT from list
        cmpobne 0,r5,.remdtmt_400       # Jif not the last on list
        st      r6,mlmt_dtmttl(r4)      # save new list tail member
.remdtmt_400:
        ldconst 0,r5
        st      r5,dml_mlmt(g4)         # clear MLMT address from DTMT
        st      r5,dml_mllist(g4)       # clear MLMT link list field
.remdtmt_1000:
        mov     0,r6
        st      r6,dtmt_link(g4)        # clear link field in DTMT
        ret
#
#******************************************************************************
#
#  NAME: dlm$upmlmt
#
#  PURPOSE:
#       Associates the DTMT with a MLMT.
#
#  DESCRIPTION:
#       Called only for XIOtech Controller link type DTMTs. Checks if a MLMT
#       already exists for the associated XIOtech Controller and if not
#       allocates an MLMT, links it to the MLMT list and links the DTMT to the
#       MLMT. If a MLMT exists for the associated XIOtech Controller, links the
#       DTMT to the MLMT/DTMT list.
#
#  CALLING SEQUENCE:
#       call    dlm$upmlmt
#
#  INPUT:
#       g4 = DTMT address associated with XIOtech Controller link
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
dlm$upmlmt:
        mov     g0,r12                  # save g0
        ld      dml_sn(g4),r11          # r11 = XIOtech Cntr serial # being
                                        #  added
        mov     r11,g0                  # g0 = Find Controller Serial Number
        call    DLM$find_controller     # Find Controller based on Serial Number
                                        # g0 = MLMT of found controller
        cmpobne 0,g0,.upmlmt_500        # Jif an MLMT was found
c       g0 = get_mlmt();                # Allocate a MLMT for this XIOtech Cntr
.ifdef M4_DEBUG_MLMT
c fprintf(stderr, "%s%s:%u get_mlmt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif # M4_DEBUG_MLMT
        st      r11,mlmt_sn(g0)         # save MAGNITUDE serial # in MLMT
        ld      dlm_mlmttl,r4           # r4 = MLMT list tail member
        cmpobne 0,r4,.upmlmt_350        # Jif list not empty
        st      g0,dlm_mlmthd           # save MLMT as list head member
        b       .upmlmt_360
#
.upmlmt_350:
        st      g0,mlmt_link(r4)        # link new MLMT onto end of list
.upmlmt_360:
        st      g0,dlm_mlmttl           # save new list tail member
.upmlmt_500:
#
# --- Check if DTMT already associated with MLMT
#
        ld      dml_mlmt(g4),r5         # r5 = assoc. MLMT address from DTMT
        cmpobne 0,r5,.upmlmt_1000       # Jif DTMT already assoc. with MLMT
#
        st      g0,dml_mlmt(g4)         # save MLMT address in DTMT
        ldconst 0,r5
        st      r5,dml_mllist(g4)
        ld      mlmt_dtmttl(g0),r4      # r4 = last DTMT on list
        cmpobne 0,r4,.upmlmt_550        # Jif list not empty
        st      g4,mlmt_dtmthd(g0)      # save DTMT as list head member
        b       .upmlmt_560
#
.upmlmt_550:
        st      g4,dml_mllist(r4)       # link new DTMT onto end of list
.upmlmt_560:
        st      g4,mlmt_dtmttl(g0)      # save new list tail member
#
.ifdef BACKEND
        cmpobne 0,r4,.upmlmt_600        # Jif list not empty
#
# --- MLMT list was empty. If the group master controller, need to send
#       a Group Master Controller Definition datagram to it.
#
        ldos    K_ii+ii_status,r4       # r4 = current initialization status
        bbc     iimaster,r4,.upmlmt_600 # Jif not the master controller
        movt    g0,r8                   # save g0-g2
        ld      mlmt_sn(g0),g1          # g1 = controller serial #
        ld      K_ficb,r6               # r6 = FICB address
        ld      fi_vcgid(r6),g0         # g0 = group VCG serial #
        ld      fi_cserial(r6),g2       # g2 = group master controller
                                        #      serial # (i.e. my serial #)
        call    DLM$pk_master           # tell the world the good news!!!
                                        # g1 = datagram ILT at nest level 2
        ldconst 4,g0                    # g0 = datagram error retry count
        call    DLM$just_senddg         # just send out datagram w/error retry
        movt    r8,g0                   # restore g0-g2
.upmlmt_600:
.endif  # ifdef BACKEND
#
.ifdef FRONTEND
#
# --- Clear the MLMT "Lost All Paths Message Sent" flag no matter.  A new path
#       means good communications until lost again.
#
        ldob    mlmt_flags(g0),r6       # r6 = MLMT flags
        clrbit  MLMT_LOST_ALL_SENT,r6,r6 # Always clear the Message Sent Flag
        stob    r6,mlmt_flags(g0)
#
# --- Send message to CCB about new path to XIOtech Controller
#
        ld      dml_alias(g4),r8        # r8 = assoc. alias serial number
        cmpobne.f 0,r8,.upmlmt_1000     # Jif primary DTMT with an alias
                                        #  defined
        ld      dtmt_lldmt(g4),r8       # r8 = LLDMT associated with DTMT
        ldob    dml_path(g4),r5         # r5 = XIOtech Controller path #
        ldob    dml_cl(g4),r6           # r6 = XIOtech Controller cluster #
        ldob    lldmt_channel(r8),r9    # r9 = This controllers path
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       r10 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        st      r11,log_dlm_ccb_path_sn(r10) # save serial # for CCB
        stob    r6,log_dlm_ccb_path_cl(r10) # save cluster # for CCB
        stob    r5,log_dlm_ccb_path_other_path(r10) # save other controllers path #
        stob    r9,log_dlm_ccb_path_this_path(r10) # save this controllers path number

        /*
        ** Add a flag if the path is of ICL type.
        */
        ldob    dtmt_icl(g4),r7
c       ICL_NotifyIclPathMade((void *)g4);
.if ICL_DEBUG
        cmpobne TRUE,r7,.upmlmt_icl01
c fprintf(stderr,"%s%s:%u <dlm$upmlmt>ICL sending ICL new path  message\n", FEBEMESSAGE, __FILE__, __LINE__);
.upmlmt_icl01:
.endif  # ICL_DEBUG
        stob    r7,log_dlm_ccb_path_icl_flag(r10) # Set ICL path flag

# This is to reduce customer log messages (warnings/info) with quit lost/made messages.
        PushRegs(r8)
c       r4 = DL_remove_delayed_message(r11, r5, r6, r9, r7);
c       if (r4 == 1 || r4 == 2) {
c           r7 = mlenewpathinfo;
c       } else {
c           r7 = mlenewpathdebug;
c       }
        PopRegsVoid(r8)
        stos    r7,mle_event(r10)       # save the message type
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], log_dlm_ccb_path_size);
.if ICL_DEBUG
        ldob    dtmt_icl(g4),r4
        cmpobne TRUE,r4,.upmlmt_icl100
c fprintf(stderr,"%s%s:%u <dlm$upmlmt> ICL -- A new path has been created...\n", FEBEMESSAGE, __FILE__, __LINE__);
.upmlmt_icl100:
.endif # ICL_DEBUG
.endif  # ifdef FRONTEND
.upmlmt_1000:
        mov     r12,g0                  # restore g0
        ret
#
#******************************************************************************
#
#  NAME: dlm$getnextdtmt
#
#  PURPOSE:
#       Returns the next DTMT on MLMT list.
#
#  DESCRIPTION:
#       Validates that the specified DTMT is still on the list. If
#       not, it returns an indication to restart the list scan. If
#       the specified DTMT is still on the MLMT list, it returns the
#       next DTMT on the MLMT list.
#
#  CALLING SEQUENCE:
#       call    dlm$getnextdtmt
#
#  INPUT:
#       g1 = DTMT to get next DTMT after
#       g7 = assoc. MLMT address
#
#  OUTPUT:
#       g0 = TRUE if DTMT was still on MLMT list
#       g0 = FALSE if DTMT is no longer on MLMT list
#       g1 = next DTMT on MLMT list if g0=TRUE
#
#  REGS DESTROYED:
#       Reg. g0, g1 destroyed.
#
#******************************************************************************
#
.ifdef BACKEND
dlm$getnextdtmt:
        ldconst FALSE,g0                # preload no longer on list indicator
        mov     g1,r4                   # r4 = specified DTMT to find on list
        ld      mlmt_dtmthd(g7),g1      # g1 = first DTMT on MLMT list
.gnxtdtmt_100:
        cmpobe  0,g1,.gnxtdtmt_1000     # Jif no DTMTs on MLMT list
        cmpobe  g1,r4,.gnxtdtmt_200     # Jif DTMT found on MLMT list
        ld      dml_mllist(g1),g1       # g1 = next DTMT on MLMT list
        b       .gnxtdtmt_100           # and check next DTMT on list
#
.gnxtdtmt_200:
        ldconst TRUE,g0                 # indicate DTMT still on list
        ld      dml_mllist(g1),g1       # g1 = next DTMT on MLMT list
.gnxtdtmt_1000:
        ret
.endif /* BACKEND */
#
#******************************************************************************
#
#  NAME: dlm$dg_retry
#
#  PURPOSE:
#       Queues a datagram request message to the datagram retry
#       message queue.
#
#  DESCRIPTION:
#       Queues a datagram request message to the datagram retry
#       message queue.
#
#  CALLING SEQUENCE:
#       call    dlm$dg_retry
#
#  INPUT:
#       g1 = datagram ILT at nest level #2.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
dlm$dg_retry:
        mov     0,r6
        ldl     dlm_rtydg_qu,r4         # r4 = first datagram on list
                                        # r5 = last datagram on list
        st      r6,il_fthd(g1)          # clear ILT fthd
        cmpobe  0,r5,.dgretry_300       # Jif list is empty
        st      g1,il_fthd(r5)          # link ILT onto end of list
        b       .dgretry_500
#
.dgretry_300:
        mov     g1,r4                   # save ILT as new head member
.dgretry_500:
        mov     g1,r5                   # save new list tail member
        stl     r4,dlm_rtydg_qu         # save updated queue pointers
        ret
#
# -----------------------------------------------------------------------------
#
#  The following code is for driving DRPs to the Front End DLM to debug the
#   code before CCB and Cache code is ready.
#
# -----------------------------------------------------------------------------
#
.if DLMFE_DRIVER
#**********************************************************************
#
#  NAME: dlmt$dlmfe_que
#
#  PURPOSE:
#       To provide a common means of queuing DRPs bound to go the the CCB or
#       Cache.
#
#  DESCRIPTION:
#       The ILT and associated request packet are queued for the executive
#       to process.  The executive is then activated to process this request.
#       This routine may be called from either the process or interrupt level.
#
#  CALLING SEQUENCE:
#       call    dlmt$dlmfe_que
#
#  INPUT:
#       g1 = ILT
#           il_w0 = DRP address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
dlmt$dlmfe_que:
#
# --- Insert ILT into executive queue
#
        ldl     dlm_fedriver_qht,r4     # Get queue head/tail
        ld      dlm_fedriver_pcb,r6     # Get PCB
        ld      dlm_fedriver_cqd,r7     # Get current queue depth
        cmpobne 0,r4,.dqfeq10           # Jif queue not empty
#
# --- Insert into empty queue
#
        mov     g1,r5                   # Update queue head/tail with
        mov     g1,r4                   #  single entry
        b       .dqfeq20
#
# --- Insert into non-empty queue
#
.dqfeq10:
        st      g1,il_fthd(r5)          # Append ILT to end of queue
        mov     g1,r5                   # Update queue tail
.dqfeq20:
        stl     r4,dlm_fedriver_qht
        addo    1,r7,r7                 # Bump queue depth
        st      r7,dlm_fedriver_cqd
#
# --- Activate executive if necessary
#
        ldob    pc_stat(r6),r3          # Get current process status
        mov     pcrdy,r8                # Get ready status
        cmpobe  pcnrdy,r3,.dqfeq40      # Jif status is "not ready"
        cmpobne pctwait,r3,.dqfeq100    # Jif status is not "timer wait"
#
.dqfeq40:
.ifdef HISTORY_KEEP
c CT_history_pcb(".dqfeq40 setting ready pcb", r6);
.endif  # HISTORY_KEEP
        stob    r8,pc_stat(r6)          # Ready process
#
# --- Exit
#
.dqfeq100:
        ret
#
#******************************************************************************
#
#  NAME:  dlmt$dlmfe_driver
#
#  PURPOSE:
#       Handles Datagrams going to the CCB or Cache.  Also initiates Datagrams
#       like the come from the CCB or Cache.
#
#  DESCRIPTION:
#       Every 10 seconds, creates a new datagram for the CCB or Cache and
#       sends that to DLM.  If a DRP is received, checks out the DRP and then
#       returns, sometimes with good status and sometimes with bad status.
#
#  CALLING SEQUENCE:
#       process call
#
#  INPUT:
#       Nothing
#
#  OUTPUT:
#       None
#
#  REGS DESTROYED:
#       None
#
#******************************************************************************
#
dlmt$dlmfe_driver:
#
# --- Wait 30 seconds before starting to send work
#
        mov     0,g13                   # Clear g13
        ldconst 30000,g0                # Wait 30 seconds
        call    K$twait
.dfe10:
#
# --- Wait for a while to simulate processing the operations
#
        ldconst 10000,g0                # Wait 10 seconds
        call    K$twait
#
# --- Get next queued request
#
        ld      dlm_fedriver_cqd,r3     # Get current queue depth
        cmpobe  0,r3,.dfe500            # Jif none - create work
#
        subo    1,r3,r3                 # Adjust current queue depth
        st      r3,dlm_fedriver_cqd
        ldl     dlm_fedriver_qht,r6     # Get next queue head/tail
#
# --- Dequeue selected request (FIFO fashion)
#
        mov     r6,r14                  # Isolate queued ILT
#
        ld      il_fthd(r6),r6          # Dequeue ILT
        cmpo    0,r6                    # Update queue head/tail
        sele    r7,0,r7
        stl     r6,dlm_fedriver_qht
#
        ld      vrvrp(r14),r13          # r13 = DRP address from the ILT
        ld      dr_req_address(r13),g4  # g4 = Datagram Request address
#
        ld      dr_req_length(r13),g5   # g5 = Datagram Request length
        ld      dr_rsp_address(r13),g6  # g6 = Response address
        ld      dr_rsp_length(r13),g7   # g7 = Response length
#
# --- Verify the DRP
#
        mov     1,g0                    # Show bad stuff!
        cmpobe  0,g6,.dfe400            # Jif there is no Response - bad!!!
        ldos    dr_func(r13),r3         # r3 = Function code
        ldconst 0x0701,r4               # Check for CCB Function
        cmpobe  r4,r3,.dfe200           # Jif it is a CCB DRP
        ldconst 0x0711,r5               # Check for Cache Function
        cmpobe  r5,r3,.dfe100           # Jif it is a Cache DRP
.dfe50:
        mov     0,g0                    # Show DRP received OK but has problems
        ldq     dlm$srvr_invfc,r4       # Show Unknown function code
        stq     r4,(g6)
        ldconst 3,r8                    # Set up EC #2
        stob    r8,dgrs_ec2(g6)
        b       .dfe400
#
# --- Cache Datagram
#
.dfe100:
        ldq     dlm$srvr_ok,r4          # Start off showing Good Status
        stq     r4,(g6)
        ldconst dgrs_size,r4            # Get the response header size
        subo    r4,g7,r5                # Get the response data size
        bswap   r5,r5                   # Change to little endian format
        st      r5,dgrs_resplen(g6)     # save the amount to transfer
        b       .dfe300                 # Nothing generic to test
                                        # Add specifics later if needed
#
# --- CCB Datagram
#
.dfe200:
        ldq     dlm$srvr_ok,r4          # Start off showing Good Status
        stq     r4,(g6)
        ldconst dgrs_size,r4            # Ensure there is only a response header
        cmpobe  r4,g7,.dfe300           # Jif there is no response data expected
        mov     0,g0                    # Show DRP received OK but has problems
        ldq     dlm$srvr_invparm,r4     # Show invalid parameter
        stq     r4,(g6)
        ldconst 4,r8                    # Set up EC #2
        stob    r8,dgrs_ec2(g6)
        b       .dfe400
#
# --- Respond to the request immediately
#
.dfe300:
        ld      DLMT_drp_in,r7          # r7 = DRP Count coming in
        addo    1,r7,r7                 # Increment the DRP Count coming in
        st      r7,DLMT_drp_in          # Store the new count
        ld      DLMT_nerr,r6            # Get the count of the next error DRP
        cmpo    r7,r6                   # Is it time to throw an Error?
        sele    dg_st_ok,dg_st_srvr,r5  #  OK if not, else Server Error if it is
        stob    r5,dgrs_status(g6)      # Update Datagram Response status
        sele    0,1,r15                 # Set up EC #1
        stob    r15,dgrs_ec1(g6)
        sele    0,2,r9                  # Set up EC #2
        stob    r9,dgrs_ec2(g6)
        mov     0,g0                    # Set up Return code
        bne     .dfe400                 # Jif not time to update the next error
        st      g0,dgrs_resplen(g6)     # Show only header to be transferred
        ldconst MAX_DRP_ERR,g0          # Calculate the next DRP to throw an
        call    dlmt$getscaledrand      #  error on
        cmpo    0,g0                    # Ensure the number is not zero
        sele    g0,1,g0
        addo    g0,r7,r7                # Add the random number to the current
        st      r7,DLMT_nerr            #  to get the next DRP to error on
        mov     0,g0                    # Set up the return code (always good)
.dfe400:
        mov     r14,g1                  # Set up the ILT to complete
        ld      il_cr(r14),r4           # Get the completion routine to call
        callx   (r4)                    # Complete primary ILT
        b       .dfe10                  # Handle some more incoming requests
#
# --- Create work
#
.dfe500:
        ld      DLMT_drp_out,r7         # r7 = DRP Count going out
        addo    1,r7,r7                 # Increment the DRP Count going out
        st      r7,DLMT_drp_out         # Store the new count
        and     3,r7,r7                 # modulo 3 the count to create a DRP
        cmpobe  0,r7,.dfe800            # Jif time for DLM1 Terminate Comm
        cmpobe  1,r7,.dfe700            # Jif time for DLM1 Establish Comm
        cmpobe  2,r7,.dfe600            # Jif time for a Cache Datagram
#
#       Create CCB Datagram
#
        ldconst dgrq_size+dgrs_size,r4  # Get the Req and Rsp Header size
        ldconst 0xFC,g0                 # Get a random number between 0 - 252
                                        #  in four byte increments
        call    dlmt$getscaledrand      # g0 = random number
        mov     g0,r5                   # save random size of request data
        addo    g0,r4,g2                # Total size of the buffer
        ldconst dgrq_size,r6            # size of the request header
        mov     g2,g0                   # Get the buffer
        addo    r6,r5,g5                # g5 = request message length
c       g0 = s_MallocC(g0, __FILE__, __LINE__); # Buffer for message
        addo    g0,r6,r7                # r7 = beginning of request data
        mov     g0,g4                   # g4 = request message (header) address
        addo    r5,r7,g6                # g6 = response message (header) address
        ldconst dgrs_size,g7            # g7 = size of response (only header)
        st      r5,dgrq_reqlen(g4)      # save the size of the request data
        stob    r6,dgrq_hdrlen(g4)      # save the size of the request header
        ldconst 0xFE,r8                 # show the CCB as the server processor
        stob    r8,dgrq_srvcpu(g4)
        ldconst 0xFF,r8                 # Use any path possible to the other
        ld      dlm_mlmthd,r3           # r3 = head of controller list
        stob    r8,dgrq_path(g4)        #  to get to the other controller
        cmpobe  0,r3,.dfe520            # Jif there are no attached controllers
        ld      mlmt_sn(r3),r3          # Get the Serial Number of the other guy
        b       .dfe540                 # Set up the other controller as dest.
#
.dfe520:
        ld      K_ficb,r3               # r3 = FICB address
        ld      fi_cserial(r3),r3       # r3 = my MAG serial #
.dfe540:
        bswap   r3,r12                  # r12 = my MAG serial # in big-endian
        st      r12,dgrq_dstsn(g4)      # save my serial # as destination
        ldconst 0x30424343,r8           # show the CCB0 as the service provider
        st      r8,dgrq_srvname(g4)
        stob    g7,dgrs_hdrlen(g6)      # save the size of the response header
        mov     0,r8                    # show there is no response data
        st      r8,dgrs_resplen(g6)
        ldconst 0x700,g0                # Show the DRP is from CCB
        b       .dfe900                 # Create the DRP and send it
#
#       Create Cache Datagram
#
.dfe600:
        ldconst dgrq_size+dgrs_size,r4  # Get the Req and Rsp Header size
        ldconst 0xFC,g0                 # Get a random number between 0 - 252
                                        #  in four byte increments
        call    dlmt$getscaledrand      # g0 = random number
        shro    1,g0,r5                 # r5 = random size of request data
        ldconst 0xFC,r9                 # Ensure request size is on a four byte
        and     r9,r5,r5                #   boundary
        subo    r5,g0,r9                # r9 = random size of response data
        addo    g0,r4,g2                # Total size of the buffer
        ldconst dgrq_size,r6            # size of the request header
        mov     g2,g0                   # Get the buffer
        addo    r6,r5,g5                # g5 = request message length
c       g0 = s_MallocC(g0, __FILE__, __LINE__); # Buffer for message
        addo    g0,r6,r7                # r7 = beginning of request data
        mov     g0,g4                   # g4 = request message (header) address
        addo    r5,r7,g6                # g6 = response message (header) address
        ldconst dgrs_size,g7            # g7 = size of response (only header)
        st      r5,dgrq_reqlen(g4)      # save the size of the request data
        stob    r6,dgrq_hdrlen(g4)      # save the size of the request header
        ldconst 0xFF,r8                 # Use any path possible to the other
        ld      dlm_mlmthd,r3           # r3 = head of controller list
        stob    r8,dgrq_path(g4)        #  to get to the other controller
        cmpobe  0,r3,.dfe620            # Jif there are no attached controllers
        ld      mlmt_sn(r3),r3          # Get the Serial Number of the other guy
        b       .dfe640                 # Set up the other controller as dest.
#
.dfe620:
        ld      K_ficb,r3               # r3 = FICB address
        ld      fi_cserial(r3),r3       # r3 = my MAG serial #
.dfe640:
        bswap   r3,r12                  # r12 = my MAG serial # in big-endian
        st      r12,dgrq_dstsn(g4)      # save my serial # as destination
        ldconst 0x30434143,r8           # show the CAC0 as the service provider
        st      r8,dgrq_srvname(g4)
        stob    g7,dgrs_hdrlen(g6)      # save the size of the response header
        mov     0,r8
        bswap   r9,r3                   # Change to little endian format
        st      r3,dgrs_resplen(g6)     # set the response data length
        addo    g7,r9,g7                # g7 = response message length (total)
        stob    r8,dgrq_srvcpu(g4)      # show the FE as the server processor
        ldconst 0x710,g0                # Show the DRP is from Cache
        b       .dfe900                 # Create the DRP and send it
#
#       Create DLM1 Establish Communications DRP
#
.dfe700:
        ldconst dgrq_size+dgrs_size,g2  # g2 = total buffer size
        ldconst dgrq_size,r6            # size of the request header
c       g0 = s_MallocC(g2, __FILE__, __LINE__); # Buffer for message
        mov     r6,g5                   # g5 = request message length
        addo    g0,r6,g6                # g6 = response message (header) address
        mov     g0,g4                   # g4 = request message (header) address
        ldconst dgrs_size,g7            # g7 = size of response (only header)
        mov     0,r8
        st      r8,dgrs_resplen(g6)     # show there is no response data
        st      r8,dgrq_reqlen(g4)      # show there is no request data
        stob    r6,dgrq_hdrlen(g4)      # save the size of the request header
        stob    r8,dgrq_srvcpu(g4)      # show the FE as the server processor
        ld      K_ficb,r3               # r3 = FICB address
        ld      fi_cserial(r3),r3       # r3 = my MAG serial #
        bswap   r3,r12                  # r12 = my MAG serial # in big-endian
        st      r12,dgrq_dstsn(g4)      # save my serial # as destination
        ldconst DLM1name,r8             # show the DLM1 as the service provider
        st      r8,dgrq_srvname(g4)
        stob    g7,dgrs_hdrlen(g6)      # save the size of the response header
        ldconst DLM1_fc_estcc,r8        # Show this function as an establish
        stob    r8,dgrq_fc(g4)          #   communications datagram
        ld      dlm_mlmthd,g0           # see if there is another controller
        cmpobe  0,g0,.dfe760            # Jif there is not a real controller
        ld      mlmt_sn(g0),g0          # g0 = Serial Number of other controller
        b       .dfe780                 # issue the ping to the other controller
#
.dfe760:
        call    dlmt$getrand            # g0 = rand number 0-FFFFFFFF
.dfe780:
        st      g0,dgrq_g2(g4)          # Store the other controllers
        ldconst 0x700,g0                # Show the DRP is from CCB
        b       .dfe900                 # Create the DRP and send it
#
#       Create DLM1 Terminate Communications DRP
#
.dfe800:
        ldconst dgrq_size+dgrs_size,g2  # g2 = total buffer size
        ldconst dgrq_size,r6            # size of the request header
c       g0 = s_MallocC(g2, __FILE__, __LINE__); # Buffer for message
        mov     r6,g5                   # g5 = request message length
        addo    g0,r6,g6                # g6 = response message (header) address
        mov     g0,g4                   # g4 = request message (header) address
        ldconst dgrs_size,g7            # g7 = size of response (only header)
        mov     0,r8
        st      r8,dgrs_resplen(g6)     # show there is no response data
        st      r8,dgrq_reqlen(g4)      # show there is no request data
        stob    r6,dgrq_hdrlen(g4)      # save the size of the request header
        stob    r8,dgrq_srvcpu(g4)      # show the FE as the server processor
        ld      K_ficb,r3               # r3 = FICB address
        ld      fi_cserial(r3),r3       # r3 = my MAG serial #
        bswap   r3,r12                  # r12 = my MAG serial # in big-endian
        st      r12,dgrq_dstsn(g4)      # save my serial # as destination
        ldconst DLM1name,r8             # show the DLM1 as the service provider
        st      r8,dgrq_srvname(g4)
        stob    g7,dgrs_hdrlen(g6)      # save the size of the response header
        ldconst DLM1_fc_trmcc,r8        # Show this function as an establish
        stob    r8,dgrq_fc(g4)          #   communications datagram
        ld      dlm_mlmthd,g0           # see if there is another controller
        cmpobe  0,g0,.dfe860            # Jif there is not a real controller
        ld      mlmt_sn(g0),g0          # g0 = Serial Number of other controller
        b       .dfe880                 # issue the ping to the other controller
#
.dfe860:
        call    dlmt$getrand            # g0 = rand number 0-FFFFFFFF
.dfe880:
        st      g0,dgrq_g2(g4)          # Store the other controllers
        ldconst 0x700,g0                # Show the DRP is from CCB
#
#       Create DRP and send
#
.dfe900:
                                        # g0 = function code to put in the DRP
                                        # g4 = request message address
                                        #       (includes header and data)
                                        # g5 = request message length
                                        #       (includes header and data)
                                        # g6 = response message address
                                        #       (includes header and data)
                                        # g7 = response message length
                                        #       (includes header and data)
        call    dlm$create_drp          # g0 = DRP
c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
        st      g0,il_w0(g1)            # save the DRP address in the ILT
        st      g2,il_w1(g1)            # save the length to free
        st      g4,il_w2(g1)            # save the address of the datagram
        lda     dlmt$drp_comp,r8        # set up the ILT completion routine
        st      r8,il_cr(g1)            #   to call when done
        lda     ILTBIAS(g1),g1          # Bump ILT levels
        mov     0,r3                    # Clear the forward thread before
        st      r3,il_fthd(g1)          #   putting it on the queue

        call    DLM$quedrp              # Queue the DRP to FE DLM
#
        b       .dfe10                  # Get the next Request or create one
#
#**********************************************************************
#
#  NAME: dlmt$drp_comp
#
#  PURPOSE:
#       Handle the completion of a DLM Test driver DRP
#
#  DESCRIPTION:
#       Release the DRP, Datagram memory, and ILT.
#
#  CALLING SEQUENCE:
#       call    dlmt$drp_comp
#
#  INPUT:
#       g0 = return status
#       g1 = ilt
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
dlmt$drp_comp:
        mov     g1,r15                  # save g1
        ld      il_w2(r15),r4           # r4 = datagram address
        ld      il_w1(r15),r5           # r5 = size of the datagram

        ld      il_w0(r15),g0           # g0 = DRP address
c       s_Free(g0, drpsiz, __FILE__, __LINE__);

c       s_Free(r4, r5, __FILE__, __LINE__); # Free the datagram memory

        mov     r15,g1                  # restore g1
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
        ret
#
#**********************************************************************
#
#  NAME: dlmt$getrand/dlmt$getscaledrand
#
#  PURPOSE:
#       To provide a pseudo random number
#
#  DESCRIPTION:
#       This module will generate a pseudo random number based on a seed
#       to be used in simulating queue depths lengths and LBAs.
#
#  CALLING SEQUENCE:
#       call    dlmt$getrand/dlmt$getscaledrand
#
#  INPUT:
#       g0 = scaling factor (rand % scale) (for dlmt$getscaledrand only)
#
#  OUTPUT:
#       g0 = random number
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
dlmt$getscaledrand:
        mov     g0,r15                  # Save the scale factor
        b       .dlmtgr10
#
dlmt$getrand:
        lda     0xffffffff,r15          # want the whole word
.dlmtgr10:
        ld      DLMT_rand_seed,g0       # Get the random seed
        lda     0x6255,r3               # rand * 25173
        mulo    r3,g0,g0
        lda     0x3619,r3               # (rand * 25173) + 13849
        addo    r3,g0,g0
        st      g0,DLMT_rand_seed       # Save for the next call
        and     r15,g0,g0               # scale the random number
        ret
#
# -----------------------------------------------------------------------------
#
#  The above code is for driving DRPs to the Front End DLM to debug the
#   code before CCB and Cache code is ready.
#
# -----------------------------------------------------------------------------
#
.endif  # DLMFE_DRIVER
#
#******************************************************************************
#
#  NAME: dlm$proc_alias_
#
#  PURPOSE:
#       This routine processes a primary DTMT handling any previous
#       alias DTMT associations that are no longer appropriate and/or
#       establishing any new alias DTMT associations.
#
#  DESCRIPTION:
#       This routine determines if any of the following cases apply
#       to the specified primary DTMT and if processes them appropriately
#       if any cases apply:
#
#       Case 1: An alias DTMT is associated with the primary DTMT
#               and remains unchanged.
#       Case 2: An alias DTMT is associated with the primary DTMT
#               but the primary DTMT either no longer has an alias
#               associated with it or has a different alias
#               associated with it.
#       Case 3: No alias DTMT was associated with the primary DTMT
#               and no alias is currently associated with the
#               primary DTMT.
#       Case 4: No alias DTMT was associated with the primary DTMT
#               but an alias is currently associated with the
#               primary DTMT.
#
#  CALLING SEQUENCE:
#       call    dlm$proc_alias_
#
#  INPUT:
#       g4 = primary DTMT address to process alias DTMTs for
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
dlm$proc_alias_:
        ld      dtmt_alias_dtmt(g4),r8  # r8 = assoc. alias DTMT address
        ld      dml_alias(g4),r4        # r4 = current alias node serial
                                        #      number
        cmpobe.t 0,r8,.procalias_300    # Jif no alias DTMT assoc. with
                                        #  this primary DTMT
        ld      dml_sn(r8),r5           # r5 = previous alias node serial
                                        #      number
        cmpobne.f r4,r5,.procalias_100  # Jif alias node serial number
                                        #  has changed
#
# --- Case 1: An alias DTMT is associated with the primary DTMT
#               and remains unchanged. Copy current information
#               from the primary DTMT to the alias DTMT.
#
        ldt     dtmt_alpa(g4),r4        # r4 = alpa address
                                        # r5-r6 = node WWN
        stt     r4,dtmt_alpa(r8)        # save in alias DTMT
        ldl     dtmt_pwwn(g4),r4        # r4-r5 = port WWN
        stl     r4,dtmt_pwwn(r8)        # save in alias DTMT
        ldq     dml_path(g4),r4         # r4 = dml_path, dml_cl, dml_vdcnt,
                                        #      dml_flag1
                                        # r5-r6 = node name
                                        # r7 = IP address
        stq     r4,dml_path(r8)         # save in alias DTMT
        b       .procalias_1000         # and we're done!
#
.procalias_100:
#
# --- Case 2: Terminate previous alias DTMT since it no longer
#             is valid.
#
.if ICL_DEBUG
        ldob    dtmt_icl(g4),r5
        cmpobne TRUE,r5,.procalias_icl01
c fprintf(stderr,"%s%s:%u <dlm$proc_alias>ICL..calling term_alias_dtmt() for  DTMT=%x\n", FEBEMESSAGE, __FILE__, __LINE__,(UINT32)g4);
.procalias_icl01:
.endif  # ICL_DEBUG
        call    dlm$term_alias_dtmt     # terminate current alias DTMT
.procalias_300:
        cmpobe.t 0,r4,.procalias_1000   # Jif no alias node is defined for
                                        #  this primary DTMT
        call    dlm$est_alias_dtmt      # establish an alias DTMT and
                                        #  associate it with this DTMT
.procalias_1000:
        ret
#
#******************************************************************************
#
#  NAME: dlm$term_alias_dtmt
#
#  PURPOSE:
#       Terminate an associated alias DTMT if necessary.
#
#  DESCRIPTION:
#       Checks the specified primary DTMT for an alias DTMT. If none
#       is defined, nothing else is done. If one is defined, it
#       terminates the association between the primary DTMT and alias
#       DTMT and then terminates the alias DTMT by removing it from
#       association with any MLMT and then deallocating the DTMT back
#       into the free DTMT pool.
#
#  CALLING SEQUENCE:
#       call    dlm$term_alias_dtmt
#
#  INPUT:
#       g4 = primary DTMT address to terminate alias DTMT with
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
dlm$term_alias_dtmt:
        movt    g4,r12                  # save g4-g6
                                        # r12 = pri. DTMT address
        ld      dtmt_alias_dtmt(r12),g4 # g4 = assoc. alias DTMT address
        cmpobe.t 0,g4,.termaliasdtmt_1000 # Jif no alias DTMT defined
        ldconst 0,r3
        ld      dtmt_lldmt(r12),g6      # g6 = assoc. LLDMT address
        st      r3,dtmt_alias_dtmt(r12) # remove alias DTMT from primary DTMT
.if ICL_DEBUG
        ldob    dtmt_icl(g4),r5
        cmpobne TRUE,r5,.termaliasdtmt_icl05
c fprintf(stderr,"%s%s:%u <dlm$term_alias_dtmt>ICL .removing alias DTMT= %x.. its primary =%x\n", FEBEMESSAGE, __FILE__, __LINE__,(UINT32)g4,(UINT32)r12);
c fprintf(stderr,"%s%s:%u <dlm$term_alias_dtmt>calling dlm$rem_dtmt and dlm$put_dtmt()\n", FEBEMESSAGE, __FILE__, __LINE__);
.termaliasdtmt_icl05:
.endif  # ICL_DEBUG
        call    dlm$rem_dtmt            # remove DTMT from LLDMT & MLMT
.ifdef M4_DEBUG_DTMT
c fprintf(stderr, "%s%s:%u put_dtmt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g4);
.endif # M4_DEBUG_DTMT
c       put_dtmt(g4);                   # deallocate DTMT back into free pool
.termaliasdtmt_1000:
        movt    r12,g4                  # restore g4-g6
        ret
#
#******************************************************************************
#
#  NAME: dlm$est_alias_dtmt
#
#  PURPOSE:
#       Establishes an alias DTMT and associates it with the
#       specified primary DTMT.
#
#  DESCRIPTION:
#       This routine allocates a DTMT and sets it up as an alias
#       DTMT and associates it with the specified primary DTMT.
#
#  CALLING SEQUENCE:
#       call    dlm$est_alias_dtmt
#
#  INPUT:
#       g4 = primary DTMT address to establish alias DTMT with
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
dlm$est_alias_dtmt:
        mov     g4,r12                  # save g4
                                        # r12 = pri. DTMT address
c       g4 = get_dtmt();                # allocate DTMT to use as the alias DTMT
.ifdef M4_DEBUG_DTMT
c fprintf(stderr, "%s%s:%u get_dtmt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g4);
.endif # M4_DEBUG_DTMT
        st      g4,dtmt_alias_dtmt(r12) # save alias DTMT in pri. DTMT
        st      r12,dtmt_pri_dtmt(g4)   # save pri. DTMT in alias DTMT

.if FE_ICL
        ldob    dtmt_icl(r12),r4
        stob    r4,dtmt_icl(g4)
.if ICL_DEBUG
        cmpobne TRUE,r4,.estaliasdtmt_icl01
c fprintf(stderr,"%s%s:%u <est_alias_dtmt>ICL. alias dtmt=%x;primary dtmt=%x\n", FEBEMESSAGE, __FILE__, __LINE__,(UINT32)g4,(UINT32)r12);
.estaliasdtmt_icl01:
.endif  # ICL_DEBUG
.endif  # FE_ICL
        ldl     dtmt_type(r12),r4       # r4 = dtmt_type, dtmt_state,
                                        #      dtmt_sulindx
                                        # r5 = dtmt_lldmt
        stl     r4,dtmt_type(g4)        # save in alias DTMT
        ldq     dtmt_ehand(r12),r4      # r4 = dtmt_ehand
                                        # r5 = dtmt_alpa
                                        # r6-r7 = node WWN
        stq     r4,dtmt_ehand(g4)       # save in alias DTMT
        ldl     dtmt_pwwn(r12),r4       # r4-r5 = port WWN
        stl     r4,dtmt_pwwn(g4)        # save in alias DTMT
        ld      dml_alias(r12),r4       # r4 = alias node serial number
        st      r4,dml_sn(g4)           # save alias node serial number
        ldq     dml_path(r12),r4        # r4 = dml_path, dml_cl, dml_vdcnt,
                                        #      dml_flag1
                                        # r5-r6 = peer node name
                                        # r7 = IP address
        stq     r4,dml_path(g4)         # save in alias DTMT
        call    dlm$upmlmt              # update MLMT structures with alias
                                        #  DTMT as appropriate
        ld      dml_mlmt(r12),r3        # Get MLMT (Group (DSC, CNC, VCG))
        ldob    mlmt_flags(r3),r4       # Get the MLMT Flags
        setbit  MLMT_CONTROLLER_GROUP,r4,r4 # Note this is the Group MLMT
        stob    r4,mlmt_flags(r3)       # Save MLMT Flags
        mov     r12,g4                  # restore g4
        ret
#
.endif  # MAG2MAG
#
#******************************************************************************
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
