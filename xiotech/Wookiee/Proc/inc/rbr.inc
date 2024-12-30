# $Id: rbr.inc 145407 2010-08-10 17:26:37Z m4 $
#**********************************************************************
#
#  NAME: rbr.inc (ReBuild Record)
#
#  DESCRIPTION:
#
#       To provide a common means of defining the Rebuild Record
#       (RBR) structure.
#
#  Copyright (c) 1996-2002 XIOtech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- rb_status definitions -------------------------------------------
#
        .set    strbnorm,0                  # Normal (either waiting to
                                            #  rebuild or rebuilding)
        .set    strbdelay,1                 # Rebuild delayed
        .set    strbcancel,2                # Stop using this RBR
#
#
# --- Begin structure -------------------------------------------------
#                                                                      *****
        .set    rbr_psd,0                   # PSD to rebuild            <w>
#
        .set    rbr_rid,rbr_psd+4           # RAID ID                   <s>
        .set    rbr_status,rbr_rid+2        # Rebuild status            <b>
        .set    rbr_delaytime,rbr_status+1  # Delay time remaining      <b>
#
        .set    rbr_rbremain,rbr_delaytime+1# Remaining blocks to rebld <l>
#                                                                      *****
        .set    rbr_next,rbr_rbremain+8     # Pointer to next record    <w>
        .set    rbr_pdd,rbr_next+4          # Pointer to PDD            <w>
        .set    rbr_delaylogtime,rbr_pdd+4  # Seconds delayed before log<w>
#
#       reserved 4                                                      <W>
#                                                                      *****
#
# --- End structure ---------------------------------------------------
#
        .set    rbrsiz,rbr_delaylogtime+8   # Size of RBR
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
