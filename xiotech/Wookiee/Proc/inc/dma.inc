# $Id: dma.inc 145407 2010-08-10 17:26:37Z m4 $
#******************************************************************************
#
#  NAME: dma.inc
#
#  DESCRIPTION:
#
#       To define DMA definitions for the 80960RP.
#
#  Copyright (c) 1996 - 1998 XIOtech Corporation.  All rights reserved.
#
#******************************************************************************
#
# --- Equates ---------------------------------------------------------
#
        .set    DMA0BLK,256             # DMA 0 allocation
        .set    DMA1BLK,256             # DMA 1 allocation
        .set    dmaqent,8               # Sizeof DMA control queue
#
        .set    DMAQSZ,(dmaqent + 1) * dmacqsiz # sizeof DMA control queue
#
        .set    DMA0SIZE,DMA0BLK*dma_size # DMA 0 mem alloc size
        .set    DMA1SIZE,DMA1BLK*dma_size # DMA 1 mem alloc size
#
# --- Relative offsets for DMA control registers
#
        .set    DMA0,0
        .set    DMA1,0x40
        .set    DMA2,0x80
#
# --- DMA register bit definitions
#
# --- Channel Control Register (DMA_CCRx)
#
        .set    ccrce,0                 # Channel Enable
        .set    ccrcr,1                 # Chain Resume
#
# --- Channel Status Register (DMA_CSRx)
#
        .set    csrpe,0                 # PCI parity error
        .set    csrta,2                 # PCI target abort flag
        .set    csrma,3                 # PCI master abort flag
        .set    csrbf,5                 # 80960 bus fault error flag
        .set    csrmf,6                 # 80960 memory fault error flag
        .set    csrec,8                 # end of chain interrupt flag
        .set    csret,9                 # end of transfer interrupt flag
        .set    csrca,10                # channel active flag
#
# --- Descriptor Control Register (DMA_DCRx)
#
        .set    dcrpc,0                 # PCI command byte (bits 0-3)
        .set    dcrie,4                 # Interrupt enable
        .set    dcrdu,5                 # Dual Address enable
        .set    dcrde,6                 # Demand Mode enable
        .set    dcrai,7                 # Address Increment hold enable
#
# --- PCI command byte definitions
#
        .set    dmrlc,0x6               # Read of less than one cacheline
        .set    dmwrt,0x7               # Memory write
        .set    dmmrm,0xc               # Memory read multiple cacheline
        .set    dmmrl,0xe               # Memory read 1 cacheline
        .set    dmmwi,0xf               # Memory write and invalidate
#
# --- XINT6# ISR (PIC_XINT6)
#
        .set    dm0ip,0                 # Channel 0 interrupt pending
        .set    dm1ip,1                 # Channel 1 interrupt pending
        .set    dm2ip,2                 # Channel 2 interrupt pending
        .set    dmext,3                 # External XINT6# int pending
#
# --- DMA complete bit (application defined)
#
        .set    dmcmp,0                 # DMA operation complete flag
#
# --- DMA descriptor block structure
#
# --- Note: structure must be aligned on an 8-word boundary.
#
        .set    dma_nda,0               # Next Descriptor address
        .set    dma_pci,dma_nda+4       # PCI address lower
        .set    dma_pciu,dma_pci+4      # PCI address upper
        .set    dma_loc,dma_pciu+4      # 80960 local address
        .set    dma_bc,dma_loc+4        # Transfer byte count
        .set    dma_dc,dma_bc+4         # Descriptor control
#
# --- Next two words are not part of 80960 DMA descriptor block, added
#     for additional information to be passed with the SGL.
#
        .set    dma_last,dma_dc+4       # Last descriptor in chain
        .set    dma_dct,dma_last+4      # Descriptor count
#
# --- End structure
#
        .set    dma_size,dma_dct+4      # sizeof DMA block
#
# --- Structure for DMA control block -----------------------------------------
#
        .set    dma_begin,0             # Begin of DMA control queue
        .set    dma_in,dma_begin+4      # IN pointer of DMA control queue
        .set    dma_out,dma_in+4        # OUT pointer of DMA control queue
        .set    dma_end,dma_out+4       # Limit of DMA control queue
        .set    dma_free,dma_end+4      # first available DMA descriptor
#
# --- end of structure
#
        .set    dmacbsiz,dma_free+4     # sizeof dmacb
#
#
# --- Structure for DMA control queue entry
#
        .set    dma_head,0              # First descriptor in chain
        .set    dma_pcb,dma_head+4      # PCB of task waiting for DMA unit
#
# --- end of structure
#
        .set    dmacqsiz,dma_pcb+4      # sizeof dma control queue entry
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
