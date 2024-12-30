# $Id: nvabe.as 147915 2010-09-21 22:16:43Z m4 $
#**********************************************************************
#
#  NAME: nvabe.as
#
#  PURPOSE:
#       To provide a means of handling the NVA records in the BE NVRAM.
#
#  FUNCTIONS:
#       M$p3init        - init Part 3 NVA records
#       M$p3chksumchk   - Check Part 3 NVRAM checksum
#       M$p3outstanding - Number of outstanding Part 3 NVA records
#       M$p3clear       - Clear the Part 3 NVRAM
#       M$ap3nva        - Allocate and fill in a Part 3 NVA Record
#       M$rp3nva        - Release a Part 3 NVA record
#
#  Copyright (c) 1996-2010 XIOtech Corp. All rights reserved.
#
#**********************************************************************
#
# --- global function declarations ------------------------------------
#
        .globl  NVA_Init                # Initialize NVA
        .globl  M$p3init                # Init P3 NVA control, maps, and NVRAM
        .globl  M$p3clear               # Clear P3 NVA
        .globl  M$ap3nva                # Allocate and fill in a P3 NVA record
        .globl  M$rp3nva                # Release a P3 NVA record
        .globl  O$recoverp3             # Resync NVA records in NVRAM
        .globl  O$recoverp4             # Resync SNVA records in NVRAM
        .globl  O$markraidscan          # Parity scan all RAIDs
        .globl  NVA_SetReSyncAStatus    # Set AStatus RDD R5 Resync Flag
        .globl  NVA_ClearReSyncAStatus  # Clear AStatus RDD R5 Resync Flag
#
# --- private function declarations -----------------------------------
#
        .globl  M$p3chksumchk           # Verify Part 3 NVRAM checksum
#
# --- global usage data definitions -----------------------------------
#
        .globl  P3_nvac                 # RAID NVA Control structure
#
# --- local usage data definitions ------------------------------------
#
        .data
        .align  2
P3_nvac:
        .space  nvacsize,0              # NVRAM Part 3 RAID NVA Control struct
#
# --- executable code (low usage) -------------------------------------
#
#**********************************************************************
#
#  NAME: NVA_Init
#
#  PURPOSE:
#       To provide a means of initializing this module for this processor
#
#  DESCRIPTION:
#       Handle BE specific requirements.
#
#  CALLING SEQUENCE:
#       call    NVA_Init
#
#  INPUT:
#       None
#
#  OUTPUT:
#       None
#
#**********************************************************************
#
        .text
#
# Re-initialize the NVA Allocation bit map and the control structures
#
# void  NVA_ReInitialize ()
# {
#   Free the previous allocation bit map if any...
#   Allocate new bit map
#   Initialize the control structure with new values.
# }
# C access
# void NVA_ReInitialize (void);
        .globl  NVA_ReInitialize
NVA_ReInitialize:
        lda     P4_nvac,r3              # Get NVAC control address
        ld      nc_mapbase(r3),g0
c       s_Free(g0, NUM_OF_P4_NVA_WK/8, __FILE__, __LINE__);
        lda     P3_nvac,r3              # Get NVAC control address
        ld      nc_mapbase(r3),g0
c       s_Free(g0, NUM_OF_P3_NVA_WK/8, __FILE__, __LINE__);
#
NVA_Init:
        call    M$p3init                # Initialize part 3 NVA records
        call    M$p4init                # Initialize part 4 NVA records
        ret
#
#**********************************************************************
#
#  NAME: M$ap3nva
#
#  PURPOSE:
#       To provide a common means of assigning and initializing a NVA
#       entry for an RRP write operation.
#
#  DESCRIPTION:
#       A NVA entry is assigned and initialized with information taken
#       directly from the RRP.  The address of the NVA is placed into
#       the primary ILT.
#
#       The actual construction of the NVA is undertaken with interrupts
#       disabled to minimize the chance of data corruption during a
#       loss of power.
#
#  CALLING SEQUENCE:
#       call    M$ap3nva
#
#  INPUT:
#       g13 = RRP
#       g14 = primary ILT
#
#  OUTPUT:
#       il_w5(g14) = NVA
#
#**********************************************************************
#
M$ap3nva:
        mov     g0,r12                  # Save register g0
        mov     g1,r13                  # Save register g1
        mov     g2,r14                  # Save register g2
#
        lda     P3_nvac,g1              # Get NVA control structure address
        ldconst pcnvawait,g2            # Get PCB wait state
#
# --- Construct data for NVA entry - r4,5,6,7 has the NVA
#
        ldconst P3_ALLOC,r4
        st      r4,nvr_part             # To indicate that it is NVRAM p-3 alloc
        ldos    rr_rid(g13),r4          # Get RAID ID
        ld      rr_rlen(g13),r5         # Get sector count
        ldl     rr_rsda(g13),r6         # Get LSDA
#
        balx    m$anva,r15              # Branch to common code
                                        #  Returns with g0 = NVA addr
#
.ifdef NVA_DEBUG
c       fprintf(stderr,"The Offset of NVA returned by M$ap3nva:%x\n",g0);
.endif #NVADEBUG
        st      g0,il_w5(g14)           # Link NVA to primary ILT
        mov     r12,g0                  # Restore registers
        mov     r13,g1
        mov     r14,g2
        ret
#
#**********************************************************************
#
#  NAME: M$rp3nva
#
#  PURPOSE:
#       To provide a common means of releasing a resync NVA structure residing
#       within NVRAM.
#
#  DESCRIPTION:
#       If a power fail condition exists, this routine stalls on that
#       condition.  The NVA entry is zeroed out with the checksum for
#       the previous data being backed out.  The allocation bitmap for
#       this entry is updated to reflect the availability of this entry.
#
#       Since the NVRAM is implemented as an 8-bit device, byte reads
#       and writes must be used to access NVRAM.
#
#  CALLING SEQUENCE:
#       call    M$rp3nva
#
#  INPUT:
#       g0 = address of NVA record to be released
#
#  OUTPUT:
#       g0      0 = Good
#
#**********************************************************************
#
M$rp3nva:
        lda     P3_nvac,r15             # Get NVAC control structure address
        ldconst pcnvawait,r14           # Get PCB wait status
        ldconst P3_REL,r3               # Indicate that P3 nva has to be released
        st      r3,p3rel
# nva Offset
        st      g0,rel_offset           # Save the Offset
#       g0 = g0+ Local P3 Base address
        ldconst nvabasesiz,r3
        ld      nc_nvarec(r15),r4
        subo    r3,r4,r4
        addo    r4,g0,g0
        b       m$rnva                  # Branch to common release code
#
#**********************************************************************
#
#  NAME: M$p3init
#
#  PURPOSE:
#       To provide a common means of initializing the part 3 NVA records.
#
#  DESCRIPTION:
#       The checksum is cleared and the bitmap for NVAC
#       allocations is initialized to zeroes indicating that all NVAC
#       entries are available for assignment.
#
#  CALLING SEQUENCE:
#       call    M$p3init
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       None.
#
#**********************************************************************
#
M$p3init:
#
# --- Setup NVA Control values
#
        lda     P3_nvac,r15             # Get NVAC pointer
#
c       r3 = NUM_OF_P3_NVA_WK;          # Number of NVA Entries in P3 NVRAM
        st      r3,nc_cur(r15)          #  to current
        st      r3,nc_min(r15)          #  to minimum
#
        lda      p_localBENVRAM_BASE,r4 # Get base of local NVRAM Part 3
                                        # Part 3 address is nothing but the
                                        # starting address
        lda     nv_csum(r4),r6          # Get address of checksum
        st      r6,nc_csum(r15)         # Set up checksum ptr
#
        ldconst nvabasesiz,r3           # Get header size
        addo    r3,r4,r4                # Increment past header
        st      r4,nc_nvarec(r15)       # Save ptr to first NVA record
#
# --- Allocate assignment bitmap
#
# 1 entry per bit => (num rec / 32 bits) * 4 bytes per word
c       r3 = s_MallocC(NUM_OF_P3_NVA_WK/8, __FILE__, __LINE__);
        st      r3,nc_mapbase(r15)      # Set up bitmap base
        st      r3,nc_mapptr(r15)       # Set up bitmap ptr
        ret
#
#**********************************************************************
#
#  NAME: M$p3clear
#
#  PURPOSE:
#       To provide a common means of clearing Part 3 of the NVRAM.
#
#  DESCRIPTION:
#       The checksum is cleared and the bitmap for NVAC
#       allocations is initialized to zeroes indicating that all NVAC
#       entries are available for assignment.
#
#       This should not be called until the NVA control structure has been
#       initialized.
#
#  CALLING SEQUENCE:
#       call    M$p3clear
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
# void NVA_ClearP3(void);
        .globl  NVA_ClearP3
NVA_ClearP3:
M$p3clear:
        lda     P3_nvac,r15             # Get NVA control structure address
c       r13 = gMPNvramP3Size-sizeof(NVA_HEADER); # Get number of bytes
c       r12 = NUM_OF_P3_NVA_WK;         # Get max number of NVA records
c       r11 = TRUE;                     # To indicate that P-3 clear.
        b       m$nvaclear
#
#**********************************************************************
#
#  NAME: M$p3chksumchk
#
#  PURPOSE:
#       Verify the checksum of the Part 3 NVA records.
#       The user must copy the NVRAM data into a temporary DRAM buffer and
#       pass a pointer to that buffer in g0.
#
#  DESCRIPTION:
#       Each word within the NVA area is summed up and compared to
#       the checksum word in the PART II header. If the checksum is good
#       g0 is returned ecok. If the checksum is bad, the NVA records
#       are cleared in NVRAM. This also updates the SNVA allocation map.
#
#       This routine sets up some r registers for either Part 3 or 4 NVA records
#       then branches to common code.
#
#  CALLING SEQUENCE:
#       call    M$p3chksumchk
#
#  INPUT:
#       g0      address of the Part 3 data in DRAM - start of header
#
#  OUTPUT:
#       g0      ecok = good checksum
#               ecchecksum = bad checksum
#
#**********************************************************************
#
M$p3chksumchk:
c       r11 = gMPNvramP3Size;           # NVA records Maximum length
c       r12 = NUM_OF_P3_NVA_WK;         # Max number of NVA records
        lda     M$p3clear,r13           # Address of clearing routine
        lda     P3_nvac,r14             # NVA Control pointer
        b       m$chksumchk             # Branch to common code in define.as
#
#**********************************************************************
#
#  NAME: O$recoverp3
#
#  PURPOSE:
#       This function will syncronize the parity data
#       RAID 5 devices that have NVA records in part 3 NVRAM.
#       This is used at power on before any new ops start coming from
#       the server or being sent to the raid layer.
#
#  DESCRIPTION:
#       This function copies the NVA records from NVRAM into a temporary
#       DRAM buffer. It calls a function to verify the checksum and generate
#       the allocation map.
#
#       If the  checksum is good it starts a task to sync any unreleased NVA
#       records in the background. I/Os can now start going to the raid layer.
#
#       If the checksum is bad an error is logged and the buffer is released.
#       The NVRAM will be cleared in this case.
#
#  CALLING SEQUENCE:
#       call    O$recoverp3
#
#  INPUT:
#       None
#
#  OUTPUT:
#       g0      0 = good or ecchecksum
#
#**********************************************************************
#
O$recoverp3:
        PushRegs                        # Save all G registers (stack relative)
#
        ldconst mlepulse,g0
        call    O_logerror              # Let CCB know we're working...
#
        ld      gMMCFound,r4
        cmpobne 1,r4,.opr10             # Jump if no MicroMemory card found
#
# Read BE NVRAM part-3 contents from micromemory card.
#
        PushRegs(r4)                    # Save the G registers
        ldconst MICRO_MEM_BE_P3_START,g0
c       g1 = (UINT32)&p_localBENVRAM_BASE
c       g2 = gMPNvramP3Size;
        call    MM_Read                 # g0=src, g1=dest, g2=lth
        PopRegsVoid(r4)                 # Restore all G registers
#
.opr10:
#
# --- Verify checksum and build allocation map
#
c       g0 = (UINT32)&p_localBENVRAM_BASE; # Verify copy in ram
c       r15 = g0;
        call    M$p3chksumchk           # Verify checksum
        cmpobne ecok,g0,.opr100         # Jif bad checksum
#
# --- Don't resync if there are no entries to do
#
        lda     P3_nvac,r4              # Get current count
        ld      nc_cur(r4),r4
c       g1 = NUM_OF_P3_NVA_WK;
        cmpobne g1,r4,.opr50            # Jif there are NVA entries to do
#
        mov     0,g6                    # g6 = Number of Valid NVA Entries
        mov     0,g7                    # g7 = Number of NVA Entries Attempted
        mov     0,g8                    # g8 = Number of Failed Attempts
        call    o$log_resyncdone        # Add log entry that nothing was done
        ldconst ecok,g0                 # Set up the Return code again
        b       .opr1000                # Exit
#
# --- Work to be done - Set the AStatus in all RAIDs that have NVRAM Records,
#       and then start the Resync Process
#
.opr50:
c       g0 = (UINT32)&p_localBENVRAM_BASE + nvabasesiz
                                        # g1 = Number of records to search
        PushRegs(r8)                    # Save register contents
        call    NVA_SetReSyncAStatus    # Set the RDD Astatus for RAIDs in list
        PopRegsVoid(r8)                 # Restore registers
#
# --- Fork the raid 5 stripe sync task
#
        lda     o$sync_stripes,g0       # Fork sync task
        ldconst OINITDRVPRI,g1          # Set priority
        mov     r15,g2                  # DRAM addr of sync data
c       g3 = NUM_OF_P3_NVA_WK;          # Maximum number of entries
        lda     o$recoverp3_comp,g4     # Completion routine
c       CT_fork_tmp = (ulong)"o$sync_stripes";
        call    K$tfork
        ldconst ecok,g0                 # Reset the Return code to good
        b       .opr1000                # Jump to finish
#
# --- Checksum bad
#
.opr100:
        ldconst ALL_RDDS,g1             # Scan ALL drives
        call    O$markraidscan          # Start the parity scan on all drives
#
        ldconst enabe+enapart3,g0       # Set logging error code
        call    M$log_nvachecksum       # Log a bad checksum event
        ldconst ecchecksum,g0           # Set return code
#
# --- Restore g registers and return
#
.opr1000:
                                        # g0 is return code
        PopRegs                         # Restore g1 to g14 (stack relative)
        ret
#
#**********************************************************************
#
#  NAME: o$recoverp3_comp
#
#  PURPOSE:
#       This function will release all part 3 NVA records in NVRAM.
#
#  DESCRIPTION:
#       Each valid NVA entry is released.
#
#  CALLING SEQUENCE:
#       call    o$recoverp3_comp
#
#  INPUT:
#       g2      Pointer to DRAM copy of NVA records
#       g3      Max number of records
#
#  OUTPUT:
#       None.
#
#**********************************************************************
#
o$recoverp3_comp:
        PushRegs                        # Save all G registers (stack relative)

        mov     g3,r15                  # Save max number of records
        lda     nvabasesiz(g2),r8       # First NVA record in DRAM
        mov     r8,r7                   # Base addr of temp NVA buffer
        ld      P3_nvac+nc_nvarec,r6    # First NVA record in NVRAM
#
# --- Work is all done - Clear the AStatus in all RAIDs that have NVRAM Records
#
        mov     r8,g0                   # g0 = Pointer to NVA Records
        mov     r15,g1                  # g1 = Number of records to search
        call    NVA_ClearReSyncAStatus  # Clear In-Progress Flags and AStatus
#
# --- Top of loop to process each NVA record
#
.orc10:
        ld      nv_length(r7),r3        # Get length
        cmpobe  0,r3,.orc20             # This record not in use, skip it
#
        subo    r8,r7,r3                # Offset from start of temp NVA in DRAM
        addo    r6,r3,g0                # Add to start of NVA section in NVRAM
                                        # g0 points to NVA record to release
        ldconst nvabasesiz,r5
        addo    r5,r3,g0                # r3 has offset from start of NVA and and r5 has header size
        call    M$rp3nva                # Release the record
#
# --- Any more entries?
#
.orc20:
        subo    1,r15,r15               # Decrement entry count
        lda     nvasiz(r7),r7           # Point to next record
        cmpobne 0,r15,.orc10            # Jif more to do
#
# --- Return
#
        PopRegsVoid                     # Restore all G registers (stack relative)
        ret
#
#**********************************************************************
#
#  NAME: O$recoverp4
#
#  PURPOSE:
#       Sync the parity data for RAID 5 devices that have NVA records.
#       This is used at power on before any new ops start coming from
#       the server or being sent to the raid layer.
#
#  DESCRIPTION:
#       This function copies the NVA records from NVRAM into a temporary
#       DRAM buffer. It calls a function to verify the checksum and generate
#       the allocation map.
#
#       If the  checksum is good it starts a task to sync the stripes in the
#       background. I/Os can now start going to the raid layer.
#
#       If the checksum is bad an error is logged and the buffer is released.
#       The NVRAM will be cleared in this case.
#
#  CALLING SEQUENCE:
#       call    O$recoverp4
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       g0      0 = good or ecchecksum
#
#**********************************************************************
#
O$recoverp4:
        PushRegs                        # Save all G registers (stack relative)
#
        ldconst mlepulse,g0
        call    O_logerror              # Let CCB know we're working...
#
        ld      gMMCFound,r4
        cmpobne 1,r4,.opr410            # If the Micromemory card is not found, don't read it
#
#  Read BE NVRAM part-4 contents from micromemory card.
#
        PushRegs(r4)                    # Save the G registers
        ldconst MICRO_MEM_BE_P4_START,g0
c       g1 = (UINT32)&p_localBENVRAM_BASE + gMPNvramP3Size
c       g2 = gMPNvramP4Size;
        call    MM_Read                 # g0=src, g1=dest, g2=lth
        PopRegsVoid(r4)                 # Restore all G registers
#
.opr410:
#
# --- Verify checksum and build allocation map
#
c       g0 = (UINT32)&p_localBENVRAM_BASE + gMPNvramP3Size
c       r15 = g0;
        call    M$p4chksumchk           # Verify checksum
        cmpobne ecok,g0,.opr4100        # Jif bad checksum
#
# --- Don't resync if there are no entries to do
#
        lda     P4_nvac,r4              # Get current count
        ld      nc_cur(r4),r4
c       g1 = NUM_OF_P4_NVA_WK;
        cmpobne g1,r4,.opr450           # Jif there are NVA entries to do
#
        mov     0,g6                    # g6 = Number of Valid NVA Entries
        mov     0,g7                    # g7 = Number of NVA Entries Attempted
        mov     0,g8                    # g8 = Number of Failed Attempts
        call    o$log_resyncdone        # Add log entry that nothing was done
        ldconst ecok,g0                 # Set up the Return code again
        b       .opr41000               # Exit
#
# --- Work to be done - Set the AStatus in all RAIDs that have NVRAM Records,
#       and then start the Resync Process
#
.opr450:
c       g0 = (UINT32)&p_localBENVRAM_BASE + gMPNvramP3Size + nvabasesiz
                                        # g1 = Number of records to search
        PushRegs(r8)                    # Save register contents
        call    NVA_SetReSyncAStatus    # Set the RDD Astatus for RAIDs in list
        PopRegsVoid(r8)                 # Restore registers
#
# --- Fork the raid 5 stripe sync task
#
        lda     o$sync_stripes,g0       # Fork sync task
        ldconst OINITDRVPRI,g1          # Set priority
        mov     r15,g2                  # DRAM addr of sync data
c       g3 = NUM_OF_P4_NVA_WK;
        lda     o$recoverp4_comp,g4     # Completion routine
c       CT_fork_tmp = (ulong)"o$sync_stripes";
        call    K$tfork
        ldconst ecok,g0                 # Reset the Return code to good
        b       .opr41000               # Jump to finish
#
# --- Checksum bad
#
.opr4100:
        ldconst ALL_RDDS,g1             # Scan ALL RAIDs
        call    O$markraidscan          # Start the parity scan on all drives
#
        ldconst enabe+enapart4,g0       # Set logging error code
        call    M$log_nvachecksum       # Log a bad checksum event
        ldconst ecchecksum,g0           # Set return code
#
# --- Restore g registers and return
#
.opr41000:
                                        # g0 is return code
        PopRegs                         # Restore g1 to g14 (stack relative)
        ret
#
#**********************************************************************
#
#  NAME: o$recoverp4_comp
#
#  PURPOSE:
#       This function will release all part 4 NVA records in NVRAM.
#
#  DESCRIPTION:
#       Each valid NVA entry is released.
#
#  CALLING SEQUENCE:
#       call    o$recoverp4_comp
#
#  INPUT:
#       g2      Pointer to DRAM copy of NVA records
#       g3      Max number of records
#
#  OUTPUT:
#       None.
#
#**********************************************************************
#
o$recoverp4_comp:
        PushRegs                        # Save all G registers (stack relative)
        mov     g3,r15                  # Save Max number of records
        lda     nvabasesiz(g2),r8       # First NVA record in DRAM
        mov     r8,r7                   # Base addr of temp NVA buffer
        ld      P4_nvac+nc_nvarec,r6    # First NVA record in NVRAM
#
# --- Work is all done - Clear the AStatus in all RAIDs that have NVRAM Records
#
        mov     r8,g0                   # g0 = Pointer to NVA Records
        mov     r15,g1                  # g1 = Number of records to search
        call    NVA_ClearReSyncAStatus  # Clear In-Progress Flags and AStatus
#
# --- Top of loop to process each NVA record
#
.or4c10:
        ld      nv_length(r7),r3        # Get length
        cmpobe  0,r3,.or4c20            # This record not in use, skip it
#
        subo    r8,r7,r3                # Offset from start of temp NVA in DRAM
        addo    r6,r3,g0                # Add to start of NVA section in NVRAM
                                        # g0 points to NVA record to release
#nva Offset
# Offset from the start of this part-4 NVRAM in 'g0' is required for M$rp4nva
        ldconst nvabasesiz,r5
        addo r5,r3,g0                   # r3 has offset from start of NVA and r6 has
                                        # size of Header
        call    M$rp4nva                # Release the record
                                        # g0 points to nva record to release
#
# --- Any more entries?
#
.or4c20:
        subo    1,r15,r15               # Decrement entry count
        lda     nvasiz(r7),r7           # Point to next record
        cmpobne 0,r15,.or4c10           # Jif more to do
#
# --- Return
#
        PopRegsVoid                     # Restore all G registers (stack relative)
        ret
#
#**********************************************************************
#
#  NAME: o$sync_stripes
#
#  PURPOSE:
#       Synchronize all RAID 5 NVA records.
#
#  DESCRIPTION:
#       This sends a parity check IO command to the RAID layer for each NVA
#       record in DRAM.
#
#  CALLING SEQUENCE:
#       process call
#
#  INPUT:
#       g2 = Pointer to DRAM copy of NVA records
#       g3 = Max number of records
#       g4 = Callback function
#
#  OUTPUT:
#       None.
#
#**********************************************************************
#
o$sync_stripes:
        mov     g2,g5                   # Save copy of g2
        ldconst nvabasesiz,r3           # NVA header size
        addo    r3,g2,r15               # First NVA record in DRAM
        mov     g3,r12                  # Get max number of NVA entries
#
        movl    0,g6                    # Clear data collection counters
        movq    0,g8
        movt    0,g12
#
# --- Top of loop to process each NVA record
#
.oss10:
        ld      nv_length(r15),r3
        cmpobe  0,r3,.oss30             # This record not in use, skip it
#
# --- Sanity check the NVA record
#     Errors are not logged.
#     These NVA records will be cleared by the completion handler.
#
        addo    1,g6,g6                 # Increment valid NVA record counter
        ldos    nv_id(r15),r13          # Get RAID ID
        ldconst MAXRAIDS,r4
        cmpobge r13,r4,.oss30           # Jif RAID ID too big
#
        ld      R_rddindx[r13*4],r14    # r14 = RDD
        cmpobe  0,r14,.oss30            # Jif unassigned
#
        ldob    rd_type(r14),r3         # Get raid type
        cmpobne rdraid5,r3,.oss30       # Invalid Raid type - can only be 5
#
        ldos    rd_vid(r14),g0          # Get the VID
#
        PushRegs(r3)                    # Save registers
        call    DL_AmIOwner             # Do I own it?  g0 = T/F
        PopRegs(r3)
#
        cmpobe  FALSE,g0,.oss30         # Do next RDD if not owned
#
# --- Process an NVA record ---------------------------------------------------
#
c       g1 = get_ilt();                 # Allocate an ILT.
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif  # M4_DEBUG_ILT
c       g2 = get_rrp();                 # Allocate an RRP.
.ifdef M4_DEBUG_RRP
c CT_history_printf("%s%s:%u get_rrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif  # M4_DEBUG_RRP
c       ((ILT *)g1)->ilt_normal.w0 = g2; # Link to RRP in ILT.
        mov     g1,r10                  # r10 = ILT
        stos    r13,rr_rid(g2)          # Save RID
        mov     0,r3                    # Clear rr_sglptr field
        st      r3,rr_sglptr(g2)
        st      r3,rr_sglsize(g2)
        st      r3,rr_flags(g2)
        ldconst rrparitychk,r3          # Parity check function
        stos    r3,rr_func(g2)
        setbit  rrpchkcorrect,0,r3      # Correct Parity Option flag
        stob    r3,rr_options(g2)
        ldconst rrhigh,r3               # Make it a high priority
        stob    r3,rr_strategy(g2)
!       ldl     nv_lsda(r15),r4         # Get SDA
        stl     r4,rr_rsda(g2)
        ld      nv_length(r15),r3       # Get length in sectors
        st      r3,rr_rlen(g2)
        lda     R$que,g0
        call    K$qw                    # Queue the request with wait
#
        ldob    rr_status(g2),r11       # Get raid status
        cmpobe  ecok,r11,.oss20         # Jif good
        cmpobne eccompare,r11,.oss15    # Jif not a miscompare
        ld      R_pcmisc,r3             # Get the miscompare count
        addo    1,r3,r3
        st      r3,R_pcmisc             # Bump it
        addo    1,g8,g8                 # Increment the error counter - fixed
                                        #  means no log of error
        b       .oss20                  # Parity corrected - treat as good
#
# --- Error completion
#
.oss15:
        addo    1,g8,g8                 # Increment error counter
        mov     r14,g0                  # Pass g0 = RDD
        ldconst SINGLE_RDD,g1           # Single RDD to mark
        call    O$markraidscan          # Parity scan the entire RAID
        call    o$log_nvarecord         # Log RAID request error
#
# --- Good completion
#
.oss20:
        addo    1,g7,g7                 # Increment resync attempted counter
        mov     r10,g1                  # Restore g1 (ILT)
        call    M$rir                   # Release the request - g1=ILT, g2=RRP
#
# --- Do next record
#
.oss30:
        subo    1,r12,r12               # Decrement entry count
        lda     nvasiz(r15),r15         # Point to next record
        cmpobne 0,r12,.oss10            # Jif more to do
#
# --- Exit
#
        call    o$log_resyncdone        # Log the completion with stats
        mov     g5,g2                   # Restore g2
        callx   (g4)                    # Completion routine
        ret                             # End task
#
#**********************************************************************
#
#  NAME: O$markraidscan
#
#  PURPOSE:
#       To provide a common means of marking a RAID 5 device as
#       requiring a parity check.
#
#  DESCRIPTION:
#       If g1 = 0, g0 is assumed to be an RDD and just that one is marked.
#       If g1 = 1, all RAID 5 devices will be marked.
#       If g1 = 2, all RAIDs that are in the "Not Mirroring" Astatus State
#       will be marked.
#       If g1 = 3, all RAIDs in the list will be marked
#
#       If the RAID is already marked for rebuild it will be remarked for
#       a parity check.
#
#       In the g1 = 1 or g1 = 2 case, only RAIDs belonging to this controller
#       will be marked.
#
#  CALLING SEQUENCE:
#       call    O$markraidscan
#
#  INPUT:
#       g0      RDD = parity scan this single RDD if g1 = 1
#               Number of RDDs in the list if g1 = 3 (must > 0)
#       g1      Type of Marking
#               0 = do single RDD (g0 is the RDD to mark, g2 ignored)
#               1 = do all RDDs (g0 and g2 ignored)
#               2 = do all RDDs in the "Not Mirroring" State (g0 and g2 ignored)
#               3 = do all RDDs in the list provided (g0 number of RDDs in the
#                   list and g2 is a pointer to the list)
#       g2      Pointer to the list of RDDs to mark (only used in g1=3 case)
#
#  OUTPUT:
#       None.
#
#**********************************************************************
#
O$markraidscan:
        movt    g0,r12                  # Save g0-g2
        ld      K_ficb,r7               # Get the FICB to get this cntrls SN
        ldconst FALSE,r9                # Set flag to assume no P2 Update reqd
        mov     g0,r10                  # Assume single RDD
        ld      fi_cserial(r7),r7       # Get this Controllers Serial Number
        ldconst 0,r8                    # Zeroing register
        cmpobe  SINGLE_RDD,g1,.osa20    # Jif a single RDD
#
        lda     R_rddindx,r11           # Pointer to the list of all RAIDs
        cmpobne LIST_RDDS,g1,.osa05     # Jif not a list of RDDs
        subo    1,g0,r6                 # Use as an index into the list
        b       .osa10
#
.osa05:
        ldconst MAXRAIDS-1,r6           # Load count
#
# --- Check next RAID device
#
.osa10:
        cmpobne LIST_RDDS,r13,.osa13    # Jif not a list of RDDs
!       ldos    (r14)[r6*2],r10         # r10 = RID being used
        ld      rx_rdd(r11)[r10*4],r10  # r10 = RDD pointed to by RID
        b       .osa14                  # Continue the checking
#
.osa13:
        ld      rx_rdd(r11)[r6*4],r10   # r10 = next RDD
.osa14:
        cmpobe  0,r10,.osa60            # Jif undefined
#
        ldos    rd_vid(r10),g0          # Get the VID
#
        PushRegs(r3)                    # Save registers
        call    DL_AmIOwner             # Do I own it?  g0 = T/F
        PopRegs(r3)
#
        cmpobe  FALSE,g0,.osa60         # Do next RDD if not owned
#
# --- Set parity sync bit if this is a RAID 5 and clear any rebuilding (if
#       rebuilding and parity out of sync, data could be corrupt - the "Parity
#       Scan" required bit will cause the drive to go inoperative).  Also do
#       not do P2 Update if already done - reduces config change churn.
#
.osa20:
        ldob    rd_type(r10),r4         # Get type
        cmpobne rdraid5,r4,.osa50       # Jif not RAID 5 - do next rdd
#
        ldob    rd_astatus(r10),r3      # Get current additional status
        cmpobne ALL_NOT_MIRROR_RDDS,r13,.osa30 # Jif not Mark All "Not  Mirror"
        ld      rd_notMirroringCSN(r10),r5 # Get Not Mirroring Controller SN
        cmpobe  0,r5,.osa50             # Jif not in the "Not Mirroring" State
        cmpobe  r7,r5,.osa50            # Jif this controller set it (do not do)
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        st      r8,rd_notMirroringCSN(r10) # Clear the "Not Mirroring" State
        ldconst TRUE,r9                 # Set flag to do a P2 Update
.osa30:
        bbs     rdaparity,r3,.osa50     # Jif already set - no need to P2 Update
#
        clrbit  rdarebuild,r3,r3        # Clear the Rebuild Required Bit (so
                                        #  that if the drive is restored, the
                                        #  parity can be recreated)
        setbit  rdaparity,r3,r3         # Set parity sync required bit
        clrbit  rdar5srip,r3,r3         # Clear the R5 Stripe Resync In Progress
                                        #  to allow the CCB to take off
c       BIT_SET(DMC_bits, CCB_DMC_raidcache);   /* Flag raid data has changed. */
        stob    r3,rd_astatus(r10)
        ldconst TRUE,r9                 # Set flag to do a P2 Update
#
# --- Advance to next RAID device
#
.osa50:
        cmpobe  SINGLE_RDD,r13,.osa70   # Jif a single RDD
#
.osa60:
        subo    1,r6,r6                 # Bump to next RDD
        cmpible 0,r6,.osa10             # Jif more
#
.osa70:
        cmpobe  FALSE,r9,.osa80         # Jif P2 Update not required
        call    D$p2update              # Update NVRAM
#
# --- Kick off the task that will do the parity scans
#
.osa80:
        ld      R_pcctrl,r3             # Get parity checker control word
        setbit  rdpcnewcmd,r3,r3        # Indicate a new parity command
        setbit  rdpcmarked,r3,r3        # Indicate there is a marked RDD
        clrbit  rdpcspecific,r3,r3      # Don't do a specific RID
        setbit  rdpc1pass,r3,r3         # Indicate only one pass is needed
        setbit  rdpccorrect,r3,r3       # Correct any errors found during resync
        setbit  rdpcenable,r3,r3        # Enable parity checking
        st      r3,R_pcctrl
#
        movt    r12,g0                  # Restore g0-g2
        ret
#
#**********************************************************************
#
#  NAME: o$log_nvarecord
#
#  PURPOSE:
#       To provide a common means of reporting that an NVA resync
#       operation failed.
#
#  DESCRIPTION:
#       The NVA record failed syncronization error log message is
#       constructed with information taken from the RDD and RRP.
#
#  CALLING SEQUENCE:
#       call    o$log_nvarecord
#
#  INPUT:
#       g0 = RDD
#       g2 = RRP
#
#  OUTPUT:
#       None.
#
#**********************************************************************
#
o$log_nvarecord:
        PushRegs                        # Save all G registers (stack relative)
        mov     g0,r15                  # r15 = RDD
        mov     g2,r14                  # r14 = RRP
#
# --- Calculate the VDisk area that may be affected
#
        ldos    rr_rid(r14),g0          # RID
        ld      rr_rlen(r14),g1         # Length in sectors
        ldl     rr_rsda(r14),g2         # Starting disk address
        mov     g0,r13                  # r13 = RID
        mov     g1,r12                  # r12 = Length of op
        movl    g2,r10                  # r10,r11 = RID SDA
        lda     16(sp),sp               # Allocate space on stack for 2 returns
        lda     -16(sp),g4              # Returned Beginning VDisk Address
        lda     -8(sp),g5               # Returned Ending VDisk Address
        call    RB_CalcAddressRange     # Get VDisk Begin and End Address
        ldl     -16(sp),r8              # r8,r9 = VDisk Beginning Address
        ldl     -8(sp),r6               # r6,r7 = VDisk Ending Address
        lda     -16(sp),sp              # Restore stack pointer
        cmpo    1,0                     # Increment the VDisk ending address
        addc    1,r6,r6
        addc    0,r7,r7
        cmpo    0,0                     # Subtract End from Begin to get Length
        subc    r8,r6,g2
        subc    r9,r7,g3                # Equals total space affected g2/g3
        ldos    rd_vid(r15),r5          # r5 = VID from RDD
# -----
        cmpobe  0,g3,.olognvarec30      # Jif no overflow
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
#
        ldconst nva_sft1,r3             # r3 = error code to log
        st      r3,efa_ec(g0)           # Save the Error Code
        stos    r5,efa_data(g0)         # Save the VID
        stos    r13,efa_data+2(g0)      # Save the RID
        st      r12,efa_data+4(g0)      # Save the RID Length
        stl     r10,efa_data+8(g0)      # Save the RID SDA
        stl     r8,efa_data+16(g0)      # Save the VID Beginning Address
        stl     r6,efa_data+24(g0)      # Save the VID Ending Address
        stl     g2,efa_data+32(g0)      # Save the calculated length
        ldconst 40+4,r3                 # Number of bytes saved (ec + data)
        st      r3,mle_len(g0)          # Save the number of bytes to send
        call    M$soft_flt              # Error Trap or Log failure
#
.olognvarec30:
        ldob    rr_status(r14),r3       # RRP status
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst mlenvarecord,r4         # Event code
        st      r4,mle_event(g0)        # Store as word to clear other bytes
        stob    r3,env_status(g0)       # Store the RRP Status
        stos    r5,env_vid(g0)          # Store the VID from the RDD
        stos    r13,env_rid(g0)         # Store the RID from the RRP
        stl     r10,env_rsda(g0)        # Store the RAID Starting Disk Address
        st      r12,env_rlen(g0)        # Store the RAID Length
        stl     r8,env_vsda(g0)         # Store the VDisk Starting Disk Address
        st      g2,env_vlen(g0)         # Store the VDisk Length
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], envlen);
#
# --- Exit
#
        PopRegsVoid                     # Restore all G registers (stack relative)
        ret
#
#**********************************************************************
#
#  NAME: o$log_resyncdone
#
#  PURPOSE:
#       To provide a common means of reporting that a stripe
#       resync has completed.
#
#  DESCRIPTION:
#       This message is constructed from g register input.
#
#  CALLING SEQUENCE:
#       call    o$log_resyncdone
#
#  INPUT:
#       g6      Valid NVA records
#       g7      Stripes attempted to be resunc
#       g8      Stripes that couldn't be resunc
#
#  OUTPUT:
#       None.
#
#**********************************************************************
#
o$log_resyncdone:
        mov     g0,r12                  # Save g0
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        ldconst mleresyncdone,r4        # Event code
        st      r4,mle_event(g0)        # Store as word to clear other bytes
        st      g6,erd_validnva(g0)     # Valid NVA records
        st      g7,erd_attempts(g0)     # Stripes attempted to be resunc
        st      g8,erd_errors(g0)       # Stripes that couldn't be resunc
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], erdlen);
        mov     r12,g0                  # Restore g0
        ret
#
#****************************************************************************
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
