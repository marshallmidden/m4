# $Id: idriver.as 159730 2012-08-27 22:03:32Z marshall_midden $
#******************************************************************************
#
#  NAME: idriver.as
#
#  PURPOSE:
#   To provide a device driver element to process MAGNITUDE to target
#   SCSI-FCP events and to manage their completion.
#
#  FUNCTIONS:
#       I$init      - Initiator driver initialization
#
#       This module employs the following processes:
#       I$exec       - Initiator driver executive
#
#  Copyright (c) 1998 - 2012 Xiotech Corporation.  All rights reserved.
#
#******************************************************************************
#
.if     INITIATOR
#
# --- local equates -----------------------------------------------------------
#
        .set    TMTMAX ,16              # Max. # TMTs to support
        .set    TLMTMAX ,TMTMAX*4       # Max. # TLMTs to support
        .set    ISMTMAX ,TLMTMAX*4      # MAx. # ISMTs to support
        .set    XLIMAX ,256             # Max. # XLIs to support
        .set    i_twait1,1000           # mag$exec timed wait (in msec.)
                                        #  when no virtual drives assigned
#       .set    SCANDELAY,2000          # 2 sec - time to wait before starting device scan
        .set    SCANDELAY,1000          # 1 sec - time to wait before starting device scan
#       .set    SCANDELAY,250           # 1/4 sec - time to wait before starting device scan

        .set    RETRY_PLOGI_PAUSE,1000 # interval before reissuing plogi...
        .set    RETRY_PLOGI_RETRIES,5  # retry plogi before giving up...

        .set    CNTLTO,120              # 30 seconds (250ms X 4) controller t/o value
        .set    FTDISCTO,20             # 5 seconds
        .set    DISCRETRY,16            # discovery retry counter
        .set    nodev_reset,1           # no local devices reset counter

        .set    FLTD_word0,0x7fffffff   # Fabric LID tables definitions
        .set    FLTD_word1,0xffffffff   # 0x81 - 0xfe
        .set    FLTD_word2,0xffffffff   #       Qlogic currently reserves
        .set    FLTD_word3,0xfffffffe   #       0x80 and 0xff.
#
# --- Port Login (PLOGI) specific error codes ---------------------------------
#
#       returned in g1:
#
        .set    PLOGI_NO_LOOP,1             # No loop
        .set    PLOGI_NO_IOCB,2             # IOCB could not be allocated
        .set    PLOGI_NO_XCHG_RSRC,3        # exchange resource could not be allocated
        .set    PLOGI_ELS_TO_NO_LOOP,4      # ELS timed out or no loop device
        .set    PLOGI_NO_FABRIC_LOOP_PORT,5 # no fabric loop port
        .set    PLOGI_REM_TARGET_NO_SPT,6   # remote device does not support target function.
        .set    PLOGI_RCVD_LS_RJT,13        # LS_RJT response to PLOGI.

#       returned in g1 if ELS timeout or no loop device:
#
        .set    PDISC_ISSUED,0              # issuing PDISC
        .set    PDISC_RESPONSE_WAIT,1       # waiting for PDISC response
        .set    PLOGI_ISSUED,2              # issuing PLOGI
        .set    PLOGI_RESPONSE_WAIT,3       # waiting for PLOGI response
        .set    PRLI_ISSUED,4               # issuing PRLI
        .set    PRLI_RESPONSE_WAIT,5        # issuing PRLI
        .set    LOGGING_IN_ERROR,6          # logging in (should not occur)
        .set    LOGOFF_UNAVAL_PORT,7        # port unavailable (PCB reinit due to LOGO)
        .set    PRLO_ISSUED,8               # issuing PRLO
        .set    PRLO_RESPONSE_WAIT,9        # waiting for PRLO response
        .set    LOGO_ISSUED,10              # issuing LOGO
        .set    LOGO_RESPONSE_WAIT,11       # waiting for LOGO response
#
# --- global function declarations
#
        .globl  I$init                  # Initiator driver initialization
        .globl  I_recv_online           # Online indication
        .globl  I$recv_online           # Online indication
        .globl  I_recv_offline          # Offline indication
        .globl  I_recv_rinit            # C - Reset and initialize indication
        .globl  I$SIF                   # Set Initiator Flags
#
# --- External global routine definitions
#
        .globl  ISP_ResetChip           # C access
        .globl  isp$dump_ql
#
# --- global usage data definitions
#
        .globl  I$IDFLAG                # true = I$WWN and I$NID are valid
        .globl  I$WWN                   # World wide name (P-name) of this I/F
        .globl  I$NID                   # loop id of this interface
                                        #  to my interface
        .globl  I_noass                 # no imt/ltmt association
#
# --- global usage data definitions
#
        .data

.if     ITRACES
#
# --- Trace support
#
        .globl  I_temp_trace            # trace record build area
I_temp_trace:
        .space  trr_recsize,0           # trace record build area
#
.endif  # ITRACES
#
        .align  4                       # align just in case
#
# --- ICIMT directory
#
        .globl  I_CIMT_dir
I_CIMT_dir:
        .space  ICIMTMAX*4,0            # ICIMT directory area
#
        .globl  at2_pcb
at2_pcb:
        .space  4*4,0            # ICIMT directory area
#
# --- Statistical counters
#
I_undef_event:
        .word   0                       # # undefined events counter
I_no_abort_1:
        .word   0                       # abort with no abort
I_no_abort_2:
        .word   0                       # abort with no abort
I_no_abort_3:
        .word   0                       # abort with no abort
I_notreg:
        .word   0                       # TMT not registered with LLD

I_noass:
        .word   0                       # no IMT/LTMT association
#
# --- Process PCBs
#
I_exec_pcb:
        .word   0                       # i$exec PCB address
APL_timer_pcb:
        .word   0                       # apl$timer PCB address
#
# --- Local storage fields
#
i_mintbl:
        .word   0                       # minimum table address
i_maxtbl:
        .word   0                       # maximum table address
i_cleanup_pcb:
        .word   0                       # TMT cleanup PCB
i$nodev_cnt:
        .byte   0,0,0,0                 # no local devices counter
                                        # (1 byte per port)
#
# --- Basic replogi trace info...
#
i_replogi_count:
        .word   0                       # total number of replogis...
i_replogi_ok_count:
        .word   0                       # total number of ok replogis...
i_replogi_tmt:
        .word   0                       # last tmt replogied...
i_replogi_ilt:
        .word   0                       # last child ilt replogied...
i_replogi_pid:
        .word   0                       # last pid replogied...
#
# --- Loop ID areas
#
I$IDFLAG:
        .byte   0                       # ID flag                       <b>
        .byte   0                       # spare
        .byte   0                       # spare
        .byte   0                       # spare
I$WWN:
        .word   0                       # World Wide Name              8<b>
        .word   0
I$NID:
        .word   0                       # N port ID                    4<b>
#
#******************************************************************************
#
# _________________ INITIATOR EVENT HANDLER TABLES ___________________________
#
#******************************************************************************
#
# --- Channel driver event handler tables
#
# --- Event definitions
#
        .set    Te_success,4            # success
        .set    Te_CS,8                 # check status
        .set    Te_nonCS,12             # non check status
        .set    Te_IOE,16               # I/O error
        .set    Te_ME,20                # Miscellaneous error
        .set    Te_abort,24             # abort from Qlogic
        .set    Te_online,28            # loop online
        .set    Te_offline,32           # loop offline
        .set    Te_timeout,36           # gross timer expiration
        .set    Te_TskAbort,40          # task aborted

        .set    Te_stop,44              # stop event
        .set    Te_cont,48              # continue event

        .set    Te_logout,52            # iscsi session logout
#######################################################################
#
#       Discovery Process Event Handler tables
#
#######################################################################
#
# --- loop is offline
#
.Ts_offline:
        .word   i$invalid               # invalid completion code
        .word   i$ignore                # success completion
        .word   i$ignore                # SCSI check status
        .word   i$ignore                # SCSI non check status
        .word   i$ignore                # I/O error
        .word   i$ignore                # Miscellaneous error
        .word   i$ignore                # abort from Qlogic
        .word   i$ignore                # online
        .word   i$ignore                # offline
        .word   i$ignore                # timeout
        .word   i$ignore                # task abort
        .word   i$ignore                # stop event
        .word   i$ignore                # continue event
        .word   i$ignore                # logout event
#
# --- process a response received while waiting to go offline
#
.Ts_going_offline:
        .word   i$invalid               # invalid completion code
        .word   i$scan_flush            # success completion
        .word   i$scan_flush            # SCSI check status
        .word   i$scan_flush            # SCSI non check status
        .word   i$scan_flush            # I/O error
        .word   i$scan_flush            # Miscellaneous error
        .word   i$ignore                # abort from Qlogic
        .word   i$wait2restart          # online
        .word   i$ignore                # offline
        .word   i$scan_flush            # timeout
        .word   i$ignore                # task abort
        .word   i$ignore                # stop event
        .word   i$ignore                # continue event
        .word   i$ignore                # logout event
#
# --- process a response received from an TUR sent to LUN 0.
#
.Ts_start:
        .word   i$invalid               # invalid completion code
        .word   i$ignore                # success completion
        .word   i$ignore                # SCSI check status
        .word   i$ignore                # SCSI non check status
        .word   i$ignore                # I/O error
        .word   i$ignore                # Miscellaneous error
        .word   i$ignore                # abort from Qlogic
        .word   i$wait2restart          # online
        .word   i$go_offline            # offline
        .word   i$ploop_start_call      # timeout
        .word   i$ignore                # task abort
        .word   i$ignore                # stop event
        .word   i$ignore                # continue event
        .word   i$ignore                # logout event
#
# --- process a response received from an TUR sent to LUN 0.
#
.Ts_login:
        .word   i$invalid               # invalid completion code
        .word   i$login                 # success completion
        .word   i$scan_flush            # SCSI check status
        .word   i$scan_flush            # SCSI non check status
        .word   i$scan_flush            # I/O error
        .word   i$scan_flush            # Miscellaneous error
        .word   i$scan_flush            # abort from Qlogic
        .word   i$scan_flush            # online
        .word   i$go_offline            # offline
        .word   i$ignore                # timeout
        .word   i$ignore                # task abort
        .word   i$ignore                # stop event
        .word   i$ignore                # continue event
        .word   i$scan_flush            # logout event
#
# --- process the completion of the GAN loop.
#
.Ts_start_fabric:
        .word   i$invalid               # invalid completion code
        .word   i$fabric_continue       # success completion
        .word   i$ignore                # SCSI check status
        .word   i$ignore                # SCSI non check status
        .word   i$ignore                # I/O error
        .word   i$ignore                # Miscellaneous error
        .word   i$ignore                # abort from Qlogic
        .word   i$wait2restart          # online
        .word   i$go_offline            # offline
        .word   i$ignore                # timeout
        .word   i$ignore                # task abort
        .word   i$ignore                # stop event
        .word   i$ignore                # continue event
        .word   i$ignore                # logout event
#
# --- process the completion fabric LID loop when issuing TUR
#     command to the LID's.
#
.Ts_fabric_TUR:
        .word   i$invalid               # invalid completion code
        .word   i$send_tur              # success completion
        .word   i$ignore                # SCSI check status
        .word   i$ignore                # SCSI non check status
        .word   i$ignore                # I/O error
        .word   i$ignore                # Miscellaneous error
        .word   i$ignore                # abort from Qlogic
        .word   i$wait2restart          # online
        .word   i$go_offline            # offline
        .word   i$ignore                # timeout
        .word   i$ignore                # task abort
        .word   i$ignore                # stop event
        .word   i$ignore                # continue event
        .word   i$ignore                # logout event
#
# --- process a response received from an TUR sent to LUN 0.
#
.Ts_restart:
        .word   i$invalid               # invalid completion code
        .word   i$restart_scan          # success completion
        .word   i$restart_scan          # SCSI check status
        .word   i$restart_scan          # SCSI non check status
        .word   i$restart_scan          # I/O error
        .word   i$restart_scan          # Miscellaneous error
        .word   i$ignore                # abort from Qlogic
        .word   i$ignore                # online
        .word   i$go_offline            # offline
        .word   i$restart_scan          # timeout
        .word   i$ignore                # task abort
        .word   i$ignore                # stop event
        .word   i$ignore                # continue event
        .word   i$scan_flush            # logout event
#
# --- process a delay of a rescan of LUN 0
#
.Ts_rescanl0_wait:
        .word   i$invalid               # invalid completion code
        .word   i$rescanl0_continue     # success completion
        .word   i$ignore                # SCSI check status
        .word   i$ignore                # SCSI non check status
        .word   i$ignore                # I/O error
        .word   i$ignore                # Miscellaneous error
        .word   i$ignore                # abort from Qlogic
        .word   i$ignore                # online
        .word   i$rescanl0_offline      # offline
        .word   i$ignore                # timeout
        .word   i$ignore                # task abort
        .word   i$ignore                # stop event
        .word   i$ignore                # continue event
        .word   i$scan_flush            # logout event
#
# --- process a delay of a rescan of LUN 0 after an offline event
#
.Ts_rescanl0_offline:
        .word   i$invalid               # invalid completion code
        .word   i$scan_flush            # success completion
        .word   i$ignore                # SCSI check status
        .word   i$ignore                # SCSI non check status
        .word   i$ignore                # I/O error
        .word   i$ignore                # Miscellaneous error
        .word   i$ignore                # abort from Qlogic
        .word   i$ignore                # online
        .word   i$ignore                # offline
        .word   i$ignore                # timeout
        .word   i$ignore                # task abort
        .word   i$ignore                # stop event
        .word   i$ignore                # continue event
        .word   i$ignore                # logout event
#
# --- process a response received from an TUR sent to LUN 0.
#
.Ts_turl0:
        .word   i$invalid               # invalid completion code
        .word   i$turl0                 # success completion
        .word   i$turl0_cs              # SCSI check status
        .word   i$turl0_noncs           # SCSI non check status
        .word   i$turl0_ioe             # I/O error
        .word   i$genl0_misc            # Miscellaneous error
        .word   i$ignore                # abort from Qlogic
        .word   i$wait2restart          # online
        .word   i$go_offline            # offline
        .word   i$ignore                # timeout
        .word   i$ignore                # task abort
        .word   i$ignore                # stop event
        .word   i$ignore                # continue event
        .word   i$scan_flush            # logout event
#
# --- process a response received from an SSU sent to LUN 0.
#
.Ts_ssul0:
        .word   i$invalid               # invalid completion code
        .word   i$ssul0                 # success completion
        .word   i$genl0_cs              # SCSI check status
        .word   i$genl0_noncs           # SCSI non check status
        .word   i$genl0_ioe             # I/O error
        .word   i$genl0_misc            # Miscellaneous error
        .word   i$ignore                # abort from Qlogic
        .word   i$wait2restart          # online
        .word   i$go_offline            # offline
        .word   i$ignore                # timeout
        .word   i$ignore                # task abort
        .word   i$ignore                # stop event
        .word   i$ignore                # continue event
        .word   i$scan_flush            # logout event
#
# --- process a response received from an inquiry sent to LUN 0.
#
.Ts_inql0:
        .word   i$invalid               # invalid completion code
        .word   i$inql0                 # success completion
        .word   i$genl0_cs              # SCSI check status
        .word   i$genl0_noncs           # SCSI non check status
        .word   i$genl0_ioe             # I/O error
        .word   i$genl0_misc            # Miscellaneous error
        .word   i$ignore                # abort from Qlogic
        .word   i$wait2restart          # online
        .word   i$go_offline            # offline
        .word   i$ignore                # timeout
        .word   i$ignore                # task abort
        .word   i$ignore                # stop event
        .word   i$ignore                # continue event
        .word   i$scan_flush            # logout event
#
# --- process a delay of a rescan of LUN n
#
.Ts_rescanln_wait:
        .word   i$invalid               # invalid completion code
        .word   i$rescanln_continue     # success completion
        .word   i$ignore                # SCSI check status
        .word   i$ignore                # SCSI non check status
        .word   i$ignore                # I/O error
        .word   i$ignore                # Miscellaneous error
        .word   i$ignore                # abort from Qlogic
        .word   i$ignore                # online
        .word   i$rescanln_offline      # offline
        .word   i$ignore                # timeout
        .word   i$ignore                # task abort
        .word   i$ignore                # stop event
        .word   i$ignore                # continue event
        .word   i$scan_flush            # logout event
#
# --- process a delay of a rescan of LUN n after an offline event
#
.Ts_rescanln_offline:
        .word   i$invalid               # invalid completion code
        .word   i$scan_flush            # success completion
        .word   i$ignore                # SCSI check status
        .word   i$ignore                # SCSI non check status
        .word   i$ignore                # I/O error
        .word   i$ignore                # Miscellaneous error
        .word   i$ignore                # abort from Qlogic
        .word   i$ignore                # online
        .word   i$ignore                # offline
        .word   i$ignore                # timeout
        .word   i$ignore                # task abort
        .word   i$ignore                # stop event
        .word   i$ignore                # continue event
        .word   i$scan_flush            # logout event
#
# --- process a response received from an TUR sent to LUN n.
#
.Ts_turln:
        .word   i$invalid               # invalid completion code
        .word   i$turln                 # success completion
        .word   i$turln_cs              # SCSI check status
        .word   i$genln_noncs           # SCSI non check status
        .word   i$genln_ioe             # I/O error
        .word   i$genln_misc            # Miscellaneous error
        .word   i$ignore                # abort from Qlogic
        .word   i$wait2restart          # online
        .word   i$go_offline            # offline
        .word   i$ignore                # timeout
        .word   i$ignore                # task abort
        .word   i$ignore                # stop event
        .word   i$ignore                # continue event
        .word   i$scan_flush            # logout event
#
# --- process a response received from an SSU sent to LUN n.
#
.Ts_ssuln:
        .word   i$invalid               # invalid completion code
        .word   i$ssuln                 # success completion
        .word   i$genln_cs              # SCSI check status
        .word   i$genln_noncs           # SCSI non check status
        .word   i$genln_ioe             # I/O error
        .word   i$genln_misc            # Miscellaneous error
        .word   i$ignore                # abort from Qlogic
        .word   i$wait2restart          # online
        .word   i$go_offline            # offline
        .word   i$ignore                # timeout
        .word   i$ignore                # task abort
        .word   i$ignore                # stop event
        .word   i$ignore                # continue event
        .word   i$scan_flush            # logout event
#
# --- process a response received from an inquiry sent to LUN n.
#
.Ts_inqln:
        .word   i$invalid               # invalid completion code
        .word   i$inqln                 # success completion
        .word   i$genln_cs              # SCSI check status
        .word   i$genln_noncs           # SCSI non check status
        .word   i$genln_ioe             # I/O error
        .word   i$genln_misc            # Miscellaneous error
        .word   i$ignore                # abort from Qlogic
        .word   i$wait2restart          # online
        .word   i$go_offline            # offline
        .word   i$ignore                # timeout
        .word   i$ignore                # task abort
        .word   i$ignore                # stop event
        .word   i$ignore                # continue event
        .word   i$scan_flush            # logout event
#
# --- Process a response received from a read capacity send to LUN n
#
.Ts_rcln:
        .word   i$invalid               # invalid completion code
        .word   i$rcl                   # success completion
        .word   i$genln_cs              # SCSI check status
        .word   i$genln_noncs           # SCSI non check status
        .word   i$genln_ioe             # I/O error
        .word   i$genln_misc            # Miscellaneous error
        .word   i$ignore                # abort from Qlogic
        .word   i$wait2restart          # online
        .word   i$go_offline            # offline
        .word   i$ignore                # timeout
        .word   i$ignore                # task abort
        .word   i$ignore                # stop event
        .word   i$ignore                # continue event
        .word   i$scan_flush            # logout event
##
# --- start retry PLOGI...
#
.Ts_fabric_retry_plogi_startup:
        .word   i$ignore                # invalid completion code
        .word   i$e02                   # success completion
        .word   i$ignore                # SCSI check status
        .word   i$ignore                # SCSI non check status
        .word   i$ignore                # I/O error
        .word   i$ignore                # Miscellaneous error
        .word   i$ignore                # abort from Qlogic
        .word   i$rd_go_online          # online
        .word   i$rd_go_offline         # offline
        .word   i$ignore                # timeout
        .word   i$ignore                # task abort
        .word   i$ignore                # stop
        .word   i$ignore                # cont
        .word   i$ignore                # logout event
##
# --- waiting for timer expiration after PLOGI fail...
#
.Ts_fabric_wait_plogi_delay:
        .word   i$ignore                # invalid completion code
        .word   i$e00                   # success completion
        .word   i$ignore                # SCSI check status
        .word   i$ignore                # SCSI non check status
        .word   i$ignore                # I/O error
        .word   i$ignore                # Miscellaneous error
        .word   i$ignore                # abort from Qlogic
        .word   i$rd_go_online          # online
        .word   i$rd_go_offline         # offline
        .word   i$e02                   # timeout
        .word   i$ignore                # task abort
        .word   i$ignore                # stop
        .word   i$ignore                # cont
        .word   i$ignore                # logout event
##
# --- waiting for PLOGI response...
#
.Ts_fabric_wait_plogi_resp:
        .word   i$ignore                # invalid completion code
        .word   i$e03                   # success completion
        .word   i$ignore                # SCSI check status
        .word   i$ignore                # SCSI non check status
        .word   i$ignore                # I/O error
        .word   i$e04                   # Miscellaneous error
        .word   i$e01                   # abort
        .word   i$rd_go_online          # online
        .word   i$rd_go_offline         # offline
        .word   i$ignore                # timeout
        .word   i$ignore                # task abort
        .word   i$ignore                # stop
        .word   i$ignore                # cont
        .word   i$ignore                # logout event
##
# --- flush pending after offline...
#
.Ts_fabric_retry_plogi_offline:
        .word   i$ignore                # invalid completion code
        .word   i$e05                   # success completion
        .word   i$ignore                # SCSI check status
        .word   i$ignore                # SCSI non check status
        .word   i$ignore                # I/O error
        .word   i$e05                   # Miscellaneous error
        .word   i$e05                   # abort from Qlogic
        .word   i$rd_go_online          # online
        .word   i$rd_go_offline         # offline
        .word   i$e05                   # timeout
        .word   i$ignore                # task abort
        .word   i$ignore                # stop
        .word   i$ignore                # cont
        .word   i$ignore                # logout event
##
# --- flush pending stuff after online, no TMT...
#
.Ts_fabric_retry_plogi_online:
        .word   i$ignore                # invalid completion code
        .word   i$e05                   # success completion
        .word   i$ignore                # SCSI check status
        .word   i$ignore                # SCSI non check status
        .word   i$ignore                # I/O error
        .word   i$ignore                # Miscellaneous error
        .word   i$e05                   # abort
        .word   i$ignore                # online
        .word   i$ignore                # offline
        .word   i$e05                   # timeout
        .word   i$ignore                # task abort
        .word   i$ignore                # stop
        .word   i$ignore                # cont
        .word   i$ignore                # logout event

#######################################################################
#
#       Normal Operation Event Handler tables
#
#######################################################################
#
# --- task is on the queue waiting to be executed. Only a gross timer
#     expiration or a abort operation should occur.
#
.Ts_wait2activate:
        .word   i$invalid               # invalid completion code
        .word   i$ignore                # success completion
        .word   i$ignore                # SCSI check status
        .word   i$ignore                # SCSI non check status
        .word   i$ignore                # I/O error
        .word   i$ignore                # Miscellaneous error
        .word   i$ignore                # abort from Qlogic
        .word   i$ignore                # online
        .word   i$ignore                # offline
        .word   apl$domtsk_abort        # gross timer expiration
        .word   apl$domtsk_abort        # abort from application
        .word   i$ignore                # stop event
        .word   i$ignore                # continue event
        .word   apl$path_logout         # logout event


# --- task has been executed and is waiting for a response
#     from the target.
#
.Ts_cmd_rsp:
        .word   i$invalid               # invalid completion code
        .word   apl$tsk_cmplt_success   # success completion
        .word   apl$tsk_cmplt_CS        # SCSI check status
        .word   apl$tsk_cmplt_nonCS     # SCSI non check status
        .word   apl$tsk_cmplt_IOE       # I/O error
        .word   apl$tsk_cmplt_misc      # Miscellaneous error
        .word   apl$tsk_cmplt_IOE       # abort from Qlogic
        .word   i$ignore                # online
        .word   i$ignore                # offline
        .word   apl$acttsk_abort        # gross timer expiration
        .word   apl$acttsk_2abortQ      # abort from application
        .word   i$ignore                # stop event
        .word   i$ignore                # continue event
        .word   apl$path_logout         # logout event
#
# --- task has been aborted by the originator and is waiting for
#     the Qlogic completion before resources are released
#
.Ts_cmd_abort:
        .word   i$invalid               # invalid completion code
        .word   apl$tsk_cmplt_success   # success completion
        .word   apl$tsk_cmplt_CS        # SCSI check status
        .word   apl$tsk_cmplt_nonCS     # SCSI non check status
        .word   apl$tsk_cmplt_IOE       # I/O error
        .word   apl$tsk_cmplt_misc      # Miscellaneous error
        .word   apl$tsk_aborted         # abort from Qlogic
        .word   i$ignore                # online
        .word   i$ignore                # offline
        .word   apl$abort_task1         # gross timer expiration
        .word   i$ignore                # abort from application
        .word   i$ignore                # stop event
        .word   i$ignore                # continue event
        .word   apl$path_logout         # logout event
#
# --- task has been aborted after a gross timeout, we are
#     waiting for the task aborted status from the Qlogic.
#     if the task completes with any other status, handle
#     that status normally.
#
.Ts_cmd_TO:
        .word   i$invalid               # invalid completion code
        .word   apl$tsk_cmplt_success   # success completion
        .word   apl$tsk_cmplt_CS        # SCSI check status
        .word   apl$tsk_cmplt_nonCS     # SCSI non check status
        .word   apl$tsk_cmplt_IOE       # I/O error
        .word   apl$tsk_cmplt_misc      # Miscellaneous error
        .word   apl$tsk_aborted         # abort from Qlogic
        .word   i$ignore                # online
        .word   i$ignore                # offline
        .word   i$ignore                # gross timer expiration
        .word   i$ignore                # abort from application
        .word   i$ignore                # stop event
        .word   i$ignore                # continue event
        .word   apl$path_logout         # logout event
#
        .text
#
##############################################################################
#
# --- beginning of code -------------------------------------------------------
#
#******************************************************************************
#
#  NAME: I$init
#
#  PURPOSE:
#       Initializes the Initiator driver environment.
#
#  DESCRIPTION:
#       The executive process is established and made ready
#       for execution.
#
#  CALLING SEQUENCE:
#       call    I$init
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
I$init:
#
# --- Clear local data structures
#
        ldconst 0,r3                    # User r3 as zero
        st      r3,I$IDFLAG             # clear the WWID active flag
        st      r3,I$WWN                # clear WWID
        st      r3,I$WWN+4
        st      r3,I$NID                # Clear node ID

        st      r3,i_mintbl             # clear minimum table address
        st      r3,i_maxtbl             # clear maximum table address

        st      r3,I_undef_event        # clear counter
#
# --- Establish executive process
#
        movl    g0,r14                  # save g0-g1
        lda     i$exec,g0               # establish executive process
        lda     IEXECPRI,g1
c       CT_fork_tmp = (ulong)"i$exec";
        call    K$tfork
        st      g0,I_exec_pcb           # save PCB address
        movl    r14,g0                  # restore g0-g1
        ret
#
#******************************************************************************
#
#  NAME: i$exec
#
#  PURPOSE:
#       MAG device driver executive process.
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
i$exec:
#
# --- Calculate and Allocate memory for TMTs, TLMTs, ISMTs, XLIs.
#
# r4 = count of items to get
# r7 = i_mintbl
# r8 = i_maxtbl
c       r7 = (UINT32)&local_memory_start;   # We have lower address of this.
c       r8 = (UINT32)&local_memory_start;   # Following allocations moves up from here.
        st      r7,i_mintbl             # save minimum table address
        st      r8,i_maxtbl             # save maximum table address
#
c       init_tmt(TMTMAX);               # Initialize TMT memory pool.
c       init_tlmt(TLMTMAX);             # Initialize TLMT memory pool.
c       init_ismt(ISMTMAX);             # Initialize ISMT memory pool.
c       init_xli(XLIMAX);               # Initialize XLI memory pool.
#
# --- Get memory for ICIMTs.
#
c       g0 = local_perm_malloc(ICIMTMAX * icimtsize, "ICIMT", 0); # initial ICIMT memory allocation.
c       memset((void *)g0, 0, ICIMTMAX * icimtsize);    # clear memory
c       r6 = 0;                         # r6 is count of ICIMTs
c       r8 = Cs_offline                 # r8 = "offline" state
        lda     I_CIMT_dir,r9           # r9 = base address of ICIMT dir.
.iex10:
        st      g0,(r9)[r6*4]           # save ICIMT address in directory
        stob    r6,ici_chpid(g0)        # save ICIMT #
        stob    r8,ici_state(g0)        # save idriver states table address
c       g4 = g0;                        # g4 is icimit argument for i$init_ici_lidtbl
        call    i$init_ici_lidtbl       # init lid table
c       r3 = 0;
        stob    r3,ici_ftenable(g0)     # MAKE SURE FOREIGN TARGET ENABLE IS CLEAR
c       r6 = r6 + 1;                    # increment ICIMT working on.
c       g0 = g0 + icimtsize;            # increment to next ICIMT memory
        cmpobne ICIMTMAX,r6,.iex10      # Jif more ICIMTs to initialize
#
.if     ITRACES
#
# --- Allocate trace areas and initialize
#
        ldconst TRACESIZE,r8            # r8 = trace area size
        mov     ICIMTMAX,r4             # r4 = # trace areas to initialize
        lda     I_CIMT_dir,r9           # r9 = base address of ICIMT dir.
        mov     0,r6                    # r6 = ICIMT dir. index trace area
                                        #  is being initialized
        ldconst DEF_ITFLG,r15           # r15 = default trace flags
        ldconst de_itrace0,r12          # r12 = offset into DDR
.ex100_i:
c       g0 = s_MallocC(r8, __FILE__, __LINE__); # allocate & clear trace memory
        mov     g0,r11                  # save buffer pointer
        addo    r6,r9,r10               # r10 = ICIMT_dir pointer
        ld      (r10),r10               # r10 = ICIMT trace area is being
                                        #  initialized for
        st      g0,ici_curtr(r10)       # save trace area pointer as current
        st      g0,ici_begtr(r10)       # save trace area pointer as beginning
        lda     TRACESIZE-trr_recsize(g0),g0 # g0 = end trace area pointer
        st      g0,ici_endtr(r10)       # save trace area pointer as ending
        ldconst 0x20637254,r3           # r3 = "Trc" banner
        stos    r15,ici_tflg(r10)       # save trace flag settings
        stos    r15,ici_dftflg(r10)     # save default trace flag settings
        st      r3,ici_trbnr(r10)       # save the banner
#
# --- Initialize a Debug Data Retrieval (DDR) itrace entry
#
        mov     r12,g0                  # Load DDR table offset
        mov     r11,g1                  # Load itrace address
        mov     r8,g2                   # Load itrace size
c       M_addDDRentry(g0, g1, g2);
        addo    1,r12,r12               # Increment DDR table offset
#
        subo    1,r4,r4                 # dec. ICIMT counter
        addo    4,r6,r6                 # inc. ICIMT_dir index
        cmpobne.t 0,r4,.ex100_i         # Jif more ICIMTs to initialize trace
                                        #  area for
#
.endif  # ITRACES
#
# --- Task initialization completed
#
        ret                             # end task
#
#******************************************************************************
#
#  NAME: I$recv_online
#
#  PURPOSE:
#       To provide an interface with the ISP layer to allow an online signal.
#
#  CALLING SEQUENCE:
#       call    I$recv_online
#
#  INPUT:
#       g0 = chip instance
#       g1 = LID of this initiators interface.
#       g2 = online event type
#               11 = Online due to Loop UP event
#               14 = Online due to DB change event
#               15 = Online due to State Change Notification
#               f0 = Online due to Enable Foreign Target
#               f1 = Online due to loss of port
#                       0x29 or 0x2a status during data transfer
#
#       note: lpmap will also be initialized with the loop PA map.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Registers g0, g1 are destroyed
#
#******************************************************************************
#
I_recv_online:
I$recv_online:
        ld      iscsimap,r5             # r5 = iscsi port map
        bbs     g0,r5,.rcv_onl_iscsi       # Jif not iSCSI port
        ld      I_CIMT_dir[g0*4],g4     # g4 = ICIMT dir pointer
        call    i$init_ici_lidtbl       # set up fabric lid table
        PushRegs(r5)
        call    I_Online
        PopRegsVoid(r5)
        ret

.rcv_onl_iscsi:
c fprintf(stderr,"%s%s:%u _I_recv_online PORT %lx lid %06lX event %02lx\n", FEBEMESSAGE, __FILE__, __LINE__,g0,g1,g2);

        movq    g4,r8                   # save g4-g7 in r8-r11
        ld      I_CIMT_dir[g0*4],g4     # g4 = ICIMT dir pointer


        ld      lpmap[g0*4],r12         # r12 = address of AL map for this chip instance
# --- We don't need  portid for icl port.
c       r5  =   ICL_IsIclPort(g0);
        ldconst 0,r13
        cmpobe  TRUE,r5,.rcv_onl_icl01  # Jif ICL port
        ld      portid[g0*4],r13        # r13 = address of port  ID for this chip instance
        bswap   r13,r13                 # change endianess
.rcv_onl_icl01:

        stob    g1,ici_mylid(g4)        # save my LID for this interface
        st      r12,ici_lpmapptr(g4)    # save address of AL map in ICIMT
        st      r13,ici_mypid(g4)       # save my pid for this interface

        ldconst 5,r5                    # r5 = 5
        stob    r5,ici_rcnt(g4)         # set rescan count to 5
        ldconst Cs_online,r5            # r5 = "online" state
        stob    r5,ici_state(g4)        # set loop online

.if     ITRACES
        call    it$trc_online           # *** trace online event ***
.endif  # ITRACES
#
# --- establish timer task
#
        ldob    ici_tmrctl(g4),r4       # get timer control byte
        ldconst TRUE,r5                 # r5 = non zero value
        cmpobe.t TRUE,r4,.rcv_onl_000   # Jif timer already setup
        stob    r5,ici_tmrctl(g4)       # set timer active

        movl    g0,r14                  # save g0-g1
        lda     apl$timer,g0            # establish timer process
        lda     APLTIMERPRI,g1
c       CT_fork_tmp = (ulong)"apl$timer";
        call    K$fork
        st      g0,APL_timer_pcb        # save apl$timer PCB address
        movl    r14,g0                  # restore g0-g1
#
# --- Check if the port type is iSCSI. If yes, call initLPMap which will launch
#     iscsi logins for the targets on the other controller.
#
.rcv_onl_000:
c       r5= ICL_IsIclPort((UINT8)g0);
        cmpobe  TRUE,r5,.rcv_onl_icl06
        ld      iscsimap,r5            # r5 = iscsi port map
        bbc     g0,r5,.rcv_onl_002     # Jif not iSCSI port
.rcv_onl_icl06:
        PushRegs(r3)
        mov     g0,r5                  # save chip instance in r5
        call    fsl_resetLPMap
        mov     r5,g0                  # chip instance in g0
        call    fsl_initLPMap
        PopRegsVoid(r3)
        b       .rcv_onl_1000
#
# --- determine if the online event is a port database change. If it is, determine
#     if the FC port is in N, NL, F, or FL mode. If the FC port is in F or FL mode,
#     bypass device discovery. Otherwise do a device discovery.
#
.rcv_onl_002:
#       cmpobne  ispodbc,g2,.rcv_onl_005 # Jif not port database change

#       ld      ispfflags,r5            # r5 = fabric support map
#       bbs     g0,r5,.rcv_onl_1000     # Jif F or FL-port
#
# --- reinitialize the free LID table
#
# .rcv_onl_005:
        call    i$init_ici_lidtbl       # set up fabric lid table
        PushRegs(r3)
        call    I_Online
        PopRegsVoid(r3)
        b       .rcv_onl_1000
#
# --- set all active TMT's to inactive state and set device lost timer on
#     all TMT's if not already active.
#

        ld      ici_tmtQ(g4),g5         # g5 = possible TMT
        cmpobe.f 0,g5,.rcv_onl_050      # Jif no link

        ldconst CNTLTO,r6               # r6 = (default) controller t/o value
        ldconst ispofte,r3              # r3 = foreign target enable event type
        cmpobne.t r3,g2,.rcv_onl_010    # Jif not foreign target enable

        ldconst FTDISCTO,r6             # r6 = foreign target enable t/o value

.rcv_onl_010:
        ldconst tmstate_inactive,r5     # r5 = inactive state
        ldob    tm_state(g5),r3         # get current state

.ifdef M4_ADDITION
c       if (r3 == tmstate_deltar) {     # Target is in process of being deleted
# c           fprintf(stderr, "%s%s:%u rcv_onl_010 TMT 0x%08lx in process of being deleted\n", FEBEMESSAGE,__FILE__, __LINE__, g5);
            b   .rcv_onl_016            # Go to possible next TMT
c       }
.endif  # M4_ADDITION
        cmpobe tmstate_active,r3,.rcv_onl_011 # JIf active state

        cmpobne tmstate_discovery,r3,.rcv_onl_012 # JIf not discovery state

        ldconst tmstate_undiscovered,r5 # r5 = undiscovered state

.rcv_onl_011:
        stob    r5,tm_state(g5)         # set new TMT state

        ldob    tm_flag(g5),r5          # r3 = flag byte
        setbit  tmflg_lundisc,r5,r5     # set LUN discovery request flag
        stob    r5,tm_flag(g5)          # save

.rcv_onl_012:
#        cmpobe tmstate_server,r3,.rcv_onl_015 # JIf server state
        cmpobe tmstate_retry_plogi,r3,.rcv_onl_015 # JIf redisc state - no timer popping
#
        ldos    tm_tmr0(g5),r3          # get current timer value
        cmpobne.t 0,r3,.rcv_onl_015     # Jif already active
        stos    r6,tm_tmr0(g5)          # save t/o value

#
# --- set loop id used by this device
#
.rcv_onl_015:
        ldos    tm_lid(g5),g0           # get g0 = lid
        call    i$use_lid               # use this  loop ID

.ifdef M4_ADDITION
.rcv_onl_016:
.endif  # M4_ADDITION
        ld      tm_link(g5),g5          # g5 = next link
        cmpobne.t 0,g5,.rcv_onl_010     # Jif there is a link
#
# --- set all TLMT's inactive (Devices)
#
        ld      ici_actqhd(g4),g6       # g6 = possible TLMT
        ldconst tlmstate_inactive,r5    # r5 = inactive state
        cmpobe.f 0,g6,.rcv_onl_050      # Jif no link

.rcv_onl_020:
        stob    r5,tlm_state(g6)        # set TLMT inactive

        ld      tlm_flink(g6),g6        # g5 = next link
        cmpobne.t 0,g6,.rcv_onl_020     # Jif there is a link
#
# --- check for discovery process. If one is active, issue event to that task.
#     Otherwise, start a discovery process.
#
.rcv_onl_050:
        ld      ici_disQ(g4),r15        # r15 = discovery queue
        cmpobe.t 0,r15,.rcv_onl_200     # JIf no discovery in process
        ldconst FALSE,r14               # r14 = (FALSE) parent task not present

.rcv_onl_100:
        lda     ILTBIAS(r15),g1         # g1 = ILT at 2nd level
        ldconst Te_online,r13           # r13 = event
        ld      oil2_ehand(g1),r12      # r12 = ICIMT event handler address

        ldob    oil1_tmode(r15),r4      # r4 = flags
        cmpobne.t oimode_parent,r4,.rcv_onl_110 # Jif not parent discovery task

        ldob    oil1_flag(r15),r4       # r4 = flags byte
        setbit  oiflg_PDBC,r4,r4        # set port data base changed indication
        stob    r4,oil1_flag(r15)       # save save new value
        ldconst TRUE,r14                # r14 = (TRUE) parent task present

.rcv_onl_110:
        ld      oil1_dslink(r15),r15    # r15 = next ILT

        addo    r12,r13,r12             # add completion event type
        ld      (r12),r12
        callx   (r12)                   # process event
                                        #   g1 = ILT at 2nd lvl
                                        #   g4 = ICIMT

        cmpobne.t 0,r15,.rcv_onl_100    # Jif another ILT
        cmpobe.t  TRUE,r14,.rcv_onl_1000 # Jif discovery process already active

.rcv_onl_200:
        call    i$discovery             # start discovery process

.rcv_onl_1000:
        movq    r8,g4                   # restore g4-g7 from
        ret
#
#******************************************************************************
#
#  NAME: I_recv_offline
#
#  PURPOSE:
#       To provide an interface with the ISP2100 layer to allow an offline
#       signal.
#
#  CALLING SEQUENCE:
#       call    I_recv_offline
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
I_recv_offline:
.if 0
        ld      iscsimap,r5             # r5 = iscsi port map
        bbc     g0,r5,.rcv_off_fc       # Jif not iSCSI port
c fprintf(stderr,"%s%s:%u _I_recv_offline ignored for iSCSI PORT %lx\n", FEBEMESSAGE, __FILE__, __LINE__,g0);
        ret
.rcv_off_fc:
.endif  # 0
        movl    g0,r10                  # save g0-g1
        movq    g4,r12                  # save g4-g7
        ld      I_CIMT_dir[g0*4],g4     # g4 = ICIMT dir pointer

        ld      iscsimap,r5             # r5 = iscsi port map
        bbs     g0,r5,.rcv_off_000      # Jif iSCSI port

        call    i$init_ici_lidtbl         # set up fabric lid table
        PushRegs(r3)
        call    I_Offline
        PopRegsVoid(r3)
        b       .rcv_off_1000
#
.rcv_off_000:
.if ICL_DEBUG
        cmpobne ICL_PORT,g0,.rcv_off_icl01
c fprintf(stderr,"%s%s:%u <I_recv_offline>ICL port ... from ISP_GenOffline\n", FEBEMESSAGE, __FILE__, __LINE__);
.rcv_off_icl01:
.endif  # ICL_DEBUG
#
# --- set channel offline
#
        ldconst Cs_offline,r4           # r4 = "offline" state
        stob    r4,ici_state(g4)        # save new state
#
# --- reinitialize the free LID table
#
        PushRegs(r3)
        call    fsl_resetLPMap
        PopRegsVoid(r3)
#
# --- set all TMT's inactive (ports)
#
        ld      ici_tmtQ(g4),g5         # g5 = possible TMT
        ldconst CNTLTO,r6               # r6 = controller t/o value
        cmpobe.f 0,g5,.rcv_off_200      # Jif no link

.rcv_off_100:
        ldob    tm_flag(g5),r3          # r3 = flag byte
        setbit  tmflg_lundisc,r3,r3     # set LUN discovery request flag
        stob    r3,tm_flag(g5)          # save
.ifdef M4_ADDITION
c       if (((TMT*)g5)->state == tmstate_deltar) { # Target is in process of being deleted
# c           fprintf(stderr, "%s%s:%u rcv_off 100 TMT 0x%08lx in process of being deleted\n", FEBEMESSAGE,__FILE__, __LINE__, g5);
            b   .rcv_off_120            # Go to possible next TMT
c       }
.endif  # M4_ADDITION
        ldconst tmstate_inactive,r5     # r5 = inactive state
        stob    r5,tm_state(g5)         # set TMT inactive

        ldos    tm_lid(g5),r5           # r5 = lid
        cmpobe.t NO_LID,r5,.rcv_off_110 # Jif already set to no lid

        ldconst 0,r3
        st      r3,ici_tmdir(g4)[r5*4]  # clear TMT in CIMT dir
        ldconst NO_LID,r3
        stos    r3,tm_lid(g5)           # Invalidate lid

.rcv_off_110:
        ldos    tm_tmr0(g5),r3          # get current timer value
        cmpobne.t 0,r3,.rcv_off_120     # Jif already active
        stos    r6,tm_tmr0(g5)          # save t/o value

.rcv_off_120:
        ld      tm_link(g5),g5          # g5 = next link
        cmpobne.t 0,g5,.rcv_off_100     # Jif there is a link
#
# --- set all TLMT's inactive (Devices)
#
        ld      ici_actqhd(g4),g6       # g6 = possible TLMT
        ldconst tlmstate_inactive,r5    # r5 = inactive state
        cmpobe.f 0,g6,.rcv_off_200      # Jif no link

.rcv_off_150:
        stob    r5,tlm_state(g6)        # set TLMT inactive

        ld      tlm_flink(g6),g6        # g5 = next link
        cmpobne.t 0,g6,.rcv_off_150     # Jif there is a link
#
# --- set node and port ID's invalid (WWID)
#
.rcv_off_200:
        ldconst FALSE,r4                # r4 = false indicator
        stob    r4,I$IDFLAG             # set values active

.if     MAG2MAG
        mov     r10,g0                  # restore g0 = chip instance
        call    LLD$offline             # notify link manager of offline event
                                        # input:
                                        #        g0 = chip instance
                                        # output:
                                        #        none.
.endif  # MAG2MAG

.if     ITRACES
        call    it$trc_offline          # *** trace offline event ***
.endif  # ITRACES

        ld      ici_disQ(g4),r7         # get possible ILT from discovery queue
        cmpobe.t 0,r7,.rcv_off_1000     # Jif no entries
rcv_off_500:
        lda     ILTBIAS(r7),g1          # g1 = ILT at 2nd level
        ldconst Te_offline,r9           # r9 = event
        ld      oil2_ehand(g1),r8       # r8 = ICIMT event handler address
        ld      oil1_dslink(r7),r7      # r7 = next ILT

        addo    r8,r9,r8                # add completion event type
        ld      (r8),r8
        callx   (r8)                    # process event
                                        #   g1 = ILT at 2nd lvl
                                        #   g4 = ICIMT
        cmpobne.t 0,r7,rcv_off_500      # Jif another ILT
.rcv_off_1000:
        movl    r10,g0                  # restore g0-g1
        movq    r12,g4                  # restore g4-g7
        ret
#
#******************************************************************************
#
#  NAME: I$recv_rinit
#
#  DESCRIPTION:
#       Reset and initialize port event
#
#  CALLING SEQUENCE:
#       call    I$recv_rinit
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
I_recv_rinit:
        movl    g0,r12                  # Save g0,g1
        movl    g4,r14                  # Save g4,g5

        ld      I_CIMT_dir[g0*4],g4     # ICIMT
        cmpobe.t 0,g4,.rcv_ri_100       # Jif no ICIMT
        ldconst FALSE,r11               # Clear 'TMT not deleted' flag
#
# --- Check the TMT queue for this port
#
        ld      ici_tmtQ(g4),g5         # g5 = possible TMT
        cmpobe.t 0,g5,.rcv_ri_100       # Jif no or last entry on queue
#
# --- Check if associated TMT exists.
#     If so, clear WWN so TMT is not reused.
#
.rcv_ri_10:
        ld      tm_link(g5),r10         # r10 = next tmt in queue
#
# --- Delete the inactive TMT
#
.if ICL_DEBUG
        cmpobne ICL_PORT,g0,.rcv_ri_icl01
c fprintf(stderr,"%s%s:%u <I_recv_rinit>Deleting TMT for ICL..\n", FEBEMESSAGE, __FILE__, __LINE__);
.rcv_ri_icl01:
.endif  # ICL_DEBUG
                                        # g5 = TMT
        call    i$del_tmt               # Delete TMT
        cmpobne FALSE,g0,.rcv_ri_20     # Jif TMT deleted
#
# --- TMT was not deleted.
#
        ldconst TRUE,r11                # Indicate IMT was not deleted.
#
# --- Advance to next TMT on inactive list
#
.rcv_ri_20:
        mov     r10,g5
        cmpobne.f 0,g5,.rcv_ri_10        # jif there is another tmt
#
# --- Check if any IMTs or TMTs still need to be deleted.
#
        cmpobe  FALSE,r11,.rcv_ri_100
        ld      i_cleanup_pcb,r3
        cmpobne 0,r3,.rcv_ri_100        # Jif process is running
c       g0 = -1;                        # Flag process being created.
        st      g0,i_cleanup_pcb
        lda     i$tmt_cleanup,g0        # establish cleanup process
        ldconst ITCPRI,g1
c       CT_fork_tmp = (ulong)"i$tmt_cleanup";
        call    K$fork
        st      g0,i_cleanup_pcb

.rcv_ri_100:
        movl    r12,g0                  # Restore g0,g1
        movl    r14,g4                  # Restore g4,g5
        ret
#
#******************************************************************************
#
#  NAME: i$tmt_cleanup
#
#  PURPOSE:
#       This routine deletes all inactive IMT for the specified port.
#
#  DESCRIPTION:
#       This routine demolishes an image (TMT).  It deallocates
#       all resources associated with the image including
#       ILMTs and TMT.
#
#  CALLING SEQUENCE:
#       call    i$tmt_cleanup
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g0,g5.
#
#******************************************************************************
#
i$tmt_cleanup:
#
# ---  Check TMT queue on all ports.  Start with port zero.
#
.itc10:
        ldconst FALSE,r13               # Clear 'TMT to delete' flag
        ldconst 0,r15                   # start with port zero
.itc20:
        ld      I_CIMT_dir[r15*4],g4    # ICIMT
        cmpobe.t 0,g4,.itc50            # Jif ICIMT does not exist
#
# --- Check the TMT queue for this port
#
        ld      ici_tmtQ(g4),g5         # g5 = possible TMT
        cmpobe.t 0,g5,.itc50            # Jif no or last entry on queue
#
# --- Check if the WWN for this TMT is zero.  This is a TMT that
#     was previously attempted to be deleted, but was still in use.
#
.itc30:
        ld      tm_link(g5),r12         # r12 = next tmt in queue

        ldl     tm_N_name(g5),r4        # Get node name of TMT
        cmpobne 0,r4,.itc40             # Jif WWN not zero
        cmpobne 0,r5,.itc40             # Jif WWN not zero
#
# --- Delete the inactive TMT
#
        call    i$del_tmt               # Delete TMT
        cmpobne FALSE,g0,.itc40         # Jif TMT not deleted
        ldconst TRUE,r13                # Indicate IMT was not deleted.
#
# --- Advance to next TMT on inactive list
#
.itc40:
        mov     r12,g5
        cmpobne.f 0,g5,.itc30           # jif there is another tmt
#
# --- Advance to next port
#
.itc50:
        addo    1,r15,r15
        cmpobg  MAXCHN,r15,.itc20
#
# --- Check if any TMTs still need to be deleted.
#
        cmpobe  FALSE,r13,.itc100
#
# --- Wait 1/4 seconds and try to delete TMTs again
#
        lda     250,g0                  # set up to wait 250 ms
        call    K$twait                 # delay task
        b       .itc10
#
# --- Exit
#
.itc100:
        ldconst 0,r3
        st      r3,i_cleanup_pcb        # Clear PCB pointer
        ret
#
#******************************************************************************
#
#  NAME: I_logout
#
#  PURPOSE:
#       This routine deletes all inactive IMT for the specified port.
#
#  DESCRIPTION:
#       This routine demolishes an image (TMT).  It deallocates
#       all resources associated with the image including
#       ILMTs and TMT.
#
#  CALLING SEQUENCE:
#       call    I_logout
#
#  INPUT:
#       g0 = TMT
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g0,g5.
#
#******************************************************************************
#
I_logout:
        movq    g4,r12

        cmpobe.f 0,g0,.ilogout_100      # Jif no link
#
?# Crash 2012-01-05, g0 points to TMT that is freed.
        ldob    tm_chipID(g0),r5        # r5 = chip ID
        ld      I_CIMT_dir[r5*4],g4     # ICIMT
        cmpobe.t 0,g4,.ilogout_100      # Jif no ICIMT

#
# --- Remove the TMT from the ICIMT active TMT queue
#
        ld      ici_tmtQ(g4),g5         # g5 = possible TMT
        lda     ici_tmtQ(g4),r5         # r5 = address of tmt queue
#
.ilogout_025:
        cmpobe.t 0,g5,.ilogout_100      # Jif ICIMT active TMTQ is NULL
        cmpobe  g5,g0,.ilogout_050
        mov     g5,r5                   # r5 = current tmt address
?# Crash 2012-01-03, g5 is   tmtQ = 0xffff6085.
        ld      tm_link(g5),g5          # g5 = next tmt in queue
        b       .ilogout_025
#
# --- unlink tmt from queue
#
.ilogout_050:
        ld      tm_link(g0),g5          # g5 = link to next tmt
        st      g5,(r5)                 # save link in previous tmt
        mov     g0,g5

        ldconst 0,r3
        ldos    tm_lid(g5),r5           # g1 = lid
        st      r3,ici_tmdir(g4)[r5*4]  # clear TMT in CIMT dir
        stos    r3,tm_tmr0(g5)          # clear t/o value
#
# --- Delete the inactive TMT
#
.ilogout_099:
.if ICL_DEBUG
        ldob tm_chipID(g5),r5
        cmpobne ICL_PORT,r5, .ilogout_icl01
c fprintf(stderr,"%s%s:%u <I_logout>Deleting TMT for ICL port...\n", FEBEMESSAGE, __FILE__, __LINE__);
.ilogout_icl01:
.endif  # ICL_DEBUG
        call    i$del_tmt               # Delete TMT
        cmpobe  TRUE,g0,.ilogout_100    # Jif TMT deleted
#
# --- Wait 1/4 seconds and try to delete TMTs again
#
        lda     250,g0                  # set up to wait 250 ms
        call    K$twait                 # delay task
        b       .ilogout_099

.ilogout_100:
        movq    r12,g4
        ret
#
#******************************************************************************
#
#  NAME: I_login
#
#  PURPOSE:
#       This routine kicks off the initiator state machine with iSCSI login
#
#  DESCRIPTION:
#       This routine creates the resources associated to establishing the
#       path to the target.
#
#  CALLING SEQUENCE:
#       call    I_login
#
#  INPUT:
#       g0 = chip instance
#       g1 = LID
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g0,g1.
#
#******************************************************************************
#
I_login:
# c fprintf(stderr,"%s%s:%u I_LOGIN\n", FEBEMESSAGE, __FILE__, __LINE__);
        movq    g4,r12
        ld      I_CIMT_dir[g0*4],g4     # ICIMT
        cmpobe.t 0,g4,.ilogin_100       # Jif no ICIMT

        ldconst 0,r7
        mov     g1,r6                   # r6=lid
        mov     g0,r5                   # r5=chipID
c       r4 = isp_handle2alpa(r5, r6);

        ldconst 512,g0                  # g0 = set size for SGL
        call    i$alloc_dis_resource    # g1 = ILT for operation (at 1st level)


        st      r7,oil1_chpid(g1)       # clear chipid, LID, LUN, alpaindx
        st      r7,oil1_tmt(g1)         # clear tmt address
        st      r4,oil1_pid(g1)         # save PID (ALPA)
        stob    r5,oil1_chpid(g1)       # save  chipid
        stos    r6,oil1_lid(g1)         # save  LID,
        setbit  oiflg_LID,r7,r7         # set LID only in flags
        setbit  oiflg_LUN,r7,r7         # set LUN only in flags
        stob    r7,oil1_flag(g1)        # save flag byte
        lda     i$ploop_cr,r4           # r4 = address of completion routine
        st      r4,il_cr(g1)            # save completion return address
        ldconst oimode_iscsi,r4         # r4 = iscsi discovery task
        stob    r4,oil1_tmode(g1)       # save task as child

        lda     ILTBIAS(g1),g1          # bump to lvl 2 of ILT nest

        ldconst .Ts_login,r4            # r4 = start event table
        st      r4,oil2_ehand(g1)       # save it
        lda     I$recv_cmplt_rsp,r4     # r4 = completion routine
        st      r4,oil2_cr(g1)          # Save completion routine
        ld      oil2_xli(g1),r4         # r4 = pointer to xli
        stob    r5, xlichipi(r4)

        PushRegs(r3)
        mov     g1,g0
        call    fsl_login
        PopRegsVoid(r3)

.ilogin_100:
        movq    r12,g4
        ret
#
#******************************************************************************
#
#  NAME: I$recv_cmplt_rsp
#
#  DESCRIPTION:
#       A small amount of convention for commonly used tables:
#               g4 = ICIMT
#               g5 = TMT
#               g6 = TLMT
#               g7 = CDB
#
#  CALLING SEQUENCE:
#       call    I$recv_cmplt_rsp
#
#  INPUT:
#       g0 = status
#       g1 = ILT at Initiator level 2.
#       g11 = status type 0 IOCB (if g0 <> 0)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
I$recv_cmplt_rsp:
        PushRegs(r3)                    # Save all G registers

        ld      oil2_xli(g1),g7         # g7 = pointer to xli
        ldob    xlichipi(g7),g10        # g10 = chip instance
        ldconst Te_success,g9           # g9 = default success
        ld      I_CIMT_dir[g10*4],g4    # g4 = ICIMT dir pointer
        cmpobe.t 0,g0,.rcr_100          # Jif ICIMT is NULL
#
# --- Check if the port type is iSCSI. If yes, skip the IOCB status check
# --- If there is error, the kernel dispatcher puts the error in g2, in case
#     of iSCSI
#
c       g8 = ICL_IsIclPort((UINT8)g10);
        cmpobe  TRUE, g8,.rcr_icl03
        ld      iscsimap,g8            # g8 = iscsi port map
        bbc     g10,g8,.rcr_050        # Jif not iSCSI port
.rcr_icl03:
        mov     g2,g9                  # g9 = status
        lda     -ILTBIAS(g1),r15        # r15 = ilt at 1st level
        ldos    oil1_lid(r15),r7        # r7 = lid
        ld      ici_tmdir(g4)[r7*4],g5  # g5 = TMT address from cimt lid dir
        cmpobe.t 0,g5,.rcr_100          # Jif TMT is NULL
        ldob    oil1_lun(r15),r6        # r6 = current lun
        ld      tm_tlmtdir(g5)[r6*4],g6 # g6 = possible TLMT address
        b      .rcr_100
#
.rcr_050:
        ldob    0x8(g11),r4             # iocb completion status
        ldob    0x16(g11),g0            # g0 = SCSI status
#c fprintf(stderr,"idriver: I$recv_cmplt_rsp scsi status %02lX iocb stat %02lX\n",g0,r4);
        ldob    scsi_st_norm[g0*1],g9   # g9 = normalized SCSI status
        cmpobne.f Te_success,g9,.rcr_100 # Jif if SCSI status not OK
#
        mov     r4,g0                   # g0 = completion status
        ldob    cmplt_st_norm[g0*1],g9  # g9 = normalized cmplt status
#
.rcr_100:
.if     ITRACES
        call    it$trc_icmdcmplt        # *** trace cmd cmplt event ***
.endif  # ITRACES

        ld      oil2_ehand(g1),g10      # g10 = ICIMT event handler routine
        addo    g10,g9,g10              # add completion event type
        ld      (g10),g10
        callx   (g10)                   # process event
                                        #   g0 = SCSI or Qlogic completion status
                                        #   g1 = ILT at initiator level 2
                                        #   g4 = ICIMT
                                        #   g7 = XLI
                                        #   g11 = IOCB

        PopRegsVoid(r3)                 # Restore all G registers
        ret
#
# --- SCSI status event code normalization table
#
#     Here is the current mapping:
#
#       (00) good            -> (04) cmd cmplt - ok
#       (02) check condition -> (08) cmd cmplt - chk stat
#       (04) condition met   -> (04) cmp cmplt - ok
#       (08) busy            -> (12) cmp cmplt - non chk stat
#       (10) intermediate    -> (04) cmd cmplt - ok
#       (14) interm-cond met -> (04) cmd cmplt - ok
#       (18) res conflict    -> (12) cmd cmplt - non chk stat
#       (22) cmd terminated  -> (12) cmd cmplt - non chk stat
#       (28) task set full   -> (16) cmd cmplt - target Q full
#       (30) ACA active      -> (12) cmd cmplt - non chk stat
#
        .data
scsi_st_norm:
        .byte    4, 0, 8, 0, 4, 0, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0 # 00-0f
        .byte    4, 0, 0, 0, 4, 0, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0 # 10-1f
        .byte    0, 0,12, 0, 0, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0 # 20-2f
        .byte   12, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0 # 30-3f
        .byte    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0 # 40-4f
        .byte    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0 # 50-5f
        .byte    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0 # 60-6f
        .byte    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0 # 70-7f
        .byte    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0 # 80-8f
        .byte    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0 # 90-9f
        .byte    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0 # a0-af
        .byte    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0 # b0-bf
        .byte    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0 # c0-cf
        .byte    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0 # d0-df
        .byte    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0 # e0-ef
        .byte    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0 # f0-ff
#
# --- Completion status event code normalization table
#
#     Here is the current mapping:
#
#       (00) complete        -> (04) cmd cmplt
#       (02) DMA error       -> (16) I/O error
#       (04) Reset           -> (16) I/O error
#       (05) Aborted         -> (24) abort from Qlogic
#       (06) Timeout         -> (16) I/O error
#       (07) Data overrun    -> (04) cmd cmplt
#       (15) Data underrun   -> (04) cmd cmplt
#       (1c) queue full      -> (12) non-CHeck status
#       (28) port unavail    -> (20) Miscellaneous error
#       (29) port logged out -> (20) Miscellaneous error
#       (2a) port conf chng  -> (20) Miscellaneous error
#
cmplt_st_norm:
        .byte    4, 0,16, 0,16,24,16,04,  0, 0, 0, 0, 0, 0, 0, 0 # 00-0f
        .byte    0,16, 0,16, 0, 4, 0, 0,  0, 0, 0, 0,12, 0, 0, 0 # 10-1f
        .byte    0, 0, 0, 0, 0, 0, 0, 0, 20,20,20, 0, 0, 0, 0, 0 # 20-2f
        .byte    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0 # 30-3f
        .byte    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0 # 40-4f
        .byte    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0 # 50-5f
        .byte    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0 # 60-6f
        .byte    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0 # 70-7f
        .byte    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0 # 80-8f
        .byte    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0 # 90-9f
        .byte    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0 # a0-af
        .byte    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0 # b0-bf
        .byte    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0 # c0-cf
        .byte    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0 # d0-df
        .byte    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0 # e0-ef
        .byte    0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0 # f0-ff
#
        .text
#
#******************************************************************************
#
#  NAME: i$discovery
#
#  PURPOSE:
#       Allocate resources for the discovery process and forks
#       the discovery task.
#
#  CALLING SEQUENCE:
#       call    i$discovery
#
#  INPUT:
#       g4 = ICIMT
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
i$discovery:
        movt    g0,r12                  # save g0-g2

        ldconst 512,g0                  # g0 = size for SGL
        call    i$alloc_dis_resource    # g1 = ILT for operation

        ldconst oimode_parent,r4        # r4 = parent task
        stob    r4,oil1_tmode(g1)       # save task as parent

        lda     ILTBIAS(g1),g1          # g1 = ILT at 2nd lvl

#        ldconst Cs_scanning,r4          # r4 = "scanning" state
#        stob    r4,ici_state(g4)        # save new state
        ldconst .Ts_start,r5            # r5 = start event table
        st      r5,oil2_ehand(g1)       # save it

        lda     i$start_discovery,g0    # establish discovery process
        mov     g1,g2                   # g2 = ILT at the 2nd level
        ldconst ISDISCPRI,g1            # g1 = Start Discovery Priority
c       CT_fork_tmp = (ulong)"i$start_discovery";
        call    K$tfork                 # create task

        movt    r12,g0                  # restore g0-g2
        ret
#
#******************************************************************************
#
#  NAME: i$start_discovery
#
#  PURPOSE:
#       Start discovery process
#
#  CALLING SEQUENCE:
#       k$tfork      i$start_discovery
#
#  INPUT:
#       g2 = ILT at 2nd level
#       g4 = ICIMT
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
i$start_discovery:
        movl    g0,r14                  # save g0-g1
        mov     g2,g1                   # g1 = ILT at 2nd level

        ldconst SCANDELAY,g0            # set up to wait before beginning scan
        call    K$twait                 # delay task

        ld      oil2_ehand(g1),r10      # r10 = ICIMT event handler address
        ldconst Te_timeout,r9           # r9 = event
        addo    r10,r9,r10              # add completion event type
        ld      (r10),r10
        callx   (r10)                   # process event
                                        #   g4 = ICIMT
        movl    r14,g0                  # restore g0-g1
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: i$ploop_start_fork
#
#  PURPOSE:
#       Start Discovery of Private Loop
#
#       Begins the process of scanning the FC-AL to determine what
#       devices exist via a forked process.
#
#  CALLING SEQUENCE:
#       K$tfork    i$ploop_start_fork
#
#  INPUT:
#       g2 = ILT at 2nd LVL
#       g4 = ICIMT
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g0-g3, and g5-g7 are destroyed
#
#******************************************************************************
#
i$ploop_start_fork:
        mov     g2,g1                   # Move ILT at level 2 to g1
#******************************************************************************
#
#  NAME: i$ploop_start_call
#
#  PURPOSE:
#       Start Discovery of Private Loop
#
#       Begins the process of scanning the FC-AL to determine what
#       devices exist via a regular call.
#
#  CALLING SEQUENCE:
#       call    i$ploop_start_call
#
#  INPUT:
#       g1 = ILT at 2nd LVL
#       g4 = ICIMT
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g0-g3, g5-g7, and g14 are destroyed
#
#******************************************************************************
#
i$ploop_start_call:
        mov     0,r9
#
# --- setup some default parameters for ploop discovery start
#
.if     ITRACES
        call    it$trc_ploop_start      # *** trace ***
.endif  # ITRACES
        mov     0,r7                    # r7 = ALPA map index start
                                        #   Byte zero is a count and is
                                        #   not currently used.

        lda     -ILTBIAS(g1),r15        # r15 = ilt at 1st level
#
# --- Request FC-AL position map from QLogic
#
        ldob    ici_chpid(g4),g0
        PushRegs(r10)
c       ISP_GetPositionMap(g0);
        PopRegsVoid(r10)
#
# --- Determine if there are any ALPA's in the loop map
#
        ld      ici_lpmapptr(g4),r10    # r10 = address of ALPA map in ICIMT

        ldob    (r10),r11               # r11 = map count
        ldob    ici_chpid(g4),g0        # g0 = chip instance
        cmpobne 0,r11,.plp_100          # Jif alpa's in map
#
# --- No ALPAs in map. If this persists, reset the QLogic chip.
#
        ldob    i$nodev_cnt[g0*1],r3    # r3 = no device counter
        addo    1,r3,r3                 # inc. no device counter
        stob    r3,i$nodev_cnt[g0*1]    # save new no device counter
        cmpobg  nodev_reset,r3,.plp_500 # Jif reset count not expired
#
# --- Reset QLogic port.
#
        mov     g1,r3                   # save g1
        ldconst ecri,g1                 # g1 = reset reason code
        call    ISP_ResetChip           # reset QLogic chip
        mov     r3,g1                   # restore g1
        ldconst 0,r3                    # reset no device counter
        stob    r3,i$nodev_cnt[g0*1]    # save new no device counter
        b       .plp_510
#
# --- get an ALPA from the map and determine if a TUR should be sent
#
.plp_100:
        ldconst 0,r3
        stob    r3,i$nodev_cnt[g0*1]    # reset no device counter
        ldconst 0xff,r3                 # r3 = invalid ALPA
        addo    1,r7,r7                 # bump ALPA map index
        addo    r10,r7,r11              # r11 = address of ALPA
        ldob    (r11),r8                # r8 = ALPA
        cmpobe.f r3,r8,.plp_500         # Jif end of map
                                        # *** This would mean that there are
                                        # *** no targets on the local loop.

        cmpobe.f 0,r8,.plp_100          # Jif SNS well known ALPA


        ldob    ici_chpid(g4),r12       # r12 = chip instance

        mov     g1,r4                   # Save g1
        mov     r12,g0                  # g0 = chip instance
        mov     r8,g1                   # g1 =  alpa
        call    ISP$is_my_ALPA          # check for my LID
        mov     r4,g1                   # restore g1
        cmpobne 0,g0,.plp_100           # Jif my LID

        PushRegs(r4)
        ldob    ici_chpid(g4),g0        # r12 = chip instance
        mov     r8,g1                   # store alpa
        ldconst 0,g2                    # set vpid to 0
        call    isp_alpa2handle
        mov     g0,r8                   # save return code
        PopRegsVoid(r4)                 # restore gregs

        lda     i$ploop_cr,r5           # r5 = address of completion routine

c if (r9 > 0xff) fprintf(stderr, ".plp_100: oil1_lun will not fit value 0x%lx\n", r9);
        stob    r9,oil1_lun(r15)        # set LUN 0
        stob    r7,oil1_lpmapidx(r15)   # save ALPA map index
        stos    r8,oil1_lid(r15)        # save LID
        stob    r12,oil1_chpid(r15)     # save chip instance
        st      r5,il_cr(r15)           # save completion return address

        stob    r9,oil1_flag(r15)       # clear flag byte
        st      r9,oil1_tmt(r15)        # clear TMT address
#
# --- Update the port datebase for this LID as required
#
        call    i$updateportdb          # update port database
#
# --- Determine if foreign targets are enabled. If they are not, check data
#     base world wide name to make sure the target is a XIO before starting
#     a discovery process.
#
        ldconst PORTDBSIZ,r13           # r13 = size of Port Database
        ld      portdb[r12*4],r14       # r14 = base of port database allocation
        mulo    r13,r8,r13              # r13 = offset = LID * PDB size
        addo    r13,r14,r14             # r14 =  Port DB address

        lda     pdbpdn(r14),g0          # g0 = ptr to port WWN name
        call    ISP$is_my_WWN           # check for my port WWN name
        cmpobe.f TRUE,g0,.plp_100       # Jif my port WWN name

        ldob    ici_ftenable(g4),r3     # r3 = foreign target enable flag
        cmpobne 0,r3,.plp_200           # Jif foreign target enabled

        ldl     pdbpdn(r14),g6          # g6-g7 = port name
c       g6 = M_chk4XIO(*(UINT64*)&g6);  # is this a XIOtech Controller ???
        cmpobe  0,g6,.plp_100           # no - try next lid
#
# --- Check if this is a XIO BE port
#
        ldl     pdbpdn(r14),g6          # g6 = MSW port name
        extract 12,4,g6                 # Get 'e' nibble from WWN
        ldconst (WWNBPort>>20)&0xF,r3   # Value for BE port
        cmpobe  g6,r3,.plp_100          # Jif a XIOtech BE port
#
# --- determine if this alpa/lid is an initiator
#
        ldob    pdbprliw3(r14),r3       # get prli ser parm w3
        bbc     5,r3,.plp_100           # JIf no initiator present
#
# --- determine if this alpa/lid is a target
#
.plp_200:
        ldob    pdbprliw3(r14),r3       # get prli ser parm w3
        bbc     4,r3,.plp_100           # JIf no target present
#
# --- deactivate controller timer during discovery
#
        ldconst TRUE,r4                 # set ctl t/o inhibit to TRUE
        stob    r4,ici_FCTOI(g4)        # set inhibit flag
#
# --- send TUR command to the target
#
        lda     I$recv_cmplt_rsp,g2     # g2 = completion routine
        ldconst .Ts_turl0,g3            # g3 = "lun 0" event table
        call    i$send_tur              # send TUR command

        b       .plp_1000
#
# --- No devices on the private loop, set state to offline and wait for a
#     change in the status of the loop.
#
.plp_500:
        ldob    ici_chpid(g4),g0        # g0 = chip instance
        ld      ispfflags,r5            # r5 = fabric support map
        bbc     g0,r5,.plp_510          # Jif not FL-port

        movt    g0,r12                  # save g0-g2
        ldconst .Ts_start_fabric,r5     # r5 = fabric start event table
        st      r5,oil2_ehand(g1)       # save it

        lda     i$fabric_start,g0       # establish fabric discovery process
        mov     g1,g2                   # g2 = ILT at nest level 2
        ldconst ISCANPRI,g1             # g1 = Fabric Scan priority
c       CT_fork_tmp = (ulong)"i$fabric_start";
        call    K$tfork                 # create task
        movt    r12,g0                  # restore g0-g2
        b       .plp_1000               # br

.plp_510:

#        ldconst Cs_offline,r3           # r3 = "offline" state
        call    i$rel_dis_resource      # release discovery resources
#        stob    r3,ici_state(g4)         # save new state
#
# --- Exit...
#
.plp_1000:
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: i$ploop_cr
#
#  PURPOSE:
#       Completion Routine For Discovery of Private Loop
#
#       Begins the process of scanning the FC-AL to determine what
#       devices exist.
#
#  CALLING SEQUENCE:
#       call    i$ploop_cr
#
#  INPUT:
#       g0 = completion status
#       g1 = ILT at 1st LVL
#       g4 = ICIMT
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g0-g3, g5-g7 destroyed.
#
#******************************************************************************
#
i$ploop_cr:
#
# --- reactivate controller timer
#
        ldconst FALSE,r4                # set ctl t/o inhibit to FALSE
        stob    r4,ici_FCTOI(g4)        # clr inhibit flag
#
# --- determine if the discovery process failed for private loop
#
        ldob    oil1_tmode(g1),r4      # r4 = task type
        cmpobe.f oimode_parent,r4,.plpcr_100 # Jif parent task
        cmpobe.f oimode_iscsi,r4,.plpcr_100 # Jif iscsi task
#
# -------------------------------------------------------------------
# --- C H I L D   T A S K   P R O C E S S I N G
#     process is a child task. Determine if a parent task is still
#     active. If not, activate resources within the child tasks
#     domain.
#
        ld      ici_disQ(g4),r14        # r14 = discovery queue
        cmpobe.t 0,r14,.plpcr_25        # JIf no discovery in process

.plpcr_20:
        ldob    oil1_tmode(r14),r4      # r4 = flags
        cmpobe.t oimode_parent,r4,.plpcr_90 # Jif parent discovery process

        ld      oil1_dslink(r14),r14    # r14 = next ILT
        cmpobne.t 0,r14,.plpcr_20       # JIf more processes on queue
#
# --- There is no parent task active. determine the domain of
#     this child task
#
.plpcr_25:
        ld      oil1_tmt(g1),g5         # g5 = tmt address
        cmpobe  0,g5,.plpcr_90          # Jif not defined

        ldob    oil1_flag(g1),r4        # r4 = domain
        bbs     oiflg_LID,r4,.plpcr_30  # JIf LID domain
#
# --- child task domain is a LUN
#
        ldob    oil1_lun(g1),r5         # r4 = lun number
        ld      tm_tlmtdir(g5)[r5*4],g6 # g6 = TLMT address from TLMT LUN dir
        cmpobe  0,g6,.plpcr_90          # Jif no TLMT defines for this lun

        ldob    tlm_state(g6),r4        # r4 = current TLMT state
        cmpobne.f tlmstate_active,r4,.plpcr_90 # Jif device not active

        call    apl$chknextask          # reissue tasks
        b       .plpcr_90               # br
#
# --- child task domain is a LID
#
.plpcr_30:
        ldconst 0,r5                    # r5 = lun index
        ldconst MAXLUN,r6               # r6 = max luns

.plpcr_32:
        ld      tm_tlmtdir(g5)[r5*4],g6 # g6 = TLMT address from TLMT LUN dir
        cmpobe  0,g6,.plpcr_35          # Jif no TLMT defines for this lun

        ldob    tlm_state(g6),r4        # r4 = current TLMT state
        cmpobne.f tlmstate_active,r4,.plpcr_35 # Jif device not active

        call    apl$chknextask          # reissue tasks

.plpcr_35:
        addo    1,r5,r5                 # increment index
        cmpobl  r5,r6,.plpcr_32         # Jif not last lun
#
# --- release child task resources
#
.plpcr_90:
        lda     ILTBIAS(g1),g1          # g1 = ILT at 2nd level
        call    i$rel_dis_resource      # release discovery resources
        b       .plpcr_1000             # exit
#
# -------------------------------------------------------------------
# --- P A R E N T   T A S K   P R O C E S S I N G
#     process is a parent task
#
.plpcr_100:
        mov     g1,r15                  # r15 = ILT lvl 1
        lda     ILTBIAS(g1),g1          # g1 = ILT at 2nd level

        cmpobne cmplt_success,g0,.plpcr_300 # jif completion error
#
# --- the discovery process did not fail for private loop
#
        ldob    ici_chpid(g4),r4        # r4 = chip instance
#
# -- skip fabric discovery task if this is iSCSI port
#
c       r5 = ICL_IsIclPort(r4);
        cmpobe  TRUE,r5,.plpcr_200      # Jif ICL support
        ld      iscsimap,r5             # r5 = iscsimap
        bbs     r4,r5,.plpcr_200        # Jif iSCSI port

        ld      ispfflags,r5            # r5 = fabric support map
        bbc     r4,r5,.plpcr_200        # Jif not FL-port
#
# --- the loop contains fabric, fork the fabric discovery task
#
        movt    g0,r12                  # save g0-g2
        ldconst .Ts_start_fabric,r5     # r5 = fabric start event table
        st      r5,oil2_ehand(g1)       # save it
        lda     i$fabric_start,g0       # establish fabric discovery process
        mov     g1,g2                   # g2 = ILT at nest level 2
        ldconst ISCANPRI,g1             # g1 = Fabric Scan priority
c       CT_fork_tmp = (ulong)"i$fabric_start";
        call    K$tfork                 # create task
        movt    r12,g0                  # restore g0-g2
        b       .plpcr_1000             # exit
#
# --- no fabric connection or private loop discovery process failed,
#     complete discovery process
#
.plpcr_200:

        ldconst cmplt_success,g0        # load good completion status

.plpcr_300:
        ldob    oil1_flag(r15),g2       # get the discovery flags

        call    i$rel_dis_resource      # release discovery resources
        call    i$iscan_complete        # complete discovery process

.plpcr_1000:
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: i$restart_scan
#
#  PURPOSE:
#       Provide a means to restart the current discovery process without
#       reallocation of resources.
#
#  DESCRIPTION:
#       If the task is a parent, jump to i$ploop_start_fork.
#       Otherwise, release the discovery resources
#
#  CALLING SEQUENCE:
#       call    i$restart_scan
#
#  INPUT:
#       g1 = ILT at 2nd level
#       g4 = ICIMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#      None:
#
#******************************************************************************
#
i$restart_scan:
        lda     -ILTBIAS(g1),r15        # r15 = ILT at 1st level
        ldob    oil1_tmode(r15),r4      # r4 = flags
        cmpobne.f oimode_parent,r4,.rss_100 # Jif not parent discovery task
#
# --- parent discovery process, restart discovery loop
#
        movt    g0,r12                  # save g0-g2
        ldconst .Ts_start,r5            # r5 = start event table
        st      r5,oil2_ehand(g1)       # save it

        lda     i$ploop_start_fork,g0   # establish discovery process
        mov     g1,g2                   # g2 = ILT at 2nd level
        ldconst ISCANPRI,g1             # g1 = Ploop Scan Task Priority
c       CT_fork_tmp = (ulong)"i$ploop_start_fork";
        call    K$tfork                 # create task
        movt    r12,g0                  # restore g0-g2
        b       .rss_1000               # br
#
# --- this is a child discovery task. Just release the resources.
#
.rss_100:
        call    i$rel_dis_resource      # release discovery resources

.rss_1000:
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: i$wait2restart
#
#  PURPOSE:
#       i$wait2restart
#
#       This entry is used to change the ICIMT event handler table
#       to Ts_restart. It also casts a vote indicating that another
#       discovery process is not required
#
#  CALLING SEQUENCE:
#       call    i$wait2restart
#
#  INPUT:
#       g1 = ILT at 2nd lvl
#       g4 = ICIMT address
#
#  OUTPUT:
#       g0 = non zero value
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
i$wait2restart:
        lda     -ILTBIAS(g1),r15        # r15 = ILT at 1st level
        ldob    oil1_tmode(r15),r4      # r4 = mode byte
        cmpobne oimode_iscsi,r4,.w2r_250  # Jif not iscsi discovery task

        call    i$scan_flush            # end the scan process
        b       .w2r_1000               # br

.w2r_250:
        ldob    oil1_flag(r15),r4       # r4 = flags byte
        setbit  oiflg_PDBC,r4,r4        # set port data base changed indication
        stob    r4,oil1_flag(r15)       # save save new value

        ldconst .Ts_restart,r4          # r4 = restart event table
        st      r4,oil2_ehand(g1)       # save it

.w2r_1000:
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: i$go_offline
#
#  PURPOSE:
#       Change the ICIMT event handler table to Ts_offline.
#
#  CALLING SEQUENCE:
#       call    i$go_offline
#
#  INPUT:
#       g1 = ILT at 2nd level
#       g4 = ICIMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
i$go_offline:
        lda     -ILTBIAS(g1),r15        # r15 = ILT at 1st level
        ldob    oil1_tmode(r15),r4      # r4 = mode byte
        cmpobne.f oimode_parent,r4,.igo_10 # Jif not parent discovery task

        ldob    oil1_flag(r15),r4       # r4 = flags byte
        setbit  oiflg_PDBC,r4,r4        # set port data base changed indication
        setbit  oiflg_ABRT,r4,r4        # set abort process bit
        stob    r4,oil1_flag(r15)       # save save new value

.igo_10:
        ldconst .Ts_going_offline,r4    # r4 = restart event table
        st      r4,oil2_ehand(g1)       # save it
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: i$login
#
#  PURPOSE:
#       Receive good responses from a iscsi login
#
#  DESCRIPTION:
#       A good response was received from the device, This means that the
#       iscsi session is established between the initiator and target. Now
#       send the turl0
#
#       The event handler table is set to ".Ts_turl0"
#
#  CALLING SEQUENCE:
#       call    i$login
#
#  INPUT:
#       g0 = SCSI or Qlogic completion status
#       g1 = ILT at Initiator level 2.
#       g4 = ICIMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g2 and g3 are destroyed.
#
#******************************************************************************
#
i$login:
#
# --- send TUR command to the target
#
        lda     I$recv_cmplt_rsp,g2     # g2 = completion routine
        ldconst .Ts_turl0,g3            # g3 = "lun 0" event table
        call    i$send_tur              # send TUR command
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: i$turl0
#
#  PURPOSE:
#       Receive good responses from an TUR command for LUN 0.
#
#  DESCRIPTION:
#       A good response was received from the device, This means that a SSU
#       command completed normally and an inquiry command should be sent to
#       the device.
#
#       The event handler table is set to ".Ts_inql0"
#
#  CALLING SEQUENCE:
#       call    i$turl0
#
#  INPUT:
#       g0 = SCSI or Qlogic completion status
#       g1 = ILT at Initiator level 2.
#       g4 = ICIMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g2 and g3 are destroyed.
#
#******************************************************************************
#
i$turl0:
#c fprintf(stderr,"idriver: i$turl0 ICIMT %08lX\n",g4);
        lda     -ILTBIAS(g1),r15        # r15 = 1st level of ILT
        ldob    oil1_flag(r15),r4       # get ILT flag byte
        bbc     oiflg_PDBC,r4,.turl0_100 # Jif data base has not changed
        call    i$next_lun              # process next lun
        b       .turl0_200

.turl0_100:
        lda     I$recv_cmplt_rsp,g2     # g2 = completion routine
        lda     .Ts_inql0,g3            # g3 = "inquire lun 0" event table
        call    i$send_inq              # send the inquiry command

.turl0_200:
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: i$turl0_cs
#
#  PURPOSE:
#       Receive check status responses from an TUR command for LUN 0.
#
#  DESCRIPTION:
#       A check status response was received from the device, This means that
#       that there is some error that requires other processing.
#
#
#       key|ASC|ASCQ|   description              processing
#       --------------------------------------------------------------------
#       05h 25h 00h     LU not supported        Scan INQ cmd
#       02h 05h 00h     LU not responding       Scan next LID
#       02h 3Ah 00h     Medium not present      Send SSU cmd
#       02h 04h 00h     LU n/ready n/reportable Send SSU cmd
#       02h 04h 01h     LU becoming ready       Reschedule scan of LID
#       02h 04h 02h     LU needs init cmd       Send SSU cmd
#       02h 04h 03h     Intervention required   Send SSU cmd
#       02h 04h 04h     Format in process       mark format/ next LUN
#       --h 28h 00h     Not RDY to RDY change   Send INQ cmd
#       06h 29h 00h     Power-on reset          Resend TUR cmd
#       06h 29h 02h     SCSI bus reset          Resend TUR cmd
#       06h 29h 03h     Device reset            Resend TUR cmd
#
#  CALLING SEQUENCE:
#       call    i$turl0
#
#  INPUT:
#       g0  = SCSI or Qlogic completion status
#       g1  = ILT at Initiator level 2.
#       g4  = ICIMT address
#       g11 = status type 0 IOCB (if g0 <> 0)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g2 and g3 are destroyed.
#
#******************************************************************************
#
i$turl0_cs:
#        c fprintf(stderr,"idriver: i$turl0_cs ICIMT ?%08lX stat %02lX\n",g4,g0);
        lda     -ILTBIAS(g1),r15        # r15 = 1st level of ILT

        ldob    oil1_flag(r15),r4       # get ILT flag byte
        bbs     oiflg_PDBC,r4,.turl0_cs_900 # Jif data base has changed
        ldob    oil1_chpid(r15),r8
c       r9 = ICL_IsIclPort((UINT8)r8);
        cmpobe  TRUE,r9,.turl0_icl01    # Jif ICL port
        ld      iscsimap,r9             # r9 = iscsi port map
        bbc     r8,r9,.turl0_050        # Jif iSCSI port
.turl0_icl01:
        ld     oil1_snsdata(r15),r7
        b       .turl0_055
#
.turl0_050:
        lda     0x24(g11),r9            # r9 = base of sense/response data
        ld      0x20(g11),r8            # r8 = response data len 0,4, or 8
        addo    r8,r9,r7                # r9 = add response len to pointer
.turl0_055:
        ldob    12(r7),r8               # r8 = ASC  data
        ldob    13(r7),r9               # r9 = ASCQ data
#         c fprintf(stderr,"idriver: i$turl0_cs ASC %02lX ASCQ %02lX\n",r8,r9);
# ***********************************************************
#       ASC 28 - check for not ready to ready condition
# ***********************************************************
        ldconst 0x28,r3                 # load reset ASC
        cmpobe.f r3,r8,.turl0_cs_920    # Jif not ready to ready change
                                        # send inquiry

# ***********************************************************
#       ASC 29 - check for some form of reset
# ***********************************************************
        ldconst 0x29,r3                 # load reset ASC
        cmpobne.t r3,r8,.turl0_cs_100   # Jif not some form of reset
        lda     I$recv_cmplt_rsp,g2     # g2 = completion routine
        lda     .Ts_turl0,g3            # g3 = "test unit ready LUN 0" event table
        call    i$send_tur              # send the TUR command
        b       .turl0_cs_1000          # b

# ***********************************************************
#       ASC 25 - check if LU not supported
# ***********************************************************
.turl0_cs_100:
        ldconst 0x25,r3                 # LU not supported ???
        cmpobne.f r3,r8,.turl0_cs_200   # Jif LU supported
#
# --- determine if this LUN was previously discovered
#
        ldos    oil1_lid(r15),r7        # r7 = lid
        ld      ici_tmdir(g4)[r7*4],g5  # g5 = TMT address from cimt lid dir
        cmpobe.f 0,g5,.turl0_cs_920     # Jif no tmt, send inquiry

#        st      g5,oil1_tmt(r15)        # save tmt address in ILT

        ldob    oil1_lun(r15),r6        # r6 = current lun
        ld      tm_tlmtdir(g5)[r6*4],g6 # g6 = possible TLMT address
        cmpobe.f 0,g6,.turl0_cs_920     # Jif no tlmt, send inquiry
#
# --- This lun was here and now it's gone. Determine if it's resources
#     can be deallocated.
#
        ldob    tlm_sescnt(g6),r4       # r4 = session count
        scanbit r4,r5                   # r5 = MSB set
        bno.f   .turl0_cs_920           # Jif zero (big time problem)
#
# --- the session count is non zero. Determine if the discovery process is
#     part of that session count. If it is, decrement the session count by
#     0x80. If session count is now zero, deallocate tlmt and it's resources.
#
        cmpobne.f 7,r5,.turl0_cs_920    # Jif discovery not part of count
        clrbit  r5,r4,r4                # remove discover count
        stob    r4,tlm_sescnt(g6)       # save new session count
        cmpobne.t 0,r4,.turl0_cs_920    # Jif session count not zero
#
# --- session count is zero, remove tlmt and it's resources
#
        call    i$removeLUN             # remove lun and resources
        b       .turl0_cs_920           # send inquiry

# ***********************************************************
#       ASC 4 - check for LU not ready conditions
# ***********************************************************
.turl0_cs_200:
        ldconst 0x04,r3
        cmpobne.f r3,r8,.turl0_cs_300   # Jif if not equal
#
#       ASC 4 / ASCQ 1 - check if device becoming ready
#
        ldconst 0x01,r3                 # r3 = "device becoming ready" status
        ldob    oil1_tmode(r15),r7      # r7 = get task mode
        cmpobne.f r3,r9,.turl0_cs_220   # Jif device not becoming ready
#
# --- device is becoming ready, determine if this task is a rediscovery
#     of this device.
#
        ldob    oil1_retry(r15),r4      # r4 = retry counter
        cmpobe.t oimode_parent,r7,.turl0_cs_210 # JIf parent discover task
#
# --- This is a rediscovery of the device. Determine if the retry counter
#     has expired. If it has, exit to next LID (which should end this task),
#     otherwise setup to reissue TUR command.
#
        cmpobe.t 0,r4,.turl0_cs_910     # Jif retry counter expired - give up
        subo    1,r4,r4                 # decrement retry count
        stob    r4,oil1_retry(r15)      # save new value

        call    i$rescanl0_call         # rescan device in this process
        b       .turl0_cs_1000          # b
#
# --- This is not a rediscovery task. The current task is sent on to
#     discover the next LID. A new task is built to continue the discovery
#     of the current LID.
#
.turl0_cs_210:
        ldob    oil1_flag(r15),r7       # r7 = flag byte
        ld      oil1_chpid(r15),r6      # r6 = current chipid,mode,LUN,ALPA index
        ld      oil1_pid(r15),r5        # r5 = PID (ALPA)
        ld      oil1_tmt(r15),r8        # r8 = possible tmt address
        ldos    oil1_lid(r15),r11       # r11 = lid
        ld      il_cr(r15),r4           # r4 = address of completion routine
.if     MAG2MAG
        setbit  oiflg_STID,r7,r3        # set suppress target ID to LLD
        stob    r3,oil1_flag(r15)       # save flag byte
.endif  # MAG2MAG
        call    i$next_lid              # process next lid

        ldconst 512,g0              # g0 = set size for SGL
        call    i$alloc_dis_resource    # g1 = ILT for operation (at 1st level)

        setbit  oiflg_LID,r7,r7         # set LID only in flags

        st      r5,oil1_pid(g1)         # save PID (ALPA)
        st      r6,oil1_chpid(g1)       # save  chipid,mode,LUN,ALPA index
        stob    r7,oil1_flag(g1)        # save flag byte
        st      r8,oil1_tmt(g1)         # save tmt address
        st      r4,il_cr(g1)            # save completion return address
        stos    r11,oil1_lid(g1)        # save lid
        #this updates the mode info brought from the previous ilt
        ldconst oimode_child,r4         # r4 = child task
        stob    r4,oil1_tmode(g1)       # save task as child

        cmpobe.t 0,r8,.turl0_cs_215     # JIf no tmt address
        ldob    oil1_lun(g1),r5         # r5 = lun

.turl0_cs_215:
        lda     ILTBIAS(g1),g1          # bump to lvl 2 of ILT nest
        call    i$rescanl0_call         # rescan device in this process
        b       .turl0_cs_1000          # b
#
#       (asc 4 / ascq 4) - check if a format is in progress
#
.turl0_cs_220:
        ldconst 0x04,r3                 # is a format in process ???
        cmpobne.f r3,r9,.turl0_cs_990   # Jif no (send SSU)
        b       .turl0_cs_920           # br to send inquiry

# ***********************************************************
#       (ASC 5  / ASCQ 0) - check if LU not responding
# ***********************************************************
.turl0_cs_300:
        ldconst 0x05,r3                 # LU not responding ???
        cmpobe.f r3,r8,.turl0_cs_910    # Jif LU not responding

#
#       check for a no sense condition (an exabyte thing)
#
        ldob    oil1_chpid(r15),r8
c       r9 = ICL_IsIclPort((UINT8)r8);
        cmpobe  TRUE,r9,.turl0_icl02
        ld      iscsimap,r9            # r9 = iscsi port map
        bbc     r8,r9,.turl0_cs_401    # Jif not iSCSI port
.turl0_icl02:
        ldconst 0x20,r3                # Storing sense size for iSCSI
        b       .turl0_cs_450

.turl0_cs_401:
        ldos    0x16(g11),r9
        bbc     9,r9,.turl0_cs_415
        ld      0x1C(g11),r3            # get sense data length
        b       .turl0_cs_450
#
.turl0_cs_415:
        ldconst 0,r3
.turl0_cs_450:
        cmpobne.f 0,r3,.turl0_cs_990    # Jif sense length - start unit
#
# --- scan next LUN
#
.turl0_cs_900:
        call    i$next_lun              # process next lun
        b       .turl0_cs_1000          # b
#
# --- scan next LID
#
.turl0_cs_910:
.if     MAG2MAG
        ldob    oil1_flag(r15),r3       # r3 = flag byte
        setbit  oiflg_STID,r3,r3        # set suppress target ID to LLD
        stob    r3,oil1_flag(r15)       # save flag byte
.endif  # MAG2MAG
        call    i$next_lid              # process next lid
        b       .turl0_cs_1000          # b
#
# --- send inquiry
#
.turl0_cs_920:
        lda     I$recv_cmplt_rsp,g2     # g2 = completion routine
        lda     .Ts_inql0,g3            # g3 = "start/stop unit LUN 0" event table
        call    i$send_inq              # send the inquiry command
        b       .turl0_cs_1000          # b
#
#      Send the SSU command
#
.turl0_cs_990:
        ldob    oil1_flag(r15),r7       # r7 = flag byte
        ld      oil1_chpid(r15),r6      # r6 = current chipid,mode,LUN,ALPA index
        ld      oil1_pid(r15),r5        # r5 = PID (ALPA)
        ld      oil1_tmt(r15),r8        # r8 = possible tmt address
        ldos    oil1_lid(r15),r11       # r11 = lid
        ld      il_cr(r15),r4           # r4 = address of completion routine
.if     MAG2MAG
        setbit  oiflg_STID,r7,r3        # set suppress target ID to LLD
        stob    r3,oil1_flag(r15)       # save flag byte
.endif  # MAG2MAG
        call    i$next_lid              # process next lid

        ldconst 512,g0              # g0 = set size for SGL
        call    i$alloc_dis_resource    # g1 = ILT for operation (at 1st level)

        setbit  oiflg_LID,r7,r7         # set lid only in flags

        st      r5,oil1_pid(g1)         # save PID (ALPA)
        st      r6,oil1_chpid(g1)       # save  chipid,mode,LUN,ALPA index
        stob    r7,oil1_flag(g1)        # save flag byte
        st      r8,oil1_tmt(g1)         # save tmt address
        st      r4,il_cr(g1)            # save completion return address
        stos    r11,oil1_lid(g1)        # save lid
        #this updates the mode info brought from the previous ilt
        ldconst oimode_child,r4         # r4 = child task
        stob    r4,oil1_tmode(g1)       # save task as child

        cmpobe.t 0,r8,.turl0_cs_995     # JIf no tmt address
        ldob    oil1_lun(g1),r5         # r5 = lun

.turl0_cs_995:
        lda     ILTBIAS(g1),g1          # bump to lvl 2 of ILT nest
        lda     I$recv_cmplt_rsp,g2     # g2 = completion routine
        lda     .Ts_ssul0,g3            # g3 = "start/stop unit LUN 0" event table
        call    i$send_ssu              # send the SSU command

.turl0_cs_1000:
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: i$turl0_noncs
#
#  PURPOSE:
#       Received a non CS error in response to to TUR command sent to
#       LUN 0.
#
#  DESCRIPTION:
#       A non CS is received from a TUR command sent to LUN 0.
#       Determine the type of error and handle it.
#
#       error   description         process
#       -----   -----------         -----------------------------------
#       08h     busy                Fork task, retry this LID
#       1ch     queue full          Fork task, retry this LID
#       18h     reserve conflict    Try next LID
#       22h     command terminated  Try next LID
#       28h     Task Set Full       Fork task, Retry this LID
#       30h     ACA error           Try next LID
#
#  CALLING SEQUENCE:
#       call    i$turl0
#
#  INPUT:
#       g0 = SCSI or Qlogic completion status
#       g1 = ILT at Initiator level 2.
#       g4 = ICIMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g2 and g3 are destroyed.
#
#******************************************************************************
#
i$turl0_noncs:
        lda     -ILTBIAS(g1),r15        # r15 = ILT at 1st level

        ldob    oil1_flag(r15),r4       # get ILT flag byte
        bbs     oiflg_PDBC,r4,.turl0_noncs_910 # Jif data base changed

#
# --- determine error type
#
        ldconst 0x08,r3                 # busy
        ldconst 0x1c,r4                 # queue full
        ldconst 0x28,r5                 # task set full

        cmpobe  r3,g0,.turl0_noncs_100  # Jif busy
        cmpobe  r4,g0,.turl0_noncs_100  # Jif queue full
        cmpobne r5,g0,.turl0_noncs_900  # Jif not task set full
#
# --- device is not currently ready, determine if this task is a
#     rediscovery of this device.
#
.turl0_noncs_100:
        ldob    oil1_tmode(r15),r7      # r7 = get task mode
        ldob    oil1_retry(r15),r4      # r4 = retry counter
        cmpobe.t oimode_parent,r7,.turl0_noncs_210 # JIf parent discover task
#
# --- This is a rediscovery of the device. Determine if the retry counter
#     has expired. If it has, exit to next LID (which should end this task),
#     otherwise setup to reissue TUR command.
#
        cmpobe.t 0,r4,.turl0_noncs_900  # Jif retry counter expired - give up
        subo    1,r4,r4                 # decrement retry count
        stob    r4,oil1_retry(r15)      # save new value

        call    i$rescanl0_call         # rescan device in this process
        b       .turl0_noncs_1000       # b
#
# --- This is not a rediscovery task. The current task is sent on to
#     discover the next LID. A new task is built to continue the discovery
#     of the current LID.
#
.turl0_noncs_210:
        ldob    oil1_flag(r15),r7       # r7 = flag byte
        ld      oil1_chpid(r15),r6      # r6 = current chipid,mode,LUN,ALPA index
        ld      oil1_pid(r15),r5        # r5 = PID (ALPA)
        ld      oil1_tmt(r15),r8        # r8 = possible tmt address
        ldos    oil1_lid(r15),r11       # r11 = lid
        ld      il_cr(r15),r4           # r4 = address of completion routine
.if     MAG2MAG
        setbit  oiflg_STID,r7,r3        # set suppress target ID to LLD
        stob    r3,oil1_flag(r15)       # save flag byte
.endif  # MAG2MAG
        call    i$next_lid              # process next lid

        ldconst 512,g0              # g0 = set size for SGL
        call    i$alloc_dis_resource    # g1 = ILT for operation (at 1st level)

        setbit  oiflg_LID,r7,r7         # set LID only in flags

        st      r5,oil1_pid(g1)         # save PID (ALPA)
        st      r6,oil1_chpid(g1)       # save  chipid,mode,LUN,ALPA index
        stob    r7,oil1_flag(g1)        # save flag byte
        st      r8,oil1_tmt(g1)         # save tmt address
        st      r4,il_cr(g1)            # save completion return address
        stos    r11,oil1_lid(g1)        # save lid
        #this updates the mode info brought from the previous ilt
        ldconst oimode_child,r4         # r4 = child task
        stob    r4,oil1_tmode(g1)       # save task as child

        cmpobe.t 0,r8,.turl0_noncs_215  # JIf no tmt address
        ldob    oil1_lun(g1),r5         # r5 = lun

.turl0_noncs_215:
        lda     ILTBIAS(g1),g1          # bump to lvl 2 of ILT nest
        call    i$rescanl0_call         # rescan device in this process
        b       .turl0_noncs_1000       # b

.turl0_noncs_900:
.if     MAG2MAG
        ldob    oil1_flag(r15),r3       # r3 = flag byte
        setbit  oiflg_STID,r3,r3        # set suppress target ID to LLD
        stob    r3,oil1_flag(r15)       # save flag byte
.endif  # MAG2MAG

.turl0_noncs_910:
        call    i$next_lid              # process next LID

.turl0_noncs_1000:
        ret                             #return to caller
#
#******************************************************************************
#
#  NAME: i$turl0_ioe
#
#  PURPOSE:
#       Received an IO error in response to to TUR command sent to
#       LUN 0.
#
#  DESCRIPTION:
#       An IO error is received from a TUR command sent to LUN 0.
#       Determine the type of error and handle it.
#
#       error   description     process
#       -----   -----------     -----------------------------------
#       02h     DMA error       End discovery process
#       04h     LIP reset       Retry the task
#       05h     Command aborted Retry the task
#       06h     timeout         Fork task. New task rediscover current
#                               LID. Old task discover next LID.
#
#  CALLING SEQUENCE:
#       call    i$turl0
#
#  INPUT:
#       g0 = SCSI or Qlogic completion status
#       g1 = ILT at Initiator level 2.
#       g4 = ICIMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g2 and g3 are destroyed.
#
#******************************************************************************
#
i$turl0_ioe:
        lda     -ILTBIAS(g1),r15        # r15 = ILT at 1st level

        ldob    oil1_flag(r15),r4       # get ILT flag byte
        bbc     oiflg_PDBC,r4,.tl0ioe_050 # Jif data base has not changed

        call    i$next_lun              # process next lun
        b       .tl0ioe_1000
#
# --- Determine if the error is a timeout or DMA error.
#
.tl0ioe_050:
        cmpobe.t  0x02,g0,.tl0ioe_900   # Jif DMA error

        cmpobe.t  0x06,g0,.tl0ioe_100   # Jif timeout
#
# --- iSCSI sg specific Error Handling
#
        cmpobe.t  0x81,g0,.tl0ioe_900 # Jif No Connection
        cmpobe.t  0x83,g0,.tl0ioe_100 # Jif Timeout
        cmpobe.t  0x84,g0,.tl0ioe_900 # Jif Bad Target
        cmpobe.t  0x85,g0,.tl0ioe_900 # Jif Abort
        cmpobe.t  0x88,g0,.tl0ioe_900 # Jif Reset
#
# --- the error is either command aborted by LIP or command aborted. In
#     either case, retry the task.
#
        lda     I$recv_cmplt_rsp,g2     # g2 = completion routine
        lda     .Ts_turl0,g3            # g3 = "test unit ready LUN 0" event table
        call    i$send_tur              # send the TUR command
        b       .tl0ioe_1000            # b
#
# --- command has timed out, determine if this task is a rediscovery
#     of this device.
#
.tl0ioe_100:
        ldob    oil1_tmode(r15),r7      # r7 = get task mode
        ldob    oil1_retry(r15),r4      # r4 = retry counter
        cmpobe.t oimode_parent,r7,.tl0ioe_200 # JIf parent discover task
#
# --- This is a rediscovery of the device. Determine if the retry counter
#     has expired. If it has, exit to next LUN (which should end this task),
#     otherwise setup to reissue TUR command.
#
        cmpobe.t 0,r4,.tl0ioe_500       # Jif retry counter expired
        subo    1,r4,r4                 # decrement retry count
        stob    r4,oil1_retry(r15)      # save new value
#       c       fprintf(stderr,"tl0ioe_100 rescan fork \n");
        call    i$rescanl0_fork         # establish a separate process to rescan
        b       .tl0ioe_1000            # b
#
# --- This is not a rediscovery task. The current task is sent on to
#     discover the next LUN. A new task is built to continue the discovery
#     of the current LUN.
#
.tl0ioe_200:
        ldob    oil1_flag(r15),r7       # r7 = flag byte
        ld      oil1_chpid(r15),r6      # r6 = current chipid,mode,LUN,ALPA index
        ld      oil1_pid(r15),r5        # r5 = PID (ALPA)
        ld      oil1_tmt(r15),r8        # r8 = possible TMT address
        ldos    oil1_lid(r15),r9        # r9 = lid
        ld      il_cr(r15),r4           # r4 = address of completion routine
.if     MAG2MAG
        setbit  oiflg_STID,r7,r3        # set suppress target ID to LLD
        stob    r3,oil1_flag(r15)       # save flag byte
.endif  # MAG2MAG
        call    i$next_lid              # process next lid

        ldconst 512,g0              # g0 = set size for SGL
        call    i$alloc_dis_resource    # g1 = ILT for operation

        setbit  oiflg_LID,r7,r7         # set LID only in flags

        st      r5,oil1_pid(g1)         # save PID (ALPA)
        st      r6,oil1_chpid(g1)       # save  chipid,mode,LUN,ALPA index
        stob    r7,oil1_flag(g1)        # save flag byte
        st      r8,oil1_tmt(g1)         # save tmt address
        st      r4,il_cr(g1)            # save completion return address
        stos    r9,oil1_lid(g1)         # save lid
        ldconst oimode_child,r4         # r4 = child task
        #this updates the mode info brought from the previous ilt
        stob    r4,oil1_tmode(g1)       # save task as child,


        lda     ILTBIAS(g1),g1          # bump to lvl 2 of ILT nest
        call    i$rescanl0_fork         # establish a separate process to rescan
        b       .tl0ioe_1000            # b
#
# --- Retries have expired, try next LID...
#
.tl0ioe_500:
.if     MAG2MAG
        ldob    oil1_flag(r15),r3       # r3 = flag byte
        setbit  oiflg_STID,r3,r3        # set suppress target ID to LLD
        stob    r3,oil1_flag(r15)       # save flag byte
.endif  # MAG2MAG
        call    i$next_lid              # process next LID
        b       .tl0ioe_1000            # b
#
# --- DMA error, kill the discovery process
#
.tl0ioe_900:
        call    i$scan_flush            # end the scan process

.tl0ioe_1000:
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: i$ssul0
#
#  PURPOSE:
#       Receive good responses from an SSU command for LUN 0.
#
#  DESCRIPTION:
#       A good response was received from the device, This means that the
#       SSU command is not required. Send an Inquiry command to the device.
#
#       The event handler table is set to ".Ts_inql0"
#
#  CALLING SEQUENCE:
#       call    i$ssul0
#
#  INPUT:
#       g0 = SCSI or Qlogic completion status
#       g1 = ILT at Initiator level 2.
#       g4 = ICIMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g2 and g3 are destroyed.
#
#******************************************************************************
#
i$ssul0:
        lda     -ILTBIAS(g1),r15        # r15 = 1st level of ILT
        ldob    oil1_flag(r15),r4       # get ILT flag byte
        bbc     oiflg_PDBC,r4,.ssul0_100 # Jif data base has not changed
        call    i$next_lun              # process next lun
        b       .ssul0_200

.ssul0_100:
        lda     I$recv_cmplt_rsp,g2     # g2 = completion routine
        lda     .Ts_turl0,g3            # g3 = "tur lun 0" event table
        call    i$send_tur              # send the tur command

.ssul0_200:
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: i$inql0
#
#  PURPOSE:
#       Receive responses from an inquiry command, determine if LUN 0 exists,
#       link control packets, and start process to determine if other LUNs
#       exist at this AL-PA.
#
#  CALLING SEQUENCE:
#       call    i$inql0
#
#  INPUT:
#       g1 = ILT at Initiator level 2.
#       g4 = ICIMT address
#       g7 = XLI
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g5, g6, and g7 are destroyed.
#
#******************************************************************************
#
i$inql0:
        mov     0,r14
        lda     -ILTBIAS(g1),r15        # r15 = 1st level of ILT

        ldob    oil1_flag(r15),r4       # get ILT flag byte
        bbs     oiflg_PDBC,r4,.inql0_210 # Jif data base changed

        ld      xlisglpt(g7),r9         # r9 = address of sg tbl
        ld      sg_addr(r9),r10         # r10 = inquiry response page
#
        ldos    oil1_lid(r15),r7        # r7 = current lid
        ldob    oil1_lun(r15),r6        # r6 = current lun
        ldob    oil1_chpid(r15),r11
#
# --- If there is a tmt defined in the ILT, don't bother hunting for
#     a match
#
        ld      oil1_tmt(r15),g5        # g5 = possible TMT
.ifdef M4_ADDITION
c       if (g5 != 0 && ((TMT*)g5)->state == tmstate_deltar) {
c           fprintf(stderr, "%s%s:%u i$inql0 TMT 0x%08lx in process of being deleted -- what do we do??\n", FEBEMESSAGE,__FILE__, __LINE__, g5);
c           abort();
c       }
.endif  # M4_ADDITION
        cmpobne.f 0,g5,.inql0_060       # Jif TMT defined
#
# --- Get port name from Data Base
#
        ldob    ici_chpid(g4),r11       # r11 = chip instance
        ld      ici_tmtQ(g4),g5         # g5 = TMT from queue
#
# --- Determine if there is a TMT with this port name.
#
#       If there is TMT with this port name, determine if
#       it is on the same LID.
#
#         If not on the same LID, clear the directory entry
#         in the ici_tmdir for the previous LID and save the
#         TMT address in the directory entry for the new LID.
#
#         If on the same LID, activate all TLMT's, clear TMR1, and
#         process next LID
#
#       If there is not TMT with this port name, allocate
#       a TMT and save appro information.
#
.inql0_010:
#
        PushRegs(r3)
        mov     r11,g0                  # g0 = chip id
        mov     r7,g1                   # g1 = lid/indx
        call    fsl_getPortWWN
        movl    g0,r4
        PopRegsVoid(r3)
#
        cmpobne.t 0,g5,.inql0_020       # Jif not end of queue
#
# --- there is no TMT that matches this target, allocate one
#     and set it up.
#
c       g5 = get_tmt();                 # Allocate a TMT
.ifdef M4_DEBUG_TMT
c fprintf(stderr, "%s%s:%u get_tmt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g5);
.endif # M4_DEBUG_TMT

        stob    r11,tm_chipID(g5)       # save chip instance
        st      g4,tm_icimt(g5)         # save ICIMT address in TMT
        stos    r7,tm_lid(g5)           # save LID
        stl     r4,tm_P_name(g5)        # save port name
#
        PushRegs(r3)
        mov     r11,g0                  # g0 = chip id
        mov     r7,g1                   # g1 = lid/indx
        call    fsl_getNodeWWN
        movl    g0,r4
        PopRegsVoid(r3)
#
        ld      ici_tmtQ(g4),r3         # Link TMT on to ici_tmtQ
        st      g5,ici_tmtQ(g4)
        st      r3,tm_link(g5)
        stl     r4,tm_N_name(g5)        # save node name

        ldob    tm_flag(g5),r3          # r3 = flag byte
        setbit  tmflg_lundisc,r3,r3     # set LUN discovery request flag
        stob    r3,tm_flag(g5)          # save
c       r3 = isp_handle2alpa(r11,r7)
        bswap   r3,r3                   # this puts it in the same format
                                        # as the gan data
#        st      g5,ici_tmdir(g4)[r7*4]  # save TMT address in cimt lid dir
        st      r3,tm_alpa(g5)          # save alpa
        b       .inql0_050              # br
#
# --- check this TMT to see if it matches the current target
#
.inql0_020:
        ldl     tm_P_name(g5),r8        # get port name of TMT
        cmpobne.f r8,r4,.inql0_025      # Jif not a match
#
#        cmpobe.f r9,r5,.inql0_030       # Jif A MATCH !!!
        cmpobne.f r9,r5,.inql0_025      # Jif not a match
#
        PushRegs(r3)
        mov     r11,g0                  # g0 = chip id
        mov     r7,g1                   # g1 = lid/indx
        call    fsl_getNodeWWN
        movl    g0,r4
        PopRegsVoid(r3)
#
        ldl     tm_N_name(g5),r8        # get node name of TMT
        cmpobne.f r8,r4,.inql0_025      # Jif not a match
        cmpobe.f r9,r5,.inql0_030       # Jif A MATCH !!!
.inql0_025:
        ld      tm_link(g5),g5          # get next TMT on queue
        b       .inql0_010              # continue
#
# --- the WWN of this TMT matches the current target, determine if the ALPA
#     /LID has changed.
#
.inql0_030:
#
        stob    r14,tm_dsrc(g5)         # clear discovery source
c       r3 = isp_handle2alpa(r11,r7)
        bswap   r3,r3                   # this puts it in the same format
        ldos    tm_lid(g5),r4
#       c       fprintf(stderr,"inql0_030 tmt_lid %06lx new lid %06lX\n",r4,r7);
        cmpobne.t r4,r7,.inql0_037      # Jif TMT NOT on same handle
        ld      tm_alpa(g5),r4          # r4 = TMT alpa
#       c       fprintf(stderr,"inql0_030 tmt_alpa %06lx new alpa %06lX\n",r4,r3);
        cmpobne.t r4,r3,.inql0_045      # Jif TMT NOT on same ALPA
#
# ---   This TMT has same WWN and ALPA/LID, determine if a LUN discovery
#       is required. If it is, discovery all LUN's. Otherwise, activate
#       all TLMT, clear TMR1, and process next LID.
#
        ldob    tm_flag(g5),r3          # r3 = flag byte
        bbs     tmflg_lundisc,r3,.inql0_050 # Jif LUN's discovery required
#
# --- this ALPA has not changed and it has completed it's discovery,
#     enable it's LUN and try net ALPA/LID
#
        ldconst MAXLUN,r4               # r4 = index
        ldconst tlmstate_active,r6      # r6 = tlmt active state
.inql0_033:
        cmpobe  0,r4,.inql0_035         # end - exit
        subo    1,r4,r4                 # decrement index
        ld      tm_tlmtdir(g5)[r4*4],r5 # get TLMT address from TLMT LUN dir
        cmpobe  0,r5,.inql0_033         # JIf no TLMT
        stob    r6,tlm_state(r5)        # set TLMT active
        b       .inql0_033              # continue
#
.inql0_035:
        ldconst tmstate_active,r6       # r6 = active tmt
        stob    r14,tm_dsrc(g5)         # clear discovery source
        stob    r6,tm_state(g5)         # save new state
        stos    r14,tm_tmr0(g5)         # clear timer
        call    i$next_lid              # process next lid
        b       .inql0_1000             # exit
#
# --- The TMT has changed LID. Determine if the current LID was assigned by
#     fabric processing. If it was, release it to the LID pool and clear
#     it's entry in the CI_TMT directory.
#
.inql0_037:
        mov     r4,g0                   # g0 = loop ID
        call    i$put_lid               # return loop ID to pool

        ld      ici_tmdir(g4)[r4*4],r8  # r8 = previous dir entry
        cmpobne g5,r8,.inql0_045        # Jif prev not same as current TMT addr
        st      r14,ici_tmdir(g4)[r4*4] # clear previous dir entry
.inql0_045:
        ldob    ici_chpid(g4),r11       # r11 = chip instance
c       r3 = isp_handle2alpa(r11,r7)
        bswap   r3,r3                   # this puts it in the same format
                                        # as the gan data
        st      r3,tm_alpa(g5)          # save alpa
        stos    r7,tm_lid(g5)           # save LID
.inql0_050:
        st      g5,oil1_tmt(r15)        # save tmt address in ILT
#
# --- determine if this target is a XIOtech type device. If it is,
#     determine if it has initiator support. If it does not, delete
#     the target then do a discovery.
#
c       g6 = M_chk4XIO(*(UINT64*)&g6);  # is this a XIOtech Controller ???
        cmpobe.t 0,g6,.inql0_060        # Jif not a XIOtech device
#
        PushRegs(r3)
        ldob    ici_chpid(g4),g0        # r11 = chip instance
        ldos    oil1_lid(r15),g1        # r7 = current lid
        call    fsl_isIFP
        mov     g0,r4
        PopRegsVoid(r3)
#
        cmpobne.f 0,r4,.inql0_060       # JIf initiator function present
#
# --- this is a XIOtech device without an initiator function. Fork
#     a task to delete the target from LLD/DLM
#
        ldconst tmstate_deltar,r4       # r3 = "delete target" state
        stob    r4,tm_state(g5)         # save new state
        ldconst tmdsrc_arb,r4           # r4 = discovered from arbitrated loop
        stob    r4,tm_dsrc(g5)          # save discovery source
        stob    r3,tm_NEWalpa(g5)       # save new alpa address
        lda     i$fabric_DeleteTarget,g0 # address of "Delete Target" process
        ldconst ITCPRI,g1               # set task priority
c       CT_fork_tmp = (ulong)"i$fabric_DeleteTarget";
        call    K$fork                  # create task

.if     MAG2MAG
        ldob    oil1_flag(r15),r3       # set suppress target ID to LLD
        setbit  oiflg_STID,r3,r3
        stob    r3,oil1_flag(r15)       # save flag byte
.endif  # MAG2MAG

        call    i$next_lid              # process next lid
        b       .inql0_1000             # exit
#
# --- determine if there is a device connected to LUN 0.
#
.inql0_060:
        ldos    oil1_lid(r15),r7        # r7 = current lid
        ldob    oil1_lun(r15),r6        # r6 = current lun

        st      g5,ici_tmdir(g4)[r7*4]  # save TMT in CIMT dir
        ldconst tmstate_active,r5       # r5 = "active" state
        stos    r14,tm_tmr0(g5)         # clear timer
        stob    r5,tm_state(g5)         # save new state
#
# --- isolate peripheral qualifier and device type
#
        ldob    0(r10),r3               # r3 = device type
        ldconst 0xe0,r4                 # r4 = mask
        and     r4,r3,r13               # r13 = peripheral qualifier
        and     0x1f,r3,r12             # r12 = peripheral device type
#
# --- determine if there is already an TLMT
#
        ld      tm_tlmtdir(g5)[r6*4],g6 # g6 = possible TLMT address
        cmpobne.f 0,g6,.inql0_080       # Jif already have one
#
# --- no TLMT, determine if one must be allocated
#
        cmpobne.f 0,r13,.inql0_075      # Jif no device not present
#
# --- device is present so allocate a TLMT and fill it in
#
c       g6 = get_tlmt();                # Allocate a TLMT
.ifdef M4_DEBUG_TLMT
c fprintf(stderr, "%s%s:%u get_tlmt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g6);
.endif # M4_DEBUG_TLMT
        st      g6,tm_tlmtdir(g5)[r6*4] # save TLMT address in TMT LUN dir
                                        # g4 = icimt address
                                        # g6 = tlmt
        call    i$qtlmt                 # place TLMT on active queue
                                        # Note: g7 destroyed
        ldob    ici_chpid(g4),r8        # r8 = chip instance
        st      g5,tlm_tmt(g6)          # save TMT address in TLMT
        st      g4,tlm_cimt(g6)         # save ICIMT address in TLMT
# NOTE: oil1_lun is a byte. and a new TLMT is zeroed.
        stos    r6,tlm_lun(g6)          # save LUN
#
# --- add 0x80 to session count. But don't readd if already there
#
        ldob    tlm_sescnt(g6),r4       # r4 = session count
        setbit  7,r4,r4                 # add 0x80
        stob    r4,tlm_sescnt(g6)       # save new session count
#
# --- OK, now that we have all the packets connected, lets determine
#     what kind of device we have and save the appro information.
#
.inql0_075:
        ldob    4(r10),r4               # r4 = get addition length (ADLEN)
        ldob    7(r10),r5               # r5 = flag byte
        stob    r12,tm_pdt(g5)          # save device type in tmt
        cmpobe.f 0,g6,.inql0_080        # Jif no TLMT allocated

        stob    r3,tlm_pdt(g6)          # save device type in tlmt
        stob    r5,tlm_dvflgs(g6)       # save flag byte in tlmt
#
.inql0_080:
        subo    8,r4,r4                 # remove header stuff
        shro    2,r4,r4                 # divide (ADLEN) by 4 to form words
        cmpo    r4,9                    # move a max of 9 words
        selg    r4,9,r4
        lda     8(r10),r11              # r11 = address of vendor id
        ldconst 0,r5                    # r5 = word index
#
.inql0_100:
        ld      (r11)[r5*4],r3          # get word
        st      r3,tm_venid(g5)[r5*4]   # store word in TLMT
        cmpobe.f 0,g6,.inql0_105        # Jif no TLMT allocated

        st      r3,tlm_venid(g6)[r5*4]  # store word in TLMT
#
.inql0_105:
        addo    1,r5,r5                 # bump index
        cmpoble.t r5,r4,.inql0_100      # continue till all copied
        cmpobe.f 0,g6,.inql0_200        # Jif no TLMT allocated

        ldconst 12,r3                   # default serial number length to 12
        stob    r3,tlm_snlen(g6)        # save serial number length
#
# --- set this TLMT active
#
        ldconst tlmstate_active,r5      # r5 = "active" state
        stob    r5,tlm_state(g6)        # set device active
#
# --- now let's determine if there is this is a direct access device.
#     If it is, send a mode sense command. Otherwise, process the LUN
#     on this LID.
#
.inql0_200:
        cmpobne.f 0,r13,.inql0_210      # Jif not device not present
        cmpobne.f 0,r12,.inql0_210      # Jif not direct access device

        st      r14,tlm_blkcnt(g6)      # clear lower block count
        st      r14,tlm_blkcnt+4(g6)    # clear upper block count
        st      r14,tlm_blksz(g6)       # clear block size

        lda     I$recv_cmplt_rsp,g2     # g2 = completion routine
        lda     .Ts_rcln,g3             # g3 = "read capacity lun n" event table
        call    i$send_rc               # send the read capacity command
        b       .inql0_1000             # br
#
.inql0_210:
        call    i$next_lun              # process next LUN on this LID
#
# --- Exit...
#
.inql0_1000:
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: i$genl0_cs
#
#  PURPOSE:
#       This routine provides general processing for a CS error response
#       received from a task issued to LUN 0.
#
#  DESCRIPTION:
#       A CS error was received from a task issued to LUN 0.
#       Determine the type of error by KEY and handle it.
#
#       key     description         process
#       -----   -----------         -----------------------------------
#        00     no error            retry task (?)
#        01     recovered error     retry task (?)
#        02     not ready           retry task
#        03     Medium error        try next LID
#        04     Hardware error      try next LID
#        05     illegal command     try next LID
#        06     unit attention      retry task
#        07     Data protect        retry task (should never happen)
#        08     Blank Check         retry task (should never happen)
#        09     Vendor              retry task (should never happen)
#        0A     copy aborted        retry task (should never happen)
#        0B     aborted command     retry task
#        0C     obsolete
#        0D     Volume Overflow     retry task (should never happen)
#        0E     miscompare          retry task (should never happen)
#
#  CALLING SEQUENCE:
#       call    i$genl0_noncs
#
#  INPUT:
#       g0 = SCSI or Qlogic completion status
#       g1 = ILT at Initiator level 2.
#       g4 = ICIMT address
#       g11 = status type 0 IOCB (if g0 <> 0)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g2 and g3 are destroyed.
#
#******************************************************************************
#
i$genl0_cs:
        lda     -ILTBIAS(g1),r15        # r15 = ILT at 1st level

        ldob    oil1_flag(r15),r4       # get ILT flag byte
        bbs     oiflg_PDBC,r4,.genl0_cs_910 # Jif data base changed

        ldob    oil1_chpid(r15),r8
c       r9 = ICL_IsIclPort((UINT8)r8);
        cmpobe  TRUE,r9,.genl0_cs_100   # Jif ICL port

        ld      iscsimap,r9             # r9 = iscsi port map
        bbc     r8,r9,.genl0_cs_200     # Jif iSCSI port

.genl0_cs_100:
        ld      oil1_snsdata(r15),r7
        b       .genl0_cs_300

.genl0_cs_200:
        lda     0x24(g11),r9            # r9 = base of sense/response data
        ld      0x20(g11),r8            # r8 = response data len 0,4, or 8
        addo    r8,r9,r7                # r9 = add response len to pointer

.genl0_cs_300:
        ldob    2(r7),r3               # Get sense key and isolate
        and     0x0F,r3,r3
        cmpobe  0x03,r3,.genl0_cs_900   # Jif Medium error
        cmpobe  0x04,r3,.genl0_cs_900   # Jif Hardware error
        cmpobe  0x05,r3,.genl0_cs_900   # Jif illegal command
#
# --- Determine if the retry counter has expired. If it has, exit to
#     next LID, otherwise setup to reissue the task.
#
        ldob    oil1_retry(r15),r4      # r4 = retry counter
        cmpobe.t 0,r4,.genl0_cs_900     # Jif retry counter expired - give up
        subo    1,r4,r4                 # decrement retry count
        stob    r4,oil1_retry(r15)      # save new value

        call    i$reissue_task          # reissue task
        b       .genl0_cs_1000          # b

.genl0_cs_900:
.if     MAG2MAG
        ldob    oil1_flag(r15),r3       # r3 = flag byte
        setbit  oiflg_STID,r3,r3        # set suppress target ID to LLD
        stob    r3,oil1_flag(r15)       # save flag byte
.endif  # MAG2MAG

.genl0_cs_910:
        call    i$next_lid              # process next LID

.genl0_cs_1000:
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: i$genl0_noncs
#
#  PURPOSE:
#       This routine provides general processing for a non CS error response
#       received from a task issued to LUN 0.
#
#  DESCRIPTION:
#       A non CS error was received from a task issued to LUN 0.
#       Determine the type of error and handle it.
#
#       error   description         process
#       -----   -----------         -----------------------------------
#       08h     busy                Retry this task
#       1ch     queue full          Retry this task
#       18h     reserve conflict    (?) Try next LID (?)
#       22h     command terminated  Try next LID
#       28h     Task Set Full       Retry this task
#       30h     ACA error           Try next LID
#
#  CALLING SEQUENCE:
#       call    i$genl0_noncs
#
#  INPUT:
#       g0 = SCSI or Qlogic completion status
#       g1 = ILT at Initiator level 2.
#       g4 = ICIMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g2 and g3 are destroyed.
#
#******************************************************************************
#
i$genl0_noncs:
        lda     -ILTBIAS(g1),r15        # r15 = ILT at 1st level

        ldob    oil1_flag(r15),r4       # get ILT flag byte
        bbs     oiflg_PDBC,r4,.genl0_noncs_910 # Jif data base changed

#
# --- determine error type
#
        ldconst 0x18,r3                 # reserve conflict
        ldconst 0x22,r4                 # command terminated
        ldconst 0x30,r5                 # ACA error

        cmpobe  r3,g0,.genl0_noncs_900  # Jif reserve conflict (?)
        cmpobe  r4,g0,.genl0_noncs_900  # Jif command terminated
        cmpobe  r5,g0,.genl0_noncs_900  # Jif ACA error
#
# --- Determine if the retry counter has expired. If it has, exit to
#     next LID, otherwise setup to reissue the task.
#
        ldob    oil1_retry(r15),r4      # r4 = retry counter
        cmpobe.t 0,r4,.genl0_noncs_900  # Jif retry counter expired - give up
        subo    1,r4,r4                 # decrement retry count
        stob    r4,oil1_retry(r15)      # save new value

        call    i$reissue_task          # reissue task
        b       .genl0_noncs_1000       # b

.genl0_noncs_900:
.if     MAG2MAG
        ldob    oil1_flag(r15),r3       # r3 = flag byte
        setbit  oiflg_STID,r3,r3        # set suppress target ID to LLD
        stob    r3,oil1_flag(r15)       # save flag byte
.endif  # MAG2MAG

.genl0_noncs_910:
        call    i$next_lid              # process next LID

.genl0_noncs_1000:
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: i$genl0_ioe
#
#  PURPOSE:
#       This routine provides general processing for a I/O error response
#       received from a task issued to LUN 0.
#
#  DESCRIPTION:
#       An I/O error was received from a task issued to LUN 0.
#       Determine the type of error and handle it.
#
#       error   description     process
#       -----   -----------     -----------------------------------
#       02h     DMA error       End discovery process
#       04h     LIP reset       Retry the task
#       05h     Command aborted Retry the task
#       06h     timeout         Retry the task
#
#  CALLING SEQUENCE:
#       call    i$genl0_ioe
#
#  INPUT:
#       g0 = SCSI or Qlogic completion status
#       g1 = ILT at Initiator level 2.
#       g4 = ICIMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g2 and g3 are destroyed.
#
#******************************************************************************
#
i$genl0_ioe:
        lda     -ILTBIAS(g1),r15        # r15 = ILT at 1st level

        ldob    oil1_flag(r15),r4       # get ILT flag byte
        bbs     oiflg_PDBC,r4,.genl0_ioe_600 # Jif data base changed
#
# --- Determine if the error is a timeout or DMA error.
#
        cmpobe.t  0x02,g0,.genl0_ioe_900 # Jif DMA error
#
# --- iSCSI sg specific Error Handling
#
        cmpobe.t  0x81,g0,.genl0_ioe_900 # Jif No Connection
        cmpobe.t  0x84,g0,.genl0_ioe_900 # Jif Bad Target
        cmpobe.t  0x85,g0,.genl0_ioe_900 # Jif Abort
        cmpobe.t  0x88,g0,.genl0_ioe_900 # Jif Reset
        cmpobe.t  0x8c,g0,.genl0_ioe_100 # Jif Retry without decrementing count
        cmpobe.t  0x8d,g0,.genl0_ioe_100 # Jif Retry without decrementing count
#
# --- the error is either command aborted by LIP, command aborted, or a
#     Timeout. decrement the retry counter and reissue the task
#
        ldob    oil1_retry(r15),r4      # r4 = retry counter
        cmpobe.t 0,r4,.genl0_ioe_500    # Jif retry counter expired - give up
        subo    1,r4,r4                 # decrement retry count
        stob    r4,oil1_retry(r15)      # save new value
#
.genl0_ioe_100:
        call    i$reissue_task          # reissue task
        b       .genl0_ioe_1000         # b
#
# --- Retries have expired, try next LID...
#
.genl0_ioe_500:
.if     MAG2MAG
        ldob    oil1_flag(r15),r3       # r3 = flag byte
        setbit  oiflg_STID,r3,r3        # set suppress target ID to LLD
        stob    r3,oil1_flag(r15)       # save flag byte
.endif  # MAG2MAG

.genl0_ioe_600:
        call    i$next_lid              # process next LID
        b       .genl0_ioe_1000         # b
#
# --- DMA error, kill the discovery process
#
.genl0_ioe_900:
        call    i$scan_flush            # end the scan process

.genl0_ioe_1000:
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: i$genl0_misc
#        i$genln_misc
#
#  PURPOSE:
#       This routine provides general processing for a misc error response
#       received from a task issued to LUN 0 or LUN n.
#
#  DESCRIPTION:
#       An misc error was received from a task issued to LUN 0 or LUN n.
#       Determine the type of error and handle it.
#
#       error   description             process
#       -----   -----------             -----------------------------
#       28h     Port unavailable        Reactivate discovery process.
#       29h     Port logged off         Reactivate discovery process.
#       2ah     port configuration      Reactivate discovery process.
#        NOTE: See fsl_sgio_cb function in fsl.c for iscsi ME values
#
#     The call to online should set up the discovery ILT in a manner that
#     will activate a rediscovery upon any event. This routine will use the
#     misc event.
#
#  CALLING SEQUENCE:
#       call    i$genl0_misc
#       call    i$genln_misc
#
#  INPUT:
#       g0 = SCSI or Qlogic completion status
#       g1 = ILT at Initiator level 2.
#       g4 = ICIMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
i$genl0_misc:
i$genln_misc:
        lda     -ILTBIAS(g1),r15        # r15 = ILT at 1st level
#
# --- the port of the intended IO has logged off, is unavailable, or the
#     port configuration has changed. Restart the Discovery process.
#
.if     MAG2MAG
        ldob    oil1_flag(r15),r3       # r7 = flag byte
        setbit  oiflg_STID,r3,r3        # set suppress target ID to LLD
        stob    r3,oil1_flag(r15)       # save flag byte
.endif  # MAG2MAG

        ldob    oil1_tmode(r15),r4      # r4 = task type
        cmpobne.f oimode_parent,r4,.genl_misc_150 # Jif not parent task

        ldob    oil1_flag(r15),r4       # r4 = flags byte
        setbit  oiflg_PDBC,r4,r4        # set port data base changed indication
        stob    r4,oil1_flag(r15)       # save save new value

        ld      oil1_tmt(r15),r5        # r5 = possible tmt address
        cmpobe  0,r5,.genl_misc_150     # jif no tmt defined

#        ldconst 0xff,r4                 # r4 = invalid ALPA flag
#        stob    r4,tm_alpa(r5)          # set alpa invalid

        ldconst CNTLTO,r6               # r6 = (default) controller t/o value
        ldos    tm_tmr0(r5),r3          # get current timer value
        cmpobne.t 0,r3,.genl_misc_150   # Jif already active
        stos    r6,tm_tmr0(r5)          # save t/o value
#
# --- since we already have the completion, the state machine requirement
#     of waiting for the task is not necessary, Just call restart_scan
#     directly.
#
.genl_misc_150:
        call    i$next_lid              # process next LID
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: i$turln
#
#  PURPOSE:
#       Receive good responses from an TUR command for LUN 0.
#
#  DESCRIPTION:
#       A good response was received from the device, This means that a SSU
#       command completed normally and an inquiry command should be sent to
#       the device.
#
#       The event handler table is set to ".Ts_inqln"
#
#  CALLING SEQUENCE:
#       call    i$turln
#
#  INPUT:
#       g0 = SCSI or Qlogic completion status
#       g1 = ILT at Initiator level 2.
#       g4 = ICIMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g2 and g3 are destroyed.
#
#******************************************************************************
#
i$turln:
        lda     -ILTBIAS(g1),r15        # r15 = 1st level of ILT
        ldob    oil1_flag(r15),r4       # get ILT flag byte
        bbc     oiflg_PDBC,r4,.turln_100 # Jif data base has not changed
        call    i$next_lun              # process next lun
        b       .turln_200

.turln_100:
        lda     I$recv_cmplt_rsp,g2     # g2 = completion routine
        lda     .Ts_inqln,g3            # g3 = "inquire lun n" event table
        call    i$send_inq              # send the inquiry command

.turln_200:
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: i$ssuln
#
#  PURPOSE:
#       Receive good responses from an SSU command for LUN 0.
#
#  DESCRIPTION:
#       A good response was received from the device, This means that the
#       SSU command is not required. Send an Inquiry command to the device.
#
#       The event handler table is set to ".Ts_inqln"
#
#  CALLING SEQUENCE:
#       call    i$ssuln
#
#  INPUT:
#       g0 = SCSI or Qlogic completion status
#       g1 = ILT at Initiator level 2.
#       g4 = ICIMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g2 and g3 are destroyed.
#
#******************************************************************************
#
i$ssuln:
        lda     -ILTBIAS(g1),r15        # r15 = 1st level of ILT
        ldob    oil1_flag(r15),r4       # get ILT flag byte
        bbc     oiflg_PDBC,r4,.ssuln_100 # Jif data base has not changed
        call    i$next_lun              # process next lun
        b       .ssuln_200

.ssuln_100:
        lda     I$recv_cmplt_rsp,g2     # g2 = completion routine
        lda     .Ts_turln,g3            # g3 = tur lun 0" event table
        call    i$send_tur              # send the tur command

.ssuln_200:
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: i$p_lun
#
#  PURPOSE:
#       Process LUN inquiry Response
#
#       Receive responses from an inquiry command, allocates and links control
#       packets, and start process to determine if other LUNs exist on this
#       AL-PA.
#
#  CALLING SEQUENCE:
#       call    i$inqln
#
#  INPUT:
#       g1 = ILT at Initiator level 2.
#       g4 = ICIMT
#       g7 = XLI
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
i$inqln:
        lda     -ILTBIAS(g1),r15        # r15 = 1st level of ILT

        ldob    oil1_flag(r15),r4       # get ILT flag byte
        bbs     oiflg_PDBC,r4,.inqln_130 # Jif data base changed

        ld      xlisglpt(g7),r9         # r9 = address of sg tbl
        ld      sg_addr(r9),r10         # r10 = inquiry response page
#
        ldob    oil1_lun(r15),r6        # r6 = current lun
        ldos    oil1_lid(r15),r7        # r7 = current lid
        ld      oil1_tmt(r15),g5        # g5 = TMT address ilt lvl 1
#
# --- isolate peripheral qualifer and device type
#
        ldob    0(r10),r3               # r3 = device type
        ldconst 0xe0,r4                 # r4 = mask
        and r4,r3,r13                   # r13 = peripheral qualifier
        and     0x1f,r3,r12             # r12 = peripheral device type
#
# --- determine if there is a TLMT already allocated
#
        ld      tm_tlmtdir(g5)[r6*4],g6 # g6 = possible TLMT address
        cmpobne.f 0,g6,.inqln_120       # Jif already have one
#
# --- no TLMT allocated, determine if one is required
#
        cmpobne.f 0,r13,.inqln_130      # Jif device not there
#
# --- Device is connected to the logical unit, so allocated a TLMT.
#
c       g6 = get_tlmt();                # Allocate a TLMT
.ifdef M4_DEBUG_TLMT
c fprintf(stderr, "%s%s:%u get_tlmt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g6);
.endif # M4_DEBUG_TLMT
        st      g6,tm_tlmtdir(g5)[r6*4] # save TLMT address in TMT LUN dir
                                        # g4 = icimt address
                                        # g6 = tlmt
        call    i$qtlmt                 # place TLMT on active queue
                                        # Note: g7 destroyed

        ldob    ici_chpid(g4),r8        # r8 = chip instance
        st      g5,tlm_tmt(g6)          # save TMT address in TLMT
        st      g4,tlm_cimt(g6)         # save ICIMT address in TLMT
# NOTE: oil1_lun is a byte. and a new TLMT is zeroed.
        stos    r6,tlm_lun(g6)          # save LUN
#
# --- add 0x80 to session count. But don't re-add if already there
#
        ldob    tlm_sescnt(g6),r4       # r4 = session count
        setbit  7,r4,r4                 # add 0x80
        stob    r4,tlm_sescnt(g6)       # save new session count
#
#
# --- OK, now that we have all the packets connected, lets determine
#     what kind of device we have and save the appro information.
#
        ldob    4(r10),r4               # r4 = get addition length (ADLEN)
        ldob    7(r10),r5               # r5 = flag byte

        stob    r12,tlm_pdt(g6)         # save device type in tlmt
        stob    r5,tlm_dvflgs(g6)       # save flag byte
#
        subo    8,r4,r4                 # remove header stuff
        shro    2,r4,r4                 # form word index
        cmpo    r4,9                    # move a max of 9 words
        selg    r4,9,r4
        lda     8(r10),r11              # r11 = address of vendor id
        ldconst 0,r5                    # r5 = word index
#
.inqln_100:
        ld      (r11)[r5*4],r3          # get word

        st      r3,tlm_venid(g6)[r5*4]  # store word in tlmt

        addo    1,r5,r5                 # bump index
        cmpoble.t r5,r4,.inqln_100      # LOOP until all copied

        ldconst 12,r3                   # default serial number length to 12
        stob    r3,tlm_snlen(g6)        # save serial number length
#
# --- set this TLMT active
#
.inqln_120:
        ldconst tlmstate_active,r5      # r5 = "active" state
        stob    r5,tlm_state(g6)        # set device active
#
# --- now let's determine if there is this is a direct access device.
#     If it is, send a mode sense command. Otherwise, process the LUN
#     on this LID.
#
        cmpobne.f 0,r13,.inqln_130      # Jif no device present
        cmpobne.f 0,r12,.inqln_130      # Jif not direct access device

        mov     0,r13
        st      r13,tlm_blkcnt(g6)      # clear lower block count
        st      r13,tlm_blkcnt+4(g6)    # clear upper block count
        st      r13,tlm_blksz(g6)       # clear block size

        lda     I$recv_cmplt_rsp,g2     # g2 = completion routine
        lda     .Ts_rcln,g3             # g3 = "read capacity lun n" event table
        call    i$send_rc               # send the read capacity command
        b       .inqln_1000             # br

.inqln_130:
        call    i$next_lun              # process next LUN
#
# --- Exit...
#
.inqln_1000:
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: i$turln_cs
#
#  PURPOSE:
#       Receive check status responses from an TUR command for LUN 0.
#
#  DESCRIPTION:
#       A check status response was received from the device, This means that
#       that there is some error that requires other processing.
#
#       key|ASC|ASCQ|   description              processing
#       --------------------------------------------------------------------
#       05h 25h 00h     LU not supported        Scan next LUN
#       02h 05h 00h     LU not responding       Scan next LID
#       02h 3Ah 00h     Medium not present      Send SSU cmd
#       02h 04h 00h     LU n/ready n/reportable Send SSU cmd
#       02h 04h 01h     LU becoming ready       Reschedule scan of LUN
#       02h 04h 02h     LU needs init cmd       Send SSU cmd
#       02h 04h 03h     Intervention required   Send SSU cmd
#       02h 04h 04h     Format in process       mark format/ next LUN
#       --h 28h 00h     Not RDY to RDY change   Send INQ cmd
#       06h 29h 00h     Power-on reset          Resend TUR cmd
#       06h 29h 02h     SCSI bus reset          Resend TUR cmd
#       06h 29h 03h     Device reset            Resend TUR cmd
#
#  CALLING SEQUENCE:
#       call    i$turln_cs
#
#  INPUT:
#       g0  = SCSI or Qlogic completion status
#       g1  = ILT at Initiator level 2.
#       g4  = ICIMT address
#       g11 = status type 0 IOCB (if g0 <> 0)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g5, g6, and g7 are destroyed.
#
#******************************************************************************
#
i$turln_cs:
        lda     -ILTBIAS(g1),r15        # r15 = 1st level of ILT

        ldob    oil1_flag(r15),r4       # get ILT flag byte
        bbs     oiflg_PDBC,r4,.turln_cs_410 # Jif data base changed
        ldob    oil1_chpid(r15),r8
#
# --- Check if the port type is ICL
#
#
c       r9 = ICL_IsIclPort((UINT8)r8);
        cmpobe  TRUE,r9,.turnln_cs_icl02   # Jif ICL support

        ld      iscsimap,r9            # r9 = iscsi port map
        bbc     r8,r9,.turln_cs_50      # Jif iSCSI port
.turnln_cs_icl02:
        ld     oil1_snsdata(r15),r7
        b       .turln_cs_55
#
.turln_cs_50:
        lda     0x24(g11),r9            # r9 = base of sense/response data
        ld      0x20(g11),r8            # r8 = response data len 0,4, or 8
        addo    r8,r9,r7                # r9 = add response len to pointer
.turln_cs_55:
        ldob    12(r7),r8               # r8 = ASC  data
        ldob    13(r7),r9               # r9 = ASCQ data
#  c fprintf(stderr,"idriver: i$turln_cs ASC %02lX ASCQ %02lX\n",r8,r9);
# ***********************************************************
#       ASC 28 - check for not ready to ready condition
# ***********************************************************
        ldconst 0x28,r3                 # load reset ASC
        cmpobe.t r3,r8,.turln_cs_225    # Jif not ready to ready

# ***********************************************************
#       ASC 29 - check for some form of reset
# ***********************************************************
        ldconst 0x29,r3                 # load reset ASC
        cmpobne.t r3,r8,.turln_cs_100   # Jif not some form of reset
        lda     I$recv_cmplt_rsp,g2     # g2 = completion routine
        lda     .Ts_turln,g3            # g3 = "test unit ready LUN 0" event table
        call    i$send_tur              # send the TUR command
        b       .turln_cs_1000          # b

# ***********************************************************
#       ASC 25 - check if LU not supported
# ***********************************************************
.turln_cs_100:
        ldconst 0x25,r3                 # LU not supported ???
        cmpobne.f r3,r8,.turln_cs_200   # Jif LU supported
#
# --- determine if this LUN was previously discovered
#
        ld      oil1_tmt(r15),g5        # g5 = TMT address ilt lvl 1
        ldob    oil1_lun(r15),r6        # r6 = current lun
        ld      tm_tlmtdir(g5)[r6*4],g6 # g6 = possible TLMT address
        cmpobe.f 0,g6,.turln_cs_410     # Jif none
#
# --- This lun was here and now it's gone. determine if it's resources
#     can be deallocated.
#
        ldob    tlm_sescnt(g6),r4       # r4 = session count
        scanbit r4,r5                   # r5 = MSB set
        bno.f   .turln_cs_225           # Jif zero (big time problem)
#
# --- the session count is non zero. Determine if the discovery process is
#     part of that session count. If it is, decrement the session count by
#     0x80. If the resulting session count is now zero, deallocate tlmt
#     and it's resources.
#
        cmpobne.f 7,r5,.turln_cs_225    # Jif discovery not part of count
        clrbit  r5,r4,r4                # remove discover count
        stob    r4,tlm_sescnt(g6)       # save new session count
        cmpobne.t 0,r4,.turln_cs_225    # Jif session count not zero
#
# --- session count is zero, remove tlmt and it's resources
#
        call    i$removeLUN             # remove lun and resources
        b       .turln_cs_225           # send inquiry

# ***********************************************************
#       ASC 4 - check for LU not ready conditions
# ***********************************************************
.turln_cs_200:
        ldconst 0x04,r3
        cmpobne.f r3,r8,.turln_cs_300   # Jif if not equal

# ***********************************************************
#       ASC 4 / ASCQ 1 - check if device becoming ready
# ***********************************************************
        ldconst 0x01,r3                 # r3 = "device becoming ready" status
        ldob    oil1_flag(r15),r7       # r7 = flag byte
        cmpobne.f r3,r9,.turln_cs_220   # Jif device not becoming ready
#
# --- device is becoming ready, determine if this task is a rediscovery
#     of this device.
#
        ldob    oil1_retry(r15),r4      # r4 = retry counter
        bbc     oiflg_LUN,r7,.turln_cs_210 # Jif not LUN only operation
#
# --- This is a rediscovery of the device. Determine if the retry counter
#     has expired. If it has, exit to next LUN (which should end this task),
#     otherwise setup to reissue TUR command.
#
        cmpobe.t 0,r4,.turln_cs_410     # Jif retry counter expired
        subo    1,r4,r4                 # decrement retry count
        stob    r4,oil1_retry(r15)      # save new value

        call    i$rescanln              # rescan device
        b       .turln_cs_1000          # b
#
# --- This is not a rediscovery task. The current task is sent on to
#     discover the next LUN. A new task is built to continue the discovery
#     of the current LUN.
#
.turln_cs_210:
        ld      oil1_chpid(r15),r6      # r6 = current chipid,mode,LUN,ALPA index
        ld      oil1_pid(r15),r5        # r5 = PID (ALPA)
        ld      oil1_tmt(r15),r8        # r8 = possible tmt
        ldos    oil1_lid(r15),r11       # r11 = lid
        ld      il_cr(r15),r4           # r4 = address of completion routine
        call    i$next_lun              # process next lun

        ldconst 512,g0              # g0 = set size for SGL
        call    i$alloc_dis_resource    # g1 = ILT for operation

        setbit  oiflg_LUN,r7,r7         # set LUN only in flags

        st      r5,oil1_pid(g1)         # save PID (ALPA)
        st      r6,oil1_chpid(g1)       # save  chipid,mode,LUN,ALPA index
        stob    r7,oil1_flag(g1)        # save flag byte
        st      r8,oil1_tmt(g1)         # save tmt address
        st      r4,il_cr(g1)            # save completion return address
        stos    r11,oil1_lid(g1)        # save lid
        #this updates the mode info brought from the previous ilt
        ldconst oimode_child,r4         # r4 = child task
        stob    r4,oil1_tmode(g1)        # save task as child

        cmpobe.t 0,r8,.turln_cs_215     # Jif no tmt
        ldob    oil1_lun(g1),r5         # r5 = lun

.turln_cs_215:
        lda     ILTBIAS(g1),g1          # bump to lvl 2 of ILT nest
        call    i$rescanln              # create task
        b       .turln_cs_1000          # b

# ***********************************************************
#       ASC 4 / ASCQ 4 - check if a format is in progress
# ***********************************************************
.turln_cs_220:
        ldconst 0x04,r3                 # is a format in process ???
        cmpobne.f r3,r9,.turln_cs_400   # Jif no

.turln_cs_225:
        lda     I$recv_cmplt_rsp,g2     # g2 = completion routine
        lda     .Ts_inqln,g3            # g3 = "inquiry LUN n" event table
        call    i$send_inq              # send the inquiry command
        b       .turln_cs_1000          # b

# ***********************************************************
#       ASC 5 - check if LU not responding
# ***********************************************************
.turln_cs_300:
        ldconst 0x05,r3                 # LU not responding ???
        cmpobne.f r3,r8,.turln_cs_400   # Jif yes

        call    i$next_lid              # process next lid
        b       .turln_cs_1000          # b
#
#       check for a no sense condition (an exabyte thing)
#

.turln_cs_400:
        ldob    oil1_chpid(r15),r8
c       r9 = ICL_IsIclPort((UINT8)r8);
        cmpobe  TRUE,r9,.turln_cs_icl01  # Jif ICL port
        ld      iscsimap,r9              # r9 = iscsi port map
        bbc     r8,r9,.turln_cs_450      # Jif not iSCSI port
.turln_cs_icl01:
        ldconst 0x20,r3                  # setting sense size for iSCSI
        b       .turln_cs_475
#
.turln_cs_450:
        ldos    0x16(g11),r9
        bbc     9,r9,.turln_cs_465
        ld      0x1C(g11),r3            # get sense data length
        b       .turln_cs_475
#
.turln_cs_465:
        ldconst 0,r3
.turln_cs_475:
        cmpobne.f 0,r3,.turln_cs_900    # Jif no sense length
.turln_cs_410:
        call    i$next_lun              # process next lun
        b       .turln_cs_1000          # b
#
#       all other cases, just send the SSU command
#
.turln_cs_900:
        lda     I$recv_cmplt_rsp,g2     # g2 = completion routine
        lda     .Ts_ssuln,g3            # g3 = "start/stop unit LUN 0" event table
        call    i$send_ssu              # send the SSU command

.turln_cs_1000:
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: i$rcl
#
#  PURPOSE:
#       Receive response from a read capacity command. Store the block count
#       and block size in the TLMT.
#
#  CALLING SEQUENCE:
#       call    i$rcl
#
#  INPUT:
#       g1 = ILT at Initiator level 2.
#       g4 = ICIMT address
#       g7 = XLI
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g5, g6, and g7 are destroyed.
#
#******************************************************************************
#
i$rcl:
        lda     -ILTBIAS(g1),r15        # r15 = 1st level of ILT

        ldob    oil1_flag(r15),r4       # get ILT flag byte
        bbs     oiflg_PDBC,r4,.rcl_1000 # Jif data base changed

        ld      xlisglpt(g7),r9         # r9 = address of sg tbl
        ld      sg_addr(r9),r10         # r10 = inquiry response page
#
        ldob    oil1_lun(r15),r6        # r6 = current lun
        ld      oil1_tmt(r15),g5        # g5 = TMT address ilt lvl 1

        ld      tm_tlmtdir(g5)[r6*4],g6 # g6 = possible TLMT address
        cmpobe.f 0,g6,.rcl_1000         # Jif not defined

        ld      0(r10),r4               # r4 = block count
        ld      4(r10),r5               # r5 = block size
        bswap   r4,r4                   # change endiance
        bswap   r5,r5                   # change endiance
        st      r4,tlm_blkcnt(g6)       # save lower block count
        st      0,tlm_blkcnt+4(g6)      # save upper block count
        st      r5,tlm_blksz(g6)        # save block size

.rcl_1000:
        call    i$next_lun              # process next lun
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: i$genln_cs
#
#  PURPOSE:
#       This routine provides general processing for a CS error response
#       received from a task issued to LUN n.
#
#  DESCRIPTION:
#       A CS error was received from a task issued to LUN n.
#       Determine the type of error by KEY and handle it.
#
#       key     description         process
#       -----   -----------         -----------------------------------
#        00     no error            retry task (?)
#        01     recovered error     retry task (?)
#        02     not ready           retry task
#        03     Medium error        try next LUN
#        04     Hardware error      try next LUN
#        05     illegal command     try next LUN
#        06     unit attention      retry task
#        07     Data protect        retry task (should never happen)
#        08     Blank Check         retry task (should never happen)
#        09     Vendor              retry task (should never happen)
#        0A     copy aborted        retry task (should never happen)
#        0B     aborted command     retry task
#        0C     obsolete
#        0D     Volume Overflow     retry task (should never happen)
#        0E     miscompare          retry task (should never happen)
#
#  CALLING SEQUENCE:
#       call    i$genln_cs
#
#  INPUT:
#       g0 = SCSI or Qlogic completion status
#       g1 = ILT at Initiator level 2.
#       g4 = ICIMT address
#       g11 = status type 0 IOCB (if g0 <> 0)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g2 and g3 are destroyed.
#
#******************************************************************************
#
i$genln_cs:
        lda     -ILTBIAS(g1),r15        # r15 = ILT at 1st level

        ldob    oil1_flag(r15),r4       # get ILT flag byte
        bbs     oiflg_PDBC,r4,.genln_cs_900 # Jif data base changed

        ldob    oil1_chpid(r15),r8
c       r9 = ICL_IsIclPort(r8);
        cmpobe  TRUE,r9,.genln_cs_100  # Jif ICL port
        ld      iscsimap,r9              # r9 = iscsi port map
        bbc     r8,r9,.genln_cs_200      # Jif iSCSI port

.genln_cs_100:
        ld      oil1_snsdata(r15),r7
        b       .genln_cs_300

.genln_cs_200:
        lda     0x24(g11),r9            # r9 = base of sense/response data
        ld      0x20(g11),r8            # r8 = response data len 0,4, or 8
        addo    r8,r9,r7                # r9 = add response len to pointer

.genln_cs_300:
        ldob    2(r7),r3               # Get sense key and isolate
        and     0x0F,r3,r3
        cmpobe  0x03,r3,.genln_cs_900   # Jif Medium error
        cmpobe  0x04,r3,.genln_cs_900   # Jif Hardware error
        cmpobe  0x05,r3,.genln_cs_900   # Jif illegal command
#
# --- Determine if the retry counter has expired. If it has, exit to
#     next LID, otherwise setup to reissue the task.
#
        ldob    oil1_retry(r15),r4      # r4 = retry counter
        cmpobe.t 0,r4,.genln_cs_900     # Jif retry counter expired - give up
        subo    1,r4,r4                 # decrement retry count
        stob    r4,oil1_retry(r15)      # save new value

        call    i$reissue_task          # reissue task
        b       .genln_cs_1000          # b

.genln_cs_900:
        call    i$next_lun              # process next LUN

.genln_cs_1000:
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: i$genln_noncs
#
#  PURPOSE:
#       This routine provides general processing for a non CS error response
#       received from a task issued to LUN n.
#
#  DESCRIPTION:
#       A non CS error was received from a task issued to LUN n.
#       Determine the type of error and handle it.
#
#       error   description         process
#       -----   -----------         -----------------------------------
#       08h     busy                Retry this task
#       1ch     queue full          Retry this task
#       18h     reserve conflict    (?) Try next LID (?)
#       22h     command terminated  Try next LID
#       28h     Task Set Full       Retry this task
#       30h     ACA error           Try next LID
#
#  CALLING SEQUENCE:
#       call    i$genln_noncs
#
#  INPUT:
#       g0 = SCSI or Qlogic completion status
#       g1 = ILT at Initiator level 2.
#       g4 = ICIMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g2 and g3 are destroyed.
#
#******************************************************************************
#
i$genln_noncs:
        lda     -ILTBIAS(g1),r15        # r15 = ILT at 1st level

        ldob    oil1_flag(r15),r4       # get ILT flag byte
        bbs     oiflg_PDBC,r4,.genln_noncs_900 # Jif data base changed

#
# --- determine error type
#
        ldconst 0x18,r3                 # reserve conflict
        ldconst 0x22,r4                 # command terminated
        ldconst 0x30,r5                 # ACA error

        cmpobe  r3,g0,.genln_noncs_900  # Jif reserve conflict (?)
        cmpobe  r4,g0,.genln_noncs_900  # Jif command terminated
        cmpobe  r5,g0,.genln_noncs_900  # Jif ACA error
#
# --- Determine if the retry counter has expired. If it has, exit to
#     next LID, otherwise setup to reissue the task.
#
        ldob    oil1_retry(r15),r4      # r4 = retry counter
        cmpobe.t 0,r4,.genln_noncs_900  # Jif retry counter expired - give up
        subo    1,r4,r4                 # decrement retry count
        stob    r4,oil1_retry(r15)      # save new value

        call    i$reissue_task          # reissue task
        b       .genln_noncs_1000       # b

.genln_noncs_900:
        call    i$next_lun              # process next LID

.genln_noncs_1000:
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: i$genln_ioe
#
#  PURPOSE:
#       This routine provides general processing for a I/O error response
#       received from a task issued to LUN n.
#
#  DESCRIPTION:
#       An I/O error was received from a task issued to LUN n.
#       Determine the type of error and handle it.
#
#       error   description     process
#       -----   -----------     -----------------------------------
#       02h     DMA error       End discovery process
#       04h     LIP reset       Retry the task
#       05h     Command aborted Retry the task
#       06h     timeout         Retry the task
#
#  CALLING SEQUENCE:
#       call    i$genln_ioe
#
#  INPUT:
#       g0 = SCSI or Qlogic completion status
#       g1 = ILT at Initiator level 2.
#       g4 = ICIMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g2 and g3 are destroyed.
#
#******************************************************************************
#
i$genln_ioe:
        lda     -ILTBIAS(g1),r15        # r15 = ILT at 1st level

        ldob    oil1_flag(r15),r4       # get ILT flag byte
        bbs     oiflg_PDBC,r4,.genln_ioe_900 # Jif data base changed

#
# --- Determine if the error is a timeout or DMA error.
#
        cmpobe.t  0x02,g0,.genln_ioe_900 # Jif DMA error
#
# --- iSCSI sg specific Error Handling
#
        cmpobe.t  0x81,g0,.genln_ioe_900 # Jif No Connection
        cmpobe.t  0x84,g0,.genln_ioe_900 # Jif Bad Target
        cmpobe.t  0x85,g0,.genln_ioe_900 # Jif Abort
        cmpobe.t  0x88,g0,.genln_ioe_900 # Jif Reset
        cmpobe.t  0x8c,g0,.genln_ioe_100 # Jif Retry without decrementing count
        cmpobe.t  0x8d,g0,.genln_ioe_100 # Jif Retry without decrementing count
#
# --- the error is either command aborted by LIP, command aborted, or a
#     Timeout. decrement the retry counter and reissue the task
#
        ldob    oil1_retry(r15),r4      # r4 = retry counter
        cmpobe.t 0,r4,.genln_ioe_900    # Jif retry counter expired - give up
        subo    1,r4,r4                 # decrement retry count
        stob    r4,oil1_retry(r15)      # save new value

.genln_ioe_100:
        call    i$reissue_task          # reissue task
        b       .genln_ioe_1000         # b
#
# --- DMA error, kill the discovery process
#
.genln_ioe_900:
        call    i$scan_flush            # end the scan process

.genln_ioe_1000:
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: i$inavild
#
#  PURPOSE:
#       Start any required process after the completion of initial
#       LUN scan.
#
#  CALLING SEQUENCE:
#       call    i$invalid
#
#  INPUT:
#       g1 = ILT at Channel Driver nest.
#       g4 = ICIMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
i$invalid:
        ld      I_undef_event,r4        # r4 = undefined event count
        addo    1,r4,r4                 # inc. counter
        st      r4,I_undef_event        # save updated count
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: i$scan_flush
#
#  PURPOSE:
#       Ends the scan process of the current task.
#
#  DESCRIPTION:
#       End the scan process of the current task. If it is a child task, it's
#       resources are released. If it is a parent task, the resources are
#       maintained and the ILT completion routine is called with a ME error.
#
#  CALLING SEQUENCE:
#       call    i$scan_flush
#
#  INPUT:
#       g1 = ILT at Initiator level 2.
#       g4 = ICIMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g0 and g1 are destroyed.
#
#******************************************************************************
#
i$scan_flush:
#
# --- If the current task is a child, release it's resources.
#
        lda     -ILTBIAS(g1),r15        # r15 = ILT at 1st lvl
        ldob    oil1_tmode(r15),r4      # r4 = task mode
        cmpobe.f oimode_parent,r4,.sflush_100 # JIf parent discover task

        call    i$rel_dis_resource      # release ILT and SGL
        b       .sflush_1000            # exit
#
# --- end the parent task by calling it's completion routine
#
.sflush_100:
        ldconst cmplt_ME,g0             # g0 = error status
        lda     -ILTBIAS(g1),g1         # g1 = ilt at 1st level
        ld      il_cr(g1),r6            # get completion routine address
        callx   (r6)                    # process end of scan stuff

.sflush_1000:
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: i$ignore
#
#  PURPOSE:
#       Perform no processing.
#
#  CALLING SEQUENCE:
#       call    i$ignore
#
#  INPUT:
#       g1 = ILT at Initiator level 2.
#       g4 = ICIMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
i$ignore:
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: i$iscan_complete
#
#  PURPOSE:
#       Start any required process after the completion of initial
#       LUN scan.
#
#  CALLING SEQUENCE:
#       call    i$iscan_complete
#
#  INPUT:
#       g0 = discovery completion status
#       g2 = discovery flags (oil1_flag)
#       g4 = ICIMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g0,g5,g6,g7 are destroyed
#
#******************************************************************************
#
# --- Entry for primary discovery task
#
i$iscan_complete:
#
# --- If the port is offline, do nothing
#
        ldob    ici_state(g4),r4        # r4 = loop state
        cmpobe.f Cs_offline,r4,.isn_cmplt_400 # JIf loop offline

#
# --- determine if there was a change in the port data base. If there was,
#     restart the discovery process.
#
        bbs  oiflg_ABRT,g2,.isn_cmplt_400 # JIf abort
        bbc  oiflg_PDBC,g2,.isn_cmplt_100 # JIf no port database change
                                        #     during discovery

        call    i$discovery             # restart discovery process
        b       .isn_cmplt_400          # br
#
# --- there was no form of data base change during discovery. determine if
#     there was a error that should end discovery. If no error perform
#     completion of the parent discovery scan task
#
.isn_cmplt_100:
        cmpobne.f cmplt_success,g0,.isn_cmplt_400   # Jif discovery ended in error

        ldob    ici_chpid(g4),r5        # r5 = chip ID
#
## Check for ICL port
#
c       r4 = ICL_IsIclPort(r5);
#if ICL_DEBUG
        cmpobe  FALSE,r4,.isn_cmplt_icl01
c fprintf(stderr,"%s%s:%u <i$iscan_complete>ICL port....\n", FEBEMESSAGE, __FILE__, __LINE__);
.isn_cmplt_icl01:
#endif  # ICL_DEBUG
        cmpobe  TRUE,r4,.isn_cmplt_150  # Jif ICL support

        ld      iscsimap,r4             # r4 = iscsi port map
        bbs     r5,r4,.isn_cmplt_150    # Jif iSCSI port
#
        ld      ispstr[r5*4],r14        # r14 = address into struct
        ld      ispicbstr(r14),r7       # r7 = ICB address
        # note the portname appears the same place in both the 2300 and 24000 icb structures
        # so this is safe but other fields specifically node name do not
        ldl     isppnm(r7),r8           # r8-r9 port name
        stl     r8,I$WWN                # and save it
        ld      ici_mypid(g4),r4        # r4 = PID
        bswap   r4,r4                   # change endiance
        st      r4,I$NID                # and save it
        ldconst TRUE,r3                 # r3 = true indicator
        stob    r3,I$IDFLAG             # set values active

.isn_cmplt_150:

.if     MAG2MAG

.if     ITRACES
        call    it$trc_disc_cmplt       # trace discovery complete
.endif  # ITRACES
        movq    g0,r4                   # save environment
        movq    g4,r8
        movq    g8,r12
        ldob    ici_chpid(g4),g0        # g0 = chip ID

        call    LLD$online              # notify link manager of online event
                                        # input:
                                        #        g0 = chip instance
                                        # output:
                                        #        none.
        movq    r4,g0                   # restore environment
        movq    r8,g4
        movq    r12,g8
.endif  # MAG2MAG

#
# --- Entry for completion of single LUN
#
        movq    g4,r8
        ld      ici_actqhd(g4),g6       # g6 = tlmt from active Q hd
        cmpobe.f 0,g6,.isn_cmplt_300    # Jif end of queue

.isn_cmplt_200:
        ld      tlm_tmt(g6),g5          # g5 = TMT address
        ldob    tm_state(g5), r4        # r4 = current TMT state
        cmpobne.f tmstate_active,r4,.isn_cmplt_220 # Jif device not active
        ldob    tlm_state(g6),r4        # r4 = current TLMT state
        cmpobne.f tlmstate_active,r4,.isn_cmplt_210 # Jif device not active

        call    apl$chknextask          # reissue tasks
        b       .isn_cmplt_250

.isn_cmplt_210:
        ldob    tm_state(g5), r4        # r4 = current TMT state

.isn_cmplt_220:
        cmpobne.f tmstate_inactive,r4,.isn_cmplt_250 # Jif device not inactive
#        PushRegs(r4)
#        ldob    tm_chipID(g5),g0
#        ldos    tm_lid(g5),g1
#        ld      ici_mypid(g4),g2        # get my alpa
#        call    ISP_LogoutFabricPort
#        PopRegsVoid(r4)
        ldob    tm_flag(g5),r4          # r4 = flag byte
        setbit  tmflg_lundisc,r4,r4     # set LUN discovery request flag
        stob    r4,tm_flag(g5)          # save
        ldconst TRUE,r6                 # r6 = (TRUE) rediscovery needed

.isn_cmplt_250:
        ld      tlm_flink(g6),g6        # g6 = link to next TLMT
        cmpobne.f 0,g6,.isn_cmplt_200   # Jif more TLMT's

.isn_cmplt_300:
        movq    r8,g4
        cmpobe.t FALSE,r6,.isn_cmplt_400 # JIf no rediscovery needed
#
# --- Skip rediscovery for iSCSI - will need a relook for GeoRep over iSCSI
#
        ldob    ici_chpid(g4),r6        # r6 = chip ID
        ld      iscsimap,r4             # r4 = iscsi port map
        bbs     r6,r4,.isn_cmplt_400    # Jif iSCSI port
#
# --- Check if rediscovery count is exhausted
#
        ldob    ici_rcnt(g4), r4        # r4 = current rediscovery count
        cmpobe.f 0,r4,.isn_cmplt_400    # JIf rediscovery count 0
#
# --- Wait for some time before kicking off another rescan
#
        ldconst 1000,g0                 # set up to wait for 1 sec before beginning scan
        divo    r4,g0,g0                # staggered retry delay
        call    K$twait                 # delay task
#
# --- Check if a discovery is in progress
#
        ld      ici_disQ(g4),r6         # r6 = discovery queue
        cmpobe.t 0,r6,.isn_cmplt_350    # JIf no discovery in process

.isn_cmplt_310:
        ldob    oil1_tmode(r6),r4       # r4 = flags
        cmpobne.t oimode_parent,r4,.isn_cmplt_320 # Jif not parent discovery task
#
# ---  discovery is in progress, set the PDBC flag and exit.
#
        ldob    oil1_flag(r6),r4        # r4 = flags byte
        setbit  oiflg_PDBC,r4,r4        # set port data base changed indication
        stob    r4,oil1_flag(r6)        # save save new value
        b       .isn_cmplt_400

.isn_cmplt_320:
        ld      oil1_dslink(r6),r6    # r6 = next ILT
        cmpobne.t 0,r6,.isn_cmplt_310 # Jif another ILT
#
# ---  No discovery is in progress, start a new one
#
.isn_cmplt_350:
        subo    1,r4,r4                 # decrement rediscovery counter
        stob    r4,ici_rcnt(g4)         # update rediscovery count
c fprintf(stderr,"%s%s:%u scan_complete Rescan ICIMT=0x%08x rcnt=0x%08x\n", FEBEMESSAGE, __FILE__, __LINE__,(UINT32)g4, (UINT32)r4);
        call    i$discovery             # restart discovery process

.isn_cmplt_400:
        ret
#
#******************************************************************************
#
#  NAME: i$rescanl0_call
#        i$rescanl0_fork
#        i$rescanl0_continue
#        i$rescanl0_offline
#
#  PURPOSE:
#       These routines are used to allow a delay of the rescan process.
#
#  DESCRIPTION:
#       i$rescanl0_call and i$rescanl0_fork
#               This routine sets up the default event handler and sets up
#               a delay of SCANDELAY. Upon completion of the delay, a call
#               is made to the current event handler with a normal
#               completion event.  i$rescanl0_call will execute this in the
#               callers process.  i$rescanl0_fork will fork a new process to
#               accomplish the delay anc call.
#
#       i$rescanl0_complete
#               This routine is the completion handler for a normal completion
#               of the scan delay. A call is made to i$send_tur.
#
#       i$rescan_offline
#               If an offline event is presented to the default event handler
#               table, this routine is called to change the event handler table.
#               the new table will flush the scan process.
#
#  CALLING SEQUENCE:
#       call    i$rescanl0_fork
#       call    i$rescanl0_call
#       call    i$rescanl0_continue
#       call    i$rescanl0_offline
#
#  INPUT:
#       g1 = ILT at 2nd level.
#       g4 = ICIMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
i$rescanl0_fork:
        movt    g0,r12                  # save g0-g3
        lda     .Ts_rescanl0_wait,r3    # r3 = event handler needs to be set up
                                        #   before the fork of the new process
        mov     g1,g2                   # g2 = ILT at nest level 2
        st      r3,oil2_ehand(g1)       # save event handler
        lda     .irescanl0_20,g0        # g0 = address to start the process at
        ldconst IRESCANPRI,g1           # g1 = Rescan process priority
c       CT_fork_tmp = (ulong)".irescanl0_20";
        call    K$tfork                 # fork the new process
        movt    r12,g0                  # restore g0-g3
        ret
#
i$rescanl0_call:
        movt    g0,r12                  # save g0-g3
        lda     .Ts_rescanl0_wait,r3    # r3 = event handler
        st      r3,oil2_ehand(g1)       # save event handler
        b       .irescanl0_50           # go do the delay
#
.irescanl0_20:
        movt    g0,r12                  # save g0-g3
        mov     g2,g1                   # g1 = ILT at nest level 2
.irescanl0_50:
        lda     SCANDELAY,g0            # set up to wait before beginning scan
        call    K$twait                 # delay task

        ld      oil2_ehand(g1),r3       # r3 = event handler base
        ldconst Te_success,r4           # r4 = event
        addo    r4,r3,r3                # add completion event type
        ld      (r3),r3
        callx   (r3)                    # process event
        movt    r12,g0                  # restore g0-g3
        ret                             # return to caller
#
i$rescanl0_continue:
        movq    g0,r12                  # save g0-g3
        lda     I$recv_cmplt_rsp,g2     # g2 = completion routine
        lda     .Ts_turl0,g3            # g3 = "lun 0" event table
        call    i$send_tur              # send TUR command
        movq    r12,g0                  # restore g0-g3
        ret                             # return to caller
#
i$rescanl0_offline:
        lda     .Ts_rescanl0_offline,r3 # r3 = event handler
        st      r3,oil2_ehand(g1)       # save event handler
        ret
#
#******************************************************************************
#
#  NAME: i$rescanln
#        i$rescanln_continue
#        i$rescanln_offline
#
#  PURPOSE:
#       These routines are used to allow a delay of the rescan process.
#
#  DESCRIPTION:
#       i$rescanln
#               This routine sets the default event handler and sets up a delay
#               of SCANDELAY. Upon completion of the delay, a call is made to
#               the current event handler with a normal completion event.
#
#       i$rescanln_complete
#               This routine is the completion handler for a normal completion
#               of the scan delay. A call is made to i$send_tur.
#
#       i$rescanln_offline
#               If an offline event is presented to the default event handler
#               table, this routine is called to change the event handler table.
#               the new table will flush the scan process.
#
#  CALLING SEQUENCE:
#       call    i$rescanln
#       call    i$rescanln_continue
#       call    i$rescanln_offline
#
#  INPUT:
#       g1 = ILT at 2nd level.
#       g4 = ICIMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
i$rescanln:
        lda     .Ts_rescanln_wait,r3    # r3 = event handler
        st      r3,oil2_ehand(g1)       # save event handler

        lda     SCANDELAY,g0            # set up to wait before beginning scan
        call    K$twait                 # delay task

        ld      oil2_ehand(g1),r3       # r3 = event handler base
        ldconst Te_success,r4           # r4 = event
        addo    r4,r3,r3                # add completion event type
        ld      (r3),r3
        callx   (r3)                    # process event
        ret                             # return to caller
#
i$rescanln_continue:
        lda     I$recv_cmplt_rsp,g2     # g2 = completion routine
        lda     .Ts_turln,g3            # g3 = "lun n" event table
        call    i$send_tur              # send TUR command
        ret                             # return to caller
#
i$rescanln_offline:
        lda     .Ts_rescanln_offline,r3 # r3 = event handler
        st      r3,oil2_ehand(g1)       # save event handler
        ret
#
#******************************************************************************
# ______________________Fabric Discovery Support__________________________________
#
#******************************************************************************
#
#  NAME: i$fabric_start
#
#  DESCRIPTION:
#     This is the task of the fabric discovery process. This loop
#     finds the ALPA's and WWID's of the ports and their nodes.
#
#  CALLING SEQUENCE:
#       k$tfork      i$fabric_start
#
#  INPUT:
#       g2 = ILT at 2nd level.
#       g4 = ICIMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Regs. g0-g3,g5-g7 destroyed.
#
#******************************************************************************
#
i$fabric_start:
        mov     g2,g1                   # g1 = ILT at 2nd level
        lda     -ILTBIAS(g1),r15        # r15 = ILT at 1st lvl

        ldconst 64,r14                  # set up rescan counter

        ldconst TRUE,r4                 # set ctl t/o inhibit to TRUE
        stob    r4,ici_FCTOI(g4)        # set inhibit flag
        b       .fabst_1st_GAN          # br
#
# --- delay the restart of the GAN loop and clear the PDBC bit in the
#     discovery the ILT.
#
.fabst_1st_GAN_delay:
        subo    1,r14,r14               # decrement rescan counter
        cmpobne 0,r14,.fabst_0xxx       # Jif not zero - delay and try again
#
# --- Reset QLogic port.
#
.fabst_reset:
        movl    g0,r4                   # save g0-g1
        ldob    ici_chpid(g4),g0        # g0 = chip instance
        ldconst ecri,g1                 # g1 = reset reason code
        call    ISP_ResetChip           # reset QLogic chip
        movl    r4,g0                   # restore g0-g1
        b       .fabst_0900             # abort this scan loop
#
.fabst_0xxx:
        ldconst 125,g0                  # set up to wait before beginning scan
        call    K$twait                 # delay task

        ld      oil2_ehand+ILTBIAS(r15),r4 # r4 = discovery event handler table
        lda     .Ts_start_fabric,r5     # r5 = expected event handler table addr
        cmpobne r4,r5,.fabst_0900       # Jif handler table has changed
#
#       clear port database change flag before first GAN...
#
.fabst_1st_GAN:

        ldob    oil1_flag(r15),r4       # get ILT flag byte
        clrbit  oiflg_PDBC,r4,r4        # clear port data base changed flag
        stob    r4,oil1_flag(r15)       # and save new value
#
# --- Register the FC4 type
#
        ldob    ici_chpid(g4),g0        # g0 = chip instance
        PushRegs(r8)                    # Save register contents
        ldconst 0x100,g1                # Set SCSI FC4 type
        ldconst 0,g2                    # Set null ptar.
        call   isp_registerFc4
        PopRegs(r8)                     # Restore registers (except g0)
#
# --- Get the 1st entry from the SNS and save it in the CIMT
#
        ldob    ici_chpid(g4),g0        # g0 = chip instance
        ld      fc4flgs,g1              # g1 = reg. FC4 flags
#        bbc     g0,g1,.fabst_0xxx       # Jif FC4 type registration not complete
        ldconst 0,g1                    # g1 = 1st alpa address
        call    ISP$get_all_next        # get information port
                                        #   g1 = Buffer address for response
                                        #   g0 = Completion code
        cmpobe  isgnnli,g0,.fabst_reset # if get_all_next returns nli status,then reset

        cmpobne.f 0,g0,.fabst_1st_GAN_delay # Jif error to restart scan

        lda     CT_HDR(g1),g1           # bump past CT-HDR

.if     ITRACES
        call    it$trc_1st_GAN          # trace 1st GAN event
.endif  # ITRACES

        ldconst PID_MASK,r11            # r11 = port ID mask
        ld      gan_ptype(g1),r5        # r5 = port ID
        ldconst 0x80000000,r8           # Check for valid Port ID
        cmpobe  r8,r5,.fabst_rstabt     # Jif not valid

        and     r5,r11,r5               # remove port type
        st      r5,ici_fstart(g4)       # save starting alpa
        st      r5,ici_fcur(g4)         # save current alpa
        b       .fabst_0050
#
# --- Determine if the port data base has changed during the previous loop.
#     If it has, restart the GAN loop. Otherwise, get the next entry from the
#     SNS and determine if it is the same as the 1st entry gathered.
#
.fabst_0000:
        ldob    oil1_flag(r15),r4       # get ILT flag byte
        bbs     oiflg_PDBC,r4,.fabst_1st_GAN_delay # Jif data base has changed

        ldob    ici_chpid(g4),g0        # g0 = chip instance
        ld      ici_fcur(g4),g1         # get current alpa
        bswap   g1,g1                   # change endianess
        call    ISP$get_all_next        # get information port
                                        #   g1 = Buffer address for response
                                        #   g0 = Completion code
        cmpobe  isgnnli,g0,.fabst_reset # if get_all_next returns nli status,then reset

        cmpobne.f 0,g0,.fabst_1st_GAN_delay # jif error to restart scan

        lda     CT_HDR(g1),g1           # bump past CT-HDR

.if     ITRACES
        call    it$trc_GAN              # trace GAN event
.endif  # ITRACES

        ld      ici_fstart(g4),r4       # r4 = starting alpa
        ldconst PID_MASK,r11            # r11 = port ID mask
        ld      gan_ptype(g1),r5        # r5 = port ID
        ldconst 0x80000000,r8           # Check for valid Port ID
        cmpobe  r8,r5,.fabst_rstabt     # Jif not valid

        and     r5,r11,r5               # remove port type
        cmpobe.f r4,r5,.fabst_0900      # Jif done with GAN scan
        st      r5,ici_fcur(g4)         # save new current

.fabst_0050:
        ldob    gan_FC4+2(g1),r3        # get the FC4 type
        cmpobne.t SCSI_FCP,r3,.fabst_0000 # Jif not SCSI FCP type device

        ldob    gan_ptype(g1),r4        # r4 = port type
        ldl     gan_P_name(g1),g6       # g6-g7 = port name
        ldl     gan_N_name(g1),r8       # r8-r9 = node name
        lda     gan_P_name(g1),g0       # g0 = ptr to port WWN name
        call    ISP$is_my_WWN           # check for my port WWN name
        cmpobe  TRUE,g0,.fabst_0000     # Jif my port WWN name
#
# --- Determine if foreign targets are enabled. If they are not, check data
#     base world wide name to make sure the target is a MAG before starting
#     a discovery process.
#
        ldob    ici_ftenable(g4),r3     # r3 = foreign target enable flag
        cmpobne.f 0,r3,.fabst_0070      # JIf foreign target enabled
        movl    g6,r6                   # save port name in r6-r7
c       g6 = M_chk4XIO(*(UINT64*)&g6);  # is this a XIOtech Controller ???
        cmpobe.t 0,g6,.fabst_0000       # Jif a foreign target
        movl    r6,g6                   # restore port name from r6-r7
#
# --- Check if this is a XIO BE port
#
        extract 12,4,r6                 # Get 'e' nibble from WWN
        cmpobe  0,r6,.fabst_0070        # Jif a Magnitude
        ldconst (WWNBPort>>20)&0xF,r3   # Value for BE port
        cmpobe  r6,r3,.fabst_0000       # Jif a XIOtech BE port

.fabst_0070:
        ld      ici_tmtQ(g4),g5         # g5 = possible TMT address
        ldob    ici_chpid(g4),r3        # r3 = chip instance

.fabst_0100:
        cmpobne.t 0,g5,.fabst_0200      # JIf not end of queue

c       g5 = get_tmt();                 # Allocate a TMT
.ifdef M4_DEBUG_TMT
c fprintf(stderr, "%s%s:%u get_tmt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g5);
.endif # M4_DEBUG_TMT

        ld      ici_tmtQ(g4),r11        # r11 = possible TMT address
        st      g5,ici_tmtQ(g4)         # link tmt to cimt tmt queue
        st      r11,tm_link(g5)         # link all other tmt's

        st      g4,tm_icimt(g5)         # save icimt address
        stl     r8,tm_N_name(g5)        # save node name
        stl     g6,tm_P_name(g5)        # save port name

        ldob    ici_chpid(g4),r3        # r3 = chip instance
        stob    r3,tm_chipID(g5)        # save chip instance
        b       .fabst_0350             # continue
#
.fabst_0200:
        ldl     tm_P_name(g5),r12       # r12-r13 tmt port name (WWID)
        cmpobne.f g6,r12,.fabst_0250    # JIf not a match
        cmpobne.f g7,r13,.fabst_0250    # Jif not a match

        ldl     tm_N_name(g5),r12       # r12-r13 = node WWN

        cmpobne.f r8,r12,.fabst_0250    # Jif node WWN does not match
        cmpobe.t r9,r13,.fabst_0300     # Jif node WWN is a MATCH !!!

.fabst_0250:
        ld      tm_link(g5),g5          # get next TMT
        b       .fabst_0100             # continue
#
.fabst_0300:
#
# --- this TMT is a match for the WWN, determine if this target is
#     in inactive state
#
        ldob    tm_state(g5),r3         # get tmt state
        cmpobe  tmstate_inactive,r3,.fabst_0310 # Jif inactive state

        cmpobne tmstate_undiscovered,r3,.fabst_0000 # Jif not undiscovered

        ldconst tmstate_discovery,r3    # r3 =  discovery state
        stob    r3,tm_state(g5)         # save new state
        st      r5,tm_alpa(g5)          # save port ID (alpa)
        b       .fabst_0000             # continue to next ALPA
#
# --- the state is correct, determine if the ALPA has changed.
#
.fabst_0310:
        ld      tm_alpa(g5),r4          # r4 = current alpa
        cmpobe.t r5,r4,.fabst_0318      # JIf alpa has not changed
#
# --- ALPA has changed, determine if the device is a XIOtech device.
#     If it is, issue a target gone to the LLD and rediscover the device.
#
c       g6 = M_chk4XIO(*(UINT64*)&g6);  # is this a XIOtech Controller ???
        cmpobe.t 0,g6,.fabst_0340       # Jif not a XIOtech device

        ld      tm_NEWalpa(g5),r4       # r4 = current new ALPA value
        st      r5,tm_NEWalpa(g5)       # save the new alpa
        cmpobne 0,r4,.fabst_0000        # skip deleting target process
                                        #  if new ALPA already processed
.fabst_0316:
        ldconst tmstate_deltar,r3       # r3 = "delete target" state
        ldconst tmdsrc_SNS,r4           # r4 = discovered from SNS
        stob    r3,tm_state(g5)         # save new state
        stob    r4,tm_dsrc(g5)          # save discovery source
        lda     i$fabric_DeleteTarget,g0 # address of "Delete Target" process
        ldconst ITCPRI,g1
c       CT_fork_tmp = (ulong)"i$fabric_DeleteTarget";
        call    K$fork                  # create task
        b       .fabst_0000             # continue to next ALPA
#
# --- the ALPA has not changed, determine if the LID has changed
#
.fabst_0318:
        movl    g0,r8                   # save g0-g1
        ldob    tm_chipID(g5),g0        # g0 = chip instance
        ldos    tm_lid(g5),g1           # g1 = LID

        ldconst NO_LID,r3               # Check for invalid LID
        cmpobe.f r3,g1,.fabst_0320      # Jif invalid LID

        bswap   r4,g2                   # g2 = ALPA
        ldconst 0x01,g3                 # g3 = no login if already logged in
        call    ISP$login_fabric_port   # g0 = completion code
        cmpobe.t 0,g0,.fabst_0330       # Jif normal completions



        ldos    tm_lid(g5),g0           # g1 = lid
        call    i$put_lid               # return lid

.fabst_0320:
        st      r4,tm_NEWalpa(g5)        # save the new alpa so del targ does rediscovery
        b       .fabst_0316             # go delete target
#
# --- this ALPA has not changed and it has completed its discovery,
#     enable its LUN and try next ALPA
#
.fabst_0330:
        ldob    tm_flag(g5),r3          # r3 = flag byte
        bbs     tmflg_lundisc,r3,.fabst_0500 # Jif LUN's discovery required

        movl    r8,g0                   # restore g0-g1
        ldconst MAXLUN,r4               # r4 = index
        ldconst tlmstate_active,r6      # r6 = tlmt active state

.fabst_0332:
        cmpobe  0,r4,.fabst_0335        # JIf last lun
        subo    1,r4,r4                 # decrement index
        ld      tm_tlmtdir(g5)[r4*4],g6 # get TLMT address from TLMT LUN dir
        cmpobe  0,g6,.fabst_0332        # JIf no TLMT
        stob    r6,tlm_state(g6)        # set TLMT active
        call    apl$chknextask          # Allow ops waiting to get kicked off
        b       .fabst_0332             # continue

.fabst_0335:
        ldconst tmdsrc_SNS,r4           # r4 = discovered from SNS
        ldconst tmstate_active,r6       # r6 = active tmt
        stob    r4,tm_dsrc(g5)          # save discovery source
        stob    r6,tm_state(g5)         # save new state
        mov     0,r3
        stos    r3,tm_tmr0(g5)          # clear timer
#
# --- determine if this target has been registered. If it hasn't, register it.
#
        ld      tm_ltmt(g5),r4          # is there a TLMT ???
        cmpobne 0,r4,.fabst_0000        # Jif yes
#
# --- inform link manager of target
#
        movq    g0,r4                   # save environment
        movq    g4,r8

        ldob    ici_chpid(g4),g0        # g0 = chip instance

.if     ITRACES
        call    it$trc_target_id        # trace discovery complete
.endif  # ITRACES
        ld      I_notreg,r3             # bump not registered counter
        addo    1,r3,r3
        st      r3,I_notreg
        ldob    ici_chpid(g4),g0        # g0 = chip instance
        call    LLD$target_ID           # id target to link manager
                                        #  input:
                                        #    g0 = chip instance
                                        #    g5 = tmt
                                        #  output:
                                        #    none
        movq    r4,g0                   # restore environment
        movq    r8,g4
        b       .fabst_0000             # continue to next ALPA
#
# --- logoff the current port
#
.fabst_0340:
#       c       fprintf(stderr,"fabst_0340\n");
        movq    g0,r8                   # save g0-g3
        ldconst 0,r3                    # r3 = 0
        ldos    tm_lid(g5),g1           # g1 = lid
        ldconst NO_LID,r6               # r6 = invalid lid
        cmpobe.f r6,g1,.fabst_0400      # Jif invalid lid

        st      r3,ici_tmdir(g4)[g1*4]  # clear TMT in CIMT dir

        mov     g1,g0                   # g0 = old LID
        call    i$put_lid               # return lid
        ldob    ici_chpid(g4),g0        # g0 = chip instance

                                        # g0 = chip instance
                                        # g1 = lid from lidt
        ld      ici_mypid(g4),g2        # get my alpa
        call    ISP_LogoutFabricPort    # logoff port
        movq    r8,g0                   # restore g0-g3
.fabst_0350:
        ldconst NO_LID,r6               # r6 = invalid lid
        stos    r6,tm_lid(g5)           # set LID invalid
.fabst_0400:
        st      r5,tm_alpa(g5)          # save port ID (alpa)

        ldob    tm_flag(g5),r3          # r3 = flag byte
        setbit  tmflg_lundisc,r3,r3     # set LUN discovery request flag
        stob    r3,tm_flag(g5)          # save
.fabst_0500:
        ldconst tmdsrc_SNS,r4           # r4 = discovered from SNS
        ldconst tmstate_discovery,r6    # r6 = discovery tmt
        stob    r4,tm_dsrc(g5)          # save discovery source
        stob    r6,tm_state(g5)         # save new state
        b       .fabst_0000             # continue
#
# --- GAN discovery is complete, send success event to event table
#     to start next process
#
.fabst_0900:
        ldconst FALSE,r4                # set ctl t/o inhibit to FALSE
        stob    r4,ici_FCTOI(g4)        # clr inhibit flag

        lda     ILTBIAS(r15),g1         # g1 = ILT at 2nd lvl
        ld      oil2_ehand(g1),r10      # r10 = ICIMT event handler address
        ldconst Te_success,r9           # r9 = event

        ld      ici_tmtQ(g4),g5         # g5 = possible TMT address

        addo    r10,r9,r10              # add completion event type
        ld      (r10),r10
        callx   (r10)                   # process event
                                        #  g1 = ILT at 2nd lvl
                                        #  g4 = cimt
                                        #  g5 = 1st tmt
        ret
#
# --- Port ID invalid (not updated) in GAN response, reset chip & abort scan.
#
.fabst_rstabt:
        movl    g0,r4                   # save g0-g1
        ldob    ici_chpid(g4),g0        # g0 = chip instance
        ldconst ecrinvganportid,g1      # g1 = reset reason code
.if ISP_DEBUG && !ISP_GAN_DEBUG
        call    isp$dump_ql             # Dump QLogic and reset chip
.else   # ISP_DEBUG && !ISP_GAN_DEBUG
        call    ISP_ResetChip           # reset QLogic chip
.endif  # ISP_DEBUG && !ISP_GAN_DEBUG
        movl    r4,g0                   # restore g0-g1
        b       .fabst_0900             # abort this scan loop
#
#******************************************************************************
#
#  NAME: i$fabric_continue
#
#  DESCRIPTION:
#     This is the second loop used in the Fabric discovery process. This
#     finds the LUNs attached to an alpa.
#
#  CALLING SEQUENCE:
#       call    i$fabric_continue
#
#  INPUT:
#       g1 = ILT at 2nd level.
#       g4 = ICIMT address
#       g5 = TMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Regs. g0-g3,g5-g7 destroyed.
#
#******************************************************************************
#
i$fabric_continue:
        cmpobe.t 0,g5,.fabcd_DiscCmplt  # JIf end of queue
#
# --- determine if there was a change in the port data base. If there was,
#     restart the discovery process.
#
        ldob    oil1_flag-ILTBIAS(g1),r4 # get the discovery flags
        bbc     oiflg_PDBC,r4,.fabcd_NoDBChg  # JIf no port database change
                                        #     during discovery
.fabcd_rediscover:
        clrbit  oiflg_PDBC,r4,r5        # clear discovery flag bit
        stob    r5,oil1_flag-ILTBIAS(g1) # save the discovery flags

        movt    g0,r12                  # save g0-g2
        ldconst .Ts_start,r5            # r5 = start event table
        st      r5,oil2_ehand(g1)       # save it

        lda     i$ploop_start_fork,g0   # establish discovery process
        mov     g1,g2                   # g2 = ILT at nest level 2
        ldconst ISCANPRI,g1             # g1 = Ploop Scan Task Priority
c       CT_fork_tmp = (ulong)"i$ploop_start_fork";
        call    K$tfork                 # create task
        movt    r12,g0                  # restore g0-g2
        b       .fabcd_end              # br
#
# --- There was no port datatbase change. Determine what state the TMT is in.
#
.fabcd_NoDBChg:
        ldob    tm_state(g5),r3         # r3 = tmt state
        cmpobne.f tmstate_discovery,r3,.fabcd_NxtTMT # JIf not discovery state

        ldob    tm_dsrc(g5),r3          # r3 = discovery source
        cmpobne.t tmdsrc_SNS,r3,.fabcd_NxtTMT # Jif not discovered from SNS
#
# --- The state indicates discovery is required. Prepare TMT for discovery
#
        call    i$fabric_DiscoverTMT    # discover this TMT

        cmpobe  dTMT_pend,g3,.fabcd_end # Jif process end requested
        cmpobe  dTMT_DiscEnd,g3,.fabcd_DiscEnd # Jif end discover
        cmpobne dTMT_restart,g3,.fabcd_NxtTMT # Jif we should process next TMT
# If here, then switch changed everything for us.
        ldob    oil1_flag-ILTBIAS(g1),r4 # get the discovery flags
        b       .fabcd_rediscover       # We know that the discovery flag is set
#
.fabcd_NxtTMT:
.if     ITRACES
        call    it$trc_floop            # trace Fabric loop event
.endif  # ITRACES
? # crash - cqt# 24349 - 2008-11-22 -- FE TMT - @ idriver.as:5919  ld 0+g5,g5 with acedaced
        ld      tm_link(g5),g5          # get next TMT
        b       i$fabric_continue       # continue
#
# --- Just end the discovery by releasing it's resources
#
.fabcd_DiscEnd:
        call    i$rel_dis_resource      # release discovery resources
        b       .fabcd_end              # exit
#
# --- all fabric ports have been scanned, release resources
#
.fabcd_DiscCmplt:
        lda     -ILTBIAS(g1),r15        # r15 = ILT at 1nd level
        ldob    oil1_flag(r15),g2       # get the discovery flags

        call    i$rel_dis_resource      # release ILT and SGL

        ldconst cmplt_success,g0        # g0 = good completion status
        call    i$iscan_complete        # complete the discovery process

.fabcd_end:
        ret                             # end process/return to caller
#
#**********************************************************************
# --- Fabric lun discovery completion entry point.
#
#     If the controller discovered is a XIOtech device, the
#     the LIDT is retained.
#
#     If there is still a discovery process still outstanding
#     for the controller, the LIDT is retained until the
#     completion of that process
#
#  INPUT:
#
#       g0 = completion status
#       g1 = ILT at 1st level
#       g4 = ICIMT address
#
#**********************************************************************
#
i$fabric_DiscCmplt:
        mov     g1,r15                  # R15 = ILT at 1st lvl
        lda     ILTBIAS(g1),g1          # g1 = ILT at 2nd level
        ld      oil1_tmt(r15),g5        # g5 = tmt address
#
# --- If completion status is non-zero
#
        cmpobne.f 0,g0,.fabdc_2290      # JIf scan had problems
#
# --- If the current task is a child, release it's resources. In all cases
#     discover the next device.
#
        ldob    oil1_tmode(r15),r4      # r4 = task mode
        cmpobe.f oimode_parent,r4,.fabdc_2500 # JIf parent discover task
#
# -------------------------------------------------------------------
# --- C H I L D   T A S K   P R O C E S S I N G
#     process is a child task. Determine if a parent task is still
#     active. If not, activate resources within the child tasks
#     domain.
#
        ld      ici_disQ(g4),r14        # r14 = discovery queue
        cmpobe.t 0,r14,.fabdc_2225      # JIf no discovery in process

.fabdc_2220:
        ldob    oil1_tmode(r14),r4      # r4 = flags
        cmpobe.t oimode_parent,r4,.fabdc_2290 # Jif parent discovery process

        ld      oil1_dslink(r14),r14    # r15 = next ILT
        cmpobne.t 0,r14,.fabdc_2220     # JIf more processes on queue
#
# --- There is no parent task active. determine the domain of
#     this child task
#
.fabdc_2225:
        cmpobe  0,g5,.fabdc_2290        # Jif not defined

        ldob    oil1_flag(r15),r4       # r4 = domain
        bbs     oiflg_LID,r4,.fabdc_2230 # Jif LID domain
#
# --- child task domain is a LUN
#
        ldob    oil1_lun(r15),r5        # r4 = lun number
        ld      tm_tlmtdir(g5)[r5*4],g6 # g6 = TLMT address from TLMT LUN dir
        cmpobe  0,g6,.fabdc_2290        # Jif no TLMT defines for this lun

        ldob    tlm_state(g6),r4        # r4 = current TLMT state
        cmpobne.f tlmstate_active,r4,.fabdc_2290 # Jif device not active

        call    apl$chknextask          # reissue tasks
        b       .fabdc_2290             # br
#
# --- child task domain is a LID
#
.fabdc_2230:
        ldconst 0,r5                    # r5 = lun index
        ldconst MAXLUN,r6               # r6 = max luns
.fabdc_2232:
? # crash - cqt@ 24596 - 2008-11-26 -- FE TMT - failed @ idriver.as:6838  ld 108+g5+(r5*4),g6 with acedaced
        ld      tm_tlmtdir(g5)[r5*4],g6 # g6 = TLMT address from TLMT LUN dir
        cmpobe  0,g6,.fabdc_2235        # Jif no TLMT defines for this lun

        ldob    tlm_state(g6),r4        # r4 = current TLMT state
        cmpobne.f tlmstate_active,r4,.fabdc_2235 # Jif device not active

        call    apl$chknextask          # reissue tasks
.fabdc_2235:
        addo    1,r5,r5                 # increment index
        cmpobl  r5,r6,.fabdc_2232       # Jif not last lun
#
# --- release child task resources
#
.fabdc_2290:
        call    i$rel_dis_resource      # release discovery resources
        b       .fabdc_10000            # exit
#
# -------------------------------------------------------------------
# --- P A R E N T   T A S K   P R O C E S S I N G
#     process is a parent task
#
.fabdc_2500:
? # crash - cqt# 24596 - 2008-12-23 -- FE TMT - idriver.as:6081  ld 0+g5,g5 with acedaced
        cmpobe  0,g5,.fabdc_2290        # In case tmt is freed memory
.ifdef M4_DEBUG_MEMORY_WITH_PATTERNS
        cmpobe  0xacedaced,g5,.fabdc_2290 # In case tmt is freed memory
.endif # M4_DEBUG_MEMORY_WITH_PATTERNS
        ld      tm_link(g5),g5          # get next TMT on link list

        movq    g0,r4                   # save most things
        movq    g4,r8
        movq    g8,r12

        call    i$fabric_continue       # continue fabric discovery
                                        # g1 = ILT at 2nd lvl
                                        # g4 = cimt
                                        # g5 = next tmt on queue
        movq    r4,g0                   # restore that which was saved
        movq    r8,g4
        movq    r12,g8
#
# --- exit
#
.fabdc_10000:
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: i$fabric_DiscoverTMT
#
#  PURPOSE:
#     This routine prepares a TMT for the Fabric discovery process.
#
#  CALLING SEQUENCE:
#       call    i$fabric_DiscoverTMT
#
#  INPUT:
#       g1 = ILT at 2nd level.
#       g4 = ICIMT address
#       g5 = tmt address
#
#  OUTPUT:
#       g3 - completion
#            dTMT_continue    - Continue with next TMT
#            dTMT_pend        - Process End
#            dTMT_DiscEnd     - Device Discovery end
#                               g0 = error status
#  REGS DESTROYED:
#       Regs. g0-g3,g6-g7 destroyed.
#
#******************************************************************************
#
i$fabric_DiscoverTMT:
        ldconst .Ts_fabric_TUR,r3       # r3 = fabric TUR event table
        st      r3,oil2_ehand(g1)       # save it
#
# --- determine if there is a LID already defined
#
        ldos    tm_lid(g5),r9           # r9 = current lid
        ldconst NO_LID,r3               # r3 = invalid lid
        cmpobne.f r3,r9,.fabdtmt_1150   # Jif lid defined
#
# -- no lid defined, so get one
#
        call    i$get_lid               # get a lid
        ldconst pe_nolid,r9             # r9 = "no more LIDs are available" error
        cmpobe.f 0,g0,.fabdtmt_1180_error # JIf no more lids

        mov     g0,r9                   # r9 = lid
        stos    g0,tm_lid(g5)           # save new lid
#
# --- try to logon to the port
#
.fabdtmt_1150:
        ldob    tm_chipID(g5),r8        # r8 = chip instance
        ld      tm_alpa(g5),r10         # r10 = alpa (PID)
        bswap   r10,r10                 # change endianess of ALPA (PID)
        movq    g0,r4                   # save g0-g3 in r4-r7

.fabdtmt_1170:
        movt    r8,g0                   # g0 = chip instance
                                        # g1 = lid from lidt
                                        # g2 = alpa
#
#       at this point we'll check for a TMT that had a timeout to a previous PLOGI.
#       if so, we'll process the TMT as if an ELS timeout had occurred, and start
#       a childtask to do the actual PLOGI. we'll continue to fake an ELS timeout
#       here in the discovery process for as long as the TMT exists, and as long
#       as we keep doing discoveries...
#
        ldob    tm_flag(g5),g3
        bbs     tmflg_badboy,g3,.fabdtmt_1180c
#
#       normal discovery continues here...
#
        ldconst 0x01,g3                 # g3 = no login if already logged in
        call    ISP$login_fabric_port   # g0 = completion code
#
#       if port database has changed, start over from scratch...
#
        lda     -ILTBIAS(r5),r15                    # preserve ILT1 for PDBC flag check...
        ldob    oil1_flag(r15),r15                  # get ILT flag byte
        bbs     oiflg_PDBC,r15,.fabdtmt_1180pdbc    # Jif data base has changed...
#
#       switch hasn't pulled the rug out from under us. continue with discovery...
#
        cmpobe.t 0,g0,.fabdtmt_1200       # Jif normal completion

.if     ITRACES
        call    it$trc_flog_e           # trace Fabric login error event
.endif  # ITRACES

        cmpobe.f islferr,g0,.fabdtmt_1180a # Jif parameter error
        cmpobe.f islfpiu,g0,.fabdtmt_1180b # Jif port in use by another LID message
        cmpobe.f islfliu,g0,.fabdtmt_1180a # Jif LID in use be another port
        cmpobe.f islfcmde,g0,.fabdtmt_1180c # Jif device not present or ELS t/o
                                           #   This will restart the discovery
                                           #   process.
#
# --- Default error condition handler
#
#     All ports (81 - ff) in use
#
.fabdtmt_1180:
        movq    r4,g0                   # restore g0-g3 from r4-r7

.fabdtmt_1180_error:
.if     ITRACES
        call    it$trc_floop_e          # trace Fabric loop error event
.if     debug_tracestop3
        ldconst 0,r3
        stos    r3,ici_tflg(g4)         # clear trace flags to stop traces
.endif  # debug_tracestop3
.endif  # ITRACES
        ldconst NO_LID,r4               # r4 = invalid lid
        stos    r4,tm_lid(g5)           # set LID invalid in tmt
        ldconst cmplt_ME,g0             # g0 = error status
        ldconst dTMT_DiscEnd,g3         # set to end discovery
        b       .fabdtmt_end            # process error
#
# --- Parameter error or LID used by another port
#
#     Parameter error - most likely cause is that the selected loop ID is
#                       the same as the ISP2100/ISP2200 loop ID. Leave
#                       the bit associated with the selected LID clear and
#                       find a new LID.
#
#     LID in use by   - This is most likely some of programming error but it
#     another port      should be handled in the same way as a parameter
#                       error.
#
.fabdtmt_1180a:
        call    i$get_lid               # get a lid
        cmpobe.f 0,g0,.fabdtmt_1180     # JIf no more lids
        mov     g0,r9                   # r9 = lid
        stos    g0,tm_lid(g5)           # save new lid
        b       .fabdtmt_1170           # br
#
# --- port is currently using another LID (the target side), so claim
#     target LID as our own.
#
.fabdtmt_1180b:
        mov     r9,g0                   # g0 = old LID
        call    i$put_lid               # return lid

        mov     g1,g0                   # g0 = new
        mov     g1,r9                   # r9 = new
        call    i$use_lid               # claim this LID as our own

        stos    r9,tm_lid(g5)           # save new lid
        b       .fabdtmt_1170           # br
#
# --- device not present or ELS t/o. restart the discovery process. Put back the
#     unused LID and clear LID in the TMT.
#
#     even if this isn't specifically a PLOGI failure, divert this port login process
#     error handling into a separate task...
#
.fabdtmt_1180c:
        mov     r9,g0                   # g0 = old LID
        call    i$put_lid               # return lid
        ldconst NO_LID,r3               # r3 = invalid lid
        stos    r3,tm_lid(g5)           # set LID invalid in tmt

        movq    r4,g0                   # restore g0-g3 from r4-r7
        lda     -ILTBIAS(g1),r15        # r15 = ilt at 1st lvl
#
#       g1 --> ILT level 2...
#
        call    i$fabric_RetryDiscoveryTaskInit # start up task to find this particular port...
#
        ldconst dTMT_continue,g3
        b       .fabdtmt_end
#
#       port database changed while we were away trying to PLOGI.
#       clean up the mess and leave...
#
.fabdtmt_1180pdbc:
        mov     r9,g0                   # g0 = old LID
        call    i$put_lid               # return lid

        movq    r4,g0                   # restore g0-g3 from r4-r7
        lda     -ILTBIAS(g1),r15        # r15 = ilt at 1st lvl
#
#       switch pulled the rug out from under us. stop discovery...
#
        ldconst dTMT_restart,g3         # Flag specially that we have to be careful.
        b       .fabdtmt_end
#
#
# --- Port is now logged on. Set up the ILT for the discovery, update
#     the port database, and determine if this port supports target.
#
.fabdtmt_1200:
        movq    r4,g0                   # restore g0-g3 from r4-r7

        ldob    tm_flag(g5),r3          # r3 = flag byte
        setbit  tmflg_dactive,r3,r3     # set discovery active flag
        stob    r3,tm_flag(g5)          # save

        lda     i$fabric_DiscCmplt,r4   # r4 = address of completion routine
        lda     -ILTBIAS(g1),r15        # r15 = ilt at 1st lvl

        ldconst oiflg_mask,r3           # r3 = mask
        ldob    oil1_flag(r15),r5       # r5 = flag value
        and     r3,r5,r5                # clear the naughty bits
        setbit  oiflg_LID,r5,r5         # set LID only in flags

        st      g5,oil1_tmt(r15)        # address of current TMT
        mov     0,r3
        stob    r3,oil1_lun(r15)        # set LUN 0
        stob    r3,oil1_lpmapidx(r15)   # clear alpa map index
        st      r4,il_cr(r15)           # save completion return address
        stob    r5,oil1_flag(r15)       # save flag value
        stob    r8,oil1_chpid(r15)      # save  chipid
        stos    r9,oil1_lid(r15)        # save LID
        st      r10,oil1_pid(r15)       # save PID (ALPA)
#
# --- Update the port datebase for this LID as required.
#
        call    i$updateportdb          # update port database

        ldob    ici_chpid(g4),r4        # r4 = chip instance
        ldos    tm_lid(g5),r5           # r5 = LID
        ldconst PORTDBSIZ,r13           # r13 = size of Port Database
        ld      portdb[r4*4],r14        # r14 = base of port database allocation
        mulo    r13,r5,r13              # r13 = offset = LID * PDB size
        addo    r13,r14,r14             # r14 =  Port DB address

        ldob    pdbprliw3(r14),r3       # get prli ser parm w3
#c fprintf(stderr,"idriver: PRLI w3 %04lX\n",r3);
        bbs     4,r3,.fabdtmt_1220        # JIf target supported
#
# --- target not active on this device, set up to rediscoverbut leave inactive
#     with timers set.
#
        ldob    oil1_flag(r15),r4       # r4 = flags byte
        setbit  oiflg_PDBC,r4,r4        # set port data base changed indication
        stob    r4,oil1_flag(r15)       # save save new value
        ldconst dTMT_continue,g3        # set to continue with next tmt
        b       .fabdtmt_end            # exit
#
# --- This ALPA/LID does not support target mode, logoff port, return
#     LID to pool, clear TMR0, and set TMT to server state.
#
#        movq    g0,r4                   # save g0-g3 in r4-r7
#        mov     r8,g0                   # g0 = chip instance
#        mov     r9,g1                   # g1 = LID
#        call    ISP$logout_fabric_port  # logout of port
#
#        bbc     7,r9,.fabdtmt_1210      # jif not an fabric lid
#        mov     r9,g0                   # g0 = chip instance
#        call    i$put_lid               # return lid
#
#.fabdtmt_1210:
#        movq    r4,g0                   # restore g0-g3 from r4-r7
#
#        ldconst tmstate_server,r6       # r6 = server tmt
#        stob    r6,tm_state(g5)         # r3 = tmt state
#        ldconst 0,r3
#        stos    r3,tm_tmr0(g5)          # clear timer
#
#        ldconst NO_LID,r4               # r4 = invalid lid
#        stos    r4,tm_lid(g5)           # set loop ID invalid
#        ldconst dTMT_continue,g3        # set to continue with next tmt
#        b       .fabdtmt_end            # try next device
#
# --- target mode is supported on this ALPA/LID, discover it's LUNs.
#
.fabdtmt_1220:
.if     ITRACES
        call    it$trc_floop            # trace Fabric loop event
.endif  # ITRACES
#
# --- set up for TUR command and call the state machine to activate it.
#
#     an online or offline event may have occurred during the fabric
#     logon or logoff operations.
#

        lda     I$recv_cmplt_rsp,g2     # g2 = completion routine
        ldconst .Ts_turl0,g3            # g3 = "lun 0" event table

        ld      oil2_ehand(g1),r10      # r10 = ICIMT event handler address
        ldconst Te_success,r9           # r9 = event
        addo    r10,r9,r10              # add completion event type
        ld      (r10),r10
        callx   (r10)                   # process event
#
# --- set return code to end process
#
        ldconst dTMT_pend,g3            # set to end process

.fabdtmt_end:
        ret                             # return to caller
#
#########################################################################################
#
#       start fabric retry PLOGI child process for the unresponsive device...
#
#       input
#         g0 - chip instance #
#         g1 - output ILT level 2
#         g5 - TMT@
#
#       output
#         same as entry
#
################################################################################
#
i$fabric_RetryDiscoveryTaskInit:
        lda     -ILTBIAS(g1),r15        # pick up and save caller's ilt level 1...
        ldconst tmstate_retry_plogi,r6  # wire TMT off from normal discovery process...
#
#       tidy up before firing up task. we've already discarded the LID#.
#       first, see if we're already running a child task. if so, return to caller...
#
        ldob    tm_state(g5),r4         # get TMT state
        cmpobe  r6,r4,.fabredisc000     # Jif already in retry PLOGI state...
.ifdef M4_ADDITION
c       if (r4 == tmstate_deltar) {     # Target is in process of being deleted
c           fprintf(stderr, "%s%s:%u i$fabric_RetryDiscoveryTaskInit TMT 0x%08lx in process of being deleted\n", FEBEMESSAGE,__FILE__, __LINE__, g5);
            b   .fabredisc000           # Exit
c       }
.endif  # M4_ADDITION
#
        stob    r6,tm_state(g5)
        ldconst 0,r3                    # disable disconnect timer...
        stos    r3,tm_tmr0(g5)
#
#       at this point, we should be preparing a single instance of the
#       retry PLOGI process associated with this TMT...
#
#       prepare copy of current task's ILT to retry PLOGI task's ILT. remember,
#       LID was invalidated by parent task processing...
#
        ldob    oil1_flag(r15),r7       # r7 = flag byte
        ld      oil1_chpid(r15),r6      # r6 = current chipid,mode,LUN,ALPA index
        ld      oil1_pid(r15),r5        # r5 = PID (ALPA)
        ldos    oil1_lid(r15),r11       # r11 = lid
        mov     g5,r8                   # r8 = this tmt address
        ld      il_cr(r15),r4           # r4 = @ of eventual completion routine...
#
#       allocate and build new ILT structure for retry PLOGI child task. puts ilt on
#       discovery queue...
#
        ldconst 512,g0                  # g0 = set size for workarea
        call    i$alloc_dis_resource    # g1 = ILT for operation (at 1st level)
#
#       our new ILT is in g1. child task domain is LID (device), though we
#       don't actually have one that's valid at the moment...
#
        setbit  oiflg_LID,r7,r7         # set scan this LID only in flags
#
#       load up our new ILT...
#
        st      r5,oil1_pid(g1)         # save PID (ALPA)
        st      r6,oil1_chpid(g1)       # save {chipid,mode,LUN,ALPA index}
        # mode will get properly set to child later
        stob    r7,oil1_flag(g1)        # save flag byte
        st      r8,oil1_tmt(g1)         # save this tmt address
        stos    r11,oil1_lid(g1)        # save lid
        st      r4,il_cr(g1)            # save completion return address
#
#       do some initial preparation of the tmt before the childtask is
#       activated...
#
        ldconst 0,g0
        stob    g0,tm_retry_plogi_count(r8)
#
#       save off some basic trace info...
#
        ld      i_replogi_count,g0
        addo    1,g0,g0
        st      g0,i_replogi_count      # bump total replogis...
        st      r8,i_replogi_tmt        # preserve last replogi tmt...
        st      g1,i_replogi_ilt        # preserve last replogi child ilt allocated...
#
#       mark this TMT mostly forever as a bad PLOGI TMT. normal discovery process will
#       then always start a childtask to handle this TMT's PLOGI...
#
        ldob    tm_flag(r8),g0
        setbit  tmflg_badboy,g0,g0
        stob    g0,tm_flag(r8)
#
#       at this point we've prepped the tmt. now if an online or offline
#       occurs while the childtask is in limbo (i.e., waiting to run), we'll not
#       blindly assume we have a valid tmt pointer while we're trying to set up the fsm
#       dispatcher (such as it is)...
#
#
        lda     ILTBIAS(g1),g0          # --> our new child ilt at level 2...

        ldconst .Ts_fabric_retry_plogi_startup,r3
        st      r3,oil2_ehand(g0)       # save it
#
#       set as child - cleanup is easier...
#
        ldconst oimode_child,r4         # r4 = child task
        stob    r4,oil1_tmode(g1)       # save task as child
#
#       g2 --> our new ilt level 1...
#
        mov     g1,g2
        ldconst ISDISCPRI,g1            # g1 = Start Discovery Priority
        lda     i$fabric_RetryDiscoveryChildtask,g0      # establish retry disc child task...
c       CT_fork_tmp = (ulong)"i$fabric_RetryDiscoveryChildTask";
        call    K$fork                  # start me up...
.fabredisc000:
#
#       restore caller's ilt level 2@...
#
        lda     ILTBIAS(r15),g1

        ret     # return to caller...
#
################################################################################
#       g2 --> child ILT level1
#       g5 --> TMT
#
i$fabric_RetryDiscoveryChildtask:
        mov     g2,g1                     # restore ILT-->
#
#       it's possible that an online or an offline occurred while the
#       child task was stuck in limbo. if so, the state table we're to use
#       has changed; we can't assume we have a valid tmt address either,
#       so we don't...
#
        ldconst Te_success,g6             # feed initialize event to state machine...
        mov     g1,r5                     # save our level1 ILT@...
#
#       to avoid pointless to-ing and fro-ing with the ILT@, just bump it once...
#
        lda     ILTBIAS(g1),g1            # set ILT lvl 2@ while we're here...

.fsm_loop:
        ld      oil2_ehand(g1),r10        # get current state table...
#
#       while unlikely, check for a null state pointer. if so, force a valid one
#       and drop into normal processing with a success event. we don't set the
#       address into the ILT, so we're depending on the called function (usually
#       i$e05) to return a 'stop' event. it might be unwise to set values into
#       what appears to be an ILT, plus we'd be covering up a defective pointer
#       value with a valid one...
#
        cmpobne 0,r10,.fsm_loop_valid    # check for reasonable state pointer...
        ldconst .Ts_fabric_retry_plogi_offline,r10
.fsm_loop_valid:

        addo    r10,g6,r10                # --> at the event for current state...
        ld      (r10),r10                 # pick up the function@...
        callx   (r10)                     # process event
#
#       event processor should set:
#         new state in ilt
#         next event in g6
#       note retry PLOGI termination discards the ILT before returning here.
#       note that the TMT isn't modified after the state has changed back to
#       discovery...
#
        ldconst Te_stop,r4
        cmpobe  r4,g6,.fsm_exit         # bail if terminating event...
#
        lda     -ILTBIAS(g1),r4         # set ILT lvl 1
        ld      oil1_tmt(r4),r10        # Get the TMT pointer
        cmpobne 0,r10,.fsm_10           # Jif still have TMT
#
#       there is no TMT. there must have been an online or offline, force a
#       cleanup. event table pointer (ehand) has been altered by this
#       point by online or offline processing. the abort will force a
#       scanflush, discarding the ILT, followed by a graceful childtask
#       exit. keep in mind the TMT formerly in use is likely in play by
#       now...
#
        ldconst Te_abort,g6             # Set abort status to force cleanup
        b       .fsm_loop
#
.fsm_10:
        ldconst Te_cont,r4
        cmpobne r4,g6,.fsm_loop
#
#       PLOGI fsm loop has completed successfully. now we have to
#       realign our processing to the usual idriver design philosophy...
#
#       force call to i$send_tur, which sets the current state table
#       to whatever is in g3, and the completion address to whatever
#       is in g2. this corresponds to the operation at the beginning of
#       i$fabric_DiscoverTMT...
#
        ldconst .Ts_fabric_TUR,r10      # r10 = ICIMT event handler address
        st      r10,oil2_ehand(g1)      # save it
#
#       set up for TUR command and call the fabric TUR success event handler...
#
#       an online or offline event may have occurred during the fabric
#       logon or logoff operations. this corresponds to the operation at
#       the end of i$fabric_DiscoverTMT (more or less)...
#
        lda     I$recv_cmplt_rsp,g2     # g2 = completion routine
        ldconst .Ts_turl0,g3            # next state table: "lun 0"

        ldconst Te_success,g6           # event

        addo    r10,g6,r10              # --> at the event for current state...
        ld      (r10),r10               # pick up the function@...
        callx   (r10)                   # process event#
#
#       all done. note that TUR processing returns g1 --> ilt level 3...
#
.fsm_exit:
        ret                              # terminate task...
#
################################################################################
#
#       asynchronous event glue logic
#
#       INPUT:
#
#       ILT level2 @ in g1...
#
#       OUTPUT:
#       none
#
#       not particularly compatible with the centralized state machine
#       dispatcher associated with the synchronous portions of
#       retry PLOGI...
#
#
#       timer active, wait for it to end before cleaning up offline...
#
i$rd_go_offline:
        ldconst .Ts_fabric_retry_plogi_offline,r10
        st      r10,oil2_ehand(g1)
#
#       we've effectively lost light on this side of the switch. wait for
#       whatever operations that are in progress to complete, then
#       clean up and end the retry PLOGI task...
#
        lda     -ILTBIAS(g1),r15
        ld      oil1_tmt(r15),r5        # get TMT@ from ILT...
#
#       check if either an offline or online has occurred earlier...
#
        cmpobe  0,r5,.rd_go_offline_00  # Jif no TMT...
.ifdef M4_ADDITION
c       if (((TMT*)r5)->state == tmstate_deltar) {
c           fprintf(stderr, "%s%s:%u i$rd_go_offline TMT 0x%08lx in process of being deleted -- what do we do??\n", FEBEMESSAGE,__FILE__, __LINE__, r5);
c           abort();
c       }
.endif  # M4_ADDITION
        ldconst tmstate_inactive,r4     # set TMT state to discoverable...
        stob    r4,tm_state(r5)
#
#       clear controller timeout...
#
        ldconst 0,r4
        st      r4,oil1_tmt(r15)
        stob    r4,tm_retry_plogi_count(r5)
        stos    r4,tm_tmr0(r5)
.rd_go_offline_00:
        ret
#
#       operation active, wait for it to end before cleaning up online.
#       we can arrive here via Loop Up, Port Database Change, or Change
#       Notification...
#
i$rd_go_online:
        ldconst .Ts_fabric_retry_plogi_online,r10
        st      r10,oil2_ehand(g1)
#
#       the switch wants to let us know something's changed in the fabric...
#
        lda     -ILTBIAS(g1),r15
        ld      oil1_tmt(r15),r5        # zero TMT@ in ILT...
        cmpobe  0,r5,.rd_go_online_00   # Jif no TMT
.ifdef M4_ADDITION
c       if (((TMT*)r5)->state == tmstate_deltar) {
c           fprintf(stderr, "%s%s:%u i$rd_go_online TMT 0x%08lx in process of being deleted -- what do we do??\n", FEBEMESSAGE,__FILE__, __LINE__, r5);
c           abort();
c       }
.endif  # M4_ADDITION
        ldconst tmstate_inactive,r4     # set TMT state to discoverable...
        stob    r4,tm_state(r5)
#
#       clear controller timeout, counter values...
#
        ldconst 0,r4
        st      r4,oil1_tmt(r15)
        stob    r4,tm_retry_plogi_count(r5)
        stos    r4,tm_tmr0(r5)
.rd_go_online_00:
        ret
#
#
################################################################################
#
#       retry PLOGI synchronous state machine functions...
#
#       inputs:
#       g1 --> childtask output ILT level2
#       g5 --> TMT
#
#       outputs:
#       next state in ILT
#       g1 --> ILT level 2
#       g6 = next event
#
################################################################################
#
#       pause to allow normal discovery to progress, and possibly
#       the current unresponsive device to wake up...
#
i$e00:
        ldconst .Ts_fabric_wait_plogi_delay,r3
        st      r3,oil2_ehand(g1)       # save it

        lda     RETRY_PLOGI_PAUSE,g0  # set up to wait before retry PLOGI...
        call    K$twait                 # delay task
        ldconst Te_timeout,g6           # issue timeout event to self...
        ret
#
#       cleaning up after an unsatisfactory PLOGI...
#
i$e01:
.ifdef M4_ADDITION
c       if (((TMT*)g5)->state == tmstate_deltar) {
c           fprintf(stderr, "%s%s:%u i$e01 TMT 0x%08lx in process of being deleted -- what do we do??\n", FEBEMESSAGE,__FILE__, __LINE__, g5);
c           abort();
c       }
.endif  # M4_ADDITION
        ldconst tmstate_inactive,r6     # set inactive state
        stob    r6,tm_state(g5)

        ldconst 0,r6                    # tidy up...
        stob    r6,tm_retry_plogi_count(g5)

        call    i$scan_flush            # discard ILT...
        ldconst Te_stop,g6              # issue exit retry PLOGI event to self...
        ret
#
#       discovery pause timer has popped, now to reissue the PLOGI...
#
i$e02:
#       g1 --> retry PLOGI childtask ILT level2
#       g5 --> TMT
#
#       incidentally, to restart port login processing for the port associated
#       with this TMT, we check the TMT state before activating the childtask
#       so we don't start up multiple retry PLOGI tasks for this TMT...
#
        ldconst .Ts_fabric_wait_plogi_resp,r3
        st      r3,oil2_ehand(g1)       # save it

        call    i$fabric_RetryDiscoverTMT
#
#       g3 has redispatchable event code;
#       at this time we can return with:
#
#       - Te_success (successful discovery, place into service)
#       - Te_ME (recoverable error, e.g., ELS timeout)
#       - Te_abort (stop discovery)
#
        mov     g3,g6
        ret
#
#       successful plogi. actual work is done in fsm exit processing...
#
i$e03:
.ifdef M4_ADDITION
c       if (((TMT*)g5)->state == tmstate_deltar) {
c           fprintf(stderr, "%s%s:%u i$e01 TMT 0x%08lx in process of being deleted -- what do we do??\n", FEBEMESSAGE,__FILE__, __LINE__, g5);
c           abort();
c       }
.endif  # M4_ADDITION
        ldconst tmstate_discovery,r6    # restore normal discovery state
        stob     r6,tm_state(g5)
        ldconst 0,r6                    # tidy up...
        stob    r6,tm_retry_plogi_count(g5)
        ldconst Te_cont,g6              # continue with TUR, etc.
#
#       Clear bad PLOGI TMT so normal discovery process will be used
#
        ldob    tm_flag(g5),r6
        clrbit  tmflg_badboy,r6,r6
        stob    r6,tm_flag(g5)

        ld      i_replogi_ok_count,r6
        addo    1,r6,r6
        st      r6,i_replogi_ok_count
        ret
#
#       unsuccessful plogi...
#
i$e04:
        ldconst RETRY_PLOGI_RETRIES,r3 #
        ldob    tm_retry_plogi_count(g5),r10
        cmpibge r3,r10,.e04_try_again       # if retry count reached...
#
#       give up...
#
        ldconst Te_abort,g6                 # set give up...
        b       .e04_done                   # else
#
.e04_try_again:
#
#       try again...
#
        addi    1,r10,r10
        stob    r10,tm_retry_plogi_count(g5) # bump count...
#
        ldconst .Ts_fabric_wait_plogi_delay,r3 # set retry PLOGI state...
        st      r3,oil2_ehand(g1)           # save it

        ldconst Te_success,g6               # set try again...
.e04_done:                                  # endif
        ret
#
#       clean up, set to end task. no TMT present...
#
i$e05:
        call    i$scan_flush            # discard ILT...
        ldconst Te_stop,g6              # issue exit retry PLOGI event to self...
        ret
#
#******************************************************************************
#
#  NAME: i$fabric_RetryDiscoverTMT
#
#  PURPOSE:
#     This routine prepares a TMT for the Fabric retry PLOGI process.
#
#  DESCRIPTION:
#     differs from i$fabric_DiscoverTMT by not retrying PLOGI immediately,
#     by dispatching events to retry PLOGI fsm, and because the task using this
#     path is dedicated to retry PLOGI of a individual device (TMT)...
#
#     while it's unpleasant to have two sets of code to do the same operation,
#     it's cleaner in that the normal discovery process doesn't have exception
#     retry PLOGI checks scattered here and there...
#
#  CALLING SEQUENCE:
#       call    i$fabric_RetryDiscoverTMT
#
#  INPUT:
#       g1 = ILT at 2nd level.
#       g4 = CIMT address
#       g5 = tmt address
#
#  OUTPUT:
#       g3 - next event
#
#  REGS DESTROYED:
#       Regs. g0-g3,g6-g7 destroyed.
#
#******************************************************************************
#
i$fabric_RetryDiscoverTMT:
#
#       determine if there is a LID already defined...
#
        ldos    tm_lid(g5),r9           # r9 = current lid
        ldconst NO_LID,r3               # r3 = invalid lid
        cmpobne.f r3,r9,.fabrdtmt_1150  # Jif lid defined
#
#       no loop ID defined, so get one, returned in g0...
#
        call    i$get_lid               # get a lid
        ldconst pe_nolid,r9             # r9 = "no more LIDs are available" err
        cmpobe.f 0,g0,.fabrdtmt_1180_error # JIf no more lids

        mov     g0,r9                   # r9 = lid
        stos    g0,tm_lid(g5)           # save new lid
#
#       try to logon to the port
#
.fabrdtmt_1150:
        ldob    tm_chipID(g5),r8        # r8 = chip instance
? # crash - cqt# 24330 - 2008-11-19 -- FE TMT - idriver.as:6850  ld 44+g5,r10 with acedaced
        ld      tm_alpa(g5),r10         # r10 = alpa (PID)
        st      r5,i_replogi_pid        # preserve last PID (ALPA) replogied...
        bswap   r10,r10                 # change endianess of ALPA (PID)
        movq    g0,r4                   # save g0-g3 in r4-r7 (r5 has ILT L2@)...
#
#       unlike normal discovery, we don't automatically retry
#       the PLOGI after an error...
#
        movt    r8,g0                   # g0 = chip instance
                                        # g1 = lid from lidt
                                        # g2 = alpa

        ldconst 0x01,g3                 # g3 = no login if already logged in
        call    ISP$login_fabric_port   # g0 = completion code
#
#       we're in a quandary here: if an online or offline event occurs while
#       we're waiting for the PLOGI to return, we can't be positive that the
#       outstanding PLOGI is immediately terminated; in other words, we might be
#       hung up here for awhile. note we don't have a parallel timer running to
#       catch PLOGI commands that never return - it's assumed they always do...
#
#       for online events, we have to return the TMT associated with the
#       outstanding PLOGI back to the well-behaved TMT fold before the online
#       event works its way down to normal discovery; that's why we dissociate
#       the TMT from the ILT in online processing, as a guarantee that whatever
#       childtask processing remaining can't dink with the badboy TMT. still,
#       though, we can't wait for the PLOGI request to complete to clean this up,
#       since the badboy TMT might be left out of the normal discovery process.
#       it is true that the badboy TMT will merely be dealt another childtask to
#       handle its PLOGI, but we'd really like to have everything neat and tidy
#       taskwise before the next childtask is fired up...
#
#       it would be nice to have the return codes from the PLOGI generate FSM
#       events, and we could gracefully deal with the absence of a valid TMT@ via
#       a distinct state table, but adding specific events to the decentralized
#       state machine that constitutes idriver might prove problematical. so, we
#       have the following awkward mechanism...
#
        lda     -ILTBIAS(r5),g3
        ld      oil1_tmt(g3),r15
        cmpobne 0,r15,.fabrd_have_tmt
#
#       at this point we don't have a valid TMT@, but we still need to do
#       something about the allocated loop ID. we couldn't willy-nilly do a
#       put_lid in offline/online processing, since we don't know the actual
#       proper disposition of the loop ID at that time - we have to wait for the
#       PLOGI to return so we know what to do with the allocated loop ID, based
#       on PLOGI return codes. essentially we do whatever the normal handling
#       does with the loop ID, but without the TMT references, and then we bail
#       out with a returned event code to be processed by the PLOGI flush...
#
        cmpobe.t 0,g0,.fabrdtmt_login           # Jif normal completion
        cmpobe.f islferr,g0,.fabrdtmt_nolid     # Jif parameter error
        cmpobe.f islfpiu,g0,.fabrdtmt_putlid    # Jif port in use by another LID
        cmpobe.f islfliu,g0,.fabrdtmt_nolid     # Jif LID in use by another port
        cmpobe.f islfcmde,g0,.fabrdtmt_putlid   # Jif device not present or ELS t/o
        b       .fabrdtmt_nolid                 # All ports (81 - ff) in use / default
#
.fabrdtmt_login:
#       we're logged in. we probably have to do something about that,
#       since we're probably not supposed to be now. if the logout
#       fails due to a port database change, no big deal. actually,
#       a port database change that occurred before we detect the TMT
#       clear is moot...
#
        mov     r9,g0                           # g0 = old LID
        call    i$put_lid                       # return lid
        movt    r8,g0                           # g0 = chip instance
                                                # g1 = lid from lidt
        ld      ici_mypid(g4),g2        # get my alpa
        call    ISP_LogoutFabricPort           # g0 = completion code
        b       .fabrdtmt_nolid
#
.fabrdtmt_putlid:
        mov     r9,g0                           # g0 = old LID
        call    i$put_lid                       # return lid
.fabrdtmt_nolid:
        movq    r4,g0                           # restore
        b       .fabrdtmt_abort         # return shutdown code...
#
#       normal, everyday have-a-valid-TMT-address processing...
#
.fabrd_have_tmt:
#
#       if port database has changed, start over from scratch...
#
        ldob    oil1_flag(g3),r15              # get ILT flag byte
        bbs     oiflg_PDBC,r15,.fabrdtmt_1180pdbc  # Jif data base has changed...
#
#       port database unchanged as far as we know. continue rePLOGI...
#
        cmpobe.t 0,g0,.fabrdtmt_1200    # Jif normal completion

.if     ITRACES
          call    it$trc_flog_e         # trace Fabric login error event
.endif  # ITRACES
#
#       some errors result in a later retry of the plogi, some
#       cause the retry PLOGI to die...
#
        cmpobe.f islferr,g0,.fabrdtmt_1180a # Jif parameter error

        cmpobe.f islfpiu,g0,.fabrdtmt_1180b # Jif port in use by another LID

        cmpobe.f islfliu,g0,.fabrdtmt_1180d # Jif LID in use by another port

        cmpobe.f islfcmde,g0,.fabrdtmt_1180c # Jif device not present or ELS t/o
                                           #   This will continue the retry PLOGI
                                           #   process.
#
# --- Default error condition handler
#     All ports (81 - ff) in use
#
.fabrdtmt_1180:
        movq      r4,g0                 # restore

.fabrdtmt_1180_error:
#
#       invalidate the LID#, since they're all busy...
#
        ldconst NO_LID,r4               # r4 = invalid lid
        stos    r4,tm_lid(g5)           # set LID invalid in tmt
        b       .fabrdtmt_abort         # return shutdown code...
#
#       Parameter error...
#
.fabrdtmt_1180a:
        call    i$get_lid
        cmpobe.f  0,g0,.fabrdtmt_1180   # go die after restoring regs...
        stos    g0,tm_lid(g5)           # pick up new lid#, save in tmt...
        b       .fabrdtmt_Te_ME         # wait, try again...
#
#       port is currently using another LID (the target side)...
#
.fabrdtmt_1180b:
        mov     r9,g0                   # g0 = old LID
        call    i$put_lid               # return lid

        mov     g1,g0                   # g0 = new
        mov     g1,r9                   # r9 = new
        call    i$use_lid               # claim this LID as our own

        stos    r9,tm_lid(g5)           # save new lid
        b       .fabrdtmt_Te_ME         # wait, try again...
#
#       device not present or ELS t/o, restart the discovery process...
#
#       g1 is returned with extended status on this error (4005)
#       if g1 is set to '04', then g2 contains additional event status...
#
#       note that if we detected the absence of a TMT on return from the
#       PLOGI we don't need to further refine the error status, since
#       we're going to abort the PLOGI process anyway...
#
.fabrdtmt_1180c:
        mov     r9,g0                   # g0 = old LID
        call    i$put_lid               # return lid

        ldconst NO_LID,r3               # r4 = invalid lid
        stos    r3,tm_lid(g5)           # set LID invalid in tmt
#
#       now we can get specific about the PLOGI response timeout...
#
        ldconst PLOGI_ELS_TO_NO_LOOP,r3 # possibly general PLOGI timeout error?...
        cmpobne r3,g1,.fabrdtmt_1180cx  # Jif not...

        ldconst PLOGI_ISSUED,r3         # was PLOGI issued?...
        cmpobe  r3,g2,.fabrdtmt_1180cc  # Jif it was...

        ldconst PLOGI_RESPONSE_WAIT,r3  # OK, were we waiting for response instead?...
        cmpobne r3,g2,.fabrdtmt_1180cx  # Jif we weren't...

.fabrdtmt_1180cc:
        b       .fabrdtmt_Te_ME         # wait, try again...

.fabrdtmt_1180cx:
        movq    r4,g0                   # restore ilt level2@
        b       .fabrdtmt_abort         # return shutdown code...
#
#       LID in use by another port...
#
.fabrdtmt_1180d:
        call    i$get_lid
        cmpobe.f  0,g0,.fabrdtmt_1180   # die if no lids...
        mov     g0,r9
        stos    g0,tm_lid(g5)
.fabrdtmt_Te_ME:
        movq    r4,g0                   # restore ilt level2@
        ldconst Te_ME,g3                # wait, try again...
        b       .fabrdtmt_end
#
#       port database changed while we were away trying to rePLOGI.
#       clean up the mess and leave...
#
.fabrdtmt_1180pdbc:
        mov     r9,g0                   # g0 = old LID
        call    i$put_lid               # return lid
        ldconst NO_LID,r3               # r3 = invalid lid
        stos    r3,tm_lid(g5)           # set LID invalid in tmt

        movq    r4,g0                   # restore g0-g3 from r4-r7
        lda     -ILTBIAS(g1),r15        # r15 = ilt at 1st lvl
        b       .fabrdtmt_abort         # return shutdown code...
#
#       Port is now logged on.
#       Set up our ILT for the discovery complete,
#       update the port database, and
#       determine if this port supports target...
#
.fabrdtmt_1200:
        movq    r4,g0                   # restore g0-g3 from r4-r7

        ldob    tm_flag(g5),r3          # r3 = flag byte
        setbit  tmflg_dactive,r3,r3     # set discovery active flag
        stob    r3,tm_flag(g5)          # save
#
#       point at task cleanup function, restart TLMT apl tasks if necessary,
#       dispose of ILT...
#
        lda     i$fabric_DiscCmplt,r4   # r4 = address of completion routine
        lda     -ILTBIAS(g1),r15        # r15 = ilt at 1st lvl
#
#       note that g1 still points at ILT level 2...
#
#       following manipulation of oil1_flag clears the
#         scan only this LUN
#       and
#         Suppress identification of target ID to LLD
#       flags...
#
        ldconst oiflg_mask,r3           # r3 = mask
        ldob    oil1_flag(r15),r5       # r5 = flag value
        and     r3,r5,r5                # clear the naughty bits
        setbit  oiflg_LID,r5,r5         # set LID only in flags
#
#       finally set the tmt--> into the ilt level 1...
#
        st      g5,oil1_tmt(r15)        # address of current TMT
        ldconst 0,g13
        stob    g13,oil1_lun(r15)       # set LUN 0
        stob    g13,oil1_lpmapidx(r15)  # clear alpa map index
        st      r4,il_cr(r15)           # save completion return address
        stob    r5,oil1_flag(r15)       # save flag value
        stob    r8,oil1_chpid(r15)      # save  chipid
        stos    r9,oil1_lid(r15)        # save LID
        st      r10,oil1_pid(r15)       # save PID (ALPA)
#
#       update the port datebase for this LID as required. issues
#       get port database command to Qlogic, and _may_ generate an
#       initiator logoff...
#
        mov     g1,r4                   # preserve ILT-->, just in case...
        call    i$updateportdb          # update port database
        mov     r4,g1                   # restore ILT-->...
#
#       we have to check after the port database update for an online event while the PDU was running.
#       if an online occurred, we don't have a valid TMT address anymore. also, assuming we need to do it,
#       send a logout command, then return an abort event to the retry plogi state machine dispatcher...
#
        ld      oil1_tmt(r15),r5        # if online occurred while we were away then...
        cmpobne 0,r5,.fabrdtmt_no_onl

        movt    g0,r4                   # ISP function clobber g-regs, so save {g0,g1,g2}...

        mov     r9,g0                   # pass old lid in g0...
        call    i$put_lid               # return lid...

        movl    r8,g0                   # g0 = chip instance
                                        # g1 = lid from lidt
        ld      ici_mypid(g4),g2        # get my alpa
        call    ISP_LogoutFabricPort    # g0 = completion code (but we don't appear to care)...

        movt    r4,g0                   # restore whatever was in {g0,g1,g2}...
        b       .fabrdtmt_abort         # return shutdown code...

.fabrdtmt_no_onl:
        ldob    ici_chpid(g4),r4        # r4 = chip instance
        ldos    tm_lid(g5),r5           # r5 = LID
        ldconst PORTDBSIZ,r13           # r13 = size of Port Database
        ld      portdb[r4*4],r14        # r14 = base of port database allocation
        mulo    r13,r5,r13              # r13 = offset = LID * PDB size
        addo    r13,r14,r14             # r14 =  Port DB address

        ldob    pdbprliw3(r14),r3       # get prli ser parm w3
        bbc     4,r3,.fabrdtmt_1230     # JIf target supported
#
#       target mode is supported on this ALPA/LID, place into service...
#
        ldconst Te_success,g3           # issue success event to self...
        b       .fabrdtmt_end           # exit
#
#       target not active on this device, terminate task, eventual port database
#       change should do the trick. we can't set the PDB change flag, since it's
#       in the parent's ilt, not ours...
#
.fabrdtmt_1230:
        ldos    tm_lid(g5),g0           # g0 = old LID
        call    i$put_lid               # return lid

        ldconst NO_LID,r3               # r4 = invalid lid
        stos    r3,tm_lid(g5)           # set LID invalid in tmt

.fabrdtmt_abort:
        ldconst Te_abort,g3
.fabrdtmt_end:
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: i$fabric_DeleteTarget
#
#  PURPOSE:
#     This process remove the TMT associations but leave the TMT intact
#     to be rediscovered.
#
#  CALLING SEQUENCE:
#       K$fork  i$fabric_DeleteTarget
#
#  INPUT:
#       g4 = ICIMT address
#       g5 = tmt address
#
#  REGS DESTROYED:
#       Regs. g0-g3,g6-g7 destroyed.
#
#******************************************************************************
#
i$fabric_DeleteTarget:
.ifdef M4_ADDITION
c       if (((TMT *)g5)->state != tmstate_deltar) {     # Target should be in process of being deleted
c           fprintf(stderr, "%s%s:%u i$fabric_DeleteTarget TMT 0x%08lx not in state tmstate_deltar (%d) -- what do we do??\n", FEBEMESSAGE,__FILE__, __LINE__, g5, ((TMT *)g5)->state);
c           abort();
c       }
.endif  # M4_ADDITION

        call    LLD$pre_target_gone     # check if OK to blow off target
        cmpobe.f TRUE,g0,.fabtg_140     # Jif OK to blow off target
#
# --- wait a little bit and try again
#
        lda     250,g0                  # set up to wait 250 ms
        call    K$twait                 # delay task
        b       i$fabric_DeleteTarget   # try again
#
# ---  It's OK to blow off targets
#
.fabtg_140:
        mov     0,r3                    # r3 = 0

        mov     r3,r7                   # r7 = zero index
        ldconst MAXLUN,r13              # r13 = maxlun

.fabtg_150:
        ld      tm_tlmtdir(g5)[r7*4],g6 # g6 = possible tlmt
        cmpobe.t 0,g6,.fabtg_165        # jif no tlmt defined

        call    i$removeLUN_no          # remove this LUN
                                        # ***********************************
                                        # *** with no session termination ***
                                        # *** indication                  ***
                                        # ***********************************
.fabtg_165:
        addo    1,r7,r7                 # bump index
        cmpobg.t r13,r7,.fabtg_150      # Jif not end of directory

.if     MAG2MAG
.if     ITRACES
        call    it$trc_target_gone
.if     debug_tracestop4
        stos    r3,ici_tflg(g4)         # clear trace flags to stop traces
.endif  # debug_tracestop4
.endif  # ITRACES
        call    LLD$target_GONE         # indicate target is gone
                                        # input:
                                        #   g5 = tmt
                                        # output:
                                        #   none:
.endif  # MAG2MAG
#
# --- Determine if there is a LID assigned to this tmt
#
        ldconst 0,r3                    # r3 = 0
        ldos    tm_lid(g5),g1           # g1 = lid
        ldconst NO_LID,r4               # determine if no LID is assigned
        cmpobe  r4,g1,.fabtg_185        # Jif no LID

        st      r3,ici_tmdir(g4)[g1*4]  # clear TMT in CIMT dir

        mov     g1,g0                   # g0 = old LID
        call    i$put_lid               # return lid
#
# --- logoff the current port
#
        ldconst NO_LID,r6               # r6 = invalid lid
        stos    r6,tm_lid(g5)           # set LID invalid

        ldob    ici_chpid(g4),g0        # g0 = chip instance
                                        # g1 = lid from lidt
        ld      ici_mypid(g4),g2        # g2 = my alpa
        call    ISP_LogoutFabricPort    # logoff port
#
# --- All Associations have been removed. Rediscover the device.
#
.fabtg_185:
        ldconst 0,r5                    # r3 = 0
        setbit  tmflg_lundisc,r5,r5     # set LUN discovery request flag
        stob    r5,tm_flag(g5)          # save
        ld      tm_NEWalpa(g5),r5       # r5 = new alpa address
        cmpobe  0,r5,.fabtg_300         # Jif new alpa is zero (error?)

        st      r5,tm_alpa(g5)          # save new alpa
        st      r3,tm_NEWalpa(g5)       # clear new alpa

        ldconst tmstate_inactive,r5     # r5 = inactive state
        stob    r5,tm_state(g5)         # save new current state
#
# --- determine if a parent task is active. If there is a parent task
#     active, set the PDBC flag to rediscover fabric. Otherwise, start a
#     new discovery process.
#
        ld      ici_disQ(g4),r15        # r15 = discovery queue
        cmpobe.t 0,r15,.fabtg_190       # JIf no discovery in process
#
# -- find possible parent task
#
.fabtg_187:
        ldob    oil1_tmode(r15),r6       # r6 = flags
        cmpobe.t oimode_parent,r6,.fabtg_200 # Jif parent discovery task

        ld      oil1_dslink(r15),r15    # r15 = next ILT
        cmpobne.t 0,r15,.fabtg_187      # JIf more task on discovery queue
#
# --- no parent task active. start new discovery process
#
.fabtg_190:
        call    i$discovery             # start a new discovery process
        b       .fabtg_300              # exit
#
# --- Parent task active. set flag indicating all devices must be rediscovered
#
.fabtg_200:
        ldob    oil1_flag(r15),r4       # r4 = flags byte
        setbit  oiflg_PDBC,r4,r4        # set port data base changed indication
        stob    r4,oil1_flag(r15)       # save save new value
#
# --- End Process
#
.fabtg_300:
        ret                             # end process
#
#******************************************************************************
# __________________________ SRP Handler Routines ___________________________
#******************************************************************************
#
#  NAME: I$SIF
#
#  PURPOSE:
#       Processes a Set Initiator Flags SRP request.
#
#  CALLING SEQUENCE:
#       call    lld$SIF
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
#******************************************************************************
#
I$SIF:
        movq    g0,r8                   # save g0-g3
        movq    g4,r12                  # save g4-g7
!       ld      sr_sif_flags(g2),r4     # r4 = initiator flags
!       ld      sr_sif_channel(g2),g0   # g0 = chip (channel) instance
        ld      I_CIMT_dir[g0*4],g4     # g4 = cimt address
        bbc     0,r4,.sif_200           # Jif foreign target not enabled
#
# --- request to enable foreign target
#
        ldob    ici_ftenable(g4),r4     # r4 = current enable flag
        cmpobne 0,r4,.sif_950           # Jif already set
        not     r4,r4                   # form a non-zero number
        stob    r4,ici_ftenable(g4)     # enable foreign targets
        b       .sif_900                # exit
#
# --- request to disable foreign target
#
.sif_200:
        ldob    ici_ftenable(g4),r4     # r4 = current enable flag
        cmpobe  0,r4,.sif_950           # Jif already clear

        mov     0,r3
        stob    r3,ici_ftenable(g4)     # disable foreign targets

#
# --- activate a discovery process to add or remove targets
#
.sif_900:
        ldob    ici_state(g4),r4        # r4 = loop state
        cmpobe.f Cs_offline,r4,.sif_950 # JIf loop offline

                                        # g0 = chip instance
        ldob    ici_mylid(g4),g1        # g1 = my loop ID
        ldconst ispofte,g2              # g2 = foreign target enable
        call    I$recv_online           # process an online

.sif_950:
        movq    r8,g0                   # restore g0-g3
        movq    r12,g4                  # restore g4-g7

        ldconst srok,g0                 # return good status
        lda     -ILTBIAS(g1),g1         # g1 = ILT/SRP at nest level #1
        ld      il_cr(g1),r4            # r4 = ILT completion handler routine
        callx   (r4)                    # call ILT/SRP completion handler routine
        ret
#
#******************************************************************************
# ____________________________ SUBROUTINES ____________________________________
#******************************************************************************
#
#  NAME: i$alloc_dis_resource
#
#  PURPOSE:
#       Allocates the resources required for the discovery process.
#
#  DESCRIPTION:
#       This routine allocated an ILT, a CDB and a SGL. The size of
#       the SGL is present in g0,
#
#  CALLING SEQUENCE:
#       call    i$alloc_dis_resource
#
#  INPUT:
#       g0 = size of SGL
#       g4 = ICIMT address
#
#  OUTPUT:
#       g1 = ITL at 1st level
#
#  REGS DESTROYED:
#       Reg. g0 and g1 destroyed.
#
#******************************************************************************
#
i$alloc_dis_resource:
        mov     0,r13
        mov     g7,r15                  # save g7

c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
        ld      ici_disQ(g4),r12        # r12 = current head of queue
        st      g1,ici_disQ(g4)         # save new head
        st      r12,oil1_dslink(g1)     # save link to old head
        st      r13,oil1_tmt(g1)        # clear tmt field
#
# --- setup default cr routine
#
        lda     i$alloc_dis_resource_cr,r3 # default cr handler
        st      r3,il_cr(g1)            # save completion return address

        lda     ILTBIAS(g1),r7          # r7 = lvl 2 of IRT nest
? # crash - cqt# 24312 - 2008-11-14 - FE TMT - get_xli() found freed (poisoned) TMT changed.
c       g7 = get_xli();                 # Allocate a XLI
.ifdef M4_DEBUG_XLI
c fprintf(stderr, "%s%s:%u get_xli 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g7);
.endif # M4_DEBUG_XLI
        st      g7,oil2_xli(r7)         # save pointer to xli

c       g0 = m_asglbuf(g0);             # allocate a SGL and buffer
        lda     xlicdb(g7),r4           # r4 = address of CDB area
        ldos    sg_scnt(g0),r6          # r6 = segment count
        lda     sghdrsiz+sg_addr(g0),r5 # r5 = address of 1st sg element
        ldconst DISCRETRY,r7            # r7 = retry counter
        stob    r6,xlisglnum_sav(g7)    # set number of SGL elements
        st      r5,xlisglpt(g7)         # set SGL segment addr
        st      r4,xlicdbptr(g7)        # set pointer to CDB
        stob    r7,oil1_retry(g1)       # save retry counter

        mov     r15,g7                  # restore g7

        ret                             # return to caller

i$alloc_dis_resource_cr:
        lda     ILTBIAS(g1),g1          # g1 = ILT at 2nd LVL
        call    i$rel_dis_resource      # release discovery resources
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: i$rel_dis_resource
#
#  PURPOSE:
#       Releases the resources require for the discovery process.
#
#  DESCRIPTION:
#       This routine releases the ILT,  CDB, and  SGL.
#
#  CALLING SEQUENCE:
#       call    i$rel_dis_resource
#
#  INPUT:
#       g1 = ILT at second level
#       g4 = ICIMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
i$rel_dis_resource:
        mov     0,r3
        movq    g0,r12                  # save g0-g2 in r12-r15
        mov     g7,r11                  # save g7
        lda     -ILTBIAS(g1),g1         # back up to ILT 1st level
#
# --- remove ILT from Discovery queue ...
#
        lda     ici_disQ(g4),r9         # r9 = address of queue head
        ld      ici_disQ(g4),r8         # r8 = current head of queue
        cmpobe.f 0,r8,.rdr_200          # Jif nothing is on the queue
        cmpobe.t r8,g1,.rdr_60          # Jif this is the entry

.rdr_40:
        lda     oil1_dslink(r8),r9      # r9 = address of current link pointer
        ld      oil1_dslink(r8),r8      # r8 = address of next ILT
        cmpobe.f 0,r8,.rdr_200          # Jif end of queue
        cmpobne.f r8,g1,.rdr_40         # Jif this is not the entry

.rdr_60:
        ld      oil1_dslink(r8),r10     # r10 =address of next ILT
        st      r10,(r9)                # save in previous ILT
        st      r3,oil1_dslink(r8)      # clear thread
#
# --- determine if there is an XLI and release it if there is
#
        lda     ILTBIAS(g1),r10         # r10 = ILT 2nd level
        ld      oil2_xli(r10),g7        # g7 = XLI address
        cmpobe.f 0,g7,.rdr_100          # Jif no XLI
        ld      xlisglpt(g7),g0         # g0 = sgl
        st      r3,xlisglpt(g7)         # clear sgl pointer
        st      r3,oil2_xli(r10)        # clear XLI pointer
.ifdef M4_DEBUG_XLI
c fprintf(stderr, "%s%s:%u put_xli 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g7);
.endif # M4_DEBUG_XLI
c       put_xli(g7);                    # Release xli back to pool.
#
# --- determine if there is a SGL and release it if there is
#
        cmpobe.f 0,g0,.rdr_100          # Jif no sgl
        lda     -sghdrsiz+sg_addr(g0),g0 # g0 = address start of header
        call    M$rsglbuf               # return sgl
#
# --- ...  release ILT.
#
.rdr_100:
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT

.rdr_200:
        mov     r11,g7                  # restore g7
        movq    r12,g0                  # restore g0-g2 from r12-r15
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: i$updateportdb
#
#  PURPOSE:
#       This routine acquired the port database from the ISP2100 modual.
#
#  CALLING SEQUENCE:
#       call    i$updateportdb
#
#  INPUT:
#       g1 = ILT at second level (oil2)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       No registers destroyed.
#
#******************************************************************************
#
i$updateportdb:
        PushRegs(r3)                    # Save all G registers

        lda     -ILTBIAS(g1),g1         # ILT at LVL 1

        ldob    oil1_chpid(g1),g0       # g0 = chip instance
        ldos    oil1_lid(g1),g1         # g1 = loop id

.if MULTI_ID
        ldconst  0,g2                   # VPID of adapter, idriver only works on the promary port
.endif  # MULTI_ID
        call    ISP_GetPortDB           # update database for this ID

        PopRegsVoid(r3)                 # Restore all G registers
        ret
#
#******************************************************************************
#
#  NAME: i$init_ici_lidtbl
#
#  PURPOSE:
#       initializes lid table
#
#  CALLING SEQUENCE:
#       process call
#
#  INPUT:
#       g4. ICIMT
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       All registers can be destroyed.
#
#******************************************************************************
#
i$init_ici_lidtbl:
        ldconst LIDtblsize,r4           # r4 = lid table size in words
        ldconst 0xffffffff,r5
.lidtb10:
        st      r5,ici_lidtbl(g4)[r4*4] # set entry
        subo    1,r4,r4                 # get previous word
        cmpobne 0,r4,.lidtb10           # continue

        ldob    ici_chpid(g4),r4        # get chip id
        ldconst LIDtblsize,r4           # r4 = lid table size in words
        ldconst 0xFFFF0000,r5           # reserve 0x7e0 - 0x7ff
        st      r5,ici_lidtbl(g4)[r4*4] # store data
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: i$get_lid
#
#  PURPOSE:
#       Provides a loop ID in the range of 80-7ff for use with
#       Fabric ports.
#
#  CALLING SEQUENCE:
#       call    i$get_lid
#
#  INPUT:
#       g4 = ICIMT
#
#  OUTPUT:
#       g0 = LID  -  g0 == 0, no LID was found.
#                    g0 <> 0, LID was found.
#
#******************************************************************************
#
        .globl  I_get_lid
I_get_lid:
        ld      I_CIMT_dir[g0*4],g4     # g4 = ICIMT dir pointer
i$get_lid:
        ldconst LIDtblsize,r4           # r4 = lid table size in words
        ldconst 0,g0                    # default g0 = no lid
        ldconst 0x1f,r8                 # r8 = xor value
.getlid_100:
        ld      ici_lidtbl(g4)[r4*4],r5 # r5 = word from table
        scanbit r5,r6                   # find a bit
        bo      .getlid_500             # Jif a bit has been found
        cmpobe.f 0,r4,.getlid_1000      # Jif not LID's available
        subo    1,r4,r4                 # get previous word
        b       .getlid_100             # continue
#
.getlid_500:
        clrbit  r6,r5,r5                # clear bit associated with LID
        st      r5,ici_lidtbl(g4)[r4*4] # save new value

        shlo    5,r4,r4                 # multiply by 32
        ldconst 0x80,r7                 # r7 = 0x80
        xor     r8,r6,r6                # invert bit position
        addo    r4,r6,g0                # g0 = LID
        addo    r7,g0,g0                # g0 = g0 + 0x80
.getlid_1000:
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: i$put_lid
#
#  PURPOSE:
#       Provides a loop ID in the range of 80-7ff for use with
#       Fabric ports.
#
#  CALLING SEQUENCE:
#       call    i$put_lid
#
#  INPUT:
#       g0 = LID - must be in the range of 0x80 - 0x7ff
#       g4 = ICIMT
#
#  OUTPUT:
#       None.
#
#******************************************************************************
# void I_put_lid(UINT8 port,UINT16 lid);
        .globl  I_put_lid
I_put_lid:
        ld      I_CIMT_dir[g0*4],g4     # g4 = ICIMT dir pointer
        mov     g1,g0                   # g0 = lid
i$put_lid:
        mov     g0,r15                  # save g0
        ldconst 0x80,r7
        cmpobg  r7,g0,.putlid_end       # Jif  0x80 > handle

        ldconst NO_LID,r7
        cmpobe  r7,g0,.putlid_end       # Jif  invalid handle

        subo    0x80,g0,g0              # g0 = lid - 0x80

        mov     g0,r8                   # make a copy
#
# --- find the word and bit position
#
        shro    5,g0,g0                 # divide by 32 to get word index
        ldconst 0x1f,r5                 # r5 = xor value
        and     r5,r8,r8                # get bit position
        xor     r5,r8,r8                # invert bit position

        ld      ici_lidtbl(g4)[g0*4],r4 # r4 = word in lid table
        setbit  r8,r4,r4                # set bit associated with lid
        st      r4,ici_lidtbl(g4)[g0*4] # save new value

.putlid_end:
        mov     r15,g0                  # restore g0
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: i$use_lid
#
#  PURPOSE:
#       Clear bit in LID table associated with a LID.
#
#  CALLING SEQUENCE:
#       call    i$use_lid
#
#  INPUT:
#       g0 = LID - must be in the range of 0x80 - 0x7ff
#       g4 = ICIMT
#
#  OUTPUT:
#       None
#
#  REGS DESTROYED:
#       g0
#
#******************************************************************************
#
i$use_lid:
#
# --- make sure the requested LID is a valid one
#
        ldconst NO_LID,r5               # check for invalid lid
        cmpobe  r5,g0,.uselid_1000      # Jif invalid lid

        ldconst 0x7ff,r5                # r5 = and value
        and     r5,g0,g0                # make sure it's only 11 bit
#
# --- determine if the requested LID is less than 0x80
#
        ldconst 0x80,r7                 # r7 = 0x80
        cmpobg.f r7,g0,.uselid_1000     # Jif LID less than 0x80
#
# --- the requested LID is greater or equal to 0x80, subtract 0x80 to form
#     index.
#
        subo    r7,g0,g0                # g0 = g0 - 0x80
        mov     g0,r8                   # make a copy
#
# --- find the word and bit position
#
        shro    5,g0,g0                 # divide by 32 to get word index
        ldconst 0x1f,r5                 # r5 = xor value
        and     r5,r8,r8                # get bit position
        xor     r5,r8,r8                # invert bit position

        ld      ici_lidtbl(g4)[g0*4],r4 # r4 = word in lid table
        clrbit  r8,r4,r4                # clear bit associated with lid
        st      r4,ici_lidtbl(g4)[g0*4] # save new value
.uselid_1000:
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: i$del_tmt
#
#  PURPOSE:
#       This deletes a TMT
#
#  DESCRIPTION:
#       Delete a TMT from the active TMT queue and frees all
#       resources associated with the TMT.
#
#  CALLING SEQUENCE:
#       call    i$del_tmt
#
#  INPUT:
#       g5 = TMT.
#
#  OUTPUT:
#       g0 = FALSE - TMT not deleted.
#          = TRUE - TMT deleted.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
i$del_tmt:
        movt    g4,r12                  # Save g4,g5,g6
#
.ifdef M4_ADDITION
# --- Do not touch TMT that is in process of being deleted.
#
c   if (((TMT *)g5)->state == tmstate_deltar) {     # Target should NOT be in process of being deleted
# c       fprintf(stderr, "%s%s:%u i$del_tmt TMT 0x%08lx already in state tmstate_deltar (%d) -- ignoring\n", FEBEMESSAGE,__FILE__, __LINE__, g5, ((TMT *)g5)->state);
        ldconst FALSE,g0                # Indicate TMT not deleted
        b       .idt100
c   }
#
.endif  # M4_ADDITION
# --- Get ICIMT for removeLun_no
#
                                        # g5 = TMT
        call    LLD$pre_target_gone     # check if OK to blow off target
        cmpobne FALSE,g0,.idt10         # Jif OK to blow off target

        ldconst tmstate_deltar,r3       # r3 = deltar state
        stob    r3,tm_state(g5)         # set TMT inactive
#
# --- Clear WWN so TMT is not reused.
#
        movl    0,r6                    # zeros
        stl     r6,tm_N_name(g5)        # Clear node name of TMT
        stl     r6,tm_P_name(g5)        # Clear port name of TMT
        ldconst FALSE,g0                # Indicate TMT not deleted
        b       .idt100
#
# --- remove LUN
#
.idt10:
        ldconst MAXLUN,r3               # r13 = maxlun
        ldconst 0,r7                    # Start with LUN 0
        ld      tm_icimt(g5),g4         # g4 = icimt
.idt20:
        ld      tm_tlmtdir(g5)[r7*4],g6 # g6 = possible tlmt
        cmpobe.t 0,g6,.idt30            # jif no tlmt defined

        call    i$removeLUN_no          # remove this LUN

.idt30:
        addo    1,r7,r7                 # bump index
        cmpobg.t r3,r7,.idt20           # Jif no end of directory
#
# --- unlink tmt from queue
#
        ld      tm_icimt(g5),r3         # r3 = ICIMT
        cmpobe  0,r3,.idt60             # Jif no ICIMT
        ld      ici_tmtQ(r3),r5         # r5 = First TNT in tmt queue
        cmpobe  0,r5,.idt60             # Jif linkage broken
        cmpobne g5,r5,.idt40            # Jif TMT doesn't match
        ld      tm_link(g5),r6          # r6 = link to next tmt
        st      r6,ici_tmtQ(r3)         # save link in head of tmt queue
        b       .idt60

.idt40:
        ld      tm_link(r5),r6          # r6 = link to next tmt
        cmpobne g5,r6,.idt50            # Jif TMT doesn't match

        ld      tm_link(g5),r6          # r6 = link to next tmt
        st      r6,tm_link(r5)          # save link in previous tmt
        b       .idt60
#
# --- Advance to next TMT in queue
#
.idt50:
        mov     r6,r5                   # r5 = next tmt in queue
        cmpobe  0,r5,.idt60             # Jif linkage broken
        b       .idt40
#
# --- Notify LLD that the target is gone.
#
.idt60:
                                        # g5 = TMT
        call    LLD$target_GONE         # indicate target is gone
#
# --- pass lid/index and port, tmt address
#
        PushRegs(r3)
        ldob    tm_chipID(g5),g0        # r5 = chip ID
        ldos    tm_lid(g5),g1           # g0 = LID of port
        ldconst 0,g2                    # TMT as NULL to deleted
        call    fsl_updateTMT
        PopRegsVoid(r3)
#
# --- Free the LID and then delete the inactive TMT
#
        ldos    tm_lid(g5),g0           # g0 = LID of port

        ld      tm_icimt(g5),g4         # g4 = ICIMT
        ldconst NO_LID,r3               # Set up to invalidate LID in TMT
        call    i$put_lid               # Return lid
        stos    r3,tm_lid(g5)           # Set LID to invalid in the TMT
                                        # g5 = TMT
.ifdef M4_DEBUG_TMT
c fprintf(stderr, "%s%s:%u put_tmt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g5);
.endif # M4_DEBUG_TMT
c       put_tmt(g5);                    # release tmt to free pool
        ldconst TRUE,g0                 # Indicate TMT deleted
.idt100:
        movt    r12,g4                  # Restore g4,g5,g6
        ret
#
#******************************************************************************
#
#  NAME: i$qtlmt
#
#  PURPOSE:
#       Queues an TLMT to the ICIMT active device queue.
#
#  DESCRIPTION:
#       Place specified TLMT onto the end of icimt active queue (sets tail).
#
#  CALLING SEQUENCE:
#       call    i$qtlm
#
#  INPUT:
#       g4 = address of icimt
#       g6 = tlmt address to add
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       Reg. g7 destroyed.
#
#******************************************************************************
#
# void I_queTLMT(g4=ICIMT *, g6=TLMT *);
    .globl  I_queTLMT
I_queTLMT:
i$qtlmt:
        lda     ici_actqhd(g4),r3       # get address of active queue
        ldl     (r3),r14                # r14 = list head, r15 = list tail
        cmpobne.t 0,r14,.qtlm_100       # Jif queue not empty
#
# --- Case: Queue was empty.
#
        mov     r3,r15                  # set base of queue as backward thread
        stl     r14,tlm_flink(g6)       # save forward/backward threads in TLMT
        mov     g6,g7                   # set tail and head as same
        stl     g6,(r3)                 # save TLMT as head & tail pointer
        b       .qtlm_1000              # and we're out of here!
#
# --- Case: Queue was NOT empty. Place on end of queue.
#
.qtlm_100:
        mov     0,r13
        st      g6,tlm_flink(r15)       # link new TLMT onto end of list
        st      g6,ici_actqtl(g4)       # save new TLMT as new tail
        st      r13,tlm_flink(g6)       # clear forward thread in new TLMT
        st      r15,tlm_blink(g6)       # save backward thread in new TLMT
.qtlm_1000:
        ret
#
#******************************************************************************
#
#  NAME: i$removeLUN
#
#  PURPOSE:
#       Performs all processes required to remove a LUN from a target. This
#       routine has two entry points.
#
#       i$removeLUN  sends indication of the session termination
#       i$removeLUN_no  does not send indication of session termination
#
#  CALLING SEQUENCE:
#       call    i$removeLUN
#       call    i$removeLUN_no
#
#  INPUT:
#       g4 = ICIMT address
#       g5 = TMT address
#       g6 = TLMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#      None
#
#******************************************************************************
#
# void I_removeLUN(g6=TLMT *, g5=TMT *, g4=ICIMT *);
        .globl  I_removeLUN
I_removeLUN:
        ld      tlm_Shead(g6),g7        # get possible session queue head
        cmpobe.t 0,g7,.irLUN_200         # Jif no session

        ld      ism_flink(g7),g8        # get link to next ismt
        st      g8,tlm_Shead(g6)        # save as new head

        ld      ism_str(g7),g8          # g8 = session termination routine
        cmpobe.f 0,g8,.irLUN_100         # Jif STR is zero

        ld      ism_ReqID(g7),g1        # g1 = requestor ID

        PushRegs(r3)                    # Save all G registers
        callx   (g8)                    # call irp completion routine
                                        # input: g1 = requestor ID
                                        # output: none.
        PopRegsVoid(r3)                 # Restore all G registers
#
.irLUN_100:
.ifdef M4_DEBUG_ISMT
c fprintf(stderr, "%s%s:%u put_ismt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g7);
.endif # M4_DEBUG_ISMT
c       put_ismt(g7);                   # return ismt to free pool.
        b       I_removeLUN             # continue
#
# --- unlink tlmt from active lun queue and release it to free tlmt
#     pool. ALso clear pointer in the TMT's tlmt directory.
#
.irLUN_200:
        mov     0,r13
        ldos    tlm_lun(g6),g2          # g7 = lun #
        st      r13,tm_tlmtdir(g5)[g2*4] # clear tlmt from tmt lun directory

                                        # g6 = TLMT to remove
                                        # g4 = icimt
        call    i$rtlmt                 # remove tlmt from queue

.ifdef M4_DEBUG_TLMT
c fprintf(stderr, "%s%s:%u put_tlmt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g6);
.endif # M4_DEBUG_TLMT
c       put_tlmt(g6);                   # Put tlmt onto free list.
        ret
#
i$removeLUN_no:
        PushRegs(r3)                    # Save all G registers
        ldconst FALSE,g0                # g0 = no session termination indication
        call    .removeLUN_common
        PopRegsVoid(r3)                 # Restore all G registers
        ret
#
i$removeLUN:
        PushRegs(r3)                    # Save all G registers
        ldconst TRUE,g0                 # r3 = session termination indication
        call    .removeLUN_common
        PopRegsVoid(r3)                 # Restore all G registers
        ret
#
# --- terminate all the task that may be queued to this LUN
#
.removeLUN_common:
        ldconst cmplt_IOE,g8            # g8 = IOE completion status
        ldconst ioe_timeout,g9          # g9 = timeout error
#
.rLUN_100:
?# Crash 2012-01-13 with TLMT freed (caffcaff) on below load.
        ld      tlm_whead(g6),g1        # g1 = ILT of task at 1st lvl
        cmpobne.t 0,g1,.rLUN_110        # Jif there is another task on queue
#
# --- check abort task list
#
        ld      tlm_ahead(g6),g1        # g1 = ILT of task at 1st lvl
        cmpobe.f 0,g1,.rLUN_200         # Jif no tasks active
#
.rLUN_110:
#
# --- trace the timeout
#
.if     ITRACES
        call    it$trc_tsk_TO           # *** trace task timeout event ***
.endif  # ITRACES
#
# --- remove task from working queue
#
        call    apl$remtask             # remove task from queue
        ld      oil2_irp(g1),g2         # g2 = irp address
#
# --- Default the completion status
#
        stob    g8,irp_cmplt(g2)        # save completion status
        st      g9,irp_extcmp(g2)       # save extended status info
        ldconst 0,r15
        stob    r15,oil2_rtycnt(g1)     # clear the retry count
#
# --- Process the event
#
        ldconst Te_timeout,g10          # r10 = event
        ld      oil2_ehand(g1),g11      # r11 = ICIMT event handler address
        addo    g10,g11,g10             # add completion event type
        ld      (g10),g10

        PushRegs(r3)                    # Save all G registers
        callx   (g10)                   # process event
        PopRegsVoid(r3)                 # Restore all G registers
        b       .rLUN_100               # continue
#
# --- all the tasks have been terminated, now terminate the sessions
#
.rLUN_200:
        ld      tlm_Shead(g6),g7        # get possible session queue head
        cmpobe.t 0,g7,.rLUN_260         # Jif no session

        ld      ism_flink(g7),g8        # get link to next ismt
        st      g8,tlm_Shead(g6)        # save as new head

        cmpobe.f FALSE,g0,.rLUN_210     # Jif no session termination indication

        ld      ism_str(g7),g8          # g8 = session termination routine
        cmpobe.f 0,g8,.rLUN_210         # Jif STR is zero

        ld      ism_ReqID(g7),g1        # g1 = requestor ID

        PushRegs(r3)                    # Save all G registers
        callx   (g8)                    # call irp completion routine
                                        # input: g1 = requestor ID
                                        # output: none.
        PopRegsVoid(r3)                 # Restore all G registers
#
.rLUN_210:
.ifdef M4_DEBUG_ISMT
c fprintf(stderr, "%s%s:%u put_ismt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g7);
.endif # M4_DEBUG_ISMT
c       put_ismt(g7);                   # return ismt to free pool.
        b       .rLUN_200               # continue
#
# --- unlink tlmt from active lun queue and release it to free tlmt
#     pool. ALso clear pointer in the TMT's tlmt directory.
#
.rLUN_260:
        mov     0,r13
        ldos    tlm_lun(g6),g2          # g7 = lun #
        st      r13,tm_tlmtdir(g5)[g2*4] # clear tlmt from tmt lun directory

                                        # g6 = TLMT to remove
                                        # g4 = icimt
        call    i$rtlmt                 # remove tlmt from queue

.ifdef M4_DEBUG_TLMT
c fprintf(stderr, "%s%s:%u put_tlmt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g6);
.endif # M4_DEBUG_TLMT
c       put_tlmt(g6);                   # Put tlmt onto free list.
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: i$rtlmt
#
#  PURPOSE:
#       Removes the specified TLMT from a cimit queue.
#
#  DESCRIPTION:
#       Removes the specified TLMT from a cimit queue, setting forward and
#       backwards links as appropriate.
#
#  CALLING SEQUENCE:
#       call    i$rtlmt
#
#  INPUT:
#       g4 = icimt
#       g6 = assoc. TLMT address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
i$rtlmt:
        ldl     tlm_flink(g6),r14       # r14 = forward link of TLMT
                                        # r15 = backward link of TLMT
        st      r14,tlm_flink(r15)      # put forward thread from removed TLMT
                                        #  as forward thread of previous TLMT
        cmpobne.t 0,r14,.rtlmt_700      # Jif non-zero forward thread
        lda     ici_actqhd(g4),r14      # r14 = base address of active queue
        cmpobne.t r14,r15,.rtlmt_700    # Jif backward thread <> base of queue
        mov     0,r15                   # queue is now empty!
.rtlmt_700:
        st      r15,tlm_blink(r14)      # put backward thread from removed
                                        #  TLMT as backward thread of previous
        ret
#
#******************************************************************************
#
#  NAME: i$next_lun
#
#  PURPOSE:
#      Send an test unit ready command to the next LUN.
#
#  DESCRIPTION:
#       This routine sends a TUR command to the next LUN. If
#       the next LUN is greater than the maximum LUN (MAXLUN),
#       the i$nextlid is called,
#
#       The following address in the ICIMT are used to define
#       the device:
#
#       oil1_chpid = chip instance of the interface
#       oil1_lid   = LID (loop ID) of the device
#       oil1_lun   = LUN of the device
#
#  CALLING SEQUENCE:
#       call    i$next_lun
#
#  INPUT:
#       g1 = ILT at Initiator level 2.
#       g4 = ICIMT
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
i$next_lun:
        movl    g0,r8                   # save g0-g3
        lda     -ILTBIAS(g1),r15        # r15 = 1st level of ILT

        ldob    oil1_flag(r15),r4       # r4 = flag byte
        ldob    oil1_lun(r15),r6        # r6 = LUN for target

        bbs     oiflg_ABRT,r4,.nlun_200 # Jif abort
        bbs     oiflg_PDBC,r4,.nlun_200 # Jif data base has changed
        bbs     oiflg_LUN,r4,.nlun_100  # Jif only this LUN

        ld      oil1_tmt(r15),r5       # r5 = possible TMT address
        cmpobe  0,r5,.nlun_200         # Jif no TMT

        PushRegs(r3)
        ldl     tm_P_name(r5),g0       # g0, g1 Portname
        call    ISP_ChkIfPeerTarget
        mov  g0,r7
        PopRegsVoid(r3)
        cmpobne 0,r7, .nlun_100

        ldconst MAXLUN,r3               # r3 = MAXLUN
        addo    1,r6,r6                 # bump to next LUN.
c if (r6 > 0xff) fprintf(stderr, "i$nex_lun: oil1_lun will not fit value 0x%lx\n", r6);
        stob    r6,oil1_lun(r15)        # save new LUN number
        cmpobe.t r3,r6,.nlun_100        # Jif lun not in range

        lda     I$recv_cmplt_rsp,g2     # g2 = completion routine
        lda     .Ts_turln,g3            # g3 = "TUR lun 0" event table
        call    i$send_tur              # send the inquiry command
        b       .nlun_1000              # br
#
# --- determine if a LUN discovery was requested. If so, determine if the clearing
#     of the LUN discovery request has been suppressed. If the suppress bit is set,
#     don't clear the LUN discovery request bit.
#
.nlun_100:
        ld      oil1_tmt(r15),r5        # r5 = possible TMT address
        cmpobe  0,r5,.nlun_200          # Jif no TMT

        ldob    tm_flag(r5),r4          # r4 = flag byte
        chkbit  tmflg_SLDC,r4           # Check if Suppress target ID requested ???
        clrbit  tmflg_SLDC,r4,r4        # clear suppress target ID request
        be      .nlun_150               # JIf Suppress LUN discovery clear bit is set

        clrbit  tmflg_lundisc,r4,r4     # clear LUN discovery in process flag

.nlun_150:
        stob    r4,tm_flag(r5)          # save flag byte
#
# --- discover next LID
#
.nlun_200:
        call    i$next_lid              # check the next LID

.nlun_1000:
        movl    r8,g0                   # restore g0-g3
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: i$next_lid
#
#  PURPOSE:
#      Send an test unit ready command to the next LUN.
#
#  DESCRIPTION:
#       This routine sends a TUR command to the next LUN. If
#       the next LUN is greater than the maximum LUN (MAXLUN),
#       the i$nextlid is called,
#
#       The following address in the ICIMT are used to define
#       the device:
#
#       oil1_chpid = chip instance of the interface
#       oil1_lid   = LID (loop ID) of the device
#       oil1_lun   = LUN of the device
#
#  CALLING SEQUENCE:
#       call    i$next_lid
#
#  INPUT:
#       g1 = ILT at Initiator level 2.
#       g4 = ICIMT
#
#  OUTPUT:
#       g0 = status
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
i$next_lid:
        lda     -ILTBIAS(g1),r15        # back up to originator's IRT nest
#
# --- If the port is offline, do nothing
#
        ldob    ici_state(g4),r4        # r4 = loop state
        cmpobe.f Cs_offline,r4,.nlid_200 # JIf loop offline

        ldob    oil1_flag(r15),r3       # r3 = flag byte
#       bbs     oiflg_LUN,r3,.nlid_200  # Jif only this LUN
        bbs     oiflg_PDBC,r3,.nlid_200 # Jif data base has changed
        bbs     oiflg_ABRT,r3,.nlid_200 # Jif data base has changed

.if     MAG2MAG
        movq    g0,r4                   # save environment
        movq    g4,r8
#
# --- Determine if suppress target ID is requested
#
        chkbit  oiflg_STID,r3           # Check if Suppress target ID requested ???
        clrbit  oiflg_STID,r3,r3        # clear suppress target ID request
        stob    r3,oil1_flag(r15)       # save new value
        be      .nlid_010               # JIf Suppress target ID requested.
#
# --- Determine if a target was discovered during this operation. If
#     one was, send a Target_Found to LLD.

        ld      oil1_tmt(r15),g5        # g5 = possible TMT address
        cmpobe  0,g5,.nlid_010          # Jif no TMT

#
# --- pass lid/index and port, tmt address
#
        PushRegs(r13)
        ldob    ici_chpid(g4),g0        # g0 = chip instance
        ldos    oil1_lid(r15),g1        # g1 = loop ID for target
        mov     g5, g2                  # TMT address
        call    fsl_updateTMT
        PopRegsVoid(r13)

#
# --- inform link manager of target
#
        ldob    ici_chpid(g4),g0        # g0 = chip instance

.if     ITRACES
        call    it$trc_target_id        # trace discovery complete
.endif  # ITRACES

        ldob    ici_chpid(g4),g0        # g0 = chip instance
        call    LLD$target_ID           # id target to link manager
                                        #  input:
                                        #    g0 = chip instance
                                        #    g5 = tmt
                                        #  output:
                                        #    none
.nlid_010:
        movq    r4,g0                   # restore environment
        movq    r8,g4
.endif  # MAG2MAG

        bbs     oiflg_LID,r3,.nlid_200  # Jif only this LID
        ldconst 0,r11
        st      r11,oil1_tmt(r15)       # clear TMT address
#
# --- last LUN on this LID. Determine if there is another LID to
#     check.
#
.nlid_100:
        ld      ici_lpmapptr(g4),r6     # r6 = address of ALPA map
        ldob    oil1_lpmapidx(r15),r7   # r7 = loop map index
        cmpobne 0,r6,.nlid_110          # Jif my LID

        ldob    ici_chpid(g4),g0        # g0 = chip instance
        b        .nlid_200
#
.nlid_110:
        addo    1,r7,r7                 # bump ALPA index
        addo    r6,r7,r5                # r5 = address of next ALPA
        ldconst 0xff,r3                 # r3 = invalid ALPA
        ldob    (r5),r4                 # r4 = ALPA
        stob    r7,oil1_lpmapidx(r15)   # save ALPA map index
        cmpobe.t r3,r4,.nlid_200        # Jif not another ALPA in map
#
# --- there is another LID, so save it off in the CMNT and reset LUN to 0
#
c if (r11 > 0xff) fprintf(stderr, ".nlid_110: oil1_lun will not fit value 0x%lx\n", r11);
        stob    r11,oil1_lun(r15)       # save new LUN number (LUN 0)
        ldob    ici_chpid(g4),r12       # r12 = chip instance

        mov     g1,r3                   # Save g1
        mov     r12,g0                  # g0 = chip instance
        mov     r4,g1                   # g1 = loop id
        call    ISP$is_my_ALPA          # check for my ALPA
        mov     r3,g1                   # restore g1
        cmpobne 0,g0,.nlid_100          # Jif my ALPA

        PushRegs(r3)
        ldob    ici_chpid(g4),g0        # r12 = chip instance
        mov     r4,g1                   # store alpa
        ldconst 0,g2                    # set vpid to 0
        call    isp_alpa2handle
        mov     g0,r4                   # save return code
        PopRegsVoid(r3)                 # restore gregs

        stos    r4,oil1_lid(r15)        # save new LID
#
# --- update port database for this LID as required
#     this should make us log into this deivece with this lid.
#
        call    i$updateportdb          # update port database
#
# --- Determine if foreign targets are enabled. If they are not, check data
#     base world wide name to make sure the target is a MAG before starting
#     a discovery process.
#
        ldconst PORTDBSIZ,r13           # r13 = size of Port Database
.if ICL_DEBUG
        cmpobne ICL_PORT,r12,.nlid_icl01
c fprintf(stderr,"%s%s:%u <next_lid(idriver)>WARNING...WARNING....WARNING...\n", FEBEMESSAGE, __FILE__, __LINE__);
.nlid_icl01:
.endif  # ICL_DEBUG
        ld      portdb[r12*4],r14       # r14 = base of port database allocation
        mulo    r13,r4,r13              # r13 = offset = LID * PDB size
        addo    r13,r14,r14             # r14 =  Port DB address
        movl    g6,r8                   # save g6-g7 in r8-r9

        lda     pdbpdn(r14),g0          # g0 = ptr to port WWN name
        call    ISP$is_my_WWN           # check for my port WWN name
        cmpobe.f TRUE,g0,.nlid_100      # Jif my port WWN name

        ldob    ici_ftenable(g4),r3     # r3 = foreign target enable flag
        cmpobne.f 0,r3,.nlid_150        # JIf foreign target enabled

        ldl     pdbpdn(r14),g6          # g6-g7 = port name
c       r10 = M_chk4XIO(*(UINT64*)&g6); # is this a XIOtech Controller ???
        movl    r8,g6                   # restore g6-g7 from r8-r9
        cmpobe  0,r10,.nlid_100         # no - try next lid
#
# --- Check if this is a XIO BE port
#
        ldl     pdbpdn(r14),r8          # r8 = MSW port name
        extract 12,4,r8                 # Get 'e' nibble from WWN
        ldconst (WWNBPort>>20)&0xF,r3   # Value for BE port
        cmpobe  r8,r3,.nlid_100         # Jif a XIOtech BE port
#
# --- determine if this alpa/lid is an initiator
#
        ldob    pdbprliw3(r14),r3       # get prli ser parm w3
        bbc     5,r3,.nlid_100          # Jif no initiator present
#
# --- determine if this alpa/lid is a target
.nlid_150:
        ldob    pdbprliw3(r14),r3       # get prli ser parm w3
        bbc     4,r3,.nlid_100          # JIf no target present
#
# --- send Test Unit Ready to next device
#
        movq    g0,r4                   # save environment
        lda     I$recv_cmplt_rsp,g2     # g2 = completion routine
        lda     .Ts_turl0,g3            # g3 = "TUR lun 0" event table
        call    i$send_tur              # send the inquiry command
        movq    r4,g0                   # restore environment
        b       .nlid_1000              # exit
#
# --- all devices have been located. PLace the initiator in online state,
#     return the ILT and CDB then exit.
#
.nlid_200:
        movq    g0,r4                   # save environment
        movq    g4,r8
        movq    g8,r12
        ldconst cmplt_success,g0        # good completion status
        lda     -ILTBIAS(g1),g1         # back up to originator's IRT nest
        ld      il_cr(g1),r3            # get completion routine address
        callx   (r3)                    # process end of scan stuff
        movq    r4,g0                   # restore environment
        movq    r8,g4
        movq    r12,g8

.nlid_1000:
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: i$reissue_task
#
#  PURPOSE:
#      This routine is used to resend the current task in the discovery
#      ILT.
#
#  DESCRIPTION:
#       This routine is used to reissue the current task in the discovery
#       ILT. It assumes that the current discovery ILT has been constructed
#       correctly.
#
#  CALLING SEQUENCE:
#       call    i$reissue_task
#
#  INPUT:
#       g1 = ILT at 2nd lvl
#       g2 = completion routine address
#       g3 = event handler table address
#       g4 = ICIMT
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
i$reissue_task:
        mov     g7,r13                  # save g7
        ld      oil2_xli(g1),g7         # g7 - xli pointer
.if     ITRACES
        call    it$trc_dtsk_reissue     #    *** trace ***
.endif  # ITRACES
        lda     ILTBIAS(g1),g1          # advance to next nest level of ILT
        call    ISP$initiate_io         # Send command to target
        mov     r13,g7                  # restore g7
        ret
#
#******************************************************************************
#
#  NAME: i$send_inq
#
#  PURPOSE:
#      Build up and send an inquiry command to a device.
#
#  DESCRIPTION:
#       The following address in the ICIMT are used to define
#       the device:
#
#       oil1_chpid = chip instance of the interface
#       oil1_lid   = LID (loop ID) of the device
#       oil1_lun   = LUN of the device
#
#  CALLING SEQUENCE:
#       call    i$send_inq
#
#  INPUT:
#       g1 = ILT at 2nd lvl
#       g2 = completion routine address
#       g3 = event handler table address
#       g4 = ICIMT
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
i$send_inq:
#c fprintf(stderr,"idriver: i$send_inq: ICIMT %08lX\n",g4);
        mov     g7,r13                  # save g7

        st      g2,oil2_cr(g1)          # Save completion routine
        st      g3,oil2_ehand(g1)       # save event handler

        lda     -ILTBIAS(g1),r15        # r15 = 1st level of ILT
        ld      oil2_xli(g1),g7         # g7 - xli pointer

        ldos    oil1_lid(r15),r3        # r3 = loop ID for target
        ldob    oil1_lun(r15),r4        # r4 = LUN for target
        ldob    oil1_chpid(r15),r5      # r5 = chip instance

        ldconst 5,r6                    # r6 = 5 second T/O value
        st      r3,xlitarget(g7)        # store target
        stos    r4,xlilun(g7)           # store LUN
        stob    r5,xlichipi(g7)         # set chip ID
        stos    r6,xlitime(g7)          # set timeout
#
        setbit  xlisimq,0,r3            # set Simple queue
        ldconst dtiread,r4              # set read transfer
        ldob    xlisglnum_sav(g7),r5    # get number of SGL elements
#
        stos    r3,xlifcflgs(g7)        # store FCAL flags
        stob    r4,xlidatadir(g7)       # store data direction
        stob    r5,xlisglnum(g7)        # set number of SGL elements
#
        ld      xlicdbptr(g7),r7        # r7 = address of CDB
        ldq     .inquiry_cdb,r8         # r8-r11 = cdb data
#
        stq     r8,(r7)                 # save in cdb

.if     ITRACES
        call    it$trc_inqlun           # *** trace ***
.endif  # ITRACES

        lda     ILTBIAS(g1),g1          # advance to next nest level of ILT
        st      g7,oil3_ptr(g1)         # Save pointer to XLI struct
#
        call    ISP$initiate_io         # Send command to target

        mov     r13,g7                  # restore g7
        ret
#
#******************************************************************************
#
#  NAME: i$send_tur
#
#  PURPOSE:
#      Build up and send a Test Unit Ready command to a device.
#
#  DESCRIPTION:
#       The following address in the ICIMT are used to define
#       the device:
#
#       oil1_chpid = chip instance of the interface
#       oil1_lid   = LID (loop ID) of the device
#       oil1_lun   = LUN of the device
#
#  CALLING SEQUENCE:
#       call    i$send_tur
#
#  INPUT:
#       g1 = ILT at 2nd lvl
#       g2 = completion routine address
#       g3 = event handler table address
#       g4 = ICIMT
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
i$send_tur:
#c fprintf(stderr,"idriver: i$send_tur: ICIMT %08lX\n",g4);
        mov     0,r13
        mov     g7,r14                  # save g7

        st      g2,oil2_cr(g1)          # Save completion routine
        st      g3,oil2_ehand(g1)       # save event handler

        lda     -ILTBIAS(g1),r15        # r15 = 1st level of ILT
        ld      oil2_xli(g1),g7         # g7 - xli pointer

        ldos    oil1_lid(r15),r3        # r3 = loop ID for target
        ldob    oil1_lun(r15),r4        # r4 = LUN for target
        ldob    oil1_chpid(r15),r5      # r5 = chip instance

        ldconst 5,r6                    # r6 = 5 second T/O value
        st      r3,xlitarget(g7)        # store target
        stos    r4,xlilun(g7)           # store LUN
        stob    r5,xlichipi(g7)         # set chip ID
        stos    r6,xlitime(g7)          # set timeout
#
        setbit  xlisimq,0,r3            # set Simple queue
        ldconst dtinone,r4              # set no data transfer

        stos    r3,xlifcflgs(g7)        # store FCAL flags
        stob    r4,xlidatadir(g7)       # store data direction
        stob    r13,xlisglnum(g7)       # set no SGL elements
#
        ld      xlicdbptr(g7),r7        # r7 = address of CDB
        ldq     .TUR_cdb,r8             # r8-r11 = cdb data
#
        stq     r8,(r7)                 # save in cdb

.if     ITRACES
        call    it$trc_turlun           # *** trace ***
.endif  # ITRACES

        lda     ILTBIAS(g1),g1          # advance to next nest level of ILT
        st      g7,oil3_ptr(g1)         # Save pointer to XLI struct

        call    ISP$initiate_io         # Send command to target

        mov     r14,g7                  # restore g7
        ret
#
#******************************************************************************
#
#  NAME: i$send_ssu
#
#  PURPOSE:
#      Build up and send a Start Stop Unit command to a device.
#
#  DESCRIPTION:
#       The following address in the ICIMT are used to define
#       the device:
#
#       oil1_chpid = chip instance of the interface
#       oil1_lid   = LID (loop ID) of the device
#       oil1_lun   = LUN of the device
#
#  CALLING SEQUENCE:
#       call    i$send_ssu
#
#  INPUT:
#       g1 = ILT at 2nd lvl
#       g2 = completion routine address
#       g3 = event handler table address
#       g4 = ICIMT
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
i$send_ssu:
        mov     0,r9
        mov     g7,r13                  # save g7

        st      g2,oil2_cr(g1)          # Save completion routine
        st      g3,oil2_ehand(g1)       # save event handler

        lda     -ILTBIAS(g1),r15        # r15 = 1st level of ILT
        ld      oil2_xli(g1),g7         # g7 - xli pointer

        ldos    oil1_lid(r15),r3        # r3 = loop ID for target
        ldob    oil1_lun(r15),r4        # r4 = LUN for target
        ldob    oil1_chpid(r15),r5      # r5 = chip instance

        ldconst 60,r6                   # r6 = 1 minute
        st      r3,xlitarget(g7)        # store target
        stos    r4,xlilun(g7)           # store LUN
        stob    r5,xlichipi(g7)         # set chip ID
        stos    r6,xlitime(g7)          # set timeout
#
        setbit  xlisimq,0,r3            # set Simple queue
        ldconst dtinone,r4              # set no data transfer

        stos    r3,xlifcflgs(g7)        # store FCAL flags
        stob    r4,xlidatadir(g7)       # store data direction
        stob    r9,xlisglnum(g7)        # set no SGL elements
#
        ld      xlicdbptr(g7),r7        # g7 = address of CDB
        ldq     .SSU_cdb,r8             # r8-r11 = cdb data
#
        stq     r8,(r7)                 # save in cdb

.if     ITRACES
        call    it$trc_ssulun           # *** trace ***
.endif  # ITRACES

        lda     ILTBIAS(g1),g1          # advance to next nest level of ILT
        st      g7,oil3_ptr(g1)         # Save pointer to XLI struct

        call    ISP$initiate_io         # Send command to target

        mov     r13,g7                  # restore g7
        ret
#
#******************************************************************************
#
#  NAME: i$send_rc
#
#  PURPOSE:
#      Build up and send a Read Capacity command to a device.
#
#  DESCRIPTION:
#       The following address in the ICIMT are used to define
#       the device:
#
#       oil1_chpid = chip instance of the interface
#       oil1_lid   = LID (loop ID) of the device
#       oil1_lun   = LUN of the device
#
#  CALLING SEQUENCE:
#       call    i$send_ms
#
#  INPUT:
#       g1 = ILT at 2nd lvl
#       g2 = completion routine address
#       g3 = event handler table address
#       g4 = ICIMT
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
i$send_rc:
        mov     g7,r13                  # save g7

        st      g2,oil2_cr(g1)          # Save completion routine
        st      g3,oil2_ehand(g1)       # save event handler

        lda     -ILTBIAS(g1),r15        # r15 = 1st level of ILT
        ld      oil2_xli(g1),g7         # g7 - xli pointer

        ldos    oil1_lid(r15),r3    # r3 = loop ID for target
        ldob    oil1_lun(r15),r4        # r4 = LUN for target
        ldob    oil1_chpid(r15),r5  # r5 = chip instance


        ldconst 5,r6                    # r6 = 5 second T/O value
        st      r3,xlitarget(g7)        # store target
        stos    r4,xlilun(g7)           # store LUN
        stob    r5,xlichipi(g7)         # set chip ID
        stos    r6,xlitime(g7)          # set timeout
#
        setbit  xlisimq,0,r3            # set Simple queue
        ldconst dtiread,r4              # set read transfer
        ldob    xlisglnum_sav(g7),r5    # get number of SGL elements

#
        stos    r3,xlifcflgs(g7)        # store FCAL flags
        stob    r4,xlidatadir(g7)       # store data direction
        stob    r5,xlisglnum(g7)        # set number of SGL elements
##
        ld      xlicdbptr(g7),r7        # g7 = address of CDB
        ldq     .readcapacity_cdb,r8    # r8-r11 = cdb data
#
        stq     r8,(r7)                 # save in cdb

# .if     ITRACES
#        call    it$trc_mslun            # *** trace ***
# .endif # ITRACES

        lda     ILTBIAS(g1),g1          # advance to next nest level of ILT
        st      g7,oil3_ptr(g1)         # Save pointer to XLI struct

        call    ISP$initiate_io         # Send command to target

        mov     r13,g7                  # restore g7
        ret
#
#       Canned Read Capacity command
#
        .data
.readcapacity_cdb:
        .byte   0x25                    # cmd - read capacity
        .byte   0x00                    # DBD
        .byte   0X00                    # PC/page
        .byte   0x00                    # reserved
        .byte   0x00                    # length - header +
        .byte   0x00                    # control
        .space  10,0                    # Remainder of CDB is zero
#
#       Canned Mode Sense command
#
.modesense_cdb:
        .byte   0x1a                    # cmd - sense mode
        .byte   0x00                    # DBD
        .byte   0X00                    # PC/page
        .byte   0x00                    # reserved
        .byte   8+8                     # length - header +
        .byte   0x00                    # control
        .space  10,0                    # Remainder of CDB is zero
#
#       Canned Inquiry command
#
.inquiry_cdb:
        .byte   0x12                    # cmd - inquiry
        .byte   0x00                    # CmdDt/EVPD -
        .byte   0X00                    # page - 0
        .byte   0x00                    # reserved
        .byte   36+12                   # length - standard + Mag S/N
        .byte   0x00                    # control
        .space  10,0                    # Remainder of CDB is zero
#
#       Canned Start Stop Unit Command
#
.SSU_cdb:
        .byte   0x1b                    # cmd - SSU
        .byte   0x00                    # reserved
        .byte   0X00                    # reserved
        .byte   0x00                    # reserved
        .byte   0x01                    # control (start unit)
        .space  11,0                    # Remainder of CDB is zero
#
#       Canned Test Unit Ready Command
#
.TUR_cdb:
        .byte   0x00                    # cmd - TUR
        .byte   0x00                    # LBA (MSB)
        .byte   0X00
        .byte   0x00                    #     (LSB)
        .byte   0x00                    # transfer length
        .byte   0x00                    # control
        .space  10,0                    # Remainder of CDB is zero
#
        .text
#
.endif  # INITIATOR
#
#******************************************************************************
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
