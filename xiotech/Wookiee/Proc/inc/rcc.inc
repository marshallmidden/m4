#**********************************************************************
#
#  NAME: rcc.inc (Retry Change Configuration request Definitions]
#
#  DESCRIPTION:
#
#       This file contains the data structures and definitions to
#       support process control requests between a client and the
#       primary copy provider.
#
#  Copyright (c) 2002 XIOtech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- Equates ---------------------------------------------------------
#
#  --- rcc1_flags definition
#
        .set    rcc1flg_abort,0         # RRC has been aborted
        .set    rcc1flg_active,1        # rcc is active
                                        #     waiting for response
#
# --- General definitions
#
#       Retry Change Configuration Level #1 Definition
#       -----------------------------------------------
#
#       This table defines the data structure used by a client to
#       manage a retry change configuration requet request. This
#       level is for the exclusive  use of the client.
#
#       This data structure is built and maintained in an ILT.
#
# --- Begin rcc1 main structure ---------------------------------------------
#
        .set    rcc1_cr,il_cr           # completion handler routine       <w>

        .set    rcc1_status,il_w0+0     # completion status                <b>
        .set    rcc1_flags,il_w0+1      # flags                            <b>
                                        # 2 spare bytes                   2<b>
        .set    rcc1_lastretry,il_w1    # processing timestamp             <w>
        .set    rcc1_reg1,il_w2         # general purpose register 1       <w>
        .set    rcc1_reg2,il_w3         # general purpose register 2       <w>

#
# --- End rcc1 main structure -----------------------------------------------
#
#
#       Re Level #2 Definition
#       -----------------------------------------------
#
#       This table defines the data structure used by a client to
#       process a retry change configuration request.
#
#
# --- Begin rcc2 main structure ---------------------------------------------
#
        .set    rcc2_status,il_w0+0     # completion status code           <b>
                                        # 3 spare bytes                   3<b>

        .set    rcc2_g0,il_w1           # General Register 0               <w>
        .set    rcc2_g1,il_w2           # General Register 1               <w>

        .set    rcc2_g2,il_w3           # General Register 2               <w>
        .set    rcc2_g3,il_w4           # General Register 3               <w>
        .set    rcc2_g4,il_w5           # General Register 4               <w>
        .set    rcc2_g5,il_w6           # General Register 5               <w>

#
# --- End rcc2 main structure -----------------------------------------------
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
