# $Id: magdd.inc 161368 2013-07-29 14:53:10Z marshall_midden $
#*****************************************************************************
#
#  NAME: magdd.inc  (MAGNITUDE Device Database)
#
#  DESCRIPTION:
#       This module defines the MAGNITUDE Device Database used to pass
#       information about the devices located on an identified MAGNITUDE
#       node between the Link-level driver and the Data-link manager.
#
#  Copyright (c) 1999-2012 XIOtech Corporarion.   All rights reserved.
#
#*****************************************************************************
#
# --- MAGDD header data structure -------------------------------------------
        .set    magdd_vds,0             # # VDISKs defined                <b>
        .set    magdd_vdrecs,magdd_vds+1 # # VDISK records defined        <b>
        .set    magdd_rsize,magdd_vdrecs+1 # VDISK record size            <b>
# --- Reserved 1
        .set    magdd_hdr,magdd_rsize+2 # size of MAGDD header
# --- End of MAGDD header data structure ------------------------------------
#
# --- VDISK record data structure -------------------------------------------
        .set    magdd_vdnum,0           # VDISK #                         <b>
        .set    magdd_type,magdd_vdnum+1 # RAID type                      <b>
        .set    magdd_secsz,magdd_type+1 # Sector size                    <s>
        .set    magdd_size,magdd_secsz+2 # Disk size (# sectors)          <w>       GT2TB see below
        .set    magdd_sn,magdd_size+4   # Base MAG serial number          <w>
        .set    magdd_cl,magdd_sn+4     # Base MAG cluster number         <b>
        .set    magdd_vd,magdd_cl+1     # Base MAG VDISK #                <b>
        .set    magdd_attr,magdd_vd+1   # Attributes                      <b>
# --- Reserved 1
        .set    magdd_snum,magdd_attr+2 # # servers assigned to VDisk     <s>
        .set    magdd_vlnum,magdd_snum+2 # # VLinks assigned to VDisk     <s>
        .set    magdd_vdname,magdd_vlnum+2 # VDisk name                  8<b>
        .set    magdd_recsz,magdd_vdname+8 # size of VDISK record
# --- VDISK GT2TB record data structure -------------------------------------------
        .set    magdd_size_GT2TB,magdd_secsz+2       # Disk size (# sectors)      <l>
        .set    magdd_sn_GT2TB,magdd_size_GT2TB+8    # Base MAG serial number     <w>
        .set    magdd_cl_GT2TB,magdd_sn_GT2TB+4      # Base MAG cluster number    <b>
        .set    magdd_vd_GT2TB,magdd_cl_GT2TB+1      # Base MAG VDISK #           <b>
        .set    magdd_attr_GT2TB,magdd_vd_GT2TB+1    # Attributes                 <b>
# --- Reserved 1
        .set    magdd_snum_GT2TB,magdd_attr_GT2TB+2  # # servers assigned to VDisk <s>
        .set    magdd_vlnum_GT2TB,magdd_snum_GT2TB+2 # # VLinks assigned to VDisk  <s>
        .set    magdd_vdname_GT2TB,magdd_vlnum_GT2TB+2 # VDisk name                8<b>
        .set    magdd_recsz_GT2TB,magdd_vdname_GT2TB+8 # size of VDISK record
# --- End of VDISK GT2TB record data structure ------------------------------------
#
# NOTE: MAX FE LUNS change needed below.
        .set    magdd_maxvds,64                    # max. # VDISKs supported
        .set    magdd_alloc,magdd_hdr+(magdd_recsz*magdd_maxvds)
                                                   # allocation for MAGDD
        .set    magdd_alloc_GT2TB,magdd_hdr+(magdd_recsz_GT2TB*magdd_maxvds)
                                                   # allocation for GT2TB MAGDD
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
