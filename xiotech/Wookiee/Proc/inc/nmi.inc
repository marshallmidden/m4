# $Id: nmi.inc 145407 2010-08-10 17:26:37Z m4 $
#**********************************************************************
#
#  NAME: nmi.inc (Non-Maskable Interrupt)
#
#  DESCRIPTION:
#
#       Provides a common structure definition for the NMI stats
#       segment of the NVRAM.
#
#  Copyright (c) 1996-2002 XIOtech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- Begin structure -------------------------------------------------
#                                                                               *****
        .set    nmi_ecc_init,0                   # All ECC init errors           <w>
#
# ---   reserved 12
#
#                                                                               *****
        .set    nmi_ecc_imce,nmi_ecc_init+16     # All ECC init MCE errors       <w>
        .set    nmi_ecc_isgl,nmi_ecc_imce+4      # Single-bit init ECC errors    <w>
        .set    nmi_ecc_imul,nmi_ecc_isgl+4      # Multi-bit init ECC errors     <w>
        .set    nmi_ecc_inot,nmi_ecc_imul+4      # Non-logged init ECC errors    <w>
#                                                                               *****
        .set    nmi_ecc_ielog0,nmi_ecc_inot+4    # Last init ELOG0 value         <w>
        .set    nmi_ecc_ielog1,nmi_ecc_ielog0+4  # Last init ELOG1 value         <w>
        .set    nmi_ecc_iecar0,nmi_ecc_ielog1+4  # Last init ECAR0 value         <w>
        .set    nmi_ecc_iecar1,nmi_ecc_iecar0+4  # Last init ECAR1 value         <w>
#                                                                               *****
        .set    nmi_cntl_sn,nmi_ecc_iecar1+4     # Proc board serial number      <w>
        .set    nmi_unit_sn,nmi_cntl_sn+4        # Proc board serial number      <w>
#
# --- reserved 8
#
#                                                                               *****
        .set    nmi_ecc_elog0,nmi_unit_sn+12     # Last ELOG0 value              <w>
        .set    nmi_ecc_elog1,nmi_ecc_elog0+4    # Last ELOG1 value              <w>
        .set    nmi_ecc_ecar0,nmi_ecc_elog1+4    # Last ECAR0 value              <w>
        .set    nmi_ecc_ecar1,nmi_ecc_ecar0+4    # Last ECAR1 value              <w>
#                                                                               *****
#
# --- reserved 16
#
#                                                                               *****
        .set    nmi_fwflt_cnt,nmi_ecc_ecar1+20   #                               <w>
        .set    nmi_dgflt_cnt,nmi_fwflt_cnt+4    #                               <w>
#
# ---- reserved 8
#
#                                                                               *****
        .set    nmi_brk_cnt,nmi_dgflt_cnt+12     #                               <w>
        .set    nmi_unexp_cnt,nmi_brk_cnt+4      #                               <w>
#
# ---- reserved 8
#
#                                                                               *****
        .set    nmi_mce_cnt,nmi_unexp_cnt+12     # ECC error count               <w>
        .set    nmi_pae_cnt,nmi_mce_cnt+4        #                               <w>
        .set    nmi_sae_cnt,nmi_pae_cnt+4        #                               <w>
        .set    nmi_pbie_cnt,nmi_sae_cnt+4       #                               <w>
#                                                                               *****
        .set    nmi_sbe_cnt,nmi_pbie_cnt+4       #                               <w>
        .set    nmi_dmac0e_cnt,nmi_sbe_cnt+4     #                               <w>
        .set    nmi_dmac1e_cnt,nmi_dmac0e_cnt+4  #                               <w>
        .set    nmi_dmac2e_cnt,nmi_dmac1e_cnt+4  #                               <w>
#                                                                               *****
        .set    nmi_mui_cnt,nmi_dmac2e_cnt+4     #                               <w>
        .set    nmi_eni_cnt,nmi_mui_cnt+4        #                               <w>
        .set    nmi_aaue_cnt,nmi_eni_cnt+4       #                               <w>
        .set    nmi_biue_cnt,nmi_aaue_cnt+4      #                               <w>
#                                                                               *****
        .set    nmi_ecc_sgl,nmi_biue_cnt+4       # Single-bit ECC error count    <w>
        .set    nmi_ecc_mul,nmi_ecc_sgl+4        # Multi-bit ECC error count     <w>
        .set    nmi_ecc_not,nmi_ecc_mul+4        # Non-logged ECC error count    <w>
        .set    nmi_cnt,nmi_ecc_not+4            # Total NMI count               <w>
#
# --- End structure ---------------------------------------------------
#
        .set    nmisiz,nmi_cnt+4                 # Size of structure
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
