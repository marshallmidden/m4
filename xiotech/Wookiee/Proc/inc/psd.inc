# $Id: psd.inc 145407 2010-08-10 17:26:37Z m4 $
#**********************************************************************
#
#  NAME: psd.inc (Physical Segment Description)
#
#  DESCRIPTION:
#       To provide a common means of defining the Physical Segment
#       Description (PSD) structure.
#
#  Copyright (c) 1996-2010 XIOtech Corporation.  All rights reserved.
#
#**********************************************************************
#
# ATTENTION:
#   If fields ps_pid, ps_status, or ps_astatus are changed, it is probably
#   necessary to update the Direct Memory Copy (DMC) raid information via:
#       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
#
#
# --- Equates ---------------------------------------------------------
#
#       ps_status definitions
#
        .set    psnonx,0x00             # Nonexistent
        .set    psinop,0x01             # Inoperable
        .set    psuninit,0x02           # Uninitialized
        .set    psinit,0x04             # Initializing
        .set    pserror,0x05            # Error detected, hotspare pending
        .set    psop,0x10               # Operational
        .set    psrebuild,0x12          # Rebuild needed
#
# --- ps_astatus bit definitions
#
                                        # 0 is reserved for RAID parity scan
        .set    psarebuild,1            # 0x02 Ready for rebuild
        .set    psauninit,2             # 0x04 Uninitialized
        .set    psadefrag,3             # 0x08 Defragmenting
        .set    psaerror,4              # 0x10 Error detected
        .set    psahotspare,5           # 0x20 Hotspare needed
        .set    psahsrequired,6         # 0x40 Hotspare required
        .set    psarebuildwait,7        # 0x80 Rebuild wait for PDD operable
#
# --- Begin structure -------------------------------------------------
#                                                                  *****
        .set    ps_pid,0                # Physical device ID        <s>
        .set    ps_status,ps_pid+2      # Status                    <b>
        .set    ps_pbfail,ps_status+1   # Prev boot fail (T/F)      <b>
                                        # Device was rebuilding/
                                        # inoperative or unavailable
#
        .set    ps_npsd,ps_pbfail+1     # Next PSD entry            <w>
#
        .set    ps_rid,ps_npsd+4        # RAID ID                   <s>
        .set    ps_astatus,ps_rid+2     # Additional status         <b>
#
#       reserved 5                                                 5<b>
#                                                                  *****
        .set    ps_sda,ps_astatus+6     # Starting disk address     <l>
        .set    ps_slen,ps_sda+8        # # of sectors in segment   <l>
#                                                                  *****
        .set    ps_rlen,ps_slen+8       # # sectors rebuilt         <l>
#
#       reserved 8                                                 5<b>
#                                                                  *****
#                                                                  *****
# --- End structure ---------------------------------------------------
#
        .set    psdsiz,ps_rlen+16        # Size of PSD
#
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
