# $Id: iram.inc 145407 2010-08-10 17:26:37Z m4 $
#**********************************************************************
#
#  NAME: iram.inc (Internal RAM)
#
#  DESCRIPTION:
#
#       To provide a common means of supplying internal RAM memory
#       definitions.
#
#  Copyright (c) 1997-2007 Xiotech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- kernel definitions ----------------------------------------- *****
#
        .section critdata, lomem
#
# --- Byte aligned storage
#
        .section        .shmem
IRAMBASE:
ispmap:
        .word   0                       # MAXISP - ISP device bitmap
isp2400:
        .word   0                       # MAXISP - ISP 2400 indicator
icb2400:
        .word   0                       # ICB Q Indicator
isprena:
        .word   0                       # Enable ISP response queue processing
                                        # MAXISP - bits - used by interrupt routine
isprqwt:
        .word   0                       # MAXISP - ISP request queue wait flags
ispaywt:
        .word   0                       # MAXISP - ISP async queue stall flags
/*        .byte   0                       # Spare */
ispfflags:
        .word   0                       # MAXISP - Fiber connect flags
fc4flgs:
        .word   0                       # MAXISP - FC-4 registration flags
#
# --- Word aligned storage
#
        .align  2
#
# --- PCI information anchors
#
K_pcidevs:
        .word   0                       # PCI device struct anchor
#
K_xpcb:
        .word   0                       # Executing PCB
K_pcborg:
        .word   0                       # Origin of PCB thread
K_time:
        .word   0                       # Executive time
K_tmrexec_pcb:
        .word   0                       # Timer exec PCB
#
# --- Kernel definitions ----------------------------------------------
#
K_poffset:
        .word   0                       # PCI address gen offset - cacheable
#
# --- Misc definitions ------------------------------------------------
#
        .align  4
m_hbeat_qu:
        .space  16,0                    # Heartbeat QCB
#
# --- Link Layer definitions ------------------------------------------
#
        .align  2
#
#     Statistics pointer
#
L_stattbl:
        .word   0                       # pointer to the lls structure
#
# --- QLogic ISP information
#
        .align  3

ispdefq:
        .space  MAXISP*8                # Head/Tail of deferred queue
                                        #  for MAXISP chips
rtpcb:
        .space  MAXISP*8                # PCBs for ISP handler tasks
                                        # +0 = I/O handler
                                        # +4 = async event handler
ilthead:
        .space  (MAXISP+MAXICL)*8,0     # Forward/back thread for ILTs
                                            #  per ISP instance - head
ilttail:
        .space  (MAXISP+MAXICL)*8,0              # Forward/back thread for ILTs
ispstr:
        .space  MAXISP*4,0              # ISP data structs
isprqptr:
        .space  MAXISP*4                # IN pointers for request handling
asyqa:
        .space  MAXISP*4,0              # Asynchronous event QCB anchors
#
        .align  2
#
# --- Kernel ----------------------------------------------------------
#
        .align  2                       # Word aligned storage
K_rrstate:
        .word   0                       # State of the round robin scheduling
K_rrtimer:
        .word   0                       # Timer value where rr will be started

#
        .align  3                       # Longword aligned structures
K_ncdram:
        .space  fmsiz,0                 # Non cacheable memory FMM
#
# -- pull in the processor specific internal ram definitions
#
.ifdef FRONTEND
        .include    "iramfe.inc"
.else  # BACKEND
        .include    "irambe.inc"
.endif
#
.iramend:
        .byte   0                       # End of SRAM

        .space  1000,0                  # Give some space.
                                 # One memory malloc-er uses space
                                 # between .iramend +16 to end for
                                 # fast variables.
IRAMEND:
#*****************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
