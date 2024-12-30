# $Id: magdt.inc 145407 2010-08-10 17:26:37Z m4 $
#*****************************************************************************
#
#  NAME: magdt.inc  (MAGNITUDE Descriptor Table)
#
#  DESCRIPTION:
#
#       This module defines the MAGNITUDE Descriptor Table used to pass
#       information about an identified MAGNITUDE node between the Link-
#       level driver and the Data-link manager.
#
#  Copyright (c) 1999-2001 - XIOtech Corporation.   All rights reserved.
#
#*****************************************************************************
#
# --- Equates ----------------------------------------------------------------
#
#
# --- Data structure ---------------------------------------------------------
#
        .set    magdt_dlmid,0           # Data-link manager session ID    <w>
        .set    magdt_nwwn,magdt_dlmid+4 # Node WWN                      8<b>
        .set    magdt_pwwn,magdt_nwwn+8 # Port WWN                       8<b>
        .set    magdt_alpa,magdt_pwwn+8 # Assigned AL-PA                 4<b>
        .set    magdt_sn,magdt_alpa+4   # MAG serial number               <w>
        .set    magdt_name,magdt_sn+4   # Assigned MAG name              8<b>
        .set    magdt_ip,magdt_name+8   # Assigned IP address            4<b>
        .set    magdt_path,magdt_ip+4   # path #                          <b>
        .set    magdt_cl,magdt_path+1   # Assigned cluster number         <b>
        .set    magdt_vds,magdt_cl+1    # # VDISKs                        <b>
        .set    magdt_flag1,magdt_vds+1 # Special INQUIRY flag byte #1    <b>
                                        # Bit 7 =
                                        #     6 =
                                        #     5 =
                                        #     4 =
                                        #     3 =
                                        #     2 =
                                        #     1 =
                                        #     0 = 1=node operating in
                                        #           target-only mode
        .set    magdt_alias,magdt_flag1+1 # Alias node serial number      <w>
        .set    magdt_size,magdt_alias+4 # size of MAGDT
#
# --- End of data structure -------------------------------------------------
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
