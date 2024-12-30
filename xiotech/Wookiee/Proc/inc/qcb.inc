# $Id: qcb.inc 145407 2010-08-10 17:26:37Z m4 $
#**********************************************************************
#
#  NAME: qcb.inc (Queue Control Block)
#
#  DESCRIPTION:
#
#       To provide a common definition of the Queue Control Block
#       structure.
#
#  Copyright (c) 1996-2000 XIOtech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- Begin structure -------------------------------------------------
#                                                                  *****
        .set    qc_begin,0              # Org of circular que ptr   <w>
        .set    qc_in,qc_begin+4        # Insert pointer            <w>
        .set    qc_out,qc_in+4          # Remove pointer            <w>
        .set    qc_end,qc_out+4         # End of circular que + 1   <w>
#                                                                  *****
# --- End structure ---------------------------------------------------
#
        .set    qcb_size,qc_end+4       # Size of QCB
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
