# $Id: icb.inc 145407 2010-08-10 17:26:37Z m4 $
#******************************************************************************
#
#  NAME: icb.inc
#
#  DESCRIPTION:
#
#       To provide definitions for the QLogic ISP 2x00 Fibre Channel chip
#       Initialization Control Block (ICB).
#
#  Copyright (c) 1996 - 2000 XIOtech Corporation.  All rights reserved.
#
#******************************************************************************
#
# --- Initialization Control Block (ICB) structure
#
        .set    ispver,0                # Version                    <b>
        .set    isprsv,1                # Reserved                   <b>
        .set    ispfwo,2                # Firmware Options           <s>
        .set    ispmfl,4                # Maximum Frame Length       <s>
        .set    ispira,6                # Maximum IOCB Res Alloc     <s>
        .set    ispext,8                # Execution Throttle         <s>
        .set    isprtc,0xA              # Retry Count                <b>
        .set    isprtd,0xB              # Retry Delay                <b>
        .set    isppnm,0xC              # Port Name                  <l>
        .set    isphad,0x14             # Hard Address               <s>
        .set    ispnnm,0x18             # Node Name                  <l>
        .set    isprqo,0x20             # Request Queue Outpointer   <s>
        .set    isprsi,0x22             # Response Queue Inpointer   <s>
        .set    isprql,0x24             # Request Queue Length       <s>
        .set    isprsl,0x26             # Response Queue Length      <s>
        .set    ispreqa,0x28            # Request Queue Address      <l>
        .set    isprsqa,0x30            # Response Queue Address     <l>
        .set    isplune,0x38            # Lun enables                <s>
        .set    ispcomc,0x3A            # Command Resource count     <b>
        .set    ispimnc,0x3B            # Immediate Notify Res Count <b>
        .set    isptimo,0x3C            # Timeout                    <s>
        .set    ispadfo,0x40            # Additional Firmware Opts   <s>
        .set    ispratm,0x42            # Response Accum Timer       <b>
        .set    ispintd,0x43            # Interrupt Delay Timer      <b>
        .set    ispspco,0x44            # Special Options            <s>
#
        .set    ispicbs,ispspco+28      # Sizeof ICB
#
# --- ICB bit definitions
#
# --- Firmware options
#
        .set    fwexc,15                # Extended Control Block
        .set    fwnmo,14                # Name Option
        .set    fwflp,13                # Login after LIP
        .set    fwspqfs,12              # Stop port queue on full status
        .set    fwpalid,11              # Previous assigned loop ID
        .set    fwdlids,10              # Descending loop ID search
        .set    fwdilip,9               # Disable initial LIP
        .set    fwdbe,8                 # Enable Port Database Changed AE
        .set    fwetdt,7                # Enable target device type
        .set    fwadisc,6               # Enable ADISC
        .set    fwimd,5                 # Initiator mode disable
        .set    fwtme,4                 # Target mode enable
        .set    fwfsp,3                 # Fast Status Posting
        .set    fwfde,2                 # Full Duplex enable
        .set    fwfen,1                 # Fairness enable
        .set    fwhae,0                 # Hard Address Enable
#
# --- Additional Firmware Options bits
#
        .set    fwencr,14               # Enable Command Reference Number
        .set    fwenfc,13               # Enable Fibre Channel Confirm
        .set    fwentp,12               # Enable Fibre Channel Tape
        .set    fwack0,9                # Enable ACK0
        .set    fwenc2,8                # Enable Class 2
        .set    fwnpha,7                # Nonparticipating if no hard addr
                                        #  acquired
        .set    fwconn,4                # bits 6-4 are Connection Options
#
#                                       # 0 = Loop only
                                        # 1 = Point to point only
                                        # 2 = Loop Preferred, otherwise p-p
                                        # 3 = P-P preferred, otherwise loop
        .set    fwrioo,3                # bits 3-0 are for Reduced
                                        #  Interrupt operation
#
# --- Special Options
#
        .set    fwenrx,0                # Enable Read XFR_RDY
        .set    fwdr,14                 # bits 15-14 are Data rate
        .set    fw50ohm,13              # bit 2 is 50 ohm termination
        .set    fwauto,2                # Auto-negotiated data rate
#*****************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
