# $Id: isp2100.inc 158811 2011-12-20 20:42:56Z m4 $
.ifndef ISP2100_INC
.set ISP2100_INC,TRUE
#******************************************************************************
#
#  NAME: isp2100.inc
#
#  DESCRIPTION:
#
#       To provide definitions for the QLogic ISP 2x00 Fibre Channel chip.
#
#  Copyright (c) 1996 - 2009 Xiotech Corporation.  All rights reserved.
#
#******************************************************************************
#
# --- Equates ---------------------------------------------------------
#
        .set    qvendid,0x1077          # QLogic Vendor ID
        .set    mmvendid,0x1332         # Micro Memory Vendor ID
        .set    intelvenid,0x8086       # Intel Corporation Vendor ID
        .set    qdev2422id,0x2422       # Qlogic Device ID for ISP2422
        .set    qdev2432id,0x2432       # Qlogic Device ID for ISP2432
        .set    qdev2532id,0x2532       # Qlogic Device ID for ISP2532
        .set    mmcardid,0x5425         # Micro Memory Card Device ID
#
        .set    LOOPPOSSZ,128           # sizeof Loop Position Map per instance
#
# --- Request and Response queue definitions
#
        .set    reqsize,64              # size in bytes of a request q entry
        .set    ressize,64              # size in bytes of a response q entry
        .set    atiosize,64             # size in bytes of a atio q entry
#
        .set    numreqs,512             # size of request queue
        .set    numresp,256             # size of response queue
        .set    atioreqs,  512             # size of atio queue
#
        .set    reqalloc,numreqs * reqsize # size of entire req queue
        .set    resalloc,numresp * ressize # size of entire res queue
        .set    atioqalloc,atioreqs * atiosize # size of entire atio queue
#
# --- The size of asynchronous event queue is big enough for all outstanding
#     requests plus a little buffer
#
        .set    ASYSIZE,(numreqs + 16)*asyqsz # Size of asynchronous event queue
#
# --- PCI latency definition
#
        .set    qlatency,248            # 248 (0xF8) clocks latency timer

#
# --- Mailbox Command Complete Status Codes
#
        .set    ismcmdc,0x4000          # Command Successful
        .set    ismcmdi,0x4001          # Invalid Command
        .set    ismcmde,0x4005          # Command Error
        .set    ismccpe,0x4006          # Command Parameter Error
        .set    ismcpiu,0x4007          # Port ID used
        .set    ismcliu,0x4008          # Loop ID used
        .set    ismciiu,0x4009          # All Loop ID's in use
        .set    ispnli,0x400A           # FFFFFC (SNS) not logged in
        .set    ismnosw,0x4095          # Not Issued, fabric is not supported
#
# --- <ISP$login_fabric_port> returned codes
#
        .set    islfnor,0               # Operation complete
        .set    islfnfc,1               # No Fabric connection available
        .set    islferr,2               # Command Parameter error
        .set    islfpiu,3               # Port ID already in use
        .set    islfliu,4               # Loop ID already in use
        .set    islfiiu,5               # All loop ID's in use
        .set    islfune,6               # Unknown error
        .set    islfcmde,7              # Command Error
#
# --- <ISP$get_all_next> returned codes
#
        .set    isgnnor,0               # Operation complete
        .set    isgnnfc,1               # No Fabric connection available
        .set    isgnerr,2               # Command Parameter error
        .set    isgnune,3               # Unknown error
        .set    isgnnli,4               # Not logged in

#
# --- ISP2400 PCI registers (non-PCI header type) (32-bit registers)
#     Value for all these equates is offset from beginning of memory-mapped
#     region programmed for the chip.
#
        .set    isp24fbiosaddr,0             # Flash BIOS address
        .set    isp24fbiosdata,4             # Flash BIOS data
        .set    isp24cntl,8               # ISP Control/Status
        .set    isp24intc,0x0C               # ISP to PCI Interrupt Control
        .set    isp24ints,0x10             # ISP to PCI Interrupt Status
        .set    isp24hccr,0x48            # ISP Host Command and Control
#
# --- 2300 Mailbox Registers (16-bit)
#
        .set    ispm0,0
        .set    ispm1,2
        .set    ispm2,4
        .set    ispm3,6
        .set    ispm4,8
        .set    ispm5,10
        .set    ispm6,12
        .set    ispm7,14

# --- 2400 Request and Response Registers (32-bit) <stadi>
#
        .set    isp24rqip,0x1C          # Request queue in pointer
        .set    isp24rqop,0x20          # Request queue out pointer
        .set    isp24rsip,0x24          # Response queue in pointer
        .set    isp24rsop,0x28          # Response queue out pointer
        .set    isp24prip,0x2C          # priority req Q in  pointer
        .set    isp24prop,0x30          # priority req Q out  pointer
        .set    isp24atip,0x3C          # ATIO  Q in  pointer
        .set    isp24atop,0x40          # ATIO  Q out  pointer
        .set    isp24rhis,0x44          # RISC to Host Interrupt Status
# --- 2400 Mailbox Registers (16-bit) <stadi>
        .set    isp24m0,0x80            # Mailbox Register 0 for 2400

#
# --- ISP to PCI(2400) Interrupt Control Register(32bit)
#
        .set    isp24inte,3              # Enable risc interrupts on PCI
#
# --- ISP Asynchronous Event Codes
#
        .set    aspsye,0x8002           # Unrecoverable Error
#
# --- ISP initialization mailbox messages (on powerup/reset)
#
        .set    isp1,0x4953             # First part of message, "IS"
        .set    isp2,0x5020             # Second part, "P "
        .set    isp3,0x2020             # Third part, "  " (blanks)
#
# --- ISP online event codes
#
        .set    ispodbc,0x14            # Online due to DB change event
        .set    ispofte,0xf0            # Online due to Foreign target enable
        .set    ispolop,0xf1            # Online due to Loss of Port
#
# --- Data structures ---------------------------------------------------------
#
        .set    ganbsize,28             # Sizeof GAN request buffer
#
        .set    ganrsbln,320            # Sizeof GAN response bfr (16-bit words)

#
# --- ISP2100 revision information structure
#
# --- begin structure
#
        .set    ispvendid,0             # Vendor ID
        .set    ispmodel,ispvendid+4    # Vendor model
        .set    isprevlvl,ispmodel+4    # Revision Level of ISP
        .set    isprsclvl,isprevlvl+4   # RISC revision Level
        .set    ispfpmlvl,isprsclvl+4   # FB & FPM revision levels
        .set    ispromlvl,ispfpmlvl+4   # RISC ROM revision level
        .set    ispfwmajor,ispromlvl+4  # ISP firmware major revision
        .set    ispfwminor,ispfwmajor+2 # ISP firmware minor revision
        .set    ispfwsub,ispfwminor+2   # ISP firmware subminor revision
        .set    ispfwattrib,ispfwsub+2  # ISP firmware attribute
        .set    ispdataRate,ispfwattrib+2 # Data Rate (1G/2G)
        .set    ispendMemAddr,ispdataRate+4 # Ending Memory Addr(Used in DebugDump Procedure)
#
# --- end structure
#
        .set    isprsiz,ispendMemAddr+2   # sizeof ISP revision struct
#
# --- ISP2100 data structure
#
# --- begin structure
#
        .set    ispbasead,0             # ISP PCI base address
        .set    ispreqque,ispbasead+4   # ISP request QCB location
        .set    ispresque,ispreqque+4   # ISP response QCB location
        .set    ispicbstr,ispresque+4   # ISP ICB location
        .set    ispmbox,ispicbstr+4     # ISP mailbox base address
        .set    isprqip,ispmbox+4       # Request queue in pointer
        .set    isprqop,isprqip+4       # Request queue out pointer
        .set    isprsip,isprqop+4       # Response queue in pointer
        .set    isprsop,isprsip+4       # Response queue out pointer
        .set    ispatioque,isprsop+4    # ISP ATIO queue QCB location
#
# --- end structure
#
        .set    ispsiz,ispatioque+4        # sizeof ISP data struct

#
# --- begin structure for asynchronous event queue
#
        .set    asevent,0               # Event Code
        .set    asevilt,asevent+4       # ILT address (handle) for event
#
# --- end structure
#
        .set    asyqsz,asevilt+4        # sizeof queue entry

.endif # ISP2100_INC
#
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
