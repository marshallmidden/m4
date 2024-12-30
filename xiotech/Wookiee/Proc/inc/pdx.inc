# $Id: pdx.inc 145407 2010-08-10 17:26:37Z m4 $
#**********************************************************************
#
#  NAME: pdx.inc (Physical Device Index table)
#
#  DESCRIPTION:
#       To define the lookup table for the Physical devices indexed
#       by the Physical ID number.
#
#  Copyright (c) 1996-2010 XIOtech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- Begin structure -------------------------------------------------
#
        .set    px_ecnt,-4              # Num entries in table      <s>
#
#       reserved 2
#
        .set    px_pdd,0                # Start of the entries      <w>
#
# --- End structure ---------------------------------------------------
#
        .set    pdxsiz,px_pdd+(4*(MAXDRIVES+MAXSES+MAXMISC)) # Size of PDX
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
