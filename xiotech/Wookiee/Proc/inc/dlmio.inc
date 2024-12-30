# $Id: dlmio.inc 149097 2010-10-14 17:20:42Z raghub $
#**********************************************************************
#
#  NAME: dlmio.inc (Data-link Manager I/O definitions)
#
#  DESCRIPTION:
#
#       This file defines equates and data structures used by the
#       data-link manager (DLM) to manage I/O operations out to
#       other storage controllers, both MAGNITUDE links as well
#       as Foreign Targets.
#
#  Copyright (c) 2000-2002 XIOtech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- Equates ---------------------------------------------------------
#
# --- Misc. primary I/O ILT related definitions -----------------------
#
        .set    diop_rtycnt,15          # default I/O retry count
        .set    diop_timeout,60         # default timeout (in secs.)
#
# --- Primary I/O ILT state code (dlmio_p_state) definitions ----------
#
        .set    diopst_act,0            # I/O active
        .set    diopst_rty,1            # waiting to retry I/O
        .set    diopst_dep,2            # waiting for dependent I/O to
                                        #  complete
        .set    diopst_abort,3          # aborted
#
# --- Begin primary I/O ILT basic data structure ----------------------
#
#       Built in 2 nest levels of the requesting ILT
#
#
#************            NEST LEVEL #1                  ***************
#
        .set    dlmio_p_cr,il_cr        # ILT completion routine    <w>
        .set    dlmio_p_depcnt,il_pcb   # dependency count (when on <w>
                                        #  dependency queue of ILT)
        .set    dlmio_p_deplist,il_pcb  # dependency list (when on  <w>
                                        #  active LDD queue)
        .set    dlmio_p_sda,il_w0       # I/O SDA                   <l>
        .set    dlmio_p_len,il_w2       # I/O length                <w>
        .set    dlmio_p_ldd,il_w3       # assoc. LDD address        <w>
        .set    dlmio_p_rdd,il_w4       # assoc. RDD address        <w>
        .set    dlmio_p_sgl,il_w5       # assoc. SGL address        <w>
        .set    dlmio_p_ilt,il_w6       # assoc. I/O ILT address    <w>
        .set    dlmio_p_fc,il_w7        # I/O function code         <b>
        .set    dlmio_p_state,il_w7+1   # I/O state code            <b>
        .set    dlmio_p_flag,il_w7+2    # Flag byte #1              <b>
                                        # Bit 7 = 1=on LDD active list
                                        #     6 =
                                        #     5 =
                                        #     4 =
                                        #     3 =
                                        #     2 =
                                        #     1 =
                                        #     0 =
        .set    dlmio_p_rtycnt,il_w7+3  # retry count               <b>
#
#************            NEST LEVEL #2                  ***************
#
        .set    dlmio_p_status,ILTBIAS+il_w0 # completion status    <b>
        .set    dlmio_p_ec1,ILTBIAS+il_w0+1 # error code #1         <b>
        .set    dlmio_p_ec2,ILTBIAS+il_w0+2 # error code #2         <b>
        .set    dlmio_p_ec3,ILTBIAS+il_w0+3 # error code #3         <b>
        .set    dlmio_p_extstat,ILTBIAS+il_w1 # extended status word<w>
        .set    dlmio_p_timeout,ILTBIAS+il_w2 # I/O timeout         <s>
                                        #  (in secs.)
        .set    dlmio_p_psda,ILTBIAS+il_w4 # precedence SDA field   <l>
        .set    dlmio_p_peda,ILTBIAS+il_w6 # precedence EDA field   <l>
#
# --- End primary I/O ILT basic data structure ------------------------
#
#
# --- Begin secondary I/O ILT basic data structure --------------------
#
# --- Secondary ILT Nest Level #1 data structure ----------------------
#
        .set    dlmio_s1_ilt,il_w0      # assoc. primary ILT    <w>
                                        #  address
        .set    dlmio_s1_srpsize,il_w1  # SRP memory allocation <w>
                                        #  size
        .set    dlmio_s1_tpmt,il_w2     # TPMT address used     <w>
#
# --- End Secondary ILT Nest Level #1 data structure ------------------
#
# --- Secondary ILT Nest Level #2 data structure ----------------------
#
        .set    dlmio_s2_srp,il_w0      # assoc. SRP address     <w>
        .set    dlmio_s2_iltvrp,il_w3   # assoc. ILT/VRP address <w>
        .set    dlmio_s2_srpvrp,il_w4   # VRP/SRP combo address  <w>
        .set    dlmio_s2_path,il_w7     # dest. path #           <w>
#
# --- End Secondary ILT Nest Level #2 data structure ------------------
#
#
# --- End secondary I/O ILT basic data structure ----------------------
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
