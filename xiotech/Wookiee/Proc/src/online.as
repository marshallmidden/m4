# $Id: online.as 161259 2013-06-26 20:16:02Z marshall_midden $
#**********************************************************************
#
#  NAME: online.as
#
#  PURPOSE:
#       To provide a means of controlling the procedures associated with
#       the spinning up of SCSI disk drives.
#
#  FUNCTIONS:
#       This module employs the following processes:
#
#       o$online   - Online process (1 copy)
#       O$init_drv - Drive initialization process (n copies)
#
#  Copyright (c) 1996-2010 Xiotech Corp. All rights reserved.
#
#**********************************************************************
#
# --- local equates ---------------------------------------------------
#
        .set    OHOTSPARECHECKTIME,0x1b77400 # time in ms between hot spare checks (8 hours)
        .set    DEFNAME,'++++'          # default name to give a device
.ifndef  MODEL_3000
.ifndef  MODEL_7400
        .set    HS_DELAY,(500/QUANTUM)  # Delay 1/2 second
.endif  # MODEL_7400
.endif  # MODEL_3000
.ifndef  MODEL_7000
.ifndef  MODEL_4700
        .set    HS_DELAY,(60000/QUANTUM)    # Delay 60 seconds
.endif  # MODEL_7000
.endif  # MODEL_4700
#
# --- global function declarations ------------------------------------
#
        .globl  O$init                  # Online initialization
        .globl  O$init_drv              # Initialize drive
        .globl  O$genreq                # Generate physical SCSI request
        .globl  O$quereq                # Queue request wait
        .globl  O$quereqnw              # Queue request w/o wait, release when complete
        .globl  O$relreq                # Release physical SCSI request
        .globl  O$ledchanged            # let CCB know an LED changed
        .globl  O$stop                  # Stop online activity
        .globl  O$resume                # Resume online activity
        .globl  O_createdefaults        # Create default targets and server
        .globl  O$writefailedlabel      # Write failed label
        .globl  O$spindowndrive         # Spin down drive
        .globl  O$pdiskspindown         # Spin down the user designated drive
#
        .globl  O_logerror              # log error code in g0 to CCB
#
# --- global data declarations ----------------------------------------
#
        .globl  O_t_inquiry             # Inquiry template - standard
        .globl  O_t_testurdy            # Test Unit Ready template
        .globl  O_t_startunit           # Start Unit template - start
        .globl  O_t_stopunit            # Start Unit template - stop
        .globl  O_t_sninquiry           # Inquiry template - unit serial no.
        .globl  O_t_verify1             # Verify checkword 1MB
        .globl  O_t_rdcap               # Read Capacity template
        .globl  O_t_mscache             # Mode Sense template - cache page
        .globl  O_t_msrwer              # Mode Sense template - r/w error recov.
        .globl  O_t_msver               # Mode Sense template - verify err recov.
        .globl  O_t_mspcpnew            # Mode sense template - power conditions
        .globl  O_t_msiecp              # Mode sense template - information exception
        .globl  O_t_msfcicp             # Mode sense template - fibre channel control
        .globl  O_t_rdwrrsvd            # Read/write Reserved Area
        .globl  O_t_rd                  # Zero length read template
        .globl  O_t_wr                  # Zero length write template
        .globl  O_t_wrsame              # Write Same template
        .globl  O_t_rdbuff              # Read buffer
        .globl  O_t_wrbuff              # Write buffer
        .globl  O_t_wrseg               # Write one Segment
        .globl  O_t_rdseg               # Read one Segment
        .globl  O_devlab                # Device label string
        .globl  O_drvinits              # Drive inits in progress
        .globl  O_p2init                # Phase II inits complete (T/F)
        .globl  O_P2Init                # Phase II inits complete (T/F)
        .globl  O_temp_nvram            # Pointer to copy of NVRAM
        .globl  O_p_pdd_list            # Pointer to list of drives at POR
        .globl  O_stopcnt               # Count of nested stop commands
        .globl  O_seagate               # 'SEAGATE ' vendor ID
#
        .globl  O_retryhotswap_pcb      # Retry hotswap PCB
        .globl  o_online_pcb            # Online PCB - debug only
        .globl  o_hotswap_pcb           # Hotswap PCB - debug only
        .globl  o_inquiries_pending     # Pending inquires - debug only
#
#
# --- global usage data definitions -----------------------------------
#
# --- raid resident data
#
        .data
        .align  2
#
# --- Global data
#
.globl O_ipcb
O_ipcb:
        .space  MAXDRIVES*4,0
#
# --- Interlocking control variables
#
O_stopcnt:
        .byte   0                       # Count of nested stop commands
o_hs_stopped:
        .byte   1                       # Hotswap task has stopped - T/F
o_tur_stopped:
        .byte   1                       # Test Unit Ready task has stopped - T/F
#
# C access to O_p2Init
O_P2Init:
O_p2init:
        .byte   FALSE                   # Phase II inits performed
        .align  2
#
# --- Template for SCSI Inquiry - (standard inquiry data)
#
#     Standard inquiry command with 56 byte return of standard inquiry data.
#
        .align  2
        .globl  gTemplateInquiry        # C access
gTemplateInquiry:
O_t_inquiry:
        .word   BTIMEOUT                # pr_timeout
        .word   0xff                    # pr_rqbytes
        .byte   prinput                 # pr_func
        .byte   6                       # pr_cbytes
.ifndef MODEL_3000
.ifndef  MODEL_7400
        .byte   prSLIb+prBCCb+prSNXb+prBNOb # pr_flags
.endif  # MODEL_7400
.endif  # MODEL_3000
.ifndef  MODEL_7000
.ifndef  MODEL_4700
        .byte   prSLIb+prBCCb+prSNXb    # pr_flags
.endif  # MODEL_7000
.endif  # MODEL_4700
        .byte   IORETRY                 # pr_retry
        .byte   0x12                    # pr_cmd (inquiry)
        .byte   0
        .byte   0
        .byte   0
        .byte   0xff
        .byte   0
        .space  10,0                    # pad to 16 bytes
#
# --- Template for SCSI Test Unit Ready
#
        .globl  gTemplateTestUnitReady  # C access
gTemplateTestUnitReady:
O_t_testurdy:
.ifndef MODEL_3000
.ifndef  MODEL_7400
        .word   BTIMEOUT                # pr_timeout
.endif  # MODEL_7400
.endif  # MODEL_3000
.ifndef  MODEL_7000
.ifndef  MODEL_4700
        .word   5                       # pr_timeout
.endif  # MODEL_7000
.endif  # MODEL_4700
        .word   0                       # pr_rqbytes
        .byte   prctl                   # pr_func
        .byte   6                       # pr_cbytes
.ifndef MODEL_3000
.ifndef  MODEL_7400
        .byte   prBCCb+prSPSb+prSNXb+prBNOb # pr_flags
.endif  # MODEL_7400
.endif  # MODEL_3000
.ifndef  MODEL_7000
.ifndef  MODEL_4700
        .byte   prBCCb+prSPSb+prSNXb    # pr_flags
.endif  # MODEL_7000
.endif  # MODEL_4700
        .byte   IORETRY                 # pr_retry
        .byte   0x00                    # pr_cmd
        .byte   0
        .byte   0
        .byte   0
        .byte   0
        .byte   0
        .space  10,0                    # pad to 16 bytes
#
# --- Template for SCSI Start/Stop Unit - (start unit)
#
        .globl  gTemplateStartUnit      # C access
gTemplateStartUnit:
O_t_startunit:
        .word   BTIMEOUT                # pr_timeout
        .word   0                       # pr_rqbytes
        .byte   prctl                   # pr_func
        .byte   6                       # pr_cbytes
.ifndef MODEL_3000
.ifndef  MODEL_7400
        .byte   prBCCb+prSNXb+prBNOb     # pr_flags
.endif  # MODEL_7400
.endif  # MODEL_3000
.ifndef  MODEL_7000
.ifndef  MODEL_4700
        .byte   prBCCb+prSNXb           # pr_flags
.endif  # MODEL_7000
.endif  # MODEL_4700
        .byte   IORETRY                 # pr_retry
        .byte   0x1b                    # pr_cmd
        .byte   0x01                    # immediate bit
        .byte   0
        .byte   0
        .byte   0x01
        .byte   0
        .space  10,0                    # pad to 16 bytes
#
# --- Template for SCSI Start/Stop Unit - (stop unit)
#
        .globl  gTemplateStopUnit       # C access
gTemplateStopUnit:
O_t_stopunit:
        .word   90                      # pr_timeout
        .word   0                       # pr_rqbytes
        .byte   prctl                   # pr_func
        .byte   6                       # pr_cbytes
.ifndef MODEL_3000
.ifndef  MODEL_7400
        .byte   prSNXb+prBNOb            # pr_flags
.endif  # MODEL_7400
.endif  # MODEL_3000
.ifndef  MODEL_7000
.ifndef  MODEL_470
        .byte   prSNXb                  # pr_flags
.endif  # MODEL_7000
.endif  # MODEL_4700
        .byte   IORETRY                 # pr_retry
        .byte   0x1b                    # pr_cmd
        .byte   0
        .byte   0
        .byte   0
        .byte   0x00
        .byte   0
        .space  10,0                    # pad to 16 bytes
#
# --- Template for SCSI Inquiry - (unit serial number)
#
        .globl  gTemplateInqSN        # C access
gTemplateInqSN:
O_t_sninquiry:
        .word   BTIMEOUT                # pr_timeout
        .word   0xff                    # pr_rqbytes
        .byte   prinput                 # pr_func
        .byte   6                       # pr_cbytes
.ifndef MODEL_3000
.ifndef  MODEL_7400
        .byte   prSLIb+prBCCb+prSNXb+prBNOb # pr_flags
.endif  # MODEL_7400
.endif  # MODEL_3000
.ifndef  MODEL_7000
.ifndef  MODEL_4700
        .byte   prSLIb+prBCCb+prSNXb    # pr_flags
.endif  # MODEL_7000
.endif  # MODEL_4700
        .byte   IORETRY                 # pr_retry
        .byte   0x12                    # pr_cmd (inquiry)
        .byte   1
        .byte   0x80
        .byte   0
        .byte   0xff
        .byte   0
        .space  10,0                    # pad to 16 bytes
#
.ifndef MODEL_3000
.ifndef  MODEL_7400
# --- Template for SCSI Inquiry - (unit serial number)
#
O_t_ise_l_s_p_30:
        .word   BTIMEOUT                # pr_timeout
        .word   0xff                    # pr_rqbytes
        .byte   prinput                 # pr_func
        .byte   10                      # pr_cbytes
        .byte   prSLIb+prBCCb+prSNXb+prBNOb # pr_flags
        .byte   IORETRY                 # pr_retry
        .byte   0x4d                    # pr_cmd (log sense page)
        .byte   0x00
        .byte   0x30                    # page 30
        .byte   0x00
        .byte   0x00
        .byte   0x00
        .byte   0x00
        .byte   0x20
        .byte   0x00
        .byte   0x00
        .space  6,0                     # pad to 16 bytes
.endif  # MODEL_7400
.endif  # MODEL_3000
#
# --- Template for SCSI Inquiry - (device ID)
#
        .globl  gTemplateInqDevID       # C access
gTemplateInqDevID:
.ifndef MODEL_3000
.ifndef  MODEL_7400
        .word   BTIMEOUT                # pr_timeout
.endif  # MODEL_7400
.endif  # MODEL_3000
.ifndef  MODEL_7000
.ifndef  MODEL_4700
        .word   5                       # pr_timeout
.endif  # MODEL_7000
.endif  # MODEL_4700
        .word   0xff                    # pr_rqbytes
        .byte   prinput                 # pr_func
        .byte   6                       # pr_cbytes
.ifndef MODEL_3000
.ifndef  MODEL_7400
        .byte   prSLIb+prBCCb+prSNXb+prBNOb # pr_flags
.endif  # MODEL_7400
.endif  # MODEL_3000
.ifndef  MODEL_7000
.ifndef  MODEL_4700
        .byte   prSLIb+prBCCb+prSNXb    # pr_flags
.endif  # MODEL_7000
.endif  # MODEL_4700
        .byte   IORETRY                 # pr_retry
        .byte   0x12                    # pr_cmd (inquiry)
        .byte   1
        .byte   0x83
        .byte   0
        .byte   0xff
        .byte   0
        .space  10,0                    # pad to 16 bytes
#
# --- Template for Write (no buffers will be allocated)
#
# Used in defbe.c to Label a PDisk. (Thus the first 128mb.)
        .globl  gTemplateWrite          # C access
gTemplateWrite:
# Used in fsys.as.c to write PDisk label. (Thus the first 128mb.)
O_t_wr:
.ifndef MODEL_3000
.ifndef  MODEL_7400
        .word   BTIMEOUT                # pr_timeout
.endif  # MODEL_7400
.endif  # MODEL_3000
.ifndef  MODEL_7000
.ifndef  MODEL_4700
        .word   4                       # pr_timeout
.endif  # MODEL_7000
.endif  # MODEL_4700
        .word   0                       # pr_rqbytes
        .byte   proutput                # pr_func
        .byte   10                      # pr_cbytes
        .byte   0                       # pr_flags
        .byte   IORETRY                 # pr_retry
        .byte   0x2a                    # pr_cmd
        .byte   0
        .byte   0                       # MSBs of LBA
        .byte   0
        .byte   0
        .byte   0                       # LSBs of LBA
        .byte   0
        .byte   0                       # MSB of length
        .byte   0                       # LSB of length
        .byte   0
        .space  6,0                     # pad to 16 bytes
#
# --- Template for Read (no buffers will be allocated)
#
# Only used for file system reads (fsys.as), thus in the first 128mb.
        .globl  gTemplateRead           # C access
gTemplateRead:
O_t_rd:
.ifndef MODEL_3000
.ifndef  MODEL_7400
        .word   BTIMEOUT                # pr_timeout
.endif  # MODEL_7400
.endif  # MODEL_3000
.ifndef  MODEL_7000
.ifndef  MODEL_4700
        .word   4                       # pr_timeout
.endif  # MODEL_7000
.endif  # MODEL_4700
        .word   0                    # pr_rqbytes
        .byte   prinput                 # pr_func
        .byte   10                      # pr_cbytes
        .byte   0                       # pr_flags
        .byte   IORETRY                 # pr_retry
        .byte   0x28                    # pr_cmd
        .byte   0
        .byte   0                       # MSBs of LBA
        .byte   0
        .byte   0
        .byte   0                       # LSBs of LBA
        .byte   0
        .byte   0                       # MSB of length
        .byte   0                       # LSB of length
        .byte   0
        .space  6,0                     # pad to 16 bytes

#
# --- Template for Verify Checkword (reserved area)
#
# --- Do one the reserved area in chunks to cover the 128MB reserved area.
#
        .globl  gTemplateVerify         # C access
gTemplateVerify:
O_t_verifyrsvd:
        .word   32                      # pr_timeout
        .word   0                       # pr_rqbytes
        .byte   prctl                   # pr_func
        .byte   10                      # pr_cbytes
        .byte   prBCCb+prSNXb           # pr_flags
        .byte   IORETRY                 # pr_retry
        .byte   0x2f                    # pr_cmd
        .byte   DPOVERIFY*16
        .byte   0
        .byte   0
        .byte   0
        .byte   0
        .byte   0
        .byte   ((SYSSRESERVE)>>11)&0xff# MSB of length
        .byte   (SYSSRESERVE>>3)&0xff   # LSB of length
        .byte   0
        .space  6,0                     # pad to 16 bytes
#
# --- Template for Verify Checkword (1st MB)
#
        .globl  gTemplateVerify1        # C access
gTemplateVerify1:
O_t_verify1:
        .word   32                      # pr_timeout
        .word   0                       # pr_rqbytes
        .byte   prctl                   # pr_func
        .byte   10                      # pr_cbytes
        .byte   prSNXb+prBCCb           # pr_flags
        .byte   IORETRY                 # pr_retry
        .byte   0x2f                    # pr_cmd
        .byte   DPOVERIFY*16
        .byte   0
        .byte   0
        .byte   0
        .byte   0
        .byte   0
        .byte   ((DSKSALLOC)>>8)&0xff   # MSB of length
        .byte   (DSKSALLOC)&0xff        # LSB of length
        .byte   0
        .space  6,0                     # pad to 16 bytes
#
# --- Template for Verify Checkword (16) (1st MB)
#
        .globl  gTemplateVerify1_16     # C access
gTemplateVerify1_16:
O_t_verify1_16:
        .word   32                      # pr_timeout
        .word   0                       # pr_rqbytes
        .byte   prctl                   # pr_func
        .byte   16                      # pr_cbytes
        .byte   prSNXb+prBCCb           # pr_flags
        .byte   IORETRY                 # pr_retry
        .byte   0x8f                    # pr_cmd    (0)
        .byte   DPOVERIFY*16            # flags     (1)
        .byte   0,0,0,0, 0,0,0,0        # lba       (2,3,4,5,6,7,8,9)
        .byte   0,0                     # MSB of lth(10,11)
        .byte   ((DSKSALLOC)>>8)&0xff   # length    (12)
        .byte   (DSKSALLOC)&0xff        # length    (13)
        .byte   0                       # reserved  (14)
        .byte   0                       # control   (15)
#
# --- Template for Read Capacity
#
O_t_rdcap:
        .word   BTIMEOUT                # pr_timeout
        .word   8                       # pr_rqbytes
        .byte   prinput                 # pr_func
        .byte   10                      # pr_cbytes
        .byte   0                       # pr_flags
        .byte   IORETRY                 # pr_retry
        .byte   0x25                    # pr_cmd
        .byte   0
        .byte   0,0,0,0
        .byte   0
        .byte   0
        .byte   0
        .byte   0
        .space  6,0                     # pad to 16 bytes
#
# --- Template for Read Capacity
#
O_t_rdcap_16:
        .word   BTIMEOUT                # pr_timeout
        .word   8                       # pr_rqbytes
        .byte   prinput                 # pr_func
        .byte   16                      # pr_cbytes
        .byte   0                       # pr_flags
        .byte   IORETRY                 # pr_retry
        .byte   0x9e                    # pr_cmd                (0)
        .byte   0x10                    # service action 0x10   (1)
        .byte   0,0,0,0,0,0,0,0         # Logical Block Address (2,3,4,5,6,7,8,9)
        .byte   0,0,0,32                # Allocation Length     (10,11,12,13)
        .byte   0                       # reserved/PMI          (14)
        .byte   0                       # Control               (15)
#
# --- Template for Mode Sense (all pages - current values)
#
#     The mode parameter header and the block descriptor each consume
#     8 bytes.  The actual page requested follows these headers.
#
        .globl  gTemplateMSAll          # C access
gTemplateMSAll:
        .word   BTIMEOUT                # pr_timeout
        .word   0x800                   # pr_rqbytes
        .byte   prinput                 # pr_func
        .byte   10                      # pr_cbytes
        .byte   prSLIb+prBCCb+prSNXb    # pr_flags
        .byte   IORETRY                 # pr_retry
        .byte   0x5a                    # pr_cmd
        .byte   0
        .byte   0x7F
        .byte   0,0,0,0
        .byte   0x08,0x00
        .byte   0
        .space  6,0                     # pad to 16 bytes
#
# --- Template for Mode Sense (caching page - changeable values)
#
#     The mode parameter header and the block descriptor each consume
#     8 bytes.  The actual page requested follows these headers.
#
        .globl  gTemplateMSCache        # C access
gTemplateMSCache:
O_t_mscache:
        .word   BTIMEOUT                # pr_timeout
        .word   0x24                    # pr_rqbytes
        .byte   prinput                 # pr_func
        .byte   10                      # pr_cbytes
        .byte   prSLIb+prBCCb+prSNXb    # pr_flags
        .byte   IORETRY                 # pr_retry
        .byte   0x5a                    # pr_cmd
        .byte   0
        .byte   0x48
        .byte   0,0,0,0
        .byte   0,0x24
        .byte   0
        .space  6,0                     # pad to 16 bytes
#
# --- Template for Mode Sense (r/w error recovery page - changeable values)
#
        .globl  gTemplateMSRWErr        # C access
gTemplateMSRWErr:
O_t_msrwer:
        .word   BTIMEOUT                # pr_timeout
        .word   0x1c                    # pr_rqbytes
        .byte   prinput                 # pr_func
        .byte   10                      # pr_cbytes
        .byte   prSLIb+prBCCb+prSNXb    # pr_flags
        .byte   IORETRY                 # pr_retry
        .byte   0x5a                    # pr_cmd
        .byte   0
        .byte   0x41
        .byte   0,0,0,0
        .byte   0,0x1c
        .byte   0
        .space  6,0                     # pad to 16 bytes
#
# --- Template for Mode Sense (verify error recovery page - changeable values)
#
        .globl  gTemplateMSVErr         # C access
gTemplateMSVErr:
O_t_msver:
        .word   BTIMEOUT                # pr_timeout
        .word   0x1c                    # pr_rqbytes
        .byte   prinput                 # pr_func
        .byte   10                      # pr_cbytes
        .byte   prSLIb+prBCCb+prSNXb    # pr_flags
        .byte   IORETRY                 # pr_retry
        .byte   0x5a                    # pr_cmd
        .byte   0
        .byte   0x47
        .byte   0,0,0,0
        .byte   0,0x1c
        .byte   0
        .space  6,0                     # pad to 16 bytes
#
# --- Template for Mode Sense (new power condition page - changeable values)
#
        .globl  gTemplateMSPower        # C access
gTemplateMSPower:
O_t_mspcpnew:
        .word   BTIMEOUT                # pr_timeout
        .word   0x1c                    # pr_rqbytes
        .byte   prinput                 # pr_func
        .byte   10                      # pr_cbytes
        .byte   prSLIb+prBCCb+prSNXb    # pr_flags
        .byte   IORETRY                 # pr_retry
        .byte   0x5a                    # pr_cmd
        .byte   0
        .byte   0x5a
        .byte   0,0,0,0
        .byte   0,0x1c
        .byte   0
        .space  6,0                     # pad to 16 bytes
#
# --- Template for Mode Sense (information exception control page) SMART
#
        .globl  gTemplateMSException    # C access
gTemplateMSException:
O_t_msiecp:
        .word   BTIMEOUT                # pr_timeout
        .word   0x1c                    # pr_rqbytes
        .byte   prinput                 # pr_func
        .byte   10                      # pr_cbytes
        .byte   prSLIb+prBCCb+prSNXb    # pr_flags
        .byte   IORETRY                 # pr_retry
        .byte   0x5a                    # pr_cmd
        .byte   0
        .byte   0x5c
        .byte   0,0,0,0
        .byte   0,0x1c
        .byte   0
        .space  6,0                     # pad to 16 bytes
#
# --- Template for Mode Sense (fibre channel interface control page)
#
        .globl  gTemplateMSFC           # C access
gTemplateMSFC:
O_t_msfcicp:
        .word   BTIMEOUT                # pr_timeout
        .word   0x18                    # pr_rqbytes
        .byte   prinput                 # pr_func
        .byte   10                      # pr_cbytes
        .byte   prSLIb+prBCCb+prSNXb    # pr_flags
        .byte   IORETRY                 # pr_retry
        .byte   0x5a                    # pr_cmd
        .byte   0                       # DPD = 0
        .byte   0x59                    # PCF = changeable + page code 19
        .byte   0,0,0,0                 # reserved
        .byte   0,0x18                  # allocation length
        .byte   0                       # control byte
        .space  6,0                     # pad to 16 bytes
#
.if 0
# --- Template for Mode Sense (SAS interface sub page)
#
        .globl  gTemplateMSSASsub1           # C access
gTemplateMSSASsub1:
O_t_mssassub1:
        .word   BTIMEOUT                # pr_timeout
        .word   0x68                    # pr_rqbytes
        .byte   prinput                 # pr_func
        .byte   10                      # pr_cbytes
        .byte   prSLIb+prBCCb+prSNXb    # pr_flags
        .byte   IORETRY                 # pr_retry
        .byte   0x5a                    # pr_cmd
        .byte   0                       # DPD = 0
        .byte   0x19                    # PCF = changeable + page code 19
        .byte   1                       # subpage 1
        .byte   0,0,0                   # reserved
        .byte   0,0x68                  # allocation length
        .byte   0                       # control byte
        .space  6,0                     # pad to 16 bytes
.endif  # 0
#
# --- Template for Read 1 Segment (read extended)
#
.if 0
        .globl  gTemplateReadExt        # C access
gTemplateReadExt:
O_t_rdseg:
        .word   BTIMEOUT                # pr_timeout
        .word   DSKBALLOC               # pr_rqbytes
        .byte   prinput                 # pr_func
        .byte   10                      # pr_cbytes
        .byte   0                       # pr_flags
        .byte   IORETRY                 # pr_retry
        .byte   0x28                    # pr_cmd
        .byte   0
#
        .byte   0,0,0,0                 # LBA
        .byte   0
        .byte   (DSKSALLOC>>8)&0xff     # LSBs of sector count
        .byte   (DSKSALLOC)&0xff        # MSBs of sector count
        .byte   0
        .space  6,0                     # pad to 16 bytes
.endif  # 0
#
# --- Template for Write 1 Segment (write extended)
#
        .globl  gTemplateWriteExt       # C access
gTemplateWriteExt:
O_t_wrseg:
        .word   BTIMEOUT                # pr_timeout
        .word   DSKBALLOC               # pr_rqbytes
        .byte   proutput                # pr_func
        .byte   10                      # pr_cbytes
        .byte   0                       # pr_flags
        .byte   IORETRY                 # pr_retry
        .byte   0x2a                    # pr_cmd
        .byte   0
        .byte   0,0,0,0                 # LBA
        .byte   0
        .byte   ((DSKBALLOC/SECSIZE)>>8)&0xff  # LSBs of sector count
        .byte   ((DSKBALLOC/SECSIZE))&0xff # MSBs of sector count
        .byte   0
        .space  6,0                     # pad to 16 bytes
#
# --- Template for Write 1 Segment (write-16 SCSI command)
#
O_t_wrseg_16:
        .word   BTIMEOUT                # pr_timeout
        .word   DSKBALLOC               # pr_rqbytes
        .byte   proutput                # pr_func
        .byte   16                      # pr_cbytes
        .byte   0                       # pr_flags
        .byte   IORETRY                 # pr_retry
        .byte   0x8A                    # pr_cmd                (0)
        .byte   0                       #                       (1)
        .byte   0,0,0,0,0,0,0,0         # LBA                   (2,3,4,5,6,7,8,9)
        .byte   0,0                     # transfer length       (10,11)
        .byte   ((DSKBALLOC/SECSIZE)>>8)&0xff # transfer length (12)
        .byte   ((DSKBALLOC/SECSIZE))&0xff    # transfer length (13)
        .byte   0                       #                       (14)
        .byte   0                       #                       (15)
.if 0
#
# --- Template for Read 1 Segment of Reserved Area (read extended)
# --- Also used for writes from the same area (change cmd byte and direction)
#
        .globl  gTemplateReadRsvd       # C access
gTemplateReadRsvd:
O_t_rdwrrsvd:
        .word   BTIMEOUT                # pr_timeout
        .word   ((SYSSRESERVE)>>3)      # pr_rqbytes
        .byte   prinput                 # pr_func
        .byte   10                      # pr_cbytes
        .byte   0                       # pr_flags
        .byte   IORETRY                 # pr_retry
        .byte   0x28                    # pr_cmd
        .byte   0
        .byte   0,0,0,0                 # LBA
        .byte   0
        .byte   ((SYSSRESERVE)>>11)&0xff# MSB of length
        .byte   (SYSSRESERVE>>3)&0xff   # LSB of length
        .byte   0
        .space  6,0                     # pad to 16 bytes
.endif  # 0
.ifndef DISABLE_WRITE_SAME
#
# --- Template for Write Same
#
#     Must subsequently patch pr_timeout, pr_sda, pr_eda, logical block
#     address and number of blocks depending upon specific requirements.
#
        .globl  gTemplateWriteSame      # C access
gTemplateWriteSame:
O_t_wrsame:
        .word   BTIMEOUT                # pr_timeout
        .word   512                     # pr_rqbytes
        .byte   proutput                # pr_func
        .byte   10                      # pr_cbytes
        .byte   0                       # pr_flags
        .byte   IORETRY                 # pr_retry
        .byte   0x41                    # pr_cmd
        .byte   0
        .byte   0,0,0,0                 # LBA
        .byte   0
        .byte   0,0                     # blocks
        .byte   0
        .space  6,0                     # pad to 16 bytes

O_t_wrsame_16:
        .word   BTIMEOUT                # pr_timeout
        .word   512                     # pr_rqbytes
        .byte   proutput                # pr_func
        .byte   16                      # pr_cbytes
        .byte   0                       # pr_flags
        .byte   IORETRY                 # pr_retry
        .byte   0x93                    # pr_cmd                (0)
        .byte   0                                               (1)
        .byte   0,0,0,0,0,0,0,0         # LBA                   (2,3,4,5,6,7,8,9)
        .byte   0,0,0,0                 # transfer length       (10,11,12,13)
        .byte   0                                               (14)
        .byte   0                                               (15)
.endif  # !DISABLE_WRITE_SAME
#
# --- Template for Read Buffer
#
#     Must subsequently patch pr_rqbytes and transfer length (CDB byte 6)
#     depending upon specific requirements.
#
        .globl  gTemplateReadBuff       # C access
gTemplateReadBuff:
O_t_rdbuff:
        .word   BTIMEOUT                # pr_timeout
        .word   0x200                   # pr_rqbytes
        .byte   prinput                 # pr_func
        .byte   10                      # pr_cbytes
        .byte   prBLPb+prBCCb           # pr_flags
.ifndef MODEL_3000
.ifndef  MODEL_7400
        .byte   IORETRY                 # pr_retry
.endif  # MODEL_7400
.endif  # MODEL_3000
.ifndef  MODEL_7000
.ifndef  MODEL_4700
        .byte   1                       # pr_retry
.endif  # MODEL_7000
.endif  # MODEL_4700
        .byte   0x3b                    # pr_cmd - CDB
        .byte   0x02                    # mode 2
        .byte   0                       # buffer id
        .byte   0,0,0                   # buffer offset
        .byte   0,0x02,0                # transfer length in bytes
        .byte   0                       # control
        .space  6,0                     # pad to 16 bytes
#
# --- Template for Write Buffer
#
#     Must subsequently patch pr_rqbytes and transfer length (CDB byte 6)
#     depending upon specific requirements.
#
        .globl  gTemplateWriteBuff      # C access
gTemplateWriteBuff:
O_t_wrbuff:
        .word   BTIMEOUT                # pr_timeout
        .word   0x200                   # pr_rqbytes
        .byte   proutput                # pr_func
        .byte   10                      # pr_cbytes
        .byte   prBLPb+prBCCb           # pr_flags
.ifndef MODEL_3000
.ifndef  MODEL_7400
        .byte   IORETRY                 # pr_retry
.endif  # MODEL_7400
.endif  # MODEL_3000
.ifndef  MODEL_7000
.ifndef  MODEL_4700
        .byte   1                       # pr_retry
.endif  # MODEL_7000
.endif  # MODEL_4700
        .byte   0x3c                    # pr_cmd - CDB
        .byte   0x02                    # mode 2
        .byte   0                       # buffer id
        .byte   0,0,0                   # buffer offset
        .byte   0,0x02,0                # transfer length in bytes
        .byte   0                       # control
        .space  6,0                     # pad to 16 bytes
#
#
# --- Template for Inquiry to fetch SES information.
#
.if 0
        .globl  gTemplateSESP0Rd        # C access
gTemplateSESP0Rd:
        .word   10                # pr_timeout
        .word   2048                    # pr_rqbytes
        .byte   prinput                 # pr_func
        .byte   6                       # pr_cbytes
        .byte   prSLIb+prBCCb+prSNXb    # pr_flags
        .byte   IORETRY                 # pr_retry
        .byte   0x1C                    # pr_cmd (receive diagnostic)
        .byte   0x01                    # Page code valid
        .byte   0x00                    # Page code 0
        .byte   0x08                    # Allocation length
        .byte   0x00                    # 2 Kbytes allocated
        .byte   0
        .space  10,0                    # pad to 16 bytes
.endif  # 0
#
.ifndef  MODEL_7000
.ifndef  MODEL_4700
        .globl  gTemplateSESP1Rd        # C access
gTemplateSESP1Rd:
        .word   10                      # pr_timeout
        .word   2048                    # pr_rqbytes
        .byte   prinput                 # pr_func
        .byte   6                       # pr_cbytes
        .byte   prSLIb+prBCCb+prSNXb    # pr_flags
        .byte   IORETRY                 # pr_retry
        .byte   0x1C                    # pr_cmd (receive diagnostic)
        .byte   0x01                    # Page code valid
        .byte   0x01                    # Page code 1
        .byte   0x08                    # Allocation length
        .byte   0x00                    # 2 Kbytes allocated
        .byte   0
        .space  10,0                    # pad to 16 bytes
#
        .globl  gTemplateSESP2Rd        # C access
gTemplateSESP2Rd:
        .word   10                      # pr_timeout
        .word   2048                    # pr_rqbytes
        .byte   prinput                 # pr_func
        .byte   6                       # pr_cbytes
        .byte   prSLIb+prBCCb+prSNXb    # pr_flags
        .byte   IORETRY                 # pr_retry
        .byte   0x1C                    # pr_cmd (receive diagnostic)
        .byte   0x01                    # Page code valid
        .byte   0x02                    # Page code 2
        .byte   0x08                    # Allocation length
        .byte   0x00                    # 2 Kbytes allocated
        .byte   0
        .space  10,0                    # pad to 16 bytes
#
        .globl  gTemplateSESP4Rd        # C access
gTemplateSESP4Rd:
        .word   10                      # pr_timeout
        .word   2048                    # pr_rqbytes
        .byte   prinput                 # pr_func
        .byte   6                       # pr_cbytes
        .byte   prSLIb+prBCCb+prSNXb    # pr_flags
        .byte   IORETRY                 # pr_retry
        .byte   0x1C                    # pr_cmd (receive diagnostic)
        .byte   0x01                    # Page code valid
        .byte   0x04                    # Page code 4
        .byte   0x08                    # Allocation length
        .byte   0x00                    # 2 Kbytes allocated
        .byte   0
        .space  10,0                    # pad to 16 bytes
#
.if 0
        .globl  gTemplateSESP4WWN       # C access
gTemplateSESP4WWN:
        .word   10                      # pr_timeout
        .word   13                      # pr_rqbytes
        .byte   proutput                # pr_func
        .byte   6                       # pr_cbytes
        .byte   prBCCb+prSNXb           # pr_flags
        .byte   IORETRY                 # pr_retry
        .byte   0x1D                    # pr_cmd (send diagnostic)
        .byte   0x10                    # Page code valid
        .byte   0x00
        .byte   0x00                    # Allocation length
        .byte   0x0D                    # 12 bytes
        .byte   0
        .space  10,0                    # pad to 16 bytes
#
        .globl  gTemplateSESP7Rd        # C access
gTemplateSESP7Rd:
        .word   10                      # pr_timeout
        .word   2048                    # pr_rqbytes
        .byte   prinput                 # pr_func
        .byte   6                       # pr_cbytes
        .byte   prSLIb+prBCCb+prSNXb    # pr_flags
        .byte   IORETRY                 # pr_retry
        .byte   0x1C                    # pr_cmd (receive diagnostic)
        .byte   0x01                    # Page code valid
        .byte   0x07                    # Page code 7
        .byte   0x08                    # Allocation length
        .byte   0x00                    # 2 Kbytes allocated
        .byte   0
        .space  10,0                    # pad to 16 bytes
#
        .globl  gTemplateSESP0ARd       # C access
gTemplateSESP0ARd:
        .word   10                      # pr_timeout
        .word   2048                    # pr_rqbytes
        .byte   prinput                 # pr_func
        .byte   6                       # pr_cbytes
        .byte   prSLIb+prBCCb+prSNXb    # pr_flags
        .byte   IORETRY                 # pr_retry
        .byte   0x1C                    # pr_cmd (receive diagnostic)
        .byte   0x01                    # Page code valid
        .byte   0x0a                    # Page code a
        .byte   0x08                    # Allocation length
        .byte   0x00                    # 2 Kbytes allocated
        .byte   0
        .space  10,0                    # pad to 16 bytes
.endif  # 0
#
        .globl  gTemplateSESP86Rd       # C access
gTemplateSESP86Rd:
        .word   10                      # pr_timeout
        .word   2048                    # pr_rqbytes
        .byte   prinput                 # pr_func
        .byte   6                       # pr_cbytes
        .byte   prSLIb+prBCCb+prSNXb    # pr_flags
        .byte   IORETRY                 # pr_retry
        .byte   0x1C                    # pr_cmd (receive diagnostic)
        .byte   0x01                    # Page code valid
        .byte   0x86                    # Page code 86
        .byte   0x08                    # Allocation length
        .byte   0x00                    # 2 Kbytes allocated
        .byte   0
        .space  10,0                    # pad to 16 bytes
#
        .globl  gTemplateSESP87Rd       # C access
gTemplateSESP87Rd:
        .word   10                      # pr_timeout
        .word   2048                    # pr_rqbytes
        .byte   prinput                 # pr_func
        .byte   6                       # pr_cbytes
        .byte   prSLIb+prBCCb+prSNXb    # pr_flags
        .byte   IORETRY                 # pr_retry
        .byte   0x1C                    # pr_cmd (receive diagnostic)
        .byte   0x01                    # Page code valid
        .byte   0x87                    # Page code 87
        .byte   0x08                    # Allocation length
        .byte   0x00                    # 2 Kbytes allocated
        .byte   0
        .space  10,0                    # pad to 16 bytes
#
        .globl  gTemplateSESP80Rd       # C access
gTemplateSESP80Rd:
        .word   10                      # pr_timeout
        .word   2048                    # pr_rqbytes
        .byte   prinput                 # pr_func
        .byte   6                       # pr_cbytes
        .byte   prSLIb+prBCCb+prSNXb    # pr_flags
        .byte   IORETRY                 # pr_retry
        .byte   0x1C                    # pr_cmd (receive diagnostic)
        .byte   0x01                    # Page code valid
        .byte   0x80                    # Page code 80
        .byte   0x08                    # Allocation length
        .byte   0x00                    # 2 Kbytes allocated
        .byte   0
        .space  10,0                    # pad to 16 bytes
#
        .globl  gTemplateSESP81Rd       # C access
gTemplateSESP81Rd:
        .word   10                      # pr_timeout
        .word   2048                    # pr_rqbytes
        .byte   prinput                 # pr_func
        .byte   6                       # pr_cbytes
        .byte   prSLIb+prBCCb+prSNXb    # pr_flags
        .byte   IORETRY                 # pr_retry
        .byte   0x1C                    # pr_cmd (receive diagnostic)
        .byte   0x01                    # Page code valid
        .byte   0x81                    # Page code 81
        .byte   0x08                    # Allocation length
        .byte   0x00                    # 2 Kbytes allocated
        .byte   0
        .space  10,0                    # pad to 16 bytes
#
        .globl  gTemplateSESP82Rd       # C access
gTemplateSESP82Rd:
        .word   10                      # pr_timeout
        .word   2048                    # pr_rqbytes
        .byte   prinput                 # pr_func
        .byte   6                       # pr_cbytes
        .byte   prSLIb+prBCCb+prSNXb    # pr_flags
        .byte   IORETRY                 # pr_retry
        .byte   0x1C                    # pr_cmd (receive diagnostic)
        .byte   0x01                    # Page code valid
        .byte   0x82                    # Page code 82
        .byte   0x08                    # Allocation length
        .byte   0x00                    # 2 Kbytes allocated
        .byte   0
        .space  10,0                    # pad to 16 bytes
#
        .globl  gTemplateSESP83Rd       # C access
gTemplateSESP83Rd:
        .word   10                      # pr_timeout
        .word   2048                    # pr_rqbytes
        .byte   prinput                 # pr_func
        .byte   6                       # pr_cbytes
        .byte   prSLIb+prBCCb+prSNXb    # pr_flags
        .byte   IORETRY                 # pr_retry
        .byte   0x1C                    # pr_cmd (receive diagnostic)
        .byte   0x01                    # Page code valid
        .byte   0x83                    # Page code 83
        .byte   0x08                    # Allocation length
        .byte   0x00                    # 2 Kbytes allocated
        .byte   0
        .space  10,0                    # pad to 16 bytes
#
        .globl  gTemplateSESP82Wr       # C access
gTemplateSESP82Wr:
        .word   10                      # pr_timeout
        .word   45                      # pr_rqbytes
        .byte   proutput                # pr_func
        .byte   6                       # pr_cbytes
        .byte   prBCCb+prSNXb+prUCLb    # pr_flags
        .byte   IORETRY                 # pr_retry
        .byte   0x1D                    # pr_cmd (send diagnostic)
        .byte   0x10                    # Page code valid
        .byte   0x00
        .byte   0x00                    # Allocation length
        .byte   0x2D                    # 44 bytes allocated
        .byte   0
        .space  10,0                    # pad to 16 bytes
#
        .globl  gTemplateSESP83Wr       # C access
gTemplateSESP83Wr:
        .word   10                      # pr_timeout
        .word   45                      # pr_rqbytes
        .byte   proutput                # pr_func
        .byte   6                       # pr_cbytes
        .byte   prBCCb+prSNXb+prUCLb    # pr_flags
        .byte   IORETRY                 # pr_retry
        .byte   0x1D                    # pr_cmd (send diagnostic)
        .byte   0x10                    # Page code valid
        .byte   0x00
        .byte   0x00                    # Allocation length
        .byte   0x2D                    # 44 bytes allocated
        .byte   0
        .space  10,0                    # pad to 16 bytes
.endif  # MODEL_4700
.endif  # MODEL_7000
#
O_temp_nvram:
        .word   0
#
O_devlab:
        .ascii  "//XIOtech Device Label//"
O_devlablen_end:
        .set    o_devlablen,O_devlablen_end-O_devlab  # Length of device label
        .byte   0                       # Null termination
#
O_faillab:
        .ascii  "//XIOtech Failed Dev//  "
O_faillab_end:
        .set    o_faillablen,O_faillab_end-O_faillab# Length of device label
        .byte   0                       # Null termination
#
        .globl  O_faillab               # Failed label

.ifndef MODEL_3000
.ifndef  MODEL_7400
# ISE-related command structures

        .globl  gISEVolumeInfo                  # C access
gISEVolumeInfo:
        .word   BTIMEOUT                # pr_timeout
        .word   96                      # pr_rqbytes
        .byte   prinput                 # pr_func
        .byte   6                       # pr_cbytes
        .byte   prSLIb+prSNXb+prBLPb+prBNOb # pr_flags
        .byte   IORETRY                 # pr_retry
        .byte   0xFE                    # pr_cmd (ISE Vendor specific command to obtain volume info)
        .byte   0
        .byte   0
        .byte   0
        .byte   0x60
        .byte   0
        .space  10,0                    # pad to 16 bytes

# Management Network Address page(ISE)
        .globl  gemplateMgmtNetworkPage85
gTemplateMgmtNetworkPage85:
        .word   BTIMEOUT                # pr_timeout
        .word   0xff                    # pr_rqbytes
        .byte   prinput                 # pr_func
        .byte   6                       # pr_cbytes
        .byte   prSLIb+prBCCb+prSNXb+prBNOb  # pr_flags
        .byte   IORETRY                 # pr_retry
        .byte   0x12                    # pr_cmd (inquiry)
        .byte   1
        .byte   0x85
        .byte   0
        .byte   0xff
        .byte   0
        .space  10,0                    # pad to 16 bytes
.endif  # MODEL_7400
.endif  # MODEL_3000

#
# --- local usage data definitions ------------------------------------
#
        .align  2
#
o_inquiries_pending:
        .word   0                       # number of inquiries currently pending for swap
#
        .globl O_drvinits
#
O_drvinits:
        .word   0                       # current # of devices in spinup
#
O_spaces:
        .ascii  "                "      # 16 spaces for string compare
#
O_seagate:
        .ascii  "SEAGATE "              # Vendor ID
#
        .globl  O_p_pdd_list            # C access
O_p_pdd_list:
        .word   0                       # Pointer to initial drive PDD list
#
# --- Processes
#
o_online_pcb:                           # Online process
        .word   0
o_hotswap_pcb:
        .word   0                       # Hotswap process
O_retryhotswap_pcb:
        .word   0                       # Retry hotswap process
o_retryhotswap_delay:
        .word   0                       # Seconds of delay before retry
#
# --- Fields for TUR polling loop
#
        .globl  o_polldrivecnt
        .globl  o_polldrive_pcb
        .globl  o_polldrivebg
#
o_polldrivecnt:
        .word   0
o_polldrive_pcb:
        .word   0
o_polldrivebg:
        .byte   FALSE
#
# --- executable code (low usage) -------------------------------------
#
        .text
#**********************************************************************
#
#  NAME: O$init
#
#  PURPOSE:
#       To provide a means of initializing this module.
#
#  DESCRIPTION:
#       The online process is established.  This process controls the
#       invocation of the initial drive spinups and POSTs.
#
#  CALLING SEQUENCE:
#       call    O$init
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g0
#       g1
#
#**********************************************************************
#
O$init:
#
# --- Initialize the lists to zeros
#
        movq    0,r4                    # Set up a quad zero
        lda     D_fillstart,r8          # Starting address
        lda     D_fillend,r9            # Ending address
#
.oi10:
        stq     r4,(r8)                 # Store a big zero
        lda     16(r8),r8               # Bump pointer
        cmpobl  r8,r9,.oi10             # Do it again if not done
#
# --- Fork online process
#
        lda     o$online,g0             # Establish online process
        ldconst ONLINEPRI,g1
c       CT_fork_tmp = (ulong)"o$online";
        call    K$tfork
        st      g0,o_online_pcb         # Save PCB
#
# --- Fork raid 5 error process
#
        lda     rb$rerror_exec,g0       # Establish executive process
        ldconst RERRORPRI,g1            # Priority
c       CT_fork_tmp = (ulong)"rb$rerror_exec";
        call    K$fork
#
# --- Fork SES discovery process
#
        lda     SES_BackGroundProcess,g0 # Establish SES process
        ldconst ONLINEPRI,g1
c       CT_fork_tmp = (ulong)"SES_BackGroundProcess";
        call    K$fork
        st      g0,S_bgppcb             # Save the PCB
.ifndef MODEL_7000
.ifndef MODEL_4700
#
# --- Fork SES Update Pages 82 and 83 process
#
        lda     SES_UpdatePages,g0      # Establish SES process
        ldconst ONLINEPRI,g1
c       CT_fork_tmp = (ulong)"SES_UpdatePages";
        call    K$fork
.endif  # MODEL_4700
.endif  # MODEL_7000
#
# Initialize the PR stuff
#
c       InitPR();
#
# --- Exit
#
        ret
#
#**********************************************************************
#
#  NAME: O$stop
#
#  PURPOSE:
#       To provide a common means of stopping all activity within this
#       layer.
#
#  DESCRIPTION:
#       The stop counter is incremented and a check is made for any
#       outstanding I/O.  When all outstanding I/O completes, this
#       routine returns to the caller.  While the stop counter is
#       non-zero, all procedures are effectively blocked.
#
#       This routine may only be called from the process level.
#
#  CALLING SEQUENCE:
#       call    O$stop
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
# C access
# void ON_Stop(void);
        .globl  ON_Stop
ON_Stop:
O$stop:
        ldob    O_stopcnt,r3            # Increment stop counter
        addo    1,r3,r3
        stob    r3,O_stopcnt
#
# --- Stall until pending I/Os complete
#
.ost10:
        ldob    o_hs_stopped,r4         # Check hotswap exec status
        cmpobe  FALSE,r4,.ost20         # Jif o$hotswap is active
        ldob    o_tur_stopped,r3        # Check TUR exec status
        cmpobne FALSE,r3,.ost100        # Jif o$polldrives not active
#
.ost20:
        mov     g0,r15                  # Save g0
        ldconst 1,g0                    # Delay for minimum time
        call    K$twait
        mov     r15,g0                  # Restore g0
        b       .ost10
#
# --- Exit
#
.ost100:
        ret
#
#**********************************************************************
#
#  NAME: O$resume
#
#  PURPOSE:
#       To provide a common means of resuming all activity within
#       this layer.
#
#  DESCRIPTION:
#       The stop counter is decremented and an immediate return is
#       made to the caller.  When this counter has returned to zero,
#       all procedures are unblocked.
#
#  CALLING SEQUENCE:
#       call    O$resume
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
# C access
# void ON_Resume(void);
        .globl  ON_Resume
ON_Resume:
O$resume:
#
# --- Adjust stop counter
#
        ldob    O_stopcnt,r3            # Decrement stop counter
        subo    1,r3,r3
        stob    r3,O_stopcnt
#
# --- Exit
#
        ret
#
#**********************************************************************
#
#  NAME: SES_StartBGProcess
#
#  PURPOSE:
#       Wrapper around C routine to save/restore gregs
#
#  CALLING SEQUENCE:
#       call    SES_StartBGProcess
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
SES_StartBGProcess:
        PushRegs(r8)                    # Save register contents
        call    SES_StartBGProcess_c    # Call the function
        PopRegsVoid(r8)                 # Restore the registers
        ret
#
#**********************************************************************
#
#  NAME: o$online
#
#  PURPOSE:
#       To provide a mechanism for invoking the initial drive spinups
#       and Power On Self Tests (POSTs) at system initialization time.
#
#  DESCRIPTION:
#       This process forks a separate process (O$init_drv) for each
#       possible drive in the system.  Each process is passed the
#       corresponding PDI entry for the device to spin up and perform
#       POSTs.  A byte pointer is passed to a memory location which
#       contains the maximum number of devices that may perform a start
#       unit command concurrently.  A separate byte pointer is employed
#       for each separate cabinet in the Storage Hub.
#
#  CALLING SEQUENCE:
#       process call
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       None.
#
#**********************************************************************
#
        .set LARGE_FLIGHT,FALSE
#
o$online:
.if     LARGE_FLIGHT
        ld      fr_queue,r4
c       s_Free(r4, fr_asize, __FILE__, __LINE__); # Release the old memory for FR
#
        ld      WcbAddr,g0              # Write buffer address
        ld      WcbSize,r3              # Sizeof flight recorder
#
        addo    r3,g0,g7                # point to last entry + 1
        addo    qcb_size,g0,g4          # leave space at front for queue ptrs
        mov     g4,g5                   # initialize IN to BEGIN pointer
        mov     g4,g6                   # initialize OUT to BEGIN pointer
        stq     g4,(g0)                 # save BEGIN, IN, OUT, END pointers
        st      g0,fr_queue             # save pointer to the queue
.endif  # LARGE_FLIGHT
#
# --- For fast path initialization (no waiting on CCB), turn on the
# --- NO_DELAY_ONLINE variable in options.inc.
#
.if NO_DELAY_ONLINE
        ldos    K_ii+ii_status,r4       # Get initialization status
        setbit  iinvramrdy,r4,r4
        setbit  iiccbipaddr,r4,r4
        stos    r4,K_ii+ii_status       # Set the two hold bits from CCB
.endif  # NO_DELAY_ONLINE
#
# --- Prepare a temp copy of the NVRAM for use throughout the
# --- initialization process.
#
c       g0 = s_MallocW(NVSRAMP2SIZ, __FILE__, __LINE__);
        st      g0,O_temp_nvram         # Save address for other fns
#
# --- Copy the data from the NVRAM into the temp location.  Note that
# --- this has to be done a byte at a time since the NVRAM is on an
# --- 8-bit bus and the processor cannot translate non-byte to byte
# --- accesses.
#
        lda     NVSRAM+NVSRAMRES,r14    # Get base of PART II
c       memcpy((void *)g0, (void *)r14, NVSRAMP2SIZ);
#
# --- Load in the system serial number from NVRAM.  Note that this is a
# --- risk since we have not checksum checked the NVRAM, but this is the
# --- only way to get this value.
#
        PushRegs(r8)                    # Save register contents
        call    NV_P2ChkSumChk          # Validate NVRAM checksum
        PopRegs(r8)                     # Restore registers (except g0)
        cmpobne TRUE,g0,.on10           # Jif not OK
#
        ld      O_temp_nvram,g0         # Input addr for VCG ID fetch
        PushRegs(r8)                    # Save register contents
        call    NV_GetVCGID             # Get the VCG ID into the ficb structure
        PopRegsVoid(r8)                 # Restore registers (except g0)
#
# --- Crank up define.  Note that this will be a "partial" define since
# --- we have not initialized the full system.  This define will allow
# --- file system operations and a few other commands.
#
.on10:
        call    D$init                  # "define" initialization
#        call    CCSM$init               # Copy Configuration and Status
                                        #  Manager initialization
#
.on15:
        ldconst 1,g0                    # Wait for 1 time amount
        call    K$twait
#
        ldos    K_ii+ii_status,r4       # Get initialization status
        bbc     iidevconfig,r4,.on15    # Jif device config not received
#
# --- Swap and wait for the physical process to cause us to wake up.
# --- This is done to cause a delay until the fibre channel loops are
# --- up and the O_p_pdd_list list is initialized.
#
.on20:
        ldconst 1,g0                    # Wait for 1 time amount
        call    K$twait
#
        ldos    K_ii+ii_status,r4       # Get initialization status
        bbc     iiphy,r4,.on20          # Jif Physical is ready bit is set
#
# --- Prepare to initialize all drives
#
        ldconst 0,r3                    # Set drive count to zero
        stos    r3,px_ecnt+P_pddindx    # Save it
        st      r3,O_drvinits           # Zero drive inits
#
        ld      O_p_pdd_list,r14
        cmpobe  0,r14,.on20             # If no list go back to sleep
        lda     -px_ecnt(r14),r14       # Adjust for entry counter
        ld      px_ecnt(r14),r15        # get list count
        PushRegs(r3)
c       StartOninitDrive((PDD**)r14,r15,2);
        PopRegsVoid(r3)
#
# --- Create a temporary physical disk list for define to use for the quorum
# --- area.  This list will be reclaimed later by zeroing out all of the
# --- entries in the list and reusing the temp PDD list provided by the
# --- physical layer.  Keeping the list from physical intact prevents the
# --- physical layer from resetting the list.
#
        ldconst MAXOPDDLISTCOUNT,r15    # Get the max drive count
        ld      O_p_pdd_list,r14        # Get the PDD list from physical
        lda     -px_ecnt(r14),r14       # Adjust for entry counter
#
.on45:
        subo    1,r15,r15               # Decrement index into pdd list
        cmpibg  0,r15,.on50             # Jif done
#
        ld      px_pdd(r14)[r15*4],g0   # Get the PDD pointer
        cmpobe  0,g0,.on45              # If no drive present, get next one
#
        ldob    pd_devtype(g0),r3       # Get device type
        cmpobge pddtmaxdisk,r3,.on46    # Jif a disk
#
        cmpobl  pddtmaxses,r3,.on46     # Jif not a bay
#
# --- Call the SES code to install this enclosure in the tables.
#
        PushRegs(r3)                    # Save the registers
        mov     g0,g2                   # Set input to SES function
        ldconst 0,g3                    # Null the counter input
        ldconst 0,g4                    # Do not log.
        call    SES_GetDirectEnclosure  # Get the slot if not known
        PopRegsVoid(r3)                 # Restore registers
        b       .on45                   # Continue
#
.on46:
        call    DEF_InsertPDD           # Put it into the appropriate table
        b       .on45
#
# --- Indicate to define that the list is now ready (drives have done the
# --- short initialization).
#
.on50:
        ld      O_temp_nvram,g0         # Get the NVRAM copy
        PushRegs(r3)                    # Save the registers
        call    NV_ReorderPDDs          # Reorder drives based upon temp nvram
        PopRegsVoid(r3)                 # Restore registers
#
        ldos    K_ii+ii_status,r4       # Get initialization status
        setbit  iitpdd,r4,r4            # Set to indicate a list available
        stos    r4,K_ii+ii_status       # Save it
#
# --- Now wait for elections to complete before re-processing the drives
# --- with the full test and then processing the NVRAM.
#
.on60:
        ldconst 1,g0                    # Wait for 1 time amount
        call    K$twait
#
        ldos    K_ii+ii_status,r4       # Get the flags
        bbc     iinvramrdy,r4,.on60     # Stay here if not told to continue
#
# --- Now if we are the slave, replace the NVRAM image (from real NVRAM)
# --- with a copy from the file system.
#
        ldos    K_ii+ii_status,r4       # Get the flags
        bbs     iireplacement,r4,.on65  # Replacements load from FSYS
        bbc     iislave,r4,.on70        # Skip reading of the file if not slave
#
.on65:
        ldconst fidbenvram,g0           # FID
        ld      O_temp_nvram,g1         # Pointer to buffer
        ldconst 1,g2                    # Length in blocks (just the header)
        ldconst 1,g3                    # Confirmation
        ldconst 1,g4                    # Start at block one
        ldconst 0,g6                    # Set pid bitmap to zero
        call    FS$MultiRead            # Read
        cmpobne 0,g0,.on110             # Jif the read failed
#
# --- Now do the read for the proper amount of data.
#
        ldconst fidbenvram,g0           # FID
        ld      O_temp_nvram,g1         # Pointer to buffer
        ld      12(g1),g2               # Length in bytes
        ldconst SECSIZE,r3              # Bump up a sector
        addo    r3,g2,g2
        divo    r3,g2,g2                # Block count (rounded)
        ldconst 1,g3                    # Confirmation
        ldconst 1,g4                    # Start at block one
        call    FS$MultiRead            # Read
        cmpobne 0,g0,.on110             # Jif the read failed
#
# --- Prepare to initialize all drives
#
.on70:
        ldconst MAXOPDDLISTCOUNT,r15    # Get the max drive count
        ld      O_p_pdd_list,r14        # Get the PDD list from physical
        cmpobe  0,r14,.on100            # Jif no disks found
        lda     -px_ecnt(r14),r14       # Adjust for entry counter
        ldconst 0,r13                   # A handy zero
        stos    r13,px_ecnt+P_pddindx   # Clear the counter
        stos    r13,px_ecnt+E_pddindx   # Clear the counter
        stos    r13,px_ecnt+M_pddindx   # Clear the counter
#
# --- Fork initialization process for the next drive
#
#       clear out global pdd tables.
c       memset((void*)&E_pddindx,0,MAXSES*4);
c       memset((void*)&M_pddindx,0,MAXMISC*4);
c       memset((void*)&P_pddindx,0,MAXDRIVES*4);
        ld      px_ecnt(r14),r15        # get list entry count
        PushRegs(r3)
c       StartOninitDrive((PDD**)r14,r15,3);
        PopRegsVoid(r3)
#
# --- We have a NVRAM image either from disk in the case of a slave or from
# --- the NVRAM in the case of a master.  Checksum it and continue.
#
.on100:
        ld      O_temp_nvram,g0         # Input addr for chksum check
        PushRegs(r8)                    # Save register contents
        call    NV_P2ChkSumChk          # Validate NVRAM checksum
        PopRegs(r8)                     # Restore registers (except g0)
        mov     g0,r10                  # Maintain g0 for later check
        cmpobe  TRUE,g0,.on170          # Jif OK
#
# --- We got an NVRAM error so just enter any drives found into
# --- the PDD list and assign them "random" PIDs.  Place misc devices
# --- and SES devices in their respective lists.
#
.on110:
        ld      O_p_pdd_list,r14        # Get the PDD list from physical
        cmpobe  0,r14,.on160            # Jif no device list
#
        lda     -px_ecnt(r14),r14       # Adjust for the entry count field
        ldos    px_ecnt(r14),r13        # Get the number of drives
        cmpobe  0,r13,.on160            # Jif no disks found
#
        ldconst 0,r3                    # A handy zero
        stos    r3,px_ecnt+P_pddindx    # Clear the counter
        stos    r3,px_ecnt+E_pddindx
        stos    r3,px_ecnt+M_pddindx
#
.on140:
        subo    1,r13,r13               # Decrement index
        cmpibg  0,r13,.on160            # Jif done
#
        ld      px_pdd(r14)[r13*4],r4   # Get PDD
        ldob    pd_devtype(r4),r3       # Get device type
#
        lda     P_pddindx,r15           # Assume PDX table
        cmpobge pddtmaxdisk,r3,.on150   # Jif a drive
#
        lda     E_pddindx,r15           # Assume EDX table
        cmpobge pddtmaxses,r3,.on150    # Jif SES device
#
        lda     M_pddindx,r15           # MDX table - misc device
#
# --- Now save the PDD in the appropo table.  Note that the table
# --- is zero based, so a count is grabbed, used, then incremented.
#
.on150:
        ldos    px_ecnt(r15),r3         # Get current device count
        st      r4,px_pdd(r15)[r3*4]    # Save PDD in real table
        stos    r3,pd_pid(r4)           # Save PID
        addo    1,r3,r3                 # Bump it
        stos    r3,px_ecnt(r15)         # Save current device count
        b       .on140                  # Test if more to do
#
.on160:
        ldos    K_ii+ii_status,g0       # Get initialization status
        setbit  iisesbkrun,g0,g0        # Set to indicate SES_BackgroundProcess can run
        stos    g0,K_ii+ii_status       # Save it
        call    SES_StartBGProcess      # Background processing function start
#
        ldconst mlenvramcsum,g0         # Log NVRAM checksum error
        call    O_logerror
#
# --- Initialize the P6 area and start the copy management tasks.  Note
# --- that r10 holds the NVRAM checksum results.
#
.on170:
        PushRegs(r3)                    # Save all "g" registers
        call    P6_Init                 # initialize P6 NVRAM handler
        PopRegsVoid(r3)                 # restore environment

        call    CCSM$init               # Copy Configuration and Status
                                        #  Manager initialization
        call    CM$init                 # Copy Manager initialization
#
        cmpobne TRUE,r10,.on180         # Jif NVRAM was not OK
#
# --- Restore NVRAM from configuration
#
        ld      O_temp_nvram,g0         # Get NVRAM image
        ld      O_p_pdd_list,g1         # Get temp pd list
        ldconst TRUE,g2                 # Initial, power on load
        ldconst TRUE,g3                 # Restart copies
        ldconst 1,g4                    # Set caller (1=online module)
        mov     g0,r4                   # save NVRAM image
        PushRegs(r3)                    # Save register contents
        call    NV_RestoreNvram         # Restore configuration
#
        call    FAB_BalanceLoad         # Rebalance devices on storage ports
#
        ldos    K_ii+ii_status,g0       # Get initialization status
        setbit  iisesbkrun,g0,g0        # Set to indicate SES_BackgroundProcess can run
        stos    g0,K_ii+ii_status       # Save it
        call    SES_StartBGProcess      # Background processing function start
        PopRegsVoid(r3)                 # Restore registers
#
# --- Log boot complete to CCB
#
.on180:
        ldconst mleboot,g0              # Log boot complete
        call    O_logerror
#
.on190:
        ldconst 1,g0                    # Wait for 1 time amount
        call    K$twait
#
        ldos    K_ii+ii_status,r4       # Get the flags
        bbc     iiccbipaddr,r4,.on190   # Stay here if no IP address sent
#
        call    D$updrmtsysinfo         # Update front end system information
        call    D$updrmtcache           # Update front end on cache state
        call    D$updrmtcacheglobal     # Update the FEP
#
# --- Start up basic processes
#
        lda     o$hotsparecheck,g0      # Fork hotspare checker
        ldconst OHOTSPARECHECKPRIO,g1   # Set priority
c       CT_fork_tmp = (ulong)"o$hotsparecheck";
        call    K$fork
#
        lda     o$hotswap,g0            # Fork hotswap
        ldconst OHOTSWAPPRIO,g1         # Set priority
c       CT_fork_tmp = (ulong)"o$hotswap";
        call    K$fork
        st      g0,o_hotswap_pcb        # Save PCB
#
# --- We are done with the temp list.  Deallocate it.
#
        ld      O_p_pdd_list,g0         # Release this RAM
        cmpobe  0,g0,.on200             # Jif no list
#
c       s_Free(g0, pdxsiz+4, __FILE__, __LINE__);
#
        ldconst 0,r3
        st      r3,O_p_pdd_list         # Clear the anchor
c       TaskReadyByState(pcocmpwait);   # Awaken ISP online event handler
#
# --- Log a configuration changed event to cause an SES update on the CCB.
#
.on200:
        ldos    K_ii+ii_status,r4       # Get initialization status
        setbit  iifulldef,r4,r4         # Set to indicate full define can run
        stos    r4,K_ii+ii_status       # Save it
#
# --- Kick off the RAID 5 Recovery tasks to get them going after setting "Full
#       Define" for the CCB.  The recovery tasks will set the AStatus R5 Stripe
#       Resync in Progress flags before returning.  The FE will be let go once
#       all the AStatus R5 Stripe Resync in Progress flags are clear.
#
        call    O$recoverp3             # Recover any Part 3/4 pending writes.
        call    O$recoverp4             # Don't allow any FE commands until
                                        # the NVRAM NVA records are handled.
#
.on210:
        ldconst 1,g0                    # Wait for 1 time amount
        call    K$twait
#
        ldos    K_ii+ii_status,r4       # Get the flags
        bbc     iip2init,r4,.on210      # Stay here if not told to continue
#
# --- Check the hot spare pool and log any issues with missing or too small
# --- of hot spares.
#
        ldconst 0,g0                    # No PDD being spared.  Just check.
        PushRegs(r3)                    # Save register contents
        call    RB_CheckHSDepletion     # Check the hot spares
#
        ldconst 0,g0
        call    RB_CheckHSCapacity      # Check for capacity problems
        PopRegsVoid(r3)                 # Restore registers
#
        call    O$p2init                # Attempt phase II initializations
#
# --- Exit
#
        ret
#
#**********************************************************************
#
#  NAME: O_createdefaults
#
#  PURPOSE:
#       To provide a mechanism to create the default targets and servers.
#
#  DESCRIPTION:
#       This function will check to see if there is a default server
#       defined.  If not, it will create it.  It also checks to see
#       if there are default targets.  If not, it will create them.
#
#  CALLING SEQUENCE:
#       O_createdefaults
#
#  INPUT:
#       g0 - controller serial number for controller which is having
#            defaults created.
#
#  OUTPUT:
#       g1 - error code to be passed to CCB if called via MRP
#
#  REGS DESTROYED:
#       g0
#       g2
#
#**********************************************************************
#
# C access
# UINT32 ON_CreateDefaults(UINT32 cncSerial);
        .globl  ON_CreateDefaults       # C access
ON_CreateDefaults:
        call    O_createdefaults
        mov     g1,g0
        ret
#
O_createdefaults:
        mov     g0,r10                  # Save g0
#
# --- Check if there is room for the servers.
#
        ldconst deinstable,g1           # Assume not enough space
#
        ldconst MAXISP,r5               # Load the number of targets to create
        ldconst 0,r6                    # Initialize channel number
#
# --- Now check for target space.
#
        lda     T_tgdindx,r12           # Get TGX pointer
        ldos    tgx_ecnt(r12),r11       # Check for enough space
        ldconst MAXTARGETS,r4           # Load the max number of targets
        subo    r4,r11,r4               # Slots available
        cmpobg  r5,r4,.crd100           # Jif not enough slots
#
# --- There was enough space.  Create the items.
#
.crd10:
        cmpobge r6,r5,.crd90            # Jif done
#
# --- Find a target slot.
#
        ldconst 0,r9                    # r9 = TGD slot number
        ldconst 0,r8                    # r8 = SDD slot number
#
.crd20:
        ld      tgx_tgd(r12)[r9*4],r3   # Get TGD pointer
        cmpobne 0,r3,.crd70             # Jif allocated
#
# --- r9 has the TGD slot and r8 has the SDD slot.  Create the data structures
# --- and put them into the slots.  Update the remote processor also.
#
        call    D_alloctarg             # Allocate a target record
        stos    r9,tgd_tid(g0)          # Set the TID
        stob    r6,tgd_port(g0)         # Set the port number
        stob    r6,tgd_pport(g0)        # Set the preferred port number
        xor     1,r6,r7
        stob    r7,tgd_aport(g0)        # Set the alternate port number
        st      g0,tgx_tgd(r12)[r9*4]   # Save in table
#
# --- Set the this controller as the owner and preferred owner
#
        st      r10,tgd_owner(g0)       # Set the owner
        st      r10,tgd_powner(g0)      # Set the preferred owner
#
# --- Set to cluster targets 2,3,6,7,10,11,...
#                 to targets 0,1,4,5,8,9,... respectively.
#
        clrbit  1,r9,r3
        bbs     1,r9,.crd40
#
# --- Set to no cluster in targets 0,1,4,5,8,9,...
#
        ldconst NOCLUSTER,r3
.crd40:
        stos    r3,tgd_cluster(g0)      # Set multi-port mode
#
# --- Create the world wide names for the target.
#
        call    O$createtargetWWN       # Fill in the WWNs
#
# --- Validate the FEP.
#
        PushRegs(r3)                    # Save all the registers
        ldconst 8,g1
        call    DEF_ValidateTarget
        PopRegsVoid(r3)                 # Restore the registers
#
# --- Update the FEP.
#
        mov     r9,g0
        ldconst FALSE,g1
        call    D_updrmttarg            # Update FEP
        addo    1,r6,r6                 # Bump the channel number
        b       .crd10
#
.crd70:
        addo    1,r9,r9                 # Bump target index pointer
        b       .crd20                  # Try again (note that we do not have
                                        # to check high bound since the count
                                        # guaranteed enough space)
#
# --- Update the counts and quit.
#
.crd90:
        addo    MAXISP,r11,r11
        stos    r11,tgx_ecnt(r12)
#
        call    D$cmpltserverupd        # Complete the server update to FE to
                                        #  let the FE QLogics go
#
        ldconst deok,g1                 # Load good return error code
#
.crd100:
        mov g1,g0                       # Return value
        ret
#
#**********************************************************************
#
#  NAME: O$createtargetWWN
#
#  PURPOSE:
#       To provide a mechanism to create a target WWN pair.
#
#  DESCRIPTION:
#       This function will create a pair of WWNs for a target given
#       the TGD structure.
#
#  CALLING SEQUENCE:
#       O$createtargetWWN
#
#  INPUT:
#       g0 - TGD.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
# C access
# void ON_CreateTargetWWN(TGD *pTGD)
        .globl  ON_CreateTargetWWN       # C access
ON_CreateTargetWWN:
O$createtargetWWN:
#
# --- Create the world wide names for the target.
#
        ldconst WWNFNode+(XIOOUI>>8),r4 # Get node name MSW
        ld      tgd_powner(g0),r5       # r5 = controller serial number
        ldconst (XIOOUI<<24)&0xFFFFFFFF,r6 # Get OUI LSB into MSB of LSW
        ldob    tgd_pport(g0),r13       # r13 = preferred port number
        or      r6,r5,r5                # Get it into r5
        shlo    16,r13,r3               # Shift the channel number 16 bits up
        or      r3,r4,r4                # Place it into the MSW
#
        movl    r4,r6                   # Save it for port name calculation
        bswap   r4,r4
        bswap   r5,r5
        stl     r4,tgd_nname(g0)        # Store node name
#
        ldconst WWNFNode,r3             # Mask to clear upper 3 nibbles
        andnot  r3,r6,r6                # Clear the node specific indicator
        ldconst WWNFPort,r3             # New upper bits
        or      r3,r6,r6
        bswap   r6,r6
        bswap   r7,r7
        stl     r6,tgd_pname(g0)        # Store port name
#
# --- Enable this target
#
        setbit  tarena,0,r3             # Set target enabled
        stob    r3,tgd_opt(g0)
        ret
#
#**********************************************************************
#
#  NAME: O$p2init
#
#  PURPOSE:
#       To provide a common means of performing phase II initialization
#       which includes the initialization of RAID, VIRTUAL, CACHE and
#       HOST layers.
#
#  DESCRIPTION:
#       The initialization routines for the above mentioned modules is
#       called followed by an attempt to recover any redundacized writes
#       that were in process when the system last went down.
#
#  CALLING SEQUENCE:
#       call    O$p2init
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
O$p2init:
        ldob    O_p2init,r3             # Get Phase II inits flag
        cmpobe.f TRUE,r3,.p100          # Jif already completed
#
        PushRegs                        # Save all G registers (stack relative)
#
# --- Perform phase II initializations
#
        call    R$init                  # RAID initialization
#
        call    V$init                  # VIRTUAL initialization
#
.if     MAG2MAG
        call    DLM$init                # DLM initialization
.endif  # MAG2MAG

        call    D$cmpltserverupd        # Complete the server update to FE to
                                        #  let the FE QLogics go
#
        ldconst TRUE,r3                 # Set Phase II inits completed
        stob    r3,O_p2init
.if DRIVE_POLLING
#
# --- Fork drive polling process
#
        lda     o$polldrives,g0         # Establish polling process
        ldconst ONLINEPRI,g1
c       CT_fork_tmp = (ulong)"o$polldrives";
        call    K$fork
        st      g0,o_polldrive_pcb      # Save PCB
.endif  # DRIVE_POLLING
#
# --- Establish executive process for file system update driver.
#
        lda     FS$cleanup,g0           # Establish clean up process
        ldconst FEXECPRI,g1
c       CT_fork_tmp = (ulong)"FS$cleanup";
        call    K$fork
        st      g0,f_cleanup_pcb        # Save PCB

c       GR_RetryPendingFailBacks();
#
# --- Restore Vdisk priorities
#
        call    D$rstvpri               # Restore Vdisk priorities
#
# --- Restore all global registers
#
        PopRegsVoid                     # Restore all G registers (stack relative)
#
# --- Exit
#
.p100:
        ret
#
.if DRIVE_POLLING
#**********************************************************************
#
#  NAME: o$polldrives
#
#  PURPOSE:
#       To provide a common means of checking for drives that have been
#       pulled from the system.
#
#  DESCRIPTION:
#       Each PDD is checked to see if there has been any activity to the
#       PDD in the last second.  If not, then a quick poll is done to see
#       if the drive still exists.
#
#  CALLING SEQUENCE:
#       fork o$polldrives
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
o$polldrives:
#
.pd10:
        ldconst TRUE,r3
        stob    r3,o_tur_stopped        # Task is inactive
#
.pd20:
        ldconst 2000,g0
        call    K$twait                 # Wait a few seconds
#
        ldob    O_stopcnt,r3            # Get stop count
        cmpobne 0,r3,.pd20              # Jif stopped
#
        ldob    o_hs_stopped,r3         # Check if hot swap is running
        cmpobe  FALSE,r3,.pd20          # Wait if it is
#
        ldconst FALSE,r3                # Indicate not stopped
        stob    r3,o_tur_stopped
#
        ldconst MAXDRIVES-1,r15         # Index into PDD list
        ldconst 0,r14                   # Preset in case we don't start any TURs
#
.pd30:
        ld      P_pddindx[r15*4],g3     # Get PDD
        cmpobe  0,g3,.pd40              # Jif no drive
.ifndef MODEL_3000
.ifndef  MODEL_7400
        ld      pd_dev(g3),r3           # get DEV
        cmpobe  0,r3,.pd40              # check for null
        ldob    dv_chn(r3),r4
        cmpoble MAXCHN,r4,.pd40
.endif  # MODEL_7400
.endif  # MODEL_3000
#
        ldob    pd_flags(g3),r4         #get the flags
        ldob    pd_miscstat(g3),r5      # Get misc status
        bbs     pdmbfserror,r5,.pd35    # force tur when fs error so we can fix the fs if functional
        bbs     pdbebusy,r4,.pd35       # if we are busy send the testur regardless of activity
        ld      pd_rps(g3),r4           # Get requests per second
        cmpobne 0,r4,.pd40              # Jif activity
.pd35:
#
# --- Ping the drive since there was no activity in the last second.  Do this
# --- by generating the request and queueing it with a custom completion
# --- routine.
#
        bbs     pdmbmissing,r5,.pd40    # Jif already missing
#
        ldob    pd_devstat(g3),r5       # Get device status
        cmpobe  pdnonx,r5,.pd40         # Jif non-existent
#
        ld      o_polldrivecnt,r14      # Get the counter
        addo    1,r14,r14               # Bump the count
        st      r14,o_polldrivecnt      # Save the counter
#
        lda     O_t_testurdy,g0         # Pass test unit ready template
        call    O$genreq                # Generate request
#
# --- Queue request w/o wait
#
        lda     P$que,g0                # Pass queuing routine
                                        # g1 has ILT from gen req
        lda     o$poll_drv_cmplt,g2     # Complete by releasing
        call    K$q                     # Queue request w/o wait
#
.pd40:
        subo    1,r15,r15               # Decrement index into PDD table
        cmpible 0,r15,.pd30             # Do more drives
#
# Now do the drive bays that are directly addressable.
#
        ldconst MAXSES-1,r15            # Index into Enclosure list
#
.pd50:
        ld      E_pddindx[r15*4],g3     # Get PDD of the bay
        cmpobe.f 0,g3,.pd60             # Jif no bay
.ifndef MODEL_3000
.ifndef  MODEL_7400
        ld      pd_dev(g3),r3           # get DEV
        cmpobe  0,r3,.pd60              # check for null
        ldob    dv_chn(r3),r4
        cmpoble MAXCHN,r4,.pd60
.endif  # MODEL_7400
.endif  # MODEL_3000
#
        PushRegs(r3)
        mov     g3,g0                   # Set input parm
        call    SES_DirectlyAddressable #  Check if bay is directly addressable
        PopRegs(r3)
        cmpobne TRUE,g0,.pd60           # Jif to next one if not direct address
#
# --- Generate the request and queueing it with a custom completion
# --- routine.  Go to sleep and the last completion routine will wake up the
# --- driver function (this function).
#
        ldob    pd_miscstat(g3),r5      # Get misc status
        bbs     pdmbmissing,r5,.pd60    # Jif already missing
#
        ldob    pd_devstat(g3),r5       # Get device status
        cmpobe  pdnonx,r5,.pd60         # Jif non-existent
#
        ld      o_polldrivecnt,r14      # Get the counter
        addo    1,r14,r14               # Bump the count
        st      r14,o_polldrivecnt      # Save the counter
#
        lda     O_t_testurdy,g0         # Pass test unit ready template
        call    O$genreq                # Generate request
#
# --- Queue request w/o wait
#
        lda     P$que,g0                # Pass queuing routine
                                        # g1 has ILT from gen req
        lda     o$poll_drv_cmplt,g2     # Complete by releasing
        call    K$q                     # Queue request w/o wait
#
.pd60:
        subo    1,r15,r15               # Decrement index into PDD table
        cmpible 0,r15,.pd50             # Do more drives
#
        cmpobe  0,r14,.pd10             # Go to top if no commands started.
#
c       TaskSetMyState(pcnrdy);         # Set task to go to sleep
        call    K$xchang
        b       .pd10
#
#**********************************************************************
#
#  NAME: o$poll_drv_cmplt
#
#  PURPOSE:
#       To provide a common means of completing the polling of a drive.
#
#  DESCRIPTION:
#       The specified drive polling has completed.  Check the status of
#       the TUR and act accordingly.  If there are no commands left then
#       wake up the driver function.
#
#  CALLING SEQUENCE:
#       process call
#
#  INPUT:
#       g1 = ILT of TUR
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
o$poll_drv_cmplt:
#
# --- Check return status
#
        ld      il_w0(g1),g2            # Get PRP
        call    M$chkstat               # Check status
        cmpobne ecok,g0,.pdt00          # Jif error
.ifndef MODEL_3000
.ifndef  MODEL_7400
        ld      pr_dev(g2),r3           # Get the dev pointer
        ld      dv_pdd(r3),r3           # Get PDD

        ldob    pd_flags(r3),r4
        bbs     pdbebusy,r4,.pdt00_5    # Jif  BUSY

        ldob    pd_miscstat(r3),r4      # Get misc status
        bbc     pdmbfserror,r4,.pdt90   # Jif not file system error
c       TaskReadyByState(pcfscleanup);  # Enable file system cleanup
        b       .pdt90

#
.pdt00_5:
c if (((DEV*)g2)->TimetoFail > 90) { # Special emulated PAB
        b       .pdt00                  # continue with busy.
c }
        PushRegs(r4)                    # Save registers
        ldos    pd_pid(r3),g0           # PID
        call    ON_TURSuccess
        PopRegsVoid(r4)                 # Restore registers
.endif  # MODEL_7400
.endif  # MODEL_3000
        b       .pdt90
#
# --- Process error
#
.pdt00:
.ifndef MODEL_3000
.ifndef  MODEL_7400
#
# --- Ignore the error if the PID is busy
#
        ld      pr_dev(g2),r3           # Get the dev pointer
        ld      dv_pdd(r3),r3           # Get PDD
        ldob    pd_flags(r3),r4
        bbs     pdbebusy,r4,.pdt90      # Jif BUSY
#
        cmpobe  ecbebusy,g0,.pdt90      # Jif error
.endif  # MODEL_7400
.endif  # MODEL_3000
        ldconst ecnonxdev,r3            # Check for non-existent device
        ldconst eclgoff,r4              # Check for logged off device -
                                        # Consider either one as nonexistent
        cmpobe  r3,g0,.pdt10            # Jif so
        cmpobe r4,g0,.pdt90             # Jif not so
#
.pdt10:
        ldconst pdnonx,r4               # Set status to non-existent
        ld      pr_dev(g2),r12          # Get the dev pointer
#
        ld      dv_pdd(r12),g3          # Get PDD
        cmpobe  0,g3,.pdt90             # no pdd pointer bail out
c fprintf(stderr, "%s%s:%u pid=%d setting device nonexistent, EC status=0x%lx\n", FEBEMESSAGE, __FILE__, __LINE__,((PDD*)((DEV*)g2))->pid,g0);
        stob    r4,pd_devstat(g3)       # Save in PDD
##
        ldob    pd_flags(g3),r4
        bbs     pduserspundown,r4,.pdt11
        b       .pdt12
#
.pdt11:
        clrbit pduserspundown,r4,r4
        stob   r4,pd_flags(g3)
.pdt12:
        bbc     pduserfailed,r4,.pdt13 # Jif user not failed pdisk
        clrbit  pduserfailed,r4,r4
        stob    r4,pd_flags(g3)
.pdt13:
##
#
        ldob    pd_miscstat(g3),r4      # Get misc status
        bbs     pdmbmissing,r4,.pdt15   # Jif already posted as missing
#
        setbit  pdmbmissing,r4,r4
        stob    r4,pd_miscstat(g3)      # Save it
.ifndef MODEL_3000
.ifndef  MODEL_7400
        ldconst 0xFFFF,r4               # Set the SES index to no enclosure
        stos    r4,pd_ses(g3)
        stob    r4,pd_slot(g3)
.endif  # MODEL_7400
.endif  # MODEL_3000
#
        ldob    pd_devtype(g3),r3       # Get the type
        cmpobl  pddtmaxdisk,r3,.pdt25   # Jif not a disk (kick off back scan)
#
        call    O$log_drive_removed     # Log event
#
# --- Search through the SES devices to see if this was the portal to that
# --- enclosure.  If it was, then kick off a background polling to find all
# --- of the enclosures again.
#
.pdt15:
        ldconst MAXSES-1,r15            # Index into list
#
.pdt20:
        ld      E_pddindx[r15*4],r14    # Get the PDD for the enclosure
        cmpobe  0,r14,.pdt30            # Jif no enclosure

#
        ld      pd_dev(r14),r13         # Get device pointer
        cmpobne r13,r12,.pdt30          # Jif not the one that got pulled
#
.pdt25:
        mov     TRUE,r3
        stob    r3,o_polldrivebg        # Set to start back ground processing
        b       .pdt90                  # Outta here
#
.pdt30:
        subo    1,r15,r15               # Decrement index
        cmpible 0,r15,.pdt20            # Jif more to do
#
.pdt90:
        call    O$relreq                # Give back the memory
#
        ld      o_polldrivecnt,r3       # Get the counter
        subo    1,r3,r3
        st      r3,o_polldrivecnt       # Save the counter
#
        cmpobne 0,r3,.pdt100            # Exit if more commands left
#
        ldob    o_polldrivebg,r3        # Check if we need to start back ground
        cmpobne TRUE,r3,.pdt95          # Jif no missing detected
#
        call    SES_StartBGProcess      # Kick off a discovery
        mov     FALSE,r3                # Reset the flag
        stob    r3,o_polldrivebg
#
.pdt95:
        ld      o_polldrive_pcb,r4
        ldob    pc_stat(r4),r6
        cmpobne r6,pcnrdy,.pdt100       # If not not ready process
        ldconst pcrdy,r3                # Enable the driver
.ifdef HISTORY_KEEP
c CT_history_pcb(".pdt96 setting ready pcb", r4);
.endif  # HISTORY_KEEP
        stob    r3,pc_stat(r4)
#
.pdt100:
        ret
.endif  # DRIVE_POLLING
#
#**********************************************************************
#
#  NAME: o$hotsparecheck
#
#  PURPOSE:
#       To provide an automated means of periodically testing all inactive
#       hotspare devices.
#
#  DESCRIPTION:
#       The POSTs are called periodically for each inactive hotspare
#       that is configured into the system.  This is done to insure that
#       any given hotspare is operational in the event that it is needed.
#
#  CALLING SEQUENCE:
#       process call
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
o$hotsparecheck:
#
# --- Wait for delay period
#
.ohs00:
        ldconst OHOTSPARECHECKTIME,g0   # Wait for delay period
        call    K$twait
#
# --- Prepare to scan PDDs
#
        ldconst MAXDRIVES-1,r14         # Set the index into the table
#
# --- Find a valid PDD
#
.ohs10:
        ld      P_pddindx[r14*4],g3     # Advance to next PDD
        cmpobe  0,g3,.ohs30             # Jif NULL - next PDD
#
# --- Examine the PDD
#
        ldob    pd_class(g3),r5         # Get device class
        cmpobne pdhotlab,r5,.ohs30      # Jif not a hotspare - next PDD
#
        ldob    pd_devstat(g3),r5       # Get device status
        cmpobne pdop,r5,.ohs30          # Jif not operable - next PDD
#
# --- Found a hot spare to check.  Perform POSTs.
#
        lda     O_drvinits,g8           # Get inits in progress pointer
        ld      (g8),r3                 # Get current number waiting init
        addo    1,r3,r3                 # Add one more device
        st      r3,(g8)
#
        ldconst 3,g2                    # regular bringup (disk testing)
        call    O$init_drv              # g3 has PDD, g8 has ptr to counter
#
        ldob    pd_devstat(g3),r5       # Get updated PDD status
        cmpobe  pdop,r5,.ohs30          # Jif operable - next PDD
#
# --- Log hotspare inoperable
#
        call    o$log_hotspareinop      # Log event
#
# --- Advance to next PDD
#
.ohs30:
        subo    1,r14,r14               # Decrement index
        cmpible 0,r14,.ohs10            # Check next one
        b       .ohs00                  # Else, delay and wait for timer
#
#**********************************************************************
#
#  NAME: O$init_drv
#
#  PURPOSE:
#       To provide a common means of initializing and spinning up a
#       single disk drive.
#
#  DESCRIPTION:
#       The designated drive is treated to the standard initialization
#       sequence which includes retrieving inquiry data, spinning up
#       the unit, running diagnostics, write/read buffer testing and
#       setting up standard mode select parameters.
#
#       The concurrency count is used to control the maximum number
#       of outstanding start unit commands that may be executing
#       simultaneously.  This routine decrements this count prior to
#       issuing a start unit command and only proceeds if that count
#       is positive.  After the start unit completes this count is
#       incremented.
#
#  CALLING SEQUENCE:
#       process call
#           or
#       call    O$init_drv
#
#  INPUT:
#       g2 = Test type 1 = Quick - inquiry only
#                      2 = no drive test
#                      3 = full test
#       g3 = PDD
#       g8 = byte ptr to number of inits currently waiting completion
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
# C access
# void ON_InitDrive(PDD* pPDD, UINT32 type, void* pInitCount);
        .globl  ON_InitDrive
ON_InitDrive:
        mov     g0,g3                   # PDD
        mov     g2,g8                   # Count pointer
        mov     g1,g2                   # Type
# fall through
#
O$init_drv:
#
.if     DEBUG_FLIGHTREC_O
        ldconst frt_h_initdrv0,r3       # Init drive function - start
        st      r3,fr_parm0             # Function
        ldos    pd_pid(g3),r3
        st      r3,fr_parm1             # PID
        st      g3,fr_parm2             # PDD of drive being initialized
        st      g2,fr_parm3             # Test type
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_O
#
        mov     g2,r15                  # Save the partial/full indicator
#
        mov     g8,r12                  # Store semaphore address someplace safe
#
        ldconst pdaprespin,r4           # Set status to active pre-spinup
        stob    r4,pd_poststat(g3)
#
        lda     pdlitefail,r4           # Assume failure
        stob    r4,pd_fled(g3)          # But do not update the light status
#
# --- Issue standard inquiry command ----------------------------------
# For some reason we observed that, Inquiry being failed in 7000
# But actually the inquiry command is failed from our physical layer
# after detecting that device error count on this dev. At this point
# the target may be in  good or bad state/condition.  This will be
# known only if we send  the inquiry command down to the target.
# So for 7000 we will retry the  inquiry commad, if there is really an
# issue with the target, this  command will be failed anyway from the
# target, and this pdisk will be  marked as inop.

        ldconst INQRETRY,r3                    # Max Inquiry retries
.oin05:
        lda     O_t_inquiry,g0          # Pass standard inquiry template
        call    O$genreq                # Generate request
        call    O$quereq                # Queue request
        call    M$chkstat               # Check status
        cmpobne ecok,g0,.oin06          # Jif not error
.ifndef MODEL_3000
.ifndef  MODEL_7400
        ld       pd_dev(g3),r4          # get the device from pdd
        cmpobe   0,r4,.oin07            # Jif dev is Null
        ldconst  0,r5
        stob     r5,dv_physerr(r4)      # Clear physical device error count
.endif  # MODEL_7400
.endif  # MODEL_3000
        b       .oin07
#
.oin06:
        cmpobe  ecnonxdev,g0,.oin20     # Jif non-existent device
        cmpobe  eclgoff,g0,.oin20       # Jif port logoff (treat as non-existent)
        ldconst eccheck,r4              # Check for check condition
        cmpobne r4,g0,.oin20            # Jif not

        ldob    pr_sense+2(g2),r4       # Get sense key and isolate
        and     0x0f,r4,r4
        cmpobne 6,r4,.oin20             # Jif not unitattention

        subo    1,r3,r3
c fprintf(stderr, "%s%s:%u Inquiry failed with status %lx retries left %lx\n", FEBEMESSAGE, __FILE__, __LINE__,g0,r3);
        cmpobe  0,r3,.oin20
        call    O$relreq                # Release previous request
        b       .oin05
#
.oin07:
        cmpobne 1,r15,.oin10            # Jif not a quick test
#
        ldob    pd_poststat(g3),r4      # Get previous post status
        ldconst (pdop<<8),r3            # Set devstat = operable
        or      r3,r4,r4                # Merge post and dev stat
        b       .oin9080                # Done
#
# --- Update PDD w/ inquiry information
#
.oin10:
        ld      pr_sglptr(g2),r3        # Locate SGL
        ld      sg_desc0+sg_addr(r3),r3 # Locate inquiry buffer
#
        lda     8(r3),g4                # Pass vendor ID found
        lda     O_spaces,g5             # Pass string of spaces
        ldconst 8,g6                    # Pass vendor ID length
c       g0 = !memcmp((void*)g4, (void*)g5, 8);
        cmpobe  TRUE,g0,.oin12          # Jif spaces
#
        lda     16(r3),g4               # Pass product ID found
        lda     O_spaces,g5             # Pass string of spaces
        ldconst 8,g6                    # Pass product ID length
c       g0 = !memcmp((void*)g4, (void*)g5, 8);
        cmpobe  TRUE,g0,.oin12          # Jif spaces
#
        ldl     8(r3),r4                # Set up vendor ID
        stl     r4,pd_vendid(g3)
#
        ldq     16(r3),r4               # Set up product ID
        stq     r4,pd_prodid(g3)
#
        ld      32(r3),r4               # Set up revision
        st      r4,pd_rev(g3)
#
        movq    0,r4                    # Clear device capacity and s/n
        stl     r4,pd_devcap(g3)
        stt     r4,pd_serial(g3)
#
.oin12:
        stob    PD_DT_UNKNOWN,pd_devtype(g3) # Default to unknown device, not what is returned by inquiry!
#
.ifndef MODEL_3000
.ifndef  MODEL_7400
        ldos    pd_lun(g3),r3           # Get the LUN
        cmpobe  0,r3,.oin13             # Jif the PDD corresponds to LUN 0
        PushRegs(r3)                    # Save registers
        mov     g3,g0                   # Set up input
        call    ISE_GetVolumeInfo       # Get the volume information
        PopRegs(r3)                     # Restore registers
        cmpobe  0,g0,.oin13             # Check the return status
        ldconst (pdinop<<8)+pdfinq,r4   # Set status to failed Inquiry
        b       .oin9080
#
.oin13:
.endif  # MODEL_7400
.endif  # MODEL_3000
        PushRegs(r3)                    # Save registers
        mov     g3,g0                   # Set up input
        call    SES_GetDeviceType       # Get the device type from tables
        PopRegs(r3)                     # Restore registers
#
        bbc     pddtkludgeecon,g0,.oin15 # If not kludging detune, continue
        clrbit  pddtkludgeecon,g0,g0    # Else, clear the bit
        call    P$DegradePerf           # Call to set flag to degrade perf
#
.oin15:
        stob    g0,pd_devtype(g3)       # Set device type
c       PHY_SetMaxTags(g3);
        b       .oin100
#
# --- Process error
#
.oin20:
        cmpobe  1,r15,.oin30            # Jif just a quick test
        movq    0,r4                    # Clear device capacity
        stl     r4,pd_devcap(g3)        #  ID, product ID, revision and
#
.oin30:
#
# --- Check for non-existent device
#
        ldconst ecnonxdev,r3            # Check for non-existent device
        ldconst eclgoff,r4              # Check for logged off device -
                                        #   Consider either one as nonexistent
        cmpobe  r3,g0,.oin40            # Jif so
        cmpobe  r4,g0,.oin40            # Jif so
#
        ldconst (pdinop<<8)+pdfinq,r4   # Set status to failed Inquiry
        b       .oin9080
#
# --- Process non_existent device
#
.oin40:
        ldconst (pdnonx<<8)+pdnonx,r4   # Set dev/post status to non-existent
        b       .oin9080
#
# --- Set the maximum number of times test unit ready is called
#     Set a longer delay for initial bringup and short delay for runtime
#
.oin100:
        ldconst 0,r10                   # Set loop counter
        ldconst 90*4,r11                # Number of TUR retry loops
                                        #  n seconds * 4 retries/second
        cmpobe  3,r15,.oin200           # Jif a full test
        ldconst 5*4,r11                 # Number of TUR retry loops =
                                        #  n seconds * 4 retries/second
#
# --- Issue Test Unit Ready -------------------------------------------
#
.oin200:
        call    O$relreq                # Release previous request
#
.oin210:
        addo    1,r10,r10               # Increment Loop Counter.
        lda     O_t_testurdy,g0         # Pass test unit ready template
        call    O$genreq                # Generate request
        call    O$quereq                # Queue request
        call    M$chkstat               # Check status
        cmpobe  ecok,g0,.oin400         # Jif OK
.ifndef MODEL_3000
.ifndef  MODEL_7400
        ldob    pd_flags(g3),r3
        bbs     pdbebusy,r3,.oin400     # Treat this as success
.endif  # MODEL_7400
.endif  # MODEL_3000
#
# --- Check for unit not ready
#
        ldconst eccheck,r3              # Check for check condition
        cmpobne r3,g0,.oin240           # Jif not
#
        ldob    pr_sense+2(g2),r5       # Get sense key and isolate
        and     0x0f,r5,r5
        cmpobe  6,r5,.oin230            # Jif 'Unit Attention'
        cmpobne 2,r5,.oin240            # Jif not 'Not Ready' error
#
        ldconst 0x0104,r4               # LUN is in process of becoming ready
        ldos    pr_sense+12(g2),r5      # Get ASC/ASCQ
        cmpobe  r4,r5,.oin220           # Jif if not becoming ready
#
        ldconst 0x0704,r4               # LUN is in process of becoming ready
        cmpobne r4,r5,.oin300           # Jif if not becoming ready
.ifndef MODEL_3000
.ifndef  MODEL_7400
        ldconst 0x0304,r4
        cmpobne r4,r5,.oin245           # Jif if not becoming ready
.endif  # MODEL_7400
.endif  # MODEL_3000
#
.oin220:
        ldconst pdaspin,r4              # Set status to active spinup
        stob    r4,pd_poststat(g3)
#
        ldconst (pdinop<<8)+pdaspin,r4  # Set status to failed and spinning up
        cmpobge r10,r11,.oin250         # Jif greater than timeout - error
#
        ldconst 250,g0                  # Wait 1/4 second.
        call    K$twait
#
# --- At this point, we are in a polling loop waiting for the drive to
# --- spin up.  To protect against a file system operation coming in and
# --- timing out which can cause an election failure and suicide, check
# --- to see if the file system has an op pending and if so, reduce the
# --- time we wait to under 15 seconds.
#
        ld      gFSOpWaiting,r3         # Get current FS op status
        cmpobe  FALSE,r3,.oin230        # Not waiting, continue
#
        ldconst 15*4,r3                 # 15 seconds
        subo    r10,r11,r5              # Get time left
        cmpobl  r5,r3,.oin230           # Jif less that 15 seconds left
#
        subo    r3,r11,r10              # Else set timer to total time - 15 sec
#
.oin230:
        call    O$relreq                # Release previous request
        cmpobl  r10,r11,.oin210         # Jif less than timeout period - retry
#
# --- Process error
#
.oin240:
        ldconst (pdinop<<8)+pdftur,r4   # Set status to failed Test Unit Rdy
        b       .oin250

.ifndef MODEL_3000
.ifndef  MODEL_7400
.oin245:
        ldconst (pdinop<<8)+pdiseluninop,r4   # Set status to failed Test Unit Rdy
.endif  # MODEL_7400
.endif  # MODEL_3000
#
.oin250:
        ldconst HS_DELAY,r5             # Delay this long before retrying
        st      r5,o_retryhotswap_delay
#
        ld      O_retryhotswap_pcb,r5
        cmpobne 0,r5,.oin9080           # Jif task already active
#
c       g0 = -1;                        # Flag that task is being created.
        st      g0,O_retryhotswap_pcb
        lda     o$retry_hotswap,g0      # Fork process to hotspare drive
        mov     g1,r5                   # Save g1
        ldconst OINQPRIO,g1
c       CT_fork_tmp = (ulong)"o$retry_hotswap";
        call    K$tfork
        st      g0,O_retryhotswap_pcb
        mov     r5,g1
        b       .oin9080                # Exit
#
# --- Issue start unit ------------------------------------------------
#
.oin300:
        ldob    pd_devtype(g3),r5       # Get device type
        cmpobl  pddtmaxdisk,r5,.oin400  # Jif not a disk
#
        call    O$relreq                # Release previous request
#
        ldconst (pdop<<8)+pdaspin,r4    # Set status to active spinup
        stos    r4,pd_poststat(g3)
#
        lda     O_t_startunit,g0        # Pass start unit template
        call    O$genreq                # Generate request
        call    O$quereq                # Queue request
        call    M$chkstat               # Check status
        cmpobe  ecok,g0,.oin220         # Jif OK
#
# --- Process error
#
        ldconst (pdinop<<8)+pdfstart,r4 # Set status to failed Start Unit
        b       .oin9080
#
# --- Issue serial number inquiry page --------------------------------
#
.oin400:
        ldconst pdapstspin,r4           # Set status to active post-spinup
        stob    r4,pd_poststat(g3)
#
        call    O$relreq                # Release previous request
#
        ldconst (pdop<<8)+pdapstspin,r4 # Set status to active post-spinup
        stos    r4,pd_poststat(g3)
#
        ldob    pd_miscstat(g3),r3
        clrbit  pdmbspindown,r3,r3      # Clear the spin down bit
        stob    r3,pd_miscstat(g3)
#
.ifndef MODEL_3000
.ifndef MODEL_7400
c       if (((PDD*)g3)->lun == 0) {
        lda     O_t_ise_l_s_p_30,g0     # Pass ISE log sense page 0x30.
c       } else {
        lda     O_t_sninquiry,g0        # Pass s/n inquiry template
c       }
.endif  # MODEL_7400
.endif  # MODEL_3000
.ifndef MODEL_7000
.ifndef MODEL_4700
        lda     O_t_sninquiry,g0        # Pass s/n inquiry template
.endif  # MODEL_4700
.endif  # MODEL_7000
        call    O$genreq                # Generate request
        call    O$quereq                # Queue request
        call    M$chkstat               # Check status
        cmpobe  ecok,g0,.oin500         # Jif OK
#
# --- Check for SAS Expander
#
        ldob    pd_devtype(g3),r3       # Get device type
        cmpobne pddtsasexp,r3,.oin450   # Jif not sas expander to error
c       sprintf((char *)((PDD*)g3)->serial, "%-16.12qx", (((PDD*)g3)->wwn & 0xFCFFFFFFFFFFFFFFLLU) >> 16);
        b       .oin525                 # Continue
#
# --- Process error
#
.oin450:
        ldconst (pdinop<<8)+pdfsninq,r4 # Set status to failed S/N Inquiry
        b       .oin9080
#
# --- Prepare to extract s/n
#
.oin500:
        ld      pr_sglptr(g2),r3        # Locate SGL
        ld      sg_desc0+sg_addr(r3),r3 # Locate s/n inquiry buffer
#
        ldconst 0x20202020,r8           # Blank fill serial number area
        mov     r8,r9
        mov     r8,r10
        stt     r8,pd_serial(g3)
#
.ifndef MODEL_3000
.ifndef  MODEL_7400
c   if (((PDD*)g3)->lun == 0) {
# Copy 8 bytes of the serial number from page 30 byte 40 (struct LOG_ISE_PAGE.serial_number[32]).
c       memmove((void*)(g3+pd_serial), (void*)(r3+32+8), 8);
c   } else {
.endif  # MODEL_7400
.endif  # MODEL_3000
#
# --- Check for both fibre drives and economy enterprise drives. If found,
# --- then pull the serial number from the first eight bytes of the serial
# --- number.  If not, then grab from the end up to 12 bytes.
#
        ldob    pd_devtype(g3),r5       # Get the type
        cmpobe  pddtfcdisk,r5,.oin502   # Jif fibre
        cmpobne pddteconent,r5,.oin505  # Not Economy enterprise
#
# --- Limit to drive S/N ignoring PCB S/N (Seagate fibre)
#
.oin502:
        ldconst 8,r4                    # Limit to drive S/N
        lda     4(r3),r3                # Leftmost position of S/N returned
        b       .oin510
#
# --- Extract maximum of 12 characters
#
.oin505:
        ldob    3(r3),r4                # Get page length
        lda     4(r3),r3                # Leftmost position of S/N returned
        cmpobge 12,r4,.oin510           # Jif s/n sized OK
#
        addo    r3,r4,r3                # Move to the end of the string
        ldconst 12,r4                   # Truncate to 12 bytes
        subo    r4,r3,r3                # Move back up (fetch last 12 bytes)
#
.oin510:
        lda     pd_serial(g3),r5        # LSB position of PDD S/N
#
.oin520:
        ldob    (r3),r6                 # Get next byte
        stob    r6,(r5)                 # Store next byte
        addo    1,r3,r3                 # Advance src/dst pointers
        lda     1(r5),r5
        subo    1,r4,r4                 # Compute remaining bytes
        cmpible 1,r4,.oin520            # Jif more
.ifndef MODEL_3000
.ifndef  MODEL_7400
c   }
.endif  # MODEL_7400
.endif  # MODEL_3000
#
.oin525:
        ldconst (pdop<<8)+pdop,r4       # Set status operable
#
        ldob    pd_devtype(g3),r5       # Get device type
        cmpobge pddtmaxdisk,r5,.oin530  # Jif a disk
#
        ldconst pdliteoff,r3            # Turn off led
        stob    r3,pd_fled(g3)
        b       .oin9080                # Release request and exit
#
# --- Issue read capacity ---------------------------------------------
#
.oin530:
        call    O$relreq                # Release previous request
#
        lda     O_t_rdcap,g0            # Pass read capacity template
        call    O$genreq                # Generate request
        call    O$quereq                # Queue request
        call    M$chkstat               # Check status
        cmpobe  ecok,g0,.oin600         # Jif OK
#
# --- Process error
#
        ldconst (pdinop<<8)+pdfrcap,r4  # Set status to failed Read Capacity
        b       .oin9080
#
# --- Update PDD w/ device capacity information (ignore raid devices for now)
# --- They'll be updated as we finish our NVRAM restore.
#
.oin600:
        ld      pr_sglptr(g2),r3        # Locate SGL
        ld      sg_desc0+sg_addr(r3),r4 # Locate buffer
#
        ld      (r4),r3                 # Get highest address
        bswap   r3,r4                   # Convert endians
# If r4 is all bits on, then try a read capacity 16 command.
c   if (r4 == 0xffffffff) {
        call    O$relreq                # Release previous request
        lda     O_t_rdcap_16,g0         # Pass read capacity template
        call    O$genreq                # Generate request
        call    O$quereq                # Queue request
        call    M$chkstat               # Check status
c     if (g0 != ecok) {
c         *((UINT64*)r4) = 0xffffffff + 1ULL;  # Go back to all bits on.
        ldconst (pdinop<<8)+pdfrcap,r4  # Set status to failed Read Capacity
        b       .oin9080
c     } else {
        ld      pr_sglptr(g2),r3        # Locate SGL
        ld      sg_desc0+sg_addr(r3),r3 # Locate buffer
        ldl     (r3),r8                 # r8/r9
# The return is all byte swapped "interestingly".
        mov     r8,r7                   # move r4
        bswap   r9,r4                   # byte swap r5 into r4
        bswap   r7,r5                   # byte swap r4 (in r7) into r5.
c       *((UINT64*)&r4) = *((UINT64*)&r4) + 1;  # Last block is one less than capacity.
c     }
c   } else {
c       *((UINT64*)&r4) = r4 + 1ULL;    # Adjust to number of blocks.
c   }
c       ((PDD*)g3)->devCap = *((UINT64*)&r4);
#
# --- Issue verify of reserved area -----------------------------------------
#
        cmpobe  2,r15,.oin720           # Jif just a partial, check file system
.ifndef MODEL_3000
.ifndef  MODEL_7400
        ldob    pd_flags(g3),r3
        bbc     pdbebusy,r3,.oin700     # Bypass Fsys operations
        ldconst (pdop<<8)+pdop,r4       # Set status to operative
        b       .oin9080
#
.oin700:
        # skip verify its not that useful on a 7000 and it takes forever.
#--        b   .oin720
.endif  # MODEL_7400
.endif  # MODEL_3000
.ifndef  MODEL_7000
.ifndef  MODEL_4700
        call    O$relreq                # Release previous request
#
        ldconst 0,r4                    # Starting LBA = 0
        ldconst FSUPDATELEN,r5          # Ending LBA
#
        lda     O_t_verifyrsvd,g0       # Pass verify reserved area template
        call    O$genreq                # Generate request
#
.oin705:
        bswap   r4,r3                   # Get into correct order for LBA
        st      r3,pr_cmd+2(g2)         # Set LBA
#
        call    O$quereq                # Queue request
        call    M$chkstat               # Check status
        cmpobne ecok,g0,.oin710         # Jif not OK
#
# --- Bump LBA and check for done
#
        ldconst ((SYSSRESERVE)>>3),r3   # Length of IO (16 MB)
        addo    r3,r4,r4
        cmpobne r4,r5,.oin705           # Jif more to do
#
        b       .oin720                 # Jif OK
#
# --- Process error
#
.oin710:
        ldconst (pdinop<<8)+pdfver,r4   # Set status to failed verify
        b       .oin9080
#
# --- Read file system directory --------------------------------------
#
# --- Check for a valid file system.  If one is found, then do a write/read
# --- compare on the scratch file.  From this point on, the device will be
# --- considered operable if a "failure" is detected which would still allow
# --- the device to be labelled.
#
.endif  # MODEL_7000
.endif  # MODEL_4700
.oin720:
        call    O$relreq                # Release previous request
#
# --- Try the FS$VerifyDirectory three times -- in case it is being written at
# --- the same time. The BE does not prevent reads and writes to same sector(s)
# --- from happening at the same time, or hdatoken rearranging the read/write
# --- order, etc.
#
c       r3 = 3;                         # Number of times to retry FS$VerifyDirectory
.oin725:
c       g0 = r3;
        call    FS$VerifyDirectory      # Check for a valid directory
        cmpobe  TRUE,g0,.oin730         # Jif valid directory
c       r3--;
c       if (r3 > 0) {
c         fprintf(stderr, "%sonline.as:%u FS$VerifyDirectory pid=%d failed %lx retries left\n", FEBEMESSAGE, __LINE__, ((PDD*)g3)->pid, r3);
c         TaskSleepMS(125);             # Sleep for 125 milliseconds (1/8th second).
          b       .oin725               # Try reading Directory three times (3/8th second).
c       }
#
# --- Process error. Not having a file system makes the device inoperative.
# --- A pdisklabel operation (0xfe or 0xff) or pdiskrestore may make operable.
#
        lda     pdliteoff,r3
        stob    r3,pd_fled(g3)          # Turn off the fail light
#
        ldconst (pdinop<<8)+pdfdir,r4   # Set status to failed directory
        b       .oin9090
#
# --- Test to see if this is a full init or a partial.  A partial init will
# --- not do these tests.  A full test will.  r15 has the test indicator.
#
.oin730:
        cmpobe  2,r15,.oin810           # Jif just a partial, check label
#
# --- Issue write to scratch buffer -----------------------------------
#
        ldconst SCRATCHSIZE*SECSIZE,r14 # Allocate the memory
c       g0 = s_MallocW(r14, __FILE__, __LINE__);
#
        mov     g0,r15                  # Save the address
        mov     r14,g1                  # Size to generate
        call    o$gendpat               # Generate data pattern
#
        ldconst fidscratch,g0           # File to write
        mov     r15,g1                  # Starting buffer address
        ldconst SCRATCHSIZE,g2          # Blocks to write
                                        # g3 still has the PDD
        ldconst 1,g4                    # Block offset to start at
        call    FS$WriteFile            # Write it to the disk
        cmpobe  0,g0,.oin740            # Jif OK
#
# --- Process error
#
        ldconst (pdinop<<8)+pdfwbuff,r4 # Set status to failed Write Buffer
        b       .oin9070
#
# --- Issue read to scratch buffer ------------------------------------
#
.oin740:
c       s_Free(r15, r14, __FILE__, __LINE__);
#
        ldconst SCRATCHSIZE*SECSIZE,r14 # Allocate the memory
c       r15 = s_MallocC(r14, __FILE__, __LINE__);
#
        ldconst fidscratch,g0           # File to read
        mov     r15,g1                  # Starting buffer address
        ldconst SCRATCHSIZE,g2          # Blocks to read
                                        # g3 has the PDD
        ldconst 1,g4                    # Block offset to start at
        call    FS$ReadFile             # Read the file
        cmpobe  0,g0,.oin750            # Jif OK
#
# --- Process error
#
        ldconst (pdinop<<8)+pdfrbuff,r4 # Set status to failed Read Buffer
        b       .oin9070
#
# --- Check data pattern ----------------------------------------------
#
.oin750:
        mov     r15,g0                  # Restore the pointer
        mov     r14,g1                  # Size of compare
        call    o$chkdpat               # Check data pattern
        cmpobe  TRUE,g0,.oin760         # Jif OK
#
# --- Process error
#
        ldconst (pdinop<<8)+pdfdpat,r4  # Set status to failed data pattern
        b       .oin9070
#
# --- Process mode sense/select Fibre Channel Interface Control page --
#
.oin760:
        PushRegs(r3)                    # Save registers
        lda     O_t_msfcicp,g0          # Pass FC I/F control page (19h)
        mov     g3,g1                   # PDD
        ldconst 0x00000000,g2           # Pass bits to set (none)
        ldconst 0x00000000,g3           # Pass bits to clear (all)
        call    ON_ModeSenseSelect      # Set the page
        PopRegs(r3)                     # Restore them
#
# --- Read device label -----------------------------------------------
#
c       s_Free(r15, r14, __FILE__, __LINE__);
#
.oin810:
        ldconst LABELSIZE*SECSIZE,r14   # Allocate memory
c       r15 = s_MallocC(r14, __FILE__, __LINE__);
#
        ldconst fidlabel,g0             # Read the label file
        mov     r15,g1                  # Into the buffer just allocated
        mov     LABELSIZE,g2            # For this many blocks
                                        # g3 still has the PDD
        ldconst 1,g4                    # Block offset to start at
        call    FS$ReadFile             # Read the file
#
        cmpobe  0,g0,.oin910            # Jif OK
#
# --- Process error
#
        ldconst (pdinop<<8)+pdfrdlab,r4 # Set status to failed rd of label
        b       .oin9070
#
# --- Check for XIOtech device label
#
.oin910:
        lda     O_devlab,g4             # Pass label template
        ldconst o_devlablen,g6          # Pass template length
        mov     r15,g7                  # Get read buffer address
        lda     xd_text(g7),g5          # Pass device label text
c       g0 = !memcmp((void*)g4, (void*)g5, g6);
        cmpobe  TRUE,g0,.oin9020        # Jif found
#
# --- Check for XIOtech failed device label
#
        lda     O_faillab,g4            # Pass failed label template
        ldconst o_faillablen,g6         # Pass template length
c       g0 = !memcmp((void*)g4, (void*)g5, g6);
        cmpobe  TRUE,g0,.oin9050        # Jif found
#
# --- Set device class to unlabelled
#
        ldconst pdunlab,r3              # Set class to unlabelled
        b       .oin9030
#
# --- Set device class to labelled and load the system serial number from
# --- the label.
#
.oin9020:
        ld      xd_sserial(g7),r3       # Get the system serial number
        st      r3,pd_sserial(g3)       # Save it
#
        ld      xd_dname(g7),r3         # Get the positioning information
        st      r3,pd_dname(g3)         # Save it
#
        ldob    xd_class(g7),r3         # Inherit label class
#
.oin9030:
        stob    r3,pd_class(g3)         # Update device class
        ldconst (pdop<<8)+pdop,r4       # Set status to operative
#
# --- Drive has passed power on test now
#
        lda     pdliteoff,r3
        stob    r3,pd_fled(g3)          # Turn off the fail light
        b       .oin9070                # Exit via the path to free memory
#
# --- Process failed device label - device was previously diagnosed as bad
#
.oin9050:
        ldob    xd_class(g7),r3         # Inherit label class
        stob    r3,pd_class(g3)
#
        ld      xd_dname(g7),r3         # Get the positioning information
        st      r3,pd_dname(g3)         # Save it
#
        ldconst (pdinop<<8)+pdfdevlab,r4# Set status to failed device label
#
# --- Free the memory pointed to by r15 and sized by r14 and exit.
#
.oin9070:
c       s_Free(r15, r14, __FILE__, __LINE__);
        b       .oin9090
#
# --- Error in processing of a command or a quick test.
#     Free the request and exit.
#
.oin9080:
        call    O$relreq                # Release the last request
#
# --- Update PDD status
#
.oin9090:
        ldob    pd_miscstat(g3),r5      # Assume no change
        clrbit  pdmbmissing,r5,r5       # Assume no longer missing
        ldconst (pdnonx<<8)+pdnonx,r3   # Check if non-existent
        cmpobne r3,r4,.oin9100          # Jif NOT non-existent (it exists)
#
        setbit  pdmbmissing,r5,r5       # It's missing - set bit for missing
.ifndef MODEL_3000
.ifndef  MODEL_7400
        ldconst 0xFFFF,r3               # Set the SES index to no enclosure
        stos    r3,pd_ses(g3)
        stob    r3,pd_slot(g3)
.endif  # MODEL_7400
.endif  # MODEL_3000
#
.oin9100:
        stob    r5,pd_miscstat(g3)      # Update misc status
        stos    r4,pd_poststat(g3)      # Update post and dev status
#
        ldob    pd_flags(g3),r3
        bbs     pduserfailed,r3,.oin9105# Jif Pdisk is failed by user
        bbc     pdmbreplace,r5,.oin9110 # Jif not replacement requested
.oin9105:
        ldob    pd_devstat(g3),r3       # Get the device status
        cmpo    pdnonx,r3               # Check if nonexistent
        sele    pdinop,pdnonx,r3        # Either set to inop or nonexistent
        stob    r3,pd_devstat(g3)
c fprintf(stderr, "%s%s:%u pid=%d setting %ld\n", FEBEMESSAGE, __FILE__, __LINE__,((PDD*)g3)->pid,r3);
#
.oin9110:

.if     DEBUG_FLIGHTREC_O
        ldconst frt_h_initdrv1,r3       # Init drive function - end
        st      r3,fr_parm0             # Function
        ldos    pd_pid(g3),r3
        st      r3,fr_parm1             # PID
        st      g3,fr_parm2             # PDD of drive being initialized
        shlo    16,r5,r5
        or      r5,r4,r4                # Get post stat, misc stat, dev stat
        st      r4,fr_parm3             # Drive status
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_O
#
        ld      (r12),r3                # Adjust drive initialization count
        subo    1,r3,r3
        st      r3,(r12)
#
# --- Exit
#
        ret
#
#**********************************************************************
#
#  NAME: o$retry_hotswap
#
#  PURPOSE:
#       Retry the hotswap cycle if some drives were in the process
#       of coming ready.
#
#  DESCRIPTION:
#       This task is created
#
#       The HS_DELAY must be loaded into o_retryhotswap_delay before forking
#       this task.
#
#  CALLING SEQUENCE:
#       process call
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
o$retry_hotswap:
#
# --- Give drives time to complete the spinup
#
.orhs10:
        ld      o_retryhotswap_delay,r3 # Get remaining delay time
        cmpobe  0,r3,.orhs20            # Jif delay is done or no delay specified
        subo    1,r3,r3                 # Decrement delay time
        st      r3,o_retryhotswap_delay
#
        ldconst QUANTUM,g0              # Wait one quantum
        call    K$twait
        b       .orhs10
#
# --- Rescan
#
.orhs20:
        ldconst mrdexisting,g0          # Rescan existing devices
                                        #  This will result in a call to
                                        #  O$hotswap to update device status
        call    P$rescanDevice          # look for all PDDs
#
        ld      o_retryhotswap_delay,r3 # Retry again if drives still aren't ready
        cmpobne 0,r3,.orhs10
#
# --- Exit
#
        ldconst 0,r3                    # Clear active PCB and exit
        st      r3,O_retryhotswap_pcb
        ret
#
#**********************************************************************
#
#  NAME: o$hotswap
#
#  PURPOSE:
#       To provide a common means of forking off processes to address
#       the insertion and removal of disk drives.
#
#  DESCRIPTION:
#       This process first synchronizes with a SCSI bus reset.  Each
#       time a SCSI drive is inserted or removed the hardware onboard
#       the shuttle generates a bus reset.
#
#       This process then synchronizes with the completion of all
#       processes previously forked by this process.
#
#       A check is made to determine if a stop request is active.  If
#       so, this process delays for two seconds and repeats this check.
#
#       An inquiry process is forked for each possible drive within
#       the system.  When all inquiries are completed, a rebuild is
#       started for any drives that need it.
#
#  CALLING SEQUENCE:
#       process call
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
# --- Place process in SCSI reset wait
#
o$hotswap:
#
.if     DEBUG_FLIGHTREC_OHSPARE
        ldconst frt_h_hswap0,r3         # Hot swap function
        st      r3,fr_parm0             # Function
        st      r4,fr_parm2             # Init counter
        ldconst 0,r3
        st      r3,fr_parm1             # NULL parm
        st      r3,fr_parm3             # NULL parm
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_OHSPARE
#
.oh10:
        ld      O_p_pdd_list,r3         # Get the temp PDD list
        cmpobne 0,r3,.oh30              # Jif list exists - process it
#
        ldconst TRUE,r3
        stob    r3,o_hs_stopped         # Task is inactive
c       TaskSetMyState(pcsresetwait);   # Place process in SCSI reset wait state
        call    K$qxchang               # Give up control
#
# --- Handle a task stop request
#
.oh20:
        ldob    O_stopcnt,r3            # Get stop counter
        cmpobe  0,r3,.oh30              # Jif not requested
#
        ldconst 2000,g0                 # Wait 2 seconds
        call    K$twait
        b       .oh20
#
.oh30:
        ldconst FALSE,r3
        stob    r3,o_hs_stopped         # Task is now active
#
# --- Scan all known devices to see if they are still present.
# --- Send a log message if any of them are now missing.
#
#
# --- Check for drives ------------------------------------------------
#
.if     DEBUG_FLIGHTREC_OHSPARE
        ldconst frt_h_hswap1,r3         # Hot swap function
        st      r3,fr_parm0             # Function
        ldconst 0,r3
        st      r3,fr_parm1             # NULL parm
        st      r3,fr_parm2             # NULL parm
        st      r3,fr_parm3             # NULL parm
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_OHSPARE
#
        lda     P_pddindx,r14           # Get PDX
        ldconst MAXDRIVES-1,r3          # Get maximum count
#
.oh40:
        ld      px_pdd(r14)[r3*4],g3    # Get PDD
        cmpobe  0,g3,.oh90              # Jif NULL
#
        ld      O_p_pdd_list,r13        # Get new list of known devices
        lda     -px_ecnt(r13),r13       # Move to list start
        ldos    px_ecnt(r13),r5         # Get number of devices
        cmpobe  0,r5,.oh80              # Jif no entries
#
.oh50:
        subo    1,r5,r5                 # Decrement index to new list
        ld      px_pdd(r13)[r5*4],r6    # Get PDD pointer
        cmpobe  r6,0,.oh70              # its NULL go to next

        ldl     pd_wwn(r6),r8           # Get the WWN
        ldl     pd_wwn(g3),r10          # Get the other WWN
        cmpobne r8,r10,.oh70            # Not a match
        cmpobne r9,r11,.oh70            # Not a match
#
        ldos    pd_lun(r6),r8           # WWN matched, check LUN
        ldos    pd_lun(g3),r10          # Get the other LUN
        cmpobe  r8,r10,.oh90           # Match
#
.oh70:
        cmpibl  0,r5,.oh50              # Jif more to check
#
# --- Not found.  Process the now missing device.  If the device was
# --- previously missing or inoperable, do nothing.
#
.oh80:

        ldob    pd_flags(g3),r6
        bbc     pdbebusy,r6,.oh85       #if  busy
        ld      pd_dev(g3),r6
        cmpobe  r6,0,.oh85              #if no null
        ld      dv_TimetoFail(r6),r5    #get time to fail
        cmpobe  r5,0,.oh85              # if not 0 go to next pdd
        b       .oh90
.oh85:
        ldob    pd_devstat(g3),r6       # Get previous device status
        ldconst (pdnonx<<8)+pdnonx,r5   # Set post and dev status to non-existent
        stos    r5,pd_poststat(g3)
        ldob    pd_miscstat(g3),r5      # Set misc status to missing
        setbit  pdmbmissing,r5,r5
        stob    r5,pd_miscstat(g3)
# Clear busy
        ldob    pd_flags(g3),r5
        clrbit  pdbebusy,r5,r5
        stob    r5,pd_flags(g3)

.ifndef MODEL_3000
.ifndef  MODEL_7400
        ldconst 0xFFFF,r5               # Set the SES index to no enclosure
        stos    r5,pd_ses(g3)
        stob    r5,pd_slot(g3)
.endif  # MODEL_7400
.endif  # MODEL_3000
#
.if     DEBUG_FLIGHTREC_OHSPARE
        ldconst frt_h_hswap2,r5         # Hot swap function
        st      r5,fr_parm0             # Function
        ldos    pd_pid(g3),r5
        st      r5,fr_parm1             # PID
        st      g3,fr_parm2             # PDD of missing drive
        st      r6,fr_parm3             # Old dev status
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_OHSPARE
#
# SERVICEABILITY42
###     Just clear the spindown bit, it might have been set during spindown.
###     Is it necessary to reset poststat >???????????
        ldob   pd_flags(g3),r5
        clrbit pduserspundown,r5,r5
        clrbit pduserfailed,r5,r5
        stob   r5,pd_flags(g3)
#
        cmpobne pdop,r6,.oh90           # Jif previously missing or inop
#
        call    O$log_drive_removed     # Log event
.oh90:
        subi    1,r3,r3                 # Decrement index
        cmpible 0,r3,.oh40              # Jif more PDDs to do
.ifndef MODEL_3000
.ifndef  MODEL_7400
############################################################
#
#
# --- Check SES devices -----------------------------------------------
#
        lda     E_pddindx,r14           # Get EDX
        ldconst MAXSES-1,r3             # Get maximum count
#
.oh95:
        ld      px_pdd(r14)[r3*4],g3    # Get PDD
        cmpobe  0,g3,.oh190             # Jif NULL
#
        ld      O_p_pdd_list,r13        # Get new list of known devices
        lda     -px_ecnt(r13),r13       # Move to list start
        ldos    px_ecnt(r13),r5         # Get number of devices
        cmpobe  0,r5,.oh98              # Jif no entries
#
.oh96:
        subo    1,r5,r5                 # Decrement index to new list
        ld      px_pdd(r13)[r5*4],r6    # Get PDD pointer
        cmpobe  0,r6,.oh97
#
        ldl     pd_wwn(r6),r8           # Get the WWN
        ldl     pd_wwn(g3),r10          # Get the other WWN
        cmpobne r8,r10,.oh97            # Not a match
        cmpobne r9,r11,.oh97            # Not a match
#
        ldos    pd_lun(r6),r8           # WWN matched, check LUN
        ldos    pd_lun(g3),r10          # Get the other LUN
        cmpobe  r8,r10,.oh190            # Match
#
.oh97:
        cmpibl  0,r5,.oh96              # Jif more to check
#
# --- Not found.  Process the now missing device.  If the device was
# --- previously missing or inoperable, do nothing.
#
.oh98:
        ldob    pd_devstat(g3),r6       # Get previous device status
        ldconst (pdnonx<<8)+pdnonx,r5   # Set post and dev status to non-existent
        stos    r5,pd_poststat(g3)
        ldob    pd_miscstat(g3),r5      # Set misc status to missing
        setbit  pdmbmissing,r5,r5
        stob    r5,pd_miscstat(g3)
.if     DEBUG_FLIGHTREC_OHSPARE
        ldconst frt_h_hswap2,r5         # Hot swap function
        st      r5,fr_parm0             # Function
        ldos    pd_pid(g3),r5
        st      r5,fr_parm1             # PID
        st      g3,fr_parm2             # PDD of missing drive
        st      r6,fr_parm3             # Old dev status
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_OHSPARE
       cmpobne pdop,r6,.oh190           # Jif previously missing or inop

.oh190:
        subi    1,r3,r3                 # Decrement index
        cmpible 0,r3,.oh95              # Jif more PDDs to do
.endif  # MODEL_7400
.endif  # MODEL_3000

############################################################
#
#
# --- Send inquiry to all devices to update PDD status
#
        ld      O_p_pdd_list,g0         # Pass list pointer (point to count)
        ldconst 2,g2                    # Do a short init_drv test
        lda     -px_ecnt(g0),r7         # Move to first PDD entry
        ld      px_ecnt(r7),r3          # Get max index
        PushRegs(r5)
c       ON_InquireAll((PDD**)r7,r3,g2);
        PopRegsVoid(r5)
#
.if     DEBUG_FLIGHTREC_O
        ldconst frt_h_hswap3,r3         # Hotswap function
        st      r3,fr_parm0             # Function
        ldconst 0,r3                    # NULL
        st      r3,fr_parm1
        st      r3,fr_parm2             # NULL
        st      r3,fr_parm3             # NULL
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_O
#
# --- Handle enclosures
#
        ldos    K_ii+ii_status,g0       # Get initialization status
        setbit  iisesbkrun,g0,g0        # Set to indicate SES_BackgroundProcess can run
        stos    g0,K_ii+ii_status       # Save it
        call    SES_StartBGProcess      # Background processing function start
#
# --- Now reset all of the largest available space and total available
# --- space count by building and releasing DAMs.
#
        ldconst MAXDRIVES-1,r3          # Do all of the drives
#
.oh240:
        mov     r3,g0
        ldconst FALSE,g1                # No force
        call    D$calcspaceshell        # Update capacities
#
        subo    1,r3,r3                 # Decrement PID
        cmpible 0,r3,.oh240             # Check the next one
#
# --- Cleanup ---------------------------------------------------------
#
# --- Update PSD, RDD, and VDD status.
# --- Hotspare if possible.
#
.ifndef MODEL_3000
.ifndef  MODEL_7400
        call    ON_UpdateVDisks         # Update the BUSY state of all VIDs
.endif  # MODEL_7400
.endif  # MODEL_3000
        call    RB_setpsdstat           # Update the PSD and RAID status
        call    RB_setvirtstat          # Update Virtual status
        call    RB_searchforfailedpsds  # Attempt to hotspare failed PSDs
#
        call    D$p2update              # Update NVRAM
#
#
# --- Deallocate the temporary PDD list
#
        ld      O_p_pdd_list,g0         # Release this RAM
        cmpobe  0,g0,.oh260             # Jif no list
#
c       s_Free(g0, pdxsiz+4, __FILE__, __LINE__);
#
        ldconst 0,r3
        st      r3,O_p_pdd_list         # Clear the anchor
c       TaskReadyByState(pcocmpwait);   # Awaken ISP online event handler
        call    K$qxchang               # Give up control
#
# --- Common branch point
#
.oh260:
        b       .oh10                   # All done - go to start of task
#
# end o$hotswap:
#
#**********************************************************************
#
#  NAME: o$inquire
#
#  PURPOSE:
#       Helper process to process an reset condition (database change).
#       This handles changes for a particular physical device.
#
#  DESCRIPTION:
#       This function will call init_Drv for the PDD passed in and will
#       compare the new status of the device with the old status.
#       PDD state changes will be logged. Devices becoming operable will
#       have their file system updated.
#
#  CALLING SEQUENCE:
#       Process call
#
#  INPUT:
#       g2      Test type 1 = Quick - inquiry and TUR only
#                         2 = no drive test
#                         3 = full test
#       g3      PDD of new device
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
o$inquire:
.if     DEBUG_FLIGHTREC_O
        ldconst frt_h_inquire0,r3       # Inquire function
        st      r3,fr_parm0             # Function
        ldos    pd_pid(g3),r3
        st      r3,fr_parm1             # PID
        st      g3,fr_parm2             # PDD of drive being inquired
        ldconst 0,r3
        st      r3,fr_parm3             # Null
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_O
#
# --- First, find the device if it already exists.
#
        lda     P_pddindx,r15           # Get PDX
        ldconst MAXDRIVES-1,r14         # Get maximum count
#
.inq10:
        ld      px_pdd(r15)[r14*4],r3   # Get old PDD
        cmpobe  r3,g3,.inq90            # Found - record it
#
# --- Not a direct match, so check WWN and blow away old PDD if this is
# --- the same device.
#
        cmpobe  0,r3,.inq15             # Jif NULL
        ldl     pd_wwn(r3),r4           # Get the WWN
        ldl     pd_wwn(g3),r6           # Get the other WWN
        cmpobne r4,r6,.inq15            # Not a match
        cmpobne r5,r7,.inq15            # Not a match
#
        ldos    pd_lun(r3),r4           # WWN matched, check LUN
        ldos    pd_lun(g3),r6           # Get the other LUN
        cmpobne r4,r6,.inq15            # Not a match
#
# --- It was a match.  Blow away the old PDD and put this one in the list.
#
c       ON_MigrateOldPDDtoNewPdd((PDD*)r3,(PDD*)g3,(PDD**)r15)
        b       .inq90
#
.inq15:
        subo    1,r14,r14               # Decrement index
        cmpible 0,r14,.inq10            # Jif more to check
#
# --- Check misc devices ----------------------------------------------
#
        lda     M_pddindx,r15           # Get MDX
        ldconst MAXMISC-1,r14           # Get maximum count
#
.inq20:
        ld      px_pdd(r15)[r14*4],r3   # Get PDD
        cmpobe  r3,g3,.inq90            # Found - record it
#
# --- Not a direct match, so check WWN and blow away old PDD if this is
# --- the same device.
#
        cmpobe  0,r3,.inq25             # Jif NULL
        ldl     pd_wwn(r3),r4           # Get the WWN
        ldl     pd_wwn(g3),r6           # Get the other WWN
        cmpobne r4,r6,.inq25            # Not a match
        cmpobne r5,r7,.inq25            # Not a match
#
        ldos    pd_lun(r3),r4           # WWN matched, check LUN
        ldos    pd_lun(g3),r6           # Get the other LUN
        cmpobne r4,r6,.inq25            # Not a match
#
# --- It was a match.  Blow away the old PDD and put this one in the list.
#
c       ON_MigrateOldPDDtoNewPdd((PDD*)r3,(PDD*)g3,(PDD**)r15)
        b       .inq90
#
.inq25:
        subo    1,r14,r14               # Decrement index
        cmpible 0,r14,.inq20            # Jif more to check
#
# --- Check SES devices -----------------------------------------------
#
        lda     E_pddindx,r15           # Get EDX
        ldconst MAXSES-1,r14            # Get maximum count
#
.inq30:
        ld      px_pdd(r15)[r14*4],r3   # Get PDD
        cmpobe  r3,g3,.inq90            # Found - record it
#
# --- Not a direct match, so check WWN and blow away old PDD if this is
# --- the same device.
#
        cmpobe  0,r3,.inq35             # Jif NULL
        ldl     pd_wwn(r3),r4           # Get the WWN
        ldl     pd_wwn(g3),r6           # Get the other WWN
        cmpobne r4,r6,.inq35            # Not a match
        cmpobne r5,r7,.inq35            # Not a match
#
        ldos    pd_lun(r3),r4           # WWN matched, check LUN
        ldos    pd_lun(g3),r6           # Get the other LUN
        cmpobne r4,r6,.inq35            # Not a match
#
# --- It was a match.  Blow away the old PDD and put this one in the list.
#
c       ON_MigrateOldPDDtoNewPdd((PDD*)r3,(PDD*)g3,(PDD**)r15)
        b       .inq90
#
.inq35:
        subo    1,r14,r14               # Decrement index
        cmpible 0,r14,.inq30            # Jif more to check
#
        ldconst 0,r15                   # Not found
        b       .inq100                 # Continue
#
# --- Common code -----------------------------------------------------
#
.inq90:
        lda     px_pdd(r15)[r14*4],r15  # Record address in table
        ld      (r15),r3                # PDD pointer
        ldob    pd_devstat(r3),r14      # Old dev status
        ldob    pd_flags(r3),r5
.ifndef MODEL_7000
.ifndef MODEL_4700
        clrbit  pdbebusy,r5,r5          # Clear the ISE BUSY flag on this PID
        stob    r5,pd_flags(r3)
.endif  # MODEL_4700
.endif  # MODEL_7000
.ifdef SERVICEABILITY42
# we will never issue 'inquire' command to a drive that was spundown by the user--- OK
        bbs     pduserspundown,r5,.inq900
.endif  # SERVICEABILITY42
#
# --- At this point, r15 holds the address of the PDD slot in the
# --- table for the device if it was found.  r14 has the old device
# --- status.
#
.inq100:
#
.if     DEBUG_FLIGHTREC_O
        ldconst frt_h_inquire1,r3       # Inquire function - pre init_drv call
        st      r3,fr_parm0             # Function
        ldos    pd_pid(g3),r3
        st      r3,fr_parm1             # PID
        st      g3,fr_parm2             # PDD of drive being inquired
        st      r15,fr_parm3            # Old slot for PDD in PDX (actual addr)
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_O
#
# --- Issue an init drive function for the device.  This will update all
# --- of the fields in the PDD which we will then compare to the previous
# --- values for previously existing devices.
#
        lda     O_drvinits,g8           # Get inits in progress pointer
        ld      (g8),r3                 # Get number of units waiting init
        addo    1,r3,r3                 # Add one more device
        st      r3,(g8)                 # Store it back
#
        PushRegs                        # Save all G registers (stack relative)
        call    O$init_drv              # Go look at the drive
        PopRegsVoid                     # Restore all G registers (stack relative)
#
# --- First check for really non-existent devices.
#
        cmpobne 0,r15,.inq110           # Previously, no device existed at all
#
# --- If the next device is a disk drive, initialize the file system
#
        ldob    pd_devtype(g3),r3       # Get device type
        cmpobge pddtmaxdisk,r3,.inq105  # Jif a disk
#
        cmpobl  pddtmaxses,r3,.inq103   # Jif not a bay
#
# --- Call the SES code to install this enclosure in the tables.
#
        PushRegs(r3)                    # Save the registers
        mov     g3,g2                   # Set input to SES function
        ldconst 0,g3                    # Null the counter input
        ldconst 0,g4                    # Do not log.
        call    SES_GetDirectEnclosure  # Get the slot if not known
        PopRegsVoid(r3)                 # Restore registers
        b       .inq900                 # And exit
#
.inq103:
# This probably means that we have a garbage PDD pointer. If a device we do not know, do not crash.
        mov     g3,g0                   # Set input to insert PDD function
        call    D_insertpdd             # Insert it
        b       .inq900                 # And out
#
.inq105:
        mov     g3,g0                   # Set input to insert PDD function
        call    D_insertpdd             # Insert it
#
.ifndef MODEL_7000
.ifndef MODEL_4700
        ldconst 0xFFFF,r3               # Set the SES index to no enclosure
        stos    r3,pd_ses(g3)
        stob    r3,pd_slot(g3)
.endif  # MODEL_4700
.endif  # MODEL_7000
#
# Following is the case when a device is inserted into one of bays
# which is being removed from one of the bays. We need to handle
# this case also to assign the location code. We set the bit
# accordingly to inform the ses background process to assign the
# location code as that of the bay in which the drive is being
# inserted.

        ldob    O_p2init,r3             # Get phase II inits complete
        cmpobne TRUE,r3,.inq106         # Jif not completed
        ldob    pd_geoflags(g3),r3      # Get the GEO Flag
        setbit  pddriveinserted,r3,r3   # Set drive inserted
        stob    r3,pd_geoflags(g3)      # Save it

# Following case is to handle multiple drive insertions for serviceability
# feature
        ldob    pd_flags(g3), r3        # Get the pd_flags
        setbit  pduserinserted,r3,r3    # Set drive inserted
        stob    r3, pd_flags(g3)        # Save it to indicate disk has been inserted.

.inq106:
        ldob    pd_devstat(g3),r3       # Get the device status
        cmpobne pdop,r3,.inq900         # Jif not operable
#
        ldob    pd_class(g3),r3         # Get device class
        cmpobe  pdunlab,r3,.inq900      # Jif unlabelled
#
        ld      K_ficb,r3               # Get system VCG from system
        ld      pd_sserial(g3),r4       # Get system serial number from drive
        ld      fi_vcgid(r3),r3
        cmpobne r3,r4,.inq900           # Jif foreign drive
#
        ldob    pd_miscstat(g3),r3      # Get the misc status
        setbit  pdmbfserror,r3,r3       # Set file system needs repair
c fprintf(stderr, "%s%s:%u Setting pdmbfs error into miscstat for pid=%d -- drive went away and came back, miscstat=0x%02lx\n", FEBEMESSAGE, __FILE__, __LINE__,((PDD*)g3)->pid,r3);
        stob    r3,pd_miscstat(g3)      # Save it
#
        b       .inq890                 # Done
#
# --- If the device was not a disk drive, pop out since there is no
# --- more processing to do.
#
.inq110:
        ldob    pd_devtype(g3),r3       # Get device type
        cmpobl  pddtmaxdisk,r3,.inq900  # Jif not a disk
#
        cmpobne pdop,r14,.inq300        # Handle operable to x transitions
        ldob    pd_devstat(g3),r4       # Get new status
#
.if     DEBUG_FLIGHTREC_O
        ldconst frt_h_inquire2,r3       # Inquire function (previous operable)
        st      r3,fr_parm0             # Function
        ldos    pd_pid(g3),r3
        st      r3,fr_parm1             # PID
        st      g3,fr_parm2             # PDD of drive being inquired
        st      r4,fr_parm3             # New drive status
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_O
#
        cmpobe  pdop,r4,.inq900         # Exit since still operable
#
        call    O$log_drive_reset       # Log event
        b       .inq900
#
# --- The only two status' left are inoperable and non-existent.  First, log
# --- the non-existant to any state transition as being reinserted.  Then,
# --- process the rest of the transition the same in both cases.
#
.inq300:
        ldob    pd_devstat(g3),r4       # Get new status
        cmpobne pdnonx,r14,.inq310      # Jif previously inoperable
        cmpobe  pdnonx,r4,.inq310       # Jif still non-existent
#
# Following is the case when a device is reattached

        ldob    O_p2init,r3             # Get phase II inits complete
        cmpobne TRUE,r3,.inq305         # Jif not completed
        ldob    pd_geoflags(g3),r3      # Get the GEO Flag
        setbit  pddrivereattached,r3,r3 # Set drive inserted
        stob    r3,pd_geoflags(g3)      # Save it

# Following is the case when multiple drives are reattached to handle
# serviceability feature

        ldob    pd_flags(g3), r3        # Get the pd_flags
        setbit  pduserreattached,r3,r3  # Set drive reattached
        stob    r3, pd_flags(g3)        # Save it to indicate disk has been reattached.
        b       .inq310                 # Delay the log message in this case
                                        # This is handled separately
.inq305:

        call    o$log_reattaching       # Log that we're reattaching this drive
#
.inq310:
.if     DEBUG_FLIGHTREC_O
        ldconst frt_h_inquire3,r3       # Inquire function (previous not operable)
        st      r3,fr_parm0             # Function
        ldos    pd_pid(g3),r3
        st      r3,fr_parm1             # PID
        st      g3,fr_parm2             # PDD of drive being inquired
        st      r4,fr_parm3             # New drive status
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_O
#
        cmpobne pdop,r4,.inq900         # If still not operable, quit
#
# --- Inoperable became operable.  Update the file system and log insertion.
#
        ld      K_ficb,r10
        ld      fi_vcgid(r10),r5        # Get this Virtual controller group
        ld      pd_sserial(g3),r6       # Get this device serial number
        cmpobne r5,r6,.inq900           # If serial numbers don't match, ignore
#
# --- Update the File System on new drive
#
        ldob    pd_miscstat(g3),r3      # Get the misc status
        setbit  pdmbfserror,r3,r3       # Set file system needs repair
        stob    r3,pd_miscstat(g3)      # Save it
c fprintf(stderr, "%s%s:%u Setting pdmbfs error into miscstat for pid=%d -- update file system on new drive, miscstat=0x%02lx\n", FEBEMESSAGE, __FILE__, __LINE__,((PDD*)g3)->pid,r3);
#
        ldos    pd_pid(g3),g0           # Get the PID
        call    D$damdirtyshell         # Dirty the DAM to force update
#
.inq890:
        PushRegs(r3)                    # Save registers
        ldconst FALSE,g0                # Set master false
        call    NV_SendFSys             # Send information to master
        PopRegsVoid(r3)                 # Restore registers
#
# --- Decrement the inquiries pending count
# --- search for failed PDDs.  If it is non-zero, we don't want to check
# --- for failed PDDs since some drives may still be coming on line.
#
.inq900:
        ld      o_inquiries_pending,r4  # Decrement outstanding inquiry count
        subo    1,r4,r4
        st      r4,o_inquiries_pending
#
# --- Exit
#
        ldos    pd_pid(g3),r3
        ldconst 0,r4
        st      r4,O_ipcb[r3*4]         # Zero PCB pointer in list
        ret
#
#**********************************************************************
#
#  NAME: O$pdiskspindown
#
#  PURPOSE:
#       To provide a means of spinning down a designated drive.
#
#  DESCRIPTION:
#       A stopunit request is constructed for the designated drive
#       and submitted to the physical layer.
#
#  CALLING SEQUENCE:
#       call    O$pdiskspindown
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = status: deok, deinvpid, depidused, depidspundown
#       g2 = return pkt size
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
# --- Setup
#
O$pdiskspindown:
        mov     g3,r11                  # Save registers
        movl    g4,r14
#
        ld      mr_ptr(g0),g0           # Get parm block pointer
        ldos    mfd_pid(g0),r7          # Get spindown PID
#
# --- PDD sanity checks
#
        ldconst deinvpid,g1             # Prep error code
        ldconst MAXDRIVES,r4
        cmpoble r4,r7,.sd1000           # Jif spindown PID out of range error

        ld      P_pddindx[r7*4],r5      # r5 = PDD pointer
        cmpobe  0,r5,.sd1000            # Jif no PDD error

        ldconst depidused,g1            # Assume PID used by any RAIDs
        mov     r5,g4                   # Pass PDD
        call    D_convpdd2psd           # Convert PDD to PSD
        cmpobne 0,g0,.sd1000            # Jif PDD used (RAID exists) - error

        ldconst depidspundown,g1        # Assume PID has already been spundown
        mov     r5,g3
        ldob    pd_flags(g3),r4           # Load this PDD as spinning down
        bbs     pduserspundown,r4,.sd1000 # Jif the designated has already been
                                        # spundown
# --- Spin down the device
#
        setbit  pduserspundown,r4,r4
        stob    r4,pd_flags(g3)         # Mark this PDD as spinning down
        ldconst pdinop,r3
        stob    r3,pd_devstat(g3)       # Mark PDD as inoperable
c fprintf(stderr, "%s%s:%u pid=%d setting devstat inop %ld\n", FEBEMESSAGE, __FILE__, __LINE__,((PDD*)g3)->pid,r3);
        ldconst pduserspundownstate,r3
        stob    r3,pd_poststat(g3)
#
        lda     O_t_stopunit,g0         # Pass stop unit template
        call    O$genreq                # Generate request
        call    O$quereq                # Queue request
        call    O$relreq                # Release request
#
#
        ldconst pdliteid,r3             # Blink the identify LED
        stob    r3,pd_fled(g3)
#
        call    O$ledchanged
#
        ldconst deok,g1                 # Indicate OK
        b       .sd1000
#
# --- Exit
#
.sd1000:
#        ldconst mfdrsiz,g2              # Set return packet size
        ldconst 1,g2                    # Set return packet size
        movl    r14,g4                  # Restore registers
        mov     r11,g3
        ret
#
#**********************************************************************
#
#  NAME: O$spindowndrive
#
#  PURPOSE:
#       To provide a means of spinning down a drive that has failed.
#
#  DESCRIPTION:
#       A stopunit request is constructed for the designated drive
#       and submitted to the physical layer.  A return is made as soon
#       as the submission is complete.
#
#  CALLING SEQUENCE:
#       call    O$spindowndrive         # Write failed device label
#
#  INPUT:
#       g3 = PDD
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
O$spindowndrive:
        movq    g0,r12                  # Save g0-g3
#
# --- Generate and issue stop unit request
#
        lda     O_t_stopunit,g0         # Pass stop unit template
        call    O$genreq                # Generate request
        call    O$quereqnw              # Queue request w/o wait
#
# --- Exit
#
        movq    r12,g0                  # Restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: O$writefailedlabel
#
#  PURPOSE:
#       To provide a means of writing the failed label to a device.
#
#  DESCRIPTION:
#       The failed label is written the the specified device.
#
#  CALLING SEQUENCE:
#       call    O$writefailedlabel       # Write failed device label
#
#  INPUT:
#       g2 = Failure Type
#       g3 = PDD
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
# C access
# void ON_WriteFailedLabel(UINT8 failureType, PDD* pPDD);
        .globl  ON_WriteFailedLabel
ON_WriteFailedLabel:
#        mov     g0,g2                   # Failure Type
#        mov     g1,g3                   # PDD
# fall through
#

O$writefailedlabel:
        movq    g0,r12                  # Save g0-g3
#
# --- Validity check the PDD
#
        cmpobe  0,r15,.wfl1000          # Exit if PDD is NULL
        ldos    pd_pid(r15),r3          # Get the PID
        ldconst MAXDRIVES,r4
        cmpoble r4,r3,.wfl1000          # Jif failing PID out of range error
        ld      P_pddindx[r3*4],r4      # Get the PDD for this PID
        cmpobne r15,r4,.wfl1000         # Exit if the PDD's are inconsistent

.if     DEBUG_FLIGHTREC_O
        ldconst frt_h_misc10,r3         # WriteFailedLabel - start
        st      r3,fr_parm0
        st      r14,fr_parm1            # Failure Type
        st      r15,fr_parm2            # PDD of drive being failed
        ldos    pd_pid(r15),r3
        st      r3,fr_parm3             # Pid
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_O
#
# --- Clear device label - write failure signature if possible
#
c       g0 = s_MallocC(LABELSIZE*SECSIZE, __FILE__, __LINE__);
        mov     g0,r10                  # Save pointer
#
        lda     O_faillab,r3            # Copy label string to device buffer
#
.wfl10:
        ldob    (r3),r4                 # Get next label byte
        cmpobe  0,r4,.wfl20             # Jif null termination
#
        stob    r4,(g0)                 # Store next label byte
        addo    1,r3,r3                 # Advance src pointer
        lda     1(g0),g0                # Advance dst pointer
        b       .wfl10
#
# --- Fill in failed label file fields
#
.wfl20:
        stob    g2,xd_failtype(r10)     # Store the failure type
        ld      K_ficb,g0               # Get system serial number
        ld      fi_vcgid(g0),r4         # Get requested virtual controller group
        st      r4,xd_sserial(r10)      # Set up device system serial number
        ldob    pd_class(g3),r11        # Get old label type rather than new one
        stob    r11,xd_class(r10)       # Set up device class
        ldl     pd_wwn(g3),r8           # Get WWN (r8,r9)
        stl     r8,xd_wwn(r10)          # Set up WWN
        ld      pd_dname(g3),r8         # Get positioning information
        st      r8,xd_dname(r10)        # Set it
#
# --- Write the label file to the drive
#
        ldconst fidlabel,g0             # Write the label
        mov     r10,g1
        ldconst LABELSIZE,g2
        ldconst 1,g4
        call    FS$WriteFile            # Write the label
#
c       s_Free(r10, LABELSIZE*SECSIZE, __FILE__, __LINE__);
#
# --- Exit
#
.if     DEBUG_FLIGHTREC_O
        ldconst frt_h_misc11,r3         # WriteFailedLabel - end
        st      r3,fr_parm0
        st      r14,fr_parm1            # Failure Type
        st      r15,fr_parm2            # PDD of drive being failed
        ldos    pd_pid(r15),r3
        st      r3,fr_parm3             # Pid
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_O
.wfl1000:
        movq    r12,g0                  # Restore g0-g3
        ret
#
#**********************************************************************
#
#  NAME: ON_SCSICmd
#
#  PURPOSE:
#       To provide a simple and efficient means of generating ILT/PRP/SGL
#       structures in support of a specific physical SCSI operation,
#       submitting the command, waiting for completion and copying the
#       data into user space.
#
#  DESCRIPTION:
#       The information present within the template is used to construct
#       an ILT and PRP.  If a data transfer is involved an SGL and
#       associated buffer is also allocated and linked.
#
#       Although it is highly unusual, the possibility exists that this
#       routine may block if any of the requested resources are
#       unavailable.
#
#       The command is then issued and when completed, checked for good
#       status.  If everything worked, then any returned data is placed
#       into memory for the caller.  The command structures are then
#       returned to the pools.
#
#  CALLING SEQUENCE:
#       call    O_inputcmd
#
#  INPUT:
#       g0 = template address
#       g1 = buffer address for output data or returned data
#       g2 = size of buffer allocated
#       g3 = PDD
#       g4 = pointer to sense data (unallocated buffer space)
#
#  OUTPUT:
#       g0 = status from the command (0 = good)
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
# C access
# UINT32 ON_SCSICmd(void* template, void* outputBuffer, UINT32 size,
#                 PDD* pPDD, SNS* * pSenseData);
        .globl  ON_SCSICmd              # C access
ON_SCSICmd:
        movq    g0,r12                  # Save inputs
        ldconst 0,r3
        st      r3,(g4)                 # Zero the return pointer
#
        call    O$genreq                # Gen request (g0 = template, g3 = PDD)
#
# --- If this is an output command, then copy the data into the output SGL
# --- buffers.
#
        ldob    pr_func(g2),r3          # Get the function code
        cmpobne proutput,r3,.sc20       # Jif not output command (no out data)
#
        ld      pr_rqbytes(g2),r3       # Get byte count to copy
        ld      pr_sglptr(g2),r4        # Get pointer to SGL
        ld      sg_desc0+sg_addr(r4),r4 # Pointer to buffer
#
.sc10:
        subo    1,r3,r3                 # Decrement counter
        ldob    (r13)[r3*1],r5          # Get a byte
        stob    r5,(r4)[r3*1]           # Save it
        cmpibl  0,r3,.sc10              # Jif more to copy
#
.sc20:
        call    O$quereq                # Queue request
        call    M$chkstat               # Check status
        mov     g0,r12                  # Save return code (good or bad)
        cmpobne ecok,g0,.sc40           # Jif error
#
# --- Operation completed OK.  Copy the data to the read buffer in the
# --- case of a read type operation.
#
        ldob    pr_func(g2),r3          # Get the function code
        cmpobne prinput,r3,.sc100       # Jif not input command (no input data)
#
        ld      pr_rqbytes(g2),r3       # Get byte count to copy
        ld      pr_sglptr(g2),r4        # Get pointer to SGL
        ld      sg_desc0+sg_addr(r4),r4 # Pointer to buffer
#
.sc30:
        subo    1,r3,r3                 # Decrement counter
        ldob    (r4)[r3*1],r5           # Get a byte
        stob    r5,(r13)[r3*1]          # Save it
        cmpibl  0,r3,.sc30              # Jif more to copy
        b       .sc100                  # Exit
#
# --- Error exit.  Copy the sense data if there was a CC, otherwise, set
# --- the return code to -1 to indicate some other error.
#
.sc40:
        ldconst eccheck,r3              # Check for check condition
        cmpobne r3,g0,.sc100            # Jif not CC (return code from queureq)
#
# --- Allocate a sense area, copy the data and put the pointer into the
# --- return area (r12).
#
c       g0 = s_MallocW(SENSESIZ, __FILE__, __LINE__);
        st      g0,(g4)                 # Set pointer from input block
#
        ldq     pr_sense(g2),r4         # Get first quad
        stq     r4,(g0)
        ldq     pr_sense+16(g2),r4      # Get second quad
        stq     r4,16(g0)
#
.sc100:
        call    O$relreq                # Release the request
        movq    r12,g0                  # Restore input parms and return code
        ret
#
#**********************************************************************
#
#  NAME: O$genreq
#
#  PURPOSE:
#       To provide a simple and efficient means of generating ILT/PRP/SGL
#       structures in support of a specific physical SCSI operation.
#
#  DESCRIPTION:
#       The information present within the template is used to construct
#       an ILT and PRP.  If a data transfer is involved an SGL and
#       associated buffer is also allocated and linked.
#
#       Although it is highly unusual, the possibility exists that this
#       routine may block if any of the requested resources are
#       unavailable.
#
#  CALLING SEQUENCE:
#       call    O$genreq
#
#  INPUT:
#       g0 = template address
#       g3 = PDD
#
#  OUTPUT:
#       g0 = data buffer address if req'd
#       g1 = ILT
#       g2 = PRP
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
# C Access
# ILT* ON_GenReq(&gTemplateVerify1, pPDD, &pDataBuffer, &pPRP)
        .globl  ON_GenReq
ON_GenReq:
        movq    g0,r12                  # Save regs
        mov     g1,g3                   # PDD
        call    O$genreq                # Call function
        ld      tpr_rqbytes(r12),r3     # Get requested bytes
        cmpobe  0,r3,.g10               # Jif none requested
        cmpobe  0,r14,.g10              # Jif no buffer
        st      g0,(r14)                # data buffer
.g10:
        st      g2,(r15)                # PRP
        mov     g1,g0                   # ILT
        ret
#
O$genreq:
#
# --- Assign ILT/PRP
#
c       {
c       PDD * pdd = (PDD*)g3;
c       UINT8   channel = pdd->channel;
c       UINT16  lun = pdd->lun;
c       UINT32  handle = pdd->id;
c       UINT32  devptr = (UINT32)pdd->pDev;

c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       g2 = get_prp();                 # Assign PRP
.ifdef M4_DEBUG_PRP
c CT_history_printf("%s%s:%u get_prp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_PRP
        st      g2,il_w0(g1)            # Link PRP to ILT
#
# --- Initialize PRP
#
        movq    0,r4                    # Clear out PRP
        stq     r4,pr_func(g2)
        stq     r4,pr_sda(g2)
        stq     r4,pr_sglptr(g2)
        stl     r4,pr_rsbytes(g2)
        stq     r4,pr_sense(g2)
        stq     r4,pr_sense+16(g2)
#
        ldob    tpr_func(g0),r3         # Set up function
        stob    r3,pr_func(g2)
#
        ldconst prhigh,r3
        stob    r3,pr_strategy(g2)      # set strategy
#
c       r3 = channel;                   # Set up SCSI channel
        stob    r3,pr_channel(g2)
#
c       r3 = lun;                       # Set up SCSI LUN
        stos    r3,pr_lun(g2)
#
c       r3 = handle;                    # Set up SCSI ID
        st      r3,pr_id(g2)
#
c       r3 = devptr;                    # Set up DEVice
        st      r3,pr_dev(g2)
#
        ld      tpr_timeout(g0),r3      # Set up timeout
        st      r3,pr_timeout(g2)
#
        ldconst SENSESIZ,r3             # Set up request sense size
        stob    r3,pr_rsbytes(g2)
#
        ldob    tpr_cbytes(g0),r3       # Set up command length
        stob    r3,pr_cbytes(g2)
#
        ldos    tpr_flags(g0),r3        # set request control flags and
        stos    r3,pr_flags(g2)         # request retry count
#
        ldq     tpr_cmd(g0),r4          # Set up command
        stq     r4,pr_cmd(g2)
#
        ld      tpr_rqbytes(g0),r15     # Set up requested bytes
        st      r15,pr_rqbytes(g2)
        mov     r15,g0
        cmpobe  0,r15,.g100             # Jif none requested
#
# --- Allocate combined SGL/buffer
#
c       g0 = m_asglbuf(g0);             # allocate a SGL and buffer
        st      g0,pr_sglptr(g2)        # Link SGL to PRP
#
        ld      sg_size(g0),r3          # Set up size of SGL
        setbit  31,r3,r3                # Indicate as borrowed
        st      r3,pr_sglsize(g2)
#
        call    M$clrsgl                # Clear buffer
#
        ld      sg_desc0+sg_addr(g0),g0 # Return buffer address
#
# --- Exit
#
c   }
.g100:
        ret
#
#**********************************************************************
#
#  NAME: o$gendpat
#
#  PURPOSE:
#       To provide a means of generating a test data pattern suitable
#       for use with a SCSI Write Buffer command.
#
#  DESCRIPTION:
#       The SGL linked to the PRP is used to obtain the buffer address
#       and length.  That buffer is then initialized to a repeatable
#       data pattern.
#
#  CALLING SEQUENCE:
#       call    o$gendpat
#
#  INPUT:
#       g0 = buffer address
#       g1 = buffer length
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
o$gendpat:
#
# --- Locate buffer address and length
#
        movl    g0,r14                  # Get buffer address and length
#
# --- Clear reserved header
#
        ldconst 0,r4                    # Clear reserved header
        st      r4,(r14)
        subo    4,r15,r15               # Allow for header
#
# --- Generate data pattern
#
        ldconst 0xfffffffe,r4           # Prime data pattern
#
.gd10:
        lda     4(r14),r14              # Bump to next location
        st      r4,(r14)                #  and store data pattern
#
        subo    4,r15,r15               # Check for additional buffer
        cmpobe  0,r15,.gd100            # Jif not
#
        rotate  1,r4,r4                 # Modify data pattern
        b       .gd10
#
# --- Exit
#
.gd100:
        ret
#
#**********************************************************************
#
#  NAME: o$chkdpat
#
#  PURPOSE:
#       To provide a means of checking a test data pattern previously
#       written by a SCSI Write Buffer command.
#
#  DESCRIPTION:
#       The SGL linked to the PRP is used to obtain the buffer address
#       and length.  That buffer is then checked for a repeatable
#       known data pattern.
#
#  CALLING SEQUENCE:
#       call    o$chkdpat
#
#  INPUT:
#       g0 = buffer address
#       g1 = buffer length
#
#  OUTPUT:
#       g0 = t/f
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
o$chkdpat:
#
# --- Locate buffer address and length
#
        movl    g0,r14                  # Get buffer address and length
        subo    4,r15,r15               # Allow for header
#
        ldconst TRUE,g0                 # Assume true condition
#
# --- Regenerate and check data pattern
#
        ldconst 0xfffffffe,r4           # Prime data pattern
#
.cd10_o:
        lda     4(r14),r14              # Bump to next location
        ld      (r14),r6                # Get data from buffer
        cmpobne r4,r6,.cd20_o           # Jif no match
#
        subo    4,r15,r15               # Check for additional buffer
        cmpobe  0,r15,.cd100_o          # Jif not
#
        rotate  1,r4,r4                 # Modify data pattern
        b       .cd10_o
#
.cd20_o:
        ldconst FALSE,g0                # Set error condition
#
# --- Exit
#
.cd100_o:
        ret
#
#**********************************************************************
#
#  NAME: O$quereq
#
#  PURPOSE:
#       To provide a common means of queuing an ILT/PRP to the physical
#       layer with wait.
#
#  DESCRIPTION:
#       The specified request is queued to the physical layer.  This
#       routine does not return until that request has been completed.
#
#  CALLING SEQUENCE:
#       call    O$quereq
#
#  INPUT:
#       g1 = ILT
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
# C Access
# void ON_QueReq(ILT* pILT)
        .globl  ON_QueReq
ON_QueReq:
        mov     g0,g1                   # ILT
O$quereq:
        mov     g0,r15                  # Save g0
#
# --- Queue request w/ wait
#
        lda     P$que,g0                # Pass queuing routine
        call    K$qw                    # Queue request w/ wait
#
# --- Exit
#
        mov     r15,g0                  # Restore g0
        ret
#
#**********************************************************************
#
#  NAME: O$quereqnw
#
#  PURPOSE:
#       To provide a common means of queuing an ILT/PRP to the physical
#       layer without wait.  Releases request upon completion.
#
#  DESCRIPTION:
#       The specified request is queued to the physical layer.  This
#       routine returns immediately.
#
#  CALLING SEQUENCE:
#       call    O$quereqnw
#
#  INPUT:
#       g1 = ILT
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
O$quereqnw:
        movt    g0,r12                  # Save g0-g2
#
# --- Queue request w/ wait
#
        lda     P$que,g0                # Pass queuing routine
        lda     O$relreq,g2             # Complete by releasing
        call    K$q                     # Queue request w/o wait
        call    K$xchang                # Temporarily give up control
#
# --- Exit
#
        movt    r12,g0                  # Restore g0-g2
        ret
#
#**********************************************************************
#
#  NAME: O$relreq
#
#  PURPOSE:
#       To provide a common means of releasing any request previously
#       generated by O$genreq.
#
#  DESCRIPTION:
#       If a possible SGL exists, the buffers pointed to by that
#       structure are released back to the system.  The ILT/PRP and
#       possible SGL are also released back to the system.
#
#  CALLING SEQUENCE:
#       call    O$relreq
#
#  INPUT:
#       g1 = ILT
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
# C Access
# void ON_RelReq(ILT* pILT)
        .globl  ON_RelReq
ON_RelReq:
        mov     g0,g1                   # ILT
        call    O$relreq                # Call function
        ret
#
O$relreq:
        movl    g0,r14                  # Save g0-g1
#
# --- Locate PRP and possible SGL
#
        ld      il_w0(g1),r3            # Get PRP
        ld      pr_sglptr(r3),g0        # Get possible SGL
        cmpobe  0,g0,.rr10_o            # Jif none
#
# --- Release combined SGL/buffer
#
        st      0,pr_sglptr(r3)         # make sure cannot use sgl pointer again
        call    M$rsglbuf               # Release combined SGL/buffer
#
# --- Release ILT/PRP
#
.rr10_o:
        call    M$rip                   # Release ILT/PRP
#
# --- Exit
#
        movl    r14,g0                  # Restore g0-g1
        ret
#
#**********************************************************************
#
#  NAME: O$log_drive_removed
#
#  PURPOSE:
#       To provide a common means of reporting a drive removed.
#
#  DESCRIPTION:
#       The drive removed error log message is constructed with
#       information taken from the PDD and sent to the CCB.
#
#  CALLING SEQUENCE:
#       call    O$log_drive_removed
#
#  INPUT:
#       g3 = PDD
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
O$log_drive_removed:
        ldconst mledevremoved,r4        # Event code
        b       send_drive_message_2_ccb

#**********************************************************************
#
#  NAME: send_drive_message_2_ccb
#
#  PURPOSE:
#       Common code for O$log_drive_removed and O$log_drive_reset.
#
#  CALLING SEQUENCE:
#       b       send_drive_message_2_ccb
#
#  INPUT:
#       g3 = PDD
#       r4 = function code
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
send_drive_message_2_ccb:
        mov     g0,r12                  # Save g0
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        st      r4,mle_event(g0)        # Store as word to clear other bytes
        ld      pd_channel(g3),r4       # FC channel number and LUN
        st      r4,edr_channel(g0)      # Store
        stob    0,edi_geoflags(g0)      # this byte is reserved but we need to clear it
                                        # because common code in logview looks at it.
        ld      pd_id(g3),r4            # FC ID
        st      r4,edr_id(g0)
        ldl     pd_wwn(g3),r4           # FC WWN (r4, r5)
        stl     r4,edr_wwn(g0)          # (r4, r5)
        ldos    pd_pid(g3),r4           # load PID
        stos    r4,edr_pid(g0)          # store PID
        ldob    pd_devtype(g3),r4       # load Device Type
        stob    r4,edr_type(g0)         # store Device Type
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], edrlen);
        mov     r12,g0                  # Restore g0
        ret
#
#**********************************************************************
#
#  NAME: O$log_drive_reset
#
#  PURPOSE:
#       To provide a common means of reporting a drive reset.
#
#  DESCRIPTION:
#       The drive reset warning log message is constructed with
#       information taken from the PDD and sent to the CCB.
#
#  CALLING SEQUENCE:
#       call    O$log_drive_reset
#
#  INPUT:
#       g3 = PDD
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
O$log_drive_reset:
        ldconst mledevreset,r4          # Event code
        b       send_drive_message_2_ccb
#
#**********************************************************************
#
#  NAME: ON_LogSerialChanged
#
#  PURPOSE:
#       To provide a common means of reporting a drive with a
#       system serial number not equal to the system serial number.
#
#  DESCRIPTION:
#       The serial number changed error log message is constructed with
#       information taken from the PDD and sent to the CCB.
#
#  CALLING SEQUENCE:
#       call    ON_LogSerialChanged
#
#  INPUT:
#       g0 = PDD
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
# C access
# void ON_LogSerialChanged(PDD* pPDD);
        .globl  ON_LogSerialChanged     # C access
ON_LogSerialChanged:
        mov     g0,r12                  # Save g0
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst mleserialwrong,r4       # Event code
        st      r4,mle_event(g0)        # Store as word to clear other bytes
        ld      pd_channel(r12),r4      # FC channel number and LUN
        st      r4,esm_channel(g0)      # Store
        ld      pd_id(r12),r4           # FC ID
        st      r4,esm_id(g0)
        ldl     pd_wwn(r12),r4          # FC WWN (r4, r5)
        stl     r4,esm_wwn(g0)          # (r4, r5)
        ld      pd_sserial(r12),r4      # System serial number
        st      r4,esm_sserial(g0)
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], esmlen);
        mov     r12,g0                  # Restore g0
        ret
#
#**********************************************************************
#
#  NAME: O_Log_Bay_Moved
#
#  PURPOSE:
#       To provide a common means of reporting a drive or bay missing.
#
#  DESCRIPTION:
#       The drive or bay missing error log message is constructed with
#       information taken from the PSD and sent to the CCB.
#
#  CALLING SEQUENCE:
#       call    O_Log_Bay_Moved
#
#  INPUT:
#       g0 = PDD
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
# C access
# void ON_LogBayMoved(PDD* pPDD);
        .globl  ON_LogBayMoved          # C access
ON_LogBayMoved:
        ldconst mlediskbaymoved,r4      # Event code
        b       .lbm10                  # Log it
#
# C access
        .globl  ON_LogBayMissing        # C access
# void ON_LogBayMissing(PDD* pPDD);
ON_LogBayMissing:
        ldconst mlediskbayremoved,r4    # Event code
c       GR_SetRemovedBayMap(((PDD*)g0)->pid);
        b       .lbm10                  # Log it
#
# C access
# void ON_LogDriveMissing(PDD* pPDD);
        .globl  ON_LogDriveMissing      # C access
ON_LogDriveMissing:
        ldconst mledevmissing,r4        # Event code
# fall through
#
.lbm10:
        mov     g0,r12                  # Save g0
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        st      r4,mle_event(g0)        # Store as word to clear other bytes
        ld      pd_channel(r12),r4      # FC channel number and LUN
        st      r4,edx_channel(g0)      # Store
        ld      pd_id(r12),r4           # FC ID
        st      r4,edx_id(g0)
        ldl     pd_wwn(r12),r4          # FC WWN (r4, r5)
        stl     r4,edx_wwn(g0)          # (r4, r5)
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], edxlen);
        mov     r12,g0                  # Restore g0
        ret
#
#**********************************************************************
#
#  NAME: O_log_drive_inserted
#
#  PURPOSE:
#       To provide a common means of reporting a drive or bay insertion.
#
#  DESCRIPTION:
#       The drive or bay inserted error log message is constructed with
#       information taken from the PDD and sent to the CCB.
#
#  CALLING SEQUENCE:
#       call    O_log_drive_inserted
#
#  INPUT:
#       g0 = PDD
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
# C access
# void ON_LogBayInserted(PDD* pPDD);
        .globl  ON_LogBayInserted       # C access
ON_LogBayInserted:
        ldconst mlediskbayinserted,r4   # Event code
        b       .lbi10                  # Log it
#
# C access
# void ON_LogDriveInserted(PDD* pPDD);
        .globl  ON_LogDriveInserted     # C access
ON_LogDriveInserted:
        ldconst mledevinsert,r4         # Event code
# fall through
#
.lbi10:
        mov     g0,r12                  # Save g0
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        st      r4,mle_event(g0)        # Store as word to clear other bytes
        ld      pd_channel(r12),r4      # FC channel number and LUN
        st      r4,edi_channel(g0)      # Store
        ldob    pd_geoflags(r12),r4
#        bbc     pdcrosslocationinsertion,r4,.lbi15
        stob    r4,edi_geoflags(g0)
# .lbi15:
        ld      pd_id(r12),r4           # FC ID
        st      r4,edi_id(g0)
        ldl     pd_wwn(r12),r4          # FC WWN (r4, r5)
        stl     r4,edi_wwn(g0)          # (r4, r5)
        ldos    pd_pid(r12),r4           # load PID
        stos    r4,edi_pid(g0)          # store PID
        ldob    pd_devtype(r12),r4      # load Device Type
        stob    r4,edi_type(g0)         # store Device Type
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], edilen);
        mov     r12,g0                  # Restore g0
        ret
#
#**********************************************************************
#
#  NAME: O_Log_Device_SPath
#
#  PURPOSE:
#       To provide a common means of reporting a device that has only
#       a single path to it.
#
#  DESCRIPTION:
#       The device single path error log message is constructed with
#       information taken from the PDD and sent to the CCB.
#
#  CALLING SEQUENCE:
#       call    O_Log_Device_SPath
#
#  INPUT:
#       g0 = PDD
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
# C access
# void ON_LogDeviceSPath(PDD* pPDD);
        .globl  ON_LogDeviceSPath       # C access
ON_LogDeviceSPath:
        mov     g0,r12                  # Save g0
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst mlesinglepath,r4        # Event code
        st      r4,mle_event(g0)        # Store as word to clear other bytes
        ldl     pd_wwn(r12),r4          # Get WWN (r4, r5)
        stl     r4,dsp_devwwn(g0)       # (r4, r5)
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], dsplen);
        mov     r12,g0                  # Restore g0
        ret
#
#**********************************************************************
#
#  NAME: o$log_reattaching
#
#  PURPOSE:
#       To provide a common means of reporting that a drive has been
#       reattached to a PDD.
#
#  DESCRIPTION:
#       The drive reattached error log message is constructed with
#       information taken from the PDD and sent to the CCB.
#
#  CALLING SEQUENCE:
#       call    o$log_reattaching
#
#  INPUT:
#       g3 = PDD
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
# C access
# void ON_LogDriveReattached(PDD* pPDD);
        .globl  ON_LogDriveReattached     # C access
ON_LogDriveReattached:
        mov     g0,g3
# fall through
#
o$log_reattaching:
        mov     g0,r12                  # Save g0
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst mledevreattach,r4       # Event code
        st      r4,mle_event(g0)        # Store as word to clear other bytes
        ld      pd_channel(g3),r4       # FC channel number and LUN
        st      r4,eda_channel(g0)      # Store
        ldob    pd_geoflags(g3),r4
#        bbc     pdcrosslocationinsertion,r4,.ldr15
        stob    r4,edi_geoflags(g0)
# .ldr15:
        ld      pd_id(g3),r4            # FC ID
        st      r4,eda_id(g0)
        ldl     pd_wwn(g3),r4           # FC WWN (r4, r5)
        stl     r4,eda_wwn(g0)          # (r4, r5)
        ldos    pd_pid(g3),r4           # load PID
        stos    r4,eda_pid(g0)          # store PID
        ldob    pd_devtype(g3),r4       # load Device Type
        stob    r4,eda_type(g0)         # store Device Type

# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], edalen);
        mov     r12,g0                  # Restore g0
        ret
#
#**********************************************************************
#
#  NAME: o$log_hotspareinop
#
#  PURPOSE:
#       To provide a common means of reporting that a hotspare
#       is no longer operative.
#
#  DESCRIPTION:
#       This message is constructed from g register input.
#
#  CALLING SEQUENCE:
#       call    o$log_hotspareinop
#
#  INPUT:
#       g3      PDD
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
o$log_hotspareinop:
        mov     g0,r12                  # Save g0
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst mlehotspareinop,r4      # Event code
        st      r4,mle_event(g0)        # Store as word to clear other bytes
        ldos    pd_pid(g3),r4           # PID
        stos    r4,hsi_pid(g0)          # PID
        ldl     pd_wwn(g3),r4           # WWN (r4, r5)
        stl     r4,hsi_wwn(g0)          # WWN (r4, r5)
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], hsilen);
        mov     r12,g0                  # Restore g0
        ret
#
#**********************************************************************
#
#  NAME: O$ledchanged
#
#  PURPOSE:
#       To provide a means of notifying the CCB that an LED state
#       should be changed.
#
#  DESCRIPTION:
#       Send a log message to the CCB indicating that an LED should
#       be changed to a new value.
#
#  CALLING SEQUENCE:
#       call    O$ledchanged
#
#  INPUT:
#       g3 - PDD for which the LED should be changed.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
# void ON_LedChanged(PDD* pPDD);
# C Access
        .globl  ON_LedChanged
ON_LedChanged:
        mov     g0,g3
# fall through
#
O$ledchanged:
        mov     g0,r12                  # Save g0
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst mleled,r4               # Event code
        st      r4,mle_event(g0)        # Store as word to clear other bytes
        ldl     pd_wwn(g3),r4           # World wide name (r4, r5)
        stl     r4,elc_wwn(g0)          # Store it (r4, r5)
        ldob    pd_fled(g3),r4          # Get the LED state
        stob    r4,elc_state(g0)        # Store it
        ldos    pd_lun(g3),r4           # Get the LUN
        stos    r4,elc_lun(g0)          # Store it
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], elclen);
        mov     r12,g0                  # Restore g0
        ret
#
#**********************************************************************
#
#  NAME: O_logerror
#
#  PURPOSE:
#       To provide a common means of logging events which contain no
#       more parameters other than the error code itself.
#
#  DESCRIPTION:
#       An error log message code is constructed with the supplied
#       error code and that message is sent to the CCB.
#
#  CALLING SEQUENCE:
#       call    O_logerror
#
#  INPUT:
#       g0 = error code
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
# C access
# void ON_LogError(UINT32 errorCode);
        .globl  ON_LogError             # same as above
ON_LogError:
O_logerror:
        mov     g0,r12                  # Save g0
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        st      r12,mle_event(g0)       # Store as word to clear other bytes
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], mlesiz);
        mov     r12,g0                  # Restore g0
        ret
#
#****************************************************************************
#
#**********************************************************************
#
#  NAME: ON_LogDriveDelay
#
#  PURPOSE:
#       To provide a common means of reporting that a drive has failed
#       many operations and is a candidate to be bypassed.
#
#  DESCRIPTION:
#       The drive delay error log message is constructed with
#       information taken from the PDD and sent to the CCB.
#
#  CALLING SEQUENCE:
#       call    ON_LogDriveDelay
#
#  INPUT:
#       g0 = PDD
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
# C access
# void ON_LogDriveDelay(PDD* pPDD);
        .globl  ON_LogDriveDelay       # C access
ON_LogDriveDelay:
        mov     g0,r12                  # Save g0
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst mledrivedelay,r4        # Event code
        st      r4,mle_event(g0)        # Store as word to clear other bytes
        ldl     pd_wwn(r12),r4          # Get WWN (r4, r5)
        stl     r4,ddly_devwwn(g0)      # (r4, r5)
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], ddlylen);
        mov     r12,g0                  # Restore g0
        ret
#
#**********************************************************************
#
#  NAME: ON_BypassCmd
#
#  PURPOSE:
#       To provide a simple and efficient means of generating ILT/PRP/SGL
#       structures in support of bypassing a drive using a SCSI
#       operation, submitting the command, waiting for completion and
#       copying the data into user space.
#
#  DESCRIPTION:
#       The information present within the template is used to construct
#       an ILT and PRP.  If a data transfer is involved an SGL and
#       associated buffer is also allocated and linked.
#
#       Although it is highly unusual, the possibility exists that this
#       routine may block if any of the requested resources are
#       unavailable.
#
#       The command is then issued and when completed, checked for good
#       status.  If everything worked, then any returned data is placed
#       into memory for the caller.  The command structures are then
#       returned to the pools.
#
#  CALLING SEQUENCE:
#       call    ON_BypassCmd
#
#  INPUT:
#       g0 = template address
#       g1 = buffer address for Page 82/83 output data
#       g2 = size of buffer allocated
#       g3 = PDD
#       g4 = pointer to sense data (unallocated buffer space)
#       g5 = channel
#       g6 = lid
#
#  OUTPUT:
#       g0 = status from the command (0 = good)
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
# C access
# UINT32 ON_BypassCmd(void* template, void* outputBuffer, UINT32 size,
#                 PDD* pPDD, SNS* * pSenseData, UINT8 channel, UINT16 lid);
        .globl  ON_BypassCmd            # C access
ON_BypassCmd:
        movq    g0,r12                  # Save inputs
        ldconst 0,r3
        st      r3,(g4)                 # Zero the return pointer
#
        call    O$genreq                # Gen request (g0 = template, g3 = PDD)
#
# --- Change the channel and the lid
#
        stob    g5,pr_channel(g2)       # Store new channel
        st      g6,pr_id(g2)            # Store new lid
#
# --- Copy the data into the output SGL buffers.
#
        ld      pr_rqbytes(g2),r3       # Get byte count to copy
        ld      pr_sglptr(g2),r4        # Get pointer to SGL
        ld      sg_desc0+sg_addr(r4),r4 # Pointer to buffer
#
.bc10:
        subo    1,r3,r3                 # Decrement counter
        ldob    (r13)[r3*1],r5          # Get a byte
        stob    r5,(r4)[r3*1]           # Save it
        cmpibl  0,r3,.bc10              # Jif more to copy
#
        call    O$quereq                # Queue request
        call    M$chkstat               # Check status
        mov     g0,r12                  # Save return code (good or bad)
        cmpobne ecok,g0,.bc40           # Jif error
#
# --- Operation completed OK.  Copy the data to the read buffer in the
# --- case of a read type operation.
#
        ldob    pr_func(g2),r3          # Get the function code
        cmpobne prinput,r3,.bc100       # Jif not input command (no input data)
#
        ld      pr_rqbytes(g2),r3       # Get byte count to copy
        ld      pr_sglptr(g2),r4        # Get pointer to SGL
        ld      sg_desc0+sg_addr(r4),r4 # Pointer to buffer
#
.bc30:
        subo    1,r3,r3                 # Decrement counter
        ldob    (r4)[r3*1],r5           # Get a byte
        stob    r5,(r13)[r3*1]          # Save it
        cmpibl  0,r3,.bc30              # Jif more to copy
        b       .bc100                  # Exit
#
# --- Error exit.  Copy the sense data if there was a CC, otherwise, set
# --- the return code to -1 to indicate some other error.
#
.bc40:
        ldconst eccheck,r3              # Check for check condition
        cmpobne r3,g0,.bc100            # Jif not CC (return code from queureq)
#
# --- Allocate a sense area, copy the data and put the pointer into the
# --- return area (r12).
#
c       g0 = s_MallocW(SENSESIZ, __FILE__, __LINE__);
        st      g0,(g4)                 # Set pointer from input block
#
        ldq     pr_sense(g2),r4         # Get first quad
        stq     r4,(g0)
        ldq     pr_sense+16(g2),r4      # Get second quad
        stq     r4,16(g0)
#
.bc100:
        call    O$relreq                # Release the request
        movq    r12,g0                  # Restore input parms and return code
        ret
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
