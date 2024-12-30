# $Id: definebe.as 162911 2014-03-20 22:45:34Z marshall_midden $
#**********************************************************************
#
#  NAME: definebe.as
#
#  PURPOSE:
#
#       To provide complete support of configuration definition requests.
#
#  FUNCTIONS:
#
#       This module employs these processes:
#
#       d$exec            - Executive (1 copy)
#
#  Copyright (c) 1997-2010  Xiotech Corporation. All rights reserved.
#
#**********************************************************************
#
# --- global function declarations ------------------------------------
#
        .globl  D$init                  # Module initialization
        .globl  D$p2update              # update part 2 nvram
        .globl  D$p2updateconfig        # update part 2 nvram
        .globl  D$cmpltserverupd        # Send Config Server Complete MRP to FEP
        .globl  D$updrmtcache           # Update front end on cache state
        .globl  D$updrmtsysinfo         # Update front end system information
        .globl  D$findpdd               # Find a PDD given the FC channel and ID
        .globl  D$signalserverupdate    # Signal FEP that an update has occurred
        .globl  D$signalvdiskupdate     # Signal FEP that an update has occurred
        .globl  D$updrmtcacheglobal     # Signal FEP to update global cache
        .globl  D$changename            # Set a name in the CCB
        .globl  D$updrmtcachesingle     # Signal FEP to update single cache record
        .globl  D$SendRefreshNV         # Refresh NRVAM on slave processor
        .globl  D$calcspaceshell        # Calculate the space on the drive
        .globl  D$damdirtyshell         # Dirty the DAM on the drive
        .globl  D$ctlrqst_cr            # PCP request ILT completion handler routine
        .globl  D$spool_expand          # expand snap pool
        .globl  D$update_spool_percent_used # update the percent full
        .globl  DEF_update_spool_percent_used  # update the percent full 'c' access
#
        .globl  D_updrmttarg            # Update front end target (C code access)
        .globl  D_insertpdd             # Insert a PDD into the PDX (C code access)
        .globl  D_updrmtserver          # Send Config Server MRP to FEP (C code access)
        .globl  D$reportEvent           # Send event report to the CCB
        .globl  d$deletevlink           # Delete VLink
        .globl  DEF_TerminateBackground#  Terminate background processes
        .globl  D$cow_q_task            # Copy on write queue handler process
#
# --- global data declarations ----------------------------------------
#
        .globl  D_glcache               # Global cache enable T/F
        .globl  D_gpri                  # Global priority
        .globl  D_Vflag                 # VDisk/VLink busy flag
        .globl  D_Vcl                   # VDisk/VLink busy cluster #
        .globl  D_Vvid                  # VDisk/VLink busy virtual ID
        .globl  D_ften                  # Foreign target enable bitmap
        .globl  D_moveinprogress        # Target move is in-progress
        .globl  D_rescanDeviceTaskPCB   # Debug purposes
        .globl  D_gp2strat              # Global priority 2 strategy table
        .globl  defTraceQue
        .globl  D_p2update_flag         # P2 NVRAM update flag
        .globl  d_resync_passkey
        .globl  DEF_oger_cnt            # C-access oger count
        .globl  oger_cnt                # Assembly access
#
# --- physical resident data
#
        .globl  P_pddindx               # Index to PDD index
        .globl  gPDX
#
# --- raid resident data
#
        .globl  R_rddindx               # Index to RDD structures
        .globl  gRDX
        .globl  R_scrubopt              # Scrub enable T/F
#
# --- virtual resident data
#
        .globl  V_vddindx               # Index to VDD index
        .globl  gVDX
#
# --- server resident data
#
        .globl  S_sddindx               # Index to SDD index
        .globl  gSDX
#
# --- enclosure resident data
#
        .globl  E_pddindx               # Index to SES devices index
        .globl  gEDX
#
# --- non-disk, non-SES resident data
#
        .globl  M_pddindx               # Index to misc SCSI devices index
        .globl  gMDX
#
# --- target table
#
        .globl  T_tgdindx               # Index to targets
        .globl  gTDX
#
# --- DLM Link Device Descriptor data
#
        .globl  DLM_lddindx
        .globl  DLM_master_sn           # Group Master Controller Serial #
        .globl  DLM$def_master          # Notify all XIOtech controllers of the
                                        #  group master
#
# --- copy manager resident routines
#
        .globl  CM$scstart              # Start secondary copy.
        .globl  CM$ctlrqstq             # Enqueue control requests
#        .globl  CM$ChkCopy              # check for copy/mirror
        .globl  CM$pksnd_local_poll     # Pack and send a local poll request
                                        #  for the specified copy operation.
        .globl  CM$pkop_dmove           # Pack a Copy Device Moved datagram
                                        #  message
        .globl  CM$whack_rcor           # Whack remote CORs associated with
                                        #  VLAR
        .globl  CM$find_cor_rid         # Find a COR associated with a S/N,ID
                                        #  pair
        .globl  CM$term_cor             # Terminate copy operation.
#
# --- Mirror partner maps
#
        .globl  D_mpmaps                # Mirror partner mappings
        .globl  D_mpcnt                 # Mirror partner mappings count
#
# --- NVRAM resident data
#
        .globl  NV_scsi_whql
#
# --- Starting and ending locations of the area in memory to clear at reset.
#
        .globl  D_fillstart
        .globl  D_fillend
#
# --- global usage data definitions -----------------------------------
#
        .section end,bss
D_fillstart:
#
gPDX:
        .space  4,0                     # Entry count for PDX
P_pddindx:
        .space  MAXDRIVES*4,0           # Index to PDD structures
#
gRDX:
        .space  4,0                     # Entry count for RDX
R_rddindx:
        .space  MAXRAIDS*4,0            # Index to RDD structures
#
gVDX:
        .space  4,0                     # Entry count for VDX
V_vddindx:
        .space  MAXVIRTUALS*4,0         # Index to VDD structures
#
gSDX:
        .space  4,0                     # Entry count for SDX
S_sddindx:
        .space  MAXSERVERS*4,0          # Index to SDD structures
#
gEDX:
        .space  4,0                     # Entry count for EDX
E_pddindx:
        .space  MAXSES*4,0              # Index to SES PDD structures
#
gMDX:
        .space  4,0                     # Entry count for MDX
M_pddindx:
        .space  MAXMISC*4,0             # Index to misc PDD structures
#
/* NOTE: gTDX  is T_tgdindx[-1] */
gTDX:
        .space  4,0                     # Entry count for TGDX
T_tgdindx:
        .space  MAXTARGETS*4,0          # Index to TGD structures
#
DLM_lddindx:
        .space  MAXLDDS*4,0             # Index to LDD structures
#
# --- This is a filler area to make sure there is enough room to
# --- quad zero fill the above area. This area will be zeroed by
# --- online so that we can move it out of code space.
#
        .align  4
D_fillend:
#
        .data
#
D_mpmaps:
        .space  MAXCTRL*4*2             # 2 Words for each entry
#
D_mpcnt:
        .byte   0                       # Count of mappings
#
D_glcache:
        .byte   FALSE                   # global cache enable flag (T/F)
#
D_ften:
        .byte   0                       # foreign target enable bit map
#
D_gpri:
        .byte   7                       # Global priority (0-7)
                                        # See MAX_GLOBAL_PRIORITY.
#
# --- Global priority 2 strategy conversion table
#
D_gp2strat:
        .byte   prlow                   # Strategy - low
        .byte   prlow                   # Strategy - low
        .byte   prlow                   # Strategy - low
        .byte   prnorm                  # Strategy - normal
        .byte   prnorm                  # Strategy - normal
        .byte   prnorm                  # Strategy - normal
        .byte   prhigh                  # Strategy - high
        .byte   prhigh                  # Strategy - high
#
# --- local usage data definitions ------------------------------------
#
# --- Executive QCB
#
        .section        .shmem
        .globl  d_exec_qu               # Easier for debugging
        .align  4
d_exec_qu:
        .word   0                       # Queue head
        .word   0                       # Queue tail
        .word   0                       # Queue count
        .word   0                       # Associated PCB
#
        .align  4
d_labelpcb:
        .word   0                       # d$label PCB
#
# ----------------------------------------------------------------------------
        .data
#
# --- Target movement synchronization
#
D_moveinprogress:                       # Target move is in-progress
        .word   FALSE
#
d_labelpnd:
        .byte   FALSE                   # Label pending state
#
D_p2update_flag:
        .byte   0                       # p2update flag
                                        # 00 = not processing p2update
                                        # 01 = p2update in process
                                        # 02 = p2update in process and needs
                                        #      to be done again
#
D_rescanDeviceTaskPCB:
        .word   0
#
# --- resynce copy list
#
d_resync_list:
        .word   0
d_resync_paskey:
        .word   0
#
d_myc_mrp_len:
        .word   0
#
DEF_oger_cnt:
oger_cnt:
        .word   0
        .word   0
spool_percent_full:
        .byte   0
        .byte   0
snapshot_max_cnt:
        .short  SS_COUNT_LIMIT
#
# --- VDisk/VLink busy control
#
D_Vflag:
        .byte   FALSE                   # VDisk/VLink busy flag [T/F]
D_Vcl:
        .byte   0                       # VDisk/VLink busy cluster #
D_Vvid:
        .short  0                       # VDisk/VLink busy virtual ID
#
# --- Error block for gang label.
#
        .globl  d_ganglabel
        .set    glt_taskcount,0
        .set    glt_errcount,4
        .set    glt_errcode,8
#
        .align  4
#
d_ganglabel:
        .word   0                       # Task count
        .word   0                       # Error count
        .word   0                       # Error code
        .word   0                       # Pad
#
# --- Locals for break mirror group function
#
d_brgp_base:                            # Sequence table addr
    .word       0
d_brgp_count:                           # Sequence count broadcasted
    .word       0
d_brgp_cur:                             # Current sequence count
    .word       0
d_brgp_exec:                            # Has sequence been executed (boolean)
    .word       0
#
# --- Sequence struct for break mirror group function
#
#      NOTE:    changes in the brpg entry structure will require changes in
#               the multiplier of some instructions
#
        .set    brgp_did,0              # <s> Destination cluster
        .set    brgp_sid,2              # <s> source VID
        .set    brgp_op,4               # <b> operation to be performed
        .set    brgp_rsvd,5
        .set    brgp_seq_siz,brgp_did+8 # Size of each sequence entry
#
        .set    brgp_mask_lower,0xfff   # Lower 12 bits are source VID.
        .set    brgp_sequence,15        # Top bit of src means sequence.
#
# --- Ordered list of action routines organized by packet type
#
        .align  4
d_pktact:
        .word   d$crexpvirt             # (0x100) Create / Exp / Test
        .word   d$cbridge               # (0x101) Get SES list
        .word   d$cbridge               # (0x102) Label a physical
        .word   RB$faildev              # (0x103) Fail a device
        .word   d$scsiio                # (0x104) Raw SCSI IO
        .word   D$initraid              # (0x105) Initialize a RAID
        .word   d$fcalanal              # (0x106) FCAL analysis
        .word   d$deletevirt            # (0x107) Delete a virtual
        .word   d$setcache              # (0x108) Set caching mode
        .word   d$setserverprop         # (0x109) Set server properties
        .word   d$cbridge               # (0x10A) Reset NVRAM, clear
        .word   d$cbridge               # (0x10B) Restore from NVRAM
        .word   d$uawake                # (0x10C) Awake
        .word   d$wwnlunlookup          # (0x10D) WWN / LUN lookup
        .word   d$cbridge               # (0x10E) Generic
        .word   d$spinstate             # (0x10F) Start or stop
        .word   d$setscrub              # (0x110) Enable/disable scrub
        .word   d$defaultlabel          # (0x111) Set default label
        .word   d$cbridge               # (0x112) Get Device Paths
        .word   d$restoredev            # (0x113) Restore device
        .word   d$cbridge               # (0x114) Defragment device
        .word   d$cbridge               # (0x115) Set attribute
        .word   d$cbridge               # (0x116) Get loop statistics
        .word   d$cbridge               # (0x117) Get server list
        .word   d$cbridge               # (0x118) Get virtual list
        .word   d$cbridge               # (0x119) Get RAID list
        .word   d$cbridge               # (0x11A) Get physical list
        .word   d$cbridge               # (0x11B) Get miscellaneous list
        .word   d$cbridge               # (0x11C) Get virtual info
        .word   d$cbridge               # (0x11D) Get RAID info
        .word   d$cbridge               # (0x11E) Get physical info
        .word   d$cbridge               # (0x11F) Map LUN to VDisk
        .word   d$cbridge               # (0x120) Unmap LUN from VDisk
        .word   d$cbridge               # (0x121) Get SES information
        .word   d$cbridge               # (0x122) Create server
        .word   d$cbridge               # (0x123) Delete server
        .word   d$cbridge               # (0x124) Get miscellaneous info
        .word   d$vdctl                 # (0x125) Virtual device control
        .word   d$setserial             # (0x126) Set system information
        .word   d$getii                 # (0x127) Get II information
        .word   d$getlink               # (0x128) Get link information
        .word   d$obsolete              # (0x129) Was Get backend boot code header
        .word   d$obsolete              # (0x12A) Was Get backend diag code header
        .word   d$getproc               # (0x12B) Get backend proc code header
        .word   D$nop                   # (0x12C) No op
        .word   d$rwmemory              # (0x12D) Read/write memory
        .word   d$configtarg            # (0x12E) Configure a target
        .word   d$cbridge               # (0x12F) Get mirror partner list
        .word   d$setgpri               # (0x130) Set global priority
        .word   d$cbridge               # (0x131) Get target list
        .word   d$cbridge               # (0x132) Reset BE chip
        .word   d$obsolete              # (0x133) OBSOLETE
        .word   d$rmtctrlcnt            # (0x134) Get remote controller count
        .word   d$rmtctrlinfo           # (0x135) Get remote controller info
        .word   d$rmtvdiskinfo          # (0x136) Get remote vdisk info
        .word   d$setforeigntarget      # (0x137) Set foreign targets
        .word   d$createvlink           # (0x138) Create a VLink
        .word   d$getvlinkinfo          # (0x139) Get VLink information
        .word   d$cbridge               # (0x13A) Create new controller
        .word   d$rescanDevice          # (0x13B) Rescan physical devices
        .word   d$resync                # (0x13C) Resync RAID or stripes
        .word   d$getlocalimage         # (0x13D) Get local image of NVRAM
        .word   d$putlocalimage         # (0x13E) Put local image of NVRAM
        .word   d$deletedevice          # (0x13F) Delete physical disk or SES
        .word   d$modepage              # (0x140) Mode page
        .word   d$devicecount           # (0x141) Get device count for serial number
        .word   d$cbridge               # (0x142) Get Vdisk Owner
        .word   RB$hotspareinfo         # (0x143) Get hotspare information
        .word   d$filecopy              # (0x144) File system file copy
        .word   d$cbridge               # (0x145) Get Device List
        .word   d$cbridge               # (0x146) Get Port List
        .word   d$brvlock               # (0x147) Break VLock
        .word   D$getsos                # (0x148) Get SOS structure
        .word   d$obsolete              # (0x149) Put SOS structure
        .word   d$forceerrortrap        # (0x14A) Force an error trap
        .word   d$putscmt               # (0x14B) Put SCMT structure
        .word   d$cbridge               # (0x14C) Perform BE Loop Primitive
        .word   d$targetcontrol         # (0x14D) Target movement control
        .word   d$cbridge               # (0x14E) Fail controller (TBD)
        .word   d$namedevice            # (0x14F) Set device name
        .word   d$ebiseDG               # (0x150) interprocessor DataGram
        .word   D$nop                   # (0x151) No op
        .word   d$putfsys               # (0x152) Put FSys report
        .word   d$get_dlink             # (0x153) Get DLink Information
        .word   d$get_dlock             # (0x154) Get DLock Information
        .word   d$cbridge               # (0x155) Degrade / restore port
        .word   d$cbridge               # (0x156) Get workset information
        .word   d$cbridge               # (0x157) Set workset information
        .word   d$cbridge               # (0x158) Set FC port configuration
        .word   d$cbridge               # (0x159) Save Async NV to file
        .word   d$cbridge               # (0x15A) Modify RAID AStatus Field
        .word   d$putldd                # (0x15B) Put the LDD
        .word   d$r5recover             # (0x15C) Start recovery on an Inop R5
        .word   d$cbridge               # (0x15D) Put device configuration
        .word   d$copydata              # (0x15e) Copy data
        .word   d$copyctl               # (0x15f) Copy control
        .word   d$cbridge               # (0x160) Refresh from NVRAM
        .word   d$cbridge               # (0x161) Set Vdisk Priority
        .word   d$cbridge               # (0x162) Enable/Disable Vdisk Priority
        .word   O$pdiskspindown         # (0x163) Spindown the designated disk
        .word   RB$failbackdev          # (0x164) Failback the designated hotspare
                                        #         back to its previous data drive
        .word   RB$autofailback         # (0x165) Auto failback feature enable / disable
        .word   d$cfgoption             # (0x166) Configure Options
        .word   d$cbridge               # (0x167) Set Target Info
        .word   d$cbridge               # (0x168) Get Target Info
        .word   d$cbridge               # (0x169) Update iSCSI Server WWN
        .word   d$cbridge               # (0x16A) Configure CHAP user
        .word   d$cbridge               # (0x16B) CHAP user Info
        .word   d$cbridge               # (0x16C) Get the Geo location code
        .word   d$cbridge               # (0x16D) Set the Geo location code
        .word   d$cbridge               # (0x16E) Clear the Geo location code
        .word   d$cbridge               # (0x16F) Get iSNS information
        .word   d$cbridge               # (0x170) Set iSNS information
        .word   d$cbridge               # (0x171) Set PR data
        .word   d$cbridge               # (0x172) Get VDisk Redundancy
        .word   d$cbridge               # (0x173) Get Async Replication data.
        .word   d$cbridge               # (0x174) Get ISE Bay IP addresses.
        .word   d$cbridge               # (0x175) All devices Missing
        .word   d$cbridge               # (0x176) Get Extended Vdisk info.
        .word   d$cbridge               # (0x177) Emulate BE Qlogic timeout on pdisk
        .word   d$get_dlink             # (0x178) Get DLink Information -- GT2TB version
        .word   d$vpreadwrite           # (0x179) Read/Write Pdisk or Vdisk
        .word   d$cbridge               # (0x17A) Swap two PIDs
        .word   d$cbridge               # (0x17B) Emulate PAB
#
        .word   d$cbridge               # (0x400) Get VID info for FEP
        .word   d$setseq                # (0x401) Set Sequence Number
        .word   d$setmpconfigbe         # (0x402) Set MP config BE
        .word   d$cbridge               # (0x403) Update Persistent Reservation
#
# --- Ordered list of request lengths
#
# --- These are defined as shorts since there are none longer at
# --- this time. The length field in the MRPs is a word, so to
# --- expand to this size if needed, only a change to this table
# --- and to the code that loads this value would be needed.
#
d_exppktlen:
        .short  0xffff                  # (0x100) Create / expand vdisk
        .short  mgxsiz                  # (0x101) Get SES list
        .short  mldsiz                  # (0x102) Label a physical
        .short  mfdsiz                  # (0x103) Fail a device
        .short  msisiz                  # (0x104) Raw SCSI IO
        .short  midsiz                  # (0x105) Initialize a RAID
        .short  mfasiz                  # (0x106) FCAL analysis
        .short  mdvsiz                  # (0x107) Delete a virtual
        .short  mscsiz                  # (0x108) Set caching mode
        .short  mprsiz                  # (0x109) Set server properties
        .short  mxnsiz                  # (0x10A) Reset NVRAM, clear
        .short  mrnsiz                  # (0x10B) Restore from NVRAM
        .short  mpgsiz                  # (0x10C) Awake
        .short  mwlsiz                  # (0x10D) WWN / LUN lookup
        .short  0xffff                  # (0x10E) Generic
        .short  msssiz                  # (0x10F) Start or stop
        .short  medsiz                  # (0x110) Enable/disable scrub
        .short  msdsiz                  # (0x111) Set default label
        .short  mflsiz                  # (0x112) Get Device Paths
        .short  mdrsiz                  # (0x113) Restore device
        .short  mddsiz                  # (0x114) Defragment device
        .short  maasiz                  # (0x115) Set attribute
        .short  mflsiz                  # (0x116) Get loop statistics
        .short  mgxsiz                  # (0x117) Get server list
        .short  mgxsiz                  # (0x118) Get virtual list
        .short  mgxsiz                  # (0x119) Get RAID list
        .short  mgxsiz                  # (0x11A) Get physical list
        .short  mgxsiz                  # (0x11B) Get miscellaneous list
        .short  mivsiz                  # (0x11C) Get virtual info
        .short  mirsiz                  # (0x11D) Get RAID info
        .short  mipsiz                  # (0x11E) Get physical info
        .short  mmlsiz                  # (0x11F) Map LUN to VDisk
        .short  mulsiz                  # (0x120) Unmap LUN from VDisk
        .short  mipsiz                  # (0x121) Get SES information
        .short  mmssiz                  # (0x122) Create server
        .short  mdssiz                  # (0x123) Delete server
        .short  mipsiz                  # (0x124) Get miscellaneous info
        .short  mvcsiz                  # (0x125) Virtual device control
        .short  massiz                  # (0x126) Set system information
        .short  mgisiz                  # (0x127) Get II information
        .short  mfpsiz                  # (0x128) Get PCI information
        .short  mfhsiz                  # (0x129) Get backend boot code header
        .short  mfhsiz                  # (0x12A) Get backend diag code header
        .short  mfhsiz                  # (0x12B) Get backend proc code header
        .short  0                       # (0x12C) No op
        .short  mrwsiz                  # (0x12D) Read/write memory
        .short  mctsiz                  # (0x12E) Configure a target
        .short  mgxsiz                  # (0x12F) Get mirror partner list
        .short  mbpsiz                  # (0x130) Set global priority
        .short  mgxsiz                  # (0x131) Get target list
        .short  mflsiz                  # (0x132) Reset BE chip
        .short  0xffff                  # (0x133) OBSOLETE
        .short  mncsiz                  # (0x134) Get remote controller count
        .short  mcisiz                  # (0x135) Get remote controller info
        .short  mvisiz                  # (0x136) Get remote vdisk info
        .short  mftsiz                  # (0x137) Set foreign targets
        .short  mcvsiz                  # (0x138) Create a VLink
        .short  mvlsiz                  # (0x139) Get VLink information
        .short  mmcsiz                  # (0x13A) Create controller
        .short  mrdsiz                  # (0x13B) Rescan physical devices
        .short  mrbsiz                  # (0x13C) Resync RAID or stripes
        .short  mgnsiz                  # (0x13D) Get local image of NVRAM
        .short  munsiz                  # (0x13E) Put local image of NVRAM
        .short  mxdsiz                  # (0x13F) Delete physical disk or SES
        .short  mmpsiz                  # (0x140) Mode page
        .short  mdcsiz                  # (0x141) Get device count for serial number
        .short  mivsiz                  # (0x142) Get Vdisk Owner
        .short  mrhsiz                  # (0x143) Get hotspare information
        .short  mfcsiz                  # (0x144) File system file copy
        .short  mflsiz                  # (0x145) Get Device List
        .short  mgxsiz                  # (0x146) Get Port List
        .short  mbvsiz                  # (0x147) Break VLock
        .short  messiz                  # (0x148) Get SOS structure
        .short  0xffff                  # (0x149) Put SOS structure
        .short  metsiz                  # (0x14A) Force an error trap
        .short  mpcsiz                  # (0x14B) Put SCMT structure
        .short  mlpsiz                  # (0x14C) Perform BE Loop Primitive
        .short  mtcsiz                  # (0x14D) Target movement control
        .short  mcosiz                  # (0x14E) Fail controller
        .short  mndsiz                  # (0x14F) Name device
        .short  0xffff                  # (0x150) interprocessor DataGram
        .short  0                       # (0x151) No op
        .short  0xffff                  # (0x152) Put FSys report
        .short  mnisiz                  # (0x153) Get DLink Information
        .short  mkisiz                  # (0x154) Get DLock Information
        .short  mposiz                  # (0x155) Degrade / restore port
        .short  mgetwssiz               # (0x156) Get workset information
        .short  msetwssiz               # (0x157) Set workset information
        .short  0xffff                  # (0x158) Set port config
        .short  0xffff                  # (0x159) Save async NV to file
        .short  mrchgraidnotmirroringsiz # (0x15A) Modify RAID AStatus Field
        .short  mplsiz                  # (0x15B) Put the LDD
        .short  mr5siz                  # (0x15C) Start recovery on an Inop R5
        .short  0xffff                  # (0x15D) Put device configuration
        .short  mydsiz                  # (0c15e) Copy data
        .short  mycsiz                  # (0x15f) Copy control
        .short  mrnsiz                  # (0x160) Refresh from NVRAM
        .short  0xffff                  # (0x161) Set Vdisk Priority
        .short  0xffff                  # (0x162) Enable/Disable Vdisk Priority
        .short  0xffff                  # (0x163) Spindown physical disk
        .short  0xffff                  # (0x164) Failback physical disk
        .short  0xffff                  # (0x165) Auto Failback enable / disable
        .short  0xffff                  # (0x166) Configure Options
        .short  0xffff                  # (0x167) Set Target Info
        .short  0xffff                  # (0x168) Get Target Info
        .short  0xffff                  # (0x169) Update iSCSI Server WWN
        .short  0xffff                  # (0x16A) Configure CHAP user
        .short  0xffff                  # (0x16B) CHAP user Info
        .short 0xffff                   # (0x16C) Get the Geo location code
        .short 0xffff                   # (0x16D) Set the Geo location code
        .short 0xffff                   # (0x16E) Clear the Geo location code
        .short 0xffff                   # (0x16F) Get iSNS information
        .short 0xffff                   # (0x170) Set iSNS information
        .short 0xffff                   # (0x171) Set PR data
        .short 0xffff                   # (0x172) Get VDisk Redundancy
        .short 0xffff                   # (0x173) Get Async Replication data.
        .short 0xffff                   # (0x174) Get ISE Bay IP addresses.
        .short 0xffff                   # (0x175) All devices missing at other DCN
        .short 0xffff                   # (0x176) Get Extended vdisk info.
        .short  mpdiskqltimeoutssiz     # (0x177) Emulate BE Qlogic timeout on pdisk
        .short  mnisiz                  # (0x178) Get DLink Information -- GT2TB version
        .short  mrw_siz                 # (0x179) Read/Write Pdisk or Vdisk
        .short  s2psiz                  # (0x17A) Swap two PIDs
        .short  epabsiz                 # (0x17B) Emulate PAB
#
        .short  mivsiz                  # (0x400) Get VID info for FEP
        .short  msqsiz                  # (0x401) Set Sequence Number
        .short  mpsetconfigbesiz        # (0x402) setmpconfigbe
        .short  0xffff                  # (0x403) Update Persistent Reservation
#
# --- Ordered list of return lengths
#
d_expretlen:
        .short  mcrrsiz                 # (0x100) Create / expand vdisk
        .short  0xffff                  # (0x101) Get SES list
        .short  mldrsiz                 # (0x102) Label a physical
        .short  mfdrsiz                 # (0x103) Fail a device
        .short  msirsiz                 # (0x104) Raw SCSI IO
        .short  midrsiz                 # (0x105) Initialize a RAID
        .short  mfarsiz                 # (0x106) FCAL analysis
        .short  mdvrsiz                 # (0x107) Delete a virtual
        .short  mscrsiz                 # (0x108) Set caching mode
        .short  mprrsiz                 # (0x109) Set server properties
        .short  mxnrsiz                 # (0x10A) Reset NVRAM, clear
        .short  mrnrsiz                 # (0x10B) Restore from NVRAM
        .short  mpgrsiz                 # (0x10C) Awake
        .short  mwlrsiz                 # (0x10D) WWN / LUN lookup
        .short  0xffff                  # (0x10E) Generic
        .short  mssrsiz                 # (0x10F) Start or stop
        .short  medrsiz                 # (0x110) Enable/disable scrub
        .short  msdrsiz                 # (0x111) Set default label
        .short  0xffff                  # (0x112) Get Device Paths
        .short  mdrrsiz                 # (0x113) Restore device
        .short  mddrsiz                 # (0x114) Defragment device
        .short  maarsiz                 # (0x115) Set attribute
        .short  0xffff                  # (0x116) Get loop statistics
        .short  0xffff                  # (0x117) Get server list
        .short  0xffff                  # (0x118) Get virtual list
        .short  0xffff                  # (0x119) Get RAID list
        .short  0xffff                  # (0x11A) Get physical list
        .short  0xffff                  # (0x11B) Get miscellaneous list
        .short  0xffff                  # (0x11C) Get virtual info
        .short  0xffff                  # (0x11D) Get RAID info
        .short  0xffff                  # (0x11E) Get physical info
        .short  mmlrsiz                 # (0x11F) Map LUN to VDisk
        .short  mulrsiz                 # (0x120) Unmap LUN from VDisk
        .short  0xffff                  # (0x121) Get SES information
        .short  mmsrsiz                 # (0x122) Create server
        .short  mdsrsiz                 # (0x123) Delete server
        .short  0xffff                  # (0x124) Get miscellaneous info
        .short  mvcrsiz                 # (0x125) Virtual device control
        .short  masrsiz                 # (0x126) Set system information
        .short  mgirsiz                 # (0x127) Get II information
        .short  mfprsiz                 # (0x128) Get PCI information
        .short  mfhrsiz                 # (0x129) Get backend boot code header
        .short  mfhrsiz                 # (0x12A) Get backend diag code header
        .short  mfhrsiz                 # (0x12B) Get backend proc code header
        .short  mrrsiz                  # (0x12C) No op
        .short  mrwrsiz                 # (0x12D) Read/write memory
        .short  mctrsiz                 # (0x12E) Configure a target
        .short  0xffff                  # (0x12F) Get mirror partner list
        .short  mbprsiz                 # (0x130) Set global priority
        .short  0xffff                  # (0x131) Get target list
        .short  mrrsiz                  # (0x132) Reset BE chip
        .short  0xffff                  # (0x133) OBSOLETE
        .short  mncrsiz                 # (0x134) Get remote controller count
        .short  mcirsiz                 # (0x135) Get remote controller info
        .short  mvirsiz                 # (0x136) Get remote vdisk info
        .short  mftrsiz                 # (0x137) Set foreign targets
        .short  mcvrsiz                 # (0x138) Create a VLink
        .short  mvlrsiz                 # (0x139) Get VLink information
        .short  mmcrsiz                 # (0x13A) Create controller
        .short  mrdrsiz                 # (0x13B) Rescan physical devices
        .short  mrbrsiz                 # (0x13C) Resync RAID or stripes
        .short  mgnrsiz                 # (0x13D) Get local image of NVRAM
        .short  munrsiz                 # (0x13E) Put local image of NVRAM
        .short  mxdrsiz                 # (0x13F) Delete physical disk or SES
        .short  mmprsiz                 # (0x140) Mode page
        .short  mdcrsiz                 # (0x141) Get device count for serial number
        .short  0xffff                  # (0x142) Get Vdisk Owner
        .short  mrhrsiz                 # (0x143) Get hotspare information
        .short  mfcrsiz                 # (0x144) File system file copy
        .short  0xffff                  # (0x145) Get Device List
        .short  0xffff                  # (0x146) Get Port List
        .short  mbvrsiz                 # (0x147) Break VLock
        .short  0xffff                  # (0x148) Get SOS structure
        .short  mpsrsiz                 # (0x149) Put SOS structure
        .short  metrsiz                 # (0x14A) Force an error trap
        .short  mpcrsiz                 # (0x14B) Put SCMT structure
        .short  mlprsiz                 # (0x14C) Perform BE Loop Primitive
        .short  mtcrsiz                 # (0x14D) Target movement control
        .short  mrrsiz                  # (0x14E) Fail controller
        .short  mndrsiz                 # (0x14F) Name device
        .short  0xffff                  # (0x150) interprocessor DataGram
        .short  mrrsiz                  # (0x151) No op
        .short  0xffff                  # (0x152) Put FSys report
        .short  mnirsiz                 # (0x153) Get DLink Information
        .short  mkirsiz                 # (0x154) Get DLock Information
        .short  mrrsiz                  # (0x155) Degrade / restore port
        .short  0xffff                  # (0x156) Get workset information
        .short  msetwsrsiz              # (0x157) Set workset information
        .short  mrrsiz                  # (0x158) Set port config
        .short  0xffff                  # (0x159) Save async NV to file
        .short  mrchgraidnotmirroringrsiz # (0x15A) Modify RAID AStatus Field
        .short  mplrsiz                 # (0x15B) Put the LDD
        .short  mr5rsiz                 # (0x15C) Start recovery on an Inop R5
        .short  mrrsiz                  # (0x15D) Put device configuration
        .short  0xffff                  # (0x15e) Copy Data
        .short  mycrsiz                 # (0x15f) Copy Control
        .short  mrnrsiz                 # (0x160) Refresh from NVRAM
        .short  0xffff                  # (0x161) Set Vdisk Priority
        .short  0xffff                  # (0x162) Enable/Disable Vdisk Priority
        .short  0xffff                  # (0x163) Spindown physical disk
        .short  0xffff                  # (0x164) Failback physical disk
        .short  0xffff                  # (0x165) Auto Failback enable / disable
        .short  mcoptrsiz               # (0x166) Configure Options
        .short  0xffff                  # (0x167) Set Target Info
        .short  0xffff                  # (0x168) Get Target Info
        .short  0xffff                  # (0x169) Update iSCSI Server WWN
        .short  0xffff                  # (0x16A) Configure CHAP user
        .short  0xffff                  # (0x16B) CHAP user Info
        .short  0xffff                  # (0x16C) Get the Geo location code
        .short  0xffff                  # (0x16D) Set the Geo location code
        .short  0xffff                  # (0x16E) Clear the Geo location code
        .short  0xffff                  # (0x16F) Get iSNS information
        .short  0xffff                  # (0x170) Set iSNS information
        .short  0xffff                  # (0x171) Set PR data
        .short  0xffff                  # (0x172) Get VDisk Redundancy
        .short  0xffff                  # (0x173) Get Async Replication data.
        .short  0xffff                  # (0x174) Get ISE Bay IP addresses.
        .short  0xffff                  # (0x175) All devices missing at Other  DCN
        .short  0xffff                  # (0x176) Get Extended vdisk info.
        .short  mpdiskqltimeoutrsiz     # (0x177) Emulate BE Qlogic timeout on pdisk
        .short  mnirsizGT2TB            # (0x178) Get DLink Information -- GT2TB version
        .short  mrwrsiz                 # (0x179) Read/Write Pdisk or Vdisk
        .short  s2prsiz                 # (0x17A) Swap two PIDs
        .short  epabrsiz                # (0x17B) Emulate PAB
#
        .short  0xffff                  # (0x400) Get VID info for FEP
        .short  msqrsiz                 # (0x401) Set Sequence Number
# There is no change in the return packet size for 'save mirror partner' and 'setmpconfigbe'
        .short  msmprsiz                # (0x402) Save mirror partner
        .short  0xffff                  # (0x403) Update Persistent Reservation
#
# --- Ordered list of concurrency with I/O to drives
#
d_pktcon:
        .byte   TRUE                    # (0x100) Create / expand vdisk
        .byte   TRUE                    # (0x101) Get SES list
        .byte   FALSE                   # (0x102) Label a physical
        .byte   TRUE                    # (0x103) Fail a device
        .byte   TRUE                    # (0x104) Raw SCSI IO
        .byte   TRUE                    # (0x105) Initialize a RAID
        .byte   TRUE                    # (0x106) FCAL analysis
        .byte   FALSE                   # (0x107) Delete a virtual
        .byte   TRUE                    # (0x108) Set caching mode
        .byte   TRUE                    # (0x109) Set server properties
        .byte   FALSE                   # (0x10A) Reset NVRAM, clear
        .byte   FALSE                   # (0x10B) Restore from NVRAM
        .byte   TRUE                    # (0x10C) Awake
        .byte   TRUE                    # (0x10D) WWN / LUN lookup
        .byte   TRUE                    # (0x10E) Generic
        .byte   FALSE                   # (0x10F) Start or stop
        .byte   TRUE                    # (0x110) Enable/disable scrub
        .byte   TRUE                    # (0x111) Set default label
        .byte   TRUE                    # (0x112) Get Device Paths
        .byte   TRUE                    # (0x113) Restore device
        .byte   TRUE                    # (0x114) Defragment device
        .byte   TRUE                    # (0x115) Set attribute
        .byte   TRUE                    # (0x116) Get loop statistics
        .byte   TRUE                    # (0x117) Get server list
        .byte   TRUE                    # (0x118) Get virtual list
        .byte   TRUE                    # (0x119) Get RAID list
        .byte   TRUE                    # (0x11A) Get physical list
        .byte   TRUE                    # (0x11B) Get miscellaneous list
        .byte   TRUE                    # (0x11C) Get virtual info
        .byte   TRUE                    # (0x11D) Get RAID info
        .byte   TRUE                    # (0x11E) Get physical info
        .byte   TRUE                    # (0x11F) Map LUN to VDisk
        .byte   FALSE                   # (0x120) Unmap LUN from VDisk
        .byte   TRUE                    # (0x121) Get SES information
        .byte   TRUE                    # (0x122) Create server
        .byte   FALSE                   # (0x123) Delete server
        .byte   TRUE                    # (0x124) Get miscellaneous info
        .byte   TRUE                    # (0x125) Virtual device control
        .byte   TRUE                    # (0x126) Set system information
        .byte   TRUE                    # (0x127) Get II information
        .byte   TRUE                    # (0x128) Get PCI information
        .byte   TRUE                    # (0x129) Get backend boot code header
        .byte   TRUE                    # (0x12A) Get backend diag code header
        .byte   TRUE                    # (0x12B) Get backend proc code header
        .byte   TRUE                    # (0x12C) No op
        .byte   TRUE                    # (0x12D) Read/write memory
        .byte   TRUE                    # (0x12E) Configure a target
        .byte   TRUE                    # (0x12F) Get mirror partner list
        .byte   FALSE                   # (0x130) Set global priority
        .byte   TRUE                    # (0x131) Get target list
        .byte   FALSE                   # (0x132) Reset BE chip
        .byte   TRUE                    # (0x133) OBSOLETE
        .byte   TRUE                    # (0x134) Get remote controller count
        .byte   TRUE                    # (0x135) Get remote controller info
        .byte   TRUE                    # (0x136) Get remote vdisk info
        .byte   FALSE                   # (0x137) Set foreign targets
        .byte   TRUE                    # (0x138) Create a VLink
        .byte   TRUE                    # (0x139) Get VLink information
        .byte   TRUE                    # (0x13A) Create controller
        .byte   TRUE                    # (0x13B) Rescan physical devices
        .byte   FALSE                   # (0x13C) Resync RAID or stripes
        .byte   TRUE                    # (0x13D) Get local image of NVRAM
        .byte   TRUE                    # (0x13E) Put local image of NVRAM
        .byte   FALSE                   # (0x13F) Delete physical disk or SES
        .byte   TRUE                    # (0x140) Mode page
        .byte   TRUE                    # (0x141) Get device count for serial number
        .byte   TRUE                    # (0x142) Get Vdisk Owner
        .byte   TRUE                    # (0x143) Get hotspare information
        .byte   TRUE                    # (0x144) File system file copy
        .byte   TRUE                    # (0x145) Get Device List
        .byte   TRUE                    # (0x146) Get Port List
        .byte   FALSE                   # (0x147) Break VLock
        .byte   TRUE                    # (0x148) Get SOS structure
        .byte   TRUE                    # (0x149) Put SOS structure
        .byte   TRUE                    # (0x14A) Force an error trap
        .byte   FALSE                   # (0x14B) Put SCMT structure
        .byte   TRUE                    # (0x14C) Perform BE Loop Primitive
        .byte   TRUE                    # (0x14D) Target movement control
        .byte   FALSE                   # (0x14E) Fail controller
        .byte   TRUE                    # (0x14F) Name device
        .byte   TRUE                    # (0x150) interprocessor DataGram
        .byte   TRUE                    # (0x151) No op
        .byte   TRUE                    # (0x152) Put FSys report
        .byte   TRUE                    # (0x153) Get DLink Information
        .byte   TRUE                    # (0x154) Get DLock Information
        .byte   TRUE                    # (0x155) Degrade / restore port
        .byte   TRUE                    # (0x156) Get workset information
        .byte   TRUE                    # (0x157) Set workset information
        .byte   TRUE                    # (0x158) Set port config
        .byte   TRUE                    # (0x159) Save async NV to file
        .byte   TRUE                    # (0x15A) Modify RAID AStatus Field
        .byte   TRUE                    # (0x15B) Put the LDD
        .byte   TRUE                    # (0x15C) Start recovery on an Inop R5
        .byte   TRUE                    # (0x15D) Put device configuration
        .byte   TRUE                    # (0x15e) Copy Data
        .byte   TRUE                    # (0x15f) Copy Control
        .byte   TRUE                    # (0x160) Refresh from NVRAM
        .byte   TRUE                    # (0x161) Set Vdisk Priority
        .byte   TRUE                    # (0x162) Enable/Disable Vdisk Priority
        .byte   TRUE                    # (0x163) Spindown physical disk
        .byte   TRUE                    # (0x164) Failback physical disk
        .byte   TRUE                    # (0x165) Auto Failback enable / disable
        .byte   TRUE                    # (0x166) Configure Options
        .byte   TRUE                    # (0x167) Set Target Info
        .byte   TRUE                    # (0x168) Get Target Info
        .byte   TRUE                    # (0x169) Update iSCSI Server WWN
        .byte   TRUE                    # (0x16A) Configure CHAP user
        .byte   TRUE                    # (0x16B) CHAP user Info
        .byte   TRUE                    # (0x16C) Get the Geo location code
        .byte   TRUE                    # (0x16D) Set the Geo location code
        .byte   TRUE                    # (0x16E) Clear the Geo location code
        .byte   TRUE                    # (0x16F) Get iSNS information
        .byte   TRUE                    # (0x170) Set iSNS information
        .byte   TRUE                    # (0x171) Set PR data
        .byte   TRUE                    # (0x172) Get VDisk Redundancy
        .byte   TRUE                    # (0x173) Get Async Replication data
        .byte   TRUE                    # (0x174) Get ISE Bay IP addresses
        .byte   TRUE                    # (0x175) All devices missing at other DCN
        .byte   TRUE                    # (0x176) Get Vdisk extended info.
        .byte   TRUE                    # (0x177) Emulate BE Qlogic timeout on pdisk
        .byte   TRUE                    # (0x178) Get DLink Information -- GT2TB version
        .byte   TRUE                    # (0x179) Read/Write Pdisk or Vdisk
        .byte   TRUE                    # (0x17A) Swap two PIDs
        .byte   TRUE                    # (0x17B) Emulate PAB
#
        .byte   TRUE                    # (0x400) Get VID info for FEP
        .byte   TRUE                    # (0x401) Set Sequence Number
        .byte   TRUE                    # (0x402) set mpconfigbe
        .byte   TRUE                    # (0x403) Update Persistent Reservation
#
# --- Ordered list of scrub concurrency
#
d_scrubcon:
        .byte   TRUE                    # (0x100) Create / expand vdisk
        .byte   TRUE                    # (0x101) Get SES list
        .byte   TRUE                    # (0x102) Label a physical
        .byte   TRUE                    # (0x103) Fail a device
        .byte   TRUE                    # (0x104) Raw SCSI IO
        .byte   TRUE                    # (0x105) Initialize a RAID
        .byte   TRUE                    # (0x106) FCAL analysis
        .byte   FALSE                   # (0x107) Delete a virtual
        .byte   TRUE                    # (0x108) Set caching mode
        .byte   TRUE                    # (0x109) Set server properties
        .byte   FALSE                   # (0x10A) Reset NVRAM, clear
        .byte   FALSE                   # (0x10B) Restore from NVRAM
        .byte   TRUE                    # (0x10C) Awake
        .byte   TRUE                    # (0x10D) WWN / LUN lookup
        .byte   TRUE                    # (0x10E) Generic
        .byte   TRUE                    # (0x10F) Start or stop
        .byte   TRUE                    # (0x110) Enable/disable scrub
        .byte   TRUE                    # (0x111) Set default label
        .byte   TRUE                    # (0x112) Get Device Paths
        .byte   TRUE                    # (0x113) Restore device
        .byte   FALSE                   # (0x114) Defragment device
        .byte   TRUE                    # (0x115) Set attribute
        .byte   TRUE                    # (0x116) Get loop statistics
        .byte   TRUE                    # (0x117) Get server list
        .byte   TRUE                    # (0x118) Get virtual list
        .byte   TRUE                    # (0x119) Get RAID list
        .byte   TRUE                    # (0x11A) Get physical list
        .byte   TRUE                    # (0x11B) Get miscellaneous list
        .byte   TRUE                    # (0x11C) Get virtual info
        .byte   TRUE                    # (0x11D) Get RAID info
        .byte   TRUE                    # (0x11E) Get physical info
        .byte   TRUE                    # (0x11F) Map LUN to VDisk
        .byte   TRUE                    # (0x120) Unmap LUN from VDisk
        .byte   TRUE                    # (0x121) Get SES information
        .byte   TRUE                    # (0x122) Create server
        .byte   TRUE                    # (0x123) Delete server
        .byte   TRUE                    # (0x124) Get miscellaneous info
        .byte   TRUE                    # (0x125) Virtual device control
        .byte   TRUE                    # (0x126) Set system information
        .byte   TRUE                    # (0x127) Get II information
        .byte   TRUE                    # (0x128) Get PCI information
        .byte   TRUE                    # (0x129) Get backend boot code header
        .byte   TRUE                    # (0x12A) Get backend diag code header
        .byte   TRUE                    # (0x12B) Get backend proc code header
        .byte   TRUE                    # (0x12C) No op
        .byte   TRUE                    # (0x12D) Read/write memory
        .byte   TRUE                    # (0x12E) Configure a target
        .byte   TRUE                    # (0x12F) Get mirror partner list
        .byte   FALSE                   # (0x130) Set global priority
        .byte   TRUE                    # (0x131) Get target list
        .byte   FALSE                   # (0x132) Reset BE chip
        .byte   TRUE                    # (0x133) OBSOLETE
        .byte   TRUE                    # (0x134) Get remote controller count
        .byte   TRUE                    # (0x135) Get remote controller info
        .byte   TRUE                    # (0x136) Get remote vdisk info
        .byte   TRUE                    # (0x137) Set foreign targets
        .byte   TRUE                    # (0x138) Create a VLink
        .byte   TRUE                    # (0x139) Get VLink information
        .byte   TRUE                    # (0x13A) Create controller
        .byte   TRUE                    # (0x13B) Rescan physical devices
        .byte   FALSE                   # (0x13C) Resync RAID or stripes
        .byte   TRUE                    # (0x13D) Get local image of NVRAM
        .byte   TRUE                    # (0x13E) Put local image of NVRAM
        .byte   TRUE                    # (0x13F) Delete physical disk or SES
        .byte   TRUE                    # (0x140) Mode page
        .byte   TRUE                    # (0x141) Get device count for serial number
        .byte   TRUE                    # (0x142) Get Vdisk Owner
        .byte   TRUE                    # (0x143) Get hotspare information
        .byte   TRUE                    # (0x144) File system file copy
        .byte   TRUE                    # (0x145) Get Device List
        .byte   TRUE                    # (0x146) Get Port List
        .byte   FALSE                   # (0x147) Break VLock
        .byte   TRUE                    # (0x148) Get SOS structure
        .byte   TRUE                    # (0x149) Put SOS structure
        .byte   TRUE                    # (0x14A) Force an error trap
        .byte   TRUE                    # (0x14B) Put SCMT structure
        .byte   TRUE                    # (0x14C) Perform BE Loop Primitive
        .byte   FALSE                   # (0x14D) Target movement control
        .byte   FALSE                   # (0x14E) Fail controller
        .byte   TRUE                    # (0x14F) Name device
        .byte   FALSE                   # (0x150) interprocessor DataGram
        .byte   TRUE                    # (0x151) No op
        .byte   TRUE                    # (0x152) Put FSys report
        .byte   TRUE                    # (0x153) Get DLink Information
        .byte   TRUE                    # (0x154) Get DLock Information
        .byte   TRUE                    # (0x155) Degrade / restore port
        .byte   TRUE                    # (0x156) Get workset information
        .byte   TRUE                    # (0x157) Set workset information
        .byte   TRUE                    # (0x158) Set port config
        .byte   TRUE                    # (0x159) Save async NV to file
        .byte   TRUE                    # (0x15A) Modify RAID AStatus Field
        .byte   TRUE                    # (0x15B) Put the LDD
        .byte   TRUE                    # (0x15C) Start recovery on an Inop R5
        .byte   TRUE                    # (0x15D) Put device configuration
        .byte   TRUE                    # (0x15e) Copy Data
        .byte   TRUE                    # (0x15f) Copy Control
        .byte   TRUE                    # (0x160) Refresh from NVRAM
        .byte   TRUE                    # (0x161) Set Vdisk Priority
        .byte   TRUE                    # (0x162) Enable/Disable Vdisk Priority
        .byte   TRUE                    # (0x163) Spindown physical disk
        .byte   TRUE                    # (0x164) Failback physical disk
        .byte   TRUE                    # (0x165) Auto Failback enable / disable
        .byte   TRUE                    # (0x166) Configure Options
        .byte   TRUE                    # (0x167) Set Target Info
        .byte   TRUE                    # (0x168) Get Target Info
        .byte   TRUE                    # (0x169) Update iSCSI Server WWN
        .byte   TRUE                    # (0x16A) Configure CHAP user
        .byte   TRUE                    # (0x16B) CHAP user Info
        .byte   TRUE                    # (0x16C) Get the Geo Location code
        .byte   TRUE                    # (0x16D) Set the Geo Location code
        .byte   TRUE                    # (0x16E) Clear the Geo Location code
        .byte   TRUE                    # (0x16F) Get iSNS information
        .byte   TRUE                    # (0x170) Set iSNS information
        .byte   TRUE                    # (0x171) Set PR data
        .byte   TRUE                    # (0x172) Get VDisk Redundancy
        .byte   TRUE                    # (0x173) Get Async Replication data.
        .byte   TRUE                    # (0x174) Get ISE Bay IP addresses.
        .byte   FALSE                   # (0x175) All devices missing at other DCN
        .byte   TRUE                    # (0x176) Get Extended vdisk info.
        .byte   TRUE                    # (0x177) Emulate BE Qlogic timeout on pdisk
        .byte   TRUE                    # (0x178) Get DLink Information -- GT2TB version
        .byte   TRUE                    # (0x179) Read/Write Pdisk or Vdisk
        .byte   TRUE                    # (0x17A) Swap two PIDs
        .byte   TRUE                    # (0x17B) Emulate PAB
#
        .byte   TRUE                    # (0x400) Get VID info for FEP
        .byte   TRUE                    # (0x401) Set Sequence Number
        .byte   FALSE                   # (0x402) set mpconfigbe
        .byte   TRUE                    # (0x403) Update Persistent Reservation
#
# --- Ordered list of NVRAM expansion
#
d_pktexp:
        .byte   TRUE                    # (0x100) Create / expand vdisk
        .byte   FALSE                   # (0x101) Get SES list
        .byte   FALSE                   # (0x102) Label a physical
        .byte   FALSE                   # (0x103) Fail a device
        .byte   FALSE                   # (0x104) Raw SCSI IO
        .byte   FALSE                   # (0x105) Initialize a RAID
        .byte   FALSE                   # (0x106) FCAL analysis
        .byte   FALSE                   # (0x107) Delete a virtual
        .byte   FALSE                   # (0x108) Set caching mode
        .byte   FALSE                   # (0x109) Set server properties
        .byte   FALSE                   # (0x10A) Reset NVRAM, clear
        .byte   FALSE                   # (0x10B) Restore from NVRAM
        .byte   FALSE                   # (0x10C) Awake
        .byte   FALSE                   # (0x10D) WWN / LUN lookup
        .byte   FALSE                   # (0x10E) Generic
        .byte   FALSE                   # (0x10F) Start or stop
        .byte   FALSE                   # (0x110) Enable/disable scrub
        .byte   FALSE                   # (0x111) Set default label
        .byte   FALSE                   # (0x112) Get Device Paths
        .byte   FALSE                   # (0x113) Restore device
        .byte   FALSE                   # (0x114) Defragment device
        .byte   FALSE                   # (0x115) Set attribute
        .byte   FALSE                   # (0x116) Get loop statistics
        .byte   FALSE                   # (0x117) Get server list
        .byte   FALSE                   # (0x118) Get virtual list
        .byte   FALSE                   # (0x119) Get RAID list
        .byte   FALSE                   # (0x11A) Get physical list
        .byte   FALSE                   # (0x11B) Get miscellaneous list
        .byte   FALSE                   # (0x11C) Get virtual info
        .byte   FALSE                   # (0x11D) Get RAID info
        .byte   FALSE                   # (0x11E) Get physical info
        .byte   TRUE                    # (0x11F) Map LUN to VDisk
        .byte   FALSE                   # (0x120) Unmap LUN from VDisk
        .byte   FALSE                   # (0x121) Get SES information
        .byte   TRUE                    # (0x122) Create server
        .byte   FALSE                   # (0x123) Delete server
        .byte   FALSE                   # (0x124) Get miscellaneous info
        .byte   TRUE                    # (0x125) Virtual device control
        .byte   FALSE                   # (0x126) Set system information
        .byte   FALSE                   # (0x127) Get II information
        .byte   FALSE                   # (0x128) Get PCI information
        .byte   FALSE                   # (0x129) Get backend boot code header
        .byte   FALSE                   # (0x12A) Get backend diag code header
        .byte   FALSE                   # (0x12B) Get backend proc code header
        .byte   FALSE                   # (0x12C) No op
        .byte   FALSE                   # (0x12D) Read/write memory
        .byte   TRUE                    # (0x12E) Configure a target
        .byte   FALSE                   # (0x12F) Get mirror partner list
        .byte   FALSE                   # (0x130) Set global priority
        .byte   FALSE                   # (0x131) Get target list
        .byte   FALSE                   # (0x132) Reset BE chip
        .byte   FALSE                   # (0x133) OBSOLETE
        .byte   FALSE                   # (0x134) Get remote controller count
        .byte   FALSE                   # (0x135) Get remote controller info
        .byte   FALSE                   # (0x136) Get remote vdisk info
        .byte   FALSE                   # (0x137) Set foreign targets
        .byte   TRUE                    # (0x138) Create a VLink
        .byte   FALSE                   # (0x139) Get VLink information
        .byte   TRUE                    # (0x13A) Create controller
        .byte   FALSE                   # (0x13B) Rescan physical devices
        .byte   FALSE                   # (0x13C) Resync RAID or stripes
        .byte   FALSE                   # (0x13D) Get local image of NVRAM
        .byte   FALSE                   # (0x13E) Put local image of NVRAM
        .byte   FALSE                   # (0x13F) Delete physical disk or SES
        .byte   FALSE                   # (0x140) Mode page
        .byte   FALSE                   # (0x141) Get device count for serial number
        .byte   FALSE                   # (0x142) Get Vdisk Owner
        .byte   FALSE                   # (0x143) Get hotspare information
        .byte   FALSE                   # (0x144) File system file copy
        .byte   FALSE                   # (0x145) Get Device List
        .byte   FALSE                   # (0x146) Get Port List
        .byte   FALSE                   # (0x147) Break VLock
        .byte   FALSE                   # (0x148) Get SOS structure
        .byte   FALSE                   # (0x149) Put SOS structure
        .byte   FALSE                   # (0x14A) Force an error trap
        .byte   FALSE                   # (0x14B) Put SCMT structure
        .byte   FALSE                   # (0x14C) Perform BE Loop Primitive
        .byte   FALSE                   # (0x14D) Target movement control
        .byte   FALSE                   # (0x14E) Fail controller
        .byte   TRUE                    # (0x14F) Name device
        .byte   FALSE                   # (0x150) interprocessor DataGram
        .byte   FALSE                   # (0x151) No op
        .byte   FALSE                   # (0x152) Put FSys report
        .byte   FALSE                   # (0x153) Get DLink Information
        .byte   FALSE                   # (0x154) Get DLock Information
        .byte   FALSE                   # (0x155) Degrade / restore port
        .byte   FALSE                   # (0x156) Get workset information
        .byte   FALSE                   # (0x157) Set workset information
        .byte   FALSE                   # (0x158) Set port config
        .byte   FALSE                   # (0x159) Save async NV to file
        .byte   FALSE                   # (0x15A) Modify RAID AStatus Field
        .byte   FALSE                   # (0x15B) Put the LDD
        .byte   FALSE                   # (0x15C) Start recovery on an Inop R5
        .byte   FALSE                   # (0x15D) Put device configuration
        .byte   FALSE                   # (0x15e) Copy Data
        .byte   FALSE                   # (0x15f) Copy Control
        .byte   FALSE                   # (0x160) Refresh from NVRAM
        .byte   FALSE                   # (0x161) Set Vdisk Priority
        .byte   FALSE                   # (0x162) Enable/Disable Vdisk Priority
        .byte   FALSE                   # (0x163) Spindown physical disk
        .byte   FALSE                   # (0x164) Failback physical disk
        .byte   FALSE                   # (0x165) Auto Failback enable / disable
        .byte   FALSE                   # (0x166) Configure Options
        .byte   FALSE                   # (0x167) Set Target Info
        .byte   FALSE                   # (0x168) Get Target Info
        .byte   FALSE                   # (0x169) Update iSCSI Server WWN
        .byte   FALSE                   # (0x16A) Configure CHAP user
        .byte   TRUE                    # (0x16B) CHAP user Info
        .byte   FALSE                   # (0x16C) Get the Geo Location code
        .byte   FALSE                   # (0x16D) Set the Geo Location code
        .byte   FALSE                   # (0x16E) Clear the Geo Location code
        .byte   FALSE                   # (0x16F) Get iSNS information
        .byte   FALSE                   # (0x170) Set iSNS information
        .byte   FALSE                   # (0x171) Set PR data
        .byte   FALSE                   # (0x172) Get VDisk Redundancy
        .byte   FALSE                   # (0x173) Get Async Replication data
        .byte   FALSE                   # (0x174) Get ISE Bay IP addresses
        .byte   FALSE                   # (0x175) All devices missing MRP
        .byte   FALSE                   # (0x176) Get Extended vdisk info
        .byte   FALSE                   # (0x177) Emulate BE Qlogic timeout on pdisk
        .byte   FALSE                   # (0x178) Get DLink Information -- GT2TB version
        .byte   FALSE                   # (0x179) Read/Write Pdisk or Vdisk
        .byte   FALSE                   # (0x17A) Swap two PIDs
        .byte   FALSE                   # (0x17B) Emulate PAB
#
        .byte   FALSE                   # (0x400) Get VID info for FEP
        .byte   FALSE                   # (0x401) Set Sequence Number
        .byte   FALSE                   # (0x402) Save mirror partner
        .byte   FALSE                   # (0x403) Update Persistent Reservation
#
# --- Ordered list of permission to run under partial define
#
d_defperm:
        .byte   FALSE                   # (0x100) Create / expand vdisk
        .byte   TRUE                    # (0x101) Get SES list
        .byte   FALSE                   # (0x102) Label a physical
        .byte   FALSE                   # (0x103) Fail a device
        .byte   TRUE                    # (0x104) Raw SCSI IO
        .byte   FALSE                   # (0x105) Initialize a RAID
        .byte   TRUE                    # (0x106) FCAL analysis
        .byte   FALSE                   # (0x107) Delete a virtual
        .byte   FALSE                   # (0x108) Set caching mode
        .byte   FALSE                   # (0x109) Set server properties
        .byte   TRUE                    # (0x10A) Reset NVRAM, clear
        .byte   TRUE                    # (0x10B) Restore from NVRAM
        .byte   TRUE                    # (0x10C) Awake
        .byte   FALSE                   # (0x10D) WWN / LUN lookup
        .byte   TRUE                    # (0x10E) Generic
        .byte   FALSE                   # (0x10F) Start or stop
        .byte   FALSE                   # (0x110) Enable/disable scrub
        .byte   FALSE                   # (0x111) Set default label
        .byte   FALSE                   # (0x112) Get Device Paths
        .byte   FALSE                   # (0x113) Restore device
        .byte   FALSE                   # (0x114) Defragment device
        .byte   FALSE                   # (0x115) Set attribute
        .byte   TRUE                    # (0x116) Get loop statistics
        .byte   TRUE                    # (0x117) Get server list
        .byte   TRUE                    # (0x118) Get virtual list
        .byte   TRUE                    # (0x119) Get RAID list
        .byte   TRUE                    # (0x11A) Get physical list
        .byte   TRUE                    # (0x11B) Get miscellaneous list
        .byte   FALSE                   # (0x11C) Get virtual info
        .byte   FALSE                   # (0x11D) Get RAID info
        .byte   TRUE                    # (0x11E) Get physical info
        .byte   FALSE                   # (0x11F) Map LUN to VDisk
        .byte   FALSE                   # (0x120) Unmap LUN from VDisk
        .byte   TRUE                    # (0x121) Get SES information
        .byte   FALSE                   # (0x122) Create server
        .byte   FALSE                   # (0x123) Delete server
        .byte   FALSE                   # (0x124) Get miscellaneous info
        .byte   FALSE                   # (0x125) Virtual device control
        .byte   TRUE                    # (0x126) Set system information
        .byte   TRUE                    # (0x127) Get II information
        .byte   TRUE                    # (0x128) Get PCI information
        .byte   TRUE                    # (0x129) Get backend boot code header
        .byte   TRUE                    # (0x12A) Get backend diag code header
        .byte   TRUE                    # (0x12B) Get backend proc code header
        .byte   TRUE                    # (0x12C) No op
        .byte   TRUE                    # (0x12D) Read/write memory
        .byte   FALSE                   # (0x12E) Configure a target
        .byte   FALSE                   # (0x12F) Get mirror partner list
        .byte   FALSE                   # (0x130) Set global priority
        .byte   TRUE                    # (0x131) Get target list
        .byte   TRUE                    # (0x132) Reset BE chip
        .byte   FALSE                   # (0x133) OBSOLETE
        .byte   FALSE                   # (0x134) Get remote controller count
        .byte   FALSE                   # (0x135) Get remote controller info
        .byte   FALSE                   # (0x136) Get remote vdisk info
        .byte   FALSE                   # (0x137) Set foreign targets
        .byte   FALSE                   # (0x138) Create a VLink
        .byte   FALSE                   # (0x139) Get VLink information
        .byte   TRUE                    # (0x13A) Create controller
        .byte   FALSE                   # (0x13B) Rescan physical devices
        .byte   FALSE                   # (0x13C) Resync RAID or stripes
        .byte   TRUE                    # (0x13D) Get local image of NVRAM
        .byte   FALSE                   # (0x13E) Put local image of NVRAM
        .byte   FALSE                   # (0x13F) Delete physical disk or SES
        .byte   TRUE                    # (0x140) Mode page
        .byte   TRUE                    # (0x141) Get device count for serial number
        .byte   FALSE                   # (0x142) Get Vdisk Owner
        .byte   FALSE                   # (0x143) Get hotspare information
        .byte   TRUE                    # (0x144) File System file copy
        .byte   TRUE                    # (0x145) Get Device List
        .byte   TRUE                    # (0x146) Get Port List
        .byte   FALSE                   # (0x147) Break VLock
        .byte   FALSE                   # (0x148) Get SOS structure
        .byte   FALSE                   # (0x149) Put SOS structure
        .byte   TRUE                    # (0x14A) Force an error trap
        .byte   FALSE                   # (0x14B) Put SCMT structure
        .byte   TRUE                    # (0x14C) Perform BE Loop Primitive
        .byte   FALSE                   # (0x14D) Target movement control
        .byte   FALSE                   # (0x14E) Fail controller
        .byte   TRUE                    # (0x14F) Name device
        .byte   FALSE                   # (0x150) interprocessor DataGram
        .byte   TRUE                    # (0x151) No op
        .byte   TRUE                    # (0x152) Put FSys report
        .byte   TRUE                    # (0x153) Get DLink Information
        .byte   TRUE                    # (0x154) Get DLock Information
        .byte   TRUE                    # (0x155) Degrade / restore port
        .byte   TRUE                    # (0x156) Get workset information
        .byte   FALSE                   # (0x157) Set workset information
        .byte   TRUE                    # (0x158) Set port config
        .byte   TRUE                    # (0x159) Save async NV to file
        .byte   FALSE                   # (0x15A) Modify RAID AStatus Field
        .byte   FALSE                   # (0x15B) Put the LDD
        .byte   FALSE                   # (0x15C) Start recovery on an Inop R5
        .byte   TRUE                    # (0x15D) Put device configuration
        .byte   FALSE                   # (0x15e) Copy Data
        .byte   FALSE                   # (0x15f) Copy Control
        .byte   TRUE                    # (0x160) Refresh from NVRAM
        .byte   FALSE                   # (0x161) Set Vdisk Priority
        .byte   FALSE                   # (0x162) Enable/Disable Vdisk Priority
        .byte   FALSE                   # (0x163) Spindown physical disk
        .byte   FALSE                   # (0x164) Failback physical disk
        .byte   FALSE                   # (0x165) Auto Failback enable / disable
        .byte   TRUE                    # (0x166) Configure Options
        .byte   FALSE                   # (0x167) Set Target Info
        .byte   FALSE                   # (0x168) Get Target Info
        .byte   FALSE                   # (0x169) Update iSCSI Server WWN
        .byte   FALSE                   # (0x16A) Configure CHAP user
        .byte   TRUE                    # (0x16B) CHAP user Info
        .byte   FALSE                   # (0x16C) Get the Geo Location code
        .byte   FALSE                   # (0x16D) Set the Geo Location code
        .byte   FALSE                   # (0x16E) Clear the Geo Location code
        .byte   FALSE                   # (0x16F) Get iSNS information
        .byte   FALSE                   # (0x170) Set iSNS information
        .byte   TRUE                    # (0x171) Set PR data
        .byte   TRUE                    # (0x172) Get VDisk Redundancy
        .byte   TRUE                    # (0x173) Get Async data
        .byte   TRUE                    # (0x174) Get ISE Bay IP addresses
        .byte   FALSE                   # (0x175) All devices missing at other DCN
        .byte   FALSE                   # (0x176) Get Extended vdisk info.
        .byte   TRUE                    # (0x177) Emulate BE Qlogic timeout on pdisk
        .byte   TRUE                    # (0x178) Get DLink Information -- GT2TB version
        .byte   TRUE                    # (0x179) Read/Write Pdisk or Vdisk
        .byte   TRUE                    # (0x17A) Swap two PIDs
        .byte   TRUE                    # (0x17B) Emulate PAB
#
        .byte   TRUE                    # (0x400) Get VID info for FEP
        .byte   TRUE                    # (0x401) Set Sequence Number
        .byte   TRUE                    # (0x402) Save mirror partner
        .byte   TRUE                    # (0x403) Update Persistent Reservation
#
# --- executable code -------------------------------------------------
#
        .text
#**********************************************************************
#
#  NAME: D$init
#
#  PURPOSE:
#       To provide a means of initializing this module.
#
#  DESCRIPTION:
#       The executive processes used by this module are established and
#       made ready for execution.
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
#       g2
#       g3
#
#**********************************************************************
#
D$init:
#
# --- Establish executive process
#
        lda     d$exec,g0               # Establish executive process
        ldconst DEXECPRI,g1
c       CT_fork_tmp = (ulong)"d$exec";
        call    K$fork
        st      g0,d_exec_qu+qu_pcb     # Save PCB
#
# --- Initialize Debug Data Retrieval (DDR) d_exec_qu PCB entry
#
c       M_addDDRentry(de_dpcb, g0, pcbsiz);
#
# --- Initialize Debug Data Retrieval (DDR) d_exec_qu entry
#
        lda     d_exec_qu,g1            # Load address of d_exec_qu header
c       M_addDDRentry(de_deque, g1, 16);    # Size of d_exe_qu header
#
# --- Establish executive process for file system.
#
        lda     FS$exec,g0              # Establish executive process
        ldconst FEXECPRI,g1
c       CT_fork_tmp = (ulong)"FS$exec";
        call    K$fork
        st      g0,f_exec_qu+qu_pcb     # Save PCB
#
# --- Initialize Debug Data Retrieval (DDR) f_exec_qu PCB entry
#
c       M_addDDRentry(de_fspcb, g0, pcbsiz);
#
# --- Initialize Debug Data Retrieval (DDR) f_exec_qu entry
#
        lda     f_exec_qu,g1            # Load address of f_exec_qu header
c       M_addDDRentry(de_fseque, g1, 16);   # Size of f_exec_qu header
#
# --- Establish raid initialization scheduling process
#
        lda     D$sched_rip,g0          # Establish rip scheduling process
        lda     DSCHEDIRAIDPRIO,g1
c       CT_fork_tmp = (ulong)"D$sched_rip";
        call    K$fork
        st      g0,d_rip_exec_qu+qu_pcb # Save PCB
#
# --- Initialize Debug Data Retrieval (DDR) Raid init PCB table entry
#
c       M_addDDRentry(de_rippcb, g0, pcbsiz);
#
# --- Initialize Debug Data Retrieval (DDR) Raid init table entry
#
        lda     d_rip_exec_qu,g1        # Load address to RIP exec qu
c       M_addDDRentry(de_ripeque, g1, 16);  # Length of RIP exec qu
#
# --- Initialize the MRP trace Debug Data Retrieval (DDR) table entry
#
        lda     defTraceQue,g1          # Load address to MRP trace
c       M_addDDRentry(de_mrp, g1, 16400);  # Length of MRP trace
#
# --- Initialize the Defrag Trace DDR table entry
#
        lda     gDFDebug,g1             # Load address to defrag trace
c       M_addDDRentry(de_defrag, g1, 128*16);  # Length of defrag trace
#
# --- Exit
#
        ret
#
#**********************************************************************
#
#  NAME: d$exec
#
#  PURPOSE:
#       To provide a means of processing MRP requests which have been
#       previously queued to this module.
#
#  DESCRIPTION:
#       The queuing routine D$que deposits a MRP request into the queue
#       and activates this executive if necessary. This executive
#       extracts the next MRP request from the queue and initiates that
#       request.
#
#       The functions called all are provided with the pointer to the MRP.
#
#       The functions are expected to return a length for data returned
#       and the status of the operation.
#
#       The following registers are used for the purposes described above.
#           g0 - MRP pointer
#           g1 - return status from function
#           g2 - return length
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
d$exec:
        b       .dex20                  # Exchange to start off
#
# --- Set this process to not ready
#
.dex10:
        ldconst pcnrdy,r4               # Set this process to not ready
        stob    r4,pc_stat(r15)
        call    K$qxchang               # Exchange processes
#
# --- Get next queued request
#
.dex20:
        lda     d_exec_qu,r11           # Get executive queue pointer
        ldq     qu_head(r11),r12        # Get queue head, tail, count and PCB
        mov     r12,r10                 # Isolate next queued ILT
        cmpobe  0,r12,.dex10            # Jif none
#
# --- Remove this request from queue ----------------------------------
#
        ld      il_fthd(r12),r12        # Dequeue this ILT
        cmpo    0,r12                   # Check for queue now empty
        subo    1,r14,r14               # Adjust queue count
        sele    r13,r12,r13             # Set up queue tail
        stt     r12,qu_head(r11)        # Update queue head, tail and count
        be      .dex30                  # Jif queue now empty
#
        st      r11,il_bthd(r12)        # Update backward thread
#
.dex30:
#
# --- Prep return packet size and error code
#
# --- Register usage
#
#     g1/g2 - return values from the called function.
#     r11 - holding register for the request pointer (MRP pointer)
#     r10 - holding register for the ILT pointer
#     r9  - opcode (MRP function code)
#
        ldconst deinvpkttyp,g1          # Set invalid function type
        ldconst mrrsiz,g2               # Return packet size
        ldconst TRUE,r5                 # Preload for error exit
        ldconst TRUE,r6                 # Preload for error exit
#
# --- Determine type of request
#
        ld      il_w0-ILTBIAS(r10),r11  # Get request ptr
        ldos    mr_func(r11),r9         # Get request function type
        ldconst mrebfuncmax,r3          # Get the max opcode
        cmpoble r9,r3,.dex40            # If less than, check low end
#
        ldconst mrfbfuncmax,r3          # Check for FEP to BEP commands
        cmpobg  r9,r3,.dex85            # Jif out of range
        ldconst mrfbfuncbase,r3         # Check for low end
#
        ldconst mrnopfsys,r7
        cmpobne r7,r9,.dex39
c fprintf(stderr,"%s%s:%u <d$exec-MRNOPFSYS>r7 = %lx r9 = %lx r3 = %lx g1 = %lx\n", FEBEMESSAGE, __FILE__, __LINE__,r7,r9,r3,g1);
.dex39:
        cmpobl  r9,r3,.dex85            # Jif out of range
#
# --- Good command, now adjust so that we look into the tables immediately
# --- following the CCB to BE opcode range.
#
        subo    r3,r9,r9                # Get zero relative to this op range
        ldconst mrebfuncmax-mrebfuncbase+1,r3
        addo    r3,r9,r3                # We are now at the end of other range
        b       .dex50
#
.dex40:
        ldconst mrebfuncbase,r3         # Get the base opcode
        cmpobl  r9,r3,.dex85            # If less than the min, illegal
        subo    r3,r9,r3                # Get offset into the tables
#
# --- Validate the length of the packet and the return length and whether
# --- or not the command is allowed at this time.
#
.dex50:
        ldos    K_ii+ii_status,r4       # Get initialization status
        bbs     iifulldef,r4,.dex51     # Jif full define running
#
        ldob    d_defperm[r3*1],r4      # Get the permission
        ldconst dedefnrdy,g1            # Prep define not ready error
        cmpobne TRUE,r4,.dex85          # Error out if cmd not allowed
#
.dex51:
        ldos    d_exppktlen[r3*2],r4    # Get the expected length
        ldconst 0xffff,r7               # Max length indicator
        cmpobe  r7,r4,.dex55            # If max, called function will validate
#
        ldconst deinvpktsiz,g1          # Prep possible error code
        ld      mr_len(r11),r7          # Get the length
        cmpobne r4,r7,.dex85            # Exit w/ error if not equal
#
.dex55:
        ldos    d_expretlen[r3*2],r4    # Get the expected return data length
        ldconst 0xffff,r7               # Max length indicator
        cmpobe  r7,r4,.dex60            # If max, called function will validate
#
        ldconst deretlenbad,g1          # Prep possible error code
        ld      mr_ralloclen(r11),r7    # Get the return allocation length
        cmpobe  r4,r7,.dex60            # If equal, continue
#
        ldconst mrebfuncmax-mrebfuncbase+1,r4
        cmpoble r3,r4,.dex85            # If from the CCB, return error
        faultg                          # Else fault
#
# --- Check for sufficient NVRAM when required
#
.dex60:
        ldob    d_pktexp[r3*1],r5       # Get NVRAM expansion indication
        cmpobe  FALSE,r5,.dex65         # Jif not required
#
        ldconst deinsnvram,g1           # Set insufficient NVRAM
        ldconst TRUE,r5                 # Prep for error path out
        ldconst TRUE,r6                 # Prep for error path out
c       if (!check_nvram_p2_available()) {
            b   .dex85                  # If insufficient space available
c       }
#
# --- Stop rebuilds, scrubbing, hotswap, RAID initialization,
#     cacheing, parity/mirror checker
#
.dex65:
        ldconst deok,r4                 # Preset the flag to show good
        ldob    d_pktcon[r3*1],r5       # Get concurrency indication
        cmpobe  TRUE,r5,.dex70          # Jif so
#
        call    RB$pause_rebld          # Pause rebuild activity
#
        setbit  mxcwait,0,g0            # Wait for cache stop to complete
        ldconst mxcdefinebe,g1          # g1 = BE Define User ID
        call    d$cachestop             # Stop I/O activity
        mov     g0,r4                   # Save the Stop status
#
        call    R$chkstop               # Stop checker activity
#
        call    O$stop                  # Stop online activity
c       apool_stop();                   # Stop async activity
#
.dex70:
        ldob    d_scrubcon[r3*1],r6     # Get scrub concurrency
        cmpobe  TRUE,r6,.dex80          # Jif so
#
        call    R$stop                  # Stop scrubbing activity
#
# --- Execute action routine
#
.dex80:
        cmpobe  deok,r4,.dex82          # Jif, still OK to continue
        mov     r4,g1                   # Not ok, set up to return the error
        b       .dex85                  # Return the error and finish processing
#
.dex82:
        ld      d_pktact[r3*4],r4       # Get action routine
#
        ldconst trMRPStart,g0           # Trace start of MRP
        mov     r9,g1                   # Makes more sense to have original MRP value r9.
        call    D$TraceEvent
#
        mov     r11,g0                  # Load the MRP pointer
        mov     r10,g14                 # Load ILT pointer
        callx   (r4)                    # Execute
#
        ldconst trMRPStop,g0            # Trace end of MRP
        mov     g1,r4                   # Save g1
        shlo    8,g1,g1                 # Shift return status code 1 byte
        or      r3,g1,g1                # OR it to the MRP command number
        call    D$TraceEvent
        mov     r4,g1                   # Restore g1
#
# --- At this point, we check to see if the MRP that was performed is an MRP
# --- which will handle its own completion call to the link layer. These
# --- MRPs are ones that are non-deterministic in time and may block the
# --- define executive from completing other tasks (read - rescan).
#
        ldconst mrrescandevice-mrebfuncbase,r4
        cmpobe  r3,r4,.dex20            # Jif rescan
#
.dex85:
        ld      mr_rptr(r11),r3         # Get the return data ptr
!       stob    g1,mr_status(r3)        # Plug return status code
!       st      g2,mr_rlen(r3)          # Set return packet size
#
# --- Resume any halted operations
#
        cmpobe  TRUE,r5,.dex90          # Jif concurrency
#
        call    R$chkresume             # Resume checker activity
        call    O$resume                # Resume online activity
c       apool_start();                  # Resume async activity
        ldconst mccclearone,g0          # g0 = Clear only one stop
        ldconst mccdefinebe,g1          # g1 = BE Define User ID
        call    d$cacheresume           # Resume I/O activity
        call    RB$resume_rebld         # Resume rebuild activity
#
.dex90:
        cmpobe  TRUE,r6,.dex100         # Jif scrub concurrency
#
        call    R$resume                # Resume scrubbing activity
#
# --- Send status response
#
.dex100:
        mov     r10,g1                  # Complete this request
        call    K$comp
        b       .dex20
#
#**********************************************************************
#
#  NAME: d$spinstate
#
#  PURPOSE:
#       To provide a means of processing the spin up/down device
#       request issued by the CCB.
#
#  DESCRIPTION:
# --- THIS FUNCTION IS NOT USED. IF IT IS USED IN THE FUTURE, THERE
# --- MUST BE CHANGES FOR THE LED CONTROL ON SPINUP. MAYBE ASSUME GOOD
# --- AND CHANGE THE INIT DRIVE FUNCTION TO CHANGE IF FAILED.
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = status
#       g2 = return pkt size
#
#  REGS DESTROYED:
#       g0
#       g3
#
#**********************************************************************
#
d$spinstate:
#
# --- Validate parameters (bus, lun, and id) by looking up the PDD.
# --- If found, then the bus and channel can be assumed to be OK.
#
        ld      mr_ptr(g0),r15          # Get the parm block
#
        ldob    mss_channel(r15),g0     # Get the channel number
        ld      mss_id(r15),g1          # Get the ID
        ldos    mss_lun(r15),g2         # Get the LUN
        call    D$findpdd               # Look it up (g0 gets PDD)
        ldconst denonxdev,g1            # Prep error code for non-existent
        cmpobe  0,g0,.sp100             # Jif valid PDD
#
        mov     g0,g3                   # g3=PDD for this device
#
        ldob    mss_dir(r15),r3         # Get direction desired
        cmpobne mssstart,r3,.sp50       # If not spin up, go spin down
#
# --- Spin up the device, perform initialization checks
#
        ldob    pd_miscstat(g3),r4      # Turn off the spindown indicator
        clrbit  pdmbspindown,r4,r4
        stob    r4,pd_miscstat(g3)
#
# --- Turn the LED off. If the LED is set to fail, then the init drive
# --- function will do this.
#
        ldconst pdliteoff,r3            # Turn the LED off
        stob    r3,pd_fled(g3)
#
        call    O$ledchanged
#
        lda     O_drvinits,g8           # Get inits in progress pointer
        ld      (g8),r3                 # Get number of units waiting init
        addo    1,r3,r3                 # Add one more device
        st      r3,(g8)                 # Store it back
#
        lda     O$init_drv,g0
        ldconst OINITDRVPRI,g1
        ldconst 3,g2                    # Do the full test
c       CT_fork_tmp = (ulong)"O$init_drv";
        call    K$tfork                 # Spawn a process for spinning up
#
        ldconst deok,g1                 # Return command completed
        b       .sp100
#
# --- Spin down the device
#
.sp50:
        ldob    pd_miscstat(g3),r4      # Mark this PDD as spinning down
        setbit  pdmbspindown,r4,r4
        stob    r4,pd_miscstat(g3)      # Mark this PDD as spinning down
#
        lda     O_t_stopunit,g0         # Pass stop unit template
        call    O$genreq                # Generate request
        call    O$quereq                # Queue request
        call    O$relreq                # Release request
#
        ldconst pdnonx,r3               # Mark PDD as non existent.
        stob    r3,pd_devstat(g3)
#
        ldconst pdliteid,r3             # Blink the identify LED
        stob    r3,pd_fled(g3)
#
        call    O$ledchanged
#
        ldconst deok,g1                 # return command completed
#
# --- Exit
#
.sp100:
        ldconst mssrsiz,g2              # Set return packet size
        ret
#
#**********************************************************************
#
#  NAME: d$restoredev
#
#  PURPOSE:
#       To restore a drive to a good state for development debugging.
#
#  DESCRIPTION:
#       Run init_drv to spinup the drive and update PDD states
#       Update entire file system - keep existing label class
#       Force name to the current slot
#       Run init_drv to update PDD states
#       Update RAID & VDisk states
#       Kick off any rebuilds
#       Update the configuration
#
#       This may take 10's of seconds to complete, especially if the drive
#       needs to be spunup.
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = status
#       g2 = return pkt size
#
#  REGS DESTROYED:
#       g0
#
#**********************************************************************
#
d$restoredev:
        ld      mr_ptr(g0),r3           # Load parm block pointer
        ldob    mdr_opt(r3),r15         # Get option parm
        ldos    mdr_pid(r3),r12         # PID we are updating
#
# --- Validate the parms ----------------------------------------------
#
        ldconst deinvpid,r10            # Set invalid PID
        ldconst MAXDRIVES,r3
        cmpobge r12,r3,.dres1000        # Jif out of range
#
        ld      P_pddindx[r12*4],g3     # Get PDD
        cmpobe  0,g3,.dres1000          # Jif not a known device
#
        ldob    pd_class(g3),r11        # Get class for later
#
# --- Spinup the drive and update PDD states --------------------------
#
        ldob    pd_miscstat(g3),r3
        clrbit  pdmbreplace,r3,r3       # Clear the replace bit or the
        stob    r3,pd_miscstat(g3)      #   devstat will stay inoperable
#
# --- Clear the spinddown bit and user fail (pdisk) bit ------------------------
#
        ldob    pd_flags(g3),r4         # Load PDD Flags
        clrbit  pduserspundown,r4,r4    # Clear spindown bit
        clrbit  pduserfailed,r4,r4      # clear pdisk fail bit
        stob    r4, pd_flags(g3)        # Store the updated value
#
        mov     g8,r14                  # Save g8
        lda     O_drvinits,g8           # Get inits in progress pointer
        ld      (g8),r3                 # Get number of units waiting init
        addo    1,r3,r3                 # Add one more device
        st      r3,(g8)                 # Store it back
#
        ldconst 3,g2                    # Full test
        call    O$init_drv              # g3=PDD, g8=inits in progress
        mov     r14,g8                  # Restore g8
#
# --- Update file system from another device --------------------------
#
        ldconst deioerr,r10             # Assume it did not work
        ldconst 1,g4                    # Do not allow UpdateFS to be interrupted
        call    FS$UpdateFS             # Update file system, g3=PDD
        cmpobne 0,g0,.dres1000
#
# --- Update the label ------------------------------------------------
#
c       g0 = s_MallocC(LABELSIZE*SECSIZE, __FILE__, __LINE__);
#
# --- Copy the standard device label string if this wasn't previously labelled
#
        cmpobne pdunlab,r11,.dres10     # Jif labelled
        ldconst 0,r8                    # Clear PDD serial number
        st      r8,pd_sserial(g3)
        b       .dres60                 # Write the label
#
.dres10:
        lda     O_devlab,r3             # Copy label str to dev label buffer
        lda     xd_text(g0),r6          # Get destination address
#
.dres20:
        ldob    (r3),r7                 # Get next label byte
        cmpobe  0,r7,.dres30            # Jif null termination
#
        stob    r7,(r6)                 # Store next label byte
        addo    1,r3,r3                 # Advance src pointer
        lda     1(r6),r6                # Advance dst pointer
        b       .dres20
#
# --- Load the label file fields.
#     The xd_failtype field is set to zero from MallocWC.
#     The dname is forced to the current slot or if that's invalid then
#     it will take the previous name from the PDD.
#
.dres30:
        ld      K_ficb,r7               # Get system serial number
        ld      fi_vcgid(r7),r7         # Get virtual controller group ID
        ldl     pd_wwn(g3),r8           # Get WWN (r8,r9)
        ldconst 0x00004450,r10          # Force 'PDxx' into dname
        ldconst 0xff,r4                 # Invalid slot
        ldob    pd_ses(g3),r5           # Get just 1 byte of SES
        ldob    pd_slot(g3),r6
        cmpobe  r4,r5,.dres40           # Jif SES invalid
        cmpobe  r4,r6,.dres40           # Jif slot invalid
        shlo    16,r5,r5                # Shift & OR in SES
        or      r5,r10,r10
        shlo    24,r6,r6                # Shift & OR in slot
        or      r6,r10,r10              # OR in slot
        b       .dres50
#
.dres40:
        ld      pd_dname(g3),r10        # Use current name since ses/slot invalid
#
.dres50:
        st      r7,xd_sserial(g0)       # Set up device system serial number
        stl     r8,xd_wwn(g0)           # Set up WWN
        st      r10,xd_dname(g0)        # Set the position information in label
        stob    r11,xd_class(g0)        # Set up device class
#
# --- Write it to the label file.
#
.dres60:
        mov     g0,r3                   # Save buffer pointer
        ldconst fidlabel,g0             # Label file
        mov     r3,g1                   # Address of buffer
        ldconst LABELSIZE,g2            # Size in blocks
                                        # PDD is in g3
        ldconst 1,g4                    # Block one of the label file
        call    FS$WriteFile            # Write it
        mov     g0,r5                   # Save return code
#
c       s_Free(r3, LABELSIZE*SECSIZE, __FILE__, __LINE__); # Release buffer for label
#
        ldconst deioerr,r10             # Assume it did not work
        cmpobne 0,r5,.dres1000          # Jif error status
#
# --- Reset the device status and rebuild info
#
        ldconst pdop,r3                 # Set to operable
        stob    r3,pd_devstat(g3)
#
        ldob    pd_miscstat(g3),r3
        clrbit  pdmbserialchange,r3,r3  # Clear serial number changed
        clrbit  pdmbrebuilding,r3,r3    # Clear the rebuilding bit
        clrbit  pdmbreplace,r3,r3       # Clear the replace bit
        stob    r3,pd_miscstat(g3)
#
c       ((PDD*)g3)->rbRemain = 0;       # Clear the remaining blocks to rebuild
c       ((PDD*)g3)->pctRem = 0;         # Clear percent rebuilding remaining on drive
#
# --- Update PDD states again -----------------------------------------
#
        mov     g8,r14                  # Save g8
        lda     O_drvinits,g8           # Get inits in progress pointer
        ld      (g8),r3                 # Get number of units waiting init
        addo    1,r3,r3                 # Add one more device
        st      r3,(g8)                 # Store it back
#
        ldconst 3,g2                    # Full test
        call    O$init_drv              # g3=PDD, g8=inits in progress
        mov     r14,g8                  # Restore g8
#
        call    O$ledchanged            # Change the LED
#
# --- Update PSD & RDD states & kick-off any required rebuilds --------
#
        call    RB_setpsdstat           # Update the PSD status
        call    RB_searchforfailedpsds  # If anything is failed and not
                                        #  spared already, this will find it
#
# --- Update NVRAM ----------------------------------------------------
#
        call    D$p2updateconfig        # Update NVRAM part II
        ldconst deok,r10                # It worked
#
.dres1000:
        mov     r10,g1                  # Set error code
        ldconst mdrrsiz,g2              # Set return packet size
        ret
#
#**********************************************************************
#
#  NAME: d$deletevlink
#
#  PURPOSE:
#       Process a delete virtual link request from the CCB.
#
#  DESCRIPTION:
#       This function is called by the delete virtual device function
#       once it has determined that the virtual device to be deleted
#       is a vlink.
#
#  INPUT:
#       g8 = VDD to delete
#       g14 = VID
#
#  OUTPUT:
#       g1 = return status code
#       g2 = return pkt size
#
#  REGS DESTROYED:
#       Reg. g1, g2 destroyed.
#
#**********************************************************************
#
d$deletevlink:
        ldob    D_Vflag,r13             # r13 = current VDisk/VLink busy flag
#
        ldconst TRUE,g1
        ldconst 0,g2                    # Cluster (jlw)
        stos    g14,D_Vvid              # Save VDisk #
        stob    g2,D_Vcl                # Save cluster #
        stob    g1,D_Vflag              # Set flag indicating VDisk/VLink busy
#
# --- Terminate VLink to destination MAGNITUDE or TBolt.
#
        ld      vd_rdd(g8),r5           # r5 = RDD of only RAID in vdisk
        ld      rd_psd(r5),r6           # r6 = PSD address
        ldos    ps_pid(r6),r7           # r7 = ordinal of LDD
        ld      DLM_lddindx[r7*4],r8    # r8 = LDD address
        mov     r8,g0                   # g0 = LDD address
        mov     g14,g3                  # g3 = VID
        cmpobe  0,r8,.dvlink_100        # Jif no LDD defined
        call    DLM$term_vl             # terminate VLink to dest. MAG
.dvlink_100:
#
# --- Delete the PSD and the RDD. The caller will take care of the VDD.
#
c       s_Free(r6, psdsiz, __FILE__, __LINE__); # Free PSD
#
        ldos    rd_rid(r5),r3
        ldconst 0,r4                    # Get a zero
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        st      r4,R_rddindx[r3*4]      # Mark the RDD as invalid
        st      r4,V_vddindx[g14*4]     # Unlink VDD from VDX
        ldos    rx_ecnt+R_rddindx,r3    # Get the count
        subo    1,r3,r3                 # Decrement it
        stos    r3,rx_ecnt+R_rddindx    # Set the count
#
c       s_Free(r5, rddsiz+4, __FILE__, __LINE__); # Free One PSD for a RDD in a Vlink
#
# --- Clean up the LDD.
#
        cmpobe  0,r8,.dvlink_700        # Jif no LDD defined
        mov     r8,g0                   # Send LDD into function
        call    DLM$chk_ldd             # Clean up
.dvlink_700:
#
        stob    r13,D_Vflag             # restore VDisk/VLink busy flag
        ret
#
#**********************************************************************
#
#  NAME: d$setcache
#
#  PURPOSE:
#       To provide a means of processing the set cacheability request
#       issued by the CCB.
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = status
#       g2 = return pkt size
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
d$setcache:
        ld      mr_ptr(g0),g0           # Get parm block
#
        ldob    msc_mode(g0),r14        # Get new mode
        ldconst deinvopt,g1
        bbc     7,r14,.vs100            # Jif not global change
#
# --- Global cache change operation.
#
        clrbit  7,r14,r14               # Clear the global indicator
        stob    r14,D_glcache           # Set global cache enable
        call    D$updrmtcacheglobal     # Send the new value to the FEP
#
# --- Send a log message to CCB
#
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst mleglobalcachemode,r3   # Get log code
        ldconst TRUE,r4
        bbs     0,r14,.vs50             # Jif global cache is ON
        ldconst FALSE,r4                # global cache is OFF
.vs50:
        st      r3,mle_event(g0)        # Store as word to clear other bytes
        stob    r4,epc_mode(g0)         # Store the cache mode
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], epclen);
#
# --- Update NVRAM
#
        call    D$p2updateconfig        # Update NVRAM part II
        mov     deok,g1                 # Return OK status
#
# --- Exit
#
.vs100:
        ldconst mscrsiz,g2              # Set return packet size
        ret
#
#**********************************************************************
#
#  NAME: d$setserverprop
#
#  PURPOSE:
#       To provide a means of processing the set server properties
#       request issued by the CCB.
#
#  DESCRIPTION:
#       The priority of the specified server is updated to reflect the
#       selected priority and attributes.
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = status
#       g2 = return pkt size
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
d$setserverprop:
        ld      mr_ptr(g0),g0           # Get parm block
#
# --- Validate parameters (server ID and priority)
#
        ldos    mpr_sid(g0),r15         # Get SID number
        ldconst deinvsid,g1             # Prep possible error code
        ldconst MAXSERVERS,r3           # Get number of servers
        cmpoble r3,r15,.ss100           # Jif incorrect
#
        lda     S_sddindx,r3            # Get SDX pointer
        ld      sx_sdd(r3)[r15*4],r3    # Get SDD pointer
        cmpobe  0,r3,.ss100             # Jif undefined
#
        ldob    mpr_pri(g0),r14         # Get priority
        ldconst deinvpri,g1             # Prep possible error code
        ldconst MAXSRVPRI,r4            # Get max allowable priority
        cmpobl  r4,r14,.ss100           # Jif incorrect
#
        stob    r14,sd_pri(r3)          # Set up priority
#
        ld      sd_attrib(r3),r14       # Get current attribute
        setbit  sddefault,0,r9          # Mask for all bits except
        setbit  sdxio,r9,r9             #    XIOtech controller bit and
        setbit  sdunmanaged,r9,r9       #    unmanaged server flag bit and
        not     r9,r9                   #    Default server bit
        ld      mpr_attrib(g0),r8       # Get attribute
        modify  r9,r8,r14               # Modify attribute bits
        st      r14,sd_attrib(r3)       # Set up attribute
#
# --- Update NVRAM
#
        call    D$p2updateconfig        # Update NVRAM part II
#
# --- Update the remote server record
#
        mov     r15,g0                  # Input parm
        ldconst FALSE,g1                # False = do not delete
        call    D_updrmtserver          # Update remote
        call    D$signalserverupdate    # Tell FEP it happened
#
        mov     deok,g1                 # Return OK status
#
# --- Exit
#
.ss100:
        ldconst mprrsiz,g2              # Set return packet size
        ret
#
#**********************************************************************
#
#  NAME: d$vpreadwrite
#
#  PURPOSE:
#       Allow reading or writing a VDisk or PDisk from ccbCL.pl.
#
#  INPUT:
#       g0 = MRP
#       g14= ILT
#
#  OUTPUT:
#       g1 = status
#       g2 = return pkt size
#
#  REGS DESTROYED:
#       g0.
#
#**********************************************************************
#
d$vpreadwrite:
        ld      mr_ptr(g0),r12                  # Set up the parm block pointer
        st      0,il_w0(g14)                    # Clear ILT values
        st      0,il_w1(g14)
        st      0,il_w2(g14)
        st      0,il_w3(g14)

c   if (((MRREADWRITEIO_REQ*)r12)->pv == 0x70) {    # 'p' physical not done yet.
        b       .vpreadwrite50
c   }
#
c       r15 = get_ilt();                        # Allocate an ILT
c       r14 = get_vrp();                        # Allocate a VRP
c       ((ILT*)r15)->ilt_normal.w0 = r14;       # Save VRP address in ilt w0
c       ((VRP*)r14)->vid = ((MRREADWRITEIO_REQ*)r12)->id; # ID to read/write
c       ((VRP*)r14)->length = 1;                # Only one sector at a time
c       ((VRP*)r14)->startDiskAddr = ((MRREADWRITEIO_REQ*)r12)->block;
c       r10 = m_asglbuf(1 * BYTES_PER_SECTOR);
c       ((VRP*)r14)->pSGL = (SGL*)r10;
c       ((VRP*)r14)->sglSize = sizeof(SGL) + sizeof(SGL_DESC);
c   if (((MRREADWRITEIO_REQ*)r12)->rw == 0x72) {    # 'r'
c       ((VRP*)r14)->function = VRP_INPUT;      # Read
c   } else {
c       ((VRP*)r14)->function = VRP_OUTPUT;     # Write
c       r9 = (UINT32)((SGL*)r10 + 1);           # Get to SGL_DESC
c       r8 = (UINT32)((SGL_DESC*)r9)->addr;     # Buffer for SGL.
c       memcpy((void *)r8, ((MRREADWRITEIO_REQ*)r12)->bptr, 512);
c   }
#
        lda     V_que,g0
c       g1 = r15;
        call     EnqueueILTW
#
c       g1 = ((VRP*)r14)->status;               # return error status
c   if (g1 != ecok) {
c       fprintf(stderr, "d$vpreadwrite ERROR, vrp status = %ld\n", g1);
c       g1 = deioerr;                           # Device I/O error
c   }
#
c   if (((MRREADWRITEIO_REQ*)r12)->rw == 0x72) {   # 'r'
c       r9 = (UINT32)((SGL*)r10 + 1);           # Get to SGL_DESC
c       r8 = (UINT32)((SGL_DESC*)r9)->addr;     # Buffer for SGL.
c       memcpy(((MRREADWRITEIO_REQ*)r12)->bptr, (void *)r8, 512);
c   }
#
c       PM_RelSGLWithBuf((void*)r10);           # Release combined SGL and data buffer
#
.ifdef M4_DEBUG_VRP
c CT_history_printf("%s%s:%u put_vrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, r14);
.endif # M4_DEBUG_VRP
c       put_vrp(r14);                           # Release VRP
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, r15);
.endif # M4_DEBUG_ILT
c       put_ilt(r15);                           # Deallocate ILT
        ldconst mrwrsiz,g2                      # Set up return packet size
        ret
#
# --- Do physical disk read below.
#
.vpreadwrite50:
c       r9 = get_prp();                         # Assign PRP
.ifdef M4_DEBUG_PRP
c CT_history_printf("%s%s:%u get_prp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, r9);
.endif # M4_DEBUG_PRP
c       r8 = get_ilt();                         # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, r8);
.endif # M4_DEBUG_ILT
c       ((ILT*)r8)->ilt_normal.w0 = r9;         # Save PRP address in ilt w0

c       g0 = ((MRREADWRITEIO_REQ*)r12)->id;     # ID to read/write
        ld      P_pddindx[g0*4],g3              # Pointer to PDD
        ldconst denonxdev,g1                    # Prep error code for non-existent
        cmpobe  0,g3,.vp5000                    # Jif invalid PDD
#
        ld      pd_dev(g3),r3                   # Get the DEVice pointer
        cmpobe  0,r3,.vp5000                    # Jif invalid DEV, must free memory, etc.
.ifndef MODEL_3000
.ifndef  MODEL_7400
        ldconst debusy,g1
        ldob    pd_flags(g3),r4
        bbs     pdbebusy,r4,.vp5000
.endif  # MODEL_7400
.endif  # MODEL_3000
#
        lda     -ILTBIAS(g14),r4                # Input ILT
#       Copy all the contents of original ILT to the ILT allocated locally
c       memcpy ((void*)r8, (void*)r4, ILT_SIZE);
#
        lda     ILTBIAS(r8),g1                  # bump the ILT to next lvl
        st      r9,il_w0(g1)                    # Link PRP to ILT
# --- Set up PRP
#--- above        ld      pd_dev(g3),r3                   # Set up DEVice pointer
        st      r3,pr_dev(r9)
        ldob    dv_chn(r3),r4                   # Set up channel
        stob    r4,pr_channel(r9)
        ldos    dv_lun(r3),r4                   # Set up LUN
        stos    r4,pr_lun(r9)
        ld      dv_id(r3),r4                    # Set up ID
        st      r4,pr_id(r9)
        stob    MRSCSIIO_FLAGS_SLI,pr_flags(r9)
        stob    1,pr_retry(r9)                  # Set up for 1 retry
c   if (((MRREADWRITEIO_REQ*)r12)->rw == 0x72) {   # 'r'
        stob    MRSCSIIO_INPUT,pr_func(r9)      # Set up function
c   } else {
        stob    MRSCSIIO_OUTPUT,pr_func(r9)     # Set up function
c   }
        stob    0,pr_tmocnt(r9)                 # Set up timeout counter
        stob    MRSCSIIO_NORM,pr_strategy(r9)   # Set up strategy
        st      8,pr_timeout(r9)                # Set up timeout
c   if (((MRREADWRITEIO_REQ*)r12)->rw == 0x72) {   # 'r'
c     if ((((MRREADWRITEIO_REQ*)r12)->block & ~0xffffffffULL) != 0ULL) {    # 16 byte command
# read-16
c       ((PRP*)r9)->cmd[0] = SCC_READ_16;       # read-16
c       ((PRP*)r9)->cmd[1] = 0x00;              # Flags/options for this command
c       *(UINT64*)&(((PRP*)r9)->cmd[2]) = bswap_64(((MRREADWRITEIO_REQ*)r12)->block); # 2,3,4,5 6,7,8,9
c       ((PRP*)r9)->cmd[10] = 0x00;             # number of blocks to write (MSB)
c       ((PRP*)r9)->cmd[11] = 0x00;             # number of blocks to write
c       ((PRP*)r9)->cmd[12] = 0x00;             # number of blocks to write
c       ((PRP*)r9)->cmd[13] = 0x01;             # number of blocks to write (LSB)
c       ((PRP*)r9)->cmd[14] = 0x00;             # reserved
c       ((PRP*)r9)->cmd[15] = 0x00;             # control
c       ((PRP*)r9)->cBytes = 16;                # Set up command length -- CDB length
c       ((PRP*)r9)->rqBytes = ((MRREADWRITEIO_REQ*)r12)->dataInLen; # Set up buffer length
c     } else {
# read-10
c       ((PRP*)r9)->cmd[0] = SCC_READEXT;       # read-10
c       ((PRP*)r9)->cmd[1] = 0x00;              # 3/reserved,DP0,FUA,2/reserved,RelAdr
c       *(UINT32*)&(((PRP*)r9)->cmd[2]) = bswap_32((((MRREADWRITEIO_REQ*)r12)->block) & 0xffffffff);
c       ((PRP*)r9)->cmd[6] = 0x00;              # reserved
c       ((PRP*)r9)->cmd[7] = 0x00;              # number of blocks to read
c       ((PRP*)r9)->cmd[8] = 0x01;              # 2nd part of # blocks
c       ((PRP*)r9)->cmd[9] = 0x00;              # control
c       ((PRP*)r9)->cmd[10] = 0x00;             # unused
c       ((PRP*)r9)->cmd[11] = 0x00;             # unused
c       ((PRP*)r9)->cmd[12] = 0x00;             # unused
c       ((PRP*)r9)->cmd[13] = 0x00;             # unused
c       ((PRP*)r9)->cmd[14] = 0x00;             # unused
c       ((PRP*)r9)->cmd[15] = 0x00;             # unused
c       ((PRP*)r9)->cBytes = 10;                # Set up command length -- CDB length
c       ((PRP*)r9)->rqBytes = ((MRREADWRITEIO_REQ*)r12)->dataInLen; # Set up buffer length
c     }
c   } else {
c     if ((((MRREADWRITEIO_REQ*)r12)->block & ~0xffffffffULL) != 0ULL) {    # 16 byte command
# write-16
c       ((PRP*)r9)->cmd[0] = SCC_WRITE_16;      # write-16
c       ((PRP*)r9)->cmd[1] = 0x00;              # Flags/options for this command
c       *(UINT64*)&(((PRP*)r9)->cmd[2]) = bswap_64(((MRREADWRITEIO_REQ*)r12)->block); # 2,3,4,5 6,7,8,9
c       ((PRP*)r9)->cmd[10] = 0x00;             # number of blocks to write (MSB)
c       ((PRP*)r9)->cmd[11] = 0x00;             # number of blocks to write
c       ((PRP*)r9)->cmd[12] = 0x00;             # number of blocks to write
c       ((PRP*)r9)->cmd[13] = 0x01;             # number of blocks to write (LSB)
c       ((PRP*)r9)->cmd[14] = 0x00;             # reserved
c       ((PRP*)r9)->cmd[15] = 0x00;             # control
c       ((PRP*)r9)->cBytes = 16;                # Set up command length -- CDB length
c       ((PRP*)r9)->rqBytes = ((MRREADWRITEIO_REQ*)r12)->dataInLen; # Set up buffer length
c     } else {
# write-10
c       ((PRP*)r9)->cmd[0] = SCC_WRITEXT        # write-10
c       ((PRP*)r9)->cmd[1] = 0x00;              # 3/WRPROTECT,1/DPO,1/FUA,1/Reserved,1/FUA_NV,1/Obsolete
c       *(UINT32*)&(((PRP*)r9)->cmd[2]) = bswap_32((((MRREADWRITEIO_REQ*)r12)->block) & 0xffffffff);
c       ((PRP*)r9)->cmd[6] = 0x00;              # 3/reserved,5/group#
c       ((PRP*)r9)->cmd[7] = 0x00;              # number of blocks to write
c       ((PRP*)r9)->cmd[8] = 0x01;              # 2nd part of # blocks
c       ((PRP*)r9)->cmd[9] = 0x00;              # control
c       ((PRP*)r9)->cmd[10] = 0x00;             # unused
c       ((PRP*)r9)->cmd[11] = 0x00;             # unused
c       ((PRP*)r9)->cmd[12] = 0x00;             # unused
c       ((PRP*)r9)->cmd[13] = 0x00;             # unused
c       ((PRP*)r9)->cmd[14] = 0x00;             # unused
c       ((PRP*)r9)->cmd[15] = 0x00;             # unused
c       ((PRP*)r9)->cBytes = 10;                # Set up command length -- CDB length
c       ((PRP*)r9)->rqBytes = ((MRREADWRITEIO_REQ*)r12)->dataInLen; # Set up buffer length
c     }
c   }

# --- Generate single descriptor SGL within il_w4 thru il_w7 of ILT
        lda     il_w4(g1),r14                   # Form SGL ptr within ILT
        st      r14,pr_sglptr(r9)
        st      0,pr_sglsize(r9)

        stos    0,sg_owners(r14)                # Clear ownership count/flags
        ldconst sghdrsiz+sgdescsiz,r3           # Set up SGL size
        st      r3,sg_size(r14)
        stos    1,sg_scnt(r14)                  # Set up SGL desc count
c       r4 = (UINT32)((MRREADWRITEIO_REQ*)r12)->bptr;
        st      r4,sghdrsiz+sg_addr(r14)        # Set up buffer
c       r5 = ((MRREADWRITEIO_REQ*)r12)->dataInLen; # Set up buffer length
        st      r5,sghdrsiz+sg_len(r14)         # Set up length
#
# --- Queue PRP to physical layer
        ld      P_que,g0                        # Pass physical queuing routine
#       g1 = ilt
        call     EnqueueILTW
#
# --- Command returned successful, check SCSI status
#
c       g2 = r9;
        call    M$chkstat                       # Check status
        mov     g0,g1                           # Save status
c   if (g1 != ecok) {
c       fprintf(stderr, "d$vpreadwrite ERROR, SCSI status = %ld\n", g1);
c       g1 = deioerr;                           # Device I/O error
c   }
#
.vp5000:
.ifdef M4_DEBUG_PRP
c CT_history_printf("%s%s:%u put_prp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, r9);
.endif # M4_DEBUG_PRP
c       put_prp(r9);                            # Release PRP
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, r8);
.endif # M4_DEBUG_ILT
c       put_ilt(r8);                            # Deallocate ILT
# Note: g1 = return or error value.
        ldconst msirsiz,g2                      # Set up return packet size
        ret

#**********************************************************************
#
#  NAME: d$scsiio
#
#  PURPOSE:
#       To provide a means of processing the physical SCSI I/O request
#       issued by the CCB.
#
#  DESCRIPTION:
#       This routine assigns a PRP and links it to the ILT that was
#       associated with the CCB request. The PRP is initialized with
#       information taken directly from the request packet. The
#       resultant request is queued to the physical layer to perform
#       the requested SCSI operation.
#
#       When completed, the return status is updated and the request
#       is returned.
#
#  INPUT:
#       g0 = MRP
#       g14= ILT
#
#  OUTPUT:
#       g1 = status
#       g2 = return pkt size
#
#  REGS DESTROYED:
#       g0.
#
#**********************************************************************
#
d$scsiio:
#
# --- Validate parameters (bus, lun, and id) by looking up the PDD.
# --- If found, then the bus and channel can be assumed to be OK.
#
        ld      mr_ptr(g0),r12          # Set up the parm block pointer
        ld      mr_rptr(g0),r13         # Set up return data pointer
        ld      msi_rsn(r12),r15        # Get RSN
        ldconst 0,r9
#
# --- Clear SCSI sense status
#
!       stob    r9,msi_skey(r13)        # Clear sense key
!       stob    r9,msi_asc(r13)         # Clear ASC
!       stob    r9,msi_ascq(r13)        # Clear ASCQ
#
        ldob    msi_cdb(r12),r11        # Get SCSI command opcode
        ldconst 0x3b,r3                 # Write buffer command
        cmpobne r3,r11,.si05            # Jif not a write buffer
        call    O$stop                  # Else stop online
                                        # Potential task switch
#
# --- Assign PRP and link to existing ILT
#
.si05:
        movq    0,r4                    # Clear ILT
        stq     r4,il_w0(g14)
#
c       g2 = get_prp();                 # Assign PRP
                                        # Potential task switch
.ifdef M4_DEBUG_PRP
c CT_history_printf("%s%s:%u get_prp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_PRP
#
c       g1 = get_ilt();                 # Allocate an ILT
                                        # Potential task switch
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       r4 = g1;                        # Save g1 (ILT)
c       r5 = g2;                        # Save g2 (PRP)
# --- If the CCB gives us a WWN and sets the idchc (identify choice) to
# --- msiwwn, then the PDD must be found using the WWN. If the idchc is
# --- set to msicli, then the PDD is found using the channel/LUN/ID.
# --- NOTE: this is first principle, after potential task switch, do again.
#
        ldob    msi_idchc(r12),r3       # Get the identify choice
        cmpobne msiwwn,r3,.si10         # Jif identify by ch/lun/id
#
# --- Find PDD by the WWN
#
        ldl     msi_wwn(r12),g0         # Get the WWN
        ldob    msi_lun(r12),g2         # Get the LUN
        call    d$findpddwwn            # Look up PDD with WWN
        b       .si20                   # Go to see if found
#
# --- Find PPD by the channel, ID, and LUN
#
.si10:
        ldob    msi_channel(r12),g0     # Get the channel number
        ld      msi_id(r12),g1          # Get the ID
        ldos    msi_lun(r12),g2         # Get the LUN
# c fprintf(stderr, "%s%s:%u ID = %lx LUN = %ld\n", FEBEMESSAGE, __FILE__, __LINE__, g1, g2);
        call    D$findpdd               # Look it up (g0 gets PDD)
#
.si20:
        ldconst denonxdev,g1            # Prep error code for non-existent
        ldconst 0,g2                    # Prep for no PRP
        cmpobe  0,g0,.si1000            # Jif invalid PDD
#
        ld      pd_dev(g0),r3           # Get the DEVice pointer
        cmpobe  0,r3,.si1000            # Jif invalid DEV, must free memory, etc.
#
.ifndef MODEL_3000
.ifndef  MODEL_7400
        ldconst debusy,g1
        ldob    pd_flags(g0),r3
        bbs     pdbebusy,r3,.si1000
.endif  # MODEL_7400
.endif  # MODEL_3000
        mov     g0,g3                   # g3=PDD for this device
c       g1 = r4;                        # Restore g1 (ILT)
c       g2 = r5;                        # Restore g2 (PRP)
#
        mov     g14,r4
        lda     -ILTBIAS(r4),r4
#
#       Copy all the contents of original ILT to the ILT allocated locally
#
c       memcpy ((void*)g1, (void*)r4, ILT_SIZE);
#
        lda     ILTBIAS(g1),g1          # bump the ILT to next lvl
        st      g2,il_w0(g1)            # Link PRP to ILT
#
# --- Set up PRP
#
        ld      pd_dev(g3),r3           # Set up DEVice pointer
        st      r3,pr_dev(g2)
        ldob    dv_chn(r3),r4           # Set up channel
        stob    r4,pr_channel(g2)
        ldos    dv_lun(r3),r4           # Set up LUN
        stos    r4,pr_lun(g2)
        ld      dv_id(r3),r4            # Set up ID
        st      r4,pr_id(g2)
        ldob    msi_flags(r12),r3       # Set up flags
        stob    r3,pr_flags(g2)
        ldob    msi_retry(r12),r3       # Set up retry
        stob    r3,pr_retry(g2)
        addo    1,r3,g5                 # Store actual command attempts in g5
        ldob    msi_func(r12),r3        # Set up function
        stob    r3,pr_func(g2)
        ldconst 0,r3                    # Set up timeout counter
        stob    r3,pr_tmocnt(g2)
        ldob    msi_strat(r12),r3       # Set up strategy
        stob    r3,pr_strategy(g2)
        ldob    msi_cmdlen(r12),r3      # Set up command length
        stob    r3,pr_cbytes(g2)
!       ldl     msi_sda(r12),r4         # Set up SDA
        stl     r4,pr_sda(g2)
!       ldl     msi_eda(r12),r4         # Set up EDA
        stl     r4,pr_eda(g2)
        ld      msi_timeout(r12),r3     # Set up timeout
        st      r3,pr_timeout(g2)
        addo    8,r3,r3                 # Add ABTS handling time to the timeout
        mulo    r3,g5,g5                # Wait = (actual command attempts) * (timeout + ABTS handling)
        ldq     msi_cdb(r12),r4         # Set up SCSI command
        stq     r4,pr_cmd(g2)
        ldl     msi_bptr(r12),r4        # Get buffer and length
        st      r5,pr_rqbytes(g2)
#
# --- Generate single descriptor SGL within il_w4 thru il_w7 of ILT
#
        lda     il_w4(g1),r14           # Form SGL ptr within ILT
        st      r14,pr_sglptr(g2)
        ldconst 0,r3                    # Declare null length SGL
        st      r3,pr_sglsize(g2)
        stos    r3,sg_owners(r14)       # Clear ownership count/flags
        ldconst sghdrsiz+sgdescsiz,r3   # Set up SGL size
        st      r3,sg_size(r14)
        ldconst 1,r3                    # Set up SGL desc count
        stos    r3,sg_scnt(r14)
        stl     r4,sghdrsiz+sg_addr(r14)# Set up buffer and length
#
# --- Fork the timeout routine
#
        mov     g1,r3                   # Save g1
# -- Note that g3 is no longer used in d$scsiio_wait.
        mov     g1,g4                   # Pass ILT to the timeout
        mov     g14,g6                  # Pass Orig ILT
        lda     d$scsiio_wait,g0        # Load timeout address
        ldconst DSCSIPRI,g1             # Set priority
c       CT_fork_tmp = (ulong)"d$scsiio_wait";
        call    K$tfork                 # Fork away
        mov     r3,g1                   # Restore g1
        st      g0,il_w3(g1)            # Save timeout pcb in ILT
#
        mov     g1,r8                   # Save g1
#
# --- Queue PRP to physical layer
#
        mov     g2,r3                   # Save PRP
        ld      P_que,g0                # Pass physical queuing routine
        lda     d$scsicomp,g2           # Pass completion routine
        call    K$q                     # Queue SCSI I/O
        mov     r3,g2                   # Restore PRP
#
# --- Set process to not ready
#
        ldconst pcscsiiowait,r3         # Sleep
        ld      d_exec_qu+qu_pcb,r4     # Retrieve define PCB
        stob    r3,pc_stat(r4)
        call    K$xchang                # Exchange processes
#
# --- Wake up point. If pr_timeout is 0, op was a success
        ldconst deok,g1
        ld      pr_timeout(g2),r9       # Load pr_timeout
        cmpobe  0,r9,.si35              # Jif op was successful
#
# --- SCSI op failed, copy everything back to the original ILT, that is in g14.
#
        mov     g14,r4
        lda    -ILTBIAS(r4), r4        # bump to 0 level for original ILT
        lda    -ILTBIAS(r8), r9        # bump to 0 level for Local ILT
c       memcpy ((void*)r4, (void*)r9, ILT_SIZE);
        b       .si50                  # Branch toward end
#
.si35:
#
# --- Command returned successful, check SCSI status
#
        call    M$chkstat               # Check status
        mov     g0,g1                   # Save status
        cmpobe  ecok,g0,.si50           # Jif status OK
#
        ldob    pr_sstatus(g2),r3       # Get SCSI status byte
        cmpobne 2,r3,.si50              # Jif not check condition
#
# --- Set up SCSI sense status
#
        ldob    pr_sense+2(g2),r3       # Get sense key and isolate
        and     0xf,r3,r3
!       stob    r3,msi_skey(r13)        # Set up sense key
        ldob    pr_sense+12(g2),r3      # Set up ASC
!       stob    r3,msi_asc(r13)
        ldob    pr_sense+13(g2),r3      # Set up ASCQ
!       stob    r3,msi_ascq(r13)
#
# --- Release PRP if present
#
.si50:
        cmpobne 0,r9,.si60              # Jif  SCSI Cmd timeout
        cmpobe  0,g2,.si60              # Jif no PRP
.ifdef M4_DEBUG_PRP
c CT_history_printf("%s%s:%u put_prp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_PRP
c       put_prp(g2);                    # Release PRP
#
.si60:
        ldconst 0x3b,r3                 # Write buffer command
        cmpobne r3,r11,.si100           # Jif not a write buffer
        call    O$resume                # Else restart online
#
# --- Exit
#
.si100:
!       st      r15,msi_rrsn(r13)       # Set up return RSN
        ldconst msirsiz,g2              # Set up return packet size
        ret
#
# Error happened, just free PRP and ILT, and restart online.
#
.si1000:
.ifdef M4_DEBUG_PRP
c CT_history_printf("%s%s:%u put_prp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, r5);
.endif # M4_DEBUG_PRP
c       put_prp(r5);                    # Release PRP
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, r4);
.endif # M4_DEBUG_ILT
c       put_ilt(r4);                    # Deallocate ILT
        b       .si60                   # Restart online
#
#**********************************************************************
#
#  NAME: d$scsicomp
#
#  PURPOSE:
#       Completion routine for SCSI IO MRP commands
#
#  DESCRIPTION:
#       After the SCSI IO command completes successfully, d$scsiio_wait
#       is awaken to handle is sent and After normal processing
#       and cleanup, the define process is readied.
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
d$scsicomp:
#
# --- Signal d$scsiio_wait
#
        ldconst 0,r4                    # Zero timeout PCB
        ld      il_w3(g1),r5            # Load d$scsiio_wait PCB
        st      r4,il_w3(g1)            # Signal d$scsiio_wait of success
        ld      il_w0(g1),r3            # r3=prp
#
# --- Signal d$scsiio that op was successful
#
        st      r4,pr_timeout(r3)
#
# --- Wake up d$scsiio_wait
#
        ldconst pcrdy,r3                # Set timeout pc_stat to pcrdy
.ifdef HISTORY_KEEP
c CT_history_pcb("d$scsicomp setting ready pcb", r5);
.endif  # HISTORY_KEEP
        stob    r3,pc_stat(r5)
#
        ret
#
#**********************************************************************
#
#  NAME: d$scsierrorcomp
#
#  PURPOSE:
#       Completion routine for timed-out SCSI cmds
#
#  DESCRIPTION:
#       This completion routine is only used if a SCSI I/O op timed
#       out. Simply clean up the PRP and die.
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
d$scsierrorcomp:
#
# --- Get the PRP
#
        ld      il_w0(g1),g2            # Get PRP
        cmpobe  0,g2,.dsec10            # Jif no PRP
#
# --- Log error message
#
        mov     g0,r12                  # Save g0
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst mlescsitimeout,r3       # Event code
        st      r3,mle_event(g0)        # Store as word to clear other bytes
#
        ldob    pr_channel(g2),r4       # Set up port number
        stob    r4,est_port(g0)
        ldconst est_flags_comp,r4       # Set up the completion function flag value
        stob    r4,est_flags(g0)
        ldos    pr_lun(g2),r4           # Set up LUN
        stos    r4,est_lun(g0)
        ld      pr_id(g2),r4            # Set up ID
        st      r4,est_id(g0)
#
        ld      pr_dev(g2),g3           # get dev ptr
        ldl     dv_wwn(g3),r4           # Set up node WWN
        stl     r4,est_wwn(g0)
#
        ldq     pr_cmd(g2),r4           # Set up SCSI command
        stq     r4,est_cdb(g0)
#
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], estlen);
#
# --- Release the PRP
#
.ifdef M4_DEBUG_PRP
c CT_history_printf("%s%s:%u put_prp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_PRP
c       put_prp(g2);                    # Release PRP
#
.dsec10:
        lda     -ILTBIAS(g1),g1         # bump to 0 level.
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate the locally allocated ILT
        mov     r12,g0                  # Restore g0
        ret                             # Die
#
#**********************************************************************
#
#  NAME: d$scsiio_wait
#
#  PURPOSE:
#       To provide a time-out for the SCSI command MRP.
#
#  DESCRIPTION:
#       This routine is forked from d$scsiio just before the SCSI cmd
#       is issued. If the SCSI cmd completes within the timeout, then
#       this process is killed. If the timeout happens before the SCSI
#       cmd completes, then the completion routine is changed to
#       d$scsierrorcomp in case the cmd ever does complete. Then an
#       error message is logged and the define process is readied.
#
#  INPUT:
#       g2 = PRP
# -- Can't use g3, might be freed --      g3 = PDD
#       g4 = ILT
#       g5 = Timeout (in seconds)
#       g6 = Original ILT
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
d$scsiio_wait:
        mov     g0,r8                   # Save g0
#
# --- Make sure the io did not complete before we were able to run
#
        ld     il_w3(g4),r6             # Jif SCSI op completed successfully
        cmpobe 0,r6,.siw20
#
# --- Call K$twait for timeout
#
        ldconst 1000,g0                 # wait for 1 second to check whether the scsi IO is complete
        mulo    g0,g5,g0                # Multiply secs by 1000
        call    K$twait                 # Wait in msecs
#
# --- Wake up point - Either by twait complete or woken by d$scsicomp
#
        ld     il_w3(g4),r6             # Jif SCSI op completed successfully
        cmpobe 0,r6,.siw20
#
# --- SCSI command timed out. If it had not, this process would have
#     been killed. Change completion routine in ILT to d$scsierrorcomp.
#
        lda     d$scsierrorcomp,r3      # r3 = error comp routine
        st      r3,il_cr(g4)
#
# --- Log error message
#
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst mlescsitimeout,r3       # Event code
        st      r3,mle_event(g0)        # Store as word to clear other bytes
#
        ldob    pr_channel(g2),r4       # Set up port number
        stob    r4,est_port(g0)
        ldconst est_flags_timeout,r4    # Set up the timeout watchdog flag value
        stob    r4,est_flags(g0)
        ldos    pr_lun(g2),r4           # Set up LUN
        stos    r4,est_lun(g0)
        ld      pr_id(g2),r4            # Set up ID
        st      r4,est_id(g0)
#
c       r4 = 0;                         # Set WWN to zero, if bad pointers.
c       r5 = 0;
        ld      pr_dev(g2),r3           # Get DEV pointer.
        cmpobe  0,r3,.siw10             # Jif bad pointer.
        ld      dv_pdd(r3),r3           # Get PDD pointer.
        cmpobe  0,r3,.siw10             # Jif bad pointer.
        ldl     pd_wwn(r3),r4           # Get WWN from PDD.
.siw10:
        stl     r4,est_wwn(g0)          # Store WNN (or zero).
#
        ldq     pr_cmd(g2),r4           # Set up SCSI command
        stq     r4,est_cdb(g0)
#
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], estlen);
        b       .siw100
#
.siw20:
#
# --- Copy  everything back to the original ILT
#
        lda    -ILTBIAS(g6),g6          # bump to 0 level for original ILT
        lda    -ILTBIAS(g4),g4          # bump to 0 level for Local ILT
c       memcpy ((void*)g6, (void*)g4, ILT_SIZE);
#
# --- Release the local ILT.
#
        mov     g4,g1                   # now g4 has ILT at 0 level
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate the ILT allocated for this task
#
.siw100:
#
# --- Wake up d$scsiio
#
c       TaskReadyByState(pcscsiiowait); # Ready d$scsiio wait
#
        mov     r8,g0                   # Restore g0
        ret
#
#**********************************************************************
#
#  NAME: d$vdctl
#
#  PURPOSE:
#       To provide a means of processing virtual drive control requests
#       issued by the CCB.
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = status          [b]
#       g2 = return pkt size [l]
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
d$vdctl:
        mov     sp,r3                   # allocate stack frame
        lda     64(sp),sp
        stq     g0,(r3)                 # save g0-g3
        stq     g4,16(r3)               # save g4-g7
        stq     g8,32(r3)               # save g8-g11
        stt     g12,48(r3)              # save g12-g14
#
        ld      mr_ptr(g0),g0           # Get parm block ptr
        ldconst 0,g5                    # clear out the g5 register GEORAID
#
        ldos    mvc_svid(g0),r6         # Get the source VID
#
        ldconst 0xc300,r4               # Mask for control bits
        and     r4,r6,r5                # Get the two control bits
#
        ldconst 0x4000,r4               # control group "start sequence"
        cmpobe  r4,r5,.vc55             # Jif start sequence packet
#
        ldconst 0x4100,r4               # Constant for "execute sequence"
        cmpobe  r4,r5,.vc57             # Jif execute sequence packet
#
        ldconst 0x4200,r4               # Constant for "cancel sequence"
        cmpobe  r4,r5,.vc58             # Jif cancel sequence packet
#
# --- Validate subtype code
#
        ldob    mvc_subtype(g0),r12     # r12 = subtype code
        lda     deinvop,g1              # Prep possible error code
        cmpobe  mvcslink,r12,.vc40      # Jif create snapshot
        cmpobl  mvcabortcopy,r12,.vc_exit# Jif invalid
        cmpobg  mvcswapvd,r12,.vc70     # Jif move or start operation
#
# --- Process swap, break, pause, resume, abort directives.
#
#     Validate VID
#
        ldos    mvc_dvid(g0),r7         # Get the destination VID
        ldconst deinvvirtid,g1          # Prep possible error code
        ldconst MAXVIRTUALS,r3          # Check for over max value
        cmpobge r7,r3,.vc_exit          # Jif over max VID
#
# --- find and validate associated VDD from VID
#
        lda     V_vddindx,r14           # Get the VDX table
        ld      vx_vdd(r14)[r7*4],r10   # Get VDD
        cmpobe  0,r10,.vc_exit          # Jif NULL pointer
#
# --- prevent raid swap to destination alink/apool
#
        ldos    vd_attr(r10),r4         # r13 = VDisk attributes
        bbc     vdbasync,r4,.vc_40      # Jif !asynch
        ldconst deinvop,g1              # Prep possible error code
        cmpobe  mvcswapvd,r12,.vc_exit  # Jif swap RAIDs directive (not allowed for async dest)
#
.vc_40:
#
# --- determine the state of the copy session
#
        ld      vd_dcd(r10),r4          # r4 = DCD
        cmpobe.f 0,r4,.vc_exit          # Jif no active copy session
#
        ld      dcd_cor(r4),g3          # g3 = COR
        cmpobe.f 0,g3,.vc_exit          # Jif no active copy session
#
# --- determine if this is a normal control command or a sequence
#     command.
#
        mov     0,r8                    # default to indicate 0 entry
        ldconst 0xffff,r4               # set mask
#        ldos    mvc_svid(g0),r6         # Get the source VID -- still set from above.
        cmpobe  r4,r6,.vc57_Process     # Jif a normal control command
#
# --- This appears to be a sequence control command(sequence entry)
# --- Try to process it.
#
        ldconst 15,r4                   # Bit set for "sequence entry"
        bbc     r4,r5,.vc57_Process     # Jif not sequence entry
                                        #     process normally
#****************************************
# --- Process sequence entry packet
#
.vc_41:
        ldconst deinvop,g1              # prep possible error code
        ld      d_brgp_exec,r5
        cmpobe  TRUE,r5,.vc_exit        # Jif already executed
#
        ld      d_brgp_base,r5          # Make sure a sequence table is defined
        cmpobe  0,r5,.vc_exit           # Jif none allocated
#
        ld      d_brgp_cur,r8           # Get current count entered
        ld      d_brgp_count,r4         # Get the count already requested
        ldconst deinstable,g1           # Prep no table space error code
        cmpobe  r8,r4,.vc_exit          # Jif counts match(requested entries
                                        #                  already entered)
        stos    r7,brgp_did(r5)[r8*8]   # save the destination VID
                                        # NOTE: changes in the brpg entry
                                        #       structure will require changes in
                                        #       the multiplier of this instruction
c       r6 = r6 & brgp_mask_lower;      # Lower bits of source VID only.
        stos    r6,brgp_sid(r5)[r8*8]   # Save the source VID
                                        # NOTE: changes in the brpg entry
                                        #       structure will require changes in
                                        #       the multiplier of this instruction
        stob    r12,brgp_op(r5)[r8*8]   # Save operation to be performed
#
# --- Make sure the entry is valid in that the destination Vdisk has a copy
#     that is in mirror state.
#
        cmpobe  mvcslink,r12,.skipchk
        ldconst deinvop,g1              # prep possible error code
        ldob    cor_cstate(g3),r4       # Get the state of the copy
        cmpobne corcst_mirror,r4,.vc_exit # Jif not in a mirrored state
.skipchk:
        addo    1,r8,r8                 # increment current count
        st      r8,d_brgp_cur           # and save it
        ldconst deok,g1                 # return successful status
        b       .vc_exit                # exit
#
# --- Process create snapshot packet.
#
.vc40:
        ldos    mvc_dvid(g0),r7         # Get the destination VID
        ldconst brgp_sequence,r4        # Is bit set for "sequence entry"
        bbs     r4,r5,.vc_41            # Yes, so branch back to sequence setup
        call    d$createslink
#
        cmpobne  deok,g1,.vc_exit
        ldconst SS_CREATE_GOOD,r5       # Sub event code
        ldconst log_as_info,r8          # Severity - will be converted to EVENT
c       logSPOOLevent((UINT8)r5,(UINT8)r8, (UINT32)g1, (UINT16)r6, (UINT16)r7);
        b       .vc_exit
#
# --- Process start sequence packet
#
.vc55:
        ldconst deinvopt,g1             # prep possible error code
#
        ldconst 0x3fff,r7               # Mask for control bits
        and     r7,r6,r6                # Throw them away
#
        cmpobe  0,r6,.vc_exit           # Jif no entries  is specified
#
# ---   Logic to handle Batch snapshot count, return error if the requested
#       count + the existing snapshot count exceeds the max snapshot count.
#
        ldob    mvc_subtype(g0),r12     # r12 = subtype code
        cmpobne mvcslink,r12,.vc55c     # Jif not snapshot
#
        ldconst demaxsnapshots,g1
#
        ldconst 0,r3                    # r3=RID
        ldconst 0,r7                    # count of snapshots (used for MAX detection)
.vc55_1:
        ld      R_rddindx[r3*4],r11     # Get RDD pointer (r5)
        cmpobe  0,r11,.vc55_2           # Jif undefined
#
        ldob    rd_type(r11),r11
        cmpobne rdslinkdev,r11,.vc55_2
        addo    1,r7,r7                 # increment snapshot count
.vc55_2:
        addo    1,r3,r3                 # Increment RID
        cmpobne MAXRAIDS,r3,.vc55_1     # Jif more RIDs
#
        addo    r6,r7,r7                # add the current slink count to requested count
        ldos    snapshot_max_cnt,r3     # at max active snapshot count?
        cmpobg  r7,r3,.vc_exit          # ..yes, so error out
        b       .vc56s
.vc55c:
        ldconst deinstable,g1           # Prep no table space error code
        cmpoble.f MAX_CORS,r6,.vc_exit  # Jif invalid count (too large)
.vc56s:
        st      r6,d_brgp_count         # save requested count
        mov     0,r3
        st      r3,d_brgp_cur           # Clear the current index counter
#
        ld      d_brgp_base,r5          # Base addr for sequence list
        cmpobne  0,r5,.vc55b            # Jif list already allocated
#
# --- Allocate a sequence table
#
        ldconst MAX_CORS,r7             # max number of entries
        mulo    brgp_seq_siz,r7,g0      # Size of table in bytes
c       g0 = s_MallocC(g0, __FILE__, __LINE__);
        st      g0,d_brgp_base          # Update the base pointer
#
# --- clear the execution flag and prepare a good status
#
.vc55b:
        ldconst FALSE,r7
        st      r7,d_brgp_exec          # Clear executed flag
        ldconst deok,g1                 # return successful status
        b       .vc_exit
#
# --- Process execute sequence packet
#
.vc57:
        ldconst deinvop,g1              # prep possible error code
        ld      d_brgp_exec,r5
        ldconst TRUE,r3                 # r3 = execution
        cmpobe  r3,r5,.vc_exit          # Jif already executed
#
        st      r3,d_brgp_exec          # Set the executed flag
        ld      d_brgp_base,r13         # Make sure a sequence table is defined
        cmpobe  0,r13,.vc_exit          # Jif none allocated
#
# --- determine if correct number of entries have been received
#
        ld      d_brgp_count,r3         # Have all sequences been received?
        ld      d_brgp_cur,r8
        cmpobe  0,r8,.vc_exit           # Jif no entries to process
        cmpobne r8,r3,.vc_exit          # Jif counts don't match
#
# --- execution loop
#
        subo    1,r8,r8                 # Adjust the count for base 0 table
#
.vc57_loop:
        ldos    brgp_did(r13)[r8*8],r4  # r7 = destination vid
                                        # NOTE: changes in the brpg entry
                                        #       structure will require changes in
                                        #       the multiplier of this instruction
        ldob    brgp_op(r13)[r8*8],r12  # Operation to be performed
        mov     r4,r9
        cmpobne mvcslink,r12,.not_ss
        stos    r4,mvc_dvid(g0)
        ldos    brgp_sid(r13)[r8*8],r4  # r4 = source vid
                                        # NOTE: changes in the brpg entry
                                        #       structure will require changes in
                                        #       the multiplier of this instruction
        stos    r4,mvc_svid(g0)
        call    d$createslink
        mov     g0,r12
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        cmpobne  deok,g1,.vc57_a
        ldconst mleSPOOL_CHANGE_I,r3    # Event code
        ldconst SS_CREATE_GOOD,r5
        b       .vc57_b
#
.vc57_a:
        ldconst mleSPOOL_CHANGE_E,r3    # Error Event code
        ldconst SS_CREATE_FAIL,r5
.vc57_b:
        st      r3,mle_event(g0)        # Store as word to clear other bytes
        stob    r5,esp_sub_event_code(g0)
#
        ldconst 0,r5
        stob    r5,esp_skip_notify(g0)
        st      g1,esp_errorCode(g0)
        st      r4,esp_value1(g0)
        st      r9,esp_value2(g0)
#
c       MSC_LogMessageStack(&TmpStackMessage[0], esplen);
        mov     r12,g0
        cmpobne deok,g1,.vc_exit
        b       .vc57_Lcheck
#
.not_ss:
        ld      V_vddindx[r4*4],r4      # Get VDD
#
        ldconst 1,r10                   # Op.failure reason = Invalid VDD
        cmpobe  0,r4,.vc57_Lcheck1      # Jif invalid VDD
#
        ldconst 2,r10                   # Op.failure reason = Invalid DCD
        ld      vd_dcd(r4),r4           # r4 = DCD
        cmpobe  0,r4,.vc57_Lcheck1      # Jif no active copy session to next entry
#
        ldconst 3,r10                   # Op.Failure reason = Invalid COR
        ld      dcd_cor(r4),g3          # r3 = COR
        cmpobe 0,g3,.vc57_Lcheck1       # Jif no cor defined to next entry
#
        ldconst 4,r10                   # Op.Failure reason = Not in mirror state
        ldob    cor_cstate(g3),r3       # Get the state of the copy
        cmpobne corcst_mirror,r3,.vc57_Lcheck1 # Jif not in a mirrored state, to next entry
#
# --- process control command
#
.vc57_Process:
        cmpobne mvcpausecopy,r12,.vc57f  # Jif not pause copy directive
        call    CCSM$pause_copy          # generate pause copy event to CCSM
        b       .vc57_Lcheck
#
.vc57f:
        cmpobne mvcresumecopy,r12,.vc57g # Jif not resume copy directive
        call    CCSM$resume_copy         # generate resume copy event to CCSM
        b       .vc57_Lcheck
#
.vc57g:
        cmpobne mvcswapvd,r12,.vc57h     # Jif not swap RAIDs directive
        mov     0,g0                     # g0 = swap RAIDs type code
        ldconst TRUE,r3
        stob    r3,cor_userswap(g3)      # Indicate this as User requested swap
#
        call    CCSM$swap_raids          # generate swap RAIDs event to CCSM
        b       .vc57_Lcheck
#
.vc57h:
        call    CCSM$term_copy           # generate terminate copy event to
        b       .vc57_Lcheck
#
# --- determine if there are any more entries in the table
#
.vc57_Lcheck1:
        ldconst defailed,g1              # Set to failed, even if specified operation
                                         # for single entry  is failed.
c fprintf(stderr,"%s%s:%u <QuickMirrorPauseBreakResume>Operation failed DestVid=%lx Reason=%lx\n", FEBEMESSAGE, __FILE__, __LINE__,r9,r10);
        b       .vc_exit
#
.vc57_Lcheck:
        ldconst deok,g1                  # Record it as OK
        cmpobe  0,r8,.vc_exit            # Jif no entries to process
#
        subo    1,r8,r8                  # Decrement the entry counter
        b       .vc57_loop               # More entries to process
#
# --- Process cancel sequence packet
# --- And also release the memory
#
.vc58:
        ldconst TRUE,r3                 # r3 = execution
        st      r3,d_brgp_exec          # Set the executed flag
#
        ld      d_brgp_base,g0          # Load buffer ptr
        cmpobe  0,g0,.vc58a             # Jif none allocated yet or
                                        # already freed
        ldconst MAX_CORS,r7             # Max number of entries
        mulo    brgp_seq_siz,r7,g1      # Size of table in bytes
c       s_Free(g0, g1, __FILE__, __LINE__); # Free buffer
        ldconst 0,r7
        st      r7,d_brgp_base          # Null the buffer pointer
.vc58a:
        ldconst deok,g1                 # Record it as OK
        b       .vc_exit
#
# --- Move, copy, copy swap, and mirror operation logic
#        r12 = 00 = move VD
#              01 = copy and break
#              02 = copy, swap, and break
#              03 = copy and mirror
#
.vc70:
##     The commands validating here are MVCMOVEVD, MVCCOPYBRK,
##     MVCCOPYSWAP, MVCCOPYCONT
#
# --- Validate the rest of the parameters
#
# --- Check if requesting more than maximum number of copies supported
#
        ldconst demaxcopies,g1          # max copies error
        ldos    cm_cor_act_cnt,r15      # active cor count
        cmpobe  MAX_CORS,r15,.vc_exit   # jif reached max copies
#
        ldos    mvc_svid(g0),r5         # r5 = source vid
        ldos    mvc_dvid(g0),r7         # r7 = destination vid
        lda     V_vddindx,r15           # Load VDX pointer
#
# --- Check if user requesting copy of a VD onto itself
#
        ldconst dedevused,g1            # Prep possible error code
        cmpobe  r5,r7,.vc_exit          # Jif VID's match
#
        lda     MAXVIRTUALS,r3          # Get max number for VID
        lda     deinvvirtid,g1          # Prep possible error code
        cmpobge r5,r3,.vc_exit          # Jif invalid source vid
        cmpobge r7,r3,.vc_exit          # Jif invalid dest. vid
#
        lda     V_vddindx,r4            # Get VDX pointer
        lda     deinvvid,g1             # Prep possible error code
        ld      vx_vdd(r4)[r5*4],r9     # r9 = source VDD
        cmpobe  0,r9,.vc_exit           # Jif source vid not defined
#
        ld      vx_vdd(r4)[r7*4],r11    # r11 = dest. VDD
#
# --- If this is a move operation we don't need to check the source and
# --- destination for caching/vlinks compatibility.
#
        cmpobe  mvcmovevd,r12,.vc90     # Jif move vdisk directive
#
# --- Check if the destination is valid
#
        cmpobe  0,r11,.vc_exit          # Jif destination vid not defined
#
# --- allow only a mirror operation to an ALink and prevent ALL ops FROM an ALink
# --- also don't allow ANY operation 'to' an APOOL, but allow all except swaps to !R10 from Apool
# --- allow only a copyswap from an SPOOL or SS, NO mirror/copy/swap opps to an SPOOL or SS
#
        ldconst deinvop,g1              # Prep possible error code
        ld      vd_rdd(r9),r4           # r4 = Non-deferred RAID assigned to src VID
        # first check to see if this is an SLINK (if so then no can do)
        ldob    rd_type(r4),r3          # r3 = RAID type
        cmpobne rdslinkdev,r3,.vc_70c   # Jif not an SS (slink)
        cmpobne mvccopyswap,r12,.vc_exit # (don't allow copy or mirror but do allow swap)
.vc_70c:                                # wasn't an SS either so continue
        ldos    vd_attr(r9),r4          # r4 = VDisk attributes for source vdisk
        bbc     vdbasync,r4,.vc_72      # if source not asynch then continue
        bbc     vdbvlink,r4,.vc_71      # if src=APOOL, then Copy/mirror/swap is ok so continue
        b       .vc_exit                # else it is ALINK...don't allow ANY copy/mirror/swap from ALINK
#
.vc_71:
        ldos    vd_attr(r11),r4         # r4 = VDisk attributes for dest VDisk
        bbs     vdbvlink,r4,.vc_exit    # Don't do any operation from Apool to vlink or alink
                                        # ---check to make sure swap is to R10 only
        cmpobne mvccopyswap,r12,.vc_72  # if not swap to dest then OK
        ldconst deinvrtype,g1           # Prep possible error code
        ld      vd_rdd(r11),r4          # r4 = Non-deferred RAIDs assigned VID
.vc_71a:
        cmpobe  0,r4,.vc_72             # Jif done checking RAIDs
        ldob    rd_type(r4),r3          # r5 = RAID type
        cmpobe  rdraid5,r3,.vc_exit     # Jif RAID 5
        ld      rd_nvrdd(r4),r4         # Advance to the next RAID
        b       .vc_71a
#
.vc_72:                                 # ---now check the destination rules
        ld      vd_rdd(r11),r4          # r4 = Non-deferred RAID assigned to dest VID
        # first check to see if this is an SLINK (if so then no can do)
        ldob    rd_type(r4),r3          # r3 = RAID type
        cmpobe  rdslinkdev,r3,.vc_exit  # Jif an SS (slink)
        ld      vd_outssms(r11),r4      # is the destination vdisk the source of a SS?
        cmpobne 0,r4,.vc_exit           # ...then don't allow a mirror/copy/swap operation to it
        # now check to check for alink and snappool overrides
        ldos    vd_attr(r11),r4         # r4 = VDisk attributes for dest VDisk
        bbs     vdbspool,r4,.vc_exit    # if dest is spool, the error exit out
        bbc     vdbasync,r4,.vc_74      # if dest not asynch then continue
        cmpobe  mvccopybrk,r12,.vc_exit # if copy to dest alink/apool (not allowed for async)
        cmpobe  mvccopyswap,r12,.vc_exit # if swap to dest alink/apool (not allowed for async)
        bbs     vdbvlink,r4,.vc_74      # else it is mirror...if ALINK: mirror TO ALINK is ok
        b       .vc_exit                # else it is APOOL...don't allow ANY mirror to APOOL
#
.vc_74:
#
# --- Validate source and destination meet the requirements for
# --- caching and vlinks (can't combine two when one has caching and
# --- the other has vlinks).
#
        PushRegs(r3)
        mov     r5,g0                   # Move source VID to g0
        call    DEF_GetVDiskAndCopyTreeAttrs  # Get the source attributes
        mov     g0,r4                   # Save the returned attributes
        mov     r7,g0                   # Move destination VID to g0
        call    DEF_GetVDiskAndCopyTreeAttrs  # Get the destination attributes
        PopRegs(r3)
#
# --- At this point r4 contains the source attributes and g0 contains
# --- the destination attributes. If caching is disabled or there
# --- are no vlinks or the source is an ALink, then the operation can
# --- continue, otherwise this is an invalid operation.
#
        ldconst deinvopt,g1             # Prep possible error codee
.ifndef MODEL_7000
.ifndef MODEL_4700
#        bbs     vdbasync,g0,.vc90       # jif dest is an alink (i.e. src can be wcached)
        or      r4,g0,r4                # Combine src and dest bits
        bbc     vdbcacheen,r4,.vc90     # Jif cache bit not set
        bbs     vdbvlink,r4,.vc_exit    # Jif cache and vlink bits set
.endif  # MODEL_4700
.endif  # MODEL_7000
#
.vc90:
        cmpobe  mvccopybrk,r12,.vc2000  # Jif copy, mirror, break directive
        cmpobe  mvccopyswap,r12,.vc1400 # Jif copy, swap, break directive
        cmpobe  mvccopycont,r12,.vc2000 # Jif copy, mirror directive
#
# --- Move virtual disk ID directive.
#
#     The assumption here is that the virtual disk is unmapped
#     and inactive in all ways. Therefore all that is needed is
#     to replace the VID number in the associated RAIDs and move
#     the VDD in the tables.
#
# --- Check if either the source or destination is in a copy. If so, do
# --- not allow the move to occur.
#
        cmpobne 0,r11,.vc_exit          # Jif destination device in use
#
# --- Check if the specified source virtual device is associated with
#       a copy and if so, deny the request.
#
# --- don't allow move of an ALink or APool
#
        ldos    vd_attr(r9),r4          # r4 = VDisk attributes
        bbs     vdbspool,r4,.vc_91a     # Jif snappool
        bbc     vdbasync,r4,.vc_92      # Jif !asynch
.vc_91a:
        ldconst deinvop,g1              # Prep possible error code
        b       .vc_exit                # exit out on an attempt to move an APOOL or ALINK
#
.vc_92:
        ldconst dedevused,g1            # g1 = possible error code
#
        ld      vd_scdhead(r9),r4       # r4 = first SCD assoc. with VDD
        cmpobne 0,r4,.vc_exit           # Jif VDD is SCD
        ld      vd_dcd(r9),r4           # r4 = DCD assoc. with VDD
        cmpobne 0,r4,.vc_exit           # Jif VDD is DCD
#
        ld      vd_vlinks(r9),r4        # r4 = assoc. VLAR if VLink defined
        cmpobne.f 0,r4,.vc_exit         # Jif VLink defined to this VDisk
#
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        ldconst 0,r3
        st      r3,V_vddindx[r5*4]      # Zero out the old VDD in the tables
        st      r9,V_vddindx[r7*4]      # Put it in the new spot
#
        ld      vd_rdd(r9),g4           # Get the RDD of the first RAID
        ldob    rd_type(g4),r13         # r13 = RAID type code
        cmpobne.t rdlinkdev,r13,.vc100  # Jif not linked device type RAID
#
# --- VLink being moved. Need to move VLink lock on destination MAG.
#
        mov     r7,g3                   # g3 = new VLink #
        mov     r5,g6                   # g6 = current owner VLink #
        ld      K_ficb,r13              # Get system serial number
        ld      fi_vcgid(r13),g7        # g7 = my VCG serial #
        call    DLM$VLmove              # start VLink move process
#
# --- Find all of the RAIDs and replace the old VID with the new one.
#
.vc100:
        stos    r7,vd_vid(r9)           # Replace the VID in the VDD
        ld      vd_rdd(r9),r3           # Get the RDD of the first RAID
#
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */

.vc110:
        stos    r7,rd_vid(r3)           # Set the VID
        ld      rd_nvrdd(r3),r3         # Get the next one
        cmpobne 0,r3,.vc110             # Another one, go set it
#
        ld      vd_drdd(r9),r3          # Get any deferred RAIDs
#
.vc115:
        cmpobe  0,r3,.vc120             # Done if NULL
        stos    r7,rd_vid(r3)           # Set the VID
        ld      rd_nvrdd(r3),r3         # Get the next one
        b       .vc115                  # Check for NULL
#
# --- Update the FEP with the deletion of the old VID and the addition
# --- of the new one.
#
.vc120:
        PushRegs(r3)
        mov     r5,g0
        call    DL_ClearVIDLUNMap       # Clear all mappings
        PopRegs(r3)
#
        mov     r5,g0                   # Deletion
        ldconst TRUE,g1                 # Indicate deletion
        call    D$updrmtcachesingle     # Update FEP
#
        mov     r7,g0                   # Deletion
        ldconst FALSE,g1                # Indicate addition
        call    D$updrmtcachesingle     # Update FEP
#
        ldconst deok,g1                 # Prep return code
#
# --- Modify all copies, that have this Vdisk as their source, to reflect
#     the change in configuration.
#
        ldob    vd_vid(r9),r3           # r3 = new VDisk # of VDD (LS)
        ldob    vd_vid+1(r9),r4         # r4 = new VDisk # of VDD (MS)
        ld      vd_scdhead(r9),r10      # r10 = first SCD assoc. with VDD
        cmpobe.t 0,r10,.vc140           # Jif no SCDs assoc. with VDD
#
.vc120a:
        ld      scd_cor(r10),g3         # g3 = assoc. COR address
        ldob    scd_type(r10),r12       # r12 = SCD type code
        cmpobne.t scdt_remote,r12,.vc120d # Jif not remote source copy device
        stob    r4,cor_rscl(g3)         # save new cluster # in COR
        stob    r3,cor_rsvd(g3)         # save new VDisk # in COR
#
# --- Send copy device moved datagram to copy manager.
#
        ld      cor_rcsn(g3),g0         # g0 = copy MAG serial #
        call    CM$pkop_dmove           # pack a copy device moved datagram
                                        # g1 = Copy Device Moved datagram ILT at nest level 2
        ldconst 4,g0                    # g0 = error retry count
        call    DLM$just_senddg         # send datagram to copy MAG
        b       .vc120z                 # and check for more copy ops. on list
#
# --- VDD is either local source copy device or both source copy device.
#
.vc120d:
        ld      cor_destvdd(g3),g4      # g4 = dest. copy device VDD
        cmpobe.f 0,g4,.vc120f           # Jif no dest. VDD assoc. with COR
        ldos    vd_vid(r9),r13          # r13 = new VDisk # of VDD (MS & LS)
        stos    r13,vd_scorvid(g4)      # save virtual ID in dest. copy
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
                                        #  device VDD
.vc120f:
        stob    r4,cor_rcscl(g3)        # save new cluster # in COR
        stob    r3,cor_rcsvd(g3)        # save new VDisk # in COR
        cmpobe.t scdt_local,r12,.vc120h # Jif local source copy device
#
# --- Source copy device is both local and remote
#
        stob    r4,cor_rscl(g3)         # save new cluster # in COR
        stob    r3,cor_rsvd(g3)         # save new VDisk #
.vc120h:
        ld      cor_rcsn(g3),r5         # r5 = copy MAG serial #
        ld      cor_rssn(g3),r6         # r6 = source MAG serial #
        cmpobe.t r5,r6,.vc120m          # Jif copy and source MAG the same
.vc120j:
        ldconst 25,r5                   # r5 = copy op. poll timer to set
        stos    r5,cor_tmr1(g3)         # set local poll timer to poll in
                                        #  5 secs. to allow VLink registration
                                        #  to occur before changing the copy
                                        #  registration on the remote nodes
        b       .vc120z
#
.vc120m:
        ld      cor_rdsn(g3),r6         # r6 = dest. MAG serial #
        cmpobne.f r5,r6,.vc120j         # Jif copy and dest. MAG different
.vc120z:
        ld      scd_link(r10),r10       # r10 = next SCD on list
        cmpobne.f 0,r10,.vc120a         # Jif more copy ops. on list
#
# --- Modify all copies, that have this Vdisk as their destination,
#     to reflect the change in configuration.
#
.vc140:
        ld      vd_dcd(r9),r10          # r10 = assoc. DCD address
        cmpobe.t 0,r10,.vc150           # Jif no DCD assoc. with VDD
        ld      dcd_cor(r10),g3         # g3 = assoc. COR address
        ldob    dcd_type(r10),r12       # r12 = DCD type code
        cmpobne.t dcdt_remote,r12,.vc140d # Jif not remote dest. copy device
        stob    r4,cor_rdcl(g3)         # save new cluster # in COR
        stob    r3,cor_rdvd(g3)         # save new VDisk # in COR
#
# --- Send copy device moved datagram to copy manager.
#
        ld      cor_rcsn(g3),g0         # g0 = copy MAG serial #
        call    CM$pkop_dmove           # pack a copy device moved datagram
                                        # g1 = Copy Device Moved datagram ILT
                                        #      at nest level 2
        ldconst 4,g0                    # g0 = error retry count
        call    DLM$just_senddg         # send datagram to copy MAG
        b       .vc150
#
.vc140d:
#
# --- VDD is either local dest. copy device or both dest. copy device.
#
        stob    r4,cor_rcdcl(g3)        # save new cluster # in COR
        stob    r3,cor_rcdvd(g3)        # save new VDisk # in COR
        cmpobe.t dcdt_local,r12,.vc140h # Jif local dest. copy device
#
# --- Destination copy device is both local and remote
#
        stob    r4,cor_rdcl(g3)         # save new cluster # in COR
        stob    r3,cor_rdvd(g3)         # save new VDisk #
.vc140h:
        ld      cor_rcsn(g3),r5         # r5 = copy MAG serial #
        ld      cor_rssn(g3),r6         # r6 = source MAG serial #
        cmpobe.t r5,r6,.vc140m          # Jif copy and source MAG the same
.vc140j:
        ldconst 25,r5                   # r5 = copy op. poll timer to set
        stos    r5,cor_tmr1(g3)         # set local poll timer to poll in
                                        #  5 secs. to allow VLink registration
                                        #  to occur before changing the copy
                                        #  registration on the remote nodes
        b       .vc150
#
.vc140m:
        ld      cor_rdsn(g3),r6         # r6 = dest. MAG serial #
        cmpobne.f r5,r6,.vc140j         # Jif copy and dest. MAG different
#
.vc150:
        call    D$p2updateconfig        # Update the configuration
        lda     deok,g1                 # return successful status
        b       .vc_exit                # exit
#
# --- Copy & Swap directive
#
.vc1400:
        ld      vd_vlinks(r9),r4        # r12 = VLink assoc. records
        cmpobe.t 0,r4,.vc2000           # Jif no VLink records assoc. with
                                        #     this device
        ldos    vd_attr(r11),r5         # r13 = VDisk attributes
        ldconst vdvlink,r4              # r12 = VLink flag bit
        and     r4,r5,r5                # r13 = VLink flag bit
        cmpobne r4,r5,.vc2000           # Jif dest. VDD is not a VLink
        ldconst deinvop,g1              # g1 = error code
        b       .vc_exit                # and return error to user
#
# --- Common routine for copy/mirror, copy/break, and
#     copy/swap/break directives
#
#       r9  = source VDD
#       r11 = destination VDD
#       r12 = subtype code
#               01 = copy&break
#               02 = copy&swap
#               03 = continuous copy
#
.vc2000:
#
# --- validate Vdisk sizes
#
c   if (((VDD *)r11)->devCap < ((VDD *)r9)->devCap) {
        lda     deinsdevcap,g1          # Prep error code
        b       .vc_exit                # Jif dest. VDD < source VDD
c   }
#
# --- determine if copy is allowed
#
        lda     dedevused,g1            # Prep possible error code
        ld      vd_dcd(r11),r3          # check if dest. VDD is already
                                        #  in a sec. copy session
        cmpobne.f 0,r3,.vc_exit         # Jif true
#
.ifndef MODEL_7000
.ifndef MODEL_4700
        ldos    vd_attr(r11),r4         # r4 = Dest VDisk attributes
        bbs     vdbasync,r4,.vc2150     # Jif async
        ld      vd_vlinks(r11),r3       # r12 = dest. VDD assoc. VLinks
                                        #  record
        cmpobne 0,r3,.vc_exit           # Jif dest. VDD has a VLink assoc.
                                        #  with it
.vc2150:
.endif  # MODEL_4700
.endif  # MODEL_7000
.ifndef  MODEL_3000
.ifndef  MODEL_7400
        ld      vd_vlinks(r11),r3       # r12 = dest. VDD assoc. VLinks
                                        #  record
        cmpobne 0,r3,.vc_exit           # Jif dest. VDD has a VLink assoc.
                                        #  with it
.endif  # MODEL_7400
.endif  # MODEL_3000
        lda     deinvop,g1              # Prep with error code
        ldos    vd_attr(r11),r4         # r4 = Dest VDisk attributes
        bbc     vdbasync,r4,.vc2290     # Jif !async
        ldos    vd_attr(r9),r4          # r4 = Src VDisk attributes
        bbs     vdbdcd,r4,.vc_exit     # jif source vdisk is dest of a copy
        #
        # The following logic simply looks through every vdisk and checks to see if it is:
        #   1) an alink AND
        #   2) the destination vdisk of a copy operation that has the destination vdisk in this
        #      desired copy operation as its source VDisk (i.e. don't allow us to copy V1 to V2
        #      if V2 is already copied to an Alink.)
        # If these two conditions are met, we will NOT allow this copy operation to complete
        #
.vc2290:
        ldos    vd_vid(r11),r13         # r13 = VDisk # of destinatin VDD
        ldconst 0,r4                    # r4=VID
.vcp01:
        lda     V_vddindx,r3            # Get VDX pointer (r3)
        ld      vx_vdd(r3)[r4*4],r5     # r5 = VDD
        cmpobe  0,r5,.vcp10             # Jif undefined
        ldos    vd_attr(r5),r6          # r6 is the attributes of the vdisk
        bbc     4,r6,.vcp10             #   not a destination copy device so skip
        bbc     vdbasync,r6,.vcp10      #   not an alink (we actually do not need to worry about apool)\
        ldos    vd_scorvid(r5),r6       # is this disk being copied to from the selected dest
        cmpobe  r13,r6,.vc_exit         #   if so, exit
.vcp10:
        addo    1,r4,r4                 # Increment VID
        ldconst MAXVIRTUALS,r8
        cmpobne r8,r4,.vcp01            # Jif more VIDs
#
# if the code above makes it to here then it means no alinks were found that had a source
# vdisk equal to the desired destination vdisk of this copy request
#
#
# --- Check if the source or destination is in an expansion mode. If so, do
#     not allow the copy.
#
        ldconst deinitinprog,g1         # Prep error code
        ldob    vd_draidcnt(r9),r3      # Get source deferred RAID count
        ldob    vd_draidcnt(r11),r4     # Get destination deferred RAID count
        addo    r3,r4,r4                # Add 'em up
        cmpobne 0,r4,.vc_exit           # Error out if not zero
#
# --- Validate the operational status of the source device's
#     RAIDs
#
        ldconst denotoprid,g1           # Prep inoperable
        ldconst rdop,r5                 # Get operable value
        ld      vd_rdd(r9),r3           # Get the first RDD of source
#
.vc2300:
        ldob    rd_status(r3),r4        # Check for non-operable
        cmpobl  r4,r5,.vc_exit          # Jif less than operable
        ld      rd_nvrdd(r3),r3         # Get next one
        cmpobne 0,r3,.vc2300            # Jif not done
#
# --- Validate the operational status of the destination device's
#     RAIDs
#
        ld      vd_rdd(r11),r3          # Get the first RDD of destination
#
.vc2400:
        ldob    rd_status(r3),r4        # Check for non-operable
        cmpobl  r4,r5,.vc_exit          # Jif less than operable
        ld      rd_nvrdd(r3),r3         # Get next one
        cmpobne 0,r3,.vc2400            # Jif not done
#
# --- translate MVC directives to CM type codes
#
        ldconst cmty_mirror,g0          # g0 = copy type code to start
        cmpobe.t mvccopycont,r12,.vc2500 # Jif continuous copy operation
#
        ldconst cmty_copyswap,g0        # g0 = copy type code to start
        cmpobe.t mvccopyswap,r12,.vc2500 # Jif copy&swap operation
#
        ldconst cmty_copybreak,g0       # g0 = copy type code to start
#
# --- Everything seems ok to start the copy operation
#
.vc2500:
.if 0 # Enable this piece of code to fix SAN-126/SAN-735
#
# ---  Don't allow the GeoRaid vdisk mirror with vlink (or) Viceversa..
#
c       r4 = BIT_TEST(((VDD*)r9)->grInfo.permFlags,GR_VD_GEORAID_BIT) && BIT_TEST((((VDD*)r11)->attr), VD_BVLINK);
c       r5 = BIT_TEST((((VDD*)r9)->attr),VD_BVLINK) && BIT_TEST(((VDD*)r11)->grInfo.permFlags, GR_VD_GEORAID_BIT);
c       r4 = r4 || r5;
        cmpobe  FALSE,r4,.vc2501
        lda     deinvop,g1
c fprintf(stderr, "%s%s:%u <GR>svid=%x(%x %x) dvid=%x(%x %x)\n", FEBEMESSAGE, __FILE__, __LINE__, ((VDD*)r9)->vid, ((VDD*)r9)->grInfo.permFlags, ((VDD*)r9)->attr, ((VDD*)r11)->vid, ((VDD*)r11)->grInfo.permFlags, ((VDD*)r11)->attr);
c       GR_LogEvent(2, (VDD*)r9, (VDD*)r11);
        b       .vc_exit
.vc2501:
.endif
#
#  Check if the src vdisk server (target) association is Ok, in case of async
#  mirror/copy
        ldconst     deinvop,g1
c       r4 = apool_validate_async_copy((VDD*)r9, (VDD*)r11);
        cmpobne deok,r4,.vc_exit
#
        ldconst cmmt_sync,g3            # g3 = mirror type code to start
                                        # Note: Force synchronous type mirror
                                        #       until logic added to support
                                        #       asynchronous type mirror.
        mov     r9,g1                   # g1 = source copy device VDD address
        mov     r11,g2                  # g2 = dest. copy device VDD address
.if CM_IM_DEBUG
c       if( r12 == mvccopycont)fprintf(stderr,"<CM_IM>definebe-vc2500>Copy/mirror-cpytype code=%lx call CM$scstart..\n",g0);
.endif  # CM_IM_DEBUG
        call    CM$scstart              # start the copy operation
                                        # g0 = request status code
        mov     g0,g1                   # g1 = status code to return to CCB
        cmpobne.t mvccopycont,r12,.vc2600
        cmpobne deok,g1,.vc2600         # Jif the status is not ok
        lda     -64(sp),r3              # get stack frame
        ld       0(r3),g0               # restore MRP
        ld       mr_rptr(g0),r3         # Get the return data ptr
# Does not change any g registers - Wed Aug 22 17:14:04 CDT 2007
c       r4 = GR_SetGeoMirrorSetType((MR_PKT*)g0);
!       stob     r4,mr_mirrorsettype(r3)  # Store the partner type
.vc2600:
#
# --- Exit
#
.vc_exit:
        ldconst mvcrsiz,g2              # Set return packet size
#
        lda     -64(sp),r3              # get stack frame
        ld      0(r3),g0                # restore g0
        ld      12(r3),g3               # restore g3
        ldq     16(r3),g4               # restore g4-g7
        ldq     32(r3),g8               # restore g8-g11
        ldt     48(r3),g12              # restore g12-g14
        ret
#
#**********************************************************************
#
#  NAME: D$ctlrqst_cr
#
#  PURPOSE:
#       To provide a common means of processing the completion of a
#       control request PCP.
#
#  DESCRIPTION:
#       Return the PCP/ILT to free pool.
#
#  INPUT:
#       g1 = pcp at lvl 1
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
D$ctlrqst_cr:
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT (PCP)
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: d$uawake
#
#  PURPOSE:
#       To provide a means of responding to the awake query issued by
#       the CCB.
#
#  DESCRIPTION:
#       The MRP buffer is modified for returning the awake query
#       packet. An ok status and who=firmware are always returned.
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = status
#       g2 = packet size
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
d$uawake:
        ld      mr_rptr(g0),r14         # Get pointer into response packet
        ld      mr_ptr(g0),g0           # Get parm block
#
# --- The ii status will be modified so grab it in preparation for
# --- the setting/clearing of the master and slave bits, the nvram
# --- ready setting, and the CCB required setting.
#
        ldos    K_ii+ii_status,r4       # Get current initialization status
        mov     r4,r5                   # Copy it
#
        ldos    mpg_step(g0),r3         # Get step
#
        bbc     mpgbreplacement,r3,.ua05# Jif not a replacement
        setbit  iireplacement,r4,r4     # Indicate a replacement
#
.ua05:
        bbc     mpgbnvramrdy,r3,.ua10   # Jif not a NVRAM ready signal
        setbit  iinvramrdy,r4,r4        # Indicate NVRAM is ready
        b       .ua50                   # Jif to ignore the rest
#
.ua10:
        clrbit  iimaster,r4,r4
        clrbit  iislave,r4,r4
#
        bbc     mpgbmaster,r3,.ua20     # Jif not a master setting
        setbit  iimaster,r4,r4          # Indicate master of the VCG
#
.ua20:
        bbc     mpgbslave,r3,.ua30      # Jif not a slave setting
        setbit  iislave,r4,r4           # Indicate slave of the VCG
#
.ua30:
        clrbit  iiccbreq,r4,r4          # Assume we will turn it off
        bbc     mpgbccbreq,r3,.ua40     # Jif if one controller
        setbit  iiccbreq,r4,r4          # Indicate more than one controller present.
#
.ua40:
        bbc     mpgbp2init,r3,.ua50     # Jif if P2 init now OK
        setbit  iip2init,r4,r4          # Indicate P2 init is OK to do
#
.ua50:
        stos    r4,K_ii+ii_status       # Set initialization status
#
# --- Check for group master controller status change
#
        bbc     iimaster,r4,.ua60       # Jif not the new master
#
# --- Am the new group master controller
#       Check if I was already the group master controller
#
        bbs     iimaster,r5,.ua70       # Jif I was the old master
#
# --- Am a newly elected group master controller
#
        PushRegs(r6)                    # Save register contents
        call    DEF_iSNSUpdateFE        # notify the iSNS of the new master
        PopRegs(r6)                     # Restore registers (except g0)
        movt    g0,r8                   # save g0-g2
        ld      K_ficb,r6               # r6 = FICB address
        ld      fi_vcgid(r6),g0         # g0 = group VCG serial #
        ld      fi_cserial(r6),g2       # g2 = group master controller
                                        #      serial # (i.e. my serial #)
        st      g2,DLM_master_sn        # save my controller serial #
                                        #  as current group master controller
                                        #  serial #
        call    DLM$def_master          # tell the world the good news!!!
        call    CCSM$new_master         # notify CCSM of the new master event
        movt    r8,g0                   # restore g0-g2
        b       .ua70
#
# --- Am not the new group master controller
#
.ua60:
#
# --- Check if I used to be the group master controller
#
        bbc     iimaster,r5,.ua70       # Jif I was not the old group master
                                        #  controller
#
# --- I just got voted off the island!!!!!
#
        PushRegs(r6)                    # Save register contents
        call    DEF_iSNSUpdateFE        # notify iSNS of being voted out
        PopRegs(r6)                     # Restore registers (except g0)
        call    CCSM$not_master         # notify CCSM of being voted out of
                                        #  being the master
        ld      DLM_master_sn,r8        # r8 = current group master
                                        #  controller serial #
        ld      K_ficb,r6               # r6 = FICB address
        ld      fi_cserial(r6),r7       # r7 = my controller serial #
        cmpobne.f r7,r8,.ua70           # Jif I was already voted off the
                                        #  island!
        ldconst 0,r6
        st      r6,DLM_master_sn        # clear current group master
                                        #  controller serial # to indicate I
                                        #  don't know who's the lucky one.
.ua70:
#
# --- Inform FE if 'more than one controller' status changed
#
        bbs     iiccbreq,r4,.ua80       # Jif if new status = TRUE
        bbc     iiccbreq,r5,.ua95       # Jif if old status = FALSE - no change
        b       .ua90                   # Status changed
#
.ua80:
        bbs     iiccbreq,r5,.ua95       # Jif if old status = TRUE - no change
#
.ua90:
        call    D$updrmtsysinfo         # Update FE info
#
.ua95:
        bbc     mpgbfeinit,r3,.ua100    # Jif FE init not OK
        call    D$cmpltserverupd        # Complete the server update to FE to
                                        # let the FE QLogics go
#
# --- Setup response
#
.ua100:
        mov     deok,g1                 # Set OK status
        ldconst mpgrsiz,g2              # Return packet size
#
# --- Exit
#
        ret
#
#**********************************************************************
#
#  NAME: d$fcalanal
#
#  PURPOSE:
#       This implements the FCAL analysis MRP.
#       This is used to analyze FCAL bus problems by using read and
#       write buffer commands to transfer data between the controller
#       and a selected drive.
#
#  DESCRIPTION:
#       This will send a read or write buffer to the input PID
#       for n times or until there is an error. The number of bytes
#       transfered is input through the MRP.
#       See the mrfcalanal structure for inputs and outputs.
#       Possible error codes are deok and deioerr.
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = status
#       g2 = packet size
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
d$fcalanal:
        ld      mr_rptr(g0),r14         # Get pointer into response packet
        ld      mr_ptr(g0),r15          # Get parm block
#
        ldob    mfa_options(r15),r3
        lda     O_t_wrbuff,g0           # Pass write buffer template
        bbs     mfawrite,r3,.dfa10      # Jif the write bit is set
        lda     O_t_rdbuff,g0           # Else read buffer template
#
.dfa10:
        ld      mfa_xsize(r15),r4
        shlo    8,r4,r5                 # Clear upper nibble, r5 is for CDB
        shro    8,r5,r4
        st      r4,tpr_rqbytes(g0)      # Save xfer size in bytes in PRP
        bswap   r5,r4                   # Swap bytes for CDB
                                        # Pass 3 bytes of xfer size,
                                        # Last byte is the control
        st      r4,tpr_cmd+6(g0)        # Save xfer size in SCSI CDB
#
        mov     deok,r11                # Default to OK status
        ldconst 0,r10                   # Clear the 'good' counter
        ld      mfa_xcount(r15),r13     # Get requested count
        mov     r13,r12                 # r12 = remaining count
        ldos    mfa_pid(r15),r3         # Get input PID
        ld      P_pddindx[r3*4],g3      # Pass PDD
        call    O$genreq                # Generate request, g0 = template
#
# --- Send the write buffer x times
#
.dfa20:
        cmpobe  0,r12,.dfa100           # Jif all done - exit
#
        subo    1,r12,r12               # Decrement remaining xfer count
        addo    1,r10,r10               # Increment 'good' counter
        call    O$quereq                # Queue request
        call    M$chkstat               # Check the status
        cmpobe  ecok,g0,.dfa20          # Jif good - continue
#
# --- Error occurred
#
        subo    1,r10,r10               # Reduce 'good' count by 1
        mov     deioerr,r11             # IO error
#
# --- Cleanup and exit
#
.dfa100:
        call    O$relreq                # Release request: ILT, PRP, and buffer
        st      r10,mfa_good(r14)       # Return 'good' count
        mov     r11,g1                  # Return error code
        ldconst mfarsiz,g2              # Return packet size
#
# --- Exit
#
        ret
#
#**********************************************************************
#
#  NAME: d$filecopy
#
#  PURPOSE:
#       To implement the file copy MRP. This MRP allows the CCB to
#       copy all of part of a internal file system file to another
#       such file.
#
#  DESCRIPTION:
#       If the offset value for the source is equal to zero and the
#       total length is zero, then an entire copy of the file will
#       be performed.
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = status
#       g2 = return pkt size
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
d$filecopy:
        mov     g0,r13                  # Save regs
        mov     g3,r14
        mov     g4,r15
        mov     g6,r8
#
        call    FS$stop                 # Stop any fsys updates
#
        ld      mr_rptr(g0),r12         # Get return data pointer
        ld      mr_ptr(g0),r5           # Parm block address within MRP
        ldos    mfc_slen(r5),r9         # Get copy length
#
# --- Allocate buffer space for file
#
        ldconst SECSIZE,r4              # Convert blocks to bytes
        mulo    r9,r4,r10               # r10 = length in bytes
c       g0 = s_Malloc(r10, __FILE__, __LINE__); # Do not clear since we are reading
        ldconst deinsmem,r7             # Preload insufficient memory ret code
        cmpobe  0,g0,.dfsc110           # Jif not enough memory available
        mov     g0,r11                  # r11 = buffer ptr
#
# --- Read file into buffer
#
        ldconst deioerr,r7              # Preload I/O error return code
        ldos    mfc_sfid(r5),g0         # Load FID of file reading from
        mov     r11,g1                  # Load buffer address
        mov     r9,g2                   # Load length in blocks
        ldconst 1,g3                    # Load confirmation (double read)
        ldos    mfc_soffset(r5),g4      # Load block offset
        ldconst 0,g6                    # Load pid bitmap to zero
        call    FS$MultiRead            # Call file system read
        cmpobne 0,g0,.dfsc100           # Jif read failed
#
# --- Write file from buffer
#
        ldos    mfc_dfid(r5),g0         # Load FID of file write to
        mov     r11,g1                  # Load buffer address
        mov     r9,g2                   # Load length in blocks
        ldos    mfc_doffset(r5),g4      # Load destination block offset
        lda     mfc_goodmap(r12),g5     # Good write bit map address
        call    FS$MultiWrite           # Call file system write
!       st      g1,mfc_good(r12)        # Save off good write count
!       st      g2,mfc_error(r12)       # Save off error count
        cmpobne 0,g0,.dfsc100           # Jif write failed
        ldconst deok,r7                 # Success!!
#
# --- Deallocate buffer space
#
.dfsc100:
c       s_Free(r11, r10, __FILE__, __LINE__); # Free buffer
#
# --- Exit
#
.dfsc110:
        call    FS$resume               # Resume any fsys updates
#
        mov     r7,g1                   # Set return code
        ldconst mfcrsiz,g2              # Set return packet size
#
        mov     r8,g6
        mov     r15,g4
        mov     r14,g3
        mov     r13,g0                  # restore regs
        ret
#
#**********************************************************************
#
#  NAME: d$defaultlabel
#
#  PURPOSE:
#       To provide a means of processing the default label
#       request issued by the CCB.
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = status
#       g2 = return pkt size
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
d$defaultlabel:
        ldob    deinvpkttyp,g1          # Error out
        ldconst msdrsiz,g2              # Set return packet size
        ret
#
#**********************************************************************
#
#  NAME: d$setserial
#
#  PURPOSE:
#       To provide a common means of setting up the system serial
#       number.
#
#  DESCRIPTION:
#       The new system serial number is associates with this system.
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = status
#       g2 = return pkt size
#
#  REGS DESTROYED:
#       g6.
#
#**********************************************************************
#
d$setserial:
        mov     g3,r9                   # Save g3
        movl    g4,r12                  # Save g4 and g5
#
        ld      mr_rptr(g0),r15         # Get return block pointer
        ld      K_ficb,r14              # Get FICB address
        ld      mr_ptr(g0),g0           # Get parm block pointer
#
# --- Check if we are setting or just polling for the number.
#
        ldob    mas_op(g0),r3           # Get the opcode
        cmpobe  maspoll,r3,.sss100      # Jif just polling
#
# --- Check for a mirror partner clear operation. If we are clearing a
# --- partner, put the last entry into the found entrys place and then
# --- decrement the count. This effectively removes the entry.
#
        cmpobne masclearmp,r3,.sss40    # Jif not a mp clear
#
        ldconst 0,r4                    # Index into map
        ldob    D_mpcnt,r5              # Last entry in map
        ld      mas_serial(g0),r6       # Search serial number
#
.sss10:
        cmpobe  r4,r5,.sss60            # If equal, did not find entry
        ld      D_mpmaps[r4*8],r7       # Get serial (serial, partner in maps)
        cmpobe  r6,r7,.sss30            # Jif found
        addo    1,r4,r4                 # Inc index
        b       .sss10                  # Try again
#
# --- Found. Move the last entry up, zero the last entry and change the
# --- count.
#
.sss30:
        subo    1,r5,r5                 # Move to the right index
        ldl     D_mpmaps[r5*8],r10      # Get the serial and mirror partner
        stl     r10,D_mpmaps[r4*8]      # Save it
        movl    0,r10
        stl     r10,D_mpmaps[r5*8]      # Zero the old one
        stob    r5,D_mpcnt              # Save new count
        call    D$p2update              # Update the slaves
        b       .sss100                 # Exit
#
# --- Check for set controller serial number
#
.sss40:
#
        cmpobne mascserset,r3,.sss60    # Jif not a cntlr s/n set
#
# --- Get controller serial number
#
        mov     g0,r11                  # Save parm block pointer
        ldob    mas_serial(g0),r4       # Get new controller S/N addr
        ldob    mas_serial+1(g0),r5     # Get new controller S/N addr
        ldob    mas_serial+2(g0),r6     # Get new controller S/N addr
        ldob    mas_serial+3(g0),r7     # Get new controller S/N addr
        lda     NVSRAM+NVSRAMCSER,r8    # Address of control s/n in nvram
#
# --- Change the NVRAM controller serial number, byte by byte
#
        stob    r4,(r8)
        stob    r5,1(r8)
        stob    r6,2(r8)
        stob    r7,3(r8)
#       No need to either push or pop g-regs, as the following is a system call
#       r8 -->  Address of control s/n in NVRAM
#       4  -->  Length in bytes
#        c       g0 = msync((void*)(r8 - (r8%r10)), r10, MS_SYNC);
#        c       fprintf(stderr, "d$setserial:  RC from msync of S/N = %ld, errno = %d\n", g0, errno);
#
# --- Recalculate the NVRAM checksum
#
c       g0 = MSC_CRC32((void *)(NVSRAM+NVSRAMSTARTSN+4),NVSRAMSNSIZ-4)               # Calculate the new checksum
        lda     NVSRAM+NVSRAMSTARTSN,r8     # Load checksum location
#
# --- Store checksum in NVRAM, byte by byte
#
        stob    g0,(r8)
        shro    8,g0,g0
        stob    g0,1(r8)
        shro    8,g0,g0
        stob    g0,2(r8)
        shro    8,g0,g0
        stob    g0,3(r8)
#       No need to either push or pop g-regs, as the following is a system call
#       r8 ---> Checksum location
#       4  ---> Length in bytes
#       NOTE:  the checksum field is located prior to S/N field, so ensure the
#              length includes the S/N since only one msync is done now
c       r10 = r8 % getpagesize();
c       r5 = r10 + (NVSRAMCSER - NVSRAMSTARTSN);
c       g0 = msync((void*)(r8 - r10), 4 + r5, MS_SYNC);
c       if (g0 != 0) fprintf(stderr, "d$setserial:  msync failed, errno = %d\n", errno);
#
# --- Send MRP to the FE to change its controller serial number
#
# --- Load up the interprocessor communications area with the MRP
# --- data payload to send.
#
        ld      mas_serial(r11),r7      # Get new controller S/N
        ldconst mcsrsiz,g4              # Size of return data
# Need memory in g3 for L$send_packet call.
c       g3 = s_MallocC(g4, __FILE__, __LINE__); # Get Return data in shared memory.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message
        st      r7,mcs_cntlsn(g0)       # Pass the serial number
#
# --- Set up the base record
#
        ldconst mcssiz,g1               # Size of base packet
        ldconst mrsetcntlsnfe,g2        # Function code
#       g3 and g4 set above.
        lda     d$rmtwait,g5            # Completion function
        ld      K_xpcb,g6               # Get caller PCB
        call    L$send_packet           # Send the frame and await return
#
c       TaskSetMyState(pcrmtwait);      # Set process to wait for signal
        call    K$xchang                # Exchange processes
c       s_Free(g3, g4, __FILE__, __LINE__); # Free return data area
        mov     r11,g0                  # Restore parm block pointer
#
# --- Change FICB
#
        ld      mas_serial(g0),r4       # Get serial number
        ld      fi_cserial(r14),r6      # Get current serial number
        ld      fi_mirrorpartner(r14),r7 # Get cucurrent mirror partner
        cmpobne r6,r7,.sss50
        st      r4,fi_mirrorpartner(r14) # Store mirror partner in FICB
.sss50:
        shro    4,r4,r5                 # Get VCGID from serial number
        st      r4,fi_cserial(r14)      # Store serial number in FICB
        st      r5,fi_vcgid(r14)        # Set VCGID in FICB
#
# --- Change Admin region in NV Memory
#
        PushRegs(r6)                    # Save register contents
c       nv_InitSNAdminRegion(r4);
        PopRegsVoid(r6)                 # Restore registers
        b       .sss70
#
# --- Check for set VCG ID
#
.sss60:
        and     masserset,r3,r4
        cmpobne masserset,r4,.sss80     # Jif not a serial (VCG ID) update
#
        ld      mas_serial(g0),r10      # Get serial number
        st      r10,fi_vcgid(r14)       # Set it locally
#
# --- Update NVRAM
#
.sss70:
        call    D$p2updateconfig        # Update NVRAM part II
#
# --- Check for update of IP address
#
.sss80:
        and     masipset,r3,r4
        cmpobne masipset,r4,.sss90      # Jif not an IP address update
#
        ld      mas_ipaddr(g0),r10      # Get IP address
        st      r10,fi_ccbipaddr(r14)   # Set it locally
#
# --- Set flag to indicate IP address received
#
        ldos    K_ii+ii_status,r4       # Get initialization status
        setbit  iiccbipaddr,r4,r3       # Indicate IP address received
        stos    r3,K_ii+ii_status       # Set initialization status
#
.sss90:
#
# --- If this is the first call, do not update remote. Online will do it.
#
        ldos    K_ii+ii_status,r4       # Get initialization status
        bbc     iiccbipaddr,r4,.sss100  # Jif clear
        call    D$updrmtsysinfo         # Update remote
#
# --- Exit
#
.sss100:
        ld      fi_vcgid(r14),r3        # Get virtual controller group
!       st      r3,mas_rvcgid(r15)      # Set it in the return parm block
        ld      fi_cserial(r14),r3      # Get controller serial number
c fprintf(stderr, "%s%s:%u d$setserial: serialno = %08lX\n", FEBEMESSAGE, __FILE__, __LINE__, r3);
!       st      r3,mas_rcserial(r15)    # Set it in the return parm block
        ld      fi_ccbipaddr(r14),r3    # Get IP address of CCB
c fprintf(stderr, "%s%s:%u d$setserial: ccb IP addr = %08lX\n", FEBEMESSAGE, __FILE__, __LINE__, r3);
!       st      r3,mas_ripaddr(r15)     # Set it in the return parm block
        ld      fi_mirrorpartner(r14),r3# Get mirror partner
!       st      r3,mas_mp(r15)          # Set it in the return parm block
#
        mov     deok,g1                 # Return OK status
        ldconst masrsiz,g2              # Set return packet size
#
        mov     r9,g3                   # Restore g3
        movl    r12,g4                  # Restore g4 and g5
        ret
#
#**********************************************************************
#
#  NAME: D$damdirtyshell
#
#  PURPOSE:
#       To provide a means of calling DA_DAMDirty.
#
#  INPUT:
#       g0 = PID
#
#  OUTPUT:
#       None
#
#**********************************************************************
#
D$damdirtyshell:
c       DA_DAMDirty(g0);
        ret                             # Our job is done, exit.
#
#**********************************************************************
#
#  NAME: D$calcspaceshell
#
#  PURPOSE:
#       To provide a means of calling DA_DAMDirty without worrying
#       about registers getting trashed.
#
#  INPUT:
#       g0 = PID
#       g1 = Force
#
#  OUTPUT:
#       None
#
#**********************************************************************
#
D$calcspaceshell:
c       DA_CalcSpace(g0, g1);           # Calculate the space
        ret                             # Our job is done, exit.
#
#**********************************************************************
#
#  NAME: D$p2updateconfig
#
#  PURPOSE:
#       To provide a common means of updating Part II of the NVRAM and
#       initiating the update of all device labels when a configuration
#       change has taken place.
#
#  DESCRIPTION:
#       The NVRAM Part II configuration is updated followed by the
#       initiation of a process to update all device labels in the
#       system. The actual updating of the device labels takes place
#       after this routine exits.
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
# void NV_P2UpdateConfig(void);
        .globl  NV_P2UpdateConfig       # C access
NV_P2UpdateConfig:
D$p2updateconfig:
        movq    g0,r12                  # Save g0 - g3
        movq    g4,r8                   # Save g4 - g7
#
.pc00:
        ldos    K_ii+ii_status,r4       # Get initialization status
        bbc     iifulldef,r4,.pc100     # Jif full define running
        bbc     iimaster,r4,.pc100      # Jif slave device
#
        ldob    D_p2update_flag,r4      # r4 = p2update flag
        cmpobe  0,r4,.pc05              # Jif not executing this routine
#
# --- If the flag is non-zero, then sleep waiting on the NVRAM to be
# --- written by the other task that got here. The tasks waiting will
# --- be woken up at exit of the writing task.
#
        ldconst 2,r4                    # Indicate waiting for wakeup
        stob    r4,D_p2update_flag
c       TaskSetMyState(pcp2wait);       # Set process to wait for signal
        call    K$xchang                # Exchange processes
        b       .pc00                   # Try again
#
# --- Start the update. When done, clear the update flag and wake up andy
# --- tasks waiting for an update.
#
.pc05:
        ldconst 0x01,r4
        stob    r4,D_p2update_flag      # Set p2update flag
#
        PushRegs(r4)                    # Save register contents
        stob    0,CCSM_mupd_timer       # Reset copy update process timer
        call    NV_P2UpdateNvram        # Update NVRAM
        PopRegsVoid(r4)                 # Restore registers
#
        call    d$labupdate             # Update device labels
#
#        ldconst mlenvramwritten,g0
#        call    O_logerror
#
        ldob    D_p2update_flag,r5
        ldconst 0,r4
        stob    r4,D_p2update_flag      # Clear p2update_flag
        cmpobe  1,r5,.pc100             # Jif no tasks waiting to update
#
c       TaskReadyByState(pcp2wait);     # Signal tasks waiting for NVRAM write
#
.pc100:
        movq    r12,g0                  # Restore g0 - g3
        movq    r8,g4                   # Restore g4 - g7
#
# --- Exit
#
        ret
#
#**********************************************************************
#
#  NAME: D$p2update
#
#  PURPOSE:
#       To provide a common means of updating Part II of the NVRAM when
#       a change has occurred that causes the change in status but not
#       in configuration.
#
#  DESCRIPTION:
#       A local image is created and the CCB is notified that the update
#       is required.
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
# void NV_P2Update(void);
        .globl  NV_P2Update             # C access
NV_P2Update:
D$p2update:
#
.if     DEBUG_FLIGHTREC_D
        ldconst frt_h_misc1,r3          # Misc function
        st      r3,fr_parm0             # Function - d$p2update
        ldconst 0,r3
        st      r3,fr_parm1
        st      r3,fr_parm2
        st      r3,fr_parm3
        call    M$flight_rec            # Record it
.endif  # DEBUG_FLIGHTREC_D
#
        movq    g0,r12                  # Save g0 - g3
        movq    g4,r8                   # Save g4 - g7
#
        ldos    K_ii+ii_status,r4       # Get initialization status
        bbc     iifulldef,r4,.pu100     # Jif full define running
#
# --- We are a slave device. Create a local image and log it to the
# --- CCB for further processing. Since we may end up in a memory wait
# --- condition, get the memory wait and clear, then recalculate the
# --- size required. If the size has changed, redo the allocation.
#
        PushRegs(r3)                    # Save register contents
        call    NV_CalcLocalImageSize   # Calculate how much buffer is needed
        PopRegs(r3)                     # Restore registers (except g0)
        mov     g0,r6                   # Save the size
#
.pu20:
c       g0 = s_MallocC(g0, __FILE__, __LINE__);
        mov     g0,r7                   # Save the pointer (g7 = ptr, r6 = size)
#
        PushRegs(r3)                    # Save register contents
        call    NV_CalcLocalImageSize   # Recalculate the size
        PopRegs(r3)                     # Restore registers (except g0)
        cmpobe  g0,r6,.pu40             # If size did not change continue
#
c       s_Free(r7, r6, __FILE__, __LINE__); # Free buffer
        b       .pu20
#
.pu40:
        mov     r7,g0                   # Make the call with the memory assigned
        PushRegs(r3)                    # Save register contents
        call    NV_BuildLocalImage      # Build the image
        PopRegsVoid(r3)                 # Restore registers
#
# --- Set up the log message.
#
# This is more difficult to fix, there already is a completion function. FIX THIS. TODO.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst mlelclimagerdy,r3       # Get event code
        st      r3,mle_event(g0)        # Store as word to clear other bytes
        ldconst eliplen,r3              # Event length
        stob    r3,mle_len(g0)
#
        mov     r7,r3                   # Translate to global address
        st      r3,eli_ptr(g0)          # Set the pointer
        st      r6,eli_len(g0)          # Save size of local image
#
        ldconst elilen,g1               # Length of packet
        ldconst mrlogbe,g2              # MRP function code
        ldconst 0,g3                    # Data buffer
        ldconst 0,g4                    # Data size
        lda     d$p2updtcmplt,g5        # P2 update completer function
        mov     r7,g6                   # g6 has local image address
#
        call    L$send_packet           # Send the packet w/o wait
#
# No need to free buffer, as it is (4+4)+(4+4) bytes long and on the stack.
# Being this short, and all messages to CCB from BE are events, it is copied
# into the VRP by L$send_packet (ultimately).
#
# --- Exit
#
.pu100:
        movq    r12,g0                  # Restore g0 - g3
        movq    r8,g4                   # Restore g4 - g7
        ret
#
#**********************************************************************
#
#  NAME: d$p2updtcmplt
#
#  PURPOSE:
#       To provide a completion function for the P2 update operation.
#
#  DESCRIPTION:
#       A local image has been sent and received by the CCB. It is the
#       responsibility of this function to deallocate the memory used
#       by the p2 update.
#
#  INPUT:
#       g0 - completion code
#       g1 - ILT
#       g2 - MRP
#       g3 - address of local image sent
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
d$p2updtcmplt:
        PushRegs                        # Save all "g" registers
        cmpobne eclinkfail,g0,.dp2updcmp10 # Jif not a Link Failure
        ldos    K_ii+ii_status,r4       # Get the II Status
        bbs     iiccbreq,r4,.dp2updcmp10 # Jif more than one controller
#
c       NV_UpdateLocalImage((void*)g3); # Put the local image into NVRAM.
        call    d$labupdate             # Update device labels
#
.dp2updcmp10:
# --- Get the local image size out of the structure and release memory.
c       r15 = NV_GetLocalImageSize(g3);
c       s_Free(g3, r15, __FILE__, __LINE__);
#
        PopRegsVoid                     # Restore all "g" registers
        ret
#
#**********************************************************************
#
#  NAME: d$labupdate
#
#  PURPOSE:
#       To provide a standard means of updating all device label
#       information in a deferred fashion with the data from NVRAM
#       PART II.
#
#  DESCRIPTION:
#       This process goes to sleep whenever there is not a label update
#       pending. When an update is pending, the PDX is searched for
#       corresponding devices that are labelled either as data devices
#       or hotspares. In either case, the information from NVRAM PART
#       II is written to the reserved label area of the disk.
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g0,g1,g2
#
#**********************************************************************
#
d$labupdate:
#
# --- Make a copy of the data to write to disks.
#
        call    FS$stop                 # Stop any file system updates
#
        ldob    D_p2writefail_flag,r4   # Get the status of the last NVRAM write
c       if (r4 != TRUE) {               # If last NVRAM write worked (did not fail)
            lda     NVSRAM+NVSRAMRES,r6 # Get base of PART II
            ld      12(r6),g0           # Get the length of the NVRAM
c           r15 = (g0 + SECSIZE - 1) & ~(SECSIZE - 1);  # wrap to disk block size
c           g0 = s_MallocW(r15, __FILE__, __LINE__); # Allocate the data space
c           r14 = g0;                   # Save the pointer
# --- Copy NVRAM PART II to buffer
c           memcpy((void*)g0, (void*)r6, r15);
            mov     r15,g2              # Get PART II byte count for FS$MultiWrite
c       } else {
# --- Generate a new P2 NVRAM image to write to disks.
# --- g0 holds the new buffer address
c           r15 = NVSRAMP2SIZ;          # This is rounded to SECSIZE already.
c           g0 = s_MallocW(r15, __FILE__, __LINE__); # Allocate the data space
c           r14 = g0;                   # Save the pointer
            PushRegs(r3)                # Save register contents
            call    NV_P2GenerateImage  # Create new NVRAM image in buffer
            PopRegsVoid(r3)             # Restore registers
            ld      12(r14),g2          # Get actual NVRAM image length for FS$MultiWrite
c           g2 = (g2 + SECSIZE - 1) & ~(SECSIZE - 1);  # wrap to disk block size
c       }
#
# --- Do a multiple write to all drives
#
        ldconst fidbenvram,g0           # File ID
        mov     r14,g1                  # Buffer pointer
        ldconst SECSIZE,r3
        divo    r3,g2,g2                # Number of blocks to write
        ldconst 1,g4                    # Start at block one
        ldconst 0,g5                    # Good write map - no map requested
        call    FS$MultiWrite           # Generate write request
#
# --- Deallocate the buffer.
#
c       s_Free(r14, r15, __FILE__, __LINE__); # Free part II buffer
#
        call    FS$resume               # Resume any fs updates
#
        ret
#
#**********************************************************************
#
#  NAME: D$findpdd
#
#  PURPOSE:
#       To provide a standard means of finding the PDD associated
#       with a given FC channel and FC ID.
#
#  DESCRIPTION:
#       This function will search the PDD records and return the
#       PDD that matches the FC channel and ID. If none are found
#       that match, a NULL pointer is returned.
#
#  INPUT:
#       g0 - FC channel.
#       g1 - FC ID.
#       g2 - SCSI Lun.
#
#  OUTPUT:
#       g0 - address of the PDD. Null if not found
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
# C access
# PDD* DEF_FindPDD(UINT8 port, UINT32 fcid, UINT16 lun);
        .globl DEF_FindPDD
DEF_FindPDD:
D$findpdd:
#
# --- Make a copy of the channel
#
        mov     g0,r10
#
        lda     P_pddindx,r15           # Get the index table address
        ldconst MAXDRIVES-1,r14         # Get number of table entries
        ldconst 1,r13                   # Indicate first list
#
.fp10:
        ld      px_pdd(r15)[r14*4],g0   # Get PDD pointer
        cmpobe  0,g0,.fp20              # If NULL, check next pointer
#
# --- Check for channel, ID, and LUN
#
        ldob    pd_channel(g0),r3       # Check channel
        ld      pd_id(g0),r4            # Check ID
        ldos    pd_lun(g0),r5           # Check LUN
        cmpobne r3,r10,.fp20            # Jif not right channel
        cmpobne r4,g1,.fp20             # Jif not right ID
        cmpobe  r5,g2,.fp50             # Jif match... done
#
.fp20:
        subo    1,r14,r14               # Bump index
        cmpible 0,r14,.fp10             # If not done, check next
#
# --- Set up for SES device list
#
        cmpobe  2,r13,.fp30             # Jif processed second list
        cmpobe  3,r13,.fp40             # Jif processed third list
        lda     E_pddindx,r15           # Get the index table address
        ldconst MAXSES-1,r14            # Get number of table entries
        ldconst 2,r13                   # Indicate second list
        b       .fp10
#
# --- Set up for misc device list
#
.fp30:
        lda     M_pddindx,r15           # Get the index table address
        ldconst MAXMISC-1,r14           # Get number of table entries
        ldconst 3,r13                   # Indicate third list
        b       .fp10
#
.fp40:
        ldconst 0,g0                    # Not found, NULL pointer
#
# --- Exit
#
.fp50:
        ret
#
#**********************************************************************
#
#  NAME: d$findpddwwn
#
#  PURPOSE:
#       To provide a standard means of finding the PDD associated
#       with a given worldwide name.
#
#  DESCRIPTION:
#       This function will search the PDD records and return the
#       PDD that matches the worldwide name. If none are found
#       that match, a NULL pointer is returned.
#
#  INPUT:
#       g0/g1 - WWN
#       g2    - LUN
#
#  OUTPUT:
#       g0 - address of the PDD. Null if not found
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
    .globl DEF_FindPDDWWN
#
DEF_FindPDDWWN:
d$findpddwwn:
#
# --- Make a copy of the WWN
#
        movl    g0,r10
        mov     g2,r12
#
        lda     P_pddindx,r15           # Get the index table address
        ldconst MAXDRIVES-1,r14         # Get number of table entries
        ldconst 1,r13                   # Indicate first list
#
.fpw10:
        ld      px_pdd(r15)[r14*4],g0   # Get PDD pointer
        cmpobe  0,g0,.fpw20             # If NULL, check next pointer
#
# --- Check for both words of the WWN
#
        ldl     pd_wwn(g0),r4           # Load WWN from PDD
        ldos    pd_lun(g0),r3           # Load LUN from PDD
        cmpobne r10,r4,.fpw20           # Jif not right WWN
        cmpobne r11,r5,.fpw20           # Jif not right WWN
        cmpobe  r12,r3,.fpw50           # Jif match... done
#
.fpw20:
        subo    1,r14,r14               # Bump index
        cmpible 0,r14,.fpw10            # If not done, check next
#
# --- Set up for SES device list
#
        cmpobe  2,r13,.fpw30            # Jif processed second list
        cmpobe  3,r13,.fpw35            # Jif processed third list
        cmpobe  4,r13,.fpw40            # Jif processed third list
        lda     E_pddindx,r15           # Get the index table address
        ldconst MAXSES-1,r14            # Get number of table entries
        ldconst 2,r13                   # Indicate second list
        b       .fpw10
#
# --- Set up for o_pdd list
#
.fpw35:
        ld      O_p_pdd_list,r15
        cmpobe  0,r15,.fpw40
        lda     -px_ecnt(r15),r15       # Adjust for entry counter
        ldconst MAXOPDDLISTCOUNT-1,r14
        ldconst 4,r13
        b       .fpw10
#
# --- Set up for misc device list
#
.fpw30:
        lda     M_pddindx,r15           # Get the index table address
        ldconst MAXMISC-1,r14           # Get number of table entries
        ldconst 3,r13                   # Indicate third list
        b       .fpw10
#
.fpw40:
        ldconst 0,g0                    # Not found, NULL pointer
#
# --- Exit
#
.fpw50:
        ret
#
#**********************************************************************
#
#  NAME: D_insertpdd
#
#  PURPOSE:
#       To provide a standard means of placing the PDD into the PDX
#       and assigning it a PID.
#
#  DESCRIPTION:
#       This function will search the PDX records and return the
#       lowest PID available.
#
#  INPUT:
#       g0 - PDD to be inserted into the PDX
#
#  OUTPUT:
#       g0 - PID of the drive - g0 = MAXDRIVES if not inserted
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
# C access
# void DEF_InsertPDD(PDD* pPDD);
        .globl  DEF_InsertPDD           # C access
DEF_InsertPDD:
D_insertpdd:
        ldob    pd_devtype(g0),r3       # Get device type
        ldconst pddtmaxdisk,r4          # Is it a disk?
        cmpobg  r3,r4,.ip10             # Jif not a disk
#
        lda     P_pddindx,r15           # Get the index table address
        ldconst MAXDRIVES,r14           # Get number of table entries
        b       .ip30                   # Process it
#
.ip10:
        ldconst pddtmaxses,r4           # Is it an SES?
        cmpobg  r3,r4,.ip20             # Jif not an SES
#
        lda     E_pddindx,r15           # Get the index table address
        ldconst MAXSES,r14              # Get number of table entries
        b       .ip30                   # Process it
#
.ip20:
        lda     M_pddindx,r15           # Get the index table address
        ldconst MAXMISC,r14             # Get number of table entries
#
.ip30:
        ldconst 0,r13                   # Assign from the low values first
#
.ip40:
        ld      px_pdd(r15)[r13*4],r12  # Get PDD pointer
        cmpobne 0,r12,.ip50             # If not NULL, check next pointer
#
# --- Found a spot. Insert the PDD.
#
        st      g0,px_pdd(r15)[r13*4]   # Insert PDD pointer
        ldos    px_ecnt(r15),r3         # Get the device count
        addo    1,r3,r3                 # Bump the count
        stos    r3,px_ecnt(r15)         # Save incremented value
#
# --- Set the PID for this new drive.
#
        stos    r13,pd_pid(g0)          # Set PID
        b       .ip60
#
.ip50:
        addo    1,r13,r13               # Bump index
        cmpobl  r13,r14,.ip40           # If not done, check next
#
# --- Exit
#
.ip60:
        mov     r13,g0                  # Set return value
        ret
#
#**********************************************************************
#
#  NAME: d$setseq
#
#  PURPOSE:
#       To provide a common means of setting up the sequence number.
#
#  DESCRIPTION:
#       The sequence number is associates with this system.
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = status
#       g2 = return pkt size
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
d$setseq:
        ld      mr_ptr(g0),g0           # Get parm block pointer
#
# --- Get and set sequence number
#
        ld      msq_seq(g0),r10         # Get sequence number
        ld      K_ficb,r3               # Get FICB address
        st      r10,fi_seq(r3)          # Set it locally
#
# --- Save the new Sequence number to NVRAM
#
        call    D$p2update              # Update the slaves
#
# --- Exit
#
        mov     deok,g1                 # Return OK status
        ldconst msqrsiz,g2              # Set return packet size
        ret
#
#**********************************************************************
#
#  NAME: d$wwnlunlookup
#
#  PURPOSE:
#       To provide a standard means of looking up a device based upon
#       the WWN and LUN.
#
#  DESCRIPTION:
#       This function will search the X_pddindx tables to find the
#       device. The ID of the device and type will be returned if
#       found.
#
#  INPUT:
#       g0 - MRP
#
#  OUTPUT:
#       g1 - error code
#       g2 - length of return packet
#
#  REGS DESTROYED:
#       g0.
#
#**********************************************************************
#
d$wwnlunlookup:
#
# --- First, grab the return data address and length allowed.
#
        ld      mr_rptr(g0),r15         # Return data pointer
#
# --- Grab the parms and search for the device.
#
        ld      mr_ptr(g0),g0           # Parm block address
        ldl     mwl_wwn(g0),r10         # Get the WWN we are looking for
        ldos    mwl_lun(g0),r12         # Get LUN
#
        ldconst MAXDRIVES-1,r8          # Max physical ID
        ldconst mwldisk,r9              # Preset type
#
.wl10:
        ld      P_pddindx[r8*4],r3      # Get PDD
        cmpobe  0,r3,.wl20              # Jif not defined
#
        ldl     pd_wwn(r3),r4           # r4/r5 has wwn
        cmpobne r4,r10,.wl20            # Jif not equal
        cmpobne r5,r11,.wl20            # Jif not equal
#
        ldos    pd_lun(r3),r4           # WWN matched, check LUN
        cmpobe  r4,r12,.wl100           # Match, we are done
#
.wl20:
        subo    1,r8,r8                 # Decrement pointer
        cmpible 0,r8,.wl10              # Jif not done
#
# --- Do miscellaneous devices
#
        ldconst MAXMISC-1,r8            # Max miscellaneous physical ID
        ldconst mwlmisc,r9              # Preset type
#
.wl30:
        ld      M_pddindx[r8*4],r3      # Get PDD
        cmpobe  0,r3,.wl40              # Jif not defined
#
        ldl     pd_wwn(r3),r4           # r4/r5 has wwn
        cmpobne r4,r10,.wl40            # Jif not equal
        cmpobne r5,r11,.wl40            # Jif not equal
#
        ldos    pd_lun(r3),r4           # WWN matched, check LUN
        cmpobe  r4,r12,.wl100           # Match, we are done
#
.wl40:
        subo    1,r8,r8                 # Decrement pointer
        cmpible 0,r8,.wl30              # Jif not done
#
# --- Do SES devices
#
        ldconst MAXSES-1,r8             # Max SES physical ID
        ldconst mwlses,r9               # Preset type
#
.wl50:
        ld      E_pddindx[r8*4],r3      # Get PDD
        cmpobe  0,r3,.wl60              # Jif not defined
#
        ldl     pd_wwn(r3),r4           # r4/r5 has wwn
        cmpobne r4,r10,.wl60            # Jif not equal
        cmpobne r5,r11,.wl60            # Jif not equal
#
        ldos    pd_lun(r3),r4           # WWN matched, check LUN
        cmpobe  r4,r12,.wl100           # Match, we are done
#
.wl60:
        subo    1,r8,r8                 # Decrement pointer
        cmpible 0,r8,.wl50              # Jif not done
#
# --- None found. Set type to none.
#
        ldconst mwlnone,r9              # None found
#
# --- Exit
#
.wl100:
!       stob    r9,mwl_type(r15)        # Set return type
!       stos    r8,mwl_id(r15)          # Set ID
#
        ldconst deok,g1
        ldconst mwlrsiz,g2
        ret
#
#**********************************************************************
#
#  NAME: D_updrmttarg
#
#  PURPOSE:
#       To provide a standard means of sending a message to the
#       front end processor to update a target record.
#
#  DESCRIPTION:
#       This function will create a MRP to send across the PCI bus
#       to the front end processor and will wait until the front end
#       processor has informed us that it was processed.
#
#  INPUT:
#       g0 - SID
#       g1 - TRUE = delete target
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
# void DEF_UpdRmtTarg(UINT16 sid, UINT16 option);
        .globl  DEF_UpdRmtTarg          # C access
DEF_UpdRmtTarg:
D_updrmttarg:
        PushRegs                        # Save all G registers (stack relative)
#
# --- Load up the interprocessor communications area with the MRP
# --- data payload to send.
#
c       r15 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        movq    0,r4                    # Clear the MRP
        stq     r4,(r15)
        stq     r4,16(r15)
#
        lda     T_tgdindx,r14           # Get TGD address
        ld      tgx_tgd(r14)[g0*4],r14  # Actual TGD pointer
        cmpobe  0,r14,.ut40             # Jif NULL pointer
        cmpobe  TRUE,g1,.ut40           # Jif deletion requested
#
# --- Set up the base record
#
        ldq     (r14),r4                # TID through node name
        stq     r4,(r15)
        ldq     16(r14),r4              # Port name through owner
        stq     r4,16(r15)
        ldq     32(r14),r4              # Cluster through end
        stq     r4,32(r15)
#
.ut40:
        ldconst mctrsiz,g4              # Size of return data
# Need this in g3 for L$send_packet call.
c       g3 = s_MallocC(g4, __FILE__, __LINE__); # Get Return data in shared memory.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message
        ldconst mctsiz,g1               # Size of packet
        ldconst mrreporttarg,g2         # Function code
#       g3 and g4 set above.
        lda     d$rmtwait,g5            # Completion function
        ld      K_xpcb,g6               # Get caller PCB
        call    L$send_packet           # Send the frame and await return
#
c       TaskSetMyState(pcrmtwait);      # Set process to wait for signal
        call    K$xchang                # Exchange processes
c       s_Free(g3, g4, __FILE__, __LINE__); # Free return data area
#
# --- Exit
#
        PopRegsVoid                     # Restore all G registers (stack relative)
        ret
#
#**********************************************************************
#
#  NAME: d$cachestop
#
#  PURPOSE:
#       To provide a standard means of sending a message to the
#       front end processor to stop I/O.
#
#  DESCRIPTION:
#       This function will create a MRP to send across the PCI bus
#       to the front end processor and will wait until I/O has been
#       stopped on the FE.
#
#  INPUT:
#       g0 - Options flag for the Stop request
#       g1 - User ID
#
#  OUTPUT:
#       g0 - Return code from the remote stop
#
#  REGS DESTROYED:
#       g0-g6
#
#**********************************************************************
#
# UINT8 DEF_CacheStop(UINT8 options, UINT8 user);
        .globl  DEF_CacheStop          # C access
DEF_CacheStop:
d$cachestop:
        movl    g0,r14                  # Save input parm
#
        ldconst mrrsiz,g4               # Size of return data
# Need this in g3 for L$send_packet call.
c       g3 = s_MallocC(g4, __FILE__, __LINE__); # Get Return data in shared memory.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message
        stob    r14,mxc_op(g0)          # Set input parm
        stob    r15,mxc_user(g0)        # Save Generic User ID as input parm
#
        ldconst mxcsiz,g1               # Size of packet
        ldconst mrstopcache,g2          # Function code
#       g3 and g4 set above.
        lda     d$rmtwait,g5            # Completion function
        ld      K_xpcb,g6               # Get caller PCB
        call    L$send_packet           # Send the frame and await return
#
c       TaskSetMyState(pcrmtwait);      # Set process to wait for signal
        call    K$xchang                # Exchange processes
#
        ldob    mr_status(g3),r14       # Return the status that was passed back.
c       s_Free(g3, g4, __FILE__, __LINE__); # Free return data area
c       g0 = r14;                       # The return status that was passed back.
        ret
#
#**********************************************************************
#
#  NAME: d$cacheresume
#
#  PURPOSE:
#       To provide a standard means of sending a message to the
#       front end processor to resume I/O.
#
#  DESCRIPTION:
#       This function will create a MRP to send across the PCI bus
#       to the front end processor and will wait until I/O has been
#       resumed on the FE.
#
#  INPUT:
#       g0 - Options flag for the Resume request
#       g1 - User ID
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g0 - g6.
#
#**********************************************************************
#
# void DEF_CacheResume(UINT8 options, UINT8 user);
        .globl  DEF_CacheResume          # C access
DEF_CacheResume:
d$cacheresume:
        movl    g0,r14                  # Save input parm
#
        ldconst mrrsiz,g4               # Size of return data
# Need this in g3 for L$send_packet call.
c       g3 = s_MallocC(g4, __FILE__, __LINE__); # Get Return data in shared memory.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message
        stob    r14,mcc_op(g0)          # Set input parm
        stob    r15,mcc_user(g0)        # Save Generic User ID as input parm
        ldconst mccsiz,g1               # Size of packet
        ldconst mrcontinuecache,g2      # Function code
#       g3 and g4 set above.
        lda     d$rmtwait,g5            # Completion function
        ld      K_xpcb,g6               # Get caller PCB
        call    L$send_packet           # Send the frame and await return
#
c       TaskSetMyState(pcrmtwait);      # Set process to wait for signal
        call    K$xchang                # Exchange processes
c       s_Free(g3, g4, __FILE__, __LINE__); # Free return data area
        ret
#
#**********************************************************************
#
#  NAME: D_updrmtserver
#
#  PURPOSE:
#       To provide a standard means of sending a message to the
#       front end processor to update a single server record.
#
#  DESCRIPTION:
#       This function will create a MRP to send across the PCI bus
#       to the front end processor and will wait until the front end
#       processor has informed us that it was processed.
#
#  INPUT:
#       g0 - SID
#       g1 - TRUE = delete server
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
D_updrmtserver:
        PushRegs(r3)                    # Save all the registers
        call    DEF_UpdRmtServer
        PopRegsVoid(r3)                 # Restore the registers
        ret
#
#**********************************************************************
#
#  NAME: D$cmpltserverupd
#
#  PURPOSE:
#       To provide a standard means of sending a message to the
#       front end processor to complete the server update.
#
#  DESCRIPTION:
#       This function will create a MRP to send across the PCI bus
#       to the front end processor to indicate that the server
#       update has been completed. This is normally done at the
#       end of a restore from NVRAM.
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
D$cmpltserverupd:
#
# --- As a precaution, save the g registers
#
        PushRegs                        # Save all G registers (stack relative)
#
        ldconst mrrsiz,g4               # Size of return data
# Need this in g3 for L$send_packet call.
c       g3 = s_MallocC(g4, __FILE__, __LINE__); # Get Return data in shared memory.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message
        ldconst mrxsiz,g1               # Size of packet
        ldconst mrsconfigcomplete,g2    # Function code
#       g3 and g4 set above.
        lda     d$rmtwait,g5            # Completion function
        ld      K_xpcb,g6               # Get caller PCB
        call    L$send_packet           # Send the frame and await return
#
c       TaskSetMyState(pcrmtwait);      # Set process to wait for signal
        call    K$xchang                # Exchange processes
c       s_Free(g3, g4, __FILE__, __LINE__); # Free return data area
#
        PopRegsVoid                     # Restore all G registers (stack relative)
        ret
#
#**********************************************************************
#
#  NAME: d$updrmtcachesingle
#
#  PURPOSE:
#       To provide a standard means of sending a message to the
#       front end processor to update a single caching record.
#
#  DESCRIPTION:
#       This function will create a MRP to send across the PCI bus
#       to the front end processor and will wait until the front end
#       processor has informed us that it was processed.
#
#  INPUT:
#       g0 - VID to be updated.
#       g1 - Boolean delete flag (TRUE = delete)
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
# void DEF_UpdRmtCacheSingle(UINT16 vid, UINT16 option);
        .globl  DEF_UpdRmtCacheSingle
DEF_UpdRmtCacheSingle:
D$updrmtcachesingle:
        PushRegs                        # Save all G registers (stack relative)
#
# --- Load up the interprocessor communications area with the MRP
# --- data payload to send.
#
c       r15 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
#
        ldconst mrcdelete,r13           # Operation - assume delete
        mov     0,g2                    # VID Features - nothing (for delete)
        cmpobe  TRUE,g1,.ucs10          # Change operation if updating
#
        ldconst mrcsingle,r13           # Operation - update
        call    d$updvidfeat            # input = g0 - VID
                                        # output = g2 - Features flag
#
.ucs10:
#
# --- Set up the base record
#
        ld      V_vddindx[g0*4],r9      # r9 = VDD address
        ldconst 0,r10                   # Current entry pointer
        stos    r10,mrc_genable(r15)    # No global entry
        stob    r13,mrc_op(r15)         # Save opcode
        lda     mrc_vidmap(r15),r8      # VID map ptr
#
c   if (r9 == 0) {
c       *(UINT64 *)(r8 + mrc_capacity) = 0;
c   } else {
c       *(UINT64 *)(r8 + mrc_capacity) = ((VDD *)r9)->devCap;
c   }
        stob    g2,mrc_features(r8)     # Set VID features
        stos    g0,mrc_vid(r8)          # Set VID
#
        ldconst 1,r10
        stos    r10,mrc_nvid(r15)       # Save number of entries
        ldconst mrrsiz,g4               # Size of return data
# Need this in g3 for L$send_packet call.
c       g3 = s_MallocC(g4, __FILE__, __LINE__); # Get Return data in shared memory.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message
        ldconst mrcsiz+mrcxsiz,g1       # Size of base packet + 1 entry
        ldconst mrreportcconfig,g2      # Function code
#       g3 and g4 set above.
        lda     d$rmtwait,g5            # Completion function
        ld      K_xpcb,g6               # Get caller PCB
        call    L$send_packet           # Send the frame and await return
#
c       TaskSetMyState(pcrmtwait);      # Set process to wait for signal
        call    K$xchang                # Exchange processes
c       s_Free(g3, g4, __FILE__, __LINE__); # Free return data area
#
# --- Send update request MRP.
#
        call    D$signalvdiskupdate     # Signal update
#
# --- Exit
#
        PopRegsVoid                     # Restore all G registers (stack relative)
        ret
#
#**********************************************************************
#
#  NAME: D$updrmtcacheglobal
#
#  PURPOSE:
#       To provide a standard means of sending a message to the
#       front end processor to update the global caching status.
#
#  DESCRIPTION:
#       This function will create a MRP to send across the PCI bus
#       to the front end processor and will wait until the front end
#       processor has informed us that it was processed.
#
#  INPUT:
#       g0 - Boolean cache enable flag
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
# void DEF_UpdRmtCacheGlobal(UINT16 option);
        .globl  DEF_UpdRmtCacheGlobal
DEF_UpdRmtCacheGlobal:
D$updrmtcacheglobal:
        PushRegs                        # Save all G registers (stack relative)
#
# --- Load up the interprocessor communications area with the MRP
# --- data payload to send.
#
c       r15 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
#
        ldconst mrcglobal,r13           # Operation - global change
#
# --- Set up the base record
#
        ldob    D_glcache,r3            # Get global cache enable
        stob    r3,mrc_genable(r15)     # Global entry
        stob    r13,mrc_op(r15)         # Save opcode
#
        ldconst 0,r10
        stos    r10,mrc_nvid(r15)       # Save number of entries
#
        ldconst mrrsiz,g4               # Size of return data
# Need this in g3 for L$send_packet call.
c       g3 = s_MallocC(g4, __FILE__, __LINE__); # Get Return data in shared memory.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message
        ldconst mrcsiz,g1               # Size of base packet
        ldconst mrreportcconfig,g2      # Function code
#       g3 and g4 set above.
        lda     d$rmtwait,g5            # Completion function
        ld      K_xpcb,g6               # Get caller PCB
        call    L$send_packet           # Send the frame and await return
#
c       TaskSetMyState(pcrmtwait);      # Set process to wait for signal
        call    K$xchang                # Exchange processes
c       s_Free(g3, g4, __FILE__, __LINE__); # Free return data area
#
# --- Exit
#
        PopRegsVoid                     # Restore all G registers (stack relative)
        ret
#
#**********************************************************************
#
#  NAME: D$updrmtcache
#
#  PURPOSE:
#       To provide a standard means of sending a message to the
#       front end processor to update all caching records.
#
#  DESCRIPTION:
#       This function will create a number of MRPs to send across
#       to the front end processor and will wait until the front end
#       processor has informed us that each of them was processed.
#
#       The packets will be contain one or more entries (one per VDD).
#       The first packet will have a command setting indicating that
#       it is the first. The FEP will delete all VCDs prior to the
#       first entry in this packet and then either update or create
#       each entry from the parm list sent to it. On subsequent
#       packets, the first entry is a repeat of the last entry in the
#       previous packet. This gives the FEP the starting location
#       for deleting old VCDs. The last packet will contain only the
#       last VCD updated to show where the deletions should continue
#       from. The FEP will also restart caching at this point.
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
# void DEF_UpdRmtCache(void);
        .globl  DEF_UpdRmtCache
DEF_UpdRmtCache:
D$updrmtcache:
        PushRegs                        # Save all G registers (stack relative)
#
# --- Load up the interprocessor communications area with the MRP
# --- data payload to send.
#
c       r15 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst MAXVIRTUALS-1,r14       # Virtual disk index number
        ldconst mrcmultistart,r13       # Operation - start multiple list
#
.uc10:
#
# --- Set up the base record. Note that at each packet, the last one sent
# --- in the previous packet will be repeated in the next packet. This allows
# --- the FE to determine where any deletions occurred.
#
        mov     r14,r12                 # Start where we left off last time
        ldconst mrcmaxentries,r11       # Max entries in MRP
        ldconst 0,r10                   # Current entry pointer
        stos    r10,mrc_genable(r15)    # No global entry
        stob    r13,mrc_op(r15)         # Save opcode
        lda     mrc_vidmap(r15),r8      # VID map ptr
        ldconst mrcsiz,g1               # Size of base packet
#
# --- Set up as many virtual disk entries as possible per MRP.
#
.uc20:
        ld      V_vddindx[r12*4],r9     # VDD pointer
        cmpobe  0,r9,.uc30              # Jif no VDD at this VID
#
# --- Load up the entry and move pointers.
#
        mov     r12,r14                 # Save the last one sent in this packet
        mov     r12,g0                  # g0 = VID
        call    d$updvidfeat            # g2 = VID features flag
        stos    r12,mrc_vid(r8)         # Save the VID
        stob    g2,mrc_features(r8)     # Save the VID Features flag
# Save capacity.
c       *(UINT64 *)(r8 + mrc_capacity) = ((VDD *)r9)->devCap;
#
        addo    1,r10,r10               # Bump entries count
        lda     mrcxsiz(r8),r8          # Move destination pointer
        lda     mrcxsiz(g1),g1          # Increment size of packet
        cmpobe  r10,r11,.uc40           # Send it if enough entries
#
.uc30:
        subo    1,r12,r12               # Move to next (prev) hash list
        cmpible 0,r12,.uc20             # Jif no more lists
#
.uc40:
        stos    r10,mrc_nvid(r15)       # Save number of entries - zero is OK
#
        ldconst mrrsiz,g4               # Size of return data
# Need this in g3 for L$send_packet call.
c       g3 = s_MallocC(g4, __FILE__, __LINE__); # Get Return data in shared memory.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message
        ldconst mrreportcconfig,g2      # Function code
#       g3 and g4 set above.
        lda     d$rmtwait,g5            # Completion function
        ld      K_xpcb,g6               # Get caller PCB
        call    L$send_packet           # Send the frame and await return
#
c       TaskSetMyState(pcrmtwait);      # Set process to wait for signal
        call    K$xchang                # Exchange processes
c       s_Free(g3, g4, __FILE__, __LINE__); # Free return data area
#
# --- If r12 is negative, leave. Otherwise, there are more items
# --- potentially in the list and we are doing a split update.
#
        ldconst mrccontinue,r13         # Set the continue flag
        cmpible 0,r12,.uc10             # Another full packet
#
# --- Send completion MRP. This is just another report with the type of done
# --- and an entry for the last value sent.
#
        ldconst mrrsiz,g4               # Size of return data
# Need this in g3 for L$send_packet call.
c       g3 = s_MallocC(g4, __FILE__, __LINE__); # Get Return data in shared memory.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message
#
        ldconst 1,r10
        stos    r10,mrc_nvid(g0)        # Number of entries
        lda     mrc_vidmap(g0),r3       # VID map ptr
        stos    r14,mrc_vid(r3)         # Save last VID done
        ldconst mrcmultidone,r3         # Done opcode
        stob    r3,mrc_op(g0)           # Save opcode
#
        ldconst mrcsiz+mrcxsiz,g1       # Size of base packet and one entry
        ldconst mrreportcconfig,g2      # Function code
#       g3 and g4 set above.
        lda     d$rmtwait,g5            # Completion function
        ld      K_xpcb,g6               # Get caller PCB
        call    L$send_packet           # Send the frame and await return
#
c       TaskSetMyState(pcrmtwait);      # Set process to wait for signal
        call    K$xchang                # Exchange processes
c       s_Free(g3, g4, __FILE__, __LINE__); # Free return data area
#
# --- Exit
#
        PopRegsVoid                     # Restore all G registers (stack relative)
        ret
#
#**********************************************************************
#
#  NAME: D$updrmtsysinfo
#
#  PURPOSE:
#       To provide a standard means of sending a message to the
#       front end processor to update the system information.
#
#  DESCRIPTION:
#       This function will create a MRP to send across the PCI bus
#       to the front end processor and will wait until the front end
#       processor has informed us that it was processed.
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
# void DEF_UpdRmtSysInfo(void);
        .globl  DEF_UpdRmtSysInfo
DEF_UpdRmtSysInfo:
D$updrmtsysinfo:
#
# --- As a precaution, save the g registers
#
        PushRegs                        # Save all G registers (stack relative)
#
# --- Load up the interprocessor communications area with the MRP
# --- data payload to send.
#
        ldconst mrrsiz,g4               # Size of return data
# Need this in g3 for L$send_packet call.
c       g3 = s_MallocC(g4, __FILE__, __LINE__); # Get Return data in shared memory.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message
#
# --- Set up the base record
#
        ld      K_ficb,r3               # Get ficb address
        ld      fi_vcgid(r3),r4         # Virtual controller group
        st      r4,mii_vcgid(g0)        # Put it into the MRP
        ld      fi_cserial(r3),r4       # Controller serial number
        st      r4,mii_cserial(g0)      # Put it into the MRP
        ld      fi_seq(r3),r4           # Sequence number
        st      r4,mii_seq(g0)          # Put it into the MRP
        ld      fi_ccbipaddr(r3),r4     # IP address of CCB
        st      r4,mii_ccbipaddr(g0)    # Put it into the MRP
        ld      fi_mirrorpartner(r3),r4 # Mirror partner serial number
        st      r4,mii_mp(g0)           # Put it into the MRP
        ldq     fi_vcgname(r3),r4       # Get the VCG name
        stq     r4,mii_vcgname(g0)      # Put it in the parm block
#
        ldconst FALSE,r4                # Assume CCB not require
        ldos    K_ii+ii_status,r3       # Get status bits
        bbc     iiccbreq,r3,.dusi10     # Jif only one controller
        ldconst TRUE,r4                 # Set CCB required
.dusi10:
        stob    r4,mii_ccbreq(g0)       # Put it into the MRP
#
        ldconst miisiz,g1               # Size of base packet
        ldconst mrsetsysinfo,g2         # Function code
#       g3 and g4 set above.
        lda     d$rmtwait,g5            # Completion function
        ld      K_xpcb,g6               # Get caller PCB
        call    L$send_packet           # Send the frame and await return
#
c       TaskSetMyState(pcrmtwait);      # Set process to wait for signal
        call    K$xchang                # Exchange processes
c       s_Free(g3, g4, __FILE__, __LINE__); # Free return data area
#
# --- Exit
#
        PopRegsVoid                     # Restore all G registers (stack relative)
        ret
#
#**********************************************************************
#
#  NAME: d$updvidfeat
#
#  PURPOSE:
#       Gather the specific features for the VID before sending information
#       to the FE processor.
#
#  DESCRIPTION:
#       This function will interrogate the VID to determine what features
#       are enabled for the virtual device. The features found enabled
#       will be returned to the caller to use in updating the FE processor.
#
#  INPUT:
#       g0 = VID
#
#  OUTPUT:
#       g2 = VID Features flag for use in the Caching Configuration Report MRP
#
#  REGS DESTROYED:
#       g2
#
#**********************************************************************
#
d$updvidfeat:
        mov     g0,r15                  # Save g0
#
# --- Determine if Cache is enabled for the VID
#
        ldconst 0,r12                   # clear branch queue
#
        ld      d_resync_paskey,r3      # increment
        addo    1,r3,r3                 # increment it
        st      r3,d_resync_paskey      # save new value
#
        ld      V_vddindx[g0*4],r14     # r14 = VDD address
        ldos    vd_attr(r14),r3         # r3 = attributes flag
        ldconst vdcacheen,r4            # Check for cache enable
        and     r3,r4,r3                # Mask it
        cmpo    r4,r3                   # Check to see if Cache is enabled
        alterbit mrc_cache_enable,0,g2  # Set Cache flag appropriately and clear
                                        #   all the other flags
#
# --- Check for the destination of a copy operation.
#
        ld      vd_dcd(r14),r4          # r4 = possible DCD
        cmpobe  0,r4,.uvf10             # Jif not copy destination
        ldob    dcd_type(r4),r6         # r6 = DCD type code
        cmpobe.f dcdt_remote,r6,.uvf10  # Jif remote dest. copy device
        ld      dcd_cor(r4),r4          # r4 = assoc. COR address
        ldob    cor_crstate(r4),r6      # r6 = copy reg. state code
        cmpobe.f corcrst_usersusp,r6,.uvf10 # Jif user suspended
        cmpobe.f corcrst_remsusp,r6,.uvf10  # Jif remote suspended
        setbit  mrc_copy_dest,g2,g2     # set the copy destination bit
#
# --- Determine if a RAID type exists in this VID, or a destination of a
# --- Copy/Mirror of this VID, that requires write information be
# --- mirrored (if in an N-Way environment). Also determine if any RAID in the
# --- VDisk is in the Rebuilding State, and if so, turn on the Rebuild before
# --- write bit.
#
# --- Deferred RAIDs first and then non-deferred RAIDs
#
.uvf10:
        ld      vd_scdhead(r14),r6      # r6 = Copy/Mirror record
.uvf20:
        ld      vd_drdd(r14),r4         # Deferred RAIDs assigned to this VID
        ld      vd_rdd(r14),r13         # r13 = Non-deferred RAIDs assigned VID
#
.uvf30:
        cmpobe  0,r4,.uvf50             # Jif no more RAIDs
        ldob    rd_type(r4),r5          # r5 = RAID type
        cmpobne rdraid5,r5,.uvf40       # Jif RAID 5 - Mirroring needed
        setbit  mrc_mirror_write_info,g2,g2 # Set the Mirror Write Info flag
        mov     r4,g0                   # g0 = RDD
        call    R$checkForR5Rebuilding  # Determine if this RAID is Rebuilding
        cmpobe  FALSE,g0,.uvf40         # Jif Rebuilding is not going on
        setbit  mrc_rebuild_check_needed,g2,g2 # Set Rebuild Check Required on
                                        #  writes to this RAID
        b       .uvf100                 # Jim says we'er out of here :-)
#
.uvf40:
        ld      rd_nvrdd(r4),r4         # Point to the next RAID
        b       .uvf30                  # Check out the next RAID (if exists)
#
.uvf50:
        cmpobe  0,r13,.uvf55            # Jif all RAIDs checked - None found,
                                        #  continue with checking copy/mirror
        mov     r13,r4                  # Start at the non-deferred RAIDs
        ldconst 0,r13                   # Clear the next RAID pointer
        b       .uvf30                  # Continue looking
#
# --- Check the Copy/Mirror records to see if the source needs mirroring even
# --- though it is of the wrong type.
#
# --- determine if the current destination is a source of a copy
#
.uvf55:
        cmpobe  0,r6,.uvf60d            # Jif no more copy/mirror records for this vdd
#
.uvf60:
        ld      scd_cor(r6),r7          # r7 = cor address
        ld      cor_destvdd(r7),r14     # get destination vdd
        cmpobe  0,r14,.uvf60a           # Jif no dest vdd
#
        ld      d_resync_paskey,r3      # passkey
        ld      cor_r5PasKey(r7),r4     # get cor pass key
        cmpobe  r3,r4,.uvf60a           # Jif we've seen this cor before
#
        ld      vd_scdhead(r14),r4      # r4 = possible copy source
        cmpobne  0,r4,.uvf60b           # Jif this destination is also a source device
#
.uvf60a:
        ld      scd_link(r6),r6         # Point to the next copy
        b       .uvf55                  # continue
#
.uvf60b:
        st      r3,cor_r5PasKey(r7)     # mark this cor as processed
        ld      scd_link(r6),r6         # Point to the next destination
        cmpobe  0,r6,.uvf60c            # Jif no forward link
#
        st      r12,scd_rlink(r6)       # save link to previous
        mov     r6,r12                  # save as new head of branch queue
#
.uvf60c:
        mov     r4,r6                   # save new scd address
        ld      scd_vdd(r6),r14         # get associated VDD address
#        ld      scd_link(r6),r6         # Point to the next copy
        b       .uvf20                  # continue
#
.uvf60d:
        cmpobe  0,r12,.uvf100           # JIF no branch point
#
        mov     r12,r6                  # copy head of branch queue
        ldconst 0,r3                    # clear r3
        ld      scd_rlink(r6),r12       # get link to next entry on list
        st      r3,scd_rlink(r6)        # clear link
        b       .uvf60                  # continue
#
# --- Exit
#
.uvf100:
        mov     r15,g0                  # Restore g0
        ret
#
#**********************************************************************
#
#  NAME: D$SndFTOO
#
#  PURPOSE:
#       To provide a standard means of sending Set Foreign Target On/Off
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       None.
#
#**********************************************************************
#
        .data
s_ftchange_pcb:
        .word   0
        .text
#
D$SndFTOO:
c   if (*(UINT32*)&s_ftchange_pcb == 0) {
c   *(UINT32*)&s_ftchange_pcb = -1;     # Flag we are going to start this task (in case out of memory)
        PushRegs                        # Save all G registers (stack relative)
        lda     s$sendftchange,g0       # Send foreign target change
        ldconst DEXECPRI,g1
c       CT_fork_tmp = (ulong)"s$sendftchange";
        call    K$fork
        st      g0,s_ftchange_pcb       # Save PCB
        PopRegsVoid                     # Restore all G registers (stack relative)
c   }
        ret
#
s$sendftchange:
        ldconst msftrsiz,g4             # Size of return data
# Need this in g3 for L$send_packet call.
c       g3 = s_MallocC(g4, __FILE__, __LINE__); # Get Return data in shared memory.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message
#
# --- Set up the configurable options
#
        ldob    D_ften,r4               # get D_ften value
        stob    r4,msft_option(g0)      # Save it in request
#
        ldconst msftsiz,g1              # Size of base packet
        ldconst mrsetft,g2              # Function code
#       g3 and g4 set above.
        lda     d$rmtwait,g5            # Completion function
        ld      K_xpcb,g6               # Get caller PCB
c fprintf(stderr, "D$SndFTOO - Set Foreign Target value to %ld\n", r4);
        call    L$send_packet           # Send the frame and await return
#
c       TaskSetMyState(pcrmtwait);      # Set process to wait for signal
        call    K$xchang                # Exchange processes
c       s_Free(g3, g4, __FILE__, __LINE__); # Free return data area
#
        st      0,s_ftchange_pcb        # Flag task is not running
        ret
#
#**********************************************************************
#
#  NAME: D$sndconfigopt
#
#  PURPOSE:
#       To provide a standard means of sending the configurable
#       options to the front end processor.
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
# void DEF_SndConfigOpt(void);
        .globl  DEF_SndConfigOpt
DEF_SndConfigOpt:
D$sndconfigopt:
#
# --- As a precaution, save the g registers
#
        PushRegs                        # Save all G registers (stack relative)
#
# --- Load up the interprocessor communications area with the MRP
# --- data payload to send.
#
        ldconst mscorsiz,g4             # Size of return data
# Need this in g3 for L$send_packet call.
c       g3 = s_MallocC(g4, __FILE__, __LINE__); # Get Return data in shared memory.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message
#
# --- Set up the configurable options
#
        ldob    NV_scsi_whql,r4         # get WHQL compliance enable
        cmpobe  TRUE,r4,.sndconfigopt_1 # If TRUE, set msco_whql to TRUE
        ldconst FALSE,r4                # If DISABLE, set msco_whql to FALSE
#
.sndconfigopt_1:
        stob    r4,msco_whql(g0)        #   and save it
#
        ldconst mscosiz,g1              # Size of base packet
        ldconst mrsetconfigopt,g2       # Function code
#       g3 and g4 set above.
        lda     d$rmtwait,g5            # Completion function
        ld      K_xpcb,g6               # Get caller PCB
        call    L$send_packet           # Send the frame and await return
#
c       TaskSetMyState(pcrmtwait);      # Set process to wait for signal
        call    K$xchang                # Exchange processes
c       s_Free(g3, g4, __FILE__, __LINE__); # Free return data area
#
# --- Exit
#
        PopRegsVoid                     # Restore all G registers (stack relative)
        ret
#
#**********************************************************************
#
#  NAME: D$signalvdiskupdate
#
#  PURPOSE:
#       To provide a standard means of sending a message to the
#       front end processor to signal that the virtual disk config
#       has changed.
#
#  DESCRIPTION:
#       This function will create a MRP to send across the PCI bus
#       to the front end processor and will wait until the front end
#       processor has informed us that it was processed.
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
# void DEF_SignalVDiskUpdate(void);
        .globl  DEF_SignalVDiskUpdate
DEF_SignalVDiskUpdate:
D$signalvdiskupdate:
#
# --- As a precaution, save the g registers
#
        PushRegs                        # Save all G registers (stack relative)
#
# --- Load up the interprocessor communications area with the MRP
# --- data payload to send.
#
        ldconst mrrsiz,g4               # Size of return data
# Need this in g3 for L$send_packet call.
c       g3 = s_MallocC(g4, __FILE__, __LINE__); # Get Return data in shared memory.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message
#
# --- Set up the base record
#
        ldconst mvusiz,g1               # Size of base packet
        ldconst mrvchange,g2            # Function code
#       g3 and g4 set above.
        lda     d$rmtwait,g5            # Completion function
        ld      K_xpcb,g6               # Get caller PCB
        call    L$send_packet           # Send the frame and await return
#
c       TaskSetMyState(pcrmtwait);      # Set process to wait for signal
        call    K$xchang                # Exchange processes
c       s_Free(g3, g4, __FILE__, __LINE__); # Free return data area
#
# --- Exit
#
        PopRegsVoid                     # Restore all G registers (stack relative)
        ret
#
#**********************************************************************
#
#  NAME: D$signalserverupdate
#
#  PURPOSE:
#       To provide a standard means of sending a message to the
#       front end processor to signal that the server config
#       has changed.
#
#  DESCRIPTION:
#       This function will create a MRP to send across the PCI bus
#       to the front end processor and will wait until the front end
#       processor has informed us that it was processed.
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
# void DEF_SignalServerUpdate(void);
        .globl  DEF_SignalServerUpdate  # C access
DEF_SignalServerUpdate:
D$signalserverupdate:
#
# --- As a precaution, save the g registers
#
        PushRegs                        # Save all G registers (stack relative)
#
# --- Load up the interprocessor communications area with the MRP
# --- data payload to send.
#
        ldconst mrrsiz,g4               # Size of return data
# Need this in g3 for L$send_packet call.
c       g3 = s_MallocC(g4, __FILE__, __LINE__); # Get Return data in shared memory.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message
#
# --- Set up the base record
#
        ldconst msusiz,g1               # Size of base packet
        ldconst mrschange,g2            # Function code
#       g3 and g4 set above.
        lda     d$rmtwait,g5            # Completion function
        ld      K_xpcb,g6               # Get caller PCB
        call    L$send_packet           # Send the frame and await return
#
c       TaskSetMyState(pcrmtwait);      # Set process to wait for signal
        call    K$xchang                # Exchange processes
c       s_Free(g3, g4, __FILE__, __LINE__); # Free return data area
#
# --- Exit
#
        PopRegsVoid                     # Restore all G registers (stack relative)
        ret
#
#**********************************************************************
#
#  NAME: d$setgpri
#
#  PURPOSE:
#       To provide a means of setting the global priority.
#
#  DESCRIPTION:
#       The packet size and priority option parameters are validated
#       with the new setting being established should these checks
#       be successful.
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = status
#       g2 = return pkt size
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
d$setgpri:
#
# --- Validate priority
#
        ld      mr_ptr(g0),r15          # Parm block address
#
        ldob    mbp_priority(r15),r4    # Get priority
        ldconst deinvopt,g1             # Prep possible error code
        cmpobl  mbpmaxpri,r4,.sg100     # Jif out of range
#
# --- Establish global priority
#
        stob    r4,D_gpri               # Establish global priority
        stob    r4,K_ii+ii_gpri
#
# --- Update NVRAM
#
        call    D$p2updateconfig        # Update NVRAM part II
        mov     deok,g1                 # Return OK status
#
# --- Exit
#
.sg100:
        mov     mbprsiz,g2              # Set return packet size
        ret
#
#**********************************************************************
#
#  NAME: D$changename
#
#  PURPOSE:
#       To provide a means of changing the name of a vdisk or controller.
#
#  DESCRIPTION:
#       The name change will be requested via a log event to the CCB.
#
#  INPUT:
#       g0 = VID to change
#       g1 = type of name to change (ctrl, vdisk, both)
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
D$changename:
#
# --- Package up the log event to kick off the name change
#
        movq    g0,r12                  # Save g0, g1, g2 and g3
        movq    g4,r8                   # Save g4-g7
#
c       g0 = s_MallocC(ecnlen, __FILE__, __LINE__); # Get the input buffer
#
# --- Copy the data into the buffer
#
        stos    r12,ecn_vlid(g0)        # Set the VID
        stos    r13,ecn_type(g0)        # Set the type
#
# --- Fill in the name space. If the pointer to the name is NULL, then
# --- skip the fill in, duh!
#
        ldconst ecnctrl,r3              # Check for controller update
        cmpobne r3,r13,.cn20            # Jif no controller name
#
        ld      vl_sulst,r3             # r3 = the pointer to the SUL area
        ldq     vl_sulr_name(r3)[r12*1],r4# Get MAG node name in SUL
        stq     r4,ecn_ctrlname(g0)
        b       .cn30                   # Send it
#
.cn20:
        ldconst ecnvlink,r3             # Check for vlink update
        cmpobne r3,r13,.cn100           # Jif the VLink name did not change
#
        ld      DLM_lddindx[r12*4],r3   # Get LDD
        cmpobe  0,r3,.cn100             # Null pointer, invalid LDD
        ldq     ld_basename(r3),r4      # Get the vlink name
        stq     r4,ecn_vdname(g0)
#
.cn30:
        ldconst mlechangename,r3        # Get event code
        st      r3,mle_event(g0)        # Store as word to clear other bytes
        ldconst ecnplen,r3              # Event length
        stob    r3,mle_len(g0)
        ldconst ecnlen,g1               # Length of packet
        ldconst mrlogbe,g2              # MRP function code
        ldconst 0,g3                    # Return data address (no data expected)
        ldconst 0,g4                    # No data allowed
        lda     d$chgnamecmplt,g5       # Change Name Log Event completer func
        mov     g0,g6                   # Return the Address to free
#
        call    L$send_packet           # Send the packet
#
# --- Exit
#
.cn100:
        movq    r12,g0                  # Restore g0-g3
        movq    r8,g4                   # Restore g4-g7
        ret
#
#**********************************************************************
#
#  NAME: d$chgnamecmplt
#
#  PURPOSE:
#       To provide a completion function for the Change Name update operation.
#
#  DESCRIPTION:
#       A Change Name Log event has been sent and received by the CCB. It is
#       the responsibility of this function to deallocate the memory used
#       by the Change Name Log event.
#
#  INPUT:
#       g3 - address of log event sent
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
d$chgnamecmplt:
#
# --- Free the memory allocated for the Change Name Log Event
#
c       s_Free(g3, ecnlen, __FILE__, __LINE__); # Free packet
        ret
#
#**********************************************************************
#
#  NAME: d$rmtctrlcnt
#
#  PURPOSE:
#       To provide a means for the CCB to determine the number of
#       remote TBolt, Magnitudes, or foreign targets attached for
#       use in SAN links.
#
#  DESCRIPTION:
#       The DLM will be queried for the list of remote controllers.
#       The count of the controllers will be returned in the MRP return
#       data.
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = status
#       g2 = return pkt size
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
d$rmtctrlcnt:
        ld      mr_ptr(g0),r15          # Parm block address
        ld      mr_rptr(g0),r14         # Return block address
#
        call    DLM$upsul               # Update the server unit list
        cmpobne deok,g1,.drmtctrlcnt100 # Jif the data was not set up correctly
#
# --- Get storage area address and then the count
#
        ld      vl_sulst,r4             # Base address of storage unit area
        ldos    vl_sulh_cnt(r4),r3      # Count in data area
!       stos    r3,mnc_count(r14)       # Save it
#
.drmtctrlcnt100:
        ldconst mncrsiz,g2              # Set return packet size
        ret
#
#**********************************************************************
#
#  NAME: d$rmtctrlinfo
#
#  PURPOSE:
#       To provide a means for the CCB to determine the information
#       regarding a specific remote TBolt, Magnitudes, or foreign target.
#
#  DESCRIPTION:
#       The DLM will have been queried for a count of remote controllers
#       and will have a list in memory of the specifics on the remote
#       controllers. This MRP will return the data regarding a specific
#       controller in that list.
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = status
#       g2 = return pkt size
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
d$rmtctrlinfo:
        ld      mr_ptr(g0),r15          # Parm block address
        ld      mr_rptr(g0),r14         # Parm block address
#
# --- Get the controller record address by offsetting into the table.
#
        ldos    mci_cid(r15),r6         # r6 = index into table
        ld      vl_sulst,r4             # r4 = base address of Storage Unit
        ldos    vl_sulh_cnt(r4),r3      # Get max offset available
        ldconst deinvctrl,g1            # Prepare error code
        cmpobge r6,r3,.rci100           # Jif out of range
#
        lda     vl_sulh_siz(r4),r4      # r4 = record pointer base
        ldconst vl_sulr_siz,r7          # r7 = record size
        mulo    r6,r7,r6                # r6 = offset into table
        addo    r6,r4,r4                # r4 = controller record address
        ld      vl_sulr_dtmt(r4),r5     # Get the DTMT pointer
        cmpobe  0,r5,.rci100            # Jif no DTMT address assoc. with
                                        #  storage controller
#
# --- Now we have the record, so fill in the return data.
#
        ldl     vl_sulr_mac(r4),r6      # World wide name
!       stl     r6,mci_wwn(r14)
        ldq     vl_sulr_name(r4),r8     # First four words of name
!       stq     r8,mci_cname(r14)
        ld      vl_sulr_name+16(r4),r8  # Last word of name
!       st      r8,mci_cname+16(r14)
        ldob    vl_sulr_luns(r4),r6     # Number of LUNs
!       stob    r6,mci_luns(r14)
        ldob    vl_sulr_type(r4),r6     # Type of controller
!       stob    dtmt_ty_MAG,mci_sctype(r14)
        ldob    vl_sulr_cl(r4),r7       # Cluster number
!       stob    r7,mci_cluster(r14)
#
        ld      dml_ip(r5),r7           # Get the CCB IP address
!       st      r7,mci_ipaddr(r14)
c   if (r6 == dtmt_ty_FT) {             # If foreign target, fix serial number to look reasonable.
        ld      dft_sn(r5),r7           # Get part of the WWN
c       r7 = (r7 & 0xffff) | 0x8000;
c   } else {
        ld      dml_sn(r5),r7           # Get the Controller serial number
c   }
!       st      r7,mci_csn(r14)
#
        ldconst deok,g1                 # Return OK status
#
.rci100:
        ldconst mcirsiz,g2              # Set return packet size
        ret
#
#**********************************************************************
#
#  NAME: d$rmtvdiskinfo
#
#  PURPOSE:
#       To provide a means for the CCB to get the information on a
#       specific virtual disk on a remote controller.
#
#  DESCRIPTION:
#       The DLM will have been queried for a count of remote controllers
#       and will have a list in memory of the specifics on the remote
#       controllers. The ordinal value of a specific controller in
#       that list will be sent to this code. The data returned will be
#       all of the virtual disk on that controller.
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = status
#       g2 = return pkt size
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
d$rmtvdiskinfo:
        ld      mr_ptr(g0),r15          # Parm block address
        ld      mr_rptr(g0),r14         # Return block address
#
# --- Get Storage Unit List index value
#
        ldos    mvi_cid(r15),g1         # Storage unit list index value
c       g2 = 64;                        # Try 64 bit vlink version first.
        call    DLM$upsud               # Call routine to process the request
c   if (g1 == deinopdev) {              # returned if non-existent device (or does not support 64 bit)
c       g2 = 32;                        # Try 32 bit vlink version next.
        call    DLM$upsud               # Call routine to process the request
c   }
        cmpobne deok,g1,.rvi100         # Jif error (g1 is an output of upsud)
#
# --- Grab all of the data and put it in the return block.
#
        ld      vl_sudata,r5            # Get address of data area
        ldob    vl_sudh_cnt(r5),r4      # Number of devices to copy
!       stob    r4,mvi_cnt(r14)         # Put in the return data area
        cmpobe  0,r4,.rvi100            # Exit if no virtual disks
#
        lda     vl_sudh_siz(r5),r5      # Get into the data area
        lda     mvi_data(r14),r14       # Get into the mvi_data area
c       memcpy((void*)r14, (void*)r5, r4*vl_sudr_siz); # vl_sudr_siz == mvixsiz

#
.rvi100:
        ldconst mvirsiz,g2              # Set return packet size
        ret
#
#**********************************************************************
#
#  NAME: d$setforeigntarget
#
#  PURPOSE:
#       To provide a means for the CCB to set the foreign target
#       enable/disable for all the front end channels.
#
#  DESCRIPTION:
#       The bitmap sent in will be used as is. In order to fetch
#       the settings, the internal information is used.
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = status
#       g2 = return pkt size
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
d$setforeigntarget:
        ld      mr_ptr(g0),r15          # Parm block address
#
# --- Search for the bits which have changed until no more changes exist.
#
        ldob    mft_bmap(r15),r14       # Bit map to change to (1 = ft enable)
c   r14 = r14 & ((1 << (MAXISP+MAXICL))-1); # Limit number of FE ports to 4 (or 5)
        ldob    D_ften,r13              # Old value
c   if (r14 != r13) {                   # Value has changed
        stob    r14,D_ften              # Save the new value
        call    D$SndFTOO               # Update FE Foreign Target value
        call    D$p2updateconfig        # Update NVRAM
c   }
#
        ldconst deok,g1                 # Return OK status
        ldconst mftrsiz,g2              # Set return packet size
        ret
#
#**********************************************************************
#
#  NAME: d$createvlink
#
#  PURPOSE:
#       To provide a means for the CCB to create a link to a virtual
#       disk on a remote controller.
#
#  DESCRIPTION:
#       The virtual disk ordinal value is used to determine the
#       virtual disk to be connected to this vlink. A virtual disk
#       of type vlink is created locally and linked to the virtual
#       disk indicated by the controller and vdisk ordinal.
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = status
#       g2 = return pkt size
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
d$createvlink:
        ld      mr_ptr(g0),r15          # Parm block address
        ld      mr_rptr(g0),r12         # Return block address
#
        ldob    D_Vflag,r7              # r7 = current VDisk/VLink busy flag
#
# --- Check the VID and grab a RID to use for this vdisk.
#
        ldconst dedevused,g1            # Prep possible error code
        ldconst MAXVIRTUALS,r6          # Maximum value allowed plus one
        ldos    mcv_rvid(r15),g3        # Get the VID to be used
        cmpobge g3,r6,.cvl1000          # Jif out of range
#
        ld      V_vddindx[g3*4],r5      # Get VDD
        cmpobne 0,r5,.cvl1000           # Jif non-NULL found (VID in use)
#
        ldconst deinstable,g1           # Prep no table space error code
        ldconst MAXRAIDS,r6             # End point
        ldconst 0,r13                   # Prep RID
#
.cvl50:
        ld      R_rddindx[r13*4],r3     # Get VDD
        cmpobe  0,r3,.cvl60             # Jif one found
#
        addo    1,r13,r13               # Bump index
        cmpobne r6,r13,.cvl50           # Jif more to do
        b       .cvl1000                # Jif no more RAIDs to use
#
# --- VID is in g3, RID is in r13. Validate parameters.
#
.cvl60:
        mov     g3,r14                  # Save the VID
        ldconst TRUE,g1
        ldconst 0,g2                    # Assume cluster zero for Mag compat
        stos    g3,D_Vvid               # Save VDisk #
        stob    g2,D_Vcl                # Save cluster #
        stob    g1,D_Vflag              # Set flag indicating VDisk/VLink busy
#
# --- Establish linked device
#
        ldos    mcv_vordinal(r15),g0    # g0 = storage unit data index value
        ldos    mcv_cid(r15),g1         # g1 = storage unit list index value
        ldconst 0x80,g4                 # Set exclusive flag
        call    DLM$est_vl              # Call routine to process the request
                                        # g0 = LDD address if successful
                                        # g1 = completion status
        cmpobne 0,g1,.cvl1000           # Jif error establishing linked device
        mov     g0,g1                   # g1 = LDD address of linked device
#
# --- Assign and initialize RDD
#
        movl    g2,r4                   # r4 = cluster #
                                        # r5 = VDisk #
        mov     r13,g2                  # g2 = RAID ID to initialize
        ldconst rdlinkdev,g3            # g3 = RAID type code to initialize
        call    d$brddpsdld             # build and initialize RDD/PSD/RDI
                                        # g0 = RDD address of RAID
#
        st      g0,R_rddindx[r13*4]     # Save RDD address in RAID table
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        ldos    R_rddindx+rx_ecnt,r3    # Get RAID count
        addo    1,r3,r3                 # Bump it
        stos    r3,R_rddindx+rx_ecnt    # Set new RAID count
#
c       *(UINT64 *)&r4 = ((LDD *)g1)->devCap; # Linked device capacity
c       ((RDD *)g0)->devCap = *(UINT64 *)&r4; # Save Raid device capacity in RDD.
c       ((RDD *)g0)->vid = r14;         # Save VID in RDD
c       ((RDD *)g0)->extension.pPSD[0]->sLen = *(UINT64 *)&r4;
# Save segment length of PSD
        mov     g0,r13                  # Save the RDD pointer
#
# --- Assign and initialize VDD
#
        call    D_allocvdd              # Get a VDD into g0
        mov     g0,r8                   # r8 = VDD address
        st      r8,V_vddindx[r14*4]     # Link VDD to VDX
        ldos    V_vddindx+vx_ecnt,r3    # Get vdisk count
        addo    1,r3,r3                 # Bump it
        stos    r3,V_vddindx+vx_ecnt    # Set new vdisk count
#
# Save VDisk capacity in VDD
c       ((VDD *)r8)->devCap = *(UINT64 *)&r4;
        stos    r14,vd_vid(r8)          # Set up virtual ID
        ldconst vdop,r3                 # Set up operational device status
        stob    r3,vd_status(r8)
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
#
.if 1 # Disable or remove this piece of code to fix SAN-126/SAN-735
#
# -- Note :- When remove this call (when fixing SAN-126/SAN-735),ensure that
#            the following function will not be called for vlinked vdisks in
#            RB_setraidstat(rebld.as) function also in addition to here.
        PushRegs(r5)
c       GR_UpdateVdiskOpState((VDD *)r8, 0, (UINT8)r3);
        PopRegsVoid(r5)
.endif
        ldconst 1,r3                     # Set up segment count
        stob    r3,vd_raidcnt(r8)
        st      r13,vd_rdd(r8)           # Save the RDD pointer
#
        ldos    vd_attr(r8),r5           # VDD attribute
        setbit  vdbvlink,r5,r5           # Set VLink flag
        ldconst vdvdscd+vdvddcd+vdvdsusp,r3 # make sure these are cleared
        andnot  r3,r5,r5
        stos    r5,vd_attr(r8)          # Save updated attribute byte
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
#
# --- Update the remote cache
#
        mov     r14,g0
        ldconst FALSE,g1                # Addition of a new VDD (not deleting)
        call    D$updrmtcachesingle     # Update single
#
# --- Update NVRAM
#
        call    D$p2updateconfig        # Update NVRAM part II
#
# --- Set up return block
#
        stos    r14,mcv_vid(r12)        # VID created
#
# --- Copy the controller name.
#
        ldos    mcv_cid(r15),r3         # Storage unit list index value
        ld      vl_sulst,r5             # Get address of controller area
        lda     vl_sulh_siz(r5),r5      # Get into the data area
#
        ldconst vl_sulr_siz,r4          # Size of one record
        mulo    r4,r3,r4                # Offset into the data area
        addo    r4,r5,r5                # r5 points to the record for name
#
        ldq     vl_sulr_name(r5),r8
        stq     r8,mcv_ctrlname(r12)    # Store it
        ld      vl_sulr_name+16(r5),r8
        st      r8,mcv_ctrlname+16(r12) # Store it
#
# --- Copy the vdisk name.
#
        ldos    mcv_vordinal(r15),r3    # Storage unit data index value
        ld      vl_sudata,r5            # Get address of data area
        lda     vl_sudh_siz(r5),r5      # Get into the data area
#
        ldconst vl_sudr_siz,r4          # Size of one record
        mulo    r4,r3,r4                # Offset into the data area
        addo    r4,r5,r5                # r5 points to the record for name
        ldconst 12,r6                   # Words to copy less one
.cvl200:
        ld      vl_sudr_devsn(r5)[r6*4],r8
        st      r8,mcv_vdname(r12)[r6*4]# Store it
        subo    1,r6,r6                 # Decrement the count of words
        cmpible 0,r6,.cvl200            # Jif more to do
#
        mov     deok,g1                 # Return OK status
#
# --- Exit
#
.cvl1000:
        stob    r7,D_Vflag              # restore VDisk/VLink busy flag
#
        ldconst mcvrsiz,g2              # Set return packet size
        ret
#
#**********************************************************************
#
#  NAME: d$brddpsdld
#
#  PURPOSE:
#       To provide a common means of building the RDD and PSDs associated
#       with the specified linked device.
#
#  DESCRIPTION:
#       An RDD is assigned from local SRAM. A PSD entry is then assigned
#       from local SRAM for the linked device. The common fields
#       within the RDD and PSD are initialized.
#
#  INPUT:
#       g1 = LDD address of linked device to use
#       g2 = RAID ID
#       g3 = RAID type
#
#  OUTPUT:
#       g0  = RDD
#
#  REGS DESTROYED:
#       Reg. g0 destroyed.
#
#**********************************************************************
#
d$brddpsdld:
        movl    g0,r14                  # Save g0-g1
#
# --- Allocate RDD and link to RDD index
#
        ldconst 1,g0                    # Allocate an RDD with one PSD
        call    D_allocrdd              # g0 = RDD address
        mov     g0,r14                  # r14 = RDD
#
# --- Initialize RDD
#
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */

        stob    g3,rd_type(g0)          # Set up RAID type
        ldconst rdop,r3                 # Set status to operational
        stob    r3,rd_status(g0)
        ldconst 1,r10                   # r10 = drive count
        stos    r10,rd_psdcnt(g0)       # Set up drive count
        stos    g2,rd_rid(g0)           # Save RAID ID
#
# --- Allocate PSD
#
        call    D_allocpsd              # PSD is in g0
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        st      g0,rd_psd(r14)          # Link PSD to RDD
        st      g0,ps_npsd(g0)          # Link PSD to itself
#
# --- Initialize PSD
#
        cmpobe  0,r15,.brd100           # Jif no LDD
        ldos    ld_ord(r15),r4          # Get the ordinal for the LDD
        stos    r4,ps_pid(g0)           # Save it in the PID value
.brd100:
        ldconst psop,r4                 # Set status to operational
        stob    r4,ps_status(g0)
        movl    0,r4
        stl     r4,ps_rlen(g0)          # Clear rebuild length
        stos    g2,ps_rid(g0)           # Set up RAID ID
#
# --- Exit
#
        movl    r14,g0                  # Restore g0-g1
        ret
#
#**********************************************************************
#
#  NAME: d$getvlinkinfo
#
#  PURPOSE:
#       To provide a means for the CCB to obtain information regarding
#       a virtual disk which is vlinked to another controller.
#
#  DESCRIPTION:
#       The virtual disk number is used to gather together information
#       regarding this virtual link. The RDD is parsed to find the LDD
#       and the LDD information is placed into the return data along
#       with interesting fields from the VDD and RDD.
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = status
#       g2 = return pkt size
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
d$getvlinkinfo:
#
# --- First, grab the return data address and length allowed.
#
        ld      mr_rptr(g0),r14         # Return data pointer
#
        ldconst MAXVIRTUALS,r10         # Max virtual device ID
        ldconst mrrsiz,g2               # Set up return size
#
        ld      mr_ptr(g0),g0           # Parm block address
        ldos    mvl_vid(g0),r12         # Virtual device ID
#
# --- Validate parms
#
        ldconst deinvvid,g1             # Load error code
        cmpoble r10,r12,.gvl100         # Branch if too big VID
#
        lda     V_vddindx,r11           # Base address of the VDX
        ld      vx_vdd(r11)[r12*4],r8   # Get the VDD
        cmpobe  0,r8,.gvl100            # Null pointer, invalid Vdisk
#
# --- Verify that this is a VLink VID.
#
        ldos    vd_attr(r8),r3          # Get attributes
        bbc     vdbvlink,r3,.gvl100     # Jif not set
#
# --- Fill in the structure with the statistical information. This is
# --- all of the data up to the RAID pointers.
#
        mov     r8,r9                   # Source pointer
        lda     mvl_data(r14),r10       # Destination pointer
        ldconst vd_drdd/16,r11          # Quad counter
#
.gvl10:
        ldq     (r9),r4                 # Get quad of virtual device data
!       stq     r4,(r10)                # Store it
#
        addo    16,r10,r10              # Move the destination pointer
        lda     16(r9),r9               # Move the source pointer
        subo    1,r11,r11               # Decrement count of quads
        cmpobne 0,r11,.gvl10            # Jif more to do
#
# --- Grab the RDD and pull that data. Start at data plus 80.
#
        ld      vd_rdd(r8),r8           # Pointer to the RAID device
        mov     r8,r9                   # Source pointer
        ldconst rd_psd/16,r11           # Quad count
#
.gvl20:
        ldq     (r9),r4                 # Get quad of RAID device data
!       stq     r4,(r10)                # Store it
#
        addo    16,r10,r10              # Move the destination pointer
        lda     16(r9),r9               # Move the source pointer
        subo    1,r11,r11               # Decrement count of quads
        cmpobne 0,r11,.gvl20            # Jif more to do
#
# --- Get the LDD and dump it into the structure for the return data.
#
        ld      rd_psd(r8),r8           # Pointer to first PSD
        ldos    ps_pid(r8),r8           # LDD ID
        ld      DLM_lddindx[r8*4],r9    # LDD - source pointer
        ldconst ld_tpmthd/16,r11        # Quad count
        cmpobe  0,r9,.gvl100            # Null pointer, invalid LDD
#
.gvl30:
        ldq     (r9),r4                 # Get quad of LDD device data
!       stq     r4,(r10)                # Store it
#
        addo    16,r10,r10              # Move the destination pointer
        lda     16(r9),r9               # Move the source pointer
        subo    1,r11,r11               # Decrement count of quads
        cmpobne 0,r11,.gvl30            # Jif more to do
#
        ldconst deok,g1                 # Load good return error code
#
# --- Exit
#
.gvl100:
        ldconst mvlrsiz,g2              # Set return packet size
        ret
#
#**********************************************************************
#
#  NAME: d$getlocalimage
#
#  PURPOSE:
#       To provide a means for the CCB to fetch the local image of
#       NVRAM from the BE.
#
#  DESCRIPTION:
#       The BE will create the local image in the memory pointed to
#       by the MRP input.
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = status
#       g2 = return pkt size
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
d$getlocalimage:
        ld      mr_rptr(g0),r15         # Return parm address
        ld      mr_ptr(g0),r14          # Parm block address
#
# --- Calculate the size of the buffer needed and if there is not enough
# --- room, report back an error.
#
        PushRegs(r3)                    # Save register contents
        call    NV_CalcLocalImageSize   # Get the size required
        PopRegs(r3)                     # Restore registers (except g0)
        mov     g0,r13                  # Save it for later
#
        ld      mgn_len(r14),r3         # Get space allocated
        ldconst detoomuchdata,g1        # Prep error code
        cmpobl  r3,r13,.gli100          # Jif too small
#
        ld      mgn_addr(r14),g0        # Buffer address of image in CCB memory
        PushRegs(r3)                    # Save register contents
        call    NV_BuildLocalImage      # Get the local image
        PopRegsVoid(r3)                 # Restore registers
        ldconst deok,g1                 # Set return packet size
#
.gli100:
        ldconst mgnrsiz,g2              # Set return packet size
!       st      r13,mgn_rlen(r15)       # Save size regardless of error code
        ret
#
#**********************************************************************
#
#  NAME: d$putlocalimage
#
#  PURPOSE:
#       To provide a means for the CCB to put a local image into
#       NVRAM.
#
#  DESCRIPTION:
#       The BE will take the local image in the memory pointed to
#       by the MRP input and place it into the NVRAM.
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = status
#       g2 = return pkt size
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
d$putlocalimage:
        ld      mr_ptr(g0),g0           # Parm block address
        ld      mun_addr(g0),g0         # Controller serial number
#
        PushRegs(r3)                    # Save register contents
        call    NV_UpdateLocalImage     # Put the local image into NVRAM
        PopRegsVoid(r3)                 # Restore registers
        call    d$labupdate             # Update device labels
#
        ldconst deok,g1                 # Set return packet size
        ldconst mgnrsiz,g2              # Set return packet size
        ret
#
#**********************************************************************
#
#  NAME: d$rescanDeviceTask
#
#  PURPOSE:  The function initiates the rescanning of physical devices.
#
#  DESCRIPTION: The monitor loop process is started. This process
#               notifies online of the list devices attached to
#               the back end. When the list has been sent, this task
#               will initiate the link layer completion and then die.
#
#  CALLING SEQUENCE:
#       fork    d$rescanDeviceTask
#
#  INPUT:
#       g2 = MRP
#       g3 = ILT
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
d$rescanDeviceTask:
        movl    g2,r14                  # Save g2 and g3
#
        ld      mr_ptr(g2),g0           # Parm block address
        ldob    mrd_type(g0),g0         # Rescan type
#
        cmpobge mrdrediscover,g0,.drd10 # Jif input parm is OK
        ldconst deinvopt,g1             # Invalid option
        b       .drd100                 # Return
#
.drd10:
        call    P$rescanDevice          # returns g1 = status
        ldconst deok,g1                 # Load good return error code
#
# --- We have finished either successfully or not. Record the completion
# --- in the define trace log and then send the completion via the link
# --- layer.
#
.drd100:
        ldconst trMRPDefComp,g0         # Trace end of MRP
        mov     g1,r4                   # Save g1
        shlo    8,g1,g1                 # Shift return status code 1 byte
        ldconst mrrescandevice-mrebfuncbase,r3
        or      r3,g1,g1                # OR it to the MRP command number
        call    D$TraceEvent
        mov     r4,g1                   # Restore g1
#
        ld      mr_rptr(r14),r3         # Get the return data ptr
!       stob    g1,mr_status(r3)        # Plug return status code
        ldconst mrdrsiz,g2              # Set return packet size
!       st      g2,mr_rlen(r3)          # Set return packet size
#
        mov     r15,g1                  # Complete this request
        call    K$comp
#
        ldconst 0,r3
        st      r3,D_rescanDeviceTaskPCB # Debug purposes
        ret
#
#**********************************************************************
#
#  NAME: d$rescanDevice
#
#  PURPOSE:  The function initiates the task to rescan physical devices.
#
#  DESCRIPTION: The task to complete the rescan is started and control
#            is returned to the define exec. The define exec will not
#            call the link layer completion function. It is up to the
#            task to do that.
#
#  INPUT:
#       g0 = MRP
#       g14 = ILT
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
d$rescanDevice:
        mov     g0,g2                   # g2 gets the MRP pointer
        mov     g14,g3                  # g3 gets the ILT
#
        lda     d$rescanDeviceTask,g0   # Task address
        ld      K_xpcb,r3
        ldob    pc_pri(r3),g1           # Priority inherited from parent
#
c       CT_fork_tmp = (ulong)"d$rescanDeviceTask";
        call    K$tfork                 # Fork it and leave
        st      g0,D_rescanDeviceTaskPCB # Debug purposes
c       g1 = deok;
        ret
#
#**********************************************************************
#
#  NAME: d$targetcontrol
#
#  PURPOSE:  This function handles the BE processing to prepare for
#            a target move and complete a target move. In general, it will
#            halt all background operations before a target movement and
#            resume them after the move is complete. This includes
#            initializations, scrubbing, rebuilds, copies.
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = status: either deinvopt or deok
#       g2 = return pkt size
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
d$targetcontrol:
        ld      mr_ptr(g0),g0           # Parm block address
        ldos    mtc_option(g0),r3       # Option
#
        cmpobe  mtcprepmove,r3,.dtc10   # Jif about to move targets
        cmpobe  mtccompmove,r3,.dtc100  # Jif move targets completed
#
        ldconst deinvopt,g1             # Invalid option
        b       .dtc1000                # Return
#
# --- Prepare to move targets -----------------------------------------
#
#     This controller may be performing operations on VDisks and RAIDs
#     that are about to be moved to a different controller.
#
.dtc10:
        ldconst TRUE,r3
        st      r3,D_moveinprogress     # Indicate a move is in-progress
#
# --- Stop I/O
#
        setbit  mxcwait,0,g0            # Wait for cache stop to complete
        ldconst mritgtctrl,g1           # g1 = Target Control User ID
        call    d$cachestop             # Stop I/O activity
#
# --- Initialization
# --- Parity/Mirror Scan
# --- Rebuild
#
### PushRegs and PopRegs are added around this C call on wookiee..V1-- Sep2
        PushRegs(r3)
#
        call    DEF_TerminateBackground
        PopRegsVoid(r3)
#
# --- Scrubbing
#
        call    R$stop                  # Stops scrubbing
#
# --- Caching
#     The move target operation is controlled directly from the RM with
#     individual MRPs to the FE & BE so no code is needed here....I think.
#
        ldconst deok,g1                 # Load good return error code
        b       .dtc1000                # Return
#
# --- Target move has completed ---------------------------------------
#     Operations that had been in-progress need to be resumed.
#
.dtc100:
        ldconst FALSE,r3
        st      r3,D_moveinprogress     # Indicate a move is not in-progress
#
# --- Get the file system updates going if needed.
#
c       TaskReadyByState(pcfscleanup);  # Enable tasks waiting for file system cleanup
#
# --- RAID Initialization
#
        PushRegs(r3)                    # Save register contents
        call    DEF_CheckRaidStates     # Start any needed initializations
        PopRegsVoid(r3)                 # Restore registers
#
# --- Scrubbing
#
        call    R$resume                # Restarts scrubbing
#
# --- Parity/Mirror Scan
#
        ld      R_pcctrl,r3             # Get parity checker control word
        setbit  rdpcnewcmd,r3,r3        # Indicate a new parity command
        setbit  rdpcmarked,r3,r3        # Indicate there is a marked RDD
        clrbit  rdpcspecific,r3,r3      # Don't do a specific RID
        setbit  rdpc1pass,r3,r3         # Just do 1 pass
        setbit  rdpccorrect,r3,r3       # Correct any out of sync parity stripes
        setbit  rdpcenable,r3,r3        # Enable parity checking
        st      r3,R_pcctrl
#
# --- Rebuild
#
        call    RB_setpsdstat           # Update latest PSD & RAID states
        call    RB_searchforfailedpsds  # Attempt to hotspare failed PSDs
        PushRegs(r3)                    # Save register contents
        call    DEF_UMiscStat           # Update the misc status'
        PopRegsVoid(r3)                 # Restore registers
#
        call   CCSM$cco                 # issue configuration change occurred
#
# @@@ FINISH @@@
#
#        ldconst MAXVIRTUALS-1,r15       # Get index into VDD list
#
#.dtc180:
#        ld      V_vddindx[r15*4],r14    # Get VDD
#        cmpobe  0,r14,.dtc190           # Jif NULL pointer (no VDD)
#
#        ld      vd_cpscmt(r14),g4       # Get SCMT if this is a destination
#        cmpobe  0,g4,.dtc190            # Jif no copy in progress
#
# --- If the copy is not in the mirrored state, restart it.
#
#        ldob    vd_mirror(r14),r3       # Get the state
#        cmpobe  vdcopymirror,r3,.dtc190 # Jif mirrored
#
#        call    V_screstart             # Restart the process
#
#.dtc190:
#        subo    1,r15,r15               # Decrement VDD index
#        cmpible 0,r15,.dtc180           # Jif more to do
#
# --- Start I/O
#
        ldconst mccclearall,g0          # g0 = all stop counts for this user
        ldconst mritgtctrl,g1           # g1 = Target Control User ID
        call    d$cacheresume           # Resume I/O activity
#
# --- Caching
#
        ldconst deok,g1                 # Load good return error code
#
# --- Return
#
.dtc1000:
        ldconst mtcrsiz,g2              # Set return packet size
        ret
#
#**********************************************************************
#
#  NAME: d$resync
#
#  PURPOSE:
#       The function initiates the resync of stripes or RAIDs.
#       This is initiated by an MRP from the CCB.
#
#  DESCRIPTION:
#       This MRP will either pass a copy of the FE part 4 NVA area,
#       a RAID ID, a flag to indicate all RAIDs on the device, a flag to
#       indicate all RAIDs that are in the "Not Mirroring" Astatus state, or
#       a List of RAIDs provided should be resync'ed.
#
#       When the FE NVA area is sent, the NVA records are converted from
#       VIDs to RIDs and put into the BE part 4 NVA area. A stripe resync
#       is kicked off in the background. If the BE Part 4 NVA area runs out
#       of room, the rest of the NVA records will be used to flag the RAID as
#       needing a Full Resync.
#
#       For a single RAID, all RAIDs, "Not Mirroring RAIDS, and the list of
#       provided RAIDs, the RDD is marked to be resync'ed and the parity scan
#       task is flagged to scan all marked RDDs.
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = status = deok, deinvrid, or dechecksum
#       g2 = return pkt size
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
d$resync:
        mov     g0,r14                  # Save regs
        mov     g13,r15
#
        ld      d_resync_paskey,r3      # r3 = current pass key
        addo    1,r3,r3                 # increment it
        st      r3,d_resync_paskey      # save new pass ket
        ldconst 0,r3                    # clear r3
        st      r3,d_resync_list        # clear list
#
        ld      mr_ptr(g0),g0           # Parm block address within MRP
        ldob    mrb_type(g0),r3         # Get type
        cmpobe  mrboneraid,r3,.drs200   # Jif one RAID
        cmpobe  mrballraids,r3,.drs300  # Jif all RAIDs
        cmpobe  mrballnotmirror,r3,.drs400 # Jif all RAIDs in "Not Mirroring"
        cmpobe  mrblistraids,r3,.drs500 # Jif a list of RIDs provided
#
# --- Stripe Resync -----------------------------------------------------------
#       Sync the stripes that were mirrored in the FE
#       A stripe resync gets a copy of the FE SNVA in the CCB DRAM.
#       Each records has a VID, SDA, and length for a write operation as
#       seen by the cache layer.
#
        ld      mrb_nva(g0),r12         # Pointer to NVA in CCB DRAM
!       ld      nv_csum(r12),r11        # Get P4 checksum
        ldconst nvabasesiz,r3           # Get header size
        addo    r3,r12,r12              # Point to first record
#
# --- Copy from CCB DRAM to local DRAM and verify checksum
#       We want to operate on a copy of the NVA records in our local DRAM.
#       In case the CCB dies while we're processing the records we don't
#       want to be loading a bunch of bad data from across the PCI bus. This
#       also allows the CCB to free that buffer as soon as this command
#       returns while the proc continues to process the data in the background.
#
c       r13 = s_MallocC(gMPNvramP4Size, __FILE__, __LINE__); # Assign memory and clear it
c       r10 = NUM_OF_P4_NVA_WK;
        ldconst 0,r8                    # Initialize checksum
#
.drs10:
        subo    1,r10,r10               # Decrement loop count
!       ldq     (r12)[r10*16],r4        # Read source data from CCB
        stq     r4,(r13)[r10*16]        # Copy data to local DRAM
#
        addo    r4,r8,r8                # Calculate checksum
        addo    r5,r8,r8
        addo    r6,r8,r8
        addo    r7,r8,r8
#
        cmpobne 0,r10,.drs10            # Jif not done
#
# --- Compare checksums
#
        ldconst dechecksum,g1           # Set bad buffer checksum
        cmpobne r8,r11,.drs1010         # Jif checksums don't match
#
# --- Top of loop to scan each record ---
#
# --- Allocate temp nva structure to add records into NVRAM
#
c       g13 = s_MallocC(nvasiz, __FILE__, __LINE__); # Assign memory and clear it
#
c       r10 = NUM_OF_P4_NVA_WK;
        ldconst FALSE,r9                # Flag to show not out of room in NVA
#
# --- Determine if VID now belongs to this controller
#
.drs20:
        subo    1,r10,r10               # Decrement loop count
#
        ld      nv_length(r13)[r10*16],r6 # r6 = Length
        cmpobe  0,r6,.drs95             # Jif record not used: do next one
        ldos    nv_id(r13)[r10*16],g0   # Get Virtual ID
        ld      V_vddindx[g0*4],r12     # r12 = VDD
        cmpobe  0,r12,.drs95            # Jif VID is no longer valid
#
        PushRegs(r3)                    # Save the registers
        call    DL_AmIOwner             # Does RDD belong to this controller?
        PopRegs(r3)                     # Restore the registers
#
                                        # g13 points to nva struct
        cmpobe  FALSE,g0,.drs95         # Jif VDD does NOT belong here:
                                        #  move on to next record
        ld      vd_scdhead(r12),r11     # r11 = possible SCD address
#
# --- Determine which RID(s) this write applied to
#
.drs21:
        ld      nv_length(r13)[r10*16],r6 # r6 = Length (reload - modified below)
        ld      vd_rdd(r12),r7          # r7 = RDD associated with VDisk
        cmpobne FALSE,r9,.drs23         # Jif no room in NVRAM
#
# --- Search for RAID segment with starting SDA, and up through EDA.
#
c   {
c       UINT64 vdlsda = ((NVA *)(r13+r10*16))->lsda;
c
c       while (((RDD *)r7)->devCap < vdlsda) {
c         vdlsda = vdlsda - ((RDD *)r7)->devCap;
c         r7 = (UINT32)((RDD *)r7)->pNRDD;  # Link to next RAID segment
c       }
#
# --- Setup nva struct
#
c       do {
c         ((NVA *)g13)->lsda = vdlsda;  # RAID Logical SDA
c         ((NVA *)g13)->id = ((RDD *)r7)->rid;    # RAID ID
c         if (vdlsda + r6 <= (((RDD *)r7)->devCap)) { # if EDA (SDA + length) <= RAID size
c           ((NVA *)g13)->length = r6;  # All length contained in this RAID
c           r5 = 0;                     # Flag that we are done with operation.
c         } else {
# --- Handle length that extends into the next RAID device
c           r5 = ((RDD *)r7)->devCap - vdlsda; # Know that length can't be very big.
c           ((NVA *)g13)->length = r5;  # Length contained within this RAID
c           r6 = r6 - r5;               # Adjust length for next loop
c           vdlsda = 0;                 # Reset Starting VDisk Address to 0 (next raid)
c         }
# --- Add record to local NVA NVRAM
c         if (((RDD *)r7)->type == rdraid5) { # Only use RAID 5 devices
            ld      P4_nvac+nc_cur,r3   # Get the number of entries left
c           if (r3 == 0) {
c             r9 = TRUE;                # Set Flag to show all future entries
              b       .drs23            #  need full resync and go do this one
c           }
# g13 points to nva structure
            call    M$ap4nva            # Add record to SNVA in NVRAM
                                        # g0 will be destroyed
c         }
c         if (r5 == 0) {                # If we are finished with operation, exit.
c           break;
c         }
c         r7 = (UINT32)((RDD *)r7)->pNRDD; # Link to next RAID segment
c       } while (r7 != 0);
c   }
        b       .drs90                  # Finish processing
#
# --- Resync all RAIDs associated with the VID in the list
#
.drs23:
c       do {
c         if (((RDD *)r7)->type == rdraid5) { # Only use RAID 5 devices
c           g0 = r7;                    # Set up to mark this RAID for Resync
c           g1 = SINGLE_RDD;            # Mark only this RAID
            call    O$markraidscan      # Mark RDD to get scanned
c         }
c         r7 = (UINT32)((RDD *)r7)->pNRDD; # Link to next RAID segment
c       } while (r7 != 0);
#
# --- determine if the current destination is a source of a copy
#
.drs90:
        cmpobe  0,r11,.drs90d0           # Jif no more copy/mirror records for this vdd
#
.drs90a0:
        ld      scd_cor(r11),r7         # r7 = cor address
        ld      cor_destvdd(r7),r12     # get destination vdd
        cmpobe  0,r12,.drs90a2          # Jif no dest vdd
#
        ld      d_resync_paskey,r3      # r3 = current pass key
        ld      cor_r5PasKey(r7),r4     # r4 = cor pass key
        cmpobe  r3,r4,.drs90a2          # Jif we've seen this cor before
#
        ld      vd_scdhead(r12),r4      # r4 = possible source from this destination
        cmpobne  0,r4,.drs90b0          # Jif this destination is also a source device
#
.drs90a2:
        ld      scd_link(r11),r11       # Point to the next copy
        b       .drs90                  # continue
#
.drs90b0:
        st      r3,cor_r5PasKey(r7)     # save in cor
        ld      scd_link(r11),r11       # Point to the next copy
        cmpobe  0,r11,.drs90c0          # Jif no forward link
#
        ld      d_resync_list,r5        # get top of list
        st      r11,d_resync_list       # save new head
        st      r5,scd_rlink(r11)       # save link to previous
#
.drs90c0:
        mov     r4,r11                  # save new scd address
        ld      scd_vdd(r11),r12        # get associated VDD address
#        ld      scd_link(r11),r11       # Point to the next copy
        b       .drs21                  # continue
#
.drs90d0:
        ld      d_resync_list,r11       # was there a previous branch?
        cmpobe  0,r11,.drs95            # Jif none
#
        ldconst 0,r3                    # clear r3
        ld      scd_rlink(r11),r4       # get link to net entry on list
        st      r4,d_resync_list        # save as new top
        st      r3,scd_rlink(r11)       # clear link
        b       .drs90a0                # continue
#
# --- determine if there are other records to check
#
.drs95:
        cmpobne 0,r10,.drs20            # Jif not done looping all records
#
# --- Release DRAM NVA structure and NVA List
#
c       s_Free(g13, nvasiz, __FILE__, __LINE__); # Release DRAM - NVA Structure
c       s_Free(r13, gMPNvramP4Size, __FILE__, __LINE__); # Release DRAM - NVA List
#
# --- Call routine to recover all writes in SNVA NVRAM
#
        call    O$recoverp4             # Kick off the stripe resync task
                                        # g0 returns with error code
        cmpobe  ecok,g0,.drs1000        # Jif good
        ldconst dechecksum,g1           # Set bad checksum return code
        b       .drs1010                # Return
#
# --- Resync one RAID ---------------------------------------------------------
#
.drs200:
        ldos    mrb_raidid(g0),r11      # Get RID
#
        ldconst MAXRAIDS-1,r4           # Max RAID ID allowed
        ldconst deinvrid,g1             # RAID ID is too big
        cmpobg  r11,r4,.drs1010         # Jif RID > maximum: return error
#
        ld      R_rddindx[r11*4],g0     # RDD pointer
        cmpobe  0,g0,.drs1000           # Jif RDD does not exist: return good
#
        ldconst SINGLE_RDD,g1           # Do only this RDD
        call    O$markraidscan          # Mark RDD to get scanned
        b       .drs1000                # Return good
#
# --- Resync all RAIDs --------------------------------------------------------
#
.drs300:
        ldconst ALL_RDDS,g1
        call    O$markraidscan          # Mark all RDDs to get scanned
        b       .drs1000                # Return good
#
# --- Resync all RAIDs in the "Not Mirroring" Astatus State -------------------
#
.drs400:
        ldconst ALL_NOT_MIRROR_RDDS,g1
        call    O$markraidscan          # Mark all RDDs in "Not Mirroring"
        b       .drs1000                # Return good
#
# --- Resync all RAIDs in the list provided -----------------------------------
#
#   Verify all the passed in RAID IDs for correctness
#
.drs500:
        ldos    mrb_numrids(g0),r5      # Number of RIDs passed in
        ld      mrb_ridlist(g0),r6      # Get the Pointer to the list
        ldconst MAXRAIDS-1,r4           # Max RAID ID allowed
        cmpobe  0,r5,.drs1000           # No RAIDs passed in - all done good!
        ldconst deinvrid,g1             # RAID ID is invalid
        mov     r5,r10                  # Save number of RIDs passed in
.drs510:
        subo    1,r5,r5                 # Use the number as an index in list
!       ldos    (r6)[r5*2],r7           # Get the RID
        cmpobg  r7,r4,.drs1010          # Jif RID > maximum: return error
#
        ld      R_rddindx[r7*4],r8      # Get the RDD to see if it exists
        cmpobe  0,r8,.drs1010           # Jif no RAID exists
#
        ldos    rd_vid(r8),g0           # Get the VID
#
        PushRegs(r3)                    # Save the registers
        call    DL_AmIOwner             # Does RDD belong to this controller?
        PopRegs(r3)                     # Restore the registers
#
        cmpobe  FALSE,g0,.drs1010       # Jif not a RAID 5 this controller owns
#
        cmpobne 0,r5,.drs510            # Jif more RAID IDs to validate
#
#   All the RAID IDs are valid - go do the marking
#
        mov     r10,g0                  # g0 = Number of RAIDs in list
        ldconst LIST_RDDS,g1            # g1 = Type of marking (List)
        mov     r6,g2                   # g2 = Pointer to the List
        call    O$markraidscan          # Mark all RDDs in the List
                                        # Fall through to return good
#
# --- Exit --------------------------------------------------------------------
#
.drs1000:
        ldconst deok,g1                 # Load good return error code
#
.drs1010:
        ldconst mrbrsiz,g2              # Set return packet size
#
        mov     r14,g0                  # restore regs
        mov     r15,g13
        ret
#
#**********************************************************************
#
#  NAME: d$savemirrorpartner
#
#  PURPOSE:
#       To provide a common means of saving the mirror partner passed
#       from the FEP.
#
#  DESCRIPTION:
#       The mirror partner serial number is received in the MRP. This
#       value is saved in the FICB and a save to NVRAM is done. This
#       causes the FICB value to be placed into NVRAM.
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = status
#       g2 = return length
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
d$setmpconfigbe:
        PushRegs(r3)                    # Save register contents
c       MP_SetMPConfigBE((MR_PKT*)g0);
        PopRegsVoid(r3)                 # Restore registers
        ldconst deok,g1                 # Return status
        ldconst msmprsiz,g2             # Return length
        ret
#
#**********************************************************************
#
#  NAME: DEF_TerminateBackground
#
#  PURPOSE:
#       Process a termination of all background activity on all RAIDs.
#
#  DESCRIPTION:
#       This function is called by any function when the background
#       operations on a RAID are to be halted. This includes rebuilds,
#       parity scans, defragmentations, and RAID initializations.
#
#  INPUT:
#       None
#
#  OUTPUT:
#       None
#
#**********************************************************************
#
DEF_TerminateBackground:
        movl    g0,r14                  # Save g0/g1
#
# --- Now traverse the RDD index and mark all RAIDs as requiring background
# --- termination. This will kill the RAID initializations taking place.
# --- While we are traversing the list, also kill any rebuilds going on if
# --- this is a hard termination.
#
        ldconst MAXRAIDS-1,g0           # Process all RAIDs
#
.tb130:                                 # Top of RDD loop
        ld      R_rddindx[g0*4],r13     # Get the RDD pointer
        cmpobe  0,r13,.tb160            # Jif undefined - next RDD
#
        call    RB$cancel_rebld         # Stop rebuilds for this RAID (g0=RID)
#
# --- Don't set the RAID termination bit unless we're initializing
#
        ldob    rd_status(r13),r3       # Get raid status
        cmpobe  rdinit,r3,.tb150        # Jif initializing
#
        mov     g0,r3
        mov     r13,g0
        call    D$delrip                # Delete it if it is in the rinit queue
        call    R$del_RB_qu             # Take RDD off RB_rerror_qu (in g0)
        mov     r3,g0
        b       .tb160
#
.tb150:
        ldob    rd_astatus(r13),r4      # Get additional status
        setbit  rdatermbg,r4,r4         # Set terminate background bit (init)
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        stob    r4,rd_astatus(r13)      # Save it back

#
.tb160:
        subo    1,g0,g0                 # Decrement RDD index
        cmpible 0,g0,.tb130             # Jif more to do
#
# --- Stop any parity scanning.
#
        ld      R_pcctrl,r3             # Get parity control
        clrbit  rdpcenable,r3,r3        # Disable scanning
        setbit  rdpcnewcmd,r3,r3        # Set up the new command
        st      r3,R_pcctrl             # Save it back out
#
# --- Now wait for everything to calm down. Check all RDDs for RAID
# --- initialization active. If active, wait a second and test again.
# --- Once done, exit the function.
#
.tb200:
        ldconst MAXRAIDS-1,r12          # Get MAX RID
#
.tb210:
        ld      R_rddindx[r12*4],r13    # Get RDD
        cmpobe  0,r13,.tb220            # Jif not defined
#
        ld      rd_iprocs(r13),r4       # Get initialization processes running
        cmpobe  0,r4,.tb220             # Jif not initializing
#
        ldconst 1000,g0                 # Wait one second
        call    K$twait
        b       .tb200                  # Start over in the checking
#
.tb220:
        subo    1,r12,r12               # Decrement RDD index
        cmpible 0,r12,.tb210            # Jif more to do
#
        movl    r14,g0                  # Restore g0/g1
        ret
#
#**********************************************************************
#
#  NAME: d$devicecount
#
#  PURPOSE:
#       Count all of the devices which have the serial number
#       passed in the MRP.
#
#  DESCRIPTION:
#       The PDD table is searched for matches on the serial number
#       and the count of the number of drives with that serial number
#       is returned to the caller.
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = status
#       g2 = return length
#
#  REGS DESTROYED:
#       none.
#
#**********************************************************************
#
d$devicecount:
        ld      mr_ptr(g0),r15          # Get parm block pointer
        ld      mr_rptr(g0),r14         # Get return block pointer
#
        ldconst 0,r11                   # Count of matches
#
        ldconst dedefnrdy,g1            # Prep error code
#
        ldos    K_ii+ii_status,r4       # Get initialization status
        bbc     iitpdd,r4,.mdc100       # Jif define hasn't processed PDD list
        bbc     iinvramrdy,r4,.mdc10    # Jif CCB has not set NVRAM ready
#
        ld      O_p_pdd_list,r3         # Get temp list
        cmpobne 0,r3,.mdc100            # Jif doing a loop reset operation
#
.mdc10:
        ldconst MAXDRIVES,r13           # Index into list
        ld      mdc_serial(r15),r12     # Search serial number
#
.mdc20:
        subo    1,r13,r13               # Decrement index
        ld      P_pddindx[r13*4],r3     # Get PDD
        cmpobe  0,r3,.mdc50             # Jif NULL
#
        ld      pd_sserial(r3),r3       # Get system serial number
        cmpobne r3,r12,.mdc50           # Jif not equal
#
        addo    1,r11,r11               # Else bump count
#
.mdc50:
        cmpobne 0,r13,.mdc20            # Jif more to do
#
        ldconst deok,g1
#
.mdc100:
!       st      r11,mdc_count(r14)      # Save count
        ldconst mdcrsiz,g2
        ret
#
#**********************************************************************
#
#  NAME: d$brvlock
#
#  PURPOSE:
#       Process a break VDisk/VLink lock request from the CCB.
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = return status code
#       g2 = return pkt size
#
#**********************************************************************
#
d$brvlock:
        mov     g0,r15                  # r15 = packet address
        mov     g3,r14                  # Save g3
#
# --- Check parameters for valid values.
#
        ld      mr_ptr(g0),g0           # Get parm block address
        ldos    mbv_vid(g0),r5          # r5 = specified VDisk #
#
        ldconst MAXVIRTUALS,r10         # r10 = max. # virtuals
        ldconst deinvvirtid,g1          # g1 = possible error code
        cmpoble r10,r5,.dbv100          # Jif invalid VDisk #
#
        ld      V_vddindx[r5*4],r9      # r9 = VDD address
        ldconst deinvvid,g1             # g1 = possible error code
        cmpobe  0,r9,.dbv100            # Jif no VDD defined
#
        ldconst FALSE,r3                # r3 = NVRAM update flag
#
.dbv50:
        ld      vd_vlinks(r9),r8        # r8 = first VLAR on list
        cmpobe  0,r8,.dbv80             # Jif no VLARs associated with VDD
#
        movl    r8,g0                   # g0 = VLAR to check if still in use
                                        # g1 = assoc. VDD address
        call    DLM$chk_vlock           # Check if lock is still needed
                                        # g0 = completion status
                                        #      00 = lock still needed
                                        #      01 = lock not needed
                                        #      02 = could not validate lock
                                        #           with lock owner
        ldconst dedevused,g1            # g1 = possible error code
        cmpobe  0,g0,.dbv90             # Jif lock still needed
#
        ld      vd_vlinks(r9),r7        # r7 = top VLAR on list
        cmpobne r7,r8,.dbv50            # Jif top VLAR has changed
#
# --- Check if any copy operations are associated with the disappearing
#       VLAR and if so remote suspend them.
#
        movl    g6,r6                   # save g6-g7
        mov     r8,g7                   # g7 = VLAR being disposed of
        mov     r9,g6                   # g6 = assoc. VDD address of VLAR
        call    CM$whack_rcor           # suspend any copy ops. assoc.
        ld      vlar_link(r8),r8        # r8 = next VLAR on list
        st      r8,vd_vlinks(r9)        # Save remaining list in VDD
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
.ifdef M4_DEBUG_VLAR
c fprintf(stderr, "%s%s:%u put_vlar 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g7);
.endif # M4_DEBUG_VLAR
c       put_vlar(g7);                   # Deallocate VLAR
        ldconst TRUE,r3                 # Indicate NVRAM update needed
        movl    r6,g6                   # restore g6-g7
        b       .dbv50                  # Check if more VLARs associated with VDD
#
.dbv80:
        ldos    vd_attr(r9),r4          # r4 = attribute
        clrbit  vdbvdlock,r4,r4         # clear VDisk/VLink lock flag
        stos    r4,vd_attr(r9)          # save updated attribute flag
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        mov     deok,g1                 # Return OK
#
# --- Update NVRAM only if there was an update to the structures.
#
.dbv90:
        cmpobe  FALSE,r3,.dbv100        # Jif NVRAM update not needed
        call    D$p2updateconfig        # Update NVRAM part II
#
# --- Exit
#
.dbv100:
        mov     1,g2                    # Set return packet size
        mov     r15,g0                  # restore g0
        mov     r14,g3                  # restore g3
        ret
#
#**********************************************************************
#
#  NAME: d$putscmt
#
#  PURPOSE:
#       Take a SCMT entry and update the SCMT tables internally based upon
#       the values in the SCMT entry.
#
#  DESCRIPTION:
#       This function will take one SCMT entries from the MRP
#       and process it.
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = return status code
#       g2 = return pkt size
#
#**********************************************************************
#
d$putscmt:
        ld      mr_ptr(g0),r15          # Get parm block pointer
        lda     mpc_scmt(r15),g0        # SCMT table address
#
        ldconst TRUE,g1                 # Assume master
        ldos    K_ii+ii_status,r4       # Get initialization status
        bbs     iimaster,r4,.psc10      # Jif master
        ldconst FALSE,g1                # Slave
#
.psc10:
#        call    N_processSCMT           # Process it
#
# --- Exit
#
        ldconst deok,g1
        ldconst mpcrsiz,g2
        ret
#
#**********************************************************************
#
#  NAME: d$putldd
#
#  PURPOSE:
#       Take a LDD entry and update the LDD tables internally based upon
#       the values in the LDD entry.
#
#  DESCRIPTION:
#       This function will take one LDD entry from the MRP
#       and process it.
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = return status code
#       g2 = return pkt size
#
#**********************************************************************
#
d$putldd:
        ld      mr_ptr(g0),r15          # Get parm block pointer
        lda     mpl_ldd(r15),r14        # LDD address
#
        ldos    K_ii+ii_status,r4       # Get initialization status
        bbc     iimaster,r4,.pldd100    # Jif not master
#
# --- Grab the LID out of the structure passed down and then blind copy into
# --- the LDD and save NVRAM.
#
        ldos    ld_ord(r14),r13         # Get the LID
        ld      DLM_lddindx[r13*4],r12  # Get the LDD pointer to copy into
        cmpobe  0,r12,.pldd100          # Exit if NULL
#
        ldconst ld_tpmthd/16,r3         # Get the number of quads to copy
                                        # Only copy NVRAM savable parts
#
.pldd10:
        ldq     (r14),r8                # Get a quad
        stq     r8,(r12)
#
        addo    16,r14,r14              # Bump the pointers
        lda     16(r12),r12
#
        subo    1,r3,r3                 # Decrement the counter of quads
        cmpobne 0,r3,.pldd10            # If not done, get another quad
#
        call    D$p2updateconfig        # Save to NVRAM
#
# --- Exit
#
.pldd100:
        ldconst deok,g1
        ldconst mplrsiz,g2
        ret
#
#**********************************************************************
#
#  NAME: D$SendRefreshNV
#
#  PURPOSE:
#       This routine provides a common means to refresh NRVAM image
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       None.
#
#**********************************************************************
#
# C access
# void NV_SendRefresh(void);
        .globl  NV_SendRefresh          # C access
NV_SendRefresh:
D$SendRefreshNV:
        movq    g0,r12                  # save g0-g3
#
# --- set up the packet
#
c       g0 = s_MallocC(mDGsiz, __FILE__, __LINE__); # Assign memory and clear it
        ldconst mDGodr_NVRMrefresh,r4   # load order
        stob    r4,mDG_order(g0)        # save order
#
# --- send the request
#
                                        # g0 = ptr to extention record
        ldconst mDGsiz,g1               # g1 = extention size
        ldconst ebiseDG,g2              # g2 = event type
        ldconst ebibtslave,g3           # g3 = broadcast
        call    D$reportEvent           # send packet
#
# --- release the allocated memory
#
c       s_Free(g0, mDGsiz, __FILE__, __LINE__); # Release DRAM - NVA List
        movq    r12,g0                  # restore g0-g3
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: d$ebiseDG
#
#  PURPOSE:
#       This routine processes the common entry for an ebiseDG
#       MRP and distributes it to the appropriate handler routines
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = return status code
#       g2 = return pkt size
#
#**********************************************************************
#
d$ebiseDG:
        mov     g0,r14                  # save g0
        ld      mr_len(g0),g1           # g1 = packet length
        ld      mr_ptr(g0),g0           # Get parm block
#
!       ldob    mDG_order(g0),r4        # get request order
#
# --- determine if the order is within range
#
        ldconst mDG_maxorder,r3         # r3 = maximum order
        cmpobge r4,r3,.ebiseDG_error    # Jif out of range
#
# --- the order is within range, jump to processing routine
#
        ld      .DGodr_tbl[r4*4],r5     # Get action routine
        callx   (r5)                    # Execute
                                        #   input:
#                                       #     g0 = Parm Block ptr
                                        #     g1 = packet length
                                        #   output:
                                        #     g1 = status
                                        #     g2 = response length
#
        mov     r14,g0                  # restore g0
        ret                             # return to caller
#
# --- the requested order is not not within range or is currently
#     not supported.
#
.ebiseDG_error:
        ldconst deinvpkttyp,g1          # g1 = error code
        ldconst mDGrsiz,g2              # g2 = response pkt size
        mov     r14,g0                  # restore g0
        ret
#
# --- DataGram order jump table
#
        .data
.DGodr_tbl:
        .word   .ebiseDG_error          # (x00) #
        .word   d$DG_NVRAMrefresh       # (x01) # Request NVRAM refresh
        .word   .ebiseDG_error          # (x02) # Process PCP
        .word   .ebiseDG_error          # (x03) # Update Copy Percent Complete
        .word   .ebiseDG_error          # (x04) # Start Copy
        .word   d$DG_ProcessCCBG        # (x05) # CCSM CCBGram packet
        .word   d$DG_ProcessGRSwap      # (x06) # This request(swap raids) is received from
                        # slave controller to master
        .word    d$DG_ProcessFailBack   # (x07) #
        .word    d$DG_ProcessHsInOp     # (x08) #
        .word   .ebiseDG_error          # (x09) #
        .word   .ebiseDG_error          # (x0a) #
        .word   .ebiseDG_error          # (x0b) #
        .word   .ebiseDG_error          # (x0c) #
        .word   .ebiseDG_error          # (x0d) #
        .word   .ebiseDG_error          # (x0e) #
        .word   .ebiseDG_error          # (x0f) #
#
        .text
#
#**********************************************************************
#
#  NAME: d$DG_ProcessGRSwap
#
#  PURPOSE:
#       This routine processes a Georaid Raid swap request.
#
#  INPUT:
#       g0 = MRP parameter block
#       g1 = packet length
#
#  OUTPUT:
#       g1 = return status code
#       g2 = return pkt size
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
d$DG_ProcessGRSwap:
        mov     g0,r13                  # save g0
        mov     g3,r14                  # save g3
        mov     g14,r15                 # save g14
#
        addo    g0,g1,r5                # ending address of packet information
#
# --- Precess the imbedded records
#
        ld      ccsm_e_len-CCSM_E_OFFSET(g0),r4   # r4 = length of this command
#
        mov     r4,g1                   # command length
        call    GR_RaidSwapEventHandler
#
        mov     r13,g0                  # restore g0
        mov     r14,g3                  # restore g3
        mov     r15,g14                 # restore g14
#
        mov     ecok,g1                 # g1 = completion status
        ldconst mDGrsiz,g2              # g2 = response pkt size
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: d$DG_ProcessFailBack
#
#  PURPOSE:
#       This routine processes a FailBack/AutoFailBack request.
#
#  INPUT:
#       g0 = MRP parameter block
#       g1 = packet length
#
#  OUTPUT:
#       g1 = return status code
#       g2 = return pkt size
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
d$DG_ProcessFailBack:
        mov     g0,r13                  # save g0
        mov     g3,r14                  # save g3
        mov     g14,r15                 # save g14
#
        addo    g0,g1,r5                # ending address of packet information
#
# --- Precess the imbedded records
#
        ld      ccsm_e_len-CCSM_E_OFFSET(g0),r4   # r4 = length of this command
#
        mov     r4,g1                   # command length
        call    RB_FailBackEventHandler
#
        mov     r13,g0                  # restore g0
        mov     r14,g3                  # restore g3
        mov     r15,g14                 # restore g14
#
        mov     ecok,g1                 # g1 = completion status
        ldconst mDGrsiz,g2              # g2 = response pkt size
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: d$DG_ProcessHsInOp
#
#  PURPOSE:
#       This routine processes a HotSpare non-op  request.
#
#  INPUT:
#       g0 = MRP parameter block
#       g1 = packet length
#
#  OUTPUT:
#       g1 = return status code
#       g2 = return pkt size
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
d$DG_ProcessHsInOp:
        mov     g0,r13                  # save g0
        mov     g3,r14                  # save g3
        mov     g14,r15                 # save g14
#
        addo    g0,g1,r5                # ending address of packet information
#
# --- Precess the imbedded records
#
        ld      ccsm_e_len-CCSM_E_OFFSET(g0),r4   # r4 = length of this command
#
        mov     r4,g1                   # command length
        call    RB_NonOpEventHandler
#
        mov     r13,g0                  # restore g0
        mov     r14,g3                  # restore g3
        mov     r15,g14                 # restore g14
#
        mov     ecok,g1                 # g1 = completion status
        ldconst mDGrsiz,g2              # g2 = response pkt size
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: d$DG_NVRAMrefresh
#
#  PURPOSE:
#       This routine processes a request for an NVRAM refresh.
#
#  INPUT:
#       g0 = MRP parameter block
#
#  OUTPUT:
#       g1 = return status code
#       g2 = return pkt size
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
d$DG_NVRAMrefresh:
        mov     g0,r12                  # save g0
        mov     g3,r13                  # save g3
        mov     g4,r14                  # save g4
        mov     g6,r10                  # save g6
        mov     g14,r15                 # save g14
#
        ldconst fidbenvram,r4           # FID
        mov     r4,g0                   # place in correct register
        ld      O_temp_nvram,g1         # Pointer to buffer
        ldconst 1,g2                    # Length in blocks (just the header)
        ldconst 1,g3                    # Confirmation
        ldconst 1,g4                    # Start at block one
        ldconst 0,g6                    # Set pid bitmap to zero
        call    FS$MultiRead            # Read
        cmpobne 0,g0,.DGNVRrefsh_error  # Jif the read failed
#
# --- Now do the read for the proper amount of data.
#
        mov     r4,g0                   # restore FID
        ld      O_temp_nvram,g1         # Pointer to buffer
        ld      12(g1),g2               # Length in bytes
        ldconst SECSIZE,r3              # Bump up a sector
        addo    r3,g2,g2
        divo    r3,g2,g2                # Block count (rounded)
        ldconst 1,g3                    # Confirmation
        ldconst 1,g4                    # Start at block one
        call    FS$MultiRead            # Read
        cmpobe  0,g0,.DGNVRrefsh_rcmplt # Jif the read worked
#
# --- Error out of reads for file system operation.
#
.DGNVRrefsh_error:
        ldconst deioerr,r11             # Error code
        b       .DGNVRrefsh_return      # Exit
#
# --- read is complete. Check the checksum from the buffer.
#
.DGNVRrefsh_rcmplt:
        ldconst debadnvrec,r11          # Get possible error code set up
        ld      O_temp_nvram,g0         # Address of data buffer
        PushRegs(r8)                    # Save register contents
        call    NV_P2ChkSumChk          # Check P2 integrity
        PopRegs(r8)                     # Restore registers (except g0)
        cmpobne TRUE,g0,.DGNVRrefsh_return # JIf P2 error
#
# --- refresh NVRAM
#
        ld      O_temp_nvram,g0         # Address of data buffer
        PushRegs(r11)                   # Save regs
        call    NV_RefreshNvram         # refresh configuration from NVRAM
        PopRegsVoid(r11)                # Restore regs
        mov     ecok,r11                # r11 = completion code
#
.DGNVRrefsh_return:
        mov     r12,g0                  # restore g0
        mov     r13,g3                  # restore g3
        mov     r14,g4                  # restore g4
        mov     r10,g6                  # restore g6
        mov     r15,g14                 # restore g14
        mov     r11,g1                  # g1 = completion status
        ldconst mDGrsiz,g2              # g2 = response pkt size
        ret
#
#**********************************************************************
#
#  NAME: d$DG_ProcessCCBG
#
#  PURPOSE:
#       This routine processes a Process Control Packet (PCP).
#
#  INPUT:
#       g0 = MRP parameter block
#       g1 = packet length
#
#  OUTPUT:
#       g1 = return status code
#       g2 = return pkt size
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
d$DG_ProcessCCBG:
        mov     g0,r13                  # save g0
        mov     g3,r14                  # save g3
        mov     g14,r15                 # save g14
#
        mov     g0,r12                  # r12 = working copy of g0
        addo    g0,g1,r5                # ending address of packet information
#
# --- Precess the imbedded records
#
DGProcessCCBG_100:
!       ld      ccsm_e_len-CCSM_E_OFFSET(r12),r4   # r4 = length of this command
#
        mov     r12,g0                  # g0 = start of command
        mov     r4,g1                   # command length
.if CM_IM_DEBUG
c fprintf(stderr,"%s%s:%u <CM_IM>definebe-d$DG_ProcessCCBG-- calling CCSM$ccbg\n", FEBEMESSAGE, __FILE__, __LINE__);
.endif  # CM_IM_DEBUG
        call    CCSM$ccbg
#
# --- bump past the current record and determine if there are any more records
#     to process.
#
        addo    r4,r12,r12              # point to next command
        cmpobl  r12,r5,DGProcessCCBG_100 # continue
#
# --- all done
#
        mov     r13,g0                  # restore g0
        mov     r14,g3                  # restore g3
        mov     r15,g14                 # restore g14
#
        mov     ecok,g1                 # g1 = completion status
        ldconst mDGrsiz,g2              # g2 = response pkt size
        ret                             # return to caller
#
#**********************************************************************
#
#  NAME: d$putfsys
#
#  PURPOSE:
#       Take a Fsys report and update the PDD internally based upon
#       the values in the Fsys report.
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = return status code
#       g2 = return pkt size
#
#**********************************************************************
#
d$putfsys:
        ld      mr_ptr(g0),r15          # Get parm block pointer
        lda     mpf_fsys(r15),g0        # Fsys report address
#
        ldconst TRUE,g1                 # Assume master
        ldos    K_ii+ii_status,r4       # Get initialization status
        bbs     iimaster,r4,.pfs10      # Jif master
        ldconst FALSE,g1                # Slave
#
.pfs10:
        PushRegs(r3)                    # Save register contents
        call    NV_ProcessFSys          # Process it
        PopRegsVoid(r3)                 # Restore registers
#
# --- Exit
#
        ldconst deok,g1
        ldconst mpfrsiz,g2
        ret
#
#**********************************************************************
#
#  NAME: DEF_ReportEvent or D$reportEvent
#
#  PURPOSE:
#       Put and entry into a message and send to the CCB for broadcast.
#
#  DESCRIPTION:
#       If the size of the packet is greater than the max packet, pass
#       the PCI address and length instead of the data directly.
#
#  INPUT:
#       g0 = record address
#       g1 = extension size
#       g2 = event type
#       g3 = broadcast type
#       g4 = serial number if specific controller requested
#
#  OUTPUT: none
#
#**********************************************************************
#
# void DEF_ReportEvent(void* nvram, UINT32 extSize, UINT32 event,
#                     UINT32 broadcastType, UINT32 serialNumber);
        .globl  DEF_ReportEvent         # C access
DEF_ReportEvent:
D$reportEvent:
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       r10 = (UINT32)&TmpStackMessage[0];
c       if (ebimaxsiz < g1) {           # if too long a message.
# --- Too big for the packet. Just calculate the PCI address, put it in
# --- the packet and place the length in the packet.
# Length is ebilen (20) + 4 =24 bytes, which fits inside MRP.
c           r11 = ebilen + 4;
c           *(UINT32 *)(r10+ebi_data) = g0;
c       } else {
# The following is less than ebimaxsiz (52 bytes) and ebilen is 20, thus < 72 bytes.
c           r11 = g1 + ebilen;
c           memcpy((char *)(r10+ebi_data), (char *)g0, g1);
c       }
        ldconst mleipcbroadcast,r4      # Event code
        st      r4,mle_event(r10)       # Store as word to clear other bytes
        stos    g2,ebi_subevent(r10)    # Subevent
        stos    g3,ebi_bcasttype(r10)   # Broadcast type
        st      g4,ebi_serial(r10)      # No serial number
        st      g1,ebi_datasize(r10)    # Data size
# r10 = address, r11 = length.
c       MSC_LogMessageStack(&TmpStackMessage[0], r11);
#
        ret
#
#**********************************************************************
#
#  NAME: d$namedevice
#
#  PURPOSE:
#       Sets the name of a specified device in NVRAM.
#
#  DESCRIPTION:
#       This function will take a device (vdisk, server, etc) and set
#       the name of the device in the structure and in the NVRAM.
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = return status code
#       g2 = return pkt size
#
#**********************************************************************
#
d$namedevice:
        ld      mr_ptr(g0),r15          # Get parm block pointer
        ldos    mnd_id(r15),r14         # Get ID
        ldos    mnd_option(r15),r13     # Get option
        ld      mr_rptr(g0),r11         # Return block address
#
# --- Check the options field for validity and then parse the device type.
#
        ldconst deinvopt,g1             # Prep error code
        cmpobne mndserver,r13,.nd10     # Jif not a server update
#
# --- Update server name
#
        ldconst deinvsid,g1             # Prep error code
        ldconst MAXSERVERS,r3           # Check for greater than max
        cmpobge r14,r3,.nd100           # Jif out of range
        ld      S_sddindx[r14*4],r12    # Get SDD pointer
        cmpobe  0,r12,.nd100            # Jif NULL pointer (no server defined)
#
c       memcpy(((SDD*)r12)->name,((MRNAMEDEVICE_REQ*)r15)->name, 16);
#
# --- Update the FEP
#
        mov     r14,g0                  # SID
        ldconst FALSE,g1                # Not a deletion
        call    D_updrmtserver          # Update it (no need to signal)
#
        b       .nd90                   # Save NVRAM and quit
#
.nd10:
        cmpobne mndvdisk,r13,.nd50      # Jif not a vdisk name change
#
# --- Update vdisk name
#
        ldconst deinvvid,g1             # Prep error code
        ldconst MAXVIRTUALS,r3
        cmpobge r14,r3,.nd100           # Jif out of range
        ld      V_vddindx[r14*4],g1     # Get VDD pointer
        cmpobe  0,g1,.nd100             # Jif NULL pointer (no vdisk defined)
#
        ldq     mnd_name(r15),r4        # Get name
        stq     r4,vd_name(g1)          # Save it
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
#
        call    DLM$VDnamechg           # Inform DLM
        b       .nd90                   # Save NVRAM and quit
#
.nd50:
        cmpobne mndvcg,r13,.nd70        # Not a VCG, check next option
#
# --- Update VCG name
#
        ldq     mnd_name(r15),g0        # Get name
        ld      K_ficb,r3               # Get FICB address
        stq     g0,fi_vcgname(r3)       # Save it
#
        call    DLM$MAGnamechg          # Tell DLM to change name (g0 has name)
#
        call    D$updrmtsysinfo         # Update the FEP FICB
        b       .nd90                   # Save NVRAM and quit
#
.nd70:
        cmpobne mndretvcg,r13,.nd100    # Not get VCG name, exit with error
#
# --- Fetch VCG name
#
        ld      K_ficb,r3               # Get FICB address
        ldq     fi_vcgname(r3),r4       # Get name
!       stq     r4,mnd_retname(r11)     # Save it
        b       .nd95                   # Exit, no NVRAM update required
#
.nd90:
        call    D$p2updateconfig        # Save NVRAM
#
.nd95:
        ldconst deok,g1
#
# --- Exit
#
.nd100:
        ldconst mndrsiz,g2
        ret
#
#**********************************************************************
#
#  NAME: D$getsos
#
#  PURPOSE:
#       Build a pseudo-SOS table and send it to the caller.
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = return status code
#       g2 = return pkt size
#
#**********************************************************************
#
D$getsos:
        ld      mr_rptr(g0),r15         # r15 = return data address
        ld      mr_ralloclen(g0),r14    # Return data maximum
#
        lda     mes_data(r15),r15       # SOS table address
        ldconst mrrsiz,g2               # Prep return size
#
# --- Check parameters for valid values.
#
        ld      mr_ptr(g0),r3           # Get parm block address
        ldos    mes_pid(r3),r13         # r10 = specified PID
#
        ldconst MAXDRIVES,r5            # r5 = max. # drives
        ldconst deinvpid,g1             # g1 = possible error code
        cmpoble r5,r13,.dgs100          # Jif invalid VDisk #
#
        ld      P_pddindx[r13*4],r10    # PDD address
        cmpobe  0,r10,.dgs100           # Jif no PDD defined
#
# --- Check the length. We are going to build a table from the DAML in
# --- the PDD, so update the DAML and then use the count within the DAML
# --- to check the length. Make sure that if the data space is not sufficient
# --- that the count is placed in the header to allow the caller to figure
# --- out how much data to request.
#
        mov     r13,g0                  # PID
c       DA_DAMBuild(r13);               # Update the DAML if required
        ld      pd_daml(r10),r12        # Get the DAML
#
        ldconst deinopdev,g1            # Prep for inoperable
        cmpobe  0,r12,.dgs100           # Exit if no DAML produced
#
        ldos    da_count(r12),r9        # Get the count
        mulo    damlxsiz,r9,r4          # Multiply by extension size
        lda     mes_sos(r4),r4          # Add the total header area
#
!       stos    r9,so_count(r15)        # Set count in return data
        ldconst detoomuchdata,g1        # Set error code
        cmpobl  r14,r4,.dgs100          # Jif not enough data space
        mov     r4,r14                  # Truncate in case sent more than enough
#
# --- Now build the return data.This is done by iterating through the
# --- DAML structure and converting AU to blocks.
#
        ldconst deok,g1                 # No error
        ldconst DSKSALLOC,r8            # Multiplication factor
#
!       stos    r13,so_pid(r15)         # Set PID
        ldob    da_flags(r12),r3        # Get flags
!       stos    r3,so_flags(r15)        # Set them
#
        ld      da_largest(r12),r3      # Get largest size available
        mulo    r3,r8,r3                # Get into blocks
!       st      r3,so_remain(r15)       # Set into remaining
#
        ld      da_total(r12),r3        # Get total space available
        mulo    r3,r8,r3                # Get into blocks
!       st      r3,so_total(r15)        # Set into total
#
        ldconst 0,r11                   # Index into DAML
        lda     damlsiz(r12),r12        # Move into the table area
        lda     mes_sos-mes_data(r15),r15# Move return data into table area
        ldconst 0,r13                   # Add in a check for the gaps being off
#
.dgs10:
        ldq     da_AUgap(r12),r4        # Get gap, start, len, RID
        mulo    r4,r8,r4                # gap in blocks
        mulo    r5,r8,r5                # starting LBA
        mulo    r6,r8,r6                # length in blocks
!       stq     r4,(r15)                # Save it
#
# --- Check the SDA to make sure it is correct
#
        cmpobe  r13,r5,.dgs30           # Jif correct
#
        ldconst debaddam,g1             # Bad DAM entry
#
.dgs30:
        addo    r4,r5,r13               # Get next expected SDA
        addo    r6,r13,r13              # SDA + SLEN + GAP = next expected SDA
#
        addo    16,r15,r15
        lda     16(r12),r12
#
        addo    1,r11,r11               # Bump index
        cmpobne r11,r9,.dgs10           # Jif more to do
#
        mov     r14,g2                  # Since the alloc was sufficient, use it
#
# --- Exit
#
.dgs100:
        ret
#
#******************************************************************************
#
#  NAME: D$TraceEvent
#
#  PURPOSE:
#       To trace and timestamp define events.
#
#  DESCRIPTION:
#       This function will flight record trace events in a circular queue.
#
#  INPUT:
#       Load g0 with the event id, g1 with the associated data you want to
#       save, then call this function. The parms will be saved in a circular
#       queue.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
        .set    trMRPStart,   0x80000000
        .set    trMRPStop,    0x800000FF
        .set    trMRPDefComp, 0xF00000FF
#
        .data
ptrdefTraceQue:                         # Allow use of this structure in .gdbinit.
        .word   defTraceQue             # Allow use of this structure in .gdbinit.
        START_SH_DATA_SECTION
defTraceQue:
        .word   defTraceQue+16
        .word   defTraceQue+16
        .word   defTraceQueEnd-16
        .word   1                       # Enable it
        .space  16*1024,0               # Allocate 1024 entries
defTraceQueEnd:
        END_SH_DATA_SECTION
#
        .text
#
D$TraceEvent:
        lda     defTraceQue,r3          # load the event queue structure ptr
        ldq     (r3),r4                 # r4=evBaseP, r5=evNextP,
                                        # r6=evEndP, r7=runFlag
        cmpibe  0,r7,.te20              # if runFlag clear, just exit
#
        ld      K_ii+ii_time,r8         # read gross timer
c       static struct itimerval dte_current_time;
c       if (getitimer (ITIMER_REAL, &dte_current_time) == 0) {
c               r9 = dte_current_time.it_interval.tv_usec - dte_current_time.it_value.tv_usec;
c       } else {
c               r9 = 0;
c       }
        stl     g0,(r5)                 # store event data in the queue
        stl     r8,8(r5)
#
        addo    r5,16,r5                # bump evNextP (in r5)
        cmpi    r5,r6                   # see if we are at the end of the queue
        st      r5,4(r3)                # save new evNextP out
        bne     .te20                   # if not at the end of queue, exit
        st      r4,4(r3)                # else store evBaseP to evNextP
#
.te20:
        ret
#
#**********************************************************************
#
#  NAME: d$get_dlink
#
#  PURPOSE:
#       Gets DLink Information for the specified VLink.
#
#  DESCRIPTION:
#       This routine will determine if the specified VID is a VLink
#       and if so will return the DLink Information for the specified
#       VLink to the requestor.
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = return status code
#       g2 = return pkt size
#
#**********************************************************************
#
d$get_dlink:
        ld      mr_ptr(g0),r15          # r15 = parm block pointer
        ld      mr_rptr(g0),g2          # g2 = response block address
        ldos    mni_did(r15),r4         # r4 = RID for DLink information
        ldconst MAXRAIDS,r5             # r5 = max. # RAIDs supported
        ldconst deinvrid,g1             # g1 = possible error code
        cmpoble r5,r4,.getdlink_1000    # Jif invalid RID specified
!       stos    r4,mni_id(g2)           # save ID in response buffer
        ld      R_rddindx[r4*4],r10     # r10 = RDD address
        cmpobe  0,r10,.getdlink_1000    # Jif specified RAID not defined
        ldob    rd_type(r10),r4         # r4 = raid type code
        cmpobne rdlinkdev,r4,.getdlink_1000 # Jif not a VLink type device
        ld      rd_psd(r10),r6          # r6 = PSD address
        ldos    ps_pid(r6),r7           # r7 = ordinal of LDD
        ld      DLM_lddindx[r7*4],r8    # r8 = LDD address
        cmpobe  0,r8,.getdlink_1000     # Jif LDD not defined
        mov     r8,g1                   # g1 = LDD address to get DLink information for
c       define_DIupdate(g0,g1,g2);
#
        ldconst deok,g1
#
# --- Exit
#
.getdlink_1000:
        ldconst mnirsiz,g2
        ret
#
#**********************************************************************
#
#  NAME: d$get_dlock
#
#  PURPOSE:
#       Gets DLock Information for the specified VDisk.
#
#  DESCRIPTION:
#       This routine will determine if the specified VID is a VDisk
#       that has a VLink associated with it and if so will return
#       the DLock Information for it to the requestor.
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = return status code
#       g2 = return pkt size
#
#**********************************************************************
#
d$get_dlock:
        ld      mr_ptr(g0),r15          # r15 = parm block pointer
        ld      mr_rptr(g0),r14         # r14 = return block address
        ldos    mki_vid(r15),r5         # r5 = specified VDisk #
#
        ldconst MAXVIRTUALS,r10         # r10 = max. # virtuals
        ldconst deinvvirtid,g1          # g1 = possible error code
        cmpoble r10,r5,.getdlock_1000   # Jif invalid VDisk #
#
        ld      V_vddindx[r5*4],r9      # r9 = VDD address
        ldconst deinvvid,g1             # g1 = possible error code
        cmpobe  0,r9,.getdlock_1000     # Jif no VDD defined
#
        stos    r5,mki_id(r14)          # save VID in response buffer
        ldconst denonxdev,g1            # g1 = possible error code
        ld      vd_vlinks(r9),r10       # r10 = assoc. VLAR with VDD
        cmpobe  0,r10,.getdlock_1000    # Jif no VLAR assoc. with VDD
        ldob    vlar_srccl(r10),r4      # r4 = lock cluster #
        ldob    vlar_srcvd(r10),r5      # r5 = lock VDisk #
        ldob    vlar_attr(r10),r6       # r6 = attributes
        bbc     vl_vd_format,r6,.getdlock_200 # Jif MAG format
        mov     r5,r6                   # change format to Vblock/VDisk
        extract 5,3,r6
        shlo    3,r4,r4
        or      r6,r4,r4
        and     0x1f,r5,r5
.getdlock_200:
        stob    r4,mki_cl(r14)          # save cluster/Vblock #
        st      r5,mki_vd(r14)          # save VDisk # and clear reserved
                                        #  bytes
        ldl     vlar_name(r10),r4       # r4-r5 = VDisk name
        stl     r4,mki_vname(r14)       # save VDisk name
        ld      vlar_srcsn(r10),r4      # r4 = lock serial #
        st      r4,mki_sn(r14)          # save lock serial #
        movl    0,r8                    # r8-r9 = node name
        ld      dlm_mlmthd,r6           # r6 = first MLMT on list
.getdlock_300:
        cmpobe  0,r6,.getdlock_400      # Jif no more MLMTs to check
        ld      mlmt_sn(r6),r7          # r7 = MLMT serial #
        cmpobe  r4,r7,.getdlock_350     # Jif matching MLMT found
        ld      mlmt_link(r6),r6        # r6 = next MLMT on list
        b       .getdlock_300           # and process next MLMT on list
#
.getdlock_350:
        ld      mlmt_dtmthd(r6),r7      # r7 = first DTMT assoc. with MLMT
        cmpobe  0,r7,.getdlock_400      # Jif no DTMTs assoc. with MLMT
        ldl     dml_pname(r7),r8        # r8-r9 = node name
.getdlock_400:
        stl     r8,mki_nname(r14)       # save node name
        ldconst deok,g1
#
# --- Exit
#
.getdlock_1000:
        ldconst mkirsiz,g2
        ret
#
#**********************************************************************
#
#  NAME: d$copydata
#
#  PURPOSE:
#       To provide a means of transferring information about all active
#       copies to the CCB/ICON.
#
#  DESCRIPTION:
#       Determine if the provided response buffer is large enough to
#       contain the required amount of data. If not, return an error.
#       Otherwise, copy the COR structures on the COR ACTIVE QUEUE into
#       the response buffer.
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = status
#       g2 = return pkt size
#
#  REGS DESTROYED:
#       Reg. g1, g2 destroyed.
#
#**********************************************************************
#
d$copydata:
        mov     g0,r12                  # save g0
        mov     g3,r13                  # save g3
        mov     g4,r14                  # save g4
        mov     g5,r15                  # save g5
#
        ld      mr_ptr(g0),r8           # r8  = request pointer
        ld      mr_rptr(g0),r9          # r9 = response pointer
                                        #   buffer
        ld      CM_cor_act_que,g3       # g3 = top COR on active queue
#
# --- determine if the provided buffer is large enough.
#
        ldconst mydr_common,g2          # g2 = default size of header
        ldos    cm_cor_act_cnt,g5       # r5 = current active cor count
        ldconst corsize,r3              # r3 = size of the cor structure
        ldob    myd_format(r8),r4       # r4 = requested format
        cmpobe  MRDRCORRSP,r4,.copydata_010 # Jif COR format
#
        ldconst m1drsize,r3             # r3 = size of the detailed copy structure
        cmpobe  MRDTLCPYRSP,r4,.copydata_010 # Jifdetailed format
#
        ldconst 16,r3                   # size of trace entry
.copydata_010:
        mulo    r3,g5,r5                # r5 = total size of required buffer
        lda     mydr_common(r5),r5      # add in response header
#
        ldconst detoomuchdata,g1        # Prep possible error code
        ld      mr_ralloclen(g0),r7     # r7 = provided buffer length
        cmpobg  r5,r7,.copydata_100     # Jif not large enough
#
# --- Ok, we seem to have enough space, determine if there are any
#     structures to copy.
#
        ldconst deok,g1                 # prep good status to requestor
        lda     mydr_common(r9),g4      # set pointer to correct location in
        cmpobe  0,g3,.copydata_100      # Jif nothing on queue
#
        mov     r7,g5                   # size of response buffer
#
# --- there's crap to copy, so get'r done....
#
        cmpobne MRDRCORRSP,r4,.copydata_020 # Jif not COR format
        call    d$copydata_fmt0         # process format
        b       .copydata_100           # br
#
.copydata_020:
        cmpobne MRDTLCPYRSP,r4,.copydata_030 # Jif not detailed format
        call    d$copydata_fmt1         # process format
        b       .copydata_100           # br
#
.copydata_030:
        cmpobne MRDRTRACES,r4,.copydata_040 # Jif not trace format
        call    d$copydata_fmt2         # process trace format
        b       .copydata_100           # br
#
.copydata_040:
        call    d$copydata_fmt3         # process IO map format
#
# --- and EXIT......
#
.copydata_100:
        ldconst 256,r5                  # set limit
        cmpobg  r5,r3,.copydata_110     # Jif under limit
        mov     0,r3                    # set size to 0
#
.copydata_110:
!       stos    g5,mydr_strctcnt(r9)    # save structure count
!       stob    r4,mydr_format(r9)      # save format
!       stob    r3,mydr_strctsiz(r9)    # set structure size
                                        # g1 = response status
                                        # g2 = response length
#
        mov     r12,g0                  # restore g0
        mov     r13,g3                  # restore g3
        mov     r14,g4                  # restore g4
        mov     r15,g5                  # restore g5
        ret
#
#**********************************************************************
#
#  NAME: d$copydata_fmt0
#
#  PURPOSE:
#       To provide a means of transferring information about all active
#       copies to the CCB/ICON.
#
#  DESCRIPTION:
#       Determine if the provided response buffer is large enough to
#       contain the required amount of data. If not, return an error.
#       Otherwise, copy the COR structures on the COR ACTIVE QUEUE into
#       the response buffer.
#
#  INPUT:
#       g0 = MRP
#       g3 = Head of COR queue
#       g4 = response buffer pointer (offset by header)
#       g5 = size of response buffer - header
#
#  OUTPUT:
#       g1 = status
#       g2 = return pkt size
#       g5 = number of structure copied
#
#  REGS DESTROYED:
#       Reg. g1, g2 destroyed.
#
#**********************************************************************
#
d$copydata_fmt0:
        ldconst mydr_common,g2          # g2 = amount of data copied
        mov     0,r3                    # reset structure count
        ldconst detoomuchdata,g1        # Prep possible error code
#
# --- copy the cor data into the response buffer
#
.copydata_fmt0_100:
        addo    1,r3,r3                 # increment cor count
c       g2 = g2 + corsize;              # increment amount copied
        cmpobg  g2,g5,.copydata_fmt0_1000    # Jif copying to much data
                                        #   NOTE: Somehow the active COR count and the
                                        #         number of CORs on the active queue are
                                        #         out of sync.
c       memcpy((void*)g4, (void*)g3, corsize);
#
c       g4 = g4 + corsize;              # bump buffer pointer by cor size
        ld      cor_link(g3),g3         # link to next possible cor
        cmpobne 0,g3,.copydata_fmt0_100 # continue with next structure
#
# --- all done
#
        ldconst deok,g1                 # return good status to requestor
#
.copydata_fmt0_1000:
        mov     r3,g5                   # set structure count
        ret
#
#**********************************************************************
#
#  NAME: d$copydata_fmt1
#
#  PURPOSE:
#       To provide a means of transferring information about all active
#       copies to the CCB/ICON.
#
#  DESCRIPTION:
#       Determine if the provided response buffer is large enough to
#       contain the required amount of data. If not, return an error.
#
#  INPUT:
#       g0 = MRP
#       g3 = Head of COR queue
#       g4 = response buffer pointer (offset by header)
#       g5 = size of response buffer - header
#
#  OUTPUT:
#       g1 = status
#       g2 = return pkt size
#       g5 = number of structure copied
#
#  REGS DESTROYED:
#       Reg. g1, g2 destroyed.
#
#**********************************************************************
#
d$copydata_fmt1:
        ldconst mydr_common,g2          # g2 = amount of data copied
        mov     0,r3                    # reset cor count
        ldconst detoomuchdata,g1        # Prep possible error code
#
# --- copy the cor data into the response buffer
#
.copydata_fmt1_100:
        addo    1,r3,r3                 # increment cor count
        ldconst m1drsize,r4             # r4 = data structure size
        addo    r4,g2,g2                # increment amount copied
        cmpobg  g2,g5,.copydata_fmt1_1000 # Jif copying to much data
                                        #   NOTE: Somehow the active COR count and the
                                        #         number of CORs on the active queue are
                                        #         out of sync.
#
# --- copy cor and cm information
#
        ld      cor_rcsn(g3),r8         # copy serial number
        ld      cor_rid(g3),r9          # registration id
        ldos    cor_rcscl(g3),r10       # source vid
        ldos    cor_rcdcl(g3),r11       # destination vid
!       st      r8,m1dr_rcsn(g4)        # save the information
!       st      r9,m1dr_rid(g4)
!       stos    r10,m1dr_rcscl(g4)
!       stos    r11,m1dr_rcdcl(g4)
#
        ldob    cor_cstate(g3),r8       # copy state
        ldob    cor_crstate(g3),r9      # copy registration state
        ldob    cor_mstate(g3),r10      # region/segment map state
        ld      cor_cm(g3),r11          # get cm address
        cmpobe  0,r11,.copydata_fmt1_110 # Jif no cm task
#
# --- copy operation type
        ldob    cm_type(r11),r12        # get copy type
!       stob    r12,m1dr_ctype(g4)      # save copy type
        ldob    cm_cstate(r11),r11       # cm copy state
.copydata_fmt1_110:
!       stob    r8,m1dr_ccstate(g4)     # save the information
!       stob    r9,m1dr_crstate(g4)
!       stob    r10,m1dr_cmstate(g4)
!       stob    r11,m1dr_mcstate(g4)
#
        ld      cor_powner(g3),r8       # get primary copy owner
        and     0x07,r8,r8              # isolate only controller
!       stob    r8,m1dr_owner(g4)       # save owner
#
# --- copy scd information
#
        ld      cor_scd(g3),r4          # scd address
        mov     0,r8                    # default scd p2 handler ordinal
        mov     0,r9                    # default scd type
        cmpobe  0,r4,.copydata_fmt1_125 # Jif no scd
#
        ld      scd_p2hand(r4),r11      # scd p2 handler
        ldob    scd_type(r4),r9         # scd type
        lda     p6UpdhndTbl,r10         # r10 = ordinal table
        cmpobe  0,r11,.copydata_fmt1_125 # Jif p2 handler is null
#
.copydata_fmt1_120:
        addo    1,r8,r8                 # increment ordinal
        ld      (r10)[r8*4],r5          # get entry from table
        cmpobe  r11,r5,.copydata_fmt1_125 # match found
        cmpobne 0,r5,.copydata_fmt1_120 # continue until null
        mov     0,r8                    # reset scd p2 handler ordinal
#
.copydata_fmt1_125:
!       stob    r8,m1dr_sp2hdlr(g4)     # save scd p2 handler ordinal
!       stob    r9,m1dr_stype(g4)       # save scd type
#
# --- copy dcd information
#
        ld      cor_dcd(g3),r4          # dcd address
        mov     0,r8                    # default dcd p2 handler ordinal
        mov     0,r9                    # default dcd type
        cmpobe  0,r4,.copydata_fmt1_135 # Jif no dcd
#
        ld      dcd_p2hand(r4),r11      # dcd p2 handler
        ldob    dcd_type(r4),r9         # dcd type
        lda     p6UpdhndTbl,r10         # r10 = ordinal table
        cmpobe  0,r11,.copydata_fmt1_135 # Jif p2 handler is null
#
.copydata_fmt1_130:
        addo    1,r8,r8                 # increment ordinal
        ld      (r10)[r8*4],r5          # get entry from table
        cmpobe  r11,r5,.copydata_fmt1_135 # match found
        cmpobne 0,r5,.copydata_fmt1_130 # continue until null
        mov     0,r8                    # reset dcd p2 handler ordinal
#
.copydata_fmt1_135:
!       stob    r8,m1dr_dp2hdlr(g4)     # save dcd p2 handler ordinal
!       stob    r9,m1dr_dtype(g4)       # save dcd type
#
# --- copy vdd information
#
        ld      cor_destvdd(g3),r4      # vdd address
        mov     0,r8                    # default vdd mirror status
        mov     0,r9                    # default vdd attributes
        cmpobe  0,r4,.copydata_fmt1_140 # jif vdd pointer is null
#
        ldob    vd_mirror(r4),r8        # get mirror status
        ldob    vd_attr(r4),r9          # get attributes
#
.copydata_fmt1_140:
!       stob    r8,m1dr_vmirror(g4)     # save vdd mirror status
!       stob    r9,m1dr_vattr(g4)       # save vdd attributes
#
# --- determine if there is another COR
#
        ldconst m1drsize,r4             # r4 = detail copy structure size
        addo    r4,g4,g4                # bump buffer pointer
#
        ld      cor_link(g3),g3         # link to next possible cor
        cmpobne 0,g3,.copydata_fmt1_100 # continue with next structure
#
# --- all done
#
        ldconst deok,g1                 # return good status to requestor
#
.copydata_fmt1_1000:
        mov     r3,g5                   # set structure count
        ret
#
#**********************************************************************
#
#  NAME: d$copydata_fmt2
#
#  PURPOSE:
#       To provide a means of transferring CCSM trace buffer information
#       to the CCB/ICON.
#
#  DESCRIPTION:
#       Determine if the provided response buffer is large enough to
#       contain the required amount of data. If not, return an error.
#       Otherwise, copy the trace table in to the buffer
#
#  INPUT:
#       g0 = MRP
#       g3 = Head of COR queue
#       g4 = response buffer pointer (offset by header)
#       g5 = size of response buffer - header
#
#  OUTPUT:
#       g1 = status
#       g2 = return pkt size
#       g5 = number of structure copied
#
#  REGS DESTROYED:
#       Reg. g1, g2, g4 destroyed.
#
#**********************************************************************
#
d$copydata_fmt2:
        ldconst mydr_common,g2          # g2 = amount of data copied
        mov     0,r3                    # reset structure count
        ldconst detoomuchdata,g1        # Prep possible error code
        ldconst 16,r4                   # r4 = trace entry size
#
# --- copy last half of trace table
#
        ld      ccsm_tr_cur,r6          # r6 = next on pointer
        ld      ccsm_tr_tail,r7         # r7 = tail pointer
        cmpobge r6,r7,.copydata_fmt2_200 # Jif nothing in last half
#
.copydata_fmt2_100:
        addo    1,r3,r3                 # increment structure count
        addo    r4,g2,g2                # increment amount copied
        cmpobg  g2,g5,.copydata_fmt2_1000 # Jif dest buffer overrun
#
        ldq     (r6),r8                 # copy trace entry
!       stq     r8,(g4)
#
        addo    r4,g4,g4                # bump dest buffer pointer
        addo    r4,r6,r6                # bump source pointer
        cmpobl  r6,r7,.copydata_fmt2_100 # continue with next structure
#
# --- copy first half of trace table
#
.copydata_fmt2_200:
        ld      ccsm_tr_head,r6          # r6 = head on pointer
        ld      ccsm_tr_cur,r7           # r7 = current pointer
        cmpobge r6,r7,.copydata_fmt2_400 # Jif nothing in first half
#
.copydata_fmt2_210:
        addo    1,r3,r3                 # increment structure count
        addo    r4,g2,g2                # increment amount copied
        cmpobg  g2,g5,.copydata_fmt2_1000 # Jif dest buffer overrun
#
        ldq     (r6),r8                 # copy trace entry
!       stq     r8,(g4)
#
        addo    r4,g4,g4                # bump dest buffer pointer
        addo    r4,r6,r6                # bump source pointer
        cmpobl r6,r7,.copydata_fmt2_210 # continue with next structure
#
# --- all done
#
.copydata_fmt2_400:
        ldconst deok,g1                 # return good status to requestor
#
# --- exit
#
.copydata_fmt2_1000:
        mov     r3,g5                   # set structure count
        ret
#
#**********************************************************************
#
#  NAME: d$copydata_fmt3
#
#  PURPOSE:
#       To provide a means of transferring information about all active
#       copies to the CCB/ICON.
#
#  DESCRIPTION:
#       Determine if the provided response buffer is large enough to
#       contain the required amount of data. If not, return an error.
#       Otherwise, format bit may of destination VDDs that are NOT
#       in a pause state. A "1" indication outstanding IO.
#
#  INPUT:
#       g0 = MRP
#       g3 = Head of COR queue
#       g4 = response buffer pointer (offset by header)
#       g5 = size of response buffer - header
#
#  OUTPUT:
#       g1 = status
#       g2 = return pkt size
#       g5 = number of structure copied
#
#  REGS DESTROYED:
#       Reg. g1, g2 destroyed.
#
#**********************************************************************
#
d$copydata_fmt3:
        ldconst mydr_common,g2          # g2 = amount of data copied
        mov     0,r3                    # reset structure count
        ldconst detoomuchdata,g1        # Prep possible error code
#
# --- copy the cor data into the response buffer
#
        ldconst (MAXVIRTUALS+7)/8,r4    # r4 = cor size
        addo    r4,g2,g2                # increment amount copied
        cmpobg  g2,g5,.copydata_fmt3_1000 # Jif buffer is too small
                                        #   NOTE: The amount requested is less
                                        #         than MAX virtuals / 8
#
# --- make damm sure the buffer is cleared out
#
        shro    2,r4,r4                 # set up for words
#
.copydata_fmt3_100:
        subo    1,r4,r4
!       st      r3,(g4)[r4*4]
        cmpobne 0,r4,.copydata_fmt3_100
#
# --- Run through the cor chain and set the associated dest vdd bit
#     for copies that are not paused
#
.copydata_fmt3_110:
        ld      cor_destvdd(g3),r9      # r9 = destination vdd address
        cmpobe  0,r9,.copydata_fmt3_200 # Jif no vdd defined (?????)
#
        ldob    cor_crstate(g3),r8      # r8 = cor cr state
        cmpobne corcrst_usersusp,r8,.copydata_fmt3_150 # set bit
#
        ldob    vd_attr(r9),r8          # get the vdd attributes
        bbs     vdbvdsusp,r8,.copydata_fmt3_200 # Jif copy is suspended
#
.copydata_fmt3_150:
        ldos    vd_vid(r9),r10          # r10 - vid
#
        and     0x7,r10,r11             # set bit number in word
#        subo    r11,7,r11               # change endiance of bits
        shro    3,r10,r10               # find word
#
        ldob    (g4)[r10*1],r8          # set the bit in the buffer
        setbit  r11,r8,r8
        stob    r8,(g4)[r10*1]
#
.copydata_fmt3_200:
        ld      cor_link(g3),g3         # link to next possible cor
        cmpobne 0,g3,.copydata_fmt3_110 # continue with next structure
#
# --- all done
#
        mov     ((MAXVIRTUALS+7)/8)/16,r3   # set structure count to 1
        ldconst deok,g1                 # return good status to requestor
#
.copydata_fmt3_1000:
        mov     r3,g5                   # set structure count
        ret
#
#**********************************************************************
#
#  NAME: d$copyctl
#
#  PURPOSE:
#       To provide a means of controlling resync copy operations.
#
#  DESCRIPTION:
#       If valid, checks if the specified copy operation is active.
#       If not, an error is returned to the user. If it is active,
#       it performs the specified function and returns the appropriate
#       status back to the requestor.
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = status
#       g2 = return pkt size
#
#  REGS DESTROYED:
#       Reg. g1, g2, g4 destroyed.
#
#**********************************************************************
#
d$copyctl:
        mov     g0,r14                  # save g0
        ld      mr_ptr(r14),r15         # r15 - parm block
#
        ld      myc_rid(r15),g0         # g0 = copy registration ID
        ld      myc_csn(r15),g1         # g1 = copy MAG serial #
        call    CM$find_cor_rid         # find specified COR
                                        # g0 = COR address if active
        ldconst denocpymatch,g1         # g1 = error status to requestor
        cmpobe.t 0,g0,.copyctl_1000     # Jif no matching COR found on active
                                        #     queue
#
# --- cor was found. Determine the FC.
#
        ldconst deinvop,g1              # g1 = default error status to return for
        ldob    myc_fc(r15),r4          # r4 = function code
        cmpobe.t 0x01,r4,.copyctl_300   # Jif set name/group function
        cmpobe.t 0x00,r4,.copyctl_400   # Jif delete COR
        cmpobe.t 0x02,r4,.copyctl_350   # Jif swap mirror function
        b       .copyctl_1000           #  unsupported function codes
#
# --- Set copy operation name/group function handler
#
.copyctl_300:
        ld      cor_cm(g0),r9           # r9 = assoc. CM address
        cmpobe.f 0,r9,.copyctl_1000     # Jif remote COR
#
        ldob    myc_gid(r15),r8         # r8 = assigned group ID
        ldq     myc_name(r15),r4        # r4-r7 = copy user defined name
        stob    r8,cor_gid(g0)          # save assigned group ID in COR
        stq     r4,cor_label(g0)        # save copy user defined name in COR
        PushRegs(r3)
        call    DEF_LogCopyLabelEvent   # g0 contains COR
        PopRegsVoid(r3)
#
        call    D$p2updateconfig        # save P2 NVRAM
        call    D$SendRefreshNV         # tell other nodes to refresh NVRAM
#
        b       .copyctl_450     # return good status to requestor
#
# --- Swap Mirror function handler, Don't let destination be async related
#
.copyctl_350:
        ld      cor_destvdd(g0),r8
        cmpobe  0,r8,.copyctl_1000      # jif no dest vdd
        ldos    vd_attr(r8),r3          # Check for async bit
        bbc     vdbasync,r3,.copyctl_355
        ldconst ecinvvid,g1
        b       .copyctl_1000           # Don't allow async destination.
#
# --- Don't allow a RAID5 or linked device to be swapped into an apool
#
.copyctl_355:
        ld      cor_srcvdd(g0),r3
        cmpobe  0,r3,.copyctl_1000      # Jif no src vdd
        ldos    vd_attr(r3),r6          # Check for apool
        bbc     vdbasync,r6,.copyctl_390
        ld      cor_destvdd(g0),r3      # Get destination VDD
        ldob    vd_raidcnt(r3),r4       # Get number of RAIDs
        ld      vd_rdd(r3),r5           # Get the first RAID
.copyctl_360:
        cmpobe  0,r4,.copyctl_390       # Jif all RAIDS checked
        subo    1,r4,r4
        ldob    rd_type(r5),r6          # Get the RAID type
        ld      rd_nvrdd(r5),r5         # Advance to the next RAID
        cmpobe  rdlinkdev,r6,.copyctl_365
        cmpobne rdraid5,r6,.copyctl_360
.copyctl_365:
        ldconst ecinvrid,g1
        b       .copyctl_1000           # Return error
#
.copyctl_390:
        mov     g3,r8                   # save g3
        mov     g0,g3                   # place cor address in correct register
        mov     0,g0                    # g0 = swap RAIDs type code
        call    CCSM$swap_raids         # generate swap RAIDs event to CCSM
        mov     r8,g3                   # restore g3
        b       .copyctl_450            # give good ending status and end
#
# --- Delete COR function handler
#
.copyctl_400:
        mov     g3,r8                   # save g3
        mov     g0,g3                   # place cor address in correct register
        call    CCSM$term_copy          # generate terminate copy event to
        mov     r8,g3                   # restore g3
#
.copyctl_450:
        ldconst deok,g1                 # return good status to requestor

.copyctl_1000:
        mov     r14,g0                  # restore g0
        mov     1,g2                    # Set return packet size
        ret
#
#**********************************************************************
#
#  NAME: d$cfgoption
#
#  PURPOSE:
#       To provide a means of setting and returning configurable options.
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = status
#       g2 = return pkt size
#
#  REGS DESTROYED:
#       Reg. g1, g2, g4 destroyed.
#
#**********************************************************************
#
d$cfgoption:
        mov     g0,r8                   # save g0
        ld      mr_ptr(g0),r15          # r15 - parm block
        ld      mr_rptr(g0),r14         # r14 = response pointer
#
        ld      mcopt_data0(r15),r12    # Get data
        ld      mcopt_mask0(r15),r13    # Get mask
#
# --- enable/disable WHQL
#
        bbc     mcoptenableWHQL,r13,.copt20 # Jif mask not set - do nothing
        ldconst TRUE,r3                 # Assume enable
        bbs     mcoptenableWHQL,r12,.copt10 # Jif if disabled
        ldconst DISABLE,r3              # Disable it
.copt10:
        stob    r3,NV_scsi_whql         # Save it
.copt20:
#
# --- Update NVRAM and the FE -----------------------------------------
#
        call    D$p2updateconfig        # Update NVRAM part II
        call    D$sndconfigopt          # inform FE of possible option changes
#
# --- build up return value
#
        ldconst 0,r4
#
        ldob    NV_scsi_whql,r3
        cmpobe  DISABLE,r3,.cotp110
        setbit  mcoptenableWHQL,r4,r4
#
.cotp110:
!       st      r4,mrcopt_options(r14)  # save options
        mov     r8,g0                   # restore g0
        mov     mcoptrsiz,g2            # Set return packet size
#
        ldconst deok,g1                 # return good status to requestor
        ret
#
#**********************************************************************
#
#  NAME: d$r5recover
#
#  PURPOSE:
#       Calls the RAID 5 function to try and Recover an Inoperable RAID 5
#
#  DESCRIPTION:
#       Calls the correct RAID 5 function, with the proper input parameters,
#       to try and recover a RAID 5 that is now in the Inoperable state.
#
#  INPUT:
#       g0 - MRP
#
#  OUTPUT:
#       g1 - error code
#       g2 - length of return packet
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
d$r5recover:
        mov     g0,r15                  # Save the input parameter
#
# --- Call the function to do the work of clearing the RAID 5 Inop Status
#
        ld      mr_ptr(g0),r12          # Parm block address
        ldos    mr5_rid(r12),g0         # g0 = RAID ID
        call    R$recover_R5inop
        mov     g0,g1                   # Load the return status
        ldconst mr5rsiz,g2              # Set up return size
        ret
#
#**********************************************************************
#
# --- global function declarations ------------------------------------
#
        .globl  D$setvpri               # Set a Vdisk priority
        .globl  D$rstvpri               # Restore Vdisk Priority
        .globl  V_primap
        .globl  rstvpritest
#
# --- global data declarations ----------------------------------------
#
        .data
d_setvpri:
        .word   vdlow                   # Low priority value
        .word   vdmed                   # Medium priority value
        .word   vdhigh                  # High priority value
#
d_priqlst:
        .word   V_exec_qu               # Queue for low pri
        .word   V_exec_mqu              # Queue for med pri
        .word   V_exec_hqu              # Queue for high pri
#
rstvpritest:
        .word  FALSE
#
        .text
#
#**********************************************************************
#
#  NAME: D$setvpri
#
#  PURPOSE:
#       To provide a common means of setting the priorities for a
#       virtual device.
#
#  DESCRIPTION:
#       This function will set the priority for a given vdisk.
#       It is assumed that the caller will check for validity of the cluster
#       and VID. The priority will be validated.
#
#  INPUT:
#       g0 = Cluster number
#       g1 = Virtual ID
#       g2 = Priority
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
    .globl D_SetVPri
# void D_SetVPri (UINT32 n, UINT16 Vid, UINT8 pri)
D_SetVPri:
D$setvpri:
        movt    g0,r4                   # r4=cluster
                                        # r5=VID
                                        # r6=priority (external)
#
# --- Validate priority
#
        ldconst visethigh,r7
        cmpobg  r6,r7,.svp1000          # Jif invalid priority
#
# --- Translate external priority to internal (value used by FW)
#
        lda     d_setvpri,r7            # Get pri table addr
        ld      (r7)[r6*4],r9           # r9= internal priority value
#
# --- Update the queing routine jump table based on priority
#
        ld      d_priqlst[g2*4],r8      # Get queue for this pri
        st      r8,V_primap[r5*4]       # Update the queing routine
                                        # jump table
#
# --- Update VDD with internal priority value
#
        lda     V_vddindx,r10            # Get VDX pointer
        ld      vx_vdd(r10)[r5*4],r12    # r12 = VDD
        cmpobe  0,r12,.svp1000          # Jif undefined
        stob    r9,vd_strategy(r12)     # Update priority in VDD
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
.svp1000:
#
# --- Update NVRAM
#
        call    D$p2updateconfig        # Update NVRAM part II
#
        movt    r4,g0
        ret
#
#**********************************************************************
#
#  NAME: D$rstvpri
#
#  PURPOSE:
#       To provide a common means of restoring the priorities for all
#       virtual devices.
#
#  DESCRIPTION:
#       This function will set the priority for all Vdisks on all
#       clusters to whatever is in the current VDD if it exists.
#       When NVRAM is restored the priority for a Vdisk is extracted
#       and put into the VDD.
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
D$rstvpri:
        movt    0,r4                    # r4=cluster
                                        # r5=VID
                                        # r6=priority (internal)
.rvp01:
#
# --- Get VDD internal priority value
#
        lda     V_vddindx,r10           # Get VDX pointer
        ld      vx_vdd(r10)[r5*4],r12   # r9 = VDD
#
        cmpobe  0,r12,.rvp10            # Jif undefined
        ldob    vd_strategy(r12),r6     # Get priority in VDD
#
# --- Validate priority
#
        ldconst vdhigh,r7
        cmpobg  r6,r7,.rvp1000          # Jif invalid priority
#
# --- Update the queing routine jump table based on priority
#
        ld      d_priqlst[r6*4],r8      # Get queue for this pri
        st      r8,V_primap[r5*4]       # Update the queing routine
                                        # jump table
.rvp10:
        addo    1,r5,r5                 # Increment VID
        ldconst MAXVIRTUALS,r8
        cmpobne r8,r5,.rvp01            # Jif more VIDs
#
.rvp1000:
        ldconst TRUE,r8
        st      r8,rstvpritest
        ret
#
#**********************************************************************
#
#  NAME: DEF_Slink_Delete
#
#  PURPOSE:
#       C Callable routine to do any cleanup needed after an NVRAM update
#       notices that a SNAPSHOT has been deleted (most likely by the other
#       'master' controller)
#
#  INPUT:
#       g0 = vdd
#
#**********************************************************************
#
# C access
# UINT32 DEF_Slink_Delete(UINT32 vid);
        .globl  DEF_Slink_Delete      # C access
DEF_Slink_Delete:
        mov     g0,r14                  # Save g0
        mov     g8,r15                  # Save g8
        cmpobe  0,g0,.sldel1000         # skip out if corrupt
        ld      vd_incssms(g0),r8       # r8 is the ssms pointer
        cmpobe  0,r8,.sldel1000         # Jif no ssms
        ld      ssm_frstoger(r8),r4
        ldconst 0xffff,r5
        cmpobe  0,r4,.sldel200
        ldos    ogr_vid(r4),r5          # Get the vid of the snappool
.sldel200:
        mov     g0,g8                   # vdd
        mov     r8,g0                   # ssms
        call    d$kill_ss               # clear up records
        cmpobe  0xffff,r5,.sldel1000    # Jif no OGER
        mov     g1,r7
        mov     r5,g1
        call    D$update_spool_percent_used
        mov     r7,g1
.sldel1000:
        mov     r14,g0                  # restore g0
        mov     r15,g8                  # restore g8
        ret
#
##############################################################################
        .data
        .globl  DEF_ssms_table          # C access
DEF_ssms_table:
ssms_table:
        .space  MAX_SNAPSHOT_COUNT*4,0  # SSMS table for the first snappool
        .space  MAX_SNAPSHOT_COUNT*4,0  # SSMS table for the second snappool
#
        .text
#**********************************************************************
#
#  NAME: DEF_build_slink_structures
#
#  PURPOSE:
#       Once a SNAPSHOT is created, this routine builds the necessary
#       snapshot structures.
#
#  NOTE:    A Task in "c" calls this, no "g" registers need be saved/restored.
#
#  INPUT:
#       g0/g1 = size of SS in sectors
#       g2 = ss vid
#       g3 = source vid
#
#  OUTPUT:
#       g0 = SSMS
#
#**********************************************************************
#
# C access
# UINT32 DEF_build_slink_structures(UINT64 SS_size, UINT32 SS_vid, UINT32 SRC_vid);
        .globl  DEF_build_slink_structures      # C access
DEF_build_slink_structures:
        movl    g0,r11                  # save g0/g1 in r11/r12 - size of SS in sectors
        mov     g2,r13                  # ss vid
        mov     g3,r15                  # source vid
#
        ldconst FALSE,r7                # clear oger NV update flag to zero.
#
        lda     V_vddindx,r3            # Get VDX pointer (r3)
        ld      vx_vdd(r3)[r15*4],r5    # r5 = VDD of source
        mov     0,r3
        cmpobe  0,r5,.bss1000           # Jif undefined
#
# --- Get the Vid of the appropriate snappool
#
        lda     V_vddindx,r3            # Get VDX pointer (r3)
        ld      vx_vdd(r3)[r13*4],r5    # r5 = VDD of snapshot
        ld      vd_rdd(r5),r4
        ldob    rd_depth(r4),r8         # Get the preferred owner(stashed in depth)
.ifndef  MODEL_3000
.ifndef  MODEL_7400
        ldos    vd_attr(r5),r4
        setbit  vdbcacheen,r4,r4
        stos    r4,vd_attr(r5)          # Auto enable WC for a new snapshot
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
.endif  # MODEL_7400
.endif  # MODEL_3000
        ld      gnv_oger_array[r8*4],r8
        ldos    ogr_vid(r8),g3
#
        and     0x1,g3,r4               # Get the last bit of the snappool VID
        ldconst MAX_SNAPSHOT_COUNT,r6
        mulo    r4,r6,r8
        mulo    4,r8,r8
        lda     ssms_table,r4
        addo    r4,r8,r8                # Index into the ssms map for the proper
                                        # snappool.
        mov     0,r4
#
# --- Find the next open slot for an ssms
#
.bssl20:
        ld      (r8)[r4*4],r5
        cmpobe  0,r5,.bssl25            # Jif open slot
        addo    1,r4,r4
        cmpobl  r4,r6,.bssl20           # Keep checking
.bssl25:
        mov     0,r3                    # Prep for no slot
        cmpobe  r4,r6,.bss1000          # Jif no slot open
#
# --- Allocate a SSMS (snapshot management structure)
#
c       r3 = p_MallocC(ssm_size|BIT31, __FILE__, __LINE__); # Assign memory for SSMS and clear it
        st      r3,(r8)[r4*4]           # Update slot with ssms pointer
        stos    r4,ssm_ordinal(r3)      # Store the ordinal in the SSMS
        ldconst 0x1,r8
        and     r8,g3,r8
        stob    r8,ssm_prefowner(r3)    # Store the preferred owner
#
# --- Allocate a region map and dirty all segments.
#
#       Number segments for device
c       r6 = (*(UINT64*)&r11 >> 32);    # Number of "extra" (full RM)
c       g0 = (*(UINT64*)&r11 & 0xffffffffULL);
c       g0 = ((UINT64)g0 + SEGSIZE_sec - 1) / SEGSIZE_sec;
c       D_init_slrm(g0, r6, (SSMS*)r3); # Create region maps for Slink.
#
# --- Allocate an OGER and associate it with this snapshot.
#
        call    D$alloc_oger            # Allocate an OGER
        cmpobne 0,g0,.goodoger          # if allocation is good, don't skip out
        stos    g0,ssm_ogercnt(r3)      # zero the OGER count
        b       .missoger
#
.goodoger:
# nsegs = cal_seg(&seg, sda, eda, devCap);  Don't need nsegs nor eda -- note for snapshot.
c       (void)cal_seg_bit((UINT32*)&r8, (*(UINT64*)&r11)/2, (*(UINT64*)&r11)/2, *(UINT64*)&r11, 1);
        st      r8,ogr_sdakey(g0)       # r8
        ldconst 1,r8                    # Init OGER count
        st      g0,ssm_frstoger(r3)     # Update the first OGER
        st      g0,ssm_tailoger(r3)     # Update tail OGER
        stos    g3,ogr_vid(g0)          # assign snappool vid to this ogr
        stos    r8,ssm_ogercnt(r3)      # Update the OGER count
c fprintf(stderr,"%s%s:%u CreateSS: snappool vid=r4=%lx, ogercnt=r8=%lx\n", FEBEMESSAGE, __FILE__, __LINE__,g3,r8);
#
# --- Update the OGER in NV storage
#
        mov     g1,r8
        and     0x1,g3,r14
        ld      gnv_oger_array[r14*4],g1
        PushRegs(r10)                   # Save all "g" registers
        call    update_oger_nv
        PopRegs(r10)                    # Restore all "g" registers and g14 = 0
        cmpobe  ecok,g0,.bss135a        # Jif update_oger_nv successful
c fprintf(stderr, "%s%s:%u SNAPCREATION: Add OGER to NV failed\n", FEBEMESSAGE, __FILE__, __LINE__);
        ldconst TRUE,r7
.bss135a:
        mov     r8,g1
        ld      ssm_frstoger(r3),g0     # Restore g0
#
# --- Update snapshot and source SSMS pointers
#
.missoger:
        lda     V_vddindx,r14           # r14 = VDX for this cluster
        ld      vx_vdd(r14)[r15*4],r6   # r6 = Source VDD
        ld      vx_vdd(r14)[r13*4],r8   # r8 = SS VDD
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        stos    r15,vd_scorvid(r8)      # track the source VID for this snapshot
        stos    r15,ssm_srcvid(r3)      # Store the source VID in the SSMS
        stos    r13,ssm_ssvid(r3)       # Store the snapsnot VID in the SSMS
#
        st      r3,vd_incssms(r8)       # Update SSMS pointer in snapshot vdd
        ldconst vdsched,r14
        ldob    vd_status(r8),r10
        cmpobne r14,r10,.bssl36
        ldconst vdop,r14
        stob    r14,vd_status(r8)
.bssl36:
        cmpobe  0,g0,.missoger2
        ld      vd_outssms(r6),r14      # Add Snapshot to the list for this source
        cmpobne 0,r14,.bssl37           # Jif list not empty
        st      r3,vd_outssms(r6)       # Update ssms head
        b       .bssl38
#
.bssl37:
        ld      vd_outssmstail(r6),r14  # get the previous tail pointer
        st      r3,ssm_link(r14)        # and update that records forward link
.bssl38:
        st      r3,vd_outssmstail(r6)   # Update current source's tail
#
.missoger2:
        ldconst 100,r10
        stob    r10,vd_scpcomp(r8)      # Start with 100% sparse VID
#
        ldconst vdinop,r5
        cmpobe  0,g0,.missoger3
        ldconst vdop,r5                 # Set up operational device
.missoger3:
        stob    r5,vd_status(r8)        #  status
#
        cmpobne TRUE, r7,.bss1000       # Equal to TRUE means update oger to NV failed,
                                        # move the SS vdd  to inop.
        ldconst vdinop,r5
        stob    r5,vd_status(r8)        #  Move snapshot vdd  to inop
#
.bss1000:
        mov     r3,g0
        ret
#
#**********************************************************************
#
#  NAME: d$createslink
#
#  PURPOSE:
#       Process a create snapshot link request from the MMC.
#
#  INPUT:
#       g0 = DEF
#
#  OUTPUT:
#       g1 = return status code
#       g2 = return pkt size
#
#  REGS DESTROYED:
#       Reg. g1, g2 destroyed.
#
#**********************************************************************
#
d$createslink:
        mov     g0,r15                  # copy of packet address
        mov     g3,r14                  # save g3
        ldob    D_Vflag,r7              # save VDisk/VLink busy flag
#
# ---   Validate src vdd before proceeding further.
#
        ldconst deinvvirtid,g1          # Prep possible error code
        ldconst MAXVIRTUALS,r3          # Get maximum virtual IDs
        ldos    mvc_svid(r15),r13       # r13 = source VID
        cmpobge r13,r3,.dcsl100         # Jif invalid source VID
        lda     V_vddindx,r3            # Get VDX pointer (r3)
        ld      vx_vdd(r3)[r13*4],r6    # r6 = VDD
        cmpobe  0,r6,.dcsl100           # Jif vdd undefined
#
# --- Validate whether a SS Buffer (SNAPPOOL) is defined first
#     Also check to make sure we are not at max SS's
#
        ldconst demaxsnapshots,g1       # Error code for Max snapshots existing
#
        ldconst 0,r4                    # r4=RID
        ldconst 0,r9                    # count of snapshots (used for MAX detection)
.dcs01:
        ld      R_rddindx[r4*4],r5      # Get RDD pointer (r5)
        cmpobe  0,r5,.dcs10             # Jif undefined
#
        ldob    rd_type(r5),r8
        cmpobne rdslinkdev,r8,.dcs10
        addo    1,r9,r9                 # increment snapshot count
.dcs10:
        addo    1,r4,r4                 # Increment RID
        ldconst MAXRAIDS,r8
        cmpobne r8,r4,.dcs01            # Jif more RIDs
        ldos    snapshot_max_cnt,r8     # at max active snapshot count?
        cmpobe  r8,r9,.dcsl100          # ..yes, so error out
#
        ldconst deinvop,g1              # Error code for no snappool
#
        ldos    mvc_svid(r15),g0
        PushRegs(r4)
        call    find_owning_dcn
        PopRegs(r4)
        mov     g0,r4
#
        mov     r15,g0                  # Restore g0
        ldconst 0xffff,r10
        cmpobe  r10,r4,.dcsl100         # skip out if no snappool...keep r4 for rdd assignment
#
#       Make sure snapshot vid is < 2tb (or 64tb).
#
c       if (((VDD*)r6)->devCap > 0xffffffffULL) {
c         if (gss_version[r4] == SS_NV_VERSION) {
c           g1 = de2tblimit;            # Prep the error code (>2tb)
            b     .dcsl100              # Error out if too large
c         } else if (((VDD*)r6)->devCap >= (64ULL*1024*1024*1024*1024/512)) { # If greater than 64 terabytes
c           g1 = de64tblimit;           # Prep the error code (>64tb)
            b     .dcsl100              # Error out if too large
c         }
c       }
#
# --- Now verify whether we have sufficient space in the snappool to warrant creating a new snapshot
#     First, search vdisks to find the snappool that has same even/oddness of the owning dcn
#     Then look at the percent full reading on the spool
#
        ldconst 0,r13                   # r13=VID
.dcs11:
        lda     V_vddindx,r3            # Get VDX pointer (r3)
        ld      vx_vdd(r3)[r13*4],r5    # r5 = VDD
        cmpobe  0,r5,.dcs12             # Jif undefined
        ldos    vd_attr(r5),r8          # check for spool
        bbc     vdbspool,r8,.dcs12      # if not, then continue
        and     0x1,r13,r8
        cmpobe  r4,r8,.dcs12a           # got the spool that matches ownership
.dcs12:
        addo    1,r13,r13               # Increment VID
        ldconst MAXVIRTUALS,r8
        cmpobne r8,r4,.dcs11            # Jif more VIDs
        ldconst deinsres,g1             # make the response nasty enough...because
.ifdef M4_DEBUG_HARD
c abort();
.endif # M4_DEBUG_HARD
        b       .dcsl100                # impossible to get here...so...hmm..
#
.dcs12a:
        mov     r13,g1
        call    D$update_spool_percent_used
#
# --- Validate parameters (Virt ID, cluster, operation)
#
        ldos    mvc_svid(r15),r13       # r13 = source VID
        ldos    mvc_dvid(r15),g3        # g3 = Destination VID
        ldconst 355,g1                  # if source=355 and ss=455 then adjust max
        cmpobne g1,r13,.dcs13a
        ldconst 455,g1
        cmpobne g1,g3,.dcs13a
        ldconst MAX_SNAPSHOT_COUNT,g1
        stos    g1,snapshot_max_cnt     # backdoor to let support test with margins
.dcs13a:
        ldconst deinvvirtid,g1          # Prep possible error code
        ldconst MAXVIRTUALS,r3          # Get maximum virtual IDs
        cmpobge.f g3,r3,.dcsl100        # Jif invalid destination VID
        cmpobge.f r13,r3,.dcsl100       # Jif invalid source VID
#
# --- Check that source device exists
#
        lda     V_vddindx,r9            # r9 = VDX for this cluster
        ld      vx_vdd(r9)[r13*4],r13   # r13 = corresponding VDD
        ldconst deinvvirtid,g1          # Prep possible error code
        cmpobe  0,r13,.dcsl100          # Jif source doesn't exist
        ldos    vd_attr(r13),r11        # check to make sure source not WC
#
        ldconst deinvop,g1              # Prep possible error code
#
# ---   Allow write caching on source vdisk for model 7000, vdisk cache is
#       is ON by default on 7000
#
.ifndef MODEL_7000
.ifndef  MODEL_4700
        bbs     vdbcacheen,r11,.dcsl100 # if writecache enabled, exit
.endif  # MODEL_4700
.endif  # MODEL_7000
#
# --- Don't allow a SS of a SNAPPOOL or APOOL!
#
        bbs     vdbspool,r11,.dcsl100   # skip out if we are SS'ing a SNAPPOOL
        bbs     vdbasync,r11,.dcsl100   # skip out if we are SS'ing an APOOL
#
# --- Ensure that you don't snapshot a snapshot
#
        ld      vd_rdd(r13),r8          # get rdd first rdd in this vdisk
        ldob    rd_type(r8),r6          # Get the RAID type
        cmpobe  rdslinkdev,r6,.dcsl100  # skip out if we are SS'ing a SS
#
# --- And also don't allow a SS of a vlink
#
        cmpobe  rdlinkdev,r6,.dcsl100  # skip out if we are SS'ing a vlink
#
# --- Validate that we are not at MAX SS total size yet
#
# ----------------------------------------------------------------------------
# USED after here:
#                                  r13 = VDD to source vdisk
#    r4       r7                   r13 r14 r15
# ----------------------------------------------------------------------------
        ldconst TRUE,g1
        ldconst 0,g2                    # Old Mag classic compatibility
        stos    g3,D_Vvid               # save VDisk #
        stob    g2,D_Vcl                # save cluster #
        stob    g1,D_Vflag              # set flag indicating VDisk/VLink busy
#
# --- Check if VDisk already defined
#
c       if (*(VDD **)((UINT32)&V_vddindx + vx_vdd + g3*4) != NULL) {
            ldconst deinvvid,g1         # Prep possible error code
            b   .dcsl100                # if virtual ID already defined
c       }
#
# --- Find unused RAID device to use ----------------------------------
#
        lda     R_rddindx,r12           # r12 = base of RDD index
        ldconst MAXRAIDS-1,r11          # r11 = maximum number of RAID devices
        mov     1,r5                    # r5 = RAID ID being checked
        addo    4,r12,r12               # skip RAID #0
#
c   do {
c       if (*(RDD **)r12 == NULL) {
c         break;
c       }
        addo    4,r12,r12               # inc. to next RDD in table
        addo    1,r5,r5                 # inc. RAID number being checked
        subo    1,r11,r11               # dec. RAID number being checked
c   } while (r11 != 0);
c   if (r11 == 0) {
        ldconst deinvrid,g1             # return error to MMC
        b       .dcsl100
c   }
#
# --- Assign and initialize RDD
#
        mov     g3,r3                   # r3 = VDisk#
        ldconst rdslinkdev,g3           # g3 = RAID type code to initialize
        mov     r5,g2                   # g2 = RAID ID to initialize
        ldconst 0,g1                    # Clear expected LDD pointer
#
        call    d$brddpsdld             # build and initialize RDD/PSD/RDI
                                        # g0 = RDD address of RAID
        st      g0,(r12)                # save RDD address in RAID table
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        ldos    R_rddindx+rx_ecnt,r11   # Get RAID count
        addo    1,r11,r11               # Bump it
        stos    r11,R_rddindx+rx_ecnt   # Set new RAID count
#
        mov     r13,r11                 # r11 = VDD of source vdisk
        mov     g0,r13                  # r13 = RDD pointer
        stos    r3,rd_vid(r13)          # Store the VID into RDD
# save RAID device capacity in RDD
c       ((RDD *)g0)->devCap = ((VDD *)r11)->devCap;
# Save segment length of PSD
c       ((RDD *)g0)->extension.pPSD[0]->sLen = ((VDD *)r11)->devCap;
# ----------------------------------------------------------------------------
# USED after here:
#                                  r13 = pointer to RDD
#                          r11 = VDD of source vdisk
#    r4       r7           r11     r13 r14 r15
# ----------------------------------------------------------------------------
#
# --- Assign and initialize VDD
#
        call    D_allocvdd              # Get a cleared VDD
        mov     g0,r8                   # r8 = VDD address
        st      r13,vd_rdd(r8)          # store the RDD pointer in the VDD
        ldos    mvc_svid(r15),r6        # r6 = source VID
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        st      r6,rd_sps(r13)          # store source VID in an unused rdd field for NVRAM store
        stob    r4,rd_depth(r13)        # store snapshot owner in an unused rdd field for NVRAM store
#
# --- Initialize and assign snapshot info
#
        ldos    mvc_dvid(r15),g1        # SS VID
        ldos    mvc_svid(r15),g2        # source VID
#
        lda     V_vddindx,r12           # r12 = VDX for this cluster
        st      r8,vx_vdd(r12)[g1*4]    # Store the VDD pointer in the array
        stos    g1,vd_vid(r8)           # Set up virtual ID
        ldconst 1,r3                    # Set up segment count
        stob    r3,vd_raidcnt(r8)       # Set up RAID count
        ldos    V_vddindx+vx_ecnt,r3    # Get vdisk count
        addo    1,r3,r3                 # Bump it
        stos    r3,V_vddindx+vx_ecnt    # Set new vdisk count
# save VDisk capacity in VDD
c       ((VDD *)r8)->devCap = ((VDD *)r11)->devCap;
#
        ldconst vdsched,r3              # Set up operational device
        stob    r3,vd_status(r8)        #  status
        stos    g2,vd_scorvid(r8)       # Store soruce vid in the destination VDD
.ifndef  MODEL_3000
.ifndef  MODEL_7400
        ldos    vd_attr(r8),r3
        setbit  vdbcacheen,r3,r3
        stos    r3,vd_attr(r8)
.endif  # MODEL_7400
.endif  # MODEL_3000
#
# --- Get and store the VDisk create time in seconds
c       r3 = GetSysTime();              # Get seconds since epoch
        st      r3,vd_createTime(r8)    # Store the time in seconds
#
# --- Update NVRAM
#
        mov     g1,g0                   # VID to update
        ldconst FALSE,g1                # Addition of a new VDD (not deleting)
        call    D$updrmtcachesingle     # Update single
#
        call    D$p2updateconfig        # save P2 NVRAM
        call    D$SendRefreshNV         # tell other nodes to refresh NVRAM
#
        mov     deok,g1                 # Return OK status
#
# --- Exit
#
.dcsl100:
        stob    r7,D_Vflag              # restore VDisk/VLink busy flag
#
        mov     r15,g0                  # restore g0
        mov     r14,g3                  # restore g3
        ret
#
#******************************************************************************
#
#  NAME:  d$dealloc_slrm
#
#  PURPOSE:
#       Deallocates ssm segment and region maps tables
#
#  INPUT:
#       g0 = SSMS address
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#******************************************************************************
#
d$dealloc_slrm:
        movl    g0,r12                  # save g0-g1
#
c       r3 = 32;                        # number of ssm_regmap[]
.dsrm50:
        cmpobe  0,r3,.dsrm1000          # exit after doing 32.
c       r3--;
        ld      ssm_regmap(r12)[r3*4],g0 # g0 = region map address
        cmpobe.f 0,g0,.dsrm50           # Jif no region map table defined
c       r9 = maxRMcnt-1;                # number of region tables -1
#
.dsrm100:
        ld      RM_tbl(g0)[r9*4],g1     # g0 = address of segment table
        cmpobe.t 0,g1,.dsrm200          # Jif no table
# Deallocate segment map table
c       p_Free(g1, sizeof(SM), __FILE__, __LINE__);
        st      0,RM_tbl(g0)[r9*4]      # clear out address
#
.dsrm200:
        cmpobe.f 0,r9,.dsrm300          # Jif last region segment table
c       r9--;                           # decrement RM index
        b       .dsrm100                # continue
#
.dsrm300:
# Release region map table (RM)
c       p_Free(g0, sizeof(RM), __FILE__, __LINE__);
        st      0,ssm_regmap(r12)[r3*4] # clear pointer to RMAp
        b       .dsrm50                 # Do the next region map
#
.dsrm1000:
        movl    r12,g0                  # restore g0-g1
        ret                             # return to caller
#
#******************************************************************************
        .data
# extern  OGER            *gnv_oger_array[2];
gnv_oger_array:
        .space  8,0                     # Pointers to nv_ogers
#
# extern  SS_HEADER_NV    *gnv_header_array[2];
gnv_header_array:
        .space  8,0                     # Pointers to ss_header_nv
#
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
        .globl  DEF_dog_table           # C access
DEF_dog_table:
dog_table:                              # Bitfield showing allocated OGERs
# First snappool
# NOTE: first OGER is where the HEADER and SSMS's are stored (thus the 1 bit below).
        .byte   0x01                    # OGER map for first snappool
        .space  (MAX_OGER_COUNT_GT2TB/8)-1,0
# Second snappool
# NOTE: first OGER is where the HEADER and SSMS's are stored (thus the 1 bit below).
        .byte   0x01                    # OGER map for second snappool
        .space  (MAX_OGER_COUNT_GT2TB/8)-1,0
# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# --- The following structure converts an 8 bit value into a numerical representation
#     of the first unset(cleared) bit number, 0 based.
#
dog_nxtopn:
        .byte   0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,5     # 0-31
        .byte   0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,6     # 32-63
        .byte   0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,5     # 64-95
        .byte   0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,7     # 96-127
        .byte   0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,5     # 128-159
        .byte   0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,6     # 160-191
        .byte   0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,5     # 192-223
        .byte   0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,4,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0,255   # 224-255
#
# --- The following converts an 8 bit value into the number of bits set in that value.
#
        .globl  d_bitcnt              # C access
d_bitcnt:
        .byte   0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5     # 0-31
        .byte   1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6     # 32-63
        .byte   1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6     # 64-95
        .byte   2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7     # 96-127
        .byte   1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6     # 128-159
        .byte   2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7     # 160-191
        .byte   2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7     # 192-223
        .byte   3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,4,5,5,6,5,6,6,7,5,6,6,7,6,7,7,8     # 224-255
#
        .text
#******************************************************************************
#
#  NAME:  D$alloc_oger
#
#  PURPOSE:
#       allocate and initializes an OGER structure
#
#  DESCRIPTION:
#       Allocates an OGER (One Gig er) structure and initialize if for the
#       snapshot SSMS. An OGER is assigned from the OGER pool contained in
#       a single Vdisk that contains all of the OGERs.
#
#  INPUT:
#       g3 = Snappool Vid
#
#  OUTPUT:
#       g0 = OGER
#
#  REGS DESTROYED:
#       g0.
#
#******************************************************************************
#
D$alloc_oger:
        mov     g3,r4
        and     0x1,g3,r10              # which snappool
        ldconst MAX_OGER_COUNT_GT2TB/8,r6   # Which dog_table snappool (2nd index)
        mulo    r6,r10,r10              # r10 is offset for spool in dog_table
        lda     V_vddindx,r7            # Get VDX pointer (r3)
        ld      vx_vdd(r7)[r4*4],r5     # r5 = VDD
# SNAPSHOT theoretical problem - if devCap really really big, r8 overflows.
c       *(UINT64*)&r12 = ((VDD*)r5)->devCap / (2048*1024); # r12/r13 convert sectors to GB. (21 bits gone)
c       r9 = *(UINT64*)&r12 % 8;        # Check for partial byte
# NOTDONEYET -- SNAPSHOT -- r3 below means that snappool has a limit of 56 bits (never get that big!).
c       r3 = *(UINT64*)&r12 / 8;
c if (r3 > (r6)) {
c   fprintf(stderr,"%s%s:%u D$alloc_oger r3=%ld > r6=%ld\n", FEBEMESSAGE, __FILE__, __LINE__, r3, r6);
c   abort();
c }
        cmpobe  0,r9,.alog10            # Jif no remainder
        ldob    dog_table(r10)[r3*1],r5 # Get the last byte in the table
#
# --- There is a remainder so we need to mark the top bits of the last byte as used (set).
#
        mov     r9,r4                   # Bit to set
.alog05:
        cmpobe  8,r4,.alog9             # Jif at the end of the byte
        setbit  r4,r5,r5                # backfill rest of byte
        addo    1,r4,r4
        b       .alog05
#
.alog9:                                 # done filling in rest of byte
        stob    r5,dog_table(r10)[r3*1] # Update the last byte in the table
        addo    1,r3,r3
.alog10:
        mov     0,r4
        ldconst 0xff,r6                 # Signifies all full
.alog100:
        ldob    dog_table(r10)[r4*1],r5 # Get a byte in the table
        cmpobne r6,r5,.alog150          # Jif not full (all bits set)
        addo    1,r4,r4
        cmpobl  r4,r3,.alog100          # Jif more to check
#
# --- No free OGERs were found, exit
#
.alog120:
        mov     0,g0
        b       .alog1002
#
# --- Found a free OGER, allocate it and return
#
.alog150:
        ldob    dog_nxtopn(r5),r7       # Compute first clear bit position
        bbs     r7,r5,.alog120          # This is an error condition
        setbit  r7,r5,r5
        stob    r5,dog_table(r10)[r4*1] # Update the OGER table
        ldconst OGERSIZE,r9
        lda     (r7)[r4*8],r8           # r8 = OGER ordinal
        PushRegs(r11)
c       r10 = ss_check_for_duplicate_oger(r8, g3);
        PopRegsVoid(r11)
        cmpobe  TRUE,r10,.alog120       # Duplicate Oger found, exit
c       *(UINT64*)&r9 = (UINT64)r8 * r9; # Calculate SDA r9/r10
#
c       g0 = p_MallocC(ogr_size|BIT31, __FILE__, __LINE__); # Assign memory for OGER.
        stl     r9,ogr_sda(g0)          # Store the SDA r9/r10
        stos    0,ogr_segcnt(g0)        # Store the number of segments
        stos    r8,ogr_ord(g0)          # Store the OGER ordinal
#
# calculate percent of spool used
        and     0x1,g3,r4
        ldob    spool_percent_full[r4*1],r6 # load old value in case we have issues
        ld      oger_cnt[r4*4],r10
        addo    1,r10,r10
        st      r10,oger_cnt[r4*4]
        mov     g3,r4                   # r4 = snappool vid
        lda     V_vddindx,r3            # Get VDX pointer (r3)
        ld      vx_vdd(r3)[r4*4],r5     # r5 = VDD
        cmpobe  0,r5,.alog1001          # Jif undefined
c       *(UINT64*)&r6 = ((VDD*)r5)->devCap / (2048*1024); # convert sectors to GB. (21 bits gone)
c   if (*(UINT64*)&r6 == 0) {
        b       .alog1001               # this would be odd, but just in case...
c   }
        addo    1,r10,r10               # add in the nv oger
c       r11 = r10 * 100;
c       *(UINT64*)&r6 = r11 / (*(UINT64*)&r6);
        stob    r6,vd_scpcomp(r5)       # Can only be 0 through 100.
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
        #
        # Now calcalate whether we are FULL (100%), or >=80%, if were not before
        cmpobl  r6,80,.alog1001        # no messages if <80% still
        cmpobne 100,r6,.alog1000a      # if !=100% full
#
# --- Log error message
#
        mov     g0,r10
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst mleSPOOL_CHANGE_E,r3    # Event code
        st      r3,mle_event(g0)        # Store as word to clear other bytes
#
        ldconst SS_BUFFER_FULL,r4
        stob    r4,esp_sub_event_code(g0)
        ldconst 0,r4
        stob    r4,esp_skip_notify(g0)
        st      r4,esp_errorCode(g0)
        and     0x1,g3,r5
        st      r5,esp_value1(g0)
        st      r4,esp_value2(g0)
#
c       MSC_LogMessageStack(&TmpStackMessage[0], esplen);
        mov     r10,g0
        b       .alog1001
#
.alog1000a:
        and     0x1,g3,r4
        ldob    spool_percent_full[r4*1],r8   # put full warning message out
        cmpobge r8,80,.alog1001         # ..but not if already have
        mov     g0,r10
#
# --- Log warning message
#
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst mleSPOOL_CHANGE_W,r3    # Event code
        st      r3,mle_event(g0)        # Store as word to clear other bytes
#
        ldconst SS_BUFFER_FULL_WARN,r4
        stob    r4,esp_sub_event_code(g0)
        ldconst 0,r4
        stob    r4,esp_skip_notify(g0)
        st      r4,esp_errorCode(g0)
        and     0x1,g3,r5
        st      r5,esp_value1(g0)
        st      r4,esp_value2(g0)
#
c       MSC_LogMessageStack(&TmpStackMessage[0], esplen);
        mov     r10,g0
.alog1001:
        and     0x1,g3,r4
        stob    r6,spool_percent_full[r4*1]
.alog1002:
        ret
#
#******************************************************************************
#
#  NAME:  D$dalloc_oger
#
#  PURPOSE:
#       Deallocate an OGER structure
#
#  DESCRIPTION:
#       Deallocates an OGER (One Gig er) structure and returns it to the free
#       pool.
#
#  INPUT:
#       g0 = OGER
#
#  OUTPUT:
#       g0 = Not any good.
#
#******************************************************************************
#
D$dalloc_oger:
        ldos    ogr_ord(g0),r3          # Get the ordinal of OGER to remove
        shro    3,r3,r4                 # r4 = byte index
        remo    8,r3,r3                 # r3 = bit index
        ldos    ogr_vid(g0),r9
        and     0x1,r9,r10
        ldconst MAX_OGER_COUNT_GT2TB/8,r7   # Which dog_table snappool (2nd index)
        mulo    r7,r10,r10              # r10 is offset for spool in dog_table
        ldob    dog_table(r10)[r4*1],r5 # Get a byte in the table
        clrbit  r3,r5,r5
        stob    r5,dog_table(r10)[r4*1]      # Update the OGER table
c       p_Free(g0, ogr_size, __FILE__, __LINE__);
        ret
#
#******************************************************************************
#
#  NAME:  D$rma_oger
#
#  PURPOSE:
#       Remove all OGERs from an SSMS
#
#  DESCRIPTION:
#       Deallocates all of the OGERs in an SSMS
#
#  INPUT:
#       g0 = SSMS
#
#  OUTPUT:
#       NONE
#
#  REGS DESTROYED:
#       NONE
#
#******************************************************************************
#
D$rma_oger:
        mov     g0,r15                  # r15 is the SSMS
        ld      ssm_frstoger(r15),r3
# calculate which oger_cnt to access
        ldob    ssm_prefowner(r15),g0
        and     0x1,g0,r5               # safety
#
        ld      oger_cnt[r5*4],r10      # This global oger count is not used now.
        ldos    ssm_ogercnt(r15),r11    # get oger count from ssms
.rmo100:
        cmpobe  0,r3,.rmo200            # Jif no more to release
        cmpobe  0,r11,.rmo200
        subo    1,r10,r10
        subo    1,r11,r11
#
        ld      ogr_link(r3),r4         # Get the next oger to release
        mov     r3,g0
        call    D$dalloc_oger
        mov     r4,r3                   # Advance to the next OGER
        b       .rmo100                 # Continue
#
.rmo200:
# calculate percent of oger used
        st      r10,oger_cnt[r5*4]
        stos    r11,ssm_ogercnt(r15)
        ldob    ssm_prefowner(r15),g0
        ld      gnv_oger_array[g0*4],r5
        ldos    ogr_vid(r5),r4
        movl    0,r5
        stl     r5,ssm_frstoger(r15)    # Clear OGER pointers
        stos    r5,ssm_ogercnt(r15)     # and count
#
#        ldconst 10000,r7
#        cmpobe  r7,r4,.rmo1001
        lda     V_vddindx,r3            # Get VDX pointer (r3)
        ld      vx_vdd(r3)[r4*4],r5     # r5 = VDD
        cmpobe  0,r5,.rmo1001           # Jif undefined
        ldos    vd_vid(r5),r3
c       *(UINT64*)&r6 = ((VDD*)r5)->devCap / (2048*1024); # convert sectors to GB. (21 bits gone)
c   if (*(UINT64*)&r6 == 0) {
        b       .rmo1001
c   }
        cmpobe  0,r10,.rmo300           # jif 0 ogers present
        addo    1,r10,r10               # add in the nv oger
.rmo300:
c       r11 = r10 * 100;
c       *(UINT64*)&r6 = r11 / (*(UINT64*)&r6);
        stob    r6,vd_scpcomp(r5)       # Can only be 0 through 100.
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
#
# Now calculate whether we are empty (0%), or <80%, if were not before
#
        ldconst 80,r7
        # Now calcalate whether we are FULL (100%), or >=80%, if were not before
        cmpobge r6,r7,.rmo1001        # no messages if >=80% still
        cmpobne 0,r6,.rmo1000a        # if !=0% full
#
# --- Log empty message
#
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst mleSPOOL_CHANGE_I,r3    # Event code
        st      r3,mle_event(g0)        # Store as word to clear other bytes
#
        ldconst SS_BUFFER_EMPTY,r4
        stob    r4,esp_sub_event_code(g0)
        ldconst 0,r4
        stob    r4,esp_skip_notify(g0)
        st      r4,esp_errorCode(g0)
        ldob    ssm_prefowner(r15),r5
        st      r5,esp_value1(g0)
        st      r4,esp_value2(g0)
#
c       MSC_LogMessageStack(&TmpStackMessage[0], esplen);
        b       .rmo1001
#
.rmo1000a:
        and     0x1,r4,r3
        ldob    spool_percent_full[r3*1],r8 # put <80% message out
        cmpobl  r8,r7,.rmo1001              # ..but not if already have
        stob    r6,spool_percent_full[r3*1] # put <80% message out
#
# --- Log <80%  message
#
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst mleSPOOL_CHANGE_I,r3    # Event code
        st      r3,mle_event(g0)        # Store as word to clear other bytes
#
        ldconst SS_BUFFER_FULL_OK,r4
        stob    r4,esp_sub_event_code(g0)
        ldconst 0,r4
        stob    r4,esp_skip_notify(g0)
        st      r4,esp_errorCode(g0)
        ldob    ssm_prefowner(r15),r5
        st      r5,esp_value1(g0)
        st      r4,esp_value2(g0)
#
c       MSC_LogMessageStack(&TmpStackMessage[0], esplen);
.rmo1001:
#
        mov     r15,g0                  # Restore g0
        ret
#
#**********************************************************************
#
#  NAME: d$kill_ss
#
#  PURPOSE:
#       To remove delete all information pertaining to a snapshot.
#
#  DESCRIPTION:
#       Accepts a pointer to an SSMS structure. All sub structures
#       including Sync records, OGERs, and Region maps are released.
#       Also any references to the snapshot are removed from the source
#       device structures. It is assumed that I/O has been stopped so
#       that all sync related I/O's will have completed.
#
#  INPUT:
#       g0 = SSMS
#       g8 = VDD for this snapshot
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
d$kill_ss:
        movl    g0,r14                  # Save g0-1
        cmpobe  0,g0,.kss1000           # Jif no SSMS pointer
        call    d$clean_ssms_refs
#
#        call    D$p2updateconfig        # save P2 NVRAM
#        call    D$SendRefreshNV         # tell other nodes to refresh NVRAM
#
        ld      ssm_synchead(g0),r3
        cmpobe 0,r3,.kss994              # Jif no sync records outstanding
        b       .kss1000
#
# --- If virtual is using this SSSM (building sync groups) then wait for it to finish before
#     freeing resources.
#
.kss994:
        ld      syg_ssms,r3
        ld      bldsgi_ssms,r4
        cmpobe  g0,r3,.kss995          # Jif virtual is working on this SSMS
        cmpobe  g0,r4,.kss995          # Jif virtual is working on this SSMS
        b       .kss996                # Go free resources
#
.kss995:
        mov     g0,r7
        ldconst 1,g0
        call    K$twait
        mov     r7,g0
        b       .kss994
#
.kss996:
#
# --- Remove all OGERs associated.
#
        call    D$rma_oger
#
# --- Deallocate Region and Segment maps
#
        call    d$dealloc_slrm
#
# --- Release the SSMS
#
        lda     gnv_header_array,r4
        ldob    ssm_prefowner(g0),r5
        ldconst 4,r6
        mulo    r5,r6,r6
        addo    r4,r6,r4
#
        lda     gnv_oger_array,r7
        addo    r7,r6,r7
#
        mov     g0,r9
        ld      (r4),g0
        ld      (r7),g1
#
        lda     ssms_table,r4
        ldos    ssm_ordinal(r9),r7
        ldob    ssm_prefowner(r9),r6
        lda     MAX_SNAPSHOT_COUNT*4,r5
        mulo    r5,r6,r6
        addo    r6,r4,r4            # Set offset for first or second snappool
        lda     (r4)[r7*4],r4
        mov     0,r7
        st      r7,(r4)             # Clear the ssms pointer
#
        PushRegs(r7)                # Save all "g" registers
        call    update_header_nv
        PopRegs(r7)                 # Restore all "g" registers
#
c       p_Free(r9, ssm_size, __FILE__, __LINE__);
#
# --- Exit point
#
.kss1000:
        movl    r14,g0                  # Restore g0-1
        ret
#
#**********************************************************************
#
#  NAME: d$kill_ss_structs
#
#  PURPOSE:
#       To remove delete all structs for a snapshot.
#
#  DESCRIPTION:
#       Accepts a pointer to an SSMS structure. All sub structures
#       including Sync records, OGERs, and Region maps are released.
#
#  INPUT:
#       g0 = SSMS
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
        .globl  DEF_Kill_SS_Structs      # C access
DEF_Kill_SS_Structs:
        movl    g0,r14                  # Save g0-1
        cmpobe  0,g0,.ksst1000          # Jif no SSMS pointer
        call    d$clean_ssms_refs
#
        ld      ssm_synchead(g0),r3
        cmpobe 0,r3,.ksst994            # Jif no sync records outstanding
        b       .ksst1000
#
# --- If virtual is using this SSSM (building sync groups) then wait for it to finish before
#     freeing resources.
#
.ksst994:
        ld      syg_ssms,r3
        ld      bldsgi_ssms,r4
        cmpobe  g0,r3,.ksst995          # Jif virtual is working on this SSMS
        cmpobe  g0,r4,.ksst995          # Jif virtual is working on this SSMS
        b       .ksst996                # Go free resources
#
.ksst995:
        mov     g0,r7
        ldconst 1,g0
        call    K$twait
        mov     r7,g0
        b       .ksst994
#
.ksst996:
#
# --- Remove all OGERs associated.
#
        call    D$rma_oger
#
# --- Deallocate Region and Segment maps
#
        call    d$dealloc_slrm
#
# --- Release the SSMS
#
        lda     ssms_table,r4
        ldos    ssm_ordinal(g0),r7
        ldob    ssm_prefowner(g0),r6
        lda     MAX_SNAPSHOT_COUNT*4,r5
        mulo    r5,r6,r6
        addo    r6,r4,r4            # Set offset for first or second snappool
        lda     (r4)[r7*4],r4
        mov     0,r7
        st      r7,(r4)             # Clear the ssms pointer
c       p_Free(g0, ssm_size, __FILE__, __LINE__);
#
# --- Exit point
#
.ksst1000:
        movl    r14,g0                  # Restore g0-1
        ret
#
#**********************************************************************
#
#  NAME: d$clean_ssms_refs
#
#  PURPOSE:
#       To remove reference to a specific ssms from VDDs.
#
#  DESCRIPTION:
#       Accepts a pointer to an SSMS structure. All VDD references
#       are removed and the snapshot VDD is marked as inop.
#
#  INPUT:
#       g0 = SSMS
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
        .globl  d$clean_ssms_refs
d$clean_ssms_refs:
        ldos    ssm_ssvid(g0),r5        # r5 = snapshot VID
        lda     V_vddindx,r7            # Get VDX pointer
        ld      vx_vdd(r7)[r5*4],r7     # r7 = corresponding VDD for SS
        cmpobe  0,r7,.csrf1             # Jif no VDD
        mov     0,r5
        st      r5,vd_incssms(r7)       # Clear the SSMS pointer
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
#
#
# --- Find the source VDD and remove reference to the SSMS
#
.csrf1:
        ldos    ssm_srcvid(g0),r3       # r3 = Source VID
#
        lda     V_vddindx,r6            # Get VDX for this cluster
        ld      vx_vdd(r6)[r3*4],r5     # r5 = Source VDD
        cmpobe  0,r5,.csrf10            # Jif no Source VDD
#
        ldl     vd_outssms(r5),r6       # r6 = head SSMS
                                        # r7 = tail SSMS
        mov     0,r8                    # r8 = previous SSMS pointer
        cmpobe  0,r6,.csrf10            # Jif list already empty
#
# --- Search for a matching SSMS entry
#
.csrf05:
        cmpobe  g0,r6,.csrf06           # Jif found a match
        mov     r6,r8                   # Save as previous
        ld      ssm_link(r6),r6         # Advance to next SSMS
        cmpobne 0,r6,.csrf05            # Jif more to search
        b       .csrf10                 # Could not find match
#
# --- Found a match, remove it from list. 4 Cases: 1--This is head. 2--This is tail.
#     3--This is somewhere in middle. 4--This is head and tail.
#
.csrf06:
        ld      vd_outssms(r5),r9       # r9 = head SSMS
                                        # r7 = tail SSMS
        cmpobne r9,r6,.csrf08           # Jif not head
        cmpobne r7,r6,.csrf07           # Jif case 1
# --- This is case 4
        movl    0,r8
        stl     r8,vd_outssms(r5)       # Clear both head and tail
        b       .csrf10
#
.csrf07:
# --- This is case 1
        ld      ssm_link(r6),r7
        st      r7,vd_outssms(r5)       # Update head pointer
        b       .csrf10
#
.csrf08:
        cmpobe  r6,r7,.csrf09           # Jif tail exists
# --- This is case 3
        ld      ssm_link(r6),r7
        st      r7,ssm_link(r8)         # Link next to previous
        b       .csrf10
#
.csrf09:
# --- This is case 2
        mov     0,r12
        st      r12,ssm_link(r8)        # Update previous to null
        st      r8,vd_outssmstail(r5)   # Update tail pointer
#
.csrf10:
        ldos    ssm_ssvid(g0),r3        # r3 = Snapshot VID
#
        lda     V_vddindx,r6            # Get VDX for this cluster
        ld      vx_vdd(r6)[r3*4],r5     # r5 = SS VDD
        cmpobe  0,r5,.csrf20            # Jif no SS VDD
#
.csrf20:
        ret
#
#**********************************************************************
#
#  NAME: D$spool_expand
#
#  PURPOSE:
#       Clear the bitmap of the Ogers before snappool expansion.
#
#  DESCRIPTION:
#       If the number of gigabytes of snappool is not exact multiple
#       of 8, in the last byte of the bit map, invalid bits are set to
#       one. These bits are cleared in this function if they are there,
#       (i.e. if the devcap of snappool before expanding is not multiple
#       of 8) otherwise it does nothing.
#
#  INPUT:
#       g2 = VID of snap pool
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
D$spool_expand:
        mov     g2,r4
        lda     V_vddindx,r7            # Get VDX pointer (r3)
        ld      vx_vdd(r7)[r4*4],r5     # r5 = VDD

c       *(UINT64*)&r8 = ((VDD*)r5)->devCap / (2048*1024); # convert sectors to GB. (21 bits gone)
c       r4 = *(UINT64*)&r8 % 8;                    # Check for partial byte
# NOTDONEYET -- SNAPSHOT -- r3 below means that snappool has a limit of 56 bits (never get that big!).
c       r3 = *(UINT64*)&r8 / 8;                    # Convert bit count(GB count)into byte count
c   if (r4 == 0) {                      # If no remainder
        b       .esp100
c   }
c       r10 = (0x1 & g2) * (MAX_OGER_COUNT_GT2TB/8); # Which snappool (2nd index).
        ldob    dog_table(r10)[r3*1],r5 # Get the last byte in the table
#
# --- There is a remainder so we need to clear the top bits of the last byte
#
.esp10:
        cmpobe  8,r4,.esp20             # Jif at the end of the byte
        clrbit  r4,r5,r5                # backfill rest of byte
        addo    1,r4,r4
        b       .esp10
#
.esp20:                                 # done filling in rest of byte
        stob    r5,dog_table(r10)[r3*1] # Update the last byte in the table
.esp100:
        ret
#
#**********************************************************************
#
#  NAME: D$update_spool_percent_used
#
#  PURPOSE:
#       Calculate the percentage used field on the snap pool owned by
#       this controller.
#
#  INPUT:
#       g1 = VID of snap pool
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
DEF_update_spool_percent_used:
        mov     g0,g1                  # get vid
D$update_spool_percent_used:
# -- Find the owning DCN of this pool
        lda     V_vddindx,r3            # Get VDX pointer (r3)
        ld      vx_vdd(r3)[g1*4],r5     # r5 = VDD
        cmpobe  0,r5,.dusp100           # Jif vdd undefined
#
        ld      K_ficb,r6               # r6 = FICB address
        ld      fi_cserial(r6),r7       # r5 = my serial number
        and     0x1,r7,r7
        ldob    vd_owner(r5),r8
        cmpobne r7,r8,.dusp100
#
        ldconst 0,r6
        ld      oger_cnt[r8*4],r10
        cmpobe  0,r10,.dusp50
# NOTDONEYET - SNAPSHOT theoretical problem - if devCap really really big, r6 overflows.
c       r6 = ((VDD*)r5)->devCap / (2048*1024); # convert sectors to GB. (21 bits gone)
        ldconst 100,r7
        addo    1,r10,r10               # add in the nv oger
        mulo    r7,r10,r11
        divo    r6,r11,r6
.dusp50:
        stob    r6,vd_scpcomp(r5)
c       BIT_SET(DMC_bits, CCB_DMC_vdiskcache);  /* Flag vdisk data has changed. */
.dusp100:
        ret
#
####
## Modelines:
## Local Variables:
## tab-width: 4
## indent-tabs-mode: nil
## End:
## vi:sw=4 ts=4 expandtab
