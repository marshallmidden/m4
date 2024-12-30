# $Id: ci.inc 145407 2010-08-10 17:26:37Z m4 $
#**********************************************************************
#
#  NAME: ci.inc (Cache Information)
#
#  DESCRIPTION:
#
#       To provide a common means of providing a consistent definition
#       for cache statistics.
#
#  Copyright (c) 1996-2007 Xiotech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- Equates ---------------------------------------------------------
#
#        .set    C_NCWBP_MAP,8           # 4K non-cached Write Buffer is
                                        #  equivalent to 16 bytes of proxy
                                        #  buffer
#
#       ca_status definitions
#
        .set    ca_ena,0                # bit 0 = 0 Global Cache Disabled
                                        #       = 1 Global Cache Enabled
        .set    ca_dis_ip,1             # bit 1 = 0 No disable in progress
                                        #       = 1 Global Cache Disable in Prog
        .set    ca_ena_pend,2           # bit 2 = 0 No enable pending
                                        #       = 1 Cache Enable is pending
        .set    ca_nwaymirror,3         # bit 3 = 0 1-way mirror to this ctrl BE
                                        #       = 1 N-way mirror to other ctrl
        .set    ca_mirrorbroken,4       # bit 4 = 0 N-way mirror working
                                        #       = 1 N-way mirror broken
        .set    ca_error,5              # bit 5 = 0 No errors detected
                                        #       = 1 Errors detected using Cache
        .set    ca_shutdown,6           # bit 6 = 0 Cache not shutdown
                                        #       = 1 Cache shutdown
        .set    ca_halt_background,7    # bit 7 = 0 Background Flush normal
                                        #       = 1 No Background Flush allowed
#
        .set    CA_NO_MORE_DATA,0x72    # Bits = 6, 5, 4, and 1 should not allow
                                        #  more data to come into cache for
                                        #  any VID
#
#       ca_status1 definitions - bits used in disabling Write Cache
#
        .set    ca_restore_data,0       # b0 = 0 WC Data from NV not Restored
                                        #    = 1 WC Data from NV is Restored
                                        # b1-b7 Reserved

#
#       ca_status2 definitions - bits used in disabling Write Cache
#
        .set    ca_my_battery,0         # b0 = 0 My Battery is Good
                                        #    = 1 My Battery is Bad
        .set    ca_mirror_battery,1     # b1 = 0 Mirror Batt is Good (Mine/MP)
                                        #    = 1 Mirror Batt is Bad (Mine or MP)
                                        # b2-b6 Reserved
        .set    ca_temp_disable,7       # b7 = 0 Normal Write Caching
                                        #    = 1 Temp Disable WC
#
#       Battery Status definitions
#
        .set    ca_bat_good,0           # battery good
        .set    ca_bat_low,1            # battery low
#
# --- Begin basic structure -------------------------------------------
#
# !!! NOTE: IF THIS STRUCTURE CHANGES, CCB mrp.h MUST ALSO CHANGE !!!!!!!!!
#                                                                  *****
        .set    ca_status,0             # Cache status              <b>
        .set    ca_status1,ca_status+1  # Cache status 1            <b>
        .set    ca_status2,ca_status+2  # Cache status 2            <b>
        .set    ca_stopcnt,ca_status2+1 # Stop I/O Count            <b>
        .set    ca_size,ca_stopcnt+1    # Cache size                <w>
        .set    ca_maxcwr,ca_size+4     # Maximum cached write      <w>
        .set    ca_maxsgl,ca_maxcwr+4   # Maximum num of SGLs/op    <w>
#                                                                  *****
        .set    ca_numTags,ca_maxsgl+4  # Total number of tags      <w>
        .set    ca_tagsDirty,ca_numTags+4 # Current num of tags dirty <w>
        .set    ca_tagsResident,ca_tagsDirty+4 # Current # of tags resident <w>
        .set    ca_tagsFree,ca_tagsResident+4 # Current num of tags free <w>
#                                                                  *****
        .set    ca_tagsFlushIP,ca_tagsFree+4 # Current # tags flush in progr <w>
        .set    ca_numBlks,ca_tagsFlushIP+4 # Total number of blocks   <w>
        .set    ca_blocksDirty,ca_numBlks+4 # Current num of dirty blocks <w>
        .set    ca_blocksResident,ca_blocksDirty+4 # Current # of blocks res <w>
#                                                                  *****
        .set    ca_blocksFree,ca_blocksResident+4 # Current # of blocks free <w>
        .set    ca_blocksFlushIP,ca_blocksFree+4 # Num blocks flush in prog <w>
#
#       reserved 8
#
#                                                                  *****
#
# --- End basic structure ---------------------------------------------
#
# CAUTION *** CAUTION *** CAUTION *** CAUTION *** CAUTION *** CAUTION *** CAUTION
#
# Keep this value quad aligned in order for the statistic collection to
# function correctly.
#
        .set    casiz,ca_blocksFlushIP+12 # Size of Cache Info
#
#**********************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
