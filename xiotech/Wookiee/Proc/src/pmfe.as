# $Id: pmfe.as 89682 2009-06-19 15:25:29Z m4 $
#**********************************************************************
#
#  NAME: pmfe.as
#
#  PURPOSE:
#
#       Support module for Front End processor specific MISC functions
#
#  FUNCTIONS:
#
#       M$ailtirp  - Assign ILT/IRP combo w/wait
#       M$rirp     - Release IRP
#
#       This module employs no processes.
#
#  Copyright (c) 1996-2008 XIOtech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- global function declarations ------------------------------------
#
        .globl  pm$init                 # Initialize packets
        .globl  M$ailtirp               # Assign ILT/IRP combo w/wait
        .globl  M$rirp                  # Release IRP
#
# --- global usage data definitions -----------------------------------
#
        .data
        .align  2
#
# --- executable code -------------------------------------------------
#
        .text
#**********************************************************************
#
#  NAME: pm$init
#
#  PURPOSE:
#       To provide a means of initializing this module for this processor
#
#  DESCRIPTION:
#       For the FE processor no initialization is currently required so it
#       simply returns.
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
#       none
#
#**********************************************************************
#
pm$init:
        ret
#
#******************************************************************************
#
#  NAME: M$ailtirp
#
#  PURPOSE:
#       Performs the necessary processing to allocate an ILT and IRP.
#
#  DESCRIPTION:
#       Allocates an ILT and IRP for the caller. The IRP is cleared
#       before returning to the caller.
#
#  CALLING SEQUENCE:
#       call    M$ailtirp
#
#  INPUT:
#       None.
#
#  OUTPUT:
#       g0 = IRP address
#       g1 = ILT address
#
#  REGS DESTROYED:
#       Reg. g0-g1 destroyed.
#
#******************************************************************************
#
M$ailtirp:
c       g1 = get_ilt();                 # Allocate an ILT
.ifdef M4_DEBUG_ILT
c CT_history_printf("%s%s:%u get_ilt 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g1);
.endif # M4_DEBUG_ILT
c       g0 = get_irp();                 # Get IRP from local memory
.ifdef M4_DEBUG_IRP
c CT_history_printf("%s%s:%u get_irp 0x%08lx\n", FEBEMESSAGE, __FILE__, __LINE__, g0);
.endif # M4_DEBUG_IRP
        ret
#
#****************************************************************************
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
