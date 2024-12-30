# $Id: sgl.inc 122127 2010-01-06 14:04:36Z m4 $
        .eject
#**********************************************************************
#
#  NAME: sgl.inc (Scatter/Gather List)
#
#  DESCRIPTION:
#
#       To provide a common means of defining the Scatter/Gather List
#       elements.
#
#  Copyright (c) 1996-2000 XIOtech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- Equates ---------------------------------------------------------
#
# --- Equates for the Scatter/Gather Flag Byte
        .set    sg_buffer_alloc,7    # bit 7 = Buffer has already been allocated
#
# --- Pre-header (hidden quadword) - used by m_asglbuf/M$rsglbuf
#
        .set    sg_alloc_len,MGRAN+1    # Precede real struct by 16 bytes
# --- Remaining 3 pre-header words reserved
#
# --- Begin structure (header) ----------------------------------------
#                                                                  *****
        .set    sg_scnt,0               # Descriptor count          <s>
        .set    sg_owners,sg_scnt+2     # Ownership count (non-zero)<b>
        .set    sg_flag,sg_owners+1     # Flag byte                 <b>
        .set    sg_size,sg_flag+1       # Size of SGL               <w>
        .set    sg_desc0,sg_size+4      # Descriptor 0 base
#
# --- End structure ---------------------------------------------------
#
        .set    sghdrsiz,sg_desc0       # Size of header
#
# --- Begin structure (descriptor) ------------------------------------
#
        .set    sg_addr,0               # Address                   <w>
        .set    sg_len,sg_addr+4        # Length (max of 24 bits)   <w>
#
# --- End structure ---------------------------------------------------
#
        .set    sgdescsiz,sg_len+4      # Size of descriptor
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
