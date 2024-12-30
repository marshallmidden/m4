# $Id: qpt.inc 145407 2010-08-10 17:26:37Z m4 $
#**********************************************************************
#
#  NAME: qpt.inc (Destination Copy Device Management Table)
#
#  DESCRIPTION:
#
#       This file defines the data structures and definitions to support
#       the queue processing table.
#
#  Copyright (c) 2001-2001 XIOtech Corporation.  All rights reserved.
#
#**********************************************************************
#
#       Queue Processing Table (qpt) Definition
#       ---------------------------------------------------------
#
# --- Begin QPT main structure -----------------------------------------------
#
#                                                               ******0x00****
        .set    qpt_qu,0                # selected queue address        <w>
        .set    qpt_cnt,qpt_qu+4        # number of active scr's        <b>
        .set    qpt_limit,qpt_cnt+1     # scr limit                     <b>
                                        # 2 spare                      2<b>

        .set    qptsize,qpt_limit+3     # size of main qpt structure
#
# --- End QPT main structure -------------------------------------------------
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
