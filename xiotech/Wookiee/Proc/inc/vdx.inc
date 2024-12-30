# $Id: vdx.inc 145407 2010-08-10 17:26:37Z m4 $
#**********************************************************************
#
#  NAME: vdx.inc (Virtual Device indeX)
#
#  DESCRIPTION:
#
#       To provide a common means of defining the Virtual Device indeX
#       (VDX) structure.
#
#  Copyright (c) 1996-2000 XIOtech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- Begin basic structure -------------------------------------------
#
        .set    vx_ecnt,-4              # Entry count               <s>
#
#       reserved 2
#
        .set    vx_vdd,0                # Pointer to VDD            <w>
#
# --- End extended structure ------------------------------------------
#
        .set    vdxsiz,vx_vdd+(4*MAXVIRTUALS) # Size of VDX
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
