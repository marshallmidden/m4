# $Id: sos.inc 145407 2010-08-10 17:26:37Z m4 $
#*****************************************************************************
#
#  NAME: sos.inc  (Segment Optimization Structure)
#
#  DESCRIPTION:
#
#       To provide a data structure to manage defragmentation of devices.
#
#  Copyright (c) 1998 - 2001 XIOtech Corporation.   All rights reserved.
#
#*****************************************************************************
#
# --- Equates ----------------------------------------------------------------
#
        .set    sofcancel,0             # Cancel this defrag
        .set    sofstepdone,1           # This step was completed
        .set    sofsteprec,2            # This step was recorded
        .set    sofrestart,3            # Restart the SOS
        .set    sofactive,5             # This SOS is active on this controller
        .set    sofprepared,6           # SOS is ready to process
        .set    softerminate,7          # Cannot continue defragmentation
        .set    sofincomplete,8         # Error in processing of read/writes
        .set    sofprepfail,9           # Prepare for failover
#
# --- Begin structure -------------------------------------------------
#                                                                  *****
        .set    so_next,0               # Next entry in SOS list    <w>
        .set    so_pid,so_next+4        # PID for defrag drive      <s>
        .set    so_flags,so_pid+2       # Flags (see above)         <s>
        .set    so_remain,so_flags+2    # Blocks remaining          <w>
        .set    so_total,so_remain+4    # Blocks total to be copied <w>
#                                                                  *****
        .set    so_pdd,so_total+4       # PDD being defraged        <w>
        .set    so_asda,so_pdd+4        # Alternate SDA             <w>
        .set    so_count,so_asda+4      # Count of entries          <s>
        .set    so_current,so_count+2   # Current entry number      <s>
        .set    so_pcb,so_current+2     # Task PCB                  <w>
#                                                                  *****
        .set    so_xtensions,so_pcb+4   # Extension start area     alias
        .set    so_psd,0                # PSD                       <w>
        .set    so_sda,so_psd+4         # Starting disk address     <w>
        .set    so_slen,so_sda+4        # Segment length            <w>
        .set    so_rdd,so_slen+4        # RDD                       <w>
#
# --- End structure ----------------------------------------------------------
#
        .set    soxsiz,so_rdd+4                       # Length of extensions
        .set    sosiz,so_xtensions+(MAXRAIDS*soxsiz)  # Length of structure
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
