# $Id: nva.as 147854 2010-09-20 14:49:35Z m4 $
#**********************************************************************
#
#  NAME: nva.as
#
#  PURPOSE:
#       To provide a means of handling NVA records.
#
#  FUNCTIONS:
#       M$ap4nva        - Allocate and fill in an NVA record
#       M$anvaw         - Allocate an NVA record w/ wait
#       M$rp4nva        - Release an NVA record
#
#       M$p4init        - init Part 4 NVA records
#       M$p4chksumchk   - Check Part 4 NVRAM checksum
#
#       This module employs no processes.
#
#  Copyright (c) 1996-2010 Xiotech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- global function declarations ------------------------------------
#
        .globl  M$p4init                # Init P4 NVA control, maps, and NVRAM
        .globl  M$p4clear               # Clear P4 NVA
.ifdef BACKEND
        .globl  M$ap4nva                # Allocate and fill in a P4 NVA record
.endif  # BACKEND
        .globl  M$rp4nva                # Release a P4 NVA record
.ifdef FRONTEND
        .globl  M$wnva                  # Write an NVA record passing NVA addr
        .globl  M$anvanw                # Allocate an NVA Address without wait
.endif  # FRONTEND
        .globl  M$rnvaa                 # Remove an NVA Address from bitmap
#
# --- private function declarations -----------------------------------
#
        .globl  M$log_nvachecksum       # Log NVA checksum error
.ifdef BACKEND
        .globl  M$p4chksumchk           # Verify Part 4 NVRAM checksum
.endif  # BACKEND
#
# --- global usage data definitions -----------------------------------
#
        .data
#
# --- NVRAM control structures
#
        .globl  P4_nvac                 # RAID Resync NVA Control structure
P4_nvac:
        .space  nvacsize,0
#
        .globl  nvr_offset
nvr_offset:
        .word 0
#
        .globl  new_csum
new_csum:
        .word 0
#
        .globl  nvr_part
nvr_part:
        .word 0
#
        .globl  p3rel
p3rel:
        .word 0
#
        .globl  real_addr
real_addr:
        .word 0
#
        .globl  rel_offset
rel_offset:
        .word 0
#
#  Following variables should be in Shared Memory section
#
        .section    .shmem
#
#  Local memory for FE and BE to maintain the copy of micro memory contents
#
.ifdef FRONTEND
         .globl  p_localFENVRAM_BASE       # Pointer to the local NVRAM
p_localFENVRAM_BASE:
         .space  LOCAL_FE_NVRAMSIZ,0
.else   # FRONTEND
         .globl p_localBENVRAM_BASE
p_localBENVRAM_BASE:
         .space  LOCAL_BE_NVRAMSIZ,0
.endif #FRONTEND
#
# --- executable code -------------------------------------------------
#
        .text
#**********************************************************************
#
#  NAME: M$p4init
#
#  PURPOSE:
#       To provide a common means of initializing the part 4 NVA records.
#
#  DESCRIPTION:
#       The checksum is cleared and the bitmap for NVAC
#       allocations is initialized to zeroes indicating that all NVAC
#       entries are available for assignment.
#
#  CALLING SEQUENCE:
#       call    M$p4init
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
M$p4init:
.ifdef  BACKEND
        lda      p_localBENVRAM_BASE, r6   # Get base of NVRAM Part 4  for BE
c       r7  =  gMPNvramP3Size;
#       ldconst NVSRAMP3SIZ, r7           # commented by vishu
        addo    r6,r7,r4
.else   # BACKEND
        lda      p_localFENVRAM_BASE, r4    # Get base of NVRAM Part 4 for FE
#
# --- Set up the CCB Communications Area so the FE P4 NVRAM can be read directly
#
        ldconst FE_NVRAM_P4_ID_ASCII_1,r6 # Get the ASCII string for FE NVRAM P4
        ldconst FE_NVRAM_P4_ID_ASCII_2,r7
c       r3     =   gMPNvramP4Size;
        stl     r6,FE_NVRAM_P4_ID       # Save the ID of the FE NVRAM P4 Area
        st      r4,FE_NVRAM_P4_ADDR     # Save the Beginning Addr of the P4 Area
        st      r3,FE_NVRAM_P4_LENGTH   # Save the Length of the P4 Area
.endif # BACKEND
#
# --- Initialize the P4 NVAC structure for the SNVA -------------------
#
c       r3 = NUM_OF_P4_NVA_WK;
        lda     P4_nvac,r15             # Get NVAC control address
        st      r3,nc_cur(r15)          #  to current
        st      r3,nc_min(r15)          #  to minimum
#
# --- Allocate SNVA assignment bitmap & setup NVA control structure
#
        lda     nv_csum(r4),r6          # Get address of checksum
        st      r6,nc_csum(r15)         # Set up checksum ptr
#
        ldconst nvabasesiz,r3           # Get header size
        addo    r3,r4,r4                # Increment past header
        st      r4,nc_nvarec(r15)       # Save ptr to first NVA record
#
# (num rec / 32 bits) * 4 bytes per word, Assign Part 3 bitmap.
c       g0 = s_MallocC(NUM_OF_P4_NVA_WK/8, __FILE__, __LINE__);
        st      g0,nc_mapbase(r15)      # Set up bitmap base
        st      g0,nc_mapptr(r15)       # Set up bitmap ptr
#
        ret
#
#**********************************************************************
#
#  NAME: M$p4clear
#
#  PURPOSE:
#       To provide a common means of clearing Part 4 of the NVRAM.
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
#       call    M$p4clear
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
# void NVA_ClearP4(void);
        .globl  NVA_ClearP4
NVA_ClearP4:
M$p4clear:
        lda     P4_nvac,r15             # Get NVA control structure address
c       r13 = gMPNvramP4Size - sizeof(NVA_HEADER);
c       r12 = NUM_OF_P4_NVA_WK;
c       r11 = FALSE;                      # To indicate that it is P4 clrear
#
# --- Common code
#
.ifdef BACKEND
m$nvaclear:
.endif  # BACKEND
.ifdef HISTORY_KEEP
c CT_HISTORY_OFF();
.endif  # HISTORY_KEEP
        mov     0,r4                    # Get a zero ready
#
# --- Clear header (includes checksum)
#
        ld      nc_nvarec(r15),r14      # Get start of NVA records
        ldconst nvabasesiz,r3           # Get header size
        subo    r3,r14,r14              # Point to start of header area
.cl10:
        subo    1,r3,r3                 # Calculate remaining size
        stob    r4,(r14)[r3*1]          # Clear it
        cmpobne 0,r3,.cl10              # Jif more
#
        st      r4,nc_scsum(r15)        # Also clear shadow checksum
#
# --- Reset NVA control counters
#
        st      r12,nc_cur(r15)         #  to current
        st      r12,nc_min(r15)         #  to minimum
#
# --- Clear NVA records
#
        ld      nc_nvarec(r15),r14      # Get start of NVA records
#        mov     r14,r12                 # save for upcoming loop comparison
.cl30:
        subo    1,r13,r13               # Calculate remaining size
        stob    r4,(r14)[r13*1]         # Clear it
        cmpobne 0,r13,.cl30             # Jif more
        ld gMMCFound,r3
        cmpobne TRUE,r3,.c136           # Dont write into Micro Memory Card in not found
        cmpobne TRUE,r11, .c135         # Clear P3 if r11 is TRUE, else clear P4
        PushRegs(r3)                    # Save the G registers
        ldconst MICRO_MEM_BE_P3_START,g0
        ldconst nvabasesiz,r13
        subo    r13,r14,r14
        mov     r14,g1                  # r14 contains the starting address
c       g2  =  gMPNvramP3Size;
        call   MM_Write
        PopRegsVoid(r3)                 # Restore all G registers
        ldconst  FALSE,r11
        b        .c136
#
.c135:
        PushRegs(r3)                    # Save the G registers
.ifdef FRONTEND
        ldconst MICRO_MEM_FE_P4_START,g0
.else  #BACKEND
        ldconst MICRO_MEM_BE_P4_START,g0
.endif #FRONTEND
        ldconst nvabasesiz,r13
        subo    r13,r14,r14
        mov     r14,g1                  # r14 contains the starting address
c       g2 = gMPNvramP4Size;
        call   MM_Write
        PopRegsVoid(r3)                 # Restore all G registers
#c           MM_Write (MICRO_MEM_BE_P4_START, r14, NVSRAMP4SIZ);
#
# --- Clear NVA allocation bitmap
#
.c136:
        ld      nc_mapbase(r15),r14     # Get base of bitmap
        st      r14,nc_mapptr(r15)      # Initialize map pointer
        ldconst 32,r8                   # Get number of bitmap entries per word
#
.cl40:
        st      r4,(r14)                # Clear bitmap
        lda     4(r14),r14              # Advance bitmap ptr
        subo    r8,r12,r12              # Adjust remaining count
        cmpobne 0,r12,.cl40             # Jif more
#
# --- Exit
#
.ifdef HISTORY_KEEP
c CT_HISTORY_ON();
.endif  # HISTORY_KEEP
        ret
#
.ifdef BACKEND
#**********************************************************************
#
#  NAME: M$ap4nva
#
#  PURPOSE:
#       To provide a common means of assigning and initializing an NVA
#       record.
#
#  DESCRIPTION:
#       A resync NVA entry is assigned and initialized with information taken
#       directly from an NVA structure. A pointer to the location of that
#       entry is returned to the user and is later used to release the
#       NVA entry.
#
#       The actual construction of the NVA is undertaken with interrupts
#       disabled to minimize the chance of data corruption during a
#       loss of power.
#
#       This function will wait for available space in the resync NVRAM area.
#
#  CALLING SEQUENCE:
#       call    M$ap4nva
#
#  INPUT:
#       g13 = address of NVA structure with data to be added to NVRAM
#
#  OUTPUT:
#       g0 = NVA entry address
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
M$ap4nva:
        ldconst P4_ALLOC,r4                    #Indicate that it is p-4 alloc
        st      r4,nvr_part
#
        mov     g1,r13                  # Save register g1
        mov     g2,r14                  # Save register g2
#
        lda     P4_nvac,g1              # Get NVA control structure address
        ldconst pcsnvawait,g2           # Get P4 PCB wait state
#
# --- Construct data for NVA entry - r4,5,6,7 has the NVA record
#
        ldos    nv_id(g13),r4           # Get ID
        ld      nv_length(g13),r5       # Get sector count
!       ldl     nv_lsda(g13),r6         # Get LSDA
#
        balx    m$anva,r15              # Branch to common code
.ifdef NVA_DEBUG
c fprintf(stderr,"%s%s:%u The Offset of NVA returned by  M$ap4nva:%x\n", FEBEMESSAGE, __FILE__, __LINE__,g0);
.endif # NVA_DEBUG
#
        mov     r13,g1                  # Restore registers
        mov     r14,g2
        ret
.endif  # BACKEND
#
#**********************************************************************
#
#  NAME: m$anva,M$wnva
#
#  PURPOSE:
#       To provide a common means of assigning and initializing an NVA
#       record.
#
#  DESCRIPTION:
#       For m$anva a resync NVA entry is assigned.  The NVA is initialized with
#       information taken directly from an NVA structure. A pointer to the
#       location of that entry is returned to the user and is later used to
#       release the NVA entry.
#
#       The actual construction of the NVA is undertaken with interrupts
#       disabled to minimize the chance of data corruption during a
#       loss of power.
#
#  CALLING SEQUENCE:
#       balx    m$anva,r15
#       balx    M$wnva,r15
#
#  INPUT:
#       g0 = NVA entry address (M$wnva only)
#       g1 = NVA Control Structure
#       g2 = PCB Wait Status if need to wait (m$anva only)
#       r4 = NVA Record ID
#       r5 = Number of Sectors of Write
#       r6,r7 = Logical Starting Disk Address
#
#  OUTPUT:
#       g0 = NVA entry address
#
#  REGS DESTROYED:
#       r3, r4, r5, r6, r7, r8, r9
#
#**********************************************************************
#
.ifdef FRONTEND
# C access
# NVA* NVA_Write(NVA* pNVA, NVA_CONTROL* pNVAC, UINT8 pcbWait, UINT32 nvaID,
#                    UINT32 sectors, UINT64 lsda);
# Since this is a branch and link interface it isn't directly callable from C
M$wnva:
#
# --- Update the NVA Control Structure with the assigned NVA Address and
#       then the checksum (record may not be zero that is being written to)
#
#   Adjust statistics
#
        ld      nc_cur(g1),r3           # Get available count
        ld      nc_min(g1),r8           # Get minimum count
        subo    1,r3,r3                 # Adjust available count
        st      r3,nc_cur(g1)
        cmpibge r3,r8,.mwnva10          # Jif current >= minimum
        st      r3,nc_min(g1)           # Set new minimum
.mwnva10:
#
#   Update allocation bitmap
#
        ld      nc_nvarec(g1),r8        # r8 = Calculate NVA entry number
        ld      nc_mapbase(g1),r9       # r9 = Get base of allocation map
        addo    16,sp,sp                # Bump the Stack to save some registers
#nva Offset
# g0 has offset from base of NVRAM P4.
        ldconst nvabasesiz,r3
        subo    r3,g0,r3
        stq     r12,-16(sp)             # Save r12-r15
        shro    4,r3,r3                 # Calculate NVA entry number
        shro    3,r3,r8                 # Isolate word offset for map
        andnot  3,r8,r8
?       ld      (r9)[r8*1],r12          # Get word of map
        and     0x1f,r3,r3              # Isolate bit position for word
#
        setbit  r3,r12,r12              # Set the allocation bit within
        st      r12,(r9)[r8*1]          #  map
#
#   Update the CRC reading the old record, subtracting from the current
#       checksum, and then go update with the new record
#
#nva Offset
        st      g0,nvr_offset               # Save the offset for future use
#
#    get absolute address from offset : address of first record - nva header size + offset
#
        ldconst nvabasesiz,r12
        ld      nc_nvarec(g1),r13
        subo    r12,r13,r13
        addo    r13,g0,g0
        ldq     (g0),r12                # Get entire NVA entry (16 bytes)
        ld      nc_scsum(g1),r3         # Get shadowed checksum
        cmpobe  0,r13,.mwnva15          # Jif the record is zero (length = 0)
#
        addo    r12,r13,r8              # Recalculate checksum
        addo    r14,r15,r9
        addo    r8,r9,r9                # r9 = checksum adjustment
        subo    r9,r3,r3                # Calculate new checksum by removing old
                                        #  record information
#
.mwnva15:
        ldq     -16(sp),r12             # Restore r12-r15
        subo    16,sp,sp                # Restore the stack pointer
#
        b       .manva20                # Update the NVRAM
.endif  /* FRONTEND */
#
# -----------------------------------------------------------------------------
#
.ifdef BACKEND
m$anva:
#
# --- Assign and construct NVA entry
#
        call    m$anvaw                 # Assign NVA entry - interrupts
                                        # disabled on return
                                        # on return: g0 = NVA addr
        ld      nc_scsum(g1),r3         # Get shadowed checksum
                                        #  Assume old record is zero
# nva Offset
#
# Copy real address to g0, because it needs to write into the NVRAM here.
#
        st      g0,nvr_offset
        ld      real_addr,g0
.endif  # BACKEND
#
# --- Common assign code ------------------------------------------------------
#
#       Recalculate checksum
#
.ifdef FRONTEND
.manva20:
.endif  /* FRONTEND */
        addo    r4,r5,r8                # Recalculate checksum
        addo    r6,r7,r9
        addo    r8,r9,r9
        addo    r3,r9,r8
#
        stob    r4,nv_id(g0)            # Record ID in NVRAM
        shro    8,r4,r4
        stob    r4,nv_id+1(g0)
        ldconst 0,r3
        stob    r3,nv_id+2(g0)          # Zero out reserved bytes
        stob    r3,nv_id+3(g0)          # Zero out reserved bytes
#
        stob    r5,nv_length(g0)        # Record sector count in NVRAM
        shro    8,r5,r5
        stob    r5,nv_length+1(g0)
        shro    8,r5,r5
        stob    r5,nv_length+2(g0)
        shro    8,r5,r5
        stob    r5,nv_length+3(g0)
#
        stob    r6,nv_lsda(g0)          # Record LSDA lower in NVRAM
        shro    8,r6,r6
        stob    r6,nv_lsda+1(g0)
        shro    8,r6,r6
        stob    r6,nv_lsda+2(g0)
        shro    8,r6,r6
        stob    r6,nv_lsda+3(g0)
#
        stob    r7,nv_lsda+4(g0)        # Record LSDA upper in NVRAM
        shro    8,r7,r7
        stob    r7,nv_lsda+5(g0)
        shro    8,r7,r7
        stob    r7,nv_lsda+6(g0)
        shro    8,r7,r7
        stob    r7,nv_lsda+7(g0)
#
# --- Update checksum
#
        st      r8,nc_scsum(g1)         # Update shadowed checksum
        ld      nc_csum(g1),r9
        stob    r8,(r9)                 # Record checksum in NVRAM
        shro    8,r8,r8
        stob    r8,1(r9)
        shro    8,r8,r8
        stob    r8,2(r9)
        shro    8,r8,r8
        stob    r8,3(r9)
#
# Following is to write the NVRAM entries onto the Micro Mem card
#
        ld      gMMCFound,r3
        cmpobne TRUE,r3,.manva26
        ld      nc_scsum(g1),r8
        st      r8,new_csum
        ld      nvr_part,r8
        ld      nvr_offset,r4
#
# write data and checksum to the Micromemory card to appropriate parts.
#
        cmpobne P3_ALLOC,r8,.manva25
# Write P-3 NVRAM ***********
        PushRegs(r3)                    # Save the G registers
        mov     g0,g1
        ldconst MICRO_MEM_BE_P3_START,r7
        b       .manva25_5
#
.manva25:
# Write P-4 NVRAM ***********
        PushRegs(r3)                    # Save the G registers
        mov     g0,g1
.ifdef FRONTEND
        ldconst MICRO_MEM_FE_P4_START,r7
.else # BACKEND
        ldconst MICRO_MEM_BE_P4_START,r7
.endif # FRONTEND
.manva25_5:
        addo    r7,r4,g0
        ldconst 16,g2
        call    MM_Write
# Write checksum to the starting address of NVRAM,
# Note: Checksum address is not taken from NVAC structure here,
#       since it should be at starting address of this part of NVRAM,
#       checksum is written there.
        mov     r7,g0
        lda     new_csum,g1
        ldconst 4,g2
        call    MM_Write
        PopRegsVoid(r3)                 # Restore all G registers
#
#nva Offset
# return OFFSET instead of absolute address.
.manva26:
        ld      nvr_offset,g0
#
# --- Exit
#
        bx      (r15)                    # return to caller
#
#**********************************************************************
#
#  NAME: m$anvaw/M$anvanw
#
#  PURPOSE:
#       To provide a common means of assigning an NVA structure within NVRAM.
#
#  DESCRIPTION:
#       If a power fail condition exists, this routine stalls on that
#       condition.  The bitmap is searched for an available NVA entry.
#       The bitmap is updated to reflect assignment for this entry.
#       The address of this entry plus the checksum location are
#       returned to the caller.
#
#       Interrupts are disabled when returning to the caller in the m$anvaw
#       function.  The m$anvanw function will not disable interrupts.
#
#       The function m$anvaw will wait for available space in the
#       resync NVRAM area.  The function M$anvanw will not wait - if full, an
#       NVA record address of 0xFFFFFFFF will be returned
#
#  CALLING SEQUENCE:
#       call    m$anvaw
#       call    M$anvanw
#
#  INPUT:
#       g1 = NVA control structure address
#       g2 = PCB wait status (m$anvaw only)
#
#  OUTPUT:
#       g0 = NVA record address (always for m$anvaw)
#            0xFFFFFFFF (for m$anvanw if none are available)
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
.ifdef FRONTEND
M$anvanw:
#
# --- Check for available entry
#
        ld      nc_cur(g1),r3           # Get available count
        ldconst FALSE,r15               # Set flag to not disable interrupts
        cmpobne 0,r3,.an20              # Jif any
#
# --- None are available, return immediately with the invalid Record ID
#
        ldconst 0xFFFFFFFF,g0           # Set the invalid Record ID
        b       .an100
.endif  # FRONTEND
#
# ---------------
#
.ifdef BACKEND
m$anvaw:
#
# --- Check for available entry
#
.an10:
        ld      nc_cur(g1),r3           # Get available count
        ldconst TRUE,r15                # Set flag to disable interrupts
        cmpobne 0,r3,.an20              # Jif any
#
c       TaskSetMyState(g2);             # Place process in wait state
#
        ld      nc_wait(g1),r3          # Bump wait count
        addo    1,r3,r3
        st      r3,nc_wait(g1)
#
        call    K$xchang                # Exchange processes
        b       .an10
.endif # BACKEND
#
# --- Adjust statistics
#
.an20:
        ld      nc_min(g1),r5           # Get minimum count
        subo    1,r3,r3                 # Adjust available count
        st      r3,nc_cur(g1)
        cmpibge r3,r5,.an30             # Jif current >= minimum
#
        st      r3,nc_min(g1)           # Set new minimum
#
# --- Search bitmap for the next available entry
#
.an30:
        ldl     nc_mapbase(g1),r12      # Get base address of map and
                                        #  current ptr into map
c       r11 =  (NUM_OF_P4_NVA_WK/8)+r12 # vishu: check the logic...
.an40:
?       ld      (r13),r3                # Get next word of map
        spanbit r3,r4                   # Check for available entry
        be      .an50                   # Jif found
#
        lda     4(r13),r13              # Advance to the next entry
        cmpo    r11,r13                 # Check for wrap of bitmap
        sele    r13,r12,r13             # Reset pointer if necessary
        b       .an40
#
# --- Update bitmap
#
.an50:
        ld      nc_nvarec(g1),g0        # Get start of NVA records
        setbit  r4,r3,r3                # Set entry as assigned in bitmap
        st      r13,nc_mapptr(g1)       # Update map ptr
?       st      r3,(r13)                # Update bitmap
#
# --- Calculate address of assignment
#
        subo    r12,r13,r13             # Compute word offset into bitmap
        shlo    3,r13,r13               # Compute total bit offset into
        addo    r13,r4,r4               #  bitmap
#
        shlo    4,r4,r4                 # Calculate NVA entry address
        addo    r4,g0,g0
#
# Get the offset to write into actual nvram.
#
        ldconst nvabasesiz,r12
        addo    r12,r4,r4
        st      r4,nvr_offset                   # store nvram offset
#nva Offset:    Return offset instead of absolute address and
#   Store the absolute address in a global variable
        st     g0,real_addr
        mov    r4,g0
.ifdef NVA_DEBUG
c fprintf(stderr,"%s%s:%u The Offset of NVA returned by M$anvaw or M$anvanw:%x\n", FEBEMESSAGE, __FILE__, __LINE__,g0);
.endif # NVA_DEBUG
#
#        cmpobe  FALSE,r15,.an100        # Jif Interrupts should not be disabled
#
# --- Exit
#
.ifdef FRONTEND
.an100:
.endif  # FRONTEND
        ret
#
#**********************************************************************
#
#  NAME: M$rp4nva
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
#       call    M$rp4nva
#       branch  m$rnva
#
#  INPUT:
#       g0 = address of NVA record to be released
#
#  OUTPUT:
#       g0      0 = Good
#               1 = Error - address not in range (for M$rp4nva only)
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
# --- Release Part 4 NVA record
#
M$rp4nva:
#
        ldconst P4_REL,r13              # To indicate that it is P4 Release
        st      r13,p3rel
        lda     P4_nvac,r15             # Get NVAC control structure address
#
        ld      nc_nvarec(r15),r3       # Get addr of 1st record
#nva Offset
        st      g0,rel_offset           # Save offset
#
#   get absolute address from offset : address of first record - nva header size + offset
#
        ldconst nvabasesiz,r4
        subo    r4,r3,r4
        addo    r4,g0,g0
c       r4      =  gMPNvramP4Size - sizeof(NVA_HEADER);
        addo    r3,r4,r4                # Addr of last record + 1 record
        cmpobl  g0,r3,.mrv10            # Jif g0 < lowest record: error
        cmpobl  g0,r4,m$rnva            # Jif g0 < highest record + 1: good
#
#   The Record ID is out of bounds.  Report the error to the caller.
#
.mrv10:
        ldconst 1,g0                    # Error return code
        b       .rv1000
#
# --- Common release code
#
m$rnva:
#
# --- Calculate checksum adjustment if this entry has not already been cleared
#       before (could have been cleared but could not send response across
#       the FE Fibre due to fibre error, which will then get retried).
#
        ldq     (g0),r4                 # Get entire NVA entry (16 bytes)
        cmpobe  0,r5,.rv100_n           # Jif already cleared (length = 0) and
                                        #  report good status
#
        addo    r4,r5,r11               # Recalculate checksum
        addo    r6,r7,r10
        addo    r10,r11,r11             # r11 = checksum adjustment
#
# --- Calculate new checksum
#
        ld      nc_scsum(r15),r3        # Get shadowed checksum
        ldconst 0,r8                    # Clear NVA entry
        ld      nc_csum(r15),r4         # Get checksum ptr
        subo    r11,r3,r3               # Calculate new checksum
#
# --- Clear NVA entry
#
        stob    r8,0(g0)                # Clear NVRAM record
        stob    r8,1(g0)
        stob    r8,2(g0)
        stob    r8,3(g0)
        stob    r8,4(g0)
        stob    r8,5(g0)
        stob    r8,6(g0)
        stob    r8,7(g0)
        stob    r8,8(g0)
        stob    r8,9(g0)
        stob    r8,10(g0)
        stob    r8,11(g0)
        stob    r8,12(g0)
        stob    r8,13(g0)
        stob    r8,14(g0)
        stob    r8,15(g0)
#
# --- Record new checksum
#
        st      r3,nc_scsum(r15)        # Update shadowed checksum
        stob    r3,(r4)                 # Update NVRAM checksum
        shro    8,r3,r3
        stob    r3,1(r4)
        shro    8,r3,r3
        stob    r3,2(r4)
        shro    8,r3,r3
        stob    r3,3(r4)
# Don't  write to Micromemory card if it is not found
        ld gMMCFound,r3
        cmpobne TRUE,r3,.rv95
# Clear the NVA entry and update new checksum in Micromem card
        ld  nc_scsum(r15),r3
        st  r3,new_csum
        ld  p3rel,r3
        cmpobne P3_REL,r3,.rv90          # Check whether it is P3 release (TRUE)
.ifdef BACKEND
        PushRegs(r4)                    # Save the G registers
#
# Clear the NVA Entry
#
        mov     g0,g1
        ldconst MICRO_MEM_BE_P3_START,r8
# nva Offset
#        lda     p_localBENVRAM_BASE,r5
#        subo    r5,g0,g0
        ld      rel_offset,g0
        addo    g0,r8,g0
        ldconst 16,g2
        call   MM_Write
#
# Write the updated Checksum
#
        ldconst MICRO_MEM_BE_P3_START,g0
        lda     new_csum,g1
        ldconst 4,g2
        call   MM_Write
        PopRegsVoid(r4)                 # Restore all G registers
.endif # BACKEND
        b .rv95
#
.rv90:
        PushRegs(r4)                    # Save the G registers
.ifdef  FRONTEND
        mov     g0,g1
        ldconst MICRO_MEM_FE_P4_START,r8
#nva Offset
#        lda      p_localFENVRAM_BASE,r5
#        subo    r5,g0,g0
        ld      rel_offset,g0
#######
        addo    g0,r8,g0
        ldconst 16,g2
        call   MM_Write
.else   # FRONTEND
        mov     g0,g1
        ldconst MICRO_MEM_BE_P4_START,r8
#nva Offset
#        lda     p_localBENVRAM_BASE,r5
#        subo    r5,g0,g0
#        ldconst NVSRAMP3SIZ,r5
#        subo    r5,g0,g0
         ld      rel_offset,g0
#######
        addo    g0,r8,g0
        ldconst 16,g2
        call   MM_Write
.endif  #FRONTEND
         PopRegsVoid(r4)
#
# Write the updated Checksum
#
        PushRegs(r4)
.ifdef FRONTEND
        ldconst MICRO_MEM_FE_P4_START,g0
.else  #BACKEND
        ldconst MICRO_MEM_BE_P4_START,g0
.endif #FRONTEND
        lda     new_csum,g1
        ldconst 4,g2
        call   MM_Write
        PopRegsVoid(r4)                 # Restore all G registers
#
.rv95:
#
# --- Update the NVA Control Structure Address Bit Map and currently available
#
        mov     g1,r3                   # Save g1
        mov     r15,g1                  # g1 = NVA Control Structure
                                        # g0 = NVA Address
#nva Offset
       ld rel_offset,g0
.ifdef NVA_DEBUG
c fprintf(stderr,"%s%s:%u  1.NVA offset passed to M$rnvaa: %x\n", FEBEMESSAGE, __FILE__, __LINE__,g0);
.endif #NVA_DEBUG
#######
        call    M$rnvaa                 # Update the NVA Control structure
        ld      nc_wait(r15),r13        # Get wait count
        mov     r3,g1                   # Restore g1
#
# --- Check for possible NVA wait condition
#
        cmpobe  0,r13,.rv100_n          # Jif none
#
        ldconst 0,r4                    # Clear wait count
        st      r4,nc_wait(r15)
#
c       TaskReadyByState(pcsnvawait);   # Activate tasks waiting for P4 NVA
#
# --- Exit
#
.rv100_n:
        ldconst 0,g0                    # Good return code
#
.rv1000:
        ret
#
#**********************************************************************
#
#  NAME: M$rnvaa
#
#  PURPOSE:
#       To provide a common means of releasing a resync NVA address
#
#  DESCRIPTION:
#       Releases the NVA Address from the bitmap and updates the control
#       structure.
#
#  CALLING SEQUENCE:
#       call    M$rnvaa
#
#  INPUT:
#       g0 = address of NVA record to be released
#       g1 = NVA Control record
#
#  OUTPUT:
#       None
#
#  REGS DESTROYED:
#       none
#
#**********************************************************************
#
M$rnvaa:
#
# --- Update statistics and allocation map
#
        ld      nc_cur(g1),r3           # Get current available count
        ld      nc_nvarec(g1),r4        # Calculate NVA entry number
        ld      nc_mapbase(g1),r5       # Get base of allocation map
        lda     1(r3),r3                # Increment available count
        st      r3,nc_cur(g1)           # Store new available count
#
# --- Update allocation map
#
#nva Offset
        ldconst  nvabasesiz,r3
c       if (g0 < r3) fprintf(stderr,"Invalid NVA offset passed to M$rnvaa: less than Lowerlimit: %lx\n",g0);
c       r7  = gMPNvramP4Size;           # we cannot hardcode this to P4 size,
                                        # since P3 and P4 sizes are equal, it is ok.
#       ldconst  NVSRAMP4SIZ,r7         # Commented by vishu..  # we cannot hardcode this to P4 size, but P3 and P4
                                        # sizes are equal...
c       if (g0 >=r7) fprintf(stderr,"Invalid NVA offset passed to M$rnvaa: more than Higher limit:%lx\n",g0);
        subo     r3,g0,r3               # g0 contains offset but not address of  NVA
        shro    4,r3,r3                 # Calculate NVA entry number
        shro    3,r3,r4                 # Isolate word offset for map
        andnot  3,r4,r4
?       ld      (r5)[r4*1],r6           # Get word of map
        and     0x1f,r3,r3              # Isolate bit position for word
#
        clrbit  r3,r6,r6                # Clear allocation bit within
        st      r6,(r5)[r4*1]           #  map
#
# --- Exit
#
        ret
#
#**********************************************************************
#
#  NAME: M$p4chksumchk
#
#  PURPOSE:
#       Verify the checksum of the Part 4 NVA records.
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
#       call    M$p4chksumchk
#
#  INPUT:
#       g0      address of the Part 4 data in DRAM - start of header
#
#  OUTPUT:
#       g0      ecok = good checksum
#               ecchecksum = bad checksum
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
M$p4chksumchk:                          # SNVA records
c       r11    =  gMPNvramP4Size;
c       r12    =  NUM_OF_P4_NVA_WK;
        lda     M$p4clear,r13           # Address of clearing routine
        lda     P4_nvac,r14             # NVA Control address
#
# --- Common Code
#
.ifdef BACKEND
m$chksumchk:
.endif  # BACKEND
.ifdef HISTORY_KEEP
c CT_HISTORY_OFF();
.endif  # HISTORY_KEEP
#
# --- Get checksum from NVRAM
#
        ld      nc_csum(r14),r6         # Get checksum pointer
#
        ld      (r6),r8                 # Get the current checksum
#
        st      r8,nc_scsum(r14)        # Update shadowed checksum
#
# --- Validate checksum
#     Step through each record in DRAM
#
        ldconst nvabasesiz,r3           # Get size of header
        addo    r3,g0,g0                # Point to first NVA record
        mov     g0,r15                  # Copy pointer to temp NVRAM
        ld      nc_mapbase(r14),r9      # Get base of allocation map
#
.dcsc20:
        ld      nv_length(r15),r4       # If length=0, not a used record
        cmpobe  0,r4,.dcsc30
#
# --- Decrement current record from presumed checksum
#
        ldl     (r15),r4
        addo    r4,r5,r4
        subo    r4,r8,r8                # Calculate new checksum
#
        ldl     8(r15),r4
        addo    r4,r5,r4
        subo    r4,r8,r8                # Calculate new checksum
#
# --- Update allocation map
#     Turn on the bit corresponding to any valid entry.
#
        subo    g0,r15,r3               # Calculate NVA offset from start
                                        #  of NVA records
        shro    4,r3,r3                 # Calculate NVA entry number
        shro    3,r3,r4                 # Isolate word offset for map
        andnot  3,r4,r4
        ld      (r9)[r4*1],r6           # Get word of map
        and     0x1f,r3,r3              # Isolate bit position for word
#
        setbit  r3,r6,r6                # Set allocation bit within map
        st      r6,(r9)[r4*1]           # Save the map entry
#
# --- Update current counter
#     Min counter is just for info so don't change it here.
#
        ld      nc_cur(r14),r3          # Update current count
        subo    1,r3,r3
        st      r3,nc_cur(r14)
#
# --- Go to next record
#
.dcsc30:
        lda     16(r15),r15             # Bump source pointer
        subo    1,r12,r12               # Decrement record count
        cmpobne 0,r12,.dcsc20           # Keep looping if not done
#
# --- Compare result to 0
#
        ldconst ecok,g0                 # Setup good checksum return code
        cmpobe  0,r8,.dcsc1000          # Jif good checksum
#
# --- Bad Checksum so log it and clear the NVA records
#
        ldconst ecchecksum,g0           # Bad Checksum
        callx   (r13)                   # Call clearing routine
#
.dcsc1000:
.ifdef HISTORY_KEEP
c CT_HISTORY_ON();
.endif  # HISTORY_KEEP
        ret
#
#**********************************************************************
#
#  NAME: M$log_nvachecksum
#
#  PURPOSE:
#       To provide a common means of reporting an NVA checksum error.
#
#  DESCRIPTION:
#       The nva checksum error log message is constructed with
#       information taken from the .......
#
#  CALLING SEQUENCE:
#       call    M$log_nvachecksum
#
#  INPUT:
#       g0      byte 0 = processor FE or BE
#               byte 1 = part 3 or part 4 NVRAM
#               See def.inc for definitions
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
M$log_nvachecksum:
        mov     g0,r12                  # Save g0
# NOTE: The message is very short, and thus is copied by L$send_packet.
c       g0 = (UINT32)&TmpStackMessage[0]; # Address of temporary message.
        st      r12,mle_event(g0)       # Store as word to clear other bytes
        ldconst mlenvabad,r4            # Event code
        st      r4,mle_event(g0)        # Store as word to clear other bytes
        stos    r12,ena_proc(g0)        # Store parms
# Message is on stack, copy it to MRP.
c       MSC_LogMessageStack(&TmpStackMessage[0], enalen);
        mov     r12,g0                  # Restore g0
        ret
#
#****************************************************************************
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
