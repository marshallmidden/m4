# $Id: errlog.inc 145407 2010-08-10 17:26:37Z m4 $
#*****************************************************************************
#
#  NAME: errlog.inc  (Error log data structures & definitions)
#
#  DESCRIPTION:
#
#       This module defines the data structures and definitions
#       to support error logging in the CDRIVER & MAGDRVR modules.
#
#  Copyright (c) 1998 - 2000 XIOtech Corporarion.   All rights reserved.
#
#*****************************************************************************
#
# --- Equates ----------------------------------------------------------------
#
#
        .set    err_recsize,32          # error log record size
#
# --- Error log size definitions
#
        .set    elog_num,256            # # error log records
        .set    elog_trsize,0x2e0       # size of trace area in error log record
        .set    ELOGSIZE,elog_num*err_recsize # size of error log area (must be
                                        #  multiple of err_recsize)
#
# --- Error log flag bit definitions ---------------------------------------------
#
        .set    eflg_offl,0             # Offline event record
        .set    eflg_imno,1             # Immediate Notify event record
        .set    eflg_IOerr,2            # I/O completion error record
        .set    eflg_staterr,3          # Error ending status record
        .set    eflg_SENSE,4            # SENSE data record
#
# --- Common error log record data structure definitions
#
        .set    err_rtc,0               # error record type code        <w>
        .set    err_time,4              # event timestamp (secs.)       <w>
        .set    err_ci,8                # chip instance                 <s>
        .set    err_exid,10             # exchange ID                   <s>
        .set    err_free,12             # error log record free area  20<b>
#
#*****************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
