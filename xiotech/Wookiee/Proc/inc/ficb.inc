# $Id: ficb.inc 145407 2010-08-10 17:26:37Z m4 $
#**********************************************************************
#
#  NAME: ficb.inc (Firmware Initialization Control Block)
#
#  DESCRIPTION:
#
#       To provide a means of conveying initialization information
#       between the POSTs and the operational firmware.
#
#  Copyright (c) 1996-2001 XIOtech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- Equates ---------------------------------------------------------
#
# --- Begin structure -------------------------------------------------
#                                                                  *****
        .set    fi_vcgid,0              # Virtual controller group  <w>
        .set    fi_cserial,fi_vcgid+4   # Controller serial number  <w>
        .set    fi_seq,fi_cserial+4     # Sequence number           <w>
        .set    fi_ccbipaddr,fi_seq+4   # IP address of CCB         <w>
#                                                                  *****
        .set    fi_mirrorpartner,fi_ccbipaddr+4 # Mirror Partner    <w>
#
#   Reserved 12 bytes                                              3<w>
#                                                                  *****
        .set    fi_vcgname,fi_mirrorpartner+16 # VCG name           <q>
#                                                                  *****
# --- End structure ---------------------------------------------------
#
        .set    fisiz,fi_vcgname+16
#*****************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
