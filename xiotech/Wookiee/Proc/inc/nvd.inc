# $Id: nvd.inc 145407 2010-08-10 17:26:37Z m4 $
#**********************************************************************
#
#  NAME:  nvd.inc (Non-Volatile Diagnostic Data storage)
#
#  DESCRIPTION:
#
#       To show the format of diagnostic data located in Part 5 of
#       NVRAM.  This data is stored whenever an errortrap occurs and
#       will aid in diagnosis of the failure.
#
#  Copyright (c) 1996-2007 Xiotech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- Equates ---------------------------------------------------------
#
#       Part 5 NVRAM Entries
#
        .set    nvd_consts,0             # Constants
        .set    nvd_ii,nvd_consts+1      # Internal information
        .set    nvd_fwhr,nvd_ii+1        # Runtime firmware header
        .set    nvd_fwhb,nvd_fwhr+1      # Boot firmware header
        .set    nvd_fwhd,nvd_fwhb+1      # Diag firmware header
        .set    nvd_frec,nvd_fwhd+1      # Flight recorder
        .set    nvd_mrp,nvd_frec+1       # MRP trace
        .set    nvd_deq,nvd_mrp+1        # Define exec que
        .set    nvd_dep,nvd_deq+1        # Define que PCB
        .set    nvd_cpcb,nvd_dep+1       # Currently running PCB
        .set    nvd_tgd,nvd_cpcb+1       # Target definition
# ISCSI_CODE
        .set    nvd_itgd,nvd_tgd+1       # I_TGD definition
        .set    nvd_tgt,nvd_itgd+1       # Target structure
# ISCSI_CODE
#        .set    nvd_tgt,nvd_tgd+1        # Target structure
        .set    nvd_lldmt_dir,nvd_tgt+1  # LLDMT directory
        .set    nvd_lldmt,nvd_lldmt_dir+1# LLDMT
        .set    nvd_dtmt,nvd_lldmt+1     # DTMT
        .set    nvd_tpmt,nvd_dtmt+1      # TPMT
        .set    nvd_mlmt,nvd_tpmt+1      # MLMT
        .set    nvd_lqcs,nvd_mlmt+1      # Link layer Q control structure
        .set    nvd_irqq0,nvd_lqcs+1     # ISP Request Queue 0
        .set    nvd_irpq0,nvd_irqq0+1    # ISP Response Queue 0
        .set    nvd_irqq1,nvd_irpq0+1    # ISP Request Queue 1
        .set    nvd_irpq1,nvd_irqq1+1    # ISP Response Queue 1
        .set    nvd_irqq2,nvd_irpq1+1    # ISP Request Queue 2
        .set    nvd_irpq2,nvd_irqq2+1    # ISP Response Queue 2
        .set    nvd_irqq3,nvd_irpq2+1    # ISP Request Queue 3
        .set    nvd_irpq3,nvd_irqq3+1    # ISP Response Queue 3
.ifdef BACKEND
        .set    nvd_beiram,nvd_irpq3+1   # BE IRAM
        .set    nvd_defrag,nvd_beiram+1  # Defrag trace
        .set    nvd_peq,nvd_defrag+1     # Physical exec que
        .set    nvd_pep,nvd_peq+1        # Physical exec PCB
        .set    nvd_pcq,nvd_pep+1        # Physical completion que
        .set    nvd_pcp,nvd_pcq+1        # Physical completion PCB
        .set    nvd_req,nvd_pcp+1        # Raid exec que
        .set    nvd_rep,nvd_req+1        # Raid exec PCB
        .set    nvd_r5eq,nvd_rep+1       # Raid 5 exec que
        .set    nvd_r5ep,nvd_r5eq+1      # Raid 5 exec PCB
        .set    nvd_veq,nvd_r5ep+1       # Virtual exec que
        .set    nvd_vep,nvd_veq+1        # Virtual exec PCB
        .set    nvd_rieq,nvd_vep+1       # Raid Init exec que
        .set    nvd_riep,nvd_rieq+1      # Raid Init exec PCB
        .set    nvd_fseq,nvd_riep+1      # File system exec que
        .set    nvd_fsep,nvd_fseq+1      # File system exec PCB
        .set    nvd_reeq,nvd_fsep+1      # Raid error exec que
        .set    nvd_reep,nvd_reeq+1      # Raid error exec PCB
        .set    nvd_vdd,nvd_reep+1       # VDDs
        .set    nvd_ipcb,nvd_vdd+1       # Inquire PCB
        .set    nvd_ipdd,nvd_ipcb+1      # Inquire PDD
.endif
.ifdef FRONTEND
        .set    nvd_feiram,nvd_irpq3+1   # FE IRAM
        .set    nvd_tmt,nvd_feiram+1     # TMT
        .set    nvd_tlmt,nvd_tmt+1       # TLMT
        .set    nvd_ismt,nvd_tlmt+1      # ISMT
        .set    nvd_ltmt,nvd_ismt+1      # LTMT
        .set    nvd_lsmt,nvd_ltmt+1      # LSMT
        .set    nvd_cimt_dir,nvd_lsmt+1  # CIMT directory
        .set    nvd_cimt,nvd_cimt_dir+1  # CIMTs
        .set    nvd_trc,nvd_cimt+1       # Incoming trace log
        .set    nvd_imt_al,nvd_trc+1     # IMTs
        .set    nvd_ilmt,nvd_imt_al+1    # ILMTs
        .set    nvd_vdmt,nvd_ilmt+1      # VDMTs
        .set    nvd_sdd,nvd_vdmt+1       # SDDs
        .set    nvd_lvm,nvd_sdd+1        # LVMs
        .set    nvd_ilvm,nvd_lvm+1       # Invisible LVMs
        .set    nvd_vcd,nvd_ilvm+1       # VCDs
        .set    nvd_sdbd,nvd_vcd+1       # Server database directory
        .set    nvd_sdb,nvd_sdbd+1       # Server database
        .set    nvd_cqht,nvd_sdb+1       # Cache queue, head and tail
        .set    nvd_cqpcb,nvd_cqht+1     # Cache PCB
        .set    nvd_ciqht,nvd_cqpcb+1    # Cache I/O queue, head & tail
        .set    nvd_ciqpcb,nvd_ciqht+1   # Cache I/O PCB
        .set    nvd_icimt_dir,nvd_ciqpcb+1  # ICIMT directory
        .set    nvd_icimt,nvd_icimt_dir+1# ICIMT
        .set    nvd_iscsi,nvd_icimt+1    # ISCSI Stats
        .set    nvd_iconn,nvd_iscsi+1    # ISCSI Stats
        .set    nvd_idd,nvd_iconn+1      # IDDs
        .set    nvd_itrc,nvd_idd+1       # Initiator trace log
.endif
#
#       Part 5 NVRAM Entry Label Size
#
        .set    nvd_lab_size,8
#
#       Part 5 Header Label size
#
        .set    nv5h_lab_size,8
#
# --- Begin Part 5 NVRAM header structure -----------------------------
#
        .set    nv5h_id,0               # ID str, Diag BE=BE,      8<b>
                                        #         Diag FE=FE
        .set    nv5h_len,nv5h_id+8      # Length of nvram pt 5 area <w>
        .set    nv5h_crc,nv5h_len+4     # CRC of pt 5 area          <w>
        .set    nv5h_time,nv5h_crc+4    # Timestamp                8<b>
#       reserved 8
#
        .set    nv5h_size,nv5h_time+16  # Size of part 5 header
#
# --- Begin Part 5 entry structure ------------------------------------
#
        .set    nv5e_id,0               # ID of data               8<b>
        .set    nv5e_addr,nv5e_id+8     # Address of data           <w>
        .set    nv5e_len,nv5e_addr+4    # Length of data            <w>
        .set    nv5e_size,nv5e_len+4    # Size of part 5 entry
#
# --- End structure ---------------------------------------------------
#
#
# --- Begin Part 5 Constants data entry structure ---------------------
#
        .set    nv5c_tgt,0              # Max # of targets          <w>
        .set    nv5c_isp,nv5c_tgt+4     # Max # of ISPs             <w>
        .set    nv5c_virt,nv5c_isp+4    # Max # of vdisks           <w>
        .set    nv5c_cimt,nv5c_virt+4   # Max # of CIMTs            <w>
#                                                                  *****
        .set    nv5c_lun,nv5c_cimt+4    # Max # of LUNs             <w>
        .set    nv5c_serv,nv5c_lun+4    # Max # of servers          <w>
        .set    nv5c_icimt,nv5c_serv+4  # Max # of ICIMTs           <w>
        .set    nv5c_chn,nv5c_icimt+4   # Max # of channels         <w>
#                                                                  *****
        .set    nv5c_raid,nv5c_chn+4    # Max # of raids            <w>
        .set    nv5c_dev,nv5c_raid+4    # Max # of devices          <w>
        .set    nv5c_vrp,nv5c_dev+4     # Max # of VRPs             <w>
        .set    nv5c_drive,nv5c_vrp+4   # Max # of drives           <w>
#                                                                  *****
        .set    nv5c_if,nv5c_drive+4    # Max # of interfaces       <w>
        .set    nv5c_ctrl,nv5c_if+4     # Max # of controllers      <w>
        .set    nv5c_ses,nv5c_ctrl+4    # Max # of SESs             <w>
        .set    nv5c_lid,nv5c_ses+4     # Max # of LIDs             <w>
#                                                                  *****
        .set    nv5c_size,nv5c_lid+4    # Size of constants entry
#
# --- End structure ---------------------------------------------------
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
