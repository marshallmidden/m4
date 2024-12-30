# $Id: vqd.inc 145407 2010-08-10 17:26:37Z m4 $
#***********************************************************************
#
#  NAME: vqd.inc (Virtual inQuiry Data)
#
#  DESCRIPTION:
#
#       To provide a common means of defining the data returned from
#       a virtual inquiry operation.
#
#  Copyright (c) 1996-1999 XIOtech Corporation.  All rights reserved.
#
#***********************************************************************
#
# --- Begin VQD structure ----------------------------------------------
#                                                                  *****
        .set    vq_vers,0               # Version                   <s>
        .set    vq_rev,vq_vers+2        # Revision                  <s>
        .set    vq_secsize,vq_rev+2     # Sector size               <w>
        .set    vq_maxio,vq_secsize+4   # Maximum sector count      <w>
        .set    vq_vcnt,vq_maxio+4      # Maximum virtual devices   <w>
#                                                                  *****
        .set    vq_capacity,vq_vcnt+4   #                MAXVIRTUALS<l>
                                        # Capacity of virtual device
                                        #  capacity of 0 indicate a
                                        #  corresponding non-
                                        #  existent device
#                                                                  *****
# --- End structure ----------------------------------------------------
#
        .set    vqsize,vq_capacity+(MAXVIRTUALS*8)   # Size of VQD
#
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
