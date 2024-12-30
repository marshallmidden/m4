# $Id: sdx.inc 145407 2010-08-10 17:26:37Z m4 $
#**********************************************************************
#
#  NAME: sdx.inc (Server Device Index table)
#
#  DESCRIPTION:
#
#       To define the lookup table for the server devices indexed
#       by the server ID number.
#
#  Copyright (c) 1996-2000 XIOtech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- Begin structure -------------------------------------------------
#
        .set    sx_ecnt,-4              # Num entries in table      <s>
#
#       reserved 2
#
        .set    sx_sdd,0                # Start of the entries      <w>
#
# --- End structure ---------------------------------------------------
#
        .set    sdxsiz,sx_sdd+(4*MAXSERVERS) # Size of SDX
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
