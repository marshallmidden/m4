# $Id: nvafe.as 151877 2010-12-14 19:21:25Z steve_wirtz $
#**********************************************************************
#
#  NAME: nvafe.as
#
#  PURPOSE:
#       To provide a means of handling the NVA records in the BE NVRAM.
#
#  FUNCTIONS:
#       This module employs no processes.
#
#  Copyright (c) 1996-2010 XIOtech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- global function declarations ------------------------------------
#
        .globl  NVA_Init            # Initialize packets
        .globl  m$recoverp4         # Recover part 4 NVA records
#
# --- executable code -------------------------------------------------
#
        .text
#**********************************************************************
#
#  NAME: NVA_Init
#
#  PURPOSE:
#       To provide a means of initializing this module for this processor
#
#  DESCRIPTION:
#       Handle FE specific requirements.
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
        .globl  NVA_Init
NVA_Init:
#
# --- Start the misc task
#
        lda     nva$exec,g0             # Establish process to run in background
        ldconst MISCPRI,g1              # Priority
c       CT_fork_tmp = (ulong)"nva$exec";
        call    K$tfork
#
        ret
#
#******************************************************************************
#
#  NAME: nva$exec
#
#  PURPOSE:
#       Handle frontend specific initializations after kernel
#       processes are running.
#
#  DESCRIPTION:
#       This task simply calls the functions that require a working kernel.
#
#  CALLING SEQUENCE:
#       fork    nva$exec
#
#  INPUT:
#       None
#
#  OUTPUT:
#       None
#
#******************************************************************************
nva$exec:
#
# --- Verify the NVA records
#
        call    M$p4init
#
        call    m$recoverp4             # Verify the part 4 NVA data - May never
                                        #  return if communications to the
                                        #  CCB are not opened.
        ret
#
#**********************************************************************
#
#  NAME: m$recoverp4
#
#  PURPOSE:
#       Verify the checksum of the part 4 NVRAM in the FE.
#       This is used at power on before any new ops start coming from
#       the server or being sent to the raid layer.
#
#  DESCRIPTION:
#       This function copies the NVA records from NVRAM into a temporary
#       DRAM buffer. It calls a function to verify the checksum and generate
#       the allocation map.
#
#       If the checksum is bad an error is logged and the buffer is released.
#       The NVRAM will be cleared in this case.
#
#  CALLING SEQUENCE:
#       call    m$recoverp4
#
#  INPUT:
#       None
#
#  OUTPUT:
#       None
#
#**********************************************************************
#
m$recoverp4:
.ifdef HISTORY_KEEP
c CT_HISTORY_OFF();
.endif  # HISTORY_KEEP
        movq     g0,r12                  # Save registers
#
# ---  Copy the SNVA records into DRAM then verify the checksum.
#      If the checksum is bad then the NVRAM be cleared when we finish
#      this section. If it is good then the bitmap will be setup.
#
c       g0 = r5 = s_MallocC(gMPNvramP4Size, __FILE__, __LINE__);
#
        lda      p_localFENVRAM_BASE, r11
                                        # Get base of NVRAM Part 4 for FE
c       r6    =   gMPNvramP4Size;
#       ldconst NVSRAMP4SIZ,r6
        addo    r11,r6,r10               # Get address of last byte of FE p4
#
# Read FE part-4 contents from Micromemory card,
#
        ld gMMCFound,r3                 # Dont Write to MM card if it is not found
        cmpobne 1,r3,.mrp10
        PushRegs(r3)
        ldconst MICRO_MEM_FE_P4_START,g0
        lda      p_localFENVRAM_BASE,g1
c       g2   = gMPNvramP4Size;
#       ldconst NVSRAMP4SIZ,g2
        call    MM_Read
        PopRegsVoid(r3)
# c       MM_Read (MICRO_MEM_FE_P4_START, p_localFENVRAM_BASE, NVSRAMP4SIZ);
#
.mrp10:
c       memcpy((char *)r5, (char *)r11, r10-r11);
#
# --- Verify checksum and build allocation map
#
        mov     g0,r11                  # Save pointer to temp NVRAM
        call    M$p4chksumchk           # Verify checksum
        mov     g0,r6                   # Save checksum return code
#
# --- Deallocate the RAM used for the temp NVRAM.
#
c       s_Free(r11, gMPNvramP4Size, __FILE__, __LINE__);
#
        cmpobe  ecok,r6,.mrp1000        # Jif good checksum
#
# --- Checksum bad so log an error once the communications to the ccb is
#       open to send the message
#
        ldconst 1000,g0                 # Wait for 1 second between checks
.mrp20:
        ldos    K_ii+ii_status,r5       # Check for open CCB communications
        bbs     iilinkCCB,r5,.mrp30     # Jif CCB is available
        call    K$twait                 # Wait for the CCB to link up
        b       .mrp20
#
.mrp30:
        ldconst enafe+enapart4,g0       # Set logging error code
        call    M$log_nvachecksum       # Log a bad checksum event
#
# --- Restore g registers and return
#
.mrp1000:
        movq    r12,g0                  # Restore registers
.ifdef HISTORY_KEEP
c CT_HISTORY_ON();
.endif  # HISTORY_KEEP
        ret
#
#**********************************************************************
#
#  NAME:  NVA_ReInitialize
#
#  INPUT:
#       None
#
#  OUTPUT:
#       None
#
#**********************************************************************
# C access
# void NVA_ReInitialize (void);
        .globl NVA_ReInitialize
NVA_ReInitialize:
        lda     P4_nvac,r3             # Get NVAC control address
        ld      nc_mapbase(r3),r4
c       s_Free(r4, NUM_OF_P4_NVA_WK/8, __FILE__, __LINE__);
        call    M$p4init
        ret
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
