# $Id: ftospb.inc 159300 2012-06-16 04:47:51Z m4 $
#*****************************************************************************
#
#  NAME: ftospb.inc  (Foreign Target Open Session Parameter Block)
#
#  DESCRIPTION:
#
#       This module defines the Open Session Parameter Block used to pass
#       information about a session being opened to a Foreign Target
#       by the Data-link manager to the Link-level driver.
#
#  Copyright (c) 1999 - 2000 XIOtech Corporation.   All rights reserved.
#
#*****************************************************************************
#
# --- Equates ----------------------------------------------------------------
#
# --- Data structure ---------------------------------------------------------
#
        .set    ftos_lldid,0            # Link-level driver session ID    <w>
        .set    ftos_devsn,ftos_lldid+4 # Device serial #               16<b>
        .set    ftos_dsize,ftos_devsn+16 # Device size (# sectors)        <w>   GT2TB Foreign Target problem
        .set    ftos_venid,ftos_dsize+4 # Vendor ID (from INQUIRY)       8<b>
        .set    ftos_prid,ftos_venid+8  # Product ID (from INQUIRY)     16<b>
        .set    ftossiz,ftos_prid+16    # size of Foreign Target OSPB
#
#--        .set    ftos_lldid,0            # Link-level driver session ID          <w>
#--        .set    ftos_devsn,ftos_lldid+4 # Device serial #                     16<b>
        .set    ftos_dsize_GT2TB,ftos_devsn+16 # Device size (# sectors)           <l>   GT2TB Foreign Target problem
        .set    ftos_venid_GT2TB,ftos_dsize_GT2TB+8 # Vendor ID (from INQUIRY)    8<b>
        .set    ftos_prid_GT2TB,ftos_venid_GT2TB+8  # Product ID (from INQUIRY)  16<b>
        .set    ftossiz_GT2TB,ftos_prid_GT2TB+16    # size of Foreign Target OSPB
#
# --- End of data structure -------------------------------------------------
#*****************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
