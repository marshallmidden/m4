# $Id: sdd.inc 145407 2010-08-10 17:26:37Z m4 $
#**********************************************************************
#
#  NAME: sdd.inc (Server Device Definition)
#
#  DESCRIPTION:
#
#       To provide a common means of providing a consistent definition
#       for a server and server statistics.
#
#  Copyright (c) 1996-2002 XIOtech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- Equates ---------------------------------------------------------
#
#       sd_status definitions
#
        .set    sdnonx,0                # Not installed
        .set    sdoper,1                # Installed and operational
        .set    sdinop,2                # Inoperative
#
#       sd_attrib bit definitions
#
        .set    sdunmanaged,0           # Server is unmanaged
        .set    sdseltarget,1           # Select target in this sdd for mapping
.if ISCSI_CODE
        .set    sdiscsi,2               # Server is an iSCSI Initiator
.endif
        .set    sdxio,30                # Server is XIOtech Controller
        .set    sddefault,31            # Server has default LUN mappings
#
# --- Begin structure -------------------------------------------------
#                                                                  *****
        .set    sd_sid,0                # Server ID                 <s>
        .set    sd_nluns,sd_sid+2       # Number of LUNs for server <s>
#
        .set    sd_tid,sd_nluns+2       # Target ID number          <s>
        .set    sd_status,sd_tid+2      # Server status             <b>
        .set    sd_pri,sd_status+1      # HAB priority              <b>
        .set    sd_attrib,sd_pri+1      # Server attributes         <w>
        .set    sd_session,sd_attrib+4  # Session identifier        <w>
#                                                                  *****
        .set    sd_reqcnt,sd_session+4  # Request count             <l>
        .set    sd_lsid,sd_reqcnt+8     # Linked SID nt             <s>
#
#       reserved 2
#
        .set    sd_owner,sd_lsid+4      # Serial num of owner ctrl  <w>
#                                                                  *****
        .set    sd_wwn,sd_owner+4       # World Wide Name of server <l>
#
#       reserved 8
#                                                                  *****
        .set    sd_name,sd_wwn+16       # Server name               <q>
#                                                                  *****
.if ISCSI_CODE
        .set    sd_iname,sd_name+16     # iSCSI Server name       <256b>
.endif
#                                                                  *****
#
# --- do not change the order of these.  they must be kept in this order.
#
        .set    sd_lvm,sd_iname+256     # LUN mappings              <w>
        .set    sd_ilvm,sd_lvm+4        # Invisible LUN mappings    <w>
#                                                                  *****
# --- End structure ---------------------------------------------------
#
        .set    sddsiz,sd_ilvm+4        # Size of sdd structure
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
