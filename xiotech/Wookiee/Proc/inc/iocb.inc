# $Id: iocb.inc 148621 2010-10-05 17:49:17Z m4 $
#******************************************************************************
#
#  NAME: iocb.inc
#
#  DESCRIPTION:
#       To provide definitions for the QLogic ISP 2400 I/O command block
#       (IOCB).  All IOCBs start with a common header, and are always
#       64 bytes in length.
#
#  Copyright (c) 1996 - 2010 Xiotech Corporation.  All rights reserved.
#
#******************************************************************************
#
        .set   iocb_with_wait_ilt_type, 0xDA    #ilt type for wait for iocb commands
# --- Equates -----------------------------------------------------------------
#
        .set    iocbsiz,64              # All IOCBs are 64 bytes long
        .set    iocblog,6               # LOG(sizeof IOCB)
#
# --- IOCB type equates
#
        .set    st0,0x03                # Status Type 0 IOCB
        .set    iomar,0x04              # Marker type IOCB
        .set    st0cn,0x10              # Status Type 0 continuation IOCB
        .set    cmio7,0x18              # Continue I/O type 7
        .set    ctio7,0x12              # Command Type 7 IOCB
        .set    ntack,0x0E              # Notify Acknowledge
        .set    msiocb,0x29             # CT pass-through IOCB
        .set    vpctrl,0x30             # Virtual Port Control IOCB
        .set    mvpc,0x31               # Modify Virtual Port Configuration IOCB
        .set    imria,0x32              # Report ID acquisition IOCB
        .set    mbiocb,0x39             # Mailbox IOCB
        .set    loginlogout,0x52        # isp login and logout iocb
        .set    elsiocb,0x53            # ELS passthrough IOCB
        .set    abtsrcv,0x54            # ABTS received
        .set    abtsack,0x55            # ABTS ack
#
# --- Common IOCB status codes
#
        .set    iocbnor,0x01            # Normal completion status
        .set    iocbabt,0x02            # Command aborted
        .set    iocbrce,0x04            # Request completed with error
        .set    iocbivr,0x06            # Invalid Request
        .set    iocbirx,0x08            # Invalid RX_ID
        .set    iocbovr,0x09            # Data overrun
        .set    iocbcmt,0x0B            # Command Timeout
        .set    iocbbsr,0x0E            # Bus Reset
        .set    iocbdma,0x10            # DMA (PCI) error
        .set    iocbdre,0x11            # Data reassembly error
        .set    iocbund,0x15            # Data underrun error
        .set    iocbplo,0x28            # Port Unavailable
        .set    iocbplg,0x29            # Port Logout
        .set    iocbpca,0x2A            # Port configuration changed
        .set    iocbsrr,0x45            # SRR received
#
# --- IOCB common header structure --------------------------------------------
#
# --- Begin structure
#
        .set    ioentyp,0               # IOCB entry type            <b>
        .set    ioencnt,1               # IOCB entry count           <b>
        .set    iosystd,2               # System Defined 1           <b>
                                        #  for driver use
        .set    ioentst,3               # IOCB Entry Status          <b>
#
# --- End structure
#
# --- Specific IOCB type structures -------------------------------------------
#
# --- System Reserved Struct
#
        .set    iosr2,4                 # start of reserved area;    <w>
                                        #  for driver use
#
#*****************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
