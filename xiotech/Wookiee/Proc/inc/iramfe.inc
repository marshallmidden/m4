# $Id: iramfe.inc 145407 2010-08-10 17:26:37Z m4 $
#**********************************************************************
#
#  NAME: iramfe.inc (Internal RAM Front End Processor)
#
#  DESCRIPTION:
#
#       To provide a common means of supplying internal RAM memory
#       definitions for the Front End processor.
#
#  Copyright (c) 2000-2007 Xiotech Corporation.  All rights reserved.
#
#**********************************************************************
#
# --- Front End definitions ------------------------------------- *****
#
#
# --- Word aligned storage
#
        .section        .shmem
        .align  2
C_exec_pcb:
        .word   0                       # Executive PCB
C_ioexec_pcb:
        .word   0                       # I/O Executive PCB
C_exec_qht:
        .space  8,0                     # Queue head/tail
C_ioexec_qht:
        .space  8,0                     # I/O queue head/tail
#
# --- Note: the forward pointer (word 0) of <c_hlruq> must be initialized to
#           point to <c_tlruq>; word 1 must be zero.
#
#           Backward pointer (word 1) of <c_tlruq> must be initialized to
#           point to <c_hlruq>; word 0 must be zero.
#
#           This initialization must be done before any items are linked
#           to the LRU queue!
#
c_hlruq:
        .space  8,0                     # Cache tag LRU queue - head
c_tlruq:
        .space  8,0                     # Cache tag LRU queue - tail
C_ca:
        .space  casiz,0                 # Cache Statistics
C_fego:
        .word   0                       # Flag to let FE run
#
        .align  4
        .globl  c_wflushq
c_wflushq:
        .space  qusiz,0                 # Flush exec queue
#
        .globl  _gMirrorQueue
gMirrorQueue:
        .space  qusiz,0                 # Mirror exec queue control block
#
        .align  4
cimtDir:
        .space  CIMTMAX*4,0             # CIMT directory area
#
# --- Word aligned storage
#
        .align  2
C_ctv:
        .word   0                       # Controller Throttle Value
C_orc:
        .word   0                       # Cache outstanding host request count
C_flush_orc:
        .word   0                       # Cache outstanding flush request count
C_exec_qcd:
        .word   0                       # Cache current queue depth
C_owsrpc:
        .word   0                       # Cache outstanding write SRP count
C_orsrpc:
        .word   0                       # Cache outstanding read SRP count

        .word   0                       # UNUSED

        .word   0                       # UNUSED

        .word   0                       # UNUSED

        .word   0                       # UNUSED
c_rbfreec:
        .word   0                       # Free RB node list anchor - CDRAM

        .word   0                       # UNUSED
c_rbtotal:
        .word   0                       # Total number of RB Nodes
c_rbavail:
        .word   0                       # Number of RB Nodes available
c_rbifreec:
        .word   0                       # Free RBI node list anchor - CDRAM

        .word   0                       # UNUSED
c_rbitotal:
        .word   0                       # Total number of RBI Nodes
c_rbiavail:
        .word   0                       # Number of RBI Nodes available
c_tgfree:
        .word   0                       # Free cache tag list
c_wcresourc:
        .word   0                       # T/F, Stalled due to WC Resources
C_exec_cqd:
        .space  4,0                     # Current queue depth
C_ioexec_cqd:
        .space  4,0                     # I/O current queue depth
c_bgflush:
        .space  4,0                     # PCB pointer for BG Flush Task
c_allowVLinkOps:
        .word   0                       # Flag to determine if VLink Ops are
                                        #  allowed to go even though C$Stop is
                                        #  active
                                        #   FALSE = VLink Ops treated Normal
                                        #   TRUE  = VLink Ops allowed through
#
# --- Registered IMT list
#       IMTs are placed on this list when not active.
#
C_imt_head:
        .word   0                       # Registered IMT list head pointer
C_imt_tail:
        .word   0                       # Registered IMT list tail pointer
mag_imt_head:
        .word   0                       # allocated IMT list head pointer
mag_imt_tail:
        .word   0                       # allocated IMT list tail pointer
#
# --- MagDriver Statistical counters
#
tag_counts:                             # tag queue type counts
tag_00:
        .word   0                       # tag type 00 counter
tag_01:
        .word   0                       # tag type 01 counter
tag_02:
        .word   0                       # tag type 02 counter
tag_03:
        .word   0                       # tag type 03 counter
tag_04:
        .word   0                       # tag type 04 counter
tag_05:
        .word   0                       # tag type 05 counter
tag_06:
        .word   0                       # tag type 06 counter
tag_07:
        .word   0                       # tag type 07 counter
#
# --- DLM VLink counters
#
D_vlorc:
        .word   0                       # DLM VLink Outstanding Request Counter
#
# --- ISP
#
req_cnt:
        .word   0                       # //// TEMP //// request counter
#**********************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
