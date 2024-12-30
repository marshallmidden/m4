# $Id: itrace.inc 145407 2010-08-10 17:26:37Z m4 $
#*****************************************************************************
#
#  NAME: itrace.inc  (Initiator race data structures & definitions)
#
#  DESCRIPTION:
#
#       This module defines the data structures and definitions
#       to support traces in the IDRIVER & APLDRV modules.
#
#  Copyright (c) 1998-2001 XIOtech Corporarion.   All rights reserved.
#
#*****************************************************************************
#
# --- Equates ----------------------------------------------------------------
#
#
# *** Note: These conditional assembly options are now enabled in options.inc
#
#        .set    ITRACES,TRUE           # enable traces
#        .set    ITRACES,FALSE          # disable traces

# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
# NOTE: TRACESIZE, tflg_wrapoff, trr_ci, trr_num, trr_recsize, trr_trt are
# defined in itrace.inc and trace.inc -- latter last in masterfe.inc, and wins.
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

        .set    DEF_ITFLG,0xfffe         # default trace flag setting
#        .set    DEF_ITFLG,0xffff         # default trace flag setting (stop on full)
#
        .set    trr_recsize,16          # trace record size
        .set    trr_num,4096            # # trace records in trace area
        .set    TRACESIZE,trr_recsize*trr_num # size of trace area (must be
                                        #  multiple of trr_recsize)
#
# --- Trace flag bit definitions ---------------------------------------------
#
        .set    tflg_wrapoff,0          # trace area wrap turn-off-traces
        .set    tflg_online,1           # trace online events
        .set    tflg_offline,2          # trace offline events
        .set    tflg_dislun,3           # trace discover LUN
        .set    tflg_RF,4               # request function
        .set    tflg_RFcmplt,5          # request function complete
        .set    tflg_cmdcmplt,6         # command complete
        .set    tflg_tagqueue,7         # tag queueing functions
        .set    tflg_error,8            # trace errors
        .set    tflg_fabric,9           # trace fabric functions
        .set    tflg_debug,10           # trace debug functions

#
# --- Trace record type definitions ------------------------------------------
#
        .set    trt_online,1            # online event
        .set    trt_offline,2           # offline event
        .set    trt_turlun,0x40         # Test Unit Ready
        .set    trt_ssulun,0x41         # Start Stop Unit
        .set    trt_inqlun,0x42         # Inquire LUN
        .set    trt_mslun,0x43          # Mode Sense
        .set    trt_fabgan,0x45         # *** fabric GAN
        .set    trt_fabdisc,0x46        # *** fabric discovery
        .set    trt_tskreissue,0x4e     # discovery task reissue
        .set    trt_icmdcmplt,0x4f      # i command complete
        .set    trt_opnses,0x50         # open session
        .set    trt_clsses,0x51         # close session
        .set    trt_reqpar,0x52         # request session parameters
        .set    trt_sndcmd,0x53         # send SCSI command
        .set    trt_tskman,0x54         # task management function
        .set    trt_enbltsk,0x60        # enable task
        .set    trt_funcmplt,0x70       # function complete
        .set    trt_cmdcmplt,0x71       # SCSI command complete
        .set    trt_target_id,0x80      # target identified
        .set    trt_discmplt,0x81       # discovery complete
        .set    trt_targgone,0x82       # target gone
        .set    trt_badid,0xe0          # bad provider or requesor id
        .set    trt_ttimeout,0xe1       # task timeout
        .set    trt_ploop_start,0xf0    # private loop discovery start
        .set    trt_gantrace,0xf1       # gan trace
        .set    trt_1gantrace,0xfc      # 1st gan trace
        .set    trt_ftrace,0xf2         # fabric discover loop
        .set    trt_ftrace_e,0xf3       # fabric discover error
        .set    trt_flog_e,0xf8         # fabric login error
        .set    trt_debug,0xff          # debug trace

#
# --- Common trace record data structure definitions
#
        .set    trr_trt,0               # trace record type code        <b>
        .set    trr_ci,1                # chip instance                 <b>
        .set    trr_alpa,2              # AL-PA                         <b>
        .set    trr_lun,3               # lun                           <b>
#
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
