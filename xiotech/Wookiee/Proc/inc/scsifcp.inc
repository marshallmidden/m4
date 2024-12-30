# $Id: scsifcp.inc 145407 2010-08-10 17:26:37Z m4 $
#*****************************************************************************
#
#  NAME: scsifcp.inc  (SCSI-FCP definitions)
#
#  DESCRIPTION:
#
#       Various SCSI-FCP related definitions.
#
#  Copyright (c) 1998 - 2000 XIOtech Corporarion.   All rights reserved.
#
#*****************************************************************************
#
# --- Equates ----------------------------------------------------------------
#
# --- Task management flag definitions
#
        .set    tf_abtaskset,1          # Abort task set flag
        .set    tf_cltaskset,2          # Clear task set flag
        .set    tf_warmreset,3          # Target warm reset flag
        .set    tf_lunreset,4           # LUN reset flag
        .set    tf_reset,5              # Target reset flag
        .set    tf_clearaca,6           # Clear ACA flag
        .set    tf_termtask,7           # Terminate task flag
#
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
