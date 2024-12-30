# $Id: masterfe.inc 89309 2009-06-16 18:26:40Z mdr $
#**********************************************************************
#
#  NAME: masterfe.inc
#
#  PURPOSE:
#
#       To provide a means of obtaining all of the additional FRONTEND
#       include files that are used to build the system (formerly FCAL HAB
#       version of master.inc).
#
#  Copyright (c) 2000-2008 XIOtech Corporation.
#
#**********************************************************************
#
# --- include files ---------------------------------------------------
#
        .include "ddrv.inc"      # device driver definitions
        .include "errlog.inc"    # Error logging
        .include "fmtunit.inc"   # FORMAT UNIT process definitions
        .include "gan.inc"       # get_all_next command response format
        .include "icimt.inc"     # Initiator Channel interface mgmt table
        .include "ilmt.inc"      # Initiator/LUN management table defs
        .include "inqmag.inc"    # MAGNITUDE link INQUIRY CDB definitions
        .include "ismt.inc"      # Initiator session management table
        .include "itrace.inc"    # Initiator trace definitions
        .include "lld.inc"       # Link-level Driver Definitions
        .include "lsmt.inc"      # Link-level Driver Session Mgmt Table
        .include "ltmt.inc"      # Link-level Driver Target Mgmt Table
        .include "rb.inc"        # RB definitions
        .include "scsifcp.inc"   # SCSI-FCP definitions
        .include "servdb.inc"    # FC-AL IEEE port names to server names
        .include "sglfc.inc"     # Scatter/Gather List
        .include "tlmt.inc"      # Target/LUN management table
        .include "tmt.inc"       # Target management table
        .include "trace.inc"     # Frontend Trace definitions
        .include "vdmt.inc"      # Virtual device management table
        .include "wcache.inc"    # Write caching related structures
        .include "xli.inc"       # translation level initiator table

#**********************************************************************
#
####
## Modelines:
## Local Variables:
## tab-width: 4
## indent-tabs-mode: nil
## End:
## vi:sw=4 ts=4 expandtab
