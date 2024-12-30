# $Id: daml.inc 145407 2010-08-10 17:26:37Z m4 $
#**********************************************************************
#
#  NAME: daml.inc (Disk Allocation Map Link)
#
#  DESCRIPTION:
#
#       To provide a common means of mapping the assigned disk space on
#       a particular disk drive.
#
#  Copyright (c) 1997-1999 XIOtech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- Equates ---------------------------------------------------------
#
        .set    dabdirty,0              # Dirty bit.  Must rebuild all
#
# --- Begin structure -------------------------------------------------
#                                                                  *****
        .set    da_pdd,0                # PDD for this DAML         <w>
        .set    da_count,da_pdd+4       # Number of entries in table<s>
        .set    da_flags,da_count+2     # Flags                     <b>
#
#       reserved 1
#
#       These two fields are in allocation units, not logical blocks.
#
        .set    da_largest,da_flags+2   # Largest available segment <w>
        .set    da_total,da_largest+4   # Total available space     <w>
#                                                                  *****
        .set    da_firstgap,da_total+4  # First gap location        <s>
#
#       reserved 14
#
# --- Extensions
#
        .set    da_AUgap,0              # Gap in allocation units
        .set    da_AUsda,da_AUgap+4     # Starting address in allocation units
        .set    da_AUslen,da_AUsda+4    # Segment length in allocation units
        .set    da_AUrid,da_AUslen+4    # RAID ID
#
#       reserved 2
#
        .set    damlxsiz,da_AUrid+4
#
# --- End structure ---------------------------------------------------
#
        .set    damlsiz,da_firstgap+16  # Size of DAML
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
