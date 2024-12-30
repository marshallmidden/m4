# $Id: inqmag.inc 145407 2010-08-10 17:26:37Z m4 $
#*****************************************************************************
#
#  NAME: inqmag.inc  (MAGNITUDE link INQUIRY CDB definitions)
#
#  DESCRIPTION:
#
#       This module defines the data structures and equates associated
#       with using the INQUIRY CDB on MAGNITUDE link images.
#
#  Copyright (c) 1999 - 2000 XIOtech Corporation.   All rights reserved.
#
#*****************************************************************************
#
# --- Equates ----------------------------------------------------------------
#
# --- MAGNITUDE Information page definitions ---------------------------------
#
        .set    inqinfo,0xc0            # information page code
#
# --- Information page data structure ----------------------------------------
#
        .set    inqinfo_bnr,0           # Banner field                    <w>
        .set    inqinfo_sn,4            # System serial number            <w>
        .set    inqinfo_path,8          # Path #                          <b>
        .set    inqinfo_cl,9            # Assigned cluster #              <b>
        .set    inqinfo_vdcnt,10        # # VDisks                        <b>
        .set    inqinfo_flag1,11        # Flag byte #1                    <b>
                                        # Bit 7 =
                                        #     6 =
                                        #     5 =
                                        #     4 =
                                        #     3 =
                                        #     2 =
                                        #     1 =
                                        #     0 = 1=responder operating
                                        #         in target-only mode
        .set    inqinfo_name,12         # Assigned node name             8<b>
        .set    inqinfo_ip,20           # Assigned IP address            4<b>
        .set    inqinfo_alias,24        # Alias node serial number        <w>
        .set    inqinfo_spare,28        # spare (unused)                12<b>
#
        .set    inqinfo_size,40         # size of information page data
#
# --- End of information page data structure ---------------------------------
#*****************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
