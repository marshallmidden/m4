# $Id: trace.inc 145407 2010-08-10 17:26:37Z m4 $
#*****************************************************************************
#
#  NAME: trace.inc  (Trace data structures & definitions)
#
#  DESCRIPTION:
#
#       This module defines the data structures and definitions
#       to support traces in the CDRIVER & MAGDRVR modules.
#
#  Copyright (c) 1998 - 2000 XIOtech Corporarion.   All rights reserved.
#
#*****************************************************************************
#
# --- Equates ----------------------------------------------------------------
#

# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
# NOTE: TRACESIZE, tflg_wrapoff, trr_ci, trr_num, trr_recsize, trr_trt are
# defined in itrace.inc and trace.inc -- latter last in masterfe.inc, and wins.
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

        .set    trr_recsize,16          # trace record size
#
# --- Trace area size definitions
#
        .set    trr_num,4096            # # trace records in trace area
        .set    TRACESIZE,trr_recsize*trr_num # size of trace area (must be
                                        #  multiple of trr_recsize)
#
# --- Trace flag bit definitions ---------------------------------------------
#
        .set    tflg_wrapoff,0          # trace area wrap turn-off-traces
        .set    tflg_XL$recv,1          # XL$receive_io input trace
        .set    tflg_c$cdb4,2           # c$cdb4 input trace
        .set    tflg_c$imno4,3          # c$imno4 input trace
        .set    tflg_c$offl4,4          # c$offl4 input trace
        .set    tflg_MAGsubv,5          # call MAG$submit_vrp trace
        .set    tflg_srpreq,6           # mag1_srpreq input trace
        .set    tflg_srpcomp,7          # mag1_srpcomp input trace
        .set    tflg_MAGcomp,8          # mag1_MAGcomp input trace
        .set    tflg_ISP$recv,9         # call ISP$receive_io trace
        .set    tflg_mag1iocr,10        # mag1_iocr input trace
        .set    tflg_data,14            # data trace
        .set    tflg_SENSE,15           # SENSE data trace
#
# --- Trace record type definitions ------------------------------------------
#
        .set    trt_XL$recv,1           # XL$receive_io input trace record
        .set    trt_c$cdb4,2            # c$cdb4 input trace record
        .set    trt_c$imno4,3           # c$imno4 input trace record
        .set    trt_c$offl4,4           # c$offl4 input trace record
        .set    trt_MAGsubv,5           # call MAG$submit_vrp trace record
        .set    trt_srpreq,6            # mag1_srpreq input trace record
        .set    trt_srpcomp,7           # mag1_srpcomp input trace record
        .set    trt_MAGcomp,8           # mag1_MAGcomp input trace record
        .set    trt_ISP$recv,9          # call ISP$receive_io trace record
        .set    trt_mag1iocr,0x0a       # nag1_iocr input trace record
        .set    trt_data,0x0e           # data trace record
        .set    trt_SENSE,0x0f          # SENSE data trace record
#
# --- Common trace record data structure definitions
#
        .set    trr_trt,0               # trace record type code        <b>
        .set    trr_ci,1                # chip instance                 <b>
        .set    trr_exid,2              # exchange ID                   <s>
#
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
