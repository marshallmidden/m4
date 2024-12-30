# $Id: pmbe.as 157461 2011-08-03 15:21:36Z m4 $
#**********************************************************************
#
#  NAME: pmbe
#
#  PURPOSE:
#       Packet management for the Back End - allocating, releasing, etc
#
#  FUNCTIONS:
#       m$initp    - Packet initialization
#       M$aipw     - Assign ILT/PRP combination w/ wait
#       M$rip      - Release ILT/PRP/SGL combination
#       M$rir      - Release ILT/RRP/SGL combination
#
#       This module employs no processes.
#
#  Copyright (c) 1996-2010 XIOtech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- global function declarations ------------------------------------
#
        .globl  pm$init                 # Initialize packets
#
        .globl  M$aipw                  # Assign ILT/PRP w/ wait
        .globl  M$rip                   # Release ILT/PRP/SGL
#
        .globl  M$rir                   # Release ILT/RRP/SGL
#
#**********************************************************************
#
#  NAME: pm$init
#
#  PURPOSE:
#       To provide a means of initializing this module for this processor
#
#  DESCRIPTION:
#       An initial quantity of PRP, RRP, SCB, RPN and RRB packets
#       are preallocated from the appropriate system pool of memory.
#       These individual quantities are defined within system.inc.
#
#  CALLING SEQUENCE:
#       call    pm$init
#
#  INPUT:
#       g3 = FICB
#
#  OUTPUT:
#       None.
#
#  REGS DESTROYED:
#       g0-g3
#       g14
#
#**********************************************************************
#
pm$init:
#
# --- Preallocate initial PRP pool ------------------------------------
#
c       init_prp(IPRPS);                # Allocate initial PRPs to pool
#
# --- Preallocate initial RRP pool ------------------------------------
#
c       init_rrp(IRRPS);                # Allocate initial RRPs to pool
#
# --- Preallocate initial RPN pool ------------------------------------
#
c       init_rpn(IRPNS);                # Allocate initial RPNs to pool
#
# --- Preallocate initial RRB pool ------------------------------------
#
c       init_rrb(IRRBS);                # Allocate initial RRBs to pool
#
# --- Exit
#
        ret
#
#**********************************************************************
#
#  NAME: M$rip
#
#  PURPOSE:
#       To provide a common means of releasing an ILT/PRP/SGL combination
#       back to the system.
#
#  DESCRIPTION:
#       The PRP and ILT are always unconditionally released back to their
#       respective pools.  The associated SGL, if present, is handled
#       accordingly.  If the SGL was borrowed, the SGL is not released
#       back to the free memory pool.  If the SGL wasn't borrowed, the
#       SGL ownership count is decremented.  If the number of owners
#       goes to zero, the SGL is released back to the free memory pool.
#
#  CALLING SEQUENCE:
#       call    M$rip
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
# void PM_RelILTPRPSGL(ILT* pILT);
        .globl PM_RelILTPRPSGL
PM_RelILTPRPSGL:
        mov     g0,g1                   # ILT
# fall through
#
M$rip:
        movt    g0,r12                  # Save g0-g2
#
# --- Locate PRP and SGL
#
        ld      il_w0(g1),g2            # Get PRP
        ld      pr_sglptr(g2),g0        # Get SGL
        cmpobe  0,g0,.ri20              # If null, jump
#
# --- Check if SGL has been borrowed
#
        ld      pr_sglsize(g2),r3       # Check SGL ownership
        bbs     31,r3,.ri20             # Jif borrowed
#
# --- Check SGL ownership
#
        ldl     sg_scnt(g0),r4          # Get SGL header
        extract 16,8,r4                 # Isolate ownership count
        subo    1,r4,r4                 # Adjust number of owners
        cmpibge 0,r4,.ri10              # Jif exhausted
#
        stob    r4,sg_owners(g0)        # Update ownership count
        b       .ri20
#
# --- Release SGL but not the buffers
#
.ri10:
c       s_Free(g0, r5, __FILE__, __LINE__); # Release SGL
        mov     r13,g1                  # Restore g1
#
# --- Release PRP and ILT
#
.ri20:
.ifdef M4_DEBUG_PRP
c CT_history_printf("%s%s:%u put_prp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_PRP
c       put_prp(g2);                    # Release PRP
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
#
        movt    r12,g0                  # Restore g0-g2
#
# --- Exit
#
        ret
#
#**********************************************************************
#
#  NAME: M$rir
#
#  PURPOSE:
#       To provide a common means of releasing an ILT/RRP/SGL combination
#       back to the system.
#
#  DESCRIPTION:
#       The RRP and ILT are always unconditionally released back to their
#       respective pools.  The associated SGL, if present, is handled
#       accordingly.  If the SGL was borrowed, the SGL is not released
#       back to the free memory pool.  If the SGL wasn't borrowed, the
#       SGL ownership count is decremented.  If the number of owners
#       goes to zero, the SGL is released back to the free memory pool.
#
#  CALLING SEQUENCE:
#       call    M$rir
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
M$rir:
        movt    g0,r12                  # Save g0-g2
#
# --- Locate RRP and SGL
#
        ld      il_w0(g1),g2            # Get RRP
        ld      rr_sglptr(g2),g0        # Get SGL
        cmpobe  0,g0,.rr20              # If null, jump
#
# --- Check if SGL has been borrowed
#
        ld      rr_sglsize(g2),r3       # Check SGL ownership
        bbs     31,r3,.rr20             # Jif borrowed
#
# --- Check SGL ownership
#
        ldl     sg_scnt(g0),r4          # Get SGL header
        extract 16,8,r4                 # Isolate ownership count
        subo    1,r4,r4                 # Adjust number of owners
        cmpibge 0,r4,.rr10              # Jif exhausted
#
        stob    r4,sg_owners(g0)        # Update ownership count
        b       .rr20
#
# --- Release SGL but not the buffers
#
.rr10:
c       s_Free(g0, r5, __FILE__, __LINE__); # Release SGL
        mov     r13,g1                  # Restore g1
#
# --- Release RRP and ILT
#
.rr20:
.ifdef M4_DEBUG_RRP
c CT_history_printf("%s%s:%u put_rrp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g2);
.endif # M4_DEBUG_RRP
c       put_rrp(g2);                    # Release RRP
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u put_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       put_ilt(g1);                    # Deallocate ILT
#
        movt    r12,g0                  # Restore g0-g2
#
# --- Exit
#
        ret
#
#****************************************************************************
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
