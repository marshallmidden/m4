# $Id: xl.inc 145407 2010-08-10 17:26:37Z m4 $
#******************************************************************************
#
#  NAME: xl.inc
#
#  DESCRIPTION:
#
#       To provide definitions for Translation Layer routines.
#
#  Copyright (c) 1996 - 2000 XIOtech Corporation.  All rights reserved.
#
#******************************************************************************
#
# --- Equates -----------------------------------------------------------------
#
#
# --- <XL$receive_io> equates
#
        .set    scsicmd,0x10            # SCSI CDB received
        .set    immncmd,0x11            # Immediate Notify received
        .set    offlcmd,0x12            # Offline event
        .set    logfcmd,0x13            # initiator logoff
        .set    rinitcmd,0x14           # Reset and initialize port event
        .set    disvpcmd,0x15           # Disable virtual port
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
