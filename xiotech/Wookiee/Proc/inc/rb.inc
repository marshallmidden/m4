# $Id: rb.inc 145407 2010-08-10 17:26:37Z m4 $
#**********************************************************************
#
#  NAME: rb.inc
#
#  DESCRIPTION:
#
#       To provide a common means of defining elements for a red-black
#       interval tree structure.  Each element is of size <rbesiz>.
#
#  Copyright (c) 1996-2000 XIOtech Corporation.  All rights reserved.
#
#******************************************************************************
#
# --- Equates -----------------------------------------------------------------
#
        .set    BLACK,0                 # Black Attribute
        .set    RED,1                   # Red Attribute
#
# --- RB Element Structure definition -----------------------------------------
#
        .set    rbkey,0                 # Key value                  <l>
        .set    rbcleft,rbkey+8         # Child pointer left         <w>
        .set    rbcright,rbcleft+4      # Child pointer right        <w>
#                                                                   *****
        .set    rbparent,rbcright+4     # Parent node pointer        <w>
        .set    rbcolor,rbparent+4      # Color attribute            <b>
#
#     Reserved 3
#
        .set    rbkeym,rbcolor+4        # Key endpoint               <l>
#                                                                   *****
        .set    rbnodem,rbkeym+8        # Node endpoint              <l>
#
        .set    rbfthd,rbnodem+8        # Forward thread pointer     <w>
        .set    rbbthd,rbfthd+4         # Backward thread pointer    <w>
#                                                                   *****
        .set    rbdpoint,rbbthd+4       # Data payload pointer       <w>
#
# --- End structure
#
        .set    rbesize,rbdpoint+4      # Sizeof tree element
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
