# $Id: err.inc 145407 2010-08-10 17:26:37Z m4 $
#******************************************************************************
#
#  NAME: err.inc
#
#  PURPOSE:
#
#       To define error codes.
#
#  Copyright (c) 1996 - 2004 XIOtech Corporation.  All rights reserved.
#
#******************************************************************************
#
# --- Kernel error codes start at KERNERRB
#
        .set    KERNERRB,0
#
# --- Processor fault codes
#                                                              i960 type
        .set    kerr01,KERNERRB+0x01    # Parallel fault          0
        .set    kerr02,KERNERRB+0x02    # Trace fault             1
        .set    kerr03,KERNERRB+0x03    # Operation fault         2
        .set    kerr04,KERNERRB+0x04    # Arithmetic fault        3
        .set    kerr05,KERNERRB+0x05    # Constraint fault        5
        .set    kerr06,KERNERRB+0x06    # Protection fault        7
        .set    kerr07,KERNERRB+0x07    # Machine fault
        .set    kerr08,KERNERRB+0x08    # Type fault              a
        .set    kerr09,KERNERRB+0x09    # Override fault
        .set    kerr10,KERNERRB+0x0a    # NMI
#
        .set    kerr11,KERNERRB+0x0b    # Spare
        .set    kerr12,KERNERRB+0x0c    # Spare
        .set    kerr13,KERNERRB+0x0d    # Spare
        .set    kerr14,KERNERRB+0x0e    # Spare
#
        .set    kerr15,KERNERRB+0x0f    # Spare
        .set    kerr16,KERNERRB+0x10    # K$dmax: DMA control queue full
#
        .set    kerr17,KERNERRB+0x11    # K$twait: illegal timeout value
        .set    kerr18,KERNERRB+0x12    # Spare
        .set    kerr19,KERNERRB+0x13    # Spare
#
# --- K$qcs error codes
#
        .set    kerr20,KERNERRB+0x14    # K$qcs got queue number out of range
        .set    kerr21,KERNERRB+0x15    # K$qcs found NULL PCB address
#
# --- DMA Interrupt (XINT6) processor error codes
#
        .set    kerr22,KERNERRB+0x16    # .dmaint got enpty control queue
        .set    kerr23,KERNERRB+0x17    # .dmaint got unknown error signal
        .set    kerr24,KERNERRB+0x18    # .dmaint got midchain interrupt
        .set    kerr25,KERNERRB+0x19    # .dmaint got Master-Abort signal
        .set    kerr26,KERNERRB+0x1a    # .dmaint got Target-Abort signal
        .set    kerr27,KERNERRB+0x1b    # .dmaint got PCI parity error
        .set    kerr28,KERNERRB+0x1c    # .dmaint got Local Bus Fault error
        .set    kerr29,KERNERRB+0x1d    # .dmaint got 80960 Memory Fault error
        .set    kerr30,KERNERRB+0x1e    # spare
        .set    kerr31,KERNERRB+0x1f    # spare
#
# --- Error codes start at <PROCERRB> to allow for kernel error codes
#
        .set    PROCERRB,0x20           # Give kernel 32 error codes
#
# --- Process-level error codes -----------------------------------------------
#
        .set    err00,PROCERRB+0x00     # d$forceerrortrap: MRP forces error trap
#
        .set    err01,PROCERRB+0x01     # l$exec_comp: VRP proxy addr out of range
#
        .set    err02,PROCERRB+0x02     # isp$rqx: got null PCI base address
#
        .set    err03,PROCERRB+0x03     # isp$rqx: got null async event anchor
#
        .set    err04,PROCERRB+0x04     # m$exec_hbeatmon: lost CCB heartbeat
#
        .set    err05,PROCERRB+0x05     # isp$rqx: no PCB assigned for this IOCB
#
        .set    err06,PROCERRB+0x06     # isp$configure: too many devices
#
        .set    err07,PROCERRB+0x07     # isp$rqx: null QRP for event
#
        .set    err08,PROCERRB+0x08     # L$que: invalid function number
#
        .set    err09,PROCERRB+0x09     # l$exec_orp: ILT dequeue error
#
        .set    err10,PROCERRB+0x0a     # d$psd_rebuilder: logic error
#
        .set    err11,PROCERRB+0x0b     # l$exec_irp: invalid function number
#
        .set    err12,PROCERRB+0x0c     # L$que: command is for THIS i960
#
        .set    err13,PROCERRB+0x0d     # l$exec_comp: no compl. routine
#
        .set    err14,PROCERRB+0x0e     # l$boot: can't allocate statistics table
#
        .set    err15,PROCERRB+0x0f     # l$exec_irp: queue pointer out of range
#
        .set    err16,PROCERRB+0x10     # L$comp: invalid function number
#
        .set    err17,PROCERRB+0x11     # l$sync: invalid state
#
        .set    err18,PROCERRB+0x12     # o$rerror_exec: no available devices
#
        .set    err19,PROCERRB+0x13     # RBI Node not found when should be or
                                        #  Outstanding Ops on I/O tree when
                                        #    VID being deleted
#
        .set    err20,PROCERRB+0x14     # K$qcsret: queue number out of range
#
        .set    err21,PROCERRB+0x15     # isp$monitor: got invalid completion routine
#
        .set    err22,PROCERRB+0x16     # isp$get_iocb: no QRP available
#
        .set    err23,PROCERRB+0x17     # isp$get_iocb: bad wakeup command response
#
        .set    err24,PROCERRB+0x18     # c$findimt: wrong IMT on Active List
#
        .set    err25,PROCERRB+0x19     # M$soft_flt: Software Fault detected
#
        .set    err26,PROCERRB+0x1a     # mag$bld_maci: bad configuration data
#
        .set    err27,PROCERRB+0x1b     # isp$initialize: can't initialize chip
#
        .set    err28,PROCERRB+0x1c     # isp$rqx: fatal error from ISP2100
#
        .set    err29,PROCERRB+0x1d     # K$start: Unsupported CPU type
#
        .set    err30,PROCERRB+0x1e     # PR$Tboltinit: Unsupported DIMM size
#
        .set    err31,PROCERRB+0x1f     # WC$recoveryFlushTask: FE/BE DRAM size
#
        .set    err32,PROCERRB+0x20     # init: No PCI devices detected
#
        .set    err33,PROCERRB+0x21     # ISP$initialize: Not successful
#
        .set    err34,PROCERRB+0x22     # isp_unthread_ilt: Bad linked list (NOT USED)
#
        .set    err35,PROCERRB+0x23     # ISP$receive_io: invalid PCI address
#
        .set    err36,PROCERRB+0x24     # isp$check_thread: ILT timeout
#
        .set    err37,PROCERRB+0x25     # isp$rqx: unexpected chip reset
#
        .set    err38,PROCERRB+0x26     # C$Stop: Too Long to end Ops Outstand
#
        .set    err39,PROCERRB+0x27     # Spare
#
        .set    errLinux,PROCERRB+0x40  # Linux generated error trap
#
# --- This trap needs to reserve the next 128 (0x80) error codes.
# --- The next error code needs to be (PROCERRB+0xC0)
#
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
