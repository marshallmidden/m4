# $Id: sglfc.inc 145407 2010-08-10 17:26:37Z m4 $
#******************************************************************************
#
#  NAME: sglfc.inc (Scatter/Gather List)
#
#  DESCRIPTION:
#
#       To provide a common means of defining the Scatter/Gather List
#       element structure for the FC-AL HAB.
#
#       VRPs from the FC-AL HAB have only one SG descriptor.  The word
#       following the SG descriptor is for use by the HAB in tracking
#       the ILT that the request is associated with.  The Magnitude
#       Host Interface will supply this address in all SRPs that
#       are derived from this request.
#
#  Copyright (c) 1996 - 2001 XIOtech Corporation.  All rights reserved.
#
#******************************************************************************
#
# --- Equates ---------------------------------------------------------
#
        .set    SG_LEN_MASK,0x00FFFFFF  # Mask off the direction byte from the
                                        #   length field
#
# --- Begin structure (header) ----------------------------------------
# The sgl structure is the same as sgl.inc -- except this one doesn't have sg_owners (reserved).
#
#--        .set    sg_scnt,0               # Descriptor count          <s>
#
#     reserved 1
#
#--        .set    sg_flag,sg_scnt+3       # Flag byte                 <b>
#--        .set    sg_size,sg_flag+1       # Size of SGL               <w>
#--        .set    sg_desc0,sg_size+4      # Descriptor 0 base
#
# --- End structure ---------------------------------------------------
#
#--        .set    sghdrsiz,sg_desc0       # Size of header
#
#
#
#
#
# --- Begin structure (descriptor) ------------------------------------
#
# The sgl descriptor matches sgl.inc, except that the sg_len field is only 3 bytes long,
# and the upper byte is the direction. This is an addition, does not hurt sgl.inc. Also sg_ilt is
# an addition.
#--        .set    sg_addr,0               # Address                   <w>
#--        .set    sg_len,sg_addr+4        # Length                   3<b>
        .set    sg_dir,sg_len+3         # Direction of transfer     <b>
            .set    SG_DIR_IN,0         #   Data coming into the controller
            .set    SG_DIR_OUT,1        #   Data going out of the controller
#
        .set    sg_ilt,sg_len+4         # ILT address for this request
                                        #  !! NOT INCLUDED IN STRUCTURE SIZE !!
#
# --- End structure ---------------------------------------------------
#
# Size matches sgl.inc.
#--        .set    sgdescsiz,sg_dir+1      # Size of SGL
#
# --- definitions for other possible descriptors
#
        .set    sg_desc1,sg_desc0+sgdescsiz # Descriptor 1 base
        .set    sg_desc2,sg_desc1+sgdescsiz # Descriptor 2 base
        .set    sg_desc3,sg_desc2+sgdescsiz # Descriptor 3 base
#
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
