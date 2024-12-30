# $Id: lvm.inc 145407 2010-08-10 17:26:37Z m4 $
#**********************************************************************
#
#  NAME: lvm.inc (LUN to VDisk mapping)
#
#  DESCRIPTION:
#
#       To provide a common means of providing a consistent definition
#       for a server's LUN to Virtual Disk mapping.
#
#  Copyright (c) 1996-2000 XIOtech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- Begin structure -------------------------------------------------
#                                                                  *****
        .set    lv_nlvm,0               # Next LVM in the hash list <w>
#
        .set    lv_lun,lv_nlvm+4        # LUN number                <s>
        .set    lv_vid,lv_lun+2         # VID associated with LUN   <s>
#
#       reserved 8
#                                                                  *****
# --- End structure ---------------------------------------------------
#
        .set    lvsiz,lv_vid+10         # Size of LVM structure
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
