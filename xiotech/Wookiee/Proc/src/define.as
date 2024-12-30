# $Id: define.as 157715 2011-08-23 15:37:55Z m4 $
#**********************************************************************
#
#  NAME: define.as
#
#  PURPOSE:
#       To provide common support of configuration definition requests.
#
#  FUNCTIONS:
#       D$que             - Queue define request
#       D$lunlookup       - Look up a LUN for a given SID
#       D_hashlun         - Hash a LUN -> vdisk into a SDD
#
#  Copyright (c) 1997-2010  Xiotech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- global function declarations ------------------------------------
#
        .globl  D$que                   # Queuing routine
.ifdef BACKEND
        .globl  D_alloctarg             # Allocate a target
.endif  # BACKEND
        .globl  D_hashlun               # Allocate and enter a LVM into an SDD
        .globl  D$lunlookup             # Find a LUN -> VID mapping on a server
        .globl  D$nop                   # No op
#
# --- global data declarations ----------------------------------------
#
# --- executable code -------------------------------------------------
#
        .text
#
#**********************************************************************
#
#  NAME: D$que
#
#  PURPOSE:
#       To provide a common means of queuing configuration definition
#       requests to this module.
#
#  DESCRIPTION:
#       The ILT and associated MRP are queued to the tail of the
#       executive queue.  The executive is activated to process this
#       request.  This routine may be called from either the process or
#       interrupt level.
#
#  CALLING SEQUENCE:
#       call    D$que
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
# C access
# void DEF_Que(ILT* pILT);
    .globl  DEF_Que
DEF_Que:
        mov     g0,g1
        call    D$que
        ret
#
D$que:
c       asm("   .globl  D$que");
c       asm("D$que:     ");
        lda     d_exec_qu,r11           # Get queue origin
        b       K$cque
#
#**********************************************************************
#
#  NAME: DEF_AllocServer
#
#  PURPOSE:
#       To provide a standard means of allocating the space and
#       clearing out the memory for a server record (SDD).
#
#  DESCRIPTION:
#       This function will allocate fixed memory for an SDD record
#       and will clear out the memory in anticipation of the caller
#       filling in pertinent fields.
#
#  CALLING SEQUENCE:
#       call DEF_AllocServer
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       g0 - address of the SDD.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
# C access
# SDD* DEF_AllocServer(void);
        .globl  DEF_AllocServer
DEF_AllocServer:
c       g0 = s_MallocC(sddsiz|BIT31, __FILE__, __LINE__);
        ret
#
#**********************************************************************
#
#  NAME: D_alloctarg
#
#  PURPOSE:
#       To provide a standard means of allocating the space and
#       clearing out the memory for a target record (TGD).
#
#  DESCRIPTION:
#       This function will allocate fixed memory for an TGD record
#       and will clear out the memory in anticipation of the caller
#       filling in pertinent fields.
#
#  CALLING SEQUENCE:
#       call D_alloctarg
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       g0 - address of the TGD.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
# C access
# TGD* DEF_AllocServer(void);
        .globl  DEF_AllocTarg           # C access
DEF_AllocTarg:
.ifdef BACKEND
D_alloctarg:
.endif  # BACKEND
c       g0 = s_MallocC(tgdsiz|BIT31, __FILE__, __LINE__);
        ret
#
#**********************************************************************
#
#  NAME: D_rsddlvm
#
#  PURPOSE:
#       To provide a means of releasing a specific SDD and its
#       associated LVMs back to local SRAM.
#
#  DESCRIPTION:
#       Each LVM referred to by the SDD is released back to local SRAM.
#       Then the SDD is released back to local SRAM.
#
#  CALLING SEQUENCE:
#       call    D_rsddlvm
#
#  INPUT:
#       g0 = SDD
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
# void DEF_RelSDDLVM(SDD* pSDD);
        .globl  DEF_RelSDDLVM             # C access
DEF_RelSDDLVM:
        movl    g0,r14                  # Save g0-g1
#
        ld      sd_lvm(r14),g0          # Get the list entry
        cmpobe  0,g0,.rs50              # If null, get next list
#
# --- Release LVM
#
.rs20_d:
        ld      lv_nlvm(g0),r9          # Get next pointer
c       s_Free(g0, lvsiz, __FILE__, __LINE__); # Release current LVM
#
        mov     r9,g0                   # Restore it
        cmpobne 0,g0,.rs20_d            # Process it
#
.rs50:
        ld      sd_ilvm(r14),g0         # Get the invisible list entry
        cmpobe  0,g0,.rs70              # If null, get next list
#
# --- Release LVM
#
.rs60:
        ld      lv_nlvm(g0),r9          # Get next pointer
c       s_Free(g0, lvsiz, __FILE__, __LINE__); # Release current LVM
#
        mov     r9,g0                   # Restore it
        cmpobne 0,g0,.rs60              # Process it
#
# --- Release SDD
#
.rs70:
c       s_Free(r14, sddsiz, __FILE__, __LINE__); # Release SDD
#
# --- Exit
#
        movl    r14,g0                  # Restore g0-g1
        ret
#
#**********************************************************************
#
#  NAME: D$nop
#
#  PURPOSE:
#       No operation.
#
#  CALLING SEQUENCE:
#       call    D$nop
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
D$nop:
        ldconst mrrsiz,g2               # Return data size
        ldconst deok,g1                 # Return value
        ret
#
#**********************************************************************
#
#  NAME: d$rmtwait
#
#  PURPOSE:
#       To provide a standard means of notifying a waiting process
#       that the front end processor has completed the MRP.
#
#  DESCRIPTION:
#       This function will be called as the completion routine for
#       an MRP that was sent to the FEP.
#
#  CALLING SEQUENCE:
#       call d$rmtwait
#
#  INPUT:
#       g3 - PCB of calling task.
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
.globl  DEF_RmtWait
DEF_RmtWait:
.ifdef BACKEND
d$rmtwait:
.endif  # BACKEND
#
        ldconst pcrdy,r3                # Get ready status
.ifdef HISTORY_KEEP
c CT_history_pcb("d$rmtwait setting ready pcb", g3);
.endif  # HISTORY_KEEP
        stob    r3,pc_stat(g3)          # Modify status of calling task
        ret
#
#**********************************************************************
#
#  NAME: D_hashlun
#
#  PURPOSE:
#       To provide a standard means of entering a LVM into a SDD.
#
#  DESCRIPTION:
#       This function will allocate fixed memory for an LVM record
#       and will enter it into the hash table contained within the
#       SDD.
#
#  CALLING SEQUENCE:
#       call D_hashlun
#
#  INPUT:
#       g0 - SDD
#       g1 - LUN
#       g3 - VID
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
# void DEF_HashLUN(SDD* pSDD, UINT32 lun, UINT32 vid);
        .globl  DEF_HashLUN
DEF_HashLUN:
        mov     g2,g3                   # VID
        call    D_hashlun
        ret
#
D_hashlun:
#
# --- First, allocate the LVM and fill it in
#
        mov     g0,r13                  # Save SDD pointer
#
c       g0 = s_MallocC(lvsiz|BIT31, __FILE__, __LINE__);
        stos    g1,lv_lun(g0)           # Save the LUN
        stos    g3,lv_vid(g0)           # Save the VID
#
# --- Select the list to place the mapping.  LUN 0xFF goes into the
# --- invisible list.  All others go into the normal list.
#
        ld      sd_lvm(r13),r14         # Get the first pointer
        lda     sd_lvm(r13),r15         # Get the address of the pointer
#
        ldconst 0xFF,r3                 # Test for FF
        cmpobne r3,g1,.dh00             # Jif not invisible
#
        ld      sd_ilvm(r13),r14        # Switch to the invisible list
        lda     sd_ilvm(r13),r15
#
# --- Check for NULL
#
.dh00:
        cmpobne 0,r14,.dh10             # If not NULL, then traverse
        st      g0,(r15)                # Save it and we're done
        b       .dh50

# --- Traverse the list until a NULL pointer is found.
#
.dh10:
        mov     r14,r15                 # Save the previous ptr
        ld      lv_nlvm(r14),r14        # Get next LVM
        cmpobne 0,r14,.dh10             # If not NULL, traverse
        st      g0,lv_nlvm(r15)         # Set the next pointer
#
# --- Exit
#
.dh50:
        mov     r13,g0                  # Restore it
#
# --- Record the fact that there is another LUN mapping
#
        ldos    sd_nluns(g0),r3         # Get count
        addo    1,r3,r3                 # Bump
        stos    r3,sd_nluns(g0)         # Store
        ret
#
#**********************************************************************
#
#  NAME: D$lunlookup
#
#  PURPOSE:
#       To provide a standard means of looking up a LUN to VDisk
#       mapping for a specified server.
#
#  DESCRIPTION:
#       This function will look through the hash tables for a server
#       and determine whether or not the server has this LUN mapped.
#       If so, the LUN is returned.  If not, an error LUN value is
#       returned.
#
#  CALLING SEQUENCE:
#       call D$lunlookup
#
#  INPUT:
#       g0 - SDD
#       g1 - LUN
#
#  OUTPUT:
#       g3 - VID (0xffffffff if not found)
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
D$lunlookup:
        ldconst 0xffffffff,g3           # Prep VID number
        cmpobe  0,g0,.ll100             # exit if g0 (SDD) is null
#
# --- Select the list to examine.  If the LUN is 0xFF, then the invisible
# --- list is used.
#
        ld      sd_lvm(g0),r14          # Get the first pointer
        ldconst 0xff,r3                 # Check for invisible
        cmpobne r3,g1,.ll10             # Jif not invisible
        ld      sd_ilvm(g0),r14         # Else switch lists
#
# --- Check for NULL
#
.ll10:
        cmpobe  0,r14,.ll100            # If NULL, done

# --- Traverse the list until a NULL pointer is found.
#
        ldos    lv_lun(r14),r3          # Get the LUN
        cmpobe  r3,g1,.ll50             # Jif found
#
        ld      lv_nlvm(r14),r14        # Get next LVM
        b       .ll10                   # Try again
#
.ll50:
        ldos    lv_vid(r14),g3          # LUN -> VID found
#
# --- Exit
#
.ll100:
        ret
#
#**********************************************************************
#
#  NAME: d$getii
#
#  PURPOSE:
#       To provide a standard means of retrieving the statistic and
#       configuration data from the II information area.
#
#  DESCRIPTION:
#       This function will dump the configuration and statistical data
#       from the II information area.
#
#       Assumes all space needed for return data is available.
#
#  CALLING SEQUENCE:
#       call d$getii
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
d$getii:
#
# --- First, grab the return data address
#
        ld      mr_rptr(g0),r15         # Return data pointer
#
# --- Fill in the structure with the statistical information
# --- We will round up the quad count and actually dump the extra
# --- data into the return structure, but the length will be
# --- truncated back to the actual data count.
#
        ldconst deok,g1                 # Load good return error code
#
        ldconst 0,r9                    # Pointer into data
        lda     K_ii,r8                 # Get K_ii structure address
        ldconst iisiz/16,r3             # Get quad count for the data
#
.gii10:
        ldq     (r8)[r9*1],r4           # Get quad of data
        stq     r4,mgi_ii(r15)[r9*1]    # Store it
#
        lda     16(r9),r9               # Bump the pointer
        subo    1,r3,r3                 # Decrement the count of quads
        cmpobne 0,r3,.gii10             # Jif more to do
#
        ldconst mgi_ii+iisiz,g2         # Set up return size
#
# --- Exit
#
        ret
#
#**********************************************************************
#
#  NAME: d$getlink
#
#  PURPOSE:
#       To provide a standard means of retrieving the statistic and
#       configuration data from the PCI information area.
#
#  DESCRIPTION:
#       This function will dump the configuration and statistical data
#       from the PCI information area.
#
#       Assumes all space needed for return data is available.
#
#  CALLING SEQUENCE:
#       call d$getlink
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
d$getlink:
#
# --- First, grab the return data address
#
        ld      mr_rptr(g0),r15         # Return data pointer
#
# --- Fill in the structure with the statistical information
# --- We will round up the quad count and actually dump the extra
# --- data into the return structure, but the length will be
# --- truncated back to the actual data count.
#
        ldconst deok,g1                 # Load good return error code
#
        ldconst 0,r9                    # Pointer into data
        ld      L_stattbl,r8            # Get link stats structure address
        ldconst lssize/16,r3            # Get quad count of data
#
.gpi10:
        ldq     (r8)[r9*1],r4           # Get quad of data
!       stq     r4,mfp_data(r15)[r9*1]  # Store it
#
        lda     16(r9),r9               # Bump the pointer
        subo    1,r3,r3                 # Decrement the count of quads
        cmpobne 0,r3,.gpi10             # Jif more to do
#
        ldconst mfp_data+lssize,g2      # Set up return size
#
# --- Exit
#
        ret
#
#**********************************************************************
#
#  NAME: d$getproc
#
#  PURPOSE:
#       To provide a standard means of retrieving the firmware header.
#
#  DESCRIPTION:
#       This function will dump the firmware header.
#
#       Assumes all space needed for return data is available.
#
#  CALLING SEQUENCE:
#       call    d$getproc
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

d$getproc:
c       r8 = (ulong)&fwHeader;          # Get proc code header starting address
#
# --- First, grab the return data address
#
        ld      mr_rptr(g0),r15         # Return data pointer
#
# --- Fill in the structure with the firmware header information
# --- We will round up the quad count and actually dump the extra
# --- data into the return structure, but the length will be
# --- truncated back to the actual data count.
#
        ldconst deok,g1                 # Load good return error code
c       memcpy((void *)(r15 + mfh_data), (void *)r8, fh_size);
#
        ldconst mfhrsiz,g2              # Set up return size
#
# --- Exit
#
        ret
#
#**********************************************************************
#
#  NAME: d$rwmemory
#
#  PURPOSE:
#       To provide a standard means of reading or writing memory
#       across the PCI bus.
#       This can also be used to copy memory within the same processor.
#
#  DESCRIPTION:
#       This function will read or write memory. The addresses must
#       be word aligned and the length must be a multiple of 4 bytes (word).
#
#       Assumes all space needed for return data is available.
#
#  CALLING SEQUENCE:
#       call d$rwmemory
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
d$rwmemory:
        mov     g0,r14                  # save regs
#
        ld      mr_ptr(g0),g0           # Parm block address within MRP
        ld      mrw_daddr(g0),r4        # destination addr
        ld      mrw_saddr(g0),r5        # source addr
        ld      mrw_length(g0),r6       # length in bytes

        ldconst 0x80000000,r3           # force flag mask
        and     r6,r3,r7                # isolate possible force flag
        notand  r6,r3,r6                # remove force flag from length
#
# --- validate memory addresses memory map
#     bypassing all tests if force is active
#
        cmpobne 0,r7,.drwm30            # Jif if force active

        ldconst deinvop,g1              # default to error status

# --- validate the destination address

c       if (r4 >= (ulong)&__executable_start && r4 < (ulong)&etext) {
c           if (r4 + r6 >= (ulong)&__executable_start && r4 + r6 < (ulong)&etext) {
                b   .drwm20
c           }
c       }
c       if (r4 >= (ulong)&__preinit_array_start && r4 < (ulong)&edata) {
c           if (r4 + r6 >= (ulong)&__preinit_array_start && r4 + r6 < (ulong)&edata) {
                b   .drwm20
c           }
c       }
c       if (r4 >= (ulong)&__bss_start && r4 < (ulong)&_end) {
c           if (r4 + r6 >= (ulong)&__bss_start && r4 + r6 < (ulong)&_end) {
                b   .drwm20
c           }
c       }
c       if (r4 >= (ulong)FE_BASEADDR && r4 < (ulong)FE_SHARE_LIM) {
c           if (r4 + r6 >= (ulong)FE_BASEADDR && r4 + r6 < (ulong)FE_SHARE_LIM) {
                b   .drwm20
c           }
c       }
c       if (r4 >= (ulong)BE_BASEADDR && r4 < (ulong)BE_SHARE_LIM) {
c           if (r4 + r6 >= (ulong)BE_BASEADDR && r4 + r6 < (ulong)BE_SHARE_LIM) {
                b   .drwm20
c           }
c       }
c       if (r4 >= (ulong)CCB_BASEADDR && r4 < ((ulong)CCB_BASEADDR + MAKE_DEFS_CCB_size)) {
c           if (r4 + r6 >= (ulong)CCB_BASEADDR && r4 + r6 < ((ulong)CCB_BASEADDR + MAKE_DEFS_CCB_size)) {
                b   .drwm20
c           }
c       }
c       if (r4 >= (ulong)NVSRAM && r4 < ((ulong)NVSRAM + MAKE_DEFS_NVR_size)) {
c           if (r4 + r6 >= (ulong)NVSRAM && r4 + r6 < ((ulong)NVSRAM + MAKE_DEFS_NVR_size)) {
                b   .drwm20
c           }
c       }
c       if (r4 >= (ulong)INFOREGION_BASE_ADDR && r4 < ((ulong)INFOREGION_BASE_ADDR + MAKE_DEFS_INFO_size)) {
c           if (r4 + r6 >= (ulong)INFOREGION_BASE_ADDR && r4 + r6 < ((ulong)INFOREGION_BASE_ADDR + MAKE_DEFS_INFO_size)) {
                b   .drwm20
c           }
c       }
c       if (r4 >= (ulong)&local_memory_start && r4 < ((ulong)&local_memory_start + PRIVATE_SIZE)) {
c           if (r4 + r6 >= (ulong)&local_memory_start && r4 + r6 < ((ulong)&local_memory_start + PRIVATE_SIZE)) {
                b   .drwm20
c           }
c       }
c fprintf(stderr, "%s%s:%u Memory read/write address (%p) is out of range.\n", FEBEMESSAGE, __FILE__, __LINE__, (void *)r4);
        b   .drwm60

# --- validate the source address

.drwm20:
c       if (r5 >= (ulong)&__executable_start && r5 < (ulong)&etext) {
c           if (r5 + r6 >= (ulong)&__executable_start && r5 + r6 < (ulong)&etext) {
                b   .drwm30
c           }
c       }
c       if (r5 >= (ulong)&__preinit_array_start && r5 < (ulong)&edata) {
c           if (r5 + r6 >= (ulong)&__preinit_array_start && r5 + r6 < (ulong)&edata) {
                b   .drwm30
c           }
c       }
c       if (r5 >= (ulong)&__bss_start && r5 < (ulong)&_end) {
c           if (r5 + r6 >= (ulong)&__bss_start && r5 + r6 < (ulong)&_end) {
                b   .drwm30
c           }
c       }
c       if (r5 >= (ulong)FE_BASEADDR && r5 < (ulong)FE_SHARE_LIM) {
c           if (r5 + r6 >= (ulong)FE_BASEADDR && r5 + r6 < (ulong)FE_SHARE_LIM) {
                b   .drwm30
c           }
c       }
c       if (r5 >= (ulong)BE_BASEADDR && r5 < (ulong)BE_SHARE_LIM) {
c           if (r5 + r6 >= (ulong)BE_BASEADDR && r5 + r6 < (ulong)BE_SHARE_LIM) {
                b   .drwm30
c           }
c       }
c       if (r5 >= (ulong)CCB_BASEADDR && r5 < (ulong)(CCB_BASEADDR + MAKE_DEFS_CCB_size)) {
c           if (r5 + r6 >= (ulong)CCB_BASEADDR && r5 + r6 < (ulong)(CCB_BASEADDR + MAKE_DEFS_CCB_size)) {
                b   .drwm30
c           }
c       }
c       if (r5 >= (ulong)NVSRAM && r5 < (ulong)(NVSRAM + MAKE_DEFS_NVR_size)) {
c           if (r5 + r6 >= (ulong)NVSRAM && r5 + r6 < (ulong)(NVSRAM + MAKE_DEFS_NVR_size)) {
                b   .drwm30
c           }
c       }
c       if (r5 >= (ulong)INFOREGION_BASE_ADDR && r5 < (ulong)(INFOREGION_BASE_ADDR + MAKE_DEFS_INFO_size)) {
c           if (r5 + r6 >= (ulong)INFOREGION_BASE_ADDR && r5 + r6 < (ulong)(INFOREGION_BASE_ADDR + MAKE_DEFS_INFO_size)) {
                b   .drwm30
c           }
c       }
c       if (r5 >= (ulong)&local_memory_start && r5 < ((ulong)&local_memory_start + PRIVATE_SIZE)) {
c           if (r5 + r6 >= (ulong)&local_memory_start && r5 + r6 < ((ulong)&local_memory_start + PRIVATE_SIZE)) {
                b   .drwm30
c           }
c       }
c fprintf(stderr, "%s%s:%u Memory read/write address (%p) is out of range.\n", FEBEMESSAGE, __FILE__, __LINE__, (void *)r5);
        b   .drwm60
#
# --- If the destination address is located in NVRAM, then the copy
# --- should be done byte-by-byte, using M$bytecpy
#
.drwm30:
        ldconst NVSRAM,r3               # Total size of NVRAM area
        cmpobl  r4,r3,.drwm40           # Jif Dest. addr is < NVRAM addr
        ldconst NVSRAM+NVSRAMSIZ,r3     # End of NVRAM area
        cmpobge r4,r3,.drwm40           # Jif Dest. addr is > NVRAM addr
#
# --- The destination address lies within NVRAM, do a byte copy
#
        mov     r4,g5                   # Load destination address
        ld      mrw_saddr(g0),g4        # Load source address
        ld      mrw_length(g0),g3       # Load length in bytes

        ldconst 0x80000000,r3           # force flag mask
        notand  g3,r3,g3                # remove force flag from length
#
c       r8 = getpagesize();
#
c       memcpy((void*)g5, (void*)g4, g3);
#       No need to either push or pop g-regs, as the following is a system call
#       g5 - Destination address
#       g3 - Length in bytes
c       r8 = g5%r8;
c       r9 = msync((void*)(g5 - r8), g3 + r8, MS_SYNC);
c       if (r9 != 0) fprintf(stderr, "d$rwmemory:  msync failed, errno = %d\n", errno);
#
        b       .drwm50                 # Go to end

.drwm40:
c       memcpy((void*)r4, (void*)r5, r6);
#
# --- Fill in the structure with the statistical information
# --- We will round up the quad count and actually dump the extra
# --- data into the return structure, but the length will be
# --- truncated back to the actual data count.
#
.drwm50:
        ldconst deok,g1                 # Load good return error code

.drwm60:
        ldconst mrwrsiz,g2              # Set up return size
#
# --- Exit
#
        mov     r14,g0                  # restore regs
        ret
#
#**********************************************************************
#
#  NAME: d$obsolete
#
#  PURPOSE:
#       For unsupported functions.
#
#  CALLING SEQUENCE:
#       call d$obsolete
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
d$obsolete:
#
        ldconst deinvpkttyp,g1          # Set error code
        ldconst mrrsiz,g2               # Set length
        ret
#
#**********************************************************************
#
#  NAME: d$configtarg
#
#  PURPOSE:
#       To provide a standard means of configuring data
#       for a target.
#
#  DESCRIPTION:
#       This function will take the information in the input parameters
#       and either create, delete or update a target record.
#
#       If the WWNs are zero, then the target will be deleted.
#
#       Since this is used on the front and back end, there is a quirk
#       to update function.  On the front end, there may not be a
#       record for the target, so a target record will have to be created
#       and recorded in the table.  This will happen when a create is
#       done on the BEP and reflected forward to the FEP.
#
#  CALLING SEQUENCE:
#       call d$configtarg
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
.ifdef BACKEND
d$configtarg:
        ldconst MAXTARGETS,r10          # Max target ID
        lda     T_tgdindx,r11           # Base address of the TGDX
#
# --- First, grab the return data address and length allowed.
#
        ld      mr_rptr(g0),r15         # Return data pointer
#
        ldconst mrrsiz,g2               # Set up return size
#
        ld      mr_ptr(g0),g0           # Parm block address
        ldos    mct_tid(g0),r12         # Target ID
#
# --- Validate parms
#
        ldconst deinvtid,g1             # Load error code
        cmpoble r10,r12,.ctd100         # Branch if too big TID
#
# --- Update entry processing
#
        ld      tgx_tgd(r11)[r12*4],r8  # Get the TGD

.ifdef BACKEND
        cmpobe  0,r8,.ctd100            # Null pointer, invalid device
#
        ld      mct_mod(g0),r10         # Get field modifier
        cmpobne 0,r10,.ctd20            # Jif some modifier bits set.
        ldconst 0xFFFF,r10              # Set to modify all fields.
#
.ctd20:
        bbc     mctmodport,r10,.ctd30   # Jif not modifying port
        ldob    mct_chan(g0),r4         # Get port number from input record
        stob    r4,tgd_port(r8)         # Save it
#
.ctd30:
        bbc     mctmodopt,r10,.ctd40    # Jif not modifying options
        ldob    mct_opt(g0),r3          # Get options from input record
        ldob    tgd_opt(r8),r4          # Get options from target
        modify  1,r3,r4
        stob    r4,tgd_opt(r8)          # Save it
        ldob    mct_fcid(g0),r4         # Get Hard ID from input record
        stob    r4,tgd_fcid(r8)         # Save it
#
.ctd40:
        bbc     mctmodlock,r10,.ctd50   # Jif not modifying options
        ldob    mct_lock(g0),r4         # Get Lock from input record
        stob    r4,tgd_lock(r8)         # Save it
#
.ctd50:
        bbc     mctmodown,r10,.ctd60    # Jif not modifying owner
        ld      mct_owner(g0),r4        # Get owner from input record
        st      r4,mct_owner(r8)        # Save it
#
.ctd60:
        bbc     mctmodcluster,r10,.ctd70 # Jif not modifying cluster
        ldos    mct_cluster(g0),r4      # Get cluster from input record
        stos    r4,tgd_cluster(r8)      # Save it
.ctd70:
#
# --- Update FE record
#
        mov     r12,g0                  # Target ID
        ldconst FALSE,g1                # Do not delete
        call    D_updrmttarg            # Update FEP
#
# --- Update NVRAM
#
        call    D$p2updateconfig
.endif  # BACKEND
#
        ldconst deok,g1                 # Exit
        ldconst mctrsiz,g2              # Set up return size
#
# --- Exit
#
.ctd100:
        ret
.endif /* BACKEND */
#
#**********************************************************************
#
#  NAME: d$modepage
#
#  PURPOSE:
#       To function will handle the mode page MRP from the CCB.
#       This checks the input masks and sets the appropriate property
#       based on the data.
#
#  DESCRIPTION:
#       The mode page MRP is intended for internal debug. Masked
#       bits are the properties being configured and data selects
#       the state.
#
#  CALLING SEQUENCE:
#       call d$modepage
#
#  INPUT:
#       g0 - MRP
#
#  OUTPUT:
#       g1 - error code - always deok
#       g2 - length of return packet
#
#  REGS DESTROYED:
#       g0
#
#**********************************************************************
#
d$modepage:
        ld      mr_ptr(g0),g0           # Parm block address
#
# --- Word 0
#
        ld      mmp_data0(g0),r12       # Get data
        ld      mmp_mask0(g0),r13       # Get mask
#
# --- Enable heartbeat checking
#
        bbc     mmp0dsblheartbeat,r13,.dmp20 # Jif mask not set - do nothing
        ldconst TRUE,r3                 # Assume enable
        bbs     mmp0dsblheartbeat,r12,.dmp10 # Jif if disabled
        ldconst FALSE,r3                # Disable it
.dmp10:
        st      r3,hbeat_disable        # Save it
.dmp20:
#
# --- Disable error trap handling by boot code
#
        bbc     mmp0disbooterrtrap,r13,.dmp40 # Jif mask not set - do nothing
        ldconst FALSE,r3                # Assume disable
        bbs     mmp0disbooterrtrap,r12,.dmp30 # Jif if disabled
        ldconst TRUE,r3                 # Enable it
.dmp30:
        st      r3,boothandlerenable    # Save it
.dmp40:
#
# --- Ignore CCB heartbeat failure because the controller is shutting down
#
        bbc     mmp0ctrlshutdown,r13,.dmp60 # Jif mask not set - do nothing
        ldconst FALSE,r3                # Assume disable
        bbc     mmp0ctrlshutdown,r12,.dmp50 # Jif if disabled
        ldconst TRUE,r3                 # Enable it
.dmp50:
        st      r3,ctrl_shutdown        # Save it
.dmp60:
#
# --- Word 1,2,3 not currently used
#
# --- Exit
#
        mov     deok,g1                 # Always return good
        ldconst mmpsiz,g2               # Set up return size
        ret
#
#**********************************************************************
#
#  NAME: d$cbridge
#
#  PURPOSE:  This module is the bridge to define functions
#            written in 'C'.
#
#  DESCRIPTION:  The g registers are saved. The function is run via C code.
#                The registers are restored and the return value is set.
#
#  CALLING SEQUENCE:
#       call    d$cbridge
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
d$cbridge:

        mov     g0,r15                  # save g0
        mov     g3,r3                   # save g3
        movq    g4,r4                   # save g4-g7
        movq    g8,r8                   # save g8-g11
        movt    g12,r12                 # save g12-g14

        call    DEF_CBridge
        mov     g0,g1                   # Get return status code

        mov     r15,g0                  # restore g0
        mov     r3,g3                   # restore g3
        movq    r4,g4                   # restore g4-g7
        movq    r8,g8                   # restore g8-g11
        movt    r12,g12                 # restore g12-g14

        ld      mr_rptr(g0),r3          # Get the return data ptr
!       ld      mr_rlen(r3),g2          # Get return packet size

        ret
#
#**********************************************************************
#
#  NAME: d$forceerrortrap
#
#  PURPOSE:
#       This function forks off a process that will call errtrap and
#       then returns a good return status.
#
#  DESCRIPTION:
#       This function is called from MRPs 0x14A or 0x51F to allow a
#       developer to force the FE (0x51F) or the BE (0x14A) down the
#       error trap path.  The error type is err00 (0x20).
#
#  CALLING SEQUENCE:
#       call    d$forceerrortrap
#
#  INPUT:
#       g0 = MRP
#
#  OUTPUT:
#       g1 = status
#       g2 = return length
#
#  REGS DESTROYED:
#       NONE
#
#**********************************************************************
#
d$forceerrortrap:
        mov     g0,r12                  # Save g0
#
# --- Fork off process to force an error trap after 3 seconds
#
        lda     d$call_errtrap,g0       # Load process address
        ldconst DERRTRAPPRIO,g1         # Load priority
c       CT_fork_tmp = (ulong)"d$call_errtrap";
        call    K$tfork                 # Spawn a proc for this PSD in the raid
#
# --- Exit
#
        mov     deok,g1                 # Always return good
        ldconst metrsiz,g2              # Set up return size
        mov     r12,g0                  # Restore g0
        ret
#
#**********************************************************************
#
#  NAME: d$call_errtrap
#
#  PURPOSE:
#       This function waits 3 seconds, then calls .err00.
#
#  DESCRIPTION:
#       This function is forked from d$forceerrtrap.
#
#
#  CALLING SEQUENCE:
#       K$tfork  d$call_errtrap
#
#  INPUT:
#       NONE
#
#  OUTPUT:
#       NONE
#
#  REGS DESTROYED:
#       NONE
#
#**********************************************************************
#
d$call_errtrap:
#
        ldconst 3000,g0                 # 3 second delay
        call    K$twait                 # block for 3 sec
#
# --- Branch to .err00
#
        b       .err00
#
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
