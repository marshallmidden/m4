# $Id: LL_Stats.inc 88786 2009-06-11 16:39:39Z m4 $
#******************************************************************************
#
#  NAME: lstats.inc (LST)
#
#  DESCRIPTION:
#
#       To define link layer statistics gathering structure.
#
#  Copyright (c) 1996 XIOtech Corporation.  All rights reserved.
#
#******************************************************************************
#
#
# --- begin structure
#
#
# --- Link960 Layer statistics
#
        .set    ls_vrpocount,0              # outstanding outbound VRP count<s>
        .set    ls_vrpicount,ls_vrpocount+2 # outstanding inbound VRP count <s>
        .set    ls_vrpotcount,ls_vrpicount+2# total outbound VRP count      <w>
        .set    ls_vrpitcount,ls_vrpotcount+4# total inbound VRP count      <w>
        .set    ls_lastiop,ls_vrpitcount+4  # last inbound op processed     <s>
        .set    ls_lastoop,ls_lastiop+2     # last outbound op processed    <s>
#
#
# --- end structure
#
# CAUTION *** CAUTION *** CAUTION *** CAUTION *** CAUTION *** CAUTION *** CAUTION
#
# Keep this value quad aligned in order for the statistic collection to
# function correctly.
#
        .set    lssize,ls_lastoop+2         # sizeof stats area
#
#******************************************************************************
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
