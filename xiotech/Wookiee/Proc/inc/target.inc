# $Id: target.inc 145407 2010-08-10 17:26:37Z m4 $
#******************************************************************************
#
#  NAME: target.inc
#
#  PURPOSE:
#
#       To define WWN-to-physical interface structures.
#
#  Copyright (c) 1996 - 2004 XIOtech Corporation.  All rights reserved.
#
#******************************************************************************
#
# --- Equates -----------------------------------------------------------------
#
        .set    MAXTARG,31              # Maximum targets defined per interface
        .set    CTLTARG,0               # Target defined as control interface
        .set    NOCLUSTER,0xFFFF        # Indicates not target clustering
#
# --- Begin structure ---------------------------------------------------------
#
        .set    tarfthd,0               # Forward thread            <w>
        .set    tartid,tarfthd+4        # Target ID                 <s>
        .set    tarentry,tartid+2       # Entry number              <s>
        .set    taropt,tarentry+2       # Target options            <b>
#       reserved 1                                                  <b>
        .set    tarhaid,taropt+2        # Target Hard Assigned ID   <b>
#       reserved 1                                                  <b>
        .set    tarptn,tarhaid+2        # Target Port name          <l>
.if ISCSI_CODE
        .set    tipprifx,tarhaid+1      # IP Mask for iSCSI intf    <b>
        .set    tipaddr,tarhaid+2       # IP Addr for iSCSI intf    <w>
        .set    tipgw,ipaddr+4          # IP GE for iSCSI intf      <w>
.endif
        .set    tarndn,tarptn+8         # Target Node name          <l>
        .set    tarflags,tarndn+8       # FC4 registration flag     <w>
#                                                                  *****
        .set    tarvpid,tarflags+4      # Virtual port ID (LID)     <w>
        .set    tarportid,tarvpid+4     # Port ID                   <w>
#
# --- End structure -----------------------------------------------------------
#
        .set    tarsize,tarportid+4     # Sizeof structure
#
# --- Bit options for <taropt> and <tgd_opt>
#
        .set    tarhid,0                # 1 = Hard Loop ID, 0 = Soft Loop ID
        .set    tarena,1                # 1 = Enabled, 0 = Disabled
        .set    tarpalid,2              # 1 = Prev Asn ID (LIPA), 0 = get new ID
        .set    taricl,3                # 1 = ICL target
        .set    tariscsi,7              # 1 = iSCSI Target, 0 = FC Target
#
# --- TGDX table
#
        .set    tgx_ecnt,-4             # Num entries in table      <s>
#       reserved 2                                                  <s>
        .set    tgx_tgd,0               # Start of the entries      <w>
#
# --- End structure ---------------------------------------------------
#
        .set    tgxsiz,tgx_tgd+(4*MAXTARGETS) # Size of TGX
#
# --- TGD definitions -------------------------------------------------
#
        .set    tgd_tid,0               # Target ID                 <s>
        .set    tgd_port,tgd_tid+2      # port number               <b>
        .set    tgd_opt,tgd_port+1      # Options                   <b>
        .set    tgd_fcid,tgd_opt+1      # FC ID if hard ID          <b>
#       reserved 1                                                  <b>
        .set    tgd_lock,tgd_fcid+2     # Locked target indicator   <b>
#       reserved 1                                                  <b>
        .set    tgd_pname,tgd_lock+2    # WWN of port               <l>
.if ISCSI_CODE
        .set    ipprifx,tgd_lock+1      # IP Mask for iSCSI intf    <b>
        .set    ipaddr,tgd_lock+2       # IP Addr for iSCSI intf    <w>
        .set    ipgw,ipaddr+4           # IP GE for iSCSI intf      <w>
.endif
#                                                                  *****
        .set    tgd_nname,tgd_pname+8   # WWN of node               <l>
        .set    tgd_powner,tgd_nname+8  # S/N of preferred owner    <w>
        .set    tgd_owner,tgd_powner+4  # S/N of owner ctrl         <w>
#                                                                  *****
        .set    tgd_cluster,tgd_owner+4 # Clustered target number   <s>
#       reserved 2                                                  <s>
        .set    tgd_pport,tgd_cluster+4 # preferred port            <b>
        .set    tgd_aport,tgd_pport+1   # alternate port            <b>
#       reserved 2                                                 <2b>
        .set    tgd_imask,tgd_aport+3   # alternate port            <w>
        .set    tgd_itgd,tgd_imask+4    # alternate port            <w>
#
# --- End structure ---------------------------------------------------
#
        .set    tgdsiz,tgd_aport+11     # Size of TGD
        .set    itgdsiz,100             # Size of I_TGD
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
