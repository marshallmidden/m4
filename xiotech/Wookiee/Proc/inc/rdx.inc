# $Id: rdx.inc 145407 2010-08-10 17:26:37Z m4 $
#**********************************************************************
#
#  NAME: rdx.inc (RAID Device Index table)
#
#  DESCRIPTION:
#
#       To define the lookup table for the RAID devices indexed
#       by the RAID ID number.
#
#  Copyright (c) 1996-2000 XIOtech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- Begin structure -------------------------------------------------
#
        .set    rx_ecnt,-4              # Num entries in table      <s>
#
#       reserved 2
#
        .set    rx_rdd,0                # Start of the entries      <w>

#
# --- End structure ---------------------------------------------------
#
        .set    rdxsiz,rx_rdd+(4*MAXRAIDS) # Size of RDX
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
