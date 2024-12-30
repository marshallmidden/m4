# $Id: fh.inc 145407 2010-08-10 17:26:37Z m4 $
#*****************************************************************************
#
#  NAME: fh.inc  (Firmware Header)
#
#  DESCRIPTION:
#
#       Structure definition for the firmware header.
#       This must match the structure in codeburn.h.
#
#  Copyright (c) 2000,2009 - Xiotech Corporation.   All rights reserved.
#
#*****************************************************************************

#
# --- Firmware header timestamp structure ------------------------------------
#
        .set    ft_size,8       # size of timestamp                  2<w>
#
# --- Firmware Load information structure ------------------------------------
#
#        .set    fl_size,24      # size of load information           6<w>
#
# --- Firmware header structure ----------------------------------------------
#
#        .set    fh_rsvd0,0                      # Space for branch instruction   8<w>
                                                #                                *****
#        .set    fh_magicNumber,fh_rsvd0+32      # Indication of a valid header    <w>
#        .set    fh_rsvd1,fh_magicNumber+4       # pad                             <w>
#        .set    fh_productID,fh_rsvd1+4         # Product Indentifier             <w>
#        .set    fh_targetID,fh_productID+4      # Target ID (CCB/FE/BE, Boot/Diag/Proc) <w>
                                                #                                *****
#        .set    fh_revision,fh_targetID+4       # Firmware Revision               <w>
        .set    fh_revision,48                  # Firmware Revision               <w>
#        .set    fh_revCount,fh_revision+4       # Firmware Revision Counter       <w>
#        .set    fh_buildID,fh_revCount+4        # Who / where firmware was built  <w>
#        .set    fh_vendorID,fh_buildID+4        # If customer / vendor unique firmware <w>
                                                #                                *****
#        .set    fh_timeStamp,fh_vendorID+4      # Time Firmware was built        2<w>
#        .set    fh_rsvd2,fh_timeStamp+ft_size   # pad                             <w>
#        .set    fh_burnSequence,fh_rsvd2+4      # Flash burn sequence number.     <w>
                                                #                                *****
#        .set    fh_loadID,fh_burnSequence+4     # Firmware load information      6<w>
#        .set    fh_rsvd3,fh_loadID+fl_size      # Pad out to 32 words            5<w>
#        .set    fh_hdrCksum,fh_rsvd3+(5*4)      # Checksum of this header only    <w>
#                                                                                *****
#        .set    fh_size,fh_hdrCksum+4           # size of firmware header
        .set    fh_size,128                     # size of firmware header
#
#*****************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
