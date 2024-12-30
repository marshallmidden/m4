# $Id: error.as 159305 2012-06-16 08:00:46Z m4 $
#**********************************************************************
#
#  NAME: error.as - consolidated error code.
#
#  PURPOSE:
#
#       This file contains the error branches from various modules.
#       See err.inc for a description of the errors.
#
#       On an ECC error, the boot code will either correct them or
#       trap and flash an error code. On all other errors the boot
#       code will just flash the LEDs and will not return.
#
#  Copyright (c) 1996 - 2008 Xiotech Corporation.  All rights reserved.
#
#**********************************************************************
        .globl  .kerr16
        .globl  .kerr17
        .globl  .kerr22
        .globl  .kerr23
        .globl  .kerr24
        .globl  .kerr25
        .globl  .kerr26
        .globl  .kerr27
        .globl  .kerr28
        .globl  .kerr29
#
        .globl  .err00
        .globl  .err01
        .globl  .err04
        .globl  .err06
        .globl  .err07
        .globl  error08
        .globl  .err09
.ifdef BACKEND
        .globl  .err10
.endif  # BACKEND
        .globl  error11
        .globl  .err12
        .globl  .err13
        .globl  .err15
        .globl  .err16
        .globl  .err17
.ifdef BACKEND
        .globl  .err18
.endif  # BACKEND
.ifdef FRONTEND
        .globl  .err19
.endif  # FRONTEND

.ifdef FRONTEND
        .globl  .err26
.endif  # FRONTEND
        .globl  .err29
        .globl  error31
        .globl  .err33
        .globl  .err35
        .globl  .err37
        .globl  errorLinux
        .globl  .errtrap
        .globl  e$LogECCEvents
        .globl  e$SaveRegs
        .globl  e$saveNVRAM5
        .globl  e$n5bytecpy
        .globl  e$padToQuad
        .globl  e$storeN5DataEntry
        .globl  e$storeN5EntryHdr
        .globl  e$storeN5Trace
        .globl  e$addrCheck
        .globl  boothandlerenable
        .globl  e_etrapped              # Flag if already error trapped
#
# --- Data
#
        .data
        .align  2
.booterrhandler:                        # function in boot code that handles
                                        # errors. This value is found at addr 0
                                        # when we initially boot up.
        .word   0
boothandlerenable:                      # Flag to indicate if the boot code
                                        # should handle error traps. (T/F)
.if     ERRTRAP_DEBUG
        .word   FALSE
.else   # ERRTRAP_DEBUG
        .word   TRUE
.endif  # ERRTRAP_DEBUG
#
# --- Error trapped flag
#
e_etrapped:                             # Flag to indicate if we are already
        .word   0                       # in an errortrapped condition
#
# --- Diagnostic data labels (ASCII) for part 5 nvram
#
part5_header_be:
        .ascii  "Diag BE "
#
part5_header_fe:
        .ascii  "Diag FE "
#
# --- ASCII label for each entry in the NVRAM part 5 diagnostic area
#     Index into these tables with the nvd_ constants in nvd.inc
#
.ifdef FRONTEND
        .globl    iscsiGetSsnHead
        .globl    fsl_get_tar

        .set    ssnsiz,sessionSize
        .set    cxnsiz,connectionSize
.endif  # FRONTEND
part5_labels:
        .ascii  "Consts  "              # Runtime constants
        .ascii  "Internal"              # Internal information (K_ii)
        .ascii  "Run FWH "              # Runtime firmware header
        .ascii  "Boot FWH"              # Boot firmware header
        .ascii  "Diag FWH"              # Diag firmware header
        .ascii  "Flt Rec "              # Flight recorder
        .ascii  "MRP Trce"              # MRP trace
        .ascii  "Defn EQ "              # Define layer exec queue
        .ascii  "Defn PCB"              # Define layer exec PCB
        .ascii  "Curr PCB"              # Currently running PCB
        .ascii  "Trgt def"              # Target definition
# ISCSI_CODE
        .ascii  "iTGDs   "              # iSCSI Info Structure
# ISCSI_CODE
        .ascii  "Target  "              # Target structure
        .ascii  "LLDMT Dr"              # LLDMT directory
        .ascii  "LLDMTs  "              # Data-Link Mgr Link lvl Driver Mgmt Tbl
        .ascii  "DTMTs   "              # Data-Link Manager Target Mgmt Table
        .ascii  "TPMTs   "              # Target Path Management Table
        .ascii  "MLMTs   "              # Magnitude Link Management Table
        .ascii  "Link QCS"              # Link Layer Queue Control Structure
        .ascii  "ISPReqQ0"              # ISP Request Queue 0
        .ascii  "ISPRspQ0"              # ISP Response Queue 0
        .ascii  "ISPReqQ1"              # ISP Request Queue 1
        .ascii  "ISPRspQ1"              # ISP Response Queue 1
        .ascii  "ISPReqQ2"              # ISP Request Queue 2
        .ascii  "ISPRspQ2"              # ISP Response Queue 2
        .ascii  "ISPReqQ3"              # ISP Request Queue 3
        .ascii  "ISPRspQ3"              # ISP Response Queue 3
.ifdef BACKEND
        .ascii  "BE IRAM "              # IRAM
        .ascii  "Defrag T"              # Defrag trace
        .ascii  "Phys EQ "              # Physical layer exec queue
        .ascii  "PhysEPCB"              # Physical layer exec PCB
        .ascii  "Phys CQ "              # Physical layer completion queue
        .ascii  "PhysCPCB"              # Physical layer completion PCB
        .ascii  "Raid EQ "              # Raid layer exec queue
        .ascii  "Raid PCB"              # Raid layer exec PCB
        .ascii  "Rd5 EQ  "              # Raid 5 exec queue
        .ascii  "Rd5 PCB "              # Raid 5 exec PCB
        .ascii  "Virt EQ "              # Virtual layer exec queue
        .ascii  "Virt PCB"              # Virtual layer exec PCB
        .ascii  "Rint EQ "              # Raid initialization exec queue
        .ascii  "Rint PCB"              # Raid initialization exec PCB
        .ascii  "FSys EQ "              # File system exec queue
        .ascii  "FSys PCB"              # File system exec PCB
        .ascii  "RdErr EQ"              # Raid error exec que
        .ascii  "RdErrPCB"              # Raid error exec PCB
        .ascii  "VDD     "              # Virtual device definitions
        .ascii  "Inq PCB "              # Inquire PCB
        .ascii  "Inq PDD "              # Inquire PDD
.endif  # BACKEND
.ifdef FRONTEND
        .ascii  "FE IRAM "              # IRAM
        .ascii  "TMTs    "              # Target Management Table
        .ascii  "TLMTs   "              # Target LUN Management Table
        .ascii  "ISMTs   "              # Initiator Session Management Table
        .ascii  "LTMTs   "              # Link-level Driver Target Mgmt Table
        .ascii  "LSMTs   "              # Link-level Driver Session Mgmt Table
        .ascii  "CIMT Dir"              # Channel interface mgmt table directory
        .ascii  "CIMTs   "              # Channel interface mgmt tables
        .ascii  "Trc log "              # Incoming trace log
        .ascii  "IMT     "              # IMTs
        .ascii  "ILMT/WET"              # ILMT and Working Environment Table
        .ascii  "VDMT    "              # Virtual device mgmt table
        .ascii  "SDD     "              # Server device definitions
        .ascii  "LVM     "              # LUN to VDisk mapping
        .ascii  "Inv LVM "              # Invisible LVM
        .ascii  "VCD     "              # Virtual cache definitions
        .ascii  "SvrdbDir"              # Server database directory
        .ascii  "Svr DB  "              # Server database
        .ascii  "CacheQHT"              # Cache queue head & tail
        .ascii  "CachePCB"              # Cache queue PCB
        .ascii  "CchIOQHT"              # Cache I/O queue head & tail
        .ascii  "CchIOPCB"              # Cache I/O queue PCB
        .ascii  "ICIMT Dr"              # ICIMT directory
        .ascii  "ICIMTs  "              # ICIMT
        .ascii  "ISCSISES"              # ISCSI Session Stats
        .ascii  "ISCSICON"              # ISCSI Connections
        .ascii  "IDD     "              # IDD
        .ascii  "Itrc log"              # Initiator trace log
.endif  # FRONTEND
#
# --- Data lengths of each data entry in the NVRAM part 5 diagnostic area
#
part5_dlengths:
        .word   nv5c_size               # Runtime constants
        .word   iisiz                   # Internal information (K_ii)
        .word   fh_size                 # Runtime firmware header
        .word   fh_size                 # Boot firmware header
        .word   fh_size                 # Diag firmware header
        .word   300*16                  # Flight recorder (300 entries)
        .word   300*16                  # MRP trace (300 entries)
        .word   16                      # Define layer exec queue
        .word   pcbsiz                  # Define layer exec PCB
        .word   pcbsiz                  # Currently running PCB
        .word   tgdsiz                  # Target definition
# ISCSI_CODE
        .word   itgdsiz                 # I_TGD definition
# ISCSI_CODE
        .word   tarsize                 # Target structure
        .word   MAXISP*4                # LLDMT directory
        .word   16                      # LLDMTs
        .word   dtmt_size               # DTMTs
        .word   tpmtsiz                 # TPMTs
        .word   mlmt_size               # MLMTs
        .word   qcsiz                   # Link QCS
        .word   6400                    # ISP Request Queue 0
        .word   6400                    # ISP Response Queue 0
        .word   6400                    # ISP Request Queue 1
        .word   6400                    # ISP Response Queue 1
        .word   6400                    # ISP Request Queue 2
        .word   6400                    # ISP Response Queue 2
        .word   6400                    # ISP Request Queue 3
        .word   6400                    # ISP Response Queue 3
.ifdef BACKEND
        .word   IRAMEND-IRAMBASE        # IRAM
        .word   128*16                  # Defrag trace
        .word   16                      # Physical layer exec queue
        .word   pcbsiz                  # Physical layer exec PCB
        .word   16                      # Physical layer completion queue
        .word   pcbsiz                  # Physical layer completion PCB
        .word   16                      # Raid layer exec queue
        .word   pcbsiz                  # Raid layer exec PCB
        .word   16                      # Raid 5 exec queue
        .word   pcbsiz                  # Raid 5 exec PCB
        .word   16                      # Virtual layer exec queue
        .word   pcbsiz                  # Virtual layer exec PCB
        .word   16                      # Raid initialization exec queue
        .word   pcbsiz                  # Raid initialization exec PCB
        .word   16                      # File system exec queue
        .word   pcbsiz                  # File system exec PCB
        .word   16                      # Raid error exec queue
        .word   pcbsiz                  # Raid error exec PCB
        .word   vddsiz                  # Virtual device definitions
        .word   pcbsiz                  # Inquire PCB
        .word   pddsiz                  # Inquire PDD
.endif  # BACKEND
.ifdef FRONTEND
        .word   IRAMEND-IRAMBASE        # IRAM
        .word   tmtsize                 # TMTs
        .word   tlmtsize                # TLMTs
        .word   ismtsize                # ISMTs
        .word   ltmt_size               # LTMTs
        .word   lsmt_size               # LSMTs
        .word   CIMTMAX*4               # Channel interface mgmt table directory
        .word   cimtsize                # Channel interface mgmt tables
        .word   trr_recsize*300         # Incoming trace log
        .word   imtsize                 # IMTs
        .word   ilmtsize+msalloc_sz     # ILMT and Working Environment Table
        .word   vdmtsize                # Virtual device mgmt table
        .word   sddsiz                  # Server device definition
        .word   lvsiz                   # LUN to VDISK mapping
        .word   lvsiz                   # Invisible LVM
        .word   vcdsiz                  # Virtual cache definitions
        .word   MAXISP*4                # Server database directory
        .word   SERVDBALLOC             # Server database
        .word   8                       # Cache queue head & tail
        .word   pcbsiz                  # Cache queue PCB
        .word   8                       # Cache I/O queue head & tail
        .word   pcbsiz                  # Cache I/O queue PCB
        .word   ICIMTMAX*4              # ICIMT Directory
        .word   icimtsize               # ICIMTs
#        .word   1657                    # Session Defn
#        .word   12451                   # Connection Defn
#        .word   ssnsiz                  # Session Defn
        .word   1680                    # Session Defn
        .word   cxnsiz                  # Connection Defn
        .word   72                      # IDD defn
        .word   trr_recsize*300         # Initiator trace log
.endif  # FRONTEND
#
# --- Constants listed for constants data entry in NVRAM
# See part5_dlengths, part5_labels, nvd.inc, nvd.h (NVRAM_P5_CONSTANTS)
#
const_list:
        .word   MAXTARGETS              # Max # of targets
        .word   MAXISP                  # Max # of ISPs
        .word   MAXVIRTUALS             # Max # of vdisks
        .word   CIMTMAX                 # Max # of CIMTs
        .word   LUNMAX                  # Max # of LUNs
        .word   MAXSERVERS              # Max # of servers
        .word   ICIMTMAX                # Max # of ICIMTs
        .word   MAXCHN                  # Max # of channels
        .word   MAXRAIDS                # Max # of raids
        .word   MAXDEV                  # Max # of devices
        .word   VRMAX                   # Max # of VRPs
        .word   MAXDRIVES               # Max # of drives
        .word   MAXIF                   # Max # of interfaces
        .word   MAXCTRL                 # Max # of controllers
        .word   MAXSES                  # Max # of drive enclosures
.ifdef FRONTEND
        .word   MAXLID                  # Max # of LIDs
.else   # FRONTEND
        .word   0                       # zero
.endif  # FRONTEND

# ----------------------------------------------------------------------------
# The following are not part of the constant list above.

# Temporary word for rediculous byte-by-byte copy into nvram.
word_length:
        .word   0

# A quad word array for padding to quad (e$padToQuad).
zero_pad:
        .word   0
        .word   0
        .word   0
        .word   0
# ----------------------------------------------------------------------------
        .text
#
#******************************************************************************
#
#  NAME: errtrap
#
#  PURPOSE:
#
#       To provide a common means of handling severe processor and firmware
#       detected faults.
#
#  DESCRIPTION:
#
#       This routine is called by either the fault handlers for processor
#       detected errors or by the firmware if logic errors are detected.
#       It saves some information and registers in SDRAM at entry.
#       It will cause a log entry in the CCB and call the boot code
#       to complete the error handling.
#
#       The boot code will return from correctable ECC errors.
#
#  CALLING SEQUENCE:
#
#       call errtrap
#
#  INPUT:
#
#       r3 - error code as defined in err.inc
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
        .text
.errtrap:
#
# --- Save info and registers in DRAM
#       Offset      Value
#       ------      -----
#       0x00        error code - NISR is in upper 2 bytes if this is an NMI
#       0x04        processor base ATU address
#       0x08        firmware revision
#       0x0C        firmware revcount
#       0x10        pfp
#       0x14        sp
#       0x18        rip
#       0x1C        r3-r15 - TRAPADDR_R3 must change if r3 is moved
#                   r3 = IMR if entry from interrupt
#                   r4 = NISR if entry from an nmi
#       0x50        g0-g14
#       0x8c        fp (g15)
#                   for error codes 0-a (processor faults), the fp will
#                     point to the fault record, I think
#       0x90-0x93   Available
#       0x94-0x9F   Timestamp from last CCB heartbeat
#
#       0x0100      start of IRAM (copy of addr 0x0000-0x0400)
#
#       0x0500      NMI counts (format in nmi.inc)
#
#       0x05C0      Internal registers (format in reg.inc) -- does not apply Wookiee.
#
# --- Create 160 (0xA0) bytes of storage on stack for save/restore space
#     0x00 - 0x3F: G register capture
#       0x00 - 0x0F: g0-g3
#       0x10 - 0x1F: g4-g7
#       0x20 - 0x2F: g8-g11
#       0x30 - 0x3F: g12-g15
#     0x40 - 0x7F: R register capture
#       0x40 - 0x43: r0 (PFP)
#       0x44 - 0x47: r1 (SP)
#       0x48 - 0x4B: r2 (RIP)
#       0x4C - 0x4F: r3
#       0x50 - 0x5F: r4-r7
#       0x60 - 0x6F: r8-r11
#       0x70 - 0x7F: r12-r15
#     0x80 - 0x9F: general storage
#       0x80 - 0x9F: Available
#
        lda     0xA0(sp),sp             # Create 160 byte storage on stack
?       stq     r4,-0x50(sp)            # r4-r7 saved to stack
#
# --- Save away the values in all registers to stack (save/restore)
#
?       stq     g0,-0xA0(sp)            # Save g0-g3 on stack
?       stq     g4,-0x90(sp)            # Save g4-g7 on stack
?       stq     g8,-0x80(sp)            # Save g8-g11 on stack
?       stq     g12,-0x70(sp)           # Save g12-g15 on stack
?       stq     pfp,-0x60(sp)           # r0-r3 saved on stack
?       stq     r8,-0x40(sp)            # r8-r11 saved on stack
?       stq     r12,-0x30(sp)           # r12-r15 saved on stack
#
# --- Adjust the saved SP back to what it was before we increased the stack
#
        lda     -0xA0(sp),r4            # r4 = original SP
        st      r4,-0x5C(sp)            # Saved SP = original SP
#
# --- Save away the values in all registers to TRAPADDR (debug)
#
        ldconst TRAPADDR,r4             # place to store regs
        st      r3,0(r4)                # error code
.ifdef BACKEND
        ldconst 1,r3                    # BE = 1, Tells CCB when we log, BE
.endif # BACKEND
.ifdef FRONTEND
        ldconst 0,r3                    # FE = 0, Tells CCB when we log, FE
.endif # FRONTEND
        st      r3,0x04(r4)
c       r5 = (ulong)&fwHeader;
        ldl     fh_revision(r5),r6      # get revision and rev count
        stl     r6,0x08(r4)
#
#        c       extern  ulong errGreg[16];
        ld      0(r4),r7                # Retrieve errCode
        cmpobge r7,errLinux,.errt07     # If Linux errtrap regs already saved
        ldconst TRUE,r7                 # Load TRUE into flag
        st      r7,e_etrapped           # to prevent next recursion
c       memcpy((void*)errGreg, (void*)greg, 0x40);
#
# NOTE: r3 saved before gotten into this routine.
?       ldt     -0x60(sp),r8            # Get r0-r2 from stack
?       stt     r8,0x10(r4)             # Copy saved r0-r2 to TRAPADDR
#
?       ldq     -0x50(sp),r8            # Get r4-r7 from stack
?       stq     r8,0x20(r4)             # Copy saved r4-r7 to TRAPADDR
#
?       stq     r8,0x30(r4)             # Copy r8-r11 to TRAPADDR
?       stq     r12,0x40(r4)            # Copy r12-r15 to TRAPADDR
#
?       stq     g0,0x50(r4)             # Copy g0-g3 to TRAPADDR
?       stq     g4,0x60(r4)             # Copy g4-g7 to TRAPADDR
?       stq     g8,0x70(r4)             # Copy g8-g11 to TRAPADDR
?       stq     g12,0x80(r4)            # Copy g12-g15 to TRAPADDR
#
.errt07:
#
# --- Save timestamp into TRAPADDR+0x94
#
        ldconst TRAPADDR+0x094,r7       # Load destination address
?       ldt     hbeat_timestamp,r4      # Load timestamp address
?       stt     r4,(r7)                 # Store timestamp
#
# --- Save internal memory at addr TRAPADDR+0x100
#
c       r5 = (ulong)&IRAMBASE;
c       r6 = (ulong)&IRAMEND;
        ldconst TRAPADDR+0x100,r7       # starting destination DRAM addr
.errt10:
        ldq     (r5),r8                 # read 16 bytes of internal RAM
        stq     r8,(r7)                 # save to DRAM
        addo    0x10,r5,r5              # bump to next addrs
        addo    0x10,r7,r7
        cmpobl  r5,r6,.errt10           # loop until we reach the end
#
# --- Copy NMI counts from NVRAM to addr TRAPADDR+0x500
#
        ldconst NVSRAMNMISTART,g4       # Load NVRAM NMI counters address
        ldconst TRAPADDR+0x500,g5       # Load DRAM address
        ldconst nmisiz,g3               # Load NMI count size
c       memcpy((void*)g5, (void*)g4, g3);   # Copy counters back into NVRAM
#
# --- Copy some internal registers to addr TRAPADDR+0x50C
#
        ldconst TRAPADDR+0x05C0,g0      # Load DRAM address
        call    e$SaveRegs
#
# --- Initialize the Debug Data Retrieval (DDR) table entries for error trap data
# --- Initialize error trap registers data
#
        ldconst TRAPADDR,g1             # Load address of trap data
c       M_addDDRentry(de_etregs, g1, 144);  # Size of trap data
#
# --- Initialize error trap internal ram data
#
        ldconst TRAPADDR+0x100,g1       # Load address of trap data
c       M_addDDRentry(de_etiram, g1, 1024);  # Length of trap data
#
# --- Initialize error trap NMI counts data
#
        ldconst TRAPADDR+0x500,g1       # Load address of trap data
c       M_addDDRentry(de_etnmi, g1, 192);  # Length of trap data
#
# --- Initialize error trap internal registers data
#
        ldconst TRAPADDR+0x5C0,g1       # Load address of trap data
c       M_addDDRentry(de_etiregs, g1, 96);  # Length of trap data
#
# --- Save diagnostic data into nvram part 5
#
        call    e$saveNVRAM5            # Save diagnostic data into nvram
#
# --- Update the Debug Data Retrieval (DDR) table NVRAM Diagnostic Data entry
#
        ldconst NVSRAMP5START,g1        # Load address of Part 5 NVRAM
        ld      nv5h_len(g1),g2         # Load length of Part 5 NVRAM
c       M_addDDRentry(de_nvram5, g1, g2);
#
# --- Log the error trap
#
        call    LL_Errtrap              # Log the error trap
#
#
# --- Fatal error trap - force crash dump, then abort.
#
c fprintf(stderr,"%sDEADLOOP(%s:%i)\n", L_MsgPrefix, __FILE__, __LINE__);
c       L_CrashDump(errLinuxSignal, trap_sinf, trap_ebp);
c       abort();
#
#******************************************************************************
#
#  NAME: e$SaveRegs
#
#  PURPOSE:
#
#       To provide a means of copying internal registers into DRAM during
#       an error trap.
#
#  DESCRIPTION:
#
#       The format of the register data is found in reg.inc -- does not apply Wookiee.
#
#  CALLING SEQUENCE:
#
#       call e$SaveRegs
#
#  INPUT:
#
#       g0 - Address of where to store in DRAM
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
e$SaveRegs:
#
# --- Store the individual registers into DRAM
#
        ret
#
#******************************************************************************
#
#  NAME: e$saveNVRAM5
#
#  PURPOSE:
#
#       To save diagnostic data into part 5 of NVRAM after an error trap.
#
#  DESCRIPTION:
#
#       The K_ii data, firmware header, flight recorder, MRP trace, defrag
#       trace, exec queues and their pcbs, and isp layer request and response
#       queues are saved to NVRAM where it can be retrieved later.
#
#       Global registers don't need to be save/restored since deadloop restores
#       them after this routine returns.
#
#  CALLING SEQUENCE:
#
#       call e$saveNVRAM5
#
#  INPUT:
#
#       None.
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
e$saveNVRAM5:
#
# --- Turn on writability of NVRAM
#
#
# --- Create the NVRAM part 5 header label and timestamp
#
.ifdef FRONTEND
        lda     part5_header_fe,g4      # Load hdr label addr
.else   # FRONTEND
        lda     part5_header_be,g4      # Load hdr label addr
.endif  # FRONTEND
        ldconst NVSRAMP5START+nv5h_id,g5  # Load nvram hdr addr
        ldconst nv5h_lab_size,g3        # Load label length
        mov     g5,r12                  # r12 = ptr to pt 5 header
        ldconst nv5h_size,r13           # Load header size
        addo    r13,g5,r13              # r13 = ptr to start of pt 5 data
c       memcpy((void*)g5, (void*)g4, g3);   # Transfer label to nvram
#
        lda     nv5h_time(r12),g5       # Load nvram pt 5 timestamp
        lda     hbeat_timestamp,g4      # Load timestamp addr
c       g3 = sizeof(FW_HEADER_TIMESTAMP);
#        ldconst ft_size,g3              # Load size of timestamp
c       memcpy((void*)g5, (void*)g4, g3);   # Transfer timestamp to pt 5 nvram
        addo    g3,g5,g5                # Increment nvram pointer
        call    e$padToQuad             # Pad to the next quad boundary

#
# --- Save runtime constants to NVRAM
#
        ldconst nvd_consts,g0           # Load constant table size
        lda     const_list,g4           # Load constant table addr
        call    e$storeN5DataEntry      # Load runtime constants to nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
#
# --- Save internal RAM (IRAM) to NVRAM
#
.ifdef FRONTEND
        ldconst nvd_feiram,g0           # Load FE IRAM index
.else   # FRONTEND
        ldconst nvd_beiram,g0           # Load BE IRAM index
.endif  # FRONTEND
        ldconst IRAMBASE,g4             # Load iram base address
        call    e$storeN5DataEntry      # Load K_ii to nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
#
# --- Save internal information (K_ii) to NVRAM
#
        ldconst nvd_ii,g0               # Load ii index
        lda     K_ii,g4                 # Load K_ii address
        call    e$storeN5DataEntry      # Load K_ii to nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
#
# --- Save firmware headers (runtime, boot and diag) to NVRAM
#
        ldconst nvd_fwhr,g0             # Load runtime fw header index
c       g4 = (ulong)&fwHeader;
        call    e$storeN5DataEntry      # Load runtime fw hdr to nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
#
#
# --- Save Flight Recorder to NVRAM (last 300 entries)
#
.if DEBUG_FLIGHTREC
        ldconst nvd_frec,g0             # Load flight recorder index
        ld      fr_queue,g4             # Load flight recorder address
        cmpobe  0,g4,.esn_fl_5          # Jif if pointer NULL
        ldl     (g4),g8                 # Load BEGIN and IN ptrs
        ld      qc_end(g4),g10          # Load END ptr
        call    e$storeN5Trace          # Save flight recorder into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
.esn_fl_5:
.endif  # DEBUG_FLIGHTREC
#
# --- Save MRP Trace to NVRAM (last 300 entries)
#
        lda     defTraceQue,r3          # Load the mrp trace ptr
        ldq     (r3),g8                 # g8 = BEGIN, g9 = IN, g10 = END
                                        #   g11 = RunFlag
        cmpibe  0,g11,.esn_10           # Jif mrp trace is not enabled
#
        ldconst nvd_mrp,g0              # Load mrp trace index
        mov     r3,g4                   # Load mrp trace address
        call    e$storeN5Trace          # Load mrp trace into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
.esn_10:
#
# --- Load the Define executive queue and its PCB to NVRAM
#
        ldconst nvd_deq,g0              # Load define q diag index
        lda     d_exec_qu,g4            # Load define q addr
        call    e$storeN5DataEntry      # Load define q into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
#
        ldconst nvd_dep,g0              # Load define pcb diag index
        ld      qu_pcb(g4),g4           # Load define pcb addr
        call    e$storeN5DataEntry      # Load define pcb into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
#
# --- Load the currently running PCB to NVRAM
#
        ldconst nvd_cpcb,g0             # Load current pcb diag index
        ld      K_xpcb,g4               # Load current pcb addr
        call    e$storeN5DataEntry      # Load current pcb into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
#
# --- Load TGDs to NVRAM
#
        ldconst MAXTARGETS,r7           # Load number of possible tgds
        ldconst 0,r8                    # Load index into tgdx
.esn_20:
        ld      T_tgdindx[r8*4],r6      # Load tgd address
        cmpobe  0,r6,.esn_30            # Jif not valid tgd
#
        ldconst nvd_tgd,g0              # Load tgd diag index
        mov     r6,g4                   # Load tgd address
        call    e$storeN5DataEntry      # Load tgd into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
.esn_30:
        addo    1,r8,r8                 # Increment tgd index
        cmpobne r7,r8,.esn_20           # Jif not the last tgd slot

# ISCSI_CODE
#
# --- Load I_TGDs to NVRAM
#
        ldconst MAXTARGETS,r7           # Load number of possible tgds
        ldconst 0,r8                    # Load index into tgdx
.esn_31:
        ld      T_tgdindx[r8*4],r6      # Load tgd address
        cmpobe  0,r6,.esn_32            # Jif not valid tgd
        ld      tgd_itgd(r6),r6         # Load i_tgd address
        cmpobe  0,r6,.esn_32            # Jif not valid i_tgd
#
        ldconst nvd_itgd,g0             # Load itgd diag index
        mov     r6,g4                   # Load itgd address
        call    e$storeN5DataEntry      # Load tgd into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
.esn_32:
        addo    1,r8,r8                 # Increment tgd index
        cmpobne r7,r8,.esn_31           # Jif not the last tgd slot
# ISCSI_CODE

#
# --- Load Target structures to NVRAM
#
        ldconst MAXISP,r7               # Load number of isps to check
        ldconst 0,r8                    # Load index into tar
.esn_40:
        ld      tar[r8*4],r6            # Load target address
.esn_50:
        cmpobe  0,r6,.esn_60            # Jif no targets left
#
        ldconst nvd_tgt,g0              # Load target diag index
        mov     r6,g4                   # Load target address
        call    e$storeN5DataEntry      # Load target into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
#
        ld      tarfthd(r6),r6          # Get next target on the linked list
        b       .esn_50
.esn_60:
        addo    1,r8,r8                 # Increment tar  index
        cmpobne r7,r8,.esn_40           # Jif not the last isp
#
# --- Save MLMTs to NVRAM
#
        ld      dlm_mlmthd,r6           # Load first mlmt
.esn_70:
        cmpobe  0,r6,.esn_80            # Jif invalid MLMT
#
        ldconst nvd_mlmt,g0             # Load mlmt diag index
        mov     r6,g4                   # Load mlmt address
        call    e$storeN5DataEntry      # Load mlmt into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
        ld      mlmt_link(r6),r6        # Get next mlmt
        b       .esn_70
#
.esn_80:
#
# --- Save Link layer QCS to NVRAM
#
        ldconst nvd_lqcs,g0             # Load link qcs diag index
#        c       extern QUEUE_CONTROL LINK_QCS;
c       g4 = (ulong)&LINK_QCS;
        call    e$storeN5DataEntry      # Load link qcs into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
.ifdef  BACKEND
#
# --- Save Defrag Trace to NVRAM
#
        ldconst nvd_defrag,g0           # Load defrag trace index
        lda     gDFDebug,g4             # Load defrag trace address
        call    e$storeN5DataEntry      # Load defrag trace into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
#
# --- Load the Physical executive queue and its PCB to NVRAM
#
        ldconst nvd_peq,g0              # Load physical q diag index
        lda     P_exec_qu,g4            # Load physical q addr
        call    e$storeN5DataEntry      # Load physical q into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
#
        ldconst nvd_pep,g0              # Load physical pcb diag index
        ld      qu_pcb(g4),g4           # Load physical pcb addr
        call    e$storeN5DataEntry      # Load physical pcb into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
#
# --- Load the Physical completion queue and its PCB to NVRAM
#
        ldconst nvd_pcq,g0              # Load physical q diag index
        lda     P_comp_qu,g4            # Load physical q addr
        call    e$storeN5DataEntry      # Load physical q into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
#
        ldconst nvd_pcp,g0              # Load physical pcb diag index
        ld      qu_pcb(g4),g4           # Load physical pcb addr
        call    e$storeN5DataEntry      # Load physical pcb into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
#
# --- Load the Raid executive queue and its PCB to NVRAM
#
        ldconst nvd_req,g0              # Load raid q diag index
        lda     R_exec_qu,g4            # Load raid q addr
        call    e$storeN5DataEntry      # Load raid q into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
#
        ldconst nvd_rep,g0              # Load raid pcb diag index
        ld      qu_pcb(g4),g4           # Load raid pcb addr
        call    e$storeN5DataEntry      # Load raid pcb into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
#
# --- Load the Raid 5 executive queue and its PCB to NVRAM
#
        ldconst nvd_r5eq,g0             # Load raid 5 q diag index
        lda     R_r5exec_qu,g4          # Load raid 5 q addr
        call    e$storeN5DataEntry      # Load raid 5 q into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
#
        ldconst nvd_r5ep,g0             # Load raid 5 pcb diag index
        ld      qu_pcb(g4),g4           # Load raid 5 pcb addr
        call    e$storeN5DataEntry      # Load raid 5 pcb into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
#
# --- Load the Virtual executive queue and its PCB to NVRAM
#
        ldconst nvd_veq,g0              # Load virtual q diag index
        lda     V_exec_qu,g4            # Load virtual q addr
        call    e$storeN5DataEntry      # Load virtual q into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
#
        ldconst nvd_vep,g0              # Load virtual pcb diag index
        ld      qu_pcb(g4),g4           # Load virtual pcb addr
        call    e$storeN5DataEntry      # Load virtual pcb into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
#
# --- Load the Raid init executive queue and its PCB to NVRAM
#
        ldconst nvd_rieq,g0             # Load raid init q diag index
        lda     d_rip_exec_qu,g4        # Load raid init q addr
        call    e$storeN5DataEntry      # Load raid init q into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
#
        ldconst nvd_riep,g0             # Load raid init pcb diag index
        ld      qu_pcb(g4),g4           # Load raid init pcb addr
        call    e$storeN5DataEntry      # Load raid init pcb into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
#
# --- Load the File system executive queue and its PCB to NVRAM
#
        ldconst nvd_fseq,g0             # Load file system q diag index
        lda     f_exec_qu,g4            # Load file system q addr
        call    e$storeN5DataEntry      # Load file system q into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
#
        ldconst nvd_fsep,g0             # Load file system pcb diag index
        ld      qu_pcb(g4),g4           # Load file system pcb addr
        call    e$storeN5DataEntry      # Load file system pcb into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
#
# --- Load the Raid error executive queue and its PCB to NVRAM
#
        ldconst nvd_reeq,g0             # Load raid error q diag index
        lda     RB_rerror_qu,g4         # Load raid error q addr
        call    e$storeN5DataEntry      # Load raid error q into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
#
        ldconst nvd_reep,g0             # Load raid error pcb diag index
        ld      qu_pcb(g4),g4           # Load raid error pcb addr
        call    e$storeN5DataEntry      # Load raid error pcb into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
#
# --- Save VDDs to NVRAM
#
        ldconst MAXVIRTUALS,r7          # Load number of possible vdds
        ldconst 0,r8                    # Load index into vdx
.esn_90:
        ld      V_vddindx[r8*4],r6      # Load vdd address
        cmpobe  0,r6,.esn_100           # Jif not valid vdd
#
        ldconst nvd_vdd,g0              # Load vdd diag index
        mov     r6,g4                   # Load vdd address
        call    e$storeN5DataEntry      # Load vdd into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
.esn_100:
        addo    1,r8,r8                 # Increment vdd index
        cmpobne r7,r8,.esn_90           # Jif not the last vdd slot
#
# --- Save all PCBs and PDDs for all outstanding inquire processes
#
        ldconst MAXDRIVES,r7            # Load number of possible PCBs
        ldconst 0,r8                    # Load index into O_ipcb
.esn_102:
        ld      O_ipcb[r8*4],r6         # Load PCB
        cmpobe  0,r6,.esn_106           # Jif not a valid PCB
#
        ldconst nvd_ipcb,g0             # Load pcb diag index
        mov     r6,g4                   # Load pcb address
        call    e$storeN5DataEntry      # Load pcb into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
#
        ldconst nvd_ipdd,g0             # Load pdd diag index
        ld      pc_g0+12(r6),g4         # Load g3 of PCB, which is the PDD
        call    e$storeN5DataEntry      # Load pdd into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
.esn_106:
        addo    1,r8,r8                 # Increment O_ipcb index
        cmpobne r7,r8,.esn_102          # Jif not the last pcb slot
.endif  # BACKEND
.ifdef  FRONTEND
#
# --- Save the CIMT directory to NVRAM
#
        ldconst nvd_cimt_dir,g0         # Load cimt dir index
        lda     cimtDir,g4              # Load cimt dir addr
        call    e$storeN5DataEntry      # Load cimt dir into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
#
# --- Save the CIMTs and their valid trace logs to NVRAM
#
        mov     g4,r6                   # r6 = ptr into cimt dir
        ldconst CIMTMAX,r3              # Load max cimts
.esn_110:
        ld      (r6),r4                 # Load cimt address
        cmpobe  0,r4,.esn_120           # Jif not a cimt address
#
# --- Valid cimt, store cimt structure into NVRAM
#
        ldconst nvd_cimt,g0             # Load cimt index
        mov     r4,g4                   # Load cimt address
        call    e$storeN5DataEntry      # Load cimt into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
#
# --- Store valid trace logs to NVRAM
#
        ldob    ci_state(r4),r7         # Load cimt state
        cmpobe  cis_init,r7,.esn_120    # Jif cimt is in uninitialized state
        ld      ci_begtr(r4),g8         # Load pointer to trace log
        cmpobe  0,g8,.esn_120           # Jif invalid pointer to trace log
        ldconst nvd_trc,g0              # Load trace diag index
        mov     g8,g4                   # Load trace address
        ld      ci_curtr(r4),g9         # Load current pointer in trace log
        ld      ci_endtr(r4),g10        # Load end pointer of trace log
        call    e$storeN5Trace          # Load trace log to nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
.esn_120:
        lda     4(r6),r6                # Point to next cimt
        subo    1,r3,r3                 # Decrement counter
        cmpobne 0,r3,.esn_110           # Jif not done looking for cimts
#
# --- Save Allocated IMTs and associated ILMTs to NVRAM
#
        ld      mag_imt_head,r6         # Load pointer to allocated IMTs
.esn_130:
        cmpobe  0,r6,.esn_160           # Jif no more IMTs
        ldconst nvd_imt_al,g0           # Load allocated imt index
        mov     r6,g4                   # Load imt address
        call    e$storeN5DataEntry      # Load imt into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
#
# --- Check for ILMTs in this IMT
#
        lda     im_ilmtdir(r6),r7       # Load ilmt directory into r7
        ldconst LUNMAX,r8               # Load number of directory entries
.esn_140:
        ld      (r7),r9
        cmpobe  0,r9,.esn_150           # Jif no valid ilmt for this lun
#
# --- Save ILMT to NVRAM
#
        ldconst nvd_ilmt,g0             # Load ilmt index
        mov     r9,g4                   # Load ilmt address
        call    e$storeN5DataEntry      # Load ilmt into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
#
.esn_150:
        addo    4,r7,r7                 # Increment ilmt directory pointer
        subo    1,r8,r8                 # Decrement lun counter
        cmpobne 0,r8,.esn_140           # Jif more ilmt/luns to check
#
        ld      im_link2(r6),r6         # Find next imt on the linked list
        b       .esn_130                # Loop back
.esn_160:
#
# --- Save VDMTs to NVRAM
#
        ldconst MAXVIRTUALS,r7          # Load number of possible vdmts
        ldconst 0,r8                    # Load index into vdmts
.esn_170:
        ld      MAG_VDMT_dir[r8*4],r6   # Load vdmt address
        cmpobe  0,r6,.esn_180           # Jif not valid vdmt
#
        ldconst nvd_vdmt,g0             # Load vdmt diag index
        mov     r6,g4                   # Load vdmt address
        call    e$storeN5DataEntry      # Load vdmt into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
.esn_180:
        addo    1,r8,r8                 # Increment vdmt index
        cmpobne r7,r8,.esn_170          # Jif not the last vdmt slot
#
# --- Save SDDs and associated LVMs to NVRAM
#
        ldconst MAXSERVERS,r7           # Load number of possible sdds
        ldconst 0,r8                    # Load index into sdx
.esn_190:
        ld      S_sddindx[r8*4],r6      # Load sdd address
        cmpobe  0,r6,.esn_230           # Jif not valid sdd
#
        ldconst nvd_sdd,g0              # Load sdd diag index
        mov     r6,g4                   # Load sdd address
        call    e$storeN5DataEntry      # Load sdd into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
#
# --- Check for valid LVMs in this SDD
#
        ld      sd_lvm(r6),r10          # Load link to lvms
.esn_200:
        cmpobe  0,r10,.esn_210          # Jif no more lvms
#
        ldconst nvd_lvm,g0              # Load lvm diag index
        mov     r10,g4                  # Load lvm address
        call    e$storeN5DataEntry      # Load lvm into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
        ld      lv_nlvm(r10),r10        # Get the next lvm
        b       .esn_200
.esn_210:
        ld      sd_ilvm(r6),r10         # Load link to invisible lvms
.esn_220:
        cmpobe  0,r10,.esn_230          # Jif no more lvms
#
        ldconst nvd_ilvm,g0             # Load invisible lvm diag index
        mov     r10,g4                  # Load lvm address
        call    e$storeN5DataEntry      # Load lvm into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
        ld      lv_nlvm(r10),r10        # Get the next invisible lvm
        b       .esn_220
.esn_230:
        addo    1,r8,r8                 # Increment sdd index
        cmpobne r7,r8,.esn_190          # Jif not the last sdd slot
#
# --- Save VCDs to NVRAM
#
        ldconst MAXVIRTUALS,r7          # Load number of possible vcds
        ldconst 0,r8                    # Load index into vcx
.esn_240:
        ld      vcdIndex[r8*4],r6       # Load vcd address
        cmpobe  0,r6,.esn_250           # Jif not valid vcd
#
        ldconst nvd_vcd,g0              # Load vcd diag index
        mov     r6,g4                   # Load vcd address
        call    e$storeN5DataEntry      # Load vcd into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
.esn_250:
        addo    1,r8,r8                 # Increment vcd index
        cmpobne r7,r8,.esn_240          # Jif not the last vcd slot
#
# --- Save Server database directory and associated server databases to NVRAM
#
        ldconst nvd_sdbd,g0             # Load server db directory diag index
        lda     servdb,g4               # Load server db directory address
        call    e$storeN5DataEntry      # Load server db directory into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
#
        ldconst MAXISP,r7               # Load number of possible server dbs
        ldconst 0,r8                    # Load index into server db directory
.esn_260:
        ld      servdb[r8*4],r6         # Load server db address
        cmpobe  0,r6,.esn_270           # Jif invalid server db address
#
        ldconst nvd_sdb,g0              # Load server db diag index
        mov     r6,g4                   # Load server db address
        call    e$storeN5DataEntry      # Load server db into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
.esn_270:
        addo    1,r8,r8                 # Increment db index
        cmpobne r7,r8,.esn_260          # Jif not the last server slot
#
# --- Save Cache queue head/tail and PCB to NVRAM
#
        ld      C_exec_qht,r6           # Load cache q head
        cmpobe  0,r6,.esn_280           # Jif q head is invalid
        ldconst nvd_cqht,g0             # Load cache q diag index
        lda     C_exec_qht,g4           # Load cache q address
        call    e$storeN5DataEntry      # Load cache q into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
.esn_280:
        ld      C_exec_pcb,g4           # Load cache q pcb
        cmpobe  0,g4,.esn_290           # Jif cache q pcb is invalid
        ldconst nvd_cqpcb,g0            # Load cache q pcb diag index
        call    e$storeN5DataEntry      # Load cache q pcb into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
.esn_290:
#
# --- Save Cache I/O queue head/tail and PCB to NVRAM
#
        ld      C_ioexec_qht+qu_head,r6 # Load cache io q head
        cmpobe  0,r6,.esn_300           # Jif q head is invalid
        ldconst nvd_ciqht,g0            # Load cache io q diag index
        lda     C_ioexec_qht,g4         # Load cache io q address
        call    e$storeN5DataEntry      # Load cache io q into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
.esn_300:
        ld      C_ioexec_pcb,g4         # Load cache io q pcb
        cmpobe  0,g4,.esn_310           # Jif cache io q pcb is invalid
        ldconst nvd_ciqpcb,g0           # Load cache io q pcb diag index
        call    e$storeN5DataEntry      # Load cache io q pcb into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
.esn_310:
# ISCSI_CODE
.ifdef FRONTEND
        ldconst MAXTARGETS,r7           # Load number of possible tgds
        ldconst 0,r8                    # Load index into tgdx
.esn_301:
        ld      T_tgdindx[r8*4],r6      # Load tgd address
        cmpobe  0,r6,.esn_308           # Jif not valid tgd
        PushRegs(r3)
        ldos    tgd_tid(r6),g0
        call    fsl_get_tar
        mov     g0,r9
        PopRegs(r3)
        cmpobe  0,r9,.esn_308
        ld      tarportid(r9),r9        # get portID from TAR
.esn_302:
        cmpobe  0,r9,.esn_308
        ld      (r9),r6                 # Point to session
        ldconst nvd_iscsi,g0            # Load istats diag index
        mov     r6,g4                   # Load session address
        call    e$storeN5DataEntry      # Load tgd into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space

# Write Connection Structure to NVRAM
        ld      40(r6),r6               # Get Connection pointer
        cmpobe  0,r6,.esn_307           # Jif no Connections
.esn_303:
        ld      (r6),g4                 # Point to connection addr
        ldconst nvd_iconn,g0            # Load istats diag index
        call    e$storeN5DataEntry      # Load tgd into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
        ld      (r6),r6                 # Move to next Connection
        b       .esn_303

.esn_307:
        ld      (r9),r9                # Move to next session
        b       .esn_302
.esn_308:
        addo    1,r8,r8                 # Increment tgd index
        cmpobne r7,r8,.esn_301          # Jif not the last tgd slot
# Write IDDs
        ldconst MAXPORTS,r7             # Load number of possible idds
        ldconst 0,r8                    # Load index into iddx
        ldconst 0,r9                    # Load index into iddx
.esn_311:
#        ld      gIdx        # get idd[r8][r9]
c       r10 = getIdd((UINT16)r8,(UINT16)r9);
#c fprintf(stderr,"IDD[%d][%d]\n",(UINT16)r8,(UINT16)r9);
        cmpobe  0,r10,.esn_312          # Jif NULL
        ldconst nvd_idd,g0              # Load IDD diag index
        mov     r10,g4                  # Load IDD address
        call    e$storeN5DataEntry      # Load IDD into nvram
.esn_312:
        addo    r9,1,r9                 # Bump counter
        cmpobne r9,r7,.esn_311          # Jif more idds

        addo   r8,1,r8                  # Bump counter
        cmpobe r8,r7,.esn_315           # Jif done
        ldconst 0,r9                    # Load index into iddx
        b       .esn_311                # Loop back
.esn_315:
.endif  # FRONTEND
# ISCSI_CODE
#
# --- Save the ICIMT directory to NVRAM
#
        ldconst nvd_icimt_dir,g0        # Load icimt dir index
        lda     I_CIMT_dir,g4           # Load icimt dir addr
        call    e$storeN5DataEntry      # Load icimt into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
#
# --- Save the ICIMTs, their valid trace logs, associated TMTs, TLMTs, ISMTs,
# --- LTMTs and LSMTs to NVRAM
#
        mov     g4,r6                   # r6 = ptr into icimt dir
        ldconst ICIMTMAX,r3             # Load max icimts
.esn_320:
        ld      (r6),r4                 # Load icimt address
        cmpobe  0,r4,.esn_400           # Jif not an icimt address
#
# --- Valid icimt, store icimt structure into NVRAM
#
        ldconst nvd_icimt,g0            # Load icimt index
        mov     r4,g4                   # Load icimt address
        call    e$storeN5DataEntry      # Load icimt into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
#
# --- Store valid trace logs to NVRAM
#
        ld      ici_begtr(r4),g8        # Load pointer to trace log
        cmpobe  0,g8,.esn_330           # Jif invalid pointer to trace log
        ldconst nvd_itrc,g0             # Load trace diag index
        mov     r7,g4                   # Load trace log address
        ld      ici_curtr(r4),g9        # Load current pointer in trace
        ld      ici_endtr(r4),g10       # Load end pointer in trace
        call    e$storeN5Trace          # Load trace log to nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
#
# --- Store valid TMT and associated LTMT to NVRAM
#
.esn_330:
        ld      ici_tmtQ(r4),r8         # Load tmt starting point
.esn_340:
        cmpobe  0,r8,.esn_365           # Jif invalid tmt address
#
        ldconst nvd_tmt,g0              # Load tmt diag index
        mov     r8,g4                   # Load tmt address
        call    e$storeN5DataEntry      # Load tmt into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
#
# --- Store valid LTMT to NVRAM
#
        ld      tm_ltmt(r8),r9          # Load ltmt address
        cmpobe  0,r9,.esn_360           # Jif invalid ltmt address
        ldconst nvd_ltmt,g0             # Load ltmt diag index
        mov     r9,g4                   # Load ltmt address
        call    e$storeN5DataEntry      # Load ltmt into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
#
# --- Store valid LSMT to NVRAM
#
        ld      ltmt_seshead(r9),r10    # Load lsmt address
.esn_350:
        cmpobe  0,r10,.esn_360          # Jif invalid lsmt
#
        ldconst nvd_lsmt,g0             # Load lsmt diag index
        mov     r10,g4                  # Load lsmt address
        call    e$storeN5DataEntry      # Load lsmt into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
        ld      lsmt_link(r10),r10      # Load next lsmt
        b       .esn_350
#
.esn_360:
        ld      tm_link(r8),r8          # Load next tmt
        b       .esn_340
#
# --- Store valid TLMTs and associated ISMTs to nvram
#
.esn_365:
        ld      ici_actqhd(r4),r8       # Load tlmt starting point
#
.esn_370:
        cmpobe  0,r8,.esn_400           # Jif invalid tlmt address
#
        ldconst nvd_tlmt,g0             # Load tlmt diag index
        mov     r8,g4                   # Load tlmt address
        call    e$storeN5DataEntry      # Load tlmt into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
#
# --- Store valid ISMTs to nvram
#
        ld      tlm_Shead(r8),r9        # Load ismt starting point
.esn_380:
        cmpobe  0,r9,.esn_390           # Jif invalid ismt address
#
        ldconst nvd_ismt,g0             # Load ismt diag index
        mov     r9,g4                   # Load ismt address
        call    e$storeN5DataEntry      # Load ismt into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
        ld      ism_flink(r9),r9        # Load next ismt
        b       .esn_380
#
.esn_390:
        ld      tlm_flink(r8),r8        # Load next tlmt
        b       .esn_370
#
.esn_400:
        lda     4(r6),r6                # Point to next icimt
        subo    1,r3,r3                 # Decrement counter
        cmpobne 0,r3,.esn_320           # Jif not done looking for icimts
.endif  # FRONTEND
#
# --- Save ISP Request and Response Queues to NVRAM
#
        ldconst nvd_irqq0,g0            # Load isp request q diag index
        ldconst 0,r6                    # Load ispstr  index
.esn_410:
        ld      ispstr[r6*4],r7         # Load isp struct pointer
        cmpobe  0,r7,.esn_440           # Jif invalid ptr
        ld      ispreqque(r7),g4        # Load isp request q ptr
        cmpobe  0,g4,.esn_420           # Jif invalid ptr
        ldl     (g4),g8                 # Load BEGIN and IN
        ld      qc_end(g4),g10          # Load END
        call    e$storeN5Trace          # Load request queue into nvram
.esn_420:
        addo    1,g0,g0                 # Increment diag index
        ld      ispresque(r7),g4        # Load isp response q ptr
        cmpobe  0,g4,.esn_430           # Jif invalid ptr
        ldl     (g4),g8                 # Load BEGIN and IN
        ld      qc_end(g4),g10          # Load END
        call    e$storeN5Trace          # Load response queue into nvram
.esn_430:
        addo    1,g0,g0                 # Increment diag index
        b       .esn_450
.esn_440:
        addo    2,g0,g0                 # Increment diag index
.esn_450:
        addo    1,r6,r6                 # Increment ispstr index
        cmpobne MAXISP,r6,.esn_410      # Jif more ispstr elements
#
# --- Save the LLDMT directory to NVRAM
#
        ldconst nvd_lldmt_dir,g0        # Load lldmt diag index
        lda     dlm_lldmt_dir,g4        # Load lldmt dir addr
        call    e$storeN5DataEntry      # Load lldmt dir into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
#
# --- Save the LLDMTs and associated DTMTs and TPMTs to NVRAM
#
        ldconst MAXISP,r7               # Load most lldmts
        ldconst 0,r8                    # Load lldmt table index
.esn_460:
        ld      dlm_lldmt_dir[r8*4],r6  # Load lldmt
        cmpobe  0,r6,.esn_500           # Jif invalid lldmt addr
#
        ldconst nvd_lldmt,g0            # Load lldmt diag index
        mov     r6,g4                   # Load lldmt address
        call    e$storeN5DataEntry      # Load lldmt into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
#
# --- Save the DTMTs and associated TPMTs to NVRAM
#
        ld      lldmt_dtmthd(r6),r9     # Load first dtmt
.esn_470:
        cmpobe  0,r9,.esn_500           # Jif no more dtmts
#
        ldconst nvd_dtmt,g0             # Load dtmt diag index
        mov     r9,g4                   # Load dtmt address
        call    e$storeN5DataEntry      # Load dtmt into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
#
# --- Save any valid TPMTs to NVRAM
#
        ld      dtmt_tpmthd(r9),r10     # Get first tpmt from dtmt
.esn_480:
        cmpobe  0,r10,.esn_490          # Jif invalid tpmt
#
        ldconst nvd_tpmt,g0             # Load tpmt diag index
        mov     r10,g4                  # Load tpmt address
        call    e$storeN5DataEntry      # Load tpmt into nvram
        cmpobe  0,g4,.esn_1000          # Jif out of nvram space
        ld      tpm_link(r10),r10       # Get next tpmt
        b       .esn_480
#
.esn_490:
        ld      dtmt_link(r9),r9        # Get next dtmt
        b       .esn_470
#
.esn_500:
        addo    1,r8,r8                 # Increment lldmt table index
        cmpobne r7,r8,.esn_460          # Jif last available lldmt
#
.esn_1000:
#
# --- Complete NVRAM header - length and checksum
#
        mov     g5,r5                   # Save nvram pointer
        subo    r13,g5,r4               # Load total pt5 length
        st      r4,word_length          # Load length for transfer
        lda     word_length,g4          # Load src addr
        lda     nv5h_len(r12),g5        # Load nvram addr
        ldconst 4,g3                    # Load length size
c       memcpy((void*)g5, (void*)g4, g3);   # Transfer total pt5 length into header
#
c       g0 = MSC_CRC32((void *)r13,r4)           # Calculate CRC across pt5 data
        st      g0,word_length          # Load CRC for transfer
        lda     nv5h_crc(r12),g5        # Load nvram addr
        lda     word_length,g4          # Load src addr
        ldconst 4,g3                    # Load crc length
c       memcpy((void*)g5, (void*)g4, g3);   # Transfer pt5 CRC into header
#
# --- Zero pad the remainder of nvram part 5
#
        ldconst NVSRAMP5START+NVSRAMP5SIZ,r6  # Load end of part 5 address
        ldconst 0,r7                    # Load zero
.esn_1010:
        cmpobe  r6,r5,.esn_1020         # Jif we're at the end
        stob    r7,(r5)                 # Store a zero
        addo    1,r5,r5                 # Increment nvram pointer
        b       .esn_1010
.esn_1020:
c       r4 = NVSRAMP5START%getpagesize();
c       r3 = msync((void *)(NVSRAMP5START - r4), NVSRAMP5SIZ + r4, MS_SYNC);
c       if (r3 != 0) fprintf(stderr, "e$saveNVRAM5:  msync failed, errno = %d\n", errno);
#
        ret
#
#******************************************************************************
#
#  NAME: e$storeN5EntryHdr
#
#  PURPOSE:
#
#       To provide e$saveNVRAM5 with a quick way to store the entry header of
#       any part 5 NVRAM diagnostic data entry.
#
#  DESCRIPTION:
#
#       Given the entry type and the address of the diagnostic data, this
#       routine will store the ascii label using part5_labels table, then
#       store the size of the data using part5_dlengths table, and then will
#       store the address of the data from the input.  Since all stores to
#       NVRAM must be byte-by-byte transfers, a local variable named
#       word_length will be used.
#
#  CALLING SEQUENCE:
#
#       call e$storeN5EntryHdr
#
#  INPUT:
#
#       g0 - Entry type
#       g4 - Data address
#       g5 - NVRAM address
#
#  OUTPUT:
#
#       g3 - Length of diagnostic data
#       g4 - Return 0 if we are at the end of nvram space
#       g5 - New NVRAM address
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
e$storeN5EntryHdr:
        mov     g4,r14                  # Save g4
#
        mov     g5,r5                   # r5 - addr into NVRAM
#
# --- Store ascii label into NVRAM pt 5
#
        mulo    nvd_lab_size,g0,r3      # Index into entry label table
        lda     part5_labels(r3),g4     # Load label addr
        lda     nv5e_id(r5),g5          # Load nvram addr
        ldconst nvd_lab_size,g3         # Load label size
        call    e$n5bytecpy             # Transfer defrag trace label
        cmpobe  0,g4,.ese_1000          # Jif out of nvram space
#
# --- Store data address into NVRAM pt 5
#
        st      r14,word_length         # Load address into local var
        lda     word_length,g4          # Load local var addr
        lda     nv5e_addr(r5),g5        # Load dest nvram addr
        ldconst 4,g3                    # Load word size
        call    e$n5bytecpy             # Transfer data addr to nvram
        cmpobe  0,g4,.ese_1000          # Jif out of nvram space
#
# --- Store data length into NVRAM pt 5
#
        mulo    4,g0,r3                 # Index into data length table
        ld      part5_dlengths(r3),r4   # Load data length
        addo    15,r4,r6                # Pad the length to even quad address
        andnot  0x0F,r6,r6              # Set low nibble to zero
        st      r6,word_length          # Store in local variable
        lda     word_length,g4          # Load src address
        lda     nv5e_len(r5),g5         # Load dest NVRAM address
        ldconst 4,g3                    # Load word size
        call    e$n5bytecpy             # Transfer flight recorder length
        cmpobe  0,g4,.ese_1000          # Jif out of nvram space
#
        addo    nv5e_size,r5,g5         # Adjust pointer into NVRAM
        mov     r4,g3                   # Return diag data length
#
        mov     r14,g4                  # Restore g4
.ese_1000:
        ret
#
#******************************************************************************
#
#  NAME: e$storeN5DataEntry
#
#  PURPOSE:
#
#       To provide e$saveNVRAM5 with a quick way to store some diagnostic data
#       into the part 5 NVRAM diagnostic area.
#
#  DESCRIPTION:
#
#       This routine will store the entry header, the diagnostic data for a
#       a given diagnostic data type.  The routine will return the new nvram
#       pointer after the storage takes place.
#
#  CALLING SEQUENCE:
#
#       call e$storeN5DataEntry
#
#  INPUT:
#
#       g0 - Diagnostic data type
#       g4 - Diagnostic data address
#       g5 - NVRAM address
#
#  OUTPUT:
#
#       g4 - Return 0 if we are at the end of nvram space
#       g5 - New NVRAM address
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
e$storeN5DataEntry:
#
        mov     g3,r9                   # Save g3
#
        call    e$storeN5EntryHdr       # Load pt 5 entry header
        cmpobe  0,g4,.esd_1000          # Jif out of nvram space
        call    e$n5bytecpy             # Transfer diag data to nvram
        addo    g3,g5,g5                # Increment nvram pointer
        cmpobe  0,g4,.esd_1000          # Jif out of nvram space
        call    e$padToQuad             # Pad the nvram to an even quad addr
#
.esd_1000:
        mov     r9,g3                   # Restore g3
        ret
#
#******************************************************************************
#
#  NAME: e$storeN5Trace
#
#  PURPOSE:
#
#       To provide e$saveNVRAM5 with a way to store the latest N bytes of a
#       trace (flight recorder, mrp trace, etc) into the part 5 NVRAM
#       diagnostic area where N is the value loaded into part5_dlengths table.
#
#  DESCRIPTION:
#
#       This routine will store the trace entries into NVRAM and return the
#       new NVRAM pointer to the caller.
#
#  CALLING SEQUENCE:
#
#       call e$storeN5Trace
#
#  INPUT:
#
#       g0  - Diagnostic data type
#       g4  - Diagnostic data address
#       g5  - NVRAM address
#       g8  - Trace BEGIN address
#       g9  - Trace IN address
#       g10 - Trace END address
#
#  OUTPUT:
#
#       g4 - Return 0 if we are at the end of nvram space
#       g5 - New NVRAM address
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
e$storeN5Trace:
        call    e$storeN5EntryHdr       # Load pt 5 entry header
        cmpobe  0,g4,.est_1000          # Jif out of nvram space
        mov     g3,r7                   # Save data length
#
        subo    g3,g9,r6                # Subtract data length from IN ptr
        cmpobge r6,g8,.est_10           # Jif we are not before IN ptr
#
# --- Load scattered entries into NVRAM
#
        subo    r6,g8,g3                # How many entries before
        subo    g3,g10,g4               # Load start addr
        call    e$n5bytecpy             # Transfer first chunk of trace
        addo    g3,g5,g5                # Increment nvram pointer
        cmpobe  0,g4,.est_1000          # Jif out of nvram space
#
        mov     g8,g4                   # Load IN ptr for src addr
        subo    g3,r7,g3                # Size of remaining trace chunk
        call    e$n5bytecpy             # Transfer second chunk of trace
        addo    g3,g5,g5                # Increment nvram pointer
        cmpobe  0,g4,.est_1000          # Jif out of nvram space
        b       .est_20                 # Continue saving diag data
#
# --- Load contiguous entries into NVRAM
#
.est_10:
        mov     r6,g4                   # Load src addr of trace
        call    e$n5bytecpy             # Transfer contiguous entries
        addo    g3,g5,g5                # Increment nvram pointer
        cmpobe  0,g4,.est_1000          # Jif out of nvram space
.est_20:
        call    e$padToQuad             # Pad the nvram to an even quad addr
.est_1000:
        ret
#
#******************************************************************************
#
#  NAME: e$padToQuad
#
#  PURPOSE:
#
#       To provide e$saveNVRAM5 a way to pad the NVRAM to an even quad
#       boundary.
#
#  DESCRIPTION:
#
#       This function will pad the NVRAM at address given in g5 to an even
#       16 byte boundary with zeros.  It will also increment g5 to the new
#       address.
#
#  CALLING SEQUENCE:
#
#       call e$padToQuad
#
#  INPUT:
#
#       g5 - NVRAM address
#
#  OUTPUT:
#
#       g5 - New NVRAM address
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
e$padToQuad:
#
        mov     g3,r11                  # Save g3
        mov     g4,r12                  # Save g4
#
        ldconst 0,r5                    # Load the zero for padding
        and     0x0F,g5,g3              # Mask all but the low nibble
        cmpobe  0,g3,.epq10             # Jif nvram addr is at quad boundary
#
        subo    g3,16,g3                # How many zeros to pad
        lda     zero_pad,g4             # Load local zero string
        call    e$n5bytecpy             # Transfer zeros to pad
        addo    g3,g5,g5                # Increment the nvram pointer
.epq10:
        mov     r12,g4                  # Restore g4
        mov     r11,g3                  # Restore g3
#
        ret
#
#**********************************************************************
#
#  NAME: e$n5bytecpy
#
#  PURPOSE:
#
#       To provide e$saveNVRAM5 a mechanism of copying from one place
#       in memory to NVRAM part 5 diagnostic area.
#
#  DESCRIPTION:
#
#       The source string is copied to the address of the destination
#       pointer.  A check is put in place to ensure that nvram writes
#       don't extend past the end of nvram space.  Another check is
#       put in place to ensure that we don't read from illegal memory.
#
#  CALLING SEQUENCE:
#
#       call    e$n5bytecpy
#
#  INPUT:
#
#       g3 = length
#       g4 = source ptr
#       g5 = destination ptr
#
#  OUTPUT:
#
#       g4 = If we ran out of NVRAM space, = 0
#       g3 = Bytes transferred
#
#  REGS DESTROYED:
#
#       None
#
#**********************************************************************
#
e$n5bytecpy:
#
        mov     g0,r10                  # Save g0
        mov     g3,r11                  # Save g3
        movl    g4,r12                  # Save g4 & g5
#
        ldconst NVSRAMP5START+NVSRAMP5SIZ,r4  # Load the end of nvram space
#
# --- Check for illegal starting address
#
        mov     g4,g0                   # Load starting addr
        call    e$addrCheck             # Check for legality
        cmpobe  FALSE,g0,.ebc_50        # Jif illegal addr, don't store
#
        addo    g4,g3,g0                # Load ending addr
        subo    1,g0,g0                 # Decrement for last address
        call    e$addrCheck             # Check for legality
        cmpobe  FALSE,g0,.ebc_50        # Jif illegal addr, don't store
.ebc_10:
        cmpobe  0,g3,.ebc_100           # Jif length is zero
        cmpoble r4,g5,.ebc_80           # Jif we are past nvram space
        subo    1,g3,g3                 # Decrement length
        ldob    (g4),r3                 # Load byte
        stob    r3,(g5)                 # Store byte
        addo    1,g4,g4                 # Increment source ptr
        addo    1,g5,g5                 # Increment destination ptr
        b       .ebc_10                 # Back up to top of loop
#
# --- If we failed the address check, fill the nvram space with zeros
#
.ebc_50:
        ldconst 0,r3                    # Preload a zero
.ebc_60:
        cmpobe  0,g3,.ebc_100           # Jif length is zero
        cmpoble r4,g5,.ebc_80           # Jif we are past nvram space
        subo    1,g3,g3                 # Decrement length
        stob    r3,(g5)                 # Store a zero byte
        addo    1,g5,g5                 # Increment destination ptr
        b       .ebc_60                 # Back up to the top of loop
#
# --- If we are past the end of nvram space, return g4 = 0, g3 = bytes
#     transferred.
#
.ebc_80:
        ldconst 0,r12                   # Load zero in g4
        subo    g3,r11,r11              # Load bytes transferred in g3
.ebc_100:
        mov     r10,g0                  # Restore g0
        mov     r11,g3                  # Restore g3
        movl    r12,g4                  # Restore g4 & g5
        ret
#
#**********************************************************************
#
#  NAME: e$addrCheck
#
#  PURPOSE:
#
#       To provide e$n5bytecpy with a mechanism of verifying if an
#       address is within legal bounds.
#
#  DESCRIPTION:
#
#       The address given is checked to see if it lies within legal
#       iram, dram, nvram or flash address ranges.  If not, it will
#       return a FALSE in g0.
#
#  CALLING SEQUENCE:
#
#       call    e$addrCheck
#
#  INPUT:
#
#       g0 = address to be checked
#
#  OUTPUT:
#
#       g0 = TRUE if legal, FALSE if illegal
#
#  REGS DESTROYED:
#
#       None
#
#**********************************************************************
#
e$addrCheck:
#
# --- Load the values for address checking
#
c       r5 = (ulong)&_init;
c       r6 = (ulong)&_end;
c       r7 = (unsigned long)SHARELOC;
.ifdef FRONTEND
c       r8 = (unsigned long)SHARELOC + SIZE_FE_LTH;
.endif # FRONTEND
.ifdef BACKEND
c       r8 = (unsigned long)SHARELOC + SIZE_BE_LTH;
.endif # BACKEND
        ldconst NVSRAM,r9               # Load start of NVRAM
        ldconst NVSRAMSIZ,r10           # Load size of NVRAM
        addo    r9,r10,r10              # Determine end of NVRAM
        ldconst SIZE_4MEG,r3            # Load size of FLASH
        addo    r3,r10,r10              # Determine the end of NVRAM
                                        #  and flash
#
# --- Check for legal address
#
        cmpobl  g0,r5,.eac_80           # Jif addr is before IRAM
        cmpobl  g0,r6,.eac_90           # Jif addr is in IRAM
        cmpobl  g0,r7,.eac_80           # Jif addr is before DRAM
        cmpobl  g0,r8,.eac_90           # Jif addr is in DRAM
        cmpobl  g0,r9,.eac_80           # Jif addr is before NVRAM
        cmpobl  g0,r10,.eac_90          # Jif addr is in NVRAM or flash
c   if (g0 >= (UINT32)&local_memory_start && g0 <= (UINT32)&local_memory_start + PRIVATE_SIZE) {
        b       .eac_90                 # Jif addr is in local non-shared memory
c   }
#
# --- If it passes all the checks, then it's past the end of flash and
#     should return a false.  In this case, fall down into .eac_90.
#
.eac_80:
c fprintf(stderr, "%se$addrCheck - Invalid address 0x%08lX\n", L_MsgPrefix, g0);
        ldconst FALSE,g0                # Return a FALSE
        b       .eac_100
.eac_90:
        ldconst TRUE,g0                 # Return a TRUE
.eac_100:
        ret
#
#
# --- error jumps -------------------------------------------------------------
#
.kerr17:
        st      r3,TRAPADDR_R3
        ldconst kerr17,r3
        b       .errtrap
#
# --- Process-level error codes
#
.err00:
        st      r3,TRAPADDR_R3
        ldconst err00,r3
        b       .errtrap

.if     SFTDEBUG
.err04:
        st      r3,TRAPADDR_R3
        ldconst err04,r3
        b       .errtrap
.endif # SFTDEBUG

.err06:
        st      r3,TRAPADDR_R3
        ldconst err06,r3
        b       .errtrap

error08:
        st      r3,TRAPADDR_R3
        ldconst err08,r3
        b       .errtrap
#
.ifdef BACKEND
.err10:
        st      r3,TRAPADDR_R3
        ldconst err10,r3
        b       .errtrap
.endif  /* BACKEND */
#
error11:
        st      r3,TRAPADDR_R3
        ldconst err11,r3
        b       .errtrap
#
.ifdef BACKEND
.err18:
        st      r3,TRAPADDR_R3
        ldconst err18,r3
        b       .errtrap
.endif  /* BACKEND */
#
.ifdef FRONTEND
.err19:
        st      r3,TRAPADDR_R3
        ldconst err19,r3
        b       .errtrap
.endif  /* FRONTEND */
#

.ifdef FRONTEND
.err26:
        st      r3,TRAPADDR_R3
        ldconst err26,r3
        b       .errtrap
.endif /* FRONTEND */
#
error31:
        st      r3,TRAPADDR_R3
        ldconst err31,r3
        b       .errtrap
#
.err33:
        st      r3,TRAPADDR_R3
        ldconst err33,r3
        b       .errtrap
#
errorLinux:
        st      r3,TRAPADDR_R3
        ldconst TRAPADDR,r3             # place to store regs
c       errTrapAddr = r3;
c       /* Copy the g and r registers to the TRAP location */
c       memcpy((void*)(r3+0x10), (void*)errGreg[15], 0x40);
c       memcpy((void*)(r3+0x50), (void*)errGreg, 0x40);
c       r3 = errLinux + errLinuxSignal; # Linux error trap
        b       .errtrap                # Will not return.
#
#******************************************************************************
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#

