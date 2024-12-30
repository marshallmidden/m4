# $Id: ftdt.inc 145407 2010-08-10 17:26:37Z m4 $
#*****************************************************************************
#
#  NAME: ftdt.inc  (Foreign Target Descriptor Table)
#
#  DESCRIPTION:
#
#       This module defines the Foreign Target Descriptor Table used to pass
#       information about an identified Foreign Target between the Link-
#       level driver and the Data-link manager.
#
#  Copyright (c) 1999-2000 - XIOtech Corporation.   All rights reserved.
#
#*****************************************************************************
#
# --- Equates ----------------------------------------------------------------
#
#
# --- Data structure ---------------------------------------------------------
#
        .set    ftdt_snlen,0            # Serial number length            <b>
        .set    ftdt_luns,ftdt_snlen+1  # # LUNs                          <b>
        .set    ftdt_dtype,ftdt_luns+1  # Device type code                <b>
#
# --- Reserved 1
#
        .set    ftdt_dlmid,ftdt_dtype+2 # Data-link manager session ID    <w>
        .set    ftdt_nwwn,ftdt_dlmid+4  # Node WWN                       8<b>
        .set    ftdt_pwwn,ftdt_nwwn+8   # Port WWN                       8<b>
        .set    ftdt_alpa,ftdt_pwwn+8   # Assigned AL-PA                 4<b>
        .set    ftdt_venid,ftdt_alpa+4  # Vendor ID (from INQUIRY)       8<b>
        .set    ftdt_prid,ftdt_venid+8  # Product ID (from INQUIRY)     16<b>
        .set    ftdt_version,ftdt_prid+16 # product revision number      4<b>
        .set    ftdt_sn,ftdt_version+4  # Serial number                 16<b>
#
        .set    ftdt_size,ftdt_sn+16    # size of FTDT
#
# --- End of data structure -------------------------------------------------
#*****************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
