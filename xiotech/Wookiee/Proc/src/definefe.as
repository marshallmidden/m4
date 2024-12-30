# $Id: definefe.as 161678 2013-09-18 19:25:16Z marshall_midden $
#**********************************************************************
#
#  NAME: definefe.as
#
#  PURPOSE:
#       To provide complete support of configuration definition requests
#       on the front end processor..
#
#  FUNCTIONS:
#       D$init            - Define initialization
#       D$que             - Queue define request
#
#       This module employs 1 processes:
#       d$exec            - Executive (1 copy)
#
#  Copyright (c) 1997-2008  Xiotech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- global function declarations ------------------------------------
#
        .globl  D$init                  # Module initialization
#
# --- global data declarations ----------------------------------------
#
        .globl  S_sddindx               # Index to SDD index
        .globl  gSDX
        .globl  gTDX
        .globl  defTraceQue
        .globl  MAGD_SCSI_WHQL          # WHQL compliance enable
#
# --- target table
#
        .globl  T_tgdindx               # Index to targets
#
# --- global usage data definitions -----------------------------------
#
        .data
        .align  2
gSDX:
        .space  4,0                     # Entry count for SDX
S_sddindx:
        .space  MAXSERVERS*4,0          # Index to SDD structures
#
gTDX:
        .space  4,0                     # Entry count for TGDX
T_tgdindx:
        .space  MAXTARGETS*4,0          # Index to TGD structures
#
# --- local usage data definitions ------------------------------------
#
# --- Ordered list of action routines organized by packet type
#
        .align  2
d_pktact:                                                                   # MRP value in tracelog
        .word   d$cbridge               # (0x200) Configure server                  0
        .word   d$confservercmplt       # (0x201) Configure server complete         1
        .word   d$confcache             # (0x202) Configure cache                   2
        .word   d$obsolete              # (0x203) OBSOLETE....                      3
        .word   d$obsolete              # (0x204) OBSOLETE....                      4
        .word   d$cachestop             # (0x205) Invoke cache stop                 5
        .word   d$cacheresume           # (0x206) Invoke cache resume               6
        .word   d$assignsysinfo         # (0x207) Assign system information         7
        .word   d$vdiskchange           # (0x208) Virtual disk has changed          8
        .word   d$serverchange          # (0x209) Server has changed                9
        .word   d$cbridge               # (0x20A) Configure a target                10
        .word   d$resetconfig           # (0x20B) Reset configuration               11
        .word   d$setcontrolsn          # (0x20C) Set controller serial number      12
        .word   d$NV_ProcessMMInfo      # (0x20D) Send MM_INFO                      13
        .word   d$setconfigopt          # (0x20E) Set configurable options BE => FE 14
        .word   d$cbridge               # (0x20F) Configure update for target       15
        .word   d$cbridge               # (0x210) Get Port Type                     16
        .word   d$cbridge               # (0x211) Configure CHAP user               17
        .word   d$cbridge               # (0x212) Set iSNS information              18
        .word   d$cbridge               # (0x213) Config Persistent Reserve         19
        .word   d$cbridge               # (0x214) Set Foreign Target (on/off)       20
#
        .word   d$cbridge               # (0x500) Get loop statistics               21
.if ISCSI_CODE
        .word   d$cbridge               # (0x501) Get server information            22
.else   # ISCSI_CODE
        .word   d$getsdata              # (0x501) Get server information            22
.endif  # ISCSI_CODE
        .word   d$getcdata              # (0x502) Get cache statistics              23
        .word   d$getlink               # (0x503) Get link statistics               24
        .word   d$getii                 # (0x504) Get II statistics                 25
        .word   d$getcddata             # (0x505) Get cache device statistics       26
        .word   d$cbridge               # (0x506) Get server statistics             27
        .word   d$setbathealth          # (0x507) Set battery health                28
        .word   d$cbridge               # (0x508) Resume cache initialization       29
        .word   d$obsolete              # (0x509) Was Get frontend boot code header 30
        .word   d$obsolete              # (0x50A) Was Get frontend diag code header 31
        .word   d$getproc               # (0x50B) Get frontend proc code header     32
        .word   D$nop                   # (0x50C) No op                             33
        .word   d$rwmemory              # (0x50D) Read/write memory                 34
        .word   d$cbridge               # (0x50E) Reset FE chip                     35
        .word   d$wwntidlookup          # (0x50F) Server lookup based on WWN/TID    36
        .word   d$cbridge               # (0x510) Generic                           37
        .word   d$setmpconfigfe         # (0x511) Set Mirror partner configuration  38
        .word   d$fibrehlist            # (0x512) FE Fibre Heartbeat List - DLM     39
        .word   d$contwomp              # (0x513) Continue Cache Init w/o MP        40
        .word   d$flushwomp             # (0x514) Flush FE Cache w/o MP             41
        .word   d$invfewc               # (0x515) Invalidate the FE Write Cache     42
        .word   d$flushbewc             # (0x516) Flush the BE Write Cache          43
        .word   d$invbewc               # (0x517) Invalidate the BE Write Cache     44
        .word   d$modepage              # (0x518) Mode page                         45
        .word   d$cbridge               # (0x519) Get device list                   46
        .word   d$cbridge               # (0x51A) Get port list                     47
        .word   d$cbridge               # (0x51B) Get target resource list          48
        .word   d$stopio                # (0x51C) Stop I/O                          49
        .word   d$startio               # (0x51D) Start I/O                         50
        .word   d$cbridge               # (0x51E) Set FE port event notification    51
        .word   d$forceerrortrap        # (0x51F) Force a FE error trap             52
        .word   d$cbridge               # (0x520) Perform FE Loop Primitive         53
        .word   d$cbridge               # (0x521) Get target information            54
        .word   d$cbridge               # (0x522) Fail / Unfail port                55
        .word   D$nop                   # (0x523) No op                             56
        .word   d$qfecc                 # (0x524) Query FE Controller Comm          57
        .word   d$qstopcomp             # (0x525) Query Stop Complete               58
        .word   d$qmpchange             # (0x526) Query Mirror Partner Change       59
        .word   d$cbridge               # (0x527) Get HBA Statistics                60
        .word   d$cbridge               # (0x528) Get MM Status                     61
        .word   d$cbridge               # (0x529) Get Mirror Partner Configuration  62
        .word   d$cbridge               # (0x52A) Put Regular Ports on FE Fabric    63
        .word   d$cbridge               # (0x52B) Set Temp Disable Cache            64
        .word   d$cbridge               # (0x52C) Clear Temp Disable Cache          65
        .word   d$cbridge               # (0x52D) Query WC Temp Disable Flush       66
        .word   d$cbridge               # (0x52E) Test driver for MM/NV Card        67
        .word   d$flerrwomp             # (0x52F) Flush WC due to Error w/o MP      68
        .word   d$cbridge               # (0x530) Get iSCSI Sessions Info           69
        .word   d$cbridge               # (0x531) Get iSCSI Sessions on a Server    70
        .word   d$cbridge               # (0x532) Get IDD Info                      71
        .word   d$cbridge               # (0x533) DLM Path stats                    72
        .word   d$cbridge               # (0x534) DLM Path algorithm selection      73
        .word   d$cbridge               # (0x535) Get persistent reserve data       74
        .word   d$cbridge               # (0x536) Clear persistent reserve data     75
        .word   d$cbridge               # (0x537) persistent reserve config complete 76
        .word   d$cbridge               # (0x538) persistent reserve config update  77
        .word   d$cbridge               # (0x539) Set port config                   78
#
# --- Ordered list of request lengths
#
d_exppktlen:
        .short  0xffff                  # (0x200) Configure server
        .short  mrxsiz                  # (0x201) Configure server complete
        .short  0xffff                  # (0x202) Configure cache
        .short  0xffff                  # (0x203) OBSOLETE....
        .short  0xffff                  # (0x204) OBSOLETE....
        .short  mxcsiz                  # (0x205) Invoke cache stop
        .short  mccsiz                  # (0x206) Invoke cache resume
        .short  miisiz                  # (0x207) Assign system information
        .short  mvusiz                  # (0x208) Virtual disk has changed
        .short  msusiz                  # (0x209) Server has changed
        .short  mctsiz                  # (0x20A) Configure a target
        .short  mresiz                  # (0x20B) Reset configuration
        .short  mcssiz                  # (0x20C) Set controller serial number
        .short  mmmsiz                  # (0x20D) Send MM_INFO
        .short  mcosiz                  # (0x20E) Set configurable options BE => FE
        .short  0xffff                  # (0x20F) Configure update for target
        .short  0xffff                  # (0x210) Get Port Type
        .short  0xffff                  # (0x211) Configure CHAP user
        .short  0xffff                  # (0x212) Set iSNS information
        .short  0xffff                  # (0x213) Config Persistent Reserve
        .short  msftsiz                 # (0x214) Set Foreign Target (on/off)
#
        .short  mflsiz                  # (0x500) Get loop statistics
        .short  missiz                  # (0x501) Get server information
        .short  mgcsiz                  # (0x502) Get cache statistics
        .short  mfpsiz                  # (0x503) Get link statistics
        .short  mgisiz                  # (0x504) Get II statistics
        .short  mcdsiz                  # (0x505) Get cache device statistics
        .short  mgssiz                  # (0x506) Get server statistics
        .short  mbhsiz                  # (0x507) Set battery health
        .short  mrcisiz                 # (0x508) Resume cache initialization
        .short  mfhsiz                  # (0x509) Get frontend boot code header
        .short  mfhsiz                  # (0x50A) Get frontend diag code header
        .short  mfhsiz                  # (0x50B) Get frontend proc code header
        .short  0                       # (0x50C) No op
        .short  mrwsiz                  # (0x50D) Read/write memory
        .short  mflsiz                  # (0x50E) Reset FE chip
        .short  mslsiz                  # (0x50F) Server lookup based on WWN/TID
        .short  0xffff                  # (0x510) Generic
        .short  mpconfigfesiz           # (0x511) set mirror partner config
        .short  0xffff                  # (0x512) FE Fibre Heartbeat List - DLM
        .short  0                       # (0x513) Continue Cache Init w/o MP
        .short  0                       # (0x514) Flush FE Cache w/o MP
        .short  0xffff                  # (0x515) Invalidate the FE Write Cache
        .short  0xffff                  # (0x516) Flush the BE Write Cache
        .short  0xffff                  # (0x517) Invalidate the BE Write Cache
        .short  mmpsiz                  # (0x518) Mode page
        .short  mflsiz                  # (0x519) Get device list
        .short  mgxsiz                  # (0x51A) Get port list
        .short  mtgrsiz                 # (0x51B) Get target resource list
        .short  mxisiz                  # (0x51C) Stop I/O
        .short  mrisiz                  # (0x51D) Start I/O
        .short  mpnsiz                  # (0x51E) Set FE port event notification
        .short  metsiz                  # (0x51F) Force a FE error trap
        .short  mlpsiz                  # (0x520) Perform FE Loop Primitive
        .short  mgtsiz                  # (0x521) Get target information
        .short  mposiz                  # (0x522) Fail / Unfail port
        .short  0                       # (0x523) No op
        .short  mqcsiz                  # (0x524) Query FE Controller Comm
        .short  0                       # (0x525) Query Stop Complete
        .short  mqmpcsiz                # (0x526) Query Mirror Partner Change
        .short  mghssiz                 # (0x527) Get HBA Statistics
        .short  4                       # (0x528) Get MM Status
        .short  0                       # (0x529) Get Mirror Partner Configuration
        .short  0                       # (0x52A) Put Regular Ports on FE Fabric
        .short  mstdcsiz                # (0x52B) Set Temp Disable Cache
        .short  mctdcsiz                # (0x52C) Clear Temp Disable Cache
        .short  0                       # (0x52D) Query WC Temp Disable Flush
        .short  mrmmtestsiz             # (0x52E) Test driver for MM/NV Card
        .short  0                       # (0x52F) Flush WC due to Error w/o MP
        .short  0xffff                  # (0x530) Get iSCSI Sessions Info
        .short  0xffff                  # (0x531) Get iSCSI Sessions on a Server
        .short  0xffff                  # (0x532) Get IDD Info
        .short  0xffff                  # (0x533) DLM Path stats
        .short  0xffff                  # (0x534) DLM Path algorithm selection
        .short  0xffff                  # (0x535) Get persistent reserve data
        .short  0xffff                  # (0x536) Clear persistent reserve data
        .short  0xffff                  # (0x537) persistent reserve config complete
        .short  0xffff                  # (0x538) persistent reserve config update
        .short  0xffff                  # (0x539) Set port config
#
# --- Ordered list of return lengths
#
d_expretlen:
        .short  0xffff                  # (0x200) Configure server
        .short  mrxrsiz                 # (0x201) Configure server complete
        .short  mrcrsiz                 # (0x202) Configure cache
        .short  0xffff                  # (0x203) OBSOLETE....
        .short  0xffff                  # (0x204) OBSOLETE....
        .short  mxcrsiz                 # (0x205) Invoke cache stop
        .short  mccrsiz                 # (0x206) Invoke cache resume
        .short  miirsiz                 # (0x207) Assign system information
        .short  mvursiz                 # (0x208) Virtual disk has changed
        .short  msursiz                 # (0x209) Server has changed
        .short  mctrsiz                 # (0x20A) Configure a target
        .short  mrersiz                 # (0x20B) Reset configuration
        .short  mcsrsiz                 # (0x20C) Set controller serial number
        .short  mmmrsiz                 # (0x20D) Send MM_INFO
        .short  mscorsiz                # (0x20E) Set configurable options BE => FE
        .short  0xffff                  # (0x20F) Configure update for target
        .short  0xffff                  # (0x210) Get Port Type
        .short  0xffff                  # (0x211) Configure CHAP user
        .short  0xffff                  # (0x212) Set iSNS information
        .short  0xffff                  # (0x213) Config Persistent Reserve
        .short  msftrsiz                # (0x214) Set Foreign Target (on/off)
#
        .short  0xffff                  # (0x500) Get loop statistics
        .short  0xffff                  # (0x501) Get server information
        .short  mgcrsiz                 # (0x502) Get cache statistics
        .short  mfprsiz                 # (0x503) Get link statistics
        .short  mgirsiz                 # (0x504) Get II statistics
        .short  mcdrsiz                 # (0x505) Get cache device statistics
        .short  mgsrsiz                 # (0x506) Get server statistics
        .short  mbhrsiz                 # (0x507) Set battery health
        .short  mrcirsiz                # (0x508) Resume cache initialization
        .short  mfhrsiz                 # (0x509) Get frontend boot code header
        .short  mfhrsiz                 # (0x50A) Get frontend diag code header
        .short  mfhrsiz                 # (0x50B) Get frontend proc code header
        .short  mrrsiz                  # (0x50C) No op
        .short  mrwrsiz                 # (0x50D) Read/write memory
        .short  mrrsiz                  # (0x50E) Reset FE chip
        .short  mslrsiz                 # (0x50F) Server lookup based on WWN/TID
        .short  0xffff                  # (0x510) Generic
        .short  mamprsiz                # (0x511) Assign a Mirror Partner
# Response packet size is same for 'setmpconfigfe' and 'assignmp'
        .short  mfhlrsiz                # (0x512) FE Fibre Heartbeat List - DLM
        .short  mrrsiz                  # (0x513) Continue Cache Init w/o MP
        .short  mrrsiz                  # (0x514) Flush FE Cache w/o MP
        .short  mifrsiz                 # (0x515) Invalidate the FE Write Cache
        .short  mfbrsiz                 # (0x516) Flush the BE Write Cache
        .short  mibrsiz                 # (0x517) Invalidate the BE Write Cache
        .short  mmprsiz                 # (0x518) Mode page
        .short  0xffff                  # (0x519) Get device list
        .short  0xffff                  # (0x51A) Get port list
        .short  0xffff                  # (0x51B) Get target resource list
        .short  mxirsiz                 # (0x51C) Stop I/O
        .short  mrirsiz                 # (0x51D) Start I/O
        .short  mrrsiz                  # (0x51E) Set FE port event notification
        .short  metrsiz                 # (0x51F) Force a FE error trap
        .short  mlprsiz                 # (0x520) Perform FE Loop Primitive
        .short  mgtrsiz                 # (0x521) Get target information
        .short  mrrsiz                  # (0x522) Fail / Unfail port
        .short  mrrsiz                  # (0x523) No op
        .short  mqcrsiz                 # (0x524) Query FE Controller Comm
        .short  mqscrsiz                # (0x525) Query Stop Complete
        .short  mqmpcrsiz               # (0x526) Query Mirror Partner Change
        .short  0xffff                  # (0x527) Get HBA Statistics
        .short  0x40                    # (0x528) Get MM Status
        .short  0x38                    # (0x529) Get Mirror Partner
                                        # Configuration : response packet size
                                        # is = rsp header size(8bytes)+mpconfig
                                        # size(48bytes)
        .short  mrrsiz                  # (0x52A) Put Regular Ports on FE Fabric
        .short  mstdcrsiz               # (0x52B) Set Temp Disable Cache
        .short  mctdcrsiz               # (0x52C) Clear Temp Disable Cache
        .short  mqtdcrsiz               # (0x52D) Query WC Temp Disable Flush
        .short  mrmmtestrsiz            # (0x52E) Test driver for MM/NV Card
        .short  mrrsiz                  # (0x52F) Flush WC due to Error w/o MP
        .short  0xffff                  # (0x530) Get iSCSI Sessions Info
        .short  0xffff                  # (0x531) Get iSCSI Sessions on a Server
        .short  0xffff                  # (0x532) Get IDD Info
        .short  0xffff                  # (0x533) DLM Path stats
        .short  0xffff                  # (0x534) DLM Path algorithm selection
        .short  0xffff                  # (0x535) Get persistent reserve data
        .short  0xffff                  # (0x536) Clear persistent reserve data
        .short  mrrsiz                  # (0x537) persistent reserve config complete
        .short  mrrsiz                  # (0x538) persistent reserve config update
        .short  mrrsiz                  # (0x539) Set port config
#
# --- Executive QCB
#
        .section    .shmem
        .align  2
        .globl  d_exec_qu               # Easier for debugging
d_exec_qu:
        .word   0                       # Queue head
        .word   0                       # Queue tail
        .word   0                       # Queue count
        .word   0                       # Associated PCB
#
        .data
        .align  2
d_nvram_csum:
        .word   0                       # Stores the NMI NVRAM Checksum value
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
#  CALLING SEQUENCE:
#       call    D$init
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
# --- Clear server index table
#
        lda     S_sddindx,r4            # Get SDX
c       memset((char *)&S_sddindx + sx_sdd, 0, MAXSERVERS*sizeof(UINT32));
        stos    0,sx_ecnt(r4)           # Zero the count
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
c       M_addDDRentry(de_dpcb, g0, pcbsiz);  # Size of d_exec_qu PCB
#
# --- Initialize Debug Data Retrieval (DDR) d_exec_qu entry
#
        lda     d_exec_qu,g1            # Load address of d_exec_qu header
c       M_addDDRentry(de_deque, g1, 16);  # Size of d_exec_qu header
#
# --- Initialize the MRP trace Debug Data Retrieval (DDR) table entry
#
        lda     defTraceQue,g1          # Load address to MRP trace
c       M_addDDRentry(de_mrp, g1, 16400);  # Size of MRP trace
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
#       and activates this executive if necessary.  This executive
#       extracts the next MRP request from the queue and initiates that
#       request.
#
#       The functions called all are provided with the pointer to the MRP.
#
#       The functions are expected to return a length for data returned
#       and the status of the operation.
#
#       The following registers are used for the purposes described above.
#
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
#  REGS DESTROYED:
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
#
.dex20:
        call    K$qxchang               # Exchange processes
#
# --- Get next queued request
#
        lda     d_exec_qu,r11           # Get executive queue pointer
        ldq     qu_head(r11),r12        # Get queue head, tail, count and PCB
        mov     r12,g14                 # Isolate next queued ILT
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
# --- Prep return packet size
#
        ldconst mrrsiz,g2               # Return packet size
#
# --- Determine type of request
#
        ldconst deinvpkttyp,g1          # Set invalid function type
#
        ld      il_w0-ILTBIAS(g14),g0   # Get request ptr
        ldos    mr_func(g0),r7          # Get request function type
#
# --- Validate the opcode.
#
        ldconst mrbffuncmax,r3          # Get maximum opcode
        cmpoble r7,r3,.dex40            # If less, then check in low range
#
        ldconst mreffuncmax,r3          # Check for CCB to FEP commands
        cmpobg  r7,r3,.dex100           # Jif out of range
        ldconst mreffuncbase,r3         # Check for low end
        cmpobl  r7,r3,.dex100           # Jif out of range
#
# --- Good command, now adjust so that we look into the tables immediately
# --- following the CCB to BE opcode range.
#
        subo    r3,r7,r7                # Get zero relative to this op range
        ldconst mrbffuncmax-mrbffuncbase+1,r3
        addo    r3,r7,r3                # We are now at the end of other range
        b       .dex50
#
.dex40:
        ldconst mrbffuncbase,r3         # Get minimum opcode
        cmpobl  r7,r3,.dex100           # If less than the min, illegal
        subo    r3,r7,r3                # Get table offset
#
# --- Validate the length of the packet and the return length
#
.dex50:
        ldos    d_exppktlen[r3*2],r4    # Get the expected length
        ldconst 0xffff,r7               # Max length indicator
        cmpobe  r7,r4,.dex55            # If max, called function will validate
#
        ldconst deinvpktsiz,g1          # Prep possible error code
        ld      mr_len(g0),r7           # Get the length
        cmpobne r4,r7,.dex100           # Exit w/ error if not equal
#
.dex55:
        ldos    d_expretlen[r3*2],r4    # Get the expected return data length
        ldconst 0xffff,r7               # Max length indicator
        cmpobe  r7,r4,.dex90            # If max, called function will validate
#
        ldconst deretlenbad,g1          # Prep possible error code
        ld      mr_ralloclen(g0),r7     # Get the return allocation length
        cmpobe  r4,r7,.dex90            # If equal, continue
#
        ldconst mrbffuncmax-mrbffuncbase+1,r4
        cmpobg  r3,r4,.dex100           # If from the CCB, return error
        faultle                         # Else fault
#
# --- Execute action routine
#
.dex90:
#TODO check for PR command
        ldconst mrprget,r7
        cmpobne r7,r3,.dex91
c fprintf(stderr,"%s%s:%u DefFE got PR Get mrp.\n", FEBEMESSAGE, __FILE__, __LINE__);
.dex91:
        ld      d_pktact[r3*4],r4       # Get function
        mov     g0,r15                  # Save g0
        mov     g14,r14                 # Save g14
#
        ldconst trMRPStart,g0           # Trace start of MRP
        mov     r3,g1
        call    D$TraceEvent
#
        mov     r15,g0                  # Restore g0, but retain it in r15
        callx   (r4)
#
        ldconst trMRPStop,g0            # Trace end of MRP
        mov     g1,r4                   # Save g1
        shlo    8,g1,g1                 # Shift return status code 1 byte
        or      r3,g1,g1                # OR it to the MRP command number
        call    D$TraceEvent
        mov     r4,g1                   # Restore g1
#
        mov     r14,g14                 # Restore g14
        mov     r15,g0                  # Restore g0
#
# --- At this point, we check to see if the MRP that was performed is an MRP
# --- which will handle its own completion call to the link layer.  These
# --- MRPs are ones that are non-deterministic in time and may block the
# --- define executive from completing other tasks (read - stopio).
#
        ldconst mrstopio-mreffuncbase+mrbffuncmax-mrbffuncbase+1,r4
        cmpobe  r3,r4,.dex20            # Jif rescan
#
# --- Send status response
#
.dex100:
        ld      mr_rptr(g0),r3          # Get the return data ptr
!       stob    g1,mr_status(r3)        # Plug return status code
!       st      g2,mr_rlen(r3)          # Set return packet size
#
        mov     g14,g1                  # Complete this request
        call    K$comp
        b       .dex20
#
#**********************************************************************
#
#  NAME: d$confservercmplt
#
#  PURPOSE:
#       To provide a standard means of informing the front end processor
#       that configuration of servers is complete when the configuration
#       is coming from a restore of NVRAM.
#
#  DESCRIPTION:
#       This function will notify any interested parties that there is no
#       more configuration data for servers expected.
#
#  CALLING SEQUENCE:
#       call d$confservercmplt
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
d$confservercmplt:
#
# --- Tell MAG driver to reconfigure.
#
        call    MAGD$serverchange       # Process the change
#
# --- Indicate server configuration is complete.
#
        lda     K_ii,r3                 # Get ptr to Internal Information
        ldos    ii_status(r3),r4        # Get initialization status
        setbit  iiserver,r4,r4          # Set servers defined bit
        stos    r4,ii_status(r3)        # Save it
#
        ldconst deok,g1                 # Everything is fine
        ldconst mrxrsiz,g2              # Return length
        ret
#
#**********************************************************************
#
#  NAME: d$confcache
#
#  PURPOSE:
#       To provide a standard method to process the configure cache
#       MRP.
#
#  DESCRIPTION:
#       This module will be called to configure the cache on the front
#       end processor in a number of different scenarios.  First, it
#       will be used to configure from NVRAM where a large number of
#       reports are expected.  This may involve more than one MRP with
#       subsequent MRPs being the "continue" type.  In this case, the
#       caching will be stopped on the first MRP and will require that
#       a cache config complete MRP be sent to resume caching.  The
#       second scenario is a simple update of a cache record.  This
#       will either be for a new VDisk creation of a cache control MRP
#       received on the back end processor.  In either case, the
#       caching will be stopped and resumed within this module.
#
#       In all cases, the VCD will be allocated if none exist at the
#       time.
#
#  CALLING SEQUENCE:
#       call d$confcache
#
#  INPUT:
#       g0 - MRP
#
#  OUTPUT:
#       g1 - error code
#       g2 - length of return packet
#
#  REGS DESTROYED:
#       g0
#
#**********************************************************************
#
d$confcache:
#
# --- First, grab the return data address and the parm block address.
#
        ld      mr_rptr(g0),r15         # Return data pointer
        ld      mr_ptr(g0),r14          # Parm block address
#
# --- Validate the length and parms.
#
        ldob    mrc_op(r14),r13         # Get the opcode
#
        ldconst deinvpktsiz,g1          # Prep error code
        ldos    mrc_nvid(r14),r12       # Get number of VIDs to process
        mulo    mrcxsiz,r12,r3          # Calculate total size of extensions
        addo    mrcsiz,r3,r3            # Add in base record size
        ld      mr_len(g0),r4           # Get input length
        cmpobne r3,r4,.rc1000           # Jif mismatch
#
# --- Parse the opcodes
#
        lda     vcdIndex,r11            # Get the index table address
#
        cmpobe  mrcglobal,r13,.rc20     # Jif not global change
        cmpobe  mrcmultistart,r13,.rc100# Jif multiple start
        cmpobe  mrccontinue,r13,.rc110  # If continue, go load them
        cmpobe  mrcmultidone,r13,.rc200 # If multiple done, delete left overs
        cmpobe  mrcsingle,r13,.rc300    # If single, add or change single
        cmpobe  mrcdelete,r13,.rc400    # If delete, delete the single
#
        ldconst deinvop,g1              # Prep error code
        b       .rc1000                 # Jif invalid opcode
#
# --- Process global on/off and ignore VID mappings.
#
.rc20:
        ldconst 0xffffffff,g0           # Set to affect all
#
        ldob    mrc_genable(r14),r3     # Get global enable
        cmpobe  0,r3,.rc30              # Jif turn caching off
#
        call    C$enable                # Enable
        b       .rc900                  # Exit
#
.rc30:
        call    C$disable               # Turn off caching
        b       .rc900                  # Exit
#
# --- This is a clear or a continue, so we have already stopped the cache
# --- and will now replace vdisk information based upon the records passed
# --- over.  The records which are in the VCD table but are not in the list
# --- we received need to be deleted and the ones that are in the list either
# --- need updating or creation.
#
.rc100:
        ldconst MAXVIRTUALS-1,r10       # Index at max (first of multiple)
        lda     mrc_vidmap(r14),r9      # VID -> cache enable entries
        cmpobe  0,r12,.rc900            # Jif empty list
        b       .rc120                  # Start deletion, additions, or updates
#
.rc110:
        lda     mrc_vidmap(r14),r9      # VID -> cache enable entries
        ldos    mrc_vid(r9),r10         # Starting VID at last one done
        lda     mrcxsiz(r9),r9          # Start at next entry since prev one done
        subo    1,r10,r10               # Start at one less than the last one done
        subo    1,r12,r12               # Decrement counter
        cmpobe  0,r12,.rc900            # Jif empty list other than delete start
#
# --- At this point, we have the index into the table (r10) and we have the
# --- the address of the next VID to process (r9).  We must delete any VIDs
# --- between r10 (inclusive) and the VID referred to by the record pointed to
# --- by r9.
#
.rc120:
        ldos    mrc_vid(r9),r7          # Get next VID to update
#
.rc130:
        cmpobe  r7,r10,.rc150           # On the one to update, stop deletions
#
        ld      vx_vcd(r11)[r10*4],g0   # Get VCD
        cmpobe  0,g0,.rc140             # Jif undefined
#
        mov     g0,r3
        setbit  mxcnoflush,0,g0         # Do not flush the write cache and
        setbit  mxcnobackground,g0,g0   #  do not allow background flushes
        ldconst mxcdefinefe,g1          # g1 = FE Define User ID
        call    C$stop                  # Stop caching - ignore and delete data
#
        mov     r10,g0                  # g0 = VID
        call    CA_RemoveVIDRetryQueue  # Return any Ops on the RETRY queue as
                                        #  a non-existent drive
#
        mov     r10,g0                  # VID
        ldconst 0,g1                    # Delete all data
        call    WC$deleteVIDdata        # Delete the cache data
#
        ldconst mccclearone,g0          # g0 = Clear only one stop
        ldconst mccdefinefe,g1          # g1 = FE Define User ID
        call    C$resume                # Resume caching
        mov     r3,g0
#
        call    d$freevcd               # Free the VCD
#
        ldconst 0,r3
        st      r3,vx_vcd(r11)[r10*4]   # Zero out the VCD pointer
#
        ldos    vx_ecnt(r11),r3         # Get the count of VCDs
        subo    1,r3,r3                 # Decrement it
        stos    r3,vx_ecnt(r11)         # Save it
#
.rc140:
        subo    1,r10,r10               # Dec index
        cmpible 0,r10,.rc130            # Jif more to check
#
# --- Now that the deletions are done, we are either updating an existing
# --- VCD or creating a new one.  If we are updating one, we need to flush
# --- cache if we changed the state of the cache enable.
#
.rc150:
        ld      vx_vcd(r11)[r7*4],g0    # Get VCD
        cmpobne 0,g0,.rc160             # Jif already defined
#
        call    d$allocvcd              # Allocate one
        st      g0,vx_vcd(r11)[r7*4]    # Save it
        stos    r7,vc_vid(g0)           # Save the VID
#
        ldos    vx_ecnt(r11),r3         # Get count
        addo    1,r3,r3                 # Bump it
        stos    r3,vx_ecnt(r11)         # Save it
#
.rc160:
        ldconst 0,r4                    # r4 = New Cache State
        ldconst 0,r5                    # r5 = New Features State
        ldob    mrc_features(r9),r3     # r3 = VID Features
        bbc     mrc_copy_dest,r3,.rc165 # Jif VDisk is not a Copy/Mirror Dest.
        setbit  vc_copy_dest_ip,r5,r5   # Show the VCD as Copy/Mirror Dest.
        ldob    vc_stat(g0),r6          # Get the current cache state
        bbc     vc_cached,r6,.rc170     # Jif Cache is not enabled and do not
                                        #  allow it to be enabled.
#
#   The VID is being cached, turn off caching while the VID is the
#       destination of a Copy/Mirror.  Since the VDisk is going to be rewritten
#       anyway, just delete any data in cache that may exist
#
        ld      vc_cache(g0),r6         # Get the Valid Data RB Root
        cmpobe  0,r6,.rc170             # Jif no data in cache
#
        mov     g0,r6                   # Save the VCD pointer
        setbit  mxcnoflush,0,g0         # Do not flush the write cache and
        setbit  mxcnobackground,g0,g0   #  do not allow background flushes
        ldconst mxcdefinefe,g1          # g1 = FE Define User ID
        call    C$stop                  # Stop caching - ignore and delete data
#
        mov     r7,g0                   # VID
        ldconst 0,g1                    # Delete all data
        call    WC$deleteVIDdata        # Delete the cache data
#
        ldconst mccclearone,g0          # g0 = Clear only one stop
        ldconst mccdefinefe,g1          # g1 = FE Define User ID
        call    C$resume                # Resume caching
        mov     r6,g0                   # Restore VCD pointer
        b       .rc170                  # Do not allow cache to be enabled
                                        #  when the destination of a Copy/Mirror
#
.rc165:
        bbc     mrc_cache_enable,r3,.rc170 # Jif cache is disabled
        setbit  vc_cached,r4,r4         # Set the Cache Enabled flag
#
.rc170:
        bbc     mrc_mirror_write_info,r3,.rc175 # Jif not Mirroring Write Info
        setbit  vc_mirror_write_info,r5,r5 # set the Mirror Write Info flag
#
.rc175:
        bbc     mrc_rebuild_check_needed,r3,.rc180 # Jif no Rebuild Chk needed
        setbit  vc_rebuild_required,r5,r5 # Set the Rebuild Check Required bit
#
# --- At this point, we have the old and new caching settings.  If caching was
# --- off and we are turning it on, then do so.  If it was on and we are
# --- turning it off, do so and flush the cache.
#
.rc180:
        ldob    vc_stat(g0),r3          # Get the previous cache state
        bbs     vc_cached,r3,.rc185     # Was off, check for on
#
# --- Off to off or off to on transitions.
#
        bbc     vc_cached,r4,.rc190     # Still off, do nothing
#
        mov     g0,r3                   # Save the VCD
        mov     r7,g0                   # Set the VID
        call    C$enable                # Enable caching for this VCD
        mov     r3,g0                   # Restore the VCD
        b       .rc190
#
# --- On to on or on to off transitions.
.rc185:
        bbs     vc_cached,r4,.rc190     # Still on, do nothing
        mov     g0,r3                   # Save the VCD
        mov     r7,g0                   # Set the VID
        call    C$disable               # Disable caching for this VCD
        mov     r3,g0                   # Restore the VCD
#
.rc190:
        ldob    vc_stat(g0),r4          # Get the current value
        clrbit  vc_mirror_write_info,r4,r4 # Clear the Mirror Write Info, Copy
        clrbit  vc_copy_dest_ip,r4,r4   #  Destination, and Rebuild Check flags
        clrbit  vc_rebuild_required,r4,r4 # and use the new set of flags
                                        #  just passed in
        or      r5,r4,r5                # Set or clear the new bits
        stob    r5,vc_stat(g0)          # Save VCD status (VID features)
#
        ldl     mrc_capacity(r9),r4     # Copy capacity to VCD
        stl     r4,vc_capacity(g0)
#
        subo    1,r7,r10                # Move deletion pointer
#
        lda     mrcxsiz(r9),r9          # Point to next entry
        subo    1,r12,r12               # Decrement the count of entries
        cmpobne 0,r12,.rc120            # Jif not done
#
        b       .rc900                  # Else exit
#
# --- This is the end of the multiple.  All we need to do is clear out any
# --- VCDs from the VID passed through zero and then complete the setup.
#
.rc200:
        lda     mrc_vidmap(r14),r9      # VID -> cache enable entries
        ldos    mrc_vid(r9),r10         # Starting VID at last one done
        b       .rc240                  # Start at one less than the last one done
#
.rc210:
        ld      vx_vcd(r11)[r10*4],g0   # Get VCD
        cmpobe  0,g0,.rc240             # Jif undefined
#
        mov     g0,r3
        setbit  mxcnoflush,0,g0         # Do not flush the write cache and
        setbit  mxcnobackground,g0,g0   #  do not allow background flushes
        ldconst mxcdefinefe,g1          # g1 = FE Define User ID
        call    C$stop                  # Stop caching - ignore and delete data
#
        mov     r10,g0                  # g0 = VID
        call    CA_RemoveVIDRetryQueue  # Return any Ops on the RETRY queue as
                                        #  a non-existent drive
#
        mov     r10,g0                  # VID
        ldconst 0,g1                    # Delete all data
        call    WC$deleteVIDdata        # Delete the cache data
#
        ldconst mccclearone,g0          # g0 = Clear only one stop
        ldconst mccdefinefe,g1          # g1 = FE Define User ID
        call    C$resume                # Resume caching
        mov     r3,g0
#
        call    d$freevcd               # Free the VCD
#
        ldconst 0,r3
        st      r3,vx_vcd(r11)[r10*4]   # Zero out the VCD pointer
#
        ldos    vx_ecnt(r11),r3         # Get the count of VCDs
        subo    1,r3,r3                 # Decrement it
        stos    r3,vx_ecnt(r11)         # Save it
#
.rc240:
        subo    1,r10,r10               # Dec index
        cmpible 0,r10,.rc210            # Jif more to check
#
# --- Complete the update.
#
        call    d$confcachecmplt        # Complete it
        b       .rc900
#
# --- This is a single.  We are either adding a new entry or changing
# --- the caching properties of a single entry.
#
.rc300:
        ldos    mrc_vid+mrc_vidmap(r14),r3
        ldob    mrc_features+mrc_vidmap(r14),r4
#
        ld      vx_vcd(r11)[r3*4],g0    # Get VCD
        cmpobne 0,g0,.rc310             # If already exists, continue
#
        call    d$allocvcd              # Allocate one
        st      g0,vx_vcd(r11)[r3*4]    # Save it
        stos    r3,vc_vid(g0)           # Save the VID
#
        ldos    vx_ecnt(r11),r6         # Get count
        addo    1,r6,r6                 # Bump it
        stos    r6,vx_ecnt(r11)         # Save it
#
.rc310:
        ldl     mrc_capacity+mrc_vidmap(r14),r6 # Copy capacity to VCD
        stl     r6,vc_capacity(g0)
#
        ldconst 0,r5                    # r5 = New Features of VDisk
        bbc     mrc_copy_dest,r4,.rc320 # Jif VDisk is not a Copy/Mirror Dest.
        setbit  vc_copy_dest_ip,r5,r5   # Show the VCD as Copy/Mirror Dest.
        clrbit  mrc_cache_enable,r4,r4  #  and turn off caching (cannot have
                                        #  both)
        ldob    vc_stat(g0),r6          # Get the current cache state
        bbc     vc_cached,r6,.rc320     # Jif Cache is not enabled and do not
                                        #  allow it to be enabled.
#
#   The VID is being cached, turn off caching while the VID is the
#       destination of a Copy/Mirror.  Since the VDisk is going to be rewritten
#       anyway, just delete any data in cache that may exist
#
        ld      vc_cache(g0),r6         # Get the Valid Data RB Root
        cmpobe  0,r6,.rc320             # Jif no data in cache
#
        mov     g0,r6                   # Save the VCD pointer
        setbit  mxcnoflush,0,g0         # Do not flush the write cache and
        setbit  mxcnobackground,g0,g0   #  do not allow background flushes
        ldconst mxcdefinefe,g1          # g1 = FE Define User ID
        call    C$stop                  # Stop caching - ignore and delete data
#
        mov     r3,g0                   # VID
        ldconst 0,g1                    # Delete all data
        call    WC$deleteVIDdata        # Delete the cache data
#
        ldconst mccclearone,g0          # g0 = Clear only one stop
        ldconst mccdefinefe,g1          # g1 = FE Define User ID
        call    C$resume                # Resume caching
        mov     r6,g0                   # Restore VCD pointer
.rc320:
        bbc     mrc_mirror_write_info,r4,.rc330 # Jif not Mirroring Write Info
        setbit  vc_mirror_write_info,r5,r5 # Turn on Mirroring of Write Info
.rc330:
        bbc     mrc_rebuild_check_needed,r4,.rc340 # Jif not Rebuild Check
        setbit  vc_rebuild_required,r5,r5 # Turn on Rebuild Check Needed bit
.rc340:
        ldob    vc_stat(g0),r6          # r6 = VCD Status
        clrbit  vc_mirror_write_info,r6,r6 # Clear the Mirror Write Info, Copy
        clrbit  vc_copy_dest_ip,r6,r6   #  Destination, and Rebuild Check flags
        clrbit  vc_rebuild_required,r6,r6 #  and use the new set of flags
                                        #  just passed in
        or      r5,r6,r5                # Set or clear the VID Features
        stob    r5,vc_stat(g0)          # Save VCD status (VID features)
#
        mov     r3,g0                   # Load the VID
#
        bbc     mrc_cache_enable,r4,.rc350 # Jif caching is not being enabled
#
        call    C$enable                # Enable caching
        b       .rc900
#
.rc350:
        call    C$disable               # Disable caching
        b       .rc900
#
# --- This is a deletion.  First, stop the caching on this device.
# --- After stopping the caching on the device, delete any data that may
# --- be stuck in cache for the device, and then delete it.
#
.rc400:
        ldos    mrc_vid+mrc_vidmap(r14),r3
#
        ld      vx_vcd(r11)[r3*4],g0    # Get VCD
        cmpobe  0,g0,.rc900             # If already gone, quit
        mov     g0,r9                   # Save the VCD
#
        mov     r3,g0                   # g0 = VID
        call    C$disable               # Disable caching on this device
#
        mov     r3,g0                   # g0 = VID
        call    CA_RemoveVIDRetryQueue  # Return any Ops on the RETRY queue as
                                        #  a non-existent drive
#
        ldconst 0,g1                    # g1 = Del all tags no matter the state
        call    WC$deleteVIDdata        # Delete any data stuck in cache
#
        mov     r9,g0                   # g0 = VCD to delete
        call    d$freevcd               # Free the VCD
        ldconst 0,r9                    # Get a zero
        st      r9,vx_vcd(r11)[r3*4]    # Zero out the VCD pointer
#
        ldos    vx_ecnt(r11),r6         # Get count
        subo    1,r6,r6                 # Decrement it
        stos    r6,vx_ecnt(r11)         # Save it
#
.rc900:
        ldconst deok,g1
#
.rc1000:
        ldconst mrcrsiz,g2              # Prep return code
        ret
#
#**********************************************************************
#
#  NAME: d$confcachecmplt
#
#  CALLING SEQUENCE:
#       call d$confcachecmplt
#
#  INPUT:
#       None
#
#  OUTPUT:
#       None
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
d$confcachecmplt:
        call    MAGD$vdiskchange        # Process the change
#
# --- Indicate VID configuration is complete.
#
        lda     K_ii,r3                 # Get ptr to Internal Information
        ldos    ii_status(r3),r4        # Get initialization status
        setbit  iivdd,r4,r4             # Set Virtual devices defined bit
        stos    r4,ii_status(r3)        # Save it
        ret
#
#**********************************************************************
#
#  NAME: d$allocvcd
#
#  PURPOSE:
#       To provide a standard means of allocating the
#       memory for a virtual cache descriptor (VCD).
#
#  DESCRIPTION:
#       This function will allocate fixed memory for a VCD record
#       and will clear out the memory in anticipation of the caller
#       filling in pertinent fields.
#
#  CALLING SEQUENCE:
#       call d$allocvcd
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       g0 - address of the VCD.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
d$allocvcd:
c       g0 = s_MallocC(vcdsiz|BIT31, __FILE__, __LINE__); # Assign memory for VCD.
        ret
#
#**********************************************************************
#
#  NAME: d$freevcd
#
#  PURPOSE:
#       To provide a standard means of deallocating the
#       memory for a virtual cache descriptor (VCD).
#
#  DESCRIPTION:
#       This function will deallocate memory for a VCD record.
#
#  CALLING SEQUENCE:
#       call d$freevcd
#
#  INPUT:
#       g0 - address of the VCD.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
d$freevcd:
c       s_Free(g0, vcdsiz, __FILE__, __LINE__);
        ret
#
#**********************************************************************
#
#  NAME: d$cachestop
#
#  PURPOSE:
#       To provide a standard means of stopping the caching on the
#       front end processor.
#
#  DESCRIPTION:
#       This function will halt I/O on the front end processor.
#
#  CALLING SEQUENCE:
#       call d$cachestop
#
#  INPUT:
#       g0 - MRP
#
#  OUTPUT:
#       g1 - error code
#       g2 - length of return packet
#
#  REGS DESTROYED:
#       g0-g2
#
#**********************************************************************
#
d$cachestop:
        ld      mr_ptr(g0),r3           # Get parm pointer
        ldob    mxc_op(r3),g0           # Set input to stop cache - options
        ldob    mxc_user(r3),g1         # Set input to stop cache - user ID
#
        call    C$stop                  # Stop the caching
        ldconst deoutops,r3             # Set up to show Outstanding Ops error
        cmpo    ecok,g0                 # Determine if Stop completed OK
        sele    r3,deok,g1              # Set return code
        ldconst mxcrsiz,g2              # Return length
        ret
#
#**********************************************************************
#
#  NAME: d$qstopcomp
#
#  PURPOSE:
#       To provide a standard means of determining if a Stop I/O has completed
#       successfully on the front end processor.
#
#  DESCRIPTION:
#       This function returns the number of Host I/O operations that are
#       still outstanding.
#
#  CALLING SEQUENCE:
#       call d$qstopcomp
#
#  INPUT:
#       g0 - MRP
#
#  OUTPUT:
#       g1 - error code
#       g2 - length of return packet
#
#  REGS DESTROYED:
#       g0-g2
#
#**********************************************************************
#
d$qstopcomp:
        ld      mr_rptr(g0),r3          # Get the return data ptr
        mov     g0,r15                  # Save g0
#
c       g0 = CA_CheckOpsOutstanding();  # Find how many Ops are outstanding?
#
        st      g0,mqscnum(r3)          # Save the count in the Response
        ldconst deok,g1                 # Show successful completion
        ldconst mqscrsiz,g2             # Return length
        mov     r15,g0                  # Restore g0
        ret
#
#**********************************************************************
#
#  NAME: d$cacheresume
#
#  PURPOSE:
#       To provide a standard means of resuming the caching on the
#       front end processor.
#
#  DESCRIPTION:
#       This function will resume I/O on the front end processor.
#
#  CALLING SEQUENCE:
#       call d$cacheresume
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
d$cacheresume:
        ld      mr_ptr(g0),r3           # Get parm block pointer
        mov     g0,r15                  # Save g0
        ldob    mcc_op(r3),g0           # g0 = Options to resume
        ldob    mcc_user(r3),g1         # g1 = User ID of stop counter to use
        call    C$resume                # Resume the caching
#
        mov     g0,g1                   # Set up completion code
        mov     r15,g0                  # Restore g0
        ldconst mccrsiz,g2              # Return length
        ret
#
#**********************************************************************
#
#  NAME: d$stopioTask
#
#  PURPOSE:  The function performs the stopio task and returns status via link.
#
#  DESCRIPTION: This task is start via the stopio call and runs to completion
#               before signalling completion via the link layer.
#
#  CALLING SEQUENCE:
#       fork    d$stopioTask
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
d$stopioTask:
        movl    g2,r14                  # Save g2 and g3
        ld      mr_ptr(g2),r3           # Get parm block pointer
#
        ldob    mxi_op(r3),g0           # Get options parm input to cache stop
        bbs     mxinowaithost,g0,.dst10 # Jif not waiting for outstanding
                                        #  Host I/O to complete
        bbs     mxinoflush,g0,.dst10    # Jif not flushing write cache
        ldob    mxi_intent(r3),r4       # Get the intent parm input
        bbc     mxishutdown,r4,.dst10   # Jif shutdown not requested
        call    C$setShutdownFlag
#
.dst10:
#
        ldob    mxi_user(r3),g1         # g1 = User ID Stop Counter
        call    C$stop                  # Stop the caching
        ldconst deoutops,r5             # Set up to show Outstanding Ops error
        cmpo    ecok,g0                 # Determine if Stop completed OK
        sele    r5,deok,g1              # Set return code
#
# --- We have finished either successfully or not.  Record the completion
# --- in the define trace log and then send the completion via the link
# --- layer.
#
        ldconst trMRPDefComp,g0         # Trace end of MRP
        mov     g1,r4                   # Save g1
        shlo    8,g1,g1                 # Shift return status code 1 byte
        ldconst mrstopio-mreffuncbase+mrbffuncmax-mrbffuncbase+1,r3
        or      r3,g1,g1                # OR it to the MRP command number
        call    D$TraceEvent
        mov     r4,g1                   # Restore g1
#
        ld      mr_rptr(r14),r3         # Get the return data ptr
!       stob    g1,mr_status(r3)        # Plug return status code
        ldconst mxirsiz,g2              # Set return packet size
!       st      g2,mr_rlen(r3)          # Set return packet size
#
        mov     r15,g1                  # Complete this request
        call    K$comp
        ret
#
#**********************************************************************
#
#  NAME: d$stopio
#
#  PURPOSE:
#       To provide a means of processing the stop io request
#       issued by the CCB.
#
#  CALLING SEQUENCE:
#       call    d$stopio
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       None
#
#  REGS DESTROYED:
#       g0-g2
#
#**********************************************************************
#
d$stopio:
        mov     g0,g2                   # g2 gets the MRP pointer
        mov     g14,g3                  # g3 gets the ILT
#
        lda     d$stopioTask,g0         # Task address
        ld      K_xpcb,r3
        ldob    pc_pri(r3),g1           # Priority inherited from parent
#
c       CT_fork_tmp = (ulong)"d$stopioTask";
        call    K$tfork                 # Fork it and leave
c       g1 = deok;                      # Set return code -- might be ok.
        ret
#
#**********************************************************************
#
#  NAME: d$startio
#
#  PURPOSE:
#       To provide a means of processing the start io request
#       issued by the CCB.
#
#  CALLING SEQUENCE:
#       call    d$startio
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
d$startio:
        ld      mr_ptr(g0),r3           # Get parm block pointer
        mov     g0,r15                  # Save g0
        ldob    mri_op(r3),g0           # g0 = Options to resume
        ldob    mri_user(r3),g1         # g1 = User ID of stop counter to use
        call    C$resume                # Resume the caching
        mov     g0,g1                   # Set up the return code
        mov     r15,g0                  # Restore g0
        ldconst mrirsiz,g2              # Set return packet size
        ret
#
#**********************************************************************
#
#  NAME: d$assignsysinfo
#
#  PURPOSE:
#       To provide a standard means of receiving system information
#       from the back end processor.
#
#  DESCRIPTION:
#       This function will set up system information based upon the
#       MRP received.
#
#  CALLING SEQUENCE:
#       call d$assignsysinfo
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
d$assignsysinfo:
        ld      mr_ptr(g0),r14          # Parm block address
#
        ldos    K_ii+ii_status,r6       # Get initialization status
#
# --- Load the information into the FICB
#
        ld      K_ficb,r3               # Get FICB
        ld      mii_vcgid(r14),r4       # Get virtual controller group
        st      r4,fi_vcgid(r3)         # Save it
        ld      mii_seq(r14),r4         # Get sequence number
        st      r4,fi_seq(r3)           # Save it
        ld      mii_ccbipaddr(r14),r4   # Get IP address
        st      r4,fi_ccbipaddr(r3)     # Save it
#
# --- The mirror partner should only be updated the first time
#     the information is setup.
#
        bbs     iimpfound,r6,.dasi10    # Jif Serial number not defined
        ld      mii_mp(r14),r4          # Get mirror partner serial number
        st      r4,fi_mirrorpartner(r3) # Save it
.dasi10:
        ldq     mii_vcgname(r14),r4     # Get VCG name
        stq     r4,fi_vcgname(r3)       # Save it
#
# --- Set FE controller serial number from BE controller serial number
#
        ld      mii_cserial(r14),r5     # Get controller serial number from MRP
        st      r5,fi_cserial(r3)       # Set controller serial number
#
# --- Indicate system information has been initialized and
#     whether CCB is required
#
        ldos    K_ii+ii_status,r6       # Get initialization status
        setbit  iisn,r6,r6              # Set Serial number defined bit
#
        clrbit  iiccbreq,r6,r6          # Set to only one controller present
        ldob    mii_ccbreq(r14),r3      # Get passed in number of controllers present bit
        cmpobe  FALSE,r3,.dasi20        # Jif only one controller present
        setbit  iiccbreq,r6,r6          # Set it
.dasi20:
        stos    r6,K_ii+ii_status       # Save it
#
        ldconst deok,g1                 # Everything is fine
        ldconst miirsiz,g2              # Return length
        ret
#
#**********************************************************************
#
#  NAME: d$vdiskchange
#
#  PURPOSE:
#       To provide a standard means of receiving updates from the BEP
#       regarding virtual disk config changes.
#
#  DESCRIPTION:
#       This function will update local structures based upon virtual disk
#       changes.
#
#  CALLING SEQUENCE:
#       call d$vdiskchange
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
d$vdiskchange:
        call    MAGD$vdiskchange        # Process the change
#
# --- Exit
#
        ldconst deok,g1                 # Everything is fine
        ldconst mvursiz,g2              # Return length
        ret
#
#**********************************************************************
#
#  NAME: d$serverchange
#
#  PURPOSE:
#       To provide a standard means of receiving updates from the BEP
#       regarding server config changes.
#
#  DESCRIPTION:
#       This function will update local structures based upon server
#       changes.
#
#  CALLING SEQUENCE:
#       call d$serverchange
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
d$serverchange:
        call    MAGD$serverchange       # Process the change
#
        ldconst deok,g1                 # Everything is fine
        ldconst msursiz,g2              # Return length
        ret
#
.if !ISCSI_CODE
#**********************************************************************
#
#  NAME: d$getsdata
#
#  PURPOSE:
#       To provide a standard means of retrieving the statistic and
#       configuration data from a server.
#
#  DESCRIPTION:
#       This function will dump the configuration and statistical data
#       from a server definition.
#
#  CALLING SEQUENCE:
#       call d$getsdata
#
#       Assumption that at least enough space is allocated for the
#       standard data portion of the server record.
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
d$getsdata:
#
# --- First, grab the return data address and length allowed.
#
        ld      mr_rptr(g0),r15         # Return data pointer
        mov     r15,r14                 # Make a copy
        ld      mr_ralloclen(g0),r13    # Return data maximum
        addo    r14,r13,r13             # Max data pointer
#
        ldconst MAXSERVERS,r10          # Max server ID
        ldconst mrrsiz,g2               # Set up return size
#
        ld      mr_ptr(g0),g0           # Parm block address
        ldos    mis_sid(g0),r12         # Server ID
#
# --- Validate parms
#
        ldconst deinvsid,g1             # Load error code
        cmpoble r10,r12,.gsd100         # Branch if too big SID
#
        lda     S_sddindx,r11           # Base address of the SDX
        ld      sx_sdd(r11)[r12*4],r8   # Get the SDD
        cmpobe  0,r8,.gsd100            # Null pointer, invalid server
#
# --- Fill in the structure with the statistical information
#
        ldq     sd_sid(r8),r4           # Get first quad of server data
        stq     r4,mis_sdata(r14)       # Store it
        ldq     sd_sid+16(r8),r4        # Get second quad of server data
        stq     r4,mis_sdata+16(r14)    # Store it
        ldq     sd_sid+32(r8),r4        # Get third quad of server data
        stq     r4,mis_sdata+32(r14)    # Store it
        ldq     sd_sid+48(r8),r4        # Get fourth quad of server data
        stq     r4,mis_sdata+48(r14)    # Store it
#
# --- Traverse the hash list for all LUN to VDisk mappings
#
        ldconst deok,g1                 # Load error code
        lda     mis_sdata+64(r14),r14   # Set the pointer to the output
#
        ld      sd_lvm(r8),r6           # LVM pointer
#
.gsd20:
        cmpobe  0,r6,.gsd40             # NULL pointer, grab next hash list
#
        cmpobg  r13,r14,.gsd30          # Jif enough space left
        ldconst detoomuchdata,g1        # Too much data to return
        b       .gsd80                  # Quit
#
.gsd30:
        ldos    lv_lun(r6),r3           # Get the LUN
        stos    r3,mis_lun(r14)
        ldos    lv_vid(r6),r3           # Get the VID
        stos    r3,mis_vid(r14)
#
        lda     4(r14),r14              # Move the output pointer
        ld      lv_nlvm(r6),r6          # Move the LVM pointer
        b       .gsd20                  # Process next LVM if it exists
#
.gsd40:
        ld      sd_ilvm(r8),r6          # Invisible LVM pointer
#
.gsd50:
        cmpobe  0,r6,.gsd80             # NULL pointer, grab next hash list
#
        cmpobg  r13,r14,.gsd60          # Jif enough space left
        ldconst detoomuchdata,g1        # Too much data to return
        b       .gsd80                  # Quit
#
.gsd60:
        ldos    lv_lun(r6),r3           # Get the LUN
        stos    r3,mis_lun(r14)
        ldos    lv_vid(r6),r3           # Get the VID
        stos    r3,mis_vid(r14)
#
        lda     4(r14),r14              # Move the output pointer
        ld      lv_nlvm(r6),r6          # Move the LVM pointer
        b       .gsd50                  # Process next LVM if it exists
#
.gsd80:
        subo    r15,r14,g2              # Calculate return data count
#
# --- Exit
#
.gsd100:
        ret
.endif  /* !ISCSI_CODE */
#
#**********************************************************************
#
#  NAME: d$getcdata
#
#  PURPOSE:
#       To provide a standard means of retrieving the statistic and
#       configuration data from the cache.
#
#  DESCRIPTION:
#       This function will dump the configuration and statistical data
#       from the global cache definition.
#
#  CALLING SEQUENCE:
#       call d$getcdata
#
#       Assumption that at least enough space is allocated for the
#       cache record.
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
d$getcdata:
        ld      mr_rptr(g0),r15         # Return data address pointer
c       memcpy((void*)(r15+mgc_data), (void*)&C_ca, casiz);
.ifndef MODEL_7400
.ifndef MODEL_3000
c       ((CA*)(r15+mgc_data))->status2 &= 0x80; # Leave temp disable bit, toss battery bad status.
.endif  # MODEL_3000
.endif  # MODEL_7400
        ldconst deok,g1                 # Load good return error code
        ldconst mgc_data+casiz,g2       # Set up return size
        ret
#
#**********************************************************************
#
#  NAME: d$getcddata
#
#  PURPOSE:
#       To provide a standard means of retrieving the statistic and
#       configuration data for the device specific cache.
#
#  DESCRIPTION:
#       This function will dump the configuration and statistical data
#       from the cache definition.
#
#  CALLING SEQUENCE:
#       call d$getcddata
#
#       Assumption that at least enough space is allocated for the
#       cache record.
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
d$getcddata:
#
# --- First, grab the return data address and length allowed.
#
        ld      mr_rptr(g0),r15         # Return data pointer
#
# --- Validate parms
#
        ldconst MAXVIRTUALS,r10         # Max virtual ID
        ld      mr_ptr(g0),g0           # Parm block address
        ldos    mcd_vid(g0),r12         # Virtual ID
#
        ldconst deinvvid,g1             # Load error code
        ldconst mrrsiz,g2               # Set up return size
        cmpoble r10,r12,.gcdd100        # Branch if virtual ID is too big
#
# --- Point to the correct VCD
#
        ld      vcdIndex[r12*4],r8      # Get the VCD address
        cmpobe  0,r8,.gcdd100           # Null pointer, invalid virtual disk
#
# --- Fill in the structure with the statistical information
#
        ldconst deok,g1                 # Load good return error code
#
c       memcpy((void*)(r15+mcd_data), (void*)(r8+vcdstatstart), vcdstatsize);
#
        ldconst mcd_data+vcdstatsize,g2 # Set up return size
#
# --- Exit
#
.gcdd100:
        ret
#
#**********************************************************************
#
#  NAME: d$setbathealth
#
#  PURPOSE:
#       Receives the status of the battery health from the CCB.
#
#  DESCRIPTION:
#       This function will report the battery health to the cache layer.
#
#  CALLING SEQUENCE:
#       call d$setbathealth
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
d$setbathealth:
#
# --- Validate parms
#
        ld      mr_ptr(g0),r3           # Parm block address
        ldob    mbh_brd(r3),g0          # Get battery Board ID
        ldob    mbh_state(r3),g1        # Get battery Health state
#
        call    WC$batHealth            # Alert write cache of battery state
#
        ldconst deok,g1                 # Load good return error code
        ldconst mbhrsiz,g2              # Return length
        ret
#
#**********************************************************************
#
#  NAME: d$wwntidlookup
#
#  PURPOSE:
#       Looks up a server based upon the WWN and TID.
#
#  DESCRIPTION:
#       This function encapsulates the wwnlookup function to provide
#       an MRP to look up a server based on WWN and TID.
#
#  CALLING SEQUENCE:
#       call d$wwntidlookup
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
d$wwntidlookup:
        ld      mr_ptr(g0),r3           # Parm block address
        ld      mr_rptr(g0),r15         # Get return parm pointer
#
        ldl     msl_wwn(r3),g0          # Get WWN
        ldos    msl_tid(r3),g2          # Get TID
        ldconst TRUE,g3                 # g3 = Find new servers
#
        PushRegs(r5)
        ldconst 0,g4                    # g4 = iSCSI name TBD
        call    DEF_WWNLookup
        mov     g0,r4
        PopRegsVoid(r5)
        mov     r4,g3
#
        ldconst deinvsid,g1             # Prep bad SID
        ldconst mslrsiz,g2              # Return length
#
        ldconst 0xffffffff,r3           # Check for bad return
        cmpobe  r3,g3,.ww100            # Exit
#
        ldconst deok,g1                 # No error
        stos    g3,msl_sid(r15)         # Save SID
#
# --- Exit
#
.ww100:
        ret
#
#**********************************************************************
#
#  NAME: d$assignmp
#
#  PURPOSE:
#       To provide a means of responding to the request to assign a
#       mirror partner to the FEP.
#
#  CALLING SEQUENCE:
#       call    d$assignmp
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = status
#       g2 = packet size
#
#  REGS DESTROYED:
#       g0-g2
#
#**********************************************************************
#
d$setmpconfigfe:
#       This is direct C - call (no proto is needed)
c       g0 = MP_SetMPConfigFE ((MR_PKT*)g0);
        mov g0,g1                       # Set return status
        ldconst mamprsiz,g2             # Return packet size
                                        # To define this MACRO.
        ret
#
#**********************************************************************
#
#  NAME: d$fibrehlist
#
#  PURPOSE:
#       To provide a means of responding to the request to set the list
#       fibre devices to poll once per second.
#
#  CALLING SEQUENCE:
#       call    d$fibrehlist
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = status
#       g2 = packet size
#
#  REGS DESTROYED:
#       g0-g2
#
#**********************************************************************
#
d$fibrehlist:
        ld      mr_ptr(g0),r15          # Get input parm block pointer
        ld      mr_rptr(g0),r14         # Get pointer into response packet
#
# --- Call the DLM function to handle this list
#
        mov     r15,g0                  # g0 = Input Parameter List pointer
        call    DLM$SetFibreHeartbeatList
                                        # Output g0 = Define Return Status
                                        # Output g1 = Invalid Controller S/N
!       st      g1,mfhl_icsn(r14)       # Save the Invalid Controller S/N
        mov     g0,g1                   # Set the Return Status
        ldconst mfhlrsiz,g2             # Return packet size
        ret
#
#**********************************************************************
#
#  NAME: d$contwomp
#
#  PURPOSE:
#       To provide a means of responding to the request to continue
#       cache initialization without a mirror partner.
#
#  CALLING SEQUENCE:
#       call    d$contwomp
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = status
#       g2 = packet size
#
#  REGS DESTROYED:
#       g0-g2
#
#**********************************************************************
#
d$contwomp:
        call    WC$continueWithoutMirrorPartner # Allow it to continue
                                        # Output g0 = Return Status
#
        mov     g0,g1                   # Move the Return Status to g1
        ldconst mrrsiz,g2               # Return packet size
        ret
#
#**********************************************************************
#
#  NAME: d$flushwomp
#
#  PURPOSE:
#       To provide a means of responding to the request to flush
#       the cache and ignore the mirror partner.
#
#  CALLING SEQUENCE:
#       call    d$flushwomp
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = status
#       g2 = packet size
#
#  REGS DESTROYED:
#       g0-g2
#
#**********************************************************************
#
d$flushwomp:
        ldconst FALSE,g1                # Show FlushWOMP not because of an error
        call    WC$FlushWOMP            # Go do the actual work
        mov     deok,g1                 # Set OK status
        ldconst mrrsiz,g2               # Return packet size
        ret
#
#**********************************************************************
#
#  NAME: d$flerrwomp
#
#  PURPOSE:
#       To provide a means of responding to the request to flush
#       the cache and ignore the mirror partner because an error occurred
#       trying to move data to/from the mirror partner.
#
#  CALLING SEQUENCE:
#       call    d$flerrwomp
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = status
#       g2 = packet size
#
#  REGS DESTROYED:
#       g0-g2
#
#**********************************************************************
#
d$flerrwomp:
        ldconst TRUE,g1                 # Show FlushWOMP because of an error
        call    WC$FlushWOMP            # Go do the actual work
        mov     deok,g1                 # Set OK status
        ldconst mrrsiz,g2               # Return packet size
        ret
#
#**********************************************************************
#
#  NAME: d$invfewc
#
#  PURPOSE:
#       To provide a means of responding to the request to invalidate
#       the front end write cache.
#
#  CALLING SEQUENCE:
#       call    d$invfewc
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = status
#       g2 = packet size
#
#  REGS DESTROYED:
#       g0-g2
#
#**********************************************************************
#
d$invfewc:
        ld      mr_ptr(g0),r15          # Get input parm block pointer
        ld      mr_rptr(g0),r14         # Get pointer into response packet
#
# --- Call the Write Cache function to handle this list
#
        mov     r15,g0                  # Input g0 = Parameter List pointer
        call    WC$InvalidateFE         # Go invalidate the FE Write Cache
                                        # Output g0 = status
                                        # Output g1 = Invalid VID
#
!       stos    g1,mif_ivid(r14)        # Save the Invalid VID
        mov     g0,g1                   # Return the returned status
        ldconst mifrsiz,g2              # Return packet size
        ret
#
#**********************************************************************
#
#  NAME: d$flushbewc
#
#  PURPOSE:
#       To provide a means of responding to the request to flush
#       the back end write cache.
#
#  CALLING SEQUENCE:
#       call    d$flushbewc
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = status
#       g2 = packet size
#
#  REGS DESTROYED:
#       g0-g2
#
#**********************************************************************
#
d$flushbewc:
        ld      mr_ptr(g0),r15          # Get input parm block pointer
        ld      mr_rptr(g0),r14         # Get pointer into response packet
#
# --- Determine if Cache is initialized.  If initialized, go and do the
#       Flush of the BE.  If not initialized, and a VID List Return an Error -
#       Invalid State.  If not initialized and Global Flush, set a flag for
#       the Recovery Task to do the Flush of the BE before setting
#       Cache Initialized.
#
        ldos    K_ii+ii_status,r13      # r13 = Initialization status
        bbs     iicinit,r13,.dflbewc50  # Jif Cache is Initialized
        ldob    mfb_op(r15),r3          # r3 = Options flag
        cmpobe  mfboinvalvid,r3,.dflbewc25 # Jif it is a VID list (not Init)
#
# --- Flush BE Global before Write Cache is initialized - Set Flag for the
#       Recovery Flush Task to Flush the BE when ready
#
        ldconst TRUE,r12                # Cache not init - set flag for Recovery
        st      r12,gWCPowerUpFlushBEFlag
        ldconst deok,g0                 # Show good status
        ldconst 0xFFFF,g1               # Show invalid VID
        b       .dflbewc100             # Go complete the request - Recovery
                                        #  task will do the Flush of the BE
#
# --- Flush BE with VID List before Write Cache is initialized - Error
#
.dflbewc25:
        ldconst deinitinprog,g0         # Show Initialization in Progress -
        ldconst 0xFFFF,g1               #  Unable to Flush BE with VID List
        b       .dflbewc100             #  if Cache is not Initialized
#
# --- Call the Write Cache function to handle this list
#
.dflbewc50:
        mov     r15,g0                  # Input g0 = Parameter List pointer
        call    WC$FlushBE              # Go flush the BE Write Cache
                                        # Output g0 = status
                                        # Output g1 = Invalid VID
#
.dflbewc100:
!       stos    g1,mfb_ivid(r14)        # Save the Invalid VID
        mov     g0,g1                   # Return the returned status
        ldconst mfbrsiz,g2              # Return packet size
        ret
#
#**********************************************************************
#
#  NAME: d$invbewc
#
#  PURPOSE:
#       To provide a means of responding to the request to invalidate
#       the back end write cache.
#
#  CALLING SEQUENCE:
#       call    d$invbewc
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = status
#       g2 = packet size
#
#  REGS DESTROYED:
#       g0-g2
#
#**********************************************************************
#
d$invbewc:
        ld      mr_ptr(g0),r15          # Get input parm block pointer
        ld      mr_rptr(g0),r14         # Get pointer into response packet
#
# --- Call the Write Cache function to handle this list
#
        mov     r15,g0                  # Input g0 = Parameter List pointer
        call    WC$InvalidateBE         # Go invalidate the BE Write Cache
                                        # Output g0 = status
                                        # Output g1 = Invalid VID
#
!       stos    g1,mib_ivid(r14)        # Save the Invalid VID
        mov     g0,g1                   # Return the returned status
        ldconst mibrsiz,g2              # Return packet size
        ret
#
#**********************************************************************
#
#  NAME: d$resetconfig
#
#  PURPOSE:
#       To provide a standard means of reseting the Front End
#       NVRAM configuration and NMI counts.
#
#  DESCRIPTION:
#       This function will clear the part 4 NVA records or the NMI
#       counts in NVRAM.
#
#  CALLING SEQUENCE:
#       call d$resetconfig
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
d$resetconfig:
        ld      mr_ptr(g0),r4           # Load MRP
        ldob    mre_options(r4),r9      # Get options
        cmpobe  mxnfenva,r9,.drsc10     # Jif only reseting FE NVA records
        cmpobe  mxnnmi,r9,.drsc20       # Jif only reseting NMI counts
        cmpobe  mxnbenva,r9,.drsc100    # Jif only reseting BE NVA records - exit
#
# --- Clear the NVA record
#
.drsc10:
        call    M$p4clear               # Clear P4 NVA records
        cmpobne mxnall,r9,.drsc100      # Jif not reseting all - return
#
# --- Clear the NMI counts
#
.drsc20:
        call    M$NMIclear              # Clear diag NMI counts
#
# --- Exit
#
.drsc100:
        ldconst deok,g1                 # Everything is fine
        ldconst mrersiz,g2              # Return length
        ret
#
#**********************************************************************
#
#  NAME: d$setcontrolsn
#
#  PURPOSE:
#        This routine changes the controller serial number to the
#        serial number contained in the MRP.
#
#  DESCRIPTION:
#        Change the controller serial number stored in NVRAM and the
#        K_FICB.
#
#  CALLING SEQUENCE:
#       call    d$setcontrolsn
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
d$setcontrolsn:
        mov     g3,r9                   # Save g3
        movl    g4,r10                  # Save g4 and g5
        ld      mr_ptr(g0),r5           # Input MRP data pointer
        ld      mcs_cntlsn(r5),r7       # Get new controller S/N
#
# --- Change the NVRAM controller serial number, byte by byte
#
        lda     NVSRAM+NVSRAMCSER,g5    # Address of control s/n in nvram
        lda     mcs_cntlsn(r5),g4       # Get new controller S/N addr
        ldconst 4,g3                    # Load 4 byte size
c       memcpy((void*)g5, (void*)g4, 4);    # Copy serial number to NVRAM
#       No need to either push or pop g-regs, as the following is a system call
#       g5 ---> Address of control s/n in NVRAM
#       g3 ---> Length (4 bytes) in bytes
#        c       msync((void*)g5, g3, MS_SYNC);

#
# --- Recalculate the NVRAM checksum
#
        mov     g0,r11                  # Save parm block pointer
c       g0 = MSC_CRC32((void *)(NVSRAM+NVSRAMSTARTSN+4),NVSRAMSNSIZ-4)           # Calculate the new checksum
        st      g0,d_nvram_csum         # Store Csum in memory
        lda     NVSRAM+NVSRAMSTARTSN,g5 # Load checksum location
        lda     d_nvram_csum,g4         # Address of new checksum
        ldconst 4,g3                    # Load 4 byte size
c       memcpy((void*)g5, (void*)g4, 4);    # g4 and g3 from last copy
        mov     r11,g0                  # Restore g0
#
#       No need to either push or pop g-regs, as the following is a system call
#       g5 ---> Address of checksum location
#       g3 ---> Length in bytes
#       NOTE:  the checksum field is located prior to S/N field, so ensure the
#              length includes the S/N since only one msync is done now
c       r4 = g5 % getpagesize();
c       r5 = r4 + (NVSRAMCSER - NVSRAMSTARTSN);
c       r3 = msync((void*)(g5 - r4), 4 + r5, MS_SYNC);
c       if (r3 != 0) fprintf(stderr, "d$setcontrolsn:  msync failed, errno = %d\n", errno);
#
# --- Change FICB
#
        ld      K_ficb,r3               # r3 = K_ficb pointer
        st      r7,fi_cserial(r3)       # Store serial number in FICB
#
        ldconst deok,g1                 # Return good status
        ldconst mcsrsiz,g2              # Return rsp size
#
        mov     r9,g3                   # Restore g3
        movl    r10,g4                  # Restore g4 and g5
        ret
#
#**********************************************************************
#
#  NAME: d$setconfigopt
#
#  PURPOSE:
#       To provide a standard means of configurable option
#       from the back end processor.
#
#  DESCRIPTION:
#       This function will set up configurable options based upon the
#       MRP received.
#
#  CALLING SEQUENCE:
#       call d$setconfigopt
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
d$setconfigopt:
        ld      mr_ptr(g0),r14          # Parm block address
#
# -- set up WHQL compliance
#
        ldob    msco_whql(r14),r4       # get WHQL compliance
        stob    r4,MAGD_SCSI_WHQL       # and save it

        ldconst deok,g1                 # Everything is fine
        ldconst mscorsiz,g2             # Return length
        ret
#
#**********************************************************************
#
#  NAME: d$NV_ProcessMMInfo
#
#  PURPOSE:
#       ASM to 'C' glue code to pass on an MM_INFO structure to the FEP.
#
#  CALLING SEQUENCE:
#       call    d$NV_ProcessMMInfo
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = status
#       g2 = packet size
#
#  REGS DESTROYED:
#       g0-g2
#
#**********************************************************************
#
d$NV_ProcessMMInfo:
c       g0 = NV_ProcessMMInfo((MR_PKT*)g0);
        mov     g0,g1                   # Set return status
        ldconst mmmrsiz,g2              # Return packet size
        ret
#
#**********************************************************************
#
#  NAME: d$qfecc
#
#  PURPOSE:
#       This routine queries the DLM module to see if the controller can be
#       talked to via the FE Fibre Communications
#
#  DESCRIPTION:
#       Queries the DLM module to see if the controller can be talked to via
#       the FE Fibre Communications
#
#  CALLING SEQUENCE:
#       call    d$qfecc
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
d$qfecc:
        ld      mr_ptr(g0),r3           # Get parm pointer
        ld      mqc_serial(r3),g0       # g0 = DLM - Controller Serial Number
#
        call    DLM$queryFEcomm         # Query the FE communications
        mov     g0,g1                   # Copy the return status to g1
        ldconst mqcrsiz,g2              # Return length
        ret
#
#**********************************************************************
#
#  NAME: d$qmpchange
#
#  PURPOSE:
#       To provide a standard means of determining if a change of the Mirror
#       Partner would complete successfully.
#
#  DESCRIPTION:
#       This function will call the routine to determine if a Mirror Partner
#       Change would complete successfully.  The Response is returned in the
#       response packet of the MRP.
#
#  CALLING SEQUENCE:
#       call d$qmpchange
#
#  INPUT:
#       g0 - MRP
#
#  OUTPUT:
#       g1 - error code
#       g2 - length of return packet
#
#  REGS DESTROYED:
#       g0-g2
#
#**********************************************************************
#
d$qmpchange:
        ld      mr_ptr(g0),r4           # Get parm pointer
        ld      mr_rptr(g0),r3          # Get the return data ptr
        mov     g0,r15                  # Save g0
        ld      mqmpc_serial(r4),g0     # g0 = Mirror Partner Serial Number
#
        call    CA_QueryMirrorPartnerChange  # Query Mirror Partner Change OK?
#
!       st      g0,mqmpcrsp(r3)         # Save the Response bits in the MRP Rsp
        ldconst deok,g1                 # Show successful completion
        ldconst mqmpcrsiz,g2            # Return length
        mov     r15,g0                  # Restore g0
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
#  CALLING SEQUENCE:
#       ld      id, g0
#       ld      data, g1
#       call    D$TraceEvent
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

#******************************************************************************
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
# Modelines:
# vi: sw=4 ts=4 expandtab
#
