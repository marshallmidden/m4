# $Id: ddrv.inc 145407 2010-08-10 17:26:37Z m4 $
#*****************************************************************************
#
#  NAME: ddrv.inc  (device driver definitions)
#
#  DESCRIPTION:
#
#       Various device driver related definitions.
#
#  Copyright (c) 1998 - 2000 XIOtech Corporarion.   All rights reserved.
#
#*****************************************************************************
#
# --- Equates ----------------------------------------------------------------
#
# --- Device driver event handler table index definitions
#
        .set    dd_cmdrcv,0             # SCSI command received
        .set    dd_abtask,4             # Abort task
        .set    dd_abtaskset,8          # Abort task set
        .set    dd_reset,12             # Reset received
        .set    dd_offline,16           # Interface offline (not operational)
        .set    dd_online,20            # Interface online (becoming
                                        #  operational)
        .set    dd_clearaca,24          # Clear ACA received
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
