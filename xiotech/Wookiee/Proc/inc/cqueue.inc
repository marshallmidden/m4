# $Id: cqueue.inc 145407 2010-08-10 17:26:37Z m4 $
#******************************************************************************
#
#  NAME: cqueue.inc
#
#  DESCRIPTION:
#
#       To provide write caching queue definitions.
#
#  Copyright (c) 2000 XIOtech Corporation.  All rights reserved.
#
#******************************************************************************
#
# --- Equates -----------------------------------------------------------------
#


#
# --- Begin structure ---------------------------------------------------------
#
        .set    chead,0                 # Head of ILT/Placeholder queue <w>
        .set    ctail,chead+4           # Tail of ILT/Placeholder queue <w>
        .set    cpcb,ctail+4            # Handler task PCB              <w>
#
#   Reserved 4
#
#
# --- End structure -----------------------------------------------------------
#
        .set    cqsize,cpcb+8           # Sizeof write cache queue structure
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
