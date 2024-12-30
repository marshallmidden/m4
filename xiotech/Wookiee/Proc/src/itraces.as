# $Id: itraces.as 143007 2010-06-22 14:48:58Z m4 $
.if     INITIATOR
#******************************************************************************
#
#  NAME: itrace.as
#
#  PURPOSE:
#
#       To provide routine to trace events in modules IDRIVER and APLDRV.
#
#  FUNCTIONS:
#
#  Copyright (c) 1996 - 2001 XIOtech Corporation.  All rights reserved.
#
#******************************************************************************
#
# Trace Formats
#
# Online (0x01)
#
#    0      1      2      3      4      5      6      7
# |------|------|------|------|------|------|------|-------|
# |  01  | I/F  |Event | fflg |  1st word of loop map      |
# |      |      |Type  |      |  CNT   ALPA   ALPA   ALPA  |
# |------|------|------|------|------|------|------|-------|
#           8      9      a      b      c      d      e      f
#        |------|------|------|------|------|------|------|-------|
#        |  2nd word of loop map     |         Time stamp         |
#        | ALPA   ALPA   ALPA   ALPA |                            |
#        |------|------|------|------|------|------|------|-------|
#
#
# offline (0x02)
#
#    0      1      2      3      4      5      6      7
# |------|------|------|------|------|------|------|-------|
# |  02  | I/F  |  00  |  00  |          ci_disq           |
# |      |      |      |      |                            |
# |------|------|------|------|------|------|------|-------|
#           8      9      a      b      c      d      e      f
#        |------|------|------|------|------|------|------|-------|
#        |  00  |  00  |  00  |  00  |         Time stamp         |
#        |      |      |      |      |                            |
#        |------|------|------|------|------|------|------|-------|
#
#
# Discovery Process - Test Unit Ready (0x40)
#
#    0      1      2      3      4      5      6      7
# |------|------|------|------|------|------|------|-------|
# |  40  | I/F  | LID  | LUN  | Flag |  00  |  00  |  00   |
# |      |      |      |      | Byte |      |      |       |
# |------|------|------|------|------|------|------|-------|
#           8      9      a      b      c      d      e      f
#        |------|------|------|------|------|------|------|-------|
#        |        ILT Address        |         Time stamp         |
#        |                           |                            |
#        |------|------|------|------|------|------|------|-------|
#
#
# Discovery Process - Start Stop Unit (0x41)
#
#    0      1      2      3      4      5      6      7
# |------|------|------|------|------|------|------|-------|
# |  41  | I/F  | LID  | LUN  |  00  |  00  |  00  |  00   |
# |      |      |      |      |      |      |      |       |
# |------|------|------|------|------|------|------|-------|
#           8      9      a      b      c      d      e      f
#        |------|------|------|------|------|------|------|-------|
#        |  00  |  00  |  00  |  00  |         Time stamp         |
#        |      |      |      |      |                            |
#        |------|------|------|------|------|------|------|-------|
#
#
# Discovery Process - Inquire LUN (0x42)
#
#    0      1      2      3      4      5      6      7
# |------|------|------|------|------|------|------|-------|
# |  42  | I/F  | LID  | LUN  |  00  |  00  |  00  |  00   |
# |      |      |      |      |      |      |      |       |
# |------|------|------|------|------|------|------|-------|
#           8      9      a      b      c      d      e      f
#        |------|------|------|------|------|------|------|-------|
#        |  00  |  00  |  00  |  00  |         Time stamp         |
#        |      |      |      |      |                            |
#        |------|------|------|------|------|------|------|-------|
#
# Discovery Process - Mode Sense LUN (0x43)
#
#    0      1      2      3      4      5      6      7
# |------|------|------|------|------|------|------|-------|
# |  43  | I/F  | LID  | LUN  |  00  |  00  |  00  |  00   |
# |      |      |      |      |      |      |      |       |
# |------|------|------|------|------|------|------|-------|
#           8      9      a      b      c      d      e      f
#        |------|------|------|------|------|------|------|-------|
#        |  00  |  00  |  00  |  00  |         Time stamp         |
#        |      |      |      |      |                            |
#        |------|------|------|------|------|------|------|-------|
#
# Discovery Process - Task Reissue LUN (0x4E)
#
#    0      1      2      3      4      5      6      7
# |------|------|------|------|------|------|------|-------|
# |  4E  | I/F  | LID  | LUN  |  00  |  00  |  00  |  00   |
# |      |      |      |      |      |      |      |       |
# |------|------|------|------|------|------|------|-------|
#           8      9      a      b      c      d      e      f
#        |------|------|------|------|------|------|------|-------|
#        |  00  |  00  |  00  |  00  |         Time stamp         |
#        |      |      |      |      |                            |
#        |------|------|------|------|------|------|------|-------|
#
#
# Discovery Process - Command Complete (0x4F)
#
#    0      1      2      3      4      5      6      7
# |------|------|------|------|------|------|------|-------|
# |  4F  | I/F  | LID  | LUN  |status|  00  |  00  |  00   |
# |      |      |      |      |      |      |      |       |
# |------|------|------|------|------|------|------|-------|
#           8      9      a      b      c      d      e      f
#        |------|------|------|------|------|------|------|-------|
#        | ASC  | ASCQ |  00  |  00  |         Time stamp         |
#        |      |      |      |      |                            |
#        |------|------|------|------|------|------|------|-------|
#
# Preform Function
# Open Session (0x50)
#
#    0      1      2      3      4      5      6      7
# |------|------|------|------|------|------|------|-------|
# |  50  | I/F  | LID  |  LUN | STC  |Device|  00  |  00   |
# |      |      |      |      |      | Flags|      |       |
# |------|------|------|------|------|------|------|-------|
#           8      9      a      b      c      d      e      f
#        |------|------|------|------|------|------|------|-------|
#        |  Req | Open |  00  |  00  |         Time stamp         |
#        |  Max |  Max |      |      |                            |
#        |------|------|------|------|------|------|------|-------|
#
# Preform Function
# Close Session (0x51)
#
#    0      1      2      3      4      5      6      7
# |------|------|------|------|------|------|------|-------|
# |  51  | I/F  | LID  | LUN  |  Req |Status|  00  |  00   |
# |      |      |      |      | Func |      |      |       |
# |------|------|------|------|------|------|------|-------|
#           8      9      a      b      c      d      e      f
#        |------|------|------|------|------|------|------|-------|
#        |      Extended Status      |         Time stamp         |
#        |                           |                            |
#        |------|------|------|------|------|------|------|-------|
#
# Preform Function
# Request Session Parameters (0x52)
#
#    0      1      2      3      4      5      6      7
# |------|------|------|------|------|------|------|-------|
# |  52  | I/F  | LID  | LUN  |  00  |  00  |  00  |  00   |
# |      |      |      |      |      |      |      |       |
# |------|------|------|------|------|------|------|-------|
#           8      9      a      b      c      d      e      f
#        |------|------|------|------|------|------|------|-------|
#        |  00  |  00  |  00  |  00  |         Time stamp         |
#        |      |      |      |      |                            |
#        |------|------|------|------|------|------|------|-------|
#
#
# Preform Function
# Send SCSI Command (0x53)
#
#    0      1      2      3      4      5      6      7
# |------|------|------|------|------|------|------|-------|
# |  53  | I/F  | LID  | LUN  | CMD  |  TTC |   Tag ID     |
# |      |      |      |      |      |      |              |
# |------|------|------|------|------|------|------|-------|
#           8      9      a      b      c      d      e      f
#        |------|------|------|------|------|------|------|-------|
#        |  00  |  00  |  00  |  00  |         Time stamp         |
#        |      |      |      |      |                            |
#        |------|------|------|------|------|------|------|-------|
#
#
# Preform Function
# Task Management Function (0x54)
#
#    0      1      2      3      4      5      6      7
# |------|------|------|------|------|------|------|-------|
# |  54  | I/F  | LID  | LUN  | TMC  |  00  |  00  |  00   |
# |      |      |      |      |      |      |      |       |
# |------|------|------|------|------|------|------|-------|
#           8      9      a      b      c      d      e      f
#        |------|------|------|------|------|------|------|-------|
#        |  00  |  00  |  00  |  00  |         Time stamp         |
#        |      |      |      |      |                            |
#        |------|------|------|------|------|------|------|-------|
#
#
# Enable Task call  (0x60)
#
#    0      1      2      3      4      5      6      7
# |------|------|------|------|------|------|------|-------|
# |  60  | I/F  | lid  | LUN  |  00  | TTC  |   TAG ID     |
# |      |      |      |      |      |      |              |
# |------|------|------|------|------|------|------|-------|
#           8      9      a      b      c      d      e      f
#        |------|------|------|------|------|------|------|-------|
#        | ENBL | MAX  | Deflt|  Cmd |         Time stamp         |
#        | CNT  | QUE  | T/O  |  T/O |                            |
#        |------|------|------|------|------|------|------|-------|
#
#
# Preform Function Completion  (0x70)
#
#    0      1      2      3      4      5      6      7
# |------|------|------|------|------|------|------|-------|
# |  70  | I/F  | lid  | LUN  |Cmplt |  00  |  00  |  00   |
# |      |      |      |      |Status|      |              |
# |------|------|------|------|------|------|------|-------|
#           8      9      a      b      c      d      e      f
#        |------|------|------|------|------|------|------|-------|
#        | extended completion status|         Time stamp         |
#        |                           |                            |
#        |------|------|------|------|------|------|------|-------|
#
#                          OR
#    0      1      2      3      4      5      6      7
# |------|------|------|------|------|------|------|-------|
# |  70  | I/F  | lid  | LUN  | RFC  |Status|   TAG ID     |
# |      |      |      |      |      |      |              |
# |------|------|------|------|------|------|------|-------|
#           8      9      a      b      c      d      e      f
#        |------|------|------|------|------|------|------|-------|
#        | extended completion status|         Time stamp         |
#        |                           |                            |
#        |------|------|------|------|------|------|------|-------|
#
#
# Command Completion  (0x71)
#
#    0      1      2      3      4      5      6      7
# |------|------|------|------|------|------|------|-------|
# |  71  | I/F  | lid  | LUN  |Cmplt |  00  |   TAG ID     |
# |      |      |      |      |      |      |              |
# |------|------|------|------|------|------|------|-------|
#           8      9      a      b      c      d      e      f
#        |------|------|------|------|------|------|------|-------|
#        | extended completion status|         Time stamp         |
#        |                           |                            |
#        |------|------|------|------|------|------|------|-------|
#
#
# Target Identified (0x80)
#
#    0      1      2      3      4      5      6      7
# |------|------|------|------|------|------|------|-------|
# |  80  |  00  |  00  |  00  |            ALPA            |
# |      |      |      |      |                            |
# |------|------|------|------|------|------|------|-------|
#           8      9      a      b      c      d      e      f
#        |------|------|------|------|------|------|------|-------|
#        |        TMT Address        |  00  |  00  |  00  |  00   |
#        |                           |      |      |      |       |
#        |------|------|------|------|------|------|------|-------|
#
#
# Discovery Complete (0x81)
#
#    0      1      2      3      4      5      6      7
# |------|------|------|------|------|------|------|-------|
# |  81  |  00  |  00  |  00  |  00  |  00  |  00  |  00   |
# |      |      |      |      |      |      |      |       |
# |------|------|------|------|------|------|------|-------|
#           8      9      a      b      c      d      e      f
#        |------|------|------|------|------|------|------|-------|
#        |  00  |  00  |  00  |  00  |  00  |  00  |  00  |  00   |
#        |      |      |      |      |      |      |      |       |
#        |------|------|------|------|------|------|------|-------|
#
#
# Target Gone (0x82)
#
#    0      1      2      3      4      5      6      7
# |------|------|------|------|------|------|------|-------|
# |  82  |  00  |  00  |  ??  |            ALPA            |
# |      |      |      |      |                            |
# |------|------|------|------|------|------|------|-------|
#           8      9      a      b      c      d      e      f
#        |------|------|------|------|------|------|------|-------|
#        | Register g5 (TMT Address?)|        LTMT Address        |
#        |                           |                            |
#        |------|------|------|------|------|------|------|-------|
#
#
# Bad Requestor or Provider ID (0xE0)
#
#    0      1      2      3      4      5      6      7
# |------|------|------|------|------|------|------|-------|
# |  E0  |  RFC |  00  |  00  |         Provider ID        |
# |      |      |      |      |                            |
# |------|------|------|------|------|------|------|-------|
#           8      9      a      b      c      d      e      f
#        |------|------|------|------|------|------|------|-------|
#        |        Requestor ID       |         Time stamp         |
#        |                           |                            |
#        |------|------|------|------|------|------|------|-------|
#
#
# Task Timeout (0xE1)
#
#    0      1      2      3      4      5      6      7
# |------|------|------|------|------|------|------|-------|
# |  E1  |  I/F | lid  | LUN  |  00  |  TTC |   TAG ID     |
# |      |      |      |      |      |      |              |
# |------|------|------|------|------|------|------|-------|
#           8      9      a      b      c      d      e      f
#        |------|------|------|------|------|------|------|-------|
#        |  00  |  00  |  00  |  00  |         Time stamp         |
#        |      |      |      |      |                            |
#        |------|------|------|------|------|------|------|-------|
#
# Ploop start trace (0xF0)
#
#    0      1      2      3      4      5      6      7
# |------|------|------|------|------|------|------|-------|
# |  F0  |                    |                            |
# |      |                    |                            |
# |------|------|------|------|------|------|------|-------|
#           8      9      a      b      c      d      e      f
#        |------|------|------|------|------|------|------|-------|
#        |                           |         Time stamp         |
#        |                           |                            |
#        |------|------|------|------|------|------|------|-------|
#
# GAN trace (0xF1)
#
#    0      1      2      3      4      5      6      7
# |------|------|------|------|------|------|------|-------|
# |  F1  | type |     PID     |            WWD             |
# |      |      |             |                            |
# |------|------|------|------|------|------|------|-------|
#           8      9      a      b      c      d      e      f
#        |------|------|------|------|------|------|------|-------|
#        |            WWD            |            FC4             |
#        |                           |                            |
#        |------|------|------|------|------|------|------|-------|
#
# Fabric Loop trace (0xF2)
#
#    0      1      2      3      4      5      6      7
# |------|------|------|------|------|------|------|-------|
# |  F2  | state| LID  | DSRC |          A L P A           |
# |      |      |      |      |                            |
# |------|------|------|------|------|------|------|-------|
#           8      9      a      b      c      d      e      f
#        |------|------|------|------|------|------|------|-------|
#        |  00  |  00  |  00  |  00  |  00  |  00  |  00  |  00   |                            |
#        |      |      |      |      |      |      |      |       |
#        |------|------|------|------|------|------|------|-------|
#
# Fabric Loop error trace (0xF3)
#
#    0      1      2      3      4      5      6      7
# |------|------|------|------|------|------|------|-------|
# |  F3  | state| LID  | DSRC |          A L P A           |
# |      |      |      |      |                            |
# |------|------|------|------|------|------|------|-------|
#           8      9      a      b      c      d      e      f
#        |------|------|------|------|------|------|------|-------|
#        |  00  |  00  |  00  |  00  |  00  |  00  |  00  |  00   |                            |
#        |      |      |      |      |      |      |      |       |
#        |------|------|------|------|------|------|------|-------|
#
# Fabric Login Error Trace (0xF8)
#
#    0      1      2      3      4      5      6      7
# |------|------|------|------|------|------|------|-------|
# |  F8  | TMT  | LID  | DSRC |          A L P A           |
# |      |state |      |      |                            |
# |------|------|------|------|------|------|------|-------|
#           8      9      a      b      c      d      e      f
#        |------|------|------|------|------|------|------|-------|
#        |login | TMT  |      |      |                            |
#        |status| flags|      |      |                            |
#        |------|------|------|------|------|------|------|-------|
#
# 1st GAN trace (0xFC)
#
#    0      1      2      3      4      5      6      7
# |------|------|------|------|------|------|------|-------|
# |  FC  | type |     PID     |            WWD             |
# |      |      |             |                            |
# |------|------|------|------|------|------|------|-------|
#           8      9      a      b      c      d      e      f
#        |------|------|------|------|------|------|------|-------|
#        |            WWD            |            FC4             |
#        |                           |                            |
#        |------|------|------|------|------|------|------|-------|
#
# Debug trace (0xFF)
#
#    0      1      2      3      4      5      6      7
# |------|------|------|------|------|------|------|-------|
# |  FF  |  00  |  00  |  00  |         User Word          |
# |      |      |      |      |                            |
# |------|------|------|------|------|------|------|-------|
#           8      9      a      b      c      d      e      f
#        |------|------|------|------|------|------|------|-------|
#        |     Return Instruction    |         Time stamp         |
#        |          Pointer          |                            |
#        |------|------|------|------|------|------|------|-------|
#
#
.if     ITRACES
#******************************************************************************
#
#  NAME: it$trc_online
#
#  PURPOSE:
#
#       This routine allows the tracing of an online call from th ISP.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    it$trc_online
#
#  INPUT:
#
#       g2 = event type
#       g4 = icimt
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
it$trc_online:
#
# --- Trace incoming event if appropriate
#
        ldos    ici_tflg(g4),r4         # r4 = trace flags
        lda     I_temp_trace,r10        # r10 = trace record build pointer
        ld      ici_curtr(g4),r3        # r3 = current trace record pointer
        bbc     tflg_online,r4,.trcon_200  # Jif event trace disabled

        mov     0,r13                   # r13 = zero
c       r4 = get_tsc_l() & ~0xf;        # Get free running bus clock.
        ld      ici_lpmapptr(g4),r8     # address of loop lap
        ldob    ici_chpid(g4),r6        # r6 = chip instance
        ldconst trt_online,r5           # r5 = trace record type code
        ld      ispfflags,r7            # r7 = fabric flags
        ldl     (r8),r8                 # r8-r9 = 8 bytes of loop map

        st      r13,0(r10)              # clear word
        stob    r5,trr_trt(r10)         # save trace record type code
        stob    r6,trr_ci(r10)          # save chip instance
        stob    g2,2(r10)               # save event type
        stob    r7,3(r10)               # save fabric flags
        stl     r8,4(r10)               # save loop map
        st      r4,12(r10)              # save timestamp
        ldq     (r10),r4                # r4-r7 = trace record
        ldl     ici_begtr(g4),r8        # r8 = trace area beginning pointer
                                        # r9 = trace area ending pointer
        lda     trr_recsize(r3),r10     # r10 = next trace record pointer
        stq     r4,(r3)                 # save trace record in CIMT trace area
        cmpoble r10,r9,.trcon_100       # Jif trace record pointer has not
                                        #  exceeded end of trace area
        ldos    ici_tflg(g4),r9         # r9 = trace flags
        mov     r8,r10                  # next trace record pointer = beginning
                                        #  trace record pointer
        bbc     tflg_wrapoff,r9,.trcon_100  # Jif wrap off flag not set
                                        #  wrapped.
        stos    r13,ici_tflg(g4)        # turn off traces due to wrap

.trcon_100:
        st      r10,ici_curtr(g4)       # save new current trace record pointer

.trcon_200:
        ret                             # return to caller
#
#******************************************************************************
#
#  NAME: it$trc_offline
#
#  PURPOSE:
#
#       This routine allows the tracing of an offline call from th ISP.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    it$trc_offline
#
#  INPUT:
#
#       g4 = icimt
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       g4-g7 are destroyed.
#
#******************************************************************************
#
it$trc_offline:
#
# --- Trace incoming event if appropriate
#
        ldos    ici_tflg(g4),r4         # r4 = trace flags
        lda     I_temp_trace,r10        # r10 = trace record build pointer
        ld      ici_curtr(g4),r3        # r3 = current trace record pointer
        bbc     tflg_offline,r4,.trcof_200  # Jif event trace disabled

        mov     0,r13                   # r13 = zero
c       r4 = get_tsc_l() & ~0xf;        # Get free running bus clock.
        ldob    ici_chpid(g4),r6        # r6 = chip instance
        ldconst trt_offline,r5          # r5 = trace record type code
        ld      ici_disQ(g4),r7         # r7 = head of discovery queue

        st      r13,0(r10)              # clear word
        stob    r5,trr_trt(r10)         # save trace record type code
        stob    r6,trr_ci(r10)          # save chip instance
        st      r7,4(r10)               # save head of discovery queue
        st      r13,8(r10)              # clear word
        st      r4,12(r10)              # save timestamp
        ldq     (r10),r4                # r4-r7 = trace record
        ldl     ici_begtr(g4),r8        # r8 = trace area beginning pointer
                                        # r9 = trace area ending pointer
        lda     trr_recsize(r3),r10     # r10 = next trace record pointer
        stq     r4,(r3)                 # save trace record in CIMT trace area
        cmpoble r10,r9,.trcof_100       # Jif trace record pointer has not
                                        #  exceeded end of trace area
        ldos    ici_tflg(g4),r9         # r9 = trace flags
        mov     r8,r10                  # next trace record pointer = beginning
                                        #  trace record pointer
        bbc     tflg_wrapoff,r9,.trcof_100  # Jif wrap off flag not set
                                        #  wrapped.
        stos    r13,ici_tflg(g4)        # turn off traces due to wrap

.trcof_100:
        st      r10,ici_curtr(g4)       # save new current trace record pointer

.trcof_200:
        ret                             # return to caller
#******************************************************************************
#
#  NAME: it$trc_inqlun
#
#  PURPOSE:
#
#       This routine records the presenting of the inquire command during
#       the scan process.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    it$trc_inqlun
#
#  INPUT:
#
#       g1 = ILT at 2nd next level
#       g4 = icimt
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
it$trc_inqlun:
#
# --- Trace incoming event if appropriate
#
        lda     -ILTBIAS(g1),r15        # r15 = ILT at 1st next level
        ldos    ici_tflg(g4),r4         # r4 = trace flags
        lda     I_temp_trace,r10        # r10 = trace record build pointer
        ld      ici_curtr(g4),r3        # r3 = current trace record pointer
        bbc     tflg_dislun,r4,.trcil_200  # Jif event trace disabled

        mov     0,r13                   # r13 = zero
c       r4 = get_tsc_l() & ~0xf;        # Get free running bus clock.
        ldob    oil1_chpid(r15),r6      # r6 = chip instance
        ldos    oil1_lid(r15),r7        # r7 = lid
        ldconst trt_inqlun,r5           # r5 = trace record type code
        ldob    oil1_lun(r15),r8        # r8 = lun

        stob    r5,trr_trt(r10)         # save trace record type code
        stob    r6,trr_ci(r10)          # save chip instance
        stob    r7,trr_alpa(r10)        # save alpa
        stob    r8,trr_lun(r10)         # save lun
        st      r13,4(r10)              # clear word
        st      r13,8(r10)              # clear word
        st      r4,12(r10)              # save timestamp
        ldq     (r10),r4                # r4-r7 = trace record
        ldl     ici_begtr(g4),r8        # r8 = trace area beginning pointer
                                        # r9 = trace area ending pointer
        lda     trr_recsize(r3),r10     # r10 = next trace record pointer
        stq     r4,(r3)                 # save trace record in CIMT trace area
        cmpoble r10,r9,.trcil_100       # Jif trace record pointer has not
                                        #  exceeded end of trace area
        ldos    ici_tflg(g4),r9         # r9 = trace flags
        mov     r8,r10                  # next trace record pointer = beginning
                                        #  trace record pointer
        bbc     tflg_wrapoff,r9,.trcil_100  # Jif wrap off flag not set
                                        #  wrapped.
        stos    r13,ici_tflg(g4)        # turn off traces due to wrap

.trcil_100:
        st      r10,ici_curtr(g4)       # save new current trace record pointer

.trcil_200:
        ret                             # return to caller


#******************************************************************************
#
#  NAME: it$trc_turlun
#
#  PURPOSE:
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    it$trc_turlun
#
#  INPUT:
#
#       g1 = ILT at 2nd next level
#       g4 = icimt
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#       This routine records the presenting of the TUR command during
#       the scan process.
#
#******************************************************************************
#
it$trc_turlun:
#
# --- Trace incoming event if appropriate
#
        lda     -ILTBIAS(g1),r15        # r15 = ILT at 1st next level
        ldos    ici_tflg(g4),r4         # r4 = trace flags
        lda     I_temp_trace,r10        # r10 = trace record build pointer
        ld      ici_curtr(g4),r3        # r3 = current trace record pointer
        bbc     tflg_dislun,r4,.trctl_200  # Jif event trace disabled

        mov     0,r13                   # r13 = zero
c       r4 = get_tsc_l() & ~0xf;        # Get free running bus clock.
        ldob    oil1_chpid(r15),r6      # r6 = chip instance
        ldos    oil1_lid(r15),r7        # r7 = lid
        ldconst trt_turlun,r5           # r5 = trace record type code
        ldob    oil1_lun(r15),r8        # r8 = lun

        stob    r5,trr_trt(r10)         # save trace record type code
        stob    r6,trr_ci(r10)          # save chip instance
        stob    r7,trr_alpa(r10)        # save alpa
        stob    r8,trr_lun(r10)         # save lun
        st      r13,4(r10)              # clear word
        st      r15,8(r10)              # save ILT address
        ldob    oil1_flag(r15),r5       # load flag byte
        stob    r5,4(r10)               # save flag byte
        st      r4,12(r10)              # save timestamp
        ldq     (r10),r4                # r4-r7 = trace record
        ldl     ici_begtr(g4),r8        # r8 = trace area beginning pointer
                                        # r9 = trace area ending pointer
        lda     trr_recsize(r3),r10     # r10 = next trace record pointer
        stq     r4,(r3)                 # save trace record in CIMT trace area
        cmpoble r10,r9,.trctl_100       # Jif trace record pointer has not
                                        #  exceeded end of trace area
        ldos    ici_tflg(g4),r9         # r9 = trace flags
        mov     r8,r10                  # next trace record pointer = beginning
                                        #  trace record pointer
        bbc     tflg_wrapoff,r9,.trctl_100  # Jif wrap off flag not set
                                        #  wrapped.
        stos    r13,ici_tflg(g4)        # turn off traces due to wrap

.trctl_100:
        st      r10,ici_curtr(g4)       # save new current trace record pointer

.trctl_200:
        ret                             # return to caller


#******************************************************************************
#
#  NAME: it$trc_ssulun
#
#  PURPOSE:
#
#       This routine records the presenting of the SSU command during
#       the scan process.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    it$trc_ssulun
#
#  INPUT:
#
#       g1 = ILT at 2nd next level
#       g4 = icimt
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
it$trc_ssulun:
#
# --- Trace incoming event if appropriate
#
        lda     -ILTBIAS(g1),r15        # r15 = ILT at 1st next level
        ldos    ici_tflg(g4),r4         # r4 = trace flags
        lda     I_temp_trace,r10        # r10 = trace record build pointer
        ld      ici_curtr(g4),r3        # r3 = current trace record pointer
        bbc     tflg_dislun,r4,.trctl_200  # Jif event trace disabled

        mov     0,r13                   # r13 = zero
c       r4 = get_tsc_l() & ~0xf;        # Get free running bus clock.
        ldob    oil1_chpid(r15),r6      # r6 = chip instance
        ldos    oil1_lid(r15),r7        # r7 = lid
        ldconst trt_ssulun,r5           # r5 = trace record type code
        ldob    oil1_lun(r15),r8        # r8 = lun

        stob    r5,trr_trt(r10)         # save trace record type code
        stob    r6,trr_ci(r10)          # save chip instance
        stob    r7,trr_alpa(r10)        # save alpa
        stob    r8,trr_lun(r10)         # save lun
        st      r13,4(r10)              # clear word
        st      r13,8(r10)              # clear word
        st      r4,12(r10)              # save timestamp
        ldq     (r10),r4                # r4-r7 = trace record
        ldl     ici_begtr(g4),r8        # r8 = trace area beginning pointer
                                        # r9 = trace area ending pointer
        lda     trr_recsize(r3),r10     # r10 = next trace record pointer
        stq     r4,(r3)                 # save trace record in CIMT trace area
        cmpoble r10,r9,.trcsl_100       # Jif trace record pointer has not
                                        #  exceeded end of trace area
        ldos    ici_tflg(g4),r9         # r9 = trace flags
        mov     r8,r10                  # next trace record pointer = beginning
                                        #  trace record pointer
        bbc     tflg_wrapoff,r9,.trcsl_100  # Jif wrap off flag not set
                                        #  wrapped.
        stos    r13,ici_tflg(g4)        # turn off traces due to wrap

.trcsl_100:
        st      r10,ici_curtr(g4)       # save new current trace record pointer

.trcsl_200:
        ret                             # return to caller
#******************************************************************************
#
#  NAME: it$trc_mslun
#
#  PURPOSE:
#
#       This routine records the presenting of the Mode Sense command during
#       the scan process.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    it$trc_mslun
#
#  INPUT:
#
#       g1 = ILT at 2nd next level
#       g4 = icimt
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
it$trc_mslun:
#
# --- Trace incoming event if appropriate
#
        lda     -ILTBIAS(g1),r15        # r15 = ILT at 1st next level
        ldos    ici_tflg(g4),r4         # r4 = trace flags
        lda     I_temp_trace,r10        # r10 = trace record build pointer
        ld      ici_curtr(g4),r3        # r3 = current trace record pointer
        bbc     tflg_dislun,r4,.trcms_200  # Jif event trace disabled

        mov     0,r13                   # r13 = zero
c       r4 = get_tsc_l() & ~0xf;        # Get free running bus clock.
        ldob    oil1_chpid(r15),r6      # r6 = chip instance
        ldos    oil1_lid(r15),r7        # r7 = lid
        ldconst trt_mslun,r5            # r5 = trace record type code
        ldob    oil1_lun(r15),r8        # r8 = lun

        stob    r5,trr_trt(r10)         # save trace record type code
        stob    r6,trr_ci(r10)          # save chip instance
        stob    r7,trr_alpa(r10)        # save alpa
        stob    r8,trr_lun(r10)         # save lun
        st      r13,4(r10)              # clear word
        st      r13,8(r10)              # clear word
        st      r4,12(r10)              # save timestamp
        ldq     (r10),r4                # r4-r7 = trace record
        ldl     ici_begtr(g4),r8        # r8 = trace area beginning pointer
                                        # r9 = trace area ending pointer
        lda     trr_recsize(r3),r10     # r10 = next trace record pointer
        stq     r4,(r3)                 # save trace record in CIMT trace area
        cmpoble r10,r9,.trcms_100       # Jif trace record pointer has not
                                        #  exceeded end of trace area
        ldos    ici_tflg(g4),r9         # r9 = trace flags
        mov     r8,r10                  # next trace record pointer = beginning
                                        #  trace record pointer
        bbc     tflg_wrapoff,r9,.trcms_100  # Jif wrap off flag not set
                                        #  wrapped.
        stos    r13,ici_tflg(g4)        # turn off traces due to wrap

.trcms_100:
        st      r10,ici_curtr(g4)       # save new current trace record pointer

.trcms_200:
        ret                             # return to caller
#******************************************************************************
#
#  NAME: it$trc_dtsk_reissue
#
#  PURPOSE:
#
#       This routine records the reissue of a task during the discovery
#       process.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    it$trc_dtsk_reissue
#
#  INPUT:
#
#       g1 = ILT at 2nd next level
#       g4 = icimt
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
it$trc_dtsk_reissue:
#
# --- Trace incoming event if appropriate
#
        lda     -ILTBIAS(g1),r15        # r15 = ILT at 1st next level
        ldos    ici_tflg(g4),r4         # r4 = trace flags
        lda     I_temp_trace,r10        # r10 = trace record build pointer
        ld      ici_curtr(g4),r3        # r3 = current trace record pointer
        bbc     tflg_dislun,r4,.trtre_200  # Jif event trace disabled

        mov     0,r13                   # r13 = zero
c       r4 = get_tsc_l() & ~0xf;        # Get free running bus clock.
        ldob    oil1_chpid(r15),r6      # r6 = chip instance
        ldos    oil1_lid(r15),r7        # r7 = lid
        ldconst trt_tskreissue,r5       # r5 = trace record type code
        ldob    oil1_lun(r15),r8        # r8 = lun

        stob    r5,trr_trt(r10)         # save trace record type code
        stob    r6,trr_ci(r10)          # save chip instance
        stob    r7,trr_alpa(r10)        # save alpa
        stob    r8,trr_lun(r10)         # save lun
        st      r13,4(r10)              # clear word
        st      r13,8(r10)              # clear word
        st      r4,12(r10)              # save timestamp
        ldq     (r10),r4                # r4-r7 = trace record
        ldl     ici_begtr(g4),r8        # r8 = trace area beginning pointer
                                        # r9 = trace area ending pointer
        lda     trr_recsize(r3),r10     # r10 = next trace record pointer
        stq     r4,(r3)                 # save trace record in CIMT trace area
        cmpoble r10,r9,.trtre_100       # Jif trace record pointer has not
                                        #  exceeded end of trace area
        ldos    ici_tflg(g4),r9         # r9 = trace flags
        mov     r8,r10                  # next trace record pointer = beginning
                                        #  trace record pointer
        bbc     tflg_wrapoff,r9,.trtre_100  # Jif wrap off flag not set
                                        #  wrapped.
        stos    r13,ici_tflg(g4)        # turn off traces due to wrap

.trtre_100:
        st      r10,ici_curtr(g4)       # save new current trace record pointer

.trtre_200:
        ret                             # return to caller

#******************************************************************************
#
#  NAME: it$trc_opnses
#
#  PURPOSE:
#
#       This routine records the processing of an open session function.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    it$trc_opnses
#
#  INPUT:
#
#       g4 = icimt
#       g2 = irp
#       g5 = TMT
#       g6 = TLMT
#       g7 = ismt
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
it$trc_opnses:
#
# --- Trace incoming event if appropriate
#
        ldos    ici_tflg(g4),r4         # r4 = trace flags
        lda     I_temp_trace,r10        # r10 = trace record build pointer
        ld      ici_curtr(g4),r3        # r3 = current trace record pointer
        bbc     tflg_RF,r4,.trcos_200   # Jif event trace disabled

        mov     0,r13                   # r13 = zero
c       r4 = get_tsc_l() & ~0xf;        # Get free running bus clock.
        ldconst trt_opnses,r5           # r5 = trace record type code
        ldob    tm_chipID(g5),r6        # r6 = chip instance
        ldos    tm_lid(g5),r7           # r7 = lid
# NOTE: tlm_lun is a short, so value may be wrong in trace.
        ldob    tlm_lun(g6),r8          # r8 = lun
        ldob    irp_STC(g2),r9          # r9 = Session Type Code

        st      r13,4(r10)              # clear word
        st      r13,8(r10)              # clear word
        stob    r5,trr_trt(r10)         # save trace record type code
        stob    r6,trr_ci(r10)          # save chip instance
        stob    r7,trr_alpa(r10)        # save alpa
        stob    r8,trr_lun(r10)         # save lun
        st      r4,12(r10)              # save timestamp
        stob    r9,4(r10)               # save session type code

        ldob    irp_maxQ(g2),r4         # requested max queue size
        ldob    tlm_maxQ(g6),r5         # open max queue size
        ldob    tlm_dvflgs(g6),r6       # device flags
        stob    r4,8(r10)
        stob    r5,9(r10)
        stob    r6,5(r10)

        ldq     (r10),r4                # r4-r7 = trace record
        ldl     ici_begtr(g4),r8        # r8 = trace area beginning pointer
                                        # r9 = trace area ending pointer
        lda     trr_recsize(r3),r10     # r10 = next trace record pointer
        stq     r4,(r3)                 # save trace record in CIMT trace area
        cmpoble r10,r9,.trcos_100       # Jif trace record pointer has not
                                        #  exceeded end of trace area
        ldos    ici_tflg(g4),r9         # r9 = trace flags
        mov     r8,r10                  # next trace record pointer = beginning
                                        #  trace record pointer
        bbc     tflg_wrapoff,r9,.trcos_100  # Jif wrap off flag not set
                                        #  wrapped.
        stos    r13,ici_tflg(g4)        # turn off traces due to wrap

.trcos_100:
        st      r10,ici_curtr(g4)       # save new current trace record pointer

.trcos_200:
        ret                             # return to caller

#******************************************************************************
#
#  NAME: it$trc_clsses
#
#  PURPOSE:
#
#       This routine records the processing of an close session function.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    it$trc_clsses
#
#  INPUT:
#
#       g4 = icimt
#       g2 = irp
#       g7 = ismt
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
it$trc_clsses:
#
# --- Trace incoming event if appropriate
#
        ldos    ici_tflg(g4),r4         # r4 = trace flags
        lda     I_temp_trace,r10        # r10 = trace record build pointer
        ld      ici_curtr(g4),r3        # r3 = current trace record pointer
        bbc     tflg_RF,r4,.trccs_200   # Jif event trace disabled

        mov     0,r13                   # r13 = zero
c       r4 = get_tsc_l() & ~0xf;        # Get free running bus clock.
        ldconst trt_clsses,r5           # r5 = trace record type code
        ldob    tm_chipID(g5),r6        # r6 = chip instance
        ldos    tm_lid(g5),r7           # r7 = lid
# NOTE: tlm_lun is a short, so value may be wrong in trace.
        ldob    tlm_lun(g6),r8          # r8 = lun

        stob    r5,trr_trt(r10)         # save trace record type code
        stob    r6,trr_ci(r10)          # save chip instance
        stob    r7,trr_alpa(r10)        # save alpa
        stob    r8,trr_lun(r10)         # save lun
        st      r13,4(r10)              # clear word
        st      r4,12(r10)              # save timestamp

        ldob    irp_cmplt(g2),r4        # r4 = completion status
        ld      irp_extcmp(g2),r5       # r5 = extended completion status
        ldob    irp_RFC(g2),r6          # r6 = request function code
        stob    r4,5(r10)               # save completion status
        st      r5,8(r10)               # save extended completion status
        stob    r6,4(r10)               # save request function code

        ldq     (r10),r4                # r4-r7 = trace record
        ldl     ici_begtr(g4),r8        # r8 = trace area beginning pointer
                                        # r9 = trace area ending pointer
        lda     trr_recsize(r3),r10     # r10 = next trace record pointer
        stq     r4,(r3)                 # save trace record in CIMT trace area
        cmpoble r10,r9,.trccs_100       # Jif trace record pointer has not
                                        #  exceeded end of trace area
        ldos    ici_tflg(g4),r9         # r9 = trace flags
        mov     r8,r10                  # next trace record pointer = beginning
                                        #  trace record pointer
        bbc     tflg_wrapoff,r9,.trccs_100  # Jif wrap off flag not set
                                        #  wrapped.
        stos    r13,ici_tflg(g4)        # turn off traces due to wrap

.trccs_100:
        st      r10,ici_curtr(g4)       # save new current trace record pointer

.trccs_200:
        ret                             # return to caller

#******************************************************************************
#
#  NAME: it$trc_sndcmd
#
#  PURPOSE:
#
#       This routine records the processing of an send command function.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    it$trc_sndcmd
#
#  INPUT:
#
#       g2 = irp
#       g4 = icimt
#       g6 = tlmt
#       g7 = ismt
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
it$trc_sndcmd:
#
# --- Trace incoming event if appropriate
#
        ldos    ici_tflg(g4),r4         # r4 = trace flags
        lda     I_temp_trace,r10        # r10 = trace record build pointer
        ld      ici_curtr(g4),r3        # r3 = current trace record pointer
        bbc     tflg_RF,r4,.trcsc_200   # Jif event trace disabled

        mov     0,r13                   # r13 = zero
c       r4 = get_tsc_l() & ~0xf;        # Get free running bus clock.
        ldconst trt_sndcmd,r5           # r5 = trace record type code
        ldob    tm_chipID(g5),r6        # r6 = chip instance
        ldos    tm_lid(g5),r7           # r7 = lid
# NOTE: tlm_lun is a short, so value may be wrong in trace.
        ldob    tlm_lun(g6),r8          # r8 = lun

        stob    r5,trr_trt(r10)         # save trace record type code
        stob    r6,trr_ci(r10)          # save chip instance
        stob    r7,trr_alpa(r10)        # save alpa
        stob    r8,trr_lun(r10)         # save lun
        st      r4,12(r10)              # save timestamp

        ldob    irp_CDB(g2),r4          # r4 = command
        ldob    irp_Tattr_TTC(g2),r6    # r6 = task type code
        ldos    irp_tagID(g2),r7        # r7 = tag id
        stob    r4,4(r10)               # save command
        stob    r6,5(r10)               # save TTC
        stos    r7,6(r10)               # save tag id

        st      r13,8(r10)              # clear word

        ldq     (r10),r4                # r4-r7 = trace record
        ldl     ici_begtr(g4),r8        # r8 = trace area beginning pointer
                                        # r9 = trace area ending pointer
        lda     trr_recsize(r3),r10     # r10 = next trace record pointer
        stq     r4,(r3)                 # save trace record in CIMT trace area
        cmpoble r10,r9,.trcsc_100       # Jif trace record pointer has not
                                        #  exceeded end of trace area
        ldos    ici_tflg(g4),r9         # r9 = trace flags
        mov     r8,r10                  # next trace record pointer = beginning
                                        #  trace record pointer
        bbc     tflg_wrapoff,r9,.trcsc_100  # Jif wrap off flag not set
                                        #  wrapped.
        stos    r13,ici_tflg(g4)        # turn off traces due to wrap

.trcsc_100:
        st      r10,ici_curtr(g4)       # save new current trace record pointer

.trcsc_200:
        ret                             # return to caller


#******************************************************************************
#
#  NAME: it$trc_fcmplt_irp
#
#  PURPOSE:
#
#       This routine records the completion of an function and no ismt is
#       present.
#
#  DESCRIPTION:
#
#       This would occur during a function that does no have a session
#       association. An example would be an open session that has some
#       form of parsing error.
#
#  CALLING SEQUENCE:
#
#       call    it$trc_fcmplt
#
#  INPUT:
#
#       g4 = icimt
#       g2 = irp
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
it$trc_fcmp_irp:
#
# --- Trace incoming event if appropriate
#
        ldos    ici_tflg(g4),r4         # r4 = trace flags
        lda     I_temp_trace,r10        # r10 = trace record build pointer
        ld      ici_curtr(g4),r3        # r3 = current trace record pointer
        bbc     tflg_RFcmplt,r4,.trcfcn_200   # Jif event trace disabled

        mov     0,r13                   # r13 = zero
c       r4 = get_tsc_l() & ~0xf;        # Get free running bus clock.
        ldconst trt_funcmplt,r5         # r5 = trace record type code
        ldob    irp_IFID(g2),r6         # r6 = chip instance
        ldob    irp_Sqlfr_InitID(g2),r7 # r7 = lun
# NOTE: trace information for lun is wrong -- it should be a short, not a byte.
        ldob    irp_Sqlfr_LUN(g2),r8    # r8 = lun

        stob    r5,trr_trt(r10)         # save trace record type code
        stob    r6,trr_ci(r10)          # save chip instance
        stob    r7,trr_alpa(r10)        # save alpa
        stob    r8,trr_lun(r10)         # save lun
        st      r13,4(r10)              # clear word
        st      r4,12(r10)              # save timestamp

        ldob    irp_cmplt(g2),r4        # r4 = completion status
        ld      irp_extcmp(g2),r5       # r5 = extended completion status
        stob    r4,4(r10)               # save completion status
        st      r5,8(r10)               # save extended completion status

        ldq     (r10),r4                # r4-r7 = trace record
        ldl     ici_begtr(g4),r8        # r8 = trace area beginning pointer
                                        # r9 = trace area ending pointer
        lda     trr_recsize(r3),r10     # r10 = next trace record pointer
        stq     r4,(r3)                 # save trace record in CIMT trace area
        cmpoble r10,r9,.trcfcn_100      # Jif trace record pointer has not
                                        #  exceeded end of trace area
        ldos    ici_tflg(g4),r9         # r9 = trace flags
        mov     r8,r10                  # next trace record pointer = beginning
                                        #  trace record pointer
        bbc     tflg_wrapoff,r9,.trcfcn_100  # Jif wrap off flag not set
                                        #  wrapped.
        stos    r13,ici_tflg(g4)        # turn off traces due to wrap

.trcfcn_100:
        st      r10,ici_curtr(g4)       # save new current trace record pointer

.trcfcn_200:
        ret                             # return to caller

#******************************************************************************
#
#  NAME: it$trc_fnccmp_ismt
#
#  PURPOSE:
#
#       This routine records the completion of an function and an ISMT is
#       present.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    it$trc_fnccmplt
#
#  INPUT:
#
#       g2 = irp
#       g4 = icimt
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
it$trc_fcmp_ismt:
#
# --- Trace incoming event if appropriate
#
        ldos    ici_tflg(g4),r4         # r4 = trace flags
        lda     I_temp_trace,r10        # r10 = trace record build pointer
        ld      ici_curtr(g4),r3        # r3 = current trace record pointer
        bbc     tflg_RFcmplt,r4,.trcfci_200   # Jif event trace disabled

        mov     0,r13                   # r13 = zero
c       r4 = get_tsc_l() & ~0xf;        # Get free running bus clock.
        ldconst trt_funcmplt,r5         # r5 = trace record type code
        ldob    tm_chipID(g5),r6        # r6 = chip instance
        ldos    tm_lid(g5),r7           # r7 = lid
# NOTE: tlm_lun is a short, so value may be wrong in trace.
        ldob    tlm_lun(g6),r8          # r8 = lun

        stob    r5,trr_trt(r10)         # save trace record type code
        stob    r6,trr_ci(r10)          # save chip instance
        stob    r7,trr_alpa(r10)        # save alpa
        stob    r8,trr_lun(r10)         # save lun
        st      r13,4(r10)              # clear word
        st      r4,12(r10)              # save timestamp

        ldob    irp_cmplt(g2),r4        # r4 = completion status
        ld      irp_extcmp(g2),r5       # r5 = extended completion status
        ldob    irp_RFC(g2),r6          # r6 = request function code
        ldos    irp_tagID(g2),r7        # r7 = tag id
        stob    r4,5(r10)               # save completion status
        st      r5,8(r10)               # save extended completion status
        stob    r6,4(r10)               # save request function code
        stos    r7,6(r10)               # save tag id

        ldq     (r10),r4                # r4-r7 = trace record
        ldl     ici_begtr(g4),r8        # r8 = trace area beginning pointer
                                        # r9 = trace area ending pointer
        lda     trr_recsize(r3),r10     # r10 = next trace record pointer
        stq     r4,(r3)                 # save trace record in CIMT trace area
        cmpoble r10,r9,.trcfci_100      # Jif trace record pointer has not
                                        #  exceeded end of trace area
        ldos    ici_tflg(g4),r9         # r9 = trace flags
        mov     r8,r10                  # next trace record pointer = beginning
                                        #  trace record pointer
        bbc     tflg_wrapoff,r9,.trcfci_100  # Jif wrap off flag not set
                                        #  wrapped.
        stos    r13,ici_tflg(g4)        # turn off traces due to wrap

.trcfci_100:
        st      r10,ici_curtr(g4)       # save new current trace record pointer

.trcfci_200:
        ret                             # return to caller

#******************************************************************************
#
#  NAME: it$trc_fcmp_badid
#
#  PURPOSE:
#
#       This routine records the completion of an function that required a
#       provider or requestor ID and one or both are invalid.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    it$trc_fcmp_badid
#
#  INPUT:
#
#       g2 = irp
#       g4 = icimt
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
it$trc_fcmp_badid:
#
# --- Trace incoming event if appropriate
#
        ldos    ici_tflg(g4),r4         # r4 = trace flags
        lda     I_temp_trace,r10        # r10 = trace record build pointer
        ld      ici_curtr(g4),r3        # r3 = current trace record pointer
        bbc     tflg_RFcmplt,r4,.trcbid_200   # Jif event trace disabled

        mov     0,r13                   # r13 = zero
c       r4 = get_tsc_l() & ~0xf;        # Get free running bus clock.
        ldconst trt_badid,r5            # r5 = trace record type code
        ldob    irp_RFC(g2),r6          # r6 = request function code
        ld      irp_Pro_ID(g2),r7       # r7 = provider ID
        ld      irp_Req_ID(g2),r8       # r8 = requestor ID

        stob    r5,trr_trt(r10)         # save trace record type code
        stob    r6,trr_ci(r10)          # save RFC
        stos    r13,trr_alpa(r10)       # save alpa
        stob    r7,4(r10)               # save provider id
        st      r8,8(r10)               # save requestor id
        st      r4,12(r10)              # save timestamp

        ldq     (r10),r4                # r4-r7 = trace record
        ldl     ici_begtr(g4),r8        # r8 = trace area beginning pointer
                                        # r9 = trace area ending pointer
        lda     trr_recsize(r3),r10     # r10 = next trace record pointer
        stq     r4,(r3)                 # save trace record in CIMT trace area
        cmpoble r10,r9,.trcbid_100      # Jif trace record pointer has not
                                        #  exceeded end of trace area
        ldos    ici_tflg(g4),r9         # r9 = trace flags
        mov     r8,r10                  # next trace record pointer = beginning
                                        #  trace record pointer
        bbc     tflg_wrapoff,r9,.trcbid_100  # Jif wrap off flag not set
                                        #  wrapped.
        stos    r13,ici_tflg(g4)        # turn off traces due to wrap

.trcbid_100:
        st      r10,ici_curtr(g4)       # save new current trace record pointer

.trcbid_200:
        ret                             # return to caller

#******************************************************************************
#
#  NAME: it$trc_icmdcmplt
#
#  PURPOSE:
#
#       This routine records the completion of an command that is only
#       associated with a CIMT.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    it$trc_icmdcmplt
#
#  INPUT:
#
#       g0 = status
#       g1 = ILT at 2nd next level
#       g4 = icimt
#       g11 = status type 0 IOCB (only if g0 <> 0)
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
it$trc_icmdcmplt:
        lda     -ILTBIAS(g1),r15        # r15 = ILT at 1st level

        ldos    ici_tflg(g4),r4         # r4 = trace flags
        lda     I_temp_trace,r10        # r10 = trace record build pointer
        ld      ici_curtr(g4),r3        # r3 = current trace record pointer
        bbc     tflg_dislun,r4,.trcicc_200  # Jif event trace disabled

        mov     0,r13                   # r13 = zero
c       r4 = get_tsc_l() & ~0xf;        # Get free running bus clock.
        ldob    oil1_chpid(r15),r6      # r6 = chip instance
        ldos    oil1_lid(r15),r7        # r7 = lid
        ldconst trt_icmdcmplt,r5        # r5 = trace record type code
        ldob    oil1_lun(r15),r8        # r8 = lun

        stob    r5,trr_trt(r10)         # save trace record type code
        stob    r6,trr_ci(r10)          # save chip instance
        stob    r7,trr_alpa(r10)        # save alpa
        stob    r8,trr_lun(r10)         # save lun
        st      r13,4(r10)              # clear word
        st      r13,8(r10)              # clear word
        st      r4,12(r10)              # save timestamp
        mov     r13,r5                  # r5 = 0

        cmpobe  0,g0,.trcicc_050        # Jif good status

        ldos    0x16(g11),r9
        bbs     8,r9,.trcicc_045
        lda     0x24(g11),r9          # r7 = pointer to sense data
        b       .trcicc_047
.trcicc_045:
        lda     0x2C(g11),r9
.trcicc_047:
        ldos    12(r9),r5       # get possible ACA/ACAQ
.trcicc_050:
        stob    g0,4(r10)               # save SCSI/Qlogic cmplt status
        stos    r5,8(r10)               # save possible ACA/ACAQ
#        stob    r5,6(r10)               # save SCSI status
#        stob    r6,7(r10)               # save qlogic status
#        st      g11,8(r10)              # save status iocb

        ldq     (r10),r4                # r4-r7 = trace record
        ldl     ici_begtr(g4),r8        # r8 = trace area beginning pointer
                                        # r9 = trace area ending pointer
        lda     trr_recsize(r3),r10     # r10 = next trace record pointer
        stq     r4,(r3)                 # save trace record in CIMT trace area
        cmpoble r10,r9,.trcicc_100      # Jif trace record pointer has not
                                        #  exceeded end of trace area
        ldos    ici_tflg(g4),r9         # r9 = trace flags
        mov     r8,r10                  # next trace record pointer = beginning
                                        #  trace record pointer
        bbc     tflg_wrapoff,r9,.trcicc_100  # Jif wrap off flag not set
                                        #  wrapped.
        stos    r13,ici_tflg(g4)        # turn off traces due to wrap

.trcicc_100:
        st      r10,ici_curtr(g4)       # save new current trace record pointer

.trcicc_200:
        ret                             # return to caller


#******************************************************************************
#
#  NAME: it$trc_cmdcmplt
#
#  PURPOSE:
#
#       This routine records the completion of an command that is
#       associated with a IRP and ISMT.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    it$trc_cmdcmplt
#
#  INPUT:
#
#       g0 = status
#       g2 = irp
#       g4 = icimt
#       g7 = ismt
#       g11 = status type 0 IOCB (only if g0 <> 0)
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
it$trc_cmdcmplt:
#
# --- Trace incoming event if appropriate
#
        ldos    ici_tflg(g4),r4         # r4 = trace flags
        ld      tlm_tmt(g6),r15         # r15 = tmt
        lda     I_temp_trace,r10        # r10 = trace record build pointer
        ld      ici_curtr(g4),r3        # r3 = current trace record pointer
        bbc     tflg_RFcmplt,r4,.trccc_200   # Jif event trace disabled

        mov     0,r13                   # r13 = zero
c       r4 = get_tsc_l() & ~0xf;        # Get free running bus clock.
        ldconst trt_cmdcmplt,r5         # r5 = trace record type code
        ldob    tm_chipID(r15),r6       # r6 = chip instance
        ldos    tm_lid(r15),r7          # r7 = lid
# NOTE: tlm_lun is a short, so value may be wrong in trace.
        ldob    tlm_lun(g6),r8          # r8 = lun

        stob    r5,trr_trt(r10)         # save trace record type code
        stob    r6,trr_ci(r10)          # save chip instance
        stob    r7,trr_alpa(r10)        # save alpa
        stob    r8,trr_lun(r10)         # save lun
        st      r4,12(r10)              # save timestamp

        ldob    irp_cmplt(g2),r4        # r4 = completion status
        ld      irp_extcmp(g2),r5       # r5 = extended completion status
        ldos    irp_tagID(g2),r7        # r7 = tag id
        stob    r4,4(r10)               # save completion status
        st      r5,8(r10)               # save extended completion status
        stob    r13,5(r10)              # clear byte
        stos    r7,6(r10)               # save tag id
        stob    g0,11(r10)              # save SCSI/Qlogic cmplt code

        ldq     (r10),r4                # r4-r7 = trace record
        ldl     ici_begtr(g4),r8        # r8 = trace area beginning pointer
                                        # r9 = trace area ending pointer
        lda     trr_recsize(r3),r10     # r10 = next trace record pointer
        stq     r4,(r3)                 # save trace record in CIMT trace area
        cmpoble r10,r9,.trccc_100       # Jif trace record pointer has not
                                        #  exceeded end of trace area
        ldos    ici_tflg(g4),r9         # r9 = trace flags
        mov     r8,r10                  # next trace record pointer = beginning
                                        #  trace record pointer
        bbc     tflg_wrapoff,r9,.trccc_100  # Jif wrap off flag not set
                                        #  wrapped.
        stos    r13,ici_tflg(g4)        # turn off traces due to wrap

.trccc_100:
        st      r10,ici_curtr(g4)       # save new current trace record pointer

.trccc_200:
        ret                             # return to caller


#******************************************************************************
#
#  NAME: it$trc_enbltsk
#
#  PURPOSE:
#
#       This routine records the enable task call.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    it$trc_enbltsk
#
#  INPUT:
#
#       g0 = status
#       g2 = irp
#       g4 = icimt
#       g6 = tlmt
#       g7 = ismt
#       g11 = status type 0 IOCB (only if g0 <> 0)
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
it$trc_enbltsk:
#
# --- Trace incoming event if appropriate
#
        ldos    ici_tflg(g4),r4         # r4 = trace flags
        lda     I_temp_trace,r10        # r10 = trace record build pointer
        ld      ici_curtr(g4),r3        # r3 = current trace record pointer
        bbc     tflg_tagqueue,r4,.trcet_200   # Jif event trace disabled

        mov     0,r13                   # r13 = zero
c       r4 = get_tsc_l() & ~0xf;        # Get free running bus clock.
        ldconst trt_enbltsk,r5          # r5 = trace record type code
        ldob    tm_chipID(g5),r6        # r6 = chip instance
        ldos    tm_lid(g5),r7           # r7 = lid
# NOTE: tlm_lun is a short, so value may be wrong in trace.
        ldob    tlm_lun(g6),r8          # r8 = lun

        stob    r5,trr_trt(r10)         # save trace record type code
        stob    r6,trr_ci(r10)          # save chip instance
        stob    r7,trr_alpa(r10)        # save alpa
        stob    r8,trr_lun(r10)         # save lun
        st      r4,12(r10)              # save timestamp

# NOTE: tlm_enblcnt is a short, so value may be wrong in trace.
        ldob    tlm_enblcnt(g6),r4      # r4 = enabled task queue count
        ldob    tlm_maxQ(g6),r5         # r5 = max queue depth
        ld      oil2_TTC(g1),r6         # r5 = tag type code
        ldos    oil2_tagID(g1),r7       # r7 = tag id
        stob    r13,4(r10)              # clear byte
        st      r6,5(r10)               # save tag type
        stos    r7,6(r10)               # save tag id
        stob    r4,8(r10)               # save enabled queue count
        stob    r5,9(r10)               # save max que depth
# NOTE: ism_defTO is a short, so value may be wrong in trace.
        ldob    ism_defTO(g7),r4        # r4 = default t/o value
# NOTE: irp_cmdTO is a short, so value may be wrong in trace.
        ldob    irp_cmdTO(g2),r5        # r5 = command t/o value
        stob    r4,10(r10)              # save default
        stob    r5,11(r10)              # save cmd
#        stos    r13,10(r10)             # clear short

        ldq     (r10),r4                # r4-r7 = trace record
        ldl     ici_begtr(g4),r8        # r8 = trace area beginning pointer
                                        # r9 = trace area ending pointer
        lda     trr_recsize(r3),r10     # r10 = next trace record pointer
        stq     r4,(r3)                 # save trace record in CIMT trace area
        cmpoble r10,r9,.trcet_100       # Jif trace record pointer has not
                                        #  exceeded end of trace area
        ldos    ici_tflg(g4),r9         # r9 = trace flags
        mov     r8,r10                  # next trace record pointer = beginning
                                        #  trace record pointer
        bbc     tflg_wrapoff,r9,.trcet_100  # Jif wrap off flag not set
                                        #  wrapped.
        stos    r13,ici_tflg(g4)        # turn off traces due to wrap

.trcet_100:
        st      r10,ici_curtr(g4)       # save new current trace record pointer

.trcet_200:
        ret                             # return to caller
#******************************************************************************
#
#  NAME: it$trc_tsk_TO
#
#  PURPOSE:
#
#       This routine records the enable task call.
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    it$trc_tsk_TO
#
#  INPUT:
#
#       g2 = irp
#       g4 = icimt
#       g6 = tlmt
#       g7 = ismt
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
it$trc_tsk_TO:
#
# --- Trace incoming event if appropriate
#
        ldos    ici_tflg(g4),r4         # r4 = trace flags
        lda     I_temp_trace,r10        # r10 = trace record build pointer
        ld      tlm_tmt(g6),r15         # r15 = tmt
        ld      ici_curtr(g4),r3        # r3 = current trace record pointer
        bbc     tflg_error,r4,.trctto_200   # Jif event trace disabled

        mov     0,r13                   # r13 = zero
c       r4 = get_tsc_l() & ~0xf;        # Get free running bus clock.
        ldconst trt_ttimeout,r5         # r5 = trace record type code
        ldob    tm_chipID(r15),r6       # r6 = chip instance
        ldos    tm_lid(r15),r7          # r7 = lid
# NOTE: tlm_lun is a short, so value may be wrong in trace.
        ldob    tlm_lun(g6),r8          # r8 = lun

        stob    r5,trr_trt(r10)         # save trace record type code
        stob    r6,trr_ci(r10)          # save chip instance
        stob    r7,trr_alpa(r10)        # save alpa
        stob    r8,trr_lun(r10)         # save lun
        st      r4,12(r10)              # save timestamp

        ld      oil2_TTC(g1),r6         # r5 = tag type code
        ldos    oil2_tagID(g1),r7       # r7 = tag id
        stob    r13,4(r10)              # clear byte
        st      r6,5(r10)               # save tag type
        stos    r7,6(r10)               # save tag id
        st      r13,8(r10)              # clear word

        ldq     (r10),r4                # r4-r7 = trace record
        ldl     ici_begtr(g4),r8        # r8 = trace area beginning pointer
                                        # r9 = trace area ending pointer
        lda     trr_recsize(r3),r10     # r10 = next trace record pointer
        stq     r4,(r3)                 # save trace record in CIMT trace area
        cmpoble r10,r9,.trctto_100      # Jif trace record pointer has not
                                        #  exceeded end of trace area
        ldos    ici_tflg(g4),r9         # r9 = trace flags
        mov     r8,r10                  # next trace record pointer = beginning
                                        #  trace record pointer
        bbc     tflg_wrapoff,r9,.trctto_100  # Jif wrap off flag not set
                                        #  wrapped.
        stos    r13,ici_tflg(g4)        # turn off traces due to wrap

.trctto_100:
        st      r10,ici_curtr(g4)       # save new current trace record pointer

.trctto_200:
        ret                             # return to caller
#******************************************************************************
#
#  NAME: it$trc_target_id
#
#  PURPOSE:
#
#       This routine records a target id to lld
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    it$trc_target_id
#
#  INPUT:
#
#       g4 = icimt
#       g5 = tmt
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
it$trc_target_id:
#
# --- Trace incoming event if appropriate
#
        ldos    ici_tflg(g4),r4         # r4 = trace flags
        lda     I_temp_trace,r10        # r10 = trace record build pointer
        ld      ici_curtr(g4),r3        # r3 = current trace record pointer
        bbc     tflg_RFcmplt,r4,.trctargid_200   # Jif event trace disabled

        mov     0,r13                   # r13 = zero
        ldconst trt_target_id,r5        # r5 = trace record type code

        stob    r5,trr_trt(r10)         # save trace record type code
        stob    r13,trr_ci(r10)         # clear
        stos    r13,trr_alpa(r10)       # clear alpa
        ld      tm_alpa(g5),r4          # get alpa
        st      r4,4(r10)               # save alpa
        st      g5,8(r10)               # save tmt address
        st      r13,12(r10)             # save timestamp

        ldq     (r10),r4                # r4-r7 = trace record
        ldl     ici_begtr(g4),r8        # r8 = trace area beginning pointer
                                        # r9 = trace area ending pointer
        lda     trr_recsize(r3),r10     # r10 = next trace record pointer
        stq     r4,(r3)                 # save trace record in CIMT trace area
        cmpoble r10,r9,.trctargid_100   # Jif trace record pointer has not
                                        #  exceeded end of trace area
        ldos    ici_tflg(g4),r9         # r9 = trace flags
        mov     r8,r10                  # next trace record pointer = beginning
                                        #  trace record pointer
        bbc     tflg_wrapoff,r9,.trctargid_100  # Jif wrap off flag not set
                                        #  wrapped.
        stos    r13,ici_tflg(g4)        # turn off traces due to wrap

.trctargid_100:
        st      r10,ici_curtr(g4)       # save new current trace record pointer

.trctargid_200:
        ret
#******************************************************************************
#
#  NAME: it$trc_disc_cmplt
#
#  PURPOSE:
#
#       This routine records the completion of the discovery process
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    it$trc_disc_cmplt
#
#  INPUT:
#
#       g4 = icimt
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
it$trc_disc_cmplt:
#
# --- Trace incoming event if appropriate
#
        ldos    ici_tflg(g4),r4         # r4 = trace flags
        lda     I_temp_trace,r10        # r10 = trace record build pointer
        ld      ici_curtr(g4),r3        # r3 = current trace record pointer
        bbc     tflg_RFcmplt,r4,.trctid_200   # Jif event trace disabled

        mov     0,r13                   # r13 = zero
        ldconst trt_discmplt,r5         # r5 = trace record type code

        stob    r5,trr_trt(r10)         # save trace record type code
        stob    r13,trr_ci(r10)         # clear
        stos    r13,trr_alpa(r10)       # clear alpa
        st      r13,4(r10)              # save provider id
        st      r13,8(r10)              # save requestor id
        st      r13,12(r10)             # save timestamp

        ldq     (r10),r4                # r4-r7 = trace record
        ldl     ici_begtr(g4),r8        # r8 = trace area beginning pointer
                                        # r9 = trace area ending pointer
        lda     trr_recsize(r3),r10     # r10 = next trace record pointer
        stq     r4,(r3)                 # save trace record in CIMT trace area
        cmpoble r10,r9,.trctid_100      # Jif trace record pointer has not
                                        #  exceeded end of trace area
        ldos    ici_tflg(g4),r9         # r9 = trace flags
        mov     r8,r10                  # next trace record pointer = beginning
                                        #  trace record pointer
        bbc     tflg_wrapoff,r9,.trctid_100  # Jif wrap off flag not set
                                        #  wrapped.
        stos    r13,ici_tflg(g4)        # turn off traces due to wrap

.trctid_100:
        st      r10,ici_curtr(g4)       # save new current trace record pointer

.trctid_200:
        ret
#******************************************************************************
#
#  NAME: it$trc_target_gone
#
#  PURPOSE:
#
#       This routine records the completion of the discovery process
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    it$trc_target_gone
#
#  INPUT:
#
#       g4 = icimt
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
it$trc_target_gone:
#
# --- Trace incoming event if appropriate
#
        ldos    ici_tflg(g4),r4         # r4 = trace flags
        lda     I_temp_trace,r10        # r10 = trace record build pointer
        ld      ici_curtr(g4),r3        # r3 = current trace record pointer
        bbc     tflg_RFcmplt,r4,.trctgone_200   # Jif event trace disabled

        mov     0,r13                   # r13 = zero
        ldconst trt_targgone,r5         # r5 = trace record type code

        stob    r5,trr_trt(r10)         # save trace record type code
        stob    r13,trr_ci(r10)         # clear
        stos    r13,trr_alpa(r10)       # clear alpa
        ld      tm_alpa(g5),r4          # get alpa
        ld      tm_ltmt(g5),r5          # get ltmt
        st      r4,4(r10)               # save alpa
        st      g5,8(r10)               # save tmt address
        st      r5,12(r10)              # save ltmt address

        ldq     (r10),r4                # r4-r7 = trace record
        ldl     ici_begtr(g4),r8        # r8 = trace area beginning pointer
                                        # r9 = trace area ending pointer
        lda     trr_recsize(r3),r10     # r10 = next trace record pointer
        stq     r4,(r3)                 # save trace record in CIMT trace area
        cmpoble r10,r9,.trctgone_100    # Jif trace record pointer has not
                                        #  exceeded end of trace area
        ldos    ici_tflg(g4),r9         # r9 = trace flags
        mov     r8,r10                  # next trace record pointer = beginning
                                        #  trace record pointer
        bbc     tflg_wrapoff,r9,.trctgone_100  # Jif wrap off flag not set
                                        #  wrapped.
        stos    r13,ici_tflg(g4)        # turn off traces due to wrap

.trctgone_100:
        st      r10,ici_curtr(g4)       # save new current trace record pointer

.trctgone_200:
        ret
#******************************************************************************
#
#  NAME: it$trc_ploop_start
#
#  PURPOSE:
#
#       This routine records the start of private loop discovery
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    it$trc_ploop_start
#  INPUT:
#
#       g4 = icimt
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
it$trc_ploop_start:
#
# --- Trace incoming event if appropriate
#
        ldos    ici_tflg(g4),r4         # r4 = trace flags
        lda     I_temp_trace,r10        # r10 = trace record build pointer
        ld      ici_curtr(g4),r3        # r3 = current trace record pointer
        bbc     tflg_dislun,r4,.trcplps_200 # Jif event trace disabled

        mov     0,r13                   # r13 = zero
c       r4 = get_tsc_l() & ~0xf;        # Get free running bus clock.
        ldconst trt_ploop_start,r5      # r5 = trace record type code

        stob    r5,trr_trt(r10)         # save trace record type code
        stob    r13,trr_ci(r10)         # clear
        stos    r13,trr_alpa(r10)       # clear alpa
        st      r13,4(r10)              # clear word
        st      r13,8(r10)              # clear word
        st      r4,12(r10)              # save timestamp

        ldq     (r10),r4                # r4-r7 = trace record
        ldl     ici_begtr(g4),r8        # r8 = trace area beginning pointer
                                        # r9 = trace area ending pointer
        lda     trr_recsize(r3),r10     # r10 = next trace record pointer
        stq     r4,(r3)                 # save trace record in ICIMT trace area
        cmpoble r10,r9,.trcplps_100     # Jif trace record pointer has not
                                        #  exceeded end of trace area
        ldos    ici_tflg(g4),r9         # r9 = trace flags
        mov     r8,r10                  # next trace record pointer = beginning
                                        #  trace record pointer
        bbc     tflg_wrapoff,r9,.trcplps_100  # Jif wrap off flag not set
                                        #  wrapped.
        stos    r13,ici_tflg(g4)        # turn off traces due to wrap

.trcplps_100:
        st      r10,ici_curtr(g4)       # save new current trace record pointer

.trcplps_200:
        ret
#******************************************************************************
#
#  NAME: it$trc_1st_GAN
#
#  PURPOSE:
#
#       This routine records the GAN entries
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    it$trc_1st_GAN
#
#  INPUT:
#
#       g1 = gan buffer
#       g4 = icimt
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
it$trc_1st_GAN:
#
# --- Trace incoming event if appropriate
#
        ldos    ici_tflg(g4),r4         # r4 = trace flags
        lda     I_temp_trace,r10        # r10 = trace record build pointer
        ld      ici_curtr(g4),r3        # r3 = current trace record pointer
        bbc     tflg_fabric,r4,.trc1gan_200  # Jif event trace disabled

        mov     0,r13                   # r13 = zero
        ldconst trt_1gantrace,r7        # r7 = trace record type code
        ldl     gan_P_name(g1),r4       # r4/5 = get port wwn
        ld      gan_ptype(g1),r6        # r6 = get type/PID

        st      r6,0(r10)               # save type/PID
        stob    r7,trr_trt(r10)         # save trace record type code
        stl     r4,4(r10)               # save port wwn

        ldob    gan_FC4+2(g1),r7        # get fc4 type
        st      r13,12(r10)             # clear 12-15
        st      r7,12(r10)              # save fc4 type

        ldq     (r10),r4                # r4-r7 = trace record
        ldl     ici_begtr(g4),r8        # r8 = trace area beginning pointer
                                        # r9 = trace area ending pointer
        lda     trr_recsize(r3),r10     # r10 = next trace record pointer
        stq     r4,(r3)                 # save trace record in CIMT trace area
        cmpoble r10,r9,.trc1gan_100     # Jif trace record pointer has not
                                        #  exceeded end of trace area
        ldos    ici_tflg(g4),r9         # r9 = trace flags
        mov     r8,r10                  # next trace record pointer = beginning
                                        #  trace record pointer
        bbc     tflg_wrapoff,r9,.trc1gan_100  # Jif wrap off flag not set
                                        #  wrapped.
        stos    r13,ici_tflg(g4)        # turn off traces due to wrap

.trc1gan_100:
        st      r10,ici_curtr(g4)       # save new current trace record pointer

.trc1gan_200:
        ret
#******************************************************************************
#
#  NAME: it$trc_GAN
#
#  PURPOSE:
#
#       This routine records the GAN entries
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    it$trc_GAN
#
#  INPUT:
#
#       g1 = gan buffer
#       g4 = icimt
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
it$trc_GAN:
#
# --- Trace incoming event if appropriate
#
        ldos    ici_tflg(g4),r4         # r4 = trace flags
        lda     I_temp_trace,r10        # r10 = trace record build pointer
        ld      ici_curtr(g4),r3        # r3 = current trace record pointer
        bbc     tflg_fabric,r4,.trcgan_200   # Jif event trace disabled

        mov     0,r13                   # r13 = zero
        ldconst trt_gantrace,r7         # r7 = trace record type code
        ldl     gan_P_name(g1),r4       # r4/5 = get port wwn
        ld      gan_ptype(g1),r6        # r6 = get type/PID

        st      r6,0(r10)               # save type/PID
        stob    r7,trr_trt(r10)         # save trace record type code
        stl     r4,4(r10)               # save port wwn

        ldob    gan_FC4+2(g1),r7        # get fc4 type
        st      r13,12(r10)             # clear 12-15
        st      r7,12(r10)              # save fc4 type

        ldq     (r10),r4                # r4-r7 = trace record
        ldl     ici_begtr(g4),r8        # r8 = trace area beginning pointer
                                        # r9 = trace area ending pointer
        lda     trr_recsize(r3),r10     # r10 = next trace record pointer
        stq     r4,(r3)                 # save trace record in CIMT trace area
        cmpoble r10,r9,.trcgan_100      # Jif trace record pointer has not
                                        #  exceeded end of trace area
        ldos    ici_tflg(g4),r9         # r9 = trace flags
        mov     r8,r10                  # next trace record pointer = beginning
                                        #  trace record pointer
        bbc     tflg_wrapoff,r9,.trcgan_100  # Jif wrap off flag not set
                                        #  wrapped.
        stos    r13,ici_tflg(g4)        # turn off traces due to wrap

.trcgan_100:
        st      r10,ici_curtr(g4)       # save new current trace record pointer

.trcgan_200:
        ret
#******************************************************************************
#
#  NAME: it$trc_floop
#
#  PURPOSE:
#
#       This routine records the fabric loop
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    it$trc_floop
#
#  INPUT:
#
#       g4 = icimt
#       g5 = tmt
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
it$trc_floop:
#
# --- Trace incoming event if appropriate
#
        ldos    ici_tflg(g4),r4         # r4 = trace flags
        lda     I_temp_trace,r10        # r10 = trace record build pointer
        ld      ici_curtr(g4),r3        # r3 = current trace record pointer
        bbc     tflg_fabric,r4,.trcfl_200   # Jif event trace disabled

        mov     0,r13                   # r13 = zero
        ldconst trt_ftrace,r7           # r7 = trace record type code
        ldob    tm_state(g5),r4         # r4 = tmt state
        ldos    tm_lid(g5),r5           # r5 = LID
        ld      tm_alpa(g5),r6          # r6 = ALPA
        ldob    tm_dsrc(g5),r8          # r8 = discovery source
        ldob    tm_flag(g5),r9          # r9 = tmt flag byte

        stob    r7,trr_trt(r10)         # save trace record type code
        stob    r4,1(r10)               # save tmt state
        stob    r5,2(r10)               # save LID
        stob    r8,3(r10)               # save discovery source
        st      r6,4(r10)               # save alpa
        st      r13,8(r10)              # clear
        stob    r9,9(r10)               # save tmt flag byte
        st      r13,12(r10)             # clear

        ldq     (r10),r4                # r4-r7 = trace record
        ldl     ici_begtr(g4),r8        # r8 = trace area beginning pointer
                                        # r9 = trace area ending pointer
        lda     trr_recsize(r3),r10     # r10 = next trace record pointer
        stq     r4,(r3)                 # save trace record in CIMT trace area
        cmpoble r10,r9,.trcfl_100       # Jif trace record pointer has not
                                        #  exceeded end of trace area
        ldos    ici_tflg(g4),r9         # r9 = trace flags
        mov     r8,r10                  # next trace record pointer = beginning
                                        #  trace record pointer
        bbc     tflg_wrapoff,r9,.trcfl_100  # Jif wrap off flag not set
                                        #  wrapped.
        stos    r13,ici_tflg(g4)        # turn off traces due to wrap

.trcfl_100:
        st      r10,ici_curtr(g4)       # save new current trace record pointer

.trcfl_200:
        ret
#******************************************************************************
#
#  NAME: it$trc_floop_e
#
#  PURPOSE:
#
#       This routine records fabric loop error
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    it$trc_floop_e
#
#  INPUT:
#
#       g4 = icimt
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
it$trc_floop_e:
#
# --- Trace incoming event if appropriate
#
        ldos    ici_tflg(g4),r4         # r4 = trace flags
        lda     I_temp_trace,r10        # r10 = trace record build pointer
        ld      ici_curtr(g4),r3        # r3 = current trace record pointer
        bbc     tflg_fabric,r4,.trcfle_200   # Jif event trace disabled

        mov     0,r13                   # r13 = zero
        ldconst trt_ftrace_e,r7         # r7 = trace record type code
        ldob    tm_state(g5),r4         # r4 = tmt state
        ldos    tm_lid(g5),r5           # r5 = LID
        ld      tm_alpa(g5),r6          # r6 = ALPA
        ldob    tm_dsrc(g5),r8          # r8 = discovery source

        stob    r7,trr_trt(r10)         # save trace record type code
        stob    r4,1(r10)               # save tmt state
        stob    r5,2(r10)               # save LID
        stob    r8,3(r10)               # save discovery source
        st      r6,4(r10)               # save alpa
        st      r13,8(r10)              # clear
        st      r13,12(r10)             # clear

        ldq     (r10),r4                # r4-r7 = trace record
        ldl     ici_begtr(g4),r8        # r8 = trace area beginning pointer
                                        # r9 = trace area ending pointer
        lda     trr_recsize(r3),r10     # r10 = next trace record pointer
        stq     r4,(r3)                 # save trace record in CIMT trace area
        cmpoble r10,r9,.trcfle_100      # Jif trace record pointer has not
                                        #  exceeded end of trace area
        ldos    ici_tflg(g4),r9         # r9 = trace flags
        mov     r8,r10                  # next trace record pointer = beginning
                                        #  trace record pointer
        bbc     tflg_wrapoff,r9,.trcfle_100  # Jif wrap off flag not set
                                        #  wrapped.
        stos    r13,ici_tflg(g4)        # turn off traces due to wrap

.trcfle_100:
        st      r10,ici_curtr(g4)       # save new current trace record pointer

.trcfle_200:
        ret
#******************************************************************************
#
#  NAME: it$trc_floop_e
#
#  PURPOSE:
#
#       This routine records fabric loop error
#
#  DESCRIPTION:
#
#
#  CALLING SEQUENCE:
#
#       call    it$trc_floop_e
#
#  INPUT:
#
#       g4 = icimt
#       g5 = tmt
#
#  OUTPUT:
#
#       None.
#
#  REGS DESTROYED:
#
#       None.
#
#******************************************************************************
#
it$trc_flog_e:
#
# --- Trace incoming event if appropriate
#
        ldos    ici_tflg(g4),r4         # r4 = trace flags
        lda     I_temp_trace,r10        # r10 = trace record build pointer
        ld      ici_curtr(g4),r3        # r3 = current trace record pointer
        bbc     tflg_fabric,r4,.trcflge_200   # Jif event trace disabled

        mov     0,r13                   # r13 = zero
        ldconst trt_flog_e,r7           # r7 = trace record type code
        ldob    tm_state(g5),r4         # r4 = tmt state
        ldos    tm_lid(g5),r5           # r5 = LID
        ld      tm_alpa(g5),r6          # r6 = ALPA
        ldob    tm_dsrc(g5),r8          # r8 = discovery source
        ldob    tm_flag(g5),r9          # r9 = tmt flag byte

        stob    r7,trr_trt(r10)         # save trace record type code
        stob    r4,1(r10)               # save tmt state
        bswap   g1,r4                   # swap alpa bytes
        stob    r5,2(r10)               # save LID
        stob    r8,3(r10)               # save discovery source
        st      r6,4(r10)               # save alpa
        stob    g0,8(r10)               # save login status
        stob    r9,9(r10)               # save tmt flag byte
        stob    g2,10(r10)              # save mailbox 2
        st      r4,12(r10)              # save mailbox 1

        ldq     (r10),r4                # r4-r7 = trace record
        ldl     ici_begtr(g4),r8        # r8 = trace area beginning pointer
                                        # r9 = trace area ending pointer
        lda     trr_recsize(r3),r10     # r10 = next trace record pointer
        stq     r4,(r3)                 # save trace record in CIMT trace area
        cmpoble r10,r9,.trcflge_100     # Jif trace record pointer has not
                                        #  exceeded end of trace area
        ldos    ici_tflg(g4),r9         # r9 = trace flags
        mov     r8,r10                  # next trace record pointer = beginning
                                        #  trace record pointer
        bbc     tflg_wrapoff,r9,.trcflge_100  # Jif wrap off flag not set
                                        #  wrapped.
        stos    r13,ici_tflg(g4)        # turn off traces due to wrap

.trcflge_100:
        st      r10,ici_curtr(g4)       # save new current trace record pointer

.trcflge_200:
        ret
.endif  # ITRACES
.endif  # INITIATOR
#
#******************************************************************************
#
# Modelines:
# vi: sw=4 ts=4 expandtab
#
