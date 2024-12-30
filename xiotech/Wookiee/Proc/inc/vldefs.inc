# $Id: vldefs.inc 161368 2013-07-29 14:53:10Z marshall_midden $
#**********************************************************************
#
#  NAME: vldefs.inc (Virtual Link Definitions]
#
#  DESCRIPTION:
#
#       This file defines equates and data structures used to
#       support Virtual Links.
#
#  Copyright (c) 2000-2001 XIOtech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- Equates ---------------------------------------------------------
#
#***********************************************************************
#
# --- Storage Unit List data structures --------------------------------
#
        .set    vl_maxsur,512           # Maximum number of Storage Units
#
# --- Storage Unit List header data structure --------------------------
#                                                                  *****
        .set    vl_sulh_cnt,0           # number of storage units   <s>
                                        #  in list
#
# --- Reserved 14                                                 14<b>
#
        .set    vl_sulh_siz,vl_sulh_cnt+16 # size of list header
#
# --- End Storage Unit List header data structure ----------------------
#
# --- Storage Unit List record data structure --------------------------
#
        .set    vl_sulr_mac,0           # MAC address              8<b>
        .set    vl_sulr_name,vl_sulr_mac+8 # storage unit name    20<b>
        .set    vl_sulr_luns,vl_sulr_name+20 # # LUNs (VDisks)      <b>
        .set    vl_sulr_type,vl_sulr_luns+1 # storage controller type <b>
                                        # 0 = MAGNITUDE Link
                                        # 1 = Foreign Target
        .set    vl_sulr_cl,vl_sulr_type+1 # assigned cluster #      <b>
#
# --- Reserved 1                                                   1<b>
#
        .set    vl_sulr_beat,vl_sulr_cl+2 # heartbeat               <w>
        .set    vl_sulr_dtmt,vl_sulr_beat+4 # assoc. DTMT address   <w>
#
# --- Reserved 8                                                   8<b>
#
        .set    vl_sulr_siz,vl_sulr_dtmt+12 # size of Storage Unit List record
#
        .set    vl_sulsiz,((vl_maxsur*vl_sulr_siz)+vl_sulh_siz) # Space needed
#
# --- End Storage Unit List record data structure ----------------------
#
#***********************************************************************
#
# --- Storage Unit Data data structures --------------------------------
#
#
# NOTE: MAX FE LUNS change needed below. (bad comment too!)
        .set    vl_maxsud,64            # Maximum number of Storage Units
#
# --- Storage Unit Data header data structure --------------------------
#                                                                  *****
        .set    vl_sudh_cnt,0           # number of LUNs in data    <s>
        .set    vl_sudh_indx,vl_sudh_cnt+2 # assoc. storage unit    <s>
                                        #  index into storage unit list
# --- Reserved 12                                                 12<b>
        .set    vl_sudh_siz,vl_sudh_indx+14 # size of data header
#
# --- End Storage Unit Data header data structure ----------------------
#
# --- Storage Unit Data record data structure --------------------------
# NOTE: this precisely matches mvi_data structure.
        .set    vl_sudr_lun,0           # LUN #                     <s>
        .set    vl_sudr_raid,vl_sudr_lun+2 # RAID type (if from MAG)<b>
        .set    vl_sudr_bcl,vl_sudr_raid+1 # base MAG cluster #     <b>
        .set    vl_sudr_attr,vl_sudr_bcl+1 # VDisk attributes       <b>
# --- Reserved 3                                                   3<b>
        .set    vl_sudr_vsiz,vl_sudr_attr+4 # Disk size (sectors)   <l>
#                                                                 *******
        .set    vl_sudr_bsn,vl_sudr_vsiz+8 # base MAG serial #      <w>
        .set    vl_sudr_vcnt,vl_sudr_bsn+4 # VDisk # (if from MAG)  <s>
        .set    vl_sudr_vid,vl_sudr_vcnt+2 # base MAG vid           <s>
        .set    vl_sudr_srvcnt,vl_sudr_vid+2 # assigned server count <s>
        .set    vl_sudr_vlcnt,vl_sudr_srvcnt+2 # assigned VLink count <s>
# --- Reserved 4                                                   4<b>
#                                                                 *******
        .set    vl_sudr_devsn,vl_sudr_vlcnt+6 # Device serial #   52<b> or 13<w>
# --- Reserved 12                                                 12<b>
        .set    vl_sudr_siz,vl_sudr_devsn+64 # size of Storage Unit Data record
        .set    vl_sudsiz,((vl_maxsud*vl_sudr_siz)+vl_sudh_siz) # Space needed
#
# --- End Storage Unit Data record data structure ----------------------
#
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
