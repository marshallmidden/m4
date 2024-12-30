# $Id: mlospb.inc 159300 2012-06-16 04:47:51Z m4 $
#*****************************************************************************
#
#  NAME: mlospb.inc  (MAGNITUDE Link Open Session Parameter Block)
#
#  DESCRIPTION:
#
#       This module defines the Open Session Parameter Block used to pass
#       information about a session being opened to a MAGNITUDE Link
#       by the Data-link manager to the Link-level driver.
#
#  Copyright (c) 1999 - XIOtech Corporation.   All rights reserved.
#
#*****************************************************************************
#
# --- Equates ----------------------------------------------------------------
#
# --- Data structure ---------------------------------------------------------
#
        .set    mlos_lldid,0            # Link-level driver session ID    <w>
        .set    mlos_dsize,mlos_lldid+4 # Disk size in sectors            <w>   GT2TB VLINK problem
        .set    mlos_dname,mlos_dsize+4 # VDisk name                     8<b>
        .set    mlos_basesn,mlos_dname+8 # Base MAG serial number         <w>
        .set    mlos_basecl,mlos_basesn+4 # Base MAG cluster number       <b>
        .set    mlos_basevd,mlos_basecl+1 # Base MAG VDISK number         <b>
#
# --- Reserved 2
#
        .set    mlossiz,mlos_basevd+3   # size of MAGNITUDE Link OSPB
#
# --- End of data structure -------------------------------------------------
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
