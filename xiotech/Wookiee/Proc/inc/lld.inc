# $Id: lld.inc 149225 2010-10-19 14:31:29Z raghub $
#*****************************************************************************
#
#  NAME: lld.inc  (Link-level Driver Definitions)
#
#  DESCRIPTION:
#
#       This module defines data structures and equates used by
#       the link-level driver.
#
#  Copyright (c) 1999 - 2003 XIOtech Corporation.   All rights reserved.
#
#*****************************************************************************
#
# --- Equates ----------------------------------------------------------------
#
        .set    DGBSYCNT,12             # datagram
        .set    SNDDG_CMDTO,4           # Default Datagram Command Timeout (Sec)
        .set    SNDDG_RTYCNT,4          # Default Datagram Command Retry Count
#
# --- Data structure for Link-level Driver ILT/IRP at nest level 1 -----------
#
        .set    lirp1_cr,il_cr          # my completion handler routine   <w>
        .set    lirp1_irp,il_w0         # assoc. IRP address         [w0] <w>
        .set    lirp1_lldid,il_w1       # assoc. lldid (ltmt/lsmt)   [w1] <w>
        .set    lirp1_srp,il_w2         # assoc. SRP                 [w2] <w>
        .set    lirp1_flags,il_w3       # flags byte #1              [w3]1<b>
                                        # Bit 7 =
                                        #     6 =
                                        #     5 =
                                        #     4 =
                                        #     3 =
                                        #     2 =
                                        #     1 =
                                        #     0 = LSMT alloced - free on error
        .set    lirp1_bsycnt,il_w3+1    # busy count                 [w3]1<b>
        .set    lirp1_g0,il_w4          # general reg. #0            [w4] <w>
        .set    lirp1_g1,il_w5          # general reg. #1            [w5] <w>
        .set    lirp1_retry,il_w6       # lld operation retry cntr   [w6]1<b>
#
# --- Data structure for Link-level driver ILT/IRP at nest level 2 ----------
#
        .set    lirp2_cr,il_cr          # completion handler routine      <w>
        .set    lirp2_irp,il_misc       # assoc. IRP address              <w>
        .set    lirp2_s_ILT,il_w0       # initiator driver ILT address    <w>
#
# --- End of data structure -------------------------------------------------
#
#
# --- Data structure for Link-level Driver ILT/VRP at nest level 2 -----------
#
        .set    lvrp2_cr,il_cr          # my completion handler routine   <w>
        .set    lvrp2_ltmt,il_w0        # assoc. LTMT address             <w>
        .set    lvrp2_sgl,il_w1         # assoc. SGL/buffer address       <w>
        .set    lvrp2_g0,il_w3          # general reg. #0                 <w>
        .set    lvrp2_g1,il_w4          # general reg. #1                 <w>
#
# --- End of data structure -------------------------------------------------
#
#
# --- Data structure for Server request/response buffer storage -------------
#


        .set    sdg_writeILT,il_w0      # write ILT ptr             1[w0] <w>
        .set    sdg_readILT,il_w1       # read ILT ptr              1[w1] <w>
        .set    sdg_flag,il_w2          # flags                     1[w2] <b>

        .set    sdg_sgl,il_w3           # assoc. SGL/buffer address 1[w3] <w>
        .set    sdg_reqptr,il_w4        # request buffer pointer    1[w4] <w>
        .set    sdg_reqlen,il_w5        # request length            1[w5] <w>
        .set    sdg_respptr,il_w6       # response buffer pointer   1[w6] <w>
        .set    sdg_resplen,il_w7       # response length           1[w7] <w>

        .set    sdg_desc0_ptr,il_w0+ILTBIAS # descriptor record 0   2[w0] <w>
        .set    sdg_desc0_len,il_w1+ILTBIAS #                       2[w1] <w>
        .set    sdg_desc1_ptr,il_w2+ILTBIAS # descriptor record 1   2[w2] <w>
        .set    sdg_desc1_len,il_w3+ILTBIAS #                       2[w3] <w>
        .set    sdg_desc2_ptr,il_w4+ILTBIAS # descriptor record 2   2[w4] <w>
        .set    sdg_desc2_len,il_w5+ILTBIAS #                       2[w5] <w>
        .set    sdg_desc3_ptr,il_w6+ILTBIAS # descriptor record 2   2[w6] <w>
        .set    sdg_desc3_len,il_w7+ILTBIAS #                       2[w7] <w>

        .set    sdg_reqhdr,il_w0+(2*ILTBIAS) # request header       3[w0]8<w>
                                             # save area
#
# --- End of data structure -------------------------------------------------
#
#
# --- Data structure for Link-level Driver Send Datagram nest lvl 2 ---------
#
        .set    sndg2_reqptr,il_w0      # request data pointer       [w0] <w>
        .set    sndg2_reqlen,il_w1      # request data length        [w1] <w>
        .set    sndg2_respptr,il_w2     # response data pointer      [w2] <w>
        .set    sndg2_resplen,il_w3     # response data length       [w3] <w>
        .set    sndg2_sgl,il_w4         # sgl ptr                    [w4] <w>
        .set    sndg2_esgl,il_w5        # extended sgl pointer       [w5] <w>
        .set    sndg2_cmdto,il_w6       # message command timeout    [w6] <b>
                                        #  0 = use default
        .set    sndg2_rtycnt,il_w6+1    # message command retry count[w6] <b>
                                        #  (these retries are for bus level
                                        #   errors (LIP, Abort, etc.))
                                        #  0 = use default
                                        # reserved                   [w6] <s>
        .set    sndg2_srp,il_w7         # srp ptr                    [w7] <w>

#
# --- End of data structure -------------------------------------------------
#
#
# --- Data structure for Link-level Driver Send Datagram nest lvl 3 ---------
#
        .set    sndg3_writeILT,il_w0    # write ILT" ptr             [w0] <w>
        .set    sndg3_readILT,il_w1     # read ILT" ptr              [w1] <w>
        .set    sndg3_seqnum,il_w2      # exchange sequence number   [w2] <s>
        .set    sndg3_xchgID,il_w2      # exchange ID                [w2] <b>
                                        # spare                      [w2] <b>
        .set    sndg3_status,il_w3      # status                     [w3] <b>
        .set    sndg3_ec1,il_w3+1       # error code 1               [w3] <b>
        .set    sndg3_ec2,il_w3+2       # error code 2               [w3] <b>
                                        # spare                      [w3] <b>
#
# --- End of data structure -------------------------------------------------
#
#
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
