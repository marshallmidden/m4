# $Id: dspif.inc 145407 2010-08-10 17:26:37Z m4 $
#*****************************************************************************
#
#  NAME: dspif.inc  (Datagram services provider interface definitions)
#
#  DESCRIPTION:
#
#       This file defines the datagram services provider interface
#       data structures and definitions needed to interface to the
#       Data-link manager's datagram service.
#
#  Copyright (c) 2000-2003 XIOtech Corporation.   All rights reserved.
#
#*****************************************************************************
#
# --- Equates ----------------------------------------------------------------
#
# --- Client side operation state code (dsc_state) definitions
#
        .set    dsc_st_active,0         # op. active to LLD
        .set    dsc_st_wait,1           # waiting to retry
#
        .set    dsc_rtycnt,4            # datagram retry count set value
        .set    dsc_timeout,4           # datagram default timeout in seconds
#
#*****************************************************************************
#
# --- Client side datagram service interface data structures -----------------
#
#       This data structure is set up to occupy an ILT in nest levels 1-4.
#
# --- Client local header nest level data structure      *** Nest Level #1 ***
#
        .set    dsc1_rqhdr,0            # request message local header  32<b>
        .set    dsc1_rshdr,32           # response message local header 16<b>
        .set    dsc1_ulvl,ILTBIAS       # user nest level
#
# --- Client user nest level data structure              *** Nest Level #2 ***
#
        .set    dsc2_rqhdr_ptr,il_w0    # local req. message header addr. <w>
        .set    dsc2_rshdr_ptr,il_w1    # local resp. message header addr.<w>
        .set    dsc2_rqbuf,il_w2        # request message buffer address  <w>
        .set    dsc2_rqlen,il_w3        # request message length          <w>
        .set    dsc2_rsbuf,il_w4        # response message buffer address <w>
        .set    dsc2_rslen,il_w5        # response buffer length          <w>
        .set    dsc2_sglptr,il_w6       # sgl pointer for other data      <w>
        .set    dsc2_g0,il_w7           # general reg. #0                 <w>
#
# --- Client DSP nest level data structure               *** Nest Level #3 ***
#
        .set    dsc3_mlmt,il_w0         # assoc. MLMT address             <w>
        .set    dsc3_path,il_w1         # specified path #                <b>
        .set    dsc3_retry,il_w1+1      # op. retry count                 <b>
        .set    dsc3_state,il_w1+2      # op. state code                  <b>
        .set    dsc3_reroute,il_w1+3    # reroute datagram count          <b>
        .set    dsc3_dtmt,il_w2         # assoc. DTMT message was sent    <w>
        .set    dsc3_srp,il_w3          # assoc. SRP address              <w>
        .set    dsc3_reqdtmt,il_w4      # requested DTMT                  <w>
        .set    dsc3_thispaths,il_w5    # This cntrls paths retried already <b>
        .set    dsc3_otherpaths,il_w5+1 # Other cntrls paths retried already <b>
                                        #         reserved                <b>
        .set    dsc3_timeout,il_w5+3    # Command Timeout requested       <b>
        .set    dsc3_orig_dsn,il_w6     # original destination serial #   <w>
                                        # (used for datagram reroute logic)
#
# --- Client SRP request nest level data structure       *** Nest Level #4 ***
#
        .set    dsc4_srp,il_w0          # SRP address                     <w>
        .set    dsc4_iltvrp,il_w3       # assoc. ILT/VRP address          <w>
        .set    dsc4_vrpsrp,il_w4       # VRP/SRP combo address           <w>
        .set    dsc4_path,il_w7         # path #                          <w>
#
# --- End client side datagram service interface data structures -------------
#
#*****************************************************************************
#
# --- Server side datagram service interface data structures -----------------
#
#       These data structures are set up to occupy an ILT in nest levels 1-4.
#
# --- Server VRP request nest level data structure       *** Nest Level #1 ***
#
        .set    dss1_vrp,il_w0          # assoc. VRP address
        .set    dss1_path,il_w7         # path #
#
# --- Server local header nest level data structure      *** Nest Level #2 ***
#
        .set    dss2_rqhdr,0            # request message local header area
        .set    dss2_rshdr,32           # response message local header area
        .set    dss2_dsplvl,ILTBIAS     # DSP nest level
#
# --- Server DSP nest level data structure               *** Nest Level #3 ***
#
        .set    dss3_rqhdr_ptr,il_w0    # local request message header addr.
        .set    dss3_rshdr_ptr,il_w1    # local response message header addr.
        .set    dss3_rqbuf,il_w2        # request message buffer address. Does
                                        #  not include header.
        .set    dss3_rqlen,il_w3        # request message length. Does not
                                        #  include header.
        .set    dss3_rsbuf,il_w4        # response message buffer address.
                                        #  Does not include header.
        .set    dss3_rslen,il_w5        # response buffer length. Does not
                                        #  include header.
        .set    dss3_g0,il_w6           # general reg. #0
        .set    dss3_g1,il_w7           # general reg. #1
#
# --- Server user nest level data structure              *** Nest Level #4 ***
#

#
# --- End server side datagram service interface data structures -------------
#
#*****************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
