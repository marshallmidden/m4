# $Id: pci.inc 145407 2010-08-10 17:26:37Z m4 $
#******************************************************************************
#
#  NAME: pci.inc
#
#  PURPOSE:
#
#       PCI header definitions and other PCI-related equates.
#
#  Copyright (c) 1996 - 2006 Xiotech Corporation.  All rights reserved.
#
#******************************************************************************
#
# --- Equates -----------------------------------------------------------------
#
        .set    PCIMAXDEV,21            # Max PCI devices on secondary bus
        .set    PCIHDSIZE,4             # Max bytes for header retrieval
        .set    PCIOFFSET,5             # Offset to map IDSEL position
                                        #   to device position
#
# --- PCI structures ----------------------------------------------------------
#
#
# --- PCI data structure
#
# --- begin structure
#
        .set    pcihdr,0                # PCI header area
        .set    pcibaddr,pcihdr+PCIHDSIZE # base address register - memory
        .set    pcibaddm,pcibaddr+4     # RAM base address
#
# --- end structure
#
        .set    pcstructsz,pcibaddm+4   # sizeof PCI data struct
#
# --- PCI header structure
#
        .set    vidr,0                  # Vendor ID                  <s>
        .set    didr,2                  # Device ID                  <s>
        .set    pcmdr,4                 # Primary Command Register   <s>
        .set    psr,6                   # Primary Status Register    <s>
        .set    ridr,8                  # Revision ID register       <b>
        .set    ccr,9                   # Class Code Register        <3b>
        .set    clsr,0xC                # Cacheline Size Register    <b>
        .set    pltr,0xD                # Primary Latency Timer Reg  <b>
        .set    htr,0xE                 # Header Type Register       <b>
        .set    bist,0xF                # BIST register              <b>
        .set    piabar,0x10             # Primary Inb. Base Addr     <w>
        .set    pbnr,0x18               # Primary Bus Number         <b>
        .set    sbnr,0x19               # Secondary Bus Number       <b>
        .set    subBNR,0x1A             # Subordinate Bus Number     <b>
        .set    sltr,0x1B               # Secondary Latency Timer    <b>
        .set    iobr,0x1C               # I/O Base Register          <b>
        .set    iolr,0x1D               # I/O Limit Register         <b>
        .set    ssr,0x1E                # Secondary Status Register  <s>
        .set    mbr,0x20                # Memory Base Register       <s>
        .set    mlr,0x22                # Memory Limit Register      <s>
        .set    pmbr,0x24               # Prefetchable Mem Base Reg  <s>
        .set    pmlr,0x26               # Prefetchable Mem Lim Reg   <s>
        .set    svir,0x2C               # Subsystem Vendor ID Reg    <s>
        .set    sir,0x2E                # Subsystem ID Register      <s>
        .set    erbar,0x30              # Expansion ROM Base Addr    <w>
        .set    bsvir,0x34              # Bridge Subsys Vendor ID    <s>
        .set    bsir,0x36               # Bridge Subsys ID           <s>
        .set    bcr,0x3E                # Bridge Control Reg         <s>
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
