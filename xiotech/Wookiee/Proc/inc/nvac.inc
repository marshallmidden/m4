# $Id: nvac.inc 145407 2010-08-10 17:26:37Z m4 $
#**********************************************************************
#
#  NAME: nvac.inc (Non-Volatile Activity Control)
#
#  DESCRIPTION:
#
#       To provide a common means of defining the NVA Control structure which
#       aids in the dynamic assignment and release of NVA entries.
#
#  Copyright (c) 1996-2001 XIOtech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- Begin control structure -----------------------------------------
#                                                                  *****
        .set    nc_nvarec,0             # Addr of 1st NVA record    <w>
        .set    nc_csum,nc_nvarec+4     # Addr of checksum in NVRAM <w>
        .set    nc_mapbase,nc_csum+4    # Base of NVA map           <w>
        .set    nc_mapptr,nc_mapbase+4  # Pointer into NVA map      <w>
#                                                                  *****
        .set    nc_scsum,nc_mapptr+4    # Shadowed checksum         <w>
        .set    nc_cur,nc_scsum+4       # Current available NVAs    <w>
        .set    nc_min,nc_cur+4         # Minimum available NVAs    <w>
        .set    nc_wait,nc_min+4        # Number waiting            <w>
#                                                                  *****
#
# --- End structure ---------------------------------------------------
#
        .set    nvacsize,nc_wait+4      # Size of NVA control
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
