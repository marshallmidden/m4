# $Id: pm.as 147979 2010-09-22 23:07:18Z m4 $
#**********************************************************************
#
#  NAME: pm.as
#
#  PURPOSE:
#       Packet Management - allocating, releasing, etc
#
#  FUNCTIONS:
#       M$aivw     - Assign ILT/VRP combination w/ wait
#       M$riv      - Release ILT/VRP/SGL combination
#
#       M$mrgsgls  - Merge 2 SGLs into 1 SGL
#       M$movsgls  - Move src SGL to dst SGL
#       M$clrsgl   - Clear SGL data buffer
#
#       M$rsglbuf  - Release SGL and buffer
#       M$asglwobuf- Assign SGL to a buffer
#       M$rsglwobuf- Release SGL to a buffer
#
#       This module employs no processes.
#
#  Copyright (c) 1996-2010 XIOtech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- global function declarations ------------------------------------
#
.ifdef FRONTEND
        .globl  M$aivw                  # Assign ILT/VRP combination w/ wait
.endif # FRONTEND
        .globl  M$riv                   # Release ILT/VRP combination
#
        .globl  M$rsglbuf               # Release SGL and buffer
.ifdef BACKEND
        .globl  M$clrsgl                # Clear SGL data buffer
        .globl  M$asglwobuf             # Assign SGL to a buffer w/ wait
        .globl  M$rsglwobuf             # Release SGL
.endif  # BACKEND
#
        .text
# --- executable code -------------------------------------------------
#
.ifdef BACKEND
#**********************************************************************
#
#  NAME: M$clrsgl
#
#  PURPOSE:
#       To provide a common means of efficiently clearing the data
#       buffer(s) associated with an SGL to zeroes.
#
#  DESCRIPTION:
#       The data buffer(s) referenced by the SGL is cleared using
#       quad-word stores whenever possible.  Each descriptor within the
#       SGL must reference some multiple of 4 bytes.
#
#  CALLING SEQUENCE:
#       call    M$clrsgl
#
#  INPUT:
#       g0 = SGL
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
# void PM_ClearSGL(SGL* pSGL);
        .globl  PM_ClearSGL
PM_ClearSGL:
#
M$clrsgl:
#
# --- Initialize descriptor information
#
        ldos    sg_scnt(g0),r15         # Get segment count
        lda     sg_desc0(g0),r14        # Get descriptor ptr
#
# --- Get next descriptor
#
.cx10:
        ldl     sg_addr(r14),r12        # Get next descriptor
        lda     sgdescsiz(r14),r14      # Advance descriptor ptr
        subo    1,r15,r15               # Adjust remaining src segment count
c       memset((void *)r12, 0, r13);
        cmpibl  0,r15,.cx10             # Jif more descriptors
# --- Exit
        ret
.endif  # BACKEND
#
#**********************************************************************
#
#  NAME: M$movsgls
#
#  PURPOSE:
#       To provide a common means of moving the data from the data buffers
#       described by the source SGL to the data buffers described by the
#       destination SGL.
#
#  DESCRIPTION:
#       The data buffer described by the source SGL is moved to the data
#       buffer described by the destination SGL.  The actual movement of
#       data is controlled by an unrolled loop which moves 1 sector's
#       worth of data per loop iteration.
#
#       Both SGLs are assumed to reference the same amount of data.
#       Each SGL, however, may employ a different number of descriptors
#       to describe the actual data.  Each descriptor within each SGL
#       must reference some multiple of the sector size and must be
#       cache line aligned.
#
#       The src and dst data buffers will typically be resident within
#       the Read DRAM and/or Write DRAM.
#
#  CALLING SEQUENCE:
#       call    M$movsgls
#
#  INPUT:
#       g0 = source SGL
#       g1 = destination SGL
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
# void PM_MoveSGL(SGL* pSourceSGL, SGL* pDestinationSGL);
        .globl  PM_MoveSGL
PM_MoveSGL:
#
# --- Initialize src and src/dst descriptor information
#
        ldos    sg_scnt(g0),r15         # Get src segment count
.ifdef M4_DEBUG_SGL
        ldos    sg_scnt(g1),r3          # Get destination segment count
.endif /* M4_DEBUG_SGL */
        lda     sg_desc0(g0),r14        # Get 1st src descriptor ptr
        lda     sg_desc0(g1),r13        # Get 1st src/dst descriptor ptr
        ldconst SECSIZE/2,r12           # Get 1/2 sector size
#
        ldl     sg_addr(r14),r10        # Get 1st src descriptor
        lda     sgdescsiz(r14),r14      # Advance src descriptor ptr
        ldl     sg_addr(r13),r8         # Get 1st src/dst descriptor
        lda     sgdescsiz(r13),r13      # Advance src/dst descriptor ptr
        subo    1,r15,r15               # Adjust remaining src segment count
#
# --- Move next sector double buffered (unrolled loop)
#
.xm10:
c       memcpy((CHAR *)r8, (CHAR *)r10, r12);
#
# --- Update src information
#
        lda     SECSIZE/2(r10),r10      # Bump src sector ptr
        subo    r12,r11,r11             # Calculate remaining byte count
#
# --- Update src/dst information
#
        lda     SECSIZE/2(r8),r8        # Bump src/dst sector ptr
        subo    r12,r9,r9               # Calculate remaining byte count
#
# --- Check current src descriptor
#
        cmpibl  0,r11,.xm20             # Jif more bytes left
        cmpobe  0,r15,.xm100            # Jif done
#
# --- Advance to next src descriptor
#
        ldl     sg_addr(r14),r10        # Get next src descriptor
        subo    1,r15,r15               # Adjust remaining src segment count
        lda     sgdescsiz(r14),r14      # Advance src descriptor ptr
#
# --- Check current src/dst descriptor
#
.xm20:
        cmpibl  0,r9,.xm10              # Jif more bytes left
#
# --- Advance to src/dst next descriptor
#
        ldl     sg_addr(r13),r8         # Get next src/dst descriptor
        lda     sgdescsiz(r13),r13      # Advance src/dst descriptor ptr
.ifdef M4_DEBUG_SGL
# Check that the destination is big enough to expand into.
c r3 = r3 - 1;
c if (r3 == 0) {
c   abort();
c }
.endif /* M4_DEBUG_SGL */
        b       .xm10
#
# --- Exit
#
.xm100:
        ret

.ifdef FRONTEND
#**********************************************************************
#
#  NAME: M$aivw
#
#  PURPOSE:
#       To provide a common means of assigning and linking an ILT/VRP
#       combination.
#
#  DESCRIPTION:
#       An ILT packet is assigned with wait from the system.  A VRP
#       packet is also assigned with wait from the system and linked
#       with the ILT.
#
#  CALLING SEQUENCE:
#       call    M$aivw
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       g1 = ILT
#       g2 = VRP
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
M$aivw:
#
# --- Assign ILT
#
c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
#
# --- Assign VRP
#
c       g2 = get_vrp();                 # Allocate a VRP
.ifdef M4_DEBUG_VRP
c CT_history_printf("%s%s:%u get_vrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_VRP
        ret
.endif # FRONTEND

#**********************************************************************
#
#  NAME: M$riv
#
#  PURPOSE:
#       To provide a common means of releasing an ILT/VRP/SGL combination
#       back to the system.
#
#  DESCRIPTION:
#       The VRP and ILT are always unconditionally released back to their
#       respective pools.  The associated SGL, if present, is handled
#       accordingly.  If the SGL was borrowed, the SGL is not released
#       back to the free memory pool.  If the SGL wasn't borrowed, the
#       SGL ownership count is decremented.  If the number of owners
#       goes to zero, the SGL is released back to the free memory pool.
#
#  CALLING SEQUENCE:
#       call    M$riv
#
#  INPUT:
#       g1 = ILT
#       g2 = VRP
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
.ifdef BACKEND
# C access
# void PM_RelILTVRPSGL(ILT* pILT, VRP* pVRP);
        .globl  PM_RelILTVRPSGL
PM_RelILTVRPSGL:
        mov     g1,g2
        mov     g0,g1
# fall through
.endif  # BACKEND
#
M$riv:
        mov     g0,r12                  # Save g0
#
# --- Locate VRP and SGL
#
        ld      vr_sglptr(g2),g0        # Get SGL
c       if (g0 == 0xfeedf00d) {
c           fprintf(stderr,"%s%s:%u M$riv sgl 0xfeedf00d\n", FEBEMESSAGE, __FILE__, __LINE__);
c           abort();
c       }
        mov     g1,r13                  # Save g1
        mov     g2,r14                  # Save g2
        cmpobe.f 0,g0,.mrr50            # If null, jump
#
# --- Check if SGL has been borrowed
#
        ld      vr_sglsize(g2),r3       # Check SGL ownership
        bbs.t   31,r3,.mrr50            # Jif borrowed
#
# --- Only release the SGL if it is not part of the VRP
#
        lda     VRPALLOC(g2),r3         # Get the end address + 1 of the VRP
        cmpobge g0,r3,.mrr05            # Jif SGL beyond the VRP
        cmpobge g0,g2,.mrr50            # Jif SGL is in the VRP
#
# --- Check SGL ownership
#
.mrr05:
        ldl     sg_scnt(g0),r4          # Get SGL header
        extract 16,8,r4                 # Isolate ownership count
        subo    1,r4,r4                 # Adjust number of owners
        cmpibge.f 0,r4,.mrr10           # Jif exhausted
#
        stob    r4,sg_owners(g0)        # Update ownership count
        b       .mrr50
#
# --- Release SGL but not the buffers
#
.mrr10:
c       s_Free(g0, r5, __FILE__, __LINE__); # Release SGL
        mov     r13,g1                  # Restore g1
#
# --- Release VRP and ILT
#
.mrr50:
.ifdef M4_DEBUG_VRP
c CT_history_printf("%s%s:%u put_vrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_VRP
c       put_vrp(g2);                    # Deallocate VRP
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
#
        mov     r12,g0                  # Restore g0
        mov     r13,g1                  # Restore g1
        mov     r14,g2                  # Restore g2
#
# --- Exit
#
        ret
#
.ifdef FRONTEND
#**********************************************************************
#
#  NAME: PM_RelILT2
#
#  PURPOSE:
#       Provide an i960 completion routine to free an ILT (see ispc.c).
#
#  CALLING SEQUENCE:
#       call    PM_RelILT2
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
# void PM_RelILT2(g1=ILT* pILT)
        .globl  PM_RelILT2
PM_RelILT2:
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                # Deallocate an ILT.
        ret
.endif  # FRONTEND
#
#**********************************************************************
#
#  NAME: M$rsglbuf
#
#  PURPOSE:
#       To provide a common means of releasing a combined SGL and data buffer.
#
#  DESCRIPTION:
#       The combined SGL and associated read buffer/write buffer is
#       released back to the Read DRAM pool.
#
#       This routine can only be called from the process level.
#
#  CALLING SEQUENCE:
#       call    M$rsglbuf
#
#  INPUT:
#       g0 = SGL
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
M$rsglbuf:
c       PM_RelSGLWithBuf((void *)g0);
        ret

.ifdef BACKEND
#**********************************************************************
#
#  NAME: M$asglwobuf
#
#  PURPOSE:
#       To provide a common means of assigning an SGL without a buffer
#       from Read DRAM.
#
#  DESCRIPTION:
#       An SGL is assigned within the Read DRAM and is set to point to the
#       buffer address passed in.
#
#       This routine will block if memory resources are not available.
#
#       This routine can only be called from the process level.
#
#  CALLING SEQUENCE:
#       call    M$asglwobuf
#
#  INPUT:
#       g0 = byte count
#       g3 = buffer pointer
#
#  OUTPUT:
#       g0 = SGL
#
#**********************************************************************
#
M$asglwobuf:
#
# --- Allocate SGL
#
        ldconst 1,r4                    # Set up descriptor count
        ldconst sghdrsiz+sgdescsiz,r5   # Set up SGL size
        mov     g3,r6                   # Buff address
        mov     g0,r7                   # Buff length
#
c       g0 = s_MallocW(r5, __FILE__, __LINE__); # Allocate SGL and buffer
#
# --- Initialize SGL
#
        stq     r4,sg_scnt(g0)
#
# --- Exit
#
        ret
#
#**********************************************************************
#
#  NAME: M$rsglwobuf
#
#  PURPOSE:
#       To provide a common means of releasing an SGL but not the buffer.
#
#  DESCRIPTION:
#       The SGL is released back to the Read DRAM pool.
#
#       This routine can only be called from the process level.
#
#  CALLING SEQUENCE:
#       call    M$rsglwobuf
#
#  INPUT:
#       g0 = SGL
#
#  OUTPUT:
#       None.
#
#**********************************************************************
#
M$rsglwobuf:
#
# --- Release combined SGL and data buffer
#
c       s_Free(g0, sghdrsiz+sgdescsiz, __FILE__, __LINE__); # Release SGL
#
# --- Exit
#
        ret
.endif  # BACKEND
#
#******************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
