# $Id: drp.inc 145407 2010-08-10 17:26:37Z m4 $
#**********************************************************************
#
#  NAME: drp.inc (DLM Request Packet)
#
#  DESCRIPTION:
#
#       To provide a common means of defining the DLM Request
#       Packet (DRP) definitions.
#
#  Copyright (c) 2001-2005 XIOtech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- Equates ---------------------------------------------------------
#
        .set    dr_max_sgl_read,2       # Maximum of 2 Read SGLs allowed in DRP
        .set    dr_max_sgl_write,2      # Maximum of 2 Write SGLs allowed in DRP
        .set    dr_max_sgls,4           # Maximum number of SGLs allowed in DRP
#
# --- Normal DRP function code definitions ----------------------------
#
        .set    drccbtodlm,0x0700       # CCB to DLM Datagram
        .set    drdlmtoccb,0x0701       # DLM to CCB Datagram
        .set    drcachetodlm,0x0710     # Cache to DLM Datagram
        .set    drdlmtocache,0x0711     # DLM to Cache Datagram
#
# --- "Normal" DRP structure -------------------------------------------
#                                                                       *****
        .set    dr_func,0               # Function                       <s>
#
#       reserved 1 (used by Link 960)                                    <b>
#
        .set    dr_status,dr_func+3     # Status                         <b>
        .set    dr_sglptr,dr_status+1   # SGL for discontiguous data     <w>
        .set    dr_timeout,dr_sglptr+4  # DRP timeout (in seconds)       <b>
        .set    dr_issue_cnt,dr_timeout+1 # DRP Issue count              <b>
                                        #   (1 = do not retry on error)
#
#       reserved 2                                                      2<b>
#
        .set    dr_pptr,dr_issue_cnt+3  # Packet physical address        <w>
#                                                                       *****
        .set    dr_rsp_length,dr_pptr+4 # Response length (hdr & extended)<w>
        .set    dr_rsp_address,dr_rsp_length+4 # Response address        <w>
        .set    dr_req_address,dr_rsp_address+4 # Request address        <w>
        .set    dr_req_length,dr_req_address+4 # Request len (hdr & ext) <w>
#                                                                       *****
# --- End "normal" structure -------------------------------------------
#
        .set    drpsiz,dr_req_length+4  # Size of DRP
#
#**********************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
