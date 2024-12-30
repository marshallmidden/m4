# $Id: ilt.inc 122127 2010-01-06 14:04:36Z m4 $
        .eject
#**********************************************************************
#
#  NAME: ilt.inc (InterLayer Transport)
#
#  DESCRIPTION:
#
#       To provide a common means of defining the InterLayer Transport
#       (ILT) definitions.
#
#  Copyright (c) 1996-2001 XIOtech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- Equates ---------------------------------------------------------
#
.ifdef FRONTEND
        .set    ILTNEST1,10              # ILT nesting depth
.endif
.ifdef BACKEND
        .set    ILTNEST1,6              # ILT nesting depth
.endif
.ifdef BUILD_CCB
        .set    ILTNEST1,3              # ILT nesting depth
.endif

        .set    ILTNEST,ILTNEST1+1
#
# --- Begin structure -------------------------------------------------
#
#       (nested ILTNEST levels deep)
#                                                                  *****
        .set    il_fthd,0               # Forward thread            <w>
        .set    il_bthd,il_fthd+4       # Backward thread           <w>
        .set    il_pcb,il_bthd+4        # PCB of caller             <w>
        .set    il_misc,il_pcb          # Misc param 1 (alias)
        .set    il_cr,il_pcb+4          # Completion routine        <w>
#
# --- il_w0 is aligned on a 16-byte boundary
#
        .set    il_w0,il_cr+4           # Parameter 0               <w>
        .set    il_w1,il_w0+4           # Parameter 1               <w>
        .set    il_w2,il_w1+4           # Parameter 2               <w>
        .set    il_w3,il_w2+4           # Parameter 3               <w>
#                                                                  *****
        .set    il_w4,il_w3+4           # Parameter 4               <w>
        .set    il_w5,il_w4+4           # Parameter 5               <w>
        .set    il_w6,il_w5+4           # Parameter 6               <w>
        .set    il_w7,il_w6+4           # Parameter 7               <w>
#                                                                  *****
# --- End structure ---------------------------------------------------
#
        # For the Linux version, reserve an extra 4 bytes, which is only
        # used by the C code
        .set    ILTBIAS,(il_w7+8)        # Bias to next nest

        .set    iltsiz,ILTBIAS*ILTNEST   # Size of ILT
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
