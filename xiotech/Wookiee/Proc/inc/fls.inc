# $Id: fls.inc 145407 2010-08-10 17:26:37Z m4 $
#*****************************************************************************
#
#  NAME: fls.inc  (FCAL Link Status)
#
#  DESCRIPTION:
#
#       To provide a data structure to return the link status
#       from a specific FC loop ID.
#
#  Copyright (c) 1998 - 2000 XIOtech Corporation.   All rights reserved.
#
#*****************************************************************************
#
# --- Begin Link Status structure --------------------------------------------
#
        .set    fl_lifcnt,0                 # Link Failure Count            <w>
        .set    fl_lsscnt,fl_lifcnt+4       # Loss of Sync Count            <w>
        .set    fl_lsgcnt,fl_lsscnt+4       # Loss of Signal Count          <w>
        .set    fl_pspec,fl_lsgcnt+4        # Primitive Seq error count     <w>
#                                                                       *******
        .set    fl_ivtqc,fl_pspec+4         # Inv. Xmission Word Count      <w>
        .set    fl_ivcrc,fl_ivtqc+4         # Invalid CRC count             <w>

        .set    flsize,fl_ivcrc+4           # Sizeof link status
#
#*****************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
