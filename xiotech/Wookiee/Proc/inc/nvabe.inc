# $Id: nvabe.inc 145407 2010-08-10 17:26:37Z m4 $
#**********************************************************************
#
#  NAME: nvabe.inc (Non-Volatile Activity Back-End Functions)
#
#  DESCRIPTION:
#
#       To provide a common place for Back-End definitions
#
#  Copyright (c) 2003-2004 XIOtech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- Equates ---------------------------------------------------------
#
#  Mark RAID Scan input parameters
#
.set    SINGLE_RDD,         0           # Single RDD to mark
.set    ALL_RDDS,           1           # All RDDs owned by controller
.set    ALL_NOT_MIRROR_RDDS,2           # All RDDs Marked "Not Mirroring"
.set    LIST_RDDS,          3           # List of RDDs to mark
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
