# $Id: nva.inc 145407 2010-08-10 17:26:37Z m4 $
#**********************************************************************
#
#  NAME: nva.inc (Non-Volatile Activity)
#
#  DESCRIPTION:
#
#       To provide a common means of defining the Non-Volatile Activity
#       structure which is used to control RAID reconstruction following
#       an abnormal shutdown with RAID 5 write operations in progress.
#
#  Copyright (c) 1996-2003 XIOtech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- Equates ---------------------------------------------------------
#
#
# --- Begin base structure --------------------------------------------
#
        .set    nv_csum,0               # Checksum                  <w>
#
#       reserved 12                                                3<w>
#                                                                  *****
#       reserved 32*16 - needed to make PxNVANUM multiple of 32    7<qw>
#                                                                  *****
        .set    nvabasesiz,nv_csum+512  # Size of the base header area
                                        # Start of dynamic records
#
# --- End structure ---------------------------------------------------
#
#
# --- Begin record structure ------------------------------------------
#                                                                  *****
        .set    nv_id,0                 # ID - RAID or Virtual      <s>
#
#       reserved 2 (set to zero when assigning or freeing)          <s>
#
        .set    nv_length,nv_id+4       # Sector count              <w>
                                        # If nv_length = 0 then this
                                        # entry is not being used.
        .set    nv_lsda,nv_length+4     # Logical Starting Disk Addr<l>
#
# --- End structure ---------------------------------------------------
#
        .set    nvasiz,nv_lsda+8        # Size of an NVA record
#
# --- Equates ---------------------------------------------------------
#
#     Note: PxNVANUM must be a multiple of 32 !!!
.ifdef BACKEND
        .set    P3NVANUM,((NVSRAMP3SIZ-nvabasesiz)/nvasiz) # # of P3 NVA entries
.endif
        .set    P4NVANUM,((NVSRAMP4SIZ-nvabasesiz)/nvasiz) # # of P4 NVA entries

        .set    P3_ALLOC,3
        .set    P4_ALLOC,4
        .set    P3_REL,3
        .set    P4_REL,4
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
