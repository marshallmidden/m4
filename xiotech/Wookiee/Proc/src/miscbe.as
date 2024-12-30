# $Id: miscbe.as 145891 2010-08-18 22:53:05Z m4 $
#**********************************************************************
#
#  NAME: miscbe
#
#  PURPOSE:
#       Support module for Back End processor specific MISC functions
#
#  FUNCTIONS:
#       M$chkstat  - Check PRP status
#       M$gpdelay  - Give up control via priority
#
#       This module employs no processes.
#
#  Copyright (c) 1996-2010 XIOtech Corporation. All rights reserved.
#
#**********************************************************************
#
# --- global function declarations ------------------------------------
#
        .globl  M$chkstat               # Check PRP status
        .globl  M$gpdelay               # Give up control via priority
#
# --- global usage data definitions -----------------------------------
#
        .data
        .align  2                       # Word aligned storage
#
# --- local usage data definitions ------------------------------------
#
#
# --- executable code -------------------------------------------------
#
        .text
#
#**********************************************************************
#
#  NAME: M$chkstat
#
#  PURPOSE:
#       To provide a common means of checking the status of a completed
#       PRP.
#
#  DESCRIPTION:
#       The PRP status return is interrogated and passed onto the
#       calling routine. Any non-zero return indicates an error.
#
#  CALLING SEQUENCE:
#       call    M$chkstat
#
#  INPUT:
#       g2 = PRP
#
#  OUTPUT:
#       g0 = pr_rstatus
#
#  REGS DESTROYED:
#       None.
#
#**********************************************************************
#
# C Access
# UINT8 MSC_ChkStat(PRP* pPRP)
        .globl  MSC_ChkStat
MSC_ChkStat:
        mov     g0,g2                   # PRP
# fall through
#
M$chkstat:
#
# --- Interrogate status return
#
        ldob    pr_rstatus(g2),g0       # Get status return
        cmpobe  ecok,g0,.cs100          # Jif OK
        cmpobe  ecbebusy,g0,.cs100      # Jif ISE BUSY
#
        ldob    pr_sstatus(g2),r3       # Get SCSI status byte
        cmpobne 2,r3,.cs100             # Jif not check condition
#
# --- Process error by sense key
#
        ldob    pr_sense+2(g2),r4       # Get sense key and isolate
        and     0x0f,r4,r4
        cmpobe  0,r4,.cs20              # Jif no error
        cmpobe  1,r4,.cs20              # Jif recovered error
        cmpobe  5,r4,.cs100             # Jif illegal command
        cmpobe  6,r4,.cs100             # Jif unit attention
        cmpobe  2,r4,.cs100             # Jif not ready
#
# --- Attempt to update PDD error count
#
        movt    g0,r12                  # Save g0/g1/g2
        ldob    pr_channel(g2),g0       # Pass channel
        ld      pr_id(g2),g1            # Pass ID
        ldos    pr_lun(g2),g2           # Pass LUN
        call    D$findpdd               # Lookup PDD
        cmpobe  0,g0,.cs10              # Jif not found
#
        ld      pd_error(g0),r5         # Bump PDD error count
        addo    1,r5,r5
        st      r5,pd_error(g0)
#
.cs10:
        movt    r12,g0                  # Restore g0/g1/g2
        b       .cs100
#
# --- Force OK status for recovered error
#
.cs20:
        mov     ecok,g0                 # Set status to OK - will copy to g0
#
# --- Exit
#
.cs100:
        ret
#
#**********************************************************************
#
#  NAME: M$gpdelay
#
#  PURPOSE:
#       To provide a common means of delaying a process based upon the
#       current global priority and overall system activity.
#
#  DESCRIPTION:
#       The global priority and outstanding request count from the RAID
#       layer are used to calculate the amount of delay, if any, to
#       perform at this time. If the system is not active or has light
#       activity a process exchange suffices, otherwise a timed wait is
#       used to regulate activity.
#
#  CALLING SEQUENCE:
#       call    M$gpdelay
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
M$gpdelay:
        mov     g0,r15                  # Save g0
#
# --- Don't delay under these conditions:
#     - no outstanding RAID requests
#     - global priority is maximized
#     - processor has spare utilization (commented out for future use)
#
#       ldob    K_ii+ii_utzn,r5         # Load spare processor utilization %
#       ldconst 40,r6                   # Load threshold %
#       cmpobge r5,r6,.gp90             # Jif plenty of spare processor power
#
        ld      R_orc,r3                # Get outstanding RAID request count
        cmpobe  0,r3,.gp90              # Jif no activity
#
        ldob    D_gpri,r4               # Get global priority
        cmpobe  mbpmaxpri,r4,.gp90      # Jif highest priority
#
# --- Calculate time delay  (R_orc*16)/(D_gpri+1)
#
        ldconst MAXGPDELAY,r14          # Get maximum delay
        ldconst QUANTUM,r13             # Get quantum
        lda     1(r4),r4                # Adjust priority by 1 to
                                        #  eliminate possibility of zero
        shlo    4,r3,r3                 # Multiply activity by 16
        divo    r4,r3,g0                # Calculate delay
        cmpo    r14,g0                  # Check for maximum delay
        sell    g0,r14,g0               # Clip delay if max exceeded
        cmpobg  r13,g0,.gp90            # Jif less than QUANTUM
#
# --- Perform timed wait
#
        call    K$twait                 # Wait
        b       .gp100
#
# --- Exchange processes
#
.gp90:
        call    K$xchang                # Exchange processes
#
# --- Exit
#
.gp100:
        mov     r15,g0                  # Restore g0
        ret
#
#****************************************************************************
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
