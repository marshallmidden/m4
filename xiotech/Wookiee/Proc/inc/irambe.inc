# $Id: irambe.inc 145407 2010-08-10 17:26:37Z m4 $
#**********************************************************************
#
#  NAME: irambe.inc (Internal RAM Back End Processor)
#
#  DESCRIPTION:
#
#       To provide a common means of supplying internal RAM memory
#       definitions for the Back End processor.
#
#  Copyright (c) 2000-2009 Xiotech Corporation.  All rights reserved.
#
#**********************************************************************
#
        .section        .shmem
#
# --- misc definitions -------------------------------------------
#
M_scborg:
        .word   0                       # Origin of free SCB list
#
# --- physical definitions ---------------------------------------
#
P_chn_ind:
        .space  MAXCHN*4,0              # CHN lookup table (pointers)
P_orc:
        .word   0                       # Outstanding req count
        .globl  P_que
P_que:
        .word   0                       # P$que addr
#
R_orc:
        .word   0                       # RAID Outstanding request count
V_orc:
        .word   0                       # Virtual outstanding req count
#
        .align  4
P_exec_qu:
        .space  16,0                    # Executive QCB
P_comp_qu:
        .space  16,0                    # Completion QCB
                                        #  for interrupt routines
#
# --- RAID definitions
#
        .globl  R_exec_qu
R_exec_qu:
        .space  16,0                    # Executive QCB
        .globl  R_r5exec_qu
R_r5exec_qu:
        .space  16,0                    # RAID 5 executive QCB
#
R_scsum:
        .word   0                       # Shadowed NVA checksum
R_nvac:                                 # NVAC structure (alias)
R_nc_nvarec:
        .word   0                       # Start of NVA records
R_nc_csum:
        .word   0                       # Address of NVA checksum
R_nc_mapbase:
        .word   0                       # Base of NVA map
R_nc_mapptr:
        .word   0                       # Base of NVA map
#
# --- virtual definitions ---------------------------------------- *****
#
        .align  4
V_exec_qu:
        .space  16,0                    # Executive QCB

V_exec_mqu:
        .space  16,0                    # Executive expedited QCB

V_exec_hqu:
        .space  16,0                    # Executive High priority QCB

V_exec_xqu:
        .space  16,0                    # Expedited Executive QCB

#*****************************************************************************
# Modelines:
# vi: sw=4 ts=4 expandtab
#
